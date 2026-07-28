/**
	\file ServerSentrySafeZoneTransitionData.c
	\brief Defines the Safe zone transition data logging data model.
*/

/**
	\brief Provides ServerSentry behavior for Safe zone transition data.
*/
class ServerSentrySafeZoneTransitionData
{
    /**
    	\brief Stores the source mod.
    */
    string sourceMod;
    /**
    	\brief Stores the transition type.
    */
    string transitionType;
    /**
    	\brief Stores the result.
    */
    string result;
    /**
    	\brief Stores the reason.
    */
    string reason;

    /**
    	\brief Stores the actor steam ID.
    */
    string actorSteamId;
    /**
    	\brief Stores the actor bohemia ID.
    */
    string actorBohemiaId;
    /**
    	\brief Stores the actor name.
    */
    string actorName;
    /**
    	\brief Stores the actor position.
    */
    ref ServerSentryVectorDto actorPosition;

    /**
    	\brief Stores the zone index.
    */
    int zoneIndex;
    /**
    	\brief Stores the zone name.
    */
    string zoneName;
    /**
    	\brief Stores the zone class name.
    */
    string zoneClassName;
    /**
    	\brief Stores the trader ID.
    */
    int traderId;
    /**
    	\brief Stores the trader name.
    */
    string traderName;
    /**
    	\brief Stores the zone position.
    */
    ref ServerSentryVectorDto zonePosition;
    /**
    	\brief Stores the zone radius.
    */
    float zoneRadius;
    /**
    	\brief Stores the distance to center.
    */
    float distanceToCenter;
    /**
    	\brief Indicates whether safe zone pv p enabled.
    */
    bool safeZonePvPEnabled;
    /**
    	\brief Indicates whether kill on entry.
    */
    bool killOnEntry;
    /**
    	\brief Stores the settings path.
    */
    string settingsPath;

    /**
    	\brief Indicates whether was inside.
    */
    bool wasInside;
    /**
    	\brief Indicates whether is inside.
    */
    bool isInside;

    /**
    	\brief Initializes a new ServerSentrySafeZoneTransitionData instance.
    */
    void ServerSentrySafeZoneTransitionData()
    {
        sourceMod = "safezone";
        transitionType = "";
        result = "success";
        reason = "movement";

        actorSteamId = "";
        actorBohemiaId = "";
        actorName = "";
        actorPosition = new ServerSentryVectorDto();

        zoneIndex = -1;
        zoneName = "";
        zoneClassName = "SafeZone";
        traderId = -1;
        traderName = "";
        zonePosition = new ServerSentryVectorDto();
        zoneRadius = 0;
        distanceToCenter = 0;
        safeZonePvPEnabled = false;
        killOnEntry = false;
        settingsPath = "";

        wasInside = false;
        isInside = false;
    }
}
