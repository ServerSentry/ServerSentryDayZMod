/**
    \file ServerSentrySampleFlagPoleDetails.c
    \brief Example provider for adding and overriding FlagPole Live Map details.

    This sample is not registered by default.
*/

/**
    \brief Example integration that adds a group name and overrides the Vanilla flag label.
*/
class ServerSentrySampleFlagPoleDetails: ServerSentryLiveBaseDetailProvider
{
    override bool Supports(EntityAI entity, ServerSentryLiveBaseDto dto)
    {
        return TerritoryFlag.Cast(entity) != null;
    }

    override void ApplyDetails(EntityAI entity, ServerSentryLiveBaseDto dto)
    {
        ServerSentryLiveDetails.UpsertBaseDetail(dto, "groupName", "Group", "Example Group");
        ServerSentryLiveDetails.UpsertBaseDetail(dto, "flagHeightPercent", "Flag status", "Managed by example mod");
    }
}

/**
    \brief Example bootstrap hook used by an integration PBO.
*/
/*
modded class ServerSentryLiveBaseDetailBootstrap
{
    override void RegisterExtensionProviders()
    {
        super.RegisterExtensionProviders();
        ServerSentryLiveBaseDetailRegistry.Register(new ServerSentrySampleFlagPoleDetails());
    }
}
*/
