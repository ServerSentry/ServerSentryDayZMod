/**
    \file Grenade_Base.c
    \brief Records live grenade releases and successful pin actions.
*/

/** \brief Captures the player releasing a live grenade before inventory ownership disappears. */
modded class Grenade_Base
{
    /** \brief Identifies the native fuse expiry before it destroys the grenade. */
    override protected void OnActivateFinished()
    {
        bool previous = ServerSentrySetKnownDetonation(true);
        super.OnActivateFinished();
        ServerSentrySetKnownDetonation(previous);
    }

    /** \brief Identifies explicit native immediate activation before destruction. */
    override protected void OnActivateImmediate()
    {
        bool previous = ServerSentrySetKnownDetonation(true);
        super.OnActivateImmediate();
        ServerSentrySetKnownDetonation(previous);
    }

    /**
        \brief Copies a verified hands-to-ground release before the fuse starts.
        \param oldLoc \p previous native inventory location.
        \param newLoc \p confirmed new native inventory location.
    */
    override void EEItemLocationChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
    {
        if (ServerSentryExplosiveResolver.IsServer() && !IsPinned() && oldLoc.GetType() == InventoryLocationType.HANDS && newLoc.GetType() == InventoryLocationType.GROUND)
        {
            Man player = Man.Cast(oldLoc.GetParent());
            ServerSentryExplosiveAttribution state = ServerSentryExplosiveState(true);
            state.releasedBy = ServerSentryExplosiveResolver.Capture(player, "live_grenade_release");
            state.releaseObserved = true;
            state.detonatedBy = null;
            state.triggeredBy = null;
            state.activation = "fuse";
            ServerSentryResetExplosionSnapshot();
        }
        super.EEItemLocationChanged(oldLoc, newLoc);
    }
}

/** \brief Captures the action's authenticated actor instead of a later inventory holder. */
modded class ActionUnpin
{
    /**
        \brief Records only a successful server-side pin removal.
        \param action_data \p verified action and acting player.
    */
    override void OnExecute(ActionData action_data)
    {
        Grenade_Base grenade = Grenade_Base.Cast(action_data.m_MainItem);
        bool wasPinned = grenade && grenade.IsPinned();
        ServerSentryExplosiveIdentity actor;
        if (ServerSentryExplosiveResolver.IsServer() && wasPinned)
            actor = ServerSentryExplosiveResolver.Capture(action_data.m_Player, "grenade_unpin");

        super.OnExecute(action_data);

        if (ServerSentryExplosiveResolver.IsServer() && grenade && wasPinned && !grenade.IsPinned())
            grenade.ServerSentryArmExplosive(actor, "fuse");
    }
}

/** \brief Ends the previous grenade activation cycle after successful repinning. */
modded class ActionPin
{
    /**
        \brief Clears stale release and damage roles only when the pin was restored.
        \param action_data \p verified action and acting player.
    */
    override void OnExecute(ActionData action_data)
    {
        Grenade_Base grenade = Grenade_Base.Cast(action_data.m_MainItem);
        bool wasUnpinned = grenade && !grenade.IsPinned();
        super.OnExecute(action_data);

        if (ServerSentryExplosiveResolver.IsServer() && grenade && wasUnpinned && grenade.IsPinned())
            grenade.ServerSentryArmExplosive(null, "unknown");
    }
}
