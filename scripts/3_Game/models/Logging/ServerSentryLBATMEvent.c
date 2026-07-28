/**
	\file ServerSentryLBATMEvent.c
	\brief Defines the Lbatm event logging data model.
*/

/**
	\brief Represents Lbatm events.
*/
class ServerSentryLBATMEvent extends ServerSentryEvent
{
    /**
    	\brief Stores the data.
    */
    ref ServerSentryLBATMEventData data;

    /**
    	\brief Initializes a new ServerSentryLBATMEvent instance.
    */
    void ServerSentryLBATMEvent()
    {
        data = new ServerSentryLBATMEventData();
    }
}
