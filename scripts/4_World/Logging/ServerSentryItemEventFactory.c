/**
	\file ServerSentryItemEventFactory.c
	\brief Provides logging support for Item event factory.
*/

/**
	\brief Creates values used by Item event.
*/
class ServerSentryItemEventFactory
{
    /**
    	\brief Creates item move.
    	\param item \p item or value processed by the operation.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    	\param sourceHook \p source hook used by the operation.
    	\param traceId \p identifier of the trace.
    	\return \p The resulting event instance, or null when unavailable.
    */
    static ServerSentryEvent CreateItemMove(EntityAI item, InventoryLocation oldLoc, InventoryLocation newLoc, string sourceHook = "unknown", int traceId = 0)
    {
        ServerSentryInventoryDebug.PrintTraceLocations(traceId, "ItemEventFactory.CreateItemMove.enter", item, oldLoc, newLoc);

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMove.newEvent.before");
        ServerSentryItemMoveEvent ev = new ServerSentryItemMoveEvent();
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMove.newEvent.after", "ev=" + ServerSentryInventoryDebug.BoolToString(ev != null));

        ev.schemaVersion = ServerSentryDefaults.SchemaVersion();
        ev.eventType = "item_move";
        ev.category = "item";
        ev.severity = "info";
        ev.mod = "ServerSentry";

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.beforeResolveItemKind");
        string itemKind = ResolveItemKind(item, traceId);
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.afterResolveItemKind", itemKind);

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.beforeResolveItemMoveAction");
        string actionName = ResolveItemMoveAction(item, itemKind, oldLoc, newLoc, traceId);
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.afterResolveItemMoveAction", actionName);

        ev.action.name = actionName;
        ev.action.displayName = actionName;
        ev.action.result = "success";

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.beforeFillItem");
        FillItem(ev.item, item, traceId);
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.afterFillItem");

        string actorInference;
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.beforeResolveItemMoveActor");
        PlayerBase actorPlayer = ResolveItemMoveActor(oldLoc, newLoc, actorInference, traceId);
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.afterResolveItemMoveActor", "actor=" + ServerSentryInventoryDebug.PlayerInfo(actorPlayer) + " inference=" + actorInference);

        if (actorPlayer)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.beforeFillPlayerActor");
            ServerSentryEntityClassifier.FillPlayer(ev.actor, actorPlayer, actorPlayer.GetIdentity());
            ev.position.FromVector(actorPlayer.GetPosition());
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.afterFillPlayerActor", "actorName=" + ev.actor.name + " steam=" + ev.actor.steamId + " pos=" + actorPlayer.GetPosition().ToString());
        }
        else if (item)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.beforeFillEnvironmentActor", "unknown");
            ServerSentryEntityClassifier.FillEnvironment(ev.actor, "unknown");
            ev.position.FromVector(item.GetPosition());
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.afterFillEnvironmentActor", "pos=" + item.GetPosition().ToString());
        }
        else
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.actorSkipped", "item=null actor=null");
        }

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.beforeResolveItemMoveTargetEntity");
        EntityAI targetEntity = ResolveItemMoveTargetEntity(actionName, oldLoc, newLoc, traceId);
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.afterResolveItemMoveTargetEntity", "target=" + ServerSentryInventoryDebug.EntityInfo(targetEntity));

        if (targetEntity)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.beforeFillEntityTarget");
            ServerSentryEntityClassifier.FillEntity(ev.target, targetEntity);
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.afterFillEntityTarget", "targetClass=" + ev.target.className + " targetName=" + ev.target.name);
        }
        else
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.beforeFillGroundTarget");
            ServerSentryEntityClassifier.FillEnvironment(ev.target, "ground");
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.afterFillGroundTarget");
        }

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.beforeCreateItemMoveData");
        ServerSentryItemMoveData data = CreateItemMoveData(item, itemKind, oldLoc, newLoc, ev.actor, actorInference, sourceHook, traceId);
        ev.data = data;
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.afterCreateItemMoveData", "data=" + ServerSentryInventoryDebug.BoolToString(data != null));

        ev.dataJson = "";

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.beforeBuildItemMoveMessage");
        ev.message = BuildItemMoveMessage(actionName, item, data, traceId);
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.afterBuildItemMoveMessage", ev.message);

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMove.done", "eventType=" + ev.eventType + " action=" + ev.action.name);
        return ev;
    }


    /**
    	\brief Processes the fill item operation.
    	\param output \p output used by the operation.
    	\param item \p item or value processed by the operation.
    	\param traceId \p identifier of the trace.
    */
    static void FillItem(ServerSentryItem output, EntityAI item, int traceId = 0)
    {
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.FillItem.enter", ServerSentryInventoryDebug.ItemInfo(item));

        if (!output)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.FillItem.skip", "output=null");
            return;
        }

        if (!item)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.FillItem.skip", "item=null");
            return;
        }

        output.className = item.GetType();
        output.displayName = item.GetDisplayName();
        output.persistentId = ServerSentryItemMoveScriptLog.ResolveItemPersistentId(item);
        output.position = new ServerSentryVectorDto();
        output.position.FromVector(item.GetPosition());
        output.health = item.GetHealth("", "Health");
        output.quantity = 0;
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.FillItem.base", "class=" + output.className + " name=" + output.displayName + " health=" + output.health.ToString());

        ItemBase itemBase = ItemBase.Cast(item);
        if (itemBase)
        {
            output.quantity = itemBase.GetQuantity();
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.FillItem.quantity.ItemBase", output.quantity.ToString());
            return;
        }

        Magazine magazine = Magazine.Cast(item);
        if (magazine)
        {
            output.quantity = magazine.GetAmmoCount();
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.FillItem.quantity.Magazine", output.quantity.ToString());
            return;
        }

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.FillItem.done", "no quantity provider");
    }

    /**
    	\brief Creates item move data.
    	\param item \p item or value processed by the operation.
    	\param itemKind \p item kind used by the operation.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    	\param actor \p actor used by the operation.
    	\param actorInference \p actor inference used by the operation.
    	\param sourceHook \p source hook used by the operation.
    	\param traceId \p identifier of the trace.
    	\return \p The resulting item move data instance, or null when unavailable.
    */
    static ServerSentryItemMoveData CreateItemMoveData(EntityAI item, string itemKind, InventoryLocation oldLoc, InventoryLocation newLoc, ServerSentryActor actor, string actorInference, string sourceHook, int traceId = 0)
    {
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMoveData.enter", "itemKind=" + itemKind + " actorInference=" + actorInference + " sourceHook=" + sourceHook);
        ServerSentryItemMoveData data = new ServerSentryItemMoveData();

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMoveData.beforeBuildItemMoveId");
        data.moveId = BuildItemMoveId(item, oldLoc, newLoc, traceId);
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMoveData.afterBuildItemMoveId", data.moveId);

        data.sourceHook = sourceHook;
        data.itemKind = itemKind;
        if (item)
        {
            data.runtimeItemId = item.GetID();
            data.itemPersistentId = ServerSentryItemMoveScriptLog.ResolveItemPersistentId(item);
        }
        data.actorInference = actorInference;

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMoveData.beforeFromLocation");
        data.from = CreateItemLocationData(oldLoc, traceId, "from");
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMoveData.afterFromLocation", "fromType=" + data.from.type + " raw=" + data.from.raw);

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMoveData.beforeToLocation");
        data.to = CreateItemLocationData(newLoc, traceId, "to");
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMoveData.afterToLocation", "toType=" + data.to.type + " raw=" + data.to.raw);

        data.relation = data.from.type + "_to_" + data.to.type;
        data.countsForPlayerItemStats = actor && actor.type == "player";
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMoveData.relation", data.relation + " counts=" + ServerSentryInventoryDebug.BoolToString(data.countsForPlayerItemStats));

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMoveData.beforeFillStorageSummary");
        FillStorageSummary(data, traceId);
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMoveData.afterFillStorageSummary");

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMoveData.beforeBuildFromLabel");
        data.fromLabel = BuildLocationLabel(data.from, traceId, "from");
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMoveData.afterBuildFromLabel", data.fromLabel);

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMoveData.beforeBuildToLabel");
        data.toLabel = BuildLocationLabel(data.to, traceId, "to");
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMoveData.afterBuildToLabel", data.toLabel);

        if (item)
        {
            data.itemClassName = item.GetType();
            data.itemDisplayName = item.GetDisplayName();
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMoveData.item", data.itemClassName + " / " + data.itemDisplayName);
        }
        else
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMoveData.item", "item=null");
        }

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemMoveData.done");
        return data;
    }


    /**
    	\brief Creates item location data.
    	\param loc \p loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\param label \p human-readable label shown to users.
    	\return \p The resulting item location data instance, or null when unavailable.
    */
    static ServerSentryItemLocationData CreateItemLocationData(InventoryLocation loc, int traceId = 0, string label = "")
    {
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.enter." + label, ServerSentryInventoryDebug.LocationInfo(loc));
        ServerSentryItemLocationData data = new ServerSentryItemLocationData();

        if (!loc)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.return." + label, "loc=null");
            return data;
        }

        if (!loc.IsValid())
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.return." + label, "loc=invalid");
            return data;
        }

        data.typeId = loc.GetType();
        data.type = ResolveInventoryLocationTypeName(data.typeId, traceId);
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.type." + label, data.type + " / " + data.typeId.ToString());

        if (ServerSentryInventoryLifecycle.IsExternalStorageLocation(loc, traceId, label + ".externalCheck"))
        {
            data.type = "storage";
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.storageType." + label);
        }

        EntityAI parent = loc.GetParent();
        if (parent)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.parent." + label, ServerSentryInventoryDebug.EntityInfo(parent));
            data.parentClassName = parent.GetType();
            data.parentName = parent.GetDisplayName();

            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.beforeGetPlayerFromEntity." + label);
            PlayerBase rootPlayer = ServerSentryInventoryLifecycle.GetPlayerFromEntity(parent, traceId, label + ".parentOwner");
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.afterGetPlayerFromEntity." + label, ServerSentryInventoryDebug.PlayerInfo(rootPlayer));
            if (rootPlayer)
            {
                ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.beforeFillOwner." + label);
                ServerSentryActor owner = new ServerSentryActor();
                ServerSentryEntityClassifier.FillPlayer(owner, rootPlayer, rootPlayer.GetIdentity());
                data.parentOwnerType = owner.type;
                data.parentOwnerKind = owner.entityKind;
                data.parentOwnerClassName = owner.className;
                data.parentOwnerSteamId = owner.steamId;
                data.parentOwnerBohemiaId = owner.bohemiaId;
                data.parentOwnerName = owner.name;
                ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.afterFillOwner." + label, "owner=" + data.parentOwnerName + " steam=" + data.parentOwnerSteamId);
            }
        }
        else
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.parent." + label, "null");
        }

        // Only read fields that are valid for the concrete InventoryLocationType.
        // Reading cargo fields from ATTACHMENT locations can crash on slot moves
        // such as dragging a weapon to shoulder/back slots.
        if (data.typeId == InventoryLocationType.ATTACHMENT)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.attachment.beforeGetPlayerFromLocation." + label);
            PlayerBase slotOwner = ServerSentryInventoryLifecycle.GetPlayerFromLocation(loc, traceId, label + ".slotOwner");
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.attachment.afterGetPlayerFromLocation." + label, ServerSentryInventoryDebug.PlayerInfo(slotOwner));
            if (slotOwner)
            {
                data.type = "player_slot";
            }
            else
            {
                data.type = "attachment";
            }

            data.slotId = loc.GetSlot();
            if (InventorySlots.IsSlotIdValid(data.slotId))
            {
                data.slotName = InventorySlots.GetSlotName(data.slotId);
                // Slot display names are nice-to-have only. Avoid extra lookup risk on
                // modded slots; slotId + slotName are enough for audit/debug.
                data.slotDisplayName = "";
            }
            else
            {
                data.slotName = "";
                data.slotDisplayName = "";
            }
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.attachment.done." + label, "slotId=" + data.slotId.ToString() + " slotName=" + data.slotName + " type=" + data.type);
        }

        if (data.typeId == InventoryLocationType.CARGO || data.typeId == InventoryLocationType.PROXYCARGO)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.cargo.beforeRead." + label);
            data.cargoIdx = loc.GetIdx();
            data.row = loc.GetRow();
            data.col = loc.GetCol();
            data.flip = loc.GetFlip();
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.cargo.afterRead." + label, "idx=" + data.cargoIdx.ToString() + " row=" + data.row.ToString() + " col=" + data.col.ToString() + " flip=" + ServerSentryInventoryDebug.BoolToString(data.flip));
        }

        if (data.typeId == InventoryLocationType.GROUND)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.position.beforeGround." + label);
            data.position.FromVector(loc.GetPos());
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.position.afterGround." + label, loc.GetPos().ToString());
        }
        else if (parent)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.position.beforeParent." + label);
            data.position.FromVector(parent.GetPosition());
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.position.afterParent." + label, parent.GetPosition().ToString());
        }

        // Avoid DumpToString() here. Some weapon drop paths can pass engine-owned
        // InventoryLocation objects that are not safe to stringify after vanilla processing.
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.beforeBuildRaw." + label);
        data.raw = ServerSentryInventoryLifecycle.BuildLocationSignature(loc, traceId, label + ".raw");
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.afterBuildRaw." + label, data.raw);
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.CreateItemLocationData.done." + label, "type=" + data.type + " parent=" + data.parentClassName);
        return data;
    }

    /**
    	\brief Resolves inventory location type name.
    	\param typeId \p type identifier used by the operation.
    	\param traceId \p identifier of the trace.
    	\return \p The resulting string value.
    */
    static string ResolveInventoryLocationTypeName(int typeId, int traceId = 0)
    {
        string result = "unknown";

        if (typeId == InventoryLocationType.GROUND)
        {
            result = "ground";
        }
        else if (typeId == InventoryLocationType.ATTACHMENT)
        {
            result = "attachment";
        }
        else if (typeId == InventoryLocationType.CARGO)
        {
            result = "cargo";
        }
        else if (typeId == InventoryLocationType.HANDS)
        {
            result = "hands";
        }
        else if (typeId == InventoryLocationType.PROXYCARGO)
        {
            result = "proxy_cargo";
        }

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveInventoryLocationTypeName", "typeId=" + typeId.ToString() + " result=" + result);
        return result;
    }

    /**
    	\brief Resolves item move action.
    	\param item \p item or value processed by the operation.
    	\param itemKind \p item kind used by the operation.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\return \p The resulting string value.
    */
    static string ResolveItemMoveAction(EntityAI item, string itemKind, InventoryLocation oldLoc, InventoryLocation newLoc, int traceId = 0)
    {
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveAction.enter", "itemKind=" + itemKind);
        bool fromGround = ServerSentryInventoryLifecycle.IsGroundLocation(oldLoc, traceId, "action.old");
        bool toGround = ServerSentryInventoryLifecycle.IsGroundLocation(newLoc, traceId, "action.new");
        bool fromStorage = ServerSentryInventoryLifecycle.IsExternalStorageLocation(oldLoc, traceId, "action.old");
        bool toStorage = ServerSentryInventoryLifecycle.IsExternalStorageLocation(newLoc, traceId, "action.new");

        PlayerBase oldPlayer = ServerSentryInventoryLifecycle.GetPlayerFromLocation(oldLoc, traceId, "action.old");
        PlayerBase newPlayer = ServerSentryInventoryLifecycle.GetPlayerFromLocation(newLoc, traceId, "action.new");

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveAction.flags", "fromGround=" + ServerSentryInventoryDebug.BoolToString(fromGround) + " toGround=" + ServerSentryInventoryDebug.BoolToString(toGround) + " fromStorage=" + ServerSentryInventoryDebug.BoolToString(fromStorage) + " toStorage=" + ServerSentryInventoryDebug.BoolToString(toStorage) + " oldPlayer=" + ServerSentryInventoryDebug.BoolToString(oldPlayer != null) + " newPlayer=" + ServerSentryInventoryDebug.BoolToString(newPlayer != null));

        if (fromGround && newPlayer)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveAction.result", "item_pickup");
            return "item_pickup";
        }

        if (oldPlayer && toGround)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveAction.result", "item_drop");
            return "item_drop";
        }

        if (oldPlayer && toStorage)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveAction.result", "storage_put");
            return "storage_put";
        }

        if (fromStorage && newPlayer)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveAction.result", "storage_take");
            return "storage_take";
        }

        if (fromStorage && toStorage)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveAction.result", "storage_transfer");
            return "storage_transfer";
        }

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveAction.result", "item_move");
        return "item_move";
    }

    /**
    	\brief Resolves item move actor.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    	\param actorInference \p receives the actor inference used by the operation.
    	\param traceId \p identifier of the trace.
    	\return \p The resulting player base instance, or null when unavailable.
    */
    static PlayerBase ResolveItemMoveActor(InventoryLocation oldLoc, InventoryLocation newLoc, out string actorInference, int traceId = 0)
    {
        actorInference = "unknown";
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveActor.enter");

        PlayerBase oldPlayer = ServerSentryInventoryLifecycle.GetPlayerFromLocation(oldLoc, traceId, "actor.old");
        PlayerBase newPlayer = ServerSentryInventoryLifecycle.GetPlayerFromLocation(newLoc, traceId, "actor.new");

        if (ServerSentryInventoryLifecycle.IsGroundLocation(oldLoc, traceId, "actor.old") && newPlayer)
        {
            actorInference = "new_owner_from_ground";
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveActor.result", "new_owner_from_ground " + ServerSentryInventoryDebug.PlayerInfo(newPlayer));
            return newPlayer;
        }

        if (oldPlayer && ServerSentryInventoryLifecycle.IsGroundLocation(newLoc, traceId, "actor.new"))
        {
            actorInference = "old_owner_to_ground";
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveActor.result", "old_owner_to_ground " + ServerSentryInventoryDebug.PlayerInfo(oldPlayer));
            return oldPlayer;
        }

        if (oldPlayer && ServerSentryInventoryLifecycle.IsExternalStorageLocation(newLoc, traceId, "actor.new"))
        {
            actorInference = "old_owner_to_storage";
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveActor.result", "old_owner_to_storage " + ServerSentryInventoryDebug.PlayerInfo(oldPlayer));
            return oldPlayer;
        }

        if (ServerSentryInventoryLifecycle.IsExternalStorageLocation(oldLoc, traceId, "actor.old") && newPlayer)
        {
            actorInference = "new_owner_from_storage";
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveActor.result", "new_owner_from_storage " + ServerSentryInventoryDebug.PlayerInfo(newPlayer));
            return newPlayer;
        }

        if (newPlayer)
        {
            actorInference = "new_owner";
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveActor.result", "new_owner " + ServerSentryInventoryDebug.PlayerInfo(newPlayer));
            return newPlayer;
        }

        if (oldPlayer)
        {
            actorInference = "old_owner";
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveActor.result", "old_owner " + ServerSentryInventoryDebug.PlayerInfo(oldPlayer));
            return oldPlayer;
        }

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveActor.result", "null");
        return null;
    }

    /**
    	\brief Resolves item move target entity.
    	\param actionName \p action name used by the operation.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\return \p The resulting entity ai instance, or null when unavailable.
    */
    static EntityAI ResolveItemMoveTargetEntity(string actionName, InventoryLocation oldLoc, InventoryLocation newLoc, int traceId = 0)
    {
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveTargetEntity.enter", "action=" + actionName);
        // For storage audit events, the top-level target should be the storage object involved.
        // The player slot/backpack destination remains available in dataJson.to.
        if (actionName == "storage_take")
        {
            EntityAI storageTakeTarget = ResolveLocationParent(oldLoc, traceId, "storage_take.old");
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveTargetEntity.result", ServerSentryInventoryDebug.EntityInfo(storageTakeTarget));
            return storageTakeTarget;
        }

        if (actionName == "storage_put")
        {
            EntityAI storagePutTarget = ResolveLocationParent(newLoc, traceId, "storage_put.new");
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveTargetEntity.result", ServerSentryInventoryDebug.EntityInfo(storagePutTarget));
            return storagePutTarget;
        }

        if (actionName == "storage_transfer")
        {
            EntityAI storageTransferTarget = ResolveLocationParent(oldLoc, traceId, "storage_transfer.old");
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveTargetEntity.result", ServerSentryInventoryDebug.EntityInfo(storageTransferTarget));
            return storageTransferTarget;
        }

        EntityAI defaultTarget = ResolveLocationParent(newLoc, traceId, "default.new");
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemMoveTargetEntity.result", ServerSentryInventoryDebug.EntityInfo(defaultTarget));
        return defaultTarget;
    }

    /**
    	\brief Resolves location parent.
    	\param loc \p loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\param label \p human-readable label shown to users.
    	\return \p The resulting entity ai instance, or null when unavailable.
    */
    static EntityAI ResolveLocationParent(InventoryLocation loc, int traceId = 0, string label = "")
    {
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveLocationParent.enter." + label, ServerSentryInventoryDebug.LocationInfo(loc));
        if (!loc)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveLocationParent.result." + label, "loc=null");
            return null;
        }

        if (!loc.IsValid())
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveLocationParent.result." + label, "loc=invalid");
            return null;
        }

        EntityAI parent = loc.GetParent();
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveLocationParent.result." + label, ServerSentryInventoryDebug.EntityInfo(parent));
        return parent;
    }

    /**
    	\brief Processes the fill storage summary operation.
    	\param data \p data processed by the operation.
    	\param traceId \p identifier of the trace.
    */
    static void FillStorageSummary(ServerSentryItemMoveData data, int traceId = 0)
    {
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.FillStorageSummary.enter");
        if (!data)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.FillStorageSummary.skip", "data=null");
            return;
        }

        if (data.from && data.from.type == "storage")
        {
            data.sourceStorageClassName = data.from.parentClassName;
            data.sourceStorageName = data.from.parentName;
            data.sourceStorageRaw = data.from.raw;
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.FillStorageSummary.source", data.sourceStorageClassName + " / " + data.sourceStorageName + " raw=" + data.sourceStorageRaw);
        }

        if (data.to && data.to.type == "storage")
        {
            data.destinationStorageClassName = data.to.parentClassName;
            data.destinationStorageName = data.to.parentName;
            data.destinationStorageRaw = data.to.raw;
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.FillStorageSummary.destination", data.destinationStorageClassName + " / " + data.destinationStorageName + " raw=" + data.destinationStorageRaw);
        }

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.FillStorageSummary.done");
    }

    /**
    	\brief Builds location label.
    	\param loc \p loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\param label \p human-readable label shown to users.
    	\return \p The resulting string value.
    */
    static string BuildLocationLabel(ServerSentryItemLocationData loc, int traceId = 0, string label = "")
    {
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.BuildLocationLabel.enter." + label, "loc=" + ServerSentryInventoryDebug.BoolToString(loc != null));
        if (!loc)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.BuildLocationLabel.result." + label, "unknown");
            return "unknown";
        }

        if (loc.type == "storage")
        {
            string storageName = loc.parentName;
            if (storageName == "")
            {
                storageName = loc.parentClassName;
            }

            string storageResult = "storage:" + storageName;
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.BuildLocationLabel.result." + label, storageResult);
            return storageResult;
        }

        if (loc.type == "player_slot")
        {
            string ownerName = loc.parentOwnerName;
            if (ownerName == "")
            {
                ownerName = loc.parentOwnerClassName;
            }

            string parentName = loc.parentName;
            if (parentName == "")
            {
                parentName = loc.parentClassName;
            }

            string slotName = loc.slotName;
            if (slotName == "")
            {
                slotName = loc.slotId.ToString();
            }

            string slotResult = "player_slot:" + ownerName + ":" + parentName + ":" + slotName;
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.BuildLocationLabel.result." + label, slotResult);
            return slotResult;
        }

        if (loc.type == "cargo" || loc.type == "hands")
        {
            string playerName = loc.parentOwnerName;
            if (playerName == "")
            {
                playerName = loc.parentName;
            }

            string cargoResult = loc.type + ":" + playerName;
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.BuildLocationLabel.result." + label, cargoResult);
            return cargoResult;
        }

        if (loc.type == "ground")
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.BuildLocationLabel.result." + label, "ground");
            return "ground";
        }

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.BuildLocationLabel.result." + label, loc.type);
        return loc.type;
    }

    /**
    	\brief Builds item move message.
    	\param actionName \p action name used by the operation.
    	\param item \p item or value processed by the operation.
    	\param data \p data processed by the operation.
    	\param traceId \p identifier of the trace.
    	\return \p The resulting string value.
    */
    static string BuildItemMoveMessage(string actionName, EntityAI item, ServerSentryItemMoveData data, int traceId = 0)
    {
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.BuildItemMoveMessage.enter", "action=" + actionName + " item=" + ServerSentryInventoryDebug.ItemInfo(item));
        string itemName = "item";
        if (item)
        {
            itemName = item.GetDisplayName();
            if (itemName == "")
            {
                itemName = item.GetType();
            }
        }

        string fromLabel = "unknown";
        string toLabel = "unknown";
        if (data)
        {
            if (data.fromLabel != "")
            {
                fromLabel = data.fromLabel;
            }

            if (data.toLabel != "")
            {
                toLabel = data.toLabel;
            }
        }

        string message = actionName + ": " + itemName + " from " + fromLabel + " to " + toLabel;
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.BuildItemMoveMessage.result", message);
        return message;
    }

    /**
    	\brief Resolves item kind.
    	\param item \p item or value processed by the operation.
    	\param traceId \p identifier of the trace.
    	\return \p The resulting string value.
    */
    static string ResolveItemKind(EntityAI item, int traceId = 0)
    {
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemKind.enter", ServerSentryInventoryDebug.ItemInfo(item));
        if (!item)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemKind.result", "unknown");
            return "unknown";
        }

        if (Weapon_Base.Cast(item))
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemKind.result", "weapon");
            return "weapon";
        }

        if (Magazine.Cast(item))
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemKind.result", "magazine");
            return "magazine";
        }

        string className = item.GetType();

        if (className.Contains("Ammo_") || className.Contains("AmmoBox") || className.Contains("Bullet"))
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemKind.result", "ammo");
            return "ammo";
        }

        if (className.Contains("Optic") || className.Contains("Suppressor") || className.Contains("Buttstock") || className.Contains("Handguard") || className.Contains("Bayonet") || className.Contains("Light") || className.Contains("Battery") || className.Contains("Bipod") || className.Contains("Muzzle"))
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemKind.result", "weapon_attachment");
            return "weapon_attachment";
        }

        if (className.Contains("Bag") || className.Contains("Backpack") || className.Contains("Vest") || className.Contains("Pants") || className.Contains("Jacket") || className.Contains("Shirt") || className.Contains("Boots") || className.Contains("Gloves") || className.Contains("Helmet") || className.Contains("Mask"))
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemKind.result", "clothing_or_container");
            return "clothing_or_container";
        }

        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.ResolveItemKind.result", "item");
        return "item";
    }

    /**
    	\brief Builds item move ID.
    	\param item \p item or value processed by the operation.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\return \p The resulting string value.
    */
    static string BuildItemMoveId(EntityAI item, InventoryLocation oldLoc, InventoryLocation newLoc, int traceId = 0)
    {
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.BuildItemMoveId.enter", ServerSentryInventoryDebug.ItemInfo(item));
        string id = "move-";

        if (item)
        {
            id += item.GetID().ToString();
        }
        else
        {
            id += "unknown";
        }

        if (GetGame())
        {
            id += "-" + GetGame().GetTime().ToString();
        }
        else
        {
            id += "-nogame";
        }

        id += "-trace" + traceId.ToString();
        ServerSentryInventoryDebug.PrintTrace(traceId, "ItemEventFactory.BuildItemMoveId.result", id);
        return id;
    }

};
