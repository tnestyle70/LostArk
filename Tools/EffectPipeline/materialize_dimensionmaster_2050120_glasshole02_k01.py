#!/usr/bin/env python3
"""Promote the single Product K-child Glasshole02 sprite to opcode 16.

The exact native DXBC remains oracle-only: its particle VF, six-slot MRT pass,
and source sampler ABI are not closed.  Product executes the already recovered
profile-29 equation through a class-neutral RuntimeMaterialV2 packet.  This
materializer admits one exact occurrence and never promotes the J-child rows.
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
    "effect.dimensionmaster.skill.2050120.clip3.unified.effect.json"
)
RECEIPT_RELATIVE_PATH = (
    "Data/Effects/AuthoredCorrections/Generated/DimensionMaster/"
    "effect.dimensionmaster.skill.2050120.clip3.glasshole02-k01.receipt.json"
)
ORACLE_RELATIVE_PATH = (
    "Data/Effects/Contracts/ue3-exact-cooked-shader-variants.v1.json"
)
TARGET_EFFECT_ID = "effect.dimensionmaster.skill.2050120.clip3.unified"
TARGET_ELEMENT_ID = "authored.source-particle.40e1b48e2f0f88dcfeff1549"
TARGET_SOURCE_NODE = (
    "authored-source-particle:effect.dimensionmaster.skill.2050120.clip3."
    "unified|source:effect.dimensionmaster.skill.2050120.imported|element:"
    "fx_pc_swp_05.par_k_swp_tentdevider_atk_00_01."
    "particlespriteemitter_47"
)
SOURCE_MATERIAL = "fx_m_mi_k_00.fx_mi.fx_k_pa_glasshole_02_01_tr"
PARENT_MATERIAL = "fx_m_mi_j_00.fx_m.fx_j_pa_glasshole_02_tr"
PROFILE_ID = "ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.glasshole.02.tr.175266c16bb2"
PRE_SOURCE_PROFILE_SHA256 = (
    "57fb8ac503c7c6e2656536d3e3f09b78e7af95e243483fafb54018e55498b8ce"
)
RUNTIME_OPCODE = 16
NATIVE_DXBC_SHA256 = (
    "e2ba1c1ef87cdd52cc74a8e661f8613d0b17f2cc7b9b1d0d7ab6ed80ec6e775b"
)
NATIVE_VARIANT_KEY_SHA256 = (
    "1065b5004b127da3e2c7a0342a0f5c7405514337a56cac2bd22edce6c718d3cf"
)
EFFECTIVE_STATIC_SET_SHA256 = (
    "126881131d06544eff5f47c996bae29975b64dda72346c14c0b7e28b17025d9f"
)

LANES = (
    {
        "laneId": "lane.0",
        "role": "aura_texture",
        "assetId": (
            "Effect/DimensionMaster/Textures/FX_TEX_06/"
            "fx_j_dustparticle_tile_02.dds"
        ),
        "sourceChannel": "RGBA",
        "colorSpace": "srgb",
        "sha256": (
            "57f00863a0be5449b1c0c5b07d9d44055f1cc1fc043bac481b83810e2c995c8f"
        ),
    },
    {
        "laneId": "lane.1",
        "role": "cracknormal_tex",
        "assetId": (
            "Effect/DimensionMaster/Textures/FX_TEX_06/"
            "fx_j_normal_bc5_09.dds"
        ),
        "sourceChannel": "RG",
        "colorSpace": "linear",
        "sha256": (
            "75829aa6ea4c6f8a4bd3c2f646dac2ce08071af06052d3b048e5c558d6f30f44"
        ),
    },
    {
        "laneId": "lane.2",
        "role": "in_hole_texture",
        "assetId": (
            "Effect/DimensionMaster/Textures/FX_TEX_06/"
            "fx_m_cybernoise_02.dds"
        ),
        "sourceChannel": "RGB",
        "colorSpace": "srgb",
        "sha256": (
            "389d6a10a5f3d321a1c085e42a9ac9cd6d2c82366cfc8c24690626610efe70b3"
        ),
    },
)

SCALAR_VALUES = (
    ("alpha_tile_x", 0.400000006),
    ("alpha_tile_y", 0.400000006),
    ("alpha_offsetx", 0.5),
    ("alpha_offsety", 0.5),
    ("aura_str", 5.0),
    ("aura_pow", 2.0),
    ("curve_power", 2.0),
    ("twist_str", 0.100000001),
    ("main_ucoord", 1.0),
    ("main_v_coord", 0.0),
    ("main_tex_upanner", 0.0),
    ("main_v_panner", -0.800000012),
    ("uvnoise_utile", 1.0),
    ("uvnoise_vtile", 1.0),
    ("uvnoise_pan", 0.0),
    ("in_hole_crackuv", 0.200000003),
    ("in_hole_panx", 0.0149999997),
    ("in_hole_pany", 0.0250000004),
    ("in_hole_pow", 2.0),
    ("in_hole_str", 1.0),
    ("in_hole_desaturation", 0.300000012),
    ("distortionpower", 10.0),
    ("distortionscale", -15.0),
    ("scale", 1.0),
    ("cracknormal_tile_x", 4.0),
    ("cracknormal_tile_y", 0.0),
    ("cracknormal_str", -7.0),
    ("edge_crack_desaturation", 0.800000012),
    ("edge_line", 4.0),
    ("edge_size", 2.0),
    ("time", 0.0),
    ("in_hole_height", 0.0),
)

VECTOR_VALUES = (
    ("aura_color", [2.0, 1.20000005, 0.800000012, 1.0]),
    ("in_hole_color", [1.0, 1.0, 1.0, 1.0]),
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


def exact_element(document: dict[str, Any]) -> dict[str, Any]:
    matches = [
        row for row in document.get("elements", [])
        if row.get("id") == TARGET_ELEMENT_ID
    ]
    require(len(matches) == 1, "Glasshole02 K-child occurrence is not singular")
    return matches[0]


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
    texture_lanes = []
    for index, source in enumerate(LANES):
        texture_lanes.append({
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
        "passIndex": 1,
        "renderState": {
            "rasterizer": "RS_Cull_None",
            "depthStencil": "DSS_ReadOnly",
            "blend": "BS_EffectAlpha",
            "stencilReference": 0,
        },
        "textureLaneCount": 3,
        "textureMask": 7,
        "textureLanes": texture_lanes,
        "dynamicConsumedMask": 1,
        "dynamicSuppressedMask": 14,
        "particleColorPolicy": 2,
        "particleColorConsumedMask": 15,
        "particleColorSuppressedMask": 0,
        "scalarCount": 32,
        "vectorCount": 2,
        "inputCount": 34,
        "inputConsumedMask": [2013236415, 3],
        "inputSuppressedMask": [2281730880, 0],
        "vectorComponentConsumedMask": [7, 7, 0],
        "vectorComponentSuppressedMask": [8, 8, 0],
        "staticInputCount": 6,
        "staticSelectedMask": 36,
        "staticConsumedMask": 63,
        "staticSuppressedMask": 0,
        "renderInputCount": 6,
        "renderConsumedMask": 47,
        "renderSuppressedMask": 16,
        "scalars": [
            {"name": name, "packedIndex": index, "value": value}
            for index, (name, value) in enumerate(SCALAR_VALUES)
        ],
        "vectors": [
            {"name": name, "packedIndex": index, "value": value}
            for index, (name, value) in enumerate(VECTOR_VALUES)
        ],
        "artistParameters": [],
        "colors": [],
    }


def validate_pre_source_profile(profile: dict[str, Any]) -> None:
    require(profile.get("enabled") is True, "source profile is disabled")
    require(profile.get("profileId") == PROFILE_ID, "profile identity changed")
    require(
        profile.get("parentMaterialPath") == PARENT_MATERIAL,
        "Glasshole02 parent changed",
    )
    require(
        profile.get("runtimeShaderProfileId") == "effect.ue3.glasshole-02.v1",
        "bounded Glasshole02 profile changed",
    )
    require(
        canonical_sha256(profile) == PRE_SOURCE_PROFILE_SHA256,
        "projected source profile changed before opcode promotion",
    )

    scalar_rows = {
        str(row.get("name")): float(row.get("value"))
        for row in profile.get("scalars", [])
    }
    vector_rows = {
        str(row.get("name")): [float(value) for value in row.get("value", [])]
        for row in profile.get("vectors", [])
    }
    require(len(scalar_rows) == len(SCALAR_VALUES), "scalar role set changed")
    require(len(vector_rows) == len(VECTOR_VALUES), "vector role set changed")
    for name, value in SCALAR_VALUES:
        require(scalar_rows.get(name) == value, f"scalar changed: {name}")
    for name, value in VECTOR_VALUES:
        require(vector_rows.get(name) == value, f"vector changed: {name}")

    textures = {str(row.get("name")): row for row in profile.get("textures", [])}
    require(set(textures) == {row["role"] for row in LANES}, "texture roles changed")
    for lane in LANES:
        row = textures[lane["role"]]
        require(row.get("assetId") == lane["assetId"], f"asset changed: {lane['role']}")
        require(row.get("colorSpace") == lane["colorSpace"], f"color space changed: {lane['role']}")
        require(row.get("addressU") == "wrap", f"addressU changed: {lane['role']}")
        require(row.get("addressV") == "wrap", f"addressV changed: {lane['role']}")


def validate_native_oracle(repository_root: Path) -> None:
    oracle = load_json(repository_root / ORACLE_RELATIVE_PATH)
    matches = [
        row for row in oracle.get("variants", [])
        if row.get("sourceMaterialPath") == SOURCE_MATERIAL
        and row.get("variantKeySha256") == NATIVE_VARIANT_KEY_SHA256
    ]
    require(len(matches) == 1, "native Glasshole02 oracle is not singular")
    row = matches[0]
    require(
        row.get("pixelShader", {}).get("sha256") == NATIVE_DXBC_SHA256,
        "native Glasshole02 DXBC identity changed",
    )
    key = row.get("variantKey", {})
    require(
        key.get("effectiveStaticParameterSetSha256") ==
        EFFECTIVE_STATIC_SET_SHA256,
        "native Glasshole02 static set changed",
    )
    require(
        key.get("rendererType") == "SpriteParticle"
        and row.get("structuralVertexFactoryPass", {}).get(
            "actualVfPassAdmission") is False
        and row.get("admission", {}).get("productRuntime") is False,
        "oracle native runtime boundary changed",
    )


def validate_runtime_resources(repository_root: Path) -> dict[str, str]:
    resources_root = repository_root / "Client/Bin/Resources"
    result: dict[str, str] = {}
    for lane in LANES:
        path = resources_root / lane["assetId"]
        require(path.is_file(), f"runtime DDS missing: {lane['assetId']}")
        digest = file_sha256(path)
        require(digest == lane["sha256"], f"runtime DDS changed: {lane['assetId']}")
        result[lane["assetId"]] = digest
    return result


def promote_document(document: dict[str, Any]) -> tuple[dict[str, Any], bool]:
    require(document.get("effectAssetId") == TARGET_EFFECT_ID, "effect id changed")
    before = copy.deepcopy(document)
    target = exact_element(document)
    require(target.get("sourceNode") == TARGET_SOURCE_NODE, "source node changed")
    require(target.get("kind") == "particle", "carrier kind changed")
    require(
        target.get("sourceRecipe", {}).get("rendererShape") == "sprite",
        "renderer shape changed",
    )
    material = target.get("material") or {}
    require(material.get("sourceMaterialPath") == SOURCE_MATERIAL, "K child changed")
    require(
        material.get("renderProfile") == "alpha_two_sided_depth_read",
        "render profile changed",
    )
    expected = expected_execution()
    if material.get("templateId") == "effect.source_material":
        require("execution" not in material, "pre-promotion execution appeared")
        validate_pre_source_profile(material.get("sourceProfile") or {})
        material["templateId"] = "effect.standard"
        material["sourceProfile"] = {"enabled": False}
        material["execution"] = expected
    else:
        require(material.get("templateId") == "effect.standard", "template changed")
        require(
            material.get("sourceProfile") == {"enabled": False},
            "post-promotion source profile changed",
        )
        require(material.get("execution") == expected, "opcode 16 packet changed")

    after = document
    before_ids = [row.get("id") for row in before.get("elements", [])]
    after_ids = [row.get("id") for row in after.get("elements", [])]
    require(before_ids == after_ids, "element order or cardinality changed")
    for row in before.get("elements", []):
        if row.get("id") == TARGET_ELEMENT_ID:
            continue
        current = next(item for item in after["elements"] if item["id"] == row["id"])
        require(current == row, f"unowned element changed: {row['id']}")
    before_target = exact_element(before)
    after_target = exact_element(after)
    for key in set(before_target) | set(after_target):
        if key == "material":
            continue
        require(
            before_target.get(key) == after_target.get(key),
            f"non-material target field changed: {key}",
        )
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


def replace_target_material(original: str, material: dict[str, Any]) -> str:
    id_token = f'"id": {json.dumps(TARGET_ELEMENT_ID)}'
    position = original.find(id_token)
    require(position >= 0 and original.find(id_token, position + 1) < 0,
            "target text identity is missing or duplicated")
    next_id = original.find('"id": ', position + len(id_token))
    element_end = len(original) if next_id < 0 else next_id
    match = re.search(r'"material"\s*:\s*\{', original[position:element_end])
    require(match is not None, "target material text is missing")
    start = position + match.end() - 1
    end = find_balanced_object_end(original, start)
    rendered = json.dumps(material, ensure_ascii=False, separators=(", ", ": "))
    return original[:start] + rendered + original[end:]


def build_receipt(resource_hashes: dict[str, str]) -> dict[str, Any]:
    receipt = {
        "schema": "lostark.effect-glasshole02-k01-semantic-replay-receipt",
        "formatVersion": 1,
        "effectAssetId": TARGET_EFFECT_ID,
        "elementId": TARGET_ELEMENT_ID,
        "sourceNode": TARGET_SOURCE_NODE,
        "familyTuple": {
            "carrierVariantId": "carrier.cascade-sprite.source-v1",
            "materialVariantId": "material.glasshole02.sprite.k01.semantic-v1",
            "renderVariantId": "render.particle.alpha-two-sided-depth-read.mrt2-v1",
            "compositionVariantId": "composition.dimensionmaster.2050120.clip3.source-v1",
        },
        "provenance": "SOURCE_EXACT",
        "evidence": "PARTIAL",
        "runtimeExecutor": "TYPED_HLSL_SEMANTIC_REPLAY",
        # The authored packet is closed, but the checked-in runtime catalog still
        # points at the pre-promotion sealed document until the global publisher
        # transaction succeeds.  Do not claim Product admission early.
        "runtimeAdmission": "AUTHORING_ONLY",
        "productJoin": "AUTHORED_NOT_PUBLISHED",
        "userReview": "PENDING",
        "runtimeMaterial": {
            "backend": "runtimeMaterialV2",
            "opcode": RUNTIME_OPCODE,
            "packetSha256": canonical_sha256(expected_execution()),
            "sourceMaterialPath": SOURCE_MATERIAL,
            "parentMaterialPath": PARENT_MATERIAL,
            "profileId": PROFILE_ID,
            "effectiveStaticParameterSetSha256": EFFECTIVE_STATIC_SET_SHA256,
            "resourceSha256": dict(sorted(resource_hashes.items())),
        },
        "nativeOracle": {
            "variantKeySha256": NATIVE_VARIANT_KEY_SHA256,
            "pixelShaderSha256": NATIVE_DXBC_SHA256,
            "runtimeAdmission": False,
            "blockers": [
                "NATIVE_EMITTER_VERTEX_FACTORY_ABI_UNPROVEN",
                "NATIVE_SIX_SLOT_MRT_ABI_UNSUPPORTED",
                "SOURCE_SAMPLER_ABI_UNPROVEN",
            ],
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
    target_path = repository_root / TARGET_RELATIVE_PATH
    original = target_path.read_text(encoding="utf-8")
    document = json.loads(original)
    validate_native_oracle(repository_root)
    resource_hashes = validate_runtime_resources(repository_root)
    promoted, changed = promote_document(document)
    material = exact_element(promoted)["material"]
    projected = replace_target_material(original, material) if changed else original
    require(json.loads(projected) == promoted, "text projection changed semantics")
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
        refreshed = json.loads(target_path.read_text(encoding="utf-8"))
        _, still_changed = promote_document(refreshed)
        require(not still_changed, "materializer did not converge")
        require(
            receipt_path.read_text(encoding="utf-8") == receipt_text,
            "receipt write did not converge",
        )
        return projected != original or not receipt_current, receipt
    require(projected == original, "Glasshole02 K-child materialization is stale")
    require(receipt_current, "Glasshole02 K-child receipt is stale")
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
        "elementId": TARGET_ELEMENT_ID,
        "runtimeOpcode": RUNTIME_OPCODE,
        "artifactSha256": receipt["artifactSha256"],
    }, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
