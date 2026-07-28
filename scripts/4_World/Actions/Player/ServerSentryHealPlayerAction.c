/**
	\file ServerSentryHealPlayerAction.c
	\brief Implements live-action support for Heal player action.
*/

/**
	\brief Implements the Heal player ServerSentry action.
*/
class ServerSentryHealPlayerAction: ServerSentryModActionHandler
{
    /**
    	\brief Returns the metadata descriptor published for this live action.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto d = new ServerSentryLiveModActionDescriptorDto();
        d.id = "player.heal";
        d.targetType = "player";
        d.label = "Heal";
        d.icon = "heart-pulse";
        d.description = "Restores health, blood and shock for an online player.";
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

        player.SetHealth("", "Health", player.GetMaxHealth("","Health"));
		player.SetHealth("", "Shock", player.GetMaxHealth("","Shock"));
		player.SetHealth("", "Blood", player.GetMaxHealth("","Blood"));
        player.GetStatEnergy().Add(5000);
        player.GetStatWater().Add(5000);

        player.GetBleedingManagerServer().RemoveAllSources();
        player.GetModifiersManager().DeactivateAllModifiers();
        player.RemoveAllAgents();
        player.SetBloodyHands(false);
        message = "Player healed.";
        return true;
    }
}
