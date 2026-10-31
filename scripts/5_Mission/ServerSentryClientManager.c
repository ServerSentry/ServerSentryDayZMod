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
        m_CF_Bindings.Bind("RequestSupportCaseFromServer", "UAToggleSupportMenu", false);

        GetRPCManager().AddRPC("ServerSentryBot", "SupportStateResponse", this, SingleplayerExecutionType.Client);
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
            GetGame().GetUIManager().EnterScriptedMenu(ServerSentrySupportDeskMenu.MENU_ID, parent);
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

    protected void ShowSupportError()
    {
        NotificationSystem.AddNotificationExtended(6, "Support unavailable", "Please try again in a moment.");
    }
};
