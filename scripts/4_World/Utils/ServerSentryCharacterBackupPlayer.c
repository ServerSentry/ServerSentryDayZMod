/**
    \file ServerSentryCharacterBackupPlayer.c
    \brief Resets live-only managers before applying persisted character state.
*/

/** \brief Clears existing symptoms before loading a replacement persistence stream. */
modded class SymptomManager
{
    /** \brief Destroys symptoms immediately so old queues cannot affect the restored character. */
    void ServerSentryClearForBackupLoad()
    {
        while (m_SymptomQueuePrimary.Count() > 0)
            m_SymptomQueuePrimary[0].Destroy();
        while (m_SymptomQueueSecondary.Count() > 0)
            m_SymptomQueueSecondary[0].Destroy();
    }
}

/** \brief Provides server-only helpers for restoring an already connected character. */
modded class PlayerBase
{
    protected bool m_ServerSentryDeathBackupAttempted;
    protected bool m_ServerSentryInsideDeathCallback;

    /** Claims one capture per dead character, never per connection or timer. */
    bool ServerSentryClaimDeathBackup()
    {
        if (IsAlive() || !m_ServerSentryInsideDeathCallback || m_ServerSentryDeathBackupAttempted)
            return false;
        m_ServerSentryDeathBackupAttempted = true;
        return true;
    }

    override void EEKilled(Object killer)
    {
        // Vanilla deletes the bleeding manager and updates the hive in super.
        if (GetGame().IsServer())
        {
            m_ServerSentryInsideDeathCallback = true;
            ServerSentryCharacterBackupService.Get().BackupOnDeath(this);
            m_ServerSentryInsideDeathCallback = false;
        }
        super.EEKilled(killer);
    }

    /**
        \brief Returns native statistics in a stable snapshot order.
        \return \p native statistic names.
    */
    protected array<string> ServerSentryBackupStatisticNames()
    {
        array<string> names = new array<string>();
        names.Insert(AnalyticsManagerServer.STAT_PLAYTIME);
        names.Insert(AnalyticsManagerServer.STAT_DISTANCE);
        names.Insert(AnalyticsManagerServer.STAT_PLAYERS_KILLED);
        names.Insert(AnalyticsManagerServer.STAT_INFECTED_KILLED);
        names.Insert(AnalyticsManagerServer.STAT_LONGEST_SURVIVOR_HIT);
        return names;
    }

    /**
        \brief Captures native counters omitted by PlayerBase.OnStoreSave.
        \param values \p receives the counters in stable order.
    */
    void ServerSentryCaptureBackupStatistics(array<float> values)
    {
        StatUpdateByTime(AnalyticsManagerServer.STAT_PLAYTIME);
        StatUpdateByPosition(AnalyticsManagerServer.STAT_DISTANCE);
        array<string> names = ServerSentryBackupStatisticNames();
        foreach (string name : names)
            values.Insert(StatGet(name));
    }

    /** \brief Removes old manager state that vanilla OnStoreLoad would otherwise merge into the snapshot. */
    void ServerSentryPrepareBackupLoad()
    {
        GetModifiersManager().DeactivateAllModifiers();
        GetSymptomManager().ServerSentryClearForBackupLoad();
        GetBleedingManagerServer().RemoveAllSources();
        m_AgentPool.RemoveAllAgents();
        GetStomach().ClearContents();
        if (GetArrowManager())
            GetArrowManager().ClearArrows();
    }

    /**
        \brief Synchronizes restored values without replaying the connection lifecycle.
        \param header \p validated snapshot metadata.
    */
    void ServerSentryFinishBackupLoad(ServerSentryCharacterBackupHeader header)
    {
        // OnStoreLoad's quickbar queue belongs to login. Shortcuts are applied
        // explicitly by the service because OnSelectPlayer is not replayed here.
        m_aQuickBarLoad = null;
        GetSoftSkillsManager().InitSpecialty(GetStatSpecialty().Get());
        GetStaminaHandler().SetStamina(header.Stamina);
        if (header.Version == 3)
        {
            // A death snapshot preserves equipment and persisted character data,
            // but must not reapply fatal wounds/bleeding to a living respawn.
            GetBleedingManagerServer().RemoveAllSources();
            GetSymptomManager().ServerSentryClearForBackupLoad();
        }
        SetRestrained(header.Restrained);
        DayZPlayerSyncJunctures.SendPlayerUnconsciousness(this, header.Unconscious);
        // Consume the teleport in the native distance baseline before replacing counters.
        StatUpdateByGivenPos(AnalyticsManagerServer.STAT_DISTANCE, GetPosition());
        array<string> names = ServerSentryBackupStatisticNames();
        for (int i = 0; i < names.Count(); i++)
            StatUpdate(names[i], header.Statistics[i] - StatGet(names[i]));
        // Reset distance/time baselines after the teleport.
        m_PlayerOldPos = GetPosition();
        SetSynchDirty();
    }
}
