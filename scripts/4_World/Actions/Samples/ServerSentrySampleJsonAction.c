/**
	\file ServerSentrySampleJsonAction.c
	\brief Implements live-action support for Sample JSON action.
*/


/* class ServerSentrySampleJsonAction: ServerSentryModActionHandler
{
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto d = new ServerSentryLiveModActionDescriptorDto();
        d.id = "server.sample.json";
        d.targetType = "server";
        d.label = "Sample: Free JSON";
        d.icon = "FileJson";
        d.description = "Demonstrates the free JSON escape hatch for payloads that do not have a fixed DayZ DTO schema.";
        d.scopes.Insert("live");

        ServerSentryLiveModActionInputDto payload = ServerSentryLiveModActionInputFactory.Json( "payload", "Custom JSON payload", true, "{\"message\":\"hello\",\"values\":[1,true,null,{\"nested\":\"value\"}]}");
        payload.helpText = "The frontend validates JSON syntax. This sample only confirms that the full payload reached the mod.";
        d.inputs.Insert(payload);
        return d;
    }

    override bool Execute(ServerSentryModActionContext ctx, out string message)
    {
        if (!ctx || ctx.inputJson == "" || ctx.inputJson == "{}")
        {
            message = "No JSON input was received.";
            return false;
        }

        Print("[ServerSentry][SampleJson] input=" + ctx.inputJson);
        message = "Free JSON payload received by the mod (" + ctx.inputJson.Length().ToString() + " characters).";
        return true;
    }
}
*/
