/**
	\file ServerSentryBuildActionFactory.c
	\brief Provides logging support for Build action factory.
*/

/**
	\brief Creates values used by Build action.
*/
class ServerSentryBuildActionFactory
{
    /**
    	\brief Creates base building action.
    	\param player \p player entity associated with the operation.
    	\param actionData \p action data used by the operation.
    	\param actionName \p action name used by the operation.
    	\param displayName \p display name used by the operation.
    	\param actionClass \p action class used by the operation.
    	\param actionKind \p action kind used by the operation.
    	\param sourceHook \p source hook used by the operation.
    	\return \p The resulting event instance, or null when unavailable.
    */
    static ServerSentryEvent CreateBaseBuildingAction(PlayerBase player, ActionData actionData, string actionName, string displayName, string actionClass, string actionKind, string sourceHook)
    {
        ServerSentryBuildActionEvent ev = new ServerSentryBuildActionEvent();
        ev.schemaVersion = ServerSentryDefaults.SchemaVersion();
        ev.eventType = "basebuilding_action";
        ev.category = "basebuilding";
        ev.severity = "info";
        ev.mod = "ServerSentry";
        ev.action.name = actionName;
        ev.action.displayName = displayName;
        ev.action.result = "success";

        FillPlayerActor(ev, player);

        Object targetObject = GetTargetObject(actionData);
        FillTarget(ev, targetObject);

        ItemBase mainItem = GetMainItem(actionData);
        if (mainItem)
        {
            ServerSentryItemEventFactory.FillItem(ev.item, mainItem);
        }

        ServerSentryBuildActionData data = CreateBuildActionData(player, actionData, targetObject, mainItem, actionName, actionClass, actionKind, sourceHook);
        ev.data = data;
        ev.dataJson = "";
        ev.message = BuildMessage(actionName, player, targetObject, mainItem, data);
        return ev;
    }

    /**
    	\brief Creates placement action.
    	\param player \p player entity associated with the operation.
    	\param actionData \p action data used by the operation.
    	\param placedEntity \p placed entity used by the operation.
    	\param placementPosition \p placement position used by the operation.
    	\param placementOrientation \p placement orientation used by the operation.
    	\param actionClass \p action class used by the operation.
    	\param sourceHook \p source hook used by the operation.
    	\return \p The resulting event instance, or null when unavailable.
    */
    static ServerSentryEvent CreatePlacementAction(PlayerBase player, ActionData actionData, EntityAI placedEntity, vector placementPosition, vector placementOrientation, string actionClass, string sourceHook)
    {
        ServerSentryBuildActionEvent ev = new ServerSentryBuildActionEvent();
        ev.schemaVersion = ServerSentryDefaults.SchemaVersion();
        ev.eventType = "placement_action";
        ev.category = "placement";
        ev.severity = "info";
        ev.mod = "ServerSentry";
        ev.action.name = "place_object";
        ev.action.displayName = "Place object";
        ev.action.result = "success";

        FillPlayerActor(ev, player);

        if (placedEntity)
        {
            ServerSentryEntityClassifier.FillEntity(ev.target, placedEntity);
            ServerSentryItemEventFactory.FillItem(ev.item, placedEntity);
        }

        ev.position.FromVector(placementPosition);

        ItemBase mainItem = GetMainItem(actionData);
        ServerSentryBuildActionData data = CreateBuildActionData(player, actionData, placedEntity, mainItem, "place_object", actionClass, "place_object", sourceHook);
        data.isPlacement = true;
        data.placementPosition.FromVector(placementPosition);
        data.placementOrientation.FromVector(placementOrientation);

        if (placedEntity)
        {
            data.placedClassName = placedEntity.GetType();
            data.placedDisplayName = placedEntity.GetDisplayName();
        }

        ev.data = data;
        ev.dataJson = "";
        ev.message = BuildPlacementMessage(player, placedEntity, data);
        return ev;
    }

    /**
    	\brief Creates build action data.
    	\param player \p player entity associated with the operation.
    	\param actionData \p action data used by the operation.
    	\param targetObject \p target object used by the operation.
    	\param mainItem \p main item used by the operation.
    	\param actionName \p action name used by the operation.
    	\param actionClass \p action class used by the operation.
    	\param actionKind \p action kind used by the operation.
    	\param sourceHook \p source hook used by the operation.
    	\return \p The resulting build action data instance, or null when unavailable.
    */
    static ServerSentryBuildActionData CreateBuildActionData(PlayerBase player, ActionData actionData, Object targetObject, ItemBase mainItem, string actionName, string actionClass, string actionKind, string sourceHook)
    {
        ServerSentryBuildActionData data = new ServerSentryBuildActionData();
        data.actionName = actionName;
        data.actionClass = actionClass;
        data.actionKind = actionKind;
        data.sourceHook = sourceHook;
        data.isBaseBuilding = actionKind.Contains("build") || actionKind.Contains("dismantle") || actionKind.Contains("destroy") || actionKind.Contains("repair");
        data.relation = "player_" + actionKind;

        if (player)
        {
            PlayerIdentity identity = player.GetIdentity();
            if (identity)
            {
                data.actorSteamId = identity.GetPlainId();
                data.actorBohemiaId = identity.GetId();
                data.actorName = identity.GetName();
            }

            data.actorPosition.FromVector(player.GetPosition());
        }

        FillTargetData(data, targetObject, actionData);
        FillMainItemData(data, mainItem);
        return data;
    }

    /**
    	\brief Processes the fill player actor operation.
    	\param ev \p ev used by the operation.
    	\param player \p player entity associated with the operation.
    */
    static void FillPlayerActor(ServerSentryEvent ev, PlayerBase player)
    {
        if (!ev)
        {
            return;
        }

        if (player)
        {
            ServerSentryEntityClassifier.FillPlayer(ev.actor, player, player.GetIdentity());
            ev.position.FromVector(player.GetPosition());
            return;
        }

        ServerSentryEntityClassifier.FillEnvironment(ev.actor, "unknown");
    }

    /**
    	\brief Processes the fill target operation.
    	\param ev \p ev used by the operation.
    	\param targetObject \p target object used by the operation.
    */
    static void FillTarget(ServerSentryEvent ev, Object targetObject)
    {
        if (!ev)
        {
            return;
        }

        EntityAI targetEntity = EntityAI.Cast(targetObject);
        if (targetEntity)
        {
            ServerSentryEntityClassifier.FillEntity(ev.target, targetEntity);
            ev.position.FromVector(targetEntity.GetPosition());
            return;
        }

        if (targetObject)
        {
            ev.target.type = "object";
            ev.target.entityKind = "object";
            ev.target.className = targetObject.GetType();
            ev.target.name = targetObject.GetType();
            ev.position.FromVector(targetObject.GetPosition());
            return;
        }

        ev.target.type = "unknown";
        ev.target.entityKind = "unknown";
    }

    /**
    	\brief Processes the fill target data operation.
    	\param data \p data processed by the operation.
    	\param targetObject \p target object used by the operation.
    	\param actionData \p action data used by the operation.
    */
    static void FillTargetData(ServerSentryBuildActionData data, Object targetObject, ActionData actionData)
    {
        if (!data)
        {
            return;
        }

        if (targetObject)
        {
            data.targetClassName = targetObject.GetType();
            data.targetName = targetObject.GetType();
            data.targetPosition.FromVector(targetObject.GetPosition());

            EntityAI targetEntity = EntityAI.Cast(targetObject);
            if (targetEntity)
            {
                data.targetId = targetEntity.GetID();
                data.targetName = targetEntity.GetDisplayName();
            }

            data.targetRaw = data.targetClassName + "#" + data.targetId.ToString();

            BaseBuildingBase baseBuilding = BaseBuildingBase.Cast(targetObject);
            if (baseBuilding)
            {
                data.targetType = "basebuilding";
            }
            else
            {
                data.targetType = "object";
            }
        }

        if (actionData && actionData.m_Target)
        {
            data.targetComponentIndex = actionData.m_Target.GetComponentIndex();
        }
    }

    /**
    	\brief Processes the fill main item data operation.
    	\param data \p data processed by the operation.
    	\param mainItem \p main item used by the operation.
    */
    static void FillMainItemData(ServerSentryBuildActionData data, ItemBase mainItem)
    {
        if (!data || !mainItem)
        {
            return;
        }

        data.mainItemClassName = mainItem.GetType();
        data.mainItemDisplayName = mainItem.GetDisplayName();
        data.mainItemHealth = mainItem.GetHealth("", "Health");
        data.mainItemQuantity = mainItem.GetQuantity();
        data.isBaseBuildingKit = mainItem.IsBasebuildingKit();
    }

    /**
    	\brief Returns target object.
    	\param actionData \p action data used by the operation.
    	\return \p The resulting object instance, or null when unavailable.
    */
    static Object GetTargetObject(ActionData actionData)
    {
        if (!actionData || !actionData.m_Target)
        {
            return null;
        }

        return actionData.m_Target.GetObject();
    }

    /**
    	\brief Returns main item.
    	\param actionData \p action data used by the operation.
    	\return \p The resulting item base instance, or null when unavailable.
    */
    static ItemBase GetMainItem(ActionData actionData)
    {
        if (!actionData)
        {
            return null;
        }

        return actionData.m_MainItem;
    }

    /**
    	\brief Builds message.
    	\param actionName \p action name used by the operation.
    	\param player \p player entity associated with the operation.
    	\param targetObject \p target object used by the operation.
    	\param mainItem \p main item used by the operation.
    	\param data \p data processed by the operation.
    	\return \p The resulting string value.
    */
    static string BuildMessage(string actionName, PlayerBase player, Object targetObject, ItemBase mainItem, ServerSentryBuildActionData data)
    {
        string playerName = "unknown";
        if (player && player.GetIdentity())
        {
            playerName = player.GetIdentity().GetName();
        }

        string targetName = "unknown";
        if (data && data.targetName != "")
        {
            targetName = data.targetName;
        }
        else if (targetObject)
        {
            targetName = targetObject.GetType();
        }

        string toolName = "";
        if (mainItem)
        {
            toolName = " using " + mainItem.GetType();
        }

        return playerName + " " + actionName + " " + targetName + toolName;
    }

    /**
    	\brief Builds placement message.
    	\param player \p player entity associated with the operation.
    	\param placedEntity \p placed entity used by the operation.
    	\param data \p data processed by the operation.
    	\return \p The resulting string value.
    */
    static string BuildPlacementMessage(PlayerBase player, EntityAI placedEntity, ServerSentryBuildActionData data)
    {
        string playerName = "unknown";
        if (player && player.GetIdentity())
        {
            playerName = player.GetIdentity().GetName();
        }

        string placedName = "unknown";
        if (placedEntity)
        {
            placedName = placedEntity.GetType();
        }

        return playerName + " placed " + placedName;
    }
}
