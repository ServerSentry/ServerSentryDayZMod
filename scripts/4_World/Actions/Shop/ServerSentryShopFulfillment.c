/**
	\file ServerSentryShopFulfillment.c
	\brief Implements shop fulfillment support for Shop fulfillment.
*/

/**
	\brief Stores the state of one idempotent shop fulfillment operation.
*/
class ServerSentryShopFulfillmentOperation
{
    /**
    	\brief Stable key used to make product operations idempotent.
    */
    string fulfillmentKey;
    /**
    	\brief Unique ID of the incoming action request.
    */
    string requestId;
    /**
    	\brief Steam ID of the target player.
    */
    string steamId;
    /**
    	\brief Stable ID of the requested action.
    */
    string actionId;
    /**
    	\brief Stores the reason.
    */
    string reason;
    /**
    	\brief Stores the amount.
    */
    int amount;

    /**
    	\brief Initializes a new ServerSentryShopFulfillmentOperation instance.
    */
    void ServerSentryShopFulfillmentOperation()
    {
        fulfillmentKey = "";
        requestId = "";
        steamId = "";
        actionId = "";
        reason = "";
        amount = 0;
    }
}

/**
	\brief Coordinates idempotent shop grants and removals.
*/
class ServerSentryShopFulfillment
{
    /**
    	\brief Constant value for failed.
    */
    static const int BEGIN_FAILED = -1;
    /**
    	\brief Constant value for fulfilled.
    */
    static const int BEGIN_ALREADY_FULFILLED = 0;
    /**
    	\brief Constant value for ready.
    */
    static const int BEGIN_READY = 1;

    /**
    	\brief Begins an idempotent fulfillment operation and reserves its key.
    	\param ctx \p action context containing request, target, and input data.
    	\param operation \p fulfillment operation to process.
    	\param reason \p human-readable operation description stored with fulfillment state.
    	\param fulfillment \p receives the fulfillment operation state.
    	\param message \p receives a human-readable result or error message.
    	\param requireSteamId \p identifier of the require steam.
    	\param amount \p amount applied by the operation.
    	\return \p The resulting integer value or status code.
    */
    static int Begin(ServerSentryModActionContext ctx, string operation, string reason, out ServerSentryShopFulfillmentOperation fulfillment, out string message, bool requireSteamId = true, int amount = 0)
    {
        fulfillment = null;
        message = "";

        if (!ctx)
        {
            message = "Missing action context.";
            return BEGIN_FAILED;
        }

        if (!ctx.RequireRequestId(message))
        {
            return BEGIN_FAILED;
        }

        if (requireSteamId && !ctx.RequireSteamId(message))
        {
            return BEGIN_FAILED;
        }

        string fulfillmentKey = ctx.GetFulfillmentKeyForOperation(operation);
        if (fulfillmentKey == "")
        {
            message = "Missing fulfillment key. Refusing execution because duplicate protection would not work.";
            return BEGIN_FAILED;
        }

        string ledgerMessage;
        if (ServerSentryShopFulfillmentLedger.IsFulfilled(fulfillmentKey, ledgerMessage))
        {
            message = ledgerMessage;
            return BEGIN_ALREADY_FULFILLED;
        }

        if (ServerSentryShopFulfillmentLedger.IsProcessing(fulfillmentKey, ledgerMessage))
        {
            message = ledgerMessage;
            return BEGIN_FAILED;
        }

        if (!ServerSentryShopFulfillmentLedger.MarkProcessing(fulfillmentKey, ctx.requestId, ctx.steamId, ctx.actionId, amount, reason, ledgerMessage))
        {
            message = ledgerMessage;
            return BEGIN_FAILED;
        }

        fulfillment = new ServerSentryShopFulfillmentOperation();
        fulfillment.fulfillmentKey = fulfillmentKey;
        fulfillment.requestId = ctx.requestId;
        fulfillment.steamId = ctx.steamId;
        fulfillment.actionId = ctx.actionId;
        fulfillment.reason = reason;
        fulfillment.amount = amount;
        message = ledgerMessage;
        return BEGIN_READY;
    }

    /**
    	\brief Begins an idempotent product grant or removal operation.
    	\param ctx \p action context containing request, target, and input data.
    	\param removeProduct \p true to remove the product effect instead of applying it.
    	\param reason \p human-readable operation description stored with fulfillment state.
    	\param fulfillment \p receives the fulfillment operation state.
    	\param message \p receives a human-readable result or error message.
    	\param requireSteamId \p identifier of the require steam.
    	\param amount \p amount applied by the operation.
    	\return \p The resulting integer value or status code.
    */
    static int BeginProductOperation(ServerSentryModActionContext ctx, bool removeProduct, string reason, out ServerSentryShopFulfillmentOperation fulfillment, out string message, bool requireSteamId = true, int amount = 0)
    {
        string operation = "";
        if (removeProduct)
        {
            operation = "remove";
        }

        return Begin(ctx, operation, reason, fulfillment, message, requireSteamId, amount);
    }

    /**
    	\brief Persists the final fulfillment state and returns the action result.
    	\param fulfillment \p receives the fulfillment operation state.
    	\param actionSucceeded \p true when the underlying action completed successfully.
    	\param actionMessage \p message produced by the underlying action.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool Finish(ServerSentryShopFulfillmentOperation fulfillment, bool actionSucceeded, string actionMessage, out string message)
    {
        message = actionMessage;
        if (!fulfillment)
        {
            message = "Missing fulfillment operation state. " + actionMessage;
            return false;
        }

        if (!actionSucceeded)
        {
            ServerSentryShopFulfillmentLedger.MarkFailed(fulfillment.fulfillmentKey, fulfillment.requestId, fulfillment.steamId, fulfillment.actionId, fulfillment.amount, fulfillment.reason, actionMessage);
            return false;
        }

        string ledgerMessage;
        if (!ServerSentryShopFulfillmentLedger.MarkFulfilled(fulfillment.fulfillmentKey, fulfillment.requestId, fulfillment.steamId, fulfillment.actionId, fulfillment.amount, fulfillment.reason, actionMessage, ledgerMessage))
        {
            message = "The operation completed, but ServerSentry could not write the fulfilled receipt. Check manually before retrying. " + ledgerMessage;
            return false;
        }

        message = actionMessage + " requestId=" + fulfillment.requestId;
        return true;
    }
}
