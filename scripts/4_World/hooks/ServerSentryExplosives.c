/**
    \file ServerSentryExplosives.c
    \brief Records arming, remote activation and IED charge transfer before native detonation.
*/

/** \brief Shares placement and activation-cycle tracking across vanilla explosives. */
modded class ExplosivesBase
{
    /**
        \brief Saves the placement actor before subclass placement can arm the device.
        \param player \p player supplied by the server placement action.
        \param position \p confirmed placement position.
        \param orientation \p confirmed placement orientation.
    */
    override void OnPlacementComplete(Man player, vector position = "0 0 0", vector orientation = "0 0 0")
    {
        if (ServerSentryExplosiveResolver.IsServer())
        {
            ServerSentryExplosiveAttribution state = ServerSentryExplosiveState(true);
            state.placedBy = ServerSentryExplosiveResolver.Capture(player, "explosive_placement");
        }
        super.OnPlacementComplete(player, position, orientation);
    }

    /** \brief Starts a fresh cycle when the native device becomes armed. */
    override void OnArmed()
    {
        if (ServerSentryExplosiveResolver.IsServer())
        {
            ServerSentryExplosiveAttribution state = ServerSentryExplosiveState(true);
            ServerSentryExplosiveIdentity actor;
            if (state.placedBy)
                actor = state.placedBy.Copy();
            if (actor)
            {
                actor.action = "explosive_arming";
                actor.occurredAtUtc = ServerSentryTime.NowUtc();
            }
            ServerSentryArmExplosive(actor, "unknown");
        }
        super.OnArmed();
    }

    /**
        \brief Clears the old activation claim before disarming can damage the device.
        \param pWithTool \p inherited disarm mode.
    */
    override void OnDisarmed(bool pWithTool)
    {
        if (ServerSentryExplosiveResolver.IsServer())
            ServerSentryArmExplosive(null, "unknown");

        super.OnDisarmed(pWithTool);
    }
}

/** \brief Holds an operator only during one verified server-side remote action. */
modded class RemoteDetonatorTrigger
{
    /** \brief Copied operator for the current action, never the arbitrary current carrier. */
    protected ref ServerSentryExplosiveIdentity m_ServerSentryTriggerOperator;

    /**
        \brief Opens or clears the synchronous server-action scope.
        \param actor \p authenticated acting player snapshot, or null after the action.
    */
    void ServerSentrySetTriggerOperator(ServerSentryExplosiveIdentity actor)
    {
        if (ServerSentryExplosiveResolver.IsServer())
            m_ServerSentryTriggerOperator = actor;
    }

    /**
        \brief Returns the explicit action operator, never an inferred inventory owner.
        \return \p scoped operator, or null for automatic or unsupported activation paths.
    */
    ServerSentryExplosiveIdentity ServerSentryTriggerOperator()
    {
        return m_ServerSentryTriggerOperator;
    }
}

/** \brief Supplies the explicit remote operator during native synchronous activation. */
modded class ActionTriggerRemotely
{
    /**
        \brief Captures the acting player before any controlled device can explode.
        \param action_data \p server-validated remote action.
    */
    override void OnExecuteServer(ActionData action_data)
    {
        RemoteDetonatorTrigger remote = RemoteDetonatorTrigger.Cast(action_data.m_MainItem);
        if (remote)
        {
            ServerSentryExplosiveIdentity actor = ServerSentryExplosiveResolver.Capture(action_data.m_Player, "remote_trigger");
            remote.ServerSentrySetTriggerOperator(actor);
        }
        super.OnExecuteServer(action_data);
        if (remote)
            remote.ServerSentrySetTriggerOperator(null);
    }
}

/** \brief Captures valid plastic-explosive activations without changing native damage decisions. */
modded class Plastic_Explosive
{
    /**
        \brief Records a valid remote operator before native activation explodes the charge.
        \param item \p activating remote or the charge itself when released from an IED.
    */
    override void OnActivatedByItem(notnull ItemBase item)
    {
        bool known = ServerSentryExplosiveResolver.RecordRemote(this, item);
        bool previous = false;
        if (known)
            previous = ServerSentrySetKnownDetonation(true);

        super.OnActivatedByItem(item);
        if (known)
            ServerSentrySetKnownDetonation(previous);
    }
}

/** \brief Captures the remote operator for claymore explosions. */
modded class ClaymoreMine
{
    /**
        \brief Records remote provenance before inherited activation.
        \param item \p activating paired device.
    */
    override void OnActivatedByItem(notnull ItemBase item)
    {
        bool known = ServerSentryExplosiveResolver.RecordRemote(this, item);
        bool previous = false;
        if (known)
            previous = ServerSentrySetKnownDetonation(true);

        super.OnActivatedByItem(item);
        if (known)
            ServerSentrySetKnownDetonation(previous);
    }
}

/** \brief Transfers IED attribution before its charges detach and activate later. */
modded class ImprovisedExplosive
{
    /**
        \brief Copies roles to all actual explosive attachments before native activation.
        \param item \p remote receiver, clock or other native activation item.
    */
    override void OnActivatedByItem(notnull ItemBase item)
    {
        if (ServerSentryExplosiveResolver.IsServer() && GetArmed())
        {
            ServerSentryExplosiveResolver.RecordRemote(this, item);
            string activation = "unknown";
            if (ClockBase.Cast(item))
                activation = "timer";

            ServerSentryExplosiveResolver.TransferCharges(this, activation);
        }
        super.OnActivatedByItem(item);
    }
}
