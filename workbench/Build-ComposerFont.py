"""Reserve one blank, 18px-wide native font glyph per picker emoji.

DayZ still handles editing and selection. The composer draws the emoji image
over each reserved glyph and converts it back to a shortcode when read.
The source must come from the installed stable game's dta/gui.pbo.
"""

from pathlib import Path
import shutil
import struct
import sys


def chunk(tag: bytes, data: bytes) -> bytes:
    return tag + struct.pack(">I", len(data)) + data


def build(source: Path) -> None:
    root = Path(__file__).resolve().parent.parent
    picker = root / "gui/emoji/EmojiPicker.tsv"
    rows = [line.split("\t")[:5] for line in picker.read_text(encoding="utf-8").splitlines()]
    first_glyph = 0xE000
    if first_glyph + len(rows) > 0xF000:
        raise ValueError("The picker exceeds the reserved emoji glyph range.")

    font = (source / "metronbook16.fnt").read_bytes()
    if font[:4] != b"FORM" or font[8:12] != b"FNT3":
        raise ValueError("Expected the stable game's FNT3 bitmap font.")
    chunks = []
    offset = 12
    while offset < len(font):
        tag = font[offset:offset + 4]
        length = struct.unpack_from(">I", font, offset + 4)[0]
        data = font[offset + 8:offset + 8 + length]
        chunks.append((tag, data))
        offset += 8 + length

    glyphs = dict(chunks)[b"GLPS"]
    size, flags, count, ranges = struct.unpack_from("<4I", glyphs)
    if size != len(glyphs) or len(glyphs) != 16 + ranges * 4:
        raise ValueError("Unexpected glyph range table.")
    entries = [struct.unpack_from("<HH", glyphs, 16 + i * 4) for i in range(ranges)]
    coordinates = dict(chunks)[b"TCRD"]
    if len(coordinates) != count * 12:
        raise ValueError("Unexpected glyph coordinate table.")
    insert_at = sum(length for start, length in entries if start < first_glyph)
    if any(start < first_glyph + len(rows) and start + length > first_glyph for start, length in entries):
        raise ValueError("The source font already uses the reserved emoji range.")
    # The space glyph is transparent; retain its UVs and reserve 18px of advance.
    space = struct.unpack_from("<6h", coordinates)
    blank = struct.pack("<6h", *space[:4], 0, 18 - space[2])
    entries.append((first_glyph, len(rows)))
    entries.sort()
    updated = {
        b"GLPS": struct.pack("<4I", size + 4, flags, count + len(rows), ranges + 1)
        + b"".join(struct.pack("<HH", *entry) for entry in entries),
        b"TCRD": coordinates[:insert_at * 12] + blank * len(rows) + coordinates[insert_at * 12:],
    }
    body = b"FNT3" + b"".join(chunk(tag, updated.get(tag, data)) for tag, data in chunks)
    output = root / "gui/fonts"
    output.mkdir(exist_ok=True)
    (output / "Composer16.fnt").write_bytes(b"FORM" + struct.pack(">I", len(body)) + body)
    shutil.copyfile(source / "metronbook16.edds", output / "Composer16.edds")
    picker.write_text("\n".join("\t".join(row + [chr(first_glyph + i)]) for i, row in enumerate(rows)) + "\n", encoding="utf-8")
    print(f"Built composer font with {len(rows)} reserved emoji glyphs.")


if __name__ == "__main__":
    build(Path(sys.argv[1]))
