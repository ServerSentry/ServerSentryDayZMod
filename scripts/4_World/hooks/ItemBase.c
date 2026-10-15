/**
	\file ItemBase.c
	\brief Connects ServerSentry to DayZ or third-party hooks for Item base.
*/

/**
	\brief Extends the DayZ Item base class with ServerSentry integration.
*/
modded class ItemBase
{
    /**
        \brief Tracks player-relevant observations for this loaded instance only; never written to persistence.
    */
    protected bool m_ServerSentryTraceRelevant = false;

    /**
    	\brief Processes the ee init operation.
    */
    override void EEInit()
    {
        super.EEInit();
        ServerSentryLiveApi.TryRegisterEntity(this);
    }

    /**
    	\brief Processes the ee delete operation.
    	\param parent \p parent used by the operation.
    */
    override void EEDelete(EntityAI parent)
    {
        // Capture before super can invalidate the native entity or its inventory.
        // Untouched world loot exits before identity reads, event allocation or serialization.
        if (m_ServerSentryTraceRelevant)
            ServerSentryItemLifecycle.ObserveDeletion(this, parent);
        ServerSentryLiveApi.UnregisterEntity(this);
        super.EEDelete(parent);
    }

    /**
    	\brief Processes the ee item location changed operation.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    */
    override void EEItemLocationChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
    {
        int ssMoveId = ServerSentryItemMoveScriptLog.NextMoveId();
        ServerSentryItemMoveSnapshot snapshot = null;

        if (ServerSentryDefaults.LogItemMoveEvents() && ServerSentryConfigManager.Get().IsAllowedToStartTelemetryLogging() && ServerSentryConfigManager.Get().IsAllowedToLogItemMovement())
        {
            // Crash fix: capture only primitive data before super. Some inventory
            // transitions can invalidate the native item after super, especially
            // in hand/animation driven moves from other mods.
            snapshot = ServerSentryItemMoveScriptLog.CaptureSnapshot(ssMoveId, this, oldLoc, newLoc);
        }

        // Mark before super: a native transition can invalidate the entity afterwards.
        // Rejected/deduplicated moves and AI/world activity never start tracking.
        if (snapshot && snapshot.allowed && snapshot.actorType == "player" && snapshot.actorSteamId != "")
            m_ServerSentryTraceRelevant = true;

        super.EEItemLocationChanged(oldLoc, newLoc);

        if (snapshot)
        {
            ServerSentryItemMoveScriptLog.EmitSnapshot(snapshot);
        }
    }
};
