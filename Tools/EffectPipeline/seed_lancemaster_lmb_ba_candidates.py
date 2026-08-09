#!/usr/bin/env python3
"""Seed LanceMaster 34010/34510 manual restoration candidates.

The candidates are deliberately separate from Product Effect IDs.  They pin
source evidence and provide a manual starting stack.  Existing candidates are
never overwritten except by the explicit, hash-guarded v12-to-v13 migration;
this tool never publishes a candidate to Character Select.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import os
from pathlib import Path
import re
import tempfile
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
DATA_ROOT = REPOSITORY_ROOT / "Data"
AUTHORED_ROOT = DATA_ROOT / "Effects/Authored"
CANDIDATE_RECEIPT_ROOT = (
    DATA_ROOT
    / "Effects/AuthoredCorrections/Generated/TrackB/LanceMasterLmbCandidates"
)
DECAL_INVENTORY_PATH = (
    DATA_ROOT
    / "Effects/AuthoredCorrections/Generated/TrackB/"
    "lancemaster-artist.decal-inventory.json"
)
LONG_IMPORTED_PATH = (
    DATA_ROOT
    / "Effects/Imported/LanceMaster/Converted/"
    "effect.lancemaster.skill.34010.imported.effect.json"
)
SHORT_IMPORTED_PATH = (
    DATA_ROOT
    / "Effects/Imported/LanceMaster/CurrentCombat/Converted/"
    "effect.lancemaster.skill.34510.imported.effect.json"
)
ANIMNOTIFY_PATH = (
    DATA_ROOT / "Animation/Reference/LanceMaster/LanceMaster.animnotify"
)
ANIMEVENTS_PATH = (
    DATA_ROOT / "Animation/Authored/LanceMaster/LanceMaster.animevents"
)

PINNED_TRACKED_INPUTS = {
    LONG_IMPORTED_PATH: (
        "1759765544d2b7e8fb316a3abf81a8c72588db56beaccc0212e556c6673a5dd2"
    ),
    SHORT_IMPORTED_PATH: (
        "0bb82e2df5bda23faf573d179e5fbb616a979105cf4957c123fba992da38e1bf"
    ),
    ANIMNOTIFY_PATH: (
        "733ebeabf5d4f388b43152b8cf6e4d78fabd1af349e85228c2a3555971f70155"
    ),
    AUTHORED_ROOT / "effect.lancemaster.skill.34010.ba1.effect.json": (
        "e3605d1efe8d187f7d1d2fa3787aa3d659311f4776e3322c9a68cbbb4dae8f1f"
    ),
    AUTHORED_ROOT / "effect.lancemaster.skill.34010.ba2.effect.json": (
        "2ac28bf821030ab21f3a3cd63a1a39d3f8eab1f9733b974df3a015d5d314dcd1"
    ),
    AUTHORED_ROOT / "effect.lancemaster.skill.34010.ba3.effect.json": (
        "f5b88b109c16c6a86f04f5cb749f3a5fdad748e9d57178a6008a6bc733dd23dd"
    ),
    AUTHORED_ROOT / "effect.lancemaster.skill.34010.ba4.effect.json": (
        "2c11f8d05562c16c98546bd1184ef98c80bc849a07d66d0e22533c371597fd90"
    ),
    AUTHORED_ROOT / "effect.lancemaster.skill.34510.ba1.effect.json": (
        "3a52eba5e23fb0a20873d2042d9305080fe9b8af2689b495c0be954d4563601d"
    ),
    AUTHORED_ROOT / "effect.lancemaster.skill.34510.ba2.effect.json": (
        "ec288e2c2d0015281d0296dfffc3f790e223f1704a48355d63bf2b629e14b5cc"
    ),
    AUTHORED_ROOT / "effect.lancemaster.skill.34510.ba3.effect.json": (
        "e6b6e422be0c70a040ab09f56e6372db80ee243bffeca4455354554bb9c55fa5"
    ),
}

# Git/worktree checkout policy changes this authored text between CRLF and LF.
# Pin its semantic UTF-8/LF content so the same evidence is portable without
# weakening the byte-exact pins used for the JSON source documents above.
PINNED_LF_TEXT_INPUTS = {
    ANIMEVENTS_PATH: (
        "ad2e04d04b7008bb52f1ce63570d5784dfc06ef12ec2a6c3ae4f170917d8ca74"
    ),
}

PRODUCT_VISUAL_STATUS = "CARRIER_INVENTORY_VISUAL_UNVERIFIED"
PRODUCT_GROUP_RISKS_BY_SKILL = {
    34010: [
        "EXTREME_EMISSIVE_SOURCE_VALUE_REQUIRES_SHADER_PARITY",
        "RECONSTRUCTED_MATERIAL_PROFILE_VISUAL_UNVERIFIED",
    ],
    34510: [],
}

EXTERNAL_MATERIAL_EVIDENCE = (
    {
        "pathHint": (
            "C:/Users/user/Desktop/Resource_LostArk/05_Reports/EffectExtraction/"
            "DIMENSIONMASTER/materials/zhj4tc4pck4pc4j22hixeyuxeu.materials.json"
        ),
        "sha256": (
            "821148cbedE5249ede6e4c9625cf47285fd656fd3f6cba86e70912d7c6ad8b48"
        ).lower(),
        "owns": [
            "fx_m_pa_ribbonmaster_03_3_tr",
            "fx_m_pa_missiletrail_01_17_tr",
        ],
    },
    {
        "pathHint": (
            "C:/Users/user/Desktop/Resource_LostArk/05_Reports/EffectExtraction/"
            "DIMENSIONMASTER/materials/ygi3sb3obj3o1mgump6qmp8b5.materials.json"
        ),
        "sha256": (
            "b5b2fc6021191c59da2875b18426bf8f4dc5f9ddb1766d55078d7b9bd2ea3c7f"
        ),
        "owns": ["fx_m_pa_ribbonmaster_01_9_tr"],
    },
    {
        "pathHint": (
            "C:/Users/user/Desktop/Resource_LostArk/05_Reports/EffectExtraction/"
            "DIMENSIONMASTER/materials/ygi3sb3obj3o1fgump6qmp8y5.materials.json"
        ),
        "sha256": (
            "496b01ea6040c74d06514200940ea3a98204263179e52507a06e159a55ef03a0"
        ),
        "owns": [
            "fx_k_me_makeflow_02_38_tr",
            "fx_k_me_makeflow_02_39_tr",
            "fx_k_pa_veldust_01_tr",
        ],
    },
    {
        "pathHint": (
            "C:/Users/user/Desktop/Resource_LostArk/05_Reports/EffectExtraction/"
            "DIMENSIONMASTER/materials/ygi3sb3obj3o11gump6qmp885.materials.json"
        ),
        "sha256": (
            "1852b140d3182c38f5377c420d1dd83e6eb3063fec8e0cc5b6a06581dee085f2"
        ),
        "owns": ["fx_a_pa_db_01_1_ad"],
    },
    {
        "pathHint": (
            "C:/Users/user/Desktop/Resource_LostArk/05_Reports/EffectExtraction/"
            "DIMENSIONMASTER/materials/ygi3sb3obj3o18gump6qmp8l5.materials.json"
        ),
        "sha256": (
            "8824cf428a59b1df56d156371b8bf5648407e79dee8854807ef03d58a22ac121"
        ),
        "owns": ["fx_k_pa_fd_01_05_tr", "fx_e_pa_ht_08_1_ad"],
    },
)

TRAIL_MATERIAL_PATH = "fx_m_mi_m_00.fx_mi.fx_m_pa_ribbonmaster_03_3_tr"
RIBBON_MATERIAL_PATH = "fx_m_mi_m_00.fx_mi.fx_m_pa_ribbonmaster_01_9_tr"
TRAIL_RESOURCES = (
    {
        "slotId": "base",
        "assetId": "Effect/LanceMaster/Textures/fx_h_atypical_01_1.dds",
    },
    {
        "slotId": "mask",
        "assetId": "Effect/LanceMaster/Textures/fx_m_trail_006.dds",
    },
    {
        "slotId": "emissive",
        "assetId": "Effect/LanceMaster/Textures/fx_h_atypical_01_1.dds",
    },
    {
        "slotId": "dissolve",
        "assetId": "Effect/LanceMaster/Textures/fx_k_caustictile_01.dds",
    },
    {
        "slotId": "noise",
        "assetId": "Effect/LanceMaster/Textures/fx_d_noise_030.dds",
    },
)
TRAIL_SOURCE_TEXTURE_ROLES = (
    {
        "parameter": "alpha_texture",
        "sourceTexture": "fx_tex_05.fx_m_trail_006",
        "candidateSlotId": "mask",
        "decision": "EXACT_ASSET_AND_ROLE",
    },
    {
        "parameter": "dissolve_tex_01",
        "sourceTexture": "fx_tex_05.fx_k_caustictile_01",
        "candidateSlotId": "dissolve",
        "decision": "EXACT_ASSET_AND_ROLE",
    },
    {
        "parameter": "emissive_tex_01",
        "sourceTexture": "fx_tex_04.fx_h_atypical_01_1",
        "candidateSlotId": "emissive",
        "decision": "EXACT_ASSET_AND_ROLE",
    },
    {
        "parameter": "1_uv_noise_texture",
        "sourceTexture": "fx_tex_02.fx_d_noise_030",
        "candidateSlotId": "noise",
        "decision": "EXACT_ASSET_AND_ROLE",
    },
    {
        "parameter": "uv_noise_texture_emissive",
        "sourceTexture": "fx_tex_02.fx_d_noise_030",
        "candidateSlotId": "noise",
        "decision": "EXACT_ASSET_ALIAS_SHARED_WITH_NOISE",
    },
    {
        "parameter": "candidate-preview-base",
        "sourceTexture": "fx_tex_04.fx_h_atypical_01_1",
        "candidateSlotId": "base",
        "decision": "SAME_MATERIAL_PREVIEW_ALIAS_NOT_SOURCE_GRAPH_EXACT",
    },
)

RING_PARTICLE_COLOR = [0.2, 0.375, 0.5, 0.5]

LONG_IMPACT_ROLES = (
    {
        "roleId": "impact-master",
        "emitter": 7,
        "kind": "mesh",
        "master": True,
        "scale": [0.02, 0.01, 0.02],
    },
    {
        "roleId": "impact-wave",
        "emitter": 5,
        "kind": "sprite",
        "master": False,
        "scale": [0.55, 2.3, 1.0],
    },
    {
        "roleId": "impact-glow-short",
        "emitter": 1,
        "kind": "sprite",
        "master": False,
        "scale": [1.0, 0.01, 1.0],
    },
    {
        "roleId": "impact-glow-long",
        "emitter": 21,
        "kind": "sprite",
        "master": False,
        "scale": [1.0, 0.01, 1.0],
    },
)

LONG_STAGES = (
    {
        "ba": 1,
        "clip": "flm_att_identity1_1_01",
        "target": "effect.lancemaster.skill.34010.ba1",
        "trailSystem": "FX_PC_FLM_01.Par_M_FLM_Trail_03",
        "trailNotifyObjectId": "LanceMaster_25010_0_1_0",
        "trailStart": 0.404013991,
        "trailDuration": 0.363292009,
        "ribbonStart": 0.449099988,
        "ribbonDuration": 0.360033005,
        "ribbonSourceSuffix": "",
        "ringSourceSuffix": "",
        "ringStart": 0.5207,
    },
    {
        "ba": 2,
        "clip": "flm_att_identity1_1_02",
        "target": "effect.lancemaster.skill.34010.ba2",
        "trailSystem": "FX_PC_FLM_01.Par_M_FLM_Trail_03_2",
        "trailNotifyObjectId": "LanceMaster_25010_0_2_0",
        "trailStart": 0.407081008,
        "trailDuration": 0.145661995,
        "ribbonStart": 0.360055,
        "ribbonDuration": 0.250703007,
        "ribbonSourceSuffix": ".event_source-event-007",
        "ringSourceSuffix": ".event_source-event-009",
        "ringStart": 0.4376,
    },
    {
        "ba": 3,
        "clip": "flm_att_identity1_1_03",
        "target": "effect.lancemaster.skill.34010.ba3",
        "trailSystem": "FX_PC_FLM_01.Par_M_FLM_Trail_03",
        "trailNotifyObjectId": "LanceMaster_25010_0_3_0",
        "trailStart": 0.466607988,
        "trailDuration": 0.301800013,
        "ribbonStart": 0.490570009,
        "ribbonDuration": 0.224956006,
        "ribbonSourceSuffix": ".event_source-event-014",
        "ringSourceSuffix": ".event_source-event-017",
        "ringStart": 0.5501999999999998,
    },
    {
        "ba": 4,
        "clip": "flm_att_identity1_1_04",
        "target": "effect.lancemaster.skill.34010.ba4",
        "trailSystem": "FX_PC_FLM_01.Par_M_FLM_Trail_03_2",
        "trailNotifyObjectId": "LanceMaster_25010_0_4_0",
        "trailStart": 0.422839999,
        "trailDuration": 0.454925001,
        "ribbonStart": 0.425729007,
        "ribbonDuration": 0.451970994,
        "ribbonSourceSuffix": ".event_source-event-022",
        "ringSourceSuffix": ".event_source-event-024",
        "ringStart": 0.4673000000000007,
        "impactStart": 0.18880000000000052,
    },
)

SHORT_STAGES = (
    {
        "ba": 1,
        "clip": "flm_att_identity2_1_01",
        "target": "effect.lancemaster.skill.34510.ba1",
        "sourceSuffix": "",
    },
    {
        "ba": 2,
        "clip": "flm_att_identity2_1_02",
        "target": "effect.lancemaster.skill.34510.ba2",
        "sourceSuffix": ".event_source-event-006",
    },
    {
        "ba": 3,
        "clip": "flm_att_identity2_1_03",
        "target": "effect.lancemaster.skill.34510.ba3",
        "sourceSuffix": ".event_source-event-010",
    },
)

# The only documents that --migrate-v13 may replace.  These are the exact
# v12 carrier-inventory seeds created by the previous generator.  Validation
# is completed for all seven documents before any output is staged so a user
# save in even one candidate fails the whole migration closed.
LEGACY_V12_CANDIDATE_SHA256 = {
    "effect.lancemaster.skill.34010.ba1": (
        "db6195e14c7e77bfc9dee71362ea856eb31017736ab8ef82dd1ec48d625cf9b1"
    ),
    "effect.lancemaster.skill.34010.ba2": (
        "760a53b4b790d5785692f989527ede5ffe5a7f31756fed44ca8312ef85b7c6bb"
    ),
    "effect.lancemaster.skill.34010.ba3": (
        "8b0b2f5796dca7d27175d8bb86ae392dd46a6505a25de424d514ddb846a3a758"
    ),
    "effect.lancemaster.skill.34010.ba4": (
        "6c5efcd71b4488cde4340280fb5e01a8e27fef63cdf73cd5331970a36b9565d0"
    ),
    "effect.lancemaster.skill.34510.ba1": (
        "7907685469f8f8f94dbd940257511164a094e96c2d506a78911a49e64a0632f5"
    ),
    "effect.lancemaster.skill.34510.ba2": (
        "1122fc9288d5cb9614f54d254f4f7e6accd42ac58c886b7f60949f17c04c9b3b"
    ),
    "effect.lancemaster.skill.34510.ba3": (
        "1e9455cba4e61871c67fd4c56fdb930f5ec378ec9ab0e734f4f6b785052ab6bd"
    ),
}

# Exact first-pass v13 outputs produced by --migrate-v13 before the source
# audit exposed the separate TypeDataRibbon carrier.  This one-shot manifest
# permits adding an invisible fallback-blocked Ribbon queue entry without
# overwriting any subsequent Effect Tool save.
PRE_RIBBON_V13_CANDIDATE_SHA256 = {
    "effect.lancemaster.skill.34010.ba1": (
        "e3d9444257afadce6445206cbb30fd22f2940c746bd5c08febc7d554170f3804"
    ),
    "effect.lancemaster.skill.34010.ba2": (
        "eb367decb5b3a22b8089b90a0910a2f3e5a38117abfaf2515f5fc48c256d35a1"
    ),
    "effect.lancemaster.skill.34010.ba3": (
        "b1055e40457bf17a2bbf1c7fc95ae9d7ffc671bdbfbde07ef7aa1a7473abaefe"
    ),
    "effect.lancemaster.skill.34010.ba4": (
        "eedf1b7d922f39503eb82f367278fdb2711524c6601332a0330f1268c74b13d4"
    ),
    "effect.lancemaster.skill.34510.ba1": (
        "9b26e991f9ede0bcf8509a4dfde43b86a6d8ea020dee1829d6f870223c33d5d2"
    ),
    "effect.lancemaster.skill.34510.ba2": (
        "2adeb3eb8a96a89f6a31a20f097326c87c6d163e9f38919972db374c348ed26b"
    ),
    "effect.lancemaster.skill.34510.ba3": (
        "32e376b6611d0a8ad25c8c46cd18c72e36d076e60f99c498d6195868d84eb54e"
    ),
}

SHORT_ROLES = (
    {
        "roleId": "short-flow-master",
        "emitter": 9,
        "kind": "mesh",
        "position": [0.0, 0.0, -0.3],
        "rotation": [0.0, -90.0, 0.0],
        "scale": [0.025, 0.01, 0.01],
        "master": True,
        "startOffset": 0.0,
    },
    {
        "roleId": "short-flow-pair",
        "emitter": 8,
        "kind": "mesh",
        "position": [0.0, 0.0, 0.3],
        "rotation": [0.0, -90.0, 0.0],
        "scale": [0.025, 0.01, 0.01],
        "master": False,
        "startOffset": 0.0,
    },
    {
        "roleId": "short-flow-cap",
        "emitter": 12,
        "kind": "mesh",
        "position": [2.0, 0.0, 0.0],
        "rotation": [0.0, 0.0, 0.0],
        "scale": [0.2, 0.07, 0.07],
        "master": False,
        "startOffset": 0.0,
    },
    {
        "roleId": "short-spatter-back",
        "emitter": 7,
        "kind": "sprite",
        "position": [-0.7, 0.0, 0.0],
        "rotation": [0.0, 0.0, 0.0],
        "scale": [0.2, 3.25, 1.0],
        "master": False,
        "startOffset": 0.0,
    },
    {
        "roleId": "short-fragment-manual-position",
        "emitter": 2,
        "kind": "sprite",
        "position": [0.0, 0.0, -0.3],
        "rotation": [0.0, 0.0, 0.0],
        "scale": [0.01, 0.07, 1.0],
        "master": False,
        "startOffset": 0.1,
        "positionDecision": "MANUAL_PENDING_RANDOM_DISTRIBUTION",
    },
    {
        "roleId": "short-fragment-back",
        "emitter": 11,
        "kind": "sprite",
        "position": [-1.0, 0.0, 0.0],
        "rotation": [0.0, 0.0, 0.0],
        "scale": [0.1, 0.45, 1.0],
        "master": False,
        "startOffset": 0.0,
    },
)

BLOCKED_SHORT_ROLES = (
    {
        "roleId": "short-veldust-low",
        "emitter": 0,
        "reason": "MANUAL_PENDING_TEXTURELESS_SOURCE_MATERIAL",
    },
    {
        "roleId": "short-veldust-high",
        "emitter": 1,
        "reason": "MANUAL_PENDING_TEXTURELESS_SOURCE_MATERIAL",
    },
)


def read_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as stream:
        value = json.load(stream)
    if not isinstance(value, dict):
        raise ValueError(f"JSON root must be an object: {path}")
    return value


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def file_sha256(path: Path) -> str:
    return sha256_bytes(path.read_bytes())


def lf_text_sha256(path: Path) -> str:
    # TextIO's universal-newline mode normalizes CRLF/CR to LF.
    return sha256_bytes(path.read_text(encoding="utf-8").encode("utf-8"))


def validate_pinned_inputs() -> None:
    for path, expected in PINNED_TRACKED_INPUTS.items():
        actual = file_sha256(path)
        if actual != expected:
            raise ValueError(f"Pinned source drifted: {path} ({actual})")
    for path, expected in PINNED_LF_TEXT_INPUTS.items():
        actual = lf_text_sha256(path)
        if actual != expected:
            raise ValueError(f"Pinned LF-normalized source drifted: {path} ({actual})")
    for evidence in EXTERNAL_MATERIAL_EVIDENCE:
        path = Path(evidence["pathHint"])
        if path.is_file() and file_sha256(path) != evidence["sha256"]:
            raise ValueError(f"External material evidence drifted: {path}")


def validated_product_target(stage: dict[str, Any], *, skill_id: int) -> dict[str, Any]:
    expected_target = f"effect.lancemaster.skill.{skill_id}.ba{stage['ba']}"
    if stage["target"] != expected_target:
        raise ValueError(f"Product target does not match stage: {stage['target']}")

    product_path = AUTHORED_ROOT / f"{stage['target']}.effect.json"
    expected_sha = PINNED_TRACKED_INPUTS.get(product_path)
    actual_sha = file_sha256(product_path)
    if expected_sha is None or actual_sha != expected_sha:
        raise ValueError(f"Pinned Product drifted: {stage['target']} ({actual_sha})")

    product_document = read_json(product_path)
    if product_document.get("effectAssetId") != stage["target"]:
        raise ValueError(f"Product ID drifted: {stage['target']}")

    grouped_elements: dict[str, list[str]] = {}
    for element in product_document.get("elements", []):
        if not isinstance(element, dict):
            raise ValueError(f"Product element is invalid: {stage['target']}")
        group_id = str(element.get("groupId") or "")
        element_id = str(element.get("id") or "")
        if not group_id or not element_id:
            raise ValueError(f"Product carrier ID is missing: {stage['target']}")
        grouped_elements.setdefault(group_id, []).append(element_id)
    if not grouped_elements:
        raise ValueError(f"Product has no carrier groups: {stage['target']}")

    risks = PRODUCT_GROUP_RISKS_BY_SKILL[skill_id]
    groups = [
        {
            "groupId": group_id,
            "masterElementId": element_ids[0],
            "masterSelectionBasis": "PROVISIONAL_FIRST_CARRIER",
            "manualAuditionOrder": element_ids,
            "risks": copy.deepcopy(risks),
        }
        for group_id, element_ids in grouped_elements.items()
    ]
    return {
        "documentPath": product_path.relative_to(REPOSITORY_ROOT).as_posix(),
        "documentSha256": actual_sha,
        "visualStatus": PRODUCT_VISUAL_STATUS,
        "groups": groups,
    }


def disabled_source_recipe() -> dict[str, Any]:
    return {
        "enabled": False,
        "rendererShape": "",
        "emitterDelaySeconds": 0.0,
        "emitterDurationSeconds": 0.0,
        "emitterLoopCount": 0,
        "bursts": [],
        "modules": [],
    }


def disabled_source_presentation() -> dict[str, Any]:
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


def root_attachment() -> dict[str, Any]:
    return {
        "enabled": True,
        "follow": False,
        "sourceAnchorSlotId": "root",
        "runtimeAnchorSlotId": "root",
        "runtimeBoneName": "",
        "socketLocalTransform": {
            "position": [0.0, 0.0, 0.0],
            "rotationDegrees": [0.0, 0.0, 0.0],
            "scale": [1.0, 1.0, 1.0],
        },
    }


def weapon_attachment() -> dict[str, Any]:
    value = root_attachment()
    value.update(
        {
            "follow": True,
            "sourceAnchorSlotId": "WP_FLM_1_Battle",
            "runtimeAnchorSlotId": "WP_FLM_1_Battle",
            "runtimeBoneName": "b_weapon_rhand",
        }
    )
    return value


def terminal_transform_master() -> dict[str, Any]:
    return {"enabled": False, "masterElementId": ""}


def inherit_master_transform(master_element_id: str) -> dict[str, Any]:
    return {"enabled": True, "masterElementId": master_element_id}


def reset_material(element: dict[str, Any], *, profile: str) -> None:
    source_path = str(element.get("material", {}).get("sourceMaterialPath") or "")
    element["material"] = {
        "templateId": "effect.standard",
        "sourceMaterialPath": source_path,
        "renderProfile": profile,
        "sourceProfile": {"enabled": False},
    }


def source_by_id(document: dict[str, Any]) -> dict[str, dict[str, Any]]:
    return {
        str(element.get("id")): element
        for element in document.get("elements", [])
        if isinstance(element, dict) and isinstance(element.get("id"), str)
    }


def resource_asset(element: dict[str, Any], slot_id: str) -> str:
    for binding in element.get("resources", []):
        if binding.get("slotId") == slot_id:
            return str(binding.get("assetId") or "")
    return ""


def validate_candidate_no_fallback(document: dict[str, Any]) -> None:
    for element in document.get("elements", []):
        kind = str(element.get("kind") or "")
        element_id = str(element.get("id") or "")
        if not element.get("visible"):
            if kind in {"mesh", "sprite", "trail", "decal"} and not resource_asset(
                element, "base"
            ):
                source_profile = element.get("material", {}).get("sourceProfile", {})
                if not (
                    source_profile.get("enabled") is True
                    and source_profile.get("runtimeShaderProfileId")
                    == "effect.ue3.fallback-blocked.v1"
                ):
                    raise ValueError(
                        "Hidden no-Base candidate is not explicitly fail-closed: "
                        f"{element_id}"
                    )
            continue
        if kind == "mesh" and not resource_asset(element, "meshModel"):
            raise ValueError(f"Visible candidate Mesh has no WModel: {element_id}")
        if kind in {"mesh", "sprite", "trail", "decal"} and not resource_asset(
            element, "base"
        ):
            raise ValueError(
                f"Visible candidate would use the renderer Base fallback: {element_id}"
            )


def candidate_id(target: str) -> str:
    return f"{target}.restoration-candidate"


def candidate_path(target: str) -> Path:
    return AUTHORED_ROOT / f"{candidate_id(target)}.effect.json"


def candidate_receipt_path(target: str) -> Path:
    return CANDIDATE_RECEIPT_ROOT / f"{target}.candidate-receipt.json"


def make_long_animtrail(
    template: dict[str, Any], stage: dict[str, Any]
) -> dict[str, Any]:
    element = copy.deepcopy(template)
    element.update(
        {
            "id": f"manual.trackb.ba{stage['ba']}.animtrail-companion",
            "displayName": "animtrail-companion",
            "groupId": f"manual.trackb.ba{stage['ba']}.weapon-trails",
            "sourceNode": stage["trailSystem"],
            "visible": True,
            "kind": "trail",
            "resources": copy.deepcopy(list(TRAIL_RESOURCES)),
            "material": {
                "templateId": "effect.standard",
                "sourceMaterialPath": TRAIL_MATERIAL_PATH,
                "renderProfile": "alpha_two_sided_depth_read",
                "sourceProfile": {"enabled": False},
            },
            "actionCueAttachment": weapon_attachment(),
            "transformInheritance": terminal_transform_master(),
            "sourceRecipe": disabled_source_recipe(),
            "sourcePresentation": disabled_source_presentation(),
        }
    )
    detail = element["detail"]
    detail["transform"] = {
        "position": [0.0, 0.0, 0.0],
        "rotationDegrees": [0.0, 0.0, 0.0],
        "revolutionDegreesPerSecond": [0.0, 0.0, 0.0],
        "scale": [1.0, 1.0, 1.0],
        "velocityPerSecond": [0.0, 0.0, 0.0],
    }
    detail["timing"] = {
        "startDelaySeconds": stage["trailStart"],
        "lifeTimeSeconds": stage["trailDuration"],
        "afterImageSeconds": 0.0,
        "dissolveStartNormalized": 1.0,
    }
    detail["color"] = {
        "offset": [0.0, 0.0, 0.0, 0.0],
        "multiply": [1.0, 1.0, 1.0, 1.0],
        "clip": 0.0,
        "emissiveIntensity": 0.0,
        "distortionIntensity": 0.0,
        "distortionOnBaseMaterial": False,
        "radialTime": 0.0,
        "radialIntensity": 0.0,
    }
    detail["mesh"] = {"useModelMaterial": False}
    detail["sprite"] = {"billboard": True, "billboardRollDegrees": 0.0}
    detail["trail"] = {
        "maxPoints": 64,
        "pointLifeTimeSeconds": 0.22,
        "sampleIntervalSeconds": 1.0 / 60.0,
        "minimumDistance": 0.01,
        "startWidth": 0.35,
        "endWidth": 0.04,
        "faceCamera": True,
    }
    detail["linearLerp"]["emissiveIntensity"] = False
    detail["linearLerp"]["endEmissiveIntensity"] = 0.0
    return element


def make_long_ring_master(
    source: dict[str, Any], stage: dict[str, Any]
) -> dict[str, Any]:
    element = copy.deepcopy(source)
    element.update(
        {
            "id": f"manual.trackb.ba{stage['ba']}.ring-master",
            "displayName": "ring-master",
            "groupId": f"manual.trackb.ba{stage['ba']}.ring-stack",
            "sourceNode": str(source["id"]),
            "visible": True,
            "kind": "mesh",
            "actionCueAttachment": root_attachment(),
            "transformInheritance": terminal_transform_master(),
            "sourceRecipe": disabled_source_recipe(),
            "sourcePresentation": disabled_source_presentation(),
        }
    )
    reset_material(element, profile="alpha_two_sided_depth_read")
    detail = element["detail"]
    detail["transform"]["position"] = [0.0, 0.0, 0.0]
    detail["transform"]["rotationDegrees"] = [0.0, 0.0, 0.0]
    detail["transform"]["revolutionDegreesPerSecond"] = [0.0, 0.0, 0.0]
    detail["transform"]["velocityPerSecond"] = [0.0, 0.0, 0.0]
    detail["transform"]["scale"] = [
        0.02700000047683716,
        0.03,
        0.02700000047683716,
    ]
    detail["timing"]["startDelaySeconds"] = stage["ringStart"]
    detail["timing"]["lifeTimeSeconds"] = 0.6000000238418579
    detail["color"]["multiply"] = copy.deepcopy(RING_PARTICLE_COLOR)
    detail["color"]["emissiveIntensity"] = 0.0
    detail["mesh"] = {"useModelMaterial": False}
    for field in ("position", "rotation", "revolution", "scale", "velocity"):
        detail["linearLerp"][field] = False
    return element


def make_long_ribbon_placeholder(
    source: dict[str, Any], stage: dict[str, Any]
) -> dict[str, Any]:
    element = copy.deepcopy(source)
    element.update(
        {
            "id": f"manual.trackb.ba{stage['ba']}.ribbon-companion-blocked",
            "displayName": "ribbon-companion-blocked",
            "groupId": f"manual.trackb.ba{stage['ba']}.weapon-ribbon",
            "sourceNode": str(source["id"]),
            "visible": False,
            "kind": "trail",
            "resources": [],
            # Preserve the Imported fallback-blocked source profile.  The exact
            # object path resolves to two physical-package candidates, so a
            # generic Base alias would silently choose the wrong material.
            "material": copy.deepcopy(source["material"]),
            "actionCueAttachment": weapon_attachment(),
            "transformInheritance": terminal_transform_master(),
            "sourceRecipe": disabled_source_recipe(),
            "sourcePresentation": disabled_source_presentation(),
        }
    )
    detail = element["detail"]
    detail["transform"] = {
        "position": [0.0, 0.0, 0.0],
        "rotationDegrees": [0.0, 0.0, 0.0],
        "revolutionDegreesPerSecond": [0.0, 0.0, 0.0],
        "scale": [1.0, 1.0, 1.0],
        "velocityPerSecond": [0.0, 0.0, 0.0],
    }
    detail["timing"] = {
        "startDelaySeconds": stage["ribbonStart"],
        "lifeTimeSeconds": stage["ribbonDuration"],
        "afterImageSeconds": 0.0,
        "dissolveStartNormalized": 1.0,
    }
    detail["trail"] = {
        "maxPoints": 64,
        "pointLifeTimeSeconds": 0.22,
        "sampleIntervalSeconds": 1.0 / 60.0,
        "minimumDistance": 0.01,
        "startWidth": 0.35,
        "endWidth": 0.04,
        "faceCamera": True,
    }
    return element


def long_impact_source_id(emitter: int) -> str:
    return (
        "fx_pc_flm_01.par_m_flm_pyungimpact_01."
        f"particlespriteemitter_{emitter}"
    )


def make_long_impact_element(
    source: dict[str, Any], stage: dict[str, Any], role: dict[str, Any]
) -> dict[str, Any]:
    element = copy.deepcopy(source)
    master_id = f"manual.trackb.ba{stage['ba']}.impact-master"
    element.update(
        {
            "id": f"manual.trackb.ba{stage['ba']}.{role['roleId']}",
            "displayName": role["roleId"],
            "groupId": f"manual.trackb.ba{stage['ba']}.impact-stack",
            "sourceNode": str(source["id"]),
            "visible": True,
            "kind": role["kind"],
            "actionCueAttachment": root_attachment(),
            "transformInheritance": (
                terminal_transform_master()
                if role["master"]
                else inherit_master_transform(master_id)
            ),
            "sourceRecipe": disabled_source_recipe(),
            "sourcePresentation": disabled_source_presentation(),
        }
    )
    reset_material(element, profile="alpha_two_sided_depth_read")
    detail = element["detail"]
    detail["transform"] = {
        "position": [0.0, 0.0, 0.0],
        "rotationDegrees": [0.0, 0.0, 0.0],
        "revolutionDegreesPerSecond": [0.0, 0.0, 0.0],
        "scale": copy.deepcopy(role["scale"]),
        "velocityPerSecond": [0.0, 0.0, 0.0],
    }
    detail["timing"]["startDelaySeconds"] = stage["impactStart"]
    detail["timing"]["lifeTimeSeconds"] = max(
        float(value) for value in detail["particle"]["lifeTimeSeconds"]
    )
    detail["color"]["emissiveIntensity"] = 0.0
    detail["mesh"] = {"useModelMaterial": False}
    detail["sprite"] = {
        "billboard": role["kind"] == "sprite",
        "billboardRollDegrees": 0.0,
    }
    for field in ("position", "rotation", "revolution", "scale", "velocity"):
        detail["linearLerp"][field] = False
    return element


def build_long_candidate(
    imported: dict[str, Any], stage: dict[str, Any]
) -> tuple[dict[str, Any], dict[str, Any]]:
    elements = source_by_id(imported)
    ring_id = (
        "fx_pc_flm_01.par_m_flm_pyungmtrail_01."
        f"particlespriteemitter_4{stage['ringSourceSuffix']}"
    )
    ring_source = elements.get(ring_id)
    if not isinstance(ring_source, dict):
        raise ValueError(f"Long-spear ring source is missing: {ring_id}")
    ribbon_id = (
        "fx_pc_flm_01.par_m_flm_ribbon_02.particlespriteemitter_0"
        f"{stage['ribbonSourceSuffix']}"
    )
    ribbon_source = elements.get(ribbon_id)
    if not isinstance(ribbon_source, dict):
        raise ValueError(f"Long-spear Ribbon source is missing: {ribbon_id}")

    ring = make_long_ring_master(ring_source, stage)
    animtrail = make_long_animtrail(ring_source, stage)
    ribbon = make_long_ribbon_placeholder(ribbon_source, stage)
    output = [ring, animtrail, ribbon]
    impact_rows: list[dict[str, Any]] = []
    if stage["ba"] == 4:
        for role in LONG_IMPACT_ROLES:
            source_id = long_impact_source_id(int(role["emitter"]))
            source = elements.get(source_id)
            if not isinstance(source, dict):
                raise ValueError(f"Long-spear impact source is missing: {source_id}")
            target = make_long_impact_element(source, stage, role)
            output.append(target)
            impact_rows.append(
                {
                    "roleId": role["roleId"],
                    "sourceElementId": source_id,
                    "targetElementId": target["id"],
                    "master": role["master"],
                    "transformExecution": (
                        "TERMINAL_MASTER_FINAL_MATRIX"
                        if role["master"]
                        else "MASTER_FINAL_MATRIX_INHERITED"
                    ),
                    "sourceMaterialPath": source["material"]["sourceMaterialPath"],
                    "resourceBindings": copy.deepcopy(source.get("resources", [])),
                    "materialExecutionStatus": (
                        "MANUAL_ASSEMBLY_PENDING_SOURCE_SHADER_PARITY"
                    ),
                }
            )

    document = {
        "schema": "lostark.effect-authoring",
        "version": 13,
        "effectAssetId": candidate_id(stage["target"]),
        "displayName": (
            f"LanceMaster 34010 BA{stage['ba']} Restoration Candidate"
        ),
        "particleSystem": {
            "uniformScaleMultiplier": 1.0,
            "yawOffsetDegrees": 0.0,
            "directionYawDegrees": 0.0,
            "initialSpeedMultiplier": 1.0,
        },
        "modelCues": [],
        "elements": output,
    }
    validate_candidate_no_fallback(document)
    receipt = base_receipt(stage, document, skill_id=34010)
    receipt.update(
        {
            "masterCarrier": {
                "roleId": "ring-master",
                "targetElementId": ring["id"],
                "what": "MESH_PARTICLE_ORIGIN_STANDALONE_MESH",
                "where": "PLAYER_ROOT_SNAPSHOT_MANUAL_TRANSFORM",
                "sourceElementId": ring_id,
                "sourceModelAssetId": resource_asset(ring, "meshModel"),
                "sourceMaterialPath": ring_source["material"]["sourceMaterialPath"],
                "sourceResourceBindings": copy.deepcopy(ring_source["resources"]),
                "sourceExtracted": {
                    "startDelaySeconds": stage["ringStart"],
                    "lifeTimeSeconds": 0.6000000238418579,
                    "decodedScale": copy.deepcopy(ring["detail"]["transform"]["scale"]),
                    "decodedPositionNotApplied": [0.0, -0.1, 0.0],
                    "meshStartRotationTurnsNotApplied": 0.7,
                    "rotationRateTurnsPerSecondNotApplied": 0.5,
                    "particleColorAppliedAsPreviewMultiply": copy.deepcopy(
                        RING_PARTICLE_COLOR
                    ),
                },
                "transformExecution": "TERMINAL_MASTER_FINAL_MATRIX",
                "materialExecutionStatus": (
                    "MANUAL_ASSEMBLY_PENDING_SOURCE_SHADER_PARITY"
                ),
                "manualPending": {
                    "finalPositionRotationScale": True,
                    "meshPivot": True,
                    "sourceMaterialGraph": True,
                    "effectiveFinalColor": True,
                },
            },
            "anchorCompanions": [
                {
                    "roleId": "animtrail-companion",
                    "targetElementId": animtrail["id"],
                    "what": "ANIMTRAIL",
                    "where": "WEAPON_TIP_MANUAL_OFFSET",
                    "sourceTypeData": "ParticleModuleTypeDataAnimTrail",
                    "sourceSystem": stage["trailSystem"],
                    "sourceNotifyObjectId": stage["trailNotifyObjectId"],
                    "sourceControlEdgeEvidence": "midcontrol",
                    "runtimeBoneName": "b_weapon_rhand",
                    "runtimeAnchorMappingStatus": (
                        "INFERRED_PROJECT_MAPPING_NOT_SOURCE_ENDPOINT"
                    ),
                    "adjacentRibbonAnchorEvidence": (
                        "WP_FLM_1_Battle_IS_RIBBON_EVENT_EVIDENCE_"
                        "NOT_ANIMTRAIL_ENDPOINT"
                    ),
                    "transformInheritance": False,
                    "inheritanceReason": (
                        "TRAIL_REQUIRES_WEAPON_FOLLOW_ANCHOR_NOT_ROOT_MESH_MATRIX"
                    ),
                    "sourceExact": {
                        "startDelaySeconds": stage["trailStart"],
                        "notifyDurationSeconds": stage["trailDuration"],
                        "materialPath": TRAIL_MATERIAL_PATH,
                        "textureRoles": copy.deepcopy(
                            list(TRAIL_SOURCE_TEXTURE_ROLES)
                        ),
                        "controlEdgeName": "midcontrol",
                        "tilingDistance": 100.0,
                        "distanceTessellationStep": 1.0,
                        "tangentTessellationScalar": 1.0,
                    },
                    "manualPending": {
                        "weaponTipLocalOffset": [0.0, 0.0, 0.0],
                        "twoEdgeWidth": [0.35, 0.04],
                        "pointLifeTimeSeconds": 0.22,
                        "shaderExecution": "MANUAL_PENDING_SOURCE_MATERIAL_GRAPH",
                    },
                },
                {
                    "roleId": "ribbon-companion",
                    "sourceElementId": ribbon_id,
                    "targetElementId": ribbon["id"],
                    "renderElementCreated": True,
                    "visible": False,
                    "what": "PARTICLE_MODULE_TYPE_DATA_RIBBON",
                    "where": "WEAPON_TIP_MANUAL_OFFSET",
                    "sourceStartDelaySeconds": stage["ribbonStart"],
                    "sourceDurationSeconds": stage["ribbonDuration"],
                    "sourceSystem": "FX_PC_FLM_01.Par_M_FLM_Ribbon_02",
                    "sourceMaterialPath": ribbon_source["material"][
                        "sourceMaterialPath"
                    ],
                    "sourceAnchorEvidence": "WP_FLM_1_Battle",
                    "status": (
                        "SOURCE_EXTRACTED_FAIL_CLOSED_MATERIAL_AMBIGUOUS_"
                        "AND_RIBBON_RUNTIME_MISSING"
                    ),
                    "failCloseReason": (
                        "SOURCE_MATERIAL_PATH_RESOLVES_TO_TWO_PHYSICAL_PACKAGES_"
                        "AND_GENERIC_TRAIL_IS_NOT_RIBBON_SEMANTICS"
                    ),
                    "materialResolution": {
                        "sourceMaterialPath": RIBBON_MATERIAL_PATH,
                        "status": "AMBIGUOUS_MATERIAL_PATH",
                        "physicalPackageCandidates": [
                            "YGI3SB3OBJ3O1MGUMP6QMP8B5.upk",
                            "ZHJ4TC4PCK4PC4J22HIXEYUXEU.upk",
                        ],
                        "candidateEvidence": [
                            {
                                "physicalPackage": (
                                    "YGI3SB3OBJ3O1MGUMP6QMP8B5.upk"
                                ),
                                "parent": "fx_mi.fx_m_pa_ribbonmaster_01_tr",
                                "textures": {
                                    "alpha_texture": "fx_b_trail_004",
                                    "emissive_tex_01": "fx_e_noise_001",
                                    "emissive_tex_02": "fx_j_mirnoise_01",
                                    "uv_noise_texture": "fx_d_normal_001_n",
                                },
                                "emissive_strength": 300.0,
                                "emissive_core_strength": 100.0,
                            },
                            {
                                "physicalPackage": (
                                    "ZHJ4TC4PCK4PC4J22HIXEYUXEU.upk"
                                ),
                                "parent": "fx_m.fx_m_pa_ribbonmaster_04_tr",
                                "textures": {
                                    "alpha_texture": (
                                        "fx_bg_lightbeam_falloff_03"
                                    ),
                                    "dissolve_tex_01": "fx_d_noise_004_1",
                                    "emissive_tex_01": "fx_d_noise_030",
                                    "emissive_tex_02": "fx_a_noise_008_n",
                                    "uv_noise_texture": "fx_m_flow_02_n",
                                },
                                "emissive_strength": 44.0,
                                "emissive_core_strength": 1.0,
                            },
                        ],
                    },
                    "requiredBeforeVisible": [
                        "PROVEN_LOGICAL_TO_PHYSICAL_MATERIAL_JOIN",
                        "EXACT_DDS_ROLE_BINDINGS",
                        "PARTICLE_MODULE_TYPE_DATA_RIBBON_RUNTIME_SEMANTICS",
                        "WEAPON_TIP_OR_DUAL_EDGE_ENDPOINT",
                    ],
                },
            ],
            "impactStack": (
                {
                    "status": "MANUAL_ASSEMBLY_PENDING",
                    "commonStartDelaySeconds": stage["impactStart"],
                    "masterElementId": impact_rows[0]["targetElementId"],
                    "carriers": impact_rows,
                }
                if impact_rows
                else None
            ),
            "whiteCaptureRca": {
                "screenshotElementKind": "mesh",
                "screenshotElementId": "authored.approx.s002.mesh01",
                "screenshotWasDecal": False,
                "screenshotWasTrail": False,
                "missingDdsFallback": False,
                "sourceScalarEvidence": {
                    "emissive_tex_strength": 5000.0,
                    "emissive_tex_power": 2.0,
                    "alpha_tex_strength": 4.0,
                },
                "rootCause": (
                    "RECONSTRUCTED_GROUPED_SHADER_HDR_SATURATION_WITHOUT_"
                    "SOURCE_PARTICLE_COLOR_ALPHA"
                ),
            },
            "candidateComposition": {
                "meshCount": 2 if impact_rows else 1,
                "spriteCount": 3 if impact_rows else 0,
                "trailElementCount": 2,
                "visibleGenericTrailCount": 1,
                "blockedRibbonCount": 1,
                "decalCount": 0,
                "runtimeParticleCount": 0,
            },
        }
    )
    return document, receipt


def short_source_id(stage: dict[str, Any], emitter: int) -> str:
    return (
        "fx_pc_flm_00.par_k_flm_shortnormal_atk_01."
        f"particlespriteemitter_{emitter}{stage['sourceSuffix']}"
    )


def convert_short_carrier(
    source: dict[str, Any], stage: dict[str, Any], role: dict[str, Any]
) -> dict[str, Any]:
    element = copy.deepcopy(source)
    element_id = f"manual.trackb.ba{stage['ba']}.{role['roleId']}"
    master_id = f"manual.trackb.ba{stage['ba']}.{SHORT_ROLES[0]['roleId']}"
    element.update(
        {
            "id": element_id,
            "displayName": role["roleId"],
            "groupId": f"manual.trackb.ba{stage['ba']}.short-stack",
            "sourceNode": str(source["id"]),
            "visible": True,
            "kind": role["kind"],
            "actionCueAttachment": root_attachment(),
            "transformInheritance": (
                terminal_transform_master()
                if role["master"]
                else inherit_master_transform(master_id)
            ),
            "sourceRecipe": disabled_source_recipe(),
            "sourcePresentation": disabled_source_presentation(),
        }
    )
    reset_material(
        element,
        profile=(
            "alpha_two_sided_depth_read"
            if role["kind"] == "sprite"
            else "alpha_two_sided_depth_read"
        ),
    )
    detail = element["detail"]
    detail["transform"]["position"] = copy.deepcopy(role["position"])
    detail["transform"]["rotationDegrees"] = copy.deepcopy(role["rotation"])
    detail["transform"]["revolutionDegreesPerSecond"] = [0.0, 0.0, 0.0]
    detail["transform"]["scale"] = copy.deepcopy(role["scale"])
    particle_life = max(float(value) for value in detail["particle"]["lifeTimeSeconds"])
    detail["timing"]["startDelaySeconds"] = 0.08
    detail["timing"]["lifeTimeSeconds"] = particle_life
    detail["mesh"] = {"useModelMaterial": False}
    detail["sprite"] = {
        "billboard": role["kind"] == "sprite",
        "billboardRollDegrees": 0.0,
    }
    return element


def rotation_matrix(rotation_degrees: list[float]) -> list[list[float]]:
    x, y, z = (math.radians(float(value)) for value in rotation_degrees)
    cx, sx = math.cos(x), math.sin(x)
    cy, sy = math.cos(y), math.sin(y)
    cz, sz = math.cos(z), math.sin(z)
    rx = [[1.0, 0.0, 0.0], [0.0, cx, sx], [0.0, -sx, cx]]
    ry = [[cy, 0.0, -sy], [0.0, 1.0, 0.0], [sy, 0.0, cy]]
    rz = [[cz, sz, 0.0], [-sz, cz, 0.0], [0.0, 0.0, 1.0]]
    return matrix_multiply(matrix_multiply(rx, ry), rz)


def matrix_multiply(
    left: list[list[float]], right: list[list[float]]
) -> list[list[float]]:
    return [
        [
            sum(left[row][inner] * right[inner][column] for inner in range(3))
            for column in range(3)
        ]
        for row in range(3)
    ]


def row_multiply(vector: list[float], matrix: list[list[float]]) -> list[float]:
    return [
        sum(vector[inner] * matrix[inner][column] for inner in range(3))
        for column in range(3)
    ]


def transpose(matrix: list[list[float]]) -> list[list[float]]:
    return [[matrix[column][row] for column in range(3)] for row in range(3)]


def clean(values: list[float] | list[list[float]]) -> Any:
    if values and isinstance(values[0], list):
        return [clean(row) for row in values]  # type: ignore[arg-type]
    return [0.0 if abs(float(value)) < 1.0e-12 else float(value) for value in values]


def short_role_receipt(role: dict[str, Any], target_id: str) -> dict[str, Any]:
    master = SHORT_ROLES[0]
    master_rotation = rotation_matrix(master["rotation"])
    if role["kind"] == "sprite":
        local_position = [
            float(role["position"][index]) - float(master["position"][index])
            for index in range(3)
        ]
        inheritance = "MASTER_TRANSLATION_ONLY"
        local_rotation = rotation_matrix(role["rotation"])
    else:
        world_offset = [
            float(role["position"][index]) - float(master["position"][index])
            for index in range(3)
        ]
        local_position = row_multiply(world_offset, transpose(master_rotation))
        local_rotation = matrix_multiply(
            rotation_matrix(role["rotation"]), transpose(master_rotation)
        )
        inheritance = "MASTER_POSITION_AND_ROTATION_NO_VISUAL_SCALE"
    composed_rotation = (
        local_rotation
        if role["kind"] == "sprite"
        else matrix_multiply(local_rotation, master_rotation)
    )
    return {
        "roleId": role["roleId"],
        "sourceEmitterIndex": role["emitter"],
        "targetElementId": target_id,
        "targetKind": role["kind"],
        "master": role["master"],
        "masterRoleId": master["roleId"],
        "sourceRelativeTransformEvidence": {
            "masterLocalPosition": clean(local_position),
            "masterLocalRotationMatrix": clean(local_rotation),
            "decodedFlatPosition": copy.deepcopy(role["position"]),
            "decodedFlatRotationDegrees": copy.deepcopy(role["rotation"]),
            "decodedFlatRotationMatrix": clean(composed_rotation),
            "previousFlatteningInheritance": inheritance,
            "executionStatus": "EVIDENCE_ONLY_NOT_FINAL_LAYOUT",
        },
        "runtimeTransformExecution": (
            "TERMINAL_MASTER_FINAL_MATRIX"
            if role["master"]
            else "MASTER_FINAL_MATRIX_INHERITED"
        ),
        "sourceEmitterStartOffsetNotApplied": float(role["startOffset"]),
        "commonPhaseStartDelaySeconds": 0.08,
        "scaleAndPivotStatus": "MANUAL_PENDING_TRACK_A",
        "positionStatus": role.get("positionDecision", "SOURCE_DETERMINISTIC"),
    }


def build_short_candidate(
    imported: dict[str, Any], stage: dict[str, Any]
) -> tuple[dict[str, Any], dict[str, Any]]:
    sources = source_by_id(imported)
    output: list[dict[str, Any]] = []
    role_rows: list[dict[str, Any]] = []
    for role in SHORT_ROLES:
        source_id = short_source_id(stage, int(role["emitter"]))
        source = sources.get(source_id)
        if not isinstance(source, dict) or source.get("kind") != "particle":
            raise ValueError(f"Short carrier source is missing: {source_id}")
        element = convert_short_carrier(source, stage, role)
        output.append(element)
        row = short_role_receipt(role, element["id"])
        row["sourceElementId"] = source_id
        row["sourceMaterialPath"] = source["material"]["sourceMaterialPath"]
        row["resourceBindings"] = copy.deepcopy(source.get("resources", []))
        row["materialExecutionStatus"] = "MANUAL_PENDING_SOURCE_MATERIAL_GRAPH"
        role_rows.append(row)

    document = {
        "schema": "lostark.effect-authoring",
        "version": 13,
        "effectAssetId": candidate_id(stage["target"]),
        "displayName": (
            f"LanceMaster 34510 BA{stage['ba']} Restoration Candidate"
        ),
        "particleSystem": {
            "uniformScaleMultiplier": 1.0,
            "yawOffsetDegrees": 0.0,
            "directionYawDegrees": 0.0,
            "initialSpeedMultiplier": 1.0,
        },
        "modelCues": [],
        "elements": output,
    }
    validate_candidate_no_fallback(document)
    blocked = []
    for role in BLOCKED_SHORT_ROLES:
        source_id = short_source_id(stage, int(role["emitter"]))
        source = sources.get(source_id)
        if not isinstance(source, dict):
            raise ValueError(f"Blocked short source is missing: {source_id}")
        if source.get("resources"):
            raise ValueError(f"Texture-less source gained resources: {source_id}")
        blocked.append(
            {
                **copy.deepcopy(role),
                "sourceElementId": source_id,
                "sourceMaterialPath": source["material"]["sourceMaterialPath"],
                "renderElementCreated": False,
                "forbiddenProxyAssetId": (
                    "Effect/LanceMaster/Textures/fx_m_spatter_001_xyclamp.dds"
                ),
            }
        )
    receipt = base_receipt(stage, document, skill_id=34510)
    receipt.update(
        {
            "masterCarrier": role_rows[0],
            "companionCarriers": role_rows[1:],
            "blockedCarriers": blocked,
            "masterFinalMatrixContract": {
                "authoringVersion": 13,
                "masterElementId": role_rows[0]["targetElementId"],
                "commonStartDelaySeconds": 0.08,
                "runtimeExecution": (
                    "ALL_VISIBLE_COMPANIONS_USE_TRANSFORM_INHERITANCE"
                ),
                "companionIndependentFields": [
                    "material",
                    "color",
                    "uv",
                    "timing.lifeTimeSeconds",
                    "dissolve",
                ],
                "sourceRelativeTransforms": "EVIDENCE_ONLY_NOT_EXECUTED",
                "elementLifetimeTransformNormalizationAllowed": False,
                "eulerComponentAdditionAllowed": False,
            },
            "candidateComposition": {
                "meshCount": 3,
                "spriteCount": 3,
                "blockedTexturelessSpriteCount": 2,
                "runtimeParticleCount": 0,
                "trailCount": 0,
                "decalCount": 0,
            },
            "unrestoredSourceCarriers": [
                {
                    "sourceSystem": "FX_CM_02.Light.Par_MP_Light_01",
                    "sourceKind": "light",
                    "stageLocalStartDelaySeconds": 0.08,
                    "emitterDurationSeconds": 1.75,
                    "status": "MISSING_OUTSIDE_CURRENT_MASTER_STACK_SLICE",
                }
            ],
        }
    )
    return document, receipt


def base_receipt(
    stage: dict[str, Any], document: dict[str, Any], *, skill_id: int
) -> dict[str, Any]:
    product_target = validated_product_target(stage, skill_id=skill_id)
    product_groups = product_target.get("groups", [])
    product_risks = sorted(
        {
            str(risk)
            for group in product_groups
            for risk in group.get("risks", [])
            if risk
        }
    )
    return {
        "schema": "lostark.effect-restoration-candidate-receipt",
        "version": 1,
        "characterClass": "LANCE_MASTER",
        "skillId": skill_id,
        "baStage": stage["ba"],
        "clip": stage["clip"],
        "productEffectAssetId": stage["target"],
        "candidateEffectAssetId": document["effectAssetId"],
        "candidatePath": (
            f"Effects/Authored/{document['effectAssetId']}.effect.json"
        ),
        "restorationStatus": {
            "current": "SOURCE_EXTRACTED",
            "sourceExtraction": "SOURCE_EXTRACTED",
            "manualAssembly": "MANUAL_VISUAL_PENDING",
            "visualApproval": "NOT_VISUAL_APPROVED",
        },
        "candidateExecutionBoundary": {
            "documentFormatVersion": 13,
            "currentWorktreeRuntimeFormatVersion": 13,
            "currentWorktreeResourceRootPresent": False,
            "executableInCurrentWorktree": False,
            "codecPlaybackValidatedWithCanonicalResourceRoot": True,
            "status": "V13_CANDIDATE_VALIDATED_NOT_PRODUCT",
            "standaloneHarnessSelfRootIntegratedInCurrentWorktree": False,
            "standaloneHarnessRequiresExplicitResourceRoot": True,
            "upstreamHarnessSelfRootValidated": True,
            "productOrRuntimeCatalogMutation": False,
        },
        "productAdmission": "BLOCKED_UNTIL_MANUAL_VISUAL_APPROVAL",
        "sourceExactClaimed": False,
        "runtimeParticleOutputAllowed": False,
        "candidateDocumentSha256": sha256_bytes(canonical_bytes(document)),
        "productCarrierInventory": {
            "schema": "lostark.trackb-product-carrier-inventory-evidence",
            "version": 1,
            "source": "PINNED_PRODUCT_DOCUMENT",
            "productDocumentPath": product_target["documentPath"],
            "productDocumentSha256": product_target["documentSha256"],
            "productVisualStatus": product_target["visualStatus"],
            "productGroupCount": len(product_groups),
            "productMasterElementIds": [
                str(group["masterElementId"]) for group in product_groups
            ],
            "productRisks": product_risks,
            "productGroups": [
                {
                    "groupId": str(group["groupId"]),
                    "masterElementId": str(group["masterElementId"]),
                    "masterSelectionBasis": str(group["masterSelectionBasis"]),
                    "manualAuditionOrder": copy.deepcopy(
                        group["manualAuditionOrder"]
                    ),
                    "risks": copy.deepcopy(group.get("risks", [])),
                }
                for group in product_groups
            ],
            "candidateStatus": (
                "SOURCE_EXTRACTED_V13_CANDIDATE_MANUAL_VISUAL_PENDING"
            ),
        },
        "sourceEvidence": [
            {
                "path": path.relative_to(REPOSITORY_ROOT).as_posix(),
                "sha256": expected,
            }
            for path, expected in PINNED_TRACKED_INPUTS.items()
        ]
        + [
            {
                "path": path.relative_to(REPOSITORY_ROOT).as_posix(),
                "sha256": expected,
                "hashNormalization": "UTF8_LF",
            }
            for path, expected in PINNED_LF_TEXT_INPUTS.items()
        ]
        + copy.deepcopy(list(EXTERNAL_MATERIAL_EVIDENCE)),
        "manualApproval": {
            "soloElement": "PENDING_MANUAL",
            "soloGroupOrTrail": "PENDING_MANUAL",
            "characterSelectInput": "NOT_ADMITTED_TO_PRODUCT",
            "automaticImageJudgement": False,
        },
        "fallbackPolicy": {
            "whiteTextureFallbackAllowed": False,
            "blackTextureFallbackAllowed": False,
            "crossEmitterDdsProxyAllowed": False,
            "sameSourceMaterialPreviewAliasAllowed": True,
            "genericPreviewCompletionAllowed": False,
            "missingRequiredBaseBehavior": "FAIL_CLOSE",
        },
        "sourceDrivenAbsence": {"decal": True},
    }


def parse_skill_id(path: Path) -> int:
    match = re.search(r"skill\.(\d+)", path.name)
    if match is None:
        raise ValueError(f"Cannot parse skill ID: {path}")
    return int(match.group(1))


def build_decal_inventory() -> dict[str, Any]:
    rows: dict[tuple[str, int], dict[str, Any]] = {}
    for class_name, directory_name in (
        ("LANCE_MASTER", "LanceMaster"),
        ("ARTIST", "Artist"),
    ):
        class_root = DATA_ROOT / "Effects/Imported" / directory_name
        for path in sorted(class_root.glob("skill.*.source-receipt.json")):
            text = path.read_text(encoding="utf-8")
            count = len(re.findall("efparticlemoduletypedatadecal", text, re.I))
            if count == 0:
                continue
            value = read_json(path)
            skill_id = int(value.get("skillId") or parse_skill_id(path))
            rows[(class_name, skill_id)] = {
                "characterClass": class_name,
                "skillId": skill_id,
                "inputSlot": str(value.get("inputSlot") or ""),
                "sourceReceipt": path.relative_to(REPOSITORY_ROOT).as_posix(),
                "sourceTypeDataDecalReferenceCount": count,
                "convertedDocument": "",
                "convertedDecalElementCount": 0,
                "sourceElementIds": [],
                "sourceMaterialPaths": [],
                "resourceAssetIds": [],
                "exactTextureBindingStatus": "PENDING_CONVERTED_DECAL",
            }
        for path in sorted(class_root.rglob("effect.*.imported.effect.json")):
            document = read_json(path)
            decals = [
                element
                for element in document.get("elements", [])
                if isinstance(element, dict) and element.get("kind") == "decal"
            ]
            if not decals:
                continue
            skill_id = parse_skill_id(path)
            row = rows.setdefault(
                (class_name, skill_id),
                {
                    "characterClass": class_name,
                    "skillId": skill_id,
                    "inputSlot": "",
                    "sourceReceipt": "",
                    "sourceTypeDataDecalReferenceCount": 0,
                    "convertedDocument": "",
                    "convertedDecalElementCount": 0,
                    "sourceElementIds": [],
                    "sourceMaterialPaths": [],
                    "resourceAssetIds": [],
                    "exactTextureBindingStatus": "",
                },
            )
            resources = sorted(
                {
                    str(binding.get("assetId"))
                    for element in decals
                    for binding in element.get("resources", [])
                    if isinstance(binding, dict) and binding.get("assetId")
                }
            )
            row.update(
                {
                    "convertedDocument": path.relative_to(REPOSITORY_ROOT).as_posix(),
                    "convertedDecalElementCount": len(decals),
                    "sourceElementIds": sorted(str(element["id"]) for element in decals),
                    "sourceMaterialPaths": sorted(
                        {
                            str(element.get("material", {}).get("sourceMaterialPath") or "")
                            for element in decals
                            if element.get("material", {}).get("sourceMaterialPath")
                        }
                    ),
                    "resourceAssetIds": resources,
                    "exactTextureBindingStatus": (
                        "SAFE_ASSET_IDS_PRESENT_SHADER_GRAPH_PENDING"
                        if resources
                        else "MANUAL_PENDING_EXACT_TEXTURE_BINDING"
                    ),
                }
            )
    ordered = [rows[key] for key in sorted(rows)]
    return {
        "schema": "lostark.effect-decal-heavy-inventory",
        "version": 1,
        "scope": ["LANCE_MASTER", "ARTIST"],
        "manualRestorationPolicy": {
            "source": "TRACK_B_MASTER_FIRST_CONTRACT",
            "masterFirst": True,
            "sourceProvenRelativeTransformOnly": True,
            "whiteOrBlackFallbackForbidden": True,
            "manualVisualApprovalRequired": True,
            "globalWorklistRequiredForGeneration": False,
            "productCompletionRule": (
                "Inventory evidence is not visual approval; use the master-first "
                "manual gate."
            ),
        },
        "classification": {
            "sourceTypeDataEvidence": "EFParticleModuleTypeDataDecal",
            "convertedEvidence": "kind=decal",
            "materialCompletionRule": (
                "No Product completion until exact source DDS roles and shader graph "
                "are executable and manually approved."
            ),
        },
        "lmbAbsence": [
            {
                "characterClass": "LANCE_MASTER",
                "skillId": 34010,
                "decalOccurrence": False,
                "note": "Captured white arc is fm_m_ring_001 Mesh, not Decal.",
            },
            {
                "characterClass": "LANCE_MASTER",
                "skillId": 34510,
                "decalOccurrence": False,
                "note": "ShortNormal source is meshParticle 3 + spriteParticle 5.",
            },
        ],
        "skills": ordered,
    }


def desired_outputs() -> dict[Path, bytes]:
    validate_pinned_inputs()
    long_imported = read_json(LONG_IMPORTED_PATH)
    short_imported = read_json(SHORT_IMPORTED_PATH)
    desired: dict[Path, bytes] = {}
    for stage in LONG_STAGES:
        document, receipt = build_long_candidate(long_imported, stage)
        desired[candidate_path(stage["target"])] = serialized(document)
        desired[candidate_receipt_path(stage["target"])] = serialized(receipt)
    for stage in SHORT_STAGES:
        document, receipt = build_short_candidate(short_imported, stage)
        desired[candidate_path(stage["target"])] = serialized(document)
        desired[candidate_receipt_path(stage["target"])] = serialized(receipt)
    desired[DECAL_INVENTORY_PATH] = serialized(build_decal_inventory())
    return desired


def serialized(value: Any) -> bytes:
    return (json.dumps(value, ensure_ascii=False, indent=2) + "\n").encode("utf-8")


def replace_outputs_atomically(outputs: dict[Path, bytes]) -> None:
    staged: dict[Path, Path] = {}
    try:
        for path, content in outputs.items():
            path.parent.mkdir(parents=True, exist_ok=True)
            descriptor, temporary_name = tempfile.mkstemp(
                prefix=f".{path.name}.", suffix=".tmp", dir=path.parent
            )
            with os.fdopen(descriptor, "wb") as stream:
                stream.write(content)
                stream.flush()
                os.fsync(stream.fileno())
            temporary = Path(temporary_name)
            json.loads(temporary.read_text(encoding="utf-8"))
            staged[path] = temporary
        for path, temporary in staged.items():
            os.replace(temporary, path)
    finally:
        for temporary in staged.values():
            temporary.unlink(missing_ok=True)


def write_new_outputs(desired: dict[Path, bytes]) -> None:
    existing = [path for path in desired if path.exists()]
    if existing:
        raise FileExistsError(
            "Candidate seed refuses existing output: "
            + ", ".join(str(path) for path in existing)
        )
    replace_outputs_atomically(desired)


def refresh_generated_metadata(desired: dict[Path, bytes]) -> None:
    candidate_documents = {
        candidate_path(stage["target"])
        for stage in (*LONG_STAGES, *SHORT_STAGES)
    }
    for path in sorted(candidate_documents):
        expected = desired[path]
        if not path.is_file():
            raise ValueError(f"Candidate document is missing: {path}")
        actual = read_json(path)
        wanted = json.loads(expected.decode("utf-8"))
        if canonical_bytes(actual) != canonical_bytes(wanted):
            raise ValueError(
                "Candidate document was manually edited; refusing metadata refresh: "
                f"{path}"
            )
    metadata = {
        path: content
        for path, content in desired.items()
        if path not in candidate_documents
    }
    replace_outputs_atomically(metadata)


def replace_exact_candidate_seed(
    desired: dict[Path, bytes],
    expected_sha_by_target: dict[str, str],
    *,
    label: str,
) -> None:
    stages = (*LONG_STAGES, *SHORT_STAGES)
    expected_targets = {str(stage["target"]) for stage in stages}
    if set(expected_sha_by_target) != expected_targets:
        raise ValueError(f"{label} manifest does not cover all candidate targets")

    # Validate the complete set before replace_outputs_atomically stages any
    # document or metadata.  A mismatching SHA means a human save may exist.
    for stage in stages:
        target = str(stage["target"])
        path = candidate_path(target)
        if not path.is_file():
            raise ValueError(f"{label} candidate is missing: {path}")
        actual = file_sha256(path)
        expected = expected_sha_by_target[target]
        if actual != expected:
            raise ValueError(
                f"Candidate is not the exact {label} seed; refusing migration: "
                f"{path} ({actual})"
            )
    replace_outputs_atomically(desired)


def migrate_v12_candidates_to_v13(desired: dict[Path, bytes]) -> None:
    replace_exact_candidate_seed(
        desired, LEGACY_V12_CANDIDATE_SHA256, label="legacy v12"
    )


def add_fail_closed_ribbon_placeholders(desired: dict[Path, bytes]) -> None:
    replace_exact_candidate_seed(
        desired, PRE_RIBBON_V13_CANDIDATE_SHA256, label="pre-Ribbon v13"
    )


def check_outputs(desired: dict[Path, bytes]) -> None:
    for path, expected in desired.items():
        if not path.is_file():
            raise ValueError(f"Candidate output is missing: {path}")
        actual = json.loads(path.read_text(encoding="utf-8"))
        wanted = json.loads(expected.decode("utf-8"))
        if canonical_bytes(actual) != canonical_bytes(wanted):
            raise ValueError(f"Candidate output drifted: {path}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--refresh-generated-metadata", action="store_true")
    mode.add_argument("--migrate-v13", action="store_true")
    mode.add_argument("--add-ribbon-placeholders", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    desired = desired_outputs()
    if args.write:
        write_new_outputs(desired)
        print(f"Created {len(desired)} LanceMaster candidate/inventory outputs.")
    elif args.migrate_v13:
        migrate_v12_candidates_to_v13(desired)
        print(
            "Migrated 7 exact legacy v12 candidates and refreshed "
            "their receipts/inventory to authoring v13."
        )
    elif args.add_ribbon_placeholders:
        add_fail_closed_ribbon_placeholders(desired)
        print(
            "Added four fallback-blocked Ribbon queue elements to the exact "
            "pre-Ribbon v13 seed and refreshed receipts/inventory."
        )
    elif args.refresh_generated_metadata:
        refresh_generated_metadata(desired)
        print("Refreshed LanceMaster candidate receipts/inventory; candidate documents unchanged.")
    else:
        check_outputs(desired)
        print(f"PASS: verified {len(desired)} LanceMaster candidate/inventory outputs.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
