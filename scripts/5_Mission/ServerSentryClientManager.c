/**
	\file ServerSentryClientManager.c
	\brief Defines ServerSentry support for Client manager.
*/

[CF_RegisterModule(ServerSentryClientManager)]
/**
	\brief Coordinates the lifecycle of Client.
*/
class ServerSentryClientManager: CF_ModuleWorld
{
    // No local config reads or reporting objects: the server must explicitly opt in.
    protected bool m_DeathReportsEnabled;
    protected bool m_TicketsEnabled;
    protected bool m_SupportCasesEnabled;
    bool IsTicketsEnabled() { return m_TicketsEnabled; }
    bool IsSupportCasesEnabled() { return m_SupportCasesEnabled; }
    bool HasSupportFeatures() { return m_TicketsEnabled || m_SupportCasesEnabled; }
    bool IsDeathReportingEnabled() { return m_DeathReportsEnabled; }

    protected ref ServerSentryTicketChat m_TicketChat;
    protected ref ServerSentryTicketNotifications m_TicketNotifications;
    ServerSentryTicketChat GetTicketChat() { return m_TicketChat; }

    protected bool m_IsWaitingForResponse;
    protected int m_RequestId;
    protected ref UIScriptedMenu m_RequestParent;
    protected bool m_IsCheckingTicketLink;
    protected int m_TicketLinkRequestId;
    protected ref ServerSentryTicketLinkStatus m_TicketLinkStatus;
    protected int m_TicketOptionsValidUntil;
    protected bool m_IsCreatingTicket;
    protected ref ServerSentryTicketSubmission m_TicketSubmission;
    protected ref ServerSentryTicketCreationResult m_TicketCreationResult;

    static ServerSentryClientManager Get()
    {
        return ServerSentryClientManager.Cast(CF_ModuleCoreManager.Get(ServerSentryClientManager));
    }

    /**
        \brief Handles the DayZ Init callback.
    */
    override void OnInit()
    {
        super.OnInit();
        m_DeathReportsEnabled = false;
        EnableMissionFinish();
        GetRPCManager().AddRPC("ServerSentryBot", "ModFeatures", this, SingleplayerExecutionType.Client);
    }

    void ModFeatures(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Client) return;
        Param1<ServerSentryModFeatures> settings;
        if (ctx.Read(settings) && settings && settings.param1) ApplyModFeatures(settings.param1);
    }

    void ApplyModFeatures(ServerSentryModFeatures features)
    {
        // This module owns only Support/J bindings; rebuild them without duplicates.
        m_CF_Bindings.m_Head = null;
        if (features.tickets || features.supportCases)
            m_CF_Bindings.Bind("RequestSupportCaseFromServer", "UAToggleSupportMenu", false);
        if (features.tickets)
        {
            if (!m_TicketChat) m_TicketChat = new ServerSentryTicketChat;
            if (!m_TicketNotifications) m_TicketNotifications = new ServerSentryTicketNotifications;
            m_CF_Bindings.Bind("OpenNotifiedTicket", "UAOpenSupportNotification", false);
            GetRPCManager().AddRPC("ServerSentryBot", "TicketConversationResponse", this, SingleplayerExecutionType.Client);
            GetRPCManager().AddRPC("ServerSentryBot", "TicketNotification", this, SingleplayerExecutionType.Client);
            GetRPCManager().AddRPC("ServerSentryBot", "TicketLinkStatusResponse", this, SingleplayerExecutionType.Client);
            GetRPCManager().AddRPC("ServerSentryBot", "TicketCreationResponse", this, SingleplayerExecutionType.Client);
        }
        else StopTicketRuntime();
        if (!features.supportCases) CancelReimbursementRequest();
        if (features.supportCases && !m_SupportCasesEnabled)
            GetRPCManager().AddRPC("ServerSentryBot", "SupportStateResponse", this, SingleplayerExecutionType.Client);
        m_TicketsEnabled = features.tickets;
        m_SupportCasesEnabled = features.supportCases;
        m_DeathReportsEnabled = features.deathReports;
    }

    /**
        \brief Opens menu.
    */
    protected void OpenResult(ServerSentrySupportCase data, UIScriptedMenu parent)
    {
        ServerSentrySupportMenu caseMenu = ServerSentrySupportMenu.Cast(GetGame().GetUIManager().EnterScriptedMenu(4771341, parent));
        if (caseMenu) caseMenu.SetCaseData(data);
    }

    /**
        \brief Requests support case from server.
    */
    void RequestSupportCaseFromServer()
    {
        if (!HasSupportFeatures()) return;
        UIScriptedMenu parent = GetGame().GetUIManager().GetMenu();
        if (parent && !InGameMenu.Cast(parent)) return;
        if (m_TicketsEnabled) GetGame().GetUIManager().EnterScriptedMenu(ServerSentryTicketsMenu.MENU_ID, parent);
        else
        {
            GetGame().GetUIManager().EnterScriptedMenu(4771341, parent);
            RequestReimbursement();
        }
    }

    void OpenNotifiedTicket()
    {
        if (!m_TicketsEnabled) return;
        ServerSentryTicketSummary ticket = m_TicketNotifications.GetTicket();
        if (!ticket) return;
        UIScriptedMenu parent = GetGame().GetUIManager().GetMenu();
        if (parent && !InGameMenu.Cast(parent) && !ServerSentrySupportMenuBase.Cast(parent)) return;
        ServerSentryTicketConversationMenu current = ServerSentryTicketConversationMenu.Cast(parent);
        if (current && current.IsTicket(ticket.ticketId))
        {
            current.RefreshFromNotification();
            m_TicketNotifications.Consume();
            return;
        }
        // Keep the Tickets root for the sidebar, and preserve any existing draft underneath.
        if (!ServerSentrySupportMenuBase.Cast(parent))
            parent = GetGame().GetUIManager().EnterScriptedMenu(ServerSentryTicketsMenu.MENU_ID, parent);
        ServerSentryTicketConversationMenu conversation = ServerSentryTicketConversationMenu.Cast(GetGame().GetUIManager().EnterScriptedMenu(ServerSentryTicketConversationMenu.MENU_ID, parent));
        if (conversation)
        {
            conversation.SetTicket(ticket);
            m_TicketNotifications.Consume();
        }
    }

    void TicketNotification(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Client) return;
        Param1<ServerSentryTicketNotification> response;
        if (ctx.Read(response) && response) ReceiveTicketNotification(response.param1);
    }

    void ReceiveTicketNotification(ServerSentryTicketNotification notification)
    {
        if (!m_TicketsEnabled) return;
        if (!notification || !notification.ticket) return;
        ServerSentryTicketConversationMenu conversation = ServerSentryTicketConversationMenu.Cast(GetGame().GetUIManager().GetMenu());
        bool alreadyOpen = conversation && conversation.IsTicket(notification.ticket.ticketId);
        if (!m_TicketNotifications.Receive(notification, alreadyOpen)) return;
        if (alreadyOpen) conversation.RefreshFromNotification();
        else m_TicketChat.ReadAfterNotification(notification.ticket.ticketId);
    }

    /** Loads reimbursements only when the player selects that section. */
    void RequestReimbursement()
    {
        if (!m_SupportCasesEnabled) return;
        if (m_IsWaitingForResponse)
            return;

        UIScriptedMenu parent = GetGame().GetUIManager().GetMenu();
        if (!ServerSentrySupportMenuBase.Cast(parent))
            return;

        m_RequestParent = parent;
        m_IsWaitingForResponse = true;
        m_RequestId++;
        SetParentLoading(true);
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(OnSupportTimeout, 10000, false, m_RequestId);
        GetRPCManager().SendRPC("ServerSentryBot", "GetSupportState", new Param1<int>(m_RequestId), true);
    }

    bool IsCheckingReimbursement() { return m_IsWaitingForResponse; }

    void CancelReimbursementRequest()
    {
        if (m_IsWaitingForResponse) FinishRequest();
    }

    void SupportStateResponse(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Client || !m_IsWaitingForResponse)
            return;

        Param2<int, ServerSentrySupportCase> response;
        if (!ctx.Read(response) || !response)
        {
            FinishRequest();
            ShowSupportError();
            return;
        }
        if (response.param1 != m_RequestId)
            return;

        UIScriptedMenu parent = m_RequestParent;
        bool canOpen = GetGame().GetUIManager().GetMenu() == parent;
        FinishRequest();
        if (canOpen)
            OpenResult(response.param2, parent);
    }

    protected void SetParentLoading(bool loading)
    {
        if (GetGame().GetUIManager().GetMenu() != m_RequestParent)
            return;

        InGameMenu pauseMenu = InGameMenu.Cast(m_RequestParent);
        if (pauseMenu)
            pauseMenu.SetSupportLoading(loading);
    }

    protected void FinishRequest()
    {
        m_IsWaitingForResponse = false;
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(OnSupportTimeout);
        SetParentLoading(false);
        m_RequestParent = null;
    }

    protected void OnSupportTimeout(int requestId)
    {
        if (!m_IsWaitingForResponse || requestId != m_RequestId)
            return;

        bool stillOnParent = GetGame().GetUIManager().GetMenu() == m_RequestParent;
        FinishRequest();
        if (stillOnParent)
            ShowSupportError();
    }

    override void OnMissionFinish(Class sender, CF_EventArgs args)
    {
        super.OnMissionFinish(sender, args);
        m_DeathReportsEnabled = false;
        CancelReimbursementRequest();
        m_CF_Bindings.m_Head = null;
        m_TicketsEnabled = false;
        m_SupportCasesEnabled = false;
        StopTicketRuntime();
    }

    protected void StopTicketRuntime()
    {
        if (m_TicketChat) m_TicketChat.Stop();
        m_TicketChat = null;
        if (m_TicketNotifications) m_TicketNotifications.Stop();
        m_TicketNotifications = null;
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(OnTicketLinkTimeout);
        m_IsCheckingTicketLink = false;
        m_TicketLinkStatus = null;
        m_TicketOptionsValidUntil = 0;
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(OnTicketCreationTimeout);
        m_IsCreatingTicket = false;
        m_TicketSubmission = null;
        m_TicketCreationResult = null;
    }

    void RequestTicketLinkStatus(bool includeTickets = true, bool refreshOptions = false)
    {
        if (!m_TicketsEnabled) return;
        if (!includeTickets && !refreshOptions && HasFreshTicketOptions())
            return;
        if (m_IsCheckingTicketLink)
            return;

        m_TicketLinkStatus = null;
        m_IsCheckingTicketLink = true;
        m_TicketLinkRequestId++;
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(OnTicketLinkTimeout, 25000, false, m_TicketLinkRequestId);
        ServerSentryTicketLinkQuery query = new ServerSentryTicketLinkQuery;
        query.requestId = m_TicketLinkRequestId;
        query.includeTickets = includeTickets;
        query.refreshOptions = refreshOptions;
        SendTicketLinkQuery(query);
    }

    protected void SendTicketLinkQuery(ServerSentryTicketLinkQuery query)
    {
        GetRPCManager().SendRPC("ServerSentryBot", "GetTicketLinkStatus", new Param1<ServerSentryTicketLinkQuery>(query), true);
    }

    protected bool HasFreshTicketOptions()
    {
        if (!m_TicketLinkStatus) return false;
        if (!m_TicketLinkStatus.success || !m_TicketLinkStatus.discordLinked) return false;
        return GetGame().GetTime() < m_TicketOptionsValidUntil;
    }

    ServerSentryTicketLinkStatus GetTicketLinkStatus()
    {
        return m_TicketLinkStatus;
    }

    bool IsCheckingTicketLink()
    {
        return m_IsCheckingTicketLink;
    }

    void TicketLinkStatusResponse(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Client || !m_IsCheckingTicketLink)
            return;

        Param2<int, ServerSentryTicketLinkStatus> response;
        if (!ctx.Read(response) || !response || response.param1 != m_TicketLinkRequestId || !response.param2)
            return;

        ApplyTicketLinkStatus(response.param2);
    }

    protected void ApplyTicketLinkStatus(ServerSentryTicketLinkStatus status)
    {
        m_TicketLinkStatus = status;
        m_TicketOptionsValidUntil = GetGame().GetTime() + Math.Clamp(m_TicketLinkStatus.optionsValidForSeconds, 0, 60) * 1000;
        m_IsCheckingTicketLink = false;
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(OnTicketLinkTimeout);
        Print("[ServerSentry][Tickets] Link status received. Success=" + m_TicketLinkStatus.success + " Linked=" + m_TicketLinkStatus.discordLinked);
    }

    protected void OnTicketLinkTimeout(int requestId)
    {
        if (!m_IsCheckingTicketLink || requestId != m_TicketLinkRequestId)
            return;

        m_IsCheckingTicketLink = false;
        m_TicketLinkStatus = new ServerSentryTicketLinkStatus;
        m_TicketLinkStatus.message = "Support is unavailable. Please try again.";
    }

    void SubmitTicket(ServerSentryTicketOption option, string message, bool includePlayerInfo)
    {
        if (!m_TicketsEnabled) return;
        if (m_IsCreatingTicket)
            return;
        if (m_TicketCreationResult && m_TicketCreationResult.success)
            return;

        if (!m_TicketSubmission || (m_TicketCreationResult && m_TicketCreationResult.canEdit))
        {
            if (!option)
                return;
            if (!m_TicketSubmission)
            {
                int uuid[4];
                UUIDApi.Generate(uuid);
                m_TicketSubmission = new ServerSentryTicketSubmission;
                m_TicketSubmission.requestId = UUIDApi.FormatString(uuid);
            }
            m_TicketSubmission.panelId = option.panelId;
            m_TicketSubmission.ticketTypeId = option.ticketTypeId;
            m_TicketSubmission.message = message.Trim();
            m_TicketSubmission.includePlayerInfo = includePlayerInfo;
        }
        if (!m_TicketSubmission.IsValid())
        {
            m_TicketCreationResult = new ServerSentryTicketCreationResult;
            m_TicketCreationResult.canEdit = true;
            m_TicketCreationResult.message = "Choose a ticket type and enter a message of up to 4000 characters.";
            return;
        }
        m_IsCreatingTicket = true;
        m_TicketCreationResult = null;
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(OnTicketCreationTimeout, 130000, false, m_TicketSubmission.requestId);
        GetRPCManager().SendRPC("ServerSentryBot", "CreateTicket", new Param1<ServerSentryTicketSubmission>(m_TicketSubmission), true);
    }

    bool IsCreatingTicket()
    {
        return m_IsCreatingTicket;
    }

    ServerSentryTicketSubmission GetTicketSubmission()
    {
        return m_TicketSubmission;
    }

    ServerSentryTicketCreationResult GetTicketCreationResult()
    {
        return m_TicketCreationResult;
    }

    void StartNewTicket()
    {
        if (m_IsCreatingTicket || !m_TicketCreationResult || !m_TicketCreationResult.success)
            return;
        m_TicketSubmission = null;
        m_TicketCreationResult = null;
    }

    void TicketCreationResponse(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Client || !m_TicketSubmission)
            return;
        Param2<string, ServerSentryTicketCreationResult> response;
        if (!ctx.Read(response) || !response || !response.param2 || response.param1 != m_TicketSubmission.requestId)
            return;

        if (m_TicketCreationResult && m_TicketCreationResult.success)
            return;

        // A late success can still confirm a request after the local timeout.
        m_TicketCreationResult = response.param2;
        m_IsCreatingTicket = false;
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(OnTicketCreationTimeout);
    }

    protected void OnTicketCreationTimeout(string requestId)
    {
        if (!m_IsCreatingTicket || !m_TicketSubmission || requestId != m_TicketSubmission.requestId)
            return;
        m_IsCreatingTicket = false;
        m_TicketCreationResult = new ServerSentryTicketCreationResult;
        m_TicketCreationResult.message = "Ticket status is unknown. Retry to check the same request.";
    }

    void TicketConversationResponse(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Client || !m_TicketsEnabled) return;
        Param1<ServerSentryTicketConversationResult> response;
        if (ctx.Read(response) && response)
            m_TicketChat.Receive(response.param1);
    }

    protected void ShowSupportError()
    {
        NotificationSystem.AddNotificationExtended(6, "Support unavailable", "Please try again in a moment.");
    }
};
