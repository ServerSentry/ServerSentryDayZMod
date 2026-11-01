/** Own open tickets and their summary; conversation transport is separate. */
class ServerSentryTicketsMenu extends ServerSentrySupportMenuBase
{
    static const int MENU_ID = 4771343;

    protected ButtonWidget m_BackButton;
    protected ButtonWidget m_RefreshButton;
    protected ButtonWidget m_NewButton;
    protected ButtonWidget m_LinkButton;
    protected TextWidget m_NewLabel;
    protected TextWidget m_Title;
    protected MultilineTextWidget m_Intro;
    protected MultilineTextWidget m_StatusText;
    protected Widget m_Scroll;
    protected WrapSpacerWidget m_List;
    protected Widget m_Detail;
    protected MultilineTextWidget m_DetailSubject;
    protected MultilineTextWidget m_DetailInfo;
    protected ref map<Widget, ref ServerSentryTicketSummary> m_Rows = new map<Widget, ref ServerSentryTicketSummary>;
    protected ref ServerSentryTicketLinkStatus m_Status;
    protected ref ServerSentryTicketCreationResult m_CreationResult;
    protected ref ServerSentryTicketSummary m_SelectedTicket;
    protected string m_SelectedId;
    protected bool m_WasChecking;
    protected bool m_WasCreating;
    protected bool m_HasState;

    override Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/TicketsMenu.layout");
        if (!layoutRoot)
            return null;
        m_BackButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnClose"));
        m_RefreshButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnRefresh"));
        m_NewButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnNewTicket"));
        m_LinkButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnLinkAccount"));
        m_NewLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("BtnNewTicket_Label"));
        m_Title = TextWidget.Cast(layoutRoot.FindAnyWidget("TitleLabel"));
        m_Intro = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("IntroLabel"));
        m_StatusText = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("TicketListStatus"));
        m_Scroll = layoutRoot.FindAnyWidget("TicketsScroll");
        m_List = WrapSpacerWidget.Cast(layoutRoot.FindAnyWidget("TicketsList"));
        m_Detail = layoutRoot.FindAnyWidget("TicketDetail");
        m_DetailSubject = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("DetailSubject"));
        m_DetailInfo = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("DetailInfo"));
        ShowList();
        SetSupportButtonEnabled(m_NewButton, false);
        return layoutRoot;
    }

    override void OnShow()
    {
        super.OnShow();
        RefreshTickets();
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);
        if (GetUApi().GetInputByID(UAUIBack).LocalPress())
        {
            GoBack();
            return;
        }
        RefreshView();
    }

    protected void RefreshTickets()
    {
        ServerSentryClientManager manager = ServerSentryClientManager.Get();
        if (manager)
            manager.RequestTicketLinkStatus();
        m_HasState = false;
        RefreshView();
    }

    protected void RefreshView()
    {
        ServerSentryClientManager manager = ServerSentryClientManager.Get();
        if (!manager)
            return;
        ServerSentryTicketLinkStatus status = manager.GetTicketLinkStatus();
        ServerSentryTicketCreationResult creation = manager.GetTicketCreationResult();
        bool checking = manager.IsCheckingTicketLink();
        bool creating = manager.IsCreatingTicket();
        if (m_HasState && status == m_Status && checking == m_WasChecking && creating == m_WasCreating && creation == m_CreationResult)
            return;
        m_HasState = true;
        m_Status = status;
        m_WasChecking = checking;
        m_WasCreating = creating;
        m_CreationResult = creation;
        bool resume = manager.GetTicketSubmission() && (!creation || !creation.success);
        m_NewButton.Show(true);
        m_LinkButton.Show(false);
        m_NewLabel.SetText("New ticket  >");
        if (resume)
            m_NewLabel.SetText("Resume request");
        SetSupportButtonEnabled(m_NewButton, resume);
        SetSupportButtonEnabled(m_RefreshButton, !checking);
        m_Scroll.Enable(!checking);
        if (checking)
        {
            m_StatusText.SetText("Loading your tickets...");
            return;
        }
        RenderStatus(status, resume);
    }

    protected void RenderStatus(ServerSentryTicketLinkStatus status, bool resume)
    {
        if (!status || !status.success)
        {
            ClearTickets();
            ShowList();
            m_StatusText.SetText("Support is unavailable. Please refresh and try again.");
            return;
        }
        if (!status.discordLinked)
        {
            ClearTickets();
            ShowList();
            if (!resume && status.accountLinkUrl != "")
            {
                m_NewButton.Show(false);
                m_LinkButton.Show(true);
                SetSupportButtonEnabled(m_LinkButton, true);
            }
            m_StatusText.SetText("Link your Discord account to view or create tickets, then select Refresh.");
            return;
        }
        bool canCreate = status.ticketOptions && status.ticketOptions.Count() > 0;
        SetSupportButtonEnabled(m_NewButton, resume || canCreate);
        if (!status.ticketList || !status.ticketList.success)
        {
            ClearTickets();
            ShowList();
            string message = "Tickets could not be loaded. Please refresh.";
            if (status.ticketList && status.ticketList.message != "")
                message = status.ticketList.message;
            m_StatusText.SetText(message);
            return;
        }
        RenderTickets(status.ticketList.tickets);
    }

    protected void ClearTickets()
    {
        m_Rows.Clear();
        Widget row = m_List.GetChildren();
        while (row)
        {
            Widget next = row.GetSibling();
            row.Unlink();
            row = next;
        }
    }

    protected void RenderTickets(array<ref ServerSentryTicketSummary> tickets)
    {
        ClearTickets();
        ServerSentryTicketSummary selected;
        if (tickets)
        {
            foreach (ServerSentryTicketSummary ticket : tickets)
            {
                if (!ticket)
                    continue;
                AddTicketRow(ticket);
                if (ticket.ticketId == m_SelectedId)
                    selected = ticket;
            }
        }
        if (selected)
        {
            OpenTicket(selected);
            return;
        }
        bool selectionClosed = m_SelectedId != "";
        ShowList();
        if (selectionClosed)
            m_StatusText.SetText("This ticket is no longer open.");
        else if (m_Rows.Count() == 0)
            m_StatusText.SetText("You have no open tickets.");
        else
        {
            string label = " open tickets. Select a ticket to view its details.";
            if (m_Rows.Count() == 1)
                label = " open ticket. Select it to view its details.";
            m_StatusText.SetText(m_Rows.Count().ToString() + label);
        }
    }

    protected void AddTicketRow(ServerSentryTicketSummary ticket)
    {
        Widget row = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/TicketRow.layout", m_List);
        if (!row)
            return;
        TextWidget subject = TextWidget.Cast(row.FindAnyWidget("TicketRow_Label"));
        TextWidget info = TextWidget.Cast(row.FindAnyWidget("TicketRow_Meta"));
        subject.SetText(ticket.subject);
        info.SetText(ticket.GetReference() + " / " + ticket.ticketTypeName + " / " + ticket.GetStatusLabel());
        m_Rows.Set(row, ticket);
    }

    protected void OpenTicket(ServerSentryTicketSummary ticket)
    {
        m_SelectedId = ticket.ticketId;
        m_SelectedTicket = ticket;
        m_Scroll.Show(false);
        m_Detail.Show(true);
        m_Title.SetText("Ticket details");
        m_Intro.SetText("Your active support request.");
        m_DetailSubject.SetText(ticket.subject);
        m_DetailInfo.SetText("Ticket: " + ticket.GetReference() + "\nID: " + ticket.ticketId + "\nType: " + ticket.ticketTypeName + "\nStatus: " + ticket.GetStatusLabel() + "\nCreated: " + ticket.GetCreatedLabel());
        m_StatusText.SetText("Use Back to return to your tickets.");
    }

    protected void ShowList()
    {
        m_SelectedId = "";
        m_SelectedTicket = null;
        m_Scroll.Show(true);
        m_Detail.Show(false);
        m_Title.SetText("Your support tickets");
        m_Intro.SetText("View your open requests or contact your server team.");
    }

    protected void GoBack()
    {
        if (m_SelectedTicket)
        {
            ShowList();
            m_HasState = false;
            RefreshView();
            return;
        }
        Close();
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (w == m_BackButton)
        {
            GoBack();
            return true;
        }
        if (w == m_RefreshButton)
        {
            RefreshTickets();
            return true;
        }
        if (w == m_NewButton)
        {
            ServerSentryClientManager manager = ServerSentryClientManager.Get();
            if (manager)
                manager.StartNewTicket();
            m_SelectedId = "";
            GetGame().GetUIManager().EnterScriptedMenu(ServerSentrySupportDeskMenu.MENU_ID, this);
            return true;
        }
        if (w == m_LinkButton)
        {
            if (m_Status && m_Status.success && !m_Status.discordLinked && m_Status.accountLinkUrl != "")
                GetGame().OpenURL(m_Status.accountLinkUrl);
            return true;
        }
        ServerSentryTicketSummary ticket;
        if (!m_WasChecking && m_Rows.Find(w, ticket))
        {
            OpenTicket(ticket);
            return true;
        }
        return super.OnClick(w, x, y, button);
    }
}
