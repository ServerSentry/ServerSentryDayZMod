/**
	\file ServerSentryLiveModActionDescriptorDto.c
	\brief Defines the Live mod action descriptor DTO live-data transfer model.
*/

/**
	\brief Describes a live action exposed to the ServerSentry website.
*/
class ServerSentryLiveModActionDescriptorDto
{
    /**
    	\brief Stable identifier of this value.
    */
    string id;
    /**
    	\brief Type of target resolved by the action.
    */
    string targetType;
    /**
    	\brief Human-readable label shown to users.
    */
    string label;
    /**
    	\brief Stores the icon.
    */
    string icon;
    /**
    	\brief Indicates whether dangerous.
    */
    bool dangerous;
    /**
    	\brief Stores the confirm text.
    */
    string confirmText;
    /**
    	\brief Stores the description.
    */
    string description;
    /**
    	\brief Collection of inputs values.
    */
    ref array<ref ServerSentryLiveModActionInputDto> inputs;
    /**
    	\brief Collection of scopes values.
    */
    ref array<string> scopes;

    /**
    	\brief Initializes a new ServerSentryLiveModActionDescriptorDto instance.
    */
    void ServerSentryLiveModActionDescriptorDto()
    {
        id = "";
        targetType = "object";
        label = "Action";
        icon = "sparkles";
        dangerous = false;
        confirmText = "";
        description = "";
        inputs = new array<ref ServerSentryLiveModActionInputDto>();
        scopes = new array<string>();
    }

    // DayZ-safe mutators. Use them as separate statements; Enforce Script
    // should not be treated like a fluent C# builder.
    /**
    	\brief Adds an execution scope to this action descriptor.
    	\param scope \p scope used by the operation.
    */
    void AddScope(string scope)
    {
        scope.TrimInPlace();
        if (scope != "" && scopes.Find(scope) == -1)
        {
            scopes.Insert(scope);
        }
    }

    /**
    	\brief Adds an input descriptor to this action descriptor.
    	\param input \p parsed input data used by the action.
    */
    void AddInput(ServerSentryLiveModActionInputDto input)
    {
        if (input)
        {
            inputs.Insert(input);
        }
    }

    /**
    	\brief Marks the action as dangerous and sets its confirmation text.
    	\param confirmationText \p confirmation text used by the operation.
    */
    void SetDangerous(string confirmationText)
    {
        dangerous = true;
        confirmText = confirmationText;
    }
}

/**
	\brief Creates base descriptors for common live-action target types.
*/
class ServerSentryLiveModActionDescriptorFactory
{
    /**
    	\brief Creates a base live-action descriptor.
    	\param id \p identifier used by the operation.
    	\param targetType \p target type used by the operation.
    	\param label \p human-readable label shown to users.
    	\param icon \p icon used by the operation.
    	\param description \p description used by the operation.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionDescriptorDto Create(string id, string targetType, string label, string icon = "sparkles", string description = "")
    {
        ServerSentryLiveModActionDescriptorDto descriptor = new ServerSentryLiveModActionDescriptorDto();
        descriptor.id = id;
        descriptor.targetType = targetType;
        descriptor.label = label;
        descriptor.icon = icon;
        descriptor.description = description;
        return descriptor;
    }

    /**
    	\brief Creates an action descriptor targeting a player.
    	\param id \p identifier used by the operation.
    	\param label \p human-readable label shown to users.
    	\param icon \p icon used by the operation.
    	\param description \p description used by the operation.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionDescriptorDto ForPlayer(string id, string label, string icon = "User", string description = "")
    {
        return Create(id, "player", label, icon, description);
    }

    /**
    	\brief Creates an action descriptor targeting a world object.
    	\param id \p identifier used by the operation.
    	\param label \p human-readable label shown to users.
    	\param icon \p icon used by the operation.
    	\param description \p description used by the operation.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionDescriptorDto ForObject(string id, string label, string icon = "Box", string description = "")
    {
        return Create(id, "object", label, icon, description);
    }

    /**
    	\brief Creates an action descriptor targeting any tracked vehicle.
    */
    static ServerSentryLiveModActionDescriptorDto ForVehicle(string id, string label, string icon = "Car", string description = "")
    {
        return Create(id, "vehicle", label, icon, description);
    }

    /**
    	\brief Creates an action descriptor targeting any tracked base object.
    */
    static ServerSentryLiveModActionDescriptorDto ForBase(string id, string label, string icon = "Fence", string description = "")
    {
        return Create(id, "base", label, icon, description);
    }

    /**
    	\brief Creates an action descriptor targeting tracked territory flags / flag poles only.
    */
    static ServerSentryLiveModActionDescriptorDto ForFlagPole(string id, string label, string icon = "Flag", string description = "")
    {
        return Create(id, "base_flagpole", label, icon, description);
    }

    /**
    	\brief Creates an action descriptor for a custom live-map object subtype.
    */
    static ServerSentryLiveModActionDescriptorDto ForWorldObjectType(string id, string worldObjectType, string label, string icon = "Box", string description = "")
    {
        worldObjectType.TrimInPlace();
        if (worldObjectType == "")
        {
            worldObjectType = "object";
        }

        return Create(id, worldObjectType, label, icon, description);
    }

    /**
    	\brief Creates a global action descriptor without a specific target.
    	\param id \p identifier used by the operation.
    	\param label \p human-readable label shown to users.
    	\param icon \p icon used by the operation.
    	\param description \p description used by the operation.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionDescriptorDto ForGlobal(string id, string label, string icon = "Globe", string description = "")
    {
        return Create(id, "global", label, icon, description);
    }

    /**
    	\brief Creates an action shown in the Live Map right-click context menu.
    	\param id \p identifier used by the operation.
    	\param label \p human-readable label shown to users.
    	\param icon \p icon used by the operation.
    	\param description \p description used by the operation.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionDescriptorDto ForContextMenu(string id, string label, string icon = "MapPin", string description = "")
    {
        return Create(id, "contextMenu", label, icon, description);
    }

    /**
    	\brief Creates a shop action descriptor targeting a player.
    	\param id \p identifier used by the operation.
    	\param label \p human-readable label shown to users.
    	\param icon \p icon used by the operation.
    	\param description \p description used by the operation.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionDescriptorDto ShopPlayer(string id, string label, string icon = "ShoppingBag", string description = "")
    {
        ServerSentryLiveModActionDescriptorDto descriptor = ForPlayer(id, label, icon, description);
        descriptor.AddScope("shop");
        return descriptor;
    }

    /**
    	\brief Creates a live shop action descriptor targeting a player.
    	\param id \p identifier used by the operation.
    	\param label \p human-readable label shown to users.
    	\param icon \p icon used by the operation.
    	\param description \p description used by the operation.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionDescriptorDto ShopPlayerLive(string id, string label, string icon = "ShoppingBag", string description = "")
    {
        ServerSentryLiveModActionDescriptorDto descriptor = ShopPlayer(id, label, icon, description);
        descriptor.AddScope("live");
        return descriptor;
    }
}
