/** Character backup listing and one-use, player-instance-bound restore authorization. */
class ServerSentryCharacterBackupDto
{
    string id;
    string createdAtUtc;
    int itemCount = -1;
    string worldName;
    int compatible;
}

class ServerSentryCharacterBackupsDto
{
    int version = 1;
    string steamId;
    string restoreToken;
    int canRestore;
    int page;
    int total;
    ref array<ref ServerSentryCharacterBackupDto> backups = new array<ref ServerSentryCharacterBackupDto>();
}

class ServerSentryCharacterRestorePermit
{
    string token;
    PlayerBase player;
    int created;
    ref array<string> backupIds = new array<string>();
}

class ServerSentryCharacterBackupAction: ServerSentryModActionHandler
{
    protected string m_Operation;
    protected static ref map<string, ref ServerSentryCharacterRestorePermit> s_Permits = new map<string, ref ServerSentryCharacterRestorePermit>();

    void ServerSentryCharacterBackupAction(string operation)
    {
        m_Operation = operation;
    }

    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto descriptor = new ServerSentryLiveModActionDescriptorDto();
        descriptor.id = "character." + m_Operation;
        descriptor.targetType = "player";
        descriptor.label = "Character " + m_Operation;
        descriptor.icon = "rotate-ccw";
        descriptor.dangerous = m_Operation == "restore";
        descriptor.scopes = new array<string>();
        descriptor.scopes.Insert("character_backups");
        return descriptor;
    }

    static void ResetPermits()
    {
        s_Permits.Clear();
    }

    protected void Prune()
    {
        int now = GetGame().GetTime();
        array<string> expired = new array<string>();
        foreach (string key, ServerSentryCharacterRestorePermit permit : s_Permits)
        {
            if (!permit || !permit.player || now < permit.created || now - permit.created > 120000)
                expired.Insert(key);
        }
        foreach (string oldKey : expired)
            s_Permits.Remove(oldKey);
    }

    /**
        \brief Lists backups or restores a selected snapshot after checking the exact player target.
        \param ctx \p authenticated live-action context.
        \param message \p receives the operation result.
        \return \p true when the requested operation succeeds.
    */
    override bool Execute(ServerSentryModActionContext ctx, out string message)
    {
        message = "An exact player identity on the game server is required.";
        if (!GetGame() || !GetGame().IsServer())
            return false;

        if (!IsExactPlayerTarget(ctx))
            return false;

        Prune();
        PlayerBase player = ServerSentryLiveActionResolver.FindPlayer(ctx);
        if (m_Operation == "backups")
            return ListBackups(ctx, player, message);

        if (m_Operation == "restore")
            return RestoreBackup(ctx, player, message);

        message = "Unknown character backup operation.";
        return false;
    }

    /**
        \brief Accepts a Steam ID without conflicting object or Bohemia ID selectors.
        \param ctx \p requested player target.
        \return \p true for one exact, numeric 17-character player identity.
    */
    protected bool IsExactPlayerTarget(ServerSentryModActionContext ctx)
    {
        if (!ctx || ctx.targetType != "player")
            return false;

        if (ctx.steamId.Length() != 17 || ctx.objectId != "" || ctx.bohemiaId != "")
            return false;

        string digits = "0123456789";
        for (int i = 0; i < ctx.steamId.Length(); i++)
        {
            if (digits.IndexOf(ctx.steamId.Substring(i, 1)) < 0)
                return false;
        }
        return true;
    }

    /**
        \brief Loads one page and issues a new permit for this exact player instance.
        \param ctx \p request containing the page and Steam ID.
        \param player \p current player, or null for an offline listing.
        \param message \p receives the listing result.
        \return \p true when the response was serialized.
    */
    protected bool ListBackups(ServerSentryModActionContext ctx, PlayerBase player, out string message)
    {
        message = "Invalid character backup page.";
        int page = ctx.GetInt("page", 1);
        if (page < 1 || page > 100000)
            return false;

        ServerSentryCharacterBackupsDto data = ServerSentryCharacterBackupService.Get().ListBackups(ctx.steamId, page, player);
        s_Permits.Remove(ctx.steamId);
        IssueRestorePermit(data, player);

        JsonSerializer serializer = new JsonSerializer();
        if (!serializer.WriteToString(data, false, ctx.resultDataJson))
        {
            message = "Could not serialize character backups.";
            return false;
        }
        message = "Character backups loaded.";
        return true;
    }

    /**
        \brief Authorizes only compatible backups returned on the current page.
        \param data \p listing receiving the one-use token.
        \param player \p exact player instance bound to the permit.
    */
    protected void IssueRestorePermit(ServerSentryCharacterBackupsDto data, PlayerBase player)
    {
        if (data.canRestore != 1 || s_Permits.Count() >= 128)
        {
            data.canRestore = 0;
            return;
        }

        ServerSentryCharacterRestorePermit permit = new ServerSentryCharacterRestorePermit();
        int uuid[4];
        UUIDApi.Generate(uuid);
        permit.token = UUIDApi.FormatString(uuid);
        permit.player = player;
        permit.created = GetGame().GetTime();
        foreach (ServerSentryCharacterBackupDto item : data.backups)
        {
            if (item.compatible == 1)
                permit.backupIds.Insert(item.id);
        }
        s_Permits.Set(data.steamId, permit);
        data.restoreToken = permit.token;
    }

    /**
        \brief Consumes the selected permit before changing the live inventory.
        \param ctx \p request containing the selected backup and token.
        \param player \p current player instance.
        \param message \p receives the restore result.
        \return \p true when the selected restore succeeds.
    */
    protected bool RestoreBackup(ServerSentryModActionContext ctx, PlayerBase player, out string message)
    {
        message = "Player changed character or the backup selection expired. Reload the backup list.";
        string backupId = ctx.GetString("backupId");
        ServerSentryCharacterRestorePermit selected = s_Permits.Get(ctx.steamId);
        if (!selected || !player || selected.player != player)
            return false;

        if (selected.token != ctx.GetString("restoreToken") || selected.backupIds.Find(backupId) < 0)
            return false;

        // Consume before mutation. Duplicate delivery and restarts cannot reuse this permit.
        s_Permits.Remove(ctx.steamId);
        return ServerSentryCharacterBackupService.Get().RestoreSelected(player, backupId, message);
    }
}
