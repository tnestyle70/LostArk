#!/usr/bin/env python3
"""Cut the HUD buff/debuff icons for the skill buffs the Server applies.

The original keeps a buff's icon in EFTable_SkillBuff (Icon = atlas name, IconIndex =
cell), and IconInfo.loa maps "<Icon>_<IconIndex>" to a page and rect.  This writes one
PNG per buff into Client/Bin/Resources/UI/HUD/Buff and records what each one came from
in Data/UI/HUD/HudBuffIcons.json, so the layout can reference a stable asset id.

Buff slot art (border, slot background) and the status icons for silence/fetter/fear
already ship from build_quickslot_hud_ui.py; this tool only adds the skill buffs and
never rewrites those.

Usage:
    python Tools/LpkPipeline/build_hud_buff_icons.py --repo .
"""

from __future__ import annotations

import argparse
import glob
import json
import sqlite3
import struct
from pathlib import Path

from PIL import Image

EXTRACTED = Path(r"D:/ClaudeWork/Extracted")
SKILLBUFF = EXTRACTED / "LpkTables/EFGame_Extra/ClientData/TableData/EFTable_SkillBuff.db"
ICONINFO = EXTRACTED / "Data3/EFGame_Extra/ClientData/XmlData/IconInfo.loa"
ICON_PAGES = EXTRACTED / "Vehicle/icons"

# (asset name, SkillBuff.PrimaryKey, the skill that applies it, buff or debuff)
BUFF_ICONS = [
    ("warlord_guardian", 171702, 17170, "buff"),
    ("warlord_oath", 172500, 17250, "buff"),
    ("artist_setting_moon", 310501, 31050, "buff"),
    ("artist_mir", 319503, 31950, "buff"),
    ("artist_shield", 319100, 31910, "buff"),
    ("artist_dream_shield", 319302, 31930, "buff"),
    ("lancemaster_short_spear", 345003, 34510, "buff"),
    ("guardianknight_dragon_mark", 490407, 49040, "debuff"),
]


def iconinfo_lookup(data: bytes, name: str):
    needle = (name + ".png").encode()
    index = data.lower().find(needle.lower())
    if index < 0:
        return None
    length = struct.unpack_from("<i", data, index - 4)[0]
    cursor = index + length
    page_length = struct.unpack_from("<i", data, cursor)[0]
    page = data[cursor + 4: cursor + 4 + page_length].split(b"\0")[0].decode()
    x, y, width, height = struct.unpack_from("<4i", data, cursor + 4 + page_length)
    return page, x, y, width, height


def open_page(stem: str) -> Image.Image:
    hits = [hit for hit in glob.glob(str(ICON_PAGES / "**" / (stem + ".*")), recursive=True)
            if hit.lower().endswith((".dds", ".tga", ".png"))]
    if not hits:
        raise SystemExit(
            f"icon page {stem} is not extracted (umodel -export EFUI_ICONATLAS_<letter>)")
    return Image.open(hits[0]).convert("RGBA")


def crop_icon(icon_data: bytes, name: str) -> Image.Image:
    found = iconinfo_lookup(icon_data, name)
    if found is None:
        raise SystemExit(f"IconInfo has no {name}")
    page, x, y, width, height = found
    return open_page(page.lower()).crop((x, y, x + width, y + height))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path("."))
    repo = parser.parse_args().repo
    icon_data = ICONINFO.read_bytes()
    buffs = sqlite3.connect(f"file:{SKILLBUFF}?mode=ro", uri=True)
    out_dir = repo / "Client/Bin/Resources/UI/HUD/Buff"
    out_dir.mkdir(parents=True, exist_ok=True)

    records = []
    for name, buff_pk, skill_id, kind in BUFF_ICONS:
        row = buffs.execute(
            "SELECT Icon, IconIndex FROM SkillBuff WHERE PrimaryKey = ?", (buff_pk,)).fetchone()
        if row is None:
            raise SystemExit(f"EFTable_SkillBuff has no row {buff_pk}")
        icon_name = f"{row[0]}_{row[1]}"
        asset = f"UI/HUD/Buff/buff_{name}.png"
        image = crop_icon(icon_data, icon_name)
        image.save(repo / "Client/Bin/Resources" / asset)
        records.append({
            "name": name, "kind": kind, "skillId": skill_id, "buffId": buff_pk,
            "iconAsset": asset, "size": list(image.size),
            "iconSource": {"table": "EFTable_SkillBuff", "primaryKey": buff_pk,
                           "icon": icon_name},
        })
        print(f"{asset}  <- {icon_name}  {image.size[0]}x{image.size[1]}")

    document = {
        "schema": "lostark.hud-buff-icons",
        "formatVersion": 1,
        "note": "Slot art and silence/fetter/fear icons come from build_quickslot_hud_ui.py.",
        "icons": records,
    }
    path = repo / "Data/UI/HUD/HudBuffIcons.json"
    path.write_text(json.dumps(document, ensure_ascii=False, indent=1) + "\n",
                    encoding="utf-8", newline="\r\n")
    print(f"{path}: {len(records)} icons")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
