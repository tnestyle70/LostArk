#!/usr/bin/env python3
"""Validate the Glasshole02 RT0 runtime carrier against the cooked PS oracle.

This is a focused executable gate, not a Product or visual admission receipt.
It compiles the source-controlled particle VS/RT0 PS and FX11 technique, proves
the VTXEFFECT_PARTICLE/source-varying bridge, evaluates the exact cooked
uniform-expression AST at several local times, and compares the raw cooked PS
with the RT0 wrapper on D3D11 WARP.
"""

from __future__ import annotations

import argparse
import ctypes
import math
import struct
import sys
from pathlib import Path
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
LEVEL_TOOLS = REPOSITORY_ROOT / "Tools" / "LevelPlacementExtractor"
if str(LEVEL_TOOLS) not in sys.path:
    sys.path.insert(0, str(LEVEL_TOOLS))

import evaluate_ue3_material_uniform_expressions as uniform_evaluator  # noqa: E402
import replay_ue3_glasshole02_hlsl_translation as translation  # noqa: E402
from extract_artist_31470_main_ref_shader_cache import (  # noqa: E402
    D3DDisassembler,
    DEFAULT_D3DCOMPILER,
    require,
)
from extract_artist_31470_shader_cache_oracle import (  # noqa: E402
    validate_dxbc_container,
)
from replay_artist_31470_main_original_dxbc import (  # noqa: E402
    HRESULT,
    UINT,
    blob_bytes,
    external_binary_identity,
    release,
)
from replay_ue3_material_pixel_shaders import (  # noqa: E402
    CarrierCompiler,
    GenericWarpReplay,
    build_carrier_source,
    close_carrier_signature,
    dxbc_chunks,
    parse_runtime_declarations,
    parse_signature,
    public_signature,
)


DEFAULT_RUNTIME_SHADER = REPOSITORY_ROOT / (
    "Client/Bin/ShaderFiles/Shader_VtxEffectGlasshole02.hlsl"
)
DEFAULT_RAW_DXBC = translation.DEFAULT_RAW_DXBC
DEFAULT_TRANSLATION_INCLUDE = translation.DEFAULT_INCLUDE
DEFAULT_EXACT_MAP_RECEIPT = REPOSITORY_ROOT / (
    "Data/Effects/Imported/DimensionMaster/Materials/"
    "skill.2050120.clip3.exact-material-maps.receipt.json"
)
DEFAULT_UNIFORM_RECEIPT = translation.DEFAULT_UNIFORM_RECEIPT

RUNTIME_LOCAL_TIMES = (0.0, 0.25, 0.6)
EXPECTED_AST_TIME_SENSITIVE_CB0_SLOTS = (
    2,
    3,
    7,
    8,
    10,
    11,
    13,
    15,
    17,
    19,
    20,
    21,
)
EXPECTED_NUMERICALLY_CHANGED_CB0_SLOTS = (2, 3, 10, 11, 13, 17, 19, 20, 21)
EXPECTED_VERTEX_INPUTS = (
    ("POSITION", 0, 0x7),
    ("TEXCOORD", 0, 0x3),
    ("WORLD", 0, 0xF),
    ("WORLD", 1, 0xF),
    ("WORLD", 2, 0xF),
    ("WORLD", 3, 0xF),
    ("COLOR", 0, 0xF),
    ("DYNAMIC", 0, 0xF),
    ("UVTRANSFORM", 0, 0xF),
    ("UVTRANSFORM", 1, 0xF),
    ("PARTICLEDATA", 0, 0x3),
)
EXPECTED_VERTEX_OUTPUTS = (
    ("TEXCOORD", 10, 0xF),
    ("TEXCOORD", 11, 0xF),
    ("TEXCOORD", 0, 0xF),
    ("TEXCOORD", 1, 0xF),
    ("TEXCOORD", 2, 0xF),
    ("TEXCOORD", 4, 0xF),
    ("TEXCOORD", 6, 0xF),
    ("TEXCOORD", 5, 0xF),
    ("SV_POSITION", 0, 0xF),
)
EXPECTED_RUNTIME_PS_CBS = {"0": 22, "1": 9, "2": 2}
EXPECTED_TEXTURE_REGISTERS = list(range(8))
EXPECTED_SAMPLER_REGISTERS = list(range(8))
FLOAT_TOLERANCE = 1.0e-6
DEPTH_EQUIVALENCE_TOLERANCE = 2.0e-3
NEAR_PLANE = 0.1
FAR_PLANE = 1000.0
D3DCOMPILE_DEBUG = 1 << 0
D3DCOMPILE_SKIP_OPTIMIZATION = 1 << 2
D3DCOMPILE_OPTIMIZATION_LEVEL1 = 0


def _float32(value: float) -> float:
    return struct.unpack("<f", struct.pack("<f", float(value)))[0]


def _compile_entry(
    path: Path,
    entry: str | None,
    profile: str,
    compiler: Path,
    flags: int = 0,
) -> bytes:
    require(path.is_file(), f"runtime Glasshole02 shader is missing: {path}")
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
            ctypes.c_void_p(1),
            entry.encode("ascii") if entry is not None else None,
            profile.encode("ascii"),
            flags,
            0,
            ctypes.byref(code),
            ctypes.byref(errors),
        )
        error_text = (
            blob_bytes(errors).decode("utf-8", "replace")
            if errors.value
            else ""
        )
        require(
            result >= 0 and code.value,
            f"Glasshole02 {profile}/{entry or '<effect>'} compile failed "
            f"0x{result & 0xFFFFFFFF:08X}: {error_text}",
        )
        return blob_bytes(code)
    finally:
        release(errors)
        release(code)


def _target(receipt: dict[str, Any]) -> dict[str, Any]:
    row = next(
        (
            value
            for value in receipt["targets"]
            if value.get("targetId") == translation.TARGET_ID
        ),
        None,
    )
    require(row is not None, "Glasshole02 target is absent")
    require(
        row.get("familyId") == translation.FAMILY_ID,
        "Glasshole02 family identity changed",
    )
    return row


def _depends_on_runtime_time(value: Any) -> bool:
    if isinstance(value, dict):
        type_name = str(value.get("typeName", "")).casefold()
        if type_name in (
            "fmaterialuniformexpressiontime",
            "fmaterialuniformexpressionrealtime",
        ):
            return True
        return any(_depends_on_runtime_time(child) for child in value.values())
    if isinstance(value, list):
        return any(_depends_on_runtime_time(child) for child in value)
    return False


def _ast_time_sensitive_slots(exact_target: dict[str, Any]) -> list[int]:
    uniform_set = exact_target["materialMap"]["uniformExpressionSet"]
    scalar_indices = {
        index
        for index, expression in enumerate(uniform_set["pixelScalarExpressions"])
        if _depends_on_runtime_time(expression)
    }
    vector_indices = {
        index
        for index, expression in enumerate(uniform_set["pixelVectorExpressions"])
        if _depends_on_runtime_time(expression)
    }
    native = exact_target["nativeShaderObjectBinding"]
    slots = set()
    for wire in native["scalarGroups"]:
        group = int(wire["expressionIndexOrGroup"])
        if any(group * 4 + lane in scalar_indices for lane in range(4)):
            slots.add(int(wire["baseIndex"]) // 16)
    for wire in native["vectors"]:
        if int(wire["expressionIndexOrGroup"]) in vector_indices:
            slots.add(int(wire["baseIndex"]) // 16)
    return sorted(slots)


def _runtime_cb0(
    exact_target: dict[str, Any],
    uniform_target: dict[str, Any],
    local_time_seconds: float,
) -> list[list[float]]:
    require(
        math.isfinite(local_time_seconds) and local_time_seconds >= 0.0,
        "Glasshole02 local time must be finite and nonnegative",
    )
    context = {
        "gameTimeSeconds": local_time_seconds,
        "realTimeSeconds": local_time_seconds,
        "floatSemantics": (
            "IEEE754_FLOAT32_ROUND_AFTER_EACH_UNIFORM_EXPRESSION_OPERATION"
        ),
    }
    evaluation = uniform_evaluator.evaluate_uniform_set_into_cb0(
        exact_target["materialMap"]["uniformExpressionSet"],
        exact_target["nativeShaderObjectBinding"],
        uniform_target["effectiveScalarOverrides"],
        uniform_target["effectiveVectorOverrides"],
        context,
    )
    native = evaluation["nativeCb0"]
    require(native["declaredFloat4Count"] == 22, "runtime CB0 extent changed")
    rows: list[list[float] | None] = [None] * 22
    for row in native["materialRows"]:
        rows[int(row["slot"])] = [float(lane) for lane in row["value"]]
    # Source c0 is renderer-owned external opacity.  Match the sealed G03-7
    # numeric fixture while leaving its runtime value an explicit caller input.
    rows[0] = [0.85, 0.0, 0.0, 0.0]
    require(all(row is not None for row in rows), "runtime CB0 is sparse")
    return [list(row) for row in rows if row is not None]


def _projection_depth_contract() -> dict[str, Any]:
    p33 = _float32(FAR_PLANE / (FAR_PLANE - NEAR_PLANE))
    p43 = _float32(
        -(NEAR_PLANE * FAR_PLANE) / (FAR_PLANE - NEAR_PLANE)
    )
    inverse_p43 = _float32(1.0 / p43)
    p33_over_p43 = _float32(p33 / p43)
    probes = []
    maximum_error = 0.0
    for view_z in (2.75, 4.5, 7.0):
        source_view_z = _float32(view_z)
        clip_z = _float32(_float32(source_view_z * p33) + p43)
        ndc_depth = _float32(clip_z / source_view_z)
        denominator = _float32(
            _float32(ndc_depth * inverse_p43) - p33_over_p43
        )
        reconstructed = _float32(1.0 / denominator)
        target_depth_y = _float32(source_view_z / 1000.0)
        engine_view_z = _float32(target_depth_y * 1000.0)
        error = abs(reconstructed - engine_view_z)
        maximum_error = max(maximum_error, error)
        probes.append(
            {
                "targetDepthX": ndc_depth,
                "targetDepthY": target_depth_y,
                "sourceProjectionReconstructedViewZ": reconstructed,
                "engineTargetDepthYTimes1000": engine_view_z,
                "absoluteError": error,
            }
        )
    require(
        maximum_error <= DEPTH_EQUIVALENCE_TOLERANCE,
        f"Target_Depth x/y view-depth equivalence changed: {maximum_error}",
    )
    return {
        "projectionP33": p33,
        "projectionP43": p43,
        "cb2Row1": [0.0, 0.0, inverse_p43, p33_over_p43],
        "probes": probes,
        "maximumAbsoluteError": maximum_error,
        "tolerance": DEPTH_EQUIVALENCE_TOLERANCE,
        "pass": True,
    }


def _engine_depth_pixel(view_z: float, depth: dict[str, Any]) -> list[float]:
    p33 = float(depth["projectionP33"])
    p43 = float(depth["projectionP43"])
    view_z = _float32(view_z)
    clip_z = _float32(_float32(view_z * p33) + p43)
    ndc_depth = _float32(clip_z / view_z)
    return [ndc_depth, _float32(view_z / 1000.0), 0.0, 1.0]


def _signature_contract(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    return [
        {
            key: value
            for key, value in row.items()
            if key != "readWriteMask"
        }
        for row in rows
    ]


def _semantic_triplets(rows: list[dict[str, Any]]) -> tuple[tuple[str, int, int], ...]:
    return tuple(
        (
            str(row["semanticName"]).upper(),
            int(row["semanticIndex"]),
            int(row["mask"]),
        )
        for row in rows
    )


def _runtime_cb1(local_time_seconds: float) -> list[list[float]]:
    # FX11 packs g_ViewMatrix, g_ProjMatrix, then the local-time scalar into b1.
    rows = [[0.0, 0.0, 0.0, 0.0] for _ in range(9)]
    rows[0] = [1.0, 0.0, 0.0, 0.0]
    rows[1] = [0.0, 1.0, 0.0, 0.0]
    rows[2] = [0.0, 0.0, 1.0, 0.0]
    rows[3] = [0.0, 0.0, 0.0, 1.0]
    rows[4] = [1.0, 0.0, 0.0, 0.0]
    rows[5] = [0.0, 1.0, 0.0, 0.0]
    rows[6] = [0.0, 0.0, 1.0, 0.0]
    rows[7] = [0.0, 0.0, 0.0, 1.0]
    rows[8] = [_float32(local_time_seconds), 0.0, 0.0, 0.0]
    return rows


def _case_fixture(
    index: int,
    local_time_seconds: float,
    cb0: list[list[float]],
    depth: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any]]:
    # Build the carrier directly so this gate never falls back to G03-7's fixed
    # time-zero CB helper.
    carrier = [[0.0, 0.0, 0.0, 0.0] for _ in range(8)]
    carrier[0] = [0.17, 0.31, 0.93, 0.0]
    carrier[1] = [0.88, 0.21, 0.37, -1.0]
    carrier[2] = (
        [0.18, 0.73, 0.0, 0.0]
        if index == 0
        else [0.77, 0.83, 0.0, 0.0]
        if index == 1
        else [0.81, 0.24, 0.0, 0.0]
    )
    carrier[3] = [0.8 - index * 0.11, 0.72, 0.91, 0.75 - index * 0.1]
    carrier[4] = [0.65, 1.05 + index * 0.13, 0.25, 0.8 - index * 0.12]
    carrier[5] = [0.01, 0.02, 0.03, 0.97]
    carrier[6] = [0.2 + index * 0.19, 0.7 - index * 0.12, 0.68, 1.0]
    carrier[7] = [0.1 - index * 0.17, -0.1 + index * 0.14, 0.0, 1.5]

    cb2 = [
        [0.5, -0.5, 0.5, 0.5],
        list(depth["cb2Row1"]),
        [0.0, 0.0, 0.0, 0.0],
        [0.11, 0.23, 0.37, 0.0],
    ]
    textures = {
        str(register): translation.coordinate_texture(register + 1)["pixels"][
            (index * 5 + register * 3) % 16
        ]
        for register in range(8)
    }
    textures["2"] = _engine_depth_pixel(3.5 + index * 1.25, depth)
    raw = {
        "constantBuffers": {"0": cb0, "2": cb2},
        "carrierRows": carrier,
        "textures": textures,
    }
    runtime = {
        "constantBuffers": {
            "0": cb0,
            "1": _runtime_cb1(local_time_seconds),
            "2": cb2[:2],
        },
        "carrierRows": carrier,
        "textures": textures,
    }
    return raw, runtime


def build_report(
    runtime_shader_path: Path = DEFAULT_RUNTIME_SHADER,
    raw_dxbc_path: Path = DEFAULT_RAW_DXBC,
    translation_include_path: Path = DEFAULT_TRANSLATION_INCLUDE,
    exact_map_receipt_path: Path = DEFAULT_EXACT_MAP_RECEIPT,
    uniform_receipt_path: Path = DEFAULT_UNIFORM_RECEIPT,
    compiler: Path = DEFAULT_D3DCOMPILER,
) -> dict[str, Any]:
    raw_dxbc = raw_dxbc_path.read_bytes()
    require(
        translation.sha256_bytes(raw_dxbc) == translation.EXPECTED_RAW_DXBC_SHA256,
        "raw Glasshole02 PS changed",
    )
    validate_dxbc_container(raw_dxbc)

    runtime_vs = _compile_entry(runtime_shader_path, "VS_MAIN", "vs_5_0", compiler)
    runtime_ps = _compile_entry(runtime_shader_path, "PS_MAIN", "ps_5_0", compiler)
    runtime_fx_debug = _compile_entry(
        runtime_shader_path,
        None,
        "fx_5_0",
        compiler,
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
    )
    runtime_fx_release = _compile_entry(
        runtime_shader_path,
        None,
        "fx_5_0",
        compiler,
        D3DCOMPILE_OPTIMIZATION_LEVEL1,
    )
    validate_dxbc_container(runtime_vs)
    validate_dxbc_container(runtime_ps)

    disassembler = D3DDisassembler(compiler)
    raw_disassembly = disassembler.disassemble(raw_dxbc)
    vs_disassembly = disassembler.disassemble(runtime_vs)
    ps_disassembly = disassembler.disassemble(runtime_ps)
    raw_chunks = dxbc_chunks(raw_dxbc)
    vs_chunks = dxbc_chunks(runtime_vs)
    ps_chunks = dxbc_chunks(runtime_ps)
    raw_input = parse_signature(raw_chunks["ISGN"])
    vs_input = parse_signature(vs_chunks["ISGN"])
    vs_output = parse_signature(vs_chunks["OSGN"])
    runtime_input = parse_signature(ps_chunks["ISGN"])
    runtime_output = parse_signature(ps_chunks["OSGN"])
    require(
        _semantic_triplets(vs_input) == EXPECTED_VERTEX_INPUTS,
        "runtime VS no longer consumes VTXEFFECT_PARTICLE",
    )
    require(
        _semantic_triplets(vs_output) == EXPECTED_VERTEX_OUTPUTS,
        "runtime VS no longer produces the Glasshole source varyings",
    )
    require(
        _signature_contract(runtime_input) == _signature_contract(raw_input),
        "runtime RT0 PS input semantic/register ABI changed",
    )
    require(
        len(runtime_output) == 1
        and runtime_output[0]["semanticName"].casefold() == "sv_target"
        and runtime_output[0]["register"] == 0
        and runtime_output[0]["mask"] == 0xF,
        "runtime Glasshole PS is not RT0-only",
    )

    raw_declarations = parse_runtime_declarations(raw_disassembly)
    runtime_declarations = parse_runtime_declarations(ps_disassembly)
    require(
        runtime_declarations["constantBufferFloat4Counts"]
        == EXPECTED_RUNTIME_PS_CBS,
        "runtime Glasshole constant-buffer ABI changed",
    )
    require(
        runtime_declarations["textureRegisters"] == EXPECTED_TEXTURE_REGISTERS
        and runtime_declarations["samplerRegisters"] == EXPECTED_SAMPLER_REGISTERS,
        "runtime Glasshole texture/sampler ABI changed",
    )
    require(
        ps_disassembly["instructionCount"] == 173,
        "runtime RT0 instruction denominator changed",
    )

    shader_source = runtime_shader_path.read_text(encoding="utf-8-sig")
    include_source = translation_include_path.read_text(encoding="utf-8-sig")
    for required_text in (
        '#include "Shader_Ue3Glasshole02.hlsli"',
        "g_Glasshole02LocalTimeSeconds",
        "clip(g_Glasshole02LocalTimeSeconds)",
        "input.uv * input.uvTransform.xy + input.uvTransform.zw",
        "output.dynamicParameter = input.dynamicParameter",
        "Target_Depth.y * 1000",
        "pass Glasshole02AlphaTwoSidedDepthRead",
    ):
        require(required_text in shader_source, f"runtime shader seam changed: {required_text}")
    require(
        "float4 g_Ue3Glasshole02CB0[22]" in include_source,
        "translated CB0 declaration changed",
    )

    exact_receipt = translation.read_json(exact_map_receipt_path)
    uniform_receipt = translation.read_json(uniform_receipt_path)
    uniform_evaluator.validate_output_receipt(uniform_receipt)
    exact_target = _target(exact_receipt)
    uniform_target = _target(uniform_receipt)
    ast_slots = _ast_time_sensitive_slots(exact_target)
    require(
        ast_slots == list(EXPECTED_AST_TIME_SENSITIVE_CB0_SLOTS),
        "Glasshole AST time-sensitive CB0 slots changed",
    )
    cb0_by_time = {
        local_time: _runtime_cb0(exact_target, uniform_target, local_time)
        for local_time in RUNTIME_LOCAL_TIMES
    }
    require(
        cb0_by_time[0.0] == translation.source_cb0(uniform_receipt),
        "runtime t=0 CB0 no longer matches the sealed translation fixture",
    )
    numeric_changed_slots = [
        slot
        for slot in range(22)
        if len(
            {
                tuple(cb0_by_time[local_time][slot])
                for local_time in RUNTIME_LOCAL_TIMES
            }
        )
        > 1
    ]
    require(
        numeric_changed_slots == list(EXPECTED_NUMERICALLY_CHANGED_CB0_SLOTS),
        "Glasshole runtime CB0 became static or changed its time projection",
    )
    signed_periodic_probe = uniform_evaluator._signed_fractional_float32(-1.25)
    require(
        signed_periodic_probe == -0.25,
        "UE3 signed Periodic semantics changed to HLSL frac semantics",
    )

    carrier_source, carrier_row_count = build_carrier_source(raw_input)
    require(carrier_row_count == 8, "Glasshole runtime carrier extent changed")
    carrier_dxbc, carrier_identity = CarrierCompiler(compiler).compile(
        carrier_source, "ue3-glasshole02-runtime-rt0-carrier"
    )
    carrier_output = parse_signature(dxbc_chunks(carrier_dxbc)["OSGN"])
    raw_carrier_closure = close_carrier_signature(raw_input, carrier_output)
    runtime_carrier_closure = close_carrier_signature(runtime_input, carrier_output)

    depth = _projection_depth_contract()
    cases = []
    runner = GenericWarpReplay()
    try:
        for index, local_time in enumerate(RUNTIME_LOCAL_TIMES):
            raw_fixture, runtime_fixture = _case_fixture(
                index, local_time, cb0_by_time[local_time], depth
            )
            raw_outputs = runner.run(
                raw_dxbc, carrier_dxbc, raw_fixture, raw_declarations, 6
            )
            runtime_outputs = runner.run(
                runtime_ps,
                carrier_dxbc,
                runtime_fixture,
                runtime_declarations,
                6,
            )
            require(
                all(
                    math.isfinite(lane)
                    for output in (raw_outputs, runtime_outputs)
                    for target in output
                    for lane in target
                ),
                f"Glasshole runtime replay became non-finite at t={local_time}",
            )
            error = max(
                abs(raw_outputs[0][lane] - runtime_outputs[0][lane])
                for lane in range(4)
            )
            require(
                error <= FLOAT_TOLERANCE,
                f"Glasshole runtime RT0 mismatch at t={local_time}: {error}",
            )
            require(
                all(
                    runtime_outputs[target] == [translation.SENTINEL] * 4
                    for target in range(1, 6)
                ),
                "runtime Glasshole wrapper wrote a non-RT0 target",
            )
            cases.append(
                {
                    "localTimeSeconds": local_time,
                    "rawDxbcRt0": raw_outputs[0],
                    "runtimeHlslRt0": runtime_outputs[0],
                    "maximumAbsoluteError": error,
                    "runtimeUndeclaredTargetsRemainSentinel": True,
                    "pass": True,
                }
            )
    finally:
        runner.close()

    report = {
        "target": {
            "targetId": translation.TARGET_ID,
            "familyId": translation.FAMILY_ID,
            "rendererType": "SpriteParticle",
        },
        "runtimeShader": {
            "path": runtime_shader_path.relative_to(REPOSITORY_ROOT).as_posix(),
            "sourceSha256": translation.normalized_text_sha256(runtime_shader_path),
            "translationIncludeSha256": translation.normalized_text_sha256(
                translation_include_path
            ),
            "fx11Debug": {
                "compileFlags": ["DEBUG", "SKIP_OPTIMIZATION"],
                "compiledByteSize": len(runtime_fx_debug),
                "compiledSha256": translation.sha256_bytes(runtime_fx_debug),
            },
            "fx11Release": {
                "compileFlags": ["OPTIMIZATION_LEVEL1"],
                "compiledByteSize": len(runtime_fx_release),
                "compiledSha256": translation.sha256_bytes(runtime_fx_release),
            },
            "vertex": {
                "entry": "VS_MAIN",
                "profile": "vs_5_0",
                "instructionCount": vs_disassembly["instructionCount"],
                "inputSignature": public_signature(vs_input),
                "outputSignature": public_signature(vs_output),
                "vtxEffectParticleInputSignatureClosed": True,
                "offsetCenterInstanceWorldAdapterContract": True,
                "psaRectangleIndependentWorldXYScaleAdapterContract": True,
                "dynamicParameterForwardedWithoutRemap": True,
                "rawSourceVertexSpatialParity": False,
            },
            "pixel": {
                "entry": "PS_MAIN",
                "profile": "ps_5_0",
                "instructionCount": ps_disassembly["instructionCount"],
                "compiledByteSize": len(runtime_ps),
                "compiledSha256": translation.sha256_bytes(runtime_ps),
                "constantBufferFloat4Counts": runtime_declarations[
                    "constantBufferFloat4Counts"
                ],
                "textureRegisters": runtime_declarations["textureRegisters"],
                "samplerRegisters": runtime_declarations["samplerRegisters"],
                "inputSignature": public_signature(runtime_input),
                "outputSignature": public_signature(runtime_output),
                "rt0Only": True,
            },
            "carrier": {
                **carrier_identity,
                "rawPixelSignatureClosure": raw_carrier_closure,
                "runtimePixelSignatureClosure": runtime_carrier_closure,
            },
        },
        "runtimeUniformExpressionContract": {
            "localTimeIdentifier": "g_Glasshole02LocalTimeSeconds",
            "materialConstantIdentifier": "g_Ue3Glasshole02CB0",
            "evaluationTimesSeconds": list(RUNTIME_LOCAL_TIMES),
            "astTimeSensitiveCb0Slots": ast_slots,
            "numericallyChangedCb0Slots": numeric_changed_slots,
            "staticTimeZeroCb0Rejected": True,
            "signedPeriodicNegativeProbe": signed_periodic_probe,
            "signedPeriodicSemantics": "x-trunc(x)",
            "pass": True,
        },
        "targetDepthContract": {
            "runtimeRenderTarget": "Target_Depth",
            "shaderResourceIdentifier": "g_Ue3Glasshole02SceneDepth",
            "sourceDepthLane": "x=NDC_Z",
            "engineViewDepthLane": "y=VIEW_Z_DIV_1000",
            **depth,
        },
        "warpComparison": {
            "backend": "D3D11_WARP_RAW_DXBC_VERSUS_RUNTIME_RT0_HLSL",
            "featureLevel": f"0x{runner.feature_level:04x}",
            "compiler": external_binary_identity(compiler),
            "caseCount": len(cases),
            "tolerance": FLOAT_TOLERANCE,
            "maximumAbsoluteError": max(
                row["maximumAbsoluteError"] for row in cases
            ),
            "cases": cases,
            "pass": True,
        },
        "decision": {
            "runtimeRt0ShaderCompiled": True,
            "runtimeRt0RawOracleParity": True,
            "runtimeVertexBridgeStructureClosed": True,
            "rawSourceVertexSpatialParity": False,
            "runtimeTimeVaryingCb0ContractClosed": True,
            "runtimeRendererBindingAdmission": False,
            "productAdmission": False,
            "visualAdmission": False,
        },
    }
    validate_report(report)
    return report


def validate_report(report: dict[str, Any]) -> None:
    require(
        report["runtimeShader"]["fx11Debug"]["compiledByteSize"] > 0
        and report["runtimeShader"]["fx11Release"]["compiledByteSize"] > 0,
        "Debug/Release FX11 compile is absent",
    )
    require(report["runtimeShader"]["pixel"]["rt0Only"] is True, "RT0 gate failed")
    require(
        report["runtimeUniformExpressionContract"]["staticTimeZeroCb0Rejected"]
        is True,
        "static CB0 regression gate failed",
    )
    require(report["targetDepthContract"]["pass"] is True, "depth gate failed")
    require(report["warpComparison"]["pass"] is True, "WARP gate failed")
    decision = report["decision"]
    require(
        decision["runtimeVertexBridgeStructureClosed"] is True
        and decision["rawSourceVertexSpatialParity"] is False
        and report["runtimeShader"]["vertex"]["rawSourceVertexSpatialParity"]
        is False
        and decision["runtimeRendererBindingAdmission"] is False
        and decision["productAdmission"] is False
        and decision["visualAdmission"] is False,
        "focused HLSL gate overclaimed runtime/product/visual admission",
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--validate-only",
        action="store_true",
        help="Build and validate the focused report without writing a receipt.",
    )
    parser.parse_args()
    try:
        report = build_report()
        print(
            "Glasshole02 runtime RT0 PASS: "
            f"cases={report['warpComparison']['caseCount']} "
            f"maxError={report['warpComparison']['maximumAbsoluteError']:.9g} "
            "renderer=0 product=0 visual=0"
        )
        return 0
    except (OSError, ValueError, RuntimeError) as error:
        print(f"Glasshole02 runtime RT0 FAIL: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
