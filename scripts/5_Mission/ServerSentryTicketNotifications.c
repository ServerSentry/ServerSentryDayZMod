/** The latest visible ticket notification owns the shortcut for ten seconds. */
class ServerSentryTicketNotifications
{
    protected ref array<string> m_Seen = {};
    protected ref array<ref ServerSentryTicketNotificationRuntime> m_OwnNotifications = {};
    protected ServerSentryTicketNotificationRuntime m_Active;
    protected int m_Deadline;
    protected bool m_Listening;

    void Stop()
    {
        NotificationSystem system = NotificationSystem.GetInstance();
        if (system && m_Listening)
        {
            system.m_OnNotificationAdded.Remove(OnAdded);
            system.m_OnNotificationRemoved.Remove(OnRemoved);
        }
        m_Listening = false;
        m_Active = null;
        m_OwnNotifications.Clear();
        m_Seen.Clear();
    }

    bool Receive(ServerSentryTicketNotification notification, bool alreadyOpen)
    {
        if (!notification || !notification.ticket || notification.ticket.ticketId == "" || notification.messageId == "" || !notification.IsFresh()) return false;
        if (m_Seen.Find(notification.messageId) >= 0) return false;
        m_Seen.Insert(notification.messageId);
        if (m_Seen.Count() > 64) m_Seen.Remove(0);
        if (alreadyOpen) return true;
        NotificationSystem system = NotificationSystem.GetInstance();
        if (!system) return false;
        if (!m_Listening)
        {
            system.m_OnNotificationAdded.Insert(OnAdded);
            system.m_OnNotificationRemoved.Insert(OnRemoved);
            m_Listening = true;
        }
        string key = ShortcutLabel();
        string detail = "New reply in your support ticket.";
        if (key != "") detail += " Press [" + key + "] to open.";
        else detail += " Open Tickets in the Support menu.";
        NotificationData appearance = new NotificationData("ServerSentryDayZMod/gui/skin/serversentry_logo.edds", notification.ticket.GetReference());
        ServerSentryTicketNotificationRuntime runtime = new ServerSentryTicketNotificationRuntime(10, appearance, detail);
        runtime.ticket = notification.ticket;
        m_OwnNotifications.Insert(runtime);
        NotificationSystem.AddTicketNotification(runtime);
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

    protected void OnAdded(NotificationRuntimeData data)
    {
        ServerSentryTicketNotificationRuntime runtime = ServerSentryTicketNotificationRuntime.Cast(data);
        if (!runtime || m_OwnNotifications.Find(runtime) < 0) return;
        m_Active = runtime;
        m_Deadline = GetGame().GetTime() + 10000;
    }

    protected void OnRemoved(NotificationRuntimeData data)
    {
        ServerSentryTicketNotificationRuntime runtime = ServerSentryTicketNotificationRuntime.Cast(data);
        if (!runtime) return;
        m_OwnNotifications.RemoveItem(runtime);
        if (m_Active == runtime) m_Active = null;
    }

    ServerSentryTicketSummary GetTicket()
    {
        if (!m_Active || GetGame().GetTime() >= m_Deadline || m_Active.GetRemainingTime() <= 0) return null;
        return m_Active.ticket;
    }

    void Consume() { m_Active = null; }
}
