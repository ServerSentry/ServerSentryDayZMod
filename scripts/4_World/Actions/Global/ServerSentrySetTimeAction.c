/**
	\file ServerSentrySetTimeAction.c
	\brief Implements live-action support for Set time action.
*/

/**
	\brief Implements the Set time ServerSentry action.
*/
class ServerSentrySetTimeAction: ServerSentryModActionHandler
{
    /**
    	\brief Returns the metadata descriptor published for this live action.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto d = new ServerSentryLiveModActionDescriptorDto();
        d.id = "server.time.set";
        d.targetType = "server";
        d.label = "Set Time";
        d.icon = "Clock";
        d.description = "Changes the current in-game server time.";
        d.scopes.Insert("live");

        ServerSentryLiveModActionInputDto hour = ServerSentryLiveModActionInputFactory.Number("hour", "Hour", true, 0, 23, 1, "12");
        hour.helpText = "24-hour in-game clock hour from 0 to 23.";
        d.inputs.Insert(hour);

        ServerSentryLiveModActionInputDto minute = ServerSentryLiveModActionInputFactory.Number("minute", "Minute", true, 0, 59, 1, "0");
        minute.helpText = "In-game clock minute from 0 to 59.";
        d.inputs.Insert(minute);

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
        int year;
        int month;
        int day;
        int currentHour;
        int currentMinute;
        GetGame().GetWorld().GetDate(year, month, day, currentHour, currentMinute);

        int hour = ServerSentryLiveActionInput.GetInt(ctx.inputJson, "hour", currentHour);
        int minute = ServerSentryLiveActionInput.GetInt(ctx.inputJson, "minute", 0);

        if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
        {
            message = "Invalid time. Hour must be 0-23 and minute must be 0-59.";
            return false;
        }

        GetGame().GetWorld().SetDate(year, month, day, hour, minute);

        message = "Changed in-game time to " + hour.ToString() + ":" + ServerSentrySetTimeAction.PadTwoDigits(minute) + ".";
        ServerSentryLiveApi.WriteSnapshot();
        return true;
    }

    /**
    	\brief Processes the pad two digits operation.
    	\param value \p value to process or assign.
    	\return \p The resulting string value.
    */
    protected static string PadTwoDigits(int value)
    {
        if (value < 10)
        {
            return "0" + value.ToString();
        }

        return value.ToString();
    }
}
