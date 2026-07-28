/**
	\file ServerSentryBaseBuildingActions.c
	\brief Connects ServerSentry to DayZ or third-party hooks for Base building actions.
*/

/**
	\brief Extends the DayZ Action build part class with ServerSentry integration.
*/
modded class ActionBuildPart
{
    /**
    	\brief Handles the DayZ Finish progress server callback.
    	\param action_data \p action data used by the operation.
    */
    override void OnFinishProgressServer(ActionData action_data)
    {
        if(action_data && ServerSentryConfigManager.Get().IsAllowedToLogBaseBuilding())
        {
            PlayerBase ssPlayer = action_data.m_Player;
            ServerSentryEvent ssEvent = ServerSentryBuildActionLifecycle.CreateBaseBuildingSnapshot(ssPlayer, action_data, "basebuilding_build_part", "Build basebuilding part", "ActionBuildPart", "build_part", "ActionBuildPart.OnFinishProgressServer");
            ServerSentryBuildActionLifecycle.EmitSnapshot(ssEvent);
        }

        super.OnFinishProgressServer(action_data);
    }
};

/**
	\brief Extends the DayZ Action dismantle part class with ServerSentry integration.
*/
modded class ActionDismantlePart
{
    /**
    	\brief Handles the DayZ Finish progress server callback.
    	\param action_data \p action data used by the operation.
    */
    override void OnFinishProgressServer(ActionData action_data)
    {
        if (action_data && ServerSentryConfigManager.Get().IsAllowedToLogDismantle())
        {
            PlayerBase ssPlayer = action_data.m_Player;
            ServerSentryEvent ssEvent = ServerSentryBuildActionLifecycle.CreateBaseBuildingSnapshot(ssPlayer, action_data, "basebuilding_dismantle_part", "Dismantle basebuilding part", "ActionDismantlePart", "dismantle_part", "ActionDismantlePart.OnFinishProgressServer");
            ServerSentryBuildActionLifecycle.EmitSnapshot(ssEvent);
        }

        super.OnFinishProgressServer(action_data);
    }
};

/**
	\brief Extends the DayZ Action destroy part class with ServerSentry integration.
*/
modded class ActionDestroyPart
{
    /**
    	\brief Handles the DayZ Finish progress server callback.
    	\param action_data \p action data used by the operation.
    */
    override void OnFinishProgressServer(ActionData action_data)
    {
        if (action_data && ServerSentryConfigManager.Get().IsAllowedToLogDestroy())
        {
            PlayerBase ssPlayer = action_data.m_Player;
            ServerSentryEvent ssEvent = ServerSentryBuildActionLifecycle.CreateBaseBuildingSnapshot(ssPlayer, action_data, "basebuilding_destroy_part", "Destroy basebuilding part", "ActionDestroyPart", "destroy_part", "ActionDestroyPart.OnFinishProgressServer");
            ServerSentryBuildActionLifecycle.EmitSnapshot(ssEvent);
        }

        super.OnFinishProgressServer(action_data);
    }
};
