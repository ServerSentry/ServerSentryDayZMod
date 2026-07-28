/**
	\file ServerSentryShopBankDepositAction.c
	\brief Implements shop fulfillment support for Shop bank deposit action.
*/

/**
	\brief Implements the Shop bank deposit ServerSentry action.
*/
class ServerSentryShopBankDepositAction: ServerSentryModActionHandler
{
    /**
    	\brief Returns the metadata descriptor published for this live action.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        string descriptorDescription = "Shop fulfillment action. Credits configured money to the buyer bank account.";
        ServerSentryLiveModActionDescriptorDto descriptor = ServerSentryLiveModActionDescriptorFactory.ShopPlayer("shop.player.bank.deposit", "Shop: Bank Deposit");
        descriptor.icon = "Landmark";
        descriptor.description = descriptorDescription;

        string amountHelp = "Money amount to credit to the buyer bank account.";
        ServerSentryLiveModActionInputDto amountInput = ServerSentryLiveModActionInputFactory.Number("amount", "Amount");
        amountInput.required = true;
        amountInput.min = 1;
        amountInput.max = 999999999;
        amountInput.step = 1;
        amountInput.defaultValue = "50000";
        amountInput.helpText = amountHelp;
        descriptor.AddInput(amountInput);

        string reasonHelp = "Optional reason for your bank transaction log.";
        ServerSentryLiveModActionInputDto reasonInput = ServerSentryLiveModActionInputFactory.Text("reason", "Reason");
        reasonInput.placeholder = "Shop purchase";
        reasonInput.helpText = reasonHelp;
        descriptor.AddInput(reasonInput);

        return descriptor;
    }

    /**
    	\brief Executes the live action using the supplied request context.
    	\param ctx \p action context containing request, target, and input data.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    override bool Execute(ServerSentryModActionContext ctx, out string message)
    {
        if (!ctx)
        {
            message = "Missing action context.";
            return false;
        }

        int amount = ctx.GetInt("amount", 0);
        string reason = ctx.GetString("reason", "Shop purchase");
        if (amount <= 0)
        {
            message = "Invalid bank deposit amount.";
            return false;
        }

        ServerSentryShopFulfillmentOperation fulfillment;
        int beginStatus = ServerSentryShopFulfillment.Begin(ctx, "", reason, fulfillment, message, true, amount);
        if (beginStatus == ServerSentryShopFulfillment.BEGIN_ALREADY_FULFILLED)
        {
            return true;
        }
        if (beginStatus != ServerSentryShopFulfillment.BEGIN_READY)
        {
            return false;
        }

        string bankMessage;
        bool granted = GrantBankMoney(ctx, amount, reason, fulfillment.fulfillmentKey, bankMessage);
        if (!ServerSentryShopFulfillment.Finish(fulfillment, granted, bankMessage, message))
        {
            return false;
        }

        message = "Credited bank account for SteamID " + ctx.steamId + " with " + amount.ToString() + ". " + message;
        return true;
    }

    /**
    	\brief Processes the grant bank money operation.
    	\param ctx \p action context containing request, target, and input data.
    	\param amount \p amount applied by the operation.
    	\param reason \p human-readable operation description stored with fulfillment state.
    	\param fulfillmentKey \p stable product-operation key used for idempotency.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    protected bool GrantBankMoney(ServerSentryModActionContext ctx, int amount, string reason, string fulfillmentKey, out string message)
    {
        message = "";

        string steamId = ctx.steamId;
        if (steamId == "")
        {
            message = "Missing buyer SteamID64.";
            return false;
        }

        // TODO: Wire your real bank mod here.
        // Preferred fail-safe shape:
        // YourBankApi.DepositBySteamId(steamId, amount, fulfillmentKey, reason);
        // Do not return true until the bank mod really accepted the credit.
        Print("[ServerSentry][ShopBank] Bank adapter not configured. steamId=" + steamId + " amount=" + amount.ToString() + " fulfillmentKey=" + fulfillmentKey);
        message = "Bank adapter not configured. Add your bank mod deposit API call in GrantBankMoney().";
        return false;
    }
}
