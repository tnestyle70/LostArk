from __future__ import annotations

"""Build the verified Bern Castle MapTool shard set.

The builder consumes the full exact StaticMesh manifests and placement sources,
then creates one BASE shard, one existing LANDSCAPE shard, and SL00 through SL10.
Every output is written and cross-validated in a temporary directory before the
flat DataFiles/Map destination is replaced.  The aggregate receipt is installed
last and acts as the commit marker for the set.
"""

import argparse
import json
import math
import os
import re
import shlex
import shutil
import uuid
from collections import Counter
from pathlib import Path, PurePosixPath
from typing import Any, Iterable, Sequence

from build_maptool_scene import (
    EDITOR_ID_MASK,
    atomic_write_text,
    compile_scene,
    convert_scale,
    imported_id,
    load_json,
    scale_flags,
    sha256,
)


AREA_ID = "LV_BER_BERNCASTLE"
STATIC_LEVEL_PREFIX = "LV_BER_BERNCASTLE_T_"
SL_SHARD_IDS = tuple(f"SL{index:02d}" for index in range(11))
SHARD_IDS = ("BASE", "LANDSCAPE", *SL_SHARD_IDS)
DEFAULT_HIDDEN_LEVELS = frozenset(
    (
        "LV_BER_BERNCASTLE_T_EVENT01",
        "LV_BER_BERNCASTLE_T_SCENE03E",
    )
)
EXPECTED_LEVEL_COUNTS = {
    "LV_BER_BERNCASTLE_T_EVENT01": 1127,
    "LV_BER_BERNCASTLE_T_LAND01": 2,
    "LV_BER_BERNCASTLE_T_LAND02": 13,
    "LV_BER_BERNCASTLE_T_PS": 337,
    "LV_BER_BERNCASTLE_T_SCENE03E": 45,
    "LV_BER_BERNCASTLE_T_SL00": 2965,
    "LV_BER_BERNCASTLE_T_SL01": 3756,
    "LV_BER_BERNCASTLE_T_SL02": 3622,
    "LV_BER_BERNCASTLE_T_SL03": 3403,
    "LV_BER_BERNCASTLE_T_SL04": 3740,
    "LV_BER_BERNCASTLE_T_SL05": 1057,
    "LV_BER_BERNCASTLE_T_SL06": 3719,
    "LV_BER_BERNCASTLE_T_SL07": 2054,
    "LV_BER_BERNCASTLE_T_SL08": 2935,
    "LV_BER_BERNCASTLE_T_SL09": 3365,
    "LV_BER_BERNCASTLE_T_SL10": 184,
}
SL_LEVEL_PATTERN = re.compile(r"^LV_BER_BERNCASTLE_T_(SL\d{2})$")
UINT64_MAX = (1 << 64) - 1
DEFAULT_RENDER_PROFILE_MANIFEST = (
    Path(__file__).resolve().parents[2]
    / "Data/Maps/Imported/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.renderprofiles.json"
)


class ShardBuildError(RuntimeError):
    pass


def parse_count_specs(specifications: Sequence[str] | None) -> dict[str, int]:
    if specifications is None:
        return dict(EXPECTED_LEVEL_COUNTS)
    result: dict[str, int] = {}
    for specification in specifications:
        level, separator, raw_count = specification.partition("=")
        if not separator or not level or level in result:
            raise ShardBuildError(f"invalid/duplicate level count: {specification}")
        count = int(raw_count)
        if count < 0:
            raise ShardBuildError(f"negative level count: {specification}")
        result[level] = count
    return result


def parse_hidden_levels(specifications: Sequence[str] | None) -> frozenset[str]:
    levels = DEFAULT_HIDDEN_LEVELS if specifications is None else frozenset(specifications)
    for level in levels:
        if not level.startswith(STATIC_LEVEL_PREFIX):
            raise ShardBuildError(
                f"default-hidden level is outside Bern Castle: {level!r}"
            )
    return frozenset(levels)


def placement_files(directories: Sequence[Path]) -> list[tuple[int, Path]]:
    files: list[tuple[int, Path]] = []
    seen: set[Path] = set()
    for directory_index, directory in enumerate(directories):
        if not directory.is_dir():
            raise ShardBuildError(f"placement directory is missing: {directory}")
        for path in sorted(
            directory.glob("*.placements.json"),
            key=lambda value: (value.name.casefold(), value.as_posix().casefold()),
        ):
            resolved = path.resolve()
            if resolved in seen:
                continue
            seen.add(resolved)
            files.append((directory_index, path))
    if not files:
        raise ShardBuildError("no *.placements.json inputs were found")
    return files


def shard_file_name(shard_id: str, extension: str) -> str:
    if shard_id not in SHARD_IDS or extension not in ("mapassets", "mapplacements"):
        raise ShardBuildError(f"invalid shard filename request: {shard_id}.{extension}")
    return f"{AREA_ID}_{shard_id}.{extension}"


def validate_relative_filename(value: str) -> str:
    path = PurePosixPath(value)
    if (
        not value
        or path.is_absolute()
        or len(path.parts) != 1
        or path.parts[0] in (".", "..")
        or "\\" in value
    ):
        raise ShardBuildError(f"mapset path must be a relative filename: {value!r}")
    return value


def scan_static_sources(
    asset_manifest_path: Path,
    placement_directories: Sequence[Path],
    expected_level_counts: dict[str, int],
    expect_assets: int,
    expect_placements: int,
    hidden_levels: frozenset[str],
) -> dict[str, Any]:
    manifest = load_json(asset_manifest_path)
    if manifest.get("areaId") != AREA_ID:
        raise ShardBuildError("static asset manifest areaId mismatch")
    assets = manifest.get("assets", [])
    if not isinstance(assets, list) or len(assets) != expect_assets:
        raise ShardBuildError(
            f"static asset count mismatch: {len(assets) if isinstance(assets, list) else 'invalid'}"
        )
    if manifest.get("assetCount", len(assets)) != len(assets):
        raise ShardBuildError("static asset manifest count mismatch")
    assets_by_path = {str(row["fullPath"]).casefold(): row for row in assets}
    asset_ids = {str(row["assetId"]) for row in assets}
    if len(assets_by_path) != len(assets) or len(asset_ids) != len(assets):
        raise ShardBuildError("duplicate static asset path/ID")

    groups = {
        shard_id: {
            "rows": [],
            "assetIds": set(),
            "sourceIds": set(),
            "runtimeIds": set(),
            "levels": set(),
            "anyNegative": 0,
            "reflected": 0,
        }
        for shard_id in ("BASE", *SL_SHARD_IDS)
    }
    level_counts: Counter[str] = Counter()
    all_source_ids: set[str] = set()
    all_runtime_ids: set[int] = set()
    all_referenced_assets: set[str] = set()
    sources = placement_files(placement_directories)

    for _, path in sources:
        document = load_json(path)
        if document.get("schemaVersion") != 1 or document.get("propertyErrors"):
            raise ShardBuildError(f"invalid placement source: {path}")
        unresolved = document.get("unresolvedPlacements", [])
        if not isinstance(unresolved, list) or unresolved:
            raise ShardBuildError(
                f"placement source contains unresolved owners: {path}: "
                f"{len(unresolved) if isinstance(unresolved, list) else 'invalid'}"
            )
        rows = document.get("placements", [])
        if not isinstance(rows, list):
            raise ShardBuildError(f"placements must be an array: {path}")
        for row in rows:
            transform = row.get("transform", {})
            actor = row.get("actor")
            if (
                isinstance(transform, dict)
                and transform.get("source") == "actor"
                and isinstance(actor, dict)
                and actor.get("exportIndex") is None
            ):
                raise ShardBuildError(
                    "placement source contains an unresolved actor fallback: "
                    f"{row.get('placementId', '')}"
                )
            level = str(row.get("levelPackage", ""))
            if not level.startswith(STATIC_LEVEL_PREFIX):
                continue
            match = SL_LEVEL_PATTERN.fullmatch(level)
            if match is None:
                shard_id = "BASE"
            else:
                shard_id = match.group(1)
                if shard_id not in SL_SHARD_IDS:
                    raise ShardBuildError(f"unsupported Bern streaming level: {level}")

            source_id = str(row.get("placementId", ""))
            if not source_id or source_id in all_source_ids:
                raise ShardBuildError(f"duplicate/empty static source placement ID: {source_id!r}")
            runtime_id = imported_id(source_id)
            if runtime_id in all_runtime_ids:
                raise ShardBuildError(f"static runtime placement ID collision: {runtime_id}")
            object_path = str(row.get("asset", {}).get("objectPath", "")).casefold()
            asset = assets_by_path.get(object_path)
            if asset is None:
                raise ShardBuildError(f"static placement asset join missing: {object_path}")
            scale = convert_scale(row["transform"]["scale3D"])
            any_negative, reflected = scale_flags(scale)

            group = groups[shard_id]
            group["rows"].append(row)
            group["assetIds"].add(str(asset["assetId"]))
            group["sourceIds"].add(source_id)
            group["runtimeIds"].add(runtime_id)
            group["levels"].add(level)
            group["anyNegative"] += int(any_negative)
            group["reflected"] += int(reflected)
            level_counts[level] += 1
            all_source_ids.add(source_id)
            all_runtime_ids.add(runtime_id)
            all_referenced_assets.add(str(asset["assetId"]))

    if dict(sorted(level_counts.items())) != dict(sorted(expected_level_counts.items())):
        raise ShardBuildError(f"static level counts mismatch: {dict(sorted(level_counts.items()))}")
    if sum(level_counts.values()) != expect_placements:
        raise ShardBuildError(f"static placement count mismatch: {sum(level_counts.values())}")
    if all_referenced_assets != asset_ids:
        missing = sorted(asset_ids - all_referenced_assets)
        extra = sorted(all_referenced_assets - asset_ids)
        raise ShardBuildError(
            f"static manifest/reference set mismatch: missing={missing[:10]} extra={extra[:10]}"
        )
    if "LV_BER_BERNCASTLE_T_PS" not in groups["BASE"]["levels"]:
        raise ShardBuildError("BASE shard is missing LV_BER_BERNCASTLE_T_PS")
    for shard_id in SL_SHARD_IDS:
        expected_level = f"{STATIC_LEVEL_PREFIX}{shard_id}"
        if groups[shard_id]["levels"] != {expected_level}:
            raise ShardBuildError(f"{shard_id} source level mismatch")

    source_records = [
        {
            "key": (
                path.name
                if len(placement_directories) == 1
                else f"{directory_index}/{path.name}"
            ),
            "path": path.as_posix(),
            "sha256": sha256(path),
        }
        for directory_index, path in sources
    ]
    return {
        "groups": groups,
        "assetIds": asset_ids,
        "sourceIds": all_source_ids,
        "runtimeIds": all_runtime_ids,
        "levelCounts": dict(sorted(level_counts.items())),
        "defaultHiddenPlacementCount": sum(
            1
            for group in groups.values()
            for row in group["rows"]
            if str(row.get("levelPackage", "")) in hidden_levels
        ),
        "sourceRecords": source_records,
    }


def empty_overlay_group() -> dict[str, Any]:
    return {
        "rows": [],
        "assetIds": set(),
        "exactAssetIds": set(),
        "overlayAssetIds": set(),
        "sourceIds": set(),
        "runtimeIds": set(),
        "levels": set(),
        "anyNegative": 0,
        "reflected": 0,
    }


def finite_overlay_vector(value: Any, length: int, label: str) -> tuple[float, ...]:
    if not isinstance(value, list) or len(value) != length:
        raise ShardBuildError(f"{label} must contain {length} values")
    result = tuple(float(component) for component in value)
    if not all(math.isfinite(component) for component in result):
        raise ShardBuildError(f"{label} contains a non-finite value")
    return result


def scan_overlay_source(
    overlay_manifest_path: Path | None,
    static_inventory: dict[str, Any],
    expect_assets: int | None,
    expect_placements: int | None,
    expect_any_negative: int | None,
    expect_reflected: int | None,
    hidden_levels: frozenset[str],
) -> dict[str, Any] | None:
    if overlay_manifest_path is None:
        return None
    document = load_json(overlay_manifest_path)
    if document.get("schemaVersion") != 1 or document.get("areaId") != AREA_ID:
        raise ShardBuildError("overlay manifest schema/areaId mismatch")
    assets = document.get("assets", [])
    placements = document.get("placements", [])
    if not isinstance(assets, list) or not isinstance(placements, list):
        raise ShardBuildError("overlay assets/placements must be arrays")

    assets_by_id: dict[str, dict[str, Any]] = {}
    prototype_tags: set[str] = set()
    for asset in assets:
        if not isinstance(asset, dict):
            raise ShardBuildError("overlay asset must be an object")
        asset_id = str(asset.get("assetId", ""))
        prototype_tag = str(asset.get("prototypeTag", ""))
        if not asset_id or asset_id in assets_by_id:
            raise ShardBuildError(f"duplicate/empty overlay asset ID: {asset_id!r}")
        if asset_id in static_inventory["assetIds"]:
            raise ShardBuildError(f"overlay asset redefines an exact asset ID: {asset_id}")
        if not prototype_tag or prototype_tag in prototype_tags:
            raise ShardBuildError(
                f"duplicate/empty overlay prototype tag: {prototype_tag!r}"
            )
        assets_by_id[asset_id] = asset
        prototype_tags.add(prototype_tag)

    groups = {
        shard_id: empty_overlay_group() for shard_id in ("BASE", *SL_SHARD_IDS)
    }
    source_ids: set[str] = set()
    runtime_ids: set[int] = set()
    referenced_overlay_assets: set[str] = set()
    any_negative_count = 0
    reflected_count = 0
    level_counts: Counter[str] = Counter()
    default_hidden_count = 0
    known_asset_ids = static_inventory["assetIds"] | set(assets_by_id)
    for placement in placements:
        if not isinstance(placement, dict):
            raise ShardBuildError("overlay placement must be an object")
        source_id = str(placement.get("sourcePlacementId", ""))
        if (
            not source_id
            or source_id in source_ids
            or source_id in static_inventory["sourceIds"]
        ):
            raise ShardBuildError(f"duplicate/empty overlay source ID: {source_id!r}")
        runtime_id = int(placement.get("placementId", 0))
        if not 0 < runtime_id <= EDITOR_ID_MASK:
            raise ShardBuildError(f"overlay placement ID is outside its domain: {runtime_id}")
        if runtime_id in runtime_ids or runtime_id in static_inventory["runtimeIds"]:
            raise ShardBuildError(f"overlay runtime placement ID collision: {runtime_id}")
        if str(placement.get("transformSource", "overlay")) != "overlay":
            raise ShardBuildError("overlay placement transformSource must be overlay")

        asset_id = str(placement.get("assetId", ""))
        if asset_id not in known_asset_ids:
            raise ShardBuildError(f"overlay placement asset join missing: {asset_id}")
        level = str(placement.get("sourceLevel", ""))
        if not level.startswith(STATIC_LEVEL_PREFIX):
            raise ShardBuildError(f"overlay sourceLevel is outside Bern Castle: {level!r}")
        match = SL_LEVEL_PATTERN.fullmatch(level)
        shard_id = "BASE" if match is None else match.group(1)
        if shard_id not in groups:
            raise ShardBuildError(f"unsupported overlay streaming level: {level}")

        finite_overlay_vector(placement.get("position", []), 3, "overlay position")
        quaternion = finite_overlay_vector(
            placement.get("quaternion", []), 4, "overlay quaternion"
        )
        if sum(component * component for component in quaternion) <= 1.0e-12:
            raise ShardBuildError(f"zero overlay quaternion: {source_id}")
        scale = finite_overlay_vector(placement.get("scale", []), 3, "overlay scale")
        if any(abs(component) < 1.0e-6 for component in scale):
            raise ShardBuildError(f"overlay placement has a zero scale axis: {source_id}")
        any_negative, reflected = scale_flags(scale)

        normalized_placement = dict(placement)
        if level in hidden_levels:
            normalized_placement["visible"] = False
            default_hidden_count += 1

        group = groups[shard_id]
        group["rows"].append(normalized_placement)
        group["assetIds"].add(asset_id)
        if asset_id in assets_by_id:
            group["overlayAssetIds"].add(asset_id)
            referenced_overlay_assets.add(asset_id)
        else:
            group["exactAssetIds"].add(asset_id)
        group["sourceIds"].add(source_id)
        group["runtimeIds"].add(runtime_id)
        group["levels"].add(level)
        group["anyNegative"] += int(any_negative)
        group["reflected"] += int(reflected)
        source_ids.add(source_id)
        runtime_ids.add(runtime_id)
        any_negative_count += int(any_negative)
        reflected_count += int(reflected)
        level_counts[level] += 1

    unreferenced = set(assets_by_id) - referenced_overlay_assets
    if unreferenced:
        raise ShardBuildError(
            f"overlay contains unreferenced assets: {sorted(unreferenced)[:10]}"
        )
    declared_asset_count = document.get("assetCount")
    if declared_asset_count is not None and int(declared_asset_count) != len(assets):
        raise ShardBuildError("overlay declared asset count mismatch")
    declared_placement_count = document.get("placementCount")
    if (
        declared_placement_count is not None
        and int(declared_placement_count) != len(placements)
    ):
        raise ShardBuildError("overlay declared placement count mismatch")
    summary = document.get("summary", {})
    if not isinstance(summary, dict):
        raise ShardBuildError("overlay summary must be an object")
    summary_gates = (
        ("overlayAssetDefinitionCount", len(assets)),
        ("instanceCount", len(placements)),
        ("anyNegativeScaleCount", any_negative_count),
        ("reflectedCount", reflected_count),
    )
    for field, actual in summary_gates:
        if field in summary and int(summary[field]) != actual:
            raise ShardBuildError(f"overlay summary {field} mismatch: {actual}")

    gates = (
        ("asset", len(assets), expect_assets),
        ("placement", len(placements), expect_placements),
        ("any-negative", any_negative_count, expect_any_negative),
        ("reflected", reflected_count, expect_reflected),
    )
    for label, actual, expected in gates:
        if expected is not None and actual != expected:
            raise ShardBuildError(f"overlay {label} count mismatch: {actual}")
    return {
        "document": document,
        "assetsById": assets_by_id,
        "groups": groups,
        "assetIds": set(assets_by_id),
        "referencedAssetIds": set().union(
            *(group["assetIds"] for group in groups.values())
        ),
        "sourceIds": source_ids,
        "runtimeIds": runtime_ids,
        "assetCount": len(assets),
        "placementCount": len(placements),
        "anyNegative": any_negative_count,
        "reflected": reflected_count,
        "levelCounts": dict(sorted(level_counts.items())),
        "defaultHiddenPlacementCount": default_hidden_count,
        "path": overlay_manifest_path,
    }


def parse_document_header(
    path: Path, expected_magic: str, expected_version: int,
) -> tuple[str, list[str]]:
    if not path.is_file():
        raise ShardBuildError(f"input document is missing: {path}")
    lines = path.read_text(encoding="utf-8").splitlines()
    if not lines:
        raise ShardBuildError(f"empty document: {path}")
    header = shlex.split(lines[0], posix=True)
    if (
        len(header) != 4
        or header[0] != expected_magic
        or int(header[1]) != expected_version
    ):
        raise ShardBuildError(f"unsupported document header: {path}")
    rows = [line for line in lines[1:] if line.strip()]
    declared_count = int(header[3])
    if declared_count != len(rows):
        raise ShardBuildError(
            f"document count mismatch: {path}: {declared_count} != {len(rows)}"
        )
    return header[2], rows


def parse_catalog(path: Path) -> dict[str, Any]:
    area_id, rows = parse_document_header(
        path, "LOSTARK_MAP_ASSET_CATALOG", 4
    )
    asset_ids: set[str] = set()
    for line_number, row in enumerate(rows, 2):
        fields = shlex.split(row, posix=True)
        if len(fields) != 26:
            raise ShardBuildError(f"catalog row {line_number} must contain 26 fields: {path}")
        asset_id = fields[0]
        if not asset_id or asset_id in asset_ids:
            raise ShardBuildError(f"duplicate/empty catalog asset ID: {asset_id!r}")
        numeric_indices = (*range(4, 7), *range(13, 26))
        if any(not math.isfinite(float(fields[index])) for index in numeric_indices):
            raise ShardBuildError(f"non-finite catalog value at row {line_number}: {path}")
        asset_ids.add(asset_id)
    return {"areaId": area_id, "rows": rows, "assetIds": asset_ids}


def parse_placements(path: Path, catalog_ids: set[str]) -> dict[str, Any]:
    area_id, rows = parse_document_header(path, "LOSTARK_MAP_PLACEMENTS", 2)
    parsed_rows: list[dict[str, Any]] = []
    source_ids: set[str] = set()
    runtime_ids: set[int] = set()
    any_negative_count = 0
    reflected_count = 0
    overlay_source_ids: set[str] = set()
    overlay_runtime_ids: set[int] = set()
    hidden_count = 0
    for line_number, row in enumerate(rows, 2):
        fields = shlex.split(row, posix=True)
        if len(fields) != 16:
            raise ShardBuildError(
                f"placement row {line_number} must contain 16 fields: {path}"
            )
        runtime_id = int(fields[0])
        source_id = fields[1]
        transform_source = fields[3]
        asset_id = fields[4]
        if not 0 < runtime_id <= UINT64_MAX:
            raise ShardBuildError(f"placement runtime ID is out of range: {runtime_id}")
        if not source_id or source_id in source_ids:
            raise ShardBuildError(f"duplicate/empty placement source ID: {source_id!r}")
        if runtime_id in runtime_ids:
            raise ShardBuildError(f"duplicate placement runtime ID: {runtime_id}")
        if transform_source == "overlay":
            if runtime_id > EDITOR_ID_MASK:
                raise ShardBuildError(
                    f"overlay placement runtime ID is outside its domain: {runtime_id}"
                )
            overlay_source_ids.add(source_id)
            overlay_runtime_ids.add(runtime_id)
        elif runtime_id != imported_id(source_id):
            raise ShardBuildError(f"placement runtime ID is not stable for {source_id}")
        if asset_id not in catalog_ids:
            raise ShardBuildError(f"placement asset join missing: {asset_id}")
        numeric = tuple(float(value) for value in fields[5:15])
        if not all(math.isfinite(value) for value in numeric):
            raise ShardBuildError(f"non-finite placement transform: {source_id}")
        scale = numeric[7:10]
        if any(abs(value) < 1.0e-6 for value in scale):
            raise ShardBuildError(f"zero placement scale: {source_id}")
        if fields[15] not in ("0", "1"):
            raise ShardBuildError(f"invalid placement visibility: {source_id}")
        visible = fields[15] == "1"
        hidden_count += int(not visible)
        any_negative, reflected = scale_flags(scale)
        any_negative_count += int(any_negative)
        reflected_count += int(reflected)
        source_ids.add(source_id)
        runtime_ids.add(runtime_id)
        parsed_rows.append(
            {
                "text": row,
                "runtimeId": runtime_id,
                "sourceId": source_id,
                "sourceLevel": fields[2],
                "assetId": asset_id,
                "anyNegative": any_negative,
                "reflected": reflected,
                "visible": visible,
            }
        )
    return {
        "areaId": area_id,
        "rows": parsed_rows,
        "sourceIds": source_ids,
        "runtimeIds": runtime_ids,
        "anyNegative": any_negative_count,
        "reflected": reflected_count,
        "overlaySourceIds": overlay_source_ids,
        "overlayRuntimeIds": overlay_runtime_ids,
        "overlayPlacementCount": len(overlay_source_ids),
        "hiddenPlacementCount": hidden_count,
    }


def write_normalized_placements(path: Path, rows: Sequence[dict[str, Any]]) -> None:
    document = {"schemaVersion": 1, "propertyErrors": [], "placements": list(rows)}
    atomic_write_text(path, json.dumps(document, ensure_ascii=False) + "\n")


def child_compile_arguments(
    args: argparse.Namespace,
    source_directory: Path,
    stage: Path,
    shard_id: str,
    group: dict[str, Any],
    *,
    overlay_group: dict[str, Any] | None = None,
    asset_manifest: Path | None = None,
    runtime_manifest: Path | None = None,
    overlay_manifest: Path | None = None,
    render_profile_manifest: Path | None = None,
) -> argparse.Namespace:
    levels = sorted(str(value) for value in group["levels"])
    overlay_asset_count = (
        0 if overlay_group is None else len(overlay_group["overlayAssetIds"])
    )
    overlay_placement_count = 0 if overlay_group is None else len(overlay_group["rows"])
    exact_asset_count = len(group["assetIds"])
    output_asset_count = len(group["assetIds"])
    output_placement_count = len(group["rows"])
    output_any_negative = group["anyNegative"]
    output_reflected = group["reflected"]
    if overlay_group is not None:
        exact_asset_count += len(overlay_group["exactAssetIds"] - group["assetIds"])
        output_asset_count = len(group["assetIds"] | overlay_group["assetIds"])
        output_placement_count += overlay_placement_count
        output_any_negative += overlay_group["anyNegative"]
        output_reflected += overlay_group["reflected"]
    return argparse.Namespace(
        area_id=AREA_ID,
        asset_manifest=args.asset_manifest if asset_manifest is None else asset_manifest,
        runtime_manifest=(
            args.runtime_manifest if runtime_manifest is None else runtime_manifest
        ),
        runtime_root=args.runtime_root,
        runtime_asset_root=getattr(args, "runtime_asset_root", None),
        overlay_manifest=overlay_manifest,
        render_profile_manifest=render_profile_manifest,
        placements_dir=source_directory,
        catalog_output=stage / shard_file_name(shard_id, "mapassets"),
        placement_output=stage / shard_file_name(shard_id, "mapplacements"),
        receipt_output=stage / f".{shard_id}.child.receipt.json",
        golden_placement_id="",
        include_source_id=[],
        include_level=levels if overlay_group is None else [],
        expect_assets=(
            args.expect_static_assets
            if overlay_group is None
            else exact_asset_count + overlay_asset_count
        ),
        expect_output_assets=output_asset_count,
        expect_source_placements=len(group["rows"]),
        expect_output_placements=output_placement_count,
        expect_output_any_negative=output_any_negative,
        expect_output_reflected=output_reflected,
        expect_any_negative=group["anyNegative"],
        expect_reflected=group["reflected"],
        expect_hidden_helpers=None,
        expect_output_hidden_helpers=None,
        expect_overlay_assets=overlay_asset_count,
        expect_overlay_placements=overlay_placement_count,
        expect_hidden_category=[],
        expect_level_count=[
            f"{level}={sum(1 for row in group['rows'] if row['levelPackage'] == level)}"
            for level in levels
        ],
    )


def write_child_manifests(
    stage: Path,
    shard_id: str,
    static_group: dict[str, Any],
    overlay_group: dict[str, Any],
    source_asset_document: dict[str, Any],
    source_runtime_document: dict[str, Any],
    overlay_inventory: dict[str, Any],
) -> tuple[Path, Path, Path]:
    source_assets = source_asset_document.get("assets", [])
    source_runtime_assets = source_runtime_document.get("assets", [])
    if not isinstance(source_assets, list) or not isinstance(
        source_runtime_assets, list
    ):
        raise ShardBuildError("invalid/duplicate static child manifest source")
    assets_by_id = {str(row["assetId"]): row for row in source_assets}
    runtime_by_id = {str(row["assetId"]): row for row in source_runtime_assets}
    if len(assets_by_id) != len(source_assets) or len(runtime_by_id) != len(
        source_runtime_assets
    ):
        raise ShardBuildError("invalid/duplicate static child manifest source")
    exact_asset_ids = set(static_group["assetIds"]) | set(
        overlay_group["exactAssetIds"]
    )
    if not exact_asset_ids.issubset(assets_by_id) or not exact_asset_ids.issubset(
        runtime_by_id
    ):
        raise ShardBuildError(f"{shard_id} exact overlay asset join missing")

    child_directory = stage / ".sources" / shard_id
    asset_path = child_directory / "assets.json"
    runtime_path = child_directory / "runtime.json"
    overlay_path = child_directory / "overlay.json"
    asset_document = {
        "schemaVersion": source_asset_document.get("schemaVersion", 1),
        "areaId": AREA_ID,
        "assetCount": len(exact_asset_ids),
        "assets": [assets_by_id[asset_id] for asset_id in sorted(exact_asset_ids)],
    }
    runtime_document = {
        "schemaVersion": source_runtime_document.get("schemaVersion", 1),
        "areaId": AREA_ID,
        "assetCount": len(exact_asset_ids),
        "assets": [runtime_by_id[asset_id] for asset_id in sorted(exact_asset_ids)],
    }
    overlay_asset_ids = set(overlay_group["overlayAssetIds"])
    overlay_document = {
        "schemaVersion": 1,
        "areaId": AREA_ID,
        "kind": overlay_inventory["document"].get("kind", "shard-overlay"),
        "assetCount": len(overlay_asset_ids),
        "placementCount": len(overlay_group["rows"]),
        "assets": [
            overlay_inventory["assetsById"][asset_id]
            for asset_id in sorted(overlay_asset_ids)
        ],
        "placements": list(overlay_group["rows"]),
    }
    atomic_write_text(asset_path, json.dumps(asset_document, ensure_ascii=False) + "\n")
    atomic_write_text(
        runtime_path, json.dumps(runtime_document, ensure_ascii=False) + "\n"
    )
    atomic_write_text(
        overlay_path, json.dumps(overlay_document, ensure_ascii=False) + "\n"
    )
    return asset_path, runtime_path, overlay_path


def load_render_profile_source(
    path: Path | None,
    known_asset_ids: set[str],
    known_source_ids: set[str],
) -> dict[str, Any] | None:
    if path is None:
        return None
    document = load_json(path)
    if document.get("schemaVersion") != 1 or document.get("areaId") != AREA_ID:
        raise ShardBuildError("render profile manifest schema/areaId mismatch")
    profiles = document.get("profiles", [])
    if not isinstance(profiles, list):
        raise ShardBuildError("render profile manifest profiles must be an array")
    profile_ids = [str(profile.get("assetId", "")) for profile in profiles]
    if any(not asset_id for asset_id in profile_ids) or len(set(profile_ids)) != len(
        profile_ids
    ):
        raise ShardBuildError("duplicate/empty render profile assetId")
    unknown = set(profile_ids) - known_asset_ids
    if unknown:
        raise ShardBuildError(
            f"render profiles reference unknown Bern assets: {sorted(unknown)}"
        )
    overrides = document.get("visibilityOverrides", [])
    if not isinstance(overrides, list):
        raise ShardBuildError("visibilityOverrides must be an array")
    override_ids: list[str] = []
    for override in overrides:
        if not isinstance(override, dict):
            raise ShardBuildError("visibility override must be an object")
        source_id = str(override.get("sourcePlacementId", ""))
        if not source_id or type(override.get("visible")) is not bool:
            raise ShardBuildError("visibility override requires sourcePlacementId and bool visible")
        override_ids.append(source_id)
    if len(set(override_ids)) != len(override_ids):
        raise ShardBuildError("duplicate visibility override sourcePlacementId")
    unknown_sources = set(override_ids) - known_source_ids
    if unknown_sources:
        raise ShardBuildError(
            "visibility overrides reference unknown Bern placements: "
            f"{sorted(unknown_sources)}"
        )
    return document


def write_child_render_profile_manifest(
    stage: Path,
    shard_id: str,
    static_group: dict[str, Any],
    overlay_group: dict[str, Any],
    hidden_levels: frozenset[str],
    render_profile_source: dict[str, Any] | None,
) -> Path | None:
    shard_asset_ids = set(static_group["assetIds"]) | set(overlay_group["assetIds"])
    profiles = (
        []
        if render_profile_source is None
        else [
            profile
            for profile in render_profile_source.get("profiles", [])
            if str(profile.get("assetId", "")) in shard_asset_ids
        ]
    )
    overrides_by_source = {
        str(row["placementId"]): False
        for row in static_group["rows"]
        if str(row.get("levelPackage", "")) in hidden_levels
    }
    shard_source_ids = set(static_group["sourceIds"]) | set(overlay_group["sourceIds"])
    if render_profile_source is not None:
        for override in render_profile_source.get("visibilityOverrides", []):
            source_id = str(override["sourcePlacementId"])
            if source_id not in shard_source_ids:
                continue
            visible = bool(override["visible"])
            previous = overrides_by_source.get(source_id)
            if previous is not None and previous != visible:
                raise ShardBuildError(
                    f"visibility override conflicts with default-hidden placement: {source_id}"
                )
            overrides_by_source[source_id] = visible
    overrides = [
        {
            "sourcePlacementId": source_id,
            "visible": visible,
        }
        for source_id, visible in sorted(overrides_by_source.items())
    ]
    if not profiles and not overrides:
        return None
    path = stage / ".sources" / shard_id / "renderprofiles.json"
    document = {
        "schemaVersion": 1,
        "areaId": AREA_ID,
        "profiles": profiles,
        "visibilityOverrides": overrides,
    }
    atomic_write_text(path, json.dumps(document, ensure_ascii=False) + "\n")
    return path


def stage_static_shards(
    args: argparse.Namespace,
    stage: Path,
    inventory: dict[str, Any],
    overlay_inventory: dict[str, Any] | None,
    hidden_levels: frozenset[str],
    render_profile_source: dict[str, Any] | None,
) -> None:
    source_asset_document = None
    source_runtime_document = None
    if overlay_inventory is not None:
        source_asset_document = load_json(args.asset_manifest)
        source_runtime_document = load_json(args.runtime_manifest)
    for shard_id in ("BASE", *SL_SHARD_IDS):
        group = inventory["groups"][shard_id]
        overlay_group = (
            empty_overlay_group()
            if overlay_inventory is None
            else overlay_inventory["groups"][shard_id]
        )
        source_directory = stage / ".sources" / shard_id
        write_normalized_placements(
            source_directory / f"{shard_id}.placements.json", group["rows"]
        )
        render_profile_path = write_child_render_profile_manifest(
            stage,
            shard_id,
            group,
            overlay_group,
            hidden_levels,
            render_profile_source,
        )
        if overlay_inventory is None:
            child_arguments = child_compile_arguments(
                args,
                source_directory,
                stage,
                shard_id,
                group,
                render_profile_manifest=render_profile_path,
            )
        else:
            asset_path, runtime_path, overlay_path = write_child_manifests(
                stage,
                shard_id,
                group,
                overlay_group,
                source_asset_document,
                source_runtime_document,
                overlay_inventory,
            )
            child_arguments = child_compile_arguments(
                args,
                source_directory,
                stage,
                shard_id,
                group,
                overlay_group=overlay_group,
                asset_manifest=asset_path,
                runtime_manifest=runtime_path,
                overlay_manifest=overlay_path,
                render_profile_manifest=render_profile_path,
            )
        compile_scene(child_arguments)


def stage_landscape_shard(
    args: argparse.Namespace, stage: Path
) -> dict[str, Any]:
    source_catalog = parse_catalog(args.landscape_catalog)
    source_placements = parse_placements(
        args.landscape_placements, source_catalog["assetIds"]
    )
    if len(source_catalog["rows"]) != args.expect_landscape_assets:
        raise ShardBuildError(
            f"landscape asset count mismatch: {len(source_catalog['rows'])}"
        )
    if len(source_placements["rows"]) != args.expect_landscape_placements:
        raise ShardBuildError(
            f"landscape placement count mismatch: {len(source_placements['rows'])}"
        )
    referenced = {str(row["assetId"]) for row in source_placements["rows"]}
    if referenced != source_catalog["assetIds"]:
        raise ShardBuildError("landscape catalog/reference set mismatch")

    catalog_output = stage / shard_file_name("LANDSCAPE", "mapassets")
    placement_output = stage / shard_file_name("LANDSCAPE", "mapplacements")
    atomic_write_text(
        catalog_output,
        f"LOSTARK_MAP_ASSET_CATALOG 4 {json.dumps(AREA_ID)} {len(source_catalog['rows'])}\n"
        + "\n".join(source_catalog["rows"])
        + "\n",
    )
    atomic_write_text(
        placement_output,
        f"LOSTARK_MAP_PLACEMENTS 2 {json.dumps(AREA_ID)} {len(source_placements['rows'])}\n"
        + "\n".join(str(row["text"]) for row in source_placements["rows"])
        + "\n",
    )
    return source_placements


def validate_staged_shard(
    stage: Path,
    shard_id: str,
    expected_asset_ids: set[str],
    expected_source_ids: set[str],
    expected_levels: set[str] | None,
    max_catalog_assets: int,
    default_hidden_levels: frozenset[str],
) -> dict[str, Any]:
    catalog_name = shard_file_name(shard_id, "mapassets")
    placements_name = shard_file_name(shard_id, "mapplacements")
    catalog = parse_catalog(stage / catalog_name)
    placements = parse_placements(stage / placements_name, catalog["assetIds"])
    if catalog["areaId"] != AREA_ID or placements["areaId"] != AREA_ID:
        raise ShardBuildError(f"{shard_id} child areaId mismatch")
    if len(catalog["assetIds"]) > max_catalog_assets:
        raise ShardBuildError(
            f"{shard_id} physical catalog exceeds {max_catalog_assets}: "
            f"{len(catalog['assetIds'])}"
        )
    if catalog["assetIds"] != expected_asset_ids:
        raise ShardBuildError(f"{shard_id} catalog asset set mismatch")
    if placements["sourceIds"] != expected_source_ids:
        raise ShardBuildError(f"{shard_id} placement source set mismatch")
    if expected_levels is not None:
        actual_levels = {str(row["sourceLevel"]) for row in placements["rows"]}
        if actual_levels != expected_levels:
            raise ShardBuildError(f"{shard_id} placement level set mismatch")
    default_hidden_count = sum(
        1
        for row in placements["rows"]
        if str(row["sourceLevel"]) in default_hidden_levels and not row["visible"]
    )
    return {
        "shardId": shard_id,
        "catalogFile": catalog_name,
        "placementFile": placements_name,
        "assetIds": catalog["assetIds"],
        "sourceIds": placements["sourceIds"],
        "runtimeIds": placements["runtimeIds"],
        "overlaySourceIds": placements["overlaySourceIds"],
        "overlayRuntimeIds": placements["overlayRuntimeIds"],
        "overlayPlacementCount": placements["overlayPlacementCount"],
        "assetCount": len(catalog["assetIds"]),
        "placementCount": len(placements["rows"]),
        "anyNegativeScaleCount": placements["anyNegative"],
        "reflectedScaleCount": placements["reflected"],
        "hiddenPlacementCount": placements["hiddenPlacementCount"],
        "defaultHiddenPlacementCount": default_hidden_count,
    }


def render_mapset(shards: Sequence[dict[str, Any]]) -> str:
    rows: list[str] = []
    for shard in shards:
        catalog_name = validate_relative_filename(str(shard["catalogFile"]))
        placement_name = validate_relative_filename(str(shard["placementFile"]))
        rows.append(
            " ".join(
                (
                    json.dumps(str(shard["shardId"])),
                    json.dumps(catalog_name),
                    json.dumps(placement_name),
                    str(shard["assetCount"]),
                    str(shard["placementCount"]),
                )
            )
        )
    return (
        f"LOSTARK_MAP_SHARD_SET 1 {json.dumps(AREA_ID)} {len(rows)}\n"
        + "\n".join(rows)
        + "\n"
    )


def validate_mapset(path: Path, expected_shards: Sequence[dict[str, Any]]) -> None:
    area_id, rows = parse_document_header(path, "LOSTARK_MAP_SHARD_SET", 1)
    if area_id != AREA_ID or len(rows) != len(SHARD_IDS):
        raise ShardBuildError("mapset area/count mismatch")
    expected_by_id = {str(row["shardId"]): row for row in expected_shards}
    if list(expected_by_id) != list(SHARD_IDS):
        raise ShardBuildError("staged shard order mismatch")
    for row, shard_id in zip(rows, SHARD_IDS):
        fields = shlex.split(row, posix=True)
        if len(fields) != 5 or fields[0] != shard_id:
            raise ShardBuildError(f"invalid mapset row: {row}")
        catalog_name = validate_relative_filename(fields[1])
        placement_name = validate_relative_filename(fields[2])
        expected = expected_by_id[shard_id]
        if (
            catalog_name != expected["catalogFile"]
            or placement_name != expected["placementFile"]
            or int(fields[3]) != expected["assetCount"]
            or int(fields[4]) != expected["placementCount"]
            or not (path.parent / catalog_name).is_file()
            or not (path.parent / placement_name).is_file()
        ):
            raise ShardBuildError(f"mapset child contract mismatch: {shard_id}")


def validate_aggregate(
    args: argparse.Namespace,
    shards: Sequence[dict[str, Any]],
    static_inventory: dict[str, Any],
    overlay_inventory: dict[str, Any] | None,
) -> None:
    all_asset_ids: set[str] = set()
    all_source_ids: set[str] = set()
    all_runtime_ids: set[int] = set()
    total_placements = 0
    static_placements = 0
    any_negative = 0
    reflected = 0
    hidden = 0
    for shard in shards:
        duplicate_sources = all_source_ids.intersection(shard["sourceIds"])
        duplicate_runtime = all_runtime_ids.intersection(shard["runtimeIds"])
        if duplicate_sources:
            raise ShardBuildError(
                f"global source placement ID collision: {sorted(duplicate_sources)[:5]}"
            )
        if duplicate_runtime:
            raise ShardBuildError(
                f"global runtime placement ID collision: {sorted(duplicate_runtime)[:5]}"
            )
        all_asset_ids.update(shard["assetIds"])
        all_source_ids.update(shard["sourceIds"])
        all_runtime_ids.update(shard["runtimeIds"])
        total_placements += int(shard["placementCount"])
        any_negative += int(shard["anyNegativeScaleCount"])
        reflected += int(shard["reflectedScaleCount"])
        hidden += int(shard["defaultHiddenPlacementCount"])
        if shard["shardId"] != "LANDSCAPE":
            static_placements += len(
                shard["sourceIds"].intersection(static_inventory["sourceIds"])
            )

    if static_placements != args.expect_static_placements:
        raise ShardBuildError(f"aggregate static placement mismatch: {static_placements}")
    if total_placements != args.expect_total_placements:
        raise ShardBuildError(f"aggregate placement mismatch: {total_placements}")
    if len(all_asset_ids) != args.expect_unique_assets:
        raise ShardBuildError(f"aggregate unique asset mismatch: {len(all_asset_ids)}")
    if any_negative != args.expect_any_negative:
        raise ShardBuildError(f"aggregate any-negative mismatch: {any_negative}")
    if reflected != args.expect_reflected:
        raise ShardBuildError(f"aggregate reflected mismatch: {reflected}")
    expected_hidden = getattr(args, "expect_default_hidden_placements", None)
    if expected_hidden is not None and hidden != expected_hidden:
        raise ShardBuildError(f"aggregate default-hidden mismatch: {hidden}")
    if not static_inventory["sourceIds"].issubset(all_source_ids):
        raise ShardBuildError("aggregate output lost static source placements")
    if not static_inventory["runtimeIds"].issubset(all_runtime_ids):
        raise ShardBuildError("aggregate output lost static runtime placements")
    if overlay_inventory is not None:
        if not overlay_inventory["sourceIds"].issubset(all_source_ids):
            raise ShardBuildError("aggregate output lost overlay source placements")
        if not overlay_inventory["runtimeIds"].issubset(all_runtime_ids):
            raise ShardBuildError("aggregate output lost overlay runtime placements")
        output_overlay_sources = set().union(
            *(shard.get("overlaySourceIds", set()) for shard in shards)
        )
        if output_overlay_sources != overlay_inventory["sourceIds"]:
            raise ShardBuildError("aggregate overlay source placement set mismatch")


def safe_output_directory(path: Path) -> Path:
    resolved = path.resolve()
    if resolved == Path(resolved.anchor):
        raise ShardBuildError(f"output directory is too broad: {resolved}")
    resolved.mkdir(parents=True, exist_ok=True)
    return resolved


def install_files(
    stage: Path, output_directory: Path, filenames: Sequence[str]
) -> None:
    backup = stage / ".backup"
    backup.mkdir()
    installed: list[str] = []
    try:
        for filename in filenames:
            validate_relative_filename(filename)
            source = stage / filename
            destination = output_directory / filename
            if not source.is_file():
                raise ShardBuildError(f"staged output is missing: {source}")
            if destination.exists():
                shutil.copy2(destination, backup / filename)
            os.replace(source, destination)
            installed.append(filename)
    except BaseException:
        for filename in reversed(installed):
            destination = output_directory / filename
            prior = backup / filename
            if prior.is_file():
                os.replace(prior, destination)
            else:
                destination.unlink(missing_ok=True)
        raise


def build_shards(args: argparse.Namespace) -> dict[str, Any]:
    expected_level_counts = parse_count_specs(args.expect_level_count)
    hidden_levels = parse_hidden_levels(
        getattr(args, "default_hidden_level", None)
    )
    placement_directories = [Path(value) for value in args.placements_dir]
    static_inventory = scan_static_sources(
        args.asset_manifest,
        placement_directories,
        expected_level_counts,
        args.expect_static_assets,
        args.expect_static_placements,
        hidden_levels,
    )
    overlay_manifest = getattr(args, "overlay_manifest", None)
    overlay_inventory = scan_overlay_source(
        overlay_manifest,
        static_inventory,
        getattr(args, "expect_overlay_assets", None),
        getattr(args, "expect_overlay_placements", None),
        getattr(args, "expect_overlay_any_negative", None),
        getattr(args, "expect_overlay_reflected", None),
        hidden_levels,
    )
    render_profile_manifest = getattr(args, "render_profile_manifest", None)
    known_asset_ids = set(static_inventory["assetIds"])
    known_source_ids = set(static_inventory["sourceIds"])
    if overlay_inventory is not None:
        known_asset_ids.update(overlay_inventory["assetIds"])
        known_source_ids.update(overlay_inventory["sourceIds"])
    render_profile_source = load_render_profile_source(
        render_profile_manifest, known_asset_ids, known_source_ids
    )
    if (
        overlay_inventory is not None
        and overlay_inventory["assetCount"] > 0
        and getattr(args, "runtime_asset_root", None) is None
    ):
        raise ShardBuildError("runtime-asset-root is required for overlay assets")
    output_directory = safe_output_directory(args.output_dir)
    # tempfile.mkdtemp applies a private ACL on Windows.  os.replace then carries
    # that ACL onto the installed files, which makes generated map data unreadable
    # to other team/tool accounts.  A normal child directory inherits the output
    # directory ACL while retaining the same stage-validate-commit workflow.
    stage = output_directory / f".{AREA_ID}.stage.{uuid.uuid4().hex}"
    stage.mkdir(parents=False, exist_ok=False)
    try:
        stage_static_shards(
            args,
            stage,
            static_inventory,
            overlay_inventory,
            hidden_levels,
            render_profile_source,
        )
        landscape_source = stage_landscape_shard(args, stage)

        shards: list[dict[str, Any]] = []
        base = static_inventory["groups"]["BASE"]
        base_overlay = (
            empty_overlay_group()
            if overlay_inventory is None
            else overlay_inventory["groups"]["BASE"]
        )
        shards.append(
            validate_staged_shard(
                stage,
                "BASE",
                set(base["assetIds"]) | set(base_overlay["assetIds"]),
                set(base["sourceIds"]) | set(base_overlay["sourceIds"]),
                set(base["levels"]) | set(base_overlay["levels"]),
                args.max_catalog_assets,
                hidden_levels,
            )
        )
        landscape_catalog = parse_catalog(stage / shard_file_name("LANDSCAPE", "mapassets"))
        shards.append(
            validate_staged_shard(
                stage,
                "LANDSCAPE",
                set(landscape_catalog["assetIds"]),
                set(landscape_source["sourceIds"]),
                None,
                args.max_catalog_assets,
                hidden_levels,
            )
        )
        for shard_id in SL_SHARD_IDS:
            group = static_inventory["groups"][shard_id]
            overlay_group = (
                empty_overlay_group()
                if overlay_inventory is None
                else overlay_inventory["groups"][shard_id]
            )
            shards.append(
                validate_staged_shard(
                    stage,
                    shard_id,
                    set(group["assetIds"]) | set(overlay_group["assetIds"]),
                    set(group["sourceIds"]) | set(overlay_group["sourceIds"]),
                    set(group["levels"]) | set(overlay_group["levels"]),
                    args.max_catalog_assets,
                    hidden_levels,
                )
            )
        if tuple(str(row["shardId"]) for row in shards) != SHARD_IDS:
            raise ShardBuildError("staged shard order mismatch")

        validate_aggregate(args, shards, static_inventory, overlay_inventory)
        mapset_name = f"{AREA_ID}.mapset"
        atomic_write_text(stage / mapset_name, render_mapset(shards))
        validate_mapset(stage / mapset_name, shards)

        output_names = [
            name
            for shard_id in SHARD_IDS
            for name in (
                shard_file_name(shard_id, "mapassets"),
                shard_file_name(shard_id, "mapplacements"),
            )
        ]
        output_names.append(mapset_name)
        receipt_name = f"{AREA_ID}.shards.receipt.json"
        receipt = {
            "schemaVersion": 1,
            "areaId": AREA_ID,
            "shardCount": len(shards),
            "staticAssetCount": args.expect_static_assets,
            "uniqueAssetCount": args.expect_unique_assets,
            "staticPlacementCount": args.expect_static_placements,
            "landscapePlacementCount": args.expect_landscape_placements,
            "placementCount": args.expect_total_placements,
            "anyNegativeScaleCount": args.expect_any_negative,
            "reflectedScaleCount": args.expect_reflected,
            "defaultHiddenLevels": sorted(hidden_levels),
            "defaultHiddenPlacementCount": sum(
                int(row["defaultHiddenPlacementCount"]) for row in shards
            ),
            "hiddenPlacementCount": sum(
                int(row["hiddenPlacementCount"]) for row in shards
            ),
            "maxPhysicalCatalogAssets": max(row["assetCount"] for row in shards),
            "physicalCatalogLimit": args.max_catalog_assets,
            "levelCounts": static_inventory["levelCounts"],
            "shards": [
                {
                    "shardId": row["shardId"],
                    "catalogFile": row["catalogFile"],
                    "placementFile": row["placementFile"],
                    "assetCount": row["assetCount"],
                    "placementCount": row["placementCount"],
                    "anyNegativeScaleCount": row["anyNegativeScaleCount"],
                    "reflectedScaleCount": row["reflectedScaleCount"],
                    "hiddenPlacementCount": row["hiddenPlacementCount"],
                    "defaultHiddenPlacementCount": row[
                        "defaultHiddenPlacementCount"
                    ],
                }
                for row in shards
            ],
            "inputs": {
                "assetManifest": {
                    "path": args.asset_manifest.as_posix(),
                    "sha256": sha256(args.asset_manifest),
                },
                "runtimeManifest": {
                    "path": args.runtime_manifest.as_posix(),
                    "sha256": sha256(args.runtime_manifest),
                },
                "runtimeRoot": args.runtime_root.as_posix(),
                "placementDirectories": [
                    directory.as_posix() for directory in placement_directories
                ],
                "placements": static_inventory["sourceRecords"],
                "landscapeCatalog": {
                    "path": args.landscape_catalog.as_posix(),
                    "sha256": sha256(args.landscape_catalog),
                },
                "landscapePlacements": {
                    "path": args.landscape_placements.as_posix(),
                    "sha256": sha256(args.landscape_placements),
                },
            },
            "outputs": {
                filename: sha256(stage / filename) for filename in output_names
            },
        }
        if overlay_inventory is not None:
            receipt.update(
                {
                    "overlayAssetCount": overlay_inventory["assetCount"],
                    "overlayReferencedAssetCount": len(
                        overlay_inventory["referencedAssetIds"]
                    ),
                    "overlayPlacementCount": overlay_inventory["placementCount"],
                    "overlayAnyNegativeScaleCount": overlay_inventory["anyNegative"],
                    "overlayReflectedScaleCount": overlay_inventory["reflected"],
                    "overlayLevelCounts": overlay_inventory["levelCounts"],
                }
            )
            receipt["inputs"]["overlayManifest"] = {
                "path": overlay_manifest.as_posix(),
                "sha256": sha256(overlay_manifest),
            }
            receipt["inputs"]["runtimeAssetRoot"] = getattr(
                args, "runtime_asset_root"
            ).as_posix()
        if render_profile_manifest is not None:
            receipt["inputs"]["renderProfileManifest"] = {
                "path": render_profile_manifest.as_posix(),
                "sha256": sha256(render_profile_manifest),
            }
        atomic_write_text(
            stage / receipt_name,
            json.dumps(receipt, ensure_ascii=False, indent=2) + "\n",
        )
        install_files(stage, output_directory, [*output_names, receipt_name])
        return receipt
    finally:
        if stage.exists():
            shutil.rmtree(stage)


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--asset-manifest", type=Path, required=True)
    parser.add_argument("--runtime-manifest", type=Path, required=True)
    parser.add_argument("--runtime-root", type=Path, required=True)
    parser.add_argument("--runtime-asset-root", type=Path)
    parser.add_argument("--overlay-manifest", type=Path)
    parser.add_argument(
        "--render-profile-manifest",
        type=Path,
        default=DEFAULT_RENDER_PROFILE_MANIFEST,
    )
    parser.add_argument("--placements-dir", type=Path, action="append", required=True)
    parser.add_argument("--landscape-catalog", type=Path, required=True)
    parser.add_argument("--landscape-placements", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--expect-static-assets", type=int, default=950)
    parser.add_argument("--expect-static-placements", type=int, default=32324)
    parser.add_argument("--expect-landscape-assets", type=int, default=42)
    parser.add_argument("--expect-landscape-placements", type=int, default=42)
    parser.add_argument("--expect-total-placements", type=int, default=32366)
    parser.add_argument("--expect-unique-assets", type=int, default=992)
    parser.add_argument("--expect-any-negative", type=int, default=7983)
    parser.add_argument("--expect-reflected", type=int, default=7979)
    parser.add_argument("--expect-overlay-assets", type=int)
    parser.add_argument("--expect-overlay-placements", type=int)
    parser.add_argument("--expect-overlay-any-negative", type=int)
    parser.add_argument("--expect-overlay-reflected", type=int)
    parser.add_argument("--default-hidden-level", action="append")
    parser.add_argument("--expect-default-hidden-placements", type=int, default=1178)
    parser.add_argument("--max-catalog-assets", type=int, default=512)
    parser.add_argument("--expect-level-count", action="append")
    return parser.parse_args(argv)


def main(argv: Iterable[str] | None = None) -> int:
    args = parse_args(argv)
    receipt = build_shards(args)
    print(json.dumps(receipt, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (ShardBuildError, ValueError, OSError, KeyError, TypeError) as error:
        print(
            json.dumps({"status": "failed", "error": str(error)}, ensure_ascii=False),
            file=os.sys.stderr,
        )
        raise SystemExit(1)
