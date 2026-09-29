#!/usr/bin/env python3
"""Retail ocean (voyage) HUD art and layout: everything oceanhud.gfx draws around the steering
wheel, so a mounted ship shows the same HUD as the retail client.

build_voyage_hud_ui.py already lifts the hull plate (shape 1024) and the wheel (shape 1017).
This script adds the rest of what the movie draws at frame 1 ("up"/"enabled" labels), each piece
resolved through the parsed movie before anything is written:

  Ship_Hud_Bezel     OceanRudderFrame shape 1026   (oceanHud_I1 435,0-549,126)
  Ship_Hud_Dome      OceanSupplieGauge dome: dark sphere shape 980 + blue sphere shape 984, composed
                     into 41 flipbook frames (0..100 % in 2.5 % steps), stage 910.1,924.6 100x100
  Ship_Hud_Icon      suppliesIcon_mc shape 1028    (901,920-940,959)
  Ship_Hud_Bottle    eventProgress shape 363       (936,575-996,667)
  Ship_Slot_<K>_Frame/_Icon/_Cooldown for the eight skillSlotList slots (Q W E R / A S D F): frame
                     shape 386 (210,866-260,916) + key tab shape 860 (942,920-969,934)
  Ship_Btn_Cruise    cruise_btn: shapes 64 + 92     (T)
  Ship_Btn_Anchor    boatParking_btn: shapes 49 + 318 (Z)
  Ship_Btn_Boost     boatBooster: gear 294, dark ring 225 halves, yellow ring 227 halves, arrow 134 (SPACE)
  Ship_Btn_Horn      boatHorn_btn shape 369         (C)
  Ship_Btn_AutoCruise autoCruiseBtn shape 67        (M)

Stage px -> HUD reference px uses the mapping build_voyage_hud_ui.py established (retail 960 <->
HUD 673.5, retail 974 <-> HUD 644.702, 2/3 scale) so the wheel keeps sitting on the saddle emblem
centre.

The numbers on the dome ("2275/3500") and the knots line are drawn by CMainApp from replicated
values; the movie's own text fields are placeholders. The dome is the ship *supply* gauge
(OceanSupplieGauge, EFTable_VoyageShip.MaxSupply): 8200 level 1 = 3500 and 8203 = 3900, which is why
the retail screenshots read 2275/3500 and 3850/3900.

Usage:
  python build_ocean_hud_ui.py [--repo <LostArk root>] [--mirror <CY_Resources dir>]
"""
from __future__ import annotations

import argparse
import copy
import json
import shutil
import sys
from pathlib import Path

from PIL import Image, ImageDraw

EXTRACTED = Path(r"C:/LostArkExtract/VoyageHud20260930")
OCEAN_ATLAS_FILE = "oceanHud_I1.tga"

HUD_EMBLEM_CENTER_X = 673.5
HUD_Q_ROW_TOP = 644.702026
RETAIL_CENTER_X = 960.0
RETAIL_Q_ROW_TOP = 974.0
SCALE = 2.0 / 3.0


def ref_x(stage_x): return HUD_EMBLEM_CENTER_X + (stage_x - RETAIL_CENTER_X) * SCALE
def ref_y(stage_y): return HUD_Q_ROW_TOP + (stage_y - RETAIL_Q_ROW_TOP) * SCALE


DOME_FRAMES = 41

# key -> (stage x, stage y) of the skillSlotList slot; all eight are 42.1 x 41.1 stage px.
# The movie's frame-1 list is spaced 47 px, but the retail client re-spaces the list at run time: the
# retail screenshot (Screenshots/스크린샷 2026-09-30 030649.png, HUD crop origin stage 568,852) measures
# a 44.3 px pitch, Q at stage x 699.0 and A at 721.3, rows at y 972.8 / 1018.8. Those measured values win.
SLOT_W, SLOT_H = 42.1, 41.1
SLOT_PITCH = 44.3
SLOT_POS = {
    "Q": (699.0, 972.8), "W": (699.0 + SLOT_PITCH, 972.8), "E": (699.0 + 2 * SLOT_PITCH, 972.8), "R": (699.0 + 3 * SLOT_PITCH, 972.8),
    "A": (721.3, 1018.8), "S": (721.3 + SLOT_PITCH, 1018.8), "D": (721.3 + 2 * SLOT_PITCH, 1018.8), "F": (721.3 + 3 * SLOT_PITCH, 1018.8),
}
SLOT_KEYS = "QWERASDF"

# (png name, movie shape id, atlas rect it must resolve to)
SHAPES = {
    "ship_bezel.png": (1026, (435, 0, 549, 126)),
    "ship_icon.png": (1028, (901, 920, 940, 959)),
    "ship_bottle.png": (363, (936, 575, 996, 667)),
    "btn_cruise_icon.png": (64, (49, 920, 96, 967)),
    "btn_cruise_ring.png": (92, (537, 789, 600, 856)),
    "btn_anchor_bg.png": (49, (545, 687, 622, 764)),
    "btn_anchor_icon.png": (318, (910, 789, 972, 852)),
    "btn_boost_gear.png": (294, (551, 0, 674, 123)),
    "btn_boost_arrow.png": (134, (1007, 208, 1019, 222)),
    "btn_horn.png": (369, (676, 0, 849, 121)),
    "btn_autocruise.png": (67, (821, 167, 881, 222)),
}
RING_HALVES = {"dark": (225, (939, 230, 981, 327)), "yellow": (227, (881, 230, 937, 342))}
DOME_SHAPES = {"base": (980, (112, 575, 216, 679)), "water": (984, (218, 575, 322, 679)),
               "glow": (986, (602, 789, 696, 855))}
SLOT_SHAPES = {"frame": (386, (210, 866, 260, 916)), "tab": (860, (942, 920, 969, 934))}

# slot id -> (png, stage x, y, w, h); document order = draw order (the movie's depth order)
BOOST_CENTER = (1153.9, 1028.5)
RING_SCALE = 0.6


def verify(movie: dict, shape_id: int, rect: tuple[int, int, int, int]) -> None:
    fills = [f for f in movie["shapes"][str(shape_id)]["fills"]
             if f["type"] == "bitmap" and str(f["bitmap"]) in movie["sub"]]
    if len(fills) != 1:
        raise SystemExit("shape %d has %d sub-image fills, expected 1" % (shape_id, len(fills)))
    sub = movie["sub"][str(fills[0]["bitmap"])]
    page = movie["ext"][str(sub["image"])]["file"]
    if page != OCEAN_ATLAS_FILE or tuple(sub["rect"]) != rect:
        raise SystemExit("shape %d resolves to %s %s, expected %s %s" % (shape_id, page, sub["rect"], OCEAN_ATLAS_FILE, rect))


def build_ring(atlas: Image.Image, rect, scale: float) -> Image.Image:
    """Two mirrored half crescents make the closed ring; the movie scales them 0.6."""
    half = atlas.crop(rect)
    ring = Image.new("RGBA", (half.size[0] * 2, half.size[1]), (0, 0, 0, 0))
    ring.paste(half.transpose(Image.FLIP_LEFT_RIGHT), (0, 0))
    ring.paste(half, (half.size[0], 0))
    return ring.resize((round(ring.size[0] * scale), round(ring.size[1] * scale)), Image.LANCZOS)


def build_dome_frames(atlas: Image.Image) -> list[Image.Image]:
    base = atlas.crop(DOME_SHAPES["base"][1])
    water = atlas.crop(DOME_SHAPES["water"][1])
    glow = atlas.crop(DOME_SHAPES["glow"][1])
    w, h = base.size
    frames = []
    for i in range(DOME_FRAMES):
        ratio = i / (DOME_FRAMES - 1)
        frame = base.copy()
        if ratio > 0.0:
            line = round(h * (1.0 - ratio))
            mask = Image.new("L", (w, h), 0)
            ImageDraw.Draw(mask).rectangle((0, line, w, h), fill=255)
            layer = Image.new("RGBA", (w, h), (0, 0, 0, 0))
            layer.paste(water, (0, 0), mask)
            layer.putalpha(Image.composite(water.split()[3], Image.new("L", (w, h), 0), mask))
            frame.alpha_composite(layer)
            if ratio < 1.0:
                # the surface glow sits on the water line, kept inside the sphere by the water's own alpha
                glow_layer = Image.new("RGBA", (w, h), (0, 0, 0, 0))
                glow_layer.alpha_composite(glow, ((w - glow.size[0]) // 2, line - glow.size[1] // 2))
                inside = Image.composite(water.split()[3], Image.new("L", (w, h), 0), mask)
                glow_alpha = glow_layer.split()[3].point(lambda v: int(v * 0.55))
                glow_layer.putalpha(Image.composite(glow_alpha, Image.new("L", (w, h), 0), inside))
                frame.alpha_composite(glow_layer)
        frames.append(frame)
    return frames


def slot_json(template: dict, slot_id: str, stage_rect, asset: str | None) -> dict:
    slot = copy.deepcopy(template)
    slot["id"] = slot_id
    slot["ownerClass"] = "VehicleRiding"
    x, y, w, h = stage_rect
    slot["rect"] = {"x": round(ref_x(x), 6), "y": round(ref_y(y), 6), "width": round(w * SCALE, 6), "height": round(h * SCALE, 6)}
    if asset is not None and slot["layers"]:
        slot["layers"][0]["path"] = asset
    return slot


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--mirror", type=Path, default=None, help="copy every written PNG under this root as well (CY_Resources)")
    args = parser.parse_args()
    repo = args.repo
    res = repo / "Client/Bin/Resources"
    movie = json.loads((EXTRACTED / "oceanhud/oceanhud.parsed.json").read_text(encoding="utf-8"))
    atlas = Image.open(EXTRACTED / "oceanhud/oceanhud_i1.dds").convert("RGBA")
    out_dir = res / "UI/HUD/Voyage"
    dome_dir = out_dir / "Dome"
    dome_dir.mkdir(parents=True, exist_ok=True)
    written: list[Path] = []

    def save(image: Image.Image, target: Path) -> None:
        image.save(target)
        written.append(target)
        print("wrote", target.relative_to(repo), image.size)

    for shape_id, rect in [*SHAPES.values(), *RING_HALVES.values(), *DOME_SHAPES.values(), *SLOT_SHAPES.values()]:
        verify(movie, shape_id, rect)

    for name, (_shape, rect) in SHAPES.items():
        save(atlas.crop(rect), out_dir / name)
    save(build_ring(atlas, RING_HALVES["dark"][1], RING_SCALE), out_dir / "btn_boost_dark.png")
    save(build_ring(atlas, RING_HALVES["yellow"][1], RING_SCALE), out_dir / "btn_boost_fill.png")

    frame = atlas.crop(SLOT_SHAPES["frame"][1]).copy()
    # the retail screenshot shows an empty slot as a dark square with a thin lighter border
    ImageDraw.Draw(frame).rectangle((1, 1, frame.size[0] - 2, frame.size[1] - 2), outline=(66, 66, 68, 255))
    frame.alpha_composite(atlas.crop(SLOT_SHAPES["tab"][1]), (12, 36))
    save(frame, out_dir / "ship_slot_frame.png")
    save(atlas.crop((435, 128, 471, 164)), out_dir / "ship_event_slot.png")

    dome_frames = build_dome_frames(atlas)
    dome_paths = []
    for i, image in enumerate(dome_frames):
        target = dome_dir / ("ship_dome_%02d.png" % i)
        save(image, target)
        dome_paths.append("UI/HUD/Voyage/Dome/" + target.name)

    # --- layout ---------------------------------------------------------------------------------
    layout_path = repo / "Data/UI/HUD/HUD_Layout.json"
    raw = layout_path.read_bytes()
    crlf = b"\r\n" in raw
    text = raw.decode("utf-8").replace("\r\n", "\n")
    layout = json.loads(text)
    existing = {s["id"] for s in layout["slots"]}
    by_id = {s["id"]: s for s in layout["slots"]}
    emblem = by_id["Vehicle_Hud_Emblem"]
    icon_tpl = by_id["Skill_Q_Icon"]
    cool_tpl = by_id["Skill_Q_Cooldown"]

    V = "UI/HUD/Voyage/"
    ring_w = 84 * RING_SCALE
    plan: list[dict] = []
    plan.append(slot_json(emblem, "Ship_Hud_Bezel", (902.0, 918.2, 101.5, 112.1), V + "ship_bezel.png"))
    dome = slot_json(emblem, "Ship_Hud_Dome", (910.1, 924.6, 99.8, 99.9), None)
    dome["layers"] = []
    dome["animation"] = {"fps": 1, "scale": 1, "offset": {"x": 0, "y": 0}, "frames": dome_paths, "loop": True, "additive": False}
    plan.append(dome)
    plan.append(slot_json(emblem, "Ship_Hud_Icon", (941.0, 1012.0, 39.0, 39.0), V + "ship_icon.png"))
    for key in SLOT_KEYS:
        x, y = SLOT_POS[key]
        plan.append(slot_json(emblem, "Ship_Slot_%s_Frame" % key, (x, y, SLOT_W, SLOT_H), V + "ship_slot_frame.png"))
    for key in SLOT_KEYS:
        x, y = SLOT_POS[key]
        plan.append(slot_json(icon_tpl, "Ship_Slot_%s_Icon" % key, (x, y, SLOT_W, SLOT_H), None))
    for key in SLOT_KEYS:
        x, y = SLOT_POS[key]
        plan.append(slot_json(cool_tpl, "Ship_Slot_%s_Cooldown" % key, (x, y, SLOT_W, SLOT_H), None))
    plan.append(slot_json(emblem, "Ship_Btn_Horn", (1122.8, 955.9, 173.0, 121.0), V + "btn_horn.png"))
    plan.append(slot_json(emblem, "Ship_Hud_Bottle", (1182.0, 948.0, 60.0, 92.0), V + "ship_bottle.png"))
    plan.append(slot_json(emblem, "Ship_Hud_EventSlot", (1195.0, 1031.0, 36.0, 36.0), V + "ship_event_slot.png"))
    plan.append(slot_json(emblem, "Ship_Btn_Anchor_Bg", (1033.0, 991.0, 77.0, 77.0), V + "btn_anchor_bg.png"))
    plan.append(slot_json(emblem, "Ship_Btn_Anchor_Icon", (1040.0, 998.0, 62.0, 63.0), V + "btn_anchor_icon.png"))
    plan.append(slot_json(emblem, "Ship_Btn_Boost_Dark", (BOOST_CENTER[0] - 84 * RING_SCALE / 2, BOOST_CENTER[1] - 97 * RING_SCALE / 2, 84 * RING_SCALE, 97 * RING_SCALE), V + "btn_boost_dark.png"))
    plan.append(slot_json(emblem, "Ship_Btn_Boost_Fill", (BOOST_CENTER[0] - 112 * RING_SCALE / 2, BOOST_CENTER[1] - 112 * RING_SCALE / 2, 112 * RING_SCALE, 112 * RING_SCALE), V + "btn_boost_fill.png"))
    plan.append(slot_json(emblem, "Ship_Btn_Boost_Gear", (BOOST_CENTER[0] - 40.0, BOOST_CENTER[1] - 39.85, 80.0, 79.7), V + "btn_boost_gear.png"))
    plan.append(slot_json(emblem, "Ship_Btn_Boost_Arrow", (1148.0, 1022.0, 12.0, 14.0), V + "btn_boost_arrow.png"))
    plan.append(slot_json(emblem, "Ship_Btn_Cruise_Icon", (1088.0, 954.0, 47.0, 47.0), V + "btn_cruise_icon.png"))
    plan.append(slot_json(emblem, "Ship_Btn_Cruise_Ring", (1083.0, 945.0, 63.0, 67.0), V + "btn_cruise_ring.png"))
    plan.append(slot_json(emblem, "Ship_Btn_AutoCruise", (1286.0, 996.0, 60.0, 55.0), V + "btn_autocruise.png"))

    # The document round-trips through json.dumps(indent=2) byte for byte (checked when this was written), so the
    # ocean slots are replaced in place: drop every earlier copy of a planned slot, then insert the plan right
    # after Ship_Hud_Wheel (document order = draw order).
    if text != json.dumps(layout, indent=2, ensure_ascii=False):
        raise SystemExit("HUD_Layout.json does not round-trip through json.dumps; refusing to rewrite it")
    planned = {s["id"] for s in plan}
    before = len(layout["slots"])
    layout["slots"] = [s for s in layout["slots"] if s["id"] not in planned]
    at = next(i for i, s in enumerate(layout["slots"]) if s["id"] == "Ship_Hud_Wheel") + 1
    layout["slots"][at:at] = plan
    out = json.dumps(layout, indent=2, ensure_ascii=False)
    if crlf:
        out = out.replace("\n", "\r\n")
    layout_path.write_bytes(out.encode("utf-8"))
    json.loads(layout_path.read_text(encoding="utf-8"))
    print("layout: %d ocean slots written (%d replaced, slots %d -> %d)" % (len(plan), before - (len(layout["slots"]) - len(plan)), before, len(layout["slots"])))

    if args.mirror is not None:
        for path in written:
            dst = args.mirror / path.relative_to(res)
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(path, dst)
        print("mirrored %d files to %s" % (len(written), args.mirror))
    return 0


if __name__ == "__main__":
    sys.exit(main())
