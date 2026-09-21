#!/usr/bin/env python3
"""Publish one class's .animevents SOUND events from the shipped Wwise packages.

build_sound_catalog.py matched events against a raw wav dump that only covered
the first classes. A class added later has no dump, so this resolves each
SOUND payload straight out of the game's packages (Tools/SoundPipeline), decodes
the media through the project's own FMOD, and deploys the wavs under
Client/Bin/Resources/Sound/Character/<Class> with the same <event>__<mediaId>
stems the rest of the tree uses.

PC_COMMON* payloads go to the shared Common bucket: an event that already has
variants there is kept, only missing or empty ones are resolved.

  python build_character_sound_catalog.py --class GuardianKnight \
      --package-filter SOUND_PC_DRAGONKNIGHT [--dry-run]
"""
from __future__ import annotations

import argparse
import json
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent))
sys.path.insert(0, str(REPO_ROOT / "Tools" / "CharacterAnimationIntake"))
import wwise_audio_package as wwise      # noqa: E402
import wwise_vorbis_to_ogg as vorbis     # noqa: E402
from build_sound_catalog import parse_sound_events, split_bank_event  # noqa: E402

ANIMEVENTS_DIR = REPO_ROOT / "Data" / "Animation" / "Authored"
SOUND_CATALOG = REPO_ROOT / "Data" / "Sound" / "CharacterSoundCatalog.json"
RESOURCE_ROOT = REPO_ROOT / "Client" / "Bin" / "Resources" / "Sound" / "Character"
COMMON_FILTER = "SOUND_PC_COMMON"


def is_common(bank: str) -> bool:
    return bank.upper().startswith("PC_COMMON")


def load_bank(package_filter: str):
    packages = wwise.load_packages(wwise.find_packages(wwise.DEFAULT_PACKAGE_ROOT, package_filter))
    if not packages:
        raise SystemExit(f"no package matches {package_filter!r} under {wwise.DEFAULT_PACKAGE_ROOT}")
    return packages, wwise.merged_objects(packages)


def render_event(event: str, packages, objects, bucket: str, decoder, scratch: Path,
                 dry_run: bool) -> tuple[list[str], str | None]:
    try:
        sources, unresolved = wwise.resolve_event(event, objects)
    except KeyError:
        return [], "not in the selected banks"
    gap = "%d Play target(s) absent from every shipped bank" % len(unresolved) if unresolved else None
    assets: list[str] = []
    for media_id in sources:
        payload = None
        for package in packages:
            entry = package.stream_by_id(media_id)
            if entry is not None:
                payload = package.payload(entry)
                break
        if payload is None:
            gap = "media %d not in any package" % media_id
            continue
        stem = "%s__%d" % (event, media_id)
        asset = "Sound/Character/%s/%s.wav" % (bucket, stem)
        if not dry_run:
            wav = RESOURCE_ROOT / bucket / (stem + ".wav")
            if not wav.is_file():
                wem = scratch / (stem + ".wem")
                wem.write_bytes(payload)
                ogg = wem.with_suffix(".ogg")
                try:
                    vorbis.convert(wem, ogg)
                except vorbis.WemError as error:
                    gap = "media %d: %s" % (media_id, error)
                    continue
                wav.parent.mkdir(parents=True, exist_ok=True)
                decoder.to_wav(ogg, wav)
        assets.append(asset)
    return assets, gap


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--class", dest="class_name", required=True)
    parser.add_argument("--package-filter", required=True,
                        help="deobfuscated package name prefix holding the class banks")
    parser.add_argument("--dry-run", action="store_true", help="resolve and report only")
    args = parser.parse_args(argv)

    animevents = ANIMEVENTS_DIR / args.class_name / f"{args.class_name}.animevents"
    if not animevents.is_file():
        raise SystemExit(f"missing {animevents}")
    events: dict[str, list[str]] = {"Common": [], args.class_name: []}
    for payload in parse_sound_events(animevents):
        bank, event = split_bank_event(payload)
        bucket = "Common" if is_common(bank) else args.class_name
        if event not in events[bucket]:
            events[bucket].append(event)

    catalog = json.loads(SOUND_CATALOG.read_text(encoding="utf-8"))
    classes = catalog["classes"]
    common = classes.setdefault("Common", {})
    pending_common = [e for e in events["Common"] if not common.get(e)]
    print("%s: %d class events, %d common events (%d unresolved so far)"
          % (args.class_name, len(events[args.class_name]), len(events["Common"]), len(pending_common)))

    decoder = None if args.dry_run else vorbis.open_wav_decoder()
    if not args.dry_run and decoder is None:
        return 2

    class_entries: dict[str, list[str]] = {}
    empty: list[str] = []
    gaps: list[str] = []
    with tempfile.TemporaryDirectory() as scratch_dir:
        scratch = Path(scratch_dir)
        class_packages, class_objects = load_bank(args.package_filter)
        for bucket, names, target in (
            (args.class_name, events[args.class_name], class_entries),
            ("Common", pending_common, common),
        ):
            if not names:
                continue
            packages, objects = class_packages, class_objects
            if "Common" == bucket:
                # A class voice on a common event can sit in the class bank.
                common_packages, _ = load_bank(COMMON_FILTER)
                packages = common_packages + class_packages
                objects = wwise.merged_objects(packages)
            print("%s <- %s" % (bucket, ", ".join(p.name for p in packages)))
            for event in names:
                assets, gap = render_event(event, packages, objects, bucket, decoder, scratch, args.dry_run)
                if gap:
                    gaps.append("%s/%s -> %s" % (bucket, event, gap))
                if assets:
                    target[event] = assets
                else:
                    target.setdefault(event, [])
                    empty.append("%s/%s" % (bucket, event))
    if decoder is not None:
        decoder.close()

    resolved = sum(1 for v in class_entries.values() if v)
    print("resolved %d/%d %s events, %d wav assets"
          % (resolved, len(events[args.class_name]), args.class_name,
             sum(len(v) for v in class_entries.values())))
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

    classes[args.class_name] = class_entries
    SOUND_CATALOG.write_text(json.dumps(catalog, ensure_ascii=False, indent=1) + "\n", encoding="utf-8")
    print("wrote %s class %s" % (SOUND_CATALOG, args.class_name))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
