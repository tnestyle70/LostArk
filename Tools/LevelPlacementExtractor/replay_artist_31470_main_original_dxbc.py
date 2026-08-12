#!/usr/bin/env python3
"""Replay the two Artist F main original pixel shaders through D3D11 WARP.

The pixel shaders are read byte-for-byte from the sealed RefShaderCache receipt
and passed directly to CreatePixelShader.  Only a small carrier vertex shader
is compiled locally.  A separately reviewed opacity SSA slice computes the CPU
oracle, so the replay is not a round-trip through the extraction parser.
"""

from __future__ import annotations

import argparse
import copy
import ctypes
import hashlib
import json
import math
import os
import struct
from pathlib import Path
from typing import Any

from extract_artist_31470_main_ref_shader_cache import (
    BASE_PASS_PIXEL,
    canonical_json_sha256,
    read_json,
    validate_receipt as validate_shader_cache_receipt,
)


SCHEMA = "lostark.artist-31470-main-original-dxbc-replay-receipt"
FORMAT_VERSION = 1
REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = Path(__file__).resolve()
DEFAULT_INPUT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.main-ref-shader-cache.receipt.json"
)
DEFAULT_OUTPUT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.main-original-dxbc-replay.receipt.json"
)
DEFAULT_COMPILER = Path(
    r"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\d3dcompiler_47.dll"
)
DEFAULT_D3D11 = Path(os.environ.get("WINDIR", r"C:\Windows")) / "System32/d3d11.dll"
DEFAULT_WARP = Path(os.environ.get("WINDIR", r"C:\Windows")) / "System32/d3d10warp.dll"

EXPECTED_BINARIES = {
    "d3dcompiler_47.dll": {
        "byteSize": 4_916_800,
        "sha256": "ce013eb1639f8e2620a509e73b33029108f55a293e304e33e38b72fd65c531b8",
    },
    "d3d11.dll": {
        "byteSize": 2_476_552,
        "sha256": "697be4db1e5e3106c61f63363d82cd2ff2a922bb2887a5fd6e23234620b7904a",
    },
    "d3d10warp.dll": {
        "byteSize": 7_434_960,
        "sha256": "9ffc32cfd7a883a4f0074fc22c842190cc87b3d8630ac5650b4e59803bbc3f90",
    },
}
PS_IDENTITIES = {
    "#9/#10_WATERTRAIL": {
        "shaderIdHex": "70bf2a6e9bf4f0478cecbfc43c4e160f",
        "dxbcSha256": "b16e274cfad5ba27b3be0f8c8bb4c1e663768ded79d4ddf119204fb8a1e9c6bb",
    },
    "#11_SPRITEWAVE": {
        "shaderIdHex": "39f7e63594b10f4a9237dc9eb19a1dfc",
        "dxbcSha256": "7e8dbb706620c5ec6d991d99c70d6daa6b9df2258060796597c0678358b4f5e0",
    },
}
FEATURE_LEVEL_11_0 = 0xB000
FLOAT_TOLERANCE = 1.0e-6


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(8 * 1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def canonical_text_sha256(path: Path) -> str:
    text = path.read_text(encoding="utf-8-sig")
    return sha256_bytes(
        text.replace("\r\n", "\n").replace("\r", "\n").encode("utf-8")
    )


def seal(receipt: dict[str, Any]) -> None:
    receipt.pop("receiptSha256", None)
    receipt["receiptSha256"] = canonical_json_sha256(receipt)


HRESULT = ctypes.c_long
UINT = ctypes.c_uint32


class BufferDesc(ctypes.Structure):
    _fields_ = [
        ("ByteWidth", UINT),
        ("Usage", UINT),
        ("BindFlags", UINT),
        ("CPUAccessFlags", UINT),
        ("MiscFlags", UINT),
        ("StructureByteStride", UINT),
    ]


class SampleDesc(ctypes.Structure):
    _fields_ = [("Count", UINT), ("Quality", UINT)]


class Texture2DDesc(ctypes.Structure):
    _fields_ = [
        ("Width", UINT),
        ("Height", UINT),
        ("MipLevels", UINT),
        ("ArraySize", UINT),
        ("Format", UINT),
        ("SampleDesc", SampleDesc),
        ("Usage", UINT),
        ("BindFlags", UINT),
        ("CPUAccessFlags", UINT),
        ("MiscFlags", UINT),
    ]


class SubresourceData(ctypes.Structure):
    _fields_ = [
        ("pSysMem", ctypes.c_void_p),
        ("SysMemPitch", UINT),
        ("SysMemSlicePitch", UINT),
    ]


class MappedSubresource(ctypes.Structure):
    _fields_ = [
        ("pData", ctypes.c_void_p),
        ("RowPitch", UINT),
        ("DepthPitch", UINT),
    ]


class Viewport(ctypes.Structure):
    _fields_ = [
        ("TopLeftX", ctypes.c_float),
        ("TopLeftY", ctypes.c_float),
        ("Width", ctypes.c_float),
        ("Height", ctypes.c_float),
        ("MinDepth", ctypes.c_float),
        ("MaxDepth", ctypes.c_float),
    ]


class SamplerDesc(ctypes.Structure):
    _fields_ = [
        ("Filter", UINT),
        ("AddressU", UINT),
        ("AddressV", UINT),
        ("AddressW", UINT),
        ("MipLODBias", ctypes.c_float),
        ("MaxAnisotropy", UINT),
        ("ComparisonFunc", UINT),
        ("BorderColor", ctypes.c_float * 4),
        ("MinLOD", ctypes.c_float),
        ("MaxLOD", ctypes.c_float),
    ]


class RasterizerDesc(ctypes.Structure):
    _fields_ = [
        ("FillMode", UINT),
        ("CullMode", UINT),
        ("FrontCounterClockwise", ctypes.c_int32),
        ("DepthBias", ctypes.c_int32),
        ("DepthBiasClamp", ctypes.c_float),
        ("SlopeScaledDepthBias", ctypes.c_float),
        ("DepthClipEnable", ctypes.c_int32),
        ("ScissorEnable", ctypes.c_int32),
        ("MultisampleEnable", ctypes.c_int32),
        ("AntialiasedLineEnable", ctypes.c_int32),
    ]


def com_method(
    pointer: ctypes.c_void_p,
    index: int,
    result_type: Any,
    *argument_types: Any,
) -> Any:
    table = ctypes.cast(
        pointer, ctypes.POINTER(ctypes.POINTER(ctypes.c_void_p))
    ).contents
    return ctypes.WINFUNCTYPE(
        result_type, ctypes.c_void_p, *argument_types
    )(table[index])


def release(pointer: ctypes.c_void_p) -> None:
    if pointer and pointer.value:
        com_method(pointer, 2, ctypes.c_ulong)(pointer)
        pointer.value = None


def checked(result: int, pointer: ctypes.c_void_p | None, label: str) -> None:
    require(result >= 0, f"{label} failed: 0x{result & 0xFFFFFFFF:08X}")
    if pointer is not None:
        require(bool(pointer.value), f"{label} returned a null object")


def blob_bytes(pointer: ctypes.c_void_p) -> bytes:
    get_pointer = com_method(pointer, 3, ctypes.c_void_p)
    get_size = com_method(pointer, 4, ctypes.c_size_t)
    return ctypes.string_at(get_pointer(pointer), get_size(pointer))


CARRIER_HLSL = b"""cbuffer Carrier:register(b0){float4 k[8];}
struct O{
 float3 a:TEXCOORD10; float4 b:TEXCOORD11; float4 c:COLOR0; float2 d:COLOR1;
 float4 e:TEXCOORD0; float4 f:TEXCOORD4; float4 g:TEXCOORD6; float4 h:TEXCOORD5;
 float4 p:SV_Position;
};
O main(uint id:SV_VertexID){
 float2 q[3]={float2(-1,-1),float2(-1,3),float2(3,-1)}; O o;
 o.a=k[0].xyz; o.b=k[1]; o.c=k[2]; o.d=k[3].xy;
 o.e=k[4]; o.f=k[5]; o.g=k[6]; o.h=k[7]; o.p=float4(q[id],0,1); return o;
}"""


def external_binary_identity(path: Path) -> dict[str, Any]:
    require(path.is_file(), f"external replay binary is missing: {path}")
    expected = EXPECTED_BINARIES[path.name.casefold()]
    require(path.stat().st_size == expected["byteSize"], f"{path.name} size changed")
    digest = sha256_file(path)
    require(digest == expected["sha256"], f"{path.name} SHA changed")
    return {
        "fileName": path.name,
        "physicalByteSize": path.stat().st_size,
        "rawSha256": digest,
        "hashRole": "EXTERNAL_RAW_BINARY_BYTES",
    }


def compile_carrier(path: Path) -> tuple[bytes, dict[str, Any]]:
    identity = external_binary_identity(path)
    dll = ctypes.WinDLL(str(path))
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
    source = ctypes.create_string_buffer(CARRIER_HLSL)
    code = ctypes.c_void_p()
    errors = ctypes.c_void_p()
    try:
        result = function(
            source,
            len(CARRIER_HLSL),
            b"artist31470-carrier-v0-v7-position-last",
            None,
            None,
            b"main",
            b"vs_5_0",
            0,
            0,
            ctypes.byref(code),
            ctypes.byref(errors),
        )
        error_text = (
            blob_bytes(errors).decode("utf-8", "replace") if errors.value else ""
        )
        require(
            result >= 0 and code.value,
            f"carrier D3DCompile failed: 0x{result & 0xFFFFFFFF:08X} {error_text}",
        )
        bytecode = blob_bytes(code)
    finally:
        release(errors)
        release(code)
    return bytecode, {
        "sourceSha256": sha256_bytes(CARRIER_HLSL),
        "compiledDxbcSha256": sha256_bytes(bytecode),
        "compiler": identity,
        "linkageContract": "VS_OUTPUT_REGISTERS_V0_THROUGH_V7_IN_DECLARATION_ORDER_WITH_SV_POSITION_LAST",
    }


def create_warp_device() -> tuple[ctypes.c_void_p, ctypes.c_void_p, int]:
    function = ctypes.WinDLL("d3d11").D3D11CreateDevice
    function.restype = HRESULT
    function.argtypes = [
        ctypes.c_void_p,
        UINT,
        ctypes.c_void_p,
        UINT,
        ctypes.c_void_p,
        UINT,
        UINT,
        ctypes.POINTER(ctypes.c_void_p),
        ctypes.POINTER(UINT),
        ctypes.POINTER(ctypes.c_void_p),
    ]
    device = ctypes.c_void_p()
    context = ctypes.c_void_p()
    feature_level = UINT()
    result = function(
        None,
        5,
        None,
        0,
        None,
        0,
        7,
        ctypes.byref(device),
        ctypes.byref(feature_level),
        ctypes.byref(context),
    )
    checked(result, device, "D3D11CreateDevice(WARP)")
    require(context.value, "D3D11CreateDevice returned a null context")
    require(feature_level.value == FEATURE_LEVEL_11_0, "WARP feature level changed")
    return device, context, int(feature_level.value)


class WarpPixelReplay:
    def __init__(self, compiler_path: Path) -> None:
        self.device, self.context, self.feature_level = create_warp_device()
        self.permanent: list[ctypes.c_void_p] = []
        self.carrier_dxbc, self.carrier_identity = compile_carrier(compiler_path)
        self.vertex_shader = self._shader(self.carrier_dxbc, 12, "CreateVertexShader")
        self.permanent.append(self.vertex_shader)
        raster = RasterizerDesc(3, 1, 0, 0, 0.0, 0.0, 1, 0, 0, 0)
        self.rasterizer = ctypes.c_void_p()
        result = com_method(
            self.device,
            22,
            HRESULT,
            ctypes.POINTER(RasterizerDesc),
            ctypes.POINTER(ctypes.c_void_p),
        )(self.device, ctypes.byref(raster), ctypes.byref(self.rasterizer))
        checked(result, self.rasterizer, "CreateRasterizerState")
        self.permanent.append(self.rasterizer)
        self.samplers: list[ctypes.c_void_p] = []
        for _ in range(4):
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
                self.device,
                23,
                HRESULT,
                ctypes.POINTER(SamplerDesc),
                ctypes.POINTER(ctypes.c_void_p),
            )(self.device, ctypes.byref(description), ctypes.byref(sampler))
            checked(result, sampler, "CreateSamplerState")
            self.samplers.append(sampler)
            self.permanent.append(sampler)

    def _shader(self, data: bytes, method_index: int, label: str) -> ctypes.c_void_p:
        result_pointer = ctypes.c_void_p()
        backing = ctypes.create_string_buffer(data)
        result = com_method(
            self.device,
            method_index,
            HRESULT,
            ctypes.c_void_p,
            ctypes.c_size_t,
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_void_p),
        )(
            self.device,
            backing,
            len(data),
            None,
            ctypes.byref(result_pointer),
        )
        checked(result, result_pointer, label)
        return result_pointer

    def _buffer(self, raw: bytes) -> ctypes.c_void_p:
        require(raw and len(raw) % 16 == 0, "constant buffer size is invalid")
        result_pointer = ctypes.c_void_p()
        backing = ctypes.create_string_buffer(raw)
        initial = SubresourceData(ctypes.cast(backing, ctypes.c_void_p), 0, 0)
        description = BufferDesc(len(raw), 0, 4, 0, 0, 0)
        result = com_method(
            self.device,
            3,
            HRESULT,
            ctypes.POINTER(BufferDesc),
            ctypes.POINTER(SubresourceData),
            ctypes.POINTER(ctypes.c_void_p),
        )(
            self.device,
            ctypes.byref(description),
            ctypes.byref(initial),
            ctypes.byref(result_pointer),
        )
        checked(result, result_pointer, "CreateBuffer")
        return result_pointer

    def _texture(
        self,
        width: int = 1,
        height: int = 1,
        pixels: list[list[float]] | None = None,
        *,
        bind: int = 8,
        staging: bool = False,
    ) -> ctypes.c_void_p:
        result_pointer = ctypes.c_void_p()
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
        if pixels is not None:
            flat = [float(value) for pixel in pixels for value in pixel]
            require(len(flat) == width * height * 4, "texture initial extent changed")
            backing = ctypes.create_string_buffer(
                struct.pack("<" + "f" * len(flat), *flat)
            )
            initial = SubresourceData(
                ctypes.cast(backing, ctypes.c_void_p), width * 16, width * height * 16
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
            ctypes.byref(result_pointer),
        )
        checked(result, result_pointer, "CreateTexture2D")
        return result_pointer

    def _view(
        self, resource: ctypes.c_void_p, method_index: int, label: str
    ) -> ctypes.c_void_p:
        result_pointer = ctypes.c_void_p()
        result = com_method(
            self.device,
            method_index,
            HRESULT,
            ctypes.c_void_p,
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_void_p),
        )(
            self.device,
            resource,
            None,
            ctypes.byref(result_pointer),
        )
        checked(result, result_pointer, label)
        return result_pointer

    def run(
        self,
        pixel_dxbc: bytes,
        cb0: list[list[float]],
        carrier: list[list[float]],
        textures: list[dict[str, Any]],
        *,
        render_target_count: int = 6,
    ) -> list[list[float]]:
        require(render_target_count in (1, 6), "render-target control count is invalid")
        local: list[ctypes.c_void_p] = []
        try:
            pixel_shader = self._shader(pixel_dxbc, 15, "CreatePixelShader")
            local.append(pixel_shader)

            def packed(rows: list[list[float]]) -> bytes:
                return struct.pack(
                    "<" + "f" * (4 * len(rows)),
                    *(value for row in rows for value in row),
                )

            pixel_cb0 = self._buffer(packed(cb0))
            local.append(pixel_cb0)
            pixel_cb2 = self._buffer(bytes(64))
            local.append(pixel_cb2)
            vertex_cb0 = self._buffer(packed(carrier))
            local.append(vertex_cb0)

            shader_resource_views: list[ctypes.c_void_p] = []
            for specification in textures:
                texture = self._texture(
                    int(specification["width"]),
                    int(specification["height"]),
                    specification["pixels"],
                )
                local.append(texture)
                view = self._view(texture, 7, "CreateShaderResourceView")
                local.append(view)
                shader_resource_views.append(view)

            render_textures: list[ctypes.c_void_p] = []
            render_views: list[ctypes.c_void_p] = []
            staging_textures: list[ctypes.c_void_p] = []
            for _ in range(render_target_count):
                texture = self._texture(bind=0x20)
                local.append(texture)
                render_textures.append(texture)
                view = self._view(texture, 9, "CreateRenderTargetView")
                local.append(view)
                render_views.append(view)
                staging = self._texture(staging=True)
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
            )(self.context, 0, 1, one_pointer(vertex_cb0.value))
            com_method(
                self.context,
                11,
                None,
                ctypes.c_void_p,
                ctypes.c_void_p,
                UINT,
            )(self.context, self.vertex_shader, None, 0)

            target_array_type = ctypes.c_void_p * render_target_count
            target_array = target_array_type(
                *[view.value for view in render_views]
            )
            com_method(
                self.context,
                33,
                None,
                UINT,
                ctypes.POINTER(ctypes.c_void_p),
                ctypes.c_void_p,
            )(
                self.context, render_target_count, target_array, None
            )
            sentinel = (ctypes.c_float * 4)(-99.0, -99.0, -99.0, -99.0)
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

            resource_count = len(shader_resource_views)
            pointer_array_type = ctypes.c_void_p * resource_count
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
                resource_count,
                pointer_array_type(
                    *[view.value for view in shader_resource_views]
                ),
            )
            com_method(
                self.context,
                10,
                None,
                UINT,
                UINT,
                ctypes.POINTER(ctypes.c_void_p),
            )(
                self.context,
                0,
                resource_count,
                pointer_array_type(
                    *[sampler.value for sampler in self.samplers[:resource_count]]
                ),
            )
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
                checked(result, None, "Map")
                try:
                    require(mapped.RowPitch >= 16, "staging RowPitch is invalid")
                    outputs.append(
                        list(struct.unpack("<4f", ctypes.string_at(mapped.pData, 16)))
                    )
                finally:
                    com_method(
                        self.context, 15, None, ctypes.c_void_p, UINT
                    )(self.context, staging, 0)
            require(
                all(math.isfinite(value) for row in outputs for value in row),
                "WARP replay produced a non-finite lane",
            )
            return outputs
        finally:
            if self.context.value:
                com_method(self.context, 110, None)(self.context)
                com_method(self.context, 111, None)(self.context)
            for pointer in reversed(local):
                release(pointer)

    def close(self) -> None:
        if self.context.value:
            com_method(self.context, 110, None)(self.context)
            com_method(self.context, 111, None)(self.context)
        for pointer in reversed(self.permanent):
            release(pointer)
        release(self.context)
        release(self.device)


def float4_rows(count: int) -> list[list[float]]:
    return [[0.0, 0.0, 0.0, 0.0] for _ in range(count)]


def texture(r: float, g: float, b: float, a: float) -> dict[str, Any]:
    return {"width": 1, "height": 1, "pixels": [[r, g, b, a]]}


def saturate(value: float) -> float:
    return min(max(value, 0.0), 1.0)


def base_fixtures() -> tuple[
    list[list[float]],
    list[list[float]],
    list[dict[str, Any]],
    list[list[float]],
    list[dict[str, Any]],
]:
    carrier = float4_rows(8)
    carrier[0] = [0.0, 0.0, 1.0, 0.0]
    carrier[1] = [1.0, 0.0, 0.0, 1.0]
    carrier[4] = [0.5, 0.5, 0.0, 0.0]
    carrier[5] = [0.0, 0.0, 0.0, 1.0]
    carrier[6] = [0.0, 0.8, 0.6, 0.0]

    water = float4_rows(16)
    water[0] = [0.8, 0.0, 0.0, 0.0]
    water[1] = [1.0, 1.0, 1.0, 0.75]
    water[4] = [1.0, 0.0, 0.0, 0.0]
    water[5] = [0.0, 1.0, 0.0, 0.0]
    water[12] = [0.0, 0.0, 0.0, 1.0]
    water[13] = [1.0, 0.0, 0.0, 0.0]
    water[15] = [1.2, 12.0, 2.0, 1.1]
    water_textures = [
        texture(0.8, 0.2, 0.3, 0.4),
        texture(0.5, 0.5, 0.0, 1.0),
        texture(0.9, 0.4, 0.2, 1.0),
    ]

    sprite = float4_rows(24)
    sprite[0] = [0.8, 0.0, 0.0, 0.0]
    sprite[1] = [1.0, 1.0, 1.0, 0.7]
    sprite[3] = [0.0, 1.0, 0.0, 0.0]
    sprite[4] = [1.0, 0.0, 0.0, 0.0]
    sprite[5] = [0.0, 1.0, 0.0, 0.0]
    sprite[8] = [1.0, 0.0, 0.0, 0.0]
    sprite[9] = [0.0, 1.0, 0.0, 0.0]
    sprite[10] = [1.0, 0.0, 0.0, 0.0]
    sprite[11] = [0.0, 1.0, 0.0, 0.0]
    sprite[12] = [1.0, 1.0, 1.0, 0.0]
    sprite[14] = [1.0, 1.0, 0.0, 0.0]
    sprite[17] = [0.0, 0.0, 0.0, 1.0]
    sprite[18] = [1.0, 0.0, 0.0, 0.0]
    sprite[19] = [1.0, 0.0, 0.0, 0.0]
    sprite[21] = [0.0, 0.0, 1.0, 0.0]
    sprite[22] = [0.0, 0.0, 0.8, 2.0]
    sprite[23] = [0.6, 0.9, 0.1, 0.9]
    sprite_textures = [
        texture(0.2, 0.1, 0.1, 1.0),
        texture(0.7, 0.1, 0.1, 1.0),
        texture(0.3, 0.1, 0.1, 1.0),
        texture(0.4, 0.1, 0.1, 1.0),
    ]
    return carrier, water, water_textures, sprite, sprite_textures


def mutate(
    base: dict[str, Any],
    case_id: str,
    expected: float,
    *,
    cb: dict[tuple[int, int], float] | None = None,
    carrier: dict[tuple[int, int], float] | None = None,
    texture_values: dict[tuple[int, int], float] | None = None,
) -> dict[str, Any]:
    result = copy.deepcopy(base)
    result["caseId"] = case_id
    result["expectedAlpha"] = expected
    for (row, lane), value in (cb or {}).items():
        result["cb0"][row][lane] = value
    for (row, lane), value in (carrier or {}).items():
        result["carrier"][row][lane] = value
    for (slot, lane), value in (texture_values or {}).items():
        result["textures"][slot]["pixels"][0][lane] = value
    return result


def water_opacity_oracle(fixture: dict[str, Any]) -> float:
    cb0 = fixture["cb0"]
    carrier = fixture["carrier"]
    textures = fixture["textures"]
    direction = carrier[6][:3]
    length = math.sqrt(sum(value * value for value in direction))
    normalized_z = abs(direction[2] / length) if length else 0.0
    fresnel = 0.0 if normalized_z < 1.0e-6 else normalized_z ** cb0[15][2]
    dissolve = saturate(
        (textures[0]["pixels"][0][0] + 1.0 - cb0[3][2]) * cb0[15][0]
    )
    u, v = carrier[4][0], carrier[4][1]
    boundary = saturate(u * (1.0 - u) * v * (1.0 - v) * cb0[15][1])
    main_mask = textures[2]["pixels"][0][0] * cb0[1][3]
    return cb0[0][0] * saturate(
        fresnel * dissolve * main_mask * boundary * cb0[15][3]
    )


def sprite_opacity_oracle(fixture: dict[str, Any]) -> float:
    cb0 = fixture["cb0"]
    carrier = fixture["carrier"]
    textures = fixture["textures"]
    u, v = carrier[4][0], carrier[4][1]
    dissolve = saturate(textures[3]["pixels"][0][0] + 1.1 - cb0[3][1])
    radial = min(
        max(
            max(1.0 - 2.0 * math.hypot(u - 0.5, v - 0.5), 0.0)
            * cb0[23][0],
            cb0[23][2],
        ),
        cb0[23][1],
    )
    product = saturate(
        dissolve
        * cb0[22][2]
        * textures[1]["pixels"][0][0]
        * cb0[23][3]
        * cb0[1][3]
    )
    return cb0[0][0] * radial * product


def replay_cases() -> list[dict[str, Any]]:
    carrier, water, water_textures, sprite, sprite_textures = base_fixtures()
    water_base = {
        "family": "#9/#10_WATERTRAIL",
        "cb0": water,
        "carrier": carrier,
        "textures": water_textures,
    }
    sprite_base = {
        "family": "#11_SPRITEWAVE",
        "cb0": sprite,
        "carrier": carrier,
        "textures": sprite_textures,
    }
    return [
        mutate(water_base, "water/baseline", 0.16038000583648682),
        mutate(water_base, "water/dissolve-scale-zero", 0.0, cb={(15, 0): 0.0}),
        mutate(water_base, "water/dissolve-unsaturated", 0.028868401423096657, cb={(15, 0): 0.1}),
        mutate(water_base, "water/noise-r-zero-unsaturated", 0.01603800244629383, cb={(15, 0): 0.1}, texture_values={(0, 0): 0.0}),
        mutate(water_base, "water/noise-gba-negative-control", 0.028868401423096657, cb={(15, 0): 0.1}, texture_values={(0, 1): 0.91, (0, 2): 0.73, (0, 3): 0.19}),
        mutate(water_base, "water/main-mask-zero", 0.0, texture_values={(2, 0): 0.0}),
        mutate(water_base, "water/boundary-zero", 0.0, carrier={(4, 0): 0.0}),
        mutate(water_base, "water/fresnel-zero", 0.0, carrier={(6, 2): 0.0}),
        mutate(water_base, "water/dynamic-threshold-1.6", 0.038491200655698776, cb={(3, 2): 1.6}),
        mutate(water_base, "water/external-opacity-half", 0.08019000291824341, cb={(0, 0): 0.4}),
        mutate(water_base, "water/renderer-alpha-zero", 0.0, cb={(1, 3): 0.0}),
        mutate(sprite_base, "sprite/baseline", 0.0846720039844513),
        mutate(sprite_base, "sprite/dissolve-multiplier-zero", 0.0, cb={(22, 2): 0.0}),
        mutate(sprite_base, "sprite/main-t1-r-zero", 0.0, texture_values={(1, 0): 0.0}),
        mutate(sprite_base, "sprite/dissolve-t3-r-zero", 0.01693440042436123, texture_values={(3, 0): 0.0}),
        mutate(sprite_base, "sprite/dissolve-t3-r-one", 0.1693440079689026, texture_values={(3, 0): 1.0}),
        mutate(sprite_base, "sprite/radial-floor", 0.014112000353634357, carrier={(4, 0): 0.0}),
        mutate(sprite_base, "sprite/rgb-exponent-negative-control", 0.0846720039844513, cb={(22, 3): 7.0}),
        mutate(sprite_base, "sprite/external-opacity-half", 0.04233600199222565, cb={(0, 0): 0.4}),
        mutate(sprite_base, "sprite/dynamic-threshold-1.5", 0.0, cb={(3, 1): 1.5}),
        mutate(sprite_base, "sprite/renderer-alpha-zero", 0.0, cb={(1, 3): 0.0}),
    ]


def load_original_pixel_shaders(
    receipt: dict[str, Any],
) -> dict[str, dict[str, Any]]:
    result = {}
    for label, expected in PS_IDENTITIES.items():
        target = next(
            row
            for row in receipt["officialRefShaderCacheV974"]["mainTargets"]
            if row["label"] == label
        )
        pixel = next(
            row
            for row in target["materialMap"]["selectedOriginalDxbc"]
            if row["shaderType"] == BASE_PASS_PIXEL
        )
        bytecode = bytes.fromhex(pixel["dxbcHex"])
        require(bytecode[:4] == b"DXBC", f"{label} DXBC magic changed")
        require(sha256_bytes(bytecode) == pixel["dxbcSha256"] == expected["dxbcSha256"], f"{label} DXBC identity changed")
        require(pixel["shaderIdHex"] == expected["shaderIdHex"], f"{label} shader ID changed")
        result[label] = {
            "shaderIdHex": pixel["shaderIdHex"],
            "dxbcSha256": pixel["dxbcSha256"],
            "dxbcByteSize": len(bytecode),
            "bytecode": bytecode,
        }
    return result


def close_enough(left: float, right: float) -> bool:
    return abs(left - right) <= FLOAT_TOLERANCE


def build_receipt(
    input_path: Path,
    compiler_path: Path,
    d3d11_path: Path,
    warp_path: Path,
) -> dict[str, Any]:
    shader_receipt = read_json(input_path)
    validate_shader_cache_receipt(shader_receipt)
    shaders = load_original_pixel_shaders(shader_receipt)
    d3d11_identity = external_binary_identity(d3d11_path)
    warp_identity = external_binary_identity(warp_path)
    replay = WarpPixelReplay(compiler_path)
    rows = []
    baseline_by_family: dict[str, list[list[float]]] = {}
    try:
        for fixture in replay_cases():
            family = fixture["family"]
            shader = shaders[family]
            outputs = replay.run(
                shader["bytecode"],
                fixture["cb0"],
                fixture["carrier"],
                fixture["textures"],
                render_target_count=6,
            )
            oracle = (
                water_opacity_oracle(fixture)
                if family == "#9/#10_WATERTRAIL"
                else sprite_opacity_oracle(fixture)
            )
            actual = outputs[0][3]
            require(close_enough(actual, fixture["expectedAlpha"]), f"{fixture['caseId']} expected alpha mismatch")
            require(close_enough(actual, oracle), f"{fixture['caseId']} CPU oracle mismatch")
            if fixture["caseId"].endswith("/baseline"):
                baseline_by_family[family] = outputs
            rows.append(
                {
                    "caseId": fixture["caseId"],
                    "family": family,
                    "shaderIdHex": shader["shaderIdHex"],
                    "dxbcSha256": shader["dxbcSha256"],
                    "expectedAlpha": fixture["expectedAlpha"],
                    "closedFormOracleAlpha": oracle,
                    "actualWarpAlpha": actual,
                    "absoluteError": abs(actual - oracle),
                    "mrtFloat4": outputs,
                    "decision": "PASS",
                }
            )

        sentinel = [-99.0, -99.0, -99.0, -99.0]
        shared_output = [1.0, 0.5, 1.0, 0.0]
        zero = [0.0, 0.0, 0.0, 0.0]
        expected_rt0 = {
            "#9/#10_WATERTRAIL": [0.9000000357627869, 0.40000003576278687, 0.19999998807907104, 0.16038000583648682],
            "#11_SPRITEWAVE": [0.8999999761581421, 0.8999999761581421, 0.8999999761581421, 0.0846720039844513],
        }
        for family, outputs in baseline_by_family.items():
            expected_matrix = [expected_rt0[family], sentinel, shared_output, zero, zero, zero]
            require(
                all(
                    close_enough(actual, expected)
                    for actual_row, expected_row in zip(outputs, expected_matrix)
                    for actual, expected in zip(actual_row, expected_row)
                ),
                f"{family} six-MRT baseline changed",
            )

        carrier, water, water_textures, sprite, sprite_textures = base_fixtures()
        single_target_controls = []
        for family, cb0, textures in (
            ("#9/#10_WATERTRAIL", water, water_textures),
            ("#11_SPRITEWAVE", sprite, sprite_textures),
        ):
            output = replay.run(
                shaders[family]["bytecode"],
                cb0,
                carrier,
                textures,
                render_target_count=1,
            )[0]
            six = baseline_by_family[family][0]
            require(
                struct.pack("<4f", *output) == struct.pack("<4f", *six),
                f"{family} one-vs-six RTV RT0 differs",
            )
            single_target_controls.append(
                {"family": family, "oneRtvFloat4": output, "sixRtvFloat4": six, "bitExact": True}
            )

        sprite_baseline = next(row for row in rows if row["caseId"] == "sprite/baseline")
        sprite_negative = next(row for row in rows if row["caseId"] == "sprite/rgb-exponent-negative-control")
        require(
            struct.pack("<f", sprite_baseline["actualWarpAlpha"])
            == struct.pack("<f", sprite_negative["actualWarpAlpha"]),
            "sprite RGB-only exponent changed alpha bits",
        )
        require(
            struct.pack("<3f", *sprite_baseline["mrtFloat4"][0][:3])
            != struct.pack("<3f", *sprite_negative["mrtFloat4"][0][:3]),
            "sprite RGB-only exponent negative control did not perturb RGB",
        )
    finally:
        replay.close()

    public_shaders = {
        label: {key: value for key, value in row.items() if key != "bytecode"}
        for label, row in shaders.items()
    }
    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "toolIdentity": {
            "repoRelativePath": SCRIPT_PATH.relative_to(REPO_ROOT).as_posix(),
            "normalizedSha256": canonical_text_sha256(SCRIPT_PATH),
        },
        "inputReceipt": {
            "repoRelativePath": input_path.relative_to(REPO_ROOT).as_posix(),
            "receiptSha256": shader_receipt["receiptSha256"],
        },
        "backend": {
            "name": "D3D11_WARP_ORIGINAL_PIXEL_SHADER_DXBC",
            "featureLevel": replay.feature_level,
            "d3d11": d3d11_identity,
            "warp": warp_identity,
            "carrierVertexShader": replay.carrier_identity,
            "pixelShaderCompilation": False,
            "pixelShaderCreation": "RAW_RECEIPT_DXBC_TO_ID3D11DEVICE_CREATEPIXELSHADER",
            "renderTargetFormat": "DXGI_FORMAT_R32G32B32A32_FLOAT",
            "textureFormat": "DXGI_FORMAT_R32G32B32A32_FLOAT",
            "sampler": "POINT_CLAMP",
        },
        "originalPixelShaders": public_shaders,
        "opacitySsaContract": {
            "water": {
                "formula": "cb0[0].x * sat(pow(abs(normalize(v6).z),cb0[15].z) * sat((t0.R+1-cb0[3].z)*cb0[15].x) * (t2.R*cb0[1].w) * sat(v4.x*(1-v4.x)*v4.y*(1-v4.y)*cb0[15].y) * cb0[15].w)",
                "dissolveChannel": "t0.R_SECOND_SAMPLE",
                "mainMaskChannel": "t2.R",
                "explicitDynamicInputs": ["cb0[0].x external opacity", "cb0[1].w renderer/pass alpha", "cb0[3].z mesh-emitter dynamic parameter", "v4.xy carrier UV", "v6.xyz carrier direction"],
            },
            "sprite": {
                "formula": "cb0[0].x * min(max(max(1-2*length(v4.xy-.5),0)*cb0[23].x,cb0[23].z),cb0[23].y) * sat(sat(t3.R+1.1-cb0[3].y)*cb0[22].z*t1.R*cb0[23].w*cb0[1].w)",
                "dissolveChannel": "t3.R",
                "mainMaskChannel": "t1.R",
                "rgbOnlyNegativeControl": "cb0[22].w",
                "explicitDynamicInputs": ["cb0[0].x external opacity", "cb0[1].w renderer/pass alpha", "cb0[3].y mesh-emitter dynamic parameter", "v4.xy carrier UV"],
            },
            "fidelity": "REVIEWED_STRAIGHT_LINE_DXBC_OPACITY_BACKWARD_SLICE_PLUS_NATIVE_CB_TEXTURE_REGISTER_JOIN",
        },
        "numericContract": {
            "absoluteTolerance": FLOAT_TOLERANCE,
            "caseCount": len(rows),
            "cases": rows,
            "oneVersusSixRenderTargetControls": single_target_controls,
            "spriteRgbExponentAlphaBitStable": True,
        },
        "decision": {
            "sameCohortOriginalPixelShaderCount": 2,
            "fixedInputReplayCaseCount": len(rows),
            "fixedInputOriginalDxbcReplayAdmission": True,
            "parameterRegisterTextureChannelClosureAdmission": True,
            "candidateOfflineEvaluatorImplementationAdmission": True,
            "occurrenceRuntimeEvaluatorMutationAdmission": False,
            "runtimeHlslMutationAdmission": False,
            "visualProgressAdmission": False,
            "nextGate": "SOURCE_REPLAYED_ISOLATED_RUNTIME_CANARY_WITH_OCCURRENCE_SELECTION_BOUNDARY",
            "status": "ORIGINAL_DXBC_FIXED_INPUT_REPLAY_PASS_CANDIDATE_IMPLEMENTATION_OPEN_OCCURRENCE_TRANSACTION_CLOSED",
        },
    }
    seal(receipt)
    return receipt


EXPECTED_CASE_IDS = [row["caseId"] for row in replay_cases()]


def validate_receipt(receipt: dict[str, Any]) -> None:
    require(receipt.get("schema") == SCHEMA, "replay receipt schema mismatch")
    require(receipt.get("formatVersion") == FORMAT_VERSION, "replay receipt version mismatch")
    claimed = receipt.get("receiptSha256")
    unsigned = dict(receipt)
    unsigned.pop("receiptSha256", None)
    require(claimed == canonical_json_sha256(unsigned), "replay receipt digest mismatch")
    backend = receipt["backend"]
    require(backend["featureLevel"] == FEATURE_LEVEL_11_0, "replay feature level changed")
    require(not backend["pixelShaderCompilation"], "original pixel shader was recompiled")
    require(
        backend["d3d11"]["rawSha256"] == EXPECTED_BINARIES["d3d11.dll"]["sha256"]
        and backend["warp"]["rawSha256"] == EXPECTED_BINARIES["d3d10warp.dll"]["sha256"]
        and backend["carrierVertexShader"]["compiler"]["rawSha256"]
        == EXPECTED_BINARIES["d3dcompiler_47.dll"]["sha256"],
        "replay binary identity changed",
    )
    for label, expected in PS_IDENTITIES.items():
        actual = receipt["originalPixelShaders"][label]
        require(actual["shaderIdHex"] == expected["shaderIdHex"], "replay shader ID changed")
        require(actual["dxbcSha256"] == expected["dxbcSha256"], "replay DXBC SHA changed")
    numeric = receipt["numericContract"]
    require(numeric["caseCount"] == len(EXPECTED_CASE_IDS), "replay case denominator changed")
    require([row["caseId"] for row in numeric["cases"]] == EXPECTED_CASE_IDS, "replay case order changed")
    require(all(row["decision"] == "PASS" for row in numeric["cases"]), "replay case is not PASS")
    require(all(row["absoluteError"] <= FLOAT_TOLERANCE for row in numeric["cases"]), "replay error exceeds tolerance")
    fixtures = replay_cases()
    for row, fixture in zip(numeric["cases"], fixtures):
        require(row["family"] == fixture["family"], "replay case family changed")
        require(
            row["shaderIdHex"] == PS_IDENTITIES[fixture["family"]]["shaderIdHex"]
            and row["dxbcSha256"] == PS_IDENTITIES[fixture["family"]]["dxbcSha256"],
            "replay case shader identity changed",
        )
        require(
            close_enough(row["expectedAlpha"], fixture["expectedAlpha"]),
            "replay case expected alpha changed",
        )
        require(
            close_enough(row["actualWarpAlpha"], fixture["expectedAlpha"])
            and close_enough(row["closedFormOracleAlpha"], fixture["expectedAlpha"]),
            "replay case numeric result changed",
        )
        require(
            close_enough(
                row["absoluteError"],
                abs(row["actualWarpAlpha"] - row["closedFormOracleAlpha"]),
            ),
            "replay case error field changed",
        )
        require(
            len(row["mrtFloat4"]) == 6
            and all(
                len(target) == 4 and all(math.isfinite(value) for value in target)
                for target in row["mrtFloat4"]
            ),
            "replay MRT shape or finiteness changed",
        )
    expected_baselines = {
        "water/baseline": [
            [0.9000000357627869, 0.40000003576278687, 0.19999998807907104, 0.16038000583648682],
            [-99.0, -99.0, -99.0, -99.0],
            [1.0, 0.5, 1.0, 0.0],
            [0.0, 0.0, 0.0, 0.0],
            [0.0, 0.0, 0.0, 0.0],
            [0.0, 0.0, 0.0, 0.0],
        ],
        "sprite/baseline": [
            [0.8999999761581421, 0.8999999761581421, 0.8999999761581421, 0.0846720039844513],
            [-99.0, -99.0, -99.0, -99.0],
            [1.0, 0.5, 1.0, 0.0],
            [0.0, 0.0, 0.0, 0.0],
            [0.0, 0.0, 0.0, 0.0],
            [0.0, 0.0, 0.0, 0.0],
        ],
    }
    for case_id, expected_matrix in expected_baselines.items():
        row = next(item for item in numeric["cases"] if item["caseId"] == case_id)
        require(
            all(
                close_enough(actual, expected)
                for actual_row, expected_row in zip(row["mrtFloat4"], expected_matrix)
                for actual, expected in zip(actual_row, expected_row)
            ),
            f"{case_id} MRT baseline changed",
        )
    require(len(numeric["oneVersusSixRenderTargetControls"]) == 2, "RTV control denominator changed")
    require(all(row["bitExact"] for row in numeric["oneVersusSixRenderTargetControls"]), "RTV control lost bit equality")
    require(numeric["spriteRgbExponentAlphaBitStable"], "sprite RGB-only negative control failed")
    decision = receipt["decision"]
    require(decision["sameCohortOriginalPixelShaderCount"] == 2, "replay shader denominator changed")
    require(decision["fixedInputReplayCaseCount"] == len(EXPECTED_CASE_IDS), "decision replay denominator changed")
    require(decision["fixedInputOriginalDxbcReplayAdmission"], "fixed-input replay admission closed")
    require(decision["parameterRegisterTextureChannelClosureAdmission"], "binding closure admission closed")
    require(
        decision["candidateOfflineEvaluatorImplementationAdmission"],
        "candidate evaluator implementation gate did not open",
    )
    require(
        not decision["occurrenceRuntimeEvaluatorMutationAdmission"],
        "unproven occurrence runtime evaluator mutation opened",
    )
    require(not decision["runtimeHlslMutationAdmission"] and not decision["visualProgressAdmission"], "unverified visual/HLSL admission opened")


def check_or_write(path: Path, receipt: dict[str, Any], check: bool) -> None:
    if check:
        require(path.is_file(), f"replay receipt is missing: {path}")
        require(read_json(path) == receipt, "replay receipt is stale")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(receipt, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
        newline="\n",
    )
    temporary.replace(path)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", type=Path, default=DEFAULT_INPUT)
    parser.add_argument("--compiler", type=Path, default=DEFAULT_COMPILER)
    parser.add_argument("--d3d11", type=Path, default=DEFAULT_D3D11)
    parser.add_argument("--warp", type=Path, default=DEFAULT_WARP)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--validate-only", action="store_true")
    args = parser.parse_args()
    if args.validate_only:
        require(args.output.is_file(), f"replay receipt is missing: {args.output}")
        validate_receipt(read_json(args.output))
        print(f"PASS: Artist F original DXBC replay shallow shaders=2 cases={len(EXPECTED_CASE_IDS)} candidate=true occurrence=false hlsl=false visual=false")
        return 0
    receipt = build_receipt(args.input, args.compiler, args.d3d11, args.warp)
    validate_receipt(receipt)
    check_or_write(args.output, receipt, args.check)
    mode = "deep-check" if args.check else "deep-write"
    print(f"PASS: Artist F original DXBC replay {mode} shaders=2 cases={len(EXPECTED_CASE_IDS)} candidate=true occurrence=false hlsl=false visual=false")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
