/**
	\file ServerSentryGroupEventData.c
	\brief Defines the Group event data logging data model.
*/

/**
	\brief Carries payload data for Group events.
*/
class ServerSentryGroupEventData
{
    /**
    	\brief Stores the source mod.
    */
    string sourceMod;
    /**
    	\brief Stores the group action.
    */
    string groupAction;
    /**
    	\brief Stores the result.
    */
    string result;
    /**
    	\brief Stores the reason.
    */
    string reason;
    /**
    	\brief Stores the source hook.
    */
    string sourceHook;

    /**
    	\brief Stores the actor steam ID.
    */
    string actorSteamId;
    /**
    	\brief Stores the actor bohemia ID.
    */
    string actorBohemiaId;
    /**
    	\brief Stores the actor name.
    */
    string actorName;
    /**
    	\brief Stores the actor position.
    */
    ref ServerSentryVectorDto actorPosition;

    /**
    	\brief Stores the initiator steam ID.
    */
    string initiatorSteamId;
    /**
    	\brief Stores the initiator bohemia ID.
    */
    string initiatorBohemiaId;
    /**
    	\brief Stores the initiator name.
    */
    string initiatorName;

    /**
    	\brief Stores the group name.
    */
    string groupName;
    /**
    	\brief Stores the group tag.
    */
    string groupTag;
    /**
    	\brief Stores the group hash.
    */
    int groupHash;
    /**
    	\brief Stores the group level.
    */
    int groupLevel;
    /**
    	\brief Stores the member count.
    */
    int memberCount;

    /**
    	\brief Stores the member steam ID.
    */
    string memberSteamId;
    /**
    	\brief Stores the member bohemia ID.
    */
    string memberBohemiaId;
    /**
    	\brief Stores the member name.
    */
    string memberName;
    /**
    	\brief Stores the member UID.
    */
    int memberUid;
    /**
    	\brief Stores the permission group.
    */
    int permissionGroup;
    /**
    	\brief Indicates whether is temporary member.
    */
    bool isTemporaryMember;

    /**
    	\brief Stores the from subgroup index.
    */
    int fromSubgroupIndex;
    /**
    	\brief Stores the from subgroup name.
    */
    string fromSubgroupName;
    /**
    	\brief Stores the to subgroup index.
    */
    int toSubgroupIndex;
    /**
    	\brief Stores the to subgroup name.
    */
    string toSubgroupName;

    /**
    	\brief Stores the raw.
    */
    string raw;
    /**
    	\brief Stores the extra.
    */
    string extra;

    /**
    	\brief Initializes a new ServerSentryGroupEventData instance.
    */
    void ServerSentryGroupEventData()
    {
        sourceMod = "group";
        groupAction = "";
        result = "success";
        reason = "";
        sourceHook = "";

        actorSteamId = "";
        actorBohemiaId = "";
        actorName = "";
        actorPosition = new ServerSentryVectorDto();

        initiatorSteamId = "";
        initiatorBohemiaId = "";
        initiatorName = "";

        groupName = "";
        groupTag = "";
        groupHash = 0;
        groupLevel = 0;
        memberCount = 0;

        memberSteamId = "";
        memberBohemiaId = "";
        memberName = "";
        memberUid = -1;
        permissionGroup = -1;
        isTemporaryMember = false;

        fromSubgroupIndex = -1;
        fromSubgroupName = "";
        toSubgroupIndex = -1;
        toSubgroupName = "";

        raw = "";
        extra = "";
    }
}
