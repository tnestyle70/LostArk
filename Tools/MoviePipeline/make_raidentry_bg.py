# -*- coding: utf-8 -*-
"""Turn extracted commander-movie PNG frames into raid-entry background flipbook DDS.

Source frames are RAD-extracted PNGs (1200x848, RGB, 300 frames @30fps).
Output: DXT1 (BC1, opaque) DDS at the boss-portrait slot size (800x560, /4 for BCn),
named <prefix>_NNN.dds, into Client/Bin/Resources/UI/Bern/<folder>/.

DXT1 = 0.5 byte/px -> ~220KB/frame, 300 frames -> ~66MB per boss in VRAM (GPU-native,
no decompress). The UI runtime already loads .dds via CreateDDSTextureFromFileEx.
"""
import os, glob
from PIL import Image

REPO = r"C:\Users\엄태준\OneDrive\Desktop\Lost Ark"
DEST = os.path.join(REPO, r"Client\Bin\Resources\UI\Bern")
W, H = 800, 560  # boss-portrait slot (~800x562), rounded to /4 for block compression

# Alpha-preserving RAD extraction (option 4: filter premultiplied -> standard alpha). The
# movie's top region is transparent (alpha=0), not the flat grey a no-alpha export bakes in.
JOBS = [
    (r"D:\ClaudeWork\movietest\out\발탄", "RaidEntry_BG_Valtan"),
    (r"D:\ClaudeWork\movietest\out\쿠크", "RaidEntry_BG_Kukusaton"),
]


def main():
    for src, folder in JOBS:
        outdir = os.path.join(DEST, folder)
        os.makedirs(outdir, exist_ok=True)
        frames = sorted(glob.glob(os.path.join(src, "*.png")))
        if not frames:
            print(f"[skip] {folder}: no source frames in {src}")
            continue
        for i, fp in enumerate(frames):
            # Premultiply against black before the resize so LANCZOS doesn't bleed opaque colour
            # out under transparent pixels (fringing), then keep straight alpha for DXT5.
            im = Image.open(fp).convert("RGBA").resize((W, H), Image.LANCZOS)
            outp = os.path.join(outdir, f"{folder}_{i:03d}.dds")
            im.save(outp, format="DDS", pixel_format="DXT5")
        total = sum(os.path.getsize(os.path.join(outdir, f))
                    for f in os.listdir(outdir) if f.endswith(".dds"))
        print(f"[ok] {folder}: {len(frames)} DDS, {total/1024/1024:.1f} MB on disk -> {outdir}")


if __name__ == "__main__":
    main()
