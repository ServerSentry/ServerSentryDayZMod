param(
    [Parameter(Mandatory)][string]$MarkdigAssembly,
    [string]$UnicodeData = "$PSScriptRoot/../.workbench/emoji-source/emoji-test-17.0.txt"
)

$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$atlasCatalog = Join-Path $root '.workbench/emoji-source/EmojiCatalog.tsv'
$null = [Reflection.Assembly]::LoadFrom($MarkdigAssembly)
$aliases = [Markdig.Extensions.Emoji.EmojiMapping]::GetDefaultEmojiShortcodeToUnicode()
$knownAliases = @{}
foreach ($entry in $aliases.GetEnumerator()) {
    $key = $entry.Value.Replace([string][char]0xFE0F, '')
    if (!$knownAliases.ContainsKey($key)) { $knownAliases[$key] = $entry.Key }
}

$groups = @('Smileys & Emotion', 'People & Body', 'Animals & Nature', 'Food & Drink',
    'Travel & Places', 'Activities', 'Objects', 'Symbols', 'Flags')
$names = @{}
$order = 0
foreach ($line in [IO.File]::ReadLines($UnicodeData)) {
    if ($line -match '^# group: (.+)$') { $group = $Matches[1]; continue }
    if ($line -notmatch '^([0-9A-F ]+)\s*;\s*\S+\s*#\s*\S+\s+E[\d.]+\s+(.+)$') { continue }
    $key = (($Matches[1].Trim().ToLowerInvariant() -split ' ') | Where-Object { $_ -and $_ -ne 'fe0f' } | ForEach-Object { $_.TrimStart('0') }) -join '-'
    if ($names.ContainsKey($key)) { continue }
    $category = [Array]::IndexOf($groups, $group)
    if ($category -lt 0) { $category = 1 } # Isolated skin/hair components belong with people.
    $names[$key] = @{ Name = $Matches[2]; Category = $category; Order = $order++ }
}

$choices = [Collections.Generic.List[object]]::new()
$agent = [Collections.Generic.List[string]]::new()
$usedCodes = @{}
foreach ($line in [IO.File]::ReadLines($atlasCatalog)) {
    $parts = $line.Split("`t")
    $key = (($parts[0] -split '-') | Where-Object { $_ -ne 'fe0f' }) -join '-'
    $unicode = -join (($parts[0] -split '-') | ForEach-Object { [char]::ConvertFromUtf32([Convert]::ToInt32($_, 16)) })
    $info = $names[$key]
    if (!$info) { $info = @{ Name = "Emoji $($parts[0])"; Category = 6; Order = $order++ } }
    $code = $knownAliases[$unicode.Replace([string][char]0xFE0F, '')]
    if (!$code) {
        $slug = $info.Name.Normalize([Text.NormalizationForm]::FormD) -replace '\p{M}', ''
        $slug = ($slug.ToLowerInvariant() -replace '[^a-z0-9]+', '_').Trim('_')
        $code = ":${slug}:"
        if ($aliases.ContainsKey($code) -and $aliases[$code].Replace([string][char]0xFE0F, '') -ne $unicode.Replace([string][char]0xFE0F, '')) {
            $code = ':' + $slug + '_' + $parts[0].Replace('-', '_') + ':'
        }
    }
    if ($usedCodes.ContainsKey($code)) { $code = ':emoji_' + $parts[0].Replace('-', '_') + ':' }
    $usedCodes[$code] = $true
    # Keep legacy non-RGI assets available for chat, but offer Unicode keyboard entries in the picker.
    if ($names.ContainsKey($key)) {
        $choices.Add([pscustomobject]@{ Code = $code; Name = $info.Name; Category = $info.Category;
            Atlas = $parts[1]; Image = 'e_' + $parts[0].Replace('-', '_'); Order = $info.Order })
    }
    $agent.Add($parts[0] + "`t" + $parts[1] + "`t" + $code)
}
$glyphIndex = 0
$picker = @($choices | Sort-Object Order | ForEach-Object {
    $glyph = [char](0xE000 + $glyphIndex)
    $glyphIndex++
    $_.Code, $_.Name, $_.Category, $_.Atlas, $_.Image, $glyph -join "`t"
})
$utf8 = [Text.UTF8Encoding]::new($false)
[IO.File]::WriteAllLines((Join-Path $root 'gui/emoji/EmojiPicker.txt'), $picker, $utf8)
[IO.File]::WriteAllLines($atlasCatalog, $agent, $utf8)
Write-Host "Built $($choices.Count) picker choices, including $(@($choices | Where-Object Category -eq 8).Count) flags."
Write-Host 'Copy the updated EmojiCatalog.tsv to the Agent when updating the picker.'
