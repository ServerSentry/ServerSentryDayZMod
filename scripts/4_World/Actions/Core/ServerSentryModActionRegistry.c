/**
	\file ServerSentryModActionRegistry.c
	\brief Implements live-action support for Mod action registry.
*/

/**
	\brief Registers live-action handlers and resolves them by action ID.
*/
class ServerSentryModActionRegistry
{
    /**
    	\brief Registered action handlers keyed by action ID.
    */
    protected static ref map<string, ref ServerSentryModActionHandler> s_Handlers;
    /**
    	\brief Cached action descriptors keyed by action ID.
    */
    protected static ref map<string, ref ServerSentryLiveModActionDescriptorDto> s_Descriptors;
    /**
    	\brief Action IDs in deterministic registration order.
    */
    protected static ref array<string> s_Order;

    /**
    	\brief Registers an action handler and caches its descriptor.
    	\param handler \p action handler to register or execute.
    	\param replaceExisting \p true to replace an existing registration with the same action ID.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool Register(ServerSentryModActionHandler handler, bool replaceExisting = false)
    {
        if (!handler)
        {
            return false;
        }

        EnsureState();
        ServerSentryLiveModActionDescriptorDto descriptor = handler.GetDescriptor();
        if (!descriptor || descriptor.id == "")
        {
            Print("[ServerSentry][LiveAction] Skipped registration of action handler with empty descriptor.");
            return false;
        }

        bool alreadyRegistered = s_Handlers.Contains(descriptor.id);
        if (alreadyRegistered && !replaceExisting)
        {
            Print("[ServerSentry][LiveAction] Duplicate action id rejected: " + descriptor.id + ". Pass replaceExisting=true only for an intentional override.");
            return false;
        }

        if (!alreadyRegistered)
        {
            s_Order.Insert(descriptor.id);
        }

        s_Handlers.Set(descriptor.id, handler);
        s_Descriptors.Set(descriptor.id, descriptor);
        Print("[ServerSentry][LiveAction] Registered action handler id=" + descriptor.id + " targetType=" + descriptor.targetType + " label=" + descriptor.label);
        return true;
    }

    /**
    	\brief Returns registered action descriptors in registration order.
    	\return \p A collection containing the resulting values.
    */
    static array<ref ServerSentryLiveModActionDescriptorDto> GetDescriptors()
    {
        EnsureState();
        array<ref ServerSentryLiveModActionDescriptorDto> descriptors = new array<ref ServerSentryLiveModActionDescriptorDto>();
        foreach (string id: s_Order)
        {
            ServerSentryLiveModActionDescriptorDto descriptor = s_Descriptors.Get(id);
            if (descriptor && descriptor.id != "")
            {
                descriptors.Insert(descriptor);
            }
        }

        return descriptors;
    }

    /**
    	\brief Executes the live action using the supplied request context.
    	\param ctx \p action context containing request, target, and input data.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool Execute(ServerSentryModActionContext ctx, out string message)
    {
        message = "";
        if (!ctx || ctx.actionId == "")
        {
            message = "Invalid action context.";
            return false;
        }

        EnsureState();
        if (!s_Handlers.Contains(ctx.actionId))
        {
            message = "Unknown action: " + ctx.actionId;
            Print("[ServerSentry][LiveAction] " + message);
            return false;
        }

        Print("[ServerSentry][LiveAction] Executing action id=" + ctx.actionId + " requestId=" + ctx.requestId + " targetType=" + ctx.targetType + " objectId=" + ctx.objectId + " steamId=" + ctx.steamId);
        bool ok = s_Handlers.Get(ctx.actionId).Execute(ctx, message);
        string okText = "false";
        if (ok)
        {
            okText = "true";
        }

        Print("[ServerSentry][LiveAction] Finished action id=" + ctx.actionId + " requestId=" + ctx.requestId + " success=" + okText + " message=" + message);
        return ok;
    }

    /**
    	\brief Initializes the registry collections when they are first used.
    */
    protected static void EnsureState()
    {
        if (!s_Handlers)
        {
            s_Handlers = new map<string, ref ServerSentryModActionHandler>();
        }

        if (!s_Descriptors)
        {
            s_Descriptors = new map<string, ref ServerSentryLiveModActionDescriptorDto>();
        }

        if (!s_Order)
        {
            s_Order = new array<string>();
        }
    }
}
