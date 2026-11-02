/** New-ticket form backed by the server/Agent file bridge. */
class ServerSentrySupportDeskMenu extends ServerSentrySupportMenuBase
{
    static const int MENU_ID = 4771342;

    protected ref ServerSentryPlayerInfoSwitch m_InfoSwitch;
    protected Widget m_CloseButton;
    protected ButtonWidget m_SubmitButton;
    protected TextWidget m_SubmitLabel;
    protected ButtonWidget m_LinkAccountButton;
    protected ButtonWidget m_CheckAgainButton;
    protected TextWidget m_CheckAgainLabel;
    protected MultilineTextWidget m_StatusText;
    protected TextWidget m_StatusMark;
    protected CheckBoxWidget m_IncludePlayerInfo;
    protected MultilineEditBoxWidget m_Message;
    protected XComboBoxWidget m_TicketTypes;
    protected ref array<ref ServerSentryTicketOption> m_Options;
    protected ref ServerSentryTicketLinkStatus m_LinkStatus;
    protected ref ServerSentryTicketCreationResult m_CreationResult;
    protected bool m_WasCheckingLink;
    protected bool m_WasCreating;
    protected bool m_HasViewState;
    protected bool m_OpenedCreatedTicket;

    override Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/NewSupportTicket.layout");
        if (!layoutRoot)
            return null;
        m_CloseButton = layoutRoot.FindAnyWidget("BtnClose");
        m_SubmitButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnSubmitTicket"));
        m_SubmitLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("BtnSubmitTicket_Label"));
        m_LinkAccountButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnLinkAccount"));
        m_CheckAgainButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnCheckAgain"));
        m_CheckAgainLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("BtnCheckAgain_Label"));
        m_StatusText = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("TicketStatus"));
        m_StatusMark = TextWidget.Cast(layoutRoot.FindAnyWidget("NoticeMark"));
        m_IncludePlayerInfo = CheckBoxWidget.Cast(layoutRoot.FindAnyWidget("IncludePlayerInfo"));
        m_InfoSwitch = new ServerSentryPlayerInfoSwitch(layoutRoot.FindAnyWidget("PlayerInfoSwitchSlot"), m_IncludePlayerInfo);
        m_Composer = new ServerSentryMessageComposer(layoutRoot.FindAnyWidget("MessageField"), this);
        m_Message = m_Composer.GetInput();
        m_TicketTypes = XComboBoxWidget.Cast(layoutRoot.FindAnyWidget("TicketTypeSelector"));
        SetSupportButtonEnabled(m_SubmitButton, false);
        InitSupportControls();
        return layoutRoot;
    }

    override void OnShow()
    {
        super.OnShow();
        ServerSentryClientManager manager = ServerSentryClientManager.Get();
        if (manager && manager.GetTicketSubmission())
        {
            ServerSentryTicketSubmission draft = manager.GetTicketSubmission();
            m_Composer.SetText(draft.message);
            m_IncludePlayerInfo.SetChecked(draft.includePlayerInfo);
        }
        CheckAccountLink();
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);
        if (IsClosing() || GetGame().GetUIManager().GetMenu() != this) return;
        m_InfoSwitch.Update();
        if (GetUApi().GetInputByID(UAUIBack).LocalPress())
        {
            if (!m_Composer.DismissPicker()) Close();
            return;
        }
        RefreshView();
        if (!m_OpenedCreatedTicket && m_CreationResult && m_CreationResult.success)
            OpenCreatedTicket();
    }

    protected void CheckAccountLink(bool refreshOptions = false)
    {
        ServerSentryClientManager manager = ServerSentryClientManager.Get();
        if (manager)
            manager.RequestTicketLinkStatus(false, refreshOptions);
        m_HasViewState = false;
        RefreshView();
    }

    protected void RefreshView()
    {
        ServerSentryClientManager manager = ServerSentryClientManager.Get();
        if (!manager)
            return;
        bool checking = manager.IsCheckingTicketLink();
        bool creating = manager.IsCreatingTicket();
        ServerSentryTicketLinkStatus status = manager.GetTicketLinkStatus();
        ServerSentryTicketCreationResult result = manager.GetTicketCreationResult();
        if (m_HasViewState && checking == m_WasCheckingLink && creating == m_WasCreating && status == m_LinkStatus && result == m_CreationResult)
            return;

        if (status != m_LinkStatus)
            FillTicketTypes(status);
        m_HasViewState = true;
        m_WasCheckingLink = checking;
        m_WasCreating = creating;
        m_LinkStatus = status;
        m_CreationResult = result;
        m_LinkAccountButton.Show(false);
        m_SubmitButton.Show(true);
        SetSupportButtonEnabled(m_LinkAccountButton, false);
        SetSupportButtonEnabled(m_SubmitButton, false);
        SetSupportButtonEnabled(m_CheckAgainButton, !checking && !creating);
        m_CheckAgainLabel.SetText("Check again");
        m_SubmitLabel.SetText("Send ticket  >");
        bool editable = !manager.GetTicketSubmission() || (result && result.canEdit);
        m_IncludePlayerInfo.Enable(editable);
        m_Message.Enable(editable);
        m_TicketTypes.Enable(editable);

        if (creating)
        {
            m_SubmitLabel.SetText("Sending...");
            SetTicketStatus("Sending your support request...", "...", ARGB(255, 147, 172, 196));
            return;
        }
        if (result && result.success)
        {
            m_SubmitLabel.SetText("Open conversation");
            SetSupportButtonEnabled(m_SubmitButton, true);
            SetSupportButtonEnabled(m_CheckAgainButton, false);
            SetTicketStatus("Ticket created. Your support team has received your request.", "+", ARGB(255, 139, 203, 174));
            return;
        }
        RefreshAccountLink(checking, status);
    }

    protected void RefreshAccountLink(bool checking, ServerSentryTicketLinkStatus status)
    {
        if (checking)
        {
            m_CheckAgainLabel.SetText("Checking...");
            SetTicketStatus("Checking your Discord account link...", "...", ARGB(255, 147, 172, 196));
            return;
        }
        if (!status || !status.success)
        {
            SetTicketStatus("Support is unavailable. Please try again.", "!", ARGB(255, 232, 171, 143));
            return;
        }
        if (!status.discordLinked)
        {
            ShowAccountLink(status);
            return;
        }
        if (m_CreationResult)
        {
            if (m_CreationResult.canEdit)
                SetSupportButtonEnabled(m_SubmitButton, ValidateDraft() == "");
            else
            {
                m_SubmitLabel.SetText("Retry request");
                SetSupportButtonEnabled(m_SubmitButton, true);
            }
            SetTicketStatus(m_CreationResult.message, "!", ARGB(255, 232, 171, 143));
            return;
        }
        if (!m_Options || m_Options.Count() == 0)
        {
            SetTicketStatus("No ticket types are available. Please contact your server team.", "!", ARGB(255, 232, 171, 143));
            return;
        }
        RefreshSendButton();
    }

    protected void ShowAccountLink(ServerSentryTicketLinkStatus status)
    {
        if (status.accountLinkUrl == "")
        {
            SetTicketStatus("Account linking is unavailable. Please try again later.", "!", ARGB(255, 232, 171, 143));
            return;
        }
        m_SubmitButton.Show(false);
        m_LinkAccountButton.Show(true);
        SetSupportButtonEnabled(m_LinkAccountButton, true);
        SetTicketStatus("Link your Discord account to create a ticket.", "i", ARGB(255, 147, 190, 220));
    }

    protected void FillTicketTypes(ServerSentryTicketLinkStatus status)
    {
        ServerSentryTicketOption selected = GetSelectedTicketType();
        m_Options = null;
        m_TicketTypes.ClearAll();
        m_TicketTypes.AddItem("Choose a ticket type");
        m_TicketTypes.SetCurrentItem(0);
        if (!status || !status.ticketOptions)
            return;
        m_Options = status.ticketOptions;
        ServerSentryClientManager manager = ServerSentryClientManager.Get();
        ServerSentryTicketSubmission draft = manager.GetTicketSubmission();
        for (int i = 0; i < m_Options.Count(); i++)
        {
            ServerSentryTicketOption option = m_Options[i];
            m_TicketTypes.AddItem(option.label);
            if (MatchesSelection(option, selected, draft))
                m_TicketTypes.SetCurrentItem(i + 1);
        }
        if (m_Options.Count() == 1)
            m_TicketTypes.SetCurrentItem(1);
    }

    protected bool MatchesSelection(ServerSentryTicketOption option, ServerSentryTicketOption selected, ServerSentryTicketSubmission draft)
    {
        if (draft)
            return option.panelId == draft.panelId && option.ticketTypeId == draft.ticketTypeId;
        return selected && option.panelId == selected.panelId && option.ticketTypeId == selected.ticketTypeId;
    }

    protected ServerSentryTicketOption GetSelectedTicketType()
    {
        int index = m_TicketTypes.GetCurrentItem() - 1;
        if (!m_Options || index < 0 || index >= m_Options.Count())
            return null;
        return m_Options[index];
    }

    protected string ValidateDraft()
    {
        if (!GetSelectedTicketType())
            return "Choose a ticket type.";
        string message;
        message = m_Composer.GetText();
        message = message.Trim();
        if (message == "")
            return "Describe what happened.";
        if (message.Length() > ServerSentryTicketSubmission.MAX_MESSAGE_LENGTH)
            return "The message must not exceed 4000 characters.";
        return "";
    }

    protected void RefreshSendButton()
    {
        if (!m_LinkStatus || !m_LinkStatus.success || !m_LinkStatus.discordLinked || m_WasCheckingLink || m_WasCreating || (m_CreationResult && !m_CreationResult.canEdit))
            return;
        string error = ValidateDraft();
        SetSupportButtonEnabled(m_SubmitButton, error == "");
        if (error == "")
            SetTicketStatus("Ready to send your support request.", "i", ARGB(255, 147, 190, 220));
        else
            SetTicketStatus(error, "i", ARGB(255, 147, 190, 220));
    }

    protected void SendTicket()
    {
        ServerSentryClientManager manager = ServerSentryClientManager.Get();
        if (manager && m_CreationResult && m_CreationResult.success)
        {
            OpenCreatedTicket();
            return;
        }
        if (!manager || !m_LinkStatus || !m_LinkStatus.success || !m_LinkStatus.discordLinked || m_WasCheckingLink)
            return;
        if (!manager.GetTicketSubmission() || (m_CreationResult && m_CreationResult.canEdit))
        {
            string error = ValidateDraft();
            if (error != "")
            {
                SetTicketStatus(error, "!", ARGB(255, 232, 171, 143));
                return;
            }
        }
        string message;
        message = m_Composer.GetText();
        manager.SubmitTicket(GetSelectedTicketType(), message, m_IncludePlayerInfo.IsChecked());
        m_HasViewState = false;
        RefreshView();
    }

    protected void OpenCreatedTicket()
    {
        if (!m_CreationResult || !m_CreationResult.success || m_CreationResult.ticketId == "") return;
        ServerSentryTicketSummary ticket = new ServerSentryTicketSummary;
        ticket.ticketId = m_CreationResult.ticketId;
        ServerSentryTicketOption option = GetSelectedTicketType();
        ticket.ticketTypeName = "Support";
        if (option) ticket.ticketTypeName = option.label;
        ServerSentryTicketConversationMenu menu = ServerSentryTicketConversationMenu.Cast(GetGame().GetUIManager().EnterScriptedMenu(ServerSentryTicketConversationMenu.MENU_ID, this));
        if (!menu) return;
        menu.SetTicket(ticket);
        // Returning to the completed form must not reopen the conversation in a loop.
        m_OpenedCreatedTicket = true;
    }

    protected void SetTicketStatus(string text, string mark, int color)
    {
        m_StatusText.SetText(text);
        m_StatusText.SetColor(color);
        m_StatusMark.SetText(mark);
        m_StatusMark.SetColor(color);
    }

    override bool OnChange(Widget w, int x, int y, bool finished)
    {
        if (w == m_Message || w == m_TicketTypes)
            RefreshSendButton();
        return super.OnChange(w, x, y, finished);
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (m_InfoSwitch.Toggle(w)) return true;
        if (w && w.GetName() == "PlayerInfoLabel")
        {
            if ((m_IncludePlayerInfo.GetFlags() & WidgetFlags.DISABLED) == 0)
                m_IncludePlayerInfo.SetChecked(!m_IncludePlayerInfo.IsChecked());
            return true;
        }
        if (m_CloseButton && w == m_CloseButton)
        {
            Close();
            return true;
        }
        if (w == m_CheckAgainButton)
        {
            CheckAccountLink(true);
            return true;
        }
        if (w == m_SubmitButton)
        {
            SendTicket();
            return true;
        }
        if (w == m_LinkAccountButton)
        {
            if (m_LinkStatus && m_LinkStatus.success && !m_LinkStatus.discordLinked && m_LinkStatus.accountLinkUrl != "")
            {
                GetGame().OpenURL(m_LinkStatus.accountLinkUrl);
                SetTicketStatus("Link Discord on the website, then select Check again.", "i", ARGB(255, 147, 190, 220));
            }
            return true;
        }
        return super.OnClick(w, x, y, button);
    }
}
