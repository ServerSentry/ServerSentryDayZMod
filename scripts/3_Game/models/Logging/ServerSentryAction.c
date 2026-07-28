/**
	\file ServerSentryAction.c
	\brief Defines the Action logging data model.
*/

/**
	\brief Implements the ServerSentry ServerSentry action.
*/
class ServerSentryAction
{
    /**
    	\brief Stores the name.
    */
    string name;
    /**
    	\brief Stores the display name.
    */
    string displayName;
    /**
    	\brief Stores the result.
    */
    string result;
    /**
    	\brief Stores the reason.
    */
    string reason;

    /**
    	\brief Initializes a new ServerSentryAction instance.
    */
    void ServerSentryAction()
    {
        name = "";
        displayName = "";
        result = "";
        reason = "";
    }
}
