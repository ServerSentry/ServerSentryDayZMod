modded class InGameMenu
{
    protected WrapSpacerWidget m_StackPanel;
    protected ButtonWidget m_SupportBtn;

    override Widget Init()
	{
        layoutRoot = super.Init();
        if (layoutRoot)
        {
            if (m_ExitButton)
                m_StackPanel = WrapSpacerWidget.Cast(m_ExitButton.GetParent());
			
            LoadSupportBtn();
        }

        return layoutRoot;
    }

    private void LoadSupportBtn()
    {
        ServerSentryClientManager manager = ServerSentryClientManager.Get();
        if (!manager || !manager.HasSupportFeatures()) return;
        if (!m_StackPanel || !m_ExitButton || m_SupportBtn)
            return;

        m_SupportBtn = ButtonWidget.Cast(GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/IngameSupportButton.layout", m_StackPanel));

        if (m_SupportBtn)
        {
            m_StackPanel.AddChildAfter(m_SupportBtn, m_ExitButton);
        }
    }

    override bool OnClick(Widget w, int x, int y, int button)
	{
        if (m_SupportBtn && w == m_SupportBtn)
        {
            ServerSentryClientManager manager = ServerSentryClientManager.Get();
            if (manager)
                manager.RequestSupportCaseFromServer();
            return true;
        }

        return super.OnClick(w, x, y, button);
    }

    void SetSupportLoading(bool loading)
    {
        if (!m_SupportBtn)
            return;

        m_SupportBtn.Enable(!loading);
        if (loading)
            ButtonSetText(m_SupportBtn, "Loading support...");
        else
            ButtonSetText(m_SupportBtn, "Support");
    }
}
