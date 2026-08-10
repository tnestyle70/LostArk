#!/usr/bin/env python3
"""Build the fail-closed Artist 31470 F reconstructed Material policy.

This policy deliberately does not change source fidelity.  It selects explicit,
versioned execution values for the 255 rows frozen at Material checkpoint
cde8f3bd, preserves every evidence blocker, and keeps runtime/Product admission
closed until a typed runtime consumer exists.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import struct
from pathlib import Path
from typing import Any, Iterable

from effect_source_contract_io import (
    load_strict_json_object,
    tracked_text_sha256,
)


ROOT = Path(__file__).resolve().parents[2]
MATERIAL_ROOT = ROOT / "Data/Effects/Imported/Artist/Materials"
DEFAULT_RUNTIME_RECEIPT = MATERIAL_ROOT / "skill.31470.material-runtime-oracle.receipt.json"
DEFAULT_ACQUISITION_RECEIPT = MATERIAL_ROOT / "skill.31470.material-source-value-acquisition.receipt.json"
DEFAULT_MATERIAL_CONTRACT = MATERIAL_ROOT / "skill.31470.typed-material-evidence-contract.json"
DEFAULT_HLSL = ROOT / "Tools/MaterialEvaluatorHarness/Shader_Artist31470MaterialReconstructedPolicy.hlsl"
DEFAULT_VERIFIER = ROOT / "Tools/LevelPlacementExtractor/verify_artist_31470_material_reconstructed_policy_hlsl.py"
DEFAULT_APPROVAL = ROOT / "Tools/LevelPlacementExtractor/artist_31470_material_reconstructed_policy_approval.py"
DIRECT_IMPORT_DEPENDENCY_PATHS = {
    "STRICT_JSON_OBJECT_LOADER": ROOT / "Tools/LevelPlacementExtractor/effect_source_contract_io.py",
    "MATERIAL_EVIDENCE_VALIDATOR": ROOT / "Tools/LevelPlacementExtractor/build_artist_31470_material_evidence_contract.py",
    "MATERIAL_RUNTIME_ORACLE_VALIDATOR": ROOT / "Tools/LevelPlacementExtractor/build_artist_31470_material_runtime_oracle.py",
    "MATERIAL_SOURCE_VALUE_ACQUISITION_VALIDATOR": ROOT / "Tools/LevelPlacementExtractor/build_artist_31470_material_source_value_acquisition.py",
    "RUNTIME_WARP_SUPPORT": ROOT / "Tools/LevelPlacementExtractor/verify_artist_31470_material_runtime_oracle_hlsl.py",
    "RECONSTRUCTED_POLICY_APPROVAL": DEFAULT_APPROVAL,
    "RECONSTRUCTED_POLICY_WARP_VERIFIER": DEFAULT_VERIFIER,
}
DEFAULT_OUTPUT = MATERIAL_ROOT / "skill.31470.material-reconstructed-approved-v1.receipt.json"

FROZEN_MATERIAL_COMMIT = "cde8f3bddea2f9415f682b387d2705fd25794075"
PINNED_RUNTIME_RECEIPT_SHA256 = "e128e281753fbd01582e588afbb682847401348836a046ad424a720360003ff6"
PINNED_ACQUISITION_RECEIPT_SHA256 = "cf45b6db4290aaffb10410bea346daf1bbbc52d585611a4347326483a7d48f43"
PINNED_MATERIAL_CONTRACT_SHA256 = "638fae77c5805a8d33cacb69b5cdd810d40d2f304606dd56f970dd2504c1cfcb"

POLICY_ID = "RECONSTRUCTED_APPROVED_V1"
POLICY_IMPLEMENTATION_VERSION = 1
RENDER_IMPLEMENTATION_ID = "ARTIST_F_RENDER_STATE_RECONSTRUCTION_V1"
STATIC_IMPLEMENTATION_ID = "ARTIST_F_STATIC_PERMUTATION_RECONSTRUCTION_V1"
SAMPLER_IMPLEMENTATION_ID = "ARTIST_F_SAMPLER_DESCRIPTOR_RECONSTRUCTION_V1"
NUMERIC_IMPLEMENTATION_ID = "ARTIST_F_POLICY_NUMERIC_ORACLE_V1"

EXPECTED_RENDER_ROWS = 89
EXPECTED_STATIC_ROWS = 94
EXPECTED_SAMPLER_ROWS = 72
EXPECTED_TOTAL_ROWS = 255
EXPECTED_STATIC_EXACT_OVERRIDE_ROWS = 23
EXPECTED_D3D_DESCRIPTOR_ROWS = 107

D3D11_FILTER_MIN_MAG_MIP_LINEAR = 0x15
D3D11_TEXTURE_ADDRESS_WRAP = 1
D3D11_TEXTURE_ADDRESS_CLAMP = 3
D3D11_COMPARISON_NEVER = 1
D3D11_FILL_SOLID = 3
D3D11_CULL_BACK = 3
D3D11_DEPTH_WRITE_MASK_ALL = 1
D3D11_COMPARISON_LESS = 2
FLOAT32_MAX = struct.unpack("<f", bytes.fromhex("ffff7f7f"))[0]

RENDER_FIELD_CODES = {
    "bdisabledepthtest": 1,
    "buseonelayerdistortion": 2,
    "opacitymaskclipvalue": 3,
    "twosided": 4,
    "lightingmodel": 5,
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def read_json(path: Path) -> dict[str, Any]:
    return load_strict_json_object(path)


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


def direct_import_closure(
    dependency_paths: dict[str, Path] = DIRECT_IMPORT_DEPENDENCY_PATHS,
) -> dict[str, Any]:
    rows = []
    for dependency_id, path in sorted(dependency_paths.items()):
        require(path.is_file(), f"direct import dependency is missing: {dependency_id}")
        identity_path = DIRECT_IMPORT_DEPENDENCY_PATHS.get(dependency_id, path)
        try:
            relative_path = identity_path.resolve().relative_to(ROOT.resolve()).as_posix()
        except ValueError:
            relative_path = identity_path.name
        rows.append(
            {
                "dependencyId": dependency_id,
                "relativePath": relative_path,
                "canonicalTextSha256": tracked_text_sha256(path),
            }
        )
    return {
        "hashRole": "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
        "dependencyCount": len(rows),
        "dependencies": rows,
        "projectionSha256": canonical_sha256(rows),
    }


def validate_direct_import_closure(
    evidence: dict[str, Any],
    dependency_paths: dict[str, Path] = DIRECT_IMPORT_DEPENDENCY_PATHS,
) -> None:
    require(
        evidence == direct_import_closure(dependency_paths),
        "reconstructed Material direct import dependency closure changed",
    )


def f32(value: float) -> float:
    require(type(value) in (int, float) and not isinstance(value, bool), "numeric value has invalid type")
    value32 = struct.unpack("<f", struct.pack("<f", float(value)))[0]
    require(math.isfinite(value32), "numeric value is not finite float32")
    return value32


def validate_finite_tree(value: Any, label: str) -> None:
    if isinstance(value, dict):
        for key, child in value.items():
            validate_finite_tree(child, f"{label}.{key}")
    elif isinstance(value, list):
        for index, child in enumerate(value):
            validate_finite_tree(child, f"{label}[{index}]")
    elif isinstance(value, float):
        require(math.isfinite(value), f"non-finite value: {label}")


def validate_self_digest(value: dict[str, Any], key: str, label: str) -> None:
    digest = value.get(key)
    require(isinstance(digest, str) and len(digest) == 64, f"{label} digest is invalid")
    payload = copy.deepcopy(value)
    payload.pop(key, None)
    require(canonical_sha256(payload) == digest, f"{label} digest mismatch")


def validate_frozen_upstreams(
    runtime_receipt: dict[str, Any],
    acquisition_receipt: dict[str, Any],
    material_contract: dict[str, Any],
) -> None:
    from build_artist_31470_material_evidence_contract import validate_contract
    from build_artist_31470_material_runtime_oracle import validate_runtime_receipt
    from build_artist_31470_material_source_value_acquisition import validate_receipt_semantics

    validate_runtime_receipt(runtime_receipt)
    validate_contract(material_contract)
    validate_receipt_semantics(acquisition_receipt, material_contract)
    require(
        runtime_receipt.get("receiptSha256") == PINNED_RUNTIME_RECEIPT_SHA256,
        "runtime oracle is not the frozen cde8f3bd input",
    )
    require(
        acquisition_receipt.get("receiptSha256") == PINNED_ACQUISITION_RECEIPT_SHA256,
        "source-value acquisition is not the frozen cde8f3bd input",
    )
    require(
        material_contract.get("contractSha256") == PINNED_MATERIAL_CONTRACT_SHA256,
        "typed Material contract is not the frozen cde8f3bd input",
    )


def policy_row_id(kind: str, source_row_id: str) -> str:
    return "material-reconstructed-policy-" + canonical_sha256(
        {"kind": kind, "sourceMatrixRowId": source_row_id, "policyId": POLICY_ID}
    )[:20]


def row_common(
    *,
    kind: str,
    policy_order: int,
    acquisition_row: dict[str, Any],
    runtime_row: dict[str, Any],
) -> dict[str, Any]:
    for key in (
        "matrixRowId",
        "materialRecipeId",
        "materialOccurrenceIds",
        "fieldId",
        "fieldKind",
        "bindingOriginAndOwner",
    ):
        require(acquisition_row.get(key) == runtime_row.get(key), f"matrix join changed: {key}")
    occurrences = acquisition_row.get("materialOccurrenceIds")
    require(
        isinstance(occurrences, list)
        and occurrences
        and all(isinstance(value, str) and value for value in occurrences),
        "policy row occurrence owner is invalid",
    )
    blockers = {
        "sourceValueAcquisition": copy.deepcopy(acquisition_row.get("remainingBlockers") or []),
        "runtimeOracle": copy.deepcopy(runtime_row.get("remainingBlockers") or []),
    }
    require(all(blockers.values()), "policy row must preserve both upstream blocker sets")
    return {
        "policyRowId": policy_row_id(kind, acquisition_row["matrixRowId"]),
        "policyOrder": policy_order,
        "policyKind": kind,
        "sourceMatrixRowId": acquisition_row["matrixRowId"],
        "materialRecipeId": acquisition_row["materialRecipeId"],
        "materialOccurrenceIds": copy.deepcopy(occurrences),
        "fieldId": acquisition_row["fieldId"],
        "fieldKind": acquisition_row["fieldKind"],
        "bindingOriginAndOwner": copy.deepcopy(acquisition_row["bindingOriginAndOwner"]),
        "upstreamEvidence": {
            "sourceValueAcquisitionRowSha256": canonical_sha256(acquisition_row),
            "runtimeOracleRowSha256": canonical_sha256(runtime_row),
        },
        "policyFidelity": POLICY_ID,
        "sourceExact": False,
        "evidenceBlockers": blockers,
        "policySelectionAdmission": True,
        "runtimeConsumerAdmission": False,
        "rendererConsumerAdmission": False,
        "productAdmission": False,
    }


def render_selected_value(field_name: str, acquisition_row: dict[str, Any]) -> tuple[dict[str, Any], dict[str, Any]]:
    if field_name == "bdisabledepthtest":
        return (
            {"type": "BOOL", "value": False},
            {"basisId": "EFFECT_DEPTH_TEST_ENABLED_ROLE_POLICY", "basisRevision": 1},
        )
    if field_name == "buseonelayerdistortion":
        return (
            {"type": "BOOL", "value": False},
            {"basisId": "OMITTED_OPT_IN_FEATURE_DISABLED_ROLE_POLICY", "basisRevision": 1},
        )
    if field_name == "opacitymaskclipvalue":
        candidate = acquisition_row["defaultChain"]["currentOnlyCdoCandidate"]
        require(candidate.get("admissibleAsSourceEra") is False, "current CDO was promoted to source evidence")
        field = candidate["field"]
        if field.get("status") == "CURRENT_SERIALIZED_EXPLICIT":
            prop = field["property"]
            require(prop.get("value") == 0.33329999446868896, "current CDO opacity candidate changed")
            basis = {
                "basisId": "CURRENT_MATERIAL_CDO_SERIALIZED_CANDIDATE_POLICY",
                "basisRevision": 1,
                "candidateId": candidate["candidateId"],
                "propertyRecordSha256": prop["recordSha256"],
                "admissibleAsSourceEra": False,
            }
        else:
            require(field.get("status") == "OMITTED_FROM_CURRENT_CDO_EXPORT", "opacity candidate status changed")
            basis = {
                "basisId": "OPACITY_MASK_STANDARD_THRESHOLD_ROLE_POLICY",
                "basisRevision": 1,
                "candidateId": candidate["candidateId"],
                "currentCdoFieldStatus": field["status"],
                "admissibleAsSourceEra": False,
            }
        return (
            {"type": "FLOAT32", "value": f32(0.33329999446868896)},
            basis,
        )
    if field_name == "twosided":
        return (
            {"type": "BOOL", "value": False},
            {"basisId": "OMITTED_TWO_SIDED_OPT_IN_DISABLED_ROLE_POLICY", "basisRevision": 1},
        )
    if field_name == "lightingmodel":
        return (
            {"type": "ENUM", "enumType": "EMaterialLightingModel", "value": "mlm_unlit", "ordinal": 0},
            {"basisId": "EFFECT_MATERIAL_UNLIT_ROLE_POLICY", "basisRevision": 1},
        )
    raise ValueError(f"unsupported render policy field: {field_name}")


def render_numeric_expected(field_name: str, selected: dict[str, Any]) -> list[float]:
    code = RENDER_FIELD_CODES[field_name]
    value = selected["value"]
    if field_name == "bdisabledepthtest":
        result = [float(value), float(not value), float(code), 1.0]
    elif field_name == "buseonelayerdistortion":
        result = [float(value), float(value), float(code), 1.0]
    elif field_name == "opacitymaskclipvalue":
        result = [float(value), 1.0 if value >= 0.0 else 0.0, float(code), 1.0]
    elif field_name == "twosided":
        result = [float(value), 1.0 if value else float(D3D11_CULL_BACK), float(code), 1.0]
    else:
        result = [float(selected["ordinal"]), 0.0, float(code), 1.0]
    return [f32(value) for value in result]


def build_render_rows(
    acquisition_rows: list[dict[str, Any]], runtime_rows: list[dict[str, Any]]
) -> list[dict[str, Any]]:
    require(len(acquisition_rows) == len(runtime_rows) == EXPECTED_RENDER_ROWS, "render denominator changed")
    result: list[dict[str, Any]] = []
    for index, (acquisition_row, runtime_row) in enumerate(zip(acquisition_rows, runtime_rows, strict=True)):
        row = row_common(
            kind="RENDER_STATE",
            policy_order=index,
            acquisition_row=acquisition_row,
            runtime_row=runtime_row,
        )
        field_name = acquisition_row.get("fieldName")
        require(field_name in RENDER_FIELD_CODES, "unknown render field")
        selected, basis = render_selected_value(field_name, acquisition_row)
        row.update(
            {
                "fieldName": field_name,
                "selectedValue": selected,
                "providerBasis": basis,
                "implementation": {
                    "implementationId": RENDER_IMPLEMENTATION_ID,
                    "implementationVersion": POLICY_IMPLEMENTATION_VERSION,
                    "consumerContract": acquisition_row["rendererConsumption"]["consumer"],
                },
                "numericOracle": {
                    "implementationId": NUMERIC_IMPLEMENTATION_ID,
                    "implementationVersion": POLICY_IMPLEMENTATION_VERSION,
                    "expectedFloat4": render_numeric_expected(field_name, selected),
                    "numericTolerance": 0.0,
                    "finiteRequired": True,
                },
                "d3dStateOracleId": (
                    "D3D11_DEPTH_STENCIL_DESC" if field_name == "bdisabledepthtest" else
                    "D3D11_RASTERIZER_DESC" if field_name == "twosided" else None
                ),
            }
        )
        row["rowSha256"] = canonical_sha256(row)
        result.append(row)
    return result


def build_static_rows(
    acquisition_rows: list[dict[str, Any]], runtime_rows: list[dict[str, Any]]
) -> list[dict[str, Any]]:
    require(len(acquisition_rows) == len(runtime_rows) == EXPECTED_STATIC_ROWS, "static denominator changed")
    result: list[dict[str, Any]] = []
    exact_override_count = 0
    for local_index, (acquisition_row, runtime_row) in enumerate(zip(acquisition_rows, runtime_rows, strict=True)):
        row = row_common(
            kind="STATIC_PERMUTATION",
            policy_order=EXPECTED_RENDER_ROWS + local_index,
            acquisition_row=acquisition_row,
            runtime_row=runtime_row,
        )
        parent_value = acquisition_row["parentExpression"]["parentDefaultValue"]
        require(type(parent_value) is bool, "static parent default is not bool")
        selection = acquisition_row["micNativeSelection"]
        entry = selection.get("entry")
        if entry is not None and entry.get("bOverride") is True:
            require(type(entry.get("value")) is bool, "static override is not bool")
            selected_value = entry["value"]
            basis_id = "SOURCE_ARCHIVE_MIC_EXACT_OVERRIDE_RETAINED_AS_RECONSTRUCTION_POLICY"
            decision_code = 1
            exact_override_count += 1
        elif entry is not None:
            require(entry.get("bOverride") is False, "static native override flag is invalid")
            selected_value = parent_value
            basis_id = "SOURCE_ARCHIVE_MIC_NONOVERRIDE_PARENT_DEFAULT_POLICY"
            decision_code = 2
        else:
            selected_value = parent_value
            basis_id = "SOURCE_ARCHIVE_PARENT_STATIC_DEFAULT_POLICY"
            decision_code = 3
        row.update(
            {
                "parameterName": acquisition_row["parameterName"],
                "selectedValue": {"type": "BOOL", "value": selected_value},
                "providerBasis": {
                    "basisId": basis_id,
                    "basisRevision": 1,
                    "parentExpressionSha256": canonical_sha256(acquisition_row["parentExpression"]),
                    "micNativeSelectionSha256": canonical_sha256(selection),
                    "sourceValueDecision": acquisition_row["sourceValueDecision"],
                    "sourceExactPolicy": False,
                },
                "implementation": {
                    "implementationId": STATIC_IMPLEMENTATION_ID,
                    "implementationVersion": POLICY_IMPLEMENTATION_VERSION,
                    "consumerContract": "MATERIAL_STATIC_SHADER_PERMUTATION",
                },
                "numericOracle": {
                    "implementationId": NUMERIC_IMPLEMENTATION_ID,
                    "implementationVersion": POLICY_IMPLEMENTATION_VERSION,
                    "expectedFloat4": [
                        f32(float(selected_value)),
                        f32(float(not selected_value)),
                        f32(float(decision_code)),
                        1.0,
                    ],
                    "numericTolerance": 0.0,
                    "finiteRequired": True,
                },
                "d3dStateOracleId": None,
            }
        )
        row["rowSha256"] = canonical_sha256(row)
        result.append(row)
    require(exact_override_count == EXPECTED_STATIC_EXACT_OVERRIDE_ROWS, "static exact override denominator changed")
    return result


def source_field_value(field: dict[str, Any]) -> Any | None:
    if field.get("status") != "SERIALIZED_EXPLICIT":
        return None
    return field["property"]["value"]


def sampler_address(field: dict[str, Any]) -> tuple[str, int, dict[str, Any]]:
    explicit = source_field_value(field)
    if explicit is None:
        return (
            "ta_wrap",
            D3D11_TEXTURE_ADDRESS_WRAP,
            {"basisId": "EFFECT_UV_WRAP_ROLE_POLICY", "basisRevision": 1},
        )
    require(explicit in {"ta_wrap", "ta_clamp"}, "unsupported source sampler address")
    return (
        explicit,
        D3D11_TEXTURE_ADDRESS_CLAMP if explicit == "ta_clamp" else D3D11_TEXTURE_ADDRESS_WRAP,
        {
            "basisId": "SOURCE_ARCHIVE_EXPLICIT_VALUE_RETAINED_AS_RECONSTRUCTION_POLICY",
            "basisRevision": 1,
            "propertyRecordSha256": field["property"]["recordSha256"],
        },
    )


def sampler_srgb(fields: dict[str, Any], lod_group: str) -> tuple[bool, dict[str, Any]]:
    explicit = source_field_value(fields["srgb"])
    if explicit is not None:
        require(type(explicit) is bool, "source sampler sRGB is not bool")
        return explicit, {
            "basisId": "SOURCE_ARCHIVE_EXPLICIT_VALUE_RETAINED_AS_RECONSTRUCTION_POLICY",
            "basisRevision": 1,
            "propertyRecordSha256": fields["srgb"]["property"]["recordSha256"],
        }
    linear_groups = {
        "texturegroup_effectsnormalmap",
        "texturegroup_characternormalmap",
        "texturegroup_characterspecular",
    }
    require(
        lod_group in linear_groups | {"texturegroup_effects", "texturegroup_character"},
        "unsupported sampler LODGroup",
    )
    return lod_group not in linear_groups, {
        "basisId": "SOURCE_LODGROUP_COLOR_ROLE_POLICY",
        "basisRevision": 1,
        "lodGroup": lod_group,
        "role": "LINEAR_DATA" if lod_group in linear_groups else "COLOR_DATA",
    }


def resolve_current_texture_filter_candidate(acquisition_receipt: dict[str, Any]) -> dict[str, Any]:
    candidates = (
        acquisition_receipt.get("externalArtifactSearch", {})
        .get("currentRevisionCandidates", {})
        .get("classDefaultObjects", [])
    )
    matches = [row for row in candidates if row.get("candidateId") == "current-default-texture"]
    require(len(matches) == 1, "current Texture CDO candidate must resolve exactly once")
    candidate = matches[0]
    export = candidate.get("export") or {}
    require(
        export.get("exportIndex") == 9747
        and export.get("packageReference") == 9748
        and export.get("objectPath") == "Default__Texture"
        and export.get("className") == "texture"
        and export.get("serialSha256") == "630d85dd8451cd9be1cbafbb75422f08b2e854e04b62ced8fd7ec03a6a2e6516",
        "current Texture CDO export identity changed",
    )
    field = (candidate.get("fields") or {}).get("filter") or {}
    prop = field.get("property") or {}
    require(
        field.get("status") == "CURRENT_SERIALIZED_EXPLICIT"
        and prop.get("propertyName") == "Filter"
        and prop.get("propertyType") == "ByteProperty"
        and prop.get("value") == "TF_Linear"
        and prop.get("encodedValueSha256") == "ddc52bfd0a4b11674a8cee97836f1d83fccff8bbdf335bfd7266b0774bd4216f"
        and prop.get("recordSha256") == "4c140c1453a336286281e8b0a4e80b17c0ffdde73bef0465494e6e046db7ef5c",
        "current Texture CDO TF_Linear evidence changed",
    )
    require(
        candidate.get("revisionFidelity") == "CURRENT_REVISION_CROSS_REVISION_CANDIDATE_ONLY"
        and candidate.get("admissibleAsSourceEra") is False,
        "current Texture CDO candidate fidelity changed",
    )
    return copy.deepcopy(candidate)


def sampler_numeric_expected(descriptor: dict[str, Any]) -> list[float]:
    return [
        f32(float(descriptor["filter"]["d3d11"])),
        f32(float(descriptor["addressU"]["d3d11"])),
        f32(float(descriptor["addressV"]["d3d11"])),
        f32(float(descriptor["sRgb"])),
    ]


def build_sampler_rows(
    acquisition_rows: list[dict[str, Any]],
    runtime_rows: list[dict[str, Any]],
    texture_filter_candidate: dict[str, Any],
) -> list[dict[str, Any]]:
    require(len(acquisition_rows) == len(runtime_rows) == EXPECTED_SAMPLER_ROWS, "sampler denominator changed")
    result: list[dict[str, Any]] = []
    for local_index, (acquisition_row, runtime_row) in enumerate(zip(acquisition_rows, runtime_rows, strict=True)):
        row = row_common(
            kind="SAMPLER_DESCRIPTOR",
            policy_order=EXPECTED_RENDER_ROWS + EXPECTED_STATIC_ROWS + local_index,
            acquisition_row=acquisition_row,
            runtime_row=runtime_row,
        )
        require(acquisition_row.get("fullDescriptorSourceExact") is False, "source-exact sampler was reintroduced")
        fields = acquisition_row["textureExportEvidence"]["fields"]
        address_u_name, address_u, basis_u = sampler_address(fields["addressx"])
        address_v_name, address_v, basis_v = sampler_address(fields["addressy"])
        lod_group = source_field_value(fields["lodgroup"])
        require(isinstance(lod_group, str) and lod_group, "sampler LODGroup must be explicit")
        srgb, basis_srgb = sampler_srgb(fields, lod_group)
        descriptor = {
            "type": "D3D11_SAMPLER_DESC_AND_SRV_COLOR_SPACE",
            "filter": {"ue3": "tf_linear", "d3d11": D3D11_FILTER_MIN_MAG_MIP_LINEAR},
            "addressU": {"ue3": address_u_name, "d3d11": address_u},
            "addressV": {"ue3": address_v_name, "d3d11": address_v},
            "addressW": {"ue3": "ta_wrap", "d3d11": D3D11_TEXTURE_ADDRESS_WRAP},
            "mipLODBias": 0.0,
            "maxAnisotropy": 0,
            "comparisonFunc": {"name": "D3D11_COMPARISON_NEVER", "d3d11": D3D11_COMPARISON_NEVER},
            "borderColor": [0.0, 0.0, 0.0, 0.0],
            "minLOD": 0.0,
            "maxLOD": FLOAT32_MAX,
            "sRgb": srgb,
            "srvColorSpace": "SRGB" if srgb else "LINEAR",
            "lodGroup": lod_group,
        }
        row.update(
            {
                "logicalTexturePath": acquisition_row["logicalTexturePath"],
                "selectedDescriptor": descriptor,
                "providerBasis": {
                    "basisId": "VERSIONED_EFFECT_SAMPLER_ROLE_POLICY",
                    "basisRevision": 1,
                    "addressU": basis_u,
                    "addressV": basis_v,
                    "addressW": {"basisId": "EFFECT_UV_W_ROLE_POLICY", "basisRevision": 1},
                    "filter": {
                        "basisId": "CURRENT_TEXTURE_CDO_TF_LINEAR_CANDIDATE_POLICY",
                        "basisRevision": 1,
                        "candidateId": texture_filter_candidate["candidateId"],
                        "exportIdentity": copy.deepcopy(texture_filter_candidate["export"]),
                        "propertyEvidence": copy.deepcopy(texture_filter_candidate["fields"]["filter"]["property"]),
                        "revisionFidelity": texture_filter_candidate["revisionFidelity"],
                        "admissibleAsSourceEra": texture_filter_candidate["admissibleAsSourceEra"],
                    },
                    "sRgb": basis_srgb,
                    "lodGroup": {
                        "basisId": "SOURCE_ARCHIVE_EXPLICIT_VALUE_RETAINED_AS_RECONSTRUCTION_POLICY",
                        "basisRevision": 1,
                        "propertyRecordSha256": fields["lodgroup"]["property"]["recordSha256"],
                    },
                    "remainingDescriptorFields": {
                        "basisId": "D3D11_EFFECT_SAMPLER_ROLE_POLICY",
                        "basisRevision": 1,
                    },
                },
                "implementation": {
                    "implementationId": SAMPLER_IMPLEMENTATION_ID,
                    "implementationVersion": POLICY_IMPLEMENTATION_VERSION,
                    "consumerContract": "D3D11_SAMPLER_DESC_AND_SRGB_SRV_FORMAT",
                },
                "numericOracle": {
                    "implementationId": NUMERIC_IMPLEMENTATION_ID,
                    "implementationVersion": POLICY_IMPLEMENTATION_VERSION,
                    "expectedFloat4": sampler_numeric_expected(descriptor),
                    "numericTolerance": 0.0,
                    "finiteRequired": True,
                },
                "d3dStateOracleId": "D3D11_SAMPLER_DESC",
            }
        )
        row["rowSha256"] = canonical_sha256(row)
        result.append(row)
    return result


def verification_placeholder(backend: str, expected_count: int) -> dict[str, Any]:
    return {"verified": False, "backend": backend, "expectedRowCount": expected_count, "rowResults": []}


def build_receipt(
    runtime_receipt: dict[str, Any],
    acquisition_receipt: dict[str, Any],
    material_contract: dict[str, Any],
    runtime_path: Path,
    acquisition_path: Path,
    contract_path: Path,
    hlsl_path: Path,
    verifier_path: Path,
    *,
    hlsl_verification: dict[str, Any] | None = None,
    warp_verification: dict[str, Any] | None = None,
) -> dict[str, Any]:
    validate_frozen_upstreams(runtime_receipt, acquisition_receipt, material_contract)
    acquisition_matrices = acquisition_receipt["matrices"]
    runtime_matrices = runtime_receipt["materialFeasibilityMatrices"]
    render_rows = build_render_rows(acquisition_matrices["renderStateRows"], runtime_matrices["renderStateRows"])
    static_rows = build_static_rows(acquisition_matrices["staticPermutationRows"], runtime_matrices["staticPermutationRows"])
    texture_filter_candidate = resolve_current_texture_filter_candidate(acquisition_receipt)
    sampler_rows = build_sampler_rows(
        acquisition_matrices["strictSamplerRows"],
        runtime_matrices["strictSamplerRows"],
        texture_filter_candidate,
    )
    all_rows = render_rows + static_rows + sampler_rows
    require([row["policyOrder"] for row in all_rows] == list(range(EXPECTED_TOTAL_ROWS)), "policy order is not stable")
    from artist_31470_material_reconstructed_policy_approval import require_approved_rows

    require_approved_rows(all_rows)

    recipe_occurrences: dict[str, list[str]] = {}
    for occurrence in material_contract["occurrences"]:
        recipe_occurrences.setdefault(occurrence["materialRecipeId"], []).append(occurrence["occurrenceId"])
    for row in all_rows:
        require(
            row["materialOccurrenceIds"] == recipe_occurrences[row["materialRecipeId"]],
            "policy row occurrence ownership changed",
        )

    receipt: dict[str, Any] = {
        "schema": "lostark.artist-31470-material-reconstructed-policy-receipt",
        "formatVersion": 1,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "policyContract": {
            "policyId": POLICY_ID,
            "policyVersion": POLICY_IMPLEMENTATION_VERSION,
            "scope": "MATERIAL_EXECUTION_VALUE_SELECTION_ONLY",
            "frozenMaterialCommit": FROZEN_MATERIAL_COMMIT,
            "sourceExact": False,
            "evidenceBlockersPreserved": True,
            "runtimeConsumerImplemented": False,
            "rendererConsumerImplemented": False,
            "productAdmission": False,
        },
        "sourceEvidence": {
            "runtimeOracle": {
                "receiptSha256": runtime_receipt["receiptSha256"],
                "trackedTextSha256": tracked_text_sha256(runtime_path),
            },
            "sourceValueAcquisition": {
                "receiptSha256": acquisition_receipt["receiptSha256"],
                "trackedTextSha256": tracked_text_sha256(acquisition_path),
            },
            "typedMaterialContract": {
                "contractSha256": material_contract["contractSha256"],
                "trackedTextSha256": tracked_text_sha256(contract_path),
            },
            "generatorTrackedTextSha256": tracked_text_sha256(Path(__file__)),
            "hlslTrackedTextSha256": tracked_text_sha256(hlsl_path),
            "verifierTrackedTextSha256": tracked_text_sha256(verifier_path),
            "approvalTrackedTextSha256": tracked_text_sha256(DEFAULT_APPROVAL),
            "directImportClosure": direct_import_closure(),
        },
        "renderStatePolicies": render_rows,
        "staticPermutationPolicies": static_rows,
        "samplerPolicies": sampler_rows,
        "hlslVerification": copy.deepcopy(hlsl_verification) if hlsl_verification is not None else verification_placeholder("D3D11_WARP_COMPUTE", EXPECTED_TOTAL_ROWS),
        "warpDescriptorVerification": copy.deepcopy(warp_verification) if warp_verification is not None else verification_placeholder("D3D11_WARP_STATE_OBJECTS", EXPECTED_D3D_DESCRIPTOR_ROWS),
        "admission": {
            "policySelection": {"ready": True, "rowCount": EXPECTED_TOTAL_ROWS},
            "runtimeConsumer": {"ready": False, "rowCount": 0},
            "rendererConsumer": {"ready": False, "rowCount": 0},
            "product": False,
            "blockers": [
                "TYPED_RUNTIME_MATERIAL_POLICY_CONSUMER_NOT_IMPLEMENTED",
                "RENDERER_POLICY_CONSUMER_NOT_IMPLEMENTED",
                "SOURCE_FIDELITY_BLOCKERS_PRESERVED",
            ],
        },
        "summary": {
            "renderStatePolicyRowCount": EXPECTED_RENDER_ROWS,
            "staticPermutationPolicyRowCount": EXPECTED_STATIC_ROWS,
            "staticExactOverrideValueRetainedCount": EXPECTED_STATIC_EXACT_OVERRIDE_ROWS,
            "samplerPolicyRowCount": EXPECTED_SAMPLER_ROWS,
            "totalPolicyRowCount": EXPECTED_TOTAL_ROWS,
            "sourceExactPolicyRowCount": 0,
            "d3dDescriptorOracleRowCount": EXPECTED_D3D_DESCRIPTOR_ROWS,
            "srvColorSpaceOracleRowCount": EXPECTED_SAMPLER_ROWS,
            "runtimeConsumerReadyRowCount": 0,
            "rendererConsumerReadyRowCount": 0,
            "productReadyRowCount": 0,
        },
    }
    validate_finite_tree(receipt, "receipt")
    receipt["receiptSha256"] = canonical_sha256(receipt)
    return receipt


def expected_d3d_descriptor(row: dict[str, Any]) -> dict[str, Any]:
    oracle_id = row.get("d3dStateOracleId")
    if oracle_id == "D3D11_DEPTH_STENCIL_DESC":
        disable_depth = row["selectedValue"]["value"]
        require(type(disable_depth) is bool, "depth policy value is invalid")
        stencil_face = {
            "StencilFailOp": 1,
            "StencilDepthFailOp": 1,
            "StencilPassOp": 1,
            "StencilFunc": 8,
        }
        return {
            "DepthEnable": not disable_depth,
            "DepthWriteMask": D3D11_DEPTH_WRITE_MASK_ALL,
            "DepthFunc": D3D11_COMPARISON_LESS,
            "StencilEnable": False,
            "StencilReadMask": 0xFF,
            "StencilWriteMask": 0xFF,
            "FrontFace": copy.deepcopy(stencil_face),
            "BackFace": copy.deepcopy(stencil_face),
        }
    if oracle_id == "D3D11_RASTERIZER_DESC":
        two_sided = row["selectedValue"]["value"]
        require(type(two_sided) is bool, "two-sided policy value is invalid")
        return {
            "FillMode": D3D11_FILL_SOLID,
            "CullMode": 1 if two_sided else D3D11_CULL_BACK,
            "FrontCounterClockwise": False,
            "DepthBias": 0,
            "DepthBiasClamp": 0.0,
            "SlopeScaledDepthBias": 0.0,
            "DepthClipEnable": True,
            "ScissorEnable": False,
            "MultisampleEnable": False,
            "AntialiasedLineEnable": False,
        }
    require(oracle_id == "D3D11_SAMPLER_DESC", "unknown D3D descriptor policy")
    selected = row["selectedDescriptor"]
    return {
        "Filter": selected["filter"]["d3d11"],
        "AddressU": selected["addressU"]["d3d11"],
        "AddressV": selected["addressV"]["d3d11"],
        "AddressW": selected["addressW"]["d3d11"],
        "MipLODBias": selected["mipLODBias"],
        "MaxAnisotropy": selected["maxAnisotropy"],
        "ComparisonFunc": selected["comparisonFunc"]["d3d11"],
        "BorderColor": copy.deepcopy(selected["borderColor"]),
        "MinLOD": selected["minLOD"],
        "MaxLOD": selected["maxLOD"],
    }


def expected_srv_projection(row: dict[str, Any]) -> dict[str, Any]:
    require(row.get("policyKind") == "SAMPLER_DESCRIPTOR", "SRV policy row kind changed")
    selected = row["selectedDescriptor"]
    srgb = selected["sRgb"]
    require(type(srgb) is bool, "SRV color-space policy is not bool")
    require(selected["srvColorSpace"] == ("SRGB" if srgb else "LINEAR"), "SRV color-space label changed")
    return {
        "Format": 29 if srgb else 28,
        "ViewDimension": 4,
        "MostDetailedMip": 0,
        "MipLevels": 1,
        "srvColorSpace": selected["srvColorSpace"],
    }


def validate_verification(receipt: dict[str, Any]) -> None:
    all_rows = receipt["renderStatePolicies"] + receipt["staticPermutationPolicies"] + receipt["samplerPolicies"]
    expected_by_id = {row["policyRowId"]: row["numericOracle"]["expectedFloat4"] for row in all_rows}
    hlsl = receipt.get("hlslVerification") or {}
    require(
        set(hlsl) == {
            "verified", "backend", "entryPoint", "targetProfile", "compiler",
            "hlslTrackedTextSha256", "compiledDxbcSha256", "sampleCount",
            "inputBytesSha256", "outputFloat32BytesSha256", "numericTolerance",
            "maxAbsoluteError", "rowResults", "rowResultsSha256",
        },
        "HLSL verification root schema changed",
    )
    require(hlsl.get("verified") is True and hlsl.get("backend") == "D3D11_WARP_COMPUTE", "HLSL verification is not complete")
    require(
        hlsl.get("entryPoint") == "main"
        and hlsl.get("targetProfile") == "cs_5_0"
        and type(hlsl.get("numericTolerance")) is float
        and hlsl["numericTolerance"] == 0.0
        and type(hlsl.get("maxAbsoluteError")) is float
        and hlsl["maxAbsoluteError"] == 0.0,
        "HLSL zero-tolerance execution metadata changed",
    )
    require(
        hlsl.get("compiler") == {
            "fileName": "d3dcompiler_47.dll",
            "byteSize": 4916800,
            "rawSha256": "ce013eb1639f8e2620a509e73b33029108f55a293e304e33e38b72fd65c531b8",
            "hashRole": "EXTERNAL_RAW_BYTES",
        },
        "HLSL compiler identity changed",
    )
    require(type(hlsl.get("sampleCount")) is int and hlsl["sampleCount"] == EXPECTED_TOTAL_ROWS, "HLSL sample denominator changed")
    results = hlsl.get("rowResults") or []
    require(len(results) == EXPECTED_TOTAL_ROWS, "HLSL row result denominator changed")
    require([row.get("policyRowId") for row in results] == list(expected_by_id), "HLSL result order changed")
    for result in results:
        require(
            set(result) == {
                "policyRowId", "expectedFloat4", "actualFloat4", "numericTolerance", "decision"
            },
            "HLSL row result schema changed",
        )
        require(result.get("expectedFloat4") == expected_by_id[result["policyRowId"]], "HLSL expected row changed")
        require(result.get("actualFloat4") == result["expectedFloat4"], "HLSL zero-tolerance result changed")
        require(result.get("numericTolerance") == 0.0 and result.get("decision") == "PASS", "HLSL row did not pass exactly")

    require(hlsl.get("rowResultsSha256") == canonical_sha256(results), "HLSL row result digest mismatch")

    expected_descriptor_rows = [row for row in all_rows if row["d3dStateOracleId"] is not None]
    expected_descriptor_ids = [row["policyRowId"] for row in expected_descriptor_rows]
    warp = receipt.get("warpDescriptorVerification") or {}
    require(
        set(warp) == {
            "verified", "backend", "featureLevel", "descriptorRowCount",
            "numericTolerance", "rowResults", "rowResultsSha256",
            "srvColorSpaceRowCount", "srvRowResults", "srvRowResultsSha256",
        },
        "WARP verification root schema changed",
    )
    require(warp.get("verified") is True and warp.get("backend") == "D3D11_WARP_STATE_OBJECTS", "WARP descriptor verification is not complete")
    require(
        type(warp.get("featureLevel")) is int
        and warp["featureLevel"] == 45056
        and type(warp.get("numericTolerance")) is float
        and warp["numericTolerance"] == 0.0,
        "WARP zero-tolerance execution metadata changed",
    )
    require(type(warp.get("descriptorRowCount")) is int and warp["descriptorRowCount"] == EXPECTED_D3D_DESCRIPTOR_ROWS, "WARP descriptor denominator changed")
    warp_rows = warp.get("rowResults") or []
    require([row.get("policyRowId") for row in warp_rows] == expected_descriptor_ids, "WARP descriptor row ownership changed")
    for policy_row, result in zip(expected_descriptor_rows, warp_rows, strict=True):
        require(
            set(result) == {
                "policyRowId", "descriptorKind", "expectedDescriptor", "actualDescriptor", "numericTolerance", "decision"
            },
            "WARP descriptor row schema changed",
        )
        expected_descriptor = expected_d3d_descriptor(policy_row)
        require(result.get("descriptorKind") == policy_row["d3dStateOracleId"], "WARP descriptor kind changed")
        require(result.get("expectedDescriptor") == expected_descriptor, "WARP expected descriptor is not policy-derived")
        require(result.get("actualDescriptor") == expected_descriptor, "WARP descriptor zero-tolerance mismatch")
        require(result.get("numericTolerance") == 0.0 and result.get("decision") == "PASS", "WARP descriptor did not pass exactly")
    require(warp.get("rowResultsSha256") == canonical_sha256(warp_rows), "WARP descriptor row digest mismatch")

    sampler_rows = receipt["samplerPolicies"]
    srv_rows = warp.get("srvRowResults") or []
    require(
        type(warp.get("srvColorSpaceRowCount")) is int
        and warp["srvColorSpaceRowCount"] == EXPECTED_SAMPLER_ROWS
        and [row.get("policyRowId") for row in srv_rows]
        == [row["policyRowId"] for row in sampler_rows],
        "WARP SRV color-space row ownership changed",
    )
    for policy_row, result in zip(sampler_rows, srv_rows, strict=True):
        require(
            set(result) == {
                "policyRowId", "expectedSrv", "actualSrv", "numericTolerance", "decision"
            },
            "WARP SRV row schema changed",
        )
        expected_srv = expected_srv_projection(policy_row)
        require(result.get("expectedSrv") == expected_srv, "WARP expected SRV is not policy-derived")
        require(result.get("actualSrv") == expected_srv, "WARP SRV zero-tolerance mismatch")
        require(result.get("numericTolerance") == 0.0 and result.get("decision") == "PASS", "WARP SRV row did not pass exactly")
    require(warp.get("srvRowResultsSha256") == canonical_sha256(srv_rows), "WARP SRV row digest mismatch")
    from artist_31470_material_reconstructed_policy_approval import require_approved_oracles

    require_approved_oracles(receipt)


def validate_policy_receipt(
    receipt: dict[str, Any],
    runtime_receipt: dict[str, Any],
    acquisition_receipt: dict[str, Any],
    material_contract: dict[str, Any],
    runtime_path: Path = DEFAULT_RUNTIME_RECEIPT,
    acquisition_path: Path = DEFAULT_ACQUISITION_RECEIPT,
    contract_path: Path = DEFAULT_MATERIAL_CONTRACT,
    hlsl_path: Path = DEFAULT_HLSL,
    verifier_path: Path = DEFAULT_VERIFIER,
) -> None:
    require(
        set(receipt) == {
            "schema", "formatVersion", "characterClass", "skillId", "inputSlot",
            "policyContract", "sourceEvidence", "renderStatePolicies",
            "staticPermutationPolicies", "samplerPolicies", "hlslVerification",
            "warpDescriptorVerification", "admission", "summary", "receiptSha256",
        },
        "policy receipt root schema changed",
    )
    require(
        receipt.get("schema") == "lostark.artist-31470-material-reconstructed-policy-receipt"
        and type(receipt.get("formatVersion")) is int and receipt["formatVersion"] == 1
        and receipt.get("characterClass") == "ARTIST"
        and type(receipt.get("skillId")) is int and receipt["skillId"] == 31470
        and receipt.get("inputSlot") == "F",
        "unsupported reconstructed Material policy receipt",
    )
    validate_finite_tree(receipt, "receipt")
    validate_self_digest(receipt, "receiptSha256", "reconstructed Material policy")
    validate_direct_import_closure(receipt["sourceEvidence"]["directImportClosure"])
    validate_verification(receipt)
    from artist_31470_material_reconstructed_policy_approval import require_approved_receipt

    require_approved_receipt(receipt)
    expected = build_receipt(
        runtime_receipt,
        acquisition_receipt,
        material_contract,
        runtime_path,
        acquisition_path,
        contract_path,
        hlsl_path,
        verifier_path,
        hlsl_verification=receipt["hlslVerification"],
        warp_verification=receipt["warpDescriptorVerification"],
    )
    require(receipt == expected, "reconstructed Material policy is not the upstream-derived candidate")


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n",
        encoding="utf-8",
        newline="\n",
    )
    temporary.replace(path)


def build_from_paths(args: argparse.Namespace) -> dict[str, Any]:
    runtime_receipt = read_json(args.runtime_receipt)
    acquisition_receipt = read_json(args.acquisition_receipt)
    material_contract = read_json(args.material_contract)
    provisional = build_receipt(
        runtime_receipt,
        acquisition_receipt,
        material_contract,
        args.runtime_receipt,
        args.acquisition_receipt,
        args.material_contract,
        args.hlsl,
        args.verifier,
    )
    if not args.run_hlsl:
        return provisional
    from verify_artist_31470_material_reconstructed_policy_hlsl import run_hlsl_oracle, run_warp_descriptor_oracle

    hlsl_verification = run_hlsl_oracle(provisional, args.hlsl, args.d3dcompiler)
    warp_verification = run_warp_descriptor_oracle(provisional)
    candidate = build_receipt(
        runtime_receipt,
        acquisition_receipt,
        material_contract,
        args.runtime_receipt,
        args.acquisition_receipt,
        args.material_contract,
        args.hlsl,
        args.verifier,
        hlsl_verification=hlsl_verification,
        warp_verification=warp_verification,
    )
    validate_policy_receipt(
        candidate,
        runtime_receipt,
        acquisition_receipt,
        material_contract,
        args.runtime_receipt,
        args.acquisition_receipt,
        args.material_contract,
        args.hlsl,
        args.verifier,
    )
    return candidate


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--runtime-receipt", type=Path, default=DEFAULT_RUNTIME_RECEIPT)
    parser.add_argument("--acquisition-receipt", type=Path, default=DEFAULT_ACQUISITION_RECEIPT)
    parser.add_argument("--material-contract", type=Path, default=DEFAULT_MATERIAL_CONTRACT)
    parser.add_argument("--hlsl", type=Path, default=DEFAULT_HLSL)
    parser.add_argument("--verifier", type=Path, default=DEFAULT_VERIFIER)
    parser.add_argument(
        "--d3dcompiler",
        type=Path,
        default=Path(r"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\d3dcompiler_47.dll"),
    )
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--run-hlsl", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--shallow-check", action="store_true")
    return parser.parse_args(argv)


def main(argv: Iterable[str] | None = None) -> int:
    args = parse_args(argv)
    runtime_receipt = read_json(args.runtime_receipt)
    acquisition_receipt = read_json(args.acquisition_receipt)
    material_contract = read_json(args.material_contract)
    if args.shallow_check:
        stored = read_json(args.output)
        validate_policy_receipt(
            stored,
            runtime_receipt,
            acquisition_receipt,
            material_contract,
            args.runtime_receipt,
            args.acquisition_receipt,
            args.material_contract,
            args.hlsl,
            args.verifier,
        )
        print("PASS: Artist F reconstructed Material policy shallow rows=255 runtime=0 product=false")
        return 0
    candidate = build_from_paths(args)
    if args.check:
        require(args.output.is_file(), f"policy receipt is missing: {args.output}")
        require(read_json(args.output) == candidate, "reconstructed Material policy receipt is stale")
        print("PASS: Artist F reconstructed Material policy deep rows=255 warp=107 runtime=0 product=false")
        return 0
    write_json_atomic(args.output, candidate)
    print(f"wrote {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
