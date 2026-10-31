/** New-ticket form, shown only after the server reports no support case. */
class ServerSentrySupportDeskMenu extends ServerSentrySupportMenuBase
{
    static const int MENU_ID = 4771342;

    protected Widget m_CloseButton;
    protected ButtonWidget m_SubmitButton;

    override Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/NewSupportTicket.layout");
        if (layoutRoot)
        {
            m_CloseButton = layoutRoot.FindAnyWidget("BtnClose");
            m_SubmitButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnSubmitTicket"));
            // Enabled when the actual ServerSentry ticket contract is connected.
            SetSupportButtonEnabled(m_SubmitButton, false);
        }

        return layoutRoot;
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);

        if (GetUApi().GetInputByID(UAUIBack).LocalPress())
            Close();
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (m_CloseButton && w == m_CloseButton)
        {
            Close();
            return true;
        }

        return super.OnClick(w, x, y, button);
    }
}
