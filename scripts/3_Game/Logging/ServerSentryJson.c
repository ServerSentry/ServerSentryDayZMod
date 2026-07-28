/**
	\file ServerSentryJson.c
	\brief Provides logging support for JSON.
*/

/**
	\brief Provides ServerSentry behavior for JSON.
*/
class ServerSentryJson
{
    /**
    	\brief Processes the empty object operation.
    	\return \p The resulting string value.
    */
    static string EmptyObject()
    {
        return "{}";
    }


    /**
    	\brief Processes the make dropped events data operation.
    	\param droppedCount \p dropped count used by the operation.
    	\return \p The resulting string value.
    */
    static string MakeDroppedEventsData(int droppedCount)
    {
        ServerSentryDroppedEventsData data = new ServerSentryDroppedEventsData();
        data.dropped = droppedCount;

        string json;
        string error;
        if (JsonFileLoader<ServerSentryDroppedEventsData>.MakeData(data, json, error, false) && json != "")
        {
            return json;
        }

        Print("[ServerSentry] Failed to serialize dropped-events data: " + error);
        return EmptyObject();
    }

    /**
    	\brief Processes the make item move data operation.
    	\param data \p data processed by the operation.
    	\return \p The resulting string value.
    */
    static string MakeItemMoveData(ServerSentryItemMoveData data)
    {
        if (!data)
        {
            return EmptyObject();
        }

        string json;
        string error;
        if (JsonFileLoader<ServerSentryItemMoveData>.MakeData(data, json, error, false) && json != "")
        {
            return json;
        }

        Print("[ServerSentry] Failed to serialize item move data: " + error);
        return EmptyObject();
    }

    /**
    	\brief Processes the make build action data operation.
    	\param data \p data processed by the operation.
    	\return \p The resulting string value.
    */
    static string MakeBuildActionData(ServerSentryBuildActionData data)
    {
        if (!data)
        {
            return EmptyObject();
        }

        string json;
        string error;
        if (JsonFileLoader<ServerSentryBuildActionData>.MakeData(data, json, error, false) && json != "")
        {
            return json;
        }

        Print("[ServerSentry] Failed to serialize build action data: " + error);
        return EmptyObject();
    }



    /**
    	\brief Processes the make market transaction data operation.
    	\param data \p data processed by the operation.
    	\return \p The resulting string value.
    */
    static string MakeMarketTransactionData(ServerSentryMarketTransactionData data)
    {
        if (!data)
        {
            return EmptyObject();
        }

        string json;
        string error;
        if (JsonFileLoader<ServerSentryMarketTransactionData>.MakeData(data, json, error, false) && json != "")
        {
            return json;
        }

        Print("[ServerSentry] Failed to serialize market transaction data: " + error);
        return EmptyObject();
    }

};
