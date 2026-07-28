/**
	\file ServerSentryShopFulfillmentLedger.c
	\brief Implements shop fulfillment support for Shop fulfillment ledger.
*/

/**
	\brief Stores the persistent receipt for one fulfillment key.
*/
class ServerSentryShopFulfillmentReceipt
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
    	\brief Stores the amount.
    */
    int amount;
    /**
    	\brief Stores the reason.
    */
    string reason;
    /**
    	\brief Stores the status.
    */
    string status;
    /**
    	\brief Human-readable result or error message.
    */
    string message;
    /**
    	\brief Stores the created at utc.
    */
    string createdAtUtc;
    /**
    	\brief Stores the updated at utc.
    */
    string updatedAtUtc;

    /**
    	\brief Initializes a new ServerSentryShopFulfillmentReceipt instance.
    */
    void ServerSentryShopFulfillmentReceipt()
    {
        fulfillmentKey = "";
        requestId = "";
        steamId = "";
        actionId = "";
        amount = 0;
        reason = "";
        status = "";
        message = "";
        createdAtUtc = "";
        updatedAtUtc = "";
    }
}

/**
	\brief Persists and reads fulfillment receipts by idempotency key.
*/
class ServerSentryShopFulfillmentLedger
{
    /**
    	\brief Returns whether a fulfillment key already completed successfully.
    	\param fulfillmentKey \p stable product-operation key used for idempotency.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsFulfilled(string fulfillmentKey, out string message)
    {
        message = "";
        ServerSentryShopFulfillmentReceipt receipt = Load(fulfillmentKey);
        if (!receipt)
        {
            return false;
        }

        if (receipt.status == "fulfilled")
        {
            message = "Shop fulfillment already completed. requestId=" + receipt.requestId + " steamId=" + receipt.steamId + " amount=" + receipt.amount.ToString();
            return true;
        }

        return false;
    }

    /**
    	\brief Returns whether a fulfillment key is currently reserved.
    	\param fulfillmentKey \p stable product-operation key used for idempotency.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsProcessing(string fulfillmentKey, out string message)
    {
        message = "";
        ServerSentryShopFulfillmentReceipt receipt = Load(fulfillmentKey);
        if (!receipt)
        {
            return false;
        }

        if (receipt.status == "processing")
        {
            message = "Shop fulfillment is already marked as processing. Refusing to execute again to avoid duplicates. fulfillmentKey=" + fulfillmentKey + " requestId=" + receipt.requestId;
            return true;
        }

        return false;
    }

    /**
    	\brief Persists a processing receipt for a fulfillment operation.
    	\param fulfillmentKey \p stable product-operation key used for idempotency.
    	\param requestId \p unique ID of the incoming request.
    	\param steamId \p Steam ID of the target player.
    	\param actionId \p stable ID of the action.
    	\param amount \p amount applied by the operation.
    	\param reason \p human-readable operation description stored with fulfillment state.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool MarkProcessing(string fulfillmentKey, string requestId, string steamId, string actionId, int amount, string reason, out string message)
    {
        message = "";
        if (!ValidateKey(fulfillmentKey, message))
        {
            return false;
        }

        if (!EnsureDirectories())
        {
            message = "Failed to create ServerSentry shop fulfillment ledger directory.";
            return false;
        }

        ServerSentryShopFulfillmentReceipt existing = Load(fulfillmentKey);
        if (existing)
        {
            if (existing.status == "fulfilled")
            {
                message = "Shop fulfillment already completed. requestId=" + existing.requestId + " amount=" + existing.amount.ToString();
                return false;
            }

            if (existing.status == "processing")
            {
                message = "Shop fulfillment is already processing. Not executing again to avoid duplicates. requestId=" + existing.requestId;
                return false;
            }
        }

        ServerSentryShopFulfillmentReceipt receipt = new ServerSentryShopFulfillmentReceipt();
        receipt.fulfillmentKey = fulfillmentKey;
        receipt.requestId = requestId;
        receipt.steamId = steamId;
        receipt.actionId = actionId;
        receipt.amount = amount;
        receipt.reason = reason;
        receipt.status = "processing";
        receipt.message = "Shop fulfillment started.";
        receipt.createdAtUtc = ServerSentryTime.NowUtc();
        receipt.updatedAtUtc = receipt.createdAtUtc;

        JsonFileLoader<ServerSentryShopFulfillmentReceipt>.JsonSaveFile(Path(fulfillmentKey), receipt);
        message = "Shop fulfillment marked processing.";
        return true;
    }

    /**
    	\brief Persists a failed receipt for a fulfillment operation.
    	\param fulfillmentKey \p stable product-operation key used for idempotency.
    	\param requestId \p unique ID of the incoming request.
    	\param steamId \p Steam ID of the target player.
    	\param actionId \p stable ID of the action.
    	\param amount \p amount applied by the operation.
    	\param reason \p human-readable operation description stored with fulfillment state.
    	\param failureMessage \p receives a human-readable result or error description.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool MarkFailed(string fulfillmentKey, string requestId, string steamId, string actionId, int amount, string reason, string failureMessage)
    {
        string ignored;
        if (!ValidateKey(fulfillmentKey, ignored))
        {
            return false;
        }

        if (!EnsureDirectories())
        {
            return false;
        }

        ServerSentryShopFulfillmentReceipt receipt = Load(fulfillmentKey);
        if (!receipt)
        {
            receipt = new ServerSentryShopFulfillmentReceipt();
            receipt.fulfillmentKey = fulfillmentKey;
            receipt.requestId = requestId;
            receipt.steamId = steamId;
            receipt.actionId = actionId;
            receipt.amount = amount;
            receipt.reason = reason;
            receipt.createdAtUtc = ServerSentryTime.NowUtc();
        }

        receipt.status = "failed";
        receipt.message = failureMessage;
        receipt.updatedAtUtc = ServerSentryTime.NowUtc();
        JsonFileLoader<ServerSentryShopFulfillmentReceipt>.JsonSaveFile(Path(fulfillmentKey), receipt);
        return true;
    }

    /**
    	\brief Persists a successful receipt for a fulfillment operation.
    	\param fulfillmentKey \p stable product-operation key used for idempotency.
    	\param requestId \p unique ID of the incoming request.
    	\param steamId \p Steam ID of the target player.
    	\param actionId \p stable ID of the action.
    	\param amount \p amount applied by the operation.
    	\param reason \p human-readable operation description stored with fulfillment state.
    	\param actionMessage \p message produced by the underlying action.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool MarkFulfilled(string fulfillmentKey, string requestId, string steamId, string actionId, int amount, string reason, string actionMessage, out string message)
    {
        message = "";
        if (!ValidateKey(fulfillmentKey, message))
        {
            return false;
        }

        if (!EnsureDirectories())
        {
            message = "Failed to create ServerSentry shop fulfillment ledger directory.";
            return false;
        }

        ServerSentryShopFulfillmentReceipt receipt = Load(fulfillmentKey);
        if (!receipt)
        {
            receipt = new ServerSentryShopFulfillmentReceipt();
            receipt.fulfillmentKey = fulfillmentKey;
            receipt.requestId = requestId;
            receipt.steamId = steamId;
            receipt.actionId = actionId;
            receipt.amount = amount;
            receipt.reason = reason;
            receipt.createdAtUtc = ServerSentryTime.NowUtc();
        }

        receipt.status = "fulfilled";
        receipt.requestId = requestId;
        receipt.steamId = steamId;
        receipt.actionId = actionId;
        receipt.amount = amount;
        receipt.reason = reason;
        receipt.message = actionMessage;
        receipt.updatedAtUtc = ServerSentryTime.NowUtc();

        JsonFileLoader<ServerSentryShopFulfillmentReceipt>.JsonSaveFile(Path(fulfillmentKey), receipt);
        message = "Shop fulfillment marked fulfilled. requestId=" + requestId;
        return true;
    }

    /**
    	\brief Loads the fulfillment receipt stored for a key.
    	\param fulfillmentKey \p stable product-operation key used for idempotency.
    	\return \p The resulting shop fulfillment receipt instance, or null when unavailable.
    */
    protected static ServerSentryShopFulfillmentReceipt Load(string fulfillmentKey)
    {
        if (fulfillmentKey == "")
        {
            return null;
        }

        string path = Path(fulfillmentKey);
        if (!FileExist(path))
        {
            return null;
        }

        ServerSentryShopFulfillmentReceipt receipt = new ServerSentryShopFulfillmentReceipt();
        JsonFileLoader<ServerSentryShopFulfillmentReceipt>.JsonLoadFile(path, receipt);
        return receipt;
    }

    /**
    	\brief Validates a fulfillment key before it is used as persistent state.
    	\param fulfillmentKey \p stable product-operation key used for idempotency.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    protected static bool ValidateKey(string fulfillmentKey, out string message)
    {
        message = "";
        if (fulfillmentKey == "")
        {
            message = "Missing fulfillment key.";
            return false;
        }

        return true;
    }

    /**
    	\brief Creates the fulfillment storage directories when necessary.
    	\return \p true when the operation succeeds; otherwise false.
    */
    protected static bool EnsureDirectories()
    {
        if (!FileExist("$profile:ServerSentry"))
        {
            if (!MakeDirectory("$profile:ServerSentry"))
            {
                return false;
            }
        }

        if (!FileExist("$profile:ServerSentry/fulfillment"))
        {
            if (!MakeDirectory("$profile:ServerSentry/fulfillment"))
            {
                return false;
            }
        }

        if (!FileExist("$profile:ServerSentry/fulfillment/shop"))
        {
            if (!MakeDirectory("$profile:ServerSentry/fulfillment/shop"))
            {
                return false;
            }
        }

        if (!FileExist(Directory()))
        {
            if (!MakeDirectory(Directory()))
            {
                return false;
            }
        }

        return true;
    }

    /**
    	\brief Returns the fulfillment receipt directory.
    	\return \p The resulting string value.
    */
    protected static string Directory()
    {
        return "$profile:ServerSentry/fulfillment/shop/actions";
    }

    /**
    	\brief Returns the receipt path for a fulfillment key.
    	\param fulfillmentKey \p stable product-operation key used for idempotency.
    	\return \p The resulting string value.
    */
    protected static string Path(string fulfillmentKey)
    {
        return Directory() + "/" + BuildSafeKey(fulfillmentKey) + ".json";
    }

    /**
    	\brief Converts a fulfillment key to a file-system-safe name.
    	\param key \p stable key used for lookup or serialization.
    	\return \p The resulting string value.
    */
    protected static string BuildSafeKey(string key)
    {
        string safe = key;
        safe.Replace("/", "_");
        safe.Replace("\\", "_");
        safe.Replace(":", "_");
        safe.Replace("*", "_");
        safe.Replace("?", "_");
        safe.Replace("\"", "_");
        safe.Replace("<", "_");
        safe.Replace(">", "_");
        safe.Replace("|", "_");
        safe.Replace(" ", "_");
        return safe;
    }
}
