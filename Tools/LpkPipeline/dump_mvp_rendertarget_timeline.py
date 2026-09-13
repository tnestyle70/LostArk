#!/usr/bin/env python3
"""Dump the mvp.gfx timeline of the 3D character render-target slots.

Why this exists
---------------
The MVP award page draws each winner into a host render target, and the gfx
declares those slots as plain PlaceObject targets with no image:
`mvpGFxRenderTarget` for the MVP panel and `renderTex0/1/2` for the party
columns. The layout/keyframe extractor only emitted layers that resolve to an
image asset, so every one of these was silently dropped -- which is why the
first build revealed all four characters at once with no fade.

Their reveal is authored on the enclosing sprite's own timeline as
PlaceObject *move* tags carrying a colorTransform (alpha) and a matrix, so the
timing is recoverable exactly: walk the sprite's subtags, count ShowFrameTag,
and record the state of the interesting depths at each frame.

Also reported is the depth of every other placement in the same sprite, because
draw order is the other thing the slots carry: the render targets sit below the
name/stat/class text, so the character is drawn behind that UI.

Usage:
  python dump_mvp_rendertarget_timeline.py --xml <mvp.xml> [--json out.json]
"""

from __future__ import annotations

import argparse
import json
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

DEFAULT_NAMES = ("mvpGFxRenderTarget", "renderTex0", "renderTex1", "renderTex2")
TARGET_NAMES = DEFAULT_NAMES
TWIPS = 20.0


def matrix_of(node):
    m = node.find("matrix")
    if m is None:
        return None
    scale_x = float(m.get("scaleX", 1.0)) if m.get("hasScale") == "true" else 1.0
    scale_y = float(m.get("scaleY", 1.0)) if m.get("hasScale") == "true" else 1.0
    return {
        "x": float(m.get("translateX", 0)) / TWIPS,
        "y": float(m.get("translateY", 0)) / TWIPS,
        "scaleX": scale_x,
        "scaleY": scale_y,
    }


def alpha_of(node):
    """Alpha this placement states, or None when it states none.

    A CXFORMWITHALPHA with hasMultTerms false is the *identity* transform -- the
    end of a fade, alpha 1.0 -- not an absent one. Flash writes exactly that on a
    tween's last frame, so reading it as "no value" and carrying the previous key
    forward leaves every faded-in panel stuck a few percent short of opaque.
    """
    c = node.find("colorTransform")
    if c is None:
        return None
    if c.get("hasMultTerms") != "true":
        return 1.0
    # CXFORMWITHALPHA mult terms are 8.8 fixed point: 256 == 1.0
    return round(float(c.get("alphaMultTerm", 256)) / 256.0, 4)


def sprite_timeline(sprite):
    """Frame-indexed placement state for every depth in one DefineSpriteTag."""
    frame = 1
    names, rows = {}, []
    for tag in sprite.find("subTags"):
        kind = tag.get("type", "")
        if kind == "ShowFrameTag":
            frame += 1
            continue
        if kind in ("PlaceObject2Tag", "PlaceObject3Tag"):
            depth = int(tag.get("depth"))
            if tag.get("placeFlagHasName") == "true":
                names[depth] = tag.get("name")
            rows.append({
                "frame": frame,
                "depth": depth,
                "name": names.get(depth),
                "move": tag.get("placeFlagMove") == "true",
                "characterId": tag.get("characterId"),
                "matrix": matrix_of(tag),
                "alpha": alpha_of(tag),
            })
        elif kind == "RemoveObject2Tag":
            depth = int(tag.get("depth"))
            rows.append({"frame": frame, "depth": depth,
                         "name": names.get(depth), "remove": True})
    return frame, names, rows


"""Which authored instance owns each staged character panel.

The MVP winner is drawn straight into `mvpGFxRenderTarget`. The three party
columns have no per-column render-target timeline of their own -- `renderTex0/1/2`
sit at frame 1 of a one-frame sprite -- because the whole column, character and
text together, is one `otherStatItemN` instance that the page's timeline fades
and slides as a unit. So the column character follows its otherStatItem.
"""
STAGE_OWNERS = (
    ("mvp", "mvpGFxRenderTarget"),
    ("party0", "otherStatItem0"),
    ("party1", "otherStatItem1"),
    ("party2", "otherStatItem2"),
)

# mvp.gfx authors on a 1920x1080 stage; this project's UI canvas is 1280x720.
STAGE_TO_CANVAS = 1280.0 / 1920.0


def reveal_document(rows_by_name, frame_rate, frame_count):
    """Per-slot alpha and offset-from-settled curves, in canvas units.

    SWF carries a placement's colorTransform and matrix independently and omits
    whichever the tween did not touch that frame, so each is forward-filled from
    the last frame that stated it. Offsets are expressed against the slot's final
    authored position, which is what a caller that already knows where the panel
    ends up can apply directly.
    """
    slots = {}
    for key, name in STAGE_OWNERS:
        rows = [r for r in rows_by_name.get(name, []) if not r.get("remove")]
        if not rows:
            continue
        settled = next((r["matrix"] for r in reversed(rows) if r["matrix"]), None)
        alpha, matrix, keys = 1.0, settled, []
        for row in rows:
            if row["alpha"] is not None:
                alpha = row["alpha"]
            if row["matrix"] is not None:
                matrix = row["matrix"]
            keys.append({
                "frame": row["frame"],
                "alpha": alpha,
                "dx": round((matrix["x"] - settled["x"]) * STAGE_TO_CANVAS, 4),
                "dy": round((matrix["y"] - settled["y"]) * STAGE_TO_CANVAS, 4),
            })
        slots[key] = {"instance": name, "keyframes": keys}
    return {"frameRate": frame_rate, "frameCount": frame_count, "slots": slots}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--xml", type=Path, required=True)
    parser.add_argument("--json", type=Path)
    parser.add_argument("--emit-reveal", type=Path,
                        help="write the staged-character reveal document "
                             "(alpha + offset per frame) here")
    parser.add_argument("--names", default="",
                        help="comma separated instance names; default is the "
                             "four 3D render-target slots")
    args = parser.parse_args()

    global TARGET_NAMES
    if args.names:
        TARGET_NAMES = tuple(n.strip() for n in args.names.split(",") if n.strip())
    elif args.emit_reveal:
        TARGET_NAMES = tuple(name for _key, name in STAGE_OWNERS)

    root = ET.parse(args.xml).getroot()
    out = []
    for sprite in root.iter("item"):
        if sprite.get("type") != "DefineSpriteTag":
            continue
        sub = sprite.find("subTags")
        if sub is None:
            continue
        placed = {t.get("name") for t in sub
                  if t.get("type", "").startswith("PlaceObject")}
        if not placed & set(TARGET_NAMES):
            continue

        frames, names, rows = sprite_timeline(sprite)
        sprite_id = sprite.get("spriteId")
        print("=== DefineSprite %s  frameCount=%s  walked frames=%d ==="
              % (sprite_id, sprite.get("frameCount"), frames))

        print("  depth order:")
        for depth in sorted(names):
            mark = "  <== 3D" if names[depth] in TARGET_NAMES else ""
            print("    depth %-3d  %s%s" % (depth, names[depth], mark))

        for target in TARGET_NAMES:
            hits = [r for r in rows if r.get("name") == target]
            if not hits:
                continue
            print("  --- %s : %d timeline rows" % (target, len(hits)))
            for r in hits:
                if r.get("remove"):
                    print("      f%-4d remove" % r["frame"])
                    continue
                m = r["matrix"]
                print("      f%-4d %-6s alpha=%-7s %s"
                      % (r["frame"], "move" if r["move"] else "place",
                         "-" if r["alpha"] is None else r["alpha"],
                         "-" if m is None else
                         "x=%.1f y=%.1f sx=%.4f sy=%.4f"
                         % (m["x"], m["y"], m["scaleX"], m["scaleY"])))
        out.append({"spriteId": sprite_id, "frames": frames,
                    "depths": {str(d): n for d, n in sorted(names.items())},
                    "rows": [r for r in rows if r.get("name") in TARGET_NAMES]})

    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(out, ensure_ascii=False, indent=2),
                             encoding="utf-8")
        print("\nwrote %s" % args.json)

    if args.emit_reveal:
        rows_by_name, frames, rate = {}, 0, 40
        for sprite in root.iter("item"):
            if sprite.get("type") != "DefineSpriteTag":
                continue
            sub = sprite.find("subTags")
            if sub is None:
                continue
            placed = {t.get("name") for t in sub
                      if t.get("type", "").startswith("PlaceObject")}
            if not placed & set(TARGET_NAMES):
                continue
            walked, _names, rows = sprite_timeline(sprite)
            frames = max(frames, int(sprite.get("frameCount") or walked))
            for row in rows:
                if row.get("name") in TARGET_NAMES:
                    rows_by_name.setdefault(row["name"], []).append(row)

        header = root.find(".//item[@type='SetFrameRateTag']")
        if header is not None and header.get("frameRate"):
            rate = float(header.get("frameRate"))
        document = reveal_document(rows_by_name, rate, frames)
        args.emit_reveal.parent.mkdir(parents=True, exist_ok=True)
        args.emit_reveal.write_text(
            json.dumps(document, ensure_ascii=False, indent=1), encoding="utf-8")
        print("\nwrote %s  (%d fps, %d frames)"
              % (args.emit_reveal, document["frameRate"], document["frameCount"]))
        for key, slot in document["slots"].items():
            lit = [k for k in slot["keyframes"] if k["alpha"] > 0.0]
            print("   %-7s %-20s alpha>0 f%d..f%d, %d keys"
                  % (key, slot["instance"],
                     lit[0]["frame"] if lit else -1,
                     slot["keyframes"][-1]["frame"], len(slot["keyframes"])))
    return 0


if __name__ == "__main__":
    sys.exit(main())
