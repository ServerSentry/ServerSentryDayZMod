/**
	\file ServerSentryDeleteObjectAction.c
	\brief Implements live-action support for Delete object action.
*/

/**
	\brief Implements the Delete object ServerSentry action.
*/
class ServerSentryDeleteObjectAction: ServerSentryModActionHandler
{
    /**
    	\brief Returns the metadata descriptor published for this live action.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto d = new ServerSentryLiveModActionDescriptorDto();
        d.id = "object.delete";
        d.targetType = "object";
        d.label = "Delete";
        d.icon = "trash";
        d.dangerous = true;
        d.confirmText = "Delete this object on the server?";
        d.description = "Deletes a whitelisted live object resolved by the mod.";
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
        EntityAI targetObject = ServerSentryLiveActionResolver.FindObject(ctx);
        if (!targetObject)
        {
            message = "Object not found.";
            return false;
        }

        if (PlayerBase.Cast(targetObject))
        {
            message = "Refusing to delete players through object.delete.";
            return false;
        }

        string objectLabel = targetObject.ToString();
        GetGame().ObjectDelete(targetObject);
        message = "Deleted object " + objectLabel + ".";
        return true;
    }
}
