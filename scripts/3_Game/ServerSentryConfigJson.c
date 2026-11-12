/**
    \file ServerSentryConfigJson.c
    \brief Merges serialized defaults while retaining existing JSON values and unknown properties.
*/

/** \brief Retains one direct object member without reserializing its value. */
class ServerSentryConfigJsonProperty
{
    /** \brief Decoded property name. */
    string Name;
    /** \brief Original JSON-encoded name. */
    string EncodedName;
    /** \brief Original or replacement JSON value. */
    string Value;
}

/** \brief Edits object members in JSON already validated by the native serializer. */
class ServerSentryConfigJson
{
    /**
        \brief Adds missing defaults recursively without changing existing scalar or array values.
        \param source \p existing JSON object.
        \param defaults \p serialized default object.
        \param updated \p receives the merged object, including unknown properties.
        \param replaceProperty \p optional direct member to replace, used for the root schema version.
        \param indentation \p indentation of this object's closing brace.
        \return \p true when both objects could be merged.
    */
    static bool MergeDefaults(string source, string defaults, out string updated, string replaceProperty = "", string indentation = "")
    {
        updated = source;
        array<ref ServerSentryConfigJsonProperty> members = new array<ref ServerSentryConfigJsonProperty>();
        array<ref ServerSentryConfigJsonProperty> defaultMembers = new array<ref ServerSentryConfigJsonProperty>();
        if (!ReadObject(source, members) || !ReadObject(defaults, defaultMembers))
            return false;

        bool changed = false;
        foreach (ServerSentryConfigJsonProperty defaultMember : defaultMembers)
        {
            ServerSentryConfigJsonProperty member = FindMember(members, defaultMember.Name);
            if (!member)
            {
                members.Insert(defaultMember);
                changed = true;
                continue;
            }

            if (member.Name == replaceProperty)
            {
                if (member.Value != defaultMember.Value)
                    changed = true;

                member.Value = defaultMember.Value;
                continue;
            }

            if (defaultMember.Value.Substring(0, 1) != "{")
                continue;

            if (member.Value == "null")
            {
                member.Value = defaultMember.Value;
                changed = true;
                continue;
            }

            string merged;
            string childIndentation = indentation + "    ";
            if (!MergeDefaults(member.Value, defaultMember.Value, merged, "", childIndentation))
                return false;

            if (member.Value != merged)
                changed = true;

            member.Value = merged;
        }

        if (changed)
            updated = WriteObject(members, indentation);

        return true;
    }

    /**
        \brief Reads direct members without treating nested keys or quoted text as properties.
        \param json \p native-validated JSON object.
        \param members \p receives members in their original order.
        \return \p true when a complete object was read without duplicate keys.
    */
    protected static bool ReadObject(string json, array<ref ServerSentryConfigJsonProperty> members)
    {
        int cursor = SkipWhitespace(json, 0);
        if (cursor >= json.Length() || json.Substring(cursor, 1) != "{")
            return false;

        cursor = SkipWhitespace(json, cursor + 1);
        JsonSerializer serializer = new JsonSerializer();
        while (cursor < json.Length())
        {
            if (json.Substring(cursor, 1) == "}")
                return SkipWhitespace(json, cursor + 1) == json.Length();

            if (json.Substring(cursor, 1) != "\"")
                return false;

            int keyEnd = ValueEnd(json, cursor);
            if (keyEnd < 0)
                return false;

            ServerSentryConfigJsonProperty member = new ServerSentryConfigJsonProperty();
            member.EncodedName = Slice(json, cursor, keyEnd - cursor);
            string name;
            if (!DecodeName(serializer, member.EncodedName, name))
                return false;

            if (FindMember(members, name))
                return false;

            member.Name = name;
            cursor = SkipWhitespace(json, keyEnd);
            if (cursor >= json.Length() || json.Substring(cursor, 1) != ":")
                return false;

            int valueStart = SkipWhitespace(json, cursor + 1);
            int valueEnd = ValueEnd(json, valueStart);
            if (valueEnd <= valueStart)
                return false;

            member.Value = Slice(json, valueStart, valueEnd - valueStart);
            members.Insert(member);
            cursor = SkipWhitespace(json, valueEnd);
            if (cursor >= json.Length())
                return false;

            string separator = json.Substring(cursor, 1);
            if (separator == "}")
                return SkipWhitespace(json, cursor + 1) == json.Length();

            if (separator != ",")
                return false;

            cursor = SkipWhitespace(json, cursor + 1);
        }
        return false;
    }

    /**
        \brief Finds a complete value boundary while skipping escaped strings and nested containers.
        \param json \p native-validated JSON source.
        \param start \p first character of the value.
        \return \p exclusive end offset, or -1 for an incomplete value.
    */
    protected static int ValueEnd(string json, int start)
    {
        int depth = 0;
        bool quoted = false;
        bool escaped = false;
        for (int i = start; i < json.Length(); i++)
        {
            string character = json.Substring(i, 1);
            if (quoted)
            {
                if (escaped)
                    escaped = false;
                else if (character == "\\")
                    escaped = true;
                else if (character == "\"")
                {
                    quoted = false;
                    if (depth == 0)
                        return i + 1;
                }
                continue;
            }

            if (character == "\"")
            {
                quoted = true;
                continue;
            }

            if (character == "{" || character == "[")
            {
                depth++;
                continue;
            }

            if (character == "}" || character == "]")
            {
                if (depth == 0)
                    return i;

                depth--;
                if (depth == 0)
                    return i + 1;

                continue;
            }

            if (depth == 0 && (character == "," || IsWhitespace(character)))
                return i;
        }
        if (quoted || depth != 0)
            return -1;

        return json.Length();
    }

    /**
        \brief Looks up an exact direct member name.
        \param members \p members of one object.
        \param name \p decoded property name.
        \return \p matching member, or null.
    */
    protected static ServerSentryConfigJsonProperty FindMember(array<ref ServerSentryConfigJsonProperty> members, string name)
    {
        foreach (ServerSentryConfigJsonProperty member : members)
        {
            if (member.Name == name)
                return member;
        }
        return null;
    }

    /**
        \brief Writes parsed members directly without copying source-offset gaps.
        \param members \p existing and newly added object members.
        \param indentation \p indentation of the closing brace.
        \return \p complete object with exact encoded names and values.
    */
    protected static string WriteObject(array<ref ServerSentryConfigJsonProperty> members, string indentation)
    {
        string result = "{";
        for (int i = 0; i < members.Count(); i++)
        {
            if (i > 0)
                result += ",";

            ServerSentryConfigJsonProperty member = members[i];
            result += "\n" + indentation + "    " + member.EncodedName + ": ";
            result += member.Value;
        }
        return result + "\n" + indentation + "}";
    }

    /**
        \brief Decodes escaped names through the native JSON array reader.
        \param serializer \p native serializer reused for this object.
        \param encoded \p complete JSON string token.
        \param name \p receives the decoded name.
        \return \p true when the name can be decoded without truncation.
    */
    protected static bool DecodeName(JsonSerializer serializer, string encoded, out string name)
    {
        name = Slice(encoded, 1, encoded.Length() - 2);
        if (name.IndexOf("\\") < 0)
            return true;

        // Config names are short; refuse oversized escaped names instead of risking native truncation.
        if (encoded.Length() > 512)
            return false;

        array<string> decoded = new array<string>();
        string data = "[" + encoded + "]";
        string error;
        if (!serializer.ReadFromString(decoded, data, error) || decoded.Count() != 1)
            return false;

        name = decoded[0];
        return true;
    }

    /**
        \brief Copies long JSON spans in bounded native substring operations.
        \param source \p complete source text.
        \param start \p first byte to copy.
        \param length \p number of bytes to copy.
        \return \p complete span without the native substring length limit.
    */
    protected static string Slice(string source, int start, int length)
    {
        string result;
        while (length > 0)
        {
            int chunkLength = length;
            if (chunkLength > 512)
                chunkLength = 512;

            result += source.Substring(start, chunkLength);
            start += chunkLength;
            length -= chunkLength;
        }
        return result;
    }

    /**
        \brief Advances past JSON whitespace.
        \param json \p source text.
        \param offset \p first offset to inspect.
        \return \p first non-whitespace offset, or the source length.
    */
    protected static int SkipWhitespace(string json, int offset)
    {
        while (offset < json.Length() && IsWhitespace(json.Substring(offset, 1)))
            offset++;

        return offset;
    }

    /**
        \brief Checks the four whitespace characters accepted by JSON.
        \param character \p one source character.
        \return \p true for JSON whitespace.
    */
    protected static bool IsWhitespace(string character)
    {
        return character == " " || character == "\t" || character == "\r" || character == "\n";
    }
}
