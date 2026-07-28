/**
	\file ServerSentryInspectObjectAction.c
	\brief Implements live-action support for Inspect object action.
*/

/**
	\brief Implements the Inspect object ServerSentry action.
*/
class ServerSentryInspectObjectAction: ServerSentryModActionHandler
{
    /**
    	\brief Returns the metadata descriptor published for this live action.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto d = new ServerSentryLiveModActionDescriptorDto();
        d.id = "object.inspect";
        d.targetType = "object";
        d.label = "Inspect";
        d.icon = "eye";
        d.description = "Logs the selected live object on the DayZ server.";
        return d;
    }

    /**
    	\brief Executes the live action using the supplied request context.
    	\param ctx \p action context containing request, target, and input data.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    override bool Execute(ServerSentryModActionContext ctx, out string message)
    {
        message = "Inspect logged.";
        Print("[ServerSentry][LiveAction][Inspect] requestId=" + ctx.requestId + " actionId=" + ctx.actionId + " targetType=" + ctx.targetType + " objectId=" + ctx.objectId + " steamId=" + ctx.steamId + " className=" + ctx.className + " input=" + ctx.inputJson);
        return true;
    }
}
