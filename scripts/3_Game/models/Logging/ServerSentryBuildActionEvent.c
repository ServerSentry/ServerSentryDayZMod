/**
	\file ServerSentryBuildActionEvent.c
	\brief Defines the Build action event logging data model.
*/

/**
	\brief Represents Build action events.
*/
class ServerSentryBuildActionEvent extends ServerSentryEvent
{
    /**
    	\brief Stores the data.
    */
    ref ServerSentryBuildActionData data;

    /**
    	\brief Initializes a new ServerSentryBuildActionEvent instance.
    */
    void ServerSentryBuildActionEvent()
    {
        data = new ServerSentryBuildActionData();
    }
}
