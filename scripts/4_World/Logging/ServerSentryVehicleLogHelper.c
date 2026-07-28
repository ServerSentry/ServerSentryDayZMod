/**
	\file ServerSentryVehicleLogHelper.c
	\brief Provides logging support for Vehicle log helper.
*/

/**
	\brief Provides helper operations for Vehicle log.
*/
class ServerSentryVehicleLogHelper
{
    /**
    	\brief Creates event.
    	\param vehicle \p vehicle used by the operation.
    	\param vehicleAction \p vehicle action used by the operation.
    	\param player \p player entity associated with the operation.
    	\param seatIndex \p seat index used by the operation.
    	\param sourceHook \p source hook used by the operation.
    	\return \p The resulting vehicle event instance, or null when unavailable.
    */
    static ServerSentryVehicleEvent CreateEvent(Transport vehicle, string vehicleAction, PlayerBase player = null, int seatIndex = -1, string sourceHook = "")
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return null;
        }

        if (!ServerSentryDefaults.LogVehicleEvents())
        {
            return null;
        }

        if (!vehicle)
        {
            return null;
        }

        ServerSentryVehicleEventData data = BuildData(vehicle, vehicleAction, player, seatIndex, sourceHook);
        return ServerSentryEventFactory.CreateVehicleEvent(data);
    }

    /**
    	\brief Creates event with engine state.
    	\param vehicle \p vehicle used by the operation.
    	\param vehicleAction \p vehicle action used by the operation.
    	\param engineOn \p true to enable engine on.
    	\param player \p player entity associated with the operation.
    	\param seatIndex \p seat index used by the operation.
    	\param sourceHook \p source hook used by the operation.
    	\return \p The resulting vehicle event instance, or null when unavailable.
    */
    static ServerSentryVehicleEvent CreateEventWithEngineState(Transport vehicle, string vehicleAction, bool engineOn, PlayerBase player = null, int seatIndex = -1, string sourceHook = "")
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return null;
        }

        if (!ServerSentryDefaults.LogVehicleEvents())
        {
            return null;
        }

        if (!vehicle)
        {
            return null;
        }

        ServerSentryVehicleEventData data = BuildData(vehicle, vehicleAction, player, seatIndex, sourceHook);
        data.engineStateKnown = true;
        data.engineOn = engineOn;
        return ServerSentryEventFactory.CreateVehicleEvent(data);
    }

    /**
    	\brief Emits event.
    	\param ev \p ev used by the operation.
    */
    static void EmitEvent(ServerSentryVehicleEvent ev)
    {
        if (!ev)
        {
            return;
        }

        ServerSentryLogger.Get().EmitVehicleEvent(ev);
    }

    /**
    	\brief Emits the requested operation.
    	\param vehicle \p vehicle used by the operation.
    	\param vehicleAction \p vehicle action used by the operation.
    	\param player \p player entity associated with the operation.
    	\param seatIndex \p seat index used by the operation.
    	\param sourceHook \p source hook used by the operation.
    */
    static void Emit(Transport vehicle, string vehicleAction, PlayerBase player = null, int seatIndex = -1, string sourceHook = "")
    {
        EmitEvent(CreateEvent(vehicle, vehicleAction, player, seatIndex, sourceHook));
    }

    /**
    	\brief Emits with engine state.
    	\param vehicle \p vehicle used by the operation.
    	\param vehicleAction \p vehicle action used by the operation.
    	\param engineOn \p true to enable engine on.
    	\param player \p player entity associated with the operation.
    	\param seatIndex \p seat index used by the operation.
    	\param sourceHook \p source hook used by the operation.
    */
    static void EmitWithEngineState(Transport vehicle, string vehicleAction, bool engineOn, PlayerBase player = null, int seatIndex = -1, string sourceHook = "")
    {
        EmitEvent(CreateEventWithEngineState(vehicle, vehicleAction, engineOn, player, seatIndex, sourceHook));
    }

    /**
    	\brief Builds data.
    	\param vehicle \p vehicle used by the operation.
    	\param vehicleAction \p vehicle action used by the operation.
    	\param player \p player entity associated with the operation.
    	\param seatIndex \p seat index used by the operation.
    	\param sourceHook \p source hook used by the operation.
    	\return \p The resulting vehicle event data instance, or null when unavailable.
    */
    static ServerSentryVehicleEventData BuildData(Transport vehicle, string vehicleAction, PlayerBase player = null, int seatIndex = -1, string sourceHook = "")
    {
        ServerSentryVehicleEventData data = new ServerSentryVehicleEventData();
        data.sourceMod = "ServerSentry";
        data.vehicleAction = vehicleAction;
        data.result = "success";
        data.sourceHook = sourceHook;
        data.seatIndex = seatIndex;
        data.seatName = ResolveSeatName(seatIndex);

        FillVehicle(data, vehicle, vehicleAction);
        FillActor(data, player);
        return data;
    }

    /**
    	\brief Processes the fill vehicle operation.
    	\param data \p data processed by the operation.
    	\param vehicle \p vehicle used by the operation.
    	\param vehicleAction \p vehicle action used by the operation.
    */
    protected static void FillVehicle(ServerSentryVehicleEventData data, Transport vehicle, string vehicleAction = "")
    {
        if (!data || !vehicle)
        {
            return;
        }

        data.vehicleClassName = vehicle.GetType();
        data.vehicleDisplayName = vehicle.GetDisplayName();
        data.vehicleObjectId = vehicle.ToString();
        data.vehiclePosition.FromVector(vehicle.GetPosition());
        data.vehicleOrientation.FromVector(vehicle.GetOrientation());

        // Delete hooks can be called while the engine object is being torn down.
        // Keep delete snapshots primitive and avoid extra live reads.
        if (vehicleAction == "delete")
        {
            data.vehicleHealth = -1;
            data.crewSize = -1;
            return;
        }

        data.vehicleHealth = vehicle.GetHealth("", "Health");
        data.crewSize = CountCrew(vehicle);
    }

    /**
    	\brief Processes the count crew operation.
    	\param vehicle \p vehicle used by the operation.
    	\return \p The resulting integer value or status code.
    */
    protected static int CountCrew(Transport vehicle)
    {
        if (!vehicle)
        {
            return 0;
        }

        int count = 0;
        int size = vehicle.CrewSize();
        for (int i = 0; i < size; i++)
        {
            if (vehicle.CrewMember(i))
            {
                count++;
            }
        }

        return count;
    }

    /**
    	\brief Processes the fill actor operation.
    	\param data \p data processed by the operation.
    	\param player \p player entity associated with the operation.
    */
    protected static void FillActor(ServerSentryVehicleEventData data, PlayerBase player)
    {
        if (!data || !player)
        {
            return;
        }

        PlayerIdentity identity = player.GetIdentity();
        if (identity)
        {
            data.actorSteamId = identity.GetPlainId();
            data.actorBohemiaId = identity.GetId();
            data.actorName = identity.GetName();
        }

        data.actorPosition.FromVector(player.GetPosition());
    }

    /**
    	\brief Returns driver.
    	\param vehicle \p vehicle used by the operation.
    	\return \p The resulting player base instance, or null when unavailable.
    */
    static PlayerBase GetDriver(Transport vehicle)
    {
        if (!vehicle)
        {
            return null;
        }

        return PlayerBase.Cast(vehicle.CrewMember(DayZPlayerConstants.VEHICLESEAT_DRIVER));
    }

    /**
    	\brief Resolves seat name.
    	\param seatIndex \p seat index used by the operation.
    	\return \p The resulting string value.
    */
    static string ResolveSeatName(int seatIndex)
    {
        if (seatIndex == DayZPlayerConstants.VEHICLESEAT_DRIVER)
        {
            return "driver";
        }

        if (seatIndex == DayZPlayerConstants.VEHICLESEAT_CODRIVER)
        {
            return "codriver";
        }

        if (seatIndex >= 0)
        {
            return "passenger_" + seatIndex.ToString();
        }

        return "unknown";
    }
}
