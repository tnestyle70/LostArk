#!/usr/bin/env python3
"""Recover a parent-default UE3 Ribbon material without inventing child DXBC.

The target manifest names one MIC whose cooked export has no native static
resource, its exact parent ``UMaterial``, and one pinned RefShaderCache.  This
tool deliberately keeps the two identities separate:

* the child MIC is reported as ``BLOCKED_NO_CHILD_NATIVE_STATIC_RESOURCE``;
* the parent Material GUID may select only the empty/default static set;
* a renderer-compatible beam/trail VF and requested passes are structural
  evidence, not child-native shader ownership; and
* absent native binding arrays or Texture2D default provenance keep runtime
  material admission fail-closed.

This is a class-neutral evidence extractor.  It writes no Effect document,
runtime catalog, Resource asset, or shader source.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import sys
from pathlib import Path
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
LEVEL_TOOLS = REPOSITORY_ROOT / "Tools" / "LevelPlacementExtractor"
if str(LEVEL_TOOLS) not in sys.path:
    sys.path.insert(0, str(LEVEL_TOOLS))

from extract_artist_31470_material_native_resource import (  # noqa: E402
    decode_export,
)
from extract_artist_31470_material_render_state import (  # noqa: E402
    export_evidence,
)
from extract_ue3_material_shader_maps import (  # noqa: E402
    D3DDisassembler,
    DEFAULT_D3DCOMPILER,
    POLICY_BLOCK_ABSENT,
    decode_mic_target,
    extract_selected_packed_dxbc,
    extract_selected_shader_objects,
    load_package,
    package_tables,
    parse_dxbc_declaration_closure,
    parse_material_map,
    parse_shader_code_layout,
    scan_base_material_contexts,
    scan_native_binding_array_candidates,
)
from extract_ue3_material_texture_sampler_closure import (  # noqa: E402
    texture_export_evidence,
)
from extract_ue3_placements import LOSTARK_KR_AES_KEY  # noqa: E402


SCHEMA = "lostark.effect-ue3-parent-default-ribbon-material-receipt"
FORMAT_VERSION = 1
TARGET_SCHEMA = "lostark.effect-ue3-parent-default-ribbon-material-target"
TARGET_FORMAT_VERSION = 1
DEFAULT_TARGET = REPOSITORY_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31950.ribbon-parent-default-shader-target.json"
)
DEFAULT_OUTPUT = REPOSITORY_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31950.ribbon-parent-default-shader.receipt.json"
)


class RibbonMaterialEvidenceError(ValueError):
    """Raised when a pinned source identity or bounded join changes."""


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RibbonMaterialEvidenceError(message)


def read_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8-sig"))
    require(isinstance(value, dict), f"expected JSON object: {path}")
    return value


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        while payload := stream.read(1024 * 1024):
            digest.update(payload)
    return digest.hexdigest()


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(
        json.dumps(
            value,
            ensure_ascii=False,
            allow_nan=False,
            sort_keys=True,
            separators=(",", ":"),
        ).encode("utf-8")
    ).hexdigest()


def file_identity(path: Path, expected: dict[str, Any]) -> dict[str, Any]:
    require(path.is_file(), f"pinned source file is missing: {path}")
    size = path.stat().st_size
    digest = sha256_file(path)
    require(
        size == expected["physicalPackageByteSize"],
        f"pinned source byte size changed: {path.name}",
    )
    require(
        digest == expected["physicalPackageSha256"],
        f"pinned source SHA changed: {path.name}",
    )
    return {
        "fileName": path.name,
        "byteSize": size,
        "sha256": digest,
    }


def public_without_private(value: dict[str, Any]) -> dict[str, Any]:
    return {key: item for key, item in value.items() if not key.startswith("_")}


def validate_target(target: dict[str, Any]) -> None:
    require(target.get("schema") == TARGET_SCHEMA, "target schema changed")
    require(
        target.get("formatVersion") == TARGET_FORMAT_VERSION,
        "target formatVersion changed",
    )
    require(bool(target.get("targetId")), "targetId is missing")
    require(bool(target.get("occurrenceId")), "occurrenceId is missing")
    require(
        isinstance(target.get("textures"), list)
        and len(target["textures"]) == 3,
        "Ribbon texture denominator changed",
    )
    passes = target.get("rendererEvidence", {}).get("passes")
    require(
        isinstance(passes, list) and len(passes) == 2,
        "Ribbon pass denominator changed",
    )


def build_receipt(
    target_path: Path,
    source_root: Path,
    cache_path: Path,
    d3dcompiler_path: Path,
    runtime_resource_root: Path,
) -> dict[str, Any]:
    target = read_json(target_path)
    validate_target(target)

    child_target = target["childMic"]
    parent_target = target["parentMaterial"]
    cache_target = target["officialRefShaderCache"]

    child_path = source_root / child_target["physicalPackage"]
    parent_path = source_root / parent_target["physicalPackage"]
    child_file = file_identity(child_path, child_target)
    parent_file = file_identity(parent_path, parent_target)

    child_package = load_package(child_path, LOSTARK_KR_AES_KEY)
    child = decode_mic_target(
        child_package,
        {
            "targetId": target["targetId"],
            "micObjectPath": child_target["objectPath"],
            "micClassName": child_target["className"],
            "expectedExportIndexZeroBased": child_target[
                "exportIndexZeroBased"
            ],
            "expectedSerialSha256": child_target["serialSha256"],
            "expectedPropertyStreamEnd": child_target["propertyStreamEnd"],
            "baseMaterialIdHex": parent_target["baseMaterialIdHex"],
            "staticParameterPolicy": POLICY_BLOCK_ABSENT,
        },
    )
    require(
        child["nativeTailByteCount"]
        == child_target["expectedNativeTailByteCount"]
        == 0,
        "child MIC unexpectedly owns a native static resource",
    )
    child["status"] = "BLOCKED_NO_CHILD_NATIVE_STATIC_RESOURCE"
    child["blocker"] = "BLOCKED_NO_CHILD_NATIVE_STATIC_RESOURCE"
    child["childNativeDxbc"] = False

    parent_package = load_package(parent_path, LOSTARK_KR_AES_KEY)
    parent_resource = decode_export(
        parent_package,
        parent_target["exportIndexZeroBased"],
        parent_target["objectPath"],
    )
    for field, expected_field in (
        ("serialSha256", "serialSha256"),
        ("propertyStreamEnd", "propertyStreamEnd"),
        ("nativeTailByteCount", "nativeTailByteCount"),
        ("nativeTailSha256", "nativeTailSha256"),
        ("materialStateGuidHex", "baseMaterialIdHex"),
    ):
        require(
            parent_resource[field] == parent_target[expected_field],
            f"parent Material {field} changed",
        )
    parent_render_state = export_evidence(
        parent_package,
        parent_target["objectPath"],
        parent_target["physicalPackageSha256"],
        parent_target["exportIndexZeroBased"],
        {parent_target["className"]},
        (
            "blendmode",
            "lightingmodel",
            "busedwithbeamtrails",
            "busesdistortion",
            "twosided",
        ),
    )
    fields = parent_render_state["fields"]
    require(
        fields["blendmode"].get("value") == "blend_translucent"
        and fields["lightingmodel"].get("value") == "mlm_unlit"
        and fields["busedwithbeamtrails"].get("value") is True
        and fields["busesdistortion"].get("value") is True,
        "parent Ribbon render-state contract changed",
    )

    require(cache_path.is_file(), f"pinned RefShaderCache is missing: {cache_path}")
    require(
        cache_path.stat().st_size == cache_target["physicalPackageByteSize"]
        and sha256_file(cache_path) == cache_target["physicalPackageSha256"],
        "pinned RefShaderCache identity changed",
    )
    cache = package_tables(cache_path)
    layout = parse_shader_code_layout(cache)
    base_material_id = parent_target["baseMaterialIdHex"]
    scan = scan_base_material_contexts(cache, layout, [base_material_id])[
        base_material_id
    ]
    require(
        scan["rawHitCount"] == cache_target["expectedRawBaseMaterialHitCount"]
        and scan["parseableStaticSetCount"]
        == cache_target["expectedParseableStaticSetCount"]
        and scan["materialMapContextCount"]
        == cache_target["expectedMaterialMapContextCount"]
        == 1,
        "parent-default MaterialShaderMap denominator changed",
    )
    context = scan["materialMapContexts"][0]
    expected_map = cache_target["expectedMaterialMap"]
    require(
        context["logicalOffset"] == expected_map["logicalOffset"]
        and context["logicalEndOffset"] == expected_map["logicalEndOffset"]
        and context["staticParameterSetRawSha256"]
        == expected_map["staticParameterSetRawSha256"]
        and context["engineEqualityStaticParameterSetSha256"]
        == expected_map["engineEqualityStaticParameterSetSha256"]
        and context["vertexFactoryCount"] == expected_map["vertexFactoryCount"],
        "parent-default MaterialShaderMap context changed",
    )
    material_map = parse_material_map(
        cache,
        layout,
        context,
        context["engineEqualityStaticParameterSetSha256"],
    )
    require(
        material_map["rawSha256"] == expected_map["rawSha256"]
        and material_map["friendlyName"] == expected_map["friendlyName"],
        "parent-default MaterialShaderMap payload changed",
    )
    static_set = material_map["staticParameterSet"]
    require(
        not static_set["staticSwitchParameters"]
        and not static_set["staticComponentMaskParameters"]
        and not static_set["normalParameters"]
        and not static_set["terrainLayerWeightParameters"],
        "selected parent Material map is not the empty/default static set",
    )

    renderer = target["rendererEvidence"]
    vf_matches = [
        row
        for row in material_map["vertexFactories"]
        if row["vertexFactoryType"] == renderer["vertexFactoryType"]
    ]
    require(len(vf_matches) == 1, "dynamic beam/trail VF join changed")
    selected_vf = vf_matches[0]
    disassembler = D3DDisassembler(d3dcompiler_path)
    pass_evidence: list[dict[str, Any]] = []
    runtime_blockers = {
        "CHILD_MIC_HAS_NO_NATIVE_STATIC_RESOURCE",
        "PARENT_DEFAULT_SHADER_IS_NOT_CHILD_NATIVE_DXBC",
    }
    for requested_pass in renderer["passes"]:
        references = [
            row
            for row in selected_vf["shaderReferences"]
            if row["shaderType"] == requested_pass["shaderType"]
            and row["shaderIdHex"] == requested_pass["shaderIdHex"]
        ]
        require(len(references) == 1, f"pass reference changed: {requested_pass['role']}")
        reference = references[0]
        dxbc_by_id = extract_selected_packed_dxbc(cache, layout, [reference])
        dxbc = dxbc_by_id[reference["shaderIdHex"]]
        require(
            dxbc["dxbc"]["byteSize"] == requested_pass["dxbcByteSize"]
            and dxbc["dxbc"]["sha256"] == requested_pass["dxbcSha256"],
            f"pass DXBC changed: {requested_pass['role']}",
        )
        disassembly = disassembler.disassemble(dxbc["_bytecode"])
        declaration_closure = parse_dxbc_declaration_closure(disassembly)
        shader_objects = extract_selected_shader_objects(cache, layout, [reference])
        shader_object = shader_objects["byShaderId"][reference["shaderIdHex"]]
        candidates = scan_native_binding_array_candidates(
            shader_object["_bytes"],
            shader_object["logicalOffset"],
            material_map["uniformExpressionCounts"],
            declaration_closure,
        )
        require(
            len(candidates)
            == requested_pass["expectedNativeBindingCandidateCount"],
            f"native binding candidate denominator changed: {requested_pass['role']}",
        )
        binding_status = (
            "EXACT_UNIQUE_NATIVE_BINDING_ARRAY"
            if len(candidates) == 1
            else "BLOCKED_NATIVE_BINDING_ARRAY_ABSENT"
        )
        if len(candidates) != 1:
            runtime_blockers.add(
                f"{requested_pass['role']}_NATIVE_BINDING_ARRAY_ABSENT"
            )
        pass_evidence.append(
            {
                "role": requested_pass["role"],
                "reference": reference,
                "dxbc": public_without_private(dxbc),
                "disassembly": disassembly,
                "shaderObject": public_without_private(shader_object),
                "nativeBindingCandidateCount": len(candidates),
                "nativeBindingStatus": binding_status,
                "nativeBindingCandidates": candidates,
                "runtimeAdmission": False,
            }
        )

    package_cache: dict[str, Any] = {}
    texture_evidence = []
    referenced_paths = [
        row["objectPath"] for row in parent_resource["referencedTextures"]
    ]
    manifest_texture_paths = [row["sourceObjectPath"] for row in target["textures"]]
    require(
        referenced_paths == manifest_texture_paths,
        "parent ReferencedTextures order changed",
    )
    for texture_target in target["textures"]:
        texture = texture_export_evidence(
            texture_target["sourceObjectPath"],
            {
                "sourceAssetPath": texture_target["sourceObjectPath"],
                "logicalPackage": texture_target["logicalPackage"],
                "physicalPackage": texture_target["physicalPackage"],
                "resolutionStatus": "PINNED_LOGICAL_PACKAGE_MAPPING",
            },
            source_root,
            package_cache,
        )
        require(
            texture["physicalPackageSha256"]
            == texture_target["physicalPackageSha256"],
            f"Texture2D package SHA changed: {texture_target['sourceObjectPath']}",
        )
        runtime_path = runtime_resource_root / texture_target["runtimeAssetId"]
        if runtime_path.is_file():
            runtime_sha = sha256_file(runtime_path)
            require(
                runtime_sha == texture_target["runtimeDdsSha256"],
                f"runtime DDS identity changed: {texture_target['runtimeAssetId']}",
            )
            deployment = {
                "status": "PRESENT_EXACT",
                "byteSize": runtime_path.stat().st_size,
                "sha256": runtime_sha,
            }
        else:
            deployment = {
                "status": "MISSING_FROM_THIS_WORKTREE_RUNTIME_RESOURCE_ROOT",
                "expectedSha256": texture_target["runtimeDdsSha256"],
            }
            runtime_blockers.add(
                "RUNTIME_DDS_MISSING_" +
                texture_target["sourceObjectPath"].rsplit(".", 1)[-1].upper()
            )
        if not texture["samplerAndColorSpace"][
            "sourceExactSamplerAndColorSpace"
        ]:
            runtime_blockers.update(
                texture["samplerAndColorSpace"]["blockers"]
            )
        texture_evidence.append(
            {
                **texture,
                "runtimeAssetId": texture_target["runtimeAssetId"],
                "expectedRuntimeDdsSha256": texture_target[
                    "runtimeDdsSha256"
                ],
                "runtimeDeployment": deployment,
            }
        )

    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "targetId": target["targetId"],
        "occurrenceId": target["occurrenceId"],
        "scope": {
            "classNeutralExtractor": True,
            "selection": "PARENT_UMATERIAL_GUID_TO_EMPTY_DEFAULT_STATIC_SET",
            "childNativeDxbc": False,
            "parentDefaultShaderEvidence": True,
            "runtimeMaterialAdmission": False,
            "carrierAdmission": "SEPARATE_TYPED_CASCADE_RIBBON_PACKET",
        },
        "inputs": {
            "targetManifest": {
                "path": target_path.resolve().relative_to(
                    REPOSITORY_ROOT
                ).as_posix(),
                "sha256": sha256_file(target_path),
            },
            "childPackage": child_file,
            "parentPackage": parent_file,
            "officialRefShaderCache": {
                "fileName": cache_path.name,
                "byteSize": cache_path.stat().st_size,
                "sha256": cache_target["physicalPackageSha256"],
                "shaderCodeLayout": layout,
            },
            "d3dcompiler": disassembler.identity,
        },
        "childMic": child,
        "parentMaterial": {
            "resource": parent_resource,
            "renderState": parent_render_state,
        },
        "parentDefaultMaterialMapScan": scan,
        "parentDefaultMaterialMap": material_map,
        "selectedRenderer": {
            "vertexFactoryType": selected_vf["vertexFactoryType"],
            "vertexFactoryIndex": selected_vf["vertexFactoryIndex"],
            "allShaderReferences": selected_vf["shaderReferences"],
            "passes": pass_evidence,
        },
        "textureLanes": texture_evidence,
        "runtimeAdmission": {
            "admitted": False,
            "materialBackend": "FAIL_CLOSED_NO_COLOR_GUESS",
            "blockers": sorted(runtime_blockers),
        },
        "result": "BLOCKED_PARENT_DEFAULT_SHADER_BINDING_AND_SAMPLER_ABI_INCOMPLETE",
    }
    receipt["receiptSha256"] = canonical_sha256(receipt)
    return receipt


def output_bytes(value: dict[str, Any]) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, allow_nan=False, indent=2) + "\n"
    ).encode("utf-8")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--target", type=Path, default=DEFAULT_TARGET)
    parser.add_argument("--source-root", type=Path)
    parser.add_argument("--cache", type=Path)
    parser.add_argument("--d3dcompiler", type=Path, default=DEFAULT_D3DCOMPILER)
    parser.add_argument(
        "--runtime-resource-root",
        type=Path,
        default=REPOSITORY_ROOT / "Client/Bin/Resources",
    )
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    arguments = parser.parse_args(argv)
    target_path = arguments.target.resolve()
    target = read_json(target_path)
    source_root = (
        arguments.source_root.resolve()
        if arguments.source_root is not None
        else Path(target["sourceRootHint"]).resolve()
    )
    cache_path = (
        arguments.cache.resolve()
        if arguments.cache is not None
        else Path(target["officialRefShaderCache"]["pathHint"]).resolve()
    )
    receipt = build_receipt(
        target_path,
        source_root,
        cache_path,
        arguments.d3dcompiler.resolve(),
        arguments.runtime_resource_root.resolve(),
    )
    expected = output_bytes(receipt)
    output = arguments.output.resolve()
    if arguments.check:
        require(output.is_file(), f"receipt is missing: {output}")
        require(output.read_bytes() == expected, f"receipt is stale: {output}")
        print(f"PASS: {output}")
    else:
        output.parent.mkdir(parents=True, exist_ok=True)
        temporary = output.with_suffix(output.suffix + ".tmp")
        temporary.write_bytes(expected)
        temporary.replace(output)
        print(f"WROTE: {output}")
    print(
        "RESULT: childNativeDxbc=false parentDefaultMap=exact "
        "runtimeMaterialAdmission=false"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
