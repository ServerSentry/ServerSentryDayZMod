/**
	\file ServerSentryChatMessageData.c
	\brief Defines the Chat message data logging data model.
*/

/**
	\brief Provides ServerSentry behavior for Chat message data.
*/
class ServerSentryChatMessageData
{
    /**
    	\brief Stores the source mod.
    */
    string sourceMod;
    /**
    	\brief Stores the result.
    */
    string result;
    /**
    	\brief Stores the source hook.
    */
    string sourceHook;

    /**
    	\brief Stores the channel ID.
    */
    int channelId;
    /**
    	\brief Stores the channel name.
    */
    string channelName;
    /**
    	\brief Stores the channel type.
    */
    string channelType;
    /**
    	\brief Indicates whether is direct.
    */
    bool isDirect;
    /**
    	\brief Indicates whether is global.
    */
    bool isGlobal;
    /**
    	\brief Indicates whether is group.
    */
    bool isGroup;
    /**
    	\brief Indicates whether is vehicle.
    */
    bool isVehicle;
    /**
    	\brief Indicates whether is admin.
    */
    bool isAdmin;
    /**
    	\brief Indicates whether is system.
    */
    bool isSystem;
    /**
    	\brief Indicates whether is command.
    */
    bool isCommand;

    /**
    	\brief Stores the sender steam ID.
    */
    string senderSteamId;
    /**
    	\brief Stores the sender bohemia ID.
    */
    string senderBohemiaId;
    /**
    	\brief Stores the sender name.
    */
    string senderName;
    /**
    	\brief Stores the sender position.
    */
    ref ServerSentryVectorDto senderPosition;

    /**
    	\brief Human-readable result or error message.
    */
    string message;
    /**
    	\brief Stores the original message.
    */
    string originalMessage;
    /**
    	\brief Stores the filtered message.
    */
    string filteredMessage;
    /**
    	\brief Stores the extra.
    */
    string extra;
    /**
    	\brief Stores the prefix.
    */
    string prefix;
    /**
    	\brief Stores the group prefix.
    */
    string groupPrefix;
    /**
    	\brief Stores the group tag.
    */
    string groupTag;
    /**
    	\brief Stores the raw.
    */
    string raw;

    /**
    	\brief Initializes a new ServerSentryChatMessageData instance.
    */
    void ServerSentryChatMessageData()
    {
        sourceMod = "DayZ_Vanilla";
        result = "success";
        sourceHook = "";

        channelId = 0;
        channelName = "";
        channelType = "";
        isDirect = false;
        isGlobal = false;
        isGroup = false;
        isVehicle = false;
        isAdmin = false;
        isSystem = false;
        isCommand = false;

        senderSteamId = "";
        senderBohemiaId = "";
        senderName = "";
        senderPosition = new ServerSentryVectorDto();

        message = "";
        originalMessage = "";
        filteredMessage = "";
        extra = "";
        prefix = "";
        groupPrefix = "";
        groupTag = "";
        raw = "";
    }
}
