param([string]$LogoPath = "$PSScriptRoot/assets/ServerSentryLogo.png")

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
. "$PSScriptRoot/Export-UiTexture.ps1"
$root = Split-Path $PSScriptRoot -Parent
$output = Join-Path $root 'gui/skin'

function Export-Icon([Drawing.Bitmap]$source, [string]$name) {
    $icon = [Drawing.Bitmap]::new(32, 32, [Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $graphics = [Drawing.Graphics]::FromImage($icon)
    try {
        $graphics.CompositingMode = [Drawing.Drawing2D.CompositingMode]::SourceCopy
        $graphics.InterpolationMode = [Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
        $graphics.PixelOffsetMode = [Drawing.Drawing2D.PixelOffsetMode]::HighQuality
        $graphics.DrawImage($source, [Drawing.Rectangle]::new(0, 0, 32, 32))
        Export-UiTexture $icon (Join-Path $output ($name + '.edds'))
    } finally { $graphics.Dispose(); $icon.Dispose() }
}

# Fit the visible mark, rather than its transparent margins, into its physical display size.
$source = [Drawing.Bitmap]::FromFile($LogoPath)
$logo = [Drawing.Bitmap]::new(128, 128, [Drawing.Imaging.PixelFormat]::Format32bppArgb)
$graphics = [Drawing.Graphics]::FromImage($logo)
try {
    $left = $source.Width; $top = $source.Height; $right = 0; $bottom = 0
    for ($y = 0; $y -lt $source.Height; $y++) {
        for ($x = 0; $x -lt $source.Width; $x++) {
            if ($source.GetPixel($x, $y).A -lt 32) { continue }
            $left = [Math]::Min($left, $x); $right = [Math]::Max($right, $x)
            $top = [Math]::Min($top, $y); $bottom = [Math]::Max($bottom, $y)
        }
    }
    $side = [Math]::Max($right - $left + 1, $bottom - $top + 1) + 24
    $crop = [Drawing.Rectangle]::new(($left + $right + 1 - $side) / 2, ($top + $bottom + 1 - $side) / 2, $side, $side)
    $graphics.InterpolationMode = [Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $graphics.DrawImage($source, [Drawing.Rectangle]::new(0, 0, 128, 128), $crop, [Drawing.GraphicsUnit]::Pixel)
    Export-UiTexture $logo (Join-Path $output 'serversentry_logo.edds')
} finally { $graphics.Dispose(); $logo.Dispose(); $source.Dispose() }

# Draw at eight times the final size, then antialias once into a pixel-sized UI texture.
foreach ($name in @('back', 'refresh', 'ticket', 'reimbursement', 'close', 'smile', 'switch_thumb')) {
    $source = [Drawing.Bitmap]::new(256, 256, [Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $graphics = [Drawing.Graphics]::FromImage($source)
    $pen = [Drawing.Pen]::new([Drawing.Color]::White, 4)
    try {
        $graphics.SmoothingMode = [Drawing.Drawing2D.SmoothingMode]::AntiAlias
        $graphics.ScaleTransform(4, 4)
        $pen.StartCap = $pen.EndCap = [Drawing.Drawing2D.LineCap]::Round
        $pen.LineJoin = [Drawing.Drawing2D.LineJoin]::Round
        switch ($name) {
            'back' {
                $graphics.DrawLine($pen, 12, 32, 52, 32)
                $graphics.DrawLines($pen, [Drawing.Point[]]@([Drawing.Point]::new(30, 14), [Drawing.Point]::new(12, 32), [Drawing.Point]::new(30, 50)))
            }
            'refresh' {
                $graphics.DrawArc($pen, 12, 12, 40, 40, 35, 285)
                $graphics.DrawLines($pen, [Drawing.Point[]]@([Drawing.Point]::new(36, 20), [Drawing.Point]::new(50, 20), [Drawing.Point]::new(50, 6)))
            }
            'ticket' {
                $graphics.DrawRectangle($pen, 16, 12, 32, 40)
                $graphics.DrawLine($pen, 24, 24, 40, 24)
                $graphics.DrawLine($pen, 24, 33, 40, 33)
                $graphics.DrawLine($pen, 24, 42, 34, 42)
            }
            'reimbursement' {
                $graphics.DrawLines($pen, [Drawing.Point[]]@(
                    [Drawing.Point]::new(12, 22), [Drawing.Point]::new(32, 12),
                    [Drawing.Point]::new(52, 22), [Drawing.Point]::new(52, 44),
                    [Drawing.Point]::new(32, 54), [Drawing.Point]::new(12, 44),
                    [Drawing.Point]::new(12, 22), [Drawing.Point]::new(32, 32),
                    [Drawing.Point]::new(52, 22)))
                $graphics.DrawLine($pen, 32, 32, 32, 54)
            }
            'close' {
                $graphics.DrawLine($pen, 16, 16, 48, 48)
                $graphics.DrawLine($pen, 48, 16, 16, 48)
            }
            'smile' {
                $graphics.DrawEllipse($pen, 10, 10, 44, 44)
                $graphics.DrawArc($pen, 21, 24, 22, 20, 15, 150)
                $graphics.FillEllipse([Drawing.Brushes]::White, 22, 23, 5, 5)
                $graphics.FillEllipse([Drawing.Brushes]::White, 37, 23, 5, 5)
            }
            'switch_thumb' {
                # A transparent gutter prevents the neighbouring atlas tile from bleeding into the knob.
                $graphics.FillEllipse([Drawing.Brushes]::White, 2, 2, 60, 60)
            }
        }
        Export-Icon $source $name
    } finally { $pen.Dispose(); $graphics.Dispose(); $source.Dispose() }
}
$discord = [Drawing.Bitmap]::FromFile((Join-Path $PSScriptRoot 'assets/DiscordSymbol.png'))
try { Export-Icon $discord 'discord' } finally { $discord.Dispose() }
