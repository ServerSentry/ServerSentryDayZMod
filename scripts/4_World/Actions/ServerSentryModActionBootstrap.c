/**
	\file ServerSentryModActionBootstrap.c
	\brief Implements live-action support for Mod action bootstrap.
*/

/**
	\brief Registers built-in actions and exposes the extension registration hook.
*/
class ServerSentryModActionBootstrap
{
    /**
    	\brief Instance state for registered.
    */
    protected bool m_Registered;

    /**
    	\brief Registers the built-in ServerSentry live actions.
    */
    void RegisterDefaults()
    {
        if (m_Registered)
        {
            return;
        }

        m_Registered = true;
        ServerSentryModActionRegistry.Register(new ServerSentryInventoryAction("snapshot"));
        ServerSentryModActionRegistry.Register(new ServerSentryInventoryAction("add"));
        ServerSentryModActionRegistry.Register(new ServerSentryInventoryAction("remove"));
        ServerSentryModActionRegistry.Register(new ServerSentryInspectObjectAction());
        ServerSentryModActionRegistry.Register(new ServerSentryHealPlayerAction());
        ServerSentryModActionRegistry.Register(new ServerSentrySpawnItemPlayerAction());
        ServerSentryModActionRegistry.Register(new ServerSentryTeleportPlayerAction());
        ServerSentryModActionRegistry.Register(new ServerSentryDeleteObjectAction());
        ServerSentryModActionRegistry.Register(new ServerSentryExplodePlayerAction());
        ServerSentryModActionRegistry.Register(new ServerSentryVoiceMutePlayerAction());
        ServerSentryModActionRegistry.Register(new ServerSentryVoiceUnMutePlayerAction());
        ServerSentryModActionRegistry.Register(new ServerSentryGodModePlayerAction());
        ServerSentryModActionRegistry.Register(new ServerSentryVehicleKickAction());
        ServerSentryModActionRegistry.Register(new ServerSentryVehicleRepairAction());
        ServerSentryModActionRegistry.Register(new ServerSentrySetTimeAction());
        ServerSentryModActionRegistry.Register(new ServerSentrySetWeatherAction());
        ServerSentryModActionRegistry.Register(new ServerSentrySpawnItemAtPositionAction());
        ServerSentryModActionRegistry.Register(new ServerSentryAirdropContextAction());
        ServerSentryModActionRegistry.Register(new ServerSentryShopBankDepositAction());
        ServerSentryModActionRegistry.Register(new ServerSentryShopSpawnBundleAction());

        // Stable extension point for integration PBOs. Addons should mod this
        // method instead of overriding the full core registration routine.
        RegisterExtensionActions();

        // Samples
        //ServerSentryModActionRegistry.Register(new ServerSentrySampleCollectionsAction());
        //ServerSentryModActionRegistry.Register(new ServerSentrySampleObjectMapAction());
        //ServerSentryModActionRegistry.Register(new ServerSentrySampleJsonAction());
        //ServerSentryModActionRegistry.Register(new ServerSentrySampleVehicleKickAction());
        //ServerSentryModActionRegistry.Register(new ServerSentrySampleRaiseFlagAction());
    }

    /**
    	\brief Registers live actions supplied by extension PBOs.
    */
    void RegisterExtensionActions()
    {
    }


    /**
    	\brief Stores the g instance.
    */
    static ref ServerSentryModActionBootstrap g_Instance;

    /**
    	\brief Returns the shared singleton instance.
    	\return \p The resulting mod action bootstrap instance, or null when unavailable.
    */
    static ServerSentryModActionBootstrap Get()
    {
        if (!g_Instance)
        {
            g_Instance = new ServerSentryModActionBootstrap;
        }

        return g_Instance;
    }
}
