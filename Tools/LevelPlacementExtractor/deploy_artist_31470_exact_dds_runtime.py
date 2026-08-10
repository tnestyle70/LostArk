#!/usr/bin/env python3
"""Transactionally deploy the four frozen Artist 31470 exact DDS fixtures.

The authority is the complete provisioning proposal path from the frozen
Material texture binding receipt.  Source bytes are addressed only by the
exact-DDS receipt's complete exported relative path and are accepted only when
their byte count, raw SHA-256, DDS header, dimensions, and FourCC all match.
"""

from __future__ import annotations

import argparse
import copy
from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import shutil
import stat
import struct
import tempfile
from typing import Any

import artist_31470_exact_dds_runtime_deployment_approval as deployment_approval
import artist_31470_material_texture_runtime_binding_approval as binding_approval
import effect_source_contract_io as strict_io


def _find_repository_root() -> Path:
    for candidate in Path(__file__).resolve().parents:
        if (candidate / "AGENTS.md").is_file() and (candidate / "Data").is_dir():
            return candidate
    raise RuntimeError("cannot locate canonical LostArk repository root")


ROOT = _find_repository_root()
DEFAULT_BINDING_RECEIPT = ROOT / deployment_approval.BINDING_RELATIVE_PATH
DEFAULT_EXACT_DDS_RECEIPT = ROOT / deployment_approval.EXACT_DDS_RELATIVE_PATH
DEFAULT_OUTPUT = (
    ROOT
    / "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.exact-dds-runtime-deployment.receipt.json"
)
DEPLOYER_RELATIVE_PATH = (
    "Tools/LevelPlacementExtractor/deploy_artist_31470_exact_dds_runtime.py"
)
STRICT_IO_RELATIVE_PATH = "Tools/LevelPlacementExtractor/effect_source_contract_io.py"
BINDING_APPROVAL_RELATIVE_PATH = (
    "Tools/LevelPlacementExtractor/"
    "artist_31470_material_texture_runtime_binding_approval.py"
)
DEPLOYMENT_APPROVAL_RELATIVE_PATH = (
    "Tools/LevelPlacementExtractor/"
    "artist_31470_exact_dds_runtime_deployment_approval.py"
)

SCHEMA = "lostark.artist-31470-exact-dds-runtime-deployment-receipt"
FORMAT_VERSION = 1
DEPLOYMENT_STATUS = "COMMITTED_POST_VERIFIED"
R4_BLOCKER = "R4_TEXTURE_SRV_CONSUMER_NOT_COMPLETE"
REPARSE_POINT_ATTRIBUTE = getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0x400)
RECOVERY_DIRECTORY_NAME = "Artist31470ExactDdsRecovery"
RECOVERY_MANIFEST_SCHEMA = (
    "lostark.artist-31470-exact-dds-runtime-deployment-recovery-manifest"
)
TRANSACTION_DIRECTORY_PREFIX = ".artist-31470-exact-dds-deploy-"
RECEIPT_TEMP_PREFIX = ".artist-31470-exact-dds-receipt-"


class ContractError(ValueError):
    """A frozen identity, path, JSON, or runtime byte contract was violated."""


class DeploymentError(RuntimeError):
    """A transactional filesystem operation failed."""


@dataclass(frozen=True)
class ContractData:
    binding_receipt: dict[str, Any]
    exact_dds_receipt: dict[str, Any]
    proposals: tuple[dict[str, Any], ...]


@dataclass(frozen=True)
class TargetBefore:
    status: str
    byte_count: int | None
    raw_sha256: str | None


def build_recovery_manifest(
    contract: ContractData,
    target_before: tuple[TargetBefore, ...],
) -> dict[str, Any]:
    require(len(target_before) == 4, "recovery target pre-state denominator changed")
    backup_identity = {
        "authorityCommit": deployment_approval.AUTHORITY_COMMIT,
        "proposals": [
            {
                "proposalId": proposal["proposalId"],
                "runtimeAssetId": proposal["proposedRuntimeAssetId"],
                "sourceRowSha256": proposal["sourceExactDdsEvidence"]["sourceRowSha256"],
            }
            for proposal in contract.proposals
        ],
        "targetBefore": [
            {
                "status": state.status,
                "byteCount": state.byte_count,
                "rawSha256": state.raw_sha256,
            }
            for state in target_before
        ],
    }
    backup_id = "artist-31470-exact-dds-" + canonical_sha256(backup_identity)[:20]
    rows: list[dict[str, Any]] = []
    for index, (proposal, before) in enumerate(
        zip(contract.proposals, target_before, strict=True)
    ):
        backup_relative_path = (
            f"files/{index}.dds" if before.status == "PRESENT_EXACT_EQUAL" else None
        )
        row = {
            "runtimeAssetId": proposal["proposedRuntimeAssetId"],
            "targetBefore": {
                "status": before.status,
                "byteCount": before.byte_count,
                "rawSha256": before.raw_sha256,
            },
            "backupFileRelativePath": backup_relative_path,
            "recoveryAction": (
                "RESTORE_EXACT_BACKUP_FILE"
                if backup_relative_path is not None
                else "REMOVE_DEPLOYED_FILE"
            ),
        }
        rows.append(row_with_digest(row))
    manifest = {
        "schema": RECOVERY_MANIFEST_SCHEMA,
        "formatVersion": 1,
        "backupId": backup_id,
        "authorityCommit": deployment_approval.AUTHORITY_COMMIT,
        "runtimeRootRole": "Client/Bin/Resources",
        "assets": rows,
        "summary": {
            "targetCount": 4,
            "backupPayloadFileCount": sum(
                state.status == "PRESENT_EXACT_EQUAL" for state in target_before
            ),
            "absentTargetMarkerCount": sum(
                state.status == "ABSENT" for state in target_before
            ),
        },
    }
    manifest["manifestSha256"] = canonical_sha256(manifest)
    return manifest


def recovery_receipt_projection(manifest: dict[str, Any]) -> dict[str, Any]:
    relative_directory = f"{RECOVERY_DIRECTORY_NAME}/{manifest['backupId']}"
    return {
        "anchor": "RUNTIME_RESOURCE_ROOT_PARENT",
        "relativeDirectory": relative_directory,
        "manifestRelativePath": f"{relative_directory}/rollback-manifest.json",
        "manifestCanonicalSelfSha256": manifest["manifestSha256"],
        "manifestRawSha256": hashlib.sha256(
            serialized_json_bytes(manifest)
        ).hexdigest(),
        "backupPayloadFileCount": manifest["summary"]["backupPayloadFileCount"],
        "absentTargetMarkerCount": manifest["summary"]["absentTargetMarkerCount"],
        "preservedAfterCommit": True,
    }


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ContractError(message)


def strict_equal(actual: Any, expected: Any) -> bool:
    if type(actual) is not type(expected):
        return False
    if isinstance(expected, dict):
        return actual.keys() == expected.keys() and all(
            strict_equal(actual[key], expected[key]) for key in expected
        )
    if isinstance(expected, list):
        return len(actual) == len(expected) and all(
            strict_equal(left, right)
            for left, right in zip(actual, expected, strict=True)
        )
    return actual == expected


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


def serialized_json_bytes(value: dict[str, Any]) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    ).encode("utf-8")


def _reject_duplicate_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ContractError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def _reject_non_finite(token: str) -> None:
    raise ContractError(f"non-finite JSON number is forbidden: {token}")


def read_strict_json(path: Path) -> dict[str, Any]:
    payload = path.read_bytes()
    require(not payload.startswith(b"\xef\xbb\xbf"), f"JSON BOM is forbidden: {path}")
    try:
        value = json.loads(
            payload.decode("utf-8"),
            object_pairs_hook=_reject_duplicate_keys,
            parse_constant=_reject_non_finite,
        )
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ContractError(f"cannot parse strict JSON {path}: {exc}") from exc
    require(isinstance(value, dict), f"JSON root must be an object: {path}")
    return value


def require_exact_keys(value: Any, expected: tuple[str, ...], label: str) -> None:
    require(isinstance(value, dict), f"{label} must be an object")
    require(tuple(value.keys()) == expected, f"{label} keys/order changed")


def require_sha256(value: Any, label: str) -> str:
    require(
        isinstance(value, str)
        and len(value) == 64
        and value == value.casefold()
        and all(character in "0123456789abcdef" for character in value),
        f"{label} must be lowercase SHA-256",
    )
    return value


def row_without_digest(row: dict[str, Any]) -> dict[str, Any]:
    candidate = copy.deepcopy(row)
    candidate.pop("rowSha256", None)
    return candidate


def row_with_digest(row: dict[str, Any]) -> dict[str, Any]:
    candidate = copy.deepcopy(row)
    candidate["rowSha256"] = canonical_sha256(candidate)
    return candidate


def validate_row_digest(row: dict[str, Any], label: str) -> None:
    digest = require_sha256(row.get("rowSha256"), f"{label}.rowSha256")
    require(digest == canonical_sha256(row_without_digest(row)), f"{label} row digest mismatch")


def validate_receipt_self(receipt: dict[str, Any], field: str) -> None:
    digest = require_sha256(receipt.get(field), field)
    candidate = copy.deepcopy(receipt)
    candidate.pop(field)
    require(digest == canonical_sha256(candidate), f"{field} mismatch")


def absolute_without_resolving(path: Path) -> Path:
    return Path(os.path.abspath(os.fspath(path)))


def windows_path_identity(path: Path) -> tuple[str, ...]:
    """Return a lexical absolute Windows identity without following links."""

    absolute = absolute_without_resolving(path)
    return tuple(part.replace("/", "\\").casefold() for part in absolute.parts)


def windows_path_is_within(candidate: Path, directory: Path) -> bool:
    candidate_identity = windows_path_identity(candidate)
    directory_identity = windows_path_identity(directory)
    return candidate_identity[: len(directory_identity)] == directory_identity


def require_windows_paths_disjoint(left: Path, right: Path, label: str) -> None:
    left_identity = windows_path_identity(left)
    right_identity = windows_path_identity(right)
    require(
        left_identity != right_identity,
        f"Windows case-insensitive path identity collision: {label}",
    )


def require_output_path_disjoint_from_runtime_transaction(
    contract: ContractData,
    runtime_root: Path,
    output_path: Path,
) -> None:
    """Fail before writes if the receipt can alias any runtime transaction path."""

    runtime_root = absolute_without_resolving(runtime_root)
    output_path = absolute_without_resolving(output_path)
    for index, proposal in enumerate(contract.proposals):
        runtime_asset_id = validate_runtime_asset_id(
            proposal["proposedRuntimeAssetId"]
        )
        target = runtime_root.joinpath(*PurePosixPath(runtime_asset_id).parts)
        require_windows_paths_disjoint(
            output_path,
            target,
            f"deployment receipt/runtime target {index}",
        )

    runtime_transaction_root = runtime_root.parent
    require(
        not windows_path_is_within(output_path, runtime_transaction_root)
        and not windows_path_is_within(runtime_transaction_root, output_path),
        "deployment receipt overlaps the runtime target/backup/stage/temp namespace",
    )


def validate_source_relative_path(value: Any) -> str:
    require(isinstance(value, str) and value == value.strip(), "source DDS path is invalid")
    require("\\" not in value and ":" not in value, f"unsafe source DDS path: {value}")
    path = PurePosixPath(value)
    require(not path.is_absolute(), f"absolute source DDS path is forbidden: {value}")
    require(".." not in path.parts and "." not in path.parts, f"traversal source DDS path: {value}")
    require(value == "/".join(path.parts), f"source DDS path is not canonical POSIX: {value}")
    require(
        len(path.parts) == 3
        and path.parts[1] == "Texture2D"
        and path.suffix == ".dds",
        f"source DDS path shape changed: {value}",
    )
    return value


def validate_runtime_asset_id(value: Any) -> str:
    require(isinstance(value, str) and value == value.strip(), "runtime asset ID is invalid")
    require("\\" not in value and ":" not in value, f"unsafe runtime asset ID: {value}")
    path = PurePosixPath(value)
    require(not path.is_absolute(), f"absolute runtime asset ID is forbidden: {value}")
    require(".." not in path.parts and "." not in path.parts, f"runtime asset traversal: {value}")
    require(value == "/".join(path.parts), f"runtime asset ID is not canonical POSIX: {value}")
    require(
        len(path.parts) == 5
        and path.parts[:3] == ("Effect", "Artist", "Textures")
        and path.parts[3].startswith("FX_TEX_")
        and path.suffix == ".dds",
        f"runtime asset ID is outside Artist texture scope: {value}",
    )
    return value


def exact_dds_projection(row: dict[str, Any]) -> dict[str, Any]:
    texture = row["sourceTexture2D"]
    dds = row["dds"]
    return {
        "logicalObjectPath": row["logicalObjectPath"],
        "fixtureAssetId": row["fixtureAssetId"],
        "copyPolicy": row["copyPolicy"],
        "sourceExtractedDdsRelativePath": row["sourceExtractedDdsRelativePath"],
        "sourceTexture2D": {
            "logicalPackage": texture["logicalPackage"],
            "physicalPackage": texture["physicalPackage"],
            "physicalPackageByteCount": texture["physicalPackageByteCount"],
            "physicalPackageSha256": texture["physicalPackageSha256"],
            "exportIndex": texture["exportIndex"],
            "packageReference": texture["packageReference"],
            "serialOffset": texture["serialOffset"],
            "serialSize": texture["serialSize"],
            "serialSha256": texture["serialSha256"],
        },
        "dds": {
            "byteCount": dds["byteCount"],
            "rawSha256": dds["sha256"],
            "header128Sha256": dds["header128Sha256"],
            "width": dds["width"],
            "height": dds["height"],
            "fourCC": dds["fourCC"],
        },
        "sourceRowSha256": canonical_sha256(row),
    }


def validate_exact_dds_evidence(evidence: dict[str, Any], label: str) -> None:
    require_exact_keys(
        evidence,
        (
            "logicalObjectPath",
            "fixtureAssetId",
            "copyPolicy",
            "sourceExtractedDdsRelativePath",
            "sourceTexture2D",
            "dds",
            "sourceRowSha256",
        ),
        label,
    )
    validate_runtime_asset_id(evidence["fixtureAssetId"])
    validate_source_relative_path(evidence["sourceExtractedDdsRelativePath"])
    require_sha256(evidence["sourceRowSha256"], f"{label}.sourceRowSha256")
    require_exact_keys(
        evidence["sourceTexture2D"],
        (
            "logicalPackage",
            "physicalPackage",
            "physicalPackageByteCount",
            "physicalPackageSha256",
            "exportIndex",
            "packageReference",
            "serialOffset",
            "serialSize",
            "serialSha256",
        ),
        f"{label}.sourceTexture2D",
    )
    texture = evidence["sourceTexture2D"]
    require(isinstance(texture["logicalPackage"], str), f"{label} logical package invalid")
    require(isinstance(texture["physicalPackage"], str), f"{label} physical package invalid")
    for field in (
        "physicalPackageByteCount",
        "exportIndex",
        "packageReference",
        "serialOffset",
        "serialSize",
    ):
        require(type(texture[field]) is int and texture[field] > 0, f"{label}.{field} invalid")
    require_sha256(texture["physicalPackageSha256"], f"{label}.physicalPackageSha256")
    require_sha256(texture["serialSha256"], f"{label}.serialSha256")
    require_exact_keys(
        evidence["dds"],
        ("byteCount", "rawSha256", "header128Sha256", "width", "height", "fourCC"),
        f"{label}.dds",
    )
    dds = evidence["dds"]
    for field in ("byteCount", "width", "height"):
        require(type(dds[field]) is int and dds[field] > 0, f"{label}.dds.{field} invalid")
    require_sha256(dds["rawSha256"], f"{label}.dds.rawSha256")
    require_sha256(dds["header128Sha256"], f"{label}.dds.header128Sha256")
    require(dds["fourCC"] in ("DXT1", "DXT5"), f"{label}.dds.fourCC invalid")


def load_contract(
    binding_path: Path = DEFAULT_BINDING_RECEIPT,
    exact_dds_path: Path = DEFAULT_EXACT_DDS_RECEIPT,
) -> ContractData:
    binding_path = absolute_without_resolving(binding_path)
    exact_dds_path = absolute_without_resolving(exact_dds_path)
    require(binding_path.is_file(), f"binding receipt is missing: {binding_path}")
    require(exact_dds_path.is_file(), f"exact-DDS receipt is missing: {exact_dds_path}")
    require(
        strict_io.tracked_text_sha256(binding_path)
        == deployment_approval.BINDING_TRACKED_TEXT_SHA256,
        "binding receipt tracked bytes changed",
    )
    require(
        strict_io.tracked_text_sha256(exact_dds_path)
        == deployment_approval.EXACT_DDS_TRACKED_TEXT_SHA256,
        "exact-DDS receipt tracked bytes changed",
    )

    binding = read_strict_json(binding_path)
    exact_dds = read_strict_json(exact_dds_path)
    require(binding.get("schema") == "lostark.artist-31470-material-texture-runtime-binding-receipt", "binding schema changed")
    require(type(binding.get("formatVersion")) is int and binding["formatVersion"] == 1, "binding version changed")
    validate_receipt_self(binding, "receiptSha256")
    require(
        binding["receiptSha256"] == deployment_approval.BINDING_RECEIPT_SHA256,
        "binding receipt self identity changed",
    )
    binding_approval.require_approved_receipt(binding)
    require(exact_dds.get("schema") == "lostark.artist-effect-exact-dds-recovery-receipt", "exact-DDS schema changed")
    require(type(exact_dds.get("formatVersion")) is int and exact_dds["formatVersion"] == 1, "exact-DDS version changed")
    require(exact_dds.get("status") == "SOURCE_EXTRACTED", "exact-DDS source status changed")
    require(exact_dds.get("productPlacementStatus") == "BLOCKED_NOT_PUBLISHED", "exact-DDS Product boundary changed")

    exact_rows: dict[str, dict[str, Any]] = {}
    for index, row in enumerate(exact_dds.get("assets", [])):
        logical = row.get("logicalObjectPath")
        require(isinstance(logical, str) and logical not in exact_rows, f"duplicate exact-DDS asset at {index}")
        exact_rows[logical] = row

    proposals = binding.get("provisioningProposals")
    require(isinstance(proposals, list) and len(proposals) == 4, "provisioning proposal denominator changed")
    require(
        tuple(row.get("logicalTexturePath") for row in proposals)
        == deployment_approval.EXPECTED_LOGICAL_TEXTURE_PATHS,
        "provisioning logical path order changed",
    )
    require(
        tuple(row.get("proposedRuntimeAssetId") for row in proposals)
        == deployment_approval.EXPECTED_RUNTIME_ASSET_IDS,
        "provisioning runtime asset order changed",
    )
    for index, proposal in enumerate(proposals):
        label = f"provisioningProposals[{index}]"
        require_exact_keys(
            proposal,
            (
                "proposalId",
                "textureResourceId",
                "logicalTexturePath",
                "policy",
                "proposedRuntimeAssetId",
                "sourceExactDdsEvidence",
                "deploymentStatus",
                "requiredReceipt",
                "sourceExact",
                "runtimeAssetAdmission",
                "productAdmission",
                "rowSha256",
            ),
            label,
        )
        validate_row_digest(proposal, label)
        require(proposal["policy"] == deployment_approval.DEPLOYMENT_POLICY, f"{label} policy changed")
        runtime_asset_id = validate_runtime_asset_id(proposal["proposedRuntimeAssetId"])
        require(
            proposal["sourceExactDdsEvidence"]["fixtureAssetId"] == runtime_asset_id,
            f"{label} fixture target mismatch",
        )
        require(
            proposal["deploymentStatus"] == "PROPOSED_TRANSACTIONAL_DEPLOYMENT_NOT_VERIFIED"
            and proposal["requiredReceipt"]
            == "TRANSACTIONAL_RUNTIME_RESOURCE_DEPLOYMENT_AND_VERIFICATION_RECEIPT",
            f"{label} pre-deployment status changed",
        )
        require(
            proposal["sourceExact"] is False
            and proposal["runtimeAssetAdmission"] is False
            and proposal["productAdmission"] is False,
            f"{label} pre-deployment admission changed",
        )
        logical = proposal["logicalTexturePath"]
        exact_row = exact_rows.get(logical)
        require(exact_row is not None, f"{label} exact-DDS asset missing")
        expected_projection = exact_dds_projection(exact_row)
        validate_exact_dds_evidence(proposal["sourceExactDdsEvidence"], f"{label}.sourceExactDdsEvidence")
        require(
            strict_equal(proposal["sourceExactDdsEvidence"], expected_projection),
            f"{label} exact-DDS projection changed",
        )

    require(set(exact_rows) == set(deployment_approval.EXPECTED_LOGICAL_TEXTURE_PATHS), "exact-DDS asset denominator changed")
    return ContractData(binding, exact_dds, tuple(copy.deepcopy(proposals)))


def _has_reparse_point(path: Path) -> bool:
    try:
        metadata = os.lstat(path)
    except FileNotFoundError:
        return False
    attributes = getattr(metadata, "st_file_attributes", 0)
    return path.is_symlink() or bool(attributes & REPARSE_POINT_ATTRIBUTE)


def _require_safe_existing_path(path: Path, label: str) -> None:
    require(path.exists(), f"{label} is missing: {path}")
    require(not _has_reparse_point(path), f"{label} is a symlink/reparse point: {path}")


def resolve_case_exact_file(root: Path, relative_path: str, label: str) -> Path:
    root = absolute_without_resolving(root)
    _require_safe_existing_path(root, f"{label} root")
    require(root.is_dir(), f"{label} root is not a directory: {root}")
    current = root
    for part in PurePosixPath(relative_path).parts:
        matches = [entry for entry in current.iterdir() if entry.name.casefold() == part.casefold()]
        require(len(matches) == 1, f"{label} path segment missing or casefold-colliding: {part}")
        child = matches[0]
        require(child.name == part, f"{label} path case changed: expected {part}, got {child.name}")
        require(not _has_reparse_point(child), f"{label} path contains symlink/reparse point: {child}")
        current = child
    require(current.is_file(), f"{label} is not a regular file: {current}")
    return current


def inspect_target_case(runtime_root: Path, runtime_asset_id: str) -> tuple[Path, bool]:
    runtime_root = absolute_without_resolving(runtime_root)
    require(runtime_root.name == "Resources", "runtime root must be the Client/Bin/Resources directory")
    _require_safe_existing_path(runtime_root.parent, "runtime root parent")
    require(runtime_root.parent.is_dir(), "runtime root parent is not a directory")
    root_matches = [
        entry
        for entry in runtime_root.parent.iterdir()
        if entry.name.casefold() == runtime_root.name.casefold()
    ]
    require(len(root_matches) <= 1, "runtime root has a casefold collision")
    if root_matches:
        require(root_matches[0].name == "Resources", "runtime root path case changed")
        current = root_matches[0]
        require(current.is_dir(), "runtime root is not a directory")
        require(not _has_reparse_point(current), "runtime root is a symlink/reparse point")
    else:
        return runtime_root.joinpath(*PurePosixPath(runtime_asset_id).parts), False

    parts = PurePosixPath(runtime_asset_id).parts
    for index, part in enumerate(parts):
        matches = [entry for entry in current.iterdir() if entry.name.casefold() == part.casefold()]
        require(len(matches) <= 1, f"runtime target casefold collision at {current}/{part}")
        if not matches:
            return runtime_root.joinpath(*parts), False
        child = matches[0]
        require(child.name == part, f"runtime target path case changed: expected {part}, got {child.name}")
        require(not _has_reparse_point(child), f"runtime target path contains symlink/reparse point: {child}")
        if index != len(parts) - 1:
            require(child.is_dir(), f"runtime target parent is not a directory: {child}")
        current = child
    require(current.is_file(), f"runtime target is not a regular file: {current}")
    return current, True


def raw_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        while chunk := stream.read(1024 * 1024):
            digest.update(chunk)
    return digest.hexdigest()


def verify_dds_file(path: Path, evidence: dict[str, Any], label: str) -> None:
    dds = evidence["dds"]
    require(path.stat().st_size == dds["byteCount"], f"{label} byte count mismatch")
    require(raw_sha256(path) == dds["rawSha256"], f"{label} raw SHA mismatch")
    with path.open("rb") as stream:
        header = stream.read(128)
    require(len(header) == 128 and header[:4] == b"DDS ", f"{label} DDS magic/header invalid")
    require(hashlib.sha256(header).hexdigest() == dds["header128Sha256"], f"{label} header SHA mismatch")
    require(struct.unpack_from("<I", header, 4)[0] == 124, f"{label} DDS header size changed")
    require(struct.unpack_from("<I", header, 12)[0] == dds["height"], f"{label} DDS height changed")
    require(struct.unpack_from("<I", header, 16)[0] == dds["width"], f"{label} DDS width changed")
    four_cc = header[84:88].decode("ascii")
    require(four_cc == dds["fourCC"], f"{label} DDS FourCC changed")


def implementation_evidence() -> dict[str, Any]:
    loaded_modules = {
        "EXACT_DDS_RUNTIME_DEPLOYER": Path(__file__),
        "STRICT_SOURCE_CONTRACT_IO": Path(strict_io.__file__),
        "MATERIAL_TEXTURE_BINDING_APPROVAL": Path(binding_approval.__file__),
        "EXACT_DDS_RUNTIME_DEPLOYMENT_APPROVAL": Path(deployment_approval.__file__),
    }
    rows = []
    for dependency_id, relative_path in (
        ("EXACT_DDS_RUNTIME_DEPLOYER", DEPLOYER_RELATIVE_PATH),
        ("STRICT_SOURCE_CONTRACT_IO", STRICT_IO_RELATIVE_PATH),
        ("MATERIAL_TEXTURE_BINDING_APPROVAL", BINDING_APPROVAL_RELATIVE_PATH),
        ("EXACT_DDS_RUNTIME_DEPLOYMENT_APPROVAL", DEPLOYMENT_APPROVAL_RELATIVE_PATH),
    ):
        path = ROOT / relative_path
        require(
            loaded_modules[dependency_id].resolve() == path.resolve(),
            f"loaded dependency path changed: {dependency_id}",
        )
        rows.append(
            {
                "dependencyId": dependency_id,
                "relativePath": relative_path,
                "trackedTextSha256": strict_io.tracked_text_sha256(path),
            }
        )
    return {
        "dependencyCount": len(rows),
        "dependencies": rows,
        "projectionSha256": canonical_sha256(rows),
    }


def source_evidence(contract: ContractData) -> dict[str, Any]:
    return {
        "authorityCommit": deployment_approval.AUTHORITY_COMMIT,
        "authorityTree": deployment_approval.AUTHORITY_TREE,
        "materialTextureBindingReceipt": {
            "relativePath": deployment_approval.BINDING_RELATIVE_PATH,
            "gitBlob": deployment_approval.BINDING_GIT_BLOB,
            "trackedTextSha256": deployment_approval.BINDING_TRACKED_TEXT_SHA256,
            "receiptSha256": deployment_approval.BINDING_RECEIPT_SHA256,
        },
        "exactDdsRecoveryReceipt": {
            "relativePath": deployment_approval.EXACT_DDS_RELATIVE_PATH,
            "gitBlob": deployment_approval.EXACT_DDS_GIT_BLOB,
            "trackedTextSha256": deployment_approval.EXACT_DDS_TRACKED_TEXT_SHA256,
            "schema": contract.exact_dds_receipt["schema"],
            "status": contract.exact_dds_receipt["status"],
        },
        "provisioningProposalProjectionSha256": canonical_sha256(list(contract.proposals)),
        "implementationEvidence": implementation_evidence(),
    }


def build_receipt(
    contract: ContractData,
    target_before: tuple[TargetBefore, ...],
) -> dict[str, Any]:
    require(len(target_before) == 4, "target pre-state denominator changed")
    recovery_manifest = build_recovery_manifest(contract, target_before)
    assets: list[dict[str, Any]] = []
    for proposal, before in zip(contract.proposals, target_before, strict=True):
        evidence = proposal["sourceExactDdsEvidence"]
        identity = {
            "proposalId": proposal["proposalId"],
            "logicalTexturePath": proposal["logicalTexturePath"],
            "runtimeAssetId": proposal["proposedRuntimeAssetId"],
            "sourceRowSha256": evidence["sourceRowSha256"],
        }
        row = {
            "deploymentRowId": "exact-dds-runtime-deployment-" + canonical_sha256(identity)[:20],
            "proposalId": proposal["proposalId"],
            "textureResourceId": proposal["textureResourceId"],
            "logicalTexturePath": proposal["logicalTexturePath"],
            "runtimeAssetId": proposal["proposedRuntimeAssetId"],
            "policy": deployment_approval.DEPLOYMENT_POLICY,
            "sourceExactDdsEvidence": copy.deepcopy(evidence),
            "targetBefore": {
                "status": before.status,
                "byteCount": before.byte_count,
                "rawSha256": before.raw_sha256,
            },
            "deployedFile": {
                "runtimeAssetId": proposal["proposedRuntimeAssetId"],
                "byteCount": evidence["dds"]["byteCount"],
                "rawSha256": evidence["dds"]["rawSha256"],
                "pathCaseVerified": True,
                "regularFileVerified": True,
                "symlinkFreeVerified": True,
                "postVerified": True,
            },
            "deploymentStatus": DEPLOYMENT_STATUS,
            "sourceExactMaterialClaim": False,
            "runtimeAssetDeploymentAdmission": True,
            "rendererConsumerAdmission": False,
            "productAdmission": False,
            "blockers": [R4_BLOCKER],
        }
        assets.append(row_with_digest(row))

    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "deploymentContract": {
            "policy": deployment_approval.DEPLOYMENT_POLICY,
            "algorithm": "FOUR_FILE_PARSE_VALIDATE_STAGE_BACKUP_COMMIT_POSTVERIFY_ROLLBACK_V1",
            "sourcePathAuthority": "EXACT_DDS_SOURCE_EXTRACTED_RELATIVE_PATH",
            "targetPathAuthority": "MATERIAL_TEXTURE_BINDING_PROPOSAL_FIXTURE_ASSET_ID",
            "runtimeRootRole": "Client/Bin/Resources",
            "copyPolicy": "BYTE_FOR_BYTE_NO_REENCODE",
            "existingTargetPolicy": "EXACT_EQUAL_BACKUP_OR_FAIL_CLOSED",
            "pathCasePolicy": "EXACT_CASE_NO_CASEFOLD_COLLISION",
            "symlinkPolicy": "FORBIDDEN",
            "rollbackScope": "ALL_FOUR_RUNTIME_TARGETS_AND_RECEIPT",
        },
        "sourceEvidence": source_evidence(contract),
        "assets": assets,
        "recoveryBackup": recovery_receipt_projection(recovery_manifest),
        "admission": {
            "transactionCommitted": True,
            "allFourRuntimeAssetsPostVerified": True,
            "sourceExactMaterialClaim": False,
            "rendererTextureSrvConsumerComplete": False,
            "r4Complete": False,
            "productReady": False,
            "blockers": [R4_BLOCKER],
        },
        "summary": {
            "requestedAssetCount": 4,
            "deployedAssetCount": 4,
            "postVerifiedAssetCount": 4,
            "runtimeAssetDeploymentAdmittedCount": 4,
            "recoveryBackupPayloadFileCount": recovery_manifest["summary"][
                "backupPayloadFileCount"
            ],
            "recoveryAbsentTargetMarkerCount": recovery_manifest["summary"][
                "absentTargetMarkerCount"
            ],
            "sourceExactMaterialClaimCount": 0,
            "rendererConsumerReadyCount": 0,
            "productReadyCount": 0,
        },
    }
    receipt["receiptSha256"] = canonical_sha256(receipt)
    return receipt


def validate_receipt(
    contract: ContractData,
    receipt: dict[str, Any],
    *,
    require_approval: bool = True,
) -> None:
    require_exact_keys(
        receipt,
        (
            "schema",
            "formatVersion",
            "characterClass",
            "skillId",
            "inputSlot",
            "deploymentContract",
            "sourceEvidence",
            "assets",
            "recoveryBackup",
            "admission",
            "summary",
            "receiptSha256",
        ),
        "deployment receipt",
    )
    require(receipt["schema"] == SCHEMA, "deployment receipt schema changed")
    require(type(receipt["formatVersion"]) is int and receipt["formatVersion"] == FORMAT_VERSION, "deployment receipt version changed")
    validate_receipt_self(receipt, "receiptSha256")
    rows = receipt.get("assets")
    require(isinstance(rows, list) and len(rows) == 4, "deployment receipt asset denominator changed")
    states: list[TargetBefore] = []
    for index, row in enumerate(rows):
        label = f"assets[{index}]"
        require_exact_keys(
            row,
            (
                "deploymentRowId",
                "proposalId",
                "textureResourceId",
                "logicalTexturePath",
                "runtimeAssetId",
                "policy",
                "sourceExactDdsEvidence",
                "targetBefore",
                "deployedFile",
                "deploymentStatus",
                "sourceExactMaterialClaim",
                "runtimeAssetDeploymentAdmission",
                "rendererConsumerAdmission",
                "productAdmission",
                "blockers",
                "rowSha256",
            ),
            label,
        )
        validate_row_digest(row, label)
        before = row["targetBefore"]
        require_exact_keys(before, ("status", "byteCount", "rawSha256"), f"{label}.targetBefore")
        expected_dds = contract.proposals[index]["sourceExactDdsEvidence"]["dds"]
        if before["status"] == "ABSENT":
            require(before["byteCount"] is None and before["rawSha256"] is None, f"{label} absent pre-state changed")
        elif before["status"] == "PRESENT_EXACT_EQUAL":
            require(type(before["byteCount"]) is int and before["byteCount"] == expected_dds["byteCount"], f"{label} pre-state byte count changed")
            require(before["rawSha256"] == expected_dds["rawSha256"], f"{label} pre-state SHA changed")
        else:
            raise ContractError(f"{label} unapproved target pre-state")
        states.append(TargetBefore(before["status"], before["byteCount"], before["rawSha256"]))

    expected = build_receipt(contract, tuple(states))
    require(strict_equal(receipt, expected), "deployment receipt does not rebuild from frozen inputs")
    if require_approval:
        deployment_approval.require_approved_receipt(receipt)


def receipt_target_states(receipt: dict[str, Any]) -> tuple[TargetBefore, ...]:
    return tuple(
        TargetBefore(
            row["targetBefore"]["status"],
            row["targetBefore"]["byteCount"],
            row["targetBefore"]["rawSha256"],
        )
        for row in receipt["assets"]
    )


def validate_recovery_manifest(
    contract: ContractData,
    target_before: tuple[TargetBefore, ...],
    manifest: dict[str, Any],
) -> None:
    require_exact_keys(
        manifest,
        (
            "schema",
            "formatVersion",
            "backupId",
            "authorityCommit",
            "runtimeRootRole",
            "assets",
            "summary",
            "manifestSha256",
        ),
        "recovery manifest",
    )
    require(manifest["schema"] == RECOVERY_MANIFEST_SCHEMA, "recovery manifest schema changed")
    require(
        type(manifest["formatVersion"]) is int and manifest["formatVersion"] == 1,
        "recovery manifest version changed",
    )
    validate_receipt_self(manifest, "manifestSha256")
    rows = manifest.get("assets")
    require(isinstance(rows, list) and len(rows) == 4, "recovery manifest denominator changed")
    for index, row in enumerate(rows):
        require_exact_keys(
            row,
            (
                "runtimeAssetId",
                "targetBefore",
                "backupFileRelativePath",
                "recoveryAction",
                "rowSha256",
            ),
            f"recovery assets[{index}]",
        )
        validate_row_digest(row, f"recovery assets[{index}]")
    expected = build_recovery_manifest(contract, target_before)
    require(strict_equal(manifest, expected), "recovery manifest does not rebuild from frozen inputs")


def recovery_backup_directory(receipt: dict[str, Any], runtime_root: Path) -> Path:
    backup = receipt["recoveryBackup"]
    require_exact_keys(
        backup,
        (
            "anchor",
            "relativeDirectory",
            "manifestRelativePath",
            "manifestCanonicalSelfSha256",
            "manifestRawSha256",
            "backupPayloadFileCount",
            "absentTargetMarkerCount",
            "preservedAfterCommit",
        ),
        "recoveryBackup",
    )
    require(backup["anchor"] == "RUNTIME_RESOURCE_ROOT_PARENT", "recovery backup anchor changed")
    relative = PurePosixPath(backup["relativeDirectory"])
    require(
        len(relative.parts) == 2
        and relative.parts[0] == RECOVERY_DIRECTORY_NAME
        and relative.parts[1].startswith("artist-31470-exact-dds-")
        and ".." not in relative.parts,
        "recovery backup relative directory is unsafe",
    )
    require(
        backup["manifestRelativePath"]
        == f"{backup['relativeDirectory']}/rollback-manifest.json",
        "recovery backup manifest path changed",
    )
    return absolute_without_resolving(runtime_root).parent.joinpath(*relative.parts)


def verify_recovery_backup(
    contract: ContractData,
    receipt: dict[str, Any],
    runtime_root: Path,
) -> None:
    runtime_root = absolute_without_resolving(runtime_root)
    backup = receipt["recoveryBackup"]
    manifest_path = resolve_case_exact_file(
        runtime_root.parent,
        backup["manifestRelativePath"],
        "recovery backup manifest",
    )
    manifest = read_strict_json(manifest_path)
    states = receipt_target_states(receipt)
    validate_recovery_manifest(contract, states, manifest)
    require(
        manifest["manifestSha256"] == backup["manifestCanonicalSelfSha256"],
        "recovery backup manifest canonical self identity changed",
    )
    require(
        raw_sha256(manifest_path) == backup["manifestRawSha256"],
        "recovery backup manifest raw file identity changed",
    )
    backup_directory = recovery_backup_directory(receipt, runtime_root)
    for index, (proposal, state) in enumerate(
        zip(contract.proposals, states, strict=True)
    ):
        relative_backup = manifest["assets"][index]["backupFileRelativePath"]
        if state.status == "ABSENT":
            require(relative_backup is None, f"unexpected recovery payload for absent target {index}")
            continue
        backup_file = resolve_case_exact_file(
            backup_directory,
            relative_backup,
            f"recovery backup payload {index}",
        )
        verify_dds_file(
            backup_file,
            proposal["sourceExactDdsEvidence"],
            f"recovery backup payload {index}",
        )


def stage_recovery_backup(
    contract: ContractData,
    target_before: tuple[TargetBefore, ...],
    targets: tuple[Path, ...],
    stage_directory: Path,
) -> dict[str, Any]:
    manifest = build_recovery_manifest(contract, target_before)
    stage_directory.mkdir()
    files_directory = stage_directory / "files"
    files_directory.mkdir()
    for index, (proposal, before, target) in enumerate(
        zip(contract.proposals, target_before, targets, strict=True)
    ):
        if before.status != "PRESENT_EXACT_EQUAL":
            continue
        backup_file = files_directory / f"{index}.dds"
        shutil.copyfile(target, backup_file)
        verify_dds_file(
            backup_file,
            proposal["sourceExactDdsEvidence"],
            f"staged recovery backup payload {index}",
        )
    manifest_path = stage_directory / "rollback-manifest.json"
    manifest_path.write_bytes(serialized_json_bytes(manifest))
    validate_recovery_manifest(
        contract,
        target_before,
        read_strict_json(manifest_path),
    )
    return manifest


def inspect_source_files(contract: ContractData, source_root: Path) -> tuple[Path, ...]:
    files: list[Path] = []
    for index, proposal in enumerate(contract.proposals):
        evidence = proposal["sourceExactDdsEvidence"]
        relative_path = validate_source_relative_path(evidence["sourceExtractedDdsRelativePath"])
        source_file = resolve_case_exact_file(source_root, relative_path, f"source DDS {index}")
        verify_dds_file(source_file, evidence, f"source DDS {index}")
        files.append(source_file)
    return tuple(files)


def inspect_target_files(
    contract: ContractData,
    runtime_root: Path,
) -> tuple[tuple[Path, ...], tuple[TargetBefore, ...]]:
    targets: list[Path] = []
    states: list[TargetBefore] = []
    folded_targets: set[str] = set()
    for index, proposal in enumerate(contract.proposals):
        runtime_asset_id = validate_runtime_asset_id(proposal["proposedRuntimeAssetId"])
        folded = runtime_asset_id.casefold()
        require(folded not in folded_targets, f"duplicate/casefold runtime target: {runtime_asset_id}")
        folded_targets.add(folded)
        target, exists = inspect_target_case(runtime_root, runtime_asset_id)
        if exists:
            verify_dds_file(target, proposal["sourceExactDdsEvidence"], f"existing runtime target {index}")
            dds = proposal["sourceExactDdsEvidence"]["dds"]
            states.append(TargetBefore("PRESENT_EXACT_EQUAL", dds["byteCount"], dds["rawSha256"]))
        else:
            states.append(TargetBefore("ABSENT", None, None))
        targets.append(target)
    return tuple(targets), tuple(states)


def _create_target_parent_directories(
    runtime_root: Path,
    contract: ContractData,
    created: list[Path],
) -> None:
    runtime_root = absolute_without_resolving(runtime_root)
    if not runtime_root.exists():
        runtime_root.mkdir()
        created.append(runtime_root)
    require(not _has_reparse_point(runtime_root), "runtime root became a symlink/reparse point")
    for proposal in contract.proposals:
        current = runtime_root
        for part in PurePosixPath(proposal["proposedRuntimeAssetId"]).parts[:-1]:
            matches = [entry for entry in current.iterdir() if entry.name.casefold() == part.casefold()]
            require(len(matches) <= 1, f"runtime directory casefold collision: {current}/{part}")
            if matches:
                child = matches[0]
                require(child.name == part and child.is_dir(), f"runtime directory case/type changed: {child}")
                require(not _has_reparse_point(child), f"runtime directory is symlink/reparse point: {child}")
            else:
                child = current / part
                child.mkdir()
                created.append(child)
            current = child


def _post_verify_targets(contract: ContractData, runtime_root: Path) -> None:
    for index, proposal in enumerate(contract.proposals):
        target = resolve_case_exact_file(
            runtime_root,
            proposal["proposedRuntimeAssetId"],
            f"deployed runtime target {index}",
        )
        verify_dds_file(target, proposal["sourceExactDdsEvidence"], f"deployed runtime target {index}")


def _verify_targets_match_prestate(
    contract: ContractData,
    runtime_root: Path,
    expected_targets: tuple[Path, ...],
    expected_states: tuple[TargetBefore, ...],
) -> None:
    current_targets, current_states = inspect_target_files(contract, runtime_root)
    require(current_targets == expected_targets, "runtime target paths changed before commit")
    require(current_states == expected_states, "runtime target bytes/state changed before commit")


def _rollback_targets(
    targets: tuple[Path, ...],
    states: tuple[TargetBefore, ...],
    backups: tuple[Path | None, ...],
    replaced_indices: tuple[int, ...],
    created_directories: list[Path],
) -> None:
    errors: list[str] = []
    for index in reversed(replaced_indices):
        target = targets[index]
        before = states[index]
        backup = backups[index]
        try:
            if before.status == "ABSENT":
                if target.exists() or target.is_symlink():
                    target.unlink()
            else:
                if backup is None or not backup.is_file():
                    raise DeploymentError(f"rollback backup missing for target {index}")
                restore_temp = backup.with_name(backup.name + ".restore")
                shutil.copyfile(backup, restore_temp)
                os.replace(restore_temp, target)
                if target.stat().st_size != before.byte_count or raw_sha256(target) != before.raw_sha256:
                    raise DeploymentError(f"rollback identity mismatch for target {index}")
        except Exception as exc:  # rollback must collect every target failure
            errors.append(f"target {index}: {exc}")
    for directory in reversed(created_directories):
        try:
            directory.rmdir()
        except FileNotFoundError:
            pass
        except OSError as exc:
            errors.append(f"directory {directory}: {exc}")
    if errors:
        raise DeploymentError("deployment rollback failed: " + "; ".join(errors))


def deploy_transaction(
    contract: ContractData,
    source_root: Path,
    runtime_root: Path,
    output_path: Path = DEFAULT_OUTPUT,
    *,
    require_approval: bool = True,
) -> dict[str, Any]:
    source_root = absolute_without_resolving(source_root)
    runtime_root = absolute_without_resolving(runtime_root)
    output_path = absolute_without_resolving(output_path)
    require_output_path_disjoint_from_runtime_transaction(
        contract,
        runtime_root,
        output_path,
    )
    require(output_path.parent.is_dir(), f"receipt output parent is missing: {output_path.parent}")
    require(not _has_reparse_point(output_path.parent), "receipt output parent is symlink/reparse point")

    if output_path.exists():
        require(output_path.is_file(), "deployment receipt output is not a regular file")
        require(not _has_reparse_point(output_path), "deployment receipt output is symlink/reparse point")
        existing = read_strict_json(output_path)
        validate_receipt(contract, existing, require_approval=require_approval)
        inspect_source_files(contract, source_root)
        _post_verify_targets(contract, runtime_root)
        verify_recovery_backup(contract, existing, runtime_root)
        return existing

    source_files = inspect_source_files(contract, source_root)
    targets, states = inspect_target_files(contract, runtime_root)
    receipt = build_receipt(contract, states)
    validate_receipt(contract, receipt, require_approval=require_approval)

    transaction_root = Path(
        tempfile.mkdtemp(prefix=TRANSACTION_DIRECTORY_PREFIX, dir=runtime_root.parent)
    )
    stage_root = transaction_root / "stage"
    backup_root = transaction_root / "backup"
    stage_root.mkdir()
    backup_root.mkdir()
    staged_files: list[Path] = []
    backups: list[Path | None] = [None] * 4
    created_directories: list[Path] = []
    receipt_temp: Path | None = None
    durable_backup_directory = recovery_backup_directory(receipt, runtime_root)
    durable_backup_parent_created = False
    durable_backup_created = False
    targets_may_have_changed = False
    replaced_indices: list[int] = []
    receipt_may_have_changed = False
    success = False
    try:
        for index, (source_file, proposal) in enumerate(zip(source_files, contract.proposals, strict=True)):
            staged = stage_root / f"{index}.dds"
            shutil.copyfile(source_file, staged)
            verify_dds_file(staged, proposal["sourceExactDdsEvidence"], f"staged DDS {index}")
            staged_files.append(staged)

        for index, (target, before) in enumerate(zip(targets, states, strict=True)):
            if before.status == "PRESENT_EXACT_EQUAL":
                backup = backup_root / f"{index}.dds"
                shutil.copyfile(target, backup)
                if backup.stat().st_size != before.byte_count or raw_sha256(backup) != before.raw_sha256:
                    raise DeploymentError(f"backup verification failed for target {index}")
                backups[index] = backup

        descriptor, receipt_temp_name = tempfile.mkstemp(
            prefix=RECEIPT_TEMP_PREFIX,
            suffix=".json.tmp",
            dir=output_path.parent,
        )
        receipt_temp = Path(receipt_temp_name)
        require_windows_paths_disjoint(
            output_path,
            receipt_temp,
            "deployment receipt/receipt temp",
        )
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(serialized_json_bytes(receipt))
            stream.flush()
            os.fsync(stream.fileno())
        validate_receipt(contract, read_strict_json(receipt_temp), require_approval=require_approval)

        durable_stage = transaction_root / "durable-recovery-backup"
        recovery_manifest = stage_recovery_backup(
            contract,
            states,
            targets,
            durable_stage,
        )
        require(
            recovery_manifest["manifestSha256"]
            == receipt["recoveryBackup"]["manifestCanonicalSelfSha256"],
            "staged recovery backup canonical self identity changed",
        )
        require(
            hashlib.sha256(serialized_json_bytes(recovery_manifest)).hexdigest()
            == receipt["recoveryBackup"]["manifestRawSha256"],
            "staged recovery backup raw file identity changed",
        )
        durable_backup_parent = durable_backup_directory.parent
        parent_matches = [
            entry
            for entry in runtime_root.parent.iterdir()
            if entry.name.casefold() == durable_backup_parent.name.casefold()
        ]
        require(len(parent_matches) <= 1, "recovery backup parent has a casefold collision")
        if parent_matches:
            require(
                parent_matches[0].name == RECOVERY_DIRECTORY_NAME
                and parent_matches[0].is_dir()
                and not _has_reparse_point(parent_matches[0]),
                "recovery backup parent path/type changed",
            )
            durable_backup_parent = parent_matches[0]
            durable_backup_directory = durable_backup_parent / durable_backup_directory.name
        else:
            durable_backup_parent.mkdir()
            durable_backup_parent_created = True
        final_matches = [
            entry
            for entry in durable_backup_parent.iterdir()
            if entry.name.casefold() == durable_backup_directory.name.casefold()
        ]
        require(not final_matches, "recovery backup directory already exists")
        os.replace(durable_stage, durable_backup_directory)
        durable_backup_created = True
        verify_recovery_backup(contract, receipt, runtime_root)

        targets_may_have_changed = True
        _create_target_parent_directories(runtime_root, contract, created_directories)
        _verify_targets_match_prestate(contract, runtime_root, targets, states)
        for index, (staged, target) in enumerate(zip(staged_files, targets, strict=True)):
            os.replace(staged, target)
            replaced_indices.append(index)

        _post_verify_targets(contract, runtime_root)
        receipt_may_have_changed = True
        os.replace(receipt_temp, output_path)
        receipt_temp = None
        validate_receipt(contract, read_strict_json(output_path), require_approval=require_approval)
        _post_verify_targets(contract, runtime_root)
        verify_recovery_backup(contract, receipt, runtime_root)
        success = True
        return receipt
    except Exception as exc:
        rollback_error: Exception | None = None
        if receipt_may_have_changed and output_path.exists():
            try:
                output_path.unlink()
            except Exception as output_exc:
                rollback_error = output_exc
        if targets_may_have_changed:
            try:
                _rollback_targets(
                    targets,
                    states,
                    tuple(backups),
                    tuple(replaced_indices),
                    created_directories,
                )
            except Exception as target_exc:
                rollback_error = target_exc if rollback_error is None else DeploymentError(
                    f"receipt rollback failed: {rollback_error}; target rollback failed: {target_exc}"
                )
        if durable_backup_created:
            try:
                shutil.rmtree(durable_backup_directory)
                if durable_backup_parent_created:
                    durable_backup_directory.parent.rmdir()
            except Exception as backup_exc:
                rollback_error = backup_exc if rollback_error is None else DeploymentError(
                    f"state rollback failed: {rollback_error}; recovery backup cleanup failed: {backup_exc}"
                )
        elif durable_backup_parent_created:
            try:
                durable_backup_directory.parent.rmdir()
            except Exception as backup_parent_exc:
                rollback_error = (
                    backup_parent_exc
                    if rollback_error is None
                    else DeploymentError(
                        f"state rollback failed: {rollback_error}; "
                        f"recovery backup parent cleanup failed: {backup_parent_exc}"
                    )
                )
        if rollback_error is not None:
            raise DeploymentError(f"deployment failed ({exc}) and rollback failed ({rollback_error})") from exc
        raise DeploymentError(f"deployment failed and all four targets were rolled back: {exc}") from exc
    finally:
        if receipt_temp is not None:
            try:
                receipt_temp.unlink()
            except FileNotFoundError:
                pass
        try:
            shutil.rmtree(transaction_root)
        except FileNotFoundError:
            pass
        except OSError:
            if not success:
                raise


def deep_verify(
    contract: ContractData,
    receipt: dict[str, Any],
    source_root: Path,
    runtime_root: Path,
) -> None:
    validate_receipt(contract, receipt)
    inspect_source_files(contract, source_root)
    _post_verify_targets(contract, runtime_root)
    verify_recovery_backup(contract, receipt, runtime_root)


def historical_receipt_states_for_refresh(
    contract: ContractData,
    receipt: dict[str, Any],
) -> tuple[TargetBefore, ...]:
    """Read only the frozen historical facts needed to refresh receipt evidence."""

    require_exact_keys(
        receipt,
        (
            "schema",
            "formatVersion",
            "characterClass",
            "skillId",
            "inputSlot",
            "deploymentContract",
            "sourceEvidence",
            "assets",
            "recoveryBackup",
            "admission",
            "summary",
            "receiptSha256",
        ),
        "historical deployment receipt",
    )
    require(
        receipt["schema"] == SCHEMA
        and type(receipt["formatVersion"]) is int
        and receipt["formatVersion"] == FORMAT_VERSION
        and receipt["characterClass"] == "ARTIST"
        and type(receipt["skillId"]) is int
        and receipt["skillId"] == 31470
        and receipt["inputSlot"] == "F",
        "historical deployment receipt identity changed",
    )
    validate_receipt_self(receipt, "receiptSha256")
    rows = receipt.get("assets")
    require(
        isinstance(rows, list) and len(rows) == 4,
        "historical deployment receipt denominator changed",
    )
    states = receipt_target_states(receipt)
    rebuilt = build_receipt(contract, states)
    require(
        strict_equal(receipt["deploymentContract"], rebuilt["deploymentContract"]),
        "historical deployment contract changed",
    )
    require(
        strict_equal(receipt["assets"], rebuilt["assets"]),
        "historical deployment asset facts changed",
    )
    require(
        strict_equal(receipt["admission"], rebuilt["admission"])
        and strict_equal(receipt["summary"], rebuilt["summary"]),
        "historical deployment admission/summary changed",
    )
    source = receipt["sourceEvidence"]
    expected_source = rebuilt["sourceEvidence"]
    require_exact_keys(
        source,
        (
            "authorityCommit",
            "authorityTree",
            "materialTextureBindingReceipt",
            "exactDdsRecoveryReceipt",
            "provisioningProposalProjectionSha256",
            "implementationEvidence",
        ),
        "historical sourceEvidence",
    )
    for field in (
        "authorityCommit",
        "authorityTree",
        "materialTextureBindingReceipt",
        "exactDdsRecoveryReceipt",
        "provisioningProposalProjectionSha256",
    ):
        require(
            strict_equal(source[field], expected_source[field]),
            f"historical sourceEvidence.{field} changed",
        )

    backup = receipt["recoveryBackup"]
    expected_backup = rebuilt["recoveryBackup"]
    legacy_keys = (
        "anchor",
        "relativeDirectory",
        "manifestRelativePath",
        "manifestSha256",
        "backupPayloadFileCount",
        "absentTargetMarkerCount",
        "preservedAfterCommit",
    )
    current_keys = (
        "anchor",
        "relativeDirectory",
        "manifestRelativePath",
        "manifestCanonicalSelfSha256",
        "manifestRawSha256",
        "backupPayloadFileCount",
        "absentTargetMarkerCount",
        "preservedAfterCommit",
    )
    require(
        isinstance(backup, dict) and tuple(backup.keys()) in (legacy_keys, current_keys),
        "historical recoveryBackup keys changed",
    )
    for field in (
        "anchor",
        "relativeDirectory",
        "manifestRelativePath",
        "backupPayloadFileCount",
        "absentTargetMarkerCount",
        "preservedAfterCommit",
    ):
        require(
            strict_equal(backup[field], expected_backup[field]),
            f"historical recoveryBackup.{field} changed",
        )
    canonical_self = backup.get(
        "manifestCanonicalSelfSha256",
        backup.get("manifestSha256"),
    )
    require(
        canonical_self == expected_backup["manifestCanonicalSelfSha256"],
        "historical recovery manifest canonical self changed",
    )
    if "manifestRawSha256" in backup:
        require(
            backup["manifestRawSha256"] == expected_backup["manifestRawSha256"],
            "historical recovery manifest raw SHA changed",
        )
    return states


def refresh_deployment_receipt(
    contract: ContractData,
    source_root: Path,
    runtime_root: Path,
    output_path: Path = DEFAULT_OUTPUT,
    *,
    require_approval: bool = True,
) -> dict[str, Any]:
    """Refresh receipt evidence without writing any runtime resource or backup."""

    source_root = absolute_without_resolving(source_root)
    runtime_root = absolute_without_resolving(runtime_root)
    output_path = absolute_without_resolving(output_path)
    require_output_path_disjoint_from_runtime_transaction(
        contract,
        runtime_root,
        output_path,
    )
    require(output_path.is_file(), f"deployment receipt is missing: {output_path}")
    require(
        not _has_reparse_point(output_path)
        and not _has_reparse_point(output_path.parent),
        "deployment receipt output or parent is symlink/reparse point",
    )
    previous_bytes = output_path.read_bytes()
    previous = read_strict_json(output_path)
    states = historical_receipt_states_for_refresh(contract, previous)
    refreshed = build_receipt(contract, states)
    validate_receipt(
        contract,
        refreshed,
        require_approval=require_approval,
    )

    inspect_source_files(contract, source_root)
    _post_verify_targets(contract, runtime_root)
    verify_recovery_backup(contract, refreshed, runtime_root)

    receipt_temp: Path | None = None
    receipt_replaced = False
    try:
        descriptor, receipt_temp_name = tempfile.mkstemp(
            prefix=RECEIPT_TEMP_PREFIX,
            suffix=".json.tmp",
            dir=output_path.parent,
        )
        receipt_temp = Path(receipt_temp_name)
        require_windows_paths_disjoint(
            output_path,
            receipt_temp,
            "deployment receipt/refresh temp",
        )
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(serialized_json_bytes(refreshed))
            stream.flush()
            os.fsync(stream.fileno())
        validate_receipt(
            contract,
            read_strict_json(receipt_temp),
            require_approval=require_approval,
        )
        _post_verify_targets(contract, runtime_root)
        verify_recovery_backup(contract, refreshed, runtime_root)
        os.replace(receipt_temp, output_path)
        receipt_temp = None
        receipt_replaced = True
        validate_receipt(
            contract,
            read_strict_json(output_path),
            require_approval=require_approval,
        )
        _post_verify_targets(contract, runtime_root)
        verify_recovery_backup(contract, refreshed, runtime_root)
        return refreshed
    except Exception as exc:
        if receipt_replaced:
            restore_descriptor, restore_name = tempfile.mkstemp(
                prefix=RECEIPT_TEMP_PREFIX,
                suffix=".restore.tmp",
                dir=output_path.parent,
            )
            restore_path = Path(restore_name)
            try:
                with os.fdopen(restore_descriptor, "wb") as stream:
                    stream.write(previous_bytes)
                    stream.flush()
                    os.fsync(stream.fileno())
                os.replace(restore_path, output_path)
            finally:
                try:
                    restore_path.unlink()
                except FileNotFoundError:
                    pass
        raise DeploymentError(
            f"deployment receipt refresh failed; runtime targets were not written: {exc}"
        ) from exc
    finally:
        if receipt_temp is not None:
            try:
                receipt_temp.unlink()
            except FileNotFoundError:
                pass


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binding-receipt", type=Path, default=DEFAULT_BINDING_RECEIPT)
    parser.add_argument("--exact-dds-receipt", type=Path, default=DEFAULT_EXACT_DDS_RECEIPT)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--source-dds-root", type=Path)
    parser.add_argument("--runtime-resource-root", type=Path)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--deploy", action="store_true")
    mode.add_argument("--refresh-receipt", action="store_true")
    mode.add_argument("--check", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    contract = load_contract(args.binding_receipt, args.exact_dds_receipt)
    if args.deploy or args.refresh_receipt:
        operation = "--deploy" if args.deploy else "--refresh-receipt"
        require(args.source_dds_root is not None, f"{operation} requires --source-dds-root")
        require(args.runtime_resource_root is not None, f"{operation} requires --runtime-resource-root")
        if args.deploy:
            receipt = deploy_transaction(
                contract,
                args.source_dds_root,
                args.runtime_resource_root,
                args.output,
            )
            mode = "deploy"
        else:
            receipt = refresh_deployment_receipt(
                contract,
                args.source_dds_root,
                args.runtime_resource_root,
                args.output,
            )
            mode = "refresh-receipt"
    else:
        require(args.output.is_file(), f"deployment receipt is missing: {args.output}")
        receipt = read_strict_json(args.output)
        validate_receipt(contract, receipt)
        if (args.source_dds_root is None) != (args.runtime_resource_root is None):
            raise ContractError("deep check requires both --source-dds-root and --runtime-resource-root")
        if args.source_dds_root is not None:
            deep_verify(contract, receipt, args.source_dds_root, args.runtime_resource_root)
            mode = "deep"
        else:
            mode = "shallow"
    print(
        "PASS: Artist F 31470 exact DDS runtime deployment "
        f"mode={mode} assets={receipt['summary']['deployedAssetCount']}/4 "
        "sourceExactMaterial=false r4=false product=false"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
