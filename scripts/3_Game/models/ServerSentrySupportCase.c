/**
	\file ServerSentrySupportCase.c
	\brief Defines ServerSentry support for Support case.
*/

/**
	\brief Provides ServerSentry behavior for Support case.
*/
class ServerSentrySupportCase
{
    /**
    	\brief Stores the schema version.
    */
    int SchemaVersion;
    /**
    	\brief Stores the support case ID.
    */
    string SupportCaseId;
    /**
    	\brief Stores the player name.
    */
    string PlayerName; //Discord @Tag
    /**
    	\brief Stores the discord user ID.
    */
    string DiscordUserId;
    /**
    	\brief Stores the steam ID 64.
    */
    string SteamId64;
    /**
    	\brief Stores the reason.
    */
    string Reason;
    /**
    	\brief Stores the status.
    */
    string Status;
    /**
    	\brief Stores the created at utc.
    */
    string CreatedAtUtc;
    /**
    	\brief Stores the published at utc.
    */
    string PublishedAtUtc;
    /**
    	\brief Collection of items values.
    */
    ref array<ref ServerSentrySupportCaseItem> Items;
};

/**
	\brief Provides ServerSentry behavior for Support case item.
*/
class ServerSentrySupportCaseItem
{
    /**
    	\brief Stores the class name.
    */
    string ClassName;
    /**
    	\brief Stores the display name.
    */
    string DisplayName;
    /**
    	\brief Stores the quantity.
    */
    int Quantity;
    /**
    	\brief Collection of attachments values.
    */
    ref array<ref ServerSentrySupportCaseItem> Attachments;
};
