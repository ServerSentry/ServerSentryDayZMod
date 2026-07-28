/**
	\file ServerSentryCodeLockEvent.c
	\brief Defines the Code lock event logging data model.
*/

/**
	\brief Represents Code lock events.
*/
class ServerSentryCodeLockEvent extends ServerSentryEvent
{
    /**
    	\brief Stores the data.
    */
    ref ServerSentryCodeLockEventData data;

    /**
    	\brief Initializes a new ServerSentryCodeLockEvent instance.
    */
    void ServerSentryCodeLockEvent()
    {
        data = new ServerSentryCodeLockEventData();
    }
}
