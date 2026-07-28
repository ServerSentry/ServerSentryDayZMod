/**
	\file ServerSentryLiveActionRequestDto.c
	\brief Defines the Live action request DTO action transfer model.
*/

/**
	\brief Carries a live-action request received from the ServerSentry backend.
*/
class ServerSentryLiveActionRequestDto
{
    /**
    	\brief Unique ID of the incoming action request.
    */
    string requestId;
    /**
    	\brief Stable key used to make product operations idempotent.
    */
    string fulfillmentKey;
    /**
    	\brief Stores the day z server ID.
    */
    string dayZServerId;
    /**
    	\brief Stores the day z server name.
    */
    string dayZServerName;
    /**
    	\brief Stable ID of the requested action.
    */
    string actionId;
    /**
    	\brief Stores the target.
    */
    ref ServerSentryLiveActionTargetDto target;
    /**
    	\brief Action-specific input encoded as JSON.
    */
    string inputJson;
    /**
    	\brief Stores the requested by.
    */
    string requestedBy;
    /**
    	\brief Stores the requested at utc.
    */
    string requestedAtUtc;
    /**
    	\brief Stores the queued at utc.
    */
    string queuedAtUtc;
    /**
    	\brief Stores the agent job ID.
    */
    string agentJobId;

    /**
    	\brief Initializes a new ServerSentryLiveActionRequestDto instance.
    */
    void ServerSentryLiveActionRequestDto()
    {
        requestId = "";
        fulfillmentKey = "";
        dayZServerId = "";
        dayZServerName = "";
        actionId = "";
        target = new ServerSentryLiveActionTargetDto();
        inputJson = "{}";
        requestedBy = "";
        requestedAtUtc = "";
        queuedAtUtc = "";
        agentJobId = "";
    }
}
