/**
    \file ServerSentrySampleTypedWorldObjectActions.c
    \brief Examples for actions that only appear on a specific Live Map object type.

    These samples are not registered by default. Register your own handlers in
    ServerSentryModActionBootstrap.RegisterExtensionActions().
*/

/**
    \brief Example FlagPole-only action. Replace the body with your flag raising logic.
*/
class ServerSentrySampleRaiseFlagAction: ServerSentryFlagPoleModActionHandler
{
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        // Explicit assignments are intentionally used for DayZ compatibility.
        ServerSentryLiveModActionDescriptorDto descriptor = new ServerSentryLiveModActionDescriptorDto();
        descriptor.id = "flagpole.raise";
        descriptor.targetType = "base_flagpole";
        descriptor.label = "Flagge hochziehen";
        descriptor.icon = "Flag";
        descriptor.description = "Runs only for a TerritoryFlag or FlagPole selected on the Live Map.";
        descriptor.AddScope("live");
        return descriptor;
    }

    override bool ExecuteFlagPole(ServerSentryModActionContext ctx, EntityAI flagPole, out string message)
    {
        message = "Sample only: implement flag raising behavior.";
        return false;
    }
}
