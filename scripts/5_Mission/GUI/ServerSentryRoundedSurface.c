/** Native nine-slice skins resize without drawing geometry from script. */
class ServerSentryRoundedSurface
{
    protected static bool s_Loaded;
    protected Widget m_Root;

    void ServerSentryRoundedSurface(Widget parent, string layoutName = "RoundedSurface")
    {
        if (!s_Loaded)
        {
            s_Loaded = LoadWidgetImageSet("ServerSentryDayZMod/gui/skin/Support.imageset");
            LoadWidgetStyles("ServerSentryDayZMod/gui/skin/Support.styles");
        }
        m_Root = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/components/" + layoutName + ".layout", parent);
        m_Root.ClearFlags(WidgetFlags.NOFILTER);
    }

    void SetColor(int color) { m_Root.SetColor(color); }
}
