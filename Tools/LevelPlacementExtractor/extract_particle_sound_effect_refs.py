#!/usr/bin/env python3
"""Extract typed effect object references from ParticleSoundNew LOA documents."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
from collections import Counter, defaultdict
from pathlib import Path


TYPED_REFERENCE_PATTERN = re.compile(
    rb"(?P<class>[A-Za-z][A-Za-z0-9_]*)'(?P<asset>[^'\x00\r\n]+)'"
)


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def source_name(path: Path) -> str:
    name = path.stem
    marker = "BaseBuff_Valtan_"
    offset = name.find(marker)
    return name[offset + len(marker):] if offset >= 0 else name


def extract_document(paths: list[Path], profile_id: str) -> dict:
    aggregate: dict[str, dict[str, object]] = {}
    source_files = []
    class_counts: Counter[str] = Counter()
    occurrence_count = 0

    for path in sorted(paths, key=lambda value: value.name.casefold()):
        source = source_name(path)
        references = []
        for match in TYPED_REFERENCE_PATTERN.finditer(path.read_bytes()):
            class_name = match.group("class").decode("ascii")
            asset = match.group("asset").decode("utf-8", errors="strict")
            references.append({"className": class_name, "sourceAsset": asset})
            class_counts[class_name] += 1
            occurrence_count += 1
            key = f"{class_name.casefold()}:{asset.casefold()}"
            row = aggregate.setdefault(
                key,
                {
                    "className": class_name,
                    "sourceAsset": asset,
                    "sourceNames": set(),
                    "occurrenceCount": 0,
                },
            )
            row["sourceNames"].add(source)
            row["occurrenceCount"] += 1

        source_files.append(
            {
                "sourceName": source,
                "path": path.as_posix(),
                "sha256": sha256_file(path),
                "typedReferences": references,
            }
        )

    def serialize(class_name: str) -> list[dict]:
        rows = []
        for row in aggregate.values():
            if str(row["className"]).casefold() != class_name.casefold():
                continue
            rows.append(
                {
                    "sourceAsset": row["sourceAsset"],
                    "actionIds": [],
                    "actionNames": sorted(row["sourceNames"], key=str.casefold),
                    "clipNames": [],
                    "occurrenceCount": row["occurrenceCount"],
                }
            )
        return sorted(rows, key=lambda row: row["sourceAsset"].casefold())

    particle_systems = serialize("ParticleSystem")
    materials = serialize("Material")
    audio_events = serialize("AkEvent")
    return {
        "schema": "lostark.particle-sound-effect-source",
        "formatVersion": 1,
        "profileId": profile_id,
        "sourceFiles": source_files,
        "particleSystems": particle_systems,
        "materials": materials,
        "audioEvents": audio_events,
        "summary": {
            "sourceFileCount": len(source_files),
            "typedReferenceOccurrenceCount": occurrence_count,
            "uniqueParticleSystemCount": len(particle_systems),
            "particleSystemOccurrenceCount": class_counts["ParticleSystem"],
            "uniqueMaterialCount": len(materials),
            "materialOccurrenceCount": class_counts["Material"],
            "uniqueAudioEventCount": len(audio_events),
            "audioEventOccurrenceCount": class_counts["AkEvent"],
            "classOccurrenceCounts": dict(sorted(class_counts.items())),
        },
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", required=True, type=Path)
    parser.add_argument("--pattern", required=True)
    parser.add_argument("--profile-id", required=True)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    paths = sorted(args.root.glob(args.pattern))
    if not paths:
        raise ValueError(f"no source documents matched {args.root / args.pattern}")
    document = extract_document(paths, args.profile_id)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(document, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    print(json.dumps(document["summary"], ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
