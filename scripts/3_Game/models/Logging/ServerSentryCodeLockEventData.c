/**
	\file ServerSentryCodeLockEventData.c
	\brief Defines the Code lock event data logging data model.
*/

/**
	\brief Carries payload data for Code lock events.
*/
class ServerSentryCodeLockEventData
{
    /**
    	\brief Stores the source mod.
    */
    string sourceMod;
    /**
    	\brief Stores the code lock action.
    */
    string codeLockAction;
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
    	\brief Type of target resolved by the action.
    */
    string targetType;
    /**
    	\brief Stores the target class name.
    */
    string targetClassName;
    /**
    	\brief Stores the target display name.
    */
    string targetDisplayName;
    /**
    	\brief Stores the target position.
    */
    ref ServerSentryVectorDto targetPosition;

    /**
    	\brief Stores the parent class name.
    */
    string parentClassName;
    /**
    	\brief Stores the parent display name.
    */
    string parentDisplayName;
    /**
    	\brief Stores the parent object ID.
    */
    string parentObjectId;
    /**
    	\brief Stores the parent position.
    */
    ref ServerSentryVectorDto parentPosition;

    /**
    	\brief Stores the lock class name.
    */
    string lockClassName;
    /**
    	\brief Stores the lock object ID.
    */
    string lockObjectId;
    /**
    	\brief Stores the lock position.
    */
    ref ServerSentryVectorDto lockPosition;
    /**
    	\brief Stores the lock health.
    */
    float lockHealth;
    /**
    	\brief Stores the lock max health.
    */
    float lockMaxHealth;
    /**
    	\brief Indicates whether is locked.
    */
    bool isLocked;
    /**
    	\brief Indicates whether lock state known.
    */
    bool lockStateKnown;

    /**
    	\brief Stores the owner bohemia ID.
    */
    string ownerBohemiaId;
    /**
    	\brief Indicates whether has owner.
    */
    bool hasOwner;
    /**
    	\brief Indicates whether actor is owner.
    */
    bool actorIsOwner;
    /**
    	\brief Indicates whether actor is guest.
    */
    bool actorIsGuest;
    /**
    	\brief Stores the guest count.
    */
    int guestCount;

    /**
    	\brief Stores the code lock log type.
    */
    string codeLockLogType;
    /**
    	\brief Stores the code lock access type.
    */
    string codeLockAccessType;
    /**
    	\brief Stores the widget name.
    */
    string widgetName;
    /**
    	\brief Indicates whether is admin.
    */
    bool isAdmin;

    /**
    	\brief Stores the raid damage.
    */
    float raidDamage;
    /**
    	\brief Stores the raid health.
    */
    float raidHealth;
    /**
    	\brief Indicates whether raid success.
    */
    bool raidSuccess;
    /**
    	\brief Indicates whether raid claim.
    */
    bool raidClaim;

    /**
    	\brief Stores the rpc type.
    */
    int rpcType;
    /**
    	\brief Stores the raw.
    */
    string raw;
    /**
    	\brief Stores the extra.
    */
    string extra;

    /**
    	\brief Initializes a new ServerSentryCodeLockEventData instance.
    */
    void ServerSentryCodeLockEventData()
    {
        sourceMod = "CodeLock";
        codeLockAction = "";
        result = "success";
        reason = "";
        sourceHook = "";

        actorSteamId = "";
        actorBohemiaId = "";
        actorName = "";
        actorPosition = new ServerSentryVectorDto();

        targetType = "";
        targetClassName = "";
        targetDisplayName = "";
        targetPosition = new ServerSentryVectorDto();

        parentClassName = "";
        parentDisplayName = "";
        parentObjectId = "";
        parentPosition = new ServerSentryVectorDto();

        lockClassName = "CodeLock";
        lockObjectId = "";
        lockPosition = new ServerSentryVectorDto();
        lockHealth = 0;
        lockMaxHealth = 0;
        isLocked = false;
        lockStateKnown = false;

        ownerBohemiaId = "";
        hasOwner = false;
        actorIsOwner = false;
        actorIsGuest = false;
        guestCount = 0;

        codeLockLogType = "";
        codeLockAccessType = "";
        widgetName = "";
        isAdmin = false;

        raidDamage = 0;
        raidHealth = 0;
        raidSuccess = false;
        raidClaim = false;

        rpcType = 0;
        raw = "";
        extra = "";
    }
}
