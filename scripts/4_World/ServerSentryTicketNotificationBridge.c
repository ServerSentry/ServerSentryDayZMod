/** One small inbox, written atomically by the authenticated Agent. No Discord polling. */
class ServerSentryTicketNotificationBridge
{
    protected static const string ROOT = "$profile:ServerSentry/tickets/notifications";

    void Start()
    {
        if (!ServerSentryConfigManager.Get().IsTicketsEnabled()) return;
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Poll, 1000, true);
    }

    void Stop()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Poll);
    }

    protected void Poll()
    {
        string name;
        FileAttr attributes;
        FindFileHandle files = CF.FindFileEx(ROOT + "/*.json", name, attributes, FindFileFlags.ALL);
        if (!files) return;
        array<Man> players = {};
        GetGame().GetPlayers(players);
        int processed;
        bool found = true;
        while (found && processed < 32)
        {
            string path = ROOT + "/" + name;
            ServerSentryTicketNotification notification;
            string error;
            if (!(attributes & FileAttr.DIRECTORY) && JsonFileLoader<ServerSentryTicketNotification>.LoadFile(path, notification, error))
            {
                if (notification && notification.ticket && notification.IsFresh())
                    Deliver(notification, players);
            }
            DeleteFile(path);
            processed++;
            found = FindNextFile(files, name, attributes);
        }
        CloseFindFile(files);
    }

    protected void Deliver(ServerSentryTicketNotification notification, array<Man> players)
    {
        foreach (Man player : players)
        {
            PlayerIdentity identity = player.GetIdentity();
            if (identity && identity.GetPlainId() == notification.steamId64)
            {
                GetRPCManager().SendRPC("ServerSentryBot", "TicketNotification", new Param1<ServerSentryTicketNotification>(notification), true, identity);
                return;
            }
        }
    }
}
