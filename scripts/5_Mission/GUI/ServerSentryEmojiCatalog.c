class ServerSentryEmojiChoice
{
    string code;
    string name;
    int category;
    string imageSet;
    string imageName;
    string glyph;
}

/** One catalog for picker images and the composer's native editable emoji slots. */
class ServerSentryEmojiCatalog
{
    protected static ref array<ref ServerSentryEmojiChoice> s_Choices;
    protected static ref map<string, ref ServerSentryEmojiChoice> s_Codes;
    protected static ref map<string, ref ServerSentryEmojiChoice> s_Glyphs;

    static array<ref ServerSentryEmojiChoice> GetChoices()
    {
        if (s_Choices) return s_Choices;
        s_Choices = {};
        s_Codes = new map<string, ref ServerSentryEmojiChoice>;
        s_Glyphs = new map<string, ref ServerSentryEmojiChoice>;
        FileHandle file = OpenFile("ServerSentryDayZMod/gui/emoji/EmojiPicker.tsv", FileMode.READ);
        if (!file)
        {
            Print("[ServerSentry] Emoji catalog could not be opened. Include gui/emoji/EmojiPicker.tsv when packing the mod.");
            return s_Choices;
        }
        // Columns: shortcode, name, category, atlas, sprite, editable glyph.
        string line;
        while (FGets(file, line) > 0)
        {
            array<string> fields = {};
            line.TrimInPlace();
            line.Split("\t", fields);
            if (fields.Count() != 6) continue;
            ServerSentryEmojiChoice choice = new ServerSentryEmojiChoice;
            choice.code = fields[0];
            choice.name = fields[1];
            choice.category = fields[2].ToInt();
            choice.imageSet = "ServerSentryEmoji" + fields[3];
            choice.imageName = fields[4];
            choice.glyph = fields[5];
            s_Choices.Insert(choice);
            s_Codes.Set(choice.code, choice);
            s_Glyphs.Set(choice.glyph, choice);
        }
        CloseFile(file);
        return s_Choices;
    }

    static ServerSentryEmojiChoice FindGlyph(string glyph)
    {
        GetChoices();
        return s_Glyphs.Get(glyph);
    }

    static string Encode(string text)
    {
        GetChoices();
        string result;
        int length = text.LengthUtf8();
        for (int i = 0; i < length; i++)
        {
            string character = text.SubstringUtf8(i, 1);
            if (character == ":")
            {
                int end;
                ServerSentryEmojiChoice choice = ReadCode(text, i, end);
                if (choice)
                {
                    character = choice.glyph;
                    i = end;
                }
            }
            result += character;
        }
        return result;
    }

    protected static ServerSentryEmojiChoice ReadCode(string text, int start, out int end)
    {
        int length = text.LengthUtf8();
        for (end = start + 1; end < length; end++)
        {
            string character = text.SubstringUtf8(end, 1);
            if (character == " " || character == "\n") return null;
            if (character == ":") return s_Codes.Get(text.SubstringUtf8(start, end - start + 1));
        }
        return null;
    }

    static string Decode(string text)
    {
        GetChoices();
        string result;
        int length = text.LengthUtf8();
        for (int i = 0; i < length; i++)
        {
            string character = text.SubstringUtf8(i, 1);
            ServerSentryEmojiChoice choice = s_Glyphs.Get(character);
            if (choice) result += choice.code;
            else result += character;
        }
        return result;
    }
}
