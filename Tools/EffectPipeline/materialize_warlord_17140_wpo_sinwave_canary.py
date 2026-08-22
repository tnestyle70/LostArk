#!/usr/bin/env python3
"""Materialize the bounded Warlord-F WPO SinWave typed RT0 Tool cohort.

The Product document is a byte-frozen input.  The output is a separate direct
authored v13 document with two source-owned occurrences: the first is the
visual canary and the second proves packet reuse without another C++/HLSL path.
Only the two source-resolved child texture roles are admitted.  Missing parent
defaults, native DXBC/register wires, and vertex WPO remain pending evidence.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import tempfile
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
PRODUCT = ROOT / "Data/Effects/Authored/effect.warlord.skill.17140.unified.effect.json"
CANDIDATE = ROOT / (
    "Data/Effects/Authored/"
    "effect.warlord.skill.17140.wpo-sinwave-v1.unified.effect.json"
)
CATALOG = ROOT / "Data/Effects/EffectCatalog.json"
SOURCE_CONTRACT = ROOT / (
    "Data/Effects/AuthoredCorrections/Generated/"
    "FourClassCombat.source-material-contract.json"
)
RECEIPT = ROOT / (
    "Data/Effects/AuthoredCorrections/Generated/Warlord/"
    "effect.warlord.skill.17140.wpo-sinwave-v1.receipt.json"
)

PRODUCT_EFFECT_ID = "effect.warlord.skill.17140.unified"
CANDIDATE_EFFECT_ID = "effect.warlord.skill.17140.wpo-sinwave-v1.unified"
PRODUCT_BYTE_SHA256 = "52d8c29d3af8e8fdf27f32bd8882b25b56c6fab737a83a5bf87bbcc6ebec81ff"
PRODUCT_ELEMENT_COUNT = 56
OPCODE = 22
CHILD = "fx_m_mi_d_00.fx_mi.fx_d_me_worldpositionoffset_sinwave_01_04_ad"
PARENT = "fx_m_mi_d_00.fx_m.fx_d_me_worldpositionoffset_sinwave_01_ad"
PROFILE = (
    "ue3.material.fx.m.mi.d.00.fx.m.fx.d.me."
    "worldpositionoffset.sinwave.01.ad.1feb93cbb95e"
)
MESH = "Effect/Warlord/Meshes/FX_SM_00/fm_d_electric_05_vertexcolor.wmodel"
THUNDER = "Effect/Warlord/Textures/FX_TEX_04/fx_i_thunder_02_ycl.dds"
EMISSION = "Effect/Warlord/Textures/FX_TEX_02/fx_d_atypical_049.dds"

TARGETS = (
    {
        "elementId": "authored.source-particle.8c0d6ab070c1a6c83479e590",
        "role": "VISUAL_CANARY",
        "sourceNode": (
            "authored-source-particle:effect.warlord.skill.17140.unified|"
            "source:effect.warlord.skill.17140.imported|element:fx_pc_wgl_07."
            "par_s_wgl_guardianlightning_01.particlespriteemitter_13"
        ),
    },
    {
        "elementId": "authored.source-particle.59e6ffa8852fba74279b6ae9",
        "role": "DATA_ONLY_REUSE_PROOF",
        "sourceNode": (
            "authored-source-particle:effect.warlord.skill.17140.unified|"
            "source:effect.warlord.skill.17140.imported|element:fx_pc_wgl_07."
            "par_s_wgl_guardianlightning_02.particlespriteemitter_52"
        ),
    },
)

RESOURCE_SHA256 = {
    THUNDER: "a24413c124f8384a73c4a54fbfc23738f399bb311505ee8b934a38a14c5250fb",
    EMISSION: "340e0d78f603b8ee7fc7599728299f11bd627e53b011625313b155bdfd017b1b",
    MESH: "a450e157c16f8930eff38eb14724c0934c97150406a1a517e880338df1def7a8",
}

SCALARS = (
    ("alpha_power", 2.0),
    ("alpha_strength", 2.0),
    ("alpha_uv_scale_x", 2.0),
    ("alpha_uv_scale_y", 1.5),
    ("emission_power", 2.0),
    ("emission_desaturation", 1.0),
    ("emission_uv_scale_x", 4.0),
    ("emission_uv_scale_y", 1.0),
    ("emission_pan_x", 0.0),
    ("emission_pan_y", 0.0),
)

SOURCE_SCALARS = {
    "37.power": 2.0,
    "36.str": 2.0,
    "21.uvscale.x": 2.0,
    "22.uvscale.y": 1.5,
    "92.emissiion_power": 2.0,
    "91.desaturation": 1.0,
    "03.map_e_uvscale_r": 4.0,
    "04.map_e_uvscale_g": 1.0,
    "05.map_e_panning_x": 0.0,
    "06.map_e_panning_y": 0.0,
}

PENDING_TEXTURES = {
    "06.map": "fx_d_noise_009",
    "12.map_f": "fx_c_noise_002",
    "umodel_dependency": "fx_tex_02.fx_d_symbol_030_cl",
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def read_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8-sig"))
    require(isinstance(value, dict), f"JSON root is not an object: {path}")
    return value


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_bytes(value)).hexdigest()


def raw_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def pretty_bytes(value: Any) -> bytes:
    return (json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n").encode("utf-8")


def atomic_write(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary: Path | None = None
    try:
        descriptor, name = tempfile.mkstemp(
            prefix=path.name + ".", suffix=".tmp", dir=path.parent
        )
        temporary = Path(name)
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, path)
        temporary = None
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def sampler() -> dict[str, Any]:
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


def execution_packet() -> dict[str, Any]:
    lanes = (
        ("alpha_mask_21_map_c", THUNDER, "R"),
        ("emission_02_map_e", EMISSION, "RGB"),
    )
    return {
        "enabled": True,
        "version": 1,
        "backend": "runtimeMaterialV2",
        "opcode": OPCODE,
        "passIndex": 4,
        "renderState": {
            "rasterizer": "RS_Default",
            "depthStencil": "DSS_ReadOnly",
            "blend": "BS_EffectAdditive",
            "stencilReference": 0,
        },
        "textureLaneCount": 2,
        "textureMask": 3,
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
            for index, (role, asset_id, channel) in enumerate(lanes)
        ],
        "dynamicConsumedMask": 2,
        "dynamicSuppressedMask": 13,
        "particleColorPolicy": 2,
        "particleColorConsumedMask": 15,
        "particleColorSuppressedMask": 0,
        "scalarCount": len(SCALARS),
        "vectorCount": 1,
        "inputCount": len(SCALARS),
        "inputConsumedMask": [0x03FF, 0],
        "inputSuppressedMask": [0, 0],
        "vectorComponentConsumedMask": [7, 0, 0],
        "vectorComponentSuppressedMask": [8, 0, 0],
        "staticInputCount": 0,
        "staticSelectedMask": 0,
        "staticConsumedMask": 0,
        "staticSuppressedMask": 0,
        "renderInputCount": 6,
        "renderConsumedMask": 47,
        "renderSuppressedMask": 16,
        "scalars": [
            {"name": name, "packedIndex": index, "value": value}
            for index, (name, value) in enumerate(SCALARS)
        ],
        "vectors": [{
            "name": "emission_color",
            "packedIndex": 0,
            "value": [5.0, 5.0, 5.0, 1.0],
        }],
        "artistParameters": [],
        "colors": [],
    }


def validate_product(product: dict[str, Any], product_path: Path = PRODUCT) -> list[dict[str, Any]]:
    require(raw_sha256(product_path) == PRODUCT_BYTE_SHA256,
            "Warlord F Product byte identity changed; hand tuning is not frozen")
    require(product.get("schema") == "lostark.effect-authoring" and product.get("version") == 13,
            "Warlord F Product header changed")
    require(product.get("effectAssetId") == PRODUCT_EFFECT_ID,
            "Warlord F Product ID changed")
    require(len(product.get("elements", [])) == PRODUCT_ELEMENT_COUNT,
            "Warlord F Product 56-row denominator changed")
    by_id = {row.get("id"): row for row in product["elements"]}
    require(len(by_id) == PRODUCT_ELEMENT_COUNT, "Warlord F Product IDs are duplicated")
    rows: list[dict[str, Any]] = []
    for target in TARGETS:
        row = by_id.get(target["elementId"])
        require(isinstance(row, dict), f"WPO occurrence missing: {target['elementId']}")
        require(row.get("sourceNode") == target["sourceNode"],
                f"WPO source node changed: {target['elementId']}")
        require(row.get("visible") is True and row.get("kind") == "particle",
                f"WPO carrier visibility/kind changed: {target['elementId']}")
        require(row.get("sourceRecipe", {}).get("enabled") is True and
                row.get("sourceRecipe", {}).get("rendererShape") == "mesh",
                f"WPO source mesh carrier changed: {target['elementId']}")
        require(row.get("resources") == [
            {"slotId": "meshModel", "assetId": MESH},
            {"slotId": "base", "assetId": EMISSION},
            {"slotId": "noise", "assetId": THUNDER},
        ], f"WPO resource tuple changed: {target['elementId']}")
        material = row.get("material", {})
        require(material.get("templateId") == "effect.source_material" and
                material.get("sourceMaterialPath") == CHILD and
                material.get("renderProfile") == "additive_one_sided_depth_read",
                f"WPO child/render tuple changed: {target['elementId']}")
        profile = material.get("sourceProfile", {})
        require(profile.get("enabled") is True and profile.get("profileId") == PROFILE and
                profile.get("parentMaterialPath") == PARENT,
                f"WPO parent/profile tuple changed: {target['elementId']}")
        scalar_map = {item.get("name"): item.get("value") for item in profile.get("scalars", [])}
        require(all(scalar_map.get(name) == value for name, value in SOURCE_SCALARS.items()),
                f"WPO proven source scalars changed: {target['elementId']}")
        vector_map = {item.get("name"): item.get("value") for item in profile.get("vectors", [])}
        require(vector_map.get("93.emissiion_color") == [5.0, 5.0, 5.0, 1.0],
                f"WPO emission vector changed: {target['elementId']}")
        rows.append(row)
    require([row["id"] for row in rows] == [target["elementId"] for target in TARGETS],
            "WPO cohort order changed")
    return rows


def validate_source_contract(contract: dict[str, Any]) -> dict[str, Any]:
    matches = [row for row in contract.get("materialIdentities", [])
               if row.get("sourceMaterialPath") == CHILD]
    require(len(matches) == 1, "WPO source material contract is not singular")
    row = matches[0]
    require(row.get("parentMaterialPath") == PARENT and
            row.get("materialEvidenceSource") == "MATERIAL_MAP_EXACT" and
            row.get("semanticStatus") == "RECONSTRUCTED_PROFILE" and
            row.get("materialResourceDecodeStatus") == "NOT_CAPTURED",
            "WPO source evidence boundary changed")
    resolved = {(item.get("parameterName"), item.get("assetId"))
                for item in row.get("roleResolvedRuntimeBindings", [])}
    require(resolved == {("21.map_c", THUNDER), ("02.map_e", EMISSION)},
            "WPO proven two-lane role set changed")
    textures = {item.get("name"): item for item in row.get("sourceParameters", {}).get("textures", [])}
    require(textures.get("21.map_c", {}).get("assetId") == THUNDER and
            textures.get("02.map_e", {}).get("assetId") == EMISSION,
            "WPO resolved child texture identities changed")
    for name, object_path in PENDING_TEXTURES.items():
        pending = textures.get(name, {})
        require(pending.get("assetId") == "" and
                pending.get("sourceObjectPath") == object_path,
                f"WPO pending texture unexpectedly changed: {name}")
    return row


def validate_resources() -> dict[str, Any]:
    result: dict[str, Any] = {}
    for asset_id, expected in RESOURCE_SHA256.items():
        path = ROOT / "Client/Bin/Resources" / asset_id
        require(path.is_file(), f"WPO runtime resource missing: {asset_id}")
        actual = raw_sha256(path)
        require(actual == expected, f"WPO runtime resource identity changed: {asset_id}")
        result[asset_id] = {"byteSize": path.stat().st_size, "sha256": actual}
    return result


def build_candidate(product: dict[str, Any]) -> dict[str, Any]:
    source_rows = validate_product(product)
    candidate = {
        "schema": "lostark.effect-authoring",
        "version": 13,
        "effectAssetId": CANDIDATE_EFFECT_ID,
        "displayName": "PROJECT_RECONSTRUCTED | Warlord F WPO SinWave RT0",
        "particleSystem": copy.deepcopy(product["particleSystem"]),
        "elements": copy.deepcopy(source_rows),
    }
    expected = execution_packet()
    for element in candidate["elements"]:
        material = element["material"]
        material["templateId"] = "effect.standard"
        material["sourceProfile"]["enabled"] = False
        material["execution"] = copy.deepcopy(expected)
    validate_candidate(candidate, product)
    return candidate


def validate_candidate(candidate: dict[str, Any], product: dict[str, Any]) -> None:
    require(candidate.get("schema") == "lostark.effect-authoring" and
            candidate.get("version") == 13 and
            candidate.get("effectAssetId") == CANDIDATE_EFFECT_ID,
            "WPO Tool candidate header changed")
    require(candidate.get("particleSystem") == product.get("particleSystem"),
            "WPO Tool candidate particle-system tuning changed")
    expected_ids = [target["elementId"] for target in TARGETS]
    require([row.get("id") for row in candidate.get("elements", [])] == expected_ids,
            "WPO Tool candidate occurrence set/order changed")
    product_by_id = {row["id"]: row for row in product["elements"]}
    expected_execution = execution_packet()
    for row in candidate["elements"]:
        original = product_by_id[row["id"]]
        for key in set(original) | set(row):
            if key != "material":
                require(original.get(key) == row.get(key),
                        f"WPO hand tuning/carrier field changed: {row['id']}/{key}")
        old_material = original["material"]
        material = row["material"]
        require(material.get("sourceMaterialPath") == old_material.get("sourceMaterialPath") == CHILD and
                material.get("renderProfile") == old_material.get("renderProfile") ==
                    "additive_one_sided_depth_read" and
                material.get("templateId") == "effect.standard",
                f"WPO Tool candidate material identity changed: {row['id']}")
        expected_profile = copy.deepcopy(old_material["sourceProfile"])
        expected_profile["enabled"] = False
        require(material.get("sourceProfile") == expected_profile,
                f"WPO Tool candidate source evidence changed: {row['id']}")
        require(material.get("execution") == expected_execution,
                f"WPO typed packet changed: {row['id']}")
    require(candidate["elements"][0]["material"]["execution"] ==
            candidate["elements"][1]["material"]["execution"],
            "data-only occurrence no longer reuses the canary packet")


def build_catalog(catalog: dict[str, Any]) -> dict[str, Any]:
    require(catalog.get("formatVersion") == 1, "EffectCatalog version changed")
    staged = copy.deepcopy(catalog)
    rows = staged.get("effects", [])
    ids = [row.get("effectAssetId") for row in rows]
    require(ids == sorted(ids) and len(ids) == len(set(ids)),
            "EffectCatalog IDs are unsorted or duplicated")
    expected = {
        "effectAssetId": CANDIDATE_EFFECT_ID,
        "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
        "authoringPath": (
            "Effects/Authored/"
            "effect.warlord.skill.17140.wpo-sinwave-v1.unified.effect.json"
        ),
    }
    matches = [row for row in rows if row.get("effectAssetId") == CANDIDATE_EFFECT_ID]
    if matches:
        require(matches == [expected], "WPO Tool candidate catalog row changed")
    else:
        rows.append(expected)
        rows.sort(key=lambda row: row["effectAssetId"])
    return staged


def descriptor(path: Path, role: str) -> dict[str, Any]:
    require(path.is_file(), f"WPO implementation input missing: {path}")
    return {
        "path": path.relative_to(ROOT).as_posix(),
        "byteSize": path.stat().st_size,
        "sha256": raw_sha256(path),
        "role": role,
    }


def build_receipt(candidate: dict[str, Any], catalog: dict[str, Any],
                  source_row: dict[str, Any], resources: dict[str, Any]) -> dict[str, Any]:
    implementation = [
        descriptor(ROOT / "Client/Bin/ShaderFiles/Shader_EffectUe3MaterialFamilies.hlsli",
                   "class-neutral opcode 22 two-proven-lane RT0 equation"),
        descriptor(ROOT / "Client/Bin/ShaderFiles/Shader_VtxEffectMeshPreview.hlsl",
                   "typed mesh carrier dispatch"),
        descriptor(ROOT / "Client/Private/Effect_DocumentRenderer.cpp",
                   "exact child/parent/carrier/occurrence admission and fail-close"),
    ]
    receipt: dict[str, Any] = {
        "schema": "lostark.effect-warlord-wpo-sinwave-v1-tool-canary-receipt",
        "formatVersion": 1,
        "identity": {
            "characterClass": "WARLORD",
            "skillId": 17140,
            "slot": "F",
            "productEffectAssetId": PRODUCT_EFFECT_ID,
            "candidateEffectAssetId": CANDIDATE_EFFECT_ID,
            "occurrences": [dict(target) for target in TARGETS],
        },
        "provenance": "PROJECT_RECONSTRUCTED",
        "fidelity": "TYPED_RT0_BASE_ONLY",
        "productFreeze": {
            "elementCount": PRODUCT_ELEMENT_COUNT,
            "byteSha256": PRODUCT_BYTE_SHA256,
            "mutated": False,
            "bulkSourceRestore": False,
        },
        "family": {
            "childMaterialPath": CHILD,
            "parentMaterialPath": PARENT,
            "carrier": MESH,
            "renderProfile": "additive_one_sided_depth_read",
            "runtimeBackend": "runtimeMaterialV2",
            "runtimeOpcode": OPCODE,
            "provenRuntimeBindings": [
                {"sourceParameter": "21.map_c", "role": "alpha_mask", "assetId": THUNDER,
                 "channel": "R", "runtimeLane": 0},
                {"sourceParameter": "02.map_e", "role": "emission", "assetId": EMISSION,
                 "channel": "RGB", "runtimeLane": 1},
            ],
            "pendingEvidence": [
                {"sourceParameter": name, "sourceObjectPath": object_path,
                 "status": "PENDING_EVIDENCE"}
                for name, object_path in PENDING_TEXTURES.items()
            ] + [
                {"subject": "SOURCE_DXBC_EQUATION_AND_NATIVE_REGISTER_WIRE",
                 "status": "PENDING_EVIDENCE"},
                {"subject": "SOURCE_WPO_VERTEX_EQUATION_AND_VF_PASS",
                 "status": "PENDING_EVIDENCE"},
                {"subject": "SOURCE_EXACT_SAMPLER_CDO",
                 "status": "PENDING_EVIDENCE"},
            ],
        },
        "sourceEvidence": {
            "materialEvidenceSource": source_row["materialEvidenceSource"],
            "semanticStatus": source_row["semanticStatus"],
            "materialResourceDecodeStatus": source_row["materialResourceDecodeStatus"],
            "sourceContractSha256": raw_sha256(SOURCE_CONTRACT),
        },
        "resources": resources,
        "packet": {
            "sha256": canonical_sha256(execution_packet()),
            "canaryAndReusePacketsIdentical": True,
            "secondOccurrenceCppChanges": 0,
            "secondOccurrenceHlslChanges": 0,
        },
        "disposition": {
            "carrierDisposition": "USER_REVIEW_PENDING",
            "candidateAction": "ADD_OR_REPLACE_PENDING",
            "terminalKeepReplaceAddRetireAssigned": False,
        },
        "admission": {
            "toolCandidate": True,
            "defaultProduct": False,
            "productCue": False,
            "nativeParity": False,
            "visual": False,
            "fallbackOnTypedFailure": False,
        },
        "artifacts": {
            "candidateCanonicalSha256": canonical_sha256(candidate),
            "candidateByteSha256": hashlib.sha256(pretty_bytes(candidate)).hexdigest(),
            "catalogCanonicalSha256": canonical_sha256(catalog),
            "implementation": implementation,
        },
    }
    receipt["receiptSha256"] = canonical_sha256(receipt)
    return receipt


def run(mode: str) -> bool:
    product_before = PRODUCT.read_bytes()
    require(hashlib.sha256(product_before).hexdigest() == PRODUCT_BYTE_SHA256,
            "Warlord F Product bytes changed before materialization")
    product = read_json(PRODUCT)
    source_row = validate_source_contract(read_json(SOURCE_CONTRACT))
    resources = validate_resources()
    candidate = build_candidate(product)
    catalog = build_catalog(read_json(CATALOG))
    receipt = build_receipt(candidate, catalog, source_row, resources)
    outputs = {
        CANDIDATE: pretty_bytes(candidate),
        CATALOG: pretty_bytes(catalog),
        RECEIPT: pretty_bytes(receipt),
    }
    stale = [path for path, payload in outputs.items()
             if not path.is_file() or path.read_bytes() != payload]
    if mode == "write":
        for path in stale:
            atomic_write(path, outputs[path])
    else:
        require(not stale, "WPO Tool canary materialization is stale: " +
                ", ".join(path.relative_to(ROOT).as_posix() for path in stale))
    require(PRODUCT.read_bytes() == product_before,
            "Warlord F Product bytes changed during materialization")
    return bool(stale)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    arguments = parser.parse_args()
    require(not (arguments.write and arguments.check), "choose one mode")
    mode = "write" if arguments.write else "check"
    changed = run(mode)
    print(
        f"Warlord F WPO SinWave Tool cohort {'written' if mode == 'write' else 'current'}; "
        f"changed={str(changed).lower()} productRows={PRODUCT_ELEMENT_COUNT} candidateRows=2 opcode={OPCODE}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
