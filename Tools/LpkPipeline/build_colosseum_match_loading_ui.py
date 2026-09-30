#!/usr/bin/env python3
"""Builds the Colosseum ("Annihilation - Colosseum") match-loading screen UI.

Inputs : UModel TGA exports of EFUI_COLOSSEUMLOADINGS3 / EFUI_LOCALRESOURCE (see
         gfx_scene_extract.py for how the movie coordinates were recovered).
Outputs: Client/Bin/Resources/UI/Colosseum/MatchLoading/*.png   (runtime input, Git-untracked)
         Data/UI/Colosseum/MatchLoading_Layout.json              (lostark.ui-layout v1, 1280x720)
         <work>/preview_layout.png                               (1920x1080 composite for review)

Authoring space is the movie stage 1920x1080; the JSON is written at the project's 1280x720
reference (x 2/3).  Every art element is one stable slot id.  Text is drawn by
CLevel_Loading (Draw_Text), its stage-space anchors live in Level_Loading.cpp.

usage: build_colosseum_match_loading_ui.py --repo <LostArk> --work <work dir with tex/ tex_shared/>
"""
import argparse, json, os
from PIL import Image

S = 2.0 / 3.0  # stage 1920x1080 -> reference 1280x720

# (png name, source tga relative to work dir, crop (w,h) or None)
ART = {
    "bg":            ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i6a_nopack.tga", (1920, 913)),
    "panel_border":  ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i69_nopack.tga", (775, 912)),
    "top_bg_wide":   ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i58_nopack.tga", (1920, 281)),
    "top_bg_title":  ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i57_nopack.tga", (1148, 86)),
    "footer_deco":   ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i68_nopack.tga", (1175, 152)),
    "progress_bg":   ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i5f_nopack.tga", (1798, 8)),
    "progress_fill": ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i5e_nopack.tga", (1803, 24)),
    "progress_head": ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i5d_nopack.tga", (356, 40)),
    "title_blue":    ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i46_nopack.tga", (731, 131)),
    "title_red":     ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i47_nopack.tga", (731, 131)),
    "slot_shield":   ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_ie_nopack.tga", None),
    "slot_plate":    ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i6_nopack.tga", None),
    "slot_highlight": ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_id_nopack.tga", None),
    "slot_glow":     ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_ic_nopack.tga", None),
    "gauge_gold":    ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i3_nopack.tga", None),
    "gauge_red":     ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i4_nopack.tga", None),
    "vs_glow":       ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i28_nopack.tga", (774, 711)),
    "vs_beam":       ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i23_nopack.tga", (479, 642)),
    "vs_sparks_a":   ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i26_nopack.tga", (339, 384)),
    "vs_sparks_b":   ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i27_nopack.tga", (344, 391)),
    "vs_flare":      ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i24_nopack.tga", (312, 321)),
    "wedge_left":    ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i1_nopack.tga", None),
    "wedge_right":   ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i2_nopack.tga", None),
    "join_icon":     ("tex/EFUI_COLOSSEUMLOADINGS3/Texture2D/colosseumloadings3_i59_nopack.tga", (24, 18)),
}
# V / S glyphs live in the shared LOCALRESOURCE atlas (sub-image rects from its GFX).
VS_ATLAS = "tex_shared/EFUI_LOCALRESOURCE/Texture2D/localresource_loc_int_i1.tga"
VS_GLYPHS = {"vs_v": (834, 404, 1020, 621), "vs_s": (542, 692, 705, 903)}

SLOT_PITCH = 260
LEFT_X0, RIGHT_X0, SLOT_Y = 61, 1098, 738
KEY_SLOT = {"A": 3, "B": 1}   # 1-based slot that holds the "key character" of each team

# element: id, png, x, y, w, h (stage px), flags
ELEMENTS = []


def add(eid, png, x, y, w=None, h=None, additive=False, tint=(1, 1, 1, 1), flip=False):
    ELEMENTS.append(dict(id=eid, png=png, x=x, y=y, w=w, h=h, additive=additive, tint=list(tint), flip=flip))


def build(args):
    repo, work = args.repo, args.work
    res_dir = os.path.join(repo, "Client", "Bin", "Resources", "UI", "Colosseum", "MatchLoading")
    os.makedirs(res_dir, exist_ok=True)
    sizes = {}
    for name, (src, crop) in ART.items():
        im = Image.open(os.path.join(work, src)).convert("RGBA")
        if crop:
            im = im.crop((0, 0, crop[0], crop[1]))
        im.save(os.path.join(res_dir, name + ".png"))
        sizes[name] = im.size
    atlas = Image.open(os.path.join(work, VS_ATLAS)).convert("RGBA")
    for name, rect in VS_GLYPHS.items():
        im = atlas.crop(rect)
        im.save(os.path.join(res_dir, name + ".png"))
        sizes[name] = im.size
    Image.new("RGBA", (4, 4), (0, 0, 0, 255)).save(os.path.join(res_dir, "black.png"))
    Image.new("RGBA", (4, 4), (0, 0, 0, 0)).save(os.path.join(res_dir, "blank.png"))
    sizes["black"] = sizes["blank"] = (4, 4)

    def nat(n):
        return sizes[n]

    # ---- draw order = back to front -------------------------------------------------------
    add("MatchLoading_Black", "black", 0, 0, 1920, 1080)
    add("MatchLoading_Background", "bg", 0, 101)
    add("MatchLoading_TeamA_WedgeGlow", "wedge_left", 130, 150, 560, 866, additive=True, tint=(1, 1, 1, 0.3))
    add("MatchLoading_TeamB_WedgeGlow", "wedge_right", 1110, 170, 640, 784, additive=True, tint=(1, 0.45, 0.35, 0.3))
    add("MatchLoading_TeamA_Portrait", "blank", 162, 262, 560, 486)
    add("MatchLoading_TeamB_Portrait", "blank", 1198, 262, 560, 486)
    add("MatchLoading_TeamA_PanelBorder", "panel_border", 56, 101)
    add("MatchLoading_TeamB_PanelBorder", "panel_border", 1088, 101)
    add("MatchLoading_TopBackground", "top_bg_wide", 0, 77)
    add("MatchLoading_TopTitleBackground", "top_bg_title", 353, 0)
    add("MatchLoading_TeamA_TitleBackground", "title_blue", 72, 81)
    add("MatchLoading_TeamB_TitleBackground", "title_red", 1107, 81)

    # VS emblem (group anchored at stage 951,420 in the movie)
    add("MatchLoading_VS_Glow", "vs_glow", 690, 130, 540, 496, additive=True, tint=(1, 1, 1, 0.45))
    add("MatchLoading_VS_Beam", "vs_beam", 720, 110, 479, 642, additive=True, tint=(1, 1, 1, 0.9))
    add("MatchLoading_VS_SparksA", "vs_sparks_a", 800, 200, 339, 384, additive=True)
    add("MatchLoading_VS_SparksB", "vs_sparks_b", 780, 190, 344, 391, additive=True, tint=(1, 1, 1, 0.8))
    add("MatchLoading_VS_Flare", "vs_flare", 810, 240, 312, 321, additive=True, tint=(1, 1, 1, 0.6))
    add("MatchLoading_VS_V", "vs_v", 856, 356, 112, 131)
    add("MatchLoading_VS_S", "vs_s", 940, 358, 98, 127)

    # lineup slots
    for team, x0 in (("A", LEFT_X0), ("B", RIGHT_X0)):
        for i in range(3):
            n = i + 1
            x = x0 + i * SLOT_PITCH
            key = KEY_SLOT[team] == n
            p = "MatchLoading_Team%s_Slot%d_" % (team, n)
            add(p + "Shield", "slot_shield", x, SLOT_Y)
            if key:
                add(p + "Glow", "slot_glow", x + 34, SLOT_Y + 8, 192, 120, additive=True)
            add(p + "Plate", "slot_plate", x + 16, SLOT_Y + 19)
            if key:
                add(p + "Gauge", "gauge_gold" if team == "A" else "gauge_red", x + 22, SLOT_Y + 27, 204, 40)
                add(p + "Highlight", "slot_highlight", x + 16, SLOT_Y + 17)

    # footer / progress
    add("MatchLoading_FooterDeco", "footer_deco", 353, 886)
    add("MatchLoading_ProgressBackground", "progress_bg", 54, 1034)
    add("MatchLoading_ProgressFill", "progress_fill", 54, 1026)
    add("MatchLoading_ProgressHead", "progress_head", 54 - 178, 1014, additive=True)

    # ---- JSON ------------------------------------------------------------------------------
    slots = []
    for e in ELEMENTS:
        w0, h0 = nat(e["png"])
        w = e["w"] if e["w"] is not None else w0
        h = e["h"] if e["h"] is not None else h0
        slots.append({
            "id": e["id"], "ownerClass": None, "type": 0,
            "rect": {"x": round(e["x"] * S, 3), "y": round(e["y"] * S, 3),
                     "width": round(w * S, 3), "height": round(h * S, 3)},
            "rotation": 0, "stages": {"baseFrom": 0, "shineFrom": 1},
            "layers": [{"path": "UI/Colosseum/MatchLoading/%s.png" % e["png"], "hoverPath": None,
                        "tint": e["tint"], "additive": e["additive"], "flipX": e["flip"]}],
            "shine": {"texture": None, "additive": False},
            "animation": {"fps": 10, "scale": 1.1, "offset": {"x": 0, "y": 0}, "frames": [],
                          "loop": True, "additive": False},
        })
        e["_w"], e["_h"] = w, h
    doc = {"schema": "lostark.ui-layout", "formatVersion": 1,
           "resolution": {"width": 1280, "height": 720}, "classes": ["Default"], "slots": slots}
    out = os.path.join(repo, "Data", "UI", "Colosseum", "MatchLoading_Layout.json")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        json.dump(doc, f, ensure_ascii=False, indent=2)
        f.write("\n")

    # ---- preview ---------------------------------------------------------------------------
    canvas = Image.new("RGBA", (1920, 1080), (0, 0, 0, 255))
    import numpy as np
    arr = np.asarray(canvas).astype("float32") / 255.0
    for e in ELEMENTS:
        im = Image.open(os.path.join(res_dir, e["png"] + ".png")).convert("RGBA").resize(
            (int(round(e["_w"])), int(round(e["_h"]))), Image.LANCZOS)
        px = np.asarray(im).astype("float32") / 255.0
        px = px * np.array(e["tint"], dtype="float32")
        x0, y0 = int(round(e["x"])), int(round(e["y"]))
        x1, y1 = min(1920, x0 + px.shape[1]), min(1080, y0 + px.shape[0])
        sx0, sy0 = max(0, -x0), max(0, -y0)
        dx0, dy0 = max(0, x0), max(0, y0)
        px = px[sy0:sy0 + (y1 - dy0), sx0:sx0 + (x1 - dx0)]
        dst = arr[dy0:y1, dx0:x1, :3]
        a = px[..., 3:4]
        if e["additive"]:
            dst[:] = np.clip(dst + px[..., :3] * a, 0, 1)
        else:
            dst[:] = px[..., :3] * a + dst * (1 - a)
    Image.fromarray((arr * 255).astype("uint8")).convert("RGB").save(os.path.join(work, "preview_layout.png"))
    print("slots:", len(slots), "->", out)


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", required=True)
    ap.add_argument("--work", required=True)
    build(ap.parse_args())
