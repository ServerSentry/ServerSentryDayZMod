class ServerSentryTicketLinkRequest
{
    string requestId;
    string steamId64;
}

class ServerSentryTicketLinkStatus
{
    string requestId;
    bool success;
    bool discordLinked;
    string accountLinkUrl;
    string message;
    ref array<ref ServerSentryTicketOption> ticketOptions;
    ref ServerSentryTicketListResult ticketList;
}

class ServerSentryTicketOption
{
    string panelId;
    string ticketTypeId;
    string label;
}
