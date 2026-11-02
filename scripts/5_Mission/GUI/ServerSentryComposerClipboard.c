/** DayZ's clipboard API uses Windows ANSI bytes; editor strings are UTF-8.
 * Keep the original text for an in-game round trip, including characters that
 * cannot be represented by the Windows clipboard's legacy text format.
 */
class ServerSentryComposerClipboard
{
    protected static string s_Text;
    protected static string s_NativeText;
    protected static ref array<int> s_Extended = {8364, 129, 8218, 402, 8222, 8230, 8224, 8225, 710, 8240, 352, 8249, 338, 141, 381, 143, 144, 8216, 8217, 8220, 8221, 8226, 8211, 8212, 732, 8482, 353, 8250, 339, 157, 382, 376};

    static void Copy(string text)
    {
        string nativeText;
        for (int i = 0; i < text.LengthUtf8(); i++)
        {
            string character = text.SubstringUtf8(i, 1);
            int code = CodePoint(character);
            int extended = s_Extended.Find(code);
            if (extended != -1) code = extended + 128;
            if (code > 255) code = 63;
            nativeText += code.AsciiToString();
        }
        GetGame().CopyToClipboard(nativeText);
        s_Text = text;
        GetGame().CopyFromClipboard(s_NativeText);
    }

    static string Paste()
    {
        string nativeText;
        GetGame().CopyFromClipboard(nativeText);
        if (nativeText == s_NativeText) return s_Text;
        string text;
        for (int i = 0; i < nativeText.Length(); i++)
        {
            int code = nativeText.Get(i).ToAscii() & 255;
            if (code >= 128 && code < 160) code = s_Extended[code - 128];
            text += Utf8(code);
        }
        return text;
    }

    protected static int CodePoint(string character)
    {
        int first = character.Get(0).ToAscii() & 255;
        if (first < 128) return first;
        int value;
        if (character.Length() == 2) value = first & 31;
        else if (character.Length() == 3) value = first & 15;
        else value = first & 7;
        for (int i = 1; i < character.Length(); i++) value = value * 64 + (character.Get(i).ToAscii() & 63);
        return value;
    }

    protected static string Utf8(int code)
    {
        if (code < 128) return code.AsciiToString();
        if (code < 2048)
        {
            int first = 192 + code / 64;
            int second = 128 + Math.ModFloat(code, 64);
            return first.AsciiToString() + second.AsciiToString();
        }
        int a = 224 + code / 4096;
        int b = 128 + Math.ModFloat(code / 64, 64);
        int c = 128 + Math.ModFloat(code, 64);
        return a.AsciiToString() + b.AsciiToString() + c.AsciiToString();
    }
}
