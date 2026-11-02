class ServerSentryTicketMessage
{
    string messageId;
    string author;
    string text;
    string createdAtUtc;
    bool isOwn;
    ref array<ref ServerSentryTicketTextRun> runs;
}

enum ServerSentryTicketTextStyle
{
    NORMAL = 0,
    BOLD = 1,
    ITALIC = 2,
    UNDERLINE = 4,
    STRIKETHROUGH = 8,
    CODE = 16,
    QUOTE = 32
}

class ServerSentryTicketTextRun
{
    string text;
    int style;
    int size = 16;
    string emojiSet;
    string emojiName;
    string url;
    int spoiler;
}

class ServerSentryTicketConversationRequest
{
    string requestId;
    string ticketId;
    bool sending;
    string message;

    bool IsValid()
    {
        if (!ServerSentryTicketSubmission.IsValidRequestId(requestId) || ticketId == "" || ticketId.Length() > 128)
            return false;
        return !sending || (message.Trim() != "" && message.Length() <= 2000);
    }
}

class ServerSentryTicketConversationFileRequest extends ServerSentryTicketConversationRequest
{
    string steamId64;
}

class ServerSentryTicketConversationResult
{
    string requestId;
    string ticketId;
    bool sending;
    bool success;
    string message;
    bool hasOlderMessages;
    bool canEdit;
    string discordUrl;
    bool hasTicketStatus;
    bool isClaimed;
    bool hasReplyStatus;
    bool isWaitingForPlayer;
    ref array<ref ServerSentryTicketMessage> messages;
}
