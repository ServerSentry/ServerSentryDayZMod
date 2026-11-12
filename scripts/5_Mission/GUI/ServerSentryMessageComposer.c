/** Shared editor: native font layout with explicit UTF-8 selection and clipboard handling. */
class ServerSentryMessageComposer extends ScriptedWidgetEventHandler
{
    protected static const int LINE_HEIGHT = 16;
    protected ScrollWidget m_Scroll;
    protected MultilineEditBoxWidget m_Input;
    protected EditBoxWidget m_Keyboard;
    protected UIScriptedMenu m_Menu;
    protected Widget m_Placeholder;
    protected ButtonWidget m_EmojiButton;
    protected ref ServerSentryRoundedSurface m_Surface;
    protected ref ServerSentrySmilePicker m_Smiles;
    protected Widget m_EmojiLayer;
    protected Widget m_SelectionLayer;
    protected Widget m_CaretView;
    protected TextWidget m_Measure;
    protected ref ServerSentryComposerText m_Text = new ServerSentryComposerText;
    protected ref array<ImageWidget> m_EmojiImages = {};
    protected ref array<Widget> m_SelectionViews = {};
    protected ref array<string> m_Lines = {};
    protected ref array<int> m_Offsets = {};
    protected ref map<string, float> m_CharacterWidths = new map<string, float>;
    protected string m_DecodedSource;
    protected string m_DecodedText;
    protected float m_LastWidth;
    protected float m_LastHeight;
    protected float m_ContentHeight;
    protected bool m_Dirty = true;
    protected bool m_Dragging;
    protected bool m_Shift;
    protected bool m_RightControl;
    protected int m_LastCaret = -1;
    protected int m_LastAnchor = -1;
    protected int m_BlinkStart;

    void ServerSentryMessageComposer(Widget parent, UIScriptedMenu menu)
    {
        m_Menu = menu;
        Widget root = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/components/MessageComposer.layout", parent);
        m_Scroll = ScrollWidget.Cast(root.FindAnyWidget("ComposerScroll"));
        m_Input = MultilineEditBoxWidget.Cast(root.FindAnyWidget("ComposerInput"));
        m_Keyboard = EditBoxWidget.Cast(root.FindAnyWidget("ComposerKeyboard"));
        m_Placeholder = root.FindAnyWidget("ComposerPlaceholder");
        m_EmojiButton = ButtonWidget.Cast(root.FindAnyWidget("BtnEmoji"));
        m_EmojiLayer = root.FindAnyWidget("ComposerEmojis");
        m_SelectionLayer = root.FindAnyWidget("ComposerSelectionLayer");
        m_Measure = TextWidget.Cast(root.FindAnyWidget("ComposerMeasure"));
        m_CaretView = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/components/ComposerCaret.layout", m_EmojiLayer);
        m_Surface = new ServerSentryRoundedSurface(root.FindAnyWidget("ComposerFill"));
        m_Surface.SetColor(ARGB(255, 18, 26, 34));
        m_Smiles = new ServerSentrySmilePicker(parent.GetParent(), this);
        m_Keyboard.SetHandler(this);
        m_Scroll.SetHandler(this);
    }

    MultilineEditBoxWidget GetInput() { return m_Input; }
    ButtonWidget GetEmojiButton() { return m_EmojiButton; }
    string GetText()
    {
        if (m_Text.text != m_DecodedSource)
        {
            m_DecodedSource = m_Text.text;
            m_DecodedText = ServerSentryEmojiCatalog.Decode(m_Text.text);
        }
        return m_DecodedText;
    }

    void SetText(string text)
    {
        m_Text.SetText(ServerSentryEmojiCatalog.Encode(text));
        m_Dirty = true;
    }

    void Focus() { SetFocus(m_Keyboard); }
    bool DismissPicker() { return m_Smiles.Dismiss(); }
    bool OnChange(Widget widget) { return m_Smiles.OnChange(widget); }
    bool OnMouseEnter(Widget widget) { return m_Smiles.OnMouseEnter(widget); }
    bool OnMouseLeave(Widget widget) { return m_Smiles.OnMouseLeave(widget); }

    protected bool ControlHeld()
    {
        return GetGame().IsLeftCtrlDown() || m_RightControl;
    }

    override bool OnKeyPress(Widget w, int x, int y, int key)
    {
        // The engine injects clipboard bytes as characters before its V key event.
        // Handle the paste once, using the clipboard string, in OnKeyDown instead.
        return ControlHeld() && !(KeyState(KeyCode.KC_RMENU) & 0x8000);
    }

    override bool OnChange(Widget w, int x, int y, bool finished)
    {
        if (w != m_Keyboard) return false;
        string typed = m_Keyboard.GetText();
        if (typed == "") return true;
        m_Keyboard.SetText("");
        if (!ControlHeld() || (KeyState(KeyCode.KC_RMENU) & 0x8000)) Insert(typed);
        return true;
    }

    override bool OnFocusLost(Widget w, int x, int y)
    {
        m_Dragging = false;
        m_Shift = false;
        m_RightControl = false;
        return false;
    }

    bool OnKeyDown(int key)
    {
        if (key == KeyCode.KC_LSHIFT || key == KeyCode.KC_RSHIFT) m_Shift = true;
        if (key == KeyCode.KC_RCONTROL) m_RightControl = true;
        if (GetFocus() != m_Keyboard) return false;
        if (m_Lines.Count() == 0) return false;
        if (ControlHeld()) return Shortcut(key);
        if (m_Input.GetFlags() & WidgetFlags.DISABLED) return false;
        switch (key)
        {
            case KeyCode.KC_LEFT: m_Text.Move(-1, m_Shift); break;
            case KeyCode.KC_RIGHT: m_Text.Move(1, m_Shift); break;
            case KeyCode.KC_UP: MoveVertical(-1); break;
            case KeyCode.KC_DOWN: MoveVertical(1); break;
            case KeyCode.KC_HOME: m_Text.Select(m_Offsets[LineAt(m_Text.caret)], m_Shift); break;
            case KeyCode.KC_END:
                int line = LineAt(m_Text.caret);
                m_Text.Select(m_Offsets[line] + m_Lines[line].LengthUtf8(), m_Shift);
                break;
            case KeyCode.KC_BACK: m_Text.Erase(true); Changed(); break;
            case KeyCode.KC_DELETE: m_Text.Erase(false); Changed(); break;
            case KeyCode.KC_RETURN: Insert("\n"); break;
            default: return false;
        }
        return true;
    }

    void OnKeyUp(int key)
    {
        if (key == KeyCode.KC_LSHIFT || key == KeyCode.KC_RSHIFT) m_Shift = false;
        if (key == KeyCode.KC_RCONTROL) m_RightControl = false;
    }

    protected bool Shortcut(int key)
    {
        switch (key)
        {
            case KeyCode.KC_A:
                m_Text.anchor = 0;
                m_Text.caret = m_Text.text.LengthUtf8();
                return true;
            case KeyCode.KC_C:
            case KeyCode.KC_X:
                if (m_Text.anchor == m_Text.caret) return true;
                ServerSentryComposerClipboard.Copy(ServerSentryEmojiCatalog.Decode(m_Text.Selection()));
                if (key == KeyCode.KC_X && !(m_Input.GetFlags() & WidgetFlags.DISABLED)) { m_Text.Insert(""); Changed(); }
                return true;
            case KeyCode.KC_V:
                if (!(m_Input.GetFlags() & WidgetFlags.DISABLED)) Insert(ServerSentryComposerClipboard.Paste());
                return true;
            case KeyCode.KC_HOME: m_Text.Select(0, m_Shift); return true;
            case KeyCode.KC_END: m_Text.Select(m_Text.text.LengthUtf8(), m_Shift); return true;
        }
        return false;
    }

    protected void Insert(string text)
    {
        text.Replace("\r\n", "\n");
        m_Text.Insert(ServerSentryEmojiCatalog.Encode(text));
        Changed();
    }

    protected void Changed()
    {
        m_Dirty = true;
        m_Menu.OnChange(m_Input, 0, 0, false);
    }

    bool OnClick(Widget widget)
    {
        if (m_Smiles.OnClick(widget)) return true;
        if (widget != m_EmojiButton) return false;
        m_Smiles.Toggle();
        return true;
    }

    void Append(string emoji)
    {
        Insert(emoji);
        Focus();
    }

    override bool OnMouseButtonDown(Widget w, int x, int y, int button)
    {
        if (w != m_Scroll || button != 0 || (m_Input.GetFlags() & WidgetFlags.DISABLED)) return false;
        Focus();
        m_Dragging = true;
        SelectPoint(x, y, m_Shift);
        return true;
    }

    override bool OnMouseButtonUp(Widget w, int x, int y, int button)
    {
        if (!m_Dragging || button != 0) return false;
        SelectPoint(x, y, true);
        m_Dragging = false;
        return true;
    }

    protected void SelectMouse(bool extend)
    {
        int mouseX, mouseY;
        GetMousePos(mouseX, mouseY);
        SelectPoint(mouseX, mouseY, extend);
    }

    protected void SelectPoint(int mouseX, int mouseY, bool extend)
    {
        if (m_Lines.Count() == 0) return;
        float x, y;
        m_Scroll.GetScreenPos(x, y);
        float height = Math.Max(0, m_ContentHeight - m_LastHeight);
        int line = Math.Clamp(Math.Floor((mouseY - y + m_Scroll.GetVScrollPos01() * height) / LINE_HEIGHT), 0, m_Lines.Count() - 1);
        m_Text.Select(m_Offsets[line] + ColumnAt(line, mouseX - x), extend);
    }

    protected int ColumnAt(int line, float x)
    {
        string prefix = "";
        float previous;
        for (int i = 0; i < m_Lines[line].LengthUtf8(); i++)
        {
            prefix += m_Lines[line].SubstringUtf8(i, 1);
            float width = TextWidth(prefix);
            if (x < (previous + width) * 0.5) return i;
            previous = width;
        }
        return m_Lines[line].LengthUtf8();
    }

    protected int LineAt(int position)
    {
        for (int line = m_Offsets.Count() - 1; line >= 0; line--)
            if (position >= m_Offsets[line]) return line;
        return 0;
    }

    protected void MoveVertical(int direction)
    {
        int line = LineAt(m_Text.caret);
        float x = TextWidth(m_Lines[line].SubstringUtf8(0, m_Text.caret - m_Offsets[line]));
        int next = Math.Clamp(line + direction, 0, m_Lines.Count() - 1);
        m_Text.Select(m_Offsets[next] + ColumnAt(next, x), m_Shift);
    }

    void Update()
    {
        float width, height;
        m_Scroll.GetScreenSize(width, height);
        width -= m_Scroll.GetScrollbarWidth();
        if (width <= 0 || height <= 0) return;
        if (m_Dirty || width != m_LastWidth || height != m_LastHeight)
        {
            LayoutText(width, height);
            m_Dirty = false;
            m_LastWidth = width;
            m_LastHeight = height;
        }
        if (m_Dragging)
        {
            if (GetMouseState(MouseState.LEFT) & 0x80000000) SelectMouse(true);
            else m_Dragging = false;
        }
        if (m_Text.caret != m_LastCaret || m_Text.anchor != m_LastAnchor)
        {
            LayoutSelection();
            m_LastCaret = m_Text.caret;
            m_LastAnchor = m_Text.anchor;
            m_BlinkStart = GetGame().GetTime();
        }
        m_CaretView.Show(GetFocus() == m_Keyboard && Math.ModFloat(GetGame().GetTime() - m_BlinkStart, 1000) < 500);
        bool enabled = (m_Input.GetFlags() & WidgetFlags.DISABLED) == 0;
        if (((m_Keyboard.GetFlags() & WidgetFlags.DISABLED) == 0) != enabled) m_Keyboard.Enable(enabled);
        m_Placeholder.Show(m_Text.text == "");
    }

    protected void LayoutText(float width, float height)
    {
        WrapText(width);
        string display;
        for (int row = 0; row < m_Lines.Count(); row++)
        {
            if (row > 0) display += "\n";
            display += m_Lines[row];
        }
        m_Input.SetText(display);
        m_ContentHeight = Math.Max(height, m_Lines.Count() * LINE_HEIGHT + 4);
        m_Input.SetScreenSize(width, m_ContentHeight);
        m_EmojiLayer.SetScreenSize(width, m_ContentHeight);
        m_SelectionLayer.SetScreenSize(width, m_ContentHeight);
        LayoutEmojis();
        m_Scroll.Update();
        m_LastCaret = -1;
    }

    protected void WrapText(float width)
    {
        m_Lines.Clear();
        m_Offsets.Clear();
        int start, breakAt;
        float used;
        for (int i = 0; i < m_Text.text.LengthUtf8(); i++)
        {
            string character = m_Text.text.SubstringUtf8(i, 1);
            if (character == "\n")
            {
                AddLine(start, i);
                start = i + 1;
                used = 0;
                breakAt = start;
                continue;
            }
            float advance;
            if (!m_CharacterWidths.Find(character, advance))
            {
                advance = TextWidth(character);
                m_CharacterWidths.Set(character, advance);
            }
            if (used + advance > width && i > start)
            {
                int end = i;
                if (breakAt > start) end = breakAt;
                AddLine(start, end);
                start = end;
                i = start - 1;
                used = 0;
                breakAt = start;
                continue;
            }
            used += advance;
            if (character == " ") breakAt = i + 1;
        }
        AddLine(start, m_Text.text.LengthUtf8());
    }

    protected void AddLine(int start, int end)
    {
        m_Offsets.Insert(start);
        m_Lines.Insert(m_Text.text.SubstringUtf8(start, end - start));
    }

    protected void LayoutSelection()
    {
        int used;
        for (int line = 0; line < m_Lines.Count(); line++)
        {
            int start = Math.Max(m_Text.Start(), m_Offsets[line]);
            int end = Math.Min(m_Text.End(), m_Offsets[line] + m_Lines[line].LengthUtf8());
            if (end <= start) continue;
            Widget fill;
            if (used < m_SelectionViews.Count()) fill = m_SelectionViews[used];
            else
            {
                fill = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/components/ComposerSelection.layout", m_SelectionLayer);
                m_SelectionViews.Insert(fill);
            }
            float x = TextWidth(m_Lines[line].SubstringUtf8(0, start - m_Offsets[line]));
            fill.SetPos(x, line * LINE_HEIGHT);
            fill.SetSize(TextWidth(m_Lines[line].SubstringUtf8(0, end - m_Offsets[line])) - x, LINE_HEIGHT);
            fill.Show(true);
            used++;
        }
        for (int unused = used; unused < m_SelectionViews.Count(); unused++) m_SelectionViews[unused].Show(false);
        int caretLine = LineAt(m_Text.caret);
        m_CaretView.SetPos(TextWidth(m_Lines[caretLine].SubstringUtf8(0, m_Text.caret - m_Offsets[caretLine])), caretLine * LINE_HEIGHT);
        float range = Math.Max(0, m_ContentHeight - m_LastHeight);
        float top = m_Scroll.GetVScrollPos01() * range;
        float caretY = caretLine * LINE_HEIGHT;
        if (caretY < top) top = caretY;
        else if (caretY + LINE_HEIGHT > top + m_LastHeight) top = caretY + LINE_HEIGHT - m_LastHeight;
        if (range > 0) m_Scroll.VScrollToPos01(Math.Clamp(top / range, 0, 1));
        else m_Scroll.VScrollToPos01(0);
    }

    protected float TextWidth(string text)
    {
        m_Measure.SetText("a" + text + "a");
        m_Measure.Update();
        float width, height;
        m_Measure.GetScreenSize(width, height);
        m_Measure.SetText("aa");
        m_Measure.Update();
        float padding;
        m_Measure.GetScreenSize(padding, height);
        return Math.Max(0, width - padding);
    }

    protected void LayoutEmojis()
    {
        int used;
        for (int line = 0; line < m_Lines.Count(); line++)
        {
            string prefix = "";
            for (int column = 0; column < m_Lines[line].LengthUtf8(); column++)
            {
                string character = m_Lines[line].SubstringUtf8(column, 1);
                ServerSentryEmojiChoice choice = ServerSentryEmojiCatalog.FindGlyph(character);
                if (choice)
                {
                    ImageWidget image;
                    if (used < m_EmojiImages.Count()) image = m_EmojiImages[used];
                    else
                    {
                        image = ImageWidget.Cast(GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/components/ComposerEmoji.layout", m_EmojiLayer));
                        m_EmojiImages.Insert(image);
                    }
                    image.SetPos(TextWidth(prefix) + 1, line * LINE_HEIGHT);
                    image.LoadImageFile(0, "set:" + choice.imageSet + " image:" + choice.imageName);
                    image.Show(true);
                    used++;
                }
                prefix += character;
            }
        }
        for (int unused = used; unused < m_EmojiImages.Count(); unused++) m_EmojiImages[unused].Show(false);
    }
}
