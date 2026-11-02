/** Keeps the draft and uncertain delivery receipt alive when the player closes the menu. */
class ServerSentryTicketChat
{
    protected ref ServerSentryTicketConversationRequest m_Read;
    protected ref ServerSentryTicketConversationRequest m_Reply;
    protected ref ServerSentryTicketConversationResult m_History;
    protected ref ServerSentryTicketConversationResult m_Delivery;
    protected ref map<string, string> m_Drafts = new map<string, string>;
    string GetDraft(string ticketId)
    {
        string text;
        m_Drafts.Find(ticketId, text);
        return text;
    }
    void SetDraft(string ticketId, string text) { m_Drafts.Set(ticketId, text); }

    protected bool m_Reading;
    protected bool m_Sending;
    protected bool m_ReplyWaitingForRead;
    protected string m_QueuedReadTicketId;

    void Stop()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(OnTimeout);
    }

    ServerSentryTicketConversationResult GetHistory() { return m_History; }
    ServerSentryTicketConversationResult GetDelivery() { return m_Delivery; }
    ServerSentryTicketConversationRequest GetReply() { return m_Reply; }
    bool IsReading() { return m_Reading; }
    bool HasUnconfirmedReply() { return m_Reply && (!m_Delivery || (!m_Delivery.success && !m_Delivery.canEdit)); }
    bool IsSending() { return m_Sending; }

    void Read(string ticketId)
    {
        if (m_Reading || m_Sending)
        {
            // Coalesce requests for the same read; retain the latest different ticket.
            if (m_Sending || m_Read.ticketId != ticketId) m_QueuedReadTicketId = ticketId;
            return;
        }
        m_QueuedReadTicketId = "";
        m_Read = NewRequest(ticketId);
        m_Reading = true;
        Send(m_Read);
    }

    void ReadAfterNotification(string ticketId)
    {
        // An in-flight read may have fetched its messages before this new reply arrived.
        if (m_Reading || m_Sending) m_QueuedReadTicketId = ticketId;
        else Read(ticketId);
    }

    void Reply(string ticketId, string text)
    {
        if (m_Sending) return;
        // An unconfirmed reply can only check the original request; its text and ID stay fixed.
        if (!HasUnconfirmedReply())
        {
            m_Reply = NewRequest(ticketId);
            m_Reply.sending = true;
            m_Reply.message = text.Trim();
        }
        if (m_Reply.ticketId != ticketId || !m_Reply.IsValid()) return;
        m_Sending = true;
        m_Delivery = null;
        // The server accepts one pending request per player, so finish the refresh first.
        m_ReplyWaitingForRead = m_Reading;
        if (!m_ReplyWaitingForRead) Send(m_Reply);
    }

    protected ServerSentryTicketConversationRequest NewRequest(string ticketId)
    {
        ServerSentryTicketConversationRequest request = new ServerSentryTicketConversationRequest;
        int uuid[4];
        UUIDApi.Generate(uuid);
        request.requestId = UUIDApi.FormatString(uuid);
        request.ticketId = ticketId;
        return request;
    }

    protected void Send(ServerSentryTicketConversationRequest request)
    {
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(OnTimeout, 130000, false, request.requestId, request.sending);
        GetRPCManager().SendRPC("ServerSentryBot", "TicketConversation", new Param1<ServerSentryTicketConversationRequest>(request), true);
    }

    void Receive(ServerSentryTicketConversationResult result)
    {
        if (!result) return;
        ServerSentryTicketConversationRequest request = m_Read;
        if (result.sending) request = m_Reply;
        if (!request || result.requestId != request.requestId || result.ticketId != request.ticketId) return;
        if (result.sending)
        {
            if (m_Delivery && m_Delivery.success) return;
            m_Delivery = result;
            m_Sending = false;
            // Delivery can arrive while the menu is closed. Keep a newer draft intact.
            if (result.success && GetDraft(result.ticketId).Trim() == request.message)
                SetDraft(result.ticketId, "");
        }
        else
        {
            m_History = result;
            m_Reading = false;
            if (m_ReplyWaitingForRead)
            {
                m_ReplyWaitingForRead = false;
                Send(m_Reply);
            }
            else if (result.success && HasUnconfirmedReply() && m_Reply.ticketId == result.ticketId && !m_Sending)
            {
                // The backend checks the original receipt; it never posts this reply twice.
                Reply(m_Reply.ticketId, m_Reply.message);
            }
        }
        StartQueuedRead();
    }

    protected void StartQueuedRead()
    {
        if (m_Reading || m_Sending || m_QueuedReadTicketId == "") return;
        string ticketId = m_QueuedReadTicketId;
        m_QueuedReadTicketId = "";
        Read(ticketId);
    }

    protected void OnTimeout(string requestId, bool sending)
    {
        ServerSentryTicketConversationRequest request = m_Read;
        bool waiting = m_Reading;
        if (sending) { request = m_Reply; waiting = m_Sending; }
        if (!waiting || !request || request.requestId != requestId) return;
        ServerSentryTicketConversationResult result = new ServerSentryTicketConversationResult;
        result.requestId = requestId;
        result.ticketId = request.ticketId;
        result.sending = sending;
        result.message = "The conversation could not be loaded. Please refresh.";
        if (sending) result.message = "Delivery is not confirmed. Check the conversation, then check delivery again.";
        Receive(result);
    }
}
