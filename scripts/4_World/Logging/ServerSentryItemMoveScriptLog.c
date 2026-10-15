/**
	\file ServerSentryItemMoveScriptLog.c
	\brief Provides logging support for Item move script log.
*/

/**
	\brief Provides ServerSentry behavior for Item move location snapshot.
*/
class ServerSentryItemMoveLocationSnapshot
{
    /**
    	\brief Indicates whether valid.
    */
    bool valid;
    /**
    	\brief Stores the type ID.
    */
    int typeId;
    /**
    	\brief Stores the type name.
    */
    string typeName;

    /**
    	\brief Stores the parent ID.
    */
    int parentId;
    /**
    	\brief Stores the parent class name.
    */
    string parentClassName;
    /**
    	\brief Stores the parent name.
    */
    string parentName;
    /**
    	\brief Stores the parent position.
    */
    vector parentPosition;

    /**
    	\brief Indicates whether has player owner.
    */
    bool hasPlayerOwner;
    /**
        \brief Indicates whether the immediate parent is a player rather than an owned item.
    */
    bool parentIsPlayer;
    /**
    	\brief Stores the parent owner type.
    */
    string parentOwnerType;
    /**
    	\brief Stores the parent owner kind.
    */
    string parentOwnerKind;
    /**
    	\brief Stores the parent owner class name.
    */
    string parentOwnerClassName;
    /**
    	\brief Stores the parent owner steam ID.
    */
    string parentOwnerSteamId;
    /**
    	\brief Stores the parent owner bohemia ID.
    */
    string parentOwnerBohemiaId;
    /**
    	\brief Stores the parent owner name.
    */
    string parentOwnerName;
    /**
    	\brief Stores the parent owner position.
    */
    vector parentOwnerPosition;

    /**
    	\brief Indicates whether is external storage.
    */
    bool isExternalStorage;

    /**
    	\brief Stores the slot ID.
    */
    int slotId;
    /**
    	\brief Stores the slot name.
    */
    string slotName;
    /**
        \brief Display name of the captured attachment slot.
    */
    string slotDisplayName;
    /**
    	\brief Stores the cargo idx.
    */
    int cargoIdx;
    /**
    	\brief Stores the row.
    */
    int row;
    /**
    	\brief Stores the col.
    */
    int col;
    /**
    	\brief Indicates whether flip.
    */
    bool flip;
    /**
    	\brief Stores the position.
    */
    vector position;
    /**
    	\brief Stores the raw.
    */
    string raw;

    /**
    	\brief Initializes a new ServerSentryItemMoveLocationSnapshot instance.
    */
    void ServerSentryItemMoveLocationSnapshot()
    {
        valid = false;
        typeId = -1;
        typeName = "unknown";
        parentId = -1;
        parentClassName = "";
        parentName = "";
        parentPosition = "0 0 0";
        hasPlayerOwner = false;
        parentIsPlayer = false;
        parentOwnerType = "";
        parentOwnerKind = "";
        parentOwnerClassName = "";
        parentOwnerSteamId = "";
        parentOwnerBohemiaId = "";
        parentOwnerName = "";
        parentOwnerPosition = "0 0 0";
        isExternalStorage = false;
        slotId = -1;
        slotName = "";
        slotDisplayName = "";
        cargoIdx = -1;
        row = -1;
        col = -1;
        flip = false;
        position = "0 0 0";
        raw = "invalid";
    }
}

/**
	\brief Provides ServerSentry behavior for Item move snapshot.
*/
class ServerSentryItemMoveSnapshot
{
    /**
    	\brief Stores the move ID.
    */
    int moveId;
    /**
    	\brief Stores the game time.
    */
    int gameTime;

    /**
    	\brief Indicates whether allowed.
    */
    bool allowed;
    /**
    	\brief Stores the reject reason.
    */
    string rejectReason;

    /**
    	\brief Stores the item ID.
    */
    int runtimeItemId;
    /**
        \brief Stores the restart-stable DayZ persistence ID when available.
    */
    string itemPersistentId;
    /**
    	\brief Stores the item class name.
    */
    string itemClassName;
    /**
    	\brief Stores the item display name.
    */
    string itemDisplayName;
    /**
    	\brief Stores the item kind.
    */
    string itemKind;
    /**
    	\brief Stores the item position.
    */
    vector itemPosition;

    /**
    	\brief Stores the action name.
    */
    string actionName;
    /**
    	\brief Stores the actor inference.
    */
    string actorInference;

    /**
    	\brief Stores the actor type.
    */
    string actorType;
    /**
    	\brief Stores the actor kind.
    */
    string actorKind;
    /**
    	\brief Stores the actor class name.
    */
    string actorClassName;
    /**
    	\brief Stores the actor steam ID.
    */
    string actorSteamId;
    /**
    	\brief Stores the actor bohemia ID.
    */
    string actorBohemiaId;
    /**
    	\brief Stores the actor name.
    */
    string actorName;
    /**
    	\brief Stores the actor position.
    */
    vector actorPosition;

    /**
    	\brief Type of target resolved by the action.
    */
    string targetType;
    /**
    	\brief Stores the target kind.
    */
    string targetKind;
    /**
    	\brief Stores the target class name.
    */
    string targetClassName;
    /**
    	\brief Stores the target name.
    */
    string targetName;

    /**
    	\brief Stores the from.
    */
    ref ServerSentryItemMoveLocationSnapshot from;
    /**
    	\brief Stores the to.
    */
    ref ServerSentryItemMoveLocationSnapshot to;

    /**
    	\brief Initializes a new ServerSentryItemMoveSnapshot instance.
    */
    void ServerSentryItemMoveSnapshot()
    {
        moveId = 0;
        gameTime = 0;
        allowed = false;
        rejectReason = "not evaluated";
        runtimeItemId = -1;
        itemPersistentId = "";
        itemClassName = "";
        itemDisplayName = "";
        itemKind = "unknown";
        itemPosition = "0 0 0";
        actionName = "item_move";
        actorInference = "unknown";
        actorType = "unknown";
        actorKind = "unknown";
        actorClassName = "";
        actorSteamId = "";
        actorBohemiaId = "";
        actorName = "";
        actorPosition = "0 0 0";
        targetType = "environment";
        targetKind = "environment";
        targetClassName = "";
        targetName = "ground";
        from = new ServerSentryItemMoveLocationSnapshot();
        to = new ServerSentryItemMoveLocationSnapshot();
    }
}

/**
	\brief Provides ServerSentry behavior for Item move script log.
*/
class ServerSentryItemMoveScriptLog
{
    /**
    	\brief Shared move counter state used by all instances.
    */
    protected static int s_MoveCounter;
    /**
    	\brief Shared recent moves state used by all instances.
    */
    protected static ref map<string, int> s_RecentMoves;

    /**
    	\brief Processes the next move ID operation.
    	\return \p The resulting integer value or status code.
    */
    static int NextMoveId()
    {
        s_MoveCounter++;
        return s_MoveCounter;
    }

    /**
    	\brief Processes the capture snapshot operation.
    	\param moveId \p identifier of the move.
    	\param item \p item or value processed by the operation.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    	\return \p The resulting item move snapshot instance, or null when unavailable.
    */
    static ServerSentryItemMoveSnapshot CaptureSnapshot(int moveId, ItemBase item, InventoryLocation oldLoc, InventoryLocation newLoc)
    {
        ServerSentryItemMoveSnapshot snapshot = new ServerSentryItemMoveSnapshot();
        snapshot.moveId = moveId;

        if (!GetGame())
        {
            snapshot.rejectReason = "GetGame=null";
            return snapshot;
        }

        snapshot.gameTime = GetGame().GetTime();

        if (!GetGame().IsServer())
        {
            snapshot.rejectReason = "not server";
            return snapshot;
        }

        if (!item)
        {
            snapshot.rejectReason = "item=null";
            return snapshot;
        }

        snapshot.itemClassName = item.GetType();
        snapshot.runtimeItemId = item.GetID();
        snapshot.itemPersistentId = ResolveItemPersistentId(item);
        snapshot.itemDisplayName = item.GetDisplayName();
        snapshot.itemPosition = item.GetPosition();
        snapshot.itemKind = ResolveItemKind(snapshot.itemClassName);

        CaptureLocation(oldLoc, snapshot.from);
        CaptureLocation(newLoc, snapshot.to);

        if (!snapshot.from.valid)
        {
            snapshot.rejectReason = "from invalid";
            return snapshot;
        }

        if (!snapshot.to.valid)
        {
            snapshot.rejectReason = "to invalid";
            return snapshot;
        }

        if (snapshot.from.raw == snapshot.to.raw)
        {
            snapshot.rejectReason = "same location";
            return snapshot;
        }

        snapshot.actionName = ResolveAction(snapshot);
        ResolveActor(snapshot);
        ResolveTarget(snapshot);

        if (!IsTrackedMove(snapshot))
        {
            snapshot.rejectReason = "not tracked/player move";
            return snapshot;
        }

        if (ServerSentryDefaults.LogOnlyPlayerItemMoveEvents() && !HasPlayerActor(snapshot))
        {
            snapshot.rejectReason = "player actor missing";
            return snapshot;
        }

        if (IsDuplicateSnapshot(snapshot))
        {
            snapshot.rejectReason = "duplicate";
            return snapshot;
        }

        snapshot.allowed = true;
        snapshot.rejectReason = "allowed";
        return snapshot;
    }

    /**
    	\brief Creates primitive snapshot event.
    	\param moveId \p identifier of the move.
    	\param item \p item or value processed by the operation.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    	\return \p The resulting event instance, or null when unavailable.
    */
    static ServerSentryEvent CreatePrimitiveSnapshotEvent(int moveId, ItemBase item, InventoryLocation oldLoc, InventoryLocation newLoc)
    {
        ServerSentryItemMoveSnapshot snapshot = CaptureSnapshot(moveId, item, oldLoc, newLoc);
        if (!snapshot || !snapshot.allowed)
        {
            return null;
        }

        return BuildEventFromSnapshot(snapshot);
    }

    /**
    	\brief Emits primitive snapshot move.
    	\param moveId \p identifier of the move.
    	\param item \p item or value processed by the operation.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool EmitPrimitiveSnapshotMove(int moveId, ItemBase item, InventoryLocation oldLoc, InventoryLocation newLoc)
    {
        if (!ServerSentryDefaults.LogItemMoveEvents())
        {
            return false;
        }

        ServerSentryEvent ev = CreatePrimitiveSnapshotEvent(moveId, item, oldLoc, newLoc);
        if (!ev)
        {
            return false;
        }

        return ServerSentryLogger.Get().Emit(ev);
    }

    /**
    	\brief Emits snapshot.
    	\param snapshot \p snapshot used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool EmitSnapshot(ServerSentryItemMoveSnapshot snapshot)
    {
        if (!snapshot || !snapshot.allowed)
        {
            return false;
        }

        if (!ServerSentryDefaults.LogItemMoveEvents())
        {
            return false;
        }

        ServerSentryEvent ev = BuildEventFromSnapshot(snapshot);
        if (!ev)
        {
            return false;
        }

        return ServerSentryLogger.Get().Emit(ev);
    }

    /**
    	\brief Processes the capture location operation.
    	\param loc \p loc used by the operation.
    	\param output \p output used by the operation.
    */
    protected static void CaptureLocation(InventoryLocation loc, ServerSentryItemMoveLocationSnapshot output)
    {
        if (!output)
        {
            return;
        }

        if (!loc)
        {
            output.raw = "null";
            return;
        }

        output.valid = loc.IsValid();
        if (!output.valid)
        {
            output.raw = "invalid";
            return;
        }

        output.typeId = loc.GetType();
        output.typeName = LocationTypeName(output.typeId);

        EntityAI parent = loc.GetParent();
        if (parent)
        {
            output.parentId = parent.GetID();
            output.parentClassName = parent.GetType();
            output.parentName = parent.GetDisplayName();
            output.parentPosition = parent.GetPosition();
            output.parentIsPlayer = PlayerBase.Cast(parent) != null;
            CapturePlayerOwner(parent, output);
        }

        if (output.typeId == InventoryLocationType.ATTACHMENT)
        {
            output.slotId = loc.GetSlot();
            if (InventorySlots.IsSlotIdValid(output.slotId))
            {
                output.slotName = InventorySlots.GetSlotName(output.slotId);
                output.slotDisplayName = InventorySlots.GetSlotDisplayName(output.slotId);
            }
        }
        else if (output.typeId == InventoryLocationType.CARGO || output.typeId == InventoryLocationType.PROXYCARGO)
        {
            output.cargoIdx = loc.GetIdx();
            output.row = loc.GetRow();
            output.col = loc.GetCol();
            output.flip = loc.GetFlip();
        }
        else if (output.typeId == InventoryLocationType.GROUND)
        {
            output.position = loc.GetPos();
        }

        output.isExternalStorage = IsExternalStorage(output);
        output.raw = BuildRawLocation(output);
    }

    /**
    	\brief Processes the capture player owner operation.
    	\param parent \p parent used by the operation.
    	\param output \p output used by the operation.
    */
    protected static void CapturePlayerOwner(EntityAI parent, ServerSentryItemMoveLocationSnapshot output)
    {
        if (!parent || !output)
        {
            return;
        }

        PlayerBase player = PlayerBase.Cast(parent);
        if (!player)
        {
            Man root = parent.GetHierarchyRootPlayer();
            player = PlayerBase.Cast(root);
        }

        if (!player)
        {
            return;
        }

        output.hasPlayerOwner = true;
        output.parentOwnerClassName = player.GetType();
        output.parentOwnerPosition = player.GetPosition();

        PlayerIdentity identity = player.GetIdentity();
        if (identity)
        {
            output.parentOwnerType = "player";
            output.parentOwnerKind = "human_player";
            output.parentOwnerSteamId = identity.GetPlainId();
            output.parentOwnerBohemiaId = identity.GetId();
            output.parentOwnerName = identity.GetName();
        }
        else
        {
            output.parentOwnerType = "ai";
            output.parentOwnerKind = "human_ai";
            output.parentOwnerSteamId = "76561111111111111";
            output.parentOwnerBohemiaId = "!!AI!!";
            output.parentOwnerName = "(AI) " + player.GetType();
        }
    }

    /**
    	\brief Resolves item kind.
    	\param className \p class name used by the operation.
    	\return \p The resulting string value.
    */
    protected static string ResolveItemKind(string className)
    {
        if (className == "")
        {
            return "unknown";
        }

        if (className.Contains("Weapon") || className.Contains("Rifle") || className.Contains("Pistol") || className.Contains("Shotgun") || className.Contains("SMG"))
        {
            return "weapon";
        }

        if (className.Contains("Mag") || className.Contains("Ammo"))
        {
            return "ammo";
        }

        if (className.Contains("Clothing") || className.Contains("Jacket") || className.Contains("Pants") || className.Contains("Backpack") || className.Contains("Vest"))
        {
            return "clothing";
        }

        return "item";
    }

    /**
    	\brief Resolves action.
    	\param snapshot \p snapshot used by the operation.
    	\return \p The resulting string value.
    */
    protected static string ResolveAction(ServerSentryItemMoveSnapshot snapshot)
    {
        if (IsItemAttachment(snapshot.to))
            return "item_attach";

        if (IsItemAttachment(snapshot.from))
            return "item_detach";

        bool fromGround = snapshot.from.typeId == InventoryLocationType.GROUND;
        bool toGround = snapshot.to.typeId == InventoryLocationType.GROUND;
        bool fromPlayer = snapshot.from.hasPlayerOwner;
        bool toPlayer = snapshot.to.hasPlayerOwner;
        bool fromStorage = snapshot.from.isExternalStorage;
        bool toStorage = snapshot.to.isExternalStorage;

        if (fromGround && toPlayer)
            return "item_pickup";

        if (fromPlayer && toGround)
            return "item_drop";

        if (fromPlayer && toStorage)
            return "storage_put";

        if (fromStorage && toPlayer)
            return "storage_take";

        if (fromStorage && toStorage)
            return "storage_transfer";

        if (snapshot.to.typeId == InventoryLocationType.ATTACHMENT)
            return "item_attach";

        if (snapshot.from.typeId == InventoryLocationType.ATTACHMENT)
            return "item_detach";

        return "item_move";
    }

    /**
    	\brief Resolves actor.
    	\param snapshot \p snapshot used by the operation.
    */
    protected static void ResolveActor(ServerSentryItemMoveSnapshot snapshot)
    {
        ServerSentryItemMoveLocationSnapshot actorLoc = null;

        if (snapshot.actionName == "item_pickup" || snapshot.actionName == "storage_take" || (snapshot.actionName == "item_detach" && snapshot.to.hasPlayerOwner))
        {
            actorLoc = snapshot.to;
            snapshot.actorInference = "new_owner";
        }
        else if (snapshot.actionName == "item_drop" || snapshot.actionName == "storage_put" || (snapshot.actionName == "item_attach" && snapshot.from.hasPlayerOwner))
        {
            actorLoc = snapshot.from;
            snapshot.actorInference = "old_owner";
        }
        else if (snapshot.to.hasPlayerOwner)
        {
            actorLoc = snapshot.to;
            snapshot.actorInference = "new_owner";
        }
        else if (snapshot.from.hasPlayerOwner)
        {
            actorLoc = snapshot.from;
            snapshot.actorInference = "old_owner";
        }

        if (actorLoc && actorLoc.hasPlayerOwner)
        {
            snapshot.actorType = actorLoc.parentOwnerType;
            snapshot.actorKind = actorLoc.parentOwnerKind;
            snapshot.actorClassName = actorLoc.parentOwnerClassName;
            snapshot.actorSteamId = actorLoc.parentOwnerSteamId;
            snapshot.actorBohemiaId = actorLoc.parentOwnerBohemiaId;
            snapshot.actorName = actorLoc.parentOwnerName;
            snapshot.actorPosition = actorLoc.parentOwnerPosition;
        }
        else
        {
            snapshot.actorType = "environment";
            snapshot.actorKind = "environment";
            snapshot.actorName = "unknown";
            snapshot.actorPosition = snapshot.itemPosition;
        }
    }

    /**
    	\brief Resolves target.
    	\param snapshot \p snapshot used by the operation.
    */
    protected static void ResolveTarget(ServerSentryItemMoveSnapshot snapshot)
    {
        ServerSentryItemMoveLocationSnapshot targetLoc = snapshot.to;
        if (snapshot.actionName == "storage_take" || snapshot.actionName == "storage_transfer" || snapshot.actionName == "item_detach")
        {
            targetLoc = snapshot.from;
        }

        if (targetLoc && (targetLoc.isExternalStorage || IsItemAttachment(targetLoc)))
        {
            snapshot.targetType = "object";
            snapshot.targetKind = "object";
            snapshot.targetClassName = targetLoc.parentClassName;
            snapshot.targetName = targetLoc.parentName;
        }
        else if (targetLoc && targetLoc.hasPlayerOwner)
        {
            snapshot.targetType = targetLoc.parentOwnerType;
            snapshot.targetKind = targetLoc.parentOwnerKind;
            snapshot.targetClassName = targetLoc.parentOwnerClassName;
            snapshot.targetName = targetLoc.parentOwnerName;
        }
        else
        {
            snapshot.targetType = "environment";
            snapshot.targetKind = "environment";
            snapshot.targetClassName = "";
            snapshot.targetName = "ground";
        }
    }

    /**
    	\brief Returns whether tracked move.
    	\param snapshot \p snapshot used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    protected static bool IsTrackedMove(ServerSentryItemMoveSnapshot snapshot)
    {
        if (snapshot.from.typeId == InventoryLocationType.ATTACHMENT || snapshot.to.typeId == InventoryLocationType.ATTACHMENT)
            return true;

        bool fromGround = snapshot.from.typeId == InventoryLocationType.GROUND;
        bool toGround = snapshot.to.typeId == InventoryLocationType.GROUND;
        bool fromPlayer = snapshot.from.hasPlayerOwner;
        bool toPlayer = snapshot.to.hasPlayerOwner;
        bool fromStorage = snapshot.from.isExternalStorage;
        bool toStorage = snapshot.to.isExternalStorage;

        if (fromGround && toPlayer)
            return true;

        if (fromPlayer && toGround)
            return true;

        if (fromPlayer && toStorage)
            return true;

        if (fromStorage && toPlayer)
            return true;

        if (fromStorage && toStorage)
            return true;

        return false;
    }

    /**
        \brief Identifies an attachment to an object, excluding equipment slots on a player.
        \param loc \p captured inventory location.
        \return \p true for attachments to weapons, clothing, vehicles, or world objects.
    */
    protected static bool IsItemAttachment(ServerSentryItemMoveLocationSnapshot loc)
    {
        return loc && loc.valid && loc.typeId == InventoryLocationType.ATTACHMENT && !loc.parentIsPlayer;
    }

    /**
    	\brief Returns whether player actor.
    	\param snapshot \p snapshot used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    protected static bool HasPlayerActor(ServerSentryItemMoveSnapshot snapshot)
    {
        if (!snapshot)
        {
            return false;
        }

        return snapshot.from.hasPlayerOwner || snapshot.to.hasPlayerOwner;
    }

    /**
    	\brief Returns whether duplicate snapshot.
    	\param snapshot \p snapshot used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    protected static bool IsDuplicateSnapshot(ServerSentryItemMoveSnapshot snapshot)
    {
        if (!snapshot || !GetGame())
        {
            return false;
        }

        if (!s_RecentMoves)
        {
            s_RecentMoves = new map<string, int>();
        }

        string signature = snapshot.itemPersistentId;
        if (signature == "")
        {
            signature = "runtime:" + snapshot.runtimeItemId.ToString();
        }
        signature += "|" + snapshot.itemClassName;
        signature += "|" + snapshot.actionName;
        signature += "|" + snapshot.from.raw;
        signature += "|" + snapshot.to.raw;

        int now = snapshot.gameTime;
        if (s_RecentMoves.Contains(signature))
        {
            int lastSeen = s_RecentMoves.Get(signature);
            if (now - lastSeen <= ServerSentryDefaults.ItemMoveDedupWindowMs())
            {
                s_RecentMoves.Set(signature, now);
                return true;
            }
        }

        s_RecentMoves.Set(signature, now);
        CleanupRecentMoves(now);
        return false;
    }

    /**
    	\brief Processes the cleanup recent moves operation.
    	\param now \p now used by the operation.
    */
    protected static void CleanupRecentMoves(int now)
    {
        if (!s_RecentMoves)
        {
            return;
        }

        array<string> keys = new array<string>();
        int i;
        for (i = 0; i < s_RecentMoves.Count(); i++)
        {
            string key = s_RecentMoves.GetKey(i);
            int lastSeen = s_RecentMoves.GetElement(i);
            if (now - lastSeen > ServerSentryDefaults.ItemMoveDedupWindowMs() * 4)
            {
                keys.Insert(key);
            }
        }

        for (i = 0; i < keys.Count(); i++)
        {
            s_RecentMoves.Remove(keys.Get(i));
        }
    }

    /**
    	\brief Returns whether external storage.
    	\param loc \p loc used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    protected static bool IsExternalStorage(ServerSentryItemMoveLocationSnapshot loc)
    {
        if (!loc || !loc.valid)
            return false;

        if (loc.typeId != InventoryLocationType.CARGO && loc.typeId != InventoryLocationType.PROXYCARGO)
            return false;

        if (loc.parentClassName == "")
            return false;

        return !loc.hasPlayerOwner;
    }

    /**
    	\brief Builds raw location.
    	\param loc \p loc used by the operation.
    	\return \p The resulting string value.
    */
    protected static string BuildRawLocation(ServerSentryItemMoveLocationSnapshot loc)
    {
        if (!loc || !loc.valid)
        {
            return "invalid";
        }

        string raw = loc.typeId.ToString() + ":" + loc.parentId.ToString();

        if (loc.typeId == InventoryLocationType.ATTACHMENT)
        {
            raw += ":slot=" + loc.slotId.ToString();
        }
        else if (loc.typeId == InventoryLocationType.CARGO || loc.typeId == InventoryLocationType.PROXYCARGO)
        {
            raw += ":idx=" + loc.cargoIdx.ToString();
            raw += ":row=" + loc.row.ToString();
            raw += ":col=" + loc.col.ToString();
            raw += ":flip=" + BoolToString(loc.flip);
        }
        else if (loc.typeId == InventoryLocationType.GROUND)
        {
            raw += ":pos=" + loc.position.ToString();
        }

        return raw;
    }

    /**
    	\brief Processes the location type name operation.
    	\param typeId \p type identifier used by the operation.
    	\return \p The resulting string value.
    */
    protected static string LocationTypeName(int typeId)
    {
        if (typeId == InventoryLocationType.GROUND)
            return "ground";
        if (typeId == InventoryLocationType.HANDS)
            return "hands";
        if (typeId == InventoryLocationType.ATTACHMENT)
            return "attachment";
        if (typeId == InventoryLocationType.CARGO)
            return "cargo";
        if (typeId == InventoryLocationType.PROXYCARGO)
            return "proxycargo";
        return "unknown";
    }

    /**
    	\brief Processes the bool to string operation.
    	\param value \p value to process or assign.
    	\return \p The resulting string value.
    */
    protected static string BoolToString(bool value)
    {
        if (value)
            return "1";
        return "0";
    }

    /**
    	\brief Builds event from snapshot.
    	\param snapshot \p snapshot used by the operation.
    	\return \p The resulting event instance, or null when unavailable.
    */
    protected static ServerSentryEvent BuildEventFromSnapshot(ServerSentryItemMoveSnapshot snapshot)
    {
        if (!snapshot)
        {
            return null;
        }

        ServerSentryItemMoveEvent ev = new ServerSentryItemMoveEvent();
        ev.schemaVersion = ServerSentryDefaults.SchemaVersion();
        ev.eventType = "item_move";
        ev.category = "item";
        ev.severity = "info";
        ev.mod = "ServerSentry";

        ev.action.name = snapshot.actionName;
        ev.action.displayName = snapshot.actionName;
        ev.action.result = "success";

        ev.item.className = snapshot.itemClassName;
        ev.item.displayName = snapshot.itemDisplayName;
        ev.item.persistentId = snapshot.itemPersistentId;
        // Use the captured destination, never an actor position or a default cargo vector.
        vector observedItemPosition = snapshot.itemPosition;
        if (snapshot.to.typeId == InventoryLocationType.GROUND)
            observedItemPosition = snapshot.to.position;
        else if (snapshot.to.hasPlayerOwner)
            observedItemPosition = snapshot.to.parentOwnerPosition;
        else if (snapshot.to.parentId >= 0)
            observedItemPosition = snapshot.to.parentPosition;
        ev.item.position = new ServerSentryVectorDto();
        ev.item.position.FromVector(observedItemPosition);
        ev.item.health = -1;
        ev.item.quantity = 0;

        ev.actor.type = snapshot.actorType;
        ev.actor.entityKind = snapshot.actorKind;
        ev.actor.className = snapshot.actorClassName;
        ev.actor.steamId = snapshot.actorSteamId;
        ev.actor.bohemiaId = snapshot.actorBohemiaId;
        ev.actor.name = snapshot.actorName;

        ev.target.type = snapshot.targetType;
        ev.target.entityKind = snapshot.targetKind;
        ev.target.className = snapshot.targetClassName;
        ev.target.name = snapshot.targetName;

        ev.position.FromVector(snapshot.actorPosition);

        // Keep the established occurrence-ID format for existing consumers. The
        // restart-stable item identity is exposed separately in item/data.
        ev.data.moveId = "move-" + snapshot.runtimeItemId.ToString() + "-" + snapshot.gameTime.ToString() + "-trace" + snapshot.moveId.ToString();
        ev.data.sourceHook = "ItemBase.EEItemLocationChanged";
        ev.data.itemKind = snapshot.itemKind;
        ev.data.itemClassName = snapshot.itemClassName;
        ev.data.itemDisplayName = snapshot.itemDisplayName;
        ev.data.itemPersistentId = snapshot.itemPersistentId;
        ev.data.runtimeItemId = snapshot.runtimeItemId;
        ev.data.actorInference = snapshot.actorInference;
        ev.data.countsForPlayerItemStats = snapshot.actorType == "player" || snapshot.actorType == "ai";

        FillLocationData(ev.data.from, snapshot.from);
        FillLocationData(ev.data.to, snapshot.to);

        ev.data.relation = ev.data.from.type + "_to_" + ev.data.to.type;
        ev.data.fromLabel = BuildLocationLabel(ev.data.from);
        ev.data.toLabel = BuildLocationLabel(ev.data.to);

        if (ev.data.from.type == "storage")
        {
            ev.data.sourceStorageClassName = ev.data.from.parentClassName;
            ev.data.sourceStorageName = ev.data.from.parentName;
            ev.data.sourceStorageRaw = ev.data.from.raw;
        }

        if (ev.data.to.type == "storage")
        {
            ev.data.destinationStorageClassName = ev.data.to.parentClassName;
            ev.data.destinationStorageName = ev.data.to.parentName;
            ev.data.destinationStorageRaw = ev.data.to.raw;
        }

        ev.dataJson = "";
        ev.message = snapshot.actionName + ": " + snapshot.itemDisplayName + " from " + ev.data.fromLabel + " to " + ev.data.toLabel;
        return ev;
    }

    /**
        \brief Builds a canonical ID from DayZ's four restart-stable persistence blocks.
        \param item \p item whose persistence ID is captured.
        \return \p A canonical ID, or an empty string when DayZ has not assigned one.
    */
    static string ResolveItemPersistentId(EntityAI item)
    {
        if (!item)
        {
            return "";
        }

        int b1;
        int b2;
        int b3;
        int b4;
        item.GetPersistentID(b1, b2, b3, b4);

        if (b1 == 0 && b2 == 0 && b3 == 0 && b4 == 0)
        {
            return "";
        }

        return "dayz-pid-v1:" + b1.ToString() + ":" + b2.ToString() + ":" + b3.ToString() + ":" + b4.ToString();
    }

    /**
    	\brief Processes the fill location data operation.
    	\param data \p data processed by the operation.
    	\param snapshot \p snapshot used by the operation.
    */
    protected static void FillLocationData(ServerSentryItemLocationData data, ServerSentryItemMoveLocationSnapshot snapshot)
    {
        if (!data || !snapshot)
        {
            return;
        }

        data.typeId = snapshot.typeId;
        data.type = snapshot.typeName;
        if (snapshot.isExternalStorage)
        {
            data.type = "storage";
        }
        else if (snapshot.typeId == InventoryLocationType.ATTACHMENT && snapshot.parentIsPlayer)
        {
            data.type = "player_slot";
        }

        data.parentClassName = snapshot.parentClassName;
        data.parentName = snapshot.parentName;
        if (snapshot.parentId >= 0)
        {
            data.parentObjectId = snapshot.parentId.ToString();
        }
        data.parentOwnerType = snapshot.parentOwnerType;
        data.parentOwnerKind = snapshot.parentOwnerKind;
        data.parentOwnerClassName = snapshot.parentOwnerClassName;
        data.parentOwnerSteamId = snapshot.parentOwnerSteamId;
        data.parentOwnerBohemiaId = snapshot.parentOwnerBohemiaId;
        data.parentOwnerName = snapshot.parentOwnerName;

        data.slotId = snapshot.slotId;
        data.slotName = snapshot.slotName;
        data.slotDisplayName = snapshot.slotDisplayName;
        data.cargoIdx = snapshot.cargoIdx;
        data.row = snapshot.row;
        data.col = snapshot.col;
        data.flip = snapshot.flip;
        data.position.FromVector(snapshot.position);
        data.raw = snapshot.raw;
    }

    /**
    	\brief Builds location label.
    	\param loc \p loc used by the operation.
    	\return \p The resulting string value.
    */
    protected static string BuildLocationLabel(ServerSentryItemLocationData loc)
    {
        if (!loc)
        {
            return "unknown";
        }

        if (loc.type == "attachment")
        {
            string parentLabel = loc.parentName;
            if (parentLabel == "")
                parentLabel = loc.parentClassName;

            string slotLabel = loc.slotDisplayName;
            if (slotLabel == "")
                slotLabel = loc.slotName;

            return "attachment:" + parentLabel + " slot:" + slotLabel;
        }

        if (loc.type == "storage")
        {
            if (loc.parentName != "")
                return "storage:" + loc.parentName;
            if (loc.parentClassName != "")
                return "storage:" + loc.parentClassName;
            return "storage";
        }

        if (loc.parentOwnerName != "")
        {
            if (loc.type == "hands")
                return loc.parentOwnerName + " hands";
            if (loc.type == "player_slot")
            {
                string playerSlotLabel = loc.slotDisplayName;
                if (playerSlotLabel == "")
                    playerSlotLabel = loc.slotName;
                return loc.parentOwnerName + " slot:" + playerSlotLabel;
            }
            if (loc.type == "cargo")
                return loc.parentOwnerName + " inventory";
            return loc.parentOwnerName + " " + loc.type;
        }

        if (loc.type == "ground")
        {
            return "ground";
        }

        if (loc.parentName != "")
        {
            return loc.type + ":" + loc.parentName;
        }

        if (loc.parentClassName != "")
        {
            return loc.type + ":" + loc.parentClassName;
        }

        return loc.type;
    }
}
