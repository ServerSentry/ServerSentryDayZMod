/**
	\file ServerSentryInventoryDebug.c
	\brief Provides logging support for Inventory debug.
*/

/**
	\brief Provides ServerSentry behavior for Inventory debug.
*/
class ServerSentryInventoryDebug
{
    /**
    	\brief Shared trace counter state used by all instances.
    */
    protected static int s_TraceCounter;

    /**
    	\brief Processes the next trace ID operation.
    	\return \p The resulting integer value or status code.
    */
    static int NextTraceId()
    {
        s_TraceCounter++;
        return s_TraceCounter;
    }

    /**
    	\brief Processes the print line operation.
    	\param message \p receives a human-readable result or error message.
    */
    static void PrintLine(string message)
    {
        if (!ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            return;
        }

        Print("[ServerSentry][InventoryDebug] " + message);
    }

    /**
    	\brief Processes the print trace operation.
    	\param traceId \p identifier of the trace.
    	\param step \p increment used by the numeric input control.
    	\param message \p receives a human-readable result or error message.
    */
    static void PrintTrace(int traceId, string step, string message = "")
    {
        if (!ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            return;
        }

        string prefix = "move#" + traceId.ToString();
        if (GetGame())
        {
            prefix += " t=" + GetGame().GetTime().ToString();
        }
        else
        {
            prefix += " t=no-game";
        }

        if (message != "")
        {
            Print("[ServerSentry][InventoryDebug][" + prefix + "][" + step + "] " + message);
        }
        else
        {
            Print("[ServerSentry][InventoryDebug][" + prefix + "][" + step + "]");
        }
    }

    /**
    	\brief Processes the print trace locations operation.
    	\param traceId \p identifier of the trace.
    	\param step \p increment used by the numeric input control.
    	\param item \p item or value processed by the operation.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    */
    static void PrintTraceLocations(int traceId, string step, EntityAI item, InventoryLocation oldLoc, InventoryLocation newLoc)
    {
        if (!ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            return;
        }

        PrintTrace(traceId, step, ItemInfo(item) + " | old=" + LocationInfo(oldLoc) + " | new=" + LocationInfo(newLoc));
    }

    /**
    	\brief Processes the bool to string operation.
    	\param value \p value to process or assign.
    	\return \p The resulting string value.
    */
    static string BoolToString(bool value)
    {
        if (value)
        {
            return "true";
        }

        return "false";
    }

    /**
    	\brief Processes the item info operation.
    	\param item \p item or value processed by the operation.
    	\return \p The resulting string value.
    */
    static string ItemInfo(EntityAI item)
    {
        if (!item)
        {
            return "item=null";
        }

        string value = "item=" + item.GetType() + " id=" + item.GetID().ToString();
        string displayName = item.GetDisplayName();
        if (displayName != "")
        {
            value += " name=" + displayName;
        }

        value += " pos=" + item.GetPosition().ToString();
        return value;
    }

    /**
    	\brief Processes the entity info operation.
    	\param entity \p entity processed by the operation.
    	\return \p The resulting string value.
    */
    static string EntityInfo(EntityAI entity)
    {
        if (!entity)
        {
            return "null";
        }

        string value = entity.GetType() + "#" + entity.GetID().ToString();
        string displayName = entity.GetDisplayName();
        if (displayName != "")
        {
            value += "(" + displayName + ")";
        }

        return value;
    }

    /**
    	\brief Processes the player info operation.
    	\param player \p player entity associated with the operation.
    	\return \p The resulting string value.
    */
    static string PlayerInfo(PlayerBase player)
    {
        if (!player)
        {
            return "player=null";
        }

        string value = "player=" + player.GetType() + "#" + player.GetID().ToString();
        PlayerIdentity identity = player.GetIdentity();
        if (identity)
        {
            value += " name=" + identity.GetName();
            value += " steam=" + identity.GetPlainId();
            value += " bohemia=" + identity.GetId();
        }
        else
        {
            value += " identity=null";
        }

        value += " pos=" + player.GetPosition().ToString();
        return value;
    }

    /**
    	\brief Processes the location info operation.
    	\param loc \p loc used by the operation.
    	\return \p The resulting string value.
    */
    static string LocationInfo(InventoryLocation loc)
    {
        if (!loc)
        {
            return "loc=null";
        }

        if (!loc.IsValid())
        {
            return "loc=invalid";
        }

        int typeId = loc.GetType();
        string value = "type=" + LocationTypeName(typeId);

        EntityAI parent = loc.GetParent();
        value += " parent=" + EntityInfo(parent);

        if (typeId == InventoryLocationType.ATTACHMENT)
        {
            int slotId = loc.GetSlot();
            value += " slot=" + slotId.ToString();
            if (InventorySlots.IsSlotIdValid(slotId))
            {
                value += " slotName=" + InventorySlots.GetSlotName(slotId);
            }
            else
            {
                value += " slotName=invalid";
            }
        }
        else if (typeId == InventoryLocationType.CARGO || typeId == InventoryLocationType.PROXYCARGO)
        {
            value += " idx=" + loc.GetIdx().ToString();
            value += " row=" + loc.GetRow().ToString();
            value += " col=" + loc.GetCol().ToString();
            value += " flip=" + BoolToString(loc.GetFlip());
        }
        else if (typeId == InventoryLocationType.GROUND)
        {
            value += " pos=" + loc.GetPos().ToString();
        }
        else if (typeId == InventoryLocationType.HANDS)
        {
            value += " hands=true";
        }

        return value;
    }

    /**
    	\brief Processes the location type name operation.
    	\param typeId \p type identifier used by the operation.
    	\return \p The resulting string value.
    */
    static string LocationTypeName(int typeId)
    {
        if (typeId == InventoryLocationType.GROUND)
        {
            return "ground";
        }

        if (typeId == InventoryLocationType.ATTACHMENT)
        {
            return "attachment";
        }

        if (typeId == InventoryLocationType.CARGO)
        {
            return "cargo";
        }

        if (typeId == InventoryLocationType.HANDS)
        {
            return "hands";
        }

        if (typeId == InventoryLocationType.PROXYCARGO)
        {
            return "proxy_cargo";
        }

        return "unknown(" + typeId.ToString() + ")";
    }
}
