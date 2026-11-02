class ServerSentryPendingTicketConversation
{
    PlayerIdentity identity;
    string requestId;
    string fileKey;
    int deadline;
    string ticketId;
    bool sending;
}

class ServerSentryTicketConversationBridge
{
    protected static const string ROOT = "$profile:ServerSentry/tickets/conversation";
    protected ref array<ref ServerSentryPendingTicketConversation> m_Pending = {};

    void Stop()
    {
        if (GetGame())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Poll);
        foreach (ServerSentryPendingTicketConversation pending : m_Pending)
            Cleanup(pending);
        m_Pending.Clear();
    }

    void ~ServerSentryTicketConversationBridge()
    {
        Stop();
    }

    void Request(PlayerIdentity identity, ServerSentryTicketConversationRequest submission)
    {
        if (!ServerSentryConfigManager.Get().IsTicketsEnabled()) return;
        if (!submission.IsValid())
        {
            ServerSentryTicketConversationResult rejected = Failure("Enter a message of up to 2000 characters.");
            rejected.ticketId = submission.ticketId;
            rejected.sending = submission.sending;
            rejected.canEdit = true;
            Reply(identity, submission.requestId, rejected);
            return;
        }
        foreach (ServerSentryPendingTicketConversation existing : m_Pending)
        {
            if (existing.identity == identity)
                return;
        }

        ServerSentryPendingTicketConversation pending = new ServerSentryPendingTicketConversation;
        pending.identity = identity;
        pending.requestId = submission.requestId;
        pending.ticketId = submission.ticketId;
        pending.sending = submission.sending;
        pending.fileKey = identity.GetPlainId() + "-" + submission.requestId;
        pending.deadline = GetGame().GetTime() + 120000;
        if (!Publish(pending, submission))
        {
            Cleanup(pending);
            ServerSentryTicketConversationResult failure = Failure("The ticket service is unavailable. Please refresh or check delivery.");
            failure.canEdit = true;
            failure.ticketId = submission.ticketId;
            failure.sending = submission.sending;
            Reply(identity, pending.requestId, failure);
            return;
        }

        m_Pending.Insert(pending);
        if (m_Pending.Count() == 1)
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Poll, 500, true);
    }

    protected bool Publish(ServerSentryPendingTicketConversation pending, ServerSentryTicketConversationRequest submission)
    {
        array<string> directories = {"$profile:ServerSentry", "$profile:ServerSentry/tickets", ROOT, ROOT + "/requests", ROOT + "/responses"};
        foreach (string directory : directories)
        {
            if (!FileExist(directory) && !MakeDirectory(directory))
                return false;
        }

        ServerSentryTicketConversationFileRequest request = new ServerSentryTicketConversationFileRequest;
        request.requestId = pending.requestId;
        request.steamId64 = pending.identity.GetPlainId();
        request.ticketId = submission.ticketId;
        request.sending = submission.sending;
        request.message = submission.message;
        string path = ROOT + "/requests/" + pending.fileKey;
        string error;
        if (!JsonFileLoader<ServerSentryTicketConversationFileRequest>.SaveFile(path + ".json", request, error))
        {
            Print("[ServerSentry][Tickets] Cannot write conversation request: " + error);
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
            ServerSentryPendingTicketConversation pending = m_Pending[i];
            string path = ROOT + "/responses/" + pending.fileKey + ".json";
            ServerSentryTicketConversationResult result;
            string error;
            if (FileExist(path))
            {
                if (!JsonFileLoader<ServerSentryTicketConversationResult>.LoadFile(path, result, error) || !result || result.requestId != pending.requestId)
                    result = Failure("The conversation is unavailable. Please refresh or check delivery.");
            }
            else if (GetGame().GetTime() >= pending.deadline)
            {
                result = Failure("The conversation is unavailable. Please refresh or check delivery.");
            }
            else
            {
                continue;
            }
            result.ticketId = pending.ticketId;
            result.sending = pending.sending;
            Cleanup(pending);
            m_Pending.Remove(i);
            Reply(pending.identity, pending.requestId, result);
        }
        if (m_Pending.Count() == 0)
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Poll);
    }

    protected void Cleanup(ServerSentryPendingTicketConversation pending)
    {
        DeleteFile(ROOT + "/requests/" + pending.fileKey + ".ready");
        DeleteFile(ROOT + "/requests/" + pending.fileKey + ".json");
        DeleteFile(ROOT + "/responses/" + pending.fileKey + ".json");
    }

    protected void Reply(PlayerIdentity identity, string requestId, ServerSentryTicketConversationResult result)
    {
        result.requestId = requestId;
        if (identity && identity.GetPlayer())
            GetRPCManager().SendRPC("ServerSentryBot", "TicketConversationResponse", new Param1<ServerSentryTicketConversationResult>(result), true, identity);
    }

    protected ServerSentryTicketConversationResult Failure(string message)
    {
        ServerSentryTicketConversationResult result = new ServerSentryTicketConversationResult;
        result.message = message;

        return result;
    }
}
