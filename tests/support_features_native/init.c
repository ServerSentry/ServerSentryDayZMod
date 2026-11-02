/** No real players, ticket requests, Discord messages or refund claims. */
class SupportFeatureReview
{
    protected static int s_Failures;
    static void Check(bool passed, string name)
    {
        if (!passed) s_Failures++;
        Print("[SupportFeatureReview] " + passed + " " + name);
    }

    static void Run()
    {
        ServerSentryConfigManager config = ServerSentryConfigManager.Get();
        ServerSentryServerManager server = ServerSentryServerManager.Cast(CF_ModuleCoreManager.Get(ServerSentryServerManager));
        Check(server && server.ReviewHasTicketBridges() == config.IsTicketsEnabled(), "server creates ticket bridges only when configured");
        config.PublishModFeatures();
        ServerSentryModFeatures published;
        string error;
        bool loaded = JsonFileLoader<ServerSentryModFeatures>.LoadFile("$profile:ServerSentry/features.json", published, error);
        Check(loaded && published && published.tickets == config.IsTicketsEnabled() && published.supportCases == config.IsSupportCasesEnabled(), "Agent feature file reflects the server's loaded settings");

        ServerSentryLoggingConfig defaults = new ServerSentryLoggingConfig;
        Check(defaults.EnableTickets && defaults.EnableSupportCases, "existing support functionality stays enabled by default");
        loaded = JsonFileLoader<ServerSentryLoggingConfig>.LoadData("{}", defaults, error);
        Check(loaded && defaults.EnableTickets && defaults.EnableSupportCases, "older configs retain tickets and refunds");

        ServerSentryClientManager client = ServerSentryClientManager.Get();
        Check(client && !client.HasSupportFeatures() && !client.ReviewHasTicketRuntime() && client.ReviewBindingCount() == 0, "client has no support runtime or hotkeys before server authorization");
        if (!client) return;
        ServerSentryModFeatures disabled = new ServerSentryModFeatures;
        client.ReviewApply(disabled);
        client.RequestSupportCaseFromServer();
        client.RequestReimbursement();
        client.RequestTicketLinkStatus();
        client.SubmitTicket(null, "must not send", false);
        client.OpenNotifiedTicket();
        Check(!client.IsCheckingTicketLink() && !client.IsCreatingTicket() && !client.IsCheckingReimbursement(), "disabled entry points start no requests or timeout state");
        Check(!GetGame().GetMission().CreateScriptedMenu(ServerSentryTicketsMenu.MENU_ID) && !GetGame().GetMission().CreateScriptedMenu(4771341), "disabled support menus are rejected before widgets are created");

        ServerSentryModFeatures tickets = new ServerSentryModFeatures;
        tickets.tickets = true;
        client.ReviewApply(tickets);
        Check(client.IsTicketsEnabled() && !client.IsSupportCasesEnabled() && client.ReviewHasTicketRuntime() && client.ReviewBindingCount() == 2, "tickets alone allocate their runtime and two hotkeys");
        ServerSentryTicketChat chat = client.GetTicketChat();
        client.ReviewApply(tickets);
        Check(client.GetTicketChat() == chat && client.ReviewBindingCount() == 2, "repeated feature settings do not duplicate chat objects or bindings");
        client.ReviewApply(disabled);
        Check(!client.ReviewHasTicketRuntime() && client.ReviewBindingCount() == 0, "disabled features release chat/notification state and hotkeys");

        ServerSentryModFeatures cases = new ServerSentryModFeatures;
        cases.supportCases = true;
        client.ReviewApply(cases);
        Check(!client.IsTicketsEnabled() && client.IsSupportCasesEnabled() && !client.ReviewHasTicketRuntime() && client.ReviewBindingCount() == 1, "refunds alone do not load ticket runtime or notification shortcut");
        client.ReviewApply(disabled);
        Check(client.ReviewBindingCount() == 0 && !client.GetTicketChat(), "cleanup returns to an empty disabled client");
        Print("[SupportFeatureReview] Finished failures=" + s_Failures);
    }
}

void main() { GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SupportFeatureReview.Run, 2000, false); }
