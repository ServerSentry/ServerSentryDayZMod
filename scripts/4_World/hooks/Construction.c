/**
    \file Construction.c
    \brief Observes completed ruined-health transitions on base-building parts.
*/

/** \brief Adds destruction observations around the inherited construction behavior. */
modded class Construction
{
    /** \brief Part names currently being observed in this synchronous callback stack. */
    protected ref array<string> m_ServerSentryRuinedPartsInProgress;

    /**
        \brief Emits a destruction outcome only after a built part becomes unbuilt.
        \param oldLevel \p previous engine health level, not a health-point value.
        \param newLevel \p resulting engine health level.
        \param zone \p original damage-zone name supplied by the engine.
        \note The inherited code owns destruction; this hook never destroys a part.
    */
    override void OnHealthLevelChanged(int oldLevel, int newLevel, string zone)
    {
        ServerSentryBaseDestructionEvent ssEvent;
        ConstructionPart ssPart;
        string ssPartName = zone;

        if (zone != "" && oldLevel != GameConstants.STATE_RUINED && newLevel == GameConstants.STATE_RUINED && ServerSentryBaseDestructionFactory.IsEnabled())
        {
            // Match the lookup performed by the vanilla ConstructionBase hook.
            ssPartName.ToLower();
            if (!m_ServerSentryRuinedPartsInProgress || m_ServerSentryRuinedPartsInProgress.Find(ssPartName) == -1)
            {
                BaseBuildingBase ssParent = GetParent();
                ssPart = GetConstructionPart(ssPartName);
                ssEvent = ServerSentryBaseDestructionFactory.CreateRuinedSnapshot(ssParent, ssPart, zone, oldLevel, newLevel);
                if (ssEvent)
                {
                    if (!m_ServerSentryRuinedPartsInProgress)
                    {
                        m_ServerSentryRuinedPartsInProgress = new array<string>();
                    }
                    m_ServerSentryRuinedPartsInProgress.Insert(ssPartName);
                }
            }
        }

        // Vanilla performs the destruction and any dependent-part updates here.
        super.OnHealthLevelChanged(oldLevel, newLevel, zone);

        if (!ssEvent)
        {
            return;
        }

        m_ServerSentryRuinedPartsInProgress.RemoveItem(ssPartName);

        // Check the captured script part only; the parent may have been deleted
        // by another mod. Never use a time window or an old attacker snapshot.
        if (!ssPart)
        {
            return;
        }
        ssEvent.data.isBuiltAfter = ssPart.IsBuilt();
        if (ssEvent.data.isBuiltAfter)
        {
            return;
        }

        ServerSentryLogger logger = ServerSentryLogger.Get();
        logger.Emit(ssEvent);
    }
}
