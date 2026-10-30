/**
	\file ServerSentryLoggingConfig.c
	\brief Defines ServerSentry support for Logging config.
*/

/**
	\brief Stores configuration for Logging.
*/
class ServerSentryLoggingConfig
{
    /** \brief Persisted schema version; zero identifies an unversioned configuration. */
    int ConfigVersion;
    /**
    	\brief Indicates whether enable item export.
    */
    bool EnableItemExport;
    /**
    	\brief Indicates whether enable telemtry logging.
    */
    bool EnableTelemtryLogging;
    /**
    	\brief Indicates whether enable live data logging.
    */
    bool EnableLiveDataLogging;
    /**
    	\brief Stores the live data update interval seconds.
    */
    int LiveDataUpdateIntervalSeconds;
    /** \brief Maximum base-detail age in seconds; zero rebuilds on every snapshot. */
    int LiveBaseDetailsCacheSeconds;
    /**
    	\brief Indicates whether log animal hits.
    */
    bool LogAnimalHits;
    /**
    	\brief Indicates whether log animal kills.
    */
    bool LogAnimalKills;
    /**
    	\brief Indicates whether log zombie hits.
    */
    bool LogZombieHits;
    /**
    	\brief Indicates whether log zombie kills.
    */
    bool LogZombieKills;
    /**
    	\brief Indicates whether log vehicle engine start.
    */
    bool LogVehicleEngineStart;
    /**
    	\brief Indicates whether log vehicle engine stop.
    */
    bool LogVehicleEngineStop;
    /**
    	\brief Indicates whether log enter vehicle.
    */
    bool LogEnterVehicle;
    /**
    	\brief Indicates whether log exit vehicle.
    */
    bool LogExitVehicle;
    /**
    	\brief Indicates whether log vehicle deletion.
    */
    bool LogVehicleDeletion;
    /**
    	\brief Indicates whether log inventory movement.
    */
    bool LogInventoryMovement;
    /**
    	\brief Indicates whether log player hits.
    */
    bool LogPlayerHits;
    /**
    	\brief Indicates whether log player kills.
    */
    bool LogPlayerKills;
    /**
    	\brief Indicates whether log base building.
    */
    bool LogBaseBuilding;
    /** \brief Enables damage events independently of base-building actions. */
    bool LogBaseDamage;
    /**
    	\brief Indicates whether log action dismantle part.
    */
    bool LogActionDismantlePart;
    /**
    	\brief Indicates whether log action destroy part.
    */
    bool LogActionDestroyPart;

    /** \brief Character backup policy, initialized before JSON overrides are applied. */
    ref ServerSentryCharBackupConfig CharBackupConfig;

    /**
    	\brief Initializes a new ServerSentryLoggingConfig instance.
    */
    void ServerSentryLoggingConfig()
    {
        // JsonFileLoader constructs this class before applying values from disk.
        // Initializing here keeps newly-added settings enabled for older config.json
        // files where those properties do not exist yet. Explicit false values in
        // the JSON are still applied afterwards.
        InitDefaultValues();
    }

    /**
    	\brief Processes the init default values operation.
    */
    void InitDefaultValues()
    {
        EnableItemExport = true;
        EnableTelemtryLogging = true;
        EnableLiveDataLogging = true;
        LiveDataUpdateIntervalSeconds = 10;
        LiveBaseDetailsCacheSeconds = 30;
        LogAnimalHits = true;
        LogAnimalKills = true;
        LogZombieHits = true;
        LogZombieKills = true;
        LogVehicleEngineStart = true;
        LogVehicleEngineStop = true;
        LogEnterVehicle = true;
        LogExitVehicle = true;
        LogVehicleDeletion = true;
        LogInventoryMovement = true;
        LogPlayerHits = true;
        LogPlayerKills = true;
        LogBaseBuilding = false;
        LogBaseDamage = false;
        LogActionDismantlePart = false;
        LogActionDestroyPart = false;
        CharBackupConfig = new ServerSentryCharBackupConfig();
    }
}
