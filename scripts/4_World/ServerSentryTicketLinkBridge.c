class ServerSentryPendingTicketLink
{
    PlayerIdentity identity;
    int clientRequestId;
    string requestId;
    int deadline;
}

/** Server-owned requests; clients cannot supply a Steam ID or a file path. */
class ServerSentryTicketLinkBridge
{
    protected static const string ROOT = "$profile:ServerSentry/tickets/link-status";
    protected ref array<ref ServerSentryPendingTicketLink> m_Pending = {};

    void ~ServerSentryTicketLinkBridge()
    {
        Stop();
    }

    void Stop()
    {
        if (GetGame())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Poll);
        foreach (ServerSentryPendingTicketLink pending : m_Pending)
            Cleanup(pending);
        m_Pending.Clear();
    }

    void Request(PlayerIdentity identity, ServerSentryTicketLinkQuery query)
    {
        if (!ServerSentryConfigManager.Get().IsTicketsEnabled()) return;
        foreach (ServerSentryPendingTicketLink existing : m_Pending)
        {
            if (existing.identity == identity)
                return;
        }

        ServerSentryPendingTicketLink pending = new ServerSentryPendingTicketLink;
        pending.identity = identity;
        pending.clientRequestId = query.requestId;
        pending.deadline = GetGame().GetTime() + 20000;

        int uuid[4];
        UUIDApi.Generate(uuid);
        pending.requestId = UUIDApi.FormatString(uuid);

        array<string> directories = {"$profile:ServerSentry", "$profile:ServerSentry/tickets", ROOT, ROOT + "/requests", ROOT + "/responses"};
        foreach (string directory : directories)
        {
            if (!FileExist(directory) && !MakeDirectory(directory))
            {
                Reply(pending, Unavailable());
                return;
            }
        }

        ServerSentryTicketLinkRequest request = new ServerSentryTicketLinkRequest;
        request.requestId = pending.requestId;
        request.steamId64 = identity.GetPlainId();
        request.includeTickets = query.includeTickets;
        request.refreshOptions = query.refreshOptions;
        string requestPath = ROOT + "/requests/" + pending.requestId;
        string error;
        if (!JsonFileLoader<ServerSentryTicketLinkRequest>.SaveFile(requestPath + ".json", request, error))
        {
            Print("[ServerSentry][Tickets] Cannot write link request: " + error);
            Cleanup(pending);
            Reply(pending, Unavailable());
            return;
        }

        // Publish only after SaveFile has closed the complete JSON file.
        FileHandle marker = OpenFile(requestPath + ".ready", FileMode.WRITE);
        if (!marker)
        {
            Cleanup(pending);
            Reply(pending, Unavailable());
            return;
        }
        CloseFile(marker);

        m_Pending.Insert(pending);
        if (m_Pending.Count() == 1)
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Poll, 500, true);
    }

    protected void Poll()
    {
        for (int i = m_Pending.Count() - 1; i >= 0; i--)
        {
            ServerSentryPendingTicketLink pending = m_Pending[i];
            string responsePath = ROOT + "/responses/" + pending.requestId + ".json";
            ServerSentryTicketLinkStatus status;
            string error;
            if (FileExist(responsePath))
            {
                if (!JsonFileLoader<ServerSentryTicketLinkStatus>.LoadFile(responsePath, status, error) || !status || status.requestId != pending.requestId)
                    status = Unavailable();
            }
            else if (GetGame().GetTime() >= pending.deadline)
            {
                status = Unavailable();
            }
            else
            {
                continue;
            }

            Cleanup(pending);
            m_Pending.Remove(i);
            Reply(pending, status);
        }

        if (m_Pending.Count() == 0)
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Poll);
    }

    protected void Cleanup(ServerSentryPendingTicketLink pending)
    {
        string requestPath = ROOT + "/requests/" + pending.requestId;
        DeleteFile(requestPath + ".ready");
        DeleteFile(requestPath + ".json");
        DeleteFile(ROOT + "/responses/" + pending.requestId + ".json");
    }

    protected void Reply(ServerSentryPendingTicketLink pending, ServerSentryTicketLinkStatus status)
    {
        if (pending.identity && pending.identity.GetPlayer())
            GetRPCManager().SendRPC("ServerSentryBot", "TicketLinkStatusResponse", new Param2<int, ServerSentryTicketLinkStatus>(pending.clientRequestId, status), true, pending.identity);
    }

    protected ServerSentryTicketLinkStatus Unavailable()
    {
        ServerSentryTicketLinkStatus status = new ServerSentryTicketLinkStatus;
        status.message = "Support is unavailable. Please try again.";
        return status;
    }
}
