class ServerSentryTicketListResult
{
    bool success;
    string message;
    ref array<ref ServerSentryTicketSummary> tickets;
}

class ServerSentryTicketSummary
{
    string ticketId;
    string subject;
    string ticketTypeName;
    string ticketNumber;
    string createdAtUtc;
    bool isClaimed;

    string GetStatusLabel()
    {
        if (isClaimed)
            return "In progress";
        return "Waiting for support";
    }

    string GetReference()
    {
        if (ticketNumber != "")
            return "#" + ticketNumber;
        return ticketId;
    }

    string GetCreatedLabel()
    {
        if (createdAtUtc.Length() >= 16)
            return createdAtUtc.Substring(0, 10) + " " + createdAtUtc.Substring(11, 5) + " UTC";
        return createdAtUtc;
    }
}
