/**
	\file ServerSentryItemMoveData.c
	\brief Defines the Item move data logging data model.
*/

/**
	\brief Provides ServerSentry behavior for Item move data.
*/
class ServerSentryItemMoveData
{
    /**
    	\brief Stores the move ID.
    */
    string moveId;
    /**
    	\brief Stores the source hook.
    */
    string sourceHook;
    /**
    	\brief Stores the item kind.
    */
    string itemKind;
    /**
    	\brief Stores the item class name.
    */
    string itemClassName;
    /**
    	\brief Stores the item display name.
    */
    string itemDisplayName;
    /**
    	\brief Stores the relation.
    */
    string relation;
    /**
    	\brief Stores the actor inference.
    */
    string actorInference;
    /**
    	\brief Indicates whether counts for player item stats.
    */
    bool countsForPlayerItemStats;

    // Flattened storage summary for frontend/admin tables.
    // The detailed source/destination data is still available in from/to.
    /**
    	\brief Stores the source storage class name.
    */
    string sourceStorageClassName;
    /**
    	\brief Stores the source storage name.
    */
    string sourceStorageName;
    /**
    	\brief Stores the source storage raw.
    */
    string sourceStorageRaw;
    /**
    	\brief Stores the destination storage class name.
    */
    string destinationStorageClassName;
    /**
    	\brief Stores the destination storage name.
    */
    string destinationStorageName;
    /**
    	\brief Stores the destination storage raw.
    */
    string destinationStorageRaw;
    /**
    	\brief Stores the from label.
    */
    string fromLabel;
    /**
    	\brief Stores the to label.
    */
    string toLabel;

    /**
    	\brief Stores the from.
    */
    ref ServerSentryItemLocationData from;
    /**
    	\brief Stores the to.
    */
    ref ServerSentryItemLocationData to;

    /**
    	\brief Initializes a new ServerSentryItemMoveData instance.
    */
    void ServerSentryItemMoveData()
    {
        moveId = "";
        sourceHook = "";
        itemKind = "unknown";
        itemClassName = "";
        itemDisplayName = "";
        relation = "unknown_to_unknown";
        actorInference = "unknown";
        countsForPlayerItemStats = false;

        sourceStorageClassName = "";
        sourceStorageName = "";
        sourceStorageRaw = "";
        destinationStorageClassName = "";
        destinationStorageName = "";
        destinationStorageRaw = "";
        fromLabel = "";
        toLabel = "";

        from = new ServerSentryItemLocationData();
        to = new ServerSentryItemLocationData();
    }
}
