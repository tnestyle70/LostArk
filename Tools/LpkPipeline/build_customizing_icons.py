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

# SecondaryKey is the customizing category, and it is what splits one class's
# CharacterPreset_* rows into the separate lists each tab shows. Lumping them together mixes
# whole-face thumbnails with lips and eyebrows.
#
# Key 0 is the base tab's preset grid: its IconIndex runs 1, 2, 3, ... and those icons match
# the retail screenshot's grid cell for cell, in order. Key 2 is the action row and key 3 the
# background picker (sys.pccrate.customizing_lv_select_title reads "background selection"),
# which character creation hides. The rest belong to the tabs this screen has not built yet,
# so they are cut and indexed but nothing reads them.
SECONDARY_BASE_PRESET = 0
SECONDARY_ACTION = 2
SECONDARY_BACKGROUND = 3
# The left column's costume row. Five rows per class, Object_Unit 0..4, which is the costume
# each cell stands for; the left panel's leftDressList has exactly five cells.
SECONDARY_COSTUME = 6

LISTS = {
    "preset": lambda icon, key: (
        icon.lower().startswith("characterpreset_") and key == SECONDARY_BASE_PRESET
    ),
    "costume": lambda icon, key: (
        icon.lower().startswith("characterpreset_") and key == SECONDARY_COSTUME
    ),
    "action": lambda icon, key: icon.lower() == "saction_01" and key == SECONDARY_ACTION,
    "background": lambda icon, key: (
        icon.lower() == "customizing_lv" and key == SECONDARY_BACKGROUND
    ),
    # Everything else keeps its own bucket so a later tab can pick it up by key. The filter
    # used to require a CharacterPreset_* package here, which silently dropped every list a
    # class draws from the shared All_Customizing_* atlases instead -- measured, that is
    # Warlord's eye make-up and cheek rows (key 10 and 11, All_Customizing_02) and
    # LanceMaster's eye make-up (key 10, All_Customizing_01), so those tabs came up empty
    # while the classes that happen to use CharacterPreset_* for the same lists were filled.
    # A bucket is the table's own category, whichever atlas the row names.
    "other": lambda icon, key: True,
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
        "select PrimaryKey, SecondaryKey, Icon, IconIndex, SortOrder, SourceRow "
        "from CharacterCustomizing"
    )
    rows = []
    for primary, secondary, icon, index, order, source in cursor.fetchall():
        rows.append(
            {
                "class": CLASSES.get(primary),
                "key": secondary,
                "icon": icon.decode("cp949", "replace"),
                "index": index,
                "order": order,
                "source": source,
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

    # SourceRow is the order the table itself lists a category in, which is the order the
    # cells appear; SortOrder repeats across categories.
    for row in sorted(rows, key=lambda r: (r["class"], r["source"])):
        kind = next((k for k, test in LISTS.items() if test(row["icon"], row["key"])), None)
        if kind is None:
            continue
        if kind == "other":
            kind = "category%d" % row["key"]
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
