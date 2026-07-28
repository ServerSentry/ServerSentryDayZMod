/**
	\file ServerSentryBuildActionLifecycle.c
	\brief Provides logging support for Build action lifecycle.
*/

/**
	\brief Tracks lifecycle events for Build action.
*/
class ServerSentryBuildActionLifecycle
{
    /**
    	\brief Shared recent build actions state used by all instances.
    */
    protected static ref map<string, int> s_RecentBuildActions;

    /**
    	\brief Creates base building snapshot.
    	\param player \p player entity associated with the operation.
    	\param actionData \p action data used by the operation.
    	\param actionName \p action name used by the operation.
    	\param displayName \p display name used by the operation.
    	\param actionClass \p action class used by the operation.
    	\param actionKind \p action kind used by the operation.
    	\param sourceHook \p source hook used by the operation.
    	\return \p The resulting event instance, or null when unavailable.
    */
    static ServerSentryEvent CreateBaseBuildingSnapshot(PlayerBase player, ActionData actionData, string actionName, string displayName, string actionClass, string actionKind, string sourceHook)
    {
        if (!ShouldLogBaseBuilding(player, actionData))
        {
            return null;
        }

        ServerSentryEvent ev = ServerSentryBuildActionFactory.CreateBaseBuildingAction(player, actionData, actionName, displayName, actionClass, actionKind, sourceHook);
        if (IsDuplicate(ev))
        {
            return null;
        }

        return ev;
    }

    /**
    	\brief Creates placement snapshot.
    	\param player \p player entity associated with the operation.
    	\param actionData \p action data used by the operation.
    	\param placedEntity \p placed entity used by the operation.
    	\param placementPosition \p placement position used by the operation.
    	\param placementOrientation \p placement orientation used by the operation.
    	\param actionClass \p action class used by the operation.
    	\param sourceHook \p source hook used by the operation.
    	\return \p The resulting event instance, or null when unavailable.
    */
    static ServerSentryEvent CreatePlacementSnapshot(PlayerBase player, ActionData actionData, EntityAI placedEntity, vector placementPosition, vector placementOrientation, string actionClass, string sourceHook)
    {
        if (!ShouldLogPlacement(player, actionData, placedEntity))
        {
            return null;
        }

        ServerSentryEvent ev = ServerSentryBuildActionFactory.CreatePlacementAction(player, actionData, placedEntity, placementPosition, placementOrientation, actionClass, sourceHook);
        if (IsDuplicate(ev))
        {
            return null;
        }

        return ev;
    }

    /**
    	\brief Emits snapshot.
    	\param ev \p ev used by the operation.
    */
    static void EmitSnapshot(ServerSentryEvent ev)
    {
        if (!ev)
        {
            return;
        }

        ServerSentryLogger.Get().Emit(ev);
    }

    /**
    	\brief Returns whether log base building.
    	\param player \p player entity associated with the operation.
    	\param actionData \p action data used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool ShouldLogBaseBuilding(PlayerBase player, ActionData actionData)
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return false;
        }

        if (!ServerSentryDefaults.LogBaseBuildingActionEvents())
        {
            return false;
        }

        if (!player || !actionData)
        {
            return false;
        }

        return true;
    }

    /**
    	\brief Returns whether log placement.
    	\param player \p player entity associated with the operation.
    	\param actionData \p action data used by the operation.
    	\param placedEntity \p placed entity used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool ShouldLogPlacement(PlayerBase player, ActionData actionData, EntityAI placedEntity)
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return false;
        }

        if (!ServerSentryDefaults.LogPlacementActionEvents())
        {
            return false;
        }

        if (!player || !actionData || !placedEntity)
        {
            return false;
        }

        return true;
    }

    /**
    	\brief Returns whether duplicate.
    	\param ev \p ev used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsDuplicate(ServerSentryEvent ev)
    {
        if (!ev || !GetGame())
        {
            return false;
        }

        if (!s_RecentBuildActions)
        {
            s_RecentBuildActions = new map<string, int>();
        }

        string signature = BuildEventSignature(ev);
        int now = GetGame().GetTime();

        if (s_RecentBuildActions.Contains(signature))
        {
            int lastSeen = s_RecentBuildActions.Get(signature);
            if ((now - lastSeen) <= ServerSentryDefaults.BuildActionDedupWindowMs())
            {
                return true;
            }
        }

        s_RecentBuildActions.Set(signature, now);
        return false;
    }

    /**
    	\brief Builds event signature.
    	\param ev \p ev used by the operation.
    	\return \p The resulting string value.
    */
    static string BuildEventSignature(ServerSentryEvent ev)
    {
        string sig = ev.eventType + "|" + ev.action.name + "|" + ev.actor.steamId + "|" + ev.target.className + "|" + ev.position.x.ToString() + ":" + ev.position.y.ToString() + ":" + ev.position.z.ToString();
        if (ev.item && ev.item.className != "")
        {
            sig += "|" + ev.item.className;
        }

        return sig;
    }
}
