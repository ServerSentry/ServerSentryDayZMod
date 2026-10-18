/**
	\file MissionServer.c
	\brief Defines ServerSentry support for Mission server.
*/

/**
	\brief Extends the DayZ Mission server class with ServerSentry integration.
*/
modded class MissionServer
{
    /**
    	\brief Instance state for login emitted.
    */
    protected ref map<string, bool> m_ServerSentryLoginEmitted;

    /**
        \brief Enables item lifecycle observations only after mission startup.
    */
    override void OnMissionStart()
    {
        super.OnMissionStart();
        ServerSentryItemLifecycle.Start();
        // ServerSentryDamageZoneExporter.Reset(); // Development diagnostics only; disabled in production.
    }

    /**
        \brief Suppresses item removals during shutdown and flushes already recorded events.
    */
    override void OnMissionFinish()
    {
        ServerSentryItemLifecycle.Stop();
        ServerSentryMapMarkers.Clear();
        if (ServerSentryConfigManager.Get().IsAllowedToStartTelemetryLogging())
            ServerSentryLogger.Get().Flush();
        super.OnMissionFinish();
    }

    /**
        \brief Stops deletion tracking and clears markers if the mission is destroyed directly.
    */
    void ~MissionServer()
    {
        ServerSentryItemLifecycle.Stop();
        ServerSentryMapMarkers.Clear();
    }

    /**
    	\brief Handles the DayZ Init callback.
    */
    override void OnInit()
    {
        super.OnInit();

        m_ServerSentryLoginEmitted = new map<string, bool>;

        if (ServerSentryConfigManager.Get().IsAllowedToExportItems())
        {
            Print("[Server Sentry] -> Item Export Enabled starting to export Items!");
            ServerSentryExporter.Export();
            Print("[Server Sentry] -> Item Export Finished!");
        }

        if (ServerSentryConfigManager.Get().IsAllowedToStartLiveLogging())
        {
            Print("[Server Sentry] -> Live Logging is enabled, starting module!");
            ServerSentryModActionBootstrap.Get().RegisterDefaults();
            ServerSentryLiveBaseDetailBootstrap.Get().RegisterDefaults();
            ServerSentryModActionPoller.Start();
            ServerSentryLiveApi.Start();
            Print("[Server Sentry] -> Live Logging module started!");
        }

        if (ServerSentryConfigManager.Get().IsAllowedToStartTelemetryLogging())
        {
            Print("[Server Sentry] -> Telemetry Logging is enabled, starting module!");
            ServerSentryLogger.Get().Start();
            Print("[Server Sentry] -> Telemetry Logging module started!");
        }
    }
    
    /**
    	\brief Handles the DayZ Client ready event callback.
    	\param identity \p player identity associated with the operation.
    	\param player \p player entity associated with the operation.
    */
    override void OnClientReadyEvent(PlayerIdentity identity, PlayerBase player)
    {
        super.OnClientReadyEvent(identity, player);

        // ServerSentryDamageZoneExporter.Observe(player); // Development diagnostics only; disabled in production.
        ServerSentryApplyVoiceMuteState(player, identity);
        ServerSentryEmitLoginOnce(player, identity);
    }

    /**
    	\brief Handles the DayZ Client respawn event callback.
    	\param identity \p player identity associated with the operation.
    	\param player \p player entity associated with the operation.
    */
    override void OnClientRespawnEvent(PlayerIdentity identity, PlayerBase player)
    {
        super.OnClientRespawnEvent(identity, player);
        // ServerSentryDamageZoneExporter.Observe(player); // Development diagnostics only; disabled in production.
        

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.ServerSentryApplyVoiceMuteState, 15000, false, player, identity);
    }

    /**
    	\brief Handles the DayZ Client reconnect event callback.
    	\param identity \p player identity associated with the operation.
    	\param player \p player entity associated with the operation.
    */
    override void OnClientReconnectEvent(PlayerIdentity identity, PlayerBase player)
    {
        super.OnClientReconnectEvent(identity, player);

        // ServerSentryDamageZoneExporter.Observe(player); // Development diagnostics only; disabled in production.
        ServerSentryApplyVoiceMuteState(player, identity);
    }

    /**
    	\brief Processes the apply voice mute state operation.
    	\param player \p player entity associated with the operation.
    	\param identity \p player identity associated with the operation.
    */
    protected void ServerSentryApplyVoiceMuteState(PlayerBase player, PlayerIdentity identity)
    {
        if (!player || !identity)
            return;

        string steamId = identity.GetPlainId();
        bool isMuted = ServerSentryVoiceMuteDb.Get().IsVoiceMuted(steamId);
        bool canSpeak = !isMuted;

        Print("[Server Sentry] Voice state apply: name=" + identity.GetName() + " steamId=" + steamId + " isMuted=" + isMuted.ToString() + " canSpeak=" + canSpeak.ToString());

        GetGame().EnableVoN(player, canSpeak);
    }

    /**
    	\brief Processes the emit login once operation.
    	\param player \p player entity associated with the operation.
    	\param identity \p player identity associated with the operation.
    */
    protected void ServerSentryEmitLoginOnce(PlayerBase player, PlayerIdentity identity)
    {
        if (!identity)
            return;

        if (!m_ServerSentryLoginEmitted)
            m_ServerSentryLoginEmitted = new map<string, bool>;

        string steamId = identity.GetPlainId();

        if (m_ServerSentryLoginEmitted.Contains(steamId))
        {
            Print("[Server Sentry] Login event skipped, already emitted for steamId=" + steamId);
            return;
        }

        m_ServerSentryLoginEmitted.Insert(steamId, true);

        ServerSentryEvent ssLoginEvent = ServerSentryEventFactory.CreatePlayerLogin(player, identity);
        if (ssLoginEvent)
        {
            ServerSentryLogger.Get().Emit(ssLoginEvent);
            Print("[Server Sentry] Login event emitted for steamId=" + steamId);
        }
    }

    /**
    	\brief Processes the invoke on disconnect operation.
    	\param player \p player entity associated with the operation.
    */
    override void InvokeOnDisconnect(PlayerBase player)
    {
        ServerSentryEvent ssLogoutEvent;

        if (player)
        {
            PlayerIdentity identity = player.GetIdentity();
            if (identity)
            {
                string steamId = identity.GetPlainId();

                if (m_ServerSentryLoginEmitted)
                    m_ServerSentryLoginEmitted.Remove(steamId);

                ssLogoutEvent = ServerSentryEventFactory.CreatePlayerLogout(player, identity);
            }
        }

        if (ssLogoutEvent)
            ServerSentryLogger.Get().Emit(ssLogoutEvent);

        ServerSentryLogger.Get().Flush();

        super.InvokeOnDisconnect(player);
    }

    /**
    	\brief Handles the DayZ Event callback.
    	\param eventTypeId \p DayZ event type identifier.
    	\param params \p parameters supplied by the DayZ event.
    */
    override void OnEvent(EventType eventTypeId, Param params)
    {
        ServerSentryVanillaChatCapture.Emit(eventTypeId, params);
        super.OnEvent(eventTypeId, params);
    }
};
