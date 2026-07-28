/**
	\file ServerSentryBreachingChargeEventData.c
	\brief Defines the Breaching charge event data logging data model.
*/

/**
	\brief Carries payload data for Breaching charge events.
*/
class ServerSentryBreachingChargeEventData
{
    /**
    	\brief Stores the source mod.
    */
    string sourceMod;
    /**
    	\brief Stores the breach action.
    */
    string breachAction;
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
    	\brief Stores the placing steam ID.
    */
    string placingSteamId;
    /**
    	\brief Stores the placing bohemia ID.
    */
    string placingBohemiaId;
    /**
    	\brief Stores the placing name.
    */
    string placingName;
    /**
    	\brief Stores the placing position.
    */
    ref ServerSentryVectorDto placingPosition;

    /**
    	\brief Stores the defusing steam ID.
    */
    string defusingSteamId;
    /**
    	\brief Stores the defusing bohemia ID.
    */
    string defusingBohemiaId;
    /**
    	\brief Stores the defusing name.
    */
    string defusingName;
    /**
    	\brief Stores the defusing position.
    */
    ref ServerSentryVectorDto defusingPosition;

    /**
    	\brief Stores the charge class name.
    */
    string chargeClassName;
    /**
    	\brief Stores the charge display name.
    */
    string chargeDisplayName;
    /**
    	\brief Stores the charge object ID.
    */
    string chargeObjectId;
    /**
    	\brief Stores the charge position.
    */
    ref ServerSentryVectorDto chargePosition;
    /**
    	\brief Stores the charge orientation.
    */
    ref ServerSentryVectorDto chargeOrientation;
    /**
    	\brief Stores the charge health.
    */
    float chargeHealth;
    /**
    	\brief Stores the charge state.
    */
    int chargeState;
    /**
    	\brief Stores the charge state name.
    */
    string chargeStateName;
    /**
    	\brief Stores the charge damage.
    */
    int chargeDamage;
    /**
    	\brief Stores the damage to objects radius.
    */
    float damageToObjectsRadius;
    /**
    	\brief Stores the damage to players radius.
    */
    float damageToPlayersRadius;
    /**
    	\brief Stores the min damage to players.
    */
    float minDamageToPlayers;
    /**
    	\brief Stores the max damage to players.
    */
    float maxDamageToPlayers;
    /**
    	\brief Stores the max damage to players radius.
    */
    float maxDamageToPlayersRadius;
    /**
    	\brief Stores the time to plant.
    */
    float timeToPlant;
    /**
    	\brief Stores the time to defuse.
    */
    float timeToDefuse;
    /**
    	\brief Stores the charge time.
    */
    float chargeTime;
    /**
    	\brief Indicates whether automatically arm.
    */
    bool automaticallyArm;
    /**
    	\brief Indicates whether delete objects directly.
    */
    bool deleteObjectsDirectly;
    /**
    	\brief Indicates whether only destroy locks.
    */
    bool onlyDestroyLocks;
    /**
    	\brief Indicates whether destroy other charges.
    */
    bool destroyOtherCharges;

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
    	\brief Stores the target object ID.
    */
    string targetObjectId;
    /**
    	\brief Stores the charge target type.
    */
    string chargeTargetType;
    /**
    	\brief Stores the target position.
    */
    ref ServerSentryVectorDto targetPosition;
    /**
    	\brief Stores the target orientation.
    */
    ref ServerSentryVectorDto targetOrientation;
    /**
    	\brief Stores the target health before.
    */
    float targetHealthBefore;
    /**
    	\brief Stores the target health after.
    */
    float targetHealthAfter;
    /**
    	\brief Indicates whether target health known.
    */
    bool targetHealthKnown;
    /**
    	\brief Indicates whether target destroyed.
    */
    bool targetDestroyed;

    /**
    	\brief Stores the affected player steam ID.
    */
    string affectedPlayerSteamId;
    /**
    	\brief Stores the affected player bohemia ID.
    */
    string affectedPlayerBohemiaId;
    /**
    	\brief Stores the affected player name.
    */
    string affectedPlayerName;
    /**
    	\brief Stores the affected player position.
    */
    ref ServerSentryVectorDto affectedPlayerPosition;
    /**
    	\brief Stores the player health before.
    */
    float playerHealthBefore;
    /**
    	\brief Stores the player health after.
    */
    float playerHealthAfter;
    /**
    	\brief Stores the player damage.
    */
    float playerDamage;
    /**
    	\brief Stores the distance to charge.
    */
    float distanceToCharge;

    /**
    	\brief Stores the radius object count.
    */
    int radiusObjectCount;
    /**
    	\brief Stores the radius player count.
    */
    int radiusPlayerCount;
    /**
    	\brief Stores the raw.
    */
    string raw;
    /**
    	\brief Stores the extra.
    */
    string extra;

    /**
    	\brief Initializes a new ServerSentryBreachingChargeEventData instance.
    */
    void ServerSentryBreachingChargeEventData()
    {
        sourceMod = "HDSN_BreachingCharge";
        breachAction = "";
        result = "success";
        reason = "";
        sourceHook = "";

        actorSteamId = "";
        actorBohemiaId = "";
        actorName = "";
        actorPosition = new ServerSentryVectorDto();

        placingSteamId = "";
        placingBohemiaId = "";
        placingName = "";
        placingPosition = new ServerSentryVectorDto();

        defusingSteamId = "";
        defusingBohemiaId = "";
        defusingName = "";
        defusingPosition = new ServerSentryVectorDto();

        chargeClassName = "";
        chargeDisplayName = "";
        chargeObjectId = "";
        chargePosition = new ServerSentryVectorDto();
        chargeOrientation = new ServerSentryVectorDto();
        chargeHealth = 0;
        chargeState = -1;
        chargeStateName = "";
        chargeDamage = 0;
        damageToObjectsRadius = 0;
        damageToPlayersRadius = 0;
        minDamageToPlayers = 0;
        maxDamageToPlayers = 0;
        maxDamageToPlayersRadius = 0;
        timeToPlant = 0;
        timeToDefuse = 0;
        chargeTime = 0;
        automaticallyArm = false;
        deleteObjectsDirectly = false;
        onlyDestroyLocks = false;
        destroyOtherCharges = false;

        targetType = "";
        targetClassName = "";
        targetDisplayName = "";
        targetObjectId = "";
        chargeTargetType = "";
        targetPosition = new ServerSentryVectorDto();
        targetOrientation = new ServerSentryVectorDto();
        targetHealthBefore = 0;
        targetHealthAfter = 0;
        targetHealthKnown = false;
        targetDestroyed = false;

        affectedPlayerSteamId = "";
        affectedPlayerBohemiaId = "";
        affectedPlayerName = "";
        affectedPlayerPosition = new ServerSentryVectorDto();
        playerHealthBefore = 0;
        playerHealthAfter = 0;
        playerDamage = 0;
        distanceToCharge = 0;

        radiusObjectCount = 0;
        radiusPlayerCount = 0;
        raw = "";
        extra = "";
    }
}
