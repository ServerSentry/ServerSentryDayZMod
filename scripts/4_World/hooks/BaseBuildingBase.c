/**
    \file BaseBuildingBase.c
    \brief Observes damage callbacks on base-building entities.
*/

/** \brief Adds base-damage telemetry without changing damage handling. */
modded class BaseBuildingBase
{
    /**
        \brief Captures one damage observation and preserves the inherited callback.
        \param damageResult \p engine-owned damage values for this callback.
        \param damageType \p engine damage category.
        \param source \p damage source, which may be null or have no player owner.
        \param component \p hit component index supplied by the engine.
        \param dmgZone \p original callback damage-zone name, possibly empty.
        \param ammo \p ammunition or damage configuration name.
        \param modelPos \p callback position in the target's model space.
        \param speedCoef \p engine speed coefficient, forwarded unchanged.
    */
    override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        // Copy values while the callback data and source identity are available.
        // Before super is NOT a pre-damage health snapshot.
        ServerSentryBaseDamageEvent ssEvent = ServerSentryBaseDamageFactory.CreateHitSnapshot(this, damageResult, damageType, source, component, dmgZone, ammo, modelPos);

        super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

        if (ssEvent)
        {
            ServerSentryLogger logger = ServerSentryLogger.Get();
            logger.Emit(ssEvent);
        }
    }
}
