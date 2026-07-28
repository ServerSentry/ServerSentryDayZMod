/**
	\file ServerSentryVehicleEventData.c
	\brief Defines the Vehicle event data logging data model.
*/

/**
	\brief Carries payload data for Vehicle events.
*/
class ServerSentryVehicleEventData
{
    /**
    	\brief Stores the source mod.
    */
    string sourceMod;
    /**
    	\brief Stores the vehicle action.
    */
    string vehicleAction;
    /**
    	\brief Stores the result.
    */
    string result;
    /**
    	\brief Stores the reason.
    */
    string reason;
    /**
    	\brief Stores the source hook.
    */
    string sourceHook;

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
    	\brief Stores the vehicle class name.
    */
    string vehicleClassName;
    /**
    	\brief Stores the vehicle display name.
    */
    string vehicleDisplayName;
    /**
    	\brief Stores the vehicle object ID.
    */
    string vehicleObjectId;
    /**
    	\brief Stores the vehicle persistent ID.
    */
    string vehiclePersistentId;
    /**
    	\brief Stores the vehicle position.
    */
    ref ServerSentryVectorDto vehiclePosition;
    /**
    	\brief Stores the vehicle orientation.
    */
    ref ServerSentryVectorDto vehicleOrientation;
    /**
    	\brief Stores the vehicle health.
    */
    float vehicleHealth;
    /**
    	\brief Indicates whether engine on.
    */
    bool engineOn;
    /**
    	\brief Indicates whether engine state known.
    */
    bool engineStateKnown;

    /**
    	\brief Stores the seat index.
    */
    int seatIndex;
    /**
    	\brief Stores the seat name.
    */
    string seatName;
    /**
    	\brief Stores the crew size.
    */
    int crewSize;

    /**
    	\brief Initializes a new ServerSentryVehicleEventData instance.
    */
    void ServerSentryVehicleEventData()
    {
        sourceMod = "ServerSentry";
        vehicleAction = "";
        result = "success";
        reason = "";
        sourceHook = "";

        actorSteamId = "";
        actorBohemiaId = "";
        actorName = "";
        actorPosition = new ServerSentryVectorDto();

        vehicleClassName = "";
        vehicleDisplayName = "";
        vehicleObjectId = "";
        vehiclePersistentId = "";
        vehiclePosition = new ServerSentryVectorDto();
        vehicleOrientation = new ServerSentryVectorDto();
        vehicleHealth = 0;
        engineOn = false;
        engineStateKnown = false;

        seatIndex = -1;
        seatName = "";
        crewSize = 0;
    }
}
