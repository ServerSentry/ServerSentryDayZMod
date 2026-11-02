/** Actual client request/cache logic, with the RPC boundary captured locally. */
class TicketOptionsReviewClient extends ServerSentryClientManager
{
    ref array<ref ServerSentryTicketLinkQuery> queries = {};

    override protected void SendTicketLinkQuery(ServerSentryTicketLinkQuery query) { queries.Insert(query); }

    void Seed(int validForSeconds, bool linked = true, bool success = true)
    {
        m_TicketsEnabled = true;
        ServerSentryTicketLinkStatus status = new ServerSentryTicketLinkStatus;
        status.success = success;
        status.discordLinked = linked;
        status.optionsValidForSeconds = validForSeconds;
        status.ticketOptions = {};
        ServerSentryTicketOption option = new ServerSentryTicketOption;
        option.panelId = "offline-panel";
        option.ticketTypeId = "support";
        option.label = "Support";
        status.ticketOptions.Insert(option);
        ApplyTicketLinkStatus(status);
    }

    void StopReview()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(OnTicketLinkTimeout);
    }
}

class TicketOptionsReviewMission extends MissionGameplay
{
    protected int m_Failures;
    protected ref TicketOptionsReviewClient m_Client;

    override void OnInit()
    {
        super.OnInit();
        vector position = "7500 0 7500";
        position[1] = GetGame().SurfaceY(position[0], position[2]) + 0.2;
        PlayerBase player = PlayerBase.Cast(GetGame().CreatePlayer(null, "SurvivorM_Mirek", position, 0, "NONE"));
        if (player) GetGame().SelectPlayer(null, player);
    }

    override void OnMissionStart()
    {
        super.OnMissionStart();
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(Run, 2000, false);
    }

    void Check(bool passed, string name)
    {
        if (!passed) m_Failures++;
        Print("[TicketOptionsReview] " + passed + " " + name);
    }

    void Run()
    {
        m_Client = new TicketOptionsReviewClient;
        m_Client.Seed(40);
        ServerSentryTicketLinkStatus cached = m_Client.GetTicketLinkStatus();
        m_Client.RequestTicketLinkStatus(false);
        Check(m_Client.queries.Count() == 0 && !m_Client.IsCheckingTicketLink(), "form opens from fresh options without an RPC or loading state");
        Check(m_Client.GetTicketLinkStatus() == cached && cached.ticketOptions[0].label == "Support", "form keeps the cached player-specific ticket types");
        m_Client.RequestTicketLinkStatus();
        Check(m_Client.queries.Count() == 1 && m_Client.queries[0].includeTickets, "ticket list still requests fresh tickets");

        m_Client.Seed(40);
        m_Client.RequestTicketLinkStatus(false, true);
        Check(m_Client.queries.Count() == 2 && !m_Client.queries[1].includeTickets && m_Client.queries[1].refreshOptions, "Check again bypasses the cache without fetching the ticket list");
        m_Client.Seed(0);
        m_Client.RequestTicketLinkStatus(false);
        Check(m_Client.queries.Count() == 3 && !m_Client.queries[2].includeTickets, "expired options cause a lightweight form request");
        m_Client.Seed(60, false);
        m_Client.RequestTicketLinkStatus(false);
        Check(m_Client.queries.Count() == 4, "unlinked account state is not reused");
        m_Client.Seed(60, true, false);
        m_Client.RequestTicketLinkStatus(false);
        Check(m_Client.queries.Count() == 5, "failed account state is not reused");
        Check(m_Client.queries[0].requestId != m_Client.queries[4].requestId, "refreshes keep their own correlation tokens");
        m_Client.StopReview();
        CheckFileContract();
        Print("[TicketOptionsReview] Finished failures=" + m_Failures);
        Print("[TicketOptionsReview] Captured RPCs only; no real backend or Discord requests");
    }

    void CheckFileContract()
    {
        ServerSentryTicketLinkRequest request = new ServerSentryTicketLinkRequest;
        request.requestId = "offline-request";
        request.steamId64 = "offline-player";
        request.includeTickets = false;
        request.refreshOptions = true;
        string error;
        JsonFileLoader<ServerSentryTicketLinkRequest>.SaveFile("$profile:options-request.json", request, error);
        ServerSentryTicketLinkRequest loaded;
        bool loadedOk = JsonFileLoader<ServerSentryTicketLinkRequest>.LoadFile("$profile:options-request.json", loaded, error);
        Check(loadedOk && loaded && !loaded.includeTickets && loaded.refreshOptions, "form/refresh intent survives native JSON serialization");
        FileHandle legacy = OpenFile("$profile:legacy-options-request.json", FileMode.WRITE);
        FPrint(legacy, "{\"requestId\":\"legacy\",\"steamId64\":\"offline\"}");
        CloseFile(legacy);
        ServerSentryTicketLinkRequest legacyLoaded = new ServerSentryTicketLinkRequest;
        loadedOk = JsonFileLoader<ServerSentryTicketLinkRequest>.LoadFile("$profile:legacy-options-request.json", legacyLoaded, error);
        Check(loadedOk && legacyLoaded && legacyLoaded.includeTickets && !legacyLoaded.refreshOptions, "fresh legacy request defaults still include the ticket list");
    }
}

void main() {}
Mission CreateCustomMission(string path) { return new TicketOptionsReviewMission; }
