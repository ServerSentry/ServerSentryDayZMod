/**
    \file ServerSentryLiveBaseDetailRegistry.c
    \brief Registers and invokes Live Map base-detail providers.
*/

/**
    \brief Stores base-detail providers in deterministic registration order.
*/
class ServerSentryLiveBaseDetailRegistry
{
    /**
        \brief Registered providers in execution order.
    */
    protected static ref array<ref ServerSentryLiveBaseDetailProvider> s_Providers;

    /**
        \brief Registers a base-detail provider.
        \param provider \p provider to register.
        \return \p true when the provider was registered; otherwise false.
    */
    static bool Register(ServerSentryLiveBaseDetailProvider provider)
    {
        if (!provider)
        {
            return false;
        }

        EnsureState();
        if (s_Providers.Find(provider) >= 0)
        {
            return false;
        }

        s_Providers.Insert(provider);
        ServerSentryLiveApi.InvalidateBaseDetails();
        Print("[ServerSentry][LiveMap] Registered base detail provider.");
        return true;
    }

    /**
        \brief Applies every matching provider after Vanilla details were created.
        \param entity \p live world entity represented by the DTO.
        \param dto \p mutable base DTO including all Vanilla details.
    */
    static void Apply(EntityAI entity, ServerSentryLiveBaseDto dto)
    {
        if (!entity || !dto)
        {
            return;
        }

        EnsureState();
        foreach (ServerSentryLiveBaseDetailProvider provider: s_Providers)
        {
            if (provider && provider.Supports(entity, dto))
            {
                provider.ApplyDetails(entity, dto);
            }
        }
    }

    /**
        \brief Initializes the provider collection when first used.
    */
    protected static void EnsureState()
    {
        if (!s_Providers)
        {
            s_Providers = new array<ref ServerSentryLiveBaseDetailProvider>();
        }
    }
}
