/**
	\file ServerSentryLiveSnapshot.c
	\brief Defines the Live snapshot live-data transfer model.
*/

/**
	\brief Provides ServerSentry behavior for Live snapshot.
*/
class ServerSentryLiveSnapshot
{
    /**
    	\brief Stores the schema version.
    */
    int schemaVersion;
    /**
    	\brief Stores the generated at utc.
    */
    string generatedAtUtc;
    /**
    	\brief Stores the map name.
    */
    string mapName;
    /**
    	\brief Stores the current fps.
    */
    int currentFPS;
    /**
    	\brief Stores the min fps.
    */
    int minFPS;
    /**
    	\brief Stores the max fps.
    */
    int maxFPS;
    /** \brief Optional additive simulation and loaded-world item measurements. */
    ref ServerSentryLivePerformanceDto performance;

    /**
    	\brief Collection of players values.
    */
    ref array<ref ServerSentryLivePlayerDto> players;
    /**
    	\brief Collection of vehicles values.
    */
    ref array<ref ServerSentryLiveVehicleDto> vehicles;
    /**
    	\brief Collection of events values.
    */
    ref array<ref ServerSentryLiveWorldEventDto> events;
    /**
    	\brief Collection of bases values.
    */
    ref array<ref ServerSentryLiveBaseDto> bases;
    /**
    	\brief Collection of actions values.
    */
    ref array<ref ServerSentryLiveModActionDescriptorDto> actions;

    /**
    	\brief Initializes a new ServerSentryLiveSnapshot instance.
    */
    void ServerSentryLiveSnapshot()
    {
        schemaVersion = ServerSentryDefaults.LiveSchemaVersion();
        generatedAtUtc = ServerSentryTime.NowUtc();
        performance = new ServerSentryLivePerformanceDto();
        players = new array<ref ServerSentryLivePlayerDto>();
        vehicles = new array<ref ServerSentryLiveVehicleDto>();
        events = new array<ref ServerSentryLiveWorldEventDto>();
        bases = new array<ref ServerSentryLiveBaseDto>();
        actions = new array<ref ServerSentryLiveModActionDescriptorDto>();
    }
}

/** \brief Snapshot measurements; negative timing sentinels mean unavailable, not zero. */
class ServerSentryLivePerformanceDto
{
    /** \brief Mean simulation frame interval in milliseconds. */
    float averageTickTimeMs = -1.0;
    /** \brief Longest frame interval in the measurement window. */
    float maxTickTimeMs = -1.0;
    /** \brief Number of frames in the measurement window. */
    int tickSampleCount = 0;
    /** \brief Loaded items without an inventory parent. */
    int groundItemCount = 0;
    /** \brief All loaded item instances, including inventories and attachments. */
    int loadedItemCount = 0;
}
