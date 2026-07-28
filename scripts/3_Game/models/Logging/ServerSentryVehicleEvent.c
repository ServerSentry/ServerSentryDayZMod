/**
	\file ServerSentryVehicleEvent.c
	\brief Defines the Vehicle event logging data model.
*/

/**
	\brief Represents Vehicle events.
*/
class ServerSentryVehicleEvent extends ServerSentryEvent
{
    /**
    	\brief Stores the data.
    */
    ref ServerSentryVehicleEventData data;

    /**
    	\brief Initializes a new ServerSentryVehicleEvent instance.
    */
    void ServerSentryVehicleEvent()
    {
        data = new ServerSentryVehicleEventData();
    }
}
