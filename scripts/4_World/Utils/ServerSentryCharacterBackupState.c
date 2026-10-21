/**
    \file ServerSentryCharacterBackupState.c
    \brief Captures engine-owned state omitted by the scripted persistence callbacks.
*/

/** \brief Stores one cartridge without losing mixed ammunition or cartridge damage. */
class ServerSentryBackupCartridge
{
    /** \brief Cartridge damage. */
    float Damage;
    /** \brief CfgAmmo class name. */
    string Type;
}

/** \brief Stores ammunition for one weapon muzzle. */
class ServerSentryBackupMuzzle
{
    /** \brief Chamber cartridge, including fired cases. */
    ref ServerSentryBackupCartridge Chamber;
    /** \brief Whether the chamber contains a fired case. */
    bool Fired;
    /** \brief Internal magazine cartridges in native index order. */
    ref array<ref ServerSentryBackupCartridge> Rounds = new array<ref ServerSentryBackupCartridge>();
}

/** \brief Stores health pools for one damage zone. */
class ServerSentryBackupDamage
{
    /** \brief Empty for global health. */
    string Zone;
    /** \brief Normalized health, blood and shock; -1 denotes an absent pool. */
    ref array<float> Pools = new array<float>();

    /**
        \brief Checks configuration before querying a native damage pool that may not exist.
        \param entity \p entity whose damage system is inspected.
        \param zone \p empty for global health, otherwise the zone name.
        \param type \p health pool name.
        \return \p true when the configured pool exists.
    */
    protected bool HasPool(EntityAI entity, string zone, string type)
    {
        // Fresh inventory entities can report HasDamageSystem() == false until
        // their first native health access. Inspect inherited config instead.
        string section = "CfgVehicles ";
        if (entity.IsWeapon())
            section = "CfgWeapons ";
        else if (entity.IsMagazine())
            section = "CfgMagazines ";
        string path = section + entity.GetType() + " DamageSystem ";
        if (zone == "")
            path += "GlobalHealth ";
        else
            path += "DamageZones " + zone + " ";
        return GetGame().ConfigIsExisting(path + type + " hitpoints");
    }

    /**
        \brief Captures the available damage pools.
        \param entity \p entity being saved.
        \param zone \p damage zone name.
    */
    void Capture(EntityAI entity, string zone)
    {
        Zone = zone;
        array<string> types = {"Health", "Blood", "Shock"};
        foreach (string type : types)
        {
            float value = -1;
            if (HasPool(entity, zone, type) && entity.GetMaxHealth(zone, type) > 0)
                value = entity.GetHealth01(zone, type);
            Pools.Insert(value);
        }
    }

    /**
        \brief Restores the available damage pools.
        \param entity \p entity being restored.
        \return \p true when all values are valid for this entity.
    */
    bool Apply(EntityAI entity)
    {
        if (!Pools || Pools.Count() != 3)
            return false;
        array<string> types = {"Health", "Blood", "Shock"};
        for (int i = 0; i < 3; i++)
        {
            float value = Pools[i];
            if (value == -1)
                continue;
            if (!(value >= 0 && value <= 1) || !HasPool(entity, Zone, types[i]) || entity.GetMaxHealth(Zone, types[i]) <= 0)
                return false;
            entity.SetHealth01(Zone, types[i], value);
        }
        return true;
    }
}

/** \brief Stores an entity's placement, native damage and ammunition alongside OnStoreSave data. */
class ServerSentryBackupEntityState
{
    /** \brief Runtime class required to recreate the entity. */
    string Type;
    /** \brief Parent's preorder index; -1 denotes the player. */
    int Parent = -1;
    /** \brief InventoryLocationType value. */
    int Location;
    /** \brief Attachment slot identifier. */
    int AttachmentSlot;
    /** \brief Cargo index. */
    int CargoIndex;
    /** \brief Cargo row. */
    int Row;
    /** \brief Cargo column. */
    int Column;
    /** \brief Cargo rotation. */
    bool Flip;
    /** \brief Quickbar shortcut, or -1. */
    int Quickbar = -1;
    /** \brief Saved damage permission, including character god mode. */
    bool AllowDamage = true;
    /** \brief Whether an energy manager was present. */
    bool HasEnergy;
    /** \brief Stored device energy. */
    float Energy;
    /** \brief Saved power switch state. */
    bool EnergyOn;
    /** \brief Saved passive energy-manager state. */
    bool EnergyPassive;
    /** \brief Energy source index inside the captured inventory, or -1. */
    int EnergySource = -1;
    /** \brief Global and local damage pools. */
    ref array<ref ServerSentryBackupDamage> Damage = new array<ref ServerSentryBackupDamage>();
    /** \brief Magazine cartridges in native index order. */
    ref array<ref ServerSentryBackupCartridge> Rounds = new array<ref ServerSentryBackupCartridge>();
    /** \brief Weapon chambers and internal magazines. */
    ref array<ref ServerSentryBackupMuzzle> Muzzles = new array<ref ServerSentryBackupMuzzle>();

    /**
        \brief Checks a cartridge before passing it to native ammunition APIs.
        \param round \p saved cartridge.
        \return \p true for finite damage and an existing ammunition class.
    */
    protected bool ValidCartridge(ServerSentryBackupCartridge round)
    {
        return round && round.Damage >= 0 && round.Damage <= 1 && round.Type != "" && GetGame().ConfigIsExisting("CfgAmmo " + round.Type);
    }

    /**
        \brief Validates native metadata before any restore operation can dereference it.
        \return \p true when the metadata is well formed.
    */
    bool IsValid()
    {
        if (HasEnergy && (!(Energy >= 0 && Energy < 1.0e30) || EnergySource < -1))
            return false;
        if (Type == "" || !Damage || Damage.Count() < 1 || Damage.Count() > 256 || !Rounds || Rounds.Count() > 100000 || !Muzzles || Muzzles.Count() > 32)
            return false;
        foreach (ServerSentryBackupDamage damage : Damage)
        {
            if (!damage || !damage.Pools || damage.Pools.Count() != 3)
                return false;
            foreach (float value : damage.Pools)
            {
                if (value != -1 && !(value >= 0 && value <= 1))
                    return false;
            }
        }
        foreach (ServerSentryBackupCartridge round : Rounds)
        {
            if (!ValidCartridge(round))
                return false;
        }
        foreach (ServerSentryBackupMuzzle muzzle : Muzzles)
        {
            if (!muzzle || !muzzle.Rounds || muzzle.Rounds.Count() > 100000)
                return false;
            if (muzzle.Chamber && !ValidCartridge(muzzle.Chamber))
                return false;
            if (muzzle.Fired && muzzle.Chamber)
                return false;
            foreach (ServerSentryBackupCartridge internalRound : muzzle.Rounds)
            {
                if (!ValidCartridge(internalRound))
                    return false;
            }
        }
        return true;
    }

    /**
        \brief Captures placement and native state without changing the entity.
        \param entity \p entity being saved.
        \param player \p inventory owner.
        \param entities \p preorder list containing the player at index zero.
        \return \p true when the full state can be captured.
    */
    bool Capture(EntityAI entity, PlayerBase player, array<EntityAI> entities)
    {
        Type = entity.GetType();
        AllowDamage = entity.GetAllowDamage();
        ComponentEnergyManager energyManager = entity.GetCompEM();
        if (energyManager)
        {
            HasEnergy = true;
            Energy = energyManager.GetEnergy();
            EnergyOn = energyManager.IsSwitchedOn();
            EnergyPassive = energyManager.IsPassive();
            EnergySource = entities.Find(energyManager.GetEnergySource());
        }
        if (entity != player)
        {
            InventoryLocation location = new InventoryLocation();
            if (!entity.GetInventory().GetCurrentInventoryLocation(location))
                return false;
            Parent = entities.Find(location.GetParent());
            if (Parent < 0 || Parent >= entities.Find(entity))
                return false;
            Location = location.GetType();
            AttachmentSlot = location.GetSlot();
            CargoIndex = location.GetIdx();
            Row = location.GetRow();
            Column = location.GetCol();
            Flip = location.GetFlip();
            Quickbar = player.FindQuickBarEntityIndex(entity);
            if (Location != InventoryLocationType.ATTACHMENT && Location != InventoryLocationType.HANDS && Location != InventoryLocationType.CARGO)
                return false;
        }

        array<string> zones = new array<string>();
        entity.GetDamageZones(zones);
        zones.InsertAt("", 0);
        foreach (string zone : zones)
        {
            ServerSentryBackupDamage damage = new ServerSentryBackupDamage();
            damage.Capture(entity, zone);
            Damage.Insert(damage);
        }

        Magazine magazine = Magazine.Cast(entity);
        if (magazine)
        {
            for (int roundIndex = 0; roundIndex < magazine.GetAmmoCount(); roundIndex++)
            {
                ServerSentryBackupCartridge round = new ServerSentryBackupCartridge();
                if (!magazine.GetCartridgeAtIndex(roundIndex, round.Damage, round.Type))
                    return false;
                Rounds.Insert(round);
            }
        }

        Weapon_Base weapon = Weapon_Base.Cast(entity);
        if (weapon)
        {
            for (int muzzleIndex = 0; muzzleIndex < weapon.GetMuzzleCount(); muzzleIndex++)
            {
                ServerSentryBackupMuzzle muzzle = new ServerSentryBackupMuzzle();
                muzzle.Fired = weapon.IsChamberFiredOut(muzzleIndex);
                if (!weapon.IsChamberEmpty(muzzleIndex) && !muzzle.Fired)
                {
                    muzzle.Chamber = new ServerSentryBackupCartridge();
                    if (!weapon.GetCartridgeInfo(muzzleIndex, muzzle.Chamber.Damage, muzzle.Chamber.Type))
                        return false;
                }
                for (int internalIndex = 0; internalIndex < weapon.GetInternalMagazineCartridgeCount(muzzleIndex); internalIndex++)
                {
                    ServerSentryBackupCartridge internalRound = new ServerSentryBackupCartridge();
                    if (!weapon.GetInternalMagazineCartridgeInfo(muzzleIndex, internalIndex, internalRound.Damage, internalRound.Type))
                        return false;
                    muzzle.Rounds.Insert(internalRound);
                }
                Muzzles.Insert(muzzle);
            }
        }
        return true;
    }

    /**
        \brief Creates an exact destination for a new or existing inventory entity.
        \param parent \p destination parent.
        \param item \p existing entity, or null for creation.
        \return \p destination, or null for an unsupported location.
    */
    InventoryLocation Destination(EntityAI parent, EntityAI item)
    {
        InventoryLocation destination = new InventoryLocation();
        switch (Location)
        {
            case InventoryLocationType.ATTACHMENT:
                destination.SetAttachment(parent, item, AttachmentSlot);
                break;
            case InventoryLocationType.HANDS:
                destination.SetHands(parent, item);
                break;
            case InventoryLocationType.CARGO:
                destination.SetCargo(parent, item, CargoIndex, Row, Column, Flip);
                break;
            default:
                return null;
        }
        return destination;
    }

    /**
        \brief Reconnects cloned devices to cloned energy sources after all children exist.
        \param entity \p restored device.
        \param entities \p restored preorder tree, with the live owner at index zero.
        \return \p true when energy state and internal connections were restored.
    */
    bool ApplyEnergy(EntityAI entity, array<EntityAI> entities)
    {
        if (!HasEnergy)
            return true;
        ComponentEnergyManager energyManager = entity.GetCompEM();
        if (!energyManager || EnergySource >= entities.Count())
            return false;
        energyManager.SetEnergy(Energy);
        energyManager.SetPassiveState(EnergyPassive);
        // Saved persistent IDs refer to the originals, never to this cloned tree.
        energyManager.RestorePlugState(false);
        if (EnergySource >= 0)
        {
            EntityAI source = entities[EnergySource];
            if (!source || source == entity || !source.GetCompEM())
                return false;
            if (energyManager.GetEnergySource() != source)
            {
                energyManager.UnplugThis();
                if (!energyManager.PlugThisInto(source))
                    return false;
            }
        }
        else if (energyManager.IsPlugged())
        {
            energyManager.UnplugThis();
        }
        if (EnergyOn)
            energyManager.SwitchOn();
        else
            energyManager.SwitchOff();
        entity.SetSynchDirty();
        return energyManager.IsSwitchedOn() == EnergyOn && Math.AbsFloat(energyManager.GetEnergy() - Energy) < 0.001;
    }

    /**
        \brief Checks magazine contents including native index order.
        \param magazine \p recreated magazine.
        \return \p true when every cartridge matches.
    */
    protected bool VerifyMagazine(Magazine magazine)
    {
        if (magazine.GetAmmoCount() != Rounds.Count())
            return false;
        for (int i = 0; i < Rounds.Count(); i++)
        {
            float damage;
            string type;
            if (!magazine.GetCartridgeAtIndex(i, damage, type) || type != Rounds[i].Type || Math.AbsFloat(damage - Rounds[i].Damage) > 0.0001)
                return false;
        }
        return true;
    }

    /**
        \brief Restores native magazine contents and checks their order.
        \param magazine \p recreated magazine.
        \return \p true when restoration succeeds.
    */
    protected bool ApplyMagazine(Magazine magazine)
    {
        if (Rounds.Count() > magazine.GetAmmoMax())
            return false;
        magazine.ServerSetAmmoCount(Rounds.Count());
        if (VerifyMagazine(magazine))
            return true;
        // Native magazine types differ in their push/index ordering.
        for (int direction = 0; direction < 2; direction++)
        {
            magazine.ServerSetAmmoCount(0);
            for (int i = 0; i < Rounds.Count(); i++)
            {
                int index = i;
                if (direction == 1)
                    index = Rounds.Count() - 1 - i;
                if (!magazine.ServerStoreCartridge(Rounds[index].Damage, Rounds[index].Type))
                    return false;
            }
            if (VerifyMagazine(magazine))
                return true;
        }
        return false;
    }

    /**
        \brief Restores the native weapon chambers and internal magazines.
        \param weapon \p recreated weapon.
        \return \p true when all native ammunition operations succeed.
    */
    protected bool ApplyWeapon(Weapon_Base weapon)
    {
        if (Muzzles.Count() != weapon.GetMuzzleCount())
            return false;
        for (int i = 0; i < Muzzles.Count(); i++)
        {
            ServerSentryBackupMuzzle muzzle = Muzzles[i];
            if (!muzzle || !muzzle.Rounds || muzzle.Rounds.Count() > weapon.GetInternalMagazineMaxCartridgeCount(i))
                return false;
            float damage;
            string type;
            if (!weapon.IsChamberEmpty(i) && !weapon.PopCartridgeFromChamber(i, damage, type))
                return false;
            int count = weapon.GetInternalMagazineCartridgeCount(i);
            for (int removeIndex = 0; removeIndex < count; removeIndex++)
            {
                if (!weapon.PopCartridgeFromInternalMagazine(i, damage, type))
                    return false;
            }
            bool matched = false;
            // Native push/index ordering differs between magazine implementations.
            for (int direction = 0; direction < 2 && !matched; direction++)
            {
                if (direction == 1)
                {
                    for (int clearIndex = 0; clearIndex < muzzle.Rounds.Count(); clearIndex++)
                    {
                        if (!weapon.PopCartridgeFromInternalMagazine(i, damage, type))
                            return false;
                    }
                }
                for (int roundIndex = 0; roundIndex < muzzle.Rounds.Count(); roundIndex++)
                {
                    int sourceIndex = roundIndex;
                    if (direction == 1)
                        sourceIndex = muzzle.Rounds.Count() - 1 - roundIndex;
                    ServerSentryBackupCartridge round = muzzle.Rounds[sourceIndex];
                    if (!round || !weapon.PushCartridgeToInternalMagazine(i, round.Damage, round.Type))
                        return false;
                }
                matched = true;
                for (int verifyIndex = 0; verifyIndex < muzzle.Rounds.Count(); verifyIndex++)
                {
                    ServerSentryBackupCartridge expected = muzzle.Rounds[verifyIndex];
                    if (!weapon.GetInternalMagazineCartridgeInfo(i, verifyIndex, damage, type) || type != expected.Type || Math.AbsFloat(damage - expected.Damage) > 0.0001)
                        matched = false;
                }
            }
            if (!matched)
                return false;
            if (muzzle.Chamber)
            {
                if (!weapon.PushCartridgeToChamber(i, muzzle.Chamber.Damage, muzzle.Chamber.Type))
                    return false;
                if (!weapon.GetCartridgeInfo(i, damage, type) || type != muzzle.Chamber.Type || Math.AbsFloat(damage - muzzle.Chamber.Damage) > 0.0001)
                    return false;
            }
        }
        // There is no safe public native setter for spent cases. Their chambers
        // remain empty; never turn a spent case into a live round or call Fire().
        weapon.RandomizeFSMState();
        weapon.Synchronize();
        return true;
    }

    /**
        \brief Applies native state after scripted state and children have loaded.
        \param entity \p recreated entity.
        \return \p true when the entity exactly supports the saved native state.
    */
    bool Apply(EntityAI entity)
    {
        if (!entity || Type != entity.GetType() || !IsValid())
            return false;
        Magazine magazine = Magazine.Cast(entity);
        Weapon_Base weapon = Weapon_Base.Cast(entity);
        if (magazine && !ApplyMagazine(magazine))
            return false;
        if (weapon && !ApplyWeapon(weapon))
            return false;
        bool previousDamagePermission = entity.GetAllowDamage();
        entity.SetAllowDamage(true);
        // Zone setters can transfer damage into global pools. Apply global pools
        // last so the saved global totals remain authoritative.
        for (int damageIndex = Damage.Count() - 1; damageIndex >= 0; damageIndex--)
        {
            ServerSentryBackupDamage damage = Damage[damageIndex];
            if (!damage || !damage.Apply(entity))
            {
                entity.SetAllowDamage(previousDamagePermission);
                return false;
            }
        }
        entity.SetAllowDamage(AllowDamage);
        entity.SetSynchDirty();
        return true;
    }
}

/** \brief Identifies a versioned character snapshot and stores non-persistent player values. */
class ServerSentryCharacterBackupHeader
{
    /** \brief Snapshot schema version. */
    int Version = 2;
    /** \brief DayZ persistence version used for the script streams. */
    int SaveVersion;
    /** \brief World required for restoring the saved coordinates. */
    string WorldName;
    /** \brief Steam identity owning this snapshot. */
    string SteamId;
    /** \brief UTC capture time. */
    string CreatedUtc;
    /** \brief Player position. */
    vector Position;
    /** \brief Player orientation. */
    vector Orientation;
    /** \brief Current stamina. */
    float Stamina;
    /** \brief Native restraint state. */
    bool Restrained;
    /** \brief Native unconsciousness state. */
    bool Unconscious;
    /** \brief Native analytics counters. */
    ref array<float> Statistics = new array<float>();
    /** \brief Entity count including the player. */
    int Count;
}
