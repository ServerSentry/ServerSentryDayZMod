/**
    \file init.c
    \brief Runs native backup round-trip checks in an isolated DayZ server mission.
*/

/** \brief Exposes retention operations for filesystem integration checks. */
class ServerSentryBackupRetentionCheck : ServerSentryCharacterBackupService
{
    /**
        \brief Verifies oldest-first retention, zero retention and path validation.
        \return \p true when all checks pass.
    */
    bool Run()
    {
        string directory = "$profile:backup-retention-check";
        MakeDirectory(directory);
        for (int i = 0; i < 6; i++)
        {
            FileHandle file = OpenFile(directory + "/20261019T000000Z_" + i.ToStringLen(4) + ".bin", FileMode.WRITE);
            if (!file)
                return false;
            FPrint(file, "retention fixture");
            CloseFile(file);
        }
        if (!TrimBackups(directory, 2))
            return false;
        array<string> names = ListNames(directory + "/*.bin");
        if (names.Count() != 2 || names[0] != "20261019T000000Z_0004.bin" || names[1] != "20261019T000000Z_0005.bin")
            return false;
        if (!TrimBackups(directory, 0))
            return false;
        names = ListNames(directory + "/*.bin");
        return names.Count() == 0 && IsSteamId("76561198000000001") && !IsSteamId("../other-player");
    }
}

/** \brief Exercises the complete production capture path with an isolated, server-owned test identity. */
class ServerSentryBackupCaptureCheck : ServerSentryCharacterBackupService
{
    /**
        \brief Supplies an isolated identity for a temporary native test character.
        \param player \p temporary fixture character.
        \return \p numeric fixture ID, or empty for a missing character.
    */
    override protected string BackupOwnerId(PlayerBase player)
    {
        if (!player)
            return "";
        return "76561198000000001";
    }

    /**
        \brief Checks age filtering and actual committed snapshot creation through the production service.
        \param player \p populated temporary character.
        \return \p true when the age gate and complete save/copy/retention path succeed.
    */
    bool Run(PlayerBase player)
    {
        s_Instance = this;
        m_Config = new ServerSentryCharBackupConfig();
        m_Config.MaxBackupFiles = 2;
        m_Config.MinItemsInInventory = 0;
        string directory = BACKUP_DIR + "/" + BackupOwnerId(player);
        MakeDirectory(BACKUP_DIR);
        MakeDirectory(directory);
        TrimBackups(directory, 0);
        string message;
        bool aliveRejected = !TryBackupOnDeath(player, message) && message.IndexOf("dead character") >= 0;
        m_Config.EnableCharacterBackup = false;
        PlayerBase disabled = Victim();
        disabled.SetHealth("", "Health", 0);
        array<string> files = ListNames(directory + "/*.bin");
        bool disabledRejected = files.Count() == 0;
        m_Config.EnableCharacterBackup = true;
        PlayerBase young = Victim();
        young.SetHealth("", "Health", 0);
        files = ListNames(directory + "/*.bin");
        bool youngRejected = files.Count() == 0;
        m_Config.MinCharacterLiveTimeSeconds = 0;
        array<PlayerBase> victims = new array<PlayerBase>();
        for (int i = 0; i < 3; i++)
        {
            PlayerBase victim = Victim();
            victims.Insert(victim);
            victim.SetHealth("", "Health", 0);
            if (TryBackupOnDeath(victim, message) || message.IndexOf("already been processed") < 0)
                return false;
        }
        files = ListNames(directory + "/*.bin");
        bool retained = files.Count() == 2;
        ServerSentryCharacterBackupsDto listed = ListBackups(BackupOwnerId(player), 1, player);
        bool metadata = listed && listed.total == 2 && listed.backups.Count() == 2 && listed.backups[0].compatible == 1 && listed.backups[0].itemCount == 5;
        // Rebuild tracking from disk, then emulate loaded-entity observations.
        // Persistent-only resolution must not depend on this mission's runtime key.
        ServerSentryBackupItemIndex.Stop();
        ServerSentryBackupItemIndex.BeginMission();
        foreach (PlayerBase indexed : victims)
        {
            array<EntityAI> indexedItems = CharacterInventory(indexed);
            foreach (EntityAI loaded : indexedItems)
                ServerSentryBackupItemIndex.Observe(loaded);
        }
        bool persisted;
        if (retained)
        {
            ServerSentryBackupItemManifest disk = ServerSentryBackupItemIndex.ReadManifest(directory + "/" + files[1]);
            ServerSentryBackupItemIdentity probe = new ServerSentryBackupItemIdentity();
            probe.Type = disk.Items[1].Type;
            probe.PersistentKey = disk.Items[1].PersistentKey;
            probe.RuntimeKey = "different-mission";
            EntityAI matched;
            persisted = probe.PersistentKey != "" && ServerSentryBackupItemIndex.Resolve(probe, matched) && matched == victims[2].GetInventory().FindAttachment(InventorySlots.BACK);
        }
        Print("[CharacterBackupNativeCheck] PersistentReindex=" + persisted.ToString());
        bool restored;
        bool rolledBack;
        bool guarded;
        if (retained)
        {
            restored = CheckRestore(player, victims[2], directory + "/" + files[1]);
            rolledBack = CheckConflict(player, victims[1], directory + "/" + files[0]);
            guarded = CheckRecoveryGuard(player, directory + "/" + files[0]);
        }
        Print("[CharacterBackupNativeCheck] DeathOnly alive=" + aliveRejected.ToString() + " disabled=" + disabledRejected.ToString() + " young=" + youngRejected.ToString() + " retention=" + retained.ToString() + " metadata=" + metadata.ToString() + " reclaim=" + restored.ToString() + " rollback=" + rolledBack.ToString());
        foreach (PlayerBase dead : victims)
            GetGame().ObjectDelete(dead);
        GetGame().ObjectDelete(disabled);
        GetGame().ObjectDelete(young);
        s_Instance = null;
        return aliveRejected && disabledRejected && youngRejected && retained && metadata && persisted && restored && rolledBack && guarded;
    }

    PlayerBase Victim()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().CreatePlayer(null, "SurvivorM_Mirek", "7500 400 7500", 0, "NONE"));
        player.OnConnect();
        EntityAI bag = player.GetInventory().CreateAttachment("MountainBag_Blue");
        bag.GetInventory().CreateInInventory("WaterBottle");
        bag.GetInventory().CreateInInventory("WorkingGloves_Black");
        // A holstered/packed rifle cannot release its magazine through normal
        // gameplay inventory checks. Restore must preserve this intact subtree.
        Weapon_Base rifle = Weapon_Base.Cast(bag.GetInventory().CreateInInventory("M4A1"));
        Magazine attached = Magazine.Cast(rifle.GetInventory().CreateAttachment("Mag_STANAG_30Rnd"));
        attached.ServerSetAmmoCount(7);
        rifle.RandomizeFSMState();
        player.SetAllowDamage(true);
        return player;
    }

    bool CheckRestore(PlayerBase recipient, PlayerBase corpse, string path)
    {
        array<EntityAI> items = CharacterInventory(corpse);
        EntityAI bag = items[1];
        EntityAI bottle;
        EntityAI gloves;
        Weapon_Base rifle;
        foreach (EntityAI item : items)
        {
            if (item.GetType() == "WaterBottle") bottle = item;
            if (item.GetType() == "WorkingGloves_Black") gloves = item;
            if (item.GetType() == "M4A1") rifle = Weapon_Base.Cast(item);
        }
        if (!bottle || !gloves || !rifle)
            return false;
        Magazine magazine = rifle.GetMagazine(0);
        magazine.ServerSetAmmoCount(3);
        // This post-death item was never in the backup. Keep it inside the
        // reclaimed container instead of deleting someone else's later loot.
        EntityAI extra = bag.GetInventory().CreateInInventory("Rag");
        PlayerBase donor = PlayerBase.Cast(GetGame().CreatePlayer(null, "SurvivorM_Mirek", "7501 400 7500", 0, "NONE"));
        donor.SetAllowDamage(false);
        InventoryLocation hands = new InventoryLocation();
        hands.SetHands(donor, gloves);
        if (!Move(gloves, hands))
            return false;
        gloves.SetHealth01("", "Health", 0.56);
        GetGame().ObjectDelete(bottle);
        bool restored = LoadPlayerState(recipient, path);
        array<EntityAI> result = CharacterInventory(recipient);
        bool hasReplacement;
        foreach (EntityAI restoredItem : result)
        {
            if (restoredItem.GetType() == "WaterBottle") hasReplacement = true;
        }
        bool exact = rifle.GetMagazine(0) == magazine && magazine.GetAmmoCount() == 3 && result.Find(rifle) >= 0 && extra && extra.GetHierarchyRootPlayer() == recipient && result.Find(bag) >= 0 && result.Find(gloves) >= 0 && gloves.GetHierarchyRootPlayer() == recipient && Math.AbsFloat(gloves.GetHealth01("", "Health") - 0.56) < 0.001 && hasReplacement;
        bool once = FileExist(path + ".restored") && !LoadPlayerState(recipient, path);
        Print("[CharacterBackupNativeCheck] Transaction success=" + restored.ToString() + " originalInstances=" + exact.ToString() + " once=" + once.ToString() + " living=" + recipient.IsAlive().ToString());
        GetGame().ObjectDelete(donor);
        return restored && exact && once && recipient.IsAlive();
    }

    bool CheckRecoveryGuard(PlayerBase player, string backup)
    {
        string recovery = BACKUP_DIR + "/" + BackupOwnerId(player) + "/restore.rollback";
        if (FileExist(recovery)) return false;
        FileHandle file = OpenFile(recovery, FileMode.WRITE);
        FPrint(file, "existing recovery must survive");
        CloseFile(file);
        array<EntityAI> before = CharacterInventory(player);
        bool refused = !LoadPlayerState(player, backup);
        string contents;
        file = OpenFile(recovery, FileMode.READ);
        ReadFile(file, contents, 128);
        CloseFile(file);
        array<EntityAI> after = CharacterInventory(player);
        bool unchanged = before.Count() == after.Count();
        foreach (EntityAI item : before)
        {
            if (after.Find(item) < 0) unchanged = false;
        }
        bool preserved = contents == "existing recovery must survive";
        Print("[CharacterBackupNativeCheck] RecoveryGuard refused=" + refused.ToString() + " filePreserved=" + preserved.ToString() + " inventoryUnchanged=" + unchanged.ToString());
        DeleteFile(recovery);
        return refused && preserved && unchanged;
    }

    bool CheckConflict(PlayerBase recipient, PlayerBase corpse, string path)
    {
        array<EntityAI> items = CharacterInventory(corpse);
        EntityAI bag = items[1];
        EntityAI bottle;
        foreach (EntityAI item : items)
        {
            if (item.GetType() == "WaterBottle") bottle = item;
        }
        ServerSentryBackupEntityState bottleState = new ServerSentryBackupEntityState();
        if (!bottleState.Capture(bottle, corpse, items))
            return false;
        GetGame().ObjectDelete(bottle);
        EntityAI blocker = GameInventory.LocationCreateEntity(bottleState.Destination(bag, null), "WaterBottle", ECE_KEEPHEIGHT, RF_DEFAULT);
        if (!blocker)
            return false;
        array<EntityAI> before = CharacterInventory(recipient);
        bool refused = !LoadPlayerState(recipient, path);
        array<EntityAI> after = CharacterInventory(recipient);
        bool same = before.Count() == after.Count();
        foreach (EntityAI old : before)
        {
            if (after.Find(old) < 0) same = false;
        }
        bool returned = bag.GetHierarchyRootPlayer() == corpse && blocker.GetHierarchyRootPlayer() == corpse;
        bool unlocked = !FileExist(path + ".restore-pending") && !FileExist(path + ".restored");
        Print("[CharacterBackupNativeCheck] Conflict refused=" + refused.ToString() + " recipientUnchanged=" + same.ToString() + " originalsReturned=" + returned.ToString() + " retryUnlocked=" + unlocked.ToString());
        return refused && same && returned && unlocked;
    }
}

/** \brief Exercises actual engine serialization, inventory placement, item quantities and mixed ammunition. */
class ServerSentryBackupNativeCheck
{
    /** \brief Creates a test inventory, round-trips it and exits with a machine-readable result. */
    static void Run()
    {
        PlayerBase source = PlayerBase.Cast(GetGame().CreatePlayer(null, "SurvivorM_Mirek", "7500 400 7500", 0, "NONE"));
        PlayerBase destination = PlayerBase.Cast(GetGame().CreatePlayer(null, "SurvivorM_Mirek", "7500 400 7500", 0, "NONE"));
        bool passed = source && destination;
        if (passed)
        {
            source.SetAllowDamage(false);
            destination.SetAllowDamage(false);
            passed = RoundTrip(source, destination);
        }
        ServerSentryBackupRetentionCheck retention = new ServerSentryBackupRetentionCheck();
        bool retained = retention.Run();
        bool captured;
        if (source)
        {
            source.OnConnect();
            ServerSentryBackupCaptureCheck capture = new ServerSentryBackupCaptureCheck();
            captured = capture.Run(source);
        }
        bool migrated = CheckConfigMigration();
        Print("[CharacterBackupNativeCheck] RoundTrip=" + passed.ToString() + " Retention=" + retained.ToString() + " Capture=" + captured.ToString() + " Migration=" + migrated.ToString());
        if (source)
            GetGame().ObjectDelete(source);
        if (destination)
            GetGame().ObjectDelete(destination);
        int result = 1;
        if (passed && retained && captured && migrated)
        {
            Print("[CharacterBackupNativeCheck] PASS");
            result = 0;
        }
        GetGame().RequestExit(result);
    }

    /**
        \brief Checks legacy JSON migration, explicit values, unknown properties and idempotency.
        \return \p true when migration preserves supplied values and adds missing settings.
    */
    static bool CheckConfigMigration()
    {
        ServerSentryCharBackupConfig config = new ServerSentryCharBackupConfig();
        string updated;
        string legacy = "{\"EnableItemExport\":false,\"Custom\":\"keep } text\"}";
        if (!ServerSentryBackupConfigMigration.Build(legacy, config, updated) || updated.IndexOf("\"Custom\":\"keep } text\"") < 0)
            return false;
        ServerSentryLoggingConfig loaded;
        string error;
        if (!JsonFileLoader<ServerSentryLoggingConfig>.LoadData(updated, loaded, error) || loaded.EnableItemExport || loaded.CharBackupConfig.MinCharacterLiveTimeSeconds != 3600)
            return false;
        string unchanged;
        if (ServerSentryBackupConfigMigration.Build(updated, config, unchanged))
            return false;
        config.EnableCharacterBackup = false;
        config.MaxBackupFiles = 2;
        legacy = "{\"CharBackupConfig\":{\"EnableCharacterBackup\":false,\"MaxBackupFiles\":2},\"Custom\":\"keep\"}";
        if (!ServerSentryBackupConfigMigration.Build(legacy, config, updated))
            return false;
        if (!JsonFileLoader<ServerSentryLoggingConfig>.LoadData(updated, loaded, error))
            return false;
        if (loaded.CharBackupConfig.EnableCharacterBackup || loaded.CharBackupConfig.MaxBackupFiles != 2 || loaded.CharBackupConfig.BackupIntervalSeconds != 0 || updated.IndexOf("\"Custom\":\"keep\"") < 0)
            return false;
        return ServerSentryBackupConfigMigration.Build("{\"CharBackupConfig\":null}", config, updated);
    }

    /**
        \brief Checks native entity state against live entities after binary serialization.
        \param source \p temporary source survivor.
        \param destination \p temporary destination survivor.
        \return \p true when the round trip preserves checked values.
    */
    static bool RoundTrip(PlayerBase source, PlayerBase destination)
    {
        EntityAI backpack = source.GetInventory().CreateAttachment("MountainBag_Blue");
        if (!backpack)
            return false;
        ItemBase bottle = ItemBase.Cast(backpack.GetInventory().CreateInInventory("WaterBottle"));
        Magazine magazine = Magazine.Cast(backpack.GetInventory().CreateInInventory("Mag_STANAG_30Rnd"));
        Weapon_Base rifle = Weapon_Base.Cast(source.GetHumanInventory().CreateInHands("Mosin9130"));
        EntityAI headtorch = source.GetInventory().CreateAttachment("Headtorch_Grey");
        if (!bottle || !magazine || !rifle || !headtorch)
            return false;
        EntityAI battery = headtorch.GetInventory().CreateAttachment("Battery9V");
        if (!battery || !battery.GetCompEM() || !headtorch.GetCompEM())
            return false;
        battery.GetCompEM().SetEnergy(37);
        if (headtorch.GetCompEM().GetEnergySource() != battery && !headtorch.GetCompEM().PlugThisInto(battery))
            return false;
        headtorch.GetCompEM().SwitchOn();
        bool expectedPower = headtorch.GetCompEM().IsSwitchedOn();
        // Switching on performs one native consumption step immediately.
        float expectedBatteryEnergy = battery.GetCompEM().GetEnergy();
        bottle.SetQuantity(230);
        bottle.SetWet(0.35);
        // Native WaterBottle wetness is clamped by its configured maximum.
        float expectedWet = bottle.GetWet();
        bottle.SetHealth01("", "Health", 0.63);
        magazine.ServerSetAmmoCount(0);
        if (!magazine.ServerStoreCartridge(0.1, "Bullet_556x45") || !magazine.ServerStoreCartridge(0.2, "Bullet_556x45Tracer"))
            return false;
        if (!rifle.PushCartridgeToChamber(0, 0.15, "Bullet_762x54") || !rifle.PushCartridgeToInternalMagazine(0, 0.25, "Bullet_762x54Tracer") || !rifle.PushCartridgeToInternalMagazine(0, 0.35, "Bullet_762x54"))
            return false;
        rifle.RandomizeFSMState();
        source.SetQuickBarEntityShortcut(bottle, 0, true);
        source.GetStatWater().Set(1700);
        source.GetStatEnergy().Set(2100);
        source.SetAllowDamage(true);
        source.SetHealth01("", "Health", 0.73);
        source.SetHealth01("", "Blood", 0.64);
        source.SetHealth01("", "Shock", 0.82);
        source.SetHealth01("RightLeg", "Health", 0.31);
        float expectedHealth = source.GetHealth01("", "Health");
        float expectedBlood = source.GetHealth01("", "Blood");
        float expectedShock = source.GetHealth01("", "Shock");
        float expectedLeg = source.GetHealth01("RightLeg", "Health");
        source.SetAllowDamage(false);

        array<EntityAI> originals = new array<EntityAI>();
        source.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, originals);
        int rootIndex = originals.Find(source);
        if (rootIndex >= 0)
            originals.RemoveOrdered(rootIndex);
        originals.InsertAt(source, 0);
        array<ref ServerSentryBackupEntityState> states = new array<ref ServerSentryBackupEntityState>();
        FileSerializer file = new FileSerializer();
        if (!file.Open("$profile:backup-native-roundtrip.tmp", FileMode.WRITE))
            return false;
        bool success = true;
        foreach (EntityAI original : originals)
        {
            ServerSentryBackupEntityState state = new ServerSentryBackupEntityState();
            if (!state.Capture(original, source, originals) || !file.Write(state))
            {
                Print("[CharacterBackupNativeCheck] Capture failed: " + original.GetType());
                success = false;
                break;
            }
            original.OnStoreSave(file);
        }
        file.Close();
        if (!success || !file.Open("$profile:backup-native-roundtrip.tmp", FileMode.READ))
            return false;
        array<EntityAI> restored = new array<EntityAI>();
        for (int i = 0; success && i < originals.Count(); i++)
        {
            ServerSentryBackupEntityState saved;
            success = file.Read(saved);
            if (!success || !saved)
            {
                Print("[CharacterBackupNativeCheck] Read metadata failed at " + i.ToString());
                break;
            }
            EntityAI entity = destination;
            if (i > 0)
            {
                InventoryLocation location = saved.Destination(restored[saved.Parent], null);
                entity = GameInventory.LocationCreateEntity(location, saved.Type, ECE_KEEPHEIGHT, RF_DEFAULT);
            }
            success = entity && entity.OnStoreLoad(file, GetGame().SaveVersion());
            if (!success)
                Print("[CharacterBackupNativeCheck] OnStoreLoad/create failed: " + saved.Type);
            if (success)
            {
                restored.Insert(entity);
                states.Insert(saved);
            }
        }
        file.Close();
        for (int index = restored.Count() - 1; success && index >= 0; index--)
        {
            success = states[index].Apply(restored[index]);
            if (!success)
                Print("[CharacterBackupNativeCheck] Native apply failed: " + states[index].Type);
            restored[index].AfterStoreLoad();
        }
        for (int energyIndex = restored.Count() - 1; success && energyIndex >= 0; energyIndex--)
            success = states[energyIndex].ApplyEnergy(restored[energyIndex], restored);
        if (!success)
            return false;
        ItemBase restoredBottle = ItemBase.Cast(restored[originals.Find(bottle)]);
        Magazine restoredMagazine = Magazine.Cast(restored[originals.Find(magazine)]);
        Weapon_Base restoredRifle = Weapon_Base.Cast(restored[originals.Find(rifle)]);
        EntityAI restoredBattery = restored[originals.Find(battery)];
        EntityAI restoredHeadtorch = restored[originals.Find(headtorch)];
        float damage;
        string ammo;
        bool chamber = restoredRifle.GetCartridgeInfo(0, damage, ammo);
        bool correctChamber = chamber && ammo == "Bullet_762x54" && Math.AbsFloat(damage - 0.15) < 0.001;
        Print("[CharacterBackupNativeCheck] Bottle quantity=" + restoredBottle.GetQuantity().ToString() + " wet=" + restoredBottle.GetWet().ToString() + " health=" + restoredBottle.GetHealth01("", "Health").ToString());
        Print("[CharacterBackupNativeCheck] Mag=" + restoredMagazine.GetAmmoCount().ToString() + " internal=" + restoredRifle.GetInternalMagazineCartridgeCount(0).ToString() + " chamber=" + correctChamber.ToString() + " water=" + destination.GetStatWater().Get().ToString() + " energy=" + destination.GetStatEnergy().Get().ToString());
        bool power = restoredHeadtorch.GetCompEM().IsSwitchedOn() == expectedPower && restoredHeadtorch.GetCompEM().GetEnergySource() == restoredBattery && Math.AbsFloat(restoredBattery.GetCompEM().GetEnergy() - expectedBatteryEnergy) < 0.001;
        bool health = !destination.GetAllowDamage() && Math.AbsFloat(destination.GetHealth01("", "Health") - expectedHealth) < 0.001 && Math.AbsFloat(destination.GetHealth01("", "Blood") - expectedBlood) < 0.001 && Math.AbsFloat(destination.GetHealth01("", "Shock") - expectedShock) < 0.001 && Math.AbsFloat(destination.GetHealth01("RightLeg", "Health") - expectedLeg) < 0.001;
        Print("[CharacterBackupNativeCheck] Power=" + power.ToString() + " Health=" + health.ToString());
        Print("[CharacterBackupNativeCheck] Health source=" + expectedHealth.ToString() + " destination=" + destination.GetHealth01("", "Health").ToString() + " blood=" + destination.GetHealth01("", "Blood").ToString() + " shock=" + destination.GetHealth01("", "Shock").ToString() + " leg=" + destination.GetHealth01("RightLeg", "Health").ToString());
        bool connected = restoredHeadtorch.GetCompEM().GetEnergySource() == restoredBattery;
        Print("[CharacterBackupNativeCheck] Device source=" + expectedPower.ToString() + " destination=" + restoredHeadtorch.GetCompEM().IsSwitchedOn().ToString() + " connected=" + connected.ToString() + " battery=" + restoredBattery.GetCompEM().GetEnergy().ToString());
        return power && health && Math.AbsFloat(restoredBottle.GetQuantity() - 230) < 0.01 && Math.AbsFloat(restoredBottle.GetWet() - expectedWet) < 0.001 && Math.AbsFloat(restoredBottle.GetHealth01("", "Health") - 0.63) < 0.01 && restoredMagazine.GetAmmoCount() == 2 && restoredRifle.GetInternalMagazineCartridgeCount(0) == 2 && correctChamber && Math.AbsFloat(destination.GetStatWater().Get() - 1700) < 0.01 && Math.AbsFloat(destination.GetStatEnergy().Get() - 2100) < 0.01;
    }
}

/** \brief Schedules checks after native mission initialization. */
void main()
{
    GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ServerSentryBackupNativeCheck.Run, 1000, false);
}
