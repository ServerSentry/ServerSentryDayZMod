/**
	\file ServerSentryInventoryLifecycle.c
	\brief Provides logging support for Inventory lifecycle.
*/

/**
	\brief Tracks lifecycle events for Inventory.
*/
class ServerSentryInventoryLifecycle
{
    /**
    	\brief Shared recent moves state used by all instances.
    */
    protected static ref map<string, int> s_RecentMoves;

    /**
    	\brief Handles the DayZ Item location changed callback.
    	\param item \p item or value processed by the operation.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    	\param sourceHook \p source hook used by the operation.
    	\param traceId \p identifier of the trace.
    */
    static void OnItemLocationChanged(EntityAI item, InventoryLocation oldLoc, InventoryLocation newLoc, string sourceHook = "unknown", int traceId = 0)
    {
        if (traceId <= 0)
        {
            traceId = ServerSentryInventoryDebug.NextTraceId();
        }

        ServerSentryInventoryDebug.PrintTraceLocations(traceId, "InventoryLifecycle.OnItemLocationChanged.enter", item, oldLoc, newLoc);
        ServerSentryEvent ev = CreateItemMoveEvent(item, oldLoc, newLoc, sourceHook, traceId);
        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.OnItemLocationChanged.afterCreate", "event=" + ServerSentryInventoryDebug.BoolToString(ev != null));

        if (ev)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.OnItemLocationChanged.beforeEmit", "eventType=" + ev.eventType + " action=" + ev.action.name);
            bool emitResult = ServerSentryLogger.Get().Emit(ev);
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.OnItemLocationChanged.afterEmit", "result=" + ServerSentryInventoryDebug.BoolToString(emitResult));
        }
    }

    /**
    	\brief Creates item move event.
    	\param item \p item or value processed by the operation.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    	\param sourceHook \p source hook used by the operation.
    	\param traceId \p identifier of the trace.
    	\return \p The resulting event instance, or null when unavailable.
    */
    static ServerSentryEvent CreateItemMoveEvent(EntityAI item, InventoryLocation oldLoc, InventoryLocation newLoc, string sourceHook = "unknown", int traceId = 0)
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return null;
        }

        if (!ServerSentryDefaults.LogItemMoveEvents())
        {
            return null;
        }

        ItemBase itemBase = ItemBase.Cast(item);
        if (!itemBase)
        {
            return null;
        }

        // Safety rule: do not pass live InventoryLocation/EntityAI references into
        // the old event factory. Capture a primitive snapshot immediately and
        // return a pure DTO event.
        return ServerSentryItemMoveScriptLog.CreatePrimitiveSnapshotEvent(traceId, itemBase, oldLoc, newLoc);
    }

    /**
    	\brief Returns whether skip move.
    	\param item \p item or value processed by the operation.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool ShouldSkipMove(EntityAI item, InventoryLocation oldLoc, InventoryLocation newLoc, int traceId = 0)
    {
        ServerSentryInventoryDebug.PrintTraceLocations(traceId, "InventoryLifecycle.ShouldSkipMove.enter", item, oldLoc, newLoc);

        if (!item)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.ShouldSkipMove.true", "item=null");
            return true;
        }

        if (!oldLoc)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.ShouldSkipMove.true", "oldLoc=null");
            return true;
        }

        if (!newLoc)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.ShouldSkipMove.true", "newLoc=null");
            return true;
        }

        if (!oldLoc.IsValid())
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.ShouldSkipMove.true", "oldLoc invalid");
            return true;
        }

        if (!newLoc.IsValid())
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.ShouldSkipMove.true", "newLoc invalid");
            return true;
        }

        // Do not skip ATTACHMENT globally. Player Shoulder/Back/Melee slots are ATTACHMENT
        // locations and must count as player inventory for storage_take/storage_put audits.
        // They are filtered later by IsTrackedInventoryMove()/HasPlayerActor().

        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.ShouldSkipMove.beforeIsSameLocation");
        if (IsSameLocation(oldLoc, newLoc, traceId))
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.ShouldSkipMove.true", "same location");
            return true;
        }
        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.ShouldSkipMove.afterIsSameLocation", "same=false");

        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.ShouldSkipMove.beforeIsTrackedInventoryMove");
        if (!IsTrackedInventoryMove(oldLoc, newLoc, traceId))
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.ShouldSkipMove.true", "not tracked inventory move");
            return true;
        }
        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.ShouldSkipMove.afterIsTrackedInventoryMove", "tracked=true");

        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.ShouldSkipMove.beforeHasPlayerActor", "LogOnlyPlayerItemMoveEvents=" + ServerSentryInventoryDebug.BoolToString(ServerSentryDefaults.LogOnlyPlayerItemMoveEvents()));
        if (ServerSentryDefaults.LogOnlyPlayerItemMoveEvents() && !HasPlayerActor(oldLoc, newLoc, traceId))
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.ShouldSkipMove.true", "player actor missing");
            return true;
        }

        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.ShouldSkipMove.false", "continue");
        return false;
    }

    /**
    	\brief Returns whether tracked inventory move.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsTrackedInventoryMove(InventoryLocation oldLoc, InventoryLocation newLoc, int traceId = 0)
    {
        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsTrackedInventoryMove.enter", "old=" + ServerSentryInventoryDebug.LocationInfo(oldLoc) + " | new=" + ServerSentryInventoryDebug.LocationInfo(newLoc));

        bool fromGround = IsGroundLocation(oldLoc, traceId, "old");
        bool toGround = IsGroundLocation(newLoc, traceId, "new");
        bool fromStorage = IsExternalStorageLocation(oldLoc, traceId, "old");
        bool toStorage = IsExternalStorageLocation(newLoc, traceId, "new");

        PlayerBase oldPlayer = GetPlayerFromLocation(oldLoc, traceId, "old");
        PlayerBase newPlayer = GetPlayerFromLocation(newLoc, traceId, "new");

        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsTrackedInventoryMove.flags", "fromGround=" + ServerSentryInventoryDebug.BoolToString(fromGround) + " toGround=" + ServerSentryInventoryDebug.BoolToString(toGround) + " fromStorage=" + ServerSentryInventoryDebug.BoolToString(fromStorage) + " toStorage=" + ServerSentryInventoryDebug.BoolToString(toStorage) + " oldPlayer=" + ServerSentryInventoryDebug.BoolToString(oldPlayer != null) + " newPlayer=" + ServerSentryInventoryDebug.BoolToString(newPlayer != null));

        // Ground <-> player inventory/hands.
        if (fromGround && newPlayer)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsTrackedInventoryMove.true", "ground_to_player");
            return true;
        }

        if (oldPlayer && toGround)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsTrackedInventoryMove.true", "player_to_ground");
            return true;
        }

        // Player inventory/hands <-> external storage.
        if (oldPlayer && toStorage)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsTrackedInventoryMove.true", "player_to_storage");
            return true;
        }

        if (fromStorage && newPlayer)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsTrackedInventoryMove.true", "storage_to_player");
            return true;
        }

        // External storage <-> external storage. Actor may be inferred poorly, but it is useful for admin audit.
        if (fromStorage && toStorage)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsTrackedInventoryMove.true", "storage_to_storage");
            return true;
        }

        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsTrackedInventoryMove.false");
        return false;
    }

    /**
    	\brief Returns whether player actor.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool HasPlayerActor(InventoryLocation oldLoc, InventoryLocation newLoc, int traceId = 0)
    {
        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.HasPlayerActor.enter");

        PlayerBase oldPlayer = GetPlayerFromLocation(oldLoc, traceId, "old");
        if (oldPlayer)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.HasPlayerActor.true", "old=" + ServerSentryInventoryDebug.PlayerInfo(oldPlayer));
            return true;
        }

        PlayerBase newPlayer = GetPlayerFromLocation(newLoc, traceId, "new");
        if (newPlayer)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.HasPlayerActor.true", "new=" + ServerSentryInventoryDebug.PlayerInfo(newPlayer));
            return true;
        }

        // Storage-to-storage can happen from player inventory actions where DayZ does not expose the actor here.
        // Keep it only when the admin explicitly disables player-only item logs.
        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.HasPlayerActor.false");
        return false;
    }

    /**
    	\brief Returns whether ground location.
    	\param loc \p loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\param label \p human-readable label shown to users.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsGroundLocation(InventoryLocation loc, int traceId = 0, string label = "")
    {
        bool result = loc && loc.IsValid() && loc.GetType() == InventoryLocationType.GROUND;
        if (traceId > 0)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsGroundLocation." + label, "result=" + ServerSentryInventoryDebug.BoolToString(result) + " loc=" + ServerSentryInventoryDebug.LocationInfo(loc));
        }

        return result;
    }

    /**
    	\brief Returns whether attachment location.
    	\param loc \p loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\param label \p human-readable label shown to users.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsAttachmentLocation(InventoryLocation loc, int traceId = 0, string label = "")
    {
        bool result = loc && loc.IsValid() && loc.GetType() == InventoryLocationType.ATTACHMENT;
        if (traceId > 0)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsAttachmentLocation." + label, "result=" + ServerSentryInventoryDebug.BoolToString(result) + " loc=" + ServerSentryInventoryDebug.LocationInfo(loc));
        }

        return result;
    }

    /**
    	\brief Returns whether player inventory location.
    	\param loc \p loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\param label \p human-readable label shown to users.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsPlayerInventoryLocation(InventoryLocation loc, int traceId = 0, string label = "")
    {
        PlayerBase player = GetPlayerFromLocation(loc, traceId, label);
        bool result = player != null;
        if (traceId > 0)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsPlayerInventoryLocation." + label, "result=" + ServerSentryInventoryDebug.BoolToString(result));
        }

        return result;
    }

    /**
    	\brief Returns whether cargo location.
    	\param loc \p loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\param label \p human-readable label shown to users.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsCargoLocation(InventoryLocation loc, int traceId = 0, string label = "")
    {
        if (!loc)
        {
            if (traceId > 0)
            {
                ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsCargoLocation." + label, "result=false loc=null");
            }
            return false;
        }

        if (!loc.IsValid())
        {
            if (traceId > 0)
            {
                ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsCargoLocation." + label, "result=false loc=invalid");
            }
            return false;
        }

        int typeId = loc.GetType();
        bool result = typeId == InventoryLocationType.CARGO || typeId == InventoryLocationType.PROXYCARGO;
        if (traceId > 0)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsCargoLocation." + label, "result=" + ServerSentryInventoryDebug.BoolToString(result) + " type=" + ServerSentryInventoryDebug.LocationTypeName(typeId));
        }

        return result;
    }

    /**
    	\brief Returns whether external storage location.
    	\param loc \p loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\param label \p human-readable label shown to users.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsExternalStorageLocation(InventoryLocation loc, int traceId = 0, string label = "")
    {
        if (traceId > 0)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsExternalStorageLocation.enter." + label, ServerSentryInventoryDebug.LocationInfo(loc));
        }

        if (!IsCargoLocation(loc, traceId, label))
        {
            if (traceId > 0)
            {
                ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsExternalStorageLocation.false." + label, "not cargo/proxycargo");
            }
            return false;
        }

        EntityAI parent = loc.GetParent();
        if (!parent)
        {
            if (traceId > 0)
            {
                ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsExternalStorageLocation.false." + label, "parent=null");
            }
            return false;
        }

        // Cargo on player-owned clothing/backpacks/weapons is still player inventory, not external storage.
        PlayerBase rootPlayer = GetPlayerFromEntity(parent, traceId, label + ".parent");
        if (rootPlayer)
        {
            if (traceId > 0)
            {
                ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsExternalStorageLocation.false." + label, "rootPlayer=" + ServerSentryInventoryDebug.PlayerInfo(rootPlayer));
            }
            return false;
        }

        if (traceId > 0)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsExternalStorageLocation.true." + label, "parent=" + ServerSentryInventoryDebug.EntityInfo(parent));
        }
        return true;
    }

    /**
    	\brief Returns whether duplicate move.
    	\param item \p item or value processed by the operation.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsDuplicateMove(EntityAI item, InventoryLocation oldLoc, InventoryLocation newLoc, int traceId = 0)
    {
        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsDuplicateMove.enter");

        if (!GetGame())
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsDuplicateMove.false", "GetGame=null");
            return false;
        }

        if (!s_RecentMoves)
        {
            s_RecentMoves = new map<string, int>();
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsDuplicateMove.initMap");
        }

        string signature = BuildMoveSignature(item, oldLoc, newLoc, traceId);
        int now = GetGame().GetTime();
        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsDuplicateMove.signature", "sig=" + signature + " now=" + now.ToString());

        if (s_RecentMoves.Contains(signature))
        {
            int lastSeen = s_RecentMoves.Get(signature);
            int age = now - lastSeen;
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsDuplicateMove.lastSeen", "last=" + lastSeen.ToString() + " age=" + age.ToString() + " window=" + ServerSentryDefaults.ItemMoveDedupWindowMs().ToString());
            if (age <= ServerSentryDefaults.ItemMoveDedupWindowMs())
            {
                ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsDuplicateMove.true");
                return true;
            }
        }

        s_RecentMoves.Set(signature, now);
        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsDuplicateMove.false", "stored signature");
        return false;
    }

    /**
    	\brief Returns whether same location.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsSameLocation(InventoryLocation oldLoc, InventoryLocation newLoc, int traceId = 0)
    {
        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsSameLocation.enter", "old=" + ServerSentryInventoryDebug.LocationInfo(oldLoc) + " | new=" + ServerSentryInventoryDebug.LocationInfo(newLoc));

        if (!oldLoc || !newLoc)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsSameLocation.false", "null location");
            return false;
        }

        if (!oldLoc.IsValid() || !newLoc.IsValid())
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsSameLocation.false", "invalid location");
            return false;
        }

        int oldType = oldLoc.GetType();
        int newType = newLoc.GetType();

        if (oldType != newType)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsSameLocation.false", "type differs old=" + ServerSentryInventoryDebug.LocationTypeName(oldType) + " new=" + ServerSentryInventoryDebug.LocationTypeName(newType));
            return false;
        }

        EntityAI oldParent = oldLoc.GetParent();
        EntityAI newParent = newLoc.GetParent();
        if (oldParent != newParent)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsSameLocation.false", "parent differs old=" + ServerSentryInventoryDebug.EntityInfo(oldParent) + " new=" + ServerSentryInventoryDebug.EntityInfo(newParent));
            return false;
        }

        if (oldType == InventoryLocationType.ATTACHMENT)
        {
            bool sameSlot = oldLoc.GetSlot() == newLoc.GetSlot();
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsSameLocation.attachment", "result=" + ServerSentryInventoryDebug.BoolToString(sameSlot));
            return sameSlot;
        }

        if (oldType == InventoryLocationType.CARGO || oldType == InventoryLocationType.PROXYCARGO)
        {
            if (oldLoc.GetIdx() != newLoc.GetIdx())
            {
                ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsSameLocation.false", "cargo idx differs");
                return false;
            }

            if (oldLoc.GetRow() != newLoc.GetRow())
            {
                ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsSameLocation.false", "cargo row differs");
                return false;
            }

            if (oldLoc.GetCol() != newLoc.GetCol())
            {
                ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsSameLocation.false", "cargo col differs");
                return false;
            }

            if (oldLoc.GetFlip() != newLoc.GetFlip())
            {
                ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsSameLocation.false", "cargo flip differs");
                return false;
            }

            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsSameLocation.true", "cargo same");
            return true;
        }

        if (oldType == InventoryLocationType.GROUND)
        {
            bool sameGround = oldLoc.GetPos().ToString() == newLoc.GetPos().ToString();
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsSameLocation.ground", "result=" + ServerSentryInventoryDebug.BoolToString(sameGround));
            return sameGround;
        }

        // Hands and unknown types: same parent + same type is enough.
        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.IsSameLocation.true", "same parent and type");
        return true;
    }

    /**
    	\brief Returns player from location.
    	\param loc \p loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\param label \p human-readable label shown to users.
    	\return \p The resulting player base instance, or null when unavailable.
    */
    static PlayerBase GetPlayerFromLocation(InventoryLocation loc, int traceId = 0, string label = "")
    {
        if (traceId > 0)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.GetPlayerFromLocation.enter." + label, ServerSentryInventoryDebug.LocationInfo(loc));
        }

        if (!loc)
        {
            if (traceId > 0)
            {
                ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.GetPlayerFromLocation.null." + label);
            }
            return null;
        }

        if (!loc.IsValid())
        {
            if (traceId > 0)
            {
                ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.GetPlayerFromLocation.invalid." + label);
            }
            return null;
        }

        EntityAI parent = loc.GetParent();
        PlayerBase player = GetPlayerFromEntity(parent, traceId, label + ".parent");
        if (traceId > 0)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.GetPlayerFromLocation.result." + label, ServerSentryInventoryDebug.PlayerInfo(player));
        }

        return player;
    }

    /**
    	\brief Returns player from entity.
    	\param entity \p entity processed by the operation.
    	\param traceId \p identifier of the trace.
    	\param label \p human-readable label shown to users.
    	\return \p The resulting player base instance, or null when unavailable.
    */
    static PlayerBase GetPlayerFromEntity(EntityAI entity, int traceId = 0, string label = "")
    {
        if (traceId > 0)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.GetPlayerFromEntity.enter." + label, "entity=" + ServerSentryInventoryDebug.EntityInfo(entity));
        }

        if (!entity)
        {
            if (traceId > 0)
            {
                ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.GetPlayerFromEntity.null." + label);
            }
            return null;
        }

        PlayerBase direct = PlayerBase.Cast(entity);
        if (direct)
        {
            if (traceId > 0)
            {
                ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.GetPlayerFromEntity.direct." + label, ServerSentryInventoryDebug.PlayerInfo(direct));
            }
            return direct;
        }

        Man root = entity.GetHierarchyRootPlayer();
        PlayerBase rootPlayer = PlayerBase.Cast(root);
        if (traceId > 0)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.GetPlayerFromEntity.root." + label, ServerSentryInventoryDebug.PlayerInfo(rootPlayer));
        }

        return rootPlayer;
    }

    /**
    	\brief Builds move signature.
    	\param item \p item or value processed by the operation.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\return \p The resulting string value.
    */
    static string BuildMoveSignature(EntityAI item, InventoryLocation oldLoc, InventoryLocation newLoc, int traceId = 0)
    {
        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.BuildMoveSignature.enter");
        string signature = "unknown";

        if (item)
        {
            signature = item.GetID().ToString();
        }

        signature += "|" + BuildLocationSignature(oldLoc, traceId, "old");
        signature += "|" + BuildLocationSignature(newLoc, traceId, "new");
        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.BuildMoveSignature.result", signature);
        return signature;
    }

    /**
    	\brief Builds location signature.
    	\param loc \p loc used by the operation.
    	\param traceId \p identifier of the trace.
    	\param label \p human-readable label shown to users.
    	\return \p The resulting string value.
    */
    static string BuildLocationSignature(InventoryLocation loc, int traceId = 0, string label = "")
    {
        if (traceId > 0)
        {
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.BuildLocationSignature.enter." + label, ServerSentryInventoryDebug.LocationInfo(loc));
        }

        if (!loc || !loc.IsValid())
        {
            if (traceId > 0)
            {
                ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.BuildLocationSignature.result." + label, "invalid");
            }
            return "invalid";
        }

        int typeId = loc.GetType();
        string value = typeId.ToString();
        EntityAI parent = loc.GetParent();
        if (parent)
        {
            value += ":" + parent.GetID().ToString();
        }
        else
        {
            value += ":none";
        }

        if (typeId == InventoryLocationType.ATTACHMENT)
        {
            value += ":slot=" + loc.GetSlot().ToString();
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.BuildLocationSignature.result." + label, value);
            return value;
        }

        if (typeId == InventoryLocationType.CARGO || typeId == InventoryLocationType.PROXYCARGO)
        {
            value += ":idx=" + loc.GetIdx().ToString();
            value += ":row=" + loc.GetRow().ToString();
            value += ":col=" + loc.GetCol().ToString();
            if (loc.GetFlip())
            {
                value += ":flip=1";
            }
            else
            {
                value += ":flip=0";
            }
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.BuildLocationSignature.result." + label, value);
            return value;
        }

        if (typeId == InventoryLocationType.GROUND)
        {
            value += ":pos=" + loc.GetPos().ToString();
            ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.BuildLocationSignature.result." + label, value);
            return value;
        }

        ServerSentryInventoryDebug.PrintTrace(traceId, "InventoryLifecycle.BuildLocationSignature.result." + label, value);
        return value;
    }
}
