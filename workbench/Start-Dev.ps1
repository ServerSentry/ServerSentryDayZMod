[CmdletBinding()]
param(
    [ValidateSet('Check', 'Workbench', 'Stage', 'Stop')]
    [string]$Action = 'Workbench',
    [ValidateSet('ServerSentryInitialize', 'PluginBuildMod', 'PluginLaunchGame', 'ServerSentryLaunchServer', 'ServerSentryLaunchClient')]
    [string]$WorkbenchPlugin = 'ServerSentryInitialize',
    [string]$SteamCommon = 'C:\Program Files (x86)\Steam\steamapps\common',
    [string]$CF = 'C:\Program Files (x86)\Steam\steamapps\workshop\content\221100\1559212036'
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$workdrive = Split-Path $projectRoot -Parent
$runtime = Join-Path $projectRoot '.workbench'
$game = Join-Path $SteamCommon 'DayZ'
$tools = Join-Path $SteamCommon 'DayZ Tools'
$workbenchExe = Join-Path $tools 'Bin\Workbench\WorkbenchApp.exe'
$workbenchDirectory = Split-Path $workbenchExe -Parent
$diagExe = Join-Path $game 'DayZDiag_x64.exe'
$dependencies = Join-Path $runtime 'dependencies'
$editorScripts = Join-Path $runtime 'vanilla-stable\scripts'
$debugMod = Join-Path $runtime '@ServerSentry'
$editorSourceMod = Join-Path $runtime '@ServerSentrySources'

if ($Action -eq 'Stop') {
    $profileArgument = '-profiles=' + (Join-Path $runtime 'profiles') + '\'
    Get-CimInstance Win32_Process -Filter "Name='DayZDiag_x64.exe'" |
        Where-Object { $_.CommandLine -and $_.CommandLine.Contains($profileArgument) } |
        ForEach-Object {
            Stop-Process -Id $_.ProcessId -Force
            Wait-Process -Id $_.ProcessId -Timeout 15 -ErrorAction SilentlyContinue
            Write-Host "Stopped local diagnostic process $($_.ProcessId)."
        }
    return
}

if ($Action -eq 'Stage') {
    $existingPbo = Join-Path $debugMod 'addons\ServerSentryDayZMod.pbo'
    if (Test-Path -LiteralPath $existingPbo) {
        try {
            $pboLock = [IO.File]::Open($existingPbo, 'Open', 'ReadWrite', 'None')
            $pboLock.Dispose()
        } catch {
            throw 'The development PBO is in use. Close the debug client and server before rebuilding.'
        }
    }
    # Pack game content only. The mirror destination must remain inside this runtime.
    $staging = [IO.Path]::GetFullPath((Join-Path $runtime 'staging\ServerSentryDayZMod'))
    $stagingRoot = [IO.Path]::GetFullPath((Join-Path $runtime 'staging')).TrimEnd('\') + '\'
    if (!$staging.StartsWith($stagingRoot, [StringComparison]::OrdinalIgnoreCase)) {
        throw 'Staging path is outside this project runtime.'
    }
    New-Item -ItemType Directory -Path $staging -Force | Out-Null
    if (@(Get-ChildItem -LiteralPath $staging -Recurse -Force -Attributes ReparsePoint).Count -gt 0 -or
        (Get-Item -LiteralPath $staging).Attributes -band [IO.FileAttributes]::ReparsePoint) {
        throw 'Staging contains a filesystem link; refusing to mirror over it.'
    }
    foreach ($name in @('scripts', 'gui')) {
        & robocopy.exe (Join-Path $projectRoot $name) (Join-Path $staging $name) /MIR /NFL /NDL /NJH /NJS /NP
        if ($LASTEXITCODE -ge 8) { throw "Staging $name failed ($LASTEXITCODE)." }
    }
    Copy-Item -LiteralPath (Join-Path $projectRoot 'config.cpp'), (Join-Path $projectRoot 'Inputs.xml') -Destination $staging -Force
    Set-Content -LiteralPath (Join-Path $staging '$PBOPREFIX$.txt') -Value 'ServerSentryDayZMod' -Encoding ASCII
    return
}

# Never regenerate scripts that are loaded by a running editor.
if (Get-Process -Name WorkbenchApp -ErrorAction SilentlyContinue) {
    if ($Action -eq 'Check') { throw 'Close Workbench before preparing its editor sources.' }
    Write-Host 'Workbench is already running. Close it before launching this project again.'
    return
}

function Assert-Path([string]$Path) {
    if (!(Test-Path -LiteralPath $Path)) { throw "Missing required path: $Path" }
}

function Invoke-Tool([string]$Exe, [string[]]$ToolArgs) {
    $toolOutput = & $Exe @ToolArgs
    $toolExitCode = $LASTEXITCODE
    $toolOutput | ForEach-Object { Write-Host $_ }
    if ($toolExitCode -ne 0 -or ($toolOutput -join "`n") -match 'Build failed') {
        throw "$Exe failed (exit code $toolExitCode). See tool output above."
    }
}

function Ensure-Junction([string]$Link, [string]$Target) {
    $existing = Get-Item -LiteralPath $Link -Force -ErrorAction SilentlyContinue
    if ($existing) {
        $actual = @($existing.Target)[0]
        if ($existing.LinkType -ne 'Junction' -or !$actual -or
            [IO.Path]::GetFullPath($actual).TrimEnd('\') -ine [IO.Path]::GetFullPath($Target).TrimEnd('\')) {
            throw "Existing path is not the expected junction: $Link -> $Target. Nothing was overwritten."
        }
    } else {
        New-Item -ItemType Junction -Path $Link -Target $Target | Out-Null
    }
}

foreach ($required in @($workbenchExe, (Join-Path $workbenchDirectory 'dayz.gproj'), $diagExe, (Join-Path $CF 'addons\scripts.pbo'),
    (Join-Path $workdrive 'DZ'), (Join-Path $workdrive 'gui\looknfeel\dayzwidgets.styles'))) { Assert-Path $required }
$wbVersion = (Get-Item -LiteralPath $workbenchExe).VersionInfo.FileVersion
$diagVersion = (Get-Item -LiteralPath $diagExe).VersionInfo.FileVersion
if ($wbVersion -ne $diagVersion) { throw "Workbench ($wbVersion) and DayZDiag ($diagVersion) must match." }
Write-Host "DayZ / Workbench: $diagVersion"

foreach ($folder in @($runtime, $dependencies, (Join-Path $debugMod 'addons'))) {
    New-Item -ItemType Directory -Path $folder -Force | Out-Null
}
$bankRev = Join-Path $tools 'Bin\PboUtils\BankRev.exe'
foreach ($pboName in @('scripts', 'gui', 'legacy')) {
    $pbo = Join-Path $CF "addons\$pboName.pbo"
    Assert-Path $pbo
    $stamp = Join-Path $dependencies "$pboName.stamp"
    $version = (Get-Item -LiteralPath $pbo).LastWriteTimeUtc.Ticks.ToString()
    if (!(Test-Path -LiteralPath $stamp) -or (Get-Content -LiteralPath $stamp -Raw).Trim() -ne $version) {
        Invoke-Tool $bankRev @('-prefix', '-f', $dependencies, $pbo)
        Set-Content -LiteralPath $stamp -Value $version -Encoding ASCII
    }
}
if (!(Test-Path -LiteralPath (Join-Path $editorScripts '1_core\defines.c'))) {
    $editorVanilla = Split-Path $editorScripts -Parent
    New-Item -ItemType Directory -Path $editorVanilla -Force | Out-Null
    Invoke-Tool $bankRev @('-prefix', '-f', $editorVanilla, (Join-Path $game 'dta\scripts.pbo'))
}
. (Join-Path $PSScriptRoot 'Initialize-DabsWorkbench.ps1')

# Load the mod under its PBO prefix; mounting the repository root duplicates scripts.
$editorPrefix = Join-Path $editorSourceMod 'ServerSentryDayZMod'
New-Item -ItemType Directory -Path $editorPrefix -Force | Out-Null
foreach ($folder in @('scripts', 'gui')) {
    Ensure-Junction (Join-Path $editorPrefix $folder) (Join-Path $projectRoot $folder)
}
Copy-Item -LiteralPath (Join-Path $projectRoot 'config.cpp') -Destination $editorSourceMod -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'Inputs.xml') -Destination $editorPrefix -Force

# Migrate only the two old generated vanilla-script junctions, never actual files.
foreach ($link in @((Join-Path $game 'scripts'), (Join-Path (Split-Path $workbenchExe) 'scripts'))) {
    $existing = Get-Item -LiteralPath $link -Force -ErrorAction SilentlyContinue
    if ($existing -and $existing.LinkType -eq 'Junction' -and @($existing.Target)[0] -ieq (Join-Path $workdrive 'scripts')) {
        [IO.Directory]::Delete($existing.FullName)
    }
}
Ensure-Junction (Join-Path $game 'ServerSentryDayZMod') $projectRoot
Ensure-Junction (Join-Path (Split-Path $workbenchExe) 'scripts') $editorScripts
Ensure-Junction (Join-Path (Split-Path $workbenchExe) 'JM') (Join-Path $dependencies 'JM')
Ensure-Junction (Join-Path (Split-Path $workbenchExe) 'ServerSentryDayZMod') $projectRoot

if ($Action -eq 'Check') {
    Write-Host 'OK: stable editor sources, CF, native layouts and Workbench tools prepared.'
    return
}
$launchArgs = @("`"-mod=$CF;$editorSourceMod`"", "`"-repository=$projectRoot`"", '-filePatching', '-doLogs',
    "`"-profiles=$(Join-Path $runtime 'profiles\workbench')`"", '-limitFPS=60', '-wbmodule=ResourceManager', "-plugin=$WorkbenchPlugin")
# Workbench resolves its default dayz.gproj relative to the process working directory.
# Dabs project settings still resolve through the explicit -repository argument.
$process = Start-Process -FilePath $workbenchExe -WorkingDirectory $workbenchDirectory -ArgumentList $launchArgs -PassThru
Write-Host "Started Workbench (PID $($process.Id))."
