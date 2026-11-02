/** Native server checks only. Synthetic identities are never sent to an Agent or CMS. */
class DeathReportConfigReview : ServerSentryConfigManager
{
    void Configure(bool enabled, bool telemetry = true, bool kills = true)
    {
        m_ServerConfig.EnableDeathReports = enabled;
        m_ServerConfig.EnableTelemtryLogging = telemetry;
        m_ServerConfig.LogPlayerKills = kills;
    }
}

class DeathReportStoreReview : ServerSentryDeathReportStore
{
    static bool HasAllocatedState() { return s_Deaths != null; }
}

class DeathReportFactoryReview : ServerSentryCombatFactory
{
    static void CapturePosition(ServerSentryHitContext ctx)
    {
        CaptureExplosiveAttackerPosition(ctx);
    }
}

class DeathReportReview
{
    protected static int s_Failures;
    protected static const string VICTIM = "76561198000000001";
    protected static const string KILLER = "76561198000000002";

    static void Check(bool passed, string name)
    {
        if (!passed) s_Failures++;
        Print("[DeathReportReview] " + passed + " " + name);
    }

    static ServerSentryHitContext Context()
    {
        ServerSentryHitContext ctx = new ServerSentryHitContext;
        ctx.isLethal = true;
        ctx.targetAliveAfter = false;
        ctx.occurredAtUtc = ServerSentryTime.NowUtc();
        ctx.hitId = "review-hit";
        ctx.target.type = "player";
        ctx.target.steamId = VICTIM;
        ctx.attacker.type = "player";
        ctx.attacker.steamId = KILLER;
        ctx.victimPosition = "7500 20 7600";
        ctx.attackerPosition = "7400 30 7700";
        ctx.hasAttackerPosition = true;
        ctx.hitZone = "Torso";
        ctx.hitComponentNames.Insert("dmgZone_torso");
        ctx.hitComponentNames.Insert("Spine3");
        ctx.healthDamage = 100;
        return ctx;
    }

    static ServerSentryEvent Emit(ServerSentryHitContext ctx)
    {
        ServerSentryEvent combatEvent = ServerSentryEventFactory.CreateCombatHit(null, ctx);
        Check(ServerSentryLogger.Get().Emit(combatEvent), "native logger accepts the fixture event");
        ServerSentryDeathReportStore.Remember(combatEvent);
        return combatEvent;
    }

    static void Run()
    {
        ServerSentryConfigManager originalConfig = ServerSentryConfigManager.Get();
        DeathReportConfigReview config = new DeathReportConfigReview;
        ServerSentryConfigManager.g_Instance = config;
        CheckFeatureFlag(config);
        config.Configure(true);
        ServerSentryDeathReportStore.Clear();
        ServerSentryHitContext ctx = Context();
        ServerSentryEvent first = Emit(ctx);
        ServerSentryDeathReportCommand stored = ServerSentryDeathReportStore.Find(VICTIM, first.eventId);
        Check(stored && stored.killEventId == first.eventId && first.bootId != "", "report references the exact emitted telemetry event");
        Check(stored.reportedSteamId64 == KILLER && first.target.steamId == VICTIM, "command identifies the resolved suspect; telemetry identifies the reporter");
        Check(first.actorPosition.x == 7400 && first.actorPosition.y == 30 && first.actorPosition.z == 7700 && first.targetPosition.x == 7500 && first.targetPosition.y == 20 && first.targetPosition.z == 7600, "existing telemetry keeps distinct killer and victim positions");
        Check(first.combat.hitZone == "Torso" && first.combat.hitComponentNames.Count() == 2 && first.combat.damage == 100, "existing telemetry keeps hitmap component names and damage");
        CheckPayload(stored);
        Check(!ServerSentryDeathReportStore.Find(KILLER, first.eventId), "another Steam account cannot select the victim's death");
        Check(!ServerSentryDeathReportStore.Find(VICTIM, "invented-id"), "invented telemetry IDs are rejected");

        ctx.isLethal = false;
        ctx.targetAliveAfter = true;
        ServerSentryEvent hit = Emit(ctx);
        Check(!ServerSentryDeathReportStore.Find(VICTIM, hit.eventId) && ServerSentryDeathReportStore.Find(VICTIM, first.eventId) == stored, "nonlethal hits cannot replace a reportable death");
        ctx.isLethal = true;
        ctx.target.type = "ai";
        ServerSentryEvent ai = Emit(ctx);
        Check(!ServerSentryDeathReportStore.Find(VICTIM, ai.eventId) && ServerSentryDeathReportStore.Find(VICTIM, first.eventId) == stored, "human AI does not create a player report context");

        ctx = Context();
        ServerSentryEvent second = Emit(ctx);
        stored = ServerSentryDeathReportStore.Find(VICTIM, second.eventId);
        Check(!ServerSentryDeathReportStore.Find(VICTIM, first.eventId) && stored && stored.killEventId == second.eventId, "a new death replaces the previous life even when hit IDs repeat");
        ctx.target.steamId = KILLER;
        ctx.attacker.steamId = VICTIM;
        ServerSentryEvent other = Emit(ctx);
        ServerSentryDeathReportStore.Forget(VICTIM);
        Check(!ServerSentryDeathReportStore.Find(VICTIM, second.eventId), "respawn/disconnect invalidation rejects the old death");
        Check(ServerSentryDeathReportStore.Find(KILLER, other.eventId) != null, "one player's respawn preserves other players' report references");
        Check(stored.killEventId == second.eventId && stored.reportedSteamId64 == KILLER, "invalidation does not mutate an already obtained command");
        ServerSentryDeathReportStore.Forget(VICTIM);
        ServerSentryDeathReportStore.Forget("");
        Check(ServerSentryDeathReportStore.Find(KILLER, other.eventId) != null, "duplicate or missing-identity cleanup does not clear another player");
        ServerSentryDeathReportStore.Clear();
        Check(!ServerSentryDeathReportStore.Find(KILLER, other.eventId), "mission cleanup removes server-owned context");

        CheckExplosion();
        ServerSentryEvent pendingReport = Emit(Context());
        config.Configure(false);
        Check(DeathReportStoreReview.HasAllocatedState() && !ServerSentryDeathReportStore.Find(VICTIM, pendingReport.eventId), "disabled feature rejects report lookups even if state exists");
        ServerSentryDeathReportStore.Clear();
        Check(!DeathReportStoreReview.HasAllocatedState(), "cleanup releases the report cache instead of allocating an empty map");
        ServerSentryConfigManager.g_Instance = originalConfig;
        ServerSentryLogger.Get().Flush();
        Print("[DeathReportReview] Finished failures=" + s_Failures);
        Print("[DeathReportReview] Local fixture only; no report submitted or acknowledged");
    }

    static void CheckFeatureFlag(DeathReportConfigReview config)
    {
        ServerSentryLoggingConfig settings = new ServerSentryLoggingConfig;
        Check(!settings.EnableDeathReports, "new server configuration defaults death reports to disabled");
        string error;
        bool loaded = JsonFileLoader<ServerSentryLoggingConfig>.LoadData("{}", settings, error);
        Check(loaded && settings && !settings.EnableDeathReports, "legacy configuration without the flag stays disabled");
        loaded = JsonFileLoader<ServerSentryLoggingConfig>.LoadData("{\"EnableDeathReports\":true}", settings, error);
        Check(loaded && settings && settings.EnableDeathReports, "explicit server opt-in survives native JSON loading");
        string defaults;
        JsonFileLoader<ServerSentryLoggingConfig>.MakeData(new ServerSentryLoggingConfig, defaults, error);
        string merged;
        bool upgraded = ServerSentryConfigJson.MergeDefaults("{\"EnableDeathReports\":false,\"Custom\":42}", defaults, merged);
        loaded = JsonFileLoader<ServerSentryLoggingConfig>.LoadData(merged, settings, error);
        Check(upgraded && loaded && !settings.EnableDeathReports && merged.Contains("42"), "config default merging preserves an explicit false and unrelated settings");

        ServerSentryDeathReportStore.Clear();
        config.Configure(false);
        ServerSentryHitContext ctx = Context();
        ServerSentryEvent ordinaryKill = Emit(ctx);
        Check(!config.IsDeathReportingEnabled() && !DeathReportStoreReview.HasAllocatedState() && !ServerSentryDeathReportStore.Find(VICTIM, ordinaryKill.eventId), "disabled reporting keeps ordinary kill logging without allocating report state");
        ctx.hasAttackerPosition = false;
        DeathReportFactoryReview.CapturePosition(ctx);
        Check(!ctx.hasAttackerPosition, "disabled reporting leaves the additional killer position unset");

        config.Configure(true, false);
        Check(!config.IsDeathReportingEnabled(), "reporting cannot activate without its required telemetry");
        config.Configure(true, true, false);
        Check(!config.IsDeathReportingEnabled(), "reporting cannot activate without player kill logs");
        config.Configure(true);
        Check(config.IsDeathReportingEnabled(), "explicit opt-in and required telemetry enable reporting");
    }

    static void CheckExplosion()
    {
        ItemBase grenade = ItemBase.Cast(GetGame().CreateObjectEx("M67Grenade", "7502 20 7600", ECE_LOCAL));
        Check(grenade != null, "native explosive source created");
        if (!grenade) return;
        ServerSentryExplosiveAttribution state = grenade.ServerSentryExplosiveState(true);
        state.activation = "fuse";
        state.releasedBy = new ServerSentryExplosiveIdentity;
        state.releasedBy.type = "player";
        state.releasedBy.steamId = KILLER;
        state.armedBy = new ServerSentryExplosiveIdentity;
        state.armedBy.steamId = "76561198000000003";

        ServerSentryHitContext ctx = Context();
        ctx.hasAttackerPosition = false;
        ctx.isExplosion = true;
        string resolution = ServerSentryExplosiveResolver.FillActor(ctx.attacker, grenade, ctx.explosive);
        DeathReportFactoryReview.CapturePosition(ctx);
        ServerSentryEvent blast = Emit(ctx);
        Check(resolution == "released_by" && blast.actor.steamId == KILLER, "existing resolver attributes the grenade to its verified releasing player");
        Check(!blast.actorPosition && blast.targetPosition.x == 7500 && blast.combat.explosive.sourcePosition.x == 7502, "offline killer stays without a position; blast origin is never substituted");

        state.activation = "damage";
        grenade.ServerSentryResetExplosionSnapshot();
        ServerSentryExplosiveResolver.FillActor(ctx.attacker, grenade, ctx.explosive);
        ServerSentryEvent unknown = Emit(ctx);
        Check(unknown.actor.type == "unknown" && unknown.actor.steamId == "" && !unknown.actorPosition, "unresolved external detonation does not blame the old owner");
        ServerSentryDeathReportCommand command = ServerSentryDeathReportStore.Find(VICTIM, unknown.eventId);
        Check(command && command.killEventId == unknown.eventId && command.reportedSteamId64 == "", "unknown attribution retains the kill reference without inventing a suspect");
        GetGame().ObjectDelete(grenade);
    }

    static void CheckPayload(ServerSentryDeathReportCommand command)
    {
        string json;
        string error;
        bool serialized = JsonFileLoader<ServerSentryDeathReportCommand>.MakeData(command, json, error, false);
        string expected = "{\"reportedSteamId64\":\"" + KILLER + "\",\"killEventId\":\"" + command.killEventId + "\"}";
        Check(serialized && json == expected, "native report JSON contains only suspect Steam ID and kill event ID");
    }
}

void main()
{
    GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DeathReportReview.Run, 2000, false);
}
