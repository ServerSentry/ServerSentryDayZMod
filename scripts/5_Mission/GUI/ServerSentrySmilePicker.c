/** Searchable image picker. Readable shortcodes keep the native editor usable. */
class ServerSentrySmilePicker
{
    protected Widget m_Root;
    protected EditBoxWidget m_Search;
    protected Widget m_Grid;
    protected ScrollWidget m_Scroll;
    protected TextWidget m_Caption;
    protected ServerSentryMessageComposer m_Composer;
    protected ref ServerSentryRoundedSurface m_Surface;
    protected ref array<ref ServerSentryEmojiChoice> m_Choices = {};
    protected ref map<Widget, ref ServerSentryEmojiChoice> m_Buttons = new map<Widget, ref ServerSentryEmojiChoice>;
    protected ref map<Widget, int> m_Categories = new map<Widget, int>;
    protected int m_Category = -1;

    void ServerSentrySmilePicker(Widget parent, ServerSentryMessageComposer composer)
    {
        m_Composer = composer;
        m_Root = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/components/SmilePicker.layout", parent);
        m_Search = EditBoxWidget.Cast(m_Root.FindAnyWidget("EmojiSearch"));
        m_Grid = m_Root.FindAnyWidget("EmojiGrid");
        m_Scroll = ScrollWidget.Cast(m_Root.FindAnyWidget("EmojiScroll"));
        m_Caption = TextWidget.Cast(m_Root.FindAnyWidget("EmojiCaption"));
        m_Surface = new ServerSentryRoundedSurface(m_Root.FindAnyWidget("PickerFill"));
        m_Surface.SetColor(ARGB(255, 12, 17, 30));
        for (int atlas = 0; atlas < 6; atlas++)
            LoadWidgetImageSet("ServerSentryDayZMod/gui/emoji/ServerSentryEmoji" + atlas.ToString() + ".imageset");
        array<string> names = {"All", "Faces", "People", "Nature", "Food", "Travel", "Activities", "Objects", "Symbols", "Flags"};
        Widget tabs = m_Root.FindAnyWidget("EmojiCategories");
        for (int i = 0; i < names.Count(); i++)
        {
            Widget tab = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/components/EmojiCategory.layout", tabs);
            tab.SetPos(Math.ModFloat(i, 5) * 0.20, Math.Floor(i / 5) * 30);
            TextWidget.Cast(tab.FindAnyWidget("CategoryLabel")).SetText(names[i]);
            m_Categories.Set(tab, i - 1);
        }
        m_Choices = ServerSentryEmojiCatalog.GetChoices();
    }

    void Toggle()
    {
        if (m_Composer.GetInput().GetFlags() & WidgetFlags.DISABLED) return;
        m_Root.Show(!m_Root.IsVisible());
        if (m_Root.IsVisible())
        {
            Render();
            SetFocus(m_Search);
        }
        else m_Composer.Focus();
    }

    protected void Render()
    {
        // Build once without relaying out the entire grid after every insertion.
        if (m_Buttons.Count() == 0)
        {
            foreach (ServerSentryEmojiChoice choice : m_Choices)
            {
                Widget button = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/components/EmojiChoice.layout", m_Grid, false);
                ImageWidget.Cast(button.FindAnyWidget("ChoiceImage")).LoadImageFile(0, "set:" + choice.imageSet + " image:" + choice.imageName);
                m_Buttons.Set(button, choice);
            }
        }
        string query = m_Search.GetText();
        query = query.Trim();
        query.ToLower();
        int matches;
        foreach (Widget tile, ServerSentryEmojiChoice entry : m_Buttons)
        {
            string searchable = entry.name + " " + entry.code;
            searchable.ToLower();
            bool visible = (m_Category == -1 || entry.category == m_Category) && (query == "" || searchable.IndexOf(query) != -1);
            tile.Show(visible, false);
            if (visible) matches++;
        }
        foreach (Widget tab, int category : m_Categories)
        {
            int color = ARGB(255, 15, 29, 37);
            if (category == m_Category) color = ARGB(255, 40, 51, 101);
            tab.FindAnyWidget("CategoryFill").SetColor(color);
        }
        m_Caption.SetText(matches.ToString() + " emojis  -  Select one to insert it.");
        if (matches == 0) m_Caption.SetText("No emojis match your search.");
        m_Grid.Update();
        m_Scroll.Update();
        m_Scroll.VScrollToPos01(0);
    }

    bool Dismiss()
    {
        if (!m_Root.IsVisible()) return false;
        m_Root.Show(false);
        m_Composer.Focus();
        return true;
    }

    bool OnChange(Widget widget)
    {
        if (widget != m_Search) return false;
        Render();
        return true;
    }

    bool OnMouseEnter(Widget widget)
    {
        ServerSentryEmojiChoice choice;
        if (!m_Buttons.Find(widget, choice)) return false;
        widget.FindAnyWidget("ChoiceFill").SetColor(ARGB(255, 38, 47, 88));
        m_Caption.SetText(choice.name + "  " + choice.code);
        return true;
    }

    bool OnMouseLeave(Widget widget)
    {
        if (!m_Buttons.Contains(widget)) return false;
        widget.FindAnyWidget("ChoiceFill").SetColor(ARGB(255, 6, 11, 14));
        return true;
    }

    bool OnClick(Widget widget)
    {
        if (!widget || !m_Root.IsVisible()) return false;
        if (widget.GetName() == "EmojiClose")
        {
            m_Root.Show(false);
            m_Composer.Focus();
            return true;
        }
        int category;
        if (m_Categories.Find(widget, category))
        {
            m_Category = category;
            Render();
            return true;
        }
        ServerSentryEmojiChoice choice;
        if (!m_Buttons.Find(widget, choice)) return false;
        if ((m_Composer.GetInput().GetFlags() & WidgetFlags.DISABLED) == 0)
            m_Composer.Append(choice.code);
        m_Root.Show(false);
        return true;
    }
}
