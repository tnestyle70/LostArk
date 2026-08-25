#!/usr/bin/env python3
"""Resolve every soundEvent referenced by Valtan.patternsoundcues.json to real
wav files under the raw extracted sound library's Boss/Valtan/Voltan{1,2,3}
folders, copy the matched files into the Client runtime Resources folder, and
merge a "Valtan" entry into the existing Data/Sound/CharacterSoundCatalog.json
(every other class's entry is left untouched).

Kept separate from build_sound_catalog.py: the raw source layout is different
in kind, not just in name -- player classes live at a single
Sound/Character/<Class>/ folder, Valtan's raw voice/impact wavs are split
across three Sound/Boss/Valtan/Voltan{1,2,3}/ folders instead. Event names
already embed which of the three they came from (e.g. G_Voltan2_Attack19), so
this script searches all three subfolders per event instead of trying to
derive the right one from the payload bank string -- simpler and matches the
data as it actually sits on disk.

No network/DB use. Pure filesystem read + deterministic copy + JSON merge.
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
CUES_PATH = (
    REPO_ROOT / "Data" / "Animation" / "Authored" / "Valtan" / "Valtan.patternsoundcues.json"
)
CATALOG_PATH = REPO_ROOT / "Data" / "Sound" / "CharacterSoundCatalog.json"
RESOURCES_SOUND_ROOT = REPO_ROOT / "Client" / "Bin" / "Resources" / "Sound"

VOLTAN_SUBFOLDERS = ["Voltan1", "Voltan2", "Voltan3"]
DEPLOY_CLASS_NAME = "Valtan"


def collect_event_names(cues_path: Path) -> list[str]:
    document = json.loads(cues_path.read_text(encoding="utf-8"))
    seen: set[str] = set()
    ordered: list[str] = []
    for cue in document.get("cues", []):
        event = cue.get("soundEvent")
        if event and event not in seen:
            seen.add(event)
            ordered.append(event)
    return ordered


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


def build_valtan_bucket(
    event_names: list[str], valtan_root: Path, dry_run: bool
) -> tuple[dict[str, list[str]], list[str]]:
    bucket: dict[str, list[str]] = {}
    unmatched: list[str] = []
    dest_dir = RESOURCES_SOUND_ROOT / DEPLOY_CLASS_NAME

    for event_name in event_names:
        variants: list[Path] = []
        for subfolder in VOLTAN_SUBFOLDERS:
            source_folder = valtan_root / subfolder
            variants.extend(find_variant_files(source_folder, event_name))
        variants.sort(key=lambda p: p.name)

        if not variants:
            bucket[event_name] = []
            unmatched.append(event_name)
            continue

        relative_paths: list[str] = []
        for variant in variants:
            dest_path = dest_dir / variant.name
            relative_paths.append(f"Sound/{DEPLOY_CLASS_NAME}/{variant.name}")
            if dry_run:
                continue
            dest_dir.mkdir(parents=True, exist_ok=True)
            if not dest_path.exists() or dest_path.stat().st_size != variant.stat().st_size:
                import shutil
                shutil.copyfile(variant, dest_path)

        bucket[event_name] = relative_paths

    return bucket, unmatched


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--raw-sound-root",
        type=Path,
        default=Path(r"D:\로아 리소스"),
        help="Root of the extracted raw sound library (contains a Sound/ subfolder). "
             "Ignored if --valtan-root is given.",
    )
    parser.add_argument(
        "--valtan-root",
        type=Path,
        default=None,
        help="Folder directly containing Voltan1/Voltan2/Voltan3 subfolders. "
             "Overrides --raw-sound-root -- use this to point at a flat local copy "
             "of just those folders when the raw library's own drive isn't reachable.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Resolve and report matches without copying files or writing the catalog.",
    )
    args = parser.parse_args()

    valtan_root = args.valtan_root or (args.raw_sound_root / "Sound" / "Boss" / "Valtan")

    event_names = collect_event_names(CUES_PATH)
    bucket, unmatched = build_valtan_bucket(event_names, valtan_root, args.dry_run)

    matched_count = sum(1 for files in bucket.values() if files)
    print(f"Events referenced by Valtan.patternsoundcues.json: {len(event_names)}")
    print(f"Events matched to at least one wav: {matched_count}")
    print(f"Events unmatched: {len(unmatched)}")
    for name in unmatched[:40]:
        print(f"  unmatched: {name}")
    if len(unmatched) > 40:
        print(f"  ... and {len(unmatched) - 40} more")

    if args.dry_run:
        return 0

    catalog = json.loads(CATALOG_PATH.read_text(encoding="utf-8"))
    catalog.setdefault("classes", {})[DEPLOY_CLASS_NAME] = bucket
    CATALOG_PATH.write_text(
        json.dumps(catalog, ensure_ascii=False, indent=1) + "\n",
        encoding="utf-8",
    )
    print(f"Merged \"{DEPLOY_CLASS_NAME}\" into {CATALOG_PATH}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
