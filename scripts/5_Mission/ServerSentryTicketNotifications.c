/** The visible reply hint and its shortcut share one real-time deadline. */
class ServerSentryTicketNotifications
{
    protected static const float DISPLAY_SECONDS = 10;
    protected ref array<string> m_Seen = {};
    protected Widget m_Hint;
    protected ref ServerSentryTicketSummary m_Ticket;
    protected float m_VisibleUntil;

    void Stop()
    {
        Consume();
        m_Seen.Clear();
    }

    bool Receive(ServerSentryTicketNotification notification, bool alreadyOpen)
    {
        if (!notification || !notification.ticket || notification.ticket.ticketId == "" || notification.messageId == "" || !notification.IsFresh()) return false;
        if (m_Seen.Find(notification.messageId) >= 0) return false;
        m_Seen.Insert(notification.messageId);
        if (m_Seen.Count() > 64) m_Seen.Remove(0);
        if (alreadyOpen)
        {
            if (m_Ticket && m_Ticket.ticketId == notification.ticket.ticketId) Consume();
            return true;
        }
        if (!m_Hint)
        {
            ServerSentryRoundedSurface.LoadStyles();
            m_Hint = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/components/TicketHint.layout");
            if (!m_Hint) return false;
            GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Insert(Update);
        }
        m_Ticket = notification.ticket;
        m_VisibleUntil = GetGame().GetTickTime() + DISPLAY_SECONDS;
        string key = ShortcutLabel();
        string detail = "Open Tickets in the Support menu.";
        if (key != "") detail = "Press [" + key + "] to open.";
        string title = "New support reply";
        if (m_Ticket.ticketNumber != "") title = "New reply in Ticket " + m_Ticket.ticketNumber;
        TextWidget.Cast(m_Hint.FindAnyWidget("Title")).SetText(title);
        TextWidget.Cast(m_Hint.FindAnyWidget("Detail")).SetText(detail);
        return true;
    }

    protected string ShortcutLabel()
    {
        map<int, ref TStringArray> alternatives = InputUtils.GetComboButtonNamesFromInput("UAOpenSupportNotification", EUAINPUT_DEVICE_KEYBOARDMOUSE);
        if (!alternatives || alternatives.Count() == 0) return "";
        TStringArray buttons = alternatives.GetElement(0);
        string label;
        foreach (string button : buttons)
        {
            if (label != "") label += " + ";
            label += button;
        }
        return label;
    }

    protected void Update(float timeslice)
    {
        if (GetGame().GetTickTime() >= m_VisibleUntil) Consume();
    }

    ServerSentryTicketSummary GetTicket()
    {
        if (m_Ticket && GetGame().GetTickTime() >= m_VisibleUntil) Consume();
        return m_Ticket;
    }

    void Consume()
    {
        m_Ticket = null;
        if (!m_Hint) return;
        GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Remove(Update);
        m_Hint.Unlink();
        m_Hint = null;
    }
}
