/**
	\file ServerSentryLiveWorldObjectHooks.c
	\brief Connects ServerSentry to DayZ or third-party hooks for Live world object hooks.
*/

// Live-map object registration must not hook EntityAI here.
// EntityAI is defined in the Game module, while this file is loaded in World.
// Hook concrete World classes instead and let ServerSentryLiveApi filter by class name.
/**
	\brief Extends the DayZ Effect area class with ServerSentry integration.
*/
modded class EffectArea
{
    /**
    	\brief Processes the ee init operation.
    */
    override void EEInit()
    {
        super.EEInit();
        ServerSentryLiveApi.TryRegisterEntity(this);
    }

    /**
    	\brief Processes the ee delete operation.
    	\param parent \p parent used by the operation.
    */
    override void EEDelete(EntityAI parent)
    {
        ServerSentryLiveApi.UnregisterEntity(this);
        super.EEDelete(parent);
    }
}
