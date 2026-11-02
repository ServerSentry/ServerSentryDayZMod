class ServerSentryTicketConversationMenu extends ServerSentrySupportMenuBase
{
    static const int MENU_ID = 4771344;
    protected ref ServerSentryTicketSummary m_Ticket;
    protected ServerSentryTicketChat m_Chat;
    protected ScrollWidget m_Scroll;
    protected WrapSpacerWidget m_Messages;
    protected MultilineTextWidget m_Status;
    protected ButtonWidget m_Send;
    protected ButtonWidget m_Refresh;
    protected ref ServerSentryTicketConversationResult m_LastHistory;
    protected ref ServerSentryTicketConversationResult m_LastDelivery;
    protected ref array<ref ServerSentryTicketMessageView> m_MessageViews = {};
    protected float m_RefreshIn;
    protected int m_LayoutFrames;
    protected bool m_StickToBottom;
    protected float m_SavedScroll;
    protected float m_LastListWidth;
    protected string m_SendLabel;
    protected string m_StatusText;

    override Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/TicketConversation.layout");
        if (!layoutRoot) return null;
        m_Chat = ServerSentryClientManager.Get().GetTicketChat();
        m_Scroll = ScrollWidget.Cast(layoutRoot.FindAnyWidget("MessagesScroll"));
        m_Messages = WrapSpacerWidget.Cast(layoutRoot.FindAnyWidget("MessagesList"));
        m_Status = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("ConversationStatus"));
        m_Send = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnSendReply"));
        m_Refresh = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnRefresh"));
        Widget field = layoutRoot.FindAnyWidget("ReplyField");
        m_Composer = new ServerSentryMessageComposer(field, this);
        InitSupportControls();
        return layoutRoot;
    }

    void SetTicket(ServerSentryTicketSummary ticket)
    {
        m_Ticket = ticket;
        TextWidget.Cast(layoutRoot.FindAnyWidget("TitleLabel")).SetText(ticket.ticketTypeName);
        RefreshTicketStatus();
        m_LastDelivery = m_Chat.GetDelivery();
        m_Composer.SetText(m_Chat.GetDraft(ticket.ticketId));
        ServerSentryTicketConversationRequest reply = m_Chat.GetReply();
        if (reply && reply.ticketId == ticket.ticketId && m_Chat.HasUnconfirmedReply() && m_Composer.GetText() == "")
            m_Composer.SetText(reply.message);
        m_RefreshIn = 10;
        m_Chat.Read(ticket.ticketId);
        RefreshChat();
    }

    protected void RefreshTicketStatus()
    {
        string subtitle = m_Ticket.GetStatusLabel();
        if (m_Ticket.ticketNumber != "") subtitle = m_Ticket.GetReference() + "  /  " + subtitle;
        MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("IntroLabel")).SetText(subtitle);
    }

    bool IsTicket(string ticketId) { return m_Ticket && m_Ticket.ticketId == ticketId; }

    void RefreshFromNotification()
    {
        m_RefreshIn = 10;
        m_Chat.ReadAfterNotification(m_Ticket.ticketId);
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);
        if (!m_Ticket) return;
        if (GetUApi().GetInputByID(UAUIBack).LocalPress())
        {
            if (!m_Composer.DismissPicker()) Close();
            return;
        }
        float listWidth, listHeight;
        m_Messages.GetScreenSize(listWidth, listHeight);
        if (listWidth != m_LastListWidth)
        {
            m_LastListWidth = listWidth;
            m_SavedScroll = m_Scroll.GetVScrollPos();
            m_StickToBottom = m_Scroll.GetVScrollPos01() > 0.95;
            m_LayoutFrames = 3;
        }
        if (m_LayoutFrames > 0)
        {
            ResizeMessages();
            m_LayoutFrames--;
        }
        m_RefreshIn -= timeslice;
        if (m_RefreshIn <= 0)
        {
            m_RefreshIn = 10;
            m_Chat.Read(m_Ticket.ticketId);
        }
        RefreshChat();
    }

    protected void RefreshChat()
    {
        if (!m_Ticket) return;
        ServerSentryTicketConversationResult history = m_Chat.GetHistory();
        ServerSentryTicketConversationResult delivery = m_Chat.GetDelivery();
        if (history && history.ticketId == m_Ticket.ticketId && history != m_LastHistory)
        {
            if (history.success && history.hasReplyStatus)
            {
                m_Ticket.isWaitingForPlayer = history.isWaitingForPlayer;
                RefreshTicketStatus();
            }
            if (history.success && !SameMessages(m_LastHistory, history)) RenderMessages(history);
            m_LastHistory = history;
        }
        if (delivery && delivery.ticketId == m_Ticket.ticketId && delivery != m_LastDelivery)
        {
            ApplyDelivery(delivery);
            if (delivery.success)
            {
                m_RefreshIn = 10;
                m_Chat.Read(m_Ticket.ticketId);
            }
        }
        RefreshComposer();
    }

    protected void ApplyDelivery(ServerSentryTicketConversationResult delivery)
    {
        m_LastDelivery = delivery;
        if (delivery.success)
        {
            m_Ticket.isWaitingForPlayer = false;
            RefreshTicketStatus();
        }
        ServerSentryTicketConversationRequest reply = m_Chat.GetReply();
        if (delivery.success && reply && m_Composer.GetText().Trim() == reply.message)
        {
            m_Composer.SetText("");
            m_Chat.SetDraft(m_Ticket.ticketId, "");
        }
    }

    protected void RefreshComposer()
    {
        bool busy = m_Chat.IsSending() || m_Chat.IsReading();
        ServerSentryTicketConversationRequest reply = m_Chat.GetReply();
        ServerSentryTicketConversationResult delivery = m_Chat.GetDelivery();
        bool uncertain = m_Chat.HasUnconfirmedReply();
        bool sameTicket = reply && reply.ticketId == m_Ticket.ticketId;
        // The pending request owns a frozen copy. Players can keep writing a new draft.
        if (m_Composer.GetInput().GetFlags() & WidgetFlags.DISABLED) m_Composer.GetInput().Enable(true);
        string text;
        text = m_Composer.GetText();
        bool valid = text.Trim() != "" && text.Length() <= 2000;
        SetSupportButtonEnabled(m_Send, !m_Chat.IsSending() && ((uncertain && sameTicket) || (!uncertain && valid)));
        SetSupportButtonEnabled(m_Refresh, !busy);
        string label = "Send message";
        if (uncertain) label = "Check delivery";
        if (m_Chat.IsSending()) label = "Sending...";
        if (label != m_SendLabel)
        {
            m_SendLabel = label;
            TextWidget.Cast(layoutRoot.FindAnyWidget("BtnSendReply_Label")).SetText(label);
        }
        string status = text.Length().ToString() + " / 2000";
        if (m_LastHistory && !m_LastHistory.success) status = m_LastHistory.message;
        else if (!m_LastHistory) status = "Loading conversation...";
        else if (m_LastHistory.hasOlderMessages) status = "Showing the latest 100 messages. Older messages are available in Discord.";
        if (delivery && sameTicket) status = delivery.message;
        if (uncertain && !sameTicket) status = "An earlier reply is unconfirmed. Open that ticket to check delivery first.";
        if (m_Chat.IsSending()) status = "Checking message delivery...";
        if (status != m_StatusText)
        {
            m_StatusText = status;
            m_Status.SetText(status);
        }
        ButtonWidget discord = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnOpenDiscord"));
        SetSupportButtonEnabled(discord, m_LastHistory && m_LastHistory.discordUrl != "");
    }

    protected bool SameMessages(ServerSentryTicketConversationResult previous, ServerSentryTicketConversationResult current)
    {
        if (!previous || !previous.success || !previous.messages || !current.messages) return false;
        if (previous.messages.Count() != current.messages.Count()) return false;
        for (int i = 0; i < current.messages.Count(); i++)
        {
            ServerSentryTicketMessage oldMessage = previous.messages[i];
            ServerSentryTicketMessage message = current.messages[i];
            if (oldMessage.messageId != message.messageId || oldMessage.text != message.text || oldMessage.author != message.author || oldMessage.isOwn != message.isOwn || !SameRuns(oldMessage.runs, message.runs))
                return false;
        }
        return true;
    }

    protected bool SameRuns(array<ref ServerSentryTicketTextRun> previous, array<ref ServerSentryTicketTextRun> current)
    {
        if (!previous || !current) return previous == current;
        if (previous.Count() != current.Count()) return false;
        for (int i = 0; i < current.Count(); i++)
        {
            ServerSentryTicketTextRun oldRun = previous[i];
            ServerSentryTicketTextRun run = current[i];
            if (oldRun.text != run.text || oldRun.style != run.style || oldRun.size != run.size || oldRun.url != run.url || oldRun.spoiler != run.spoiler || oldRun.emojiSet != run.emojiSet || oldRun.emojiName != run.emojiName)
                return false;
        }
        return true;
    }

    protected void RenderMessages(ServerSentryTicketConversationResult history)
    {
        m_SavedScroll = m_Scroll.GetVScrollPos();
        m_StickToBottom = !m_LastHistory || m_Scroll.GetVScrollPos01() > 0.95;
        m_MessageViews.Clear();
        Widget child = m_Messages.GetChildren();
        while (child)
        {
            Widget next = child.GetSibling();
            child.Unlink();
            child = next;
        }
        if (history.messages)
        {
            foreach (ServerSentryTicketMessage message : history.messages)
                AddMessage(message);
        }
        m_Messages.Update();
        m_Scroll.Update();
        // Native text measurements become valid after the first layout pass.
        m_LayoutFrames = 3;
    }

    protected void AddMessage(ServerSentryTicketMessage message)
    {
        m_MessageViews.Insert(new ServerSentryTicketMessageView(m_Messages, message));
    }

    protected void ResizeMessages()
    {
        foreach (ServerSentryTicketMessageView view : m_MessageViews) view.Layout(true);
        m_Messages.Update();
        m_Scroll.Update();
        if (m_StickToBottom) m_Scroll.VScrollToPos01(1);
        else m_Scroll.VScrollToPos(m_SavedScroll);
    }
    override void OnHide()
    {
        if (m_Ticket)
        {
            // A confirmation can arrive immediately before closing, before Update handles it.
            ServerSentryTicketConversationResult delivery = m_Chat.GetDelivery();
            if (delivery && delivery.ticketId == m_Ticket.ticketId && delivery != m_LastDelivery)
                ApplyDelivery(delivery);
            string text;
            text = m_Composer.GetText();
            m_Chat.SetDraft(m_Ticket.ticketId, text);
        }
        super.OnHide();
    }

    override bool OnChange(Widget w, int x, int y, bool finished)
    {
        if (m_Ticket && w == m_Composer.GetInput())
        {
            string text;
            text = m_Composer.GetText();
            m_Chat.SetDraft(m_Ticket.ticketId, text);
            RefreshComposer();
        }
        return super.OnChange(w, x, y, finished);
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (!m_Ticket || !w) return super.OnClick(w, x, y, button);
        if (w.GetName() == "BtnOpenDiscord")
        {
            if (m_LastHistory && m_LastHistory.discordUrl.IndexOf("https://discord.com/channels/") == 0)
                GetGame().OpenURL(m_LastHistory.discordUrl);
            return true;
        }
        foreach (ServerSentryTicketMessageView view : m_MessageViews)
        {
            if (view.OnClick(w))
            {
                m_SavedScroll = m_Scroll.GetVScrollPos();
                m_StickToBottom = false;
                m_LayoutFrames = 2;
                return true;
            }
        }
        if (w.GetName() == "BtnClose") { Close(); return true; }
        if (w == m_Refresh) { m_Chat.Read(m_Ticket.ticketId); m_RefreshIn = 10; return true; }
        if (w == m_Send)
        {
            string text;
            text = m_Composer.GetText();
            m_Chat.Reply(m_Ticket.ticketId, text);
            RefreshComposer();
            return true;
        }
        return super.OnClick(w, x, y, button);
    }
}
