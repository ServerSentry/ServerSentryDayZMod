/**
	\file AnimalBase.c
	\brief Connects ServerSentry to DayZ or third-party hooks for Animal base.
*/

/**
	\brief Extends the DayZ Animal base class with ServerSentry integration.
*/
modded class AnimalBase
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
        super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

        ServerSentryHitRequest request = new ServerSentryHitRequest(EntityAI.Cast(this), GetSSCombatState(), damageResult, damageType, source, component, dmgZone, ammo);
        request.CapturePostHitState();

        bool logHits = ServerSentryConfigManager.Get().IsAllowedToLogAnimalDamage();
        bool logKills = ServerSentryConfigManager.Get().IsAllowedToLogAnimalKills();
        if (!logHits && !logKills)
        {
            return;
        }

        request.ConfigureLogging(logHits, logKills);
        request.SetTargetTypeHint("animal");
        ServerSentryCombatLifecycle.LogHit(request);
    }
};
