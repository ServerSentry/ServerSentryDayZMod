class ServerSentryGodModePlayerAction: ServerSentryModActionHandler
{
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto d = new ServerSentryLiveModActionDescriptorDto();
        d.id = "player.godmode";
        d.targetType = "player";
        d.label = "God Mode";
        d.icon = "shield-plus";
        d.description = "Toggles god mode for a player.";
        return d;
    }

    override bool Execute(ServerSentryModActionContext ctx, out string message)
    {
        PlayerBase player = ServerSentryLiveActionResolver.FindPlayer(ctx);
        if (!player)
        {
            message = "Player not found or offline.";
            return false;
        }

        bool isGodModeEnabled = player.GetAllowDamage();
        player.SetAllowDamage(!isGodModeEnabled);
        string status;

        if(isGodModeEnabled)
        {
            status = "enabled";
        }
        else
        {
            status = "disabled";
        }

        message = "God mode " + status + ".";

        return true;
    }
};
