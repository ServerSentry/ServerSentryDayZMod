/** One guarded boundary for capture and restore's native cartridge reads. */
class ServerSentryBackupMagazineReader
{
    static const int MAX_ROUNDS = 4096;
    protected static ref map<string, bool> s_AmmoClasses = new map<string, bool>;
    protected static ref map<string, bool> s_MagazineClasses = new map<string, bool>;

    static bool ValidCount(int count, int capacity)
    {
        return count >= 0 && count <= MAX_ROUNDS && capacity >= 0 && capacity <= 100000 && count <= capacity;
    }

    static bool ValidRound(float damage, string type)
    {
        if (!(damage >= 0 && damage <= 1) || type == "") return false;
        bool valid;
        if (s_AmmoClasses.Find(type, valid)) return valid;
        valid = GetGame().ConfigIsExisting("CfgAmmo " + type);
        if (s_AmmoClasses.Count() < 1024) s_AmmoClasses.Set(type, valid);
        return valid;
    }

    static bool Ready(EntityAI entity, out string reason)
    {
        reason = "entity unavailable or deleting";
        if (!entity || entity.IsSetForDeletion()) return false;
        reason = "entity is ruined";
        if (entity.IsRuined()) return false;
        EntityAI parent = entity.GetHierarchyParent();
        reason = "parent is deleting";
        if (parent && parent.IsSetForDeletion()) return false;
        PlayerBase owner = PlayerBase.Cast(entity.GetHierarchyRootPlayer());
        reason = "inventory or weapon action in progress";
        if (InventoryBusy(owner)) return false;
        Weapon_Base weapon = Weapon_Base.Cast(entity);
        if (!weapon) weapon = Weapon_Base.Cast(parent);
        if (weapon)
        {
            reason = "weapon has no running FSM";
            if (!weapon.CanProcessWeaponEvents()) return false;
            reason = "weapon is in an FSM transition";
            WeaponStateBase state = weapon.GetCurrentState();
            if (!WeaponStableState.Cast(state))
            {
                if (state) reason += " state=" + state.Type().ToString();
                return false;
            }
        }
        reason = "";
        return true;
    }

    static bool InventoryBusy(PlayerBase player)
    {
        return player && ((player.GetWeaponManager() && player.GetWeaponManager().IsRunning()) || (player.GetDayZPlayerInventory() && player.GetDayZPlayerInventory().IsProcessing()));
    }

    static void Log(EntityAI entity, string phase, string outcome, int count = -1, int capacity = -1)
    {
        string identity = "unavailable";
        if (entity)
        {
            identity = entity.GetType();
            // Do not traverse the hierarchy of an entity already pending deletion.
            if (!entity.IsSetForDeletion())
            {
                identity += " netId=" + entity.GetNetworkIDString();
                EntityAI parent = entity.GetHierarchyParent();
                if (parent) identity += " parent=" + parent.GetType();
                PlayerBase owner = PlayerBase.Cast(entity.GetHierarchyRootPlayer());
                if (owner && owner.GetIdentity()) identity += " owner=" + owner.GetIdentity().GetPlainId();
            }
        }
        Print("[ServerSentry][CharacterBackup] Ammo phase=" + phase + " item=" + identity + " count=" + count + " capacity=" + capacity + " " + outcome);
    }

    protected bool HasConfiguredAmmo(Magazine magazine)
    {
        string type = magazine.GetType();
        bool valid;
        if (s_MagazineClasses.Find(type, valid)) return valid;
        string ammo;
        GetGame().ConfigGetText("CfgMagazines " + type + " ammo", ammo);
        valid = ammo != "" && GetGame().ConfigIsExisting("CfgAmmo " + ammo);
        if (s_MagazineClasses.Count() < 1024) s_MagazineClasses.Set(type, valid);
        return valid;
    }

    /** Native boundary, reached only after validating the magazine and its current count. */
    protected bool ReadCartridge(Magazine magazine, int index, out float damage, out string type)
    {
        return magazine.GetCartridgeAtIndex(index, damage, type);
    }

    bool Read(Magazine magazine, array<ref ServerSentryBackupCartridge> rounds, bool verify = false, ServerSentryBackupReadBudget budget = null)
    {
        string phase = "capture";
        if (verify) phase = "restore-verify";
        string reason;
        if (!Ready(magazine, reason)) { Log(magazine, phase, "rejected: " + reason); return false; }
        int count = magazine.GetAmmoCount();
        int capacity = magazine.GetAmmoMax();
        if (!rounds || !ValidCount(count, capacity) || !HasConfiguredAmmo(magazine))
        {
            Log(magazine, phase, "rejected: invalid count, capacity or ammo configuration", count, capacity);
            return false;
        }
        if (verify && count != rounds.Count()) { Log(magazine, phase, "rejected: expected count=" + rounds.Count(), count, capacity); return false; }
        if (budget && !budget.Reserve(count)) { Log(magazine, phase, "rejected: cartridge/time budget exhausted", count, capacity); return false; }
        // A native crash cannot return false. This breadcrumb identifies the last magazine entered.
        Log(magazine, phase, "read begin", count, capacity);
        for (int index = 0; index < count; index++)
        {
            if (magazine.IsSetForDeletion() || magazine.GetAmmoCount() != count || (budget && Math.ModFloat(index, 32) == 0 && !budget.HasTime()))
            {
                Log(magazine, phase, "rejected before round=" + index + ": magazine changed, deleting or time budget", count, capacity);
                return false;
            }
            float damage;
            string type;
            if (!ReadCartridge(magazine, index, damage, type) || !ValidRound(damage, type))
            {
                Log(magazine, phase, "rejected at round=" + index + ": invalid cartridge", count, capacity);
                return false;
            }
            if (verify)
            {
                if (!rounds[index] || type != rounds[index].Type || Math.AbsFloat(damage - rounds[index].Damage) > 0.0001)
                {
                    Log(magazine, phase, "mismatch at round=" + index, count, capacity);
                    return false;
                }
            }
            else
            {
                ServerSentryBackupCartridge round = new ServerSentryBackupCartridge;
                round.Damage = damage;
                round.Type = type;
                rounds.Insert(round);
            }
        }
        if (magazine.IsSetForDeletion() || magazine.GetAmmoCount() != count)
        {
            Log(magazine, phase, "rejected: magazine changed during read", count, capacity);
            return false;
        }
        Log(magazine, phase, "read complete", count, capacity);
        return true;
    }
}
