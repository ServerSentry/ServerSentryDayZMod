/**
	\file ServerSentryEventFactory.c
	\brief Provides logging support for Event factory.
*/

/**
	\brief Creates values used by Event.
*/
class ServerSentryEventFactory
{
    /**
    	\brief Creates base.
    	\param eventType \p type of event being processed.
    	\param category \p category used by the operation.
    	\param severity \p severity used by the operation.
    	\return \p The resulting event instance, or null when unavailable.
    */
    static ServerSentryEvent CreateBase(string eventType, string category, string severity)
    {
        ServerSentryEvent ev = new ServerSentryEvent();

        ev.schemaVersion = ServerSentryDefaults.SchemaVersion();
        ev.eventType = eventType;
        ev.category = category;
        ev.severity = severity;
        ev.mod = "ServerSentry";
        return ev;
    }

    /**
    	\brief Creates player login.
    	\param player \p player entity associated with the operation.
    	\param identity \p player identity associated with the operation.
    	\return \p The resulting event instance, or null when unavailable.
    */
    static ServerSentryEvent CreatePlayerLogin(DayZPlayer player, PlayerIdentity identity)
    {
        ServerSentryEvent ev = CreateBase("player_login", "player", "info");

        ev.action.name = "login";
        ev.action.displayName = "Player login";
        ev.action.result = "success";
        FillActor(ev.actor, player, identity);

        if (player)
        {
            ev.position.FromVector(player.GetPosition());
        }

        ev.message = "Player logged in";
        return ev;
    }

    /**
    	\brief Creates player logout.
    	\param player \p player entity associated with the operation.
    	\param identity \p player identity associated with the operation.
    	\return \p The resulting event instance, or null when unavailable.
    */
    static ServerSentryEvent CreatePlayerLogout(DayZPlayer player, PlayerIdentity identity)
    {
        ServerSentryEvent ev = CreateBase("player_logout", "player", "info");

        ev.action.name = "logout";
        ev.action.displayName = "Player logout";
        ev.action.result = "success";
        FillActor(ev.actor, player, identity);

        if (player)
        {
            ev.position.FromVector(player.GetPosition());
        }

        ev.message = "Player logged out";
        return ev;
    }

    /**
    	\brief Creates combat hit.
    	\param victim \p victim used by the operation.
    	\param ctx \p action context containing request, target, and input data.
    	\return \p The resulting event instance, or null when unavailable.
    */
    static ServerSentryEvent CreateCombatHit(EntityAI victim, ServerSentryHitContext ctx)
    {
        bool isKill = false;
        if (ctx)
        {
            isKill = ctx.isLethal;
        }
        string eventType = "combat_hit";
        string severity = "debug";
        string actionName = "hit";
        string actionDisplayName = "Combat hit";
        string message = "Combat hit";

        if (isKill)
        {
            eventType = "combat_kill";
            severity = "info";
            actionName = "kill";
            actionDisplayName = "Combat kill";
            message = "Combat kill";
        }

        ServerSentryEvent ev = CreateBase(eventType, "combat", severity);
        ev.action.name = actionName;
        ev.action.displayName = actionDisplayName;
        ev.action.result = "success";

        if (ctx)
        {
            CopyActor(ctx.attacker, ev.actor);
            CopyActor(ctx.target, ev.target);
            ApplyHitContext(ev, ctx);
            ApplyCombatDimensions(ev);
            ev.message = message;
            return ev;
        }

        if (victim)
        {
            FillTargetFromEntity(ev.target, victim);
            ev.position.FromVector(victim.GetPosition());
            ev.combat.targetHealthAfter = victim.GetHealth("", "Health");
            ev.combat.targetAliveAfter = victim.IsAlive();
            ev.combat.isLethal = !ev.combat.targetAliveAfter || ev.combat.targetHealthAfter <= 0;
            ApplyCombatDimensions(ev);
        }

        ev.message = "Combat hit";
        return ev;
    }

    // Backward-compatible wrapper for existing internal/third-party hit callers.
    /**
    	\brief Creates player hit.
    	\param victim \p victim used by the operation.
    	\param ctx \p action context containing request, target, and input data.
    	\return \p The resulting event instance, or null when unavailable.
    */
    static ServerSentryEvent CreatePlayerHit(DayZPlayer victim, ServerSentryHitContext ctx)
    {
        return CreateCombatHit(victim, ctx);
    }

    /**
    	\brief Creates custom.
    	\param modName \p mod name used by the operation.
    	\param eventType \p type of event being processed.
    	\param category \p category used by the operation.
    	\param actionName \p action name used by the operation.
    	\param message \p receives a human-readable result or error message.
    	\param dataJson \p data JSON used by the operation.
    	\param player \p player entity associated with the operation.
    	\param severity \p severity used by the operation.
    	\return \p The resulting event instance, or null when unavailable.
    */
    static ServerSentryEvent CreateCustom(string modName, string eventType, string category, string actionName, string message, string dataJson, DayZPlayer player = null, string severity = "info")
    {
        ServerSentryEvent ev = CreateBase(eventType, category, severity);

        ev.mod = modName;
        ev.action.name = actionName;
        ev.action.displayName = actionName;
        ev.action.result = "success";

        if (player)
        {
            FillActor(ev.actor, player, player.GetIdentity());
            ev.position.FromVector(player.GetPosition());
        }

        ev.message = message;
        ev.dataJson = dataJson;
        return ev;
    }

    /**
    	\brief Creates market transaction.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    	\return \p The resulting market event instance, or null when unavailable.
    */
    static ServerSentryMarketEvent CreateMarketTransaction(ServerSentryMarketTransactionData data, DayZPlayer player = null)
    {
        ServerSentryMarketEvent ev = new ServerSentryMarketEvent();

        ev.schemaVersion = ServerSentryDefaults.SchemaVersion();
        ev.eventType = "market_transaction";
        ev.category = "market";
        ev.severity = "info";
        ev.mod = "market";

        ev.action.name = "market_transaction";
        ev.action.displayName = "Market transaction";
        ev.action.result = "success";

        if (data)
        {
            string sourceMod = data.sourceMod;
            if (sourceMod == "")
            {
                sourceMod = "market";
            }

            ev.mod = sourceMod;
            ev.action.name = data.transactionType;
            ev.action.displayName = "Market " + data.transactionType;
            ev.action.result = data.result;
            ev.item.className = data.itemClassName;
            ev.item.displayName = data.itemDisplayName;
            ev.item.quantity = data.totalAmount;
            FillMarketTraderTarget(ev.target, data);
            ev.raw = data.rawLog;

            // Store the detailed transaction as a typed nested object. JsonFileLoader<ServerSentryMarketEvent>.MakeData
            // will serialize this as "data": { ... } instead of an escaped JSON string in dataJson.
            ev.data = data;
            ev.dataJson = "";

            ev.message = sourceMod + " market " + data.transactionType + " " + data.itemClassName + " x" + data.totalAmount.ToString() + " for " + data.totalPrice.ToString();
        }
        else
        {
            ev.message = "Market transaction";
            ev.dataJson = ServerSentryJson.EmptyObject();
        }

        if (data && (data.actorName != "" || data.actorSteamId != "" || data.actorBohemiaId != ""))
        {
            FillPlayerActorData(ev.actor, data.actorSteamId, data.actorBohemiaId, data.actorName);
            ev.position.x = data.actorPosition.x;
            ev.position.y = data.actorPosition.y;
            ev.position.z = data.actorPosition.z;
        }
        else
        {
            ServerSentryEntityClassifier.FillEnvironment(ev.actor, "server");
        }

        return ev;
    }

    /**
    	\brief Creates safe zone transition.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    	\return \p The resulting safe zone event instance, or null when unavailable.
    */
    static ServerSentrySafeZoneEvent CreateSafeZoneTransition(ServerSentrySafeZoneTransitionData data, DayZPlayer player = null)
    {
        ServerSentrySafeZoneEvent ev = new ServerSentrySafeZoneEvent();

        ev.schemaVersion = ServerSentryDefaults.SchemaVersion();
        ev.eventType = "safezone_transition";
        ev.category = "safezone";
        ev.severity = "info";
        ev.mod = "safezone";

        ev.action.name = "safezone_transition";
        ev.action.displayName = "Safezone transition";
        ev.action.result = "success";

        if (data)
        {
            string sourceMod = data.sourceMod;
            if (sourceMod == "")
            {
                sourceMod = "safezone";
            }

            ev.mod = sourceMod;
            ev.action.name = data.transitionType;
            ev.action.displayName = "Safezone " + data.transitionType;
            ev.action.result = data.result;

            ev.target.type = "safezone";
            ev.target.entityKind = "safezone";
            ev.target.className = data.zoneClassName;
            ev.target.name = data.zoneName;
            if (ev.target.name == "")
            {
                ev.target.name = "Safezone " + data.zoneIndex.ToString();
            }

            ev.position.x = data.actorPosition.x;
            ev.position.y = data.actorPosition.y;
            ev.position.z = data.actorPosition.z;
            ev.position.zone = data.zoneName;

            ev.data = data;
            ev.dataJson = "";

            ev.message = sourceMod + " safezone " + data.transitionType + " " + data.zoneName;
        }
        else
        {
            ev.message = "Safezone transition";
            ev.dataJson = ServerSentryJson.EmptyObject();
        }

        if (data && (data.actorName != "" || data.actorSteamId != "" || data.actorBohemiaId != ""))
        {
            FillPlayerActorData(ev.actor, data.actorSteamId, data.actorBohemiaId, data.actorName);
        }
        else
        {
            ServerSentryEntityClassifier.FillEnvironment(ev.actor, "server");
        }

        return ev;
    }


    /**
    	\brief Creates vehicle event.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    	\return \p The resulting vehicle event instance, or null when unavailable.
    */
    static ServerSentryVehicleEvent CreateVehicleEvent(ServerSentryVehicleEventData data, DayZPlayer player = null)
    {
        ServerSentryVehicleEvent ev = new ServerSentryVehicleEvent();

        ev.schemaVersion = ServerSentryDefaults.SchemaVersion();
        ev.eventType = "vehicle_event";
        ev.category = "vehicle";
        ev.severity = "info";
        ev.mod = "ServerSentry";

        ev.action.name = "vehicle_event";
        ev.action.displayName = "Vehicle event";
        ev.action.result = "success";

        if (data)
        {
            if (data.sourceMod != "")
            {
                ev.mod = data.sourceMod;
            }

            ev.action.name = data.vehicleAction;
            ev.action.displayName = "Vehicle " + data.vehicleAction;
            ev.action.result = data.result;

            ev.target.type = "vehicle";
            ev.target.entityKind = "vehicle";
            ev.target.className = data.vehicleClassName;
            ev.target.name = data.vehicleDisplayName;
            if (ev.target.name == "")
            {
                ev.target.name = data.vehicleClassName;
            }

            ev.position.x = data.vehiclePosition.x;
            ev.position.y = data.vehiclePosition.y;
            ev.position.z = data.vehiclePosition.z;

            ev.data = data;
            ev.dataJson = "";

            ev.message = "Vehicle " + data.vehicleAction + " " + data.vehicleClassName;
        }
        else
        {
            ev.message = "Vehicle event";
            ev.dataJson = ServerSentryJson.EmptyObject();
        }

        if (data && (data.actorName != "" || data.actorSteamId != "" || data.actorBohemiaId != ""))
        {
            FillPlayerActorData(ev.actor, data.actorSteamId, data.actorBohemiaId, data.actorName);
        }
        else
        {
            ServerSentryEntityClassifier.FillEnvironment(ev.actor, "server");
        }

        return ev;
    }

    /**
    	\brief Creates chat message.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    	\return \p The resulting chat event instance, or null when unavailable.
    */
    static ServerSentryChatEvent CreateChatMessage(ServerSentryChatMessageData data, DayZPlayer player = null)
    {
        ServerSentryChatEvent ev = new ServerSentryChatEvent();

        ev.schemaVersion = ServerSentryDefaults.SchemaVersion();
        ev.eventType = "chat_message";
        ev.category = "chat";
        ev.severity = "info";
        ev.mod = "DayZ_Vanilla";

        ev.action.name = "chat_message";
        ev.action.displayName = "Chat message";
        ev.action.result = "success";

        if (data)
        {
            if (data.sourceMod != "")
            {
                ev.mod = data.sourceMod;
            }

            ev.action.name = "chat_" + data.channelType;
            if (data.channelType == "")
            {
                ev.action.name = "chat_message";
            }
            ev.action.displayName = "Chat " + data.channelName;
            ev.action.result = data.result;

            ev.target.type = "channel";
            ev.target.entityKind = "chat_channel";
            ev.target.className = data.channelId.ToString();
            ev.target.name = data.channelName;

            ev.position.x = data.senderPosition.x;
            ev.position.y = data.senderPosition.y;
            ev.position.z = data.senderPosition.z;

            ev.data = data;
            ev.dataJson = "";

            ev.message = data.senderName + " [" + data.channelName + "]: " + data.message;
        }
        else
        {
            ev.message = "Chat message";
            ev.dataJson = ServerSentryJson.EmptyObject();
        }

        if (data && (data.senderName != "" || data.senderSteamId != "" || data.senderBohemiaId != ""))
        {
            FillPlayerActorData(ev.actor, data.senderSteamId, data.senderBohemiaId, data.senderName);
        }
        else
        {
            ServerSentryEntityClassifier.FillEnvironment(ev.actor, "server");
        }

        return ev;
    }


    /**
    	\brief Creates group event.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    	\return \p The resulting group event instance, or null when unavailable.
    */
    static ServerSentryGroupEvent CreateGroupEvent(ServerSentryGroupEventData data, DayZPlayer player = null)
    {
        ServerSentryGroupEvent ev = new ServerSentryGroupEvent();

        ev.schemaVersion = ServerSentryDefaults.SchemaVersion();
        ev.eventType = "group_event";
        ev.category = "group";
        ev.severity = "info";
        ev.mod = "group";

        ev.action.name = "group_event";
        ev.action.displayName = "Group event";
        ev.action.result = "success";

        if (data)
        {
            string sourceMod = data.sourceMod;
            if (sourceMod == "")
            {
                sourceMod = "group";
            }

            ev.mod = sourceMod;
            ev.action.name = data.groupAction;
            ev.action.displayName = "Group " + data.groupAction;
            ev.action.result = data.result;

            ev.target.type = "group";
            ev.target.entityKind = "player_group";
            ev.target.className = data.groupTag;
            ev.target.name = data.groupName;
            if (ev.target.name == "")
            {
                ev.target.name = data.groupTag;
            }

            ev.position.x = data.actorPosition.x;
            ev.position.y = data.actorPosition.y;
            ev.position.z = data.actorPosition.z;

            ev.data = data;
            ev.dataJson = "";

            string memberName = data.memberName;
            if (memberName == "")
            {
                memberName = data.actorName;
            }
            if (memberName == "")
            {
                memberName = data.memberSteamId;
            }

            string groupName = data.groupTag;
            if (groupName == "")
            {
                groupName = data.groupName;
            }

            if (data.groupAction == "subgroup_change")
            {
                ev.message = memberName + " changed subgroup in " + groupName + " from " + data.fromSubgroupName + " to " + data.toSubgroupName;
            }
            else
            {
                ev.message = memberName + " group " + data.groupAction + " " + groupName;
            }
        }
        else
        {
            ev.message = "Group event";
            ev.dataJson = ServerSentryJson.EmptyObject();
        }

        if (data && (data.actorName != "" || data.actorSteamId != "" || data.actorBohemiaId != ""))
        {
            FillPlayerActorData(ev.actor, data.actorSteamId, data.actorBohemiaId, data.actorName);
        }
        else if (data && (data.memberName != "" || data.memberSteamId != "" || data.memberBohemiaId != ""))
        {
            FillPlayerActorData(ev.actor, data.memberSteamId, data.memberBohemiaId, data.memberName);
        }
        else
        {
            ServerSentryEntityClassifier.FillEnvironment(ev.actor, "server");
        }

        return ev;
    }


    /**
    	\brief Creates code lock event.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    	\return \p The resulting code lock event instance, or null when unavailable.
    */
    static ServerSentryCodeLockEvent CreateCodeLockEvent(ServerSentryCodeLockEventData data, DayZPlayer player = null)
    {
        ServerSentryCodeLockEvent ev = new ServerSentryCodeLockEvent();

        ev.schemaVersion = ServerSentryDefaults.SchemaVersion();
        ev.eventType = "codelock_event";
        ev.category = "codelock";
        ev.severity = "info";
        ev.mod = "CodeLock";

        ev.action.name = "codelock_event";
        ev.action.displayName = "CodeLock event";
        ev.action.result = "success";

        if (data)
        {
            if (data.sourceMod != "")
            {
                ev.mod = data.sourceMod;
            }

            ev.action.name = data.codeLockAction;
            if (ev.action.name == "")
            {
                ev.action.name = "codelock_event";
            }

            ev.action.displayName = "CodeLock " + ev.action.name;
            ev.action.result = data.result;
            ev.action.reason = data.reason;

            ev.target.type = data.targetType;
            if (ev.target.type == "")
            {
                ev.target.type = "codelock";
            }
            ev.target.entityKind = "codelock_target";
            ev.target.className = data.targetClassName;
            ev.target.name = data.targetDisplayName;
            if (ev.target.className == "")
            {
                ev.target.className = data.parentClassName;
            }
            if (ev.target.name == "")
            {
                ev.target.name = data.parentDisplayName;
            }
            if (ev.target.name == "")
            {
                ev.target.name = data.lockClassName;
            }

            ev.position.x = data.targetPosition.x;
            ev.position.y = data.targetPosition.y;
            ev.position.z = data.targetPosition.z;
            if (ev.position.x == 0 && ev.position.y == 0 && ev.position.z == 0)
            {
                ev.position.x = data.lockPosition.x;
                ev.position.y = data.lockPosition.y;
                ev.position.z = data.lockPosition.z;
            }

            ev.data = data;
            ev.dataJson = "";

            string actorName = data.actorName;
            if (actorName == "")
            {
                actorName = data.actorSteamId;
            }
            if (actorName == "")
            {
                actorName = "unknown";
            }

            string targetName = ev.target.name;
            if (targetName == "")
            {
                targetName = ev.target.className;
            }
            if (targetName == "")
            {
                targetName = "CodeLock";
            }

            ev.message = actorName + " codelock " + ev.action.name + " " + targetName;
        }
        else
        {
            ev.message = "CodeLock event";
            ev.dataJson = ServerSentryJson.EmptyObject();
        }

        if (data && (data.actorName != "" || data.actorSteamId != "" || data.actorBohemiaId != ""))
        {
            FillPlayerActorData(ev.actor, data.actorSteamId, data.actorBohemiaId, data.actorName);
        }
        else
        {
            ServerSentryEntityClassifier.FillEnvironment(ev.actor, "server");
        }

        return ev;
    }


    /**
    	\brief Creates breaching charge event.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    	\return \p The resulting breaching charge event instance, or null when unavailable.
    */
    static ServerSentryBreachingChargeEvent CreateBreachingChargeEvent(ServerSentryBreachingChargeEventData data, DayZPlayer player = null)
    {
        ServerSentryBreachingChargeEvent ev = new ServerSentryBreachingChargeEvent();

        ev.schemaVersion = ServerSentryDefaults.SchemaVersion();
        ev.eventType = "breaching_charge_event";
        ev.category = "breaching_charge";
        ev.severity = "info";
        ev.mod = "HDSN_BreachingCharge";

        ev.action.name = "breaching_charge_event";
        ev.action.displayName = "BreachingCharge event";
        ev.action.result = "success";

        if (data)
        {
            if (data.sourceMod != "")
            {
                ev.mod = data.sourceMod;
            }

            ev.action.name = data.breachAction;
            if (ev.action.name == "")
            {
                ev.action.name = "breaching_charge_event";
            }

            ev.action.displayName = "BreachingCharge " + ev.action.name;
            ev.action.result = data.result;
            ev.action.reason = data.reason;

            ev.target.type = data.targetType;
            if (ev.target.type == "")
            {
                if (data.targetClassName != "")
                {
                    ev.target.type = "breach_target";
                }
                else
                {
                    ev.target.type = "breaching_charge";
                }
            }

            ev.target.entityKind = "breaching_charge_target";
            ev.target.className = data.targetClassName;
            ev.target.name = data.targetDisplayName;
            if (ev.target.className == "")
            {
                ev.target.className = data.chargeClassName;
            }
            if (ev.target.name == "")
            {
                ev.target.name = data.chargeTargetType;
            }
            if (ev.target.name == "")
            {
                ev.target.name = data.targetClassName;
            }
            if (ev.target.name == "")
            {
                ev.target.name = data.chargeDisplayName;
            }
            if (ev.target.name == "")
            {
                ev.target.name = data.chargeClassName;
            }

            ev.position.x = data.chargePosition.x;
            ev.position.y = data.chargePosition.y;
            ev.position.z = data.chargePosition.z;

            if ((ev.position.x == 0 && ev.position.y == 0 && ev.position.z == 0) && data.targetClassName != "")
            {
                ev.position.x = data.targetPosition.x;
                ev.position.y = data.targetPosition.y;
                ev.position.z = data.targetPosition.z;
            }

            ev.data = data;
            ev.dataJson = "";

            string actorName = data.actorName;
            if (actorName == "")
            {
                actorName = data.placingName;
            }
            if (actorName == "")
            {
                actorName = data.actorSteamId;
            }
            if (actorName == "")
            {
                actorName = "server";
            }

            string targetName = ev.target.name;
            if (targetName == "")
            {
                targetName = ev.target.className;
            }
            if (targetName == "")
            {
                targetName = "target";
            }

            ev.message = actorName + " breaching_charge " + ev.action.name + " " + targetName;
        }
        else
        {
            ev.message = "BreachingCharge event";
            ev.dataJson = ServerSentryJson.EmptyObject();
        }

        if (data && (data.actorName != "" || data.actorSteamId != "" || data.actorBohemiaId != ""))
        {
            FillPlayerActorData(ev.actor, data.actorSteamId, data.actorBohemiaId, data.actorName);
        }
        else if (data && (data.placingName != "" || data.placingSteamId != "" || data.placingBohemiaId != ""))
        {
            FillPlayerActorData(ev.actor, data.placingSteamId, data.placingBohemiaId, data.placingName);
        }
        else
        {
            ServerSentryEntityClassifier.FillEnvironment(ev.actor, "server");
        }

        return ev;
    }

    /**
    	\brief Processes the fill market trader target operation.
    	\param target \p target object associated with the operation.
    	\param data \p data processed by the operation.
    */
    static void FillMarketTraderTarget(ServerSentryTarget target, ServerSentryMarketTransactionData data)
    {
        if (!target || !data)
        {
            return;
        }

        target.type = "trader";
        target.entityKind = "market_trader";
        target.className = data.traderEntityClassName;
        target.name = data.traderDisplayName;

        if (target.className == "")
        {
            target.className = data.traderFileName;
        }

        if (target.name == "")
        {
            target.name = data.traderFileName;
        }

        if (target.name == "")
        {
            target.name = data.sourceMod + " Trader";
        }
    }


    /**
    	\brief Creates lbatm event.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    	\return \p The resulting lbatm event instance, or null when unavailable.
    */
    static ServerSentryLBATMEvent CreateLBATMEvent(ServerSentryLBATMEventData data, DayZPlayer player = null)
    {
        ServerSentryLBATMEvent ev = new ServerSentryLBATMEvent();

        ev.schemaVersion = ServerSentryDefaults.SchemaVersion();
        ev.eventType = "lbatm_event";
        ev.category = "atm";
        ev.severity = "info";
        ev.mod = "LBmaster_Groups_ATM";

        ev.action.name = "lbatm_event";
        ev.action.displayName = "LB ATM event";
        ev.action.result = "success";

        if (data)
        {
            if (data.sourceMod != "")
            {
                ev.mod = data.sourceMod;
            }

            ev.action.name = data.atmAction;
            if (ev.action.name == "")
            {
                ev.action.name = "lbatm_event";
            }

            ev.action.displayName = "LB ATM " + ev.action.name;
            ev.action.result = data.result;
            ev.action.reason = data.reason;

            ev.target.type = "atm";
            ev.target.entityKind = "atm_event_target";
            ev.target.className = data.accountType;
            ev.target.name = data.atmName;

            if (data.recipientSteamId != "")
            {
                ev.target.type = "player";
                ev.target.entityKind = "human_player";
                ev.target.className = "PlayerBase";
                ev.target.steamId = data.recipientSteamId;
                ev.target.bohemiaId = data.recipientBohemiaId;
                ev.target.name = data.recipientName;
            }
            else if (data.targetSteamId != "")
            {
                ev.target.type = "player";
                ev.target.entityKind = "human_player";
                ev.target.className = "PlayerBase";
                ev.target.steamId = data.targetSteamId;
                ev.target.bohemiaId = data.targetBohemiaId;
                ev.target.name = data.targetName;
            }
            else if (data.groupTag != "" || data.groupName != "")
            {
                ev.target.type = "group";
                ev.target.entityKind = "player_group";
                ev.target.className = data.groupTag;
                ev.target.name = data.groupName;
            }
            else if (data.cardUid != "")
            {
                ev.target.type = "credit_card";
                ev.target.entityKind = "atm_credit_card";
                ev.target.className = data.cardClassName;
                ev.target.name = "Credit card " + data.cardUid;
            }
            else if (data.atmName != "")
            {
                ev.target.type = "atm";
                ev.target.entityKind = "atm_machine";
                ev.target.className = data.atmClassName;
                ev.target.name = data.atmName;
            }

            if (ev.target.name == "")
            {
                ev.target.name = data.accountType;
            }
            if (ev.target.name == "")
            {
                ev.target.name = data.webhookType;
            }

            ev.position.x = data.atmPosition.x;
            ev.position.y = data.atmPosition.y;
            ev.position.z = data.atmPosition.z;

            if (ev.position.x == 0 && ev.position.y == 0 && ev.position.z == 0)
            {
                ev.position.x = data.actorPosition.x;
                ev.position.y = data.actorPosition.y;
                ev.position.z = data.actorPosition.z;
            }

            ev.data = data;
            ev.dataJson = "";

            string actorName = data.actorName;
            if (actorName == "")
            {
                actorName = data.actorSteamId;
            }
            if (actorName == "")
            {
                actorName = "server";
            }

            string amountPart = data.amountText;
            if (amountPart == "" && data.amount > 0)
            {
                amountPart = data.amount.ToString();
            }

            string targetName = ev.target.name;
            if (targetName == "")
            {
                targetName = "ATM";
            }

            ev.message = actorName + " atm " + ev.action.name;
            if (amountPart != "")
            {
                ev.message = ev.message + " " + amountPart;
            }
            if (targetName != "")
            {
                ev.message = ev.message + " " + targetName;
            }
        }
        else
        {
            ev.message = "LB ATM event";
            ev.dataJson = ServerSentryJson.EmptyObject();
        }

        if (data && (data.actorName != "" || data.actorSteamId != "" || data.actorBohemiaId != ""))
        {
            FillPlayerActorData(ev.actor, data.actorSteamId, data.actorBohemiaId, data.actorName);
        }
        else
        {
            ServerSentryEntityClassifier.FillEnvironment(ev.actor, "server");
        }

        return ev;
    }

    /**
    	\brief Creates dropped events.
    	\param droppedCount \p dropped count used by the operation.
    	\return \p The resulting event instance, or null when unavailable.
    */
    static ServerSentryEvent CreateDroppedEvents(int droppedCount)
    {
        ServerSentryEvent ev = CreateBase("server_sentry_dropped_events", "system", "warning");

        ev.action.name = "drop_events";
        ev.action.displayName = "Dropped events";
        ev.action.result = "partial";
        ev.message = "ServerSentry queue dropped events because producers were faster than disk flush.";
        ev.dataJson = ServerSentryJson.MakeDroppedEventsData(droppedCount);
        return ev;
    }

    /**
    	\brief Applies hit context.
    	\param ev \p ev used by the operation.
    	\param ctx \p action context containing request, target, and input data.
    */
    static void ApplyHitContext(ServerSentryEvent ev, ServerSentryHitContext ctx)
    {
        if (!ev || !ctx)
        {
            return;
        }

        if (ctx.occurredAtUtc != "")
        {
            ev.occurredAtUtc = ctx.occurredAtUtc;
        }

        ev.position.FromVector(ctx.victimPosition);

        ev.weapon.className = ctx.weaponClassName;
        ev.weapon.ammoType = ctx.ammoType;
        ev.weapon.distance = ctx.distance;

        ev.combat.hitId = ctx.hitId;
        ev.combat.hitSequence = ctx.eventSequence;
        ev.combat.damage = ctx.healthDamage;
        ev.combat.bloodDamage = ctx.bloodDamage;
        ev.combat.shockDamage = ctx.shockDamage;
        ev.combat.targetHealthBefore = ctx.targetHealthBefore;
        ev.combat.targetHealthAfter = ctx.targetHealthAfter;
        ev.combat.damageType = ctx.damageType;
        ev.combat.component = ctx.component;
        ev.combat.hitZone = ctx.hitZone;
        ev.combat.targetAliveAfter = ctx.targetAliveAfter;
        ev.combat.isHeadshot = ctx.isHeadshot;
        ev.combat.isMelee = ctx.isMelee;
        ev.combat.isExplosion = ctx.isExplosion;
        ev.combat.isLethal = ctx.isLethal;
    }

    /**
    	\brief Applies combat dimensions.
    	\param ev \p ev used by the operation.
    */
    static void ApplyCombatDimensions(ServerSentryEvent ev)
    {
        if (!ev)
        {
            return;
        }

        ev.combat.actorGroup = ResolveGroup(ev.actor);
        ev.combat.actorKind = ResolveKind(ev.actor);
        ev.combat.targetGroup = ResolveGroup(ev.target);
        ev.combat.targetKind = ResolveKind(ev.target);
        ev.combat.relation = ResolveCombatRelation(ev.actor, ev.target);
        ev.combat.countsForPlayerStats = ShouldCountForPlayerStats(ev.actor, ev.target);
        ev.combat.countsForPlayerHitStats = ShouldCountForPlayerHitStats(ev);
        ev.combat.countsForPlayerKillStats = ShouldCountForPlayerKillStats(ev);
    }

    /**
    	\brief Resolves group.
    	\param actor \p actor used by the operation.
    	\return \p The resulting string value.
    */
    static string ResolveGroup(ServerSentryActor actor)
    {
        if (actor && actor.type != "")
        {
            return actor.type;
        }

        return "unknown";
    }

    /**
    	\brief Resolves kind.
    	\param actor \p actor used by the operation.
    	\return \p The resulting string value.
    */
    static string ResolveKind(ServerSentryActor actor)
    {
        if (actor && actor.entityKind != "")
        {
            return actor.entityKind;
        }

        return "unknown";
    }

    /**
    	\brief Resolves combat relation.
    	\param actor \p actor used by the operation.
    	\param target \p target object associated with the operation.
    	\return \p The resulting string value.
    */
    static string ResolveCombatRelation(ServerSentryActor actor, ServerSentryTarget target)
    {
        string actorType = "unknown";
        string targetType = "unknown";

        if (actor && actor.type != "")
        {
            actorType = actor.type;
        }

        if (target && target.type != "")
        {
            targetType = target.type;
        }

        return actorType + "_vs_" + targetType;
    }

    /**
    	\brief Returns whether count for player stats.
    	\param actor \p actor used by the operation.
    	\param target \p target object associated with the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool ShouldCountForPlayerStats(ServerSentryActor actor, ServerSentryTarget target)
    {
        if (!actor)
        {
            return false;
        }

        return actor.type == "player";
    }

    /**
    	\brief Returns whether count for player hit stats.
    	\param ev \p ev used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool ShouldCountForPlayerHitStats(ServerSentryEvent ev)
    {
        if (!ev || !ev.actor)
        {
            return false;
        }

        return ev.eventType == "combat_hit" && ev.actor.type == "player";
    }

    /**
    	\brief Returns whether count for player kill stats.
    	\param ev \p ev used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool ShouldCountForPlayerKillStats(ServerSentryEvent ev)
    {
        if (!ev || !ev.actor || !ev.combat)
        {
            return false;
        }

        bool isKillEvent = ev.eventType == "combat_kill" || (ev.eventType == "combat_hit" && ev.combat.isLethal);
        return isKillEvent && ev.actor.type == "player";
    }



    /**
    	\brief Processes the fill actor operation.
    	\param actor \p actor used by the operation.
    	\param player \p player entity associated with the operation.
    	\param identity \p player identity associated with the operation.
    */
    static void FillActor(ServerSentryActor actor, DayZPlayer player, PlayerIdentity identity)
    {
        ServerSentryEntityClassifier.FillPlayer(actor, player, identity);
    }

    /**
    	\brief Processes the fill actor from entity operation.
    	\param actor \p actor used by the operation.
    	\param entity \p entity processed by the operation.
    */
    static void FillActorFromEntity(ServerSentryActor actor, EntityAI entity)
    {
        ServerSentryEntityClassifier.FillEntity(actor, entity);
    }

    /**
    	\brief Processes the fill target from entity operation.
    	\param target \p target object associated with the operation.
    	\param entity \p entity processed by the operation.
    */
    static void FillTargetFromEntity(ServerSentryTarget target, EntityAI entity)
    {
        ServerSentryEntityClassifier.FillEntity(target, entity);
    }

    /**
    	\brief Processes the fill target from player operation.
    	\param target \p target object associated with the operation.
    	\param player \p player entity associated with the operation.
    */
    static void FillTargetFromPlayer(ServerSentryTarget target, DayZPlayer player)
    {
        FillTargetFromEntity(target, player);
    }

    /**
    	\brief Resolves player like type.
    	\param player \p player entity associated with the operation.
    	\param identity \p player identity associated with the operation.
    	\return \p The resulting string value.
    */
    static string ResolvePlayerLikeType(DayZPlayer player, PlayerIdentity identity)
    {
        return ServerSentryEntityClassifier.ResolvePlayerLikeType(player, identity);
    }

    /**
    	\brief Processes the fill player actor data operation.
    	\param actor \p actor used by the operation.
    	\param steamId \p Steam ID of the target player.
    	\param bohemiaId \p identifier of the bohemia.
    	\param name \p name used by the operation.
    */
    static void FillPlayerActorData(ServerSentryActor actor, string steamId, string bohemiaId, string name)
    {
        if (!actor)
        {
            return;
        }

        actor.type = "player";
        actor.entityKind = "human_player";
        actor.className = "PlayerBase";
        actor.steamId = steamId;
        actor.bohemiaId = bohemiaId;
        actor.name = name;
    }

    /**
    	\brief Copies actor.
    	\param source \p source used by the operation.
    	\param target \p target object associated with the operation.
    */
    static void CopyActor(ServerSentryActor source, ServerSentryActor target)
    {
        if (!source || !target)
        {
            return;
        }

        target.type = source.type;
        target.entityKind = source.entityKind;
        target.className = source.className;
        target.steamId = source.steamId;
        target.bohemiaId = source.bohemiaId;
        target.name = source.name;
    }
};
