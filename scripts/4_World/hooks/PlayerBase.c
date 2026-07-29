/**
	\file PlayerBase.c
	\brief Connects ServerSentry to DayZ or third-party hooks for Player base.
*/

/**
	\brief Extends the DayZ Player base class with ServerSentry integration.
*/
modded class PlayerBase
{
    /**
    	\brief Instance state for combat state.
    */
    protected ref ServerSentryCombatState m_ServerSentryCombatState;

    /**
    	\brief Returns ss combat state.
    	\return \p The resulting combat state instance, or null when unavailable.
    */
    protected ServerSentryCombatState GetSSCombatState()
    {
        if (!m_ServerSentryCombatState)
        {
            m_ServerSentryCombatState = new ServerSentryCombatState();
        }

        return m_ServerSentryCombatState;
    }

    /**
    	\brief Processes the ee hit by operation.
    	\param damageResult \p damage result used by the operation.
    	\param damageType \p damage type used by the operation.
    	\param source \p source used by the operation.
    	\param component \p component used by the operation.
    	\param dmgZone \p dmg zone used by the operation.
    	\param ammo \p ammo used by the operation.
    	\param modelPos \p model pos used by the operation.
    	\param speedCoef \p speed coef used by the operation.
    */
    override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        // A lethal super.EEHitBy call can detach the network identity before the
        // telemetry event is created. Snapshot it first so a real player victim
        // is not intermittently emitted as human AI with placeholder IDs.
        string targetSteamId = "";
        string targetBohemiaId = "";
        string targetName = "";
        PlayerIdentity targetIdentity = GetIdentity();
        if (targetIdentity)
        {
            targetSteamId = targetIdentity.GetPlainId();
            targetBohemiaId = targetIdentity.GetId();
            targetName = targetIdentity.GetName();
        }

        super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

        ServerSentryHitRequest request = new ServerSentryHitRequest(EntityAI.Cast(this), GetSSCombatState(), damageResult, damageType, source, component, dmgZone, ammo);
        request.CapturePostHitState();

        bool logHits = ServerSentryConfigManager.Get().IsAllowedToLogPlayerHits();
        bool logKills = ServerSentryConfigManager.Get().IsAllowedToLogPlayerKills();
        if (!logHits && !logKills)
        {
            return;
        }

        request.ConfigureLogging(logHits, logKills);

        // No hard-coded "player" hint: PlayerBase is also used by human AI.
        // The classifier resolves identity-backed players as "player" and bots as "ai".
        request.SetTargetIdentity(targetSteamId, targetBohemiaId, targetName);
        ServerSentryCombatLifecycle.LogHit(request);
    }

    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);

        if (rpc_type == ServerSentryRPCs.VEHICLE_KICK)
        {
            Param1<bool> param;
            if (!ctx.Read(param))
                return;

            bool kick = param.param1;
            if (kick)
            {
                HumanCommandVehicle vehicleCommand = GetCommand_Vehicle();
                if (vehicleCommand)
                {
                    vehicleCommand.GetOutVehicle();
                }
            }
        }
    }
}
