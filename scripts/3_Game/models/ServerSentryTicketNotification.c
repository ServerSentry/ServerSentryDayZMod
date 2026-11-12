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
