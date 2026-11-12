/** UTF-8 cursor and selection state. Each displayed emoji occupies one character. */
class ServerSentryComposerText
{
    string text;
    int caret;
    int anchor;

    void SetText(string value)
    {
        text = value;
        Select(text.LengthUtf8(), false);
    }

    void Select(int position, bool extend)
    {
        caret = Math.Clamp(position, 0, text.LengthUtf8());
        if (!extend) anchor = caret;
    }

    int Start() { return Math.Min(anchor, caret); }
    int End() { return Math.Max(anchor, caret); }

    string Selection() { return text.SubstringUtf8(Start(), End() - Start()); }

    void Insert(string value)
    {
        int start = Start();
        text = text.SubstringUtf8(0, start) + value + text.SubstringUtf8(End(), text.LengthUtf8() - End());
        Select(start + value.LengthUtf8(), false);
    }

    void Erase(bool backwards)
    {
        if (anchor == caret)
        {
            if (backwards) anchor = Math.Max(0, caret - 1);
            else anchor = Math.Min(text.LengthUtf8(), caret + 1);
        }
        Insert("");
    }

    void Move(int direction, bool extend)
    {
        if (!extend && anchor != caret)
        {
            if (direction < 0) Select(Start(), false);
            else Select(End(), false);
            return;
        }
        Select(caret + direction, extend);
    }
}
