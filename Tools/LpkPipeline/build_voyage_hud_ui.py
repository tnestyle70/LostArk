#!/usr/bin/env python3
"""Ship (voyage) HUD frame art and the Waterpang water gun HUD icons, from the retail
oceanhud movie and the EFUI_ICONATLAS_V icon pages.

Outputs
-------
  Client/Bin/Resources/UI/HUD/Voyage/ship_plate.png
  Client/Bin/Resources/UI/HUD/Voyage/ship_wheel.png
      OceanRudderFrame (oceanhud.gfx, sprite 1031): the hull plate (shape 1024, oceanHud_I1
      0,230-712,349, placed at stage 652,956) and the steering wheel (BoatRudderComponent ->
      BoatRudder sprite 1019 -> sprite 1018 -> shape 1017, oceanHud_I1 247,0-433,165, centred on the
      rudder origin at stage 960,975). Both crops are the exact sub-image rectangles the movie's own
      DefineSubImage tags name; they are checked against the parsed movie before anything is written.
  Client/Bin/Resources/UI/HUD/Waterpang/skill_<id>.png
      the four water gun skills of Shared/Public/Gameplay/MaharakaWaterpangContract.h
      (56900 Q / 56910 W / 56920 E / 56930 R): EFTable_Skill Icon/IconIndex -> IconInfo.loa ->
      EFUI_ICONATLAS_V page, 64x64 each.
  Data/UI/HUD/HUD_Layout.json (insert at the front, idempotent)
      Ship_Hud_Plate, Ship_Hud_Wheel: ownerClass VehicleRiding, drawn under the quick slots.
      Retail stage px -> HUD reference px uses the same mapping build_quickslot_hud_ui.py does
      (retail 960 <-> HUD 673.5 horizontally, retail 974 <-> HUD 644.702 vertically, 2/3 scale), so
      the wheel lands on the Vehicle_Hud_Emblem centre and the plate starts at the Q slot column.

Everything else the ocean HUD draws (supply dome, slots, T/Z/SPACE/C/M buttons, knots line) is
built by build_ocean_hud_ui.py. The dome is the ship *supply* gauge (OceanSupplieGauge, MaxSupply),
not durability as an earlier revision of this note said.

Inputs (produced by dump_upk_movie.py / gfx_native_parse.py / UModel, see the RESULT document):
  <extracted>/oceanhud/oceanhud_i1.dds + oceanhud.parsed.json
  <extracted>/icons_V/vehicle_0.dds, vehicle_1.png
  IconInfo.loa, EFTable_Skill.db

Usage:
  python build_voyage_hud_ui.py [--repo <LostArk root>] [--mirror <CY_Resources dir>]
"""
from __future__ import annotations

import argparse
import copy
import json
import re
import shutil
import sqlite3
import struct
import sys
from pathlib import Path

from PIL import Image

EXTRACTED = Path(r"C:/LostArkExtract/VoyageHud20260930")
ICONINFO = Path(r"C:/LostArkExtract/MinimapSymbols_20260919/IconInfo.loa")
SKILL_DB = Path(r"C:/LostArkExtract/MaharakaFunctions20260926/db/EFGame_Extra/ClientData/TableData/EFTable_Skill.db")

# retail 1920x1080 stage -> HUD reference 1280x720 (same constants as build_quickslot_hud_ui.py)
HUD_EMBLEM_CENTER_X = 673.5
HUD_Q_ROW_TOP = 644.702026
RETAIL_CENTER_X = 960.0
RETAIL_Q_ROW_TOP = 974.0
SCALE = 2.0 / 3.0


def ref_x(stage_x): return HUD_EMBLEM_CENTER_X + (stage_x - RETAIL_CENTER_X) * SCALE
def ref_y(stage_y): return HUD_Q_ROW_TOP + (stage_y - RETAIL_Q_ROW_TOP) * SCALE


# (output name, movie shape id, atlas rect it must resolve to, retail stage rect x,y,w,h)
SHIP_ART = [
    ("ship_plate.png", 1024, (0, 230, 712, 349), (652.0, 956.0, 712.0, 119.0), "Ship_Hud_Plate"),
    ("ship_wheel.png", 1017, (247, 0, 433, 165), (867.0, 892.5, 186.0, 165.0), "Ship_Hud_Wheel"),
]
OCEAN_ATLAS_FILE = "oceanHud_I1.tga"

HEADER = Path(__file__).resolve().parents[2] / "Shared/Public/Gameplay/MaharakaWaterpangContract.h"


def water_gun_skills() -> list[tuple[int, str]]:
    text = HEADER.read_text(encoding="utf-8", errors="replace")
    rows = re.findall(r"\{\s*(\d+)u,\s*'([A-Z])',\s*MAHARAKA_WATERGUN_KIND::", text)
    if len(rows) != 4:
        raise SystemExit("expected 4 water gun skills in %s, found %d" % (HEADER.name, len(rows)))
    return [(int(skill_id), slot) for skill_id, slot in rows]


def iconinfo_lookup(data: bytes, name: str):
    needle = (name + ".png").encode()
    i = data.lower().find(needle.lower())
    if i < 0:
        return None
    n = struct.unpack_from("<i", data, i - 4)[0]
    j = i + n
    plen = struct.unpack_from("<i", data, j)[0]
    page = data[j + 4:j + 4 + plen].split(b"\0")[0].decode()
    x, y, w, h = struct.unpack_from("<4i", data, j + 4 + plen)
    return page, x, y, w, h


def skill_icon_name(skill_id: int) -> str:
    con = sqlite3.connect("file:%s?mode=ro" % SKILL_DB.as_posix(), uri=True)
    con.text_factory = bytes
    table = con.execute("select name from sqlite_master where type='table'").fetchone()[0].decode()
    row = con.execute("select Icon, IconIndex from %s where PrimaryKey=?" % table, (skill_id,)).fetchone()
    if row is None:
        raise SystemExit("EFTable_Skill has no skill %d" % skill_id)
    return "%s_%d" % (row[0].decode(), int(row[1]))


def open_icon_page(stem: str) -> Image.Image:
    for ext in (".dds", ".png", ".tga"):
        path = EXTRACTED / "icons_V" / (stem.lower() + ext)
        if path.exists():
            return Image.open(path).convert("RGBA")
    raise SystemExit("icon page %s is not extracted under %s" % (stem, EXTRACTED / "icons_V"))


def layout_slot(template: dict, slot_id: str, rect: tuple[float, float, float, float], asset: str) -> dict:
    slot = copy.deepcopy(template)
    slot["id"] = slot_id
    slot["ownerClass"] = "VehicleRiding"
    slot["rect"] = {"x": round(rect[0], 6), "y": round(rect[1], 6), "width": round(rect[2], 6), "height": round(rect[3], 6)}
    slot["layers"][0]["path"] = asset
    return slot


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--mirror", type=Path, default=None, help="copy every written PNG under this root as well (CY_Resources)")
    args = parser.parse_args()
    repo = args.repo
    res = repo / "Client/Bin/Resources"
    written: list[Path] = []

    # --- ship frame art, verified against the parsed movie ---------------------------------
    movie = json.loads((EXTRACTED / "oceanhud/oceanhud.parsed.json").read_text(encoding="utf-8"))
    atlas = Image.open(EXTRACTED / "oceanhud/oceanhud_i1.dds").convert("RGBA")
    voyage_dir = res / "UI/HUD/Voyage"
    voyage_dir.mkdir(parents=True, exist_ok=True)
    for out_name, shape_id, rect, stage, _slot_id in SHIP_ART:
        # A shape lists a placeholder bitmap fill (id 65535) next to the real sub-image fill.
        fills = [f for f in movie["shapes"][str(shape_id)]["fills"]
                 if f["type"] == "bitmap" and str(f["bitmap"]) in movie["sub"]]
        if len(fills) != 1:
            raise SystemExit("shape %d has %d sub-image fills, expected 1" % (shape_id, len(fills)))
        sub = movie["sub"][str(fills[0]["bitmap"])]
        page = movie["ext"][str(sub["image"])]["file"]
        if page != OCEAN_ATLAS_FILE or tuple(sub["rect"]) != rect:
            raise SystemExit("shape %d resolves to %s %s, expected %s %s" % (shape_id, page, sub["rect"], OCEAN_ATLAS_FILE, rect))
        crop = atlas.crop(rect)
        if crop.size != (int(stage[2]), int(stage[3])) and shape_id == 1024:
            raise SystemExit("plate crop %s does not match its stage size %s" % (crop.size, stage[2:]))
        target = voyage_dir / out_name
        crop.save(target)
        written.append(target)
        print("wrote", target.relative_to(repo), crop.size)

    # --- water gun skill icons --------------------------------------------------------------
    icon_data = ICONINFO.read_bytes()
    gun_dir = res / "UI/HUD/Waterpang"
    gun_dir.mkdir(parents=True, exist_ok=True)
    for skill_id, slot in water_gun_skills():
        name = skill_icon_name(skill_id)
        hit = iconinfo_lookup(icon_data, name)
        if hit is None:
            raise SystemExit("IconInfo has no %s for skill %d" % (name, skill_id))
        page, x, y, w, h = hit
        icon = open_icon_page(page).crop((x, y, x + w, y + h))
        target = gun_dir / ("skill_%d.png" % skill_id)
        icon.save(target)
        written.append(target)
        print("wrote", target.relative_to(repo), icon.size, "slot", slot, "icon", name, "page", page)

    # --- HUD layout: insert the two ship slots at the front (idempotent) -------------------------
    layout_path = repo / "Data/UI/HUD/HUD_Layout.json"
    raw = layout_path.read_bytes()
    crlf = b"\r\n" in raw
    text = raw.decode("utf-8").replace("\r\n", "\n")
    layout = json.loads(text)
    existing = {s["id"] for s in layout["slots"]}
    template = next(s for s in layout["slots"] if s["id"] == "Vehicle_Hud_Emblem")
    new_slots = []
    for out_name, _shape_id, _rect, stage, slot_id in SHIP_ART:
        if slot_id in existing:
            continue
        rect = (ref_x(stage[0]), ref_y(stage[1]), stage[2] * SCALE, stage[3] * SCALE)
        new_slots.append(layout_slot(template, slot_id, rect, "UI/HUD/Voyage/" + out_name))
    if new_slots:
        head = '  "slots": [\n'
        at = text.index(head) + len(head)
        body = ",\n".join("    " + json.dumps(s, indent=2, ensure_ascii=False).replace("\n", "\n    ") for s in new_slots)
        text = text[:at] + body + ",\n" + text[at:]
        if crlf:
            text = text.replace("\n", "\r\n")
        layout_path.write_bytes(text.encode("utf-8"))
        json.loads(layout_path.read_text(encoding="utf-8"))
    print("layout: inserted %d slots" % len(new_slots))

    if args.mirror is not None:
        for path in written:
            dst = args.mirror / path.relative_to(res)
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(path, dst)
        print("mirrored %d files to %s" % (len(written), args.mirror))
    return 0


if __name__ == "__main__":
    sys.exit(main())
