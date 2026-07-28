/**
	\file ServerSentryLiveDetailDto.c
	\brief Defines the Live detail DTO live-data transfer model.
*/

/**
	\brief Data transfer object for Live detail.
*/
class ServerSentryLiveDetailDto
{
    /**
    	\brief Stable key used for lookup and serialization.
    */
    string key;
    /**
    	\brief Human-readable label shown to users.
    */
    string label;
    /**
    	\brief Stores the value.
    */
    string value;

    /**
    	\brief Initializes a new ServerSentryLiveDetailDto instance.
    */
    void ServerSentryLiveDetailDto()
    {
        key = "";
        label = "";
        value = "";
    }
}
