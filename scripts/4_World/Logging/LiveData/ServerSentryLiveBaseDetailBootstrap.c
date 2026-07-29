/**
    \file ServerSentryLiveBaseDetailBootstrap.c
    \brief Exposes the registration hook for Live Map base-detail providers.
*/

/**
    \brief Registers built-in and extension-provided Live Map base-detail providers.
*/
class ServerSentryLiveBaseDetailBootstrap
{
    /**
        \brief Instance state for completed registration.
    */
    protected bool m_Registered;

    /**
        \brief Registers all base-detail providers once.
    */
    void RegisterDefaults()
    {
        if (m_Registered)
        {
            return;
        }

        m_Registered = true;
        RegisterExtensionProviders();
    }

    /**
        \brief Registers base-detail providers supplied by extension PBOs.
    */
    void RegisterExtensionProviders()
    {
    }

    /**
        \brief Shared singleton instance.
    */
    static ref ServerSentryLiveBaseDetailBootstrap g_Instance;

    /**
        \brief Returns the shared singleton instance.
        \return \p the shared bootstrap instance.
    */
    static ServerSentryLiveBaseDetailBootstrap Get()
    {
        if (!g_Instance)
        {
            g_Instance = new ServerSentryLiveBaseDetailBootstrap;
        }

        return g_Instance;
    }
}
