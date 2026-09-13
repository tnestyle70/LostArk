#!/usr/bin/env python3
"""Check baked .spritefont files against what SpriteBatch expects of them.

The check that matters most is the last one.  A font atlas drawn through
DirectXTK's default (premultiplied) blend state has to carry premultiplied
alpha: the colour half must track the coverage, not sit at a constant white.
A constant-white colour half still passes an alpha-only inspection -- the
glyph's coverage is all there -- but every texel inside the glyph's box draws
opaque, so the page shows solid blocks where the letters should be.

Usage:
  python verify_spritefont.py <file.spritefont> [more...]
"""

from __future__ import annotations

import struct
import sys
from pathlib import Path

MAGIC = b"DXTKfont"
GLYPH = struct.Struct("<Iiiiifff")
DXGI_FORMAT_BC2_UNORM = 74


def load(path: Path):
    with path.open("rb") as f:
        if f.read(8) != MAGIC:
            raise SystemExit(f"{path}: not a DXTKfont file")
        count = struct.unpack("<I", f.read(4))[0]
        glyphs = f.read(count * GLYPH.size)
        line_spacing = struct.unpack("<f", f.read(4))[0]
        f.read(4)
        tw, th, fmt, stride, rows = struct.unpack("<IIIII", f.read(20))
        texture = f.read()
    return count, glyphs, line_spacing, tw, th, fmt, stride, rows, texture


def decode_block(block: bytes):
    alpha = []
    for row in range(4):
        bits = block[row * 2] | (block[row * 2 + 1] << 8)
        alpha.append([((bits >> (c * 4)) & 0xF) * 17 for c in range(4)])
    c0, c1 = struct.unpack_from("<HH", block, 8)
    idx = struct.unpack_from("<I", block, 12)[0]

    def rgb565(v):
        return (((v >> 11) & 0x1F) * 255 // 31,
                ((v >> 5) & 0x3F) * 255 // 63,
                (v & 0x1F) * 255 // 31)

    a, b = rgb565(c0), rgb565(c1)
    palette = [a, b,
               tuple((2 * a[i] + b[i]) // 3 for i in range(3)),
               tuple((a[i] + 2 * b[i]) // 3 for i in range(3))]
    rgb = [[palette[(idx >> ((row * 4 + c) * 2)) & 3] for c in range(4)]
           for row in range(4)]
    return alpha, rgb


def check(path: Path) -> bool:
    count, glyphs, line_spacing, tw, th, fmt, stride, rows, texture = load(path)
    problems = []

    if fmt != DXGI_FORMAT_BC2_UNORM:
        problems.append(f"texture format {fmt}, expected BC2 ({DXGI_FORMAT_BC2_UNORM})")
    if len(texture) != stride * rows:
        problems.append(f"texture is {len(texture)} bytes, header says {stride * rows}")
    if stride != (tw // 4) * 16 or rows != th // 4:
        problems.append("stride/rows do not match the atlas size")

    previous = -1
    for i in range(count):
        cp, l, t, r, b, xo, yo, xa = GLYPH.unpack_from(glyphs, i * GLYPH.size)
        if cp <= previous:
            problems.append("glyphs are not in ascending codepoint order")
            break
        previous = cp
        if l < 0 or t < 0 or r > tw or b > th or r <= l or b <= t:
            problems.append(f"U+{cp:04X} subrect is outside the atlas")
            break

    # Sample the glyphs with the most ink; a blank one proves nothing.
    ink = 0
    mismatched = 0
    sampled = 0
    for i in range(count):
        cp, l, t, r, b, xo, yo, xa = GLYPH.unpack_from(glyphs, i * GLYPH.size)
        if (r - l) < 4 or (b - t) < 4 or sampled >= 24:
            continue
        sampled += 1
        for by in range(t // 4, (b + 3) // 4):
            for bx in range(l // 4, (r + 3) // 4):
                off = by * stride + bx * 16
                alpha, rgb = decode_block(texture[off:off + 16])
                for row in range(4):
                    for col in range(4):
                        a = alpha[row][col]
                        if a > 32:
                            ink += 1
                        # Premultiplied: the grey must follow the coverage. The
                        # colour half only has four levels, so allow a step.
                        if abs(a - rgb[row][col][0]) > 96:
                            mismatched += 1

    if sampled == 0:
        problems.append("no glyph large enough to sample")
    elif ink == 0:
        problems.append("sampled glyphs have no ink at all")
    elif mismatched * 100 > ink * 5:
        problems.append(
            f"colour half does not follow the alpha ({mismatched} texels off) "
            f"-- the atlas is not premultiplied and will draw as solid blocks")

    status = "OK  " if not problems else "FAIL"
    print("%s %-34s glyphs=%-6d ls=%-5.0f atlas=%dx%d"
          % (status, path.name, count, line_spacing, tw, th))
    for problem in problems:
        print("       %s" % problem)
    return not problems


def main() -> int:
    paths = [Path(p) for p in sys.argv[1:]]
    if not paths:
        raise SystemExit(__doc__)
    return 0 if all(check(p) for p in paths) else 1


if __name__ == "__main__":
    sys.exit(main())
