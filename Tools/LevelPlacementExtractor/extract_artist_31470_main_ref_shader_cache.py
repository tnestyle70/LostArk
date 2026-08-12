#!/usr/bin/env python3
"""Extract the exact Artist F main material maps from Lost Ark RefShaderCache.

This is deliberately a focused, fail-closed extractor.  It joins only the two
material families used by Artist skill 31470 occurrences #9/#10/#11, preserves
their complete FStaticParameterSet, material-map/VF references, original DXBC,
and the recursively serialized uniform-expression set.  It does not mutate the
runtime evaluator or treat a successful structural join as visual admission.
"""

from __future__ import annotations

import argparse
import ctypes
import hashlib
import json
import math
import os
import re
import struct
import subprocess
from pathlib import Path
from typing import Any, Iterable

from derive_artist_31470_main_shader_map_identity import (
    engine_equivalent_static_parameter_set,
    normalized_static_parameter_set,
)
from build_artist_31470_custom_handler_oracle import parse_pe_exports
from extract_artist_31470_shader_cache_oracle import (
    canonical_json_sha256,
    parse_static_parameter_set,
    read_json,
    validate_dxbc_container,
)
from extract_ue3_placements import (
    LostArkPackageRangeReader,
    decompress_lz4_block,
    package_ref_name,
    package_ref_path,
    parse_export_table,
    parse_import_table,
    parse_name_table,
    read_package_summary,
)


SCHEMA = "lostark.artist-31470-main-ref-shader-cache-receipt"
FORMAT_VERSION = 2
REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = Path(__file__).resolve()
DEFAULT_IDENTITY_RECEIPT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.main-shader-map-identity.receipt.json"
)
DEFAULT_MATERIAL_CONTRACT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.typed-material-evidence-contract.json"
)
DEFAULT_NATIVE_RECEIPT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-native-resource.receipt.json"
)
DEFAULT_SOURCE_INVENTORY = REPO_ROOT / (
    "Data/Effects/Imported/Artist/skill.31470.source-active-effect-inventory.receipt.json"
)
DEFAULT_OUTPUT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.main-ref-shader-cache.receipt.json"
)
DEFAULT_EVIDENCE_ROOT = Path(
    r"C:\Users\user\Desktop\Resource_LostArk\01_Extracted\Effect\ARTIST\31470_TrackA_20260812\OfficialRefShaderCacheV974"
)
DEFAULT_OFFICIAL_MANIFEST = DEFAULT_EVIDENCE_ROOT / "45_975.json"
DEFAULT_OFFICIAL_ARCHIVE = DEFAULT_EVIDENCE_ROOT / "v974_16.gz"
DEFAULT_OFFICIAL_CACHE = (
    DEFAULT_EVIDENCE_ROOT / "EV2LG3OVEH3HGV7THTFFTM7TOKMCC.v974.upk"
)
DEFAULT_INSTALLED_ROOT = Path(
    r"C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC"
)
DEFAULT_INSTALLED_CACHE = DEFAULT_INSTALLED_ROOT / "EV2LG3OVEH3HGV7THTFFTM7TOKMCC.upk"
DEFAULT_D3DCOMPILER = Path(
    r"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\d3dcompiler_47.dll"
)
DEFAULT_OFFICIAL_EFENGINE = DEFAULT_EVIDENCE_ROOT / "EFEngine.v975.dll"
DEFAULT_OFFICIAL_EFENGINE_ARCHIVE = DEFAULT_EVIDENCE_ROOT / "v975_2.gz"


EXPECTED_MANIFEST = {
    "fileName": "45_975.json",
    "byteSize": 9_930_955,
    "rawSha256": "331bfb3ef14cafc5a31f9006bc7590540589d61718527f2668eeb58ee7ec96e9",
    "serviceCode": "45",
    "versionNo": 975,
}
EXPECTED_OFFICIAL_ARCHIVE = {
    "fileName": "v974_16.gz",
    "byteSize": 215_822_993,
    "md5": "6e90123c08642eae5a151170dc5649fa",
    "sha256": "d67bc17e5ff5ca08290fa4b49d5cf99eca72ef52a2cbe85e3e40f8c9f5f91e93",
}
EXPECTED_OFFICIAL_CACHE = {
    "fileName": "EV2LG3OVEH3HGV7THTFFTM7TOKMCC.v974.upk",
    "byteSize": 245_114_448,
    "md5": "1799ee1116fbb2811465576d8bb9e360",
    "sha256": "83609267d9476f393cfe793d3d8cd265535c3d855e2f1f79774c618f85ed6455",
    "logicalByteSize": 919_307_435,
    "nameCount": 12_599,
    "importCount": 2,
    "exportCount": 1,
    "serialOffset": 364_181,
    "serialSize": 918_943_254,
    "packageGuidHex": "55c36cd9990cd4f5da3c8e4b2528b661",
    "shaderTypeGroupCount": 111,
    "descriptorCount": 265_979,
    "embeddedCodeCount": 247_480,
    "shaderCodeSectionEndLogicalOffset": 613_360_985,
}
EXPECTED_INSTALLED_CACHE = {
    "fileName": "EV2LG3OVEH3HGV7THTFFTM7TOKMCC.upk",
    "byteSize": 249_252_877,
    "sha256": "747753186a5810722cf1a64df6cb6e0d3689ddf76f333949c4e54e3418077977",
    "logicalByteSize": 931_966_609,
    "nameCount": 12_633,
    "importCount": 2,
    "exportCount": 1,
    "serialOffset": 365_162,
    "serialSize": 931_601_447,
    "packageGuidHex": "55c36cd9990cd4f5da3c8e4b2528b661",
    "shaderTypeGroupCount": 111,
    "descriptorCount": 268_959,
    "embeddedCodeCount": 250_370,
    "shaderCodeSectionEndLogicalOffset": 622_539_223,
}
EXPECTED_D3DCOMPILER = {
    "fileName": "d3dcompiler_47.dll",
    "byteSize": 4_916_800,
    "sha256": "ce013eb1639f8e2620a509e73b33029108f55a293e304e33e38b72fd65c531b8",
}
EXPECTED_OFFICIAL_EFENGINE = {
    "fileName": "EFEngine.v975.dll",
    "byteSize": 19_929_968,
    "md5": "bff4869c0810da6dd249d016e599a65c",
    "sha256": "cc09e083c21bde705e4b8bc7a07d7014c618ef3f3a3ecf533e5b1fa326aa10e7",
    "archiveFileName": "v975_2.gz",
    "archiveByteSize": 16_955_137,
    "archiveMd5": "282714ea8c1df05bed62907bc5729625",
    "archiveSha256": "65d2003890edb0819b3a8f9f7191a1efc7d41136800973fc82d993b81828ccfd",
    "manifestFileVersion": 975,
    "manifestSequence": 2,
}
OFFICIAL_ABI_EXPORTS = {
    "materialShaderMapFindId": ("?FindId@FMaterialShaderMap@@SAPEAV1@AEBVFStaticParameterSet@@W4EShaderPlatform@@@Z", 0x009FC280),
    "materialShaderMapGetMaterialId": ("?GetMaterialId@FMaterialShaderMap@@QEBAAEBVFStaticParameterSet@@XZ", 0x002487C0),
    "shaderCacheFindStaticShaderMap": ("?FindStaticShaderMap@UShaderCache@@QEAAIAEBVFStaticParameterSet@@@Z", 0x00238880),
    "materialShaderMapGlobalLookup": ("?GIdToMaterialShaderMap@FMaterialShaderMap@@0PAV?$TMap@VFStaticParameterSet@@PEAVFMaterialShaderMap@@VFDefaultSetAllocator@@@@A", 0x02191138),
    "meshEmitterDynamicData": ("?GetDynamicData@FParticleMeshEmitterInstance@@UEAAPEAUFDynamicEmitterDataBase@@I@Z", 0x00B6E4E0),
    "meshEmitterReplayData": ("?FillReplayData@FParticleMeshEmitterInstance@@MEAAIAEAUFDynamicEmitterReplayDataBase@@@Z", 0x00B6AF10),
    "localVertexFactorySetup": ("?SetupVertexFactory@FStaticMeshRenderData@@QEAAXAEAVFLocalVertexFactory@@PEAVUStaticMesh@@PEAVFColorVertexBuffer@@@Z", 0x009C7300),
}
EXPECTED_SOURCE_INVENTORY_SHA256 = "96ac2147e97c9c7ba2a442367b1e55882eec0d8d021278185900cb5c3837c29a"
EXPECTED_MESH_OCCURRENCES = {
    "source-active-009": {
        "material": "fx_m_mi_01.fx_mi.fx_h_me_watertrail_01_2_tr",
        "emitter": "FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_7",
        "typeDataNodeId": "FX_PC_SDM_07:export:1246",
        "typeDataRecordSha256": "6015815bf49acf996d51fb126518761d165d1de40e60f7e06cb34dd1821bebce",
    },
    "source-active-010": {
        "material": "fx_m_mi_01.fx_mi.fx_h_me_watertrail_01_2_tr",
        "emitter": "FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_13",
        "typeDataNodeId": "FX_PC_SDM_07:export:1239",
        "typeDataRecordSha256": "d0ec079279f872be15a38ce5575e0fd831b38a5af82d626e8bc1ade1e86c18cc",
    },
    "source-active-011": {
        "material": "fx_m_mi_m_00.fx_m_pa_spritewave_01_19_tr",
        "emitter": "FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_6",
        "typeDataNodeId": "FX_PC_SDM_07:export:1245",
        "typeDataRecordSha256": "3f6666af85a71fc5612d2dd9b3978854c5b4b4c6afec2554a082bc20ae439f62",
    },
}


TARGETS = (
    {
        "label": "#9/#10_WATERTRAIL",
        "familyId": "material-family-89af5c77d8e35f99",
        "recipeId": "material-recipe-03cc03b86c1a4c8f",
        "occurrenceIds": ["source-active-009", "source-active-010"],
        "baseMaterialIdHex": "06e9a0ae14b09646b949a245fc42aa3c",
        "normalizedStaticParameterSetSha256": "32d71b8321f85a868c18f48662fedf79a74c79b85ce41cf708c1d0b88605c8c3",
        "engineEqualityStaticParameterSetSha256": "54a52a78d4c82bd42962193bcb3a64e28cce1275eb69c9affb1d3006478abcc3",
        "expectedVfCount": 5,
        "expectedFriendlyName": "FX_M_Me_WaterTrail_01_Tr",
        "expectedOfficialMapStart": 890_388_705,
        "expectedOfficialMapEnd": 890_392_632,
        "expectedInstalledMapStart": 904_339_872,
        "expectedInstalledMapEnd": 904_343_799,
        "expectedPixelShaderIdHex": "70bf2a6e9bf4f0478cecbfc43c4e160f",
        "expectedPixelDxbcSha256": "b16e274cfad5ba27b3be0f8c8bb4c1e663768ded79d4ddf119204fb8a1e9c6bb",
        "expectedUniformCounts": [5, 37, 3, 0],
        "expectedShaderObjectByteSize": 728,
        "expectedBindingCounts": [9, 5, 3],
        "expectedSamplePairs": {"t0/s0": 2, "t1/s1": 1, "t2/s2": 1},
        "expectedBindingArraysSha256": "0be0f5bd8a231dcea84e8c5ebe91d5ea0a0f64ac059b4d127a466b7f2946018f",
        "expectedConstantBuffer0Float4Count": 16,
        "expectedUnboundScalarGroups": [9],
        "expectedTextureBindings": [
            [0, "t0", "s0", "fx_tex_00.fx_a_noise_011"],
            [1, "t1", "s1", "fx_tex_00.fx_a_fluid_017_n"],
            [2, "t2", "s2", "fx_tex_04.fx_h_wave_01"],
        ],
        "expectedScalarBindingProjection": [[index, (index + 7) * 16, 16, 0] for index in range(9)],
        "expectedVectorBindingProjection": [[index, (index + 2) * 16, 16, 0] for index in range(5)],
        "expectedTextureBindingProjection": [[0, 0, 1, 0], [1, 1, 1, 1], [2, 2, 1, 2]],
        "expectedOfficialShaderObject": {"logicalOffset": 614255083, "rawSha256": "4e0268ea1a84cbdc6f8713ff17ddf3f659c1c87c44c0a46749a8c4864d3cc503", "prefix188Sha256": "bcbeea8f5e04e5b440c547f9ef089b1c54ff4a9f37918aba82fe58364c0b9269"},
        "expectedInstalledShaderObject": {"logicalOffset": 623438895, "rawSha256": "515fd22d6303c531ddcee9681bcbf8da737e25c30ee2fc60cb8d003059448bb9", "prefix188Sha256": "2e895505847e8f0ed69425a79406f44da879a01d2b607875874f8d0894f7e363"},
    },
    {
        "label": "#11_SPRITEWAVE",
        "familyId": "material-family-097bd8d9597721b5",
        "recipeId": "material-recipe-daf220acad2b656e",
        "occurrenceIds": ["source-active-011"],
        "baseMaterialIdHex": "b1f4ebf9dc948c41bd2830d999ba16cc",
        "normalizedStaticParameterSetSha256": "6d75571de189229a3052963c9c227d1be9e2d38ccbf88a1cf245abdd90a219a2",
        "engineEqualityStaticParameterSetSha256": "468bfdf79d6dc23e741433c076e865a0dc985c19ebfc0e1519efd8ca20aad846",
        "expectedVfCount": 8,
        "expectedFriendlyName": "FX_M_Pa_Spritewave_01_Tr",
        "expectedOfficialMapStart": 918_433_886,
        "expectedOfficialMapEnd": 918_441_159,
        "expectedInstalledMapStart": 931_093_060,
        "expectedInstalledMapEnd": 931_100_333,
        "expectedPixelShaderIdHex": "39f7e63594b10f4a9237dc9eb19a1dfc",
        "expectedPixelDxbcSha256": "7e8dbb706620c5ec6d991d99c70d6daa6b9df2258060796597c0678358b4f5e0",
        "expectedUniformCounts": [11, 52, 4, 0],
        "expectedShaderObjectByteSize": 818,
        "expectedBindingCounts": [11, 11, 4],
        "expectedSamplePairs": {"t0/s1": 1, "t1/s0": 1, "t2/s2": 1, "t3/s3": 1},
        "expectedBindingArraysSha256": "cc93d18931fdfd6dfdfa2b7ff5385428376131e690615fd6e9aa4846ad0a4de6",
        "expectedConstantBuffer0Float4Count": 24,
        "expectedUnboundScalarGroups": [0, 3],
        "expectedTextureBindings": [
            [0, "t1", "s0", "fx_tex_05.fx_m_trail_004_cl"],
            [1, "t0", "s1", "fx_tex_02.fx_d_noise_033"],
            [2, "t2", "s2", "fx_tex_05.fx_m_noise_001"],
            [3, "t3", "s3", "fx_tex_05.fx_m_atypical_012"],
        ],
        "expectedScalarBindingProjection": [
            [group, slot * 16, 16, 0]
            for group, slot in zip([1, 2, *range(4, 13)], range(13, 24))
        ],
        "expectedVectorBindingProjection": [[index, (index + 2) * 16, 16, 0] for index in range(11)],
        "expectedTextureBindingProjection": [[0, 1, 1, 0], [1, 0, 1, 1], [2, 2, 1, 2], [3, 3, 1, 3]],
        "expectedOfficialShaderObject": {"logicalOffset": 803756449, "rawSha256": "fb32feb5256bddc9ed71b7cbdee160b16f11965836c09319104bd0c2a7e1a9a0", "prefix188Sha256": "14cb3e141a4ad705a38dd725f67fc381d8590dd805b9ec378bdf88469a3d314f"},
        "expectedInstalledShaderObject": {"logicalOffset": 815177517, "rawSha256": "1c489c52becf0f3222942ff9d253e211954503355ab247d086092d6ff8a150bd", "prefix188Sha256": "32604ca5003c3094e08209b693486b3e6f200894339d5ed9abc78c328e9b5e07"},
    },
)

LOCAL_VF = "flocalvertexfactory"
BASE_PASS_PIXEL = "tbasepasspixelshaderfnolightmappolicyskylight"
BASE_PASS_VERTEX_TYPES = (
    "tbasepassvertexshaderfnolightmappolicyfspheredensitypolicy",
    "tbasepassvertexshaderfnolightmappolicyflinearhalfspacedensitypolicy",
    "tbasepassvertexshaderfnolightmappolicyfconstantdensitypolicy",
    "tbasepassvertexshaderfnolightmappolicyfnodensitypolicy",
)
EXPECTED_SHARED_VERTEX_DXBC = {
    "tbasepassvertexshaderfnolightmappolicyfspheredensitypolicy": "c2f6d7a0da34e1b0d47ca2addf9e4c93cf8453946fc6783a702639c64b1d5527",
    "tbasepassvertexshaderfnolightmappolicyflinearhalfspacedensitypolicy": "b912f7eddafaec8f5e398dbe126758df8155a6684f47ea4bd54ac209ead93398",
    "tbasepassvertexshaderfnolightmappolicyfconstantdensitypolicy": "03101ef7f12c18011615fe836897ade13196d60af897b0c194cc57d5e7ae2528",
    "tbasepassvertexshaderfnolightmappolicyfnodensitypolicy": "48ff9e247e34d438a3bcf08cdfa8e6891213b409be6d4781ec7224f1d8e67491",
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def digest_file(path: Path, algorithm: str = "sha256") -> str:
    digest = hashlib.new(algorithm)
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


class BufferedLogicalCursor:
    """Small-read cursor over a streaming package range reader."""

    def __init__(
        self,
        reader: LostArkPackageRangeReader,
        offset: int,
        buffer_size: int = 8 * 1024 * 1024,
    ) -> None:
        self.reader = reader
        self.offset = offset
        self.buffer_size = buffer_size
        self._buffer_start = 0
        self._buffer = b""

    def read(self, size: int) -> bytes:
        require(size >= 0, "negative cursor read")
        if not (
            self._buffer_start <= self.offset
            and self.offset + size <= self._buffer_start + len(self._buffer)
        ):
            self._buffer_start = self.offset
            available = self.reader.logical_size - self.offset
            require(size <= available, "cursor read exceeds logical package")
            self._buffer = self.reader.read_logical_range(
                self.offset, min(max(size, self.buffer_size), available)
            )
        local = self.offset - self._buffer_start
        result = self._buffer[local : local + size]
        require(len(result) == size, "short buffered logical read")
        self.offset += size
        return result

    def skip(self, size: int) -> None:
        require(size >= 0, "negative cursor skip")
        require(self.offset + size <= self.reader.logical_size, "cursor skip exceeds package")
        self.offset += size

    def u32(self) -> int:
        return struct.unpack("<I", self.read(4))[0]

    def i32(self) -> int:
        return struct.unpack("<i", self.read(4))[0]

    def fname(self, names: list[str]) -> tuple[str, int]:
        index, number = struct.unpack("<ii", self.read(8))
        require(0 <= index < len(names), "streamed FName index is invalid")
        return names[index], number


def read_fname_at(data: bytes, offset: int, names: list[str]) -> tuple[str, int, int]:
    require(offset + 8 <= len(data), "FName is truncated")
    index, number = struct.unpack_from("<ii", data, offset)
    require(0 <= index < len(names), "FName index is invalid")
    return names[index], number, offset + 8


def read_fstring_at(data: bytes, offset: int) -> tuple[str, int]:
    require(offset + 4 <= len(data), "FString length is truncated")
    count = struct.unpack_from("<i", data, offset)[0]
    offset += 4
    require(abs(count) <= 1_000_000, "FString length is invalid")
    if count == 0:
        return "", offset
    if count > 0:
        require(offset + count <= len(data), "ANSI FString is truncated")
        raw = data[offset : offset + count]
        require(raw[-1:] == b"\x00", "ANSI FString terminator is absent")
        return raw[:-1].decode("utf-8", "strict"), offset + count
    byte_count = -count * 2
    require(offset + byte_count <= len(data), "UTF-16 FString is truncated")
    raw = data[offset : offset + byte_count]
    require(raw[-2:] == b"\x00\x00", "UTF-16 FString terminator is absent")
    return raw[:-2].decode("utf-16-le", "strict"), offset + byte_count


def _public_expression(value: dict[str, Any]) -> dict[str, Any]:
    return {
        key: (
            _public_expression(item)
            if isinstance(item, dict) and "typeName" in item
            else item
        )
        for key, item in value.items()
        if key not in ("endOffset",)
    }


def _semantic_expression(value: dict[str, Any]) -> dict[str, Any]:
    result: dict[str, Any] = {"typeName": value["typeName"]}
    for key, item in value.items():
        if key in ("typeName", "offset", "byteSize", "endOffset"):
            continue
        if isinstance(item, dict) and "typeName" in item:
            result[key] = _semantic_expression(item)
        else:
            result[key] = item
    return result


def parse_uniform_expression(
    data: bytes,
    offset: int,
    names: list[str],
    *,
    depth: int = 0,
    node_budget: list[int] | None = None,
) -> dict[str, Any]:
    """Parse the packed UE3 expression forms observed in both exact maps."""

    require(depth <= 64, "uniform expression recursion is excessive")
    if node_budget is None:
        node_budget = [0]
    node_budget[0] += 1
    require(node_budget[0] <= 8192, "uniform expression node budget is excessive")
    start = offset
    type_name, number, offset = read_fname_at(data, offset, names)
    require(number == 0, "numbered uniform-expression type is unsupported")
    require(
        type_name.casefold().startswith("fmaterialuniformexpression"),
        "uniform-expression type name is invalid",
    )
    folded = type_name.casefold()
    row: dict[str, Any] = {"typeName": folded, "offset": start}

    if folded == "fmaterialuniformexpressionfoldedmath":
        row["a"] = parse_uniform_expression(data, offset, names, depth=depth + 1, node_budget=node_budget)
        row["b"] = parse_uniform_expression(data, row["a"]["endOffset"], names, depth=depth + 1, node_budget=node_budget)
        offset = row["b"]["endOffset"]
        require(offset < len(data), "FoldedMath operation is truncated")
        row["operationOrdinal"] = data[offset]
        row["operationNameIfObserved"] = (
            "MUL" if row["operationOrdinal"] == 2 else None
        )
        require(row["operationOrdinal"] == 2, "unobserved FoldedMath operation is unsupported")
        offset += 1
    elif folded == "fmaterialuniformexpressionappendvector":
        row["a"] = parse_uniform_expression(data, offset, names, depth=depth + 1, node_budget=node_budget)
        row["b"] = parse_uniform_expression(data, row["a"]["endOffset"], names, depth=depth + 1, node_budget=node_budget)
        offset = row["b"]["endOffset"]
        require(offset + 4 <= len(data), "AppendVector component count is truncated")
        row["componentsFromA"] = struct.unpack_from("<I", data, offset)[0]
        require(row["componentsFromA"] == 1, "unobserved AppendVector component count is unsupported")
        offset += 4
    elif folded == "fmaterialuniformexpressionsine":
        row["input"] = parse_uniform_expression(data, offset, names, depth=depth + 1, node_budget=node_budget)
        offset = row["input"]["endOffset"]
        require(offset + 4 <= len(data), "Sine UBOOL is truncated")
        value = struct.unpack_from("<I", data, offset)[0]
        require(value in (0, 1), "Sine UBOOL is invalid")
        row["isCosine"] = bool(value)
        offset += 4
    elif folded == "fmaterialuniformexpressionconstant":
        require(offset + 17 <= len(data), "constant uniform expression is truncated")
        row["value"] = list(struct.unpack_from("<4f", data, offset))
        require(all(math.isfinite(value) for value in row["value"]), "constant value is non-finite")
        row["valueTypeOrdinal"] = data[offset + 16]
        require(row["valueTypeOrdinal"] == 15, "constant value type changed")
        offset += 17
    elif folded == "fmaterialuniformexpressionscalarparameter":
        name, name_number, offset = read_fname_at(data, offset, names)
        require(name_number == 0, "numbered scalar parameter is unsupported")
        require(offset + 4 <= len(data), "scalar default is truncated")
        row["parameterName"] = name
        row["defaultValue"] = struct.unpack_from("<f", data, offset)[0]
        require(math.isfinite(row["defaultValue"]), "scalar default is non-finite")
        offset += 4
    elif folded == "fmaterialuniformexpressionvectorparameter":
        name, name_number, offset = read_fname_at(data, offset, names)
        require(name_number == 0, "numbered vector parameter is unsupported")
        require(offset + 16 <= len(data), "vector default is truncated")
        row["parameterName"] = name
        row["defaultValue"] = list(struct.unpack_from("<4f", data, offset))
        require(all(math.isfinite(value) for value in row["defaultValue"]), "vector default is non-finite")
        offset += 16
    elif folded == "fmaterialuniformexpressiontime":
        pass
    elif folded == "fmaterialuniformexpressiontexture":
        require(offset + 4 <= len(data), "fixed texture index is truncated")
        row["referencedTextureIndex"] = struct.unpack_from("<i", data, offset)[0]
        require(row["referencedTextureIndex"] >= 0, "fixed texture index is negative")
        offset += 4
    elif folded == "fmaterialuniformexpressiontextureparameter":
        name, name_number, offset = read_fname_at(data, offset, names)
        require(name_number == 0, "numbered texture parameter is unsupported")
        require(offset + 4 <= len(data), "texture fallback index is truncated")
        row["parameterName"] = name
        row["referencedTextureIndex"] = struct.unpack_from("<i", data, offset)[0]
        require(row["referencedTextureIndex"] >= 0, "texture fallback index is negative")
        offset += 4
    else:
        raise ValueError(f"unsupported uniform-expression type: {type_name}")

    row["byteSize"] = offset - start
    row["endOffset"] = offset
    return row


def parse_uniform_expression_set(
    data: bytes, offset: int, names: list[str]
) -> dict[str, Any]:
    start = offset
    arrays: dict[str, list[dict[str, Any]]] = {}
    node_budget = [0]
    total_top_level = 0
    for array_name in (
        "pixelVectorExpressions",
        "pixelScalarExpressions",
        "pixelTexture2DExpressions",
        "textureCubeExpressions",
        "vertexVectorExpressions",
        "vertexScalarExpressions",
        "vertexTexture2DExpressions",
        "hullVectorExpressions",
        "hullScalarExpressions",
        "hullTexture2DExpressions",
        "domainVectorExpressions",
        "domainScalarExpressions",
        "domainTexture2DExpressions",
    ):
        require(offset + 4 <= len(data), f"{array_name} count is truncated")
        count = struct.unpack_from("<i", data, offset)[0]
        offset += 4
        require(0 <= count <= 4096, f"{array_name} count is invalid")
        total_top_level += count
        require(total_top_level <= 4096, "uniform-expression top-level denominator is excessive")
        rows = []
        for _ in range(count):
            expression = parse_uniform_expression(data, offset, names, node_budget=node_budget)
            rows.append(_public_expression(expression))
            offset = expression["endOffset"]
        arrays[array_name] = rows
    raw = data[start:offset]
    semantic = {
        key: [_semantic_expression(row) for row in rows]
        for key, rows in arrays.items()
    }
    return {
        **arrays,
        "offset": start,
        "byteSize": offset - start,
        "rawSha256": sha256_bytes(raw),
        "semanticSha256": canonical_json_sha256(semantic),
        "endOffset": offset,
    }


class D3DDisassembler:
    def __init__(self, path: Path) -> None:
        require(path.is_file(), f"D3D compiler is missing: {path}")
        require(path.name.casefold() == EXPECTED_D3DCOMPILER["fileName"].casefold(), "D3D compiler name changed")
        require(path.stat().st_size == EXPECTED_D3DCOMPILER["byteSize"], "D3D compiler size changed")
        require(digest_file(path) == EXPECTED_D3DCOMPILER["sha256"], "D3D compiler SHA changed")
        self.identity = {
            "fileName": path.name,
            "physicalByteSize": path.stat().st_size,
            "rawSha256": digest_file(path),
        }
        self._dll = ctypes.WinDLL(str(path))
        self._function = self._dll.D3DDisassemble
        self._function.argtypes = [
            ctypes.c_void_p,
            ctypes.c_size_t,
            ctypes.c_uint32,
            ctypes.c_char_p,
            ctypes.POINTER(ctypes.c_void_p),
        ]
        self._function.restype = ctypes.c_long

    def disassemble(self, bytecode: bytes) -> dict[str, Any]:
        source = ctypes.create_string_buffer(bytecode)
        output = ctypes.c_void_p()
        result = self._function(source, len(bytecode), 0, None, ctypes.byref(output))
        require(result >= 0 and bool(output.value), f"D3DDisassemble failed: 0x{result & 0xFFFFFFFF:08X}")
        table = ctypes.cast(output, ctypes.POINTER(ctypes.POINTER(ctypes.c_void_p))).contents
        get_pointer = ctypes.WINFUNCTYPE(ctypes.c_void_p, ctypes.c_void_p)(table[3])
        get_size = ctypes.WINFUNCTYPE(ctypes.c_size_t, ctypes.c_void_p)(table[4])
        release = ctypes.WINFUNCTYPE(ctypes.c_ulong, ctypes.c_void_p)(table[2])
        try:
            text = ctypes.string_at(get_pointer(output), get_size(output)).decode("utf-8", "strict").rstrip("\x00")
        finally:
            release(output)
        normalized = text.replace("\r\n", "\n").replace("\r", "\n")
        lines = [line.strip() for line in normalized.splitlines()]
        instructions = [
            line
            for line in lines
            if line
            and not line.startswith("//")
            and not line.startswith("dcl_")
            and not re.fullmatch(r"(?:ps|vs|gs|hs|ds|cs)_\d+_\d+", line)
        ]
        profiles = [line for line in lines if re.fullmatch(r"(?:ps|vs|gs|hs|ds|cs)_\d+_\d+", line)]
        require(len(profiles) == 1, "DXBC profile is ambiguous")
        declarations = [line for line in lines if line.startswith("dcl_")]
        sample_rows = []
        for index, line in enumerate(instructions):
            if line.startswith("sample_"):
                registers = re.findall(r"\b([ts]\d+)\b", line)
                require(len(registers) == 2, "sample resource/sampler registers are ambiguous")
                sample_rows.append(
                    {
                        "instructionIndex": index,
                        "instruction": line,
                        "textureRegister": registers[0],
                        "samplerRegister": registers[1],
                    }
                )
        return {
            "profile": profiles[0],
            "normalizedDisassemblySha256": sha256_bytes(normalized.encode("utf-8")),
            "declarationSha256": canonical_json_sha256(declarations),
            "instructionSha256": canonical_json_sha256(instructions),
            "instructionCount": len(instructions),
            "declarations": declarations,
            "instructions": instructions,
            "sampleInstructions": sample_rows,
        }


def package_tables(path: Path) -> dict[str, Any]:
    summary = read_package_summary(path)
    reader = LostArkPackageRangeReader(path, summary)
    header = reader.read_logical_range(0, summary.header_size)
    names = parse_name_table(header, summary)
    imports = parse_import_table(header, summary, names)
    exports = parse_export_table(header, summary, names)
    require(len(exports) == 1, "RefShaderCache export denominator changed")
    export = exports[0]
    class_name = package_ref_name(export.class_index, imports, exports) or ""
    require(class_name.casefold() == "shadercache", "RefShaderCache export class changed")
    require(export.object_name.casefold() == "cacheobject", "RefShaderCache object name changed")
    return {
        "summary": summary,
        "reader": reader,
        "names": names,
        "imports": imports,
        "exports": exports,
        "export": export,
        "identity": {
            "fileName": path.name,
            "physicalByteSize": path.stat().st_size,
            "rawSha256": digest_file(path),
            "packageVersion": summary.version,
            "licenseeVersion": summary.licensee_version,
            "engineVersion": summary.engine_version,
            "cookerVersion": summary.cooker_version,
            "packageGuidHex": summary.package_guid_hex,
            "logicalByteSize": reader.logical_size,
            "nameCount": summary.name_count,
            "importCount": summary.import_count,
            "exportCount": summary.export_count,
            "objectPath": package_ref_path(1, imports, exports),
            "className": class_name,
            "serialOffset": export.serial_offset,
            "serialSize": export.serial_size,
        },
    }


def parse_cache_code_index(package: dict[str, Any]) -> dict[str, Any]:
    reader: LostArkPackageRangeReader = package["reader"]
    names: list[str] = package["names"]
    export = package["export"]
    cursor = BufferedLogicalCursor(reader, export.serial_offset)
    require(cursor.i32() == -1, "RefShaderCache net index changed")
    property_name, property_number = cursor.fname(names)
    require(property_name.casefold() == "none" and property_number == 0, "RefShaderCache property terminator changed")
    require(cursor.u32() == 0, "RefShaderCache native revision changed")
    platform = cursor.read(1)[0]
    require(platform == 4, "RefShaderCache shader platform changed")
    group_count = cursor.u32()
    require(group_count == 111, "RefShaderCache group denominator changed")

    descriptor_by_id: dict[str, dict[str, Any]] = {}
    groups = []
    total_descriptors = 0
    total_codes = 0
    for group_index in range(group_count):
        group_offset = cursor.offset
        shader_type, shader_type_number = cursor.fname(names)
        require(shader_type_number == 0, "numbered shader type is unsupported")
        descriptor_count = cursor.u32()
        require(0 < descriptor_count <= 100_000, "descriptor count is invalid")
        descriptor_offset = cursor.offset
        descriptor_bytes = cursor.read(descriptor_count * 24)
        code_count = cursor.u32()
        require(0 < code_count <= descriptor_count, "embedded code count is invalid")
        code_positions = []
        code_header_digest = hashlib.sha256()
        for code_index in range(code_count):
            code_header_offset = cursor.offset
            uncompressed_size, compressed_size = struct.unpack("<II", cursor.read(8))
            require(32 <= uncompressed_size <= 64 * 1024 * 1024, "DXBC uncompressed size is invalid")
            require(0 < compressed_size <= reader.logical_size - cursor.offset, "DXBC compressed size is invalid")
            compressed_offset = cursor.offset
            cursor.skip(compressed_size)
            code_positions.append(
                {
                    "codeIndex": code_index,
                    "codeHeaderLogicalOffset": code_header_offset,
                    "compressedLogicalOffset": compressed_offset,
                    "compressedByteSize": compressed_size,
                    "uncompressedByteSize": uncompressed_size,
                }
            )
            code_header_digest.update(struct.pack("<II", uncompressed_size, compressed_size))
        mapped_descriptor_count = 0
        for descriptor_index in range(descriptor_count):
            raw = descriptor_bytes[descriptor_index * 24 : (descriptor_index + 1) * 24]
            shader_id = raw[:16].hex()
            code_index, opaque = struct.unpack_from("<II", raw, 16)
            require(shader_id not in descriptor_by_id, "ShaderCache descriptor ID is duplicated")
            code_position = code_positions[code_index] if code_index < code_count else None
            if code_position is not None:
                mapped_descriptor_count += 1
            descriptor_by_id[shader_id] = {
                "groupIndex": group_index,
                "shaderType": shader_type,
                "descriptorIndex": descriptor_index,
                "descriptorLogicalOffset": descriptor_offset + descriptor_index * 24,
                "descriptorRawSha256": sha256_bytes(raw),
                "codeIndexCandidate": code_index,
                "opaqueDescriptorTailU32": opaque,
                "codePosition": code_position,
            }
        groups.append(
            {
                "groupIndex": group_index,
                "logicalOffset": group_offset,
                "shaderType": shader_type,
                "descriptorCount": descriptor_count,
                "embeddedCodeCount": code_count,
                "mappedDescriptorCount": mapped_descriptor_count,
                "descriptorTableSha256": sha256_bytes(descriptor_bytes),
                "codeHeaderProjectionSha256": code_header_digest.hexdigest(),
            }
        )
        total_descriptors += descriptor_count
        total_codes += code_count

    shader_code_end = cursor.offset
    tail_header = reader.read_logical_range(shader_code_end, 8)
    tail_platform, shader_object_count = struct.unpack("<II", tail_header)
    require(tail_platform == platform, "native shader-object platform changed")
    require(shader_object_count == total_descriptors, "native shader-object denominator differs from descriptors")
    return {
        "platform": platform,
        "groupCount": group_count,
        "groups": groups,
        "descriptorCount": total_descriptors,
        "embeddedCodeCount": total_codes,
        "shaderCodeSectionEndLogicalOffset": shader_code_end,
        "shaderObjectCount": shader_object_count,
        "descriptorById": descriptor_by_id,
    }


def find_shader_object_binding(
    package: dict[str, Any],
    code_index: dict[str, Any],
    shader_id_hex: str,
) -> dict[str, Any]:
    """Find the first native shader object and decode its three 10-byte maps."""

    reader: LostArkPackageRangeReader = package["reader"]
    names: list[str] = package["names"]
    descriptor = code_index["descriptorById"].get(shader_id_hex)
    require(descriptor is not None, "shader-object descriptor is absent")
    target = next(
        (row for row in TARGETS if row["expectedPixelShaderIdHex"] == shader_id_hex),
        None,
    )
    require(target is not None, "shader-object target is outside the focused denominator")
    expected_object = target[
        "expectedOfficialShaderObject"
        if package["identity"]["rawSha256"] == EXPECTED_OFFICIAL_CACHE["sha256"]
        else "expectedInstalledShaderObject"
    ]
    shader_type = descriptor["shaderType"]
    type_indices = [index for index, name in enumerate(names) if name == shader_type]
    require(len(type_indices) == 1, "shader-object type NameTable index is ambiguous")
    pattern = struct.pack("<ii", type_indices[0], 0) + bytes.fromhex(shader_id_hex)
    start = code_index["shaderCodeSectionEndLogicalOffset"] + 8
    candidates: list[int] = []
    overlap = b""
    cursor = start
    while cursor < reader.logical_size:
        payload = reader.read_logical_range(
            cursor, min(8 * 1024 * 1024, reader.logical_size - cursor)
        )
        combined = overlap + payload
        origin = cursor - len(overlap)
        local = 0
        while True:
            found = combined.find(pattern, local)
            if found < 0:
                break
            absolute = origin + found
            if absolute >= start:
                candidates.append(absolute)
            local = found + 1
        overlap = combined[-(len(pattern) - 1) :]
        cursor += len(payload)

    valid = []
    for absolute in sorted(set(candidates)):
        payload = reader.read_logical_range(
            absolute, min(2048, reader.logical_size - absolute)
        )
        offset = 188
        arrays: dict[str, list[dict[str, int]]] = {}
        try:
            expected_size = int(target["expectedShaderObjectByteSize"])
            require(len(payload) >= expected_size, "shader object is truncated")
            require(absolute == expected_object["logicalOffset"], "shader-object logical offset changed")
            object_bytes = payload[:expected_size]
            require(sha256_bytes(object_bytes) == expected_object["rawSha256"], "shader-object raw SHA changed")
            require(sha256_bytes(object_bytes[:188]) == expected_object["prefix188Sha256"], "shader-object prefix SHA changed")
            require(
                struct.unpack_from("<I", payload, 44)[0] == absolute + expected_size,
                "shader-object absolute end pointer changed",
            )
            require(
                payload[24:44] == bytes.fromhex("8925513f6e7ff806f9b013b9ce9813eea9c62aab"),
                "shader-object shared preamble identity changed",
            )
            for array_name in ("scalarGroups", "vectors", "textures"):
                require(offset + 4 <= len(payload), "shader binding count is truncated")
                count = struct.unpack_from("<I", payload, offset)[0]
                offset += 4
                require(count <= 128, "shader binding count is invalid")
                rows = []
                for expression_index in range(count):
                    require(offset + 10 <= len(payload), "shader binding row is truncated")
                    wire_index, base_index, count_or_size, buffer_or_sampler = struct.unpack_from(
                        "<IHHH", payload, offset
                    )
                    rows.append(
                        {
                            "expressionIndexOrGroup": wire_index,
                            "baseIndex": base_index,
                            "numBytesOrResources": count_or_size,
                            "bufferIndexOrSamplerIndex": buffer_or_sampler,
                            "logicalOffset": absolute + offset,
                        }
                    )
                    offset += 10
                arrays[array_name] = rows
            require(arrays["scalarGroups"] and arrays["vectors"] and arrays["textures"], "shader binding arrays are empty")
            require(
                [len(arrays[name]) for name in ("scalarGroups", "vectors", "textures")]
                == target["expectedBindingCounts"],
                "shader binding array denominator changed",
            )
            require(all(row["numBytesOrResources"] == 16 and row["bufferIndexOrSamplerIndex"] == 0 for row in arrays["scalarGroups"]), "scalar binding wire shape changed")
            require(all(row["numBytesOrResources"] == 16 and row["bufferIndexOrSamplerIndex"] == 0 for row in arrays["vectors"]), "vector binding wire shape changed")
            require(all(row["numBytesOrResources"] == 1 for row in arrays["textures"]), "texture binding wire shape changed")
            for array_name in ("scalarGroups", "vectors", "textures"):
                keys = [row["expressionIndexOrGroup"] for row in arrays[array_name]]
                require(len(keys) == len(set(keys)), f"{array_name} binding key is duplicated")
            require(offset <= expected_size, "shader binding arrays exceed shader object")
        except (ValueError, struct.error):
            continue
        valid.append(
            {
                "logicalOffset": absolute,
                "shaderObjectByteSize": expected_size,
                "shaderObjectEndLogicalOffset": absolute + expected_size,
                "shaderObjectRawSha256": sha256_bytes(object_bytes),
                "shaderObjectPrefix188Sha256": sha256_bytes(object_bytes[:188]),
                "sharedPreamble20ByteHex": payload[24:44].hex(),
                "sharedPreamble20ByteSha256": sha256_bytes(payload[24:44]),
                "bindingArraysOffsetInShaderObject": 188,
                "bindingArraysByteSize": offset - 188,
                "bindingArraysRawSha256": sha256_bytes(payload[188:offset]),
                **arrays,
            }
        )
    require(len(valid) == 1, "native shader-object binding candidate is absent or ambiguous")
    require(
        valid[0]["bindingArraysRawSha256"] == target["expectedBindingArraysSha256"],
        "native shader-object binding-array SHA changed",
    )
    return valid[0]


def scan_static_parameter_sets(
    package: dict[str, Any],
    code_index: dict[str, Any],
    identity_targets: dict[str, dict[str, Any]],
) -> dict[str, dict[str, Any]]:
    reader: LostArkPackageRangeReader = package["reader"]
    names: list[str] = package["names"]
    patterns = {
        family_id: bytes.fromhex(target["baseMaterialIdHex"])
        for family_id, target in identity_targets.items()
    }
    hits = {family_id: [] for family_id in patterns}
    start = code_index["shaderCodeSectionEndLogicalOffset"] + 8
    overlap = b""
    cursor = start
    chunk_size = 8 * 1024 * 1024
    while cursor < reader.logical_size:
        payload = reader.read_logical_range(
            cursor, min(chunk_size, reader.logical_size - cursor)
        )
        combined = overlap + payload
        origin = cursor - len(overlap)
        for family_id, pattern in patterns.items():
            local = 0
            while True:
                found = combined.find(pattern, local)
                if found < 0:
                    break
                absolute = origin + found
                if absolute >= start:
                    hits[family_id].append(absolute)
                local = found + 1
        overlap = combined[-15:]
        cursor += len(payload)

    result: dict[str, dict[str, Any]] = {}
    for family_id, offsets in hits.items():
        target = identity_targets[family_id]
        exact_rows = []
        for absolute in sorted(set(offsets)):
            candidate = reader.read_logical_range(
                absolute, min(2048, reader.logical_size - absolute)
            )
            try:
                static_set = parse_static_parameter_set(candidate, 0, names)
                normalized = normalized_static_parameter_set(static_set)
                engine_equality = engine_equivalent_static_parameter_set(static_set)
            except (ValueError, struct.error):
                continue
            normalized_sha = canonical_json_sha256(normalized)
            engine_equality_sha = canonical_json_sha256(engine_equality)
            if engine_equality_sha != target["engineEqualityStaticParameterSetSha256"]:
                continue
            suffix = None
            if static_set["endOffset"] + 20 <= len(candidate):
                suffix = list(struct.unpack_from("<IIIII", candidate, static_set["endOffset"]))
            exact_rows.append(
                {
                    "logicalOffset": absolute,
                    "byteSize": static_set["byteSize"],
                    "rawSha256": static_set["rawSha256"],
                    "normalizedSha256": normalized_sha,
                    "engineEqualitySha256": engine_equality_sha,
                    "suffixU32": suffix,
                    "mapContext": bool(
                        suffix
                        and suffix[0] == 868
                        and suffix[1] == 16
                        and suffix[3] == 0
                        and suffix[4] == target["expectedVfCount"]
                        and suffix[2] > absolute
                        and suffix[2] <= reader.logical_size
                    ),
                }
            )
        map_rows = [row for row in exact_rows if row["mapContext"]]
        require(len(exact_rows) == 2, f"{family_id} exact static-set occurrence denominator changed")
        require(len(map_rows) == 1, f"{family_id} exact material-map context is ambiguous")
        result[family_id] = {
            "baseMaterialIdRawHitCount": len(set(offsets)),
            "engineEquivalentStaticSetOccurrenceCount": len(exact_rows),
            "normalizedExactStaticSetOccurrenceCount": len(exact_rows),
            "exactMapContextCount": len(map_rows),
            "exactRows": exact_rows,
            "mapRow": map_rows[0],
        }
    return result


def parse_material_map(
    package: dict[str, Any],
    code_index: dict[str, Any],
    target: dict[str, Any],
    search: dict[str, Any],
    disassembler: D3DDisassembler,
) -> dict[str, Any]:
    reader: LostArkPackageRangeReader = package["reader"]
    names: list[str] = package["names"]
    start = search["mapRow"]["logicalOffset"]
    end = search["mapRow"]["suffixU32"][2]
    require(end > start, "material-map range is invalid")
    data = reader.read_logical_range(start, end - start)
    static_set = parse_static_parameter_set(data, 0, names)
    normalized = normalized_static_parameter_set(static_set)
    engine_equality = engine_equivalent_static_parameter_set(static_set)
    require(
        canonical_json_sha256(normalized)
        == target["normalizedStaticParameterSetSha256"],
        "material-map normalized static set changed",
    )
    require(
        canonical_json_sha256(engine_equality)
        == target["engineEqualityStaticParameterSetSha256"],
        "material-map engine-equality static set changed",
    )
    offset = static_set["endOffset"]
    suffix = list(struct.unpack_from("<IIIII", data, offset))
    offset += 20
    require(suffix == [868, 16, end, 0, target["expectedVfCount"]], "material-map suffix changed")

    vertex_factories = []
    all_shader_ids = []
    for vf_index in range(suffix[4]):
        require(offset + 4 <= len(data), "VF shader-reference count is truncated")
        count = struct.unpack_from("<I", data, offset)[0]
        offset += 4
        require(0 < count <= 256, "VF shader-reference count is invalid")
        references = []
        for _ in range(count):
            shader_type, number, _ = read_fname_at(data, offset, names)
            require(number == 0, "numbered shader reference is unsupported")
            shader_id = data[offset + 8 : offset + 24].hex()
            repeated_type, repeated_number, _ = read_fname_at(data, offset + 24, names)
            require(repeated_number == 0 and repeated_type == shader_type, "shader-reference type repeat changed")
            require(shader_id in code_index["descriptorById"], "material map references an unknown shader ID")
            descriptor = code_index["descriptorById"][shader_id]
            require(descriptor["shaderType"] == shader_type, "descriptor shader type differs from map reference")
            references.append(
                {
                    "shaderType": shader_type,
                    "shaderIdHex": shader_id,
                    "descriptor": {
                        key: value
                        for key, value in descriptor.items()
                        if key != "codePosition"
                    },
                }
            )
            all_shader_ids.append(shader_id)
            offset += 32
        vertex_factory, number, offset = read_fname_at(data, offset, names)
        require(number == 0, "numbered vertex-factory type is unsupported")
        vertex_factories.append(
            {
                "vertexFactoryIndex": vf_index,
                "vertexFactoryType": vertex_factory,
                "shaderReferenceCount": count,
                "shaderReferences": references,
            }
        )

    require(offset + 16 <= len(data), "material-map opaque identity is truncated")
    opaque_identity = data[offset : offset + 16]
    offset += 16
    friendly_name, offset = read_fstring_at(data, offset)
    require(friendly_name == target["expectedFriendlyName"], "material-map friendly name changed")
    repeated_set = parse_static_parameter_set(data, offset, names)
    repeated_normalized = normalized_static_parameter_set(repeated_set)
    repeated_engine_equality = engine_equivalent_static_parameter_set(repeated_set)
    require(repeated_set["rawSha256"] == static_set["rawSha256"], "repeated static set raw bytes changed")
    require(repeated_normalized == normalized, "repeated static set semantics changed")
    require(repeated_engine_equality == engine_equality, "repeated engine-equality static set changed")
    offset = repeated_set["endOffset"]
    uniform_set = parse_uniform_expression_set(data, offset, names)
    offset = uniform_set["endOffset"]
    require(offset + 4 == len(data), "material-map post-uniform trailer size changed")
    trailer = data[offset:]
    trailer_platform = struct.unpack_from("<I", trailer, 0)[0]
    require(trailer_platform == code_index["platform"], "material-map trailer platform changed")
    counts = [
        len(uniform_set[name])
        for name in (
            "pixelVectorExpressions",
            "pixelScalarExpressions",
            "pixelTexture2DExpressions",
            "textureCubeExpressions",
            "vertexVectorExpressions",
            "vertexScalarExpressions",
            "vertexTexture2DExpressions",
            "hullVectorExpressions",
            "hullScalarExpressions",
            "hullTexture2DExpressions",
            "domainVectorExpressions",
            "domainScalarExpressions",
            "domainTexture2DExpressions",
        )
    ]
    require(counts == target["expectedUniformCounts"] + [0] * 9, "uniform-expression denominator changed")

    local_rows = [
        row for row in vertex_factories if row["vertexFactoryType"] == LOCAL_VF
    ]
    require(len(local_rows) == 1, "LocalVF reference group is absent or ambiguous")
    selected_types = {BASE_PASS_PIXEL, *BASE_PASS_VERTEX_TYPES}
    selected_references = [
        row
        for row in local_rows[0]["shaderReferences"]
        if row["shaderType"] in selected_types
    ]
    require(len(selected_references) == 5, "LocalVF BasePass shader denominator changed")
    dxbc_rows = []
    for reference in selected_references:
        descriptor = code_index["descriptorById"][reference["shaderIdHex"]]
        position = descriptor["codePosition"]
        require(position is not None, "selected shader descriptor has no embedded code mapping")
        compressed = reader.read_logical_range(
            position["compressedLogicalOffset"], position["compressedByteSize"]
        )
        bytecode = decompress_lz4_block(compressed, position["uncompressedByteSize"])
        container = validate_dxbc_container(bytecode)
        disassembly = disassembler.disassemble(bytecode)
        expected = (
            target["expectedPixelDxbcSha256"]
            if reference["shaderType"] == BASE_PASS_PIXEL
            else EXPECTED_SHARED_VERTEX_DXBC[reference["shaderType"]]
        )
        require(container["sha256"] == expected, "selected original DXBC SHA changed")
        chunk_four_cc = [row["fourCc"] for row in container["chunks"]]
        rdef_present = "RDEF" in chunk_four_cc
        if reference["shaderType"] == BASE_PASS_PIXEL:
            require(chunk_four_cc == ["ISGN", "OSGN", "SHEX"], "pixel DXBC chunk set changed")
            require(not rdef_present, "pixel DXBC unexpectedly acquired reflection symbols")
        dxbc_rows.append(
            {
                "shaderType": reference["shaderType"],
                "shaderIdHex": reference["shaderIdHex"],
                "compressedLogicalOffset": position["compressedLogicalOffset"],
                "compressedByteSize": position["compressedByteSize"],
                "compressedSha256": sha256_bytes(compressed),
                "uncompressedByteSize": len(bytecode),
                "dxbcSha256": container["sha256"],
                "dxbcHex": bytecode.hex(),
                "container": container,
                "rdefChunkPresent": rdef_present,
                "reflectionNameAdmission": False,
                "disassembly": disassembly,
            }
        )

    semantic_map = {
        "staticParameterSet": normalized,
        "engineEqualityStaticParameterSet": engine_equality,
        "suffixVersion": suffix[:2],
        "vertexFactories": [
            {
                "vertexFactoryType": row["vertexFactoryType"],
                "shaderReferences": [
                    {
                        "shaderType": reference["shaderType"],
                        "shaderIdHex": reference["shaderIdHex"],
                    }
                    for reference in row["shaderReferences"]
                ],
            }
            for row in vertex_factories
        ],
        "opaqueIdentityHex": opaque_identity.hex(),
        "friendlyName": friendly_name,
        "uniformExpressionSemanticSha256": uniform_set["semanticSha256"],
        "trailerPlatform": trailer_platform,
    }
    public_static_set = dict(static_set)
    public_static_set.pop("endOffset", None)
    public_repeated_set = dict(repeated_set)
    public_repeated_set.pop("endOffset", None)
    public_uniform_set = dict(uniform_set)
    public_uniform_set.pop("endOffset", None)
    return {
        "logicalOffset": start,
        "logicalEndOffset": end,
        "byteSize": len(data),
        "rawSha256": sha256_bytes(data),
        "staticParameterSet": public_static_set,
        "normalizedStaticParameterSet": normalized,
        "engineEqualityStaticParameterSet": engine_equality,
        "suffix": {
            "packageVersion": suffix[0],
            "licenseeVersion": suffix[1],
            "serializedMapEndLogicalOffset": suffix[2],
            "reserved": suffix[3],
            "vertexFactoryCount": suffix[4],
        },
        "vertexFactories": vertex_factories,
        "opaqueIdentityHex": opaque_identity.hex(),
        "friendlyName": friendly_name,
        "repeatedStaticParameterSet": public_repeated_set,
        "uniformExpressionSet": public_uniform_set,
        "postUniformTrailer": {
            "reservedZeroByteCount": 0,
            "shaderPlatformOrdinal": trailer_platform,
            "rawSha256": sha256_bytes(trailer),
        },
        "meshParticlePassSelection": {
            "sourceOccurrenceRendererType": "MeshParticle",
            "selectedVertexFactory": LOCAL_VF,
            "selectedPass": "BASE_PASS_NO_LIGHTMAP_SKYLIGHT",
            "selectedShaderCount": len(dxbc_rows),
            "selectionFidelity": "STRUCTURAL_MAP_REFERENCE_PLUS_SOURCE_MESHPARTICLE_RENDERER;_MESH_EMITTER_TO_LOCALVF_NATIVE_ABI_CORROBORATION_REQUIRED",
        },
        "selectedOriginalDxbc": dxbc_rows,
        "semanticMapSha256": canonical_json_sha256(semantic_map),
    }


def resolve_texture_registers(
    target: dict[str, Any],
    material_map: dict[str, Any],
    shader_binding: dict[str, Any],
    material_contract: dict[str, Any],
    native_receipt: dict[str, Any],
) -> list[dict[str, Any]]:
    recipe = next(
        row
        for row in material_contract["materialRecipes"]
        if row["recipeId"] == target["recipeId"]
    )
    family = next(
        row
        for row in native_receipt["families"]
        if row["familyId"] == target["familyId"]
    )
    overrides = {
        str(row["normalizedParameterName"]).casefold(): row
        for row in recipe["inputs"]["textureOverrides"]
    }
    referenced = {
        int(row["index"]): row for row in family["source"]["referencedTextures"]
    }
    pixel = next(
        row
        for row in material_map["selectedOriginalDxbc"]
        if row["shaderType"] == BASE_PASS_PIXEL
    )
    texture_count = len(material_map["uniformExpressionSet"]["pixelTexture2DExpressions"])
    declared_textures = {
        int(match.group(1))
        for line in pixel["disassembly"]["declarations"]
        for match in [re.search(r"\bt(\d+)\b", line)]
        if match
    }
    declared_samplers = {
        int(match.group(1))
        for line in pixel["disassembly"]["declarations"]
        for match in [re.search(r"\bs(\d+)\b", line)]
        if match
    }
    texture_wire = {
        row["expressionIndexOrGroup"]: row for row in shader_binding["textures"]
    }
    require(set(texture_wire) == set(range(texture_count)), "texture expression binding denominator changed")
    declared_from_wire = {
        register
        for row in texture_wire.values()
        for register in range(row["baseIndex"], row["baseIndex"] + row["numBytesOrResources"])
    }
    require(declared_textures == declared_from_wire, "DXBC declarations differ from explicit texture bindings")
    declared_samplers_from_wire = {
        int(row["bufferIndexOrSamplerIndex"]) for row in texture_wire.values()
    }
    require(declared_samplers == declared_samplers_from_wire, "DXBC sampler declarations differ from explicit texture bindings")
    sampler_uses: dict[int, set[str]] = {index: set() for index in declared_textures}
    observed_pair_counts: dict[str, int] = {}
    for sample in pixel["disassembly"]["sampleInstructions"]:
        texture_index = int(sample["textureRegister"][1:])
        require(texture_index in sampler_uses, "DXBC sample references an undeclared texture")
        sampler_uses[texture_index].add(sample["samplerRegister"])
        pair = f"{sample['textureRegister']}/{sample['samplerRegister']}"
        observed_pair_counts[pair] = observed_pair_counts.get(pair, 0) + 1
    require(observed_pair_counts == target["expectedSamplePairs"], "DXBC texture/sampler sample pairs changed")
    rows = []
    for expression_index, expression in enumerate(
        material_map["uniformExpressionSet"]["pixelTexture2DExpressions"]
    ):
        binding = texture_wire[expression_index]
        texture_index = binding["baseIndex"]
        sampler_index = binding["bufferIndexOrSamplerIndex"]
        require(
            sampler_uses[texture_index] == {f"s{sampler_index}"},
            "DXBC sample sampler differs from native texture binding",
        )
        fallback_index = int(expression["referencedTextureIndex"])
        require(fallback_index in referenced, "uniform texture fallback is outside ReferencedTextures")
        fallback = referenced[fallback_index]
        parameter_name = expression.get("parameterName")
        override = overrides.get(str(parameter_name).casefold()) if parameter_name else None
        effective_path = override["value"] if override else fallback["objectPath"]
        rows.append(
            {
                "uniformTexture2DExpressionIndex": expression_index,
                "textureRegister": f"t{texture_index}",
                "samplerRegister": f"s{sampler_index}",
                "samplerRegistersObserved": sorted(sampler_uses[texture_index]),
                "expressionType": expression["typeName"],
                "parameterName": parameter_name,
                "fallbackReferencedTextureIndex": fallback_index,
                "fallbackSourceObjectPath": fallback["objectPath"],
                "activeMicOverrideApplied": override is not None,
                "effectiveSourceObjectPath": effective_path,
                "bindingFidelity": "SOURCE_EXACT_UNIFORM_EXPRESSION_ORDER_PLUS_ACTIVE_MIC_OVERRIDE",
            }
        )
    return rows


def resolve_constant_buffer_bindings(
    material_map: dict[str, Any], shader_binding: dict[str, Any]
) -> dict[str, Any]:
    uniform = material_map["uniformExpressionSet"]
    pixel = next(
        row
        for row in material_map["selectedOriginalDxbc"]
        if row["shaderType"] == BASE_PASS_PIXEL
    )
    vector_count = len(uniform["pixelVectorExpressions"])
    scalar_count = len(uniform["pixelScalarExpressions"])
    vector_rows = []
    vector_wire = {
        row["expressionIndexOrGroup"]: row for row in shader_binding["vectors"]
    }
    require(set(vector_wire) == set(range(vector_count)), "vector binding denominator changed")
    for expression_index in range(vector_count):
        wire = vector_wire[expression_index]
        require(wire["baseIndex"] % 16 == 0, "vector CB byte offset is unaligned")
        vector_rows.append(
            {
                "expressionIndex": expression_index,
                "constantBufferIndex": wire["bufferIndexOrSamplerIndex"],
                "constantBufferSlot": wire["baseIndex"] // 16,
                "componentMask": "xyzw",
                "byteSize": wire["numBytesOrResources"],
            }
        )

    scalar_wire = {
        row["expressionIndexOrGroup"]: row for row in shader_binding["scalarGroups"]
    }
    require(len(scalar_wire) == len(shader_binding["scalarGroups"]), "scalar binding key is duplicated")
    scalar_rows = []
    unbound_groups = []
    for group in range(math.ceil(scalar_count / 4)):
        wire = scalar_wire.get(group)
        if wire is None:
            unbound_groups.append(group)
            continue
        require(wire["baseIndex"] % 16 == 0, "scalar CB byte offset is unaligned")
        for component in range(4):
            expression_index = group * 4 + component
            if expression_index >= scalar_count:
                break
            scalar_rows.append(
                {
                    "expressionIndex": expression_index,
                    "packedScalarGroup": group,
                    "constantBufferIndex": wire["bufferIndexOrSamplerIndex"],
                    "constantBufferSlot": wire["baseIndex"] // 16,
                    "component": "xyzw"[component],
                }
            )
    bound_slots = [
        row["constantBufferSlot"] for row in vector_rows + scalar_rows
    ]
    cb0_declarations = []
    for line in pixel["disassembly"]["declarations"]:
        match = re.search(r"\bcb0\[(\d+)\]", line, re.IGNORECASE)
        if match:
            cb0_declarations.append(int(match.group(1)))
    require(len(cb0_declarations) == 1, "DXBC CB0 declaration is absent or ambiguous")
    require(bound_slots and cb0_declarations[0] == max(bound_slots) + 1, "DXBC CB0 declaration differs from native bindings")
    return {
        "wireEntryFormat": "<u32 expressionIndexOrPackedScalarGroup,u16 baseByteOrResourceIndex,u16 numBytesOrResources,u16 bufferOrSamplerIndex>",
        "scalarPackingRule": "group=expressionIndex//4;component=xyzw[expressionIndex%4];missing group is optimized/unbound",
        "vectorBindings": vector_rows,
        "scalarBindings": scalar_rows,
        "unboundPackedScalarGroups": unbound_groups,
        "declaredConstantBuffer0Float4Count": cb0_declarations[0],
        "maximumNativeBoundConstantBuffer0Slot": max(bound_slots),
        "bindingFidelity": "OFFICIAL_AND_CURRENT_EXACT_SHADER_OBJECT_WIRE_AND_DXBC_DECLARATION_EQUALITY;_SYMBOLIC_PARAMETER_NAMES_FROM_UNIFORM_EXPRESSIONS_ONLY",
    }


def source_mesh_occurrence_join(
    source_inventory_path: Path,
    material_contract: dict[str, Any],
) -> dict[str, Any]:
    require(source_inventory_path.is_file(), "source-active inventory is missing")
    require(
        digest_file(source_inventory_path) == EXPECTED_SOURCE_INVENTORY_SHA256,
        "source-active inventory SHA changed",
    )
    source_inventory = read_json(source_inventory_path)
    require(
        source_inventory.get("schema") == "lostark.source-active-effect-inventory-receipt",
        "source-active inventory schema changed",
    )
    source_by_id = {
        row["activeElementId"]: row for row in source_inventory["activeElements"]
    }
    contract_by_id = {
        row["occurrenceId"]: row for row in material_contract["occurrences"]
    }
    rows = []
    for occurrence_id, expected in EXPECTED_MESH_OCCURRENCES.items():
        source = source_by_id.get(occurrence_id)
        contract = contract_by_id.get(occurrence_id)
        require(source is not None and contract is not None, "main source occurrence is absent")
        require(
            source["rendererType"] == contract["rendererType"] == "MeshParticle",
            "main source occurrence renderer changed",
        )
        require(source["sourceEmitter"] == expected["emitter"], "main source emitter changed")
        require(
            source["sourceMaterials"] == [expected["material"]]
            and contract["sourceMaterialPath"] == expected["material"],
            "main source material join changed",
        )
        type_data_rows = [
            row
            for row in source["moduleEvidence"]
            if row["className"].casefold() == "particlemoduletypedatamesh"
        ]
        require(len(type_data_rows) == 1, "main mesh type-data row is absent or ambiguous")
        type_data = type_data_rows[0]
        require(type_data["nodeId"] == expected["typeDataNodeId"], "main mesh type-data node changed")
        require(type_data["recordSha256"] == expected["typeDataRecordSha256"], "main mesh type-data SHA changed")
        rows.append(
            {
                "occurrenceId": occurrence_id,
                "rendererType": source["rendererType"],
                "sourceEmitter": source["sourceEmitter"],
                "sourceMaterialPath": expected["material"],
                "materialRecipeId": contract["materialRecipeId"],
                "typeDataMesh": {
                    "nodeId": type_data["nodeId"],
                    "recordSha256": type_data["recordSha256"],
                    "objectPath": type_data["objectPath"],
                },
            }
        )
    return {
        "repoRelativePath": source_inventory_path.relative_to(REPO_ROOT).as_posix(),
        "rawSha256": digest_file(source_inventory_path),
        "occurrenceCount": len(rows),
        "occurrences": rows,
        "rendererAdmission": "SOURCE_EXACT_MESH_PARTICLE_AND_TYPEDATAMESH",
        "localVertexFactorySelectionAdmission": False,
        "localVertexFactorySelectionBoundary": "OFFICIAL_EFENGINE_EXPORT_COHORT_CORROBORATION_PRESENT_BUT_NATIVE_CALL_CHAIN_NOT_YET_DISASSEMBLED",
    }


def manifest_rows(
    manifest_path: Path,
    archive_path: Path,
    official_cache_path: Path,
    official_efengine_path: Path,
    official_efengine_archive_path: Path,
    installed_root: Path,
    identity_receipt: dict[str, Any],
) -> dict[str, Any]:
    require(manifest_path.is_file(), f"official manifest is missing: {manifest_path}")
    require(manifest_path.name == EXPECTED_MANIFEST["fileName"], "manifest name changed")
    require(manifest_path.stat().st_size == EXPECTED_MANIFEST["byteSize"], "manifest size changed")
    require(digest_file(manifest_path) == EXPECTED_MANIFEST["rawSha256"], "manifest SHA changed")
    manifest = read_json(manifest_path)
    require(str(manifest.get("service_code")) == EXPECTED_MANIFEST["serviceCode"], "manifest service changed")
    require(manifest.get("version_no") == EXPECTED_MANIFEST["versionNo"], "manifest version changed")
    strings: list[str] = []

    def collect(value: Any) -> None:
        if isinstance(value, str):
            strings.append(value)
        elif isinstance(value, dict):
            for item in value.values():
                collect(item)
        elif isinstance(value, list):
            for item in value:
                collect(item)

    collect(manifest)
    wanted = {
        "EV2LG3OVEH3HGV7THTFFTM7TOKMCC.upk": None,
        "EFEngine.dll": None,
        "YGI3SB3OBJ3O18GUMP6QMP8L5.upk": None,
        "YGI3SB3OBJ3O1MGUMP6QMP8B5.upk": None,
        "ZHJ4TC4PCK4PC4J22HIXEYUXEU.upk": None,
    }
    for value in strings:
        for leaf in wanted:
            if f"/{leaf}" in value:
                require(wanted[leaf] is None, f"manifest row is duplicated: {leaf}")
                wanted[leaf] = value
    require(all(value is not None for value in wanted.values()), "manifest target row is missing")

    parsed_rows = []
    for leaf, raw in wanted.items():
        assert raw is not None
        fields = [field.strip() for field in raw.split("|")]
        require(len(fields) >= 11, "manifest target row shape changed")
        path = fields[2]
        extracted_size = int(fields[6])
        packed_size = int(fields[7])
        extracted_md5 = fields[8].casefold()
        packed_md5 = fields[9].casefold()
        parsed_rows.append(
            {
                "leaf": leaf,
                "rawRow": raw,
                "rowSha256": sha256_bytes(raw.encode("utf-8")),
                "path": path,
                "fileVersion": int(fields[3]),
                "sequence": int(fields[4]),
                "payloadKind": fields[5],
                "extractedByteSize": extracted_size,
                "packedByteSize": packed_size,
                "extractedMd5": extracted_md5,
                "packedMd5": packed_md5,
            }
        )

    archive_row = next(row for row in parsed_rows if row["leaf"].startswith("EV2"))
    efengine_row = next(row for row in parsed_rows if row["leaf"] == "EFEngine.dll")
    require(archive_path.is_file(), f"official cache archive is missing: {archive_path}")
    require(archive_path.name == EXPECTED_OFFICIAL_ARCHIVE["fileName"], "official archive name changed")
    require(archive_path.stat().st_size == archive_row["packedByteSize"] == EXPECTED_OFFICIAL_ARCHIVE["byteSize"], "official archive size changed")
    require(digest_file(archive_path, "md5") == archive_row["packedMd5"] == EXPECTED_OFFICIAL_ARCHIVE["md5"], "official archive MD5 changed")
    require(digest_file(archive_path) == EXPECTED_OFFICIAL_ARCHIVE["sha256"], "official archive SHA changed")
    require(official_cache_path.stat().st_size == archive_row["extractedByteSize"] == EXPECTED_OFFICIAL_CACHE["byteSize"], "official cache size differs from manifest")
    require(digest_file(official_cache_path, "md5") == archive_row["extractedMd5"] == EXPECTED_OFFICIAL_CACHE["md5"], "official cache MD5 differs from manifest")
    require(
        efengine_row["fileVersion"] == EXPECTED_OFFICIAL_EFENGINE["manifestFileVersion"]
        and efengine_row["sequence"] == EXPECTED_OFFICIAL_EFENGINE["manifestSequence"],
        "official EFEngine manifest revision changed",
    )
    require(official_efengine_path.is_file(), "official EFEngine evidence is missing")
    require(official_efengine_path.name == EXPECTED_OFFICIAL_EFENGINE["fileName"], "official EFEngine evidence name changed")
    require(official_efengine_path.stat().st_size == efengine_row["extractedByteSize"] == EXPECTED_OFFICIAL_EFENGINE["byteSize"], "official EFEngine size changed")
    require(digest_file(official_efengine_path, "md5") == efengine_row["extractedMd5"] == EXPECTED_OFFICIAL_EFENGINE["md5"], "official EFEngine MD5 changed")
    require(digest_file(official_efengine_path) == EXPECTED_OFFICIAL_EFENGINE["sha256"], "official EFEngine SHA changed")
    require(official_efengine_archive_path.is_file(), "official EFEngine archive is missing")
    require(official_efengine_archive_path.name == EXPECTED_OFFICIAL_EFENGINE["archiveFileName"], "official EFEngine archive name changed")
    require(official_efengine_archive_path.stat().st_size == efengine_row["packedByteSize"] == EXPECTED_OFFICIAL_EFENGINE["archiveByteSize"], "official EFEngine archive size changed")
    require(digest_file(official_efengine_archive_path, "md5") == efengine_row["packedMd5"] == EXPECTED_OFFICIAL_EFENGINE["archiveMd5"], "official EFEngine archive MD5 changed")
    require(digest_file(official_efengine_archive_path) == EXPECTED_OFFICIAL_EFENGINE["archiveSha256"], "official EFEngine archive SHA changed")
    exports = parse_pe_exports(official_efengine_path)
    abi_exports = []
    for role, (decorated_name, expected_rva) in OFFICIAL_ABI_EXPORTS.items():
        require(exports.get(decorated_name) == expected_rva, f"official EFEngine ABI export changed: {role}")
        abi_exports.append({"role": role, "decoratedName": decorated_name, "rva": expected_rva})
    signature_script = (
        "$s=Get-AuthenticodeSignature -LiteralPath $env:ARTIST31470_OFFICIAL_EFENGINE;"
        "$v=[Diagnostics.FileVersionInfo]::GetVersionInfo($env:ARTIST31470_OFFICIAL_EFENGINE);"
        "[pscustomobject]@{status=[string]$s.Status;subject=[string]$s.SignerCertificate.Subject;"
        "thumbprint=[string]$s.SignerCertificate.Thumbprint;fileVersion=[string]$v.FileVersion}|ConvertTo-Json -Compress"
    )
    environment = dict(os.environ)
    environment["ARTIST31470_OFFICIAL_EFENGINE"] = str(official_efengine_path)
    signature = json.loads(subprocess.run(
        ["powershell", "-NoProfile", "-Command", signature_script],
        check=True, capture_output=True, text=True, encoding="utf-8", env=environment,
    ).stdout)
    require(signature["status"] == "Valid", "official EFEngine Authenticode status changed")
    require(signature["thumbprint"] == "111E6CACF5BC32D5B50632411F523905284AEBCC", "official EFEngine signer changed")

    source_sha_by_leaf: dict[str, str] = {}
    for target in identity_receipt["mainTargets"]:
        for role in ("baseMaterial", "activeMic"):
            package = target[role]["package"]
            source_sha_by_leaf[package["fileName"]] = package["rawSha256"]
    local_rows = []
    for row in parsed_rows:
        if row["leaf"].startswith("EV2") or row["leaf"] == "EFEngine.dll":
            continue
        path = installed_root / "Packages" / row["leaf"]
        require(path.is_file(), f"co-resident source package is missing: {path}")
        require(path.stat().st_size == row["extractedByteSize"], "co-resident source package size changed")
        require(digest_file(path, "md5") == row["extractedMd5"], "co-resident source package MD5 changed")
        require(digest_file(path) == source_sha_by_leaf[row["leaf"]], "co-resident source package SHA differs from identity receipt")
        local_rows.append(
            {
                "leaf": row["leaf"],
                "physicalByteSize": path.stat().st_size,
                "rawMd5": digest_file(path, "md5"),
                "rawSha256": digest_file(path),
                "manifestFileVersion": row["fileVersion"],
                "manifestSequence": row["sequence"],
            }
        )
    return {
        "manifest": {
            "fileName": manifest_path.name,
            "physicalByteSize": manifest_path.stat().st_size,
            "rawSha256": digest_file(manifest_path),
            "serviceCode": str(manifest["service_code"]),
            "versionNo": manifest["version_no"],
        },
        "targetRows": parsed_rows,
        "downloadUrl": "http://la.cdn.stovegame.net/stove/live/game/dpms_45/v974/16.gz",
        "packedPayload": {
            "fileName": archive_path.name,
            "physicalByteSize": archive_path.stat().st_size,
            "rawMd5": digest_file(archive_path, "md5"),
            "rawSha256": digest_file(archive_path),
        },
        "officialEfEngine": {
            "fileName": official_efengine_path.name,
            "physicalByteSize": official_efengine_path.stat().st_size,
            "rawMd5": digest_file(official_efengine_path, "md5"),
            "rawSha256": digest_file(official_efengine_path),
            "packedFileName": official_efengine_archive_path.name,
            "packedByteSize": official_efengine_archive_path.stat().st_size,
            "packedMd5": digest_file(official_efengine_archive_path, "md5"),
            "packedSha256": digest_file(official_efengine_archive_path),
            "authenticode": signature,
            "abiExports": abi_exports,
        },
        "coResidentSourcePackages": local_rows,
        "cohortConclusion": "OFFICIAL_VENDOR_MANIFEST_975_CO_RESIDENT_MAIN_SOURCE_PACKAGES_AND_REFSHADERCACHE_PAYLOAD",
    }


def validate_cache_identity(identity: dict[str, Any], expected: dict[str, Any]) -> None:
    for actual_key, expected_key in (
        ("physicalByteSize", "byteSize"),
        ("rawSha256", "sha256"),
        ("logicalByteSize", "logicalByteSize"),
        ("nameCount", "nameCount"),
        ("importCount", "importCount"),
        ("exportCount", "exportCount"),
        ("serialOffset", "serialOffset"),
        ("serialSize", "serialSize"),
        ("packageGuidHex", "packageGuidHex"),
    ):
        require(identity[actual_key] == expected[expected_key], f"cache {actual_key} changed")
    require(identity["packageVersion"] == 868 and identity["licenseeVersion"] == 16, "cache UE version changed")
    require(identity["engineVersion"] == 12097 and identity["cookerVersion"] == 136, "cache engine/cooker version changed")


def extract_one_cache(
    path: Path,
    expected: dict[str, Any],
    identity_receipt: dict[str, Any],
    disassembler: D3DDisassembler,
) -> dict[str, Any]:
    package = package_tables(path)
    validate_cache_identity(package["identity"], expected)
    code_index = parse_cache_code_index(package)
    require(code_index["groupCount"] == expected["shaderTypeGroupCount"], "cache group count changed")
    require(code_index["descriptorCount"] == expected["descriptorCount"], "cache descriptor count changed")
    require(code_index["embeddedCodeCount"] == expected["embeddedCodeCount"], "cache embedded-code count changed")
    require(code_index["shaderCodeSectionEndLogicalOffset"] == expected["shaderCodeSectionEndLogicalOffset"], "cache code-section end changed")
    identities = {
        target["familyId"]: {
            **target,
            "staticParameterSet": next(
                row
                for row in identity_receipt["mainTargets"]
                if row["familyId"] == target["familyId"]
            )["derivedLookupIdentity"]["staticParameterSet"],
            "engineEqualityStaticParameterSet": next(
                row
                for row in identity_receipt["mainTargets"]
                if row["familyId"] == target["familyId"]
            )["derivedLookupIdentity"]["engineEqualityStaticParameterSet"],
        }
        for target in TARGETS
    }
    for target in identities.values():
        require(
            canonical_json_sha256(target["staticParameterSet"])
            == target["normalizedStaticParameterSetSha256"],
            "identity serialized-semantic static set changed",
        )
        require(
            canonical_json_sha256(target["engineEqualityStaticParameterSet"])
            == target["engineEqualityStaticParameterSetSha256"],
            "identity engine-equality static set changed",
        )
    search = scan_static_parameter_sets(package, code_index, identities)
    maps = []
    for target in TARGETS:
        material_map = parse_material_map(
            package, code_index, target, search[target["familyId"]], disassembler
        )
        shader_binding = find_shader_object_binding(
            package,
            code_index,
            target["expectedPixelShaderIdHex"],
        )
        expected_start_key = (
            "expectedOfficialMapStart"
            if expected is EXPECTED_OFFICIAL_CACHE
            else "expectedInstalledMapStart"
        )
        expected_end_key = (
            "expectedOfficialMapEnd"
            if expected is EXPECTED_OFFICIAL_CACHE
            else "expectedInstalledMapEnd"
        )
        require(material_map["logicalOffset"] == target[expected_start_key], "exact map start changed")
        require(material_map["logicalEndOffset"] == target[expected_end_key], "exact map end changed")
        maps.append(
            {
                "label": target["label"],
                "familyId": target["familyId"],
                "recipeId": target["recipeId"],
                "occurrenceIds": target["occurrenceIds"],
                "search": search[target["familyId"]],
                "materialMap": material_map,
                "pixelShaderObjectBinding": shader_binding,
                "constantBufferBindings": resolve_constant_buffer_bindings(
                    material_map, shader_binding
                ),
            }
        )
    public_index = dict(code_index)
    public_index.pop("descriptorById")
    return {
        "package": package["identity"],
        "codeIndex": public_index,
        "mainTargets": maps,
    }


def build_receipt(
    identity_receipt_path: Path,
    material_contract_path: Path,
    native_receipt_path: Path,
    source_inventory_path: Path,
    manifest_path: Path,
    archive_path: Path,
    official_cache_path: Path,
    official_efengine_path: Path,
    official_efengine_archive_path: Path,
    installed_cache_path: Path,
    installed_root: Path,
    d3dcompiler_path: Path,
) -> dict[str, Any]:
    identity_receipt = read_json(identity_receipt_path)
    material_contract = read_json(material_contract_path)
    native_receipt = read_json(native_receipt_path)
    require(identity_receipt["schema"] == "lostark.artist-31470-main-shader-map-identity-receipt", "identity receipt schema changed")
    require(material_contract["schema"] == "lostark.artist-31470-typed-material-evidence-contract", "material contract schema changed")
    require(native_receipt["schema"] == "lostark.artist-31470-material-native-resource-receipt", "native receipt schema changed")
    disassembler = D3DDisassembler(d3dcompiler_path)
    mesh_occurrence_join = source_mesh_occurrence_join(
        source_inventory_path, material_contract
    )
    cohort = manifest_rows(
        manifest_path,
        archive_path,
        official_cache_path,
        official_efengine_path,
        official_efengine_archive_path,
        installed_root,
        identity_receipt,
    )
    official = extract_one_cache(
        official_cache_path,
        EXPECTED_OFFICIAL_CACHE,
        identity_receipt,
        disassembler,
    )
    installed = extract_one_cache(
        installed_cache_path,
        EXPECTED_INSTALLED_CACHE,
        identity_receipt,
        disassembler,
    )
    for target in TARGETS:
        official_row = next(row for row in official["mainTargets"] if row["familyId"] == target["familyId"])
        installed_row = next(row for row in installed["mainTargets"] if row["familyId"] == target["familyId"])
        require(
            official_row["materialMap"]["semanticMapSha256"]
            == installed_row["materialMap"]["semanticMapSha256"],
            "official/current material-map semantics differ",
        )
        official_dxbc = {
            row["shaderType"]: row["dxbcSha256"]
            for row in official_row["materialMap"]["selectedOriginalDxbc"]
        }
        installed_dxbc = {
            row["shaderType"]: row["dxbcSha256"]
            for row in installed_row["materialMap"]["selectedOriginalDxbc"]
        }
        require(official_dxbc == installed_dxbc, "official/current LocalVF BasePass DXBC differs")
        official_row["textureRegisterBindings"] = resolve_texture_registers(
            target,
            official_row["materialMap"],
            official_row["pixelShaderObjectBinding"],
            material_contract,
            native_receipt,
        )
        installed_row["textureRegisterBindings"] = resolve_texture_registers(
            target,
            installed_row["materialMap"],
            installed_row["pixelShaderObjectBinding"],
            material_contract,
            native_receipt,
        )
        for row in (official_row, installed_row):
            require(
                [
                    [
                        binding["uniformTexture2DExpressionIndex"],
                        binding["textureRegister"],
                        binding["samplerRegister"],
                        binding["effectiveSourceObjectPath"],
                    ]
                    for binding in row["textureRegisterBindings"]
                ]
                == target["expectedTextureBindings"],
                "resolved main texture/register binding changed",
            )
            constant_bindings = row["constantBufferBindings"]
            require(
                constant_bindings["declaredConstantBuffer0Float4Count"]
                == target["expectedConstantBuffer0Float4Count"],
                "main CB0 declaration count changed",
            )
            require(
                constant_bindings["unboundPackedScalarGroups"]
                == target["expectedUnboundScalarGroups"],
                "main unbound scalar group set changed",
            )
        require(
            [
                {
                    key: value
                    for key, value in binding.items()
                    if key != "logicalOffset"
                }
                for array_name in ("scalarGroups", "vectors", "textures")
                for binding in official_row["pixelShaderObjectBinding"][array_name]
            ]
            == [
                {
                    key: value
                    for key, value in binding.items()
                    if key != "logicalOffset"
                }
                for array_name in ("scalarGroups", "vectors", "textures")
                for binding in installed_row["pixelShaderObjectBinding"][array_name]
            ],
            "official/current native shader-object bindings differ",
        )
        require(
            official_row["constantBufferBindings"]
            == installed_row["constantBufferBindings"],
            "official/current constant-buffer bindings differ",
        )
        require(
            [
                {
                    key: value
                    for key, value in row.items()
                    if key not in ("samplerRegistersObserved",)
                }
                for row in official_row["textureRegisterBindings"]
            ]
            == [
                {
                    key: value
                    for key, value in row.items()
                    if key not in ("samplerRegistersObserved",)
                }
                for row in installed_row["textureRegisterBindings"]
            ],
            "official/current texture expression bindings differ",
        )

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
        "inputReceipts": [
            {
                "repoRelativePath": identity_receipt_path.relative_to(REPO_ROOT).as_posix(),
                "receiptSha256": identity_receipt["receiptSha256"],
            },
            {
                "repoRelativePath": material_contract_path.relative_to(REPO_ROOT).as_posix(),
                "contractSha256": material_contract["contractSha256"],
            },
            {
                "repoRelativePath": native_receipt_path.relative_to(REPO_ROOT).as_posix(),
                "receiptSha256": native_receipt["receiptSha256"],
            },
        ],
        "officialDistributionCohort": cohort,
        "sourceMeshOccurrenceJoin": mesh_occurrence_join,
        "d3dDisassembler": disassembler.identity,
        "officialRefShaderCacheV974": official,
        "currentInstalledRefShaderCache": installed,
        "crossRevisionCorroboration": {
            "familyCount": 2,
            "semanticMaterialMapEqualityCount": 2,
            "localVfBasePassDxbcEqualityCount": 2,
            "conclusion": "OFFICIAL_V974_AND_CURRENT_INSTALLED_CACHE_HAVE_IDENTICAL_MAIN_MAP_SEMANTICS_AND_LOCALVF_BASEPASS_DXBC",
        },
        "decision": {
            "exactSameDistributionCohortShaderMapJoinCount": 2,
            "targetFamilyCount": 2,
            "targetOccurrenceCount": 3,
            "exactLocalVfBasePassPixelDxbcCount": 2,
            "exactLocalVfBasePassVertexPermutationCount": 4,
            "uniformExpressionSetParseCount": 2,
            "textureRegisterBindingCount": sum(
                len(row["textureRegisterBindings"])
                for row in official["mainTargets"]
            ),
            "constantBufferRegisterBindingAdmission": True,
            "nativeEngineEqualityKeyAdmission": True,
            "serializedSemanticProjectionIsNativeEqualityKey": False,
            "shaderObjectParameterNameAdmission": False,
            "dxbcReflectionNameAdmission": False,
            "sourceMeshParticleRendererAdmission": True,
            "meshParticleToLocalVfNativeCallChainAdmission": False,
            "sourceMeshOccurrenceJoinCount": 3,
            "selectedLocalVfDxbcReplayCandidateAdmission": True,
            "occurrenceSelectedLocalVfRuntimeAdmission": False,
            "fixedInputOriginalDxbcReplayAdmission": False,
            "runtimeHlslMutationAdmission": False,
            "visualProgressAdmission": False,
            "nextGate": "ORIGINAL_DXBC_FIXED_INPUT_WARP_REPLAY",
            "status": "EXACT_MAP_DXBC_UNIFORM_TEXTURE_CB_AND_CHANNEL_BINDINGS_ACQUIRED_NUMERIC_REPLAY_PENDING",
        },
    }
    seal(receipt)
    return receipt


def validate_receipt(receipt: dict[str, Any]) -> None:
    require(receipt.get("schema") == SCHEMA, "RefShaderCache receipt schema mismatch")
    require(receipt.get("formatVersion") == FORMAT_VERSION, "RefShaderCache receipt version mismatch")
    claimed = receipt.get("receiptSha256")
    unsigned = dict(receipt)
    unsigned.pop("receiptSha256", None)
    require(claimed == canonical_json_sha256(unsigned), "RefShaderCache receipt digest mismatch")
    decision = receipt["decision"]
    require(
        receipt["officialDistributionCohort"]["cohortConclusion"]
        == "OFFICIAL_VENDOR_MANIFEST_975_CO_RESIDENT_MAIN_SOURCE_PACKAGES_AND_REFSHADERCACHE_PAYLOAD",
        "official distribution cohort conclusion changed",
    )
    official_engine = receipt["officialDistributionCohort"]["officialEfEngine"]
    require(official_engine["rawSha256"] == EXPECTED_OFFICIAL_EFENGINE["sha256"], "official EFEngine identity changed")
    require(official_engine["authenticode"]["status"] == "Valid", "official EFEngine signature admission changed")
    require(official_engine["authenticode"]["thumbprint"] == "111E6CACF5BC32D5B50632411F523905284AEBCC", "official EFEngine signer changed")
    require(
        official_engine["abiExports"]
        == [
            {"role": role, "decoratedName": value[0], "rva": value[1]}
            for role, value in OFFICIAL_ABI_EXPORTS.items()
        ],
        "official EFEngine ABI projection changed",
    )
    mesh_join = receipt["sourceMeshOccurrenceJoin"]
    require(mesh_join["occurrenceCount"] == 3, "source mesh occurrence denominator changed")
    require(mesh_join["rendererAdmission"] == "SOURCE_EXACT_MESH_PARTICLE_AND_TYPEDATAMESH", "source mesh renderer admission changed")
    require(not mesh_join["localVertexFactorySelectionAdmission"], "unproven LocalVF native call-chain admission opened")
    require(
        [
            {
                "occurrenceId": row["occurrenceId"],
                "rendererType": row["rendererType"],
                "sourceEmitter": row["sourceEmitter"],
                "sourceMaterialPath": row["sourceMaterialPath"],
                "typeDataNodeId": row["typeDataMesh"]["nodeId"],
                "typeDataRecordSha256": row["typeDataMesh"]["recordSha256"],
            }
            for row in mesh_join["occurrences"]
        ]
        == [
            {
                "occurrenceId": occurrence_id,
                "rendererType": "MeshParticle",
                "sourceEmitter": expected["emitter"],
                "sourceMaterialPath": expected["material"],
                "typeDataNodeId": expected["typeDataNodeId"],
                "typeDataRecordSha256": expected["typeDataRecordSha256"],
            }
            for occurrence_id, expected in EXPECTED_MESH_OCCURRENCES.items()
        ],
        "source mesh occurrence projection changed",
    )
    require(decision["exactSameDistributionCohortShaderMapJoinCount"] == 2, "exact map join denominator changed")
    require(decision["exactLocalVfBasePassPixelDxbcCount"] == 2, "pixel DXBC denominator changed")
    require(decision["exactLocalVfBasePassVertexPermutationCount"] == 4, "vertex DXBC denominator changed")
    require(decision["uniformExpressionSetParseCount"] == 2, "uniform parse denominator changed")
    require(decision["textureRegisterBindingCount"] == 7, "texture binding denominator changed")
    require(decision["sourceMeshOccurrenceJoinCount"] == 3, "source mesh join denominator changed")
    require(decision["selectedLocalVfDxbcReplayCandidateAdmission"], "LocalVF replay candidate admission closed")
    require(
        not decision["meshParticleToLocalVfNativeCallChainAdmission"]
        and not decision["occurrenceSelectedLocalVfRuntimeAdmission"],
        "unproven occurrence LocalVF runtime admission opened",
    )
    require(
        decision["constantBufferRegisterBindingAdmission"]
        and not decision["fixedInputOriginalDxbcReplayAdmission"]
        and not decision["runtimeHlslMutationAdmission"]
        and not decision["visualProgressAdmission"],
        "unreviewed downstream admission opened",
    )
    official = receipt["officialRefShaderCacheV974"]
    installed = receipt["currentInstalledRefShaderCache"]
    require(len(official["mainTargets"]) == len(installed["mainTargets"]) == 2, "main target denominator changed")
    for target in TARGETS:
        for cache in (official, installed):
            row = next(item for item in cache["mainTargets"] if item["familyId"] == target["familyId"])
            require(row["search"]["normalizedExactStaticSetOccurrenceCount"] == 2, "normalized static-set count changed")
            require(row["search"]["exactMapContextCount"] == 1, "map context count changed")
            require(len(row["materialMap"]["selectedOriginalDxbc"]) == 5, "selected DXBC denominator changed")
            require(len(row["textureRegisterBindings"]) == len(row["materialMap"]["uniformExpressionSet"]["pixelTexture2DExpressions"]), "texture binding closure changed")
            binding = row["pixelShaderObjectBinding"]
            expected_object = target[
                "expectedOfficialShaderObject" if cache is official else "expectedInstalledShaderObject"
            ]
            require(binding["logicalOffset"] == expected_object["logicalOffset"], "shader-object offset changed")
            require(binding["shaderObjectRawSha256"] == expected_object["rawSha256"], "shader-object SHA changed")
            require(binding["bindingArraysRawSha256"] == target["expectedBindingArraysSha256"], "shader binding-array SHA changed")
            require(
                [len(binding[name]) for name in ("scalarGroups", "vectors", "textures")]
                == target["expectedBindingCounts"],
                "shader binding denominator changed",
            )
            for array_name, expected_projection in (
                ("scalarGroups", target["expectedScalarBindingProjection"]),
                ("vectors", target["expectedVectorBindingProjection"]),
                ("textures", target["expectedTextureBindingProjection"]),
            ):
                require(
                    [
                        [
                            item["expressionIndexOrGroup"],
                            item["baseIndex"],
                            item["numBytesOrResources"],
                            item["bufferIndexOrSamplerIndex"],
                        ]
                        for item in binding[array_name]
                    ] == expected_projection,
                    f"{array_name} native binding projection changed",
                )
            cb = row["constantBufferBindings"]
            require(cb["declaredConstantBuffer0Float4Count"] == target["expectedConstantBuffer0Float4Count"], "CB0 size changed")
            require(cb["unboundPackedScalarGroups"] == target["expectedUnboundScalarGroups"], "unbound scalar groups changed")
            require(cb["maximumNativeBoundConstantBuffer0Slot"] + 1 == cb["declaredConstantBuffer0Float4Count"], "CB0 max-slot closure changed")
            require(
                [
                    [
                        item["expressionIndex"],
                        item["constantBufferIndex"],
                        item["constantBufferSlot"],
                        item["componentMask"],
                        item["byteSize"],
                    ]
                    for item in cb["vectorBindings"]
                ]
                == [
                    [expression_index, 0, base_index // 16, "xyzw", 16]
                    for expression_index, base_index, _, _
                    in target["expectedVectorBindingProjection"]
                ],
                "decoded vector-to-CB projection changed",
            )
            expected_scalar_bindings = []
            scalar_expression_count = target["expectedUniformCounts"][1]
            for group, base_index, _, _ in target["expectedScalarBindingProjection"]:
                for component_index, component in enumerate("xyzw"):
                    expression_index = group * 4 + component_index
                    if expression_index >= scalar_expression_count:
                        break
                    expected_scalar_bindings.append(
                        [expression_index, group, 0, base_index // 16, component]
                    )
            require(
                [
                    [
                        item["expressionIndex"],
                        item["packedScalarGroup"],
                        item["constantBufferIndex"],
                        item["constantBufferSlot"],
                        item["component"],
                    ]
                    for item in cb["scalarBindings"]
                ] == expected_scalar_bindings,
                "decoded scalar-to-CB projection changed",
            )
            pixel = next(item for item in row["materialMap"]["selectedOriginalDxbc"] if item["shaderType"] == BASE_PASS_PIXEL)
            require(pixel["dxbcSha256"] == target["expectedPixelDxbcSha256"], "pixel DXBC SHA changed")
            require(not pixel["rdefChunkPresent"] and not pixel["reflectionNameAdmission"], "stripped DXBC name admission opened")
            pairs: dict[str, int] = {}
            for sample in pixel["disassembly"]["sampleInstructions"]:
                key = f"{sample['textureRegister']}/{sample['samplerRegister']}"
                pairs[key] = pairs.get(key, 0) + 1
            require(pairs == target["expectedSamplePairs"], "DXBC sample pairs changed")
            selection = row["materialMap"]["meshParticlePassSelection"]
            require(
                selection["sourceOccurrenceRendererType"] == "MeshParticle"
                and selection["selectedVertexFactory"] == LOCAL_VF
                and selection["selectedPass"] == "BASE_PASS_NO_LIGHTMAP_SKYLIGHT",
                "replay candidate pass selection changed",
            )
            require(
                [
                    [item["uniformTexture2DExpressionIndex"], item["textureRegister"], item["samplerRegister"], item["effectiveSourceObjectPath"]]
                    for item in row["textureRegisterBindings"]
                ] == target["expectedTextureBindings"],
                "resolved texture/register binding changed",
            )


def check_or_write(path: Path, receipt: dict[str, Any], check: bool) -> None:
    if check:
        require(path.is_file(), f"RefShaderCache receipt is missing: {path}")
        require(read_json(path) == receipt, "RefShaderCache receipt is stale")
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
    parser.add_argument("--identity-receipt", type=Path, default=DEFAULT_IDENTITY_RECEIPT)
    parser.add_argument("--material-contract", type=Path, default=DEFAULT_MATERIAL_CONTRACT)
    parser.add_argument("--native-receipt", type=Path, default=DEFAULT_NATIVE_RECEIPT)
    parser.add_argument("--source-inventory", type=Path, default=DEFAULT_SOURCE_INVENTORY)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_OFFICIAL_MANIFEST)
    parser.add_argument("--official-archive", type=Path, default=DEFAULT_OFFICIAL_ARCHIVE)
    parser.add_argument("--official-cache", type=Path, default=DEFAULT_OFFICIAL_CACHE)
    parser.add_argument("--official-efengine", type=Path, default=DEFAULT_OFFICIAL_EFENGINE)
    parser.add_argument("--official-efengine-archive", type=Path, default=DEFAULT_OFFICIAL_EFENGINE_ARCHIVE)
    parser.add_argument("--installed-cache", type=Path, default=DEFAULT_INSTALLED_CACHE)
    parser.add_argument("--installed-root", type=Path, default=DEFAULT_INSTALLED_ROOT)
    parser.add_argument("--d3dcompiler", type=Path, default=DEFAULT_D3DCOMPILER)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--validate-only", action="store_true")
    args = parser.parse_args()
    if args.validate_only:
        require(args.output.is_file(), f"RefShaderCache receipt is missing: {args.output}")
        validate_receipt(read_json(args.output))
        print("PASS: Artist F main RefShaderCache receipt shallow maps=2 dxbc=2+4 textures=7 cb=true replay=false hlsl=false")
        return 0
    receipt = build_receipt(
        args.identity_receipt,
        args.material_contract,
        args.native_receipt,
        args.source_inventory,
        args.manifest,
        args.official_archive,
        args.official_cache,
        args.official_efengine,
        args.official_efengine_archive,
        args.installed_cache,
        args.installed_root,
        args.d3dcompiler,
    )
    validate_receipt(receipt)
    check_or_write(args.output, receipt, args.check)
    mode = "deep-check" if args.check else "deep-write"
    print(f"PASS: Artist F main RefShaderCache receipt {mode} maps=2 dxbc=2+4 textures=7 cb=true replay=false hlsl=false")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
