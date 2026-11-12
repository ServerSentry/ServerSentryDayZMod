[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$ArgumentFile,
    [ValidateSet('client', 'server', 'offline')][string]$Role = 'client'
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$argumentRoot = [IO.Path]::GetFullPath((Join-Path $projectRoot '.workbench\native-launch')).TrimEnd('\') + '\'
$resolvedArgumentFile = [IO.Path]::GetFullPath($ArgumentFile)
if (!$resolvedArgumentFile.StartsWith($argumentRoot, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Native launch arguments must be inside this project runtime directory.'
}
$launchLines = @(Get-Content -LiteralPath $resolvedArgumentFile)
if ($launchLines.Count -lt 2) { throw 'Incomplete native launch arguments.' }
$gameExe = $launchLines[0]
if (!(Test-Path -LiteralPath $gameExe -PathType Leaf)) { throw "Game executable missing: $gameExe" }
$launchOptions = @{
    FilePath = $gameExe
    WorkingDirectory = Split-Path $gameExe -Parent
    ArgumentList = ($launchLines[1..($launchLines.Count - 1)] -join ' ')
    PassThru = $true
}
if ($Role -eq 'server') { $launchOptions.WindowStyle = 'Hidden' }
$gameProcess = Start-Process @launchOptions
Write-Output "Started native $Role (PID $($gameProcess.Id))."
