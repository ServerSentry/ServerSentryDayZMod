/**
    \file ServerSentryLaunch.c
    \brief Adds separate client and server commands to the Dabs launch tools.
*/

/** Selects the native debugger target before starting a diagnostic instance. */
void ServerSentrySelectDebugger(bool server)
{
    Workbench.OpenModule(ScriptEditor);
    ScriptEditor editor = Workbench.GetModule(ScriptEditor);
    if (!editor) return;

    if (server)
        editor.ExecuteAction({"Debug", "Debug Server"});
    else
        editor.ExecuteAction({"Debug", "Debug Client"});
}

/** \brief Starts the diagnostic server without closing an existing client. */
[WorkbenchPluginAttribute("Launch Server", "Start the local diagnostic server", "Shift+F6", "", {"ScriptEditor", "ResourceManager"})]
class ServerSentryLaunchServer: PluginLaunchGameBase
{
    override void RunCommandline() { Run(); }
    /** \brief Loads project settings and starts only the server. */
    override void Run()
    {
        super.Run();
        m_LaunchSettings.LaunchType = GameLaunchType.SERVER;
        m_LaunchSettings.AutoClose = false;
        ServerSentrySelectDebugger(true);
        LaunchGame(m_LaunchSettings);
    }
}

/** \brief Connects a diagnostic client to the configured local server. */
[WorkbenchPluginAttribute("Launch Client", "Connect to the local diagnostic server", "Ctrl+F6", "", {"ScriptEditor", "ResourceManager"})]
class ServerSentryLaunchClient: PluginLaunchGameBase
{
    override void RunCommandline() { Run(); }
    /** \brief Loads project settings and starts only the client. */
    override void Run()
    {
        super.Run();
        m_LaunchSettings.LaunchType = GameLaunchType.CLIENT;
        m_LaunchSettings.AutoClose = false;
        ServerSentrySelectDebugger(false);
        LaunchGame(m_LaunchSettings);
    }

    /**
        \brief Supplies a local connection independently of the launch mode.
        \param ip \p receives the loopback address.
        \param port \p receives the configured port.
        \param password \p receives the server password.
        \param args \p receives optional arguments.
        \return \p true when connection arguments are available.
    */
    override bool GetConnectionArguments(out string ip, out int port, out string password, out array<string> args)
    {
        ip = "127.0.0.1";
        port = m_LaunchSettings.Port;
        password = m_LaunchSettings.ServerPassword;
        return true;
    }
}
