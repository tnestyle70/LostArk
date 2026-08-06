#!/usr/bin/env python3
"""Build a deterministic per-skill inventory from Animation Tool source data.

This is the extraction gate before decoding UE3 particle graphs.  It records
which bound animation clip owns each source notify and deliberately keeps the
combined skinned character mesh under Animation/CModel ownership.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from collections import Counter
from pathlib import Path
from typing import Any

from build_skill_effect_source_receipt import build_timeline, parse_animnotify


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def source_package(source_asset: str) -> str | None:
    package, separator, _ = source_asset.partition(".")
    return package if separator and package else None


def event_resolution(event: dict[str, Any]) -> str:
    if event["kind"] == "SHAKE":
        return "OUTSIDE_EFFECT_DOCUMENT"
    if event["sourceType"] == "PlayParticleEffect" and event["sourceAsset"]:
        return "PARTICLE_GRAPH_REQUIRED"
    return "UNSUPPORTED_SOURCE_NOTIFY"


def build_inventory(bindings_path: Path, animnotify_path: Path) -> dict[str, Any]:
    bindings = json.loads(bindings_path.read_text(encoding="utf-8-sig"))
    clip_catalog = parse_animnotify(animnotify_path)
    rows = bindings.get("bindings")
    if not isinstance(rows, list) or not rows:
        raise ValueError("bindings must be a non-empty array")

    seen_skills: set[int] = set()
    skills: list[dict[str, Any]] = []
    all_particle_assets: set[str] = set()
    all_packages: set[str] = set()
    total_events = 0
    status_counts: Counter[str] = Counter()

    for binding in rows:
        skill_id = int(binding.get("skillId", -1))
        if skill_id < 0 or skill_id in seen_skills:
            raise ValueError(f"invalid or duplicate skillId: {skill_id}")
        seen_skills.add(skill_id)
        clip_names = binding.get("clips")
        if not isinstance(clip_names, list) or not clip_names or not all(
            isinstance(item, str) and item for item in clip_names
        ):
            raise ValueError(f"skill {skill_id} has an invalid clips array")

        clips, source_events, duration = build_timeline(
            clip_names, clip_catalog, skill_id
        )
        events: list[dict[str, Any]] = []
        particle_assets: set[str] = set()
        packages: set[str] = set()
        per_skill_status: Counter[str] = Counter()
        for event in source_events:
            status = event_resolution(event)
            package = source_package(event["sourceAsset"])
            row = {**event, "extractionStatus": status}
            if package is not None:
                row["logicalPackage"] = package
            events.append(row)
            per_skill_status[status] += 1
            status_counts[status] += 1
            if status == "PARTICLE_GRAPH_REQUIRED":
                particle_assets.add(event["sourceAsset"])
                all_particle_assets.add(event["sourceAsset"])
                if package is not None:
                    packages.add(package)
                    all_packages.add(package)

        total_events += len(events)
        skills.append(
            {
                "skillId": skill_id,
                "durationSeconds": duration,
                "clips": clips,
                "sourceEvents": events,
                "particleSystems": sorted(particle_assets, key=str.casefold),
                "logicalPackages": sorted(packages, key=str.casefold),
                "summary": {
                    "clipCount": len(clips),
                    "sourceEventCount": len(events),
                    "uniqueParticleSystemCount": len(particle_assets),
                    "logicalPackageCount": len(packages),
                    "statusCounts": dict(sorted(per_skill_status.items())),
                },
            }
        )

    return {
        "schema": "lostark.class-skill-effect-source-inventory",
        "formatVersion": 1,
        "animationAssetId": bindings.get("animationAssetId"),
        "characterClass": bindings.get("characterClass"),
        "source": {
            "skillBindings": bindings_path.as_posix(),
            "skillBindingsSha256": sha256_file(bindings_path),
            "animationNotify": animnotify_path.as_posix(),
            "animationNotifySha256": sha256_file(animnotify_path),
        },
        "ownership": {
            "combinedSkinnedMeshAndAnimation": "Animation/CModel",
            "effectOverlay": "Effect Document after graph conversion",
            "reconstructCombinedMeshAsEffectElement": False,
        },
        "skills": skills,
        "particleSystems": sorted(all_particle_assets, key=str.casefold),
        "logicalPackages": sorted(all_packages, key=str.casefold),
        "summary": {
            "skillCount": len(skills),
            "clipCount": sum(len(row["clips"]) for row in skills),
            "sourceEventCount": total_events,
            "uniqueParticleSystemCount": len(all_particle_assets),
            "logicalPackageCount": len(all_packages),
            "statusCounts": dict(sorted(status_counts.items())),
        },
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--bindings", required=True, type=Path)
    parser.add_argument("--animnotify", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    document = build_inventory(args.bindings, args.animnotify)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(document, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps(document["summary"], ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
