#!/usr/bin/env python3
"""Publish every vehicle sound as the "Vehicle" class of
Data/Sound/CharacterSoundCatalog.json.

Two kinds of cue end up there.

Skill cues already live in Data/Actors/VehicleCatalog.json as
``skills[].soundCues`` and name a Wwise event such as
"S_Vehicle_TrisionHorse_Dash1".

Locomotion cues are read here from the UE3 Action evidence and written back
into the catalog as ``locomotionSoundCues``. The source MOVE action carries an
``AkEventSwitchFloorMaterial`` notify at each moment a foot reaches the ground
during the run clip, plus -- on the same clip -- the plain footstep foley
event. Those notify times are clip-local and the catalog's ``vehicleRunClip``
is that same clip, so the runtime can fire them straight off the clip clock.
Vehicles whose run clip has no floor-material notify (boards, the tube, the
swing) have a looping run sound instead of per-foot contacts and are left out.

Every event is resolved through Tools/SoundPipeline: its Play actions are
followed to the .wem media, the media is extracted, and vgmstream converts it.
The original switches footsteps on the floor material under the vehicle; there
is no such surface query on our side, so one branch is baked in (--floor,
default stone).

Usage:
  python build_vehicle_sound_catalog.py [--dry-run] [--floor stone] [--vgmstream PATH]
"""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT / "Tools" / "SoundPipeline"))
sys.path.insert(0, str(Path(__file__).resolve().parent))
from wwise_audio_package import (  # noqa: E402
    DEFAULT_PACKAGE_ROOT,
    AudioPackage,
    find_packages,
    merged_objects,
    resolve_event,
)
from build_vehicle_skills import write_catalog  # noqa: E402

VEHICLE_CATALOG = REPO_ROOT / "Data" / "Actors" / "VehicleCatalog.json"
SOUND_CATALOG = REPO_ROOT / "Data" / "Sound" / "CharacterSoundCatalog.json"
RESOURCE_DIR = REPO_ROOT / "Client" / "Bin" / "Resources" / "Sound" / "Vehicle"
ACTION_EVIDENCE = REPO_ROOT / "out" / "VehicleLocomotionSound20260916" / "actions"
ASSET_PREFIX = "Sound/Vehicle"
CLASS_NAME = "Vehicle"
PACKAGE_FILTER = "SOUND_VEHICLE"
CATALOG_FORMAT_VERSION = 4

MOVE_ACTION_ID = 1
FLOOR_NOTIFY = "AkEventSwitchFloorMaterial"
EVENT_NOTIFY = "AKEvent"

VGMSTREAM_CANDIDATES = [
    Path("C:/Users/95jus/Downloads/DSAS_4.9.9_HOTFIX_b/Res/vgmstream/vgmstream_cmd.exe"),
]


def find_vgmstream(explicit: Path | None) -> Path:
    if explicit is not None:
        if not explicit.is_file():
            raise SystemExit(f"vgmstream not found: {explicit}")
        return explicit
    for candidate in VGMSTREAM_CANDIDATES:
        if candidate.is_file():
            return candidate
    raise SystemExit("vgmstream_cmd.exe not found; pass --vgmstream")


def profile_id(vehicle: dict) -> str:
    """Character/Vehicle/<Profile>/<Profile>.wmodel -> the action evidence name."""
    return vehicle["modelAssetId"].split("/")[2]


def event_name(notify: dict) -> str | None:
    """'S_Vehicle4.S_Vehicle_AncientDragon1_FootStep1' -> the event name."""
    for reference in notify.get("assetReferences", []):
        path = reference.get("objectPath") or reference.get("reference") or ""
        if "." in path:
            return path.rsplit(".", 1)[1]
    return None


def locomotion_cues(evidence: Path, vehicle: dict) -> list[dict]:
    """Ground-contact cues on the vehicle's run clip, in time order.

    A run clip with no floor-material notify has no per-foot contact to play,
    so it produces nothing rather than a guess.
    """
    path = evidence / (profile_id(vehicle) + ".action-effects.json")
    if not path.is_file():
        raise SystemExit(f"missing action evidence: {path}")
    document = json.loads(path.read_text(encoding="utf-8"))
    run_clip = vehicle["vehicleRunClip"]
    for action in document["actions"]:
        if action["actionId"] != MOVE_ACTION_ID:
            continue
        for stage in action["stages"]:
            clips = [c.get("clipName") or c.get("name") or "" for c in stage.get("animationClips", [])]
            if not any(run_clip.endswith(c.lower()) for c in clips if c):
                continue
            contacts = [n for n in stage["notifies"] if n["sourceType"] == FLOOR_NOTIFY]
            if not contacts:
                return []
            foley = [
                n
                for n in stage["notifies"]
                if n["sourceType"] == EVENT_NOTIFY
                and "footstep" in (event_name(n) or "").lower()
            ]
            cues = []
            for notify in contacts + foley:
                name = event_name(notify)
                if name is None:
                    continue
                cues.append(
                    {
                        "clip": run_clip,
                        "startMs": int(round(notify["localTimeSeconds"] * 1000.0)),
                        "event": name,
                    }
                )
            cues.sort(key=lambda c: (c["startMs"], c["event"]))
            return cues
    return []


def catalog_events(catalog: dict) -> list[tuple[str, bool]]:
    """(event name, is a floor-material footstep), in catalog order."""
    events: list[tuple[str, bool]] = []
    seen: set[str] = set()

    def add(name: str, footstep: bool):
        if name not in seen:
            seen.add(name)
            events.append((name, footstep))

    for vehicle in catalog["vehicles"]:
        for skill in vehicle.get("skills", []):
            for cue in skill.get("soundCues", []):
                add(cue["event"], False)
        for cue in vehicle.get("locomotionSoundCues", []):
            add(cue["event"], True)
    return events


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--package-root", type=Path, default=DEFAULT_PACKAGE_ROOT)
    parser.add_argument("--action-evidence", type=Path, default=ACTION_EVIDENCE)
    parser.add_argument("--floor", default="stone", help="floor material branch to bake in")
    parser.add_argument("--vgmstream", type=Path, default=None)
    parser.add_argument("--dry-run", action="store_true", help="resolve and report only")
    args = parser.parse_args(argv)

    catalog = json.loads(VEHICLE_CATALOG.read_text(encoding="utf-8"))
    for vehicle in catalog["vehicles"]:
        cues = locomotion_cues(args.action_evidence, vehicle)
        print("%-30s locomotion cues %d" % (vehicle["archetypeId"], len(cues)))
        for cue in cues:
            print("    %6d ms  %s" % (cue["startMs"], cue["event"]))
        if cues:
            vehicle["locomotionSoundCues"] = cues
        else:
            vehicle.pop("locomotionSoundCues", None)
    catalog["formatVersion"] = CATALOG_FORMAT_VERSION

    events = catalog_events(catalog)
    print("%d distinct events (%d footstep)" % (events and len(events), sum(1 for _, f in events if f)))

    paths = find_packages(args.package_root, PACKAGE_FILTER)
    if not paths:
        raise SystemExit(f"no {PACKAGE_FILTER}* package under {args.package_root}")
    packages = [AudioPackage(p) for p in paths]
    print("packages: " + ", ".join(p.name for p in packages))
    objects = merged_objects(packages)

    vgmstream = None if args.dry_run else find_vgmstream(args.vgmstream)
    if not args.dry_run:
        RESOURCE_DIR.mkdir(parents=True, exist_ok=True)

    entries: dict[str, list[str]] = {}
    empty: list[str] = []
    gaps: list[str] = []
    converted = 0
    with tempfile.TemporaryDirectory() as scratch:
        for name, footstep in events:
            try:
                sources, unresolved = resolve_event(
                    name, objects, args.floor if footstep else None
                )
            except KeyError:
                empty.append(name + " (event not in any SOUND_VEHICLE* bank)")
                continue
            if unresolved:
                gaps.append(
                    "%s -> %d Play target(s) absent from every shipped bank"
                    % (name, len(unresolved))
                )
            assets: list[str] = []
            for source in sources:
                payload = None
                for package in packages:
                    entry = package.stream_by_id(source)
                    if entry is not None:
                        payload = package.payload(entry)
                        break
                if payload is None:
                    gaps.append("%s -> media %d not in any package" % (name, source))
                    continue
                stem = "%s__%d" % (name, source)
                if not args.dry_run:
                    wem = Path(scratch) / (stem + ".wem")
                    wem.write_bytes(payload)
                    wav = RESOURCE_DIR / (stem + ".wav")
                    result = subprocess.run(
                        [str(vgmstream), "-o", str(wav), str(wem)], capture_output=True
                    )
                    if result.returncode != 0 or not wav.is_file():
                        gaps.append("%s -> vgmstream failed on media %d" % (name, source))
                        continue
                    converted += 1
                assets.append("%s/%s.wav" % (ASSET_PREFIX, stem))
            if assets:
                entries[name] = assets
            else:
                empty.append(name)

    for name, footstep in events:
        print("  %-42s %s %d" % (name, "foot" if footstep else "    ", len(entries.get(name, []))))
    print("resolved %d/%d events, %d wav files" % (len(entries), len(events), converted))
    if empty:
        print("no audio (runtime plays nothing for these):")
        for line in empty:
            print("  " + line)
    if gaps:
        print("gaps:")
        for line in gaps:
            print("  " + line)

    if args.dry_run:
        return 0

    write_catalog(catalog)
    print("wrote %s formatVersion %d" % (VEHICLE_CATALOG, CATALOG_FORMAT_VERSION))
    sound_catalog = json.loads(SOUND_CATALOG.read_text(encoding="utf-8"))
    sound_catalog["classes"][CLASS_NAME] = entries
    SOUND_CATALOG.write_text(
        json.dumps(sound_catalog, ensure_ascii=False, indent=1) + "\n", encoding="utf-8"
    )
    print("wrote %s class %s" % (SOUND_CATALOG, CLASS_NAME))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
