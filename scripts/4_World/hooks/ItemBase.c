/**
	\file ItemBase.c
	\brief Connects ServerSentry to DayZ or third-party hooks for Item base.
*/

/**
	\brief Extends the DayZ Item base class with ServerSentry integration.
*/
modded class ItemBase
{
    /** \brief Mission-local item identity, allocated only when a backup needs one. */
    protected string m_ServerSentryBackupRuntimeKey;
    /** \brief Whether the backup index needs a deletion notification for this instance. */
    protected bool m_ServerSentryBackupTracked;

    /** \brief Enables deletion notification for an item referenced by a retained backup. */
    void ServerSentryMarkBackupTracked()
    {
        m_ServerSentryBackupTracked = true;
    }

    /**
        \brief Returns an existing identity or allocates one when explicitly requested.
        \param create \p true to create a mission-local key when absent.
        \return \p existing/generated key, or an empty string for an untracked item.
    */
    string ServerSentryBackupRuntimeKey(bool create)
    {
        if (create && m_ServerSentryBackupRuntimeKey == "")
            m_ServerSentryBackupRuntimeKey = ServerSentryBackupItemIndex.NewRuntimeKey();
        return m_ServerSentryBackupRuntimeKey;
    }

    /** \brief Reconnects restored native entities with the retained backup index. */
    override void EEOnAfterLoad()
    {
        super.EEOnAfterLoad();
        ServerSentryBackupItemIndex.Observe(this);
    }

    /**
        \brief Tracks player-relevant observations for this loaded instance only; never written to persistence.
    */
    protected bool m_ServerSentryTraceRelevant = false;
    /** \brief Prevents duplicate lifecycle registration for this runtime instance. */
    protected bool m_ServerSentryCounted = false;
    /** \brief Remembers the primitive ground state before native inventory transitions. */
    protected bool m_ServerSentryCountedOnGround = false;

    /**
    	\brief Processes the ee init operation.
    */
    override void EEInit()
    {
        super.EEInit();
        ServerSentryLiveApi.TryRegisterEntity(this);
        if (GetGame() && GetGame().IsServer() && !m_ServerSentryCounted)
        {
            m_ServerSentryCounted = true;
            m_ServerSentryCountedOnGround = GetHierarchyParent() == null;
            ServerSentryWorldItemCounts.Loaded++;
            if (m_ServerSentryCountedOnGround)
                ServerSentryWorldItemCounts.Ground++;
        }
    }

    /**
    	\brief Processes the ee delete operation.
    	\param parent \p parent used by the operation.
    */
    override void EEDelete(EntityAI parent)
    {
        if (m_ServerSentryBackupTracked)
            ServerSentryBackupItemIndex.Forget(this);
        // Capture before super can invalidate the native entity or its inventory.
        // Untouched world loot exits before identity reads, event allocation or serialization.
        if (m_ServerSentryTraceRelevant)
            ServerSentryItemLifecycle.ObserveDeletion(this, parent);
        if (m_ServerSentryCounted)
        {
            if (ServerSentryWorldItemCounts.Loaded > 0)
                ServerSentryWorldItemCounts.Loaded--;
            if (m_ServerSentryCountedOnGround && ServerSentryWorldItemCounts.Ground > 0)
                ServerSentryWorldItemCounts.Ground--;
            m_ServerSentryCounted = false;
            m_ServerSentryCountedOnGround = false;
        }
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
        // Update primitives before super can invalidate an entity during inventory moves.
        if (m_ServerSentryCounted)
        {
            bool onGround = newLoc.GetType() == InventoryLocationType.GROUND;
            if (onGround != m_ServerSentryCountedOnGround)
            {
                if (onGround)
                    ServerSentryWorldItemCounts.Ground++;
                else if (ServerSentryWorldItemCounts.Ground > 0)
                    ServerSentryWorldItemCounts.Ground--;
                m_ServerSentryCountedOnGround = onGround;
            }
        }
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
