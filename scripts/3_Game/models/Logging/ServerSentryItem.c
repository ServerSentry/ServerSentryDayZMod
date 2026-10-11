/**
	\file ServerSentryItem.c
	\brief Defines the Item logging data model.
*/

/**
	\brief Provides ServerSentry behavior for Item.
*/
class ServerSentryItem
{
    /**
    	\brief Stores the class name.
    */
    string className;
    /**
    	\brief Stores the display name.
    */
    string displayName;
    /**
        \brief Stores the restart-stable DayZ persistence ID when available.
    */
    string persistentId;

    /**
    	\brief Stores the quantity.
    */
    float quantity;
    /**
    	\brief Stores the health.
    */
    float health;

    /**
    	\brief Initializes a new ServerSentryItem instance.
    */
    void ServerSentryItem()
    {
        className = "";
        displayName = "";
        persistentId = "";
        quantity = 0;
        health = 0;
    }
}
