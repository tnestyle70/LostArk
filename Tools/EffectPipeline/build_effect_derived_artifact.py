#!/usr/bin/env python3
"""Build and validate immutable Effect derived-artifact bundles.

The v13 authoring document and Assembly produced here are identity-only
carriers.  Runtime semantics live exclusively in the compiled IR embedded in
the compiled artifact.  That generic compiled-IR path remains asset-agnostic.
The isolated Product-false reconstructed-program publication branch
deliberately pins the frozen Artist 31470 candidate and its corpus identities.
"""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import subprocess
import sys
import tempfile
from typing import Any, Iterable


BUILD_REQUEST_SCHEMA = "lostark.effect-derived-build-request"
DERIVED_IDENTITY_SCHEMA = "lostark.effect-derived-identity"
AUTHORING_SCHEMA = "lostark.effect-authoring"
ASSEMBLY_SCHEMA = "lostark.effect-assembly"
COMPILED_IR_SCHEMA = "lostark.effect-compiled-ir"
COMPILER_RECEIPT_SCHEMA = "lostark.effect-compiler-receipt"
COMPILED_ARTIFACT_SCHEMA = "lostark.effect-compiled-artifact"
COMPILED_RECEIPT_SCHEMA = "lostark.effect-compiled-artifact-receipt"
RUNTIME_CATALOG_SCHEMA = "lostark.effect-runtime-catalog"

FORMAT_VERSION = 1
AUTHORING_VERSION = 13
SOURCE_CONTRACT_VERSION = 14
ASSEMBLY_VERSION = 2
RUNTIME_CATALOG_VERSION = 3

IDENTITY_CARRIER_ROLE = "DERIVED_IDENTITY_CARRIER"
SEMANTIC_AUTHORITY = "IMMUTABLE_COMPILED_IR"
LEGACY_PAYLOAD_KIND = "LEGACY_ASSEMBLY_V1"
RECONSTRUCTED_PAYLOAD_KIND = "IMMUTABLE_RECONSTRUCTED_RUNTIME_PROGRAM"
CODE_ONLY_PUBLICATION_STATE = "CODE_ONLY_NOT_ADMITTED"
SOURCE_INPUT_ROLE = "READ_ONLY_SOURCE_CONTRACT"
COMPILER_RECEIPT_AUTHORITY = "TYPED_CASCADE_COMPILER_RECEIPT_V1"

RECONSTRUCTED_LINK_SCHEMA = "lostark.effect-reconstructed-runtime-program-link"
RECONSTRUCTED_RECEIPT_SCHEMA = (
    "lostark.effect-reconstructed-runtime-program-publish-receipt"
)
RECONSTRUCTED_RECEIPT_ROLE = (
    "PUBLICATION_PROVENANCE_ONLY_NOT_EXECUTION_AUTHORITY"
)
RECONSTRUCTED_RECEIPT_DIGEST_DOMAIN = (
    "CANONICAL_JSON_EXCLUDING_RECEIPT_SHA256"
)
RECONSTRUCTED_ENCODING = "UTF8_JSON_EXACT"
RECONSTRUCTED_EFFECT_ID = "effect.artist.skill.31470"
RECONSTRUCTED_ARTIFACT_REVISION = 1
RECONSTRUCTED_COMPILER_REVISION = (
    "artist31470.reconstructed-runtime-program-link-v1"
)
RECONSTRUCTED_CANDIDATE_BUILDER_COMMIT = (
    "a85b8b41afb2f2a51bceafa55d06bf0937b1a245"
)
RECONSTRUCTED_CANDIDATE_BUILDER_TREE = (
    "384ed35ca808ab9a71a4edb703ca4d9121b48c18"
)
RECONSTRUCTED_CANDIDATE_BLOB = "345ab15bbb76648a650eaa854f18c4cd63cb1556"
RECONSTRUCTED_RESOURCE_BINDING_SHA256 = (
    "df15009e41b6c1fe9161af873b96dfc428771944786c14f9435f7c0ffa4d869c"
)
RECONSTRUCTED_INPUT_ARTIFACT_COUNT = 13
RECONSTRUCTED_INPUT_ARTIFACTS_ORDERED_SHA256 = (
    "938dbd9573ca3a5784675ba9d412b9dc3c12a7431a06c70e37d8c9bf2e614eaa"
)
RECONSTRUCTED_PROGRAM_ID = (
    "effect.artist.skill.31470.reconstructed-approved-v1"
)
RECONSTRUCTED_PROGRAM_VERSION = 1
RECONSTRUCTED_PROGRAM_SHA256 = (
    "618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b"
)
RECONSTRUCTED_CANDIDATE_RAW_SHA256 = (
    "72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849"
)
RECONSTRUCTED_CANDIDATE_BYTE_COUNT = 15_072_141

RECONSTRUCTED_BASE_ENTRY_CANONICAL_SHA256 = (
    "e9694f000a50a426386afd6ff8f65b4a2a5fcafe9883860efff9103e1fff82d2"
)
RECONSTRUCTED_BASE_LINK_SHA256 = (
    "74175fe1e41b22ae593a9d1ff92027606bc0b31d62d17927ef6ac5673dd4a7a2"
)
RECONSTRUCTED_BASE_RECEIPT_SELF_SHA256 = (
    "5c91709f2f0ec855c54c94e6dad5bcd7ed048c6133ca9a9af7d4873f20da1bd3"
)
RECONSTRUCTED_BASE_RECEIPT_SHA256 = (
    "92c883f78d88018a50d8dec09eb6fb155974bec4b3756a796b3499fc2f839d94"
)

RECONSTRUCTED_RENDER_RESOURCE_LINK_SCHEMA = (
    "lostark.effect-reconstructed-render-resource-authority-link"
)
RECONSTRUCTED_RENDER_RESOURCE_RECEIPT_SCHEMA = (
    "lostark.effect-reconstructed-render-resource-publication-receipt"
)
RECONSTRUCTED_RENDER_RESOURCE_RECEIPT_ROLE = (
    "PUBLICATION_PROVENANCE_ONLY_NOT_EXECUTION_SUBMIT_RENDER_AUTHORITY"
)
RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_SCHEMA = (
    "lostark.artist-31470-reconstructed-render-resource-authority-receipt"
)
RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_AUTHORITY_ID = (
    "ARTIST_31470_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY_V1"
)
RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_BYTE_COUNT = 746_788
RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_RAW_SHA256 = (
    "bc5cd1accbbe3c628993a47093dc829eec6f050ab8467fca82f6b7bcf2dfe0ff"
)
RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_RECEIPT_SHA256 = (
    "bd05c7dca6bdef205b27c208644be19bb94bdbef2e05712bfc49b9b946d8f28a"
)
RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_DECISION_SHA256 = (
    "4efa9ea724df336a5f3af719e24211b7206fe21dfd97becc630f88c5dbd9b412"
)

RECONSTRUCTED_BASE_ENTRY_KEYS = (
    "payloadKind",
    "effectAssetId",
    "artifactRevision",
    "compilerRevision",
    "sourceExact",
    "runtimeExecutionAdmission",
    "productAdmission",
    "publishReceiptSha256",
    "publishReceipt",
    "reconstructedRuntimeProgram",
)
RECONSTRUCTED_ENTRY_KEYS = RECONSTRUCTED_BASE_ENTRY_KEYS + (
    "renderResourcePublishReceiptSha256",
    "renderResourcePublishReceipt",
    "reconstructedRenderResourceAuthority",
)
RECONSTRUCTED_LINK_KEYS = (
    "schema",
    "formatVersion",
    "encoding",
    "effectAssetId",
    "candidateBuilderCommitId",
    "candidateBuilderTreeId",
    "candidateBlobId",
    "resourceBindingHash",
    "inputArtifactCount",
    "inputArtifactsOrderedSha256",
    "programId",
    "programVersion",
    "programSha256",
    "candidateRawSha256",
    "candidateByteCount",
    "candidateUtf8Json",
)
RECONSTRUCTED_RECEIPT_KEYS = (
    "schema",
    "formatVersion",
    "receiptRole",
    "payloadKind",
    "effectAssetId",
    "artifactRevision",
    "compilerRevision",
    "sourceExact",
    "runtimeExecutionAdmission",
    "productAdmission",
    "candidateBuilderCommitId",
    "candidateBuilderTreeId",
    "candidateBlobId",
    "resourceBindingHash",
    "inputArtifactCount",
    "inputArtifactsOrderedSha256",
    "programId",
    "programVersion",
    "programSha256",
    "candidateRawSha256",
    "candidateByteCount",
    "reconstructedRuntimeProgramSha256",
    "toolDependencies",
    "receiptSha256Domain",
    "receiptSha256",
)
RECONSTRUCTED_HISTORICAL_TOOL_DEPENDENCIES = (
    (
        "RECONSTRUCTED_RUNTIME_PROGRAM_CANDIDATE_BUILDER",
        "Tools/EffectPipeline/build_artist_31470_reconstructed_runtime_program.py",
        "5c207e04952971adb553249540e336ba3ad065719e438a9892c6850d2c989c4e",
    ),
    (
        "RECONSTRUCTED_RUNTIME_PROGRAM_CATALOG_VALIDATOR",
        "Tools/EffectPipeline/build_effect_derived_artifact.py",
        "5407c3d0983c3aaf4bf085904ef8d7b5f3e9119ae448703ff7e8f612a1c144fb",
    ),
    (
        "EFFECT_PUBLISHER",
        "Tools/EffectPipeline/Publish-Effects.ps1",
        "ee4a12cf5cbd63bc9af6b0af18ca37da7631a4b0b6ed1465c95bf99fb9be8825",
    ),
)
RECONSTRUCTED_RENDER_RESOURCE_LINK_KEYS = (
    "schema",
    "formatVersion",
    "encoding",
    "effectAssetId",
    "programId",
    "programVersion",
    "programSha256",
    "sidecarSchema",
    "sidecarFormatVersion",
    "sidecarAuthorityId",
    "sidecarDecisionProjectionSha256",
    "sidecarReceiptSha256",
    "sidecarRawSha256",
    "sidecarByteCount",
    "sourceExact",
    "runtimeExecutionAdmission",
    "executeAdmission",
    "submitAdmission",
    "renderAdmission",
    "productAdmission",
    "sidecarUtf8Json",
)
RECONSTRUCTED_RENDER_RESOURCE_RECEIPT_KEYS = (
    "schema",
    "formatVersion",
    "receiptRole",
    "payloadKind",
    "effectAssetId",
    "artifactRevision",
    "compilerRevision",
    "sourceExact",
    "runtimeExecutionAdmission",
    "executeAdmission",
    "submitAdmission",
    "renderAdmission",
    "productAdmission",
    "programId",
    "programVersion",
    "programSha256",
    "baseRuntimeEntryProjectionSha256",
    "reconstructedRuntimeProgramSha256",
    "basePublishReceiptSha256",
    "renderResourceAuthorityLinkSha256",
    "sidecarRawSha256",
    "sidecarReceiptSha256",
    "sidecarDecisionProjectionSha256",
    "toolDependencies",
    "receiptSha256Domain",
    "receiptSha256",
)
RECONSTRUCTED_RENDER_RESOURCE_TOOL_DEPENDENCIES = (
    (
        "RECONSTRUCTED_RENDER_RESOURCE_INDEPENDENT_PINS",
        "Tools/LevelPlacementExtractor/"
        "artist_31470_reconstructed_render_resource_authority.py",
    ),
    (
        "RECONSTRUCTED_RENDER_RESOURCE_CATALOG_VALIDATOR",
        "Tools/EffectPipeline/build_effect_derived_artifact.py",
    ),
    (
        "EFFECT_PUBLISHER",
        "Tools/EffectPipeline/Publish-Effects.ps1",
    ),
)
RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_ROOT_KEYS = (
    "schema",
    "formatVersion",
    "authorityId",
    "characterClass",
    "skillId",
    "inputSlot",
    "authorityContract",
    "sourceEvidence",
    "textureResources",
    "textureBindings",
    "neutralProviders",
    "recipeTextureBindings",
    "rendererSlotBindings",
    "renderStateDescriptors",
    "blockerProjection",
    "admission",
    "summary",
    "decisionProjectionSha256",
    "receiptSha256",
)

SHA_PATTERN = re.compile(r"^[0-9a-f]{64}$")
STABLE_ID_PATTERN = re.compile(r"^[A-Za-z0-9_.-]{1,128}$")
SAFE_RELATIVE_PATH_PATTERN = re.compile(r"^[A-Za-z0-9_.\-/]+$")

IDENTITY_FIELDS = (
    "sourceContractHash",
    "sourceSemanticClosureHash",
    "geometryContractHash",
    "materialContractHash",
    "resourceBindingHash",
    "compilerInputHash",
)

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
TOOL_DEPENDENCIES = (
    (
        "DERIVED_ARTIFACT_GENERATOR",
        "Tools/EffectPipeline/build_effect_derived_artifact.py",
        "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
    ),
    (
        "DERIVED_ARTIFACT_SCHEMA",
        "Tools/EffectPipeline/Schemas/lostark.effect-derived-artifact-contract.schema.json",
        "CANONICAL_JSON",
    ),
    (
        "EFFECT_PUBLISHER",
        "Tools/EffectPipeline/Publish-Effects.ps1",
        "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
    ),
)
INPUT_TO_IDENTITY = (
    ("sourceContract", "sourceContractHash"),
    ("sourceSemanticClosure", "sourceSemanticClosureHash"),
    ("geometryContract", "geometryContractHash"),
    ("materialContract", "materialContractHash"),
    ("resourceBinding", "resourceBindingHash"),
    ("compilerInput", "compilerInputHash"),
)
UPSTREAM_INPUT_CONTRACTS = {
    "sourceSemanticClosure": (
        "lostark.effect-source-semantic-closure",
        "SOURCE_SEMANTIC_CLOSURE",
    ),
    "geometryContract": (
        "lostark.effect-geometry-contract",
        "GEOMETRY_CONTRACT",
    ),
    "materialContract": (
        "lostark.effect-material-contract",
        "MATERIAL_CONTRACT",
    ),
    "resourceBinding": (
        "lostark.effect-resource-binding-contract",
        "RESOURCE_BINDING_CONTRACT",
    ),
    "compilerInput": (
        "lostark.effect-compiler-input-contract",
        "COMPILER_INPUT_CONTRACT",
    ),
}


class ContractError(ValueError):
    """Raised when a derived-artifact contract fails closed."""


def _object_no_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ContractError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def load_json(path: Path, *, require_lf: bool = False) -> dict[str, Any]:
    try:
        payload = path.read_bytes()
        if payload.startswith(b"\xef\xbb\xbf"):
            raise ContractError(f"JSON must be UTF-8 without BOM: {path}")
        if require_lf and b"\r" in payload:
            raise ContractError(f"JSON must use LF-only newlines: {path}")
        value = json.loads(
            payload.decode("utf-8"),
            object_pairs_hook=_object_no_duplicates,
        )
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise ContractError(f"cannot read JSON {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise ContractError(f"JSON root must be an object: {path}")
    return value


def canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def pretty_json_bytes(value: Any) -> bytes:
    return (
        json.dumps(
            value,
            ensure_ascii=False,
            indent=2,
            allow_nan=False,
        )
        + "\n"
    ).encode("utf-8")


def canonical_text_bytes(payload: bytes, label: str) -> bytes:
    if payload.startswith(b"\xef\xbb\xbf"):
        raise ContractError(f"{label} must be UTF-8 without BOM")
    try:
        text = payload.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise ContractError(f"{label} must be UTF-8 text") from exc
    return text.replace("\r\n", "\n").replace("\r", "\n").encode("utf-8")


def sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def sha256_file(path: Path) -> str:
    try:
        digest = hashlib.sha256()
        with path.open("rb") as stream:
            for chunk in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(chunk)
        return digest.hexdigest()
    except OSError as exc:
        raise ContractError(f"cannot hash file {path}: {exc}") from exc


def _require_exact_keys(value: dict[str, Any], keys: Iterable[str], label: str) -> None:
    expected = set(keys)
    actual = set(value)
    if actual != expected:
        missing = sorted(expected - actual)
        extra = sorted(actual - expected)
        raise ContractError(f"{label} keys mismatch; missing={missing}, extra={extra}")


def _require_exact_key_order(
    value: dict[str, Any], keys: Iterable[str], label: str
) -> None:
    expected = tuple(keys)
    actual = tuple(value)
    if actual != expected:
        raise ContractError(
            f"{label} key order mismatch; expected={expected}, actual={actual}"
        )


def _require_exact_int(value: Any, expected: int, label: str) -> int:
    if type(value) is not int or value != expected:
        raise ContractError(f"{label} must be JSON integer {expected}")
    return value


def _require_positive_int(value: Any, label: str) -> int:
    if type(value) is not int or value <= 0:
        raise ContractError(f"{label} must be a positive JSON integer")
    return value


def _require_bool(value: Any, expected: bool, label: str) -> bool:
    if type(value) is not bool or value is not expected:
        raise ContractError(f"{label} must be JSON boolean {str(expected).lower()}")
    return value


def _require_string(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value:
        raise ContractError(f"{label} must be a non-empty string")
    return value


def _require_stable_id(value: Any, label: str) -> str:
    text = _require_string(value, label)
    if STABLE_ID_PATTERN.fullmatch(text) is None:
        raise ContractError(f"{label} must be a stable ID")
    return text


def _require_sha(value: Any, label: str) -> str:
    text = _require_string(value, label)
    if SHA_PATTERN.fullmatch(text) is None:
        raise ContractError(f"{label} must be a lower-case SHA-256")
    return text


def _require_display_name(value: Any) -> str:
    text = _require_string(value, "displayName")
    if not text.strip() or len(text.encode("utf-8")) > 64:
        raise ContractError("displayName must be 1-64 non-blank UTF-8 bytes")
    return text


def _require_string_array(value: Any, label: str) -> list[str]:
    if not isinstance(value, list):
        raise ContractError(f"{label} must be an array")
    result: list[str] = []
    seen: set[str] = set()
    for index, item in enumerate(value):
        token = _require_stable_id(item, f"{label}[{index}]")
        if token in seen:
            raise ContractError(f"{label} contains duplicate blocker: {token}")
        seen.add(token)
        result.append(token)
    if result != sorted(result):
        raise ContractError(f"{label} must use canonical ordinal order")
    return result


def _validate_execution_contract(value: Any, label: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ContractError(f"{label} must be an object")
    _require_exact_keys(
        value,
        (
            "artifactBindingBlockerSet",
            "artifactBindingBlockerCount",
            "executionBlockerSet",
            "executionBlockerCount",
            "executionAdmission",
        ),
        label,
    )
    artifact = _require_string_array(
        value["artifactBindingBlockerSet"],
        f"{label}.artifactBindingBlockerSet",
    )
    execution = _require_string_array(
        value["executionBlockerSet"], f"{label}.executionBlockerSet"
    )
    _require_exact_int(
        value["artifactBindingBlockerCount"],
        len(artifact),
        f"{label}.artifactBindingBlockerCount",
    )
    _require_exact_int(
        value["executionBlockerCount"],
        len(execution),
        f"{label}.executionBlockerCount",
    )
    expected_admission = not artifact and not execution
    _require_bool(
        value["executionAdmission"], expected_admission, f"{label}.executionAdmission"
    )
    return {
        "artifactBindingBlockerSet": artifact,
        "artifactBindingBlockerCount": len(artifact),
        "executionBlockerSet": execution,
        "executionBlockerCount": len(execution),
        "executionAdmission": expected_admission,
    }


def _make_execution_contract(
    artifact_blockers: Iterable[str], execution_blockers: Iterable[str]
) -> dict[str, Any]:
    artifact = sorted(set(artifact_blockers))
    execution = sorted(set(execution_blockers))
    value = {
        "artifactBindingBlockerSet": artifact,
        "artifactBindingBlockerCount": len(artifact),
        "executionBlockerSet": execution,
        "executionBlockerCount": len(execution),
        "executionAdmission": not artifact and not execution,
    }
    _validate_execution_contract(value, "executionContract")
    return value


def _combine_execution_contracts(
    contracts: Iterable[dict[str, Any]], label: str
) -> dict[str, Any]:
    artifact: set[str] = set()
    execution: set[str] = set()
    for contract in contracts:
        artifact.update(contract["artifactBindingBlockerSet"])
        execution.update(contract["executionBlockerSet"])
    combined = _make_execution_contract(artifact, execution)
    _validate_execution_contract(combined, label)
    return combined


def _validate_evidence_rows(value: Any, label: str) -> dict[str, Any]:
    if not isinstance(value, list):
        raise ContractError(f"{label} must be an array")
    ids: set[str] = set()
    contracts: list[dict[str, Any]] = []
    for index, row in enumerate(value):
        row_label = f"{label}[{index}]"
        if not isinstance(row, dict):
            raise ContractError(f"{row_label} must be an object")
        _require_exact_keys(
            row,
            ("evidenceId", "evidenceSha256", "executionContract"),
            row_label,
        )
        evidence_id = _require_stable_id(row["evidenceId"], f"{row_label}.evidenceId")
        if evidence_id in ids:
            raise ContractError(f"{label} contains duplicate evidenceId: {evidence_id}")
        ids.add(evidence_id)
        _require_sha(row["evidenceSha256"], f"{row_label}.evidenceSha256")
        contracts.append(
            _validate_execution_contract(
                row["executionContract"], f"{row_label}.executionContract"
            )
        )
    return _combine_execution_contracts(contracts, f"{label}.aggregate")


def _safe_input_path(input_root: Path, relative: Any, label: str) -> Path:
    text = _require_string(relative, label)
    if (
        "\\" in text
        or ":" in text
        or SAFE_RELATIVE_PATH_PATTERN.fullmatch(text) is None
        or any(segment in ("", ".", "..") for segment in text.split("/"))
    ):
        raise ContractError(f"{label} must be a forward-slash relative path")
    pure = PurePosixPath(text)
    if pure.is_absolute() or any(part in ("", ".", "..") for part in pure.parts):
        raise ContractError(f"{label} escapes the input root")
    root = input_root.resolve()
    resolved = (root / Path(*pure.parts)).resolve()
    try:
        resolved.relative_to(root)
    except ValueError as exc:
        raise ContractError(f"{label} escapes the input root") from exc
    if not resolved.is_file():
        raise ContractError(f"{label} is missing: {resolved}")
    return resolved


def _validate_file_reference(
    value: Any, input_root: Path, label: str
) -> tuple[Path, str, str]:
    if not isinstance(value, dict):
        raise ContractError(f"{label} must be an object")
    _require_exact_keys(value, ("path", "rawSha256", "canonicalSha256"), label)
    path = _safe_input_path(input_root, value["path"], f"{label}.path")
    expected_raw = _require_sha(value["rawSha256"], f"{label}.rawSha256")
    expected_canonical = _require_sha(
        value["canonicalSha256"], f"{label}.canonicalSha256"
    )
    if sha256_file(path) != expected_raw:
        raise ContractError(f"{label} raw SHA mismatch")
    actual_canonical = sha256_bytes(canonical_json_bytes(load_json(path)))
    if actual_canonical != expected_canonical:
        raise ContractError(f"{label} canonical SHA mismatch")
    return path, expected_raw, expected_canonical


def validate_derived_identity(value: Any) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ContractError("derivedIdentity must be an object")
    expected_keys = ("schema", "formatVersion", *IDENTITY_FIELDS)
    _require_exact_keys(value, expected_keys, "derivedIdentity")
    if tuple(value.keys()) != expected_keys:
        raise ContractError("derivedIdentity fields are not in canonical six-hash order")
    if value["schema"] != DERIVED_IDENTITY_SCHEMA:
        raise ContractError("derivedIdentity schema mismatch")
    _require_exact_int(value["formatVersion"], FORMAT_VERSION, "derivedIdentity.formatVersion")
    for field in IDENTITY_FIELDS:
        _require_sha(value[field], f"derivedIdentity.{field}")
    return dict(value)


def _make_derived_identity(hashes: dict[str, str]) -> dict[str, Any]:
    identity: dict[str, Any] = {
        "schema": DERIVED_IDENTITY_SCHEMA,
        "formatVersion": FORMAT_VERSION,
    }
    for field in IDENTITY_FIELDS:
        identity[field] = hashes[field]
    validate_derived_identity(identity)
    return identity


def _current_tool_dependencies() -> list[dict[str, Any]]:
    dependencies: list[dict[str, Any]] = []
    for role, relative_path, hash_domain in TOOL_DEPENDENCIES:
        path = REPOSITORY_ROOT / Path(*PurePosixPath(relative_path).parts)
        if not path.is_file():
            raise ContractError(f"missing tracked tool dependency: {relative_path}")
        payload = path.read_bytes()
        if hash_domain == "CANONICAL_JSON":
            canonical = canonical_json_bytes(load_json(path))
        elif hash_domain == "TRACKED_SOURCE_EOL_CANONICAL_TEXT":
            canonical = canonical_text_bytes(payload, relative_path)
        else:
            raise ContractError(f"unsupported tool dependency hash domain: {hash_domain}")
        dependencies.append(
            {
                "role": role,
                "path": relative_path,
                "rawSha256": sha256_bytes(payload),
                "canonicalSha256": sha256_bytes(canonical),
                "hashDomain": hash_domain,
                "verificationRole": "CANONICAL_REQUIRED_RAW_OBSERVED",
            }
        )
    return dependencies


def _validate_tool_dependencies(value: Any) -> None:
    if not isinstance(value, list) or len(value) != len(TOOL_DEPENDENCIES):
        raise ContractError("compiled receipt toolDependencies are incomplete")
    current = _current_tool_dependencies()
    for index, (record, expected) in enumerate(zip(value, current, strict=True)):
        if not isinstance(record, dict):
            raise ContractError(f"toolDependencies[{index}] must be an object")
        _require_exact_keys(
            record,
            (
                "role",
                "path",
                "rawSha256",
                "canonicalSha256",
                "hashDomain",
                "verificationRole",
            ),
            f"toolDependencies[{index}]",
        )
        _require_sha(record["rawSha256"], f"toolDependencies[{index}].rawSha256")
        _require_sha(
            record["canonicalSha256"],
            f"toolDependencies[{index}].canonicalSha256",
        )
        for field in (
            "role",
            "path",
            "canonicalSha256",
            "hashDomain",
            "verificationRole",
        ):
            if record[field] != expected[field]:
                raise ContractError(
                    f"compiled receipt tool dependency {field} mismatch: {expected['path']}"
                )
        dependency_path = REPOSITORY_ROOT / Path(
            *PurePosixPath(expected["path"]).parts
        )
        current_payload = dependency_path.read_bytes()
        normalized_lf = canonical_text_bytes(current_payload, expected["path"])
        allowed_raw_hashes = {
            sha256_bytes(current_payload),
            sha256_bytes(normalized_lf),
            sha256_bytes(normalized_lf.replace(b"\n", b"\r\n")),
        }
        if record["rawSha256"] not in allowed_raw_hashes:
            raise ContractError(
                f"compiled receipt tool dependency raw SHA mismatch: {expected['path']}"
            )


def validate_authoring_carrier(value: dict[str, Any]) -> None:
    keys = (
        "schema",
        "version",
        "effectAssetId",
        "displayName",
        "documentRole",
        "runtimeSemanticAuthority",
        "sourceContract",
        "derivedIdentity",
        "elements",
    )
    _require_exact_keys(value, keys, "authoring carrier")
    if value["schema"] != AUTHORING_SCHEMA:
        raise ContractError("authoring carrier schema mismatch")
    _require_exact_int(value["version"], AUTHORING_VERSION, "authoring carrier version")
    _require_stable_id(value["effectAssetId"], "authoring carrier effectAssetId")
    _require_display_name(value["displayName"])
    if value["documentRole"] != IDENTITY_CARRIER_ROLE:
        raise ContractError("authoring carrier role mismatch")
    if value["runtimeSemanticAuthority"] != SEMANTIC_AUTHORITY:
        raise ContractError("authoring carrier semantic authority mismatch")
    source = value["sourceContract"]
    if not isinstance(source, dict):
        raise ContractError("authoring carrier sourceContract must be an object")
    _require_exact_keys(source, ("formatVersion", "readOnly", "drawable"), "sourceContract")
    _require_exact_int(source["formatVersion"], SOURCE_CONTRACT_VERSION, "sourceContract.formatVersion")
    _require_bool(source["readOnly"], True, "sourceContract.readOnly")
    _require_bool(source["drawable"], False, "sourceContract.drawable")
    validate_derived_identity(value["derivedIdentity"])
    if type(value["elements"]) is not list or value["elements"]:
        raise ContractError("identity-only authoring carrier elements must be empty")


def validate_assembly_carrier(value: dict[str, Any]) -> None:
    keys = (
        "schema",
        "version",
        "effectAssetId",
        "displayName",
        "assemblyRole",
        "sourceAuthoringVersion",
        "sourceDocumentFileSha256",
        "runtimeSemanticAuthority",
        "derivedIdentity",
        "compiledArtifactIdentity",
    )
    _require_exact_keys(value, keys, "Assembly carrier")
    if value["schema"] != ASSEMBLY_SCHEMA:
        raise ContractError("Assembly carrier schema mismatch")
    _require_exact_int(value["version"], ASSEMBLY_VERSION, "Assembly carrier version")
    _require_stable_id(value["effectAssetId"], "Assembly carrier effectAssetId")
    _require_display_name(value["displayName"])
    if value["assemblyRole"] != IDENTITY_CARRIER_ROLE:
        raise ContractError("Assembly carrier role mismatch")
    _require_exact_int(
        value["sourceAuthoringVersion"],
        AUTHORING_VERSION,
        "Assembly carrier sourceAuthoringVersion",
    )
    _require_sha(value["sourceDocumentFileSha256"], "Assembly source document SHA")
    if value["runtimeSemanticAuthority"] != SEMANTIC_AUTHORITY:
        raise ContractError("Assembly carrier semantic authority mismatch")
    validate_derived_identity(value["derivedIdentity"])
    compiled = value["compiledArtifactIdentity"]
    if not isinstance(compiled, dict):
        raise ContractError("compiledArtifactIdentity must be an object")
    _require_exact_keys(
        compiled,
        (
            "artifactRevision",
            "compilerRevision",
            "compiledIrSha256",
            "compilerReceiptTokenSha256",
        ),
        "compiledArtifactIdentity",
    )
    _require_positive_int(compiled["artifactRevision"], "artifactRevision")
    _require_stable_id(compiled["compilerRevision"], "compilerRevision")
    _require_sha(compiled["compiledIrSha256"], "compiledIrSha256")
    _require_sha(
        compiled["compilerReceiptTokenSha256"], "compilerReceiptTokenSha256"
    )


def _validate_upstream_contract(
    input_name: str, value: dict[str, Any], target_effect_id: str
) -> dict[str, Any]:
    if input_name == "sourceContract":
        _require_exact_keys(
            value,
            (
                "schema",
                "version",
                "effectAssetId",
                "displayName",
                "documentRole",
                "readOnly",
                "runtimeSemanticAuthority",
                "derivedTargetEffectAssetId",
                "executionContract",
                "evidenceRows",
            ),
            input_name,
        )
        if value["schema"] != AUTHORING_SCHEMA:
            raise ContractError("source contract schema mismatch")
        _require_exact_int(value["version"], SOURCE_CONTRACT_VERSION, "source contract version")
        _require_stable_id(value["effectAssetId"], "source contract effectAssetId")
        _require_display_name(value["displayName"])
        if value["documentRole"] != SOURCE_INPUT_ROLE:
            raise ContractError("source contract documentRole mismatch")
        _require_bool(value["readOnly"], True, "source contract readOnly")
    else:
        schema, contract_role = UPSTREAM_INPUT_CONTRACTS[input_name]
        _require_exact_keys(
            value,
            (
                "schema",
                "formatVersion",
                "effectAssetId",
                "contractRole",
                "runtimeSemanticAuthority",
                "derivedTargetEffectAssetId",
                "executionContract",
                "evidenceRows",
            ),
            input_name,
        )
        if value["schema"] != schema:
            raise ContractError(f"{input_name} schema mismatch")
        _require_exact_int(value["formatVersion"], FORMAT_VERSION, f"{input_name}.formatVersion")
        _require_stable_id(value["effectAssetId"], f"{input_name}.effectAssetId")
        if value["contractRole"] != contract_role:
            raise ContractError(f"{input_name}.contractRole mismatch")
    if value["runtimeSemanticAuthority"] != "EVIDENCE_ONLY_NOT_RUNTIME_SEMANTICS":
        raise ContractError(f"{input_name} runtimeSemanticAuthority mismatch")
    if value["derivedTargetEffectAssetId"] != target_effect_id:
        raise ContractError(f"{input_name} target effect identity mismatch")
    root_contract = _validate_execution_contract(
        value["executionContract"], f"{input_name}.executionContract"
    )
    row_contract = _validate_evidence_rows(value["evidenceRows"], f"{input_name}.evidenceRows")
    if root_contract != row_contract:
        raise ContractError(f"{input_name} execution contract is not derived from evidence rows")
    return root_contract


def _reject_reserved_execution_fields(value: Any, label: str) -> None:
    reserved = {
        "artifactBindingBlockerSet",
        "artifactBindingBlockerCount",
        "executionBlockerSet",
        "executionBlockerCount",
        "executionAdmission",
        "blockers",
    }
    if isinstance(value, dict):
        for key, child in value.items():
            if key in reserved:
                raise ContractError(f"{label} contains execution authority outside typed receipt: {key}")
            _reject_reserved_execution_fields(child, f"{label}.{key}")
    elif isinstance(value, list):
        for index, child in enumerate(value):
            _reject_reserved_execution_fields(child, f"{label}[{index}]")


def _validate_handler_receipts(value: Any) -> dict[str, Any]:
    if not isinstance(value, list):
        raise ContractError("compiled IR handlerReceipts must be an array")
    ids: set[str] = set()
    contracts: list[dict[str, Any]] = []
    for index, row in enumerate(value):
        label = f"compiled IR handlerReceipts[{index}]"
        if not isinstance(row, dict):
            raise ContractError(f"{label} must be an object")
        _require_exact_keys(
            row, ("handlerId", "handlerSha256", "executionContract"), label
        )
        handler_id = _require_stable_id(row["handlerId"], f"{label}.handlerId")
        if handler_id in ids:
            raise ContractError(f"duplicate compiled IR handlerId: {handler_id}")
        ids.add(handler_id)
        _require_sha(row["handlerSha256"], f"{label}.handlerSha256")
        contracts.append(
            _validate_execution_contract(
                row["executionContract"], f"{label}.executionContract"
            )
        )
    return _combine_execution_contracts(contracts, "compiled IR handler aggregate")


def validate_compiled_ir(
    value: dict[str, Any],
    effect_asset_id: str,
    compiler_revision: str,
    artifact_revision: int,
    derived_identity: dict[str, Any],
) -> tuple[str, dict[str, Any]]:
    _require_exact_keys(
        value,
        (
            "schema",
            "formatVersion",
            "effectAssetId",
            "artifactRevision",
            "compilerRevision",
            "runtimeSemanticAuthority",
            "derivedIdentity",
            "executionContract",
            "program",
        ),
        "compiled IR",
    )
    if value["schema"] != COMPILED_IR_SCHEMA:
        raise ContractError("compiled IR schema mismatch")
    _require_exact_int(value["formatVersion"], FORMAT_VERSION, "compiled IR formatVersion")
    if value["effectAssetId"] != effect_asset_id:
        raise ContractError("compiled IR effectAssetId mismatch")
    if value["compilerRevision"] != compiler_revision:
        raise ContractError("compiled IR compilerRevision mismatch")
    if value["artifactRevision"] != artifact_revision:
        raise ContractError("compiled IR artifactRevision mismatch")
    if value["runtimeSemanticAuthority"] != SEMANTIC_AUTHORITY:
        raise ContractError("compiled IR semantic authority mismatch")
    if validate_derived_identity(value["derivedIdentity"]) != derived_identity:
        raise ContractError("compiled IR canonical six-hash identity mismatch")
    program = value["program"]
    if not isinstance(program, dict):
        raise ContractError("compiled IR program must be an object")
    _require_exact_keys(
        program, ("opcodes", "resourceBindings", "handlerReceipts"), "compiled IR program"
    )
    if not isinstance(program["opcodes"], list) or not isinstance(
        program["resourceBindings"], list
    ):
        raise ContractError("compiled IR program arrays are invalid")
    _reject_reserved_execution_fields(program["opcodes"], "compiled IR opcodes")
    _reject_reserved_execution_fields(
        program["resourceBindings"], "compiled IR resourceBindings"
    )
    root_contract = _validate_execution_contract(
        value["executionContract"], "compiled IR executionContract"
    )
    handler_contract = _validate_handler_receipts(program["handlerReceipts"])
    if root_contract != handler_contract:
        raise ContractError("compiled IR execution contract is not derived from handler receipts")
    return sha256_bytes(canonical_json_bytes(value)), root_contract


def _compiler_receipt_token_payload(value: dict[str, Any]) -> dict[str, Any]:
    return {key: item for key, item in value.items() if key != "compilerReceiptTokenSha256"}


def make_compiler_receipt_token(value: dict[str, Any]) -> str:
    return sha256_bytes(canonical_json_bytes(_compiler_receipt_token_payload(value)))


def validate_compiler_receipt(
    value: dict[str, Any],
    effect_asset_id: str,
    compiler_revision: str,
    artifact_revision: int,
    derived_identity: dict[str, Any],
    compiled_ir_sha256: str,
    compiled_ir_contract: dict[str, Any],
) -> str:
    _require_exact_keys(
        value,
        (
            "schema",
            "formatVersion",
            "effectAssetId",
            "artifactRevision",
            "compilerRevision",
            "runtimeSemanticAuthority",
            "receiptAuthority",
            "derivedIdentity",
            "compilerInputHash",
            "compiledIrSha256",
            "executionContract",
            "compilerReceiptTokenSha256",
        ),
        "compiler receipt",
    )
    if value["schema"] != COMPILER_RECEIPT_SCHEMA:
        raise ContractError("compiler receipt schema mismatch")
    _require_exact_int(value["formatVersion"], FORMAT_VERSION, "compiler receipt formatVersion")
    if value["effectAssetId"] != effect_asset_id:
        raise ContractError("compiler receipt effectAssetId mismatch")
    if value["compilerRevision"] != compiler_revision:
        raise ContractError("compiler receipt compilerRevision mismatch")
    if value["artifactRevision"] != artifact_revision:
        raise ContractError("compiler receipt artifactRevision mismatch")
    if value["runtimeSemanticAuthority"] != SEMANTIC_AUTHORITY:
        raise ContractError("compiler receipt semantic authority mismatch")
    if value["receiptAuthority"] != COMPILER_RECEIPT_AUTHORITY:
        raise ContractError("compiler receipt authority mismatch")
    if validate_derived_identity(value["derivedIdentity"]) != derived_identity:
        raise ContractError("compiler receipt canonical six-hash identity mismatch")
    if value["compilerInputHash"] != derived_identity["compilerInputHash"]:
        raise ContractError("compiler receipt compiler input hash mismatch")
    if value["compiledIrSha256"] != compiled_ir_sha256:
        raise ContractError("compiler receipt compiled IR hash mismatch")
    if (
        _validate_execution_contract(
            value["executionContract"], "compiler receipt executionContract"
        )
        != compiled_ir_contract
    ):
        raise ContractError("compiler receipt execution contract mismatch")
    token = _require_sha(
        value["compilerReceiptTokenSha256"], "compiler receipt token"
    )
    if token != make_compiler_receipt_token(value):
        raise ContractError("compiler receipt authentication token mismatch")
    return token


def validate_compiled_artifact(value: dict[str, Any]) -> None:
    keys = (
        "schema",
        "formatVersion",
        "effectAssetId",
        "artifactRevision",
        "compilerRevision",
        "runtimeSemanticAuthority",
        "derivedIdentity",
        "compiledIrSha256",
        "compilerReceiptTokenSha256",
        "compiledIr",
        "executionAdmission",
        "productAdmission",
    )
    _require_exact_keys(value, keys, "compiled artifact")
    if value["schema"] != COMPILED_ARTIFACT_SCHEMA:
        raise ContractError("compiled artifact schema mismatch")
    _require_exact_int(value["formatVersion"], FORMAT_VERSION, "compiled artifact formatVersion")
    effect_id = _require_stable_id(value["effectAssetId"], "compiled artifact effectAssetId")
    revision = _require_positive_int(value["artifactRevision"], "compiled artifact revision")
    compiler = _require_stable_id(value["compilerRevision"], "compiled artifact compilerRevision")
    if value["runtimeSemanticAuthority"] != SEMANTIC_AUTHORITY:
        raise ContractError("compiled artifact semantic authority mismatch")
    validate_derived_identity(value["derivedIdentity"])
    expected_ir_sha = _require_sha(value["compiledIrSha256"], "compiled artifact compiledIrSha256")
    _require_sha(
        value["compilerReceiptTokenSha256"],
        "compiled artifact compilerReceiptTokenSha256",
    )
    if not isinstance(value["compiledIr"], dict):
        raise ContractError("compiled artifact compiledIr must be an object")
    actual_ir_sha, execution_contract = validate_compiled_ir(
        value["compiledIr"],
        effect_id,
        compiler,
        revision,
        value["derivedIdentity"],
    )
    if actual_ir_sha != expected_ir_sha:
        raise ContractError("compiled artifact IR hash mismatch")
    if not execution_contract["executionAdmission"]:
        raise ContractError("compiled artifact embeds blocked compiled IR")
    _require_bool(value["executionAdmission"], True, "compiled artifact executionAdmission")
    _require_bool(value["productAdmission"], False, "compiled artifact productAdmission")


def validate_compiled_receipt(value: dict[str, Any]) -> None:
    keys = (
        "schema",
        "formatVersion",
        "effectAssetId",
        "artifactRevision",
        "compilerRevision",
        "runtimeSemanticAuthority",
        "derivedIdentity",
        "sourceContractVersion",
        "authoringCarrierSha256",
        "assemblySha256",
        "compiledArtifactSha256",
        "compiledIrSha256",
        "compilerReceiptRawSha256",
        "compilerReceiptCanonicalSha256",
        "compilerReceiptTokenSha256",
        "toolDependencies",
        "artifactBindingBlockerSet",
        "artifactBindingBlockerCount",
        "executionBlockerSet",
        "executionBlockerCount",
        "executionAdmission",
        "productAdmission",
        "publicationState",
    )
    _require_exact_keys(value, keys, "compiled receipt")
    if value["schema"] != COMPILED_RECEIPT_SCHEMA:
        raise ContractError("compiled receipt schema mismatch")
    _require_exact_int(value["formatVersion"], FORMAT_VERSION, "compiled receipt formatVersion")
    _require_stable_id(value["effectAssetId"], "compiled receipt effectAssetId")
    _require_positive_int(value["artifactRevision"], "compiled receipt artifactRevision")
    _require_stable_id(value["compilerRevision"], "compiled receipt compilerRevision")
    if value["runtimeSemanticAuthority"] != SEMANTIC_AUTHORITY:
        raise ContractError("compiled receipt semantic authority mismatch")
    validate_derived_identity(value["derivedIdentity"])
    _require_exact_int(
        value["sourceContractVersion"], SOURCE_CONTRACT_VERSION, "compiled receipt sourceContractVersion"
    )
    for field in (
        "authoringCarrierSha256",
        "assemblySha256",
        "compiledArtifactSha256",
        "compiledIrSha256",
        "compilerReceiptRawSha256",
        "compilerReceiptCanonicalSha256",
        "compilerReceiptTokenSha256",
    ):
        _require_sha(value[field], f"compiled receipt {field}")
    _validate_tool_dependencies(value["toolDependencies"])
    artifact_blockers = _require_string_array(
        value["artifactBindingBlockerSet"], "artifactBindingBlockerSet"
    )
    execution_blockers = _require_string_array(
        value["executionBlockerSet"], "executionBlockerSet"
    )
    _require_exact_int(
        value["artifactBindingBlockerCount"], len(artifact_blockers), "artifactBindingBlockerCount"
    )
    _require_exact_int(
        value["executionBlockerCount"], len(execution_blockers), "executionBlockerCount"
    )
    if artifact_blockers or execution_blockers:
        raise ContractError("compiled receipt blockers must be zero before execution admission")
    _require_bool(value["executionAdmission"], True, "compiled receipt executionAdmission")
    _require_bool(value["productAdmission"], False, "compiled receipt productAdmission")
    if value["publicationState"] != CODE_ONLY_PUBLICATION_STATE:
        raise ContractError("compiled receipt publicationState mismatch")


def _validate_request(request: dict[str, Any], input_root: Path) -> dict[str, Any]:
    keys = (
        "schema",
        "formatVersion",
        "effectAssetId",
        "displayName",
        "inputs",
        "compilerRevision",
        "artifactRevision",
        "artifactBindingBlockerSet",
        "executionBlockerSet",
    )
    _require_exact_keys(request, keys, "build request")
    if request["schema"] != BUILD_REQUEST_SCHEMA:
        raise ContractError("build request schema mismatch")
    _require_exact_int(request["formatVersion"], FORMAT_VERSION, "build request formatVersion")
    effect_id = _require_stable_id(request["effectAssetId"], "effectAssetId")
    display_name = _require_display_name(request["displayName"])
    compiler_revision = _require_stable_id(request["compilerRevision"], "compilerRevision")
    artifact_revision = _require_positive_int(request["artifactRevision"], "artifactRevision")
    artifact_blockers = _require_string_array(
        request["artifactBindingBlockerSet"], "artifactBindingBlockerSet"
    )
    execution_blockers = _require_string_array(
        request["executionBlockerSet"], "executionBlockerSet"
    )
    inputs = request["inputs"]
    if not isinstance(inputs, dict):
        raise ContractError("build request inputs must be an object")
    _require_exact_keys(
        inputs,
        tuple(name for name, _ in INPUT_TO_IDENTITY)
        + ("compiledIr", "compilerReceipt"),
        "build request inputs",
    )
    hashes: dict[str, str] = {}
    upstream_contracts: list[dict[str, Any]] = []
    for input_name, identity_name in INPUT_TO_IDENTITY:
        path, _, canonical_sha = _validate_file_reference(
            inputs[input_name], input_root, input_name
        )
        hashes[identity_name] = canonical_sha
        upstream_contracts.append(
            _validate_upstream_contract(input_name, load_json(path), effect_id)
        )
    derived_identity = _make_derived_identity(hashes)

    compiled_path, _, expected_compiled_sha = _validate_file_reference(
        inputs["compiledIr"], input_root, "compiledIr"
    )
    compiled_ir = load_json(compiled_path)
    actual_canonical_sha, compiled_ir_contract = validate_compiled_ir(
        compiled_ir,
        effect_id,
        compiler_revision,
        artifact_revision,
        derived_identity,
    )
    # The request pins canonical compiled semantics, not checkout-dependent JSON whitespace.
    if actual_canonical_sha != expected_compiled_sha:
        raise ContractError("compiledIr.sha256 must pin canonical JSON semantics")

    compiler_receipt_path, compiler_receipt_raw, compiler_receipt_canonical = (
        _validate_file_reference(
            inputs["compilerReceipt"], input_root, "compilerReceipt"
        )
    )
    compiler_receipt = load_json(compiler_receipt_path)
    compiler_receipt_token = validate_compiler_receipt(
        compiler_receipt,
        effect_id,
        compiler_revision,
        artifact_revision,
        derived_identity,
        actual_canonical_sha,
        compiled_ir_contract,
    )

    derived_contract = _combine_execution_contracts(
        (*upstream_contracts, compiled_ir_contract), "derived execution contract"
    )
    if artifact_blockers != derived_contract["artifactBindingBlockerSet"]:
        raise ContractError(
            "request artifactBindingBlockerSet does not match authenticated upstream union"
        )
    if execution_blockers != derived_contract["executionBlockerSet"]:
        raise ContractError(
            "request executionBlockerSet does not match authenticated upstream union"
        )
    if not derived_contract["executionAdmission"]:
        raise ContractError("derived output blocked by authenticated upstream evidence")

    return {
        "effectAssetId": effect_id,
        "displayName": display_name,
        "compilerRevision": compiler_revision,
        "artifactRevision": artifact_revision,
        "derivedIdentity": derived_identity,
        "compiledIr": compiled_ir,
        "compiledIrSha256": actual_canonical_sha,
        "compilerReceiptRawSha256": compiler_receipt_raw,
        "compilerReceiptCanonicalSha256": compiler_receipt_canonical,
        "compilerReceiptTokenSha256": compiler_receipt_token,
        "artifactBindingBlockerSet": derived_contract[
            "artifactBindingBlockerSet"
        ],
        "executionBlockerSet": derived_contract["executionBlockerSet"],
    }


def build_bundle_documents(request: dict[str, Any], input_root: Path) -> dict[str, dict[str, Any]]:
    normalized = _validate_request(request, input_root)
    effect_id = normalized["effectAssetId"]
    display_name = normalized["displayName"]
    identity = normalized["derivedIdentity"]
    compiled_identity = {
        "artifactRevision": normalized["artifactRevision"],
        "compilerRevision": normalized["compilerRevision"],
        "compiledIrSha256": normalized["compiledIrSha256"],
        "compilerReceiptTokenSha256": normalized[
            "compilerReceiptTokenSha256"
        ],
    }
    authoring = {
        "schema": AUTHORING_SCHEMA,
        "version": AUTHORING_VERSION,
        "effectAssetId": effect_id,
        "displayName": display_name,
        "documentRole": IDENTITY_CARRIER_ROLE,
        "runtimeSemanticAuthority": SEMANTIC_AUTHORITY,
        "sourceContract": {
            "formatVersion": SOURCE_CONTRACT_VERSION,
            "readOnly": True,
            "drawable": False,
        },
        "derivedIdentity": identity,
        "elements": [],
    }
    authoring_bytes = pretty_json_bytes(authoring)
    assembly = {
        "schema": ASSEMBLY_SCHEMA,
        "version": ASSEMBLY_VERSION,
        "effectAssetId": effect_id,
        "displayName": display_name,
        "assemblyRole": IDENTITY_CARRIER_ROLE,
        "sourceAuthoringVersion": AUTHORING_VERSION,
        "sourceDocumentFileSha256": sha256_bytes(authoring_bytes),
        "runtimeSemanticAuthority": SEMANTIC_AUTHORITY,
        "derivedIdentity": identity,
        "compiledArtifactIdentity": compiled_identity,
    }
    artifact = {
        "schema": COMPILED_ARTIFACT_SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "effectAssetId": effect_id,
        "artifactRevision": normalized["artifactRevision"],
        "compilerRevision": normalized["compilerRevision"],
        "runtimeSemanticAuthority": SEMANTIC_AUTHORITY,
        "derivedIdentity": identity,
        "compiledIrSha256": normalized["compiledIrSha256"],
        "compilerReceiptTokenSha256": normalized[
            "compilerReceiptTokenSha256"
        ],
        "compiledIr": normalized["compiledIr"],
        "executionAdmission": True,
        "productAdmission": False,
    }
    assembly_bytes = pretty_json_bytes(assembly)
    artifact_bytes = pretty_json_bytes(artifact)
    receipt = {
        "schema": COMPILED_RECEIPT_SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "effectAssetId": effect_id,
        "artifactRevision": normalized["artifactRevision"],
        "compilerRevision": normalized["compilerRevision"],
        "runtimeSemanticAuthority": SEMANTIC_AUTHORITY,
        "derivedIdentity": identity,
        "sourceContractVersion": SOURCE_CONTRACT_VERSION,
        "authoringCarrierSha256": sha256_bytes(authoring_bytes),
        "assemblySha256": sha256_bytes(assembly_bytes),
        "compiledArtifactSha256": sha256_bytes(artifact_bytes),
        "compiledIrSha256": normalized["compiledIrSha256"],
        "compilerReceiptRawSha256": normalized["compilerReceiptRawSha256"],
        "compilerReceiptCanonicalSha256": normalized[
            "compilerReceiptCanonicalSha256"
        ],
        "compilerReceiptTokenSha256": normalized[
            "compilerReceiptTokenSha256"
        ],
        "toolDependencies": _current_tool_dependencies(),
        "artifactBindingBlockerSet": normalized["artifactBindingBlockerSet"],
        "artifactBindingBlockerCount": 0,
        "executionBlockerSet": normalized["executionBlockerSet"],
        "executionBlockerCount": 0,
        "executionAdmission": True,
        "productAdmission": False,
        "publicationState": CODE_ONLY_PUBLICATION_STATE,
    }

    validate_authoring_carrier(authoring)
    validate_assembly_carrier(assembly)
    validate_compiled_artifact(artifact)
    validate_compiled_receipt(receipt)
    return {
        "authoring": authoring,
        "assembly": assembly,
        "artifact": artifact,
        "receipt": receipt,
    }


def _expected_output_name(effect_id: str, kind: str) -> str:
    suffixes = {
        "authoring": ".effect.json",
        "assembly": ".assembly.json",
        "artifact": ".compiled-effect.json",
        "receipt": ".compiled-effect.receipt.json",
    }
    return effect_id + suffixes[kind]


def _commit_staged_files(staged: list[tuple[Path, Path]]) -> None:
    backups: list[tuple[Path, Path]] = []
    committed: list[Path] = []
    try:
        for staged_path, target in staged:
            target.parent.mkdir(parents=True, exist_ok=True)
            backup = target.with_name(target.name + ".derived-backup")
            if backup.exists():
                raise ContractError(f"stale derived backup exists: {backup}")
            if target.exists():
                os.replace(target, backup)
                backups.append((backup, target))
            os.replace(staged_path, target)
            committed.append(target)
    except Exception:
        for target in reversed(committed):
            try:
                target.unlink(missing_ok=True)
            except OSError:
                pass
        for backup, target in reversed(backups):
            if backup.exists():
                os.replace(backup, target)
        raise
    for backup, _ in backups:
        backup.unlink(missing_ok=True)


def write_bundle_transactionally(
    documents: dict[str, dict[str, Any]], outputs: dict[str, Path]
) -> None:
    effect_id = documents["authoring"]["effectAssetId"]
    _require_exact_keys(outputs, ("authoring", "assembly", "artifact", "receipt"), "outputs")
    resolved_outputs: dict[str, Path] = {}
    for kind, target in outputs.items():
        target = target.resolve()
        if target.name != _expected_output_name(effect_id, kind):
            raise ContractError(f"{kind} output filename does not match effectAssetId")
        if target in resolved_outputs.values():
            raise ContractError("derived output paths must be unique")
        resolved_outputs[kind] = target

    stage_root = Path(tempfile.mkdtemp(prefix="lostark-effect-derived-"))
    try:
        staged: list[tuple[Path, Path]] = []
        for kind in ("authoring", "assembly", "artifact", "receipt"):
            payload = pretty_json_bytes(documents[kind])
            staged_path = stage_root / _expected_output_name(effect_id, kind)
            staged_path.write_bytes(payload)
            round_trip = load_json(staged_path)
            if canonical_json_bytes(round_trip) != canonical_json_bytes(documents[kind]):
                raise ContractError(f"{kind} staging round-trip mismatch")
            staged.append((staged_path, resolved_outputs[kind]))
        _commit_staged_files(staged)
    finally:
        shutil.rmtree(stage_root, ignore_errors=True)


def validate_bundle_files(
    authoring_path: Path,
    assembly_path: Path,
    artifact_path: Path,
    receipt_path: Path,
) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any], dict[str, Any]]:
    authoring = load_json(authoring_path)
    assembly = load_json(assembly_path)
    artifact = load_json(artifact_path)
    receipt = load_json(receipt_path)
    validate_authoring_carrier(authoring)
    validate_assembly_carrier(assembly)
    validate_compiled_artifact(artifact)
    validate_compiled_receipt(receipt)

    effect_id = authoring["effectAssetId"]
    identity = authoring["derivedIdentity"]
    compiler_revision = artifact["compilerRevision"]
    artifact_revision = artifact["artifactRevision"]
    compiled_ir_sha = artifact["compiledIrSha256"]
    for label, value in (
        ("Assembly", assembly),
        ("compiled artifact", artifact),
        ("compiled receipt", receipt),
    ):
        if value["effectAssetId"] != effect_id:
            raise ContractError(f"{label} effectAssetId mismatch")
        if value["derivedIdentity"] != identity:
            raise ContractError(f"{label} six-hash identity mismatch")
    if assembly["sourceDocumentFileSha256"] != sha256_file(authoring_path):
        raise ContractError("Assembly authoring carrier raw SHA mismatch")
    compiled_identity = assembly["compiledArtifactIdentity"]
    if (
        compiled_identity["compilerRevision"] != compiler_revision
        or compiled_identity["artifactRevision"] != artifact_revision
        or compiled_identity["compiledIrSha256"] != compiled_ir_sha
        or compiled_identity["compilerReceiptTokenSha256"]
        != artifact["compilerReceiptTokenSha256"]
    ):
        raise ContractError("Assembly compiled artifact identity mismatch")
    if (
        receipt["compilerRevision"] != compiler_revision
        or receipt["artifactRevision"] != artifact_revision
        or receipt["compiledIrSha256"] != compiled_ir_sha
        or receipt["compilerReceiptTokenSha256"]
        != artifact["compilerReceiptTokenSha256"]
    ):
        raise ContractError("compiled receipt revision mismatch")
    expected_raw = {
        "authoringCarrierSha256": sha256_file(authoring_path),
        "assemblySha256": sha256_file(assembly_path),
        "compiledArtifactSha256": sha256_file(artifact_path),
    }
    for field, expected in expected_raw.items():
        if receipt[field] != expected:
            raise ContractError(f"compiled receipt {field} raw SHA mismatch")
    return authoring, assembly, artifact, receipt


def _parse_reconstructed_candidate(payload: bytes, label: str) -> dict[str, Any]:
    if payload.startswith(b"\xef\xbb\xbf"):
        raise ContractError(f"{label} must be UTF-8 without BOM")
    if b"\r" in payload:
        raise ContractError(f"{label} must use LF-only newlines")
    try:
        text = payload.decode("utf-8")
        value = json.loads(
            text,
            object_pairs_hook=_object_no_duplicates,
            parse_constant=lambda token: (_ for _ in ()).throw(
                ContractError(f"{label} contains non-finite JSON token: {token}")
            ),
        )
    except (UnicodeError, json.JSONDecodeError) as exc:
        raise ContractError(f"cannot parse {label}: {exc}") from exc
    if not isinstance(value, dict):
        raise ContractError(f"{label} root must be an object")
    return value


def _validate_reconstructed_candidate_identity(
    payload: bytes, require_frozen_builder: bool
) -> dict[str, Any]:
    if len(payload) != RECONSTRUCTED_CANDIDATE_BYTE_COUNT:
        raise ContractError("reconstructed candidate byte count mismatch")
    if sha256_bytes(payload) != RECONSTRUCTED_CANDIDATE_RAW_SHA256:
        raise ContractError("reconstructed candidate raw SHA mismatch")
    candidate = _parse_reconstructed_candidate(payload, "reconstructed candidate")
    if candidate.get("schema") != "lostark.artist-31470-reconstructed-runtime-program":
        raise ContractError("reconstructed candidate schema mismatch")
    _require_exact_int(
        candidate.get("formatVersion"),
        RECONSTRUCTED_PROGRAM_VERSION,
        "reconstructed candidate formatVersion",
    )
    if candidate.get("programId") != RECONSTRUCTED_PROGRAM_ID:
        raise ContractError("reconstructed candidate programId mismatch")
    _require_exact_int(
        candidate.get("programVersion"),
        RECONSTRUCTED_PROGRAM_VERSION,
        "reconstructed candidate programVersion",
    )
    if candidate.get("programSha256") != RECONSTRUCTED_PROGRAM_SHA256:
        raise ContractError("reconstructed candidate program SHA mismatch")
    target = candidate.get("target")
    if not isinstance(target, dict) or target.get("runtimeCatalogAssetId") != (
        RECONSTRUCTED_EFFECT_ID
    ):
        raise ContractError("reconstructed candidate target identity mismatch")
    admission = candidate.get("admission")
    summary = candidate.get("summary")
    if (
        not isinstance(admission, dict)
        or admission.get("sourceExact") is not False
        or admission.get("runtimeExecution") is not False
        or admission.get("product") is not False
        or not isinstance(summary, dict)
        or summary.get("runtimeExecution") is not False
        or summary.get("product") is not False
    ):
        raise ContractError("reconstructed candidate admission must remain false")
    input_artifacts = candidate.get("inputArtifacts")
    if not isinstance(input_artifacts, list) or len(input_artifacts) != (
        RECONSTRUCTED_INPUT_ARTIFACT_COUNT
    ):
        raise ContractError("reconstructed candidate input artifact count mismatch")
    ordered_row_shas: list[str] = []
    for index, row in enumerate(input_artifacts):
        if not isinstance(row, dict):
            raise ContractError(
                f"reconstructed candidate inputArtifacts[{index}] must be an object"
            )
        ordered_row_shas.append(
            _require_sha(
                row.get("rowSha256"),
                f"reconstructed candidate inputArtifacts[{index}].rowSha256",
            )
        )
    if sha256_bytes(canonical_json_bytes(ordered_row_shas)) != (
        RECONSTRUCTED_INPUT_ARTIFACTS_ORDERED_SHA256
    ):
        raise ContractError("reconstructed candidate input artifact projection mismatch")
    if require_frozen_builder:
        module_path = REPOSITORY_ROOT / (
            "Tools/EffectPipeline/"
            "build_artist_31470_reconstructed_runtime_program.py"
        )
        try:
            spec = importlib.util.spec_from_file_location(
                "artist_31470_reconstructed_runtime_program_authority",
                module_path,
            )
            if spec is None or spec.loader is None:
                raise ContractError("cannot load reconstructed candidate builder")
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            expected = module.build_program()
            module.validate_program(candidate, expected=expected)
            if module.output_bytes(expected) != payload:
                raise ContractError(
                    "reconstructed candidate differs from frozen builder output"
                )
        except ContractError:
            raise
        except Exception as exc:
            raise ContractError(
                f"frozen reconstructed candidate builder rejected input: {exc}"
            ) from exc
        candidate_path = (
            "Data/Effects/Imported/Artist/Candidates/"
            "skill.31470.reconstructed-runtime-program.candidate.json"
        )
        try:
            tree = subprocess.run(
                [
                    "git",
                    "-C",
                    str(REPOSITORY_ROOT),
                    "rev-parse",
                    f"{RECONSTRUCTED_CANDIDATE_BUILDER_COMMIT}^{{tree}}",
                ],
                check=True,
                capture_output=True,
                text=True,
            ).stdout.strip()
            blob = subprocess.run(
                [
                    "git",
                    "-C",
                    str(REPOSITORY_ROOT),
                    "rev-parse",
                    f"{RECONSTRUCTED_CANDIDATE_BUILDER_COMMIT}:{candidate_path}",
                ],
                check=True,
                capture_output=True,
                text=True,
            ).stdout.strip()
            authority_payload = subprocess.run(
                ["git", "-C", str(REPOSITORY_ROOT), "cat-file", "blob", blob],
                check=True,
                capture_output=True,
            ).stdout
        except (OSError, subprocess.CalledProcessError) as exc:
            raise ContractError(
                f"cannot authenticate reconstructed candidate Git authority: {exc}"
            ) from exc
        if tree != RECONSTRUCTED_CANDIDATE_BUILDER_TREE:
            raise ContractError("reconstructed candidate builder tree mismatch")
        if blob != RECONSTRUCTED_CANDIDATE_BLOB:
            raise ContractError("reconstructed candidate blob mismatch")
        if authority_payload != payload:
            raise ContractError("reconstructed candidate authority blob byte mismatch")
    return candidate


def _historical_reconstructed_tool_dependencies() -> list[dict[str, Any]]:
    return [
        {
            "role": role,
            "path": relative,
            "hashDomain": "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
            "sha256": digest,
        }
        for role, relative, digest in RECONSTRUCTED_HISTORICAL_TOOL_DEPENDENCIES
    ]


def _current_render_resource_tool_dependencies() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for role, relative in RECONSTRUCTED_RENDER_RESOURCE_TOOL_DEPENDENCIES:
        path = REPOSITORY_ROOT / relative
        try:
            payload = path.read_bytes()
        except OSError as exc:
            raise ContractError(
                f"cannot read render-resource publisher dependency {relative}: {exc}"
            ) from exc
        rows.append(
            {
                "role": role,
                "path": relative,
                "hashDomain": "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
                "sha256": sha256_bytes(
                    canonical_text_bytes(
                        payload, f"render-resource tool dependency {relative}"
                    )
                ),
            }
        )
    return rows


def _make_reconstructed_link(candidate_payload: bytes) -> dict[str, Any]:
    _validate_reconstructed_candidate_identity(candidate_payload, True)
    return {
        "schema": RECONSTRUCTED_LINK_SCHEMA,
        "formatVersion": 1,
        "encoding": RECONSTRUCTED_ENCODING,
        "effectAssetId": RECONSTRUCTED_EFFECT_ID,
        "candidateBuilderCommitId": RECONSTRUCTED_CANDIDATE_BUILDER_COMMIT,
        "candidateBuilderTreeId": RECONSTRUCTED_CANDIDATE_BUILDER_TREE,
        "candidateBlobId": RECONSTRUCTED_CANDIDATE_BLOB,
        "resourceBindingHash": RECONSTRUCTED_RESOURCE_BINDING_SHA256,
        "inputArtifactCount": RECONSTRUCTED_INPUT_ARTIFACT_COUNT,
        "inputArtifactsOrderedSha256": (
            RECONSTRUCTED_INPUT_ARTIFACTS_ORDERED_SHA256
        ),
        "programId": RECONSTRUCTED_PROGRAM_ID,
        "programVersion": RECONSTRUCTED_PROGRAM_VERSION,
        "programSha256": RECONSTRUCTED_PROGRAM_SHA256,
        "candidateRawSha256": RECONSTRUCTED_CANDIDATE_RAW_SHA256,
        "candidateByteCount": RECONSTRUCTED_CANDIDATE_BYTE_COUNT,
        "candidateUtf8Json": candidate_payload.decode("utf-8"),
    }


def _make_reconstructed_publish_receipt(link: dict[str, Any]) -> dict[str, Any]:
    receipt = {
        "schema": RECONSTRUCTED_RECEIPT_SCHEMA,
        "formatVersion": 1,
        "receiptRole": RECONSTRUCTED_RECEIPT_ROLE,
        "payloadKind": RECONSTRUCTED_PAYLOAD_KIND,
        "effectAssetId": RECONSTRUCTED_EFFECT_ID,
        "artifactRevision": RECONSTRUCTED_ARTIFACT_REVISION,
        "compilerRevision": RECONSTRUCTED_COMPILER_REVISION,
        "sourceExact": False,
        "runtimeExecutionAdmission": False,
        "productAdmission": False,
        "candidateBuilderCommitId": RECONSTRUCTED_CANDIDATE_BUILDER_COMMIT,
        "candidateBuilderTreeId": RECONSTRUCTED_CANDIDATE_BUILDER_TREE,
        "candidateBlobId": RECONSTRUCTED_CANDIDATE_BLOB,
        "resourceBindingHash": RECONSTRUCTED_RESOURCE_BINDING_SHA256,
        "inputArtifactCount": RECONSTRUCTED_INPUT_ARTIFACT_COUNT,
        "inputArtifactsOrderedSha256": (
            RECONSTRUCTED_INPUT_ARTIFACTS_ORDERED_SHA256
        ),
        "programId": RECONSTRUCTED_PROGRAM_ID,
        "programVersion": RECONSTRUCTED_PROGRAM_VERSION,
        "programSha256": RECONSTRUCTED_PROGRAM_SHA256,
        "candidateRawSha256": RECONSTRUCTED_CANDIDATE_RAW_SHA256,
        "candidateByteCount": RECONSTRUCTED_CANDIDATE_BYTE_COUNT,
        "reconstructedRuntimeProgramSha256": sha256_bytes(
            canonical_json_bytes(link)
        ),
        "toolDependencies": _historical_reconstructed_tool_dependencies(),
        "receiptSha256Domain": RECONSTRUCTED_RECEIPT_DIGEST_DOMAIN,
    }
    receipt["receiptSha256"] = sha256_bytes(canonical_json_bytes(receipt))
    return receipt


def _validate_historical_reconstructed_tool_dependencies(value: Any) -> None:
    if not isinstance(value, list) or len(value) != len(
        RECONSTRUCTED_HISTORICAL_TOOL_DEPENDENCIES
    ):
        raise ContractError("historical reconstructed tool dependency count mismatch")
    expected = _historical_reconstructed_tool_dependencies()
    for index, ((role, path, digest), row) in enumerate(
        zip(RECONSTRUCTED_HISTORICAL_TOOL_DEPENDENCIES, value, strict=True)
    ):
        label = f"historical reconstructed receipt toolDependencies[{index}]"
        if not isinstance(row, dict):
            raise ContractError(f"{label} must be an object")
        _require_exact_key_order(
            row, ("role", "path", "hashDomain", "sha256"), label
        )
        if (
            row["role"] != role
            or row["path"] != path
            or row["hashDomain"] != "TRACKED_SOURCE_EOL_CANONICAL_TEXT"
            or row["sha256"] != digest
        ):
            raise ContractError(f"{label} identity mismatch")
        _require_sha(row["sha256"], f"{label}.sha256")
        if row != expected[index]:
            raise ContractError(f"{label} frozen source hash mismatch")


def validate_reconstructed_publish_receipt(
    receipt: Any,
    link: dict[str, Any],
) -> None:
    if not isinstance(receipt, dict):
        raise ContractError("reconstructed publish receipt must be an object")
    _require_exact_key_order(
        receipt, RECONSTRUCTED_RECEIPT_KEYS, "reconstructed publish receipt"
    )
    expected_scalars = {
        "schema": RECONSTRUCTED_RECEIPT_SCHEMA,
        "formatVersion": 1,
        "receiptRole": RECONSTRUCTED_RECEIPT_ROLE,
        "payloadKind": RECONSTRUCTED_PAYLOAD_KIND,
        "effectAssetId": RECONSTRUCTED_EFFECT_ID,
        "artifactRevision": RECONSTRUCTED_ARTIFACT_REVISION,
        "compilerRevision": RECONSTRUCTED_COMPILER_REVISION,
        "sourceExact": False,
        "runtimeExecutionAdmission": False,
        "productAdmission": False,
        "candidateBuilderCommitId": RECONSTRUCTED_CANDIDATE_BUILDER_COMMIT,
        "candidateBuilderTreeId": RECONSTRUCTED_CANDIDATE_BUILDER_TREE,
        "candidateBlobId": RECONSTRUCTED_CANDIDATE_BLOB,
        "resourceBindingHash": RECONSTRUCTED_RESOURCE_BINDING_SHA256,
        "inputArtifactCount": RECONSTRUCTED_INPUT_ARTIFACT_COUNT,
        "inputArtifactsOrderedSha256": (
            RECONSTRUCTED_INPUT_ARTIFACTS_ORDERED_SHA256
        ),
        "programId": RECONSTRUCTED_PROGRAM_ID,
        "programVersion": RECONSTRUCTED_PROGRAM_VERSION,
        "programSha256": RECONSTRUCTED_PROGRAM_SHA256,
        "candidateRawSha256": RECONSTRUCTED_CANDIDATE_RAW_SHA256,
        "candidateByteCount": RECONSTRUCTED_CANDIDATE_BYTE_COUNT,
        "reconstructedRuntimeProgramSha256": sha256_bytes(
            canonical_json_bytes(link)
        ),
        "receiptSha256Domain": RECONSTRUCTED_RECEIPT_DIGEST_DOMAIN,
    }
    for field, expected in expected_scalars.items():
        actual = receipt[field]
        if type(expected) is int:
            _require_exact_int(actual, expected, f"reconstructed receipt {field}")
        elif type(expected) is bool:
            _require_bool(actual, expected, f"reconstructed receipt {field}")
        elif actual != expected:
            raise ContractError(f"reconstructed receipt {field} mismatch")
    _validate_historical_reconstructed_tool_dependencies(
        receipt["toolDependencies"]
    )
    receipt_sha = _require_sha(
        receipt["receiptSha256"], "reconstructed receipt receiptSha256"
    )
    unsigned = dict(receipt)
    del unsigned["receiptSha256"]
    if receipt_sha != sha256_bytes(canonical_json_bytes(unsigned)):
        raise ContractError("reconstructed publish receipt self digest mismatch")


def _validate_reconstructed_base_entry(value: Any) -> None:
    if not isinstance(value, dict):
        raise ContractError("historical reconstructed base entry must be an object")
    _require_exact_key_order(
        value,
        RECONSTRUCTED_BASE_ENTRY_KEYS,
        "historical reconstructed base entry",
    )
    if value["payloadKind"] != RECONSTRUCTED_PAYLOAD_KIND:
        raise ContractError("reconstructed runtime payloadKind mismatch")
    if value["effectAssetId"] != RECONSTRUCTED_EFFECT_ID:
        raise ContractError("reconstructed runtime effect identity mismatch")
    _require_exact_int(
        value["artifactRevision"],
        RECONSTRUCTED_ARTIFACT_REVISION,
        "reconstructed runtime artifactRevision",
    )
    if value["compilerRevision"] != RECONSTRUCTED_COMPILER_REVISION:
        raise ContractError("reconstructed runtime compilerRevision mismatch")
    _require_bool(value["sourceExact"], False, "reconstructed runtime sourceExact")
    _require_bool(
        value["runtimeExecutionAdmission"],
        False,
        "reconstructed runtime runtimeExecutionAdmission",
    )
    _require_bool(
        value["productAdmission"], False, "reconstructed runtime productAdmission"
    )
    link = value["reconstructedRuntimeProgram"]
    if not isinstance(link, dict):
        raise ContractError("reconstructed runtime link must be an object")
    _require_exact_key_order(link, RECONSTRUCTED_LINK_KEYS, "reconstructed runtime link")
    expected_link_scalars = {
        "schema": RECONSTRUCTED_LINK_SCHEMA,
        "formatVersion": 1,
        "encoding": RECONSTRUCTED_ENCODING,
        "effectAssetId": RECONSTRUCTED_EFFECT_ID,
        "candidateBuilderCommitId": RECONSTRUCTED_CANDIDATE_BUILDER_COMMIT,
        "candidateBuilderTreeId": RECONSTRUCTED_CANDIDATE_BUILDER_TREE,
        "candidateBlobId": RECONSTRUCTED_CANDIDATE_BLOB,
        "resourceBindingHash": RECONSTRUCTED_RESOURCE_BINDING_SHA256,
        "inputArtifactCount": RECONSTRUCTED_INPUT_ARTIFACT_COUNT,
        "inputArtifactsOrderedSha256": (
            RECONSTRUCTED_INPUT_ARTIFACTS_ORDERED_SHA256
        ),
        "programId": RECONSTRUCTED_PROGRAM_ID,
        "programVersion": RECONSTRUCTED_PROGRAM_VERSION,
        "programSha256": RECONSTRUCTED_PROGRAM_SHA256,
        "candidateRawSha256": RECONSTRUCTED_CANDIDATE_RAW_SHA256,
        "candidateByteCount": RECONSTRUCTED_CANDIDATE_BYTE_COUNT,
    }
    for field, expected in expected_link_scalars.items():
        actual = link[field]
        if type(expected) is int:
            _require_exact_int(actual, expected, f"reconstructed link {field}")
        elif actual != expected:
            raise ContractError(f"reconstructed link {field} mismatch")
    if (
        link["effectAssetId"] != value["effectAssetId"]
        or value["artifactRevision"] != RECONSTRUCTED_ARTIFACT_REVISION
        or value["compilerRevision"] != RECONSTRUCTED_COMPILER_REVISION
    ):
        raise ContractError("reconstructed outer/link identity mismatch")
    candidate_text = link["candidateUtf8Json"]
    if not isinstance(candidate_text, str):
        raise ContractError("reconstructed candidateUtf8Json must be a string")
    try:
        candidate_payload = candidate_text.encode("utf-8")
    except UnicodeEncodeError as exc:
        raise ContractError("reconstructed candidateUtf8Json is not UTF-8") from exc
    _validate_reconstructed_candidate_identity(candidate_payload, False)
    receipt = value["publishReceipt"]
    validate_reconstructed_publish_receipt(receipt, link)
    outer_receipt_sha = _require_sha(
        value["publishReceiptSha256"],
        "reconstructed runtime publishReceiptSha256",
    )
    if outer_receipt_sha != sha256_bytes(canonical_json_bytes(receipt)):
        raise ContractError("reconstructed runtime publish receipt SHA mismatch")
    if sha256_bytes(canonical_json_bytes(link)) != RECONSTRUCTED_BASE_LINK_SHA256:
        raise ContractError("historical reconstructed link identity mismatch")
    if receipt["receiptSha256"] != RECONSTRUCTED_BASE_RECEIPT_SELF_SHA256:
        raise ContractError("historical reconstructed receipt self identity mismatch")
    if outer_receipt_sha != RECONSTRUCTED_BASE_RECEIPT_SHA256:
        raise ContractError("historical reconstructed receipt identity mismatch")
    if sha256_bytes(canonical_json_bytes(value)) != (
        RECONSTRUCTED_BASE_ENTRY_CANONICAL_SHA256
    ):
        raise ContractError("historical reconstructed base projection mismatch")


def _validate_render_resource_sidecar_identity(
    payload: bytes,
) -> dict[str, Any]:
    if len(payload) != RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_BYTE_COUNT:
        raise ContractError("render-resource sidecar byte count mismatch")
    if sha256_bytes(payload) != RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_RAW_SHA256:
        raise ContractError("render-resource sidecar raw SHA mismatch")
    sidecar = _parse_reconstructed_candidate(payload, "render-resource sidecar")
    _require_exact_key_order(
        sidecar,
        RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_ROOT_KEYS,
        "render-resource sidecar",
    )
    if sidecar["schema"] != RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_SCHEMA:
        raise ContractError("render-resource sidecar schema mismatch")
    _require_exact_int(
        sidecar["formatVersion"], 1, "render-resource sidecar formatVersion"
    )
    if (
        sidecar["authorityId"]
        != RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_AUTHORITY_ID
        or sidecar["characterClass"] != "ARTIST"
        or sidecar["inputSlot"] != "F"
    ):
        raise ContractError("render-resource sidecar owner identity mismatch")
    _require_exact_int(sidecar["skillId"], 31470, "render-resource sidecar skillId")

    unsigned = dict(sidecar)
    sidecar_receipt_sha = _require_sha(
        unsigned.pop("receiptSha256"), "render-resource sidecar receiptSha256"
    )
    if (
        sidecar_receipt_sha
        != RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_RECEIPT_SHA256
        or sidecar_receipt_sha != sha256_bytes(canonical_json_bytes(unsigned))
    ):
        raise ContractError("render-resource sidecar self digest mismatch")
    if (
        _require_sha(
            sidecar["decisionProjectionSha256"],
            "render-resource sidecar decisionProjectionSha256",
        )
        != RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_DECISION_SHA256
    ):
        raise ContractError("render-resource sidecar decision identity mismatch")

    pin_path = REPOSITORY_ROOT / (
        "Tools/LevelPlacementExtractor/"
        "artist_31470_reconstructed_render_resource_authority.py"
    )
    try:
        spec = importlib.util.spec_from_file_location(
            "artist_31470_render_resource_bridge_pins", pin_path
        )
        if spec is None or spec.loader is None:
            raise ContractError("cannot load render-resource independent pins")
        pin_module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(pin_module)
        pin_module.require_approved_receipt(sidecar)
    except ContractError:
        raise
    except Exception as exc:
        raise ContractError(
            f"render-resource independent pins rejected sidecar: {exc}"
        ) from exc

    program = sidecar.get("sourceEvidence", {}).get("programAndParserTuple")
    publisher = sidecar.get("sourceEvidence", {}).get(
        "publisherRuntimeCatalogAuthority"
    )
    if not isinstance(program, dict) or not isinstance(publisher, dict):
        raise ContractError("render-resource sidecar source evidence is invalid")
    if (
        program.get("programId") != RECONSTRUCTED_PROGRAM_ID
        or program.get("programSha256") != RECONSTRUCTED_PROGRAM_SHA256
        or program.get("rawSha256") != RECONSTRUCTED_CANDIDATE_RAW_SHA256
    ):
        raise ContractError("render-resource sidecar program identity mismatch")
    _require_exact_int(
        program.get("programVersion"),
        RECONSTRUCTED_PROGRAM_VERSION,
        "render-resource sidecar programVersion",
    )
    _require_exact_int(
        program.get("rawByteCount"),
        RECONSTRUCTED_CANDIDATE_BYTE_COUNT,
        "render-resource sidecar candidate byte count",
    )
    if (
        publisher.get("effectAssetId") != RECONSTRUCTED_EFFECT_ID
        or publisher.get("outerCanonicalSha256")
        != RECONSTRUCTED_BASE_ENTRY_CANONICAL_SHA256
        or publisher.get("linkCanonicalSha256")
        != RECONSTRUCTED_BASE_LINK_SHA256
        or publisher.get("receiptSelfSha256")
        != RECONSTRUCTED_BASE_RECEIPT_SELF_SHA256
        or publisher.get("outerPublishReceiptSha256")
        != RECONSTRUCTED_BASE_RECEIPT_SHA256
    ):
        raise ContractError("render-resource sidecar historical base mismatch")

    authority = sidecar.get("authorityContract")
    admission = sidecar.get("admission")
    blocker = sidecar.get("blockerProjection")
    summary = sidecar.get("summary")
    for label, value in (
        ("authorityContract", authority),
        ("admission", admission),
        ("blockerProjection", blocker),
        ("summary", summary),
    ):
        if not isinstance(value, dict):
            raise ContractError(f"render-resource sidecar {label} must be an object")
        _require_bool(value.get("sourceExact"), False, f"sidecar {label}.sourceExact")
        _require_bool(
            value.get("runtimeExecutionAdmission"),
            False,
            f"sidecar {label}.runtimeExecutionAdmission",
        )
        _require_bool(value.get("product"), False, f"sidecar {label}.product")
    _require_bool(
        authority.get("actionTimeIoAllowed"),
        False,
        "sidecar authorityContract.actionTimeIoAllowed",
    )
    _require_bool(
        blocker.get("actionTimeIoAllowed"),
        False,
        "sidecar blockerProjection.actionTimeIoAllowed",
    )
    return sidecar


def _make_render_resource_link(sidecar_payload: bytes) -> dict[str, Any]:
    sidecar = _validate_render_resource_sidecar_identity(sidecar_payload)
    return {
        "schema": RECONSTRUCTED_RENDER_RESOURCE_LINK_SCHEMA,
        "formatVersion": 1,
        "encoding": RECONSTRUCTED_ENCODING,
        "effectAssetId": RECONSTRUCTED_EFFECT_ID,
        "programId": RECONSTRUCTED_PROGRAM_ID,
        "programVersion": RECONSTRUCTED_PROGRAM_VERSION,
        "programSha256": RECONSTRUCTED_PROGRAM_SHA256,
        "sidecarSchema": RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_SCHEMA,
        "sidecarFormatVersion": 1,
        "sidecarAuthorityId": RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_AUTHORITY_ID,
        "sidecarDecisionProjectionSha256": (
            RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_DECISION_SHA256
        ),
        "sidecarReceiptSha256": (
            RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_RECEIPT_SHA256
        ),
        "sidecarRawSha256": RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_RAW_SHA256,
        "sidecarByteCount": RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_BYTE_COUNT,
        "sourceExact": False,
        "runtimeExecutionAdmission": False,
        "executeAdmission": False,
        "submitAdmission": False,
        "renderAdmission": False,
        "productAdmission": False,
        "sidecarUtf8Json": sidecar_payload.decode("utf-8"),
    }


def _make_render_resource_publish_receipt(
    base_entry: dict[str, Any],
    resource_link: dict[str, Any],
) -> dict[str, Any]:
    receipt = {
        "schema": RECONSTRUCTED_RENDER_RESOURCE_RECEIPT_SCHEMA,
        "formatVersion": 1,
        "receiptRole": RECONSTRUCTED_RENDER_RESOURCE_RECEIPT_ROLE,
        "payloadKind": RECONSTRUCTED_PAYLOAD_KIND,
        "effectAssetId": RECONSTRUCTED_EFFECT_ID,
        "artifactRevision": RECONSTRUCTED_ARTIFACT_REVISION,
        "compilerRevision": RECONSTRUCTED_COMPILER_REVISION,
        "sourceExact": False,
        "runtimeExecutionAdmission": False,
        "executeAdmission": False,
        "submitAdmission": False,
        "renderAdmission": False,
        "productAdmission": False,
        "programId": RECONSTRUCTED_PROGRAM_ID,
        "programVersion": RECONSTRUCTED_PROGRAM_VERSION,
        "programSha256": RECONSTRUCTED_PROGRAM_SHA256,
        "baseRuntimeEntryProjectionSha256": sha256_bytes(
            canonical_json_bytes(base_entry)
        ),
        "reconstructedRuntimeProgramSha256": sha256_bytes(
            canonical_json_bytes(base_entry["reconstructedRuntimeProgram"])
        ),
        "basePublishReceiptSha256": base_entry["publishReceiptSha256"],
        "renderResourceAuthorityLinkSha256": sha256_bytes(
            canonical_json_bytes(resource_link)
        ),
        "sidecarRawSha256": RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_RAW_SHA256,
        "sidecarReceiptSha256": (
            RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_RECEIPT_SHA256
        ),
        "sidecarDecisionProjectionSha256": (
            RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_DECISION_SHA256
        ),
        "toolDependencies": _current_render_resource_tool_dependencies(),
        "receiptSha256Domain": RECONSTRUCTED_RECEIPT_DIGEST_DOMAIN,
    }
    receipt["receiptSha256"] = sha256_bytes(canonical_json_bytes(receipt))
    return receipt


def _validate_current_render_resource_tool_dependencies(value: Any) -> None:
    if not isinstance(value, list) or len(value) != len(
        RECONSTRUCTED_RENDER_RESOURCE_TOOL_DEPENDENCIES
    ):
        raise ContractError("render-resource tool dependency count mismatch")
    expected = _current_render_resource_tool_dependencies()
    for index, ((role, path), row) in enumerate(
        zip(RECONSTRUCTED_RENDER_RESOURCE_TOOL_DEPENDENCIES, value, strict=True)
    ):
        label = f"render-resource receipt toolDependencies[{index}]"
        if not isinstance(row, dict):
            raise ContractError(f"{label} must be an object")
        _require_exact_key_order(
            row, ("role", "path", "hashDomain", "sha256"), label
        )
        if (
            row["role"] != role
            or row["path"] != path
            or row["hashDomain"] != "TRACKED_SOURCE_EOL_CANONICAL_TEXT"
        ):
            raise ContractError(f"{label} identity mismatch")
        _require_sha(row["sha256"], f"{label}.sha256")
        if row != expected[index]:
            raise ContractError(f"{label} current source hash mismatch")


def validate_reconstructed_render_resource_publish_receipt(
    receipt: Any,
    base_entry: dict[str, Any],
    resource_link: dict[str, Any],
) -> None:
    if not isinstance(receipt, dict):
        raise ContractError("render-resource publish receipt must be an object")
    _require_exact_key_order(
        receipt,
        RECONSTRUCTED_RENDER_RESOURCE_RECEIPT_KEYS,
        "render-resource publish receipt",
    )
    expected_scalars = {
        "schema": RECONSTRUCTED_RENDER_RESOURCE_RECEIPT_SCHEMA,
        "formatVersion": 1,
        "receiptRole": RECONSTRUCTED_RENDER_RESOURCE_RECEIPT_ROLE,
        "payloadKind": RECONSTRUCTED_PAYLOAD_KIND,
        "effectAssetId": RECONSTRUCTED_EFFECT_ID,
        "artifactRevision": RECONSTRUCTED_ARTIFACT_REVISION,
        "compilerRevision": RECONSTRUCTED_COMPILER_REVISION,
        "sourceExact": False,
        "runtimeExecutionAdmission": False,
        "executeAdmission": False,
        "submitAdmission": False,
        "renderAdmission": False,
        "productAdmission": False,
        "programId": RECONSTRUCTED_PROGRAM_ID,
        "programVersion": RECONSTRUCTED_PROGRAM_VERSION,
        "programSha256": RECONSTRUCTED_PROGRAM_SHA256,
        "baseRuntimeEntryProjectionSha256": RECONSTRUCTED_BASE_ENTRY_CANONICAL_SHA256,
        "reconstructedRuntimeProgramSha256": RECONSTRUCTED_BASE_LINK_SHA256,
        "basePublishReceiptSha256": RECONSTRUCTED_BASE_RECEIPT_SHA256,
        "renderResourceAuthorityLinkSha256": sha256_bytes(
            canonical_json_bytes(resource_link)
        ),
        "sidecarRawSha256": RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_RAW_SHA256,
        "sidecarReceiptSha256": (
            RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_RECEIPT_SHA256
        ),
        "sidecarDecisionProjectionSha256": (
            RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_DECISION_SHA256
        ),
        "receiptSha256Domain": RECONSTRUCTED_RECEIPT_DIGEST_DOMAIN,
    }
    if sha256_bytes(canonical_json_bytes(base_entry)) != (
        RECONSTRUCTED_BASE_ENTRY_CANONICAL_SHA256
    ):
        raise ContractError("render-resource receipt base projection mismatch")
    for field, expected in expected_scalars.items():
        actual = receipt[field]
        if type(expected) is int:
            _require_exact_int(actual, expected, f"render-resource receipt {field}")
        elif type(expected) is bool:
            _require_bool(actual, expected, f"render-resource receipt {field}")
        elif actual != expected:
            raise ContractError(f"render-resource receipt {field} mismatch")
    _validate_current_render_resource_tool_dependencies(
        receipt["toolDependencies"]
    )
    receipt_sha = _require_sha(
        receipt["receiptSha256"], "render-resource receipt receiptSha256"
    )
    unsigned = dict(receipt)
    del unsigned["receiptSha256"]
    if receipt_sha != sha256_bytes(canonical_json_bytes(unsigned)):
        raise ContractError("render-resource publish receipt self digest mismatch")


def validate_reconstructed_runtime_entry(value: Any) -> None:
    if not isinstance(value, dict):
        raise ContractError("reconstructed runtime entry must be an object")
    _require_exact_key_order(
        value, RECONSTRUCTED_ENTRY_KEYS, "reconstructed runtime entry"
    )
    base_entry = {key: value[key] for key in RECONSTRUCTED_BASE_ENTRY_KEYS}
    _validate_reconstructed_base_entry(base_entry)

    resource_link = value["reconstructedRenderResourceAuthority"]
    if not isinstance(resource_link, dict):
        raise ContractError("reconstructed render-resource link must be an object")
    _require_exact_key_order(
        resource_link,
        RECONSTRUCTED_RENDER_RESOURCE_LINK_KEYS,
        "reconstructed render-resource link",
    )
    expected_scalars = {
        "schema": RECONSTRUCTED_RENDER_RESOURCE_LINK_SCHEMA,
        "formatVersion": 1,
        "encoding": RECONSTRUCTED_ENCODING,
        "effectAssetId": RECONSTRUCTED_EFFECT_ID,
        "programId": RECONSTRUCTED_PROGRAM_ID,
        "programVersion": RECONSTRUCTED_PROGRAM_VERSION,
        "programSha256": RECONSTRUCTED_PROGRAM_SHA256,
        "sidecarSchema": RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_SCHEMA,
        "sidecarFormatVersion": 1,
        "sidecarAuthorityId": RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_AUTHORITY_ID,
        "sidecarDecisionProjectionSha256": (
            RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_DECISION_SHA256
        ),
        "sidecarReceiptSha256": (
            RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_RECEIPT_SHA256
        ),
        "sidecarRawSha256": RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_RAW_SHA256,
        "sidecarByteCount": RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_BYTE_COUNT,
        "sourceExact": False,
        "runtimeExecutionAdmission": False,
        "executeAdmission": False,
        "submitAdmission": False,
        "renderAdmission": False,
        "productAdmission": False,
    }
    for field, expected in expected_scalars.items():
        actual = resource_link[field]
        if type(expected) is int:
            _require_exact_int(actual, expected, f"render-resource link {field}")
        elif type(expected) is bool:
            _require_bool(actual, expected, f"render-resource link {field}")
        elif actual != expected:
            raise ContractError(f"render-resource link {field} mismatch")
    sidecar_text = resource_link["sidecarUtf8Json"]
    if not isinstance(sidecar_text, str):
        raise ContractError("render-resource sidecarUtf8Json must be a string")
    try:
        sidecar_payload = sidecar_text.encode("utf-8")
    except UnicodeEncodeError as exc:
        raise ContractError("render-resource sidecarUtf8Json is not UTF-8") from exc
    _validate_render_resource_sidecar_identity(sidecar_payload)

    receipt = value["renderResourcePublishReceipt"]
    validate_reconstructed_render_resource_publish_receipt(
        receipt, base_entry, resource_link
    )
    outer_receipt_sha = _require_sha(
        value["renderResourcePublishReceiptSha256"],
        "render-resource outer publish receipt SHA",
    )
    if outer_receipt_sha != sha256_bytes(canonical_json_bytes(receipt)):
        raise ContractError("render-resource outer publish receipt SHA mismatch")


def prepare_reconstructed_runtime_entry(
    candidate_path: Path,
    render_resource_authority_path: Path,
) -> dict[str, Any]:
    try:
        candidate_payload = candidate_path.read_bytes()
    except OSError as exc:
        raise ContractError(
            f"cannot read reconstructed runtime candidate {candidate_path}: {exc}"
        ) from exc
    try:
        sidecar_payload = render_resource_authority_path.read_bytes()
    except OSError as exc:
        raise ContractError(
            "cannot read reconstructed render-resource authority "
            f"{render_resource_authority_path}: {exc}"
        ) from exc
    link = _make_reconstructed_link(candidate_payload)
    receipt = _make_reconstructed_publish_receipt(link)
    base_entry = {
        "payloadKind": RECONSTRUCTED_PAYLOAD_KIND,
        "effectAssetId": RECONSTRUCTED_EFFECT_ID,
        "artifactRevision": RECONSTRUCTED_ARTIFACT_REVISION,
        "compilerRevision": RECONSTRUCTED_COMPILER_REVISION,
        "sourceExact": False,
        "runtimeExecutionAdmission": False,
        "productAdmission": False,
        "publishReceiptSha256": sha256_bytes(canonical_json_bytes(receipt)),
        "publishReceipt": receipt,
        "reconstructedRuntimeProgram": link,
    }
    _validate_reconstructed_base_entry(base_entry)
    resource_link = _make_render_resource_link(sidecar_payload)
    resource_receipt = _make_render_resource_publish_receipt(
        base_entry, resource_link
    )
    entry = {
        **base_entry,
        "renderResourcePublishReceiptSha256": sha256_bytes(
            canonical_json_bytes(resource_receipt)
        ),
        "renderResourcePublishReceipt": resource_receipt,
        "reconstructedRenderResourceAuthority": resource_link,
    }
    validate_reconstructed_runtime_entry(entry)
    return entry


def prepare_runtime_entry(
    authoring_path: Path,
    assembly_path: Path,
    artifact_path: Path,
    receipt_path: Path,
) -> dict[str, Any]:
    authoring, _, artifact, receipt = validate_bundle_files(
        authoring_path, assembly_path, artifact_path, receipt_path
    )
    # Deliberately exclude the carrier and Assembly objects.  Their raw fields
    # cannot become runtime semantics through a format-3 catalog entry.
    entry = {
        "payloadKind": SEMANTIC_AUTHORITY,
        "effectAssetId": authoring["effectAssetId"],
        "authoringFormatVersion": AUTHORING_VERSION,
        "runtimeSemanticAuthority": SEMANTIC_AUTHORITY,
        "derivedIdentity": authoring["derivedIdentity"],
        "authoringCarrierSha256": receipt["authoringCarrierSha256"],
        "assemblySha256": receipt["assemblySha256"],
        "compiledArtifactSha256": receipt["compiledArtifactSha256"],
        "compiledReceiptSha256": sha256_file(receipt_path),
        "artifactRevision": artifact["artifactRevision"],
        "compilerRevision": artifact["compilerRevision"],
        "compiledIrSha256": artifact["compiledIrSha256"],
        "compilerReceiptTokenSha256": artifact[
            "compilerReceiptTokenSha256"
        ],
        "executionAdmission": True,
        "productAdmission": False,
        "compiledArtifact": artifact,
        "compiledReceipt": receipt,
    }
    validate_derived_runtime_entry(entry)
    return entry


def validate_derived_runtime_entry(value: dict[str, Any]) -> None:
    keys = (
        "payloadKind",
        "effectAssetId",
        "authoringFormatVersion",
        "runtimeSemanticAuthority",
        "derivedIdentity",
        "authoringCarrierSha256",
        "assemblySha256",
        "compiledArtifactSha256",
        "compiledReceiptSha256",
        "artifactRevision",
        "compilerRevision",
        "compiledIrSha256",
        "compilerReceiptTokenSha256",
        "executionAdmission",
        "productAdmission",
        "compiledArtifact",
        "compiledReceipt",
    )
    _require_exact_keys(value, keys, "derived runtime entry")
    if value["payloadKind"] != SEMANTIC_AUTHORITY:
        raise ContractError("derived runtime payloadKind mismatch")
    effect_id = _require_stable_id(value["effectAssetId"], "runtime effectAssetId")
    _require_exact_int(
        value["authoringFormatVersion"], AUTHORING_VERSION, "runtime authoringFormatVersion"
    )
    if value["runtimeSemanticAuthority"] != SEMANTIC_AUTHORITY:
        raise ContractError("runtime semantic authority mismatch")
    identity = validate_derived_identity(value["derivedIdentity"])
    for field in (
        "authoringCarrierSha256",
        "assemblySha256",
        "compiledArtifactSha256",
        "compiledReceiptSha256",
        "compiledIrSha256",
        "compilerReceiptTokenSha256",
    ):
        _require_sha(value[field], f"runtime {field}")
    artifact_revision = _require_positive_int(value["artifactRevision"], "runtime artifactRevision")
    compiler_revision = _require_stable_id(value["compilerRevision"], "runtime compilerRevision")
    _require_bool(value["executionAdmission"], True, "runtime executionAdmission")
    _require_bool(value["productAdmission"], False, "runtime productAdmission")
    if not isinstance(value["compiledArtifact"], dict) or not isinstance(
        value["compiledReceipt"], dict
    ):
        raise ContractError("runtime compiled payloads must be objects")
    artifact = value["compiledArtifact"]
    receipt = value["compiledReceipt"]
    validate_compiled_artifact(artifact)
    validate_compiled_receipt(receipt)
    if sha256_bytes(pretty_json_bytes(artifact)) != value["compiledArtifactSha256"]:
        raise ContractError("runtime embedded compiled artifact SHA mismatch")
    if sha256_bytes(pretty_json_bytes(receipt)) != value["compiledReceiptSha256"]:
        raise ContractError("runtime embedded compiled receipt SHA mismatch")
    if (
        artifact["effectAssetId"] != effect_id
        or receipt["effectAssetId"] != effect_id
        or artifact["artifactRevision"] != artifact_revision
        or receipt["artifactRevision"] != artifact_revision
        or artifact["compilerRevision"] != compiler_revision
        or receipt["compilerRevision"] != compiler_revision
        or artifact["compiledIrSha256"] != value["compiledIrSha256"]
        or receipt["compiledIrSha256"] != value["compiledIrSha256"]
        or artifact["compilerReceiptTokenSha256"]
        != value["compilerReceiptTokenSha256"]
        or receipt["compilerReceiptTokenSha256"]
        != value["compilerReceiptTokenSha256"]
        or artifact["derivedIdentity"] != identity
        or receipt["derivedIdentity"] != identity
    ):
        raise ContractError("runtime embedded compiled identity mismatch")
    if (
        receipt["authoringCarrierSha256"] != value["authoringCarrierSha256"]
        or receipt["assemblySha256"] != value["assemblySha256"]
        or receipt["compiledArtifactSha256"] != value["compiledArtifactSha256"]
    ):
        raise ContractError("runtime receipt binding mismatch")


def validate_runtime_catalog(value: dict[str, Any]) -> None:
    _require_exact_keys(value, ("schema", "formatVersion", "components", "effects"), "runtime catalog")
    if value["schema"] != RUNTIME_CATALOG_SCHEMA:
        raise ContractError("runtime catalog schema mismatch")
    _require_exact_int(value["formatVersion"], RUNTIME_CATALOG_VERSION, "runtime catalog formatVersion")
    if not isinstance(value["components"], list) or not isinstance(value["effects"], list):
        raise ContractError("runtime catalog arrays are invalid")
    effect_ids: set[str] = set()
    for index, entry in enumerate(value["effects"]):
        if not isinstance(entry, dict):
            raise ContractError(f"runtime effect[{index}] must be an object")
        payload_kind = entry.get("payloadKind")
        effect_id = _require_stable_id(entry.get("effectAssetId"), f"runtime effect[{index}] ID")
        if effect_id in effect_ids:
            raise ContractError(f"duplicate runtime effect ID: {effect_id}")
        effect_ids.add(effect_id)
        if (
            effect_id == RECONSTRUCTED_EFFECT_ID
            and payload_kind != RECONSTRUCTED_PAYLOAD_KIND
        ):
            raise ContractError(
                "reserved Artist 31470 runtime effect must use the reconstructed payload kind"
            )
        if payload_kind == SEMANTIC_AUTHORITY:
            validate_derived_runtime_entry(entry)
        elif payload_kind == RECONSTRUCTED_PAYLOAD_KIND:
            validate_reconstructed_runtime_entry(entry)
        elif payload_kind == LEGACY_PAYLOAD_KIND:
            _require_exact_keys(
                entry,
                (
                    "payloadKind",
                    "effectAssetId",
                    "authoringFormatVersion",
                    "contentSha256",
                    "dependencies",
                    "assembly",
                ),
                "legacy runtime entry",
            )
            version = entry["authoringFormatVersion"]
            if type(version) is not int or version not in range(5, 13):
                raise ContractError("legacy runtime authoring version is invalid")
            _require_sha(entry["contentSha256"], "legacy runtime contentSha256")
            if not isinstance(entry["dependencies"], list) or not isinstance(entry["assembly"], dict):
                raise ContractError("legacy runtime payload is invalid")
        else:
            raise ContractError(f"unsupported runtime payloadKind: {payload_kind}")


def _write_single_json(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + ".tmp")
    try:
        temporary.write_bytes(pretty_json_bytes(value))
        os.replace(temporary, path)
    finally:
        temporary.unlink(missing_ok=True)


def _command_build(args: argparse.Namespace) -> None:
    request = load_json(args.request)
    documents = build_bundle_documents(request, args.input_root)
    write_bundle_transactionally(
        documents,
        {
            "authoring": args.authoring_output,
            "assembly": args.assembly_output,
            "artifact": args.artifact_output,
            "receipt": args.receipt_output,
        },
    )


def _command_prepare(args: argparse.Namespace) -> None:
    entry = prepare_runtime_entry(
        args.authoring, args.assembly, args.artifact, args.receipt
    )
    _write_single_json(args.output, entry)


def _command_prepare_reconstructed(args: argparse.Namespace) -> None:
    entry = prepare_reconstructed_runtime_entry(
        args.candidate, args.render_resource_authority
    )
    _write_single_json(args.output, entry)


def _command_validate_catalog(args: argparse.Namespace) -> None:
    validate_runtime_catalog(load_json(args.catalog, require_lf=True))


def make_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    build = subparsers.add_parser("build", help="build a derived bundle transactionally")
    build.add_argument("--request", type=Path, required=True)
    build.add_argument("--input-root", type=Path, required=True)
    build.add_argument("--authoring-output", type=Path, required=True)
    build.add_argument("--assembly-output", type=Path, required=True)
    build.add_argument("--artifact-output", type=Path, required=True)
    build.add_argument("--receipt-output", type=Path, required=True)
    build.set_defaults(func=_command_build)

    prepare = subparsers.add_parser(
        "prepare-runtime-entry", help="validate a bundle and stage one format-3 entry"
    )
    prepare.add_argument("--authoring", type=Path, required=True)
    prepare.add_argument("--assembly", type=Path, required=True)
    prepare.add_argument("--artifact", type=Path, required=True)
    prepare.add_argument("--receipt", type=Path, required=True)
    prepare.add_argument("--output", type=Path, required=True)
    prepare.set_defaults(func=_command_prepare)

    prepare_reconstructed = subparsers.add_parser(
        "prepare-reconstructed-runtime-entry",
        help="authenticate and embed the frozen Product-false runtime program",
    )
    prepare_reconstructed.add_argument("--candidate", type=Path, required=True)
    prepare_reconstructed.add_argument(
        "--render-resource-authority", type=Path, required=True
    )
    prepare_reconstructed.add_argument("--output", type=Path, required=True)
    prepare_reconstructed.set_defaults(func=_command_prepare_reconstructed)

    validate = subparsers.add_parser(
        "validate-runtime-catalog", help="validate a staged format-3 runtime catalog"
    )
    validate.add_argument("--catalog", type=Path, required=True)
    validate.set_defaults(func=_command_validate_catalog)
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = make_parser()
    args = parser.parse_args(argv)
    try:
        args.func(args)
    except ContractError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
