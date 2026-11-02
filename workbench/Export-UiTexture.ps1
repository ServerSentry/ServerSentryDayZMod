function Export-UiTexture([Drawing.Bitmap]$Bitmap, [string]$Path) {
    # Standard BGRA DDS, one uncompressed level: no DXT alpha quantization or reduced UI mipmaps.
    $writer = [IO.BinaryWriter]::new([IO.File]::Create($Path))
    try {
        $writer.Write([Text.Encoding]::ASCII.GetBytes('DDS '))
        foreach ($value in @(124, 0x100F, $Bitmap.Height, $Bitmap.Width, ($Bitmap.Width * 4), 0, 0)) {
            $writer.Write([uint32]$value)
        }
        for ($index = 0; $index -lt 11; $index++) { $writer.Write([uint32]0) }
        foreach ($value in @(32, 0x41, 0, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 4278190080, 0x1000, 0, 0, 0, 0)) {
            $writer.Write([uint32]$value)
        }
        $rectangle = [Drawing.Rectangle]::new(0, 0, $Bitmap.Width, $Bitmap.Height)
        $pixels = $Bitmap.LockBits($rectangle, [Drawing.Imaging.ImageLockMode]::ReadOnly, [Drawing.Imaging.PixelFormat]::Format32bppArgb)
        try {
            $row = [byte[]]::new($Bitmap.Width * 4)
            for ($y = 0; $y -lt $Bitmap.Height; $y++) {
                [Runtime.InteropServices.Marshal]::Copy([IntPtr]::Add($pixels.Scan0, $y * $pixels.Stride), $row, 0, $row.Length)
                $writer.Write($row)
            }
        } finally { $Bitmap.UnlockBits($pixels) }
    } finally { $writer.Dispose() }
}
