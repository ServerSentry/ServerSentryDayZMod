/**
	\file ServerSentryPlacementActions.c
	\brief Connects ServerSentry to DayZ or third-party hooks for Placement actions.
*/

/**
	\brief Extends the DayZ Action deploy object class with ServerSentry integration.
*/
modded class ActionDeployObject
{
    /**
    	\brief Handles the DayZ End server callback.
    	\param action_data \p action data used by the operation.
    */
    override void OnEndServer(ActionData action_data)
    {
        ServerSentryEvent ssEvent;

        if (action_data && action_data.m_Player && action_data.m_MainItem)
        {
            PlaceObjectActionData poActionData = PlaceObjectActionData.Cast(action_data);
            if (poActionData && poActionData.m_AlreadyPlaced)
            {
                EntityAI placedEntity = EntityAI.Cast(action_data.m_MainItem);
                vector position = poActionData.m_Position;
                vector orientation = poActionData.m_Orientation;

                ssEvent = ServerSentryBuildActionLifecycle.CreatePlacementSnapshot(action_data.m_Player, action_data, placedEntity, position, orientation, "ActionDeployObject", "ActionDeployObject.OnEndServer");
            }
        }

        super.OnEndServer(action_data);
        ServerSentryBuildActionLifecycle.EmitSnapshot(ssEvent);
    }
};
