class ServerSentryPendingTicketCreation
{
    PlayerIdentity identity;
    string requestId;
    string fileKey;
    int deadline;
}

class ServerSentryTicketCreationBridge
{
    protected static const string ROOT = "$profile:ServerSentry/tickets/create";
    protected ref array<ref ServerSentryPendingTicketCreation> m_Pending = {};

    void Stop()
    {
        if (GetGame())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Poll);
        foreach (ServerSentryPendingTicketCreation pending : m_Pending)
            Cleanup(pending);
        m_Pending.Clear();
    }

    void ~ServerSentryTicketCreationBridge()
    {
        Stop();
    }

    void Request(PlayerIdentity identity, ServerSentryTicketSubmission submission)
    {
        if (!submission.IsValid())
        {
            Reply(identity, submission.requestId, Failure("Choose a ticket type and enter a subject and message within the length limits."));
            return;
        }
        foreach (ServerSentryPendingTicketCreation existing : m_Pending)
        {
            if (existing.identity == identity)
                return;
        }

        ServerSentryPendingTicketCreation pending = new ServerSentryPendingTicketCreation;
        pending.identity = identity;
        pending.requestId = submission.requestId;
        pending.fileKey = identity.GetPlainId() + "-" + submission.requestId;
        pending.deadline = GetGame().GetTime() + 120000;
        if (!Publish(pending, submission))
        {
            Cleanup(pending);
            Reply(identity, pending.requestId, Failure("Ticket service is unavailable. Retry to check the same request."));
            return;
        }

        m_Pending.Insert(pending);
        if (m_Pending.Count() == 1)
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Poll, 500, true);
    }

    protected bool Publish(ServerSentryPendingTicketCreation pending, ServerSentryTicketSubmission submission)
    {
        array<string> directories = {"$profile:ServerSentry", "$profile:ServerSentry/tickets", ROOT, ROOT + "/requests", ROOT + "/responses"};
        foreach (string directory : directories)
        {
            if (!FileExist(directory) && !MakeDirectory(directory))
                return false;
        }

        ServerSentryTicketCreationRequest request = new ServerSentryTicketCreationRequest;
        request.requestId = pending.requestId;
        request.steamId64 = pending.identity.GetPlainId();
        request.panelId = submission.panelId;
        request.ticketTypeId = submission.ticketTypeId;
        request.subject = submission.subject;
        request.message = submission.message;
        string path = ROOT + "/requests/" + pending.fileKey;
        string error;
        if (!JsonFileLoader<ServerSentryTicketCreationRequest>.SaveFile(path + ".json", request, error))
        {
            Print("[ServerSentry][Tickets] Cannot write creation request: " + error);
            return false;
        }
        FileHandle marker = OpenFile(path + ".ready", FileMode.WRITE);
        if (!marker)
            return false;
        CloseFile(marker);
        return true;
    }

    protected void Poll()
    {
        for (int i = m_Pending.Count() - 1; i >= 0; i--)
        {
            ServerSentryPendingTicketCreation pending = m_Pending[i];
            string path = ROOT + "/responses/" + pending.fileKey + ".json";
            ServerSentryTicketCreationResult result;
            string error;
            if (FileExist(path))
            {
                if (!JsonFileLoader<ServerSentryTicketCreationResult>.LoadFile(path, result, error) || !result || result.requestId != pending.requestId)
                    result = Failure("Ticket status is unknown. Retry to check the same request.");
            }
            else if (GetGame().GetTime() >= pending.deadline)
            {
                result = Failure("Ticket status is unknown. Retry to check the same request.");
            }
            else
            {
                continue;
            }
            Cleanup(pending);
            m_Pending.Remove(i);
            Reply(pending.identity, pending.requestId, result);
        }
        if (m_Pending.Count() == 0)
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Poll);
    }

    protected void Cleanup(ServerSentryPendingTicketCreation pending)
    {
        DeleteFile(ROOT + "/requests/" + pending.fileKey + ".ready");
        DeleteFile(ROOT + "/requests/" + pending.fileKey + ".json");
        DeleteFile(ROOT + "/responses/" + pending.fileKey + ".json");
    }

    protected void Reply(PlayerIdentity identity, string requestId, ServerSentryTicketCreationResult result)
    {
        if (identity && identity.GetPlayer())
            GetRPCManager().SendRPC("ServerSentryBot", "TicketCreationResponse", new Param2<string, ServerSentryTicketCreationResult>(requestId, result), true, identity);
    }

    protected ServerSentryTicketCreationResult Failure(string message)
    {
        ServerSentryTicketCreationResult result = new ServerSentryTicketCreationResult;
        result.message = message;
        return result;
    }
}
