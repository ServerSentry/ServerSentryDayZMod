/** Minimal server-to-Agent payload. Never send the resolved suspect identity to the client. */
class ServerSentryDeathReportCommand
{
    string reportedSteamId64;
    string killEventId;
}

/** Server-only references valid until respawn or disconnect. No report delivery is implied. */
class ServerSentryDeathReportStore
{
    protected static const int MAX_PLAYERS = 1024;
    protected static ref map<string, ref ServerSentryDeathReportCommand> s_Deaths;

    /** Keeps only the resolved suspect and emitted kill ID; the CMS reads evidence from telemetry. */
    static void Remember(ServerSentryEvent combatEvent)
    {
        if (!GetGame() || !GetGame().IsServer() || !combatEvent || combatEvent.eventType != "combat_kill") return;
        if (!ServerSentryConfigManager.Get().IsDeathReportingEnabled()) return;
        if (combatEvent.eventId == "" || !combatEvent.target || combatEvent.target.type != "player" || combatEvent.target.steamId == "") return;
        if (combatEvent.actor && combatEvent.actor.steamId == combatEvent.target.steamId) return;
        bool playerSource = combatEvent.actor && combatEvent.actor.type == "player" && combatEvent.actor.steamId != "";
        bool unknownExplosion = combatEvent.combat && combatEvent.combat.isExplosion && (!combatEvent.actor || combatEvent.actor.steamId == "");
        if (!playerSource && !unknownExplosion) return;

        if (!s_Deaths) s_Deaths = new map<string, ref ServerSentryDeathReportCommand>;
        string steamId = combatEvent.target.steamId;
        ServerSentryDeathReportCommand previous = s_Deaths.Get(steamId);
        if (previous && previous.killEventId == combatEvent.eventId) return;
        if (!previous && s_Deaths.Count() >= MAX_PLAYERS) return;

        ServerSentryDeathReportCommand command = new ServerSentryDeathReportCommand;
        command.killEventId = combatEvent.eventId;
        if (combatEvent.actor && combatEvent.actor.type == "player")
            command.reportedSteamId64 = combatEvent.actor.steamId;
        s_Deaths.Set(steamId, command);
    }

    /** Sends only the owner's opaque event reference to the client, never suspect details. */
    static ServerSentryDeathReportCommand Current(string steamId)
    {
        if (!GetGame() || !GetGame().IsServer() || steamId == "" || !s_Deaths) return null;
        if (!ServerSentryConfigManager.Get().IsDeathReportingEnabled()) return null;
        return s_Deaths.Get(steamId);
    }

    /** Callers must use the authenticated RPC sender's Steam ID, never a client-supplied identity. */
    static ServerSentryDeathReportCommand Find(string steamId, string eventId)
    {
        if (!GetGame() || !GetGame().IsServer() || steamId == "" || eventId == "") return null;
        if (!ServerSentryConfigManager.Get().IsDeathReportingEnabled() || !s_Deaths) return null;
        ServerSentryDeathReportCommand command = s_Deaths.Get(steamId);
        if (!command || command.killEventId != eventId) return null;
        return command;
    }

    /** Invalidates only this player's death when the server handles respawn or disconnect. */
    static void Forget(string steamId)
    {
        if (GetGame() && GetGame().IsServer() && s_Deaths && steamId != "")
            s_Deaths.Remove(steamId);
    }

    /** Clears mission-local evidence when the server mission ends. */
    static void Clear()
    {
        if (s_Deaths) s_Deaths.Clear();
        s_Deaths = null;
    }
}
