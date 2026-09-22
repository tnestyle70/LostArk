# -*- coding: utf-8 -*-
"""Cook the commander-entrance movies into the raid-entry background flipbook.

Chain, all of it in this folder:

  1. dump_commander_bk2.py   decrypts every EFGame/Movies .ipk and writes the 1200x848
                             commander entrances as .bk2 plus a manifest naming each one
                             (movies_names.txt resolves the obfuscated file names).
  2. bink_to_png.py          decodes a .bk2 to straight-alpha PNGs with the game's own
                             bink2w64.dll.
  3. this script             saves those frames as DDS next to each other.

**Frames are kept at the movie's own 1200x848.** They used to be cooked down to 800x560,
the portrait slot's size in the 1280x720 authoring reference. That is only 1:1 on a
1280-wide window: the layout projects the slot by the real viewport, so a 1920-wide window
draws it at 1202x843 and the client was upscaling a frame it had already thrown detail
away from -- a 1200 -> 800 -> 1202 round trip. Every other UI asset in the project is cut
from retail's own 1920x1080 stage (the median authored-size / source-size across the UI
documents is 0.667), so native frames put this movie on that same footing: 1:1 at 1920,
downscaled and sharp below it.

The cost is disk and VRAM: 1200x848 is 2.27x the pixels of 800x560, so roughly 150 MB per
boss across 300 frames rather than 66.

DXT5, because the movie's top region is genuinely transparent (alpha 0) rather than the
flat grey a no-alpha export bakes in -- extract with RAD option 4 (filter premultiplied ->
standard alpha) or bink_to_png.py, which already writes straight alpha.
"""
import os, glob
from PIL import Image

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
DEST = os.path.join(REPO, r"Client\Bin\Resources\UI\RaidEntry")

# The movie's own size. Not the slot's -- see the note above.
W, H = 1200, 848

JOBS = [
    (r"D:\ClaudeWork\movietest\out_native\valtan", "RaidEntry_BG_Valtan"),
    (r"D:\ClaudeWork\movietest\out_native\kouku", "RaidEntry_BG_Kukusaton"),
]


def main():
    for src, folder in JOBS:
        outdir = os.path.join(DEST, folder)
        os.makedirs(outdir, exist_ok=True)
        frames = sorted(glob.glob(os.path.join(src, "*.png")))
        if not frames:
            print("[skip] %s: no source frames in %s" % (folder, src))
            continue
        for stale in glob.glob(os.path.join(outdir, "*.dds")):
            os.remove(stale)
        for i, fp in enumerate(frames):
            im = Image.open(fp).convert("RGBA")
            if im.size != (W, H):
                im = im.resize((W, H), Image.LANCZOS)
            im.save(os.path.join(outdir, "%s_%03d.dds" % (folder, i)), format="DDS",
                    pixel_format="DXT5")
        total = sum(os.path.getsize(os.path.join(outdir, f))
                    for f in os.listdir(outdir) if f.endswith(".dds"))
        print("[ok] %s: %d DDS at %dx%d, %.1f MB -> %s"
              % (folder, len(frames), W, H, total / 1024 / 1024, outdir))


if __name__ == "__main__":
    main()
