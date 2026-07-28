/**
	\file ServerSentryModActionContext.c
	\brief Implements live-action support for Mod action context.
*/

/**
	\brief Carries request identifiers, target data, and input for one live action.
*/
class ServerSentryModActionContext
{
    /**
    	\brief Unique ID of the incoming action request.
    */
    string requestId;
    /**
    	\brief Stable key used to make product operations idempotent.
    */
    string fulfillmentKey;
    /**
    	\brief Stable ID of the requested action.
    */
    string actionId;
    /**
    	\brief Type of target resolved by the action.
    */
    string targetType;
    /**
    	\brief Network or persistent ID of the target object.
    */
    string objectId;
    /**
    	\brief Steam ID of the target player.
    */
    string steamId;
    /**
    	\brief Stores the bohemia ID.
    */
    string bohemiaId;
    /**
    	\brief Stores the class name.
    */
    string className;
    /**
    	\brief Stores the position.
    */
    ref ServerSentryVectorDto position;
    /**
    	\brief Action-specific input encoded as JSON.
    */
    string inputJson;
    /**
    	\brief Stores the requested by.
    */
    string requestedBy;

    /**
    	\brief Initializes a new ServerSentryModActionContext instance.
    */
    void ServerSentryModActionContext()
    {
        requestId = "";
        fulfillmentKey = "";
        actionId = "";
        targetType = "object";
        objectId = "";
        steamId = "";
        bohemiaId = "";
        className = "";
        position = new ServerSentryVectorDto();
        inputJson = "{}";
        requestedBy = "";
    }

    /**
    	\brief Returns a string input value or the supplied fallback.
    	\param key \p stable key used for lookup or serialization.
    	\param fallback \p fallback used by the operation.
    	\return \p The resulting string value.
    */
    string GetString(string key, string fallback = "")
    {
        return ServerSentryLiveActionInput.GetString(inputJson, key, fallback);
    }

    /**
    	\brief Returns an integer input value or the supplied fallback.
    	\param key \p stable key used for lookup or serialization.
    	\param fallback \p fallback used by the operation.
    	\return \p The resulting integer value or status code.
    */
    int GetInt(string key, int fallback = 0)
    {
        return ServerSentryLiveActionInput.GetInt(inputJson, key, fallback);
    }

    /**
    	\brief Returns a floating-point input value or the supplied fallback.
    	\param key \p stable key used for lookup or serialization.
    	\param fallback \p fallback used by the operation.
    	\return \p The resulting floating-point value.
    */
    float GetFloat(string key, float fallback = 0)
    {
        return ServerSentryLiveActionInput.GetFloat(inputJson, key, fallback);
    }

    /**
    	\brief Returns a boolean input value or the supplied fallback.
    	\param key \p stable key used for lookup or serialization.
    	\param fallback \p true to enable fallback.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool GetBool(string key, bool fallback = false)
    {
        return ServerSentryLiveActionInput.GetBool(inputJson, key, fallback);
    }

    /**
    	\brief Returns whether the current action removes a product effect.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    bool IsRemoveProduct()
    {
        return GetBool("removeProduct", false);
    }

    /**
    	\brief Returns the base idempotency key for this request.
    	\return \p The resulting string value.
    */
    string GetFulfillmentKey()
    {
        if (fulfillmentKey != "")
        {
            return fulfillmentKey;
        }

        return requestId;
    }

    /**
    	\brief Returns the idempotency key for a named product operation.
    	\param operation \p fulfillment operation to process.
    	\return \p The resulting string value.
    */
    string GetFulfillmentKeyForOperation(string operation = "")
    {
        string key = GetFulfillmentKey();
        operation.TrimInPlace();
        if (key == "" || operation == "")
        {
            return key;
        }

        return key + ":" + operation;
    }

    /**
    	\brief Validates that the action request contains a request ID.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool RequireRequestId(out string message)
    {
        if (requestId == "")
        {
            message = "Missing requestId. Refusing execution because duplicate protection would not work.";
            return false;
        }

        message = "";
        return true;
    }

    /**
    	\brief Validates that the action request contains a target Steam ID.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool RequireSteamId(out string message)
    {
        if (steamId == "")
        {
            message = "Missing target SteamID64.";
            return false;
        }

        message = "";
        return true;
    }

    /**
    	\brief Attempts to resolve the target player from the action context.
    	\param player \p receives the player entity associated with the operation.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool TryGetPlayer(out PlayerBase player, out string message)
    {
        player = ServerSentryLiveActionResolver.FindPlayer(this);
        if (!player)
        {
            message = "Player not found or offline.";
            return false;
        }

        message = "";
        return true;
    }

    /**
    	\brief Attempts to resolve the target world object from the action context.
    	\param targetObject \p receives the target object used by the operation.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool TryGetObject(out EntityAI targetObject, out string message)
    {
        targetObject = ServerSentryLiveActionResolver.FindObject(this);
        if (!targetObject)
        {
            message = "Target object not found.";
            return false;
        }

        message = "";
        return true;
    }
}
