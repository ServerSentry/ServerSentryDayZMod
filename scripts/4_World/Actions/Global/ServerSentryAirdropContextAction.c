/**
    \file ServerSentryAirdropContextAction.c
    \brief Provides the registered Live Map context-menu hook for airdrop integrations.
*/

/**
    \brief Publishes a visible "Spawn Airdrop" map action and forwards the selected
    terrain position to ExecuteAtPosition. Replace the no-op body with the actual
    airdrop implementation or override this class from an integration PBO.
*/
class ServerSentryAirdropContextAction: ServerSentryContextMenuModActionHandler
{
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        // Explicit field assignments are used for maximum DayZ Enforce compatibility.
        ServerSentryLiveModActionDescriptorDto descriptor = new ServerSentryLiveModActionDescriptorDto();
        descriptor.id = "world.airdrop.spawn";
        descriptor.targetType = "contextMenu";
        descriptor.label = "Spawn Airdrop";
        descriptor.icon = "Plane";
        descriptor.description = "Runs the airdrop hook at the position selected with the Live Map context menu.";
        descriptor.AddScope("live");
        descriptor.SetDangerous("Run the airdrop action at the selected map position?");
        return descriptor;
    }

    override bool ExecuteAtPosition(ServerSentryModActionContext ctx, vector targetPosition, out string message)
    {
        // Wiring-only default. Put the actual airdrop spawn call here, or override this
        // method from an integration PBO with `modded class ServerSentryAirdropContextAction`.
        Print("[ServerSentry][Airdrop] Context action received at " + targetPosition.ToString());
        message = "Airdrop hook received the map position " + targetPosition.ToString() + ". Add the spawn logic in ServerSentryAirdropContextAction.ExecuteAtPosition.";
        return true;
    }
}
