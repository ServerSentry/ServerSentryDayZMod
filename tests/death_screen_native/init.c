/** Synthetic transport boundary; all widgets below are the production component. */
class DeathReportReviewState extends ServerSentryDeathReports
{
    DeathScreenReviewMission mission;
    void Configure() { m_Dead = true; m_KillEventId = "local-fixture-only"; m_Sending = false; m_Confirming = false; }
    override void Submit()
    {
        if (!CanSubmit()) return;
        m_Sending = true;
        mission.ReportPressed();
    }
}

class UnrelatedDeathMenu extends UIScriptedMenu
{
    override Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets("$mission:CustomDeath.layout");
        TextWidget.Cast(layoutRoot.FindAnyWidget("ThirdPartyTitle")).SetTextExactSize(24);
        TextWidget.Cast(layoutRoot.FindAnyWidget("ThirdPartySubtitle")).SetTextExactSize(16);
        return layoutRoot;
    }
}

class DeathScreenReviewMission extends MissionGameplay
{
    protected ref ServerSentryDeathReportButton m_ReportButton;
    protected ref DeathReportReviewState m_Reports;
    protected ref UnrelatedDeathMenu m_CustomMenu;
    protected int m_Clicks;

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
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ShowVanilla, 2000, false);
    }

    void ShowVanilla()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        player.SetHealth("", "Health", 0);
        player.SimulateDeath(true);
        player.ShowDeadScreen(true, 0);
        m_Reports = new DeathReportReviewState;
        m_Reports.mission = this;
        m_Reports.Configure();
        m_ReportButton = new ServerSentryDeathReportButton(m_Reports);
        GetGame().GetUIManager().ShowCursor(true);
        Print("[DeathScreenReview] Ready vanilla; native fade=" + GetGame().GetUIManager().ScreenFadeVisible());
    }

    void ReportPressed()
    {
        m_Clicks++;
        Print("[DeathScreenReview] CLICK stage=" + m_Clicks + " native fade=" + GetGame().GetUIManager().ScreenFadeVisible());
        if (m_Clicks == 1) GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ShowCustom, 1000, false);
        if (m_Clicks == 2) GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ShowAdapter, 1000, false);
        if (m_Clicks == 3)
        {
            m_Reports.ReceiveReceipt("local-fixture-only", true, "Local receipt fixture; no CMS report was created.");
            Print("[DeathScreenReview] Finished clicks=3; no reports sent");
        }
    }

    void ShowCustom()
    {
        m_CustomMenu = new UnrelatedDeathMenu;
        m_CustomMenu.SetID(4771490);
        GetGame().GetUIManager().ShowScriptedMenu(m_CustomMenu, null);
        // A custom death-screen owner must render its own root above the native fade.
        m_CustomMenu.GetLayoutRoot().SetSort(90000);
        m_Reports.Configure();
        m_ReportButton.Refresh();
        Print("[DeathScreenReview] Ready custom menu; report button is still an independent root");
    }

    void ShowAdapter()
    {
        m_ReportButton.Detach();
        Widget slot = m_CustomMenu.GetLayoutRoot().FindAnyWidget("ProvidedReportSlot");
        m_Reports.Configure();
        m_ReportButton = m_Reports.AttachButton(slot);
        Print("[DeathScreenReview] Ready explicit adapter; same component attached to the owner's provided slot");
    }
}

void main() {}
Mission CreateCustomMission(string path) { return new DeathScreenReviewMission; }
