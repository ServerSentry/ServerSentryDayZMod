/**
	\file ServerSentryMarketEvent.c
	\brief Defines the Market event logging data model.
*/

/**
	\brief Represents Market events.
*/
class ServerSentryMarketEvent extends ServerSentryEvent
{
    /**
    	\brief Stores the data.
    */
    ref ServerSentryMarketTransactionData data;

    /**
    	\brief Initializes a new ServerSentryMarketEvent instance.
    */
    void ServerSentryMarketEvent()
    {
        data = new ServerSentryMarketTransactionData();
    }
}
