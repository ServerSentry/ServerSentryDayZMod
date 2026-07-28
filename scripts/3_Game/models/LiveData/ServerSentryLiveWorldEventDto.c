/**
	\file ServerSentryLiveWorldEventDto.c
	\brief Defines the Live world event DTO live-data transfer model.
*/

/**
	\brief Data transfer object for Live world event.
*/
class ServerSentryLiveWorldEventDto
{
    /**
    	\brief Stores the event type.
    */
    string eventType;
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
    	\brief Stores the shape.
    */
    string shape;
    /**
    	\brief Stores the icon.
    */
    string icon;
    /**
    	\brief Collection of points values.
    */
    ref array<ref ServerSentryVectorDto> points;
    /**
    	\brief Collection of details values.
    */
    ref array<ref ServerSentryLiveDetailDto> details;
    /**
    	\brief Collection of actions values.
    */
    ref array<string> actions;

    /**
    	\brief Initializes a new ServerSentryLiveWorldEventDto instance.
    */
    void ServerSentryLiveWorldEventDto()
    {
        eventType = "";
        className = "";
        displayName = "";
        objectId = "";
        position = new ServerSentryVectorDto();
        orientation = new ServerSentryVectorDto();
        health = 0;
        radius = 0;
        shape = "point";
        icon = "";
        points = new array<ref ServerSentryVectorDto>();
        details = new array<ref ServerSentryLiveDetailDto>();
        actions = new array<string>();
    }
}
