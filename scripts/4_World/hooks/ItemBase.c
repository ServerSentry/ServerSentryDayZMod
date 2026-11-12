/**
	\file ItemBase.c
	\brief Connects ServerSentry to DayZ or third-party hooks for Item base.
*/

/**
	\brief Extends the DayZ Item base class with ServerSentry integration.
*/
modded class ItemBase
{
    /** \brief Persistent explosive roles, allocated only for supported tracked devices. */
    protected ref ServerSentryExplosiveAttribution m_ServerSentryExplosiveState;
    /** \brief Immutable provenance of the current physical detonation. */
    protected ref ServerSentryExplosiveAttribution m_ServerSentryExplosionSnapshot;
    /** \brief Marks only the synchronous native fuse/remote activation scope. */
    protected bool m_ServerSentryKnownDetonation;

    /**
        \brief Returns device roles without allocating state for ordinary loot.
        \param create \p allocates roles when explicitly requested on the server.
        \return \p attribution state, or null.
    */
    ServerSentryExplosiveAttribution ServerSentryExplosiveState(bool create = false)
    {
        if (!m_ServerSentryExplosiveState && create && ServerSentryExplosiveResolver.IsServer() && ServerSentryExplosiveResolver.Supports(this))
        {
            m_ServerSentryExplosiveState = new ServerSentryExplosiveAttribution();
            m_ServerSentryExplosiveState.deviceId = ServerSentryExplosiveResolver.NewId();
        }
        return m_ServerSentryExplosiveState;
    }

    /**
        \brief Accepts transferred controller roles before a charge detaches.
        \param state \p independent attribution for this physical device.
    */
    void ServerSentrySetExplosiveState(ServerSentryExplosiveAttribution state)
    {
        if (!ServerSentryExplosiveResolver.IsServer())
            return;

        m_ServerSentryExplosiveState = state;
        ServerSentryResetExplosionSnapshot();
    }

    /** \brief Starts a new activation cycle without carrying over an old detonation. */
    void ServerSentryResetExplosionSnapshot()
    {
        m_ServerSentryExplosionSnapshot = null;
    }

    /**
        \brief Starts arming with a server-confirmed identity, retaining the placement role.
        \param identity \p arming actor; null means an unidentified new activation.
        \param activation \p fuse or trap activation path.
    */
    void ServerSentryArmExplosive(ServerSentryExplosiveIdentity identity, string activation)
    {
        ServerSentryExplosiveAttribution state = ServerSentryExplosiveState(true);
        if (!state)
            return;

        state.armedBy = identity;
        state.releasedBy = null;
        state.releaseObserved = false;
        state.detonatedBy = null;
        state.triggeredBy = null;
        state.parentDeviceId = "";
        state.parentDetonationId = "";
        state.activation = activation;
        ServerSentryResetExplosionSnapshot();
        m_ServerSentryKnownDetonation = false;
    }

    /**
        \brief Opens a synchronous fuse/remote scope and returns the previous scope state.
        \param active \p whether a verified native activation is executing.
        \return \p previous scope state for nested callers.
    */
    bool ServerSentrySetKnownDetonation(bool active)
    {
        bool previous = m_ServerSentryKnownDetonation;
        m_ServerSentryKnownDetonation = active;
        return previous;
    }

    /**
        \brief Freezes the physical source and all roles before explosion damage is dispatched.
        \return \p independent snapshot suitable for one hit or kill event.
    */
    ServerSentryExplosiveAttribution ServerSentryExplosionSnapshot()
    {
        if (!m_ServerSentryExplosionSnapshot)
        {
            ServerSentryExplosiveAttribution state = ServerSentryExplosiveState(true);
            if (!state)
                return null;

            m_ServerSentryExplosionSnapshot = state.Copy();
            m_ServerSentryExplosionSnapshot.detonationId = ServerSentryExplosiveResolver.NewId();
            m_ServerSentryExplosionSnapshot.sourceClassName = GetType();
            m_ServerSentryExplosionSnapshot.sourcePosition = new ServerSentryVectorDto();
            m_ServerSentryExplosionSnapshot.sourcePosition.FromVector(GetPosition());
        }
        return m_ServerSentryExplosionSnapshot.Copy();
    }

    /**
        \brief Uses the actual killing source before inherited code can detonate the device.
        \param killer \p native killing source outside an explicit fuse/remote activation.
    */
    override void EEKilled(Object killer)
    {
        if (ServerSentryExplosiveResolver.IsServer() && ServerSentryExplosiveResolver.Supports(this))
        {
            if (!m_ServerSentryKnownDetonation)
            {
                ServerSentryActor actor = new ServerSentryActor();
                ServerSentryExplosiveAttribution parent;
                EntityAI source = EntityAI.Cast(killer);
                if (source == this)
                    source = null;

                ServerSentryExplosiveResolver.FillActor(actor, source, parent);
                ServerSentryExplosiveAttribution state = ServerSentryExplosiveState(true);
                state.activation = "damage";
                state.detonatedBy = null;
                state.parentDetonationId = "";
                if (actor.type == "player")
                {
                    state.detonatedBy = new ServerSentryExplosiveIdentity();
                    ServerSentryEventFactory.CopyActor(actor, state.detonatedBy);
                    state.detonatedBy.action = "killing_source";
                    state.detonatedBy.occurredAtUtc = ServerSentryTime.NowUtc();
                }
                if (parent)
                    state.parentDetonationId = parent.detonationId;

                ServerSentryResetExplosionSnapshot();
            }
        }
        super.EEKilled(killer);
    }

    /**
        \brief Freezes attribution before native explosion damage reaches any victim.
        \param damageType \p engine damage category.
        \param ammoType \p explosion ammunition configuration.
    */
    override void Explode(int damageType, string ammoType = "")
    {
        if (ServerSentryExplosiveResolver.IsServer() && ServerSentryExplosiveResolver.Supports(this))
            ServerSentryExplosionSnapshot();

        super.Explode(damageType, ammoType);
    }

    /**
        \brief Saves only tracked devices in the ServerSentry CF storage namespace.
        \param storage \p independent mod storage streams.
    */
    override void CF_OnStoreSave(CF_ModStorageMap storage)
    {
        super.CF_OnStoreSave(storage);
        if (!m_ServerSentryExplosiveState)
            return;

        CF_ModStorage context = storage["ServerSentry"];
        if (context)
            ServerSentryExplosiveStorage.Save(context, m_ServerSentryExplosiveState);
    }

    /**
        \brief Loads optional provenance without changing vanilla or other mods' persistence.
        \param storage \p independent mod storage streams.
        \return \p inherited load result; invalid attribution is discarded without deleting the item.
    */
    override bool CF_OnStoreLoad(CF_ModStorageMap storage)
    {
        if (!super.CF_OnStoreLoad(storage))
            return false;

        m_ServerSentryExplosiveState = null;
        ServerSentryResetExplosionSnapshot();
        m_ServerSentryKnownDetonation = false;
        CF_ModStorage context = storage["ServerSentry"];
        if (!context)
            return true;

        ServerSentryExplosiveAttribution loaded;
        if (!ServerSentryExplosiveStorage.Load(context, loaded))
        {
            Print("[ServerSentry][Explosives] Invalid or unsupported attribution discarded for " + GetType());
            return true;
        }
        m_ServerSentryExplosiveState = loaded;
        return true;
    }

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
