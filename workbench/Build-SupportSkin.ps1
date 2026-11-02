$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
. "$PSScriptRoot/Export-UiTexture.ps1"
$root = Split-Path $PSScriptRoot -Parent
$output = Join-Path $root 'gui/skin'
$bitmap = [Drawing.Bitmap]::new(256, 64, [Drawing.Imaging.PixelFormat]::Format32bppArgb)
$definitions = [Collections.Generic.List[string]]::new()

function Add-RoundedTile([string]$name, [int]$offsetX, [int]$radius, [int]$border) {
    for ($y = 0; $y -lt 64; $y++) {
        for ($x = 0; $x -lt 64; $x++) {
            $dx = [Math]::Max(0, [Math]::Abs($x + 0.5 - 32) - (32 - $radius))
            $dy = [Math]::Max(0, [Math]::Abs($y + 0.5 - 32) - (32 - $radius))
            $distance = [Math]::Sqrt($dx * $dx + $dy * $dy)
            $coverage = 0
            if ($distance -le $radius - 0.708) { $coverage = 1 }
            elseif ($distance -lt $radius + 0.708) {
                # Integrate boundary pixels with 8x8 samples; all four corners use the same mask.
                $inside = 0
                for ($sy = 0; $sy -lt 8; $sy++) {
                    for ($sx = 0; $sx -lt 8; $sx++) {
                        $sampleX = [Math]::Max(0, [Math]::Abs($x + ($sx + 0.5) / 8 - 32) - (32 - $radius))
                        $sampleY = [Math]::Max(0, [Math]::Abs($y + ($sy + 0.5) / 8 - 32) - (32 - $radius))
                        if ($sampleX * $sampleX + $sampleY * $sampleY -le $radius * $radius) { $inside++ }
                    }
                }
                $coverage = $inside / 64.0
            }
            $alpha = [int][Math]::Round(255 * $coverage)
            $bitmap.SetPixel($offsetX + $x, $y, [Drawing.Color]::FromArgb($alpha, 255, 255, 255))
        }
    }
    if ($border -eq 0) {
        $definitions.Add(@"
  ImageSetDefClass $name {
   Name "$name"
   Pos $offsetX 0
   Size 64 64
   Flags 0
  }
"@)
        return
    }
    $axis = @(0, $border, (64 - $border))
    $sizes = @($border, (64 - 2 * $border), $border)
    $names = @('LeftTop', 'Top', 'RightTop', 'Left', 'Center', 'Right', 'LeftBottom', 'Bottom', 'RightBottom')
    for ($row = 0; $row -lt 3; $row++) {
        for ($column = 0; $column -lt 3; $column++) {
            $part = $name + $names[$row * 3 + $column]
            $definitions.Add(@"
  ImageSetDefClass $part {
   Name "$part"
   Pos $($offsetX + $axis[$column]) $($axis[$row])
   Size $($sizes[$column]) $($sizes[$row])
   Flags 0
  }
"@)
        }
    }
}

try {
    # Preserve the native skin's dimensions, using identical coverage in all four corners.
    Add-RoundedTile 'Round' 0 8 8
    Add-RoundedTile 'Pill' 64 16 16
    Add-RoundedTile 'Circle' 128 32 0
    $png = Join-Path $output 'support_ca.png'
    $bitmap.Save($png, [Drawing.Imaging.ImageFormat]::Png)
    Export-UiTexture $bitmap (Join-Path $output 'support.edds')
} finally { $bitmap.Dispose() }
$imageset = @"
ImageSetClass {
 Name "ServerSentrySupport"
 RefSize 256 64
 Textures {
  ImageSetTextureClass {
   mpix 0
   path "ServerSentryDayZMod/gui/skin/support.edds"
  }
 }
 Images {
$($definitions -join "`n")
 }
}
"@
[IO.File]::WriteAllText((Join-Path $output 'Support.imageset'), $imageset, [Text.UTF8Encoding]::new($false))
