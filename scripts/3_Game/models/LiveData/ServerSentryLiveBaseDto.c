/**
	\file ServerSentryLiveBaseDto.c
	\brief Defines the Live base DTO live-data transfer model.
*/

/**
	\brief Data transfer object for Live base.
*/
class ServerSentryLiveBaseDto
{
    /**
    	\brief Input or event type exposed to external consumers.
    */
    string type;
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
    	\brief Stores the radius.
    */
    float radius;
    /**
    	\brief Collection of details values.
    */
    ref array<ref ServerSentryLiveDetailDto> details;
    /**
    	\brief Collection of actions values.
    */
    ref array<string> actions;

    string icon;

    /**
    	\brief Initializes a new ServerSentryLiveBaseDto instance.
    */
    void ServerSentryLiveBaseDto()
    {
        type = "base_flagpole";
        className = "";
        icon = "base";
        displayName = "Base / Flag Pole";
        objectId = "";
        position = new ServerSentryVectorDto();
        orientation = new ServerSentryVectorDto();
        health = -1;
        radius = 60;
        details = new array<ref ServerSentryLiveDetailDto>();
        actions = new array<string>();
    }
}
