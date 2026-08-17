#!/usr/bin/env python3
"""Replay class-neutral UE3 Material pixel shaders with synthetic fixed inputs.

G03-3 already proves the exact cooked pixel DXBC and native Material binding
wires.  This tool advances only the next structural gate: raw DXBC creation,
signature-compatible carrier linkage, complete CB/SRV/sampler binding, and
deterministic WARP execution.  It deliberately does *not* claim that the
synthetic constant/texture values are the source MIC values.  Source-value
replay stays closed until the UE3 uniform-expression evaluator, parent default
textures, and sampler state evidence are complete.

The replay unit is supplied by the input receipt (family + effective static
set + platform + structural renderer/pass shader).  No character, skill, or
family name is used to select code paths.
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
from typing import Any, Iterable


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
LEVEL_TOOLS = REPOSITORY_ROOT / "Tools" / "LevelPlacementExtractor"
if str(LEVEL_TOOLS) not in sys.path:
    sys.path.insert(0, str(LEVEL_TOOLS))

from extract_artist_31470_main_ref_shader_cache import (  # noqa: E402
    D3DDisassembler,
    DEFAULT_D3DCOMPILER,
    package_tables,
    require,
)
from extract_artist_31470_shader_cache_oracle import (  # noqa: E402
    canonical_json_sha256,
    validate_dxbc_container,
)
from extract_ue3_placements import decompress_lz4_block  # noqa: E402
from replay_artist_31470_main_original_dxbc import (  # noqa: E402
    BufferDesc,
    HRESULT,
    MappedSubresource,
    RasterizerDesc,
    SampleDesc,
    SamplerDesc,
    SubresourceData,
    Texture2DDesc,
    UINT,
    Viewport,
    blob_bytes,
    checked,
    com_method,
    create_warp_device,
    external_binary_identity,
    release,
)


SCHEMA = "lostark.effect-ue3-material-fixed-input-replay-receipt"
FORMAT_VERSION = 1
INPUT_SCHEMA = "lostark.effect-ue3-material-shader-map-receipt"
INPUT_FORMAT_VERSION = 3
SCRIPT_PATH = Path(__file__).resolve()

DEFAULT_INPUT = REPOSITORY_ROOT / (
    "Data/Effects/Imported/DimensionMaster/Materials/"
    "skill.2050120.clip3.exact-material-maps.receipt.json"
)
DEFAULT_OUTPUT = REPOSITORY_ROOT / (
    "Data/Effects/Imported/DimensionMaster/Materials/"
    "skill.2050120.clip3.structural-fixed-input-replay.receipt.json"
)
DEFAULT_CACHE = Path(
    "C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Effect/ARTIST/"
    "31470_TrackA_20260812/OfficialRefShaderCacheV974/"
    "EV2LG3OVEH3HGV7THTFFTM7TOKMCC.v974.upk"
)
DEFAULT_D3D11 = Path("C:/Windows/System32/d3d11.dll")
DEFAULT_WARP = Path("C:/Windows/System32/d3d10warp.dll")

SENTINEL = -99.0
FLOAT_EPSILON = 1.0e-6
STATUS_EXACT = "EXACT_MATERIAL_SHADER_MAP"
NATIVE_EXACT = "EXACT_NATIVE_SHADER_OBJECT_BINDING"


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


def validate_input_receipt(document: dict[str, Any]) -> list[dict[str, Any]]:
    require(document.get("schema") == INPUT_SCHEMA, "G03-3 receipt schema changed")
    require(
        document.get("formatVersion") == INPUT_FORMAT_VERSION,
        "G03-3 receipt format changed",
    )
    claimed = document.get("receiptSha256")
    unsigned = dict(document)
    unsigned.pop("receiptSha256", None)
    require(claimed == canonical_json_sha256(unsigned), "G03-3 receipt seal changed")
    targets = document.get("targets")
    require(isinstance(targets, list), "G03-3 targets are absent")
    exact = [
        row
        for row in targets
        if row.get("status") == STATUS_EXACT
        and isinstance(row.get("nativeShaderObjectBinding"), dict)
        and row["nativeShaderObjectBinding"].get("status") == NATIVE_EXACT
    ]
    ids = [row.get("targetId") for row in exact]
    require(all(isinstance(value, str) and value for value in ids), "target ID is absent")
    require(len(ids) == len(set(ids)), "exact replay target ID is duplicated")
    for row in exact:
        require(isinstance(row.get("familyId"), str), "family ID is absent")
        require(row.get("cookedPixelShader", {}).get("exactOneDxbcContainer"), "exact DXBC evidence is absent")
        require(not row["cookedPixelShader"].get("actualVfPassAdmission"), "G03-4 input unexpectedly admits actual VF/pass")
        require(not row["nativeShaderObjectBinding"].get("runtimeAdmission"), "G03-4 input unexpectedly admits runtime")
    return exact


def build_replay_plan(targets: Iterable[dict[str, Any]]) -> list[dict[str, Any]]:
    """Normalize a 0/1/N target denominator without skill-specific policy."""

    result = []
    seen: set[str] = set()
    for target in targets:
        target_id = target.get("targetId")
        require(isinstance(target_id, str) and target_id, "replay target ID is invalid")
        require(target_id not in seen, "replay target ID is duplicated")
        seen.add(target_id)
        pixel = target.get("cookedPixelShader")
        binding = target.get("nativeShaderObjectBinding")
        require(isinstance(pixel, dict) and pixel.get("exactOneDxbcContainer"), "replay target has no exact DXBC")
        require(isinstance(binding, dict) and binding.get("status") == NATIVE_EXACT, "replay target has no exact native binding")
        result.append(target)
    return result


def rehydrate_dxbc(cache: dict[str, Any], target: dict[str, Any]) -> bytes:
    pixel = target["cookedPixelShader"]
    compressed_row = pixel["compressedCodeBlob"]
    descriptor = pixel["packedDescriptor"]
    reader = cache["reader"]
    compressed = reader.read_logical_range(
        compressed_row["compressedLogicalOffset"],
        compressed_row["compressedByteSize"],
    )
    require(
        sha256_bytes(compressed) == compressed_row["compressedSha256"],
        f"compressed code identity changed: {target['targetId']}",
    )
    uncompressed = decompress_lz4_block(
        compressed, compressed_row["uncompressedByteSize"]
    )
    require(
        sha256_bytes(uncompressed) == compressed_row["uncompressedSha256"],
        f"uncompressed code identity changed: {target['targetId']}",
    )
    offset = descriptor["sliceOffsetInUncompressedBlob"]
    size = descriptor["sliceByteSize"]
    bytecode = uncompressed[offset : offset + size]
    require(len(bytecode) == size, f"DXBC slice is truncated: {target['targetId']}")
    container = validate_dxbc_container(bytecode)
    require(
        container["sha256"] == pixel["dxbc"]["sha256"]
        and container["byteSize"] == pixel["dxbc"]["byteSize"],
        f"DXBC identity changed: {target['targetId']}",
    )
    return bytecode


def dxbc_chunks(bytecode: bytes) -> dict[str, bytes]:
    require(bytecode[:4] == b"DXBC" and len(bytecode) >= 32, "DXBC header is invalid")
    total_size, count = struct.unpack_from("<II", bytecode, 24)
    require(total_size == len(bytecode), "DXBC total size changed")
    require(32 + count * 4 <= len(bytecode), "DXBC chunk table is truncated")
    result: dict[str, bytes] = {}
    for index in range(count):
        offset = struct.unpack_from("<I", bytecode, 32 + index * 4)[0]
        require(offset + 8 <= len(bytecode), "DXBC chunk header is truncated")
        four_cc = bytecode[offset : offset + 4].decode("ascii", "strict")
        size = struct.unpack_from("<I", bytecode, offset + 4)[0]
        require(offset + 8 + size <= len(bytecode), "DXBC chunk is truncated")
        require(four_cc not in result, f"DXBC chunk is duplicated: {four_cc}")
        result[four_cc] = bytecode[offset + 8 : offset + 8 + size]
    return result


def parse_signature(payload: bytes) -> list[dict[str, Any]]:
    require(len(payload) >= 8, "signature payload is truncated")
    count, reserved = struct.unpack_from("<II", payload, 0)
    require(reserved in (0, 8), "signature header changed")
    require(8 + count * 24 <= len(payload), "signature rows are truncated")
    rows = []
    for index in range(count):
        offset = 8 + index * 24
        (
            name_offset,
            semantic_index,
            system_value,
            component_type,
            register,
            mask,
            read_write_mask,
            stream,
        ) = struct.unpack_from("<5I2BH", payload, offset)
        require(name_offset < len(payload), "signature semantic name offset is invalid")
        end = payload.find(b"\0", name_offset)
        require(end >= 0, "signature semantic name is unterminated")
        name = payload[name_offset:end].decode("ascii", "strict")
        require(mask != 0 and mask <= 0xF, "signature component mask is invalid")
        rows.append(
            {
                "semanticName": name,
                "semanticIndex": semantic_index,
                "systemValueType": system_value,
                "componentType": component_type,
                "register": register,
                "mask": mask,
                "readWriteMask": read_write_mask,
                "stream": stream,
            }
        )
    return rows


def public_signature(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    return [dict(row) for row in rows]


def _hlsl_scalar(component_type: int) -> str:
    require(component_type in (1, 2, 3), "signature component type is unsupported")
    return {1: "uint", 2: "int", 3: "float"}[component_type]


def _component_width(mask: int) -> int:
    return max(index + 1 for index in range(4) if mask & (1 << index))


def build_carrier_source(input_signature: list[dict[str, Any]]) -> tuple[bytes, int]:
    max_register = max(
        (row["register"] for row in input_signature if row["systemValueType"] != 9),
        default=0,
    )
    row_count = max_register + 1
    fields = []
    assignments = []
    has_position = False
    for index, row in enumerate(input_signature):
        name = row["semanticName"]
        if row["systemValueType"] == 9 or name.casefold() == "sv_isfrontface":
            continue
        width = _component_width(row["mask"])
        scalar = _hlsl_scalar(row["componentType"])
        type_name = scalar if width == 1 else f"{scalar}{width}"
        semantic = f"{name}{row['semanticIndex']}"
        field = f"f{index}"
        fields.append(f" {type_name} {field}:{semantic};")
        if row["systemValueType"] == 1 or name.casefold() == "sv_position":
            require(width == 4 and scalar == "float", "SV_Position signature changed")
            assignments.append(f" o.{field}=float4(q[id],0,1);")
            has_position = True
        else:
            swizzle = "xyzw"[:width]
            cast = "" if scalar == "float" else f"({type_name})"
            assignments.append(
                f" o.{field}={cast}k[{row['register']}].{swizzle};"
            )
    # Pixel ISGN normally omits SV_Position when the shader never reads it,
    # but the rasterizer still requires the carrier VS to produce one.
    if not has_position:
        fields.append(" float4 carrierPosition:SV_Position;")
        assignments.append(" o.carrierPosition=float4(q[id],0,1);")
    source = (
        f"cbuffer Carrier:register(b0){{float4 k[{row_count}];}}\n"
        "struct O{\n"
        + "\n".join(fields)
        + "\n};\n"
        "O main(uint id:SV_VertexID){"
        "float2 q[3]={float2(-1,-1),float2(-1,3),float2(3,-1)};O o;"
        + "".join(assignments)
        + "return o;}"
    ).encode("ascii")
    return source, row_count


def close_carrier_signature(
    pixel_inputs: list[dict[str, Any]], carrier_outputs: list[dict[str, Any]]
) -> dict[str, Any]:
    producers: dict[tuple[str, int], list[dict[str, Any]]] = {}
    for row in carrier_outputs:
        key = (row["semanticName"].casefold(), row["semanticIndex"])
        producers.setdefault(key, []).append(row)
    links = []
    for consumer in pixel_inputs:
        if (
            consumer["systemValueType"] == 9
            or consumer["semanticName"].casefold() == "sv_isfrontface"
        ):
            continue
        key = (
            consumer["semanticName"].casefold(),
            consumer["semanticIndex"],
        )
        candidates = producers.get(key, [])
        require(len(candidates) == 1, f"carrier semantic is absent or ambiguous: {key}")
        producer = candidates[0]
        require(
            producer["componentType"] == consumer["componentType"],
            f"carrier component type changed: {key}",
        )
        require(
            producer["mask"] & consumer["mask"] == consumer["mask"],
            f"carrier component mask does not cover pixel input: {key}",
        )
        links.append(
            {
                "semanticName": consumer["semanticName"],
                "semanticIndex": consumer["semanticIndex"],
                "consumerRegister": consumer["register"],
                "consumerMask": consumer["mask"],
                "producerRegister": producer["register"],
                "producerMask": producer["mask"],
                "componentType": consumer["componentType"],
            }
        )
    require(links, "carrier signature has no linked pixel inputs")
    return {
        "linkedSemanticCount": len(links),
        "rasterizerOwnedSystemValueCount": sum(
            row["systemValueType"] == 9
            or row["semanticName"].casefold() == "sv_isfrontface"
            for row in pixel_inputs
        ),
        "links": links,
        "pass": True,
    }


class CarrierCompiler:
    def __init__(self, path: Path) -> None:
        self.path = path
        self.identity = external_binary_identity(path)
        self.dll = ctypes.WinDLL(str(path))
        self.function = self.dll.D3DCompile
        self.function.restype = HRESULT
        self.function.argtypes = [
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

    def compile(self, source: bytes, label: str) -> tuple[bytes, dict[str, Any]]:
        backing = ctypes.create_string_buffer(source)
        code = ctypes.c_void_p()
        errors = ctypes.c_void_p()
        try:
            result = self.function(
                backing,
                len(source),
                label.encode("ascii"),
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
                blob_bytes(errors).decode("utf-8", "replace")
                if errors.value
                else ""
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
            "sourceSha256": sha256_bytes(source),
            "compiledDxbcSha256": sha256_bytes(bytecode),
            "compiler": self.identity,
            "linkageContract": "PS_ISGN_SEMANTIC_COMPATIBLE_GENERATED_FULLSCREEN_TRIANGLE",
        }


def parse_runtime_declarations(disassembly: dict[str, Any]) -> dict[str, Any]:
    constant_buffers: dict[int, int] = {}
    textures: set[int] = set()
    samplers: set[int] = set()
    for line in disassembly["declarations"]:
        match = re.search(r"\bcb(\d+)\[(\d+)\]", line, re.IGNORECASE)
        if match:
            constant_buffers[int(match.group(1))] = int(match.group(2))
        match = re.search(r"\bt(\d+)\b", line)
        if line.startswith("dcl_resource") and match:
            textures.add(int(match.group(1)))
        match = re.search(r"\bs(\d+)\b", line)
        if line.startswith("dcl_sampler") and match:
            samplers.add(int(match.group(1)))
    require(0 in constant_buffers, "pixel shader does not declare cb0")
    return {
        "constantBufferFloat4Counts": {
            str(key): value for key, value in sorted(constant_buffers.items())
        },
        "textureRegisters": sorted(textures),
        "samplerRegisters": sorted(samplers),
    }


class GenericWarpReplay:
    def __init__(self) -> None:
        self.device, self.context, self.feature_level = create_warp_device()
        self.permanent: list[ctypes.c_void_p] = []
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
        self.sampler_cache: dict[int, ctypes.c_void_p] = {}

    def _shader(self, data: bytes, method_index: int, label: str) -> ctypes.c_void_p:
        pointer = ctypes.c_void_p()
        backing = ctypes.create_string_buffer(data)
        result = com_method(
            self.device,
            method_index,
            HRESULT,
            ctypes.c_void_p,
            ctypes.c_size_t,
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_void_p),
        )(self.device, backing, len(data), None, ctypes.byref(pointer))
        checked(result, pointer, label)
        return pointer

    def _buffer(self, rows: list[list[float]]) -> ctypes.c_void_p:
        require(rows and all(len(row) == 4 for row in rows), "constant buffer rows are invalid")
        raw = struct.pack(
            "<" + "f" * (4 * len(rows)),
            *(value for row in rows for value in row),
        )
        pointer = ctypes.c_void_p()
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
        )(self.device, ctypes.byref(description), ctypes.byref(initial), ctypes.byref(pointer))
        checked(result, pointer, "CreateBuffer")
        return pointer

    def _texture(
        self,
        pixel: list[float] | None = None,
        *,
        bind: int = 8,
        staging: bool = False,
    ) -> ctypes.c_void_p:
        pointer = ctypes.c_void_p()
        description = Texture2DDesc(
            1,
            1,
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
        if pixel is not None:
            require(len(pixel) == 4, "texture pixel is invalid")
            backing = ctypes.create_string_buffer(struct.pack("<4f", *pixel))
            initial = SubresourceData(ctypes.cast(backing, ctypes.c_void_p), 16, 16)
            initial_pointer = ctypes.byref(initial)
        result = com_method(
            self.device,
            5,
            HRESULT,
            ctypes.POINTER(Texture2DDesc),
            ctypes.POINTER(SubresourceData),
            ctypes.POINTER(ctypes.c_void_p),
        )(self.device, ctypes.byref(description), initial_pointer, ctypes.byref(pointer))
        checked(result, pointer, "CreateTexture2D")
        return pointer

    def _view(self, resource: ctypes.c_void_p, method_index: int, label: str) -> ctypes.c_void_p:
        pointer = ctypes.c_void_p()
        result = com_method(
            self.device,
            method_index,
            HRESULT,
            ctypes.c_void_p,
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_void_p),
        )(self.device, resource, None, ctypes.byref(pointer))
        checked(result, pointer, label)
        return pointer

    def _sampler(self, slot: int) -> ctypes.c_void_p:
        if slot in self.sampler_cache:
            return self.sampler_cache[slot]
        description = SamplerDesc()
        description.Filter = 0
        description.AddressU = 3
        description.AddressV = 3
        description.AddressW = 3
        description.MaxAnisotropy = 1
        description.ComparisonFunc = 1
        description.MinLOD = -3.402823466e38
        description.MaxLOD = 3.402823466e38
        pointer = ctypes.c_void_p()
        result = com_method(
            self.device,
            23,
            HRESULT,
            ctypes.POINTER(SamplerDesc),
            ctypes.POINTER(ctypes.c_void_p),
        )(self.device, ctypes.byref(description), ctypes.byref(pointer))
        checked(result, pointer, "CreateSamplerState")
        self.sampler_cache[slot] = pointer
        self.permanent.append(pointer)
        return pointer

    def run(
        self,
        pixel_dxbc: bytes,
        carrier_dxbc: bytes,
        fixture: dict[str, Any],
        declarations: dict[str, Any],
        render_target_count: int,
    ) -> list[list[float]]:
        require(1 <= render_target_count <= 8, "render-target count is invalid")
        local: list[ctypes.c_void_p] = []
        try:
            pixel_shader = self._shader(pixel_dxbc, 15, "CreatePixelShader")
            vertex_shader = self._shader(carrier_dxbc, 12, "CreateVertexShader")
            local.extend((pixel_shader, vertex_shader))

            vertex_cb = self._buffer(fixture["carrierRows"])
            local.append(vertex_cb)
            cb_counts = {
                int(key): int(value)
                for key, value in declarations["constantBufferFloat4Counts"].items()
            }
            max_cb = max(cb_counts)
            cb_pointers: list[int] = []
            for register in range(max_cb + 1):
                if register not in cb_counts:
                    cb_pointers.append(0)
                    continue
                rows = fixture["constantBuffers"].get(str(register))
                require(
                    isinstance(rows, list) and len(rows) == cb_counts[register],
                    f"cb{register} fixture extent changed",
                )
                pointer = self._buffer(rows)
                local.append(pointer)
                cb_pointers.append(pointer.value)

            texture_registers = declarations["textureRegisters"]
            max_texture = max(texture_registers, default=-1)
            srv_values: list[int] = []
            for register in range(max_texture + 1):
                require(register in texture_registers, "sparse texture declaration is unsupported")
                pixel = fixture["textures"][str(register)]
                texture = self._texture(pixel)
                local.append(texture)
                view = self._view(texture, 7, "CreateShaderResourceView")
                local.append(view)
                srv_values.append(view.value)

            sampler_registers = declarations["samplerRegisters"]
            max_sampler = max(sampler_registers, default=-1)
            sampler_values: list[int] = []
            for register in range(max_sampler + 1):
                require(register in sampler_registers, "sparse sampler declaration is unsupported")
                sampler_values.append(self._sampler(register).value)

            render_textures = []
            render_views = []
            staging_textures = []
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

            com_method(self.context, 43, None, ctypes.c_void_p)(self.context, self.rasterizer)
            viewport = Viewport(0.0, 0.0, 1.0, 1.0, 0.0, 1.0)
            com_method(self.context, 44, None, UINT, ctypes.POINTER(Viewport))(
                self.context, 1, ctypes.byref(viewport)
            )
            com_method(self.context, 24, None, UINT)(self.context, 4)

            one_pointer = ctypes.c_void_p * 1
            com_method(self.context, 7, None, UINT, UINT, ctypes.POINTER(ctypes.c_void_p))(
                self.context, 0, 1, one_pointer(vertex_cb.value)
            )
            com_method(self.context, 11, None, ctypes.c_void_p, ctypes.c_void_p, UINT)(
                self.context, vertex_shader, None, 0
            )

            target_type = ctypes.c_void_p * render_target_count
            com_method(self.context, 33, None, UINT, ctypes.POINTER(ctypes.c_void_p), ctypes.c_void_p)(
                self.context,
                render_target_count,
                target_type(*[view.value for view in render_views]),
                None,
            )
            sentinel = (ctypes.c_float * 4)(SENTINEL, SENTINEL, SENTINEL, SENTINEL)
            clear = com_method(self.context, 50, None, ctypes.c_void_p, ctypes.POINTER(ctypes.c_float))
            for view in render_views:
                clear(self.context, view, sentinel)

            cb_type = ctypes.c_void_p * len(cb_pointers)
            com_method(self.context, 16, None, UINT, UINT, ctypes.POINTER(ctypes.c_void_p))(
                self.context, 0, len(cb_pointers), cb_type(*cb_pointers)
            )
            com_method(self.context, 9, None, ctypes.c_void_p, ctypes.c_void_p, UINT)(
                self.context, pixel_shader, None, 0
            )
            if srv_values:
                srv_type = ctypes.c_void_p * len(srv_values)
                com_method(self.context, 8, None, UINT, UINT, ctypes.POINTER(ctypes.c_void_p))(
                    self.context, 0, len(srv_values), srv_type(*srv_values)
                )
            if sampler_values:
                sampler_type = ctypes.c_void_p * len(sampler_values)
                com_method(self.context, 10, None, UINT, UINT, ctypes.POINTER(ctypes.c_void_p))(
                    self.context,
                    0,
                    len(sampler_values),
                    sampler_type(*sampler_values),
                )
            com_method(self.context, 13, None, UINT, UINT)(self.context, 3, 0)

            outputs = []
            for source, staging in zip(render_textures, staging_textures):
                com_method(self.context, 47, None, ctypes.c_void_p, ctypes.c_void_p)(
                    self.context, staging, source
                )
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
                )(self.context, staging, 0, 1, 0, ctypes.byref(mapped))
                checked(result, None, "Map")
                try:
                    require(mapped.RowPitch >= 16, "staging RowPitch is invalid")
                    outputs.append(list(struct.unpack("<4f", ctypes.string_at(mapped.pData, 16))))
                finally:
                    com_method(self.context, 15, None, ctypes.c_void_p, UINT)(
                        self.context, staging, 0
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


def output_register_count(output_signature: list[dict[str, Any]]) -> int:
    registers = [
        row["register"]
        for row in output_signature
        if row["semanticName"].casefold().startswith("sv_target")
        or row["systemValueType"] == 64
    ]
    if not registers:
        registers = [row["register"] for row in output_signature]
    require(registers, "pixel shader output signature is empty")
    return max(registers) + 1


def validate_mrt_contract(
    outputs: list[list[float]], output_signature: list[dict[str, Any]]
) -> dict[str, Any]:
    declared = {
        row["register"]: row
        for row in output_signature
        if row["semanticName"].casefold().startswith("sv_target")
    }
    require(declared, "pixel shader has no SV_Target output")
    require(len(outputs) == max(declared) + 1, "bound MRT extent changed")
    written = []
    for register, row in sorted(declared.items()):
        lanes = [lane for lane in range(4) if row["mask"] & (1 << lane)]
        require(lanes, "declared MRT output mask is empty")
        require(
            all(
                math.isfinite(outputs[register][lane])
                and abs(outputs[register][lane] - SENTINEL) > FLOAT_EPSILON
                for lane in lanes
            ),
            f"declared MRT output stayed sentinel: o{register}",
        )
        written.append({"register": register, "writtenLanes": lanes})
    holes = [register for register in range(len(outputs)) if register not in declared]
    for register in holes:
        require(
            outputs[register] == [SENTINEL] * 4,
            f"undeclared MRT hole was modified: o{register}",
        )
    return {
        "declaredTargets": written,
        "sentinelHoleRegisters": holes,
        "pass": True,
    }


def validate_native_binding_closure(
    native: dict[str, Any], declarations: dict[str, Any]
) -> dict[str, Any]:
    native_declarations = native["dxbcDeclarationClosure"]
    cb0_count = declarations["constantBufferFloat4Counts"].get("0")
    require(
        cb0_count == native_declarations["declaredConstantBuffer0Float4Count"]
        == native["constantBufferClosure"]["declaredConstantBuffer0Float4Count"],
        "native/DXBC cb0 extent changed",
    )
    bound_slots = native["constantBufferClosure"]["boundConstantBuffer0Slots"]
    require(
        bound_slots
        and all(isinstance(slot, int) and 0 <= slot < cb0_count for slot in bound_slots),
        "native cb0 bound slot is outside the declaration",
    )
    require(
        declarations["textureRegisters"]
        == native_declarations["declaredTextureRegisters"]
        and declarations["samplerRegisters"]
        == native_declarations["declaredSamplerRegisters"],
        "native/DXBC texture or sampler declaration changed",
    )
    samples = native["textureSampleClosure"]
    material_pairs = set(samples["materialSamplePairs"])
    engine_pairs = set(samples["unownedEngineSamplePairs"])
    observed_pairs = set(samples["allObservedSamplePairCounts"])
    require(not material_pairs & engine_pairs, "material and engine sample ownership overlaps")
    require(material_pairs | engine_pairs == observed_pairs, "sample-pair ownership closure changed")
    for pair in observed_pairs:
        match = re.fullmatch(r"t(\d+)/s(\d+)", pair)
        require(match is not None, f"sample pair is malformed: {pair}")
        require(
            int(match.group(1)) in declarations["textureRegisters"]
            and int(match.group(2)) in declarations["samplerRegisters"],
            f"native sample pair is not declared by DXBC: {pair}",
        )
    return {
        "declaredConstantBuffer0Float4Count": cb0_count,
        "boundConstantBuffer0Slots": bound_slots,
        "materialSamplePairs": sorted(material_pairs),
        "unownedEngineSamplePairs": sorted(engine_pairs),
        "observedSamplePairs": sorted(observed_pairs),
        "pass": True,
    }


def _pattern(seed: int, row: int, lane: int) -> float:
    table = (0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 2.0, -0.25, -0.5)
    return table[(seed * 17 + row * 5 + lane * 3) % len(table)]


def make_synthetic_fixture(
    target: dict[str, Any],
    declarations: dict[str, Any],
    carrier_row_count: int,
    seed: int,
) -> dict[str, Any]:
    cbuffers: dict[str, list[list[float]]] = {}
    for register_text, count in declarations["constantBufferFloat4Counts"].items():
        register = int(register_text)
        if seed < 3:
            fill = (1.0, 0.5, 0.25)[seed]
            rows = [[fill, fill, fill, fill] for _ in range(count)]
        else:
            rows = [
                [_pattern(seed, row + register * 31, lane) for lane in range(4)]
                for row in range(count)
            ]
        cbuffers[register_text] = rows
    cbuffers["0"][0] = [1.0, 1.0, 1.0, 1.0]
    if len(cbuffers["0"]) > 1:
        cbuffers["0"][1] = [1.0, 1.0, 1.0, 1.0]
    if "2" in cbuffers:
        cbuffers["2"] = [
            [0.5, -0.5, 0.5, 0.5],
            [1.0, 0.0, 1.0, 0.0],
            [1.0, 1.0, 1.0, 1.0],
            [1.0, 1.0, 1.0, 1.0],
        ][: len(cbuffers["2"])]

    carrier = [[0.5, 0.5, 0.5, 1.0] for _ in range(carrier_row_count)]
    defaults = {
        0: [0.0, 0.0, 1.0, 1.0],
        1: [1.0, 1.0, 1.0, 1.0],
        2: [0.37, 0.61, 0.0, 1.0],
        3: [1.0, 1.0, 1.0, 1.0],
        4: [0.37, 0.61, 0.5, 0.5],
        5: [0.0, 0.0, 0.0, 1.0],
        6: [0.0, 0.0, 1.0, 1.0],
        7: [0.5, 0.5, 0.5, 1.0],
    }
    for register, value in defaults.items():
        if register < len(carrier):
            carrier[register] = value

    textures = {}
    for register in declarations["textureRegisters"]:
        base = 0.55 + 0.05 * ((seed + register) % 5)
        textures[str(register)] = [base, min(base + 0.1, 1.0), 0.8, 1.0]
    return {
        "fidelity": "SYNTHETIC_STRUCTURAL_VALUES_NOT_SOURCE_MIC_EVALUATION",
        "seed": seed,
        "constantBuffers": cbuffers,
        "carrierRows": carrier,
        "textures": textures,
        "nativeBindingSemanticSha256": target["nativeShaderObjectBinding"]["bindingSemanticSha256"],
    }


def rt0_written_nonzero(outputs: list[list[float]]) -> bool:
    if not outputs:
        return False
    if not all(math.isfinite(value) for row in outputs for value in row):
        return False
    row = outputs[0]
    if all(abs(value - SENTINEL) <= FLOAT_EPSILON for value in row):
        return False
    return any(abs(value) > FLOAT_EPSILON for value in row)


def sensitivity_mutations(
    fixture: dict[str, Any], target: dict[str, Any]
) -> Iterable[tuple[dict[str, Any], dict[str, Any]]]:
    cb0 = fixture["constantBuffers"]["0"]
    require(cb0 and len(cb0[0]) == 4, "external opacity slot is absent")
    changed = copy.deepcopy(fixture)
    before = changed["constantBuffers"]["0"][0][0]
    after = before * 0.5 if abs(before) > FLOAT_EPSILON else 0.5
    changed["constantBuffers"]["0"][0][0] = after
    yield changed, {
        "kind": "CONSTANT_BUFFER",
        "semanticRole": "UE3_EXTERNAL_OPACITY",
        "register": 0,
        "row": 0,
        "lane": 0,
        "before": before,
        "after": after,
    }


def find_baseline_and_sensitivity(
    replay: GenericWarpReplay,
    target: dict[str, Any],
    pixel_dxbc: bytes,
    carrier_dxbc: bytes,
    declarations: dict[str, Any],
    carrier_row_count: int,
    render_target_count: int,
) -> tuple[dict[str, Any], list[list[float]], dict[str, Any], list[list[float]]]:
    baseline_fixture = None
    baseline_outputs = None
    for seed in range(64):
        fixture = make_synthetic_fixture(
            target, declarations, carrier_row_count, seed
        )
        outputs = replay.run(
            pixel_dxbc,
            carrier_dxbc,
            fixture,
            declarations,
            render_target_count,
        )
        if rt0_written_nonzero(outputs):
            baseline_fixture = fixture
            baseline_outputs = outputs
            break
    require(
        baseline_fixture is not None and baseline_outputs is not None,
        f"no nonzero synthetic RT0 fixture found: {target['targetId']}",
    )
    baseline_rt0 = baseline_outputs[0]
    for changed, mutation in sensitivity_mutations(baseline_fixture, target):
        outputs = replay.run(
            pixel_dxbc,
            carrier_dxbc,
            changed,
            declarations,
            render_target_count,
        )
        actual_rt0 = outputs[0]
        deltas = [abs(actual - baseline) for actual, baseline in zip(actual_rt0, baseline_rt0)]
        changed_lanes = [index for index, delta in enumerate(deltas) if delta > FLOAT_EPSILON]
        if (
            all(math.isfinite(value) for row in outputs for value in row)
            and all(abs(value - SENTINEL) > FLOAT_EPSILON for value in actual_rt0)
            and changed_lanes
        ):
            mutation["baselineRt0"] = baseline_rt0
            mutation["mutatedRt0"] = actual_rt0
            mutation["absoluteLaneDeltas"] = deltas
            mutation["changedRt0Lanes"] = changed_lanes
            mutation["outputAlphaChanged"] = 3 in changed_lanes
            return baseline_fixture, baseline_outputs, mutation, outputs
    raise RuntimeError(f"external opacity input does not perturb RT0: {target['targetId']}")


def cache_identity_matches(
    cache_path: Path, input_receipt: dict[str, Any]
) -> dict[str, Any]:
    expected = input_receipt["officialRefShaderCache"]["package"]
    require(cache_path.is_file(), f"pinned RefShaderCache is missing: {cache_path}")
    actual = {
        "fileName": cache_path.name,
        "physicalByteSize": cache_path.stat().st_size,
        "rawSha256": sha256_file(cache_path),
    }
    require(
        actual["fileName"] == expected["fileName"]
        and actual["physicalByteSize"] == expected["physicalByteSize"]
        and actual["rawSha256"] == expected["rawSha256"],
        "pinned RefShaderCache identity changed",
    )
    return actual


def build_receipt(
    input_path: Path,
    cache_path: Path,
    compiler_path: Path,
    d3d11_path: Path,
    warp_path: Path,
) -> dict[str, Any]:
    input_receipt = read_json(input_path)
    targets = build_replay_plan(validate_input_receipt(input_receipt))
    require(targets, "G03-4 exact target denominator is empty")
    cache_identity = cache_identity_matches(cache_path, input_receipt)
    cache = package_tables(cache_path)
    compiler = CarrierCompiler(compiler_path)
    disassembler = D3DDisassembler(compiler_path)
    d3d11_identity = external_binary_identity(d3d11_path)
    warp_identity = external_binary_identity(warp_path)
    replay = GenericWarpReplay()
    rows = []
    try:
        for target in targets:
            pixel_dxbc = rehydrate_dxbc(cache, target)
            chunks = dxbc_chunks(pixel_dxbc)
            require("ISGN" in chunks and "OSGN" in chunks, "pixel shader signature chunks are absent")
            input_signature = parse_signature(chunks["ISGN"])
            output_signature = parse_signature(chunks["OSGN"])
            carrier_source, carrier_row_count = build_carrier_source(input_signature)
            carrier_dxbc, carrier_identity = compiler.compile(
                carrier_source, f"ue3-material-carrier-{target['targetId']}"
            )
            carrier_chunks = dxbc_chunks(carrier_dxbc)
            require("OSGN" in carrier_chunks, "compiled carrier output signature is absent")
            carrier_output_signature = parse_signature(carrier_chunks["OSGN"])
            carrier_link = close_carrier_signature(
                input_signature, carrier_output_signature
            )
            declarations = parse_runtime_declarations(
                disassembler.disassemble(pixel_dxbc)
            )
            native_closure = validate_native_binding_closure(
                target["nativeShaderObjectBinding"], declarations
            )
            render_target_count = output_register_count(output_signature)
            (
                baseline_fixture,
                baseline_outputs,
                mutation,
                mutation_outputs,
            ) = find_baseline_and_sensitivity(
                replay,
                target,
                pixel_dxbc,
                carrier_dxbc,
                declarations,
                carrier_row_count,
                render_target_count,
            )
            baseline_mrt = validate_mrt_contract(
                baseline_outputs, output_signature
            )
            mutation_mrt = validate_mrt_contract(
                mutation_outputs, output_signature
            )
            native = target["nativeShaderObjectBinding"]
            rows.append(
                {
                    "targetId": target["targetId"],
                    "familyId": target["familyId"],
                    "rendererType": target["rendererType"],
                    "shaderType": target["cookedPixelShader"]["shaderType"],
                    "shaderIdHex": target["cookedPixelShader"]["shaderIdHex"],
                    "dxbc": target["cookedPixelShader"]["dxbc"],
                    "pixelShaderCreation": "RAW_EXACT_DXBC_TO_ID3D11DEVICE_CREATEPIXELSHADER",
                    "carrierVertexShader": {
                        **carrier_identity,
                        "constantBufferFloat4Count": carrier_row_count,
                        "inputSignature": public_signature(input_signature),
                        "outputSignature": public_signature(
                            carrier_output_signature
                        ),
                        "signatureClosure": carrier_link,
                    },
                    "outputSignature": public_signature(output_signature),
                    "renderTargetCount": render_target_count,
                    "runtimeDeclarations": declarations,
                    "nativeBinding": {
                        "bindingSemanticSha256": native["bindingSemanticSha256"],
                        **native_closure,
                    },
                    "fixedInputFixture": {
                        "fidelity": baseline_fixture["fidelity"],
                        "seed": baseline_fixture["seed"],
                        "constantBufferValueSource": "DETERMINISTIC_SYNTHETIC_VALUES_PACKED_THROUGH_EXACT_NATIVE_CB0_EXTENTS",
                        "textureValueSource": "DETERMINISTIC_ONE_BY_ONE_RGBA32F_SYNTHETIC_VALUES_AT_ALL_DECLARED_SRVS",
                        "samplerValueSource": "POINT_CLAMP_SYNTHETIC_NOT_SOURCE_SAMPLER_EVIDENCE",
                        "engineFixture": "DECLARED_CB2_AND_UNOWNED_SCENE_DEPTH_SLOTS_BOUND_WITH_SYNTHETIC_VALUES",
                    },
                    "baseline": {
                        "rt0Nonzero": True,
                        "mrtFloat4": baseline_outputs,
                        "mrtContract": baseline_mrt,
                    },
                    "externalOpacityInputSensitivity": {
                        "mutation": mutation,
                        "mrtFloat4": mutation_outputs,
                        "mrtContract": mutation_mrt,
                        "pass": True,
                    },
                    "structuralFixedInputReplayAdmission": True,
                    "sourceValueReplayAdmission": False,
                    "actualVfPassAdmission": False,
                    "runtimeAdmission": False,
                    "visualAdmission": False,
                }
            )
    finally:
        replay.close()

    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "identity": {
            "stage": "G03_4_STRUCTURAL_FIXED_INPUT_REPLAY",
            "tool": {
                "repoRelativePath": SCRIPT_PATH.relative_to(REPOSITORY_ROOT).as_posix(),
                "normalizedSha256": normalized_text_sha256(SCRIPT_PATH),
            },
        },
        "input": {
            "repoRelativePath": input_path.relative_to(REPOSITORY_ROOT).as_posix(),
            "receiptSha256": input_receipt["receiptSha256"],
            "pinnedRefShaderCache": cache_identity,
        },
        "backend": {
            "name": "D3D11_WARP_RAW_COOKED_PIXEL_DXBC_DYNAMIC_SIGNATURE_CARRIER",
            "featureLevel": replay.feature_level,
            "d3d11": d3d11_identity,
            "warp": warp_identity,
            "d3dcompiler": compiler.identity,
            "renderTargetFormat": "DXGI_FORMAT_R32G32B32A32_FLOAT",
            "textureFormat": "DXGI_FORMAT_R32G32B32A32_FLOAT",
        },
        "targetReplays": rows,
        "summary": {
            "exactInputTargetCount": len(targets),
            "rawCreatePixelShaderPassCount": len(rows),
            "dynamicCarrierLinkPassCount": len(rows),
            "nativeBindingStructuralReplayPassCount": len(rows),
            "nonzeroRt0BaselineCount": sum(row["baseline"]["rt0Nonzero"] for row in rows),
            "externalOpacityInputSensitivityPassCount": sum(row["externalOpacityInputSensitivity"]["pass"] for row in rows),
            "sourceValueReplayAdmissionCount": 0,
            "actualVfPassAdmissionCount": 0,
            "runtimeAdmissionCount": 0,
            "visualAdmissionCount": 0,
            "result": "PASS_G03_4_STRUCTURAL_FIXED_INPUT_REPLAY_SOURCE_VALUE_REPLAY_BLOCKED",
        },
        "decision": {
            "structuralFixedInputReplayAdmission": True,
            "sourceValueReplayAdmission": False,
            "actualVfPassAdmission": False,
            "runtimeAdmission": False,
            "visualAdmission": False,
            "sourceValueReplayBlockers": [
                "UNIFORM_EXPRESSION_FOLDEDMATH_ORDINAL_0_SEMANTICS_UNPROVEN",
                "PARENT_DEFAULT_TEXTURE_CLOSURE_INCOMPLETE",
                "SOURCE_SAMPLER_FILTER_AND_ADDRESS_EVIDENCE_INCOMPLETE",
            ],
            "familyLiteFallbackPreserved": True,
            "nextGate": "SOURCE_VALUE_EXPRESSION_TEXTURE_SAMPLER_REPLAY_THEN_ACTUAL_VF_PASS_ADMISSION",
        },
    }
    seal(receipt)
    return receipt


def validate_output_receipt(receipt: dict[str, Any]) -> None:
    require(receipt.get("schema") == SCHEMA, "G03-4 receipt schema changed")
    require(receipt.get("formatVersion") == FORMAT_VERSION, "G03-4 receipt version changed")
    claimed = receipt.get("receiptSha256")
    unsigned = dict(receipt)
    unsigned.pop("receiptSha256", None)
    require(claimed == canonical_json_sha256(unsigned), "G03-4 receipt seal changed")
    rows = receipt.get("targetReplays")
    require(isinstance(rows, list) and rows, "G03-4 target replay denominator is empty")
    summary = receipt["summary"]
    denominator = summary["exactInputTargetCount"]
    require(len(rows) == denominator, "G03-4 replay denominator changed")
    require(
        summary["rawCreatePixelShaderPassCount"]
        == summary["dynamicCarrierLinkPassCount"]
        == summary["nativeBindingStructuralReplayPassCount"]
        == summary["nonzeroRt0BaselineCount"]
        == summary["externalOpacityInputSensitivityPassCount"]
        == denominator,
        "G03-4 structural replay closure changed",
    )
    for row in rows:
        require(row["baseline"]["rt0Nonzero"], "G03-4 RT0 baseline became zero")
        require(row["externalOpacityInputSensitivity"]["pass"], "G03-4 external-opacity sensitivity failed")
        require(
            row["externalOpacityInputSensitivity"]["mutation"]["semanticRole"]
            == "UE3_EXTERNAL_OPACITY",
            "G03-4 sensitivity no longer uses the external-opacity slot",
        )
        require(
            row["carrierVertexShader"]["signatureClosure"]["pass"],
            "G03-4 carrier signature closure failed",
        )
        require(row["nativeBinding"]["pass"], "G03-4 native binding closure failed")
        require(
            row["baseline"]["mrtContract"]["pass"]
            and row["externalOpacityInputSensitivity"]["mrtContract"]["pass"],
            "G03-4 MRT contract failed",
        )
        require(
            row["fixedInputFixture"]["fidelity"]
            == "SYNTHETIC_STRUCTURAL_VALUES_NOT_SOURCE_MIC_EVALUATION",
            "synthetic fixture was mislabeled as source exact",
        )
        require(
            row["structuralFixedInputReplayAdmission"]
            and not row["sourceValueReplayAdmission"]
            and not row["actualVfPassAdmission"]
            and not row["runtimeAdmission"]
            and not row["visualAdmission"],
            "G03-4 admission boundary changed",
        )
    decision = receipt["decision"]
    require(decision["familyLiteFallbackPreserved"], "family-lite fallback was removed")
    require(
        decision["structuralFixedInputReplayAdmission"]
        and not decision["sourceValueReplayAdmission"]
        and not decision["actualVfPassAdmission"]
        and not decision["runtimeAdmission"]
        and not decision["visualAdmission"],
        "G03-4 decision overclaims runtime/source/visual admission",
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, default=DEFAULT_INPUT)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--cache", type=Path, default=DEFAULT_CACHE)
    parser.add_argument("--d3dcompiler", type=Path, default=DEFAULT_D3DCOMPILER)
    parser.add_argument("--d3d11", type=Path, default=DEFAULT_D3D11)
    parser.add_argument("--warp", type=Path, default=DEFAULT_WARP)
    parser.add_argument("--check", action="store_true")
    arguments = parser.parse_args(argv)
    receipt = build_receipt(
        arguments.input.resolve(),
        arguments.cache.resolve(),
        arguments.d3dcompiler.resolve(),
        arguments.d3d11.resolve(),
        arguments.warp.resolve(),
    )
    validate_output_receipt(receipt)
    if arguments.check:
        require(arguments.output.is_file(), f"G03-4 receipt is missing: {arguments.output}")
        require(read_json(arguments.output) == receipt, "G03-4 receipt is stale")
    else:
        write_json_atomic(arguments.output, receipt)
    print(
        "PASS: G03-4 structural fixed-input replay "
        f"targets={receipt['summary']['exactInputTargetCount']} "
        "sourceValueReplay=blocked runtimeAdmission=0 visualAdmission=0"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
