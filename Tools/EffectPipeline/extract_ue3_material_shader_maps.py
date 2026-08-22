#!/usr/bin/env python3
"""Join UE3 MaterialInstanceConstant static sets to exact cooked shader maps.

This is the class-neutral cooked Material recovery path through G03-6.  It:

* read a pinned source MIC and decode its native ``FStaticParameterSet``;
* join that set to exactly one map in a pinned RefShaderCache using the Lost
  Ark v868 engine-equality projection;
* decode only that map's vertex-factory references and uniform-expression set;
* select a renderer-compatible structural VF/pass candidate and extract the
  exact packed DXBC slice named by its shader reference;
* walk the native shader-object table by each object's serialized end pointer,
  select one exact shaderType+shaderId object, and close its scalar/vector/
  texture binding wires against the uniform-expression denominators and DXBC
  declarations;
* fail closed when a MIC has no native static resource instead of silently
  selecting its parent's default map.
* for manifest-declared canaries, join an authored emitter ``sourceRecipe``
  to one exact particle vertex factory and an authoring-bounded NoDensity
  BasePass vertex shader, then close its output signature to the selected
  pixel shader input signature.

The frozen Artist 31470 extractors remain unchanged.  This tool reuses their
already-proven binary primitives, but owns no character-, skill-, family-,
offset-, or denominator-specific policy.  A target manifest supplies those
facts.  A successful receipt is structural evidence only: it does not admit a
runtime renderer, actual vertex factory/pass, numeric replay, or visual
fidelity.
"""

from __future__ import annotations

import argparse
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

from derive_artist_31470_main_shader_map_identity import (  # noqa: E402
    engine_equivalent_static_parameter_set,
    normalized_static_parameter_set,
)
from extract_artist_31470_main_ref_shader_cache import (  # noqa: E402
    BufferedLogicalCursor,
    D3DDisassembler,
    DEFAULT_D3DCOMPILER,
    package_tables,
    read_fname_at,
    read_fstring_at,
    require,
)
from extract_artist_31470_all_core_ref_shader_cache import (  # noqa: E402
    decode_packed_shader_code_slice,
)
from extract_artist_31470_shader_cache_oracle import (  # noqa: E402
    canonical_json_sha256,
    parse_static_parameter_set,
    validate_dxbc_container,
)
from extract_ue3_effect_material_closure import load_package  # noqa: E402
from extract_ue3_placements import (  # noqa: E402
    LOSTARK_KR_AES_KEY,
    LostArkPackageRangeReader,
    decompress_lz4_block,
    package_ref_name,
    package_ref_path,
    parse_tagged_properties,
)


TARGET_SCHEMA = "lostark.effect-ue3-material-shader-map-targets"
TARGET_FORMAT_VERSION = 3
RECEIPT_SCHEMA = "lostark.effect-ue3-material-shader-map-receipt"
RECEIPT_FORMAT_VERSION = 3

DEFAULT_TARGETS = REPOSITORY_ROOT / (
    "Data/Effects/Imported/DimensionMaster/Materials/"
    "skill.2050120.clip3.exact-shader-targets.json"
)
DEFAULT_OUTPUT = REPOSITORY_ROOT / (
    "Data/Effects/Imported/DimensionMaster/Materials/"
    "skill.2050120.clip3.exact-material-maps.receipt.json"
)
DEFAULT_SOURCE_ROOT = Path(
    "C:/Users/user/Desktop/Resource_LostArk/00_SourcePackages/"
    "Effect_DIMENSIONMASTER_20260803_v3/Dependencies"
)
DEFAULT_CACHE = Path(
    "C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Effect/ARTIST/"
    "31470_TrackA_20260812/OfficialRefShaderCacheV974/"
    "EV2LG3OVEH3HGV7THTFFTM7TOKMCC.v974.upk"
)

POLICY_REQUIRE_NATIVE = "REQUIRE_MIC_NATIVE_FSTATICPARAMETERSET"
POLICY_BLOCK_ABSENT = "BLOCK_IF_MIC_HAS_NO_NATIVE_STATIC_RESOURCE"
STATUS_EXACT = "EXACT_MATERIAL_SHADER_MAP"
STATUS_BLOCKED = "BLOCKED_NO_EFFECTIVE_STATIC_SET_ABI_EVIDENCE"
DENSITY_NO_DENSITY_AUTHORING_BOUNDED = "NO_DENSITY_AUTHORING_BOUNDED"
SOURCE_EMITTER_VF_PASS_FIELD = "expectedSourceEmitterVertexFactoryPass"


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def digest_file(path: Path, algorithm: str = "sha256") -> str:
    digest = hashlib.new(algorithm)
    with path.open("rb") as stream:
        while True:
            payload = stream.read(1024 * 1024)
            if not payload:
                break
            digest.update(payload)
    return digest.hexdigest()


def read_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8-sig"))
    require(isinstance(value, dict), f"expected JSON object: {path}")
    return value


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    temporary.replace(path)


def public_static_set(value: dict[str, Any]) -> dict[str, Any]:
    result = dict(value)
    result.pop("endOffset", None)
    return result


def seal(value: dict[str, Any]) -> None:
    value.pop("receiptSha256", None)
    value["receiptSha256"] = canonical_json_sha256(value)


def validate_target_manifest(document: dict[str, Any]) -> list[dict[str, Any]]:
    require(document.get("schema") == TARGET_SCHEMA, "target manifest schema changed")
    require(
        document.get("formatVersion") == TARGET_FORMAT_VERSION,
        "target manifest version changed",
    )
    inputs = document.get("inputs")
    targets = document.get("targets")
    summary = document.get("summary")
    require(isinstance(inputs, dict), "target manifest inputs are absent")
    require(isinstance(targets, list) and targets, "target manifest targets are absent")
    require(isinstance(summary, dict), "target manifest summary is absent")
    cache = inputs.get("officialRefShaderCache")
    authored = inputs.get("authoredEffectDocument")
    sources = inputs.get("sourcePackages")
    policies = document.get("vertexFactoryPolicies")
    native_policy = document.get("nativeShaderObjectBindingPolicy")
    require(isinstance(cache, dict), "official RefShaderCache identity is absent")
    require(isinstance(authored, dict), "authored effect-document input is absent")
    authored_path_text = str(authored.get("repoRelativePath", ""))
    authored_path = Path(authored_path_text)
    require(
        authored_path_text
        and not authored_path.is_absolute()
        and ".." not in authored_path.parts
        and authored_path.as_posix() == authored_path_text,
        "authored effect-document path must be repository-relative POSIX",
    )
    require(
        authored.get("schema") == "lostark.effect-authoring"
        and isinstance(authored.get("version"), int)
        and isinstance(authored.get("effectAssetId"), str)
        and authored["effectAssetId"]
        and authored.get("effectAssetId")
        == document.get("identity", {}).get("effectAssetId"),
        "authored effect-document identity is invalid",
    )
    require(isinstance(sources, list) and sources, "source-package identities are absent")
    require(isinstance(policies, dict) and policies, "vertex-factory policies are absent")
    require(
        isinstance(native_policy, dict),
        "native shader-object binding policy is absent",
    )
    require(
        native_policy.get("candidateAdmission") == "EXACTLY_ONE_STRUCTURAL_CLOSURE",
        "native binding candidate policy changed",
    )
    require(
        native_policy.get("actualVfPassAdmission") is False
        and native_policy.get("runtimeAdmission") is False,
        "native binding evidence must not admit runtime/VF",
    )

    source_names: set[str] = set()
    for source in sources:
        require(isinstance(source, dict), "source-package row is invalid")
        name = str(source.get("fileName", "")).casefold()
        require(name and name not in source_names, "source-package identity is duplicated")
        source_names.add(name)
        require(len(str(source.get("rawSha256", ""))) == 64, "source-package SHA is invalid")

    target_ids: set[str] = set()
    occurrence_ids: set[str] = set()
    expected_source_emitter_vf_pass_count = 0
    for target in targets:
        require(isinstance(target, dict), "target row is invalid")
        target_id = str(target.get("targetId", ""))
        require(target_id and target_id not in target_ids, "target ID is duplicated")
        target_ids.add(target_id)
        require(
            str(target.get("sourcePackageFileName", "")).casefold() in source_names,
            f"target source package is not declared: {target_id}",
        )
        renderer_type = str(target.get("rendererType", ""))
        require(
            renderer_type in policies,
            f"target renderer policy is absent: {target_id}",
        )
        policy = policies[renderer_type]
        require(
            policy.get("family") in ("PARTICLE_SPRITE", "LOCAL_MESH"),
            f"unsupported G03-3 vertex-factory family: {renderer_type}",
        )
        require(
            policy.get("actualVfPassAdmission") is False,
            f"target policy must not admit an actual VF/pass: {renderer_type}",
        )
        require(
            isinstance(policy.get("passPixelShaderType"), str)
            and policy["passPixelShaderType"],
            f"target pass policy is absent: {renderer_type}",
        )
        require(
            target.get("staticParameterPolicy")
            in (POLICY_REQUIRE_NATIVE, POLICY_BLOCK_ABSENT),
            f"target static-parameter policy is invalid: {target_id}",
        )
        require(
            target.get("expectedStatus") in (STATUS_EXACT, STATUS_BLOCKED),
            f"target expected status is invalid: {target_id}",
        )
        base_id = str(target.get("baseMaterialIdHex", ""))
        require(len(base_id) == 32, f"target base Material ID is invalid: {target_id}")
        bytes.fromhex(base_id)
        rows = target.get("occurrenceIds")
        require(isinstance(rows, list) and rows, f"target occurrences are absent: {target_id}")
        for occurrence_id in rows:
            require(
                occurrence_id not in occurrence_ids,
                f"occurrence is assigned to multiple targets: {occurrence_id}",
            )
            occurrence_ids.add(occurrence_id)

        source_vf_pass = target.get(SOURCE_EMITTER_VF_PASS_FIELD)
        if source_vf_pass is not None:
            expected_source_emitter_vf_pass_count += 1
            require(
                isinstance(source_vf_pass, dict),
                f"source emitter VF/pass contract is invalid: {target_id}",
            )
            require(
                renderer_type == "SpriteParticle"
                and target.get("expectedStatus") == STATUS_EXACT,
                f"source emitter VF/pass requires an exact SpriteParticle target: {target_id}",
            )
            source_occurrence_id = source_vf_pass.get("occurrenceId")
            require(
                source_occurrence_id in rows,
                f"source emitter VF/pass occurrence is not owned by target: {target_id}",
            )
            required_module = source_vf_pass.get("requiredModule")
            dynamic_module = source_vf_pass.get("dynamicModule")
            require(
                isinstance(required_module, dict)
                and isinstance(required_module.get("boffsetcenter"), bool)
                and isinstance(required_module.get("screenalignment"), str),
                f"source Required module contract is invalid: {target_id}",
            )
            require(
                isinstance(dynamic_module, dict)
                and isinstance(dynamic_module.get("className"), str)
                and dynamic_module["className"].casefold()
                == "particlemoduleparameterdynamic"
                and isinstance(dynamic_module.get("updateflags"), int)
                and not isinstance(dynamic_module.get("updateflags"), bool),
                f"source DynamicParameter module contract is invalid: {target_id}",
            )
            require(
                source_vf_pass.get("densityPolicy")
                == DENSITY_NO_DENSITY_AUTHORING_BOUNDED,
                f"source vertex pass must remain authoring-bounded NoDensity: {target_id}",
            )
            require(
                all(
                    isinstance(source_vf_pass.get(field), str)
                    and source_vf_pass[field]
                    for field in (
                        "vertexFactoryType",
                        "vertexShaderType",
                        "vertexShaderIdHex",
                    )
                ),
                f"source emitter VF/pass identity is incomplete: {target_id}",
            )
            vertex_shader_id = str(source_vf_pass["vertexShaderIdHex"])
            require(
                len(vertex_shader_id) == 32,
                f"source vertex shader ID is invalid: {target_id}",
            )
            bytes.fromhex(vertex_shader_id)
            expected_dxbc = source_vf_pass.get("expectedDxbc")
            require(
                isinstance(expected_dxbc, dict)
                and isinstance(expected_dxbc.get("byteSize"), int)
                and expected_dxbc["byteSize"] > 0
                and len(str(expected_dxbc.get("sha256", ""))) == 64,
                f"source vertex shader DXBC identity is invalid: {target_id}",
            )
            bytes.fromhex(str(expected_dxbc["sha256"]))
            expected_cbs = source_vf_pass.get(
                "expectedConstantBufferFloat4Counts"
            )
            require(
                isinstance(expected_cbs, dict)
                and expected_cbs
                and all(
                    str(register).isdigit()
                    and isinstance(count, int)
                    and count > 0
                    for register, count in expected_cbs.items()
                ),
                f"source vertex shader constant-buffer contract is invalid: {target_id}",
            )

    require(
        summary.get("targetCount") == len(targets),
        "target manifest target denominator changed",
    )
    require(
        summary.get("occurrenceCount") == len(occurrence_ids),
        "target manifest occurrence denominator changed",
    )
    require(
        summary.get("expectedExactCount")
        == sum(target["expectedStatus"] == STATUS_EXACT for target in targets),
        "target manifest exact denominator changed",
    )
    require(
        summary.get("expectedBlockedCount")
        == sum(target["expectedStatus"] == STATUS_BLOCKED for target in targets),
        "target manifest blocked denominator changed",
    )
    require(
        summary.get("expectedExactDxbcCount") == summary.get("expectedExactCount"),
        "target manifest exact DXBC denominator changed",
    )
    require(
        summary.get("expectedExactNativeBindingCount")
        == summary.get("expectedExactCount"),
        "target manifest exact native-binding denominator changed",
    )
    require(
        summary.get("expectedSourceEmitterVertexFactoryPassCount")
        == expected_source_emitter_vf_pass_count,
        "target manifest source emitter VF/pass denominator changed",
    )
    return targets


def load_authored_effect_document(
    expected: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any]]:
    """Load one repository-owned authored document without an absolute fallback."""

    repo_relative_path = str(expected["repoRelativePath"])
    path = (REPOSITORY_ROOT / Path(repo_relative_path)).resolve()
    try:
        actual_relative_path = path.relative_to(REPOSITORY_ROOT).as_posix()
    except ValueError as error:
        raise ValueError("authored effect document escapes the repository") from error
    require(
        actual_relative_path == repo_relative_path,
        "authored effect-document path changed after resolution",
    )
    require(path.is_file(), f"authored effect document is missing: {path}")
    document = read_json(path)
    require(
        document.get("schema") == expected["schema"]
        and document.get("version") == expected["version"]
        and document.get("effectAssetId") == expected["effectAssetId"],
        "authored effect-document identity changed",
    )
    require(
        isinstance(document.get("elements"), list),
        "authored effect-document elements are absent",
    )
    return document, {
        "repoRelativePath": repo_relative_path,
        "rawSha256": digest_file(path),
        "schema": document["schema"],
        "version": document["version"],
        "effectAssetId": document["effectAssetId"],
    }


def find_exact_export(package: Any, target: dict[str, Any]) -> Any:
    wanted_path = target["micObjectPath"].casefold()
    wanted_class = target.get("micClassName", "materialinstanceconstant").casefold()
    rows = []
    for entry in package.exports:
        actual_path = package_ref_path(
            entry.index + 1, package.imports, package.exports
        )
        actual_class = package_ref_name(
            entry.class_index, package.imports, package.exports
        ) or ""
        if (
            actual_path.casefold() == wanted_path
            and actual_class.casefold() == wanted_class
        ):
            rows.append(entry)
    require(
        len(rows) == 1,
        f"MIC export is absent or ambiguous: {target['targetId']}",
    )
    entry = rows[0]
    expected_index = target.get("expectedExportIndexZeroBased")
    if expected_index is not None:
        require(
            entry.index == expected_index,
            f"MIC export index changed: {target['targetId']}",
        )
    return entry


def decode_static_set_from_tail(
    tail: bytes,
    base_material_id: bytes,
    names: list[str],
    policy: str,
) -> dict[str, Any]:
    """Decode one MIC key, or return the explicit no-resource blocker."""

    if not tail:
        require(
            policy == POLICY_BLOCK_ABSENT,
            "required MIC native static resource is absent",
        )
        return {
            "status": STATUS_BLOCKED,
            "blocker": STATUS_BLOCKED,
            "nativeTailByteCount": 0,
            "baseMaterialIdOccurrenceCount": 0,
        }

    offsets: list[int] = []
    cursor = 0
    while True:
        found = tail.find(base_material_id, cursor)
        if found < 0:
            break
        offsets.append(found)
        cursor = found + 1
    require(
        len(offsets) == 1,
        "MIC base Material ID occurrence is absent or ambiguous",
    )
    decoded = parse_static_parameter_set(tail, offsets[0], names)
    require(
        decoded["baseMaterialIdHex"] == base_material_id.hex(),
        "decoded MIC base Material ID changed",
    )
    normalized = normalized_static_parameter_set(decoded)
    engine_equality = engine_equivalent_static_parameter_set(decoded)
    return {
        "status": "MIC_NATIVE_STATIC_SET_DECODED",
        "nativeTailByteCount": len(tail),
        "baseMaterialIdOccurrenceCount": len(offsets),
        "staticParameterSetOffsetInNativeTail": offsets[0],
        "staticParameterSet": public_static_set(decoded),
        "normalizedStaticParameterSet": normalized,
        "normalizedStaticParameterSetSha256": canonical_json_sha256(normalized),
        "engineEqualityStaticParameterSet": engine_equality,
        "engineEqualityStaticParameterSetSha256": canonical_json_sha256(
            engine_equality
        ),
    }


def decode_mic_target(
    package: Any,
    target: dict[str, Any],
) -> dict[str, Any]:
    entry = find_exact_export(package, target)
    serial = package.logical[
        entry.serial_offset : entry.serial_offset + entry.serial_size
    ]
    properties, property_end = parse_tagged_properties(
        serial, package.names, package.summary.version
    )
    expected_serial_sha = target.get("expectedSerialSha256")
    if expected_serial_sha:
        require(
            sha256_bytes(serial) == expected_serial_sha,
            f"MIC serial SHA changed: {target['targetId']}",
        )
    if target.get("expectedPropertyStreamEnd") is not None:
        require(
            property_end == target["expectedPropertyStreamEnd"],
            f"MIC property end changed: {target['targetId']}",
        )
    tail = serial[property_end:]
    decoded = decode_static_set_from_tail(
        tail,
        bytes.fromhex(target["baseMaterialIdHex"]),
        package.names,
        target["staticParameterPolicy"],
    )
    has_static_resource_property = any(
        str(name).casefold() == "bhasstaticpermutationresource"
        for name in properties
    )
    result = {
        "sourcePackageFileName": package.path.name,
        "micObjectPath": package_ref_path(
            entry.index + 1, package.imports, package.exports
        ),
        "micClassName": package_ref_name(
            entry.class_index, package.imports, package.exports
        ),
        "exportIndexZeroBased": entry.index,
        "serialOffset": entry.serial_offset,
        "serialByteSize": entry.serial_size,
        "serialSha256": sha256_bytes(serial),
        "propertyStreamEnd": property_end,
        "nativeTailByteCount": len(tail),
        "hasStaticPermutationResourceTaggedProperty": has_static_resource_property,
        **decoded,
    }
    expected_switch_count = target.get("expectedStaticSwitchCount")
    if decoded["status"] != STATUS_BLOCKED and expected_switch_count is not None:
        require(
            len(decoded["staticParameterSet"]["staticSwitchParameters"])
            == expected_switch_count,
            f"MIC static-switch denominator changed: {target['targetId']}",
        )
    expected_raw = target.get("expectedStaticParameterSetRawSha256")
    if decoded["status"] != STATUS_BLOCKED and expected_raw:
        require(
            decoded["staticParameterSet"]["rawSha256"] == expected_raw,
            f"MIC static-set raw SHA changed: {target['targetId']}",
        )
    return result


def parse_shader_code_layout(package: dict[str, Any]) -> dict[str, Any]:
    """Derive the map-table start without retaining 265k shader descriptors."""

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
    group_count = cursor.u32()
    require(0 < group_count <= 4096, "RefShaderCache group count is invalid")

    descriptor_count = 0
    code_blob_count = 0
    groups = []
    for group_index in range(group_count):
        group_offset = cursor.offset
        shader_type, shader_type_number = cursor.fname(names)
        require(shader_type_number == 0, "numbered shader type is unsupported")
        descriptors = cursor.u32()
        require(0 < descriptors <= 1_000_000, "shader descriptor count is invalid")
        cursor.skip(descriptors * 24)
        blobs = cursor.u32()
        require(0 < blobs <= descriptors, "shader code-blob count is invalid")
        for _ in range(blobs):
            uncompressed_size = cursor.u32()
            compressed_size = cursor.u32()
            require(
                32 <= uncompressed_size <= 64 * 1024 * 1024,
                "shader code uncompressed size is invalid",
            )
            require(
                0 < compressed_size <= reader.logical_size - cursor.offset,
                "shader code compressed size is invalid",
            )
            cursor.skip(compressed_size)
        groups.append(
            {
                "groupIndex": group_index,
                "logicalOffset": group_offset,
                "shaderType": shader_type,
                "descriptorCount": descriptors,
                "codeBlobCount": blobs,
            }
        )
        descriptor_count += descriptors
        code_blob_count += blobs

    code_end = cursor.offset
    tail_platform, shader_object_count = struct.unpack(
        "<II", reader.read_logical_range(code_end, 8)
    )
    require(tail_platform == platform, "shader-object platform changed")
    require(
        shader_object_count == descriptor_count,
        "shader-object denominator differs from descriptor count",
    )
    return {
        "platform": platform,
        "groupCount": group_count,
        "descriptorCount": descriptor_count,
        "codeBlobCount": code_blob_count,
        "shaderObjectCount": shader_object_count,
        "shaderCodeSectionEndLogicalOffset": code_end,
        "materialMapScanStartLogicalOffset": code_end + 8,
        "groupsSemanticSha256": canonical_json_sha256(groups),
    }


def _public_expression(value: dict[str, Any]) -> dict[str, Any]:
    return {
        key: (
            _public_expression(item)
            if isinstance(item, dict) and "typeName" in item
            else item
        )
        for key, item in value.items()
        if key != "endOffset"
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
    """Parse all UE868 uniform-expression shapes named by the pinned cache.

    Artist's frozen extractor intentionally admits only the shapes its two
    exact maps observed.  The class-neutral path retains those layouts and adds
    the standard UE3 unary, binary and ternary wrappers present in the pinned
    cache NameTable.  It records arithmetic op ordinals without assigning
    evaluator meaning; register binding and replay remain later stages.
    """

    require(depth <= 64, "uniform expression recursion is excessive")
    if node_budget is None:
        node_budget = [0]
    node_budget[0] += 1
    require(node_budget[0] <= 8192, "uniform expression node budget is excessive")
    start = offset
    type_name, number, offset = read_fname_at(data, offset, names)
    require(number == 0, "numbered uniform-expression type is unsupported")
    folded = type_name.casefold()
    require(
        folded.startswith("fmaterialuniformexpression"),
        "uniform-expression type name is invalid",
    )
    row: dict[str, Any] = {"typeName": folded, "offset": start}

    def child(at: int) -> dict[str, Any]:
        return parse_uniform_expression(
            data,
            at,
            names,
            depth=depth + 1,
            node_budget=node_budget,
        )

    binary_types = {
        "fmaterialuniformexpressionfmod",
        "fmaterialuniformexpressionmax",
    }
    unary_types = {
        "fmaterialuniformexpressionabs",
        "fmaterialuniformexpressionceil",
        "fmaterialuniformexpressionfloor",
        "fmaterialuniformexpressionfrac",
        "fmaterialuniformexpressionperiodic",
        "fmaterialuniformexpressionsquareroot",
    }
    if folded == "fmaterialuniformexpressionfoldedmath":
        row["a"] = child(offset)
        row["b"] = child(row["a"]["endOffset"])
        offset = row["b"]["endOffset"]
        require(offset < len(data), "FoldedMath operation is truncated")
        row["operationOrdinal"] = data[offset]
        offset += 1
    elif folded in binary_types:
        row["a"] = child(offset)
        row["b"] = child(row["a"]["endOffset"])
        offset = row["b"]["endOffset"]
    elif folded == "fmaterialuniformexpressionclamp":
        row["input"] = child(offset)
        row["minimum"] = child(row["input"]["endOffset"])
        row["maximum"] = child(row["minimum"]["endOffset"])
        offset = row["maximum"]["endOffset"]
    elif folded in unary_types:
        row["input"] = child(offset)
        offset = row["input"]["endOffset"]
    elif folded == "fmaterialuniformexpressionappendvector":
        row["a"] = child(offset)
        row["b"] = child(row["a"]["endOffset"])
        offset = row["b"]["endOffset"]
        require(offset + 4 <= len(data), "AppendVector component count is truncated")
        row["componentsFromA"] = struct.unpack_from("<I", data, offset)[0]
        require(
            1 <= row["componentsFromA"] <= 3,
            "AppendVector component count is invalid",
        )
        offset += 4
    elif folded == "fmaterialuniformexpressionsine":
        row["input"] = child(offset)
        offset = row["input"]["endOffset"]
        require(offset + 4 <= len(data), "Sine UBOOL is truncated")
        value = struct.unpack_from("<I", data, offset)[0]
        require(value in (0, 1), "Sine UBOOL is invalid")
        row["isCosine"] = bool(value)
        offset += 4
    elif folded == "fmaterialuniformexpressionconstant":
        require(offset + 17 <= len(data), "constant uniform expression is truncated")
        row["value"] = list(struct.unpack_from("<4f", data, offset))
        require(
            all(math.isfinite(value) for value in row["value"]),
            "constant value is non-finite",
        )
        row["valueTypeOrdinal"] = data[offset + 16]
        require(row["valueTypeOrdinal"] == 15, "constant value type changed")
        offset += 17
    elif folded == "fmaterialuniformexpressionscalarparameter":
        name, name_number, offset = read_fname_at(data, offset, names)
        require(offset + 4 <= len(data), "scalar default is truncated")
        row["parameterName"] = name
        row["parameterNameNumber"] = name_number
        row["defaultValue"] = struct.unpack_from("<f", data, offset)[0]
        require(math.isfinite(row["defaultValue"]), "scalar default is non-finite")
        offset += 4
    elif folded == "fmaterialuniformexpressionvectorparameter":
        name, name_number, offset = read_fname_at(data, offset, names)
        require(offset + 16 <= len(data), "vector default is truncated")
        row["parameterName"] = name
        row["parameterNameNumber"] = name_number
        row["defaultValue"] = list(struct.unpack_from("<4f", data, offset))
        require(
            all(math.isfinite(value) for value in row["defaultValue"]),
            "vector default is non-finite",
        )
        offset += 16
    elif folded in (
        "fmaterialuniformexpressiontime",
        "fmaterialuniformexpressionrealtime",
    ):
        pass
    elif folded == "fmaterialuniformexpressiontexture":
        require(offset + 4 <= len(data), "fixed texture index is truncated")
        row["referencedTextureIndex"] = struct.unpack_from("<i", data, offset)[0]
        require(row["referencedTextureIndex"] >= 0, "fixed texture index is negative")
        offset += 4
    elif folded == "fmaterialuniformexpressiontextureparameter":
        name, name_number, offset = read_fname_at(data, offset, names)
        require(offset + 4 <= len(data), "texture fallback index is truncated")
        row["parameterName"] = name
        row["parameterNameNumber"] = name_number
        row["referencedTextureIndex"] = struct.unpack_from("<i", data, offset)[0]
        require(
            row["referencedTextureIndex"] >= 0,
            "texture fallback index is negative",
        )
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
        require(
            total_top_level <= 4096,
            "uniform-expression top-level denominator is excessive",
        )
        rows = []
        for _ in range(count):
            expression = parse_uniform_expression(
                data, offset, names, node_budget=node_budget
            )
            rows.append(_public_expression(expression))
            offset = expression["endOffset"]
        arrays[array_name] = rows
    raw = data[start:offset]
    semantic = {
        name: [_semantic_expression(row) for row in rows]
        for name, rows in arrays.items()
    }
    return {
        **arrays,
        "offset": start,
        "byteSize": offset - start,
        "rawSha256": sha256_bytes(raw),
        "semanticSha256": canonical_json_sha256(semantic),
        "endOffset": offset,
    }


def scan_base_material_contexts(
    package: dict[str, Any],
    layout: dict[str, Any],
    base_material_ids: list[str],
) -> dict[str, dict[str, Any]]:
    reader: LostArkPackageRangeReader = package["reader"]
    names: list[str] = package["names"]
    start = layout["materialMapScanStartLogicalOffset"]
    patterns = {value: bytes.fromhex(value) for value in base_material_ids}
    hits: dict[str, list[int]] = {value: [] for value in patterns}

    overlap = b""
    cursor = start
    while cursor < reader.logical_size:
        payload = reader.read_logical_range(
            cursor, min(8 * 1024 * 1024, reader.logical_size - cursor)
        )
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

    result: dict[str, dict[str, Any]] = {}
    for base_id, raw_offsets in hits.items():
        offsets = sorted(set(raw_offsets))
        parseable = []
        map_contexts = []
        for absolute in offsets:
            candidate = reader.read_logical_range(
                absolute, min(8192, reader.logical_size - absolute)
            )
            try:
                static_set = parse_static_parameter_set(candidate, 0, names)
            except (KeyError, ValueError, struct.error):
                continue
            if static_set["baseMaterialIdHex"] != base_id:
                continue
            equality = engine_equivalent_static_parameter_set(static_set)
            equality_sha = canonical_json_sha256(equality)
            parsed = {
                "logicalOffset": absolute,
                "staticParameterSetRawSha256": static_set["rawSha256"],
                "engineEqualityStaticParameterSetSha256": equality_sha,
            }
            parseable.append(parsed)
            if static_set["endOffset"] + 20 > len(candidate):
                continue
            suffix = list(
                struct.unpack_from("<IIIII", candidate, static_set["endOffset"])
            )
            if not (
                suffix[0] == package["summary"].version
                and suffix[1] == package["summary"].licensee_version
                and suffix[3] == 0
                and 0 < suffix[4] <= 64
                and absolute < suffix[2] <= reader.logical_size
            ):
                continue
            map_contexts.append(
                {
                    **parsed,
                    "logicalEndOffset": suffix[2],
                    "vertexFactoryCount": suffix[4],
                }
            )
        result[base_id] = {
            "baseMaterialIdHex": base_id,
            "rawHitCount": len(offsets),
            "parseableStaticSetCount": len(parseable),
            "materialMapContextCount": len(map_contexts),
            "parseableStaticSets": parseable,
            "materialMapContexts": map_contexts,
        }
    return result


def select_unique_map_context(
    scan: dict[str, Any], equality_sha256: str
) -> dict[str, Any]:
    candidates = [
        row
        for row in scan["materialMapContexts"]
        if row["engineEqualityStaticParameterSetSha256"] == equality_sha256
    ]
    require(
        len(candidates) == 1,
        "engine-equality material-map context is absent or ambiguous",
    )
    return candidates[0]


def parse_material_map(
    package: dict[str, Any],
    layout: dict[str, Any],
    context: dict[str, Any],
    expected_equality_sha256: str,
) -> dict[str, Any]:
    reader: LostArkPackageRangeReader = package["reader"]
    names: list[str] = package["names"]
    start = context["logicalOffset"]
    end = context["logicalEndOffset"]
    require(start < end, "material-map range is invalid")
    data = reader.read_logical_range(start, end - start)
    static_set = parse_static_parameter_set(data, 0, names)
    equality = engine_equivalent_static_parameter_set(static_set)
    require(
        canonical_json_sha256(equality) == expected_equality_sha256,
        "selected map engine-equality identity changed",
    )
    offset = static_set["endOffset"]
    suffix = list(struct.unpack_from("<IIIII", data, offset))
    offset += 20
    require(
        suffix
        == [
            package["summary"].version,
            package["summary"].licensee_version,
            end,
            0,
            context["vertexFactoryCount"],
        ],
        "material-map suffix changed",
    )

    vertex_factories = []
    for vf_index in range(suffix[4]):
        require(offset + 4 <= len(data), "VF shader-reference count is truncated")
        reference_count = struct.unpack_from("<I", data, offset)[0]
        offset += 4
        require(0 < reference_count <= 256, "VF shader-reference count is invalid")
        references = []
        for _ in range(reference_count):
            shader_type, number, _ = read_fname_at(data, offset, names)
            require(number == 0, "numbered shader reference is unsupported")
            shader_id = data[offset + 8 : offset + 24].hex()
            repeated_type, repeated_number, _ = read_fname_at(
                data, offset + 24, names
            )
            require(
                repeated_number == 0 and repeated_type == shader_type,
                "shader-reference type repeat changed",
            )
            references.append(
                {"shaderType": shader_type, "shaderIdHex": shader_id}
            )
            offset += 32
        vertex_factory, number, offset = read_fname_at(data, offset, names)
        require(number == 0, "numbered vertex factory is unsupported")
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
    require(
        repeated_set["rawSha256"] == static_set["rawSha256"],
        "material-map repeated static set differs",
    )
    require(
        engine_equivalent_static_parameter_set(repeated_set) == equality,
        "material-map repeated engine-equality set differs",
    )
    offset = repeated_set["endOffset"]
    uniform_set = parse_uniform_expression_set(data, offset, names)
    offset = uniform_set["endOffset"]
    require(offset + 4 == len(data), "material-map uniform trailer boundary changed")
    trailer_platform = struct.unpack_from("<I", data, offset)[0]
    require(
        trailer_platform == layout["platform"],
        "material-map trailer platform changed",
    )
    expression_array_names = (
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
    counts = {
        name: len(uniform_set[name]) for name in expression_array_names
    }
    public_uniform = dict(uniform_set)
    public_uniform.pop("endOffset", None)
    return {
        "logicalOffset": start,
        "logicalEndOffset": end,
        "byteSize": len(data),
        "rawSha256": sha256_bytes(data),
        "staticParameterSet": public_static_set(static_set),
        "engineEqualityStaticParameterSet": equality,
        "engineEqualityStaticParameterSetSha256": expected_equality_sha256,
        "suffixU32": suffix,
        "vertexFactories": vertex_factories,
        "opaqueIdentityHex": opaque_identity.hex(),
        "friendlyName": friendly_name,
        "uniformExpressionCounts": counts,
        "uniformExpressionSet": public_uniform,
        "trailerPlatformOrdinal": trailer_platform,
        "runtimeAdmission": False,
        "reason": "REGISTER_BINDING_REPLAY_AND_RUNTIME_ABI_ARE_OUTSIDE_G03_2",
    }


def select_structural_vf_pass_candidate(
    material_map: dict[str, Any],
    renderer_type: str,
    policy: dict[str, Any],
) -> dict[str, Any]:
    """Select bytecode structurally while keeping actual VF/pass unadmitted."""

    family = policy["family"]
    excluded = [str(value).casefold() for value in policy.get("excludeNameContains", [])]
    candidates = []
    for row in material_map["vertexFactories"]:
        folded = row["vertexFactoryType"].casefold()
        if family == "PARTICLE_SPRITE":
            matches = folded.startswith("fparticle") and not any(
                token in folded for token in excluded
            )
        elif family == "LOCAL_MESH":
            matches = folded == str(policy["vertexFactoryType"]).casefold()
        else:
            matches = False
        if matches:
            candidates.append(row)
    require(candidates, f"structural VF candidate is absent: {renderer_type}")

    pass_type = policy["passPixelShaderType"]
    pass_rows: dict[tuple[str, str], dict[str, Any]] = {}
    candidate_rows = []
    for vf in candidates:
        references = [
            row
            for row in vf["shaderReferences"]
            if row["shaderType"].casefold() == pass_type.casefold()
        ]
        require(
            len(references) == 1,
            f"structural BasePass reference is absent or ambiguous: {vf['vertexFactoryType']}",
        )
        reference = references[0]
        key = (reference["shaderType"], reference["shaderIdHex"])
        aggregate = pass_rows.setdefault(
            key,
            {
                "shaderType": reference["shaderType"],
                "shaderIdHex": reference["shaderIdHex"],
                "vertexFactoryTypes": [],
            },
        )
        aggregate["vertexFactoryTypes"].append(vf["vertexFactoryType"])
        candidate_rows.append(
            {
                "vertexFactoryType": vf["vertexFactoryType"],
                "shaderType": reference["shaderType"],
                "shaderIdHex": reference["shaderIdHex"],
            }
        )

    unique_passes = list(pass_rows.values())
    selected = unique_passes[0] if len(unique_passes) == 1 else None
    blockers = ["NATIVE_EMITTER_VERTEX_FACTORY_ABI_UNPROVEN"]
    if len(unique_passes) != 1:
        blockers.append("STRUCTURAL_PIXEL_PASS_ID_IS_AMBIGUOUS")
    return {
        "rendererType": renderer_type,
        "policy": policy,
        "vertexFactoryCandidateCount": len(candidates),
        "vertexFactoryCandidates": candidate_rows,
        "vertexFactoryCandidateStatus": (
            "UNIQUE_STRUCTURAL_VF_TYPE"
            if len(candidates) == 1
            else "MULTIPLE_STRUCTURAL_VF_TYPES"
        ),
        "uniquePixelPassReferenceCount": len(unique_passes),
        "uniquePixelPassReferences": unique_passes,
        "pixelPassCandidateStatus": (
            "UNIQUE_SHARED_PIXEL_SHADER_REFERENCE"
            if selected is not None
            else "AMBIGUOUS_PIXEL_SHADER_REFERENCES"
        ),
        "selectedPixelPassReference": selected,
        "actualVfPassAdmission": False,
        "admissionBlockers": blockers,
    }


def _module_literals(module: dict[str, Any]) -> dict[str, Any]:
    rows = module.get("literals")
    require(isinstance(rows, list), "source module literals are absent")
    result: dict[str, Any] = {}
    for row in rows:
        require(isinstance(row, dict), "source module literal row is invalid")
        path = str(row.get("propertyPath", "")).casefold()
        require(path and path not in result, "source module literal path is duplicated")
        result[path] = row.get("value")
    return result


def _unique_source_module(
    modules: list[dict[str, Any]], class_name: str
) -> dict[str, Any]:
    matches = [
        row
        for row in modules
        if isinstance(row, dict)
        and str(row.get("className", "")).casefold() == class_name.casefold()
    ]
    require(
        len(matches) == 1,
        f"source module is absent or ambiguous: {class_name}",
    )
    return matches[0]


def derive_particle_sprite_vertex_factory_type(
    *, boffsetcenter: bool, has_dynamic_parameter_module: bool
) -> str:
    """Project the two source flags that select this UE3 particle VF branch."""

    require(
        has_dynamic_parameter_module,
        "source DynamicParameter module is required by this VF contract",
    )
    return (
        "fparticleoffsetcenterdynamicparametervertexfactory"
        if boffsetcenter
        else "fparticledynamicparametervertexfactory"
    )


def select_source_emitter_vertex_factory_pass(
    authored_document: dict[str, Any],
    target: dict[str, Any],
    material_map: dict[str, Any],
) -> dict[str, Any]:
    """Join one authored sourceRecipe to one exact VF row and vertex pass."""

    expected = target.get(SOURCE_EMITTER_VF_PASS_FIELD)
    require(isinstance(expected, dict), "source emitter VF/pass contract is absent")
    occurrence_id = expected["occurrenceId"]
    elements = [
        row
        for row in authored_document.get("elements", [])
        if isinstance(row, dict) and row.get("id") == occurrence_id
    ]
    require(
        len(elements) == 1,
        f"authored source occurrence is absent or ambiguous: {occurrence_id}",
    )
    element = elements[0]
    require(
        element.get("kind") == "particle",
        "authored source VF occurrence is not a particle",
    )
    require(
        element.get("material", {}).get("sourceMaterialPath")
        == target["sourceMaterialPath"],
        "authored source occurrence material differs from target",
    )
    recipe = element.get("sourceRecipe")
    require(
        isinstance(recipe, dict)
        and recipe.get("enabled") is True
        and str(recipe.get("rendererShape", "")).casefold() == "sprite",
        "authored source occurrence has no enabled sprite sourceRecipe",
    )
    modules = recipe.get("modules")
    require(isinstance(modules, list), "authored sourceRecipe modules are absent")

    required_module = _unique_source_module(modules, "particlemodulerequired")
    required_literals = _module_literals(required_module)
    for path, value in expected["requiredModule"].items():
        require(
            required_literals.get(path.casefold()) == value,
            f"source Required module literal changed: {path}",
        )

    dynamic_class = expected["dynamicModule"]["className"]
    dynamic_module = _unique_source_module(modules, dynamic_class)
    dynamic_literals = _module_literals(dynamic_module)
    updateflags = dynamic_literals.get("updateflags")
    require(
        isinstance(updateflags, int)
        and not isinstance(updateflags, bool)
        and updateflags == expected["dynamicModule"]["updateflags"],
        "source DynamicParameter updateflags changed",
    )

    selected_vf_type = derive_particle_sprite_vertex_factory_type(
        boffsetcenter=required_literals["boffsetcenter"],
        has_dynamic_parameter_module=True,
    )
    require(
        selected_vf_type.casefold() == expected["vertexFactoryType"].casefold(),
        "source module combination selected a different vertex factory",
    )
    vf_rows = [
        row
        for row in material_map["vertexFactories"]
        if row["vertexFactoryType"].casefold() == selected_vf_type.casefold()
    ]
    require(
        len(vf_rows) == 1,
        "selected emitter vertex-factory row is absent or ambiguous",
    )
    vertex_references = [
        row
        for row in vf_rows[0]["shaderReferences"]
        if row["shaderType"].casefold()
        == expected["vertexShaderType"].casefold()
    ]
    require(
        len(vertex_references) == 1,
        "selected emitter NoDensity vertex shader is absent or ambiguous",
    )
    selected_reference = vertex_references[0]
    require(
        selected_reference["shaderIdHex"] == expected["vertexShaderIdHex"],
        "selected emitter NoDensity vertex shader identity changed",
    )
    rejected_competitors = sorted(
        row["vertexFactoryType"]
        for row in material_map["vertexFactories"]
        if row["vertexFactoryType"].casefold().startswith("fparticle")
        and row["vertexFactoryType"].casefold() != selected_vf_type.casefold()
    )
    return {
        "occurrenceId": occurrence_id,
        "sourceRecipe": {
            "rendererShape": recipe["rendererShape"],
            "requiredModule": {
                "stableId": required_module.get("stableId"),
                "className": required_module.get("className"),
                "boffsetcenter": required_literals["boffsetcenter"],
                "screenalignment": required_literals["screenalignment"],
            },
            "dynamicModule": {
                "stableId": dynamic_module.get("stableId"),
                "className": dynamic_module.get("className"),
                "updateflags": updateflags,
            },
        },
        "sourceEmitterVertexFactorySelection": {
            "selectionRule": (
                "PARTICLE_REQUIRED_BOFFSETCENTER_PLUS_DYNAMIC_PARAMETER_MODULE"
            ),
            "vertexFactoryType": selected_vf_type,
            "rejectedCompetingVertexFactoryTypes": rejected_competitors,
        },
        "authoringVertexPassSelection": {
            "densityPolicy": expected["densityPolicy"],
            "vertexShaderType": selected_reference["shaderType"],
            "vertexShaderIdHex": selected_reference["shaderIdHex"],
        },
        "selectedVertexShaderReference": dict(selected_reference),
    }


def dxbc_chunk_payloads(bytecode: bytes) -> dict[str, bytes]:
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


def parse_dxbc_signature(payload: bytes) -> list[dict[str, Any]]:
    require(len(payload) >= 8, "DXBC signature payload is truncated")
    count, reserved = struct.unpack_from("<II", payload, 0)
    require(reserved in (0, 8), "DXBC signature header changed")
    require(8 + count * 24 <= len(payload), "DXBC signature rows are truncated")
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
        require(name_offset < len(payload), "DXBC semantic name offset is invalid")
        end = payload.find(b"\0", name_offset)
        require(end >= 0, "DXBC semantic name is unterminated")
        require(mask != 0 and mask <= 0xF, "DXBC signature mask is invalid")
        rows.append(
            {
                "semanticName": payload[name_offset:end].decode("ascii", "strict"),
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


def parse_vertex_shader_declaration_closure(
    disassembly: dict[str, Any],
) -> dict[str, Any]:
    profile = str(disassembly.get("profile", ""))
    require(profile.startswith("vs_"), "selected BasePass shader is not a vertex shader")
    counts: dict[str, int] = {}
    for line in disassembly.get("declarations", []):
        for match in re.finditer(r"\bcb(\d+)\[(\d+)\]", line, re.IGNORECASE):
            register, count = match.groups()
            require(register not in counts, "vertex DXBC constant buffer is duplicated")
            counts[register] = int(count)
    require(counts, "vertex DXBC constant-buffer declarations are absent")
    return {
        "profile": profile,
        "constantBufferFloat4Counts": dict(
            sorted(counts.items(), key=lambda row: int(row[0]))
        ),
        "normalizedDisassemblySha256": disassembly["normalizedDisassemblySha256"],
        "declarationSha256": disassembly["declarationSha256"],
        "instructionSha256": disassembly["instructionSha256"],
        "instructionCount": disassembly["instructionCount"],
    }


def close_vertex_pixel_signatures(
    vertex_outputs: list[dict[str, Any]],
    pixel_inputs: list[dict[str, Any]],
) -> dict[str, Any]:
    producers: dict[tuple[str, int], list[dict[str, Any]]] = {}
    for row in vertex_outputs:
        key = (row["semanticName"].casefold(), row["semanticIndex"])
        producers.setdefault(key, []).append(row)
    links = []
    rasterizer_owned = 0
    for consumer in pixel_inputs:
        if (
            consumer["systemValueType"] == 9
            or consumer["semanticName"].casefold() == "sv_isfrontface"
        ):
            rasterizer_owned += 1
            continue
        key = (consumer["semanticName"].casefold(), consumer["semanticIndex"])
        candidates = producers.get(key, [])
        require(
            len(candidates) == 1,
            f"vertex output semantic is absent or ambiguous: {key}",
        )
        producer = candidates[0]
        require(
            producer["componentType"] == consumer["componentType"],
            f"vertex/pixel semantic component type changed: {key}",
        )
        require(
            producer["mask"] & consumer["mask"] == consumer["mask"],
            f"vertex output mask does not cover pixel input: {key}",
        )
        links.append(
            {
                "semanticName": consumer["semanticName"],
                "semanticIndex": consumer["semanticIndex"],
                "producerRegister": producer["register"],
                "producerMask": producer["mask"],
                "consumerRegister": consumer["register"],
                "consumerMask": consumer["mask"],
                "componentType": consumer["componentType"],
            }
        )
    require(links, "vertex/pixel signature closure has no linked semantics")
    return {
        "linkedSemanticCount": len(links),
        "rasterizerOwnedSystemValueCount": rasterizer_owned,
        "links": links,
        "pass": True,
    }


def validate_expected_vertex_shader_identity(
    extracted: dict[str, Any], expected: dict[str, Any]
) -> None:
    expected_dxbc = expected["expectedDxbc"]
    require(
        extracted["shaderType"] == expected["vertexShaderType"]
        and extracted["shaderIdHex"] == expected["vertexShaderIdHex"],
        "exact vertex shader reference identity changed",
    )
    require(
        extracted.get("exactOneDxbcContainer") is True
        and extracted["dxbc"]["byteSize"] == expected_dxbc["byteSize"]
        and extracted["dxbc"]["sha256"] == expected_dxbc["sha256"],
        "exact vertex shader DXBC identity changed",
    )


def complete_source_emitter_vertex_factory_pass(
    selection: dict[str, Any],
    expected: dict[str, Any],
    vertex_shader: dict[str, Any],
    pixel_shader: dict[str, Any],
    disassembler: D3DDisassembler,
) -> dict[str, Any]:
    validate_expected_vertex_shader_identity(vertex_shader, expected)
    vertex_bytecode = vertex_shader["_bytecode"]
    pixel_bytecode = pixel_shader["_bytecode"]
    declaration_closure = parse_vertex_shader_declaration_closure(
        disassembler.disassemble(vertex_bytecode)
    )
    require(
        declaration_closure["constantBufferFloat4Counts"]
        == expected["expectedConstantBufferFloat4Counts"],
        "exact vertex shader constant-buffer declarations changed",
    )
    vertex_chunks = dxbc_chunk_payloads(vertex_bytecode)
    pixel_chunks = dxbc_chunk_payloads(pixel_bytecode)
    require(
        "ISGN" in vertex_chunks
        and "OSGN" in vertex_chunks
        and "ISGN" in pixel_chunks,
        "vertex/pixel signature chunks are absent",
    )
    vertex_inputs = parse_dxbc_signature(vertex_chunks["ISGN"])
    vertex_outputs = parse_dxbc_signature(vertex_chunks["OSGN"])
    pixel_inputs = parse_dxbc_signature(pixel_chunks["ISGN"])
    signature_closure = close_vertex_pixel_signatures(vertex_outputs, pixel_inputs)
    public_vertex_shader = {
        key: value for key, value in vertex_shader.items() if not key.startswith("_")
    }
    return {
        **selection,
        "exactVertexShader": public_vertex_shader,
        "vertexShaderDeclarationClosure": declaration_closure,
        "vertexShaderInputSignature": vertex_inputs,
        "vertexShaderOutputSignature": vertex_outputs,
        "selectedPixelShaderInputSignature": pixel_inputs,
        "vertexPixelSignatureClosure": signature_closure,
        "admission": {
            "sourceEmitterVertexFactorySelection": True,
            "exactVertexShaderBlob": True,
            "exactVertexPixelSignatureClosure": True,
            "authoringNoDensityPass": True,
            "rawVertexShaderExecution": False,
            "sourceExactVertexCb": False,
            "productVfPass": False,
            "runtime": False,
            "visual": False,
        },
    }


def extract_selected_packed_dxbc(
    package: dict[str, Any],
    layout: dict[str, Any],
    selected_references: list[dict[str, str]],
) -> dict[str, dict[str, Any]]:
    """Resolve shader IDs with Artist's proven packed descriptor primitive."""

    wanted: dict[str, str] = {}
    for reference in selected_references:
        shader_id = reference["shaderIdHex"]
        shader_type = reference["shaderType"]
        if shader_id in wanted:
            require(
                wanted[shader_id] == shader_type,
                "one shader ID is requested with multiple shader types",
            )
        wanted[shader_id] = shader_type
    require(wanted, "selected shader references are absent")

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
    group_count = cursor.u32()
    require(
        platform == layout["platform"] and group_count == layout["groupCount"],
        "packed descriptor scan header differs from G03-1 layout",
    )

    selected_descriptors: dict[str, dict[str, Any]] = {}
    wanted_types = set(wanted.values())
    for group_index in range(group_count):
        shader_type, shader_type_number = cursor.fname(names)
        require(shader_type_number == 0, "numbered shader type is unsupported")
        descriptor_count = cursor.u32()
        descriptor_offset = cursor.offset
        descriptor_bytes = cursor.read(descriptor_count * 24)
        code_count = cursor.u32()
        code_positions = []
        for code_blob_index in range(code_count):
            header_offset = cursor.offset
            uncompressed_size = cursor.u32()
            compressed_size = cursor.u32()
            compressed_offset = cursor.offset
            require(
                32 <= uncompressed_size <= 64 * 1024 * 1024,
                "packed code-blob uncompressed size is invalid",
            )
            require(
                0 < compressed_size <= reader.logical_size - cursor.offset,
                "packed code-blob compressed size is invalid",
            )
            cursor.skip(compressed_size)
            code_positions.append(
                {
                    "codeIndex": code_blob_index,
                    "codeHeaderLogicalOffset": header_offset,
                    "compressedLogicalOffset": compressed_offset,
                    "compressedByteSize": compressed_size,
                    "uncompressedByteSize": uncompressed_size,
                }
            )

        if shader_type not in wanted_types:
            continue
        for descriptor_index in range(descriptor_count):
            raw = descriptor_bytes[
                descriptor_index * 24 : (descriptor_index + 1) * 24
            ]
            shader_id = raw[:16].hex()
            if shader_id not in wanted:
                continue
            require(
                wanted[shader_id] == shader_type,
                "selected descriptor shader type changed",
            )
            require(
                shader_id not in selected_descriptors,
                "selected shader descriptor is duplicated",
            )
            decoded = decode_packed_shader_code_slice(raw, code_positions)
            selected_descriptors[shader_id] = {
                "shaderType": shader_type,
                "shaderIdHex": shader_id,
                "groupIndex": group_index,
                "descriptorIndex": descriptor_index,
                "descriptorLogicalOffset": descriptor_offset
                + descriptor_index * 24,
                "descriptorRawSha256": sha256_bytes(raw),
                "packedIndexAndOffsetU32": struct.unpack_from("<I", raw, 16)[0],
                "packedSizeU32": struct.unpack_from("<I", raw, 20)[0],
                **decoded,
            }

    require(
        cursor.offset == layout["shaderCodeSectionEndLogicalOffset"],
        "packed descriptor scan code-section end changed",
    )
    require(
        set(selected_descriptors) == set(wanted),
        "selected packed descriptor denominator changed",
    )

    blob_cache: dict[tuple[int, int], tuple[bytes, bytes]] = {}
    result: dict[str, dict[str, Any]] = {}
    for shader_id, descriptor in selected_descriptors.items():
        position = descriptor["codeBlobPosition"]
        cache_key = (descriptor["groupIndex"], descriptor["codeBlobIndex"])
        if cache_key not in blob_cache:
            compressed = reader.read_logical_range(
                position["compressedLogicalOffset"],
                position["compressedByteSize"],
            )
            blob = decompress_lz4_block(
                compressed, position["uncompressedByteSize"]
            )
            blob_cache[cache_key] = (compressed, blob)
        compressed, blob = blob_cache[cache_key]
        slice_offset = descriptor["sliceOffsetInUncompressedBlob"]
        slice_size = descriptor["sliceByteSize"]
        bytecode = blob[slice_offset : slice_offset + slice_size]
        require(len(bytecode) == slice_size, "selected DXBC slice is truncated")
        container = validate_dxbc_container(bytecode)
        public_descriptor = dict(descriptor)
        public_descriptor.pop("codeBlobPosition", None)
        result[shader_id] = {
            "shaderType": descriptor["shaderType"],
            "shaderIdHex": shader_id,
            "packedDescriptor": public_descriptor,
            "compressedCodeBlob": {
                **position,
                "compressedSha256": sha256_bytes(compressed),
                "uncompressedSha256": sha256_bytes(blob),
            },
            "dxbc": container,
            "exactOneDxbcContainer": True,
            "actualVfPassAdmission": False,
            "admissionBlocker": "NATIVE_SHADER_OBJECT_BINDING_AND_VF_ABI_UNPROVEN",
            "_bytecode": bytecode,
        }
    return result


def extract_selected_shader_objects(
    package: dict[str, Any],
    layout: dict[str, Any],
    selected_references: list[dict[str, str]],
) -> dict[str, Any]:
    """Walk the native shader-object table by serialized absolute end pointers.

    No target offset, object size, array offset, or selected-object count is
    supplied by the manifest.  The table denominator comes from the cache's
    own native header, while shaderType FName + shader ID selects the objects.
    """

    wanted: dict[str, str] = {}
    for reference in selected_references:
        shader_id = reference["shaderIdHex"]
        shader_type = reference["shaderType"]
        previous = wanted.get(shader_id)
        require(
            previous is None or previous == shader_type,
            "one shader ID is requested with multiple shader types",
        )
        wanted[shader_id] = shader_type
    require(wanted, "selected shader-object references are absent")

    reader: LostArkPackageRangeReader = package["reader"]
    names: list[str] = package["names"]
    header_offset = layout["shaderCodeSectionEndLogicalOffset"]
    cursor = BufferedLogicalCursor(reader, header_offset)
    platform = cursor.u32()
    object_count = cursor.u32()
    require(platform == layout["platform"], "shader-object table platform changed")
    require(
        object_count == layout["shaderObjectCount"],
        "shader-object table denominator changed",
    )
    table_start = cursor.offset
    selected: dict[str, dict[str, Any]] = {}
    for object_index in range(object_count):
        logical_offset = cursor.offset
        header = cursor.read(48)
        name_index, name_number = struct.unpack_from("<ii", header, 0)
        require(0 <= name_index < len(names), "shader-object FName index is invalid")
        require(name_number == 0, "numbered shader-object type is unsupported")
        shader_type = names[name_index]
        shader_id = header[8:24].hex()
        serialized_code_sha1 = header[24:44]
        logical_end = struct.unpack_from("<I", header, 44)[0]
        require(
            logical_offset + 48 < logical_end <= reader.logical_size,
            "shader-object serialized end pointer is invalid",
        )
        byte_size = logical_end - logical_offset
        require(byte_size <= 64 * 1024 * 1024, "shader-object byte size is excessive")
        if shader_id in wanted:
            require(
                shader_type == wanted[shader_id],
                "selected shader-object type differs from its descriptor",
            )
            require(shader_id not in selected, "selected shader object is duplicated")
            payload = header + cursor.read(byte_size - 48)
            selected[shader_id] = {
                "shaderObjectIndex": object_index,
                "shaderType": shader_type,
                "shaderTypeNameNumber": name_number,
                "shaderIdHex": shader_id,
                "logicalOffset": logical_offset,
                "logicalEndOffset": logical_end,
                "byteSize": byte_size,
                "rawSha256": sha256_bytes(payload),
                "serializedShaderCodeSha1Hex": serialized_code_sha1.hex(),
                "serializedShaderCodeSha1Sha256": sha256_bytes(serialized_code_sha1),
                "_bytes": payload,
            }
        else:
            cursor.skip(byte_size - 48)
        require(cursor.offset == logical_end, "shader-object traversal lost contiguity")

    table_end = cursor.offset
    require(
        set(selected) == set(wanted),
        "selected shader-object denominator changed",
    )
    material_map_count = cursor.u32()
    require(0 < material_map_count <= 1_000_000, "material-map denominator is invalid")
    common_sha1 = {
        row["serializedShaderCodeSha1Hex"] for row in selected.values()
    }
    require(
        len(common_sha1) == 1,
        "selected shader objects do not share one serialized code preamble",
    )
    common_value = bytes.fromhex(next(iter(common_sha1)))
    return {
        "table": {
            "platform": platform,
            "shaderObjectCount": object_count,
            "logicalOffset": table_start,
            "logicalEndOffset": table_end,
            "materialMapCountFollowingTable": material_map_count,
            "traversal": "SERIALIZED_ABSOLUTE_END_POINTER_MONOTONIC_CONTIGUOUS",
            "selectedCommonShaderCodeSha1Hex": common_value.hex(),
            "selectedCommonShaderCodeSha1Sha256": sha256_bytes(common_value),
        },
        "byShaderId": selected,
    }


def parse_dxbc_declaration_closure(disassembly: dict[str, Any]) -> dict[str, Any]:
    """Project the DXBC declarations and sample pairs needed by native wires."""

    profile = str(disassembly.get("profile", ""))
    require(profile.startswith("ps_"), "selected BasePass shader is not a pixel shader")
    declarations = list(disassembly.get("declarations", []))
    cb0_sizes = [
        int(match.group(1))
        for line in declarations
        for match in [re.search(r"\bcb0\[(\d+)\]", line, re.IGNORECASE)]
        if match
    ]
    require(len(cb0_sizes) == 1, "DXBC CB0 declaration is absent or ambiguous")
    declared_textures = sorted(
        {
            int(match.group(1))
            for line in declarations
            if line.startswith("dcl_resource_")
            for match in [re.search(r"\bt(\d+)\b", line, re.IGNORECASE)]
            if match
        }
    )
    declared_samplers = sorted(
        {
            int(match.group(1))
            for line in declarations
            if line.startswith("dcl_sampler ")
            for match in [re.search(r"\bs(\d+)\b", line, re.IGNORECASE)]
            if match
        }
    )
    sample_pair_counts: dict[str, int] = {}
    for line in disassembly.get("instructions", []):
        if not re.match(r"^sample(?:_|\s)", line, re.IGNORECASE):
            continue
        registers = re.findall(r"\b([ts]\d+)\b", line, re.IGNORECASE)
        require(len(registers) == 2, "DXBC sample register pair is ambiguous")
        pair = f"{registers[0].casefold()}/{registers[1].casefold()}"
        sample_pair_counts[pair] = sample_pair_counts.get(pair, 0) + 1
    require(sample_pair_counts, "DXBC contains no texture/sampler sample pair")
    sampled_textures = sorted(
        {int(pair.split("/")[0][1:]) for pair in sample_pair_counts}
    )
    sampled_samplers = sorted(
        {int(pair.split("/")[1][1:]) for pair in sample_pair_counts}
    )
    require(
        declared_textures == sampled_textures,
        "DXBC declared texture registers differ from sampled registers",
    )
    require(
        declared_samplers == sampled_samplers,
        "DXBC declared sampler registers differ from sampled registers",
    )
    return {
        "profile": profile,
        "normalizedDisassemblySha256": disassembly["normalizedDisassemblySha256"],
        "declarationSha256": disassembly["declarationSha256"],
        "instructionSha256": disassembly["instructionSha256"],
        "instructionCount": disassembly["instructionCount"],
        "declaredConstantBuffer0Float4Count": cb0_sizes[0],
        "declaredTextureRegisters": declared_textures,
        "declaredSamplerRegisters": declared_samplers,
        "observedSamplePairCounts": dict(sorted(sample_pair_counts.items())),
    }


def _parse_wire_array(
    payload: bytes,
    offset: int,
    name: str,
    expected_count: int | None,
    maximum_count: int,
    object_logical_offset: int,
    *,
    allow_empty: bool = False,
) -> tuple[list[dict[str, int]], int]:
    require(offset + 4 <= len(payload), f"{name} binding count is truncated")
    count = struct.unpack_from("<I", payload, offset)[0]
    offset += 4
    minimum_count = 0 if allow_empty else 1
    require(
        minimum_count <= count <= maximum_count,
        f"{name} binding denominator is invalid",
    )
    if expected_count is not None:
        require(count == expected_count, f"{name} binding denominator changed")
    require(offset + count * 10 <= len(payload), f"{name} bindings are truncated")
    rows = []
    for _ in range(count):
        row_offset = offset
        wire_index, base_index, count_or_size, buffer_or_sampler = struct.unpack_from(
            "<IHHH", payload, offset
        )
        rows.append(
            {
                "expressionIndexOrGroup": wire_index,
                "baseIndex": base_index,
                "numBytesOrResources": count_or_size,
                "bufferIndexOrSamplerIndex": buffer_or_sampler,
                "logicalOffset": object_logical_offset + row_offset,
            }
        )
        offset += 10
    return rows, offset


def scan_native_binding_array_candidates(
    object_bytes: bytes,
    object_logical_offset: int,
    uniform_counts: dict[str, int],
    dxbc_closure: dict[str, Any],
) -> list[dict[str, Any]]:
    """Find every three-array wire triple satisfying native+DXBC closure."""

    scalar_count = int(uniform_counts["pixelScalarExpressions"])
    vector_count = int(uniform_counts["pixelVectorExpressions"])
    texture_count = int(uniform_counts["pixelTexture2DExpressions"])
    scalar_group_count = math.ceil(scalar_count / 4)
    require(
        scalar_group_count > 0 and vector_count > 0 and texture_count > 0,
        "G03-3 requires non-empty scalar/vector/texture expression denominators",
    )
    cb0_size = int(dxbc_closure["declaredConstantBuffer0Float4Count"])
    declared_textures = set(dxbc_closure["declaredTextureRegisters"])
    declared_samplers = set(dxbc_closure["declaredSamplerRegisters"])
    observed_pairs = set(dxbc_closure["observedSamplePairCounts"])
    candidates = []
    for start in range(48, max(48, len(object_bytes) - 11)):
        try:
            offset = start
            scalar_rows, offset = _parse_wire_array(
                object_bytes,
                offset,
                "scalarGroups",
                None,
                scalar_group_count,
                object_logical_offset,
            )
            vector_rows, offset = _parse_wire_array(
                object_bytes,
                offset,
                "vectors",
                None,
                vector_count,
                object_logical_offset,
                allow_empty=True,
            )
            texture_rows, offset = _parse_wire_array(
                object_bytes,
                offset,
                "textures",
                None,
                texture_count,
                object_logical_offset,
            )

            scalar_keys = [row["expressionIndexOrGroup"] for row in scalar_rows]
            vector_keys = [row["expressionIndexOrGroup"] for row in vector_rows]
            texture_keys = [row["expressionIndexOrGroup"] for row in texture_rows]
            require(
                len(set(scalar_keys)) == len(scalar_keys)
                and set(scalar_keys).issubset(range(scalar_group_count)),
                "scalar group keys do not close over uniform expressions",
            )
            require(
                len(set(vector_keys)) == len(vector_keys)
                and set(vector_keys).issubset(range(vector_count)),
                "vector keys do not close over uniform expressions",
            )
            require(
                len(set(texture_keys)) == len(texture_keys)
                and set(texture_keys).issubset(range(texture_count)),
                "texture keys do not close over uniform expressions",
            )
            constant_rows = scalar_rows + vector_rows
            require(
                all(
                    row["numBytesOrResources"] == 16
                    and row["bufferIndexOrSamplerIndex"] == 0
                    and row["baseIndex"] % 16 == 0
                    for row in constant_rows
                ),
                "constant-buffer wire shape changed",
            )
            cb_slots = [row["baseIndex"] // 16 for row in constant_rows]
            require(
                len(cb_slots) == len(set(cb_slots)),
                "native constant-buffer slots overlap",
            )
            require(
                cb_slots and max(cb_slots) + 1 == cb0_size,
                "native wires do not close over the DXBC CB0 declaration",
            )
            require(
                all(row["numBytesOrResources"] == 1 for row in texture_rows),
                "texture binding wire shape changed",
            )
            material_pairs = {
                f"t{row['baseIndex']}/s{row['bufferIndexOrSamplerIndex']}"
                for row in texture_rows
            }
            require(
                len(material_pairs) == len(texture_rows),
                "native texture binding pairs overlap",
            )
            require(
                material_pairs.issubset(observed_pairs),
                "native texture wires are not sampled by the DXBC",
            )
            material_textures = {row["baseIndex"] for row in texture_rows}
            material_samplers = {
                row["bufferIndexOrSamplerIndex"] for row in texture_rows
            }
            require(
                material_textures.issubset(declared_textures)
                and material_samplers.issubset(declared_samplers),
                "native texture wires are outside DXBC declarations",
            )
            extra_pairs = observed_pairs - material_pairs
            for pair in extra_pairs:
                texture_text, sampler_text = pair.split("/")
                require(
                    int(texture_text[1:]) not in material_textures
                    and int(sampler_text[1:]) not in material_samplers,
                    "non-material engine sample pair conflicts with material ABI",
                )

            arrays_bytes = object_bytes[start:offset]
            semantic_rows = {
                name: [
                    {
                        key: value
                        for key, value in row.items()
                        if key != "logicalOffset"
                    }
                    for row in rows
                ]
                for name, rows in (
                    ("scalarGroups", scalar_rows),
                    ("vectors", vector_rows),
                    ("textures", texture_rows),
                )
            }
            candidate = {
                "bindingArraysOffsetInShaderObject": start,
                "bindingArraysByteSize": offset - start,
                "bindingArraysRawSha256": sha256_bytes(arrays_bytes),
                **semantic_rows,
                "constantBufferClosure": {
                    "declaredConstantBuffer0Float4Count": cb0_size,
                    "maximumNativeBoundConstantBuffer0Slot": max(cb_slots),
                    "boundConstantBuffer0Slots": sorted(cb_slots),
                },
                "textureSampleClosure": {
                    "materialSamplePairs": sorted(material_pairs),
                    "unownedEngineSamplePairs": sorted(extra_pairs),
                    "allObservedSamplePairCounts": dxbc_closure[
                        "observedSamplePairCounts"
                    ],
                },
            }
            candidate["bindingSemanticSha256"] = canonical_json_sha256(
                {
                    key: candidate[key]
                    for key in (
                        "scalarGroups",
                        "vectors",
                        "textures",
                        "constantBufferClosure",
                        "textureSampleClosure",
                    )
                }
            )
            candidates.append(candidate)
        except (ValueError, struct.error, KeyError, IndexError):
            continue
    return candidates


def select_unique_native_binding_arrays(
    object_bytes: bytes,
    object_logical_offset: int,
    uniform_counts: dict[str, int],
    dxbc_closure: dict[str, Any],
) -> dict[str, Any]:
    candidates = scan_native_binding_array_candidates(
        object_bytes,
        object_logical_offset,
        uniform_counts,
        dxbc_closure,
    )
    require(
        len(candidates) == 1,
        f"native binding-array candidate is absent or ambiguous: {len(candidates)}",
    )
    return {**candidates[0], "candidateCount": 1}


def validate_cache_identity(
    package: dict[str, Any],
    layout: dict[str, Any],
    expected: dict[str, Any],
) -> None:
    identity = package["identity"]
    for field in (
        "fileName",
        "physicalByteSize",
        "rawSha256",
        "logicalByteSize",
        "packageVersion",
        "licenseeVersion",
        "packageGuidHex",
    ):
        require(identity[field] == expected[field], f"official cache {field} changed")
    expected_layout = expected["shaderCodeLayout"]
    for field in (
        "platform",
        "groupCount",
        "descriptorCount",
        "codeBlobCount",
        "shaderCodeSectionEndLogicalOffset",
    ):
        require(layout[field] == expected_layout[field], f"cache layout {field} changed")


def validate_source_package(
    path: Path, expected: dict[str, Any]
) -> dict[str, Any]:
    require(path.is_file(), f"pinned source package is missing: {path}")
    identity = {
        "fileName": path.name,
        "physicalByteSize": path.stat().st_size,
        "rawSha256": digest_file(path),
        "rawMd5": digest_file(path, "md5"),
    }
    require(identity["fileName"] == expected["fileName"], "source package name changed")
    require(
        identity["physicalByteSize"] == expected["physicalByteSize"],
        f"source package size changed: {path.name}",
    )
    require(
        identity["rawSha256"] == expected["rawSha256"],
        f"source package SHA changed: {path.name}",
    )
    relationship = expected["officialManifest975Relationship"]
    official = expected["officialManifest975"]
    if relationship == "EXACT_EXTRACTED_PAYLOAD":
        require(
            identity["physicalByteSize"] == official["extractedByteSize"]
            and identity["rawMd5"] == official["extractedMd5"],
            f"source package no longer matches manifest-975: {path.name}",
        )
    elif relationship == "PINNED_SNAPSHOT_DRIFTED_FROM_EXTRACTED_PAYLOAD":
        require(
            identity["physicalByteSize"] != official["extractedByteSize"]
            or identity["rawMd5"] != official["extractedMd5"],
            f"source package drift declaration is stale: {path.name}",
        )
    else:
        raise ValueError(f"unknown source relationship: {relationship}")
    return {**identity, **expected}


def build_receipt(
    targets_path: Path,
    source_root: Path,
    cache_path: Path,
    d3dcompiler_path: Path,
) -> dict[str, Any]:
    targets_document = read_json(targets_path)
    targets = validate_target_manifest(targets_document)
    inputs = targets_document["inputs"]
    vf_policies = targets_document["vertexFactoryPolicies"]
    authored_document, authored_identity = load_authored_effect_document(
        inputs["authoredEffectDocument"]
    )

    source_rows = {
        row["fileName"].casefold(): row for row in inputs["sourcePackages"]
    }
    source_packages: dict[str, Any] = {}
    source_identities = []
    for key in sorted(source_rows):
        expected = source_rows[key]
        path = source_root / expected["fileName"]
        source_identities.append(validate_source_package(path, expected))
        source_packages[key] = load_package(path, LOSTARK_KR_AES_KEY)

    decoded_targets = []
    for target in targets:
        package = source_packages[target["sourcePackageFileName"].casefold()]
        decoded_targets.append(
            {
                "targetId": target["targetId"],
                "familyId": target["familyId"],
                "rendererType": target["rendererType"],
                "occurrenceIds": target["occurrenceIds"],
                "sourceMaterialPath": target["sourceMaterialPath"],
                "parentMaterialPath": target["parentMaterialPath"],
                "baseMaterialIdHex": target["baseMaterialIdHex"],
                "expectedStatus": target["expectedStatus"],
                "mic": decode_mic_target(package, target),
            }
        )

    require(cache_path.is_file(), f"pinned official RefShaderCache is missing: {cache_path}")
    cache = package_tables(cache_path)
    layout = parse_shader_code_layout(cache)
    validate_cache_identity(cache, layout, inputs["officialRefShaderCache"])
    scans = scan_base_material_contexts(
        cache,
        layout,
        [target["baseMaterialIdHex"] for target in targets],
    )

    final_targets = []
    exact_count = 0
    blocked_count = 0
    targets_by_id = {target["targetId"]: target for target in targets}
    for decoded in decoded_targets:
        target = targets_by_id[decoded["targetId"]]
        scan = scans[target["baseMaterialIdHex"]]
        expected_raw_hits = target.get("expectedBaseMaterialRawHitCount")
        if expected_raw_hits is not None:
            require(
                scan["rawHitCount"] == expected_raw_hits,
                f"base Material hit denominator changed: {target['targetId']}",
            )
        if decoded["mic"]["status"] == STATUS_BLOCKED:
            require(
                target["expectedStatus"] == STATUS_BLOCKED,
                f"unexpected blocked target: {target['targetId']}",
            )
            final_targets.append(
                {
                    **decoded,
                    "status": STATUS_BLOCKED,
                    "blocker": STATUS_BLOCKED,
                    "baseMaterialScan": scan,
                    "materialMap": None,
                    "parentDefaultFallbackApplied": False,
                }
            )
            blocked_count += 1
            continue

        equality_sha = decoded["mic"][
            "engineEqualityStaticParameterSetSha256"
        ]
        context = select_unique_map_context(scan, equality_sha)
        expected_map = target.get("expectedMaterialMap")
        if expected_map:
            for field in (
                "logicalOffset",
                "logicalEndOffset",
                "vertexFactoryCount",
            ):
                require(
                    context[field] == expected_map[field],
                    f"material-map {field} changed: {target['targetId']}",
                )
        material_map = parse_material_map(cache, layout, context, equality_sha)
        structural_selection = select_structural_vf_pass_candidate(
            material_map,
            target["rendererType"],
            vf_policies[target["rendererType"]],
        )
        selected_reference = structural_selection["selectedPixelPassReference"]
        require(
            selected_reference is not None,
            f"structural pixel pass is ambiguous: {target['targetId']}",
        )
        expected_selection = target.get("expectedStructuralSelection")
        if expected_selection:
            require(
                structural_selection["vertexFactoryCandidateCount"]
                == expected_selection["vertexFactoryCandidateCount"],
                f"structural VF candidate denominator changed: {target['targetId']}",
            )
            require(
                selected_reference["shaderType"]
                == expected_selection["shaderType"]
                and selected_reference["shaderIdHex"]
                == expected_selection["shaderIdHex"],
                f"structural pixel pass identity changed: {target['targetId']}",
            )
        require(
            target["expectedStatus"] == STATUS_EXACT,
            f"unexpected exact target: {target['targetId']}",
        )
        final_target = {
            **decoded,
            "status": STATUS_EXACT,
            "baseMaterialScan": scan,
            "materialMap": material_map,
            "structuralVfPassCandidate": structural_selection,
            "parentDefaultFallbackApplied": False,
        }
        if SOURCE_EMITTER_VF_PASS_FIELD in target:
            final_target["sourceEmitterVertexFactoryPass"] = (
                select_source_emitter_vertex_factory_pass(
                    authored_document,
                    target,
                    material_map,
                )
            )
        final_targets.append(final_target)
        exact_count += 1

    selected_references = [
        target["structuralVfPassCandidate"]["selectedPixelPassReference"]
        for target in final_targets
        if target["status"] == STATUS_EXACT
    ]
    selected_vertex_references = [
        target["sourceEmitterVertexFactoryPass"]["selectedVertexShaderReference"]
        for target in final_targets
        if isinstance(target.get("sourceEmitterVertexFactoryPass"), dict)
    ]
    exact_dxbc = extract_selected_packed_dxbc(
        cache,
        layout,
        selected_references + selected_vertex_references,
    )
    unique_pixel_shader_ids = {
        row["shaderIdHex"] for row in selected_references
    }
    exact_dxbc_count = 0
    for target in final_targets:
        if target["status"] != STATUS_EXACT:
            target["cookedPixelShader"] = None
            target["nativeShaderObjectBinding"] = None
            continue
        shader_id = target["structuralVfPassCandidate"][
            "selectedPixelPassReference"
        ]["shaderIdHex"]
        target["cookedPixelShader"] = {
            key: value
            for key, value in exact_dxbc[shader_id].items()
            if not key.startswith("_")
        }
        manifest_target = targets_by_id[target["targetId"]]
        expected_dxbc = manifest_target["expectedStructuralSelection"].get(
            "expectedDxbc"
        )
        if expected_dxbc:
            require(
                target["cookedPixelShader"]["dxbc"]["byteSize"]
                == expected_dxbc["byteSize"]
                and target["cookedPixelShader"]["dxbc"]["sha256"]
                == expected_dxbc["sha256"],
                f"selected DXBC identity changed: {target['targetId']}",
            )
        exact_dxbc_count += 1

    disassembler = D3DDisassembler(d3dcompiler_path)
    source_emitter_vf_pass_count = 0
    for target in final_targets:
        selection = target.get("sourceEmitterVertexFactoryPass")
        if not isinstance(selection, dict):
            continue
        manifest_target = targets_by_id[target["targetId"]]
        expected = manifest_target[SOURCE_EMITTER_VF_PASS_FIELD]
        vertex_shader_id = selection["selectedVertexShaderReference"][
            "shaderIdHex"
        ]
        pixel_shader_id = target["structuralVfPassCandidate"][
            "selectedPixelPassReference"
        ]["shaderIdHex"]
        target["sourceEmitterVertexFactoryPass"] = (
            complete_source_emitter_vertex_factory_pass(
                selection,
                expected,
                exact_dxbc[vertex_shader_id],
                exact_dxbc[pixel_shader_id],
                disassembler,
            )
        )
        source_emitter_vf_pass_count += 1

    selected_objects = extract_selected_shader_objects(
        cache,
        layout,
        selected_references,
    )
    exact_native_binding_count = 0
    blocked_native_binding_count = 0
    for target in final_targets:
        if target["status"] != STATUS_EXACT:
            continue
        shader_id = target["structuralVfPassCandidate"][
            "selectedPixelPassReference"
        ]["shaderIdHex"]
        shader_object = selected_objects["byShaderId"][shader_id]
        bytecode = exact_dxbc[shader_id]["_bytecode"]
        public_object = {
            key: value
            for key, value in shader_object.items()
            if not key.startswith("_")
        }
        try:
            dxbc_closure = parse_dxbc_declaration_closure(
                disassembler.disassemble(bytecode)
            )
            candidates = scan_native_binding_array_candidates(
                shader_object["_bytes"],
                shader_object["logicalOffset"],
                target["materialMap"]["uniformExpressionCounts"],
                dxbc_closure,
            )
        except ValueError as error:
            target["nativeShaderObjectBinding"] = {
                "status": "BLOCKED_DXBC_DECLARATION_OR_NATIVE_WIRE_SHAPE",
                "blocker": str(error),
                "shaderObject": public_object,
                "runtimeAdmission": False,
                "actualVfPassAdmission": False,
            }
            blocked_native_binding_count += 1
            continue
        if len(candidates) != 1:
            target["nativeShaderObjectBinding"] = {
                "status": "BLOCKED_NATIVE_BINDING_ARRAY_CANDIDATE_ABSENT_OR_AMBIGUOUS",
                "shaderObject": public_object,
                "bindingArrayCandidateCount": len(candidates),
                "bindingArrayCandidateOffsets": [
                    row["bindingArraysOffsetInShaderObject"] for row in candidates
                ],
                "dxbcDeclarationClosure": dxbc_closure,
                "runtimeAdmission": False,
                "actualVfPassAdmission": False,
            }
            blocked_native_binding_count += 1
            continue
        binding = candidates[0]
        target["nativeShaderObjectBinding"] = {
            "status": "EXACT_NATIVE_SHADER_OBJECT_BINDING",
            "shaderObject": public_object,
            "bindingArrayCandidateCount": 1,
            "bindingArraysOffsetInShaderObject": binding[
                "bindingArraysOffsetInShaderObject"
            ],
            "bindingArraysByteSize": binding["bindingArraysByteSize"],
            "bindingArraysRawSha256": binding["bindingArraysRawSha256"],
            "bindingSemanticSha256": binding["bindingSemanticSha256"],
            "scalarGroups": binding["scalarGroups"],
            "vectors": binding["vectors"],
            "textures": binding["textures"],
            "constantBufferClosure": binding["constantBufferClosure"],
            "textureSampleClosure": binding["textureSampleClosure"],
            "dxbcDeclarationClosure": dxbc_closure,
            "wireEntryFormat": (
                "<u32 expressionIndexOrPackedScalarGroup,u16 baseByteOrResourceIndex,"
                "u16 numBytesOrResources,u16 bufferOrSamplerIndex>"
            ),
            "selectionFidelity": (
                "SHADER_TYPE_FNAME_PLUS_SHADER_ID;_SERIALIZED_OBJECT_END_POINTER;_"
                "UNIQUE_SCALAR_VECTOR_TEXTURE_WIRE_CLOSURE_WITH_UNIFORM_"
                "DENOMINATORS_AND_DXBC_CB0_TEXTURE_SAMPLER_DECLARATIONS"
            ),
            "runtimeAdmission": False,
            "actualVfPassAdmission": False,
        }
        exact_native_binding_count += 1

    summary = targets_document["summary"]
    require(exact_count == summary["expectedExactCount"], "exact result denominator changed")
    require(blocked_count == summary["expectedBlockedCount"], "blocked result denominator changed")
    require(
        exact_dxbc_count == summary["expectedExactDxbcCount"],
        "exact DXBC result denominator changed",
    )
    require(
        source_emitter_vf_pass_count
        == summary["expectedSourceEmitterVertexFactoryPassCount"],
        "source emitter VF/pass result denominator changed",
    )
    overall_result = (
        "PASS_G03_6_EXACT_NATIVE_BINDINGS_AND_SOURCE_EMITTER_VF_PASS_EVIDENCE"
        if exact_native_binding_count == summary["expectedExactNativeBindingCount"]
        and blocked_native_binding_count == 0
        else "BLOCKED_G03_6_NATIVE_BINDING_OR_SOURCE_EMITTER_VF_PASS_EVIDENCE"
    )
    receipt = {
        "schema": RECEIPT_SCHEMA,
        "formatVersion": RECEIPT_FORMAT_VERSION,
        "identity": targets_document["identity"],
        "scope": {
            "stage": "G03_6_SOURCE_EMITTER_VF_NO_DENSITY_VERTEX_PASS_EVIDENCE",
            "classNeutralExtractor": True,
            "runtimeAdmission": False,
            "visualAdmission": False,
            "dxbcExtracted": True,
            "nativeShaderObjectBindingsDecoded": True,
            "nativeShaderObjectBindingRuntimeAdmission": False,
            "sourceEmitterVertexFactorySelection": True,
            "exactVertexShaderBlob": True,
            "exactVertexPixelSignatureClosure": True,
            "authoringNoDensityPass": True,
            "densityPolicy": DENSITY_NO_DENSITY_AUTHORING_BOUNDED,
            "rawVertexShaderExecution": False,
            "sourceExactVertexCb": False,
            "productVfPass": False,
            "installedShaderCacheRead": False,
            "installedShaderCachePolicy": "EXCLUDED_PINNED_V974_IS_AUTHORITATIVE",
        },
        "inputs": {
            "targetManifest": {
                "repoRelativePath": targets_path.relative_to(
                    REPOSITORY_ROOT
                ).as_posix(),
                "rawSha256": digest_file(targets_path),
            },
            "extractor": {
                "repoRelativePath": Path(__file__).resolve().relative_to(
                    REPOSITORY_ROOT
                ).as_posix(),
                "rawSha256": digest_file(Path(__file__).resolve()),
            },
            "authoredEffectDocument": authored_identity,
            "sourcePackages": source_identities,
            "d3dcompiler": disassembler.identity,
        },
        "officialRefShaderCache": {
            "package": cache["identity"],
            "shaderCodeLayout": layout,
            "shaderObjectTable": selected_objects["table"],
        },
        "targets": final_targets,
        "summary": {
            "targetCount": len(final_targets),
            "exactMaterialShaderMapCount": exact_count,
            "blockedNoEffectiveStaticSetAbiEvidenceCount": blocked_count,
            "parentDefaultFallbackCount": 0,
            "uniformExpressionSetDecodedCount": exact_count,
            "exactPixelShaderDxbcCount": exact_dxbc_count,
            "uniqueExactPixelShaderDxbcCount": len(unique_pixel_shader_ids),
            "sourceEmitterVertexFactorySelectionCount": source_emitter_vf_pass_count,
            "exactVertexShaderBlobCount": source_emitter_vf_pass_count,
            "exactVertexPixelSignatureClosureCount": source_emitter_vf_pass_count,
            "authoringNoDensityPassCount": source_emitter_vf_pass_count,
            "rawVertexShaderExecutionCount": 0,
            "sourceExactVertexCbCount": 0,
            "productVfPassCount": 0,
            "exactNativeShaderObjectBindingCount": exact_native_binding_count,
            "blockedNativeShaderObjectBindingCount": blocked_native_binding_count,
            "actualVfPassAdmissionCount": 0,
            "runtimeAdmissionCount": 0,
            "visualAdmissionCount": 0,
            "result": overall_result,
        },
    }
    seal(receipt)
    return receipt


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--targets", type=Path, default=DEFAULT_TARGETS)
    parser.add_argument("--source-root", type=Path, default=DEFAULT_SOURCE_ROOT)
    parser.add_argument("--cache", type=Path, default=DEFAULT_CACHE)
    parser.add_argument("--d3dcompiler", type=Path, default=DEFAULT_D3DCOMPILER)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument(
        "--check",
        action="store_true",
        help="Compare deterministic output with the existing receipt.",
    )
    arguments = parser.parse_args(argv)
    receipt = build_receipt(
        arguments.targets.resolve(),
        arguments.source_root.resolve(),
        arguments.cache.resolve(),
        arguments.d3dcompiler.resolve(),
    )
    if arguments.check:
        require(arguments.output.is_file(), f"receipt is missing: {arguments.output}")
        existing = read_json(arguments.output)
        require(existing == receipt, "generated receipt differs from checked-in receipt")
        print(f"PASS: {arguments.output}")
    else:
        write_json_atomic(arguments.output, receipt)
        print(f"WROTE: {arguments.output}")
    print(
        "RESULT: "
        f"exact={receipt['summary']['exactMaterialShaderMapCount']} "
        f"dxbc={receipt['summary']['exactPixelShaderDxbcCount']} "
        f"native={receipt['summary']['exactNativeShaderObjectBindingCount']} "
        f"blocked={receipt['summary']['blockedNoEffectiveStaticSetAbiEvidenceCount']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
