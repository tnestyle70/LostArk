#!/usr/bin/env python3
"""Materialize the complete strict-mapped four-class Track A source surface.

The 101 authored documents are occurrence targets, not a hand-picked Element
denominator.  Every imported Particle that has an exact source event, stage,
and clip join is projected once with a stable source-occurrence ID.  Its Detail,
recipe, material identity, DDS, and WModel start from the imported source.  A
Base alias is admitted only when an approximation receipt proves that the alias
comes from the same source Element/group.  Legacy baked Detail is never layered
over the source recipe.

Unsupported recipes, unresolved material identities, and rows without a safe
Base stay in the document as durable hidden fail-closed evidence.  Existing
artist fields survive only after the stable occurrence ID has already been
materialized; this prevents old approximate carriers and wrong clip joins from
being mistaken for authored source-occurrence edits.  The script never mutates
EffectCatalog or animation events.
"""

from __future__ import annotations

import argparse
import base64
import copy
import hashlib
import json
import math
import os
import re
import struct
import sys
import tempfile
from collections import Counter
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import Any

from materialize_artist_31470_portable_particle_carriers import (
    MaterializeError as PortableRecipeError,
    SOURCE_ONLY_DISTRIBUTION_FIELDS,
    SOURCE_ONLY_RECIPE_FIELDS,
    normalized_module_class,
    portable_recipe,
    source_literal_value,
)

LEVEL_PLACEMENT_EXTRACTOR_ROOT = (
    Path(__file__).resolve().parents[1] / "LevelPlacementExtractor"
)
if str(LEVEL_PLACEMENT_EXTRACTOR_ROOT) not in sys.path:
    sys.path.insert(0, str(LEVEL_PLACEMENT_EXTRACTOR_ROOT))

from build_action_cue_recipe import event_matches_notify, select_stage
from extract_action_effect_notifies import extract_action_document


ROOT = Path(__file__).resolve().parents[2]
BATCH_PATH = ROOT / (
    "Data/Effects/AuthoredCorrections/Generated/"
    "FourClassCombat.track-a-authored-import-batch.json"
)
RECEIPT_PATH = ROOT / (
    "Data/Effects/AuthoredCorrections/Generated/"
    "FourClassCombat.track-a-restoration-receipt.json"
)
TARGETED_RECEIPT_PATH = ROOT / (
    "Data/Effects/AuthoredCorrections/Generated/"
    "FourClassCombat.track-a-targeted-current-combat-receipt.json"
)
RESOURCE_ROOT = ROOT / "Client/Bin/Resources"
APPROXIMATION_RECEIPT_ROOT = ROOT / "Data/Effects/AuthoredCorrections/Generated"
MATERIAL_EXECUTION_LANE_PREFIX = "materialExecutionLane:"
SOURCE_MATERIAL_TEXTURE_PREFIX = "sourceMaterialTexture:"
AUTHORING_OVERRIDE_DROP_REASONS = (
    "RESOURCE_SLOT_VANISHED",
    "SCALAR_PARAMETER_VANISHED",
    "COLOR_PARAMETER_VANISHED",
)
TARGETED_CURRENT_COMBAT_LANE = "TARGETED_CURRENT_COMBAT"

CLASS_MANIFESTS = {
    "ARTIST": ROOT
    / "Data/Effects/Imported/Artist/Artist.combat-source-stage-manifest.json",
    "DIMENSIONMASTER": ROOT
    / (
        "Data/Effects/Imported/DimensionMaster/"
        "DimensionMaster.combat-source-stage-manifest.json"
    ),
    "LANCE_MASTER": ROOT
    / (
        "Data/Effects/Imported/LanceMaster/"
        "LanceMaster.combat-source-stage-manifest.json"
    ),
    "WARLORD": ROOT
    / "Data/Effects/Imported/Warlord/Warlord.combat-source-stage-manifest.json",
}
CLASS_RESOURCE_MANIFESTS = {
    "ARTIST": ROOT
    / "Data/Effects/Imported/Artist/Artist.resource-source-manifest.json",
    "DIMENSIONMASTER": ROOT
    / (
        "Data/Effects/Imported/DimensionMaster/"
        "DimensionMaster.resource-source-manifest.json"
    ),
    "LANCE_MASTER": ROOT
    / (
        "Data/Effects/Imported/LanceMaster/"
        "LanceMaster.resource-source-manifest.json"
    ),
}
DM_A_CANONICAL_V12_PATH = ROOT / (
    "Data/Effects/Authored/effect.dimensionmaster.skill.2050210.effect.json"
)
DM_A_CANONICAL_V12_RAW_SHA256 = (
    "35371322b17dee5bc90783ecb1da03822c0023f4a6a9b1c4187dae428f36822b"
)
DM_A_TEXTURE_SAMPLING_EVIDENCE_PATH = ROOT / (
    "Data/Effects/Imported/DimensionMaster/ActionSource/"
    "DimensionMaster.texture-sampling-evidence.json"
)
DM_A_CANONICAL_SOURCE_EFFECT_ID = (
    "effect.dimensionmaster.skill.2050210.imported"
)
DM_A_CANONICAL_EFFECT_ID = "effect.dimensionmaster.skill.2050210"
DM_A_GOLDEN_SOURCE_ELEMENT_BASE_IDS = frozenset(
    {
        "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1."
        "particlespriteemitter_2",
        "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1."
        "particlespriteemitter_14",
        "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1."
        "particlespriteemitter_15",
        "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1."
        "particlespriteemitter_20",
        "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1."
        "particlespriteemitter_3",
        "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1."
        "particlespriteemitter_9",
    }
)
DM_A_GOLDEN_EVENT_SUFFIXES = (
    "",
    ".event_source-event-030",
    ".event_source-event-045",
    ".event_source-event-060",
)
DM_A_CANONICAL_ALIAS_PACKAGE_SHA256 = {
    "ygi3sorgm3i1tgha5bmj8o5cz.upk": (
        "7af83c5dc7bf9dc228fddf200b3b7fcda6a75c3a378cbc682c647dc22b909ce9"
    ),
    "ygi3sorgm3i10gha5bmj815cz.upk": (
        "7c312729211f44aa7981a4a14c5054bd928de8a32819ec265eb43b47985e839c"
    ),
}
CANONICAL_V12_PACKAGE_QUALIFIED_ASSET_ID = (
    "CANONICAL_V12_PACKAGE_QUALIFIED_ASSET_ID"
)
DM_CANONICAL_TEXTURE_ASSET_ID = re.compile(
    r"^Effect/DimensionMaster/Textures/"
    r"(?P<logical_package>[^/]+)/(?P<object_name>[^/]+)\.dds$",
    re.IGNORECASE,
)
CLASS_ORDER = {
    "ARTIST": 0,
    "DIMENSIONMASTER": 1,
    "LANCE_MASTER": 2,
    "WARLORD": 3,
}
SOURCE_MATERIAL_CONTRACT_PATHS = (
    ROOT
    / "Data/Effects/Imported/Artist/Converted/skill.31000.source-material-contract.json",
    ROOT
    / (
        "Data/Effects/Imported/DimensionMaster/ActionSource/"
        "DimensionMaster.A.source-material-contract.json"
    ),
    ROOT
    / "Data/Effects/Imported/LanceMaster/Converted/skill.34010.source-material-contract.json",
    ROOT
    / (
        "Data/Effects/AuthoredCorrections/Generated/"
        "FourClassCombat.source-material-contract.json"
    ),
)
EXTERNAL_EFFECT_EXTRACTION_ROOT = ROOT.parent / (
    "Resource_LostArk/05_Reports/EffectExtraction"
)
ANIMATION_TRAIL_GRAPH_SOURCES = {
    "LANCE_MASTER": {
        "path": EXTERNAL_EFFECT_EXTRACTION_ROOT
        / "LANCEMASTER/all_bound_skills/particle_graphs/FX_PC_FLM_01.particle-graph.json",
        "sha256": "e41f5aed862bc798ed95225b4a0f39808e7b6eca2b89e552dfe6972078fb7482",
    },
    "WARLORD": {
        "path": EXTERNAL_EFFECT_EXTRACTION_ROOT
        / "WARLORD/all_core_packages/particle_graphs/FX_BS_01.particle-graph.json",
        "sha256": "e6342d64246c7365192958c5f602bb75b95d02f2872b60edacb45895bbf7d255",
    },
}
ANIMATION_TRAIL_AUTHORING_TEMPLATE_PATH = ROOT / (
    "Data/Effects/Authored/"
    "effect.lancemaster.skill.34010.ba1.restoration-candidate.effect.json"
)
ANIMATION_TRAIL_AUTHORING_TEMPLATE_RAW_SHA256 = (
    "397ab6724e5e00f73c3747c3cf3a1bafb1b9ff5a913ad43b1b7c6dc4671ee2b9"
)
ANIMATION_TRAIL_MATERIAL_SOURCES = {
    "fx_m_mi_m_00.fx_mi.fx_m_pa_ribbonmaster_03_3_tr": {
        "path": EXTERNAL_EFFECT_EXTRACTION_ROOT
        / "DIMENSIONMASTER/materials/zhj4tc4pck4pc4j22hixeyuxeu.materials.json",
        "sha256": "821148cbede5249ede6e4c9625cf47285fd656fd3f6cba86e70912d7c6ad8b48",
        "physicalPackage": "ZHJ4TC4PCK4PC4J22HIXEYUXEU.upk",
        "materialPath": "fx_mi.fx_m_pa_ribbonmaster_03_3_tr",
        "renderProfile": "alpha_two_sided_depth_read",
        "resources": (
            ("base", "Effect/LanceMaster/Textures/fx_h_atypical_01_1.dds"),
            ("mask", "Effect/LanceMaster/Textures/fx_m_trail_006.dds"),
            ("emissive", "Effect/LanceMaster/Textures/fx_h_atypical_01_1.dds"),
            ("dissolve", "Effect/LanceMaster/Textures/fx_k_caustictile_01.dds"),
            ("noise", "Effect/LanceMaster/Textures/fx_d_noise_030.dds"),
        ),
        "sourceTextures": (
            ("alpha_texture", "fx_tex_05.fx_m_trail_006"),
            ("dissolve_tex_01", "fx_tex_05.fx_k_caustictile_01"),
            ("emissive_tex_01", "fx_tex_04.fx_h_atypical_01_1"),
            ("1_uv_noise_texture", "fx_tex_02.fx_d_noise_030"),
            ("uv_noise_texture_emissive", "fx_tex_02.fx_d_noise_030"),
        ),
        "baseDecision": "SAME_MATERIAL_PREVIEW_ALIAS_NOT_SOURCE_GRAPH_EXACT",
    },
    "fx_m_mi_05.fx_m.fx_a_tl_trail_02_ad": {
        "path": EXTERNAL_EFFECT_EXTRACTION_ROOT
        / "DIMENSIONMASTER/materials/ygi3sb3obj3o10gump6qmp815.materials.json",
        "sha256": "df3ed05958db33b260e590ca6e266cf025ebae67bf0574f59869ce32ab01ea49",
        "physicalPackage": "YGI3SB3OBJ3O10GUMP6QMP815.upk",
        "materialPath": "fx_m.fx_a_tl_trail_02_ad",
        "renderProfile": "additive_two_sided_depth_read",
        "resources": (
            ("base", "Effect/Warlord/Textures/FX_TEX_00/fx_a_blankwhite_01.dds"),
        ),
        "sourceTextures": (),
        "baseDecision": "COMPILER_OWNED_NEUTRAL_CARRIER_FOR_EXACT_TEXTURELESS_ADDITIVE",
    },
    "fx_m_mi_00.fx_mi.fx_d_pa_atta_02_16_tr": {
        "path": EXTERNAL_EFFECT_EXTRACTION_ROOT
        / "DIMENSIONMASTER/materials/ygi3sb3obj3o11gump6qmp885.materials.json",
        "sha256": "1852b140d3182c38f5377c420d1dd83e6eb3063fec8e0cc5b6a06581dee085f2",
        "physicalPackage": "YGI3SB3OBJ3O11GUMP6QMP885.upk",
        "materialPath": "fx_mi.fx_d_pa_atta_02_16_tr",
        "renderProfile": "alpha_two_sided_depth_read",
        "resources": (
            ("base", "Effect/Warlord/Textures/FX_TEX_00/fx_a_trail_011.dds"),
            ("emissive", "Effect/Warlord/Textures/FX_TEX_00/fx_a_trail_011.dds"),
        ),
        "sourceTextures": (("emissive_tex", "fx_tex_00.fx_a_trail_011"),),
        "baseDecision": "EXACT_EMISSIVE_TEXTURE_PREVIEW_ALIAS",
    },
}
ANIMATION_TRAIL_SOURCE_SPECS = (
    {
        "characterClass": "LANCE_MASTER",
        "skillId": 34010,
        "receiptPath": ROOT
        / "Data/Effects/Imported/LanceMaster/skill.34010.source-receipt.json",
        "eventIds": (
            "source-event-001",
            "source-event-008",
            "source-event-013",
            "source-event-021",
        ),
        "sourceAssetEmitters": {
            "FX_PC_FLM_01.Par_M_FLM_Trail_03": (
                "par_m_flm_trail_03.particlespriteemitter_1",
            ),
            "FX_PC_FLM_01.Par_M_FLM_Trail_03_2": (
                "par_m_flm_trail_03_2.particlespriteemitter_1",
            ),
        },
        "runtimeAnchorSlotId": "WP_FLM_1_Battle",
        "runtimeBoneName": "b_weapon_rhand",
    },
    {
        "characterClass": "WARLORD",
        "skillId": 17040,
        "receiptPath": ROOT
        / "Data/Effects/Imported/Warlord/CurrentCombat/skill.17040.source-receipt.json",
        "eventIds": ("source-event-001",),
        "sourceAssetEmitters": {
            "FX_BS_01.Trail.Par_A_Trail_007": (
                "trail.par_a_trail_007.particlespriteemitter_1",
            ),
        },
        "runtimeAnchorSlotId": "b_weapon_rhand",
        "runtimeBoneName": "b_weapon_rhand",
    },
    {
        "characterClass": "WARLORD",
        "skillId": 17080,
        "receiptPath": ROOT
        / "Data/Effects/Imported/Warlord/CurrentCombat/skill.17080.source-receipt.json",
        "eventIds": (
            "source-event-011",
            "source-event-012",
            "source-event-013",
        ),
        "sourceAssetEmitters": {
            "FX_BS_01.Trail.Par_O_WGL_DUFTrail_01_01": (
                "trail.par_o_wgl_duftrail_01_01.particlespriteemitter_1",
                "trail.par_o_wgl_duftrail_01_01.particlespriteemitter_4",
            ),
        },
        "runtimeAnchorSlotId": "b_weapon_rhand",
        "runtimeBoneName": "b_weapon_rhand",
    },
)
DIMENSION_SUMMON_MODEL_CUE_TARGET = (
    "effect.dimensionmaster.skill.2050500.unified"
)
DIMENSION_SUMMON_MODEL_CUE_ID = "dimension_summon"
DIMENSION_SUMMON_MODEL_ASSET_ID = (
    "Character/DimensionMaster/DimensionMaster_DimensionSummon.wmodel"
)
DIMENSION_SUMMON_CLIP_NAME = "sk_swp_dms_00_sk_sk_dimensionprison"
DIMENSION_SUMMON_MASK_CUTOFF = 0.333
DIMENSION_SUMMON_DIFFUSE_ALPHA_CONTRACT = (
    {
        "section": 1,
        "assetId": (
            "Character/DimensionMaster/Textures/sk_swp_dms_00_01_d.dds"
        ),
        "sha256": "bebd752c5769387a577554de1def08703c5d4cb57703d8c8cf7a2766ccf913ca",
        "fourCC": "DXT1",
        "belowCutoffPixelCount": 0,
        "zeroAlphaPixelCount": 0,
        "fullAlphaPixelCount": 1_048_576,
    },
    {
        "section": 2,
        "assetId": (
            "Character/DimensionMaster/Textures/sk_swp_dms_00_02_d.dds"
        ),
        "sha256": "ddcf9e0fa60ec55e978c308d62a0e7ff81ac4c407241a6181dd8e0e4833ca344",
        "fourCC": "DXT1",
        "belowCutoffPixelCount": 0,
        "zeroAlphaPixelCount": 0,
        "fullAlphaPixelCount": 1_048_576,
    },
    {
        "section": 3,
        "assetId": (
            "Character/DimensionMaster/Textures/sk_swp_dms_00_03_d.dds"
        ),
        "sha256": "47b7187f4f46098a71d8bc6741d6801c8e4dd1fcc50546e1ee0e709d93e005cc",
        "fourCC": "DXT1",
        "belowCutoffPixelCount": 0,
        "zeroAlphaPixelCount": 0,
        "fullAlphaPixelCount": 1_048_576,
    },
    {
        "section": 4,
        "assetId": (
            "Character/DimensionMaster/Textures/sk_swp_dms_00_04_d.dds"
        ),
        "sha256": "e308552ac2cbc7eae93008171c1592de1e7893bcb0c28c12d5444b7319c61d7a",
        "fourCC": "DXT5",
        "belowCutoffPixelCount": 643_895,
        "zeroAlphaPixelCount": 615_626,
        "fullAlphaPixelCount": 369_761,
    },
)

EXPLICIT_SOURCE_NODE = re.compile(
    r"^.+\|source:(?P<effect>effect\.[^|]+)\|element:(?P<element>[^|]+)$"
)
ELEMENT_ONLY_SOURCE_NODE = re.compile(r"^.+\|source:(?P<element>[^|]+)$")

EXPECTED_TARGET_COUNT = 101
EXPECTED_SOURCE_PARTICLE_CORPUS_COUNT = 4846
EXPECTED_STRICT_MAPPED_PARTICLE_COUNT = 4687
EXPECTED_EXCLUDED_PARTICLE_COUNT = 159
EXPECTED_DRAWABLE_DECISION_CORPUS_COUNT = 3443
EXPECTED_CURRENT_EXCLUDED_PARTICLE_COUNT = 9
EXPECTED_CURRENT_RETARGETED_PARTICLE_COUNT = 2
EXPECTED_SOURCE_ANIMATION_TRAIL_NOTIFY_COUNT = 8
EXPECTED_SOURCE_ANIMATION_TRAIL_ELEMENT_COUNT = 11
EXPECTED_CHARACTER_GHOST_CUE_COUNT = 72
EXPECTED_CHARACTER_GHOST_TARGET_COUNT = 29
EXPECTED_CHARACTER_GHOST_CUE_COUNT_BY_CLASS = {
    "ARTIST": 1,
    "DIMENSIONMASTER": 41,
    "LANCE_MASTER": 24,
    "WARLORD": 6,
}
EXPECTED_PLACEHOLDER_TRAIL_COUNT = 0
EXPECTED_SOURCE_DECAL_COUNT = 79
EXPECTED_SOURCE_DECAL_BASE_COUNT = 46
EXPECTED_SOURCE_DECAL_MISSING_BASE_COUNT = 33
EXPECTED_SOURCE_DECAL_SOURCE_RESOURCE_BINDING_COUNT = 149
EXPECTED_OUTPUT_ELEMENT_COUNT = 4777
EXPECTED_DRAWABLE_ADMITTED_COUNT = 2631
EXPECTED_PORTABLE_FAIL_CLOSED_COUNT = 1857
EXPECTED_DM_A_GOLDEN_ELEMENT_COUNT = 24
EXPECTED_DM_A_GOLDEN_ADMITTED_COUNT = 24
EXPECTED_DM_A_GOLDEN_FAIL_CLOSED_COUNT = 0
EXPECTED_CANONICAL_V12_PACKAGE_QUALIFIED_ASSET_COUNT = 20
EXPECTED_CANONICAL_V12_PACKAGE_QUALIFIED_ELEMENT_COUNT = 20
EXPECTED_NON_EXACT_NAMED_TEXTURE_ALIAS_COUNT = 1624
EXPECTED_NON_EXACT_NAMED_TEXTURE_ALIAS_ELEMENT_COUNT = 823
EXPECTED_CANONICAL_MATERIAL_JOINED_COUNT = 784
EXPECTED_SHARED_MATERIAL_CONTRACT_MATCHED_COUNT = 4646
EXPECTED_SHARED_MATERIAL_CONTRACT_RESOLVED_COUNT = 4646
EXPECTED_SHARED_MATERIAL_PROFILE_JOINED_COUNT = 3739
EXPECTED_SHARED_MATERIAL_OCCURRENCE_PROMOTED_COUNT = 294
EXPECTED_SHARED_MATERIAL_CANONICAL_FALLBACK_REPLACED_COUNT = 38
EXPECTED_SOURCE_PROFILE_ENABLED_COUNT = 4571
EXPECTED_SOURCE_PROFILE_READY_COUNT = 3540
EXPECTED_SOURCE_PROFILE_BLOCKED_COUNT = 1031
EXPECTED_NAMED_TEXTURE_COUNT = 15360
EXPECTED_EXACT_NAMED_TEXTURE_COUNT = 7225
EXPECTED_REBASED_NAMED_TEXTURE_COUNT = 2127
EXPECTED_MANIFEST_NAMED_TEXTURE_COUNT = 2050
EXPECTED_UNRESOLVED_NAMED_TEXTURE_COUNT = 6085
EXPECTED_DM_BASELESS_GROUPED_SPRITE_ADMITTED_COUNT = 8
EXACT_SOURCE_MATERIAL_STATUS = "RESOLVED_EXACT_SOURCE_PACKAGE"
FAIL_CLOSED_EXECUTION = {"enabled": False, "failClosed": True}
AUTHORING_APPROXIMATE_EXECUTION = {
    "enabled": False,
    "failClosed": True,
    "authoringApproximate": True,
}
# Reasons that preserve an otherwise executable source profile and resource
# contract while weakening only the source-object identity proof.  Profile or
# resource-contract blockers are not approximate previews: the native renderer
# would suppress them or fail staging, so they remain hard fail-closed until a
# real evaluator/resource contract exists.
AUTHORING_APPROXIMATE_REASONS = frozenset({
    "NON_EXACT_NAMED_TEXTURE_ALIAS",
    "SOURCE_DYNAMIC_PARAMETER_ARITHMETIC_UNAVAILABLE",
})
SOURCE_DYNAMIC_PARAMETER_ARITHMETIC_UNAVAILABLE = (
    "SOURCE_DYNAMIC_PARAMETER_ARITHMETIC_UNAVAILABLE"
)
# Direct extraction of this exact physical parent proves that 159/260 cooked
# expression entries and 55 input edges are absent.  Both spellings below are
# emitted by existing exact source contracts for the same parent object; the
# physical package prevents a same-name parent in another package from being
# treated as evidence-equivalent.
COOKED_PARTIAL_DYNAMIC_PARENT_IDENTITIES = frozenset({
    (
        "fx_m.fx_m_pa_spritewave_01_tr",
        "zhj4tc4pck4pc4j22hixeyuxeu.upk",
    ),
    (
        "fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr",
        "zhj4tc4pck4pc4j22hixeyuxeu.upk",
    ),
})


def is_authoring_execution_target(element: dict[str, Any]) -> bool:
    """Validate the fail-closed/approximate state and return runtime intent.

    A hard fail-closed row may remain hidden as durable source evidence.  An
    authoring-approximate row is an execution target despite retaining the
    failClosed marker, so downstream material/module/draw/event validators
    must never skip it.
    """
    visible = element.get("visible")
    if not isinstance(visible, bool):
        raise RestorationError(
            f"authored Element Visible state is invalid: {element.get('id', '')}"
        )
    material = element.get("material")
    if not isinstance(material, dict):
        raise RestorationError(
            f"authored Element Material is invalid: {element.get('id', '')}"
        )
    execution = material.get("execution")
    if execution is None:
        return True
    if not isinstance(execution, dict):
        raise RestorationError(
            f"authored Material execution is invalid: {element.get('id', '')}"
        )
    fail_closed_value = execution.get("failClosed", False)
    approximate_value = execution.get("authoringApproximate", False)
    enabled_value = execution.get("enabled", False)
    if not isinstance(fail_closed_value, bool) or not isinstance(
        enabled_value, bool
    ):
        raise RestorationError(
            f"authored Material execution flags are invalid: {element.get('id', '')}"
        )
    if "authoringApproximate" in execution and approximate_value is not True:
        raise RestorationError(
            "authoringApproximate must be true when present: "
            f"{element.get('id', '')}"
        )
    if approximate_value and (not fail_closed_value or enabled_value):
        raise RestorationError(
            "authoringApproximate requires disabled fail-closed execution: "
            f"{element.get('id', '')}"
        )
    execution_target = not fail_closed_value or bool(approximate_value)
    if visible and not execution_target:
        raise RestorationError(
            "hard fail-closed authored Element cannot be activated: "
            f"{element.get('id', '')}"
        )
    return execution_target


def authoring_approximate_admitted(
    fail_closed_reasons: list[str],
    shape: str,
    resources: list[dict[str, Any]],
    source_profile_enabled: bool,
    runtime_shader_profile_id: str,
) -> bool:
    """Return whether a fail-closed carrier may still preview for authoring."""
    if (
        not fail_closed_reasons
        or not source_profile_enabled
        or runtime_shader_profile_id != "effect.ue3.grouped-translucent.v1"
    ):
        return False
    if any(
        reason not in AUTHORING_APPROXIMATE_REASONS
        for reason in fail_closed_reasons
    ):
        return False
    bindings = {
        str(row.get("slotId") or ""): str(row.get("assetId") or "")
        for row in resources
        if isinstance(row, dict)
    }
    textures = [
        asset for slot, asset in bindings.items()
        if slot != "meshModel" and asset
    ]
    if not textures:
        return False
    if shape == "mesh":
        return bool(bindings.get("meshModel"))
    return True
FALLBACK_BLOCKED_PROFILE = "effect.ue3.fallback-blocked.v1"
GROUPED_TRANSLUCENT_PROFILE = "effect.ue3.grouped-translucent.v1"
LINEARFLOW_PROFILE = "effect.ue3.linearflow-02.v1"
BLACKLINE_PROFILE = "effect.ue3.blackline-aura.v1"
LOCAL_CRACK_PROFILE = "effect.ue3.local-crack.v1"
LEGACY_MISSILETRAIL_PROFILE = "effect.ue3.missiletrail-01.v1"
MISSILETRAIL_PROFILE = "effect.ue3.missiletrail-two-emissive.v1"
WATERTRAIL_PROFILE = "effect.ue3.watertrail-01.v1"
LINEARFLOW_NAMED_TEXTURES = {
    "diff_tex",
    "diff_noise_tex",
    "a_mask_tex",
    "a_noise_01_tex",
    "b_mask_tex",
    "b_noise_01_tex",
    "dissolve_tex",
}
BLACKLINE_NAMED_TEXTURES = {
    "diffuse_tex",
    "flow_tex",
    "mask_a_tex",
    "mask_b_tex",
    "dissolve_tex",
}
LOCAL_CRACK_NAMED_TEXTURES = {"normal_tex", "refle_tex", "dissolve_tex"}
WATERTRAIL_NAMED_TEXTURES = {"maintex", "uv_noise_tex"}
MISSILETRAIL_NAMED_TEXTURES = {
    "alpha_tex",
    "emissive_tex01",
    "emissive_tex02",
    "uv_dissolve_tex",
    "uv_noise_tex",
}

STRICT_MISSILETRAIL_IDENTITY = {
    "sourceMaterialPath": (
        "fx_m_mi_m_00.fx_mi.fx_m_pa_missiletrail_01_17_tr"
    ),
    "profileId": (
        "ue3.material.fx.m.mi.03.fx.m.fx.m.me.trail.02.tr.8742928bef93"
    ),
    "parentMaterialPath": "fx_m_mi_03.fx_m.fx_m_me_trail_02_tr",
}
STRICT_MISSILETRAIL_RESOURCES = {
    "meshModel": "Effect/LanceMaster/Meshes/fm_m_ring_001.wmodel",
    "base": "Effect/LanceMaster/Textures/fx_h_atypical_01_1.dds",
    "mask": "Effect/LanceMaster/Textures/fx_m_trail_007.dds",
    "emissive": "Effect/LanceMaster/Textures/fx_m_noise_003.dds",
    "dissolve": "Effect/LanceMaster/Textures/fx_m_noise_001.dds",
    "noise": "Effect/LanceMaster/Textures/fx_d_noise_030.dds",
}
STRICT_WATERTRAIL_IDENTITY = {
    "sourceMaterialPath": "fx_m_mi_01.fx_mi.fx_h_me_watertrail_01_2_tr",
    "profileId": (
        "ue3.material.fx.m.mi.03.fx.m.fx.m.me.watertrail.01.tr.afa4aeba0c50"
    ),
    "parentMaterialPath": "fx_m_mi_03.fx_m.fx_m_me_watertrail_01_tr",
}
STRICT_WATERTRAIL_RESOURCES = {
    "meshModel": "Effect/DimensionMaster/Meshes/fm_h_swing_03.wmodel",
    "base": (
        "Effect/DimensionMaster/Textures/FX_TEX_04/fx_h_wave_01.dds"
    ),
    "noise": (
        "Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_noise_011.dds"
    ),
}
WARLORD_17090_TARGET_EFFECT_ID = "effect.warlord.skill.17090.unified"
WARLORD_CHAIN_SOURCE_MATERIAL_PATH = (
    "fx_m_mi_d_00.fx_mi.fx_d_me_chain_01_101_ma"
)
WARLORD_CHAIN_MODEL_ASSET_IDS = {
    "Effect/Warlord/Meshes/FX_SM_01/fm_d_berchain_06.wmodel",
    "Effect/Warlord/Meshes/FX_SM_01/fm_d_berchain_07.wmodel",
}
# This is only a tunable same-group starting point.  The source evidence does
# not prove that it, or any of the parent's nine referenced textures, owns the
# masked Base lane.  Keeping it as the compiler baseline gives G3 a real DDS
# target to override/reset without promoting the carrier to exact Product.
WARLORD_CHAIN_PREVIEW_BASE_ALIAS_ASSET_ID = (
    "Effect/Warlord/Textures/FX_TEX_02/fx_d_atypical_028.dds"
)
WARLORD_CHAIN_PROFILE_ID = (
    "ue3.material.fx.m.mi.00.fx.m.fx.d.me.chain.01.ma.a8a92d2a6abc"
)
WARLORD_CHAIN_PARENT_MATERIAL_PATH = "fx_m_mi_00.fx_m.fx_d_me_chain_01_ma"
WARLORD_CHAIN_APPROXIMATION_REASON = (
    "SOURCE_MASKED_WPO_ARITHMETIC_UNAVAILABLE"
)

CHARACTER_GHOST_SEMANTIC_FAMILY = "CHARACTER_AFTERIMAGE"
CHARACTER_GHOST_RUNTIME_BLOCKER = (
    "POSE_RUNTIME_BODY_EQUIPMENT_SNAPSHOT_AND_GHOST_MATERIAL_EXECUTION_UNAVAILABLE"
)
CHARACTER_GHOST_ACTION_SOURCE_ROOT = ROOT.parent / (
    "LostArk_Legacy_Quarantine_20260803/Resources/LostArk_SourceData/LPK/"
    "data3/EFGame_Extra/ClientData/XmlData/Action"
)
CHARACTER_GHOST_ACTION_SOURCES = {
    "ARTIST": {
        "filename": "YINYANGSHI.loa",
        "sha256": "0b0da69133ed6baa172ad1aeebebe48b9c8ea930c0cde85cb1b3ff34bf710def",
    },
    "LANCE_MASTER": {
        "filename": "LANCEMASTER.loa",
        "sha256": "42e536b66581c4b8e54622dcec7c623f2b310e79263ef28fc2d5035ba2588b19",
    },
    "WARLORD": {
        "filename": "GUNLANCER.loa",
        "sha256": "c7b2d97f86becbcf8e129f92a5a17cc132ed5dbd400f3b6db48eea411550c019",
    },
}
DIMENSIONMASTER_ACTION_CUE_RECIPE_ROOT = (
    ROOT / "Data/Effects/Imported/DimensionMaster/Converted"
)


class RestorationError(RuntimeError):
    """Raised when a source join or staged output is unsafe."""


@dataclass(frozen=True)
class CandidateRecord:
    character_class: str
    skill_id: int
    stage_index: int
    stage_clip_index: int
    clip: str
    target_effect_id: str
    target_path: Path
    blueprint_path: Path
    source_only: bool = False


@dataclass(frozen=True)
class SourceDocument:
    character_class: str
    path: Path
    effect_id: str
    document: dict[str, Any]
    elements: dict[str, dict[str, Any]]
    canonical_materials: dict[str, dict[str, Any]]
    generated_receipt_path: Path
    timeline_events: dict[str, dict[str, Any]]
    first_event_by_system: dict[str, str]
    material_resolution_by_path: dict[str, str]
    material_physical_package_by_path: dict[str, str]
    runtime_texture_by_source_path: dict[str, str]
    manifest_texture_by_source_path: dict[str, str]
    canonical_v12_package_aliases: dict[
        tuple[str, str, str], dict[str, str]
    ]
    shared_material_contracts: dict[tuple[str, str], dict[str, Any]]


@dataclass(frozen=True)
class SourceParticleAssignment:
    character_class: str
    skill_id: int
    source_document: SourceDocument
    source_element_id: str
    source_event_id: str
    source_order: int
    target_element_id: str
    target_effect_id: str
    clip_timeline_offset_seconds: float


@dataclass(frozen=True)
class SourceParticleExclusion:
    character_class: str
    skill_id: int
    source_effect_id: str
    source_element_id: str
    source_event_id: str
    reason: str


@dataclass(frozen=True)
class DrawableResourceDecision:
    receipt_path: Path
    decision: str
    disposition: str
    source_resources: tuple[tuple[str, str], ...]
    target_resources: tuple[tuple[str, str], ...]
    target_use_model_material: bool


@dataclass(frozen=True)
class SourceDecalAssignment:
    source_document: SourceDocument
    source_element_id: str
    source_event_id: str
    target_element_id: str
    clip_timeline_offset_seconds: float


@dataclass(frozen=True)
class SourceAnimationTrailAssignment:
    character_class: str
    skill_id: int
    source_event_id: str
    source_asset: str
    source_element_id: str
    source_emitter_path: str
    source_typed_data_path: str
    source_material_path: str
    source_graph_path: Path
    source_graph_sha256: str
    source_graph_evidence_sha256: str
    source_material_map_path: Path
    source_material_map_sha256: str
    source_physical_package: str
    source_material_object_path: str
    target_element_id: str
    target_effect_id: str
    local_time_seconds: float
    duration_seconds: float
    control_edge_name: str
    tiling_distance_world_units: float
    distance_tessellation_step_world_units: float
    runtime_anchor_slot_id: str
    runtime_bone_name: str
    render_profile: str
    resources: tuple[tuple[str, str], ...]
    base_decision: str


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(
            path.read_text(encoding="utf-8-sig"),
            parse_constant=lambda value: (_ for _ in ()).throw(
                RestorationError(f"non-finite JSON value in {path}: {value}")
            ),
        )
    except (OSError, json.JSONDecodeError) as error:
        raise RestorationError(f"cannot read JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise RestorationError(f"JSON root must be an object: {path}")
    return value


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_bytes(value)).hexdigest()


def raw_sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def decode_dimension_summon_diffuse_alpha(asset_id: str) -> dict[str, Any]:
    """Decode the exact top-mip BC alpha lane used by the masked ModelCue."""

    path = resource_path(asset_id)
    try:
        payload = path.read_bytes()
    except OSError as error:
        raise RestorationError(
            f"cannot read Dimension Summon diffuse DDS: {asset_id}: {error}"
        ) from error
    if (
        len(payload) < 128
        or payload[:4] != b"DDS "
        or struct.unpack_from("<I", payload, 4)[0] != 124
        or struct.unpack_from("<I", payload, 76)[0] != 32
    ):
        raise RestorationError(
            f"Dimension Summon diffuse DDS header is invalid: {asset_id}"
        )
    height = struct.unpack_from("<I", payload, 12)[0]
    width = struct.unpack_from("<I", payload, 16)[0]
    four_cc = payload[84:88]
    if width != 1024 or height != 1024 or four_cc not in {b"DXT1", b"DXT5"}:
        raise RestorationError(
            f"Dimension Summon diffuse DDS format drifted: {asset_id}"
        )
    block_size = 8 if four_cc == b"DXT1" else 16
    block_count = ((width + 3) // 4) * ((height + 3) // 4)
    top_mip_size = block_count * block_size
    if len(payload) < 128 + top_mip_size:
        raise RestorationError(
            f"Dimension Summon diffuse DDS top mip is truncated: {asset_id}"
        )

    below_cutoff = 0
    zero_alpha = 0
    full_alpha = 0
    byte_offset = 128
    cutoff_byte = math.ceil(DIMENSION_SUMMON_MASK_CUTOFF * 255.0)
    for _ in range(block_count):
        block = payload[byte_offset : byte_offset + block_size]
        byte_offset += block_size
        if four_cc == b"DXT1":
            color0, color1, indices = struct.unpack_from("<HHI", block)
            palette = (255, 255, 255, 255 if color0 > color1 else 0)
            index_mask = 0x3
            index_shift = 2
        else:
            alpha0, alpha1 = block[0], block[1]
            indices = int.from_bytes(block[2:8], "little")
            if alpha0 > alpha1:
                palette = tuple(
                    [alpha0, alpha1]
                    + [
                        ((8 - index) * alpha0 + (index - 1) * alpha1) // 7
                        for index in range(2, 8)
                    ]
                )
            else:
                palette = tuple(
                    [alpha0, alpha1]
                    + [
                        ((6 - index) * alpha0 + (index - 1) * alpha1) // 5
                        for index in range(2, 6)
                    ]
                    + [0, 255]
                )
            index_mask = 0x7
            index_shift = 3
        for _ in range(16):
            alpha = palette[indices & index_mask]
            indices >>= index_shift
            below_cutoff += alpha < cutoff_byte
            zero_alpha += alpha == 0
            full_alpha += alpha == 255
    return {
        "assetId": asset_id,
        "sha256": hashlib.sha256(payload).hexdigest(),
        "fourCC": four_cc.decode("ascii"),
        "width": width,
        "height": height,
        "pixelCount": width * height,
        "belowCutoffPixelCount": below_cutoff,
        "zeroAlphaPixelCount": zero_alpha,
        "fullAlphaPixelCount": full_alpha,
    }


def dimension_summon_mask_contract() -> dict[str, Any]:
    sections: list[dict[str, Any]] = []
    for expected in DIMENSION_SUMMON_DIFFUSE_ALPHA_CONTRACT:
        actual = decode_dimension_summon_diffuse_alpha(expected["assetId"])
        for field in (
            "sha256",
            "fourCC",
            "belowCutoffPixelCount",
            "zeroAlphaPixelCount",
            "fullAlphaPixelCount",
        ):
            if actual[field] != expected[field]:
                raise RestorationError(
                    "Dimension Summon diffuse alpha contract drifted: "
                    f"section={expected['section']} field={field} "
                    f"expected={expected[field]} actual={actual[field]}"
                )
        sections.append({"section": expected["section"], **actual})
    if (
        any(row["belowCutoffPixelCount"] != 0 for row in sections[:3])
        or sections[3]["belowCutoffPixelCount"] == 0
        or sections[3]["belowCutoffPixelCount"] == sections[3]["pixelCount"]
    ):
        raise RestorationError(
            "Dimension Summon section alpha ownership is no longer 01-03 full / 04 masked"
        )
    return {
        "sourceParentMaterial": (
            "efmaster_material_prologue.mastermaterial_ch.monster.high.realpbr."
            "monster_dead_msk_high_realpbr"
        ),
        "blendMode": "BLEND_Masked",
        "twoSided": False,
        "opacityMaskClipValue": DIMENSION_SUMMON_MASK_CUTOFF,
        "runtimeOpacityProjection": "texture_basecolor.alpha",
        "sections": sections,
    }


def serialized(value: dict[str, Any]) -> str:
    return json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n"


def repository_path(relative: str, label: str) -> Path:
    if not isinstance(relative, str) or not relative:
        raise RestorationError(f"{label} must be a non-empty repository path")
    pure = PurePosixPath(relative.replace("\\", "/"))
    if pure.is_absolute() or ".." in pure.parts or ":" in pure.parts[0]:
        raise RestorationError(f"unsafe {label}: {relative}")
    path = (ROOT / Path(*pure.parts)).resolve()
    try:
        path.relative_to(ROOT.resolve())
    except ValueError as error:
        raise RestorationError(f"{label} escapes the repository: {relative}") from error
    return path


def resource_path(asset_id: str) -> Path:
    if not isinstance(asset_id, str) or not asset_id:
        raise RestorationError("resource assetId must be non-empty")
    pure = PurePosixPath(asset_id.replace("\\", "/"))
    if pure.is_absolute() or ".." in pure.parts or ":" in pure.parts[0]:
        raise RestorationError(f"unsafe Resources-relative assetId: {asset_id}")
    path = (RESOURCE_ROOT / Path(*pure.parts)).resolve()
    try:
        path.relative_to(RESOURCE_ROOT.resolve())
    except ValueError as error:
        raise RestorationError(f"resource escapes Resources: {asset_id}") from error
    return path


def strict_effect_resource_kind(asset_id: Any, label: str) -> str:
    """Validate the native Effect codec's Resources-relative asset contract."""

    if (
        not isinstance(asset_id, str)
        or not asset_id
        or len(asset_id.encode("utf-8")) > 512
        or not asset_id.startswith("Effect/")
        or "\\" in asset_id
        or ":" in asset_id
        or "\x00" in asset_id
    ):
        raise RestorationError(f"unsafe {label}: {asset_id}")
    pure = PurePosixPath(asset_id)
    if (
        pure.is_absolute()
        or any(part in {"", ".", ".."} for part in pure.parts)
        or pure.as_posix() != asset_id
    ):
        raise RestorationError(f"unsafe {label}: {asset_id}")
    suffix = pure.suffix.casefold()
    if suffix == ".dds":
        kind = "texture"
    elif suffix == ".wmodel":
        kind = "model"
    else:
        raise RestorationError(f"invalid {label} resource type: {asset_id}")
    try:
        path = resource_path(asset_id)
    except (OSError, ValueError) as error:
        raise RestorationError(f"unsafe {label}: {asset_id}") from error
    if not path.is_file():
        raise RestorationError(f"missing {label}: {asset_id}")
    return kind


def _finite_number(value: Any, label: str) -> int | float:
    if (
        isinstance(value, bool)
        or not isinstance(value, (int, float))
        or not math.isfinite(float(value))
    ):
        raise RestorationError(f"{label} must be a finite number")
    return value


def _finite_float4(value: Any, label: str) -> list[int | float]:
    if not isinstance(value, list) or len(value) != 4:
        raise RestorationError(f"{label} must be a finite float4")
    return [
        _finite_number(component, f"{label}[{index}]")
        for index, component in enumerate(value)
    ]


def _resource_override_target(
    element: dict[str, Any], target_id: str, label: str
) -> tuple[dict[str, Any], str] | None:
    if target_id.startswith(SOURCE_MATERIAL_TEXTURE_PREFIX):
        texture_name = target_id[len(SOURCE_MATERIAL_TEXTURE_PREFIX) :]
        if not texture_name:
            raise RestorationError(f"invalid {label} source texture target")
        material = element.get("material")
        profile = (
            material.get("sourceProfile") if isinstance(material, dict) else None
        )
        if profile is None:
            return None
        if not isinstance(profile, dict):
            raise RestorationError(f"invalid {label} source profile")
        textures = profile.get("textures")
        if textures is None:
            return None
        if not isinstance(textures, list):
            raise RestorationError(f"invalid {label} source profile textures")
        matches = [
            row
            for row in textures
            if isinstance(row, dict) and row.get("name") == texture_name
        ]
        if len(matches) > 1:
            raise RestorationError(
                f"duplicate {label} source profile texture: {texture_name}"
            )
        if not matches:
            return None
        row = matches[0]
        actual_kind = strict_effect_resource_kind(
            row.get("assetId"), f"{label} source texture asset"
        )
        if actual_kind != "texture":
            raise RestorationError(
                f"invalid {label} source texture resource type: {texture_name}"
            )
        return row, "texture"

    if target_id.startswith(MATERIAL_EXECUTION_LANE_PREFIX):
        lane_id = target_id[len(MATERIAL_EXECUTION_LANE_PREFIX) :]
        if not lane_id:
            raise RestorationError(f"invalid {label} execution lane target")
        material = element.get("material")
        execution = material.get("execution") if isinstance(material, dict) else None
        if execution is None:
            return None
        if not isinstance(execution, dict):
            raise RestorationError(f"invalid {label} material execution")
        lanes = execution.get("textureLanes")
        if lanes is None:
            return None
        if not isinstance(lanes, list):
            raise RestorationError(f"invalid {label} texture lanes")
        matches = [
            row
            for row in lanes
            if isinstance(row, dict) and row.get("laneId") == lane_id
        ]
        if len(matches) > 1:
            raise RestorationError(f"duplicate {label} execution lane: {lane_id}")
        if not matches:
            return None
        row = matches[0]
        actual_kind = strict_effect_resource_kind(
            row.get("assetId"), f"{label} execution lane asset"
        )
        if actual_kind != "texture":
            raise RestorationError(
                f"invalid {label} execution lane resource type: {lane_id}"
            )
        return row, "texture"

    bindings = element.get("resources")
    if bindings is None:
        return None
    if not isinstance(bindings, list):
        raise RestorationError(f"invalid {label} resources")
    matches = [
        row
        for row in bindings
        if isinstance(row, dict) and row.get("slotId") == target_id
    ]
    if len(matches) > 1:
        raise RestorationError(f"duplicate {label} resource slot: {target_id}")
    if not matches:
        return None
    row = matches[0]
    actual_kind = strict_effect_resource_kind(
        row.get("assetId"), f"{label} resource asset"
    )
    expected_kind = "model" if target_id == "meshModel" else "texture"
    if actual_kind != expected_kind:
        raise RestorationError(
            f"invalid {label} resource type for target: {target_id}"
        )
    return row, expected_kind


def _source_profile_parameter(
    element: dict[str, Any], collection: str, name: str, label: str
) -> dict[str, Any] | None:
    material = element.get("material")
    profile = material.get("sourceProfile") if isinstance(material, dict) else None
    if profile is None:
        return None
    if not isinstance(profile, dict):
        raise RestorationError(f"invalid {label} source profile")
    rows = profile.get(collection)
    if rows is None:
        return None
    if not isinstance(rows, list):
        raise RestorationError(f"invalid {label} source profile {collection}")
    matches = [
        row
        for row in rows
        if isinstance(row, dict) and row.get("name") == name
    ]
    if len(matches) > 1:
        raise RestorationError(
            f"duplicate {label} source profile parameter: {name}"
        )
    return matches[0] if matches else None


def _execution_parameter_rows(
    element: dict[str, Any], name: str, label: str
) -> list[tuple[str, dict[str, Any]]]:
    material = element.get("material")
    execution = material.get("execution") if isinstance(material, dict) else None
    if execution is None:
        return []
    if not isinstance(execution, dict):
        raise RestorationError(f"invalid {label} material execution")
    matches: list[tuple[str, dict[str, Any]]] = []
    for collection in ("scalars", "vectors", "artistParameters", "colors"):
        rows = execution.get(collection)
        if rows is None:
            continue
        if not isinstance(rows, list):
            raise RestorationError(f"invalid {label} execution {collection}")
        collection_matches = [
            row
            for row in rows
            if isinstance(row, dict) and row.get("name") == name
        ]
        if len(collection_matches) > 1:
            raise RestorationError(
                f"duplicate {label} execution parameter: {collection}/{name}"
            )
        if collection_matches:
            matches.append((collection, collection_matches[0]))
    return matches


def _scalar_override_target(
    element: dict[str, Any], name: str, label: str
) -> tuple[list[dict[str, Any]], int | float] | None:
    """Resolve one scalar identity across source/execution effective mirrors."""

    source = _source_profile_parameter(element, "scalars", name, label)
    wrong_source = _source_profile_parameter(element, "vectors", name, label)
    execution = _execution_parameter_rows(element, name, label)
    if wrong_source is not None or any(
        collection != "scalars" for collection, _ in execution
    ):
        raise RestorationError(f"{label} parameter type mismatch: {name}")
    targets = ([] if source is None else [source]) + [
        row for _, row in execution
    ]
    if not targets:
        return None
    values = [
        _finite_number(row.get("value"), f"{label} effective value")
        for row in targets
    ]
    if any(value != values[0] for value in values[1:]):
        raise RestorationError(f"{label} effective mirror mismatch: {name}")
    return targets, values[0]


def _color_override_target(
    element: dict[str, Any], name: str, label: str
) -> tuple[list[dict[str, Any]], list[int | float]] | None:
    """Resolve one float4 identity across source/execution effective mirrors."""

    source = _source_profile_parameter(element, "vectors", name, label)
    wrong_source = _source_profile_parameter(element, "scalars", name, label)
    execution = _execution_parameter_rows(element, name, label)
    if wrong_source is not None or any(
        collection == "scalars" for collection, _ in execution
    ):
        raise RestorationError(f"{label} parameter type mismatch: {name}")
    targets = ([] if source is None else [source]) + [
        row for _, row in execution
    ]
    if not targets:
        return None
    values = [
        _finite_float4(row.get("value"), f"{label} effective value")
        for row in targets
    ]
    if any(value != values[0] for value in values[1:]):
        raise RestorationError(f"{label} effective mirror mismatch: {name}")
    return targets, values[0]


def reapply_authoring_overrides(
    compiler_staged: dict[str, Any],
    stable_current: dict[str, Any],
) -> tuple[dict[str, Any], list[dict[str, str]]]:
    """Apply valid artist deltas after compiler refresh without partial commit."""

    candidate = copy.deepcopy(compiler_staged)
    raw = stable_current.get("authoringOverrides")
    if raw is None:
        candidate.pop("authoringOverrides", None)
        return candidate, []
    if not isinstance(raw, dict) or set(raw) != {"resources", "scalars", "colors"}:
        raise RestorationError("authoringOverrides shape is invalid")
    for collection in ("resources", "scalars", "colors"):
        if not isinstance(raw.get(collection), list):
            raise RestorationError(
                f"authoringOverrides.{collection} must be an array"
            )

    resource_rows = copy.deepcopy(raw["resources"])
    scalar_rows = copy.deepcopy(raw["scalars"])
    color_rows = copy.deepcopy(raw["colors"])
    drops: list[dict[str, str]] = []
    surviving_resources: list[dict[str, Any]] = []
    surviving_scalars: list[dict[str, Any]] = []
    surviving_colors: list[dict[str, Any]] = []

    seen_resources: set[str] = set()
    for override in resource_rows:
        if not isinstance(override, dict) or set(override) != {
            "slotId",
            "assetId",
            "compilerAssetId",
        }:
            raise RestorationError("authoring resource override shape is invalid")
        target_id = override.get("slotId")
        if (
            not isinstance(target_id, str)
            or not target_id
            or target_id in seen_resources
        ):
            raise RestorationError("duplicate/invalid authoring resource target")
        seen_resources.add(target_id)
        previous_target = _resource_override_target(
            stable_current, target_id, "stable authoring override"
        )
        if previous_target is None:
            raise RestorationError(
                f"authoring resource override target is unknown: {target_id}"
            )
        previous_row, expected_kind = previous_target
        artist_kind = strict_effect_resource_kind(
            override.get("assetId"), "authoring override asset"
        )
        baseline_kind = strict_effect_resource_kind(
            override.get("compilerAssetId"), "authoring override compiler asset"
        )
        if artist_kind != expected_kind or baseline_kind != expected_kind:
            raise RestorationError(
                f"authoring resource override type mismatch: {target_id}"
            )
        if override["assetId"] == override["compilerAssetId"]:
            raise RestorationError(
                f"authoring resource override is a no-op: {target_id}"
            )
        if previous_row.get("assetId") != override["assetId"]:
            raise RestorationError(
                f"authoring resource override effective value mismatch: {target_id}"
            )
        refreshed_target = _resource_override_target(
            candidate, target_id, "compiler authoring override"
        )
        if refreshed_target is None:
            drops.append(
                {
                    "targetKind": "resource",
                    "targetId": target_id,
                    "reason": "RESOURCE_SLOT_VANISHED",
                }
            )
            continue
        refreshed_row, refreshed_kind = refreshed_target
        if refreshed_kind != expected_kind:
            raise RestorationError(
                f"compiler authoring resource target type changed: {target_id}"
            )
        compiler_asset_id = refreshed_row["assetId"]
        if compiler_asset_id == override["assetId"]:
            continue
        refreshed_row["assetId"] = override["assetId"]
        surviving_resources.append(
            {
                "slotId": target_id,
                "assetId": override["assetId"],
                "compilerAssetId": compiler_asset_id,
            }
        )

    seen_scalars: set[str] = set()
    for override in scalar_rows:
        if not isinstance(override, dict) or set(override) != {
            "name",
            "value",
            "compilerValue",
        }:
            raise RestorationError("authoring scalar override shape is invalid")
        name = override.get("name")
        if not isinstance(name, str) or not name or name in seen_scalars:
            raise RestorationError("duplicate/invalid authoring scalar target")
        seen_scalars.add(name)
        artist_value = _finite_number(
            override.get("value"), f"authoring scalar {name}"
        )
        previous_compiler_value = _finite_number(
            override.get("compilerValue"), f"authoring scalar baseline {name}"
        )
        if artist_value == previous_compiler_value:
            raise RestorationError(f"authoring scalar override is a no-op: {name}")
        previous_target = _scalar_override_target(
            stable_current, name, "stable authoring scalar"
        )
        if previous_target is None:
            raise RestorationError(f"authoring scalar target is unknown: {name}")
        _, previous_value = previous_target
        if previous_value != artist_value:
            raise RestorationError(
                f"authoring scalar effective value mismatch: {name}"
            )
        refreshed_target = _scalar_override_target(
            candidate, name, "compiler authoring scalar"
        )
        if refreshed_target is None:
            drops.append(
                {
                    "targetKind": "scalar",
                    "targetId": name,
                    "reason": "SCALAR_PARAMETER_VANISHED",
                }
            )
            continue
        refreshed_rows, compiler_value = refreshed_target
        if compiler_value == artist_value:
            continue
        for row in refreshed_rows:
            row["value"] = artist_value
        surviving_scalars.append(
            {
                "name": name,
                "value": artist_value,
                "compilerValue": compiler_value,
            }
        )

    seen_colors: set[str] = set()
    for override in color_rows:
        if not isinstance(override, dict) or set(override) != {
            "name",
            "value",
            "compilerValue",
        }:
            raise RestorationError("authoring color override shape is invalid")
        name = override.get("name")
        if (
            not isinstance(name, str)
            or not name
            or name in seen_colors
            or name in seen_scalars
        ):
            raise RestorationError("duplicate/invalid authoring color target")
        seen_colors.add(name)
        artist_value = _finite_float4(
            override.get("value"), f"authoring color {name}"
        )
        previous_compiler_value = _finite_float4(
            override.get("compilerValue"), f"authoring color baseline {name}"
        )
        if artist_value == previous_compiler_value:
            raise RestorationError(f"authoring color override is a no-op: {name}")
        previous_target = _color_override_target(
            stable_current, name, "stable authoring color"
        )
        if previous_target is None:
            raise RestorationError(f"authoring color target is unknown: {name}")
        _, previous_value = previous_target
        if previous_value != artist_value:
            raise RestorationError(
                f"authoring color effective value mismatch: {name}"
            )
        refreshed_target = _color_override_target(
            candidate, name, "compiler authoring color"
        )
        if refreshed_target is None:
            drops.append(
                {
                    "targetKind": "color",
                    "targetId": name,
                    "reason": "COLOR_PARAMETER_VANISHED",
                }
            )
            continue
        refreshed_rows, compiler_value = refreshed_target
        if compiler_value == artist_value:
            continue
        for row in refreshed_rows:
            row["value"] = copy.deepcopy(artist_value)
        surviving_colors.append(
            {
                "name": name,
                "value": copy.deepcopy(artist_value),
                "compilerValue": copy.deepcopy(compiler_value),
            }
        )

    surviving = {
        "resources": surviving_resources,
        "scalars": surviving_scalars,
        "colors": surviving_colors,
    }
    if any(surviving.values()):
        candidate["authoringOverrides"] = surviving
    else:
        candidate.pop("authoringOverrides", None)
    return candidate, drops


def require_authoring_document(
    document: dict[str, Any],
    *,
    path: Path,
    effect_id: str,
    versions: set[int],
) -> list[dict[str, Any]]:
    if (
        document.get("schema") != "lostark.effect-authoring"
        or document.get("version") not in versions
        or document.get("effectAssetId") != effect_id
    ):
        raise RestorationError(f"Effect document identity changed: {path}")
    raw_elements = document.get("elements")
    if not isinstance(raw_elements, list):
        raise RestorationError(f"Effect elements must be an array: {path}")
    elements: list[dict[str, Any]] = []
    ids: set[str] = set()
    for value in raw_elements:
        if not isinstance(value, dict):
            raise RestorationError(f"Effect element must be an object: {path}")
        element_id = value.get("id")
        if not isinstance(element_id, str) or not element_id or element_id in ids:
            raise RestorationError(f"missing/duplicate Element ID in {path}: {element_id}")
        ids.add(element_id)
        elements.append(value)
    return elements


def candidate_records(batch: dict[str, Any]) -> list[CandidateRecord]:
    records: list[CandidateRecord] = []
    for raw_stage in batch.get("stages", []):
        if not isinstance(raw_stage, dict):
            raise RestorationError("batch stage must be an object")
        target = raw_stage.get("target")
        if not isinstance(target, dict):
            raise RestorationError("batch Track A stage has no target")
        legacy = target.get("legacyRollbackBaseline")
        if not isinstance(legacy, dict):
            raise RestorationError("batch Track A target has no legacy rollback baseline")
        records.append(
            CandidateRecord(
                character_class=str(raw_stage["characterClass"]),
                skill_id=int(raw_stage["skillId"]),
                stage_index=int(raw_stage["stageIndex"]),
                stage_clip_index=0,
                clip=str(raw_stage["clip"]),
                target_effect_id=str(target["effectAssetId"]),
                target_path=repository_path(str(target["path"]), "target path"),
                blueprint_path=repository_path(str(legacy["path"]), "blueprint path"),
            )
        )
    for raw_candidate in batch.get("legacyStarterCandidates", []):
        if not isinstance(raw_candidate, dict):
            raise RestorationError("batch candidate must be an object")
        target = raw_candidate.get("target")
        legacy = raw_candidate.get("legacyRollbackBaseline")
        if not isinstance(target, dict) or not isinstance(legacy, dict):
            raise RestorationError("batch candidate target/baseline is invalid")
        records.append(
            CandidateRecord(
                character_class=str(raw_candidate["characterClass"]),
                skill_id=int(raw_candidate["skillId"]),
                stage_index=int(raw_candidate["stageIndex"]),
                stage_clip_index=int(raw_candidate["stageClipIndex"]),
                clip=str(raw_candidate["clip"]),
                target_effect_id=str(target["effectAssetId"]),
                target_path=repository_path(str(target["path"]), "target path"),
                blueprint_path=repository_path(str(legacy["path"]), "blueprint path"),
            )
        )
    records.sort(
        key=lambda row: (
            CLASS_ORDER[row.character_class],
            row.skill_id,
            row.stage_index,
            row.stage_clip_index,
            row.target_effect_id,
        )
    )
    target_ids = [row.target_effect_id for row in records]
    target_paths = [row.target_path for row in records]
    if (
        len(records) != EXPECTED_TARGET_COUNT
        or len(target_ids) != len(set(target_ids))
        or len(target_paths) != len(set(target_paths))
    ):
        raise RestorationError(
            "four-class candidate denominator/identity changed: "
            f"records={len(records)} uniqueIds={len(set(target_ids))} "
            f"uniquePaths={len(set(target_paths))}"
        )
    return records


def _character_ghost_material_references(
    references: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    return [
        copy.deepcopy(row)
        for row in references
        if isinstance(row, dict)
        and "material" in str(row.get("className") or "").casefold()
    ]


def _validate_character_ghost_payload(
    payload: dict[str, Any], *, source_identity: str
) -> dict[str, Any]:
    if payload.get("encoding") != "base64":
        raise RestorationError(
            f"character ghost payload encoding changed: {source_identity}"
        )
    try:
        decoded = base64.b64decode(str(payload.get("data") or ""), validate=True)
    except (ValueError, TypeError) as error:
        raise RestorationError(
            f"character ghost payload is not valid base64: {source_identity}"
        ) from error
    if len(decoded) != int(payload.get("byteSize", -1)):
        raise RestorationError(
            f"character ghost payload byte size changed: {source_identity}"
        )
    digest = hashlib.sha256(decoded).hexdigest()
    if digest != str(payload.get("sha256") or "").casefold():
        raise RestorationError(
            f"character ghost payload SHA changed: {source_identity}"
        )
    return copy.deepcopy(payload)


def _character_ghost_cue_row(
    *,
    context: dict[str, Any],
    raw_notify_id: str,
    source_stage_index: int,
    local_time_seconds: float,
    duration_seconds: float,
    global_time_seconds: float,
    serialized_payload: dict[str, Any],
    serialized_labels: list[Any],
    asset_references: list[dict[str, Any]],
    source_artifact: dict[str, Any],
) -> dict[str, Any]:
    record: CandidateRecord = context["record"]
    event: dict[str, Any] = context["event"]
    payload = _validate_character_ghost_payload(
        serialized_payload,
        source_identity=(
            f"{record.character_class}/{record.skill_id}/"
            f"{event['eventId']}/{raw_notify_id}"
        ),
    )
    identity = "\0".join(
        (
            record.character_class,
            str(record.skill_id),
            record.target_effect_id,
            str(record.stage_index),
            str(record.stage_clip_index),
            record.clip,
            str(event["eventId"]),
            raw_notify_id,
            str(payload["sha256"]),
        )
    )
    return {
        "cueId": (
            "character-ghost."
            + hashlib.sha256(identity.encode("utf-8")).hexdigest()[:24]
        ),
        "characterClass": record.character_class,
        "skillId": record.skill_id,
        "stageIndex": record.stage_index,
        "stageClipIndex": record.stage_clip_index,
        "targetEffectAssetId": record.target_effect_id,
        "clip": record.clip,
        "sourceEventId": str(event["eventId"]),
        "sourceReceiptEventIndex": int(context["receiptEventIndex"]),
        "rawNotifyId": raw_notify_id,
        "sourceStageIndex": source_stage_index,
        "localTimeSeconds": local_time_seconds,
        "globalTimeSeconds": global_time_seconds,
        "durationSeconds": duration_seconds,
        "serializedPayload": payload,
        "serializedLabels": copy.deepcopy(serialized_labels),
        "assetReferences": copy.deepcopy(asset_references),
        "materialReferences": _character_ghost_material_references(
            asset_references
        ),
        "sourceArtifact": copy.deepcopy(source_artifact),
        "semanticFamily": CHARACTER_GHOST_SEMANTIC_FAMILY,
        "admitted": False,
        "admission": {
            "admitted": False,
            "failClosed": True,
            "blocker": CHARACTER_GHOST_RUNTIME_BLOCKER,
        },
    }


def _character_ghost_source_contexts(
    records: list[CandidateRecord],
) -> list[dict[str, Any]]:
    records_by_class: dict[str, list[CandidateRecord]] = {}
    for record in records:
        records_by_class.setdefault(record.character_class, []).append(record)
    receipt_cache: dict[Path, dict[str, Any]] = {}
    contexts: list[dict[str, Any]] = []
    for character_class, manifest_path in CLASS_MANIFESTS.items():
        manifest = load_json(manifest_path)
        if (
            manifest.get("characterClass") != character_class
            or not isinstance(manifest.get("skills"), list)
        ):
            raise RestorationError(
                f"character ghost stage manifest identity changed: {manifest_path}"
            )
        skills = {
            int(row["productSkillId"]): row
            for row in manifest["skills"]
            if isinstance(row, dict) and "productSkillId" in row
        }
        for record in records_by_class.get(character_class, []):
            skill = skills.get(record.skill_id)
            if skill is None:
                raise RestorationError(
                    f"character ghost skill has no manifest row: "
                    f"{character_class}/{record.skill_id}"
                )
            stages = [
                row
                for row in skill.get("stages", [])
                if int(row.get("stageIndex", -1)) == record.stage_index
            ]
            if len(stages) != 1:
                raise RestorationError(
                    f"character ghost stage join is not unique: "
                    f"{character_class}/{record.skill_id}/{record.stage_index}"
                )
            stage = stages[0]
            clips = [
                row
                for row in stage.get("clips", [])
                if str(row.get("clip") or "").casefold() == record.clip.casefold()
                and int(row.get("stageClipIndex", -1)) == record.stage_clip_index
            ]
            if len(clips) != 1:
                raise RestorationError(
                    f"character ghost clip join is not unique: "
                    f"{character_class}/{record.skill_id}/{record.clip}"
                )
            source_skill_id = int(clips[0].get("sourceSkillId", record.skill_id))
            artifacts = [
                row
                for row in stage.get("sourceArtifacts", [])
                if int(row.get("sourceSkillId", source_skill_id)) == source_skill_id
            ]
            if len(artifacts) != 1:
                raise RestorationError(
                    f"character ghost source artifact join is not unique: "
                    f"{character_class}/{record.skill_id}/{record.clip}"
                )
            generated = artifacts[0].get("generatedSourceReceipt")
            if not isinstance(generated, dict):
                raise RestorationError(
                    f"character ghost source receipt is missing: "
                    f"{character_class}/{record.skill_id}/{record.clip}"
                )
            receipt_path = repository_path(
                str(generated.get("path") or ""), "character ghost source receipt"
            )
            declared_sha = str(generated.get("sha256") or "").casefold()
            receipt_sha = raw_sha256(receipt_path)
            if receipt_sha != declared_sha:
                raise RestorationError(
                    f"character ghost source receipt SHA changed: {receipt_path}"
                )
            receipt = receipt_cache.setdefault(receipt_path, load_json(receipt_path))
            events = receipt.get("timeline", {}).get("events", [])
            if not isinstance(events, list):
                raise RestorationError(
                    f"character ghost source timeline is invalid: {receipt_path}"
                )
            for event_index, event in enumerate(events):
                if (
                    str(event.get("clip") or "").casefold()
                    != record.clip.casefold()
                    or event.get("sourceType") != "TrailGhostEffect"
                ):
                    continue
                contexts.append(
                    {
                        "record": record,
                        "event": event,
                        "receiptEventIndex": event_index,
                        "receiptPath": receipt_path,
                        "receiptRawSha256": receipt_sha,
                    }
                )
    return contexts


def _dimensionmaster_character_ghost_cues(
    contexts: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    contexts_by_skill_event = {
        (context["record"].skill_id, int(context["receiptEventIndex"])): context
        for context in contexts
        if context["record"].character_class == "DIMENSIONMASTER"
    }
    result: list[dict[str, Any]] = []
    for skill_id in sorted({key[0] for key in contexts_by_skill_event}):
        recipe_path = (
            DIMENSIONMASTER_ACTION_CUE_RECIPE_ROOT
            / f"skill.{skill_id}.action-cue-recipe.json"
        )
        recipe = load_json(recipe_path)
        if (
            recipe.get("schema") != "lostark.effect-action-cue-recipe"
            or int(recipe.get("skillId", -1)) != skill_id
        ):
            raise RestorationError(
                f"DimensionMaster character ghost recipe identity changed: {recipe_path}"
            )
        for cue in recipe.get("cues", []):
            if cue.get("sourceType") != "TrailGhostEffect":
                continue
            reference_index = cue.get("sourceReceiptEventIndex")
            if not isinstance(reference_index, int):
                raise RestorationError(
                    f"DimensionMaster character ghost lost its source event: {recipe_path}"
                )
            context = contexts_by_skill_event.get((skill_id, reference_index))
            if context is None:
                continue
            source_stage_index = int(cue["sourceStageIndex"])
            source_occurrence = cue.get("sourceOccurrence")
            if isinstance(source_occurrence, dict):
                raw_notify_id = str(source_occurrence.get("notifyId") or "")
            else:
                notify_suffix = str(cue.get("cueId") or "").rsplit("/", 1)[-1]
                if not re.fullmatch(r"notify-\d{3}", notify_suffix):
                    raise RestorationError(
                        f"DimensionMaster raw notify identity changed: {recipe_path}"
                    )
                raw_notify_id = (
                    f"action-{skill_id}/stage-{source_stage_index:03d}/"
                    f"{notify_suffix}"
                )
            source_artifact = {
                "kind": "EXISTING_ACTION_CUE_RECIPE",
                "path": recipe_path.relative_to(ROOT).as_posix(),
                "rawSha256": raw_sha256(recipe_path),
                "upstreamActionSourceSha256": str(
                    recipe.get("source", {}).get("actionSourceSha256") or ""
                ),
                "upstreamLoaSha256": str(
                    recipe.get("source", {}).get("loaSha256") or ""
                ),
                "sourceReceipt": {
                    "path": context["receiptPath"].relative_to(ROOT).as_posix(),
                    "rawSha256": context["receiptRawSha256"],
                },
            }
            result.append(
                _character_ghost_cue_row(
                    context=context,
                    raw_notify_id=raw_notify_id,
                    source_stage_index=source_stage_index,
                    local_time_seconds=float(cue["localTimeSeconds"]),
                    global_time_seconds=float(cue["globalTimeSeconds"]),
                    duration_seconds=float(cue["durationSeconds"]),
                    serialized_payload=cue["serializedPayload"],
                    serialized_labels=list(cue.get("serializedLabels", [])),
                    asset_references=list(cue.get("assetReferences", [])),
                    source_artifact=source_artifact,
                )
            )
    if len(result) != len(contexts_by_skill_event):
        raise RestorationError(
            "DimensionMaster character ghost recipe coverage changed: "
            f"cues={len(result)} contexts={len(contexts_by_skill_event)}"
        )
    return result


def _normalized_character_ghost_clip(value: str) -> str:
    return re.sub(r"[^a-z0-9]", "", value.casefold())


def _extracted_character_ghost_cues(
    contexts: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    for character_class, source in CHARACTER_GHOST_ACTION_SOURCES.items():
        class_contexts = [
            row
            for row in contexts
            if row["record"].character_class == character_class
        ]
        if not class_contexts:
            continue
        source_path = CHARACTER_GHOST_ACTION_SOURCE_ROOT / str(source["filename"])
        source_sha = raw_sha256(source_path)
        if source_sha != source["sha256"]:
            raise RestorationError(
                f"character ghost Action source SHA changed: {source_path}"
            )
        skill_ids = {row["record"].skill_id for row in class_contexts}
        logical_path = f"XmlData/Action/{source['filename']}"
        extracted = extract_action_document(
            source_path,
            character_class,
            action_ids=skill_ids,
            source_logical_path=logical_path,
        )
        actions = {
            int(row["actionId"]): row for row in extracted.get("actions", [])
        }
        contexts_by_target: dict[str, list[dict[str, Any]]] = {}
        for context in class_contexts:
            contexts_by_target.setdefault(
                context["record"].target_effect_id, []
            ).append(context)
        for target_effect_id, target_contexts in contexts_by_target.items():
            record: CandidateRecord = target_contexts[0]["record"]
            action = actions.get(record.skill_id)
            if action is None:
                raise RestorationError(
                    f"character ghost Action is missing: "
                    f"{character_class}/{record.skill_id}"
                )
            runtime_clip_key = _normalized_character_ghost_clip(record.clip)
            source_clips = {
                str(clip.get("clipName") or "")
                for stage in action.get("stages", [])
                for clip in stage.get("animationClips", [])
                if str(clip.get("clipName") or "")
            }
            matching_source_clips = [
                clip
                for clip in source_clips
                if runtime_clip_key.endswith(_normalized_character_ghost_clip(clip))
                or _normalized_character_ghost_clip(clip).endswith(runtime_clip_key)
            ]
            if len(matching_source_clips) != 1:
                raise RestorationError(
                    f"character ghost raw clip join is not unique: "
                    f"{character_class}/{record.skill_id}/{record.clip}/"
                    f"matches={sorted(matching_source_clips)}"
                )
            reference_events = [row["event"] for row in target_contexts]
            try:
                stage, _ = select_stage(
                    action, matching_source_clips[0], reference_events
                )
            except ValueError as error:
                raise RestorationError(
                    f"character ghost stage selection failed: {target_effect_id}: {error}"
                ) from error
            available = sorted(
                target_contexts, key=lambda row: int(row["receiptEventIndex"])
            )
            matched = 0
            for notify in stage.get("notifies", []):
                if notify.get("sourceType") != "TrailGhostEffect":
                    continue
                match = next(
                    (
                        context
                        for context in available
                        if event_matches_notify(context["event"], notify)
                    ),
                    None,
                )
                if match is None:
                    continue
                available.remove(match)
                event = match["event"]
                source_artifact = {
                    "kind": "RAW_ACTION_LOA_NOTIFY_PAYLOAD",
                    "path": logical_path,
                    "rawSha256": source_sha,
                    "sourceReceipt": {
                        "path": match["receiptPath"].relative_to(ROOT).as_posix(),
                        "rawSha256": match["receiptRawSha256"],
                    },
                }
                result.append(
                    _character_ghost_cue_row(
                        context=match,
                        raw_notify_id=str(notify["notifyId"]),
                        source_stage_index=int(stage["stageIndex"]),
                        local_time_seconds=float(notify["localTimeSeconds"]),
                        global_time_seconds=float(event["globalTimeSeconds"]),
                        duration_seconds=float(notify["durationSeconds"]),
                        serialized_payload=notify["serializedPayload"],
                        serialized_labels=list(notify.get("serializedLabels", [])),
                        asset_references=list(notify.get("assetReferences", [])),
                        source_artifact=source_artifact,
                    )
                )
                matched += 1
            if available or matched != len(target_contexts):
                raise RestorationError(
                    f"character ghost notify/event coverage changed: {target_effect_id} "
                    f"matched={matched} expected={len(target_contexts)}"
                )
    return result


def load_character_ghost_cues(
    records: list[CandidateRecord],
) -> list[dict[str, Any]]:
    contexts = _character_ghost_source_contexts(records)
    cues = _dimensionmaster_character_ghost_cues(contexts)
    cues.extend(_extracted_character_ghost_cues(contexts))
    cues.sort(
        key=lambda row: (
            CLASS_ORDER[row["characterClass"]],
            row["skillId"],
            row["stageIndex"],
            row["stageClipIndex"],
            row["sourceReceiptEventIndex"],
            row["rawNotifyId"],
        )
    )
    count_by_class = Counter(row["characterClass"] for row in cues)
    target_count = len({row["targetEffectAssetId"] for row in cues})
    source_event_joins = {
        (row["characterClass"], row["skillId"], row["sourceEventId"])
        for row in cues
    }
    if (
        len(cues) != EXPECTED_CHARACTER_GHOST_CUE_COUNT
        or target_count != EXPECTED_CHARACTER_GHOST_TARGET_COUNT
        or dict(count_by_class) != EXPECTED_CHARACTER_GHOST_CUE_COUNT_BY_CLASS
        or len(source_event_joins) != len(cues)
        or len({row["cueId"] for row in cues}) != len(cues)
    ):
        raise RestorationError(
            "character ghost preservation denominator/identity changed: "
            f"cues={len(cues)} targets={target_count} "
            f"classes={dict(count_by_class)} joins={len(source_event_joins)}"
        )
    if any(row["admitted"] for row in cues):
        raise RestorationError("character ghost cue escaped fail-closed admission")
    return cues


def load_canonical_materials(
    character_class: str,
    source_effect_id: str,
    source_elements: list[dict[str, Any]],
) -> dict[str, dict[str, Any]]:
    """Load an exact existing authored material projection when one is authoritative.

    DimensionMaster's imported source documents intentionally retain only the
    raw material identity.  The matching full authored documents contain
    the already reconstructed finite source profile for the same Element IDs.
    Joining by any display/order heuristic would recreate the original bug, so
    this bridge is enabled only for that class and requires complete Particle
    identity plus exact source-material-path agreement.
    """

    if character_class != "DIMENSIONMASTER":
        return {}
    suffix = ".imported"
    if not source_effect_id.endswith(suffix):
        raise RestorationError(
            f"DimensionMaster imported Effect identity is invalid: {source_effect_id}"
        )
    canonical_effect_id = source_effect_id[: -len(suffix)]
    canonical_path = (
        ROOT / "Data/Effects/Authored" / f"{canonical_effect_id}.effect.json"
    )
    canonical_document = load_json(canonical_path)
    canonical_elements = require_authoring_document(
        canonical_document,
        path=canonical_path,
        effect_id=canonical_effect_id,
        versions={11, 12},
    )
    source_particles = {
        str(element["id"]): element
        for element in source_elements
        if element.get("kind") == "particle"
    }
    canonical_particles = {
        str(element["id"]): element
        for element in canonical_elements
        if element.get("kind") == "particle"
    }
    if set(source_particles) != set(canonical_particles):
        raise RestorationError(
            "DimensionMaster canonical material Element denominator changed: "
            f"{source_effect_id} source={len(source_particles)} "
            f"canonical={len(canonical_particles)}"
        )
    materials: dict[str, dict[str, Any]] = {}
    for element_id, source_element in source_particles.items():
        source_material = source_element.get("material")
        canonical_material = canonical_particles[element_id].get("material")
        if (
            not isinstance(source_material, dict)
            or not isinstance(canonical_material, dict)
            or source_material.get("sourceMaterialPath")
            != canonical_material.get("sourceMaterialPath")
        ):
            raise RestorationError(
                "DimensionMaster canonical material identity drifted: "
                f"{source_effect_id}/{element_id}"
            )
        materials[element_id] = copy.deepcopy(canonical_material)
    return materials


def build_dm_a_canonical_v12_package_alias_authority(
    character_class: str,
    source_effect_id: str,
    source_elements: list[dict[str, Any]],
    canonical_document: dict[str, Any],
    canonical_raw_sha256: str,
    resource_manifest: dict[str, Any],
    sampling_evidence: dict[str, Any],
) -> dict[tuple[str, str, str], dict[str, str]]:
    """Validate the narrow package-qualified asset authority for DM A.

    Five of the six reviewed 2050210 slash layers retain an unqualified UE3
    texture object in their finite profile.  The hash-pinned canonical v12
    document also retains the package-qualified runtime asset ID.  That asset
    ID becomes exact only when the package/object tuple has one exact manifest
    row, one matching texture-sampling row (physical package plus SHA), and a
    physically present DDS.  No object-name suffix search participates here.
    """

    if (
        character_class != "DIMENSIONMASTER"
        or source_effect_id != DM_A_CANONICAL_SOURCE_EFFECT_ID
    ):
        return {}
    if canonical_raw_sha256.casefold() != DM_A_CANONICAL_V12_RAW_SHA256:
        raise RestorationError(
            "DimensionMaster A canonical v12 raw SHA drifted: "
            f"{canonical_raw_sha256}"
        )

    canonical_elements = require_authoring_document(
        canonical_document,
        path=DM_A_CANONICAL_V12_PATH,
        effect_id=DM_A_CANONICAL_EFFECT_ID,
        versions={12},
    )
    canonical_particles = {
        str(element["id"]): element
        for element in canonical_elements
        if element.get("kind") == "particle"
    }
    source_particles = {
        str(element["id"]): element
        for element in source_elements
        if element.get("kind") == "particle"
    }
    if (
        resource_manifest.get("schema")
        != "lostark.class-effect-resource-source-manifest"
        or resource_manifest.get("formatVersion") != 1
        or resource_manifest.get("characterClass") != "DIMENSIONMASTER"
        or not isinstance(resource_manifest.get("assets"), list)
    ):
        raise RestorationError(
            "DimensionMaster A canonical alias resource manifest is invalid"
        )
    if (
        sampling_evidence.get("schema")
        != "lostark.ue3-texture-sampling-evidence"
        or sampling_evidence.get("formatVersion") != 1
        or sampling_evidence.get("characterClass") != "DIMENSIONMASTER"
        or not isinstance(sampling_evidence.get("textures"), list)
    ):
        raise RestorationError(
            "DimensionMaster A canonical alias sampling evidence is invalid"
        )

    manifest_assets = resource_manifest["assets"]
    sampling_rows = sampling_evidence["textures"]
    authority: dict[tuple[str, str, str], dict[str, str]] = {}
    for base_element_id in sorted(DM_A_GOLDEN_SOURCE_ELEMENT_BASE_IDS):
        expected_ids = {
            base_element_id + suffix for suffix in DM_A_GOLDEN_EVENT_SUFFIXES
        }
        canonical_ids = {
            element_id
            for element_id in canonical_particles
            if element_id == base_element_id
            or element_id.startswith(base_element_id + ".event_source-event-")
        }
        source_ids = {
            element_id
            for element_id in source_particles
            if element_id == base_element_id
            or element_id.startswith(base_element_id + ".event_source-event-")
        }
        if canonical_ids != expected_ids or source_ids != expected_ids:
            raise RestorationError(
                "DimensionMaster A golden source occurrence identity drifted: "
                f"{base_element_id}/canonical={sorted(canonical_ids)}/"
                f"source={sorted(source_ids)}"
            )

        base_material = canonical_particles[base_element_id].get("material")
        base_profile = (
            base_material.get("sourceProfile")
            if isinstance(base_material, dict)
            else None
        )
        if (
            not isinstance(base_material, dict)
            or not isinstance(base_profile, dict)
            or base_profile.get("enabled") is not True
        ):
            raise RestorationError(
                "DimensionMaster A golden canonical profile is invalid: "
                f"{base_element_id}"
            )
        base_material_path = str(base_material.get("sourceMaterialPath", ""))
        base_profile_sha256 = canonical_sha256(base_profile)
        if not base_material_path:
            raise RestorationError(
                "DimensionMaster A golden canonical material path is missing: "
                f"{base_element_id}"
            )

        for element_id in sorted(expected_ids):
            canonical_material = canonical_particles[element_id].get("material")
            source_material = source_particles[element_id].get("material")
            profile = (
                canonical_material.get("sourceProfile")
                if isinstance(canonical_material, dict)
                else None
            )
            if (
                not isinstance(canonical_material, dict)
                or not isinstance(source_material, dict)
                or not isinstance(profile, dict)
                or profile.get("enabled") is not True
                or canonical_material.get("sourceMaterialPath")
                != base_material_path
                or source_material.get("sourceMaterialPath")
                != base_material_path
                or canonical_sha256(profile) != base_profile_sha256
            ):
                raise RestorationError(
                    "DimensionMaster A canonical/source material profile drifted "
                    f"across event copies: {element_id}"
                )
            textures = profile.get("textures")
            if not isinstance(textures, list):
                raise RestorationError(
                    "DimensionMaster A canonical profile textures are invalid: "
                    f"{element_id}"
                )
            for texture in textures:
                if not isinstance(texture, dict):
                    raise RestorationError(
                        "DimensionMaster A canonical texture row is invalid: "
                        f"{element_id}"
                    )
                source_object_path = str(texture.get("sourceObjectPath", ""))
                asset_id = str(texture.get("assetId", ""))
                asset_match = DM_CANONICAL_TEXTURE_ASSET_ID.fullmatch(asset_id)
                # Package-qualified source paths are already exact through the
                # normal receipt/manifest join.  This authority is solely for
                # the reviewed unqualified object retained by canonical v12.
                if (
                    not source_object_path
                    or "." in source_object_path
                    or asset_match is None
                ):
                    continue
                logical_package = asset_match.group("logical_package")
                object_name = asset_match.group("object_name")
                source_object_name = source_object_path.rsplit(".", 1)[-1]
                if object_name.casefold() != source_object_name.casefold():
                    raise RestorationError(
                        "DimensionMaster A canonical package/object identity "
                        f"mismatch: {element_id}/{source_object_path}/{asset_id}"
                    )
                qualified_source_path = f"{logical_package}.{object_name}"
                matching_manifest_rows = [
                    row
                    for row in manifest_assets
                    if isinstance(row, dict)
                    and str(row.get("sourceAssetPath", "")).casefold()
                    == qualified_source_path.casefold()
                ]
                if len(matching_manifest_rows) != 1:
                    raise RestorationError(
                        "DimensionMaster A canonical alias exact manifest row "
                        f"is missing/duplicate: {qualified_source_path}/"
                        f"count={len(matching_manifest_rows)}"
                    )
                manifest_row = matching_manifest_rows[0]
                roles = manifest_row.get("roles")
                skill_ids = manifest_row.get("skillIds")
                physical_package = str(
                    manifest_row.get("physicalPackage", "")
                )
                if (
                    str(manifest_row.get("logicalPackage", "")).casefold()
                    != logical_package.casefold()
                    or not isinstance(roles, list)
                    or "texture"
                    not in {str(role).casefold() for role in roles}
                    or not isinstance(skill_ids, list)
                    or 2050210 not in {int(skill_id) for skill_id in skill_ids}
                    or str(manifest_row.get("resolutionStatus", "")).casefold()
                    != "resolved_source_package"
                    or not physical_package
                ):
                    raise RestorationError(
                        "DimensionMaster A canonical alias manifest identity is "
                        f"invalid: {qualified_source_path}"
                    )

                matching_sampling_rows = [
                    row
                    for row in sampling_rows
                    if isinstance(row, dict)
                    and str(row.get("sourceObjectPath", "")).casefold()
                    == qualified_source_path.casefold()
                ]
                if len(matching_sampling_rows) != 1:
                    raise RestorationError(
                        "DimensionMaster A canonical alias exact sampling row "
                        f"is missing/duplicate: {qualified_source_path}/"
                        f"count={len(matching_sampling_rows)}"
                    )
                sampling_row = matching_sampling_rows[0]
                package_key = physical_package.casefold()
                expected_package_sha256 = (
                    DM_A_CANONICAL_ALIAS_PACKAGE_SHA256.get(package_key)
                )
                sampling_package_sha256 = str(
                    sampling_row.get("physicalPackageSha256", "")
                ).casefold()
                if (
                    str(sampling_row.get("physicalPackage", "")).casefold()
                    != package_key
                    or expected_package_sha256 is None
                    or sampling_package_sha256 != expected_package_sha256
                    or any(
                        str(sampling_row.get(field, "")).casefold()
                        != str(texture.get(field, "")).casefold()
                        for field in (
                            "addressU",
                            "addressV",
                            "colorSpace",
                            "samplingEvidence",
                        )
                    )
                ):
                    raise RestorationError(
                        "DimensionMaster A canonical alias sampling package/SHA "
                        f"mismatch: {qualified_source_path}"
                    )
                if not resource_path(asset_id).is_file():
                    raise RestorationError(
                        "DimensionMaster A canonical alias DDS is missing: "
                        f"{asset_id}"
                    )

                texture_name = str(texture.get("name", "")).casefold()
                key = (
                    element_id.casefold(),
                    texture_name,
                    source_object_path.casefold(),
                )
                if not texture_name or key in authority:
                    raise RestorationError(
                        "DimensionMaster A canonical alias texture identity is "
                        f"missing/duplicate: {element_id}/{texture_name}"
                    )
                authority[key] = {
                    "assetId": asset_id,
                    "provenance": CANONICAL_V12_PACKAGE_QUALIFIED_ASSET_ID,
                    "canonicalEffectAssetId": DM_A_CANONICAL_EFFECT_ID,
                    "canonicalRawSha256": DM_A_CANONICAL_V12_RAW_SHA256,
                    "sourceMaterialPath": base_material_path,
                    "profileCanonicalSha256": base_profile_sha256,
                    "qualifiedSourceObjectPath": qualified_source_path,
                    "logicalPackage": logical_package,
                    "objectName": object_name,
                    "physicalPackage": physical_package,
                    "physicalPackageSha256": sampling_package_sha256,
                    "samplingEvidence": str(
                        sampling_row.get("samplingEvidence", "")
                    ),
                }

    if len(authority) != EXPECTED_CANONICAL_V12_PACKAGE_QUALIFIED_ASSET_COUNT:
        raise RestorationError(
            "DimensionMaster A canonical package-qualified alias denominator "
            f"changed: {len(authority)}"
        )
    return authority


def load_dm_a_canonical_v12_package_alias_authority(
    character_class: str,
    source_effect_id: str,
    source_elements: list[dict[str, Any]],
) -> dict[tuple[str, str, str], dict[str, str]]:
    if (
        character_class != "DIMENSIONMASTER"
        or source_effect_id != DM_A_CANONICAL_SOURCE_EFFECT_ID
    ):
        return {}
    return build_dm_a_canonical_v12_package_alias_authority(
        character_class,
        source_effect_id,
        source_elements,
        load_json(DM_A_CANONICAL_V12_PATH),
        raw_sha256(DM_A_CANONICAL_V12_PATH),
        load_json(CLASS_RESOURCE_MANIFESTS["DIMENSIONMASTER"]),
        load_json(DM_A_TEXTURE_SAMPLING_EVIDENCE_PATH),
    )


def load_manifest_texture_index(character_class: str) -> dict[str, str]:
    """Return exact, physically present class-local texture bindings."""

    manifest_path = CLASS_RESOURCE_MANIFESTS.get(character_class)
    if manifest_path is None:
        return {}
    manifest = load_json(manifest_path)
    assets = manifest.get("assets")
    if (
        manifest.get("schema") != "lostark.class-effect-resource-source-manifest"
        or manifest.get("characterClass") != character_class
        or not isinstance(assets, list)
    ):
        raise RestorationError(
            f"class resource manifest identity changed: {manifest_path}"
        )
    class_folder = {
        "ARTIST": "Artist",
        "DIMENSIONMASTER": "DimensionMaster",
        "LANCE_MASTER": "LanceMaster",
    }[character_class]
    nested_package = character_class == "DIMENSIONMASTER"
    result: dict[str, str] = {}
    for row in assets:
        if not isinstance(row, dict):
            raise RestorationError(
                f"class resource manifest row is invalid: {manifest_path}"
            )
        roles = row.get("roles")
        if (
            not isinstance(roles, list)
            or "texture" not in {str(role).casefold() for role in roles}
            or str(row.get("resolutionStatus", "")).casefold()
            != "resolved_source_package"
        ):
            continue
        source_path = str(row.get("sourceAssetPath", "")).casefold()
        logical_package = str(row.get("logicalPackage", "")).upper()
        source_name = str(row.get("sourceAssetPath", "")).rsplit(".", 1)[-1]
        if not source_path or not logical_package or not source_name:
            raise RestorationError(
                f"resolved class texture identity is incomplete: {manifest_path}"
            )
        relative = (
            f"{logical_package}/{source_name}.dds"
            if nested_package
            else f"{source_name}.dds"
        )
        asset_id = f"Effect/{class_folder}/Textures/{relative}"
        existing = result.get(source_path)
        if existing is not None and existing != asset_id:
            raise RestorationError(
                "class resource texture identity is ambiguous: "
                f"{manifest_path}/{source_path}"
            )
        if not resource_path(asset_id).is_file():
            raise RestorationError(
                f"class resource texture is missing: {manifest_path}/{asset_id}"
            )
        result[source_path] = asset_id
    return result


def load_shared_material_contracts() -> dict[tuple[str, str], dict[str, Any]]:
    """Load the exact material/package tuples without occurrence fingerprints."""

    result: dict[tuple[str, str], dict[str, Any]] = {}
    for path in SOURCE_MATERIAL_CONTRACT_PATHS:
        contract = load_json(path)
        identities = contract.get("materialIdentities")
        if (
            contract.get("schema") != "lostark.effect-source-material-contract"
            or contract.get("formatVersion") != 1
            or not isinstance(identities, list)
        ):
            raise RestorationError(
                f"source Material contract identity changed: {path}"
            )
        for identity in identities:
            if not isinstance(identity, dict):
                raise RestorationError(
                    f"source Material contract row is invalid: {path}"
                )
            key = (
                str(identity.get("sourceMaterialPath", "")).casefold(),
                str(identity.get("sourcePhysicalPackage", "")).casefold(),
            )
            if not key[0] or not key[1] or key in result:
                raise RestorationError(
                    f"source Material contract tuple is missing/duplicate: {path}/{key}"
                )
            for required in (
                "profileId",
                "runtimeShaderProfileId",
                "parentMaterialPath",
                "sourceParameters",
                "sourceEvidenceResolved",
                "productAdmissionStatus",
            ):
                if required not in identity:
                    raise RestorationError(
                        f"source Material contract field is missing: {path}/{key}/{required}"
                    )
            source_resolved = identity.get("sourceEvidenceResolved")
            admission = str(identity.get("productAdmissionStatus") or "")
            runtime_profile = str(identity.get("runtimeShaderProfileId") or "")
            expected_admission = (
                "BLOCKED_SOURCE_EVIDENCE"
                if source_resolved is not True
                else (
                    "BLOCKED_FALLBACK_PROFILE"
                    if runtime_profile == FALLBACK_BLOCKED_PROFILE
                    else "ADMITTED_RECONSTRUCTED_PROFILE"
                )
            )
            if not isinstance(source_resolved, bool) or admission != expected_admission:
                raise RestorationError(
                    "source Material contract admission is inconsistent: "
                    f"{path}/{key}/{source_resolved}/{admission}"
                )
            result[key] = copy.deepcopy(identity)
    # 810 compiled + 50 seed identities.  LanceMaster D/F add 34 exact
    # package-qualified identities to the prior 776-identity compiled corpus.
    if len(result) != 860:
        raise RestorationError(
            f"shared source Material contract denominator changed: {len(result)}"
        )
    return result


def load_source_index() -> tuple[
    dict[str, SourceDocument],
    dict[tuple[str, int, int], tuple[str, ...]],
]:
    by_effect: dict[str, SourceDocument] = {}
    stage_effects: dict[tuple[str, int, int], tuple[str, ...]] = {}
    shared_material_contracts = load_shared_material_contracts()
    for character_class, manifest_path in CLASS_MANIFESTS.items():
        manifest_texture_by_source_path = load_manifest_texture_index(
            character_class
        )
        manifest = load_json(manifest_path)
        if (
            manifest.get("schema") != "lostark.combat-effect-source-stage-manifest"
            or manifest.get("characterClass") != character_class
        ):
            raise RestorationError(f"source manifest identity changed: {manifest_path}")
        for raw_skill in manifest.get("skills", []):
            if not isinstance(raw_skill, dict):
                raise RestorationError(f"manifest skill is invalid: {manifest_path}")
            skill_id = int(raw_skill["productSkillId"])
            for raw_stage in raw_skill.get("stages", []):
                if not isinstance(raw_stage, dict):
                    raise RestorationError(f"manifest stage is invalid: {manifest_path}")
                stage_index = int(raw_stage["stageIndex"])
                effect_ids: list[str] = []
                for raw_artifact in raw_stage.get("sourceArtifacts", []):
                    if not isinstance(raw_artifact, dict):
                        raise RestorationError("source artifact must be an object")
                    descriptor = raw_artifact.get("importedDocument")
                    generated_receipt = raw_artifact.get("generatedSourceReceipt")
                    if not isinstance(descriptor, dict) or not isinstance(
                        generated_receipt, dict
                    ):
                        raise RestorationError(
                            "source artifact has no imported document/source receipt"
                        )
                    path = repository_path(str(descriptor["path"]), "imported document")
                    expected_effect_id = str(descriptor["effectAssetId"])
                    document = load_json(path)
                    elements = require_authoring_document(
                        document,
                        path=path,
                        effect_id=expected_effect_id,
                        versions={12},
                    )
                    generated_receipt_path = repository_path(
                        str(generated_receipt["path"]), "generated source receipt"
                    )
                    source_receipt = load_json(generated_receipt_path)
                    timeline = source_receipt.get("timeline")
                    raw_events = (
                        timeline.get("events") if isinstance(timeline, dict) else None
                    )
                    if not isinstance(raw_events, list):
                        raise RestorationError(
                            f"source receipt events are missing: {generated_receipt_path}"
                        )
                    timeline_events: dict[str, dict[str, Any]] = {}
                    first_event_by_system: dict[str, str] = {}
                    for raw_event in raw_events:
                        if not isinstance(raw_event, dict):
                            raise RestorationError(
                                f"source receipt event must be an object: {generated_receipt_path}"
                            )
                        event_id = str(raw_event.get("eventId", ""))
                        if not event_id or event_id in timeline_events:
                            raise RestorationError(
                                f"missing/duplicate source event ID: {generated_receipt_path}"
                            )
                        timeline_events[event_id] = raw_event
                        source_system = str(
                            raw_event.get("sourceSystemId", "")
                        ).casefold()
                        if source_system and str(
                            raw_event.get("resolutionStatus", "")
                        ).startswith("RESOLVED_"):
                            first_event_by_system.setdefault(source_system, event_id)

                    raw_materials = source_receipt.get("materialParameterBindings")
                    if not isinstance(raw_materials, list):
                        raise RestorationError(
                            f"source receipt material bindings are missing: {generated_receipt_path}"
                        )
                    material_resolution_by_path: dict[str, str] = {}
                    material_physical_package_by_path: dict[str, str] = {}
                    for raw_material in raw_materials:
                        if not isinstance(raw_material, dict):
                            raise RestorationError(
                                f"source material binding must be an object: {generated_receipt_path}"
                            )
                        material_path = str(
                            raw_material.get("sourceMaterialPath", "")
                        ).casefold()
                        resolution = str(raw_material.get("resolutionStatus", ""))
                        physical_package = str(
                            raw_material.get("sourcePhysicalPackage", "")
                        ).casefold()
                        if (
                            not material_path
                            or not resolution
                            or material_path in material_resolution_by_path
                        ):
                            raise RestorationError(
                                f"missing/duplicate source material identity: {generated_receipt_path}"
                            )
                        material_resolution_by_path[material_path] = resolution
                        material_physical_package_by_path[material_path] = (
                            physical_package
                        )
                    raw_runtime_bindings = source_receipt.get(
                        "runtimeResourceBindings"
                    )
                    if not isinstance(raw_runtime_bindings, list):
                        raise RestorationError(
                            "source receipt runtime resource bindings are missing: "
                            f"{generated_receipt_path}"
                        )
                    runtime_texture_by_source_path: dict[str, str] = {}
                    for raw_binding in raw_runtime_bindings:
                        if not isinstance(raw_binding, dict):
                            raise RestorationError(
                                "source runtime resource binding must be an object: "
                                f"{generated_receipt_path}"
                            )
                        if (
                            raw_binding.get("role") != "texture"
                            or raw_binding.get("resolutionStatus")
                            != "RESOLVED_RUNTIME_ASSET"
                        ):
                            continue
                        source_object_path = str(
                            raw_binding.get("sourceObjectPath", "")
                        ).casefold()
                        asset_id = str(raw_binding.get("assetId", ""))
                        existing = runtime_texture_by_source_path.get(
                            source_object_path
                        )
                        if (
                            not source_object_path
                            or not asset_id
                            or (existing is not None and existing != asset_id)
                            or not resource_path(asset_id).is_file()
                        ):
                            raise RestorationError(
                                "source runtime texture identity is invalid/ambiguous: "
                                f"{generated_receipt_path}/{source_object_path}"
                            )
                        runtime_texture_by_source_path[source_object_path] = asset_id
                    if expected_effect_id not in by_effect:
                        canonical_materials = load_canonical_materials(
                            character_class, expected_effect_id, elements
                        )
                        canonical_v12_package_aliases = (
                            load_dm_a_canonical_v12_package_alias_authority(
                                character_class, expected_effect_id, elements
                            )
                        )
                        by_effect[expected_effect_id] = SourceDocument(
                            character_class=character_class,
                            path=path,
                            effect_id=expected_effect_id,
                            document=document,
                            elements={str(element["id"]): element for element in elements},
                            canonical_materials=canonical_materials,
                            generated_receipt_path=generated_receipt_path,
                            timeline_events=timeline_events,
                            first_event_by_system=first_event_by_system,
                            material_resolution_by_path=material_resolution_by_path,
                            material_physical_package_by_path=(
                                material_physical_package_by_path
                            ),
                            runtime_texture_by_source_path=(
                                runtime_texture_by_source_path
                            ),
                            manifest_texture_by_source_path=(
                                manifest_texture_by_source_path
                            ),
                            canonical_v12_package_aliases=(
                                canonical_v12_package_aliases
                            ),
                            shared_material_contracts=shared_material_contracts,
                        )
                    elif (
                        by_effect[expected_effect_id].path != path
                        or by_effect[expected_effect_id].generated_receipt_path
                        != generated_receipt_path
                    ):
                        raise RestorationError(
                            "source Effect ID maps to two document/receipt pairs: "
                            f"{expected_effect_id}"
                        )
                    if expected_effect_id not in effect_ids:
                        effect_ids.append(expected_effect_id)
                key = (character_class, skill_id, stage_index)
                if key in stage_effects and stage_effects[key] != tuple(effect_ids):
                    raise RestorationError(f"manifest stage source set drifted: {key}")
                stage_effects[key] = tuple(effect_ids)
    return by_effect, stage_effects


def source_system_from_element_id(element_id: str) -> str:
    marker = element_id.casefold().find(".particle")
    if marker <= 0:
        raise RestorationError(f"source Element has no Particle emitter identity: {element_id}")
    return element_id[:marker].casefold()


def is_targeted_current_combat_skill(raw_skill: dict[str, Any]) -> bool:
    return str(raw_skill.get("materializationLane") or "") == (
        TARGETED_CURRENT_COMBAT_LANE
    )


def stable_particle_element_id(
    *,
    character_class: str,
    skill_id: int,
    source_effect_id: str,
    source_element_id: str,
    source_event_id: str,
    target_effect_id: str,
) -> str:
    identity = "\0".join(
        (
            character_class,
            str(skill_id),
            source_effect_id,
            source_element_id,
            source_event_id,
            target_effect_id,
        )
    )
    return "authored.source-particle." + hashlib.sha256(
        identity.encode("utf-8")
    ).hexdigest()[:24]


def load_source_particle_assignments(
    records: list[CandidateRecord],
    source_by_effect: dict[str, SourceDocument],
) -> tuple[
    dict[str, tuple[SourceParticleAssignment, ...]],
    tuple[SourceParticleExclusion, ...],
]:
    """Join every imported Particle to one exact event/stage/clip target or exclude it."""

    record_by_key = {
        (
            record.character_class,
            record.skill_id,
            record.stage_index,
            record.stage_clip_index,
            record.clip,
        ): record
        for record in records
    }
    assignments: dict[str, list[SourceParticleAssignment]] = {}
    exclusions: list[SourceParticleExclusion] = []
    source_claims: set[tuple[str, str]] = set()
    target_ids: set[str] = set()
    source_particle_count = 0

    def exclude(
        *,
        character_class: str,
        skill_id: int,
        source_effect_id: str,
        source_element_id: str,
        source_event_id: str,
        reason: str,
    ) -> None:
        exclusions.append(
            SourceParticleExclusion(
                character_class=character_class,
                skill_id=skill_id,
                source_effect_id=source_effect_id,
                source_element_id=source_element_id,
                source_event_id=source_event_id,
                reason=reason,
            )
        )

    for character_class, manifest_path in CLASS_MANIFESTS.items():
        manifest = load_json(manifest_path)
        for raw_skill in manifest.get("skills", []):
            if not isinstance(raw_skill, dict):
                raise RestorationError(f"manifest skill is invalid: {manifest_path}")
            if is_targeted_current_combat_skill(raw_skill):
                continue
            skill_id = int(raw_skill["productSkillId"])
            stage_by_event_id: dict[str, dict[str, Any]] = {}
            artifact_by_effect_id: dict[str, dict[str, Any]] = {}
            for raw_stage in raw_skill.get("stages", []):
                if not isinstance(raw_stage, dict):
                    raise RestorationError(f"manifest stage is invalid: {manifest_path}")
                for raw_event_id in raw_stage.get("sourceEventIds", []):
                    event_id = str(raw_event_id)
                    if event_id in stage_by_event_id:
                        raise RestorationError(
                            f"source event belongs to two stages: {character_class}/{skill_id}/{event_id}"
                        )
                    stage_by_event_id[event_id] = raw_stage
                for raw_artifact in raw_stage.get("sourceArtifacts", []):
                    if not isinstance(raw_artifact, dict):
                        raise RestorationError("source artifact must be an object")
                    imported = raw_artifact.get("importedDocument")
                    if not isinstance(imported, dict):
                        raise RestorationError("source artifact has no imported document")
                    effect_id = str(imported.get("effectAssetId", ""))
                    existing_artifact = artifact_by_effect_id.get(effect_id)
                    if existing_artifact is None:
                        artifact_by_effect_id[effect_id] = raw_artifact
                    elif existing_artifact.get("importedDocument") != imported:
                        raise RestorationError(
                            f"source artifact identity drifted across stages: {effect_id}"
                        )

            for source_effect_id in sorted(artifact_by_effect_id):
                source_document = source_by_effect.get(source_effect_id)
                if source_document is None:
                    raise RestorationError(
                        f"source Particle document is not indexed: {source_effect_id}"
                    )
                for source_order, source_element in enumerate(
                    source_document.document.get("elements", [])
                ):
                    if not isinstance(source_element, dict):
                        raise RestorationError(
                            f"source Element must be an object: {source_document.path}"
                        )
                    if source_element.get("kind") != "particle":
                        continue
                    source_particle_count += 1
                    source_element_id = str(source_element.get("id", ""))
                    claim = (source_effect_id, source_element_id)
                    if claim in source_claims:
                        raise RestorationError(
                            f"source Particle was visited twice: {source_effect_id}/{source_element_id}"
                        )
                    source_claims.add(claim)

                    source_presentation = source_element.get("sourcePresentation")
                    explicit_event_id = (
                        str(source_presentation.get("sourceEventId", ""))
                        if isinstance(source_presentation, dict)
                        else ""
                    )
                    source_system = source_system_from_element_id(source_element_id)
                    source_event_id = explicit_event_id or (
                        source_document.first_event_by_system.get(source_system, "")
                    )
                    source_event = source_document.timeline_events.get(source_event_id)
                    raw_stage = stage_by_event_id.get(source_event_id)
                    if not source_event_id or source_event is None or raw_stage is None:
                        exclude(
                            character_class=character_class,
                            skill_id=skill_id,
                            source_effect_id=source_effect_id,
                            source_element_id=source_element_id,
                            source_event_id=source_event_id,
                            reason="NO_EVENT_JOIN",
                        )
                        continue

                    timing = source_element.get("detail", {}).get("timing", {})
                    source_delay = timing.get("startDelaySeconds")
                    if isinstance(source_delay, bool) or not isinstance(
                        source_delay, (int, float)
                    ):
                        raise RestorationError(
                            f"source Particle timing is invalid: {source_effect_id}/{source_element_id}"
                        )
                    stage_start = float(raw_stage["timelineOffsetSeconds"])
                    stage_end = stage_start + float(raw_stage["durationSeconds"])
                    source_delay_value = float(source_delay)
                    if (
                        source_delay_value + 1.0e-6 < stage_start
                        or source_delay_value >= stage_end - 1.0e-6
                    ):
                        exclude(
                            character_class=character_class,
                            skill_id=skill_id,
                            source_effect_id=source_effect_id,
                            source_element_id=source_element_id,
                            source_event_id=source_event_id,
                            reason="OUTSIDE_STAGE_WINDOW_OR_EVENT_SET",
                        )
                        continue

                    clip_matches = [
                        raw_clip
                        for raw_clip in raw_stage.get("clips", [])
                        if int(raw_clip["sequenceIndex"])
                        == int(source_event["clipSequenceIndex"])
                        and str(raw_clip["clip"]) == str(source_event["clip"])
                    ]
                    if len(clip_matches) != 1:
                        raise RestorationError(
                            "source Particle clip join is missing/ambiguous: "
                            f"{source_effect_id}/{source_element_id}/{source_event_id}"
                        )
                    raw_clip = clip_matches[0]
                    record_key = (
                        character_class,
                        skill_id,
                        int(raw_stage["stageIndex"]),
                        int(raw_clip["stageClipIndex"]),
                        str(raw_clip["clip"]),
                    )
                    record = record_by_key.get(record_key)
                    if record is None:
                        exclude(
                            character_class=character_class,
                            skill_id=skill_id,
                            source_effect_id=source_effect_id,
                            source_element_id=source_element_id,
                            source_event_id=source_event_id,
                            reason="NO_CURRENT_101_TARGET",
                        )
                        continue

                    target_element_id = stable_particle_element_id(
                        character_class=character_class,
                        skill_id=skill_id,
                        source_effect_id=source_effect_id,
                        source_element_id=source_element_id,
                        source_event_id=source_event_id,
                        target_effect_id=record.target_effect_id,
                    )
                    if target_element_id in target_ids:
                        raise RestorationError(
                            f"source Particle target ID collision: {target_element_id}"
                        )
                    target_ids.add(target_element_id)
                    assignments.setdefault(record.target_effect_id, []).append(
                        SourceParticleAssignment(
                            character_class=character_class,
                            skill_id=skill_id,
                            source_document=source_document,
                            source_element_id=source_element_id,
                            source_event_id=source_event_id,
                            source_order=source_order,
                            target_element_id=target_element_id,
                            target_effect_id=record.target_effect_id,
                            clip_timeline_offset_seconds=float(
                                raw_clip["timelineOffsetSeconds"]
                            ),
                        )
                    )

    flattened = [row for rows in assignments.values() for row in rows]
    if (
        source_particle_count != EXPECTED_SOURCE_PARTICLE_CORPUS_COUNT
        or len(flattened) != EXPECTED_STRICT_MAPPED_PARTICLE_COUNT
        or len(exclusions) != EXPECTED_EXCLUDED_PARTICLE_COUNT
        or len(source_claims) != source_particle_count
    ):
        raise RestorationError(
            "source Particle denominator changed: "
            f"corpus={source_particle_count} mapped={len(flattened)} "
            f"excluded={len(exclusions)} claims={len(source_claims)}"
        )
    if set(assignments) - {record.target_effect_id for record in records}:
        raise RestorationError("source Particle assignment escaped the 101 targets")
    return (
        {
            target_effect_id: tuple(
                sorted(
                    rows,
                    key=lambda row: (
                        row.source_document.effect_id,
                        row.source_order,
                        row.source_event_id,
                        row.target_element_id,
                    ),
                )
            )
            for target_effect_id, rows in assignments.items()
        },
        tuple(
            sorted(
                exclusions,
                key=lambda row: (
                    CLASS_ORDER[row.character_class],
                    row.skill_id,
                    row.source_effect_id,
                    row.source_element_id,
                    row.reason,
                ),
            )
        ),
    )


def _walk_drawable_decision_candidates(value: Any) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    if isinstance(value, dict):
        if "sourceElementId" in value and "standaloneDrawableResourceDecision" in value:
            result.append(value)
        for child in value.values():
            result.extend(_walk_drawable_decision_candidates(child))
    elif isinstance(value, list):
        for child in value:
            result.extend(_walk_drawable_decision_candidates(child))
    return result


def _resource_pairs(value: Any, label: str) -> tuple[tuple[str, str], ...]:
    if not isinstance(value, list):
        raise RestorationError(f"{label} must be an array")
    pairs: list[tuple[str, str]] = []
    slots: set[str] = set()
    for raw_binding in value:
        if not isinstance(raw_binding, dict):
            raise RestorationError(f"{label} binding must be an object")
        slot_id = raw_binding.get("slotId")
        asset_id = raw_binding.get("assetId")
        if (
            not isinstance(slot_id, str)
            or not slot_id
            or slot_id in slots
            or not isinstance(asset_id, str)
            or not asset_id
        ):
            raise RestorationError(f"invalid/duplicate {label} binding")
        if not resource_path(asset_id).is_file():
            raise RestorationError(f"missing runtime resource: {asset_id}")
        slots.add(slot_id)
        pairs.append((slot_id, asset_id))
    return tuple(pairs)


def load_drawable_resource_decisions(
    source_by_effect: dict[str, SourceDocument],
) -> dict[tuple[str, str], DrawableResourceDecision]:
    """Load only hash-pinned same-source standalone drawable decisions."""

    allowed_decisions = {
        "spriteBaseResourcePreserved",
        "meshElementMaterialBasePreserved",
        "acceptedSourceTextureBaseAlias",
        "acceptedGroupTextureApproximation",
        "acceptedElementTextureApproximation",
    }
    decisions: dict[tuple[str, str], DrawableResourceDecision] = {}
    for receipt_path in sorted(
        APPROXIMATION_RECEIPT_ROOT.rglob("*.approximation-receipt.json")
    ):
        receipt = load_json(receipt_path)
        source_artifact = receipt.get("sourceArtifact")
        imported = (
            source_artifact.get("importedDocument")
            if isinstance(source_artifact, dict)
            else None
        )
        if not isinstance(imported, dict):
            continue
        source_effect_id = str(imported.get("effectAssetId", ""))
        source_document = source_by_effect.get(source_effect_id)
        if source_document is None:
            continue
        for candidate in _walk_drawable_decision_candidates(receipt):
            source_element_id = str(candidate.get("sourceElementId", ""))
            source_element = source_document.elements.get(source_element_id)
            raw_decision = candidate.get("standaloneDrawableResourceDecision")
            if source_element is None or not isinstance(raw_decision, dict):
                raise RestorationError(
                    f"drawable receipt source Element is missing: {receipt_path}/{source_element_id}"
                )
            source_element_sha = candidate.get("sourceElementSha256")
            if (
                not isinstance(source_element_sha, str)
                or canonical_sha256(source_element) != source_element_sha
            ):
                raise RestorationError(
                    "drawable receipt source Element drifted: "
                    f"{receipt_path}/{source_element_id}"
                )
            decision = str(raw_decision.get("decision", ""))
            disposition = str(raw_decision.get("disposition", ""))
            if decision not in allowed_decisions or disposition not in {
                "sourceResourcePreserved",
                "acceptedApproximation",
            }:
                raise RestorationError(
                    f"unsupported drawable receipt decision: {receipt_path}/{decision}/{disposition}"
                )
            source_pairs = _resource_pairs(
                raw_decision.get("sourceResourceBindings"),
                f"drawable receipt source {source_effect_id}/{source_element_id}",
            )
            target_pairs = _resource_pairs(
                raw_decision.get("targetResourceBindings"),
                f"drawable receipt target {source_effect_id}/{source_element_id}",
            )
            imported_pairs = _resource_pairs(
                source_element.get("resources"),
                f"imported source {source_effect_id}/{source_element_id}",
            )
            if source_pairs != imported_pairs:
                raise RestorationError(
                    f"drawable receipt source bindings drifted: {receipt_path}/{source_element_id}"
                )
            target_hash = raw_decision.get("targetResourceBindingsSha256")
            target_json = [
                {"slotId": slot_id, "assetId": asset_id}
                for slot_id, asset_id in target_pairs
            ]
            if (
                not isinstance(target_hash, str)
                or canonical_sha256(target_json) != target_hash
            ):
                raise RestorationError(
                    f"drawable receipt target binding hash drifted: {receipt_path}/{source_element_id}"
                )
            source_by_slot = dict(source_pairs)
            target_by_slot = dict(target_pairs)
            if any(target_by_slot.get(slot_id) != asset_id for slot_id, asset_id in source_pairs):
                raise RestorationError(
                    f"drawable receipt replaced a source lane: {receipt_path}/{source_element_id}"
                )
            supplemental_slots = set(target_by_slot) - set(source_by_slot)
            if supplemental_slots - {"base"}:
                raise RestorationError(
                    f"drawable receipt added a non-Base lane: {receipt_path}/{source_element_id}"
                )
            if supplemental_slots:
                promoted = raw_decision.get("promotedSourceResource")
                if (
                    not isinstance(promoted, dict)
                    or promoted.get("assetId") != target_by_slot["base"]
                    or promoted.get("slotId") == "meshModel"
                ):
                    raise RestorationError(
                        f"drawable receipt Base is not source-proven: {receipt_path}/{source_element_id}"
                    )
                promoted_asset_id = str(promoted["assetId"])
                promoted_sha = promoted.get("resourceSha256")
                if (
                    not isinstance(promoted_sha, str)
                    or raw_sha256(resource_path(promoted_asset_id)) != promoted_sha
                ):
                    raise RestorationError(
                        f"drawable receipt promoted resource drifted: {receipt_path}/{source_element_id}"
                    )
            target_use_model_material = raw_decision.get("targetUseModelMaterial")
            if not isinstance(target_use_model_material, bool):
                raise RestorationError(
                    f"drawable receipt target material flag is invalid: {receipt_path}/{source_element_id}"
                )
            key = (source_effect_id, source_element_id)
            if key in decisions:
                raise RestorationError(f"duplicate drawable receipt decision: {key}")
            decisions[key] = DrawableResourceDecision(
                receipt_path=receipt_path,
                decision=decision,
                disposition=disposition,
                source_resources=source_pairs,
                target_resources=target_pairs,
                target_use_model_material=target_use_model_material,
            )

    if len(decisions) != EXPECTED_DRAWABLE_DECISION_CORPUS_COUNT:
        raise RestorationError(
            "drawable decision denominator changed: "
            f"{len(decisions)} expected={EXPECTED_DRAWABLE_DECISION_CORPUS_COUNT}"
        )
    return decisions


def load_source_decal_assignments(
    records: list[CandidateRecord],
    source_by_effect: dict[str, SourceDocument],
) -> dict[str, tuple[SourceDecalAssignment, ...]]:
    record_by_key = {
        (
            record.character_class,
            record.skill_id,
            record.stage_index,
            record.stage_clip_index,
            record.clip,
        ): record
        for record in records
    }
    assignments: dict[str, list[SourceDecalAssignment]] = {}
    source_claims: dict[tuple[str, str, str], tuple[str, str]] = {}
    target_ids: set[str] = set()

    for character_class, manifest_path in CLASS_MANIFESTS.items():
        manifest = load_json(manifest_path)
        for raw_skill in manifest.get("skills", []):
            if not isinstance(raw_skill, dict):
                raise RestorationError(f"manifest skill is invalid: {manifest_path}")
            if is_targeted_current_combat_skill(raw_skill):
                continue
            skill_id = int(raw_skill["productSkillId"])
            for raw_stage in raw_skill.get("stages", []):
                stage_index = int(raw_stage["stageIndex"])
                stage_start = float(raw_stage["timelineOffsetSeconds"])
                stage_end = stage_start + float(raw_stage["durationSeconds"])
                stage_event_ids = {
                    str(value) for value in raw_stage.get("sourceEventIds", [])
                }
                clips_by_event_key: dict[tuple[int, str], dict[str, Any]] = {}
                for raw_clip in raw_stage.get("clips", []):
                    key = (int(raw_clip["sequenceIndex"]), str(raw_clip["clip"]))
                    if key in clips_by_event_key:
                        raise RestorationError(
                            f"duplicate manifest clip occurrence: {character_class}/{skill_id}/{key}"
                        )
                    clips_by_event_key[key] = raw_clip

                for raw_artifact in raw_stage.get("sourceArtifacts", []):
                    imported = raw_artifact.get("importedDocument")
                    generated_receipt = raw_artifact.get("generatedSourceReceipt")
                    if not isinstance(imported, dict) or not isinstance(
                        generated_receipt, dict
                    ):
                        raise RestorationError(
                            "source Decal join requires imported document and generated receipt"
                        )
                    source_effect_id = str(imported["effectAssetId"])
                    source_document = source_by_effect.get(source_effect_id)
                    if source_document is None:
                        raise RestorationError(
                            f"source Decal document is not indexed: {source_effect_id}"
                        )
                    receipt_path = repository_path(
                        str(generated_receipt["path"]), "generated source receipt"
                    )
                    source_receipt = load_json(receipt_path)
                    timeline = source_receipt.get("timeline")
                    if not isinstance(timeline, dict):
                        raise RestorationError(
                            f"source receipt timeline is missing: {receipt_path}"
                        )
                    raw_events = timeline.get("events")
                    if not isinstance(raw_events, list):
                        raise RestorationError(
                            f"source receipt events are missing: {receipt_path}"
                        )
                    event_by_id: dict[str, dict[str, Any]] = {}
                    first_event_by_system: dict[str, str] = {}
                    for raw_event in raw_events:
                        if not isinstance(raw_event, dict):
                            raise RestorationError("source receipt event must be an object")
                        event_id = str(raw_event.get("eventId", ""))
                        if not event_id or event_id in event_by_id:
                            raise RestorationError(
                                f"missing/duplicate source event ID: {receipt_path}"
                            )
                        event_by_id[event_id] = raw_event
                        source_system = str(
                            raw_event.get("sourceSystemId", "")
                        ).casefold()
                        resolution = str(raw_event.get("resolutionStatus", ""))
                        if source_system and resolution.startswith("RESOLVED_"):
                            first_event_by_system.setdefault(source_system, event_id)

                    for source_element in source_document.elements.values():
                        if source_element.get("kind") != "decal":
                            continue
                        element_id = str(source_element["id"])
                        timing = source_element.get("detail", {}).get("timing", {})
                        source_delay = timing.get("startDelaySeconds")
                        if (
                            isinstance(source_delay, bool)
                            or not isinstance(source_delay, (int, float))
                        ):
                            raise RestorationError(
                                f"source Decal timing is invalid: {source_effect_id}/{element_id}"
                            )
                        source_delay_value = float(source_delay)
                        if (
                            source_delay_value + 1.0e-6 < stage_start
                            or source_delay_value >= stage_end - 1.0e-6
                        ):
                            continue
                        source_presentation = source_element.get(
                            "sourcePresentation", {}
                        )
                        explicit_event_id = (
                            str(source_presentation.get("sourceEventId", ""))
                            if isinstance(source_presentation, dict)
                            else ""
                        )
                        source_system = element_id.casefold().split(
                            ".particle", 1
                        )[0]
                        event_id = explicit_event_id or first_event_by_system.get(
                            source_system, ""
                        )
                        if not event_id or event_id not in event_by_id:
                            raise RestorationError(
                                f"source Decal event join is missing: {source_effect_id}/{element_id}"
                            )
                        if stage_event_ids and event_id not in stage_event_ids:
                            continue
                        event = event_by_id[event_id]
                        event_key = (
                            int(event["clipSequenceIndex"]),
                            str(event["clip"]),
                        )
                        manifest_clip = clips_by_event_key.get(event_key)
                        if manifest_clip is None:
                            raise RestorationError(
                                f"source Decal clip join is missing: {source_effect_id}/{element_id}/{event_key}"
                            )
                        stage_clip_index = int(manifest_clip["stageClipIndex"])
                        record_key = (
                            character_class,
                            skill_id,
                            stage_index,
                            stage_clip_index,
                            str(manifest_clip["clip"]),
                        )
                        record = record_by_key.get(record_key)
                        if record is None:
                            raise RestorationError(
                                f"source Decal target join is missing: {record_key}"
                            )
                        target_digest = hashlib.sha256(
                            f"{source_effect_id}\0{element_id}".encode("utf-8")
                        ).hexdigest()[:24]
                        target_element_id = (
                            "authored.source-decal." + target_digest
                        )
                        claim_key = (
                            character_class,
                            source_effect_id,
                            element_id,
                        )
                        claim_value = (record.target_effect_id, target_element_id)
                        existing_claim = source_claims.get(claim_key)
                        if existing_claim is not None:
                            if existing_claim != claim_value:
                                raise RestorationError(
                                    f"source Decal maps to two targets: {claim_key}"
                                )
                            continue
                        if target_element_id in target_ids:
                            raise RestorationError(
                                f"source Decal target ID collision: {target_element_id}"
                            )
                        source_claims[claim_key] = claim_value
                        target_ids.add(target_element_id)
                        assignments.setdefault(record.target_effect_id, []).append(
                            SourceDecalAssignment(
                                source_document=source_document,
                                source_element_id=element_id,
                                source_event_id=event_id,
                                target_element_id=target_element_id,
                                clip_timeline_offset_seconds=float(
                                    manifest_clip["timelineOffsetSeconds"]
                                ),
                            )
                        )

    flattened = [row for rows in assignments.values() for row in rows]
    base_count = sum(
        any(
            binding.get("slotId") == "base"
            for binding in row.source_document.elements[
                row.source_element_id
            ].get("resources", [])
        )
        for row in flattened
    )
    if (
        len(flattened) != EXPECTED_SOURCE_DECAL_COUNT
        or len(assignments) != 24
        or base_count != EXPECTED_SOURCE_DECAL_BASE_COUNT
        or len(flattened) - base_count
        != EXPECTED_SOURCE_DECAL_MISSING_BASE_COUNT
    ):
        raise RestorationError(
            "source Decal denominator changed: "
            f"rows={len(flattened)} targets={len(assignments)} base={base_count}"
        )
    return {
        target_effect_id: tuple(
            sorted(rows, key=lambda row: row.target_element_id)
        )
        for target_effect_id, rows in assignments.items()
    }


def source_join(
    source_node: Any,
    record: CandidateRecord,
    source_by_effect: dict[str, SourceDocument],
    stage_effects: dict[tuple[str, int, int], tuple[str, ...]],
) -> tuple[SourceDocument, dict[str, Any]]:
    if not isinstance(source_node, str) or not source_node:
        raise RestorationError(
            f"target Element has no sourceNode: {record.target_effect_id}"
        )
    explicit = EXPLICIT_SOURCE_NODE.fullmatch(source_node)
    if explicit:
        effect_id = explicit.group("effect")
        element_id = explicit.group("element")
        source_document = source_by_effect.get(effect_id)
        if source_document is None:
            raise RestorationError(f"source Effect join is missing: {effect_id}")
        source_element = source_document.elements.get(element_id)
        if source_element is None:
            raise RestorationError(
                f"source Element join is missing: {effect_id}/{element_id}"
            )
        return source_document, source_element

    implicit = ELEMENT_ONLY_SOURCE_NODE.fullmatch(source_node)
    if not implicit:
        raise RestorationError(f"unsupported sourceNode contract: {source_node}")
    element_id = implicit.group("element")
    key = (record.character_class, record.skill_id, record.stage_index)
    matches: list[tuple[SourceDocument, dict[str, Any]]] = []
    for effect_id in stage_effects.get(key, ()):
        source_document = source_by_effect[effect_id]
        source_element = source_document.elements.get(element_id)
        if source_element is not None:
            matches.append((source_document, source_element))
    if len(matches) != 1:
        raise RestorationError(
            "implicit source Element join is missing/ambiguous: "
            f"{record.target_effect_id}/{element_id} matches={len(matches)}"
        )
    return matches[0]


def normalized_source_recipe(source_recipe: Any) -> dict[str, Any]:
    if not isinstance(source_recipe, dict) or source_recipe.get("enabled") is not True:
        raise RestorationError("selected source Element has no enabled sourceRecipe")
    staged = copy.deepcopy(source_recipe)
    for field in SOURCE_ONLY_RECIPE_FIELDS:
        staged.pop(field, None)
    # Imported Detail timing already contains schedule + emitter delay.  Keep
    # the source module graph but prevent a second delay in ordinary playback.
    staged["emitterDelaySeconds"] = 0
    modules = staged.get("modules")
    if not isinstance(modules, list) or not modules:
        raise RestorationError("selected sourceRecipe has no modules")
    for module in modules:
        if not isinstance(module, dict):
            raise RestorationError("sourceRecipe module must be an object")
        distributions = module.get("distributions")
        if not isinstance(distributions, list):
            raise RestorationError("sourceRecipe distributions must be an array")
        for distribution in distributions:
            if not isinstance(distribution, dict):
                raise RestorationError("source distribution must be an object")
            for field in SOURCE_ONLY_DISTRIBUTION_FIELDS:
                distribution.pop(field, None)
    return staged


def resources(value: Any, label: str) -> list[dict[str, str]]:
    if not isinstance(value, list):
        raise RestorationError(f"{label} resources must be an array")
    result: list[dict[str, str]] = []
    slots: set[str] = set()
    for binding in value:
        if not isinstance(binding, dict):
            raise RestorationError(f"{label} resource must be an object")
        slot_id = binding.get("slotId")
        asset_id = binding.get("assetId")
        if (
            not isinstance(slot_id, str)
            or not slot_id
            or slot_id in slots
            or not isinstance(asset_id, str)
            or not asset_id
        ):
            raise RestorationError(f"invalid/duplicate {label} resource binding")
        slots.add(slot_id)
        path = resource_path(asset_id)
        if not path.is_file():
            raise RestorationError(f"missing runtime resource: {asset_id}")
        result.append({"slotId": slot_id, "assetId": asset_id})
    return result


def normalize_contract_scalar(row: dict[str, Any]) -> dict[str, Any]:
    result: dict[str, Any] = {
        "name": str(row.get("name") or ""),
        "value": float(row.get("value") or 0.0),
    }
    if row.get("group") is not None:
        result["group"] = str(row.get("group") or "")
    return result


def normalize_contract_texture(row: dict[str, Any]) -> dict[str, Any]:
    result: dict[str, Any] = {
        "name": str(row.get("name") or ""),
        "sourceObjectPath": str(
            row.get("sourceObjectPath") or row.get("texture") or ""
        ),
        "assetId": str(row.get("assetId") or ""),
        "addressU": str(row.get("addressU") or "wrap"),
        "addressV": str(row.get("addressV") or "wrap"),
        "colorSpace": str(row.get("colorSpace") or "linear"),
        "samplingEvidence": str(
            row.get("samplingEvidence") or "legacy_default"
        ),
    }
    if row.get("group") is not None:
        result["group"] = str(row.get("group") or "")
    return result


def normalize_contract_vector(row: dict[str, Any]) -> dict[str, Any]:
    value = row.get("value")
    if isinstance(value, dict):
        value = [
            float(value.get("r", 0.0)),
            float(value.get("g", 0.0)),
            float(value.get("b", 0.0)),
            float(value.get("a", 0.0)),
        ]
    if not isinstance(value, list) or len(value) != 4:
        value = [0.0, 0.0, 0.0, 0.0]
    result: dict[str, Any] = {
        "name": str(row.get("name") or ""),
        "value": [float(component) for component in value],
    }
    if row.get("group") is not None:
        result["group"] = str(row.get("group") or "")
    return result


def normalize_contract_switch(row: dict[str, Any]) -> dict[str, Any]:
    result: dict[str, Any] = {
        "name": str(row.get("name") or ""),
        "value": bool(row.get("value")),
    }
    if row.get("group") is not None:
        result["group"] = str(row.get("group") or "")
    return result


def classify_dynamic_parameter(name: str) -> str:
    value = name.strip().casefold()
    exact = {
        "mask_a_offset": "mask_a_offset",
        "mask_b_offset": "mask_b_offset",
        "mask_a_distort": "mask_a_distort",
        "mask_b_distort": "mask_b_distort",
        "maksa_pan": "mask_a_pan",
        "maska_pan": "mask_a_pan",
        "flow_str": "flow_strength",
        "maksb_pan": "mask_b_pan",
        "maskb_pan": "mask_b_pan",
        "diff_pan": "diffuse_pan",
    }
    if value in exact:
        return exact[value]
    if "dissolve" in value:
        return "dissolve"
    if "pan" in value or "uv" in value:
        return "uv_pan"
    if "rot" in value:
        return "unbound"
    if any(token in value for token in ("opacity", "fade")) or value in {
        "alpha",
        "alpha_str",
        "alpha_strength",
    }:
        return "opacity"
    if any(token in value for token in ("emissive", "bright", "power")):
        return "emissive"
    if "noise" in value or "distort" in value:
        return "distortion"
    if "size" in value or "radius" in value:
        return "radial_size"
    return "unbound"


def exact_dynamic_parameter_channels(
    source_element: dict[str, Any], parameter_name: str
) -> set[int]:
    """Return exact dynamic-parameter channel names without substring guesses."""

    expected = parameter_name.strip().casefold()
    channels: set[int] = set()
    for module in source_element.get("sourceRecipe", {}).get("modules", []):
        if (
            not isinstance(module, dict)
            or str(module.get("className", "")).casefold()
            != "particlemoduleparameterdynamic"
        ):
            continue
        for literal in module.get("literals", []):
            if not isinstance(literal, dict):
                continue
            match = re.fullmatch(
                r"dynamicparams\[(\d+)\]\.paramname",
                str(literal.get("propertyPath", "")).casefold(),
            )
            if (
                match is not None
                and str(literal.get("value") or "").strip().casefold()
                == expected
                and 0 <= int(match.group(1)) < 4
            ):
                channels.add(int(match.group(1)))
    return channels


def source_dynamic_parameter_arithmetic_unavailable(
    source_material_identity: dict[str, Any] | None,
) -> bool:
    """Require an exact child tuple and direct cooked-parent evidence.

    Explicit graph fields take precedence over the frozen forensic bridge so
    a later runtime-exact extraction automatically disables the approximation.
    """

    if not isinstance(source_material_identity, dict):
        return False
    source_path = str(
        source_material_identity.get("sourceMaterialPath") or ""
    ).casefold()
    source_package = str(
        source_material_identity.get("sourcePhysicalPackage") or ""
    ).casefold()
    if not source_path or not source_package:
        return False
    topology = source_material_identity.get("cookedGraphTopologyStatus")
    runtime_exact = source_material_identity.get(
        "cookedGraphRuntimeExactEligible"
    )
    if topology is not None or runtime_exact is not None:
        return topology == "COOKED_PARTIAL" and runtime_exact is False
    parent_identity = (
        str(source_material_identity.get("parentMaterialPath") or "").casefold(),
        str(
            source_material_identity.get("parentSourcePhysicalPackage") or ""
        ).casefold(),
    )
    return parent_identity in COOKED_PARTIAL_DYNAMIC_PARENT_IDENTITIES


def contract_dynamic_parameter_semantics(
    runtime_profile_id: str,
    source_element: dict[str, Any],
    source_material_identity: dict[str, Any] | None = None,
) -> list[str]:
    missile = runtime_profile_id == MISSILETRAIL_PROFILE
    water = runtime_profile_id == WATERTRAIL_PROFILE
    missile_exact = {
        "alpha_pan": "missile_alpha_pan",
        "uv_noise_velue": "missile_noise_strength",
        "uv_noise_value": "missile_noise_strength",
        "noise_velue": "missile_noise_strength",
        "uv_noise_pan": "missile_noise_pan",
        "alpha_dissolve": "missile_dissolve",
        "dissolve": "missile_dissolve",
    }
    water_exact = {
        "alpha_pan": "water_alpha_pan",
        "uv_noise_pan": "water_noise_pan",
        "dissolve": "water_dissolve",
        "alpha_dissolve": "water_dissolve",
        "noise_velue": "water_noise_strength",
        "uv_noise_velue": "water_noise_strength",
        "uv_noise_value": "water_noise_strength",
    }
    result = ["unbound", "unbound", "unbound", "unbound"]
    for module in source_element.get("sourceRecipe", {}).get("modules", []):
        if (
            not isinstance(module, dict)
            or str(module.get("className", "")).casefold()
            != "particlemoduleparameterdynamic"
        ):
            continue
        for literal in module.get("literals", []):
            if not isinstance(literal, dict):
                continue
            match = re.fullmatch(
                r"dynamicparams\[(\d+)\]\.paramname",
                str(literal.get("propertyPath", "")).casefold(),
            )
            if match is None:
                continue
            index = int(match.group(1))
            if 0 <= index < len(result):
                name = str(literal.get("value") or "")
                result[index] = (
                    missile_exact.get(name.casefold(), "unbound")
                    if missile
                    else (
                        water_exact.get(name.casefold(), "unbound")
                        if water
                        else classify_dynamic_parameter(name)
                    )
                )
    if (
        runtime_profile_id == GROUPED_TRANSLUCENT_PROFILE
        and source_dynamic_parameter_arithmetic_unavailable(
            source_material_identity
        )
    ):
        for index in exact_dynamic_parameter_channels(source_element, "dissolve"):
            result[index] = "unbound"
    return result


def contract_subuv_mode(source_element: dict[str, Any]) -> str:
    modules = source_element.get("sourceRecipe", {}).get("modules", [])
    if not any(
        isinstance(module, dict)
        and str(module.get("className", "")).casefold() == "particlemodulesubuv"
        for module in modules
    ):
        return "none"
    literals = {
        str(literal.get("propertyPath", "")).casefold(): literal.get("value")
        for module in modules
        if isinstance(module, dict)
        and str(module.get("className", "")).casefold()
        == "particlemodulerequired"
        for literal in module.get("literals", [])
        if isinstance(literal, dict)
    }
    allow_flip = bool(literals.get("ballowimageflipping"))
    square_flip = bool(literals.get("bsquareimageflipping"))
    random_time = float(literals.get("randomimagetime") or 0.0)
    return (
        "psuvim_linear_blend_random_flip_square"
        if allow_flip and square_flip and random_time > 0.0
        else "psuvim_linear_blend"
    )


def material_contract_profile(
    identity: dict[str, Any],
    source_element: dict[str, Any],
) -> dict[str, Any]:
    parameters = identity.get("sourceParameters")
    if not isinstance(parameters, dict):
        raise RestorationError("source Material contract parameters are invalid")
    runtime_profile_id = str(identity.get("runtimeShaderProfileId") or "")
    return {
        "enabled": True,
        "profileId": str(identity.get("profileId") or ""),
        "runtimeShaderProfileId": runtime_profile_id,
        "parentMaterialPath": str(identity.get("parentMaterialPath") or ""),
        "semanticStatus": "reconstructed_profile",
        "textures": [
            normalize_contract_texture(row)
            for row in parameters.get("textures", [])
            if isinstance(row, dict)
        ],
        "scalars": [
            normalize_contract_scalar(row)
            for row in parameters.get("scalars", [])
            if isinstance(row, dict)
        ],
        "vectors": [
            normalize_contract_vector(row)
            for row in parameters.get("vectors", [])
            if isinstance(row, dict)
        ],
        "staticSwitches": [
            normalize_contract_switch(row)
            for row in parameters.get("staticSwitches", [])
            if isinstance(row, dict)
        ],
        "dynamicParameterSemantics": contract_dynamic_parameter_semantics(
            runtime_profile_id, source_element, identity
        ),
        "subUVMode": contract_subuv_mode(source_element),
    }


def apply_dynamic_parameter_arithmetic_boundary(
    material: dict[str, Any],
    source_material_identity: dict[str, Any] | None,
    source_element: dict[str, Any],
) -> bool:
    """Downgrade only evidence-unclosed grouped ``dissolve`` channels."""

    profile = material.get("sourceProfile")
    if (
        not isinstance(profile, dict)
        or profile.get("enabled") is not True
        or profile.get("runtimeShaderProfileId") != GROUPED_TRANSLUCENT_PROFILE
        or not source_dynamic_parameter_arithmetic_unavailable(
            source_material_identity
        )
    ):
        return False
    channels = exact_dynamic_parameter_channels(source_element, "dissolve")
    if not channels:
        return False
    semantics = contract_dynamic_parameter_semantics(
        GROUPED_TRANSLUCENT_PROFILE,
        source_element,
        source_material_identity,
    )
    if not all(semantics[index] == "unbound" for index in channels):
        raise RestorationError(
            "cooked-partial dynamic parameter arithmetic stayed executable"
        )
    profile["dynamicParameterSemantics"] = semantics
    return True


def exact_admitted_canonical_fallback_profile(
    existing_profile: Any,
    shared_contract: dict[str, Any] | None,
    source_element: dict[str, Any],
) -> tuple[dict[str, Any] | None, bool]:
    """Replace only one identity-exact enabled fallback profile.

    The existing canonical profile is the authored occurrence overlay.  Keep
    its non-compiler fields and annotations on identity-matched parameter
    rows, while the exact contract replaces stale compiler-owned parameter
    membership and the executable runtime profile.  This avoids carrying a
    sibling profile's extra parameters into another Material identity.
    """

    if not isinstance(existing_profile, dict):
        return None, False
    preserved = copy.deepcopy(existing_profile)
    if (
        preserved.get("enabled") is not True
        or preserved.get("runtimeShaderProfileId")
        != FALLBACK_BLOCKED_PROFILE
        or not isinstance(shared_contract, dict)
        or shared_contract.get("sourceEvidenceResolved") is not True
        or shared_contract.get("productAdmissionStatus")
        != "ADMITTED_RECONSTRUCTED_PROFILE"
    ):
        return preserved, False

    existing_profile_id = str(preserved.get("profileId") or "")
    contract_profile_id = str(shared_contract.get("profileId") or "")
    existing_parent = str(preserved.get("parentMaterialPath") or "")
    contract_parent = str(shared_contract.get("parentMaterialPath") or "")
    if (
        not existing_profile_id
        or not contract_profile_id
        or existing_profile_id.casefold() != contract_profile_id.casefold()
        or not existing_parent
        or not contract_parent
        or existing_parent.casefold() != contract_parent.casefold()
    ):
        return preserved, False

    replacement = material_contract_profile(shared_contract, source_element)
    for field in (
        "enabled",
        "profileId",
        "runtimeShaderProfileId",
        "parentMaterialPath",
        "dynamicParameterSemantics",
        "subUVMode",
    ):
        preserved[field] = copy.deepcopy(replacement[field])
    preserved.setdefault(
        "semanticStatus", copy.deepcopy(replacement["semanticStatus"])
    )
    for family in ("textures", "scalars", "vectors", "staticSwitches"):
        authored_rows = (
            preserved.get(family)
            if isinstance(preserved.get(family), list)
            else []
        )
        authored_by_name = {
            str(row.get("name") or "").casefold(): row
            for row in authored_rows
            if isinstance(row, dict) and str(row.get("name") or "")
        }
        merged_rows: list[dict[str, Any]] = []
        for row in replacement[family]:
            name = str(row.get("name") or "").casefold()
            if not name:
                continue
            merged = copy.deepcopy(authored_by_name.get(name, {}))
            merged.update(copy.deepcopy(row))
            merged_rows.append(merged)
        preserved[family] = merged_rows
    return preserved, True


def manifest_texture_asset(
    source_document: SourceDocument, source_object_path: str
) -> tuple[str | None, bool]:
    """Return a manifest asset and whether it relied on object-name guessing."""

    key = source_object_path.casefold()
    exact = source_document.manifest_texture_by_source_path.get(key)
    if exact is not None:
        return exact, False
    object_name = key.rsplit(".", 1)[-1]
    if not object_name:
        return None, False
    candidates = {
        asset_id
        for manifest_path, asset_id in (
            source_document.manifest_texture_by_source_path.items()
        )
        if manifest_path == object_name
        or manifest_path.endswith("." + object_name)
    }
    return (next(iter(candidates)), True) if len(candidates) == 1 else (None, False)


def apply_strict_typed_material_profile(
    material: dict[str, Any],
    shared_contract: dict[str, Any] | None,
    source_element: dict[str, Any],
    target_resources: list[dict[str, str]],
) -> str:
    """Apply one exact family evaluator without changing admission state.

    The predicate intentionally mirrors the native effective-profile seam:
    exact child/parent/profile identity plus the complete expected runtime
    resource tuple.  The compiler-owned profile rows come from the admitted
    shared contract; existing row annotations survive only by stable name.
    """

    profile = material.get("sourceProfile")
    if (
        not isinstance(profile, dict)
        or profile.get("enabled") is not True
        or profile.get("runtimeShaderProfileId")
        != GROUPED_TRANSLUCENT_PROFILE
        or not isinstance(shared_contract, dict)
        or shared_contract.get("sourceEvidenceResolved") is not True
        or shared_contract.get("productAdmissionStatus")
        != "ADMITTED_RECONSTRUCTED_PROFILE"
    ):
        return ""

    bindings = {
        str(row.get("slotId") or ""): str(row.get("assetId") or "")
        for row in target_resources
        if isinstance(row, dict)
    }
    source_path = str(material.get("sourceMaterialPath") or "")
    candidates = (
        (
            STRICT_MISSILETRAIL_IDENTITY,
            STRICT_MISSILETRAIL_RESOURCES,
            MISSILETRAIL_PROFILE,
            MISSILETRAIL_NAMED_TEXTURES,
        ),
        (
            STRICT_WATERTRAIL_IDENTITY,
            STRICT_WATERTRAIL_RESOURCES,
            WATERTRAIL_PROFILE,
            WATERTRAIL_NAMED_TEXTURES,
        ),
    )
    for identity, expected_resources, runtime_profile, texture_names in candidates:
        if (
            source_path != identity["sourceMaterialPath"]
            or str(profile.get("profileId") or "") != identity["profileId"]
            or str(profile.get("parentMaterialPath") or "")
            != identity["parentMaterialPath"]
            or str(shared_contract.get("sourceMaterialPath") or "")
            != identity["sourceMaterialPath"]
            or str(shared_contract.get("profileId") or "")
            != identity["profileId"]
            or str(shared_contract.get("parentMaterialPath") or "")
            != identity["parentMaterialPath"]
            or bindings != expected_resources
        ):
            continue

        replacement = material_contract_profile(shared_contract, source_element)
        replacement["runtimeShaderProfileId"] = runtime_profile
        replacement["dynamicParameterSemantics"] = (
            contract_dynamic_parameter_semantics(
                runtime_profile, source_element, shared_contract
            )
        )
        replacement["textures"] = [
            row
            for row in replacement.get("textures", [])
            if str(row.get("name") or "") in texture_names
        ]
        if {
            str(row.get("name") or "")
            for row in replacement["textures"]
            if isinstance(row, dict) and row.get("assetId")
        } != texture_names:
            raise RestorationError(
                "strict typed material named texture contract is incomplete: "
                f"{source_path}"
            )

        preserved = copy.deepcopy(profile)
        for field in (
            "enabled",
            "profileId",
            "runtimeShaderProfileId",
            "parentMaterialPath",
            "dynamicParameterSemantics",
            "subUVMode",
        ):
            preserved[field] = copy.deepcopy(replacement[field])
        for family in ("textures", "scalars", "vectors", "staticSwitches"):
            authored_rows = (
                preserved.get(family)
                if isinstance(preserved.get(family), list)
                else []
            )
            authored_by_name = {
                str(row.get("name") or "").casefold(): row
                for row in authored_rows
                if isinstance(row, dict) and row.get("name")
            }
            merged_rows: list[dict[str, Any]] = []
            for row in replacement[family]:
                merged = copy.deepcopy(
                    authored_by_name.get(
                        str(row.get("name") or "").casefold(), {}
                    )
                )
                merged.update(copy.deepcopy(row))
                merged_rows.append(merged)
            preserved[family] = merged_rows
        material["sourceProfile"] = preserved
        return runtime_profile
    return ""


def canonical_particle_material(
    source_document: SourceDocument,
    source_element_id: str,
    target_resources: list[dict[str, str]],
) -> tuple[dict[str, Any], dict[str, Any]]:
    """Return compiler-owned material data with exact runtime texture identity.

    DimensionMaster's imported documents predate the finite SourceProfile
    projection, so their material is joined from the matching authored
    document by stable Element ID.  Named texture asset IDs are then rebound
    to the exact source-receipt runtime binding whenever that binding exists.
    Parent-default textures that are absent from the occurrence graph retain
    only a physically present manifest-derived asset ID.
    """

    source_element = source_document.elements[source_element_id]
    source_material = source_element.get("material")
    if not isinstance(source_material, dict):
        raise RestorationError(
            f"source Particle material is missing: {source_document.effect_id}/"
            f"{source_element_id}"
        )
    canonical = source_document.canonical_materials.get(source_element_id)
    material = copy.deepcopy(canonical if canonical is not None else source_material)
    if (
        not isinstance(material, dict)
        or material.get("sourceMaterialPath")
        != source_material.get("sourceMaterialPath")
    ):
        raise RestorationError(
            "canonical Particle material identity drifted: "
            f"{source_document.effect_id}/{source_element_id}"
        )

    material_path = str(material.get("sourceMaterialPath", "")).casefold()
    physical_package = source_document.material_physical_package_by_path.get(
        material_path, ""
    )
    shared_contract = source_document.shared_material_contracts.get(
        (material_path, physical_package)
    )
    shared_contract_resolved = bool(
        shared_contract is not None
        and shared_contract.get("sourceEvidenceResolved") is True
        and shared_contract.get("productAdmissionStatus")
        in {"ADMITTED_RECONSTRUCTED_PROFILE", "BLOCKED_FALLBACK_PROFILE"}
    )
    shared_contract_admitted = bool(
        shared_contract_resolved
        and shared_contract.get("productAdmissionStatus")
        == "ADMITTED_RECONSTRUCTED_PROFILE"
    )
    exact_wrapper_evidence = (
        (shared_contract or {}).get("exactPhysicalWrapperEvidence")
    )
    exact_wrapper_contract_blocked = bool(
        isinstance(exact_wrapper_evidence, dict)
        and not shared_contract_admitted
    )
    source_profile = material.get("sourceProfile")
    shared_profile_joined = False
    if (
        canonical is None
        and shared_contract_resolved
        and not exact_wrapper_contract_blocked
        and (
            not isinstance(source_profile, dict)
            or source_profile.get("enabled") is not True
        )
    ):
        material["sourceProfile"] = material_contract_profile(
            shared_contract, source_element
        )
        shared_profile_joined = True

    canonical_fallback_replaced = False
    source_profile = material.get("sourceProfile")
    replacement_profile, canonical_fallback_replaced = (
        exact_admitted_canonical_fallback_profile(
            source_profile, shared_contract, source_element
        )
    )
    if replacement_profile is not None:
        material["sourceProfile"] = replacement_profile

    occurrence_profile_promoted = False
    source_profile = material.get("sourceProfile")
    fallback_reason = str(
        (shared_contract or {}).get("fallbackBlockedReason") or ""
    )
    if (
        shared_contract_resolved
        and not exact_wrapper_contract_blocked
        and isinstance(source_profile, dict)
        and source_profile.get("enabled") is True
        and source_profile.get("runtimeShaderProfileId")
        == FALLBACK_BLOCKED_PROFILE
        and fallback_reason
        in {
            "UNKNOWN_GROUPED_TRANSPARENT_INPUT",
            "MISSING_GROUPED_TRANSPARENT_RUNTIME_RESOURCE",
        }
    ):
        bindings = {
            str(row.get("slotId") or ""): str(row.get("assetId") or "")
            for row in target_resources
            if isinstance(row, dict)
        }
        safe_base = bool(bindings.get("base")) and not is_unsafe_base_asset(
            bindings["base"]
        )
        if safe_base or bool(bindings.get("mask")) or bool(
            bindings.get("emissive")
        ):
            source_profile["runtimeShaderProfileId"] = (
                GROUPED_TRANSLUCENT_PROFILE
            )
            source_profile["dynamicParameterSemantics"] = (
                contract_dynamic_parameter_semantics(
                    GROUPED_TRANSLUCENT_PROFILE, source_element,
                    shared_contract,
                )
            )
            occurrence_profile_promoted = True

    strict_typed_material_profile = apply_strict_typed_material_profile(
        material, shared_contract, source_element, target_resources
    )

    # Render state belongs to the exact material/package contract, not to the
    # act of grafting a missing SourceProfile.  DimensionMaster already owns a
    # canonical profile for every occurrence, so keeping this inside the graft
    # branch silently lost one-sided policy on exact canonical matches.
    if shared_contract_resolved:
        render_state = shared_contract.get("renderState")
        if isinstance(render_state, dict) and render_state.get("twoSided") is False:
            blend_mode = str(render_state.get("blendMode") or "").casefold()
            if blend_mode == "blend_translucent":
                material["renderProfile"] = "alpha_one_sided_depth_read"
            elif blend_mode == "blend_additive":
                material["renderProfile"] = "additive_one_sided_depth_read"

    exact_count = 0
    rebased_count = 0
    manifest_count = 0
    canonical_v12_package_qualified_count = 0
    canonical_v12_package_qualified_assets: list[dict[str, str]] = []
    non_exact_alias_count = 0
    unresolved_count = 0
    profile = material.get("sourceProfile")
    if isinstance(profile, dict) and profile.get("enabled") is True:
        # An enabled finite profile must be consumed by the source-material
        # renderer.  Leaving the imported effect.standard template in place
        # made readiness PASS while ordinary rendering ignored the recovered
        # profile and reproduced the white/flat legacy appearance.
        material["templateId"] = "effect.source_material"
        textures = profile.get("textures", [])
        if not isinstance(textures, list):
            raise RestorationError(
                "enabled source Material textures are invalid: "
                f"{source_document.effect_id}/{source_element_id}"
            )
        for texture in textures:
            if not isinstance(texture, dict):
                raise RestorationError(
                    "source Material named texture must be an object: "
                    f"{source_document.effect_id}/{source_element_id}"
                )
            source_object_path = str(texture.get("sourceObjectPath", ""))
            asset_id = str(texture.get("assetId", ""))
            source_object_key = source_object_path.casefold()
            exact_asset_id = source_document.runtime_texture_by_source_path.get(
                source_object_key
            )
            if exact_asset_id is not None:
                exact_count += 1
                if asset_id != exact_asset_id:
                    rebased_count += 1
                texture["assetId"] = exact_asset_id
                continue
            canonical_alias = (
                source_document.canonical_v12_package_aliases.get(
                    (
                        source_element_id.casefold(),
                        str(texture.get("name", "")).casefold(),
                        source_object_key,
                    )
                )
            )
            if canonical_alias is not None:
                canonical_asset_id = canonical_alias["assetId"]
                if (
                    material.get("sourceMaterialPath")
                    != canonical_alias["sourceMaterialPath"]
                    or asset_id != canonical_asset_id
                    or not resource_path(canonical_asset_id).is_file()
                ):
                    raise RestorationError(
                        "DimensionMaster A canonical package-qualified alias "
                        "profile drifted during projection: "
                        f"{source_document.effect_id}/{source_element_id}/"
                        f"{texture.get('name', '')}"
                    )
                texture["assetId"] = canonical_asset_id
                exact_count += 1
                canonical_v12_package_qualified_count += 1
                canonical_v12_package_qualified_assets.append(
                    {
                        "textureName": str(texture.get("name", "")),
                        "sourceObjectPath": source_object_path,
                        **copy.deepcopy(canonical_alias),
                    }
                )
                continue
            manifest_asset_id, used_object_name_suffix = manifest_texture_asset(
                source_document, source_object_key
            )
            if manifest_asset_id is not None:
                if asset_id != manifest_asset_id:
                    rebased_count += 1
                texture["assetId"] = manifest_asset_id
                manifest_count += 1
                if used_object_name_suffix:
                    non_exact_alias_count += 1
            else:
                texture["assetId"] = ""
                unresolved_count += 1

        intended_profile = str(
            (shared_contract or {}).get("intendedRuntimeShaderProfileId") or ""
        )
        if (
            profile.get("runtimeShaderProfileId") == FALLBACK_BLOCKED_PROFILE
            and not exact_wrapper_contract_blocked
            and intended_profile
            and fallback_reason
            in {
                "MISSING_CLASS_LOCAL_FINITE_PROFILE_RESOURCE",
                "LOCAL_CRACK_NAMED_TEXTURE_OR_SAMPLING_CONTRACT_INCOMPLETE",
                "MISSING_EXISTING_FINITE_PROFILE_REQUIRED_RUNTIME_RESOURCE",
            }
        ):
            profile["runtimeShaderProfileId"] = intended_profile
            profile["dynamicParameterSemantics"] = (
                contract_dynamic_parameter_semantics(
                    intended_profile, source_element, shared_contract
                )
            )
            renderer_shape = str(
                source_element.get("sourceRecipe", {}).get(
                    "rendererShape", ""
                )
            )
            profile_ready, _, _ = source_profile_readiness(
                material, target_resources, renderer_shape
            )
            if profile_ready:
                occurrence_profile_promoted = True
            else:
                profile["runtimeShaderProfileId"] = FALLBACK_BLOCKED_PROFILE
                profile["dynamicParameterSemantics"] = (
                    contract_dynamic_parameter_semantics(
                        FALLBACK_BLOCKED_PROFILE, source_element,
                        shared_contract,
                    )
                )

    dynamic_parameter_arithmetic_unavailable = (
        apply_dynamic_parameter_arithmetic_boundary(
            material, shared_contract, source_element
        )
    )
    material.pop("execution", None)
    return material, {
        "canonicalMaterialJoined": canonical is not None,
        "sharedMaterialContractMatched": shared_contract is not None,
        "sharedMaterialContractResolved": shared_contract_resolved,
        "sharedMaterialProfileJoined": shared_profile_joined,
        "sharedMaterialCanonicalFallbackReplaced": (
            canonical_fallback_replaced
        ),
        "sharedMaterialOccurrenceProfilePromoted": (
            occurrence_profile_promoted
        ),
        "strictTypedMaterialProfile": strict_typed_material_profile,
        "namedTextureCount": exact_count + manifest_count + unresolved_count,
        "exactNamedTextureCount": exact_count,
        "rebasedNamedTextureCount": rebased_count,
        "manifestNamedTextureCount": manifest_count,
        "canonicalV12PackageQualifiedAssetCount": (
            canonical_v12_package_qualified_count
        ),
        "canonicalV12PackageQualifiedAssets": (
            canonical_v12_package_qualified_assets
        ),
        "nonExactNamedTextureAliasCount": non_exact_alias_count,
        "sourceDynamicParameterArithmeticUnavailable": (
            dynamic_parameter_arithmetic_unavailable
        ),
        "unresolvedNamedTextureCount": unresolved_count,
    }


def is_unsafe_base_asset(asset_id: str) -> bool:
    folded = asset_id.casefold()
    return not asset_id or any(
        token in folded
        for token in ("blankwhite", "normal", "bump", "_n.dds", "_n_")
    )


def named_texture_contract(
    profile: dict[str, Any], required_names: set[str], *, local_crack: bool = False
) -> bool:
    rows = {
        str(row.get("name", "")): row
        for row in profile.get("textures", [])
        if isinstance(row, dict)
    }
    for name in required_names:
        row = rows.get(name)
        if row is None or not row.get("assetId"):
            return False
        if local_crack and (
            not row.get("samplingEvidence")
            or row.get("samplingEvidence") == "legacy_default"
        ):
            return False
    return True


def source_profile_readiness(
    material: dict[str, Any], target_resources: list[dict[str, str]], shape: str
) -> tuple[bool, bool, str]:
    """Mirror the codec/renderer SourceProfile drawable resource contract.

    Returns (profile_ready, profile_owns_drawable, reason).  A disabled profile
    is neutral: ordinary Base/model-material execution may still be valid.
    """

    profile = material.get("sourceProfile")
    if not isinstance(profile, dict) or profile.get("enabled") is not True:
        return True, False, "SOURCE_PROFILE_DISABLED"
    profile_id = str(profile.get("runtimeShaderProfileId", ""))
    if profile_id == FALLBACK_BLOCKED_PROFILE:
        return False, True, "SOURCE_PROFILE_FALLBACK_BLOCKED"

    supported = {
        "effect.ue3.reconstructed-standard.v1",
        "effect.ue3.circle.v1",
        "effect.ue3.dot.v1",
        "effect.ue3.ring.v1",
        "effect.ue3.aura.v1",
        "effect.ue3.one-layer-distortion.v1",
        GROUPED_TRANSLUCENT_PROFILE,
        "effect.ue3.shine.v1",
        BLACKLINE_PROFILE,
        LINEARFLOW_PROFILE,
        "effect.ue3.slice.v1",
        LEGACY_MISSILETRAIL_PROFILE,
        MISSILETRAIL_PROFILE,
        WATERTRAIL_PROFILE,
        LOCAL_CRACK_PROFILE,
        "effect.ue3.procedural-center-glow.v1",
    }
    if profile_id not in supported:
        return False, False, "SOURCE_PROFILE_UNSUPPORTED:" + profile_id

    bindings = {row["slotId"]: row["assetId"] for row in target_resources}
    safe_base = "base" in bindings and not is_unsafe_base_asset(bindings["base"])
    has_noise = "noise" in bindings
    has_mask = "mask" in bindings
    has_emissive = "emissive" in bindings
    has_dissolve = "dissolve" in bindings
    has_mesh = shape == "mesh" and "meshModel" in bindings

    if profile_id == GROUPED_TRANSLUCENT_PROFILE:
        scalar_rows = [
            row for row in profile.get("scalars", []) if isinstance(row, dict)
        ]
        vector_rows = [
            row for row in profile.get("vectors", []) if isinstance(row, dict)
        ]
        parameter_values = [
            (str(row.get("name", "")) + " " + str(row.get("group", ""))).casefold()
            for row in scalar_rows
        ]
        has_alpha_parameter = any(
            any(token in value for token in ("alpha", "mask", "opacity", "density"))
            for value in parameter_values
        )
        has_emissive_parameter = any(
            "emiss" in value for value in parameter_values
        ) or any(
            "emiss" in (
                str(row.get("name", "")) + " " + str(row.get("group", ""))
            ).casefold()
            for row in vector_rows
        )
        ready = (
            (not has_alpha_parameter or safe_base or has_mask or has_dissolve)
            and (not has_emissive_parameter or safe_base or has_emissive)
            and (safe_base or has_mask or has_emissive)
        )
        return ready, ready, (
            "SOURCE_PROFILE_READY"
            if ready
            else "SOURCE_PROFILE_GROUPED_RESOURCE_CONTRACT"
        )

    if profile_id == LINEARFLOW_PROFILE:
        ready = named_texture_contract(profile, LINEARFLOW_NAMED_TEXTURES)
        return ready, ready and has_mesh, (
            "SOURCE_PROFILE_READY"
            if ready
            else "SOURCE_PROFILE_LINEARFLOW_NAMED_TEXTURE_CONTRACT"
        )
    if profile_id == BLACKLINE_PROFILE:
        ready = named_texture_contract(profile, BLACKLINE_NAMED_TEXTURES)
        return ready, ready, (
            "SOURCE_PROFILE_READY"
            if ready
            else "SOURCE_PROFILE_BLACKLINE_RESOURCE_CONTRACT"
        )
    if profile_id == LOCAL_CRACK_PROFILE:
        ready = has_mesh and named_texture_contract(
            profile, LOCAL_CRACK_NAMED_TEXTURES, local_crack=True
        )
        return ready, ready, (
            "SOURCE_PROFILE_READY"
            if ready
            else "SOURCE_PROFILE_LOCAL_CRACK_RESOURCE_CONTRACT"
        )

    if profile_id == "effect.ue3.reconstructed-standard.v1":
        return safe_base, False, (
            "SOURCE_PROFILE_READY"
            if safe_base
            else "SOURCE_PROFILE_RECONSTRUCTED_STANDARD_SAFE_BASE"
        )

    finite_ready = True
    owns_drawable = False
    if profile_id == "effect.ue3.shine.v1":
        finite_ready = safe_base and has_mask
        owns_drawable = True
    elif profile_id == "effect.ue3.aura.v1":
        finite_ready = safe_base and has_noise
        owns_drawable = True
    elif profile_id == "effect.ue3.one-layer-distortion.v1":
        finite_ready = has_noise
        owns_drawable = True
    elif profile_id == "effect.ue3.slice.v1":
        finite_ready = safe_base
        owns_drawable = True
    elif profile_id == LEGACY_MISSILETRAIL_PROFILE:
        finite_ready = (
            safe_base
            and has_noise
            and has_mask
            and has_dissolve
            and has_mesh
        )
        owns_drawable = True
    elif profile_id == MISSILETRAIL_PROFILE:
        finite_ready = (
            safe_base
            and has_noise
            and has_mask
            and has_emissive
            and has_dissolve
            and has_mesh
            and named_texture_contract(profile, MISSILETRAIL_NAMED_TEXTURES)
        )
        owns_drawable = True
    elif profile_id == WATERTRAIL_PROFILE:
        finite_ready = (
            safe_base
            and has_noise
            and has_mesh
            and named_texture_contract(profile, WATERTRAIL_NAMED_TEXTURES)
        )
        owns_drawable = True
    elif profile_id == "effect.ue3.procedural-center-glow.v1":
        owns_drawable = True
    return finite_ready, owns_drawable, (
        "SOURCE_PROFILE_READY"
        if finite_ready
        else "SOURCE_PROFILE_FINITE_RESOURCE_CONTRACT"
    )


def merge_source_resources(
    source: list[dict[str, str]],
    authored_starter: list[dict[str, str]],
) -> tuple[list[dict[str, str]], list[dict[str, str]], int, int]:
    """Make every source lane exact and retain only source-missing starter lanes."""

    source_by_slot = {binding["slotId"]: binding for binding in source}
    starter_by_slot = {
        binding["slotId"]: binding for binding in authored_starter
    }
    starter_slots = {binding["slotId"] for binding in authored_starter}
    conflict_overwrite_count = sum(
        starter_by_slot.get(binding["slotId"]) is not None
        and starter_by_slot[binding["slotId"]] != binding
        for binding in source
    )
    source_added_count = sum(
        binding["slotId"] not in starter_slots for binding in source
    )
    merged: list[dict[str, str]] = []
    supplemental: list[dict[str, str]] = []
    for binding in authored_starter:
        source_binding = source_by_slot.get(binding["slotId"])
        if source_binding is not None:
            merged.append(copy.deepcopy(source_binding))
        else:
            retained = copy.deepcopy(binding)
            merged.append(retained)
            supplemental.append(copy.deepcopy(retained))
    for binding in source:
        if binding["slotId"] not in starter_slots:
            merged.append(copy.deepcopy(binding))

    merged_by_slot = {binding["slotId"]: binding for binding in merged}
    if len(merged_by_slot) != len(merged) or any(
        merged_by_slot.get(binding["slotId"]) != binding for binding in source
    ):
        raise RestorationError("compiler-owned source resource merge lost a binding")
    return merged, supplemental, conflict_overwrite_count, source_added_count


def particle_source_identity(
    element: dict[str, Any],
    record: CandidateRecord,
    source_by_effect: dict[str, SourceDocument],
    stage_effects: dict[tuple[str, int, int], tuple[str, ...]],
) -> tuple[str, str]:
    source_document, source_element = source_join(
        element.get("sourceNode"), record, source_by_effect, stage_effects
    )
    return source_document.effect_id, str(source_element["id"])


def source_type_data_mesh_rotation_degrees(
    source_recipe: dict[str, Any],
) -> list[int | float]:
    """Project the one UE3 TypeDataMesh rotation in [roll,pitch,yaw] order."""

    if not isinstance(source_recipe, dict) or source_recipe.get(
        "rendererShape"
    ) != "mesh":
        raise RestorationError("TypeDataMesh rotation requires a Mesh recipe")
    modules = source_recipe.get("modules")
    if not isinstance(modules, list):
        raise RestorationError("Mesh recipe modules are invalid")
    type_data = [
        module
        for module in modules
        if isinstance(module, dict)
        and normalized_module_class(str(module.get("className") or ""))
        == "particlemoduletypedatamesh"
    ]
    if len(type_data) != 1:
        raise RestorationError(
            "Mesh recipe must contain exactly one TypeDataMesh module"
        )
    literals = type_data[0].get("literals")
    if not isinstance(literals, list):
        raise RestorationError("TypeDataMesh literals are invalid")
    result: list[int | float] = []
    for axis in ("roll", "pitch", "yaw"):
        matches = [
            literal
            for literal in literals
            if isinstance(literal, dict)
            and str(literal.get("propertyPath") or "").casefold() == axis
        ]
        if len(matches) > 1:
            raise RestorationError(f"duplicate TypeDataMesh {axis} literal")
        if not matches:
            result.append(0.0)
            continue
        literal = matches[0]
        value = literal.get("value")
        if (
            literal.get("kind") != "number"
            or isinstance(value, bool)
            or not isinstance(value, (int, float))
            or not math.isfinite(float(value))
        ):
            raise RestorationError(f"TypeDataMesh {axis} must be finite number")
        result.append(value)
    return result


def is_warlord_17090_chain_source(
    assignment: SourceParticleAssignment,
    source_element: dict[str, Any],
    source_resources: list[dict[str, str]],
) -> bool:
    if (
        assignment.target_effect_id != WARLORD_17090_TARGET_EFFECT_ID
        or source_element.get("sourceRecipe", {}).get("rendererShape") != "mesh"
        or source_element.get("material", {}).get("sourceMaterialPath")
        != WARLORD_CHAIN_SOURCE_MATERIAL_PATH
    ):
        return False
    bindings = {
        str(row.get("slotId") or ""): str(row.get("assetId") or "")
        for row in source_resources
        if isinstance(row, dict)
    }
    return (
        set(bindings) == {"meshModel"}
        and bindings["meshModel"] in WARLORD_CHAIN_MODEL_ASSET_IDS
    )


def apply_warlord_chain_preview_boundary(
    material: dict[str, Any], detail: dict[str, Any],
    target_resources: list[dict[str, str]],
) -> None:
    mesh = detail.get("mesh")
    if not isinstance(mesh, dict):
        raise RestorationError("Warlord chain Mesh Detail is missing")
    profile = material.get("sourceProfile")
    if (
        strict_effect_resource_kind(
            WARLORD_CHAIN_PREVIEW_BASE_ALIAS_ASSET_ID,
            "Warlord chain non-exact preview Base",
        )
        != "texture"
    ):
        raise RestorationError(
            "Warlord chain non-exact preview Base is not a texture"
        )
    if (
        material.get("templateId") != "effect.source_material"
        or material.get("sourceMaterialPath")
        != WARLORD_CHAIN_SOURCE_MATERIAL_PATH
        or not isinstance(profile, dict)
        or profile.get("enabled") is not True
        or profile.get("profileId") != WARLORD_CHAIN_PROFILE_ID
        or profile.get("parentMaterialPath")
        != WARLORD_CHAIN_PARENT_MATERIAL_PATH
    ):
        raise RestorationError(
            "Warlord chain non-exact preview source profile is invalid"
        )
    mesh_rows = [
        row for row in target_resources if row.get("slotId") == "meshModel"
    ]
    base_rows = [
        row for row in target_resources if row.get("slotId") == "base"
    ]
    unsupported_rows = [
        row for row in target_resources
        if row.get("slotId") not in {"meshModel", "base"}
    ]
    if (
        len(mesh_rows) != 1
        or mesh_rows[0].get("assetId") not in WARLORD_CHAIN_MODEL_ASSET_IDS
        or len(base_rows) > 1
        or (
            base_rows
            and base_rows[0].get("assetId")
            != WARLORD_CHAIN_PREVIEW_BASE_ALIAS_ASSET_ID
        )
        or unsupported_rows
    ):
        raise RestorationError(
            "Warlord chain preview resource identity/cardinality is invalid"
        )
    target_resources[:] = [
        copy.deepcopy(mesh_rows[0]),
        {
            "slotId": "base",
            "assetId": WARLORD_CHAIN_PREVIEW_BASE_ALIAS_ASSET_ID,
        },
    ]
    # The exact parent is MASKED/two-sided/WPO, but its cooked arithmetic and
    # texture-role graph are absent.  Keep the source identity and parameters,
    # then deliberately select the existing grouped evaluator as an editable
    # Approximate preview.  This is not a Full promotion: the typed reason and
    # authoringApproximate execution marker remain authoritative, while the
    # physical same-group DDS gives the artist a real Base lane to replace.
    profile["runtimeShaderProfileId"] = GROUPED_TRANSLUCENT_PROFILE
    profile["productAdmissionStatus"] = "AUTHORING_APPROXIMATE"
    # Both converted berchain WModels have WMA2 rows with empty texture lanes.
    # Keep a physical, explicitly non-exact DDS baseline so the Tool can draw
    # and tune the approximate carrier without entering the white fallback.
    # The enabled grouped profile remains compiler-owned; the unavailable
    # masked/WPO arithmetic is carried by the typed approximation reason below.
    mesh["useModelMaterial"] = False
    material.pop("execution", None)


def reassert_warlord_mesh_compiler_fields(
    detail: dict[str, Any], rotation: list[int | float], chain_preview: bool
) -> bool:
    mesh = detail.get("mesh")
    if not isinstance(mesh, dict):
        raise RestorationError("stable Warlord Mesh Detail is missing")
    changed = mesh.get("sourceTypeDataRotationDegrees") != rotation
    mesh["sourceTypeDataRotationDegrees"] = copy.deepcopy(rotation)
    if chain_preview:
        changed = changed or mesh.get("useModelMaterial") is not False
        mesh["useModelMaterial"] = False
    return changed


def canonical_particle_detail(
    source_element: dict[str, Any],
    assignment: SourceParticleAssignment,
    drawable_decision: DrawableResourceDecision | None,
) -> dict[str, Any]:
    detail = copy.deepcopy(source_element.get("detail"))
    if not isinstance(detail, dict):
        raise RestorationError(
            f"source Particle Detail is invalid: {assignment.source_document.effect_id}/"
            f"{assignment.source_element_id}"
        )
    timing = detail.get("timing")
    if not isinstance(timing, dict):
        raise RestorationError(
            f"source Particle timing is missing: {assignment.source_document.effect_id}/"
            f"{assignment.source_element_id}"
        )
    source_delay = timing.get("startDelaySeconds")
    if isinstance(source_delay, bool) or not isinstance(source_delay, (int, float)):
        raise RestorationError(
            f"source Particle timing is invalid: {assignment.source_document.effect_id}/"
            f"{assignment.source_element_id}"
        )
    local_delay = float(source_delay) - assignment.clip_timeline_offset_seconds
    if local_delay < -1.0e-6:
        raise RestorationError(
            f"source Particle precedes its target clip: {assignment.target_effect_id}/"
            f"{assignment.source_element_id}"
        )
    timing["startDelaySeconds"] = max(0.0, local_delay)
    if drawable_decision is not None:
        mesh = detail.get("mesh")
        if not isinstance(mesh, dict):
            raise RestorationError(
                f"source Particle mesh Detail is missing: {assignment.source_element_id}"
            )
        mesh["useModelMaterial"] = drawable_decision.target_use_model_material
    if source_element.get("sourceRecipe", {}).get("rendererShape") == "mesh":
        mesh = detail.get("mesh")
        if not isinstance(mesh, dict):
            raise RestorationError(
                f"source Mesh Detail is missing: {assignment.source_element_id}"
            )
        # Imported UE3 WModels are glTF-scaled by 100.  The registry contract
        # consumes that basis exactly once at the renderer, never by baking
        # legacy Size/MeshRotation into the Element Transform.
        mesh["modelPreScale"] = 0.01
        if assignment.target_effect_id == WARLORD_17090_TARGET_EFFECT_ID:
            mesh["sourceTypeDataRotationDegrees"] = (
                source_type_data_mesh_rotation_degrees(
                    source_element["sourceRecipe"]
                )
            )
    return detail


def canonical_particle_attachment(
    source_element: dict[str, Any], assignment: SourceParticleAssignment
) -> dict[str, Any]:
    attachment = copy.deepcopy(source_element.get("actionCueAttachment"))
    if not isinstance(attachment, dict):
        raise RestorationError(
            f"source Particle attachment is invalid: {assignment.source_document.effect_id}/"
            f"{assignment.source_element_id}"
        )
    attachment.setdefault("snapshotRootSourceBasisYawDegrees", 0.0)
    return attachment


def materialize_source_particle(
    assignment: SourceParticleAssignment,
    drawable_decision: DrawableResourceDecision | None,
    stable_current_element: dict[str, Any] | None,
) -> tuple[dict[str, Any], dict[str, Any]]:
    source_document = assignment.source_document
    source_element = source_document.elements[assignment.source_element_id]
    source_resources = resources(
        source_element.get("resources"),
        f"{source_document.effect_id}/{assignment.source_element_id}",
    )
    warlord_chain_preview = is_warlord_17090_chain_source(
        assignment, source_element, source_resources
    )
    effective_drawable_decision = (
        None if warlord_chain_preview else drawable_decision
    )
    if effective_drawable_decision is None:
        target_resources = copy.deepcopy(source_resources)
    else:
        source_pairs = tuple(
            (binding["slotId"], binding["assetId"]) for binding in source_resources
        )
        if source_pairs != effective_drawable_decision.source_resources:
            raise RestorationError(
                f"drawable decision/source resource drift: {source_document.effect_id}/"
                f"{assignment.source_element_id}"
            )
        target_resources = [
            {"slotId": slot_id, "assetId": asset_id}
            for slot_id, asset_id in effective_drawable_decision.target_resources
        ]

    normalized_recipe = normalized_source_recipe(source_element.get("sourceRecipe"))
    shape = normalized_recipe.get("rendererShape")
    if shape not in {"mesh", "sprite"}:
        raise RestorationError(
            f"strict Particle is not Mesh/Sprite: {source_document.effect_id}/"
            f"{assignment.source_element_id}/{shape}"
        )
    mesh_bindings = [
        binding for binding in target_resources if binding["slotId"] == "meshModel"
    ]
    source_mesh_bindings = [
        binding for binding in source_resources if binding["slotId"] == "meshModel"
    ]
    mesh_resource_valid = len(mesh_bindings) == (1 if shape == "mesh" else 0)
    if shape == "mesh" and mesh_bindings != source_mesh_bindings:
        raise RestorationError(
            f"WModel source identity drift: {assignment.target_effect_id}/"
            f"{assignment.source_element_id}"
        )

    material, material_projection = canonical_particle_material(
        source_document, assignment.source_element_id, target_resources
    )
    source_material_path = material.get("sourceMaterialPath")
    if not isinstance(source_material_path, str) or not source_material_path:
        raise RestorationError(
            f"source Particle material identity is invalid: {source_document.effect_id}/"
            f"{assignment.source_element_id}"
        )
    material_resolution = source_document.material_resolution_by_path.get(
        source_material_path.casefold(), "MISSING_MATERIAL_RECEIPT"
    )
    engine_builtin_material = source_material_path.casefold().startswith(
        "enginematerials."
    )

    canonical_detail = canonical_particle_detail(
        source_element, assignment, effective_drawable_decision
    )
    if warlord_chain_preview:
        apply_warlord_chain_preview_boundary(
            material, canonical_detail, target_resources
        )
    canonical_use_model_material = bool(
        canonical_detail.get("mesh", {}).get("useModelMaterial", False)
    )

    staged = copy.deepcopy(source_element)
    staged["id"] = assignment.target_element_id
    staged["displayName"] = (
        f"{assignment.source_event_id} "
        f"{assignment.source_element_id.rsplit('.', 1)[-1]}"
    )
    staged["groupId"] = "authored.source-particle"
    staged["sourceNode"] = (
        f"authored-source-particle:{assignment.target_effect_id}|source:"
        f"{source_document.effect_id}|element:{assignment.source_element_id}"
    )
    staged["kind"] = "particle"
    staged["detail"] = canonical_detail
    staged["actionCueAttachment"] = canonical_particle_attachment(
        source_element, assignment
    )
    staged["transformInheritance"] = copy.deepcopy(
        source_element.get(
            "transformInheritance", {"enabled": False, "masterElementId": ""}
        )
    )
    staged["resources"] = target_resources
    staged["material"] = material
    staged["sourceRecipe"] = normalized_recipe
    source_presentation = copy.deepcopy(
        source_element.get("sourcePresentation", reset_source_presentation())
    )
    if not isinstance(source_presentation, dict):
        raise RestorationError(
            f"source Particle presentation is invalid: {assignment.source_element_id}"
        )
    source_presentation["sourceEventId"] = assignment.source_event_id
    staged["sourcePresentation"] = source_presentation
    # Imported/compiler input must never smuggle an artist delta forward. A
    # stable current element is the only authority for authoringOverrides.
    staged.pop("authoringOverrides", None)

    stable_reimport = False
    if stable_current_element is not None:
        if (
            stable_current_element.get("id") != assignment.target_element_id
            or stable_current_element.get("kind") != "particle"
        ):
            raise RestorationError(
                f"stable Particle identity changed: {assignment.target_effect_id}/"
                f"{assignment.target_element_id}"
            )
        if (
            not isinstance(stable_current_element.get("displayName"), str)
            or not stable_current_element["displayName"]
            or not isinstance(stable_current_element.get("groupId"), str)
        ):
            raise RestorationError(
                f"stable Particle artist identity fields are invalid: "
                f"{assignment.target_element_id}"
            )
        for artist_field in (
            "displayName",
            "groupId",
            "detail",
            "actionCueAttachment",
            "transformInheritance",
        ):
            if artist_field not in stable_current_element:
                raise RestorationError(
                    f"stable Particle artist field is missing: {assignment.target_element_id}/"
                    f"{artist_field}"
                )
            if stable_current_element[artist_field] != staged[artist_field]:
                stable_reimport = True
                staged[artist_field] = copy.deepcopy(
                    stable_current_element[artist_field]
                )
        if not isinstance(stable_current_element.get("visible"), bool):
            raise RestorationError(
                f"stable Particle Visible is invalid: {assignment.target_element_id}"
            )
        if stable_current_element["visible"] != staged.get("visible"):
            stable_reimport = True
            staged["visible"] = stable_current_element["visible"]
        if (
            assignment.target_effect_id == WARLORD_17090_TARGET_EFFECT_ID
            and shape == "mesh"
        ):
            canonical_rotation = canonical_detail["mesh"][
                "sourceTypeDataRotationDegrees"
            ]
            stable_reimport = reassert_warlord_mesh_compiler_fields(
                staged["detail"], canonical_rotation, warlord_chain_preview
            ) or stable_reimport
    portable = False
    deferred_reason = ""
    try:
        portable_value = portable_recipe(source_element["sourceRecipe"])
        if canonical_sha256(portable_value) != canonical_sha256(normalized_recipe):
            raise RestorationError(
                f"portable normalization drift: {assignment.target_effect_id}/"
                f"{assignment.source_element_id}"
            )
        portable = True
    except PortableRecipeError as error:
        deferred_reason = str(error)

    base_assets = [
        binding["assetId"]
        for binding in target_resources
        if binding["slotId"] == "base"
    ]
    has_safe_base = len(base_assets) == 1 and not is_unsafe_base_asset(
        base_assets[0]
    )
    profile_ready, profile_owns_drawable, profile_reason = (
        source_profile_readiness(material, target_resources, shape)
    )
    source_profile = material.get("sourceProfile")
    source_profile_enabled = (
        isinstance(source_profile, dict)
        and source_profile.get("enabled") is True
    )
    requires_exact_material = not (
        shape == "mesh" and canonical_use_model_material
    )
    source_material_identity_ready = (
        material_resolution == EXACT_SOURCE_MATERIAL_STATUS
        or bool(material_projection["canonicalMaterialJoined"])
        or bool(material_projection["sharedMaterialContractResolved"])
    )
    exact_material_ready = (
        not requires_exact_material
        or (
            source_material_identity_ready
            and source_profile_enabled
            and material.get("templateId") == "effect.source_material"
        )
    )
    if shape == "mesh":
        drawable_contract_ready = (
            mesh_resource_valid
            and (
                canonical_use_model_material
                or has_safe_base
                or profile_owns_drawable
                or material.get("templateId") == "effect.source_material"
            )
        )
    else:
        drawable_contract_ready = has_safe_base or (
            material.get("templateId") == "effect.source_material"
            and source_profile_enabled
        )
    has_safe_drawable = (
        drawable_contract_ready and profile_ready and exact_material_ready
    )
    fail_closed_reasons: list[str] = []
    if not portable:
        fail_closed_reasons.append("UNSUPPORTED_ORDINARY_RECIPE:" + deferred_reason)
    if engine_builtin_material:
        fail_closed_reasons.append(
            "SOURCE_MATERIAL_ENGINE_BUILTIN_POLICY_NOT_AUTHORED"
        )
    if not mesh_resource_valid:
        fail_closed_reasons.append(
            "MISSING_EXACT_MESH_MODEL"
            if shape == "mesh" and not mesh_bindings
            else "INVALID_EXACT_MESH_MODEL_CARDINALITY"
        )
    if not profile_ready:
        fail_closed_reasons.append(profile_reason)
    if material_projection["nonExactNamedTextureAliasCount"]:
        fail_closed_reasons.append("NON_EXACT_NAMED_TEXTURE_ALIAS")
    if material_projection["sourceDynamicParameterArithmeticUnavailable"]:
        fail_closed_reasons.append(
            SOURCE_DYNAMIC_PARAMETER_ARITHMETIC_UNAVAILABLE
        )
    if not drawable_contract_ready:
        fail_closed_reasons.append("MISSING_EXECUTABLE_DRAWABLE_CONTRACT")
    if requires_exact_material and not source_profile_enabled:
        fail_closed_reasons.append("SOURCE_PROFILE_NOT_COMPILED")
    elif requires_exact_material and not exact_material_ready:
        fail_closed_reasons.append(
            "NON_EXACT_SOURCE_MATERIAL:" + material_resolution
        )
    if warlord_chain_preview:
        fail_closed_reasons = [
            reason
            for reason in fail_closed_reasons
            if reason != "SOURCE_PROFILE_NOT_COMPILED"
        ]
        fail_closed_reasons.append(WARLORD_CHAIN_APPROXIMATION_REASON)

    source_visible = source_element.get("visible")
    if not isinstance(source_visible, bool):
        raise RestorationError(
            f"source Particle Visible is invalid: {source_document.effect_id}/"
            f"{assignment.source_element_id}"
        )
    if stable_current_element is None:
        staged["visible"] = source_visible
    authoring_approximate = (
        warlord_chain_preview
        and fail_closed_reasons == [WARLORD_CHAIN_APPROXIMATION_REASON]
    ) or authoring_approximate_admitted(
        fail_closed_reasons, shape, staged.get("resources") or [],
        bool(source_profile_enabled),
        str(
            source_profile.get("runtimeShaderProfileId")
            if isinstance(source_profile, dict)
            else ""
        ),
    )
    if fail_closed_reasons and authoring_approximate:
        # Keep the source Visible value and the real source material template
        # so the carrier previews with its own DDS/WModel.  Product admission
        # is refused by the publisher and the runtime catalog.
        staged["material"]["execution"] = copy.deepcopy(
            AUTHORING_APPROXIMATE_EXECUTION
        )
    elif fail_closed_reasons:
        staged["visible"] = False
        if not source_profile_enabled:
            # `effect.source_material` is an executable renderer template and
            # therefore requires an enabled, validated profile even for a
            # hidden row.  Evidence-only particles without a compiled profile
            # stay durable through sourceMaterialPath/receipt fields, while a
            # fail-closed standard material keeps the ordinary v13 document
            # parseable without creating a white/flat fallback draw.
            staged["material"]["templateId"] = "effect.standard"
        staged["material"]["execution"] = copy.deepcopy(FAIL_CLOSED_EXECUTION)
    else:
        staged["material"].pop("execution", None)

    authoring_override_drops: list[dict[str, str]] = []
    if stable_current_element is not None:
        staged, authoring_override_drops = reapply_authoring_overrides(
            staged, stable_current_element
        )
        if "authoringOverrides" in staged:
            stable_reimport = True

    receipt_row = {
        "targetElementId": assignment.target_element_id,
        "sourceEffectAssetId": source_document.effect_id,
        "sourceElementId": assignment.source_element_id,
        "sourceEventId": assignment.source_event_id,
        "sourceOrder": assignment.source_order,
        "sourceRecipeCanonicalSha256": canonical_sha256(
            source_element["sourceRecipe"]
        ),
        "normalizedRecipeCanonicalSha256": canonical_sha256(normalized_recipe),
        "sourceDetailCanonicalSha256": canonical_sha256(source_element["detail"]),
        "targetDetailCanonicalSha256": canonical_sha256(staged["detail"]),
        "sourceBindings": source_resources,
        "targetBindings": target_resources,
        "effectiveBindings": copy.deepcopy(staged["resources"]),
        "drawableDecision": (
            drawable_decision.decision if drawable_decision is not None else "NONE"
        ),
        "drawableDecisionReceipt": (
            drawable_decision.receipt_path.relative_to(ROOT).as_posix()
            if drawable_decision is not None
            else None
        ),
        "drawableDecisionSuppressedReason": (
            WARLORD_CHAIN_APPROXIMATION_REASON if warlord_chain_preview else ""
        ),
        "materialResolutionStatus": material_resolution,
        "materialTemplateId": material.get("templateId"),
        "runtimeShaderProfileId": (
            source_profile.get("runtimeShaderProfileId")
            if isinstance(source_profile, dict) and source_profile_enabled
            else ""
        ),
        "sourceProfileReady": profile_ready,
        "sourceProfileReadinessReason": profile_reason,
        "sourceProfileOwnsDrawable": profile_owns_drawable,
        **material_projection,
        "rendererShape": shape,
        "portable": portable,
        "deferredReason": deferred_reason,
        "hasSafeBase": has_safe_base,
        "hasSafeDrawable": has_safe_drawable,
        "canonicalUseModelMaterial": canonical_use_model_material,
        "meshResourceValid": mesh_resource_valid,
        "failClosedReasons": fail_closed_reasons,
        "authoringApproximate": authoring_approximate,
        "hardPortableFailClosed": bool(
            portable and fail_closed_reasons and not authoring_approximate
        ),
        "previewTarget": bool(
            portable and (not fail_closed_reasons or authoring_approximate)
        ),
        "stableReimportPreserved": stable_reimport,
        "authoringOverrideDrops": authoring_override_drops,
    }
    return staged, receipt_row


def materialize_document_model_cues(
    record: CandidateRecord,
    current: dict[str, Any],
    staged: dict[str, Any],
) -> list[dict[str, Any]]:
    """Preserve every ModelCue and pin the exact summon source mask contract."""

    current_model_cues = current.get("modelCues")
    if current_model_cues is None:
        return []
    if not isinstance(current_model_cues, list):
        raise RestorationError(
            f"document modelCues must be an array: {record.target_effect_id}"
        )
    staged_model_cues = copy.deepcopy(current_model_cues)
    if record.target_effect_id != DIMENSION_SUMMON_MODEL_CUE_TARGET:
        staged["modelCues"] = staged_model_cues
        return []

    matches = [
        cue
        for cue in staged_model_cues
        if isinstance(cue, dict)
        and cue.get("cueId") == DIMENSION_SUMMON_MODEL_CUE_ID
        and cue.get("modelAssetId") == DIMENSION_SUMMON_MODEL_ASSET_ID
    ]
    if len(matches) != 1:
        raise RestorationError(
            "Dimension Summon ModelCue identity is missing/ambiguous: "
            f"{record.target_effect_id}"
        )
    cue = matches[0]
    if cue.get("clipName") != DIMENSION_SUMMON_CLIP_NAME:
        raise RestorationError(
            "Dimension Summon ModelCue clip identity drifted: "
            f"{cue.get('clipName')}"
        )
    alpha_mode = cue.get("alphaMode")
    if alpha_mode is not None and alpha_mode != "MASKED":
        raise RestorationError(
            f"Dimension Summon ModelCue alphaMode drifted: {alpha_mode}"
        )
    mask_contract = dimension_summon_mask_contract()
    cue["alphaMode"] = "MASKED"
    staged["modelCues"] = staged_model_cues
    return [
        {
            "cueId": DIMENSION_SUMMON_MODEL_CUE_ID,
            "modelAssetId": DIMENSION_SUMMON_MODEL_ASSET_ID,
            "clipName": DIMENSION_SUMMON_CLIP_NAME,
            "alphaMode": "MASKED",
            "provenance": "RAW_UE3_PARENT_BLEND_MASKED_EXACT",
            "policy": "STABLE_CUE_MODEL_AND_CLIP_ID_ONLY_NO_GLOBAL_DEFAULT",
            "sourceMaskContract": mask_contract,
        }
    ]


def reset_source_presentation() -> dict[str, Any]:
    return {
        "enabled": False,
        "schema": "lostark.effect-source-presentation",
        "version": 1,
        "profileId": "",
        "status": "unresolved",
        "sourceObjectPath": "",
        "sourceActionCueId": "",
        "sourceEventId": "",
        "sourceOccurrenceIndex": 0,
        "sourceTimeSeconds": 0.0,
        "parameters": [],
    }


def graph_property_value(
    graph_object: dict[str, Any], name: str, default: Any = None
) -> Any:
    raw_property = graph_object.get("properties", {}).get(name.casefold())
    if not isinstance(raw_property, dict):
        return default
    return raw_property.get("value", default)


def graph_reference_path(
    graph_object: dict[str, Any], property_name: str
) -> str:
    matches = [
        str(reference.get("objectPath", ""))
        for reference in graph_object.get("references", [])
        if isinstance(reference, dict)
        and str(reference.get("property", "")).casefold()
        == property_name.casefold()
        and isinstance(reference.get("objectPath"), str)
        and reference.get("objectPath")
    ]
    if not matches:
        raise RestorationError(
            f"AnimationTrail graph reference is missing: "
            f"{graph_object.get('objectPath')}/{property_name}"
        )
    return matches[0]


def validate_animation_trail_material_source(
    source_material_path: str,
) -> tuple[dict[str, Any], dict[str, Any]]:
    material_source = ANIMATION_TRAIL_MATERIAL_SOURCES.get(source_material_path)
    if material_source is None:
        raise RestorationError(
            f"AnimationTrail Material identity is not admitted: {source_material_path}"
        )
    material_map_path = material_source["path"]
    if raw_sha256(material_map_path) != material_source["sha256"]:
        raise RestorationError(
            f"AnimationTrail physical Material map drifted: {material_map_path}"
        )
    material_map = load_json(material_map_path)
    source = material_map.get("source")
    if (
        not isinstance(source, dict)
        or source.get("file") != material_source["physicalPackage"]
    ):
        raise RestorationError(
            f"AnimationTrail physical package identity drifted: {material_map_path}"
        )
    matches = [
        material
        for material in material_map.get("materials", [])
        if isinstance(material, dict)
        and str(material.get("material_path", "")).casefold()
        == str(material_source["materialPath"]).casefold()
    ]
    if len(matches) != 1:
        raise RestorationError(
            f"AnimationTrail Material map join is missing/ambiguous: "
            f"{source_material_path}"
        )
    material = matches[0]
    actual_textures = tuple(
        sorted(
            (
                str(texture.get("name", "")).casefold(),
                str(texture.get("texture", "")).casefold(),
            )
            for texture in material.get("textures", [])
            if isinstance(texture, dict)
        )
    )
    expected_textures = tuple(
        sorted(
            (str(name).casefold(), str(texture).casefold())
            for name, texture in material_source["sourceTextures"]
        )
    )
    if actual_textures != expected_textures:
        raise RestorationError(
            f"AnimationTrail Material texture contract drifted: {source_material_path}"
        )
    resource_slots: set[str] = set()
    for slot_id, asset_id in material_source["resources"]:
        if slot_id in resource_slots or not resource_path(asset_id).is_file():
            raise RestorationError(
                f"AnimationTrail runtime resource is missing/duplicate: "
                f"{source_material_path}/{slot_id}/{asset_id}"
            )
        resource_slots.add(slot_id)
    if "base" not in resource_slots:
        raise RestorationError(
            f"AnimationTrail authoring carrier has no bounded Base: {source_material_path}"
        )
    return material_source, material


def load_source_animation_trail_assignments(
    records: list[CandidateRecord],
) -> dict[str, tuple[SourceAnimationTrailAssignment, ...]]:
    record_by_key: dict[tuple[str, int, str], CandidateRecord] = {}
    for record in records:
        key = (record.character_class, record.skill_id, record.clip)
        if key in record_by_key:
            raise RestorationError(
                f"AnimationTrail target clip is ambiguous: {key}"
            )
        record_by_key[key] = record

    graph_cache: dict[str, tuple[Path, str, dict[str, dict[str, Any]], str]] = {}
    assignments: dict[str, list[SourceAnimationTrailAssignment]] = {}
    target_ids: set[str] = set()
    source_claims: set[tuple[str, str, str]] = set()
    notify_claims: set[tuple[str, int, str]] = set()

    for source_spec in ANIMATION_TRAIL_SOURCE_SPECS:
        character_class = str(source_spec["characterClass"])
        skill_id = int(source_spec["skillId"])
        graph_source = ANIMATION_TRAIL_GRAPH_SOURCES[character_class]
        graph_path = graph_source["path"]
        graph_sha = str(graph_source["sha256"])
        if raw_sha256(graph_path) != graph_sha:
            raise RestorationError(
                f"AnimationTrail particle graph drifted: {graph_path}"
            )
        if character_class not in graph_cache:
            graph = load_json(graph_path)
            raw_objects = graph.get("objects")
            if not isinstance(raw_objects, list):
                raise RestorationError(
                    f"AnimationTrail particle graph objects are missing: {graph_path}"
                )
            by_path: dict[str, dict[str, Any]] = {}
            for graph_object in raw_objects:
                if not isinstance(graph_object, dict):
                    raise RestorationError(
                        f"AnimationTrail graph object is invalid: {graph_path}"
                    )
                object_path = str(graph_object.get("objectPath", "")).casefold()
                if not object_path or object_path in by_path:
                    raise RestorationError(
                        f"AnimationTrail graph object path is missing/duplicate: {graph_path}"
                    )
                by_path[object_path] = graph_object
            graph_package = str(graph.get("package", ""))
            if not graph_package:
                raise RestorationError(
                    f"AnimationTrail graph package identity is missing: {graph_path}"
                )
            graph_cache[character_class] = (
                graph_path,
                graph_sha,
                by_path,
                graph_package,
            )
        _, _, objects_by_path, graph_package = graph_cache[character_class]

        receipt_path = source_spec["receiptPath"]
        source_receipt = load_json(receipt_path)
        timeline = source_receipt.get("timeline")
        raw_events = timeline.get("events") if isinstance(timeline, dict) else None
        if not isinstance(raw_events, list):
            raise RestorationError(
                f"AnimationTrail source receipt events are missing: {receipt_path}"
            )
        event_by_id = {
            str(event.get("eventId", "")): event
            for event in raw_events
            if isinstance(event, dict)
        }
        for occurrence_index, event_id in enumerate(source_spec["eventIds"]):
            event = event_by_id.get(event_id)
            if event is None:
                raise RestorationError(
                    f"AnimationTrail source event is missing: {receipt_path}/{event_id}"
                )
            source_asset = event.get("sourceAsset")
            clip = event.get("clip")
            local_time = event.get("localTimeSeconds")
            duration = event.get("durationSeconds")
            if (
                event.get("kind") != "EFFECT"
                or event.get("sourceType") != "Trails"
                or event.get("resolutionStatus") != "UNSUPPORTED_SOURCE_NOTIFY"
                or not isinstance(source_asset, str)
                or source_asset not in source_spec["sourceAssetEmitters"]
                or not isinstance(clip, str)
                or not clip
                or isinstance(local_time, bool)
                or not isinstance(local_time, (int, float))
                or float(local_time) < 0.0
                or isinstance(duration, bool)
                or not isinstance(duration, (int, float))
                or float(duration) <= 0.0
            ):
                raise RestorationError(
                    f"AnimationTrail source occurrence drifted: {receipt_path}/{event_id}"
                )
            record = record_by_key.get((character_class, skill_id, clip))
            if record is None:
                raise RestorationError(
                    f"AnimationTrail target clip join is missing: "
                    f"{character_class}/{skill_id}/{clip}"
                )
            notify_key = (character_class, skill_id, event_id)
            if notify_key in notify_claims:
                raise RestorationError(
                    f"AnimationTrail source notify is duplicated: {notify_key}"
                )
            notify_claims.add(notify_key)

            for emitter_path in source_spec["sourceAssetEmitters"][source_asset]:
                emitter = objects_by_path.get(str(emitter_path).casefold())
                if (
                    emitter is None
                    or str(emitter.get("className", "")).casefold()
                    != "particlespriteemitter"
                ):
                    raise RestorationError(
                        f"AnimationTrail source emitter is missing: "
                        f"{graph_path}/{emitter_path}"
                    )
                lod_path = graph_reference_path(emitter, "lodlevels")
                lod = objects_by_path.get(lod_path.casefold())
                if lod is None or str(lod.get("className", "")).casefold() != "particlelodlevel":
                    raise RestorationError(
                        f"AnimationTrail source LOD is missing: {graph_path}/{lod_path}"
                    )
                typed_data_path = graph_reference_path(lod, "typedatamodule")
                typed_data = objects_by_path.get(typed_data_path.casefold())
                if (
                    typed_data is None
                    or str(typed_data.get("className", "")).casefold()
                    != "particlemoduletypedataanimtrail"
                ):
                    raise RestorationError(
                        f"AnimationTrail source TypeData is not AnimTrail: "
                        f"{graph_path}/{typed_data_path}"
                    )
                required_path = graph_reference_path(lod, "requiredmodule")
                required = objects_by_path.get(required_path.casefold())
                if required is None:
                    raise RestorationError(
                        f"AnimationTrail Required module is missing: "
                        f"{graph_path}/{required_path}"
                    )
                source_material_path = graph_reference_path(required, "material")
                material_source, material_row = (
                    validate_animation_trail_material_source(source_material_path)
                )
                control_edge = str(
                    graph_property_value(typed_data, "controledgename", "")
                )
                if not control_edge:
                    raise RestorationError(
                        f"AnimationTrail control edge is missing: {typed_data_path}"
                    )
                tiling_distance = float(
                    graph_property_value(typed_data, "tilingdistance", 0.0)
                )
                tessellation_step = float(
                    graph_property_value(
                        typed_data, "distancetessellationstep", 0.0
                    )
                )
                source_element_id = (
                    f"{graph_package}.{emitter_path}".casefold()
                )
                source_claim = (source_element_id, event_id, record.target_effect_id)
                if source_claim in source_claims:
                    raise RestorationError(
                        f"AnimationTrail source occurrence is duplicated: {source_claim}"
                    )
                source_claims.add(source_claim)
                target_digest = hashlib.sha256(
                    (
                        f"{character_class}\0{skill_id}\0{event_id}\0{source_asset}\0"
                        f"{source_element_id}\0{record.target_effect_id}"
                    ).encode("utf-8")
                ).hexdigest()[:24]
                target_element_id = (
                    "authored.source-animation-trail." + target_digest
                )
                if target_element_id in target_ids:
                    raise RestorationError(
                        f"AnimationTrail target ID collision: {target_element_id}"
                    )
                target_ids.add(target_element_id)
                graph_evidence = {
                    "emitter": emitter,
                    "lod": lod,
                    "typedData": typed_data,
                    "required": required,
                    "material": material_row,
                }
                assignments.setdefault(record.target_effect_id, []).append(
                    SourceAnimationTrailAssignment(
                        character_class=character_class,
                        skill_id=skill_id,
                        source_event_id=event_id,
                        source_asset=source_asset,
                        source_element_id=source_element_id,
                        source_emitter_path=str(emitter_path),
                        source_typed_data_path=typed_data_path,
                        source_material_path=source_material_path,
                        source_graph_path=graph_path,
                        source_graph_sha256=graph_sha,
                        source_graph_evidence_sha256=canonical_sha256(graph_evidence),
                        source_material_map_path=material_source["path"],
                        source_material_map_sha256=material_source["sha256"],
                        source_physical_package=material_source["physicalPackage"],
                        source_material_object_path=material_source["materialPath"],
                        target_element_id=target_element_id,
                        target_effect_id=record.target_effect_id,
                        local_time_seconds=float(local_time),
                        duration_seconds=float(duration),
                        control_edge_name=control_edge,
                        tiling_distance_world_units=tiling_distance,
                        distance_tessellation_step_world_units=tessellation_step,
                        runtime_anchor_slot_id=str(
                            source_spec["runtimeAnchorSlotId"]
                        ),
                        runtime_bone_name=str(source_spec["runtimeBoneName"]),
                        render_profile=str(material_source["renderProfile"]),
                        resources=tuple(material_source["resources"]),
                        base_decision=str(material_source["baseDecision"]),
                    )
                )

    flattened = [row for rows in assignments.values() for row in rows]
    if (
        len(notify_claims) != EXPECTED_SOURCE_ANIMATION_TRAIL_NOTIFY_COUNT
        or len(flattened) != EXPECTED_SOURCE_ANIMATION_TRAIL_ELEMENT_COUNT
        or len(source_claims) != EXPECTED_SOURCE_ANIMATION_TRAIL_ELEMENT_COUNT
    ):
        raise RestorationError(
            "AnimationTrail source denominator changed: "
            f"notifies={len(notify_claims)} elements={len(flattened)}"
        )
    return {
        target_effect_id: tuple(
            sorted(rows, key=lambda row: row.target_element_id)
        )
        for target_effect_id, rows in assignments.items()
    }


def load_animation_trail_authoring_template() -> dict[str, Any]:
    if (
        raw_sha256(ANIMATION_TRAIL_AUTHORING_TEMPLATE_PATH)
        != ANIMATION_TRAIL_AUTHORING_TEMPLATE_RAW_SHA256
    ):
        raise RestorationError("AnimationTrail bounded authoring template drifted")
    document = load_json(ANIMATION_TRAIL_AUTHORING_TEMPLATE_PATH)
    elements = require_authoring_document(
        document,
        path=ANIMATION_TRAIL_AUTHORING_TEMPLATE_PATH,
        effect_id=str(document.get("effectAssetId", "")),
        versions={12, 13},
    )
    trails = [
        element
        for element in elements
        if element.get("kind") == "trail"
        and element.get("id") == "manual.trackb.ba1.animtrail-companion"
    ]
    if len(trails) != 1:
        raise RestorationError(
            "AnimationTrail bounded authoring template is missing/ambiguous"
        )
    template = copy.deepcopy(trails[0])
    detail = template.get("detail")
    trail_detail = detail.get("trail") if isinstance(detail, dict) else None
    if (
        not isinstance(trail_detail, dict)
        or trail_detail.get("maxPoints") != 64
        or not math.isclose(
            float(trail_detail.get("sampleIntervalSeconds", 0.0)),
            1.0 / 60.0,
            rel_tol=0.0,
            abs_tol=1.0e-9,
        )
    ):
        raise RestorationError("AnimationTrail bounded authoring template drifted")
    return template


def source_presentation_parameter(
    *, name: str, kind: str, source_path: str, value: Any
) -> dict[str, Any]:
    return {
        "name": name,
        "type": kind,
        "status": "source_explicit",
        "sourcePropertyPath": source_path,
        "numberValue": float(value) if kind == "number" else 0.0,
        "boolValue": bool(value) if kind == "boolean" else False,
        "vectorValue": list(value) if kind == "vector" else [0.0, 0.0, 0.0, 0.0],
        "stringValue": str(value) if kind == "string" else "",
    }


def materialize_source_animation_trail(
    record: CandidateRecord,
    assignment: SourceAnimationTrailAssignment,
    template: dict[str, Any],
    current_element: dict[str, Any] | None,
) -> tuple[dict[str, Any], dict[str, Any]]:
    if assignment.target_effect_id != record.target_effect_id:
        raise RestorationError(
            f"AnimationTrail target identity drifted: {assignment.target_effect_id}"
        )
    stable_reimport = current_element is not None
    if stable_reimport and (
        current_element.get("id") != assignment.target_element_id
        or current_element.get("kind") != "trail"
    ):
        raise RestorationError(
            f"AnimationTrail stable reimport identity drifted: "
            f"{record.target_effect_id}/{assignment.target_element_id}"
        )

    staged = copy.deepcopy(template)
    staged["id"] = assignment.target_element_id
    staged["displayName"] = (
        f"animtrail {assignment.source_event_id} "
        f"{assignment.source_emitter_path.rsplit('.', 1)[-1]}"
    )
    staged["groupId"] = "authored.source-animation-trail"
    staged["sourceNode"] = (
        f"authored-source-animation-trail:{record.target_effect_id}|source:"
        f"{assignment.source_asset}|element:{assignment.source_element_id}"
    )
    staged["kind"] = "trail"
    staged["visible"] = (
        bool(current_element.get("visible")) if stable_reimport else True
    )

    target_resources = [
        {"slotId": slot_id, "assetId": asset_id}
        for slot_id, asset_id in assignment.resources
    ]
    canonical_base_asset_id = next(
        (
            binding["assetId"]
            for binding in target_resources
            if binding["slotId"] == "base"
        ),
        "",
    )
    preserved_base_asset_id = canonical_base_asset_id
    if stable_reimport:
        existing_resources = resources(
            current_element.get("resources"),
            f"existing AnimationTrail {record.target_effect_id}/"
            f"{assignment.target_element_id}",
        )
        existing_base = next(
            (
                binding
                for binding in existing_resources
                if binding["slotId"] == "base"
            ),
            None,
        )
        if existing_base is not None:
            preserved_base_asset_id = existing_base["assetId"]
            for binding in target_resources:
                if binding["slotId"] == "base":
                    binding["assetId"] = existing_base["assetId"]
                    break
    staged["resources"] = target_resources
    staged["material"] = {
        "templateId": "effect.standard",
        "sourceMaterialPath": assignment.source_material_path,
        "renderProfile": assignment.render_profile,
        "sourceProfile": {"enabled": False},
    }
    staged["actionCueAttachment"] = {
        "enabled": True,
        "follow": True,
        "sourceAnchorSlotId": assignment.runtime_anchor_slot_id,
        "runtimeAnchorSlotId": assignment.runtime_anchor_slot_id,
        "runtimeBoneName": assignment.runtime_bone_name,
        "snapshotRootSourceBasisYawDegrees": 0.0,
        "socketLocalTransform": {
            "position": [0.0, 0.0, 0.0],
            "rotationDegrees": [0.0, 0.0, 0.0],
            "scale": [1.0, 1.0, 1.0],
        },
    }
    staged["transformInheritance"] = {
        "enabled": False,
        "masterElementId": "",
    }
    canonical_detail = copy.deepcopy(template["detail"])
    canonical_detail["timing"]["startDelaySeconds"] = (
        assignment.local_time_seconds
    )
    canonical_detail["timing"]["lifeTimeSeconds"] = (
        assignment.duration_seconds
    )
    canonical_detail["trail"]["pointLifeTimeSeconds"] = min(
        0.22, assignment.duration_seconds
    )
    canonical_detail["trail"]["tilingDistanceWorldUnits"] = (
        assignment.tiling_distance_world_units
    )
    canonical_detail["trail"]["distanceTessellationStepWorldUnits"] = (
        assignment.distance_tessellation_step_world_units
    )
    if stable_reimport:
        staged["detail"] = copy.deepcopy(current_element["detail"])
    else:
        staged["detail"] = canonical_detail
    staged["sourceRecipe"] = {
        "enabled": False,
        "rendererShape": "",
        "emitterDelaySeconds": 0.0,
        "emitterDurationSeconds": 0.0,
        "emitterLoopCount": 0,
        "bursts": [],
        "modules": [],
    }
    staged["sourcePresentation"] = {
        "enabled": True,
        "schema": "lostark.effect-source-presentation",
        "version": 1,
        "profileId": "four-class.animation-trail-history.v1",
        "status": "reconstructed",
        "sourceObjectPath": assignment.source_asset,
        "sourceActionCueId": (
            f"skill.{record.skill_id}.stage.{record.stage_index}"
        ),
        "sourceEventId": assignment.source_event_id,
        "sourceOccurrenceIndex": int(
            assignment.source_event_id.rsplit("-", 1)[-1]
        ),
        "sourceTimeSeconds": assignment.local_time_seconds,
        "parameters": [
            source_presentation_parameter(
                name="controlEdgeName",
                kind="string",
                source_path=(
                    f"{assignment.source_typed_data_path}.controledgename"
                ),
                value=assignment.control_edge_name,
            ),
            source_presentation_parameter(
                name="tilingDistanceWorldUnits",
                kind="number",
                source_path=(
                    f"{assignment.source_typed_data_path}.tilingdistance"
                ),
                value=assignment.tiling_distance_world_units,
            ),
            source_presentation_parameter(
                name="distanceTessellationStepWorldUnits",
                kind="number",
                source_path=(
                    f"{assignment.source_typed_data_path}.distancetessellationstep"
                ),
                value=assignment.distance_tessellation_step_world_units,
            ),
        ],
    }
    source_presentation_sha = canonical_sha256(staged["sourcePresentation"])
    stable_reimport_preserved = bool(
        stable_reimport
        and (
            not bool(current_element.get("visible"))
            or current_element["detail"] != canonical_detail
            or preserved_base_asset_id != canonical_base_asset_id
        )
    )
    return staged, {
        "targetElementId": assignment.target_element_id,
        "sourceElementId": assignment.source_element_id,
        "sourceEventId": assignment.source_event_id,
        "sourceAsset": assignment.source_asset,
        "sourceEmitterPath": assignment.source_emitter_path,
        "sourceTypedDataPath": assignment.source_typed_data_path,
        "sourceMaterialPath": assignment.source_material_path,
        "sourcePhysicalPackage": assignment.source_physical_package,
        "sourceMaterialObjectPath": assignment.source_material_object_path,
        "sourceGraphPath": assignment.source_graph_path.as_posix(),
        "sourceGraphRawSha256": assignment.source_graph_sha256,
        "sourceGraphEvidenceCanonicalSha256": (
            assignment.source_graph_evidence_sha256
        ),
        "sourceMaterialMapPath": assignment.source_material_map_path.as_posix(),
        "sourceMaterialMapRawSha256": assignment.source_material_map_sha256,
        "sourcePresentationCanonicalSha256": source_presentation_sha,
        "sourceResourceBindings": [
            {"slotId": slot_id, "assetId": asset_id}
            for slot_id, asset_id in assignment.resources
        ],
        "targetResourceBindings": target_resources,
        "baseDecision": assignment.base_decision,
        "historyRuntimeProfileId": "four-class.animation-trail-history.v1",
        "materialExecutionStatus": (
            "BOUNDED_GENERIC_TRAIL_PREVIEW_EXACT_SOURCE_MATERIAL_IDENTITY"
        ),
        "stableReimportPreserved": stable_reimport_preserved,
        "portable": True,
        "admitted": True,
        "deferredReason": "",
        "failClosedReasons": [],
    }


def validate_portable_event_route_closure(
    staged_elements: list[dict[str, Any]],
) -> int:
    """Require admitted Spawn-event routes to close inside one target document."""

    generators: dict[str, set[str]] = {}
    receivers: dict[str, set[str]] = {}
    for element in staged_elements:
        execution_target = is_authoring_execution_target(element)
        if element.get("kind") != "particle":
            continue
        if not execution_target:
            continue
        recipe = element.get("sourceRecipe")
        if not isinstance(recipe, dict) or recipe.get("enabled") is not True:
            continue
        element_id = str(element["id"])
        for module in recipe.get("modules", []):
            if not isinstance(module, dict):
                raise RestorationError(
                    f"portable event route module is invalid: {element_id}"
                )
            module_class = normalized_module_class(
                str(module.get("className", "")).casefold()
            )
            if module_class == "particlemoduleeventgenerator":
                event_type = source_literal_value(
                    module, "events[0].type", ""
                )
                event_name = source_literal_value(
                    module, "events[0].customname", ""
                )
                if event_type != "epet_spawn" or not isinstance(
                    event_name, str
                ) or not event_name:
                    raise RestorationError(
                        f"portable event generator identity is invalid: {element_id}"
                    )
                generators.setdefault(event_name, set()).add(element_id)
            elif module_class == "particlemoduleeventreceiverspawn":
                event_type = source_literal_value(
                    module, "eventgeneratortype", ""
                )
                event_name = source_literal_value(module, "eventname", "")
                if event_type not in {"epet_spawn", "epet_any"} or not isinstance(
                    event_name, str
                ) or not event_name:
                    raise RestorationError(
                        f"portable event receiver identity is invalid: {element_id}"
                    )
                receivers.setdefault(event_name, set()).add(element_id)

    if set(generators) != set(receivers):
        raise RestorationError(
            "portable event routes do not have a same-document generator/receiver "
            f"closure: generators={sorted(generators)} receivers={sorted(receivers)}"
        )
    edges: set[tuple[str, str]] = set()
    for event_name in generators:
        for generator_id in generators[event_name]:
            for receiver_id in receivers[event_name]:
                edges.add((generator_id, receiver_id))
    adjacency: dict[str, set[str]] = {}
    for source_id, target_id in edges:
        adjacency.setdefault(source_id, set()).add(target_id)
    visit: dict[str, int] = {}

    def visit_element(element_id: str) -> None:
        state = visit.get(element_id, 0)
        if state == 1:
            raise RestorationError(
                f"portable event route cycle is not allowed: {element_id}"
            )
        if state == 2:
            return
        visit[element_id] = 1
        for target_id in adjacency.get(element_id, ()):
            visit_element(target_id)
        visit[element_id] = 2

    for element_id in adjacency:
        visit_element(element_id)
    return len(edges)


def materialize_source_decal(
    record: CandidateRecord,
    assignment: SourceDecalAssignment,
    current_element: dict[str, Any] | None,
) -> tuple[dict[str, Any], dict[str, Any]]:
    source_element = assignment.source_document.elements[
        assignment.source_element_id
    ]
    source_resources = resources(
        source_element.get("resources"),
        f"{assignment.source_document.effect_id}/{assignment.source_element_id}",
    )
    source_base = next(
        (binding for binding in source_resources if binding["slotId"] == "base"),
        None,
    )
    existing_resources: list[dict[str, str]] = []
    existing_base: dict[str, str] | None = None
    if current_element is not None:
        if (
            current_element.get("id") != assignment.target_element_id
            or current_element.get("kind") != "decal"
        ):
            raise RestorationError(
                f"source Decal target identity changed: {record.target_effect_id}/"
                f"{assignment.target_element_id}"
            )
        existing_resources = resources(
            current_element.get("resources"),
            f"existing {record.target_effect_id}/{assignment.target_element_id}",
        )
        existing_base = next(
            (
                binding
                for binding in existing_resources
                if binding["slotId"] == "base"
            ),
            None,
        )

    target_resources = copy.deepcopy(source_resources)
    if existing_base is not None:
        replaced = False
        for binding in target_resources:
            if binding["slotId"] == "base":
                binding["assetId"] = existing_base["assetId"]
                replaced = True
                break
        if not replaced:
            target_resources.append(copy.deepcopy(existing_base))
    has_base = any(binding["slotId"] == "base" for binding in target_resources)

    normalized_recipe = normalized_source_recipe(source_element.get("sourceRecipe"))
    if normalized_recipe.get("rendererShape") != "decal":
        raise RestorationError(
            f"source Decal recipe shape changed: {assignment.source_document.effect_id}/"
            f"{assignment.source_element_id}"
        )

    staged = copy.deepcopy(source_element)
    staged["id"] = assignment.target_element_id
    staged["displayName"] = (
        f"decal {assignment.source_event_id} "
        f"{assignment.source_element_id.rsplit('.', 1)[-1]}"
    )
    staged["groupId"] = "authored.source-decal"
    staged["sourceNode"] = (
        f"authored-source-decal:{record.target_effect_id}|source:"
        f"{assignment.source_document.effect_id}|element:"
        f"{assignment.source_element_id}"
    )
    staged["kind"] = "decal"
    staged["resources"] = target_resources
    staged["sourceRecipe"] = normalized_recipe
    staged["sourcePresentation"] = reset_source_presentation()
    staged["transformInheritance"] = {
        "enabled": False,
        "masterElementId": "",
    }

    if current_element is None:
        source_delay = float(
            staged["detail"]["timing"]["startDelaySeconds"]
        )
        local_delay = (
            source_delay - assignment.clip_timeline_offset_seconds
        )
        if local_delay < -1.0e-6:
            raise RestorationError(
                f"source Decal precedes its target clip: {record.target_effect_id}/"
                f"{assignment.target_element_id}"
            )
        staged["detail"]["timing"]["startDelaySeconds"] = max(
            0.0, local_delay
        )
        staged["actionCueAttachment"] = {
            "enabled": True,
            "follow": False,
            "sourceAnchorSlotId": "root",
            "runtimeAnchorSlotId": "root",
            "runtimeBoneName": "",
            "snapshotRootSourceBasisYawDegrees": 0.0,
            "socketLocalTransform": {
                "position": [0.0, 0.0, 0.0],
                "rotationDegrees": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        }
        material = copy.deepcopy(source_element["material"])
        material["templateId"] = "effect.standard"
        material["sourceProfile"] = {"enabled": False}
        material.pop("execution", None)
        staged["material"] = material
        staged["visible"] = has_base
    else:
        staged["detail"] = copy.deepcopy(current_element["detail"])
        staged["actionCueAttachment"] = copy.deepcopy(
            current_element["actionCueAttachment"]
        )
        staged["transformInheritance"] = copy.deepcopy(
            current_element.get(
                "transformInheritance",
                {"enabled": False, "masterElementId": ""},
            )
        )
        staged["material"] = copy.deepcopy(current_element["material"])
        staged["visible"] = bool(current_element.get("visible")) if has_base else False

    fail_closed_marker = {"enabled": False, "failClosed": True}
    if not has_base:
        staged["visible"] = False
        staged["material"]["execution"] = copy.deepcopy(fail_closed_marker)
    elif staged["material"].get("execution") == fail_closed_marker:
        # Binding a safe Base DDS is the explicit authoring action that promotes
        # a blank Decal draft from incomplete to ordinary editable playback.
        staged["material"].pop("execution")

    return staged, {
        "targetElementId": assignment.target_element_id,
        "sourceEffectAssetId": assignment.source_document.effect_id,
        "sourceElementId": assignment.source_element_id,
        "sourceEventId": assignment.source_event_id,
        "sourceRecipeCanonicalSha256": canonical_sha256(
            source_element["sourceRecipe"]
        ),
        "normalizedRecipeCanonicalSha256": canonical_sha256(normalized_recipe),
        "sourceBindings": source_resources,
        "targetBindings": target_resources,
        "baseStatus": "SOURCE_OR_ARTIST_BOUND" if has_base else "AUTHORING_INCOMPLETE",
    }


def materialize_candidate(
    record: CandidateRecord,
    source_by_effect: dict[str, SourceDocument],
    stage_effects: dict[tuple[str, int, int], tuple[str, ...]],
    source_particles: dict[str, tuple[SourceParticleAssignment, ...]],
    drawable_decisions: dict[tuple[str, str], DrawableResourceDecision],
    source_decals: dict[str, tuple[SourceDecalAssignment, ...]],
    source_animation_trails: dict[
        str, tuple[SourceAnimationTrailAssignment, ...]
    ],
    animation_trail_template: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any]]:
    if record.source_only:
        blueprint = {
            "schema": "lostark.effect-authoring",
            "version": 12,
            "effectAssetId": record.target_effect_id,
            "displayName": record.target_effect_id,
            "particleSystem": {},
            "modelCues": [],
            "elements": [],
        }
        blueprint_elements: list[dict[str, Any]] = []
    else:
        blueprint = load_json(record.blueprint_path)
        blueprint_effect_id = str(blueprint.get("effectAssetId", ""))
        blueprint_elements = require_authoring_document(
            blueprint,
            path=record.blueprint_path,
            effect_id=blueprint_effect_id,
            versions={12},
        )

    current_exists = record.target_path.is_file()
    if current_exists:
        current = load_json(record.target_path)
        current_elements = require_authoring_document(
            current,
            path=record.target_path,
            effect_id=record.target_effect_id,
            versions={12, 13},
        )
    else:
        current = copy.deepcopy(blueprint)
        current_elements = current["elements"]

    current_by_id = {str(element["id"]): element for element in current_elements}
    current_particle_by_identity: dict[tuple[str, str], dict[str, Any]] = {}
    for current_element in current_elements:
        if current_element.get("kind") != "particle":
            continue
        identity = particle_source_identity(
            current_element, record, source_by_effect, stage_effects
        )
        if identity in current_particle_by_identity:
            raise RestorationError(
                f"candidate has duplicate source Particle identity: "
                f"{record.target_effect_id}/{identity}"
            )
        current_particle_by_identity[identity] = current_element

    particle_assignments = source_particles.get(record.target_effect_id, ())
    decal_assignments = source_decals.get(record.target_effect_id, ())
    animation_trail_assignments = source_animation_trails.get(
        record.target_effect_id, ()
    )
    source_decal_ids = {row.target_element_id for row in decal_assignments}
    source_animation_trail_ids = {
        row.target_element_id for row in animation_trail_assignments
    }
    trail_elements = [
        element for element in current_elements if element.get("kind") == "trail"
    ]
    unexpected_current = [
        str(element.get("id", ""))
        for element in current_elements
        if element.get("kind") not in {"particle", "decal", "trail"}
    ]
    unexpected_decals = [
        str(element["id"])
        for element in current_elements
        if element.get("kind") == "decal"
        and str(element["id"]) not in source_decal_ids
    ]
    legacy_placeholder_id = (
        f"manual.trackb.ba{record.stage_index + 1}.animtrail-companion"
    )
    unexpected_trails = [
        str(element["id"])
        for element in trail_elements
        if str(element["id"]) not in source_animation_trail_ids
        and not (
            record.character_class == "LANCE_MASTER"
            and record.skill_id == 34010
            and str(element["id"]) == legacy_placeholder_id
            and element.get("sourceRecipe", {}).get("enabled") is not True
        )
    ]
    if unexpected_current or unexpected_decals or unexpected_trails:
        raise RestorationError(
            f"unclassified current Element cannot enter strict Track A: "
            f"{record.target_effect_id}/kinds={unexpected_current}/"
            f"decals={unexpected_decals}/trails={unexpected_trails}"
        )

    staged = copy.deepcopy(current)
    staged["schema"] = "lostark.effect-authoring"
    staged["version"] = 13
    staged["effectAssetId"] = record.target_effect_id
    staged["displayName"] = record.target_effect_id
    model_cue_rows = materialize_document_model_cues(record, current, staged)

    staged_elements: list[dict[str, Any]] = []
    portable_count = 0
    deferred_count = 0
    mesh_count = 0
    resource_binding_count = 0
    source_resource_binding_count = 0
    receipt_supplemental_binding_count = 0
    direct_resource_elements = 0
    drawable_decision_element_count = 0
    drawable_admitted_count = 0
    portable_fail_closed_count = 0
    missing_safe_base_count = 0
    missing_exact_mesh_model_count = 0
    missing_executable_drawable_count = 0
    non_exact_material_count = 0
    stable_reimport_preserved_count = 0
    deferred_reasons: Counter[str] = Counter()
    fail_closed_reasons: Counter[str] = Counter()
    placeholder_trail_count = 0
    source_decal_count = 0
    source_decal_base_ready_count = 0
    source_decal_incomplete_count = 0
    source_decal_resource_binding_count = 0
    source_decal_source_resource_binding_count = 0
    source_decal_rows: list[dict[str, Any]] = []
    source_animation_trail_rows: list[dict[str, Any]] = []
    particle_rows: list[dict[str, Any]] = []

    for assignment in particle_assignments:
        identity = (
            assignment.source_document.effect_id,
            assignment.source_element_id,
        )
        current_element = current_particle_by_identity.get(identity)
        stable_current = (
            current_element
            if current_element is not None
            and current_element.get("id") == assignment.target_element_id
            else None
        )
        drawable_decision = drawable_decisions.get(identity)
        staged_element, particle_receipt = materialize_source_particle(
            assignment, drawable_decision, stable_current
        )
        staged_elements.append(staged_element)
        particle_rows.append(particle_receipt)

        portable = bool(particle_receipt["portable"])
        if portable:
            portable_count += 1
        else:
            deferred_count += 1
            deferred_reasons[str(particle_receipt["deferredReason"])] += 1
        if assignment.source_document.elements[
            assignment.source_element_id
        ]["sourceRecipe"].get("rendererShape") == "mesh":
            mesh_count += 1
        source_bindings = particle_receipt["sourceBindings"]
        target_bindings = particle_receipt["targetBindings"]
        source_resource_binding_count += len(source_bindings)
        resource_binding_count += len(target_bindings)
        receipt_supplemental_binding_count += len(target_bindings) - len(
            source_bindings
        )
        if target_bindings == source_bindings:
            direct_resource_elements += 1
        if drawable_decision is not None:
            drawable_decision_element_count += 1
        if particle_receipt["failClosedReasons"]:
            if portable:
                portable_fail_closed_count += 1
            for reason in particle_receipt["failClosedReasons"]:
                fail_closed_reasons[str(reason)] += 1
        else:
            drawable_admitted_count += 1
        if not particle_receipt["hasSafeBase"]:
            missing_safe_base_count += 1
        if not particle_receipt["meshResourceValid"]:
            missing_exact_mesh_model_count += 1
        if not particle_receipt["hasSafeDrawable"]:
            missing_executable_drawable_count += 1
        if (
            particle_receipt["materialResolutionStatus"]
            != EXACT_SOURCE_MATERIAL_STATUS
        ):
            non_exact_material_count += 1
        if particle_receipt["stableReimportPreserved"]:
            stable_reimport_preserved_count += 1

    for assignment in decal_assignments:
        current_decal = current_by_id.get(assignment.target_element_id)
        staged_decal, decal_receipt = materialize_source_decal(
            record, assignment, current_decal
        )
        staged_elements.append(staged_decal)
        source_decal_rows.append(decal_receipt)
        source_decal_count += 1
        source_decal_resource_binding_count += len(staged_decal["resources"])
        source_decal_source_resource_binding_count += len(
            decal_receipt["sourceBindings"]
        )
        if decal_receipt["baseStatus"] == "AUTHORING_INCOMPLETE":
            source_decal_incomplete_count += 1
        else:
            source_decal_base_ready_count += 1

    for assignment in animation_trail_assignments:
        current_trail = current_by_id.get(assignment.target_element_id)
        staged_trail, trail_receipt = materialize_source_animation_trail(
            record,
            assignment,
            animation_trail_template,
            current_trail,
        )
        staged_elements.append(staged_trail)
        source_animation_trail_rows.append(trail_receipt)
    portable_event_route_count = validate_portable_event_route_closure(
        staged_elements
    )
    staged["elements"] = staged_elements
    if (
        record.target_effect_id != DIMENSION_SUMMON_MODEL_CUE_TARGET
        and staged.get("modelCues") != current.get("modelCues")
    ):
        raise RestorationError(
            f"document ModelCue contract changed during Particle projection: "
            f"{record.target_effect_id}"
        )
    require_authoring_document(
        staged,
        path=record.target_path,
        effect_id=record.target_effect_id,
        versions={13},
    )
    if json.loads(serialized(staged)) != staged:
        raise RestorationError(f"JSON round-trip changed staged candidate: {record.target_effect_id}")

    target_receipt = {
        "targetEffectAssetId": record.target_effect_id,
        "targetPath": record.target_path.relative_to(ROOT).as_posix(),
        "characterClass": record.character_class,
        "skillId": record.skill_id,
        "stageIndex": record.stage_index,
        "stageClipIndex": record.stage_clip_index,
        "clip": record.clip,
        "blueprintPath": (
            None
            if record.source_only
            else record.blueprint_path.relative_to(ROOT).as_posix()
        ),
        "blueprintRawSha256": (
            None if record.source_only else raw_sha256(record.blueprint_path)
        ),
        "blueprintPolicy": (
            "SOURCE_ONLY_EMPTY_TARGET"
            if record.source_only
            else "LEGACY_ROLLBACK_BASELINE"
        ),
        "sourceParticleCount": len(particle_assignments),
        "sourceDecalCount": source_decal_count,
        "sourceDecalBaseReadyCount": source_decal_base_ready_count,
        "sourceDecalIncompleteCount": source_decal_incomplete_count,
        "sourceDecalResourceBindingCount": source_decal_resource_binding_count,
        "sourceDecalSourceResourceBindingCount": (
            source_decal_source_resource_binding_count
        ),
        "sourceAnimationTrailCount": len(animation_trail_assignments),
        "portableEventRouteCount": portable_event_route_count,
        "supplementalPreservedCount": len(animation_trail_assignments),
        "placeholderTrailExcludedCount": placeholder_trail_count,
        "outputElementCount": len(staged_elements),
        "portableCount": portable_count,
        "sourcePreservedDeferredCount": deferred_count,
        "meshCount": mesh_count,
        "resourceBindingCount": resource_binding_count,
        "sourceResourceBindingCount": source_resource_binding_count,
        "receiptSupplementalBindingCount": receipt_supplemental_binding_count,
        "drawableDecisionElementCount": drawable_decision_element_count,
        "drawableAdmittedCount": drawable_admitted_count,
        "portableFailClosedCount": portable_fail_closed_count,
        "authoringApproximateCount": sum(
            bool(row["authoringApproximate"]) for row in particle_rows
        ),
        "hardPortableFailClosedCount": sum(
            bool(row["hardPortableFailClosed"]) for row in particle_rows
        ),
        "previewTargetCount": sum(
            bool(row["previewTarget"]) for row in particle_rows
        ),
        "missingSafeBaseCount": missing_safe_base_count,
        "missingExactMeshModelCount": missing_exact_mesh_model_count,
        "missingExecutableDrawableCount": missing_executable_drawable_count,
        "nonExactMaterialCount": non_exact_material_count,
        "stableReimportPreservedCount": stable_reimport_preserved_count,
        "authoringOverrideDropCount": sum(
            len(row["authoringOverrideDrops"]) for row in particle_rows
        ),
        "authoringOverrideDrops": [
            {
                "targetElementId": row["targetElementId"],
                **drop,
            }
            for row in particle_rows
            for drop in row["authoringOverrideDrops"]
        ],
        "canonicalMaterialJoinedCount": sum(
            bool(row["canonicalMaterialJoined"]) for row in particle_rows
        ),
        "sharedMaterialContractMatchedCount": sum(
            bool(row["sharedMaterialContractMatched"]) for row in particle_rows
        ),
        "sharedMaterialContractResolvedCount": sum(
            bool(row["sharedMaterialContractResolved"]) for row in particle_rows
        ),
        "sharedMaterialProfileJoinedCount": sum(
            bool(row["sharedMaterialProfileJoined"]) for row in particle_rows
        ),
        "sharedMaterialOccurrenceProfilePromotedCount": sum(
            bool(row["sharedMaterialOccurrenceProfilePromoted"])
            for row in particle_rows
        ),
        "sharedMaterialCanonicalFallbackReplacedCount": sum(
            bool(row["sharedMaterialCanonicalFallbackReplaced"])
            for row in particle_rows
        ),
        "sourceProfileEnabledCount": sum(
            bool(row["runtimeShaderProfileId"]) for row in particle_rows
        ),
        "sourceProfileReadyCount": sum(
            bool(row["runtimeShaderProfileId"]) and row["sourceProfileReady"]
            for row in particle_rows
        ),
        "sourceProfileBlockedCount": sum(
            bool(row["runtimeShaderProfileId"])
            and not row["sourceProfileReady"]
            for row in particle_rows
        ),
        "namedTextureCount": sum(
            row["namedTextureCount"] for row in particle_rows
        ),
        "exactNamedTextureCount": sum(
            row["exactNamedTextureCount"] for row in particle_rows
        ),
        "rebasedNamedTextureCount": sum(
            row["rebasedNamedTextureCount"] for row in particle_rows
        ),
        "manifestNamedTextureCount": sum(
            row["manifestNamedTextureCount"] for row in particle_rows
        ),
        "canonicalV12PackageQualifiedAssetCount": sum(
            row["canonicalV12PackageQualifiedAssetCount"]
            for row in particle_rows
        ),
        "canonicalV12PackageQualifiedElementCount": sum(
            row["canonicalV12PackageQualifiedAssetCount"] > 0
            for row in particle_rows
        ),
        "nonExactNamedTextureAliasCount": sum(
            row["nonExactNamedTextureAliasCount"] for row in particle_rows
        ),
        "nonExactNamedTextureAliasElementCount": sum(
            row["nonExactNamedTextureAliasCount"] > 0 for row in particle_rows
        ),
        "unresolvedNamedTextureCount": sum(
            row["unresolvedNamedTextureCount"] for row in particle_rows
        ),
        "directSourceResourceElementCount": direct_resource_elements,
        "receiptResourceProjectionElementCount": (
            len(particle_assignments) - direct_resource_elements
        ),
        "particleProjectionCanonicalSha256": canonical_sha256(particle_rows),
        "deferredReasonCounts": dict(sorted(deferred_reasons.items())),
        "failClosedReasonCounts": dict(sorted(fail_closed_reasons.items())),
        "particleRows": particle_rows,
        "modelCueRows": model_cue_rows,
        "modelCueAlphaModePinnedCount": len(model_cue_rows),
        "sourceDecalRows": source_decal_rows,
        "animationTrailRows": source_animation_trail_rows,
        "outputCanonicalSha256": canonical_sha256(staged),
    }
    return staged, target_receipt


def targeted_candidate_records(raw_specs: list[str]) -> list[CandidateRecord]:
    """Resolve explicit source-only targets from exact manifest stage/clip IDs."""

    records: list[CandidateRecord] = []
    seen_keys: set[tuple[str, int, int, int]] = set()
    seen_targets: set[str] = set()
    for raw_spec in raw_specs:
        parts = raw_spec.split(":", 4)
        if len(parts) != 5:
            raise RestorationError(
                "target spec must be CLASS:SKILL:STAGE:STAGE_CLIP:EFFECT_ID: "
                f"{raw_spec}"
            )
        character_class, raw_skill, raw_stage, raw_clip, target_effect_id = parts
        if character_class not in CLASS_MANIFESTS:
            raise RestorationError(
                f"target spec character class is unsupported: {character_class}"
            )
        try:
            skill_id = int(raw_skill)
            stage_index = int(raw_stage)
            stage_clip_index = int(raw_clip)
        except ValueError as error:
            raise RestorationError(
                f"target spec numeric identity is invalid: {raw_spec}"
            ) from error
        if not re.fullmatch(r"effect\.[a-z0-9_.-]+", target_effect_id):
            raise RestorationError(
                f"target Effect identity is invalid: {target_effect_id}"
            )

        manifest_path = CLASS_MANIFESTS[character_class]
        manifest = load_json(manifest_path)
        skills = [
            row
            for row in manifest.get("skills", [])
            if isinstance(row, dict) and int(row.get("productSkillId", -1)) == skill_id
        ]
        if len(skills) != 1 or not is_targeted_current_combat_skill(skills[0]):
            raise RestorationError(
                "explicit target must resolve one TARGETED_CURRENT_COMBAT skill: "
                f"{character_class}/{skill_id}"
            )
        stages = [
            row
            for row in skills[0].get("stages", [])
            if isinstance(row, dict) and int(row.get("stageIndex", -1)) == stage_index
        ]
        if len(stages) != 1:
            raise RestorationError(
                f"explicit target stage is missing/ambiguous: {raw_spec}"
            )
        clips = [
            row
            for row in stages[0].get("clips", [])
            if isinstance(row, dict)
            and int(row.get("stageClipIndex", -1)) == stage_clip_index
        ]
        if len(clips) != 1:
            raise RestorationError(
                f"explicit target clip is missing/ambiguous: {raw_spec}"
            )
        key = (character_class, skill_id, stage_index, stage_clip_index)
        if key in seen_keys or target_effect_id in seen_targets:
            raise RestorationError(f"duplicate explicit target: {raw_spec}")
        seen_keys.add(key)
        seen_targets.add(target_effect_id)
        target_path = (
            ROOT / "Data/Effects/Authored" / f"{target_effect_id}.effect.json"
        )
        records.append(
            CandidateRecord(
                character_class=character_class,
                skill_id=skill_id,
                stage_index=stage_index,
                stage_clip_index=stage_clip_index,
                clip=str(clips[0]["clip"]),
                target_effect_id=target_effect_id,
                target_path=target_path,
                blueprint_path=target_path,
                source_only=True,
            )
        )
    if not records:
        raise RestorationError("at least one explicit target is required")
    return sorted(
        records,
        key=lambda row: (
            CLASS_ORDER[row.character_class],
            row.skill_id,
            row.stage_index,
            row.stage_clip_index,
        ),
    )


def targeted_manifest_context(
    record: CandidateRecord,
) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any]]:
    manifest = load_json(CLASS_MANIFESTS[record.character_class])
    skills = [
        row
        for row in manifest.get("skills", [])
        if isinstance(row, dict)
        and int(row.get("productSkillId", -1)) == record.skill_id
    ]
    if len(skills) != 1 or not is_targeted_current_combat_skill(skills[0]):
        raise RestorationError(
            f"targeted manifest skill drifted: {record.character_class}/{record.skill_id}"
        )
    stages = [
        row
        for row in skills[0].get("stages", [])
        if isinstance(row, dict)
        and int(row.get("stageIndex", -1)) == record.stage_index
    ]
    if len(stages) != 1:
        raise RestorationError(
            f"targeted manifest stage drifted: {record.target_effect_id}"
        )
    clips = [
        row
        for row in stages[0].get("clips", [])
        if isinstance(row, dict)
        and int(row.get("stageClipIndex", -1)) == record.stage_clip_index
        and str(row.get("clip", "")) == record.clip
    ]
    if len(clips) != 1:
        raise RestorationError(
            f"targeted manifest clip drifted: {record.target_effect_id}"
        )
    return skills[0], stages[0], clips[0]


def load_targeted_source_assignments(
    records: list[CandidateRecord],
    source_by_effect: dict[str, SourceDocument],
) -> tuple[
    dict[str, tuple[SourceParticleAssignment, ...]],
    dict[str, tuple[SourceDecalAssignment, ...]],
    dict[str, dict[str, int]],
]:
    """Map every Particle/Decal in explicit source-only targets or fail closed."""

    particle_rows: dict[str, list[SourceParticleAssignment]] = {}
    decal_rows: dict[str, list[SourceDecalAssignment]] = {}
    target_ids: set[str] = set()
    scoped_counts: dict[str, dict[str, int]] = {}
    for record in records:
        _, stage, manifest_clip = targeted_manifest_context(record)
        stage_start = float(stage["timelineOffsetSeconds"])
        stage_end = stage_start + float(stage["durationSeconds"])
        stage_event_ids = {str(value) for value in stage.get("sourceEventIds", [])}
        artifacts = stage.get("sourceArtifacts", [])
        if not isinstance(artifacts, list) or len(artifacts) != 1:
            raise RestorationError(
                f"targeted stage must own one source artifact: {record.target_effect_id}"
            )
        descriptor = artifacts[0].get("importedDocument")
        if not isinstance(descriptor, dict):
            raise RestorationError(
                f"targeted stage Imported descriptor is missing: {record.target_effect_id}"
            )
        source_effect_id = str(descriptor.get("effectAssetId", ""))
        source_document = source_by_effect.get(source_effect_id)
        if source_document is None:
            raise RestorationError(
                f"targeted source document is not indexed: {source_effect_id}"
            )
        source_particle_count = 0
        source_decal_count = 0
        for source_order, source_element in enumerate(
            source_document.document.get("elements", [])
        ):
            if not isinstance(source_element, dict):
                raise RestorationError(
                    f"targeted source Element is invalid: {source_document.path}"
                )
            kind = source_element.get("kind")
            if kind not in {"particle", "decal"}:
                continue
            source_element_id = str(source_element.get("id", ""))
            source_system = source_element_id.casefold().split(".particle", 1)[0]
            source_presentation = source_element.get("sourcePresentation", {})
            explicit_event_id = (
                str(source_presentation.get("sourceEventId", ""))
                if isinstance(source_presentation, dict)
                else ""
            )
            source_event_id = explicit_event_id or (
                source_document.first_event_by_system.get(source_system, "")
            )
            source_event = source_document.timeline_events.get(source_event_id)
            if source_event is None or source_event_id not in stage_event_ids:
                raise RestorationError(
                    "targeted source occurrence has no exact stage event join: "
                    f"{source_effect_id}/{source_element_id}/{source_event_id}"
                )
            if (
                int(source_event.get("clipSequenceIndex", -1))
                != int(manifest_clip["sequenceIndex"])
                or str(source_event.get("clip", "")) != record.clip
            ):
                raise RestorationError(
                    "targeted source occurrence escaped its exact clip: "
                    f"{source_effect_id}/{source_element_id}/{source_event_id}"
                )
            source_delay = source_element.get("detail", {}).get("timing", {}).get(
                "startDelaySeconds"
            )
            if (
                isinstance(source_delay, bool)
                or not isinstance(source_delay, (int, float))
                or float(source_delay) + 1.0e-6 < stage_start
                or float(source_delay) >= stage_end - 1.0e-6
            ):
                raise RestorationError(
                    f"targeted source timing escaped its stage: {source_element_id}"
                )
            if kind == "particle":
                source_particle_count += 1
                target_element_id = stable_particle_element_id(
                    character_class=record.character_class,
                    skill_id=record.skill_id,
                    source_effect_id=source_effect_id,
                    source_element_id=source_element_id,
                    source_event_id=source_event_id,
                    target_effect_id=record.target_effect_id,
                )
                assignment: SourceParticleAssignment | SourceDecalAssignment = (
                    SourceParticleAssignment(
                        character_class=record.character_class,
                        skill_id=record.skill_id,
                        source_document=source_document,
                        source_element_id=source_element_id,
                        source_event_id=source_event_id,
                        source_order=source_order,
                        target_element_id=target_element_id,
                        target_effect_id=record.target_effect_id,
                        clip_timeline_offset_seconds=float(
                            manifest_clip["timelineOffsetSeconds"]
                        ),
                    )
                )
                particle_rows.setdefault(record.target_effect_id, []).append(
                    assignment
                )
            else:
                source_decal_count += 1
                digest = hashlib.sha256(
                    f"{source_effect_id}\0{source_element_id}".encode("utf-8")
                ).hexdigest()[:24]
                target_element_id = "authored.source-decal." + digest
                decal_rows.setdefault(record.target_effect_id, []).append(
                    SourceDecalAssignment(
                        source_document=source_document,
                        source_element_id=source_element_id,
                        source_event_id=source_event_id,
                        target_element_id=target_element_id,
                        clip_timeline_offset_seconds=float(
                            manifest_clip["timelineOffsetSeconds"]
                        ),
                    )
                )
            if target_element_id in target_ids:
                raise RestorationError(
                    f"targeted Element ID collision: {target_element_id}"
                )
            target_ids.add(target_element_id)
        if source_particle_count <= 0:
            raise RestorationError(
                f"targeted source has no Particle occurrences: {record.target_effect_id}"
            )
        scoped_counts[record.target_effect_id] = {
            "sourceParticleCount": source_particle_count,
            "mappedParticleCount": len(
                particle_rows.get(record.target_effect_id, [])
            ),
            "sourceDecalCount": source_decal_count,
            "mappedDecalCount": len(decal_rows.get(record.target_effect_id, [])),
        }
        if scoped_counts[record.target_effect_id] != {
            "sourceParticleCount": source_particle_count,
            "mappedParticleCount": source_particle_count,
            "sourceDecalCount": source_decal_count,
            "mappedDecalCount": source_decal_count,
        }:
            raise RestorationError(
                f"targeted scoped denominator changed: {record.target_effect_id}"
            )
    return (
        {
            key: tuple(sorted(value, key=lambda row: row.source_order))
            for key, value in particle_rows.items()
        },
        {
            key: tuple(sorted(value, key=lambda row: row.target_element_id))
            for key, value in decal_rows.items()
        },
        scoped_counts,
    )


def build_targeted_projection(
    raw_specs: list[str],
) -> tuple[dict[Path, str], dict[str, Any]]:
    """Build only explicit source-only targets with scoped exact denominators."""

    records = targeted_candidate_records(raw_specs)
    source_by_effect, stage_effects = load_source_index()
    source_particles, source_decals, scoped_counts = (
        load_targeted_source_assignments(records, source_by_effect)
    )
    drawable_decisions = load_drawable_resource_decisions(source_by_effect)
    outputs: dict[Path, str] = {}
    receipts: list[dict[str, Any]] = []
    for record in records:
        document, receipt = materialize_candidate(
            record,
            source_by_effect,
            stage_effects,
            source_particles,
            drawable_decisions,
            source_decals,
            {},
            {},
        )
        missing_resources: list[str] = []
        for element in document.get("elements", []):
            execution = element.get("material", {}).get("execution", {})
            if (
                isinstance(execution, dict)
                and execution.get("failClosed") is True
                and execution.get("authoringApproximate") is not True
                and element.get("visible") is not False
            ):
                raise RestorationError(
                    "targeted hard fail-closed Element is visible: "
                    f"{record.target_effect_id}/{element.get('id', '')}"
                )
            for binding in element.get("resources", []):
                if not isinstance(binding, dict):
                    raise RestorationError(
                        f"targeted resource binding is invalid: {record.target_effect_id}"
                    )
                asset_id = str(binding.get("assetId", ""))
                if asset_id and not resource_path(asset_id).is_file():
                    missing_resources.append(asset_id)
        if missing_resources:
            raise RestorationError(
                "targeted physical resource is missing: "
                f"{record.target_effect_id}/{sorted(set(missing_resources))}"
            )
        receipt["scopedDenominators"] = scoped_counts[record.target_effect_id]
        receipt["physicalResourceMissingCount"] = 0
        outputs[record.target_path] = serialized(document)
        receipts.append(receipt)

    counts = {
        "targetCount": len(receipts),
        "sourceParticleCount": sum(
            row["sourceParticleCount"] for row in scoped_counts.values()
        ),
        "mappedParticleCount": sum(
            row["mappedParticleCount"] for row in scoped_counts.values()
        ),
        "sourceDecalCount": sum(
            row["sourceDecalCount"] for row in scoped_counts.values()
        ),
        "mappedDecalCount": sum(
            row["mappedDecalCount"] for row in scoped_counts.values()
        ),
        "outputElementCount": sum(row["outputElementCount"] for row in receipts),
        "drawableAdmittedCount": sum(
            row["drawableAdmittedCount"] for row in receipts
        ),
        "authoringApproximateCount": sum(
            row["authoringApproximateCount"] for row in receipts
        ),
        "hardPortableFailClosedCount": sum(
            row["hardPortableFailClosedCount"] for row in receipts
        ),
        "sourcePreservedDeferredCount": sum(
            row["sourcePreservedDeferredCount"] for row in receipts
        ),
        "physicalResourceMissingCount": 0,
    }
    if (
        counts["sourceParticleCount"] != counts["mappedParticleCount"]
        or counts["sourceDecalCount"] != counts["mappedDecalCount"]
    ):
        raise RestorationError("targeted scoped denominator validation failed")
    receipt = {
        "schema": (
            "lostark.four-class-track-a-targeted-current-combat-receipt"
        ),
        "formatVersion": 1,
        "policy": {
            "defaultFullGateUnchanged": True,
            "explicitTargetsOnly": True,
            "sourceOnlyTarget": True,
            "globalDenominatorBypass": False,
            "catalogMutation": False,
            "animeventMutation": False,
            "roleVisibilityMutation": False,
        },
        "sources": {
            "sourceMaterialContract": {
                "path": SOURCE_MATERIAL_CONTRACT_PATHS[-1]
                .relative_to(ROOT)
                .as_posix(),
                "sha256": raw_sha256(SOURCE_MATERIAL_CONTRACT_PATHS[-1]),
            },
            "classStageManifests": [
                {
                    "characterClass": character_class,
                    "path": CLASS_MANIFESTS[character_class]
                    .relative_to(ROOT)
                    .as_posix(),
                    "sha256": raw_sha256(CLASS_MANIFESTS[character_class]),
                }
                for character_class in sorted(
                    {record.character_class for record in records}
                )
            ],
        },
        "counts": counts,
        "targets": receipts,
    }
    receipt["artifactSha256"] = canonical_sha256(receipt)
    outputs[TARGETED_RECEIPT_PATH] = serialized(receipt)
    return outputs, receipt


def build_projection(
    *, validate_expected_denominators: bool = True
) -> tuple[dict[Path, str], dict[str, Any]]:
    batch = load_json(BATCH_PATH)
    if (
        batch.get("schema") != "lostark.effect-authored-import-batch"
        or batch.get("formatVersion") != 1
    ):
        raise RestorationError("four-class authored import batch identity changed")
    records = candidate_records(batch)
    character_ghost_cues = load_character_ghost_cues(records)
    source_by_effect, stage_effects = load_source_index()
    source_particles, particle_exclusions = load_source_particle_assignments(
        records, source_by_effect
    )
    drawable_decisions = load_drawable_resource_decisions(source_by_effect)
    source_decals = load_source_decal_assignments(records, source_by_effect)
    source_animation_trails = load_source_animation_trail_assignments(records)
    animation_trail_template = load_animation_trail_authoring_template()

    assignment_target_by_identity = {
        (row.source_document.effect_id, row.source_element_id): row.target_effect_id
        for rows in source_particles.values()
        for row in rows
    }
    if len(assignment_target_by_identity) != EXPECTED_STRICT_MAPPED_PARTICLE_COUNT:
        raise RestorationError("strict Particle source identity is not one-to-one")
    exclusion_identities = {
        (row.source_effect_id, row.source_element_id) for row in particle_exclusions
    }
    if len(exclusion_identities) != EXPECTED_EXCLUDED_PARTICLE_COUNT:
        raise RestorationError("excluded Particle source identity is not one-to-one")
    if set(assignment_target_by_identity) & exclusion_identities:
        raise RestorationError("a source Particle is both mapped and excluded")

    legacy_selected_count = 0
    legacy_selected_excluded_count = 0
    legacy_selected_retargeted_count = 0
    for record in records:
        blueprint = load_json(record.blueprint_path)
        blueprint_effect_id = str(blueprint.get("effectAssetId", ""))
        blueprint_elements = require_authoring_document(
            blueprint,
            path=record.blueprint_path,
            effect_id=blueprint_effect_id,
            versions={12},
        )
        for blueprint_element in blueprint_elements:
            identity = particle_source_identity(
                blueprint_element, record, source_by_effect, stage_effects
            )
            legacy_selected_count += 1
            if identity in exclusion_identities:
                legacy_selected_excluded_count += 1
            elif assignment_target_by_identity.get(identity) != record.target_effect_id:
                legacy_selected_retargeted_count += 1
    if (
        legacy_selected_count != 2160
        or legacy_selected_excluded_count
        != EXPECTED_CURRENT_EXCLUDED_PARTICLE_COUNT
        or legacy_selected_retargeted_count
        != EXPECTED_CURRENT_RETARGETED_PARTICLE_COUNT
    ):
        raise RestorationError(
            "legacy selected occurrence audit changed: "
            f"selected={legacy_selected_count} excluded={legacy_selected_excluded_count} "
            f"retargeted={legacy_selected_retargeted_count}"
        )

    outputs: dict[Path, str] = {}
    receipts: list[dict[str, Any]] = []
    for record in records:
        document, receipt = materialize_candidate(
            record,
            source_by_effect,
            stage_effects,
            source_particles,
            drawable_decisions,
            source_decals,
            source_animation_trails,
            animation_trail_template,
        )
        outputs[record.target_path] = serialized(document)
        receipts.append(receipt)

    counts = {
        "targetCount": len(receipts),
        "sourceParticleCorpusCount": (
            sum(row["sourceParticleCount"] for row in receipts)
            + len(particle_exclusions)
        ),
        "strictMappedParticleCount": sum(
            row["sourceParticleCount"] for row in receipts
        ),
        "excludedParticleCount": len(particle_exclusions),
        "legacySelectedParticleCount": legacy_selected_count,
        "legacySelectedExcludedCount": legacy_selected_excluded_count,
        "legacySelectedRetargetedCount": legacy_selected_retargeted_count,
        "drawableDecisionCorpusCount": len(drawable_decisions),
        "sourceDecalCount": sum(row["sourceDecalCount"] for row in receipts),
        "sourceDecalBaseReadyCount": sum(
            row["sourceDecalBaseReadyCount"] for row in receipts
        ),
        "sourceDecalIncompleteCount": sum(
            row["sourceDecalIncompleteCount"] for row in receipts
        ),
        "sourceDecalResourceBindingCount": sum(
            row["sourceDecalResourceBindingCount"] for row in receipts
        ),
        "sourceDecalSourceResourceBindingCount": sum(
            row["sourceDecalSourceResourceBindingCount"] for row in receipts
        ),
        "sourceAnimationTrailNotifyCount": len(
            {
                (
                    row["characterClass"],
                    row["skillId"],
                    trail_row["sourceEventId"],
                )
                for row in receipts
                for trail_row in row["animationTrailRows"]
            }
        ),
        "sourceAnimationTrailElementCount": sum(
            row["sourceAnimationTrailCount"] for row in receipts
        ),
        "characterGhostCueCount": len(character_ghost_cues),
        "characterGhostTargetCount": len(
            {row["targetEffectAssetId"] for row in character_ghost_cues}
        ),
        "portableEventRouteCount": sum(
            row["portableEventRouteCount"] for row in receipts
        ),
        "supplementalPreservedCount": sum(
            row["supplementalPreservedCount"] for row in receipts
        ),
        "placeholderTrailExcludedCount": sum(
            row["placeholderTrailExcludedCount"] for row in receipts
        ),
        "outputElementCount": sum(row["outputElementCount"] for row in receipts),
        "portableCount": sum(row["portableCount"] for row in receipts),
        "sourcePreservedDeferredCount": sum(
            row["sourcePreservedDeferredCount"] for row in receipts
        ),
        "meshCount": sum(row["meshCount"] for row in receipts),
        "resourceBindingCount": sum(row["resourceBindingCount"] for row in receipts),
        "sourceResourceBindingCount": sum(
            row["sourceResourceBindingCount"] for row in receipts
        ),
        "receiptSupplementalBindingCount": sum(
            row["receiptSupplementalBindingCount"] for row in receipts
        ),
        "drawableDecisionElementCount": sum(
            row["drawableDecisionElementCount"] for row in receipts
        ),
        "drawableAdmittedCount": sum(
            row["drawableAdmittedCount"] for row in receipts
        ),
        "portableFailClosedCount": sum(
            row["portableFailClosedCount"] for row in receipts
        ),
        "authoringApproximateCount": sum(
            row["authoringApproximateCount"] for row in receipts
        ),
        "hardPortableFailClosedCount": sum(
            row["hardPortableFailClosedCount"] for row in receipts
        ),
        "previewTargetCount": sum(
            row["previewTargetCount"] for row in receipts
        ),
        "missingSafeBaseCount": sum(
            row["missingSafeBaseCount"] for row in receipts
        ),
        "missingExactMeshModelCount": sum(
            row["missingExactMeshModelCount"] for row in receipts
        ),
        "missingExecutableDrawableCount": sum(
            row["missingExecutableDrawableCount"] for row in receipts
        ),
        "nonExactMaterialCount": sum(
            row["nonExactMaterialCount"] for row in receipts
        ),
        "stableReimportPreservedCount": sum(
            row["stableReimportPreservedCount"] for row in receipts
        ),
        "canonicalMaterialJoinedCount": sum(
            row["canonicalMaterialJoinedCount"] for row in receipts
        ),
        "sharedMaterialContractMatchedCount": sum(
            row["sharedMaterialContractMatchedCount"] for row in receipts
        ),
        "sharedMaterialContractResolvedCount": sum(
            row["sharedMaterialContractResolvedCount"] for row in receipts
        ),
        "sharedMaterialProfileJoinedCount": sum(
            row["sharedMaterialProfileJoinedCount"] for row in receipts
        ),
        "sharedMaterialOccurrenceProfilePromotedCount": sum(
            row["sharedMaterialOccurrenceProfilePromotedCount"]
            for row in receipts
        ),
        "sharedMaterialCanonicalFallbackReplacedCount": sum(
            row["sharedMaterialCanonicalFallbackReplacedCount"]
            for row in receipts
        ),
        "sourceProfileEnabledCount": sum(
            row["sourceProfileEnabledCount"] for row in receipts
        ),
        "sourceProfileReadyCount": sum(
            row["sourceProfileReadyCount"] for row in receipts
        ),
        "sourceProfileBlockedCount": sum(
            row["sourceProfileBlockedCount"] for row in receipts
        ),
        "namedTextureCount": sum(row["namedTextureCount"] for row in receipts),
        "exactNamedTextureCount": sum(
            row["exactNamedTextureCount"] for row in receipts
        ),
        "rebasedNamedTextureCount": sum(
            row["rebasedNamedTextureCount"] for row in receipts
        ),
        "manifestNamedTextureCount": sum(
            row["manifestNamedTextureCount"] for row in receipts
        ),
        "canonicalV12PackageQualifiedAssetCount": sum(
            row["canonicalV12PackageQualifiedAssetCount"] for row in receipts
        ),
        "canonicalV12PackageQualifiedElementCount": sum(
            row["canonicalV12PackageQualifiedElementCount"] for row in receipts
        ),
        "nonExactNamedTextureAliasCount": sum(
            row["nonExactNamedTextureAliasCount"] for row in receipts
        ),
        "nonExactNamedTextureAliasElementCount": sum(
            row["nonExactNamedTextureAliasElementCount"] for row in receipts
        ),
        "unresolvedNamedTextureCount": sum(
            row["unresolvedNamedTextureCount"] for row in receipts
        ),
        "dimensionMasterBaseLessGroupedSpriteAdmittedCount": sum(
            row["rendererShape"] == "sprite"
            and row["runtimeShaderProfileId"] == GROUPED_TRANSLUCENT_PROFILE
            and not row["hasSafeBase"]
            and not row["failClosedReasons"]
            for receipt_row in receipts
            if receipt_row["characterClass"] == "DIMENSIONMASTER"
            for row in receipt_row["particleRows"]
        ),
        "modelCueAlphaModePinnedCount": sum(
            row["modelCueAlphaModePinnedCount"] for row in receipts
        ),
        "directSourceResourceElementCount": sum(
            row["directSourceResourceElementCount"] for row in receipts
        ),
        "receiptResourceProjectionElementCount": sum(
            row["receiptResourceProjectionElementCount"] for row in receipts
        ),
    }
    dm_a_targets = [
        row
        for row in receipts
        if row["targetEffectAssetId"]
        == "effect.dimensionmaster.skill.2050210.unified"
    ]
    if len(dm_a_targets) != 1:
        raise RestorationError(
            "DimensionMaster A unified target identity changed: "
            f"{len(dm_a_targets)}"
        )
    dm_a_golden_rows = [
        row
        for row in dm_a_targets[0]["particleRows"]
        if str(row["sourceElementId"]).split(
            ".event_source-event-", 1
        )[0]
        in DM_A_GOLDEN_SOURCE_ELEMENT_BASE_IDS
    ]
    counts["dimensionMaster2050210GoldenElementCount"] = len(
        dm_a_golden_rows
    )
    counts["dimensionMaster2050210GoldenAdmittedCount"] = sum(
        not row["failClosedReasons"] for row in dm_a_golden_rows
    )
    counts["dimensionMaster2050210GoldenFailClosedCount"] = sum(
        bool(row["failClosedReasons"]) for row in dm_a_golden_rows
    )
    expected = {
        "targetCount": EXPECTED_TARGET_COUNT,
        "sourceParticleCorpusCount": EXPECTED_SOURCE_PARTICLE_CORPUS_COUNT,
        "strictMappedParticleCount": EXPECTED_STRICT_MAPPED_PARTICLE_COUNT,
        "excludedParticleCount": EXPECTED_EXCLUDED_PARTICLE_COUNT,
        "legacySelectedParticleCount": 2160,
        "legacySelectedExcludedCount": EXPECTED_CURRENT_EXCLUDED_PARTICLE_COUNT,
        "legacySelectedRetargetedCount": EXPECTED_CURRENT_RETARGETED_PARTICLE_COUNT,
        "drawableDecisionCorpusCount": EXPECTED_DRAWABLE_DECISION_CORPUS_COUNT,
        "sourceDecalCount": EXPECTED_SOURCE_DECAL_COUNT,
        "sourceDecalBaseReadyCount": EXPECTED_SOURCE_DECAL_BASE_COUNT,
        "sourceDecalIncompleteCount": EXPECTED_SOURCE_DECAL_MISSING_BASE_COUNT,
        "sourceDecalSourceResourceBindingCount": (
            EXPECTED_SOURCE_DECAL_SOURCE_RESOURCE_BINDING_COUNT
        ),
        "sourceAnimationTrailNotifyCount": (
            EXPECTED_SOURCE_ANIMATION_TRAIL_NOTIFY_COUNT
        ),
        "sourceAnimationTrailElementCount": (
            EXPECTED_SOURCE_ANIMATION_TRAIL_ELEMENT_COUNT
        ),
        "characterGhostCueCount": EXPECTED_CHARACTER_GHOST_CUE_COUNT,
        "characterGhostTargetCount": EXPECTED_CHARACTER_GHOST_TARGET_COUNT,
        "outputElementCount": EXPECTED_OUTPUT_ELEMENT_COUNT,
        "placeholderTrailExcludedCount": EXPECTED_PLACEHOLDER_TRAIL_COUNT,
        "drawableAdmittedCount": EXPECTED_DRAWABLE_ADMITTED_COUNT,
        "portableFailClosedCount": EXPECTED_PORTABLE_FAIL_CLOSED_COUNT,
        "dimensionMaster2050210GoldenElementCount": (
            EXPECTED_DM_A_GOLDEN_ELEMENT_COUNT
        ),
        "dimensionMaster2050210GoldenAdmittedCount": (
            EXPECTED_DM_A_GOLDEN_ADMITTED_COUNT
        ),
        "dimensionMaster2050210GoldenFailClosedCount": (
            EXPECTED_DM_A_GOLDEN_FAIL_CLOSED_COUNT
        ),
        "canonicalMaterialJoinedCount": EXPECTED_CANONICAL_MATERIAL_JOINED_COUNT,
        "sharedMaterialContractMatchedCount": (
            EXPECTED_SHARED_MATERIAL_CONTRACT_MATCHED_COUNT
        ),
        "sharedMaterialContractResolvedCount": (
            EXPECTED_SHARED_MATERIAL_CONTRACT_RESOLVED_COUNT
        ),
        "sharedMaterialProfileJoinedCount": (
            EXPECTED_SHARED_MATERIAL_PROFILE_JOINED_COUNT
        ),
        "sharedMaterialOccurrenceProfilePromotedCount": (
            EXPECTED_SHARED_MATERIAL_OCCURRENCE_PROMOTED_COUNT
        ),
        "sharedMaterialCanonicalFallbackReplacedCount": (
            EXPECTED_SHARED_MATERIAL_CANONICAL_FALLBACK_REPLACED_COUNT
        ),
        "sourceProfileEnabledCount": EXPECTED_SOURCE_PROFILE_ENABLED_COUNT,
        "sourceProfileReadyCount": EXPECTED_SOURCE_PROFILE_READY_COUNT,
        "sourceProfileBlockedCount": EXPECTED_SOURCE_PROFILE_BLOCKED_COUNT,
        "namedTextureCount": EXPECTED_NAMED_TEXTURE_COUNT,
        "exactNamedTextureCount": EXPECTED_EXACT_NAMED_TEXTURE_COUNT,
        "rebasedNamedTextureCount": EXPECTED_REBASED_NAMED_TEXTURE_COUNT,
        "manifestNamedTextureCount": EXPECTED_MANIFEST_NAMED_TEXTURE_COUNT,
        "canonicalV12PackageQualifiedAssetCount": (
            EXPECTED_CANONICAL_V12_PACKAGE_QUALIFIED_ASSET_COUNT
        ),
        "canonicalV12PackageQualifiedElementCount": (
            EXPECTED_CANONICAL_V12_PACKAGE_QUALIFIED_ELEMENT_COUNT
        ),
        "nonExactNamedTextureAliasCount": (
            EXPECTED_NON_EXACT_NAMED_TEXTURE_ALIAS_COUNT
        ),
        "nonExactNamedTextureAliasElementCount": (
            EXPECTED_NON_EXACT_NAMED_TEXTURE_ALIAS_ELEMENT_COUNT
        ),
        "unresolvedNamedTextureCount": EXPECTED_UNRESOLVED_NAMED_TEXTURE_COUNT,
        "dimensionMasterBaseLessGroupedSpriteAdmittedCount": (
            EXPECTED_DM_BASELESS_GROUPED_SPRITE_ADMITTED_COUNT
        ),
    }
    if validate_expected_denominators:
        for key, expected_value in expected.items():
            if counts[key] != expected_value:
                raise RestorationError(
                    f"four-class restoration denominator changed: {key}="
                    f"{counts[key]} expected={expected_value}"
                )

    deferred_reason_counts: Counter[str] = Counter()
    fail_closed_reason_counts: Counter[str] = Counter()
    for row in receipts:
        deferred_reason_counts.update(row["deferredReasonCounts"])
        fail_closed_reason_counts.update(row["failClosedReasonCounts"])
    if sum(deferred_reason_counts.values()) != counts[
        "sourcePreservedDeferredCount"
    ]:
        raise RestorationError("deferred capability reason denominator changed")
    if (
        counts["portableCount"] + counts["sourcePreservedDeferredCount"]
        != counts["strictMappedParticleCount"]
    ):
        raise RestorationError("portable/deferred Particle denominator changed")
    if (
        counts["drawableAdmittedCount"] + counts["portableFailClosedCount"]
        != counts["portableCount"]
    ):
        raise RestorationError("portable admission denominator changed")
    if (
        counts["drawableAdmittedCount"]
        + counts["authoringApproximateCount"]
        + counts["hardPortableFailClosedCount"]
        != counts["portableCount"]
        or counts["previewTargetCount"]
        != counts["drawableAdmittedCount"]
        + counts["authoringApproximateCount"]
        or counts["portableFailClosedCount"]
        != counts["authoringApproximateCount"]
        + counts["hardPortableFailClosedCount"]
    ):
        raise RestorationError(
            "portable product/approximate/hard-fail denominator changed"
        )
    if (
        counts["sourceProfileReadyCount"] + counts["sourceProfileBlockedCount"]
        != counts["sourceProfileEnabledCount"]
    ):
        raise RestorationError("source Material profile readiness denominator changed")
    if (
        counts["exactNamedTextureCount"]
        + counts["manifestNamedTextureCount"]
        + counts["unresolvedNamedTextureCount"]
        != counts["namedTextureCount"]
        or counts["rebasedNamedTextureCount"]
        > counts["exactNamedTextureCount"]
    ):
        raise RestorationError("source Material named texture denominator changed")

    exclusion_reason_counts = Counter(row.reason for row in particle_exclusions)
    exclusion_rows = []
    for row in particle_exclusions:
        exclusion_identity = "\0".join(
            (
                row.character_class,
                str(row.skill_id),
                row.source_effect_id,
                row.source_element_id,
                row.source_event_id,
                row.reason,
            )
        )
        exclusion_rows.append(
            {
                "exclusionId": "excluded.source-particle."
                + hashlib.sha256(exclusion_identity.encode("utf-8")).hexdigest()[:24],
                "characterClass": row.character_class,
                "skillId": row.skill_id,
                "sourceEffectAssetId": row.source_effect_id,
                "sourceElementId": row.source_element_id,
                "sourceEventId": row.source_event_id,
                "reason": row.reason,
            }
        )

    source_profile_coverage_by_class = {
        character_class: {
            "canonicalMaterialJoinedCount": sum(
                row["canonicalMaterialJoinedCount"]
                for row in receipts
                if row["characterClass"] == character_class
            ),
            "sharedMaterialContractMatchedCount": sum(
                row["sharedMaterialContractMatchedCount"]
                for row in receipts
                if row["characterClass"] == character_class
            ),
            "sharedMaterialContractResolvedCount": sum(
                row["sharedMaterialContractResolvedCount"]
                for row in receipts
                if row["characterClass"] == character_class
            ),
            "sharedMaterialProfileJoinedCount": sum(
                row["sharedMaterialProfileJoinedCount"]
                for row in receipts
                if row["characterClass"] == character_class
            ),
            "sharedMaterialCanonicalFallbackReplacedCount": sum(
                row["sharedMaterialCanonicalFallbackReplacedCount"]
                for row in receipts
                if row["characterClass"] == character_class
            ),
            "enabledCount": sum(
                row["sourceProfileEnabledCount"]
                for row in receipts
                if row["characterClass"] == character_class
            ),
            "readyCount": sum(
                row["sourceProfileReadyCount"]
                for row in receipts
                if row["characterClass"] == character_class
            ),
            "blockedCount": sum(
                row["sourceProfileBlockedCount"]
                for row in receipts
                if row["characterClass"] == character_class
            ),
            "rebasedNamedTextureCount": sum(
                row["rebasedNamedTextureCount"]
                for row in receipts
                if row["characterClass"] == character_class
            ),
        }
        for character_class in CLASS_ORDER
    }

    receipt: dict[str, Any] = {
        "schema": "lostark.four-class-track-a-restoration-receipt",
        "formatVersion": 2,
        "contractRole": "OFFLINE_CANDIDATE_NOT_PRODUCT_MAPPING",
        "sourceBatch": {
            "path": BATCH_PATH.relative_to(ROOT).as_posix(),
            "rawSha256": raw_sha256(BATCH_PATH),
            "canonicalJsonSha256": canonical_sha256(batch),
        },
        "ownership": {
            "compilerOwned": [
                "stableSourceOccurrenceIdentity",
                "sourceDetailOnFirstMaterialization",
                "sourceRecipe",
                "sourceNode",
                "kind",
                "exactSourceResourceBindings",
                "receiptProvenSupplementalBaseOnly",
                "sourceMaterialIdentity",
                "sourceMaterialProfileAndAdmission",
                "exactCanonicalMaterialProjectionWhenAvailable",
                "exactSourceReceiptNamedTextureRuntimeAssetIds",
                "canonicalV12PackageQualifiedNamedTextureAssetIds",
                "sourceDecalRecipeAndNonBaseBindings",
                "animationTrailSourceEventEmitterTypeDataAndMaterialIdentity",
                "animationTrailWeaponBoneHistoryAttachment",
                "animationTrailNonBaseBindings",
                "characterGhostRawNotifyPayloadAndSourceOccurrenceIdentity",
            ],
            "artistOwnedPreservedOnReimport": [
                "onlyAfterStableSourceOccurrenceIdExists",
                "displayNameAndGroupId",
                "visibleWhenPortable",
                "detail",
                "actionCueAttachment",
                "transformInheritance",
                "authoringOverridesResourcesScalarsAndColors",
                "validOverridesReappliedAfterCompilerRefresh",
                "decalBaseDiffuseAndMaterial",
                "animationTrailVisibleDetailAndBaseDiffuse",
            ],
            "authoringOverrideDropReasons": list(
                AUTHORING_OVERRIDE_DROP_REASONS
            ),
            "documentOwnedPreserved": [
                "modelCuesIncludingAlphaMode",
                "allNonElementDocumentFields",
            ],
            "forcedFailClosed": (
                "unsupported ordinary-v13 recipe capability, non-executable "
                "source-profile resource contract, missing drawable carrier, "
                "non-exact source material identity, or object-name-only named "
                "texture alias"
            ),
            "animationTrail": (
                "11 stable source-event/emitter occurrences from 8 Trails notifies; "
                "ordinary Trail history follows b_weapon_rhand, sourceRecipe remains "
                "disabled so Cascade Ribbon can never enter this family, and exact "
                "source Material identity/resources are retained for artist tuning"
            ),
            "characterGhost": (
                "72 byte-lossless TrailGhostEffect occurrences target 29 authored "
                "documents as CHARACTER_AFTERIMAGE receipt evidence only; they do "
                "not become generic Trail Elements and remain fail-closed until a "
                "body/equipment pose snapshot plus ghost Material runtime exists"
            ),
        },
        "resourceContract": {
            "wmodel": "EXACT_SOURCE_ELEMENT",
            "meshModelPreScale": 0.01,
            "sourceBindings": "EXACT_SOURCE_ELEMENT",
            "sourceResourceBindingCount": counts["sourceResourceBindingCount"],
            "directSourceResourceElementCount": counts[
                "directSourceResourceElementCount"
            ],
            "receiptResourceProjectionElementCount": counts[
                "receiptResourceProjectionElementCount"
            ],
            "receiptSupplementalBindingCount": counts[
                "receiptSupplementalBindingCount"
            ],
            "supplementalPolicy": (
                "only a hash-pinned approximation receipt may alias a same-source "
                "texture to Base; legacy starter lanes are never copied"
            ),
            "canonicalV12PackageQualifiedAssetAuthority": {
                "provenance": CANONICAL_V12_PACKAGE_QUALIFIED_ASSET_ID,
                "characterClass": "DIMENSIONMASTER",
                "skillId": 2050210,
                "canonicalEffectAssetId": DM_A_CANONICAL_EFFECT_ID,
                "canonicalRawSha256": DM_A_CANONICAL_V12_RAW_SHA256,
                "assetCount": counts[
                    "canonicalV12PackageQualifiedAssetCount"
                ],
                "elementCount": counts[
                    "canonicalV12PackageQualifiedElementCount"
                ],
                "policy": (
                    "EXACT_STABLE_ELEMENT_PROFILE_PLUS_PACKAGE_OBJECT_"
                    "MANIFEST_PLUS_SAMPLING_PACKAGE_SHA_PLUS_PHYSICAL_DDS"
                ),
            },
            "decal": {
                "sourceCount": counts["sourceDecalCount"],
                "sourceBindingCount": counts[
                    "sourceDecalSourceResourceBindingCount"
                ],
                "baseReadyCount": counts["sourceDecalBaseReadyCount"],
                "authoringIncompleteMissingBaseCount": counts[
                    "sourceDecalIncompleteCount"
                ],
                "baseOwnership": "ARTIST_WINS_WHEN_BOUND",
                "missingBasePolicy": "HIDDEN_MATERIAL_EXECUTION_FAIL_CLOSED",
            },
            "animationTrail": {
                "sourceNotifyCount": counts["sourceAnimationTrailNotifyCount"],
                "sourceElementCount": counts[
                    "sourceAnimationTrailElementCount"
                ],
                "historyRuntimeProfileId": (
                    "four-class.animation-trail-history.v1"
                ),
                "sourceRecipePolicy": (
                    "DISABLED_ANIMATION_TRAIL_IS_NOT_CASCADE_RIBBON"
                ),
                "attachment": "FOLLOW_EXACT_B_WEAPON_RHAND",
                "materialExecution": (
                    "BOUNDED_GENERIC_PREVIEW_EXACT_IDENTITY_ARTIST_TUNED"
                ),
            },
            "characterGhost": {
                "semanticFamily": CHARACTER_GHOST_SEMANTIC_FAMILY,
                "sourceCueCount": counts["characterGhostCueCount"],
                "targetDocumentCount": counts["characterGhostTargetCount"],
                "admission": "FAIL_CLOSED_RECEIPT_ONLY",
                "runtimeBlocker": CHARACTER_GHOST_RUNTIME_BLOCKER,
                "genericTrailElementProjection": False,
            },
            "allElementBindingsAndNonemptyNamedTextureBindingsPhysicallyPresent": True,
        },
        "sourceProfileCoverageByClass": source_profile_coverage_by_class,
        "counts": counts,
        "deferredReasonCounts": dict(sorted(deferred_reason_counts.items())),
        "failClosedReasonCounts": dict(sorted(fail_closed_reason_counts.items())),
        "particleExclusionReasonCounts": dict(
            sorted(exclusion_reason_counts.items())
        ),
        "particleExclusions": exclusion_rows,
        "characterGhostCues": character_ghost_cues,
        "productMutation": False,
        "visualApproval": False,
        "targets": receipts,
    }
    receipt["artifactSha256"] = canonical_sha256(receipt)
    outputs[RECEIPT_PATH] = serialized(receipt)
    return outputs, receipt


def commit_transaction(outputs: dict[Path, str]) -> None:
    prepared: dict[Path, Path] = {}
    backups: dict[Path, Path | None] = {}
    committed: list[Path] = []
    commit_succeeded = False
    try:
        for path, value in outputs.items():
            path.parent.mkdir(parents=True, exist_ok=True)
            descriptor, temporary_name = tempfile.mkstemp(
                prefix=f".{path.name}.", suffix=".stage", dir=path.parent
            )
            temporary = Path(temporary_name)
            with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as stream:
                stream.write(value)
                stream.flush()
                os.fsync(stream.fileno())
            if json.loads(temporary.read_text(encoding="utf-8")) != json.loads(value):
                raise RestorationError(f"staged JSON changed before commit: {path}")
            prepared[path] = temporary

        for path in sorted(outputs, key=lambda item: item.as_posix()):
            backup: Path | None = None
            if path.exists():
                descriptor, backup_name = tempfile.mkstemp(
                    prefix=f".{path.name}.", suffix=".rollback", dir=path.parent
                )
                os.close(descriptor)
                backup = Path(backup_name)
                backup.unlink()
                os.replace(path, backup)
            backups[path] = backup
            os.replace(prepared[path], path)
            prepared.pop(path, None)
            committed.append(path)
        commit_succeeded = True
    except Exception as commit_error:
        rollback_errors: list[str] = []
        for path in reversed(committed):
            try:
                if path.exists():
                    path.unlink()
                backup = backups.get(path)
                if backup is not None and backup.exists():
                    os.replace(backup, path)
                    backups[path] = None
            except OSError as rollback_error:
                rollback_errors.append(f"{path}: {rollback_error}")
        for path, backup in backups.items():
            if path not in committed and backup is not None and backup.exists():
                try:
                    os.replace(backup, path)
                    backups[path] = None
                except OSError as rollback_error:
                    rollback_errors.append(f"{path}: {rollback_error}")
        if rollback_errors:
            raise RestorationError(
                "candidate commit failed and rollback needs manual recovery; "
                "rollback files were retained: " + "; ".join(rollback_errors)
            ) from commit_error
        raise
    finally:
        for temporary in prepared.values():
            try:
                temporary.unlink()
            except OSError:
                pass
        if commit_succeeded:
            for backup in backups.values():
                if backup is not None:
                    try:
                        backup.unlink()
                    except OSError:
                        pass


def check_outputs(outputs: dict[Path, str]) -> None:
    stale: list[str] = []
    for path, expected in outputs.items():
        if not path.is_file():
            stale.append(f"missing:{path.relative_to(ROOT).as_posix()}")
            continue
        try:
            current = json.loads(path.read_text(encoding="utf-8-sig"))
        except (OSError, json.JSONDecodeError) as error:
            raise RestorationError(f"cannot check output {path}: {error}") from error
        if current != json.loads(expected):
            stale.append(f"drift:{path.relative_to(ROOT).as_posix()}")
    if stale:
        preview = ", ".join(stale[:10])
        raise RestorationError(
            f"four-class Track A outputs are stale ({len(stale)}): {preview}"
        )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--dry-run", action="store_true")
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--write", action="store_true")
    parser.add_argument(
        "--target",
        action="append",
        default=[],
        metavar="CLASS:SKILL:STAGE:STAGE_CLIP:EFFECT_ID",
        help=(
            "Materialize one manifest-marked TARGETED_CURRENT_COMBAT source-only "
            "target. Repeat for an atomic multi-target transaction."
        ),
    )
    args = parser.parse_args()

    outputs, receipt = (
        build_targeted_projection(args.target)
        if args.target
        else build_projection()
    )
    counts = receipt["counts"]
    if args.check:
        check_outputs(outputs)
        print(f"Four-class Track A candidate check PASS: {counts}")
        return 0
    if args.write:
        commit_transaction(outputs)
        check_outputs(outputs)
        print(f"Four-class Track A candidates materialized: {counts}")
        return 0
    print(f"Four-class Track A candidate dry-run PASS: {counts}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except RestorationError as error:
        print(f"ERROR: {error}")
        raise SystemExit(1)
