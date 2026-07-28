/**
	\file ConfigManager.c
	\brief Defines ServerSentry support for Config manager.
*/

/**
	\brief Coordinates the lifecycle of Config.
*/
class ServerSentryConfigManager
{
    /**
    	\brief Constant value for dir.
    */
    static const string CONFIG_DIR = "$profile:ServerSentry";
    /**
    	\brief Constant value for filename.
    */
    static const string CONFIG_FILENAME = CONFIG_DIR + "/config.json";
    /**
    	\brief Instance state for server config.
    */
    protected ref ServerSentryLoggingConfig m_ServerConfig;

    /**
    	\brief Initializes a new ServerSentryConfigManager instance.
    */
    void ServerSentryConfigManager()
    {
        if (!FileExist(CONFIG_DIR))
        {
            MakeDirectory(CONFIG_DIR);
            Print("[Server Sentry] Created directory: " + CONFIG_DIR);
        }

        if (!FileExist(CONFIG_FILENAME))
        {
            m_ServerConfig = new ServerSentryLoggingConfig();
            m_ServerConfig.InitDefaultValues();
            JsonFileLoader<ServerSentryLoggingConfig>.JsonSaveFile(CONFIG_FILENAME, m_ServerConfig);
            Print("[Server Sentry] Created Default config!");
            return;
        }

        Print("[Server Sentry] Reading config at: " + CONFIG_FILENAME);
        // Start from current defaults so older config files can safely omit newly
        // added properties such as LogAnimalKills. Values present in JSON override
        // these defaults during deserialization.
        m_ServerConfig = new ServerSentryLoggingConfig();
        JsonFileLoader<ServerSentryLoggingConfig>.JsonLoadFile(CONFIG_FILENAME, m_ServerConfig);
    }

    /**
    	\brief Returns whether allowed to export items.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    bool IsAllowedToExportItems()
    {
        return m_ServerConfig.EnableItemExport;
    }

    /**
    	\brief Returns whether allowed to start live logging.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    bool IsAllowedToStartLiveLogging()
    {
        return m_ServerConfig.EnableLiveDataLogging;
    }

    /**
    	\brief Returns whether allowed to start telemetry logging.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    bool IsAllowedToStartTelemetryLogging()
    {
        return m_ServerConfig.EnableTelemtryLogging;
    }

    /**
    	\brief Returns live update polling delay.
    	\return \p The resulting floating-point value.
    */
    float GetLiveUpdatePollingDelay()
    {
        return m_ServerConfig.LiveDataUpdateIntervalSeconds * 1000;
    }

    /**
    	\brief Returns whether allowed to log animal damage.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    bool IsAllowedToLogAnimalDamage()
    {
        return m_ServerConfig.LogAnimalHits;
    }

    /**
    	\brief Returns whether allowed to log animal kills.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    bool IsAllowedToLogAnimalKills()
    {
        return m_ServerConfig.LogAnimalKills;
    }

    /**
    	\brief Returns whether allowed to log zombie hits.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    bool IsAllowedToLogZombieHits()
    {
        return m_ServerConfig.LogZombieHits;
    }

    /**
    	\brief Returns whether allowed to log zombie kills.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    bool IsAllowedToLogZombieKills()
    {
        return m_ServerConfig.LogZombieKills;
    }

    /**
    	\brief Returns whether allowed to log vehicle deletion.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    bool IsAllowedToLogVehicleDeletion()
    {
        return m_ServerConfig.LogVehicleDeletion;
    }

    /**
    	\brief Returns whether allowed to log engine start.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    bool IsAllowedToLogEngineStart()
    {
        return m_ServerConfig.LogVehicleEngineStart;
    }

    /**
    	\brief Returns whether allowed to log engine stop.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    bool IsAllowedToLogEngineStop()
    {
        return m_ServerConfig.LogVehicleEngineStop;
    }

    /**
    	\brief Returns whether allowed to log item movement.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    bool IsAllowedToLogItemMovement()
    {
        return m_ServerConfig.LogInventoryMovement;
    }

    /**
    	\brief Returns whether allowed to log player hits.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    bool IsAllowedToLogPlayerHits()
    {
        return m_ServerConfig.LogPlayerHits;
    }

    /**
    	\brief Returns whether allowed to log player kills.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    bool IsAllowedToLogPlayerKills()
    {
        return m_ServerConfig.LogPlayerKills;
    }

    /**
    	\brief Returns whether allowed to log base building.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    bool IsAllowedToLogBaseBuilding()
    {
        return m_ServerConfig.LogBaseBuilding;
    }

    /**
    	\brief Returns whether allowed to log dismantle.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    bool IsAllowedToLogDismantle()
    {
        return m_ServerConfig.LogActionDismantlePart;
    }

    /**
    	\brief Returns whether allowed to log destroy.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    bool IsAllowedToLogDestroy()
    {
        return m_ServerConfig.LogActionDestroyPart;
    }

    /**
    	\brief Stores the g instance.
    */
    static ref ServerSentryConfigManager g_Instance;
    /**
    	\brief Returns the shared singleton instance.
    	\return \p The resulting config manager instance, or null when unavailable.
    */
    static ref ServerSentryConfigManager Get()
    {
        if(!g_Instance)
            g_Instance = new ServerSentryConfigManager();

        return g_Instance;
    }
};
