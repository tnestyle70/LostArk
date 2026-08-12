#!/usr/bin/env python3
"""Seal fresh DDS exports for Artist 31470 base-Material lookup defaults.

These four textures are source-exact cooked base defaults, but every active
31470 recipe replaces them with a closer MIC override.  The receipt therefore
preserves extraction evidence while explicitly refusing runtime binding and
Product admission.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
from pathlib import Path
from typing import Any

from extract_artist_31470_shader_cache_oracle import (
    DEFAULT_SOURCE_PACKAGE_ROOT,
    canonical_json_sha256,
    raw_file_sha256,
    read_json,
)
from extract_ue3_effect_material_closure import load_package
from extract_ue3_placements import (
    LOSTARK_KR_AES_KEY,
    package_ref_name,
    package_ref_path,
    parse_tagged_properties,
)


SCHEMA = "lostark.artist-31470-native-base-texture-extraction-receipt"
FORMAT_VERSION = 1
REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = Path(__file__).resolve()
MATERIAL_ROOT = REPO_ROOT / "Data/Effects/Imported/Artist/Materials"
DEFAULT_NATIVE_RECEIPT = MATERIAL_ROOT / "skill.31470.material-native-resource.receipt.json"
DEFAULT_OUTPUT = MATERIAL_ROOT / "skill.31470.native-base-texture-extraction.receipt.json"
DEFAULT_RESOURCE_ROOT = Path(r"C:\Users\user\Desktop\Resource_LostArk")
UMODEL_RELATIVE_PATH = Path("06_Tools/UEViewerLostArk_runtime/umodel_lostark_v7.exe")
EXTRACTION_RELATIVE_ROOT = Path(
    "01_Extracted/Effect/ARTIST/31470_NativeBaseLookup_20260812"
)

EXPECTED_UMODEL = {
    "byteCount": 1766400,
    "rawSha256": "b9573cdcbb7e9d26dbf60a0e3af47fb5af8543140873da8483c26d58cf40b249",
}

EXPECTED_PACKAGES = {
    "WP_WGDH_01": {
        "fileName": "7U1M73IA1BZ6E7CKHX64S5P.upk",
        "byteCount": 1194109,
        "rawSha256": "bdfda5a2f65ce9f3c3ee7bdc2e4ae7c344f8b886b4eada7aca5d5e71b4eda5ee",
        "packageVersion": 868,
        "licenseeVersion": 16,
        "exportCount": 51,
    },
    "FX_TEX_HIGH_03": {
        "fileName": "ZHJ4TPSHN4JDK6D4T2NH2DJURL.upk",
        "byteCount": 49333042,
        "rawSha256": "887fa9c48aa14d54120dd6066bd0182328217775230cc0726e72e9d910bce7b3",
        "packageVersion": 868,
        "licenseeVersion": 16,
        "exportCount": 613,
    },
}

EXPECTED_ASSETS = [
    {
        "logicalTexturePath": "wp_wgdh_01.tex.wp_wgdh_01s_d",
        "familyId": "material-family-2c00ce5593538d7c",
        "parameterName": "diffuse_tex",
        "effectiveOverrideValue": "wp_mn_lrcn_01.tex.wp_mn_lrcn_01_d",
        "logicalPackage": "WP_WGDH_01",
        "exportObjectPath": "tex.wp_wgdh_01s_d",
        "exportIndex": 48,
        "serialOffset": 1154219,
        "serialSize": 84870,
        "serialSha256": "ae7045967e939043917d050d26feca11cde12ea9a7c74b174e47e828c12337d9",
        "propertyStreamEnd": 297,
        "properties": {
            "sizex": 512,
            "sizey": 512,
            "originalsizex": 512,
            "originalsizey": 512,
            "format": "pf_dxt1",
            "miptailbaseidx": 9,
            "busecrunchcompression": True,
            "lodgroup": "texturegroup_weapon",
        },
        "sourceExtractedDdsRelativePath": "WP_WGDH_01/Texture2D/wp_wgdh_01s_d.dds",
        "dds": {
            "byteCount": 131200,
            "rawSha256": "f6404f62f5bdcf0081b57f07d6d17523427eeb920c7ed5a1c36d873ad1102398",
            "width": 512,
            "height": 512,
            "fourCC": "DXT1",
        },
    },
    {
        "logicalTexturePath": "wp_wgdh_01.tex.wp_wgdh_01s_n",
        "familyId": "material-family-2c00ce5593538d7c",
        "parameterName": "normal_tex",
        "effectiveOverrideValue": "wp_mn_lrcn_01.tex.wp_mn_lrcn_01_n",
        "logicalPackage": "WP_WGDH_01",
        "exportObjectPath": "tex.wp_wgdh_01s_n",
        "exportIndex": 49,
        "serialOffset": 1239089,
        "serialSize": 186949,
        "serialSha256": "5bf53c5e76390fe4e74959b251dd6e497d866e036c4879581d4034ebe77b290a",
        "propertyStreamEnd": 446,
        "properties": {
            "sizex": 512,
            "sizey": 512,
            "originalsizex": 512,
            "originalsizey": 512,
            "format": "pf_bc5",
            "miptailbaseidx": 9,
            "srgb": False,
            "busecrunchcompression": True,
            "compressionsettings": "tc_normalmapbc5",
            "lodgroup": "texturegroup_weaponnormalmap",
        },
        "sourceExtractedDdsRelativePath": "WP_WGDH_01/Texture2D/wp_wgdh_01s_n.dds",
        "dds": {
            "byteCount": 262272,
            "rawSha256": "5c1f99b0fed0e4b406c392524ba562b5df32482fbac0fc16aff367c3c5dd0333",
            "width": 512,
            "height": 512,
            "fourCC": "ATI2",
        },
    },
    {
        "logicalTexturePath": "wp_wgdh_01.tex.wp_wgdh_01s_s",
        "familyId": "material-family-2c00ce5593538d7c",
        "parameterName": "spec_tex",
        "effectiveOverrideValue": "wp_mn_lrcn_01.tex.wp_mn_lrcn_01_s",
        "logicalPackage": "WP_WGDH_01",
        "exportObjectPath": "tex.wp_wgdh_01s_s",
        "exportIndex": 50,
        "serialOffset": 1426038,
        "serialSize": 46478,
        "serialSha256": "adf65eeae0bdaadc49c6b91dcf5ff63399936a2cf5e6fe6b3d3323d12564f33f",
        "propertyStreamEnd": 297,
        "properties": {
            "sizex": 256,
            "sizey": 256,
            "originalsizex": 256,
            "originalsizey": 256,
            "format": "pf_dxt5",
            "miptailbaseidx": 8,
            "busecrunchcompression": True,
            "lodgroup": "texturegroup_weaponspecular",
        },
        "sourceExtractedDdsRelativePath": "WP_WGDH_01/Texture2D/wp_wgdh_01s_s.dds",
        "dds": {
            "byteCount": 65664,
            "rawSha256": "add721e576b2b81479e13ba6f9e9ca3e7718a225a7aecfe07f02ca4adbdb2df9",
            "width": 256,
            "height": 256,
            "fourCC": "DXT5",
        },
    },
    {
        "logicalTexturePath": "fx_tex_high_03.fx_d_cloud_006",
        "familyId": "material-family-b53107e635922285",
        "parameterName": "01.map",
        "effectiveOverrideValue": "fx_tex_05.fx_m_smokesq_01",
        "logicalPackage": "FX_TEX_HIGH_03",
        "exportObjectPath": "fx_d_cloud_006",
        "exportIndex": 124,
        "serialOffset": 69428,
        "serialSize": 82143,
        "serialSha256": "b361d8cd2bd33f2821214e7ffd7c90f2a27979b2983377905d8075f242e5ad3a",
        "propertyStreamEnd": 333,
        "properties": {
            "sizex": 512,
            "sizey": 512,
            "originalsizex": 512,
            "originalsizey": 512,
            "format": "pf_dxt5",
            "miptailbaseidx": 9,
            "busecrunchcompression": True,
            "lodgroup": "texturegroup_effects",
            "lodbias": -1,
        },
        "sourceExtractedDdsRelativePath": "FX_TEX_HIGH_03/Texture2D/fx_d_cloud_006.dds",
        "dds": {
            "byteCount": 262272,
            "rawSha256": "a4c5e500c8fde3e778cce3f72a36873aae6290710ee372d879717da7c326148c",
            "width": 512,
            "height": 512,
            "fourCC": "DXT5",
        },
    },
]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def canonical_text_sha256(path: Path) -> str:
    text = path.read_text(encoding="utf-8").replace("\r\n", "\n").replace("\r", "\n")
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def parse_dds(raw: bytes, label: str) -> dict[str, Any]:
    require(len(raw) >= 128, f"DDS is truncated: {label}")
    require(raw[:4] == b"DDS ", f"DDS magic mismatch: {label}")
    header_size = struct.unpack_from("<I", raw, 4)[0]
    flags, height, width, pitch, depth, mip_count = struct.unpack_from("<6I", raw, 8)
    pixel_format_size, pixel_format_flags = struct.unpack_from("<2I", raw, 76)
    try:
        fourcc = raw[84:88].decode("ascii")
    except UnicodeDecodeError as exc:
        raise ValueError(f"DDS FourCC is not ASCII: {label}") from exc
    caps, caps2, caps3, caps4, reserved2 = struct.unpack_from("<5I", raw, 108)
    require(header_size == 124, f"DDS header size mismatch: {label}")
    require(pixel_format_size == 32, f"DDS pixel format size mismatch: {label}")
    require((pixel_format_flags & 0x4) != 0, f"DDS is not FourCC-compressed: {label}")
    require(fourcc in {"DXT1", "DXT5", "ATI2"}, f"unsupported DDS FourCC: {label}:{fourcc}")
    require(depth in (0, 1) and caps2 == caps3 == caps4 == reserved2 == 0, f"DDS is not plain 2D: {label}")
    require(mip_count in (0, 1), f"fresh UModel DDS must contain only the top mip: {label}")
    block_bytes = 8 if fourcc == "DXT1" else 16
    payload_size = max(1, (width + 3) // 4) * max(1, (height + 3) // 4) * block_bytes
    require(len(raw) == 128 + payload_size, f"DDS payload size mismatch: {label}")
    require(pitch == payload_size, f"DDS linear size mismatch: {label}")
    return {
        "byteCount": len(raw),
        "rawSha256": hashlib.sha256(raw).hexdigest(),
        "header128Sha256": hashlib.sha256(raw[:128]).hexdigest(),
        "magic": "DDS ",
        "headerSize": header_size,
        "flagsHex": f"0x{flags:08x}",
        "width": width,
        "height": height,
        "pitchOrLinearSize": pitch,
        "pixelFormatFlagsHex": f"0x{pixel_format_flags:08x}",
        "fourCC": fourcc,
        "rawMipMapCount": mip_count,
        "capsHex": f"0x{caps:08x}",
        "payloadByteCount": payload_size,
        "payloadScope": "UMODEL_DECODED_TOP_MIP_EXPORT",
    }


def find_native_lookup(native_receipt: dict[str, Any], expected: dict[str, Any]) -> dict[str, Any]:
    logical = expected["logicalTexturePath"].casefold()
    family = next(
        (row for row in native_receipt.get("families", []) if row.get("familyId") == expected["familyId"]),
        None,
    )
    require(family is not None, f"native lookup family is missing: {expected['familyId']}")
    lookup = next(
        (
            row
            for row in family["source"].get("legacyTextureLookups", [])
            if str(row.get("textureObjectPath") or "").casefold() == logical
        ),
        None,
    )
    require(lookup is not None, f"native lookup is missing: {logical}")
    diagnostic = next(
        (
            row
            for row in native_receipt["textureClosureDiagnostics"].get("diagnostics", [])
            if str(row.get("logicalTexturePath") or "").casefold() == logical
        ),
        None,
    )
    require(diagnostic is not None, f"native lookup diagnostic is missing: {logical}")
    require(
        diagnostic.get("status") == "SOURCE_EXACT_BASE_DEFAULT_REPLACED_BY_ACTIVE_MIC",
        f"base lookup is no longer replaced: {logical}",
    )
    resolutions = diagnostic.get("legacyTextureLookupResolutions") or []
    require(len(resolutions) == 1, f"lookup precedence is ambiguous: {logical}")
    resolution = resolutions[0]
    require(
        resolution.get("parameterNames") == [expected["parameterName"]]
        and resolution.get("effectiveOverrideValues") == [expected["effectiveOverrideValue"]]
        and resolution.get("baseDefaultOverriddenInAllActiveRecipes") is True
        and resolution.get("overridePreservesBaseLogicalTexture") is False
        and resolution.get("effectiveOverrideValuesRuntimeBound") is True,
        f"MIC override precedence changed: {logical}",
    )
    return {
        "familyId": expected["familyId"],
        "legacyLookup": lookup,
        "parameterName": expected["parameterName"],
        "activeRecipeResolutions": resolution["activeRecipes"],
        "effectiveOverrideValue": expected["effectiveOverrideValue"],
        "status": diagnostic["status"],
        "baseDefaultEffectiveForActiveArtist31470": False,
    }


def source_texture_evidence(package: Any, expected: dict[str, Any]) -> dict[str, Any]:
    export_index = expected["exportIndex"]
    require(0 <= export_index < len(package.exports), "Texture2D export index is invalid")
    entry = package.exports[export_index]
    object_path = package_ref_path(entry.index + 1, package.imports, package.exports)
    class_name = package_ref_name(entry.class_index, package.imports, package.exports).casefold()
    require(class_name == "texture2d", f"export is not Texture2D: {object_path}")
    require(object_path.casefold() == expected["exportObjectPath"].casefold(), "Texture2D path changed")
    require(entry.serial_offset == expected["serialOffset"], "Texture2D serial offset changed")
    require(entry.serial_size == expected["serialSize"], "Texture2D serial size changed")
    serial = package.logical[entry.serial_offset : entry.serial_offset + entry.serial_size]
    require(hashlib.sha256(serial).hexdigest() == expected["serialSha256"], "Texture2D serial SHA changed")
    properties, property_end = parse_tagged_properties(serial, package.names, package.summary.version)
    require(property_end == expected["propertyStreamEnd"], "Texture2D property boundary changed")
    selected = {}
    for name, value in expected["properties"].items():
        require(name in properties, f"Texture2D property is missing: {object_path}:{name}")
        actual = properties[name].get("value")
        require(actual == value, f"Texture2D property changed: {object_path}:{name}")
        selected[name] = actual
    return {
        "objectPath": object_path,
        "className": class_name,
        "exportIndex": entry.index,
        "packageReference": entry.index + 1,
        "serialOffset": entry.serial_offset,
        "serialSize": entry.serial_size,
        "serialSha256": expected["serialSha256"],
        "propertyStreamEnd": property_end,
        "selectedProperties": selected,
    }


def expected_asset_projection(row: dict[str, Any]) -> dict[str, Any]:
    return {
        "logicalTexturePath": row["logicalTexturePath"],
        "familyId": row["familyId"],
        "parameterName": row["parameterName"],
        "effectiveOverrideValue": row["effectiveOverrideValue"],
        "logicalPackage": row["logicalPackage"],
        "exportObjectPath": row["exportObjectPath"],
        "exportIndex": row["exportIndex"],
        "serialSha256": row["serialSha256"],
        "sourceExtractedDdsRelativePath": row["sourceExtractedDdsRelativePath"],
        "dds": row["dds"],
    }


def receipt_asset_projection(row: dict[str, Any]) -> dict[str, Any]:
    dds = row["dds"]
    source = row["sourceTexture2D"]
    return {
        "logicalTexturePath": row["logicalTexturePath"],
        "familyId": row["baseLookupEvidence"]["familyId"],
        "parameterName": row["baseLookupEvidence"]["parameterName"],
        "effectiveOverrideValue": row["baseLookupEvidence"]["effectiveOverrideValue"],
        "logicalPackage": row["logicalPackage"],
        "exportObjectPath": source["objectPath"],
        "exportIndex": source["exportIndex"],
        "serialSha256": source["serialSha256"],
        "sourceExtractedDdsRelativePath": row["sourceExtractedDdsRelativePath"],
        "dds": {
            "byteCount": dds["byteCount"],
            "rawSha256": dds["rawSha256"],
            "width": dds["width"],
            "height": dds["height"],
            "fourCC": dds["fourCC"],
        },
    }


def build_receipt(
    source_package_root: Path,
    resource_root: Path,
    extraction_root: Path,
    native_receipt_path: Path,
) -> dict[str, Any]:
    native_receipt = read_json(native_receipt_path)
    require(
        native_receipt.get("schema") == "lostark.artist-31470-material-native-resource-receipt",
        "native Material receipt schema changed",
    )
    umodel_path = resource_root / UMODEL_RELATIVE_PATH
    require(umodel_path.is_file(), "pinned UModel executable is missing")
    require(umodel_path.stat().st_size == EXPECTED_UMODEL["byteCount"], "UModel byte count changed")
    require(raw_file_sha256(umodel_path) == EXPECTED_UMODEL["rawSha256"], "UModel SHA changed")

    package_cache = {}
    package_rows = []
    for logical_package, expected in EXPECTED_PACKAGES.items():
        path = source_package_root / expected["fileName"]
        require(path.is_file(), f"source texture package is missing: {expected['fileName']}")
        require(path.stat().st_size == expected["byteCount"], "source texture package byte count changed")
        require(raw_file_sha256(path) == expected["rawSha256"], "source texture package SHA changed")
        package = load_package(path, LOSTARK_KR_AES_KEY)
        require(package.summary.version == expected["packageVersion"], "source package version changed")
        require(package.summary.licensee_version == expected["licenseeVersion"], "source package licensee version changed")
        require(len(package.exports) == expected["exportCount"], "source package export count changed")
        package_cache[logical_package] = package
        package_rows.append({"logicalPackage": logical_package, **expected})

    assets = []
    for expected in EXPECTED_ASSETS:
        dds_path = extraction_root / expected["sourceExtractedDdsRelativePath"]
        require(dds_path.is_file(), f"fresh extracted DDS is missing: {dds_path}")
        dds = parse_dds(dds_path.read_bytes(), expected["logicalTexturePath"])
        for field, value in expected["dds"].items():
            require(dds[field] == value, f"fresh DDS changed: {expected['logicalTexturePath']}:{field}")
        base_lookup = find_native_lookup(native_receipt, expected)
        texture = source_texture_evidence(package_cache[expected["logicalPackage"]], expected)
        assets.append(
            {
                "logicalTexturePath": expected["logicalTexturePath"],
                "logicalPackage": expected["logicalPackage"],
                "baseLookupEvidence": base_lookup,
                "sourceTexture2D": texture,
                "sourceExtractedDdsRelativePath": expected["sourceExtractedDdsRelativePath"],
                "copyPolicy": "FRESH_UMODEL_DDS_NO_REENCODE_EXTERNAL_EVIDENCE_ONLY",
                "dds": dds,
                "runtimeAssetId": None,
                "runtimeBindingAdmission": False,
                "productAdmission": False,
            }
        )

    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "status": "SOURCE_EXTRACTED_BASE_DEFAULT_ONLY",
        "scope": "EXTERNAL_EVIDENCE_STAGING_NOT_RUNTIME_DEPLOYMENT",
        "sourceEvidence": {
            "nativeMaterialReceipt": {
                "path": native_receipt_path.relative_to(REPO_ROOT).as_posix(),
                "receiptSha256": native_receipt["receiptSha256"],
                "canonicalJsonSha256": canonical_json_sha256(native_receipt),
            },
            "extractor": {
                "sourceRootId": "Resource_LostArk",
                "relativePath": UMODEL_RELATIVE_PATH.as_posix(),
                **EXPECTED_UMODEL,
            },
            "sourcePackages": package_rows,
        },
        "localBindings": {
            "sourcePackageRootId": "LostArkInstallPackages",
            "resourceRootId": "Resource_LostArk",
            "extractionRelativeRoot": EXTRACTION_RELATIVE_ROOT.as_posix(),
            "trackingPolicy": "EXTERNAL_READ_ONLY_RECONSTRUCTION_EVIDENCE",
        },
        "extractionInvocations": [
            {
                "logicalPackage": "WP_WGDH_01",
                "argv": [
                    "{umodel}", "-export", "-game=lostark", "-kr", "-nameresolve",
                    "-path={sourcePackageRoot}", "-out={extractionRoot}", "-dds", "-nooverwrite",
                    "-obj=wp_wgdh_01s_d", "-obj=wp_wgdh_01s_n", "-obj=wp_wgdh_01s_s", "WP_WGDH_01",
                ],
                "foundPhysicalPackage": EXPECTED_PACKAGES["WP_WGDH_01"]["fileName"],
                "foundObjectCount": 3,
                "exitCode": 0,
            },
            {
                "logicalPackage": "FX_TEX_HIGH_03",
                "argv": [
                    "{umodel}", "-export", "-game=lostark", "-kr", "-nameresolve",
                    "-path={sourcePackageRoot}", "-out={extractionRoot}", "-dds", "-nooverwrite",
                    "-obj=fx_d_cloud_006", "FX_TEX_HIGH_03",
                ],
                "foundPhysicalPackage": EXPECTED_PACKAGES["FX_TEX_HIGH_03"]["fileName"],
                "foundObjectCount": 1,
                "exitCode": 0,
            },
        ],
        "toolDependencies": [
            {
                "path": SCRIPT_PATH.relative_to(REPO_ROOT).as_posix(),
                "sha256": canonical_text_sha256(SCRIPT_PATH),
                "hashRole": "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
            }
        ],
        "assets": assets,
        "decision": {
            "sourceExactBaseDefaultTextureEvidence": True,
            "activeArtist31470BaseDefaultEffective": False,
            "runtimeDeploymentAdmission": False,
            "runtimeBindingAdmission": False,
            "shaderGraphAdmission": False,
            "productAdmission": False,
            "reason": "ALL_FOUR_BASE_DEFAULTS_REPLACED_BY_SOURCE_EXACT_ACTIVE_MIC_OVERRIDES",
        },
        "summary": {
            "sourcePackageCount": 2,
            "assetCount": 4,
            "sourceExactTexture2DCount": 4,
            "freshDdsVerifiedCount": 4,
            "baseDefaultReplacedByActiveMicCount": 4,
            "runtimeDeploymentAdmittedCount": 0,
            "runtimeBindingAdmittedCount": 0,
            "productAdmittedCount": 0,
        },
    }
    receipt["receiptSha256"] = canonical_json_sha256(receipt)
    return receipt


def validate_receipt(receipt: dict[str, Any], native_receipt_path: Path) -> None:
    require(receipt.get("schema") == SCHEMA, "native base texture receipt schema changed")
    require(receipt.get("formatVersion") == FORMAT_VERSION, "native base texture receipt version changed")
    sealed = dict(receipt)
    claimed = sealed.pop("receiptSha256", None)
    require(claimed == canonical_json_sha256(sealed), "native base texture receipt digest changed")
    native_receipt = read_json(native_receipt_path)
    native_dependency = receipt.get("sourceEvidence", {}).get("nativeMaterialReceipt", {})
    require(
        native_dependency == {
            "path": native_receipt_path.relative_to(REPO_ROOT).as_posix(),
            "receiptSha256": native_receipt["receiptSha256"],
            "canonicalJsonSha256": canonical_json_sha256(native_receipt),
        },
        "native Material receipt dependency changed",
    )
    require(receipt.get("sourceEvidence", {}).get("extractor") == {
        "sourceRootId": "Resource_LostArk",
        "relativePath": UMODEL_RELATIVE_PATH.as_posix(),
        **EXPECTED_UMODEL,
    }, "native base texture extractor identity changed")
    require(receipt.get("summary") == {
        "sourcePackageCount": 2,
        "assetCount": 4,
        "sourceExactTexture2DCount": 4,
        "freshDdsVerifiedCount": 4,
        "baseDefaultReplacedByActiveMicCount": 4,
        "runtimeDeploymentAdmittedCount": 0,
        "runtimeBindingAdmittedCount": 0,
        "productAdmittedCount": 0,
    }, "native base texture denominator changed")
    require(
        [receipt_asset_projection(row) for row in receipt.get("assets", [])]
        == [expected_asset_projection(row) for row in EXPECTED_ASSETS],
        "native base texture asset projection changed",
    )
    for row in receipt.get("assets", []):
        require(row.get("runtimeAssetId") is None, "base default received a runtime asset ID")
        require(row.get("runtimeBindingAdmission") is False, "base default runtime binding opened")
        require(row.get("productAdmission") is False, "base default Product admission opened")
        require(
            row.get("baseLookupEvidence", {}).get("status")
            == "SOURCE_EXACT_BASE_DEFAULT_REPLACED_BY_ACTIVE_MIC",
            "base default replacement boundary changed",
        )
    decision = receipt.get("decision") or {}
    require(
        decision.get("sourceExactBaseDefaultTextureEvidence") is True
        and decision.get("activeArtist31470BaseDefaultEffective") is False
        and decision.get("runtimeDeploymentAdmission") is False
        and decision.get("runtimeBindingAdmission") is False
        and decision.get("shaderGraphAdmission") is False
        and decision.get("productAdmission") is False,
        "native base texture admission boundary changed",
    )
    dependencies = receipt.get("toolDependencies") or []
    require(dependencies == [{
        "path": SCRIPT_PATH.relative_to(REPO_ROOT).as_posix(),
        "sha256": canonical_text_sha256(SCRIPT_PATH),
        "hashRole": "TRACKED_SOURCE_EOL_CANONICAL_TEXT",
    }], "native base texture tool identity changed")


def write_json(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-package-root", type=Path, default=DEFAULT_SOURCE_PACKAGE_ROOT)
    parser.add_argument("--resource-root", type=Path, default=DEFAULT_RESOURCE_ROOT)
    parser.add_argument("--extraction-root", type=Path)
    parser.add_argument("--native-receipt", type=Path, default=DEFAULT_NATIVE_RECEIPT)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--validate-only", action="store_true")
    args = parser.parse_args()
    extraction_root = args.extraction_root or (args.resource_root / EXTRACTION_RELATIVE_ROOT)
    if args.validate_only:
        validate_receipt(read_json(args.output), args.native_receipt)
    else:
        receipt = build_receipt(
            args.source_package_root,
            args.resource_root,
            extraction_root,
            args.native_receipt,
        )
        validate_receipt(receipt, args.native_receipt)
        if args.check:
            require(read_json(args.output) == receipt, "native base texture extraction receipt is stale")
        else:
            write_json(args.output, receipt)
    print(
        "Artist F native base texture extraction: packages=2 assets=4 fresh-dds=4 "
        "active-effective=0 runtime-binding=0 product=0"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
