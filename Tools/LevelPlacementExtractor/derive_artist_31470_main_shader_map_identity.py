#!/usr/bin/env python3
"""Derive the bounded Lost Ark v868 shader-map lookup identity for Artist F.

The installed EFEngine ABI shows that this build indexes material shader maps by
``FStaticParameterSet`` and selects them by ``EShaderPlatform``.  This tool
therefore preserves the source bytes and offsets, but uses an offset-free
semantic projection for cross-package joins.  It does not claim that the
current installed binary and the pinned source UPKs are the same revision.
"""

from __future__ import annotations

import argparse
import ctypes
import hashlib
import json
import os
import struct
import subprocess
from pathlib import Path
from typing import Any

from build_artist_31470_custom_handler_oracle import (
    parse_pe_exports,
)
from extract_artist_31470_shader_cache_oracle import (
    canonical_json_sha256,
    parse_static_parameter_set,
    read_json,
    windows_file_version,
)
from extract_ue3_effect_material_closure import load_package
from extract_ue3_placements import (
    LOSTARK_KR_AES_KEY,
    package_ref_name,
    package_ref_path,
    parse_tagged_properties,
)


SCHEMA = "lostark.artist-31470-main-shader-map-identity-receipt"
FORMAT_VERSION = 2
REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = Path(__file__).resolve()
DEFAULT_SHADER_RECEIPT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.shader-cache-oracle.receipt.json"
)
DEFAULT_NATIVE_RECEIPT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-native-resource.receipt.json"
)
DEFAULT_OUTPUT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.main-shader-map-identity.receipt.json"
)
DEFAULT_SOURCE_ROOT = Path(
    r"C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages"
)
DEFAULT_EFENGINE = Path(
    r"C:\ProgramData\Smilegate\Games\LOSTARK\Binaries\Win64\EFEngine.dll"
)

EXPECTED_EFENGINE = {
    "fileName": "EFEngine.dll",
    "byteSize": 19_606_896,
    "rawSha256": "d086134c7c9e5be10e55c2efdad5a6f35d0c8dc9a02e217def5593aa738f3f4d",
    "fixedFileVersion": "1.0.12097.0",
    "fileVersionResourceString": "1, 0, 8767, 0",
    "signatureStatus": "Valid",
    "signerSubject": 'CN="SMILEGATE RPG, Inc.", O="SMILEGATE RPG, Inc.", L=Seongnam-si, S=Gyeonggi-do, C=KR',
    "signerThumbprint": "111E6CACF5BC32D5B50632411F523905284AEBCC",
}

EXPORT_EVIDENCE = (
    {
        "role": "GLOBAL_LOOKUP_MAP",
        "decoratedName": "?GIdToMaterialShaderMap@FMaterialShaderMap@@0PAV?$TMap@VFStaticParameterSet@@PEAVFMaterialShaderMap@@VFDefaultSetAllocator@@@@A",
        "expectedRva": 0x021900B8,
        "byteCount": 0,
        "expectedByteSha256": None,
    },
    {
        "role": "LOOKUP_WITH_PLATFORM",
        "decoratedName": "?FindId@FMaterialShaderMap@@SAPEAV1@AEBVFStaticParameterSet@@W4EShaderPlatform@@@Z",
        "expectedRva": 0x009FCE90,
        "byteCount": 81,
        "expectedByteSha256": "f77c43d835df480276e582833d35a38feff9a11724d598aeffface32a90a0ff1",
    },
    {
        "role": "MATERIAL_ID_ACCESSOR",
        "decoratedName": "?GetMaterialId@FMaterialShaderMap@@QEBAAEBVFStaticParameterSet@@XZ",
        "expectedRva": 0x002486E0,
        "byteCount": 8,
        "expectedByteHex": "488d81a4000000c3",
    },
    {
        "role": "PLATFORM_ACCESSOR",
        "decoratedName": "?GetShaderPlatform@FMaterialShaderMap@@QEBA?AW4EShaderPlatform@@XZ",
        "expectedRva": 0x002EB640,
        "byteCount": 7,
        "expectedByteHex": "8b81a0000000c3",
    },
    {
        "role": "CACHE_STATIC_MAP_LOOKUP",
        "decoratedName": "?FindStaticShaderMap@UShaderCache@@QEAAIAEBVFStaticParameterSet@@@Z",
        "expectedRva": 0x00238780,
        "byteCount": 74,
        "expectedByteSha256": "94db141f0568101886d2a6604b78f9c3aeee26e1e279014905f0a585dd7fd28d",
    },
    {
        "role": "MATERIAL_SHADER_MAP_SERIALIZER",
        "decoratedName": "?Serialize@FMaterialShaderMap@@QEAAXAEAVFArchive@@@Z",
        "expectedRva": 0x00A23580,
        "byteCount": 256,
        "expectedByteSha256": "50a926d554c03975a3e8337a61c45613582c71e4228bb2afc3d23cafddcaad50",
    },
    {
        "role": "SHADER_CACHE_SERIALIZER",
        "decoratedName": "?Serialize@UShaderCache@@UEAAXAEAVFArchive@@@Z",
        "expectedRva": 0x005ED930,
        "byteCount": 256,
        "expectedByteSha256": "289eabddd4eeedba6882d627f62c03f7d2f709b1fbfa72e9c8019c88a2fc35ba",
    },
)

TARGETS = (
    {
        "label": "#9/#10_WATERTRAIL",
        "occurrenceIds": ["source-active-009", "source-active-010"],
        "familyId": "material-family-89af5c77d8e35f99",
        "recipeId": "material-recipe-03cc03b86c1a4c8f",
        "baseMaterialIdHex": "06e9a0ae14b09646b949a245fc42aa3c",
        "expectedSwitchCount": 13,
        "expectedRawSetSha256": "2aed9a10b51648697338e561a3447fdbf5045ec460936dfe0f217f5038fc7c6f",
        "expectedNormalizedSetSha256": "32d71b8321f85a868c18f48662fedf79a74c79b85ce41cf708c1d0b88605c8c3",
        "expectedEngineEqualitySetSha256": "54a52a78d4c82bd42962193bcb3a64e28cce1275eb69c9affb1d3006478abcc3",
        "rejectedMicTailCandidateHex": "754b282586cf134ebdb44d9797eeb9ed",
    },
    {
        "label": "#11_SPRITEWAVE",
        "occurrenceIds": ["source-active-011"],
        "familyId": "material-family-097bd8d9597721b5",
        "recipeId": "material-recipe-daf220acad2b656e",
        "baseMaterialIdHex": "b1f4ebf9dc948c41bd2830d999ba16cc",
        "expectedSwitchCount": 20,
        "expectedRawSetSha256": "6c65f31c8b18dad28e25cbd334d14dea161793e492e536d3587728ae20521027",
        "expectedNormalizedSetSha256": "6d75571de189229a3052963c9c227d1be9e2d38ccbf88a1cf245abdd90a219a2",
        "expectedEngineEqualitySetSha256": "468bfdf79d6dc23e741433c076e865a0dc985c19ebfc0e1519efd8ca20aad846",
        "rejectedMicTailCandidateHex": "e2cceb273bcc31468b32126083992f8a",
    },
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


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
    return sha256_bytes(text.replace("\r\n", "\n").replace("\r", "\n").encode("utf-8"))


def seal(receipt: dict[str, Any]) -> None:
    receipt.pop("receiptSha256", None)
    receipt["receiptSha256"] = canonical_json_sha256(receipt)


def normalized_static_parameter_set(value: dict[str, Any]) -> dict[str, Any]:
    """Return the serialized semantic projection without package-local fields.

    This projection deliberately preserves array order, ``bOverride`` and the
    legacy normal array.  It is useful for proving that two serialized payloads
    carry the same rows, but it is not the native FStaticParameterSet equality
    key used by Lost Ark v868.
    """

    projection: dict[str, Any] = {"baseMaterialIdHex": value["baseMaterialIdHex"]}
    for array_name in (
        "staticSwitchParameters",
        "staticComponentMaskParameters",
        "normalParameters",
        "terrainLayerWeightParameters",
    ):
        rows = value.get(array_name)
        require(isinstance(rows, list), f"{array_name} is absent")
        projected_rows = []
        for row in rows:
            require(isinstance(row, dict), f"{array_name} row is invalid")
            projected = {
                key: item
                for key, item in row.items()
                if key not in ("entryOffset", "parameterName")
            }
            projected = {
                "parameterNameComparisonCasefold": str(row["parameterName"]).casefold(),
                "parameterNameNumber": 0,
                **projected,
            }
            projected_rows.append(projected)
        projection[array_name] = projected_rows
    return projection


def engine_equivalent_static_parameter_set(value: dict[str, Any]) -> dict[str, Any]:
    """Project Lost Ark v868 FStaticParameterSet native equality semantics.

    The official-cohort EFEngine equality routine compares the base material
    GUID and finds switch, component-mask and terrain rows by the
    case-insensitive FName plus ExpressionGUID.  Row order and ``bOverride`` do
    not participate.  The legacy serialized normal array is read temporarily
    by this package version but is not resident in, or compared by, the native
    set.  Sorting here only canonicalizes an order-independent native lookup.
    """

    projection: dict[str, Any] = {"baseMaterialIdHex": value["baseMaterialIdHex"]}
    specifications = (
        ("staticSwitchParameters", ("value",)),
        ("staticComponentMaskParameters", ("r", "g", "b", "a")),
        ("terrainLayerWeightParameters", ("valueOrdinalCandidate",)),
    )
    for array_name, value_fields in specifications:
        rows = value.get(array_name)
        require(isinstance(rows, list), f"{array_name} is absent")
        projected_rows = []
        seen_keys: set[tuple[str, int, str]] = set()
        for row in rows:
            require(isinstance(row, dict), f"{array_name} row is invalid")
            key = (
                str(row["parameterName"]).casefold(),
                0,
                str(row["expressionGuidHex"]).casefold(),
            )
            require(key not in seen_keys, f"{array_name} native equality key is duplicated")
            seen_keys.add(key)
            projected = {
                "parameterNameComparisonCasefold": key[0],
                "parameterNameNumber": key[1],
                "expressionGuidHex": key[2],
            }
            for field in value_fields:
                require(field in row, f"{array_name} {field} is absent")
                public_field = (
                    "weightmapIndexOrdinal"
                    if array_name == "terrainLayerWeightParameters"
                    and field == "valueOrdinalCandidate"
                    else field
                )
                projected[public_field] = row[field]
            projected_rows.append(projected)
        projection[array_name] = sorted(
            projected_rows,
            key=lambda row: (
                row["parameterNameComparisonCasefold"],
                row["parameterNameNumber"],
                row["expressionGuidHex"],
            ),
        )
    return projection


def package_identity(package: Any) -> dict[str, Any]:
    summary = package.summary
    return {
        "fileName": package.path.name,
        "physicalByteSize": package.path.stat().st_size,
        "rawSha256": package.sha256,
        "packageVersion": summary.version,
        "licenseeVersion": summary.licensee_version,
        "engineVersion": summary.engine_version,
        "cookerVersion": summary.cooker_version,
        "packageGuidOffsetInPhysicalSummary": summary.package_guid_offset,
        "packageGuidRawHex": summary.package_guid_hex,
        "logicalByteSize": len(package.logical),
        "logicalPackageSha256": sha256_bytes(package.logical),
        "nameCount": summary.name_count,
        "importCount": summary.import_count,
        "exportCount": summary.export_count,
        "hashRole": "EXTERNAL_RAW_PACKAGE_BYTES",
    }


def find_indexed_export(package: Any, index: int, expected_path: str, expected_class: str) -> Any:
    require(type(index) is int and 0 <= index < len(package.exports), "export index is invalid")
    entry = package.exports[index]
    actual_path = package_ref_path(entry.index + 1, package.imports, package.exports)
    actual_class = package_ref_name(entry.class_index, package.imports, package.exports)
    require(actual_path.casefold() == expected_path.casefold(), "export object path changed")
    require(actual_class.casefold() == expected_class.casefold(), "export class changed")
    return entry


def authenticode_identity(path: Path) -> dict[str, Any]:
    script = (
        "$s=Get-AuthenticodeSignature -LiteralPath $env:ARTIST31470_EFENGINE_PATH;"
        "$v=[System.Diagnostics.FileVersionInfo]::GetVersionInfo($env:ARTIST31470_EFENGINE_PATH);"
        "[pscustomobject]@{status=[string]$s.Status;"
        "subject=[string]$s.SignerCertificate.Subject;"
        "thumbprint=[string]$s.SignerCertificate.Thumbprint;"
        "fileVersionResourceString=[string]$v.FileVersion}|ConvertTo-Json -Compress"
    )
    environment = dict(os.environ)
    environment["ARTIST31470_EFENGINE_PATH"] = str(path)
    result = subprocess.run(
        ["powershell", "-NoProfile", "-Command", script],
        check=True,
        capture_output=True,
        text=True,
        encoding="utf-8",
        env=environment,
    )
    value = json.loads(result.stdout.strip())
    return {
        "status": value["status"],
        "signerSubject": value["subject"],
        "signerThumbprint": value["thumbprint"],
        "fileVersionResourceString": value["fileVersionResourceString"],
    }


def pe_abi_identity(path: Path) -> dict[str, Any]:
    require(path.is_file(), f"EFEngine is missing: {path}")
    raw = path.read_bytes()
    identity = {
        "fileName": path.name,
        "physicalByteSize": len(raw),
        "rawSha256": sha256_bytes(raw),
        "fixedFileVersion": windows_file_version(path),
        "authenticode": authenticode_identity(path),
    }
    require(identity["fileName"] == EXPECTED_EFENGINE["fileName"], "EFEngine name changed")
    require(identity["physicalByteSize"] == EXPECTED_EFENGINE["byteSize"], "EFEngine size changed")
    require(identity["rawSha256"] == EXPECTED_EFENGINE["rawSha256"], "EFEngine SHA changed")
    require(identity["fixedFileVersion"] == EXPECTED_EFENGINE["fixedFileVersion"], "EFEngine fixed file version changed")
    signature = identity["authenticode"]
    require(signature["status"] == EXPECTED_EFENGINE["signatureStatus"], "EFEngine signature status changed")
    require(signature["signerSubject"] == EXPECTED_EFENGINE["signerSubject"], "EFEngine signer changed")
    require(signature["signerThumbprint"] == EXPECTED_EFENGINE["signerThumbprint"], "EFEngine signer thumbprint changed")
    require(signature["fileVersionResourceString"] == EXPECTED_EFENGINE["fileVersionResourceString"], "EFEngine file version resource string changed")

    exports = parse_pe_exports(path)
    library = ctypes.WinDLL(str(path))
    kernel32 = ctypes.windll.kernel32
    kernel32.GetProcAddress.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
    kernel32.GetProcAddress.restype = ctypes.c_void_p
    rows = []
    for expected in EXPORT_EVIDENCE:
        name = expected["decoratedName"]
        require(name in exports, f"required EFEngine export is missing: {name}")
        rva = exports[name]
        require(rva == expected["expectedRva"], f"EFEngine export RVA changed: {name}")
        byte_count = expected["byteCount"]
        row = {
            "role": expected["role"],
            "decoratedName": name,
            "rva": rva,
            "rvaHex": f"0x{rva:08X}",
        }
        if byte_count:
            mapped_address = kernel32.GetProcAddress(library._handle, name.encode("ascii"))
            require(bool(mapped_address), f"GetProcAddress failed: {name}")
            function_bytes = ctypes.string_at(mapped_address, byte_count)
            require(len(function_bytes) == byte_count, "EFEngine function evidence is truncated")
            row.update(
                {
                    "postLoaderMappedByteCount": byte_count,
                    "postLoaderMappedRawHex": function_bytes.hex() if byte_count <= 16 else None,
                    "postLoaderMappedSha256": sha256_bytes(function_bytes),
                    "byteEvidenceRole": "POST_LOADER_MAPPED_CODE_FROM_HASH_PINNED_AUTHENTICODE_BINARY",
                }
            )
            if expected.get("expectedByteHex"):
                require(function_bytes.hex() == expected["expectedByteHex"], f"EFEngine accessor bytes changed: {name}")
            if expected.get("expectedByteSha256"):
                require(row["postLoaderMappedSha256"] == expected["expectedByteSha256"], f"EFEngine function bytes changed: {name}")
        rows.append(row)
    identity["exports"] = rows
    identity["abiConclusion"] = {
        "lookupKeyType": "FStaticParameterSet",
        "platformSelectionType": "EShaderPlatform",
        "materialShaderMapPlatformFieldOffsetHex": "0xA0",
        "materialShaderMapIdFieldOffsetHex": "0xA4",
        "separate16ByteMaterialShaderMapIdSupported": False,
        "fidelity": "CURRENT_INSTALLED_HASH_PINNED_BINARY_ABI_CORROBORATION_NOT_SOURCE_REVISION_PROOF",
    }
    return identity


def decode_target(
    target: dict[str, Any],
    native_receipt: dict[str, Any],
    shader_receipt: dict[str, Any],
    source_root: Path,
    cache: dict[str, Any],
) -> dict[str, Any]:
    native = next(row for row in native_receipt["families"] if row["familyId"] == target["familyId"])
    recipe = next(row for row in shader_receipt["recipeNativeKeys"] if row["recipeId"] == target["recipeId"])
    require(recipe["arithmeticFamilyId"] == target["familyId"], "recipe family changed")

    def open_package(name: str) -> Any:
        key = name.casefold()
        if key not in cache:
            cache[key] = load_package(source_root / name, LOSTARK_KR_AES_KEY)
        return cache[key]

    base_package = open_package(native["sourcePhysicalPackage"])
    require(base_package.sha256 == native["sourcePhysicalPackageSha256"], "base package SHA changed")
    base_source = native["source"]
    base_entry = find_indexed_export(
        base_package,
        base_source["exportIndex"],
        base_source["objectPath"],
        base_source["className"],
    )
    base_serial = base_package.logical[base_entry.serial_offset : base_entry.serial_offset + base_entry.serial_size]
    require(sha256_bytes(base_serial) == base_source["serialSha256"], "base serial SHA changed")
    _base_properties, base_property_end = parse_tagged_properties(base_serial, base_package.names, base_package.summary.version)
    require(base_property_end == base_source["propertyStreamEnd"], "base property end changed")
    base_tail = base_serial[base_property_end:]
    base_guid_offset = 16
    base_guid = base_tail[base_guid_offset : base_guid_offset + 16]
    require(base_guid.hex() == target["baseMaterialIdHex"], "base Material state GUID changed")
    require(base_guid.hex() == base_source["materialStateGuidHex"], "native receipt base GUID changed")

    mic_package = open_package(recipe["physicalPackage"])
    require(mic_package.sha256 == recipe["physicalPackageSha256"], "MIC package SHA changed")
    mic_entry = find_indexed_export(
        mic_package,
        recipe["exportIndex"],
        recipe["materialObjectPath"],
        recipe["className"],
    )
    mic_serial = mic_package.logical[mic_entry.serial_offset : mic_entry.serial_offset + mic_entry.serial_size]
    require(sha256_bytes(mic_serial) == recipe["serialSha256"], "MIC serial SHA changed")
    _mic_properties, mic_property_end = parse_tagged_properties(mic_serial, mic_package.names, mic_package.summary.version)
    require(mic_property_end == recipe["propertyStreamEnd"], "MIC property end changed")
    mic_tail = mic_serial[mic_property_end:]
    set_offset = mic_tail.find(base_guid)
    require(set_offset >= 0 and mic_tail.find(base_guid, set_offset + 1) < 0, "MIC base GUID occurrence is not unique")
    require(set_offset == recipe["baseMaterialNativeKeyOffsetInTail"], "MIC set offset changed")
    decoded = parse_static_parameter_set(mic_tail, set_offset, mic_package.names)
    raw_set = mic_tail[set_offset : decoded["endOffset"]]
    require(decoded["rawSha256"] == target["expectedRawSetSha256"], "MIC static set raw SHA changed")
    require(len(decoded["staticSwitchParameters"]) == target["expectedSwitchCount"], "MIC switch denominator changed")
    require(not decoded["staticComponentMaskParameters"] and not decoded["normalParameters"] and not decoded["terrainLayerWeightParameters"], "unexpected main static parameter array became populated")
    normalized = normalized_static_parameter_set(decoded)
    normalized_sha = canonical_json_sha256(normalized)
    engine_equality = engine_equivalent_static_parameter_set(decoded)
    engine_equality_sha = canonical_json_sha256(engine_equality)
    parameter_offsets = []
    for array_name in (
        "staticSwitchParameters",
        "staticComponentMaskParameters",
        "normalParameters",
        "terrainLayerWeightParameters",
    ):
        for index, row in enumerate(decoded[array_name]):
            parameter_offsets.append(
                {
                    "arrayName": array_name,
                    "index": index,
                    "parameterName": row["parameterName"],
                    "entryOffsetInStaticParameterSet": row["entryOffset"] - set_offset,
                    "entryOffsetInMicNativeTail": row["entryOffset"],
                    "logicalPackageOffset": mic_entry.serial_offset + mic_property_end + row["entryOffset"],
                }
            )

    platform = 4
    lookup_projection = {
        "identityType": "FStaticParameterSet",
        "shaderPlatformOrdinal": platform,
        "staticParameterSet": normalized,
    }
    rejected = mic_tail[16:32]
    require(rejected.hex() == target["rejectedMicTailCandidateHex"], "rejected MIC candidate changed")
    require(rejected != base_guid, "rejected MIC candidate aliases BaseMaterialId")
    return {
        "label": target["label"],
        "occurrenceIds": target["occurrenceIds"],
        "familyId": target["familyId"],
        "recipeId": target["recipeId"],
        "baseMaterial": {
            "package": package_identity(base_package),
            "objectPath": base_source["objectPath"],
            "className": base_source["className"],
            "exportIndexZeroBased": base_entry.index,
            "serialOffset": base_entry.serial_offset,
            "serialSize": base_entry.serial_size,
            "serialSha256": sha256_bytes(base_serial),
            "propertyStreamEnd": base_property_end,
            "nativeTailOffsetInSerial": base_property_end,
            "nativeTailByteCount": len(base_tail),
            "nativeTailSha256": sha256_bytes(base_tail),
            "materialStateGuidOffsetInNativeTail": base_guid_offset,
            "materialStateGuidLogicalPackageOffset": base_entry.serial_offset + base_property_end + base_guid_offset,
            "materialStateGuidRawHex": base_guid.hex(),
        },
        "activeMic": {
            "package": package_identity(mic_package),
            "objectPath": recipe["materialObjectPath"],
            "className": recipe["className"],
            "exportIndexZeroBased": mic_entry.index,
            "serialOffset": mic_entry.serial_offset,
            "serialSize": mic_entry.serial_size,
            "serialSha256": sha256_bytes(mic_serial),
            "propertyStreamEnd": mic_property_end,
            "nativeTailOffsetInSerial": mic_property_end,
            "nativeTailByteCount": len(mic_tail),
            "nativeTailSha256": sha256_bytes(mic_tail),
            "staticParameterSetOffsetInNativeTail": set_offset,
            "staticParameterSetLogicalPackageOffset": mic_entry.serial_offset + mic_property_end + set_offset,
            "staticParameterSetByteCount": len(raw_set),
            "staticParameterSetRawHex": raw_set.hex(),
            "staticParameterSetRawSha256": sha256_bytes(raw_set),
            "parameterOffsets": parameter_offsets,
        },
        "derivedLookupIdentity": {
            **lookup_projection,
            "normalizedStaticParameterSetSha256": normalized_sha,
            "serializedSemanticProjectionRole": "CROSS_PACKAGE_SERIALIZED_ROW_PARITY_NOT_NATIVE_LOOKUP_EQUALITY",
            "engineEqualityStaticParameterSet": engine_equality,
            "engineEqualityStaticParameterSetSha256": engine_equality_sha,
            "lookupIdentitySha256": canonical_json_sha256(
                {
                    "identityType": "FStaticParameterSet",
                    "shaderPlatformOrdinal": platform,
                    "engineEqualityStaticParameterSet": engine_equality,
                }
            ),
            "crossPackageEqualityRule": "BASE_GUID_PLUS_ORDER_INDEPENDENT_FNAME_CASEFOLD_AND_EXPRESSION_GUID_SWITCH_VALUE_COMPONENT_MASK_RGBA_TERRAIN_WEIGHTMAP_INDEX;_BOVERRIDE_AND_LEGACY_NORMAL_EXCLUDED",
            "fidelity": "LOSTARK_V868_BOUNDED_SHADER_MAP_ID_HYPOTHESIS_CURRENT_BINARY_ABI_CORROBORATED",
        },
        "rejectedIdentityCandidate": {
            "offsetInMicNativeTail": 16,
            "rawHex": rejected.hex(),
            "status": "NOT_FMATERIALSHADERMAP_ID_CURRENT_BINARY_ABI_USES_FULL_FSTATICPARAMETERSET",
        },
        "sameRevisionShaderMapJoinCount": 0,
        "dxbcReplayAdmission": False,
        "runtimeEvaluatorAdmission": False,
        "visualProgressAdmission": False,
    }


def map_serial_boundary_evidence(shader_receipt: dict[str, Any]) -> dict[str, Any]:
    primary = shader_receipt["primaryShaderCache"]
    structure = primary["decodedNativeStructure"]
    serial_offset = next(
        row["serialOffset"]
        for row in shader_receipt["shaderCacheCandidates"]
        if row["objectPath"].casefold() == primary["objectPath"].casefold()
    )
    tail_offset = structure["tailOffsetInSerial"]
    rows = []
    for material_map in structure["materialShaderMaps"]:
        candidate = material_map["opaqueMapSuffix"]["mapSerialCandidate"]
        calculated = serial_offset + tail_offset + material_map["offset"] + material_map["byteSize"]
        require(candidate == calculated, "mapSerialCandidate is not the absolute map-end boundary")
        rows.append(
            {
                "materialShaderMapIndex": material_map["materialShaderMapIndex"],
                "serializedCandidate": candidate,
                "calculatedAbsoluteMapEnd": calculated,
                "equal": True,
            }
        )
    require(len(rows) == 25, "central cache map denominator changed")
    return {
        "shaderCacheExportObjectPath": primary["objectPath"],
        "shaderCacheExportSerialOffset": serial_offset,
        "nativeTailOffsetInSerial": tail_offset,
        "materialShaderMapCount": len(rows),
        "absoluteMapEndEqualityCount": sum(row["equal"] for row in rows),
        "rows": rows,
        "conclusion": "MAP_SERIAL_CANDIDATE_IS_ABSOLUTE_MAP_END_NOT_SHADER_MAP_ID",
    }


def build_receipt(
    shader_receipt_path: Path,
    native_receipt_path: Path,
    source_root: Path,
    efengine_path: Path,
) -> dict[str, Any]:
    shader_receipt = read_json(shader_receipt_path)
    native_receipt = read_json(native_receipt_path)
    require(shader_receipt["schema"] == "lostark.artist-31470-shader-cache-oracle-receipt", "shader receipt schema changed")
    require(native_receipt["schema"] == "lostark.artist-31470-material-native-resource-receipt", "native receipt schema changed")
    packages: dict[str, Any] = {}
    targets = [decode_target(row, native_receipt, shader_receipt, source_root, packages) for row in TARGETS]
    for package in packages.values():
        require(package.summary.version == 868, "source package version changed")
        require(package.summary.licensee_version == 16, "source package licensee changed")
        require(package.summary.engine_version == 12097, "source package engine version changed")
        require(package.summary.cooker_version == 136, "source package cooker version changed")
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
                "repoRelativePath": shader_receipt_path.relative_to(REPO_ROOT).as_posix(),
                "receiptSha256": shader_receipt["receiptSha256"],
            },
            {
                "repoRelativePath": native_receipt_path.relative_to(REPO_ROOT).as_posix(),
                "receiptSha256": native_receipt["receiptSha256"],
            },
        ],
        "currentInstalledBinaryAbi": pe_abi_identity(efengine_path),
        "mainTargets": targets,
        "centralInstalledCacheMapSerialBoundary": map_serial_boundary_evidence(shader_receipt),
        "decision": {
            "identityDerivationImplemented": True,
            "lookupKeyType": "FStaticParameterSet_PLUS_EShaderPlatform_SELECTION",
            "targetFamilyCount": 2,
            "targetOccurrenceCount": 3,
            "exactSameRevisionShaderMapJoinCount": 0,
            "dxbcReplayCount": 0,
            "runtimeHlslMutationAdmission": False,
            "visualProgressAdmission": False,
            "nextGate": "FULL_INSTALLED_AND_SOURCE_ARCHIVE_STREAMING_SHADERCACHE_STRUCTURAL_JOIN",
            "status": "IDENTITY_DERIVED_JOIN_NOT_YET_ACQUIRED",
        },
    }
    seal(receipt)
    return receipt


def validate_receipt(receipt: dict[str, Any]) -> None:
    require(receipt.get("schema") == SCHEMA, "identity receipt schema mismatch")
    require(receipt.get("formatVersion") == FORMAT_VERSION, "identity receipt version mismatch")
    claimed = receipt.get("receiptSha256")
    unsigned = dict(receipt)
    unsigned.pop("receiptSha256", None)
    require(claimed == canonical_json_sha256(unsigned), "identity receipt digest mismatch")
    targets = receipt.get("mainTargets")
    require(isinstance(targets, list) and len(targets) == 2, "identity target denominator changed")
    require(sum(len(row["occurrenceIds"]) for row in targets) == 3, "identity occurrence denominator changed")
    require({row["familyId"] for row in targets} == {row["familyId"] for row in TARGETS}, "identity family set changed")
    for row in targets:
        expected = next(item for item in TARGETS if item["familyId"] == row["familyId"])
        derived = row["derivedLookupIdentity"]
        normalized = derived["staticParameterSet"]
        engine_equality = derived["engineEqualityStaticParameterSet"]
        require(normalized["baseMaterialIdHex"] == expected["baseMaterialIdHex"], "normalized BaseMaterialId changed")
        require(len(normalized["staticSwitchParameters"]) == expected["expectedSwitchCount"], "normalized switch count changed")
        require(derived["normalizedStaticParameterSetSha256"] == canonical_json_sha256(normalized), "normalized set digest changed")
        require(
            derived["normalizedStaticParameterSetSha256"]
            == expected["expectedNormalizedSetSha256"],
            "normalized target identity changed",
        )
        require(engine_equality["baseMaterialIdHex"] == expected["baseMaterialIdHex"], "engine-equality BaseMaterialId changed")
        require(len(engine_equality["staticSwitchParameters"]) == expected["expectedSwitchCount"], "engine-equality switch count changed")
        require(derived["engineEqualityStaticParameterSetSha256"] == canonical_json_sha256(engine_equality), "engine-equality set digest changed")
        require(
            derived["engineEqualityStaticParameterSetSha256"]
            == expected["expectedEngineEqualitySetSha256"],
            "engine-equality target identity changed",
        )
        lookup = {
            "identityType": derived["identityType"],
            "shaderPlatformOrdinal": derived["shaderPlatformOrdinal"],
            "engineEqualityStaticParameterSet": engine_equality,
        }
        require(derived["lookupIdentitySha256"] == canonical_json_sha256(lookup), "lookup identity digest changed")
        require(row["activeMic"]["staticParameterSetRawSha256"] == expected["expectedRawSetSha256"], "raw static set digest changed")
        require(row["rejectedIdentityCandidate"]["rawHex"] == expected["rejectedMicTailCandidateHex"], "rejected MIC candidate changed")
        require(not row["dxbcReplayAdmission"] and not row["runtimeEvaluatorAdmission"] and not row["visualProgressAdmission"], "identity receipt opened a downstream admission")
    boundary = receipt["centralInstalledCacheMapSerialBoundary"]
    require(boundary["materialShaderMapCount"] == 25 and boundary["absoluteMapEndEqualityCount"] == 25, "map boundary proof changed")
    decision = receipt["decision"]
    require(decision["identityDerivationImplemented"] is True, "identity derivation is not closed")
    require(decision["exactSameRevisionShaderMapJoinCount"] == 0, "unreviewed shader-map join appeared")
    require(decision["runtimeHlslMutationAdmission"] is False and decision["visualProgressAdmission"] is False, "identity stage opened visual admission")


def check_or_write(path: Path, receipt: dict[str, Any], check: bool) -> None:
    if check:
        require(path.is_file(), f"identity receipt is missing: {path}")
        require(read_json(path) == receipt, "identity receipt is stale")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(json.dumps(receipt, ensure_ascii=False, indent=2) + "\n", encoding="utf-8", newline="\n")
    temporary.replace(path)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--shader-receipt", type=Path, default=DEFAULT_SHADER_RECEIPT)
    parser.add_argument("--native-receipt", type=Path, default=DEFAULT_NATIVE_RECEIPT)
    parser.add_argument("--source-root", type=Path, default=DEFAULT_SOURCE_ROOT)
    parser.add_argument("--efengine", type=Path, default=DEFAULT_EFENGINE)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--validate-only", action="store_true")
    args = parser.parse_args()
    if args.validate_only:
        require(args.output.is_file(), f"identity receipt is missing: {args.output}")
        validate_receipt(read_json(args.output))
        print("PASS: Artist F main shader-map identity target=2 occurrence=3 join=0 visual=false")
        return 0
    receipt = build_receipt(args.shader_receipt, args.native_receipt, args.source_root, args.efengine)
    validate_receipt(receipt)
    check_or_write(args.output, receipt, args.check)
    mode = "check" if args.check else "write"
    print(f"PASS: Artist F main shader-map identity {mode} target=2 occurrence=3 join=0 visual=false")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
