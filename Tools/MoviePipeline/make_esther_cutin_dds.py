# -*- coding: utf-8 -*-
"""Cook the Esther skill cut-in movies into full-screen DXT5 flipbooks.

Retail plays the 1920x1080 alpha movie into a screen-sized render target that
epicSkillAni.gfx pins to the bottom-right corner (pivot bottomRight, U=V=1,
fixedWidth 1920) -- i.e. a plain full-screen overlay. Our layout reference is
1280x720, so every frame is scaled to that and kept at 30 fps, every frame
(including fully transparent ones, so timing stays exact).

  python make_esther_cutin_dds.py [<src_dir>]

src_dir holds ESTHER_SKILL_<NAME>.bk2 (decrypted with ipk_to_bk2.py); output goes to
Client/Bin/Resources/UI/EstherCutin/<Name>/<Name>_NNN.dds.
"""
import os, sys
import numpy as np
from PIL import Image
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from bink_to_png import BinkDecoder

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
DEST = os.path.join(REPO, "Client", "Bin", "Resources", "UI", "EstherCutin")
W, H = 1280, 720
NAMES = ("Silian", "Waye", "Bahunturr", "Inanna", "Ninave")


def resize_premultiplied(img: Image.Image) -> Image.Image:
    """Bink leaves garbage RGB under alpha=0, so premultiply before filtering (no
    colour bleed into the edges), then return straight alpha for DXT5."""
    a = np.asarray(img, dtype=np.float32)
    rgb = a[..., :3] * (a[..., 3:4] / 255.0)
    pm = Image.fromarray(np.concatenate([rgb, a[..., 3:4]], axis=2).astype(np.uint8), "RGBA")
    pm = pm.resize((W, H), Image.LANCZOS)
    b = np.asarray(pm, dtype=np.float32)
    alpha = b[..., 3:4]
    rgb = np.where(alpha > 0, b[..., :3] * 255.0 / np.maximum(alpha, 1.0), 0.0)
    out = np.concatenate([np.clip(rgb, 0, 255), alpha], axis=2).astype(np.uint8)
    return Image.fromarray(out, "RGBA")


def main(argv):
    src_dir = argv[0] if argv else r"D:\ClaudeWork\movietest\esther_named"
    for name in NAMES:
        src = os.path.join(src_dir, f"ESTHER_SKILL_{name.upper()}.bk2")
        outdir = os.path.join(DEST, name)
        os.makedirs(outdir, exist_ok=True)
        dec = BinkDecoder(src)
        for i in range(dec.frames):
            frame = resize_premultiplied(dec.frame_rgba())
            frame.save(os.path.join(outdir, f"{name}_{i:03d}.dds"), format="DDS", pixel_format="DXT5")
        dec.close()
        total = sum(os.path.getsize(os.path.join(outdir, f)) for f in os.listdir(outdir) if f.endswith(".dds"))
        print(f"[ok] {name}: {dec.frames} DDS {W}x{H} DXT5, {total / 1024 / 1024:.1f} MB -> {outdir}", flush=True)


if __name__ == "__main__":
    main(sys.argv[1:])
