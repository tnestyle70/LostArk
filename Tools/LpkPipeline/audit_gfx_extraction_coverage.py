#!/usr/bin/env python3
"""Account for every element of a .gfx against what we actually extracted.

Why this exists
---------------
Rebuilding a retail screen has repeatedly shipped with pieces missing, and every
time the cause was the same shape of mistake: an extractor emits only the things
it knows how to classify and drops the rest without saying so. The MVP page lost
all four 3D character slots (`mvpGFxRenderTarget`, `renderTex0/1/2`) because they
are PlaceObject targets with no image, and the keyframe extractor only emitted
layers that resolved to an image asset. Nothing failed; the timing and draw order
simply were not in the data.

So this does not extract anything. It enumerates *everything* the gfx contains
and reports what our Data documents do not mention, so a gap has to be either
fixed or explicitly written off -- it cannot pass silently.

What it enumerates
------------------
  - every DefineSprite, with its frame count and whether its timeline animates
  - every named instance (PlaceObject name=), the sprite and depth it sits at,
    whether it is moved/faded over the timeline, and its character type
  - every DefineEditText, with font class, size and initial text
  - every frame label
  - every SymbolClass binding (AS3 class -> character)
  - every image the shapes reference
  - every embedded sound tag, so "this screen has none" is a reported fact rather
    than an assumption -- and, with --sound-library, the cues that exist in the
    extracted audio for this screen's keywords but that nothing in --data plays

Coverage is checked by looking for each name/asset in the Data documents given
with --data. Matching is by substring, deliberately loose: the point is to
surface what is definitely absent, not to prove what is present is correct.

Usage:
  python audit_gfx_extraction_coverage.py --xml <screen.xml> \
      --data Data/UI/MVP --sprite 275 [--sprite 221] [--json report.json]
"""

from __future__ import annotations

import argparse
import json
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

PLACE = ("PlaceObject2Tag", "PlaceObject3Tag", "PlaceObjectTag")
SOUND_TAGS = ("DefineSoundTag", "SoundStreamHeadTag", "SoundStreamHead2Tag",
              "StartSoundTag", "StartSound2Tag", "DefineButtonSoundTag")


def gather(root):
    """Every sprite, named instance, edit text, label, symbol class and image."""
    sprites, texts, labels, symbols, images = {}, [], [], [], {}

    for item in root.iter("item"):
        kind = item.get("type", "")

        if kind == "DefineEditTextTag":
            texts.append({
                "characterId": item.get("characterID"),
                "fontClass": item.get("fontClass"),
                "fontHeightPt": (int(item.get("fontHeight") or 0) / 20.0),
                "initialText": (item.get("initialText") or "")[:120],
            })
        elif kind == "SymbolClassTag":
            for tag in item.iter("tags"):
                pass
            names = [n.text for n in item.iter() if n.tag == "names" and n.text]
            symbols.extend(names)
        elif kind == "DefineSubImage":
            images[item.get("characterID")] = {
                "imageId": item.get("imageId"),
                "rect": [item.get("x1"), item.get("y1"), item.get("x2"), item.get("y2")],
            }
        elif kind in ("DefineExternalImage2", "DefineExternalImage"):
            images[item.get("characterId") or item.get("characterID")] = {
                "external": item.get("exportName") or item.get("fileName"),
                "idType": item.get("idType"),
            }
        elif kind == "FrameLabelTag":
            labels.append(item.get("name"))

        if kind != "DefineSpriteTag":
            continue

        sub = item.find("subTags")
        if sub is None:
            continue
        frame, names, moved, faded, depth_of, char_of = 1, {}, set(), set(), {}, {}
        for tag in sub:
            tkind = tag.get("type", "")
            if tkind == "ShowFrameTag":
                frame += 1
                continue
            if tkind == "FrameLabelTag":
                labels.append("%s@sprite%s:f%d"
                              % (tag.get("name"), item.get("spriteId"), frame))
                continue
            if tkind not in PLACE:
                continue
            depth = tag.get("depth")
            if tag.get("placeFlagHasName") == "true":
                names[depth] = tag.get("name")
            if tag.get("characterId"):
                char_of.setdefault(depth, tag.get("characterId"))
            depth_of[depth] = depth
            if tag.get("placeFlagMove") == "true":
                moved.add(depth)
            if tag.find("colorTransform") is not None:
                faded.add(depth)
        sprites[item.get("spriteId")] = {
            "frameCount": int(item.get("frameCount") or 1),
            "walkedFrames": frame,
            "depths": sorted(depth_of, key=lambda d: int(d)),
            "names": names,
            "moved": sorted(moved, key=int),
            "faded": sorted(faded, key=int),
            "characterOf": char_of,
        }
    return sprites, texts, labels, symbols, images


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--xml", type=Path, required=True)
    parser.add_argument("--data", type=Path, action="append", default=[],
                        help="directory or file of extracted Data documents")
    parser.add_argument("--sprite", action="append", default=[],
                        help="report these sprites in full (repeatable)")
    parser.add_argument("--sound-library", type=Path,
                        help="directory of extracted cue wavs to cross-reference")
    parser.add_argument("--sound-keyword", action="append", default=[],
                        help="cue name substring this screen would use (repeatable)")
    parser.add_argument("--json", type=Path)
    args = parser.parse_args()

    haystack = ""
    for entry in args.data:
        files = sorted(entry.rglob("*.json")) if entry.is_dir() else [entry]
        for path in files:
            haystack += path.read_text(encoding="utf-8", errors="ignore")

    root = ET.parse(args.xml).getroot()
    sprites, texts, labels, symbols, images = gather(root)

    every_name = {}
    for sprite_id, sprite in sprites.items():
        for depth, name in sprite["names"].items():
            every_name.setdefault(name, []).append((sprite_id, depth))

    print("=== %s ===" % args.xml)
    print("sprites %d   named instances %d   edit texts %d   labels %d   "
          "symbol classes %d   images %d"
          % (len(sprites), len(every_name), len(texts), len(labels),
             len(symbols), len(images)))

    for sprite_id in args.sprite:
        sprite = sprites.get(sprite_id)
        if sprite is None:
            print("\n!! sprite %s not in this file" % sprite_id)
            continue
        print("\n--- sprite %s : %d frames, %d placements ---"
              % (sprite_id, sprite["frameCount"], len(sprite["depths"])))
        for depth in sprite["depths"]:
            name = sprite["names"].get(depth)
            flags = "".join(("M" if depth in sprite["moved"] else "-",
                             "A" if depth in sprite["faded"] else "-"))
            covered = "" if name is None else (
                "  OK" if name in haystack else "  <== 추출본에 없음")
            print("   depth %-4s %s char=%-5s %s%s"
                  % (depth, flags, sprite["characterOf"].get(depth, "-"),
                     name or "(unnamed art)", covered))

    missing = sorted(n for n in every_name if n not in haystack)
    print("\n=== 추출본이 언급하지 않는 named instance %d개 (전체 %d개 중) ==="
          % (len(missing), len(every_name)))
    for name in missing:
        where = ", ".join("sprite%s:d%s" % (s, d) for s, d in every_name[name][:3])
        print("   %-40s %s" % (name, where))

    sound_tags = [i.get("type") for i in root.iter("item")
                  if i.get("type") in SOUND_TAGS]
    print("\n=== gfx 내장 사운드 태그 %d개 ===" % len(sound_tags))
    if not sound_tags:
        print("   없음 -- 이 화면의 소리는 호스트가 재생한다. gfx만 봐서는 알 수 없으므로"
              " --sound-library 로 추출된 큐와 대조할 것.")
    else:
        for kind in sorted(set(sound_tags)):
            print("   %-24s %d" % (kind, sound_tags.count(kind)))

    if args.sound_library and args.sound_keyword:
        cues = sorted(p.name for p in args.sound_library.rglob("*.wav"))
        hits = [c for c in cues
                if any(k.lower() in c.lower() for k in args.sound_keyword)]
        unused = [c for c in hits if c.rsplit(".", 1)[0] not in haystack]
        print("\n=== 추출 오디오 중 이 화면 키워드에 맞는 %d개, 그 중 코드가 "
              "재생하지 않는 %d개 ===" % (len(hits), len(unused)))
        for name in unused:
            print("   %s" % name)

    missing_fonts = sorted({t["fontClass"] for t in texts
                            if t["fontClass"] and t["fontClass"] not in haystack})
    if missing_fonts:
        print("\n=== 추출본이 언급하지 않는 fontClass ===")
        for font in missing_fonts:
            sizes = sorted({t["fontHeightPt"] for t in texts
                            if t["fontClass"] == font})
            print("   %-22s %s pt" % (font, ", ".join("%g" % s for s in sizes)))

    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps({
            "sprites": sprites, "editTexts": texts, "labels": labels,
            "symbolClasses": symbols, "images": images,
            "namedInstancesMissingFromData": missing,
            "embeddedSoundTags": sound_tags,
        }, ensure_ascii=False, indent=1), encoding="utf-8")
        print("\nwrote %s" % args.json)
    return 0


if __name__ == "__main__":
    sys.exit(main())
