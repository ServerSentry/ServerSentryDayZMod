/**
	\file ServerSentryChatEvent.c
	\brief Defines the Chat event logging data model.
*/

/**
	\brief Represents Chat events.
*/
class ServerSentryChatEvent extends ServerSentryEvent
{
    /**
    	\brief Stores the data.
    */
    ref ServerSentryChatMessageData data;

    /**
    	\brief Initializes a new ServerSentryChatEvent instance.
    */
    void ServerSentryChatEvent()
    {
        data = new ServerSentryChatMessageData();
    }
}
