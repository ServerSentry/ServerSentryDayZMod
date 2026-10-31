/**
    \file ServerSentryWorkbench.c
    \brief Initializes the Dabs Workbench environment for ServerSentry development.
*/

/**
    \brief Seeds local launch/build settings and opens the mod in Script Editor.
*/
void ServerSentryCreateWorkbench()
{
    string directory = PluginDialogBase.GetCurrentDirectory();
    if (!directory.Contains("ServerSentryDayZMod")) return;
    string repository = PluginDialogBase.GetDirectory(directory.Substring(0, directory.Length() - 1));
    string launchFile = directory + "launch.bin";
    if (!FileExist(launchFile))
    {
        LaunchSettings launch = LaunchSettings.Load(launchFile);
        launch.Repository = repository;
        launch.Profiles = repository + ".workbench\\profiles";
        launch.Missions = repository + ".workbench\\mpmissions";
        launch.Mods = repository + ".workbench\\mods";
        launch.LaunchType = GameLaunchType.BOTH;
        launch.EnvironmentType = DayZEnvironmentType.CUSTOM;
        launch.CustomGameDirectory = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\DayZ";
        launch.Executable = "DayZDiag_x64.exe";
        launch.Map = "ChernarusPlus";
        launch.Port = 2322;
        launch.FilePatching = true;
        launch.LoadMission = false;
        launch.AutoClose = true;
        launch.Deloginator = false;
        launch.Name = "ServerSentry Dev";
        launch.LaunchArgs = "-doLogs -adminLog -scriptDebug=true -window -x=1600 -y=900 -noPause -noSplash -ip=127.0.0.1 -no_preload_vehicles";
        launch.Save();
    }

    string buildFile = directory + "build.bin";
    if (!FileExist(buildFile))
    {
        BuildSettings build = BuildSettings.Load(buildFile);
        build.Builder = BuilderType.PBO_PROJECT;
        build.PboProject_Args = "+H";
        build.Dependencies = false;
        build.CopyAddons = false;
        build.ClearTemp = false;
        build.TempFolder = repository + ".workbench\\build-temp";
        build.Save(buildFile);
    }

    Print("[ServerSentry Workbench] Dabs tools loaded. F6 Launch Game; F8 Build Mod; Ctrl+Shift+F10 Close Game.");
    Workbench.OpenModule(ScriptEditor);
    ScriptEditor editor = Workbench.GetModule(ScriptEditor);
    if (editor)
        editor.SetOpenedResource("ServerSentryDayZMod/scripts/3_Game/DayZGame.c");
}

[WorkbenchPluginAttribute("Initialize ServerSentry", "Open the ServerSentry development environment", "", "", {"ScriptEditor", "ResourceManager"})]
class ServerSentryInitialize: WorkbenchPlugin
{
    override void Run() { ServerSentryCreateWorkbench(); }
    override void RunCommandline() { ServerSentryCreateWorkbench(); }
}
