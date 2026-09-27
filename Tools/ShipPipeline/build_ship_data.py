#!/usr/bin/env python3
"""Author the retail Voyage ships as vehicle data (catalog, Server profile, UI rows, icons).

Inputs (all read-only):
  <tables>/EFTable_VoyageShip.db, EFTable_GameMsg.db   unpacked with Tools/LpkPipeline/unpack_lpk.py
  <work>/cook/<Asset>/<Asset>.cook.json                written by Tools/ShipPipeline/cook_ships.py
  <work>/icons/.../voyage_ship_0.tga                   UModel export of EFUI_ICONATLAS_V
  <iconinfo>                                           IconInfo.loa from data3.lpk

Outputs (merged, never regenerated wholesale):
  Data/Actors/VehicleCatalog.json        9 vehicle rows   (text insertion, other bytes untouched)
  Data/Vehicles/VehicleProfiles.json     9 Server rows    (text insertion, other bytes untouched)
  Data/UI/Vehicle/VehicleUiCatalog.json  9 UI rows + shipTitle string
  Client/Bin/Resources/UI/Vehicle/Icons/ship_<id>.png     new files only

Each destination is skipped for ids it already contains, so the tool can be re-run.
"""

from __future__ import annotations

import argparse
import json
import math
import os
import re
import shutil
import sqlite3
import struct
import tempfile
from pathlib import Path

from PIL import Image

REPO = Path(__file__).resolve().parents[2]

# vehicleId -> (archetype suffix, cook asset). Order = EFTable_VoyageShip.SortOrder.
SHIPS = [
    (8200, "ESTOC", "Ship_Estoc"), (8201, "WHITEWIND", "Ship_Whitewind"), (8202, "ASTRAY", "Ship_Astray"),
    (8203, "BARKSTORM", "Ship_Barkstorm"), (8204, "GHOST", "Ship_Ghost"), (8205, "BRAHMS", "Ship_Brahms"),
    (8206, "TRAGON", "Ship_Tragon"), (8207, "PNEUMA", "Ship_Pneuma"), (8208, "LUMINOUS", "Ship_Luminous"),
]

# characterClass -> clip prefix; the rider stands on the deck with the class's own battle idle.
RIDER_CLASSES = [
    ("LANCE_MASTER", "flm"), ("WARLORD", "wgl"), ("ARTIST", "sdm"), ("DIMENSIONMASTER", "pc_sp_m_00_sk"),
    ("GUARDIANKNIGHT", "ddk"), ("GUNSLINGER", "gdh"), ("SLAYER", "wbk"),
]


def fix(value):
    if isinstance(value, bytes):
        for encoding in ("utf-8", "cp949"):
            try:
                return value.decode(encoding)
            except UnicodeDecodeError:
                pass
        return value.decode("latin1")
    return value


def read_table(tables: Path, name: str) -> list[dict]:
    con = sqlite3.connect(str(tables / f"EFTable_{name}.db"))
    con.text_factory = bytes
    columns = [fix(r[1]) for r in con.execute(f'pragma table_info("{name}")')]
    return [dict(zip(columns, [fix(v) for v in row])) for row in con.execute(f'select * from "{name}"')]


def game_msg(tables: Path, keys: set[str]) -> dict[str, str]:
    con = sqlite3.connect(str(tables / "EFTable_GameMsg.db"))
    con.text_factory = bytes
    found = {}
    for key, message in con.execute("select KEY, MSG from GameMsg"):
        lowered = fix(key).lower()
        if lowered in keys:
            found[lowered] = re.sub(r"<[^>]+>", "", fix(message))
    return found


def iconinfo_lookup(data: bytes, name: str):
    needle = (name + ".png").encode()
    i = data.lower().find(needle.lower())
    if i < 0:
        raise SystemExit(f"IconInfo has no {name}")
    n = struct.unpack_from("<i", data, i - 4)[0]
    j = i + n
    plen = struct.unpack_from("<i", data, j)[0]
    page = data[j + 4:j + 4 + plen].split(b"\0")[0].decode()
    x, y, w, h = struct.unpack_from("<4i", data, j + 4 + plen)
    return page, x, y, w, h


def format_speed(value: int) -> str:
    """value/100 as the shortest exact decimal, always with a fraction (5 -> 5.0, 183 -> 1.83)."""
    text = f"{value / 100.0:.2f}".rstrip("0")
    return text + "0" if text.endswith(".") else text


def crlf(text: str) -> str:
    return text.replace("\r\n", "\n").replace("\n", "\r\n")


def atomic_write(path: Path, data: bytes) -> None:
    handle, temp = tempfile.mkstemp(dir=str(path.parent), prefix=path.name + ".", suffix=".tmp")
    with os.fdopen(handle, "wb") as stream:
        stream.write(data)
    os.replace(temp, path)


def backup(path: Path, root: Path) -> None:
    destination = root / "backup" / path.relative_to(REPO)
    if not destination.exists():
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(path, destination)


def insert_before_array_end(path: Path, existing_ids: set[int], entries: list[tuple[int, str]], backup_root: Path) -> int:
    """Append entry text blocks before the closing of the top-level vehicles array."""
    raw = path.read_bytes()
    text = raw.decode("utf-8")
    fresh = [(i, t) for i, t in entries if i not in existing_ids]
    if not fresh:
        return 0
    tail = "\r\n  ]\r\n}\r\n"
    if not text.endswith(tail):
        raise SystemExit(f"{path.name}: unexpected file ending, refusing to edit")
    backup(path, backup_root)
    body = ",\r\n".join(crlf(t) for _, t in fresh)
    atomic_write(path, (text[:-len(tail)] + ",\r\n" + body + tail).encode("utf-8"))
    return len(fresh)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--tables", type=Path, default=REPO / "out/Bern3Ship20260925/lpk/EFGame_Extra/ClientData/TableData")
    parser.add_argument("--iconinfo", type=Path, default=REPO / "out/Bern3Ship20260925/lpk3/EFGame_Extra/ClientData/XmlData/IconInfo.loa")
    parser.add_argument("--work", type=Path, default=Path("C:/LostArkExtract/Ship20260925"))
    parser.add_argument("--backup", type=Path, default=REPO / "out/Bern3Ship20260925")
    args = parser.parse_args()

    ship_rows = read_table(args.tables, "VoyageShip")
    by_id = {}
    for row in ship_rows:
        by_id.setdefault(int(row["PrimaryKey"]), []).append(row)
    keys = set()
    for ship_id, _, _ in SHIPS:
        keys.add(str(by_id[ship_id][0]["Name"]).lower())
        keys.add(str(by_id[ship_id][0]["GetHintComment"]).lower())
    messages = game_msg(args.tables, keys)

    atlas = Image.open(args.work / "icons/EFUI_ICONATLAS_V/Texture2D/voyage_ship_0.tga").convert("RGBA")
    icon_data = args.iconinfo.read_bytes()
    icon_dir = REPO / "Client/Bin/Resources/UI/Vehicle/Icons"

    catalog_entries, profile_entries, ui_rows = [], [], []
    for ship_id, suffix, asset in SHIPS:
        first = sorted(by_id[ship_id], key=lambda r: int(r["SecondaryKey"]))[0]
        receipt = json.loads((args.work / "cook" / asset / f"{asset}.cook.json").read_text(encoding="utf-8"))
        bones = receipt["analysis"]["boneNames"]
        clips = receipt["analysis"]["clips"]
        seat_bone = next((b for b in ("b_root", "b_body", "b_cameratarget") if b in bones), None)
        if seat_bone is None or "idle_normal_1" not in clips or "run_battle_1" not in clips:
            raise SystemExit(f"{asset}: needs a seat bone plus idle_normal_1 and run_battle_1")
        deck_cm = receipt["analysis"]["deckHeightCmEstimate"] or 60
        speed_value = int(round(float(first["MoveSpeed"])))
        speed = speed_value / 100.0

        riders = ",\r\n".join(
            '        { "characterClass": "%s", "idleClip": "%s_idle_battle_1", "runClip": "%s_idle_battle_1" }' % (c, p, p)
            for c, p in RIDER_CLASSES)
        catalog_entries.append((ship_id, (
            '    {\n'
            f'      "vehicleId": {ship_id},\n'
            f'      "archetypeId": "VEHICLE_SHIP_{suffix}",\n'
            '      "ship": true,\n'
            f'      "seatOffset": [0.0, {deck_cm / 100.0:.2f}, 0.0],\n'
            f'      "modelLiftMeters": {math.ceil(max(0.0, -receipt["analysis"]["bboxCm"]["y"][0])) / 100.0:.2f},\n'
            f'      "modelAssetId": "Character/Vehicle/{asset}/{asset}.wmodel",\n'
            '      "modelPreScale": 0.01,\n'
            f'      "seatBone": "{seat_bone}",\n'
            '      "vehicleIdleClip": "idle_normal_1",\n'
            '      "vehicleRunClip": "run_battle_1",\n'
            '      "riders": [\n' + riders.replace("\r\n", "\n") + '\n      ],\n'
            '      "skills": [],\n'
            '      "runtimeStatus": "supported",\n'
            '      "modelMaterialOverrides": []\n'
            '    }')))
        profile_entries.append((ship_id, (
            '    {\n'
            f'      "vehicleId": {ship_id},\n'
            f'      "moveSpeed": {format_speed(speed_value)},\n'
            '      "source": {\n'
            '        "table": "EFTable_VoyageShip",\n'
            f'        "primaryKey": {ship_id},\n'
            '        "column": "MoveSpeed",\n'
            f'        "value": {speed_value},\n'
            '        "divisor": 100\n'
            '      },\n'
            '      "skills": []\n'
            '    }')))

        sort_order = int(first["SortOrder"])
        icon_name = f"Voyage_Ship_1_{sort_order - 1}"
        page, x, y, w, h = iconinfo_lookup(icon_data, icon_name)
        icon_asset = f"UI/Vehicle/Icons/ship_{ship_id}.png"
        icon_path = icon_dir / f"ship_{ship_id}.png"
        if not icon_path.exists():
            icon_dir.mkdir(parents=True, exist_ok=True)
            atlas.crop((x, y, x + w, y + h)).save(icon_path)
        ui_rows.append({
            "vehicleId": ship_id, "archetypeId": f"VEHICLE_SHIP_{suffix}",
            "name": messages.get(str(first["Name"]).lower(), ""),
            "description": messages.get(str(first["GetHintComment"]).lower(), ""),
            "moveSpeed": speed, "iconAsset": icon_asset,
            "iconSource": {"page": page, "x": x, "y": y, "width": w, "height": h, "icon": icon_name},
            "skills": [],
        })
        print(f"  {ship_id} {suffix:<10} {ui_rows[-1]['name']:<8} seat={seat_bone} deck={deck_cm}cm speed={speed} icon={icon_name}")

    catalog_path = REPO / "Data/Actors/VehicleCatalog.json"
    existing = {int(v["vehicleId"]) for v in json.loads(catalog_path.read_text(encoding="utf-8"))["vehicles"]}
    print("VehicleCatalog.json: added", insert_before_array_end(catalog_path, existing, catalog_entries, args.backup))
    profile_path = REPO / "Data/Vehicles/VehicleProfiles.json"
    existing = {int(v["vehicleId"]) for v in json.loads(profile_path.read_text(encoding="utf-8"))["vehicles"]}
    print("VehicleProfiles.json: added", insert_before_array_end(profile_path, existing, profile_entries, args.backup))

    ui_path = REPO / "Data/UI/Vehicle/VehicleUiCatalog.json"
    document = json.loads(ui_path.read_text(encoding="utf-8"))
    known = {int(v["vehicleId"]) for v in document["vehicles"]}
    added = [row for row in ui_rows if row["vehicleId"] not in known]
    changed = bool(added) or "shipTitle" not in document["strings"]
    if changed:
        backup(ui_path, args.backup)
        document["vehicles"].extend(added)
        document["strings"].setdefault("shipTitle", "선박")
        text = json.dumps(document, ensure_ascii=False, indent=1)
        atomic_write(ui_path, (crlf(text) + "\r\n").encode("utf-8"))
    print("VehicleUiCatalog.json: added", len(added))


if __name__ == "__main__":
    main()
