# Loaded by Start-Dev.ps1 after installation paths and helper functions are set.
function Write-EditorSource([string]$Path, [string]$Content) {
    # Windows PowerShell 5's UTF8 encoding prepends a BOM that Enforce rejects.
    [IO.File]::WriteAllText($Path, $Content, [Text.UTF8Encoding]::new($false))
}
$dabsCommit = '245190266b4e72bb0519fbfdd68bbed71c1edc1f'
$dabsRepository = Join-Path $runtime 'vendor\DabsFramework'
$upstream = Join-Path $dabsRepository 'DabsFramework'
if (!(Test-Path -LiteralPath (Join-Path $upstream 'Workbench\ToolAddons'))) {
    New-Item -ItemType Directory -Path (Split-Path $dabsRepository -Parent) -Force | Out-Null
    Invoke-Tool 'git.exe' @('clone', '--filter=blob:none', '--sparse', 'https://github.com/InclementDab/DayZ-Dabs-Framework.git', $dabsRepository)
    Invoke-Tool 'git.exe' @('-C', $dabsRepository, 'sparse-checkout', 'set', 'DabsFramework/Scripts', 'DabsFramework/Workbench')
    Invoke-Tool 'git.exe' @('-C', $dabsRepository, 'checkout', $dabsCommit)
}

# Keep the upstream checkout unchanged. Local compatibility changes live in ignored generated files.
$dabsToolsRoot = Join-Path $runtime 'dabs-tools'
$dabsTools = Join-Path $dabsToolsRoot 'DabsFramework'
New-Item -ItemType Directory -Path (Join-Path $dabsTools 'Scripts'), (Join-Path $dabsTools 'Workbench') -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $upstream 'Scripts\1_Core') -Destination (Join-Path $dabsTools 'Scripts') -Recurse -Force
Copy-Item -LiteralPath (Join-Path $upstream 'Workbench\ToolAddons') -Destination (Join-Path $dabsTools 'Workbench') -Recurse -Force
Copy-Item -LiteralPath (Join-Path $dabsRepository 'LICENSE.md') -Destination (Join-Path $dabsToolsRoot 'LICENSE.md') -Force
Set-Content -LiteralPath (Join-Path $dabsTools 'Workbench\ToolAddons\ResourceManager\CreateWorkbench.c') -Value "void CreateWorkbench()`n{`n    ServerSentryCreateWorkbench();`n}" -Encoding ASCII

$pluginRoot = Join-Path $dabsTools 'Workbench\ToolAddons\ScriptEditor'
$escapedRepository = $projectRoot.Replace('\', '\\')
$escapedGame = $game.Replace('\', '\\')
$launchDefaultsFile = Join-Path $pluginRoot 'LaunchSettings.c'
$launchDefaults = Get-Content -LiteralPath $launchDefaultsFile -Raw -Encoding UTF8
$launchDefaults = $launchDefaults.Replace('settings.LaunchType = GameLaunchType.BOTH;', "settings.Repository = `"$escapedRepository`";`n`t`tsettings.LaunchType = GameLaunchType.BOTH;")
$launchDefaults = $launchDefaults.Replace('settings.Profiles = "P:\\Profiles";', "settings.Profiles = `"$escapedRepository\\.workbench\\profiles`";")
$launchDefaults = $launchDefaults.Replace('settings.Missions = "P:\\Missions";', "settings.Missions = `"$escapedRepository\\.workbench\\mpmissions`";")
$launchDefaults = $launchDefaults.Replace('settings.Mods = "P:\\Mods";', "settings.Mods = `"$escapedRepository\\.workbench\\mods`";")
$launchDefaults = $launchDefaults.Replace('settings.EnvironmentType = DayZEnvironmentType.STABLE;', 'settings.EnvironmentType = DayZEnvironmentType.CUSTOM;')
$launchDefaults = $launchDefaults.Replace('settings.CustomGameDirectory = "";', "settings.CustomGameDirectory = `"$escapedGame`";")
$launchDefaults = $launchDefaults.Replace('settings.Port = 2302;', 'settings.Port = 2322;')
$launchDefaults = $launchDefaults.Replace('settings.LoadMission = true;', 'settings.LoadMission = false;')
$launchDefaults = $launchDefaults.Replace('-newErrorsAreWarnings=1 ', '').Replace('-scrDef=ENABLE_LOGGING', '-scrDef=ENABLE_LOGGING -ip=127.0.0.1')
Write-EditorSource $launchDefaultsFile $launchDefaults
$dialogFile = Join-Path $pluginRoot 'PluginDialogBase.c'
$dialog = Get-Content -LiteralPath $dialogFile -Raw -Encoding UTF8
$dialog = $dialog.Replace('protected ResourceBrowser ', 'protected ResourceManager ')
$dialog = $dialog.Replace('Workbench.GetModule("ScriptEditor")', 'Workbench.GetModule(ScriptEditor)').Replace('Workbench.GetModule("ResourceManager")', 'Workbench.GetModule(ResourceManager)')
$dialog = $dialog.Replace('Workbench.GetAbsolutePath("$CurrentDir:", abs);', 'string repository; if (GetCLIParam("repository", repository)) { abs = repository + "\\workbench\\"; } else { abs = GetDirectory(Workbench.GetCurrentGameProjectFile()); }')
$dialog = $dialog.Replace('Workbench.GetAbsolutePath(string.Empty, root_dir);', 'Workbench.GetAbsolutePath("$Workdrive:", root_dir);')
$scopedStop = @'
static void KillTask(string task_name)
    {
        if (task_name != "DayZDiag_x64.exe") return;
        string repository = GetDirectory(GetCurrentDirectory().Substring(0, GetCurrentDirectory().Length() - 1));
        Workbench.RunCmd(string.Format("powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"%1workbench\\Start-Dev.ps1\" -Action Stop", repository), true);
    }

'@
$dialog = [regex]::Replace($dialog, '(?s)static void KillTask\(string task_name\).*?(?=\tstatic void CleanLogFolder)', $scopedStop)
$nativeLaunchHelper = @'
static void LaunchFromGameDirectory(string executable, string arguments, string role)
    {
        string currentDirectory = GetCurrentDirectory();
        string repository = GetDirectory(currentDirectory.Substring(0, currentDirectory.Length() - 1));
        string launchDirectory = repository + ".workbench\\native-launch";
        MakeDirectory(launchDirectory);
        string argumentFile = launchDirectory + "\\" + role + ".args";
        FileHandle handle = OpenFile(argumentFile, FileMode.WRITE);
        if (!handle) { Error("Cannot write native launch arguments: " + argumentFile); return; }
        FPrintln(handle, executable);
        FPrintln(handle, arguments);
        CloseFile(handle);
        Workbench.RunCmd(string.Format("powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"%1workbench\\Start-NativeGame.ps1\" -ArgumentFile \"%2\" -Role %3", repository, argumentFile, role), true);
    }

'@
$dialog = $dialog.Replace('static void CleanLogFolder', $nativeLaunchHelper + 'static void CleanLogFolder')
Write-EditorSource $dialogFile $dialog
$killFile = Join-Path $pluginRoot 'PluginDialogBase\PluginKillGame.c'
$kill = (Get-Content -LiteralPath $killFile -Raw -Encoding UTF8).Replace('Workbench.RunCmd(string.Format("taskkill /f /im %1 /t", launch_settings.GetExecutableName()));', 'KillTask(launch_settings.GetExecutableName());')
$kill = $kill.Replace('"F10"', '"Ctrl+Shift+F10"')
Write-EditorSource $killFile $kill
$joinFile = Join-Path $pluginRoot 'PluginDialogBase\PluginProject\PluginLaunchGameBase\PluginJoinRemote.c'
Write-EditorSource $joinFile ((Get-Content -LiteralPath $joinFile -Raw -Encoding UTF8).Replace('"Ctrl+F6"', '""'))

$projectPluginFile = Join-Path $pluginRoot 'PluginDialogBase\PluginProject.c'
$projectPlugin = Get-Content -LiteralPath $projectPluginFile -Raw -Encoding UTF8
$projectPlugin = $projectPlugin.Replace('string current_dir = GetCurrentDirectory();', "m_ProjectSettings.Clear();`n`t`tm_Prefixes.Clear();`n`t`tstring current_dir = GetCurrentDirectory();")
Write-EditorSource $projectPluginFile $projectPlugin

# The original launch cleanup removes unknown folders beside WorkbenchApp.exe.
# Retain the installed tool directories and only create missing file-patching junctions.
$launchFile = Join-Path $pluginRoot 'PluginDialogBase\PluginProject\PluginLaunchGameBase.c'
$launch = Get-Content -LiteralPath $launchFile -Raw -Encoding UTF8
$launch = [regex]::Replace($launch, '(?s)\t\tif \(!is_game_and_workbench_same_directory\) \{\s*// Now FindFile each.*?\n\t\t\}\s*(?=// Reformats mod list)', '')
$launch = $launch.Replace('Workbench.RunCmd(string.Format("%1 %2", game_exe, client_launch_params));', 'Workbench.RunCmd(string.Format("\"%1\" %2", game_exe, client_launch_params));')
$launch = $launch.Replace('Workbench.RunCmd(game_exe + " " + server_launch_params);', 'Workbench.RunCmd(string.Format("\"%1\" %2", game_exe, server_launch_params));')
$launch = $launch.Replace('Workbench.RunCmd(game_exe + " " + offline_launch_params);', 'Workbench.RunCmd(string.Format("\"%1\" %2", game_exe, offline_launch_params));')
$launch = $launch.Replace('Workbench.RunCmd(string.Format("\"%1\" %2", game_exe, client_launch_params));', 'LaunchFromGameDirectory(game_exe, client_launch_params, "client");')
$launch = $launch.Replace('Workbench.RunCmd(string.Format("\"%1\" %2", game_exe, server_launch_params));', 'LaunchFromGameDirectory(game_exe, server_launch_params, "server");')
$launch = $launch.Replace('Workbench.RunCmd(string.Format("\"%1\" %2", game_exe, offline_launch_params));', 'LaunchFromGameDirectory(game_exe, offline_launch_params, "offline");')
$launch = $launch.Replace('port = 2302;', 'port = m_LaunchSettings.Port;')
# Start the dedicated server before the connecting client, using the same settings.
$serverStart = $launch.IndexOf("`t`tif ((launch_settings.LaunchType & GameLaunchType.SERVER)")
$offlineStart = $launch.IndexOf("`t`tif ((launch_settings.LaunchType & GameLaunchType.OFFLINE)", $serverStart)
$serverBlock = $launch.Substring($serverStart, $offlineStart - $serverStart)
$launch = $launch.Remove($serverStart, $serverBlock.Length)
$clientStart = $launch.IndexOf("`t`tif ((launch_settings.LaunchType & GameLaunchType.CLIENT)")
$launch = $launch.Insert($clientStart, $serverBlock)
if ($launch.Contains('rmdir /s /q') -and $launch.Contains('// Now FindFile each')) { throw 'Dabs launch cleanup patch did not apply.' }
Write-EditorSource $launchFile $launch
$launchGameFile = Join-Path $pluginRoot 'PluginDialogBase\PluginProject\PluginLaunchGameBase\PluginLaunchGame.c'
$launchGame = (Get-Content -LiteralPath $launchGameFile -Raw -Encoding UTF8).Replace('port = 2302;', 'port = m_LaunchSettings.Port;')
$launchGame = $launchGame.Replace('void PluginLaunchGame()', 'override void RunCommandline() { Run(); }' + "`n`tvoid PluginLaunchGame()")
$sessionSettings = @"
super.Run();
        if (!m_LaunchSettings) return;
        m_LaunchSettings.LaunchType = GameLaunchType.BOTH;
        m_LaunchSettings.LoadMission = false;
        m_LaunchSettings.SandboxieEnabled = false;
        m_LaunchSettings.FilePatching = true;
        m_LaunchSettings.Executable = "DayZDiag_x64.exe";
        m_LaunchSettings.EnvironmentType = DayZEnvironmentType.CUSTOM;
        m_LaunchSettings.CustomGameDirectory = "$escapedGame";
        m_LaunchSettings.AutoClose = false;
        if (!m_LaunchSettings.LaunchArgs.Contains("-scriptDebug=true"))
            m_LaunchSettings.LaunchArgs += " -scriptDebug=true";
        ServerSentrySelectDebugger(true);
"@
$launchGame = $launchGame.Replace('super.Run();', $sessionSettings)
$launchGame = $launchGame.Replace('"Lanches DayZ"', '"Restart the local Stable server and connect a diagnostic client"')
Write-EditorSource $launchGameFile $launchGame

$buildDefaultsFile = Join-Path $pluginRoot 'BuildSettings.c'
$buildDefaults = (Get-Content -LiteralPath $buildDefaultsFile -Raw -Encoding UTF8).Replace('settings.Builder = BuilderType.PBO_PROJECT;', 'settings.Builder = BuilderType.ADDON_BUILDER;')
Write-EditorSource $buildDefaultsFile $buildDefaults
$buildFile = Join-Path $pluginRoot 'PluginDialogBase\PluginProject\PluginBuildMod.c'
$build = Get-Content -LiteralPath $buildFile -Raw -Encoding UTF8
$build = $build.Replace("`tvoid PluginBuildMod()", "`toverride void RunCommandline()`n`t{`n`t`tRun();`n`t}`n`n`tvoid PluginBuildMod()")
$build = $build.Replace('string main_mod_folder = GetAbsolutePath(string.Format("$Workdrive:%1", mod_prefix));', @'
string stage_command = string.Format("powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"%1\\workbench\\Start-Dev.ps1\" -Action Stage", m_LaunchSettings.Repository);
        if (Workbench.RunCmd(stage_command, true) != 0) {
            ErrorDialog("Staging failed. See the output for details; another process may be holding the development PBO open.");
            return;
        }
        string main_mod_folder = m_LaunchSettings.Repository + "\\.workbench\\staging\\ServerSentryDayZMod";
'@)
$build = $build.Replace('string command = string.Format("\"%1\" -Mod=%2 %3 %4 -W +X=\"%5\"", cmd, mod_output, mod_input, args, excludes);', 'string command = string.Format("\"%1\" -P -Key -Restore \"-Mod=%2\" \"%3\" %4 -W", cmd, mod_output, mod_input, args);')
$build = $build.Replace("return 0;`n`t`treturn Workbench.RunCmd(command, true);", 'return Workbench.RunCmd(command, true);')
$build = $build.Replace("return 0;`r`n`t`treturn Workbench.RunCmd(command, true);", 'return Workbench.RunCmd(command, true);')
$addonMethod = @'
int BuildUsingAddonBuilder(string mod_input, string mod_output)
    {
        string exe = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\DayZ Tools\\Bin\\AddonBuilder\\AddonBuilder.exe";
        MakeDirectory(mod_output);
        MakeDirectory(mod_output + "\\Addons");
        string includes = m_LaunchSettings.Repository + "\\workbench\\addon-include.txt";
        string command = string.Format("\"%1\" \"%2\" \"%3\\Addons\" -packonly -clear -prefix=ServerSentryDayZMod \"-include=%4\" %5", exe, mod_input, mod_output, includes, m_BuildSettings.AddonBuilder_Args);
        PrintFormat("Building mod with Addon Builder: %1", command);
        return Workbench.RunCmd(command, true);
    }
}
'@
$build = [regex]::Replace($build, '(?s)int BuildUsingAddonBuilder\(string mod_input, string mod_output\).*\z', $addonMethod)
Write-EditorSource $buildFile $build
$configureBuildFile = Join-Path $pluginRoot 'PluginDialogBase\PluginProject\PluginConfigureBuildMod.c'
$configureBuild = (Get-Content -LiteralPath $configureBuildFile -Raw -Encoding UTF8).Replace('Workbench.ScriptDialog("Edit Build", DIALOG_TAB_SIZE, m_BuildSettings);', "super.Run();`n`t`tWorkbench.ScriptDialog(`"Edit Build`", DIALOG_TAB_SIZE, m_BuildSettings);")
Write-EditorSource $configureBuildFile $configureBuild

# The editor only needs module types for symbols. Instantiating gameplay CF
# modules during editor reload invokes input APIs while Game is being unloaded.
# This extracted editor copy is separate from the installed/published CF PBO.
$cfRegisterFile = Join-Path $dependencies 'JM\CF\Scripts\1_Core\communityframework\module\cf_registermodule.c'
$cfRegister = Get-Content -LiteralPath $cfRegisterFile -Raw -Encoding UTF8
if (!$cfRegister.Contains('#ifndef WORKBENCH')) {
    $cfRegister = $cfRegister.Replace('CF_ModuleCoreManager._Insert(type);', "#ifndef WORKBENCH`n`t`tCF_ModuleCoreManager._Insert(type);`n#endif")
    Write-EditorSource $cfRegisterFile $cfRegister
}
$mods = Join-Path $runtime 'mods'
New-Item -ItemType Directory -Path $mods -Force | Out-Null
Ensure-Junction (Join-Path $mods '@CF') $CF
Ensure-Junction (Join-Path $mods '@ServerSentryDayZMod') $debugMod
$dabsMission = Join-Path $runtime 'mpmissions\ServerSentryDayZMod.ChernarusPlus'
if (!(Test-Path -LiteralPath (Join-Path $dabsMission 'init.c'))) {
    $missionSource = Join-Path $SteamCommon 'DayZServer\mpmissions\dayzOffline.chernarusplus'
    New-Item -ItemType Directory -Path $dabsMission -Force | Out-Null
    & robocopy.exe $missionSource $dabsMission /E /XD 'storage*' /XF '*.log' '*.RPT' /NFL /NDL /NJH /NJS /NP
    if ($LASTEXITCODE -ge 8) { throw "Dabs mission copy failed ($LASTEXITCODE)." }
}
(Get-Content -LiteralPath (Join-Path $PSScriptRoot 'serverDZ.cfg') -Raw) |
    Set-Content -LiteralPath (Join-Path $PSScriptRoot 'server.cfg') -Encoding ASCII

# The script directory scanner does not reliably descend through nested junctions.
# Keep actual files inside the isolated stock module directories instead.
foreach ($copy in @(
    @{ Source = (Join-Path $dabsTools 'Scripts\1_Core'); Destination = (Join-Path $editorScripts '1_core\ServerSentryDabsCore') },
    @{ Source = (Join-Path $dabsTools 'Workbench\ToolAddons'); Destination = (Join-Path $editorScripts 'editor\Workbench\ServerSentryDabsTools') },
    @{ Source = (Join-Path $PSScriptRoot 'ToolAddons'); Destination = (Join-Path $editorScripts 'editor\Workbench\ServerSentryLocalTools') }
)) {
    $copyRoot = [IO.Path]::GetFullPath($editorScripts).TrimEnd('\') + '\'
    if (![IO.Path]::GetFullPath($copy.Destination).StartsWith($copyRoot, [StringComparison]::OrdinalIgnoreCase)) {
        throw 'Editor source copy is outside the isolated scripts directory.'
    }
    $existing = Get-Item -LiteralPath $copy.Destination -Force -ErrorAction SilentlyContinue
    if ($existing -and $existing.LinkType -eq 'Junction') {
        if (@($existing.Target)[0] -ine $copy.Source) { throw 'Unexpected editor source junction target.' }
        [IO.Directory]::Delete($existing.FullName)
    }
    New-Item -ItemType Directory -Path $copy.Destination -Force | Out-Null
    Get-ChildItem -LiteralPath $copy.Source -Force | Copy-Item -Destination $copy.Destination -Recurse -Force
}
