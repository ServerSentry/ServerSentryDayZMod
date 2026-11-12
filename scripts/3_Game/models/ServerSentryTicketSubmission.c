class ServerSentryTicketSubmission
{
    static const int MAX_MESSAGE_LENGTH = 4000;

    string requestId;
    string panelId;
    string ticketTypeId;
    string message;
    bool includePlayerInfo;

    static bool IsValidRequestId(string value)
    {
        // UUIDApi.FormatString produces this canonical UUID format.
        if (value.Length() != 36)
            return false;
        value.ToLower();
        string hexadecimal = "0123456789abcdef";
        for (int i = 0; i < value.Length(); i++)
        {
            string character = value.Get(i);
            if (i == 8 || i == 13 || i == 18 || i == 23)
            {
                if (character != "-")
                    return false;
            }
            else if (!hexadecimal.Contains(character))
            {
                return false;
            }
        }
        return value != "00000000-0000-0000-0000-000000000000";
    }

    bool IsValid()
    {
        return IsValidRequestId(requestId) && panelId.Trim() != "" && ticketTypeId.Trim() != "" && message.Trim() != "" && message.Length() <= MAX_MESSAGE_LENGTH;
    }
}

class ServerSentryTicketCreationRequest extends ServerSentryTicketSubmission
{
    string steamId64;
    ref ServerSentryTicketPlayerInfo playerInfo;
}

class ServerSentryTicketPlayerInfo
{
    float x;
    float y;
    float z;
    float health;
    float blood;
    float shock;
    float water;
    float energy;
    int bleedingSources;
    bool unconscious;
}

class ServerSentryTicketCreationResult
{
    string requestId;
    bool success;
    bool canEdit;
    string ticketId;
    string message;
}
