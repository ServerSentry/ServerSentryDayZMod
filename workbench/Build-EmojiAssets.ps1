param(
    [string]$SourceDirectory,
    [string]$ImageToPaa = 'C:\Program Files (x86)\Steam\steamapps\common\DayZ Tools\Bin\ImageToPAA\ImageToPAA.exe'
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$cache = Join-Path $projectRoot '.workbench/emoji-source'
if (!$SourceDirectory) { $SourceDirectory = Join-Path $cache 'twemoji-17.0.3/assets/72x72' }
if (!(Test-Path -LiteralPath $SourceDirectory) -or !(Test-Path -LiteralPath $ImageToPaa)) {
    throw 'Twemoji 17.0.3 sources and Bohemia ImageToPAA are required. See gui/emoji/README.txt.'
}

Add-Type -AssemblyName System.Drawing
$output = Join-Path $projectRoot 'gui/emoji'
New-Item -ItemType Directory -Path $output, $cache -Force | Out-Null
# Sort code points without punctuation so PowerShell versions keep the same atlas assignments.
$files = @(Get-ChildItem -LiteralPath $SourceDirectory -Filter '*.png' | Sort-Object { $_.BaseName.Replace('-', '') })
$catalog = [Collections.Generic.List[string]]::new()
$utf8 = [Text.UTF8Encoding]::new($false)
$perAtlas = 28 * 28

for ($offset = 0; $offset -lt $files.Count; $offset += $perAtlas) {
    $atlas = [int]($offset / $perAtlas)
    $name = "ServerSentryEmoji$atlas"
    $bitmap = [Drawing.Bitmap]::new(1024, 1024, [Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $graphics = [Drawing.Graphics]::FromImage($bitmap)
    $graphics.Clear([Drawing.Color]::Transparent)
    $graphics.InterpolationMode = [Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $definitions = [Collections.Generic.List[string]]::new()
    try {
        for ($index = 0; $index -lt $perAtlas -and ($offset + $index) -lt $files.Count; $index++) {
            $file = $files[$offset + $index]
            $x = ($index % 28) * 36 + 2
            $y = [Math]::Floor($index / 28) * 36 + 2
            $image = [Drawing.Image]::FromFile($file.FullName)
            # Keep the visible contour inside the sprite's UV bounds when DayZ filters it.
            try { $graphics.DrawImage($image, [int]($x + 2), [int]($y + 2), 28, 28) }
            finally { $image.Dispose() }
            $imageName = 'e_' + $file.BaseName.Replace('-', '_')
            $definitions.Add(@"
  ImageSetDefClass $imageName {
   Name "$imageName"
   Pos $x $y
   Size 32 32
   Flags 0
  }
"@)
            $catalog.Add($file.BaseName + "`t" + $atlas)
        }
        $png = Join-Path $cache "emoji${atlas}_ca.png"
        $bitmap.Save($png, [Drawing.Imaging.ImageFormat]::Png)
    }
    finally { $graphics.Dispose(); $bitmap.Dispose() }

    $texture = "emoji${atlas}_ca.paa"
    & $ImageToPaa $png (Join-Path $output $texture)
    if ($LASTEXITCODE -ne 0) { throw "Could not convert emoji atlas $atlas." }
    $imageset = @"
ImageSetClass {
 Name "$name"
 RefSize 1024 1024
 Textures {
  ImageSetTextureClass {
   mpix 0
   path "ServerSentryDayZMod/gui/emoji/$texture"
  }
 }
 Images {
$($definitions -join "`n")
 }
}
"@
    [IO.File]::WriteAllText((Join-Path $output "$name.imageset"), $imageset, $utf8)
}
[IO.File]::WriteAllLines((Join-Path $cache 'EmojiCatalog.tsv'), $catalog, $utf8)
Write-Host "Built $($files.Count) emoji images. Copy EmojiCatalog.tsv to the Agent's DayZ/Formatting directory when updating the atlas."
