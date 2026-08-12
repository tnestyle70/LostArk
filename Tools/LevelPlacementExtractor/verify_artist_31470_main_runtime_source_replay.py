#!/usr/bin/env python3
"""Replay the shipped Artist F main material translations on D3D11 WARP.

The verifier has two independent numeric layers:

* nineteen constant-texture mutations are joined to the sealed original-DXBC
  replay receipt, with only the documented unresolved c0.x opacity carrier
  changed from 0.8 to the runtime-neutral value 1.0;
* fifteen non-uniform 4x4 POINT_CLAMP cases run the raw original pixel shader
  and the compiled shipped runtime pixel shader side by side.  This closes the
  recovered UV/channel arithmetic without claiming the source sampler policy.

The runtime shader is compiled from the exact shipped PS_MAIN entry.  The
original pixel shaders are never recompiled.
"""

from __future__ import annotations

import argparse
import copy
import ctypes
import hashlib
import json
import math
import re
import struct
from pathlib import Path
from typing import Any

from extract_artist_31470_main_ref_shader_cache import (
    read_json,
    validate_receipt as validate_shader_cache_receipt,
)
from replay_artist_31470_main_original_dxbc import (
    DEFAULT_COMPILER,
    FLOAT_TOLERANCE,
    HRESULT,
    UINT,
    SamplerDesc,
    WarpPixelReplay,
    blob_bytes,
    canonical_text_sha256,
    checked,
    com_method,
    external_binary_identity,
    float4_rows,
    load_original_pixel_shaders,
    release,
    require,
    sha256_bytes,
    texture,
    validate_receipt as validate_original_replay_receipt,
)


SCHEMA = "lostark.artist-31470-main-runtime-source-replay-receipt"
FORMAT_VERSION = 2
REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_SHADER = REPO_ROOT / "Client/Bin/ShaderFiles/Shader_VtxEffectMeshPreview.hlsl"
DEFAULT_RUNTIME_INCLUDE = REPO_ROOT / (
    "Client/Bin/ShaderFiles/Shader_Artist31470RuntimeMaterial.hlsli"
)
DEFAULT_ACTIVE011_INCLUDE = REPO_ROOT / (
    "Client/Bin/ShaderFiles/Shader_Artist31470Active011OuterMaterial.hlsli"
)
DEFAULT_ORIGINAL_REPLAY = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.main-original-dxbc-replay.receipt.json"
)
DEFAULT_OUTPUT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.main-runtime-source-replay.receipt.json"
)

EXPECTED_RUNTIME_PIXEL_DXBC_SHA256 = (
    "88183a1dc47dbb815e56d1e6f659b18e6f70b338dd492202aff1c2de4c16c2cb"
)
EXPECTED_RUNTIME_PIXEL_DXBC_BYTE_SIZE = 65_744
EXPECTED_RUNTIME_CARRIER_DXBC_SHA256 = (
    "3177a250856484ee672a9b4f2c7ca8426ca3e11ae915cf77074dbccb44898757"
)
EXPECTED_SPATIAL_NUMERIC_PROJECTION_SHA256 = (
    "e0ac89abefa3dfb16698be6ee9ad972acba43877211a66c648fe42e7c4a0c108"
)
ORIGINAL_EXTERNAL_OPACITY = 0.8
RUNTIME_NEUTRAL_EXTERNAL_OPACITY = 1.0
SPATIAL_TOLERANCE = 2.0e-5

RUNTIME_CARRIER_HLSL = b"""
cbuffer Carrier : register(b0) { float4 k[5]; }
struct O {
 float4 p : SV_Position; float3 n : NORMAL; float2 u : TEXCOORD0;
 float3 w : TEXCOORD1; float3 t : TEXCOORD2; float3 b : TEXCOORD3;
};
O main(uint i : SV_VertexID) {
 float2 q[3] = { float2(-1,-1), float2(-1,3), float2(3,-1) }; O o;
 o.p=float4(q[i],0,1); o.n=k[0].xyz; o.u=k[1].xy; o.w=k[2].xyz;
 o.t=k[3].xyz; o.b=k[4].xyz; return o;
}
"""

CONSTANT_CASE_MAP = [
    ("water/runtime-neutral-c0-baseline", "water/baseline"),
    ("water/dissolve-scale-zero", "water/dissolve-scale-zero"),
    ("water/dissolve-unsaturated", "water/dissolve-unsaturated"),
    ("water/noise-r-zero-unsaturated", "water/noise-r-zero-unsaturated"),
    ("water/noise-gba-negative-control", "water/noise-gba-negative-control"),
    ("water/main-mask-zero", "water/main-mask-zero"),
    ("water/boundary-zero", "water/boundary-zero"),
    ("water/fresnel-zero", "water/fresnel-zero"),
    ("water/dynamic-threshold-1.6", "water/dynamic-threshold-1.6"),
    ("water/renderer-alpha-zero", "water/renderer-alpha-zero"),
    ("sprite/runtime-neutral-c0-baseline", "sprite/baseline"),
    ("sprite/dissolve-multiplier-zero", "sprite/dissolve-multiplier-zero"),
    ("sprite/main-r-zero", "sprite/main-t1-r-zero"),
    ("sprite/dissolve-r-zero", "sprite/dissolve-t3-r-zero"),
    ("sprite/dissolve-r-one", "sprite/dissolve-t3-r-one"),
    ("sprite/radial-floor", "sprite/radial-floor"),
    ("sprite/rgb-exponent-negative-control", "sprite/rgb-exponent-negative-control"),
    ("sprite/dynamic-threshold-1.5", "sprite/dynamic-threshold-1.5"),
    ("sprite/renderer-alpha-zero", "sprite/renderer-alpha-zero"),
]

SPATIAL_CASES = [
    ("water/spatial-base", "#9/#10_WATERTRAIL", "base", "BASELINE"),
    ("water/spatial-time", "#9/#10_WATERTRAIL", "time", "INVARIANT_CONTROL"),
    ("water/spatial-dynamic-x", "#9/#10_WATERTRAIL", "dx", "SENSITIVE"),
    ("water/spatial-dynamic-y", "#9/#10_WATERTRAIL", "dy", "FIXTURE_BIN_CONTROL"),
    ("water/spatial-dynamic-z", "#9/#10_WATERTRAIL", "dz", "SENSITIVE"),
    ("water/spatial-dynamic-w", "#9/#10_WATERTRAIL", "dw", "FIXTURE_BIN_CONTROL"),
    ("water/spatial-uv", "#9/#10_WATERTRAIL", "uv", "SENSITIVE"),
    ("water/spatial-view", "#9/#10_WATERTRAIL", "view", "NORMALIZED_VIEW_CONTROL"),
    ("sprite/spatial-base", "#11_SPRITEWAVE", "base", "BASELINE"),
    ("sprite/spatial-time", "#11_SPRITEWAVE", "time", "INVARIANT_CONTROL"),
    ("sprite/spatial-dynamic-x", "#11_SPRITEWAVE", "dx", "SENSITIVE"),
    ("sprite/spatial-dynamic-y", "#11_SPRITEWAVE", "dy", "SENSITIVE"),
    ("sprite/spatial-dynamic-z", "#11_SPRITEWAVE", "dz", "SENSITIVE"),
    ("sprite/spatial-dynamic-w-dead", "#11_SPRITEWAVE", "dw", "INVARIANT_CONTROL"),
    ("sprite/spatial-uv", "#11_SPRITEWAVE", "uv", "SENSITIVE"),
]


def seal(receipt: dict[str, Any]) -> None:
    receipt.pop("receiptSha256", None)
    raw = json.dumps(
        receipt,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")
    receipt["receiptSha256"] = hashlib.sha256(raw).hexdigest()


def spatial_numeric_projection_sha256(cases: list[dict[str, Any]]) -> str:
    keys = (
        "caseId",
        "family",
        "expectedRelation",
        "originalShaderIdHex",
        "originalDxbcSha256",
        "originalRawDxbcRgba",
        "actualRuntimeRgba",
        "maximumAbsoluteError",
        "originalVersusFamilyBaselineMaximumDelta",
        "comparisonAdmission",
    )
    projection = [{key: row[key] for key in keys} for row in cases]
    raw = json.dumps(
        projection,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")
    return hashlib.sha256(raw).hexdigest()


def compile_memory(
    source: bytes, source_name: bytes, entry: bytes, profile: bytes, compiler: Path
) -> bytes:
    external_binary_identity(compiler)
    dll = ctypes.WinDLL(str(compiler))
    function = dll.D3DCompile
    function.restype = HRESULT
    function.argtypes = [
        ctypes.c_void_p,
        ctypes.c_size_t,
        ctypes.c_char_p,
        ctypes.c_void_p,
        ctypes.c_void_p,
        ctypes.c_char_p,
        ctypes.c_char_p,
        UINT,
        UINT,
        ctypes.POINTER(ctypes.c_void_p),
        ctypes.POINTER(ctypes.c_void_p),
    ]
    backing = ctypes.create_string_buffer(source)
    code = ctypes.c_void_p()
    errors = ctypes.c_void_p()
    try:
        result = function(
            backing,
            len(source),
            source_name,
            None,
            None,
            entry,
            profile,
            0,
            0,
            ctypes.byref(code),
            ctypes.byref(errors),
        )
        error_text = blob_bytes(errors).decode("utf-8", "replace") if errors.value else ""
        require(
            result >= 0 and code.value,
            f"D3DCompile failed 0x{result & 0xFFFFFFFF:08X}: {error_text}",
        )
        return blob_bytes(code)
    finally:
        release(errors)
        release(code)


def compile_file(path: Path, compiler: Path) -> bytes:
    require(path.is_file(), f"runtime shader is missing: {path}")
    external_binary_identity(compiler)
    dll = ctypes.WinDLL(str(compiler))
    function = dll.D3DCompileFromFile
    function.restype = HRESULT
    function.argtypes = [
        ctypes.c_wchar_p,
        ctypes.c_void_p,
        ctypes.c_void_p,
        ctypes.c_char_p,
        ctypes.c_char_p,
        UINT,
        UINT,
        ctypes.POINTER(ctypes.c_void_p),
        ctypes.POINTER(ctypes.c_void_p),
    ]
    code = ctypes.c_void_p()
    errors = ctypes.c_void_p()
    try:
        result = function(
            str(path.resolve()),
            None,
            ctypes.c_void_p(1),  # D3D_COMPILE_STANDARD_FILE_INCLUDE
            b"PS_MAIN",
            b"ps_5_0",
            0,
            0,
            ctypes.byref(code),
            ctypes.byref(errors),
        )
        error_text = blob_bytes(errors).decode("utf-8", "replace") if errors.value else ""
        require(
            result >= 0 and code.value,
            f"runtime D3DCompileFromFile failed 0x{result & 0xFFFFFFFF:08X}: {error_text}",
        )
        return blob_bytes(code)
    finally:
        release(errors)
        release(code)


def disassemble_runtime(bytecode: bytes, compiler: Path) -> dict[str, Any]:
    dll = ctypes.WinDLL(str(compiler))
    function = dll.D3DDisassemble
    function.restype = HRESULT
    function.argtypes = [
        ctypes.c_void_p,
        ctypes.c_size_t,
        UINT,
        ctypes.c_char_p,
        ctypes.POINTER(ctypes.c_void_p),
    ]
    backing = ctypes.create_string_buffer(bytecode)
    output = ctypes.c_void_p()
    try:
        result = function(backing, len(bytecode), 0, None, ctypes.byref(output))
        checked(result, output, "D3DDisassemble(runtime PS)")
        normalized = blob_bytes(output).decode("utf-8", "strict").rstrip("\x00")
        normalized = normalized.replace("\r\n", "\n").replace("\r", "\n")
    finally:
        release(output)

    required_fragments = [
        "dcl_constantbuffer CB0[110], immediateIndexed",
        "uint g_RuntimeMaterialV2Enabled;   // Offset: 1072 Size:     4",
        "float4 g_RuntimeMaterialV2ScalarBlocks[13];// Offset: 1168 Size:   208",
        "float4 g_RuntimeMaterialV2Vectors[3];// Offset: 1376 Size:    48",
        "uint3 g_RuntimeMaterialV2VectorComponentConsumedMask;// Offset: 1424 Size:    12",
        "uint3 g_RuntimeMaterialV2VectorComponentSuppressedMask;// Offset: 1440 Size:    12",
        "float4 g_CameraPosition;           // Offset: 1712 Size:    16",
        "float4 g_EffectDynamicParameter;   // Offset: 1744 Size:    16",
    ]
    for fragment in required_fragments:
        require(fragment in normalized, f"runtime compiled ABI fragment changed: {fragment}")
    for index in range(4):
        require(
            re.search(
                rf"// g_SourceTexture{index}\s+texture\s+float4\s+2d\s+t{5 + index}\s+1",
                normalized,
            )
            is not None,
            f"runtime SourceTexture{index} RDEF binding changed",
        )
        require(
            re.search(
                rf"// g_RuntimeMaterialV2Sampler{index}\s+sampler\s+NA\s+NA\s+s{5 + index}\s+1",
                normalized,
            )
            is not None,
            f"runtime sampler{index} RDEF binding changed",
        )
        require(f"dcl_sampler s{5 + index}, mode_default" in normalized, "runtime sampler declaration changed")
        require(
            f"dcl_resource_texture2d (float,float,float,float) t{5 + index}" in normalized,
            "runtime texture declaration changed",
        )
    require("// $Globals                          cbuffer" in normalized and "cb0" in normalized, "runtime $Globals binding changed")
    return {
        "normalizedDisassemblySha256": sha256_bytes(normalized.encode("utf-8")),
        "constantBufferRegister": "cb0",
        "constantBufferByteSize": 1760,
        "constantBufferFloat4Count": 110,
        "runtimeHeaderByteOffset": 1072,
        "runtimeScalarBlocksByteOffset": 1168,
        "runtimeVectorsByteOffset": 1376,
        "runtimeVectorConsumedMaskByteOffset": 1424,
        "runtimeVectorSuppressedMaskByteOffset": 1440,
        "cameraPositionByteOffset": 1712,
        "dynamicParameterByteOffset": 1744,
        "sourceTextureRegisters": ["t5", "t6", "t7", "t8"],
        "runtimeSamplerRegisters": ["s5", "s6", "s7", "s8"],
        "rdefAdmission": True,
        "declarationAdmission": True,
    }


def uint_as_float(value: int) -> float:
    return struct.unpack("<f", struct.pack("<I", value))[0]


def write_uint(row: list[float], lane: int, value: int) -> None:
    row[lane] = uint_as_float(value)


def packet_header(rows: list[list[float]], *, sprite: bool) -> None:
    values = (
        (
            (1, 8, 4, 15),
            (7, 8, 2, 15),
            (0, 47, 1, 55),
            (0xFFFFFFFF, 0x7F, 0, 0x7FFF80),
            (9, 0x1EF, 0x1FF, 0),
            (6, 0x2F, 0x10, 0),
        )
        if sprite
        else (
            (1, 3, 2, 3),
            (0x0F, 0, 2, 0x0F),
            (0, 29, 1, 32),
            (0xCFFFFFF7, 0, 0x30000008, 0),
            (14, 0x33FF, 0x3FFF, 0),
            (6, 0x2F, 0x10, 0),
        )
    )
    for destination, source in zip(range(67, 73), values):
        for lane, value in enumerate(source):
            write_uint(rows[destination], lane, value)
    write_uint(rows[89], 0, 0x07 if sprite else 0)
    write_uint(rows[90], 0, 0x08 if sprite else 0x0F)


def constant_runtime_fixtures() -> list[dict[str, Any]]:
    carrier = float4_rows(5)
    carrier[0] = [0.0, 0.0, 1.0, 0.0]
    carrier[1] = [0.5, 0.5, 0.0, 0.0]
    carrier[3] = [1.0, 0.0, 0.0, 0.0]
    carrier[4] = [0.0, 1.0, 0.0, 0.0]

    water = float4_rows(110)
    packet_header(water, sprite=False)
    water[2] = [1.0, 1.0, 1.0, 0.75]
    water[74] = [2.0, 0.0, 0.0, 0.0]
    water[75] = [1.0, 1.0, 0.0, 0.0]
    water[79] = [12.0, 1.2, 0.0, 0.0]
    water[80] = [1.1, 0.0, 0.0, 0.0]
    water[107] = [0.0, 0.8, 0.6, 0.0]
    water_textures = [texture(0, 0, 0, 0) for _ in range(9)]
    water_textures[5] = texture(0.9, 0.4, 0.2, 1.0)
    water_textures[6] = texture(0.8, 0.2, 0.3, 0.4)

    sprite = float4_rows(110)
    packet_header(sprite, sprite=True)
    sprite[2] = [1.0, 1.0, 1.0, 0.7]
    scalar_values = [0.0] * 47
    for index, value in {
        3: 1.0,
        4: 0.8,
        6: 2.0,
        7: 1.0,
        13: 1.0,
        14: 1.0,
        15: 0.9,
        22: 1.0,
        23: 1.0,
        31: 0.6,
        32: 0.9,
        33: 0.1,
    }.items():
        scalar_values[index] = value
    for index, value in enumerate(scalar_values):
        sprite[73 + index // 4][index % 4] = value
    sprite[86] = [1.0, 1.0, 1.0, 0.0]
    sprite[109] = [0.0, 1.0, 0.0, 0.0]
    sprite_textures = [texture(0, 0, 0, 0) for _ in range(9)]
    sprite_textures[5] = texture(0.7, 0.1, 0.1, 1.0)
    sprite_textures[6] = texture(0.2, 0.1, 0.1, 1.0)
    sprite_textures[7] = texture(0.4, 0.1, 0.1, 1.0)
    sprite_textures[8] = texture(0.3, 0.1, 0.1, 1.0)

    bases = [
        {
            "caseId": CONSTANT_CASE_MAP[0][0],
            "originalCaseId": CONSTANT_CASE_MAP[0][1],
            "cb0": water,
            "carrier": carrier,
            "textures": water_textures,
        },
        {
            "caseId": CONSTANT_CASE_MAP[10][0],
            "originalCaseId": CONSTANT_CASE_MAP[10][1],
            "cb0": sprite,
            "carrier": carrier,
            "textures": sprite_textures,
        },
    ]

    def mutate(
        base: dict[str, Any],
        case_id: str,
        original_case_id: str,
        *,
        cb: dict[tuple[int, int], float] | None = None,
        carrier_values: dict[tuple[int, int], float] | None = None,
        texture_values: dict[tuple[int, int], float] | None = None,
    ) -> dict[str, Any]:
        item = copy.deepcopy(base)
        item["caseId"] = case_id
        item["originalCaseId"] = original_case_id
        for (row, lane), value in (cb or {}).items():
            item["cb0"][row][lane] = value
        for (row, lane), value in (carrier_values or {}).items():
            item["carrier"][row][lane] = value
        for (slot, lane), value in (texture_values or {}).items():
            item["textures"][slot]["pixels"][0][lane] = value
        return item

    water_base, sprite_base = bases
    return [
        water_base,
        mutate(water_base, *CONSTANT_CASE_MAP[1], cb={(79, 1): 0.0}),
        mutate(water_base, *CONSTANT_CASE_MAP[2], cb={(79, 1): 0.1}),
        mutate(water_base, *CONSTANT_CASE_MAP[3], cb={(79, 1): 0.1}, texture_values={(6, 0): 0.0}),
        mutate(water_base, *CONSTANT_CASE_MAP[4], cb={(79, 1): 0.1}, texture_values={(6, 1): 0.91, (6, 2): 0.73, (6, 3): 0.19}),
        mutate(water_base, *CONSTANT_CASE_MAP[5], texture_values={(5, 0): 0.0}),
        mutate(water_base, *CONSTANT_CASE_MAP[6], carrier_values={(1, 0): 0.0}),
        mutate(water_base, *CONSTANT_CASE_MAP[7], cb={(107, 2): 0.0}),
        mutate(water_base, *CONSTANT_CASE_MAP[8], cb={(109, 2): 1.6}),
        mutate(water_base, *CONSTANT_CASE_MAP[9], cb={(2, 3): 0.0}),
        sprite_base,
        mutate(sprite_base, *CONSTANT_CASE_MAP[11], cb={(74, 0): 0.0}),
        mutate(sprite_base, *CONSTANT_CASE_MAP[12], texture_values={(5, 0): 0.0}),
        mutate(sprite_base, *CONSTANT_CASE_MAP[13], texture_values={(7, 0): 0.0}),
        mutate(sprite_base, *CONSTANT_CASE_MAP[14], texture_values={(7, 0): 1.0}),
        mutate(sprite_base, *CONSTANT_CASE_MAP[15], carrier_values={(1, 0): 0.0}),
        mutate(sprite_base, *CONSTANT_CASE_MAP[16], cb={(74, 2): 7.0}),
        mutate(sprite_base, *CONSTANT_CASE_MAP[17], cb={(109, 1): 1.5}),
        mutate(sprite_base, *CONSTANT_CASE_MAP[18], cb={(2, 3): 0.0}),
    ]


def grid(seed: int) -> dict[str, Any]:
    pixels = []
    for y in range(4):
        for x in range(4):
            pixels.append(
                [
                    0.05 * seed + 0.04 * x + 0.07 * y,
                    0.03 * seed + 0.06 * x + 0.02 * y,
                    0.02 * seed + 0.03 * x + 0.05 * y,
                    0.4 + 0.01 * seed,
                ]
            )
    return {"width": 4, "height": 4, "pixels": pixels}


def runtime_carrier(uv: tuple[float, float]) -> list[list[float]]:
    rows = float4_rows(5)
    rows[0] = [0.0, 0.0, 1.0, 0.0]
    rows[1] = [uv[0], uv[1], 0.0, 0.0]
    rows[3] = [1.0, 0.0, 0.0, 0.0]
    rows[4] = [0.0, 1.0, 0.0, 0.0]
    return rows


def original_carrier(
    uv: tuple[float, float], view: tuple[float, float, float]
) -> list[list[float]]:
    rows = float4_rows(8)
    rows[0] = [0.0, 0.0, 1.0, 0.0]
    rows[1] = [1.0, 0.0, 0.0, 1.0]
    rows[4] = [uv[0], uv[1], 0.0, 0.0]
    rows[5] = [0.0, 0.0, 0.0, 1.0]
    rows[6] = [view[0], view[1], view[2], 0.0]
    return rows


def water_spatial_fixture(
    *,
    current_time: float = 0.25,
    dynamic: tuple[float, float, float, float] = (1.2, 0.35, 0.55, 0.3),
    uv: tuple[float, float] = (0.37, 0.42),
    particle: tuple[float, float, float, float] = (0.8, 0.9, 0.7, 0.75),
    view: tuple[float, float, float] = (0.0, 0.8, 0.6),
) -> dict[str, Any]:
    raw = float4_rows(16)
    raw[0] = [1.0, 0.0, 0.0, 0.0]
    raw[1] = list(particle)
    raw[2] = [0.0, 0.0, 0.0, 1.0]
    raw[3] = list(dynamic)
    raw[4] = [1.0, 0.0, 0.0, 0.0]
    raw[5] = [0.0, 1.0, 0.0, 0.0]
    raw[6] = [1.0, 0.1, 0.1, 0.1]
    raw[7] = [0.0, 0.0, current_time, 0.400000006]
    raw[8] = [1.0, 0.0, 0.0, 0.0]
    raw[9] = [0.0, 0.0, 1.0, 0.0]
    raw[10] = [0.1, 1.0, 0.0, 0.0]
    raw[11] = [1.0, 1.0, 0.0, 1.0]
    raw[12] = [0.0, 1.0, 0.200000003, 2.0]
    raw[13] = [0.01, 1.0, 0.0, 0.0]
    raw[14] = [1.0, 0.1, 0.0, 1.0]
    raw[15] = [2.0, 40.0, 1.20000005, 1.0]

    runtime = float4_rows(110)
    packet_header(runtime, sprite=False)
    runtime[2] = list(particle)
    runtime[60][1] = current_time
    runtime[107] = [view[0], view[1], view[2], 0.0]
    runtime[109] = list(dynamic)
    scalars = [
        1, 1, 0.1, 2, 1.2, 0, 0, 1, 2, 0.01, 1, 0.1, 0, 1, 0,
        0.4, 1, 0, 1, 0, 0, 1, 1, 0, 40, 2, 1, 0.2, 1,
    ]
    for index, value in enumerate(scalars):
        runtime[73 + index // 4][index % 4] = float(value)
    dummy = [grid(9) for _ in range(5)]
    return {
        "rawCb0": raw,
        "rawCarrier": original_carrier(uv, view),
        "rawTextures": [grid(1), grid(2), grid(3)],
        "runtimeCb0": runtime,
        "runtimeCarrier": runtime_carrier(uv),
        "runtimeTextures": dummy + [grid(3), grid(1), grid(9), grid(9)],
    }


def sprite_spatial_fixture(
    *,
    current_time: float = 0.25,
    dynamic: tuple[float, float, float, float] = (0.3, 0.65, 0.4, 0.8),
    uv: tuple[float, float] = (0.37, 0.42),
    particle: tuple[float, float, float, float] = (0.8, 0.9, 0.7, 0.75),
) -> dict[str, Any]:
    raw = float4_rows(24)
    raw[0] = [1.0, 0.0, 0.0, 0.0]
    raw[1] = list(particle)
    raw[2] = [0.0, 0.0, 0.0, 1.0]
    raw[3] = list(dynamic)
    raw[4] = [0.000796274, -0.999999702, 0.0, 0.0]
    raw[5] = [0.999999702, 0.000796274, 0.0, 0.0]
    raw[6] = [0.800000012, 0.0299999993, 0.0299999993, 0.0299999993]
    raw[8] = [-0.999998748, -0.001592548, 0.0, 0.0]
    raw[9] = [0.001592548, -0.999998748, 0.0, 0.0]
    raw[10] = [1.0, 0.0, 0.0, 0.0]
    raw[11] = [0.0, 1.0, 0.0, 0.0]
    raw[12] = [1.0, 1.0, 1.0, 1.0]
    raw[13] = [-0.001592548, -0.999998748, 0.0, current_time]
    raw[14] = [1.5, 1.0, 0.200000003, 0.0]
    raw[15] = [0.000796274, 0.0, 0.0, 0.5]
    raw[16] = [1.0, 0.0, 0.03, 0.8]
    raw[17] = [-1.0, 0.0, 0.0, 4.0]
    raw[18] = [100.0, 0.0, 0.0, 1.0]
    raw[19] = [2.0, 0.0, 0.0, 0.0]
    raw[20] = [0.0, 1.0, 0.0, 1.0]
    raw[21] = [1.0, 0.0, 0.5, 0.0]
    raw[22] = [0.0, 0.0, 1.0, 10.0]
    raw[23] = [5.0, 1.20000005, 0.0, 2000.0]

    runtime = float4_rows(110)
    packet_header(runtime, sprite=True)
    runtime[2] = list(particle)
    runtime[60][1] = current_time
    runtime[86] = [1.0, 1.0, 1.0, 1.0]
    runtime[109] = list(dynamic)
    scalars = [
        0, 0, 1, 2, 1, 0, 10, 0.5, 0, 0, 1, 1, 0, 4, 100, 2000,
        0.200000003, 0, 0, 0, 0, 2, 1.5, 1, -1, 0, 0, 0.5, 1,
        0.800000012, 0.0299999993, 5, 1.20000005, 0, 1, 0.5, 1, 1, 1,
        0.300000012, 2, 1, -0.100000001, 2, 1, 1, 0.100000001,
    ]
    for index, value in enumerate(scalars):
        runtime[73 + index // 4][index % 4] = float(value)
    dummy = [grid(9) for _ in range(5)]
    return {
        "rawCb0": raw,
        "rawCarrier": original_carrier(uv, (0.0, 0.8, 0.6)),
        "rawTextures": [grid(4), grid(5), grid(6), grid(7)],
        "runtimeCb0": runtime,
        "runtimeCarrier": runtime_carrier(uv),
        "runtimeTextures": dummy + [grid(5), grid(4), grid(7), grid(6)],
    }


def spatial_fixtures() -> list[dict[str, Any]]:
    rows = [
        water_spatial_fixture(),
        water_spatial_fixture(current_time=0.61),
        water_spatial_fixture(dynamic=(0.55, 0.35, 0.55, 0.3)),
        water_spatial_fixture(dynamic=(1.2, 0.82, 0.55, 0.3)),
        water_spatial_fixture(dynamic=(1.2, 0.35, 1.25, 0.3)),
        water_spatial_fixture(dynamic=(1.2, 0.35, 0.55, 0.9)),
        water_spatial_fixture(uv=(0.18, 0.73)),
        water_spatial_fixture(view=(0.8, 0.0, 0.6)),
        sprite_spatial_fixture(),
        sprite_spatial_fixture(current_time=0.61),
        sprite_spatial_fixture(dynamic=(0.85, 0.65, 0.4, 0.8)),
        sprite_spatial_fixture(dynamic=(0.3, 1.1, 0.4, 0.8)),
        sprite_spatial_fixture(dynamic=(0.3, 0.65, 1.1, 0.8)),
        sprite_spatial_fixture(dynamic=(0.3, 0.65, 0.4, 0.1)),
        sprite_spatial_fixture(uv=(0.18, 0.73)),
    ]
    require(len(rows) == len(SPATIAL_CASES), "spatial fixture denominator changed")
    for row, (case_id, family, _, relation) in zip(rows, SPATIAL_CASES):
        row["caseId"] = case_id
        row["family"] = family
        row["expectedRelation"] = relation
    return rows


def add_point_clamp_samplers(runner: WarpPixelReplay, required_count: int) -> None:
    while len(runner.samplers) < required_count:
        description = SamplerDesc()
        description.Filter = 0
        description.AddressU = 3
        description.AddressV = 3
        description.AddressW = 3
        description.MaxAnisotropy = 1
        description.ComparisonFunc = 1
        description.MinLOD = -3.402823466e38
        description.MaxLOD = 3.402823466e38
        sampler = ctypes.c_void_p()
        result = com_method(
            runner.device,
            23,
            HRESULT,
            ctypes.POINTER(SamplerDesc),
            ctypes.POINTER(ctypes.c_void_p),
        )(runner.device, ctypes.byref(description), ctypes.byref(sampler))
        checked(result, sampler, "CreateSamplerState(runtime replay)")
        runner.samplers.append(sampler)
        runner.permanent.append(sampler)


def original_prerequisites(path: Path) -> tuple[dict[str, Any], dict[str, Any]]:
    require(path.resolve() == DEFAULT_ORIGINAL_REPLAY.resolve(), "original replay path changed")
    original = read_json(path)
    validate_original_replay_receipt(original)
    input_row = original["inputReceipt"]
    shader_cache_path = REPO_ROOT / input_row["repoRelativePath"]
    shader_cache = read_json(shader_cache_path)
    validate_shader_cache_receipt(shader_cache)
    require(
        shader_cache["receiptSha256"] == input_row["receiptSha256"],
        "original replay ShaderCache prerequisite changed",
    )
    return original, shader_cache


def expected_constant_rgba(original_row: dict[str, Any]) -> list[float]:
    original_rt0 = original_row["mrtFloat4"][0]
    require(
        abs(original_rt0[3] - original_row["actualWarpAlpha"]) <= FLOAT_TOLERANCE,
        "original replay RT0 alpha projection changed",
    )
    return [
        *original_rt0[:3],
        original_row["actualWarpAlpha"]
        / ORIGINAL_EXTERNAL_OPACITY
        * RUNTIME_NEUTRAL_EXTERNAL_OPACITY,
    ]


def build_receipt(shader: Path, compiler: Path, original_replay_path: Path) -> dict[str, Any]:
    original, shader_cache = original_prerequisites(original_replay_path)
    original_rows = original["numericContract"]["cases"]
    original_by_id = {row["caseId"]: row for row in original_rows}
    require(len(original_by_id) == 21, "original replay case denominator changed")

    pixel = compile_file(shader, compiler)
    require(len(pixel) == EXPECTED_RUNTIME_PIXEL_DXBC_BYTE_SIZE, "runtime PS byte size changed")
    require(sha256_bytes(pixel) == EXPECTED_RUNTIME_PIXEL_DXBC_SHA256, "runtime PS DXBC changed")
    compiled_contract = disassemble_runtime(pixel, compiler)
    carrier = compile_memory(
        RUNTIME_CARRIER_HLSL,
        b"artist31470-runtime-carrier",
        b"main",
        b"vs_5_0",
        compiler,
    )
    require(sha256_bytes(carrier) == EXPECTED_RUNTIME_CARRIER_DXBC_SHA256, "runtime carrier DXBC changed")

    runtime_runner = WarpPixelReplay(compiler)
    original_runner = WarpPixelReplay(compiler)
    try:
        replacement = runtime_runner._shader(carrier, 12, "CreateVertexShader(runtime carrier)")
        runtime_runner.permanent.append(replacement)
        runtime_runner.vertex_shader = replacement
        add_point_clamp_samplers(runtime_runner, 9)

        constant_cases = []
        for fixture in constant_runtime_fixtures():
            original_row = original_by_id[fixture["originalCaseId"]]
            expected = expected_constant_rgba(original_row)
            actual = runtime_runner.run(
                pixel,
                fixture["cb0"],
                fixture["carrier"],
                fixture["textures"],
                render_target_count=1,
            )[0]
            maximum_error = max(abs(left - right) for left, right in zip(actual, expected))
            require(maximum_error <= FLOAT_TOLERANCE, f"{fixture['caseId']} mismatch: {actual} != {expected}")
            constant_cases.append(
                {
                    "caseId": fixture["caseId"],
                    "originalCaseId": fixture["originalCaseId"],
                    "originalShaderIdHex": original_row["shaderIdHex"],
                    "originalDxbcSha256": original_row["dxbcSha256"],
                    "originalC0Point8Rgba": original_row["mrtFloat4"][0],
                    "expectedNeutralC0Rgba": expected,
                    "actualRuntimeRgba": actual,
                    "maximumAbsoluteError": maximum_error,
                    "comparisonAdmission": True,
                }
            )

        water_unsaturated = next(row for row in constant_cases if row["caseId"] == "water/dissolve-unsaturated")
        water_negative = next(row for row in constant_cases if row["caseId"] == "water/noise-gba-negative-control")
        require(
            struct.pack("<f", water_unsaturated["actualRuntimeRgba"][3])
            == struct.pack("<f", water_negative["actualRuntimeRgba"][3]),
            "water non-R noise negative control changed alpha",
        )
        sprite_base = next(row for row in constant_cases if row["caseId"] == "sprite/runtime-neutral-c0-baseline")
        sprite_negative = next(row for row in constant_cases if row["caseId"] == "sprite/rgb-exponent-negative-control")
        require(
            struct.pack("<f", sprite_base["actualRuntimeRgba"][3])
            == struct.pack("<f", sprite_negative["actualRuntimeRgba"][3]),
            "sprite RGB-only exponent changed alpha",
        )
        require(
            sprite_base["actualRuntimeRgba"][:3] != sprite_negative["actualRuntimeRgba"][:3],
            "sprite RGB-only exponent negative control did not change RGB",
        )

        original_shaders = load_original_pixel_shaders(shader_cache)
        spatial_cases = []
        family_baseline: dict[str, list[float]] = {}
        for fixture in spatial_fixtures():
            raw = original_runner.run(
                original_shaders[fixture["family"]]["bytecode"],
                fixture["rawCb0"],
                fixture["rawCarrier"],
                fixture["rawTextures"],
                render_target_count=6,
            )[0]
            actual = runtime_runner.run(
                pixel,
                fixture["runtimeCb0"],
                fixture["runtimeCarrier"],
                fixture["runtimeTextures"],
                render_target_count=1,
            )[0]
            maximum_error = max(abs(left - right) for left, right in zip(raw, actual))
            require(maximum_error <= SPATIAL_TOLERANCE, f"{fixture['caseId']} spatial mismatch: {raw} != {actual}")
            if fixture["expectedRelation"] == "BASELINE":
                family_baseline[fixture["family"]] = raw
            spatial_cases.append(
                {
                    "caseId": fixture["caseId"],
                    "family": fixture["family"],
                    "expectedRelation": fixture["expectedRelation"],
                    "originalShaderIdHex": original_shaders[fixture["family"]]["shaderIdHex"],
                    "originalDxbcSha256": original_shaders[fixture["family"]]["dxbcSha256"],
                    "originalRawDxbcRgba": raw,
                    "actualRuntimeRgba": actual,
                    "maximumAbsoluteError": maximum_error,
                    "comparisonAdmission": True,
                }
            )
        for row in spatial_cases:
            baseline = family_baseline[row["family"]]
            delta = max(abs(left - right) for left, right in zip(row["originalRawDxbcRgba"], baseline))
            row["originalVersusFamilyBaselineMaximumDelta"] = delta
            if row["expectedRelation"] == "SENSITIVE":
                require(delta > SPATIAL_TOLERANCE, f"{row['caseId']} did not exercise a sensitive path")
            elif row["expectedRelation"] in ("INVARIANT_CONTROL", "NORMALIZED_VIEW_CONTROL"):
                require(delta <= SPATIAL_TOLERANCE, f"{row['caseId']} invariant control changed")
        spatial_projection_sha256 = spatial_numeric_projection_sha256(spatial_cases)
        require(
            spatial_projection_sha256 == EXPECTED_SPATIAL_NUMERIC_PROJECTION_SHA256,
            "spatial numeric projection changed",
        )

        receipt = {
            "schema": SCHEMA,
            "formatVersion": FORMAT_VERSION,
            "runtimePixelShader": {
                "sourcePath": shader.relative_to(REPO_ROOT).as_posix(),
                "normalizedSourceSha256": canonical_text_sha256(shader),
                "compiledDxbcSha256": sha256_bytes(pixel),
                "compiledByteSize": len(pixel),
                "entry": "PS_MAIN",
                "profile": "ps_5_0",
                "compiledBindingContract": compiled_contract,
            },
            "includedSourceReplayShaders": {
                "runtimeMaterialPath": DEFAULT_RUNTIME_INCLUDE.relative_to(REPO_ROOT).as_posix(),
                "runtimeMaterialSha256": canonical_text_sha256(DEFAULT_RUNTIME_INCLUDE),
                "active011Path": DEFAULT_ACTIVE011_INCLUDE.relative_to(REPO_ROOT).as_posix(),
                "active011Sha256": canonical_text_sha256(DEFAULT_ACTIVE011_INCLUDE),
            },
            "backend": {
                "name": "D3D11_WARP_ORIGINAL_DXBC_VERSUS_COMPILED_SHIPPED_RUNTIME_HLSL",
                "featureLevel": f"0x{runtime_runner.feature_level:04x}",
                "compiler": external_binary_identity(compiler),
                "carrierSourceSha256": sha256_bytes(RUNTIME_CARRIER_HLSL),
                "carrierDxbcSha256": sha256_bytes(carrier),
            },
            "originalDxbcReplayPrerequisite": {
                "path": original_replay_path.relative_to(REPO_ROOT).as_posix(),
                "receiptSha256": original["receiptSha256"],
                "caseCount": len(original_rows),
                "shaderCacheReceiptPath": original["inputReceipt"]["repoRelativePath"],
                "shaderCacheReceiptSha256": shader_cache["receiptSha256"],
                "fixedInputOriginalDxbcReplayAdmission": True,
                "parameterRegisterTextureChannelClosureAdmission": True,
            },
            "boundary": {
                "originalExternalOpacityC0Recovered": False,
                "originalConstantFixtureExternalOpacity": ORIGINAL_EXTERNAL_OPACITY,
                "runtimeNeutralExternalOpacity": RUNTIME_NEUTRAL_EXTERNAL_OPACITY,
                "passFogAdmission": "IDENTITY_NEUTRAL_RUNTIME_PATH",
                "selectionColorAdmission": "BLACK_NEUTRAL_FIXED_FIXTURE",
                "particleColorCarrierAdmission": "DXBC_SSA_RENDERER_ABI_CORRELATION",
                "dynamicParameterAdmission": "EXACT_UNIFORM_VECTOR_EXPRESSION_C3",
                "pointClampSamplerPolicy": "COMMON_NUMERIC_FIXTURE_ONLY",
                "sourceExactSamplerPolicyAdmission": False,
                "occurrenceSelectedLocalVfRuntimeAdmission": False,
            },
            "constantTextureMutationContract": {
                "caseCount": len(constant_cases),
                "caseIds": [row["caseId"] for row in constant_cases],
                "cases": constant_cases,
                "waterNoiseGbaAlphaBitStable": True,
                "spriteRgbExponentAlphaBitStable": True,
            },
            "nonuniformPointClampTranslationContract": {
                "textureExtent": [4, 4],
                "textureGenerator": "RGBA_COORDINATE_CODED_GRID_V1",
                "caseCount": len(spatial_cases),
                "caseIds": [row["caseId"] for row in spatial_cases],
                "cases": spatial_cases,
                "numericProjectionSha256": spatial_projection_sha256,
                "maximumAbsoluteError": max(row["maximumAbsoluteError"] for row in spatial_cases),
                "fixedFixtureTranslationParityAdmission": True,
                "sourceExactSamplerPolicyAdmission": False,
            },
            "decision": {
                "shippedRuntimeHlslCompiled": True,
                "compiledRuntimeRegisterContractAdmission": True,
                "constantTextureMutationReplayAdmission": True,
                "nonuniformUvTranslationReplayAdmission": True,
                "candidateRuntimeEvaluatorImplementationAdmission": True,
                "occurrenceSelectedLocalVfRuntimeAdmission": False,
                "occurrenceRuntimeEvaluatorMutationAdmission": False,
                "sourceExactExternalOpacityAdmission": False,
                "sourceExactSamplerPolicyAdmission": False,
                "userVisualAdmission": False,
                "productAdmission": False,
                "nextGate": "DEBUG_RELEASE_BUILD_AND_ACTUAL_WARP_FIRST_DRAW_THEN_USER_VISUAL_REVIEW",
            },
        }
        seal(receipt)
        return receipt
    finally:
        original_runner.close()
        runtime_runner.close()


def validate_receipt(receipt: dict[str, Any]) -> None:
    require(receipt.get("schema") == SCHEMA, "runtime replay schema changed")
    require(receipt.get("formatVersion") == FORMAT_VERSION, "runtime replay format changed")
    claimed = receipt.get("receiptSha256")
    unsigned = dict(receipt)
    unsigned.pop("receiptSha256", None)
    seal(unsigned)
    require(claimed == unsigned["receiptSha256"], "runtime replay seal changed")

    runtime = receipt["runtimePixelShader"]
    require(runtime["sourcePath"] == DEFAULT_SHADER.relative_to(REPO_ROOT).as_posix(), "runtime shader path changed")
    require(runtime["normalizedSourceSha256"] == canonical_text_sha256(DEFAULT_SHADER), "runtime shader source is stale")
    require(runtime["compiledDxbcSha256"] == EXPECTED_RUNTIME_PIXEL_DXBC_SHA256, "runtime DXBC identity changed")
    require(runtime["compiledByteSize"] == EXPECTED_RUNTIME_PIXEL_DXBC_BYTE_SIZE, "runtime DXBC size changed")
    require(runtime["entry"] == "PS_MAIN" and runtime["profile"] == "ps_5_0", "runtime shader entry/profile changed")
    compiled = runtime["compiledBindingContract"]
    expected_compiled_contract = {
        "normalizedDisassemblySha256": "9832c7f14f0e7d8cc19a619915429fee820871ce2795bba724586a3de6468adc",
        "constantBufferRegister": "cb0",
        "constantBufferByteSize": 1760,
        "constantBufferFloat4Count": 110,
        "runtimeHeaderByteOffset": 1072,
        "runtimeScalarBlocksByteOffset": 1168,
        "runtimeVectorsByteOffset": 1376,
        "runtimeVectorConsumedMaskByteOffset": 1424,
        "runtimeVectorSuppressedMaskByteOffset": 1440,
        "cameraPositionByteOffset": 1712,
        "dynamicParameterByteOffset": 1744,
        "sourceTextureRegisters": ["t5", "t6", "t7", "t8"],
        "runtimeSamplerRegisters": ["s5", "s6", "s7", "s8"],
        "rdefAdmission": True,
        "declarationAdmission": True,
    }
    require(compiled == expected_compiled_contract, "runtime compiled ABI changed")
    require(compiled["sourceTextureRegisters"] == ["t5", "t6", "t7", "t8"], "runtime texture registers changed")
    require(compiled["runtimeSamplerRegisters"] == ["s5", "s6", "s7", "s8"], "runtime sampler registers changed")
    require(compiled["rdefAdmission"] and compiled["declarationAdmission"], "runtime compiled binding admission closed")

    includes = receipt["includedSourceReplayShaders"]
    require(includes["runtimeMaterialSha256"] == canonical_text_sha256(DEFAULT_RUNTIME_INCLUDE), "water source replay include is stale")
    require(includes["active011Sha256"] == canonical_text_sha256(DEFAULT_ACTIVE011_INCLUDE), "sprite source replay include is stale")
    require(receipt["backend"]["carrierDxbcSha256"] == EXPECTED_RUNTIME_CARRIER_DXBC_SHA256, "runtime carrier identity changed")
    require(
        receipt["backend"]["compiler"] == external_binary_identity(DEFAULT_COMPILER),
        "runtime compiler identity changed",
    )

    expected_boundary = {
        "originalExternalOpacityC0Recovered": False,
        "originalConstantFixtureExternalOpacity": ORIGINAL_EXTERNAL_OPACITY,
        "runtimeNeutralExternalOpacity": RUNTIME_NEUTRAL_EXTERNAL_OPACITY,
        "passFogAdmission": "IDENTITY_NEUTRAL_RUNTIME_PATH",
        "selectionColorAdmission": "BLACK_NEUTRAL_FIXED_FIXTURE",
        "particleColorCarrierAdmission": "DXBC_SSA_RENDERER_ABI_CORRELATION",
        "dynamicParameterAdmission": "EXACT_UNIFORM_VECTOR_EXPRESSION_C3",
        "pointClampSamplerPolicy": "COMMON_NUMERIC_FIXTURE_ONLY",
        "sourceExactSamplerPolicyAdmission": False,
        "occurrenceSelectedLocalVfRuntimeAdmission": False,
    }
    require(receipt["boundary"] == expected_boundary, "runtime replay boundary changed")

    prerequisite = receipt["originalDxbcReplayPrerequisite"]
    original_path = REPO_ROOT / prerequisite["path"]
    original = read_json(original_path)
    validate_original_replay_receipt(original)
    require(original["receiptSha256"] == prerequisite["receiptSha256"], "original replay prerequisite seal changed")
    require(prerequisite["caseCount"] == 21, "original replay prerequisite case count changed")
    require(prerequisite["fixedInputOriginalDxbcReplayAdmission"] and prerequisite["parameterRegisterTextureChannelClosureAdmission"], "original replay prerequisite admission closed")
    original_by_id = {row["caseId"]: row for row in original["numericContract"]["cases"]}

    constant = receipt["constantTextureMutationContract"]
    expected_constant_ids = [row[0] for row in CONSTANT_CASE_MAP]
    require(constant["caseCount"] == len(expected_constant_ids), "runtime constant case count changed")
    require(constant["caseIds"] == expected_constant_ids, "runtime constant case IDs changed")
    require([row["caseId"] for row in constant["cases"]] == expected_constant_ids, "runtime constant case order changed")
    mapping = dict(CONSTANT_CASE_MAP)
    for row in constant["cases"]:
        require(row["originalCaseId"] == mapping[row["caseId"]], "runtime/original case join changed")
        original_row = original_by_id[row["originalCaseId"]]
        expected = expected_constant_rgba(original_row)
        require(
            row["originalShaderIdHex"] == original_row["shaderIdHex"]
            and row["originalDxbcSha256"] == original_row["dxbcSha256"],
            "runtime constant original shader provenance changed",
        )
        require(
            all(
                abs(left - right) <= FLOAT_TOLERANCE
                for left, right in zip(
                    row["originalC0Point8Rgba"], original_row["mrtFloat4"][0]
                )
            ),
            "runtime constant original RT0 provenance changed",
        )
        require(all(abs(left - right) <= FLOAT_TOLERANCE for left, right in zip(row["expectedNeutralC0Rgba"], expected)), "runtime neutral-c0 expected value changed")
        require(all(abs(left - right) <= FLOAT_TOLERANCE for left, right in zip(row["actualRuntimeRgba"], expected)), "runtime constant replay result changed")
        require(row["maximumAbsoluteError"] <= FLOAT_TOLERANCE and row["comparisonAdmission"], "runtime constant replay admission failed")
    require(constant["waterNoiseGbaAlphaBitStable"] and constant["spriteRgbExponentAlphaBitStable"], "runtime negative control failed")

    spatial = receipt["nonuniformPointClampTranslationContract"]
    expected_spatial_ids = [row[0] for row in SPATIAL_CASES]
    require(spatial["textureExtent"] == [4, 4], "spatial texture extent changed")
    require(spatial["caseCount"] == len(expected_spatial_ids), "spatial case count changed")
    require(spatial["caseIds"] == expected_spatial_ids, "spatial case IDs changed")
    require([row["caseId"] for row in spatial["cases"]] == expected_spatial_ids, "spatial case order changed")
    require(spatial["maximumAbsoluteError"] <= SPATIAL_TOLERANCE, "spatial replay error changed")
    require(spatial["fixedFixtureTranslationParityAdmission"] and not spatial["sourceExactSamplerPolicyAdmission"], "spatial replay boundary changed")
    projection_sha256 = spatial_numeric_projection_sha256(spatial["cases"])
    require(
        projection_sha256 == spatial["numericProjectionSha256"]
        == EXPECTED_SPATIAL_NUMERIC_PROJECTION_SHA256,
        "spatial numeric projection changed",
    )
    relation_by_id = {row[0]: row[3] for row in SPATIAL_CASES}
    baseline_by_family = {
        row["family"]: row["originalRawDxbcRgba"]
        for row in spatial["cases"]
        if row["expectedRelation"] == "BASELINE"
    }
    require(len(baseline_by_family) == 2, "spatial baseline denominator changed")
    for row in spatial["cases"]:
        require(row["expectedRelation"] == relation_by_id[row["caseId"]], "spatial relation changed")
        require(row["comparisonAdmission"] and row["maximumAbsoluteError"] <= SPATIAL_TOLERANCE, "spatial case replay failed")
        require(all(abs(left - right) <= SPATIAL_TOLERANCE for left, right in zip(row["originalRawDxbcRgba"], row["actualRuntimeRgba"])), "spatial raw/runtime result changed")
        recomputed_baseline_delta = max(
            abs(left - right)
            for left, right in zip(
                row["originalRawDxbcRgba"], baseline_by_family[row["family"]]
            )
        )
        require(
            abs(
                recomputed_baseline_delta
                - row["originalVersusFamilyBaselineMaximumDelta"]
            )
            <= FLOAT_TOLERANCE,
            "spatial baseline delta projection changed",
        )
        if row["expectedRelation"] == "SENSITIVE":
            require(row["originalVersusFamilyBaselineMaximumDelta"] > SPATIAL_TOLERANCE, "spatial sensitive axis collapsed")

    decision = receipt["decision"]
    require(
        decision["shippedRuntimeHlslCompiled"]
        and decision["compiledRuntimeRegisterContractAdmission"]
        and decision["constantTextureMutationReplayAdmission"]
        and decision["nonuniformUvTranslationReplayAdmission"]
        and decision["candidateRuntimeEvaluatorImplementationAdmission"],
        "runtime source replay admission closed",
    )
    require(
        not decision["occurrenceSelectedLocalVfRuntimeAdmission"]
        and not decision["occurrenceRuntimeEvaluatorMutationAdmission"]
        and not decision["sourceExactExternalOpacityAdmission"]
        and not decision["sourceExactSamplerPolicyAdmission"]
        and not decision["userVisualAdmission"]
        and not decision["productAdmission"],
        "runtime source replay overclaims admission",
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--shader", type=Path, default=DEFAULT_SHADER)
    parser.add_argument("--compiler", type=Path, default=DEFAULT_COMPILER)
    parser.add_argument("--original-replay", type=Path, default=DEFAULT_ORIGINAL_REPLAY)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--validate-only", action="store_true")
    args = parser.parse_args()
    if args.validate_only:
        receipt = read_json(args.output)
        validate_receipt(receipt)
        print("Artist 31470 main shipped-runtime source replay receipt PASS")
        return 0
    receipt = build_receipt(args.shader, args.compiler, args.original_replay)
    validate_receipt(receipt)
    encoded = json.dumps(receipt, ensure_ascii=False, indent=2) + "\n"
    if args.check:
        require(args.output.is_file(), f"receipt is missing: {args.output}")
        require(args.output.read_text(encoding="utf-8-sig") == encoded, "runtime replay receipt is stale")
    else:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(encoded, encoding="utf-8", newline="\n")
    print(
        "Artist 31470 main shipped-runtime source replay PASS: "
        f"constant={len(receipt['constantTextureMutationContract']['cases'])} "
        f"spatial={len(receipt['nonuniformPointClampTranslationContract']['cases'])} "
        "product=false visual=false"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
