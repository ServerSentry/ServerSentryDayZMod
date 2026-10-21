/**
	\file ServerSentryDefaults.c
	\brief Defines ServerSentry support for Defaults.
*/

/**
	\brief Provides ServerSentry behavior for Defaults.
*/
class ServerSentryDefaults
{
    /**
    	\brief Processes the root directory operation.
    	\return \p The resulting string value.
    */
    static string RootDirectory()
    {
        return "$profile:ServerSentry";
    }

    /**
    	\brief Processes the event directory operation.
    	\return \p The resulting string value.
    */
    static string EventDirectory()
    {
        return "$profile:ServerSentry/events";
    }

    /**
    	\brief Processes the schema version operation.
    	\return \p The resulting integer value or status code.
    */
    static int SchemaVersion()
    {
        return 5;
    }

    // Stable logical server/shard id for Postgres. Change this per server.
    /**
    	\brief Processes the server ID operation.
    	\return \p The resulting string value.
    */
    static string ServerId()
    {
        return "default";
    }

    // Prefix used to generate boot ids. Boot id is created at runtime so sequence
    // resets after restart remain uniquely sortable with serverId + bootId + sequence.
    /**
    	\brief Processes the boot ID prefix operation.
    	\return \p The resulting string value.
    */
    static string BootIdPrefix()
    {
        return "boot";
    }

    /**
    	\brief Flushes interval ms.
    	\return \p The resulting integer value or status code.
    */
    static int FlushIntervalMs()
    {
        return 1000;
    }

    /**
    	\brief Processes the max queue size operation.
    	\return \p The resulting integer value or status code.
    */
    static int MaxQueueSize()
    {
        return 10000;
    }

    /**
    	\brief Processes the dropped event warn every operation.
    	\return \p The resulting integer value or status code.
    */
    static int DroppedEventWarnEvery()
    {
        return 100;
    }

    // Prevent duplicate hit spam caused by several DayZ damage callbacks for the
    // same impact/frame. Keep this small so rapid legitimate hits still appear.
    /**
    	\brief Processes the hit dedup window ms operation.
    	\return \p The resulting integer value or status code.
    */
    static int HitDedupWindowMs()
    {
        return 250;
    }

    // Suppress pure no-op hit callbacks. Some environmental damage callbacks
    // can arrive as separate Health/Shock notifications with zero resolved damage.
    /**
    	\brief Processes the suppress zero damage hit events operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool SuppressZeroDamageHitEvents()
    {
        return true;
    }

    // Debug switch for inventory move diagnostics. Keep false in production.
    // The previous debug build printed hundreds of lines per fast-loot move and can
    // destabilize the server under heavy inventory spam.
    /**
    	\brief Processes the debug inventory move events operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool DebugInventoryMoveEvents()
    {
        return false;
    }

    // ATTACHMENT locations such as Shoulder/Back/Melee are not external storage,
    // but they are valid player inventory destinations/sources when the parent
    // or parent root is a PlayerBase. Keep false so storage -> shoulder and
    // shoulder -> storage are logged as storage_take/storage_put.
    /**
    	\brief Processes the skip attachment item move events operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool SkipAttachmentItemMoveEvents()
    {
        return false;
    }

    // Item-move diagnostic mode. Keep false in production.
    // When true, ItemBase writes step-by-step probe lines to script.log.
    /**
    	\brief Processes the script log only item move events operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool ScriptLogOnlyItemMoveEvents()
    {
        return false;
    }

    // Stepwise
    // 0 = before/after super only. This is the known-stable baseline.
    // 1 = item basics: type, id, displayName, position.
    // 2 = old/new location IsValid + GetType only.
    // 3 = old/new parent entity basics.
    // 4 = type-specific InventoryLocation fields: slot/cargo/ground pos.
    // 5 = player/root/identity lookup.
    // 6 = lifecycle checks only: same/tracked/player/duplicate.
    // 7 = primitive snapshot + DTO build without using original InventoryLocation factory.
    // 8 = serialize snapshot-built item move event to JSON, but do not enqueue.
    // 9 = emit/enqueue snapshot-built item move event. Use only after 0-8 are stable.
    /**
    	\brief Processes the item move script log step operation.
    	\return \p The resulting integer value or status code.
    */
    static int ItemMoveScriptLogStep()
    {
        return 0;
    }

    // Production item-move logging uses the primitive-snapshot path.
    // It does not pass live InventoryLocation references into the old factory.
    /**
    	\brief Logs item move events.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool LogItemMoveEvents()
    {
        return true;
    }

    // Keep this true by default. World/CE/container-only movements can be very noisy
    // and are usually not useful for admin player stats.
    /**
    	\brief Logs only player item move events.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool LogOnlyPlayerItemMoveEvents()
    {
        return true;
    }

    // Inventory moves can generate duplicate callbacks across generic item, magazine,
    // weapon and attachment paths. Keep a small dedupe window per move signature.
    /**
    	\brief Processes the item move dedup window ms operation.
    	\return \p The resulting integer value or status code.
    */
    static int ItemMoveDedupWindowMs()
    {
        return 300;
    }

    // Weapon-internal chamber changes are not normal item location changes. Keep them
    // out of inventory-move stats unless a dedicated weapon-state logger is added.
    /**
    	\brief Logs weapon internal ammo state events.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool LogWeaponInternalAmmoStateEvents()
    {
        return false;
    }


    /**
    	\brief Logs base building action events.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool LogBaseBuildingActionEvents()
    {
        return true;
    }

    /**
    	\brief Logs placement action events.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool LogPlacementActionEvents()
    {
        return true;
    }

    /**
    	\brief Logs expansion safe zone events.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool LogExpansionSafeZoneEvents()
    {
        return true;
    }

    /**
    	\brief Logs dr jones safe zone events.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool LogDrJonesSafeZoneEvents()
    {
        return true;
    }

    /**
    	\brief Builds action dedup window ms.
    	\return \p The resulting integer value or status code.
    */
    static int BuildActionDedupWindowMs()
    {
        return 500;
    }

    /**
    	\brief Logs vehicle events.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool LogVehicleEvents()
    {
        return true;
    }

    /**
    	\brief Logs chat events.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool LogChatEvents()
    {
        return true;
    }

    /**
    	\brief Logs expansion chat events.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool LogExpansionChatEvents()
    {
        return true;
    }

    /**
    	\brief Logs LB master chat events.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool LogLBMasterChatEvents()
    {
        return true;
    }

    /**
    	\brief Logs LB master group events.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool LogLBMasterGroupEvents()
    {
        return true;
    }

    /**
    	\brief Logs code lock events.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool LogCodeLockEvents()
    {
        return true;
    }

    /**
    	\brief Logs breaching charge events.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool LogBreachingChargeEvents()
    {
        return true;
    }


    /**
    	\brief Logs lbatm events.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool LogLBATMEvents()
    {
        return true;
    }

    /**
    	\brief Processes the suppress vanilla chat when external provider active operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool SuppressVanillaChatWhenExternalProviderActive()
    {
        return true;
    }

    // Chat mods can forward the same player message through vanilla OnEvent and
    // their own channel-aware hook. Dedup on sender/channel/message so the more
    // specific bridge event wins when it fires first.
    /**
    	\brief Processes the chat dedup window ms operation.
    	\return \p The resulting integer value or status code.
    */
    static int ChatDedupWindowMs()
    {
        return 1500;
    }


    // Live map snapshot export. This is intentionally separate from JSONL telemetry
    // so high-frequency player/vehicle positions do not flood the event pipeline.
    /**
    	\brief Processes the live data enabled operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool LiveDataEnabled()
    {
        return true;
    }

    /**
    	\brief Processes the live snapshot interval ms operation.
    	\return \p The resulting integer value or status code.
    */
    static int LiveSnapshotIntervalMs()
    {
        return 1000;
    }

    /**
    	\brief Processes the live schema version operation.
    	\return \p The resulting integer value or status code.
    */
    static int LiveSchemaVersion()
    {
        return 4;
    }

    /**
    	\brief Processes the live directory operation.
    	\return \p The resulting string value.
    */
    static string LiveDirectory()
    {
        return "$profile:ServerSentry/live";
    }

    /**
    	\brief Processes the live state file path operation.
    	\return \p The resulting string value.
    */
    static string LiveStateFilePath()
    {
        return "$profile:ServerSentry/live/state.json";
    }

};
