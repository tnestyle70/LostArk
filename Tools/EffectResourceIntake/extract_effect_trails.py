"""Extract weapon-trail invocation records from a LostArk ANIMNOTIFY_TRAILS package.

Each record ties a skill key to one ParticleSystem, the window it plays in, and
the baked edge/control point samples recorded at roughly 1/60 s.  This is the
only exact effect timing currently recoverable from the packages, so the raw
values are kept alongside the client-space conversion rather than replaced.

Usage:
    python extract_effect_trails.py <package.upk> --out <file.json>
"""
from __future__ import annotations

import argparse
import datetime
import json
import re
import statistics
import sys
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).resolve().parent))

import extract_effect_recipes as recipes  # noqa: E402

SCHEMA_VERSION = 1
TRAIL_CLASSES = ("animnotify_trails", "efanimnotify_trails")
KEY_CLASS = "efdata_animnotify_trails"

# UE3 centimetres (X, Y, Z) -> client metres (X, Z, -Y).  Same rule the map
# placement pipeline uses; see .md/GB/07-30 map reconstruction notes.
UE3_TO_CLIENT_SCALE = 0.01


def to_client(v: Any) -> dict[str, float] | None:
    if not isinstance(v, dict) or "x" not in v:
        return None
    return {
        "x": round(v["x"] * UE3_TO_CLIENT_SCALE, 6),
        "y": round(v["z"] * UE3_TO_CLIENT_SCALE, 6),
        "z": round(-v["y"] * UE3_TO_CLIENT_SCALE, 6),
    }


def parse_skill_key(key: str) -> dict[str, Any]:
    """`lancemaster_25010_0_1_0` -> class, skill id, variant fields."""
    match = re.match(r"^([a-z]+)_(\d+)((?:_\d+)*)$", key or "", re.IGNORECASE)
    if match is None:
        return {"raw": key}
    variant = [int(part) for part in match.group(3).split("_") if part]
    return {
        "raw": key,
        "class": match.group(1).lower(),
        "skill_id": int(match.group(2)),
        "variant": variant,
    }


def sample_of(entry: dict[str, Any]) -> dict[str, Any] | None:
    if not isinstance(entry, dict) or "relativetime" not in entry:
        return None
    out: dict[str, Any] = {"relative_time": round(entry["relativetime"], 6)}
    for source, target in (
        ("firstedgesample", "first_edge"),
        ("controlpointsample", "control_point"),
        ("secondedgesample", "second_edge"),
    ):
        raw = entry.get(source)
        if isinstance(raw, dict) and "x" in raw:
            out[target] = {k: round(raw[k], 6) for k in ("x", "y", "z")}
            out[target + "_client"] = to_client(raw)
    return out


def read_trail(reader: recipes.PackageReader, entry) -> dict[str, Any]:
    values = recipes.simplify(reader, reader.properties_of(entry))
    outer = reader.ref_path(entry.package_index) if entry.package_index else None
    template = values.get("pstemplate")
    start = values.get("laststarttime")
    end = values.get("endtime")

    samples = [
        sample for raw in (values.get("trailsampleddata") or [])
        if (sample := sample_of(raw)) is not None
    ]
    intervals = [
        round(b["relative_time"] - a["relative_time"], 6)
        for a, b in zip(samples, samples[1:])
    ]

    record: dict[str, Any] = {
        "skill_key": parse_skill_key(outer or entry.object_name),
        "class": reader.class_of(entry),
        "export_index": entry.index,
        "particle_system": template.get("path") if isinstance(template, dict) else None,
        "start_seconds": start,
        "end_seconds": end,
        "duration_seconds": (
            round(end - start, 6)
            if isinstance(start, float) and isinstance(end, float)
            else None
        ),
        "sample_count": len(samples),
        "samples": samples,
    }
    if intervals:
        record["sample_interval_seconds"] = round(statistics.median(intervals), 6)
    return record


def extract(path: Path) -> dict[str, Any]:
    reader = recipes.PackageReader(path)

    # The key objects carry the skill name and point at the trail that holds the
    # payload; keep the mapping so a record can be traced back to both.
    key_links: dict[int, str] = {}
    for entry in reader.exports:
        if reader.class_of(entry) != KEY_CLASS or entry.serial_size <= 0:
            continue
        values = recipes.simplify(reader, reader.properties_of(entry))
        for value in values.values():
            if isinstance(value, dict) and isinstance(value.get("ref"), int):
                target = reader.export_of(value["ref"])
                if target is not None:
                    key_links[target.index] = entry.object_name

    records = []
    for entry in reader.exports:
        if reader.class_of(entry) not in TRAIL_CLASSES or entry.serial_size <= 0:
            continue
        record = read_trail(reader, entry)
        if entry.index in key_links:
            record["key_object"] = key_links[entry.index]
        records.append(record)

    records.sort(key=lambda r: (r["skill_key"].get("skill_id", 0), r["skill_key"]["raw"]))
    by_skill: dict[str, int] = {}
    for record in records:
        skill = str(record["skill_key"].get("skill_id", "?"))
        by_skill[skill] = by_skill.get(skill, 0) + 1

    return {
        "schema_version": SCHEMA_VERSION,
        "generated_at": datetime.datetime.now().astimezone().isoformat(),
        "provenance": "game_original",
        "coordinate_note": (
            "*_client fields are UE3 cm (X,Y,Z) converted to client m (X,Z,-Y). "
            "Raw fields are the untouched package values."
        ),
        "source": {
            "file": path.name,
            "sha256": reader.sha256,
            "package_version": reader.summary.version,
        },
        "stats": {
            "records": len(records),
            "distinct_skill_ids": len(by_skill),
            "records_per_skill_id": dict(sorted(by_skill.items())),
            "total_samples": sum(r["sample_count"] for r in records),
            "particle_systems": sorted({
                r["particle_system"] for r in records if r["particle_system"]
            }),
        },
        "parse_errors": reader.errors,
        "records": records,
    }


def parse_args(argv=None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("package", type=Path)
    parser.add_argument("--out", type=Path, required=True)
    return parser.parse_args(argv)


def main(argv=None) -> int:
    args = parse_args(argv)
    document = extract(args.package)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(
        json.dumps(document, indent=1, ensure_ascii=False) + "\n", encoding="utf-8"
    )
    stats = document["stats"]
    print(
        f"records={stats['records']} skills={stats['distinct_skill_ids']} "
        f"samples={stats['total_samples']} "
        f"particles={len(stats['particle_systems'])} "
        f"errors={len(document['parse_errors'])} -> {args.out}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
