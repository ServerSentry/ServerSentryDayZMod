class ServerSentryVehicleKickAction: ServerSentryVehicleModActionHandler
{
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        // Keep descriptor construction simple for the DayZ Enforce compiler.
        // Do not use a C#-style multiline factory call here.
        ServerSentryLiveModActionDescriptorDto descriptor = new ServerSentryLiveModActionDescriptorDto();
        descriptor.id = "vehicle.kick";
        descriptor.targetType = "vehicle";
        descriptor.label = "Kick out of vehicle";
        descriptor.icon = "LogOut";
        descriptor.description = "Kicks everyone from the selected vehicle.";
        descriptor.AddScope("live");
        return descriptor;
    }

    override bool ExecuteVehicle(ServerSentryModActionContext ctx, Transport vehicle, out string message)
    {
        if (!vehicle)
        {
            message = "No valid vehicle was selected.";
            return false;
        }

        array<PlayerBase> occupants = new array<PlayerBase>();

        for (int seatIndex = 0; seatIndex < vehicle.CrewSize(); seatIndex++)
        {
            PlayerBase player = PlayerBase.Cast(vehicle.CrewMember(seatIndex));

            if (player)
                occupants.Insert(player);
        }

        if (occupants.Count() == 0)
        {
            message = "The selected vehicle is empty.";
            return true;
        }

        int kickedCount = 0;

        foreach (PlayerBase occupant : occupants)
        {
            HumanCommandVehicle vehicleCommand = occupant.GetCommand_Vehicle();

            if (!vehicleCommand)
                continue;

            if (vehicleCommand.GetTransport() != vehicle)
                continue;
            
            GetGame().RPCSingleParam(occupant, ServerSentryRPCs.VEHICLE_KICK, new Param1<bool>(true), true, occupant.GetIdentity());
            vehicleCommand.GetOutVehicle();
            kickedCount++;
        }

        message = string.Format("Kicked %1 occupant(s) from the selected vehicle.",  kickedCount);
        return kickedCount > 0;
    }
}
