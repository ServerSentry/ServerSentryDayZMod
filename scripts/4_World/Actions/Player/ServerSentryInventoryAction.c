/** \file ServerSentryInventoryAction.c
    \brief Captures authoritative player inventories and applies snapshot-bound single-item changes.
*/

/** \brief Describes one concrete item in a captured inventory. */
class ServerSentryInventoryItemDto
{
    /** \brief Index valid only within the returned snapshot. */
    string id;
    /** \brief Index of the captured parent, or empty for player-level items. */
    string parentId;
    /** \brief Exact engine class. */
    string className;
    /** \brief Engine display name. */
    string displayName;
    /** \brief Stack quantity or magazine rounds, or -1 if not applicable. */
    float quantity = -1;
    /** \brief Overall health as a percentage. */
    float healthPercent;
    /** \brief Contained descendants and attachments; nonempty items cannot be removed. */
    int childCount;
    /** \brief Human-readable location. */
    string location;
    /** \brief Exact immediate destination captured from the engine location. */
    string destinationId;
}

/** \brief Carries a bounded inventory response without exposing engine references. */
class ServerSentryInventoryDataDto
{
    /** \brief Version of this typed response contract. */
    int version = 2;
    /** \brief Exact Steam identity supplied by the CMS. */
    string steamId;
    /** \brief Whether this identity is currently connected. */
    bool online;
    /** \brief Random request ID identifying this capture, not a persistent item ID. */
    string snapshotId;
    /** \brief UTC capture timestamp. */
    string capturedAtUtc;
    /** \brief Whether the response omits items beyond its fixed bound. */
    bool truncated;
    /** \brief Captured item metadata in deterministic inventory traversal order. */
    ref array<ref ServerSentryInventoryItemDto> items = new array<ref ServerSentryInventoryItemDto>();
    /** \brief Bounded exact cargo/attachment/hands destinations, including empty slots. */
    ref array<ref ServerSentryInventoryDestinationDto> destinations = new array<ref ServerSentryInventoryDestinationDto>();
}

/** \brief Public destination capability, valid only for this capture. */
class ServerSentryInventoryDestinationDto
{
    /** \brief Snapshot-local destination identity. */
    string id;
    /** \brief Snapshot-local parent item identity, or empty for the player. */
    string parentId;
    /** \brief Exact placement type: hands, attachment, or cargo. */
    string kind;
    /** \brief Human-readable engine slot or container label. */
    string label;
    /** \brief Technical attachment slot name for catalog filtering. */
    string slotName;
}

/** \brief Server-local exact parent reference and native attachment slot. */
class ServerSentryInventoryDestinationCapture
{
    /** \brief Concrete parent whose placement was captured. */
    EntityAI parent;
    /** \brief Captured native placement category. */
    string kind;
    /** \brief Native attachment slot ID, or -1 for other placements. */
    int slotId = -1;
}

/** \brief Keeps short-lived server-local instance references; never serializes these references. */
class ServerSentryInventoryCapture
{
    /** \brief Captured player instance; a new character cannot reuse this capture. */
    PlayerBase player;
    /** \brief Immutable capture ID. */
    string id;
    /** \brief Monotonic server time when captured. */
    int created;
    /** \brief Concrete engine entities matching response indices. */
    ref array<EntityAI> entities = new array<EntityAI>();
    /** \brief Exact destination references aligned with the returned destination IDs. */
    ref array<ref ServerSentryInventoryDestinationCapture> destinations = new array<ref ServerSentryInventoryDestinationCapture>();
}

/** \brief Handles inventory-only actions independently of shop/support fulfillment. */
class ServerSentryInventoryAction: ServerSentryModActionHandler
{
    /** \brief Operation implemented by this registered handler. */
    protected string m_Operation;
    /** \brief Latest capture per player, bounded and valid for two minutes. */
    protected static ref map<string, ref ServerSentryInventoryCapture> s_Captures = new map<string, ref ServerSentryInventoryCapture>();

    /** \brief Initializes a handler for a fixed operation.
        \param operation \p snapshot, add, or remove.
    */
    void ServerSentryInventoryAction(string operation)
    {
        m_Operation = operation;
    }

    /** \brief Invalidates all exact item references before another workflow replaces this inventory. */
    static void InvalidatePlayerCapture(string steamId)
    {
        s_Captures.Remove(steamId);
    }

    /** \brief Registers only the dedicated inventory scope, never shop or generic live execution.
        \return \p The fixed descriptor for this handler.
    */
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto descriptor = new ServerSentryLiveModActionDescriptorDto();
        descriptor.id = "inventory." + m_Operation;
        descriptor.targetType = "player";
        descriptor.label = "Inventory " + m_Operation;
        descriptor.dangerous = m_Operation != "snapshot";
        descriptor.icon = "backpack";
        descriptor.description = "Dedicated CMS inventory workflow; changes require a fresh instance snapshot.";
        descriptor.scopes = new array<string>();
        descriptor.scopes.Insert("inventory");
        return descriptor;
    }

    /** \brief Counts owned descendants, including attachment-held cargo.
        \param item \p selected entity.
        \return \p Number of contained item instances.
    */
    protected int Children(EntityAI item)
    {
        int count = 0;
        array<EntityAI> children = new array<EntityAI>();
        item.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, children);
        foreach (EntityAI child: children)
        {
            if (child && child != item)
            {
                count++;
            }
        }
        return count;
    }

    /** \brief Removes expired captures and refuses unbounded per-player state.
        \return \p Whether a new capture may be stored.
    */
    protected bool Prune()
    {
        array<string> expired = new array<string>();
        int now = GetGame().GetTime();
        foreach (string key, ServerSentryInventoryCapture capture: s_Captures)
        {
            if (!capture || !capture.player || now < capture.created || now - capture.created > 120000)
            {
                expired.Insert(key);
            }
        }
        foreach (string oldKey: expired)
        {
            s_Captures.Remove(oldKey);
        }
        return s_Captures.Count() < 128;
    }

    /** \brief Captures a bounded destination without serializing native references.
        \param parent \p Concrete entity owning this placement.
        \param parentId \p Snapshot-local parent item ID, or empty for the player.
        \param kind \p Hands, attachment or cargo placement type.
        \param slotId \p Native attachment slot ID, or -1 for other placements.
        \param label \p Human-readable slot or container name.
        \param capture \p Server-local capture receiving the native reference.
        \param data \p Response receiving public destination metadata.
    */
    protected void AddDestination(EntityAI parent, string parentId, string kind, int slotId, string label, ServerSentryInventoryCapture capture, ServerSentryInventoryDataDto data)
    {
        if (capture.destinations.Count() >= 2048)
        {
            data.truncated = true;
            return;
        }
        ServerSentryInventoryDestinationCapture target = new ServerSentryInventoryDestinationCapture();
        target.parent = parent;
        target.kind = kind;
        target.slotId = slotId;
        ServerSentryInventoryDestinationDto dto = new ServerSentryInventoryDestinationDto();
        dto.id = capture.destinations.Count().ToString();
        dto.parentId = parentId;
        dto.kind = kind;
        dto.label = label;
        if (kind == "attachment" && InventorySlots.IsSlotIdValid(slotId))
        {
            dto.slotName = InventorySlots.GetSlotName(slotId);
        }
        capture.destinations.Insert(target);
        data.destinations.Insert(dto);
    }

    /** \brief Enumerates only this exact parent's cargo and configured attachment slots.
        \param parent \p Entity whose destinations are captured.
        \param parentId \p Snapshot-local identity of this entity.
        \param capture \p Server-local capture storing exact placements.
        \param data \p Public snapshot receiving available destinations.
    */
    protected void CaptureDestinations(EntityAI parent, string parentId, ServerSentryInventoryCapture capture, ServerSentryInventoryDataDto data)
    {
        if (!parent || !parent.GetInventory())
        {
            return;
        }
        if (parent == capture.player)
        {
            AddDestination(parent, parentId, "hands", -1, "Hands", capture, data);
        }
        if (parent.GetInventory().GetCargo())
        {
            AddDestination(parent, parentId, "cargo", -1, "Contents", capture, data);
        }
        for (int slotIndex = 0; slotIndex < parent.GetInventory().GetAttachmentSlotsCount(); slotIndex++)
        {
            int slot = parent.GetInventory().GetAttachmentSlotId(slotIndex);
            string label = InventorySlots.GetSlotDisplayName(slot);
            if (label == "")
            {
                label = InventorySlots.GetSlotName(slot);
            }
            AddDestination(parent, parentId, "attachment", slot, label, capture, data);
            if (data.truncated)
            {
                return;
            }
        }
    }

    /** \brief Finds the exact captured placement for a concrete item.
        \param item \p Owned item whose current location is inspected.
        \param capture \p Capture containing possible destination references.
        \return \p Matching snapshot-local destination ID, or empty when absent.
    */
    protected string FindDestination(EntityAI item, ServerSentryInventoryCapture capture)
    {
        InventoryLocation location = new InventoryLocation();
        if (!item.GetInventory().GetCurrentInventoryLocation(location))
        {
            return "";
        }
        for (int index = 0; index < capture.destinations.Count(); index++)
        {
            ServerSentryInventoryDestinationCapture target = capture.destinations[index];
            if (target.parent != item.GetHierarchyParent())
            {
                continue;
            }
            int type = location.GetType();
            if ((target.kind == "hands" && type == InventoryLocationType.HANDS) || (target.kind == "cargo" && type == InventoryLocationType.CARGO) || (target.kind == "attachment" && type == InventoryLocationType.ATTACHMENT && target.slotId == location.GetSlot()))
            {
                return index.ToString();
            }
        }
        return "";
    }

    /** \brief Returns current inventory data, including an authoritative offline response.
        \param ctx \p Exact player request context.
        \param player \p Connected player, or null if offline.
        \param message \p Receives the result description.
        \return \p Whether a typed snapshot response was generated.
    */
    protected bool Snapshot(ServerSentryModActionContext ctx, PlayerBase player, out string message)
    {
        s_Captures.Remove(ctx.steamId);
        ServerSentryInventoryDataDto data = new ServerSentryInventoryDataDto();
        data.steamId = ctx.steamId;
        data.online = player != null;
        data.capturedAtUtc = ServerSentryTime.NowUtc();
        if (player)
        {
            if (!Prune())
            {
                message = "Too many inventory captures. Retry shortly.";
                return false;
            }
            ServerSentryInventoryCapture capture = new ServerSentryInventoryCapture();
            capture.player = player;
            capture.id = ctx.requestId;
            capture.created = GetGame().GetTime();
            array<EntityAI> inventory = new array<EntityAI>();
            player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, inventory);
            EntityAI hands = player.GetHumanInventory().GetEntityInHands();
            if (hands)
            {
                array<EntityAI> heldInventory = new array<EntityAI>();
                hands.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, heldInventory);
                if (heldInventory.Find(hands) == -1)
                {
                    heldInventory.Insert(hands);
                }
                foreach (EntityAI heldItem: heldInventory)
                {
                    if (inventory.Find(heldItem) == -1)
                    {
                        inventory.Insert(heldItem);
                    }
                }
            }
            foreach (EntityAI entity: inventory)
            {
                if (!entity || entity == player || entity.GetHierarchyRootPlayer() != player)
                {
                    continue;
                }
                if (capture.entities.Count() >= 512)
                {
                    data.truncated = true;
                    break;
                }
                capture.entities.Insert(entity);
            }
            CaptureDestinations(player, "", capture, data);
            for (int parentIndexForTargets = 0; parentIndexForTargets < capture.entities.Count(); parentIndexForTargets++)
            {
                CaptureDestinations(capture.entities[parentIndexForTargets], parentIndexForTargets.ToString(), capture, data);
                if (data.truncated)
                {
                    break;
                }
            }
            for (int index = 0; index < capture.entities.Count(); index++)
            {
                EntityAI item = capture.entities[index];
                ServerSentryInventoryItemDto row = new ServerSentryInventoryItemDto();
                row.id = index.ToString();
                row.destinationId = FindDestination(item, capture);
                row.className = item.GetType();
                row.displayName = item.GetDisplayName();
                row.healthPercent = item.GetHealth01("", "Health") * 100;
                row.childCount = Children(item);
                EntityAI parent = item.GetHierarchyParent();
                int parentIndex = capture.entities.Find(parent);
                if (parentIndex >= 0)
                {
                    row.parentId = parentIndex.ToString();
                }
                row.location = "Equipped";
                if (parent && parent != player)
                {
                    row.location = parent.GetDisplayName();
                }
                if (item == hands)
                {
                    row.location = "Hands";
                }
                ItemBase stack = ItemBase.Cast(item);
                Magazine magazine = Magazine.Cast(item);
                if (magazine)
                {
                    row.quantity = magazine.GetAmmoCount();
                }
                else if (stack && stack.HasQuantity())
                {
                    row.quantity = stack.GetQuantity();
                }
                data.items.Insert(row);
            }
            data.snapshotId = capture.id;
            if (!data.truncated)
            {
                s_Captures.Set(ctx.steamId, capture);
            }
        }
        JsonSerializer serializer = new JsonSerializer();
        if (!serializer.WriteToString(data, false, ctx.resultDataJson))
        {
            s_Captures.Remove(ctx.steamId);
            message = "Inventory could not be serialized. No changes are available.";
            return false;
        }
        message = "Inventory snapshot captured.";
        return true;
    }

    /** \brief Applies at most one change from a fresh player-bound snapshot.
        \param ctx \p Authenticated inventory request with reason and capture ID.
        \param message \p Receives a confirmed result or explicit refusal.
        \return \p Whether the requested operation completed.
    */
    override bool Execute(ServerSentryModActionContext ctx, out string message)
    {
        if (!GetGame() || !GetGame().IsServer() || !ctx || ctx.steamId.Length() != 17)
        {
            message = "An exact SteamID on the game server is required.";
            return false;
        }
        PlayerBase player = ServerSentryLiveActionResolver.FindPlayer(ctx);
        if (m_Operation == "snapshot")
        {
            return Snapshot(ctx, player, message);
        }
        ServerSentryInventoryCapture capture = s_Captures.Get(ctx.steamId);
        string snapshotId = ctx.GetString("snapshotId");
        int now = GetGame().GetTime();
        if (!player || !player.IsAlive() || !capture || capture.player != player || capture.id != snapshotId || now < capture.created || now - capture.created > 120000)
        {
            message = "Player is offline, changed character, or the snapshot expired. Refresh inventory.";
            return false;
        }
        // Consume before touching the game. Retries cannot reapply this operation, even on failure.
        // A server restart clears all captures and consequently rejects queued old mutations.
        s_Captures.Remove(ctx.steamId);
        if (ctx.GetString("reason") == "")
        {
            message = "An administrative reason is required.";
            return false;
        }
        if (m_Operation == "add")
        {
            string className = ctx.GetString("className");
            if (className == "")
            {
                message = "Choose a catalog item.";
                return false;
            }
            string destinationId = ctx.GetString("destinationId");
            int destinationIndex = destinationId.ToInt();
            if (ctx.GetInt("placementVersion") != 2 || destinationIndex < 0 || destinationIndex >= capture.destinations.Count() || destinationIndex.ToString() != destinationId)
            {
                message = "Choose an exact destination from a fresh inventory snapshot.";
                return false;
            }
            ServerSentryInventoryDestinationCapture destination = capture.destinations[destinationIndex];
            if (!destination || !destination.parent || (destination.parent != player && destination.parent.GetHierarchyRootPlayer() != player))
            {
                message = "Destination ownership changed. Refresh inventory.";
                return false;
            }
            EntityAI added;
            if (destination.kind == "hands" && destination.parent == player && !player.GetHumanInventory().GetEntityInHands())
            {
                added = player.GetHumanInventory().CreateInHands(className);
            }
            else if (destination.kind == "cargo")
            {
                added = destination.parent.GetInventory().CreateEntityInCargo(className);
            }
            else if (destination.kind == "attachment" && !destination.parent.GetInventory().FindAttachment(destination.slotId))
            {
                added = destination.parent.GetInventory().CreateAttachmentEx(className, destination.slotId);
            }
            if (!added)
            {
                message = "The selected destination is full, occupied or incompatible. Nothing was spawned elsewhere.";
                return false;
            }
            if (!ItemBase.Cast(added) || added.GetHierarchyRootPlayer() != player || added.GetHierarchyParent() != destination.parent)
            {
                GetGame().ObjectDelete(added);
                message = "The class is not an inventory item.";
                return false;
            }
            message = "Added one " + className + " to the selected destination.";
            return true;
        }
        if (m_Operation == "remove")
        {
            string itemId = ctx.GetString("itemId");
            int selected = itemId.ToInt();
            if (selected < 0 || selected >= capture.entities.Count() || selected.ToString() != itemId)
            {
                message = "The selected item does not belong to this capture.";
                return false;
            }
            EntityAI removed = capture.entities[selected];
            if (!removed || removed == player || removed.GetHierarchyRootPlayer() != player)
            {
                message = "Item ownership changed. Refresh inventory.";
                return false;
            }
            if (Children(removed) > 0)
            {
                message = "Remove the contents and attachments first. Containers are never deleted recursively.";
                return false;
            }
            GetGame().ObjectDelete(removed);
            message = "Selected inventory item removed.";
            return true;
        }
        message = "Unknown inventory operation.";
        return false;
    }
}
