#!/usr/bin/env python3
"""Build the fail-closed Artist F reconstructed Material numeric oracle.

The evaluator in this file is an explicit reconstruction.  It never upgrades
the cooked Material graph to SOURCE_EXACT.  Its purpose is to give the later
runtime MaterialBinding compiler a versioned arithmetic contract whose CPU and
HLSL implementations can be compared without image-based validation.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any, Iterable

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
FORMAT_VERSION = 1
EVALUATOR_VERSION = 1
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


def read_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8-sig"), object_pairs_hook=strict_object)
    require(isinstance(value, dict), f"expected JSON object: {path}")
    return value


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
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


def f32(value: float) -> float:
    return struct.unpack("<f", struct.pack("<f", float(value)))[0]


def saturate(value: float) -> float:
    return min(1.0, max(0.0, value))


def evaluate_cpu(feature_mask: int, sample: dict[str, Any]) -> list[float]:
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


def build_receipt(
    material_contract: dict[str, Any],
    render_receipt: dict[str, Any],
    shader_receipt: dict[str, Any],
    material_contract_path: Path,
    render_receipt_path: Path,
    shader_receipt_path: Path,
    hlsl_path: Path,
    source_archive: dict[str, Any],
    hlsl_verification: dict[str, Any] | None = None,
) -> dict[str, Any]:
    validate_contract(material_contract)
    validate_shader_receipt(shader_receipt, material_contract_path)
    require(
        render_receipt.get("schema")
        == "lostark.artist-31470-material-render-state-evidence-receipt",
        "render receipt schema mismatch",
    )
    require(type(render_receipt.get("formatVersion")) is int and render_receipt["formatVersion"] == 3, "render receipt version mismatch")
    require(render_receipt.get("characterClass") == "ARTIST" and render_receipt.get("skillId") == 31470 and render_receipt.get("inputSlot") == "F", "render receipt root identity mismatch")
    sealed_render = dict(render_receipt)
    claimed_render = sealed_render.pop("receiptSha256", None)
    require(claimed_render == canonical_sha256(sealed_render), "render receipt digest mismatch")
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
    require(source_archive.get("shaderCacheNameCandidateCount") == 0, "source archive ShaderCache candidate requires review")

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
        expression_projection = [
            {
                "evidenceId": row["evidenceId"],
                "sourceOrder": row["sourceOrder"],
                "rawReferenceFromBaseExpressions": row["rawReferenceFromBaseExpressions"],
                "className": row["className"],
                "objectPath": row["objectPath"],
                "projection": row["projection"],
                "serialSha256": row["serialSha256"],
            }
            for row in sorted(expressions, key=lambda row: (row["sourceOrder"], row["exportIndex"]))
        ]
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
            for field in inputs.get(section) or []:
                field_id = field.get("fieldId")
                require(isinstance(field_id, str) and field_id not in all_field_ids, "duplicate Material input field ID")
                all_field_ids.add(field_id)
                role = classify_binding_role(field)
                role_counts[role] += 1
                kind = str(field["fieldKind"]).casefold()
                kind_counts[kind] += 1
                fields.append(
                    {
                        "fieldId": field_id,
                        "fieldKind": kind,
                        "bindingRole": role,
                        "bindingOrigin": field["bindingOrigin"],
                        "parameterName": field["parameterName"],
                        "normalizedParameterName": field["normalizedParameterName"],
                        "typedValue": normalize_typed_value(field),
                        "typedValueSha256": canonical_sha256(normalize_typed_value(field)),
                        "sourceFieldValueSha256": canonical_sha256(field.get("value")),
                        "sourceLineageSha256": field["provenance"]["lineageSha256"],
                    }
                )
        require(fields, f"recipe has no Material inputs: {recipe['recipeId']}")

        static_switches = []
        static_permutation = recipe.get("staticPermutation") or {}
        for section in ("selectedParameters", "parentDefaults"):
            for field in static_permutation.get(section) or []:
                field_id = field.get("fieldId")
                require(isinstance(field_id, str) and field_id not in all_field_ids, "duplicate Material static switch field ID")
                all_field_ids.add(field_id)
                static_switch_count += 1
                static_switches.append(
                    {
                        "fieldId": field_id,
                        "parameterName": field["parameterName"],
                        "normalizedParameterName": field["normalizedParameterName"],
                        "bindingOrigin": field["bindingOrigin"],
                        "typedValue": normalize_typed_value(field),
                        "typedValueSha256": canonical_sha256(normalize_typed_value(field)),
                        "sourceFieldValueSha256": canonical_sha256(field.get("value")),
                        "sourceLineageSha256": field["provenance"]["lineageSha256"],
                    }
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
        },
        "sourceRevisionShaderCacheAcquisition": source_archive,
        "controlledCaptureAssessment": {
            "available": False,
            "reason": "NO_SOURCE_REVISION_UE3_RUNTIME_INSTRUMENTATION_OR_SHADERCACHE_PACKAGE",
            "uncontrolledInstalledGameProcessUsed": False,
            "decision": "USE_EXPLICIT_RECONSTRUCTED_NUMERIC_ORACLE_WITH_SOURCE_EXACT_FALSE",
        },
        "evaluatorContract": {
            "version": EVALUATOR_VERSION,
            "operationOrder": [
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
            ],
            "inputSampleCountPerFamily": len(ORACLE_INPUTS),
            "inputSamples": list(ORACLE_INPUTS),
            "numericTolerance": 2.0e-5,
            "fidelity": "RECONSTRUCTED_NUMERICALLY_VERIFIED",
            "sourceExact": False,
        },
        "familyEvaluators": evaluators,
        "materialRecipeBindings": recipe_bindings,
        "occurrenceBindings": occurrence_bindings,
        "hlslVerification": hlsl_verification
        or {
            "verified": False,
            "blocker": "HLSL_WARP_NUMERIC_ORACLE_NOT_EXECUTED",
        },
        "admission": {
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
        },
    }
    seal_receipt(receipt)
    validate_runtime_receipt(receipt)
    validate_runtime_receipt_source_bindings(receipt, material_contract)
    return receipt


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
        "hlslTrackedTextSha256",
        "generatorTrackedTextSha256",
        "materialContractBuilderTrackedTextSha256",
        "shaderCacheOracleTrackedTextSha256",
        "ue3PackageParserTrackedTextSha256",
        "hlslVerifierTrackedTextSha256",
    ):
        require(isinstance(source.get(key), str) and len(source[key]) == 64, f"source evidence SHA is invalid: {key}")
    acquisition = receipt.get("sourceRevisionShaderCacheAcquisition") or {}
    require(type(acquisition.get("fileCount")) is int and acquisition["fileCount"] > 0, "source archive file denominator is invalid")
    require(type(acquisition.get("uniquePackageContentCount")) is int and acquisition["uniquePackageContentCount"] > 0, "source archive unique denominator is invalid")
    require(acquisition.get("shaderCacheNameCandidateCount") == 0 and acquisition.get("decision") == "SOURCE_REVISION_SHADER_CACHE_NOT_PRESENT_IN_SCANNED_ARCHIVE", "source archive ShaderCache decision changed")
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
            require(all(abs(float(a) - float(b)) <= 1.0e-7 for a, b in zip(sample["expectedFloat4"], expected, strict=True)), "family CPU numeric output changed")
    recipe_ids: set[str] = set()
    binding_shas: set[str] = set()
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
        binding_shas.add(claimed_recipe)
        family = evaluator_by_family.get(recipe["familyId"])
        require(family is not None and recipe["evaluatorId"] == family["evaluatorId"], "Material recipe evaluator reference changed")
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
        static_switches = recipe.get("orderedStaticSwitchBindings") or []
        require(len(static_switches) == recipe.get("staticSwitchBindingCount"), "Material static switch denominator changed")
        for field in static_switches:
            require(field["fieldId"] not in field_ids and field["fieldId"] not in static_field_ids, "Material static switch ownership changed")
            static_field_ids.add(field["fieldId"])
            require(type(field.get("typedValue")) is bool, "Material static switch value changed")
            require(field.get("typedValueSha256") == canonical_sha256(field["typedValue"]), "Material static switch value digest changed")
        recipe_feature_mask, decisions = feature_mask_for_static_switches(
            family["featureMask"], static_switches
        )
        require(recipe.get("recipeFeatureMask") == recipe_feature_mask and recipe.get("staticFeatureDecisions") == decisions, "Material recipe feature mask changed")
        expected_operands = build_recipe_operands(fields, recipe_feature_mask)
        require(recipe.get("runtimeOperandBindings") == expected_operands, "Material recipe runtime operands changed")
        expected_samples = build_recipe_numeric_samples(recipe_feature_mask, expected_operands)
        require(recipe.get("numericBindingSamples") == expected_samples, "Material recipe numeric binding oracle changed")
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
        require(occurrence["materialRecipeId"] in recipe_ids and occurrence["materialBindingSha256"] in binding_shas, "Material occurrence recipe binding changed")
        require(occurrence.get("runtimeHandlerConsumptionAdmission") is False and occurrence.get("productAdmission") is False, "Material occurrence admission changed")
    summary = receipt.get("summary") or {}
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
    admission = receipt.get("admission") or {}
    require(admission.get("materialRuntimeHandlerConsumptionAdmission") is False and admission.get("rendererConsumptionAdmission") is False and admission.get("productAdmission") is False, "Material Product admission opened")
    require(admission.get("arithmeticFamilyEvaluationAdmission") is bool(hlsl.get("verified")), "Material arithmetic admission changed")


def validate_runtime_receipt_source_bindings(
    receipt: dict[str, Any], material_contract: dict[str, Any]
) -> None:
    """Join every executable-looking runtime binding back to the pinned contract.

    ``validate_runtime_receipt`` proves only internal consistency.  This join is
    intentionally separate so shallow integrity checks cannot be mistaken for
    source authentication.
    """

    validate_contract(material_contract)
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
        expected_inputs: dict[str, dict[str, Any]] = {}
        for section in (
            "scalarOverrides",
            "vectorOverrides",
            "textureOverrides",
            "parentDefaults",
        ):
            for field in (source.get("inputs") or {}).get(section) or []:
                typed_value = normalize_typed_value(field)
                expected_inputs[field["fieldId"]] = {
                    "fieldId": field["fieldId"],
                    "fieldKind": str(field["fieldKind"]).casefold(),
                    "bindingRole": classify_binding_role(field),
                    "bindingOrigin": field["bindingOrigin"],
                    "parameterName": field["parameterName"],
                    "normalizedParameterName": field["normalizedParameterName"],
                    "typedValue": typed_value,
                    "typedValueSha256": canonical_sha256(typed_value),
                    "sourceFieldValueSha256": canonical_sha256(field.get("value")),
                    "sourceLineageSha256": field["provenance"]["lineageSha256"],
                }
        runtime_inputs = runtime.get("orderedInputBindings") or []
        require(
            len(runtime_inputs) == len(expected_inputs)
            and all(
                expected_inputs.get(field["fieldId"]) == field
                for field in runtime_inputs
            ),
            "runtime typed input is not source-bound",
        )

        expected_switches: dict[str, dict[str, Any]] = {}
        for section in ("selectedParameters", "parentDefaults"):
            for field in (source.get("staticPermutation") or {}).get(section) or []:
                typed_value = normalize_typed_value(field)
                expected_switches[field["fieldId"]] = {
                    "fieldId": field["fieldId"],
                    "parameterName": field["parameterName"],
                    "normalizedParameterName": field["normalizedParameterName"],
                    "bindingOrigin": field["bindingOrigin"],
                    "typedValue": typed_value,
                    "typedValueSha256": canonical_sha256(typed_value),
                    "sourceFieldValueSha256": canonical_sha256(field.get("value")),
                    "sourceLineageSha256": field["provenance"]["lineageSha256"],
                }
        runtime_switches = runtime.get("orderedStaticSwitchBindings") or []
        require(
            len(runtime_switches) == len(expected_switches)
            and all(
                expected_switches.get(field["fieldId"]) == field
                for field in runtime_switches
            ),
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


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8", newline="\n")
    temporary.replace(path)


def build_from_paths(args: argparse.Namespace) -> dict[str, Any]:
    material_contract = read_json(args.material_contract)
    render_receipt = read_json(args.render_receipt)
    shader_receipt = read_json(args.shader_receipt)
    source_archive = scan_source_archive(args.source_archive_root)
    initial = build_receipt(
        material_contract,
        render_receipt,
        shader_receipt,
        args.material_contract,
        args.render_receipt,
        args.shader_receipt,
        args.hlsl,
        source_archive,
    )
    if args.run_hlsl:
        from verify_artist_31470_material_runtime_oracle_hlsl import run_hlsl_oracle

        hlsl_verification = run_hlsl_oracle(initial, args.hlsl, args.d3dcompiler)
        return build_receipt(
            material_contract,
            render_receipt,
            shader_receipt,
            args.material_contract,
            args.render_receipt,
            args.shader_receipt,
            args.hlsl,
            source_archive,
            hlsl_verification,
        )
    return initial


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--material-contract", type=Path, default=DEFAULT_MATERIAL_CONTRACT)
    parser.add_argument("--render-receipt", type=Path, default=DEFAULT_RENDER_RECEIPT)
    parser.add_argument("--shader-receipt", type=Path, default=DEFAULT_SHADER_RECEIPT)
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
        validate_runtime_receipt_source_bindings(
            receipt, read_json(args.material_contract)
        )
        tracked_sources = {
            "hlslTrackedTextSha256": args.hlsl,
            "generatorTrackedTextSha256": GENERATOR_PATH,
            "materialContractBuilderTrackedTextSha256": MATERIAL_CONTRACT_BUILDER_PATH,
            "shaderCacheOracleTrackedTextSha256": SHADER_CACHE_ORACLE_PATH,
            "ue3PackageParserTrackedTextSha256": UE3_PACKAGE_PARSER_PATH,
            "hlslVerifierTrackedTextSha256": HLSL_VERIFIER_PATH,
        }
        for key, path in tracked_sources.items():
            require(
                receipt["sourceEvidence"][key] == tracked_text_sha256(path),
                f"Material oracle tracked source changed: {key}",
            )
        print("PASS: Artist F Material runtime oracle shallow family=23 recipe=27 occurrence=34 product=false")
        return 0
    candidate = build_from_paths(args)
    if args.check:
        require(args.output.is_file(), f"Material runtime oracle receipt is missing: {args.output}")
        require(read_json(args.output) == candidate, "Material runtime oracle receipt is stale")
        print("PASS: Artist F Material runtime oracle deep family=23 recipe=27 occurrence=34 product=false")
        return 0
    write_json_atomic(args.output, candidate)
    print(f"wrote {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
