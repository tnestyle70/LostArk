#!/usr/bin/env python3
"""Locate every Effect family's cooked material shader maps in the pinned cache.

`extract_ue3_material_shader_maps.py` admits one pre-pinned target at a time:
it is the gate a family passes through once its offsets, denominators and
hashes are already known.  Nothing produced those facts for a family that has
never been joined, so a family with no receipt looked identical to a family
with no recoverable evidence.  They are not the same thing, and this tool tells
them apart.

For every parent Material named by the authored corpus it:

* resolves the parent to a staged source package and its native export;
* reads the base Material ID out of the export's native tail, which is where
  UE3 serializes it after the tagged property stream;
* scans the pinned RefShaderCache once for every base ID at the same time, and
  keeps only the hits that parse as an FStaticParameterSet followed by a
  well-formed material-map header.

The result is a per-family census of cooked static permutations: how many
exist, where each one starts and ends, and how many vertex factories it
carries.  It admits nothing.  Selecting a permutation, extracting its packed
DXBC and translating the program remain separate, individually gated steps.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import json
import struct
import sys
import time
from pathlib import Path
from typing import Any

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
LEVEL_TOOLS = REPOSITORY_ROOT / "Tools" / "LevelPlacementExtractor"
for extra in (str(Path(__file__).resolve().parent), str(LEVEL_TOOLS)):
    if extra not in sys.path:
        sys.path.insert(0, extra)

from derive_artist_31470_main_shader_map_identity import (  # noqa: E402
    engine_equivalent_static_parameter_set,
)
from extract_artist_31470_main_ref_shader_cache import (  # noqa: E402
    package_tables,
)
from extract_artist_31470_shader_cache_oracle import (  # noqa: E402
    canonical_json_sha256,
    parse_static_parameter_set,
)
from extract_ue3_effect_material_closure import load_package  # noqa: E402
from extract_ue3_material_shader_maps import (  # noqa: E402
    parse_shader_code_layout,
)
from extract_ue3_placements import (  # noqa: E402
    LOSTARK_KR_AES_KEY,
    package_ref_name,
    package_ref_path,
    parse_tagged_properties,
)

AUTHORED_DIRECTORY = REPOSITORY_ROOT / "Data/Effects/Authored"
DEFAULT_OUTPUT = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-shader-map-index.v1.json")
DEFAULT_SOURCE_PACK_MANIFEST = Path(
    r"C:\Users\user\Desktop\Resource_LostArk\00_SourcePackages"
    r"\Effect_DIMENSIONMASTER_20260803_v3\source_pack_manifest.json"
)
DEFAULT_CACHE = Path(
    r"C:\Users\user\Desktop\Resource_LostArk\01_Extracted\Effect\ARTIST"
    r"\31470_TrackA_20260812\OfficialRefShaderCacheV974"
    r"\EV2LG3OVEH3HGV7THTFFTM7TOKMCC.v974.upk"
)

SCHEMA = "lostark.effect-family-shader-map-index"
FORMAT_VERSION = 1

MATERIAL_CLASSES = ("material", "materialinstanceconstant")

RESOLVED_DECLARED = "DECLARED_PACKAGE_EXPORT"
RESOLVED_SEARCH = "LEAF_NAME_SEARCH"
RESOLVED_AMBIGUOUS = "LEAF_NAME_AMBIGUOUS"
RESOLVED_ABSENT = "PARENT_MATERIAL_EXPORT_ABSENT"

# The native tail of a Material export starts with three serialized counters
# before the FGuid.  Reading it positionally is what the frozen Artist F
# extractors already do, and the constant is named here so a layout change
# fails loudly instead of silently producing a wrong key.
BASE_MATERIAL_ID_TAIL_OFFSET = 16
BASE_MATERIAL_ID_BYTE_SIZE = 16


class IndexError_(RuntimeError):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise IndexError_(message)


def canonical_json(value: Any) -> str:
    return json.dumps(
        value, sort_keys=True, separators=(",", ":"), ensure_ascii=False
    )


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json(value).encode("utf-8")).hexdigest()


def read_json(path: Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8"))


def raw_file_identity(path: Path, description: str) -> dict[str, Any]:
    require(path.is_file(), f"{description} is missing: {path}")
    digest = hashlib.sha256()
    byte_size = 0
    with path.open("rb") as stream:
        while chunk := stream.read(1024 * 1024):
            digest.update(chunk)
            byte_size += len(chunk)
    return {
        "rawSha256": digest.hexdigest(),
        "byteSize": byte_size,
    }


def collect_parent_materials(authored: Path) -> dict[str, dict[str, Any]]:
    """Parent material path -> its occurrence and class denominators."""
    parents: dict[str, dict[str, Any]] = {}
    for path in sorted(authored.glob("*.effect.json")):
        document = read_json(path)
        effect_asset_id = document.get("effectAssetId", path.stem)
        parts = effect_asset_id.split(".")
        klass = parts[1] if len(parts) > 2 else "unknown"
        for element in document.get("elements", []):
            profile = (element.get("material") or {}).get(
                "sourceProfile") or {}
            parent = profile.get("parentMaterialPath")
            if not parent:
                continue
            row = parents.setdefault(parent, {
                "occurrenceCount": 0,
                "classes": collections.Counter(),
            })
            row["occurrenceCount"] += 1
            row["classes"][klass] += 1
    require(bool(parents), "authored corpus names no parent materials")
    return parents


def load_package_index(manifest_path: Path) -> dict[str, Path]:
    require(manifest_path.is_file(),
            f"source pack manifest is missing: {manifest_path}")
    manifest = read_json(manifest_path)
    root = manifest_path.parent
    index: dict[str, Path] = {}
    for package in manifest.get("packages", []):
        if not package.get("resolved"):
            continue
        name = str(package.get("logicalPackage") or "").lower()
        relative = package.get("relativePath")
        if not name or not relative:
            continue
        index.setdefault(name, root / relative)
    require(bool(index), "source pack manifest resolved no packages")
    return index


def export_material_rows(package: Any) -> dict[str, list[dict[str, Any]]]:
    """Leaf object name -> every Material/MIC export that carries it."""
    rows: dict[str, list[dict[str, Any]]] = collections.defaultdict(list)
    for entry in package.exports:
        class_name = package_ref_name(
            entry.class_index, package.imports, package.exports).lower()
        if class_name not in MATERIAL_CLASSES:
            continue
        rows[entry.object_name.lower()].append({
            "className": class_name,
            "exportIndexZeroBased": entry.index,
            "objectPath": package_ref_path(
                entry.index + 1, package.imports, package.exports),
            "serialOffset": entry.serial_offset,
            "serialByteSize": entry.serial_size,
        })
    return rows


def base_material_id(package: Any, row: dict[str, Any]) -> str | None:
    """Read the FGuid UE3 writes after a Material's tagged property stream."""
    serial = package.logical[
        row["serialOffset"]: row["serialOffset"] + row["serialByteSize"]]
    try:
        _, property_end = parse_tagged_properties(
            serial, package.names, package.summary.version)
    except Exception:
        return None
    tail = serial[property_end:]
    end = BASE_MATERIAL_ID_TAIL_OFFSET + BASE_MATERIAL_ID_BYTE_SIZE
    if len(tail) < end:
        return None
    return tail[BASE_MATERIAL_ID_TAIL_OFFSET:end].hex()


def resolve_parents(
    parents: dict[str, dict[str, Any]],
    package_index: dict[str, Path],
    verbose: bool,
) -> tuple[dict[str, dict[str, Any]], dict[str, int]]:
    """Resolve every parent to one package export, declared first."""
    declared: dict[str, list[str]] = collections.defaultdict(list)
    orphans: list[str] = []
    for parent in sorted(parents):
        segments = parent.split(".")
        package_name = segments[0].lower() if len(segments) >= 3 else None
        if package_name is not None and package_name in package_index:
            declared[package_name].append(parent)
        else:
            orphans.append(parent)

    resolutions: dict[str, dict[str, Any]] = {}
    opened = 0
    for package_name in sorted(declared):
        package = load_package(package_index[package_name], LOSTARK_KR_AES_KEY)
        opened += 1
        rows = export_material_rows(package)
        for parent in declared[package_name]:
            leaf = parent.rsplit(".", 1)[-1].lower()
            candidates = rows.get(leaf) or []
            materials = [row for row in candidates
                         if row["className"] == "material"]
            if not materials:
                orphans.append(parent)
                continue
            row = materials[0]
            resolutions[parent] = {
                "resolvedBy": RESOLVED_DECLARED,
                "packageName": package_name,
                "packageFileName": package_index[package_name].name,
                **row,
                "baseMaterialIdHex": base_material_id(package, row),
                "duplicateExportCount": len(materials) - 1,
            }

    if orphans:
        if verbose:
            print(f"  searching all packages for {len(orphans)} parents",
                  flush=True)
        wanted = {parent.rsplit(".", 1)[-1].lower() for parent in orphans}
        found: dict[str, list[dict[str, Any]]] = collections.defaultdict(list)
        for package_name in sorted(package_index):
            try:
                package = load_package(
                    package_index[package_name], LOSTARK_KR_AES_KEY)
            except Exception:
                continue
            opened += 1
            rows = export_material_rows(package)
            for leaf in wanted & set(rows):
                for row in rows[leaf]:
                    if row["className"] != "material":
                        continue
                    found[leaf].append({
                        "packageName": package_name,
                        "packageFileName": package_index[package_name].name,
                        **row,
                        "baseMaterialIdHex": base_material_id(package, row),
                    })
        for parent in orphans:
            leaf = parent.rsplit(".", 1)[-1].lower()
            candidates = found.get(leaf) or []
            identities = {row["baseMaterialIdHex"] for row in candidates}
            if not candidates:
                resolutions[parent] = {
                    "resolvedBy": RESOLVED_ABSENT,
                    "packageName": None,
                    "baseMaterialIdHex": None,
                }
            elif len(identities) > 1:
                resolutions[parent] = {
                    "resolvedBy": RESOLVED_AMBIGUOUS,
                    "candidatePackages": sorted(
                        row["packageName"] for row in candidates),
                    "baseMaterialIdHex": None,
                }
            else:
                resolutions[parent] = {
                    "resolvedBy": RESOLVED_SEARCH,
                    "duplicateExportCount": len(candidates) - 1,
                    **candidates[0],
                }

    counts = collections.Counter(
        row["resolvedBy"] for row in resolutions.values())
    return resolutions, dict(sorted(counts.items()))


def scan_cache(
    cache_path: Path,
    base_material_ids: list[str],
    verbose: bool,
) -> dict[str, dict[str, Any]]:
    """One pass over the cooked shader region for every base Material ID."""
    require(cache_path.is_file(),
            f"pinned RefShaderCache is missing: {cache_path}")
    cache = package_tables(cache_path)
    layout = parse_shader_code_layout(cache)
    reader = cache["reader"]
    names = cache["names"]
    summary = cache["summary"]
    start = layout["materialMapScanStartLogicalOffset"]
    if verbose:
        print(f"  materializing cooked region from {start}", flush=True)
    region = reader.read_logical_range(start, reader.logical_size - start)

    scans: dict[str, dict[str, Any]] = {}
    for base_id in sorted(set(base_material_ids)):
        needle = bytes.fromhex(base_id)
        offsets = []
        cursor = 0
        while True:
            found = region.find(needle, cursor)
            if found < 0:
                break
            offsets.append(found)
            cursor = found + 1

        contexts = []
        for local in offsets:
            candidate = region[local: local + 8192]
            try:
                static_set = parse_static_parameter_set(candidate, 0, names)
            except Exception:
                continue
            if static_set["baseMaterialIdHex"] != base_id:
                continue
            if static_set["endOffset"] + 20 > len(candidate):
                continue
            suffix = struct.unpack_from(
                "<IIIII", candidate, static_set["endOffset"])
            absolute = start + local
            if not (suffix[0] == summary.version
                    and suffix[1] == summary.licensee_version
                    and suffix[3] == 0
                    and 0 < suffix[4] <= 64
                    and absolute < suffix[2] <= reader.logical_size):
                continue
            equality = engine_equivalent_static_parameter_set(static_set)
            contexts.append({
                "logicalOffset": absolute,
                "logicalEndOffset": suffix[2],
                "vertexFactoryCount": suffix[4],
                "staticSwitchCount": len(
                    static_set.get("staticSwitchParameters") or []),
                "staticParameterSetRawSha256": static_set["rawSha256"],
                "engineEqualityStaticParameterSetSha256":
                    canonical_json_sha256(equality),
            })
        contexts.sort(key=lambda row: row["logicalOffset"])
        scans[base_id] = {
            "rawHitCount": len(offsets),
            "materialMapContextCount": len(contexts),
            "materialMapContexts": contexts,
        }
    return scans


def build_index(
    source_pack_manifest: Path,
    cache_path: Path,
    verbose: bool,
) -> dict[str, Any]:
    source_pack_identity = raw_file_identity(
        source_pack_manifest, "source pack manifest")
    cache_identity = raw_file_identity(
        cache_path, "pinned RefShaderCache")
    parents = collect_parent_materials(AUTHORED_DIRECTORY)
    package_index = load_package_index(source_pack_manifest)
    if verbose:
        print(f"parents={len(parents)} packages={len(package_index)}",
              flush=True)
    started = time.time()
    resolutions, resolution_counts = resolve_parents(
        parents, package_index, verbose)
    if verbose:
        print(f"resolved parents in {time.time() - started:.1f}s", flush=True)

    base_ids = [
        row["baseMaterialIdHex"] for row in resolutions.values()
        if row.get("baseMaterialIdHex")
    ]
    started = time.time()
    scans = scan_cache(cache_path, base_ids, verbose)
    if verbose:
        print(f"scanned cache in {time.time() - started:.1f}s", flush=True)

    families = []
    for parent in sorted(parents):
        resolution = resolutions[parent]
        base_id = resolution.get("baseMaterialIdHex")
        scan = scans.get(base_id) if base_id else None
        if scan is None:
            evidence = "BASE_MATERIAL_ID_UNRESOLVED"
        elif scan["materialMapContextCount"] > 0:
            evidence = "COOKED_MATERIAL_MAPS_PRESENT"
        elif scan["rawHitCount"] > 0:
            evidence = "BASE_ID_HIT_WITHOUT_PARSEABLE_MAP"
        else:
            evidence = "BASE_ID_ABSENT_FROM_CACHE"
        families.append({
            "familyId": "family-" + hashlib.sha256(
                parent.encode("utf-8")).hexdigest()[:16],
            "parentMaterialPath": parent,
            "occurrenceCount": parents[parent]["occurrenceCount"],
            "classes": dict(sorted(parents[parent]["classes"].items())),
            "resolution": resolution,
            "cookedEvidence": evidence,
            "cacheScan": scan,
        })
    families.sort(
        key=lambda row: (-row["occurrenceCount"], row["parentMaterialPath"]))
    for family in families:
        family["rowSha256"] = canonical_sha256(family)

    evidence_counts = collections.Counter(
        row["cookedEvidence"] for row in families)
    index = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "identity": {
            "scope": "AUTHORED_CORPUS_PARENT_MATERIALS",
            "admits": "NOTHING",
            "note": (
                "A located material map is evidence that the cooked program "
                "exists, not that a permutation, vertex factory, pass or "
                "runtime binding has been selected."
            ),
        },
        "inputs": {
            "authoredDirectory": "Data/Effects/Authored",
            "sourcePackManifest": str(source_pack_manifest),
            "sourcePackManifestRawSha256":
                source_pack_identity["rawSha256"],
            "sourcePackManifestByteSize":
                source_pack_identity["byteSize"],
            "refShaderCacheFileName": cache_path.name,
            "refShaderCacheRawSha256": cache_identity["rawSha256"],
            "refShaderCacheByteSize": cache_identity["byteSize"],
            "packagesIndexed": len(package_index),
        },
        "summary": {
            "parentMaterialCount": len(families),
            "occurrenceCount": sum(
                row["occurrenceCount"] for row in families),
            "resolutionCounts": resolution_counts,
            "distinctBaseMaterialIdCount": len(set(base_ids)),
            "cookedEvidenceCounts": dict(sorted(evidence_counts.items())),
            "materialMapContextCount": sum(
                row["cacheScan"]["materialMapContextCount"]
                for row in families if row["cacheScan"]),
        },
        "families": families,
    }
    index["artifactSha256"] = canonical_sha256(index)
    return index


def write_index(path: Path, index: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    with temporary.open("w", encoding="utf-8", newline="\n") as stream:
        stream.write(json.dumps(index, indent=2, ensure_ascii=False) + "\n")
    temporary.replace(path)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-pack-manifest", type=Path,
                        default=DEFAULT_SOURCE_PACK_MANIFEST)
    parser.add_argument("--cache", type=Path, default=DEFAULT_CACHE)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--quiet", action="store_true")
    parser.add_argument(
        "--check",
        action="store_true",
        help="Compare deterministic output with the existing index.",
    )
    arguments = parser.parse_args(argv)
    try:
        index = build_index(
            arguments.source_pack_manifest.resolve(),
            arguments.cache.resolve(),
            not arguments.quiet,
        )
    except IndexError_ as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1

    if arguments.check:
        if not arguments.output.is_file():
            print(f"FAIL: index is missing: {arguments.output}",
                  file=sys.stderr)
            return 1
        if read_json(arguments.output) != index:
            print("FAIL: generated index differs from the checked-in file",
                  file=sys.stderr)
            return 1
        print(f"PASS: {arguments.output}")
    else:
        write_index(arguments.output, index)
        print(f"WROTE: {arguments.output}")

    summary = index["summary"]
    print(
        "RESULT: "
        f"parents={summary['parentMaterialCount']} "
        f"baseIds={summary['distinctBaseMaterialIdCount']} "
        f"mapContexts={summary['materialMapContextCount']} "
        f"evidence={summary['cookedEvidenceCounts']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
