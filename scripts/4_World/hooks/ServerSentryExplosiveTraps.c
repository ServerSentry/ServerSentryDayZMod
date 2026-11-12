/**
    \file ServerSentryExplosiveTraps.c
    \brief Tracks landmine and tripwire roles independently of ExplosivesBase.
*/

/** \brief Captures explicit trap placement and arming without inferring an owner at explosion time. */
modded class TrapBase
{
    /**
        \brief Captures the server placement actor for supported explosive traps.
        \param player \p placing player.
        \param position \p confirmed position.
        \param orientation \p confirmed orientation.
    */
    override void OnPlacementComplete(Man player, vector position = "0 0 0", vector orientation = "0 0 0")
    {
        if (ServerSentryExplosiveResolver.IsServer() && ServerSentryExplosiveResolver.Supports(this))
        {
            ServerSentryExplosiveAttribution state = ServerSentryExplosiveState(true);
            state.placedBy = ServerSentryExplosiveResolver.Capture(player, "trap_placement");
        }
        super.OnPlacementComplete(player, position, orientation);
    }

    /**
        \brief Retains the arming identity across the native activation timer.
        \param player \p explicit arming player, or null during persistence loading.
    */
    override void StartActivate(PlayerBase player)
    {
        if (ServerSentryExplosiveResolver.IsServer() && player && ServerSentryExplosiveResolver.Supports(this))
        {
            ServerSentryExplosiveIdentity actor = ServerSentryExplosiveResolver.Capture(player, "trap_arming");
            ServerSentryArmExplosive(actor, "trap");
        }
        super.StartActivate(player);
    }

    /**
        \brief Captures direct trap deployment paths used by tripwires.
        \param player \p deploying player supplied by the native server path.
        \param set_position \p inherited placement flag.
    */
    override void SetupTrapPlayer(PlayerBase player, bool set_position = true)
    {
        if (ServerSentryExplosiveResolver.IsServer() && player && ServerSentryExplosiveResolver.Supports(this))
        {
            ServerSentryExplosiveIdentity actor = ServerSentryExplosiveResolver.Capture(player, "trap_deployment");
            ServerSentryExplosiveAttribution state = ServerSentryExplosiveState(true);
            state.placedBy = actor;
            ServerSentryArmExplosive(actor, "trap");
        }
        super.SetupTrapPlayer(player, set_position);
    }
}

/** \brief Separates the landmine's triggering victim from the arming player. */
modded class LandMineTrap
{
    /**
        \brief Records the triggering player before native blast damage is dispatched.
        \param victim \p player, vehicle or item that triggered the mine.
    */
    override void OnSteppedOn(EntityAI victim)
    {
        if (ServerSentryExplosiveResolver.IsServer() && victim && victim.GetAllowDamage())
        {
            ServerSentryExplosiveAttribution state = ServerSentryExplosiveState(true);
            Man triggeringPlayer = Man.Cast(victim);
            state.triggeredBy = ServerSentryExplosiveResolver.Capture(triggeringPlayer, "stepped_on_mine");
        }
        super.OnSteppedOn(victim);
    }

    /**
        \brief Clears completed arming roles after successful native deactivation.
        \param stop_timer \p inherited activation timer flag.
    */
    override void SetInactive(bool stop_timer = true)
    {
        super.SetInactive(stop_timer);
        if (ServerSentryExplosiveResolver.IsServer())
            ServerSentryArmExplosive(null, "unknown");
    }
}

/** \brief Propagates tripwire roles before its grenade is activated and dropped. */
modded class TripwireTrap
{
    /**
        \brief Records a triggering victim without awarding that player the resulting kills.
        \param victim \p entity that crossed the wire.
    */
    override void OnSteppedOn(EntityAI victim)
    {
        if (ServerSentryExplosiveResolver.IsServer() && victim && victim.GetAllowDamage())
        {
            ServerSentryExplosiveAttribution state = ServerSentryExplosiveState(true);
            Man triggeringPlayer = Man.Cast(victim);
            state.triggeredBy = ServerSentryExplosiveResolver.Capture(triggeringPlayer, "crossed_tripwire");
        }
        super.OnSteppedOn(victim);
    }

    /**
        \brief Transfers context before native deactivation activates attached explosives.
        \param stop_timer \p inherited activation timer flag.
    */
    override void SetInactive(bool stop_timer = true)
    {
        ServerSentryExplosiveResolver.TransferCharges(this, "trap");
        super.SetInactive(stop_timer);
        if (ServerSentryExplosiveResolver.IsServer())
            ServerSentryArmExplosive(null, "unknown");
    }
}
