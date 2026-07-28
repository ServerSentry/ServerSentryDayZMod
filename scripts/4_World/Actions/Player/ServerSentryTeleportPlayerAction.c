/**
	\file ServerSentryTeleportPlayerAction.c
	\brief Implements live-action support for Teleport player action.
*/

/**
	\brief Implements the Teleport player ServerSentry action.
*/
class ServerSentryTeleportPlayerAction: ServerSentryModActionHandler
{
    /**
    	\brief Returns the metadata descriptor published for this live action.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto d = new ServerSentryLiveModActionDescriptorDto();
        d.id = "player.teleport";
        d.targetType = "player";
        d.label = "Teleport";
        d.icon = "map-pin";
        d.dangerous = true;
        d.confirmText = "Teleport this player to the selected map position?";
        d.description = "Teleports an online player to the target position sent by the live map.";
        return d;
    }

    /**
    	\brief Executes the live action using the supplied request context.
    	\param ctx \p action context containing request, target, and input data.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    override bool Execute(ServerSentryModActionContext ctx, out string message)
    {
        PlayerBase player = ServerSentryLiveActionResolver.FindPlayer(ctx);
        if (!player)
        {
            message = "Player not found or offline.";
            return false;
        }

        if (!ctx || !ctx.position)
        {
            message = "Teleport target position is missing.";
            return false;
        }

        if (ctx.position.x < 0 || ctx.position.z < 0)
        {
            message = "Teleport target position is outside the map.";
            return false;
        }

        float surfaceY = GetGame().SurfaceY(ctx.position.x, ctx.position.z);
        vector targetPosition = Vector(ctx.position.x, surfaceY + 0.35, ctx.position.z);
        vector oldPosition = player.GetPosition();

        player.SetPosition(targetPosition);

        string oldPositionText = Math.Round(oldPosition[0]).ToString() + "," + Math.Round(oldPosition[1]).ToString() + "," + Math.Round(oldPosition[2]).ToString();
        string targetPositionText = Math.Round(targetPosition[0]).ToString() + "," + Math.Round(targetPosition[1]).ToString() + "," + Math.Round(targetPosition[2]).ToString();
        Print("[ServerSentry][LiveAction][Teleport] requestId=" + ctx.requestId + " player=" + player.ToString() + " steamId=" + ctx.steamId + " from=" + oldPositionText + " to=" + targetPositionText + " requestedBy=" + ctx.requestedBy);
        message = "Teleported player to X " + Math.Round(ctx.position.x).ToString() + " / Z " + Math.Round(ctx.position.z).ToString() + ".";
        ServerSentryLiveApi.WriteSnapshot(); // refresh live data
        return true;
    }
}
