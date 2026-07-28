/**
	\file ServerSentryCatalog.c
	\brief Defines ServerSentry support for Catalog.
*/

/**
	\brief Provides ServerSentry behavior for Catalog.
*/
class ServerSentryCatalog
{
    /**
    	\brief Stores the schema version.
    */
    int schemaVersion;
    /**
    	\brief Stores the generator.
    */
    string generator;
    /**
    	\brief Stores the item count.
    */
    int itemCount;

    /**
    	\brief Collection of items values.
    */
    ref array<ref ServerSentryModItem> items;

    /**
    	\brief Initializes a new ServerSentryCatalog instance.
    */
    void ServerSentryCatalog()
    {
        schemaVersion = 7;
        generator = "Server Sentry";
        itemCount = 0;
        items = new array<ref ServerSentryModItem>;
    }
};
