#!/usr/bin/env python3
"""Build the fail-closed Artist F reconstructed Material numeric oracle.

The evaluator in this file is an explicit reconstruction.  It never upgrades
the cooked Material graph to SOURCE_EXACT.  Its purpose is to give the later
runtime MaterialBinding compiler a versioned arithmetic contract whose CPU and
HLSL implementations can be compared without image-based validation.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import struct
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any, Iterable

from artist_31470_material_evidence_approval import (
    APPROVED_CONTROLLED_CAPTURE_ASSESSMENT_SHA256,
    APPROVED_EVALUATOR_CONTRACT_SHA256,
    APPROVED_HLSL_REPLAY_BINDING_SHA256,
    APPROVED_WARP_STATE_PILOT_PROJECTION_SHA256,
    PINNED_COMPILED_DXBC_SHA256,
    PINNED_D3DCOMPILER_BYTE_SIZE,
    PINNED_D3DCOMPILER_SHA256,
    PINNED_HLSL_INPUT_BYTES_SHA256,
    PINNED_HLSL_MAX_ABSOLUTE_ERROR,
    PINNED_HLSL_OUTPUT_BYTES_SHA256,
    PINNED_HLSL_TRACKED_TEXT_SHA256,
)
from build_artist_31470_material_evidence_contract import validate_contract
from extract_artist_31470_shader_cache_oracle import validate_receipt as validate_shader_receipt
from extract_ue3_placements import (
    LOSTARK_KR_AES_KEY,
    WindowsAesEcbDecryptor,
    decompress_chunk,
    parse_name_table,
    parse_summary,
)


SCHEMA = "lostark.artist-31470-material-runtime-oracle-receipt"
FORMAT_VERSION = 3
EVALUATOR_VERSION = 1
NUMERIC_TOLERANCE = 2.0e-5
REPO_ROOT = Path(__file__).resolve().parents[2]
GENERATOR_PATH = Path(__file__).resolve()
MATERIAL_CONTRACT_BUILDER_PATH = Path(
    __file__
).resolve().with_name("build_artist_31470_material_evidence_contract.py")
SHADER_CACHE_ORACLE_PATH = Path(
    __file__
).resolve().with_name("extract_artist_31470_shader_cache_oracle.py")
UE3_PACKAGE_PARSER_PATH = Path(
    __file__
).resolve().with_name("extract_ue3_placements.py")
HLSL_VERIFIER_PATH = Path(
    __file__
).resolve().with_name("verify_artist_31470_material_runtime_oracle_hlsl.py")
SOURCE_VALUE_ACQUISITION_GENERATOR_PATH = Path(__file__).resolve().with_name(
    "build_artist_31470_material_source_value_acquisition.py"
)
MATERIAL_EVIDENCE_APPROVAL_PATH = Path(__file__).resolve().with_name(
    "artist_31470_material_evidence_approval.py"
)
DEFAULT_MATERIAL_CONTRACT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.typed-material-evidence-contract.json"
)
DEFAULT_RENDER_RECEIPT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-render-state-evidence.receipt.json"
)
DEFAULT_SHADER_RECEIPT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.shader-cache-oracle.receipt.json"
)
DEFAULT_SOURCE_VALUE_ACQUISITION_RECEIPT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-source-value-acquisition.receipt.json"
)
DEFAULT_HLSL = REPO_ROOT / (
    "Tools/MaterialEvaluatorHarness/Shader_Artist31470MaterialOracle.hlsl"
)
DEFAULT_OUTPUT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-runtime-oracle.receipt.json"
)
DEFAULT_SOURCE_ARCHIVE_ROOT = Path(
    r"C:\Users\user\Desktop\Resource_LostArk\00_SourcePackages"
)

PINNED_SOURCE_ARCHIVE_PROJECTION = {
    "scope": "LOCAL_SOURCE_ARCHIVE_RAW_UPK_NAME_TABLE_INVENTORY",
    "hashRole": "EXTERNAL_RAW_BYTES",
    "fileCount": 1813,
    "duplicateContentFileCount": 1189,
    "uniquePackageContentCount": 624,
    "physicalByteCountAllFiles": 1932762844,
    "physicalByteCountUniqueContent": 795157410,
    "inventoryProjectionSha256": "60922d43d423006e9a7868bbf5eef8cd68bddeeeef38113098822cc30c7fbbec",
    "shaderCacheNameCandidateCount": 0,
    "shaderCacheCandidateProjectionSha256": "4f53cda18c2baa0c0354bb5f9a3ecbe5ed12ab4d8e11ba873c2f11161202b945",
    "decision": "SOURCE_REVISION_SHADER_CACHE_NOT_PRESENT_IN_SCANNED_ARCHIVE",
}


FEATURE_SECOND_TEXTURE = 1 << 0
FEATURE_UV_TRANSFORM = 1 << 1
FEATURE_PANNER = 1 << 2
FEATURE_COLOR = 1 << 3
FEATURE_DESATURATION = 1 << 4
FEATURE_POWER = 1 << 5
FEATURE_DISSOLVE = 1 << 6
FEATURE_FRESNEL = 1 << 7
FEATURE_DISTORTION = 1 << 8
FEATURE_ALPHA = 1 << 9

FEATURES = (
    ("SECOND_TEXTURE", FEATURE_SECOND_TEXTURE),
    ("UV_TRANSFORM", FEATURE_UV_TRANSFORM),
    ("PANNER", FEATURE_PANNER),
    ("COLOR", FEATURE_COLOR),
    ("DESATURATION", FEATURE_DESATURATION),
    ("POWER", FEATURE_POWER),
    ("DISSOLVE", FEATURE_DISSOLVE),
    ("FRESNEL", FEATURE_FRESNEL),
    ("DISTORTION", FEATURE_DISTORTION),
    ("ALPHA", FEATURE_ALPHA),
)

EVALUATOR_OPERATION_ORDER = (
    "SECOND_TEXTURE_MULTIPLY",
    "UV_TRANSFORM_PHASE",
    "PANNER_PHASE",
    "COLOR_MULTIPLY",
    "DESATURATION",
    "SIGNED_POWER",
    "FRESNEL_GAIN",
    "DISTORTION_OFFSET",
    "DISSOLVE_ALPHA",
    "ALPHA_MULTIPLY",
)

FEATURE_TOKENS = {
    "UV_TRANSFORM": ("uv", "texcoord", "tile", "scale", "coord"),
    "PANNER": ("pan", "panning", "move", "speed", "time", "rotat"),
    "COLOR": ("color", "emiss", "diff", "spec", "light", "transmission"),
    "DESATURATION": ("desat",),
    "POWER": ("power", "pow", "strength", "intensity", "str"),
    "DISSOLVE": ("dissolve", "disslove", "transition", "edge"),
    "FRESNEL": ("fresn", "camera"),
    "DISTORTION": ("distort", "noise", "flow"),
    "ALPHA": ("alpha", "opacity", "mask", "falloff", "depth"),
}


ORACLE_INPUTS = (
    {
        "sampleId": "sample-0",
        "time": 0.0,
        "uvScale": [1.0, 1.0],
        "panRotationAux": [0.0, 0.0, 0.0, 0.0],
        "texture0": [0.2, 0.4, 0.6, 0.8],
        "texture1": [0.9, 0.7, 0.5, 0.3],
        "color": [1.0, 0.75, 0.5, 1.25],
        "params0": [1.2, 1.5, 0.25, 0.9],
        "params1": [0.35, 0.6, 0.15, 0.0],
    },
    {
        "sampleId": "sample-1",
        "time": 0.25,
        "uvScale": [2.0, 0.5],
        "panRotationAux": [0.1, -0.2, 0.3, 0.4],
        "texture0": [0.95, 0.1, 0.35, 0.65],
        "texture1": [0.2, 0.8, 0.45, 0.9],
        "color": [0.6, 1.1, 0.8, 0.75],
        "params0": [2.0, 0.75, 0.8, 1.1],
        "params1": [0.2, 0.35, -0.1, 0.0],
    },
    {
        "sampleId": "sample-2",
        "time": 1.0,
        "uvScale": [-1.0, 1.5],
        "panRotationAux": [-0.35, 0.6, -0.2, 0.0],
        "texture0": [0.0, 0.25, 1.0, 0.45],
        "texture1": [1.0, 0.5, 0.0, 0.75],
        "color": [1.4, 0.4, 1.2, 0.5],
        "params0": [0.5, 2.25, 0.5, 0.6],
        "params1": [0.55, 0.9, 0.25, 0.0],
    },
    {
        "sampleId": "sample-3",
        "time": 3.5,
        "uvScale": [0.125, -2.0],
        "panRotationAux": [1.25, -0.75, 0.9, -0.5],
        "texture0": [0.33, 0.66, 0.99, 0.12],
        "texture1": [0.77, 0.11, 0.55, 0.88],
        "color": [0.9, 0.9, 1.3, 2.0],
        "params0": [3.0, 1.1, 1.0, 1.4],
        "params1": [0.05, 0.2, 0.5, 0.0],
    },
)

RECIPE_TEXTURE_PROBES = (
    ([0.17, 0.41, 0.73, 0.29], [0.83, 0.19, 0.61, 0.97]),
    ([0.91, 0.07, 0.37, 0.63], [0.23, 0.79, 0.13, 0.47]),
    ([0.05, 0.55, 0.95, 0.35], [0.75, 0.45, 0.15, 0.85]),
    ([0.31, 0.69, 0.27, 0.81], [0.59, 0.11, 0.89, 0.21]),
)

RENDER_STATE_FIELDS = (
    "blendmode",
    "lightingmodel",
    "twosided",
    "bdisabledepthtest",
    "opacitymaskclipvalue",
    "buseonelayerdistortion",
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def strict_object(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        require(key not in result, f"duplicate JSON key: {key}")
        result[key] = value
    return result


def reject_nonfinite_json_constant(token: str) -> None:
    raise ValueError(f"non-finite JSON constant is forbidden: {token}")


def read_json(path: Path) -> dict[str, Any]:
    value = json.loads(
        path.read_text(encoding="utf-8-sig"),
        object_pairs_hook=strict_object,
        parse_constant=reject_nonfinite_json_constant,
    )
    require(isinstance(value, dict), f"expected JSON object: {path}")
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


def raw_sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def tracked_text_sha256(path: Path) -> str:
    text = path.read_bytes()
    if text.startswith(b"\xef\xbb\xbf"):
        text = text[3:]
    return raw_sha256(text.replace(b"\r\n", b"\n").replace(b"\r", b"\n"))


def seal_receipt(receipt: dict[str, Any]) -> None:
    receipt.pop("receiptSha256", None)
    receipt["receiptSha256"] = canonical_sha256(receipt)


def expected_evaluator_contract() -> dict[str, Any]:
    return {
        "version": EVALUATOR_VERSION,
        "operationOrder": list(EVALUATOR_OPERATION_ORDER),
        "inputSampleCountPerFamily": len(ORACLE_INPUTS),
        "inputSamples": copy.deepcopy(list(ORACLE_INPUTS)),
        "numericTolerance": NUMERIC_TOLERANCE,
        "fidelity": "RECONSTRUCTED_NUMERICALLY_VERIFIED",
        "sourceExact": False,
    }


def validate_source_value_acquisition_semantics(
    source_value_acquisition: dict[str, Any],
    material_contract: dict[str, Any],
) -> None:
    """Validate the raw/source-value meaning, not only the receipt self hash.

    The import stays local because the acquisition generator reuses this module's
    matrix builder while constructing its own receipt.  Runtime entry points call
    this only after both modules have finished loading.
    """

    from build_artist_31470_material_source_value_acquisition import (
        validate_receipt_semantics,
    )

    validate_receipt_semantics(source_value_acquisition, material_contract)


def hlsl_replay_binding_sha256(
    receipt: dict[str, Any], verification_without_binding: dict[str, Any]
) -> str:
    projection = {
        "evaluatorContractSha256": canonical_sha256(
            receipt.get("evaluatorContract")
        ),
        "familyEvaluatorProjection": [
            {
                "familyId": row.get("familyId"),
                "featureMask": row.get("featureMask"),
                "sampleRowsSha256": canonical_sha256(row.get("sampleRows")),
            }
            for row in receipt.get("familyEvaluators") or []
        ],
        "recipeBindingProjection": [
            {
                "recipeId": row.get("recipeId"),
                "bindingSha256": row.get("bindingSha256"),
                "numericBindingSamplesSha256": canonical_sha256(
                    row.get("numericBindingSamples")
                ),
            }
            for row in receipt.get("materialRecipeBindings") or []
        ],
        "verification": copy.deepcopy(verification_without_binding),
    }
    return canonical_sha256(projection)


def feature_names(mask: int) -> list[str]:
    return [name for name, bit in FEATURES if mask & bit]


def classify_family_features(expressions: list[dict[str, Any]]) -> tuple[int, dict[str, list[str]]]:
    evidence: dict[str, set[str]] = defaultdict(set)
    texture_count = 0
    for expression in expressions:
        projection = expression.get("projection") or {}
        parameter = str(projection.get("parameterName") or "").casefold()
        class_name = str(projection.get("className") or expression.get("className") or "").casefold()
        texture_path = projection.get("textureObjectPath")
        if "texture" in class_name or isinstance(texture_path, str):
            texture_count += 1
        for feature, tokens in FEATURE_TOKENS.items():
            if any(token in parameter for token in tokens):
                evidence[feature].add(parameter)

    mask = 0
    if texture_count >= 2:
        mask |= FEATURE_SECOND_TEXTURE
        evidence["SECOND_TEXTURE"].add(f"texture-parameter-count:{texture_count}")
    for name, bit in FEATURES:
        if evidence.get(name):
            mask |= bit
    require(mask != 0, "reconstructed family has no surviving semantic feature")
    return mask, {key: sorted(values) for key, values in sorted(evidence.items())}


def classify_binding_role(field: dict[str, Any]) -> str:
    kind = str(field.get("fieldKind") or "").casefold()
    name = str(field.get("normalizedParameterName") or field.get("parameterName") or "").casefold()
    require(kind in {"scalar", "vector", "texture"}, "unsupported Material input kind")
    require(name, "Material input name is blank")
    if kind == "texture":
        if "normal" in name or "spec" in name or "height" in name:
            return "NORMAL_SPEC_TEXTURE"
        if any(token in name for token in ("noise", "distort", "flow")):
            return "NOISE_TEXTURE"
        if any(token in name for token in ("alpha", "mask", "dissolve", "disslove", "opacity")):
            return "MASK_TEXTURE"
        if any(token in name for token in ("emiss", "color", "diff", "main", "map", "tex")):
            return "COLOR_TEXTURE"
        return "AUXILIARY_EXPLICIT_TEXTURE"
    if kind == "vector":
        if any(token in name for token in ("color", "emiss", "diff", "spec", "light", "transmission", "reflection", "dissolve")):
            return "COLOR_VECTOR"
        if any(token in name for token in ("uv", "position", "positon", "coord")):
            return "UV_VECTOR"
        return "AUXILIARY_EXPLICIT_VECTOR"
    if any(token in name for token in ("uv", "texcoord", "tile", "scale", "coord")):
        return "UV_TRANSFORM_SCALAR"
    if any(token in name for token in ("pan", "panning", "move", "speed", "time", "rotat")):
        return "ANIMATION_SCALAR"
    if any(token in name for token in ("dissolve", "disslove", "transition", "edge")):
        return "DISSOLVE_SCALAR"
    if any(token in name for token in ("fresn", "camera")):
        return "FRESNEL_SCALAR"
    if any(token in name for token in ("distort", "noise", "flow")):
        return "DISTORTION_SCALAR"
    if any(token in name for token in ("alpha", "opacity", "mask", "falloff", "depth")):
        return "ALPHA_SCALAR"
    if any(token in name for token in ("color", "emiss", "diff", "spec", "light", "transmission")):
        return "COLOR_SCALAR"
    if any(token in name for token in ("power", "pow", "strength", "intensity", "str", "value", "velue", "bias", "radius", "hard")):
        return "GAIN_SHAPE_SCALAR"
    return "AUXILIARY_EXPLICIT_SCALAR"


def normalize_typed_value(field: dict[str, Any]) -> Any:
    kind = str(field.get("fieldKind") or "").casefold()
    value = field.get("value")
    if kind == "scalar":
        require(type(value) in {int, float} and math.isfinite(float(value)), "Material scalar value is invalid")
        return f32(float(value))
    if kind == "vector":
        if isinstance(value, list):
            require(len(value) == 4, "Material vector value must have four lanes")
            lanes = value
        else:
            require(isinstance(value, dict), "Material vector value is invalid")
            require(type(value.get("size")) is int and value["size"] == 16, "Material vector byte size changed")
            encoded = value.get("hex")
            require(isinstance(encoded, str) and len(encoded) == 32, "Material vector bytes are invalid")
            try:
                lanes = list(struct.unpack("<4f", bytes.fromhex(encoded)))
            except (ValueError, struct.error) as error:
                raise ValueError("Material vector bytes are invalid") from error
        require(all(type(lane) in {int, float} and math.isfinite(float(lane)) for lane in lanes), "Material vector lane is invalid")
        return [f32(float(lane)) for lane in lanes]
    if kind == "texture":
        require(isinstance(value, str) and value and len(value) <= 512, "Material texture path is invalid")
        return value
    if kind == "staticswitch":
        require(type(value) is bool, "Material static switch value is invalid")
        return value
    raise ValueError(f"unsupported typed Material value kind: {kind}")


def source_owner_recipe_id(field: dict[str, Any]) -> str:
    owner = (((field.get("provenance") or {}).get("lineage") or {}).get("owner") or {})
    recipe_id = owner.get("recipeId")
    require(isinstance(recipe_id, str) and recipe_id, "Material field source owner is missing")
    return recipe_id


def build_ordered_input_binding(
    recipe_id: str,
    source_section: str,
    source_section_index: int,
    field: dict[str, Any],
) -> dict[str, Any]:
    require(
        source_owner_recipe_id(field) == recipe_id,
        "Material input source owner recipe changed",
    )
    typed_value = normalize_typed_value(field)
    return {
        "fieldId": field["fieldId"],
        "fieldKind": str(field["fieldKind"]).casefold(),
        "sourceSection": source_section,
        "sourceSectionIndex": source_section_index,
        "sourceOwnerRecipeId": recipe_id,
        "bindingRole": classify_binding_role(field),
        "bindingOrigin": field["bindingOrigin"],
        "parameterName": field["parameterName"],
        "normalizedParameterName": field["normalizedParameterName"],
        "typedValue": typed_value,
        "typedValueSha256": canonical_sha256(typed_value),
        "sourceFieldValueSha256": canonical_sha256(field.get("value")),
        "sourceLineageSha256": field["provenance"]["lineageSha256"],
    }


def build_ordered_static_binding(
    recipe_id: str,
    source_section: str,
    source_section_index: int,
    field: dict[str, Any],
) -> dict[str, Any]:
    require(
        source_owner_recipe_id(field) == recipe_id,
        "Material static switch source owner recipe changed",
    )
    typed_value = normalize_typed_value(field)
    return {
        "fieldId": field["fieldId"],
        "sourceSection": source_section,
        "sourceSectionIndex": source_section_index,
        "sourceOwnerRecipeId": recipe_id,
        "parameterName": field["parameterName"],
        "normalizedParameterName": field["normalizedParameterName"],
        "bindingOrigin": field["bindingOrigin"],
        "selectionRole": field.get("selectionRole"),
        "typedValue": typed_value,
        "typedValueSha256": canonical_sha256(typed_value),
        "sourceFieldValueSha256": canonical_sha256(field.get("value")),
        "sourceLineageSha256": field["provenance"]["lineageSha256"],
    }


def family_expression_projection(
    expressions: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    return [
        {
            "evidenceId": row["evidenceId"],
            "sourceOrder": row["sourceOrder"],
            "rawReferenceFromBaseExpressions": row["rawReferenceFromBaseExpressions"],
            "className": row["className"],
            "objectPath": row["objectPath"],
            "projection": row["projection"],
            "serialSha256": row["serialSha256"],
        }
        for row in sorted(
            expressions, key=lambda row: (row["sourceOrder"], row["exportIndex"])
        )
    ]


def feature_mask_for_static_switches(
    family_mask: int, switches: list[dict[str, Any]]
) -> tuple[int, list[dict[str, Any]]]:
    mask = family_mask
    decisions: list[dict[str, Any]] = []
    for switch in switches:
        name = str(switch["normalizedParameterName"]).casefold()
        matched = [
            (feature, bit)
            for feature, bit in FEATURES
            if any(token in name for token in FEATURE_TOKENS.get(feature, ()))
        ]
        if not matched:
            continue
        value = bool(switch["typedValue"])
        for feature, bit in matched:
            if value:
                mask |= bit
            else:
                mask &= ~bit
            decisions.append(
                {
                    "fieldId": switch["fieldId"],
                    "feature": feature,
                    "value": value,
                }
            )
    require(mask != 0, "Material recipe static permutation disabled every evaluator feature")
    return mask, decisions


def build_recipe_operands(
    input_bindings: list[dict[str, Any]], recipe_feature_mask: int
) -> dict[str, Any]:
    by_role: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for field in input_bindings:
        by_role[field["bindingRole"]].append(field)

    def scalar_values(role: str) -> list[dict[str, Any]]:
        return [row for row in by_role.get(role, []) if row["fieldKind"] == "scalar"]

    def scalar(role: str, index: int, fallback: float) -> tuple[float, list[str]]:
        rows = scalar_values(role)
        if index >= len(rows):
            return f32(fallback), []
        return f32(rows[index]["typedValue"]), [rows[index]["fieldId"]]

    uv_x, uv_x_ids = scalar("UV_TRANSFORM_SCALAR", 0, 1.0)
    uv_y, uv_y_ids = scalar("UV_TRANSFORM_SCALAR", 1, 1.0)
    pan = []
    pan_ids: list[str] = []
    for index in range(4):
        value, ids = scalar("ANIMATION_SCALAR", index, 0.0)
        pan.append(value)
        pan_ids.extend(ids)

    color_rows = by_role.get("COLOR_VECTOR", [])
    if color_rows:
        color = list(color_rows[0]["typedValue"])
        color_ids = [color_rows[0]["fieldId"]]
    else:
        color = []
        color_ids = []
        for index in range(4):
            value, ids = scalar("COLOR_SCALAR", index, 1.0)
            color.append(value)
            color_ids.extend(ids)

    gain, gain_ids = scalar("GAIN_SHAPE_SCALAR", 0, 1.0)
    power, power_ids = scalar("GAIN_SHAPE_SCALAR", 1, 1.0)
    desaturation, desaturation_ids = scalar("AUXILIARY_EXPLICIT_SCALAR", 0, 0.0)
    alpha, alpha_ids = scalar("ALPHA_SCALAR", 0, 1.0)
    dissolve, dissolve_ids = scalar("DISSOLVE_SCALAR", 0, 0.5)
    fresnel, fresnel_ids = scalar("FRESNEL_SCALAR", 0, 0.5)
    distortion, distortion_ids = scalar("DISTORTION_SCALAR", 0, 0.0)

    lanes = {
        "uvScale": {"value": [uv_x, uv_y], "sourceFieldIds": uv_x_ids + uv_y_ids},
        "panRotationAux": {"value": pan, "sourceFieldIds": pan_ids},
        "color": {"value": color, "sourceFieldIds": color_ids},
        "params0": {
            "value": [gain, power, desaturation, alpha],
            "sourceFieldIds": gain_ids + power_ids + desaturation_ids + alpha_ids,
        },
        "params1": {
            "value": [dissolve, fresnel, distortion, 0.0],
            "sourceFieldIds": dissolve_ids + fresnel_ids + distortion_ids,
        },
    }
    return {
        "recipeFeatureMask": recipe_feature_mask,
        "lanes": lanes,
        "bindingProjectionSha256": canonical_sha256(lanes),
    }


def build_recipe_numeric_samples(
    recipe_feature_mask: int, operands: dict[str, Any]
) -> list[dict[str, Any]]:
    rows = []
    lanes = operands["lanes"]
    for index, source_sample in enumerate(ORACLE_INPUTS):
        texture0, texture1 = RECIPE_TEXTURE_PROBES[index]
        sample = {
            "sampleId": source_sample["sampleId"],
            "time": source_sample["time"],
            "uvScale": lanes["uvScale"]["value"],
            "panRotationAux": lanes["panRotationAux"]["value"],
            "texture0": texture0,
            "texture1": texture1,
            "color": lanes["color"]["value"],
            "params0": lanes["params0"]["value"],
            "params1": lanes["params1"]["value"],
        }
        rows.append(
            {
                "sampleId": source_sample["sampleId"],
                "input": sample,
                "inputSha256": canonical_sha256(sample),
                "expectedFloat4": evaluate_cpu(recipe_feature_mask, sample),
            }
        )
    return rows


def require_finite_number(value: Any, label: str) -> float:
    require(
        type(value) in {int, float} and not isinstance(value, bool),
        f"{label} is not an exact JSON number",
    )
    numeric = float(value)
    require(math.isfinite(numeric), f"{label} is non-finite")
    return numeric


def f32(value: float) -> float:
    numeric = require_finite_number(value, "float32 input")
    try:
        converted = struct.unpack("<f", struct.pack("<f", numeric))[0]
    except (OverflowError, struct.error) as error:
        raise ValueError("float32 input is out of range") from error
    require(math.isfinite(converted), "float32 conversion produced a non-finite value")
    return converted


def require_exact_f32_number(value: Any, label: str) -> float:
    numeric = require_finite_number(value, label)
    require(f32(numeric) == numeric, f"{label} is not an exact float32 value")
    return numeric


def validate_cpu_input_sample(sample: dict[str, Any]) -> None:
    expected_keys = {
        "sampleId",
        "time",
        "uvScale",
        "panRotationAux",
        "texture0",
        "texture1",
        "color",
        "params0",
        "params1",
    }
    require(isinstance(sample, dict) and set(sample) == expected_keys, "CPU sample schema changed")
    require(isinstance(sample.get("sampleId"), str) and sample["sampleId"], "CPU sample ID changed")
    f32(require_finite_number(sample["time"], "CPU sample time"))
    for field_name, width in (
        ("uvScale", 2),
        ("panRotationAux", 4),
        ("texture0", 4),
        ("texture1", 4),
        ("color", 4),
        ("params0", 4),
        ("params1", 4),
    ):
        lanes = sample.get(field_name)
        require(isinstance(lanes, list) and len(lanes) == width, f"CPU sample {field_name} width changed")
        for lane_index, lane in enumerate(lanes):
            f32(require_finite_number(lane, f"CPU sample {field_name}[{lane_index}]"))


def validate_cpu_float4(values: Any, label: str) -> list[float]:
    require(isinstance(values, list) and len(values) == 4, f"{label} width changed")
    return [
        require_exact_f32_number(value, f"{label}[{lane_index}]")
        for lane_index, value in enumerate(values)
    ]


def saturate(value: float) -> float:
    return min(1.0, max(0.0, value))


def evaluate_cpu(feature_mask: int, sample: dict[str, Any]) -> list[float]:
    validate_cpu_input_sample(sample)
    time = f32(sample["time"])
    uv_scale = [f32(value) for value in sample["uvScale"]]
    pan = [f32(value) for value in sample["panRotationAux"]]
    texture0 = [f32(value) for value in sample["texture0"]]
    texture1 = [f32(value) for value in sample["texture1"]]
    color = [f32(value) for value in sample["color"]]
    params0 = [f32(value) for value in sample["params0"]]
    params1 = [f32(value) for value in sample["params1"]]

    result = texture0[:]
    uv_x = f32(f32(0.375 * uv_scale[0]) + f32(pan[0] * time))
    uv_y = f32(f32(0.625 * uv_scale[1]) + f32(pan[1] * time))
    if feature_mask & FEATURE_SECOND_TEXTURE:
        result = [f32(result[index] * f32(0.5 + 0.5 * texture1[index])) for index in range(4)]
    if feature_mask & FEATURE_UV_TRANSFORM:
        uv_mod = f32((uv_x - math.floor(uv_x) + uv_y - math.floor(uv_y)) * 0.03125)
        result[0] = f32(result[0] + uv_mod)
        result[1] = f32(result[1] + uv_mod)
        result[2] = f32(result[2] + uv_mod)
    if feature_mask & FEATURE_PANNER:
        phase = f32((time * 0.125 + pan[2]) - math.floor(time * 0.125 + pan[2]))
        result[0] = f32(result[0] + phase * 0.015625)
        result[1] = f32(result[1] + phase * 0.015625)
        result[2] = f32(result[2] + phase * 0.015625)
    if feature_mask & FEATURE_COLOR:
        result[0] = f32(result[0] * color[0] * max(color[3], 0.0))
        result[1] = f32(result[1] * color[1] * max(color[3], 0.0))
        result[2] = f32(result[2] * color[2] * max(color[3], 0.0))
    if feature_mask & FEATURE_DESATURATION:
        luminance = f32(result[0] * 0.299 + result[1] * 0.587 + result[2] * 0.114)
        amount = saturate(params0[2])
        result[0] = f32(result[0] + (luminance - result[0]) * amount)
        result[1] = f32(result[1] + (luminance - result[1]) * amount)
        result[2] = f32(result[2] + (luminance - result[2]) * amount)
    if feature_mask & FEATURE_POWER:
        exponent = max(abs(params0[1]), 0.001)
        result[0] = f32(math.pow(abs(result[0]), exponent) * (1.0 if result[0] >= 0.0 else -1.0))
        result[1] = f32(math.pow(abs(result[1]), exponent) * (1.0 if result[1] >= 0.0 else -1.0))
        result[2] = f32(math.pow(abs(result[2]), exponent) * (1.0 if result[2] >= 0.0 else -1.0))
    if feature_mask & FEATURE_FRESNEL:
        gain = f32(1.0 + math.pow(saturate(params1[1]), max(abs(params0[1]), 0.001)))
        result[0] = f32(result[0] * gain)
        result[1] = f32(result[1] * gain)
        result[2] = f32(result[2] * gain)
    if feature_mask & FEATURE_DISTORTION:
        result[0] = f32(result[0] + f32((texture1[0] * 2.0 - 1.0) * params1[2]))
        result[1] = f32(result[1] + f32((texture1[1] * 2.0 - 1.0) * params1[2]))
    if feature_mask & FEATURE_DISSOLVE:
        result[3] = f32(saturate((result[3] - params1[0]) * max(abs(params0[0]), 0.001)))
    if feature_mask & FEATURE_ALPHA:
        result[3] = f32(saturate(result[3] * params0[3]))
    return [f32(value) for value in result]


def _name_table_only(path: Path) -> tuple[bytes, Any, list[str]]:
    physical = path.read_bytes()
    summary = parse_summary(physical)
    next_offsets = [
        offset
        for offset in (summary.import_offset, summary.export_offset, summary.depends_offset)
        if offset > summary.name_offset
    ]
    require(next_offsets, f"package has no NameTable boundary: {path.name}")
    table_end = min(next_offsets)
    logical = bytearray(table_end)
    first_offset = min(chunk.uncompressed_offset for chunk in summary.chunks)
    logical[: min(first_offset, table_end)] = physical[: min(first_offset, table_end)]
    with WindowsAesEcbDecryptor(LOSTARK_KR_AES_KEY.encode("ascii")) as aes:
        for chunk in summary.chunks:
            start = chunk.uncompressed_offset
            end = start + chunk.uncompressed_size
            if start < table_end and end > summary.name_offset:
                decoded = decompress_chunk(physical, chunk, aes)
                copy_start = max(start, 0)
                copy_end = min(end, table_end)
                logical[copy_start:copy_end] = decoded[
                    copy_start - start : copy_end - start
                ]
    return physical, summary, parse_name_table(bytes(logical), summary)


def scan_source_archive(root: Path) -> dict[str, Any]:
    require(root.is_dir(), f"source archive root is missing: {root}")
    paths = sorted(root.rglob("*.upk"), key=lambda item: str(item).casefold())
    require(paths, "source archive contains no UPK files")
    by_sha: dict[str, dict[str, Any]] = {}
    physical_all = 0
    for path in paths:
        data = path.read_bytes()
        physical_all += len(data)
        digest = raw_sha256(data)
        row = by_sha.get(digest)
        if row is not None:
            row["aliases"].append(path.name)
            continue
        _, summary, names = _name_table_only(path)
        hits = sorted(
            {
                name
                for name in names
                if "shadercache" in name.casefold()
                or name.casefold().startswith("sc_lv_")
            },
            key=str.casefold,
        )
        by_sha[digest] = {
            "rawSha256": digest,
            "byteSize": len(data),
            "packageVersion": summary.version,
            "licenseeVersion": summary.licensee_version,
            "exportCount": summary.export_count,
            "aliases": [path.name],
            "shaderCacheNameHits": hits,
        }
    rows = []
    for row in by_sha.values():
        row = dict(row)
        row["aliases"] = sorted(set(row["aliases"]), key=str.casefold)
        rows.append(row)
    rows.sort(key=lambda row: row["rawSha256"])
    candidates = [row for row in rows if row["shaderCacheNameHits"]]
    return {
        "scope": "LOCAL_SOURCE_ARCHIVE_RAW_UPK_NAME_TABLE_INVENTORY",
        "hashRole": "EXTERNAL_RAW_BYTES",
        "fileCount": len(paths),
        "duplicateContentFileCount": len(paths) - len(rows),
        "uniquePackageContentCount": len(rows),
        "physicalByteCountAllFiles": physical_all,
        "physicalByteCountUniqueContent": sum(row["byteSize"] for row in rows),
        "inventoryProjectionSha256": canonical_sha256(rows),
        "shaderCacheNameCandidateCount": len(candidates),
        "shaderCacheCandidateProjectionSha256": canonical_sha256(candidates),
        "decision": (
            "SOURCE_REVISION_SHADER_CACHE_NOT_PRESENT_IN_SCANNED_ARCHIVE"
            if not candidates
            else "SOURCE_REVISION_SHADER_CACHE_CANDIDATE_REQUIRES_FULL_DECODE"
        ),
    }


RENDER_STATE_CONSUMERS = {
    "lightingmodel": "MATERIAL_SHADER_LIGHTING_PERMUTATION",
    "twosided": "D3D11_RASTERIZER_DESC.CullMode",
    "bdisabledepthtest": "D3D11_DEPTH_STENCIL_DESC.DepthEnable",
    "opacitymaskclipvalue": "PIXEL_SHADER_OPACITY_MASK_CLIP_THRESHOLD",
    "buseonelayerdistortion": "EFFECT_DISTORTION_COMPOSITE_PATH",
}

RENDER_STATE_PILOTS = {
    "twosided": ["warp-rasterizer-two-sided-toggle"],
    "bdisabledepthtest": ["warp-depth-disable-toggle"],
}


def compact_export_identity(value: dict[str, Any] | None) -> dict[str, Any]:
    source = value or {}
    return {
        key: source.get(key)
        for key in (
            "evidenceId",
            "logicalPackage",
            "physicalPackage",
            "physicalPackageSha256",
            "exportIndex",
            "objectPath",
            "serialSha256",
            "rawExportEvidenceId",
        )
        if source.get(key) is not None
    }


def feasibility_row_id(kind: str, recipe_id: str, field_id: str) -> str:
    digest = canonical_sha256(
        {"kind": kind, "recipeId": recipe_id, "fieldId": field_id}
    )
    return f"material-feasibility-{kind}-{digest[:16]}"


def unavailable_identity(reason: str) -> dict[str, Any]:
    return {"available": False, "outcome": reason}


def build_material_feasibility_matrices(
    material_contract: dict[str, Any],
    shader_receipt: dict[str, Any],
    warp_state_verification: dict[str, Any] | None,
    source_value_acquisition: dict[str, Any] | None = None,
) -> dict[str, Any]:
    occurrences_by_recipe: dict[str, list[str]] = defaultdict(list)
    for occurrence in material_contract.get("occurrences") or []:
        occurrences_by_recipe[occurrence["materialRecipeId"]].append(
            occurrence["occurrenceId"]
        )
    for occurrence_ids in occurrences_by_recipe.values():
        occurrence_ids.sort()

    shader_summary = shader_receipt.get("summary") or {}
    shader_identity = {
        "available": False,
        "receiptSha256": shader_receipt.get("receiptSha256"),
        "exactMaterialShaderMapJoinCount": shader_summary.get(
            "exactMaterialShaderMapJoinCount"
        ),
        "sourceBaseMaterialIdJoinCount": shader_summary.get(
            "sourceBaseMaterialIdJoinCount"
        ),
        "sourceMicStaticParameterSetJoinCount": shader_summary.get(
            "sourceMicStaticParameterSetJoinCount"
        ),
        "outcome": "NO_SOURCE_REVISION_MATERIAL_OR_MIC_SHADER_MAP_JOIN",
    }
    capture_identity = unavailable_identity(
        "NO_SOURCE_REVISION_CONTROLLED_RUNTIME_CAPTURE_PROVIDER"
    )
    state_provider_verified = bool(
        warp_state_verification and warp_state_verification.get("verified")
    )
    native_by_recipe: dict[str, dict[str, Any]] = {}
    for native_row in shader_receipt.get("recipeNativeKeys") or []:
        recipe_id = str(native_row.get("recipeId") or "")
        require(
            bool(recipe_id) and recipe_id not in native_by_recipe,
            "duplicate or blank MIC native recipe identity",
        )
        native_by_recipe[recipe_id] = native_row
    require(
        len(native_by_recipe) == 27,
        "MIC native recipe denominator changed",
    )

    render_rows: list[dict[str, Any]] = []
    static_rows: list[dict[str, Any]] = []
    sampler_rows: list[dict[str, Any]] = []

    for recipe in sorted(
        material_contract.get("materialRecipes") or [],
        key=lambda row: row["recipeId"],
    ):
        recipe_id = recipe["recipeId"]
        occurrence_ids = occurrences_by_recipe.get(recipe_id, [])
        identity = recipe.get("identity") or {}
        instance_identity = compact_export_identity(identity.get("rawMaterialExport"))
        parent_identity = compact_export_identity(identity.get("selectedGraphIdentity"))

        render_fields = (recipe.get("renderState") or {}).get("fields") or {}
        for field_name in RENDER_STATE_FIELDS:
            source_field = render_fields[field_name]
            if source_field.get("status") != "OMITTED_FROM_EXPORT":
                continue
            pilot_ids = RENDER_STATE_PILOTS.get(field_name, [])
            provider_available = state_provider_verified and bool(pilot_ids)
            render_rows.append(
                {
                    "matrixRowId": feasibility_row_id(
                        "render-state", recipe_id, field_name
                    ),
                    "materialRecipeId": recipe_id,
                    "materialOccurrenceIds": occurrence_ids,
                    "fieldId": f"{recipe_id}:{field_name}",
                    "fieldKind": "RENDER_STATE_DEFAULT",
                    "bindingOriginAndOwner": {
                        "bindingOrigin": source_field.get("bindingOrigin"),
                        "evidenceOwnerRecipeId": recipe_id,
                    },
                    "instanceRecordIdentity": {
                        "available": bool(instance_identity),
                        "identity": instance_identity,
                        "fieldOutcome": "OMITTED_FROM_INSTANCE_EXPORT",
                    },
                    "parentIdentity": {
                        "available": bool(parent_identity),
                        "identity": parent_identity,
                        "fieldOutcome": "OMITTED_FROM_PARENT_EXPORT",
                    },
                    "nestedDefaultIdentity": unavailable_identity(
                        "NO_SERIALIZED_NESTED_DEFAULT_RECORD"
                    ),
                    "classCdoIdentity": unavailable_identity(
                        "SOURCE_REVISION_MATERIAL_CDO_NOT_ACQUIRED"
                    ),
                    "shaderCacheIdentity": shader_identity,
                    "runtimeCaptureIdentity": capture_identity,
                    "rendererConsumption": {
                        "consumer": RENDER_STATE_CONSUMERS[field_name],
                        "status": "FINAL_RUNTIME_CONSUMER_NOT_IMPLEMENTED",
                    },
                    "acquisitionPath": [
                        "INSTANCE_RECORD",
                        "PARENT_MATERIAL",
                        "NESTED_DEFAULT",
                        "CLASS_CDO",
                        "SOURCE_REVISION_SHADER_CACHE",
                        "CONTROLLED_RUNTIME_CAPTURE",
                    ],
                    "oracleProvider": {
                        "providerId": "D3D11_WARP_STATE_DESCRIPTOR_PILOT"
                        if pilot_ids
                        else "NONE",
                        "providerAvailable": provider_available,
                        "sourceValueProviderAvailable": False,
                    },
                    "pilotFixtureIds": pilot_ids,
                    "numericOracleInputDomain": [False, True]
                    if pilot_ids
                    else [],
                    "numericOracleExpectedOutput": (
                        "EXACT_D3D11_STATE_DESCRIPTOR_MUTATION"
                        if pilot_ids
                        else None
                    ),
                    "numericTolerance": 0.0 if pilot_ids else None,
                    "pilotDecision": (
                        "PROVIDER_PILOT_PASS_SOURCE_VALUE_UNAVAILABLE"
                        if provider_available
                        else "BLOCKED_NO_APPLICABLE_SOURCE_VALUE_PILOT"
                    ),
                    "fidelityDecision": "UNRESOLVED_DEFAULT_PROVENANCE",
                    "executionDecision": "BLOCKED",
                    "owner": "MATERIAL_CORRECTIVE_EVIDENCE_OWNER",
                    "finalRuntimeOwner": "G09_MATERIAL_BINDING_AND_RENDERER_CONSUMER",
                    "remainingBlockers": [
                        "RENDER_STATE_DEFAULT_PROVENANCE_UNRESOLVED",
                        "SOURCE_VALUE_PROVIDER_UNAVAILABLE",
                        "FINAL_RUNTIME_CONSUMER_NOT_IMPLEMENTED",
                    ],
                }
            )

        static_permutation = recipe.get("staticPermutation") or {}
        native_row = native_by_recipe[recipe_id]
        static_parameter_set = native_row.get("staticParameterSet")
        native_switches = (
            static_parameter_set.get("staticSwitchParameters") or []
            if isinstance(static_parameter_set, dict)
            else []
        )
        for source_section in ("selectedParameters", "parentDefaults"):
            for source_index, field in enumerate(
                static_permutation.get(source_section) or []
            ):
                field_id = field["fieldId"]
                expression_guid_hex = str(field.get("expressionGuidHex") or "")
                require(
                    len(expression_guid_hex) == 32,
                    f"static ExpressionGUID is missing: {field_id}",
                )
                name_matches = [
                    row
                    for row in native_switches
                    if str(row.get("parameterName") or "").casefold()
                    == str(field.get("parameterName") or "").casefold()
                ]
                guid_matches = [
                    row
                    for row in name_matches
                    if row.get("expressionGuidHex") == expression_guid_hex
                ]
                require(
                    len(guid_matches) <= 1,
                    f"ambiguous MIC static GUID join: {field_id}",
                )
                matched = guid_matches[0] if guid_matches else None
                source_value_acquired = bool(
                    matched is not None and matched.get("bOverride") is True
                )
                if matched is None:
                    selection_outcome = "NO_EXACT_GUID_NATIVE_ENTRY"
                    fidelity_decision = (
                        "SOURCE_EXACT_PARENT_DEFAULT_ONLY_NATIVE_ENTRY_ABSENT"
                    )
                    remaining_blockers = [
                        "STATIC_PERMUTATION_SELECTION_UNRESOLVED",
                        "NO_EXACT_GUID_NATIVE_ENTRY",
                        "STATIC_PERMUTATION_CONSUMER_OUTPUT_PILOT_MISSING",
                        "FINAL_RUNTIME_CONSUMER_NOT_IMPLEMENTED",
                    ]
                elif matched.get("bOverride") is True:
                    selection_outcome = "EXACT_GUID_INSTANCE_OVERRIDE_ENTRY"
                    fidelity_decision = (
                        "SOURCE_EXACT_INSTANCE_OVERRIDE_VALUE_ACQUIRED"
                    )
                    remaining_blockers = [
                        "STATIC_PERMUTATION_CONSUMER_OUTPUT_PILOT_MISSING",
                        "FINAL_RUNTIME_CONSUMER_NOT_IMPLEMENTED",
                    ]
                else:
                    require(
                        matched.get("value") is field.get("value"),
                        f"MIC nonoverride value disagrees with parent default: {field_id}",
                    )
                    selection_outcome = "EXACT_GUID_NONOVERRIDE_ENTRY"
                    fidelity_decision = (
                        "SOURCE_EXACT_NONOVERRIDE_ENTRY_OBSERVED_"
                        "INHERITANCE_SEMANTICS_UNVERIFIED"
                    )
                    remaining_blockers = [
                        "NONOVERRIDE_STATIC_INHERITANCE_SEMANTICS_UNVERIFIED",
                        "STATIC_PERMUTATION_CONSUMER_OUTPUT_PILOT_MISSING",
                        "FINAL_RUNTIME_CONSUMER_NOT_IMPLEMENTED",
                    ]
                native_selection = {
                    "staticParameterSetPresent": isinstance(
                        static_parameter_set, dict
                    ),
                    "nameMatchCount": len(name_matches),
                    "exactNameAndGuidMatchCount": len(guid_matches),
                    "selectionOutcome": selection_outcome,
                }
                if matched is not None:
                    native_selection["nativeTail"] = {
                        "physicalPackage": native_row["physicalPackage"],
                        "physicalPackageSha256": native_row[
                            "physicalPackageSha256"
                        ],
                        "exportIndex": native_row["exportIndex"],
                        "serialSha256": native_row["serialSha256"],
                        "propertyStreamEnd": native_row["propertyStreamEnd"],
                        "nativeTailByteCount": native_row["nativeTailByteCount"],
                        "nativeTailSha256": native_row["nativeTailSha256"],
                        "staticParameterSetOffset": static_parameter_set["offset"],
                        "staticParameterSetByteSize": static_parameter_set[
                            "byteSize"
                        ],
                        "staticParameterSetRawSha256": static_parameter_set[
                            "rawSha256"
                        ],
                        "staticParameterSetSemanticSha256": static_parameter_set[
                            "semanticSha256"
                        ],
                    }
                    native_selection["entry"] = {
                        "parameterName": matched["parameterName"],
                        "entryOffset": matched["entryOffset"],
                        "value": matched["value"],
                        "bOverride": matched["bOverride"],
                        "expressionGuidHex": matched["expressionGuidHex"],
                    }
                static_rows.append(
                    {
                        "matrixRowId": feasibility_row_id(
                            "static", recipe_id, field_id
                        ),
                        "materialRecipeId": recipe_id,
                        "materialOccurrenceIds": occurrence_ids,
                        "fieldId": field_id,
                        "fieldKind": "STATIC_PERMUTATION_SELECTION",
                        "bindingOriginAndOwner": {
                            "bindingOrigin": field.get("bindingOrigin"),
                            "evidenceOwnerRecipeId": source_owner_recipe_id(field),
                            "sourceSection": source_section,
                            "sourceSectionIndex": source_index,
                        },
                        "instanceRecordIdentity": {
                            "available": bool(instance_identity),
                            "identity": instance_identity,
                            "selectionOutcome": selection_outcome,
                            "micNativeSelection": native_selection,
                        },
                        "parentIdentity": {
                            "available": bool(parent_identity),
                            "identity": parent_identity,
                            "defaultLineageSha256": field["provenance"][
                                "lineageSha256"
                            ],
                            "selectionRole": field.get("selectionRole"),
                            "parameterName": field.get("parameterName"),
                            "parentDefaultValue": field.get("value"),
                            "expressionGuidHex": expression_guid_hex,
                            "defaultValuePropertyRecordSha256": field[
                                "provenance"
                            ]["valuePropertyRecordSha256"],
                            "expressionGuidPropertyRecordSha256": field[
                                "provenance"
                            ]["expressionGuidPropertyRecordSha256"],
                        },
                        "nestedDefaultIdentity": unavailable_identity(
                            "NO_ADDITIONAL_STATIC_SELECTION_RECORD"
                        ),
                        "classCdoIdentity": unavailable_identity(
                            "SOURCE_REVISION_STATIC_PARAMETER_CDO_NOT_ACQUIRED"
                        ),
                        "shaderCacheIdentity": shader_identity,
                        "runtimeCaptureIdentity": capture_identity,
                        "rendererConsumption": {
                            "consumer": "MATERIAL_STATIC_SHADER_PERMUTATION",
                            "status": "FINAL_RUNTIME_CONSUMER_NOT_IMPLEMENTED",
                        },
                        "acquisitionPath": [
                            "INSTANCE_MIC_STATIC_SELECTION",
                            "PARENT_EXPRESSION_DEFAULT",
                            "SOURCE_REVISION_SHADER_CACHE",
                            "CONTROLLED_RUNTIME_CAPTURE",
                        ],
                        "oracleProvider": {
                            "providerId": "SOURCE_MIC_NATIVE_STATIC_PARAMETER_SET",
                            "providerAvailable": matched is not None,
                            "parentDefaultIsNotInstanceSelection": True,
                            "sourceValueAcquired": source_value_acquired,
                        },
                        "pilotFixtureIds": [],
                        "numericOracleInputDomain": [False, True],
                        "numericOracleExpectedOutput": None,
                        "numericTolerance": None,
                        "pilotDecision": (
                            "SOURCE_VALUE_ACQUIRED_CONSUMER_OUTPUT_PILOT_MISSING"
                            if source_value_acquired
                            else "BLOCKED_NO_SOURCE_SELECTION_OUTPUT_PILOT"
                        ),
                        "sourceValueAcquired": source_value_acquired,
                        "fidelityDecision": fidelity_decision,
                        "executionDecision": "BLOCKED",
                        "owner": "MATERIAL_CORRECTIVE_EVIDENCE_OWNER",
                        "finalRuntimeOwner": "G09_STATIC_PERMUTATION_COMPILER",
                        "remainingBlockers": remaining_blockers,
                    }
                )

        inputs = recipe.get("inputs") or {}
        sampler_sources: list[tuple[str, int, dict[str, Any]]] = [
            ("textureOverrides", source_index, field)
            for source_index, field in enumerate(inputs.get("textureOverrides") or [])
        ]
        sampler_sources.extend(
            ("parentDefaults", source_index, field)
            for source_index, field in enumerate(inputs.get("parentDefaults") or [])
            if field.get("fieldKind") == "texture"
            and field.get("sampler", {}).get("fidelity")
            == "UNRESOLVED_SAMPLER_PROVENANCE"
        )
        for source_section, source_index, field in sampler_sources:
            sampler = field.get("sampler") or {}
            require(
                sampler.get("fidelity")
                in {"UNRESOLVED", "UNRESOLVED_SAMPLER_PROVENANCE"},
                f"unexpected sampler fidelity reached strict matrix: {field.get('fieldId')}",
            )
            field_id = field["fieldId"]
            source_texture_evidence = sampler.get("sourceTextureEvidence")
            sampler_blocker = str(
                sampler.get("blocker") or "SAMPLER_EVIDENCE_MISSING"
            )
            sampler_rows.append(
                {
                    "matrixRowId": feasibility_row_id(
                        "sampler", recipe_id, field_id
                    ),
                    "materialRecipeId": recipe_id,
                    "materialOccurrenceIds": occurrence_ids,
                    "fieldId": field_id,
                    "fieldKind": (
                        "DIRECT_TEXTURE_SAMPLER"
                        if source_section == "textureOverrides"
                        else "PARENT_DEFAULT_TEXTURE_SAMPLER"
                    ),
                    "bindingOriginAndOwner": {
                        "bindingOrigin": field.get("bindingOrigin"),
                        "evidenceOwnerRecipeId": source_owner_recipe_id(field),
                        "sourceSection": source_section,
                        "sourceSectionIndex": source_index,
                    },
                    "instanceRecordIdentity": {
                        "available": True,
                        "identity": instance_identity,
                        "textureObjectPath": field.get("value"),
                        "sourceLineageSha256": field["provenance"][
                            "lineageSha256"
                        ],
                        "samplerOutcome": sampler_blocker,
                    },
                    "parentIdentity": {
                        "available": bool(parent_identity),
                        "identity": parent_identity,
                        "samplerOutcome": (
                            "SOURCE_TEXTURE_EXPORT_PARTIAL_FIELDS_ONLY"
                            if isinstance(source_texture_evidence, dict)
                            else "NO_MATCHED_TEXTURE_EXPORT_SAMPLER_RECORD"
                        ),
                    },
                    "nestedDefaultIdentity": unavailable_identity(
                        "NO_SERIALIZED_TEXTURE_DEFAULT_SAMPLER_RECORD"
                    ),
                    "classCdoIdentity": unavailable_identity(
                        "SOURCE_REVISION_TEXTURE2D_CDO_NOT_ACQUIRED"
                    ),
                    "shaderCacheIdentity": shader_identity,
                    "runtimeCaptureIdentity": capture_identity,
                    "sourceTextureEvidence": source_texture_evidence,
                    "previouslyAdmittedExactSamplerReaudit": (
                        sampler.get("fidelity")
                        == "UNRESOLVED_SAMPLER_PROVENANCE"
                    ),
                    "rendererConsumption": {
                        "consumer": "D3D11_SAMPLER_DESC_AND_SRGB_SRV_FORMAT",
                        "status": "FINAL_RUNTIME_CONSUMER_NOT_IMPLEMENTED",
                    },
                    "acquisitionPath": [
                        "TEXTURE_EXPORT_RECORD",
                        "PARENT_TEXTURE_DEFAULT",
                        "TEXTURE2D_CLASS_CDO",
                        "SOURCE_REVISION_TEXTURE_GROUP_FILTER_CONFIGURATION",
                        "SOURCE_REVISION_SHADER_CACHE",
                        "CONTROLLED_RUNTIME_CAPTURE",
                    ],
                    "oracleProvider": {
                        "providerId": "D3D11_WARP_SAMPLER_DESCRIPTOR_PILOT",
                        "providerAvailable": state_provider_verified,
                        "sourceValueProviderAvailable": False,
                    },
                    "pilotFixtureIds": ["warp-sampler-address-toggle"],
                    "numericOracleInputDomain": ["WRAP", "CLAMP"],
                    "numericOracleExpectedOutput": "EXACT_D3D11_SAMPLER_DESCRIPTOR_MUTATION",
                    "numericTolerance": 0.0,
                    "pilotDecision": (
                        "PROVIDER_PILOT_PASS_SOURCE_VALUE_UNAVAILABLE"
                        if state_provider_verified
                        else "BLOCKED_PROVIDER_PILOT_NOT_EXECUTED"
                    ),
                    "fidelityDecision": "UNRESOLVED_SAMPLER_PROVENANCE",
                    "executionDecision": "BLOCKED",
                    "owner": "MATERIAL_CORRECTIVE_EVIDENCE_OWNER",
                    "finalRuntimeOwner": "G09_SAMPLER_BINDING_COMPILER",
                    "remainingBlockers": [
                        sampler_blocker,
                        "SOURCE_VALUE_PROVIDER_UNAVAILABLE",
                        "FINAL_RUNTIME_CONSUMER_NOT_IMPLEMENTED",
                    ],
                }
            )

    if source_value_acquisition is not None:
        validate_source_value_acquisition_semantics(
            source_value_acquisition,
            material_contract,
        )
        require(
            source_value_acquisition.get("schema")
            == "lostark.artist-31470-material-source-value-acquisition-receipt"
            and source_value_acquisition.get("formatVersion") == 2,
            "unsupported Material source-value acquisition receipt",
        )
        acquisition_payload = copy.deepcopy(source_value_acquisition)
        acquisition_digest = acquisition_payload.pop("receiptSha256", None)
        require(
            acquisition_digest == canonical_sha256(acquisition_payload),
            "Material source-value acquisition receipt digest mismatch",
        )
        acquisition_sampler_rows = (
            source_value_acquisition.get("matrices", {}).get("strictSamplerRows")
            or []
        )
        require(
            len(acquisition_sampler_rows) == 72,
            "Material source-value sampler denominator changed",
        )
        acquisition_by_id = {
            row.get("matrixRowId"): row for row in acquisition_sampler_rows
        }
        require(
            len(acquisition_by_id) == 72 and None not in acquisition_by_id,
            "duplicate Material source-value sampler row identity",
        )
        for row in sampler_rows:
            acquisition_row = acquisition_by_id.get(row["matrixRowId"])
            require(
                isinstance(acquisition_row, dict)
                and acquisition_row.get("materialRecipeId")
                == row["materialRecipeId"]
                and acquisition_row.get("fieldId") == row["fieldId"]
                and acquisition_row.get("bindingOriginAndOwner")
                == row["bindingOriginAndOwner"],
                f"Material source-value sampler row join changed: {row['matrixRowId']}",
            )
            texture_evidence = acquisition_row.get("textureExportEvidence")
            raw_fields = (
                texture_evidence.get("fields")
                if isinstance(texture_evidence, dict)
                else None
            )
            require(
                isinstance(raw_fields, dict)
                and set(raw_fields)
                == {"addressx", "addressy", "srgb", "filter", "lodgroup"}
                and all(
                    raw_fields[name].get("status")
                    in {"SERIALIZED_EXPLICIT", "OMITTED_FROM_EXPORT"}
                    for name in raw_fields
                )
                and acquisition_row.get("fullDescriptorSourceExact") is False
                and acquisition_row.get("sourceValueAcquired") is False
                and acquisition_row.get("executionReady") is False
                and acquisition_row.get("strictReauditDecision") == "BLOCKED"
                and acquisition_row.get("sourceValueDecision")
                in {
                    "BLOCKED_FULL_SAMPLER_DESCRIPTOR_DEFAULT_PROVENANCE_UNRESOLVED",
                    "BLOCKED_SOURCE_TEXTURE_PACKAGE_NOT_IN_ARCHIVE",
                },
                f"Material source-value sampler provenance changed: {row['matrixRowId']}",
            )
            row["sourceTextureEvidence"] = copy.deepcopy(texture_evidence)
            row["sourceValueAcquisitionEvidence"] = {
                "receiptSha256": source_value_acquisition["receiptSha256"],
                "baselineKind": acquisition_row["baselineKind"],
                "partialSourceExactFields": copy.deepcopy(
                    acquisition_row["partialSourceExactFields"]
                ),
                "partialCurrentOnlyFields": copy.deepcopy(
                    acquisition_row["partialCurrentOnlyFields"]
                ),
                "fullDescriptorSourceExact": acquisition_row[
                    "fullDescriptorSourceExact"
                ],
                "sourceValueDecision": acquisition_row["sourceValueDecision"],
                "strictReauditDecision": acquisition_row[
                    "strictReauditDecision"
                ],
            }

    require(len(render_rows) == 89, "Material render-state feasibility denominator changed")
    require(len(static_rows) == 94, "Material static feasibility denominator changed")
    require(len(sampler_rows) == 72, "Material sampler feasibility denominator changed")
    static_outcomes = Counter(
        row["instanceRecordIdentity"]["selectionOutcome"] for row in static_rows
    )
    require(
        static_outcomes
        == Counter(
            {
                "EXACT_GUID_INSTANCE_OVERRIDE_ENTRY": 23,
                "EXACT_GUID_NONOVERRIDE_ENTRY": 43,
                "NO_EXACT_GUID_NATIVE_ENTRY": 28,
            }
        ),
        "Material static GUID/native outcome denominator changed",
    )
    sampler_origins = Counter(
        row["bindingOriginAndOwner"]["bindingOrigin"] for row in sampler_rows
    )
    require(
        sampler_origins
        == Counter({"INSTANCE_OVERRIDE": 71, "PARENT_DEFAULT": 1}),
        "Material strict sampler origin denominator changed",
    )
    all_rows = render_rows + static_rows + sampler_rows
    require(
        len({row["matrixRowId"] for row in all_rows}) == len(all_rows),
        "duplicate Material feasibility matrix row ID",
    )
    require(
        all(row["owner"] and row["finalRuntimeOwner"] for row in all_rows),
        "ownerless Material feasibility row",
    )
    readiness_count = sum(
        row["executionDecision"] in {"READY", "VERIFIED_IRRELEVANT"}
        for row in all_rows
    )
    return {
        "renderStateRows": render_rows,
        "staticPermutationRows": static_rows,
        "strictSamplerRows": sampler_rows,
        "summary": {
            "renderStateRowCount": len(render_rows),
            "renderStateReadinessCount": sum(
                row["executionDecision"] in {"READY", "VERIFIED_IRRELEVANT"}
                for row in render_rows
            ),
            "staticPermutationRowCount": len(static_rows),
            "staticPermutationReadinessCount": sum(
                row["executionDecision"] in {"READY", "VERIFIED_IRRELEVANT"}
                for row in static_rows
            ),
            "staticExactGuidJoinCount": sum(
                count
                for outcome, count in static_outcomes.items()
                if outcome != "NO_EXACT_GUID_NATIVE_ENTRY"
            ),
            "staticOverrideTrueSourceValueAcquiredCount": static_outcomes[
                "EXACT_GUID_INSTANCE_OVERRIDE_ENTRY"
            ],
            "staticNonoverrideSemanticsUnverifiedCount": static_outcomes[
                "EXACT_GUID_NONOVERRIDE_ENTRY"
            ],
            "staticNoExactGuidEntryCount": static_outcomes[
                "NO_EXACT_GUID_NATIVE_ENTRY"
            ],
            "staticPermutationRowSetSha256": canonical_sha256(static_rows),
            "strictSamplerRowCount": len(sampler_rows),
            "strictSamplerReadinessCount": sum(
                row["executionDecision"] in {"READY", "VERIFIED_IRRELEVANT"}
                for row in sampler_rows
            ),
            "strictSamplerRejectedLegacyExactRowCount": sum(
                row.get("previouslyAdmittedExactSamplerReaudit") is True
                for row in sampler_rows
            ),
            "strictSamplerSourceTextureEvidenceRowCount": sum(
                isinstance(row.get("sourceTextureEvidence"), dict)
                and set(row["sourceTextureEvidence"].get("fields", {}))
                == {"addressx", "addressy", "srgb", "filter", "lodgroup"}
                for row in sampler_rows
            ),
            "strictSamplerRowSetSha256": canonical_sha256(sampler_rows),
            "totalRowCount": len(all_rows),
            "executionReadinessCount": readiness_count,
            "blockedRowCount": len(all_rows) - readiness_count,
            "ownerlessRowCount": 0,
            "unknownDecisionRowCount": 0,
            "evidenceIntegrity": True,
            "executionReadiness": readiness_count == len(all_rows),
        },
    }


def validate_upstream_material_receipts(
    material_contract: dict[str, Any],
    render_receipt: dict[str, Any],
    shader_receipt: dict[str, Any],
    material_contract_path: Path,
    render_receipt_path: Path,
) -> None:
    """Authenticate every upstream receipt before trusting downstream joins."""

    validate_contract(material_contract)
    validate_shader_receipt(shader_receipt, material_contract_path)
    require(
        render_receipt.get("schema")
        == "lostark.artist-31470-material-render-state-evidence-receipt",
        "render receipt schema mismatch",
    )
    require(
        type(render_receipt.get("formatVersion")) is int
        and render_receipt["formatVersion"] == 3,
        "render receipt version mismatch",
    )
    require(
        render_receipt.get("characterClass") == "ARTIST"
        and type(render_receipt.get("skillId")) is int
        and render_receipt["skillId"] == 31470
        and render_receipt.get("inputSlot") == "F",
        "render receipt root identity mismatch",
    )
    sealed_render = dict(render_receipt)
    claimed_render = sealed_render.pop("receiptSha256", None)
    require(
        claimed_render == canonical_sha256(sealed_render),
        "render receipt digest mismatch",
    )
    pinned_render = (
        (material_contract.get("sourceEvidence") or {})
        .get("renderStateReceipt")
        or {}
    )
    require(
        pinned_render.get("receiptSha256") == claimed_render
        and pinned_render.get("canonicalTextSha256")
        == tracked_text_sha256(render_receipt_path),
        "Material contract does not pin the supplied render receipt",
    )


def build_receipt(
    material_contract: dict[str, Any],
    render_receipt: dict[str, Any],
    shader_receipt: dict[str, Any],
    source_value_acquisition: dict[str, Any],
    material_contract_path: Path,
    render_receipt_path: Path,
    shader_receipt_path: Path,
    source_value_acquisition_path: Path,
    hlsl_path: Path,
    source_archive: dict[str, Any],
    hlsl_verification: dict[str, Any] | None = None,
    warp_state_verification: dict[str, Any] | None = None,
    *,
    provisional_for_warp_replay: bool = False,
) -> dict[str, Any]:
    validate_upstream_material_receipts(
        material_contract,
        render_receipt,
        shader_receipt,
        material_contract_path,
        render_receipt_path,
    )
    validate_source_value_acquisition_semantics(
        source_value_acquisition,
        material_contract,
    )
    require(
        source_archive == PINNED_SOURCE_ARCHIVE_PROJECTION,
        "source archive deep projection changed",
    )

    expressions_by_base: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for expression in render_receipt.get("graphExpressions") or []:
        base = expression.get("baseMaterialEvidenceId")
        require(isinstance(base, str) and base, "graph expression base evidence is missing")
        expressions_by_base[base].append(expression)
    render_exports = {
        str(row.get("evidenceId")): row
        for row in render_receipt.get("exports") or []
    }

    family_to_recipes: dict[str, list[dict[str, Any]]] = defaultdict(list)
    recipes = material_contract.get("materialRecipes") or []
    for recipe in recipes:
        family_to_recipes[str(recipe.get("arithmeticFamilyId"))].append(recipe)

    evaluators = []
    for family in sorted(material_contract.get("graphFamilies") or [], key=lambda row: row["familyId"]):
        family_id = family["familyId"]
        base = family["rawEvidence"]["baseMaterialEvidenceId"]
        expressions = expressions_by_base.get(base) or []
        require(expressions, f"family has no raw expression evidence: {family_id}")
        base_export = render_exports.get(base)
        require(base_export is not None, f"family base Material export is missing: {family_id}")
        expressions_field = ((base_export.get("fields") or {}).get("expressions") or {})
        require(
            expressions_field.get("recordSha256")
            == family["rawEvidence"]["expressionsRecordSha256"],
            f"family expression-array record changed: {family_id}",
        )
        raw_expression_digest = canonical_sha256(
            [
                {
                    "evidenceId": row["evidenceId"],
                    "sourceOrder": row["sourceOrder"],
                    "rawReference": row["rawReferenceFromBaseExpressions"],
                    "inputPackageIndices": [
                        input_row.get("packageIndex")
                        for input_row in (row.get("projection") or {}).get("inputs") or []
                    ],
                }
                for row in sorted(expressions, key=lambda row: row["sourceOrder"])
            ]
        )
        require(
            raw_expression_digest
            == family["rawEvidence"]["expressionEvidenceSha256"],
            f"family expression topology changed: {family_id}",
        )
        unresolved_input_edges = sum(
            input_row.get("packageIndex") == 0
            for row in expressions
            for input_row in (row.get("projection") or {}).get("inputs") or []
        )
        require(
            len(expressions) == family["rawEvidence"]["nonNullExpressionCount"]
            and unresolved_input_edges
            == family["rawEvidence"]["unresolvedInputEdgeCount"],
            f"family expression denominator changed: {family_id}",
        )
        mask, evidence = classify_family_features(expressions)
        evaluator_id = family["evaluator"]["evaluatorId"]
        sample_rows = []
        for sample in ORACLE_INPUTS:
            sample_rows.append(
                {
                    "sampleId": sample["sampleId"],
                    "inputSha256": canonical_sha256(sample),
                    "expectedFloat4": evaluate_cpu(mask, sample),
                }
            )
        expression_projection = family_expression_projection(expressions)
        renderer_shapes = sorted(
            {
                shape
                for recipe in family_to_recipes[family_id]
                for shape in recipe.get("rendererShapes") or []
            },
            key=str.casefold,
        )
        evaluator = {
            "familyId": family_id,
            "familyIdentitySha256": family["identitySha256"],
            "evaluatorId": evaluator_id,
            "evaluatorVersion": EVALUATOR_VERSION,
            "rendererShapes": renderer_shapes,
            "featureMask": mask,
            "features": feature_names(mask),
            "featureEvidence": evidence,
            "rawExpressionProjectionSha256": canonical_sha256(expression_projection),
            "rawExpressionCount": len(expression_projection),
            "fidelity": "RECONSTRUCTED_NUMERICALLY_VERIFIED",
            "graphProvenance": "RECONSTRUCTED_GRAPH",
            "sourceExact": False,
            "implemented": True,
            "cpuNumericOracleVerified": True,
            "hlslNumericOracleVerified": bool(hlsl_verification and hlsl_verification.get("verified")),
            "sampleRows": sample_rows,
            "evidenceBlockers": [
                "COOKED_STRIPPED_ARITHMETIC_GRAPH",
                "SOURCE_REVISION_SHADER_CACHE_NOT_ACQUIRED",
            ],
            "runtimeBlockers": [
                "MATERIAL_RUNTIME_HANDLER_CONSUMPTION_PENDING",
                "RECONSTRUCTED_EVALUATOR_NOT_YET_BOUND_TO_COMMON_SHADER_PATH",
            ],
            "arithmeticEvaluationAdmission": bool(
                hlsl_verification and hlsl_verification.get("verified")
            ),
        }
        evaluator["evaluatorSha256"] = canonical_sha256(evaluator)
        evaluators.append(evaluator)

    evaluator_by_family = {row["familyId"]: row for row in evaluators}
    recipe_bindings = []
    all_field_ids: set[str] = set()
    role_counts: Counter[str] = Counter()
    kind_counts: Counter[str] = Counter()
    static_switch_count = 0
    render_state_count = 0
    resolved_render_state_count = 0
    for recipe in sorted(recipes, key=lambda row: row["recipeId"]):
        fields = []
        inputs = recipe.get("inputs") or {}
        for section in ("scalarOverrides", "vectorOverrides", "textureOverrides", "parentDefaults"):
            for source_index, field in enumerate(inputs.get(section) or []):
                field_id = field.get("fieldId")
                require(isinstance(field_id, str) and field_id not in all_field_ids, "duplicate Material input field ID")
                all_field_ids.add(field_id)
                binding = build_ordered_input_binding(
                    recipe["recipeId"], section, source_index, field
                )
                role = binding["bindingRole"]
                role_counts[role] += 1
                kind = binding["fieldKind"]
                kind_counts[kind] += 1
                fields.append(binding)
        require(fields, f"recipe has no Material inputs: {recipe['recipeId']}")

        static_switches = []
        static_permutation = recipe.get("staticPermutation") or {}
        for section in ("selectedParameters", "parentDefaults"):
            for source_index, field in enumerate(static_permutation.get(section) or []):
                field_id = field.get("fieldId")
                require(isinstance(field_id, str) and field_id not in all_field_ids, "duplicate Material static switch field ID")
                all_field_ids.add(field_id)
                static_switch_count += 1
                static_switches.append(
                    build_ordered_static_binding(
                        recipe["recipeId"], section, source_index, field
                    )
                )

        family_id = recipe["arithmeticFamilyId"]
        evaluator = evaluator_by_family.get(family_id)
        require(evaluator is not None, "recipe references unknown evaluator family")
        recipe_feature_mask, static_feature_decisions = feature_mask_for_static_switches(
            evaluator["featureMask"], static_switches
        )
        runtime_operands = build_recipe_operands(fields, recipe_feature_mask)
        numeric_samples = build_recipe_numeric_samples(
            recipe_feature_mask, runtime_operands
        )

        render_state_bindings = []
        render_fields = (recipe.get("renderState") or {}).get("fields") or {}
        require(set(render_fields) == set(RENDER_STATE_FIELDS), "Material render-state field set changed")
        for field_name in RENDER_STATE_FIELDS:
            source_field = render_fields[field_name]
            status = source_field.get("status")
            require(status in {"SERIALIZED_EXPLICIT", "OMITTED_FROM_EXPORT"}, "Material render-state status changed")
            render_state_count += 1
            if status == "SERIALIZED_EXPLICIT":
                resolved_render_state_count += 1
            render_state_bindings.append(
                {
                    "fieldName": field_name,
                    "status": status,
                    "bindingOrigin": source_field.get("bindingOrigin"),
                    "fidelity": source_field.get("fidelity"),
                    "typedValue": source_field.get("value") if status == "SERIALIZED_EXPLICIT" else None,
                    "sourceRecordSha256": source_field.get("recordSha256"),
                    "blocker": source_field.get("blocker"),
                }
            )
        row = {
            "recipeId": recipe["recipeId"],
            "sourceMaterialPath": recipe["sourceMaterialPath"],
            "sourceRecipeCompositionSha256": recipe["compositionSha256"],
            "familyId": family_id,
            "evaluatorId": evaluator["evaluatorId"],
            "evaluatorVersion": evaluator["evaluatorVersion"],
            "orderedInputBindings": fields,
            "inputBindingCount": len(fields),
            "inputRoleCounts": dict(sorted(Counter(field["bindingRole"] for field in fields).items())),
            "orderedStaticSwitchBindings": static_switches,
            "staticSwitchBindingCount": len(static_switches),
            "staticFeatureDecisions": static_feature_decisions,
            "recipeFeatureMask": recipe_feature_mask,
            "runtimeOperandBindings": runtime_operands,
            "numericBindingSamples": numeric_samples,
            "renderStateBindings": render_state_bindings,
            "renderStateBindingCount": len(render_state_bindings),
            "fullRenderStateAdmission": all(
                field["status"] == "SERIALIZED_EXPLICIT"
                for field in render_state_bindings
            ),
            "bindingCompileAdmission": True,
            "runtimeHandlerConsumptionAdmission": False,
            "productAdmission": False,
            "runtimeBlockers": sorted(
                set(recipe.get("blockers") or [])
                - {"RECONSTRUCTED_ARITHMETIC_EVALUATOR_UNIMPLEMENTED"}
                | {
                    "MATERIAL_RUNTIME_HANDLER_CONSUMPTION_PENDING",
                    "RECONSTRUCTED_EVALUATOR_NOT_YET_BOUND_TO_COMMON_SHADER_PATH",
                }
            ),
        }
        row["bindingSha256"] = canonical_sha256(row)
        recipe_bindings.append(row)

    binding_by_recipe = {row["recipeId"]: row for row in recipe_bindings}
    occurrence_bindings = []
    for occurrence in sorted(material_contract.get("occurrences") or [], key=lambda row: row["occurrenceId"]):
        binding = binding_by_recipe.get(occurrence["materialRecipeId"])
        require(binding is not None, "occurrence references unknown Material binding")
        row = {
            "occurrenceId": occurrence["occurrenceId"],
            "sourceOccurrenceIdentitySha256": occurrence["identitySha256"],
            "cueId": occurrence["cueId"],
            "rendererType": occurrence["rendererType"],
            "materialRecipeId": occurrence["materialRecipeId"],
            "materialBindingSha256": binding["bindingSha256"],
            "evaluatorId": binding["evaluatorId"],
            "evaluatorVersion": binding["evaluatorVersion"],
            "runtimeHandlerConsumptionAdmission": False,
            "productAdmission": False,
        }
        row["bindingSha256"] = canonical_sha256(row)
        occurrence_bindings.append(row)

    feasibility_matrices = build_material_feasibility_matrices(
        material_contract,
        shader_receipt,
        warp_state_verification,
        source_value_acquisition,
    )
    feasibility_summary = feasibility_matrices["summary"]
    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "sourceEvidence": {
            "materialContractSha256": material_contract["contractSha256"],
            "materialContractTrackedTextSha256": tracked_text_sha256(material_contract_path),
            "renderReceiptSha256": render_receipt["receiptSha256"],
            "renderReceiptTrackedTextSha256": tracked_text_sha256(render_receipt_path),
            "shaderCacheReceiptSha256": shader_receipt["receiptSha256"],
            "shaderCacheReceiptTrackedTextSha256": tracked_text_sha256(shader_receipt_path),
            "sourceValueAcquisitionReceiptSha256": source_value_acquisition[
                "receiptSha256"
            ],
            "sourceValueAcquisitionTrackedTextSha256": tracked_text_sha256(
                source_value_acquisition_path
            ),
            "hlslTrackedTextSha256": tracked_text_sha256(hlsl_path),
            "generatorTrackedTextSha256": tracked_text_sha256(GENERATOR_PATH),
            "materialContractBuilderTrackedTextSha256": tracked_text_sha256(
                MATERIAL_CONTRACT_BUILDER_PATH
            ),
            "shaderCacheOracleTrackedTextSha256": tracked_text_sha256(
                SHADER_CACHE_ORACLE_PATH
            ),
            "ue3PackageParserTrackedTextSha256": tracked_text_sha256(
                UE3_PACKAGE_PARSER_PATH
            ),
            "hlslVerifierTrackedTextSha256": tracked_text_sha256(
                HLSL_VERIFIER_PATH
            ),
            "sourceValueAcquisitionGeneratorTrackedTextSha256": tracked_text_sha256(
                SOURCE_VALUE_ACQUISITION_GENERATOR_PATH
            ),
            "materialEvidenceApprovalTrackedTextSha256": tracked_text_sha256(
                MATERIAL_EVIDENCE_APPROVAL_PATH
            ),
        },
        "sourceRevisionShaderCacheAcquisition": source_archive,
        "controlledCaptureAssessment": {
            "available": False,
            "reason": "NO_SOURCE_REVISION_UE3_RUNTIME_INSTRUMENTATION_OR_SHADERCACHE_PACKAGE",
            "uncontrolledInstalledGameProcessUsed": False,
            "decision": "USE_EXPLICIT_RECONSTRUCTED_NUMERIC_ORACLE_WITH_SOURCE_EXACT_FALSE",
        },
        "evaluatorContract": expected_evaluator_contract(),
        "familyEvaluators": evaluators,
        "materialRecipeBindings": recipe_bindings,
        "occurrenceBindings": occurrence_bindings,
        "materialFeasibilityMatrices": feasibility_matrices,
        "hlslVerification": hlsl_verification
        or {
            "verified": False,
            "blocker": "HLSL_WARP_NUMERIC_ORACLE_NOT_EXECUTED",
        },
        "warpStateProviderVerification": warp_state_verification
        or {
            "verified": False,
            "blocker": "D3D11_WARP_STATE_PROVIDER_PILOT_NOT_EXECUTED",
        },
        "admission": {
            "evidenceIntegrityAdmission": True,
            "executionReadinessAdmission": feasibility_summary[
                "executionReadiness"
            ],
            "arithmeticFamilyEvaluationAdmission": bool(
                hlsl_verification and hlsl_verification.get("verified")
            ),
            "materialRuntimeHandlerConsumptionAdmission": False,
            "rendererConsumptionAdmission": False,
            "productAdmission": False,
            "blockers": [
                "FULL_RENDER_AND_STATIC_STATE_CLOSURE_PENDING",
                "MATERIAL_RUNTIME_HANDLER_CONSUMPTION_PENDING",
                "RECONSTRUCTED_EVALUATOR_NOT_YET_BOUND_TO_COMMON_SHADER_PATH",
                "SAMPLER_BINDINGS_INCOMPLETE",
            ],
        },
        "summary": {
            "materialFamilyCount": len(evaluators),
            "implementedEvaluatorCount": sum(row["implemented"] for row in evaluators),
            "cpuVerifiedEvaluatorCount": sum(row["cpuNumericOracleVerified"] for row in evaluators),
            "hlslVerifiedEvaluatorCount": sum(row["hlslNumericOracleVerified"] for row in evaluators),
            "reconstructedNumericallyVerifiedEvaluatorCount": sum(
                row["fidelity"] == "RECONSTRUCTED_NUMERICALLY_VERIFIED" for row in evaluators
            ),
            "sourceExactEvaluatorCount": sum(row["sourceExact"] for row in evaluators),
            "materialRecipeBindingCount": len(recipe_bindings),
            "materialOccurrenceBindingCount": len(occurrence_bindings),
            "inputBindingCount": sum(kind_counts.values()),
            "inputKindCounts": dict(sorted(kind_counts.items())),
            "inputRoleCounts": dict(sorted(role_counts.items())),
            "unknownInputRoleCount": 0,
            "staticSwitchBindingCount": static_switch_count,
            "totalTypedFieldBindingCount": len(all_field_ids),
            "renderStateBindingCount": render_state_count,
            "resolvedRenderStateBindingCount": resolved_render_state_count,
            "unresolvedRenderStateBindingCount": render_state_count - resolved_render_state_count,
            "familyNumericSampleCount": len(evaluators) * len(ORACLE_INPUTS),
            "recipeNumericSampleCount": len(recipe_bindings) * len(ORACLE_INPUTS),
            "hlslSampleCount": (len(evaluators) + len(recipe_bindings)) * len(ORACLE_INPUTS),
            "runtimeHandlerConsumedRecipeCount": 0,
            "runtimeHandlerConsumedOccurrenceCount": 0,
            "productRecipeCount": 0,
            "productOccurrenceCount": 0,
            "materialFeasibilityRowCount": feasibility_summary["totalRowCount"],
            "materialFeasibilityReadyCount": feasibility_summary[
                "executionReadinessCount"
            ],
            "materialFeasibilityBlockedCount": feasibility_summary[
                "blockedRowCount"
            ],
        },
    }
    seal_receipt(receipt)
    if not provisional_for_warp_replay:
        validate_runtime_receipt(receipt)
        validate_runtime_receipt_source_bindings(
            receipt,
            material_contract,
            render_receipt,
            shader_receipt,
            source_value_acquisition,
            material_contract_path=material_contract_path,
            render_receipt_path=render_receipt_path,
        )
    return receipt


def require_sha256_identity(
    value: Any, label: str, expected: str | None = None
) -> str:
    require(
        isinstance(value, str)
        and len(value) == 64
        and value == value.lower()
        and value != "0" * 64
        and all(character in "0123456789abcdef" for character in value),
        f"{label} SHA-256 identity is invalid",
    )
    if expected is not None:
        require(value == expected, f"{label} SHA-256 identity changed")
    return value


def validate_evaluator_contract(receipt: dict[str, Any]) -> None:
    contract = receipt.get("evaluatorContract")
    require(isinstance(contract, dict), "Material evaluator contract is missing")
    require(
        canonical_sha256(contract) == APPROVED_EVALUATOR_CONTRACT_SHA256
        and canonical_sha256(contract)
        == canonical_sha256(expected_evaluator_contract()),
        "Material evaluator contract projection changed",
    )
    require(
        type(contract.get("version")) is int
        and contract["version"] == EVALUATOR_VERSION
        and contract.get("operationOrder") == list(EVALUATOR_OPERATION_ORDER)
        and contract.get("fidelity") == "RECONSTRUCTED_NUMERICALLY_VERIFIED"
        and contract.get("sourceExact") is False,
        "Material evaluator reconstruction classification changed",
    )
    tolerance = require_finite_number(
        contract.get("numericTolerance"), "Material evaluator numeric tolerance"
    )
    f32(tolerance)
    require(tolerance == NUMERIC_TOLERANCE, "Material evaluator numeric tolerance changed")
    samples = contract.get("inputSamples")
    require(
        type(contract.get("inputSampleCountPerFamily")) is int
        and contract["inputSampleCountPerFamily"] == len(ORACLE_INPUTS)
        and isinstance(samples, list)
        and len(samples) == len(ORACLE_INPUTS),
        "Material evaluator input denominator changed",
    )
    for sample, expected in zip(samples, ORACLE_INPUTS, strict=True):
        validate_cpu_input_sample(sample)
        require(
            canonical_sha256(sample) == canonical_sha256(expected),
            "Material evaluator input sample changed",
        )


def validate_hlsl_verification(receipt: dict[str, Any]) -> None:
    verification = receipt.get("hlslVerification")
    require(isinstance(verification, dict), "Material HLSL verification is missing")
    expected_keys = {
        "verified",
        "backend",
        "entryPoint",
        "targetProfile",
        "compiler",
        "hlslTrackedTextSha256",
        "compiledDxbcSha256",
        "sampleCount",
        "inputBytesSha256",
        "outputFloat32BytesSha256",
        "numericTolerance",
        "maxAbsoluteError",
        "replayBindingSha256",
    }
    require(set(verification) == expected_keys, "Material HLSL verification schema changed")
    require(
        verification.get("verified") is True
        and verification.get("backend") == "D3D11_WARP_COMPUTE"
        and verification.get("entryPoint") == "main"
        and verification.get("targetProfile") == "cs_5_0",
        "Material HLSL execution identity changed",
    )
    compiler = verification.get("compiler")
    require(
        isinstance(compiler, dict)
        and set(compiler) == {"fileName", "byteSize", "rawSha256", "hashRole"}
        and compiler.get("fileName") == "d3dcompiler_47.dll"
        and type(compiler.get("byteSize")) is int
        and compiler["byteSize"] == PINNED_D3DCOMPILER_BYTE_SIZE
        and compiler.get("hashRole") == "EXTERNAL_RAW_BYTES",
        "Material HLSL compiler identity changed",
    )
    require_sha256_identity(
        compiler.get("rawSha256"),
        "Material HLSL compiler",
        PINNED_D3DCOMPILER_SHA256,
    )
    source = receipt.get("sourceEvidence") or {}
    require(
        source.get("hlslTrackedTextSha256")
        == PINNED_HLSL_TRACKED_TEXT_SHA256,
        "Material HLSL approved source identity changed",
    )
    require_sha256_identity(
        verification.get("hlslTrackedTextSha256"),
        "Material HLSL source",
        PINNED_HLSL_TRACKED_TEXT_SHA256,
    )
    require_sha256_identity(
        verification.get("compiledDxbcSha256"),
        "Material HLSL DXBC",
        PINNED_COMPILED_DXBC_SHA256,
    )
    require_sha256_identity(
        verification.get("inputBytesSha256"),
        "Material HLSL input bytes",
        PINNED_HLSL_INPUT_BYTES_SHA256,
    )
    require_sha256_identity(
        verification.get("outputFloat32BytesSha256"),
        "Material HLSL output bytes",
        PINNED_HLSL_OUTPUT_BYTES_SHA256,
    )
    require(
        type(verification.get("sampleCount")) is int
        and verification["sampleCount"] == 200,
        "Material HLSL sample denominator changed",
    )
    tolerance = require_finite_number(
        verification.get("numericTolerance"), "Material HLSL numeric tolerance"
    )
    f32(tolerance)
    require(
        tolerance == NUMERIC_TOLERANCE
        and tolerance
        == require_finite_number(
            (receipt.get("evaluatorContract") or {}).get("numericTolerance"),
            "Material evaluator numeric tolerance",
        ),
        "Material HLSL tolerance binding changed",
    )
    max_error = require_finite_number(
        verification.get("maxAbsoluteError"), "Material HLSL maximum error"
    )
    f32(max_error)
    require(
        0.0 <= max_error <= tolerance
        and max_error == PINNED_HLSL_MAX_ABSOLUTE_ERROR,
        "Material HLSL maximum error changed",
    )
    replay = copy.deepcopy(verification)
    claimed_replay = replay.pop("replayBindingSha256", None)
    require_sha256_identity(
        claimed_replay,
        "Material HLSL replay binding",
        APPROVED_HLSL_REPLAY_BINDING_SHA256,
    )
    require(
        claimed_replay == hlsl_replay_binding_sha256(receipt, replay),
        "Material HLSL replay binding projection changed",
    )


def validate_controlled_capture_assessment(receipt: dict[str, Any]) -> None:
    expected = {
        "available": False,
        "reason": (
            "NO_SOURCE_REVISION_UE3_RUNTIME_INSTRUMENTATION_OR_"
            "SHADERCACHE_PACKAGE"
        ),
        "uncontrolledInstalledGameProcessUsed": False,
        "decision": (
            "USE_EXPLICIT_RECONSTRUCTED_NUMERIC_ORACLE_WITH_SOURCE_EXACT_FALSE"
        ),
    }
    assessment = receipt.get("controlledCaptureAssessment")
    require(
        isinstance(assessment, dict)
        and set(assessment) == set(expected)
        and canonical_sha256(assessment)
        == APPROVED_CONTROLLED_CAPTURE_ASSESSMENT_SHA256
        and canonical_sha256(assessment) == canonical_sha256(expected),
        "Material controlled-capture assessment changed",
    )
    require(
        assessment.get("available") is False
        and assessment.get("uncontrolledInstalledGameProcessUsed") is False,
        "Material uncontrolled or source-exact capture was laundered",
    )


def validate_warp_state_provider_verification(receipt: dict[str, Any]) -> None:
    verification = receipt.get("warpStateProviderVerification")
    require(
        isinstance(verification, dict)
        and set(verification)
        == {
            "verified",
            "backend",
            "featureLevel",
            "pilotCount",
            "pilots",
            "pilotProjectionSha256",
        }
        and verification.get("verified") is True
        and verification.get("backend") == "D3D11_WARP_STATE_OBJECTS"
        and type(verification.get("featureLevel")) is int
        and verification["featureLevel"] == 0xB000
        and type(verification.get("pilotCount")) is int
        and verification["pilotCount"] == 4,
        "Material WARP state provider identity changed",
    )
    pilot_layouts = {
        "warp-blend-mode-toggle": {
            "inputDomain": ["OPAQUE", "TRANSLUCENT"],
            "mutatedOutputFields": ["BlendEnable", "SrcBlend", "DestBlend"],
            "boolFields": {"BlendEnable"},
            "intFields": {
                "SrcBlend",
                "DestBlend",
                "BlendOp",
                "SrcBlendAlpha",
                "DestBlendAlpha",
                "BlendOpAlpha",
                "RenderTargetWriteMask",
            },
            "floatFields": set(),
        },
        "warp-depth-disable-toggle": {
            "inputDomain": [False, True],
            "mutatedOutputFields": ["DepthEnable"],
            "boolFields": {"DepthEnable", "StencilEnable"},
            "intFields": {"DepthWriteMask", "DepthFunc"},
            "floatFields": set(),
        },
        "warp-rasterizer-two-sided-toggle": {
            "inputDomain": [False, True],
            "mutatedOutputFields": ["CullMode"],
            "boolFields": {"FrontCounterClockwise", "DepthClipEnable"},
            "intFields": {"FillMode", "CullMode"},
            "floatFields": set(),
        },
        "warp-sampler-address-toggle": {
            "inputDomain": ["WRAP", "CLAMP"],
            "mutatedOutputFields": ["AddressU", "AddressV", "AddressW"],
            "boolFields": set(),
            "intFields": {
                "Filter",
                "AddressU",
                "AddressV",
                "AddressW",
                "MaxAnisotropy",
                "ComparisonFunc",
            },
            "floatFields": {"MipLODBias", "MinLOD", "MaxLOD"},
        },
    }
    pilots = verification.get("pilots")
    require(
        isinstance(pilots, list)
        and [pilot.get("pilotId") for pilot in pilots]
        == list(pilot_layouts),
        "Material WARP state pilot order changed",
    )
    for pilot in pilots:
        require(
            isinstance(pilot, dict)
            and set(pilot)
            == {
                "pilotId",
                "inputDomain",
                "expectedStateOutputs",
                "actualStateOutputs",
                "mutatedOutputFields",
                "numericTolerance",
                "decision",
            },
            "Material WARP state pilot schema changed",
        )
        layout = pilot_layouts[pilot["pilotId"]]
        require(
            canonical_sha256(pilot.get("inputDomain"))
            == canonical_sha256(layout["inputDomain"])
            and pilot.get("mutatedOutputFields")
            == layout["mutatedOutputFields"]
            and type(pilot.get("numericTolerance")) is float
            and pilot["numericTolerance"] == 0.0
            and pilot.get("decision") == "PASS",
            f"Material WARP state pilot metadata changed: {pilot['pilotId']}",
        )
        expected_outputs = pilot.get("expectedStateOutputs")
        actual_outputs = pilot.get("actualStateOutputs")
        require(
            isinstance(expected_outputs, list)
            and isinstance(actual_outputs, list)
            and len(expected_outputs) == len(layout["inputDomain"])
            and len(actual_outputs) == len(layout["inputDomain"]),
            f"Material WARP state output denominator changed: {pilot['pilotId']}",
        )
        output_keys = (
            layout["boolFields"] | layout["intFields"] | layout["floatFields"]
        )
        for lane_name, outputs in (
            ("expected", expected_outputs),
            ("actual", actual_outputs),
        ):
            for output in outputs:
                require(
                    isinstance(output, dict) and set(output) == output_keys,
                    f"Material WARP {lane_name} state schema changed: "
                    f"{pilot['pilotId']}",
                )
                require(
                    all(type(output[name]) is bool for name in layout["boolFields"])
                    and all(
                        type(output[name]) is int for name in layout["intFields"]
                    ),
                    f"Material WARP {lane_name} state scalar type changed: "
                    f"{pilot['pilotId']}",
                )
                for name in layout["floatFields"]:
                    require(
                        type(output[name]) is float,
                        f"Material WARP {lane_name} state float type changed: "
                        f"{pilot['pilotId']}:{name}",
                    )
                    require_exact_f32_number(
                        output[name],
                        f"Material WARP {lane_name} state float",
                    )
        require(
            canonical_sha256(actual_outputs) == canonical_sha256(expected_outputs),
            f"Material WARP state output replay changed: {pilot['pilotId']}",
        )
    require(
        verification.get("pilotProjectionSha256")
        == canonical_sha256(pilots)
        == APPROVED_WARP_STATE_PILOT_PROJECTION_SHA256,
        "Material WARP state pilot projection changed",
    )


def validate_runtime_receipt(receipt: dict[str, Any]) -> None:
    require(receipt.get("schema") == SCHEMA, "Material runtime oracle schema mismatch")
    require(type(receipt.get("formatVersion")) is int and receipt["formatVersion"] == FORMAT_VERSION, "Material runtime oracle version mismatch")
    require(receipt.get("characterClass") == "ARTIST" and type(receipt.get("skillId")) is int and receipt["skillId"] == 31470 and receipt.get("inputSlot") == "F", "Material runtime oracle root identity mismatch")
    sealed = dict(receipt)
    claimed = sealed.pop("receiptSha256", None)
    require(claimed == canonical_sha256(sealed), "Material runtime oracle self digest mismatch")
    source = receipt.get("sourceEvidence") or {}
    for key in (
        "materialContractSha256",
        "materialContractTrackedTextSha256",
        "renderReceiptSha256",
        "renderReceiptTrackedTextSha256",
        "shaderCacheReceiptSha256",
        "shaderCacheReceiptTrackedTextSha256",
        "sourceValueAcquisitionReceiptSha256",
        "sourceValueAcquisitionTrackedTextSha256",
        "hlslTrackedTextSha256",
        "generatorTrackedTextSha256",
        "materialContractBuilderTrackedTextSha256",
        "shaderCacheOracleTrackedTextSha256",
        "ue3PackageParserTrackedTextSha256",
        "hlslVerifierTrackedTextSha256",
        "sourceValueAcquisitionGeneratorTrackedTextSha256",
        "materialEvidenceApprovalTrackedTextSha256",
    ):
        require(isinstance(source.get(key), str) and len(source[key]) == 64, f"source evidence SHA is invalid: {key}")
    acquisition = receipt.get("sourceRevisionShaderCacheAcquisition") or {}
    require(
        acquisition == PINNED_SOURCE_ARCHIVE_PROJECTION,
        "source archive deep projection changed",
    )
    validate_evaluator_contract(receipt)
    validate_controlled_capture_assessment(receipt)
    validate_warp_state_provider_verification(receipt)
    families = receipt.get("familyEvaluators") or []
    recipes = receipt.get("materialRecipeBindings") or []
    occurrences = receipt.get("occurrenceBindings") or []
    require(len(families) == 23 and len(recipes) == 27 and len(occurrences) == 34, "Material runtime denominator changed")
    family_ids: set[str] = set()
    evaluator_ids: set[str] = set()
    evaluator_by_family: dict[str, dict[str, Any]] = {}
    for family in families:
        require(family["familyId"] not in family_ids and family["evaluatorId"] not in evaluator_ids, "duplicate Material evaluator identity")
        family_ids.add(family["familyId"])
        evaluator_ids.add(family["evaluatorId"])
        evaluator_by_family[family["familyId"]] = family
        sealed_family = dict(family)
        claimed_family = sealed_family.pop("evaluatorSha256", None)
        require(claimed_family == canonical_sha256(sealed_family), "family evaluator digest mismatch")
        require(family.get("evaluatorVersion") == EVALUATOR_VERSION and family.get("fidelity") == "RECONSTRUCTED_NUMERICALLY_VERIFIED" and family.get("graphProvenance") == "RECONSTRUCTED_GRAPH", "family evaluator fidelity changed")
        require(family.get("sourceExact") is False and family.get("implemented") is True and family.get("cpuNumericOracleVerified") is True, "family evaluator admission changed")
        require(len(family.get("sampleRows") or []) == len(ORACLE_INPUTS), "family sample denominator changed")
        require(family.get("features") == feature_names(family["featureMask"]), "family feature mask changed")
        for sample, source_sample in zip(family["sampleRows"], ORACLE_INPUTS, strict=True):
            require(sample["sampleId"] == source_sample["sampleId"] and sample["inputSha256"] == canonical_sha256(source_sample), "family input sample identity changed")
            expected = evaluate_cpu(family["featureMask"], source_sample)
            actual = validate_cpu_float4(
                sample.get("expectedFloat4"), "family CPU numeric output"
            )
            require(
                canonical_sha256(actual) == canonical_sha256(expected),
                "family CPU numeric output changed",
            )
    recipe_ids: set[str] = set()
    recipe_binding_by_id: dict[str, dict[str, Any]] = {}
    field_ids: set[str] = set()
    static_field_ids: set[str] = set()
    resolved_render_states = 0
    render_state_count = 0
    for recipe in recipes:
        require(recipe["recipeId"] not in recipe_ids, "duplicate Material recipe binding")
        recipe_ids.add(recipe["recipeId"])
        sealed_recipe = dict(recipe)
        claimed_recipe = sealed_recipe.pop("bindingSha256", None)
        require(claimed_recipe == canonical_sha256(sealed_recipe), "Material recipe binding digest mismatch")
        recipe_binding_by_id[recipe["recipeId"]] = recipe
        family = evaluator_by_family.get(recipe["familyId"])
        require(
            family is not None
            and recipe["evaluatorId"] == family["evaluatorId"]
            and recipe["evaluatorVersion"] == family["evaluatorVersion"],
            "Material recipe evaluator reference changed",
        )
        require(recipe.get("bindingCompileAdmission") is True and recipe.get("runtimeHandlerConsumptionAdmission") is False and recipe.get("productAdmission") is False, "Material recipe admission changed")
        fields = recipe.get("orderedInputBindings") or []
        require(len(fields) == recipe.get("inputBindingCount") and fields, "Material recipe input denominator changed")
        for field in fields:
            require(field["fieldId"] not in field_ids, "Material input field ownership changed")
            field_ids.add(field["fieldId"])
            require(not field["bindingRole"].startswith("UNKNOWN"), "unknown Material input role")
            require(field.get("fieldKind") in {"scalar", "vector", "texture"}, "Material typed input kind changed")
            require(field.get("typedValue") is not None, "Material typed input value is missing")
            require(field.get("typedValueSha256") == canonical_sha256(field["typedValue"]), "Material typed input value digest changed")
            require(
                field.get("sourceSection")
                in {"scalarOverrides", "vectorOverrides", "textureOverrides", "parentDefaults"}
                and type(field.get("sourceSectionIndex")) is int
                and field["sourceSectionIndex"] >= 0
                and field.get("sourceOwnerRecipeId") == recipe["recipeId"],
                "Material input order owner changed",
            )
        static_switches = recipe.get("orderedStaticSwitchBindings") or []
        require(len(static_switches) == recipe.get("staticSwitchBindingCount"), "Material static switch denominator changed")
        for field in static_switches:
            require(field["fieldId"] not in field_ids and field["fieldId"] not in static_field_ids, "Material static switch ownership changed")
            static_field_ids.add(field["fieldId"])
            require(type(field.get("typedValue")) is bool, "Material static switch value changed")
            require(field.get("typedValueSha256") == canonical_sha256(field["typedValue"]), "Material static switch value digest changed")
            require(
                field.get("sourceSection") in {"selectedParameters", "parentDefaults"}
                and type(field.get("sourceSectionIndex")) is int
                and field["sourceSectionIndex"] >= 0
                and field.get("sourceOwnerRecipeId") == recipe["recipeId"],
                "Material static switch order owner changed",
            )
        recipe_feature_mask, decisions = feature_mask_for_static_switches(
            family["featureMask"], static_switches
        )
        require(recipe.get("recipeFeatureMask") == recipe_feature_mask and recipe.get("staticFeatureDecisions") == decisions, "Material recipe feature mask changed")
        expected_operands = build_recipe_operands(fields, recipe_feature_mask)
        require(recipe.get("runtimeOperandBindings") == expected_operands, "Material recipe runtime operands changed")
        expected_samples = build_recipe_numeric_samples(recipe_feature_mask, expected_operands)
        actual_samples = recipe.get("numericBindingSamples") or []
        require(
            len(actual_samples) == len(expected_samples),
            "Material recipe numeric binding denominator changed",
        )
        for sample, expected_sample in zip(
            actual_samples, expected_samples, strict=True
        ):
            validate_cpu_input_sample(sample.get("input"))
            validate_cpu_float4(
                sample.get("expectedFloat4"),
                "Material recipe CPU numeric output",
            )
            require(
                canonical_sha256(sample) == canonical_sha256(expected_sample),
                "Material recipe numeric binding oracle changed",
            )
        render_state = recipe.get("renderStateBindings") or []
        require(len(render_state) == len(RENDER_STATE_FIELDS) and len(render_state) == recipe.get("renderStateBindingCount"), "Material render-state denominator changed")
        require([field.get("fieldName") for field in render_state] == list(RENDER_STATE_FIELDS), "Material render-state order changed")
        for field in render_state:
            render_state_count += 1
            status = field.get("status")
            require(status in {"SERIALIZED_EXPLICIT", "OMITTED_FROM_EXPORT"}, "Material render-state status changed")
            if status == "SERIALIZED_EXPLICIT":
                resolved_render_states += 1
                require(field.get("typedValue") is not None and isinstance(field.get("sourceRecordSha256"), str), "Material explicit render state evidence changed")
            else:
                require(field.get("typedValue") is None and isinstance(field.get("blocker"), str), "Material unresolved render state blocker changed")
        require(recipe.get("fullRenderStateAdmission") is all(field["status"] == "SERIALIZED_EXPLICIT" for field in render_state), "Material render-state admission changed")
    occurrence_ids: set[str] = set()
    for occurrence in occurrences:
        require(occurrence["occurrenceId"] not in occurrence_ids, "duplicate Material occurrence binding")
        occurrence_ids.add(occurrence["occurrenceId"])
        sealed_occurrence = dict(occurrence)
        claimed_occurrence = sealed_occurrence.pop("bindingSha256", None)
        require(claimed_occurrence == canonical_sha256(sealed_occurrence), "Material occurrence binding digest mismatch")
        recipe_binding = recipe_binding_by_id.get(occurrence["materialRecipeId"])
        require(
            recipe_binding is not None
            and occurrence["materialBindingSha256"]
            == recipe_binding["bindingSha256"]
            and occurrence["evaluatorId"] == recipe_binding["evaluatorId"]
            and occurrence["evaluatorVersion"] == recipe_binding["evaluatorVersion"],
            "Material occurrence exact recipe binding changed",
        )
        require(occurrence.get("runtimeHandlerConsumptionAdmission") is False and occurrence.get("productAdmission") is False, "Material occurrence admission changed")
    validate_hlsl_verification(receipt)
    summary = receipt.get("summary") or {}
    runtime_recipe_count = sum(
        row.get("runtimeHandlerConsumptionAdmission") is True for row in recipes
    )
    runtime_occurrence_count = sum(
        row.get("runtimeHandlerConsumptionAdmission") is True
        for row in occurrences
    )
    product_recipe_count = sum(row.get("productAdmission") is True for row in recipes)
    product_occurrence_count = sum(
        row.get("productAdmission") is True for row in occurrences
    )
    require(
        summary.get("runtimeHandlerConsumedRecipeCount")
        == runtime_recipe_count
        == 0
        and summary.get("runtimeHandlerConsumedOccurrenceCount")
        == runtime_occurrence_count
        == 0
        and summary.get("productRecipeCount") == product_recipe_count == 0
        and summary.get("productOccurrenceCount")
        == product_occurrence_count
        == 0,
        "Material runtime/Product consumption summary changed",
    )
    require(summary.get("materialFamilyCount") == 23 and summary.get("implementedEvaluatorCount") == 23 and summary.get("cpuVerifiedEvaluatorCount") == 23, "Material evaluator summary changed")
    hlsl = receipt.get("hlslVerification") or {}
    expected_hlsl_count = 23 if hlsl.get("verified") else 0
    require(summary.get("hlslVerifiedEvaluatorCount") == expected_hlsl_count, "HLSL evaluator summary changed")
    require(summary.get("sourceExactEvaluatorCount") == 0 and summary.get("materialRecipeBindingCount") == 27 and summary.get("materialOccurrenceBindingCount") == 34 and summary.get("unknownInputRoleCount") == 0, "Material binding summary changed")
    require(summary.get("inputBindingCount") == len(field_ids), "Material input summary changed")
    require(summary.get("staticSwitchBindingCount") == len(static_field_ids), "Material static switch summary changed")
    require(summary.get("totalTypedFieldBindingCount") == len(field_ids) + len(static_field_ids), "Material typed field summary changed")
    require(summary.get("renderStateBindingCount") == render_state_count and summary.get("resolvedRenderStateBindingCount") == resolved_render_states and summary.get("unresolvedRenderStateBindingCount") == render_state_count - resolved_render_states, "Material render-state summary changed")
    require(summary.get("familyNumericSampleCount") == 92 and summary.get("recipeNumericSampleCount") == 108 and summary.get("hlslSampleCount") == 200, "Material numeric sample summary changed")
    matrices = receipt.get("materialFeasibilityMatrices") or {}
    matrix_summary = matrices.get("summary") or {}
    render_matrix = matrices.get("renderStateRows") or []
    static_matrix = matrices.get("staticPermutationRows") or []
    sampler_matrix = matrices.get("strictSamplerRows") or []
    require(
        len(render_matrix) == 89
        and len(static_matrix) == 94
        and len(sampler_matrix) == 72,
        "Material feasibility denominator changed",
    )
    all_matrix_rows = render_matrix + static_matrix + sampler_matrix
    require(
        len({row.get("matrixRowId") for row in all_matrix_rows}) == 255,
        "Material feasibility row identity changed",
    )
    require(
        all(
            row.get("owner")
            and row.get("finalRuntimeOwner")
            and row.get("executionDecision") == "BLOCKED"
            and row.get("remainingBlockers")
            for row in all_matrix_rows
        ),
        "Material feasibility owner or decision changed",
    )
    static_outcomes = Counter(
        row.get("instanceRecordIdentity", {}).get("selectionOutcome")
        for row in static_matrix
    )
    require(
        static_outcomes
        == Counter(
            {
                "EXACT_GUID_INSTANCE_OVERRIDE_ENTRY": 23,
                "EXACT_GUID_NONOVERRIDE_ENTRY": 43,
                "NO_EXACT_GUID_NATIVE_ENTRY": 28,
            }
        ),
        "Material static GUID/native outcome changed",
    )
    for row in static_matrix:
        parent = row.get("parentIdentity") or {}
        native = row.get("instanceRecordIdentity", {}).get("micNativeSelection") or {}
        outcome = native.get("selectionOutcome")
        require(
            len(str(parent.get("expressionGuidHex") or "")) == 32
            and isinstance(parent.get("parentDefaultValue"), bool)
            and len(str(parent.get("defaultValuePropertyRecordSha256") or "")) == 64
            and len(str(parent.get("expressionGuidPropertyRecordSha256") or "")) == 64
            and outcome
            == row.get("instanceRecordIdentity", {}).get("selectionOutcome"),
            "Material static parent/native identity changed",
        )
        if outcome == "NO_EXACT_GUID_NATIVE_ENTRY":
            require(
                native.get("exactNameAndGuidMatchCount") == 0
                and row.get("sourceValueAcquired") is False,
                "Material unmatched static row gained a source value",
            )
            continue
        entry = native.get("entry") or {}
        native_tail = native.get("nativeTail") or {}
        require(
            native.get("exactNameAndGuidMatchCount") == 1
            and entry.get("expressionGuidHex") == parent.get("expressionGuidHex")
            and str(entry.get("parameterName") or "").casefold()
            == str(parent.get("parameterName") or "").casefold()
            and isinstance(entry.get("value"), bool)
            and isinstance(entry.get("bOverride"), bool)
            and type(entry.get("entryOffset")) is int
            and len(str(native_tail.get("nativeTailSha256") or "")) == 64
            and len(str(native_tail.get("staticParameterSetRawSha256") or "")) == 64
            and len(str(native_tail.get("staticParameterSetSemanticSha256") or ""))
            == 64
            and row.get("sourceValueAcquired") is entry.get("bOverride"),
            "Material static GUID/value/bOverride provenance changed",
        )
        if entry.get("bOverride") is False:
            require(
                entry.get("value") is parent.get("parentDefaultValue")
                and "NONOVERRIDE_STATIC_INHERITANCE_SEMANTICS_UNVERIFIED"
                in row.get("remainingBlockers", []),
                "Material nonoverride static semantics were laundered",
            )
    sampler_origins = Counter(
        row.get("bindingOriginAndOwner", {}).get("bindingOrigin")
        for row in sampler_matrix
    )
    require(
        sampler_origins
        == Counter({"INSTANCE_OVERRIDE": 71, "PARENT_DEFAULT": 1})
        and sum(
            row.get("previouslyAdmittedExactSamplerReaudit") is True
            for row in sampler_matrix
        )
        == 4
        and all(
            set(row.get("sourceTextureEvidence", {}).get("fields", {}))
            == {"addressx", "addressy", "srgb", "filter", "lodgroup"}
            and len(
                str(
                    row.get("sourceValueAcquisitionEvidence", {}).get(
                        "receiptSha256"
                    )
                    or ""
                )
            )
            == 64
            for row in sampler_matrix
        ),
        "Material strict sampler origin/evidence denominator changed",
    )
    require(
        matrix_summary.get("renderStateRowCount") == 89
        and matrix_summary.get("staticPermutationRowCount") == 94
        and matrix_summary.get("staticExactGuidJoinCount") == 66
        and matrix_summary.get("staticOverrideTrueSourceValueAcquiredCount") == 23
        and matrix_summary.get("staticNonoverrideSemanticsUnverifiedCount") == 43
        and matrix_summary.get("staticNoExactGuidEntryCount") == 28
        and matrix_summary.get("staticPermutationRowSetSha256")
        == canonical_sha256(static_matrix)
        and matrix_summary.get("strictSamplerRowCount") == 72
        and matrix_summary.get("strictSamplerReadinessCount") == 0
        and matrix_summary.get("strictSamplerRejectedLegacyExactRowCount") == 4
        and matrix_summary.get("strictSamplerSourceTextureEvidenceRowCount") == 72
        and matrix_summary.get("strictSamplerRowSetSha256")
        == canonical_sha256(sampler_matrix)
        and matrix_summary.get("totalRowCount") == 255
        and matrix_summary.get("executionReadinessCount") == 0
        and matrix_summary.get("blockedRowCount") == 255
        and matrix_summary.get("ownerlessRowCount") == 0
        and matrix_summary.get("unknownDecisionRowCount") == 0
        and matrix_summary.get("evidenceIntegrity") is True
        and matrix_summary.get("executionReadiness") is False,
        "Material feasibility summary changed",
    )
    validate_warp_state_provider_verification(receipt)
    require(
        summary.get("materialFeasibilityRowCount") == 255
        and summary.get("materialFeasibilityReadyCount") == 0
        and summary.get("materialFeasibilityBlockedCount") == 255,
        "Material feasibility top-level summary changed",
    )
    admission = receipt.get("admission") or {}
    require(admission.get("materialRuntimeHandlerConsumptionAdmission") is False and admission.get("rendererConsumptionAdmission") is False and admission.get("productAdmission") is False, "Material Product admission opened")
    require(admission.get("arithmeticFamilyEvaluationAdmission") is bool(hlsl.get("verified")), "Material arithmetic admission changed")
    require(
        admission.get("evidenceIntegrityAdmission") is True
        and admission.get("executionReadinessAdmission") is False,
        "Material evidence/readiness admission changed",
    )


def validate_runtime_receipt_source_bindings(
    receipt: dict[str, Any],
    material_contract: dict[str, Any],
    render_receipt: dict[str, Any],
    shader_receipt: dict[str, Any],
    source_value_acquisition: dict[str, Any],
    *,
    material_contract_path: Path = DEFAULT_MATERIAL_CONTRACT,
    render_receipt_path: Path = DEFAULT_RENDER_RECEIPT,
) -> None:
    """Join every executable-looking runtime binding back to the pinned contract.

    ``validate_runtime_receipt`` proves only internal consistency.  This join is
    intentionally separate so shallow integrity checks cannot be mistaken for
    source authentication.
    """

    validate_upstream_material_receipts(
        material_contract,
        render_receipt,
        shader_receipt,
        material_contract_path,
        render_receipt_path,
    )
    validate_source_value_acquisition_semantics(
        source_value_acquisition,
        material_contract,
    )
    expressions_by_base: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for expression in render_receipt.get("graphExpressions") or []:
        expressions_by_base[expression["baseMaterialEvidenceId"]].append(expression)
    source_families = {
        row["familyId"]: row
        for row in material_contract.get("graphFamilies") or []
    }
    for runtime in receipt.get("familyEvaluators") or []:
        source = source_families.get(runtime["familyId"])
        require(source is not None, "runtime family is absent from Material contract")
        require(
            runtime["familyIdentitySha256"] == source["identitySha256"]
            and runtime["evaluatorId"] == source["evaluator"]["evaluatorId"],
            "runtime family identity is not source-bound",
        )
        expressions = expressions_by_base.get(
            source["rawEvidence"]["baseMaterialEvidenceId"]
        ) or []
        require(expressions, "runtime family raw expression evidence is missing")
        expected_mask, expected_evidence = classify_family_features(expressions)
        expected_projection = family_expression_projection(expressions)
        require(
            runtime["featureMask"] == expected_mask
            and runtime["features"] == feature_names(expected_mask)
            and runtime["featureEvidence"] == expected_evidence
            and runtime["rawExpressionCount"] == len(expected_projection)
            and runtime["rawExpressionProjectionSha256"]
            == canonical_sha256(expected_projection),
            "runtime family feature mask is not raw-expression-bound",
        )

    source_recipes = {
        row["recipeId"]: row
        for row in material_contract.get("materialRecipes") or []
    }
    for runtime in receipt.get("materialRecipeBindings") or []:
        source = source_recipes.get(runtime["recipeId"])
        require(source is not None, "runtime recipe is absent from Material contract")
        require(
            runtime["sourceMaterialPath"] == source["sourceMaterialPath"]
            and runtime["sourceRecipeCompositionSha256"]
            == source["compositionSha256"]
            and runtime["familyId"] == source["arithmeticFamilyId"],
            "runtime recipe identity is not source-bound",
        )
        expected_inputs: list[dict[str, Any]] = []
        for section in (
            "scalarOverrides",
            "vectorOverrides",
            "textureOverrides",
            "parentDefaults",
        ):
            for source_index, field in enumerate(
                (source.get("inputs") or {}).get(section) or []
            ):
                expected_inputs.append(
                    build_ordered_input_binding(
                        source["recipeId"], section, source_index, field
                    )
                )
        runtime_inputs = runtime.get("orderedInputBindings") or []
        require(
            runtime_inputs == expected_inputs,
            "runtime typed input is not source-bound",
        )

        expected_switches: list[dict[str, Any]] = []
        for section in ("selectedParameters", "parentDefaults"):
            for source_index, field in enumerate(
                (source.get("staticPermutation") or {}).get(section) or []
            ):
                expected_switches.append(
                    build_ordered_static_binding(
                        source["recipeId"], section, source_index, field
                    )
                )
        runtime_switches = runtime.get("orderedStaticSwitchBindings") or []
        require(
            runtime_switches == expected_switches,
            "runtime static switch is not source-bound",
        )

        source_render = (source.get("renderState") or {}).get("fields") or {}
        expected_render = []
        for field_name in RENDER_STATE_FIELDS:
            source_field = source_render[field_name]
            status = source_field["status"]
            expected_render.append(
                {
                    "fieldName": field_name,
                    "status": status,
                    "bindingOrigin": source_field.get("bindingOrigin"),
                    "fidelity": source_field.get("fidelity"),
                    "typedValue": source_field.get("value")
                    if status == "SERIALIZED_EXPLICIT"
                    else None,
                    "sourceRecordSha256": source_field.get("recordSha256"),
                    "blocker": source_field.get("blocker"),
                }
            )
        require(
            runtime.get("renderStateBindings") == expected_render,
            "runtime render state is not source-bound",
        )

    source_occurrences = {
        row["occurrenceId"]: row
        for row in material_contract.get("occurrences") or []
    }
    runtime_recipe_bindings = {
        row["recipeId"]: row
        for row in receipt.get("materialRecipeBindings") or []
    }
    for runtime in receipt.get("occurrenceBindings") or []:
        source = source_occurrences.get(runtime["occurrenceId"])
        require(source is not None, "runtime occurrence is absent from Material contract")
        require(
            runtime["sourceOccurrenceIdentitySha256"] == source["identitySha256"]
            and runtime["cueId"] == source["cueId"]
            and runtime["rendererType"] == source["rendererType"]
            and runtime["materialRecipeId"] == source["materialRecipeId"],
            "runtime occurrence identity is not source-bound",
        )
        binding = runtime_recipe_bindings.get(runtime["materialRecipeId"])
        require(
            binding is not None
            and runtime["materialBindingSha256"] == binding["bindingSha256"]
            and runtime["evaluatorId"] == binding["evaluatorId"]
            and runtime["evaluatorVersion"] == binding["evaluatorVersion"],
            "runtime occurrence exact recipe binding is not source-bound",
        )

    expected_matrices = build_material_feasibility_matrices(
        material_contract,
        shader_receipt,
        receipt.get("warpStateProviderVerification"),
        source_value_acquisition,
    )
    require(
        receipt.get("materialFeasibilityMatrices") == expected_matrices,
        "Material feasibility matrices are not source-bound",
    )


def validate_runtime_receipt_tracked_sources(
    receipt: dict[str, Any],
    material_contract_path: Path,
    render_receipt_path: Path,
    shader_receipt_path: Path,
    source_value_acquisition_path: Path,
    hlsl_path: Path,
) -> None:
    source = receipt.get("sourceEvidence") or {}
    material_contract = read_json(material_contract_path)
    render_receipt = read_json(render_receipt_path)
    shader_receipt = read_json(shader_receipt_path)
    source_value_acquisition = read_json(source_value_acquisition_path)
    validate_source_value_acquisition_semantics(
        source_value_acquisition,
        material_contract,
    )
    require(
        source.get("materialContractSha256")
        == material_contract.get("contractSha256")
        and source.get("renderReceiptSha256")
        == render_receipt.get("receiptSha256")
        and source.get("shaderCacheReceiptSha256")
        == shader_receipt.get("receiptSha256")
        and source.get("sourceValueAcquisitionReceiptSha256")
        == source_value_acquisition.get("receiptSha256"),
        "Material oracle checked source identity changed",
    )
    tracked_sources = {
        "materialContractTrackedTextSha256": material_contract_path,
        "renderReceiptTrackedTextSha256": render_receipt_path,
        "shaderCacheReceiptTrackedTextSha256": shader_receipt_path,
        "sourceValueAcquisitionTrackedTextSha256": source_value_acquisition_path,
        "hlslTrackedTextSha256": hlsl_path,
        "generatorTrackedTextSha256": GENERATOR_PATH,
        "materialContractBuilderTrackedTextSha256": MATERIAL_CONTRACT_BUILDER_PATH,
        "shaderCacheOracleTrackedTextSha256": SHADER_CACHE_ORACLE_PATH,
        "ue3PackageParserTrackedTextSha256": UE3_PACKAGE_PARSER_PATH,
        "hlslVerifierTrackedTextSha256": HLSL_VERIFIER_PATH,
        "sourceValueAcquisitionGeneratorTrackedTextSha256": (
            SOURCE_VALUE_ACQUISITION_GENERATOR_PATH
        ),
        "materialEvidenceApprovalTrackedTextSha256": (
            MATERIAL_EVIDENCE_APPROVAL_PATH
        ),
    }
    for key, path in tracked_sources.items():
        require(
            source.get(key) == tracked_text_sha256(path),
            f"Material oracle tracked source changed: {key}",
        )


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
    material_contract = read_json(args.material_contract)
    render_receipt = read_json(args.render_receipt)
    shader_receipt = read_json(args.shader_receipt)
    source_value_acquisition = read_json(args.source_value_acquisition_receipt)
    validate_source_value_acquisition_semantics(
        source_value_acquisition,
        material_contract,
    )
    source_archive = scan_source_archive(args.source_archive_root)
    initial = build_receipt(
        material_contract,
        render_receipt,
        shader_receipt,
        source_value_acquisition,
        args.material_contract,
        args.render_receipt,
        args.shader_receipt,
        args.source_value_acquisition_receipt,
        args.hlsl,
        source_archive,
        provisional_for_warp_replay=True,
    )
    if args.run_hlsl:
        from verify_artist_31470_material_runtime_oracle_hlsl import (
            run_hlsl_oracle,
            run_warp_state_provider_oracle,
        )

        hlsl_verification = run_hlsl_oracle(initial, args.hlsl, args.d3dcompiler)
        warp_state_verification = run_warp_state_provider_oracle()
        return build_receipt(
            material_contract,
            render_receipt,
            shader_receipt,
            source_value_acquisition,
            args.material_contract,
            args.render_receipt,
            args.shader_receipt,
            args.source_value_acquisition_receipt,
            args.hlsl,
            source_archive,
            hlsl_verification,
            warp_state_verification,
        )
    return initial


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--material-contract", type=Path, default=DEFAULT_MATERIAL_CONTRACT)
    parser.add_argument("--render-receipt", type=Path, default=DEFAULT_RENDER_RECEIPT)
    parser.add_argument("--shader-receipt", type=Path, default=DEFAULT_SHADER_RECEIPT)
    parser.add_argument(
        "--source-value-acquisition-receipt",
        type=Path,
        default=DEFAULT_SOURCE_VALUE_ACQUISITION_RECEIPT,
    )
    parser.add_argument("--hlsl", type=Path, default=DEFAULT_HLSL)
    parser.add_argument("--source-archive-root", type=Path, default=DEFAULT_SOURCE_ARCHIVE_ROOT)
    parser.add_argument("--d3dcompiler", type=Path, default=Path(r"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\d3dcompiler_47.dll"))
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--run-hlsl", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--shallow-check", action="store_true")
    return parser.parse_args(argv)


def main(argv: Iterable[str] | None = None) -> int:
    args = parse_args(argv)
    if args.shallow_check:
        receipt = read_json(args.output)
        validate_runtime_receipt(receipt)
        material_contract = read_json(args.material_contract)
        render_receipt = read_json(args.render_receipt)
        shader_receipt = read_json(args.shader_receipt)
        source_value_acquisition = read_json(
            args.source_value_acquisition_receipt
        )
        validate_runtime_receipt_source_bindings(
            receipt,
            material_contract,
            render_receipt,
            shader_receipt,
            source_value_acquisition,
            material_contract_path=args.material_contract,
            render_receipt_path=args.render_receipt,
        )
        validate_runtime_receipt_tracked_sources(
            receipt,
            args.material_contract,
            args.render_receipt,
            args.shader_receipt,
            args.source_value_acquisition_receipt,
            args.hlsl,
        )
        print(
            "PASS: Artist F Material runtime oracle shallow "
            "family=23 recipe=27 occurrence=34 feasibility=0/255 product=false"
        )
        return 0
    candidate = build_from_paths(args)
    if args.check:
        require(args.output.is_file(), f"Material runtime oracle receipt is missing: {args.output}")
        require(read_json(args.output) == candidate, "Material runtime oracle receipt is stale")
        print(
            "PASS: Artist F Material runtime oracle deep "
            "family=23 recipe=27 occurrence=34 feasibility=0/255 product=false"
        )
        return 0
    write_json_atomic(args.output, candidate)
    print(f"wrote {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
