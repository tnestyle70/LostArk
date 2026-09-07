#!/usr/bin/env python3
"""Cut the character-creation icons out of the retail icon atlases.

Three pieces have to meet:

* ``EFTable_CharacterCustomizing.db`` (data2.lpk) says which icon each list cell
  shows, as an icon package name plus an index -- ``CharacterPreset_Fighter`` and
  ``310``, for instance.
* ``IconInfo.loa`` (data3.lpk) maps ``<package>_<index>.png`` to an atlas page
  and a pixel rect.  The index is not a grid position: the same package spills
  across pages and skips numbers, which is why a fixed 64-pixel grid never lined
  up with it.
* The atlas pages themselves are ``texture2d`` exports in the ``EFUI_ICONATLAS_*``
  packages, extracted with UModel.

This walks the table for the four playable classes, resolves every cell through
IconInfo, cuts it from the page and writes one PNG per icon plus a JSON index the
runtime can read.

Usage:
  python build_customizing_icons.py --table <EFTable_CharacterCustomizing.db>
                                    --icon-info <IconInfo.loa>
                                    --atlas-dir <dir with the exported .dds>
                                    --out-images <dir> --out-index <file.json>
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import sqlite3
import struct
import sys

from PIL import Image

ICON_INFO_HEADER = 16
ICON_INFO_TRAILER = 28

# EFTable_CharacterCustomizing PrimaryKey -> the class this project actually builds.
CLASSES = {
    301: "LanceMaster",
    601: "Artist",
    611: "DimensionMaster",
    101: "Warlord",
}

# The icon packages each left-column list draws from.
# Customizing_lv is the background picker (sys.pccrate.customizing_lv_select_title reads
# "background selection"), not the recommended styles. Character creation hides that list, so
# the icons are still cut for the salon screen but nothing on this screen uses them. The
# recommended-style row is account content the client fetches, with no table behind it.
LISTS = {
    "preset": lambda icon: icon.lower().startswith("characterpreset_"),
    "action": lambda icon: icon.lower() == "saction_01",
    "background": lambda icon: icon.lower() == "customizing_lv",
}


def read_icon_info(path: Path) -> dict[str, tuple[str, int, int, int, int]]:
    """name -> (atlas page, x, y, width, height)."""
    blob = path.read_bytes()
    out: dict[str, tuple[str, int, int, int, int]] = {}
    offset = ICON_INFO_HEADER
    while offset + 8 < len(blob):
        length = struct.unpack_from("<i", blob, offset)[0]
        if not 0 < length < 300:
            break
        name = blob[offset + 4 : offset + 4 + length].split(b"\0")[0]
        cursor = offset + 4 + length
        length = struct.unpack_from("<i", blob, cursor)[0]
        if not 0 < length < 300:
            break
        page = blob[cursor + 4 : cursor + 4 + length].split(b"\0")[0]
        cursor += 4 + length
        x, y, width, height = struct.unpack_from("<4i", blob, cursor)
        out[name.decode("ascii", "replace").lower()] = (
            page.decode("ascii", "replace"),
            x,
            y,
            width,
            height,
        )
        offset = cursor + ICON_INFO_TRAILER
    if not out:
        raise SystemExit(f"{path}: no icon records parsed")
    return out


def read_table(path: Path) -> list[dict]:
    connection = sqlite3.connect(path)
    connection.text_factory = bytes
    cursor = connection.cursor()
    cursor.execute(
        "select PrimaryKey, Icon, IconIndex, SortOrder from CharacterCustomizing"
    )
    rows = []
    for primary, icon, index, order in cursor.fetchall():
        rows.append(
            {
                "class": CLASSES.get(primary),
                "icon": icon.decode("cp949", "replace"),
                "index": index,
                "order": order,
            }
        )
    connection.close()
    return [r for r in rows if r["class"] and r["icon"]]


def load_atlases(directory: Path) -> dict[str, Image.Image]:
    pages: dict[str, Image.Image] = {}
    for path in sorted(directory.rglob("*.dds")):
        pages[path.stem.lower()] = Image.open(path).convert("RGBA")
    if not pages:
        raise SystemExit(f"{directory}: no atlas pages found")
    return pages


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--table", required=True, type=Path)
    parser.add_argument("--icon-info", required=True, type=Path)
    parser.add_argument("--atlas-dir", required=True, type=Path)
    parser.add_argument("--out-images", required=True, type=Path)
    parser.add_argument("--out-index", required=True, type=Path)
    parser.add_argument("--asset-prefix", default="UI/Customizing/Icons")
    args = parser.parse_args()

    icons = read_icon_info(args.icon_info)
    atlases = load_atlases(args.atlas_dir)
    rows = read_table(args.table)

    args.out_images.mkdir(parents=True, exist_ok=True)
    index: dict[str, dict[str, list[str]]] = {}
    written: set[str] = set()
    missing_icon: list[str] = []
    missing_page: set[str] = set()

    for row in sorted(rows, key=lambda r: (r["class"], r["icon"], r["order"])):
        kind = next((k for k, test in LISTS.items() if test(row["icon"])), None)
        if kind is None:
            continue
        name = "%s_%d.png" % (row["icon"].lower(), row["index"])
        placement = icons.get(name)
        if placement is None:
            missing_icon.append(name)
            continue
        page, x, y, width, height = placement
        atlas = atlases.get(page.lower())
        if atlas is None:
            missing_page.add(page)
            continue
        if x < 0 or y < 0 or x + width > atlas.width or y + height > atlas.height:
            missing_icon.append(name + " (rect outside page)")
            continue

        stem = name[: -len(".png")]
        if stem not in written:
            atlas.crop((x, y, x + width, y + height)).save(args.out_images / (stem + ".png"))
            written.add(stem)
        index.setdefault(row["class"], {}).setdefault(kind, []).append(
            "%s/%s.png" % (args.asset_prefix, stem)
        )

    document = {
        "schema": "lostark.customizing-icons",
        "formatVersion": 1,
        "classes": index,
    }
    args.out_index.parent.mkdir(parents=True, exist_ok=True)
    args.out_index.write_text(
        json.dumps(document, ensure_ascii=False, indent=1) + "\n", encoding="utf-8"
    )

    print(f"{len(written)} icons -> {args.out_images}")
    for name, lists in index.items():
        print("   %-16s %s" % (name, {k: len(v) for k, v in lists.items()}))
    if missing_page:
        print("  atlas page missing: " + ", ".join(sorted(missing_page)), file=sys.stderr)
    if missing_icon:
        print(f"  {len(missing_icon)} icons unresolved, first: {missing_icon[:4]}",
              file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
