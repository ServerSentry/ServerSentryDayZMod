/**
	\file ServerSentryBreachingChargeEvent.c
	\brief Defines the Breaching charge event logging data model.
*/

/**
	\brief Represents Breaching charge events.
*/
class ServerSentryBreachingChargeEvent extends ServerSentryEvent
{
    /**
    	\brief Stores the data.
    */
    ref ServerSentryBreachingChargeEventData data;

    /**
    	\brief Initializes a new ServerSentryBreachingChargeEvent instance.
    */
    void ServerSentryBreachingChargeEvent()
    {
        data = new ServerSentryBreachingChargeEventData();
    }
}
