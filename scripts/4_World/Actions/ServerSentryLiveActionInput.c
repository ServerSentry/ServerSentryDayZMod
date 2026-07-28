/**
	\file ServerSentryLiveActionInput.c
	\brief Implements live-action support for Live action input.
*/

/**
	\brief Stores a normalized RGB color and its channel values.
*/
class ServerSentryRgbColor
{
    /**
    	\brief Red RGB channel in the range 0 to 255.
    */
    int red;
    /**
    	\brief Green RGB channel in the range 0 to 255.
    */
    int green;
    /**
    	\brief Blue RGB channel in the range 0 to 255.
    */
    int blue;
    /**
    	\brief Normalized RGB color encoded as #RRGGBB.
    */
    string hex;

    /**
    	\brief Initializes a new ServerSentryRgbColor instance.
    */
    void ServerSentryRgbColor()
    {
        red = 0;
        green = 0;
        blue = 0;
        hex = "#000000";
    }

    /**
    	\brief Converts the RGB channels to a DayZ ARGB integer.
    	\param alpha \p alpha used by the operation.
    	\return \p The resulting integer value or status code.
    */
    int ToARGB(int alpha = 255)
    {
        if (alpha < 0)
        {
            alpha = 0;
        }
        else if (alpha > 255)
        {
            alpha = 255;
        }

        return (alpha << 24) | (red << 16) | (green << 8) | blue;
    }
}

/**
	\brief Parses primitive and color values from action-input JSON.
*/
class ServerSentryLiveActionInput
{
    /**
    	\brief Returns an integer input value or the supplied fallback.
    	\param json \p json used by the operation.
    	\param key \p stable key used for lookup or serialization.
    	\param fallback \p fallback used by the operation.
    	\return \p The resulting integer value or status code.
    */
    static int GetInt(string json, string key, int fallback = 0)
    {
        string value = GetPrimitiveValue(json, key);
        if (value == "")
        {
            return fallback;
        }

        return value.ToInt();
    }

    /**
    	\brief Returns a floating-point input value or the supplied fallback.
    	\param json \p json used by the operation.
    	\param key \p stable key used for lookup or serialization.
    	\param fallback \p fallback used by the operation.
    	\return \p The resulting floating-point value.
    */
    static float GetFloat(string json, string key, float fallback = 0)
    {
        string value = GetPrimitiveValue(json, key);
        if (value == "")
        {
            return fallback;
        }

        return value.ToFloat();
    }

    /**
    	\brief Returns a string input value or the supplied fallback.
    	\param json \p json used by the operation.
    	\param key \p stable key used for lookup or serialization.
    	\param fallback \p fallback used by the operation.
    	\return \p The resulting string value.
    */
    static string GetString(string json, string key, string fallback = "")
    {
        string value = GetPrimitiveValue(json, key);
        if (value == "")
        {
            return fallback;
        }

        return value;
    }

    /**
    	\brief Returns a boolean input value or the supplied fallback.
    	\param json \p json used by the operation.
    	\param key \p stable key used for lookup or serialization.
    	\param fallback \p true to enable fallback.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool GetBool(string json, string key, bool fallback = false)
    {
        string value = GetPrimitiveValue(json, key);
        value.ToLower();
        if (value == "true" || value == "1")
        {
            return true;
        }

        if (value == "false" || value == "0")
        {
            return false;
        }

        return fallback;
    }

    /**
    	\brief Reads and parses a hexadecimal color from action-input JSON.
    	\param json \p json used by the operation.
    	\param key \p stable key used for lookup or serialization.
    	\param color \p receives the color value used by the operation.
    	\param message \p receives a human-readable result or error message.
    	\param fallback \p fallback used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool TryGetHexColor(string json, string key, out ServerSentryRgbColor color, out string message, string fallback = "")
    {
        return TryParseHexColorValue(GetString(json, key, fallback), color, message);
    }

    /**
    	\brief Parses and normalizes an RGB color encoded as #RRGGBB.
    	\param rawColor \p raw color used by the operation.
    	\param color \p receives the color value used by the operation.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool TryParseHexColorValue(string rawColor, out ServerSentryRgbColor color, out string message)
    {
        color = new ServerSentryRgbColor();
        string normalizedColor;
        if (!TryParseHexColor(rawColor, color.red, color.green, color.blue, normalizedColor, message))
        {
            color = null;
            return false;
        }

        color.hex = normalizedColor;
        return true;
    }

    /**
    	\brief Parses an RGB color and returns its individual channels.
    	\param rawColor \p raw color used by the operation.
    	\param red \p receives the red RGB channel in the range 0 to 255.
    	\param green \p receives the green RGB channel in the range 0 to 255.
    	\param blue \p receives the blue RGB channel in the range 0 to 255.
    	\param normalizedColor \p receives the normalized color used by the operation.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool TryParseHexColor(string rawColor, out int red, out int green, out int blue, out string normalizedColor, out string message)
    {
        red = 0;
        green = 0;
        blue = 0;
        normalizedColor = "";
        message = "";

        string hex = rawColor;
        hex.TrimInPlace();

        if (hex.Length() == 7 && hex.Substring(0, 1) == "#")
        {
            hex = hex.Substring(1, 6);
        }

        if (hex.Length() != 6)
        {
            message = "Invalid color. Expected #RRGGBB, for example #FF0000.";
            return false;
        }

        string upperHex = hex;
        upperHex.ToUpper();

        int r1 = HexNibble(upperHex.Substring(0, 1));
        int r2 = HexNibble(upperHex.Substring(1, 1));
        int g1 = HexNibble(upperHex.Substring(2, 1));
        int g2 = HexNibble(upperHex.Substring(3, 1));
        int b1 = HexNibble(upperHex.Substring(4, 1));
        int b2 = HexNibble(upperHex.Substring(5, 1));

        if (r1 < 0 || r2 < 0 || g1 < 0 || g2 < 0 || b1 < 0 || b2 < 0)
        {
            message = "Invalid hexadecimal color: " + rawColor + ". Expected #RRGGBB.";
            return false;
        }

        red = (r1 * 16) + r2;
        green = (g1 * 16) + g2;
        blue = (b1 * 16) + b2;
        normalizedColor = "#" + upperHex;
        return true;
    }

    /**
    	\brief Converts one hexadecimal character to its numeric value.
    	\param value \p value to process or assign.
    	\return \p The resulting integer value or status code.
    */
    protected static int HexNibble(string value)
    {
        string digits = "0123456789ABCDEF";
        return digits.IndexOf(value);
    }

    /**
    	\brief Reads an unquoted primitive value from a JSON object string.
    	\param json \p json used by the operation.
    	\param key \p stable key used for lookup or serialization.
    	\return \p The resulting string value.
    */
    protected static string GetPrimitiveValue(string json, string key)
    {
        if (json == "" || key == "")
        {
            return "";
        }

        string needle = "\"" + key + "\"";
        int keyIndex = json.IndexOf(needle);
        if (keyIndex < 0)
        {
            return "";
        }

        string tail = json.Substring(keyIndex + needle.Length(), json.Length() - keyIndex - needle.Length());
        int colonIndex = tail.IndexOf(":");
        if (colonIndex < 0)
        {
            return "";
        }

        tail = tail.Substring(colonIndex + 1, tail.Length() - colonIndex - 1);
        tail.TrimInPlace();
        if (tail == "")
        {
            return "";
        }

        if (tail.Substring(0, 1) == "\"")
        {
            string afterQuote = tail.Substring(1, tail.Length() - 1);
            int endQuote = afterQuote.IndexOf("\"");
            if (endQuote < 0)
            {
                return "";
            }

            return afterQuote.Substring(0, endQuote);
        }

        int commaIndex = tail.IndexOf(",");
        int endIndex = commaIndex;
        int braceIndex = tail.IndexOf("}");
        if (endIndex < 0 || (braceIndex >= 0 && braceIndex < endIndex))
        {
            endIndex = braceIndex;
        }

        if (endIndex < 0)
        {
            endIndex = tail.Length();
        }

        string value = tail.Substring(0, endIndex);
        value.TrimInPlace();
        return value;
    }
}
