/**
	\file ServerSentryItemLocationData.c
	\brief Defines the Item location data logging data model.
*/

/**
	\brief Provides ServerSentry behavior for Item location data.
*/
class ServerSentryItemLocationData
{
    /**
    	\brief Input or event type exposed to external consumers.
    */
    string type;
    /**
    	\brief Stores the type ID.
    */
    int typeId;

    /**
    	\brief Stores the parent class name.
    */
    string parentClassName;
    /**
    	\brief Stores the parent name.
    */
    string parentName;
    /**
    	\brief Stores the parent owner type.
    */
    string parentOwnerType;
    /**
    	\brief Stores the parent owner kind.
    */
    string parentOwnerKind;
    /**
    	\brief Stores the parent owner class name.
    */
    string parentOwnerClassName;
    /**
    	\brief Stores the parent owner steam ID.
    */
    string parentOwnerSteamId;
    /**
    	\brief Stores the parent owner bohemia ID.
    */
    string parentOwnerBohemiaId;
    /**
    	\brief Stores the parent owner name.
    */
    string parentOwnerName;

    /**
    	\brief Stores the slot ID.
    */
    int slotId;
    /**
    	\brief Stores the slot name.
    */
    string slotName;
    /**
    	\brief Stores the slot display name.
    */
    string slotDisplayName;

    /**
    	\brief Stores the cargo idx.
    */
    int cargoIdx;
    /**
    	\brief Stores the row.
    */
    int row;
    /**
    	\brief Stores the col.
    */
    int col;
    /**
    	\brief Indicates whether flip.
    */
    bool flip;

    /**
    	\brief Stores the position.
    */
    ref ServerSentryVectorDto position;
    /**
    	\brief Stores the raw.
    */
    string raw;

    /**
    	\brief Initializes a new ServerSentryItemLocationData instance.
    */
    void ServerSentryItemLocationData()
    {
        type = "unknown";
        typeId = -1;

        parentClassName = "";
        parentName = "";
        parentOwnerType = "";
        parentOwnerKind = "";
        parentOwnerClassName = "";
        parentOwnerSteamId = "";
        parentOwnerBohemiaId = "";
        parentOwnerName = "";

        slotId = -1;
        slotName = "";
        slotDisplayName = "";

        cargoIdx = -1;
        row = -1;
        col = -1;
        flip = false;

        position = new ServerSentryVectorDto();
        raw = "";
    }
}
