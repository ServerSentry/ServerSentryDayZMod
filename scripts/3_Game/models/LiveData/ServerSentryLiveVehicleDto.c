/**
	\file ServerSentryLiveVehicleDto.c
	\brief Defines the Live vehicle DTO live-data transfer model.
*/

/**
	\brief Data transfer object for Live vehicle.
*/
class ServerSentryLiveVehicleDto
{
    /**
    	\brief Stores the class name.
    */
    string className;
    /**
    	\brief Stores the display name.
    */
    string displayName;
    /**
    	\brief Network or persistent ID of the target object.
    */
    string objectId;
    /**
    	\brief Stores the position.
    */
    ref ServerSentryVectorDto position;
    /**
    	\brief Stores the orientation.
    */
    ref ServerSentryVectorDto orientation;
    /**
    	\brief Stores the health.
    */
    float health;
    /**
    	\brief Stores the crew size.
    */
    int crewSize;
    /**
    	\brief Indicates whether engine on.
    */
    bool engineOn;
    /**
    	\brief Indicates whether engine state known.
    */
    bool engineStateKnown;

    /**
    	\brief Stores the icon.
    */
    string icon;

    /**
    	\brief Initializes a new ServerSentryLiveVehicleDto instance.
    */
    void ServerSentryLiveVehicleDto()
    {
        className = "";
        displayName = "";
        objectId = "";
        icon = "vehicle";
        position = new ServerSentryVectorDto();
        orientation = new ServerSentryVectorDto();
        health = 0;
        crewSize = 0;
        engineOn = false;
        engineStateKnown = false;
    }
}
