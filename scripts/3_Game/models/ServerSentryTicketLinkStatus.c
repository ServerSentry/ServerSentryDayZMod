class ServerSentryTicketLinkQuery
{
    int requestId;
    bool includeTickets = true;
    bool refreshOptions;
}

class ServerSentryTicketLinkRequest
{
    string requestId;
    string steamId64;
    bool includeTickets = true;
    bool refreshOptions;
}

class ServerSentryTicketLinkStatus
{
    string requestId;
    bool success;
    bool discordLinked;
    string accountLinkUrl;
    string message;
    int optionsValidForSeconds;
    ref array<ref ServerSentryTicketOption> ticketOptions;
    ref ServerSentryTicketListResult ticketList;
}

class ServerSentryTicketOption
{
    string panelId;
    string ticketTypeId;
    string label;
}
