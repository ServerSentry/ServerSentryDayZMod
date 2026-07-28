/**
	\file ServerSentryDroppedEventsData.c
	\brief Defines the Dropped events data logging data model.
*/

/**
	\brief Provides ServerSentry behavior for Dropped events data.
*/
class ServerSentryDroppedEventsData
{
    /**
    	\brief Stores the dropped.
    */
    int dropped;

    /**
    	\brief Initializes a new ServerSentryDroppedEventsData instance.
    */
    void ServerSentryDroppedEventsData()
    {
        dropped = 0;
    }
};
