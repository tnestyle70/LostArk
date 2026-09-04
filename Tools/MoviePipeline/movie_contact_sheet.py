# -*- coding: utf-8 -*-
"""Contact sheet + alpha bounding box for decrypted Bink movies, via bink2w64.dll.

  python movie_contact_sheet.py <out_dir> <movie.bk2|.bik> [...]

Writes <out_dir>/<stem>_sheet.png (8 evenly spaced frames over black) and prints
the union bounding box of alpha > 0 across all frames -- the crop that keeps the
whole cut-in.
"""
import os, sys
import numpy as np
from PIL import Image
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from bink_to_png import BinkDecoder

COLS, TILE_W = 4, 480


def main(argv):
    out_dir = argv[0]
    os.makedirs(out_dir, exist_ok=True)
    for src in argv[1:]:
        stem = os.path.splitext(os.path.basename(src))[0]
        dec = BinkDecoder(src)
        picks = sorted({int(round(i * (dec.frames - 1) / 7)) for i in range(8)})
        tiles = []
        x0, y0, x1, y1 = dec.width, dec.height, -1, -1
        for i in range(dec.frames):
            img = dec.frame_rgba()
            a = np.asarray(img.getchannel("A"))
            ys, xs = np.nonzero(a)
            if len(xs):
                x0, y0 = min(x0, int(xs.min())), min(y0, int(ys.min()))
                x1, y1 = max(x1, int(xs.max())), max(y1, int(ys.max()))
            if i in picks:
                bg = Image.new("RGBA", img.size, (0, 0, 0, 255))
                bg.alpha_composite(img)
                tile = bg.convert("RGB").resize((TILE_W, TILE_W * dec.height // dec.width))
                tiles.append((i, tile))
        dec.close()
        th = tiles[0][1].height
        rows = (len(tiles) + COLS - 1) // COLS
        sheet = Image.new("RGB", (COLS * TILE_W, rows * th), (40, 40, 40))
        for k, (_, tile) in enumerate(tiles):
            sheet.paste(tile, ((k % COLS) * TILE_W, (k // COLS) * th))
        sheet.save(os.path.join(out_dir, f"{stem}_sheet.png"))
        print(f"{stem:<32} {dec.width}x{dec.height} f={dec.frames} alpha bbox x={x0}..{x1} y={y0}..{y1} "
              f"({x1 - x0 + 1}x{y1 - y0 + 1}) frames={[t[0] for t in tiles]}")


if __name__ == "__main__":
    main(sys.argv[1:])
