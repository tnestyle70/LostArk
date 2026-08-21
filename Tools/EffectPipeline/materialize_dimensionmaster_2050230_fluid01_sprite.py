#!/usr/bin/env python3
"""Promote the two Product DimensionMaster-F Fluid01 sprites to opcode 17.

The product evaluator is a bounded typed source reconstruction.  The source
parent and child parameter evidence closes four named texture roles and the
22-scalar packet, but it does not close a native UE3 vertex-factory/pass/DXBC
ABI.  This materializer therefore never claims or selects native DXBC.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import re
import tempfile
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
TARGET_RELATIVE_PATH = (
    "Data/Effects/Authored/"
    "effect.dimensionmaster.skill.2050230.unified.effect.json"
)
RECEIPT_RELATIVE_PATH = (
    "Data/Effects/AuthoredCorrections/Generated/DimensionMaster/"
    "effect.dimensionmaster.skill.2050230.fluid01-sprite.receipt.json"
)
SOURCE_EVIDENCE_RELATIVE_PATH = (
    "Data/Effects/Imported/DimensionMaster/ActionSource/"
    "DimensionMaster.source-material-evidence.json"
)
SOURCE_CONTRACT_RELATIVE_PATH = (
    "Data/Effects/AuthoredCorrections/Generated/"
    "FourClassCombat.source-material-contract.json"
)
ARTIST_F_RELATIVE_PATH = (
    "Data/Effects/Authored/effect.artist.skill.31470.unified.effect.json"
)

TARGET_EFFECT_ID = "effect.dimensionmaster.skill.2050230.unified"
TARGET_ELEMENT_IDS = (
    "authored.source-particle.1ae3416ac205fee634b746a9",
    "authored.source-particle.ed33fb10661afb8854e76957",
)
TARGET_SOURCE_NODES = {
    TARGET_ELEMENT_IDS[0]: (
        "authored-source-particle:effect.dimensionmaster.skill.2050230."
        "unified|source:effect.dimensionmaster.skill.2050230.imported|element:"
        "fx_pc_swp_03.par_s_swp_chrono_atk_01.particlespriteemitter_24"
    ),
    TARGET_ELEMENT_IDS[1]: (
        "authored-source-particle:effect.dimensionmaster.skill.2050230."
        "unified|source:effect.dimensionmaster.skill.2050230.imported|element:"
        "fx_pc_swp_03.par_s_swp_chrono_rewind_02.particlespriteemitter_37"
    ),
}
SOURCE_MATERIAL = "fx_m_mi_w_00.mi.fx_w_pa_fd_01_3_tr"
PARENT_MATERIAL = "fx_mastermaterial.fx_mm.fx_mm_fluid_01_tr"
PROFILE_ID = "ue3.material.fx.mastermaterial.fx.mm.fx.mm.fluid.01.tr.99f00cf3e57f"
PRE_SOURCE_PROFILE_SHA256 = (
    "a88bf80f220fb58e681942983be352fb50b9828447493869938367ae2333a322"
)
PARENT_PROPS_SHA256 = (
    "1f84c17359062aefd06b6735e206477fb3cf795e2d015fe453e480623d329638"
)
CHILD_PROPS_SHA256 = (
    "85b0d143b60c2f59aa7d2624d334e282541c4264e067b863d55bf517760d08bc"
)
RUNTIME_OPCODE = 17

ARTIST_F_BYTE_SHA256 = (
    "b503dc4fcf9f8f5408ca01f820c9716d18f65a4d742dd07624c7d5edc5251bbc"
)
ARTIST_F_CANONICAL_SHA256 = (
    "b1cc0de1e22731c16740a1152364481a0aecf731fbaf4f704cf3afdd755c3925"
)

UNOWNED_ELEMENT_SHA256 = {
    "authored.source-particle.e1db200793b7112c1dde8034":
        "313a54df962df5e474d3fd78469dd7f7b6d58dfb3ed492508d99968a65f8d04a",
    "authored.source-particle.78d3ec510ff0ee50ebdb6d04":
        "cc881cbe126a88520bfae36b6ff63d973886057978e8de54188b5d4eb9b2ecb1",
    "authored.source-particle.10113b0b33e8063631e88914":
        "907c02d3d48aab1118559e56091fa24a4affb1dc69bbc7836adbbf0d818b9183",
    "authored.source-particle.0f7aa9cd0601769f9e51f5cc":
        "715c7a4f3edd24399bbc585794d36530943fc4559ab95f2b67ed5392edb6c25a",
    "authored.source-particle.586d4f29b8ac91bd7d938052":
        "e539424695f22a0141126b0792c23c7e80f8b96252207d9224a0b4d115088d76",
    "authored.source-particle.fc4ef87d47a878a70100a4d8":
        "7bb5aac32f557b741c9c2cdd2e73a6d6e81f195c2afdbc51b73b56995c94d27e",
}

LANES = (
    {
        "laneId": "lane.0",
        "role": "transition_texture",
        "sourceParameterName": "transition texture",
        "assetId": (
            "Effect/DimensionMaster/Textures/FX_TEX_02/"
            "fx_d_cloud_035.dds"
        ),
        "sourceObjectPath": "fx_tex_02.fx_d_cloud_035",
        "valueSource": "INSTANCE_OVERRIDE",
        "sourceChannel": "RGB",
        "colorSpace": "linear",
        "sha256": (
            "fd6d84cf50bbdb17f23b0add48ea6d6244bcf65056ed3750f890293cfb7042e9"
        ),
    },
    {
        "laneId": "lane.1",
        "role": "emissive_tex",
        "sourceParameterName": "emissive_tex",
        "assetId": (
            "Effect/DimensionMaster/Textures/FX_TEX_HIGH_03/"
            "fx_o_glass_01.dds"
        ),
        "sourceObjectPath": "fx_tex_high_03.fx_o_glass_01",
        "valueSource": "INSTANCE_OVERRIDE",
        "sourceChannel": "RGB",
        "colorSpace": "linear",
        "sha256": (
            "60219a6a829e860cb426b85a329572b1e9642a9434aa89efc339be9c7cab3f08"
        ),
    },
    {
        "laneId": "lane.2",
        "role": "uv_noise_01_tex",
        "sourceParameterName": "uv_noise_01_tex",
        "assetId": (
            "Effect/DimensionMaster/Textures/FX_TEX_00/"
            "fx_bg_softriver_02_n.dds"
        ),
        "sourceObjectPath": "fx_bg_softriver_02_n",
        "valueSource": "PARENT_DEFAULT",
        "sourceChannel": "RG",
        "colorSpace": "linear",
        "sha256": (
            "86c9ba1f8301ba7bb5d22a0990b7c81b9858f53b81fd53f94c8e1b32f7cb2003"
        ),
    },
    {
        "laneId": "lane.3",
        "role": "uv_noise_02_tex",
        "sourceParameterName": "uv_noise_02_tex",
        # DimensionMaster's runtime subset does not contain this parent default.
        # The existing Warlord asset is the same source DDS, sealed by hash.
        "assetId": (
            "Effect/Warlord/Textures/FX_TEX_00/"
            "fx_bg_softriver_01_n.dds"
        ),
        "sourceObjectPath": "fx_bg_softriver_01_n",
        "valueSource": "PARENT_DEFAULT",
        "sourceChannel": "RG",
        "colorSpace": "linear",
        "sha256": (
            "a069682a18e82b6e2c0bac7001f1d117f3f9b77efa751c901f91bc9905bb79e1"
        ),
    },
)

SCALAR_VALUES = (
    ("transition_thickness", "transition thickness", 0.3),
    ("transition_direction", "transition direction", 0.1),
    ("transition_tiling", "transition_tiling", 4.0),
    ("transition_panning_y", "transition_panning_y", 0.2),
    ("transition_panning_x", "transition_panning_x", 0.02),
    ("emissive_line_intensity", "emissive_line_intensity", 2.0),
    ("transition_line_thickness", "transition line thickness", 2.0),
    ("uv_noise_01_tiling", "uv_noise_01_tiling", 0.5),
    ("uv_noise_01_panning_y", "uv_noise_01_panning_y", 0.1),
    ("uv_noise_01_panning_x", "uv_noise_01_panning_x", 0.2),
    ("uv_noise_01_intensity", "uv_noise_01_intensity", 0.0),
    ("uv_noise_02_tiling", "uv_noise_02_tiling", 0.7),
    ("uv_noise_02_panning_y", "uv_noise_02_panning_y", 0.07),
    ("uv_noise_02_panning_x", "uv_noise_02_panning_x", 0.15),
    ("uv_noise_02_intensity", "uv_noise_02_intensity", 0.15),
    ("emissive_intensity", "emissive_intensity", 1.0),
    ("emissive_desaturation", "emissive_desaturation", 0.0),
    ("emissive_uv_scale_x", "emissive_uv_scale_x", 2.0),
    ("emissive_uv_scale_y", "emissive_uv_scale_y", 2.0),
    ("fresnel_power", "fresnel_power", 1.0),
    ("distortion_intensity", "distortion_intensity", 1.0),
    ("total_scale", "total_scale", 1.0),
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def canonical(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical(value)).hexdigest()


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def exact_elements(document: dict[str, Any]) -> list[dict[str, Any]]:
    rows = [
        row for row in document.get("elements", [])
        if row.get("id") in TARGET_ELEMENT_IDS
    ]
    require(
        [row.get("id") for row in rows] == list(TARGET_ELEMENT_IDS),
        "Fluid01 Product occurrences are missing, duplicated, or reordered",
    )
    return rows


def default_sampler() -> dict[str, Any]:
    return {
        "filter": "linear",
        "addressU": "wrap",
        "addressV": "wrap",
        "addressW": "wrap",
        "mipLodBias": 0,
        "maxAnisotropy": 1,
        "comparison": "never",
        "borderColor": [0, 0, 0, 0],
        "minLod": 0,
        "maxLod": 3.40282347e38,
    }


def expected_execution() -> dict[str, Any]:
    lanes = []
    for index, source in enumerate(LANES):
        lanes.append({
            "laneId": source["laneId"],
            "role": source["role"],
            "assetId": source["assetId"],
            "textureRegister": index,
            "samplerRegister": 5 + index,
            "sourceChannel": source["sourceChannel"],
            "colorSpace": source["colorSpace"],
            "sampler": default_sampler(),
        })
    return {
        "enabled": True,
        "version": 1,
        "backend": "runtimeMaterialV2",
        "opcode": RUNTIME_OPCODE,
        "passIndex": 3,
        "renderState": {
            "rasterizer": "RS_Default",
            "depthStencil": "DSS_ReadOnly",
            "blend": "BS_EffectAlpha",
            "stencilReference": 0,
        },
        "textureLaneCount": 4,
        "textureMask": 15,
        "textureLanes": lanes,
        "dynamicConsumedMask": 7,
        "dynamicSuppressedMask": 8,
        "particleColorPolicy": 2,
        "particleColorConsumedMask": 15,
        "particleColorSuppressedMask": 0,
        "scalarCount": 22,
        "vectorCount": 0,
        "inputCount": 22,
        "inputConsumedMask": [0x003FFFFF, 0],
        "inputSuppressedMask": [0, 0],
        "vectorComponentConsumedMask": [0, 0, 0],
        "vectorComponentSuppressedMask": [0, 0, 0],
        "staticInputCount": 0,
        "staticSelectedMask": 0,
        "staticConsumedMask": 0,
        "staticSuppressedMask": 0,
        "renderInputCount": 0,
        "renderConsumedMask": 0,
        "renderSuppressedMask": 0,
        "scalars": [
            {"name": runtime_name, "packedIndex": index, "value": value}
            for index, (runtime_name, _source_name, value)
            in enumerate(SCALAR_VALUES)
        ],
        "vectors": [],
        "artistParameters": [],
        "colors": [],
    }


def validate_artist_f_golden(repository_root: Path) -> None:
    path = repository_root / ARTIST_F_RELATIVE_PATH
    require(file_sha256(path) == ARTIST_F_BYTE_SHA256,
            "Artist F golden byte identity changed")
    document = load_json(path)
    require(len(document.get("elements", [])) == 17,
            "Artist F golden element cardinality changed")
    require(canonical_sha256(document) == ARTIST_F_CANONICAL_SHA256,
            "Artist F golden semantic identity changed")


def validate_source_evidence(repository_root: Path) -> None:
    evidence = load_json(repository_root / SOURCE_EVIDENCE_RELATIVE_PATH)
    parent_matches = [
        row for row in evidence.get("parentMaterialEvidence", {}).values()
        if row.get("parentMaterialPath") == PARENT_MATERIAL
    ]
    require(len(parent_matches) == 1, "Fluid01 parent evidence is not singular")
    parent = parent_matches[0]
    require(parent.get("propsFileSha256") == PARENT_PROPS_SHA256,
            "Fluid01 parent props identity changed")
    material = parent.get("materialEvidence", {})
    roles = {
        row.get("name"): row.get("texture")
        for row in material.get("collectedTextureParameters", [])
    }
    require(roles == {
        "transition texture": "fx_c_noise_001",
        "emissive_tex": "fx_e_atypical_006",
        "uv_noise_01_tex": "fx_bg_softriver_02_n",
        "uv_noise_02_tex": "fx_bg_softriver_01_n",
    }, "Fluid01 parent texture role set changed")

    child_rows = evidence.get("materialInstances", {}).get(SOURCE_MATERIAL)
    if child_rows is None:
        # The source evidence schema stores material instances in a dynamically
        # named object on older captures.  Locate the unique matching mapping.
        child_rows = []
        for value in evidence.values():
            if isinstance(value, dict) and SOURCE_MATERIAL in value:
                child_rows = value[SOURCE_MATERIAL]
                break
    require(isinstance(child_rows, list) and len(child_rows) == 1,
            "Fluid01 child evidence is not singular")
    child = child_rows[0]
    require(child.get("parent") == PARENT_MATERIAL,
            "Fluid01 child parent changed")
    require(child.get("propsFileSha256") == CHILD_PROPS_SHA256,
            "Fluid01 child props identity changed")

    contract = load_json(repository_root / SOURCE_CONTRACT_RELATIVE_PATH)
    matches = [
        row for row in contract.get("materialIdentities", [])
        if row.get("sourceMaterialPath") == SOURCE_MATERIAL
    ]
    require(len(matches) == 1, "Fluid01 generated source contract is not singular")
    source = matches[0].get("sourceParameters", {})
    scalar_rows = {
        str(row.get("name")): float(row.get("value"))
        for row in source.get("scalars", [])
    }
    require(len(scalar_rows) == len(SCALAR_VALUES),
            "Fluid01 recovered scalar role set changed")
    for _runtime_name, source_name, value in SCALAR_VALUES:
        require(scalar_rows.get(source_name) == value,
                f"source scalar changed: {source_name}")
    texture_rows = {
        str(row.get("name")): row for row in source.get("textures", [])
        if row.get("name") in {lane["sourceParameterName"] for lane in LANES}
    }
    require(set(texture_rows) == {lane["sourceParameterName"] for lane in LANES},
            "Fluid01 source texture role set changed")
    for lane in LANES:
        row = texture_rows[lane["sourceParameterName"]]
        require(row.get("sourceObjectPath") == lane["sourceObjectPath"],
                f"source object changed: {lane['role']}")
        require(row.get("valueSource") == lane["valueSource"],
                f"source ownership changed: {lane['role']}")
        require(row.get("colorSpace") == "linear",
                f"source color space changed: {lane['role']}")


def validate_pre_source_profile(profile: dict[str, Any]) -> None:
    require(profile.get("enabled") is True, "Fluid01 source profile is disabled")
    require(profile.get("profileId") == PROFILE_ID, "Fluid01 profile changed")
    require(profile.get("parentMaterialPath") == PARENT_MATERIAL,
            "Fluid01 parent changed")
    require(profile.get("runtimeShaderProfileId") ==
            "effect.ue3.grouped-translucent.v1",
            "Fluid01 legacy profile changed")
    require(canonical_sha256(profile) == PRE_SOURCE_PROFILE_SHA256,
            "Fluid01 projected source profile changed before promotion")
    require(profile.get("vectors") == [], "Fluid01 unexpectedly gained vectors")


def validate_runtime_resources(repository_root: Path) -> dict[str, str]:
    resources_root = repository_root / "Client/Bin/Resources"
    result: dict[str, str] = {}
    for lane in LANES:
        path = resources_root / lane["assetId"]
        require(path.is_file(), f"runtime DDS missing: {lane['assetId']}")
        digest = file_sha256(path)
        require(digest == lane["sha256"],
                f"runtime DDS changed: {lane['assetId']}")
        result[lane["assetId"]] = digest
    return result


def validate_unowned_elements(document: dict[str, Any]) -> None:
    rows = {row.get("id"): row for row in document.get("elements", [])}
    require(len(rows) == len(document.get("elements", [])) == 8,
            "DimensionMaster F element identity/cardinality changed")
    for element_id, digest in UNOWNED_ELEMENT_SHA256.items():
        require(element_id in rows, f"unowned F row missing: {element_id}")
        require(canonical_sha256(rows[element_id]) == digest,
                f"unowned F row changed: {element_id}")


def promote_document(document: dict[str, Any]) -> tuple[dict[str, Any], bool]:
    require(document.get("effectAssetId") == TARGET_EFFECT_ID,
            "DimensionMaster F effect id changed")
    validate_unowned_elements(document)
    before = copy.deepcopy(document)
    expected = expected_execution()
    for target in exact_elements(document):
        element_id = target["id"]
        require(target.get("sourceNode") == TARGET_SOURCE_NODES[element_id],
                f"source node changed: {element_id}")
        require(target.get("kind") == "particle", f"carrier changed: {element_id}")
        require(target.get("sourceRecipe", {}).get("rendererShape") == "sprite",
                f"renderer shape changed: {element_id}")
        material = target.get("material") or {}
        require(material.get("sourceMaterialPath") == SOURCE_MATERIAL,
                f"Fluid01 child changed: {element_id}")
        require(material.get("renderProfile") ==
                "alpha_one_sided_depth_read",
                f"Fluid01 render profile changed: {element_id}")
        if material.get("templateId") == "effect.source_material":
            require("execution" not in material,
                    f"pre-promotion execution appeared: {element_id}")
            validate_pre_source_profile(material.get("sourceProfile") or {})
            material["templateId"] = "effect.standard"
            material["sourceProfile"] = {"enabled": False}
            material["execution"] = copy.deepcopy(expected)
        else:
            require(material.get("templateId") == "effect.standard",
                    f"post-promotion template changed: {element_id}")
            require(material.get("sourceProfile") == {"enabled": False},
                    f"post-promotion source profile changed: {element_id}")
            require(material.get("execution") == expected,
                    f"opcode 17 packet changed: {element_id}")

    after = document
    require([row.get("id") for row in before["elements"]] ==
            [row.get("id") for row in after["elements"]],
            "DimensionMaster F element order changed")
    for old in before["elements"]:
        current = next(row for row in after["elements"]
                       if row["id"] == old["id"])
        if old["id"] not in TARGET_ELEMENT_IDS:
            require(current == old, f"unowned F row changed: {old['id']}")
            continue
        for key in set(old) | set(current):
            if key != "material":
                require(old.get(key) == current.get(key),
                        f"non-material target field changed: {old['id']}:{key}")
    return document, before != document


def find_balanced_object_end(text: str, start: int) -> int:
    require(start < len(text) and text[start] == "{", "object start is invalid")
    depth = 0
    in_string = False
    escaped = False
    for index in range(start, len(text)):
        character = text[index]
        if in_string:
            if escaped:
                escaped = False
            elif character == "\\":
                escaped = True
            elif character == '"':
                in_string = False
            continue
        if character == '"':
            in_string = True
        elif character == "{":
            depth += 1
        elif character == "}":
            depth -= 1
            if depth == 0:
                return index + 1
    raise RuntimeError("unterminated object")


def replace_target_material(
    original: str, element_id: str, material: dict[str, Any]
) -> str:
    id_token = f'"id": {json.dumps(element_id)}'
    position = original.find(id_token)
    require(position >= 0 and original.find(id_token, position + 1) < 0,
            f"target text identity is missing or duplicated: {element_id}")
    next_id = original.find('"id": ', position + len(id_token))
    element_end = len(original) if next_id < 0 else next_id
    match = re.search(r'"material"\s*:\s*\{', original[position:element_end])
    require(match is not None, f"target material text is missing: {element_id}")
    start = position + match.end() - 1
    end = find_balanced_object_end(original, start)
    rendered = json.dumps(material, ensure_ascii=False, separators=(", ", ": "))
    return original[:start] + rendered + original[end:]


def build_receipt(resource_hashes: dict[str, str]) -> dict[str, Any]:
    execution = expected_execution()
    receipt = {
        "schema": "lostark.effect-fluid01-sprite-semantic-replay-receipt",
        "formatVersion": 1,
        "effectAssetId": TARGET_EFFECT_ID,
        "elementIds": list(TARGET_ELEMENT_IDS),
        "sourceNodes": dict(TARGET_SOURCE_NODES),
        "familyTuple": {
            "carrierVariantId": "carrier.cascade-sprite.source-v1",
            "materialVariantId": "material.fluid01.sprite.w-fd-01-3.semantic-v1",
            "renderVariantId": "render.particle.alpha-one-sided-depth-read.mrt2-v1",
            "compositionVariantId": "composition.dimensionmaster.2050230.f-source-v1",
        },
        "provenance": "SOURCE_EXACT",
        "evidence": "PARTIAL",
        "runtimeExecutor": "TYPED_SOURCE_RECONSTRUCTION",
        "runtimeAdmission": "AUTHORING_ONLY",
        "productJoin": "AUTHORED_NOT_PUBLISHED",
        "userReview": "PENDING",
        "runtimeMaterial": {
            "backend": "runtimeMaterialV2",
            "opcode": RUNTIME_OPCODE,
            "packetSha256": canonical_sha256(execution),
            "sourceMaterialPath": SOURCE_MATERIAL,
            "parentMaterialPath": PARENT_MATERIAL,
            "profileId": PROFILE_ID,
            "parentPropsSha256": PARENT_PROPS_SHA256,
            "childPropsSha256": CHILD_PROPS_SHA256,
            "textureRoles": [
                {
                    "laneId": lane["laneId"],
                    "role": lane["role"],
                    "sourceObjectPath": lane["sourceObjectPath"],
                    "valueSource": lane["valueSource"],
                    "assetId": lane["assetId"],
                    "sourceChannel": lane["sourceChannel"],
                    "colorSpace": lane["colorSpace"],
                    "sha256": lane["sha256"],
                }
                for lane in LANES
            ],
            "scalarCount": len(SCALAR_VALUES),
            "vectorCount": 0,
            "resourceSha256": dict(sorted(resource_hashes.items())),
        },
        "nativeOracle": {
            "selected": False,
            "runtimeAdmission": False,
            "pixelShaderSha256": None,
            "blockers": [
                "EXACT_CHILD_COOKED_SHADER_VARIANT_NOT_JOINED",
                "NATIVE_EMITTER_VERTEX_FACTORY_ABI_UNPROVEN",
                "NATIVE_PASS_AND_MRT_ABI_UNPROVEN",
                "SOURCE_SAMPLER_ABI_LEGACY_DEFAULT_ONLY",
            ],
        },
        "goldenControls": {
            "artistFByteSha256": ARTIST_F_BYTE_SHA256,
            "artistFCanonicalSha256": ARTIST_F_CANONICAL_SHA256,
            "unownedDimensionMasterFElementSha256":
                dict(sorted(UNOWNED_ELEMENT_SHA256.items())),
        },
    }
    receipt["artifactSha256"] = canonical_sha256(receipt)
    return receipt


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


def run(repository_root: Path, mode: str) -> tuple[bool, dict[str, Any]]:
    validate_artist_f_golden(repository_root)
    validate_source_evidence(repository_root)
    resource_hashes = validate_runtime_resources(repository_root)
    target_path = repository_root / TARGET_RELATIVE_PATH
    original = target_path.read_text(encoding="utf-8")
    document = json.loads(original)
    promoted, changed = promote_document(document)
    projected = original
    if changed:
        for target in exact_elements(promoted):
            projected = replace_target_material(
                projected, target["id"], target["material"]
            )
    require(json.loads(projected) == promoted,
            "Fluid01 text projection changed document semantics")
    receipt = build_receipt(resource_hashes)
    receipt_text = json.dumps(receipt, ensure_ascii=False, indent=2) + "\n"
    receipt_path = repository_root / RECEIPT_RELATIVE_PATH
    receipt_current = receipt_path.is_file() and (
        receipt_path.read_text(encoding="utf-8") == receipt_text
    )
    if mode == "write":
        if projected != original:
            atomic_write(target_path, projected)
        if not receipt_current:
            atomic_write(receipt_path, receipt_text)
        refreshed = load_json(target_path)
        _, still_changed = promote_document(refreshed)
        require(not still_changed, "Fluid01 materializer did not converge")
        require(receipt_path.read_text(encoding="utf-8") == receipt_text,
                "Fluid01 receipt write did not converge")
        return projected != original or not receipt_current, receipt
    require(projected == original, "Fluid01 Sprite materialization is stale")
    require(receipt_current, "Fluid01 Sprite receipt is stale")
    return False, receipt


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mode", choices=("check", "write"), default="check")
    parser.add_argument("--repository-root", type=Path, default=ROOT)
    arguments = parser.parse_args(argv)
    try:
        changed, receipt = run(arguments.repository_root.resolve(), arguments.mode)
    except (OSError, RuntimeError, ValueError, json.JSONDecodeError) as error:
        print(f"FAIL: {error}")
        return 1
    print(json.dumps({
        "status": "updated" if changed else "stable",
        "effectAssetId": TARGET_EFFECT_ID,
        "elementIds": list(TARGET_ELEMENT_IDS),
        "runtimeOpcode": RUNTIME_OPCODE,
        "artifactSha256": receipt["artifactSha256"],
    }, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
