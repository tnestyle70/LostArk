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
import hashlib
import json
import math
from pathlib import Path, PurePosixPath
import re
import struct
import sys
from typing import Any, Mapping


REGISTRY_SCHEMA = "lostark.effect-material-program-registry"
REGISTRY_VERSION = 1
FRAGMENT_SCHEMA = "lostark.effect-material-program-registry-fragment"
FRAGMENT_VERSION = 1
DIRECT_PAYLOAD_KIND = "DIRECT_AUTHORED_DOCUMENT_V13"
AUTHORING_SCHEMA = "lostark.effect-authoring"
AUTHORING_VERSION = 13
CANONICAL_SPRITE_ADAPTER_ID = (
    "effect.adapter.sprite-particle.scene-color-rt0."
    "zero-distortion-rt1.alpha-two-sided.v1"
)
MESH_ALPHA_TWO_SIDED_ADAPTER_ID = (
    "effect.adapter.mesh-particle.cmodel.scene-color-rt0."
    "zero-distortion-rt1.alpha-two-sided.v1"
)
DECAL_ALPHA_ONE_SIDED_ADAPTER_ID = (
    "effect.adapter.local-decal.projector.scene-color-rt0."
    "zero-distortion-rt1.alpha-one-sided.v1"
)
SPRITE_ALPHA_ONE_SIDED_ADAPTER_ID = (
    "effect.adapter.sprite-particle.scene-color-rt0."
    "zero-distortion-rt1.alpha-one-sided.v1"
)
SPRITE_ADDITIVE_TWO_SIDED_ADAPTER_ID = (
    "effect.adapter.sprite-particle.scene-color-rt0."
    "zero-distortion-rt1.additive-two-sided.v1"
)
SPRITE_ADDITIVE_ONE_SIDED_ADAPTER_ID = (
    "effect.adapter.sprite-particle.scene-color-rt0."
    "zero-distortion-rt1.additive-one-sided.v1"
)
MESH_ALPHA_ONE_SIDED_ADAPTER_ID = (
    "effect.adapter.mesh-particle.cmodel.scene-color-rt0."
    "zero-distortion-rt1.alpha-one-sided.v1"
)
DECAL_ALPHA_TWO_SIDED_ADAPTER_ID = (
    "effect.adapter.local-decal.projector.scene-color-rt0."
    "zero-distortion-rt1.alpha-two-sided.v1"
)

INLINE_MIRROR_REQUIRED = "INLINE_MIRROR_REQUIRED"

COMPILED_ADAPTERS: dict[str, dict[str, Any]] = {
    CANONICAL_SPRITE_ADAPTER_ID: {
        "carrier": "SPRITE_PARTICLE",
        "renderProfile": "alpha_two_sided_depth_read",
        "passIndex": 1,
        "renderState": ("RS_Cull_None", "DSS_ReadOnly", "BS_EffectAlpha"),
        "programs": frozenset(
            (("runtimeMaterialV2", 6), ("standardColorV1", 1))
        ),
    },
    MESH_ALPHA_TWO_SIDED_ADAPTER_ID: {
        "carrier": "MESH_PARTICLE",
        "renderProfile": "alpha_two_sided_depth_read",
        "passIndex": 1,
        "renderState": ("RS_Cull_None", "DSS_ReadOnly", "BS_EffectAlpha"),
        "programs": frozenset(
            (("runtimeMaterialV2", 3), ("standardColorV1", 1))
        ),
    },
    DECAL_ALPHA_ONE_SIDED_ADAPTER_ID: {
        "carrier": "LOCAL_DECAL",
        "renderProfile": "alpha_one_sided_depth_read",
        "passIndex": 3,
        "renderState": ("RS_Default", "DSS_ReadOnly", "BS_EffectAlpha"),
        "programs": frozenset(
            (("localDecal", 14), ("standardColorV1", 1))
        ),
    },
    SPRITE_ALPHA_ONE_SIDED_ADAPTER_ID: {
        "carrier": "SPRITE_PARTICLE",
        "renderProfile": "alpha_one_sided_depth_read",
        "passIndex": 3,
        "renderState": ("RS_Default", "DSS_ReadOnly", "BS_EffectAlpha"),
        "programs": frozenset((("standardColorV1", 1),)),
    },
    SPRITE_ADDITIVE_TWO_SIDED_ADAPTER_ID: {
        "carrier": "SPRITE_PARTICLE",
        "renderProfile": "additive_two_sided_depth_read",
        "passIndex": 2,
        "renderState": (
            "RS_Cull_None",
            "DSS_ReadOnly",
            "BS_EffectAdditive",
        ),
        "programs": frozenset((("standardColorV1", 1),)),
    },
    SPRITE_ADDITIVE_ONE_SIDED_ADAPTER_ID: {
        "carrier": "SPRITE_PARTICLE",
        "renderProfile": "additive_one_sided_depth_read",
        "passIndex": 4,
        "renderState": ("RS_Default", "DSS_ReadOnly", "BS_EffectAdditive"),
        "programs": frozenset((("standardColorV1", 1),)),
    },
    MESH_ALPHA_ONE_SIDED_ADAPTER_ID: {
        "carrier": "MESH_PARTICLE",
        "renderProfile": "alpha_one_sided_depth_read",
        "passIndex": 3,
        "renderState": ("RS_Default", "DSS_ReadOnly", "BS_EffectAlpha"),
        "programs": frozenset((("standardColorV1", 1),)),
    },
    DECAL_ALPHA_TWO_SIDED_ADAPTER_ID: {
        "carrier": "LOCAL_DECAL",
        "renderProfile": "alpha_two_sided_depth_read",
        "passIndex": 1,
        "renderState": (
            "RS_Cull_None",
            "DSS_ZNone",
            "BS_EffectAlpha",
        ),
        "programs": frozenset((("standardColorV1", 1),)),
    },
}

ROOT_KEYS = (
    "schema",
    "formatVersion",
    "programs",
    "layouts",
    "descriptors",
    "adapters",
    "bindings",
)
FRAGMENT_ROOT_KEYS = (
    "schema",
    "formatVersion",
    "domainId",
    "programs",
    "layouts",
    "descriptors",
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
    "inlineMirrorPolicy",
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
STANDARD_COLOR_EXECUTION_KEYS = (
    *EXECUTION_KEYS[:9],
    "standardColor",
    *EXECUTION_KEYS[9:],
)
STANDARD_COLOR_KEYS = (
    "packetVersion",
    "baseRadianceLaneId",
    "baseRadianceChannel",
    "coverageLaneId",
    "coverageChannel",
    "emissiveMode",
    "lifetimeEnvelope",
    "dissolveMode",
    "dissolveLaneId",
    "dissolveChannel",
    "dissolveSoftness",
    "missingLanePolicy",
)


def _packed_abi_rows(prefix: str, count: int) -> list[dict[str, Any]]:
    return [
        {"name": f"{prefix}.{index}", "packedIndex": index}
        for index in range(count)
    ]


def _texture_abi_rows(
    roles: tuple[str, ...],
    channels: tuple[str, ...],
    color_spaces: tuple[str, ...],
) -> list[dict[str, Any]]:
    return [
        {
            "laneId": f"lane.{index}",
            "role": role,
            "textureRegister": index,
            "samplerRegister": 5 + index,
            "sourceChannel": channels[index],
            "colorSpace": color_spaces[index],
        }
        for index, role in enumerate(roles)
    ]


def _compiled_layout_abi(
    *,
    roles: tuple[str, ...],
    channels: tuple[str, ...],
    color_spaces: tuple[str, ...],
    dynamic: tuple[int, int],
    particle: tuple[int, int, int],
    scalar_count: int,
    vector_count: int,
    input_count: int,
    input_masks: tuple[tuple[int, int], tuple[int, int]],
    vector_masks: tuple[tuple[int, int, int], tuple[int, int, int]],
    static: tuple[int, int, int, int],
    render: tuple[int, int, int],
) -> dict[str, Any]:
    lane_count = len(roles)
    return {
        "executionVersion": 1,
        "textureLaneCount": lane_count,
        "textureMask": (1 << lane_count) - 1 if lane_count else 0,
        "textureLanes": _texture_abi_rows(roles, channels, color_spaces),
        "dynamicConsumedMask": dynamic[0],
        "dynamicSuppressedMask": dynamic[1],
        "particleColorPolicy": particle[0],
        "particleColorConsumedMask": particle[1],
        "particleColorSuppressedMask": particle[2],
        "scalarCount": scalar_count,
        "vectorCount": vector_count,
        "inputCount": input_count,
        "inputConsumedMask": list(input_masks[0]),
        "inputSuppressedMask": list(input_masks[1]),
        "vectorComponentConsumedMask": list(vector_masks[0]),
        "vectorComponentSuppressedMask": list(vector_masks[1]),
        "staticInputCount": static[0],
        "staticSelectedMask": static[1],
        "staticConsumedMask": static[2],
        "staticSuppressedMask": static[3],
        "renderInputCount": render[0],
        "renderConsumedMask": render[1],
        "renderSuppressedMask": render[2],
        "scalarRows": _packed_abi_rows("scalar", scalar_count),
        "vectorRows": _packed_abi_rows("vector", vector_count),
        "artistParameterRows": [],
        "colorRows": [],
    }


COMPILED_PROGRAM_LAYOUT_ABIS: dict[tuple[str, int], dict[str, Any]] = {
    ("runtimeMaterialV2", 6): _compiled_layout_abi(
        roles=("sparkle_tex", "edgedeco.texture01"),
        channels=("", ""),
        color_spaces=("srgb", "srgb"),
        dynamic=(0x0F, 0),
        particle=(3, 0x0F, 0),
        scalar_count=3,
        vector_count=0,
        input_count=6,
        input_masks=((0x37, 0), (0x08, 0)),
        vector_masks=((0, 0, 0), (0, 0, 0)),
        static=(0, 0, 0, 0),
        render=(6, 0x2F, 0x10),
    ),
    ("runtimeMaterialV2", 3): _compiled_layout_abi(
        roles=("maintex", "uv_noise_tex"),
        channels=("", ""),
        color_spaces=("srgb", "srgb"),
        dynamic=(0x0F, 0),
        particle=(2, 0x0F, 0),
        scalar_count=29,
        vector_count=1,
        input_count=32,
        input_masks=((0xCFFFFFF7, 0), (0x30000008, 0)),
        vector_masks=((0, 0, 0), (0x0F, 0, 0)),
        static=(14, 0x33FF, 0x3FFF, 0),
        render=(6, 0x2F, 0x10),
    ),
    ("localDecal", 14): _compiled_layout_abi(
        roles=("height", "diffuse", "dissolve", "normal", "specular", "emissive"),
        channels=("B", "RGBA", "G", "RG", "RGB", "R"),
        color_spaces=("linear", "srgb", "linear", "linear", "srgb", "srgb"),
        dynamic=(0, 0x0F),
        particle=(0, 0, 0),
        scalar_count=22,
        vector_count=3,
        input_count=33,
        input_masks=((0x820EC1FF, 0x1), (0x7DF13E00, 0)),
        vector_masks=((0x0F, 0x0F, 0), (0, 0, 0x0F)),
        static=(18, 0x3FFFB, 0x3FFFF, 0),
        render=(6, 0x03, 0x3C),
    ),
}
COMPILED_PROGRAM_IDS: dict[tuple[str, int], str] = {
    ("runtimeMaterialV2", 6): "effect.program.runtime-material-v2.opcode-6.v1",
    ("runtimeMaterialV2", 3): "effect.program.runtime-material-v2.opcode-3.v1",
    ("localDecal", 14): "effect.program.local-decal.opcode-14.v1",
    ("standardColorV1", 1): "effect.program.standard-color-v1.opcode-1.v1",
}
COMPILED_LAYOUT_IDS: dict[tuple[str, int], str] = {
    ("runtimeMaterialV2", 6):
        "effect.layout.runtime-material-v2.opcode-6.abi-3aafae1b4639c551.v1",
    ("runtimeMaterialV2", 3):
        "effect.layout.runtime-material-v2.opcode-3.abi-85c02e5f1f646d22.v1",
    ("localDecal", 14):
        "effect.layout.local-decal.opcode-14.abi-c6b52a791b98f0c5.v1",
}
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
        or len(value) > 512
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
        backend = row["backend"]
        if backend not in (
            "runtimeMaterialV2",
            "localDecal",
            "standardColorV1",
        ):
            raise ContractError(f"program {row_id} backend is unsupported")
        opcode = _require_uint32(row["opcode"], f"program {row_id} opcode")
        if (
            (backend, opcode) not in COMPILED_PROGRAM_LAYOUT_ABIS
            and (backend, opcode) != ("standardColorV1", 1)
        ):
            raise ContractError(
                f"program {row_id} has no compiled Program/Layout ABI receipt"
            )
        expected_id = COMPILED_PROGRAM_IDS[(backend, opcode)]
        if row_id != expected_id:
            raise ContractError(
                f"program {row_id} must use canonical compiled ID {expected_id}"
            )
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
            _require_text(lane["sourceChannel"], f"{label}.sourceChannel", 32)
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
        _validate_mask_count(
            row["dynamicConsumedMask"] | row["dynamicSuppressedMask"],
            4,
            f"layout {row_id} dynamic",
        )
        _validate_mask_count(
            row["particleColorConsumedMask"] | row["particleColorSuppressedMask"],
            4,
            f"layout {row_id} particleColor",
        )
        if row["particleColorPolicy"] > 3:
            raise ContractError(f"layout {row_id} particleColorPolicy is unsupported")
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
        standard_color_prefix = "effect.layout.standard-color-v1."
        if row_id.startswith(standard_color_prefix):
            _validate_compiled_program_layout_abi(
                {"backend": "standardColorV1", "opcode": 1}, row, row_id
            )
            signature = [
                {
                    "role": lane["role"],
                    "sourceChannel": lane["sourceChannel"],
                    "colorSpace": lane["colorSpace"],
                }
                for lane in lanes
            ]
            digest = hashlib.sha256(
                json.dumps(
                    signature,
                    ensure_ascii=False,
                    sort_keys=True,
                    separators=(",", ":"),
                ).encode("utf-8")
            ).hexdigest()[:16]
            expected_id = (
                f"{standard_color_prefix}{lane_count}-lane.{digest}.v1"
            )
        else:
            matching_receipts = [
                identity
                for identity, receipt in COMPILED_PROGRAM_LAYOUT_ABIS.items()
                if all(row.get(field) == expected for field, expected in receipt.items())
            ]
            if len(matching_receipts) != 1:
                raise ContractError(
                    f"layout {row_id} does not match exactly one compiled ABI receipt"
                )
            expected_id = COMPILED_LAYOUT_IDS[matching_receipts[0]]
        if row_id != expected_id:
            raise ContractError(
                f"layout {row_id} must use canonical compiled ABI ID {expected_id}"
            )
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
        if adapter_id not in COMPILED_ADAPTERS:
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
    adapter = COMPILED_ADAPTERS[binding["adapterId"]]
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

    materialized: dict[str, Any] = {
        "enabled": True,
        "version": layout["executionVersion"],
        "backend": program["backend"],
        "opcode": program["opcode"],
        "passIndex": adapter["passIndex"],
        "renderState": {
            "rasterizer": adapter["renderState"][0],
            "depthStencil": adapter["renderState"][1],
            "blend": adapter["renderState"][2],
            "stencilReference": 0,
        },
        "textureLaneCount": layout["textureLaneCount"],
        "textureMask": layout["textureMask"],
        "textureLanes": texture_lanes,
    }
    if (program["backend"], program["opcode"]) == ("standardColorV1", 1):
        has_dissolve = layout["textureLaneCount"] == 3
        materialized["standardColor"] = {
            "packetVersion": 1,
            "baseRadianceLaneId": layout["textureLanes"][0]["laneId"],
            "baseRadianceChannel": layout["textureLanes"][0]["sourceChannel"],
            "coverageLaneId": layout["textureLanes"][1]["laneId"],
            "coverageChannel": layout["textureLanes"][1]["sourceChannel"],
            "emissiveMode": "baseRadiance",
            "lifetimeEnvelope": "carrierAlpha",
            "dissolveMode": "laneThreshold" if has_dissolve else "none",
            "dissolveLaneId": (
                layout["textureLanes"][2]["laneId"] if has_dissolve else ""
            ),
            "dissolveChannel": (
                layout["textureLanes"][2]["sourceChannel"]
                if has_dissolve
                else "invalid"
            ),
            "dissolveSoftness": 0.1 if has_dissolve else 0.0,
            "missingLanePolicy": "failClosed",
        }
    materialized.update({
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
    })
    return materialized


def _require_same(actual: Any, expected: Any, label: str) -> None:
    if type(actual) is not type(expected) or actual != expected:
        raise ContractError(f"materialized execution mismatch at {label}")


def _validate_compiled_program_layout_abi(
    program: Mapping[str, Any], layout: Mapping[str, Any], label: str
) -> None:
    identity = (program["backend"], program["opcode"])
    if identity == ("standardColorV1", 1):
        lane_count = layout.get("textureLaneCount")
        lanes = layout.get("textureLanes")
        expected_roles = (
            ("base_radiance", "coverage")
            if lane_count == 2
            else ("base_radiance", "coverage", "dissolve")
        )
        base_channels = {"R", "G", "B", "RGB"}
        scalar_channels = {"R", "G", "B", "A"}
        zero_fields = (
            "dynamicConsumedMask",
            "dynamicSuppressedMask",
            "particleColorPolicy",
            "particleColorConsumedMask",
            "particleColorSuppressedMask",
            "scalarCount",
            "vectorCount",
            "inputCount",
            "staticInputCount",
            "staticSelectedMask",
            "staticConsumedMask",
            "staticSuppressedMask",
            "renderInputCount",
            "renderConsumedMask",
            "renderSuppressedMask",
        )
        zero_arrays = (
            ("inputConsumedMask", [0, 0]),
            ("inputSuppressedMask", [0, 0]),
            ("vectorComponentConsumedMask", [0, 0, 0]),
            ("vectorComponentSuppressedMask", [0, 0, 0]),
            ("scalarRows", []),
            ("vectorRows", []),
            ("artistParameterRows", []),
            ("colorRows", []),
        )
        valid = (
            layout.get("executionVersion") == 1
            and lane_count in (2, 3)
            and layout.get("textureMask") == (1 << lane_count) - 1
            and isinstance(lanes, list)
            and len(lanes) == lane_count
            and all(layout.get(field) == 0 for field in zero_fields)
            and all(layout.get(field) == expected for field, expected in zero_arrays)
        )
        for index, (lane, role) in enumerate(zip(lanes or (), expected_roles)):
            allowed_channels = base_channels if index == 0 else scalar_channels
            valid = valid and (
                lane.get("laneId") == f"lane.{index}"
                and lane.get("role") == role
                and lane.get("textureRegister") == index
                and lane.get("samplerRegister") == 5 + index
                and lane.get("sourceChannel") in allowed_channels
                and lane.get("colorSpace") in ("linear", "srgb")
                and (
                    index == 0
                    or lane.get("sourceChannel") == "A"
                    or lane.get("colorSpace") == "linear"
                )
            )
        if not valid:
            raise ContractError(
                f"binding {label} Program/Layout ABI mismatch at StandardColorV1"
            )
        return
    expected = COMPILED_PROGRAM_LAYOUT_ABIS.get(identity)
    if expected is None:
        raise ContractError(f"binding {label} has no compiled Program/Layout ABI receipt")
    for field, expected_value in expected.items():
        actual_value = layout.get(field)
        if type(actual_value) is not type(expected_value) or actual_value != expected_value:
            raise ContractError(
                f"binding {label} Program/Layout ABI mismatch at {field}"
            )


def assert_execution_bit_exact(
    materialized: Mapping[str, Any], authored: Any, label: str
) -> None:
    execution_keys = (
        STANDARD_COLOR_EXECUTION_KEYS
        if materialized.get("backend") == "standardColorV1"
        else EXECUTION_KEYS
    )
    _require_exact_order(
        materialized, execution_keys, f"{label} materialized execution"
    )
    _require_exact_order(authored, execution_keys, f"{label} authored execution")
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

    if materialized.get("backend") == "standardColorV1":
        expected_standard = materialized["standardColor"]
        actual_standard = authored["standardColor"]
        _require_exact_order(
            expected_standard, STANDARD_COLOR_KEYS, f"{label}.standardColor"
        )
        _require_exact_order(
            actual_standard, STANDARD_COLOR_KEYS, f"{label}.standardColor"
        )
        for field in STANDARD_COLOR_KEYS:
            if field == "dissolveSoftness":
                if _float32_bits(
                    actual_standard[field], f"{label}.standardColor.{field}"
                ) != _float32_bits(
                    expected_standard[field], f"{label}.standardColor.{field}"
                ):
                    raise ContractError(
                        "materialized execution float-bit mismatch at "
                        f"{label}.standardColor.{field}"
                    )
            else:
                _require_same(
                    actual_standard[field],
                    expected_standard[field],
                    f"{label}.standardColor.{field}",
                )

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


def _find_authored_element(
    authored_documents: Mapping[str, dict[str, Any]],
    effect_id: str,
    element_id: str,
) -> dict[str, Any]:
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
    return matching[0]


def _validate_binding_carrier(
    element: Mapping[str, Any],
    adapter: Mapping[str, Any],
    program: Mapping[str, Any],
    label: str,
) -> None:
    kind = element.get("kind")
    source_recipe = element.get("sourceRecipe")
    renderer_shape = (
        source_recipe.get("rendererShape")
        if isinstance(source_recipe, dict) and source_recipe.get("enabled") is True
        else ""
    )
    resources = element.get("resources")
    resource_slots = (
        {row.get("slotId") for row in resources if isinstance(row, dict)}
        if isinstance(resources, list)
        else set()
    )
    carrier = adapter["carrier"]
    material = element.get("material")
    render_profile = material.get("renderProfile") if isinstance(material, dict) else None
    matches = False
    if program.get("backend") == "standardColorV1":
        if carrier == "SPRITE_PARTICLE":
            matches = (
                kind == "particle"
                and renderer_shape == "sprite"
                and isinstance(resources, list)
                and len(resources) == 0
            )
        elif carrier == "MESH_PARTICLE":
            matches = (
                kind == "particle"
                and renderer_shape == "mesh"
                and isinstance(resources, list)
                and len(resources) == 1
                and isinstance(resources[0], dict)
                and set(resources[0]) == {"slotId", "assetId"}
                and resources[0].get("slotId") == "meshModel"
                and isinstance(resources[0].get("assetId"), str)
                and bool(resources[0]["assetId"])
            )
        elif carrier == "LOCAL_DECAL":
            matches = (
                kind == "decal"
                and renderer_shape == "decal"
                and isinstance(resources, list)
                and len(resources) == 0
            )
    elif carrier == "SPRITE_PARTICLE":
        matches = (
            kind == "particle"
            and renderer_shape == "sprite"
            and "meshModel" not in resource_slots
        )
    elif carrier == "MESH_PARTICLE":
        matches = (
            kind == "particle"
            and renderer_shape == "mesh"
            and "meshModel" in resource_slots
        )
    elif carrier == "LOCAL_DECAL":
        matches = kind == "decal" and "meshModel" not in resource_slots
    if not matches or render_profile != adapter["renderProfile"]:
        raise ContractError(f"binding {label} carrier is incompatible with its adapter")


def _find_authored_execution(
    element: Mapping[str, Any],
    effect_id: str,
    element_id: str,
    *,
    require_enabled: bool,
) -> Any | None:
    material = element.get("material")
    execution = material.get("execution") if isinstance(material, dict) else None
    enabled = isinstance(execution, dict) and execution.get("enabled") is True
    if require_enabled and not enabled:
        raise ContractError(
            f"binding target has no enabled inline material execution: "
            f"{effect_id}/{element_id}"
        )
    return execution if enabled else None


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
        mirror_policy = row["inlineMirrorPolicy"]
        if mirror_policy != INLINE_MIRROR_REQUIRED:
            raise ContractError(
                f"binding {effect_id}/{element_id} inlineMirrorPolicy is unsupported"
            )
        adapter = COMPILED_ADAPTERS[row["adapterId"]]
        program = programs[row["programId"]]
        if (
            (program["backend"], program["opcode"]) not in adapter["programs"]
        ):
            raise ContractError(f"binding {effect_id}/{element_id} program is unsupported by its adapter")
        _validate_compiled_program_layout_abi(
            program,
            layouts[row["layoutId"]],
            f"{effect_id}/{element_id}",
        )
        if authored_documents is not None:
            materialized = materialize_binding(row, programs, layouts, descriptors)
            element = _find_authored_element(
                authored_documents, effect_id, element_id
            )
            _validate_binding_carrier(
                element, adapter, program, f"{effect_id}/{element_id}"
            )
            authored = _find_authored_execution(
                element, effect_id, element_id,
                require_enabled=True,
            )
            if authored is not None:
                assert_execution_bit_exact(
                    materialized, authored, f"{effect_id}/{element_id}"
                )
    if bindings and authored_documents is None:
        raise ContractError("binding target validation requires authored documents")
    return value


def merge_registry_fragments(
    base: Any,
    fragment_root: Path,
) -> dict[str, Any]:
    """Merge domain rows into the compiled-adapter base in stable filename order.

    Adapter declarations remain integration-owned in the base document.  A domain
    fragment can only contribute Program/Layout/Descriptor/Binding rows, so two
    feature branches never need to edit the same monolithic authoring document.
    Duplicate IDs and cross-fragment dangling references are rejected later by the
    ordinary full-registry validator.
    """

    _require_exact_order(base, ROOT_KEYS, "material-program registry base root")
    if base["schema"] != REGISTRY_SCHEMA:
        raise ContractError("material-program registry base schema mismatch")
    if type(base["formatVersion"]) is not int or base["formatVersion"] != REGISTRY_VERSION:
        raise ContractError("material-program registry base formatVersion mismatch")

    for key, maximum in (
        ("programs", MAX_PROGRAM_ROWS),
        ("layouts", MAX_LAYOUT_ROWS),
        ("descriptors", MAX_DESCRIPTOR_ROWS),
        ("bindings", MAX_BINDING_ROWS),
    ):
        rows = _require_array(base[key], f"base.{key}", 0, maximum)
        if rows:
            raise ContractError(
                f"material-program registry base.{key} must be empty; "
                "domain rows belong to Fragments"
            )
    base_adapters = list(
        _require_array(base["adapters"], "base.adapters", 1, MAX_ADAPTER_ROWS)
    )
    expected_adapters = [
        {"adapterId": adapter_id} for adapter_id in COMPILED_ADAPTERS
    ]
    if base_adapters != expected_adapters:
        raise ContractError(
            "material-program registry base.adapters must exactly match the "
            "compiled adapter table and order"
        )

    merged: dict[str, Any] = {
        "schema": base["schema"],
        "formatVersion": base["formatVersion"],
        "programs": [],
        "layouts": [],
        "descriptors": [],
        "adapters": base_adapters,
        "bindings": [],
    }

    if not fragment_root.exists():
        return merged
    if not fragment_root.is_dir() or fragment_root.is_symlink():
        raise ContractError("material-program fragment root is not a regular directory")

    fragment_paths = sorted(fragment_root.glob("*.json"), key=lambda path: path.name)
    domain_ids: set[str] = set()
    for path in fragment_paths:
        if path.is_symlink() or not path.is_file():
            raise ContractError(f"material-program fragment is not a regular file: {path.name}")
        if not path.name.endswith(".material-program-fragment.v1.json"):
            raise ContractError(f"material-program fragment filename is unsupported: {path.name}")
        fragment = load_json(path, f"material-program fragment {path.name}")
        _require_exact_order(fragment, FRAGMENT_ROOT_KEYS, f"fragment {path.name}")
        if fragment["schema"] != FRAGMENT_SCHEMA:
            raise ContractError(f"material-program fragment schema mismatch: {path.name}")
        if (
            type(fragment["formatVersion"]) is not int
            or fragment["formatVersion"] != FRAGMENT_VERSION
        ):
            raise ContractError(
                f"material-program fragment formatVersion mismatch: {path.name}"
            )
        domain_id = _require_stable_id(
            fragment["domainId"], f"fragment {path.name}.domainId"
        )
        if domain_id in domain_ids:
            raise ContractError(f"duplicate material-program fragment domainId: {domain_id}")
        domain_ids.add(domain_id)
        contributed = 0
        for key, maximum in (
            ("programs", MAX_PROGRAM_ROWS),
            ("layouts", MAX_LAYOUT_ROWS),
            ("descriptors", MAX_DESCRIPTOR_ROWS),
            ("bindings", MAX_BINDING_ROWS),
        ):
            rows = _require_array(
                fragment[key], f"fragment {path.name}.{key}", 0, maximum
            )
            merged[key].extend(rows)
            contributed += len(rows)
        if contributed == 0:
            raise ContractError(f"material-program fragment is empty: {path.name}")
    return merged


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
    fragment_root: Path | None = None,
) -> dict[str, Any]:
    base = load_json(source_path)
    registry = merge_registry_fragments(
        base,
        fragment_root if fragment_root is not None else source_path.parent / "Fragments",
    )
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
    parser.add_argument("--fragment-root", type=Path)
    return parser


def main(argv: list[str] | None = None) -> int:
    args = make_parser().parse_args(argv)
    try:
        registry = build_registry(
            args.source.resolve(),
            args.effect_catalog.resolve() if args.effect_catalog else None,
            args.data_root.resolve() if args.data_root else None,
            args.fragment_root.resolve() if args.fragment_root else None,
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
