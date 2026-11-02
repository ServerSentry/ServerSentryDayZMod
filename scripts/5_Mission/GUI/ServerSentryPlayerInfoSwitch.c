class ServerSentryPlayerInfoSwitch
{
    protected ButtonWidget m_Button;
    protected Widget m_Thumb;
    protected ref ServerSentryRoundedSurface m_ThumbSurface;
    protected CheckBoxWidget m_Value;
    protected ref ServerSentryRoundedSurface m_Track;

    void ServerSentryPlayerInfoSwitch(Widget parent, CheckBoxWidget value)
    {
        m_Value = value;
        m_Value.Show(false);
        m_Button = ButtonWidget.Cast(GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/components/PlayerInfoSwitch.layout", parent));
        m_Thumb = m_Button.FindAnyWidget("SwitchThumb");
        m_ThumbSurface = new ServerSentryRoundedSurface(m_Thumb, "CircleSurface");
        m_Track = new ServerSentryRoundedSurface(m_Button.FindAnyWidget("SwitchTrack"), "PillSurface");
        Update();
    }

    bool Toggle(Widget widget)
    {
        if (widget != m_Button) return false;
        if ((m_Value.GetFlags() & WidgetFlags.DISABLED) == 0)
            m_Value.SetChecked(!m_Value.IsChecked());
        Update();
        return true;
    }

    void Update()
    {
        bool enabled = (m_Value.GetFlags() & WidgetFlags.DISABLED) == 0;
        m_Button.Enable(enabled);
        m_ThumbSurface.SetColor(ARGB(255, 242, 249, 255));
        m_Thumb.SetPos(3, 3);
        m_Track.SetColor(ARGB(255, 38, 49, 61));
        if (m_Value.IsChecked())
        {
            m_Thumb.SetPos(25, 3);
            m_Track.SetColor(ARGB(255, 65, 104, 240));
        }
        if (!enabled) m_Button.SetAlpha(0.45);
        else m_Button.SetAlpha(1);
    }
}
