/**
	\file ServerSentryLogger.c
	\brief Provides logging support for Logger.
*/

/**
	\brief Provides ServerSentry behavior for Logger.
*/
class ServerSentryLogger
{
    /**
    	\brief Shared instance state used by all instances.
    */
    protected static ref ServerSentryLogger s_Instance;

    /**
    	\brief Instance state for writer.
    */
    protected ref ServerSentryJsonLineWriter m_Writer;
    /**
    	\brief Instance state for buffer.
    */
    protected ref array<string> m_Buffer;

    /**
    	\brief Instance state for sequence.
    */
    protected int m_Sequence;
    /**
    	\brief Instance state for dropped events.
    */
    protected int m_DroppedEvents;
    /**
    	\brief Instance state for started.
    */
    protected bool m_Started;
    /**
    	\brief Instance state for boot ID.
    */
    protected string m_BootId;

    /**
    	\brief Returns the shared singleton instance.
    	\return \p The resulting logger instance, or null when unavailable.
    */
    static ServerSentryLogger Get()
    {
        if (!s_Instance)
        {
            s_Instance = new ServerSentryLogger();
        }

        return s_Instance;
    }

    /**
    	\brief Initializes a new ServerSentryLogger instance.
    */
    void ServerSentryLogger()
    {
        m_Sequence = 0;
        m_DroppedEvents = 0;
        m_Started = false;
        m_BootId = ServerSentryDefaults.BootIdPrefix() + "-" + ServerSentryTime.NowUtc();
        m_Buffer = new array<string>();
        m_Writer = new ServerSentryJsonLineWriter();
        Start();
    }

    /**
    	\brief Processes the start operation.
    */
    void Start()
    {
        if (m_Started)
        {
            return;
        }

        m_Started = true;

        if (GetGame())
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Flush, ServerSentryDefaults.FlushIntervalMs(), true);
        }
    }

    /**
    	\brief Emits the requested operation.
    	\param ev \p ev used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool Emit(ServerSentryEvent ev)
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return false;
        }

        if (!ev)
        {
            return false;
        }

        ServerSentryItemMoveEvent itemMoveEv;
        if (Class.CastTo(itemMoveEv, ev))
        {
            return EmitItemMoveEvent(itemMoveEv);
        }

        ServerSentryBuildActionEvent buildActionEv;
        if (Class.CastTo(buildActionEv, ev))
        {
            return EmitBuildActionEvent(buildActionEv);
        }

        ServerSentryBaseDamageEvent baseDamageEv;
        if (Class.CastTo(baseDamageEv, ev))
        {
            return EmitBaseDamageEvent(baseDamageEv);
        }

        PrepareEvent(ev);

        string json;
        string error;
        if (!JsonFileLoader<ServerSentryEvent>.MakeData(ev, json, error, false) || json == "")
        {
            Print("[ServerSentry] Failed to serialize event: " + ev.eventType + " - " + error);
            return false;
        }

        return EnqueueJson(json, ev.severity);
    }

    /**
    	\brief Emits item move event.
    	\param ev \p ev used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool EmitItemMoveEvent(ServerSentryItemMoveEvent ev)
    {
        if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            Print("[ServerSentry][InventoryDebug][Logger.EmitItemMoveEvent.enter]");
        }

        if (!GetGame())
        {
            if (ServerSentryDefaults.DebugInventoryMoveEvents())
            {
                Print("[ServerSentry][InventoryDebug][Logger.EmitItemMoveEvent.reject] GetGame=null");
            }
            return false;
        }

        if (!GetGame().IsServer())
        {
            if (ServerSentryDefaults.DebugInventoryMoveEvents())
            {
                Print("[ServerSentry][InventoryDebug][Logger.EmitItemMoveEvent.reject] not server");
            }
            return false;
        }

        if (!ev)
        {
            if (ServerSentryDefaults.DebugInventoryMoveEvents())
            {
                Print("[ServerSentry][InventoryDebug][Logger.EmitItemMoveEvent.reject] ev=null");
            }
            return false;
        }

        if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            string actionName = "";
            if (ev.action)
            {
                actionName = ev.action.name;
            }

            string moveId = "";
            if (ev.data)
            {
                moveId = ev.data.moveId;
            }

            Print("[ServerSentry][InventoryDebug][Logger.EmitItemMoveEvent.beforePrepare] eventType=" + ev.eventType + " action=" + actionName + " moveId=" + moveId);
        }

        PrepareEvent(ev);

        if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            Print("[ServerSentry][InventoryDebug][Logger.EmitItemMoveEvent.afterPrepare] eventId=" + ev.eventId + " seq=" + ev.sequence.ToString());
            Print("[ServerSentry][InventoryDebug][Logger.EmitItemMoveEvent.beforeSerialize]");
        }

        string json;
        string error;
        if (!JsonFileLoader<ServerSentryItemMoveEvent>.MakeData(ev, json, error, false) || json == "")
        {
            Print("[ServerSentry] Failed to serialize item move event: " + error);
            if (ServerSentryDefaults.DebugInventoryMoveEvents())
            {
                Print("[ServerSentry][InventoryDebug][Logger.EmitItemMoveEvent.serializeFailed] error=" + error);
            }
            return false;
        }

        if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            Print("[ServerSentry][InventoryDebug][Logger.EmitItemMoveEvent.afterSerialize] jsonLen=" + json.Length().ToString());
            Print("[ServerSentry][InventoryDebug][Logger.EmitItemMoveEvent.beforeEnqueue]");
        }

        bool result = EnqueueJson(json, ev.severity);

        if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            string resultText = "false";
            if (result)
            {
                resultText = "true";
            }
            Print("[ServerSentry][InventoryDebug][Logger.EmitItemMoveEvent.afterEnqueue] result=" + resultText + " buffer=" + m_Buffer.Count().ToString());
        }

        return result;
    }

    /**
    	\brief Emits build action event.
    	\param ev \p ev used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool EmitBuildActionEvent(ServerSentryBuildActionEvent ev)
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return false;
        }

        if (!ev)
        {
            return false;
        }

        PrepareEvent(ev);

        string json;
        string error;
        if (!JsonFileLoader<ServerSentryBuildActionEvent>.MakeData(ev, json, error, false) || json == "")
        {
            Print("[ServerSentry] Failed to serialize build action event: " + error);
            return false;
        }

        return EnqueueJson(json, ev.severity);
    }

    /**
        \brief Serializes and queues a base-building damage event with its typed payload.
        \param ev \p immutable base-building damage snapshot.
        \return \p true when the serialized event was accepted by the queue.
    */
    bool EmitBaseDamageEvent(ServerSentryBaseDamageEvent ev)
    {
        if (!GetGame() || !GetGame().IsServer() || !ev)
        {
            return false;
        }

        PrepareEvent(ev);

        string json;
        string error;
        if (!JsonFileLoader<ServerSentryBaseDamageEvent>.MakeData(ev, json, error, false) || json == "")
        {
            Print("[ServerSentry] Failed to serialize base damage event: " + error);
            return false;
        }

        return EnqueueJson(json, ev.severity);
    }

    /**
    	\brief Emits market transaction.
    	\param ev \p ev used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool EmitMarketTransaction(ServerSentryMarketEvent ev)
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return false;
        }

        if (!ev)
        {
            return false;
        }

        PrepareEvent(ev);

        string json;
        string error;
        if (!JsonFileLoader<ServerSentryMarketEvent>.MakeData(ev, json, error, false) || json == "")
        {
            Print("[ServerSentry] Failed to serialize market event: " + error);
            return false;
        }

        return EnqueueJson(json, ev.severity);
    }

    /**
    	\brief Emits safe zone transition.
    	\param ev \p ev used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool EmitSafeZoneTransition(ServerSentrySafeZoneEvent ev)
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return false;
        }

        if (!ev)
        {
            return false;
        }

        PrepareEvent(ev);

        string json;
        string error;
        if (!JsonFileLoader<ServerSentrySafeZoneEvent>.MakeData(ev, json, error, false) || json == "")
        {
            Print("[ServerSentry] Failed to serialize safezone event: " + error);
            return false;
        }

        return EnqueueJson(json, ev.severity);
    }


    /**
    	\brief Emits vehicle event.
    	\param ev \p ev used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool EmitVehicleEvent(ServerSentryVehicleEvent ev)
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return false;
        }

        if (!ev)
        {
            return false;
        }

        PrepareEvent(ev);

        string json;
        string error;
        if (!JsonFileLoader<ServerSentryVehicleEvent>.MakeData(ev, json, error, false) || json == "")
        {
            Print("[ServerSentry] Failed to serialize vehicle event: " + error);
            return false;
        }

        return EnqueueJson(json, ev.severity);
    }

    /**
    	\brief Emits chat message.
    	\param ev \p ev used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool EmitChatMessage(ServerSentryChatEvent ev)
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return false;
        }

        if (!ev)
        {
            return false;
        }

        PrepareEvent(ev);

        string json;
        string error;
        if (!JsonFileLoader<ServerSentryChatEvent>.MakeData(ev, json, error, false) || json == "")
        {
            Print("[ServerSentry] Failed to serialize chat event: " + error);
            return false;
        }

        return EnqueueJson(json, ev.severity);
    }

    /**
    	\brief Emits group event.
    	\param ev \p ev used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool EmitGroupEvent(ServerSentryGroupEvent ev)
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return false;
        }

        if (!ev)
        {
            return false;
        }

        PrepareEvent(ev);

        string json;
        string error;
        if (!JsonFileLoader<ServerSentryGroupEvent>.MakeData(ev, json, error, false) || json == "")
        {
            Print("[ServerSentry] Failed to serialize group event: " + error);
            return false;
        }

        return EnqueueJson(json, ev.severity);
    }

    /**
    	\brief Emits code lock event.
    	\param ev \p ev used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool EmitCodeLockEvent(ServerSentryCodeLockEvent ev)
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return false;
        }

        if (!ev)
        {
            return false;
        }

        PrepareEvent(ev);

        string json;
        string error;
        if (!JsonFileLoader<ServerSentryCodeLockEvent>.MakeData(ev, json, error, false) || json == "")
        {
            Print("[ServerSentry] Failed to serialize codelock event: " + error);
            return false;
        }

        return EnqueueJson(json, ev.severity);
    }

    /**
    	\brief Emits breaching charge event.
    	\param ev \p ev used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool EmitBreachingChargeEvent(ServerSentryBreachingChargeEvent ev)
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return false;
        }

        if (!ev)
        {
            return false;
        }

        PrepareEvent(ev);

        string json;
        string error;
        if (!JsonFileLoader<ServerSentryBreachingChargeEvent>.MakeData(ev, json, error, false) || json == "")
        {
            Print("[ServerSentry] Failed to serialize breaching charge event: " + error);
            return false;
        }

        return EnqueueJson(json, ev.severity);
    }


    /**
    	\brief Emits lbatm event.
    	\param ev \p ev used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool EmitLBATMEvent(ServerSentryLBATMEvent ev)
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return false;
        }

        if (!ev)
        {
            return false;
        }

        PrepareEvent(ev);

        string json;
        string error;
        if (!JsonFileLoader<ServerSentryLBATMEvent>.MakeData(ev, json, error, false) || json == "")
        {
            Print("[ServerSentry] Failed to serialize LB ATM event: " + error);
            return false;
        }

        return EnqueueJson(json, ev.severity);
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
    bool EmitCustom(string sourceMod, string eventType, string category, string actionName, string message, string dataJson = "{}", DayZPlayer player = null, string severity = "info")
    {
        ServerSentryEvent ev = ServerSentryEventFactory.CreateCustom(sourceMod, eventType, category, actionName, message, dataJson, player, severity);
        return Emit(ev);
    }

    /**
    	\brief Flushes the requested operation.
    */
    void Flush()
    {
        if (!m_Buffer || m_Buffer.Count() == 0)
        {
            return;
        }

        if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            Print("[ServerSentry][InventoryDebug][Logger.Flush.enter] buffer=" + m_Buffer.Count().ToString() + " dropped=" + m_DroppedEvents.ToString());
        }

        array<string> batch = new array<string>();
        foreach (string line: m_Buffer)
        {
            batch.Insert(line);
        }
        m_Buffer.Clear();

        if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            Print("[ServerSentry][InventoryDebug][Logger.Flush.afterCopy] batch=" + batch.Count().ToString() + " buffer=" + m_Buffer.Count().ToString());
        }

        AppendDroppedEvent(batch);

        if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            Print("[ServerSentry][InventoryDebug][Logger.Flush.beforeWriteLines] batch=" + batch.Count().ToString());
        }

        if (!m_Writer.WriteLines(batch))
        {
            if (ServerSentryDefaults.DebugInventoryMoveEvents())
            {
                Print("[ServerSentry][InventoryDebug][Logger.Flush.writeLinesFailed] requeue batch=" + batch.Count().ToString());
            }
            RequeueBatch(batch);
        }
        else if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            Print("[ServerSentry][InventoryDebug][Logger.Flush.writeLinesOk]");
        }
    }

    /**
    	\brief Processes the prepare event operation.
    	\param ev \p ev used by the operation.
    */
    protected void PrepareEvent(ServerSentryEvent ev)
    {
        m_Sequence++;
        ev.sequence = m_Sequence;

        if (ev.schemaVersion <= 0)
        {
            ev.schemaVersion = ServerSentryDefaults.SchemaVersion();
        }

        if (ev.occurredAtUtc == "")
        {
            ev.occurredAtUtc = ServerSentryTime.NowUtc();
        }

        if (ev.serverId == "")
        {
            ev.serverId = ServerSentryDefaults.ServerId();
        }

        if (ev.bootId == "")
        {
            ev.bootId = m_BootId;
        }

        if (ev.eventId == "")
        {
            ev.eventId = ev.serverId + "-" + ev.bootId + "-" + ev.sequence.ToString();
        }

        if (ev.mod == "")
        {
            ev.mod = "ServerSentry";
        }

        if (ev.severity == "")
        {
            ev.severity = "info";
        }
    }

    /**
    	\brief Processes the enqueue JSON operation.
    	\param json \p json used by the operation.
    	\param severity \p severity used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    protected bool EnqueueJson(string json, string severity)
    {
        if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            Print("[ServerSentry][InventoryDebug][Logger.EnqueueJson.enter] severity=" + severity + " jsonLen=" + json.Length().ToString() + " buffer=" + m_Buffer.Count().ToString() + " max=" + ServerSentryDefaults.MaxQueueSize().ToString());
        }

        if (m_Buffer.Count() >= ServerSentryDefaults.MaxQueueSize())
        {
            m_DroppedEvents++;

            int warnEvery = ServerSentryDefaults.DroppedEventWarnEvery();
            if (warnEvery > 0 && (m_DroppedEvents == 1 || (m_DroppedEvents % warnEvery) == 0))
            {
                Print("[ServerSentry] Event queue is full. Dropped events: " + m_DroppedEvents.ToString());
            }

            if (ServerSentryDefaults.DebugInventoryMoveEvents())
            {
                Print("[ServerSentry][InventoryDebug][Logger.EnqueueJson.full] dropped=" + m_DroppedEvents.ToString());
            }
            return false;
        }

        m_Buffer.Insert(json);

        if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            Print("[ServerSentry][InventoryDebug][Logger.EnqueueJson.done] buffer=" + m_Buffer.Count().ToString());
        }
        return true;
    }

    /**
    	\brief Appends dropped event.
    	\param batch \p collection of batch values.
    */
    protected void AppendDroppedEvent(array<string> batch)
    {
        if (m_DroppedEvents <= 0)
        {
            return;
        }

        int dropped = m_DroppedEvents;
        m_DroppedEvents = 0;

        ServerSentryEvent droppedEvent = ServerSentryEventFactory.CreateDroppedEvents(dropped);
        PrepareEvent(droppedEvent);

        string json;
        string error;
        if (JsonFileLoader<ServerSentryEvent>.MakeData(droppedEvent, json, error, false) && json != "")
        {
            batch.Insert(json);
        }
        else
        {
            Print("[ServerSentry] Failed to serialize dropped-events marker - " + error);
        }
    }

    /**
    	\brief Processes the requeue batch operation.
    	\param batch \p collection of batch values.
    */
    protected void RequeueBatch(array<string> batch)
    {
        if (!batch)
        {
            return;
        }

        for (int i = batch.Count() - 1; i >= 0; i--)
        {
            if (m_Buffer.Count() >= ServerSentryDefaults.MaxQueueSize())
            {
                m_DroppedEvents++;
                continue;
            }

            m_Buffer.InsertAt(batch.Get(i), 0);
        }
    }

    /**
    	\brief Processes the time now utc operation.
    	\return \p The resulting string value.
    */
    protected string ServerSentryTimeNowUtc()
    {
        return ServerSentryTime.NowUtc();
    }

    /**
    	\brief Processes the player login operation.
    	\param player \p player entity associated with the operation.
    	\param identity \p player identity associated with the operation.
    */
    void PlayerLogin(DayZPlayer player, PlayerIdentity identity)
    {
        Emit(ServerSentryEventFactory.CreatePlayerLogin(player, identity));
    }

    /**
    	\brief Processes the player logout operation.
    	\param player \p player entity associated with the operation.
    	\param identity \p player identity associated with the operation.
    */
    void PlayerLogout(DayZPlayer player, PlayerIdentity identity)
    {
        Emit(ServerSentryEventFactory.CreatePlayerLogout(player, identity));
    }

    /**
    	\brief Processes the combat hit operation.
    	\param victim \p victim used by the operation.
    	\param hitContext \p hit context used by the operation.
    */
    void CombatHit(EntityAI victim, ServerSentryHitContext hitContext)
    {
        if (hitContext)
        {
            Emit(ServerSentryEventFactory.CreateCombatHit(null, hitContext));
            return;
        }

        Emit(ServerSentryEventFactory.CreateCombatHit(victim, null));
    }


    /**
    	\brief Processes the player hit operation.
    	\param victim \p victim used by the operation.
    	\param hitContext \p hit context used by the operation.
    */
    void PlayerHit(DayZPlayer victim, ServerSentryHitContext hitContext)
    {
        CombatHit(victim, hitContext);
    }


    /**
    	\brief Processes the mod action operation.
    	\param sourceMod \p source mod used by the operation.
    	\param player \p player entity associated with the operation.
    	\param actionName \p action name used by the operation.
    	\param message \p receives a human-readable result or error message.
    	\param dataJson \p data JSON used by the operation.
    */
    void ModAction(string sourceMod, DayZPlayer player, string actionName, string message, string dataJson = "{}")
    {
        EmitCustom(sourceMod, "mod_action", "mod", actionName, message, dataJson, player, "info");
    }

    /**
    	\brief Marks et transaction.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    */
    void MarketTransaction(ServerSentryMarketTransactionData data, DayZPlayer player = null)
    {
        EmitMarketTransaction(ServerSentryEventFactory.CreateMarketTransaction(data));
    }

    /**
    	\brief Processes the vehicle event operation.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    */
    void VehicleEvent(ServerSentryVehicleEventData data, DayZPlayer player = null)
    {
        EmitVehicleEvent(ServerSentryEventFactory.CreateVehicleEvent(data));
    }

    /**
    	\brief Processes the chat message operation.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    */
    void ChatMessage(ServerSentryChatMessageData data, DayZPlayer player = null)
    {
        if (!ServerSentryChatDeduper.ShouldEmit(data))
        {
            return;
        }

        EmitChatMessage(ServerSentryEventFactory.CreateChatMessage(data));
    }
};
