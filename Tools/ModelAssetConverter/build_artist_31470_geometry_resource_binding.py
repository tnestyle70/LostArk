#!/usr/bin/env python3
"""Cook, bind, and transactionally deploy the seven Artist 31470 WModels."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import struct
import subprocess
import uuid
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Callable

from cook_wmodel_geometry_contract import (
    canonical_json_bytes,
    canonical_lf_utf8_bytes,
    cook_wmodel_geometry_contract,
    load_geometry_provenance_evidence,
    load_json_object,
    load_tracked_json_object,
    parse_geometry_wmodel,
    sha256_file,
    write_atomic,
)
from verify_artist_31470_wmodel_geometry_contract import ASSETS, strict_json_equal


BINDING_SCHEMA = "lostark.effect-geometry-binding"
BINDING_FORMAT_VERSION = 1
RECEIPT_SCHEMA = "lostark.artist-31470-geometry-resource-binding-receipt"
RECEIPT_FORMAT_VERSION = 1
CHARACTER_CLASS = "Artist"
SKILL_ID = 31470
G02_APPROVED_COMMIT = "2b3d7a6c410f963b2e47aa7999504c422fff7c32"
G02_APPROVED_TREE_SHA = "1b217af4a159e69c95daa4b71f4de86b2b8ded18"
G02_TREE_EQUIVALENT_BASE_COMMIT = "513a2dde5ae317cab8fee18777397d887075e5c5"
G02_REQUIRED_BLOB_PATHS = (
    "Data/Effects/Imported/Artist/Artist.resource-source-manifest.json",
    "Tools/ModelAssetConverter/cook_wmodel_geometry_contract.py",
    "Tools/ModelAssetConverter/verify_artist_31470_wmodel_geometry_contract.py",
    "Tools/WModelGeometryContractHarness/Fixtures/artist_31470_v11_expected.json",
)
ASSET_PREFIX = "Effect/Artist/Meshes/"
EXPECTED_FORMAT_VERSION = "1.1"
EXPECTED_SOURCE_TO_WMODEL_SCALE = 100.0
EXPECTED_GEOMETRY_PRE_SCALE = 0.01
EXPECTED_GEOMETRY_PRE_SCALE_F32_HEX = "3c23d70a"
PRODUCT_BLOCKERS = (
    "RUNTIME_GEOMETRY_PRESCALE_NOT_CONSUMED",
    "PRODUCT_ADMISSION_NOT_OPEN",
)
TYPED_BINDING_ASSET_ID = (
    "Data/Effects/Imported/Artist/Geometry/skill.31470.geometry-binding.json"
)
GEOMETRY_PRE_SCALE_JSON_TYPE_POLICY = "JSON_FLOAT_ONLY"
TARGET_BASENAMES = tuple(f"{name}.wmodel" for _, name in ASSETS)


class BindingError(ValueError):
    """Raised when an evidence or deployment boundary is not satisfied."""


def require(condition: bool, message: str) -> None:
    if not condition:
        raise BindingError(message)


def is_exact_json_integer(value: Any, expected: int) -> bool:
    return type(value) is int and value == expected


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def reject_duplicate_json_object(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        require(key not in result, f"duplicate JSON key: {key}")
        result[key] = value
    return result


def load_strict_json_object(path: Path, label: str) -> dict[str, Any]:
    require(path.is_file(), f"{label} is missing: {path}")
    payload = path.read_bytes()
    require(not payload.startswith(b"\xef\xbb\xbf"), f"{label} may not contain a UTF-8 BOM")
    try:
        value = json.loads(
            payload.decode("utf-8"), object_pairs_hook=reject_duplicate_json_object
        )
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise BindingError(f"{label} is not strict UTF-8 JSON") from error
    require(isinstance(value, dict), f"{label} root must be an object")
    return value


def canonical_tracked_sha256(path: Path, label: str) -> str:
    return sha256_bytes(canonical_lf_utf8_bytes(path.read_bytes(), label))


def raw_file_identity(path: Path) -> dict[str, Any]:
    require(path.is_file(), f"required file is missing: {path}")
    return {
        "byteSize": path.stat().st_size,
        "sha256": sha256_file(path).hex(),
    }


def f32_hex(value: float) -> str:
    return struct.pack(">f", value).hex()


def require_exact_geometry_pre_scale(value: Any, label: str) -> None:
    require(
        type(value) is float
        and math.isfinite(value)
        and f32_hex(value) == EXPECTED_GEOMETRY_PRE_SCALE_F32_HEX,
        f"{label} must be a JSON float equal to float32(0.01)",
    )


def git_output(repository_root: Path, *arguments: str, binary: bool = False) -> bytes | str:
    completed = subprocess.run(
        ["git", "-C", str(repository_root.resolve()), *arguments],
        check=False,
        capture_output=True,
    )
    require(
        completed.returncode == 0,
        "git evidence query failed: "
        + " ".join(arguments)
        + " "
        + completed.stderr.decode("utf-8", "replace").strip(),
    )
    if binary:
        return completed.stdout
    return completed.stdout.decode("utf-8", "strict").strip()


def validate_g02_approved_tree_equivalence(repository_root: Path) -> dict[str, Any]:
    approved_tree = str(
        git_output(repository_root, "show", "-s", "--format=%T", G02_APPROVED_COMMIT)
    )
    equivalent_tree = str(
        git_output(
            repository_root,
            "show",
            "-s",
            "--format=%T",
            G02_TREE_EQUIVALENT_BASE_COMMIT,
        )
    )
    require(
        approved_tree == G02_APPROVED_TREE_SHA
        and equivalent_tree == G02_APPROVED_TREE_SHA,
        "G02 approved/equivalent commit tree identity differs",
    )
    blobs: list[dict[str, Any]] = []
    for asset_path in G02_REQUIRED_BLOB_PATHS:
        approved_bytes = bytes(
            git_output(
                repository_root,
                "show",
                f"{G02_APPROVED_COMMIT}:{asset_path}",
                binary=True,
            )
        )
        equivalent_bytes = bytes(
            git_output(
                repository_root,
                "show",
                f"{G02_TREE_EQUIVALENT_BASE_COMMIT}:{asset_path}",
                binary=True,
            )
        )
        current_path = repository_root / Path(asset_path)
        require(current_path.is_file(), f"G02 required blob is missing: {asset_path}")
        approved_canonical = canonical_lf_utf8_bytes(
            approved_bytes, f"approved G02 blob {asset_path}"
        )
        equivalent_canonical = canonical_lf_utf8_bytes(
            equivalent_bytes, f"tree-equivalent G02 blob {asset_path}"
        )
        current_canonical = canonical_lf_utf8_bytes(
            current_path.read_bytes(), f"current G02 blob {asset_path}"
        )
        require(
            approved_canonical == equivalent_canonical == current_canonical,
            f"current required blob is not G02 tree-equivalent: {asset_path}",
        )
        blobs.append(
            {
                "assetId": asset_path,
                "canonicalLfSha256": sha256_bytes(approved_canonical),
                "fidelity": "APPROVED_COMMIT_TREE_BLOB_EQUIVALENT",
            }
        )
    return {
        "relationship": (
            "APPROVED_COMMIT_TREE_PIN_AND_REQUIRED_BLOB_EQUIVALENCE_"
            "NOT_GRAPH_ANCESTRY"
        ),
        "approvedCommit": G02_APPROVED_COMMIT,
        "approvedTreeSha": G02_APPROVED_TREE_SHA,
        "treeEquivalentBaseCommit": G02_TREE_EQUIVALENT_BASE_COMMIT,
        "treeEquivalentBaseTreeSha": G02_APPROVED_TREE_SHA,
        "graphAncestryClaimed": False,
        "requiredBlobCount": len(blobs),
        "requiredBlobs": blobs,
    }


def asset_name(package: str, name: str) -> str:
    del package
    return name


def source_object(package: str, name: str) -> str:
    return f"{package.casefold()}.{name}"


def asset_id(package: str, name: str) -> str:
    del package
    return f"{ASSET_PREFIX}{name}.wmodel"


def asset_relative_path(value: str) -> Path:
    require(value.startswith(ASSET_PREFIX), f"unexpected geometry asset ID: {value}")
    suffix = value[len(ASSET_PREFIX) :]
    require(
        suffix and "/" not in suffix and "\\" not in suffix and suffix.endswith(".wmodel"),
        f"geometry asset ID does not name one WModel: {value}",
    )
    path = Path(value.replace("/", os.sep))
    require(not path.is_absolute() and ".." not in path.parts, f"unsafe asset ID: {value}")
    return path


def scan_exact_target_basenames(root: Path, label: str) -> dict[str, Path]:
    resolved = root.resolve()
    require(resolved.is_dir(), f"{label} root is missing: {resolved}")
    expected_by_fold: dict[str, str] = {}
    for name in TARGET_BASENAMES:
        folded = name.casefold()
        require(folded not in expected_by_fold, f"expected target case-fold collision: {name}")
        expected_by_fold[folded] = name

    matches: dict[str, Path] = {}
    matched_casefolds: set[str] = set()
    with os.scandir(resolved) as entries:
        for entry in entries:
            folded = entry.name.casefold()
            expected = expected_by_fold.get(folded)
            if expected is None:
                continue
            require(
                folded not in matched_casefolds,
                f"{label} has a case-fold alias collision for {expected}",
            )
            matched_casefolds.add(folded)
            require(
                entry.name == expected,
                f"{label} target basename is not ordinal-exact: {entry.name} != {expected}",
            )
            require(
                entry.is_file(follow_symlinks=False) and not entry.is_symlink(),
                f"{label} target is not a regular non-symlink file: {entry.name}",
            )
            matches[expected] = Path(entry.path).resolve()
    require(
        set(matches) == set(TARGET_BASENAMES) and len(matches) == len(TARGET_BASENAMES),
        f"{label} must expose the ordinal-exact seven target basenames",
    )
    return matches


def load_expected_semantics(path: Path) -> tuple[dict[str, Any], dict[str, dict[str, Any]], str]:
    document, canonical_sha = load_tracked_json_object(
        path, "Artist 31470 immutable geometry semantics"
    )
    require(
        document.get("schema")
        == "lostark.artist-31470-wmodel-decoded-semantic-golden"
        and is_exact_json_integer(document.get("formatVersion"), 1)
        and document.get("characterClass") == CHARACTER_CLASS
        and is_exact_json_integer(document.get("skillId"), SKILL_ID)
        and document.get("productAdmission") is False,
        "Artist 31470 immutable geometry semantics root is unsupported",
    )
    rows = document.get("assets")
    require(isinstance(rows, list) and len(rows) == len(ASSETS), "expected seven geometry rows")
    by_source: dict[str, dict[str, Any]] = {}
    for row in rows:
        require(isinstance(row, dict), "geometry semantic row is not an object")
        key = str(row.get("sourceObject", ""))
        require(key and key not in by_source, f"duplicate geometry semantic source: {key}")
        by_source[key] = row
    expected_order = [source_object(package, name) for package, name in ASSETS]
    require(
        [str(row.get("sourceObject", "")) for row in rows] == expected_order,
        "geometry semantic rows are not in the canonical seven-carrier order",
    )
    return document, by_source, canonical_sha.hex()


def load_source_manifest_package_map(path: Path) -> dict[str, str]:
    manifest, _ = load_tracked_json_object(path, "Artist resource source manifest")
    require(
        manifest.get("schema") == "lostark.class-effect-resource-source-manifest"
        and is_exact_json_integer(manifest.get("formatVersion"), 1)
        and str(manifest.get("characterClass", "")).casefold()
        == CHARACTER_CLASS.casefold(),
        "Artist source manifest root is unsupported",
    )
    by_source: dict[str, str] = {}
    for row in manifest.get("assets") or []:
        if not isinstance(row, dict):
            continue
        key = str(row.get("sourceAssetPath", ""))
        if key not in {source_object(package, name) for package, name in ASSETS}:
            continue
        require(key not in by_source, f"duplicate Artist source manifest row: {key}")
        require(
            row.get("resolutionStatus") == "RESOLVED_SOURCE_PACKAGE",
            f"Artist source package is unresolved: {key}",
        )
        physical = str(row.get("physicalPackage", ""))
        require(physical.endswith(".upk") and Path(physical).name == physical, f"unsafe package: {physical}")
        by_source[key] = physical
    require(len(by_source) == len(ASSETS), "Artist source manifest does not resolve seven carriers")
    return by_source


def unique_evidence_row(
    rows: Any, predicate: Callable[[dict[str, Any]], bool], label: str
) -> dict[str, Any]:
    require(isinstance(rows, list), f"{label} container is not an array")
    matches = [row for row in rows if isinstance(row, dict) and predicate(row)]
    require(len(matches) == 1, f"{label} must resolve exactly one row")
    return matches[0]


def sealed_row(row: dict[str, Any]) -> dict[str, Any]:
    return {
        "row": row,
        "rowSha256": sha256_bytes(canonical_json_bytes(row)),
    }


def resolve_ordinal_named_file(root: Path, expected_name: str, label: str) -> Path:
    resolved = root.resolve()
    require(resolved.is_dir(), f"{label} root is missing: {resolved}")
    matches: list[os.DirEntry[str]] = []
    with os.scandir(resolved) as entries:
        for entry in entries:
            if entry.name.casefold() == expected_name.casefold():
                matches.append(entry)
    require(len(matches) == 1, f"{label} must resolve one case-fold-unique file")
    entry = matches[0]
    require(entry.name == expected_name, f"{label} basename is not ordinal-exact")
    require(
        entry.is_file(follow_symlinks=False) and not entry.is_symlink(),
        f"{label} is not a regular non-symlink file",
    )
    return Path(entry.path)


def collect_external_evidence(
    source_root: Path,
    legacy_mesh_root: Path,
    source_manifest_path: Path,
    source_export_receipt_path: Path,
    legacy_cook_receipt_path: Path,
    source_package_root: Path,
    legacy_converter_path: Path,
    expected_semantics_path: Path,
) -> dict[str, Any]:
    repository_root = Path(__file__).resolve().parents[2]
    approval = validate_g02_approved_tree_equivalence(repository_root)
    manifest = load_strict_json_object(source_manifest_path, "Artist source manifest")
    export_receipt = load_strict_json_object(
        source_export_receipt_path, "Artist source export receipt"
    )
    cook_receipt = load_strict_json_object(
        legacy_cook_receipt_path, "Artist legacy cook receipt"
    )
    require(
        manifest.get("schema") == "lostark.class-effect-resource-source-manifest"
        and is_exact_json_integer(manifest.get("formatVersion"), 1)
        and str(manifest.get("characterClass", "")).casefold()
        == CHARACTER_CLASS.casefold(),
        "Artist source manifest root is unsupported",
    )
    require(
        export_receipt.get("schema") == "lostark.effect-resource-export-receipt"
        and is_exact_json_integer(export_receipt.get("formatVersion"), 1)
        and export_receipt.get("characterClass") == "ARTIST",
        "Artist source export receipt root is unsupported",
    )
    require(
        cook_receipt.get("schema") == "lostark.effect-runtime-resource-cook-receipt"
        and is_exact_json_integer(cook_receipt.get("formatVersion"), 1)
        and cook_receipt.get("characterClass") == "ARTIST"
        and type(cook_receipt.get("scale")) is float
        and cook_receipt.get("scale") == EXPECTED_SOURCE_TO_WMODEL_SCALE,
        "Artist legacy cook receipt root is unsupported",
    )
    manifest_canonical = canonical_lf_utf8_bytes(
        source_manifest_path.read_bytes(), "Artist source manifest"
    )
    manifest_receipt_sha = str(export_receipt.get("sourceManifestSha256", ""))
    manifest_lf_sha = sha256_bytes(manifest_canonical)
    manifest_crlf_sha = sha256_bytes(manifest_canonical.replace(b"\n", b"\r\n"))
    require(
        manifest_receipt_sha in (manifest_lf_sha, manifest_crlf_sha),
        "source export receipt does not correlate the approved source manifest",
    )
    export_raw = raw_file_identity(source_export_receipt_path)
    cook_raw = raw_file_identity(legacy_cook_receipt_path)
    require(
        cook_receipt.get("sourceExportReceiptSha256") == export_raw["sha256"],
        "legacy cook receipt does not pin the supplied export receipt raw bytes",
    )
    require(
        legacy_converter_path.is_file() and not legacy_converter_path.is_symlink(),
        "legacy converter is missing or is a symlink",
    )
    converter_identity = raw_file_identity(legacy_converter_path)
    _, expected_by_source, _ = load_expected_semantics(expected_semantics_path)
    legacy_targets = scan_exact_target_basenames(
        legacy_mesh_root, "legacy geometry resource root"
    )
    assets: dict[str, dict[str, Any]] = {}
    for package, name in ASSETS:
        key = source_object(package, name)
        expected = expected_by_source[key]
        manifest_row = unique_evidence_row(
            manifest.get("assets"),
            lambda row, source_key=key: row.get("sourceAssetPath") == source_key,
            f"source manifest row {key}",
        )
        require(
            manifest_row.get("logicalPackage") == package.casefold()
            and manifest_row.get("physicalPackage")
            and manifest_row.get("resolutionStatus") == "RESOLVED_SOURCE_PACKAGE"
            and manifest_row.get("roles") == ["mesh"]
            and SKILL_ID in (manifest_row.get("skillIds") or []),
            f"source manifest row is not the expected Artist mesh: {key}",
        )
        physical_package_name = str(manifest_row["physicalPackage"])
        require(
            Path(physical_package_name).name == physical_package_name
            and physical_package_name.endswith(".upk"),
            f"source package name is unsafe: {key}",
        )
        source_package = resolve_ordinal_named_file(
            source_package_root,
            physical_package_name,
            f"source package {key}",
        )

        export_row = unique_evidence_row(
            export_receipt.get("assets"),
            lambda row, source_key=key: row.get("sourceAssetPath") == source_key,
            f"source export receipt row {key}",
        )
        require(
            export_row.get("objectName") == name
            and export_row.get("roles") == ["mesh"]
            and isinstance(export_row.get("skillIds"), list)
            and SKILL_ID in export_row["skillIds"]
            and all(type(value) is int for value in export_row["skillIds"])
            and export_row.get("logicalPackage") == package.casefold()
            and export_row.get("resolutionStatus") == "EXPORTED",
            f"source export receipt row differs: {key}",
        )
        source_gltf = source_root / package / "StaticMesh3" / f"{name}.gltf"
        expected_gltf_relative = f"{package}/StaticMesh3/{name}.gltf"
        gltf_row = unique_evidence_row(
            export_row.get("outputs"),
            lambda row, relative=expected_gltf_relative: row.get("relativePath")
            == relative,
            f"source glTF output row {key}",
        )
        gltf_identity = raw_file_identity(source_gltf)
        require(
            type(gltf_row.get("byteSize")) is int
            and gltf_row.get("byteSize") == gltf_identity["byteSize"]
            and gltf_row.get("sha256") == gltf_identity["sha256"]
            and gltf_identity["sha256"] == expected.get("sourceGltfSha256"),
            f"source glTF bytes/row/G02 golden differ: {key}",
        )
        gltf_document = load_strict_json_object(source_gltf, f"source glTF {key}")
        buffer_evidence: list[dict[str, Any]] = []
        for buffer in gltf_document.get("buffers") or []:
            require(isinstance(buffer, dict), f"source glTF buffer is not an object: {key}")
            uri = str(buffer.get("uri", ""))
            uri_path = Path(uri)
            require(
                uri
                and not uri.startswith("data:")
                and not uri_path.is_absolute()
                and ".." not in uri_path.parts,
                f"source glTF buffer URI is unsafe: {key}",
            )
            buffer_path = source_gltf.parent / uri_path
            buffer_relative = f"{package}/StaticMesh3/{uri_path.as_posix()}"
            buffer_row = unique_evidence_row(
                export_row.get("outputs"),
                lambda row, relative=buffer_relative: row.get("relativePath")
                == relative,
                f"source buffer output row {key}/{uri}",
            )
            buffer_identity = raw_file_identity(buffer_path)
            require(
                type(buffer_row.get("byteSize")) is int
                and buffer_row.get("byteSize") == buffer_identity["byteSize"]
                and buffer_row.get("sha256") == buffer_identity["sha256"],
                f"source buffer bytes/row differ: {key}/{uri}",
            )
            buffer_evidence.append(
                {
                    **sealed_row(buffer_row),
                    "rawBytes": buffer_identity,
                }
            )

        cook_row = unique_evidence_row(
            cook_receipt.get("assets"),
            lambda row, source_key=key: row.get("sourceAssetPath") == source_key
            and row.get("role") == "mesh",
            f"legacy cook receipt row {key}",
        )
        legacy_path = legacy_targets[f"{name}.wmodel"]
        legacy_identity = raw_file_identity(legacy_path)
        require(
            cook_row.get("sourceFile") == expected_gltf_relative
            and cook_row.get("runtimeAssetId") == asset_id(package, name)
            and type(cook_row.get("converterExitCode")) is int
            and cook_row.get("converterExitCode") == 0
            and cook_row.get("status") == "COOKED"
            and type(cook_row.get("byteSize")) is int
            and cook_row.get("byteSize") == legacy_identity["byteSize"]
            and cook_row.get("sha256") == legacy_identity["sha256"]
            and legacy_identity["sha256"] == expected.get("legacyWModelSha256"),
            f"legacy WModel bytes/row/G02 golden differ: {key}",
        )
        assets[key] = {
            "manifestRow": sealed_row(manifest_row),
            "sourceExportRow": sealed_row(export_row),
            "sourceGltfOutputRow": {
                **sealed_row(gltf_row),
                "rawBytes": gltf_identity,
            },
            "sourceBufferOutputRows": buffer_evidence,
            "legacyCookRow": sealed_row(cook_row),
            "legacyResourceRawBytes": legacy_identity,
            "sourcePackageObserved": {
                "basename": physical_package_name,
                **raw_file_identity(source_package),
                "fidelity": "OBSERVED_UNVERIFIED",
            },
        }

    return {
        "approval": approval,
        "inputs": {
            "sourceManifest": {
                "assetId": G02_REQUIRED_BLOB_PATHS[0],
                "hashRole": "TRACKED_CANONICAL_LF",
                "canonicalLfSha256": manifest_lf_sha,
                "legacyReceiptCorrelation": (
                    "CANONICAL_LF"
                    if manifest_receipt_sha == manifest_lf_sha
                    else "CANONICAL_CRLF_VARIANT"
                ),
                "fidelity": "APPROVED_COMMIT_TREE_BLOB_EQUIVALENT",
            },
            "sourceExportReceipt": {
                "hashRole": "EXTERNAL_RAW_BYTES",
                **export_raw,
                "fidelity": "OBSERVED_UNVERIFIED",
            },
            "legacyCookReceipt": {
                "hashRole": "EXTERNAL_RAW_BYTES",
                **cook_raw,
                "fidelity": "OBSERVED_UNVERIFIED",
            },
            "legacyConverter": {
                "hashRole": "EXTERNAL_RAW_BYTES",
                **converter_identity,
                "fidelity": "OBSERVED_UNVERIFIED",
            },
        },
        "assets": assets,
        "unverifiedExternalAuthorityCount": 3 + len(assets),
    }


def decoded_tuple_from_golden(expected_row: dict[str, Any]) -> dict[str, Any]:
    decoded = expected_row.get("decodedSemantics")
    require(isinstance(decoded, dict), "expected decoded semantics is missing")
    require(
        decoded.get("schema") == "lostark.wmodel-decoded-semantic-dump"
        and is_exact_json_integer(decoded.get("formatVersion"), 1),
        "expected decoded semantic schema/version is unsupported",
    )
    meshes = decoded.get("meshes")
    require(isinstance(meshes, list) and meshes, "expected decoded semantics has no meshes")
    submeshes: list[dict[str, Any]] = []
    for mesh in meshes:
        require(isinstance(mesh, dict), "expected submesh is not an object")
        vertex_count = mesh.get("vertexCount")
        index_count = mesh.get("indexCount")
        has_color0 = mesh.get("hasColor0")
        require(
            type(vertex_count) is int
            and vertex_count > 0
            and type(index_count) is int
            and index_count > 0
            and type(has_color0) is bool,
            "expected submesh count/channel state is invalid",
        )
        bounds = mesh.get("boundsF32Hex")
        require(
            isinstance(bounds, list)
            and len(bounds) == 10
            and all(isinstance(value, str) and len(value) == 8 for value in bounds),
            "expected submesh bounds are invalid",
        )
        submeshes.append(
            {
                "name": str(mesh.get("name", "")),
                "materialIndex": mesh.get("materialIndex"),
                "vertexCount": vertex_count,
                "indexCount": index_count,
                "channelCounts": {
                    "position": vertex_count,
                    "normal": vertex_count,
                    "tangentXyz": vertex_count,
                    "tangentW": vertex_count,
                    "uv0": vertex_count,
                    "color0": vertex_count if has_color0 else 0,
                },
                "channelSha256": {
                    "position": mesh.get("positionSha256"),
                    "normal": mesh.get("normalSha256"),
                    "tangentXyz": mesh.get("tangentXyzSha256"),
                    "tangentW": mesh.get("tangentWSha256"),
                    "uv0": mesh.get("uv0Sha256"),
                    "color0": mesh.get("color0Sha256"),
                    "indicesU32": mesh.get("indexU32Sha256"),
                },
                "boundsF32Hex": bounds,
            }
        )
    metadata_identity = str(decoded.get("metadataIdentitySha256", ""))
    require(
        type(decoded.get("channelMask")) is int
        and type(decoded.get("evidenceFlags")) is int,
        "expected channel/evidence masks must be exact JSON integers",
    )
    return {
        "formatVersion": EXPECTED_FORMAT_VERSION,
        "channelMask": decoded.get("channelMask"),
        "evidenceFlags": decoded.get("evidenceFlags"),
        "geometryPreScale": EXPECTED_GEOMETRY_PRE_SCALE,
        "geometryPreScaleF32Hex": decoded.get("geometryPreScaleF32Hex"),
        "payloadSha256": decoded.get("payloadSha256"),
        "provenanceSha256": metadata_identity,
        "provenanceRole": "WMODEL_METADATA_IDENTITY_SHA256",
        "metadataIdentitySha256": metadata_identity,
        "submeshes": submeshes,
    }


def validate_runtime_against_expected(
    runtime: dict[str, Any], expected_row: dict[str, Any], candidate_bytes: bytes
) -> dict[str, Any]:
    expected_tuple = decoded_tuple_from_golden(expected_row)
    require(
        runtime.get("vertexFlags") == expected_tuple["channelMask"]
        and runtime.get("evidenceFlags") == expected_tuple["evidenceFlags"]
        and f32_hex(float(runtime.get("geometryPreScale", math.nan)))
        == expected_tuple["geometryPreScaleF32Hex"]
        and bytes(runtime.get("payloadSha256", b"")).hex()
        == expected_tuple["payloadSha256"]
        and bytes(runtime.get("metadataIdentitySha256", b"")).hex()
        == expected_tuple["metadataIdentitySha256"],
        "candidate metadata does not match immutable expected tuple",
    )
    require(
        expected_tuple["geometryPreScaleF32Hex"] == EXPECTED_GEOMETRY_PRE_SCALE_F32_HEX,
        "immutable geometry pre-scale is not float32(0.01)",
    )
    require(
        sha256_bytes(candidate_bytes) == str(expected_row.get("candidateWModelSha256", "")),
        "candidate WModel SHA-256 differs from immutable G02 golden",
    )
    runtime_meshes = runtime.get("submeshes")
    expected_meshes = expected_tuple["submeshes"]
    require(
        isinstance(runtime_meshes, list) and len(runtime_meshes) == len(expected_meshes),
        "candidate submesh count differs from immutable expected tuple",
    )
    for runtime_mesh, expected_mesh in zip(runtime_meshes, expected_meshes):
        require(
            runtime_mesh.get("name") == expected_mesh["name"]
            and runtime_mesh.get("materialIndex") == expected_mesh["materialIndex"]
            and len(runtime_mesh.get("vertices") or []) == expected_mesh["vertexCount"]
            and len(runtime_mesh.get("indices") or []) == expected_mesh["indexCount"]
            and [f32_hex(float(value)) for value in runtime_mesh.get("bounds") or []]
            == expected_mesh["boundsF32Hex"],
            f"candidate submesh semantics differ: {expected_mesh['name']}",
        )
        runtime_color_count = sum(
            vertex.get("color0") is not None for vertex in runtime_mesh.get("vertices") or []
        )
        require(
            runtime_color_count == expected_mesh["channelCounts"]["color0"],
            f"candidate COLOR_0 count differs: {expected_mesh['name']}",
        )
    return expected_tuple


def validate_with_decoder(
    candidate: Path, decoder_harness: Path, expected_row: dict[str, Any]
) -> None:
    require(decoder_harness.is_file(), f"decoder harness is missing: {decoder_harness}")
    completed = subprocess.run(
        [str(decoder_harness.resolve()), "--dump-candidate", str(candidate.resolve())],
        check=False,
        capture_output=True,
        text=True,
    )
    require(
        completed.returncode == 0,
        f"C++ decoder rejected {candidate.name}: "
        + (completed.stdout + completed.stderr).strip(),
    )
    try:
        actual = json.loads(completed.stdout)
    except json.JSONDecodeError as error:
        raise BindingError(f"C++ decoder emitted invalid JSON for {candidate.name}") from error
    require(
        strict_json_equal(actual, expected_row.get("decodedSemantics")),
        f"C++ decoder semantics differ for {candidate.name}",
    )


def build_input_identities(
    external_evidence: dict[str, Any],
) -> dict[str, Any]:
    builder = Path(__file__).resolve()
    approval = external_evidence["approval"]
    approved_blobs = {
        row["assetId"]: row for row in approval["requiredBlobs"]
    }
    return {
        "g02Approval": approval,
        "sourceManifest": external_evidence["inputs"]["sourceManifest"],
        "expectedSemantics": {
            "assetId": G02_REQUIRED_BLOB_PATHS[3],
            "hashRole": "TRACKED_CANONICAL_LF",
            "canonicalLfSha256": approved_blobs[G02_REQUIRED_BLOB_PATHS[3]][
                "canonicalLfSha256"
            ],
            "fidelity": "APPROVED_COMMIT_TREE_BLOB_EQUIVALENT",
        },
        "sourceExportReceipt": external_evidence["inputs"]["sourceExportReceipt"],
        "legacyCookReceipt": external_evidence["inputs"]["legacyCookReceipt"],
        "legacyConverter": external_evidence["inputs"]["legacyConverter"],
        "builder": {
            "assetId": "Tools/ModelAssetConverter/build_artist_31470_geometry_resource_binding.py",
            "hashRole": "TRACKED_CANONICAL_LF",
            "canonicalLfSha256": canonical_tracked_sha256(
                builder, "geometry resource binding builder"
            ),
            "fidelity": "SELF_RECORDED_NOT_EXTERNAL_AUTHORITY",
        },
        "geometryCooker": {
            "assetId": G02_REQUIRED_BLOB_PATHS[1],
            "hashRole": "TRACKED_CANONICAL_LF",
            "canonicalLfSha256": approved_blobs[G02_REQUIRED_BLOB_PATHS[1]][
                "canonicalLfSha256"
            ],
            "fidelity": "APPROVED_COMMIT_TREE_BLOB_EQUIVALENT",
        },
        "geometryVerifier": {
            "assetId": G02_REQUIRED_BLOB_PATHS[2],
            "hashRole": "TRACKED_CANONICAL_LF",
            "canonicalLfSha256": approved_blobs[G02_REQUIRED_BLOB_PATHS[2]][
                "canonicalLfSha256"
            ],
            "fidelity": "APPROVED_COMMIT_TREE_BLOB_EQUIVALENT",
        },
        "unverifiedExternalAuthorityCount": external_evidence[
            "unverifiedExternalAuthorityCount"
        ],
    }


def prepare_empty_staging_root(path: Path) -> Path:
    resolved = path.resolve()
    if resolved.exists():
        require(resolved.is_dir(), f"staging root is not a directory: {resolved}")
        require(not any(resolved.iterdir()), f"staging root is not empty: {resolved}")
    else:
        resolved.mkdir(parents=True)
    return resolved


def cache_identity_for_binding(asset_id_value: str, expected_tuple: dict[str, Any]) -> str:
    require(
        type(expected_tuple.get("channelMask")) is int
        and type(expected_tuple.get("evidenceFlags")) is int,
        f"geometry cache identity masks must be exact integers: {asset_id_value}",
    )
    require_exact_geometry_pre_scale(
        expected_tuple.get("geometryPreScale"),
        f"geometry cache identity pre-scale {asset_id_value}",
    )
    require(
        expected_tuple.get("geometryPreScaleF32Hex")
        == EXPECTED_GEOMETRY_PRE_SCALE_F32_HEX,
        f"geometry cache identity pre-scale hex differs: {asset_id_value}",
    )
    return sha256_bytes(
        canonical_json_bytes(
            {
                "assetId": asset_id_value,
                "payloadSha256": expected_tuple["payloadSha256"],
                "provenanceSha256": expected_tuple["provenanceSha256"],
                "geometryPreScaleF32Hex": expected_tuple["geometryPreScaleF32Hex"],
                "channelMask": expected_tuple["channelMask"],
                "evidenceFlags": expected_tuple["evidenceFlags"],
                "submeshes": expected_tuple["submeshes"],
            }
        )
    )


def build_typed_binding(rows: list[dict[str, Any]]) -> dict[str, Any]:
    bindings: list[dict[str, Any]] = []
    for row in rows:
        expected_tuple = row["expectedTuple"]
        carrier_asset_id = row["assetId"]
        bindings.append(
            {
                "bindingId": f"artist.31470:{carrier_asset_id}",
                "sourceObject": row["sourceObject"],
                "carrierAssetId": carrier_asset_id,
                "formatVersion": expected_tuple["formatVersion"],
                "payloadSha256": expected_tuple["payloadSha256"],
                "provenanceSha256": expected_tuple["provenanceSha256"],
                "provenanceRole": expected_tuple["provenanceRole"],
                "geometryPreScale": expected_tuple["geometryPreScale"],
                "geometryPreScaleF32Hex": expected_tuple["geometryPreScaleF32Hex"],
                "channelMask": expected_tuple["channelMask"],
                "evidenceFlags": expected_tuple["evidenceFlags"],
                "submeshes": expected_tuple["submeshes"],
                "cacheIdentitySha256": cache_identity_for_binding(
                    carrier_asset_id, expected_tuple
                ),
                "runtimeGeometryPreScaleConsumed": False,
                "productAdmission": False,
                "productBlockers": list(PRODUCT_BLOCKERS),
            }
        )
    binding: dict[str, Any] = {
        "schema": BINDING_SCHEMA,
        "formatVersion": BINDING_FORMAT_VERSION,
        "assetId": "effect.artist.skill.31470",
        "characterClass": CHARACTER_CLASS,
        "skillId": SKILL_ID,
        "resourceRootRole": "CLIENT_RESOURCES_RELATIVE",
        "numericTypePolicy": {
            "geometryPreScale": GEOMETRY_PRE_SCALE_JSON_TYPE_POLICY,
            "channelMask": "JSON_INTEGER_ONLY",
            "evidenceFlags": "JSON_INTEGER_ONLY",
        },
        "bindings": bindings,
        "summary": {
            "carrierCount": len(bindings),
            "submeshCount": sum(len(row["submeshes"]) for row in bindings),
            "runtimeGeometryPreScaleConsumedCount": 0,
            "productAdmissionCount": 0,
        },
        "productAdmission": False,
        "productBlockers": list(PRODUCT_BLOCKERS),
    }
    binding["bindingSha256"] = sha256_bytes(canonical_json_bytes(binding))
    return binding


def build_binding_artifacts(
    source_root: Path,
    legacy_mesh_root: Path,
    source_manifest: Path,
    source_export_receipt: Path,
    legacy_cook_receipt: Path,
    source_package_root: Path,
    legacy_converter: Path,
    expected_semantics_path: Path,
    staging_root: Path,
    decoder_harness: Path | None = None,
) -> tuple[dict[str, Any], dict[str, Any]]:
    _, expected_by_source, _ = load_expected_semantics(expected_semantics_path)
    external_evidence = collect_external_evidence(
        source_root,
        legacy_mesh_root,
        source_manifest,
        source_export_receipt,
        legacy_cook_receipt,
        source_package_root,
        legacy_converter,
        expected_semantics_path,
    )
    package_by_source = load_source_manifest_package_map(source_manifest)
    stage = prepare_empty_staging_root(staging_root)
    rows: list[dict[str, Any]] = []

    for package, name in ASSETS:
        key = source_object(package, name)
        expected_row = expected_by_source[key]
        source_gltf = source_root / package / "StaticMesh3" / f"{name}.gltf"
        legacy_wmodel = legacy_mesh_root / f"{name}.wmodel"
        source_package = source_package_root / package_by_source[key]
        provenance, expected_gltf, expected_legacy = load_geometry_provenance_evidence(
            key,
            source_gltf,
            legacy_wmodel,
            source_manifest,
            source_export_receipt,
            legacy_cook_receipt,
            source_package,
            legacy_converter,
        )
        candidate_bytes, cook_receipt = cook_wmodel_geometry_contract(
            source_gltf,
            legacy_wmodel,
            provenance,
            EXPECTED_SOURCE_TO_WMODEL_SCALE,
            EXPECTED_GEOMETRY_PRE_SCALE,
            expected_gltf,
            expected_legacy,
        )
        require(
            cook_receipt.get("runtimeProductAdmission") is False,
            f"G05-G cook unexpectedly admitted Product: {key}",
        )
        runtime = parse_geometry_wmodel(candidate_bytes)
        expected_tuple = validate_runtime_against_expected(runtime, expected_row, candidate_bytes)
        relative = asset_relative_path(asset_id(package, name))
        candidate = stage / relative
        candidate.parent.mkdir(parents=True, exist_ok=True)
        write_atomic(candidate, candidate_bytes)
        require(sha256_file(candidate).hex() == sha256_bytes(candidate_bytes), "stage write changed bytes")
        if decoder_harness is not None:
            validate_with_decoder(candidate, decoder_harness, expected_row)
        rows.append(
            {
                "sourceObject": key,
                "assetId": asset_id(package, name),
                "sourceGltf": {
                    "logicalPath": f"{package}/StaticMesh3/{name}.gltf",
                    "byteSize": source_gltf.stat().st_size,
                    "sha256": expected_gltf.hex(),
                    "fidelity": "APPROVED_G02_SEMANTIC_GOLDEN_PINNED_BYTES",
                },
                "legacyResource": {
                    "byteSize": legacy_wmodel.stat().st_size,
                    "sha256": expected_legacy.hex(),
                    "fidelity": "APPROVED_G02_SEMANTIC_GOLDEN_PINNED_BYTES",
                },
                "candidateResource": {
                    "byteSize": len(candidate_bytes),
                    "sha256": sha256_bytes(candidate_bytes),
                },
                "expectedTuple": expected_tuple,
                "sourceEvidenceJoin": external_evidence["assets"][key],
                "artifactBindingIntegrity": "EXPECTED_G02_TUPLE_MATCHES_STAGED_BYTES",
                "trackedRuntimeDeploymentAssertion": False,
                "runtimeGeometryPreScaleConsumed": False,
                "productAdmission": False,
            }
        )

    require(len(rows) == len(ASSETS), "geometry binding did not cover seven carriers")
    require(
        len({row["assetId"] for row in rows}) == len(ASSETS),
        "geometry binding contains duplicate asset IDs",
    )
    typed_binding = build_typed_binding(rows)
    receipt: dict[str, Any] = {
        "schema": RECEIPT_SCHEMA,
        "formatVersion": RECEIPT_FORMAT_VERSION,
        "characterClass": CHARACTER_CLASS,
        "skillId": SKILL_ID,
        "scope": "G05-G_GEOMETRY_CANDIDATE_AND_EXPECTED_TUPLE",
        "inputs": build_input_identities(external_evidence),
        "resourcePolicy": {
            "assetIdPrefix": ASSET_PREFIX,
            "physicalResourceRootOwnership": "TEAM_LEAD_MANAGED_NOT_GIT",
            "deployment": "BACKUP_VALIDATE_ATOMIC_REPLACE_ALL_OR_ROLLBACK_ALL",
            "trackedResourcePayloadCount": 0,
        },
        "typedBinding": {
            "assetId": TYPED_BINDING_ASSET_ID,
            "hashRole": "TRACKED_CANONICAL_JSON",
            "sha256": sha256_bytes(canonical_json_bytes(typed_binding)),
            "bindingSha256": typed_binding["bindingSha256"],
        },
        "assets": rows,
        "summary": {
            "carrierCount": len(rows),
            "submeshCount": sum(len(row["expectedTuple"]["submeshes"]) for row in rows),
            "vertexCount": sum(
                mesh["vertexCount"]
                for row in rows
                for mesh in row["expectedTuple"]["submeshes"]
            ),
            "indexCount": sum(
                mesh["indexCount"]
                for row in rows
                for mesh in row["expectedTuple"]["submeshes"]
            ),
            "color0CarrierCount": sum(
                any(mesh["channelCounts"]["color0"] > 0 for mesh in row["expectedTuple"]["submeshes"])
                for row in rows
            ),
            "stagedExpectedTupleMatchCount": len(rows),
            "trackedRuntimeDeploymentAssertionCount": 0,
            "runtimeGeometryPreScaleConsumedCount": 0,
            "productAdmissionCount": 0,
            "unverifiedExternalAuthorityCount": external_evidence[
                "unverifiedExternalAuthorityCount"
            ],
        },
        "productAdmission": False,
        "productBlockers": list(PRODUCT_BLOCKERS),
    }
    receipt["receiptSha256"] = sha256_bytes(canonical_json_bytes(receipt))
    validate_typed_binding(typed_binding, expected_semantics_path)
    validate_binding_receipt(
        receipt, typed_binding, expected_semantics_path, external_evidence
    )
    return typed_binding, receipt


def expected_asset_rows(expected_semantics_path: Path) -> list[dict[str, Any]]:
    _, expected_by_source, _ = load_expected_semantics(expected_semantics_path)
    rows: list[dict[str, Any]] = []
    for package, name in ASSETS:
        key = source_object(package, name)
        expected = expected_by_source[key]
        rows.append(
            {
                "sourceObject": key,
                "assetId": asset_id(package, name),
                "sourceGltfSha256": expected.get("sourceGltfSha256"),
                "legacyWModelSha256": expected.get("legacyWModelSha256"),
                "candidateSha256": expected.get("candidateWModelSha256"),
                "expectedTuple": decoded_tuple_from_golden(expected),
            }
        )
    return rows


def validate_sha256(value: Any, label: str) -> None:
    require(
        isinstance(value, str)
        and len(value) == 64
        and all(character in "0123456789abcdef" for character in value),
        f"{label} is not a lowercase SHA-256",
    )


def validate_sealed_row(value: Any, label: str) -> None:
    require(isinstance(value, dict), f"{label} is not an object")
    row = value.get("row")
    require(isinstance(row, dict), f"{label} row is not an object")
    validate_sha256(value.get("rowSha256"), f"{label} row SHA-256")
    require(
        value.get("rowSha256") == sha256_bytes(canonical_json_bytes(row)),
        f"{label} row SHA-256 differs",
    )


def validate_typed_binding(
    binding: dict[str, Any], expected_semantics_path: Path
) -> None:
    require(
        binding.get("schema") == BINDING_SCHEMA
        and is_exact_json_integer(
            binding.get("formatVersion"), BINDING_FORMAT_VERSION
        )
        and binding.get("assetId") == "effect.artist.skill.31470"
        and binding.get("characterClass") == CHARACTER_CLASS
        and is_exact_json_integer(binding.get("skillId"), SKILL_ID)
        and binding.get("resourceRootRole") == "CLIENT_RESOURCES_RELATIVE"
        and binding.get("numericTypePolicy")
        == {
            "geometryPreScale": GEOMETRY_PRE_SCALE_JSON_TYPE_POLICY,
            "channelMask": "JSON_INTEGER_ONLY",
            "evidenceFlags": "JSON_INTEGER_ONLY",
        },
        "typed geometry binding root is unsupported",
    )
    stored_sha = binding.get("bindingSha256")
    validate_sha256(stored_sha, "typed binding SHA-256")
    unsigned = dict(binding)
    unsigned.pop("bindingSha256", None)
    require(
        stored_sha == sha256_bytes(canonical_json_bytes(unsigned)),
        "typed geometry binding SHA-256 differs",
    )
    expected_rows = expected_asset_rows(expected_semantics_path)
    rows = binding.get("bindings")
    require(
        isinstance(rows, list) and len(rows) == len(expected_rows),
        "typed geometry binding must have seven rows",
    )
    asset_ids: set[str] = set()
    binding_ids: set[str] = set()
    cache_identities: set[str] = set()
    for actual, expected in zip(rows, expected_rows):
        require(isinstance(actual, dict), "typed geometry binding row is not an object")
        expected_tuple = expected["expectedTuple"]
        carrier_asset_id = expected["assetId"]
        expected_binding_id = f"artist.31470:{carrier_asset_id}"
        require(
            actual.get("bindingId") == expected_binding_id
            and actual.get("sourceObject") == expected["sourceObject"]
            and actual.get("carrierAssetId") == carrier_asset_id
            and actual.get("formatVersion") == expected_tuple["formatVersion"]
            and actual.get("payloadSha256") == expected_tuple["payloadSha256"]
            and actual.get("provenanceSha256") == expected_tuple["provenanceSha256"]
            and actual.get("provenanceRole") == expected_tuple["provenanceRole"]
            and type(actual.get("geometryPreScale")) is float
            and math.isfinite(actual["geometryPreScale"])
            and f32_hex(actual["geometryPreScale"])
            == EXPECTED_GEOMETRY_PRE_SCALE_F32_HEX
            and actual.get("geometryPreScaleF32Hex")
            == expected_tuple["geometryPreScaleF32Hex"]
            and type(actual.get("channelMask")) is int
            and actual.get("channelMask") == expected_tuple["channelMask"]
            and type(actual.get("evidenceFlags")) is int
            and actual.get("evidenceFlags") == expected_tuple["evidenceFlags"]
            and strict_json_equal(actual.get("submeshes"), expected_tuple["submeshes"]),
            f"typed geometry binding differs from G02: {carrier_asset_id}",
        )
        expected_cache_identity = cache_identity_for_binding(
            carrier_asset_id, actual
        )
        require(
            actual.get("cacheIdentitySha256") == expected_cache_identity,
            f"geometry cache identity differs: {carrier_asset_id}",
        )
        require(
            actual.get("runtimeGeometryPreScaleConsumed") is False
            and actual.get("productAdmission") is False
            and actual.get("productBlockers") == list(PRODUCT_BLOCKERS),
            f"typed geometry binding admission differs: {carrier_asset_id}",
        )
        require(
            carrier_asset_id not in asset_ids
            and expected_binding_id not in binding_ids
            and expected_cache_identity not in cache_identities,
            f"duplicate or cache-colliding geometry binding: {carrier_asset_id}",
        )
        asset_ids.add(carrier_asset_id)
        binding_ids.add(expected_binding_id)
        cache_identities.add(expected_cache_identity)
    expected_summary = {
        "carrierCount": len(rows),
        "submeshCount": sum(len(row["submeshes"]) for row in rows),
        "runtimeGeometryPreScaleConsumedCount": 0,
        "productAdmissionCount": 0,
    }
    require(
        strict_json_equal(binding.get("summary"), expected_summary)
        and binding.get("productAdmission") is False
        and binding.get("productBlockers") == list(PRODUCT_BLOCKERS),
        "typed geometry binding summary/admission differs",
    )


def validate_binding_receipt(
    receipt: dict[str, Any],
    typed_binding: dict[str, Any],
    expected_semantics_path: Path,
    external_evidence: dict[str, Any] | None = None,
) -> None:
    require(
        receipt.get("schema") == RECEIPT_SCHEMA
        and is_exact_json_integer(
            receipt.get("formatVersion"), RECEIPT_FORMAT_VERSION
        )
        and receipt.get("characterClass") == CHARACTER_CLASS
        and is_exact_json_integer(receipt.get("skillId"), SKILL_ID)
        and receipt.get("scope") == "G05-G_GEOMETRY_CANDIDATE_AND_EXPECTED_TUPLE",
        "geometry resource binding root is unsupported",
    )
    stored_sha = receipt.get("receiptSha256")
    validate_sha256(stored_sha, "receipt SHA-256")
    unsigned = dict(receipt)
    unsigned.pop("receiptSha256", None)
    require(
        stored_sha == sha256_bytes(canonical_json_bytes(unsigned)),
        "geometry resource binding receipt SHA-256 differs",
    )
    expected_rows = expected_asset_rows(expected_semantics_path)
    rows = receipt.get("assets")
    require(isinstance(rows, list) and len(rows) == len(expected_rows), "binding must have seven assets")
    for actual, expected in zip(rows, expected_rows):
        require(isinstance(actual, dict), "geometry binding row is not an object")
        require(
            actual.get("sourceObject") == expected["sourceObject"]
            and actual.get("assetId") == expected["assetId"]
            and actual.get("sourceGltf", {}).get("sha256")
            == expected["sourceGltfSha256"]
            and actual.get("legacyResource", {}).get("sha256")
            == expected["legacyWModelSha256"]
            and actual.get("candidateResource", {}).get("sha256")
            == expected["candidateSha256"]
            and strict_json_equal(actual.get("expectedTuple"), expected["expectedTuple"]),
            f"geometry binding row differs from immutable G02 semantics: {expected['assetId']}",
        )
        require(
            actual.get("artifactBindingIntegrity")
            == "EXPECTED_G02_TUPLE_MATCHES_STAGED_BYTES"
            and actual.get("trackedRuntimeDeploymentAssertion") is False
            and actual.get("runtimeGeometryPreScaleConsumed") is False
            and actual.get("productAdmission") is False,
            f"geometry binding admission state is invalid: {expected['assetId']}",
        )
        legacy = actual.get("legacyResource")
        source = actual.get("sourceGltf")
        candidate = actual.get("candidateResource")
        require(
            isinstance(legacy, dict)
            and type(legacy.get("byteSize")) is int
            and legacy["byteSize"] > 0
            and isinstance(source, dict)
            and type(source.get("byteSize")) is int
            and source["byteSize"] > 0
            and isinstance(candidate, dict)
            and type(candidate.get("byteSize")) is int
            and candidate["byteSize"] > 0,
            f"geometry binding byte identity is invalid: {expected['assetId']}",
        )
        expected_basename = Path(expected["assetId"]).name
        expected_package = expected["sourceObject"].split(".", 1)[0].upper()
        expected_gltf_path = (
            f"{expected_package}/StaticMesh3/{Path(expected_basename).stem}.gltf"
        )
        require(
            source.get("logicalPath") == expected_gltf_path
            and source.get("fidelity")
            == "APPROVED_G02_SEMANTIC_GOLDEN_PINNED_BYTES"
            and legacy.get("fidelity")
            == "APPROVED_G02_SEMANTIC_GOLDEN_PINNED_BYTES",
            f"geometry source/legacy fidelity join differs: {expected['assetId']}",
        )
        validate_sha256(legacy.get("sha256"), "legacy resource SHA-256")
        validate_sha256(source.get("sha256"), "source glTF SHA-256")
        validate_sha256(candidate.get("sha256"), "candidate resource SHA-256")
        join = actual.get("sourceEvidenceJoin")
        require(isinstance(join, dict), "source evidence join is missing")
        for field in (
            "manifestRow",
            "sourceExportRow",
            "sourceGltfOutputRow",
            "legacyCookRow",
        ):
            validate_sealed_row(join.get(field), f"{field} {expected['assetId']}")
        manifest_row = join["manifestRow"]["row"]
        export_row = join["sourceExportRow"]["row"]
        gltf_output_row = join["sourceGltfOutputRow"]["row"]
        cook_row = join["legacyCookRow"]["row"]
        require(
            manifest_row.get("sourceAssetPath") == expected["sourceObject"]
            and manifest_row.get("logicalPackage")
            == expected["sourceObject"].split(".", 1)[0]
            and manifest_row.get("resolutionStatus") == "RESOLVED_SOURCE_PACKAGE"
            and export_row.get("sourceAssetPath") == expected["sourceObject"]
            and export_row.get("logicalPackage") == manifest_row.get("logicalPackage")
            and export_row.get("resolutionStatus") == "EXPORTED"
            and gltf_output_row in (export_row.get("outputs") or [])
            and gltf_output_row.get("relativePath") == source["logicalPath"]
            and gltf_output_row.get("byteSize") == source["byteSize"]
            and gltf_output_row.get("sha256") == source["sha256"]
            and cook_row.get("sourceAssetPath") == expected["sourceObject"]
            and cook_row.get("role") == "mesh"
            and cook_row.get("sourceFile") == source["logicalPath"]
            and cook_row.get("runtimeAssetId") == expected["assetId"]
            and cook_row.get("byteSize") == legacy["byteSize"]
            and cook_row.get("sha256") == legacy["sha256"],
            f"stored evidence rows are not compositionally joined: {expected['assetId']}",
        )
        buffer_rows = join.get("sourceBufferOutputRows")
        require(
            isinstance(buffer_rows, list) and buffer_rows,
            f"source buffer evidence is missing: {expected['assetId']}",
        )
        for index, buffer_row in enumerate(buffer_rows):
            validate_sealed_row(
                buffer_row, f"source buffer row {expected['assetId']}[{index}]"
            )
            raw = buffer_row.get("rawBytes")
            require(
                isinstance(raw, dict)
                and type(raw.get("byteSize")) is int
                and raw["byteSize"] > 0,
                f"source buffer raw identity is invalid: {expected['assetId']}",
            )
            validate_sha256(raw.get("sha256"), "source buffer raw SHA-256")
            require(
                buffer_row["row"] in (export_row.get("outputs") or [])
                and buffer_row["row"].get("byteSize") == raw["byteSize"]
                and buffer_row["row"].get("sha256") == raw["sha256"],
                f"source buffer row/raw join differs: {expected['assetId']}[{index}]",
            )
        gltf_raw = join["sourceGltfOutputRow"].get("rawBytes")
        legacy_raw = join.get("legacyResourceRawBytes")
        require(
            strict_json_equal(gltf_raw, {"byteSize": source["byteSize"], "sha256": source["sha256"]})
            and strict_json_equal(
                legacy_raw,
                {"byteSize": legacy["byteSize"], "sha256": legacy["sha256"]},
            ),
            f"raw source/legacy identity differs from receipt row: {expected['assetId']}",
        )
        observed_package = join.get("sourcePackageObserved")
        require(
            isinstance(observed_package, dict)
            and observed_package.get("basename") == manifest_row.get("physicalPackage")
            and observed_package.get("fidelity") == "OBSERVED_UNVERIFIED"
            and type(observed_package.get("byteSize")) is int
            and observed_package["byteSize"] > 0,
            f"source package observation fidelity differs: {expected['assetId']}",
        )
        validate_sha256(observed_package.get("sha256"), "source package observed SHA-256")
        if external_evidence is not None:
            require(
                strict_json_equal(
                    join, external_evidence["assets"][expected["sourceObject"]]
                ),
                f"stored source evidence does not match supplied raw inputs: {expected['assetId']}",
            )
    summary = receipt.get("summary")
    require(isinstance(summary, dict), "geometry resource binding summary is missing")
    recalculated = {
        "carrierCount": len(rows),
        "submeshCount": sum(len(row["expectedTuple"]["submeshes"]) for row in rows),
        "vertexCount": sum(
            mesh["vertexCount"] for row in rows for mesh in row["expectedTuple"]["submeshes"]
        ),
        "indexCount": sum(
            mesh["indexCount"] for row in rows for mesh in row["expectedTuple"]["submeshes"]
        ),
        "color0CarrierCount": sum(
            any(mesh["channelCounts"]["color0"] > 0 for mesh in row["expectedTuple"]["submeshes"])
            for row in rows
        ),
        "stagedExpectedTupleMatchCount": len(rows),
        "trackedRuntimeDeploymentAssertionCount": 0,
        "runtimeGeometryPreScaleConsumedCount": 0,
        "productAdmissionCount": 0,
        "unverifiedExternalAuthorityCount": 3 + len(ASSETS),
    }
    require(strict_json_equal(summary, recalculated), "geometry binding summary differs")
    require(
        receipt.get("productAdmission") is False
        and receipt.get("productBlockers") == list(PRODUCT_BLOCKERS),
        "geometry resource binding may not admit Product",
    )
    validate_typed_binding(typed_binding, expected_semantics_path)
    typed_identity = receipt.get("typedBinding")
    require(
        isinstance(typed_identity, dict)
        and typed_identity.get("assetId") == TYPED_BINDING_ASSET_ID
        and typed_identity.get("hashRole") == "TRACKED_CANONICAL_JSON"
        and typed_identity.get("sha256")
        == sha256_bytes(canonical_json_bytes(typed_binding))
        and typed_identity.get("bindingSha256")
        == typed_binding.get("bindingSha256"),
        "receipt does not bind the typed GeometryBinding artifact",
    )
    inputs = receipt.get("inputs")
    require(isinstance(inputs, dict), "geometry binding inputs are missing")
    repository_root = Path(__file__).resolve().parents[2]
    expected_approval = validate_g02_approved_tree_equivalence(repository_root)
    require(
        strict_json_equal(inputs.get("g02Approval"), expected_approval),
        "binding does not record the approved G02 tree-equivalence contract",
    )
    approved_blobs = {
        row["assetId"]: row for row in expected_approval["requiredBlobs"]
    }
    expected_golden_sha = canonical_tracked_sha256(
        expected_semantics_path, "G02 expected semantics"
    )
    require(
        inputs.get("expectedSemantics")
        == {
            "assetId": G02_REQUIRED_BLOB_PATHS[3],
            "hashRole": "TRACKED_CANONICAL_LF",
            "canonicalLfSha256": expected_golden_sha,
            "fidelity": "APPROVED_COMMIT_TREE_BLOB_EQUIVALENT",
        }
        and expected_golden_sha
        == approved_blobs[G02_REQUIRED_BLOB_PATHS[3]]["canonicalLfSha256"],
        "binding does not pin the approved G02 semantic blob",
    )
    builder = Path(__file__).resolve()
    require(
        inputs.get("builder", {}).get("canonicalLfSha256")
        == canonical_tracked_sha256(builder, "geometry resource binding builder"),
        "binding builder identity is stale",
    )
    require(
        inputs.get("builder", {}).get("fidelity")
        == "SELF_RECORDED_NOT_EXTERNAL_AUTHORITY"
        and inputs.get("geometryCooker", {}).get("canonicalLfSha256")
        == approved_blobs[G02_REQUIRED_BLOB_PATHS[1]]["canonicalLfSha256"]
        and inputs.get("geometryCooker", {}).get("fidelity")
        == "APPROVED_COMMIT_TREE_BLOB_EQUIVALENT"
        and inputs.get("geometryVerifier", {}).get("canonicalLfSha256")
        == approved_blobs[G02_REQUIRED_BLOB_PATHS[2]]["canonicalLfSha256"]
        and inputs.get("geometryVerifier", {}).get("fidelity")
        == "APPROVED_COMMIT_TREE_BLOB_EQUIVALENT"
        and is_exact_json_integer(
            inputs.get("unverifiedExternalAuthorityCount"), 3 + len(ASSETS)
        ),
        "geometry tool fidelity/evidence count differs",
    )
    for field in ("sourceExportReceipt", "legacyCookReceipt", "legacyConverter"):
        identity = inputs.get(field)
        require(
            isinstance(identity, dict)
            and identity.get("hashRole") == "EXTERNAL_RAW_BYTES"
            and identity.get("fidelity") == "OBSERVED_UNVERIFIED"
            and type(identity.get("byteSize")) is int
            and identity["byteSize"] > 0,
            f"{field} must remain an observed unverified raw identity",
        )
        validate_sha256(identity.get("sha256"), f"{field} SHA-256")
    source_manifest_identity = inputs.get("sourceManifest")
    require(
        isinstance(source_manifest_identity, dict)
        and source_manifest_identity.get("assetId") == G02_REQUIRED_BLOB_PATHS[0]
        and source_manifest_identity.get("hashRole") == "TRACKED_CANONICAL_LF"
        and source_manifest_identity.get("canonicalLfSha256")
        == approved_blobs[G02_REQUIRED_BLOB_PATHS[0]]["canonicalLfSha256"]
        and source_manifest_identity.get("fidelity")
        == "APPROVED_COMMIT_TREE_BLOB_EQUIVALENT"
        and source_manifest_identity.get("legacyReceiptCorrelation")
        in ("CANONICAL_LF", "CANONICAL_CRLF_VARIANT"),
        "source manifest approved-tree identity differs",
    )
    if external_evidence is not None:
        require(
            strict_json_equal(inputs, build_input_identities(external_evidence)),
            "binding input identities/rows do not match supplied raw evidence",
        )


def load_and_validate_artifacts(
    binding_path: Path,
    receipt_path: Path,
    expected_semantics_path: Path,
    external_evidence: dict[str, Any] | None = None,
) -> tuple[dict[str, Any], dict[str, Any]]:
    binding = load_strict_json_object(binding_path, "typed geometry binding")
    receipt = load_strict_json_object(receipt_path, "geometry resource binding receipt")
    validate_typed_binding(binding, expected_semantics_path)
    validate_binding_receipt(
        receipt, binding, expected_semantics_path, external_evidence
    )
    return binding, receipt


def staged_candidate_path(staging_root: Path, asset: dict[str, Any]) -> Path:
    return staging_root.resolve() / asset_relative_path(str(asset.get("assetId", "")))


def validate_bound_candidate_bytes(
    value: bytes, asset: dict[str, Any], expected_row: dict[str, Any]
) -> None:
    candidate = asset.get("candidateResource") or {}
    require(
        len(value) == candidate.get("byteSize")
        and sha256_bytes(value) == candidate.get("sha256"),
        f"candidate bytes differ for {asset.get('assetId')}",
    )
    runtime = parse_geometry_wmodel(value)
    validate_runtime_against_expected(runtime, expected_row, value)


def validate_staging_tree(
    receipt: dict[str, Any],
    expected_semantics_path: Path,
    staging_root: Path,
    decoder_harness: Path | None = None,
) -> None:
    _, expected_by_source, _ = load_expected_semantics(expected_semantics_path)
    expected_paths: set[Path] = set()
    for asset in receipt["assets"]:
        candidate = staged_candidate_path(staging_root, asset)
        expected_paths.add(candidate.resolve())
        require(candidate.is_file(), f"staged candidate is missing: {candidate}")
        validate_bound_candidate_bytes(
            candidate.read_bytes(), asset, expected_by_source[asset["sourceObject"]]
        )
        if decoder_harness is not None:
            validate_with_decoder(
                candidate, decoder_harness, expected_by_source[asset["sourceObject"]]
            )
    actual_paths = {path.resolve() for path in staging_root.rglob("*.wmodel") if path.is_file()}
    require(actual_paths == expected_paths, "staging tree has missing or unexpected WModels")


def validate_physical_state(
    receipt: dict[str, Any],
    expected_semantics_path: Path,
    physical_mesh_root: Path,
    decoder_harness: Path | None = None,
) -> dict[str, Any]:
    _, expected_by_source, _ = load_expected_semantics(expected_semantics_path)
    exact_targets = scan_exact_target_basenames(
        physical_mesh_root, "physical geometry resource root"
    )
    rows: list[dict[str, Any]] = []
    for asset in receipt["assets"]:
        target = exact_targets[Path(asset["assetId"]).name]
        require(target.is_file(), f"physical geometry resource is missing: {target}")
        value = target.read_bytes()
        validate_bound_candidate_bytes(value, asset, expected_by_source[asset["sourceObject"]])
        if decoder_harness is not None:
            validate_with_decoder(target, decoder_harness, expected_by_source[asset["sourceObject"]])
        rows.append(
            {
                "assetId": asset["assetId"],
                "byteSize": len(value),
                "sha256": sha256_bytes(value),
                "expectedTupleMatch": True,
            }
        )
    return {
        "schema": "lostark.artist-31470-geometry-resource-deployment-verification",
        "formatVersion": 1,
        "characterClass": CHARACTER_CLASS,
        "skillId": SKILL_ID,
        "carrierCount": len(rows),
        "runtimeGeometryPreScaleConsumedCount": 0,
        "productAdmission": False,
        "assets": rows,
    }


def write_replace(path: Path, value: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.parent / f".{path.name}.artist31470.{uuid.uuid4().hex}.tmp"
    try:
        with temporary.open("xb") as stream:
            stream.write(value)
            stream.flush()
            os.fsync(stream.fileno())
        require(sha256_bytes(temporary.read_bytes()) == sha256_bytes(value), "staged replace bytes differ")
        os.replace(temporary, path)
    finally:
        if temporary.exists():
            temporary.unlink()


def prepare_backup_root(path: Path) -> Path:
    resolved = path.resolve()
    if resolved.exists():
        require(resolved.is_dir() and not any(resolved.iterdir()), "backup root must be empty")
    else:
        resolved.mkdir(parents=True)
    return resolved


def transactionally_replace_targets(
    targets: list[tuple[dict[str, Any], Path, bytes, bytes]],
    backup_root: Path,
    fail_after_replace: int | None = None,
    post_validate: Callable[[], None] | None = None,
) -> dict[str, Any]:
    require(len(targets) == len(ASSETS), "deployment must contain exactly seven targets")
    require(
        len({target.resolve() for _, target, _, _ in targets}) == len(targets),
        "deployment contains duplicate target paths",
    )
    physical_roots = {target.resolve().parent for _, target, _, _ in targets}
    require(len(physical_roots) == 1, "deployment targets must share one physical root")
    physical_root = next(iter(physical_roots))
    scanned_preflight = scan_exact_target_basenames(
        physical_root, "deployment preflight geometry resource root"
    )
    for asset, target, original, candidate in targets:
        require(
            scanned_preflight.get(target.name, Path()) == target.resolve(),
            f"deployment target did not come from ordinal scandir preflight: {asset['assetId']}",
        )
        require(target.is_file(), f"physical target is missing: {target}")
        require(not target.is_symlink(), f"physical target may not be a symlink: {target}")
        require(
            target.read_bytes() == original,
            f"physical target changed after preflight: {asset['assetId']}",
        )
        require(candidate != original, f"candidate did not replace legacy bytes: {asset['assetId']}")

    backup = prepare_backup_root(backup_root)
    backup_rows: list[dict[str, Any]] = []
    for asset, _, original, _ in targets:
        backup_path = backup / asset_relative_path(asset["assetId"])
        write_replace(backup_path, original)
        require(sha256_file(backup_path).hex() == sha256_bytes(original), "backup verification failed")
        backup_rows.append(
            {
                "assetId": asset["assetId"],
                "byteSize": len(original),
                "sha256": sha256_bytes(original),
            }
        )
    backup_manifest = {
        "schema": "lostark.artist-31470-geometry-resource-backup",
        "formatVersion": 1,
        "g02ApprovedCommit": G02_APPROVED_COMMIT,
        "g02ApprovedTreeSha": G02_APPROVED_TREE_SHA,
        "carrierCount": len(backup_rows),
        "assets": backup_rows,
    }
    backup_manifest["receiptSha256"] = sha256_bytes(canonical_json_bytes(backup_manifest))
    backup_manifest_path = backup / "artist-31470.geometry-resource-backup.receipt.json"
    write_atomic(
        backup_manifest_path,
        json.dumps(backup_manifest, ensure_ascii=False, indent=2).encode("utf-8") + b"\n",
    )

    replaced = 0
    try:
        for _, target, _, candidate in targets:
            write_replace(target, candidate)
            replaced += 1
            if fail_after_replace is not None and replaced == fail_after_replace:
                raise BindingError("injected deployment failure")
        scan_exact_target_basenames(
            physical_root, "deployment post-write geometry resource root"
        )
        if post_validate is not None:
            post_validate()
    except Exception as deployment_error:
        rollback_failures: list[str] = []
        for asset, target, original, _ in targets:
            try:
                write_replace(target, original)
                require(
                    sha256_file(target).hex() == sha256_bytes(original),
                    f"rollback bytes differ: {asset['assetId']}",
                )
            except Exception as rollback_error:  # pragma: no cover - catastrophic filesystem failure
                rollback_failures.append(f"{asset['assetId']}: {rollback_error}")
        try:
            scan_exact_target_basenames(
                physical_root, "deployment rollback geometry resource root"
            )
        except Exception as rollback_scan_error:  # pragma: no cover - catastrophic filesystem failure
            rollback_failures.append(f"target basename rollback: {rollback_scan_error}")
        if rollback_failures:
            raise BindingError(
                f"deployment failed ({deployment_error}); rollback incomplete: "
                + "; ".join(rollback_failures)
            ) from deployment_error
        raise BindingError(
            f"deployment failed and all seven resources were restored: {deployment_error}"
        ) from deployment_error

    return {
        "rootRole": "LOCAL_EXTERNAL_BACKUP",
        "manifestByteSize": backup_manifest_path.stat().st_size,
        "manifestSha256": sha256_file(backup_manifest_path).hex(),
        "carrierCount": len(backup_rows),
    }


def deploy_binding(
    typed_binding: dict[str, Any],
    receipt: dict[str, Any],
    expected_semantics_path: Path,
    staging_root: Path,
    physical_mesh_root: Path,
    backup_root: Path | None,
    dry_run: bool,
    approved_g02_commit: str,
    approved_g02_tree: str,
    decoder_harness: Path | None = None,
    fail_after_replace: int | None = None,
    post_validate: Callable[[], None] | None = None,
) -> dict[str, Any]:
    validate_binding_receipt(receipt, typed_binding, expected_semantics_path)
    validate_staging_tree(receipt, expected_semantics_path, staging_root, decoder_harness)
    mesh_root = physical_mesh_root.resolve()
    require(mesh_root.is_dir(), f"physical Meshes root is missing: {mesh_root}")
    exact_targets = scan_exact_target_basenames(
        mesh_root, "deployment physical geometry resource root"
    )
    targets: list[tuple[dict[str, Any], Path, bytes, bytes]] = []
    for asset in receipt["assets"]:
        target = exact_targets[Path(asset["assetId"]).name]
        require(target.is_file(), f"physical target is missing: {target}")
        require(not target.is_symlink(), f"physical target may not be a symlink: {target}")
        original = target.read_bytes()
        candidate = staged_candidate_path(staging_root, asset).read_bytes()
        legacy = asset["legacyResource"]
        require(
            len(original) == legacy["byteSize"] and sha256_bytes(original) == legacy["sha256"],
            f"physical target is not the receipt-pinned legacy input: {asset['assetId']}",
        )
        targets.append((asset, target, original, candidate))
    before = [
        {"assetId": asset["assetId"], "byteSize": len(original), "sha256": sha256_bytes(original)}
        for asset, _, original, _ in targets
    ]
    if dry_run:
        exact_after = scan_exact_target_basenames(
            mesh_root, "deployment dry-run postflight geometry resource root"
        )
        after = [
            {
                "assetId": asset["assetId"],
                "byteSize": target.stat().st_size,
                "sha256": sha256_file(target).hex(),
            }
            for asset, target, _, _ in targets
        ]
        require(
            all(exact_after[target.name] == target.resolve() for _, target, _, _ in targets),
            "dry-run target basename identity changed",
        )
        require(before == after, "dry-run changed a physical geometry resource")
        return {
            "schema": "lostark.artist-31470-geometry-resource-deployment-dry-run",
            "formatVersion": 1,
            "carrierCount": len(targets),
            "wouldReplaceCount": len(targets),
            "physicalMutationCount": 0,
            "before": before,
            "after": after,
            "productAdmission": False,
        }

    require(
        approved_g02_commit == G02_APPROVED_COMMIT
        and approved_g02_tree == G02_APPROVED_TREE_SHA,
        "physical deployment requires the exact independently approved G02 commit",
    )
    require(decoder_harness is not None, "physical deployment requires a C++ decoder harness")
    require(backup_root is not None, "physical deployment requires a persistent backup root")
    def validate_after_replace() -> None:
        validate_physical_state(
            receipt, expected_semantics_path, mesh_root, decoder_harness
        )
        if post_validate is not None:
            post_validate()

    backup_identity = transactionally_replace_targets(
        targets,
        backup_root,
        fail_after_replace=fail_after_replace,
        post_validate=validate_after_replace,
    )

    after = [
        {
            "assetId": asset["assetId"],
            "byteSize": target.stat().st_size,
            "sha256": sha256_file(target).hex(),
        }
        for asset, target, _, _ in targets
    ]
    return {
        "schema": "lostark.artist-31470-geometry-resource-deployment-result",
        "formatVersion": 1,
        "characterClass": CHARACTER_CLASS,
        "skillId": SKILL_ID,
        "g02ApprovedCommit": G02_APPROVED_COMMIT,
        "g02ApprovedTreeSha": G02_APPROVED_TREE_SHA,
        "completedUtc": datetime.now(timezone.utc).isoformat(),
        "carrierCount": len(targets),
        "backup": backup_identity,
        "before": before,
        "after": after,
        "runtimeExpectedTupleMatchCount": len(targets),
        "runtimeGeometryPreScaleConsumedCount": 0,
        "productAdmission": False,
    }


def write_json(path: Path, value: dict[str, Any]) -> None:
    write_atomic(
        path,
        json.dumps(value, ensure_ascii=False, indent=2).encode("utf-8") + b"\n",
    )


def add_external_evidence_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--source-root", required=True, type=Path)
    parser.add_argument("--legacy-mesh-root", required=True, type=Path)
    parser.add_argument("--source-manifest", required=True, type=Path)
    parser.add_argument("--source-export-receipt", required=True, type=Path)
    parser.add_argument("--legacy-cook-receipt", required=True, type=Path)
    parser.add_argument("--source-package-root", required=True, type=Path)
    parser.add_argument("--legacy-converter", required=True, type=Path)
    parser.add_argument("--expected-semantics", required=True, type=Path)


def add_build_arguments(parser: argparse.ArgumentParser) -> None:
    add_external_evidence_arguments(parser)
    parser.add_argument("--staging-root", required=True, type=Path)
    parser.add_argument("--binding-output", required=True, type=Path)
    parser.add_argument("--receipt-output", required=True, type=Path)
    parser.add_argument("--decoder-harness", type=Path)
    parser.add_argument("--check", action="store_true")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    build = subparsers.add_parser("build", help="cook seven staged candidates and build/check the binding")
    add_build_arguments(build)

    verify = subparsers.add_parser("verify-deployed", help="verify seven physical resources against the binding")
    add_external_evidence_arguments(verify)
    verify.add_argument("--binding", required=True, type=Path)
    verify.add_argument("--receipt", required=True, type=Path)
    verify.add_argument("--physical-mesh-root", required=True, type=Path)
    verify.add_argument("--decoder-harness", type=Path)

    deploy = subparsers.add_parser("deploy", help="dry-run or transactionally replace the seven resources")
    add_external_evidence_arguments(deploy)
    deploy.add_argument("--binding", required=True, type=Path)
    deploy.add_argument("--receipt", required=True, type=Path)
    deploy.add_argument("--staging-root", required=True, type=Path)
    deploy.add_argument("--physical-mesh-root", required=True, type=Path)
    deploy.add_argument("--backup-root", type=Path)
    deploy.add_argument("--approved-g02-commit", default="")
    deploy.add_argument("--approved-g02-tree", default="")
    deploy.add_argument("--decoder-harness", type=Path)
    deploy.add_argument("--deployment-result", type=Path)
    deploy.add_argument("--dry-run", action="store_true")

    args = parser.parse_args()
    if args.command == "build":
        typed_binding, receipt = build_binding_artifacts(
            args.source_root,
            args.legacy_mesh_root,
            args.source_manifest,
            args.source_export_receipt,
            args.legacy_cook_receipt,
            args.source_package_root,
            args.legacy_converter,
            args.expected_semantics,
            args.staging_root,
            args.decoder_harness,
        )
        binding_content = json.dumps(typed_binding, ensure_ascii=False, indent=2) + "\n"
        receipt_content = json.dumps(receipt, ensure_ascii=False, indent=2) + "\n"
        if args.check:
            require(
                args.binding_output.is_file(),
                f"checked typed binding is missing: {args.binding_output}",
            )
            require(
                args.receipt_output.is_file(),
                f"checked receipt is missing: {args.receipt_output}",
            )
            require(
                args.binding_output.read_text(encoding="utf-8") == binding_content,
                "checked typed geometry binding is stale",
            )
            require(
                args.receipt_output.read_text(encoding="utf-8") == receipt_content,
                "checked geometry resource receipt is stale",
            )
        else:
            write_json(args.binding_output, typed_binding)
            write_json(args.receipt_output, receipt)
        print(
            "Artist 31470 geometry resource binding PASS "
            "carriers=7 staged=7 expectedTuple=7 deployed=0 preScaleConsumed=0 product=false"
        )
        return 0

    external_evidence = collect_external_evidence(
        args.source_root,
        args.legacy_mesh_root,
        args.source_manifest,
        args.source_export_receipt,
        args.legacy_cook_receipt,
        args.source_package_root,
        args.legacy_converter,
        args.expected_semantics,
    )
    typed_binding, receipt = load_and_validate_artifacts(
        args.binding, args.receipt, args.expected_semantics, external_evidence
    )
    if args.command == "verify-deployed":
        result = validate_physical_state(
            receipt,
            args.expected_semantics,
            args.physical_mesh_root,
            args.decoder_harness,
        )
        print(
            "Artist 31470 deployed geometry verification PASS "
            f"carriers={result['carrierCount']} preScaleConsumed=0 product=false"
        )
        return 0

    result = deploy_binding(
        typed_binding,
        receipt,
        args.expected_semantics,
        args.staging_root,
        args.physical_mesh_root,
        args.backup_root,
        args.dry_run,
        args.approved_g02_commit,
        args.approved_g02_tree,
        args.decoder_harness,
    )
    if args.deployment_result is not None:
        write_json(args.deployment_result, result)
    print(
        "Artist 31470 geometry resource deployment "
        f"{'DRY-RUN' if args.dry_run else 'PASS'} carriers=7 "
        f"mutations={0 if args.dry_run else 7} preScaleConsumed=0 product=false"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
