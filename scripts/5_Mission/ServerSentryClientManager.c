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
    protected bool m_IsWaitingForResponse;
    protected int m_RequestId;
    protected ref UIScriptedMenu m_RequestParent;
    protected bool m_IsCheckingTicketLink;
    protected int m_TicketLinkRequestId;
    protected ref ServerSentryTicketLinkStatus m_TicketLinkStatus;
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
        EnableMissionFinish();
        m_CF_Bindings.Bind("RequestSupportCaseFromServer", "UAToggleSupportMenu", false);

        GetRPCManager().AddRPC("ServerSentryBot", "SupportStateResponse", this, SingleplayerExecutionType.Client);
        GetRPCManager().AddRPC("ServerSentryBot", "TicketLinkStatusResponse", this, SingleplayerExecutionType.Client);
        GetRPCManager().AddRPC("ServerSentryBot", "TicketCreationResponse", this, SingleplayerExecutionType.Client);
    }

    /**
        \brief Opens menu.
    */
    protected void OpenResult(ServerSentrySupportCase data, UIScriptedMenu parent)
    {
        if (data)
        {
            ServerSentrySupportMenu caseMenu = ServerSentrySupportMenu.Cast(GetGame().GetUIManager().EnterScriptedMenu(4771341, parent));
            if (caseMenu)
                caseMenu.SetCaseData(data);
        }
        else
        {
            GetGame().GetUIManager().EnterScriptedMenu(ServerSentryTicketsMenu.MENU_ID, parent);
        }
    }

    /**
        \brief Requests support case from server.
    */
    void RequestSupportCaseFromServer()
    {
        if (m_IsWaitingForResponse)
            return;

        UIScriptedMenu parent = GetGame().GetUIManager().GetMenu();
        if (parent && !InGameMenu.Cast(parent))
            return;

        m_RequestParent = parent;
        m_IsWaitingForResponse = true;
        m_RequestId++;
        SetParentLoading(true);
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(OnSupportTimeout, 10000, false, m_RequestId);
        GetRPCManager().SendRPC("ServerSentryBot", "GetSupportState", new Param1<int>(m_RequestId), true);
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
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(OnTicketLinkTimeout);
        m_IsCheckingTicketLink = false;
        m_TicketLinkStatus = null;
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(OnTicketCreationTimeout);
        m_IsCreatingTicket = false;
        m_TicketSubmission = null;
        m_TicketCreationResult = null;
    }

    void RequestTicketLinkStatus()
    {
        if (m_IsCheckingTicketLink)
            return;

        m_TicketLinkStatus = null;
        m_IsCheckingTicketLink = true;
        m_TicketLinkRequestId++;
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(OnTicketLinkTimeout, 25000, false, m_TicketLinkRequestId);
        GetRPCManager().SendRPC("ServerSentryBot", "GetTicketLinkStatus", new Param1<int>(m_TicketLinkRequestId), true);
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

        m_TicketLinkStatus = response.param2;
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

    void SubmitTicket(ServerSentryTicketOption option, string subject, string message)
    {
        if (m_IsCreatingTicket)
            return;
        if (m_TicketCreationResult && m_TicketCreationResult.success)
            return;

        if (!m_TicketSubmission)
        {
            if (!option)
                return;
            int uuid[4];
            UUIDApi.Generate(uuid);
            m_TicketSubmission = new ServerSentryTicketSubmission;
            m_TicketSubmission.requestId = UUIDApi.FormatString(uuid);
            m_TicketSubmission.panelId = option.panelId;
            m_TicketSubmission.ticketTypeId = option.ticketTypeId;
            m_TicketSubmission.subject = subject.Trim();
            m_TicketSubmission.message = message.Trim();
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

    protected void ShowSupportError()
    {
        NotificationSystem.AddNotificationExtended(6, "Support unavailable", "Please try again in a moment.");
    }
};
