class ServerSentryPendingDeathReport
{
    PlayerIdentity identity;
    string killEventId;
    string requestId;
    int deadline;
}

/** Durable command files survive a player's respawn and transport interruptions. */
class ServerSentryDeathReportBridge
{
    protected static const string ROOT = "$profile:ServerSentry/reports";
    protected ref array<ref ServerSentryPendingDeathReport> m_Pending = {};

    void Request(PlayerIdentity identity, string eventId)
    {
        if (!identity || !ServerSentryConfigManager.Get().IsDeathReportingEnabled()) return;
        ServerSentryDeathReportCommand command = ServerSentryDeathReportStore.Find(identity.GetPlainId(), eventId);
        if (!command) return;
        foreach (ServerSentryPendingDeathReport existing : m_Pending)
        {
            if (existing.identity == identity && existing.killEventId == eventId) return;
        }
        if (m_Pending.Count() >= 1024) return;
        ServerSentryPendingDeathReport pending = new ServerSentryPendingDeathReport;
        pending.identity = identity;
        pending.killEventId = eventId;
        int uuid[4];
        UUIDApi.Generate(uuid);
        pending.requestId = UUIDApi.FormatString(uuid);
        pending.deadline = GetGame().GetTime() + 60000;
        if (!Publish(pending, command))
        {
            Reply(pending, false, "The report could not be queued. Please retry.");
            return;
        }
        m_Pending.Insert(pending);
        if (m_Pending.Count() == 1) GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Poll, 500, true);
    }

    protected bool Publish(ServerSentryPendingDeathReport pending, ServerSentryDeathReportCommand command)
    {
        array<string> directories = {"$profile:ServerSentry", ROOT, ROOT + "/requests", ROOT + "/responses"};
        foreach (string directory : directories)
        {
            if (!FileExist(directory) && !MakeDirectory(directory)) return false;
        }
        string path = ROOT + "/requests/" + pending.requestId;
        string error;
        if (!JsonFileLoader<ServerSentryDeathReportCommand>.SaveFile(path + ".json", command, error))
        {
            Print("[ServerSentry][Reports] Cannot queue report: " + error);
            return false;
        }
        FileHandle marker = OpenFile(path + ".ready", FileMode.WRITE);
        if (!marker) return false;
        CloseFile(marker);
        return true;
    }

    protected void Poll()
    {
        for (int index = m_Pending.Count() - 1; index >= 0; index--)
        {
            ServerSentryPendingDeathReport pending = m_Pending[index];
            string path = ROOT + "/responses/" + pending.requestId + ".json";
            if (FileExist(path))
            {
                ServerSentryDeathReportResult result;
                string error;
                if (JsonFileLoader<ServerSentryDeathReportResult>.LoadFile(path, result, error) && result && result.requestId == pending.requestId)
                {
                    Reply(pending, result.accepted, result.message);
                    DeleteFile(path);
                    m_Pending.Remove(index);
                    continue;
                }
            }
            if (GetGame().GetTime() >= pending.deadline)
            {
                Reply(pending, false, "Delivery is taking longer. Retry to check your report.");
                // Leave the durable request with the Agent, including when the player disconnects.
                m_Pending.Remove(index);
            }
        }
        if (m_Pending.Count() == 0) GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Poll);
    }

    protected void Reply(ServerSentryPendingDeathReport pending, bool accepted, string message)
    {
        if (pending.identity)
            GetRPCManager().SendRPC("ServerSentryBot", "DeathReportReceipt", new Param3<string, bool, string>(pending.killEventId, accepted, message), true, pending.identity);
    }

    void Stop()
    {
        if (GetGame()) GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Poll);
        m_Pending.Clear();
    }
    void ~ServerSentryDeathReportBridge() { Stop(); }
}
