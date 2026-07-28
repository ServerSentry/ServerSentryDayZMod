/**
	\file ServerSentryShopSpawnBundleAction.c
	\brief Implements shop fulfillment support for Shop spawn bundle action.
*/

/**
	\brief Represents parsed input for Shop action metadata.
*/
class ServerSentryShopActionMetadataInput
{
    /**
    	\brief Stores the source.
    */
    string source;
    /**
    	\brief Stable key used to make product operations idempotent.
    */
    string fulfillmentKey;
    /**
    	\brief Unique ID of the incoming action request.
    */
    string requestId;
    /**
    	\brief Stores the order ID.
    */
    string orderId;
    /**
    	\brief Stores the product ID.
    */
    string productId;
    /**
    	\brief Stores the product name.
    */
    string productName;
    /**
    	\brief Stores the shop action ID.
    */
    string shopActionId;
    /**
    	\brief Stores the mod action ID.
    */
    string modActionId;
    /**
    	\brief Stores the day z server ID.
    */
    string dayZServerId;
    /**
    	\brief Stores the steam ID 64.
    */
    string steamId64;
    /**
    	\brief Stores the fulfillment action.
    */
    string fulfillmentAction;
    /**
    	\brief Indicates whether is cancellation.
    */
    bool isCancellation;
}

/**
	\brief Represents parsed input for Shop spawn bundle item.
*/
class ServerSentryShopSpawnBundleItemInput
{
    /**
    	\brief Stores the class name.
    */
    string className;
    /**
    	\brief Stores the quantity.
    */
    int quantity;
    /**
    	\brief Collection of attachments values.
    */
    ref array<string> attachments;

    /**
    	\brief Initializes a new ServerSentryShopSpawnBundleItemInput instance.
    */
    void ServerSentryShopSpawnBundleItemInput()
    {
        className = "";
        quantity = 1;
        attachments = new array<string>();
    }
}

/**
	\brief Represents parsed input for Shop spawn bundle.
*/
class ServerSentryShopSpawnBundleInput
{
    /**
    	\brief Collection of items values.
    */
    ref array<ref ServerSentryShopSpawnBundleItemInput> items;
    /**
    	\brief Indicates whether fallback to ground.
    */
    bool fallbackToGround;
    /**
    	\brief Lookup table for labels values.
    */
    ref map<string, string> labels;
    /**
    	\brief Stores the server sentry.
    */
    ref ServerSentryShopActionMetadataInput _serverSentry;

    /**
    	\brief Initializes a new ServerSentryShopSpawnBundleInput instance.
    */
    void ServerSentryShopSpawnBundleInput()
    {
        items = new array<ref ServerSentryShopSpawnBundleItemInput>();
        fallbackToGround = true;
        labels = new map<string, string>();
        _serverSentry = null;
    }
}

/**
	\brief Implements the Shop spawn bundle ServerSentry action.
*/
class ServerSentryShopSpawnBundleAction: ServerSentryModActionHandler
{
    /**
    	\brief Returns the metadata descriptor published for this live action.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto d = new ServerSentryLiveModActionDescriptorDto();
        d.id = "shop.spawn_bundle";
        d.targetType = "player";
        d.label = "Spawn Item Bundle";
        d.icon = "PackageOpen";
        d.description = "Spawns a typed list of items and attachments for a player. Intended as a shop and admin example action.";
        d.scopes.Insert("shop");
        d.scopes.Insert("live");

        array<ref ServerSentryLiveModActionInputDto> itemFields = new array<ref ServerSentryLiveModActionInputDto>();
        ServerSentryLiveModActionInputDto className = ServerSentryLiveModActionInputFactory.Text("className", "Item classname", true, "e.g. M4A1");
        className.helpText = "DayZ classname to create.";
        itemFields.Insert(className);

        ServerSentryLiveModActionInputDto quantity = ServerSentryLiveModActionInputFactory.Integer("quantity", "Quantity", true, 1, 999999, 1, "1");
        quantity.helpText = "Applied to quantity-based items. Non-stackable items should use 1.";
        itemFields.Insert(quantity);

        ServerSentryLiveModActionInputDto attachmentItem = ServerSentryLiveModActionInputFactory.Text("attachment", "Attachment classname", true, "e.g. M4_RISHndgrd");
        ServerSentryLiveModActionInputDto attachments = ServerSentryLiveModActionInputFactory.List("attachments", "Attachments", attachmentItem, false, 0, 50, "[]");
        itemFields.Insert(attachments);

        ServerSentryLiveModActionInputDto itemObject = ServerSentryLiveModActionInputFactory.Object("item", "Bundle item", itemFields, true, "{\"className\":\"Apple\",\"quantity\":1,\"attachments\":[]}");

        ServerSentryLiveModActionInputDto items = ServerSentryLiveModActionInputFactory.List("items", "Bundle items", itemObject, true, 1, 100, "[{\"className\":\"Apple\",\"quantity\":2,\"attachments\":[]}]");
        items.helpText = "An array<object> where every item follows the same typed schema.";
        d.inputs.Insert(items);

        d.inputs.Insert(ServerSentryLiveModActionInputFactory.Boolean("fallbackToGround", "Spawn at feet when inventory is full", true, "When disabled, the action fails if an item cannot be placed in inventory."));

        ServerSentryLiveModActionMapKeyDto labelKey = ServerSentryLiveModActionInputFactory.StringMapKey("Label key", "e.g. bundle");
        labelKey.maxLength = 80;
        ServerSentryLiveModActionInputDto labelValue = ServerSentryLiveModActionInputFactory.Text("label", "Label value", true, "e.g. Starter Kit");
        ServerSentryLiveModActionInputDto labels = ServerSentryLiveModActionInputFactory.MapInput("labels", "Optional labels", labelKey, labelValue, false, 0, 50, "{}");
        labels.helpText = "Example map<string, string> metadata available to the action.";
        d.inputs.Insert(labels);

        return d;
    }

    /**
    	\brief Executes the live action using the supplied request context.
    	\param ctx \p action context containing request, target, and input data.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    override bool Execute(ServerSentryModActionContext ctx, out string message)
    {
        PlayerBase player = ServerSentryLiveActionResolver.FindPlayer(ctx);
        if (!player)
        {
            message = "Player not found or offline.";
            return false;
        }

        ServerSentryShopSpawnBundleInput input;
        string inputError;
        if (!JsonFileLoader<ServerSentryShopSpawnBundleInput>.LoadData(ctx.inputJson, input, inputError))
        {
            message = "Invalid bundle input JSON: " + inputError;
            return false;
        }

        if (!input || !input.items || input.items.Count() == 0)
        {
            message = "At least one bundle item is required.";
            return false;
        }

        int spawnedCount = 0;
        int groundCount = 0;
        int attachmentCount = 0;

        foreach (ServerSentryShopSpawnBundleItemInput item: input.items)
        {
            if (!item)
            {
                continue;
            }

            item.className.TrimInPlace();
            if (item.className == "")
            {
                message = "A bundle item is missing its classname.";
                return false;
            }

            EntityAI spawned = player.GetInventory().CreateInInventory(item.className);
            bool spawnedOnGround = false;
            if (!spawned && input.fallbackToGround)
            {
                spawned = EntityAI.Cast(GetGame().CreateObjectEx(item.className, player.GetPosition(), ECE_PLACE_ON_SURFACE));
                spawnedOnGround = true;
            }

            if (!spawned)
            {
                message = "Failed to spawn item class " + item.className + ".";
                return false;
            }

            ApplyQuantity(spawned, item.quantity);
            spawnedCount++;
            if (spawnedOnGround)
            {
                groundCount++;
            }

            if (item.attachments)
            {
                foreach (string attachmentClass: item.attachments)
                {
                    attachmentClass.TrimInPlace();
                    if (attachmentClass == "")
                    {
                        continue;
                    }

                    EntityAI attachment = spawned.GetInventory().CreateAttachment(attachmentClass);
                    if (attachment)
                    {
                        attachmentCount++;
                    }
                }
            }
        }

        message = "Spawned bundle items=" + spawnedCount.ToString() + ", attachments=" + attachmentCount.ToString() + ", groundFallbacks=" + groundCount.ToString() + ".";
        return true;
    }

    /**
    	\brief Applies quantity.
    	\param entity \p entity processed by the operation.
    	\param quantity \p quantity used by the operation.
    */
    protected void ApplyQuantity(EntityAI entity, int quantity)
    {
        if (!entity || quantity <= 1)
        {
            return;
        }

        ItemBase itemBase = ItemBase.Cast(entity);
        if (itemBase)
        {
            itemBase.SetQuantity(quantity);
        }
    }
}
