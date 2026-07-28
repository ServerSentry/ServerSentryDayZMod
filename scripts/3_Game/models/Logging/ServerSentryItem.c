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
        quantity = 0;
        health = 0;
    }
}
