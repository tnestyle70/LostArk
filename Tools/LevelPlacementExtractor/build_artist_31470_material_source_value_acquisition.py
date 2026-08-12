#!/usr/bin/env python3
"""Freeze Artist F Material source-value acquisition without opening Product.

The generator re-parses authenticated source UPKs.  It intentionally separates
an observed source value from execution readiness: current-revision defaults,
consumer-only WARP pilots, and omitted UE3 class defaults never certify a
source-era value.
"""

from __future__ import annotations

import argparse
import copy
import gc
import hashlib
import json
import struct
import sys
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any

from artist_31470_material_evidence_approval import (
    APPROVED_ACQUISITION_SEMANTIC_PROJECTION_SHA256,
    APPROVED_CONTROLLED_RUNTIME_CAPTURE_SHA256,
    APPROVED_EXTERNAL_ARTIFACT_SEARCH_SHA256,
    APPROVED_STATIC_ROW_SET_SHA256,
    APPROVED_STRICT_SAMPLER_ROW_SET_SHA256,
)
import build_artist_31470_material_evidence_contract as material_contract
import build_artist_31470_material_runtime_oracle as material_runtime_oracle
import extract_artist_31470_shader_cache_oracle as shader_oracle
from extract_artist_31470_material_render_state import parse_property_records
from extract_ue3_effect_material_closure import load_package
from extract_ue3_placements import (
    LOSTARK_KR_AES_KEY,
    package_ref_name,
    package_ref_path,
)


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = Path(__file__).resolve()
APPROVAL_PATH = SCRIPT_PATH.with_name(
    "artist_31470_material_evidence_approval.py"
)
CONTRACT_ROOT = "ARTIST/31470/F"

EXPECTED_SOURCE_MANIFEST_BYTES = 270014
EXPECTED_SOURCE_MANIFEST_SHA256 = (
    "8ddce11f3cdd36efc4098b127da860b3e77e0f6916263412f1089cce3967d62d"
)
EXPECTED_ARCHIVE_FILE_COUNT = 1813
EXPECTED_ARCHIVE_UNIQUE_COUNT = 624
EXPECTED_ARCHIVE_DUPLICATE_COUNT = 1189
EXPECTED_ARCHIVE_BYTE_COUNT = 1932762844
EXPECTED_MANIFEST_PACKAGE_COUNT = 621
EXPECTED_ARCHIVE_UNIQUE_IDENTITY_SHA256 = (
    "93a3b392d61c2a221d998da6bd4ee6225ce79ee06b991f31f8e26ce51a086873"
)
EXPECTED_EXTRA_PACKAGES = {
    "809f2816b894d25e2f0c510a44a23603713c6e3732473785edaafc3451e9335f": {
        "fileName": "GW2N4GC2R5GN2C00F84CE9Z4.upk",
        "byteCount": 424656,
        "exportCount": 6,
    },
    "d347b2ffc158fdcc04d3c44dc468a9515bec6c62f8f57321ac7555caa194d5f9": {
        "fileName": "0IK5UBC5U05Q145Q3NGM5FXYYTC.upk",
        "byteCount": 4655,
        "exportCount": 42,
    },
    "e097d79eb731f5998b4b4389469bd9c6f15d12d80ce4601d08772dfffc9de31b": {
        "fileName": "UB1M7321QZZEZCKD61SSPSB.upk",
        "byteCount": 16396041,
        "exportCount": 453,
    },
}

EXPECTED_ENGINE_PACKAGE = {
    "relativePath": "EFGame/ReleasePC/NE1FENCQ4UNE9ZPRENOQS.u",
    "byteCount": 1396327,
    "sha256": "cee4257abe9a60730d48bab16e742f12123c71dd7f13faf7807c14647e989434",
}
EXPECTED_CORE_PACKAGE = {
    "relativePath": "EFGame/ReleasePC/9L6NC53E9WINO5FELWUN0.u",
}
EXPECTED_CURRENT_WP_PACKAGE = {
    "relativePath": "EFGame/ReleasePC/Packages/8V2NAH2N39CH2C07F8BEHLRC.upk",
    "byteCount": 1066958,
    "sha256": "63b1e90a1428cad5c497a7bb34a13f2df814e837f0207991d345d1cb4ead60fb",
}

TARGET_RENDER_FIELDS = (
    "bdisabledepthtest",
    "buseonelayerdistortion",
    "lightingmodel",
    "opacitymaskclipvalue",
    "twosided",
)
TARGET_TEXTURE_FIELDS = ("addressx", "addressy", "srgb", "filter", "lodgroup")

DRIVER_CACHE_AUDIT = {
    "evidenceKind": "EXTERNAL_READ_ONLY_AUDIT_SNAPSHOT",
    "admissionInput": False,
    "corroborationOnly": True,
    "regeneratedByThisGenerator": False,
    "verificationManifest": None,
    "reproducibility": "CORROBORATION_ONLY_NOT_REGENERATED_BY_THIS_BUILDER",
    "observedDate": "2026-08-10",
    "observationTimePrecision": "SESSION_DATE_ONLY",
    "accessCaveat": (
        "COUNTS_AND_ZERO_MATCHES_COVER_ONLY_THE_PATHS_AND_READABLE_BYTES_REPORTED_"
        "BY_THE_EXTERNAL_READ_ONLY_AUDIT;_13_ACTIVE_NVIDIA_NVPH_FILES_WERE_SHARE_LOCKED"
    ),
    "roots": [
        {
            "rootKind": "WINDOWS_D3D_SHADER_CACHE",
            "fileCount": 72,
            "readableFileCount": 72,
            "readableByteCount": 1626256,
            "validUniqueDxbcCount": 10,
            "targetDxbcCount": 0,
        },
        {
            "rootKind": "NVIDIA_DX_CACHE",
            "fileCount": 561,
            "readableFileCount": 548,
            "readableByteCount": 1408549888,
            "shareLockedFileCount": 13,
            "shareLockedByteCount": 2756608,
            "validUniqueDxbcCount": 0,
            "targetDxbcCount": 0,
        },
        {
            "rootKind": "NVIDIA_GL_CACHE",
            "fileCount": 74,
            "readableFileCount": 74,
            "readableByteCount": 79888677,
            "validUniqueDxbcCount": 0,
            "targetDxbcCount": 0,
        },
    ],
    "targetSearch": {
        "nativeStateKeyCount": 23,
        "nativeStateKeyRawMatchCount": 0,
        "uniqueShaderIdCandidateCount": 271,
        "shaderIdRawMatchCount": 0,
        "uniqueDxbcSha256Count": 240,
        "dxbcSha256RawMatchCount": 0,
        "dxbcSha256AsciiMatchCount": 0,
        "coverageQualifier": "ZEROES_APPLY_TO_READABLE_BYTES_ONLY",
    },
    "historicalLead": {
        "classification": "HISTORICAL_DRIVER_CACHE_BASENAME_ONLY",
        "fileCount": 8,
        "byteCount": 2263040,
        "literalProcessBasename": "lostark.exe",
        "canonicalManifestSha256": (
            "7f2834aea4d5aaf8de8f187ef02127088e6bbf9d2006a9a373e02379d3255d01"
        ),
        "sourceRevisionIdentityAvailable": False,
        "materialOrShaderIdentityAvailable": False,
    },
    "decision": "NO_ADMISSIBLE_SOURCE_SPECIFIC_PROVIDER",
}

GIT_AND_REMOTE_AUDIT = {
    "evidenceKind": "EXTERNAL_READ_ONLY_AUDIT_SNAPSHOT",
    "admissionInput": False,
    "corroborationOnly": True,
    "regeneratedByThisGenerator": False,
    "verificationManifest": None,
    "reproducibility": "CORROBORATION_ONLY_NOT_REGENERATED_BY_THIS_BUILDER",
    "observedDate": "2026-08-10",
    "observationTimePrecision": "SESSION_DATE_ONLY",
    "accessCaveat": (
        "COUNTS_COVER_ONLY_REPOSITORY_OBJECTS_LOCAL_LFS_STORAGE_AND_REMOTE_"
        "ENDPOINTS_ACCESSIBLE_TO_THE_EXTERNAL_READ_ONLY_AUDIT_SESSION"
    ),
    "reachableGit": {
        "ue3UpkOrScriptPackagePathCount": 0,
        "providerDecision": "NO_PROVIDER",
    },
    "unreachableGit": {
        "onlyUe3UpkAtLeastOneMiB": {
            "gitBlobOid": "7d19e7036aee09d6502f6f1527425aa3d52853ff",
            "byteCount": 5303178,
            "sha256": (
                "f130d3d0b4a048832885cc0d13eb8a88b7d6eb0b1a46dba6181c4d1cd3c4239f"
            ),
            "packageVersion": 868,
            "engineVersion": 12097,
            "exportCount": 1871,
            "materialExportCount": 0,
            "materialInstanceConstantExportCount": 0,
            "shaderCacheExportCount": 0,
            "classification": "COOKED_LEVEL_MAP_ONLY",
        },
        "otherLargeArtifactProviderCount": 0,
        "subOneMiBReferenceScan": {
            "objectCount": 5980,
            "byteCount": 379505715,
            "lfsPointerCount": 1232,
            "otherObjectCount": 4748,
            "peCount": 0,
            "ue3UpkCount": 0,
            "zipCount": 0,
            "providerDecision": "NO_PROVIDER",
        },
        "providerDecision": "NO_PROVIDER",
    },
    "gitLfs": {
        "contentStoreObjectCount": 1546,
        "contentStoreByteCount": 728803042,
        "ue3UpkCount": 0,
        "gameNativeProviderCount": 0,
        "exactEngineCoreShaderCacheEfNativeMatchCount": 0,
        "pointerTreeEntryCount": 798,
        "unmappedPointerCount": 757,
        "unmappedPointerLocalContentPresentCount": 757,
        "unmappedPointerPeUpkZipCount": 0,
        "providerDecision": "NO_PROVIDER",
    },
    "origin": {
        "githubActionsArtifactCount": 0,
        "githubActionsCacheCount": 0,
        "releaseCount": 0,
        "userContainerSourcePackageCount": 0,
        "sourceEraProviderCount": 0,
        "providerDecision": "NO_PROVIDER",
    },
}

CONTROLLED_CAPTURE_ASSESSMENT = {
    "safeProviderAvailable": False,
    "sourceRevisionRuntimeBundleAvailable": False,
    "sourceRevisionDebugOrCaptureApiAvailable": False,
    "currentInstalledProcessIsSourceRevisionAuthenticated": False,
    "processInjectionAllowed": False,
    "antiCheatBypassAllowed": False,
    "uncontrolledInstalledGameProcessUsed": False,
    "decision": "BLOCKED_NO_SAFE_SOURCE_REVISION_CAPTURE_PROVIDER",
    "minimumExternalArtifact": (
        "authenticated source-revision Engine.u/Core.u plus version-coupled native "
        "EFEngine/LOSTARK binaries, SystemSettings texture-group configuration, "
        "ShaderCache/material maps, and a vendor-authorized offline capture path"
    ),
}

ACQUISITION_ROOT_KEYS = {
    "schema",
    "formatVersion",
    "root",
    "characterClass",
    "skillId",
    "inputSlot",
    "scope",
    "source",
    "externalArtifactSearch",
    "provenanceClusters",
    "matrices",
    "coordinatedCorrectiveRequirements",
    "minimumMissingExternalArtifacts",
    "summary",
    "admission",
    "receiptSha256",
}

ACQUISITION_SOURCE_SPECS = (
    (
        "TYPED_MATERIAL_EVIDENCE_CONTRACT",
        "Data/Effects/Imported/Artist/Materials/"
        "skill.31470.typed-material-evidence-contract.json",
        "TRACKED_DERIVED_EOL_CANONICAL_TEXT",
        True,
    ),
    (
        "RAW_RENDER_STATE_EVIDENCE_RECEIPT",
        "Data/Effects/Imported/Artist/Materials/"
        "skill.31470.material-render-state-evidence.receipt.json",
        "TRACKED_DERIVED_EOL_CANONICAL_TEXT",
        True,
    ),
    (
        "SHADER_CACHE_NATIVE_TAIL_RECEIPT",
        "Data/Effects/Imported/Artist/Materials/"
        "skill.31470.shader-cache-oracle.receipt.json",
        "TRACKED_DERIVED_EOL_CANONICAL_TEXT",
        True,
    ),
    (
        "SOURCE_VALUE_ACQUISITION_GENERATOR",
        "Tools/LevelPlacementExtractor/"
        "build_artist_31470_material_source_value_acquisition.py",
        "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
        False,
    ),
    (
        "INDEPENDENT_MATERIAL_EVIDENCE_APPROVAL",
        "Tools/LevelPlacementExtractor/artist_31470_material_evidence_approval.py",
        "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
        False,
    ),
    (
        "MATERIAL_FEASIBILITY_MATRIX_BUILDER",
        "Tools/LevelPlacementExtractor/"
        "build_artist_31470_material_runtime_oracle.py",
        "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
        False,
    ),
    (
        "UE3_TAGGED_PROPERTY_PARSER",
        "Tools/LevelPlacementExtractor/"
        "extract_artist_31470_material_render_state.py",
        "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
        False,
    ),
    (
        "UE3_MIC_NATIVE_TAIL_PARSER",
        "Tools/LevelPlacementExtractor/"
        "extract_artist_31470_shader_cache_oracle.py",
        "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
        False,
    ),
)

ACQUISITION_SUMMARY_KEYS = {
    "renderStateRowCount",
    "renderStateSourceValueAcquiredCount",
    "staticPermutationRowCount",
    "staticExactGuidJoinCount",
    "staticOverrideTrueSourceValueAcquiredCount",
    "staticNonoverrideSemanticsUnverifiedCount",
    "staticNoExactGuidEntryCount",
    "previousDirectUnprovenSamplerRowCount",
    "newSelfDefaultSamplerRowCount",
    "previouslyAdmittedExactSamplerReauditCount",
    "previouslyAdmittedExactSamplerBlockedCount",
    "strictSamplerRowCount",
    "strictSamplerSourceValueAcquiredCount",
    "strictExecutionRowCount",
    "strictExecutionReadyCount",
    "valueProvenanceDelta",
    "executionReadinessDelta",
    "productCount",
    "renderRowSetSha256",
    "staticRowSetSha256",
    "strictSamplerRowSetSha256",
    "invalidatedPreviousExactSamplerSetSha256",
}

ACQUISITION_ADMISSION_KEYS = {
    "acquisitionReceiptEvidenceIntegrity",
    "upstreamMaterialEvidenceIntegrity",
    "sourceValueProviderPartial",
    "executionReady",
    "product",
    "r2Entry",
    "decision",
    "blockers",
}

ACQUISITION_BLOCKERS = [
    "RENDER_STATE_89_SOURCE_VALUES_UNRESOLVED",
    "STATIC_71_SOURCE_SELECTIONS_UNRESOLVED",
    "STRICT_SAMPLER_77_FULL_DESCRIPTORS_UNRESOLVED",
    "SOURCE_SPECIFIC_ACTUAL_OUTPUT_READINESS_ZERO",
    "FINAL_RUNTIME_CONSUMERS_NOT_IMPLEMENTED",
]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def reject_duplicate_object_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        require(key not in result, f"duplicate JSON key: {key}")
        result[key] = value
    return result


def load_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as stream:
        value = json.load(stream, object_pairs_hook=reject_duplicate_object_keys)
    require(isinstance(value, dict), f"JSON root must be an object: {path}")
    return value


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_bytes(value)).hexdigest()


def raw_sha256(path_or_bytes: Path | bytes) -> str:
    if isinstance(path_or_bytes, bytes):
        return hashlib.sha256(path_or_bytes).hexdigest()
    digest = hashlib.sha256()
    with path_or_bytes.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def stable_id(prefix: str, *pieces: Any) -> str:
    payload = "\0".join(str(piece) for piece in pieces).encode("utf-8")
    return f"{prefix}-{hashlib.sha256(payload).hexdigest()[:16]}"


def repository_path(path: Path) -> str:
    return path.resolve().relative_to(REPO_ROOT.resolve()).as_posix()


def source_evidence(path: Path, role: str, *, json_input: bool = True) -> dict[str, Any]:
    digest = (
        material_contract.tracked_json_text_sha256(path)
        if json_input
        else material_contract.tracked_source_text_sha256(path)
    )
    return {
        "role": role,
        "path": repository_path(path),
        "hashDomain": (
            "TRACKED_DERIVED_EOL_CANONICAL_TEXT"
            if json_input
            else "TRACKED_SOURCE_EOL_CANONICAL_TEXT"
        ),
        "canonicalTextSha256": digest,
    }


def acquisition_semantic_projection(receipt: dict[str, Any]) -> dict[str, Any]:
    """Return the acyclic, independently approved Material acquisition meaning.

    ``source`` contains the tracked hashes of this generator and of the approval
    module, and ``receiptSha256`` seals the complete candidate.  Excluding only
    those two dependency-cycle fields leaves every evidence, matrix, decision,
    count, and admission field under an independent fixed identity.
    """

    return {
        key: copy.deepcopy(receipt[key])
        for key in sorted(ACQUISITION_ROOT_KEYS - {"source", "receiptSha256"})
    }


def _validate_acquisition_sources(receipt: dict[str, Any]) -> None:
    sources = receipt.get("source")
    require(
        isinstance(sources, list) and len(sources) == len(ACQUISITION_SOURCE_SPECS),
        "acquisition source identity set changed",
    )
    for actual, (role, relative_path, hash_domain, json_input) in zip(
        sources, ACQUISITION_SOURCE_SPECS, strict=True
    ):
        require(
            isinstance(actual, dict)
            and set(actual)
            == {"role", "path", "hashDomain", "canonicalTextSha256"},
            f"acquisition source schema changed: {role}",
        )
        expected = source_evidence(
            REPO_ROOT / relative_path,
            role,
            json_input=json_input,
        )
        require(
            actual == expected and actual["hashDomain"] == hash_domain,
            f"acquisition source identity changed: {role}",
        )


def _validate_external_artifact_search(receipt: dict[str, Any]) -> None:
    search = receipt.get("externalArtifactSearch")
    require(
        isinstance(search, dict)
        and set(search)
        == {
            "scopeBoundary",
            "sourcePackManifest",
            "sourceArchive",
            "currentRevisionCandidates",
            "driverShaderCaches",
            "gitAndRemote",
            "controlledRuntimeCapture",
        },
        "external artifact search schema changed",
    )
    require(
        canonical_sha256(search) == APPROVED_EXTERNAL_ARTIFACT_SEARCH_SHA256,
        "external artifact search approved projection changed",
    )
    require(
        search.get("scopeBoundary")
        == {
            "claim": "ACCESSIBLE_LOCAL_AND_REMOTE_SCOPE_ONLY",
            "globalExhaustionClaim": False,
            "volumeShadowCopy": {
                "status": "PERMISSION_UNCHECKED",
                "admissionInput": False,
            },
        },
        "external artifact access boundary changed",
    )
    for key, expected in (
        ("driverShaderCaches", DRIVER_CACHE_AUDIT),
        ("gitAndRemote", GIT_AND_REMOTE_AUDIT),
    ):
        snapshot = search.get(key)
        require(
            snapshot == expected
            and snapshot.get("evidenceKind")
            == "EXTERNAL_READ_ONLY_AUDIT_SNAPSHOT"
            and snapshot.get("admissionInput") is False
            and snapshot.get("corroborationOnly") is True
            and snapshot.get("regeneratedByThisGenerator") is False
            and snapshot.get("verificationManifest") is None,
            f"external corroboration boundary changed: {key}",
        )
    controlled = search.get("controlledRuntimeCapture")
    require(
        controlled == CONTROLLED_CAPTURE_ASSESSMENT
        and canonical_sha256(controlled)
        == APPROVED_CONTROLLED_RUNTIME_CAPTURE_SHA256,
        "controlled runtime capture boundary changed",
    )


def compact_property(record: dict[str, Any]) -> dict[str, Any]:
    result = {
        "propertyName": record["propertyName"],
        "arrayIndex": record["arrayIndex"],
        "propertyType": record["propertyType"],
        "structType": record.get("structType"),
        "declaredDataSize": record["declaredDataSize"],
        "serializedPayloadSize": record["serializedPayloadSize"],
        "tagOffset": record["tagOffset"],
        "valueOffset": record["valueOffset"],
        "recordEndOffset": record["recordEndOffset"],
        "value": record["value"],
        "encodedValueHex": record["encodedValueHex"],
        "encodedValueSha256": record["encodedValueSha256"],
        "recordSha256": record["recordSha256"],
    }
    return result


def package_export_identity(package: Any, entry: Any) -> dict[str, Any]:
    serial = package.logical[
        entry.serial_offset : entry.serial_offset + entry.serial_size
    ]
    return {
        "exportIndex": entry.index,
        "packageReference": entry.index + 1,
        "objectPath": package_ref_path(
            entry.index + 1, package.imports, package.exports
        ),
        "className": package_ref_name(
            entry.class_index, package.imports, package.exports
        ).casefold(),
        "serialOffset": entry.serial_offset,
        "serialSize": entry.serial_size,
        "serialSha256": raw_sha256(serial),
    }


def manifest_index(source_pack_root: Path) -> tuple[dict[str, dict[str, Any]], dict[str, Any]]:
    path = source_pack_root / "source_pack_manifest.json"
    require(path.is_file(), f"source-pack manifest missing: {path}")
    raw = path.read_bytes()
    require(
        len(raw) == EXPECTED_SOURCE_MANIFEST_BYTES
        and raw_sha256(raw) == EXPECTED_SOURCE_MANIFEST_SHA256,
        "source-pack manifest identity changed",
    )
    manifest = json.loads(raw, object_pairs_hook=reject_duplicate_object_keys)
    rows = manifest.get("packages")
    require(isinstance(rows, list), "source-pack package rows missing")
    resolved = [row for row in rows if isinstance(row, dict) and row.get("resolved") is True]
    require(len(resolved) == EXPECTED_MANIFEST_PACKAGE_COUNT, "source-pack denominator changed")
    by_logical: dict[str, dict[str, Any]] = {}
    for row in resolved:
        logical = str(row.get("logicalPackage") or "").casefold()
        require(logical and logical not in by_logical, f"duplicate logical package: {logical}")
        path_value = source_pack_root / str(row.get("relativePath") or "")
        require(path_value.is_file(), f"source package missing: {path_value}")
        require(
            path_value.stat().st_size == row.get("byteSize")
            and raw_sha256(path_value) == row.get("sha256"),
            f"source package identity changed: {logical}",
        )
        by_logical[logical] = row
    return by_logical, {
        "pathHint": path.name,
        "byteCount": len(raw),
        "sha256": raw_sha256(raw),
        "resolvedPackageCount": len(resolved),
        "unresolvedPackages": manifest.get("unresolvedPackages"),
    }


def source_archive_inventory(
    source_archive_root: Path,
    source_pack_root: Path,
    manifest_by_logical: dict[str, dict[str, Any]],
    native_state_keys: list[bytes],
) -> dict[str, Any]:
    require(source_archive_root.is_dir(), "source archive root missing")
    paths = sorted(source_archive_root.rglob("*.upk"), key=lambda path: str(path).casefold())
    by_hash: dict[str, list[Path]] = defaultdict(list)
    byte_count = 0
    for path in paths:
        byte_count += path.stat().st_size
        by_hash[raw_sha256(path)].append(path)
    require(
        len(paths) == EXPECTED_ARCHIVE_FILE_COUNT
        and len(by_hash) == EXPECTED_ARCHIVE_UNIQUE_COUNT
        and len(paths) - len(by_hash) == EXPECTED_ARCHIVE_DUPLICATE_COUNT
        and byte_count == EXPECTED_ARCHIVE_BYTE_COUNT,
        "source archive denominator changed",
    )
    manifest_hashes = {str(row["sha256"]) for row in manifest_by_logical.values()}
    require(manifest_hashes <= set(by_hash), "manifest package missing from archive")
    extra_hashes = set(by_hash) - manifest_hashes
    require(extra_hashes == set(EXPECTED_EXTRA_PACKAGES), "source archive extras changed")

    identity_lines = "".join(
        f"{min(path.name for path in copies)}|{copies[0].stat().st_size}|{digest}|{len(copies)}\n"
        for digest, copies in sorted(by_hash.items())
    )
    require(
        raw_sha256(identity_lines.encode("utf-8"))
        == EXPECTED_ARCHIVE_UNIQUE_IDENTITY_SHA256,
        "source archive identity projection changed",
    )

    extras: list[dict[str, Any]] = []
    for digest in sorted(extra_hashes):
        copies = by_hash[digest]
        expected = EXPECTED_EXTRA_PACKAGES[digest]
        path = copies[0]
        require(
            path.name == expected["fileName"]
            and path.stat().st_size == expected["byteCount"],
            "source archive extra identity changed",
        )
        package = load_package(path, LOSTARK_KR_AES_KEY)
        class_counts = Counter(
            package_ref_name(entry.class_index, package.imports, package.exports).casefold()
            for entry in package.exports
        )
        require(len(package.exports) == expected["exportCount"], "extra export count changed")
        raw = path.read_bytes()
        raw_hits = [key.hex() for key in native_state_keys if key in raw]
        logical_hits = [key.hex() for key in native_state_keys if key in package.logical]
        require(not raw_hits and not logical_hits, "Artist native key found in archive extra")
        extras.append(
            {
                "fileName": path.name,
                "byteCount": path.stat().st_size,
                "sha256": digest,
                "packageVersion": package.summary.version,
                "engineVersion": package.summary.engine_version,
                "exportCount": len(package.exports),
                "relevantClassCounts": {
                    key: class_counts[key]
                    for key in sorted(class_counts)
                    if key
                    in {
                        "material",
                        "decalmaterial",
                        "materialinstanceconstant",
                        "particlesystem",
                        "shadercache",
                        "texture2d",
                    }
                },
                "artistNativeStateKeyRawHitCount": len(raw_hits),
                "artistNativeStateKeyLogicalHitCount": len(logical_hits),
                "providerDecision": "NO_ARTIST_MATERIAL_PROVIDER",
            }
        )
        del package
        gc.collect()

    logical_names = sorted(manifest_by_logical)
    exact_provider_names = [
        name
        for name in logical_names
        if name in {"engine", "core", "shadercache"}
        or "shadercache" in name
    ]
    all_files = [path for path in source_archive_root.rglob("*") if path.is_file()]
    extension_counts = Counter(path.suffix.casefold() for path in all_files)
    return {
        "rootHint": source_archive_root.name,
        "upkFileCount": len(paths),
        "rawUpkByteCount": byte_count,
        "uniqueRawSha256Count": len(by_hash),
        "duplicateCopyCount": len(paths) - len(by_hash),
        "manifestUniquePackageCount": len(manifest_hashes),
        "extraUniquePackageCount": len(extra_hashes),
        "uniquePackageIdentitySha256": EXPECTED_ARCHIVE_UNIQUE_IDENTITY_SHA256,
        "scriptPackageFileCount": extension_counts[".u"],
        "textureFileCacheCount": extension_counts[".tfc"],
        "manifestEngineCoreShaderCacheLogicalMatches": exact_provider_names,
        "extraPackages": extras,
        "providerDecision": "NO_SOURCE_ERA_CDO_SHADERCACHE_OR_NATIVE_PROVIDER",
    }


class PackageCache:
    def __init__(self, source_pack_root: Path, manifest_by_logical: dict[str, dict[str, Any]]) -> None:
        self.source_pack_root = source_pack_root
        self.by_logical = manifest_by_logical
        self.by_file_name = {
            str(row["physicalPackage"]).casefold(): row for row in manifest_by_logical.values()
        }
        self.packages: dict[str, Any] = {}

    def by_physical(self, file_name: str, expected_sha256: str) -> Any:
        row = self.by_file_name.get(file_name.casefold())
        require(isinstance(row, dict), f"physical package absent from manifest: {file_name}")
        require(row["sha256"] == expected_sha256, f"physical package SHA changed: {file_name}")
        key = file_name.casefold()
        if key not in self.packages:
            self.packages[key] = load_package(
                self.source_pack_root / row["relativePath"], LOSTARK_KR_AES_KEY
            )
        return self.packages[key]

    def by_logical_name(self, logical_name: str) -> tuple[Any, dict[str, Any]] | None:
        row = self.by_logical.get(logical_name.casefold())
        if not isinstance(row, dict):
            return None
        return self.by_physical(row["physicalPackage"], row["sha256"]), row


def find_named_export(package: Any, object_path: str, class_name: str) -> Any:
    leaf = object_path.rsplit(".", 1)[-1].casefold()
    matches = [
        entry
        for entry in package.exports
        if entry.object_name.casefold() == leaf
        and package_ref_name(entry.class_index, package.imports, package.exports).casefold()
        == class_name.casefold()
        and (
            (package_ref_path(entry.index + 1, package.imports, package.exports) or "")
            .casefold()
            .endswith(object_path.casefold())
        )
    ]
    require(len(matches) == 1, f"export match is not unique: {object_path}/{class_name}")
    return matches[0]


def current_candidate_snapshot(current_install_root: Path) -> dict[str, Any]:
    engine_path = current_install_root / EXPECTED_ENGINE_PACKAGE["relativePath"]
    core_path = current_install_root / EXPECTED_CORE_PACKAGE["relativePath"]
    wp_path = current_install_root / EXPECTED_CURRENT_WP_PACKAGE["relativePath"]
    require(engine_path.is_file() and core_path.is_file() and wp_path.is_file(), "current candidate missing")
    require(
        engine_path.stat().st_size == EXPECTED_ENGINE_PACKAGE["byteCount"]
        and raw_sha256(engine_path) == EXPECTED_ENGINE_PACKAGE["sha256"],
        "current Engine package identity changed",
    )
    require(
        wp_path.stat().st_size == EXPECTED_CURRENT_WP_PACKAGE["byteCount"]
        and raw_sha256(wp_path) == EXPECTED_CURRENT_WP_PACKAGE["sha256"],
        "current WP package identity changed",
    )

    package = load_package(engine_path, LOSTARK_KR_AES_KEY)
    cdo_specs = {
        "current-default-texture": ("Default__Texture", TARGET_TEXTURE_FIELDS),
        "current-default-texture2d": ("Default__Texture2D", TARGET_TEXTURE_FIELDS),
        "current-default-material": ("Default__Material", TARGET_RENDER_FIELDS),
        "current-default-decal-material": ("Default__DecalMaterial", TARGET_RENDER_FIELDS),
    }
    cdos: list[dict[str, Any]] = []
    for candidate_id, (object_name, target_fields) in cdo_specs.items():
        matches = [entry for entry in package.exports if entry.object_name == object_name]
        require(len(matches) == 1, f"current CDO not unique: {object_name}")
        entry = matches[0]
        serial = package.logical[
            entry.serial_offset : entry.serial_offset + entry.serial_size
        ]
        records, stream_start, stream_end = parse_property_records(
            serial, package.names, package.summary.version
        )
        by_name = {record["propertyName"].casefold(): record for record in records}
        fields = {}
        for field_name in target_fields:
            record = by_name.get(field_name)
            fields[field_name] = (
                {
                    "status": "CURRENT_SERIALIZED_EXPLICIT",
                    "property": compact_property(record),
                }
                if record is not None
                else {
                    "status": "OMITTED_FROM_CURRENT_CDO_EXPORT",
                    "sourceEraDefaultResolved": False,
                }
            )
        cdos.append(
            {
                "candidateId": candidate_id,
                "export": package_export_identity(package, entry),
                "propertyStreamStart": stream_start,
                "propertyStreamEnd": stream_end,
                "fields": fields,
                "revisionFidelity": "CURRENT_REVISION_CROSS_REVISION_CANDIDATE_ONLY",
                "admissibleAsSourceEra": False,
            }
        )
    engine_identity = {
        **EXPECTED_ENGINE_PACKAGE,
        "packageVersion": package.summary.version,
        "engineVersion": package.summary.engine_version,
        "exportCount": len(package.exports),
    }
    del package
    gc.collect()
    return {
        "rootKind": "CURRENT_INSTALLED_RELEASE_TREE",
        "enginePackage": engine_identity,
        "corePackage": {
            "relativePath": EXPECTED_CORE_PACKAGE["relativePath"],
            "byteCount": core_path.stat().st_size,
            "sha256": raw_sha256(core_path),
        },
        "classDefaultObjects": cdos,
        "currentOnlyWpTexturePackage": {
            **EXPECTED_CURRENT_WP_PACKAGE,
            "packageVersion": 868,
            "engineVersion": 12097,
        },
        "decision": "CURRENT_REVISION_VALUES_EXCLUDED_FROM_SOURCE_EXACT",
    }


def static_native_sets(
    shader_receipt: dict[str, Any], package_cache: PackageCache
) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for row in shader_receipt["recipeNativeKeys"]:
        recipe_id = row["recipeId"]
        decoded_expected = row.get("staticParameterSet")
        if decoded_expected is None:
            result[recipe_id] = {"nativeRow": row, "decoded": None, "tail": None}
            continue
        package = package_cache.by_physical(
            row["physicalPackage"], row["physicalPackageSha256"]
        )
        entry = package.exports[row["exportIndex"]]
        serial = package.logical[
            entry.serial_offset : entry.serial_offset + entry.serial_size
        ]
        require(raw_sha256(serial) == row["serialSha256"], "MIC serial SHA changed")
        _, property_end = shader_oracle.parse_tagged_properties(
            serial, package.names, package.summary.version
        )
        require(property_end == row["propertyStreamEnd"], "MIC property boundary changed")
        tail = serial[property_end:]
        require(raw_sha256(tail) == row["nativeTailSha256"], "MIC native tail changed")
        decoded = shader_oracle.parse_static_parameter_set(
            tail, row["baseMaterialNativeKeyOffsetInTail"], package.names
        )
        comparable = dict(decoded)
        comparable.pop("endOffset")
        require(comparable == decoded_expected, "MIC static parameter set changed")
        result[recipe_id] = {
            "nativeRow": row,
            "decoded": decoded,
            "tail": tail,
            "entry": entry,
        }
    require(len(result) == 27, "native recipe denominator changed")
    return result


def parent_expression_evidence(
    field: dict[str, Any], package_cache: PackageCache
) -> dict[str, Any]:
    provenance = field["provenance"]
    package = package_cache.by_physical(
        provenance["physicalPackage"], provenance["physicalPackageSha256"]
    )
    entry = package.exports[field["expressionExportIndex"]]
    serial = package.logical[
        entry.serial_offset : entry.serial_offset + entry.serial_size
    ]
    require(
        raw_sha256(serial) == provenance["graphExpressionSerialSha256"],
        "parent expression serial changed",
    )
    records, stream_start, stream_end = parse_property_records(
        serial, package.names, package.summary.version
    )
    by_name = {record["propertyName"].casefold(): record for record in records}
    for name in ("parametername", "defaultvalue", "expressionguid"):
        require(name in by_name, f"static parent field missing: {name}")
    parameter = by_name["parametername"]
    default = by_name["defaultvalue"]
    guid = by_name["expressionguid"]
    guid_value = guid["value"]
    require(
        str(parameter["value"]).casefold() == field["parameterName"].casefold()
        and default["value"] is field["value"]
        and isinstance(guid_value, dict)
        and len(str(guid_value.get("hex") or "")) == 32,
        "static parent projection changed",
    )
    return {
        "export": {
            "physicalPackage": provenance["physicalPackage"],
            "physicalPackageSha256": provenance["physicalPackageSha256"],
            **package_export_identity(package, entry),
            "propertyStreamStart": stream_start,
            "propertyStreamEnd": stream_end,
        },
        "parameterNameProperty": compact_property(parameter),
        "defaultValueProperty": compact_property(default),
        "expressionGuidProperty": compact_property(guid),
        "parameterName": parameter["value"],
        "parentDefaultValue": default["value"],
        "expressionGuidHex": guid_value["hex"],
    }


def build_static_rows(
    contract: dict[str, Any],
    runtime_receipt: dict[str, Any],
    native_sets: dict[str, dict[str, Any]],
    package_cache: PackageCache,
) -> list[dict[str, Any]]:
    recipes = {row["recipeId"]: row for row in contract["materialRecipes"]}
    fields = {
        field["fieldId"]: field
        for recipe in contract["materialRecipes"]
        for field in recipe["staticPermutation"]["parentDefaults"]
    }
    rows: list[dict[str, Any]] = []
    for baseline in runtime_receipt["materialFeasibilityMatrices"]["staticPermutationRows"]:
        field = fields[baseline["fieldId"]]
        recipe_id = baseline["materialRecipeId"]
        require(recipe_id in recipes, "static recipe missing")
        parent = parent_expression_evidence(field, package_cache)
        native = native_sets[recipe_id]
        decoded = native["decoded"]
        switches = [] if decoded is None else decoded["staticSwitchParameters"]
        name_matches = [
            row
            for row in switches
            if row["parameterName"].casefold() == parent["parameterName"].casefold()
        ]
        guid_matches = [
            row
            for row in name_matches
            if row["expressionGuidHex"] == parent["expressionGuidHex"]
        ]
        require(len(guid_matches) <= 1, "ambiguous static GUID join")
        matched = guid_matches[0] if guid_matches else None
        mic_evidence: dict[str, Any]
        source_value_acquired = False
        if matched is None:
            mic_evidence = {
                "staticParameterSetPresent": decoded is not None,
                "nameMatchCount": len(name_matches),
                "exactNameAndGuidMatchCount": 0,
                "decision": "NO_EXACT_GUID_NATIVE_ENTRY",
            }
            source_decision = "SOURCE_EXACT_PARENT_DEFAULT_ONLY_NATIVE_ENTRY_ABSENT"
        else:
            tail = native["tail"]
            native_row = native["nativeRow"]
            entry_offset = matched["entryOffset"]
            require(isinstance(tail, bytes), "MIC tail missing")
            value_ordinal, override_ordinal = struct.unpack_from("<II", tail, entry_offset + 8)
            require(
                bool(value_ordinal) is matched["value"]
                and bool(override_ordinal) is matched["bOverride"]
                and tail[entry_offset + 16 : entry_offset + 32].hex()
                == matched["expressionGuidHex"],
                "MIC raw entry disagrees with decoded projection",
            )
            mic_evidence = {
                "staticParameterSetPresent": True,
                "nameMatchCount": len(name_matches),
                "exactNameAndGuidMatchCount": 1,
                "nativeTail": {
                    "physicalPackage": native_row["physicalPackage"],
                    "physicalPackageSha256": native_row["physicalPackageSha256"],
                    "exportIndex": native_row["exportIndex"],
                    "serialSha256": native_row["serialSha256"],
                    "propertyStreamEnd": native_row["propertyStreamEnd"],
                    "nativeTailByteCount": native_row["nativeTailByteCount"],
                    "nativeTailSha256": native_row["nativeTailSha256"],
                    "staticParameterSetOffset": decoded["offset"],
                    "staticParameterSetByteSize": decoded["byteSize"],
                    "staticParameterSetRawSha256": decoded["rawSha256"],
                    "staticParameterSetSemanticSha256": decoded["semanticSha256"],
                },
                "entry": {
                    **matched,
                    "rawNameFNameHex": tail[entry_offset : entry_offset + 8].hex(),
                    "rawValueUint32Hex": tail[entry_offset + 8 : entry_offset + 12].hex(),
                    "rawOverrideUint32Hex": tail[entry_offset + 12 : entry_offset + 16].hex(),
                    "rawExpressionGuidHex": tail[entry_offset + 16 : entry_offset + 32].hex(),
                    "serialRelativeEntryOffset": native_row["propertyStreamEnd"]
                    + entry_offset,
                },
                "decision": (
                    "EXACT_GUID_INSTANCE_OVERRIDE_ENTRY"
                    if matched["bOverride"]
                    else "EXACT_GUID_NONOVERRIDE_ENTRY"
                ),
            }
            if matched["bOverride"]:
                source_value_acquired = True
                source_decision = "SOURCE_EXACT_INSTANCE_OVERRIDE_VALUE_ACQUIRED"
            else:
                require(
                    matched["value"] is parent["parentDefaultValue"],
                    "nonoverride native value disagrees with parent default",
                )
                source_decision = (
                    "SOURCE_EXACT_NONOVERRIDE_ENTRY_OBSERVED_"
                    "INHERITANCE_SEMANTICS_UNVERIFIED"
                )
        rows.append(
            {
                "matrixRowId": baseline["matrixRowId"],
                "materialRecipeId": recipe_id,
                "materialOccurrenceIds": baseline["materialOccurrenceIds"],
                "fieldId": field["fieldId"],
                "fieldKind": "STATIC_PERMUTATION_SELECTION",
                "parameterName": field["parameterName"],
                "bindingOriginAndOwner": baseline["bindingOriginAndOwner"],
                "parentExpression": parent,
                "micNativeSelection": mic_evidence,
                "sourceValueAcquired": source_value_acquired,
                "sourceValueDecision": source_decision,
                "rendererConsumption": baseline["rendererConsumption"],
                "consumerPilot": {
                    "pilotFixtureIds": baseline["pilotFixtureIds"],
                    "decision": baseline["pilotDecision"],
                    "sourceSpecificActualOutputVerified": False,
                },
                "numericOracleInputDomain": baseline["numericOracleInputDomain"],
                "numericOracleExpectedOutput": baseline["numericOracleExpectedOutput"],
                "numericTolerance": baseline["numericTolerance"],
                "executionReady": False,
                "owner": baseline["owner"],
                "finalRuntimeOwner": baseline["finalRuntimeOwner"],
                "remainingBlockers": sorted(
                    {
                        "STATIC_PERMUTATION_CONSUMER_OUTPUT_PILOT_MISSING",
                        "FINAL_RUNTIME_CONSUMER_NOT_IMPLEMENTED",
                        *(
                            []
                            if source_value_acquired
                            else ["STATIC_SELECTION_SOURCE_VALUE_NOT_ACQUIRED"]
                        ),
                    }
                ),
            }
        )
    require(len(rows) == 94, "strict static denominator changed")
    return rows


def build_render_rows(
    runtime_receipt: dict[str, Any],
    render_receipt: dict[str, Any],
    current_snapshot: dict[str, Any],
) -> list[dict[str, Any]]:
    raw_exports = {row["evidenceId"]: row for row in render_receipt["exports"]}
    cdo_by_id = {
        row["candidateId"]: row for row in current_snapshot["classDefaultObjects"]
    }
    rows: list[dict[str, Any]] = []
    for baseline in runtime_receipt["materialFeasibilityMatrices"]["renderStateRows"]:
        field_name = baseline["fieldId"].split(":", 1)[1]
        require(field_name in TARGET_RENDER_FIELDS, "unexpected render field")
        parent_id = baseline["parentIdentity"]["identity"]["rawExportEvidenceId"]
        parent_class = raw_exports[parent_id]["className"].casefold()
        candidate_id = (
            "current-default-decal-material"
            if parent_class == "decalmaterial"
            else "current-default-material"
        )
        candidate = cdo_by_id[candidate_id]
        rows.append(
            {
                "matrixRowId": baseline["matrixRowId"],
                "materialRecipeId": baseline["materialRecipeId"],
                "materialOccurrenceIds": baseline["materialOccurrenceIds"],
                "fieldId": baseline["fieldId"],
                "fieldKind": "RENDER_STATE_DEFAULT",
                "fieldName": field_name,
                "bindingOriginAndOwner": baseline["bindingOriginAndOwner"],
                "defaultChain": {
                    "instanceRecordIdentity": baseline["instanceRecordIdentity"],
                    "parentIdentity": baseline["parentIdentity"],
                    "nestedDefaultIdentity": baseline["nestedDefaultIdentity"],
                    "sourceRevisionClassCdo": {
                        "available": False,
                        "outcome": "SOURCE_REVISION_CDO_AND_NATIVE_DEFAULT_PROVIDER_NOT_ACQUIRED",
                    },
                    "currentOnlyCdoCandidate": {
                        "candidateId": candidate_id,
                        "field": candidate["fields"][field_name],
                        "admissibleAsSourceEra": False,
                    },
                },
                "sourceValueAcquired": False,
                "sourceValueDecision": "BLOCKED_SOURCE_REVISION_DEFAULT_PROVIDER_UNAVAILABLE",
                "rendererConsumption": baseline["rendererConsumption"],
                "consumerPilot": {
                    "pilotFixtureIds": baseline["pilotFixtureIds"],
                    "decision": baseline["pilotDecision"],
                    "consumerSemanticsOnly": bool(baseline["pilotFixtureIds"]),
                    "sourceSpecificValueVerified": False,
                },
                "numericOracleInputDomain": baseline["numericOracleInputDomain"],
                "numericOracleExpectedOutput": baseline["numericOracleExpectedOutput"],
                "numericTolerance": baseline["numericTolerance"],
                "executionReady": False,
                "owner": baseline["owner"],
                "finalRuntimeOwner": baseline["finalRuntimeOwner"],
                "remainingBlockers": [
                    "RENDER_STATE_SOURCE_DEFAULT_PROVENANCE_UNRESOLVED",
                    "SOURCE_REVISION_CDO_OR_NATIVE_CAPTURE_MISSING",
                    "FINAL_RUNTIME_CONSUMER_NOT_IMPLEMENTED",
                ],
            }
        )
    require(len(rows) == 89, "strict render denominator changed")
    return rows


def texture_export_evidence(
    logical_texture_path: str,
    package_cache: PackageCache,
    current_install_root: Path,
) -> dict[str, Any]:
    logical_package, object_path = logical_texture_path.split(".", 1)
    source = package_cache.by_logical_name(logical_package)
    revision_fidelity = "SOURCE_ARCHIVE_EXACT_TEXTURE_EXPORT"
    if source is None:
        require(logical_package.casefold() == "wp_mn_lrcn_01", "unknown missing texture package")
        path = current_install_root / EXPECTED_CURRENT_WP_PACKAGE["relativePath"]
        package = load_package(path, LOSTARK_KR_AES_KEY)
        package_row = {
            "physicalPackage": path.name,
            "sha256": raw_sha256(path),
            "byteSize": path.stat().st_size,
        }
        revision_fidelity = "CURRENT_REVISION_CROSS_REVISION_CANDIDATE_ONLY"
    else:
        package, package_row = source
    entry = find_named_export(package, object_path, "texture2d")
    serial = package.logical[
        entry.serial_offset : entry.serial_offset + entry.serial_size
    ]
    records, stream_start, stream_end = parse_property_records(
        serial, package.names, package.summary.version
    )
    by_name = {record["propertyName"].casefold(): record for record in records}
    fields: dict[str, Any] = {}
    for name in TARGET_TEXTURE_FIELDS:
        record = by_name.get(name)
        fields[name] = (
            {"status": "SERIALIZED_EXPLICIT", "property": compact_property(record)}
            if record is not None
            else {
                "status": "OMITTED_FROM_EXPORT",
                "fidelity": "UNRESOLVED_DEFAULT_PROVENANCE",
            }
        )
    result = {
        "logicalTexturePath": logical_texture_path,
        "physicalPackage": package_row["physicalPackage"],
        "physicalPackageByteCount": package_row["byteSize"],
        "physicalPackageSha256": package_row["sha256"],
        "packageVersion": package.summary.version,
        "engineVersion": package.summary.engine_version,
        "export": package_export_identity(package, entry),
        "propertyStreamStart": stream_start,
        "propertyStreamEnd": stream_end,
        "fields": fields,
        "revisionFidelity": revision_fidelity,
        "admissibleAsSourceEra": revision_fidelity.startswith("SOURCE_ARCHIVE"),
    }
    if source is None:
        del package
        gc.collect()
    return result


def validate_previous_exact_sampler(
    binding: dict[str, Any], texture: dict[str, Any]
) -> None:
    previous = binding["sourceTextureEvidence"]["rawSamplerFields"]
    for name in TARGET_TEXTURE_FIELDS:
        expected = previous[name]
        actual = texture["fields"][name]
        require(expected["status"] == actual["status"], "exact sampler field status changed")
        if expected["status"] == "SERIALIZED_EXPLICIT":
            require(
                expected["recordSha256"] == actual["property"]["recordSha256"]
                and expected["encodedValueHex"] == actual["property"]["encodedValueHex"],
                "exact sampler raw property changed",
            )


def build_sampler_rows(
    contract: dict[str, Any],
    runtime_receipt: dict[str, Any],
    package_cache: PackageCache,
    current_install_root: Path,
) -> list[dict[str, Any]]:
    rejected_by_field = {
        row["inputFieldId"]: row for row in contract["rejectedSamplerBindings"]
    }
    require(len(rejected_by_field) == 4, "rejected sampler denominator changed")
    seeds: list[dict[str, Any]] = []
    for baseline in runtime_receipt["materialFeasibilityMatrices"]["strictSamplerRows"]:
        previous = rejected_by_field.get(baseline["fieldId"])
        seeds.append(
            {
                "baselineKind": (
                    "PREVIOUSLY_ADMITTED_EXACT_REAUDIT"
                    if previous is not None
                    else (
                        "NEW_SELF_DEFAULT_UNRESOLVED_5"
                        if (baseline.get("bindingOriginAndOwner") or {}).get(
                            "bindingOrigin"
                        )
                        == "SELF_DEFAULT"
                        else "PREVIOUSLY_BLOCKED_68"
                    )
                ),
                "matrixRowId": baseline["matrixRowId"],
                "materialRecipeId": baseline["materialRecipeId"],
                "materialOccurrenceIds": baseline["materialOccurrenceIds"],
                "fieldId": baseline["fieldId"],
                "fieldKind": baseline["fieldKind"],
                "bindingOriginAndOwner": baseline["bindingOriginAndOwner"],
                "logicalTexturePath": baseline["instanceRecordIdentity"]["textureObjectPath"],
                "rendererConsumption": baseline["rendererConsumption"],
                "pilotFixtureIds": baseline["pilotFixtureIds"],
                "pilotDecision": baseline["pilotDecision"],
                "numericOracleInputDomain": baseline["numericOracleInputDomain"],
                "numericOracleExpectedOutput": baseline["numericOracleExpectedOutput"],
                "numericTolerance": baseline["numericTolerance"],
                "owner": baseline["owner"],
                "finalRuntimeOwner": baseline["finalRuntimeOwner"],
                "previousExactBinding": previous,
            }
        )
    require(len(seeds) == 77, "strict sampler seed denominator changed")

    texture_cache: dict[str, dict[str, Any]] = {}
    rows: list[dict[str, Any]] = []
    for seed in seeds:
        texture_path = seed["logicalTexturePath"]
        key = texture_path.casefold()
        if key not in texture_cache:
            texture_cache[key] = texture_export_evidence(
                texture_path, package_cache, current_install_root
            )
        texture = texture_cache[key]
        previous = seed.pop("previousExactBinding")
        if previous is not None:
            validate_previous_exact_sampler(previous, texture)
        explicit_fields = [
            name
            for name in TARGET_TEXTURE_FIELDS
            if texture["fields"][name]["status"] == "SERIALIZED_EXPLICIT"
        ]
        source_exact_fields = explicit_fields if texture["admissibleAsSourceEra"] else []
        current_only_fields = [] if texture["admissibleAsSourceEra"] else explicit_fields
        full_descriptor_source_exact = False
        source_decision = (
            "BLOCKED_SOURCE_TEXTURE_PACKAGE_NOT_IN_ARCHIVE"
            if not texture["admissibleAsSourceEra"]
            else "BLOCKED_FULL_SAMPLER_DESCRIPTOR_DEFAULT_PROVENANCE_UNRESOLVED"
        )
        row = {
            **seed,
            "fieldKind": seed.get("fieldKind", "DIRECT_TEXTURE_SAMPLER"),
            "textureExportEvidence": texture,
            "partialSourceExactFields": source_exact_fields,
            "partialCurrentOnlyFields": current_only_fields,
            "fullDescriptorSourceExact": full_descriptor_source_exact,
            "sourceValueAcquired": False,
            "sourceValueDecision": source_decision,
            "previousAdmission": (
                "SOURCE_EXACT_SAMPLER"
                if seed["baselineKind"] == "PREVIOUSLY_ADMITTED_EXACT_REAUDIT"
                else "BLOCKED"
            ),
            "strictReauditDecision": "BLOCKED",
            "consumerPilot": {
                "pilotFixtureIds": seed.pop("pilotFixtureIds"),
                "decision": seed.pop("pilotDecision"),
                "consumerSemanticsOnly": True,
                "sourceSpecificFullDescriptorVerified": False,
            },
            "executionReady": False,
            "remainingBlockers": [
                "FULL_SAMPLER_DESCRIPTOR_SOURCE_VALUE_UNRESOLVED",
                "SOURCE_REVISION_TEXTURE_CDO_OR_CONFIG_MISSING",
                "FINAL_RUNTIME_CONSUMER_NOT_IMPLEMENTED",
            ],
        }
        rows.append(row)
    require(len(rows) == 77, "strict sampler denominator changed")
    return rows


def cluster_summary(
    render_rows: list[dict[str, Any]],
    static_rows: list[dict[str, Any]],
    sampler_rows: list[dict[str, Any]],
) -> dict[str, Any]:
    return {
        "renderByField": dict(
            sorted(Counter(row["fieldName"] for row in render_rows).items())
        ),
        "staticByDecision": dict(
            sorted(Counter(row["sourceValueDecision"] for row in static_rows).items())
        ),
        "staticRecipeCount": len({row["materialRecipeId"] for row in static_rows}),
        "samplerByPreviousAdmission": dict(
            sorted(Counter(row["previousAdmission"] for row in sampler_rows).items())
        ),
        "samplerBindingOriginCounts": dict(
            sorted(
                Counter(
                    row["bindingOriginAndOwner"]["bindingOrigin"]
                    for row in sampler_rows
                ).items()
            )
        ),
        "samplerUniqueTextureCount": len(
            {row["logicalTexturePath"].casefold() for row in sampler_rows}
        ),
        "samplerSourceArchiveTextureCount": len(
            {
                row["logicalTexturePath"].casefold()
                for row in sampler_rows
                if row["textureExportEvidence"]["admissibleAsSourceEra"]
            }
        ),
        "samplerCurrentOnlyTextureCount": len(
            {
                row["logicalTexturePath"].casefold()
                for row in sampler_rows
                if not row["textureExportEvidence"]["admissibleAsSourceEra"]
            }
        ),
    }


def build_receipt(
    contract: dict[str, Any],
    render_receipt: dict[str, Any],
    shader_receipt: dict[str, Any],
    contract_path: Path,
    render_receipt_path: Path,
    shader_receipt_path: Path,
    source_archive_root: Path,
    source_pack_root: Path,
    current_install_root: Path,
) -> dict[str, Any]:
    require(contract.get("contractSha256"), "typed contract digest missing")
    require(render_receipt.get("receiptSha256"), "render receipt digest missing")
    require(shader_receipt.get("receiptSha256"), "shader receipt digest missing")
    manifest_by_logical, manifest_identity = manifest_index(source_pack_root)
    package_cache = PackageCache(source_pack_root, manifest_by_logical)
    native_keys = sorted(
        {
            bytes.fromhex(row["nativeStateKeyCandidateHex"])
            for row in shader_receipt["materialKeySearch"]
        }
    )
    require(len(native_keys) == 23, "native-state-key denominator changed")

    archive = source_archive_inventory(
        source_archive_root, source_pack_root, manifest_by_logical, native_keys
    )
    current_snapshot = current_candidate_snapshot(current_install_root)
    native_sets = static_native_sets(shader_receipt, package_cache)
    baseline_runtime = {
        "materialFeasibilityMatrices": (
            material_runtime_oracle.build_material_feasibility_matrices(
                contract,
                shader_receipt,
                warp_state_verification=None,
                source_value_acquisition=None,
            )
        )
    }
    render_rows = build_render_rows(
        baseline_runtime, render_receipt, current_snapshot
    )
    static_rows = build_static_rows(
        contract, baseline_runtime, native_sets, package_cache
    )
    sampler_rows = build_sampler_rows(
        contract, baseline_runtime, package_cache, current_install_root
    )

    exact4 = [
        {
            "matrixRowId": row["matrixRowId"],
            "fieldId": row["fieldId"],
            "logicalTexturePath": row["logicalTexturePath"],
            "rawSamplerFields": row["textureExportEvidence"]["fields"],
            "strictReauditDecision": row["strictReauditDecision"],
        }
        for row in sampler_rows
        if row["baselineKind"] == "PREVIOUSLY_ADMITTED_EXACT_REAUDIT"
    ]
    require(len(exact4) == 4, "exact sampler reaudit denominator changed")

    summary = {
        "renderStateRowCount": len(render_rows),
        "renderStateSourceValueAcquiredCount": sum(
            row["sourceValueAcquired"] for row in render_rows
        ),
        "staticPermutationRowCount": len(static_rows),
        "staticExactGuidJoinCount": sum(
            row["micNativeSelection"]["exactNameAndGuidMatchCount"]
            for row in static_rows
        ),
        "staticOverrideTrueSourceValueAcquiredCount": sum(
            row["sourceValueAcquired"] for row in static_rows
        ),
        "staticNonoverrideSemanticsUnverifiedCount": sum(
            "NONOVERRIDE_ENTRY" in row["sourceValueDecision"] for row in static_rows
        ),
        "staticNoExactGuidEntryCount": sum(
            row["micNativeSelection"]["exactNameAndGuidMatchCount"] == 0
            for row in static_rows
        ),
        "previousDirectUnprovenSamplerRowCount": 68,
        "newSelfDefaultSamplerRowCount": sum(
            row["baselineKind"] == "NEW_SELF_DEFAULT_UNRESOLVED_5"
            for row in sampler_rows
        ),
        "previouslyAdmittedExactSamplerReauditCount": len(exact4),
        "previouslyAdmittedExactSamplerBlockedCount": sum(
            row["strictReauditDecision"] == "BLOCKED" for row in exact4
        ),
        "strictSamplerRowCount": len(sampler_rows),
        "strictSamplerSourceValueAcquiredCount": sum(
            row["sourceValueAcquired"] for row in sampler_rows
        ),
        "strictExecutionRowCount": len(render_rows) + len(static_rows) + len(sampler_rows),
        "strictExecutionReadyCount": sum(
            row["executionReady"]
            for row in render_rows + static_rows + sampler_rows
        ),
        "valueProvenanceDelta": {
            "renderState": 0,
            "staticPermutation": 23,
            "fullSamplerDescriptor": 0,
        },
        "executionReadinessDelta": 0,
        "productCount": 0,
        "renderRowSetSha256": canonical_sha256(render_rows),
        "staticRowSetSha256": canonical_sha256(static_rows),
        "strictSamplerRowSetSha256": canonical_sha256(sampler_rows),
        "invalidatedPreviousExactSamplerSetSha256": canonical_sha256(exact4),
    }
    require(
        summary["renderStateRowCount"] == 89
        and summary["staticPermutationRowCount"] == 94
        and summary["staticExactGuidJoinCount"] == 66
        and summary["staticOverrideTrueSourceValueAcquiredCount"] == 23
        and summary["staticNonoverrideSemanticsUnverifiedCount"] == 43
        and summary["staticNoExactGuidEntryCount"] == 28
        and summary["newSelfDefaultSamplerRowCount"] == 5
        and summary["strictSamplerRowCount"] == 77
        and summary["previouslyAdmittedExactSamplerBlockedCount"] == 4
        and summary["strictExecutionRowCount"] == 260
        and summary["strictExecutionReadyCount"] == 0,
        "source-value acquisition summary changed",
    )

    receipt = {
        "schema": "lostark.artist-31470-material-source-value-acquisition-receipt",
        "formatVersion": 2,
        "root": CONTRACT_ROOT,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "scope": "SOURCE_VALUE_ACQUISITION_ONLY",
        "source": [
            source_evidence(contract_path, "TYPED_MATERIAL_EVIDENCE_CONTRACT"),
            source_evidence(render_receipt_path, "RAW_RENDER_STATE_EVIDENCE_RECEIPT"),
            source_evidence(shader_receipt_path, "SHADER_CACHE_NATIVE_TAIL_RECEIPT"),
            source_evidence(SCRIPT_PATH, "SOURCE_VALUE_ACQUISITION_GENERATOR", json_input=False),
            source_evidence(
                APPROVAL_PATH,
                "INDEPENDENT_MATERIAL_EVIDENCE_APPROVAL",
                json_input=False,
            ),
            source_evidence(
                SCRIPT_PATH.with_name(
                    "build_artist_31470_material_runtime_oracle.py"
                ),
                "MATERIAL_FEASIBILITY_MATRIX_BUILDER",
                json_input=False,
            ),
            source_evidence(
                SCRIPT_PATH.with_name("extract_artist_31470_material_render_state.py"),
                "UE3_TAGGED_PROPERTY_PARSER",
                json_input=False,
            ),
            source_evidence(
                SCRIPT_PATH.with_name("extract_artist_31470_shader_cache_oracle.py"),
                "UE3_MIC_NATIVE_TAIL_PARSER",
                json_input=False,
            ),
        ],
        "externalArtifactSearch": {
            "scopeBoundary": {
                "claim": "ACCESSIBLE_LOCAL_AND_REMOTE_SCOPE_ONLY",
                "globalExhaustionClaim": False,
                "volumeShadowCopy": {
                    "status": "PERMISSION_UNCHECKED",
                    "admissionInput": False,
                },
            },
            "sourcePackManifest": manifest_identity,
            "sourceArchive": archive,
            "currentRevisionCandidates": current_snapshot,
            "driverShaderCaches": copy.deepcopy(DRIVER_CACHE_AUDIT),
            "gitAndRemote": copy.deepcopy(GIT_AND_REMOTE_AUDIT),
            "controlledRuntimeCapture": copy.deepcopy(
                CONTROLLED_CAPTURE_ASSESSMENT
            ),
        },
        "provenanceClusters": cluster_summary(render_rows, static_rows, sampler_rows),
        "matrices": {
            "renderStateRows": render_rows,
            "staticPermutationRows": static_rows,
            "strictSamplerRows": sampler_rows,
        },
        "coordinatedCorrectiveRequirements": [
            {
                "requirementId": "strict-sampler-denominator-77",
                "decision": "REQUIRED",
                "baselineDirectUnproven": 68,
                "previousExactReaudit": 4,
                "strictDenominator": 77,
                "bindingOriginCounts": {"INSTANCE_OVERRIDE": 71, "PARENT_DEFAULT": 1},
                "shared627ReceiptChangedHere": True,
            },
            {
                "requirementId": "static-blanket-reason-retirement",
                "decision": "REQUIRED",
                "exactGuidOverrideTrue": 23,
                "exactGuidNonoverrideSemanticsUnverified": 43,
                "noExactGuidEntry": 28,
                "shared627ReceiptChangedHere": True,
            },
            {
                "requirementId": "artist-resource-manifest-texture-selector",
                "decision": "REQUIRED",
                "logicalTexturePath": "fx_tex_04.fx_h_wave_01",
                "currentManifestSkillIds": [31920],
                "requiredSkillId": 31470,
                "sharedManifestChangedHere": False,
            },
        ],
        "minimumMissingExternalArtifacts": {
            "renderState": CONTROLLED_CAPTURE_ASSESSMENT["minimumExternalArtifact"],
            "staticPermutation": (
                "source-revision FStaticParameterSet inheritance semantics or an "
                "authenticated native/runtime output capture for the 43 nonoverride "
                "and 28 no-exact-GUID rows"
            ),
            "sampler": (
                "source-revision Texture/Texture2D CDO native defaults, exact "
                "TextureGroup filter configuration, and the missing WP_MN_LRCN_01 "
                "source package or authenticated capture"
            ),
        },
        "summary": summary,
        "admission": {
            "acquisitionReceiptEvidenceIntegrity": True,
            "upstreamMaterialEvidenceIntegrity": True,
            "sourceValueProviderPartial": True,
            "executionReady": False,
            "product": False,
            "r2Entry": False,
            "decision": "R0_BLOCK_R2_NO_GO",
            "blockers": [
                "RENDER_STATE_89_SOURCE_VALUES_UNRESOLVED",
                "STATIC_71_SOURCE_SELECTIONS_UNRESOLVED",
                "STRICT_SAMPLER_77_FULL_DESCRIPTORS_UNRESOLVED",
                "SOURCE_SPECIFIC_ACTUAL_OUTPUT_READINESS_ZERO",
                "FINAL_RUNTIME_CONSUMERS_NOT_IMPLEMENTED",
            ],
        },
    }
    receipt["receiptSha256"] = canonical_sha256(receipt)
    return receipt


def _require_sha256(value: Any, label: str) -> str:
    require(
        isinstance(value, str)
        and len(value) == 64
        and value == value.lower()
        and value != "0" * 64
        and all(character in "0123456789abcdef" for character in value),
        f"{label} SHA-256 identity is invalid",
    )
    return value


def _strict_sampler_membership_from_contract(
    contract: dict[str, Any],
) -> list[dict[str, Any]]:
    membership: list[dict[str, Any]] = []
    for recipe in contract.get("materialRecipes") or []:
        recipe_id = recipe["recipeId"]
        inputs = recipe.get("inputs") or {}
        for source_section in ("textureOverrides", "parentDefaults"):
            for field in inputs.get(source_section) or []:
                if field.get("fieldKind") != "texture":
                    continue
                sampler = field.get("sampler") or {}
                if sampler.get("fidelity") not in {
                    "UNRESOLVED",
                    "UNRESOLVED_SAMPLER_PROVENANCE",
                }:
                    continue
                if source_section == "parentDefaults":
                    origin = field.get("bindingOrigin")
                    fidelity = sampler.get("fidelity")
                    if not (
                        (
                            origin == "PARENT_DEFAULT"
                            and fidelity == "UNRESOLVED_SAMPLER_PROVENANCE"
                        )
                        or (origin == "SELF_DEFAULT" and fidelity == "UNRESOLVED")
                    ):
                        continue
                membership.append(
                    {
                        "materialRecipeId": recipe_id,
                        "fieldId": field["fieldId"],
                        "logicalTexturePath": field["value"],
                        "bindingOrigin": field["bindingOrigin"],
                        "sourceSection": source_section,
                    }
                )
    return sorted(membership, key=lambda row: row["fieldId"])


def _validate_explicit_sampler_property(
    field_name: str, field: dict[str, Any]
) -> None:
    require(
        set(field) == {"status", "property"}
        and field.get("status") == "SERIALIZED_EXPLICIT",
        f"strict sampler explicit field schema changed: {field_name}",
    )
    prop = field.get("property")
    expected_property_keys = {
        "propertyName",
        "arrayIndex",
        "propertyType",
        "structType",
        "declaredDataSize",
        "serializedPayloadSize",
        "tagOffset",
        "valueOffset",
        "recordEndOffset",
        "value",
        "encodedValueHex",
        "encodedValueSha256",
        "recordSha256",
    }
    require(
        isinstance(prop, dict)
        and set(prop) == expected_property_keys
        and prop.get("propertyName") == field_name
        and type(prop.get("arrayIndex")) is int
        and prop["arrayIndex"] == 0
        and prop.get("structType") is None
        and all(
            type(prop.get(name)) is int and prop[name] >= 0
            for name in (
                "declaredDataSize",
                "serializedPayloadSize",
                "tagOffset",
                "valueOffset",
                "recordEndOffset",
            )
        )
        and prop["tagOffset"] < prop["valueOffset"] < prop["recordEndOffset"],
        f"strict sampler raw property layout changed: {field_name}",
    )
    encoded_hex = prop.get("encodedValueHex")
    require(
        isinstance(encoded_hex, str) and len(encoded_hex) % 2 == 0,
        f"strict sampler encoded value changed: {field_name}",
    )
    try:
        encoded = bytes.fromhex(encoded_hex)
    except ValueError as error:
        raise ValueError(
            f"strict sampler encoded value is invalid: {field_name}"
        ) from error
    require(
        hashlib.sha256(encoded).hexdigest() == prop.get("encodedValueSha256"),
        f"strict sampler encoded value digest changed: {field_name}",
    )
    _require_sha256(prop.get("recordSha256"), f"strict sampler {field_name} record")
    if field_name == "srgb":
        require(
            prop.get("propertyType") == "boolproperty"
            and prop.get("declaredDataSize") == 0
            and prop.get("serializedPayloadSize") == 0
            and encoded in {b"\x00", b"\x01"}
            and type(prop.get("value")) is bool
            and prop["value"] is bool(encoded[0]),
            "strict sampler sRGB decoded/raw semantics changed",
        )
    else:
        require(
            prop.get("propertyType") == "byteproperty"
            and prop.get("declaredDataSize") == 8
            and prop.get("serializedPayloadSize") == 8
            and len(encoded) == 8
            and isinstance(prop.get("value"), str)
            and prop["value"],
            f"strict sampler byte-property decoded/raw semantics changed: {field_name}",
        )


def _decode_hex(value: Any, byte_count: int, label: str) -> bytes:
    require(
        isinstance(value, str) and len(value) == byte_count * 2,
        f"{label} encoded bytes changed",
    )
    try:
        decoded = bytes.fromhex(value)
    except ValueError as error:
        raise ValueError(f"{label} encoded bytes are invalid") from error
    require(len(decoded) == byte_count, f"{label} byte count changed")
    return decoded


def _validate_parent_static_expression(row: dict[str, Any]) -> None:
    parent = row.get("parentExpression")
    require(isinstance(parent, dict), "static parent expression is missing")
    parameter_prop = parent.get("parameterNameProperty")
    default_prop = parent.get("defaultValueProperty")
    guid_prop = parent.get("expressionGuidProperty")
    require(
        isinstance(parameter_prop, dict)
        and parameter_prop.get("propertyName") == "parametername"
        and parameter_prop.get("propertyType") == "nameproperty"
        and parameter_prop.get("structType") is None
        and type(parameter_prop.get("arrayIndex")) is int
        and parameter_prop["arrayIndex"] == 0
        and parameter_prop.get("declaredDataSize") == 8
        and parameter_prop.get("serializedPayloadSize") == 8
        and isinstance(parameter_prop.get("value"), str)
        and parameter_prop["value"]
        and parent.get("parameterName") == parameter_prop["value"]
        and row.get("parameterName") == parent["parameterName"],
        "static parent parameter-name projection changed",
    )
    parameter_bytes = _decode_hex(
        parameter_prop.get("encodedValueHex"),
        8,
        "static parent parameter name",
    )
    require(
        hashlib.sha256(parameter_bytes).hexdigest()
        == parameter_prop.get("encodedValueSha256"),
        "static parent parameter-name digest changed",
    )
    _require_sha256(
        parameter_prop.get("recordSha256"),
        "static parent parameter-name record",
    )

    require(
        isinstance(default_prop, dict)
        and default_prop.get("propertyName") == "defaultvalue"
        and default_prop.get("propertyType") == "boolproperty"
        and default_prop.get("structType") is None
        and type(default_prop.get("arrayIndex")) is int
        and default_prop["arrayIndex"] == 0
        and default_prop.get("declaredDataSize") == 0
        and default_prop.get("serializedPayloadSize") == 0,
        "static parent default property schema changed",
    )
    default_bytes = _decode_hex(
        default_prop.get("encodedValueHex"),
        1,
        "static parent default",
    )
    require(
        default_bytes in {b"\x00", b"\x01"}
        and type(default_prop.get("value")) is bool
        and default_prop["value"] is bool(default_bytes[0])
        and parent.get("parentDefaultValue") is default_prop["value"]
        and hashlib.sha256(default_bytes).hexdigest()
        == default_prop.get("encodedValueSha256"),
        "static parent default decoded/raw semantics changed",
    )
    _require_sha256(
        default_prop.get("recordSha256"),
        "static parent default record",
    )

    require(
        isinstance(guid_prop, dict)
        and guid_prop.get("propertyName") == "expressionguid"
        and guid_prop.get("propertyType") == "structproperty"
        and guid_prop.get("structType") == "guid"
        and type(guid_prop.get("arrayIndex")) is int
        and guid_prop["arrayIndex"] == 0
        and guid_prop.get("declaredDataSize") == 16
        and guid_prop.get("serializedPayloadSize") == 16,
        "static parent ExpressionGUID property schema changed",
    )
    guid_bytes = _decode_hex(
        guid_prop.get("encodedValueHex"),
        16,
        "static parent ExpressionGUID",
    )
    guid_value = guid_prop.get("value")
    require(
        isinstance(guid_value, dict)
        and set(guid_value) == {"size", "hex"}
        and guid_value.get("size") == 16
        and guid_value.get("hex") == guid_bytes.hex()
        and parent.get("expressionGuidHex") == guid_bytes.hex()
        and hashlib.sha256(guid_bytes).hexdigest()
        == guid_prop.get("encodedValueSha256"),
        "static parent ExpressionGUID decoded/raw semantics changed",
    )
    _require_sha256(
        guid_prop.get("recordSha256"),
        "static parent ExpressionGUID record",
    )


def _validate_static_selection_row(row: dict[str, Any]) -> None:
    _validate_parent_static_expression(row)
    selection = row.get("micNativeSelection")
    require(
        isinstance(selection, dict)
        and type(selection.get("nameMatchCount")) is int
        and selection["nameMatchCount"] >= 0
        and type(selection.get("exactNameAndGuidMatchCount")) is int
        and selection["exactNameAndGuidMatchCount"] in {0, 1}
        and type(selection.get("staticParameterSetPresent")) is bool,
        "static MIC selection denominator changed",
    )
    parent = row["parentExpression"]
    exact_count = selection["exactNameAndGuidMatchCount"]
    require(
        row.get("executionReady") is False
        and (row.get("consumerPilot") or {}).get(
            "sourceSpecificActualOutputVerified"
        )
        is False,
        "static execution admission changed",
    )
    if exact_count == 0:
        require(
            "entry" not in selection
            and "nativeTail" not in selection
            and selection.get("decision") == "NO_EXACT_GUID_NATIVE_ENTRY"
            and row.get("sourceValueAcquired") is False
            and row.get("sourceValueDecision")
            == "SOURCE_EXACT_PARENT_DEFAULT_ONLY_NATIVE_ENTRY_ABSENT"
            and "STATIC_SELECTION_SOURCE_VALUE_NOT_ACQUIRED"
            in (row.get("remainingBlockers") or []),
            "static unmatched selection admission changed",
        )
        return

    entry = selection.get("entry")
    native_tail = selection.get("nativeTail")
    require(
        isinstance(entry, dict)
        and isinstance(native_tail, dict)
        and isinstance(entry.get("parameterName"), str)
        and entry["parameterName"].casefold()
        == str(parent.get("parameterName") or "").casefold()
        and type(entry.get("entryOffset")) is int
        and type(entry.get("serialRelativeEntryOffset")) is int,
        "static MIC entry owner/offset changed",
    )
    _decode_hex(entry.get("rawNameFNameHex"), 8, "static MIC FName")
    raw_value = int.from_bytes(
        _decode_hex(entry.get("rawValueUint32Hex"), 4, "static MIC value"),
        "little",
    )
    raw_override = int.from_bytes(
        _decode_hex(
            entry.get("rawOverrideUint32Hex"),
            4,
            "static MIC override",
        ),
        "little",
    )
    raw_guid = _decode_hex(
        entry.get("rawExpressionGuidHex"),
        16,
        "static MIC ExpressionGUID",
    ).hex()
    require(
        raw_value in {0, 1}
        and type(entry.get("value")) is bool
        and entry["value"] is bool(raw_value),
        "static MIC value decoded/raw semantics changed",
    )
    require(
        raw_override in {0, 1}
        and type(entry.get("bOverride")) is bool
        and entry["bOverride"] is bool(raw_override),
        "static MIC bOverride decoded/raw semantics changed",
    )
    require(
        entry.get("expressionGuidHex") == raw_guid
        and parent.get("expressionGuidHex") == raw_guid,
        "static MIC ExpressionGUID decoded/raw semantics changed",
    )
    for key in (
        "physicalPackageSha256",
        "serialSha256",
        "nativeTailSha256",
        "staticParameterSetRawSha256",
        "staticParameterSetSemanticSha256",
    ):
        _require_sha256(native_tail.get(key), f"static MIC native tail {key}")

    if entry["bOverride"]:
        require(
            selection.get("decision") == "EXACT_GUID_INSTANCE_OVERRIDE_ENTRY"
            and row.get("sourceValueAcquired") is True
            and row.get("sourceValueDecision")
            == "SOURCE_EXACT_INSTANCE_OVERRIDE_VALUE_ACQUIRED"
            and "STATIC_SELECTION_SOURCE_VALUE_NOT_ACQUIRED"
            not in (row.get("remainingBlockers") or []),
            "static override selection admission changed",
        )
    else:
        require(
            entry["value"] is parent.get("parentDefaultValue")
            and selection.get("decision") == "EXACT_GUID_NONOVERRIDE_ENTRY"
            and row.get("sourceValueAcquired") is False
            and row.get("sourceValueDecision")
            == (
                "SOURCE_EXACT_NONOVERRIDE_ENTRY_OBSERVED_"
                "INHERITANCE_SEMANTICS_UNVERIFIED"
            )
            and "STATIC_SELECTION_SOURCE_VALUE_NOT_ACQUIRED"
            in (row.get("remainingBlockers") or []),
            "static nonoverride selection admission changed",
        )


def validate_receipt_semantics(
    receipt: dict[str, Any], contract: dict[str, Any]
) -> None:
    require(
        isinstance(receipt, dict) and set(receipt) == ACQUISITION_ROOT_KEYS,
        "source-value acquisition root schema changed",
    )
    require(
        receipt.get("schema")
        == "lostark.artist-31470-material-source-value-acquisition-receipt"
        and type(receipt.get("formatVersion")) is int
        and receipt["formatVersion"] == 2
        and receipt.get("root") == CONTRACT_ROOT
        and receipt.get("characterClass") == "ARTIST"
        and type(receipt.get("skillId")) is int
        and receipt["skillId"] == 31470
        and receipt.get("inputSlot") == "F"
        and receipt.get("scope") == "SOURCE_VALUE_ACQUISITION_ONLY",
        "unsupported source-value acquisition receipt",
    )
    digest = _require_sha256(receipt.get("receiptSha256"), "acquisition receipt")
    payload = copy.deepcopy(receipt)
    payload.pop("receiptSha256", None)
    require(canonical_sha256(payload) == digest, "receipt digest mismatch")

    _validate_acquisition_sources(receipt)
    _validate_external_artifact_search(receipt)

    matrices = receipt.get("matrices") or {}
    require(
        isinstance(matrices, dict)
        and set(matrices)
        == {"renderStateRows", "staticPermutationRows", "strictSamplerRows"},
        "source-value acquisition matrix schema changed",
    )
    render_rows = matrices.get("renderStateRows") or []
    static_rows = matrices.get("staticPermutationRows") or []
    sampler_rows = matrices.get("strictSamplerRows") or []
    require(
        len(render_rows) == 89
        and len(static_rows) == 94
        and len(sampler_rows) == 77,
        "source-value acquisition matrix denominator changed",
    )
    require(
        len(
            {
                row.get("matrixRowId")
                for row in render_rows + static_rows + sampler_rows
            }
        )
        == 260,
        "source-value acquisition matrix identity changed",
    )
    require(
        canonical_sha256(static_rows) == APPROVED_STATIC_ROW_SET_SHA256,
        "static approved semantic projection changed",
    )
    require(
        all(
            row.get("sourceValueAcquired") is False
            and row.get("executionReady") is False
            for row in render_rows
        )
        and all(row.get("executionReady") is False for row in static_rows),
        "source-value render/static execution admission changed",
    )
    for row in static_rows:
        _validate_static_selection_row(row)
    static_decisions = Counter(
        (row.get("micNativeSelection") or {}).get("decision")
        for row in static_rows
    )
    require(
        static_decisions
        == Counter(
            {
                "EXACT_GUID_INSTANCE_OVERRIDE_ENTRY": 23,
                "EXACT_GUID_NONOVERRIDE_ENTRY": 43,
                "NO_EXACT_GUID_NATIVE_ENTRY": 28,
            }
        )
        and sum(row.get("sourceValueAcquired") is True for row in static_rows)
        == 23,
        "static semantic outcome denominator changed",
    )

    expected_membership = _strict_sampler_membership_from_contract(contract)
    actual_membership = sorted(
        [
            {
                "materialRecipeId": row.get("materialRecipeId"),
                "fieldId": row.get("fieldId"),
                "logicalTexturePath": row.get("logicalTexturePath"),
                "bindingOrigin": (
                    row.get("bindingOriginAndOwner") or {}
                ).get("bindingOrigin"),
                "sourceSection": (
                    row.get("bindingOriginAndOwner") or {}
                ).get("sourceSection"),
            }
            for row in sampler_rows
        ],
        key=lambda row: row["fieldId"],
    )
    require(
        canonical_sha256(actual_membership)
        == canonical_sha256(expected_membership),
        "strict sampler contract membership changed",
    )
    require(
        canonical_sha256(sampler_rows)
        == APPROVED_STRICT_SAMPLER_ROW_SET_SHA256,
        "strict sampler approved semantic projection changed",
    )

    expected_exact4 = {
        row["inputFieldId"]: row
        for row in contract.get("rejectedSamplerBindings") or []
    }
    actual_exact4 = {
        row["fieldId"]: row
        for row in sampler_rows
        if row.get("baselineKind") == "PREVIOUSLY_ADMITTED_EXACT_REAUDIT"
    }
    require(
        len(expected_exact4) == 4
        and set(actual_exact4) == set(expected_exact4),
        "legacy exact sampler membership changed",
    )
    compact_property_keys = {
        "propertyName",
        "arrayIndex",
        "propertyType",
        "structType",
        "declaredDataSize",
        "serializedPayloadSize",
        "tagOffset",
        "valueOffset",
        "recordEndOffset",
        "value",
        "encodedValueHex",
        "encodedValueSha256",
        "recordSha256",
    }
    for row in sampler_rows:
        require(
            row.get("fullDescriptorSourceExact") is False
            and row.get("sourceValueAcquired") is False
            and row.get("executionReady") is False
            and row.get("strictReauditDecision") == "BLOCKED"
            and isinstance(row.get("sourceValueDecision"), str)
            and row["sourceValueDecision"].startswith("BLOCKED_")
            and (
                row.get("consumerPilot") or {}
            ).get("sourceSpecificFullDescriptorVerified")
            is False,
            f"strict sampler admission changed: {row.get('matrixRowId')}",
        )
        fields = (row.get("textureExportEvidence") or {}).get("fields")
        require(
            isinstance(fields, dict)
            and set(fields) == set(TARGET_TEXTURE_FIELDS),
            f"strict sampler field denominator changed: {row.get('matrixRowId')}",
        )
        for field_name, field in fields.items():
            if field.get("status") == "SERIALIZED_EXPLICIT":
                _validate_explicit_sampler_property(field_name, field)
            else:
                require(
                    field
                    == {
                        "status": "OMITTED_FROM_EXPORT",
                        "fidelity": "UNRESOLVED_DEFAULT_PROVENANCE",
                    },
                    f"strict sampler omitted field changed: {field_name}",
                )
        expected_legacy = expected_exact4.get(row["fieldId"])
        if expected_legacy is None:
            expected_baseline = (
                "NEW_SELF_DEFAULT_UNRESOLVED_5"
                if (row.get("bindingOriginAndOwner") or {}).get("bindingOrigin")
                == "SELF_DEFAULT"
                else "PREVIOUSLY_BLOCKED_68"
            )
            require(
                row.get("baselineKind") == expected_baseline
                and row.get("previousAdmission") == "BLOCKED",
                "strict sampler nonlegacy classification changed",
            )
            continue
        require(
            row.get("previousAdmission") == "SOURCE_EXACT_SAMPLER"
            and row.get("materialRecipeId")
            == expected_legacy["materialRecipeId"]
            and row.get("logicalTexturePath")
            == expected_legacy["logicalTexturePath"]
            and (row.get("bindingOriginAndOwner") or {}).get("bindingOrigin")
            == expected_legacy["bindingOrigin"],
            "legacy exact sampler owner identity changed",
        )
        expected_fields = expected_legacy["sourceTextureEvidence"][
            "rawSamplerFields"
        ]
        for field_name in TARGET_TEXTURE_FIELDS:
            actual_field = fields[field_name]
            expected_field = expected_fields[field_name]
            require(
                actual_field["status"] == expected_field["status"],
                "legacy exact sampler field status changed",
            )
            if actual_field["status"] == "SERIALIZED_EXPLICIT":
                require(
                    canonical_sha256(actual_field["property"])
                    == canonical_sha256(
                        {
                            key: expected_field[key]
                            for key in compact_property_keys
                        }
                    ),
                    "legacy exact sampler decoded/raw projection changed",
                )

    clusters = receipt.get("provenanceClusters")
    require(
        isinstance(clusters, dict)
        and set(clusters)
        == {
            "renderByField",
            "staticByDecision",
            "staticRecipeCount",
            "samplerByPreviousAdmission",
            "samplerBindingOriginCounts",
            "samplerUniqueTextureCount",
            "samplerSourceArchiveTextureCount",
            "samplerCurrentOnlyTextureCount",
        },
        "source-value acquisition provenance cluster schema changed",
    )
    corrective = receipt.get("coordinatedCorrectiveRequirements")
    require(
        isinstance(corrective, list)
        and len(corrective) == 3
        and [row.get("requirementId") for row in corrective]
        == [
            "strict-sampler-denominator-77",
            "static-blanket-reason-retirement",
            "artist-resource-manifest-texture-selector",
        ]
        and all(row.get("decision") == "REQUIRED" for row in corrective),
        "coordinated corrective requirement changed",
    )
    missing = receipt.get("minimumMissingExternalArtifacts")
    require(
        isinstance(missing, dict)
        and set(missing) == {"renderState", "staticPermutation", "sampler"}
        and all(isinstance(value, str) and value for value in missing.values()),
        "minimum missing external artifact boundary changed",
    )

    summary = receipt.get("summary") or {}
    require(
        isinstance(summary, dict) and set(summary) == ACQUISITION_SUMMARY_KEYS,
        "source-value acquisition summary schema changed",
    )
    expected_invalidated_exact4 = [
        {
            "matrixRowId": row["matrixRowId"],
            "fieldId": row["fieldId"],
            "logicalTexturePath": row["logicalTexturePath"],
            "rawSamplerFields": row["textureExportEvidence"]["fields"],
            "strictReauditDecision": row["strictReauditDecision"],
        }
        for row in sampler_rows
        if row.get("baselineKind") == "PREVIOUSLY_ADMITTED_EXACT_REAUDIT"
    ]
    require(
        summary.get("renderStateRowCount") == 89
        and summary.get("renderStateSourceValueAcquiredCount")
        == sum(row.get("sourceValueAcquired") is True for row in render_rows)
        == 0
        and summary.get("staticPermutationRowCount") == 94
        and summary.get("staticExactGuidJoinCount") == 66
        and summary.get("staticOverrideTrueSourceValueAcquiredCount") == 23
        and summary.get("staticNonoverrideSemanticsUnverifiedCount") == 43
        and summary.get("staticNoExactGuidEntryCount") == 28
        and summary.get("previouslyAdmittedExactSamplerReauditCount") == 4
        and summary.get("previouslyAdmittedExactSamplerBlockedCount") == 4
        and summary.get("previousDirectUnprovenSamplerRowCount") == 68
        and summary.get("newSelfDefaultSamplerRowCount") == 5
        and summary.get("strictSamplerRowCount") == 77
        and summary.get("strictSamplerSourceValueAcquiredCount") == 0
        and summary.get("strictExecutionRowCount") == 260
        and summary.get("strictExecutionReadyCount") == 0
        and summary.get("productCount") == 0
        and summary.get("valueProvenanceDelta")
        == {
            "renderState": 0,
            "staticPermutation": 23,
            "fullSamplerDescriptor": 0,
        }
        and summary.get("executionReadinessDelta") == 0
        and summary.get("renderRowSetSha256") == canonical_sha256(render_rows)
        and summary.get("staticRowSetSha256")
        == canonical_sha256(static_rows)
        == APPROVED_STATIC_ROW_SET_SHA256
        and summary.get("strictSamplerRowSetSha256")
        == APPROVED_STRICT_SAMPLER_ROW_SET_SHA256
        and summary.get("invalidatedPreviousExactSamplerSetSha256")
        == canonical_sha256(expected_invalidated_exact4),
        "source-value acquisition summary changed",
    )
    admission = receipt.get("admission") or {}
    require(
        isinstance(admission, dict) and set(admission) == ACQUISITION_ADMISSION_KEYS,
        "source-value acquisition admission schema changed",
    )
    require(
        admission.get("acquisitionReceiptEvidenceIntegrity") is True
        and admission.get("upstreamMaterialEvidenceIntegrity") is True
        and admission.get("sourceValueProviderPartial") is True
        and admission.get("executionReady") is False
        and admission.get("product") is False
        and admission.get("r2Entry") is False
        and admission.get("decision") == "R0_BLOCK_R2_NO_GO"
        and admission.get("blockers") == ACQUISITION_BLOCKERS,
        "source-value acquisition admission changed",
    )
    require(
        canonical_sha256(acquisition_semantic_projection(receipt))
        == APPROVED_ACQUISITION_SEMANTIC_PROJECTION_SHA256,
        "source-value acquisition approved semantic projection changed",
    )


def validate_receipt(
    receipt: dict[str, Any],
    contract: dict[str, Any],
    render_receipt: dict[str, Any],
    shader_receipt: dict[str, Any],
    contract_path: Path,
    render_receipt_path: Path,
    shader_receipt_path: Path,
    source_archive_root: Path,
    source_pack_root: Path,
    current_install_root: Path,
) -> None:
    validate_receipt_semantics(receipt, contract)
    expected = build_receipt(
        contract,
        render_receipt,
        shader_receipt,
        contract_path,
        render_receipt_path,
        shader_receipt_path,
        source_archive_root,
        source_pack_root,
        current_install_root,
    )
    require(receipt == expected, "source-value acquisition receipt is stale or laundered")


def output_bytes(value: dict[str, Any]) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    ).encode("utf-8")


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--contract", type=Path, required=True)
    parser.add_argument("--render-receipt", type=Path, required=True)
    parser.add_argument("--shader-receipt", type=Path, required=True)
    parser.add_argument("--source-archive-root", type=Path, required=True)
    parser.add_argument("--source-pack-root", type=Path, required=True)
    parser.add_argument("--current-install-root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--check", action="store_true")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    contract = load_json(args.contract)
    render_receipt = load_json(args.render_receipt)
    shader_receipt = load_json(args.shader_receipt)
    if args.check:
        checked = load_json(args.output)
        validate_receipt(
            checked,
            contract,
            render_receipt,
            shader_receipt,
            args.contract,
            args.render_receipt,
            args.shader_receipt,
            args.source_archive_root,
            args.source_pack_root,
            args.current_install_root,
        )
        print(
            "Artist F Material source-value acquisition check: "
            f"render={checked['summary']['renderStateSourceValueAcquiredCount']}/89 "
            f"static={checked['summary']['staticOverrideTrueSourceValueAcquiredCount']}/94 "
            f"sampler={checked['summary']['strictSamplerSourceValueAcquiredCount']}/77 "
            "execution=0/260 product=false R2=NO-GO"
        )
        return 0
    receipt = build_receipt(
        contract,
        render_receipt,
        shader_receipt,
        args.contract,
        args.render_receipt,
        args.shader_receipt,
        args.source_archive_root,
        args.source_pack_root,
        args.current_install_root,
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(output_bytes(receipt))
    print(
        "Artist F Material source-value acquisition write: "
        f"render=0/89 static=23/94 sampler=0/77 "
        "execution=0/260 product=false R2=NO-GO"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
