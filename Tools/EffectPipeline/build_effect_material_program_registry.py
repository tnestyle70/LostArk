#!/usr/bin/env python3
"""Validate and emit the immutable Effect material-program registry.

The registry describes data only.  Its adapter IDs select a compiled Client
allowlist; authoring data cannot provide shader paths, passes, MRTs, or draw
dispatch.  When bindings exist, this tool resolves their Program/Layout/
Descriptor packet and compares it with the direct-authored inline execution
mirror, including every float's IEEE-754 binary32 representation.
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path, PurePosixPath
import re
import struct
import sys
from typing import Any, Mapping


REGISTRY_SCHEMA = "lostark.effect-material-program-registry"
REGISTRY_VERSION = 1
DIRECT_PAYLOAD_KIND = "DIRECT_AUTHORED_DOCUMENT_V13"
AUTHORING_SCHEMA = "lostark.effect-authoring"
AUTHORING_VERSION = 13
CANONICAL_SPRITE_ADAPTER_ID = (
    "effect.adapter.sprite-particle.scene-color-rt0."
    "zero-distortion-rt1.alpha-two-sided.v1"
)

ROOT_KEYS = (
    "schema",
    "formatVersion",
    "programs",
    "layouts",
    "descriptors",
    "adapters",
    "bindings",
)
PROGRAM_KEYS = ("programId", "backend", "opcode")
LAYOUT_KEYS = (
    "layoutId",
    "executionVersion",
    "textureLaneCount",
    "textureMask",
    "textureLanes",
    "dynamicConsumedMask",
    "dynamicSuppressedMask",
    "particleColorPolicy",
    "particleColorConsumedMask",
    "particleColorSuppressedMask",
    "scalarCount",
    "vectorCount",
    "inputCount",
    "inputConsumedMask",
    "inputSuppressedMask",
    "vectorComponentConsumedMask",
    "vectorComponentSuppressedMask",
    "staticInputCount",
    "staticSelectedMask",
    "staticConsumedMask",
    "staticSuppressedMask",
    "renderInputCount",
    "renderConsumedMask",
    "renderSuppressedMask",
    "scalarRows",
    "vectorRows",
    "artistParameterRows",
    "colorRows",
)
LAYOUT_TEXTURE_LANE_KEYS = (
    "laneId",
    "role",
    "textureRegister",
    "samplerRegister",
    "sourceChannel",
    "colorSpace",
)
PACKED_ROW_KEYS = ("name", "packedIndex")
DESCRIPTOR_KEYS = (
    "descriptorId",
    "layoutId",
    "textureLanes",
    "scalars",
    "vectors",
    "artistParameters",
    "colors",
)
DESCRIPTOR_TEXTURE_LANE_KEYS = ("laneId", "assetId", "sampler")
SAMPLER_KEYS = (
    "filter",
    "addressU",
    "addressV",
    "addressW",
    "mipLodBias",
    "maxAnisotropy",
    "comparison",
    "borderColor",
    "minLod",
    "maxLod",
)
SCALAR_VALUE_KEYS = ("name", "value")
VECTOR_VALUE_KEYS = ("name", "value")
ADAPTER_KEYS = ("adapterId",)
BINDING_KEYS = (
    "effectAssetId",
    "elementId",
    "programId",
    "layoutId",
    "descriptorId",
    "adapterId",
)
EXECUTION_KEYS = (
    "enabled",
    "version",
    "backend",
    "opcode",
    "passIndex",
    "renderState",
    "textureLaneCount",
    "textureMask",
    "textureLanes",
    "dynamicConsumedMask",
    "dynamicSuppressedMask",
    "particleColorPolicy",
    "particleColorConsumedMask",
    "particleColorSuppressedMask",
    "scalarCount",
    "vectorCount",
    "inputCount",
    "inputConsumedMask",
    "inputSuppressedMask",
    "vectorComponentConsumedMask",
    "vectorComponentSuppressedMask",
    "staticInputCount",
    "staticSelectedMask",
    "staticConsumedMask",
    "staticSuppressedMask",
    "renderInputCount",
    "renderConsumedMask",
    "renderSuppressedMask",
    "scalars",
    "vectors",
    "artistParameters",
    "colors",
)
RENDER_STATE_KEYS = (
    "rasterizer",
    "depthStencil",
    "blend",
    "stencilReference",
)
EXECUTION_TEXTURE_LANE_KEYS = (
    "laneId",
    "role",
    "assetId",
    "textureRegister",
    "samplerRegister",
    "sourceChannel",
    "colorSpace",
    "sampler",
)
EXECUTION_SCALAR_KEYS = ("name", "packedIndex", "value")
EXECUTION_VECTOR_KEYS = ("name", "packedIndex", "value")

STABLE_ID_PATTERN = re.compile(r"^[a-z0-9]+(?:[._-][a-z0-9]+)*$")
UINT32_MAX = (1 << 32) - 1
MAX_PROGRAM_ROWS = 4096
MAX_LAYOUT_ROWS = 4096
MAX_DESCRIPTOR_ROWS = 4096
MAX_ADAPTER_ROWS = 64
MAX_BINDING_ROWS = 65536
MAX_TEXTURE_LANES = 6
MAX_SCALAR_ROWS = 52
MAX_VECTOR_ROWS = 3
MAX_NAMED_VECTOR_ROWS = 64


class ContractError(RuntimeError):
    """Raised when the material-program registry violates its contract."""


def _object_no_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ContractError(f"duplicate JSON property: {key}")
        result[key] = value
    return result


def load_json(
    path: Path,
    label: str = "material-program registry",
    require_lf: bool = True,
) -> dict[str, Any]:
    try:
        payload = path.read_bytes()
    except OSError as exc:
        raise ContractError(f"cannot read {label}: {exc}") from exc
    if payload.startswith(b"\xef\xbb\xbf"):
        raise ContractError(f"{label} must be UTF-8 without BOM")
    if require_lf and b"\r" in payload:
        raise ContractError(f"{label} must use LF line endings")
    try:
        value = json.loads(
            payload.decode("utf-8"),
            object_pairs_hook=_object_no_duplicates,
            parse_constant=lambda token: (_ for _ in ()).throw(
                ContractError(f"{label} contains non-finite number: {token}")
            ),
        )
    except ContractError:
        raise
    except (UnicodeError, json.JSONDecodeError) as exc:
        raise ContractError(f"{label} JSON is invalid: {exc}") from exc
    if not isinstance(value, dict):
        raise ContractError(f"{label} root must be an object")
    return value


def _require_exact_order(value: Any, keys: tuple[str, ...], label: str) -> dict[str, Any]:
    if not isinstance(value, dict) or tuple(value.keys()) != keys:
        raise ContractError(f"{label} fields or order are invalid")
    return value


def _require_array(
    value: Any, label: str, minimum: int, maximum: int
) -> list[Any]:
    if not isinstance(value, list) or not minimum <= len(value) <= maximum:
        raise ContractError(f"{label} array size is invalid")
    return value


def _require_uint32(value: Any, label: str, maximum: int = UINT32_MAX) -> int:
    if type(value) is not int or not 0 <= value <= maximum:
        raise ContractError(f"{label} must be an unsigned integer")
    return value


def _require_stable_id(value: Any, label: str) -> str:
    if (
        not isinstance(value, str)
        or len(value) > 255
        or STABLE_ID_PATTERN.fullmatch(value) is None
    ):
        raise ContractError(f"{label} is not a stable ID")
    return value


def _require_text(value: Any, label: str, maximum: int = 255) -> str:
    if (
        not isinstance(value, str)
        or len(value) > maximum
        or any(ord(character) < 0x20 or ord(character) == 0x7F for character in value)
    ):
        raise ContractError(f"{label} text is invalid")
    return value


def _float32_bits(value: Any, label: str) -> int:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ContractError(f"{label} must be a finite binary32 value")
    converted = float(value)
    if not math.isfinite(converted):
        raise ContractError(f"{label} must be a finite binary32 value")
    try:
        packed = struct.pack("<f", converted)
    except (OverflowError, struct.error) as exc:
        raise ContractError(f"{label} is outside binary32 range") from exc
    if not math.isfinite(struct.unpack("<f", packed)[0]):
        raise ContractError(f"{label} is outside finite binary32 range")
    return struct.unpack("<I", packed)[0]


def _validate_float4(value: Any, label: str) -> tuple[int, int, int, int]:
    if not isinstance(value, list) or len(value) != 4:
        raise ContractError(f"{label} must contain exactly four values")
    return tuple(
        _float32_bits(component, f"{label}[{index}]")
        for index, component in enumerate(value)
    )  # type: ignore[return-value]


def _validate_uint32_array(value: Any, length: int, label: str) -> list[int]:
    if not isinstance(value, list) or len(value) != length:
        raise ContractError(f"{label} must contain exactly {length} masks")
    return [
        _require_uint32(item, f"{label}[{index}]")
        for index, item in enumerate(value)
    ]


def _validate_mask_pair(consumed: int, suppressed: int, label: str) -> None:
    if consumed & suppressed:
        raise ContractError(f"{label} consumed/suppressed masks overlap")


def _validate_mask_count(mask: int, count: int, label: str) -> None:
    allowed = UINT32_MAX if count >= 32 else ((1 << count) - 1 if count else 0)
    if mask & ~allowed:
        raise ContractError(f"{label} contains bits outside its declared count")


def _validate_packed_rows(
    value: Any, label: str, maximum: int
) -> list[dict[str, Any]]:
    rows = _require_array(value, label, 0, maximum)
    names: set[str] = set()
    for index, row in enumerate(rows):
        _require_exact_order(row, PACKED_ROW_KEYS, f"{label}[{index}]")
        name = _require_stable_id(row["name"], f"{label}[{index}].name")
        if name in names:
            raise ContractError(f"duplicate {label} name: {name}")
        names.add(name)
        if _require_uint32(row["packedIndex"], f"{label}[{index}].packedIndex") != index:
            raise ContractError(f"{label} packed indices must be contiguous and ordered")
    return rows


def _validate_asset_id(value: Any, label: str) -> str:
    if (
        not isinstance(value, str)
        or len(value) > 1024
        or not value.startswith("Effect/")
        or not value.endswith(".dds")
        or "\\" in value
        or ":" in value
        or "//" in value
        or any(part in ("", ".", "..") for part in value.split("/"))
        or any(ord(character) < 0x20 or ord(character) == 0x7F for character in value)
    ):
        raise ContractError(f"{label} is not a safe Effect texture asset ID")
    return value


def _validate_sampler(value: Any, label: str) -> None:
    _require_exact_order(value, SAMPLER_KEYS, label)
    if value["filter"] not in ("point", "linear", "anisotropic"):
        raise ContractError(f"{label}.filter is unsupported")
    for field in ("addressU", "addressV", "addressW"):
        if value[field] not in ("wrap", "mirror", "clamp", "border"):
            raise ContractError(f"{label}.{field} is unsupported")
    _float32_bits(value["mipLodBias"], f"{label}.mipLodBias")
    _require_uint32(value["maxAnisotropy"], f"{label}.maxAnisotropy", 16)
    if value["maxAnisotropy"] < 1:
        raise ContractError(f"{label}.maxAnisotropy must be at least one")
    if value["comparison"] not in (
        "never",
        "less",
        "equal",
        "lessEqual",
        "greater",
        "notEqual",
        "greaterEqual",
        "always",
    ):
        raise ContractError(f"{label}.comparison is unsupported")
    _validate_float4(value["borderColor"], f"{label}.borderColor")
    min_lod = _float32_bits(value["minLod"], f"{label}.minLod")
    max_lod = _float32_bits(value["maxLod"], f"{label}.maxLod")
    if struct.unpack("<f", struct.pack("<I", min_lod))[0] > struct.unpack(
        "<f", struct.pack("<I", max_lod)
    )[0]:
        raise ContractError(f"{label} minLod exceeds maxLod")


def _validate_programs(value: Any) -> dict[str, dict[str, Any]]:
    programs = _require_array(value, "programs", 1, MAX_PROGRAM_ROWS)
    result: dict[str, dict[str, Any]] = {}
    for index, row in enumerate(programs):
        _require_exact_order(row, PROGRAM_KEYS, f"programs[{index}]")
        row_id = _require_stable_id(row["programId"], f"programs[{index}].programId")
        if row_id in result:
            raise ContractError(f"duplicate programId: {row_id}")
        if row["backend"] != "runtimeMaterialV2":
            raise ContractError(f"program {row_id} backend is unsupported")
        _require_uint32(row["opcode"], f"program {row_id} opcode")
        result[row_id] = row
    return result


def _validate_layouts(value: Any) -> dict[str, dict[str, Any]]:
    layouts = _require_array(value, "layouts", 1, MAX_LAYOUT_ROWS)
    result: dict[str, dict[str, Any]] = {}
    for index, row in enumerate(layouts):
        _require_exact_order(row, LAYOUT_KEYS, f"layouts[{index}]")
        row_id = _require_stable_id(row["layoutId"], f"layouts[{index}].layoutId")
        if row_id in result:
            raise ContractError(f"duplicate layoutId: {row_id}")
        if type(row["executionVersion"]) is not int or row["executionVersion"] != 1:
            raise ContractError(f"layout {row_id} executionVersion is unsupported")
        lane_count = _require_uint32(
            row["textureLaneCount"], f"layout {row_id} textureLaneCount", MAX_TEXTURE_LANES
        )
        texture_mask = _require_uint32(row["textureMask"], f"layout {row_id} textureMask")
        expected_texture_mask = (1 << lane_count) - 1 if lane_count else 0
        if texture_mask != expected_texture_mask:
            raise ContractError(f"layout {row_id} textureMask/count mismatch")
        lanes = _require_array(row["textureLanes"], f"layout {row_id} textureLanes", lane_count, lane_count)
        lane_ids: set[str] = set()
        for lane_index, lane in enumerate(lanes):
            label = f"layout {row_id} textureLanes[{lane_index}]"
            _require_exact_order(lane, LAYOUT_TEXTURE_LANE_KEYS, label)
            lane_id = _require_stable_id(lane["laneId"], f"{label}.laneId")
            if lane_id in lane_ids:
                raise ContractError(f"layout {row_id} has duplicate laneId: {lane_id}")
            lane_ids.add(lane_id)
            _require_stable_id(lane["role"], f"{label}.role")
            if _require_uint32(lane["textureRegister"], f"{label}.textureRegister", 15) != lane_index:
                raise ContractError(f"layout {row_id} texture registers must be contiguous and ordered")
            sampler_register = _require_uint32(
                lane["samplerRegister"], f"{label}.samplerRegister", 15
            )
            if sampler_register != 5 + lane_index:
                raise ContractError(
                    f"layout {row_id} sampler registers must be contiguous from s5"
                )
            _require_text(lane["sourceChannel"], f"{label}.sourceChannel", 64)
            if lane["colorSpace"] not in ("linear", "srgb"):
                raise ContractError(f"{label}.colorSpace is unsupported")

        uint_fields = (
            "dynamicConsumedMask",
            "dynamicSuppressedMask",
            "particleColorPolicy",
            "particleColorConsumedMask",
            "particleColorSuppressedMask",
            "inputCount",
            "staticInputCount",
            "staticSelectedMask",
            "staticConsumedMask",
            "staticSuppressedMask",
            "renderInputCount",
            "renderConsumedMask",
            "renderSuppressedMask",
        )
        for field in uint_fields:
            _require_uint32(row[field], f"layout {row_id} {field}")
        scalar_count = _require_uint32(row["scalarCount"], f"layout {row_id} scalarCount", MAX_SCALAR_ROWS)
        vector_count = _require_uint32(row["vectorCount"], f"layout {row_id} vectorCount", MAX_VECTOR_ROWS)
        input_masks = _validate_uint32_array(row["inputConsumedMask"], 2, f"layout {row_id} inputConsumedMask")
        input_suppressed = _validate_uint32_array(row["inputSuppressedMask"], 2, f"layout {row_id} inputSuppressedMask")
        vector_masks = _validate_uint32_array(row["vectorComponentConsumedMask"], 3, f"layout {row_id} vectorComponentConsumedMask")
        vector_suppressed = _validate_uint32_array(row["vectorComponentSuppressedMask"], 3, f"layout {row_id} vectorComponentSuppressedMask")
        for mask_index in range(2):
            _validate_mask_pair(input_masks[mask_index], input_suppressed[mask_index], f"layout {row_id} input[{mask_index}]")
        for mask_index in range(3):
            _validate_mask_pair(vector_masks[mask_index], vector_suppressed[mask_index], f"layout {row_id} vectorComponent[{mask_index}]")
            if mask_index >= vector_count and (vector_masks[mask_index] or vector_suppressed[mask_index]):
                raise ContractError(f"layout {row_id} vector component mask has no vector row")
        _validate_mask_pair(row["dynamicConsumedMask"], row["dynamicSuppressedMask"], f"layout {row_id} dynamic")
        _validate_mask_pair(row["particleColorConsumedMask"], row["particleColorSuppressedMask"], f"layout {row_id} particleColor")
        _validate_mask_pair(row["staticConsumedMask"], row["staticSuppressedMask"], f"layout {row_id} static")
        _validate_mask_pair(row["renderConsumedMask"], row["renderSuppressedMask"], f"layout {row_id} render")
        input_count = row["inputCount"]
        if input_count > 64:
            raise ContractError(f"layout {row_id} inputCount exceeds its two-word masks")
        for mask_index, (consumed, suppressed) in enumerate(zip(input_masks, input_suppressed)):
            word_count = max(0, min(32, input_count - mask_index * 32))
            _validate_mask_count(consumed | suppressed, word_count, f"layout {row_id} input[{mask_index}]")
        if row["staticInputCount"] > 32 or row["renderInputCount"] > 32:
            raise ContractError(f"layout {row_id} static/render input count exceeds one mask word")
        _validate_mask_count(row["staticSelectedMask"] | row["staticConsumedMask"] | row["staticSuppressedMask"], row["staticInputCount"], f"layout {row_id} static")
        _validate_mask_count(row["renderConsumedMask"] | row["renderSuppressedMask"], row["renderInputCount"], f"layout {row_id} render")

        scalar_rows = _validate_packed_rows(row["scalarRows"], f"layout {row_id} scalarRows", MAX_SCALAR_ROWS)
        vector_rows = _validate_packed_rows(row["vectorRows"], f"layout {row_id} vectorRows", MAX_VECTOR_ROWS)
        _validate_packed_rows(row["artistParameterRows"], f"layout {row_id} artistParameterRows", MAX_NAMED_VECTOR_ROWS)
        _validate_packed_rows(row["colorRows"], f"layout {row_id} colorRows", MAX_NAMED_VECTOR_ROWS)
        if len(scalar_rows) != scalar_count or len(vector_rows) != vector_count:
            raise ContractError(f"layout {row_id} packed row count mismatch")
        result[row_id] = row
    return result


def _validate_value_rows(
    value: Any,
    layout_rows: list[dict[str, Any]],
    label: str,
    vector: bool,
) -> None:
    rows = _require_array(value, label, len(layout_rows), len(layout_rows))
    keys = VECTOR_VALUE_KEYS if vector else SCALAR_VALUE_KEYS
    for index, (row, layout_row) in enumerate(zip(rows, layout_rows)):
        _require_exact_order(row, keys, f"{label}[{index}]")
        if row["name"] != layout_row["name"]:
            raise ContractError(f"{label}[{index}] name/layout mismatch")
        if vector:
            _validate_float4(row["value"], f"{label}[{index}].value")
        else:
            _float32_bits(row["value"], f"{label}[{index}].value")


def _validate_descriptors(
    value: Any, layouts: Mapping[str, dict[str, Any]]
) -> dict[str, dict[str, Any]]:
    descriptors = _require_array(value, "descriptors", 1, MAX_DESCRIPTOR_ROWS)
    result: dict[str, dict[str, Any]] = {}
    for index, row in enumerate(descriptors):
        _require_exact_order(row, DESCRIPTOR_KEYS, f"descriptors[{index}]")
        row_id = _require_stable_id(row["descriptorId"], f"descriptors[{index}].descriptorId")
        if row_id in result:
            raise ContractError(f"duplicate descriptorId: {row_id}")
        layout_id = _require_stable_id(row["layoutId"], f"descriptor {row_id} layoutId")
        layout = layouts.get(layout_id)
        if layout is None:
            raise ContractError(f"descriptor {row_id} references unknown layoutId: {layout_id}")
        lanes = _require_array(row["textureLanes"], f"descriptor {row_id} textureLanes", layout["textureLaneCount"], layout["textureLaneCount"])
        for lane_index, (lane, layout_lane) in enumerate(zip(lanes, layout["textureLanes"])):
            label = f"descriptor {row_id} textureLanes[{lane_index}]"
            _require_exact_order(lane, DESCRIPTOR_TEXTURE_LANE_KEYS, label)
            if lane["laneId"] != layout_lane["laneId"]:
                raise ContractError(f"{label}.laneId/layout mismatch")
            _validate_asset_id(lane["assetId"], f"{label}.assetId")
            _validate_sampler(lane["sampler"], f"{label}.sampler")
        _validate_value_rows(row["scalars"], layout["scalarRows"], f"descriptor {row_id} scalars", False)
        _validate_value_rows(row["vectors"], layout["vectorRows"], f"descriptor {row_id} vectors", True)
        _validate_value_rows(row["artistParameters"], layout["artistParameterRows"], f"descriptor {row_id} artistParameters", True)
        _validate_value_rows(row["colors"], layout["colorRows"], f"descriptor {row_id} colors", True)
        result[row_id] = row
    return result


def _validate_adapters(value: Any) -> dict[str, dict[str, Any]]:
    adapters = _require_array(value, "adapters", 1, MAX_ADAPTER_ROWS)
    result: dict[str, dict[str, Any]] = {}
    for index, row in enumerate(adapters):
        _require_exact_order(row, ADAPTER_KEYS, f"adapters[{index}]")
        adapter_id = _require_stable_id(row["adapterId"], f"adapters[{index}].adapterId")
        if adapter_id != CANONICAL_SPRITE_ADAPTER_ID:
            raise ContractError(f"unsupported compiled adapterId: {adapter_id}")
        if adapter_id in result:
            raise ContractError(f"duplicate adapterId: {adapter_id}")
        result[adapter_id] = row
    return result


def materialize_binding(
    binding: Mapping[str, Any],
    programs: Mapping[str, dict[str, Any]],
    layouts: Mapping[str, dict[str, Any]],
    descriptors: Mapping[str, dict[str, Any]],
) -> dict[str, Any]:
    program = programs[binding["programId"]]
    layout = layouts[binding["layoutId"]]
    descriptor = descriptors[binding["descriptorId"]]
    texture_lanes = []
    for layout_lane, descriptor_lane in zip(
        layout["textureLanes"], descriptor["textureLanes"]
    ):
        texture_lanes.append(
            {
                "laneId": layout_lane["laneId"],
                "role": layout_lane["role"],
                "assetId": descriptor_lane["assetId"],
                "textureRegister": layout_lane["textureRegister"],
                "samplerRegister": layout_lane["samplerRegister"],
                "sourceChannel": layout_lane["sourceChannel"],
                "colorSpace": layout_lane["colorSpace"],
                "sampler": descriptor_lane["sampler"],
            }
        )

    def merge_rows(layout_key: str, descriptor_key: str) -> list[dict[str, Any]]:
        return [
            {
                "name": layout_row["name"],
                "packedIndex": layout_row["packedIndex"],
                "value": descriptor_row["value"],
            }
            for layout_row, descriptor_row in zip(
                layout[layout_key], descriptor[descriptor_key]
            )
        ]

    return {
        "enabled": True,
        "version": layout["executionVersion"],
        "backend": program["backend"],
        "opcode": program["opcode"],
        "passIndex": 1,
        "renderState": {
            "rasterizer": "RS_Cull_None",
            "depthStencil": "DSS_ReadOnly",
            "blend": "BS_EffectAlpha",
            "stencilReference": 0,
        },
        "textureLaneCount": layout["textureLaneCount"],
        "textureMask": layout["textureMask"],
        "textureLanes": texture_lanes,
        "dynamicConsumedMask": layout["dynamicConsumedMask"],
        "dynamicSuppressedMask": layout["dynamicSuppressedMask"],
        "particleColorPolicy": layout["particleColorPolicy"],
        "particleColorConsumedMask": layout["particleColorConsumedMask"],
        "particleColorSuppressedMask": layout["particleColorSuppressedMask"],
        "scalarCount": layout["scalarCount"],
        "vectorCount": layout["vectorCount"],
        "inputCount": layout["inputCount"],
        "inputConsumedMask": layout["inputConsumedMask"],
        "inputSuppressedMask": layout["inputSuppressedMask"],
        "vectorComponentConsumedMask": layout["vectorComponentConsumedMask"],
        "vectorComponentSuppressedMask": layout["vectorComponentSuppressedMask"],
        "staticInputCount": layout["staticInputCount"],
        "staticSelectedMask": layout["staticSelectedMask"],
        "staticConsumedMask": layout["staticConsumedMask"],
        "staticSuppressedMask": layout["staticSuppressedMask"],
        "renderInputCount": layout["renderInputCount"],
        "renderConsumedMask": layout["renderConsumedMask"],
        "renderSuppressedMask": layout["renderSuppressedMask"],
        "scalars": merge_rows("scalarRows", "scalars"),
        "vectors": merge_rows("vectorRows", "vectors"),
        "artistParameters": merge_rows("artistParameterRows", "artistParameters"),
        "colors": merge_rows("colorRows", "colors"),
    }


def _require_same(actual: Any, expected: Any, label: str) -> None:
    if type(actual) is not type(expected) or actual != expected:
        raise ContractError(f"materialized execution mismatch at {label}")


def assert_execution_bit_exact(
    materialized: Mapping[str, Any], authored: Any, label: str
) -> None:
    _require_exact_order(materialized, EXECUTION_KEYS, f"{label} materialized execution")
    _require_exact_order(authored, EXECUTION_KEYS, f"{label} authored execution")
    simple_fields = (
        "enabled",
        "version",
        "backend",
        "opcode",
        "passIndex",
        "textureLaneCount",
        "textureMask",
        "dynamicConsumedMask",
        "dynamicSuppressedMask",
        "particleColorPolicy",
        "particleColorConsumedMask",
        "particleColorSuppressedMask",
        "scalarCount",
        "vectorCount",
        "inputCount",
        "inputConsumedMask",
        "inputSuppressedMask",
        "vectorComponentConsumedMask",
        "vectorComponentSuppressedMask",
        "staticInputCount",
        "staticSelectedMask",
        "staticConsumedMask",
        "staticSuppressedMask",
        "renderInputCount",
        "renderConsumedMask",
        "renderSuppressedMask",
    )
    for field in simple_fields:
        _require_same(authored[field], materialized[field], f"{label}.{field}")
    _require_exact_order(materialized["renderState"], RENDER_STATE_KEYS, f"{label}.renderState")
    _require_exact_order(authored["renderState"], RENDER_STATE_KEYS, f"{label}.renderState")
    for field in RENDER_STATE_KEYS:
        _require_same(authored["renderState"][field], materialized["renderState"][field], f"{label}.renderState.{field}")

    materialized_lanes = materialized["textureLanes"]
    authored_lanes = authored["textureLanes"]
    if not isinstance(authored_lanes, list) or len(authored_lanes) != len(materialized_lanes):
        raise ContractError(f"materialized execution mismatch at {label}.textureLanes")
    for index, (expected_lane, actual_lane) in enumerate(zip(materialized_lanes, authored_lanes)):
        lane_label = f"{label}.textureLanes[{index}]"
        _require_exact_order(expected_lane, EXECUTION_TEXTURE_LANE_KEYS, lane_label)
        _require_exact_order(actual_lane, EXECUTION_TEXTURE_LANE_KEYS, lane_label)
        for field in EXECUTION_TEXTURE_LANE_KEYS[:-1]:
            _require_same(actual_lane[field], expected_lane[field], f"{lane_label}.{field}")
        expected_sampler = expected_lane["sampler"]
        actual_sampler = actual_lane["sampler"]
        _require_exact_order(expected_sampler, SAMPLER_KEYS, f"{lane_label}.sampler")
        _require_exact_order(actual_sampler, SAMPLER_KEYS, f"{lane_label}.sampler")
        for field in ("filter", "addressU", "addressV", "addressW", "maxAnisotropy", "comparison"):
            _require_same(actual_sampler[field], expected_sampler[field], f"{lane_label}.sampler.{field}")
        for field in ("mipLodBias", "minLod", "maxLod"):
            if _float32_bits(actual_sampler[field], f"{lane_label}.sampler.{field}") != _float32_bits(expected_sampler[field], f"{lane_label}.sampler.{field}"):
                raise ContractError(f"materialized execution float-bit mismatch at {lane_label}.sampler.{field}")
        if _validate_float4(actual_sampler["borderColor"], f"{lane_label}.sampler.borderColor") != _validate_float4(expected_sampler["borderColor"], f"{lane_label}.sampler.borderColor"):
            raise ContractError(f"materialized execution float-bit mismatch at {lane_label}.sampler.borderColor")

    for field, row_keys, vector in (
        ("scalars", EXECUTION_SCALAR_KEYS, False),
        ("vectors", EXECUTION_VECTOR_KEYS, True),
        ("artistParameters", EXECUTION_VECTOR_KEYS, True),
        ("colors", EXECUTION_VECTOR_KEYS, True),
    ):
        expected_rows = materialized[field]
        actual_rows = authored[field]
        if not isinstance(actual_rows, list) or len(actual_rows) != len(expected_rows):
            raise ContractError(f"materialized execution mismatch at {label}.{field}")
        for index, (expected_row, actual_row) in enumerate(zip(expected_rows, actual_rows)):
            row_label = f"{label}.{field}[{index}]"
            _require_exact_order(expected_row, row_keys, row_label)
            _require_exact_order(actual_row, row_keys, row_label)
            _require_same(actual_row["name"], expected_row["name"], f"{row_label}.name")
            _require_same(actual_row["packedIndex"], expected_row["packedIndex"], f"{row_label}.packedIndex")
            if vector:
                if _validate_float4(actual_row["value"], f"{row_label}.value") != _validate_float4(expected_row["value"], f"{row_label}.value"):
                    raise ContractError(f"materialized execution float-bit mismatch at {row_label}.value")
            elif _float32_bits(actual_row["value"], f"{row_label}.value") != _float32_bits(expected_row["value"], f"{row_label}.value"):
                raise ContractError(f"materialized execution float-bit mismatch at {row_label}.value")


def _find_authored_execution(
    authored_documents: Mapping[str, dict[str, Any]],
    effect_id: str,
    element_id: str,
) -> Any:
    document = authored_documents.get(effect_id)
    if document is None:
        raise ContractError(f"binding target effect is not direct-authored: {effect_id}")
    if (
        document.get("schema") != AUTHORING_SCHEMA
        or type(document.get("version")) is not int
        or document.get("version") != AUTHORING_VERSION
        or document.get("effectAssetId") != effect_id
        or not isinstance(document.get("elements"), list)
    ):
        raise ContractError(f"binding target authored document identity mismatch: {effect_id}")
    matching = [
        element
        for element in document["elements"]
        if isinstance(element, dict) and element.get("id") == element_id
    ]
    if len(matching) != 1:
        raise ContractError(f"binding target element is missing or duplicate: {effect_id}/{element_id}")
    material = matching[0].get("material")
    if not isinstance(material, dict) or "execution" not in material:
        raise ContractError(f"binding target has no inline material execution: {effect_id}/{element_id}")
    return material["execution"]


def validate_registry(
    value: Any,
    authored_documents: Mapping[str, dict[str, Any]] | None = None,
) -> dict[str, Any]:
    _require_exact_order(value, ROOT_KEYS, "material-program registry root")
    if value["schema"] != REGISTRY_SCHEMA:
        raise ContractError("material-program registry schema mismatch")
    if type(value["formatVersion"]) is not int or value["formatVersion"] != REGISTRY_VERSION:
        raise ContractError("material-program registry formatVersion mismatch")
    programs = _validate_programs(value["programs"])
    layouts = _validate_layouts(value["layouts"])
    descriptors = _validate_descriptors(value["descriptors"], layouts)
    adapters = _validate_adapters(value["adapters"])
    bindings = _require_array(value["bindings"], "bindings", 0, MAX_BINDING_ROWS)
    identities: set[tuple[str, str]] = set()
    for index, row in enumerate(bindings):
        _require_exact_order(row, BINDING_KEYS, f"bindings[{index}]")
        effect_id = _require_stable_id(row["effectAssetId"], f"bindings[{index}].effectAssetId")
        element_id = _require_stable_id(row["elementId"], f"bindings[{index}].elementId")
        identity = (effect_id, element_id)
        if identity in identities:
            raise ContractError(f"duplicate material-program binding: {effect_id}/{element_id}")
        identities.add(identity)
        for field, lookup in (
            ("programId", programs),
            ("layoutId", layouts),
            ("descriptorId", descriptors),
            ("adapterId", adapters),
        ):
            target_id = _require_stable_id(row[field], f"bindings[{index}].{field}")
            if target_id not in lookup:
                raise ContractError(f"binding {effect_id}/{element_id} references unknown {field}: {target_id}")
        descriptor = descriptors[row["descriptorId"]]
        if descriptor["layoutId"] != row["layoutId"]:
            raise ContractError(f"binding {effect_id}/{element_id} descriptor/layout mismatch")
        if (
            row["adapterId"] == CANONICAL_SPRITE_ADAPTER_ID
            and (
                programs[row["programId"]]["backend"] != "runtimeMaterialV2"
                or programs[row["programId"]]["opcode"] != 6
            )
        ):
            raise ContractError(f"binding {effect_id}/{element_id} program is unsupported by its adapter")
        if authored_documents is not None:
            authored = _find_authored_execution(authored_documents, effect_id, element_id)
            materialized = materialize_binding(row, programs, layouts, descriptors)
            assert_execution_bit_exact(materialized, authored, f"{effect_id}/{element_id}")
    if bindings and authored_documents is None:
        raise ContractError("binding target validation requires authored documents")
    return value


def _safe_authoring_path(data_root: Path, value: Any, effect_id: str) -> Path:
    if not isinstance(value, str):
        raise ContractError(f"direct-authored source path is invalid: {effect_id}")
    relative = PurePosixPath(value)
    if (
        relative.is_absolute()
        or "\\" in value
        or ":" in value
        or "//" in value
        or not relative.parts
        or relative.parts[0] != "Effects"
        or any(part in ("", ".", "..") for part in relative.parts)
    ):
        raise ContractError(f"direct-authored source path is unsafe: {effect_id}")
    root = data_root.resolve(strict=True)
    candidate = data_root.joinpath(*relative.parts).resolve(strict=True)
    try:
        candidate.relative_to(root)
    except ValueError as exc:
        raise ContractError(f"direct-authored source path escapes Data root: {effect_id}") from exc
    if not candidate.is_file():
        raise ContractError(f"direct-authored source is not a regular file: {effect_id}")
    return candidate


def load_binding_authored_documents(
    registry: Mapping[str, Any], effect_catalog_path: Path, data_root: Path
) -> dict[str, dict[str, Any]]:
    requested = {
        binding.get("effectAssetId")
        for binding in registry.get("bindings", [])
        if isinstance(binding, dict)
    }
    if not requested:
        return {}
    catalog = load_json(
        effect_catalog_path, "Effect source catalog", require_lf=False
    )
    entries = catalog.get("effects")
    if not isinstance(entries, list):
        raise ContractError("Effect source catalog effects array is invalid")
    by_id: dict[str, dict[str, Any]] = {}
    for entry in entries:
        if not isinstance(entry, dict) or not isinstance(entry.get("effectAssetId"), str):
            continue
        effect_id = entry["effectAssetId"]
        if effect_id not in requested:
            continue
        if effect_id in by_id:
            raise ContractError(f"duplicate binding target in Effect source catalog: {effect_id}")
        if entry.get("payloadKind") != DIRECT_PAYLOAD_KIND:
            raise ContractError(f"binding target must be direct-authored v13: {effect_id}")
        path = _safe_authoring_path(data_root, entry.get("authoringPath"), effect_id)
        by_id[effect_id] = load_json(
            path, f"authored Effect document {effect_id}", require_lf=False
        )
    missing = sorted(requested - set(by_id))
    if missing:
        raise ContractError(f"binding target effect is missing from source catalog: {missing[0]}")
    return by_id


def build_registry(
    source_path: Path,
    effect_catalog_path: Path | None = None,
    data_root: Path | None = None,
) -> dict[str, Any]:
    registry = load_json(source_path)
    authored_documents: Mapping[str, dict[str, Any]] | None = None
    if registry.get("bindings"):
        if effect_catalog_path is None or data_root is None:
            raise ContractError("bound registry build requires --effect-catalog and --data-root")
        authored_documents = load_binding_authored_documents(
            registry, effect_catalog_path, data_root
        )
    return validate_registry(registry, authored_documents)


def make_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--effect-catalog", type=Path)
    parser.add_argument("--data-root", type=Path)
    return parser


def main(argv: list[str] | None = None) -> int:
    args = make_parser().parse_args(argv)
    try:
        registry = build_registry(
            args.source.resolve(),
            args.effect_catalog.resolve() if args.effect_catalog else None,
            args.data_root.resolve() if args.data_root else None,
        )
        print(
            json.dumps(
                registry,
                ensure_ascii=False,
                separators=(",", ":"),
                allow_nan=False,
            )
        )
    except (ContractError, OSError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
