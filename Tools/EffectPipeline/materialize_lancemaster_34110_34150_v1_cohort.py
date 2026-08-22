#!/usr/bin/env python3
"""Seal Lance D as a control and promote one Lance F MakeFlow occurrence."""

from __future__ import annotations

import argparse
import codecs
import copy
import hashlib
import json
import os
from pathlib import Path
import tempfile
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
D_EFFECT_ID = "effect.lancemaster.skill.34110.unified"
F_EFFECT_ID = "effect.lancemaster.skill.34150.unified"
D_ELEMENT_ID = "authored.source-particle.c7469f2311b49e44ed801be8"
F_ELEMENT_ID = "authored.source-particle.dfc359983bf57e958f75740d"

D_DOCUMENT = Path("Data/Effects/Authored") / f"{D_EFFECT_ID}.effect.json"
F_DOCUMENT = Path("Data/Effects/Authored") / f"{F_EFFECT_ID}.effect.json"
FAMILY_MANIFEST = Path("Data/Effects/Contracts/effect-family-manifest.v1.json")
SOURCE_RECEIPT = Path(
    "Data/Effects/Imported/LanceMaster/CurrentCombat/skill.34150.source-receipt.json"
)
EFFECT_CATALOG = Path("Data/Effects/EffectCatalog.json")
SKILL_BINDINGS = Path(
    "Data/Animation/Authored/LanceMaster/LanceMaster.skillbindings.json"
)
ANIMATION_EVENTS = Path(
    "Data/Animation/Authored/LanceMaster/LanceMaster.animevents"
)
OUTPUT_RECEIPT = Path(
    "Data/Effects/AuthoredCorrections/Generated/LanceMaster/"
    "effect.lancemaster.skill.34110-34150.v1-cohort.receipt.json"
)
RUNTIME_SHADER = Path("Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli")
MATERIAL_TEMPLATE = Path("Client/Public/Effect_MaterialTemplate.h")
DOCUMENT_RENDERER = Path("Client/Private/Effect_DocumentRenderer.cpp")

D_DOCUMENT_SHA256 = "6bf6930079de827d275d3ecbac2e96edfe9147e73c0187300d988a8207291347"
D_ROW_SHA256 = "e3602ed98b60c95da84361c0019eef45a74c178a8b72dc3135fc67f2fb1efba9"
F_ENVELOPE_SHA256 = "785a205c8d60ebfcfb4f9ca8a6d1a5bf32a7e61e4f1dda8a35b6c6ed86c3dce5"
F_NON_TARGET_ROWS_SHA256 = "5640e946777892028dc07c332231a01e3545779cab2a57ca4ca8d02086087b7e"
F_NON_MATERIAL_SHA256 = "b01a14d7653037f734d0fae97168b198712813ad014c2c631205fd9229bdd324"
F_MATERIAL_HEAD_SHA256 = "8594d8e71612a16925667a97da5a36f8ae8158b5770678881057d0bdfaee7b87"
F_LEGACY_SOURCE_PROFILE_SHA256 = (
    "035fa70b50ffac3c1ffec355303e77fb701247f1da9e449b3674b2f8f7d7f35d"
)
F_DYNAMIC_MODULE_SHA256 = "705e63d29969119a8ac518edf2c3cd94d7b8cad4fa17155772db51e320b984a9"
FAMILY_ID = "family-5059859991f80b18"
FAMILY_ROW_SHA256 = "7cbeb2a7463fc9f2633ac39ac6406171e7f91ff980d5c247fe908922a2283073"
SOURCE_BINDING_SHA256 = "8d9a44a0e9d025d916ff7c08c2e77a0b1665ed47f146704c6102455731a10e94"

CANONICAL_PROFILE = (
    "ue3.material.fx.m.mi.02.fx.m.fx.k.me.makeflow.02.tr.5059859991f8"
)
CANONICAL_PARENT = "fx_m_mi_02.fx_m.fx_k_me_makeflow_02_tr"
F_CHILD = "fx_m_mi_02.fx_mi.fx_k_me_makeflow_02_13_tr"
REQUIRED_TEXTURE_NAMES = (
    "opacity_tex",
    "diff_tex1",
    "diff_tex2",
    "color_tex",
    "flowtex",
)
DYNAMIC_FALLBACKS = (1.0, 0.0, 0.0, 0.0)

RESOURCE_SHA256 = {
    "Effect/LanceMaster/Meshes/fm_m_ring_001.wmodel":
        "8a53b3615e507ec1062ae6cd2f8c056811e6127fd9d71e5aba7311e70a6ecca0",
    "Effect/LanceMaster/Textures/fx_m_trail_007.dds":
        "1a8035451dc5d8e0aea10b97086affb3cca8a4729f862d6b2f528c33079f524b",
    "Effect/LanceMaster/Textures/fx_h_atypical_01_1.dds":
        "762858e2707dea6783690193df7c72bf59b65fb924de429272632750d7557dec",
    "Effect/LanceMaster/Textures/fx_m_noise_003.dds":
        "424d8477961361dc34945fe73ea4fe402e6c0c50efa59d427a3737034a7f50f5",
    "Effect/LanceMaster/Textures/fx_m_noise_001.dds":
        "19843f9ee15e94e629926f45e1887ad6ca9815bfd785527ed8b4ce63692918b8",
    "Effect/LanceMaster/Textures/fx_d_noise_030.dds":
        "9a876ceb5d173869af5350d348462f54fc8a9b00134957bd3cd56805860d0581",
    "Effect/LanceMaster/Meshes/fm_o_swing_02.wmodel":
        "c8d2fa4c28d5aa98fae5092d8c79d5314b7ecd5ccfa48a926b137c8f118bc126",
    "Effect/LanceMaster/Textures/fx_i_atypical_03_2_xcl.dds":
        "2840c919fada6df580d88d1c383a730c41ef8f82df8a745f3ff0cbfb5e135cee",
    "Effect/LanceMaster/Textures/fx_d_noise_006.dds":
        "0beffe804436417387719c14b5eee538537f0306915bfcd644e18861ae619be4",
    "Effect/LanceMaster/Textures/fx_m_spatter_001_xyclamp.dds":
        "b07b82732e5bc5ab99558b771a98934ffc0d787295cde66324eff52fddbae3e2",
    "Effect/LanceMaster/Textures/fx_l_environment_001.dds":
        "cd9285960a7fde29076a46a64911c23786d28eca4f64a6f9c7e6d468455b48d1",
}


class LanceV1CohortError(RuntimeError):
    """Raised when a sealed D/F cohort input drifts."""


def canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise LanceV1CohortError(message)


def load_json(root: Path, relative: Path) -> dict[str, Any]:
    path = root / relative
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, json.JSONDecodeError) as error:
        raise LanceV1CohortError(f"cannot read {relative}: {error}") from error
    require(isinstance(value, dict), f"JSON root is not an object: {relative}")
    return value


def unique_element(document: dict[str, Any], element_id: str) -> dict[str, Any]:
    rows = [row for row in document.get("elements", [])
            if isinstance(row, dict) and row.get("id") == element_id]
    require(len(rows) == 1, f"element is not singular: {element_id}")
    return rows[0]


def validate_product_joins(root: Path) -> None:
    catalog = load_json(root, EFFECT_CATALOG)
    indexed = {row.get("effectAssetId"): row
               for row in catalog.get("effects", []) if isinstance(row, dict)}
    for effect_id in (D_EFFECT_ID, F_EFFECT_ID):
        row = indexed.get(effect_id)
        require(row == {
            "effectAssetId": effect_id,
            "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
            "authoringPath": f"Effects/Authored/{effect_id}.effect.json",
        }, f"Product EffectCatalog join changed: {effect_id}")

    bindings = load_json(root, SKILL_BINDINGS)
    indexed_bindings = {row.get("skillId"): row
                        for row in bindings.get("bindings", [])
                        if isinstance(row, dict)}
    require(indexed_bindings.get(34110) == {
        "skillId": 34110,
        "clips": [{"clip": "flm_sk_crescentsweep", "playRate": 1.2}],
    }, "Lance D skill binding changed")
    require(indexed_bindings.get(34150) == {
        "skillId": 34150,
        "clips": [{"clip": "flm_sk_crushingblow", "playRate": 1.2}],
    }, "Lance F skill binding changed")

    event_text = (root / ANIMATION_EVENTS).read_text(encoding="utf-8-sig")
    expected = (
        '"flm_sk_crescentsweep" EFFECT startms=0 '
        'payload="effect.lancemaster.skill.34110.unified" effectref=asset ',
        '"flm_sk_crushingblow" EFFECT startms=0 '
        'payload="effect.lancemaster.skill.34150.unified" effectref=asset ',
    )
    for prefix in expected:
        require(event_text.count(prefix) == 1,
                f"Product animevent join changed: {prefix}")


def validate_resources(root: Path) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for asset_id, expected_hash in RESOURCE_SHA256.items():
        path = root / "Client/Bin/Resources" / asset_id
        require(path.is_file(), f"runtime resource is missing: {asset_id}")
        payload = path.read_bytes()
        actual_hash = hashlib.sha256(payload).hexdigest()
        require(actual_hash == expected_hash,
                f"runtime resource identity changed: {asset_id}")
        result[asset_id] = {"bytes": len(payload), "sha256": actual_hash}
    return result


def validate_profile36_runtime(root: Path) -> dict[str, str]:
    shader = (root / RUNTIME_SHADER).read_text(encoding="utf-8-sig")
    shader_start = shader.find(
        "else if (16 == g_SourceMaterialProfile || 36 == g_SourceMaterialProfile)"
    )
    shader_end = shader.find(
        "else if (17 == g_SourceMaterialProfile)", shader_start)
    require(shader_start >= 0 and shader_end > shader_start,
            "MakeFlow profile16/36 HLSL branch is missing")
    shader_block = shader[shader_start:shader_end]
    for semantic_index, fallback in ((27, "1.f"), (28, "0.f"), (29, "0.f")):
        require(
            f"dynamicParameter, {semantic_index}u, {fallback}" in shader_block,
            f"MakeFlow profile36 dynamic fallback changed: {semantic_index}",
        )
    require("30u" not in shader_block,
            "MakeFlow profile36 unexpectedly consumes channel 3")
    require("clip(g_SourceTextureMask == 0x1fu" in shader_block and
            "Sample_SourceTexture4(flowUV)" in shader_block,
            "MakeFlow profile36 five-lane shader contract changed")

    material_template = (root / MATERIAL_TEMPLATE).read_text(
        encoding="utf-8-sig")
    profile_start = material_template.find(f'"{CANONICAL_PROFILE}"')
    profile_end = material_template.find("return EFFECT_STRICT_TYPED_SOURCE_PROFILE::MAKEFLOW_02;",
                                         profile_start)
    require(profile_start >= 0 and profile_end > profile_start,
            "MakeFlow02 strict profile admission is missing")
    profile_block = material_template[profile_start:profile_end + 61]
    require(f'"{CANONICAL_PARENT}"' in profile_block,
            "MakeFlow02 strict parent admission changed")

    renderer = (root / DOCUMENT_RENDERER).read_text(encoding="utf-8-sig")
    renderer_start = renderer.find("else if (36u == Staged.iSourceMaterialProfile)")
    renderer_end = renderer.find("else if (37u == Staged.iSourceMaterialProfile)",
                                 renderer_start)
    require(renderer_start >= 0 and renderer_end > renderer_start,
            "MakeFlow profile36 renderer branch is missing")
    renderer_block = renderer[renderer_start:renderer_end]
    require("Build_MakeFlowConstants(" in renderer_block,
            "MakeFlow profile36 constant builder changed")
    return {
        "shaderBranchSha256": hashlib.sha256(
            shader_block.encode("utf-8")).hexdigest(),
        "strictAdmissionSha256": hashlib.sha256(
            profile_block.encode("utf-8")).hexdigest(),
        "rendererBranchSha256": hashlib.sha256(
            renderer_block.encode("utf-8")).hexdigest(),
    }


def validate_d_control(root: Path) -> tuple[dict[str, Any], dict[str, Any]]:
    document = load_json(root, D_DOCUMENT)
    require(document.get("effectAssetId") == D_EFFECT_ID and
            document.get("version") == 13 and
            len(document.get("elements", [])) == 88,
            "Lance D Product document identity changed")
    require(canonical_sha256(document) == D_DOCUMENT_SHA256,
            "Lance D Product document changed outside the KEEP control")
    element = unique_element(document, D_ELEMENT_ID)
    require(canonical_sha256(element) == D_ROW_SHA256,
            "Lance D KEEP control row changed")
    require(element.get("kind") == "particle" and
            element.get("sourceRecipe", {}).get("rendererShape") == "mesh",
            "Lance D KEEP carrier is not MeshParticle")
    require(element.get("material", {}).get("sourceProfile", {}).get(
        "runtimeShaderProfileId") == "effect.ue3.missiletrail-two-emissive.v1",
        "Lance D KEEP runtime material changed")
    return document, element


def source_material_binding(root: Path) -> dict[str, Any]:
    receipt = load_json(root, SOURCE_RECEIPT)
    matches = [row for row in receipt.get("materialParameterBindings", [])
               if isinstance(row, dict) and row.get("sourceMaterialPath") == F_CHILD]
    require(len(matches) == 1, "Lance F source MIC binding is not singular")
    row = matches[0]
    require(canonical_sha256(row) == SOURCE_BINDING_SHA256,
            "Lance F exact source MIC binding changed")
    require(row.get("resolutionStatus") == "RESOLVED_EXACT_SOURCE_PACKAGE" and
            row.get("sourcePhysicalPackage") ==
            "YGI3SB3OBJ3O1FGUMP6QMP8Y5.upk",
            "Lance F exact source package identity changed")
    return row


def makeflow_family(root: Path) -> dict[str, Any]:
    manifest = load_json(root, FAMILY_MANIFEST)
    matches = [row for row in manifest.get("families", [])
               if isinstance(row, dict) and row.get("familyId") == FAMILY_ID]
    require(len(matches) == 1, "MakeFlow02 family row is not singular")
    row = matches[0]
    require(row.get("rowSha256") == FAMILY_ROW_SHA256 and
            canonical_sha256({key: value for key, value in row.items()
                              if key != "rowSha256"}) == FAMILY_ROW_SHA256,
            "MakeFlow02 family evidence changed")
    require(row.get("parentMaterialPath") == CANONICAL_PARENT,
            "MakeFlow02 effective parent changed")
    return row


def texture_descriptor(
    name: str,
    source_object_path: str,
    asset_id: str,
    group: str,
) -> dict[str, Any]:
    return {
        "name": name,
        "sourceObjectPath": source_object_path,
        "assetId": asset_id,
        "addressU": "wrap",
        "addressV": "wrap",
        "colorSpace": "linear",
        "samplingEvidence": "legacy_default",
        "group": group,
    }


def build_effective_source_profile(
    source_binding: dict[str, Any], family: dict[str, Any]
) -> dict[str, Any]:
    child_textures = {row["name"]: row
                      for row in source_binding.get("textures", [])}
    require(set(child_textures) == {
        "diff_tex1", "diff_tex2", "flowtex", "opacity_tex"
    }, "Lance F child MIC texture override set changed")
    parent_textures = {row["name"]: row
                       for row in family.get("evidence", {}).get(
                           "textureParameters", [])}
    require(set(parent_textures) == {
        "opacity_tex", "diff_tex1", "diff_tex2", "color_tex"
    }, "MakeFlow02 parent texture declaration set changed")

    textures: list[dict[str, Any]] = []
    for name in REQUIRED_TEXTURE_NAMES:
        if name == "color_tex":
            parent = parent_textures[name]
            require(parent.get("texture") == "fx_l_environment_001" and
                    parent.get("group") == "color",
                    "MakeFlow02 inherited color texture changed")
            textures.append(texture_descriptor(
                name, "fx_l_environment_001",
                "Effect/LanceMaster/Textures/fx_l_environment_001.dds",
                "color"))
            continue
        child = child_textures[name]
        group = "opacity" if name == "opacity_tex" else (
            "diff" if name.startswith("diff_tex") else "")
        textures.append(texture_descriptor(
            name, child["texture"],
            "Effect/LanceMaster/Textures/" +
            child["dds_path"].replace("\\", "/").split("/")[-1], group))

    child_scalars = {row["name"]: row["value"]
                     for row in source_binding.get("scalars", [])}
    parent_scalars = family.get("evidence", {}).get("scalarParameters", [])
    parent_names = [row.get("name") for row in parent_scalars]
    require(len(parent_names) == 28 and len(set(parent_names)) == 28,
            "MakeFlow02 parent scalar declaration set changed")
    require(set(child_scalars).issubset(set(parent_names)),
            "Lance F child scalar is not declared by the effective parent")
    scalars = [{
        "name": row["name"],
        "value": child_scalars.get(row["name"], row["value"]),
        "group": row["group"],
    } for row in parent_scalars]

    return {
        "enabled": True,
        "profileId": CANONICAL_PROFILE,
        "runtimeShaderProfileId": "effect.ue3.grouped-translucent.v1",
        "parentMaterialPath": CANONICAL_PARENT,
        "semanticStatus": "reconstructed_profile",
        "textures": textures,
        "scalars": scalars,
        "vectors": [],
        "staticSwitches": [],
        "dynamicParameterSemantics": ["unbound"] * 4,
        "subUVMode": "none",
    }


def build_legacy_source_profile(source_binding: dict[str, Any]) -> dict[str, Any]:
    """Rebuild the sealed pre-cohort descriptor for focused fixture tests."""
    textures = []
    for row in source_binding["textures"]:
        textures.append(texture_descriptor(
            row["name"], row["texture"],
            "Effect/LanceMaster/Textures/" +
            row["dds_path"].replace("\\", "/").split("/")[-1], ""))
    scalars = [{"name": row["name"], "value": row["value"], "group": ""}
               for row in source_binding["scalars"]]
    result = {
        "enabled": True,
        "profileId": "ue3.material.fx.m.fx.k.me.makeflow.02.tr.4e9c934f58ab",
        "runtimeShaderProfileId": "effect.ue3.grouped-translucent.v1",
        "parentMaterialPath": "fx_m.fx_k_me_makeflow_02_tr",
        "semanticStatus": "reconstructed_profile",
        "textures": textures,
        "scalars": scalars,
        "vectors": [],
        "staticSwitches": [],
        "dynamicParameterSemantics": ["unbound"] * 4,
        "subUVMode": "none",
    }
    require(canonical_sha256(result) == F_LEGACY_SOURCE_PROFILE_SHA256,
            "sealed Lance F legacy source profile reconstruction changed")
    return result


def validate_f_document(
    document: dict[str, Any], expected_profile: dict[str, Any]
) -> tuple[dict[str, Any], bool]:
    require(document.get("effectAssetId") == F_EFFECT_ID and
            document.get("version") == 13 and
            len(document.get("elements", [])) == 186,
            "Lance F Product document identity changed")
    envelope = {key: value for key, value in document.items()
                if key != "elements"}
    require(canonical_sha256(envelope) == F_ENVELOPE_SHA256,
            "Lance F Product document envelope changed")
    non_targets = [row for row in document["elements"]
                   if row.get("id") != F_ELEMENT_ID]
    require(len(non_targets) == 185 and
            canonical_sha256(non_targets) == F_NON_TARGET_ROWS_SHA256,
            "Lance F non-target rows changed")
    element = unique_element(document, F_ELEMENT_ID)
    non_material = {key: value for key, value in element.items()
                    if key != "material"}
    require(canonical_sha256(non_material) == F_NON_MATERIAL_SHA256,
            "Lance F carrier/transform/timing/source recipe changed")
    material = element.get("material", {})
    require(canonical_sha256({key: value for key, value in material.items()
                              if key != "sourceProfile"}) ==
            F_MATERIAL_HEAD_SHA256,
            "Lance F child material or render state changed")
    require(element.get("kind") == "particle" and
            element.get("sourceRecipe", {}).get("rendererShape") == "mesh" and
            element.get("detail", {}).get("timing", {}).get(
                "startDelaySeconds") == 1.4053,
            "Lance F MeshParticle carrier signature changed")

    dynamic_modules = [row for row in element.get("sourceRecipe", {}).get(
        "modules", []) if row.get("className") ==
        "particlemoduleparameterdynamic"]
    require(len(dynamic_modules) == 1 and
            canonical_sha256(dynamic_modules[0]) == F_DYNAMIC_MODULE_SHA256,
            "Lance F source DynamicParameter module changed")
    dynamic_names = {row.get("propertyPath"): row.get("value")
                     for row in dynamic_modules[0].get("literals", [])
                     if row.get("kind") == "string"}
    require([dynamic_names.get(f"dynamicparams[{index}].paramname")
             for index in range(4)] == ["none"] * 4,
            "Lance F anonymous dynamic lanes changed")

    current = material.get("sourceProfile")
    if current == expected_profile:
        return element, True
    require(canonical_sha256(current) == F_LEGACY_SOURCE_PROFILE_SHA256,
            "Lance F source profile is neither sealed legacy nor cohort output")
    return element, False


def extract_element_blocks(
    document_text: str,
) -> tuple[str, str, dict[str, dict[str, Any]], dict[str, str]]:
    marker = document_text.find('"elements"')
    array_start = document_text.find("[", marker)
    require(marker >= 0 and array_start >= 0, "elements array is missing")
    decoder = json.JSONDecoder()
    position = array_start + 1
    parsed: dict[str, dict[str, Any]] = {}
    blocks: dict[str, str] = {}
    first_start: int | None = None
    last_end: int | None = None
    while True:
        while position < len(document_text) and (
            document_text[position].isspace() or document_text[position] == ","
        ):
            position += 1
        require(position < len(document_text), "elements array is unterminated")
        if document_text[position] == "]":
            break
        value, end = decoder.raw_decode(document_text, position)
        require(isinstance(value, dict) and isinstance(value.get("id"), str),
                "element block has no stable ID")
        element_id = value["id"]
        require(element_id not in blocks, f"duplicate element ID: {element_id}")
        first_start = position if first_start is None else first_start
        last_end = end
        parsed[element_id] = value
        blocks[element_id] = document_text[position:end]
        position = end
    require(first_start is not None and last_end is not None,
            "elements array is empty")
    return document_text[:first_start], document_text[last_end:], parsed, blocks


def serialize_preserving_non_targets(
    source_text: str,
    target: dict[str, Any],
    target_ids: set[str],
) -> str:
    source = json.loads(source_text)
    source_outer = copy.deepcopy(source)
    target_outer = copy.deepcopy(target)
    source_outer.pop("elements", None)
    target_outer.pop("elements", None)
    require(source_outer == target_outer, "materializer cannot rewrite envelope")
    canonical = json.dumps(target, ensure_ascii=False, indent=2) + "\n"
    head, tail, source_by_id, source_blocks = extract_element_blocks(source_text)
    _, _, target_by_id, target_blocks = extract_element_blocks(canonical)
    output_blocks: list[str] = []
    for element in target["elements"]:
        element_id = element["id"]
        if element_id in target_ids:
            output_blocks.append(target_blocks[element_id])
        else:
            require(source_by_id.get(element_id) == element,
                    f"unowned element changed: {element_id}")
            output_blocks.append(source_blocks[element_id])
    output = head + ",\n    ".join(output_blocks) + tail
    require(json.loads(output) == target,
            "raw-preserving projection changed semantics")
    return output


def atomic_write(path: Path, text: str, *, bom: bool = False) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary: Path | None = None
    try:
        descriptor, name = tempfile.mkstemp(
            prefix=path.name + ".", suffix=".tmp", dir=path.parent)
        temporary = Path(name)
        with os.fdopen(descriptor, "wb") as stream:
            if bom:
                stream.write(codecs.BOM_UTF8)
            stream.write(text.encode("utf-8"))
            stream.flush()
            os.fsync(stream.fileno())
        json.loads(temporary.read_text(encoding="utf-8-sig"))
        os.replace(temporary, path)
        temporary = None
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def build_receipt(
    d_document: dict[str, Any],
    d_element: dict[str, Any],
    f_document: dict[str, Any],
    f_element: dict[str, Any],
    expected_profile: dict[str, Any],
    resources: dict[str, dict[str, Any]],
    runtime_evidence: dict[str, str],
) -> dict[str, Any]:
    receipt: dict[str, Any] = {
        "schema": "lostark.effect-lancemaster-d-f-v1-cohort-receipt",
        "formatVersion": 1,
        "scope": {
            "characterClass": "LANCE_MASTER",
            "skillIds": [34110, 34150],
            "effectAssetIds": [D_EFFECT_ID, F_EFFECT_ID],
        },
        "candidateDispositions": [
            {
                "effectAssetId": D_EFFECT_ID,
                "occurrenceId": D_ELEMENT_ID,
                "disposition": "KEEP",
                "reason": "NO_CODE_MESHPARTICLE_MISSILETRAIL_CONTROL",
            },
            {
                "effectAssetId": F_EFFECT_ID,
                "occurrenceId": F_ELEMENT_ID,
                "disposition": "KEEP",
                "reason": "PRESERVE_MESHPARTICLE_CARRIER_ROLE_ROW",
            },
        ],
        "candidateDenominator": {
            "keep": 2,
            "replace": 0,
            "add": 0,
            "retire": 0,
            "bulkRestore": False,
        },
        "materialActions": [
            {
                "effectAssetId": F_EFFECT_ID,
                "occurrenceId": F_ELEMENT_ID,
                "action": "PROMOTE_EFFECTIVE_PARENT_PROFILE36",
                "ownedField": "material.sourceProfile",
                "reason": "MAKEFLOW02_FIVE_LANE_EFFECTIVE_PARENT_CONTRACT",
            },
        ],
        "dControl": {
            "carrier": "MeshParticle",
            "runtimeProfile": "effect.ue3.missiletrail-two-emissive.v1",
            "effectiveSourceMaterialProfileIndex": 15,
            "rowSha256": canonical_sha256(d_element),
            "documentSha256": canonical_sha256(d_document),
            "productJoin": "CURRENT",
        },
        "fCanary": {
            "carrier": "MeshParticle",
            "sourceMaterialPath": F_CHILD,
            "familyId": FAMILY_ID,
            "profileId": CANONICAL_PROFILE,
            "parentMaterialPath": CANONICAL_PARENT,
            "effectiveSourceMaterialProfileIndex": 36,
            "textureLaneOrder": list(REQUIRED_TEXTURE_NAMES),
            "textureLaneAssets": {
                row["name"]: row["assetId"]
                for row in expected_profile["textures"]
            },
            "scalarCount": len(expected_profile["scalars"]),
            "dynamicParameter": {
                "sourceNames": ["none"] * 4,
                "runtimeSemantics": ["unbound"] * 4,
                "profile36Defaults": list(DYNAMIC_FALLBACKS),
                "channel3Policy": "SUPPRESSED_UNUSED_DEFAULT_ZERO",
                "claim": "SOURCE_ORDINAL_UNRESOLVED_NO_GUESSED_PROMOTION",
            },
            "fidelity": "PROJECT_RECONSTRUCTED_BOUNDED",
            "productJoin": "CURRENT",
            "userReview": "PENDING",
            "sourceProfileSha256": canonical_sha256(expected_profile),
            "nonMaterialSha256": F_NON_MATERIAL_SHA256,
            "rowSha256": canonical_sha256(f_element),
            "documentSha256": canonical_sha256(f_document),
        },
        "evidence": {
            "familyRowSha256": FAMILY_ROW_SHA256,
            "sourceBindingSha256": SOURCE_BINDING_SHA256,
            "dynamicModuleSha256": F_DYNAMIC_MODULE_SHA256,
            "resourceIdentity": dict(sorted(resources.items())),
        },
        "runtime": {
            "newCpp": False,
            "newHlsl": False,
            "existingExecutor": "Effect_DocumentRenderer profile 36",
            "existingEquation": "Shader_EffectCommon MakeFlow RT0 branch",
            "automaticState": "IMPLEMENTED_AUTOMATED",
            "visualState": "USER_REVIEW_PENDING",
            "evidence": runtime_evidence,
        },
    }
    receipt["artifactSha256"] = canonical_sha256(receipt)
    return receipt


def run(root: Path, mode: str) -> tuple[bool, dict[str, Any]]:
    root = root.resolve()
    validate_product_joins(root)
    resources = validate_resources(root)
    runtime_evidence = validate_profile36_runtime(root)
    d_document, d_element = validate_d_control(root)
    source_binding = source_material_binding(root)
    family = makeflow_family(root)
    expected_profile = build_effective_source_profile(source_binding, family)
    legacy_profile = build_legacy_source_profile(source_binding)
    require(canonical_sha256(legacy_profile) == F_LEGACY_SOURCE_PROFILE_SHA256,
            "legacy profile evidence changed")

    f_path = root / F_DOCUMENT
    raw = f_path.read_bytes()
    source_text = raw.decode("utf-8-sig")
    f_document = json.loads(source_text)
    f_element, promoted = validate_f_document(f_document, expected_profile)
    staged_document = copy.deepcopy(f_document)
    staged_element = unique_element(staged_document, F_ELEMENT_ID)
    staged_element["material"]["sourceProfile"] = copy.deepcopy(expected_profile)
    staged_text = serialize_preserving_non_targets(
        source_text, staged_document, {F_ELEMENT_ID})
    staged_element = unique_element(staged_document, F_ELEMENT_ID)
    validate_f_document(staged_document, expected_profile)

    receipt = build_receipt(
        d_document, d_element, staged_document, staged_element,
        expected_profile, resources, runtime_evidence)
    receipt_text = json.dumps(receipt, ensure_ascii=False, indent=2) + "\n"
    receipt_path = root / OUTPUT_RECEIPT
    receipt_current = receipt_path.is_file() and (
        receipt_path.read_text(encoding="utf-8-sig") == receipt_text)
    stale_document = not promoted or source_text != staged_text

    if mode == "check":
        require(not stale_document,
                "Lance F MakeFlow02 cohort materialization is stale")
        require(receipt_current, "Lance D/F V1 cohort receipt is stale")
        return False, receipt
    require(mode == "write", f"unsupported mode: {mode}")
    if stale_document:
        atomic_write(f_path, staged_text, bom=raw.startswith(codecs.BOM_UTF8))
    if not receipt_current:
        atomic_write(receipt_path, receipt_text)
    return stale_document or not receipt_current, receipt


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mode", choices=("check", "write"), default="check")
    parser.add_argument("--repository-root", type=Path, default=ROOT)
    arguments = parser.parse_args(argv)
    try:
        changed, receipt = run(arguments.repository_root, arguments.mode)
    except (LanceV1CohortError, OSError, json.JSONDecodeError) as error:
        print(f"FAIL: {error}")
        return 1
    print(json.dumps({
        "status": "updated" if changed else "stable",
        "dDisposition": "KEEP",
        "fDisposition": "KEEP",
        "fMaterialAction": "PROMOTE_EFFECTIVE_PARENT_PROFILE36",
        "fRuntimeProfile": 36,
        "fTextureLaneCount": 5,
        "dynamicSemantics": ["unbound"] * 4,
        "artifactSha256": receipt["artifactSha256"],
    }, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
