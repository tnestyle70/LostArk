#!/usr/bin/env python3
"""Compare the Glasshole02 cooked pixel DXBC with its shipped HLSL translation.

The cooked shader remains an offline numeric oracle.  Both pixel shaders are
drawn through the same generated signature carrier on D3D11 WARP with identical
CB0/CB2 bytes, texture grids, sampler states, and particle varyings.  Passing
this receipt admits only the fixed-fixture translation equation; it does not
admit the HLSL to Product, runtime authoring, or visual fidelity.
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
import sys
from pathlib import Path
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
LEVEL_TOOLS = REPOSITORY_ROOT / "Tools" / "LevelPlacementExtractor"
if str(LEVEL_TOOLS) not in sys.path:
    sys.path.insert(0, str(LEVEL_TOOLS))

import evaluate_ue3_material_uniform_expressions as uniform_evaluator  # noqa: E402
from extract_artist_31470_main_ref_shader_cache import (  # noqa: E402
    D3DDisassembler,
    DEFAULT_D3DCOMPILER,
    require,
)
from extract_artist_31470_shader_cache_oracle import (  # noqa: E402
    canonical_json_sha256,
    validate_dxbc_container,
)
from replay_artist_31470_main_original_dxbc import (  # noqa: E402
    BufferDesc,
    HRESULT,
    MappedSubresource,
    SampleDesc,
    SubresourceData,
    Texture2DDesc,
    UINT,
    Viewport,
    blob_bytes,
    checked,
    com_method,
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


SCHEMA = "lostark.effect-ue3-glasshole02-hlsl-translation-receipt"
FORMAT_VERSION = 1
TARGET_ID = "dimensionmaster-w-glasshole-02"
FAMILY_ID = (
    "ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.glasshole.02.tr.175266c16bb2"
)
OCCURRENCE_ID = "authored.source-particle.40e1b48e2f0f88dcfeff1549"

DEFAULT_RAW_DXBC = REPOSITORY_ROOT / (
    "Data/Effects/CookedShaders/"
    "e2ba1c1ef87cdd52cc74a8e661f8613d0b17f2cc7b9b1d0d7ab6ed80ec6e775b.dxbc"
)
DEFAULT_INCLUDE = REPOSITORY_ROOT / (
    "Client/Bin/ShaderFiles/Shader_Ue3Glasshole02.hlsli"
)
DEFAULT_SHADER = REPOSITORY_ROOT / (
    "Client/Bin/ShaderFiles/Shader_Ue3Glasshole02Translation.hlsl"
)
DEFAULT_UNIFORM_RECEIPT = REPOSITORY_ROOT / (
    "Data/Effects/Imported/DimensionMaster/Materials/"
    "skill.2050120.clip3.source-value-uniform-evaluation.receipt.json"
)
DEFAULT_OUTPUT = REPOSITORY_ROOT / (
    "Data/Effects/Imported/DimensionMaster/Materials/"
    "skill.2050120.clip3.glasshole02-hlsl-translation.receipt.json"
)

EXPECTED_RAW_DXBC_SHA256 = (
    "e2ba1c1ef87cdd52cc74a8e661f8613d0b17f2cc7b9b1d0d7ab6ed80ec6e775b"
)
EXPECTED_RAW_DXBC_BYTE_SIZE = 7_584
EXPECTED_RAW_INSTRUCTION_COUNT = 198
EXPECTED_CB_COUNTS = {"0": 22, "2": 4}
EXPECTED_TEXTURE_REGISTERS = list(range(8))
EXPECTED_SAMPLER_REGISTERS = list(range(8))
EXPECTED_MRT_REGISTERS = [0, 2, 3, 4, 5]
TEXTURE_EXTENT = 4
SENTINEL = -99.0
FLOAT_TOLERANCE = 1.0e-6
SENSITIVITY_THRESHOLD = 1.0e-5


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(8 * 1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def normalized_text_sha256(path: Path) -> str:
    text = path.read_text(encoding="utf-8-sig")
    return sha256_bytes(
        text.replace("\r\n", "\n").replace("\r", "\n").encode("utf-8")
    )


def read_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8-sig") as source:
        value = json.load(source)
    require(isinstance(value, dict), f"JSON root must be an object: {path}")
    return value


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
        newline="\n",
    )
    temporary.replace(path)


def seal(value: dict[str, Any]) -> None:
    value.pop("receiptSha256", None)
    value["receiptSha256"] = canonical_json_sha256(value)


def descriptor(path: Path, role: str) -> dict[str, Any]:
    resolved = path.resolve()
    require(resolved.is_file(), f"required input is missing: {resolved}")
    try:
        display_path = resolved.relative_to(REPOSITORY_ROOT.resolve()).as_posix()
    except ValueError:
        display_path = resolved.as_posix()
    return {
        "path": display_path,
        "byteSize": resolved.stat().st_size,
        "sha256": sha256_file(resolved),
        "role": role,
    }


def compile_pixel_shader(path: Path, compiler: Path) -> bytes:
    require(path.is_file(), f"translated shader is missing: {path}")
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
            b"PS_MAIN",
            b"ps_5_0",
            0,
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
            f"Glasshole02 HLSL compile failed 0x{result & 0xFFFFFFFF:08X}: "
            f"{error_text}",
        )
        return blob_bytes(code)
    finally:
        release(errors)
        release(code)


def source_cb0(uniform_receipt: dict[str, Any]) -> list[list[float]]:
    uniform_evaluator.validate_output_receipt(uniform_receipt)
    target = next(
        (
            row
            for row in uniform_receipt["targets"]
            if row.get("targetId") == TARGET_ID
        ),
        None,
    )
    require(target is not None, "Glasshole02 source-value target is absent")
    require(
        target.get("familyId") == FAMILY_ID,
        "Glasshole02 source-value family changed",
    )
    native = target["sourceValueUniformEvaluation"]["nativeCb0"]
    require(native["declaredFloat4Count"] == 22, "Glasshole02 CB0 extent changed")
    rows: list[list[float] | None] = [None] * 22
    for row in native["allRows"]:
        slot = int(row["slot"])
        value = row["value"]
        if slot == 0:
            require(value is None, "renderer-owned c0 unexpectedly became source material")
            continue
        require(
            isinstance(value, list) and len(value) == 4,
            f"Glasshole02 source CB0 row is invalid: c{slot}",
        )
        rows[slot] = [float(lane) for lane in value]
    rows[0] = [0.85, 0.0, 0.0, 0.0]
    require(all(row is not None for row in rows), "Glasshole02 source CB0 is sparse")
    return [list(row) for row in rows if row is not None]


def coordinate_texture(seed: int, *, scene_depth: bool = False) -> dict[str, Any]:
    pixels = []
    for y in range(TEXTURE_EXTENT):
        for x in range(TEXTURE_EXTENT):
            if scene_depth:
                depth = 0.56 + 0.035 * x + 0.025 * y + 0.004 * seed
                pixels.append([depth, 0.0, 0.0, 1.0])
            else:
                pixels.append(
                    [
                        0.08 + 0.025 * seed + 0.11 * x + 0.035 * y,
                        0.12 + 0.017 * seed + 0.045 * x + 0.09 * y,
                        0.16 + 0.013 * seed + 0.07 * x + 0.055 * y,
                        0.62 + 0.01 * ((seed + x + 2 * y) % 7),
                    ]
                )
    return {
        "width": TEXTURE_EXTENT,
        "height": TEXTURE_EXTENT,
        "pixels": pixels,
    }


def base_fixture(uniform_receipt: dict[str, Any]) -> dict[str, Any]:
    carrier = [[0.0, 0.0, 0.0, 0.0] for _ in range(8)]
    carrier[0] = [0.17, 0.31, 0.93, 0.0]
    carrier[1] = [0.88, 0.21, 0.37, -1.0]
    carrier[2] = [0.37, 0.42, 0.0, 0.0]
    carrier[3] = [0.8, 0.9, 0.7, 0.75]
    carrier[4] = [0.65, 1.05, 0.25, 0.8]
    carrier[5] = [0.01, 0.02, 0.03, 0.97]
    carrier[6] = [0.2, 0.7, 0.68, 1.0]
    carrier[7] = [0.1, -0.1, 0.0, 1.0]
    cb2 = [
        [0.5, 0.5, 0.5, 0.5],
        [1.0, 0.0, 2.0, 1.0],
        [0.0, 0.0, 0.0, 0.0],
        [0.11, 0.23, 0.37, 0.0],
    ]
    textures = [coordinate_texture(index + 1) for index in range(8)]
    textures[2] = coordinate_texture(2, scene_depth=True)
    return {
        "constantBuffers": {"0": source_cb0(uniform_receipt), "2": cb2},
        "carrierRows": carrier,
        "textures": textures,
    }


def translation_cases(uniform_receipt: dict[str, Any]) -> list[dict[str, Any]]:
    baseline = base_fixture(uniform_receipt)

    def case(
        case_id: str,
        relation: str,
        mutator: Any | None = None,
        *,
        spatial_pattern: bool = False,
        sensitivity_target: int = 0,
    ) -> dict[str, Any]:
        fixture = copy.deepcopy(baseline)
        if mutator is not None:
            mutator(fixture)
        return {
            "caseId": case_id,
            "expectedRelation": relation,
            "spatialPattern": spatial_pattern,
            "sensitivityTargetRegister": sensitivity_target,
            "fixture": fixture,
        }

    def uv_left(value: dict[str, Any]) -> None:
        value["carrierRows"][2][:2] = [0.18, 0.73]

    def uv_right(value: dict[str, Any]) -> None:
        value["carrierRows"][2][:2] = [0.81, 0.24]

    def uv_upper_right(value: dict[str, Any]) -> None:
        value["carrierRows"][2][:2] = [0.77, 0.83]

    def dynamic(value: dict[str, Any]) -> None:
        value["carrierRows"][4] = [0.22, 1.38, 0.71, 0.43]

    def camera(value: dict[str, Any]) -> None:
        value["carrierRows"][6] = [0.79, 0.16, 0.59, 1.0]

    def scene_depth(value: dict[str, Any]) -> None:
        value["carrierRows"][7] = [-0.32, 0.28, 0.0, 1.32]
        value["textures"][2] = coordinate_texture(19, scene_depth=True)

    def particle_color(value: dict[str, Any]) -> None:
        value["carrierRows"][3] = [0.31, 0.72, 0.96, 0.41]

    def material_curve(value: dict[str, Any]) -> None:
        value["constantBuffers"]["0"][14] = [1.3, -11.0, -4.2, 0.72]
        value["constantBuffers"]["0"][16] = [0.17, 1.6, 0.83, 0.52]

    def fog_selection(value: dict[str, Any]) -> None:
        value["carrierRows"][5] = [0.21, 0.07, 0.11, 0.64]

    def alternate_grid(value: dict[str, Any]) -> None:
        value["textures"] = [coordinate_texture(23 - index) for index in range(8)]
        value["textures"][2] = coordinate_texture(11, scene_depth=True)

    def tangent_frame(value: dict[str, Any]) -> None:
        value["carrierRows"][0] = [0.64, -0.21, 0.74, 0.0]
        value["carrierRows"][1] = [-0.31, 0.89, 0.33, 1.0]

    def scene_metadata(value: dict[str, Any]) -> None:
        value["constantBuffers"]["2"][3] = [0.71, 0.13, 0.52, 0.0]

    return [
        case("source-cb/spatial-baseline", "BASELINE", spatial_pattern=True),
        case("particle-uv/spatial-left", "SENSITIVE", uv_left, spatial_pattern=True),
        case("particle-uv/spatial-right", "SENSITIVE", uv_right, spatial_pattern=True),
        case(
            "particle-uv/spatial-upper-right",
            "SENSITIVE",
            uv_upper_right,
            spatial_pattern=True,
        ),
        case("dynamic-parameter/all-lanes", "SENSITIVE", dynamic),
        case("camera-to-particle/direction", "SENSITIVE", camera),
        case("scene-depth/projected", "SENSITIVE", scene_depth, spatial_pattern=True),
        case("particle-color/rgba", "SENSITIVE", particle_color),
        case("material-scalar/curve-inner-hole", "SENSITIVE", material_curve),
        case("fog-selection/composite", "SENSITIVE", fog_selection),
        case("textures/spatial-alternate-grid", "SENSITIVE", alternate_grid, spatial_pattern=True),
        case(
            "tangent-frame/encoded-normal",
            "SENSITIVE",
            tangent_frame,
            sensitivity_target=2,
        ),
        case(
            "scene-metadata/cb2-row3",
            "SENSITIVE",
            scene_metadata,
            sensitivity_target=3,
        ),
    ]


class GlassholeWarpReplay(GenericWarpReplay):
    def _spatial_texture(
        self,
        specification: dict[str, Any] | None = None,
        *,
        bind: int = 8,
        staging: bool = False,
    ) -> ctypes.c_void_p:
        width = 1 if specification is None else int(specification["width"])
        height = 1 if specification is None else int(specification["height"])
        pointer = ctypes.c_void_p()
        description = Texture2DDesc(
            width,
            height,
            1,
            1,
            2,
            SampleDesc(1, 0),
            3 if staging else 0,
            0 if staging else bind,
            0x20000 if staging else 0,
            0,
        )
        backing = None
        initial = None
        initial_pointer = None
        if specification is not None:
            pixels = specification["pixels"]
            flat = [float(lane) for pixel in pixels for lane in pixel]
            require(
                len(flat) == width * height * 4,
                "Glasshole02 texture fixture extent changed",
            )
            backing = ctypes.create_string_buffer(
                struct.pack("<" + "f" * len(flat), *flat)
            )
            initial = SubresourceData(
                ctypes.cast(backing, ctypes.c_void_p),
                width * 16,
                width * height * 16,
            )
            initial_pointer = ctypes.byref(initial)
        result = com_method(
            self.device,
            5,
            HRESULT,
            ctypes.POINTER(Texture2DDesc),
            ctypes.POINTER(SubresourceData),
            ctypes.POINTER(ctypes.c_void_p),
        )(
            self.device,
            ctypes.byref(description),
            initial_pointer,
            ctypes.byref(pointer),
        )
        checked(result, pointer, "CreateTexture2D(Glasshole02)")
        return pointer

    def run_glasshole(
        self,
        pixel_dxbc: bytes,
        carrier_dxbc: bytes,
        fixture: dict[str, Any],
    ) -> list[list[float]]:
        local: list[ctypes.c_void_p] = []
        render_target_count = 6
        try:
            pixel_shader = self._shader(pixel_dxbc, 15, "CreatePixelShader")
            vertex_shader = self._shader(carrier_dxbc, 12, "CreateVertexShader")
            local.extend((pixel_shader, vertex_shader))
            vertex_cb = self._buffer(fixture["carrierRows"])
            pixel_cb0 = self._buffer(fixture["constantBuffers"]["0"])
            pixel_cb2 = self._buffer(fixture["constantBuffers"]["2"])
            local.extend((vertex_cb, pixel_cb0, pixel_cb2))

            shader_resource_views = []
            for specification in fixture["textures"]:
                texture = self._spatial_texture(specification)
                local.append(texture)
                view = self._view(texture, 7, "CreateShaderResourceView")
                local.append(view)
                shader_resource_views.append(view)

            sampler_values = [self._sampler(register).value for register in range(8)]
            render_textures = []
            render_views = []
            staging_textures = []
            for _ in range(render_target_count):
                texture = self._spatial_texture(bind=0x20)
                local.append(texture)
                render_textures.append(texture)
                view = self._view(texture, 9, "CreateRenderTargetView")
                local.append(view)
                render_views.append(view)
                staging = self._spatial_texture(staging=True)
                local.append(staging)
                staging_textures.append(staging)

            com_method(self.context, 43, None, ctypes.c_void_p)(
                self.context, self.rasterizer
            )
            viewport = Viewport(0.0, 0.0, 1.0, 1.0, 0.0, 1.0)
            com_method(
                self.context, 44, None, UINT, ctypes.POINTER(Viewport)
            )(self.context, 1, ctypes.byref(viewport))
            com_method(self.context, 24, None, UINT)(self.context, 4)

            one_pointer = ctypes.c_void_p * 1
            com_method(
                self.context,
                7,
                None,
                UINT,
                UINT,
                ctypes.POINTER(ctypes.c_void_p),
            )(self.context, 0, 1, one_pointer(vertex_cb.value))
            com_method(
                self.context,
                11,
                None,
                ctypes.c_void_p,
                ctypes.c_void_p,
                UINT,
            )(self.context, vertex_shader, None, 0)

            target_type = ctypes.c_void_p * render_target_count
            com_method(
                self.context,
                33,
                None,
                UINT,
                ctypes.POINTER(ctypes.c_void_p),
                ctypes.c_void_p,
            )(
                self.context,
                render_target_count,
                target_type(*[view.value for view in render_views]),
                None,
            )
            sentinel = (ctypes.c_float * 4)(
                SENTINEL, SENTINEL, SENTINEL, SENTINEL
            )
            clear = com_method(
                self.context,
                50,
                None,
                ctypes.c_void_p,
                ctypes.POINTER(ctypes.c_float),
            )
            for view in render_views:
                clear(self.context, view, sentinel)

            three_pointers = ctypes.c_void_p * 3
            com_method(
                self.context,
                16,
                None,
                UINT,
                UINT,
                ctypes.POINTER(ctypes.c_void_p),
            )(
                self.context,
                0,
                3,
                three_pointers(pixel_cb0.value, 0, pixel_cb2.value),
            )
            com_method(
                self.context,
                9,
                None,
                ctypes.c_void_p,
                ctypes.c_void_p,
                UINT,
            )(self.context, pixel_shader, None, 0)

            pointer_array = ctypes.c_void_p * 8
            com_method(
                self.context,
                8,
                None,
                UINT,
                UINT,
                ctypes.POINTER(ctypes.c_void_p),
            )(
                self.context,
                0,
                8,
                pointer_array(*[view.value for view in shader_resource_views]),
            )
            com_method(
                self.context,
                10,
                None,
                UINT,
                UINT,
                ctypes.POINTER(ctypes.c_void_p),
            )(self.context, 0, 8, pointer_array(*sampler_values))
            com_method(self.context, 13, None, UINT, UINT)(self.context, 3, 0)

            outputs = []
            for source, staging in zip(render_textures, staging_textures):
                com_method(
                    self.context, 47, None, ctypes.c_void_p, ctypes.c_void_p
                )(self.context, staging, source)
                mapped = MappedSubresource()
                result = com_method(
                    self.context,
                    14,
                    HRESULT,
                    ctypes.c_void_p,
                    UINT,
                    UINT,
                    UINT,
                    ctypes.POINTER(MappedSubresource),
                )(
                    self.context,
                    staging,
                    0,
                    1,
                    0,
                    ctypes.byref(mapped),
                )
                checked(result, None, "Map(Glasshole02)")
                try:
                    require(mapped.RowPitch >= 16, "Glasshole02 RowPitch is invalid")
                    outputs.append(
                        list(
                            struct.unpack(
                                "<4f", ctypes.string_at(mapped.pData, 16)
                            )
                        )
                    )
                finally:
                    com_method(
                        self.context, 15, None, ctypes.c_void_p, UINT
                    )(self.context, staging, 0)
            require(
                all(math.isfinite(lane) for row in outputs for lane in row),
                "Glasshole02 WARP result is non-finite",
            )
            return outputs
        finally:
            if self.context.value:
                com_method(self.context, 110, None)(self.context)
                com_method(self.context, 111, None)(self.context)
            for pointer in reversed(local):
                release(pointer)


def output_registers(signature: list[dict[str, Any]]) -> list[int]:
    return sorted(
        row["register"]
        for row in signature
        if row["semanticName"].casefold() == "sv_target"
    )


def maximum_error(left: list[list[float]], right: list[list[float]]) -> float:
    require(len(left) == len(right) == 6, "Glasshole02 MRT extent changed")
    return max(
        abs(left[target][lane] - right[target][lane])
        for target in range(6)
        for lane in range(4)
    )


def target_errors(
    left: list[list[float]], right: list[list[float]]
) -> dict[str, float]:
    return {
        f"rt{target}": max(
            abs(left[target][lane] - right[target][lane]) for lane in range(4)
        )
        for target in EXPECTED_MRT_REGISTERS
    }


def build_receipt(
    raw_dxbc_path: Path,
    include_path: Path,
    shader_path: Path,
    uniform_receipt_path: Path,
    compiler: Path,
) -> dict[str, Any]:
    raw_dxbc = raw_dxbc_path.read_bytes()
    require(len(raw_dxbc) == EXPECTED_RAW_DXBC_BYTE_SIZE, "raw DXBC size changed")
    require(sha256_bytes(raw_dxbc) == EXPECTED_RAW_DXBC_SHA256, "raw DXBC changed")
    validate_dxbc_container(raw_dxbc)

    translated_dxbc = compile_pixel_shader(shader_path, compiler)
    validate_dxbc_container(translated_dxbc)
    disassembler = D3DDisassembler(compiler)
    raw_disassembly = disassembler.disassemble(raw_dxbc)
    translated_disassembly = disassembler.disassemble(translated_dxbc)
    require(
        raw_disassembly["instructionCount"] == EXPECTED_RAW_INSTRUCTION_COUNT,
        "raw Glasshole02 instruction denominator changed",
    )

    raw_chunks = dxbc_chunks(raw_dxbc)
    translated_chunks = dxbc_chunks(translated_dxbc)
    raw_input = parse_signature(raw_chunks["ISGN"])
    translated_input = parse_signature(translated_chunks["ISGN"])
    raw_output = parse_signature(raw_chunks["OSGN"])
    translated_output = parse_signature(translated_chunks["OSGN"])
    require(raw_input == translated_input, "translated pixel input signature changed")
    require(raw_output == translated_output, "translated pixel output signature changed")
    require(
        output_registers(raw_output) == EXPECTED_MRT_REGISTERS,
        "Glasshole02 MRT signature changed",
    )

    raw_declarations = parse_runtime_declarations(raw_disassembly)
    translated_declarations = parse_runtime_declarations(translated_disassembly)
    require(raw_declarations == translated_declarations, "translated register ABI changed")
    require(
        raw_declarations["constantBufferFloat4Counts"] == EXPECTED_CB_COUNTS,
        "Glasshole02 constant-buffer ABI changed",
    )
    require(
        raw_declarations["textureRegisters"] == EXPECTED_TEXTURE_REGISTERS
        and raw_declarations["samplerRegisters"] == EXPECTED_SAMPLER_REGISTERS,
        "Glasshole02 texture/sampler ABI changed",
    )
    for declaration in (
        "dcl_input_ps linear centroid v6.xyz",
        "dcl_input_ps linear centroid v7.xyw",
    ):
        require(
            declaration in raw_disassembly["declarations"]
            and declaration in translated_disassembly["declarations"],
            f"Glasshole02 centroid interpolation changed: {declaration}",
        )

    carrier_source, carrier_row_count = build_carrier_source(raw_input)
    require(carrier_row_count == 8, "Glasshole02 carrier register extent changed")
    carrier_compiler = CarrierCompiler(compiler)
    carrier_dxbc, carrier_identity = carrier_compiler.compile(
        carrier_source, "ue3-glasshole02-translation-carrier"
    )
    carrier_output = parse_signature(dxbc_chunks(carrier_dxbc)["OSGN"])
    carrier_closure = close_carrier_signature(raw_input, carrier_output)

    uniform_receipt = read_json(uniform_receipt_path)
    uniform_evaluator.validate_output_receipt(uniform_receipt)
    cases = []
    baseline_raw = None
    runner = GlassholeWarpReplay()
    try:
        for specification in translation_cases(uniform_receipt):
            fixture = specification["fixture"]
            raw_outputs = runner.run_glasshole(raw_dxbc, carrier_dxbc, fixture)
            translated_outputs = runner.run_glasshole(
                translated_dxbc, carrier_dxbc, fixture
            )
            require(
                raw_outputs[1] == [SENTINEL] * 4
                and translated_outputs[1] == [SENTINEL] * 4,
                "undeclared RT1 was modified",
            )
            error = maximum_error(raw_outputs, translated_outputs)
            require(
                error <= FLOAT_TOLERANCE,
                f"{specification['caseId']} translation mismatch: {error}",
            )
            if specification["expectedRelation"] == "BASELINE":
                baseline_raw = raw_outputs
                baseline_delta = 0.0
            else:
                require(baseline_raw is not None, "translation baseline is absent")
                target_register = specification["sensitivityTargetRegister"]
                baseline_delta = max(
                    abs(
                        raw_outputs[target_register][lane]
                        - baseline_raw[target_register][lane]
                    )
                    for lane in range(4)
                )
                require(
                    baseline_delta > SENSITIVITY_THRESHOLD,
                    f"{specification['caseId']} did not exercise RT0",
                )
            cases.append(
                {
                    "caseId": specification["caseId"],
                    "expectedRelation": specification["expectedRelation"],
                    "spatialPattern": specification["spatialPattern"],
                    "sensitivityTargetRegister": specification[
                        "sensitivityTargetRegister"
                    ],
                    "rawDxbcMrt": raw_outputs,
                    "translatedHlslMrt": translated_outputs,
                    "targetMaximumAbsoluteError": target_errors(
                        raw_outputs, translated_outputs
                    ),
                    "maximumAbsoluteError": error,
                    "rawTargetVersusBaselineMaximumDelta": baseline_delta,
                    "comparisonAdmission": True,
                }
            )
    finally:
        runner.close()

    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "target": {
            "targetId": TARGET_ID,
            "familyId": FAMILY_ID,
            "occurrenceId": OCCURRENCE_ID,
            "rendererType": "SpriteParticle",
            "sourceParentMaterial": "fx_m_mi_j_00.fx_m.fx_j_pa_glasshole_02_tr",
        },
        "inputs": {
            "rawPixelDxbc": descriptor(
                raw_dxbc_path, "SOURCE_EXACT_COOKED_PIXEL_SHADER_NUMERIC_ORACLE"
            ),
            "translatedHlslInclude": descriptor(
                include_path, "READABLE_GLASSHOLE02_EQUATION_TRANSLATION"
            ),
            "translatedHlslWrapper": descriptor(
                shader_path, "D3DCOMPILE_PS_MAIN_WRAPPER"
            ),
            "sourceUniformReceipt": {
                **descriptor(
                    uniform_receipt_path,
                    "SOURCE_VALUE_CB0_MATERIAL_ROWS_PREREQUISITE",
                ),
                "receiptSha256": uniform_receipt["receiptSha256"],
            },
        },
        "shaderAbi": {
            "profile": "ps_5_0",
            "entry": "PS_MAIN",
            "rawInstructionCount": raw_disassembly["instructionCount"],
            "rawInstructionSha256": raw_disassembly["instructionSha256"],
            "translatedInstructionCount": translated_disassembly[
                "instructionCount"
            ],
            "translatedInstructionSha256": translated_disassembly[
                "instructionSha256"
            ],
            "translatedDxbcByteSize": len(translated_dxbc),
            "translatedDxbcSha256": sha256_bytes(translated_dxbc),
            "constantBufferFloat4Counts": EXPECTED_CB_COUNTS,
            "textureRegisters": EXPECTED_TEXTURE_REGISTERS,
            "samplerRegisters": EXPECTED_SAMPLER_REGISTERS,
            "inputSignature": public_signature(raw_input),
            "outputSignature": public_signature(raw_output),
            "centroidInputRegisters": [6, 7],
            "declaredMrtRegisters": EXPECTED_MRT_REGISTERS,
            "carrier": {
                **carrier_identity,
                "constantBufferFloat4Count": carrier_row_count,
                "signatureClosure": carrier_closure,
            },
            "registerAndSignatureParityAdmission": True,
        },
        "warpComparison": {
            "backend": "D3D11_WARP_RAW_DXBC_VERSUS_COMPILED_TRANSLATED_HLSL",
            "featureLevel": f"0x{runner.feature_level:04x}",
            "compiler": external_binary_identity(compiler),
            "samplerPolicy": "POINT_CLAMP_COMMON_NUMERIC_FIXTURE_ONLY",
            "sourceExactSamplerPolicyAdmission": False,
            "textureExtent": [TEXTURE_EXTENT, TEXTURE_EXTENT],
            "constantBufferInputs": ["CB0[22]", "CB2[4]"],
            "textureInputs": [f"t{index}/s{index}" for index in range(8)],
            "caseCount": len(cases),
            "spatialPatternCaseCount": sum(row["spatialPattern"] for row in cases),
            "tolerance": FLOAT_TOLERANCE,
            "maximumAbsoluteError": max(
                row["maximumAbsoluteError"] for row in cases
            ),
            "cases": cases,
            "fixedFixtureEquationParityAdmission": True,
        },
        "decision": {
            "rawDxbcNumericOracle": True,
            "rawDxbcProductExecution": False,
            "translatedHlslCompiled": True,
            "translatedHlslFixedFixtureEquationParity": True,
            "translatedHlslRuntimeAdmission": False,
            "translatedHlslAuthoringCanaryAdmission": False,
            "sourceExactSamplerAdmission": False,
            "sourceExactVertexConstantBufferAdmission": False,
            "productAdmission": False,
            "visualAdmission": False,
            "nextGate": "TYPED_RENDERER_CB2_SCENE_DEPTH_STATE_AND_SINGLE_AUTHORING_CANARY",
        },
    }
    seal(receipt)
    return receipt


def validate_receipt(receipt: dict[str, Any]) -> None:
    require(receipt.get("schema") == SCHEMA, "translation receipt schema changed")
    require(
        receipt.get("formatVersion") == FORMAT_VERSION,
        "translation receipt format changed",
    )
    claimed = receipt.get("receiptSha256")
    unsigned = dict(receipt)
    unsigned.pop("receiptSha256", None)
    require(claimed == canonical_json_sha256(unsigned), "translation receipt seal changed")

    target = receipt["target"]
    require(
        target["targetId"] == TARGET_ID
        and target["familyId"] == FAMILY_ID
        and target["occurrenceId"] == OCCURRENCE_ID,
        "translation target identity changed",
    )
    inputs = receipt["inputs"]
    require(
        inputs["rawPixelDxbc"] == descriptor(
            DEFAULT_RAW_DXBC, "SOURCE_EXACT_COOKED_PIXEL_SHADER_NUMERIC_ORACLE"
        ),
        "raw Glasshole02 input descriptor changed",
    )
    require(
        inputs["translatedHlslInclude"] == descriptor(
            DEFAULT_INCLUDE, "READABLE_GLASSHOLE02_EQUATION_TRANSLATION"
        ),
        "Glasshole02 translated include is stale",
    )
    require(
        inputs["translatedHlslWrapper"] == descriptor(
            DEFAULT_SHADER, "D3DCOMPILE_PS_MAIN_WRAPPER"
        ),
        "Glasshole02 translated wrapper is stale",
    )
    uniform_receipt = read_json(DEFAULT_UNIFORM_RECEIPT)
    uniform_evaluator.validate_output_receipt(uniform_receipt)
    require(
        inputs["sourceUniformReceipt"]["receiptSha256"]
        == uniform_receipt["receiptSha256"],
        "Glasshole02 source uniform prerequisite changed",
    )

    abi = receipt["shaderAbi"]
    require(
        abi["rawInstructionCount"] == EXPECTED_RAW_INSTRUCTION_COUNT,
        "raw instruction denominator changed",
    )
    require(
        abi["constantBufferFloat4Counts"] == EXPECTED_CB_COUNTS
        and abi["textureRegisters"] == EXPECTED_TEXTURE_REGISTERS
        and abi["samplerRegisters"] == EXPECTED_SAMPLER_REGISTERS,
        "translation register ABI changed",
    )
    require(
        abi["centroidInputRegisters"] == [6, 7]
        and abi["declaredMrtRegisters"] == EXPECTED_MRT_REGISTERS,
        "translation interpolation or MRT ABI changed",
    )
    require(
        abi["carrier"]["constantBufferFloat4Count"] == 8
        and abi["carrier"]["signatureClosure"]["pass"]
        and abi["registerAndSignatureParityAdmission"],
        "translation carrier/signature closure failed",
    )

    warp = receipt["warpComparison"]
    require(warp["caseCount"] == 13, "translation case denominator changed")
    require(
        warp["spatialPatternCaseCount"] >= 4,
        "translation spatial-pattern denominator changed",
    )
    require(warp["textureExtent"] == [4, 4], "translation texture extent changed")
    require(warp["tolerance"] == FLOAT_TOLERANCE, "translation tolerance changed")
    require(
        warp["maximumAbsoluteError"] <= FLOAT_TOLERANCE
        and warp["fixedFixtureEquationParityAdmission"]
        and not warp["sourceExactSamplerPolicyAdmission"],
        "translation numeric boundary changed",
    )
    require(
        [row["caseId"] for row in warp["cases"]]
        == [row["caseId"] for row in translation_cases(uniform_receipt)],
        "translation case order changed",
    )
    for row in warp["cases"]:
        require(
            row["comparisonAdmission"]
            and row["maximumAbsoluteError"] <= FLOAT_TOLERANCE,
            f"translation case failed: {row['caseId']}",
        )
        require(
            row["rawDxbcMrt"][1] == [SENTINEL] * 4
            and row["translatedHlslMrt"][1] == [SENTINEL] * 4,
            "translation RT1 sentinel changed",
        )
        if row["expectedRelation"] == "SENSITIVE":
            require(
                row["rawTargetVersusBaselineMaximumDelta"]
                > SENSITIVITY_THRESHOLD,
                f"translation sensitivity collapsed: {row['caseId']}",
            )

    decision = receipt["decision"]
    require(
        decision["rawDxbcNumericOracle"]
        and not decision["rawDxbcProductExecution"]
        and decision["translatedHlslCompiled"]
        and decision["translatedHlslFixedFixtureEquationParity"],
        "translation oracle/parity decision changed",
    )
    for key in (
        "translatedHlslRuntimeAdmission",
        "translatedHlslAuthoringCanaryAdmission",
        "sourceExactSamplerAdmission",
        "sourceExactVertexConstantBufferAdmission",
        "productAdmission",
        "visualAdmission",
    ):
        require(not decision[key], f"translation receipt overclaims {key}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Replay Glasshole02 raw DXBC against translated HLSL on WARP."
    )
    parser.add_argument("--raw-dxbc", type=Path, default=DEFAULT_RAW_DXBC)
    parser.add_argument("--include", type=Path, default=DEFAULT_INCLUDE)
    parser.add_argument("--shader", type=Path, default=DEFAULT_SHADER)
    parser.add_argument(
        "--uniform-receipt", type=Path, default=DEFAULT_UNIFORM_RECEIPT
    )
    parser.add_argument("--compiler", type=Path, default=DEFAULT_D3DCOMPILER)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--validate-only", action="store_true")
    args = parser.parse_args()

    if args.validate_only:
        receipt = read_json(args.output)
        validate_receipt(receipt)
        print("Glasshole02 translated HLSL receipt PASS")
        return 0

    receipt = build_receipt(
        args.raw_dxbc,
        args.include,
        args.shader,
        args.uniform_receipt,
        args.compiler,
    )
    validate_receipt(receipt)
    encoded = json.dumps(receipt, ensure_ascii=False, indent=2) + "\n"
    if args.check:
        require(args.output.is_file(), f"translation receipt is missing: {args.output}")
        require(
            args.output.read_text(encoding="utf-8-sig") == encoded,
            "translation receipt is stale",
        )
    else:
        write_json_atomic(args.output, receipt)
    print(
        "Glasshole02 translated HLSL WARP parity PASS: "
        f"cases={receipt['warpComparison']['caseCount']} "
        f"maxError={receipt['warpComparison']['maximumAbsoluteError']:.9g} "
        "runtime=false product=false visual=false"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
