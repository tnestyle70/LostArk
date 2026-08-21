#!/usr/bin/env python3
"""Materialize the bounded Lance dragon MeshParticle material family.

The five Product documents already contain the six body and six head source
occurrences.  This tool changes only those twelve rows.  It preserves every
other top-level element byte-for-byte, retains the disabled source profile as
evidence, and attaches RuntimeMaterialV2 opcode 19.

The recovered native pixel shader is a deferred five-MRT base-pass program and
the native emitter/VF binding array is unresolved.  Opcode 19 is therefore a
typed SceneColor semantic replay, never a native/source-exact runtime claim.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import tempfile
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
RESOURCE_ROOT = ROOT / "Client/Bin/Resources"
TARGET_MANIFEST = ROOT / (
    "Data/Effects/Imported/LanceMaster/Materials/"
    "skill.34630-34650.dragon.exact-shader-targets.json"
)
EXACT_RECEIPT = ROOT / (
    "Data/Effects/Imported/LanceMaster/Materials/"
    "skill.34630-34650.dragon.exact-material-maps.receipt.json"
)
OUTPUT_RECEIPT = ROOT / (
    "Data/Effects/AuthoredCorrections/Generated/LanceMaster/"
    "effect.lancemaster.skill.34630-34650.dragon-flow."
    "materialization-receipt.v1.json"
)

OPCODE = 19
PARENT_MATERIAL = "fx_m_mi_00.fx_m.fx_d_me_master_01_ph_msk"
PROFILE_ID = "ue3.material.fx.m.mi.00.fx.m.fx.d.me.master.01.ph.msk.8230663740c0"
NATIVE_DXBC_SHA256 = (
    "3dca33f820e403fe870c6d20e5cc14a2a9460c17db900f18b48d32e0e722f687"
)
STATIC_SELECTED_MASK = 0x0013B74F
STATIC_CONSUMED_MASK = 0x007FFFFF

SCALARS = (
    ("02.n.uvscale.x", 1.0),
    ("03.n.uvscale.y", 1.0),
    ("05.n.panning.x", 0.0),
    ("06.n.panning.y", 0.0),
    ("11.normalmap.str", 1.5),
    ("02.map_a_uvscale_r", 10.0),
    ("03.map_a_uvscale_g", 10.0),
    ("04.map_a_panning_x", 0.0),
    ("05.map_a_panning_y", -0.125),
    ("36.str", 10.0),
    ("37.power", 1.0),
    ("03.emap_uv.x.scale", 1.0),
    ("04.emap_uv.y.scale", 1.0),
    ("15.emissiion_power", 0.1),
    ("02.uvscale.x", 1.0),
    ("03.uvscale.y", 1.0),
    ("91.desaturation", 0.0),
    ("92.emissiion_power", 1.0),
    ("05.specmap_uvscale.x", 1.0),
    ("06.specmap_uvscale.y", 1.0),
    ("02.specmap_str", 0.25),
    ("07.desaturation", 0.0),
    ("08.specmap_power", 1.2),
    ("05.power", 2.0),
    ("06.str", 1.0),
)

VECTORS = (
    ("93.emissiion_color", [1.0, 1.0, 1.0, 1.0]),
    ("09.specmap_color", [5.0, 2.5, 0.75, 1.0]),
    ("19.emissiion_color", [10.0, 0.0, 0.0, 1.0]),
)

VARIANTS: dict[str, dict[str, str]] = {
    "body": {
        "child": "fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_01_msk",
        "mesh": "Effect/LanceMaster/Meshes/fm_x_flm_gdr_01.wmodel",
        "normal": "Effect/LanceMaster/Textures/sk_flm_gdr_01_n.dds",
        "alpha": "Effect/LanceMaster/Textures/fx_d_noise_043.dds",
        "emission": "Effect/LanceMaster/Textures/sk_flm_gdr_01_e.dds",
        "diffuse": "Effect/LanceMaster/Textures/sk_flm_gdr_01_d.dds",
        "specular": "Effect/LanceMaster/Textures/fx_d_atypical_010.dds",
    },
    "head": {
        "child": "fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_02_msk",
        "mesh": "Effect/LanceMaster/Meshes/fm_x_flm_gdr_01_dragon.wmodel",
        "normal": "Effect/LanceMaster/Textures/sk_flm_gdr_02_n.dds",
        "alpha": "Effect/LanceMaster/Textures/fx_d_noise_043.dds",
        "emission": "Effect/LanceMaster/Textures/sk_flm_gdr_02_e.dds",
        "diffuse": "Effect/LanceMaster/Textures/sk_flm_gdr_02_d.dds",
        "specular": "Effect/LanceMaster/Textures/sk_flm_gdr_02_s.dds",
    },
}

ROWS = (
    ("effect.lancemaster.skill.34630.clip1.unified",
     "authored.source-particle.2b0f00d91a20ba785ba034ec", "body",
     "fx_pc_flm_09.par_s_flm_superlance_wp_loop.particlespriteemitter_14",
     "FX_PC_DDK_03:export:4292@ref:5"),
    ("effect.lancemaster.skill.34630.clip1.unified",
     "authored.source-particle.71ac47f40d13b3a7ca6ed561", "head",
     "fx_pc_flm_09.par_s_flm_superlance_wp_loop.particlespriteemitter_15",
     "FX_PC_DDK_03:export:4292@ref:5"),
    ("effect.lancemaster.skill.34630.clip1.unified",
     "authored.source-particle.aa3beb2d7ebbe4922f6df595", "body",
     "fx_pc_flm_09.par_s_flm_superlance_wp_start.particlespriteemitter_3",
     "FX_PC_DDK_03:export:4292@ref:5"),
    ("effect.lancemaster.skill.34630.clip1.unified",
     "authored.source-particle.237b5cd9d1fafb4b95b41212", "head",
     "fx_pc_flm_09.par_s_flm_superlance_wp_start.particlespriteemitter_4",
     "FX_PC_DDK_03:export:4292@ref:5"),
    ("effect.lancemaster.skill.34630.clip2.unified",
     "authored.source-particle.6542736b94e7b9cd8ed5f2fd", "body",
     "fx_pc_flm_09.par_s_flm_superlance_wp_loop.particlespriteemitter_14."
     "event_source-event-033", "FX_PC_DDK_03:export:4292@ref:5"),
    ("effect.lancemaster.skill.34630.clip2.unified",
     "authored.source-particle.85571ac576a68cc3ff037cae", "head",
     "fx_pc_flm_09.par_s_flm_superlance_wp_loop.particlespriteemitter_15."
     "event_source-event-033", "FX_PC_DDK_03:export:4292@ref:5"),
    ("effect.lancemaster.skill.34630.clip3.unified",
     "authored.source-particle.92af24faaaeb30c6ac77d37c", "body",
     "fx_pc_flm_09.par_s_flm_superlance_wp_loop.particlespriteemitter_14."
     "event_source-event-048", "FX_PC_DDK_03:export:4292@ref:5"),
    ("effect.lancemaster.skill.34630.clip3.unified",
     "authored.source-particle.80d7156c29e9bf140631ad2e", "head",
     "fx_pc_flm_09.par_s_flm_superlance_wp_loop.particlespriteemitter_15."
     "event_source-event-048", "FX_PC_DDK_03:export:4292@ref:5"),
    ("effect.lancemaster.skill.34630.clip4.unified",
     "authored.source-particle.538fe0779d0718d30b68ef11", "body",
     "fx_pc_flm_09.par_s_flm_superlance_wp_end.particlespriteemitter_0",
     "FX_FS_AV_08:export:509@ref:5"),
    ("effect.lancemaster.skill.34630.clip4.unified",
     "authored.source-particle.50385d998091ed0e55a047f8", "head",
     "fx_pc_flm_09.par_s_flm_superlance_wp_end.particlespriteemitter_1",
     "FX_PC_DDK_03:export:4292@ref:5"),
    ("effect.lancemaster.skill.34650.clip1.unified",
     "authored.source-particle.0a019ebaff2bb55941d23ab8", "body",
     "fx_pc_flm_08.par_t_flm_dragoncleave_01_wpcast_01_s."
     "particlespriteemitter_14", "FX_PC_DDK_03:export:4292@ref:5"),
    ("effect.lancemaster.skill.34650.clip1.unified",
     "authored.source-particle.65b74589de96c3f44e625f24", "head",
     "fx_pc_flm_08.par_t_flm_dragoncleave_01_wpcast_01_s."
     "particlespriteemitter_15", "FX_PC_DDK_03:export:4292@ref:5"),
)

RESOURCE_SHA256 = {
    "Effect/LanceMaster/Meshes/fm_x_flm_gdr_01.wmodel":
        "c8426d51f9c494988798dd2ed620ff509066863e14ed82c77262787757b50004",
    "Effect/LanceMaster/Meshes/fm_x_flm_gdr_01_dragon.wmodel":
        "815529e09b960016a189dc5f441ec93ca810b6ad6086466d140ef6d53989f3ac",
    "Effect/LanceMaster/Textures/sk_flm_gdr_01_n.dds":
        "dc5816742ac1baba1b307492caa48970476c3d149319b780d81d577e4b15ddf1",
    "Effect/LanceMaster/Textures/sk_flm_gdr_01_d.dds":
        "79ad7fa4be8ab2b1359bc16933305efc47546dc3e96769c0758b056964b218b0",
    "Effect/LanceMaster/Textures/sk_flm_gdr_01_e.dds":
        "a0d30e9c8be7899516743ade58a2835641d5e0aa7d99c36e3df720b60cc5b9ac",
    "Effect/LanceMaster/Textures/fx_d_noise_043.dds":
        "04546eb958938a8fd102bd94c43efaa0cda153a05c08ac7477b5d8fcb527a20b",
    "Effect/LanceMaster/Textures/fx_d_atypical_010.dds":
        "93fa9c935e9be8f68ea5f686d574dcd723648ae3d3e2f0bcbbde246b94542b96",
    "Effect/LanceMaster/Textures/sk_flm_gdr_02_n.dds":
        "1fb36517a26c3aed30ed079fc1c2300eaeb38ac6568c57da8ca65768b90d4c7e",
    "Effect/LanceMaster/Textures/sk_flm_gdr_02_d.dds":
        "632cbb8870064c709b5699adf0ae518999fb3c3622141e7018ccf313aaa79bcc",
    "Effect/LanceMaster/Textures/sk_flm_gdr_02_e.dds":
        "0cff8459ff727e774f6105787ebd4817434d467c9d73a4f219d88a036dea7905",
    "Effect/LanceMaster/Textures/sk_flm_gdr_02_s.dds":
        "a626399c80a73caf26f062d041963cf425dbf3abd6949dd188d1525e8683ae39",
}


class DragonMaterializationError(RuntimeError):
    """Raised when the exact twelve-row contract drifts."""


def require(condition: bool, message: str) -> None:
    if not condition:
        raise DragonMaterializationError(message)


def canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def raw_sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8-sig"))
    require(isinstance(value, dict), f"JSON root must be an object: {path}")
    return value


def sampler() -> dict[str, Any]:
    return {
        "filter": "linear",
        "addressU": "wrap",
        "addressV": "wrap",
        "addressW": "wrap",
        "mipLodBias": 0.0,
        "maxAnisotropy": 1,
        "comparison": "never",
        "borderColor": [0.0, 0.0, 0.0, 0.0],
        "minLod": 0.0,
        "maxLod": 3.40282347e38,
    }


def expected_resources(variant: dict[str, str]) -> list[dict[str, str]]:
    return [
        {"slotId": "meshModel", "assetId": variant["mesh"]},
        {"slotId": "base", "assetId": variant["diffuse"]},
        {"slotId": "dissolve", "assetId": variant["alpha"]},
        {"slotId": "noise", "assetId": variant["normal"]},
        {"slotId": "mask", "assetId": variant["specular"]},
        {"slotId": "emissive", "assetId": variant["emission"]},
    ]


def expected_execution(variant: dict[str, str]) -> dict[str, Any]:
    lane_values = (
        ("normal_map", variant["normal"], "RG"),
        ("alpha_map", variant["alpha"], "R"),
        ("emission_map", variant["emission"], "RGB"),
        ("diffuse_map", variant["diffuse"], "RGB"),
        ("specular_map", variant["specular"], "RGB"),
    )
    return {
        "enabled": True,
        "version": 1,
        "backend": "runtimeMaterialV2",
        "opcode": OPCODE,
        "passIndex": 3,
        "renderState": {
            "rasterizer": "RS_Default",
            "depthStencil": "DSS_ReadOnly",
            "blend": "BS_EffectAlpha",
            "stencilReference": 0,
        },
        "textureLaneCount": 5,
        "textureMask": 31,
        "textureLanes": [
            {
                "laneId": f"lane.{index}",
                "role": role,
                "assetId": asset_id,
                "textureRegister": index,
                "samplerRegister": 5 + index,
                "sourceChannel": channel,
                "colorSpace": "linear",
                "sampler": sampler(),
            }
            for index, (role, asset_id, channel) in enumerate(lane_values)
        ],
        "dynamicConsumedMask": 8,
        "dynamicSuppressedMask": 7,
        "particleColorPolicy": 2,
        "particleColorConsumedMask": 15,
        "particleColorSuppressedMask": 0,
        "scalarCount": len(SCALARS),
        "vectorCount": len(VECTORS),
        "inputCount": len(SCALARS),
        "inputConsumedMask": [(1 << len(SCALARS)) - 1, 0],
        "inputSuppressedMask": [0, 0],
        "vectorComponentConsumedMask": [7, 7, 7],
        "vectorComponentSuppressedMask": [8, 8, 8],
        "staticInputCount": 23,
        "staticSelectedMask": STATIC_SELECTED_MASK,
        "staticConsumedMask": STATIC_CONSUMED_MASK,
        "staticSuppressedMask": 0,
        "renderInputCount": 6,
        "renderConsumedMask": 47,
        "renderSuppressedMask": 16,
        "scalars": [
            {"name": name, "packedIndex": index, "value": value}
            for index, (name, value) in enumerate(SCALARS)
        ],
        "vectors": [
            {"name": name, "packedIndex": index, "value": value}
            for index, (name, value) in enumerate(VECTORS)
        ],
        "artistParameters": [],
        "colors": [],
    }


def validate_exact_evidence() -> None:
    manifest = load_json(TARGET_MANIFEST)
    receipt = load_json(EXACT_RECEIPT)
    expected_ids = {row[1] for row in ROWS}
    manifest_ids = {
        element_id
        for target in manifest.get("targets", [])
        for element_id in target.get("occurrenceIds", [])
    }
    summary = receipt.get("summary", {})
    require(manifest_ids == expected_ids, "dragon exact occurrence evidence drifted")
    require(
        manifest.get("identity", {}).get("effectAssetIds") ==
        sorted({row[0] for row in ROWS}),
        "dragon exact effect-document identity drifted",
    )
    require(
        summary.get("targetCount") == 2
        and summary.get("exactPixelShaderDxbcCount") == 2
        and summary.get("uniqueExactPixelShaderDxbcCount") == 1
        and summary.get("exactNativeShaderObjectBindingCount") == 0
        and summary.get("actualVfPassAdmissionCount") == 0
        and summary.get("runtimeAdmissionCount") == 0,
        "dragon native shader admission boundary drifted",
    )
    cooked_hashes = {
        target.get("cookedPixelShader", {}).get("dxbc", {}).get("sha256")
        for target in receipt.get("targets", [])
    }
    require(cooked_hashes == {NATIVE_DXBC_SHA256}, "dragon DXBC identity drifted")


def validate_resources() -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for asset_id, expected_hash in RESOURCE_SHA256.items():
        path = RESOURCE_ROOT / asset_id
        require(path.is_file(), f"dragon runtime resource is missing: {asset_id}")
        actual_hash = raw_sha256(path)
        require(actual_hash == expected_hash,
                f"dragon runtime resource changed: {asset_id}/{actual_hash}")
        result[asset_id] = {"byteSize": path.stat().st_size, "sha256": actual_hash}
    return result


def expected_source_node(effect_id: str, source_element: str) -> str:
    imported = effect_id.replace("effect.lancemaster", "effect.lance_master")
    imported = imported.replace(".clip1", "").replace(".clip2", "")
    imported = imported.replace(".clip3", "").replace(".clip4", "")
    imported = imported.removesuffix(".unified") + ".imported"
    return (
        f"authored-source-particle:{effect_id}|source:{imported}|element:"
        f"{source_element}"
    )


def validate_dynamic_module(
    element: dict[str, Any], expected_stable_id: str
) -> None:
    matches = [
        module for module in element.get("sourceRecipe", {}).get("modules", [])
        if module.get("className") == "particlemoduleparameterdynamic"
    ]
    require(len(matches) == 1, f"dynamic module count changed: {element.get('id')}")
    module = matches[0]
    require(module.get("stableId") == expected_stable_id,
            f"dynamic module identity changed: {element.get('id')}")
    distributions = module.get("distributions", [])
    require(len(distributions) == 4,
            f"dynamic distribution count changed: {element.get('id')}")
    for index, distribution in enumerate(distributions):
        require(
            distribution.get("propertyPath") ==
            f"dynamicparams[{index}].paramvalue"
            and distribution.get("lookupTable") == [1.0, 1.0, 1.0, 1.0],
            f"dynamic constant-one lane changed: {element.get('id')}/{index}",
        )


def validate_source_profile(profile: dict[str, Any], variant: dict[str, str]) -> None:
    require(profile.get("profileId") == PROFILE_ID,
            "dragon source profile identity changed")
    require(profile.get("parentMaterialPath") == PARENT_MATERIAL,
            "dragon parent Material changed")
    require(profile.get("staticSwitches") == [],
            "projected source static-switch payload changed")
    scalar_map = {row.get("name"): row.get("value")
                  for row in profile.get("scalars", [])}
    for name, value in SCALARS:
        require(scalar_map.get(name) == value, f"dragon source scalar changed: {name}")
    vector_map = {row.get("name"): row.get("value")
                  for row in profile.get("vectors", [])}
    for name, value in VECTORS:
        require(vector_map.get(name) == value, f"dragon source vector changed: {name}")
    texture_map = {row.get("name"): row for row in profile.get("textures", [])}
    source_textures = {
        "01.map.n": variant["normal"],
        "01.map_a": variant["alpha"],
        "01.emismap": variant["emission"],
        "01.map.d": variant["diffuse"],
    }
    for name, asset_id in source_textures.items():
        row = texture_map.get(name, {})
        require(
            row.get("assetId") == asset_id
            and row.get("addressU") == "wrap"
            and row.get("addressV") == "wrap"
            and row.get("colorSpace") == "linear",
            f"dragon projected source texture changed: {name}",
        )
    spec = texture_map.get("01.specmap", {})
    require(spec.get("assetId") in ("", variant["specular"]),
            "dragon projected source specular identity changed")


def promote_element(
    element: dict[str, Any], effect_id: str, variant_id: str,
    source_element: str, dynamic_stable_id: str,
) -> bool:
    variant = VARIANTS[variant_id]
    element_id = element.get("id")
    require(element.get("visible") is True, f"dragon visibility changed: {element_id}")
    require(element.get("kind") == "particle", f"dragon kind changed: {element_id}")
    require(element.get("sourceNode") == expected_source_node(effect_id, source_element),
            f"dragon sourceNode changed: {element_id}")
    recipe = element.get("sourceRecipe", {})
    require(recipe.get("enabled") is True and recipe.get("rendererShape") == "mesh",
            f"dragon MeshParticle carrier changed: {element_id}")
    validate_dynamic_module(element, dynamic_stable_id)
    material = element.get("material", {})
    require(material.get("sourceMaterialPath") == variant["child"],
            f"dragon MIC child changed: {element_id}")
    source_profile = material.get("sourceProfile", {})
    validate_source_profile(source_profile, variant)
    expected = expected_execution(variant)
    expected_post_resources = expected_resources(variant)
    if material.get("templateId") == "effect.source_material":
        require(material.get("renderProfile") == "alpha_two_sided_depth_read",
                f"dragon pre-promotion render profile changed: {element_id}")
        require(source_profile.get("enabled") is True,
                f"dragon pre-promotion source profile disabled: {element_id}")
        pre_resources = [
            {"slotId": "meshModel", "assetId": variant["mesh"]},
            {"slotId": "base", "assetId": variant["emission"]},
            {"slotId": "noise", "assetId": variant["diffuse"]},
            {"slotId": "mask", "assetId": variant["normal"]},
            {"slotId": "emissive", "assetId": variant["alpha"]},
        ]
        require(element.get("resources") == pre_resources,
                f"dragon generic pre-promotion bindings changed: {element_id}")
        material["templateId"] = "effect.standard"
        material["renderProfile"] = "alpha_one_sided_depth_read"
        source_profile["enabled"] = False
        material["execution"] = expected
        element["resources"] = expected_post_resources
        return True
    require(material.get("templateId") == "effect.standard",
            f"dragon post-promotion template changed: {element_id}")
    require(material.get("renderProfile") == "alpha_one_sided_depth_read",
            f"dragon post-promotion render profile changed: {element_id}")
    require(source_profile.get("enabled") is False,
            f"dragon post-promotion source profile enabled: {element_id}")
    require(material.get("execution") == expected,
            f"dragon opcode 19 packet changed: {element_id}")
    require(element.get("resources") == expected_post_resources,
            f"dragon typed bindings changed: {element_id}")
    return False


def extract_element_blocks(
    document_text: str,
) -> tuple[str, str, dict[str, dict[str, Any]], dict[str, str]]:
    marker = document_text.find('"elements"')
    array_start = document_text.find("[", marker)
    require(marker >= 0 and array_start >= 0, "document has no elements array")
    decoder = json.JSONDecoder()
    position = array_start + 1
    first_start: int | None = None
    last_end: int | None = None
    parsed: dict[str, dict[str, Any]] = {}
    blocks: dict[str, str] = {}
    while True:
        while position < len(document_text) and document_text[position] in " \t\r\n,":
            position += 1
        require(position < len(document_text), "elements array is truncated")
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
    source_text: str, target: dict[str, Any], target_ids: set[str]
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
    require(json.loads(output) == target, "raw-preserving projection changed semantics")
    return output


def atomic_write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary: Path | None = None
    try:
        descriptor, name = tempfile.mkstemp(
            prefix=path.name + ".", suffix=".tmp", dir=path.parent
        )
        temporary = Path(name)
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="") as stream:
            stream.write(text)
            stream.flush()
            os.fsync(stream.fileno())
        json.loads(temporary.read_text(encoding="utf-8"))
        os.replace(temporary, path)
        temporary = None
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def build_receipt(
    resources: dict[str, dict[str, Any]], document_hashes: dict[str, str]
) -> dict[str, Any]:
    receipt: dict[str, Any] = {
        "schema": "lostark.effect-lancemaster-dragon-semantic-replay-receipt",
        "formatVersion": 1,
        "identity": {
            "characterClass": "LANCE_MASTER",
            "skillIds": [34630, 34650],
            "effectAssetIds": sorted({row[0] for row in ROWS}),
            "occurrenceIds": [row[1] for row in ROWS],
        },
        "familyTuple": {
            "carrierVariantId": "carrier.cascade-mesh-particle.local-v1",
            "materialVariantId": "material.dragon-ph-masked.semantic-v1",
            "renderVariantId": "render.mesh.alpha-one-sided-depth-read.mrt2-v1",
            "compositionVariantId": "composition.lancemaster.dragon-source-cohort-v1",
        },
        "provenance": "SOURCE_EXACT",
        "evidence": "PARTIAL",
        "runtimeExecutor": "TYPED_HLSL_SEMANTIC_REPLAY",
        "runtimeFidelityClaim": "TYPED_SEMANTIC_REPLAY_NOT_SOURCE_EXACT",
        "runtimeAdmission": "AUTHORING_ONLY",
        "productJoin": "AUTHORED_NOT_PUBLISHED",
        "userReview": "PENDING",
        "runtimeMaterial": {
            "backend": "runtimeMaterialV2",
            "opcode": OPCODE,
            "parentMaterialPath": PARENT_MATERIAL,
            "children": {
                variant_id: {
                    "sourceMaterialPath": variant["child"],
                    "packetSha256": canonical_sha256(expected_execution(variant)),
                    "meshAssetId": variant["mesh"],
                }
                for variant_id, variant in VARIANTS.items()
            },
            "laneOrder": [
                "normal_map", "alpha_map", "emission_map", "diffuse_map",
                "specular_map",
            ],
            "alphaUv": {
                "scale": [10.0, 10.0],
                "panPerSecond": [0.0, -0.125],
                "sourceChannel": "R",
                "invert": True,
                "dissolveThreshold": "dynamicParameter.w",
            },
            "stationaryUvLanes": [
                "normal_map", "emission_map", "diffuse_map", "specular_map"
            ],
            "lifetimeEnvelope": "particleColor.a_independent_of_dynamic_w",
            "staticSwitchCount": 23,
            "staticSelectedMask": STATIC_SELECTED_MASK,
            "resourceIdentity": dict(sorted(resources.items())),
        },
        "nativeOracle": {
            "pixelShaderSha256": NATIVE_DXBC_SHA256,
            "runtimeAdmission": False,
            "blockers": [
                "NATIVE_SHADER_OBJECT_BINDING_ARRAY_UNRESOLVED",
                "NATIVE_MESH_PARTICLE_VERTEX_FACTORY_ABI_UNPROVEN",
                "SOURCE_DEFERRED_FIVE_MRT_PASS_UNAVAILABLE_IN_EFFECT_COMPOSITION",
            ],
        },
        "boundedReplay": {
            "sceneColorAndDistortionMrtOnly": True,
            "sourceBlendIntent": "MASKED_ONE_SIDED_DEPTH_WRITE",
            "runtimeRenderState": "ALPHA_ONE_SIDED_DEPTH_READ_WITH_EXPLICIT_CLIP",
            "knownDifference": "NO_SOURCE_DEFERRED_GBUFFER_OR_DEPTH_WRITE",
        },
        "documents": dict(sorted(document_hashes.items())),
    }
    receipt["artifactSha256"] = canonical_sha256(receipt)
    return receipt


def run(repository_root: Path, mode: str) -> tuple[bool, dict[str, Any]]:
    global ROOT, RESOURCE_ROOT, TARGET_MANIFEST, EXACT_RECEIPT, OUTPUT_RECEIPT
    ROOT = repository_root
    RESOURCE_ROOT = ROOT / "Client/Bin/Resources"
    TARGET_MANIFEST = ROOT / (
        "Data/Effects/Imported/LanceMaster/Materials/"
        "skill.34630-34650.dragon.exact-shader-targets.json"
    )
    EXACT_RECEIPT = ROOT / (
        "Data/Effects/Imported/LanceMaster/Materials/"
        "skill.34630-34650.dragon.exact-material-maps.receipt.json"
    )
    OUTPUT_RECEIPT = ROOT / (
        "Data/Effects/AuthoredCorrections/Generated/LanceMaster/"
        "effect.lancemaster.skill.34630-34650.dragon-flow."
        "materialization-receipt.v1.json"
    )
    validate_exact_evidence()
    resources = validate_resources()
    rows_by_effect: dict[str, list[tuple[str, str, str, str, str]]] = {}
    for row in ROWS:
        rows_by_effect.setdefault(row[0], []).append(row)
    changed_any = False
    projected_documents: dict[Path, str] = {}
    document_hashes: dict[str, str] = {}
    for effect_id, rows in rows_by_effect.items():
        path = ROOT / "Data/Effects/Authored" / f"{effect_id}.effect.json"
        original_text = path.read_text(encoding="utf-8-sig")
        document = json.loads(original_text)
        require(document.get("effectAssetId") == effect_id,
                f"effect ID changed: {path}")
        before = copy.deepcopy(document)
        targets = {row[1]: row for row in rows}
        found: set[str] = set()
        for element in document.get("elements", []):
            row = targets.get(element.get("id"))
            if row is None:
                continue
            found.add(row[1])
            changed_any = promote_element(
                element, row[0], row[2], row[3], row[4]
            ) or changed_any
        require(found == set(targets), f"target occurrence set changed: {effect_id}")
        before_by_id = {row["id"]: row for row in before["elements"]}
        for element in document["elements"]:
            if element["id"] not in targets:
                require(element == before_by_id[element["id"]],
                        f"unowned element changed: {element['id']}")
            else:
                old = before_by_id[element["id"]]
                for key in set(old) | set(element):
                    if key not in {"material", "resources"}:
                        require(old.get(key) == element.get(key),
                                f"non-material target field changed: {element['id']}/{key}")
        projected = serialize_preserving_non_targets(
            original_text, document, set(targets)
        )
        projected_documents[path] = projected
        document_hashes[path.relative_to(ROOT).as_posix()] = canonical_sha256(document)

    receipt = build_receipt(resources, document_hashes)
    receipt_text = json.dumps(receipt, ensure_ascii=False, indent=2) + "\n"
    receipt_current = OUTPUT_RECEIPT.is_file() and (
        OUTPUT_RECEIPT.read_text(encoding="utf-8") == receipt_text
    )
    stale_documents = {
        path: projected for path, projected in projected_documents.items()
        if path.read_text(encoding="utf-8-sig") != projected
    }
    if mode == "write":
        for path, projected in stale_documents.items():
            atomic_write(path, projected)
        if not receipt_current:
            atomic_write(OUTPUT_RECEIPT, receipt_text)
        return bool(stale_documents) or not receipt_current, receipt
    require(not stale_documents, "dragon Product materialization is stale")
    require(receipt_current, "dragon materialization receipt is stale")
    return False, receipt


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mode", choices=("check", "write"), default="check")
    parser.add_argument("--repository-root", type=Path, default=ROOT)
    arguments = parser.parse_args(argv)
    try:
        changed, receipt = run(arguments.repository_root.resolve(), arguments.mode)
    except (OSError, ValueError, json.JSONDecodeError,
            DragonMaterializationError) as error:
        print(f"FAIL: {error}")
        return 1
    print(json.dumps({
        "status": "updated" if changed else "stable",
        "runtimeOpcode": OPCODE,
        "effectDocumentCount": len({row[0] for row in ROWS}),
        "occurrenceCount": len(ROWS),
        "artifactSha256": receipt["artifactSha256"],
    }, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
