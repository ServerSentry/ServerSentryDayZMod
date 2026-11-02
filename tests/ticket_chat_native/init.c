/** Offline regression: fake transport, actual chat state and conversation widgets. */
class TicketChatReviewTransport extends ServerSentryTicketChat
{
    ref array<ref ServerSentryTicketConversationRequest> sent = {};

    override protected void Send(ServerSentryTicketConversationRequest request)
    {
        sent.Insert(request);
    }

    void History(bool waitingForPlayer, bool hasStatus = true)
    {
        ServerSentryTicketConversationResult result = new ServerSentryTicketConversationResult;
        result.requestId = m_Read.requestId;
        result.ticketId = m_Read.ticketId;
        result.success = true;
        result.messages = {};
        result.hasReplyStatus = hasStatus;
        result.isWaitingForPlayer = waitingForPlayer;
        Receive(result);
    }

    void Delivery(bool success)
    {
        ServerSentryTicketConversationResult result = new ServerSentryTicketConversationResult;
        result.requestId = m_Reply.requestId;
        result.ticketId = m_Reply.ticketId;
        result.sending = true;
        result.success = success;
        Receive(result);
    }
}

class TicketChatReviewMenu extends ServerSentryTicketConversationMenu
{
    protected string m_ReviewStatus;
    override protected void RefreshTicketStatus()
    {
        super.RefreshTicketStatus();
        m_ReviewStatus = m_Ticket.GetStatusLabel();
    }
    void UseTransport(TicketChatReviewTransport chat) { m_Chat = chat; }
    void Write(string text)
    {
        m_Composer.SetText(text);
        m_Chat.SetDraft(m_Ticket.ticketId, text);
    }
    void RefreshNow() { RefreshChat(); }
    string Text() { return m_Composer.GetText(); }
    bool CanWrite() { return !(m_Composer.GetInput().GetFlags() & WidgetFlags.DISABLED); }
    string Subtitle() { return m_ReviewStatus; }
    void SendReply() { OnClick(m_Send, 0, 0, 0); }
}

class TicketChatReviewMission extends MissionGameplay
{
    protected int m_Failures;
    protected ref TicketChatReviewTransport m_Transport;
    protected ref TicketChatReviewMenu m_Menu;
    protected ref ServerSentryTicketSummary m_Ticket;

    override void OnInit()
    {
        super.OnInit();
        vector position = "7500 0 7500";
        position[1] = GetGame().SurfaceY(position[0], position[2]) + 0.2;
        PlayerBase player = PlayerBase.Cast(GetGame().CreatePlayer(null, "SurvivorM_Mirek", position, 0, "NONE"));
        if (player) GetGame().SelectPlayer(null, player);
    }

    override void OnMissionStart()
    {
        super.OnMissionStart();
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(Run, 2000, false);
    }

    void Check(bool passed, string name)
    {
        if (!passed) m_Failures++;
        Print("[TicketChatReview] " + passed + " " + name);
    }

    void OpenConversation()
    {
        m_Menu = new TicketChatReviewMenu;
        GetGame().GetUIManager().ShowScriptedMenu(m_Menu, null);
        m_Menu.UseTransport(m_Transport);
        m_Menu.SetTicket(m_Ticket);
    }

    void Run()
    {
        ServerSentryModFeatures features = new ServerSentryModFeatures;
        features.tickets = true;
        features.supportCases = true;
        ServerSentryClientManager.Get().ApplyModFeatures(features);
        CheckQueuedReads();
        m_Transport = new TicketChatReviewTransport;
        m_Ticket = new ServerSentryTicketSummary;
        m_Ticket.ticketId = "offline-ticket";
        m_Ticket.ticketTypeName = "Offline regression";
        m_Ticket.ticketNumber = "1";
        m_Ticket.isClaimed = true;
        OpenConversation();
        m_Transport.History(false);
        m_Menu.RefreshNow();
        Check(m_Menu.Subtitle() == "Waiting for support", "owner reply waits for support regardless of claim state");
        m_Menu.Write("First reply");
        m_Menu.SendReply();
        string requestId = m_Transport.GetReply().requestId;
        m_Transport.Delivery(false);
        m_Menu.RefreshNow();
        Check(m_Menu.Text() == "First reply" && m_Menu.CanWrite(), "uncertain delivery preserves editable text");

        int calls = m_Transport.sent.Count();
        m_Transport.Read(m_Ticket.ticketId);
        m_Transport.History(true);
        m_Menu.RefreshNow();
        Check(m_Transport.sent.Count() == calls + 2, "refresh automatically checks the delivery receipt");
        Check(m_Transport.GetReply().requestId == requestId && m_Transport.GetReply().message == "First reply", "receipt check retains original request identity and text");
        Check(m_Menu.Subtitle() == "Waiting for you", "support reply changes status even with unchanged rendered messages");
        m_Menu.Write("Next draft :smile:\nSecond line");
        m_Transport.Delivery(true);
        m_Menu.RefreshNow();
        Check(m_Menu.Text() == "Next draft :smile:\nSecond line" && m_Menu.CanWrite(), "late success preserves a newer editable draft");
        Check(m_Transport.GetDraft(m_Ticket.ticketId) == m_Menu.Text(), "newer saved draft survives confirmation");
        Check(m_Menu.Subtitle() == "Waiting for support", "confirmed own reply immediately waits for support");
        m_Transport.History(false, false);
        m_Menu.RefreshNow();
        Check(m_Menu.Subtitle() == "Waiting for support", "older backend without reply status preserves the known status");
        m_Transport.Read(m_Ticket.ticketId);
        m_Transport.History(false);
        m_Menu.RefreshNow();
        Check(m_Menu.Subtitle() == "Waiting for support", "owner reply status refreshes without reopening");

        m_Menu.Write("Sent while closed");
        m_Menu.SendReply();
        m_Menu.Close();
        m_Transport.Delivery(true);
        Check(m_Transport.GetDraft(m_Ticket.ticketId) == "", "success while closed removes only the sent draft");
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(Reopen, 200, false);
    }

    void CheckQueuedReads()
    {
        TicketChatReviewTransport chat = new TicketChatReviewTransport;
        chat.Read("first-ticket");
        chat.Read("second-ticket");
        chat.Read("second-ticket");
        Check(chat.sent.Count() == 1, "different-ticket read waits without duplicate requests");
        chat.History(false);
        Check(chat.sent.Count() == 2 && chat.sent[1].ticketId == "second-ticket", "requested ticket starts immediately when the old read finishes");
        chat.Read("second-ticket");
        chat.History(false);
        Check(chat.sent.Count() == 2, "same-ticket prefetch is reused without another queued read");

        chat.Read("second-ticket");
        chat.Reply("second-ticket", "A queued reply");
        chat.Read("third-ticket");
        chat.History(false);
        Check(chat.sent.Count() == 4 && chat.sent[3].sending, "a queued reply keeps priority over a later ticket read");
        chat.Delivery(true);
        Check(chat.sent.Count() == 5 && chat.sent[4].ticketId == "third-ticket", "next ticket read starts immediately after reply confirmation");
        chat.ReadAfterNotification("third-ticket");
        chat.ReadAfterNotification("third-ticket");
        Check(chat.sent.Count() == 5, "new notifications coalesce while an older same-ticket read is active");
        chat.History(false);
        Check(chat.sent.Count() == 6 && chat.sent[5].ticketId == "third-ticket", "notification refreshes immediately after an older same-ticket read");
        chat.Stop();
    }

    void Reopen()
    {
        OpenConversation();
        Check(m_Menu.Text() == "" && m_Menu.CanWrite(), "reopening does not restore the sent message or lock input");
        m_Transport.History(false);
        m_Menu.Write("Normal reply");
        m_Menu.SendReply();
        m_Transport.Delivery(true);
        m_Menu.RefreshNow();
        Check(m_Menu.Text() == "" && m_Menu.CanWrite(), "ordinary send clears the editor and permits another reply");
        m_Transport.History(false);
        m_Menu.Write("Confirmed just before closing");
        m_Menu.SendReply();
        m_Transport.Delivery(true);
        m_Menu.Close();
        Check(m_Transport.GetDraft(m_Ticket.ticketId) == "", "closing before the next UI update cannot save the confirmed text again");
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ReopenAfterConfirmation, 200, false);
    }

    void ReopenAfterConfirmation()
    {
        OpenConversation();
        Check(m_Menu.Text() == "" && m_Menu.CanWrite(), "confirmed close race reopens with an empty editable composer");
        Print("[TicketChatReview] Finished failures=" + m_Failures);
    }
}

void main() {}
Mission CreateCustomMission(string path) { return new TicketChatReviewMission; }
