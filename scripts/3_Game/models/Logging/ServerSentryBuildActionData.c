/**
	\file ServerSentryBuildActionData.c
	\brief Defines the Build action data logging data model.
*/

/**
	\brief Provides ServerSentry behavior for Build action data.
*/
class ServerSentryBuildActionData
{
    /**
    	\brief Stores the action class.
    */
    string actionClass;
    /**
    	\brief Stores the action kind.
    */
    string actionKind;
    /**
    	\brief Stores the action name.
    */
    string actionName;
    /**
    	\brief Stores the source hook.
    */
    string sourceHook;

    /**
    	\brief Stores the target class name.
    */
    string targetClassName;
    /**
    	\brief Stores the target name.
    */
    string targetName;
    /**
    	\brief Type of target resolved by the action.
    */
    string targetType;
    /**
    	\brief Stores the target raw.
    */
    string targetRaw;
    /**
    	\brief Stores the target ID.
    */
    int targetId;
    /**
    	\brief Stores the target component index.
    */
    int targetComponentIndex;
    /**
    	\brief Stores the target position.
    */
    ref ServerSentryVectorDto targetPosition;

    /**
    	\brief Stores the main item class name.
    */
    string mainItemClassName;
    /**
    	\brief Stores the main item display name.
    */
    string mainItemDisplayName;
    /**
    	\brief Stores the main item health.
    */
    float mainItemHealth;
    /**
    	\brief Stores the main item quantity.
    */
    float mainItemQuantity;

    /**
    	\brief Stores the placed class name.
    */
    string placedClassName;
    /**
    	\brief Stores the placed display name.
    */
    string placedDisplayName;
    /**
    	\brief Indicates whether is placement.
    */
    bool isPlacement;
    /**
    	\brief Indicates whether is base building.
    */
    bool isBaseBuilding;
    /**
    	\brief Indicates whether is base building kit.
    */
    bool isBaseBuildingKit;
    /**
    	\brief Stores the placement position.
    */
    ref ServerSentryVectorDto placementPosition;
    /**
    	\brief Stores the placement orientation.
    */
    ref ServerSentryVectorDto placementOrientation;

    /**
    	\brief Stores the relation.
    */
    string relation;
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
    	\brief Initializes a new ServerSentryBuildActionData instance.
    */
    void ServerSentryBuildActionData()
    {
        actionClass = "";
        actionKind = "";
        actionName = "";
        sourceHook = "";

        targetClassName = "";
        targetName = "";
        targetType = "";
        targetRaw = "";
        targetId = 0;
        targetComponentIndex = -1;
        targetPosition = new ServerSentryVectorDto();

        mainItemClassName = "";
        mainItemDisplayName = "";
        mainItemHealth = 0;
        mainItemQuantity = 0;

        placedClassName = "";
        placedDisplayName = "";
        isPlacement = false;
        isBaseBuilding = false;
        isBaseBuildingKit = false;
        placementPosition = new ServerSentryVectorDto();
        placementOrientation = new ServerSentryVectorDto();

        relation = "";
        actorSteamId = "";
        actorBohemiaId = "";
        actorName = "";
        actorPosition = new ServerSentryVectorDto();
    }
}
