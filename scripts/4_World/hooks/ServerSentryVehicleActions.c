/**
	\file ServerSentryVehicleActions.c
	\brief Connects ServerSentry to DayZ or third-party hooks for Vehicle actions.
*/

/**
	\brief Extends the DayZ Action get in transport class with ServerSentry integration.
*/
modded class ActionGetInTransport
{
    /**
    	\brief Handles the DayZ End server callback.
    	\param action_data \p action data used by the operation.
    */
    override void OnEndServer(ActionData action_data)
    {
        super.OnEndServer(action_data);

        if (!action_data || !action_data.m_Player)
        {
            return;
        }

        PlayerBase player = PlayerBase.Cast(action_data.m_Player);
        Transport vehicle;
        int seatIndex = -1;

        HumanCommandVehicle vehicleCommand = action_data.m_Player.GetCommand_Vehicle();
        if (vehicleCommand)
        {
            vehicle = vehicleCommand.GetTransport();
            seatIndex = vehicleCommand.GetVehicleSeat();
        }

        if (!vehicle && action_data.m_Target)
        {
            vehicle = Transport.Cast(action_data.m_Target.GetObject());
            if (vehicle)
            {
                seatIndex = vehicle.CrewPositionIndex(action_data.m_Target.GetComponentIndex());
            }
        }

        if (vehicle)
        {
            ServerSentryVehicleLogHelper.Emit(vehicle, "enter", player, seatIndex, "ActionGetInTransport.OnEndServer");
        }
    }
}

/**
	\brief Extends the DayZ Action get out transport class with ServerSentry integration.
*/
modded class ActionGetOutTransport
{
    /**
    	\brief Handles the DayZ Start server callback.
    	\param action_data \p action data used by the operation.
    */
    override void OnStartServer(ActionData action_data)
    {
        PlayerBase player;
        Transport vehicle;
        int seatIndex = -1;

        if (action_data && action_data.m_Player)
        {
            player = PlayerBase.Cast(action_data.m_Player);

            HumanCommandVehicle vehicleCommand = action_data.m_Player.GetCommand_Vehicle();
            if (vehicleCommand)
            {
                vehicle = vehicleCommand.GetTransport();
                seatIndex = vehicleCommand.GetVehicleSeat();
            }
        }

        ServerSentryVehicleEvent ssEvent = ServerSentryVehicleLogHelper.CreateEvent(vehicle, "exit", player, seatIndex, "ActionGetOutTransport.OnStartServer");

        super.OnStartServer(action_data);

        ServerSentryVehicleLogHelper.EmitEvent(ssEvent);
    }
}
