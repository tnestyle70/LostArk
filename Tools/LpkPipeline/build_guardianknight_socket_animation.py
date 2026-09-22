#!/usr/bin/env python3
"""Bake DragonKnightBloodSocket's own timeline into an identity keyframe document.

An ember socket does not pop in and out: EFUI_IDENTITYDRAGONKNIGHTEMBERETHGAUGE gives the
sprite three labelled stretches that
`IdentityDragonKnightEmberethGauge.invokeDragonKnightBloodGauge` drives with
gotoAndPlay(state) on a change and gotoAndStop(state + "End") when the state is already held:

  show  f2..f21   the lit gem appears and four glow/burst layers bloom over it and are then
                  removed at f22 -- the pop -- leaving the gem alone.
  hide  f23..f38  the ember (a 35x75 flame) rises from y -25 to -41 while shrinking 0.97 -> 0.40
                  and the gem is dropped, leaving the empty socket.
  lock  f43..f55  the padlock scales 0.91 -> 1.00 into place.

CHUDRuntimeView/CUILayoutRuntime already play a `lostark.identity-keyframe-animation` document
from a label up to the next label and hold the last frame, which is exactly that contract, so
this writes one document carrying all six labels rather than three files.

The document's own frame rate is the movie's (40). Coordinates stay in the sprite's own local
twips/20 because the runtime multiplies a keyframe's x/y AND its texture size by the slot's
`keyframeAnimationScale` -- pre-scaling here would apply the HUD's 2/3 twice. The slot carries
that 2/3, and its rect is the socket's origin.

Writes:
  Client/Bin/Resources/UI/HUD/GuardianKnight/Embereth/Socket/<char>.png
  Data/UI/HUD/IdentityAnimation/GuardianKnight/EmberSocket.json
"""
from __future__ import annotations

import argparse
import json
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

from PIL import Image

EXTRACTED = Path(r"D:/ClaudeWork/Extracted")
XML = EXTRACTED / "HudGfx_dragonknight/dragonknight_embereth.xml"
PAGES = EXTRACTED / "HudGfx_dragonknight/tex_eb/EFUI_IDENTITYDRAGONKNIGHTEMBERETHGAUGE"
SOCKET_SPRITE = "32"


def number(value, fallback=1.0):
    try:
        return float(value)
    except (TypeError, ValueError):
        return fallback


class Movie:
    def __init__(self, path: Path):
        root = ET.parse(path).getroot()
        self.frame_rate = number(root.get("frameRate"), 40.0)
        self.chars, self.sub, self.ext = {}, {}, {}
        for item in root.iter("item"):
            kind = item.get("type", "")
            cid = item.get("characterID") or item.get("spriteId") or item.get("shapeId")
            if kind == "DefineSubImage":
                self.sub[item.get("characterID")] = tuple(
                    int(item.get(k)) for k in ("x1", "y1", "x2", "y2"))
            elif kind == "DefineExternalImage2":
                self.ext[item.get("imageID")] = item.get("fileName")
            elif cid:
                self.chars[cid] = item

    def bitmap(self, character, depth=0):
        """The one atlas rect a socket character draws, through its wrapping sprite."""
        if character in self.sub:
            return character
        item = self.chars.get(character)
        if item is None or depth > 6:
            return None
        for child in item.iter("item"):
            for key in ("characterId", "bitmapId", "fillBitmapId", "shapeId"):
                value = child.get(key)
                if value in self.sub:
                    return value
                if value and value != character:
                    found = self.bitmap(value, depth + 1)
                    if found:
                        return found
        return None


def matrix_of(tag):
    element = tag.find("matrix")
    if element is None:
        return None
    has_scale = element.get("hasScale") == "true"
    return {
        "x": number(element.get("translateX"), 0.0) / 20.0,
        "y": number(element.get("translateY"), 0.0) / 20.0,
        "scaleX": number(element.get("scaleX")) if has_scale else 1.0,
        "scaleY": number(element.get("scaleY")) if has_scale else 1.0,
    }


def alpha_of(tag):
    """The placement's own alpha. A CXFORMWITHALPHA multiplier is out of 256, and it is what
    fades the burst away and drops the gem at the start of `hide` -- a frame that carries only
    a colour transform and no matrix. Reading the matrix alone left both drawn."""
    element = tag.find("colorTransform")
    if element is None or element.get("hasMultTerms") != "true":
        return None
    return max(0.0, min(1.0, number(element.get("alphaMultTerm"), 256.0) / 256.0))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    repo = parser.parse_args().repo

    movie = Movie(XML)
    # The one external page this document has; every sub-image is a rect inside it.
    page_file = next(iter(movie.ext.values()))
    page = Image.open(PAGES / (Path(page_file).stem.lower() + ".png")).convert("RGBA")

    sprite = movie.chars[SOCKET_SPRITE]
    timeline, labels = [], {}
    display = {}
    frame = 1
    for tag in sprite.find("subTags").iter("item"):
        kind = tag.get("type", "")
        if kind == "FrameLabelTag":
            labels[tag.get("name")] = frame
        elif kind.startswith("PlaceObject"):
            depth = tag.get("depth")
            character = tag.get("characterId")
            entry = dict(display.get(depth) or {})
            if character:
                entry["character"] = character
            placed = matrix_of(tag)
            if placed:
                entry.update(placed)
            alpha = alpha_of(tag)
            if alpha is not None:
                entry["alpha"] = alpha
            elif character:
                entry["alpha"] = 1.0
            entry.setdefault("alpha", 1.0)
            entry.setdefault("scaleX", 1.0)
            entry.setdefault("scaleY", 1.0)
            entry.setdefault("x", 0.0)
            entry.setdefault("y", 0.0)
            display[depth] = entry
        elif kind == "RemoveObject2Tag":
            display.pop(tag.get("depth"), None)
        elif kind == "ShowFrameTag":
            timeline.append({d: dict(e) for d, e in display.items()})
            frame += 1
    timeline.append({d: dict(e) for d, e in display.items()})

    # Crop every character the timeline uses, once.
    assets, used = {}, sorted({e["character"] for f in timeline for e in f.values()
                               if e.get("character")}, key=int)
    out_dir = repo / "Client/Bin/Resources/UI/HUD/GuardianKnight/Embereth/Socket"
    out_dir.mkdir(parents=True, exist_ok=True)
    sizes = {}
    for character in used:
        sub_id = movie.bitmap(character)
        if sub_id is None:
            print("   비트맵을 찾지 못함: char %s" % character)
            continue
        x1, y1, x2, y2 = movie.sub[sub_id]
        name = "Socket_%s.png" % character
        page.crop((x1, y1, x2, y2)).save(out_dir / name)
        assets[character] = "UI/HUD/GuardianKnight/Embereth/Socket/" + name
        sizes[character] = (x2 - x1, y2 - y1)

    depths = sorted({d for f in timeline for d in f}, key=int)
    layers = []
    for depth in depths:
        keys = []
        previous = None
        for index, snapshot in enumerate(timeline):
            entry = snapshot.get(depth)
            if entry is None or entry.get("character") not in assets:
                current = {"asset": None, "x": 0.0, "y": 0.0,
                           "scaleX": 1.0, "scaleY": 1.0, "alpha": 0.0}
            else:
                current = {"asset": assets[entry["character"]],
                           "x": round(entry["x"], 4),
                           "y": round(entry["y"], 4),
                           "scaleX": round(entry["scaleX"], 4),
                           "scaleY": round(entry["scaleY"], 4),
                           "alpha": round(entry["alpha"], 4)}
            if current != previous:
                keys.append(dict(frame=index, rotationDeg=0.0, additive=False, **current))
                previous = current
        if keys:
            layers.append({"depth": int(depth), "sourceKey": "depth%s" % depth,
                           "keyframes": keys})

    document = {
        "schema": "lostark.identity-keyframe-animation",
        "formatVersion": 1,
        "frameRate": movie.frame_rate,
        "frameCount": len(timeline),
        "labels": {name: index - 1 for name, index in sorted(labels.items(),
                                                             key=lambda kv: kv[1])},
        "layers": layers,
    }
    target = repo / "Data/UI/HUD/IdentityAnimation/GuardianKnight/EmberSocket.json"
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps(document, ensure_ascii=False, indent=1) + "\n",
                      encoding="utf-8")
    print("소켓 애니메이션: %d프레임, 레이어 %d개, 라벨 %s" % (
        len(timeline), len(layers), ", ".join(document["labels"])))
    print("조각 %d개 -> %s" % (len(assets), out_dir))
    for character in used:
        if character in sizes:
            print("   char %-4s %dx%d" % (character, *sizes[character]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
