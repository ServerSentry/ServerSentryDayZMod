/**
	\file ServerSentryLiveActionTargetDto.c
	\brief Defines the Live action target DTO action transfer model.
*/

/**
	\brief Describes the target selected for a live action.
*/
class ServerSentryLiveActionTargetDto
{
    /**
    	\brief Input or event type exposed to external consumers.
    */
    string type;
    /**
    	\brief Network or persistent ID of the target object.
    */
    string objectId;
    /**
    	\brief Steam ID of the target player.
    */
    string steamId;
    /**
    	\brief Stores the bohemia ID.
    */
    string bohemiaId;
    /**
    	\brief Stores the class name.
    */
    string className;
    /**
    	\brief Stores the position.
    */
    ref ServerSentryVectorDto position;

    /**
    	\brief Initializes a new ServerSentryLiveActionTargetDto instance.
    */
    void ServerSentryLiveActionTargetDto()
    {
        type = "object";
        objectId = "";
        steamId = "";
        bohemiaId = "";
        className = "";
        position = new ServerSentryVectorDto();
    }
}
