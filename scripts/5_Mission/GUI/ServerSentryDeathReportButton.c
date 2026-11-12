/** Additive component: no dependency on another mod's death-screen widget names. */
class ServerSentryDeathReportButton : ScriptedWidgetEventHandler
{
    protected Widget m_Root;
    protected Widget m_Button;
    protected Widget m_Confirmation;
    protected Widget m_Send;
    protected Widget m_Cancel;
    protected TextWidget m_Label;
    protected TextWidget m_Status;
    protected ref ServerSentryRoundedSurface m_Surface;
    protected ref ServerSentryRoundedSurface m_SendSurface;
    protected ServerSentryDeathReports m_Reports;

    void ServerSentryDeathReportButton(ServerSentryDeathReports reports, Widget parent = null)
    {
        m_Reports = reports;
        m_Root = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/components/DeathReportButton.layout", parent);
        if (!m_Root) return;
        m_Root.SetSort(100000);
        m_Button = m_Root.FindAnyWidget("DeathReportButton");
        m_Button.SetHandler(this);
        m_Confirmation = m_Root.FindAnyWidget("DeathReportConfirmation");
        m_Send = m_Root.FindAnyWidget("DeathReportSend");
        m_Cancel = m_Root.FindAnyWidget("DeathReportCancel");
        m_Send.SetHandler(this);
        m_Cancel.SetHandler(this);
        TextWidget.Cast(m_Root.FindAnyWidget("DeathReportSendLabel")).SetTextExactSize(18);
        TextWidget.Cast(m_Root.FindAnyWidget("DeathReportCancelLabel")).SetTextExactSize(18);
        m_Label = TextWidget.Cast(m_Root.FindAnyWidget("DeathReportLabel"));
        m_Status = TextWidget.Cast(m_Root.FindAnyWidget("DeathReportStatus"));
        m_Label.SetTextExactSize(18);
        m_Status.SetTextExactSize(14);
        m_Surface = new ServerSentryRoundedSurface(m_Root.FindAnyWidget("DeathReportFill"));
        m_Surface.SetColor(ARGB(245, 20, 51, 64));
        m_SendSurface = new ServerSentryRoundedSurface(m_Root.FindAnyWidget("DeathReportSendFill"));
        m_SendSurface.SetColor(ARGB(245, 20, 51, 64));
        Refresh();
    }

    void Refresh()
    {
        if (!m_Root || !m_Reports) return;
        m_Root.Show(m_Reports.CanShow());
        bool confirming = m_Reports.IsConfirming() && m_Reports.CanSubmit();
        m_Button.Show(!confirming);
        m_Button.Enable(m_Reports.CanSubmit());
        m_Confirmation.Show(confirming);
        if (m_Reports.IsReceived()) m_Label.SetText("Report received");
        else if (m_Reports.IsSending()) m_Label.SetText("Submitting report...");
        else m_Label.SetText("Report suspected cheating");
        if (confirming) m_Status.SetText("Send this death and its combat details to the admin team for review?");
        else if (m_Reports.GetMessage() != "") m_Status.SetText(m_Reports.GetMessage());
        else m_Status.SetText("Think someone cheated? Ask the admin team to review this death.");
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (!m_Reports || button != 0) return false;
        if (w == m_Button) m_Reports.AskToReport();
        else if (w == m_Send && m_Reports.IsConfirming()) m_Reports.Submit();
        else if (w == m_Cancel) m_Reports.CancelReport();
        else return false;
        Refresh();
        return true;
    }
    override bool OnMouseEnter(Widget w, int x, int y)
    {
        if (w == m_Button && m_Surface) m_Surface.SetColor(ARGB(255, 29, 78, 96));
        if (w == m_Send && m_SendSurface) m_SendSurface.SetColor(ARGB(255, 29, 78, 96));
        return false;
    }
    override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
    {
        if (w == m_Button && m_Surface) m_Surface.SetColor(ARGB(245, 20, 51, 64));
        if (w == m_Send && m_SendSurface) m_SendSurface.SetColor(ARGB(245, 20, 51, 64));
        return false;
    }
    Widget GetRoot() { return m_Root; }
    void Detach()
    {
        if (m_Root) m_Root.Unlink();
        m_Root = null;
        m_Reports = null;
    }
    void ~ServerSentryDeathReportButton() { Detach(); }
}
