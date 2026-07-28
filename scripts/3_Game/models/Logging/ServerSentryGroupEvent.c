/**
	\file ServerSentryGroupEvent.c
	\brief Defines the Group event logging data model.
*/

/**
	\brief Represents Group events.
*/
class ServerSentryGroupEvent extends ServerSentryEvent
{
    /**
    	\brief Stores the data.
    */
    ref ServerSentryGroupEventData data;

    /**
    	\brief Initializes a new ServerSentryGroupEvent instance.
    */
    void ServerSentryGroupEvent()
    {
        data = new ServerSentryGroupEventData();
    }
}
