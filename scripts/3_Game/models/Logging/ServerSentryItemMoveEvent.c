/**
	\file ServerSentryItemMoveEvent.c
	\brief Defines the Item move event logging data model.
*/

/**
	\brief Represents Item move events.
*/
class ServerSentryItemMoveEvent extends ServerSentryEvent
{
    /**
    	\brief Stores the data.
    */
    ref ServerSentryItemMoveData data;

    /**
    	\brief Initializes a new ServerSentryItemMoveEvent instance.
    */
    void ServerSentryItemMoveEvent()
    {
        data = new ServerSentryItemMoveData();
    }
}
