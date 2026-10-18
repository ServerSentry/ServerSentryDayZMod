/**
	\file ServerSentryLiveActionResultDto.c
	\brief Defines the Live action result DTO action transfer model.
*/

/**
	\brief Carries the result returned after a live action executes.
*/
class ServerSentryLiveActionResultDto
{
    /**
    	\brief Unique ID of the incoming action request.
    */
    string requestId;
    /**
    	\brief Stable ID of the requested action.
    */
    string actionId;
    /**
    	\brief Type of target resolved by the action.
    */
    string targetType;
    /**
    	\brief Stores the target object ID.
    */
    string targetObjectId;
    /**
    	\brief Indicates whether the operation completed successfully.
    */
    bool success;
    /**
    	\brief Human-readable result or error message.
    */
    string message;
    /** \brief Optional typed response; empty for actions without response data. */
    string dataJson;
    /**
    	\brief Stores the executed at utc.
    */
    string executedAtUtc;

    /**
    	\brief Initializes a new ServerSentryLiveActionResultDto instance.
    */
    void ServerSentryLiveActionResultDto()
    {
        requestId = "";
        actionId = "";
        targetType = "";
        targetObjectId = "";
        success = false;
        message = "";
        dataJson = "";
        executedAtUtc = "";
    }
}
