/**
	\file ServerSentryItemCountState.c
	\brief Defines ServerSentry support for Item count state.
*/

/**
	\brief Stores runtime state for Item count.
*/
class ServerSentryItemCountState
{
    /**
    	\brief Instance state for schema version.
    */
    int m_SchemaVersion;
    /**
    	\brief Instance state for generator.
    */
    string m_Generator;
    /**
    	\brief Instance state for total server items.
    */
    int m_TotalServerItems;
    /**
    	\brief Instance state for catalog schema version.
    */
    int m_CatalogSchemaVersion;
    /**
    	\brief Instance state for cfg vehicles children.
    */
    int m_CfgVehiclesChildren;
    /**
    	\brief Instance state for cfg weapons children.
    */
    int m_CfgWeaponsChildren;
    /**
    	\brief Instance state for cfg magazines children.
    */
    int m_CfgMagazinesChildren;
    /**
    	\brief Instance state for cfg slots children.
    */
    int m_CfgSlotsChildren;
    /**
    	\brief Instance state for cfg patches children.
    */
    int m_CfgPatchesChildren;

    /**
    	\brief Initializes a new ServerSentryItemCountState instance.
    */
    void ServerSentryItemCountState()
    {
        m_SchemaVersion = 6;
        m_Generator = "Server Sentry";
        m_TotalServerItems = 0;
        m_CatalogSchemaVersion = 0;
        m_CfgVehiclesChildren = 0;
        m_CfgWeaponsChildren = 0;
        m_CfgMagazinesChildren = 0;
        m_CfgSlotsChildren = 0;
        m_CfgPatchesChildren = 0;
    }
};
