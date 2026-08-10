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
G02_COMMIT = "2b3d7a6c410f963b2e47aa7999504c422fff7c32"
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
    source_manifest: Path,
    source_export_receipt: Path,
    legacy_cook_receipt: Path,
    expected_semantics: Path,
) -> dict[str, Any]:
    builder = Path(__file__).resolve()
    cooker = builder.parent / "cook_wmodel_geometry_contract.py"
    verifier = builder.parent / "verify_artist_31470_wmodel_geometry_contract.py"
    return {
        "g02Commit": G02_COMMIT,
        "sourceManifest": {
            "assetId": "Data/Effects/Imported/Artist/Artist.resource-source-manifest.json",
            "hashRole": "TRACKED_CANONICAL_LF",
            "sha256": canonical_tracked_sha256(source_manifest, "Artist source manifest"),
        },
        "expectedSemantics": {
            "assetId": "Tools/WModelGeometryContractHarness/Fixtures/artist_31470_v11_expected.json",
            "hashRole": "TRACKED_CANONICAL_LF",
            "sha256": canonical_tracked_sha256(expected_semantics, "G02 expected semantics"),
        },
        "sourceExportReceipt": {
            "hashRole": "EXTERNAL_RAW_BYTES",
            **raw_file_identity(source_export_receipt),
        },
        "legacyCookReceipt": {
            "hashRole": "EXTERNAL_RAW_BYTES",
            **raw_file_identity(legacy_cook_receipt),
        },
        "builder": {
            "assetId": "Tools/ModelAssetConverter/build_artist_31470_geometry_resource_binding.py",
            "hashRole": "TRACKED_CANONICAL_LF",
            "sha256": canonical_tracked_sha256(builder, "geometry resource binding builder"),
        },
        "geometryCooker": {
            "assetId": "Tools/ModelAssetConverter/cook_wmodel_geometry_contract.py",
            "hashRole": "TRACKED_RAW_LF_BYTES",
            **raw_file_identity(cooker),
        },
        "geometryVerifier": {
            "assetId": "Tools/ModelAssetConverter/verify_artist_31470_wmodel_geometry_contract.py",
            "hashRole": "TRACKED_CANONICAL_LF",
            "sha256": canonical_tracked_sha256(verifier, "Artist geometry verifier"),
        },
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
                },
                "legacyResource": {
                    "byteSize": legacy_wmodel.stat().st_size,
                    "sha256": expected_legacy.hex(),
                },
                "candidateResource": {
                    "byteSize": len(candidate_bytes),
                    "sha256": sha256_bytes(candidate_bytes),
                },
                "expectedTuple": expected_tuple,
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
        "inputs": build_input_identities(
            source_manifest,
            source_export_receipt,
            legacy_cook_receipt,
            expected_semantics_path,
        ),
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
        },
        "productAdmission": False,
        "productBlockers": list(PRODUCT_BLOCKERS),
    }
    receipt["receiptSha256"] = sha256_bytes(canonical_json_bytes(receipt))
    validate_typed_binding(typed_binding, expected_semantics_path)
    validate_binding_receipt(receipt, typed_binding, expected_semantics_path)
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
        and binding.get("resourceRootRole") == "CLIENT_RESOURCES_RELATIVE",
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
            and f32_hex(float(actual.get("geometryPreScale", math.nan)))
            == EXPECTED_GEOMETRY_PRE_SCALE_F32_HEX
            and actual.get("geometryPreScaleF32Hex")
            == expected_tuple["geometryPreScaleF32Hex"]
            and actual.get("channelMask") == expected_tuple["channelMask"]
            and actual.get("evidenceFlags") == expected_tuple["evidenceFlags"]
            and strict_json_equal(actual.get("submeshes"), expected_tuple["submeshes"]),
            f"typed geometry binding differs from G02: {carrier_asset_id}",
        )
        expected_cache_identity = cache_identity_for_binding(
            carrier_asset_id, expected_tuple
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
        validate_sha256(legacy.get("sha256"), "legacy resource SHA-256")
        validate_sha256(source.get("sha256"), "source glTF SHA-256")
        validate_sha256(candidate.get("sha256"), "candidate resource SHA-256")
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
    require(isinstance(inputs, dict) and inputs.get("g02Commit") == G02_COMMIT, "G02 commit differs")
    expected_golden_sha = canonical_tracked_sha256(
        expected_semantics_path, "G02 expected semantics"
    )
    require(
        inputs.get("expectedSemantics", {}).get("sha256") == expected_golden_sha,
        "binding does not pin the supplied immutable G02 semantics",
    )
    builder = Path(__file__).resolve()
    require(
        inputs.get("builder", {}).get("sha256")
        == canonical_tracked_sha256(builder, "geometry resource binding builder"),
        "binding builder identity is stale",
    )


def load_and_validate_artifacts(
    binding_path: Path, receipt_path: Path, expected_semantics_path: Path
) -> tuple[dict[str, Any], dict[str, Any]]:
    binding = load_strict_json_object(binding_path, "typed geometry binding")
    receipt = load_strict_json_object(receipt_path, "geometry resource binding receipt")
    validate_typed_binding(binding, expected_semantics_path)
    validate_binding_receipt(receipt, binding, expected_semantics_path)
    return binding, receipt


def staged_candidate_path(staging_root: Path, asset: dict[str, Any]) -> Path:
    return staging_root.resolve() / asset_relative_path(str(asset.get("assetId", "")))


def physical_target_path(physical_mesh_root: Path, asset: dict[str, Any]) -> Path:
    relative = asset_relative_path(str(asset.get("assetId", "")))
    target = physical_mesh_root.resolve() / relative.name
    require(target.parent == physical_mesh_root.resolve(), "geometry target escaped the Meshes root")
    return target


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
    rows: list[dict[str, Any]] = []
    for asset in receipt["assets"]:
        target = physical_target_path(physical_mesh_root, asset)
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
    for asset, target, original, candidate in targets:
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
        "g02Commit": G02_COMMIT,
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
    decoder_harness: Path | None = None,
    fail_after_replace: int | None = None,
    post_validate: Callable[[], None] | None = None,
) -> dict[str, Any]:
    validate_binding_receipt(receipt, typed_binding, expected_semantics_path)
    validate_staging_tree(receipt, expected_semantics_path, staging_root, decoder_harness)
    mesh_root = physical_mesh_root.resolve()
    require(mesh_root.is_dir(), f"physical Meshes root is missing: {mesh_root}")
    targets: list[tuple[dict[str, Any], Path, bytes, bytes]] = []
    for asset in receipt["assets"]:
        target = physical_target_path(mesh_root, asset)
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
        after = [
            {
                "assetId": asset["assetId"],
                "byteSize": target.stat().st_size,
                "sha256": sha256_file(target).hex(),
            }
            for asset, target, _, _ in targets
        ]
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
        approved_g02_commit == G02_COMMIT,
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
        "g02Commit": G02_COMMIT,
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


def add_build_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--source-root", required=True, type=Path)
    parser.add_argument("--legacy-mesh-root", required=True, type=Path)
    parser.add_argument("--source-manifest", required=True, type=Path)
    parser.add_argument("--source-export-receipt", required=True, type=Path)
    parser.add_argument("--legacy-cook-receipt", required=True, type=Path)
    parser.add_argument("--source-package-root", required=True, type=Path)
    parser.add_argument("--legacy-converter", required=True, type=Path)
    parser.add_argument("--expected-semantics", required=True, type=Path)
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
    verify.add_argument("--binding", required=True, type=Path)
    verify.add_argument("--receipt", required=True, type=Path)
    verify.add_argument("--expected-semantics", required=True, type=Path)
    verify.add_argument("--physical-mesh-root", required=True, type=Path)
    verify.add_argument("--decoder-harness", type=Path)

    deploy = subparsers.add_parser("deploy", help="dry-run or transactionally replace the seven resources")
    deploy.add_argument("--binding", required=True, type=Path)
    deploy.add_argument("--receipt", required=True, type=Path)
    deploy.add_argument("--expected-semantics", required=True, type=Path)
    deploy.add_argument("--staging-root", required=True, type=Path)
    deploy.add_argument("--physical-mesh-root", required=True, type=Path)
    deploy.add_argument("--backup-root", type=Path)
    deploy.add_argument("--approved-g02-commit", default="")
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

    typed_binding, receipt = load_and_validate_artifacts(
        args.binding, args.receipt, args.expected_semantics
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
