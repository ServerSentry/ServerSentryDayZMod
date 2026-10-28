/**
	\file CarScript.c
	\brief Connects ServerSentry to DayZ or third-party hooks for Car script.
*/

/**
	\brief Extends the DayZ Car script class with ServerSentry integration.
*/
#ifdef SERVER
modded class MotorbikeScript
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

        if (ServerSentryConfigManager.Get().IsAllowedToLogVehicleDeletion())
        {
            ServerSentryVehicleLogHelper.Emit(this, "delete", null, -1, "MotorbikeScript.EEDelete");
        }

        super.EEDelete(parent);
    }

    /**
    	\brief Handles the DayZ Engine start callback.
    */
    override void OnEngineStart()
    {
        super.OnEngineStart();

        if (ServerSentryConfigManager.Get().IsAllowedToLogEngineStart())
        {
            PlayerBase driver = ServerSentryVehicleLogHelper.GetDriver(this);
            ServerSentryVehicleLogHelper.EmitWithEngineState(this, "engine_on", true, driver, DayZPlayerConstants.VEHICLESEAT_DRIVER, "MotorbikeScript.OnEngineStart");
        }
    }

    /**
    	\brief Handles the DayZ Engine stop callback.
    */
    override void OnEngineStop()
    {
        super.OnEngineStop();

        if (ServerSentryConfigManager.Get().IsAllowedToLogEngineStop())
        {
            PlayerBase driver = ServerSentryVehicleLogHelper.GetDriver(this);
            ServerSentryVehicleLogHelper.EmitWithEngineState(this, "engine_off", false, driver, DayZPlayerConstants.VEHICLESEAT_DRIVER, "MotorbikeScript.OnEngineStop");
        }
    }
}
#endif
