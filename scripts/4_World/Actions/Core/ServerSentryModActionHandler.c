/**
	\file ServerSentryModActionHandler.c
	\brief Implements live-action support for Mod action handler.
*/

/**
	\brief Defines the interface implemented by all ServerSentry live actions.
*/
class ServerSentryModActionHandler
{
    /**
    	\brief Returns the metadata descriptor published for this live action.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        return null;
    }

    /**
    	\brief Executes the live action using the supplied request context.
    	\param ctx \p action context containing request, target, and input data.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool Execute(ServerSentryModActionContext ctx, out string message)
    {
        message = "Action handler is not implemented.";
        return false;
    }
}

// Base for actions that require an online player.
/**
	\brief Resolves a player target before executing a player action.
*/
class ServerSentryPlayerModActionHandler: ServerSentryModActionHandler
{
    /**
    	\brief Executes the live action using the supplied request context.
    	\param ctx \p action context containing request, target, and input data.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    override bool Execute(ServerSentryModActionContext ctx, out string message)
    {
        if (!ctx)
        {
            message = "Missing action context.";
            return false;
        }

        PlayerBase player;
        if (!ctx.TryGetPlayer(player, message))
        {
            return false;
        }

        return ExecutePlayer(ctx, player, message);
    }

    /**
    	\brief Executes the player-specific action implementation.
    	\param ctx \p action context containing request, target, and input data.
    	\param player \p player entity associated with the operation.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool ExecutePlayer(ServerSentryModActionContext ctx, PlayerBase player, out string message)
    {
        message = "Player action handler is not implemented.";
        return false;
    }
}

// Optional base for actions that require a live object or vehicle target.
/**
	\brief Resolves a world-object target before executing an object action.
*/
class ServerSentryObjectModActionHandler: ServerSentryModActionHandler
{
    /**
    	\brief Executes the live action using the supplied request context.
    	\param ctx \p action context containing request, target, and input data.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    override bool Execute(ServerSentryModActionContext ctx, out string message)
    {
        if (!ctx)
        {
            message = "Missing action context.";
            return false;
        }

        EntityAI targetObject;
        if (!ctx.TryGetObject(targetObject, message))
        {
            return false;
        }

        return ExecuteObject(ctx, targetObject, message);
    }

    /**
    	\brief Executes the object-specific action implementation.
    	\param ctx \p action context containing request, target, and input data.
    	\param targetObject \p target object used by the operation.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool ExecuteObject(ServerSentryModActionContext ctx, EntityAI targetObject, out string message)
    {
        message = "Object action handler is not implemented.";
        return false;
    }
}
