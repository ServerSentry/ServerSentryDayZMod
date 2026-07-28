/**
	\file ServerSentrySafeZoneEvent.c
	\brief Defines the Safe zone event logging data model.
*/

/**
	\brief Represents Safe zone events.
*/
class ServerSentrySafeZoneEvent extends ServerSentryEvent
{
    /**
    	\brief Stores the data.
    */
    ref ServerSentrySafeZoneTransitionData data;

    /**
    	\brief Initializes a new ServerSentrySafeZoneEvent instance.
    */
    void ServerSentrySafeZoneEvent()
    {
        data = new ServerSentrySafeZoneTransitionData();
    }
}
