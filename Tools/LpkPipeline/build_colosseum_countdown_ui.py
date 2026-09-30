#!/usr/bin/env python3
"""Builds the art and the layout of the Colosseum match-start countdown (banner + red announce).

Retail source: EFUI_COLOSSEUM.colosseumplaying_loc_int (the in-match HUD movie of the Colosseum)
and EFUI_LOCALRESOURCE.localresource_loc_int (shared library, holds the class the movie links for
the red announce splash). Both were dumped with dump_upk_movie.py and read with
gfx_scene_extract.py; the images are the movies' own sub-images (rect = x0, y0, x1, y1 in the
atlas TGA that UModel exports):

  waitGroup_mc          top banner group of the waiting phase (colosseumMapTitle_lb, remainTimeStr_lb,
                        remainProgress). Its shape uses sub-image 389 (a 10 x 182 vertical gradient,
                        stretched to 2560 x 182).
  remainProgress        bar frame  = sub-image 348 (321 x 10, dark)
                        bar fill   = sub-image 352 (317 x 6, amber; the movie names it "track" but it
                                     is the fill, its width follows the remaining time)
  countDownAnnounce_mc  red splash behind "N sec later the match starts" = the linked class
                        colossenumPlaying_countDownAnnounce_bg = sub-image 369 of localresource
                        (306 x 94). "Match start!" reuses the same splash (measured on the retail
                        video: same size, same place).

Inputs : UModel TGA exports (umodel -export ... EFUI_COLOSSEUM and EFUI_LOCALRESOURCE):
         <tex>/colosseumplaying_loc_int_i<hex>.tga   and   <tex_shared>/localresource_loc_int_ie1.tga
Outputs: Client/Bin/Resources/UI/Colosseum/Countdown/*.png   (runtime input, Git-untracked)
         Data/UI/Colosseum/MatchCountdown_Layout.json         (lostark.ui-layout v1, 1280x720)

The layout only carries the default 16:9 placement. CColosseumMatchStart re-places every slot at
runtime from Data/Camera/ColosseumMatchStart.json (stage space, centred on the screen, uniform
scale by height) so an ultra-wide window does not stretch the art.

usage: build_colosseum_countdown_ui.py --repo <LostArk> --tex <dir> --tex-shared <dir>
"""
import argparse
import json
import os

from PIL import Image

S = 2.0 / 3.0  # stage 1920x1080 -> reference 1280x720

# png name: (atlas file, crop rect x0,y0,x1,y1) -- crop rects are the movies' sub-image rects.
ART = {
    "top_gradient": ("tex", "colosseumplaying_loc_int_ie.tga", (673, 704, 683, 886)),        # sub 389
    "bar_frame": ("tex", "colosseumplaying_loc_int_i15.tga", (55, 828, 376, 838)),           # sub 348
    "bar_fill": ("tex", "colosseumplaying_loc_int_i160.tga", (546, 443, 863, 449)),          # sub 352
    "announce_bg": ("shared", "localresource_loc_int_ie1.tga", (647, 795, 953, 889)),        # sub 369
}

# Layout in stage px, x measured from the screen centre (960). The banner group is drawn at
# BANNER_SCALE of the movie's authored size: the retail capture's bar is 258 px long against the
# movie's 317 (4 independent measurements -- bar length, bar row, title row, line row -- agree on 0.81).
BANNER_SCALE = 0.81
# waitGroup_mc sits at x -320 in the movie's stage, so its children's authored x is 320 too large.
SLOTS = [
    # id, png, dx, y, w, h (stage px, banner slots already scaled)
    ("Count_TopGradient", "top_gradient", -1280 * BANNER_SCALE, 0.0, 2560 * BANNER_SCALE, 182 * BANNER_SCALE),
    ("Count_BarFrame", "bar_frame", (1119 - 320 - 960) * BANNER_SCALE, 58 * BANNER_SCALE, 321 * BANNER_SCALE, 10 * BANNER_SCALE),
    ("Count_BarFill", "bar_fill", (1121 - 320 - 960) * BANNER_SCALE, 60 * BANNER_SCALE, 317 * BANNER_SCALE, 6 * BANNER_SCALE),
    ("Count_AnnounceBg", "announce_bg", 784.7 + 24.0 - 960.0, 250.0, 306.0, 94.0),
]


def slot(eid, png, x, y, w, h):
    return {
        "id": eid, "ownerClass": None, "type": 0,
        "rect": {"x": round(x * S, 3), "y": round(y * S, 3), "width": round(w * S, 3), "height": round(h * S, 3)},
        "rotation": 0, "stages": {"baseFrom": 0, "shineFrom": 1},
        "layers": [{"path": "UI/Colosseum/Countdown/%s.png" % png, "hoverPath": None,
                    "tint": [1, 1, 1, 1], "additive": False, "flipX": False}],
        "shine": {"texture": None, "additive": False},
        "animation": {"fps": 10, "scale": 1.1, "offset": {"x": 0, "y": 0}, "frames": [], "loop": True, "additive": False},
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", required=True)
    ap.add_argument("--tex", required=True)
    ap.add_argument("--tex-shared", required=True)
    args = ap.parse_args()

    res_dir = os.path.join(args.repo, "Client", "Bin", "Resources", "UI", "Colosseum", "Countdown")
    os.makedirs(res_dir, exist_ok=True)
    roots = {"tex": args.tex, "shared": args.tex_shared}
    for name, (which, tga, rect) in ART.items():
        path = None
        for base, _, files in os.walk(roots[which]):
            for f in files:
                if f.lower() == tga:
                    path = os.path.join(base, f)
        if path is None:
            raise SystemExit("missing atlas %s under %s" % (tga, roots[which]))
        im = Image.open(path).convert("RGBA").crop(rect)
        im.save(os.path.join(res_dir, name + ".png"))
        print(name, im.size)

    slots = []
    for eid, png, dx, y, w, h in SLOTS:
        slots.append(slot(eid, png, 960.0 + dx, y, w, h))
    doc = {"schema": "lostark.ui-layout", "formatVersion": 1,
           "resolution": {"width": 1280, "height": 720}, "classes": ["Default"], "slots": slots}
    out = os.path.join(args.repo, "Data", "UI", "Colosseum", "MatchCountdown_Layout.json")
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        json.dump(doc, f, indent=2)
        f.write("\n")
    print("wrote", out)


if __name__ == "__main__":
    main()
