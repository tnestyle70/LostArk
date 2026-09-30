#!/usr/bin/env python3
"""Write Data/Sound/CharacterVoiceTypes.json: which CharacterSoundCatalog.json
wav belongs to which character-creation voice type (Type1..Type8).

A voice line event plays through a Switch container keyed by the voice type
the player picked at creation; each branch is that voice's own takes. The
catalog flattened every branch into one list, so the runtime had no way to
keep one voice. This walks the shipped Wwise packages again and records the
branch each catalog wav came from. Events without a voice switch are left out.

  python build_character_voice_types.py [--dry-run]
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent))
import wwise_audio_package as wwise  # noqa: E402

SOUND_CATALOG = REPO_ROOT / "Data" / "Sound" / "CharacterSoundCatalog.json"
VOICE_TYPES = REPO_ROOT / "Data" / "Sound" / "CharacterVoiceTypes.json"
FORMAT_VERSION = 1
DEFAULT_VOICE_TYPE = "Type1"

CLASS_PACKAGES = {
    "Common": "SOUND_PC_COMMON",
    "LanceMaster": "SOUND_PC_LANCEMASTER",
    "Artist": "SOUND_PC_YINYANGSHI",
    "DimensionMaster": "SOUND_PC_DIMENSIONMASTER",
    "Warlord": "SOUND_PC_GUNLANCER",
    "GuardianKnight": "SOUND_PC_DRAGONKNIGHT",
}

TYPE_NAMES = {wwise.fnv1_32("Type%d" % i): "Type%d" % i for i in range(1, 9)}
MEDIA_SUFFIX = re.compile(r"__(\d+)\.wav$")


def media_voice_types(target: int, objects: dict, out: dict[int, set[str]],
                      seen: set[int], voice: str | None = None) -> None:
    if target in seen or target not in objects:
        return
    seen.add(target)
    hirc_type, payload = objects[target]
    if hirc_type == wwise.HIRC_SOUND:
        if voice is not None:
            out.setdefault(wwise.sound_source_id(payload), set()).add(voice)
        return
    if hirc_type not in wwise.HIRC_CONTAINERS:
        return
    children = wwise.container_children(payload, objects)
    if hirc_type == wwise.HIRC_SWITCH:
        branches = wwise.switch_branches(payload, children)
        if any(switch_id in TYPE_NAMES for switch_id, _ in branches):
            for switch_id, nodes in branches:
                for node in nodes:
                    media_voice_types(node, objects, out, seen, TYPE_NAMES.get(switch_id))
            return
    for child in children:
        media_voice_types(child, objects, out, seen, voice)


def event_voice_types(event: str, objects: dict) -> dict[int, set[str]]:
    entry = objects.get(wwise.fnv1_32(event))
    if entry is None or entry[0] != wwise.HIRC_EVENT:
        raise KeyError(event)
    out: dict[int, set[str]] = {}
    for action_id in wwise.event_action_ids(entry[1]):
        action = objects.get(action_id)
        if action is None or action[0] != wwise.HIRC_ACTION:
            continue
        action_type, target = wwise.action_fields(action[1])
        if action_type == wwise.ACTION_PLAY:
            media_voice_types(target, objects, out, set())
    return out


def build(catalog: dict, previous: dict) -> tuple[dict, list[str]]:
    classes: dict = {}
    problems: list[str] = []
    for class_name, package_filter in CLASS_PACKAGES.items():
        events = catalog["classes"].get(class_name)
        if not events:
            continue
        paths = wwise.find_packages(wwise.DEFAULT_PACKAGE_ROOT, package_filter)
        if not paths:
            problems.append("%s: no %s* package under %s" % (class_name, package_filter, wwise.DEFAULT_PACKAGE_ROOT))
            continue
        objects = wwise.merged_objects(wwise.load_packages(paths))
        voiced: dict = {}
        for event_name in sorted(events):
            assets = events[event_name]
            if not assets:
                continue
            try:
                by_media = event_voice_types(event_name, objects)
            except KeyError:
                problems.append("%s/%s: event not in %s" % (class_name, event_name, package_filter))
                continue
            if not by_media:
                continue
            by_type: dict[str, list[str]] = {}
            for asset in assets:
                match = MEDIA_SUFFIX.search(asset)
                types = by_media.get(int(match.group(1))) if match else None
                if not types:
                    problems.append("%s/%s: %s is not under the voice switch" % (class_name, event_name, asset))
                    continue
                for voice in sorted(types):
                    by_type.setdefault(voice, []).append(asset)
            if by_type:
                voiced[event_name] = dict(sorted(by_type.items()))
        if not voiced:
            continue
        selected = previous.get("classes", {}).get(class_name, {}).get("selectedVoiceType", DEFAULT_VOICE_TYPE)
        classes[class_name] = {"selectedVoiceType": selected, "events": voiced}
    return {"formatVersion": FORMAT_VERSION, "classes": classes}, problems


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args(argv)

    catalog = json.loads(SOUND_CATALOG.read_text(encoding="utf-8"))
    previous = json.loads(VOICE_TYPES.read_text(encoding="utf-8")) if VOICE_TYPES.is_file() else {}
    document, problems = build(catalog, previous)

    for class_name, row in document["classes"].items():
        counts: dict[str, int] = {}
        for by_type in row["events"].values():
            for voice, assets in by_type.items():
                counts[voice] = counts.get(voice, 0) + len(assets)
        print("%-16s %3d voiced events, selected %s, files per type %s"
              % (class_name, len(row["events"]), row["selectedVoiceType"], counts))
    for line in problems:
        print("  ! " + line)
    if problems:
        return 1
    if not args.dry_run:
        VOICE_TYPES.write_text(json.dumps(document, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        print("wrote %s" % VOICE_TYPES)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
