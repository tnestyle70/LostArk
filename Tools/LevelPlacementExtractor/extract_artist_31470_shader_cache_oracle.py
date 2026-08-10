#!/usr/bin/env python3
"""Extract fail-closed Artist F ShaderCache evidence from installed UE3 packages.

This tool intentionally stops at the boundary proven by the serialized bytes.
It can decode the native shader-type/code table and its raw LZ4 DXBC payloads,
but it does not call an unparsed material-shader-map tail a material binding.
"""

from __future__ import annotations

import argparse
import bisect
import ctypes
import gc
import hashlib
import json
import re
import struct
from collections import Counter
from pathlib import Path
from typing import Any, Callable

from extract_ue3_effect_material_closure import load_package, tagged_value
from extract_ue3_material_graph import (
    MATERIAL_OUTPUTS,
    classify_topology,
    expression_input,
    expression_inputs,
    property_value,
)
from extract_ue3_placements import (
    LOSTARK_KR_AES_KEY,
    decompress_lz4_block,
    package_ref_name,
    package_ref_path,
    parse_tagged_properties,
)


SCHEMA = "lostark.artist-31470-shader-cache-oracle-receipt"
FORMAT_VERSION = 1
REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_MATERIAL_CONTRACT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.typed-material-evidence-contract.json"
)
DEFAULT_OUTPUT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.shader-cache-oracle.receipt.json"
)
DEFAULT_INSTALL_ROOT = Path(
    r"C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC"
)
DEFAULT_GLOBAL_MATERIAL_PACKAGE = DEFAULT_INSTALL_ROOT / "DKV6KRSCXY3T6D9CJIK3G.upk"
DEFAULT_SHADER_CACHE_PACKAGE = DEFAULT_INSTALL_ROOT / "9XUFAXIP8BXBAP1NIEG66EF.upk"
DEFAULT_SOURCE_PACKAGE_ROOT = DEFAULT_INSTALL_ROOT / "Packages"
DEFAULT_MATERIAL_INVENTORY_ROOT = Path(
    r"C:\Users\user\Desktop\Resource_LostArk\05_Reports\EffectExtraction\DIMENSIONMASTER\materials"
)
DEFAULT_D3DCOMPILER = Path(
    r"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\d3dcompiler_47.dll"
)


EXPECTED_EXTERNAL_IDENTITIES = {
    "globalMaterialPackage": {
        "fileName": "DKV6KRSCXY3T6D9CJIK3G.upk",
        "byteSize": 141_154_941,
        "sha256": "c0c3e35b48d8589d2e5014c99c64c0c32e05eace7ae02cfc8e6566f4eaf40150",
        "packageVersion": 868,
        "logicalByteSize": 602_422_069,
        "exportCount": 1_323_421,
    },
    "shaderCachePackage": {
        "fileName": "9XUFAXIP8BXBAP1NIEG66EF.upk",
        "byteSize": 270_965_156,
        "sha256": "be77e8af4443c4cca5614bec0545c0c735ab04a8b68a3781fb9dfb5a5f2123ad",
        "packageVersion": 868,
        "logicalByteSize": 943_207_579,
        "exportCount": 1_596,
    },
    "d3dcompiler": {
        "fileName": "d3dcompiler_47.dll",
        "byteSize": 4_916_800,
        "sha256": "ce013eb1639f8e2620a509e73b33029108f55a293e304e33e38b72fd65c531b8",
        "fileVersion": "10.0.22621.5040",
    },
}


# familyId, G03 exact graph identity, installed global-package identity, class.
MATERIAL_TARGETS = (
    ("material-family-097bd8d9597721b5", "fx_m.fx_m_pa_spritewave_01_tr", "fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr", "material"),
    ("material-family-0f9365f5179c729b", "fx_m.fx_k_me_makeflow_02_tr", "fx_m_mi_02.fx_m.fx_k_me_makeflow_02_tr", "material"),
    ("material-family-2174c88b2cbf6c72", "fx_m.fx_m_pa_skull_02_tr", "fx_m_mi_03.fx_m.fx_m_pa_skull_02_tr", "material"),
    ("material-family-2c00ce5593538d7c", "fx_m.fx_o_transition_01_ma", "fx_m_mi_03.fx_m.fx_o_transition_01_ma", "material"),
    ("material-family-344b3aeb8eb1418d", "fx_mm.fx_mm_onelayerdistortion_03_ad", "fx_mastermaterial.fx_mm.fx_mm_onelayerdistortion_03_ad", "material"),
    ("material-family-3919858495713fad", "fx_m.fx_k_me_makeflow_03_tr", "fx_m_mi_k_00.fx_m.fx_k_me_makeflow_03_tr", "material"),
    ("material-family-472b1be487b92e70", "fx_mm.fx_mm_de_basic_01_tr", "fx_mastermaterial.fx_mm.fx_mm_de_basic_01_tr", "decalmaterial"),
    ("material-family-4e1ecdcff38a53d1", "fx_m.fx_m_pa_missiletrail_01_tr", "fx_m_mi_03.fx_m.fx_m_pa_missiletrail_01_tr", "material"),
    ("material-family-5fc89efe09353236", "fx_m.fx_e_me_rock_01_ma", "fx_m_mi_05.fx_m.fx_e_me_rock_01_ma", "material"),
    ("material-family-600687e6c2277444", "fx_mm.fx_mm_basic_01_tr", "fx_mastermaterial.fx_mm.fx_mm_basic_01_tr", "material"),
    ("material-family-61182c380df3a6cd", "bfx_m.bfx_d_pa_spla_05_tr", "bfx_m_mi_00.bfx_m.bfx_d_pa_spla_05_tr", "material"),
    ("material-family-89af5c77d8e35f99", "fx_m.fx_m_me_watertrail_01_tr", "fx_m_mi_03.fx_m.fx_m_me_watertrail_01_tr", "material"),
    ("material-family-918f4ae1ed4d8a70", "fx_m.fx_j_maskedrib_01_tr", "fx_m_mi_02.fx_m.fx_j_maskedrib_01_tr", "material"),
    ("material-family-9fce58ec032dee02", "bfx_m.bfx_d_pa_circ_01_ad", "bfx_m_mi_00.bfx_m.bfx_d_pa_circ_01_ad", "material"),
    ("material-family-b53107e635922285", "fx_m.fx_d_pa_sqc_01_tr", "fx_m_mi_02.fx_m.fx_d_pa_sqc_01_tr", "material"),
    ("material-family-b6660d647ef8c502", "fx_m.fx_c_po_zoomblur_01", "fx_post.fx_m.fx_c_po_zoomblur_01", "material"),
    ("material-family-bcd3ca469af508c1", "fx_m.fx_m_me_trail_02_tr", "fx_m_mi_03.fx_m.fx_m_me_trail_02_tr", "material"),
    ("material-family-ce499c1d6d0fddcc", "fx_m.fx_d_me_flow_02_tr", "fx_m_mi_00.fx_m.fx_d_me_flow_02_tr", "material"),
    ("material-family-ce6de128d3435d31", "fx_m.fx_f_pa_wind_05_tr", "fx_m_mi_01.fx_m.fx_f_pa_wind_05_tr", "material"),
    ("material-family-d0f9d7bb8b80fee0", "fx_m.fx_d_pa_master_01_tr", "fx_m_mi_00.fx_m.fx_d_pa_master_01_tr", "material"),
    ("material-family-ee42f716afdf6145", "fx_m.fx_e_pa_fluid_02_tr", "fx_m_mi_05.fx_m.fx_e_pa_fluid_02_tr", "material"),
    ("material-family-f1667adae7da4bdd", "fx_m.fx_d_de_master_01_tr", "fx_m_mi_04.fx_m.fx_d_de_master_01_tr", "material"),
    ("material-family-fcc81a924169d053", "fx_m.fx_k_pa_flowmask_01_tr", "fx_m_mi_k_00.fx_m.fx_k_pa_flowmask_01_tr", "material"),
)

CACHE_CANDIDATES = (
    "sc_lv_customizingtool_classselect_sl01",
    "sc_lv_customizingtool_classselect_sl02",
    "sc_lv_customizingtool_classselect_sl03",
    "sc_lv_customizingtool_classselect_sl06",
    "sc_lv_customizingtool_classselect_sl08",
    "sc_lv_eflobby_sl_class",
    "sc_lv_lobby_classselect_sl01",
    "sc_lv_lobby_classselect_sl02",
    "sc_lv_lobby_classselect_sl03",
    "sc_lv_lobby_classselect_sl06",
    "sc_lv_lobby_classselect_sl08",
)
PRIMARY_CACHE = "sc_lv_eflobby_sl_class"
PRIMARY_EXPECTED = {
    "exportIndex": 318,
    "serialOffset": 193_280_597,
    "serialSize": 1_034_215,
    "serialSha256": "ad990dc604d784cef5aebee575aed3798e6acb7ab78b7654a0e4c5cc8f2a2b28",
    "shaderTypeGroupCount": 32,
    "compressedCodeRecordCount": 271,
    "uniqueDxbcCount": 240,
}
MATERIAL_RECIPE_PROJECTION_SHA256 = "43db9f75e2f93906ac9a53ac21f46efbb8a2a3878066b7d33415c76213590ee8"

BLOCKERS = (
    "ARITHMETIC_GRAPH_TO_SHADER_INVERSION_UNPROVEN",
    "DETERMINISTIC_NUMERIC_SAMPLE_ORACLE_UNAVAILABLE",
    "DUPLICATE_MATERIAL_INVENTORY_NOT_INSTALLATION_EXHAUSTIVE",
    "DXBC_REGISTER_BINDINGS_NOT_MATERIAL_PARAMETER_NAMES",
    "MATERIAL_NATIVE_KEY_SEMANTICS_UNPROVEN",
    "MIC_STATIC_PARAMETER_SET_NATIVE_TAIL_UNPARSED",
    "MATERIAL_SHADER_MAP_KEY_UNRESOLVED",
    "MATERIAL_SHADER_PERMUTATION_JOIN_UNPROVEN",
    "MIC_TAIL_TO_SHADER_OBJECT_IDENTITY_UNRESOLVED",
    "SHADER_OBJECT_AND_MATERIAL_MAP_NATIVE_TAIL_UNPARSED",
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def raw_sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def raw_file_sha256(path: Path) -> str:
    return raw_sha256(path.read_bytes())


def canonical_text_bytes(path: Path) -> bytes:
    text = path.read_text(encoding="utf-8-sig")
    return text.replace("\r\n", "\n").replace("\r", "\n").encode("utf-8")


def canonical_text_sha256(path: Path) -> str:
    return raw_sha256(canonical_text_bytes(path))


def canonical_json_sha256(value: Any) -> str:
    return raw_sha256(
        json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")).encode(
            "utf-8"
        )
    )


def read_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8-sig"))
    require(isinstance(value, dict), f"expected JSON object: {path}")
    return value


def seal_receipt(receipt: dict[str, Any]) -> None:
    receipt.pop("receiptSha256", None)
    receipt["receiptSha256"] = canonical_json_sha256(receipt)


def verify_external_file(path: Path, expected: dict[str, Any]) -> bytes:
    require(path.is_file(), f"external evidence is missing: {path}")
    data = path.read_bytes()
    require(path.name.casefold() == expected["fileName"].casefold(), "external file name mismatch")
    require(len(data) == expected["byteSize"], f"external file size mismatch: {path.name}")
    require(raw_sha256(data) == expected["sha256"], f"external file SHA mismatch: {path.name}")
    return data


def windows_file_version(path: Path) -> str:
    version = ctypes.WinDLL("version", use_last_error=True)
    get_size = version.GetFileVersionInfoSizeW
    get_size.argtypes = [ctypes.c_wchar_p, ctypes.POINTER(ctypes.c_uint32)]
    get_size.restype = ctypes.c_uint32
    get_info = version.GetFileVersionInfoW
    get_info.argtypes = [ctypes.c_wchar_p, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_void_p]
    get_info.restype = ctypes.c_int
    query = version.VerQueryValueW
    query.argtypes = [ctypes.c_void_p, ctypes.c_wchar_p, ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(ctypes.c_uint32)]
    query.restype = ctypes.c_int

    ignored = ctypes.c_uint32()
    size = get_size(str(path), ctypes.byref(ignored))
    require(size > 0, "D3D compiler version resource is unavailable")
    buffer = ctypes.create_string_buffer(size)
    require(bool(get_info(str(path), 0, size, buffer)), "GetFileVersionInfoW failed")
    pointer = ctypes.c_void_p()
    length = ctypes.c_uint32()
    require(bool(query(buffer, "\\", ctypes.byref(pointer), ctypes.byref(length))), "VerQueryValueW failed")
    require(length.value >= 52, "VS_FIXEDFILEINFO is truncated")
    values = struct.unpack_from("<13I", ctypes.string_at(pointer, length.value))
    require(values[0] == 0xFEEF04BD, "VS_FIXEDFILEINFO signature mismatch")
    ms, ls = values[2], values[3]
    return f"{ms >> 16}.{ms & 0xFFFF}.{ls >> 16}.{ls & 0xFFFF}"


class D3DDisassembler:
    def __init__(self, dll_path: Path) -> None:
        expected = EXPECTED_EXTERNAL_IDENTITIES["d3dcompiler"]
        data = verify_external_file(dll_path, expected)
        version = windows_file_version(dll_path)
        require(version == expected["fileVersion"], "D3D compiler file version mismatch")
        self.identity = {
            "fileName": dll_path.name,
            "byteSize": len(data),
            "rawSha256": raw_sha256(data),
            "fileVersion": version,
            "hashRole": "EXTERNAL_RAW_BYTES",
        }
        self._dll = ctypes.WinDLL(str(dll_path))
        self._disassemble = self._dll.D3DDisassemble
        self._disassemble.argtypes = [
            ctypes.c_void_p,
            ctypes.c_size_t,
            ctypes.c_uint32,
            ctypes.c_char_p,
            ctypes.POINTER(ctypes.c_void_p),
        ]
        self._disassemble.restype = ctypes.c_long

    def __call__(self, bytecode: bytes) -> dict[str, Any]:
        source = ctypes.create_string_buffer(bytecode)
        output = ctypes.c_void_p()
        result = self._disassemble(source, len(bytecode), 0, None, ctypes.byref(output))
        require(result >= 0 and bool(output.value), f"D3DDisassemble failed: 0x{result & 0xFFFFFFFF:08x}")
        table = ctypes.cast(
            output, ctypes.POINTER(ctypes.POINTER(ctypes.c_void_p))
        ).contents
        get_pointer = ctypes.WINFUNCTYPE(ctypes.c_void_p, ctypes.c_void_p)(table[3])
        get_size = ctypes.WINFUNCTYPE(ctypes.c_size_t, ctypes.c_void_p)(table[4])
        release = ctypes.WINFUNCTYPE(ctypes.c_ulong, ctypes.c_void_p)(table[2])
        try:
            text = ctypes.string_at(get_pointer(output), get_size(output)).decode(
                "utf-8", "strict"
            )
        finally:
            release(output)
        return summarize_disassembly(text)


PROFILE_PATTERN = re.compile(r"^(?:ps|vs|gs|hs|ds|cs)_\d+_\d+$")


def summarize_disassembly(text: str) -> dict[str, Any]:
    lines = [line.strip() for line in text.splitlines()]
    profiles = [line for line in lines if PROFILE_PATTERN.fullmatch(line)]
    require(len(profiles) == 1, "DXBC disassembly profile is ambiguous")
    declarations = sorted(
        {
            line
            for line in lines
            if line.startswith("dcl_constantbuffer ")
            or line.startswith("dcl_sampler ")
            or line.startswith("dcl_resource_")
            or line.startswith("dcl_uav_")
        }
    )
    constant_buffers = [line for line in declarations if line.startswith("dcl_constantbuffer ")]
    samplers = [line for line in declarations if line.startswith("dcl_sampler ")]
    resources = [line for line in declarations if line.startswith("dcl_resource_")]
    uavs = [line for line in declarations if line.startswith("dcl_uav_")]
    return {
        "profile": profiles[0],
        "constantBufferDeclarations": constant_buffers,
        "samplerDeclarations": samplers,
        "resourceDeclarations": resources,
        "uavDeclarations": uavs,
        "declarationSha256": canonical_json_sha256(declarations),
    }


def validate_dxbc_container(bytecode: bytes) -> dict[str, Any]:
    require(len(bytecode) >= 36, "DXBC container is truncated")
    require(bytecode[:4] == b"DXBC", "DXBC magic is missing")
    declared_size = struct.unpack_from("<I", bytecode, 24)[0]
    require(declared_size == len(bytecode), "DXBC total size mismatch")
    chunk_count = struct.unpack_from("<I", bytecode, 28)[0]
    require(0 < chunk_count <= 64, "DXBC chunk count is invalid")
    require(32 + chunk_count * 4 <= len(bytecode), "DXBC chunk table is truncated")
    chunks: list[dict[str, Any]] = []
    intervals: list[tuple[int, int]] = []
    for index in range(chunk_count):
        offset = struct.unpack_from("<I", bytecode, 32 + index * 4)[0]
        require(offset + 8 <= len(bytecode), "DXBC chunk header is outside container")
        size = struct.unpack_from("<I", bytecode, offset + 4)[0]
        end = offset + 8 + size
        require(end <= len(bytecode), "DXBC chunk payload is outside container")
        intervals.append((offset, end))
        chunks.append(
            {
                "fourCc": bytecode[offset : offset + 4].decode("ascii", "strict"),
                "offset": offset,
                "byteSize": size,
                "payloadSha256": raw_sha256(bytecode[offset + 8 : end]),
            }
        )
    require(len(set(intervals)) == len(intervals), "DXBC chunk table contains duplicates")
    ordered = sorted(intervals)
    require(all(a[1] <= b[0] for a, b in zip(ordered, ordered[1:])), "DXBC chunks overlap")
    return {
        "byteSize": len(bytecode),
        "sha256": raw_sha256(bytecode),
        "chunkCount": chunk_count,
        "chunks": chunks,
    }


def read_fname(serial: bytes, offset: int, names: list[str]) -> tuple[str, int, int]:
    require(offset + 8 <= len(serial), "ShaderCache FName is truncated")
    name_index, number = struct.unpack_from("<ii", serial, offset)
    require(0 <= name_index < len(names), "ShaderCache FName index is invalid")
    name = names[name_index]
    if number > 0:
        name = f"{name}_{number - 1}"
    return name, number, offset + 8


def parse_shader_cache_serial(
    serial: bytes,
    names: list[str],
    disassemble: Callable[[bytes], dict[str, Any]] | None = None,
) -> dict[str, Any]:
    require(len(serial) >= 29, "ShaderCache serial is truncated")
    require(struct.unpack_from("<i", serial, 0)[0] == -1, "ShaderCache net index mismatch")
    property_name, property_number, offset = read_fname(serial, 4, names)
    require(property_name.casefold() == "none" and property_number == 0, "ShaderCache property terminator mismatch")
    native_header = struct.unpack_from("<I", serial, offset)[0]
    offset += 4
    require(native_header == 0, "ShaderCache native header revision is unsupported")
    platform = serial[offset]
    offset += 1
    group_count = struct.unpack_from("<I", serial, offset)[0]
    offset += 4
    require(0 < group_count <= 4096, "ShaderCache shader-type group count is invalid")

    groups: list[dict[str, Any]] = []
    records: list[dict[str, Any]] = []
    compressed_intervals: list[tuple[int, int]] = []
    for group_index in range(group_count):
        group_offset = offset
        shader_type, shader_type_number, offset = read_fname(serial, offset, names)
        require(shader_type_number == 0, "numbered ShaderCache shader type is unsupported")
        require(offset + 4 <= len(serial), "ShaderCache descriptor count is truncated")
        descriptor_count = struct.unpack_from("<I", serial, offset)[0]
        offset += 4
        require(0 < descriptor_count <= 100_000, "ShaderCache descriptor count is invalid")
        descriptor_start = offset
        descriptor_size = descriptor_count * 24
        require(offset + descriptor_size + 4 <= len(serial), "ShaderCache descriptor table is truncated")
        descriptors = [serial[offset + i * 24 : offset + (i + 1) * 24] for i in range(descriptor_count)]
        offset += descriptor_size
        code_count = struct.unpack_from("<I", serial, offset)[0]
        offset += 4
        require(code_count == descriptor_count, "ShaderCache descriptor/code count mismatch")

        group_records: list[dict[str, Any]] = []
        for code_index, descriptor in enumerate(descriptors):
            require(offset + 8 <= len(serial), "ShaderCache compressed code header is truncated")
            uncompressed_size, compressed_size = struct.unpack_from("<II", serial, offset)
            code_header_offset = offset
            offset += 8
            require(32 <= uncompressed_size <= 64 * 1024 * 1024, "ShaderCache uncompressed code size is invalid")
            require(0 < compressed_size <= len(serial) - offset, "ShaderCache compressed code size is invalid")
            compressed = serial[offset : offset + compressed_size]
            compressed_intervals.append((offset, offset + compressed_size))
            offset += compressed_size
            bytecode = decompress_lz4_block(compressed, uncompressed_size)
            dxbc = validate_dxbc_container(bytecode)
            reflection = disassemble(bytecode) if disassemble is not None else None
            row = {
                "groupIndex": group_index,
                "shaderType": shader_type,
                "codeIndex": code_index,
                "descriptorOffset": descriptor_start + code_index * 24,
                "descriptorSha256": raw_sha256(descriptor),
                "shaderIdCandidateHex": descriptor[:16].hex(),
                "opaqueDescriptorTailHex": descriptor[16:].hex(),
                "codeHeaderOffset": code_header_offset,
                "compressedOffset": code_header_offset + 8,
                "compressedByteSize": compressed_size,
                "compressedSha256": raw_sha256(compressed),
                "uncompressedByteSize": uncompressed_size,
                "dxbcSha256": dxbc["sha256"],
                "dxbcChunkCount": dxbc["chunkCount"],
                "disassembly": reflection,
            }
            group_records.append(row)
            records.append(row)
        groups.append(
            {
                "groupIndex": group_index,
                "serialOffset": group_offset,
                "shaderType": shader_type,
                "descriptorCount": descriptor_count,
                "codeRecordCount": code_count,
                "descriptorTableSha256": raw_sha256(
                    serial[descriptor_start : descriptor_start + descriptor_size]
                ),
                "codeRowsSha256": canonical_json_sha256(group_records),
            }
        )

    require(
        all(a[1] <= b[0] for a, b in zip(compressed_intervals, compressed_intervals[1:])),
        "ShaderCache compressed code ranges overlap",
    )
    shader_code_end = offset
    require(offset + 8 <= len(serial), "ShaderCache native shader-object table header is truncated")
    shader_object_platform, shader_object_count = struct.unpack_from("<II", serial, offset)
    require(shader_object_platform == platform, "ShaderCache native tail platform mismatch")
    require(shader_object_count == len(records), "ShaderCache native shader-object count mismatch")
    tail = serial[offset:]
    return {
        "nativeHeader": native_header,
        "shaderPlatformOrdinal": platform,
        "shaderTypeGroupCount": group_count,
        "groups": groups,
        "codeRecords": records,
        "shaderCodeSectionEnd": shader_code_end,
        "shaderObjectTableHeader": {
            "offset": shader_code_end,
            "shaderPlatformOrdinal": shader_object_platform,
            "shaderObjectCount": shader_object_count,
        },
        "unparsedNativeTailByteCount": len(tail),
        "unparsedNativeTailSha256": raw_sha256(tail),
    }


def extract_material_topology(package: Any, entry: Any) -> dict[str, Any]:
    """Decode only the surviving cooked graph shape for one Material export."""
    class_name = package_ref_name(
        entry.class_index, package.imports, package.exports
    ).casefold()
    require(
        class_name in ("material", "decalmaterial"),
        "topology target is not a Material/DecalMaterial",
    )
    serial = package.logical[
        entry.serial_offset : entry.serial_offset + entry.serial_size
    ]
    properties, property_end = parse_tagged_properties(
        serial, package.names, package.summary.version
    )
    expression_references = property_value(properties, "expressions")
    require(
        isinstance(expression_references, list),
        "Material Expressions array is missing",
    )
    outputs: dict[str, dict[str, Any]] = {}
    for output_name in MATERIAL_OUTPUTS:
        decoded = expression_input(property_value(properties, output_name))
        if decoded is not None:
            outputs[output_name] = decoded
    non_null = 0
    unresolved_edges = 0
    for reference in expression_references:
        if not isinstance(reference, int) or reference == 0:
            continue
        require(
            1 <= reference <= len(package.exports),
            "Material expression reference is invalid",
        )
        expression_entry = package.exports[reference - 1]
        expression_class = package_ref_name(
            expression_entry.class_index, package.imports, package.exports
        ).casefold()
        require(
            expression_class.startswith("materialexpression"),
            "Material expression target class is invalid",
        )
        expression_serial = package.logical[
            expression_entry.serial_offset :
            expression_entry.serial_offset + expression_entry.serial_size
        ]
        expression_properties, _expression_end = parse_tagged_properties(
            expression_serial, package.names, package.summary.version
        )
        non_null += 1
        unresolved_edges += sum(
            int(int(row["packageIndex"]) == 0)
            for row in expression_inputs(expression_properties)
        )
    expression_count = len(expression_references)
    null_count = expression_count - non_null
    return {
        "topologyStatus": classify_topology(
            expression_count, non_null, outputs, unresolved_edges
        ),
        "expressionEntryCount": expression_count,
        "nonNullExpressionCount": non_null,
        "nullExpressionCount": null_count,
        "unresolvedInputEdgeCount": unresolved_edges,
        "outputBindingCount": len(outputs),
        "materialSerialSha256": raw_sha256(serial),
        "materialPropertyStreamEnd": property_end,
    }


def material_family_projection(contract: dict[str, Any]) -> list[dict[str, Any]]:
    require(contract.get("characterClass") == "ARTIST", "Material contract class mismatch")
    require(contract.get("skillId") == 31470, "Material contract skill mismatch")
    expected = {
        family_id: (source_path, class_name)
        for family_id, source_path, _global_path, class_name in MATERIAL_TARGETS
    }
    contract_rows = {
        str(row.get("familyId") or ""): row
        for row in contract.get("graphFamilies", [])
    }
    require(set(contract_rows) == set(expected), "Artist F 23-family Material projection changed")
    result: list[dict[str, Any]] = []
    for family_id in sorted(expected):
        source_path, class_name = expected[family_id]
        row = contract_rows[family_id]
        identity = row.get("exactIdentity") or {}
        cooked = row.get("cookedEvidence") or {}
        require(
            str(identity.get("materialObjectPath") or "") == source_path,
            "Artist F Material object identity changed",
        )
        require(
            row.get("graphProvenance") == "RECONSTRUCTED_GRAPH"
            and cooked.get("topologyStatus") == "COOKED_PARTIAL",
            "Artist F Material graph fidelity changed without review",
        )
        package_name = str(identity.get("physicalPackage") or "")
        package_sha = str(identity.get("physicalPackageSha256") or "")
        export_index = identity.get("materialExportIndex")
        require(package_name.casefold().endswith(".upk"), "Material package identity is missing")
        require(re.fullmatch(r"[0-9a-f]{64}", package_sha) is not None, "Material package SHA is invalid")
        require(type(export_index) is int and export_index >= 0, "Material export index is invalid")
        topology = {
            name: cooked.get(name)
            for name in (
                "topologyStatus",
                "expressionEntryCount",
                "nonNullExpressionCount",
                "nullExpressionCount",
                "unresolvedInputEdgeCount",
            )
        }
        require(
            all(type(topology[name]) is int for name in topology if name != "topologyStatus"),
            "Material topology denominator is invalid",
        )
        result.append(
            {
                "familyId": family_id,
                "materialObjectPath": source_path,
                "materialClass": class_name,
                "physicalPackage": package_name,
                "physicalPackageSha256": package_sha,
                "materialExportIndex": export_index,
                "sourceTopology": topology,
            }
        )
    return result


def material_recipe_projection(contract: dict[str, Any]) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for recipe in contract.get("materialRecipes", []):
        identity = recipe.get("identity") or {}
        rows.append(
            {
                "recipeId": str(recipe.get("recipeId") or ""),
                "sourceMaterialPath": str(recipe.get("sourceMaterialPath") or ""),
                "physicalPackage": str(identity.get("physicalPackage") or ""),
                "physicalPackageSha256": str(identity.get("physicalPackageSha256") or ""),
                "materialObjectPath": str(identity.get("materialObjectPath") or ""),
                "materialClass": str(identity.get("materialClass") or ""),
                "materialExportIndex": identity.get("materialExportIndex"),
                "arithmeticFamilyId": str(recipe.get("arithmeticFamilyId") or ""),
            }
        )
    rows.sort(key=lambda row: row["recipeId"])
    require(len(rows) == 27, "Artist F Material recipe denominator changed")
    require(
        canonical_json_sha256(rows) == MATERIAL_RECIPE_PROJECTION_SHA256,
        "Artist F Material recipe identity projection changed",
    )
    return rows


def extract_material_native_keys(package: Any) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    by_path = {
        package_ref_path(entry.index + 1, package.imports, package.exports).casefold(): entry
        for entry in package.exports
    }
    for family_id, source_path, global_path, expected_class in MATERIAL_TARGETS:
        entry = by_path.get(global_path.casefold())
        require(entry is not None, f"global Material export is missing: {global_path}")
        class_name = package_ref_name(entry.class_index, package.imports, package.exports).casefold()
        require(class_name == expected_class, f"global Material class mismatch: {global_path}")
        serial = package.logical[entry.serial_offset : entry.serial_offset + entry.serial_size]
        _properties, property_end = parse_tagged_properties(
            serial, package.names, package.summary.version
        )
        tail = serial[property_end:]
        require(len(tail) >= 84 and len(tail) % 4 == 0, "Material native tail shape is invalid")
        require(tail[:4] == b"\x01\x00\x00\x00", "Material native tail prefix mismatch")
        require(tail[4:12] == b"\x00" * 8, "Material native tail reserved bytes changed")
        key = tail[16:32]
        require(len(key) == 16 and key != b"\x00" * 16, "Material native key candidate is invalid")
        topology = extract_material_topology(package, entry)
        rows.append(
            {
                "familyId": family_id,
                "sourceMaterialObjectPath": source_path,
                "globalMaterialObjectPath": global_path,
                "className": class_name,
                "exportIndex": entry.index,
                "serialOffset": entry.serial_offset,
                "serialSize": entry.serial_size,
                "serialSha256": raw_sha256(serial),
                "propertyStreamEnd": property_end,
                "nativeTailByteCount": len(tail),
                "nativeTailSha256": raw_sha256(tail),
                "nativeStateKeyCandidateHex": key.hex(),
                "keyFidelity": "OBSERVED_NATIVE_TAIL_KEY_UNAUTHENTICATED_AS_SHADER_MAP_ID",
                "topology": topology,
                "topologyFidelity": "CURRENT_GLOBAL_PACKAGE_CROSS_REVISION_COOKED_PARTIAL",
            }
        )
    require(len(rows) == 23 and len({row["familyId"] for row in rows}) == 23, "Material family denominator changed")
    return rows


def extract_recipe_native_keys(
    source_package_root: Path,
    recipe_projection: list[dict[str, Any]],
    family_projection: list[dict[str, Any]],
    base_material_rows: list[dict[str, Any]],
) -> tuple[
    list[dict[str, Any]],
    list[dict[str, Any]],
    list[dict[str, Any]],
    dict[str, list[tuple[int, bytes]]],
]:
    require(source_package_root.is_dir(), "Artist F source package root is missing")
    base_keys = {
        row["familyId"]: bytes.fromhex(row["nativeStateKeyCandidateHex"])
        for row in base_material_rows
    }
    result: list[dict[str, Any]] = []
    package_identities: list[dict[str, Any]] = []
    topology_candidates: list[dict[str, Any]] = []
    mic_tail_windows: dict[str, list[tuple[int, bytes]]] = {}
    package_names = sorted({row["physicalPackage"] for row in recipe_projection})
    require(
        {row["physicalPackage"] for row in family_projection}.issubset(package_names),
        "Material family package is outside the recipe package projection",
    )
    for package_name in package_names:
        package_path = source_package_root / package_name
        package_rows = [row for row in recipe_projection if row["physicalPackage"] == package_name]
        family_rows = [row for row in family_projection if row["physicalPackage"] == package_name]
        expected_hashes = {row["physicalPackageSha256"] for row in package_rows}
        expected_hashes.update(row["physicalPackageSha256"] for row in family_rows)
        require(len(expected_hashes) == 1, f"recipe package hash is ambiguous: {package_name}")
        expected_hash = next(iter(expected_hashes))
        require(package_path.is_file(), f"recipe package is missing: {package_name}")
        physical_size = package_path.stat().st_size
        physical_hash = raw_file_sha256(package_path)
        require(physical_hash == expected_hash, f"recipe package SHA mismatch: {package_name}")
        package = load_package(package_path, LOSTARK_KR_AES_KEY)
        require(package.summary.version == 868, f"recipe package version mismatch: {package_name}")
        package_identities.append(
            {
                "fileName": package_name,
                "physicalByteSize": physical_size,
                "rawSha256": physical_hash,
                "packageVersion": package.summary.version,
                "licenseeVersion": package.summary.licensee_version,
                "logicalByteSize": len(package.logical),
                "exportCount": len(package.exports),
                "hashRole": "EXTERNAL_RAW_BYTES",
            }
        )
        material_exports: list[tuple[Any, str, str]] = []
        for candidate_entry in package.exports:
            candidate_class = package_ref_name(
                candidate_entry.class_index, package.imports, package.exports
            ).casefold()
            if candidate_class not in ("material", "decalmaterial"):
                continue
            candidate_path = package_ref_path(
                candidate_entry.index + 1, package.imports, package.exports
            )
            material_exports.append((candidate_entry, candidate_path, candidate_class))
        for family in family_rows:
            exact_index = family["materialExportIndex"]
            require(0 <= exact_index < len(package.exports), "family Material export index is invalid")
            exact_entry = package.exports[exact_index]
            exact_path = package_ref_path(
                exact_entry.index + 1, package.imports, package.exports
            )
            exact_class = package_ref_name(
                exact_entry.class_index, package.imports, package.exports
            ).casefold()
            require(
                exact_path.casefold() == family["materialObjectPath"].casefold()
                and exact_class == family["materialClass"],
                "family Material exact export identity mismatch",
            )
            exact_topology = extract_material_topology(package, exact_entry)
            for field, expected in family["sourceTopology"].items():
                require(
                    exact_topology[field] == expected,
                    f"family Material topology changed: {family['familyId']}.{field}",
                )
            topology_candidates.append(
                {
                    "familyId": family["familyId"],
                    "candidateRole": "SOURCE_EXACT_DEPENDENCY",
                    "physicalPackage": package_name,
                    "physicalPackageSha256": physical_hash,
                    "materialObjectPath": exact_path,
                    "materialClass": exact_class,
                    "materialExportIndex": exact_entry.index,
                    "topology": exact_topology,
                    "fidelity": "SOURCE_EXACT_COOKED_PARTIAL_GRAPH_SHAPE",
                }
            )
            leaf = exact_path.rsplit(".", 1)[-1].casefold()
            for alternate_entry, alternate_path, alternate_class in material_exports:
                if alternate_entry.index == exact_entry.index:
                    continue
                if alternate_path.rsplit(".", 1)[-1].casefold() != leaf:
                    continue
                topology_candidates.append(
                    {
                        "familyId": family["familyId"],
                        "candidateRole": "SAME_PACKAGE_SAME_LEAF_ALTERNATE_OBJECT",
                        "physicalPackage": package_name,
                        "physicalPackageSha256": physical_hash,
                        "materialObjectPath": alternate_path,
                        "materialClass": alternate_class,
                        "materialExportIndex": alternate_entry.index,
                        "topology": extract_material_topology(package, alternate_entry),
                        "fidelity": "OBSERVED_SAME_PACKAGE_ALTERNATE_OBJECT_UNBOUND",
                    }
                )
        for recipe in package_rows:
            export_index = recipe["materialExportIndex"]
            require(type(export_index) is int and 0 <= export_index < len(package.exports), "recipe export index is invalid")
            entry = package.exports[export_index]
            object_path = package_ref_path(
                entry.index + 1, package.imports, package.exports
            )
            class_name = package_ref_name(
                entry.class_index, package.imports, package.exports
            ).casefold()
            require(object_path.casefold() == recipe["materialObjectPath"].casefold(), "recipe object path/export index mismatch")
            require(class_name == recipe["materialClass"].casefold(), "recipe Material class mismatch")
            serial = package.logical[
                entry.serial_offset : entry.serial_offset + entry.serial_size
            ]
            properties, property_end = parse_tagged_properties(
                serial, package.names, package.summary.version
            )
            tail = serial[property_end:]
            is_mic = class_name == "materialinstanceconstant"
            static_permutation = bool(
                tagged_value(properties, "bhasstaticpermutationresource")
            )
            mic_key = b""
            base_key_offset: int | None = None
            if is_mic and tail:
                require(len(tail) >= 32 and len(tail) % 4 == 0, "MIC native tail shape is invalid")
                require(tail[:4] == b"\x03\x00\x00\x00", "MIC native tail prefix mismatch")
                mic_key = tail[16:32]
                require(mic_key != b"\x00" * 16, "MIC native key candidate is zero")
                base_key = base_keys[recipe["arithmeticFamilyId"]]
                base_key_offset = tail.find(base_key)
                require(base_key_offset >= 0, "MIC tail does not retain its base Material native key")
                require(static_permutation, "MIC native tail exists without static permutation resource")
            if is_mic and not tail:
                require(not static_permutation, "static permutation MIC has no native tail")
            aligned_nonzero_windows = [
                (offset, tail[offset : offset + 16])
                for offset in range(0, max(0, len(tail) - 15), 4)
                if tail[offset : offset + 16] != b"\x00" * 16
            ]
            mic_tail_windows[recipe["recipeId"]] = aligned_nonzero_windows
            result.append(
                {
                    "recipeId": recipe["recipeId"],
                    "arithmeticFamilyId": recipe["arithmeticFamilyId"],
                    "sourceMaterialPath": recipe["sourceMaterialPath"],
                    "physicalPackage": package_name,
                    "physicalPackageSha256": physical_hash,
                    "materialObjectPath": object_path,
                    "className": class_name,
                    "exportIndex": entry.index,
                    "serialOffset": entry.serial_offset,
                    "serialSize": entry.serial_size,
                    "serialSha256": raw_sha256(serial),
                    "propertyStreamEnd": property_end,
                    "nativeTailByteCount": len(tail),
                    "nativeTailSha256": raw_sha256(tail),
                    "hasStaticPermutationResource": static_permutation,
                    "micNativeStateKeyCandidateHex": mic_key.hex() if mic_key else None,
                    "baseMaterialNativeKeyOffsetInTail": base_key_offset,
                    "alignedNonzero16ByteWindowCount": len(aligned_nonzero_windows),
                    "alignedNonzero16ByteWindowSha256": canonical_json_sha256(
                        [
                            {"offset": offset, "candidateHex": candidate.hex()}
                            for offset, candidate in aligned_nonzero_windows
                        ]
                    ),
                    "keyFidelity": (
                        "OBSERVED_MIC_NATIVE_TAIL_KEY_UNAUTHENTICATED_AS_STATIC_PARAMETER_SET_ID"
                        if mic_key
                        else "NO_MIC_NATIVE_KEY"
                    ),
                }
            )
        del package
        gc.collect()
    result.sort(key=lambda row: row["recipeId"])
    package_identities.sort(key=lambda row: row["fileName"].casefold())
    require(len(result) == 27, "recipe native-tail denominator changed")
    require(sum(row["className"] == "materialinstanceconstant" for row in result) == 25, "MIC denominator changed")
    require(sum(bool(row["micNativeStateKeyCandidateHex"]) for row in result) == 24, "MIC native key denominator changed")
    require(
        sum(row["candidateRole"] == "SOURCE_EXACT_DEPENDENCY" for row in topology_candidates) == 23,
        "source Material topology denominator changed",
    )
    topology_candidates.sort(
        key=lambda row: (
            row["familyId"],
            row["candidateRole"],
            row["physicalPackage"].casefold(),
            row["materialObjectPath"].casefold(),
        )
    )
    require(len(mic_tail_windows) == 27, "MIC tail window recipe denominator changed")
    return result, package_identities, topology_candidates, mic_tail_windows


def scan_material_inventory_reports(
    inventory_root: Path,
    family_projection: list[dict[str, Any]],
) -> dict[str, Any]:
    """Use reconstructed report inventories only to bound duplicate candidates."""
    require(inventory_root.is_dir(), "Material inventory report root is missing")
    reports = sorted(inventory_root.glob("*.materials.json"))
    require(len(reports) == 30, "Material inventory report denominator changed")
    target_by_leaf = {
        row["materialObjectPath"].rsplit(".", 1)[-1].casefold(): row
        for row in family_projection
    }
    report_identities: list[dict[str, Any]] = []
    matches: list[dict[str, Any]] = []
    for path in reports:
        raw = path.read_bytes()
        try:
            document = json.loads(raw.decode("utf-8-sig"))
        except (UnicodeDecodeError, json.JSONDecodeError) as error:
            raise ValueError(f"Material inventory report is invalid: {path.name}") from error
        require(
            type(document.get("schema_version")) is int
            and document.get("schema_version") == 2,
            "Material inventory report version changed",
        )
        source = document.get("source") or {}
        source_file = str(source.get("file") or "")
        source_sha = str(source.get("sha256") or "").casefold()
        require(source_file.casefold().endswith(".upk"), "Material inventory source file is invalid")
        require(re.fullmatch(r"[0-9a-f]{64}", source_sha) is not None, "Material inventory source SHA is invalid")
        report_identities.append(
            {
                "fileName": path.name,
                "byteSize": len(raw),
                "rawSha256": raw_sha256(raw),
                "sourcePackage": source_file,
                "claimedSourcePackageSha256": source_sha,
                "hashRole": "EXTERNAL_RECONSTRUCTED_REPORT_RAW_BYTES",
            }
        )
        materials = document.get("materials")
        require(isinstance(materials, list), "Material inventory rows are missing")
        for material in materials:
            if not isinstance(material, dict):
                continue
            class_name = str(material.get("class") or "").casefold()
            if class_name not in ("material", "decalmaterial"):
                continue
            object_path = str(material.get("material_path") or "")
            object_name = str(
                material.get("object_name")
                or object_path.rsplit(".", 1)[-1]
            )
            family = target_by_leaf.get(object_name.casefold())
            if family is None:
                continue
            export_index = material.get("export_index")
            require(type(export_index) is int and export_index >= 0, "Material inventory export index is invalid")
            matches.append(
                {
                    "familyId": family["familyId"],
                    "reportFileName": path.name,
                    "sourcePackage": source_file,
                    "claimedSourcePackageSha256": source_sha,
                    "materialObjectPath": object_path,
                    "materialClass": class_name,
                    "materialExportIndex": export_index,
                    "fidelity": "RECONSTRUCTED_INVENTORY_CANDIDATE_NOT_PACKAGE_PROOF",
                }
            )
    report_identities.sort(key=lambda row: row["fileName"].casefold())
    matches.sort(
        key=lambda row: (
            row["familyId"], row["sourcePackage"].casefold(),
            row["materialObjectPath"].casefold(),
        )
    )
    family_by_id = {row["familyId"]: row for row in family_projection}
    unexpected = [
        row
        for row in matches
        if row["sourcePackage"].casefold()
        != family_by_id[row["familyId"]]["physicalPackage"].casefold()
        or row["claimedSourcePackageSha256"]
        != family_by_id[row["familyId"]]["physicalPackageSha256"]
    ]
    require(not unexpected, "inventory found an unverified alternate physical package candidate")
    covered = {row["familyId"] for row in matches}
    alternate_rows = [
        row
        for row in matches
        if row["materialObjectPath"].casefold()
        != family_by_id[row["familyId"]]["materialObjectPath"].casefold()
    ]
    require(len(matches) == 22 and len(covered) == 21, "Material inventory target coverage changed")
    require(len(alternate_rows) == 1, "Material inventory alternate-object denominator changed")
    return {
        "scope": "30_RECONSTRUCTED_DIMENSIONMASTER_MATERIAL_REPORTS_NOT_FULL_INSTALLATION",
        "reportCount": len(report_identities),
        "reportIdentities": report_identities,
        "targetFamilyCoverageCount": len(covered),
        "targetMaterialRowCount": len(matches),
        "alternateObjectCandidateCount": len(alternate_rows),
        "unexpectedPhysicalPackageCandidateCount": len(unexpected),
        "rows": matches,
        "fidelity": "OBSERVED_RECONSTRUCTED_REPORT_SCOPE_ONLY",
    }


def build_topology_completeness_matrix(
    family_projection: list[dict[str, Any]],
    global_material_rows: list[dict[str, Any]],
    source_candidates: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    global_by_family = {row["familyId"]: row for row in global_material_rows}
    candidates_by_family: dict[str, list[dict[str, Any]]] = {}
    for row in source_candidates:
        candidates_by_family.setdefault(row["familyId"], []).append(dict(row))
    result: list[dict[str, Any]] = []
    for family in family_projection:
        family_id = family["familyId"]
        global_row = global_by_family[family_id]
        candidates = candidates_by_family.get(family_id, [])
        candidates.append(
            {
                "familyId": family_id,
                "candidateRole": "CURRENT_GLOBAL_PACKAGE_CROSS_REVISION",
                "physicalPackage": EXPECTED_EXTERNAL_IDENTITIES["globalMaterialPackage"]["fileName"],
                "physicalPackageSha256": EXPECTED_EXTERNAL_IDENTITIES["globalMaterialPackage"]["sha256"],
                "materialObjectPath": global_row["globalMaterialObjectPath"],
                "materialClass": global_row["className"],
                "materialExportIndex": global_row["exportIndex"],
                "topology": global_row["topology"],
                "fidelity": "CURRENT_GLOBAL_PACKAGE_CROSS_REVISION_COOKED_PARTIAL",
            }
        )
        source = next(
            row for row in candidates
            if row["candidateRole"] == "SOURCE_EXACT_DEPENDENCY"
        )
        source_topology = source["topology"]
        for candidate in candidates:
            topology = candidate["topology"]
            same_denominator = (
                topology["expressionEntryCount"]
                == source_topology["expressionEntryCount"]
            )
            no_worse = (
                topology["nullExpressionCount"]
                <= source_topology["nullExpressionCount"]
                and topology["unresolvedInputEdgeCount"]
                <= source_topology["unresolvedInputEdgeCount"]
            )
            strictly_better = (
                topology["nullExpressionCount"]
                < source_topology["nullExpressionCount"]
                or topology["unresolvedInputEdgeCount"]
                < source_topology["unresolvedInputEdgeCount"]
            )
            candidate["sameExpressionEntryDenominatorAsSource"] = same_denominator
            candidate["strictParetoImprovementOverSource"] = (
                candidate["candidateRole"] != "SOURCE_EXACT_DEPENDENCY"
                and same_denominator and no_worse and strictly_better
            )
        candidates.sort(
            key=lambda row: (
                row["candidateRole"], row["physicalPackage"].casefold(),
                row["materialObjectPath"].casefold(),
            )
        )
        improving = [
            row for row in candidates
            if row["strictParetoImprovementOverSource"]
        ]
        result.append(
            {
                "familyId": family_id,
                "sourceExpressionEntryCount": source_topology["expressionEntryCount"],
                "minimumObservedNullExpressionCount": min(
                    row["topology"]["nullExpressionCount"] for row in candidates
                ),
                "minimumObservedUnresolvedInputEdgeCount": min(
                    row["topology"]["unresolvedInputEdgeCount"] for row in candidates
                ),
                "strictParetoImprovementCandidateCount": len(improving),
                "reconstructedEvaluatorOracleCandidate": (
                    {
                        "physicalPackage": improving[0]["physicalPackage"],
                        "physicalPackageSha256": improving[0]["physicalPackageSha256"],
                        "materialObjectPath": improving[0]["materialObjectPath"],
                        "materialExportIndex": improving[0]["materialExportIndex"],
                        "fidelity": "CROSS_REVISION_RECONSTRUCTED_EVALUATOR_ORACLE_CANDIDATE_NOT_SOURCE_EXACT",
                    }
                    if len(improving) == 1
                    else None
                ),
                "candidates": candidates,
            }
        )
    require(len(result) == 23, "Material topology matrix denominator changed")
    return result


def key_variants(raw: bytes) -> dict[str, bytes]:
    require(len(raw) == 16, "Material native key must be 16 bytes")
    return {
        "DIRECT": raw,
        "REVERSE_ALL": raw[::-1],
        "REVERSE_U32_WORDS": b"".join(raw[index : index + 4][::-1] for index in range(0, 16, 4)),
        "REVERSE_GUID_TEXT_FIELDS": raw[:4][::-1] + raw[4:6][::-1] + raw[6:8][::-1] + raw[8:],
        "REVERSE_U32_WORD_ORDER": raw[12:16] + raw[8:12] + raw[4:8] + raw[:4],
    }


def mic_key_variants(raw: bytes) -> dict[str, bytes]:
    variants = key_variants(raw)
    variants.update(
        {
            "MD5_NATIVE_KEY": hashlib.md5(raw).digest(),
            "SHA1_NATIVE_KEY": hashlib.sha1(raw).digest(),
            "SHA256_NATIVE_KEY": hashlib.sha256(raw).digest(),
        }
    )
    return variants


def locate_export_for_offset(package: Any, logical_offset: int) -> tuple[Any | None, int | None]:
    rows = sorted((entry.serial_offset, entry.serial_offset + entry.serial_size, entry) for entry in package.exports)
    starts = [row[0] for row in rows]
    index = bisect.bisect_right(starts, logical_offset) - 1
    if index >= 0 and logical_offset + 16 <= rows[index][1]:
        return rows[index][2], logical_offset - rows[index][0]
    return None, None


def scan_material_keys(package: Any, material_rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    for row in material_rows:
        variants = key_variants(bytes.fromhex(row["nativeStateKeyCandidateHex"]))
        matches: list[dict[str, Any]] = []
        for variant_name, needle in variants.items():
            start = 0
            while True:
                found = package.logical.find(needle, start)
                if found < 0:
                    break
                owner, relative = locate_export_for_offset(package, found)
                matches.append(
                    {
                        "variant": variant_name,
                        "logicalOffset": found,
                        "shaderCacheExportIndex": owner.index if owner is not None else None,
                        "shaderCacheObjectPath": package_ref_path(
                            owner.index + 1, package.imports, package.exports
                        ) if owner is not None else None,
                        "serialRelativeOffset": relative,
                    }
                )
                start = found + 1
        results.append(
            {
                "familyId": row["familyId"],
                "nativeStateKeyCandidateHex": row["nativeStateKeyCandidateHex"],
                "variantCount": len(variants),
                "matches": matches,
                "matchCount": len(matches),
            }
        )
    return results


def scan_mic_keys(package: Any, recipe_rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    for row in recipe_rows:
        encoded = row.get("micNativeStateKeyCandidateHex")
        if not encoded:
            results.append(
                {
                    "recipeId": row["recipeId"],
                    "micNativeStateKeyCandidateHex": None,
                    "variantCount": 0,
                    "matches": [],
                    "matchCount": 0,
                }
            )
            continue
        variants = mic_key_variants(bytes.fromhex(encoded))
        matches: list[dict[str, Any]] = []
        for variant_name, needle in variants.items():
            start = 0
            while True:
                found = package.logical.find(needle, start)
                if found < 0:
                    break
                owner, relative = locate_export_for_offset(package, found)
                matches.append(
                    {
                        "variant": variant_name,
                        "logicalOffset": found,
                        "shaderCacheExportIndex": owner.index if owner is not None else None,
                        "shaderCacheObjectPath": package_ref_path(
                            owner.index + 1, package.imports, package.exports
                        ) if owner is not None else None,
                        "serialRelativeOffset": relative,
                    }
                )
                start = found + 1
        results.append(
            {
                "recipeId": row["recipeId"],
                "micNativeStateKeyCandidateHex": encoded,
                "variantCount": len(variants),
                "matches": matches,
                "matchCount": len(matches),
            }
        )
    return results


def intersect_mic_tail_shader_ids(
    recipe_rows: list[dict[str, Any]],
    mic_tail_windows: dict[str, list[tuple[int, bytes]]],
    shader_records: list[dict[str, Any]],
) -> dict[str, Any]:
    shader_ids = {
        bytes.fromhex(str(row.get("shaderIdCandidateHex") or ""))
        for row in shader_records
    }
    require(
        len(shader_ids) == len(shader_records)
        and all(len(candidate) == 16 for candidate in shader_ids),
        "Shader descriptor ID candidate set is invalid",
    )
    rows: list[dict[str, Any]] = []
    all_matches: list[dict[str, Any]] = []
    for recipe in recipe_rows:
        recipe_id = recipe["recipeId"]
        windows = mic_tail_windows.get(recipe_id)
        require(windows is not None, "MIC tail window candidates are missing")
        candidate_projection = [
            {"offset": offset, "candidateHex": candidate.hex()}
            for offset, candidate in windows
        ]
        require(
            len(windows) == recipe["alignedNonzero16ByteWindowCount"]
            and canonical_json_sha256(candidate_projection)
            == recipe["alignedNonzero16ByteWindowSha256"],
            "MIC tail window candidate digest mismatch",
        )
        matches = [
            {"offsetInNativeTail": offset, "shaderIdCandidateHex": candidate.hex()}
            for offset, candidate in windows
            if candidate in shader_ids
        ]
        for match in matches:
            all_matches.append({"recipeId": recipe_id, **match})
        rows.append(
            {
                "recipeId": recipe_id,
                "nativeTailByteCount": recipe["nativeTailByteCount"],
                "alignedNonzero16ByteWindowCount": len(windows),
                "candidateDigestSha256": recipe["alignedNonzero16ByteWindowSha256"],
                "directShaderIdMatchCount": len(matches),
                "matches": matches,
            }
        )
    return {
        "recipeCount": len(rows),
        "staticPermutationMicCount": sum(
            bool(row.get("micNativeStateKeyCandidateHex")) for row in recipe_rows
        ),
        "descriptorShaderIdCandidateCount": len(shader_records),
        "uniqueDescriptorShaderIdCandidateCount": len(shader_ids),
        "alignedNonzero16ByteWindowCount": sum(
            row["alignedNonzero16ByteWindowCount"] for row in rows
        ),
        "directShaderIdMatchCount": len(all_matches),
        "status": (
            "MIC_TAIL_CONTAINS_NO_DIRECT_SHADER_OBJECT_ID"
            if not all_matches
            else "MIC_TAIL_SHADER_OBJECT_ID_WINDOW_MATCH_UNBOUND"
        ),
        "rows": rows,
        "matches": all_matches,
        "fidelity": "BOUNDED_ALIGNED_WINDOW_INTERSECTION_NOT_STRUCTURAL_FIELD_DECODING",
    }


def verify_loaded_package(package: Any, expected: dict[str, Any]) -> dict[str, Any]:
    require(package.path.name.casefold() == expected["fileName"].casefold(), "loaded package name mismatch")
    require(package.path.stat().st_size == expected["byteSize"], "loaded package physical size mismatch")
    require(package.sha256 == expected["sha256"], "loaded package SHA mismatch")
    require(package.summary.version == expected["packageVersion"], "loaded package version mismatch")
    require(len(package.logical) == expected["logicalByteSize"], "loaded package logical size mismatch")
    require(len(package.exports) == expected["exportCount"], "loaded package export count mismatch")
    return {
        "fileName": package.path.name,
        "physicalByteSize": package.path.stat().st_size,
        "rawSha256": package.sha256,
        "packageVersion": package.summary.version,
        "licenseeVersion": package.summary.licensee_version,
        "logicalByteSize": len(package.logical),
        "exportCount": len(package.exports),
        "importCount": len(package.imports),
        "hashRole": "EXTERNAL_RAW_BYTES",
    }


def aggregate_primary_records(records: list[dict[str, Any]]) -> dict[str, Any]:
    profiles = Counter(
        row["disassembly"]["profile"]
        for row in records
        if row.get("disassembly") is not None
    )
    declarations = [
        {
            "dxbcSha256": row["dxbcSha256"],
            "profile": row["disassembly"]["profile"],
            "constantBuffers": row["disassembly"]["constantBufferDeclarations"],
            "samplers": row["disassembly"]["samplerDeclarations"],
            "resources": row["disassembly"]["resourceDeclarations"],
            "uavs": row["disassembly"]["uavDeclarations"],
        }
        for row in records
    ]
    return {
        "profileCounts": dict(sorted(profiles.items())),
        "constantBufferDeclarationCount": sum(len(row["constantBuffers"]) for row in declarations),
        "samplerDeclarationCount": sum(len(row["samplers"]) for row in declarations),
        "resourceDeclarationCount": sum(len(row["resources"]) for row in declarations),
        "uavDeclarationCount": sum(len(row["uavs"]) for row in declarations),
        "uniqueBindingSignatureCount": len(
            {
                canonical_json_sha256(
                    {
                        "profile": row["profile"],
                        "constantBuffers": row["constantBuffers"],
                        "samplers": row["samplers"],
                        "resources": row["resources"],
                        "uavs": row["uavs"],
                    }
                )
                for row in declarations
            }
        ),
        "bindingDeclarationSha256": canonical_json_sha256(declarations),
    }


def build_receipt(
    material_contract_path: Path,
    source_package_root: Path,
    material_inventory_root: Path,
    global_material_package_path: Path,
    shader_cache_package_path: Path,
    d3dcompiler_path: Path,
) -> dict[str, Any]:
    material_contract = read_json(material_contract_path)
    projection = material_family_projection(material_contract)
    recipe_projection = material_recipe_projection(material_contract)
    inventory_evidence = scan_material_inventory_reports(
        material_inventory_root, projection
    )
    verify_external_file(
        global_material_package_path,
        EXPECTED_EXTERNAL_IDENTITIES["globalMaterialPackage"],
    )
    global_package = load_package(global_material_package_path, LOSTARK_KR_AES_KEY)
    global_identity = verify_loaded_package(
        global_package, EXPECTED_EXTERNAL_IDENTITIES["globalMaterialPackage"]
    )
    material_rows = extract_material_native_keys(global_package)
    del global_package
    gc.collect()

    (
        recipe_native_rows,
        recipe_package_identities,
        source_topology_candidates,
        mic_tail_windows,
    ) = extract_recipe_native_keys(
        source_package_root, recipe_projection, projection, material_rows
    )
    topology_matrix = build_topology_completeness_matrix(
        projection, material_rows, source_topology_candidates
    )
    strict_topology_improvement_count = sum(
        row["strictParetoImprovementCandidateCount"]
        for row in topology_matrix
    )

    verify_external_file(
        shader_cache_package_path,
        EXPECTED_EXTERNAL_IDENTITIES["shaderCachePackage"],
    )
    shader_package = load_package(shader_cache_package_path, LOSTARK_KR_AES_KEY)
    shader_identity = verify_loaded_package(
        shader_package, EXPECTED_EXTERNAL_IDENTITIES["shaderCachePackage"]
    )
    shader_cache_exports = [
        entry
        for entry in shader_package.exports
        if package_ref_name(entry.class_index, shader_package.imports, shader_package.exports).casefold()
        == "shadercache"
    ]
    require(len(shader_cache_exports) == 1_596, "ShaderCache export denominator changed")
    by_path = {
        package_ref_path(entry.index + 1, shader_package.imports, shader_package.exports).casefold(): entry
        for entry in shader_cache_exports
    }
    disassembler = D3DDisassembler(d3dcompiler_path)
    candidates: list[dict[str, Any]] = []
    primary_parsed: dict[str, Any] | None = None
    primary_serial: bytes | None = None
    for object_path in CACHE_CANDIDATES:
        entry = by_path.get(object_path.casefold())
        require(entry is not None, f"ShaderCache candidate is missing: {object_path}")
        serial = shader_package.logical[
            entry.serial_offset : entry.serial_offset + entry.serial_size
        ]
        parsed = parse_shader_cache_serial(
            serial,
            shader_package.names,
            disassembler if object_path == PRIMARY_CACHE else None,
        )
        candidate = {
            "objectPath": object_path,
            "exportIndex": entry.index,
            "serialOffset": entry.serial_offset,
            "serialSize": entry.serial_size,
            "serialSha256": raw_sha256(serial),
            "shaderTypeGroupCount": parsed["shaderTypeGroupCount"],
            "compressedCodeRecordCount": len(parsed["codeRecords"]),
            "uniqueDxbcCount": len({row["dxbcSha256"] for row in parsed["codeRecords"]}),
            "shaderCodeSectionEnd": parsed["shaderCodeSectionEnd"],
            "shaderObjectCount": parsed["shaderObjectTableHeader"]["shaderObjectCount"],
            "unparsedNativeTailByteCount": parsed["unparsedNativeTailByteCount"],
            "unparsedNativeTailSha256": parsed["unparsedNativeTailSha256"],
        }
        candidates.append(candidate)
        if object_path == PRIMARY_CACHE:
            primary_parsed = parsed
            primary_serial = serial
            for key, expected in PRIMARY_EXPECTED.items():
                actual = candidate.get(key)
                require(actual == expected, f"primary ShaderCache {key} changed")
    require(primary_parsed is not None and primary_serial is not None, "primary ShaderCache was not parsed")
    primary_records = primary_parsed["codeRecords"]
    require(all(row.get("disassembly") for row in primary_records), "primary DXBC disassembly is incomplete")
    first_record = primary_records[0]
    compressed = primary_serial[
        first_record["compressedOffset"] :
        first_record["compressedOffset"] + first_record["compressedByteSize"]
    ]
    raw_marker_offset = compressed.find(b"DXBC")
    require(raw_marker_offset >= 0, "primary compressed record lacks the expected raw DXBC marker")
    raw_marker_direct_disassembly_rejected = False
    try:
        disassembler(compressed[raw_marker_offset:])
    except ValueError:
        raw_marker_direct_disassembly_rejected = True
    require(
        raw_marker_direct_disassembly_rejected,
        "compressed raw DXBC marker unexpectedly disassembled without LZ4 decode",
    )
    key_scan = scan_material_keys(shader_package, material_rows)
    mic_key_scan = scan_mic_keys(shader_package, recipe_native_rows)
    mic_tail_shader_id_probe = intersect_mic_tail_shader_ids(
        recipe_native_rows, mic_tail_windows, primary_records
    )
    direct_matches = sum(
        1
        for row in key_scan
        for match in row["matches"]
        if match["variant"] == "DIRECT"
    )
    transformed_matches = sum(
        1
        for row in key_scan
        for match in row["matches"]
        if match["variant"] != "DIRECT"
    )
    require(direct_matches == 0 and transformed_matches == 0, "unreviewed Material key match requires native map decoding")
    mic_matches = sum(row["matchCount"] for row in mic_key_scan)
    require(mic_matches == 0, "unreviewed MIC key/hash match requires native map decoding")
    require(
        mic_tail_shader_id_probe["directShaderIdMatchCount"] == 0,
        "MIC tail contains an unreviewed direct ShaderCache descriptor ID candidate",
    )

    dependencies = []
    for relative in (
        "Tools/LevelPlacementExtractor/extract_artist_31470_shader_cache_oracle.py",
        "Tools/LevelPlacementExtractor/extract_ue3_placements.py",
        "Tools/LevelPlacementExtractor/extract_ue3_effect_material_closure.py",
        "Tools/LevelPlacementExtractor/extract_ue3_material_graph.py",
    ):
        path = REPO_ROOT / relative
        dependencies.append(
            {
                "path": relative,
                "sha256": canonical_text_sha256(path),
                "hashRole": "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
            }
        )

    primary_public_records = []
    for row in primary_records:
        public = dict(row)
        primary_public_records.append(public)
    receipt: dict[str, Any] = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "sourceFamilyProjection": {
            "familyCount": len(projection),
            "projectionSha256": canonical_json_sha256(projection),
            "rows": projection,
        },
        "sourceRecipeProjection": {
            "recipeCount": len(recipe_projection),
            "projectionSha256": canonical_json_sha256(recipe_projection),
            "rows": recipe_projection,
        },
        "externalEvidence": {
            "globalMaterialPackage": global_identity,
            "shaderCachePackage": shader_identity,
            "d3dcompiler": disassembler.identity,
            "recipePackages": recipe_package_identities,
            "boundedMaterialInventory": inventory_evidence,
        },
        "toolDependencies": dependencies,
        "materialNativeKeys": material_rows,
        "recipeNativeKeys": recipe_native_rows,
        "materialTopologyCompleteness": topology_matrix,
        "shaderCacheCandidates": candidates,
        "primaryShaderCache": {
            "objectPath": PRIMARY_CACHE,
            "nativeHeader": primary_parsed["nativeHeader"],
            "shaderPlatformOrdinal": primary_parsed["shaderPlatformOrdinal"],
            "shaderTypeGroupCount": primary_parsed["shaderTypeGroupCount"],
            "groups": primary_parsed["groups"],
            "codeRecords": primary_public_records,
            "shaderCodeSectionEnd": primary_parsed["shaderCodeSectionEnd"],
            "shaderObjectTableHeader": primary_parsed["shaderObjectTableHeader"],
            "unparsedNativeTailByteCount": primary_parsed["unparsedNativeTailByteCount"],
            "unparsedNativeTailSha256": primary_parsed["unparsedNativeTailSha256"],
            "bindingSummary": aggregate_primary_records(primary_records),
            "rawCompressedMarkerProbe": {
                "recordIndex": 0,
                "markerOffsetInCompressedBlock": raw_marker_offset,
                "directD3dDisassemblyRejected": raw_marker_direct_disassembly_rejected,
                "decodedD3dDisassemblyAccepted": True,
            },
        },
        "materialKeySearch": key_scan,
        "micKeySearch": mic_key_scan,
        "micTailShaderObjectIdProbe": mic_tail_shader_id_probe,
        "joinDecision": {
            "materialFamilyCount": 23,
            "directNativeKeyMatchCount": direct_matches,
            "commonEndianOrOrderMatchCount": transformed_matches,
            "micRecipeCount": 25,
            "staticPermutationMicCount": 24,
            "micNativeKeyCandidateCount": 24,
            "micDirectEndianOrHashMatchCount": mic_matches,
            "micTailAlignedNonzeroWindowCount": mic_tail_shader_id_probe["alignedNonzero16ByteWindowCount"],
            "micTailDirectShaderObjectIdMatchCount": mic_tail_shader_id_probe["directShaderIdMatchCount"],
            "micTailDirectShaderObjectIdStatus": mic_tail_shader_id_probe["status"],
            "strictCrossRevisionTopologyImprovementCandidateCount": strict_topology_improvement_count,
            "exactMaterialShaderMapJoinCount": 0,
            "exactMaterialShaderMapJoinRequired": 23,
            "fidelity": "UNRESOLVED",
            "blockers": list(BLOCKERS),
            "minimumRequiredEvidence": [
                "UE868_LICENSEE16_SHADER_OBJECT_TABLE_DECODER",
                "UE868_LICENSEE16_MATERIAL_SHADER_MAP_AND_STATIC_PARAMETER_SET_DECODER",
                "SOURCE_REVISION_BASE_MATERIAL_STATE_TO_SHADER_MAP_KEY_DERIVATION",
                "23_OF_23_MATERIAL_TO_SHADER_PERMUTATION_IDENTITY_JOIN",
                "PERMUTATION_CONSTANT_TEXTURE_SAMPLER_SEMANTIC_LAYOUT",
                "FIXED_INPUT_AND_OUTPUT_NUMERIC_SAMPLE_ORACLE",
                "FULL_INSTALLED_PACKAGE_EXPORT_NAME_INVENTORY_WITH_RAW_PACKAGE_IDENTITIES",
            ],
        },
        "admission": {
            "arithmeticEvaluatorImplementedCount": 0,
            "executionAdmission": False,
            "productAdmission": False,
        },
        "summary": {
            "installedShaderCacheExportCount": 1_596,
            "candidateShaderCacheCount": len(candidates),
            "materialFamilyCount": len(material_rows),
            "materialRecipeCount": len(recipe_native_rows),
            "materialInstanceRecipeCount": 25,
            "staticPermutationMicCount": 24,
            "micNativeKeyCandidateCount": 24,
            "micKeyVariantMatchCount": mic_matches,
            "micTailAlignedNonzeroWindowCount": mic_tail_shader_id_probe["alignedNonzero16ByteWindowCount"],
            "micTailDirectShaderObjectIdMatchCount": mic_tail_shader_id_probe["directShaderIdMatchCount"],
            "boundedInventoryReportCount": inventory_evidence["reportCount"],
            "boundedInventoryFamilyCoverageCount": inventory_evidence["targetFamilyCoverageCount"],
            "boundedInventoryAlternateObjectCandidateCount": inventory_evidence["alternateObjectCandidateCount"],
            "strictCrossRevisionTopologyImprovementCandidateCount": strict_topology_improvement_count,
            "primaryCompressedCodeRecordCount": len(primary_records),
            "primaryLz4DecodedCodeRecordCount": len(primary_records),
            "primaryDxbcTotalSizeValidatedCount": len(primary_records),
            "primaryD3dDisassemblyValidatedCount": len(primary_records),
            "primaryUniqueDxbcCount": len({row["dxbcSha256"] for row in primary_records}),
            "exactMaterialShaderMapJoinCount": 0,
            "arithmeticEvaluatorImplementedCount": 0,
        },
    }
    seal_receipt(receipt)
    return receipt


def validate_receipt(
    receipt: dict[str, Any], material_contract_path: Path | None = None
) -> None:
    require(receipt.get("schema") == SCHEMA, "ShaderCache receipt schema mismatch")
    require(type(receipt.get("formatVersion")) is int and receipt["formatVersion"] == FORMAT_VERSION, "ShaderCache receipt version mismatch")
    require(receipt.get("characterClass") == "ARTIST", "ShaderCache receipt class mismatch")
    require(receipt.get("skillId") == 31470 and receipt.get("inputSlot") == "F", "ShaderCache receipt skill identity mismatch")
    sealed = dict(receipt)
    claimed = sealed.pop("receiptSha256", None)
    require(claimed == canonical_json_sha256(sealed), "ShaderCache receipt digest mismatch")
    if material_contract_path is not None:
        material_contract = read_json(material_contract_path)
        projection = material_family_projection(material_contract)
        require(receipt["sourceFamilyProjection"]["projectionSha256"] == canonical_json_sha256(projection), "Material family projection digest mismatch")
        recipes = material_recipe_projection(material_contract)
        require(receipt["sourceRecipeProjection"]["projectionSha256"] == canonical_json_sha256(recipes), "Material recipe projection digest mismatch")
    for dependency in receipt.get("toolDependencies", []):
        require(dependency.get("hashRole") == "TRACKED_SOURCE_EOL_CANONICAL_TEXT", "tool dependency hash role mismatch")
        path = REPO_ROOT / str(dependency.get("path") or "")
        require(path.is_file(), "tool dependency is missing")
        require(dependency.get("sha256") == canonical_text_sha256(path), "tool dependency hash mismatch")
    summary = receipt.get("summary") or {}
    expected_summary = {
        "installedShaderCacheExportCount": 1_596,
        "candidateShaderCacheCount": 11,
        "materialFamilyCount": 23,
        "materialRecipeCount": 27,
        "materialInstanceRecipeCount": 25,
        "staticPermutationMicCount": 24,
        "micNativeKeyCandidateCount": 24,
        "micKeyVariantMatchCount": 0,
        "micTailAlignedNonzeroWindowCount": 4_816,
        "micTailDirectShaderObjectIdMatchCount": 0,
        "boundedInventoryReportCount": 30,
        "boundedInventoryFamilyCoverageCount": 21,
        "boundedInventoryAlternateObjectCandidateCount": 1,
        "strictCrossRevisionTopologyImprovementCandidateCount": 0,
        "primaryCompressedCodeRecordCount": 271,
        "primaryLz4DecodedCodeRecordCount": 271,
        "primaryDxbcTotalSizeValidatedCount": 271,
        "primaryD3dDisassemblyValidatedCount": 271,
        "primaryUniqueDxbcCount": 240,
        "exactMaterialShaderMapJoinCount": 0,
        "arithmeticEvaluatorImplementedCount": 0,
    }
    require(summary == expected_summary, "ShaderCache receipt denominator changed")
    require(len(receipt.get("materialNativeKeys", [])) == 23, "Material native key denominator changed")
    require(len(receipt.get("recipeNativeKeys", [])) == 27, "recipe native key denominator changed")
    mic_tail_probe = receipt.get("micTailShaderObjectIdProbe") or {}
    require(
        mic_tail_probe.get("recipeCount") == 27
        and mic_tail_probe.get("staticPermutationMicCount") == 24
        and mic_tail_probe.get("descriptorShaderIdCandidateCount") == 271
        and mic_tail_probe.get("uniqueDescriptorShaderIdCandidateCount") == 271
        and mic_tail_probe.get("alignedNonzero16ByteWindowCount") == 4_816
        and mic_tail_probe.get("directShaderIdMatchCount") == 0
        and mic_tail_probe.get("status") == "MIC_TAIL_CONTAINS_NO_DIRECT_SHADER_OBJECT_ID"
        and len(mic_tail_probe.get("rows", [])) == 27
        and not mic_tail_probe.get("matches"),
        "MIC tail/direct shader-object ID probe changed",
    )
    recipe_by_id = {
        row["recipeId"]: row for row in receipt.get("recipeNativeKeys", [])
    }
    for row in mic_tail_probe["rows"]:
        recipe = recipe_by_id.get(row.get("recipeId")) or {}
        require(
            row.get("alignedNonzero16ByteWindowCount")
            == recipe.get("alignedNonzero16ByteWindowCount")
            and row.get("candidateDigestSha256")
            == recipe.get("alignedNonzero16ByteWindowSha256")
            and row.get("directShaderIdMatchCount") == 0
            and not row.get("matches"),
            "MIC tail shader-object probe row changed",
        )
    inventory = (receipt.get("externalEvidence") or {}).get("boundedMaterialInventory") or {}
    require(
        inventory.get("scope") == "30_RECONSTRUCTED_DIMENSIONMASTER_MATERIAL_REPORTS_NOT_FULL_INSTALLATION"
        and inventory.get("reportCount") == 30
        and len(inventory.get("reportIdentities", [])) == 30
        and inventory.get("targetFamilyCoverageCount") == 21
        and inventory.get("targetMaterialRowCount") == 22
        and inventory.get("alternateObjectCandidateCount") == 1
        and inventory.get("unexpectedPhysicalPackageCandidateCount") == 0,
        "bounded Material inventory evidence changed",
    )
    matrices = receipt.get("materialTopologyCompleteness", [])
    require(len(matrices) == 23, "Material topology matrix denominator changed")
    alternate_count = 0
    improvement_count = 0
    for matrix in matrices:
        candidates = matrix.get("candidates") or []
        source_rows = [row for row in candidates if row.get("candidateRole") == "SOURCE_EXACT_DEPENDENCY"]
        global_rows = [row for row in candidates if row.get("candidateRole") == "CURRENT_GLOBAL_PACKAGE_CROSS_REVISION"]
        require(len(source_rows) == 1 and len(global_rows) == 1, "Material topology source/current pairing changed")
        source_topology = source_rows[0].get("topology") or {}
        recomputed_improving = []
        for candidate in candidates:
            topology = candidate.get("topology") or {}
            require(topology.get("topologyStatus") == "COOKED_PARTIAL", "Material topology fidelity changed")
            same_denominator = topology.get("expressionEntryCount") == source_topology.get("expressionEntryCount")
            improves = (
                candidate.get("candidateRole") != "SOURCE_EXACT_DEPENDENCY"
                and same_denominator
                and topology.get("nullExpressionCount") <= source_topology.get("nullExpressionCount")
                and topology.get("unresolvedInputEdgeCount") <= source_topology.get("unresolvedInputEdgeCount")
                and (
                    topology.get("nullExpressionCount") < source_topology.get("nullExpressionCount")
                    or topology.get("unresolvedInputEdgeCount") < source_topology.get("unresolvedInputEdgeCount")
                )
            )
            require(candidate.get("sameExpressionEntryDenominatorAsSource") == same_denominator, "Material topology denominator flag changed")
            require(candidate.get("strictParetoImprovementOverSource") == improves, "Material topology improvement flag changed")
            if improves:
                recomputed_improving.append(candidate)
            if candidate.get("candidateRole") == "SAME_PACKAGE_SAME_LEAF_ALTERNATE_OBJECT":
                alternate_count += 1
        require(
            matrix.get("minimumObservedNullExpressionCount")
            == min(row["topology"]["nullExpressionCount"] for row in candidates)
            and matrix.get("minimumObservedUnresolvedInputEdgeCount")
            == min(row["topology"]["unresolvedInputEdgeCount"] for row in candidates)
            and matrix.get("strictParetoImprovementCandidateCount") == len(recomputed_improving),
            "Material topology completeness summary changed",
        )
        require(
            (matrix.get("reconstructedEvaluatorOracleCandidate") is not None)
            == (len(recomputed_improving) == 1),
            "Material topology evaluator candidate changed",
        )
        improvement_count += len(recomputed_improving)
    require(alternate_count == 1, "Material alternate topology candidate denominator changed")
    require(improvement_count == summary["strictCrossRevisionTopologyImprovementCandidateCount"], "Material topology improvement total changed")
    require(len(receipt.get("shaderCacheCandidates", [])) == 11, "ShaderCache candidate denominator changed")
    primary = receipt.get("primaryShaderCache") or {}
    require(len(primary.get("codeRecords", [])) == 271, "primary ShaderCache code rows changed")
    require(len(primary.get("groups", [])) == 32, "primary ShaderCache group rows changed")
    decision = receipt.get("joinDecision") or {}
    require(decision.get("directNativeKeyMatchCount") == 0, "direct Material key match changed")
    require(decision.get("commonEndianOrOrderMatchCount") == 0, "transformed Material key match changed")
    require(decision.get("micDirectEndianOrHashMatchCount") == 0, "MIC key/hash match changed")
    require(
        decision.get("micTailAlignedNonzeroWindowCount") == 4_816
        and decision.get("micTailDirectShaderObjectIdMatchCount") == 0
        and decision.get("micTailDirectShaderObjectIdStatus")
        == "MIC_TAIL_CONTAINS_NO_DIRECT_SHADER_OBJECT_ID",
        "MIC tail direct shader-object join decision changed",
    )
    require(
        decision.get("strictCrossRevisionTopologyImprovementCandidateCount")
        == improvement_count,
        "Material topology join decision changed",
    )
    require(decision.get("exactMaterialShaderMapJoinCount") == 0, "unproven Material shader-map join opened")
    require(tuple(decision.get("blockers", [])) == BLOCKERS, "ShaderCache blocker set changed")
    admission = receipt.get("admission") or {}
    require(admission == {"arithmeticEvaluatorImplementedCount": 0, "executionAdmission": False, "productAdmission": False}, "ShaderCache admission opened")


def check_or_write(path: Path, receipt: dict[str, Any], check: bool) -> None:
    if check:
        require(path.is_file(), f"ShaderCache receipt is missing: {path}")
        current = read_json(path)
        require(current == receipt, "ShaderCache receipt is stale")
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
    parser.add_argument("--material-contract", type=Path, default=DEFAULT_MATERIAL_CONTRACT)
    parser.add_argument("--global-material-package", type=Path, default=DEFAULT_GLOBAL_MATERIAL_PACKAGE)
    parser.add_argument("--source-package-root", type=Path, default=DEFAULT_SOURCE_PACKAGE_ROOT)
    parser.add_argument("--material-inventory-root", type=Path, default=DEFAULT_MATERIAL_INVENTORY_ROOT)
    parser.add_argument("--shader-cache-package", type=Path, default=DEFAULT_SHADER_CACHE_PACKAGE)
    parser.add_argument("--d3dcompiler", type=Path, default=DEFAULT_D3DCOMPILER)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--validate-only", action="store_true")
    args = parser.parse_args()

    if args.validate_only:
        require(args.output.is_file(), f"ShaderCache receipt is missing: {args.output}")
        validate_receipt(read_json(args.output), args.material_contract)
        print("PASS: Artist F ShaderCache receipt shallow material=23 cache=1596 primary=271/271/271 unique=240 join=0 product=false")
        return 0

    receipt = build_receipt(
        args.material_contract,
        args.source_package_root,
        args.material_inventory_root,
        args.global_material_package,
        args.shader_cache_package,
        args.d3dcompiler,
    )
    validate_receipt(receipt, args.material_contract)
    check_or_write(args.output, receipt, args.check)
    mode = "check" if args.check else "write"
    print(f"PASS: Artist F ShaderCache receipt {mode} material=23 cache=1596 primary=271/271/271 unique=240 join=0 product=false")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
