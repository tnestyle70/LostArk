#!/usr/bin/env python3
"""Materialize Artist D's bounded BLACK_TIGER_STROKE typed family.

The Product document was intentionally pruned to its 56 admitted rows while
the role manifest retained twelve source-proven SpriteParticle occurrences as
BLACK_TIGER_STROKE.  This materializer reconstructs only those twelve rows from
the imported source document and its role receipt, inserts them by sourceOrder,
and attaches the class-neutral RuntimeMaterialV2 opcode 18 packet.

This is a typed source reconstruction, not native DXBC replay.  The receipt
therefore keeps native shader/DXBC fidelity PENDING and source exactness PARTIAL.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
from pathlib import Path
from typing import Any

import materialize_four_class_track_a_candidates as source_materializer


ROOT = Path(__file__).resolve().parents[2]
TARGET_PATH = ROOT / (
    "Data/Effects/Authored/effect.artist.skill.31490.unified.effect.json"
)
ROLE_MANIFEST_PATH = ROOT / (
    "Data/Effects/AuthoredCorrections/Artist/"
    "effect.artist.skill.31490.role-manifest.json"
)
ROLE_RECEIPT_PATH = ROOT / (
    "Data/Effects/AuthoredCorrections/Artist/"
    "effect.artist.skill.31490.unified.role-materialization.receipt.json"
)
SOURCE_PATH = ROOT / (
    "Data/Effects/Imported/Artist/CurrentCombat/Converted/"
    "effect.artist.skill.31490.imported.effect.json"
)
RECEIPT_PATH = ROOT / (
    "Data/Effects/AuthoredCorrections/Generated/Artist/"
    "effect.artist.skill.31490.black-tiger-stroke.materialization-receipt.v1.json"
)
RESOURCE_ROOT = ROOT / "Client/Bin/Resources"

EFFECT_ID = "effect.artist.skill.31490.unified"
SOURCE_EFFECT_ID = "effect.artist.skill.31490.imported"
ROLE = "BLACK_TIGER_STROKE"
OPCODE = 18

TIGER_ROWS = (
    (
        "authored.source-particle.763aea38ab1100ba9072dbfb",
        45,
        "FX_PC_MSR_03:export:2587@ref:4",
    ),
    (
        "authored.source-particle.e6c3ffec9fbc27024e2ce78c",
        46,
        "FX_PC_MSR_03:export:2587@ref:4",
    ),
    (
        "authored.source-particle.91392dd3a1710c9d411bfff6",
        47,
        "FX_PC_SDM_08:export:1241@ref:3",
    ),
    (
        "authored.source-particle.382ed3229ddf083cfd22ee11",
        48,
        "FX_PC_SDM_08:export:1241@ref:3",
    ),
    (
        "authored.source-particle.4f0381d175d441978f26ebfc",
        51,
        "FX_PC_SDM_08:export:1240@ref:3",
    ),
    (
        "authored.source-particle.31fa700c084ab0b11447f7c7",
        52,
        "FX_PC_SDM_08:export:1240@ref:3",
    ),
    (
        "authored.source-particle.5571970d95f97aecb889fed7",
        81,
        "FX_PC_MSR_03:export:2587@ref:4",
    ),
    (
        "authored.source-particle.87c8abd0423fcb7e9a725659",
        82,
        "FX_PC_SDM_08:export:1241@ref:3",
    ),
    (
        "authored.source-particle.ac2d4d3e467dc4442cba60c3",
        84,
        "FX_PC_SDM_08:export:1240@ref:3",
    ),
    (
        "authored.source-particle.01c398219f73706b66509e77",
        110,
        "FX_PC_MSR_03:export:2587@ref:4",
    ),
    (
        "authored.source-particle.93420edbc5815b8a01b38ef4",
        111,
        "FX_PC_SDM_08:export:1241@ref:3",
    ),
    (
        "authored.source-particle.76d0b67fe194395ce21c51ab",
        113,
        "FX_PC_SDM_08:export:1240@ref:3",
    ),
)
TIGER_IDS = tuple(row[0] for row in TIGER_ROWS)
TIGER_ID_SET = frozenset(TIGER_IDS)
EXPECTED_SOURCE_ORDER = {row[0]: row[1] for row in TIGER_ROWS}
EXPECTED_DYNAMIC_MODULE = {row[0]: row[2] for row in TIGER_ROWS}

CHILD5_PATH = "fx_m_mi_l_00.fx_mi.fx_l_pa_spritewave_01_5_ad"
CHILD6_PATH = "fx_m_mi_l_00.fx_mi.fx_l_pa_spritewave_01_6_ad"
PARENT_PATH = "fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_ad"

CHILD5_SCALARS = (
    ("disslovetex_01_panspeed_x", 0.0),
    ("disslovetex_01_panspeed_y", 0.0),
    ("disslovetex_01_tile_x", 1.0),
    ("disslovetex_01_tile_y", 1.0),
    ("dissolve_hardness", 1.0),
    ("edge_thin", 1.100000023841858),
    ("emissive_base", 1.0),
    ("emissive_core_power", 0.0),
    ("emissive_core_strength", 1.0),
    ("maintex_alpha_strength", 4.0),
    ("maintex_dynamicpan_x_velue", 0.10000000149011612),
    ("maintex_dynamicpan_y_velue", 0.0),
    ("maintex_move_x", 0.0),
    ("maintex_move_y", 0.0),
    ("maintex_panspeed_x", -0.10000000149011612),
    ("maintex_panspeed_y", 0.0),
    ("maintex_rotator", 2.0),
    ("maintex_tile_x", 1.0),
    ("maintex_tile_y", 1.0),
    ("spheremask_strength", 2.0),
    ("spheremask_strength_max", 3.0),
    ("uv_noise_velue", -0.20000000298023224),
    ("uv_noisetex_pan_x", 0.0),
    ("uv_noisetex_pan_y", 0.0),
    ("uv_noisetex_tile_x", 0.5),
    ("uv_noisetex_tile_y", 0.5),
    ("uvnoise_move_x", 0.0),
    ("uvnoise_move_y", 0.0),
)
CHILD6_SCALARS = (
    ("disslovetex_01_panspeed_x", 0.0),
    ("disslovetex_01_panspeed_y", 0.0),
    ("disslovetex_01_tile_x", 1.0),
    ("disslovetex_01_tile_y", 1.0),
    ("dissolve_hardness", 2.0),
    ("edge_thin", 1.2000000476837158),
    ("emissive_base", 0.10000000149011612),
    ("emissive_core_power", 5.0),
    ("emissive_core_strength", 25.0),
    ("maintex_alpha_strength", 15.0),
    ("maintex_move_x", 0.0),
    ("maintex_move_y", -0.10000000149011612),
    ("maintex_panspeed_x", 0.0),
    ("maintex_panspeed_y", 0.0),
    ("maintex_rotator", 1.0),
    ("maintex_tile_x", 1.0),
    ("maintex_tile_y", 1.0),
    ("uv_noise_velue", -0.800000011920929),
    ("uv_noisetex_pan_x", 0.0),
    ("uv_noisetex_pan_y", 0.0),
    ("uv_noisetex_tile_x", 1.0),
    ("uv_noisetex_tile_y", 1.0),
    ("uvnoise_move_x", 0.0),
    ("uvnoise_move_y", 0.0),
)

VARIANTS = {
    CHILD5_PATH: {
        "variantId": "child5",
        "base": "Effect/Artist/Textures/fx_m_trail_010.dds",
        "noise": "Effect/Artist/Textures/fx_c_noise_009.dds",
        "dissolve": "Effect/Artist/Textures/fx_o_symbol_14.dds",
        "scalars": CHILD5_SCALARS,
        "edgeColor": [1.0, 1.0, 1.0, 1.0],
    },
    CHILD6_PATH: {
        "variantId": "child6",
        "base": "Effect/Artist/Textures/fx_m_trail_004_cl.dds",
        "noise": "Effect/Artist/Textures/fx_bg_dustpanner_01.dds",
        "dissolve": "Effect/Artist/Textures/fx_o_symbol_14.dds",
        "scalars": CHILD6_SCALARS,
        "edgeColor": [20.0, 20.0, 20.0, 1.0],
    },
}

RESOURCE_SHA256 = {
    "Effect/Artist/Textures/fx_m_trail_010.dds": (
        "59725dffeb4593957f5dc12e848c93609858689da01de3ac84a3b282cdc50709"
    ),
    "Effect/Artist/Textures/fx_c_noise_009.dds": (
        "d9b2b59b2657fcfe333852ef2580492fcd1e7769af2d8d5096be47050cddc65e"
    ),
    "Effect/Artist/Textures/fx_o_symbol_14.dds": (
        "574dd0f33ac53559be424c0d9efee28d5d2e34174b4dc29cd90e54fc5ec2988e"
    ),
    "Effect/Artist/Textures/fx_m_trail_004_cl.dds": (
        "5681360a77c21948e854a46cd2b6a547f40f676f3ff31c73902e777f112c30b0"
    ),
    "Effect/Artist/Textures/fx_bg_dustpanner_01.dds": (
        "2eed480e6ca2baff90c08cc81b4cc6847ce61f79a2c14559d7a2984d6aa444c4"
    ),
}


class TigerMaterializationError(RuntimeError):
    """Raised when the bounded Artist D source/target contract drifts."""


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, json.JSONDecodeError) as error:
        raise TigerMaterializationError(f"cannot load {path}: {error}") from error
    if not isinstance(value, dict):
        raise TigerMaterializationError(f"JSON root must be an object: {path}")
    return value


def canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def raw_sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def extract_element_raw_blocks(
    document_text: str,
) -> tuple[str, str, dict[str, dict[str, Any]], dict[str, str]]:
    """Return the exact top-level element object bytes and outer envelope.

    JSON semantic equality protects tuning values, but this role is inserted into
    a hand-tuned Product document.  Preserve every untouched object's key order,
    number spelling, and indentation as an additional authoring invariant.
    """

    marker = document_text.find('"elements"')
    array_start = document_text.find("[", marker)
    if marker < 0 or array_start < 0:
        raise TigerMaterializationError("Artist D target has no elements array")
    decoder = json.JSONDecoder()
    position = array_start + 1
    first_start: int | None = None
    last_end: int | None = None
    parsed: dict[str, dict[str, Any]] = {}
    blocks: dict[str, str] = {}
    while True:
        while position < len(document_text) and document_text[position] in " \t\r\n,":
            position += 1
        if position >= len(document_text):
            raise TigerMaterializationError("Artist D elements array is truncated")
        if document_text[position] == "]":
            break
        try:
            value, end = decoder.raw_decode(document_text, position)
        except json.JSONDecodeError as error:
            raise TigerMaterializationError(
                f"Artist D element raw decode failed: {error}"
            ) from error
        if not isinstance(value, dict) or not isinstance(value.get("id"), str):
            raise TigerMaterializationError("Artist D raw element has no stable ID")
        element_id = value["id"]
        if element_id in blocks:
            raise TigerMaterializationError(
                f"Artist D raw element ID is duplicated: {element_id}"
            )
        first_start = position if first_start is None else first_start
        last_end = end
        parsed[element_id] = value
        blocks[element_id] = document_text[position:end]
        position = end
    if first_start is None or last_end is None:
        raise TigerMaterializationError("Artist D elements array is empty")
    return (
        document_text[:first_start],
        document_text[last_end:],
        parsed,
        blocks,
    )


def raw_block_sequence_sha256(
    element_ids: list[str], raw_blocks: dict[str, str]
) -> str:
    digest = hashlib.sha256()
    for element_id in element_ids:
        block = raw_blocks.get(element_id)
        if block is None:
            raise TigerMaterializationError(
                f"Artist D raw preservation block is missing: {element_id}"
            )
        digest.update(element_id.encode("utf-8"))
        digest.update(b"\0")
        digest.update(block.encode("utf-8"))
        digest.update(b"\0")
    return digest.hexdigest()


def serialize_preserving_untouched_elements(
    target: dict[str, Any], source_text: str | None
) -> str:
    canonical_output = source_materializer.serialized(target)
    if source_text is None:
        return canonical_output
    try:
        source_document = json.loads(source_text)
    except json.JSONDecodeError as error:
        raise TigerMaterializationError(
            f"Artist D current target text is invalid: {error}"
        ) from error
    if source_document == target:
        return source_text

    source_outer = copy.deepcopy(source_document)
    target_outer = copy.deepcopy(target)
    source_outer.pop("elements", None)
    target_outer.pop("elements", None)
    if source_outer != target_outer:
        raise TigerMaterializationError(
            "Artist D selective materializer cannot rewrite the document envelope"
        )
    head, tail, source_by_id, source_blocks = extract_element_raw_blocks(source_text)
    _, _, canonical_by_id, canonical_blocks = extract_element_raw_blocks(
        canonical_output
    )
    output_blocks: list[str] = []
    for element in target["elements"]:
        element_id = element["id"]
        if source_by_id.get(element_id) == element:
            output_blocks.append(source_blocks[element_id])
        elif canonical_by_id.get(element_id) == element:
            output_blocks.append(canonical_blocks[element_id])
        else:
            raise TigerMaterializationError(
                f"Artist D cannot serialize exact target row: {element_id}"
            )
    output = head + ",\n    ".join(output_blocks) + tail
    try:
        round_trip = json.loads(output)
    except json.JSONDecodeError as error:
        raise TigerMaterializationError(
            f"Artist D raw-preserving output is invalid: {error}"
        ) from error
    if round_trip != target:
        raise TigerMaterializationError(
            "Artist D raw-preserving output changed target semantics"
        )
    return output


def validate_role_manifest(manifest: dict[str, Any]) -> None:
    visible = manifest.get("visibleRoleAllowlists")
    locked = manifest.get("roleLockedAllowlists")
    product = manifest.get("productMaterialization")
    if (
        manifest.get("schema") != "lostark.effect-role-manifest"
        or manifest.get("effectAssetId") != EFFECT_ID
        or not isinstance(visible, dict)
        or visible.get(ROLE) != list(TIGER_IDS)
        or not isinstance(locked, dict)
        or ROLE in locked
        or product
        != {
            "mode": "SELECTIVE_VISIBLE_ROLE_SUBSET",
            "retainedElementCount": 68,
            "preservedBaselineElementCount": 56,
            "typedRole": ROLE,
            "typedMaterializationReceipt": RECEIPT_PATH.relative_to(ROOT).as_posix(),
            "fidelity": "TYPED_SOURCE_RECONSTRUCTION",
            "sourceExactness": "PARTIAL",
            "nativeShaderStatus": "PENDING",
        }
    ):
        raise TigerMaterializationError("Artist D role manifest is stale")
    baseline = manifest.get("baseline")
    if baseline != {
        "documentElementCount": 117,
        "visibleElementCount": 68,
        "roleLockedElementCount": 25,
        "suppressedByDefaultCount": 24,
    }:
        raise TigerMaterializationError("Artist D source-role denominator changed")


def validate_resources() -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    for asset_id, expected_hash in RESOURCE_SHA256.items():
        path = RESOURCE_ROOT / asset_id
        if not path.is_file():
            raise TigerMaterializationError(f"missing tiger resource: {asset_id}")
        actual_hash = raw_sha256(path)
        if actual_hash != expected_hash:
            raise TigerMaterializationError(
                f"tiger resource hash changed: {asset_id}/{actual_hash}"
            )
        rows.append({"assetId": asset_id, "sha256": actual_hash})
    return rows


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


def execution_packet(variant: dict[str, Any]) -> dict[str, Any]:
    scalars = variant["scalars"]
    scalar_count = len(scalars)
    consumed_low = (1 << scalar_count) - 1
    return {
        "enabled": True,
        "version": 1,
        "backend": "runtimeMaterialV2",
        "opcode": OPCODE,
        "passIndex": 2,
        "renderState": {
            "rasterizer": "RS_Cull_None",
            "depthStencil": "DSS_ReadOnly",
            "blend": "BS_EffectAdditive",
            "stencilReference": 0,
        },
        "textureLaneCount": 3,
        "textureMask": 7,
        "textureLanes": [
            {
                "laneId": "lane.0",
                "role": "maintex",
                "assetId": variant["base"],
                "textureRegister": 0,
                "samplerRegister": 5,
                "sourceChannel": "RGB",
                "colorSpace": "linear",
                "sampler": sampler(),
            },
            {
                "laneId": "lane.1",
                "role": "uv_noise_tex",
                "assetId": variant["noise"],
                "textureRegister": 1,
                "samplerRegister": 6,
                "sourceChannel": "RG",
                "colorSpace": "linear",
                "sampler": sampler(),
            },
            {
                "laneId": "lane.2",
                "role": "dissolve_tex_01",
                "assetId": variant["dissolve"],
                "textureRegister": 2,
                "samplerRegister": 7,
                "sourceChannel": "R",
                "colorSpace": "linear",
                "sampler": sampler(),
            },
        ],
        "dynamicConsumedMask": 15,
        "dynamicSuppressedMask": 0,
        "particleColorPolicy": 2,
        "particleColorConsumedMask": 15,
        "particleColorSuppressedMask": 0,
        "scalarCount": scalar_count,
        "vectorCount": 1,
        "inputCount": scalar_count,
        "inputConsumedMask": [consumed_low, 0],
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
            {"name": f"scalar.{index}", "packedIndex": index, "value": value}
            for index, (_, value) in enumerate(scalars)
        ],
        "vectors": [
            {"name": "vector.0", "packedIndex": 0, "value": variant["edgeColor"]}
        ],
        "artistParameters": [],
        "colors": [],
    }


def validate_source_profile(
    element_id: str,
    material: dict[str, Any],
    variant: dict[str, Any],
) -> None:
    source = material.get("sourceProfile")
    if (
        material.get("sourceMaterialPath") not in VARIANTS
        or material.get("renderProfile") != "additive_two_sided_depth_read"
        or not isinstance(source, dict)
        or source.get("enabled") is not True
        or source.get("parentMaterialPath") != PARENT_PATH
        or source.get("staticSwitches") != []
        or source.get("dynamicParameterSemantics")
        != ["uv_pan", "dissolve", "uv_pan", "uv_pan"]
    ):
        raise TigerMaterializationError(
            f"tiger source Material identity changed: {element_id}"
        )
    actual_scalars = {
        row.get("name"): row.get("value") for row in source.get("scalars", [])
    }
    if actual_scalars != dict(variant["scalars"]):
        raise TigerMaterializationError(
            f"tiger source scalar packet changed: {element_id}"
        )
    vectors = source.get("vectors")
    if vectors != [{"name": "edge_color", "value": variant["edgeColor"], "group": ""}]:
        raise TigerMaterializationError(
            f"tiger source edge color changed: {element_id}"
        )
    textures = {
        row.get("name"): row for row in source.get("textures", [])
        if isinstance(row, dict)
    }
    expected_textures = {
        "maintex": variant["base"],
        "uv_noise_tex": variant["noise"],
        "dissolve_tex_01": variant["dissolve"],
    }
    if set(textures) != set(expected_textures):
        raise TigerMaterializationError(
            f"tiger source texture roles changed: {element_id}"
        )
    for name, asset_id in expected_textures.items():
        texture = textures[name]
        if (
            texture.get("assetId") != asset_id
            or texture.get("addressU") != "wrap"
            or texture.get("addressV") != "wrap"
            or texture.get("colorSpace") != "linear"
            or texture.get("samplingEvidence") != "legacy_default"
        ):
            raise TigerMaterializationError(
                f"tiger source texture contract changed: {element_id}/{name}"
            )


def validate_dynamic_module(
    element_id: str, source_recipe: dict[str, Any]
) -> None:
    matches = [
        module
        for module in source_recipe.get("modules", [])
        if isinstance(module, dict)
        and module.get("className") == "particlemoduleparameterdynamic"
    ]
    expected_paths = [f"dynamicparams[{index}].paramvalue" for index in range(4)]
    if (
        len(matches) != 1
        or matches[0].get("stableId") != EXPECTED_DYNAMIC_MODULE[element_id]
        or [row.get("propertyPath") for row in matches[0].get("distributions", [])]
        != expected_paths
    ):
        raise TigerMaterializationError(
            f"tiger ParameterDynamic contract changed: {element_id}"
        )


def source_receipt_rows(
    receipt: dict[str, Any],
) -> dict[str, dict[str, Any]]:
    rows = {
        row.get("targetElementId"): row
        for row in receipt.get("particleRows", [])
        if isinstance(row, dict) and row.get("targetElementId") in TIGER_ID_SET
    }
    if set(rows) != TIGER_ID_SET:
        raise TigerMaterializationError("tiger source receipt denominator changed")
    for element_id, expected_order, _ in TIGER_ROWS:
        row = rows[element_id]
        if (
            row.get("sourceEffectAssetId") != SOURCE_EFFECT_ID
            or row.get("sourceOrder") != expected_order
            or not isinstance(row.get("sourceElementId"), str)
            or not isinstance(row.get("sourceEventId"), str)
            or row.get("rendererShape") != "sprite"
            or row.get("sourceBindings") != row.get("targetBindings")
        ):
            raise TigerMaterializationError(
                f"tiger source receipt row changed: {element_id}"
            )
    return rows


def materialize_tiger_rows(
    role_receipt: dict[str, Any],
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    receipt_rows = source_receipt_rows(role_receipt)
    source_by_effect, _ = source_materializer.load_source_index()
    source_document = source_by_effect.get(SOURCE_EFFECT_ID)
    if source_document is None:
        raise TigerMaterializationError("Artist D imported source is not indexed")

    staged_rows: list[dict[str, Any]] = []
    evidence_rows: list[dict[str, Any]] = []
    for element_id, source_order, _ in TIGER_ROWS:
        receipt_row = receipt_rows[element_id]
        source_element_id = receipt_row["sourceElementId"]
        source_element = source_document.elements.get(source_element_id)
        if source_element is None:
            raise TigerMaterializationError(
                f"tiger source element is missing: {source_element_id}"
            )
        if (
            canonical_sha256(source_element.get("sourceRecipe"))
            != receipt_row["sourceRecipeCanonicalSha256"]
            or canonical_sha256(source_element.get("detail"))
            != receipt_row["sourceDetailCanonicalSha256"]
        ):
            raise TigerMaterializationError(
                f"tiger source row hash changed: {element_id}"
            )
        assignment = source_materializer.SourceParticleAssignment(
            character_class="ARTIST",
            skill_id=31490,
            source_document=source_document,
            source_element_id=source_element_id,
            source_event_id=receipt_row["sourceEventId"],
            source_order=source_order,
            target_element_id=element_id,
            target_effect_id=EFFECT_ID,
            clip_timeline_offset_seconds=0.0,
        )
        staged, projection = source_materializer.materialize_source_particle(
            assignment, None, None
        )
        if (
            canonical_sha256(staged.get("sourceRecipe"))
            != receipt_row["normalizedRecipeCanonicalSha256"]
            or projection.get("sourceOrder") != source_order
            or staged.get("kind") != "particle"
            or staged.get("sourceRecipe", {}).get("enabled") is not True
            or staged.get("sourceRecipe", {}).get("rendererShape") != "sprite"
        ):
            raise TigerMaterializationError(
                f"tiger target projection changed: {element_id}"
            )
        validate_dynamic_module(element_id, staged["sourceRecipe"])
        source_material_path = staged["material"].get("sourceMaterialPath")
        variant = VARIANTS.get(source_material_path)
        if variant is None:
            raise TigerMaterializationError(
                f"tiger child Material changed: {element_id}/{source_material_path}"
            )
        validate_source_profile(element_id, staged["material"], variant)
        expected_bindings = [
            {"slotId": "base", "assetId": variant["base"]},
            {"slotId": "dissolve", "assetId": variant["dissolve"]},
            {"slotId": "noise", "assetId": variant["noise"]},
        ]
        if staged.get("resources") != expected_bindings:
            raise TigerMaterializationError(
                f"tiger resource carrier changed: {element_id}"
            )

        staged["visible"] = True
        staged["material"] = {
            "templateId": "effect.standard",
            "sourceMaterialPath": source_material_path,
            "renderProfile": "additive_two_sided_depth_read",
            "sourceProfile": {"enabled": False},
            "execution": execution_packet(variant),
        }
        staged_rows.append(staged)
        evidence_rows.append(
            {
                "targetElementId": element_id,
                "sourceOrder": source_order,
                "sourceElementId": source_element_id,
                "sourceEventId": receipt_row["sourceEventId"],
                "sourceRecipeCanonicalSha256": receipt_row[
                    "sourceRecipeCanonicalSha256"
                ],
                "normalizedRecipeCanonicalSha256": receipt_row[
                    "normalizedRecipeCanonicalSha256"
                ],
                "sourceDetailCanonicalSha256": receipt_row[
                    "sourceDetailCanonicalSha256"
                ],
                "dynamicParameterModuleStableId": EXPECTED_DYNAMIC_MODULE[element_id],
                "materialChildPath": source_material_path,
                "materialParentPath": PARENT_PATH,
                "packetVariant": variant["variantId"],
                "targetElementCanonicalSha256": canonical_sha256(staged),
            }
        )
    child_counts = {
        path: sum(row["material"]["sourceMaterialPath"] == path for row in staged_rows)
        for path in VARIANTS
    }
    if child_counts != {CHILD5_PATH: 4, CHILD6_PATH: 8}:
        raise TigerMaterializationError(f"tiger child split changed: {child_counts}")
    return staged_rows, evidence_rows


def build_projection(
    *,
    target_document: dict[str, Any] | None = None,
    role_manifest: dict[str, Any] | None = None,
    role_receipt: dict[str, Any] | None = None,
) -> tuple[dict[Path, str], dict[str, Any]]:
    target_source_text: str | None = None
    if target_document is None:
        try:
            target_source_text = TARGET_PATH.read_text(encoding="utf-8-sig")
            target = json.loads(target_source_text)
        except (OSError, json.JSONDecodeError) as error:
            raise TigerMaterializationError(
                f"cannot load {TARGET_PATH}: {error}"
            ) from error
    else:
        target = copy.deepcopy(target_document)
    manifest = copy.deepcopy(role_manifest or load_json(ROLE_MANIFEST_PATH))
    materialization_receipt = copy.deepcopy(role_receipt or load_json(ROLE_RECEIPT_PATH))
    validate_role_manifest(manifest)
    exact_resources = validate_resources()
    tiger_rows, evidence_rows = materialize_tiger_rows(materialization_receipt)

    if (
        target.get("schema") != "lostark.effect-authoring"
        or target.get("version") != 13
        or target.get("effectAssetId") != EFFECT_ID
        or not isinstance(target.get("elements"), list)
    ):
        raise TigerMaterializationError("Artist D target identity changed")
    current_elements = target["elements"]
    current_ids = [row.get("id") for row in current_elements if isinstance(row, dict)]
    if len(current_ids) != len(current_elements) or len(set(current_ids)) != len(current_ids):
        raise TigerMaterializationError("Artist D target IDs are missing/duplicated")
    present_tiger = TIGER_ID_SET.intersection(current_ids)
    if present_tiger and present_tiger != TIGER_ID_SET:
        raise TigerMaterializationError("Artist D target has a partial tiger insertion")
    baseline_elements = [
        copy.deepcopy(row) for row in current_elements if row["id"] not in TIGER_ID_SET
    ]
    if len(baseline_elements) != 56:
        raise TigerMaterializationError(
            f"Artist D preserved baseline must remain 56: {len(baseline_elements)}"
        )

    receipt_particle_rows = {
        row["targetElementId"]: row
        for row in materialization_receipt.get("particleRows", [])
        if isinstance(row, dict)
    }
    baseline_particles = [row for row in baseline_elements if row.get("kind") == "particle"]
    baseline_non_particles = [
        row for row in baseline_elements if row.get("kind") != "particle"
    ]
    if (
        len(baseline_particles) != 48
        or len(baseline_non_particles) != 8
        or any(row.get("kind") != "decal" for row in baseline_non_particles)
        or any(row["id"] not in receipt_particle_rows for row in baseline_particles)
    ):
        raise TigerMaterializationError("Artist D 48 Particle + 8 Decal baseline changed")
    all_particles = baseline_particles + tiger_rows
    all_particles.sort(key=lambda row: receipt_particle_rows[row["id"]]["sourceOrder"])
    target["elements"] = all_particles + baseline_non_particles
    if [row for row in target["elements"] if row["id"] not in TIGER_ID_SET] != baseline_elements:
        raise TigerMaterializationError("Artist D baseline rows changed during insertion")
    if len(target["elements"]) != 68:
        raise TigerMaterializationError("Artist D output cardinality must be 68")

    target_output = serialize_preserving_untouched_elements(
        target, target_source_text
    )
    _, _, output_by_id, output_raw_blocks = extract_element_raw_blocks(
        target_output
    )
    baseline_ids = [row["id"] for row in baseline_elements]
    baseline_raw_sequence_sha256 = raw_block_sequence_sha256(
        baseline_ids, output_raw_blocks
    )
    if target_source_text is not None:
        _, _, source_by_id, source_raw_blocks = extract_element_raw_blocks(
            target_source_text
        )
        for row in baseline_elements:
            element_id = row["id"]
            if (
                source_by_id.get(element_id) != row
                or output_by_id.get(element_id) != row
                or source_raw_blocks.get(element_id)
                != output_raw_blocks.get(element_id)
            ):
                raise TigerMaterializationError(
                    "Artist D untouched baseline raw object changed: " + element_id
                )

    packet_variants = []
    for path, variant in VARIANTS.items():
        packet_variants.append(
            {
                "variantId": variant["variantId"],
                "materialChildPath": path,
                "materialParentPath": PARENT_PATH,
                "scalarParameters": [
                    {"name": name, "value": value}
                    for name, value in variant["scalars"]
                ],
                "edgeColor": variant["edgeColor"],
                "textureLanes": execution_packet(variant)["textureLanes"],
                "packetCanonicalSha256": canonical_sha256(execution_packet(variant)),
            }
        )
    receipt = {
        "schema": "lostark.artist-black-tiger-stroke-materialization-receipt",
        "formatVersion": 1,
        "effectAssetId": EFFECT_ID,
        "role": ROLE,
        "runtimeMaterialV2Opcode": OPCODE,
        "provenance": {
            "fidelity": "TYPED_SOURCE_RECONSTRUCTION",
            "sourceExactness": "PARTIAL",
            "nativeShaderStatus": "PENDING",
            "nativeDxbcStatus": "PENDING",
            "nativeDxbcExecuted": False,
            "visualApproval": "PENDING_USER_FIRST_PIXEL",
        },
        "policy": {
            "selectiveSourceOrderInsertion": True,
            "preservedBaselineDeepEqual": True,
            "preservedBaselineRawObjectBytes": True,
            "exactOccurrenceAllowlistFailClosed": True,
            "sourceRecipeCarrierPreserved": True,
            "sourceStaticSwitchSetStatus": "EXACT_EMPTY",
            "nativeStaticSetRecoveryStatus": "PENDING",
            "colorContract": {
                "mainRgb": "LINEAR_BASE_RADIANCE",
                "mainR": "EXPLICIT_COVERAGE",
                "mainAlpha": "SUPPRESSED_DXT1_OPAQUE_ALPHA",
                "noiseRg": "UV_WARP",
                "dissolveR": "DISSOLVE_GATE",
                "particleColorRgb": "SOURCE_OWNED_SIGNED_MAGNITUDE_NEUTRAL_ZERO",
                "particleColorAlpha": "INDEPENDENT_LIFETIME_ENVELOPE",
                "pass": "ADDITIVE_TWO_SIDED_DEPTH_READ",
            },
        },
        "sources": {
            "roleManifest": {
                "path": ROLE_MANIFEST_PATH.relative_to(ROOT).as_posix(),
                "sha256": raw_sha256(ROLE_MANIFEST_PATH),
            },
            "roleMaterializationReceipt": {
                "path": ROLE_RECEIPT_PATH.relative_to(ROOT).as_posix(),
                "sha256": raw_sha256(ROLE_RECEIPT_PATH),
            },
            "importedDocument": {
                "path": SOURCE_PATH.relative_to(ROOT).as_posix(),
                "sha256": raw_sha256(SOURCE_PATH),
            },
            "sourceProjectionImplementation": {
                "path": Path(source_materializer.__file__).resolve().relative_to(ROOT).as_posix(),
                "sha256": raw_sha256(Path(source_materializer.__file__).resolve()),
            },
            "exactResources": exact_resources,
        },
        "counts": {
            "preservedBaselineElementCount": 56,
            "insertedSpriteParticleCount": 12,
            "child5Count": 4,
            "child6Count": 8,
            "outputElementCount": 68,
            "outputParticleCount": 60,
            "outputDecalCount": 8,
        },
        "preservedBaselineCanonicalSha256": canonical_sha256(baseline_elements),
        "preservedBaselineRawObjectSequenceSha256": (
            baseline_raw_sequence_sha256
        ),
        "packetVariants": packet_variants,
        "rows": evidence_rows,
        "outputCanonicalSha256": canonical_sha256(target),
    }
    receipt["artifactSha256"] = canonical_sha256(receipt)
    outputs = {
        TARGET_PATH: target_output,
        RECEIPT_PATH: source_materializer.serialized(receipt),
    }
    return outputs, receipt


def check_outputs(outputs: dict[Path, str]) -> None:
    stale = []
    for path, expected in outputs.items():
        try:
            actual = path.read_text(encoding="utf-8")
        except OSError:
            actual = ""
        if actual != expected:
            stale.append(path.relative_to(ROOT).as_posix())
    if stale:
        raise TigerMaterializationError(
            "Artist D BLACK_TIGER_STROKE outputs are stale: " + ", ".join(stale)
        )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--dry-run", action="store_true")
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--write", action="store_true")
    arguments = parser.parse_args()
    outputs, receipt = build_projection()
    if arguments.check:
        check_outputs(outputs)
        print(f"Artist D BLACK_TIGER_STROKE check PASS: {receipt['counts']}")
        return 0
    if arguments.write:
        source_materializer.commit_transaction(outputs)
        check_outputs(outputs)
        print(f"Artist D BLACK_TIGER_STROKE materialized: {receipt['counts']}")
        return 0
    print(f"Artist D BLACK_TIGER_STROKE dry-run PASS: {receipt['counts']}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except TigerMaterializationError as error:
        print(f"ERROR: {error}")
        raise SystemExit(1)
