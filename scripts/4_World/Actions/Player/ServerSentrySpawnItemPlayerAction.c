/**
	\file ServerSentrySpawnItemPlayerAction.c
	\brief Implements live-action support for Spawn item player action.
*/

/**
	\brief Implements the Spawn item player ServerSentry action.
*/
class ServerSentrySpawnItemPlayerAction: ServerSentryModActionHandler
{
    /**
    	\brief Returns the metadata descriptor published for this live action.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto d = new ServerSentryLiveModActionDescriptorDto();
        d.id = "player.spawn_item";
        d.targetType = "player";
        d.label = "Spawn Item";
        d.icon = "PackagePlus";
        d.description = "Spawns an item into the selected player's inventory. If the inventory is full, the item is spawned at the player's feet.";

        // Live/Admin-only. Shop products must use explicit shop.* actions.
        if (!d.scopes)
        {
            d.scopes = new array<string>();
        }
        d.scopes.Insert("live");

        ServerSentryLiveModActionInputDto itemClass = ServerSentryLiveModActionInputFactory.Text("itemClass", "Item class", true, "e.g. Apple, M4A1, BandageDressing");
        itemClass.helpText = "DayZ classname of the item to spawn.";
        d.inputs.Insert(itemClass);

        ServerSentryLiveModActionInputDto quantity = ServerSentryLiveModActionInputFactory.Number("quantity", "Quantity", false, 1, 999999, 1, "1");
        quantity.helpText = "For stackable/quantity items this sets the item quantity. For normal items use 1.";
        d.inputs.Insert(quantity);

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

        string itemClass = ServerSentryLiveActionInput.GetString(ctx.inputJson, "itemClass", "");
        itemClass.TrimInPlace();
        if (itemClass == "")
        {
            message = "Missing item class.";
            return false;
        }

        int quantity = ServerSentryLiveActionInput.GetInt(ctx.inputJson, "quantity", 1);
        if (quantity < 1)
        {
            quantity = 1;
        }

        EntityAI spawned = player.GetInventory().CreateInInventory(itemClass);
        bool spawnedInInventory = true;

        if (!spawned)
        {
            vector spawnPosition = player.GetPosition();
            spawned = EntityAI.Cast(GetGame().CreateObjectEx(itemClass, spawnPosition, ECE_PLACE_ON_SURFACE));
            spawnedInInventory = false;
        }

        if (!spawned)
        {
            message = "Failed to spawn item class: " + itemClass;
            return false;
        }

        ApplyQuantity(spawned, quantity);

        if (spawnedInInventory)
        {
            message = "Spawned " + itemClass + " x" + quantity.ToString() + " in player inventory.";
        }
        else
        {
            message = "Inventory full or item cannot be placed there. Spawned " + itemClass + " x" + quantity.ToString() + " at player position.";
        }

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
