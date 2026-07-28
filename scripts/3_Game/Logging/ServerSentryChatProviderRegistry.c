/**
	\file ServerSentryChatProviderRegistry.c
	\brief Provides logging support for Chat provider registry.
*/

/**
	\brief Stores and resolves registrations for Chat provider.
*/
class ServerSentryChatProviderRegistry
{
    /**
    	\brief Shared external chat provider active state used by all instances.
    */
    protected static bool s_ExternalChatProviderActive;
    /**
    	\brief Shared external chat providers state used by all instances.
    */
    protected static ref array<string> s_ExternalChatProviders;

    /**
    	\brief Registers external chat provider.
    	\param providerName \p provider name used by the operation.
    */
    static void RegisterExternalChatProvider(string providerName)
    {
        if (providerName == "")
        {
            providerName = "unknown";
        }

        if (!s_ExternalChatProviders)
        {
            s_ExternalChatProviders = new array<string>();
        }

        if (s_ExternalChatProviders.Find(providerName) == -1)
        {
            s_ExternalChatProviders.Insert(providerName);
        }

        s_ExternalChatProviderActive = true;
    }

    /**
    	\brief Returns whether external chat provider active.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsExternalChatProviderActive()
    {
        return s_ExternalChatProviderActive;
    }

    /**
    	\brief Returns whether suppress vanilla chat.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool ShouldSuppressVanillaChat()
    {
        return s_ExternalChatProviderActive;
    }

    /**
    	\brief Returns external chat providers.
    	\return \p The resulting string value.
    */
    static string GetExternalChatProviders()
    {
        if (!s_ExternalChatProviders || s_ExternalChatProviders.Count() == 0)
        {
            return "";
        }

        string providers = "";
        for (int i = 0; i < s_ExternalChatProviders.Count(); i++)
        {
            if (i > 0)
            {
                providers += ",";
            }

            providers += s_ExternalChatProviders.Get(i);
        }

        return providers;
    }
}
