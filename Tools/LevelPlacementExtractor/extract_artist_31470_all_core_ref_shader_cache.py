#!/usr/bin/env python3
"""Acquire Artist 31470 core material maps without profile-number fallbacks.

The original #9/#10/#11 extractor remains the frozen, deeply decoded oracle.
This sibling expands only the manifest-975 same-cohort denominator.  Every
result is joined by the native FStaticParameterSet equality projection and is
then keyed per occurrence by recipe, renderer and vertex-factory type.  A
missing or duplicated map/VF/pass is an error; ScreenPost is deliberately
recorded without inventing a VF or executable pass.

This tool acquires material-map bytes and decodes the ShaderCache descriptor's
packed code-blob index plus DWORD offset/size slice.  It proves that every
descriptor in both pinned caches exactly partitions every uncompressed code
blob, then validates every shader reference reachable from the target maps as
one complete DXBC container.  The frozen #9/#10/#11 oracle remains an
independent hash corroboration.  Exact DXBC identity still does not admit an
occurrence's actual VF/pass, shader-object constant bindings, render state or
product runtime.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
from pathlib import Path
from typing import Any

from derive_artist_31470_main_shader_map_identity import (
    engine_equivalent_static_parameter_set,
    normalized_static_parameter_set,
)
from extract_artist_31470_main_ref_shader_cache import (
    BufferedLogicalCursor,
    EXPECTED_INSTALLED_CACHE,
    EXPECTED_OFFICIAL_CACHE,
    digest_file,
    package_tables,
    parse_cache_code_index,
    read_fname_at,
    read_fstring_at,
    require,
    sha256_bytes,
    validate_cache_identity,
)
from extract_artist_31470_shader_cache_oracle import (
    canonical_json_sha256,
    parse_static_parameter_set,
    read_json,
    validate_dxbc_container,
)
from extract_ue3_placements import LostArkPackageRangeReader, decompress_lz4_block


SCHEMA = "lostark.artist-31470-all-core-ref-shader-cache-receipt"
FORMAT_VERSION = 2
TARGET_SCHEMA = "lostark.artist-31470-all-core-ref-shader-targets"
PACKED_CODE_INDEX_BITS = 18
PACKED_CODE_INDEX_MASK = (1 << PACKED_CODE_INDEX_BITS) - 1
PACKED_DWORD_BYTE_SIZE = 4
REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = Path(__file__).resolve()
DEFAULT_TARGETS = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.all-core-ref-shader-targets.json"
)
DEFAULT_OUTPUT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.all-core-ref-shader-cache.receipt.json"
)
DEFAULT_EVIDENCE_ROOT = Path(
    r"C:\Users\user\Desktop\Resource_LostArk\01_Extracted\Effect\ARTIST\31470_TrackA_20260812\OfficialRefShaderCacheV974"
)
DEFAULT_OFFICIAL_MANIFEST = DEFAULT_EVIDENCE_ROOT / "45_975.json"
DEFAULT_OFFICIAL_CACHE = (
    DEFAULT_EVIDENCE_ROOT / "EV2LG3OVEH3HGV7THTFFTM7TOKMCC.v974.upk"
)
DEFAULT_INSTALLED_ROOT = Path(
    r"C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC"
)
DEFAULT_INSTALLED_CACHE = (
    DEFAULT_INSTALLED_ROOT / "EV2LG3OVEH3HGV7THTFFTM7TOKMCC.upk"
)


def public_static_set(value: dict[str, Any]) -> dict[str, Any]:
    result = dict(value)
    result.pop("endOffset", None)
    return result


def seal(receipt: dict[str, Any]) -> None:
    receipt.pop("receiptSha256", None)
    receipt["receiptSha256"] = canonical_json_sha256(receipt)


def validate_seal(document: dict[str, Any], expected_sha256: str, label: str) -> None:
    actual_seal = document.get("receiptSha256")
    require(actual_seal == expected_sha256, f"{label} pinned receipt SHA changed")
    payload = dict(document)
    payload.pop("receiptSha256", None)
    require(canonical_json_sha256(payload) == actual_seal, f"{label} receipt seal is invalid")


def repo_file_identity(path: Path) -> dict[str, Any]:
    return {
        "repoRelativePath": path.relative_to(REPO_ROOT).as_posix(),
        "rawSha256": digest_file(path),
    }


def decode_packed_shader_code_slice(
    raw_descriptor: bytes,
    code_positions: list[dict[str, int]],
) -> dict[str, Any]:
    """Decode Lost Ark's packed ShaderCache descriptor-to-DXBC slice.

    The 8-byte descriptor tail is not a plain u32 code index.  EFEngine packs
    a blob index and DWORD slice offset into the first word, then a DWORD slice
    size into the high bits of the second word::

        word0[17:0]  = compressed-code blob index
        word0[31:18] = uncompressed blob slice offset / 4
        word1[17:0]  = reserved, always zero in both pinned caches
        word1[31:18] = uncompressed blob slice size / 4

    One compressed blob may therefore contain several adjacent DXBC
    containers.  Returning the whole blob for one shader is incorrect.
    """

    require(len(raw_descriptor) == 24, "ShaderCache descriptor byte size changed")
    packed_index_and_offset, packed_size = struct.unpack_from(
        "<II", raw_descriptor, 16
    )
    code_blob_index = packed_index_and_offset & PACKED_CODE_INDEX_MASK
    slice_offset = (
        packed_index_and_offset >> PACKED_CODE_INDEX_BITS
    ) * PACKED_DWORD_BYTE_SIZE
    reserved = packed_size & PACKED_CODE_INDEX_MASK
    slice_size = (packed_size >> PACKED_CODE_INDEX_BITS) * PACKED_DWORD_BYTE_SIZE
    require(reserved == 0, "packed ShaderCache descriptor reserved bits changed")
    require(
        code_blob_index < len(code_positions),
        "packed ShaderCache code-blob index is out of range",
    )
    require(slice_size > 0, "packed ShaderCache DXBC slice is empty")
    code_position = code_positions[code_blob_index]
    require(
        slice_offset + slice_size <= code_position["uncompressedByteSize"],
        "packed ShaderCache DXBC slice exceeds its code blob",
    )
    return {
        "codeBlobIndex": code_blob_index,
        "sliceOffsetInUncompressedBlob": slice_offset,
        "sliceByteSize": slice_size,
        "reservedLow18Bits": reserved,
        "codeBlobPosition": code_position,
    }


def parse_manifest_package_rows(
    manifest_path: Path,
    targets_document: dict[str, Any],
) -> dict[str, dict[str, Any]]:
    expected = targets_document["inputs"]["officialManifest"]
    require(manifest_path.is_file(), f"official manifest is missing: {manifest_path}")
    require(manifest_path.name == expected["fileName"], "official manifest name changed")
    require(digest_file(manifest_path) == expected["rawSha256"], "official manifest SHA changed")
    manifest = read_json(manifest_path)
    require(str(manifest.get("service_code")) == expected["serviceCode"], "official manifest service changed")
    require(manifest.get("version_no") == expected["versionNo"], "official manifest version changed")
    rows = manifest.get("files")
    require(isinstance(rows, list), "official manifest file rows are absent")
    wanted: dict[str, dict[str, Any]] = {}
    for target in targets_document["targets"]:
        source = target["sourcePackage"]
        leaf = source["fileName"].casefold()
        if leaf in wanted:
            require(wanted[leaf] == source, f"one source package name maps to multiple identities: {leaf}")
        else:
            wanted[leaf] = source
    found: dict[str, dict[str, Any]] = {}
    for raw in rows:
        if not isinstance(raw, str):
            continue
        fields = [field.strip() for field in raw.split("|")]
        if len(fields) < 11:
            continue
        leaf = Path(fields[2]).name.casefold()
        if leaf not in wanted:
            continue
        require(leaf not in found, f"official manifest package row is duplicated: {leaf}")
        source = wanted[leaf]
        parsed = {
            "fileName": Path(fields[2]).name,
            "rawRowSha256": sha256_bytes(raw.encode("utf-8")),
            "path": fields[2],
            "fileVersion": int(fields[3]),
            "sequence": int(fields[4]),
            "payloadKind": fields[5],
            "extractedByteSize": int(fields[6]),
            "packedByteSize": int(fields[7]),
            "extractedMd5": fields[8].casefold(),
            "packedMd5": fields[9].casefold(),
        }
        require(parsed["fileVersion"] == source["manifestFileVersion"], f"official source package version changed: {leaf}")
        require(parsed["sequence"] == source["manifestSequence"], f"official source package sequence changed: {leaf}")
        require(parsed["extractedMd5"] == source["manifestExtractedMd5"], f"official source package MD5 changed: {leaf}")
        found[leaf] = parsed
    require(set(found) == set(wanted), "official manifest source-package denominator changed")
    return found


def validate_source_packages(
    installed_root: Path,
    targets_document: dict[str, Any],
    manifest_rows: dict[str, dict[str, Any]],
) -> list[dict[str, Any]]:
    result = []
    unique_sources: dict[str, dict[str, Any]] = {}
    for target in targets_document["targets"]:
        source = target["sourcePackage"]
        leaf = source["fileName"].casefold()
        if leaf in unique_sources:
            require(unique_sources[leaf] == source, f"one source package name maps to multiple identities: {leaf}")
        else:
            unique_sources[leaf] = source
    for leaf in sorted(unique_sources):
        source = unique_sources[leaf]
        path = installed_root / "Packages" / source["fileName"]
        require(path.is_file(), f"same-cohort source package is missing: {path}")
        require(digest_file(path) == source["rawSha256"], f"installed source package SHA changed: {leaf}")
        manifest_row = manifest_rows[leaf]
        require(path.stat().st_size == manifest_row["extractedByteSize"], f"installed source package size differs from official manifest: {leaf}")
        require(digest_file(path, "md5") == manifest_row["extractedMd5"], f"installed source package MD5 differs from official manifest: {leaf}")
        result.append(
            {
                "fileName": source["fileName"],
                "physicalByteSize": path.stat().st_size,
                "rawSha256": source["rawSha256"],
                "officialManifest": manifest_row,
                "cohortJoin": "INSTALLED_SOURCE_BYTES_EQUAL_OFFICIAL_MANIFEST_975_EXTRACTED_PAYLOAD",
            }
        )
    return result


def validate_target_joins(
    targets_document: dict[str, Any],
    material_contract: dict[str, Any],
    native_receipt: dict[str, Any],
) -> list[dict[str, Any]]:
    require(targets_document.get("schema") == TARGET_SCHEMA, "target manifest schema changed")
    require(targets_document.get("formatVersion") == 1, "target manifest version changed")
    contract_input = targets_document["inputs"]["typedMaterialContract"]
    native_input = targets_document["inputs"]["nativeMaterialReceipt"]
    require(material_contract.get("schema") == contract_input["schema"], "typed material contract schema changed")
    require(native_receipt.get("schema") == native_input["schema"], "native material receipt schema changed")

    targets = targets_document.get("targets")
    policies = targets_document.get("vertexFactoryPolicies")
    require(isinstance(targets, list) and isinstance(policies, dict), "target manifest body is invalid")
    summary = targets_document["summary"]
    occurrence_ids = [occurrence for target in targets for occurrence in target["occurrenceIds"]]
    require(len(targets) == summary["targetRecipeCount"] == 18, "all-core recipe denominator changed")
    require(len(set(target["familyId"] for target in targets)) == summary["targetFamilyCount"] == 15, "all-core family denominator changed")
    require(len(occurrence_ids) == len(set(occurrence_ids)) == summary["targetOccurrenceCount"] == 22, "all-core occurrence denominator changed")
    require(len({target["recipeId"] for target in targets}) == len(targets), "target recipe is duplicated")

    occurrences = {row["occurrenceId"]: row for row in material_contract["occurrences"]}
    native_rows = {row["recipeId"]: row for row in native_receipt["recipeNativeKeys"]}
    require(len(native_rows) == len(native_receipt["recipeNativeKeys"]), "native receipt recipe is duplicated")
    result = []
    renderer_counts: dict[str, int] = {renderer: 0 for renderer in policies}
    for target in targets:
        recipe_id = target["recipeId"]
        renderer_type = target["rendererType"]
        require(renderer_type in policies, f"target renderer policy is absent: {renderer_type}")
        native = native_rows.get(recipe_id)
        require(native is not None, f"target native recipe is absent: {recipe_id}")
        static_set = native.get("staticParameterSet")
        require(isinstance(static_set, dict) and static_set, f"target static parameter set is absent: {recipe_id}")
        require(native["arithmeticFamilyId"] == target["familyId"], f"target family join changed: {recipe_id}")
        require(native["sourceMaterialPath"] == target["sourceMaterialPath"], f"target material path changed: {recipe_id}")
        require(native["physicalPackage"] == target["sourcePackage"]["fileName"], f"target package name changed: {recipe_id}")
        require(native["physicalPackageSha256"] == target["sourcePackage"]["rawSha256"], f"target package SHA changed: {recipe_id}")
        require(static_set["baseMaterialIdHex"] == target["baseMaterialIdHex"], f"target base material ID changed: {recipe_id}")
        require(static_set["semanticSha256"] == target["staticParameterSetSemanticSha256"], f"target serialized static-set semantics changed: {recipe_id}")
        normalized = normalized_static_parameter_set(static_set)
        engine_equality = engine_equivalent_static_parameter_set(static_set)
        normalized_sha = canonical_json_sha256(normalized)
        engine_equality_sha = canonical_json_sha256(engine_equality)
        for occurrence_id in target["occurrenceIds"]:
            occurrence = occurrences.get(occurrence_id)
            require(occurrence is not None, f"target occurrence is absent: {occurrence_id}")
            require(occurrence["materialRecipeId"] == recipe_id, f"target occurrence recipe changed: {occurrence_id}")
            require(occurrence["rendererType"] == renderer_type, f"target occurrence renderer changed: {occurrence_id}")
            require(occurrence["sourceMaterialPath"] == target["sourceMaterialPath"], f"target occurrence material changed: {occurrence_id}")
            renderer_counts[renderer_type] += 1
        result.append(
            {
                **target,
                "normalizedStaticParameterSet": normalized,
                "normalizedStaticParameterSetSha256": normalized_sha,
                "engineEqualityStaticParameterSet": engine_equality,
                "engineEqualityStaticParameterSetSha256": engine_equality_sha,
                "vertexFactoryPolicy": policies[renderer_type],
            }
        )
    require(renderer_counts == {"MeshParticle": 10, "SpriteParticle": 11, "ScreenPost": 1}, "target renderer denominator changed")
    require(
        len({target["engineEqualityStaticParameterSetSha256"] for target in result})
        == summary["targetEngineEqualityStaticParameterSetCount"]
        == 16,
        "target engine-equality static-set denominator changed",
    )
    require(
        all(not bool(policy["registryAdmission"]) for policy in policies.values()),
        "cache acquisition must not admit an actual occurrence VF/pass",
    )
    return result


def load_exact_code_oracle(
    targets_document: dict[str, Any],
) -> tuple[Path, dict[tuple[str, str, str], dict[str, Any]]]:
    oracle_input = targets_document["inputs"]["exactCodeOracle"]
    oracle_path = REPO_ROOT / oracle_input["repoRelativePath"]
    require(oracle_path.is_file(), f"exact code oracle is missing: {oracle_path}")
    oracle = read_json(oracle_path)
    require(oracle.get("schema") == oracle_input["schema"], "exact code oracle schema changed")
    require(oracle.get("formatVersion") == oracle_input["formatVersion"], "exact code oracle version changed")
    validate_seal(oracle, oracle_input["receiptSha256"], "exact code oracle")

    side_rows: dict[str, dict[tuple[str, str, str], dict[str, Any]]] = {}
    pass_type = targets_document["vertexFactoryPolicies"]["MeshParticle"][
        "passPixelShaderType"
    ]
    for side_name in ("officialRefShaderCacheV974", "currentInstalledRefShaderCache"):
        rows: dict[tuple[str, str, str], dict[str, Any]] = {}
        for target in oracle[side_name]["mainTargets"]:
            recipe_id = target["recipeId"]
            selected = [
                row
                for row in target["materialMap"]["selectedOriginalDxbc"]
                if row["shaderType"] == pass_type
            ]
            require(len(selected) == 1, f"exact code oracle pixel pass is ambiguous: {recipe_id}")
            row = selected[0]
            key = (recipe_id, row["shaderType"], row["shaderIdHex"])
            require(key not in rows, f"exact code oracle key is duplicated: {recipe_id}")
            rows[key] = {
                "dxbcSha256": row["dxbcSha256"],
                "byteSize": row["uncompressedByteSize"],
            }
        side_rows[side_name] = rows

    require(
        side_rows["officialRefShaderCacheV974"] == side_rows["currentInstalledRefShaderCache"],
        "exact code oracle official/current pixel evidence differs",
    )
    exact_rows = side_rows["officialRefShaderCacheV974"]
    require(len(exact_rows) == 2, "exact code oracle recipe denominator changed")
    require(
        {key[0] for key in exact_rows}
        == {"material-recipe-03cc03b86c1a4c8f", "material-recipe-daf220acad2b656e"},
        "exact code oracle recipe identity changed",
    )
    return oracle_path, exact_rows


def parse_packed_shader_code_index(
    package: dict[str, Any],
    legacy_index: dict[str, Any],
) -> dict[str, Any]:
    """Reparse the code section and prove every descriptor slice partition.

    ``parse_cache_code_index`` intentionally preserves the old plain-u32
    candidate boundary used by the frozen main extractor.  This parser does
    not mutate that oracle.  It independently decodes the packed layout and
    requires every descriptor in every shader-type group to form a complete,
    gap-free, non-overlapping partition of every uncompressed code blob.
    """

    reader: LostArkPackageRangeReader = package["reader"]
    names: list[str] = package["names"]
    export = package["export"]
    cursor = BufferedLogicalCursor(reader, export.serial_offset)
    require(cursor.i32() == -1, "RefShaderCache net index changed")
    property_name, property_number = cursor.fname(names)
    require(
        property_name.casefold() == "none" and property_number == 0,
        "RefShaderCache property terminator changed",
    )
    require(cursor.u32() == 0, "RefShaderCache native revision changed")
    platform = cursor.read(1)[0]
    require(platform == legacy_index["platform"], "packed parser platform differs")
    group_count = cursor.u32()
    require(
        group_count == legacy_index["groupCount"],
        "packed parser group denominator differs",
    )

    descriptor_by_id: dict[str, dict[str, Any]] = {}
    groups = []
    descriptor_projection_digest = hashlib.sha256()
    partition_projection_digest = hashlib.sha256()
    total_descriptors = 0
    total_blobs = 0
    total_slices = 0
    multi_slice_blob_count = 0
    maximum_slices_per_blob = 0

    for group_index in range(group_count):
        expected_group = legacy_index["groups"][group_index]
        group_offset = cursor.offset
        shader_type, shader_type_number = cursor.fname(names)
        require(shader_type_number == 0, "numbered shader type is unsupported")
        descriptor_count = cursor.u32()
        descriptor_offset = cursor.offset
        descriptor_bytes = cursor.read(descriptor_count * 24)
        code_count = cursor.u32()
        require(
            group_offset == expected_group["logicalOffset"]
            and shader_type == expected_group["shaderType"]
            and descriptor_count == expected_group["descriptorCount"]
            and code_count == expected_group["embeddedCodeCount"]
            and sha256_bytes(descriptor_bytes)
            == expected_group["descriptorTableSha256"],
            "packed parser group projection differs from frozen parser",
        )

        code_positions = []
        code_header_digest = hashlib.sha256()
        for code_blob_index in range(code_count):
            code_header_offset = cursor.offset
            uncompressed_size, compressed_size = struct.unpack("<II", cursor.read(8))
            require(
                32 <= uncompressed_size <= 64 * 1024 * 1024,
                "packed parser uncompressed code size is invalid",
            )
            require(
                0 < compressed_size <= reader.logical_size - cursor.offset,
                "packed parser compressed code size is invalid",
            )
            compressed_offset = cursor.offset
            cursor.skip(compressed_size)
            code_positions.append(
                {
                    "codeIndex": code_blob_index,
                    "codeHeaderLogicalOffset": code_header_offset,
                    "compressedLogicalOffset": compressed_offset,
                    "compressedByteSize": compressed_size,
                    "uncompressedByteSize": uncompressed_size,
                }
            )
            code_header_digest.update(
                struct.pack("<II", uncompressed_size, compressed_size)
            )
        require(
            code_header_digest.hexdigest()
            == expected_group["codeHeaderProjectionSha256"],
            "packed parser code-header projection differs from frozen parser",
        )

        descriptors_by_blob: list[list[dict[str, Any]]] = [
            [] for _ in range(code_count)
        ]
        offset_zero_count = 0
        for descriptor_index in range(descriptor_count):
            raw = descriptor_bytes[
                descriptor_index * 24 : (descriptor_index + 1) * 24
            ]
            shader_id = raw[:16].hex()
            decoded = decode_packed_shader_code_slice(raw, code_positions)
            legacy_descriptor = legacy_index["descriptorById"].get(shader_id)
            require(
                legacy_descriptor is not None
                and legacy_descriptor["groupIndex"] == group_index
                and legacy_descriptor["shaderType"] == shader_type
                and legacy_descriptor["descriptorIndex"] == descriptor_index
                and legacy_descriptor["descriptorLogicalOffset"]
                == descriptor_offset + descriptor_index * 24
                and legacy_descriptor["descriptorRawSha256"] == sha256_bytes(raw),
                "packed descriptor identity differs from frozen parser",
            )
            if legacy_descriptor["codePosition"] is not None:
                require(
                    legacy_descriptor["codePosition"]
                    == decoded["codeBlobPosition"],
                    "frozen zero-offset code candidate differs from packed blob",
                )
            if decoded["sliceOffsetInUncompressedBlob"] == 0:
                offset_zero_count += 1
            packed_index_and_offset, packed_size = struct.unpack_from("<II", raw, 16)
            descriptor = {
                "groupIndex": group_index,
                "shaderType": shader_type,
                "descriptorIndex": descriptor_index,
                "descriptorLogicalOffset": descriptor_offset + descriptor_index * 24,
                "descriptorRawSha256": sha256_bytes(raw),
                "packedIndexAndOffsetU32": packed_index_and_offset,
                "packedSizeU32": packed_size,
                **decoded,
            }
            require(
                shader_id not in descriptor_by_id,
                "packed ShaderCache descriptor ID is duplicated",
            )
            descriptor_by_id[shader_id] = descriptor
            descriptors_by_blob[decoded["codeBlobIndex"]].append(descriptor)
            descriptor_projection_digest.update(
                json.dumps(
                    {
                        "shaderIdHex": shader_id,
                        "shaderType": shader_type,
                        "groupIndex": group_index,
                        "descriptorIndex": descriptor_index,
                        "codeBlobIndex": decoded["codeBlobIndex"],
                        "sliceOffsetInUncompressedBlob": decoded[
                            "sliceOffsetInUncompressedBlob"
                        ],
                        "sliceByteSize": decoded["sliceByteSize"],
                    },
                    sort_keys=True,
                    separators=(",", ":"),
                ).encode("utf-8")
            )

        require(
            offset_zero_count == code_count == expected_group["mappedDescriptorCount"],
            "one zero-offset descriptor per code blob was not preserved",
        )
        group_multi_slice_count = 0
        group_maximum_slices = 0
        group_slice_count = 0
        for code_blob_index, descriptors in enumerate(descriptors_by_blob):
            require(descriptors, "packed ShaderCache code blob has no descriptor slice")
            unique_slices = sorted(
                {
                    (
                        row["sliceOffsetInUncompressedBlob"],
                        row["sliceByteSize"],
                    )
                    for row in descriptors
                }
            )
            require(
                len(unique_slices) == len(descriptors),
                "packed ShaderCache descriptor slice is duplicated",
            )
            cursor_in_blob = 0
            for slice_offset, slice_size in unique_slices:
                require(
                    slice_offset == cursor_in_blob,
                    "packed ShaderCache code-blob slices have a gap or overlap",
                )
                cursor_in_blob += slice_size
            require(
                cursor_in_blob == code_positions[code_blob_index]["uncompressedByteSize"],
                "packed ShaderCache slices do not cover the complete code blob",
            )
            partition = {
                "codeBlobIndex": code_blob_index,
                "uncompressedByteSize": cursor_in_blob,
                "sliceCount": len(unique_slices),
                "sliceOffsets": [row[0] for row in unique_slices],
                "sliceByteSizes": [row[1] for row in unique_slices],
            }
            partition["semanticSha256"] = canonical_json_sha256(partition)
            for descriptor in descriptors:
                descriptor["codeBlobSlicePartition"] = partition
            partition_projection_digest.update(
                json.dumps(
                    {
                        "groupIndex": group_index,
                        "shaderType": shader_type,
                        **partition,
                    },
                    sort_keys=True,
                    separators=(",", ":"),
                ).encode("utf-8")
            )
            group_slice_count += len(unique_slices)
            if len(unique_slices) > 1:
                group_multi_slice_count += 1
            group_maximum_slices = max(group_maximum_slices, len(unique_slices))

        groups.append(
            {
                "groupIndex": group_index,
                "shaderType": shader_type,
                "descriptorCount": descriptor_count,
                "codeBlobCount": code_count,
                "sliceCount": group_slice_count,
                "multiSliceBlobCount": group_multi_slice_count,
                "maximumSlicesPerBlob": group_maximum_slices,
                "allCodeBlobsExactlyPartitioned": True,
            }
        )
        total_descriptors += descriptor_count
        total_blobs += code_count
        total_slices += group_slice_count
        multi_slice_blob_count += group_multi_slice_count
        maximum_slices_per_blob = max(
            maximum_slices_per_blob, group_maximum_slices
        )

    require(
        cursor.offset == legacy_index["shaderCodeSectionEndLogicalOffset"],
        "packed parser code-section end differs from frozen parser",
    )
    require(
        total_descriptors == legacy_index["descriptorCount"]
        and total_blobs == legacy_index["embeddedCodeCount"]
        and total_slices == total_descriptors,
        "packed parser descriptor/blob/slice denominator differs",
    )
    return {
        "platform": platform,
        "groupCount": group_count,
        "shaderCodeSectionEndLogicalOffset": cursor.offset,
        "layout": {
            "descriptorByteSize": 24,
            "codeBlobIndexBitCount": PACKED_CODE_INDEX_BITS,
            "codeBlobIndexMaskHex": f"0x{PACKED_CODE_INDEX_MASK:08x}",
            "sliceOffsetUnitBytes": PACKED_DWORD_BYTE_SIZE,
            "sliceSizeUnitBytes": PACKED_DWORD_BYTE_SIZE,
            "secondWordReservedLowBitCount": PACKED_CODE_INDEX_BITS,
        },
        "descriptorCount": total_descriptors,
        "codeBlobCount": total_blobs,
        "sliceCount": total_slices,
        "exactlyPartitionedCodeBlobCount": total_blobs,
        "multiSliceCodeBlobCount": multi_slice_blob_count,
        "maximumSlicesPerBlob": maximum_slices_per_blob,
        "descriptorSliceProjectionSha256": descriptor_projection_digest.hexdigest(),
        "codeBlobPartitionProjectionSha256": partition_projection_digest.hexdigest(),
        "groups": groups,
        "descriptorById": descriptor_by_id,
    }


def scan_map_contexts(
    package: dict[str, Any],
    code_index: dict[str, Any],
    targets: list[dict[str, Any]],
) -> dict[str, dict[str, Any]]:
    reader: LostArkPackageRangeReader = package["reader"]
    names: list[str] = package["names"]
    target_equalities = {
        target["engineEqualityStaticParameterSetSha256"] for target in targets
    }
    require(len(target_equalities) == 16, "cache target equality denominator changed")
    targets_by_base: dict[str, list[dict[str, Any]]] = {}
    for target in targets:
        targets_by_base.setdefault(target["baseMaterialIdHex"], []).append(target)
    patterns = {base_id: bytes.fromhex(base_id) for base_id in targets_by_base}
    hits = {base_id: [] for base_id in patterns}
    start = code_index["shaderCodeSectionEndLogicalOffset"] + 8
    overlap = b""
    cursor = start
    while cursor < reader.logical_size:
        payload = reader.read_logical_range(cursor, min(8 * 1024 * 1024, reader.logical_size - cursor))
        combined = overlap + payload
        origin = cursor - len(overlap)
        for base_id, pattern in patterns.items():
            local = 0
            while True:
                found = combined.find(pattern, local)
                if found < 0:
                    break
                absolute = origin + found
                if absolute >= start:
                    hits[base_id].append(absolute)
                local = found + 1
        overlap = combined[-15:]
        cursor += len(payload)

    contexts: dict[str, list[dict[str, Any]]] = {
        target["engineEqualityStaticParameterSetSha256"]: [] for target in targets
    }
    raw_hit_counts: dict[str, int] = {}
    for base_id, offsets in hits.items():
        raw_hit_counts[base_id] = len(set(offsets))
        for absolute in sorted(set(offsets)):
            candidate = reader.read_logical_range(absolute, min(4096, reader.logical_size - absolute))
            try:
                static_set = parse_static_parameter_set(candidate, 0, names)
                equality_sha = canonical_json_sha256(
                    engine_equivalent_static_parameter_set(static_set)
                )
            except (KeyError, ValueError, struct.error):
                continue
            if equality_sha not in target_equalities or static_set["baseMaterialIdHex"] != base_id:
                continue
            if static_set["endOffset"] + 20 > len(candidate):
                continue
            suffix = list(struct.unpack_from("<IIIII", candidate, static_set["endOffset"]))
            if not (
                suffix[0] == 868
                and suffix[1] == 16
                and suffix[3] == 0
                and 0 < suffix[4] <= 64
                and absolute < suffix[2] <= reader.logical_size
            ):
                continue
            contexts[equality_sha].append(
                {
                    "logicalOffset": absolute,
                    "logicalEndOffset": suffix[2],
                    "vertexFactoryCount": suffix[4],
                    "staticParameterSetRawSha256": static_set["rawSha256"],
                }
            )

    result: dict[str, dict[str, Any]] = {}
    for target in targets:
        equality_sha = target["engineEqualityStaticParameterSetSha256"]
        candidates = contexts[equality_sha]
        require(len(candidates) == 1, f"material-map context is absent or ambiguous: {target['recipeId']}")
        result[target["recipeId"]] = {
            "baseMaterialIdRawHitCount": raw_hit_counts[target["baseMaterialIdHex"]],
            "engineEqualityStaticParameterSetSha256": equality_sha,
            "candidateCount": len(candidates),
            **candidates[0],
        }
    return result


def enumerate_dxbc_containers(payload: bytes) -> dict[str, Any]:
    containers = []
    invalid_markers = []
    cursor = 0
    while cursor < len(payload):
        marker = payload.find(b"DXBC", cursor)
        if marker < 0:
            break
        if marker + 32 > len(payload):
            invalid_markers.append(
                {"payloadOffset": marker, "reason": "TRUNCATED_DXBC_HEADER"}
            )
            cursor = marker + 4
            continue
        declared_size = struct.unpack_from("<I", payload, marker + 24)[0]
        if declared_size < 36 or marker + declared_size > len(payload):
            invalid_markers.append(
                {
                    "payloadOffset": marker,
                    "declaredByteSize": declared_size,
                    "reason": "INVALID_OR_OUT_OF_RANGE_DXBC_SIZE",
                }
            )
            cursor = marker + 4
            continue
        bytecode = payload[marker : marker + declared_size]
        try:
            container = validate_dxbc_container(bytecode)
        except ValueError as error:
            invalid_markers.append(
                {
                    "payloadOffset": marker,
                    "declaredByteSize": declared_size,
                    "reason": f"INVALID_DXBC_CONTAINER:{error}",
                }
            )
            cursor = marker + 4
            continue
        containers.append(
            {
                "payloadOffset": marker,
                "byteSize": len(bytecode),
                "dxbcSha256": container["sha256"],
                "dxbcHex": bytecode.hex(),
                "container": container,
            }
        )
        cursor = marker + declared_size

    uncovered_ranges = []
    covered_end = 0
    for container in containers:
        start = container["payloadOffset"]
        if covered_end < start:
            raw = payload[covered_end:start]
            uncovered_ranges.append(
                {
                    "payloadOffset": covered_end,
                    "byteSize": len(raw),
                    "rawSha256": sha256_bytes(raw),
                    "rawHex": raw.hex(),
                }
            )
        covered_end = start + container["byteSize"]
    if covered_end < len(payload):
        raw = payload[covered_end:]
        uncovered_ranges.append(
            {
                "payloadOffset": covered_end,
                "byteSize": len(raw),
                "rawSha256": sha256_bytes(raw),
                "rawHex": raw.hex(),
            }
        )
    covered_byte_count = sum(row["byteSize"] for row in containers)
    semantic_projection = {
        "payloadByteSize": len(payload),
        "payloadSha256": sha256_bytes(payload),
        "containers": [
            {
                key: row[key]
                for key in ("payloadOffset", "byteSize", "dxbcSha256")
            }
            for row in containers
        ],
        "uncoveredRanges": [
            {
                key: row[key]
                for key in ("payloadOffset", "byteSize", "rawSha256")
            }
            for row in uncovered_ranges
        ],
        "invalidMarkers": invalid_markers,
    }
    return {
        **semantic_projection,
        "containerCount": len(containers),
        "containerCoveredByteCount": covered_byte_count,
        "uncoveredByteCount": len(payload) - covered_byte_count,
        "containers": containers,
        "uncoveredRanges": uncovered_ranges,
        "semanticSha256": canonical_json_sha256(semantic_projection),
    }


def classify_code_mapping(
    package: dict[str, Any],
    code_index: dict[str, Any],
    reference: dict[str, Any],
    exact_evidence: dict[str, Any] | None,
) -> dict[str, Any]:
    descriptor = code_index["descriptorById"][reference["shaderIdHex"]]
    position = descriptor["codeBlobPosition"]
    descriptor_evidence = {
        key: value
        for key, value in descriptor.items()
        if key not in ("codeBlobPosition", "codeBlobSlicePartition")
    }
    base = {
        "shaderType": reference["shaderType"],
        "shaderIdHex": reference["shaderIdHex"],
        "packedDescriptor": descriptor_evidence,
    }
    reader: LostArkPackageRangeReader = package["reader"]
    compressed = reader.read_logical_range(
        position["compressedLogicalOffset"], position["compressedByteSize"]
    )
    code_blob = decompress_lz4_block(compressed, position["uncompressedByteSize"])
    slice_offset = descriptor["sliceOffsetInUncompressedBlob"]
    slice_size = descriptor["sliceByteSize"]
    payload = code_blob[slice_offset : slice_offset + slice_size]
    require(len(payload) == slice_size, "packed descriptor DXBC slice is truncated")
    enumeration = enumerate_dxbc_containers(payload)
    require(
        enumeration["containerCount"] == 1,
        "packed descriptor slice is not exactly one DXBC container",
    )
    container = enumeration["containers"][0]
    require(
        container["payloadOffset"] == 0
        and container["byteSize"] == slice_size
        and enumeration["uncoveredByteCount"] == 0,
        "packed descriptor slice does not exactly cover one DXBC container",
    )
    candidate_payload = {
        "codeBlobPosition": position,
        "codeBlobSlicePartition": descriptor["codeBlobSlicePartition"],
        "compressedByteSize": len(compressed),
        "compressedSha256": sha256_bytes(compressed),
        "uncompressedCodeBlobByteSize": len(code_blob),
        "uncompressedCodeBlobSha256": sha256_bytes(code_blob),
        "sliceOffsetInUncompressedBlob": slice_offset,
        "sliceByteSize": slice_size,
        **enumeration,
    }

    if exact_evidence is not None:
        require(container["byteSize"] == exact_evidence["byteSize"], "exact DXBC byte size changed")
        require(container["dxbcSha256"] == exact_evidence["dxbcSha256"], "exact DXBC SHA changed")
        basis = (
            "REFSHADERCACHE_PACKED_DESCRIPTOR_SLICE_FULL_PARTITION_AND_VALID_DXBC;_"
            "FROZEN_MAIN_REF_SHADER_CACHE_DXBC_HASH_CORROBORATION"
        )
    else:
        basis = "REFSHADERCACHE_PACKED_DESCRIPTOR_SLICE_FULL_PARTITION_AND_VALID_DXBC"
    return {
        **base,
        "codeMappingStatus": "EXACT",
        "unresolvedReason": None,
        "candidatePayload": candidate_payload,
        "exactEvidence": {
            "byteSize": container["byteSize"],
            "dxbcSha256": container["dxbcSha256"],
            "basis": basis,
        },
    }


def validate_target_shader_code_closure(
    package: dict[str, Any],
    code_index: dict[str, Any],
    maps: list[dict[str, Any]],
) -> dict[str, Any]:
    """Validate every shader reference reachable from the target maps."""

    references = [
        reference
        for target in maps
        for vertex_factory in target["materialMap"]["vertexFactories"]
        for reference in vertex_factory["shaderReferences"]
    ]
    unique_references = {
        (reference["shaderType"], reference["shaderIdHex"])
        for reference in references
    }
    reader: LostArkPackageRangeReader = package["reader"]
    decompressed_blobs: dict[tuple[int, int], bytes] = {}
    rows = []
    for shader_type, shader_id in sorted(unique_references):
        descriptor = code_index["descriptorById"].get(shader_id)
        require(
            descriptor is not None and descriptor["shaderType"] == shader_type,
            "target map shader reference has no packed descriptor",
        )
        blob_key = (descriptor["groupIndex"], descriptor["codeBlobIndex"])
        if blob_key not in decompressed_blobs:
            position = descriptor["codeBlobPosition"]
            compressed = reader.read_logical_range(
                position["compressedLogicalOffset"],
                position["compressedByteSize"],
            )
            decompressed_blobs[blob_key] = decompress_lz4_block(
                compressed, position["uncompressedByteSize"]
            )
        blob = decompressed_blobs[blob_key]
        offset = descriptor["sliceOffsetInUncompressedBlob"]
        size = descriptor["sliceByteSize"]
        payload = blob[offset : offset + size]
        enumeration = enumerate_dxbc_containers(payload)
        require(
            enumeration["containerCount"] == 1
            and enumeration["uncoveredByteCount"] == 0
            and enumeration["containers"][0]["payloadOffset"] == 0
            and enumeration["containers"][0]["byteSize"] == size,
            "target map packed descriptor does not select exactly one DXBC",
        )
        rows.append(
            {
                "shaderType": shader_type,
                "shaderIdHex": shader_id,
                "groupIndex": descriptor["groupIndex"],
                "codeBlobIndex": descriptor["codeBlobIndex"],
                "sliceOffsetInUncompressedBlob": offset,
                "sliceByteSize": size,
                "dxbcSha256": enumeration["containers"][0]["dxbcSha256"],
                "codeBlobSlicePartitionSha256": descriptor[
                    "codeBlobSlicePartition"
                ]["semanticSha256"],
            }
        )
    semantic_projection = {
        "shaderReferenceOccurrenceCount": len(references),
        "uniqueShaderReferenceCount": len(rows),
        "uniqueCodeBlobCount": len(decompressed_blobs),
        "rows": rows,
    }
    portable_projection = [
        {
            key: row[key]
            for key in (
                "shaderType",
                "shaderIdHex",
                "sliceByteSize",
                "dxbcSha256",
            )
        }
        for row in rows
    ]
    return {
        **semantic_projection,
        "allReferencesSelectExactlyOneValidDxbc": True,
        "semanticSha256": canonical_json_sha256(semantic_projection),
        "portableShaderIdentityAndDxbcSha256": canonical_json_sha256(
            portable_projection
        ),
    }


def parse_material_map(
    package: dict[str, Any],
    code_index: dict[str, Any],
    target: dict[str, Any],
    context: dict[str, Any],
    exact_code_oracle: dict[tuple[str, str, str], dict[str, Any]],
) -> dict[str, Any]:
    reader: LostArkPackageRangeReader = package["reader"]
    names: list[str] = package["names"]
    start = context["logicalOffset"]
    end = context["logicalEndOffset"]
    data = reader.read_logical_range(start, end - start)
    static_set = parse_static_parameter_set(data, 0, names)
    normalized = normalized_static_parameter_set(static_set)
    engine_equality = engine_equivalent_static_parameter_set(static_set)
    normalized_sha = canonical_json_sha256(normalized)
    engine_equality_sha = canonical_json_sha256(engine_equality)
    require(engine_equality_sha == target["engineEqualityStaticParameterSetSha256"], f"material-map equality set changed: {target['recipeId']}")
    offset = static_set["endOffset"]
    suffix = list(struct.unpack_from("<IIIII", data, offset))
    offset += 20
    require(suffix == [868, 16, end, 0, context["vertexFactoryCount"]], f"material-map suffix changed: {target['recipeId']}")

    vertex_factories = []
    for vf_index in range(suffix[4]):
        require(offset + 4 <= len(data), "VF reference count is truncated")
        reference_count = struct.unpack_from("<I", data, offset)[0]
        offset += 4
        require(0 < reference_count <= 256, "VF reference count is invalid")
        references = []
        for _ in range(reference_count):
            shader_type, shader_type_number, _ = read_fname_at(data, offset, names)
            require(shader_type_number == 0, "numbered shader type is unsupported")
            shader_id = data[offset + 8 : offset + 24].hex()
            repeated_type, repeated_number, _ = read_fname_at(data, offset + 24, names)
            require(repeated_number == 0 and repeated_type == shader_type, "shader type repeat changed")
            descriptor = code_index["descriptorById"].get(shader_id)
            require(descriptor is not None and descriptor["shaderType"] == shader_type, "material map references an unknown shader")
            references.append({"shaderType": shader_type, "shaderIdHex": shader_id})
            offset += 32
        vertex_factory, vf_number, offset = read_fname_at(data, offset, names)
        require(vf_number == 0, "numbered vertex factory is unsupported")
        vertex_factories.append(
            {
                "vertexFactoryIndex": vf_index,
                "vertexFactoryType": vertex_factory,
                "shaderReferenceCount": reference_count,
                "shaderReferences": references,
            }
        )

    require(offset + 16 <= len(data), "material-map opaque identity is truncated")
    opaque_identity = data[offset : offset + 16]
    offset += 16
    friendly_name, offset = read_fstring_at(data, offset)
    repeated_set = parse_static_parameter_set(data, offset, names)
    require(repeated_set["rawSha256"] == static_set["rawSha256"], "repeated static-set raw bytes changed")
    require(engine_equivalent_static_parameter_set(repeated_set) == engine_equality, "repeated equality set changed")
    offset = repeated_set["endOffset"]
    require(offset + 4 <= len(data), "material-map uniform payload or trailer is truncated")
    uniform_payload = data[offset:-4]
    require(uniform_payload, "material-map uniform payload is empty")
    trailer_platform = struct.unpack_from("<I", data, len(data) - 4)[0]
    require(trailer_platform == code_index["platform"], "material-map trailer platform changed")

    policy = target["vertexFactoryPolicy"]
    selected_vf_type = policy["vertexFactoryType"]
    selected_vf_family = policy["vertexFactoryFamily"]
    selected_pass = None
    unresolved_selection_reason = None
    selected_vf_candidates: list[str] = []
    if selected_vf_type is not None or selected_vf_family is not None:
        require(
            not (selected_vf_type is not None and selected_vf_family is not None),
            "vertex factory exact type and family cannot both be selected",
        )
        vf_rows = [
            row
            for row in vertex_factories
            if (
                selected_vf_type is not None
                and row["vertexFactoryType"].casefold()
                == selected_vf_type.casefold()
            )
            or (
                selected_vf_family == "PARTICLE_SPRITE"
                and row["vertexFactoryType"].casefold().startswith("fparticle")
                and "beamtrail" not in row["vertexFactoryType"].casefold()
            )
        ]
        selected_vf_candidates = [row["vertexFactoryType"] for row in vf_rows]
        if not vf_rows:
            unresolved_selection_reason = "VERTEX_FACTORY_STRUCTURAL_CANDIDATE_ABSENT_OR_AMBIGUOUS"
        else:
            pass_rows_by_identity = {
                (row["shaderType"], row["shaderIdHex"]): row
                for vf_row in vf_rows
                for row in vf_row["shaderReferences"]
                if row["shaderType"].casefold()
                == policy["passPixelShaderType"].casefold()
            }
            pass_rows = list(pass_rows_by_identity.values())
            if len(pass_rows) != 1:
                unresolved_selection_reason = "PIXEL_PASS_STRUCTURAL_CANDIDATE_ABSENT_OR_AMBIGUOUS"
            else:
                reference = pass_rows[0]
                oracle_key = (
                    target["recipeId"],
                    reference["shaderType"],
                    reference["shaderIdHex"],
                )
                selected_pass = classify_code_mapping(
                    package,
                    code_index,
                    reference,
                    exact_code_oracle.get(oracle_key),
                )
    else:
        unresolved_selection_reason = "SCREEN_POST_VERTEX_FACTORY_AND_PASS_UNRESOLVED"

    public_uniform_payload = {
        "byteSize": len(uniform_payload),
        "rawSha256": sha256_bytes(uniform_payload),
        "rawHex": uniform_payload.hex(),
        "parseAdmission": False,
        "reason": "ALL_CORE_UNIFORM_EXPRESSION_TYPES_ARE_OUTSIDE_SHADER_CODE_ACQUISITION_SCOPE",
    }
    semantic_map = {
        "engineEqualityStaticParameterSet": engine_equality,
        "friendlyName": friendly_name,
        "vertexFactories": vertex_factories,
        "uniformExpressionPayloadSha256": public_uniform_payload["rawSha256"],
        "trailerPlatform": trailer_platform,
    }
    return {
        "logicalOffset": start,
        "logicalEndOffset": end,
        "byteSize": len(data),
        "rawSha256": sha256_bytes(data),
        "staticParameterSet": public_static_set(static_set),
        "normalizedStaticParameterSet": normalized,
        "normalizedStaticParameterSetSha256": normalized_sha,
        "targetSerializedStaticParameterSetEqual": (
            normalized_sha == target["normalizedStaticParameterSetSha256"]
        ),
        "engineEqualityStaticParameterSet": engine_equality,
        "engineEqualityStaticParameterSetSha256": engine_equality_sha,
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
        "repeatedStaticParameterSet": public_static_set(repeated_set),
        "uniformExpressionPayload": public_uniform_payload,
        "trailerPlatform": trailer_platform,
        "semanticMapSha256": canonical_json_sha256(semantic_map),
        "selectedPass": selected_pass,
        "selection": {
            **policy,
            "candidateVertexFactoryTypes": selected_vf_candidates,
            "structuralCandidateFound": selected_pass is not None,
            "codeMappingStatus": (
                selected_pass["codeMappingStatus"]
                if selected_pass is not None
                else "CODE_UNRESOLVED"
            ),
            "unresolvedReason": (
                selected_pass["unresolvedReason"]
                if selected_pass is not None
                else unresolved_selection_reason
            ),
            "actualVfPassAdmission": False,
            "productRuntimeAdmission": False,
        },
    }


def extract_cache(
    path: Path,
    expected: dict[str, Any],
    targets: list[dict[str, Any]],
    exact_code_oracle: dict[tuple[str, str, str], dict[str, Any]],
) -> dict[str, Any]:
    package = package_tables(path)
    validate_cache_identity(package["identity"], expected)
    legacy_code_index = parse_cache_code_index(package)
    require(legacy_code_index["groupCount"] == expected["shaderTypeGroupCount"], "cache shader-type denominator changed")
    require(legacy_code_index["descriptorCount"] == expected["descriptorCount"], "cache descriptor denominator changed")
    require(legacy_code_index["embeddedCodeCount"] == expected["embeddedCodeCount"], "cache code denominator changed")
    require(legacy_code_index["shaderCodeSectionEndLogicalOffset"] == expected["shaderCodeSectionEndLogicalOffset"], "cache code-section end changed")
    packed_code_index = parse_packed_shader_code_index(package, legacy_code_index)
    contexts = scan_map_contexts(package, legacy_code_index, targets)
    maps = []
    for target in targets:
        maps.append(
            {
                "recipeId": target["recipeId"],
                "familyId": target["familyId"],
                "occurrenceIds": target["occurrenceIds"],
                "rendererType": target["rendererType"],
                "sourceMaterialPath": target["sourceMaterialPath"],
                "mapSearch": contexts[target["recipeId"]],
                "materialMap": parse_material_map(
                    package,
                    packed_code_index,
                    target,
                    contexts[target["recipeId"]],
                    exact_code_oracle,
                ),
            }
        )
    target_shader_code_closure = validate_target_shader_code_closure(
        package, packed_code_index, maps
    )
    public_legacy_index = dict(legacy_code_index)
    public_legacy_index.pop("descriptorById")
    public_packed_index = dict(packed_code_index)
    public_packed_index.pop("descriptorById")
    return {
        "package": package["identity"],
        "legacyDescriptorTailCandidateIndex": public_legacy_index,
        "packedShaderCodeSliceIndex": public_packed_index,
        "targetShaderCodeClosure": target_shader_code_closure,
        "targets": maps,
    }


def build_registry(
    targets: list[dict[str, Any]],
    official: dict[str, Any],
    installed: dict[str, Any],
) -> list[dict[str, Any]]:
    official_by_recipe = {row["recipeId"]: row for row in official["targets"]}
    installed_by_recipe = {row["recipeId"]: row for row in installed["targets"]}
    result = []
    for target in targets:
        recipe_id = target["recipeId"]
        official_row = official_by_recipe[recipe_id]
        installed_row = installed_by_recipe[recipe_id]
        official_map = official_row["materialMap"]
        installed_map = installed_row["materialMap"]
        official_pass = official_map["selectedPass"]
        installed_pass = installed_map["selectedPass"]
        require((official_pass is None) == (installed_pass is None), f"official/current pass selection differs: {recipe_id}")
        official_status = official_map["selection"]["codeMappingStatus"]
        installed_status = installed_map["selection"]["codeMappingStatus"]
        require(official_status == installed_status, f"official/current code status differs: {recipe_id}")
        shader_identity_equal = official_pass is None
        candidate_payload_equal = official_pass is None
        if official_pass is not None:
            shader_identity_equal = (
                official_pass["shaderType"] == installed_pass["shaderType"]
                and official_pass["shaderIdHex"] == installed_pass["shaderIdHex"]
            )
            official_payload = official_pass["candidatePayload"]
            installed_payload = installed_pass["candidatePayload"]
            candidate_payload_equal = (
                (official_payload is None) == (installed_payload is None)
                and (
                    official_payload is None
                    or official_payload["semanticSha256"]
                    == installed_payload["semanticSha256"]
                )
            )
            if official_status == "EXACT":
                require(shader_identity_equal, f"official/current exact shader identity differs: {recipe_id}")
                require(candidate_payload_equal, f"official/current exact DXBC payload differs: {recipe_id}")
                require(
                    official_pass["exactEvidence"] == installed_pass["exactEvidence"],
                    f"official/current exact DXBC evidence differs: {recipe_id}",
                )
        policy = target["vertexFactoryPolicy"]
        vertex_factory_type = policy["vertexFactoryType"]
        vertex_factory_family = policy["vertexFactoryFamily"]
        material_map_equal = (
            official_map["semanticMapSha256"] == installed_map["semanticMapSha256"]
        )
        for occurrence_id in target["occurrenceIds"]:
            identity = {
                "occurrenceId": occurrence_id,
                "recipeId": recipe_id,
                "engineEqualityStaticParameterSetSha256": target["engineEqualityStaticParameterSetSha256"],
                "rendererType": target["rendererType"],
                "vertexFactoryType": vertex_factory_type,
                "vertexFactoryFamily": vertex_factory_family,
            }
            result.append(
                {
                    **identity,
                    "stableOccurrenceShaderKeySha256": canonical_json_sha256(identity),
                    "officialMaterialMapSemanticSha256": official_map["semanticMapSha256"],
                    "installedMaterialMapSemanticSha256": installed_map["semanticMapSha256"],
                    "officialInstalledMaterialMapSemanticsEqual": material_map_equal,
                    "pixelShaderType": official_pass["shaderType"] if official_pass else None,
                    "pixelShaderIdHex": official_pass["shaderIdHex"] if official_pass else None,
                    "pixelDxbcSha256": (
                        official_pass["exactEvidence"]["dxbcSha256"]
                        if official_status == "EXACT"
                        else None
                    ),
                    "codeMappingStatus": official_status,
                    "codeMappingUnresolvedReason": (
                        official_map["selection"]["unresolvedReason"]
                        if official_status == "CODE_UNRESOLVED"
                        else None
                    ),
                    "officialInstalledShaderIdentityEqual": shader_identity_equal,
                    "officialInstalledCandidatePayloadEqual": candidate_payload_equal,
                    "selectionFidelity": policy["selectionFidelity"],
                    "registryAdmission": False,
                    "actualVfPassAdmission": False,
                    "productRuntimeAdmission": False,
                    "blockers": [
                        "OCCURRENCE_ACTUAL_VF_PASS_SELECTION_PENDING",
                        "PRODUCT_SHADER_OBJECT_BINDING_AND_RENDER_STATE_INTEGRATION_PENDING",
                        *(
                            ["UNRESOLVED_CODE_MAPPING"]
                            if official_status == "CODE_UNRESOLVED"
                            else []
                        ),
                    ],
                }
            )
    require(len(result) == 22, "stable occurrence shader registry denominator changed")
    require(len({row["stableOccurrenceShaderKeySha256"] for row in result}) == len(result), "stable occurrence shader key is duplicated")
    return result


def build_receipt(
    targets_path: Path,
    official_manifest_path: Path,
    official_cache_path: Path,
    installed_cache_path: Path,
    installed_root: Path,
) -> dict[str, Any]:
    targets_document = read_json(targets_path)
    contract_path = REPO_ROOT / targets_document["inputs"]["typedMaterialContract"]["repoRelativePath"]
    native_path = REPO_ROOT / targets_document["inputs"]["nativeMaterialReceipt"]["repoRelativePath"]
    material_contract = read_json(contract_path)
    native_receipt = read_json(native_path)
    targets = validate_target_joins(targets_document, material_contract, native_receipt)
    exact_oracle_path, exact_code_oracle = load_exact_code_oracle(targets_document)
    manifest_rows = parse_manifest_package_rows(official_manifest_path, targets_document)
    source_packages = validate_source_packages(installed_root, targets_document, manifest_rows)

    expected_official = targets_document["inputs"]["officialRefShaderCache"]
    require(official_cache_path.name == expected_official["fileName"], "official RefShaderCache name changed")
    require(digest_file(official_cache_path) == expected_official["rawSha256"], "official RefShaderCache SHA changed")
    require(digest_file(official_cache_path, "md5") == expected_official["extractedMd5"], "official RefShaderCache MD5 changed")
    official = extract_cache(
        official_cache_path, EXPECTED_OFFICIAL_CACHE, targets, exact_code_oracle
    )
    installed = extract_cache(
        installed_cache_path, EXPECTED_INSTALLED_CACHE, targets, exact_code_oracle
    )
    require(
        official["targetShaderCodeClosure"][
            "portableShaderIdentityAndDxbcSha256"
        ]
        == installed["targetShaderCodeClosure"][
            "portableShaderIdentityAndDxbcSha256"
        ],
        "official/current target shader identities or DXBC slices differ",
    )
    registry = build_registry(targets, official, installed)
    recipe_statuses = {
        row["recipeId"]: row["materialMap"]["selection"]["codeMappingStatus"]
        for row in official["targets"]
    }
    require(
        set(recipe_statuses.values()) <= {"EXACT", "CODE_UNRESOLVED"},
        "unknown code mapping status",
    )
    occurrence_statuses = [row["codeMappingStatus"] for row in registry]
    exact_recipe_count = sum(status == "EXACT" for status in recipe_statuses.values())
    exact_occurrence_count = sum(status == "EXACT" for status in occurrence_statuses)
    unresolved_recipe_count = len(recipe_statuses) - exact_recipe_count
    unresolved_occurrence_count = len(occurrence_statuses) - exact_occurrence_count
    require(
        (exact_recipe_count, unresolved_recipe_count) == (17, 1),
        "EXACT/CODE_UNRESOLVED recipe denominator changed",
    )
    require(
        (exact_occurrence_count, unresolved_occurrence_count) == (21, 1),
        "EXACT/CODE_UNRESOLVED occurrence denominator changed",
    )
    official_by_occurrence = {
        occurrence_id: row["materialMap"]["selectedPass"]
        for row in official["targets"]
        for occurrence_id in row["occurrenceIds"]
    }
    bundled_slice = official_by_occurrence["source-active-005"]["candidatePayload"]
    require(
        official_by_occurrence["source-active-005"]["codeMappingStatus"] == "EXACT"
        and bundled_slice["containerCount"] == 1
        and bundled_slice["sliceOffsetInUncompressedBlob"] == 0
        and bundled_slice["sliceByteSize"] == 2004
        and bundled_slice["codeBlobSlicePartition"]["sliceByteSizes"]
        == [2004, 1936],
        "#5/#6 packed DXBC slice boundary changed",
    )
    no_position_recovered = official_by_occurrence["source-active-023"]
    require(
        no_position_recovered["codeMappingStatus"] == "EXACT"
        and no_position_recovered["packedDescriptor"]["codeBlobIndex"] == 1761
        and no_position_recovered["candidatePayload"][
            "sliceOffsetInUncompressedBlob"
        ]
        == 956
        and no_position_recovered["candidatePayload"]["sliceByteSize"] == 1496,
        "#23 packed DXBC slice recovery boundary changed",
    )
    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "characterClass": "ARTIST",
        "skillId": 31470,
        "inputSlot": "F",
        "scope": targets_document["scope"],
        "inputs": {
            "tool": repo_file_identity(SCRIPT_PATH),
            "targetManifest": repo_file_identity(targets_path),
            "typedMaterialContract": repo_file_identity(contract_path),
            "nativeMaterialReceipt": repo_file_identity(native_path),
            "exactCodeOracle": repo_file_identity(exact_oracle_path),
            "officialManifest": {
                "fileName": official_manifest_path.name,
                "rawSha256": digest_file(official_manifest_path),
            },
            "sourcePackages": source_packages,
        },
        "derivedTargetIdentities": [
            {
                key: target[key]
                for key in (
                    "recipeId",
                    "familyId",
                    "occurrenceIds",
                    "rendererType",
                    "sourceMaterialPath",
                    "baseMaterialIdHex",
                    "normalizedStaticParameterSetSha256",
                    "engineEqualityStaticParameterSetSha256",
                    "vertexFactoryPolicy",
                )
            }
            for target in targets
        ],
        "officialRefShaderCache": official,
        "installedRefShaderCache": installed,
        "stableOccurrenceShaderRegistry": registry,
        "admission": {
            "sameCohortSourcePackageExact": True,
            "engineEqualityStaticParameterSetExact": True,
            "officialInstalledAllMaterialMapSemanticsEqual": all(
                row["officialInstalledMaterialMapSemanticsEqual"] for row in registry
            ),
            "descriptorTailU32IsExactCodeIndex": False,
            "packedDescriptorCodeBlobAndSliceLayoutExact": True,
            "allTargetMapShaderReferencesSelectOneValidDxbc": True,
            "officialInstalledTargetShaderIdentityAndDxbcEqual": True,
            "actualVfPassSelection": False,
            "registry": False,
            "productRuntime": False,
            "visualFidelity": False,
        },
        "summary": {
            "targetRecipeCount": len(targets),
            "targetFamilyCount": len({target["familyId"] for target in targets}),
            "targetEngineEqualityStaticParameterSetCount": len(
                {target["engineEqualityStaticParameterSetSha256"] for target in targets}
            ),
            "targetOccurrenceCount": len(registry),
            "registryAdmittedOccurrenceCount": sum(row["registryAdmission"] for row in registry),
            "productRuntimeAdmittedOccurrenceCount": 0,
            "codeMappingRecipeDenominator": {
                "total": len(recipe_statuses),
                "EXACT": exact_recipe_count,
                "CODE_UNRESOLVED": unresolved_recipe_count,
            },
            "codeMappingOccurrenceDenominator": {
                "total": len(occurrence_statuses),
                "EXACT": exact_occurrence_count,
                "CODE_UNRESOLVED": unresolved_occurrence_count,
            },
            "ambiguityPolicy": "PACKED_SHADER_CODE_SLICE_EXACT;_ACTUAL_VF_PASS_AND_PRODUCT_BINDING_REMAIN_FAIL_CLOSED",
        },
    }
    seal(receipt)
    return receipt


def check_or_write(path: Path, receipt: dict[str, Any], check: bool) -> None:
    text = json.dumps(receipt, ensure_ascii=False, indent=2) + "\n"
    if check:
        require(path.is_file(), f"receipt is missing: {path}")
        require(path.read_text(encoding="utf-8") == text, "receipt is stale")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(text, encoding="utf-8")
    temporary.replace(path)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--targets", type=Path, default=DEFAULT_TARGETS)
    parser.add_argument("--official-manifest", type=Path, default=DEFAULT_OFFICIAL_MANIFEST)
    parser.add_argument("--official-cache", type=Path, default=DEFAULT_OFFICIAL_CACHE)
    parser.add_argument("--installed-cache", type=Path, default=DEFAULT_INSTALLED_CACHE)
    parser.add_argument("--installed-root", type=Path, default=DEFAULT_INSTALLED_ROOT)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    receipt = build_receipt(
        args.targets,
        args.official_manifest,
        args.official_cache,
        args.installed_cache,
        args.installed_root,
    )
    check_or_write(args.output, receipt, args.check)
    print(
        "Artist 31470 all-core RefShaderCache acquisition: "
        f"{receipt['summary']['targetFamilyCount']} families / "
        f"{receipt['summary']['targetOccurrenceCount']} occurrences; "
        f"EXACT {receipt['summary']['codeMappingOccurrenceDenominator']['EXACT']} / "
        f"CODE_UNRESOLVED {receipt['summary']['codeMappingOccurrenceDenominator']['CODE_UNRESOLVED']}; "
        "product runtime admission remains false"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
