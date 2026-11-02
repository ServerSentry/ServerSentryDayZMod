class NavigationReviewMission extends MissionGameplay
{
    protected int m_Step;
    protected int m_Failures;
    protected UIScriptedMenu m_Pause;
    protected ServerSentryTicketsMenu m_Tickets;
    protected ServerSentrySupportMenu m_Case;
    protected UIScriptedMenu m_Form;

    override void OnInit()
    {
        super.OnInit();
        vector position = "7500 0 7500";
        position[1] = GetGame().SurfaceY(position[0], position[2]) + 0.2;
        PlayerBase player = PlayerBase.Cast(GetGame().CreatePlayer(null, "SurvivorM_Mirek", position, 0, "NONE"));
        if (player) GetGame().SelectPlayer(null, player);
    }

    override void OnMissionStart()
    {
        super.OnMissionStart();
        ServerSentryModFeatures features = new ServerSentryModFeatures;
        features.tickets = true;
        features.supportCases = true;
        ServerSentryClientManager.Get().ApplyModFeatures(features);
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(Step, 1800, false);
    }

    void Check(bool passed, string name)
    {
        if (!passed) m_Failures++;
        Print("[MenuNavigationReview] " + passed + " " + name);
    }

    void OpenPause()
    {
        m_Pause = GetGame().GetUIManager().GetMenu();
        if (!m_Pause) m_Pause = GetGame().GetUIManager().EnterScriptedMenu(MENU_INGAME, null);
        GetGame().GetMission().AddActiveInputExcludes({"menu"});
    }

    void OpenTickets()
    {
        ServerSentryClientManager.Get().RequestSupportCaseFromServer();
        m_Tickets = ServerSentryTicketsMenu.Cast(GetGame().GetUIManager().GetMenu());
    }

    void OpenCase()
    {
        m_Case = ServerSentrySupportMenu.Cast(GetGame().GetUIManager().EnterScriptedMenu(4771341, m_Tickets));
        if (m_Case) m_Case.SetCaseData(null);
    }

    void SelectTickets()
    {
        m_Case.OnClick(m_Case.GetLayoutRoot().FindAnyWidget("NavSupport"), 0, 0, 0);
    }

    void Step()
    {
        switch (m_Step)
        {
            case 0: OpenPause(); break;
            case 1:
                Check(m_Pause && m_Pause.GetID() == MENU_INGAME, "pause fixture ready");
                OpenTickets();
                break;
            case 2:
                Check(m_Tickets && m_Tickets.GetParentMenu() == m_Pause, "tickets preserve pause parent");
                if (!m_Tickets) return;
                OpenCase();
                break;
            case 3:
                if (!m_Case) { Check(false, "reimbursement menu created"); return; }
                SelectTickets();
                break;
            case 4:
                Check(GetGame().GetUIManager().GetMenu() == m_Tickets, "first navigation click returns to tickets");
                Check(GetFocus() == m_Tickets.GetLayoutRoot(), "tickets regain widget focus");
                Check(IsInputExcludeActive("menu"), "pause input exclusion preserved");
                OpenCase();
                break;
            case 5: SelectTickets(); break;
            case 6:
                Check(GetGame().GetUIManager().GetMenu() == m_Tickets, "repeated navigation still needs one click");
                m_Form = GetGame().GetUIManager().EnterScriptedMenu(ServerSentrySupportDeskMenu.MENU_ID, m_Tickets);
                break;
            case 7:
                m_Form.OnClick(m_Form.GetLayoutRoot().FindAnyWidget("SupportClose"), 0, 0, 0);
                break;
            case 8:
                Check(GetGame().GetUIManager().GetMenu() == m_Pause, "close exits entire support stack");
                Check(IsInputExcludeActive("menu"), "closing support keeps pause inputs excluded");
                Check(GetFocus() == m_Pause.GetLayoutRoot(), "pause regains widget focus");
                Print("[MenuNavigationReview] cursor while paused=" + GetGame().GetUIManager().IsCursorVisible());
                GetGame().GetMission().Continue();
                break;
            case 9:
                Check(!GetGame().GetUIManager().GetMenu(), "continue closes pause");
                Check(!IsInputExcludeActive("menu"), "continue releases pause input exclusion");
                OpenTickets();
                break;
            case 10:
                Check(m_Tickets && !m_Tickets.GetParentMenu(), "hotkey entry has no hidden pause parent");
                Check(IsInputExcludeActive("menu"), "hotkey entry owns input exclusion");
                OpenCase();
                break;
            case 11:
                m_Case.OnClick(m_Case.GetLayoutRoot().FindAnyWidget("SupportClose"), 0, 0, 0);
                break;
            case 12:
                Check(!GetGame().GetUIManager().GetMenu(), "close from hotkey reimbursement returns to game");
                Check(!IsInputExcludeActive("menu"), "hotkey close releases its own exclusion");
                Print("[MenuNavigationReview] Finished failures=" + m_Failures);
                OpenPause();
                break;
            case 13: OpenTickets(); break;
            case 14:
                OpenCase();
                Print("[MenuNavigationReview] Ready for mouse-only review; no backend calls or claims");
                return;
        }
        m_Step++;
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(Step, 250, false);
    }
}
void main() {}
Mission CreateCustomMission(string path) { return new NavigationReviewMission; }