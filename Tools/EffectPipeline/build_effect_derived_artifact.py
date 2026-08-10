#!/usr/bin/env python3
"""Build and validate immutable Effect derived-artifact bundles.

The v13 authoring document and Assembly produced here are identity-only
carriers.  Runtime semantics live exclusively in the compiled IR embedded in
the compiled artifact.  This module intentionally has no Artist-specific IDs
or corpus counts.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import sys
import tempfile
from typing import Any, Iterable


BUILD_REQUEST_SCHEMA = "lostark.effect-derived-build-request"
DERIVED_IDENTITY_SCHEMA = "lostark.effect-derived-identity"
AUTHORING_SCHEMA = "lostark.effect-authoring"
ASSEMBLY_SCHEMA = "lostark.effect-assembly"
COMPILED_IR_SCHEMA = "lostark.effect-compiled-ir"
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
CODE_ONLY_PUBLICATION_STATE = "CODE_ONLY_NOT_ADMITTED"

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


class ContractError(ValueError):
    """Raised when a derived-artifact contract fails closed."""


def _object_no_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ContractError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def load_json(path: Path) -> dict[str, Any]:
    try:
        payload = path.read_bytes()
        if payload.startswith(b"\xef\xbb\xbf"):
            raise ContractError(f"JSON must be UTF-8 without BOM: {path}")
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
    return result


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
) -> tuple[Path, str]:
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
    return path, expected_canonical


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
        ("artifactRevision", "compilerRevision", "compiledIrSha256"),
        "compiledArtifactIdentity",
    )
    _require_positive_int(compiled["artifactRevision"], "artifactRevision")
    _require_stable_id(compiled["compilerRevision"], "compilerRevision")
    _require_sha(compiled["compiledIrSha256"], "compiledIrSha256")


def validate_compiled_ir(
    value: dict[str, Any], effect_asset_id: str, compiler_revision: str, artifact_revision: int
) -> str:
    if value.get("schema") != COMPILED_IR_SCHEMA:
        raise ContractError("compiled IR schema mismatch")
    _require_exact_int(value.get("formatVersion"), FORMAT_VERSION, "compiled IR formatVersion")
    if value.get("effectAssetId") != effect_asset_id:
        raise ContractError("compiled IR effectAssetId mismatch")
    if value.get("compilerRevision") != compiler_revision:
        raise ContractError("compiled IR compilerRevision mismatch")
    if value.get("artifactRevision") != artifact_revision:
        raise ContractError("compiled IR artifactRevision mismatch")
    if value.get("runtimeSemanticAuthority") != SEMANTIC_AUTHORITY:
        raise ContractError("compiled IR semantic authority mismatch")
    return sha256_bytes(canonical_json_bytes(value))


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
    if not isinstance(value["compiledIr"], dict):
        raise ContractError("compiled artifact compiledIr must be an object")
    actual_ir_sha = validate_compiled_ir(value["compiledIr"], effect_id, compiler, revision)
    if actual_ir_sha != expected_ir_sha:
        raise ContractError("compiled artifact IR hash mismatch")
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
    if artifact_blockers or execution_blockers:
        raise ContractError("derived output blocked before staging")

    inputs = request["inputs"]
    if not isinstance(inputs, dict):
        raise ContractError("build request inputs must be an object")
    _require_exact_keys(
        inputs,
        tuple(name for name, _ in INPUT_TO_IDENTITY) + ("compiledIr",),
        "build request inputs",
    )
    hashes: dict[str, str] = {}
    source_path: Path | None = None
    for input_name, identity_name in INPUT_TO_IDENTITY:
        path, digest = _validate_file_reference(inputs[input_name], input_root, input_name)
        hashes[identity_name] = digest
        if input_name == "sourceContract":
            source_path = path
    assert source_path is not None
    source = load_json(source_path)
    if source.get("schema") != AUTHORING_SCHEMA:
        raise ContractError("source contract schema mismatch")
    _require_exact_int(source.get("version"), SOURCE_CONTRACT_VERSION, "source contract version")

    compiled_path, expected_compiled_sha = _validate_file_reference(
        inputs["compiledIr"], input_root, "compiledIr"
    )
    compiled_ir = load_json(compiled_path)
    actual_canonical_sha = validate_compiled_ir(
        compiled_ir, effect_id, compiler_revision, artifact_revision
    )
    # The request pins canonical compiled semantics, not checkout-dependent JSON whitespace.
    if actual_canonical_sha != expected_compiled_sha:
        raise ContractError("compiledIr.sha256 must pin canonical JSON semantics")

    return {
        "effectAssetId": effect_id,
        "displayName": display_name,
        "compilerRevision": compiler_revision,
        "artifactRevision": artifact_revision,
        "derivedIdentity": _make_derived_identity(hashes),
        "compiledIr": compiled_ir,
        "compiledIrSha256": actual_canonical_sha,
        "artifactBindingBlockerSet": artifact_blockers,
        "executionBlockerSet": execution_blockers,
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
    ):
        raise ContractError("Assembly compiled artifact identity mismatch")
    if (
        receipt["compilerRevision"] != compiler_revision
        or receipt["artifactRevision"] != artifact_revision
        or receipt["compiledIrSha256"] != compiled_ir_sha
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
        if payload_kind == SEMANTIC_AUTHORITY:
            validate_derived_runtime_entry(entry)
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


def _command_validate_catalog(args: argparse.Namespace) -> None:
    validate_runtime_catalog(load_json(args.catalog))


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
