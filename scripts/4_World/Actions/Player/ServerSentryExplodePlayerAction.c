/**
	\file ServerSentryExplodePlayerAction.c
	\brief Implements live-action support for Explode player action.
*/

/**
	\brief Implements the Explode player ServerSentry action.
*/
class ServerSentryExplodePlayerAction: ServerSentryModActionHandler
{
    /**
    	\brief Returns the metadata descriptor published for this live action.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto d = new ServerSentryLiveModActionDescriptorDto();
        d.id = "player.explode";
        d.targetType = "player";
        d.label = "Explode";
        d.icon = "Bomb";
        d.description = "Explodes a player.";
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

        player.Explode(DamageType.EXPLOSION, "LandFuelFeed_Ammo");
        message = "Player exploded.";
        return true;
    }
}
