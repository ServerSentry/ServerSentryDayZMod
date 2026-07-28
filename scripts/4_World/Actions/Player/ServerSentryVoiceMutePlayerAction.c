/**
	\file ServerSentryVoiceMutePlayerAction.c
	\brief Implements live-action support for Voice mute player action.
*/

/**
	\brief Implements the Voice mute player ServerSentry action.
*/
class ServerSentryVoiceMutePlayerAction: ServerSentryModActionHandler
{
    /**
    	\brief Unmutes by steam ID.
    	\param steamId \p Steam ID of the target player.
    */
    static void UnmuteBySteamId(string steamId)
    {
        if (steamId == "")
        {
            return;
        }

        ServerSentryVoiceMuteDb.Get().RemoveMute(steamId);

        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);
        foreach (Man man: players)
        {
            PlayerBase player = PlayerBase.Cast(man);
            if (!player || !player.GetIdentity())
            {
                continue;
            }

            if (player.GetIdentity().GetPlainId() == steamId)
            {
                GetGame().EnableVoN(player, true);
                return;
            }
        }
    }

    /**
    	\brief Returns the metadata descriptor published for this live action.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto d = new ServerSentryLiveModActionDescriptorDto();
        d.id = "player.voice.mute";
        d.targetType = "player";
        d.label = "Voice Mute";
        d.icon = "MicOff";
        d.description = "Blocks voice communication of a player. Optional duration is sent by the CMS action input.";
        ServerSentryLiveModActionInputDto minutes = ServerSentryLiveModActionInputFactory.Number("minutes", "Minutes", false, 0, 10080, 1, "0");
        minutes.placeholder = "0 = permanent";
        minutes.helpText = "0 keeps the player muted until UnMute is used. A value greater than 0 schedules an automatic unmute while the server stays online.";
        d.inputs.Insert(minutes);
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

        PlayerIdentity identity = player.GetIdentity();
        if (!identity)
        {
            message = "Player identity not available.";
            return false;
        }

        int minutes = ServerSentryLiveActionInput.GetInt(ctx.inputJson, "minutes", 0);
        if (minutes < 0)
        {
            minutes = 0;
        }

        GetGame().EnableVoN(player, false);
        string steamId = identity.GetPlainId();
        ServerSentryVoiceMuteDb.Get().AddMute(steamId);

        if (minutes > 0)
        {
            int delayMs = minutes * 60 * 1000;
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ServerSentryVoiceMutePlayerAction.UnmuteBySteamId, delayMs, false, steamId);
            message = "Players voice disabled for " + minutes.ToString() + " minutes.";
            return true;
        }

        message = "Players voice disabled permanently.";
        return true;
    }
}
