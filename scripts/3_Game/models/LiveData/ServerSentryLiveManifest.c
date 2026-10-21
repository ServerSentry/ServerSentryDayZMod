/**
    \file ServerSentryLiveManifest.c
    \brief Publishes hot world data and references complete, immutable slow-data sections.
*/

/** \brief Schema-four file contract; readers validate every section before accepting a frame. */
class ServerSentryLiveManifest
{
    /** \brief Version of the local file transport. */
    int schemaVersion = 4;
    /** \brief Unique identity of this exporter lifetime. */
    string sessionId;
    /** \brief Monotonic successful manifest publication number. */
    int sequence;
    /** \brief Time at which the Mod sampled this frame. */
    string generatedAtUtc;
    /** \brief Native world name. */
    string mapName;
    /** \brief Current server frames per second. */
    int currentFPS;
    /** \brief Minimum observed server frames per second. */
    int minFPS;
    /** \brief Maximum observed server frames per second. */
    int maxFPS;
    /** \brief Simulation and loaded-world measurements. */
    ref ServerSentryLivePerformanceDto performance;
    /** \brief Complete current player collection. */
    ref array<ref ServerSentryLivePlayerDto> players;
    /** \brief Complete current vehicle collection. */
    ref array<ref ServerSentryLiveVehicleDto> vehicles;
    /** \brief Complete base collection in bases-sessionId-revision.json. */
    int basesRevision;
    /** \brief Complete event collection in events-sessionId-revision.json. */
    int eventsRevision;
    /** \brief Complete action collection in actions-sessionId-revision.json. */
    int actionsRevision;

    /**
        \brief Shares completed hot data without copying the large slow collections.
        \param snapshot \p complete current in-memory snapshot.
    */
    void ServerSentryLiveManifest(ServerSentryLiveSnapshot snapshot)
    {
        generatedAtUtc = snapshot.generatedAtUtc;
        mapName = snapshot.mapName;
        currentFPS = snapshot.currentFPS;
        minFPS = snapshot.minFPS;
        maxFPS = snapshot.maxFPS;
        performance = snapshot.performance;
        players = snapshot.players;
        vehicles = snapshot.vehicles;
    }
}
