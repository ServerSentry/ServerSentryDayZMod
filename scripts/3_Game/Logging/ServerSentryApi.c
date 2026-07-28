/**
	\file ServerSentryApi.c
	\brief Provides logging support for API.
*/

/**
	\brief Provides the public API for ServerSentry.
*/
class ServerSentryApi
{
    /**
    	\brief Emits event.
    	\param ev \p ev used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool EmitEvent(ServerSentryEvent ev)
    {
        return ServerSentryLogger.Get().Emit(ev);
    }

    /**
    	\brief Emits custom.
    	\param sourceMod \p source mod used by the operation.
    	\param eventType \p type of event being processed.
    	\param category \p category used by the operation.
    	\param actionName \p action name used by the operation.
    	\param message \p receives a human-readable result or error message.
    	\param dataJson \p data JSON used by the operation.
    	\param player \p player entity associated with the operation.
    	\param severity \p severity used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool EmitCustom(string sourceMod, string eventType, string category, string actionName, string message, string dataJson = "{}", DayZPlayer player = null, string severity = "info")
    {
        ServerSentryEvent ev = ServerSentryEventFactory.CreateCustom(sourceMod, eventType, category, actionName, message, dataJson, player, severity);
        return ServerSentryLogger.Get().Emit(ev);
    }

    /**
    	\brief Emits player action.
    	\param sourceMod \p source mod used by the operation.
    	\param player \p player entity associated with the operation.
    	\param actionName \p action name used by the operation.
    	\param message \p receives a human-readable result or error message.
    	\param dataJson \p data JSON used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool EmitPlayerAction(string sourceMod, DayZPlayer player, string actionName, string message, string dataJson = "{}")
    {
        return EmitCustom(sourceMod, "mod_action", "mod", actionName, message, dataJson, player, "info");
    }

    /**
    	\brief Emits market transaction.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool EmitMarketTransaction(ServerSentryMarketTransactionData data, DayZPlayer player = null)
    {
        return ServerSentryLogger.Get().EmitMarketTransaction(ServerSentryEventFactory.CreateMarketTransaction(data));
    }

    /**
    	\brief Emits safe zone transition.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool EmitSafeZoneTransition(ServerSentrySafeZoneTransitionData data, DayZPlayer player = null)
    {
        return ServerSentryLogger.Get().EmitSafeZoneTransition(ServerSentryEventFactory.CreateSafeZoneTransition(data));
    }

    /**
    	\brief Emits vehicle event.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool EmitVehicleEvent(ServerSentryVehicleEventData data, DayZPlayer player = null)
    {
        return ServerSentryLogger.Get().EmitVehicleEvent(ServerSentryEventFactory.CreateVehicleEvent(data));
    }

    /**
    	\brief Emits chat message.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool EmitChatMessage(ServerSentryChatMessageData data, DayZPlayer player = null)
    {
        if (!ServerSentryChatDeduper.ShouldEmit(data))
        {
            return false;
        }

        return ServerSentryLogger.Get().EmitChatMessage(ServerSentryEventFactory.CreateChatMessage(data));
    }

    /**
    	\brief Emits group event.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool EmitGroupEvent(ServerSentryGroupEventData data, DayZPlayer player = null)
    {
        return ServerSentryLogger.Get().EmitGroupEvent(ServerSentryEventFactory.CreateGroupEvent(data));
    }

    /**
    	\brief Emits code lock event.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool EmitCodeLockEvent(ServerSentryCodeLockEventData data, DayZPlayer player = null)
    {
        return ServerSentryLogger.Get().EmitCodeLockEvent(ServerSentryEventFactory.CreateCodeLockEvent(data));
    }

    /**
    	\brief Emits breaching charge event.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool EmitBreachingChargeEvent(ServerSentryBreachingChargeEventData data, DayZPlayer player = null)
    {
        return ServerSentryLogger.Get().EmitBreachingChargeEvent(ServerSentryEventFactory.CreateBreachingChargeEvent(data));
    }

    /**
    	\brief Emits lbatm event.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool EmitLBATMEvent(ServerSentryLBATMEventData data, DayZPlayer player = null)
    {
        return ServerSentryLogger.Get().EmitLBATMEvent(ServerSentryEventFactory.CreateLBATMEvent(data));
    }
}
