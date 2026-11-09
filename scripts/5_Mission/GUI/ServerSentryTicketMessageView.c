/** Draws structured Discord text without passing message content to the engine's markup parser. */
class ServerSentryTicketMessageView
{
    protected static bool s_ImagesLoaded;
    protected Widget m_Row;
    protected Widget m_Body;
    protected Widget m_MeasureRoot;
    protected ref ServerSentryTicketMessage m_Message;
    protected ref array<Widget> m_Spans = {};
    protected ref map<Widget, string> m_Links = new map<Widget, string>;
    protected ref map<Widget, int> m_Spoilers = new map<Widget, int>;
    protected ref set<int> m_Revealed = new set<int>;
    protected Widget m_CurrentSpan;
    protected TextWidget m_CurrentText;
    protected ServerSentryTicketTextRun m_CurrentFormat;
    protected string m_CurrentValue;
    protected float m_CurrentWidth;
    protected float m_X;
    protected float m_Y;
    protected float m_LineHeight;
    protected float m_Width;
    protected float m_LastWidth = -1;
    protected bool m_Dirty = true;

    void ServerSentryTicketMessageView(Widget parent, ServerSentryTicketMessage message)
    {
        if (!s_ImagesLoaded)
        {
            for (int atlas = 0; atlas < 6; atlas++)
                LoadWidgetImageSet("ServerSentryDayZMod/gui/emoji/ServerSentryEmoji" + atlas.ToString() + ".imageset");
            s_ImagesLoaded = true;
        }
        m_Row = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/components/TicketMessage.layout", parent, false);
        m_Body = m_Row.FindAnyWidget("MessageBody");
        m_MeasureRoot = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/components/TicketTextMeasure.layout", m_Body, false);
        m_MeasureRoot.SetAlpha(0);
        UpdateMessage(message);
    }

    void ~ServerSentryTicketMessageView()
    {
        Dispose();
    }

    Widget GetRow() { return m_Row; }
    int GetSpanCount() { return m_Spans.Count(); }
    string GetMessageId()
    {
        if (!m_Message) return "";
        return m_Message.messageId;
    }

    /** A changed spoiler group must be hidden again; unchanged messages keep their reveal state. */
    bool UpdateMessage(ServerSentryTicketMessage message)
    {
        if (!message || !m_Row) return false;
        bool sameContent = m_Message && m_Message.text == message.text && SameRuns(m_Message.runs, message.runs);
        if (sameContent && m_Message.messageId == message.messageId && m_Message.author == message.author && m_Message.createdAtUtc == message.createdAtUtc && m_Message.isOwn == message.isOwn)
        {
            m_Message = message;
            return false;
        }
        m_Message = message;
        string time = ServerSentryTicketTime.LocalDisplay(message.createdAtUtc);
        TextWidget.Cast(m_Row.FindAnyWidget("MessageAuthor")).SetText(message.author + "  /  " + time);
        Widget bubble = m_Row.FindAnyWidget("Bubble");
        bubble.SetColor(ARGB(255, 10, 15, 26));
        if (message.isOwn) bubble.SetColor(ARGB(255, 17, 22, 45));
        if (!sameContent)
        {
            m_Revealed.Clear();
            m_Dirty = true;
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
            if (!oldRun || !run)
            {
                if (oldRun != run) return false;
                continue;
            }
            if (oldRun.text != run.text || oldRun.style != run.style || oldRun.size != run.size || oldRun.url != run.url || oldRun.spoiler != run.spoiler || oldRun.emojiSet != run.emojiSet || oldRun.emojiName != run.emojiName)
                return false;
        }
        return true;
    }

    /** Release the complete row, including measurement widgets and message data. */
    void Dispose()
    {
        if (m_Row) m_Row.Unlink();
        m_Row = null;
        m_Body = null;
        m_MeasureRoot = null;
        m_Message = null;
        m_Spans.Clear();
        m_Links.Clear();
        m_Spoilers.Clear();
        m_Revealed.Clear();
        m_CurrentSpan = null;
        m_CurrentText = null;
        m_CurrentFormat = null;
        m_CurrentValue = "";
    }

    protected void ConfigureText(TextWidget text, ServerSentryTicketTextRun format)
    {
        text.SetTextExactSize(Math.Clamp(format.size, 12, 28));
        text.SetBold((format.style & ServerSentryTicketTextStyle.BOLD) != 0);
        text.SetItalic((format.style & ServerSentryTicketTextStyle.ITALIC) != 0);
    }

    protected float Measure(ServerSentryTicketTextRun format)
    {
        string name = "MeasureText";
        if (format.style & ServerSentryTicketTextStyle.CODE) name = "MeasureCode";
        TextWidget text = TextWidget.Cast(m_MeasureRoot.FindAnyWidget(name));
        ConfigureText(text, format);
        text.SetText("a" + format.text + "a");
        text.Update();
        float width, height;
        text.GetScreenSize(width, height);
        text.SetText("aa");
        text.Update();
        float padding;
        text.GetScreenSize(padding, height);
        // Measuring between visible letters preserves whitespace widths.
        return Math.Max(0, width - padding);
    }

    protected void ClearSpans()
    {
        foreach (Widget span : m_Spans) span.Unlink();
        m_Spans.Clear();
        m_Links.Clear();
        m_Spoilers.Clear();
        m_CurrentSpan = null;
        m_CurrentText = null;
        m_CurrentFormat = null;
        m_CurrentValue = "";
    }

    void Layout(bool force = false)
    {
        if (!m_Row || !m_Message) return;
        float width, height;
        m_Row.GetScreenSize(width, height);
        if (!force && !m_Dirty && width == m_LastWidth) return;
        m_LastWidth = width;
        m_Dirty = false;
        ClearSpans();
        m_Width = Math.Max(100, width * 0.95);
        m_X = 0;
        m_Y = 0;
        m_LineHeight = 24;
        set<int> hidden = new set<int>;
        if (m_Message.runs && m_Message.runs.Count() > 0)
        {
            foreach (ServerSentryTicketTextRun run : m_Message.runs)
                AppendVisibleRun(run, hidden);
        }
        else
        {
            AppendFallback(m_Message.text);
        }
        m_Body.SetSize(0.95, m_Y + m_LineHeight);
        m_Row.SetSize(1, m_Y + m_LineHeight + 52);
    }

    protected void AppendFallback(string text)
    {
        string part;
        for (int i = 0; i < text.LengthUtf8(); i++)
        {
            string character = text.SubstringUtf8(i, 1);
            if (character == "\n")
            {
                AppendFallbackPart(part);
                part = "";
                NewLine();
                continue;
            }
            part += character;
            if (character == " " || part.LengthUtf8() >= 24)
            {
                AppendFallbackPart(part);
                part = "";
            }
        }
        AppendFallbackPart(part);
    }

    protected void AppendFallbackPart(string text)
    {
        if (text == "") return;
        ServerSentryTicketTextRun fallback = new ServerSentryTicketTextRun;
        fallback.text = text;
        AppendRun(fallback);
    }

    protected void AppendVisibleRun(ServerSentryTicketTextRun run, set<int> hidden)
    {
        if (run.spoiler > 0 && m_Revealed.Find(run.spoiler) == -1)
        {
            if (hidden.Find(run.spoiler) != -1) return;
            hidden.Insert(run.spoiler);
            ServerSentryTicketTextRun reveal = new ServerSentryTicketTextRun;
            reveal.text = "[Show spoiler]";
            reveal.spoiler = run.spoiler;
            AppendRun(reveal);
            m_CurrentSpan = null;
            return;
        }
        AppendRun(run);
    }

    protected void NewLine()
    {
        m_X = 0;
        m_Y += m_LineHeight;
        m_LineHeight = 24;
        m_CurrentSpan = null;
    }

    protected bool SameFormat(ServerSentryTicketTextRun format)
    {
        return m_CurrentFormat && m_CurrentFormat.style == format.style && m_CurrentFormat.size == format.size && m_CurrentFormat.url == format.url && m_CurrentFormat.spoiler == format.spoiler;
    }

    protected void AppendRun(ServerSentryTicketTextRun run)
    {
        if (run.text == "\n") { NewLine(); return; }
        bool emoji = run.emojiSet != "" && run.emojiName != "";
        float width = Math.Clamp(run.size + 10, 26, 38);
        if (!emoji) width = Measure(run);
        if (width <= 0) return;
        if (m_X > 0 && m_X + width > m_Width) NewLine();
        if (!emoji && m_CurrentSpan && SameFormat(run))
        {
            m_CurrentValue += run.text;
            m_CurrentText.SetText(m_CurrentValue);
            m_CurrentWidth += width;
            m_CurrentSpan.SetSize(m_CurrentWidth, m_LineHeight);
            m_X += width;
            return;
        }
        width = CreateSpan(run, width, emoji);
        m_X += width;
        if (emoji) m_CurrentSpan = null;
    }

    protected float CreateSpan(ServerSentryTicketTextRun run, float width, bool emoji)
    {
        Widget span = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/components/TicketTextSpan.layout", m_Body, false);
        m_Spans.Insert(span);
        float height = Math.Max(24, run.size + 8);
        m_LineHeight = Math.Max(m_LineHeight, height);
        span.SetPos(m_X, m_Y);
        span.SetSize(width, height);
        span.FindAnyWidget("CodeFill").Show((run.style & ServerSentryTicketTextStyle.CODE) != 0);
        span.FindAnyWidget("Underline").Show((run.style & ServerSentryTicketTextStyle.UNDERLINE) != 0 || run.url != "");
        span.FindAnyWidget("Underline").SetPos(0, height - 5);
        span.FindAnyWidget("Strikethrough").Show((run.style & ServerSentryTicketTextStyle.STRIKETHROUGH) != 0);
        span.FindAnyWidget("Strikethrough").SetPos(0, height * 0.5);
        TextWidget normal = TextWidget.Cast(span.FindAnyWidget("SpanText"));
        TextWidget code = TextWidget.Cast(span.FindAnyWidget("CodeText"));
        bool isCode = (run.style & ServerSentryTicketTextStyle.CODE) != 0;
        normal.Show(!isCode && !emoji);
        code.Show(isCode && !emoji);
        TextWidget text = normal;
        if (isCode) text = code;
        ConfigureText(text, run);
        text.SetText(run.text);
        if (run.style & ServerSentryTicketTextStyle.QUOTE) text.SetColor(ARGB(255, 157, 174, 191));
        if (run.url != "" || run.spoiler > 0) text.SetColor(ARGB(255, 89, 190, 230));
        ImageWidget image = ImageWidget.Cast(span.FindAnyWidget("EmojiImage"));
        float imageSize = Math.Clamp(run.size + 8, 24, 36);
        image.SetSize(imageSize, imageSize);
        image.SetPos(0, (height - imageSize) * 0.5);
        image.Show(emoji);
        if (emoji && !image.LoadImageFile(0, "set:" + run.emojiSet + " image:" + run.emojiName))
        {
            image.Show(false);
            normal.Show(true);
            normal.SetText(run.text);
            width = Math.Max(width, Measure(run));
            if (m_X > 0 && m_X + width > m_Width) NewLine();
            m_LineHeight = Math.Max(m_LineHeight, height);
            span.SetPos(m_X, m_Y);
            span.SetSize(width, height);
        }
        if (run.spoiler > 0) m_Spoilers.Set(span, run.spoiler);
        else if (run.url != "") m_Links.Set(span, run.url);
        else span.SetFlags(WidgetFlags.IGNOREPOINTER);
        m_CurrentSpan = span;
        m_CurrentText = text;
        m_CurrentValue = run.text;
        m_CurrentWidth = width;
        m_CurrentFormat = run;
        return width;
    }

    bool OnClick(Widget widget)
    {
        int spoiler;
        if (m_Spoilers.Find(widget, spoiler))
        {
            int index = m_Revealed.Find(spoiler);
            if (index == -1) m_Revealed.Insert(spoiler);
            else m_Revealed.Remove(index);
            m_Dirty = true;
            return true;
        }
        string url;
        if (m_Links.Find(widget, url))
        {
            if (url.IndexOf("https://") == 0 || url.IndexOf("http://") == 0) GetGame().OpenURL(url);
            return true;
        }
        return false;
    }
}
