#!/usr/bin/env python3
"""Build the fail-closed Artist 31470 Material texture runtime binding receipt.

The receipt joins reconstructed sampler-policy ownership to the frozen Artist
resource export/cook evidence by the complete logical texture path.  It never
uses an object basename as an identity.  Exact DDS recovery evidence is
admitted only through the frozen transactional deployment receipt.  That path
remains reconstructed and never becomes a source-exact Material claim.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import importlib
import json
import math
from pathlib import Path, PurePosixPath
from typing import Any, Iterable

import effect_source_contract_io as strict_io_module


def discover_repository_root() -> Path:
    current = Path.cwd().resolve()
    for candidate in (current, *current.parents):
        if (
            (candidate / ".git").exists()
            and (candidate / "Tools/LevelPlacementExtractor").is_dir()
            and (candidate / "Data/Effects/Imported/Artist/Materials").is_dir()
        ):
            return candidate
    raise RuntimeError("cannot locate canonical LostArk repository root")


ROOT = discover_repository_root()
MATERIAL_ROOT = ROOT / "Data/Effects/Imported/Artist/Materials"
DEFAULT_POLICY = MATERIAL_ROOT / "skill.31470.material-reconstructed-approved-v1.receipt.json"
DEFAULT_CONTRACT = MATERIAL_ROOT / "skill.31470.typed-material-evidence-contract.json"
DEFAULT_RUNTIME_ORACLE = MATERIAL_ROOT / "skill.31470.material-runtime-oracle.receipt.json"
DEFAULT_ACQUISITION = MATERIAL_ROOT / "skill.31470.material-source-value-acquisition.receipt.json"
DEFAULT_EXACT_DDS = MATERIAL_ROOT / "skill.31470.exact-dds-recovery.receipt.json"
DEFAULT_EXACT_DDS_DEPLOYMENT = (
    MATERIAL_ROOT / "skill.31470.exact-dds-runtime-deployment.receipt.json"
)
DEFAULT_RESOURCE_MANIFEST = ROOT / "Data/Effects/Imported/Artist/Artist.resource-source-manifest.json"
DEFAULT_CANDIDATE = (
    ROOT
    / "Data/Effects/Imported/Artist/Candidates/"
    "effect.artist.skill.31470.native-v14.source-contract-candidate.effect.json"
)
DEFAULT_EXTERNAL_ROOT = Path(
    r"C:\Users\user\Desktop\Resource_LostArk\05_Reports\EffectExtraction\ARTIST\all_bound_skills"
)
DEFAULT_RUNTIME_COOK = DEFAULT_EXTERNAL_ROOT / "runtime-cook-receipt.json"
DEFAULT_RESOURCE_EXPORT = DEFAULT_EXTERNAL_ROOT / "resource-export-receipt.json"
DEFAULT_SOURCE_PACK = Path(
    r"C:\Users\user\Desktop\Resource_LostArk\00_SourcePackages\Effect_DIMENSIONMASTER_20260803_v3\source_pack_manifest.json"
)
DEFAULT_RUNTIME_RESOURCES = Path(r"C:\Users\user\Desktop\LostArk\Client\Bin\Resources")
DEFAULT_OUTPUT = MATERIAL_ROOT / "skill.31470.material-texture-runtime-binding.receipt.json"
DEFAULT_APPROVAL = (
    ROOT
    / "Tools/LevelPlacementExtractor/artist_31470_material_texture_runtime_binding_approval.py"
)

DIRECT_IMPORT_DEPENDENCY_PATHS = {
    "STRICT_JSON_OBJECT_LOADER": ROOT / "Tools/LevelPlacementExtractor/effect_source_contract_io.py",
    "MATERIAL_EVIDENCE_VALIDATOR": ROOT / "Tools/LevelPlacementExtractor/build_artist_31470_material_evidence_contract.py",
    "MATERIAL_POLICY_VALIDATOR": ROOT / "Tools/LevelPlacementExtractor/build_artist_31470_material_reconstructed_policy.py",
    "TEXTURE_RUNTIME_BINDING_APPROVAL": DEFAULT_APPROVAL,
    "EXACT_DDS_RUNTIME_DEPLOYMENT_APPROVAL": (
        ROOT
        / "Tools/LevelPlacementExtractor/"
        "artist_31470_exact_dds_runtime_deployment_approval.py"
    ),
}
DIRECT_IMPORT_MODULE_NAMES = {
    "STRICT_JSON_OBJECT_LOADER": "effect_source_contract_io",
    "MATERIAL_EVIDENCE_VALIDATOR": "build_artist_31470_material_evidence_contract",
    "MATERIAL_POLICY_VALIDATOR": "build_artist_31470_material_reconstructed_policy",
    "TEXTURE_RUNTIME_BINDING_APPROVAL": "artist_31470_material_texture_runtime_binding_approval",
    "EXACT_DDS_RUNTIME_DEPLOYMENT_APPROVAL": (
        "artist_31470_exact_dds_runtime_deployment_approval"
    ),
}

SCHEMA = "lostark.artist-31470-material-texture-runtime-binding-receipt"
FORMAT_VERSION = 2
BINDING_CONTRACT_ID = "ARTIST_31470_MATERIAL_TEXTURE_RUNTIME_BINDING_V2"
RESOLVED_STATUS = "RESOLVED_EXACT_RUNTIME_COOK_RECEIPT"
DEPLOYED_STATUS = "RESOLVED_RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT"
PROVISIONING_POLICY = "RECONSTRUCTED_RUNTIME_DEPLOYMENT_FROM_EXACT_DDS_FIXTURE_V1"
DEPLOYMENT_BASIS = "RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT"
DEPLOYMENT_COMPLETE_STATUS = "COMPLETED_POST_VERIFIED_EXACT_DDS_DEPLOYMENT"
R4_BLOCKER = "R4_TEXTURE_SRV_CONSUMER_NOT_COMPLETE"
FROZEN_MATERIAL_POLICY_COMMIT = "3ba493de5fde8d058ddee7e0fa0e6c3e466faa43"
EXPECTED_POLICY_ROWS = 77
EXPECTED_UNIQUE_TEXTURES = 52
EXPECTED_RESOLVED_ROWS = 77
EXPECTED_UNRESOLVED_ROWS = 0
EXPECTED_COOK_BINDING_ROWS = 73
EXPECTED_DEPLOYMENT_BINDING_ROWS = 4
EXPECTED_RESOLVED_TEXTURES = 52
EXPECTED_UNRESOLVED_TEXTURES = 0
EXPECTED_COOK_TEXTURES = 48
EXPECTED_DEPLOYMENT_TEXTURES = 4
EXPECTED_OCCURRENCE_LINKS = 94
EXPECTED_MISSING_LOGICAL_PATHS = {
    "fx_tex_00.fx_a_decal_014",
    "fx_tex_00.fx_a_noise_011",
    "fx_tex_01.fx_c_atypical_016",
    "fx_tex_03.fx_e_ring_001_cl",
}
TEXTURE_RESOURCE_KEYS = {
    "textureResourceId", "logicalTexturePath", "status", "runtimeAssetId",
    "sourceResourceManifest", "sourcePackage", "sourceEvidenceBlockers",
    "runtimeCookEvidence", "resourceExportEvidence", "exactDdsEvidence",
    "deploymentEvidence",
    "candidateObservations", "blockers", "provisioningProposalId",
    "runtimeAssetAdmission", "sourceExact", "rowSha256",
}
RESOURCE_MANIFEST_EVIDENCE_KEYS = {
    "logicalPackage", "physicalPackage", "roles", "skillIds", "sourceSystems",
    "resolutionStatus", "sourceRowSha256",
}
SOURCE_PACKAGE_EVIDENCE_KEYS = {
    "logicalPackage", "physicalPackage", "relativePath", "byteSize", "rawSha256",
    "sourceRowSha256",
}
COOK_EVIDENCE_KEYS = {
    "sourceAssetPath", "sourceFile", "runtimeAssetId", "byteSize", "rawSha256",
    "sourceRowSha256",
}
EXPORT_EVIDENCE_KEYS = {
    "sourceAssetPath", "logicalPackage", "outputRelativePath", "byteSize",
    "rawSha256", "sourceRowSha256", "outputRowSha256",
}
CANDIDATE_OBSERVATION_KEYS = {"elementId", "slotId", "runtimeAssetId"}
EXACT_DDS_EVIDENCE_KEYS = {
    "logicalObjectPath", "fixtureAssetId", "copyPolicy",
    "sourceExtractedDdsRelativePath", "sourceTexture2D", "dds", "sourceRowSha256",
}
EXACT_DDS_TEXTURE_KEYS = {
    "logicalPackage", "physicalPackage", "physicalPackageByteCount",
    "physicalPackageSha256", "exportIndex", "packageReference", "serialOffset",
    "serialSize", "serialSha256",
}
EXACT_DDS_PAYLOAD_KEYS = {
    "byteCount", "rawSha256", "header128Sha256", "width", "height", "fourCC",
}
DEPLOYMENT_EVIDENCE_KEYS = {
    "basis", "artifactAuthorityCommit", "artifactAuthorityTree",
    "receiptRelativePath", "receiptGitBlob", "receiptTrackedTextSha256",
    "receiptSha256", "approvalProjectionSha256", "implementationProjectionSha256",
    "deploymentRowId", "proposalId", "deploymentRowSha256", "deploymentStatus",
    "runtimeAssetId", "byteCount", "rawSha256", "pathCaseVerified",
    "regularFileVerified", "symlinkFreeVerified", "postVerified",
    "sourceExactMaterialClaim", "runtimeAssetDeploymentAdmission",
}
DEPLOYMENT_ROW_KEYS = {
    "deploymentRowId", "proposalId", "textureResourceId", "logicalTexturePath",
    "runtimeAssetId", "policy", "sourceExactDdsEvidence", "targetBefore",
    "deployedFile", "deploymentStatus", "sourceExactMaterialClaim",
    "runtimeAssetDeploymentAdmission", "rendererConsumerAdmission",
    "productAdmission", "blockers", "rowSha256",
}
DEPLOYED_FILE_KEYS = {
    "runtimeAssetId", "byteCount", "rawSha256", "pathCaseVerified",
    "regularFileVerified", "symlinkFreeVerified", "postVerified",
}
PROVISIONING_PROPOSAL_KEYS = {
    "proposalId", "textureResourceId", "logicalTexturePath", "policy",
    "proposedRuntimeAssetId", "sourceExactDdsEvidence", "deploymentStatus",
    "requiredReceipt", "deploymentEvidence", "sourceExact",
    "runtimeAssetAdmission", "productAdmission",
    "rowSha256",
}
MATERIAL_BINDING_KEYS = {
    "bindingId", "bindingOrder", "samplerPolicyRowId", "samplerPolicyOrder",
    "recipeId", "materialInputFieldId", "materialOccurrenceIds",
    "logicalTexturePath", "textureResourceId", "status", "runtimeAssetId",
    "samplerDescriptor", "samplerDescriptorSha256", "srvIdentity",
    "srvIdentitySha256", "bindingOriginAndOwner", "policySourceRowSha256",
    "runtimeAssetAdmission", "rendererConsumerAdmission", "productAdmission",
    "sourceExact", "rowSha256",
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def strict_equal(actual: Any, expected: Any) -> bool:
    if type(actual) is not type(expected):
        return False
    if isinstance(expected, dict):
        return set(actual) == set(expected) and all(
            strict_equal(actual[key], expected[key]) for key in expected
        )
    if isinstance(expected, list):
        return len(actual) == len(expected) and all(
            strict_equal(left, right) for left, right in zip(actual, expected, strict=True)
        )
    return actual == expected


def require_exact_keys(value: Any, expected: set[str], label: str) -> dict[str, Any]:
    require(isinstance(value, dict) and set(value) == expected, f"{label} schema changed")
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


def tracked_text_sha256(path: Path) -> str:
    return strict_io_module.tracked_text_sha256(path)


def tracked_text_crlf_sha256(path: Path) -> str:
    normalized = strict_io_module.normalize_utf8_eol(path.read_bytes())
    return hashlib.sha256(normalized.replace(b"\n", b"\r\n")).hexdigest()


def raw_file_sha256(path: Path) -> str:
    return strict_io_module.raw_file_sha256(path)


def read_json(path: Path) -> dict[str, Any]:
    return strict_io_module.load_strict_json_object(path)


def load_approved_tracked_authorities() -> dict[str, dict[str, Any]]:
    approval = approval_module()
    specifications = {
        "materialPolicy": (
            DEFAULT_POLICY,
            approval.POLICY_TRACKED_TEXT_SHA256,
            "receiptSha256",
            approval.POLICY_RECEIPT_SHA256,
        ),
        "typedMaterialContract": (
            DEFAULT_CONTRACT,
            approval.MATERIAL_CONTRACT_TRACKED_TEXT_SHA256,
            "contractSha256",
            approval.MATERIAL_CONTRACT_SHA256,
        ),
        "materialRuntimeOracle": (
            DEFAULT_RUNTIME_ORACLE,
            approval.RUNTIME_ORACLE_TRACKED_TEXT_SHA256,
            "receiptSha256",
            approval.RUNTIME_ORACLE_RECEIPT_SHA256,
        ),
        "materialSourceValueAcquisition": (
            DEFAULT_ACQUISITION,
            approval.ACQUISITION_TRACKED_TEXT_SHA256,
            "receiptSha256",
            approval.ACQUISITION_RECEIPT_SHA256,
        ),
        "resourceSourceManifest": (
            DEFAULT_RESOURCE_MANIFEST,
            approval.RESOURCE_MANIFEST_TRACKED_TEXT_SHA256,
            None,
            None,
        ),
        "exactDdsRecoveryReceipt": (
            DEFAULT_EXACT_DDS,
            approval.EXACT_DDS_TRACKED_TEXT_SHA256,
            None,
            None,
        ),
        "nativeV14Candidate": (
            DEFAULT_CANDIDATE,
            approval.CANDIDATE_TRACKED_TEXT_SHA256,
            None,
            None,
        ),
    }
    authorities: dict[str, dict[str, Any]] = {}
    for evidence_id, (path, expected_digest, self_key, expected_self) in (
        specifications.items()
    ):
        require(path.is_file(), f"approved tracked evidence is missing: {evidence_id}")
        before = path.read_bytes()
        actual_digest = tracked_text_sha256(path)
        require(
            actual_digest == expected_digest,
            f"approved tracked evidence changed: {evidence_id}",
        )
        document = read_json(path)
        require(
            path.read_bytes() == before,
            f"approved tracked evidence changed while reading: {evidence_id}",
        )
        if self_key is not None:
            require(
                document.get(self_key) == expected_self,
                f"approved tracked self identity changed: {evidence_id}",
            )
        authorities[evidence_id] = {
            "relativePath": path.resolve().relative_to(ROOT.resolve()).as_posix(),
            "trackedTextSha256": actual_digest,
            "selfDigest": expected_self,
            "document": document,
        }
    return authorities


def bind_supplied_tracked_inputs(
    policy: dict[str, Any],
    contract: dict[str, Any],
    runtime_oracle: dict[str, Any] | None,
    acquisition: dict[str, Any] | None,
    resource_manifest: dict[str, Any],
    exact_dds: dict[str, Any],
    candidate: dict[str, Any],
) -> tuple[dict[str, Any], ...]:
    authorities = load_approved_tracked_authorities()
    supplied = {
        "materialPolicy": policy,
        "typedMaterialContract": contract,
        "materialRuntimeOracle": runtime_oracle,
        "materialSourceValueAcquisition": acquisition,
        "resourceSourceManifest": resource_manifest,
        "exactDdsRecoveryReceipt": exact_dds,
        "nativeV14Candidate": candidate,
    }
    for evidence_id, supplied_document in supplied.items():
        if supplied_document is not None:
            require(
                strict_equal(supplied_document, authorities[evidence_id]["document"]),
                f"supplied tracked object differs from approved authority: {evidence_id}",
            )
    return (
        authorities["materialPolicy"]["document"],
        authorities["typedMaterialContract"]["document"],
        authorities["materialRuntimeOracle"]["document"],
        authorities["materialSourceValueAcquisition"]["document"],
        authorities["resourceSourceManifest"]["document"],
        authorities["exactDdsRecoveryReceipt"]["document"],
        authorities["nativeV14Candidate"]["document"],
        authorities,
    )


def load_approved_external_authorities() -> dict[str, dict[str, Any]]:
    approval = approval_module()
    specifications = {
        "runtimeCookReceipt": (
            DEFAULT_RUNTIME_COOK,
            approval.RUNTIME_COOK_BYTE_COUNT,
            approval.RUNTIME_COOK_RAW_SHA256,
            approval.RUNTIME_COOK_CANONICAL_SHA256,
        ),
        "resourceExportReceipt": (
            DEFAULT_RESOURCE_EXPORT,
            approval.RESOURCE_EXPORT_BYTE_COUNT,
            approval.RESOURCE_EXPORT_RAW_SHA256,
            approval.RESOURCE_EXPORT_CANONICAL_SHA256,
        ),
        "sourcePackManifest": (
            DEFAULT_SOURCE_PACK,
            approval.SOURCE_PACK_MANIFEST_BYTE_COUNT,
            approval.SOURCE_PACK_MANIFEST_RAW_SHA256,
            approval.SOURCE_PACK_MANIFEST_CANONICAL_SHA256,
        ),
    }
    authorities: dict[str, dict[str, Any]] = {}
    for evidence_id, (path, byte_count, raw_sha256, canonical_json_sha256) in (
        specifications.items()
    ):
        require(path.is_file(), f"approved external evidence is missing: {evidence_id}")
        payload = path.read_bytes()
        require(
            len(payload) == byte_count,
            f"approved external evidence byte count changed: {evidence_id}",
        )
        actual_raw_sha256 = hashlib.sha256(payload).hexdigest()
        require(
            actual_raw_sha256 == raw_sha256,
            f"approved external raw evidence changed: {evidence_id}",
        )
        document = read_json(path)
        actual_canonical_sha256 = canonical_sha256(document)
        require(
            actual_canonical_sha256 == canonical_json_sha256,
            f"approved external canonical identity changed: {evidence_id}",
        )
        require(
            path.read_bytes() == payload,
            f"approved external evidence changed while reading: {evidence_id}",
        )
        authorities[evidence_id] = {
            "byteCount": len(payload),
            "rawSha256": actual_raw_sha256,
            "canonicalJsonSha256": actual_canonical_sha256,
            "document": document,
        }
    return authorities


def require_sha256(value: Any, label: str) -> str:
    require(
        isinstance(value, str)
        and len(value) == 64
        and all(character in "0123456789abcdef" for character in value),
        f"{label} must be a lowercase SHA-256",
    )
    return value


def require_exact_integer(value: Any, expected: int, label: str) -> None:
    require(type(value) is int and value == expected, f"{label} must be exact JSON integer {expected}")


def validate_finite_tree(value: Any, label: str = "root") -> None:
    if isinstance(value, float):
        require(math.isfinite(value), f"{label} must be finite")
    elif isinstance(value, list):
        for index, child in enumerate(value):
            validate_finite_tree(child, f"{label}[{index}]")
    elif isinstance(value, dict):
        for key, child in value.items():
            validate_finite_tree(child, f"{label}.{key}")


def folded(value: Any) -> str:
    require(isinstance(value, str) and value, "identity must be a non-empty string")
    return value.casefold()


def validate_logical_texture_path(value: Any) -> str:
    require(isinstance(value, str) and value == value.strip(), "logical texture path is invalid")
    require(value == value.casefold(), f"logical texture path must be lowercase: {value}")
    require("/" not in value and "\\" not in value and ":" not in value, f"unsafe logical path: {value}")
    require(".." not in value and value.count(".") >= 1, f"unsafe logical path: {value}")
    return value


def validate_runtime_asset_id(value: Any) -> str:
    require(isinstance(value, str) and value == value.strip(), "runtime asset ID is invalid")
    require("\\" not in value and ":" not in value, f"unsafe runtime asset ID: {value}")
    path = PurePosixPath(value)
    require(not path.is_absolute() and ".." not in path.parts and "." not in path.parts, f"unsafe runtime asset ID: {value}")
    require(value == "/".join(path.parts), f"runtime asset ID is not canonical POSIX relative form: {value}")
    require(all(part == part.strip() and part for part in path.parts), f"runtime asset ID contains an invalid segment: {value}")
    require(
        len(path.parts) >= 4
        and path.parts[:3] == ("Effect", "Artist", "Textures")
        and path.suffix.casefold() == ".dds",
        f"runtime asset ID is outside Effect/Artist/Textures: {value}",
    )
    return value


def validate_relative_path(value: Any, label: str) -> str:
    require(isinstance(value, str) and value and "\\" not in value and ":" not in value, f"unsafe {label}")
    path = PurePosixPath(value)
    require(not path.is_absolute() and ".." not in path.parts and "." not in path.parts, f"unsafe {label}")
    require(value == "/".join(path.parts), f"{label} is not canonical POSIX relative form")
    require(all(part == part.strip() and part for part in path.parts), f"{label} contains an invalid segment")
    return value


def row_with_digest(row: dict[str, Any]) -> dict[str, Any]:
    candidate = copy.deepcopy(row)
    candidate["rowSha256"] = canonical_sha256(candidate)
    return candidate


def validate_row_digest(row: dict[str, Any], label: str) -> None:
    digest = require_sha256(row.get("rowSha256"), f"{label}.rowSha256")
    candidate = copy.deepcopy(row)
    candidate.pop("rowSha256", None)
    require(digest == canonical_sha256(candidate), f"{label} row digest mismatch")


def load_direct_import_module(dependency_id: str) -> Any:
    require(dependency_id in DIRECT_IMPORT_MODULE_NAMES, f"unknown direct dependency: {dependency_id}")
    module = (
        strict_io_module
        if dependency_id == "STRICT_JSON_OBJECT_LOADER"
        else importlib.import_module(DIRECT_IMPORT_MODULE_NAMES[dependency_id])
    )
    expected = DIRECT_IMPORT_DEPENDENCY_PATHS[dependency_id].resolve()
    actual_file = getattr(module, "__file__", None)
    require(isinstance(actual_file, str), f"loaded dependency has no path: {dependency_id}")
    actual = Path(actual_file).resolve()
    require(actual == expected, f"loaded dependency path mismatch: {dependency_id}")
    return module


def direct_import_closure() -> dict[str, Any]:
    builder_path = Path(__file__).resolve()
    expected_builder_path = (
        ROOT
        / "Tools/LevelPlacementExtractor/build_artist_31470_material_texture_runtime_binding.py"
    ).resolve()
    require(builder_path == expected_builder_path, "binding builder module path mismatch")
    rows = [
        {
            "dependencyId": "TEXTURE_RUNTIME_BINDING_BUILDER",
            "relativePath": builder_path.relative_to(ROOT.resolve()).as_posix(),
            "trackedTextSha256": tracked_text_sha256(builder_path),
        }
    ]
    for dependency_id in sorted(DIRECT_IMPORT_MODULE_NAMES):
        module = load_direct_import_module(dependency_id)
        path = Path(module.__file__).resolve()
        rows.append(
            {
                "dependencyId": dependency_id,
                "relativePath": path.relative_to(ROOT.resolve()).as_posix(),
                "trackedTextSha256": tracked_text_sha256(path),
            }
        )
    return {
        "dependencyCount": len(rows),
        "dependencies": rows,
        "projectionSha256": canonical_sha256(rows),
    }


def validate_direct_import_closure(closure: Any) -> None:
    require(isinstance(closure, dict), "direct import closure must be an object")
    expected = direct_import_closure()
    require(strict_equal(closure, expected), "direct import closure changed")


def validate_upstream_documents(
    policy: dict[str, Any],
    contract: dict[str, Any],
    runtime_oracle: dict[str, Any],
    acquisition: dict[str, Any],
    exact_dds: dict[str, Any],
) -> dict[str, dict[str, Any]]:
    policy_builder = load_direct_import_module("MATERIAL_POLICY_VALIDATOR")
    evidence_builder = load_direct_import_module("MATERIAL_EVIDENCE_VALIDATOR")
    evidence_builder.validate_contract(contract)
    policy_builder.validate_policy_receipt(
        policy,
        runtime_oracle,
        acquisition,
        contract,
        DEFAULT_RUNTIME_ORACLE,
        DEFAULT_ACQUISITION,
        DEFAULT_CONTRACT,
    )
    exact_rows = evidence_builder.validate_dds_receipt(exact_dds)
    return {folded(row["logicalObjectPath"]): row for row in exact_rows}


def validate_resource_manifest(manifest: dict[str, Any]) -> dict[str, dict[str, Any]]:
    require(
        manifest.get("schema") == "lostark.class-effect-resource-source-manifest"
        and manifest.get("characterClass") == "ARTIST",
        "unsupported Artist resource source manifest",
    )
    require_exact_integer(manifest.get("formatVersion"), 1, "resource manifest formatVersion")
    assets = manifest.get("assets")
    require(isinstance(assets, list) and len(assets) == 574, "resource manifest denominator changed")
    result: dict[str, dict[str, Any]] = {}
    for row in assets:
        require(isinstance(row, dict), "resource manifest asset must be an object")
        key = folded(row.get("sourceAssetPath"))
        require(key not in result, f"duplicate resource manifest logical path: {key}")
        result[key] = row
    return result


def validate_candidate(candidate: dict[str, Any]) -> None:
    require(
        candidate.get("schema") == "lostark.effect-authoring"
        and type(candidate.get("version")) is int
        and candidate["version"] == 14
        and candidate.get("purpose") == "source_contract"
        and candidate.get("effectAssetId")
        == "effect.artist.skill.31470.native-v14.source-contract-candidate",
        "unsupported Artist 31470 source-contract candidate",
    )
    elements = candidate.get("elements")
    require(isinstance(elements, list) and len(elements) == 35, "candidate element denominator changed")


def candidate_observations(candidate: dict[str, Any], runtime_asset_id: str | None) -> list[dict[str, Any]]:
    if runtime_asset_id is None:
        return []
    rows: list[dict[str, Any]] = []
    for element in candidate["elements"]:
        require(isinstance(element, dict), "candidate element must be an object")
        resources = element.get("resources")
        require(isinstance(resources, list), "candidate element resources must be a list")
        for resource in resources:
            require(isinstance(resource, dict), "candidate resource must be an object")
            if resource.get("assetId") == runtime_asset_id:
                rows.append(
                    {
                        "elementId": element.get("id"),
                        "slotId": resource.get("slotId"),
                        "runtimeAssetId": runtime_asset_id,
                    }
                )
    return sorted(rows, key=lambda row: (str(row["elementId"]), str(row["slotId"])))


def validate_runtime_cook(cook: dict[str, Any]) -> dict[str, dict[str, Any]]:
    require(
        cook.get("schema") == "lostark.effect-runtime-resource-cook-receipt"
        and cook.get("characterClass") == "ARTIST",
        "unsupported Artist runtime cook receipt",
    )
    require_exact_integer(cook.get("formatVersion"), 1, "runtime cook formatVersion")
    require(cook.get("sourceExportReceiptSha256") == approval_module().RESOURCE_EXPORT_RAW_SHA256, "cook export receipt pin changed")
    assets = cook.get("assets")
    require(isinstance(assets, list) and len(assets) == 272, "runtime cook asset denominator changed")
    require(cook.get("failures") == [], "runtime cook contains failures")
    by_source: dict[str, dict[str, Any]] = {}
    runtime_ids: set[str] = set()
    for row in assets:
        require(isinstance(row, dict), "runtime cook asset must be an object")
        key = folded(row.get("sourceAssetPath"))
        require(key not in by_source, f"duplicate runtime cook source path: {key}")
        by_source[key] = row
        if row.get("role") == "texture":
            asset_id = validate_runtime_asset_id(row.get("runtimeAssetId"))
            folded_asset = asset_id.casefold()
            require(folded_asset not in runtime_ids, f"runtime cook asset ID casefold collision: {asset_id}")
            runtime_ids.add(folded_asset)
            require(row.get("status") == "COPIED", f"runtime texture was not copied: {key}")
            require(type(row.get("byteSize")) is int and row["byteSize"] > 0, f"invalid cook byte size: {key}")
            require_sha256(row.get("sha256"), f"cook[{key}].sha256")
            validate_relative_path(row.get("sourceFile"), f"cook[{key}].sourceFile")
    return by_source


def validate_resource_export(export: dict[str, Any]) -> dict[str, dict[str, Any]]:
    require(
        export.get("schema") == "lostark.effect-resource-export-receipt"
        and export.get("characterClass") == "ARTIST",
        "unsupported Artist resource export receipt",
    )
    require_exact_integer(export.get("formatVersion"), 1, "resource export formatVersion")
    require(
        export.get("sourceManifestSha256")
        == approval_module().RESOURCE_EXPORT_SOURCE_MANIFEST_CRLF_SHA256
        == tracked_text_crlf_sha256(DEFAULT_RESOURCE_MANIFEST),
        "resource export source manifest pin changed",
    )
    require(export.get("processFailures") == [] and export.get("missingAssets") == [], "resource export is incomplete")
    assets = export.get("assets")
    require(isinstance(assets, list) and len(assets) == 272, "resource export denominator changed")
    result: dict[str, dict[str, Any]] = {}
    for row in assets:
        require(isinstance(row, dict), "resource export asset must be an object")
        key = folded(row.get("sourceAssetPath"))
        require(key not in result, f"duplicate resource export source path: {key}")
        result[key] = row
    return result


def validate_source_pack(source_pack: dict[str, Any]) -> dict[str, dict[str, Any]]:
    require_exact_integer(source_pack.get("schemaVersion"), 1, "source pack schemaVersion")
    packages = source_pack.get("packages")
    require(isinstance(packages, list) and len(packages) == 621, "source pack package denominator changed")
    result: dict[str, dict[str, Any]] = {}
    physical_names: set[str] = set()
    for row in packages:
        require(isinstance(row, dict), "source pack package must be an object")
        key = folded(row.get("logicalPackage"))
        require(key not in result, f"duplicate source pack logical package: {key}")
        result[key] = row
        physical = folded(row.get("physicalPackage"))
        require(physical not in physical_names, f"source pack physical package collision: {physical}")
        physical_names.add(physical)
    return result


def approval_module() -> Any:
    return load_direct_import_module("TEXTURE_RUNTIME_BINDING_APPROVAL")


def deployment_approval_module() -> Any:
    return load_direct_import_module("EXACT_DDS_RUNTIME_DEPLOYMENT_APPROVAL")


def load_approved_deployment_authority() -> dict[str, Any]:
    approval = approval_module()
    path = DEFAULT_EXACT_DDS_DEPLOYMENT
    require(path.is_file(), "approved exact-DDS deployment receipt is missing")
    tracked_digest = tracked_text_sha256(path)
    require(
        tracked_digest == approval.DEPLOYMENT_RECEIPT_TRACKED_TEXT_SHA256,
        "approved exact-DDS deployment tracked bytes changed",
    )
    before = path.read_bytes()
    document = read_json(path)
    require(path.read_bytes() == before, "exact-DDS deployment receipt changed while reading")
    require_exact_keys(
        document,
        {
            "schema", "formatVersion", "characterClass", "skillId", "inputSlot",
            "deploymentContract", "sourceEvidence", "assets", "recoveryBackup",
            "admission", "summary", "receiptSha256",
        },
        "exact-DDS deployment receipt",
    )
    require(
        document.get("schema")
        == "lostark.artist-31470-exact-dds-runtime-deployment-receipt"
        and document.get("characterClass") == "ARTIST"
        and type(document.get("skillId")) is int
        and document["skillId"] == 31470
        and document.get("inputSlot") == "F",
        "unsupported exact-DDS deployment receipt",
    )
    require_exact_integer(document.get("formatVersion"), 1, "deployment formatVersion")
    self_digest = require_sha256(document.get("receiptSha256"), "deployment receiptSha256")
    self_candidate = copy.deepcopy(document)
    self_candidate.pop("receiptSha256")
    require(
        self_digest == canonical_sha256(self_candidate)
        == approval.DEPLOYMENT_RECEIPT_SHA256,
        "exact-DDS deployment receipt self identity changed",
    )
    source = document.get("sourceEvidence")
    require(isinstance(source, dict), "deployment source evidence is missing")
    require(
        source.get("authorityCommit") == "fda3b5637847f9205915ad25ff02215424024b88"
        and source.get("authorityTree") == "2f00f00851ee93f498dd6c13d6a3055209d4d8c3"
        and source.get("implementationEvidence", {}).get("projectionSha256")
        == approval.DEPLOYMENT_IMPLEMENTATION_PROJECTION_SHA256,
        "exact-DDS deployment internal authority changed",
    )
    deployment_approval = deployment_approval_module()
    require(
        deployment_approval.APPROVED_RECEIPT_PROJECTION_SHA256
        == approval.DEPLOYMENT_APPROVAL_PROJECTION_SHA256
        and deployment_approval.receipt_projection_sha256(document)
        == approval.DEPLOYMENT_APPROVAL_PROJECTION_SHA256,
        "exact-DDS deployment independent approval changed",
    )
    deployment_approval.require_approved_receipt(document)
    require(
        strict_equal(
            document.get("admission"),
            {
                "transactionCommitted": True,
                "allFourRuntimeAssetsPostVerified": True,
                "sourceExactMaterialClaim": False,
                "rendererTextureSrvConsumerComplete": False,
                "r4Complete": False,
                "productReady": False,
                "blockers": [R4_BLOCKER],
            },
        ),
        "exact-DDS deployment admission changed",
    )
    require(
        strict_equal(
            document.get("summary"),
            {
                "requestedAssetCount": 4,
                "deployedAssetCount": 4,
                "postVerifiedAssetCount": 4,
                "runtimeAssetDeploymentAdmittedCount": 4,
                "recoveryBackupPayloadFileCount": 0,
                "recoveryAbsentTargetMarkerCount": 4,
                "sourceExactMaterialClaimCount": 0,
                "rendererConsumerReadyCount": 0,
                "productReadyCount": 0,
            },
        ),
        "exact-DDS deployment summary changed",
    )
    return {
        "relativePath": approval.DEPLOYMENT_RECEIPT_RELATIVE_PATH,
        "artifactAuthorityCommit": approval.DEPLOYMENT_ARTIFACT_COMMIT,
        "artifactAuthorityTree": approval.DEPLOYMENT_ARTIFACT_TREE,
        "gitBlob": approval.DEPLOYMENT_RECEIPT_GIT_BLOB,
        "trackedTextSha256": tracked_digest,
        "receiptSha256": self_digest,
        "approvalProjectionSha256": approval.DEPLOYMENT_APPROVAL_PROJECTION_SHA256,
        "implementationProjectionSha256": approval.DEPLOYMENT_IMPLEMENTATION_PROJECTION_SHA256,
        "document": document,
    }


def validate_deployment_rows(
    deployment: dict[str, Any],
    exact_dds_rows: dict[str, dict[str, Any]],
) -> dict[str, dict[str, Any]]:
    rows = deployment.get("assets")
    require(isinstance(rows, list) and len(rows) == EXPECTED_DEPLOYMENT_TEXTURES, "deployment row denominator changed")
    result: dict[str, dict[str, Any]] = {}
    runtime_ids: set[str] = set()
    for index, row in enumerate(rows):
        require_exact_keys(row, DEPLOYMENT_ROW_KEYS, f"deployment assets[{index}]")
        validate_row_digest(row, f"deployment assets[{index}]")
        logical = validate_logical_texture_path(row.get("logicalTexturePath"))
        require(
            logical in EXPECTED_MISSING_LOGICAL_PATHS and logical not in result,
            f"unexpected or duplicate deployment logical path: {logical}",
        )
        exact_row = exact_dds_rows.get(logical.casefold())
        exact_evidence = row.get("sourceExactDdsEvidence")
        require_exact_keys(exact_evidence, EXACT_DDS_EVIDENCE_KEYS, f"deployment[{logical}].exactDds")
        require(
            exact_row is not None
            and strict_equal(exact_evidence, exact_dds_projection(exact_row)),
            f"deployment exact-DDS evidence mismatch: {logical}",
        )
        asset_id = validate_runtime_asset_id(row.get("runtimeAssetId"))
        require(asset_id.casefold() not in runtime_ids, f"deployment runtime asset collision: {asset_id}")
        runtime_ids.add(asset_id.casefold())
        deployed_file = row.get("deployedFile")
        require_exact_keys(deployed_file, DEPLOYED_FILE_KEYS, f"deployment[{logical}].deployedFile")
        dds = exact_evidence["dds"]
        require(
            row.get("policy") == PROVISIONING_POLICY
            and row.get("deploymentStatus") == "COMMITTED_POST_VERIFIED"
            and asset_id == exact_evidence["fixtureAssetId"]
            and deployed_file.get("runtimeAssetId")
            == exact_evidence["fixtureAssetId"]
            and row.get("proposalId")
            == "material-texture-provisioning-"
            + hashlib.sha256(logical.encode("utf-8")).hexdigest()[:20]
            and row.get("textureResourceId")
            == "material-texture-resource-"
            + hashlib.sha256(logical.encode("utf-8")).hexdigest()[:20]
            and strict_equal(
                deployed_file,
                {
                    "runtimeAssetId": asset_id,
                    "byteCount": dds["byteCount"],
                    "rawSha256": dds["rawSha256"],
                    "pathCaseVerified": True,
                    "regularFileVerified": True,
                    "symlinkFreeVerified": True,
                    "postVerified": True,
                },
            )
            and row.get("sourceExactMaterialClaim") is False
            and row.get("runtimeAssetDeploymentAdmission") is True
            and row.get("rendererConsumerAdmission") is False
            and row.get("productAdmission") is False
            and row.get("blockers") == [R4_BLOCKER],
            f"deployment row contract changed: {logical}",
        )
        result[logical] = row
    require(set(result) == EXPECTED_MISSING_LOGICAL_PATHS, "deployment reverse coverage changed")
    return result


def deployment_projection(
    row: dict[str, Any],
    authority: dict[str, Any],
) -> dict[str, Any]:
    deployed_file = row["deployedFile"]
    return {
        "basis": DEPLOYMENT_BASIS,
        "artifactAuthorityCommit": authority["artifactAuthorityCommit"],
        "artifactAuthorityTree": authority["artifactAuthorityTree"],
        "receiptRelativePath": authority["relativePath"],
        "receiptGitBlob": authority["gitBlob"],
        "receiptTrackedTextSha256": authority["trackedTextSha256"],
        "receiptSha256": authority["receiptSha256"],
        "approvalProjectionSha256": authority["approvalProjectionSha256"],
        "implementationProjectionSha256": authority["implementationProjectionSha256"],
        "deploymentRowId": row["deploymentRowId"],
        "proposalId": row["proposalId"],
        "deploymentRowSha256": row["rowSha256"],
        "deploymentStatus": row["deploymentStatus"],
        "runtimeAssetId": row["runtimeAssetId"],
        "byteCount": deployed_file["byteCount"],
        "rawSha256": deployed_file["rawSha256"],
        "pathCaseVerified": deployed_file["pathCaseVerified"],
        "regularFileVerified": deployed_file["regularFileVerified"],
        "symlinkFreeVerified": deployed_file["symlinkFreeVerified"],
        "postVerified": deployed_file["postVerified"],
        "sourceExactMaterialClaim": row["sourceExactMaterialClaim"],
        "runtimeAssetDeploymentAdmission": row["runtimeAssetDeploymentAdmission"],
    }


def resolve_case_exact_runtime_file(
    runtime_root: Path,
    runtime_asset_id: str,
) -> Path:
    asset_id = validate_runtime_asset_id(runtime_asset_id)
    require(
        runtime_root.is_dir() and not runtime_root.is_symlink(),
        "runtime Resources authority root is missing or is a symlink",
    )
    current = runtime_root
    for index, part in enumerate(PurePosixPath(asset_id).parts):
        matches = [
            entry for entry in current.iterdir()
            if entry.name.casefold() == part.casefold()
        ]
        require(
            len(matches) == 1 and matches[0].name == part,
            f"runtime asset path case/collision mismatch: {asset_id}",
        )
        current = matches[0]
        require(
            not current.is_symlink(),
            f"runtime asset path contains a symlink: {asset_id}",
        )
        if index != len(PurePosixPath(asset_id).parts) - 1:
            require(current.is_dir(), f"runtime asset parent is not a directory: {asset_id}")
    require(current.is_file(), f"runtime asset is not a regular file: {asset_id}")
    return current


def verify_deployed_runtime_files(
    deployment_rows: dict[str, dict[str, Any]],
    runtime_root: Path | None = None,
) -> None:
    if runtime_root is None:
        runtime_root = DEFAULT_RUNTIME_RESOURCES
    require(
        set(deployment_rows) == EXPECTED_MISSING_LOGICAL_PATHS,
        "runtime deployment verification denominator changed",
    )
    verified: set[str] = set()
    for logical in sorted(deployment_rows):
        row = deployment_rows[logical]
        asset_id = row["runtimeAssetId"]
        require(asset_id.casefold() not in verified, f"runtime deployment alias: {asset_id}")
        verified.add(asset_id.casefold())
        path = resolve_case_exact_runtime_file(runtime_root, asset_id)
        deployed = row["deployedFile"]
        require(
            path.stat().st_size == deployed["byteCount"]
            and raw_file_sha256(path) == deployed["rawSha256"],
            f"deployed runtime payload identity changed: {logical}",
        )
    require(len(verified) == EXPECTED_DEPLOYMENT_TEXTURES, "runtime deployment file count changed")


def source_evidence(
    runtime_cook_path: Path,
    resource_export_path: Path,
    source_pack_path: Path,
    deployment_authority: dict[str, Any],
    tracked_authorities: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    approval = approval_module()
    evidence: dict[str, Any] = {
        "frozenMaterialPolicyCommit": FROZEN_MATERIAL_POLICY_COMMIT,
        "directImportClosure": direct_import_closure(),
        "exactDdsRuntimeDeploymentReceipt": {
            "relativePath": deployment_authority["relativePath"],
            "artifactAuthorityCommit": deployment_authority["artifactAuthorityCommit"],
            "artifactAuthorityTree": deployment_authority["artifactAuthorityTree"],
            "gitBlob": deployment_authority["gitBlob"],
            "trackedTextSha256": deployment_authority["trackedTextSha256"],
            "receiptSha256": deployment_authority["receiptSha256"],
            "approvalProjectionSha256": deployment_authority[
                "approvalProjectionSha256"
            ],
            "implementationProjectionSha256": deployment_authority[
                "implementationProjectionSha256"
            ],
            "hashRole": "FROZEN_GIT_ARTIFACT_AND_TRACKED_UTF8_EOL_CANONICAL",
        },
    }
    for evidence_id, authority in tracked_authorities.items():
        row = {
            "relativePath": authority["relativePath"],
            "trackedTextSha256": authority["trackedTextSha256"],
            "hashRole": "TRACKED_UTF8_EOL_CANONICAL",
        }
        self_digest = authority["selfDigest"]
        if self_digest is not None:
            row["selfDigest"] = self_digest
        evidence[evidence_id] = row
    external_rows = {
        "runtimeCookReceipt": (
            runtime_cook_path,
            approval.RUNTIME_COOK_BYTE_COUNT,
            approval.RUNTIME_COOK_RAW_SHA256,
        ),
        "resourceExportReceipt": (
            resource_export_path,
            approval.RESOURCE_EXPORT_BYTE_COUNT,
            approval.RESOURCE_EXPORT_RAW_SHA256,
        ),
        "sourcePackManifest": (
            source_pack_path,
            approval.SOURCE_PACK_MANIFEST_BYTE_COUNT,
            approval.SOURCE_PACK_MANIFEST_RAW_SHA256,
        ),
    }
    for evidence_id, (path, byte_count, digest) in external_rows.items():
        require(path.is_file(), f"external evidence is missing: {path}")
        require(path.stat().st_size == byte_count, f"external evidence byte count changed: {evidence_id}")
        require(raw_file_sha256(path) == digest, f"external raw evidence changed: {evidence_id}")
        evidence[evidence_id] = {
            "byteCount": byte_count,
            "rawSha256": digest,
            "hashRole": "EXTERNAL_RAW_BYTES",
        }
    return evidence


def validate_tracked_source_evidence(
    evidence: dict[str, Any],
    tracked_authorities: dict[str, dict[str, Any]],
    external_authorities: dict[str, dict[str, Any]],
    deployment_authority: dict[str, Any],
) -> None:
    require_exact_keys(
        evidence,
        {
            "frozenMaterialPolicyCommit", "directImportClosure", "materialPolicy",
            "typedMaterialContract", "materialRuntimeOracle",
            "materialSourceValueAcquisition", "resourceSourceManifest",
            "exactDdsRecoveryReceipt", "nativeV14Candidate", "runtimeCookReceipt",
            "resourceExportReceipt",
            "sourcePackManifest", "exactDdsRuntimeDeploymentReceipt",
        },
        "source evidence",
    )
    require(evidence.get("frozenMaterialPolicyCommit") == FROZEN_MATERIAL_POLICY_COMMIT, "frozen policy commit changed")
    validate_direct_import_closure(evidence.get("directImportClosure"))
    require(
        strict_equal(
            evidence.get("exactDdsRuntimeDeploymentReceipt"),
            {
                "relativePath": deployment_authority["relativePath"],
                "artifactAuthorityCommit": deployment_authority[
                    "artifactAuthorityCommit"
                ],
                "artifactAuthorityTree": deployment_authority["artifactAuthorityTree"],
                "gitBlob": deployment_authority["gitBlob"],
                "trackedTextSha256": deployment_authority["trackedTextSha256"],
                "receiptSha256": deployment_authority["receiptSha256"],
                "approvalProjectionSha256": deployment_authority[
                    "approvalProjectionSha256"
                ],
                "implementationProjectionSha256": deployment_authority[
                    "implementationProjectionSha256"
                ],
                "hashRole": "FROZEN_GIT_ARTIFACT_AND_TRACKED_UTF8_EOL_CANONICAL",
            },
        )
        and tracked_text_sha256(DEFAULT_EXACT_DDS_DEPLOYMENT)
        == deployment_authority["trackedTextSha256"],
        "exact-DDS deployment source evidence changed",
    )
    require(
        set(tracked_authorities)
        == {
            "materialPolicy", "typedMaterialContract", "materialRuntimeOracle",
            "materialSourceValueAcquisition", "resourceSourceManifest",
            "exactDdsRecoveryReceipt", "nativeV14Candidate",
        },
        "approved tracked authority set changed",
    )
    for evidence_id, authority in tracked_authorities.items():
        row = evidence.get(evidence_id)
        require(isinstance(row, dict), f"missing tracked evidence: {evidence_id}")
        expected_row = {
            "relativePath": authority["relativePath"],
            "trackedTextSha256": authority["trackedTextSha256"],
            "hashRole": "TRACKED_UTF8_EOL_CANONICAL",
        }
        self_digest = authority["selfDigest"]
        if self_digest is not None:
            expected_row["selfDigest"] = self_digest
        require(strict_equal(row, expected_row), f"tracked source evidence changed: {evidence_id}")
    require(
        set(external_authorities)
        == {"runtimeCookReceipt", "resourceExportReceipt", "sourcePackManifest"},
        "approved external authority set changed",
    )
    for evidence_id, authority in external_authorities.items():
        require(
            strict_equal(
                evidence.get(evidence_id),
                {
                    "byteCount": authority["byteCount"],
                    "rawSha256": authority["rawSha256"],
                    "hashRole": "EXTERNAL_RAW_BYTES",
                },
            ),
            f"external source evidence changed: {evidence_id}",
        )


def resource_manifest_projection(row: dict[str, Any]) -> dict[str, Any]:
    return {
        "logicalPackage": row["logicalPackage"],
        "physicalPackage": row["physicalPackage"],
        "roles": copy.deepcopy(row["roles"]),
        "skillIds": copy.deepcopy(row["skillIds"]),
        "sourceSystems": copy.deepcopy(row["sourceSystems"]),
        "resolutionStatus": row["resolutionStatus"],
        "sourceRowSha256": canonical_sha256(row),
    }


def source_package_projection(row: dict[str, Any]) -> dict[str, Any]:
    require(row.get("resolved") is True, f"source package is unresolved: {row.get('logicalPackage')}")
    relative_path = validate_relative_path(row.get("relativePath"), "source package relativePath")
    require(type(row.get("byteSize")) is int and row["byteSize"] > 0, "source package byte size invalid")
    return {
        "logicalPackage": row["logicalPackage"],
        "physicalPackage": row["physicalPackage"],
        "relativePath": relative_path,
        "byteSize": row["byteSize"],
        "rawSha256": require_sha256(row.get("sha256"), "source package SHA"),
        "sourceRowSha256": canonical_sha256(row),
    }


def cook_projection(row: dict[str, Any]) -> dict[str, Any]:
    return {
        "sourceAssetPath": row["sourceAssetPath"],
        "sourceFile": row["sourceFile"],
        "runtimeAssetId": row["runtimeAssetId"],
        "byteSize": row["byteSize"],
        "rawSha256": row["sha256"],
        "sourceRowSha256": canonical_sha256(row),
    }


def export_projection(row: dict[str, Any], output: dict[str, Any]) -> dict[str, Any]:
    return {
        "sourceAssetPath": row["sourceAssetPath"],
        "logicalPackage": row["logicalPackage"],
        "outputRelativePath": output["relativePath"],
        "byteSize": output["byteSize"],
        "rawSha256": output["sha256"],
        "sourceRowSha256": canonical_sha256(row),
        "outputRowSha256": canonical_sha256(output),
    }


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


def build_texture_resources(
    logical_paths: set[str],
    resource_rows: dict[str, dict[str, Any]],
    source_packages: dict[str, dict[str, Any]],
    cook_rows: dict[str, dict[str, Any]],
    export_rows: dict[str, dict[str, Any]],
    exact_dds_rows: dict[str, dict[str, Any]],
    deployment_rows: dict[str, dict[str, Any]],
    deployment_authority: dict[str, Any],
    candidate: dict[str, Any],
) -> list[dict[str, Any]]:
    resources: list[dict[str, Any]] = []
    used_asset_ids: dict[str, str] = {}
    for logical_path in sorted(logical_paths):
        validate_logical_texture_path(logical_path)
        manifest_row = resource_rows.get(logical_path.casefold())
        dds_row = exact_dds_rows.get(logical_path.casefold())
        if manifest_row is not None:
            require(manifest_row.get("resolutionStatus") == "RESOLVED_SOURCE_PACKAGE", f"logical texture source package unresolved: {logical_path}")
            logical_package = manifest_row.get("logicalPackage")
            physical_package = manifest_row.get("physicalPackage")
        else:
            require(
                logical_path in EXPECTED_MISSING_LOGICAL_PATHS and dds_row is not None,
                f"resource manifest is missing unapproved logical texture: {logical_path}",
            )
            logical_package = dds_row["sourceTexture2D"]["logicalPackage"]
            physical_package = dds_row["sourceTexture2D"]["physicalPackage"]
        package_row = source_packages.get(folded(logical_package))
        if package_row is not None:
            require(
                folded(package_row.get("physicalPackage")) == folded(physical_package),
                f"source package physical identity mismatch: {logical_path}",
            )
        source_package = source_package_projection(package_row) if package_row is not None else None
        source_package_blockers = (
            [] if package_row is not None else ["SOURCE_PACKAGE_NOT_IN_PINNED_SOURCE_PACK_MANIFEST"]
        )
        texture_id = "material-texture-resource-" + hashlib.sha256(logical_path.encode("utf-8")).hexdigest()[:20]
        cook_row = cook_rows.get(logical_path.casefold())
        if cook_row is not None:
            require(cook_row.get("role") == "texture", f"runtime cook role mismatch: {logical_path}")
            require(cook_row.get("sourceAssetPath") == logical_path, f"runtime cook full-path case mismatch: {logical_path}")
            asset_id = validate_runtime_asset_id(cook_row.get("runtimeAssetId"))
            asset_folded = asset_id.casefold()
            require(asset_folded not in used_asset_ids, f"runtime asset many-to-one mapping: {asset_id}")
            used_asset_ids[asset_folded] = logical_path
            export_row = export_rows.get(logical_path.casefold())
            require(export_row is not None and export_row.get("sourceAssetPath") == logical_path, f"resource export full-path mismatch: {logical_path}")
            outputs = export_row.get("outputs")
            require(isinstance(outputs, list), f"resource export outputs missing: {logical_path}")
            matching_outputs = [row for row in outputs if row.get("relativePath") == cook_row.get("sourceFile")]
            require(len(matching_outputs) == 1, f"cook/export output join is not unique: {logical_path}")
            output = matching_outputs[0]
            require(
                output.get("byteSize") == cook_row.get("byteSize")
                and output.get("sha256") == cook_row.get("sha256"),
                f"cook/export payload identity mismatch: {logical_path}",
            )
            row = {
                "textureResourceId": texture_id,
                "logicalTexturePath": logical_path,
                "status": RESOLVED_STATUS,
                "runtimeAssetId": asset_id,
                "sourceResourceManifest": resource_manifest_projection(manifest_row),
                "sourcePackage": source_package,
                "sourceEvidenceBlockers": source_package_blockers,
                "runtimeCookEvidence": cook_projection(cook_row),
                "resourceExportEvidence": export_projection(export_row, output),
                "exactDdsEvidence": None,
                "deploymentEvidence": None,
                "candidateObservations": candidate_observations(candidate, asset_id),
                "blockers": [],
                "provisioningProposalId": None,
                "runtimeAssetAdmission": True,
                "sourceExact": False,
            }
        else:
            require(logical_path in EXPECTED_MISSING_LOGICAL_PATHS, f"unexpected runtime cook gap: {logical_path}")
            require(dds_row is not None, f"runtime cook gap lacks exact DDS evidence: {logical_path}")
            deployment_row = deployment_rows.get(logical_path)
            require(
                deployment_row is not None,
                f"runtime cook gap lacks exact deployment evidence: {logical_path}",
            )
            proposal_id = "material-texture-provisioning-" + hashlib.sha256(logical_path.encode("utf-8")).hexdigest()[:20]
            asset_id = validate_runtime_asset_id(deployment_row["runtimeAssetId"])
            asset_folded = asset_id.casefold()
            require(asset_folded not in used_asset_ids, f"runtime asset many-to-one mapping: {asset_id}")
            used_asset_ids[asset_folded] = logical_path
            row = {
                "textureResourceId": texture_id,
                "logicalTexturePath": logical_path,
                "status": DEPLOYED_STATUS,
                "runtimeAssetId": asset_id,
                "sourceResourceManifest": None,
                "sourcePackage": source_package,
                "sourceEvidenceBlockers": source_package_blockers,
                "runtimeCookEvidence": None,
                "resourceExportEvidence": None,
                "exactDdsEvidence": exact_dds_projection(dds_row),
                "deploymentEvidence": deployment_projection(
                    deployment_row,
                    deployment_authority,
                ),
                "candidateObservations": candidate_observations(candidate, asset_id),
                "blockers": [],
                "provisioningProposalId": proposal_id,
                "runtimeAssetAdmission": True,
                "sourceExact": False,
            }
        resources.append(row_with_digest(row))
    require(len(resources) == EXPECTED_UNIQUE_TEXTURES, "unique texture denominator changed")
    return resources


def build_provisioning_proposals(
    resources: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for resource in resources:
        if resource["status"] != DEPLOYED_STATUS:
            continue
        exact = resource["exactDdsEvidence"]
        target = validate_runtime_asset_id(exact["fixtureAssetId"])
        row = {
            "proposalId": resource["provisioningProposalId"],
            "textureResourceId": resource["textureResourceId"],
            "logicalTexturePath": resource["logicalTexturePath"],
            "policy": PROVISIONING_POLICY,
            "proposedRuntimeAssetId": target,
            "sourceExactDdsEvidence": copy.deepcopy(exact),
            "deploymentStatus": DEPLOYMENT_COMPLETE_STATUS,
            "requiredReceipt": approval_module().DEPLOYMENT_RECEIPT_RELATIVE_PATH,
            "deploymentEvidence": copy.deepcopy(resource["deploymentEvidence"]),
            "sourceExact": False,
            "runtimeAssetAdmission": True,
            "productAdmission": False,
        }
        rows.append(row_with_digest(row))
    require(len(rows) == EXPECTED_DEPLOYMENT_TEXTURES, "provisioning proposal denominator changed")
    return rows


def contract_texture_fields(contract: dict[str, Any]) -> dict[str, dict[str, Any]]:
    fields: dict[str, dict[str, Any]] = {}
    for recipe in contract["materialRecipes"]:
        recipe_id = recipe["recipeId"]
        for section, section_rows in recipe["inputs"].items():
            if not isinstance(section_rows, list):
                continue
            for section_index, row in enumerate(section_rows):
                field_id = row.get("fieldId")
                require(isinstance(field_id, str) and field_id not in fields, f"duplicate contract field: {field_id}")
                fields[field_id] = {
                    "recipeId": recipe_id,
                    "section": section,
                    "sectionIndex": section_index,
                    "row": row,
                }
    return fields


def expected_recipe_occurrences(contract: dict[str, Any]) -> dict[str, list[str]]:
    result: dict[str, list[str]] = {}
    for occurrence in contract["occurrences"]:
        result.setdefault(occurrence["materialRecipeId"], []).append(occurrence["occurrenceId"])
    return result


def build_material_bindings(
    policy: dict[str, Any],
    contract: dict[str, Any],
    resources: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    resource_by_logical = {row["logicalTexturePath"]: row for row in resources}
    fields = contract_texture_fields(contract)
    occurrence_map = expected_recipe_occurrences(contract)
    bindings: list[dict[str, Any]] = []
    for binding_order, policy_row in enumerate(policy["samplerPolicies"]):
        field_id = policy_row["fieldId"]
        field = fields.get(field_id)
        require(field is not None, f"sampler policy field is absent from contract: {field_id}")
        owner = policy_row["bindingOriginAndOwner"]
        require(
            field["recipeId"] == policy_row["materialRecipeId"]
            and owner.get("evidenceOwnerRecipeId") == field["recipeId"]
            and owner.get("sourceSection") == field["section"]
            and type(owner.get("sourceSectionIndex")) is int
            and owner["sourceSectionIndex"] == field["sectionIndex"],
            f"sampler policy owner mismatch: {policy_row['policyRowId']}",
        )
        logical_path = validate_logical_texture_path(policy_row["logicalTexturePath"])
        require(field["row"].get("value") == logical_path, f"sampler policy logical path is not contract-derived: {field_id}")
        occurrences = occurrence_map.get(field["recipeId"], [])
        require(policy_row["materialOccurrenceIds"] == occurrences, f"sampler occurrence ownership mismatch: {field_id}")
        resource = resource_by_logical.get(logical_path)
        require(resource is not None, f"sampler logical texture lacks resource row: {logical_path}")
        identity = {
            "samplerPolicyRowId": policy_row["policyRowId"],
            "recipeId": field["recipeId"],
            "materialInputFieldId": field_id,
            "logicalTexturePath": logical_path,
            "materialOccurrenceIds": copy.deepcopy(occurrences),
        }
        binding_id = "material-texture-binding-" + canonical_sha256(identity)[:20]
        descriptor = copy.deepcopy(policy_row["selectedDescriptor"])
        srv_identity = {
            "sRgb": descriptor["sRgb"],
            "srvColorSpace": descriptor["srvColorSpace"],
            "lodGroup": descriptor["lodGroup"],
        }
        row = {
            "bindingId": binding_id,
            "bindingOrder": binding_order,
            "samplerPolicyRowId": policy_row["policyRowId"],
            "samplerPolicyOrder": policy_row["policyOrder"],
            "recipeId": field["recipeId"],
            "materialInputFieldId": field_id,
            "materialOccurrenceIds": copy.deepcopy(occurrences),
            "logicalTexturePath": logical_path,
            "textureResourceId": resource["textureResourceId"],
            "status": resource["status"],
            "runtimeAssetId": resource["runtimeAssetId"],
            "samplerDescriptor": descriptor,
            "samplerDescriptorSha256": canonical_sha256(descriptor),
            "srvIdentity": srv_identity,
            "srvIdentitySha256": canonical_sha256(srv_identity),
            "bindingOriginAndOwner": copy.deepcopy(owner),
            "policySourceRowSha256": policy_row["rowSha256"],
            "runtimeAssetAdmission": resource["runtimeAssetAdmission"],
            "rendererConsumerAdmission": False,
            "productAdmission": False,
            "sourceExact": False,
        }
        bindings.append(row_with_digest(row))
    require(len(bindings) == EXPECTED_POLICY_ROWS, "sampler policy denominator changed")
    return bindings


def build_receipt(
    policy: dict[str, Any],
    contract: dict[str, Any],
    resource_manifest: dict[str, Any],
    exact_dds: dict[str, Any],
    candidate: dict[str, Any],
    runtime_cook: dict[str, Any],
    resource_export: dict[str, Any],
    source_pack: dict[str, Any],
    *,
    policy_path: Path = DEFAULT_POLICY,
    contract_path: Path = DEFAULT_CONTRACT,
    resource_manifest_path: Path = DEFAULT_RESOURCE_MANIFEST,
    exact_dds_path: Path = DEFAULT_EXACT_DDS,
    candidate_path: Path = DEFAULT_CANDIDATE,
    runtime_cook_path: Path = DEFAULT_RUNTIME_COOK,
    resource_export_path: Path = DEFAULT_RESOURCE_EXPORT,
    source_pack_path: Path = DEFAULT_SOURCE_PACK,
    runtime_oracle: dict[str, Any] | None = None,
    acquisition: dict[str, Any] | None = None,
    deployment_receipt: dict[str, Any] | None = None,
) -> dict[str, Any]:
    (
        policy,
        contract,
        runtime_oracle,
        acquisition,
        resource_manifest,
        exact_dds,
        candidate,
        tracked_authorities,
    ) = bind_supplied_tracked_inputs(
        policy,
        contract,
        runtime_oracle,
        acquisition,
        resource_manifest,
        exact_dds,
        candidate,
    )
    exact_dds_rows = validate_upstream_documents(policy, contract, runtime_oracle, acquisition, exact_dds)
    deployment_authority = load_approved_deployment_authority()
    approved_deployment = deployment_authority["document"]
    if deployment_receipt is not None:
        require(
            strict_equal(deployment_receipt, approved_deployment),
            "supplied deployment receipt differs from approved tracked authority",
        )
    deployment_rows = validate_deployment_rows(
        approved_deployment,
        exact_dds_rows,
    )
    verify_deployed_runtime_files(deployment_rows)
    resource_rows = validate_resource_manifest(resource_manifest)
    validate_candidate(candidate)
    cook_rows = validate_runtime_cook(runtime_cook)
    export_rows = validate_resource_export(resource_export)
    source_packages = validate_source_pack(source_pack)
    logical_paths = {validate_logical_texture_path(row["logicalTexturePath"]) for row in policy["samplerPolicies"]}
    require(len(logical_paths) == EXPECTED_UNIQUE_TEXTURES, "logical texture denominator changed")
    resources = build_texture_resources(
        logical_paths,
        resource_rows,
        source_packages,
        cook_rows,
        export_rows,
        exact_dds_rows,
        deployment_rows,
        deployment_authority,
        candidate,
    )
    actual_deployed = {
        row["logicalTexturePath"]
        for row in resources
        if row["status"] == DEPLOYED_STATUS
    }
    require(
        actual_deployed == EXPECTED_MISSING_LOGICAL_PATHS,
        "exact-DDS deployment logical texture set changed",
    )
    proposals = build_provisioning_proposals(resources)
    bindings = build_material_bindings(policy, contract, resources)
    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "bindingContract": {
            "contractId": BINDING_CONTRACT_ID,
            "contractVersion": 2,
            "logicalJoinKey": "FULL_CASE_SENSITIVE_LOGICAL_TEXTURE_PATH",
            "runtimeAssetAuthority": (
                "FROZEN_ARTIST_RUNTIME_COOK_OR_EXACT_DDS_DEPLOYMENT_RECEIPT"
            ),
            "basenameInferenceAllowed": False,
            "provisioningPolicy": PROVISIONING_POLICY,
            "sourceExact": False,
        },
        "sourceEvidence": source_evidence(
            runtime_cook_path,
            resource_export_path,
            source_pack_path,
            deployment_authority,
            tracked_authorities,
        ),
        "textureResources": resources,
        "materialTextureBindings": bindings,
        "provisioningProposals": proposals,
        "admission": {
            "bindingReceipt": {"ready": True, "rowCount": EXPECTED_POLICY_ROWS},
            "resolvedRuntimeAssets": {"ready": True, "rowCount": EXPECTED_RESOLVED_ROWS},
            "completeRuntimeBinding": {"ready": True, "blockers": []},
            "rendererConsumer": {"ready": False, "blockers": [R4_BLOCKER]},
            "product": False,
        },
        "summary": {
            "samplerPolicyRowCount": len(bindings),
            "uniqueLogicalTextureCount": len(resources),
            "resolvedBindingRowCount": sum(
                row["status"] in {RESOLVED_STATUS, DEPLOYED_STATUS}
                for row in bindings
            ),
            "unresolvedBindingRowCount": 0,
            "runtimeCookBindingRowCount": sum(
                row["status"] == RESOLVED_STATUS for row in bindings
            ),
            "deploymentBindingRowCount": sum(
                row["status"] == DEPLOYED_STATUS for row in bindings
            ),
            "resolvedUniqueTextureCount": len(resources),
            "unresolvedUniqueTextureCount": 0,
            "runtimeCookUniqueTextureCount": sum(
                row["status"] == RESOLVED_STATUS for row in resources
            ),
            "deploymentUniqueTextureCount": sum(
                row["status"] == DEPLOYED_STATUS for row in resources
            ),
            "sourcePackageBoundUniqueTextureCount": sum(row["sourcePackage"] is not None for row in resources),
            "sourcePackageUnboundUniqueTextureCount": sum(row["sourcePackage"] is None for row in resources),
            "materialOccurrenceLinkCount": sum(len(row["materialOccurrenceIds"]) for row in bindings),
            "completedProvisioningProposalCount": len(proposals),
            "sourceExactBindingRowCount": 0,
            "rendererReadyBindingRowCount": 0,
            "productReadyBindingRowCount": 0,
        },
    }
    receipt["receiptSha256"] = canonical_sha256(receipt)
    validate_receipt(
        receipt,
        policy,
        contract,
        resource_manifest,
        exact_dds,
        candidate,
        policy_path=policy_path,
        contract_path=contract_path,
        resource_manifest_path=resource_manifest_path,
        exact_dds_path=exact_dds_path,
        candidate_path=candidate_path,
        runtime_cook=runtime_cook,
        resource_export=resource_export,
        source_pack=source_pack,
        deployment_receipt=approved_deployment,
        require_approval=False,
    )
    return receipt


def validate_receipt(
    receipt: dict[str, Any],
    policy: dict[str, Any],
    contract: dict[str, Any],
    resource_manifest: dict[str, Any],
    exact_dds: dict[str, Any],
    candidate: dict[str, Any],
    *,
    policy_path: Path = DEFAULT_POLICY,
    contract_path: Path = DEFAULT_CONTRACT,
    resource_manifest_path: Path = DEFAULT_RESOURCE_MANIFEST,
    exact_dds_path: Path = DEFAULT_EXACT_DDS,
    candidate_path: Path = DEFAULT_CANDIDATE,
    runtime_oracle: dict[str, Any] | None = None,
    acquisition: dict[str, Any] | None = None,
    runtime_cook: dict[str, Any] | None = None,
    resource_export: dict[str, Any] | None = None,
    source_pack: dict[str, Any] | None = None,
    deployment_receipt: dict[str, Any] | None = None,
    require_approval: bool = True,
) -> None:
    require(
        set(receipt)
        == {
            "schema", "formatVersion", "characterClass", "skillId", "inputSlot",
            "bindingContract", "sourceEvidence", "textureResources",
            "materialTextureBindings", "provisioningProposals", "admission",
            "summary", "receiptSha256",
        },
        "binding receipt root schema changed",
    )
    require(
        receipt.get("schema") == SCHEMA
        and receipt.get("characterClass") == "ARTIST"
        and type(receipt.get("skillId")) is int
        and receipt["skillId"] == 31470
        and receipt.get("inputSlot") == "F",
        "unsupported Material texture binding receipt",
    )
    require_exact_integer(receipt.get("formatVersion"), FORMAT_VERSION, "binding receipt formatVersion")
    validate_finite_tree(receipt)
    digest = require_sha256(receipt.get("receiptSha256"), "receiptSha256")
    candidate_receipt = copy.deepcopy(receipt)
    candidate_receipt.pop("receiptSha256")
    require(digest == canonical_sha256(candidate_receipt), "binding receipt self digest mismatch")
    (
        policy,
        contract,
        runtime_oracle,
        acquisition,
        resource_manifest,
        exact_dds,
        candidate,
        tracked_authorities,
    ) = bind_supplied_tracked_inputs(
        policy,
        contract,
        runtime_oracle,
        acquisition,
        resource_manifest,
        exact_dds,
        candidate,
    )
    external_authorities = load_approved_external_authorities()
    approved_runtime_cook = external_authorities["runtimeCookReceipt"]["document"]
    approved_resource_export = external_authorities["resourceExportReceipt"]["document"]
    approved_source_pack = external_authorities["sourcePackManifest"]["document"]
    deployment_authority = load_approved_deployment_authority()
    approved_deployment = deployment_authority["document"]
    for label, supplied, approved in (
        ("runtime cook", runtime_cook, approved_runtime_cook),
        ("resource export", resource_export, approved_resource_export),
        ("source pack", source_pack, approved_source_pack),
    ):
        if supplied is not None:
            require(
                strict_equal(supplied, approved),
                f"supplied {label} object differs from approved external authority",
            )
    if deployment_receipt is not None:
        require(
            strict_equal(deployment_receipt, approved_deployment),
            "supplied deployment receipt differs from approved tracked authority",
        )
    exact_rows = validate_upstream_documents(policy, contract, runtime_oracle, acquisition, exact_dds)
    deployment_rows = validate_deployment_rows(approved_deployment, exact_rows)
    verify_deployed_runtime_files(deployment_rows)
    manifest_rows = validate_resource_manifest(resource_manifest)
    validate_candidate(candidate)
    validate_tracked_source_evidence(
        receipt["sourceEvidence"],
        tracked_authorities,
        external_authorities,
        deployment_authority,
    )
    require(
        strict_equal(
            receipt["bindingContract"],
            {
            "contractId": BINDING_CONTRACT_ID,
            "contractVersion": 2,
            "logicalJoinKey": "FULL_CASE_SENSITIVE_LOGICAL_TEXTURE_PATH",
            "runtimeAssetAuthority": (
                "FROZEN_ARTIST_RUNTIME_COOK_OR_EXACT_DDS_DEPLOYMENT_RECEIPT"
            ),
            "basenameInferenceAllowed": False,
            "provisioningPolicy": PROVISIONING_POLICY,
            "sourceExact": False,
            },
        ),
        "binding contract changed",
    )
    resources = receipt["textureResources"]
    require(isinstance(resources, list) and len(resources) == EXPECTED_UNIQUE_TEXTURES, "texture resource denominator changed")
    require(
        [row.get("logicalTexturePath") for row in resources]
        == sorted(row.get("logicalTexturePath") for row in resources),
        "texture resources are not in stable logical-path order",
    )
    resource_by_logical: dict[str, dict[str, Any]] = {}
    resource_ids: set[str] = set()
    asset_ids: set[str] = set()
    for row in resources:
        require_exact_keys(row, TEXTURE_RESOURCE_KEYS, "texture resource")
        validate_row_digest(row, f"texture resource {row.get('logicalTexturePath')}")
        logical = validate_logical_texture_path(row.get("logicalTexturePath"))
        require(logical not in resource_by_logical, f"duplicate texture resource: {logical}")
        resource_by_logical[logical] = row
        expected_id = "material-texture-resource-" + hashlib.sha256(logical.encode("utf-8")).hexdigest()[:20]
        require(row.get("textureResourceId") == expected_id and expected_id not in resource_ids, f"texture resource ID mismatch: {logical}")
        resource_ids.add(expected_id)
        manifest_row = manifest_rows.get(logical.casefold())
        if manifest_row is not None:
            require_exact_keys(
                row.get("sourceResourceManifest"),
                RESOURCE_MANIFEST_EVIDENCE_KEYS,
                f"{logical}.sourceResourceManifest",
            )
            require(
                strict_equal(row.get("sourceResourceManifest"), resource_manifest_projection(manifest_row)),
                f"resource manifest evidence mismatch: {logical}",
            )
            expected_logical_package = manifest_row.get("logicalPackage")
            expected_physical_package = manifest_row.get("physicalPackage")
        else:
            exact_for_source = exact_rows.get(logical.casefold())
            require(
                logical in EXPECTED_MISSING_LOGICAL_PATHS
                and exact_for_source is not None
                and row.get("sourceResourceManifest") is None,
                f"resource manifest gap is not exact-DDS-authorized: {logical}",
            )
            expected_logical_package = exact_for_source["sourceTexture2D"]["logicalPackage"]
            expected_physical_package = exact_for_source["sourceTexture2D"]["physicalPackage"]
        package = row.get("sourcePackage")
        if package is None:
            require(
                expected_logical_package.casefold() == "wp_mn_lrcn_01"
                and row.get("sourceEvidenceBlockers")
                == ["SOURCE_PACKAGE_NOT_IN_PINNED_SOURCE_PACK_MANIFEST"],
                f"unexpected source package evidence gap: {logical}",
            )
        else:
            require_exact_keys(package, SOURCE_PACKAGE_EVIDENCE_KEYS, f"{logical}.sourcePackage")
            require(
                folded(package.get("logicalPackage")) == folded(expected_logical_package)
                and folded(package.get("physicalPackage")) == folded(expected_physical_package),
                f"source package identity mismatch: {logical}",
            )
            validate_relative_path(package.get("relativePath"), f"{logical}.sourcePackage.relativePath")
            require(type(package.get("byteSize")) is int and package["byteSize"] > 0, f"source package byte size invalid: {logical}")
            require_sha256(package.get("rawSha256"), f"{logical}.sourcePackage.rawSha256")
            require_sha256(package.get("sourceRowSha256"), f"{logical}.sourcePackage.sourceRowSha256")
            require(row.get("sourceEvidenceBlockers") == [], f"bound source package has blockers: {logical}")
        status = row.get("status")
        require(status in {RESOLVED_STATUS, DEPLOYED_STATUS}, f"invalid texture resource status: {logical}")
        require(row.get("sourceExact") is False, f"texture binding was promoted to source exact: {logical}")
        if status == RESOLVED_STATUS:
            asset_id = validate_runtime_asset_id(row.get("runtimeAssetId"))
            require(asset_id.casefold() not in asset_ids, f"runtime asset ID collision: {asset_id}")
            asset_ids.add(asset_id.casefold())
            cook = row.get("runtimeCookEvidence")
            export = row.get("resourceExportEvidence")
            require_exact_keys(cook, COOK_EVIDENCE_KEYS, f"{logical}.runtimeCookEvidence")
            require_exact_keys(export, EXPORT_EVIDENCE_KEYS, f"{logical}.resourceExportEvidence")
            require(
                cook.get("sourceAssetPath") == logical
                and export.get("sourceAssetPath") == logical
                and cook.get("runtimeAssetId") == asset_id
                and cook.get("sourceFile") == export.get("outputRelativePath")
                and cook.get("byteSize") == export.get("byteSize")
                and cook.get("rawSha256") == export.get("rawSha256"),
                f"resolved cook/export join mismatch: {logical}",
            )
            validate_relative_path(cook.get("sourceFile"), f"{logical}.cook.sourceFile")
            require(type(cook.get("byteSize")) is int and cook["byteSize"] > 0, f"resolved byte size invalid: {logical}")
            require(type(export.get("byteSize")) is int and export["byteSize"] > 0, f"export byte size invalid: {logical}")
            for evidence_row, name in ((cook, "cook"), (export, "export")):
                require_sha256(evidence_row.get("rawSha256"), f"{logical}.{name}.rawSha256")
                require_sha256(evidence_row.get("sourceRowSha256"), f"{logical}.{name}.sourceRowSha256")
            require_sha256(export.get("outputRowSha256"), f"{logical}.export.outputRowSha256")
            require(
                row.get("exactDdsEvidence") is None
                and row.get("deploymentEvidence") is None
                and row.get("blockers") == [],
                f"resolved cook row retains alternate evidence/blockers: {logical}",
            )
            require(row.get("provisioningProposalId") is None and row.get("runtimeAssetAdmission") is True, f"resolved admission mismatch: {logical}")
            observations = row.get("candidateObservations")
            require(isinstance(observations, list), f"candidate observations must be a list: {logical}")
            for observation in observations:
                require_exact_keys(observation, CANDIDATE_OBSERVATION_KEYS, f"{logical}.candidateObservation")
            require(
                strict_equal(observations, candidate_observations(candidate, asset_id)),
                f"candidate observation mismatch: {logical}",
            )
        else:
            require(logical in EXPECTED_MISSING_LOGICAL_PATHS, f"unexpected deployed texture: {logical}")
            asset_id = validate_runtime_asset_id(row.get("runtimeAssetId"))
            require(asset_id.casefold() not in asset_ids, f"runtime asset ID collision: {asset_id}")
            asset_ids.add(asset_id.casefold())
            require(
                row.get("runtimeCookEvidence") is None
                and row.get("resourceExportEvidence") is None,
                f"deployed texture has cook/export evidence: {logical}",
            )
            exact = exact_rows.get(logical.casefold())
            exact_evidence = row.get("exactDdsEvidence")
            require_exact_keys(exact_evidence, EXACT_DDS_EVIDENCE_KEYS, f"{logical}.exactDdsEvidence")
            require_exact_keys(
                exact_evidence.get("sourceTexture2D"),
                EXACT_DDS_TEXTURE_KEYS,
                f"{logical}.exactDdsEvidence.sourceTexture2D",
            )
            require_exact_keys(
                exact_evidence.get("dds"),
                EXACT_DDS_PAYLOAD_KEYS,
                f"{logical}.exactDdsEvidence.dds",
            )
            require(
                exact is not None and strict_equal(exact_evidence, exact_dds_projection(exact)),
                f"exact DDS evidence mismatch: {logical}",
            )
            deployment_row = deployment_rows.get(logical)
            deployment_evidence = row.get("deploymentEvidence")
            require_exact_keys(
                deployment_evidence,
                DEPLOYMENT_EVIDENCE_KEYS,
                f"{logical}.deploymentEvidence",
            )
            require(
                deployment_row is not None
                and strict_equal(
                    deployment_evidence,
                    deployment_projection(deployment_row, deployment_authority),
                )
                and deployment_evidence["runtimeAssetId"] == asset_id,
                f"exact deployment evidence mismatch: {logical}",
            )
            proposal_id = "material-texture-provisioning-" + hashlib.sha256(logical.encode("utf-8")).hexdigest()[:20]
            observations = row.get("candidateObservations")
            require(isinstance(observations, list), f"candidate observations must be a list: {logical}")
            for observation in observations:
                require_exact_keys(observation, CANDIDATE_OBSERVATION_KEYS, f"{logical}.candidateObservation")
            require(
                strict_equal(observations, candidate_observations(candidate, asset_id))
                and row.get("blockers") == []
                and row.get("provisioningProposalId") == proposal_id
                and row.get("runtimeAssetAdmission") is True,
                f"deployed admission boundary changed: {logical}",
            )
    require(set(resource_by_logical) == {row["logicalTexturePath"] for row in policy["samplerPolicies"]}, "texture resource reverse coverage changed")
    expected_resources = build_texture_resources(
        {row["logicalTexturePath"] for row in policy["samplerPolicies"]},
        manifest_rows,
        validate_source_pack(approved_source_pack),
        validate_runtime_cook(approved_runtime_cook),
        validate_resource_export(approved_resource_export),
        exact_rows,
        deployment_rows,
        deployment_authority,
        candidate,
    )
    require(
        strict_equal(resources, expected_resources),
        "texture resources are not pinned external-evidence-derived",
    )
    proposals = receipt["provisioningProposals"]
    expected_proposals = build_provisioning_proposals(resources)
    require(isinstance(proposals, list), "provisioning proposals must be a list")
    for proposal in proposals:
        require_exact_keys(proposal, PROVISIONING_PROPOSAL_KEYS, "provisioning proposal")
        exact_proposal = proposal.get("sourceExactDdsEvidence")
        require_exact_keys(exact_proposal, EXACT_DDS_EVIDENCE_KEYS, "proposal exact DDS evidence")
        require_exact_keys(exact_proposal.get("sourceTexture2D"), EXACT_DDS_TEXTURE_KEYS, "proposal exact DDS texture")
        require_exact_keys(exact_proposal.get("dds"), EXACT_DDS_PAYLOAD_KEYS, "proposal exact DDS payload")
        require_exact_keys(
            proposal.get("deploymentEvidence"),
            DEPLOYMENT_EVIDENCE_KEYS,
            "proposal deployment evidence",
        )
        validate_row_digest(proposal, f"proposal {proposal.get('proposalId')}")
    require(
        strict_equal(proposals, expected_proposals),
        "provisioning proposals are not exact-DDS-derived",
    )
    expected_bindings = build_material_bindings(policy, contract, resources)
    bindings = receipt["materialTextureBindings"]
    require(isinstance(bindings, list), "Material texture bindings must be a list")
    for binding in bindings:
        require_exact_keys(binding, MATERIAL_BINDING_KEYS, "Material texture binding")
        validate_row_digest(binding, f"binding {binding.get('bindingId')}")
        validate_runtime_asset_id(binding["runtimeAssetId"]) if binding["runtimeAssetId"] is not None else None
    require(
        strict_equal(bindings, expected_bindings),
        "Material texture bindings are not policy/owner-derived",
    )
    resolved_rows = sum(
        row["status"] in {RESOLVED_STATUS, DEPLOYED_STATUS}
        for row in expected_bindings
    )
    unresolved_rows = len(expected_bindings) - resolved_rows
    require(resolved_rows == EXPECTED_RESOLVED_ROWS and unresolved_rows == EXPECTED_UNRESOLVED_ROWS, "binding resolution denominator changed")
    require(
        sum(row["status"] == RESOLVED_STATUS for row in expected_bindings)
        == EXPECTED_COOK_BINDING_ROWS
        and sum(row["status"] == DEPLOYED_STATUS for row in expected_bindings)
        == EXPECTED_DEPLOYMENT_BINDING_ROWS,
        "binding authority denominator changed",
    )
    require(sum(len(row["materialOccurrenceIds"]) for row in expected_bindings) == EXPECTED_OCCURRENCE_LINKS, "occurrence link denominator changed")
    expected_admission = {
        "bindingReceipt": {"ready": True, "rowCount": EXPECTED_POLICY_ROWS},
        "resolvedRuntimeAssets": {"ready": True, "rowCount": EXPECTED_RESOLVED_ROWS},
        "completeRuntimeBinding": {"ready": True, "blockers": []},
        "rendererConsumer": {"ready": False, "blockers": [R4_BLOCKER]},
        "product": False,
    }
    require(strict_equal(receipt["admission"], expected_admission), "binding admission changed")
    expected_summary = {
        "samplerPolicyRowCount": EXPECTED_POLICY_ROWS,
        "uniqueLogicalTextureCount": EXPECTED_UNIQUE_TEXTURES,
        "resolvedBindingRowCount": EXPECTED_RESOLVED_ROWS,
        "unresolvedBindingRowCount": EXPECTED_UNRESOLVED_ROWS,
        "runtimeCookBindingRowCount": EXPECTED_COOK_BINDING_ROWS,
        "deploymentBindingRowCount": EXPECTED_DEPLOYMENT_BINDING_ROWS,
        "resolvedUniqueTextureCount": EXPECTED_RESOLVED_TEXTURES,
        "unresolvedUniqueTextureCount": EXPECTED_UNRESOLVED_TEXTURES,
        "runtimeCookUniqueTextureCount": EXPECTED_COOK_TEXTURES,
        "deploymentUniqueTextureCount": EXPECTED_DEPLOYMENT_TEXTURES,
        "sourcePackageBoundUniqueTextureCount": 49,
        "sourcePackageUnboundUniqueTextureCount": 3,
        "materialOccurrenceLinkCount": EXPECTED_OCCURRENCE_LINKS,
        "completedProvisioningProposalCount": EXPECTED_DEPLOYMENT_TEXTURES,
        "sourceExactBindingRowCount": 0,
        "rendererReadyBindingRowCount": 0,
        "productReadyBindingRowCount": 0,
    }
    require(strict_equal(receipt["summary"], expected_summary), "binding summary changed")
    if require_approval:
        approval_module().require_approved_receipt(receipt)


def deep_verify(
    receipt: dict[str, Any],
    source_pack_path: Path,
    runtime_resources: Path,
) -> None:
    require(runtime_resources.is_dir(), f"runtime Resources root is missing: {runtime_resources}")
    source_root = source_pack_path.parent.resolve()
    verified_packages: set[str] = set()
    verified_assets: set[str] = set()
    resource_by_id: dict[str, dict[str, Any]] = {}
    physical_casefold: dict[str, str] = {}
    texture_root = runtime_resources / "Effect/Artist/Textures"
    require(texture_root.is_dir(), f"Artist texture root is missing: {texture_root}")
    for path in texture_root.rglob("*"):
        if not path.is_file():
            continue
        relative = path.relative_to(runtime_resources).as_posix()
        key = relative.casefold()
        require(key not in physical_casefold, f"runtime Resources casefold collision: {relative}")
        physical_casefold[key] = relative
    for resource in receipt["textureResources"]:
        resource_by_id[resource["textureResourceId"]] = resource
        package = resource["sourcePackage"]
        if package is not None:
            relative_package = validate_relative_path(package["relativePath"], "source package relativePath")
            if relative_package not in verified_packages:
                package_path = (source_root / Path(*PurePosixPath(relative_package).parts)).resolve()
                require(source_root in package_path.parents, f"source package escaped root: {relative_package}")
                require(package_path.is_file(), f"source package missing: {relative_package}")
                require(package_path.stat().st_size == package["byteSize"], f"source package size mismatch: {relative_package}")
                require(raw_file_sha256(package_path) == package["rawSha256"], f"source package hash mismatch: {relative_package}")
                verified_packages.add(relative_package)
        asset_id = resource["runtimeAssetId"]
        if asset_id is not None and asset_id not in verified_assets:
            validate_runtime_asset_id(asset_id)
            actual_case = physical_casefold.get(asset_id.casefold())
            require(actual_case == asset_id, f"runtime asset path/case mismatch: expected={asset_id} actual={actual_case}")
            lexical_path = runtime_resources / Path(*PurePosixPath(asset_id).parts)
            path = lexical_path.resolve()
            require(
                runtime_resources.resolve() in path.parents
                and lexical_path.is_file()
                and not lexical_path.is_symlink(),
                f"runtime asset escaped root or is not a regular non-symlink file: {asset_id}",
            )
            payload = (
                resource["runtimeCookEvidence"]
                if resource["status"] == RESOLVED_STATUS
                else resource["deploymentEvidence"]
            )
            expected_byte_count = (
                payload["byteSize"]
                if resource["status"] == RESOLVED_STATUS
                else payload["byteCount"]
            )
            require(path.stat().st_size == expected_byte_count, f"runtime asset size mismatch: {asset_id}")
            require(raw_file_sha256(path) == payload["rawSha256"], f"runtime asset hash mismatch: {asset_id}")
            verified_assets.add(asset_id)
    require(len(verified_assets) == EXPECTED_RESOLVED_TEXTURES, "deep runtime asset denominator changed")
    verified_bindings = 0
    for binding_row in receipt["materialTextureBindings"]:
        resource = resource_by_id.get(binding_row["textureResourceId"])
        require(
            resource is not None
            and binding_row["runtimeAssetId"] == resource["runtimeAssetId"]
            and resource["runtimeAssetId"] in verified_assets,
            f"deep binding/runtime reverse join failed: {binding_row['bindingId']}",
        )
        verified_bindings += 1
    require(verified_bindings == EXPECTED_POLICY_ROWS, "deep binding denominator changed")


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n",
        encoding="utf-8",
        newline="\n",
    )
    temporary.replace(path)


def load_tracked_inputs(args: argparse.Namespace) -> tuple[dict[str, Any], ...]:
    return (
        read_json(args.policy),
        read_json(args.contract),
        read_json(args.runtime_oracle),
        read_json(args.acquisition),
        read_json(args.resource_manifest),
        read_json(args.exact_dds),
        read_json(args.candidate),
    )


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--policy", type=Path, default=DEFAULT_POLICY)
    parser.add_argument("--contract", type=Path, default=DEFAULT_CONTRACT)
    parser.add_argument("--runtime-oracle", type=Path, default=DEFAULT_RUNTIME_ORACLE)
    parser.add_argument("--acquisition", type=Path, default=DEFAULT_ACQUISITION)
    parser.add_argument("--resource-manifest", type=Path, default=DEFAULT_RESOURCE_MANIFEST)
    parser.add_argument("--exact-dds", type=Path, default=DEFAULT_EXACT_DDS)
    parser.add_argument("--candidate", type=Path, default=DEFAULT_CANDIDATE)
    parser.add_argument("--runtime-cook", type=Path, default=DEFAULT_RUNTIME_COOK)
    parser.add_argument("--resource-export", type=Path, default=DEFAULT_RESOURCE_EXPORT)
    parser.add_argument("--source-pack", type=Path, default=DEFAULT_SOURCE_PACK)
    parser.add_argument("--runtime-resources", type=Path, default=DEFAULT_RUNTIME_RESOURCES)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--validate-only", action="store_true")
    parser.add_argument("--deep-verify", action="store_true")
    return parser.parse_args(argv)


def main(argv: Iterable[str] | None = None) -> int:
    args = parse_args(argv)
    (
        policy,
        contract,
        runtime_oracle,
        acquisition,
        resource_manifest,
        exact_dds,
        candidate,
    ) = load_tracked_inputs(args)
    if args.validate_only:
        checked = read_json(args.output)
        validate_receipt(
            checked,
            policy,
            contract,
            resource_manifest,
            exact_dds,
            candidate,
            policy_path=args.policy,
            contract_path=args.contract,
            resource_manifest_path=args.resource_manifest,
            exact_dds_path=args.exact_dds,
            candidate_path=args.candidate,
            runtime_oracle=runtime_oracle,
            acquisition=acquisition,
        )
        print("PASS: Artist F Material texture runtime binding validate-only 77/77 resolved 52/52 unique deployment=4 product=false")
        return 0
    runtime_cook = read_json(args.runtime_cook)
    resource_export = read_json(args.resource_export)
    source_pack = read_json(args.source_pack)
    candidate_receipt = build_receipt(
        policy,
        contract,
        resource_manifest,
        exact_dds,
        candidate,
        runtime_cook,
        resource_export,
        source_pack,
        policy_path=args.policy,
        contract_path=args.contract,
        resource_manifest_path=args.resource_manifest,
        exact_dds_path=args.exact_dds,
        candidate_path=args.candidate,
        runtime_cook_path=args.runtime_cook,
        resource_export_path=args.resource_export,
        source_pack_path=args.source_pack,
        runtime_oracle=runtime_oracle,
        acquisition=acquisition,
    )
    if args.check:
        checked = read_json(args.output)
        require(
            strict_equal(checked, candidate_receipt),
            "checked binding receipt is not the deterministic external-evidence rebuild",
        )
        validate_receipt(
            checked,
            policy,
            contract,
            resource_manifest,
            exact_dds,
            candidate,
            policy_path=args.policy,
            contract_path=args.contract,
            resource_manifest_path=args.resource_manifest,
            exact_dds_path=args.exact_dds,
            candidate_path=args.candidate,
            runtime_oracle=runtime_oracle,
            acquisition=acquisition,
            runtime_cook=runtime_cook,
            resource_export=resource_export,
            source_pack=source_pack,
        )
    else:
        write_json_atomic(args.output, candidate_receipt)
    if args.deep_verify:
        deep_verify(candidate_receipt, args.source_pack, args.runtime_resources)
    mode = "check" if args.check else "write"
    if args.deep_verify:
        mode += "+deep"
    print(
        f"PASS: Artist F Material texture runtime binding mode={mode} "
        "rows=73cook+4deployment/77 unique=48cook+4deployment/52 "
        "completedProposals=4 product=false"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError) as error:
        print(f"ERROR: {error}")
        raise SystemExit(1)
