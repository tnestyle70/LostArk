#!/usr/bin/env python3
"""Bake a DirectXTK .spritefont from the retail TTF at an exact line spacing.

Why this exists
---------------
The shipped atlases are baked once per family (YoonGasiIIM at 32 px line
spacing, YG760 at 42) and SpriteBatch resamples them for whatever size a label
asks for.  Anything far from the baked size looks wrong: a 16 pt row is a 0.38x
minify with no mips and turns to mush, a 110 pt title is a 3.4x magnify off a
27 px glyph and looks like a blown-up screenshot.

UILabelFont::Resolve already fixes this for 10..20 px by picking a
pre-downsampled variant, but those were Lanczos downsamples of the 32 px atlas,
so they stop where the original does.  Larger sizes have to be re-rasterised
from the TTF, which is what this does.

Character set
-------------
Taken from an existing .spritefont so a new size covers exactly what the old one
did -- no glyph silently missing at one size and present at another.  Pass
--charset-from to copy it, or --ascii for a Latin-only atlas (used for the
display sizes where a full Korean set would be hundreds of megabytes and no
Korean is ever drawn).

Texture format
--------------
BC2, matching the shipped files.  A font atlas is white with a coverage alpha,
which BC2 stores exactly: the 4-bit alpha block is the coverage and the colour
block is a constant white, so there is nothing to search for and no loss beyond
the alpha quantisation the shipped files already have.

Usage:
  python bake_spritefont.py --ttf <src.ttf> --line-spacing 40 --out <dst.spritefont>
                            [--charset-from <existing.spritefont> | --ascii]
                            [--atlas-width 4096]
"""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

MAGIC = b"DXTKfont"
GLYPH_STRUCT = struct.Struct("<Iiiiifff")
DXGI_FORMAT_BC2_UNORM = 74
PADDING = 1


def read_charset(path: Path) -> list[int]:
    with path.open("rb") as f:
        if f.read(8) != MAGIC:
            raise SystemExit(f"{path} is not a DXTKfont file")
        count = struct.unpack("<I", f.read(4))[0]
        data = f.read(count * GLYPH_STRUCT.size)
    return [GLYPH_STRUCT.unpack_from(data, i * GLYPH_STRUCT.size)[0]
            for i in range(count)]


def pick_pixel_size(ttf: Path, line_spacing: int) -> tuple[ImageFont.FreeTypeFont, int]:
    """Largest pixel size whose ascent+descent still fits the wanted spacing."""
    best = None
    for size in range(4, line_spacing * 3):
        font = ImageFont.truetype(str(ttf), size)
        ascent, descent = font.getmetrics()
        if ascent + descent <= line_spacing:
            best = (font, size)
        else:
            break
    if best is None:
        raise SystemExit(f"no pixel size fits line spacing {line_spacing}")
    return best


def render_glyphs(font: ImageFont.FreeTypeFont, codepoints: list[int],
                  line_spacing: int):
    """One entry per codepoint: (ink image or None, xoff, yoff, advance).

    The ink box is taken from the rendered bitmap, not from textbbox: Pillow's
    layout box starts at the pen and keeps the side bearings, so trusting it
    gives every glyph a zero X offset and a subrect padded out to the advance.
    Draw onto a generous canvas, then crop to what actually got ink."""
    margin = line_spacing
    canvas_w = line_spacing * 3 + margin * 2
    canvas_h = line_spacing * 2 + margin * 2
    out = []
    for cp in codepoints:
        ch = chr(cp)
        advance = font.getlength(ch)
        sheet = Image.new("L", (canvas_w, canvas_h), 0)
        try:
            ImageDraw.Draw(sheet).text((margin, margin), ch, font=font, fill=255)
        except Exception:
            out.append((None, 0, 0, advance))
            continue
        box = sheet.getbbox()
        if box is None:
            out.append((None, 0, 0, advance))
            continue
        cell = sheet.crop(box)
        # Offsets are relative to the pen, which sat at (margin, margin).
        out.append((cell, box[0] - margin, box[1] - margin, advance))
    return out


def pack(rendered, atlas_width: int):
    """Shelf packing, tallest-shelf-first is unnecessary here: glyphs of one
    font size are near-uniform, so a simple left-to-right shelf wastes little."""
    placements = []
    x = y = shelf_h = 0
    for cell, _xoff, _yoff, _adv in rendered:
        if cell is None:
            placements.append(None)
            continue
        w, h = cell.size
        if x + w + PADDING > atlas_width:
            x = 0
            y += shelf_h + PADDING
            shelf_h = 0
        placements.append((x, y))
        x += w + PADDING
        shelf_h = max(shelf_h, h)
    height = y + shelf_h + PADDING
    return placements, ((height + 3) // 4) * 4


# The colour half is a white-to-black ramp, so a 2-bit index picks one of four
# greys.  With color0 white and color1 black the palette is:
#   index 0 -> 255, index 1 -> 0, index 2 -> 170, index 3 -> 85
_GREY_INDEX = ((255, 0), (170, 2), (85, 3), (0, 1))


def encode_bc2(image: Image.Image) -> bytes:
    """BC2 with the coverage in BOTH halves.

    The shipped atlases hold premultiplied alpha -- RGB equals the coverage, not
    a constant white -- because SpriteBatch draws them through the premultiplied
    blend state.  A constant-white colour half makes every texel inside a
    glyph's subrect opaque white, which draws the glyph box as a solid block
    instead of the glyph.  So the alpha half carries the coverage at 4 bits and
    the colour half carries the same value quantised to the ramp's four greys,
    exactly as the shipped files do."""
    w, h = image.size
    pixels = image.load()
    out = bytearray()
    for by in range(0, h, 4):
        for bx in range(0, w, 4):
            alpha = bytearray(8)
            indices = 0
            for row in range(4):
                bits = 0
                for col in range(4):
                    px = pixels[bx + col, by + row] if (
                        bx + col < w and by + row < h) else 0
                    bits |= (px >> 4) << (col * 4)
                    nearest = min(_GREY_INDEX, key=lambda g: abs(g[0] - px))[1]
                    indices |= nearest << ((row * 4 + col) * 2)
                alpha[row * 2] = bits & 0xFF
                alpha[row * 2 + 1] = (bits >> 8) & 0xFF
            out += alpha
            out += struct.pack("<HHI", 0xFFFF, 0x0000, indices)
    return bytes(out)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--ttf", type=Path, required=True)
    parser.add_argument("--line-spacing", type=int, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--charset-from", type=Path)
    parser.add_argument("--ascii", action="store_true")
    parser.add_argument("--atlas-width", type=int, default=4096)
    args = parser.parse_args()

    if args.ascii:
        codepoints = list(range(32, 127))
    elif args.charset_from:
        codepoints = read_charset(args.charset_from)
    else:
        raise SystemExit("pass --charset-from or --ascii")
    codepoints = sorted(set(codepoints))

    font, pixel_size = pick_pixel_size(args.ttf, args.line_spacing)
    rendered = render_glyphs(font, codepoints, args.line_spacing)
    placements, atlas_height = pack(rendered, args.atlas_width)

    atlas = Image.new("L", (args.atlas_width, atlas_height), 0)
    glyphs = bytearray()
    for cp, (cell, xoff, yoff, advance), place in zip(
            codepoints, rendered, placements):
        if cell is None:
            # DXTK still needs a subrect; a 1x1 empty cell keeps the advance.
            left = top = 0
            right = bottom = 1
            x_advance = advance - 1
        else:
            atlas.paste(cell, place)
            left, top = place
            right, bottom = left + cell.size[0], top + cell.size[1]
            # ForEachGlyph moves the pen by XOffset, then by subrect width plus
            # XAdvance, and never takes the XOffset back -- so the stored
            # XAdvance is whatever is left of the real advance after both.
            x_advance = advance - cell.size[0] - xoff
        glyphs += GLYPH_STRUCT.pack(
            cp, left, top, right, bottom, float(xoff), float(yoff),
            float(x_advance))

    texture = encode_bc2(atlas)
    stride = (args.atlas_width // 4) * 16
    rows = atlas_height // 4

    with args.out.open("wb") as f:
        f.write(MAGIC)
        f.write(struct.pack("<I", len(codepoints)))
        f.write(glyphs)
        f.write(struct.pack("<f", float(args.line_spacing)))
        f.write(struct.pack("<I", 0))
        f.write(struct.pack("<IIIII", args.atlas_width, atlas_height,
                            DXGI_FORMAT_BC2_UNORM, stride, rows))
        f.write(texture)

    print("%-32s px=%-3d glyphs=%-6d atlas=%dx%d  %.1f MB"
          % (args.out.name, pixel_size, len(codepoints), args.atlas_width,
             atlas_height, args.out.stat().st_size / 1048576.0))
    return 0


if __name__ == "__main__":
    sys.exit(main())
