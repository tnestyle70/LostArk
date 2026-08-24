#!/usr/bin/env python3
"""Scan every class .animevents for SOUND rows, resolve each payload event name
to real wav files under the raw extracted sound library, copy the matched files
into the Client runtime Resources folder, and emit Data/Sound/CharacterSoundCatalog.json.

No network/DB use. Pure filesystem read + deterministic copy + JSON write.
"""
from __future__ import annotations

import argparse
import json
import re
import shutil
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
ANIMEVENTS_DIR = REPO_ROOT / "Data" / "Animation" / "Authored"
CATALOG_PATH = REPO_ROOT / "Data" / "Sound" / "CharacterSoundCatalog.json"
RESOURCES_SOUND_ROOT = REPO_ROOT / "Client" / "Bin" / "Resources" / "Sound"

# class folder name (as it appears in Data/Animation/Authored/<Class>) -- also
# the source wav folder name under the raw extracted library's Character/
# subfolder, and the Resources deploy subfolder name (all three match).
CLASSES = [
    "Artist",
    "DimensionMaster",
    "GunSlinger",
    "LanceMaster",
    "Slayer",
    "Warlord",
]

SOUND_ROW_RE = re.compile(
    r'^"(?P<clip>[^"]+)"\s+SOUND\s+(?P<fields>.*?)\s*src=\S+\s*$'
)
PAYLOAD_RE = re.compile(r'payload="([^"]*)"')


def parse_sound_events(animevents_path: Path) -> list[str]:
    """Returns the ordered (possibly duplicated) list of payload strings for
    every SOUND row in one .animevents file. Duplicates are expected (the same
    cue can be authored on more than one clip) and collapsed by the caller."""
    if not animevents_path.is_file():
        return []
    payloads: list[str] = []
    text = animevents_path.read_text(encoding="utf-8")
    for line in text.splitlines():
        line = line.strip()
        if not line or " SOUND " not in line:
            continue
        match = SOUND_ROW_RE.match(line)
        if not match:
            continue
        payload_match = PAYLOAD_RE.search(match.group("fields"))
        if not payload_match or not payload_match.group(1):
            continue
        payloads.append(payload_match.group(1))
    return payloads


def split_bank_event(payload: str) -> tuple[str, str]:
    """'PC_COMMON_DUAL.PC_Common_Dual1_1' -> ('PC_COMMON_DUAL', 'PC_Common_Dual1_1')."""
    bank, _, event = payload.partition(".")
    return bank, event if event else bank


def resolve_source_folder(bank: str, class_name: str, raw_sound_root: Path) -> Path:
    """PC_COMMON_* payloads live under the shared Common folder; everything
    else is looked up under the owning class's own folder."""
    if bank.upper().startswith("PC_COMMON"):
        return raw_sound_root / "Sound" / "Character" / "Common"
    return raw_sound_root / "Sound" / "Character" / class_name


def find_variant_files(source_folder: Path, event_name: str) -> list[Path]:
    if not source_folder.is_dir():
        return []
    prefix = (event_name + "__").lower()
    matches = [
        entry for entry in source_folder.iterdir()
        if entry.is_file() and entry.name.lower().startswith(prefix)
    ]
    matches.sort(key=lambda p: p.name)
    return matches


def build_catalog(raw_sound_root: Path, dry_run: bool) -> tuple[dict, list[str]]:
    catalog: dict[str, dict[str, list[str]]] = {}
    unmatched: list[str] = []

    common_bucket: dict[str, list[str]] = {}
    catalog["Common"] = common_bucket

    for class_name in CLASSES:
        animevents_path = (
            ANIMEVENTS_DIR / class_name / f"{class_name}.animevents"
        )
        payloads = parse_sound_events(animevents_path)
        class_bucket: dict[str, list[str]] = {}

        seen_events: set[str] = set()
        for payload in payloads:
            bank, event_name = split_bank_event(payload)
            if event_name in seen_events:
                continue
            seen_events.add(event_name)

            source_folder = resolve_source_folder(bank, class_name, raw_sound_root)
            variants = find_variant_files(source_folder, event_name)
            is_common = bank.upper().startswith("PC_COMMON")
            bucket = common_bucket if is_common else class_bucket
            deploy_subfolder = "Common" if is_common else class_name

            if not variants:
                bucket.setdefault(event_name, [])
                unmatched.append(f"{class_name}: {payload}")
                continue

            relative_paths: list[str] = []
            for variant in variants:
                dest_dir = RESOURCES_SOUND_ROOT / "Character" / deploy_subfolder
                dest_path = dest_dir / variant.name
                relative_paths.append(
                    f"Sound/Character/{deploy_subfolder}/{variant.name}"
                )
                if dry_run:
                    continue
                dest_dir.mkdir(parents=True, exist_ok=True)
                if not dest_path.exists() or dest_path.stat().st_size != variant.stat().st_size:
                    shutil.copyfile(variant, dest_path)

            bucket[event_name] = relative_paths

        catalog[class_name] = class_bucket

    return {"formatVersion": 1, "classes": catalog}, unmatched


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--raw-sound-root",
        type=Path,
        default=Path(r"D:\로아 리소스"),
        help="Root of the extracted raw sound library (contains a Sound/ subfolder).",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Resolve and report matches without copying files or writing the catalog.",
    )
    args = parser.parse_args()

    catalog, unmatched = build_catalog(args.raw_sound_root, args.dry_run)

    total_events = sum(
        len(bucket) for bucket in catalog["classes"].values()
    )
    matched_events = sum(
        1
        for bucket in catalog["classes"].values()
        for files in bucket.values()
        if files
    )
    print(f"Events discovered: {total_events}")
    print(f"Events matched to at least one wav: {matched_events}")
    print(f"Events unmatched: {len(unmatched)}")
    for line in unmatched[:40]:
        print(f"  unmatched: {line}")
    if len(unmatched) > 40:
        print(f"  ... and {len(unmatched) - 40} more")

    if args.dry_run:
        return 0

    CATALOG_PATH.parent.mkdir(parents=True, exist_ok=True)
    CATALOG_PATH.write_text(
        json.dumps(catalog, ensure_ascii=False, indent=1) + "\n",
        encoding="utf-8",
    )
    print(f"Wrote {CATALOG_PATH}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
