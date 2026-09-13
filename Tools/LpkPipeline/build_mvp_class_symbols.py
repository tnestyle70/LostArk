#!/usr/bin/env python3
"""Build Data/UI/MVP/MvpClassSymbols.json -- the class emblem the award page
watermarks behind the MVP, and the small one each party column shows.

Chain
-----
  mvp.gfx d194 classMc
    -> Shared_MvpClassBigSymbol   (componentsv2)
         frame label "class_N" places MvpClassIcon_NN at its own offset
    -> MvpClassIcon_NN            (shareimagev2)

The host gotoAndStop()s the label for the player's class, so the label number is
the class key and each label carries its icon's own offset -- the emblems are
not centred on a common origin.

The party columns use goldClassIcon_<CODE> from shareiconimage instead, which is
named by the class code rather than numbered.

Usage:
  python build_mvp_class_symbols.py --componentsv2 <componentsv2.xml>
                                    --shareiconimage <symbols_shareiconimage.txt>
                                    --mvp <mvp.xml>
                                    --out Data/UI/MVP/MvpClassSymbols.json
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

PLACE_CLASS = re.compile(r'\sclassName="(MvpClassIcon_\d+)"')
MATRIX_ATTRS = re.compile(r'translateX="(-?\d+)"[^>]*translateY="(-?\d+)"')
GOLD = re.compile(r'^\s*(\d+)\s+(goldClassIcon_\w+)\s*$', re.MULTILINE)

TWIPS = 20.0

# Which class_N label each playable class uses.
#
# NOT EXTRACTED from a table: nothing in the client data joins a class id to
# these labels, and the project's own catalog renamed every body model to an
# English project name, so the retail code is gone from our side.  The project
# owner read the extracted icons and named them.
#
# Three of the four are corroborated: componentsv2 places <CODE>_classPlay under
# the same class_N labels, and class_9 is GL (Gunlancer = Warlord), class_25 is
# LM (Lance Master), class_23 is YS (YinyangShi, which is Artist's internal
# codename).  class_41 carries no _classPlay entry.
#
# Gunslinger and Slayer are in the class enum but not built, so they are left
# out rather than mapped on a guess.
PLAYABLE_CLASS_LABELS = [
    {"networkClassId": "WARLORD", "classKey": 9, "sourceCode": "GL"},
    {"networkClassId": "ARTIST", "classKey": 23, "sourceCode": "YS"},
    {"networkClassId": "LANCE_MASTER", "classKey": 25, "sourceCode": "LM"},
    {"networkClassId": "DIMENSIONMASTER", "classKey": 41, "sourceCode": None},
]


def parse_big_symbol(path: Path):
    """Every class_N label and the icon placed right after it.

    The dump writes one tag per line with its attributes in whatever order the
    writer chose, so this scans lines and reads the attributes it needs rather
    than matching a fixed shape."""
    entries = []
    pending_label = None
    pending_icon = None

    with path.open(encoding="utf-8", errors="replace") as handle:
        for line in handle:
            if 'type="FrameLabelTag"' in line:
                match = re.search(r'\sname="(class_\d+)"', line)
                if match:
                    pending_label = match.group(1)
                continue

            if 'type="PlaceObject3Tag"' in line and pending_label:
                match = PLACE_CLASS.search(line)
                if match:
                    pending_icon = match.group(1)
                    # A self-closing place tag carries no matrix line after it.
                    inline = MATRIX_ATTRS.search(line)
                    if inline:
                        entries.append(_entry(pending_label, pending_icon,
                                              inline))
                        pending_label = pending_icon = None
                continue

            if pending_icon and "<matrix" in line:
                match = MATRIX_ATTRS.search(line)
                entries.append(_entry(pending_label, pending_icon, match))
                pending_label = pending_icon = None

    entries.sort(key=lambda e: e["classKey"])
    return entries


def _entry(label: str, icon: str, matrix_match):
    tx = int(matrix_match.group(1)) / TWIPS if matrix_match else 0.0
    ty = int(matrix_match.group(2)) / TWIPS if matrix_match else 0.0
    return {
        "label": label,
        "classKey": int(label.split("_")[1]),
        "icon": icon,
        "offsetX": tx,
        "offsetY": ty,
    }


def parse_gold_icons(path: Path):
    text = path.read_text(encoding="utf-8", errors="replace")
    icons = [{"characterId": int(cid), "symbol": name}
             for cid, name in GOLD.findall(text)]
    icons.sort(key=lambda i: i["symbol"])
    return icons


def parse_placement(path: Path):
    """Where mvp.gfx puts the two emblems, and the watermark's fade.

    depth 194 is classMc on MvpResultFrame -- placed with alphaMultTerm 0 and
    stepped up one keyframe at a time until the colorTransform is dropped
    altogether, which is full opacity. depth 27 inside the party column sprite
    is that column's own small classMc, constant."""
    sprite = None
    frame = 1
    pending = None
    big_place = None
    fade = []
    column = None

    with path.open(encoding="utf-8", errors="replace") as handle:
        for line in handle:
            if 'type="DefineSpriteTag"' in line:
                match = re.search(r'spriteId="(\d+)"', line)
                sprite, frame, pending = (match.group(1) if match else None), 1, None
                continue
            if 'type="ShowFrameTag"' in line:
                frame += 1
                pending = None
                continue
            if "PlaceObject" in line:
                depth = re.search(r'\sdepth="(\d+)"', line)
                pending = None
                if not depth:
                    continue
                if depth.group(1) == "194":
                    pending = {"frame": frame, "kind": "big", "alpha": None}
                    fade.append(pending)
                    if 'name="classMc"' in line:
                        big_place = pending
                        pending["initial"] = True
                elif depth.group(1) == "27" and 'name="classMc"' in line:
                    pending = {"kind": "column"}
                    column = pending
                continue
            if pending is None:
                continue
            if "<matrix" in line:
                tx = re.search(r'translateX="(-?\d+)"', line)
                ty = re.search(r'translateY="(-?\d+)"', line)
                sx = re.search(r'scaleX="([\d.]+)"', line)
                if tx:
                    pending["x"] = int(tx.group(1)) / TWIPS
                if ty:
                    pending["y"] = int(ty.group(1)) / TWIPS
                if sx:
                    pending["scale"] = float(sx.group(1))
            elif "<colorTransform" in line:
                alpha = re.search(r'alphaMultTerm="(-?\d+)"', line)
                pending["alpha"] = int(alpha.group(1)) / 256.0 if alpha else 1.0

    if big_place is None or column is None:
        raise SystemExit("mvp.gfx does not place classMc where expected")

    keyed = [f for f in fade if f.get("alpha") is not None and not f.get("initial")]
    keyed.sort(key=lambda f: f["frame"])
    # The last keyframe drops the colorTransform, so it lands on full opacity.
    final = [f for f in fade if f.get("alpha") is None and not f.get("initial")]

    return {
        "bigSymbol": {
            "stageX": big_place.get("x", 0.0),
            "stageY": big_place.get("y", 0.0),
            "fadeInStartFrame": keyed[0]["frame"] if keyed else 0,
            "fadeInEndFrame": (final[-1]["frame"] if final
                               else (keyed[-1]["frame"] if keyed else 0)),
            "holdAlpha": 1.0,
        },
        "partyColumnIcon": {
            "localX": column.get("x", 0.0),
            "localY": column.get("y", 0.0),
            "scale": column.get("scale", 1.0),
        },
    }


def parse_icon_sizes(structure: Path, symbols: Path):
    """MvpClassIcon_NN -> its pixel size, from the DefineSubImage rect.

    The page needs an explicit rect to draw into, and the icons are not a
    uniform size (178x179 to 193x181 among the playable four)."""
    ids = {}
    for line in symbols.read_text(encoding="utf-8", errors="replace").splitlines():
        match = re.match(r"\s*(\d+)\s+(MvpClassIcon_\d+)\s*$", line)
        if match:
            ids[int(match.group(1))] = match.group(2)

    sizes = {}
    with structure.open(encoding="utf-8", errors="replace") as handle:
        for line in handle:
            if 'type="DefineSubImage"' not in line:
                continue
            attrs = dict(re.findall(r'(\w+)="(-?\d+)"', line))
            name = ids.get(int(attrs["characterID"]))
            if name:
                sizes[name] = (int(attrs["x2"]) - int(attrs["x1"]),
                               int(attrs["y2"]) - int(attrs["y1"]))
    return sizes


def resolve_playable(big, sizes):
    """Join the named classes onto the labels actually present."""
    by_key = {entry["classKey"]: entry for entry in big}
    rows = []
    for wanted in PLAYABLE_CLASS_LABELS:
        entry = by_key.get(wanted["classKey"])
        if entry is None:
            raise SystemExit("class_%d is not in the symbol, so %s cannot be "
                             "mapped" % (wanted["classKey"],
                                         wanted["networkClassId"]))
        rows.append({
            "networkClassId": wanted["networkClassId"],
            "classKey": entry["classKey"],
            "label": entry["label"],
            "icon": entry["icon"],
            "offsetX": entry["offsetX"],
            "offsetY": entry["offsetY"],
            "sourceCode": wanted["sourceCode"],
            "width": sizes.get(entry["icon"], (0, 0))[0],
            "height": sizes.get(entry["icon"], (0, 0))[1],
            "bigAsset": "UI/MVP/ClassIcons/%s.png" % entry["icon"],
            "smallAsset": "UI/MVP/ClassIcons/MvpClassIconSmall_%02d.png"
                          % entry["classKey"],
        })
    return rows


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--componentsv2", type=Path, required=True)
    parser.add_argument("--shareiconimage", type=Path, required=True)
    parser.add_argument("--mvp", type=Path, required=True)
    parser.add_argument("--shareimagev2", type=Path, required=True)
    parser.add_argument("--symbols-v2", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()

    big = parse_big_symbol(args.componentsv2)
    gold = parse_gold_icons(args.shareiconimage)
    if not big:
        raise SystemExit("no class_N frame placed a MvpClassIcon")
    if not gold:
        raise SystemExit("no goldClassIcon symbols found")

    document = {
        "formatVersion": 1,
        "source": "componentsv2 Shared_MvpClassBigSymbol frame labels class_N; "
                  "shareiconimage goldClassIcon_<CODE>",
        "note": "classKey is the frame label number the host gotoAndStop()s, "
                "not a CHARACTER_CLASS_ID. Offsets are the icon's own placement "
                "inside the symbol, in stage pixels.",
        "bigSymbols": big,
        "goldIcons": gold,
        "classes": resolve_playable(
            big, parse_icon_sizes(args.shareimagev2, args.symbols_v2)),
        "placement": parse_placement(args.mvp),
    }

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(
        json.dumps(document, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8")
    print("%s: %d big symbols, %d gold icons"
          % (args.out, len(big), len(gold)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
