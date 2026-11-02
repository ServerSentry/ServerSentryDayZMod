/** Test-only button with its own handler: no dependency on a death menu's OnClick. */
class DeathReportProbeButton extends ScriptedWidgetEventHandler
{
    protected Widget m_Root;
    protected Widget m_Button;
    protected DeathScreenReviewMission m_Mission;
    protected ref ServerSentryRoundedSurface m_Surface;

    void DeathReportProbeButton(DeathScreenReviewMission mission, Widget parent = null)
    {
        m_Mission = mission;
        m_Root = GetGame().GetWorkspace().CreateWidgets("$mission:ReportOverlay.layout", parent);
        m_Root.SetSort(100000);
        m_Button = m_Root.FindAnyWidget("ReportProbeButton");
        m_Button.SetHandler(this);
        TextWidget.Cast(m_Root.FindAnyWidget("ReportProbeLabel")).SetTextExactSize(20);
        TextWidget.Cast(m_Root.FindAnyWidget("ProbeStatus")).SetTextExactSize(14);
        m_Surface = new ServerSentryRoundedSurface(m_Root.FindAnyWidget("ReportProbeFill"));
        m_Surface.SetColor(ARGB(255, 20, 51, 64));
    }

    void Detach() { if (m_Root) m_Root.Unlink(); }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (w != m_Button) return false;
        m_Mission.ReportPressed();
        return true;
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
    protected ref DeathReportProbeButton m_ReportButton;
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
        m_ReportButton = new DeathReportProbeButton(this);
        Print("[DeathScreenReview] Ready vanilla; native fade=" + GetGame().GetUIManager().ScreenFadeVisible());
    }

    void ReportPressed()
    {
        m_Clicks++;
        Print("[DeathScreenReview] CLICK stage=" + m_Clicks + " native fade=" + GetGame().GetUIManager().ScreenFadeVisible());
        if (m_Clicks == 1) GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ShowCustom, 1000, false);
        if (m_Clicks == 2) GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ShowAdapter, 1000, false);
        if (m_Clicks == 3) Print("[DeathScreenReview] Finished clicks=3; no reports sent");
    }

    void ShowCustom()
    {
        m_CustomMenu = new UnrelatedDeathMenu;
        m_CustomMenu.SetID(4771490);
        GetGame().GetUIManager().ShowScriptedMenu(m_CustomMenu, null);
        Print("[DeathScreenReview] Ready custom menu; report button is still an independent root");
    }

    void ShowAdapter()
    {
        m_ReportButton.Detach();
        Widget slot = m_CustomMenu.GetLayoutRoot().FindAnyWidget("ProvidedReportSlot");
        m_ReportButton = new DeathReportProbeButton(this, slot);
        Print("[DeathScreenReview] Ready explicit adapter; same component attached to the owner's provided slot");
    }
}

void main() {}
Mission CreateCustomMission(string path) { return new DeathScreenReviewMission; }
