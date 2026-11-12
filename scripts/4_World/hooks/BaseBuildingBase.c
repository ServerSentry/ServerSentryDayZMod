/**
    \file BaseBuildingBase.c
    \brief Logs only hits that ruin a base entity or one of its damage zones.
*/

/** \brief Correlates ruined health transitions with the current damage callback. */
modded class BaseBuildingBase
{
    /** \brief Whether an accepted hit is being observed on the server. */
    protected bool m_ServerSentryDamagePending;
    /** \brief Whether that hit changed a health zone to ruined. */
    protected bool m_ServerSentryHitRuined;

    /**
        \brief Starts a fresh observation before the engine applies accepted damage.
        \param damageResult \p engine-owned damage values for this callback.
        \param damageType \p engine damage category.
        \param source \p damage source, possibly without a player owner.
        \param component \p engine hit component index.
        \param dmgZone \p callback damage-zone name, possibly empty.
        \param ammo \p ammunition or damage configuration name.
        \param modelPos \p hit position in the target's model space.
        \param speedCoef \p engine speed coefficient, forwarded unchanged.
        \return \p unchanged inherited decision about applying damage.
    */
    override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        m_ServerSentryDamagePending = false;
        m_ServerSentryHitRuined = false;

        bool applyDamage = super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
        if (applyDamage && damageResult && ServerSentryBaseDamageFactory.IsEnabled() && GetAllowDamage())
            m_ServerSentryDamagePending = true;

        return applyDamage;
    }

    /**
        \brief Remembers actual ruined transitions before construction cleanup changes the zone.
        \param oldLevel \p previous health level; negative values indicate initialization.
        \param newLevel \p health level applied by the engine.
        \param zone \p affected zone, or empty for global health.
    */
    override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
    {
        if (m_ServerSentryDamagePending && oldLevel >= 0 && oldLevel != GameConstants.STATE_RUINED && newLevel == GameConstants.STATE_RUINED)
            m_ServerSentryHitRuined = true;

        super.EEHealthLevelChanged(oldLevel, newLevel, zone);
    }

    /**
        \brief Emits only the hit that caused a ruined transition and consumes its observation.
        \param damageResult \p engine-owned damage values for this callback.
        \param damageType \p engine damage category.
        \param source \p damage source, possibly without a player owner.
        \param component \p engine hit component index.
        \param dmgZone \p callback damage-zone name, possibly empty.
        \param ammo \p ammunition or damage configuration name.
        \param modelPos \p hit position in the target's model space.
        \param speedCoef \p engine speed coefficient, forwarded unchanged.
    */
    override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        bool shouldLog = m_ServerSentryDamagePending && m_ServerSentryHitRuined;
        m_ServerSentryDamagePending = false;
        m_ServerSentryHitRuined = false;

        ServerSentryBaseDamageEvent ssEvent;
        if (shouldLog)
        {
            ssEvent = ServerSentryBaseDamageFactory.CreateHitSnapshot(this, damageResult, damageType, source, component, dmgZone, ammo, modelPos);
            if (ssEvent)
                ssEvent.data.sourceHook = "BaseBuildingBase.EEHitBy";
        }

        super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

        if (ssEvent)
        {
            ServerSentryLogger logger = ServerSentryLogger.Get();
            logger.Emit(ssEvent);
        }
    }
}

/** \brief Correlates ruined health transitions with the current damage callback. */
modded class BuildingBase
{
    /** \brief Whether an accepted hit is being observed on the server. */
    protected bool m_ServerSentryDamagePending;
    /** \brief Whether that hit changed a health zone to ruined. */
    protected bool m_ServerSentryHitRuined;

    /**
        \brief Starts a fresh observation before the engine applies accepted damage.
        \param damageResult \p engine-owned damage values for this callback.
        \param damageType \p engine damage category.
        \param source \p damage source, possibly without a player owner.
        \param component \p engine hit component index.
        \param dmgZone \p callback damage-zone name, possibly empty.
        \param ammo \p ammunition or damage configuration name.
        \param modelPos \p hit position in the target's model space.
        \param speedCoef \p engine speed coefficient, forwarded unchanged.
        \return \p unchanged inherited decision about applying damage.
    */
    override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        m_ServerSentryDamagePending = false;
        m_ServerSentryHitRuined = false;

        bool applyDamage = super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
        if (applyDamage && damageResult && ServerSentryBaseDamageFactory.IsEnabled() && GetAllowDamage())
            m_ServerSentryDamagePending = true;

        return applyDamage;
    }

    /**
        \brief Remembers actual ruined transitions before construction cleanup changes the zone.
        \param oldLevel \p previous health level; negative values indicate initialization.
        \param newLevel \p health level applied by the engine.
        \param zone \p affected zone, or empty for global health.
    */
    override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
    {
        if (m_ServerSentryDamagePending && oldLevel >= 0 && oldLevel != GameConstants.STATE_RUINED && newLevel == GameConstants.STATE_RUINED)
            m_ServerSentryHitRuined = true;

        super.EEHealthLevelChanged(oldLevel, newLevel, zone);
    }

    /**
        \brief Emits only the hit that caused a ruined transition and consumes its observation.
        \param damageResult \p engine-owned damage values for this callback.
        \param damageType \p engine damage category.
        \param source \p damage source, possibly without a player owner.
        \param component \p engine hit component index.
        \param dmgZone \p callback damage-zone name, possibly empty.
        \param ammo \p ammunition or damage configuration name.
        \param modelPos \p hit position in the target's model space.
        \param speedCoef \p engine speed coefficient, forwarded unchanged.
    */
    override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        bool shouldLog = m_ServerSentryDamagePending && m_ServerSentryHitRuined;
        m_ServerSentryDamagePending = false;
        m_ServerSentryHitRuined = false;

        ServerSentryBaseDamageEvent ssEvent;
        if (shouldLog)
        {
            ssEvent = ServerSentryBaseDamageFactory.CreateHitSnapshot(this, damageResult, damageType, source, component, dmgZone, ammo, modelPos);
            if (ssEvent)
                ssEvent.data.sourceHook = "BuildingBase.EEHitBy";
        }

        super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

        if (ssEvent)
        {
            ServerSentryLogger logger = ServerSentryLogger.Get();
            logger.Emit(ssEvent);
        }
    }
}
