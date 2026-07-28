/**
	\file ServerSentryVoiceUnMutePlayerAction.c
	\brief Implements live-action support for Voice un mute player action.
*/

/**
	\brief Implements the Voice un mute player ServerSentry action.
*/
class ServerSentryVoiceUnMutePlayerAction: ServerSentryModActionHandler
{
    /**
    	\brief Returns the metadata descriptor published for this live action.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto d = new ServerSentryLiveModActionDescriptorDto();
        d.id = "player.voice.unmute";
        d.targetType = "player";
        d.label = "Voice UnMute";
        d.icon = "mic";
        d.description = "Enables voice communication of a player.";
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

        GetGame().EnableVoN(player, true);
        ServerSentryVoiceMuteDb.Get().RemoveMute(player.GetIdentity().GetPlainId());
        message = "Players voice enabled!";
        return true;
    }
}
