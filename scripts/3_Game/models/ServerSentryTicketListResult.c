class ServerSentryTicketListResult
{
    bool success;
    string message;
    ref array<ref ServerSentryTicketSummary> tickets;
}

class ServerSentryTicketSummary
{
    string ticketId;
    string ticketTypeName;
    string ticketNumber;
    string createdAtUtc;
    bool isClaimed;
    bool isWaitingForPlayer;

    string GetStatusLabel()
    {
        if (isWaitingForPlayer)
            return "Waiting for you";
        return "Waiting for support";
    }

    string GetReference()
    {
        if (ticketNumber != "")
            return "Ticket " + ticketNumber;
        return ticketId;
    }

    string GetCreatedLabel()
    {
        if (createdAtUtc.Length() >= 16)
            return createdAtUtc.Substring(0, 10) + " " + createdAtUtc.Substring(11, 5) + " UTC";
        return createdAtUtc;
    }
}
