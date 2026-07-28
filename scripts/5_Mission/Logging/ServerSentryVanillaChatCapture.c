/**
	\file ServerSentryVanillaChatCapture.c
	\brief Provides logging support for Vanilla chat capture.
*/

/**
	\brief Provides ServerSentry behavior for Vanilla chat capture.
*/
class ServerSentryVanillaChatCapture
{
    /**
    	\brief Emits the requested operation.
    	\param eventTypeId \p DayZ event type identifier.
    	\param params \p parameters supplied by the DayZ event.
    */
    static void Emit(EventType eventTypeId, Param params)
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return;
        }

        if (!ServerSentryDefaults.LogChatEvents())
        {
            return;
        }

        if (ServerSentryDefaults.SuppressVanillaChatWhenExternalProviderActive() && ServerSentryChatProviderRegistry.ShouldSuppressVanillaChat())
        {
            return;
        }

        if (eventTypeId != ChatMessageEventTypeID)
        {
            return;
        }

        ChatMessageEventParams chatParams = ChatMessageEventParams.Cast(params);
        if (!chatParams)
        {
            return;
        }

        string message = NormalizeMessage(chatParams.param3);
        if (message == "")
        {
            return;
        }

        ServerSentryChatMessageData data = new ServerSentryChatMessageData();
        data.sourceMod = "DayZ_Vanilla";
        data.sourceHook = "MissionServer.OnEvent";
        data.result = "success";
        data.channelId = chatParams.param1;
        data.channelName = ResolveVanillaChannelName(chatParams.param1);
        data.channelType = ResolveVanillaChannelType(chatParams.param1);
        data.senderName = chatParams.param2;
        data.message = message;
        data.originalMessage = message;
        data.filteredMessage = message;
        data.extra = chatParams.param4;
        data.isCommand = IsCommand(message);
        FillChannelFlags(data);

        PlayerBase player = ResolvePlayerByName(data.senderName);
        FillSender(data, player);

        ServerSentryApi.EmitChatMessage(data);
    }

    /**
    	\brief Normalizes message.
    	\param message \p receives a human-readable result or error message.
    	\return \p The resulting string value.
    */
    static string NormalizeMessage(string message)
    {
        if (message == "")
        {
            return "";
        }

        if (message.Length() > 0 && message[0] == "+")
        {
            return message.Substring(1, message.Length() - 1);
        }

        return message;
    }

    /**
    	\brief Returns whether command.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsCommand(string message)
    {
        if (message == "")
        {
            return false;
        }

        return message[0] == "!" || message[0] == "/" || message[0] == "#";
    }

    /**
    	\brief Processes the fill sender operation.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    */
    static void FillSender(ServerSentryChatMessageData data, PlayerBase player)
    {
        if (!data || !player)
        {
            return;
        }

        PlayerIdentity identity = player.GetIdentity();
        if (identity)
        {
            data.senderSteamId = identity.GetPlainId();
            data.senderBohemiaId = identity.GetId();
            data.senderName = identity.GetName();
        }

        data.senderPosition.FromVector(player.GetPosition());
    }

    /**
    	\brief Resolves player by name.
    	\param name \p name used by the operation.
    	\return \p The resulting player base instance, or null when unavailable.
    */
    static PlayerBase ResolvePlayerByName(string name)
    {
        if (!GetGame() || name == "")
        {
            return null;
        }

        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);

        foreach (Man man: players)
        {
            PlayerBase player = PlayerBase.Cast(man);
            if (!player || !player.GetIdentity())
            {
                continue;
            }

            if (player.GetIdentity().GetName() == name)
            {
                return player;
            }
        }

        return null;
    }

    /**
    	\brief Processes the fill channel flags operation.
    	\param data \p data processed by the operation.
    */
    static void FillChannelFlags(ServerSentryChatMessageData data)
    {
        if (!data)
        {
            return;
        }

        data.isDirect = (data.channelId == 0) || ((data.channelId & CCDirect) != 0) || ((data.channelId & CCMegaphone) != 0) || ((data.channelId & CCTransmitter) != 0) || ((data.channelId & CCPublicAddressSystem) != 0);
        data.isGlobal = false;
        data.isGroup = false;
        data.isVehicle = false;
        data.isAdmin = ((data.channelId & CCAdmin) != 0);
        data.isSystem = ((data.channelId & CCSystem) != 0) || ((data.channelId & CCBattlEye) != 0);
    }

    /**
    	\brief Resolves vanilla channel name.
    	\param channel \p channel used by the operation.
    	\return \p The resulting string value.
    */
    static string ResolveVanillaChannelName(int channel)
    {
        if ((channel & CCSystem) != 0)
        {
            return "System";
        }

        if ((channel & CCBattlEye) != 0)
        {
            return "BattlEye";
        }

        if ((channel & CCAdmin) != 0)
        {
            return "Admin";
        }

        if ((channel & CCMegaphone) != 0)
        {
            return "Megaphone";
        }

        if ((channel & CCTransmitter) != 0)
        {
            return "Radio";
        }

        if ((channel & CCPublicAddressSystem) != 0)
        {
            return "PAS";
        }

        if ((channel & CCDirect) != 0 || channel == 0)
        {
            return "Direct";
        }

        return "Channel " + channel.ToString();
    }

    /**
    	\brief Resolves vanilla channel type.
    	\param channel \p channel used by the operation.
    	\return \p The resulting string value.
    */
    static string ResolveVanillaChannelType(int channel)
    {
        string name = ResolveVanillaChannelName(channel);
        name.ToLower();
        return name;
    }
}
