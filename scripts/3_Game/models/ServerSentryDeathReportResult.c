/** Agent receipt. Accepted means the CMS durably stored this server's report. */
class ServerSentryDeathReportResult
{
    string requestId;
    bool accepted;
    string message;
}
