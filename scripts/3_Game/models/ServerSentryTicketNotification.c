class ServerSentryTicketNotification
{
    string steamId64;
    string messageId;
    string expiresAtUtc;
    ref ServerSentryTicketSummary ticket;

    bool IsFresh()
    {
        string now = ServerSentryTime.NowUtc();
        if (expiresAtUtc.Length() != now.Length()) return false;
        for (int i = 0; i < now.Length(); i++)
        {
            int current = now.Substring(i, 1).ToAscii();
            int expiry = expiresAtUtc.Substring(i, 1).ToAscii();
            if (current != expiry) return current < expiry;
        }
        return false;
    }
}

class ServerSentryTicketNotificationRuntime extends NotificationRuntimeData
{
    ref ServerSentryTicketSummary ticket;

    void ServerSentryTicketNotificationRuntime(float time, NotificationData data, string detail_text)
    {
        // CF adjusts the base duration for fading; the shortcut uses ten visible seconds.
        m_NotificationTime = time;
        m_TimeRemaining = time;
    }
}

/** Uses the native queue so the shortcut starts when the notification becomes visible. */
modded class NotificationSystem
{
    static void AddTicketNotification(ServerSentryTicketNotificationRuntime data)
    {
        if (!m_Instance) return;
        if (m_Instance.m_TimeArray.Count() < MAX_NOTIFICATIONS)
        {
            m_Instance.m_TimeArray.Insert(data);
            m_Instance.m_OnNotificationAdded.Invoke(data);
        }
        else m_Instance.m_DeferredArray.Insert(data);
    }

    /** Remove only the opened ticket's popup and let the native UI fade it out. */
    static void DismissTicketNotification(ServerSentryTicketNotificationRuntime data)
    {
        if (!m_Instance || !data) return;
        m_Instance.m_DeferredArray.RemoveItem(data);
        if (m_Instance.m_TimeArray.Find(data) < 0) return;
        m_Instance.m_TimeArray.RemoveItem(data);
        m_Instance.m_OnNotificationRemoved.Invoke(data);
        if (m_Instance.m_DeferredArray.Count() == 0) return;
        int next = m_Instance.m_DeferredArray.Count() - 1;
        NotificationRuntimeData deferred = m_Instance.m_DeferredArray.Get(next);
        m_Instance.m_DeferredArray.Remove(next);
        m_Instance.m_TimeArray.Insert(deferred);
        m_Instance.m_OnNotificationAdded.Invoke(deferred);
    }
}
