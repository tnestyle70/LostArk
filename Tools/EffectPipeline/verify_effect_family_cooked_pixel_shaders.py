#!/usr/bin/env python3
"""Verify the checked Effect-family cooked shader evidence chain.

This is intentionally a cheap gate: it does not parse the shader cache or
re-run extraction.  It proves that the checked receipt still names the exact
shader-map artifact, source manifest, RefShaderCache and content-addressed
DXBC bytes that produced it.  Any missing pin, denominator contradiction or
upstream byte drift fails closed.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import json
import re
import sys
from pathlib import Path
from typing import Any

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_RECEIPT = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-cooked-pixel-shaders.v1.json")
DEFAULT_SHADER_MAP_INDEX = (
    REPOSITORY_ROOT
    / "Data/Effects/Contracts/effect-family-shader-map-index.v1.json")
DEFAULT_BLOB_DIRECTORY = REPOSITORY_ROOT / "Data/Effects/CookedShaders"
DEFAULT_SOURCE_PACK_MANIFEST = Path(
    r"C:\Users\user\Desktop\Resource_LostArk\00_SourcePackages"
    r"\Effect_DIMENSIONMASTER_20260803_v3\source_pack_manifest.json"
)
DEFAULT_CACHE = Path(
    r"C:\Users\user\Desktop\Resource_LostArk\01_Extracted\Effect\ARTIST"
    r"\31470_TrackA_20260812\OfficialRefShaderCacheV974"
    r"\EV2LG3OVEH3HGV7THTFFTM7TOKMCC.v974.upk"
)

RECEIPT_SCHEMA = "lostark.effect-family-cooked-pixel-shaders"
SHADER_MAP_SCHEMA = "lostark.effect-family-shader-map-index"
FORMAT_VERSION = 1
EXTRACTED = "EXTRACTED"
BLOCKED = "BLOCKED"
PRESENT = "COOKED_MATERIAL_MAPS_PRESENT"
SHA256_PATTERN = re.compile(r"[0-9a-f]{64}")


class VerificationError(RuntimeError):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise VerificationError(message)


def canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value, sort_keys=True, separators=(",", ":"), ensure_ascii=False)
    return hashlib.sha256(payload.encode("utf-8")).hexdigest()


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


def read_document(
    path: Path,
    description: str,
) -> tuple[dict[str, Any], dict[str, Any]]:
    identity = raw_file_identity(path, description)
    try:
        document = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise VerificationError(
            f"{description} is not valid UTF-8 JSON: {error}") from error
    require(isinstance(document, dict),
            f"{description} root must be an object")
    return document, identity


def require_artifact_identity(
    document: dict[str, Any],
    schema: str,
    description: str,
) -> None:
    require(document.get("schema") == schema,
            f"{description} schema is not supported")
    require(document.get("formatVersion") == FORMAT_VERSION,
            f"{description} formatVersion is not supported")
    artifact_sha = document.get("artifactSha256")
    require(
        isinstance(artifact_sha, str)
        and SHA256_PATTERN.fullmatch(artifact_sha) is not None,
        f"{description} artifactSha256 is missing or malformed",
    )
    payload = dict(document)
    payload.pop("artifactSha256", None)
    require(canonical_sha256(payload) == artifact_sha,
            f"{description} artifactSha256 drifted")


def require_count(value: Any, description: str) -> int:
    require(isinstance(value, int) and not isinstance(value, bool) and value >= 0,
            f"{description} must be a non-negative integer")
    return value


def blocker_key(row: dict[str, Any]) -> str:
    return row.get("blocker", "").split(":")[0].split("(")[0].strip()


def verify_shader_map_summary(index: dict[str, Any]) -> dict[str, dict[str, Any]]:
    families = index.get("families")
    require(isinstance(families, list),
            "shader-map index families must be an array")
    by_parent: dict[str, dict[str, Any]] = {}
    resolution_counts: collections.Counter[str] = collections.Counter()
    evidence_counts: collections.Counter[str] = collections.Counter()
    base_ids: set[str] = set()
    occurrence_count = 0
    context_count = 0
    for offset, row in enumerate(families):
        require(isinstance(row, dict),
                f"shader-map family {offset} must be an object")
        parent = row.get("parentMaterialPath")
        require(isinstance(parent, str) and bool(parent),
                f"shader-map family {offset} has no parentMaterialPath")
        require(parent not in by_parent,
                f"shader-map index duplicates parent material: {parent}")
        by_parent[parent] = row
        occurrence_count += require_count(
            row.get("occurrenceCount"),
            f"shader-map family {parent} occurrenceCount")
        resolution = row.get("resolution")
        require(isinstance(resolution, dict),
                f"shader-map family {parent} has no resolution")
        resolved_by = resolution.get("resolvedBy")
        require(isinstance(resolved_by, str) and bool(resolved_by),
                f"shader-map family {parent} has no resolvedBy")
        resolution_counts[resolved_by] += 1
        base_id = resolution.get("baseMaterialIdHex")
        if base_id is not None:
            require(isinstance(base_id, str) and bool(base_id),
                    f"shader-map family {parent} base ID is malformed")
            base_ids.add(base_id)
        evidence = row.get("cookedEvidence")
        require(isinstance(evidence, str) and bool(evidence),
                f"shader-map family {parent} has no cookedEvidence")
        evidence_counts[evidence] += 1
        scan = row.get("cacheScan")
        if scan is not None:
            require(isinstance(scan, dict),
                    f"shader-map family {parent} cacheScan is malformed")
            context_count += require_count(
                scan.get("materialMapContextCount"),
                f"shader-map family {parent} materialMapContextCount")
        row_sha = row.get("rowSha256")
        require(
            isinstance(row_sha, str)
            and SHA256_PATTERN.fullmatch(row_sha) is not None,
            f"shader-map family {parent} rowSha256 is malformed",
        )
        row_payload = dict(row)
        row_payload.pop("rowSha256", None)
        require(canonical_sha256(row_payload) == row_sha,
                f"shader-map family {parent} rowSha256 drifted")

    summary = index.get("summary")
    require(isinstance(summary, dict),
            "shader-map index summary must be an object")
    expected = {
        "parentMaterialCount": len(families),
        "occurrenceCount": occurrence_count,
        "resolutionCounts": dict(sorted(resolution_counts.items())),
        "distinctBaseMaterialIdCount": len(base_ids),
        "cookedEvidenceCounts": dict(sorted(evidence_counts.items())),
        "materialMapContextCount": context_count,
    }
    for key, value in expected.items():
        require(summary.get(key) == value,
                f"shader-map index summary.{key} is inconsistent")
    return by_parent


def require_pinned_file(
    inputs: dict[str, Any],
    prefix: str,
    path: Path,
    description: str,
) -> dict[str, Any]:
    identity = raw_file_identity(path, description)
    require(inputs.get(f"{prefix}RawSha256") == identity["rawSha256"],
            f"{description} raw SHA-256 drifted")
    require(inputs.get(f"{prefix}ByteSize") == identity["byteSize"],
            f"{description} byte size drifted")
    return identity


def verify_shader_map_inputs(
    index: dict[str, Any],
    source_pack_manifest: Path,
    cache_path: Path,
) -> dict[str, Any]:
    inputs = index.get("inputs")
    require(isinstance(inputs, dict),
            "shader-map index inputs must be an object")
    require_pinned_file(
        inputs, "sourcePackManifest", source_pack_manifest,
        "source pack manifest")
    cache_identity = require_pinned_file(
        inputs, "refShaderCache", cache_path, "pinned RefShaderCache")
    require(inputs.get("refShaderCacheFileName") == cache_path.name,
            "shader-map index cache filename differs from the current cache")
    return cache_identity


def verify_receipt_summary_and_blobs(
    receipt: dict[str, Any],
    shader_map_families: dict[str, dict[str, Any]],
    blob_directory: Path,
) -> None:
    families = receipt.get("families")
    require(isinstance(families, list),
            "cooked receipt families must be an array")
    by_parent: dict[str, dict[str, Any]] = {}
    blocker_counts: collections.Counter[str] = collections.Counter()
    extracted_count = 0
    extracted_occurrences = 0
    for offset, row in enumerate(families):
        require(isinstance(row, dict),
                f"cooked family {offset} must be an object")
        parent = row.get("parentMaterialPath")
        require(isinstance(parent, str) and bool(parent),
                f"cooked family {offset} has no parentMaterialPath")
        require(parent not in by_parent,
                f"cooked receipt duplicates parent material: {parent}")
        by_parent[parent] = row
        map_row = shader_map_families.get(parent)
        require(map_row is not None,
                f"cooked family is absent from shader-map index: {parent}")
        require(map_row.get("cookedEvidence") == PRESENT,
                f"cooked family has no cooked material-map evidence: {parent}")
        occurrence_count = require_count(
            row.get("occurrenceCount"),
            f"cooked family {parent} occurrenceCount")
        require(occurrence_count == map_row.get("occurrenceCount"),
                f"cooked family {parent} occurrenceCount drifted")
        status = row.get("status")
        require(status in {EXTRACTED, BLOCKED},
                f"cooked family {parent} has unknown status: {status}")
        if status == EXTRACTED:
            require("blocker" not in row,
                    f"EXTRACTED family carries a blocker: {parent}")
            dxbc_sha = row.get("dxbcSha256")
            require(
                isinstance(dxbc_sha, str)
                and SHA256_PATTERN.fullmatch(dxbc_sha) is not None,
                f"EXTRACTED family has malformed dxbcSha256: {parent}",
            )
            byte_size = require_count(
                row.get("dxbcByteSize"),
                f"EXTRACTED family {parent} dxbcByteSize")
            require(byte_size > 0,
                    f"EXTRACTED family has an empty DXBC blob: {parent}")
            blob_path = blob_directory / f"{dxbc_sha}.dxbc"
            blob_identity = raw_file_identity(
                blob_path, f"DXBC blob for {parent}")
            require(blob_path.stem == dxbc_sha,
                    f"DXBC blob filename differs from its SHA-256: {parent}")
            require(blob_identity["rawSha256"] == dxbc_sha,
                    f"DXBC blob raw SHA-256 drifted: {parent}")
            require(blob_identity["byteSize"] == byte_size,
                    f"DXBC blob byte size drifted: {parent}")
            with blob_path.open("rb") as stream:
                require(stream.read(4) == b"DXBC",
                        f"DXBC blob has no DXBC signature: {parent}")
            extracted_count += 1
            extracted_occurrences += occurrence_count
        else:
            blocker = row.get("blocker")
            require(isinstance(blocker, str) and bool(blocker.strip()),
                    f"BLOCKED family has no blocker: {parent}")
            require("dxbcSha256" not in row and "dxbcByteSize" not in row,
                    f"BLOCKED family carries extracted blob identity: {parent}")
            blocker_counts[blocker_key(row)] += 1

    expected_parents = {
        parent for parent, row in shader_map_families.items()
        if row.get("cookedEvidence") == PRESENT
    }
    require(set(by_parent) == expected_parents,
            "cooked receipt family denominator differs from shader-map index")
    summary = receipt.get("summary")
    require(isinstance(summary, dict),
            "cooked receipt summary must be an object")
    expected = {
        "familyCount": len(families),
        "extractedCount": extracted_count,
        "blockedCount": len(families) - extracted_count,
        "extractedOccurrenceCount": extracted_occurrences,
        "blockerCounts": dict(sorted(blocker_counts.items())),
    }
    for key, value in expected.items():
        require(summary.get(key) == value,
                f"cooked receipt summary.{key} is inconsistent")


def verify(
    receipt_path: Path,
    shader_map_path: Path,
    source_pack_manifest: Path,
    cache_path: Path,
    blob_directory: Path,
) -> dict[str, int]:
    shader_map, shader_map_identity = read_document(
        shader_map_path, "shader-map index")
    require_artifact_identity(
        shader_map, SHADER_MAP_SCHEMA, "shader-map index")
    shader_map_families = verify_shader_map_summary(shader_map)
    map_cache_identity = verify_shader_map_inputs(
        shader_map, source_pack_manifest, cache_path)

    receipt, _ = read_document(receipt_path, "cooked receipt")
    require_artifact_identity(receipt, RECEIPT_SCHEMA, "cooked receipt")
    inputs = receipt.get("inputs")
    require(isinstance(inputs, dict),
            "cooked receipt inputs must be an object")
    require(
        inputs.get("shaderMapArtifactSha256")
        == shader_map.get("artifactSha256"),
        "cooked receipt shader-map artifact pin drifted",
    )
    require(
        inputs.get("shaderMapRawSha256")
        == shader_map_identity["rawSha256"],
        "cooked receipt shader-map raw SHA-256 drifted",
    )
    cache_identity = require_pinned_file(
        inputs, "refShaderCache", cache_path, "cooked receipt RefShaderCache")
    require(inputs.get("refShaderCacheFileName") == cache_path.name,
            "cooked receipt cache filename differs from the current cache")
    require(cache_identity == map_cache_identity,
            "cooked receipt and shader-map index pin different caches")
    verify_receipt_summary_and_blobs(
        receipt, shader_map_families, blob_directory)
    summary = receipt["summary"]
    return {
        "familyCount": summary["familyCount"],
        "extractedCount": summary["extractedCount"],
        "blockedCount": summary["blockedCount"],
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--receipt", type=Path, default=DEFAULT_RECEIPT)
    parser.add_argument("--shader-map-index", type=Path,
                        default=DEFAULT_SHADER_MAP_INDEX)
    parser.add_argument("--source-pack-manifest", type=Path,
                        default=DEFAULT_SOURCE_PACK_MANIFEST)
    parser.add_argument("--cache", type=Path, default=DEFAULT_CACHE)
    parser.add_argument("--blob-directory", type=Path,
                        default=DEFAULT_BLOB_DIRECTORY)
    arguments = parser.parse_args(argv)
    try:
        result = verify(
            arguments.receipt.resolve(),
            arguments.shader_map_index.resolve(),
            arguments.source_pack_manifest.resolve(),
            arguments.cache.resolve(),
            arguments.blob_directory.resolve(),
        )
    except VerificationError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print(
        "PASS: cooked Effect-family shader evidence "
        f"families={result['familyCount']} "
        f"extracted={result['extractedCount']} "
        f"blocked={result['blockedCount']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
