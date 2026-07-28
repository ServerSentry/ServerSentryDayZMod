/**
	\file ServerSentrySpawnItemAtPositionAction.c
	\brief Implements live-action support for Spawn item at position action.
*/

/**
	\brief Implements the Spawn item at position ServerSentry action.
*/
class ServerSentrySpawnItemAtPositionAction: ServerSentryModActionHandler
{
    /**
    	\brief Returns the metadata descriptor published for this live action.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto d = new ServerSentryLiveModActionDescriptorDto();
        d.id = "world.item.spawn_at";
        d.targetType = "world";
        d.label = "Spawn Item";
        d.icon = "PackagePlus";
        d.dangerous = true;
        d.confirmText = "Spawn this item at the provided world coordinates?";
        d.description = "Spawns an item at explicit world coordinates.";
        d.scopes.Insert("live");

        ServerSentryLiveModActionInputDto itemClass = ServerSentryLiveModActionInputFactory.Text("itemClass", "Item class", true, "e.g. Apple, M4A1, BandageDressing");
        itemClass.helpText = "DayZ classname of the item to spawn.";
        d.inputs.Insert(itemClass);

        ServerSentryLiveModActionInputDto quantity = ServerSentryLiveModActionInputFactory.Number("quantity", "Quantity", false, 1, 999999, 1, "1");
        quantity.helpText = "For stackable/quantity items this sets the item quantity. For normal items use 1.";
        d.inputs.Insert(quantity);

        ServerSentryLiveModActionInputDto x = ServerSentryLiveModActionInputFactory.Number("x", "X coordinate", true, 0, 0, 1, "");
        x.helpText = "World X coordinate.";
        d.inputs.Insert(x);

        ServerSentryLiveModActionInputDto z = ServerSentryLiveModActionInputFactory.Number("z", "Z coordinate", true, 0, 0, 1, "");
        z.helpText = "World Z coordinate.";
        d.inputs.Insert(z);

        ServerSentryLiveModActionInputDto y = ServerSentryLiveModActionInputFactory.Number("y", "Y coordinate", false, 0, 0, 0.1, "0");
        y.helpText = "Optional height. Use 0 to place the item on the surface.";
        d.inputs.Insert(y);

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

        float x = ServerSentryLiveActionInput.GetFloat(ctx.inputJson, "x", 0);
        float y = ServerSentryLiveActionInput.GetFloat(ctx.inputJson, "y", 0);
        float z = ServerSentryLiveActionInput.GetFloat(ctx.inputJson, "z", 0);

        if (ctx && ctx.position)
        {
            if (x == 0 && z == 0 && (ctx.position.x != 0 || ctx.position.z != 0))
            {
                x = ctx.position.x;
                z = ctx.position.z;
            }

            if (y == 0 && ctx.position.y != 0)
            {
                y = ctx.position.y;
            }
        }

        if (x < 0 || z < 0)
        {
            message = "Invalid coordinates. X and Z must be greater than or equal to 0.";
            return false;
        }

        if (y <= 0)
        {
            y = GetGame().SurfaceY(x, z);
        }

        vector spawnPosition = Vector(x, y + 0.05, z);
        EntityAI spawned = EntityAI.Cast(GetGame().CreateObjectEx(itemClass, spawnPosition, ECE_PLACE_ON_SURFACE));
        if (!spawned)
        {
            message = "Failed to spawn item class: " + itemClass + ".";
            return false;
        }

        ApplyQuantity(spawned, quantity);

        message = "Spawned " + itemClass + " x" + quantity.ToString() + " at X " + Math.Round(x).ToString() + " / Z " + Math.Round(z).ToString() + ".";
        ServerSentryLiveApi.WriteSnapshot();
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
