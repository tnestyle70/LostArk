#!/usr/bin/env python3
"""Selectively restore DimensionMaster A's four MakeFlow occurrences.

The current nine Product rows are user-tuned authoring and are immutable input
to this transaction. The retained ``a58f...`` row remains PROJECT_TUNED: its
source identity is event-030 at 0.60 seconds while its authored timing is 0.25
seconds. The four source rows are a separate SOURCE_EXACT evidence lane
projected from the immutable imported document and Track A receipt. The source-
base 0.25 row is not admitted because the current tuned row occupies that
visual slot; only the missing 0.60/0.90/1.30 rows are appended. Keeping those
provenance lanes separate prevents both a fifth visible slash and a full
34/63-row rollback.
"""

from __future__ import annotations

import argparse
import codecs
import copy
import json
import math
import os
import pathlib
import re
import sys
import tempfile
from typing import Any

from materialize_four_class_track_a_candidates import (
    canonical_sha256,
    load_canonical_materials,
    normalized_source_recipe,
    stable_particle_element_id,
)


REPOSITORY_ROOT = pathlib.Path(__file__).resolve().parents[2]
TARGET_PATH = REPOSITORY_ROOT / (
    "Data/Effects/Authored/"
    "effect.dimensionmaster.skill.2050210.unified.effect.json"
)
IMPORTED_PATH = REPOSITORY_ROOT / (
    "Data/Effects/Imported/DimensionMaster/Converted/"
    "effect.dimensionmaster.skill.2050210.imported.effect.json"
)
RESTORATION_RECEIPT_PATH = REPOSITORY_ROOT / (
    "Data/Effects/AuthoredCorrections/Generated/"
    "FourClassCombat.track-a-restoration-receipt.json"
)
ANIMATION_EVENTS_PATH = REPOSITORY_ROOT / (
    "Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents"
)

TARGET_EFFECT_ID = "effect.dimensionmaster.skill.2050210.unified"
SOURCE_EFFECT_ID = "effect.dimensionmaster.skill.2050210.imported"
CHARACTER_CLASS = "DIMENSIONMASTER"
SKILL_ID = 2050210
SOURCE_BASE_ID = (
    "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1."
    "particlespriteemitter_2"
)
SOURCE_ELEMENT_IDS = (
    SOURCE_BASE_ID,
    SOURCE_BASE_ID + ".event_source-event-030",
    SOURCE_BASE_ID + ".event_source-event-045",
    SOURCE_BASE_ID + ".event_source-event-060",
)
SOURCE_EVENT_IDS = (
    "source-event-009",
    "source-event-030",
    "source-event-045",
    "source-event-060",
)
SOURCE_START_SECONDS = (0.25, 0.60, 0.90, 1.30)
SOURCE_LOCAL_POSITIONS = (
    (0.5, -0.9, 0.5),
    (0.5, 0.8000000000000002, 0.5),
    (0.5, -0.9, -0.30000000000000004),
    (0.5, -0.8, -0.6000000000000001),
)
SOURCE_TARGET_IDS = (
    "authored.source-particle.9b7cc465b8261b6b7e76de7e",
    "authored.source-particle.53c11a9082f088279597515c",
    "authored.source-particle.83f005256fb59d87b99de92f",
    "authored.source-particle.ca6dc295e0267400d6968003",
)
SOURCE_PROJECTED_SHA256 = (
    "ebc8c57d0c673805becb5e41b6208233c2b52b41f2586829a5d8dedd84bbb5ba",
    "0597866e6e2a644be0e518da4c9973aed2fec3636b19928a1246b327ddf163d7",
    "1a4fd72a851655e26076b08c2960617cd35f02c16afa31f2d4b4e60a9e1dd55f",
    "cc86beb93af3c895e827aa8dcd6aeff567b26131d34e62d230cd74ad65bd5549",
)

EXPECTED_HEADER_SHA256 = (
    "4bfff9de8f35e9f378e6bce97cadb37e3e50a987c5b40eeeaeba9d73fb748c07"
)
EXPECTED_TUNED_ROWS = (
    ("authored.source-particle.10b56e310f21e647598982ce", "2f684e2263d6b2391f29b3ab4afa5d1e92b3b16abbe45f58bca8498bddef2c14"),
    ("authored.source-particle.51e1a97ff607dbbaaa5149bf", "d21c7ca4bd1d69a5125e005d936384139958e9a03910d7535168943506d14d74"),
    ("authored.source-particle.af78afd810918ca8c7f99099", "a28702ed68cc998e3fc5fe0041060a46bae4d586897b705d8381988f5b88eb33"),
    ("authored.source-particle.5a8cf08b7d34a1d6a1bcc41b", "9c6bc72cf326457e70acf1490826935c7040df587b26974171442bdb890385e7"),
    ("authored.source-particle.5bb299c8af9714768261ff3f", "d909167092b1e8b7062613fce0a5ee8c735c6343f46bd18e8201180bf856777a"),
    ("authored.source-particle.abe290ff167e9cfe33584aa0", "1a7675838ae3a724d52561fbf2be25fcc75eb202cf0f3cddda4757b2394e22f5"),
    ("authored.source-particle.46dc276e3fc49ebcda2f4770", "ab917246791494dcd94065abbe8db741cf0702c920ea922aebcf182bf8e81bd4"),
    ("authored.source-particle.905bf4a220b0c685271b30a2", "12aa63f90aff0e43c6a582f57a73db697ac965c4d5a05cd2cbf0cd019850bfa6"),
    ("authored.source-particle.a58f7a015a0bab4c53a664fd", "0604eae2a3f887046dee2d97ad116ac576757c1ed91d1a389ac6a9ab8d3a4f50"),
)

TUNED_REFERENCE_ID = "authored.source-particle.a58f7a015a0bab4c53a664fd"
TUNED_FIELD_PROVENANCE = {
    "sourceIdentity": {
        "sourceElementId": SOURCE_ELEMENT_IDS[1],
        "sourceEventId": SOURCE_EVENT_IDS[1],
        "sourceStartDelaySeconds": SOURCE_START_SECONDS[1],
    },
    "projectTunedFields": {
        "detail.timing.startDelaySeconds": {
            "value": 0.25,
            "provenance": "PROJECT_TUNED",
        }
    },
}

SOURCE_OCCURRENCE_ADMISSION_RECEIPT = {
    "schema": "lostark.dimensionmaster-2050210-occurrence-admission",
    "formatVersion": 1,
    "visualCardinality": 4,
    "rows": [
        {
            "sourceElementId": SOURCE_ELEMENT_IDS[0],
            "canonicalStableId": SOURCE_TARGET_IDS[0],
            "sourceStartDelaySeconds": SOURCE_START_SECONDS[0],
            "disposition": "SOURCE_EXACT_EVIDENCE_ONLY_NOT_ADMITTED",
            "substitutedByCurrent": TUNED_REFERENCE_ID,
            "substitutionProvenance": "PROJECT_TUNED",
        },
        *[
            {
                "sourceElementId": SOURCE_ELEMENT_IDS[index],
                "canonicalStableId": SOURCE_TARGET_IDS[index],
                "sourceStartDelaySeconds": SOURCE_START_SECONDS[index],
                "disposition": "ADMITTED_SOURCE_EXACT",
                "substitutedByCurrent": None,
            }
            for index in range(1, 4)
        ],
    ],
    "duplicateSourceNodePolicy": {
        "sourceElementId": SOURCE_ELEMENT_IDS[1],
        "projectTunedStableId": TUNED_REFERENCE_ID,
        "sourceExactStableId": SOURCE_TARGET_IDS[1],
        "allowed": True,
        "reason": "SEPARATE_PROJECT_TUNED_025_AND_SOURCE_EXACT_060_TIMINGS",
    },
}

EXPECTED_SOURCE_BINDINGS = (
    ("meshModel", "Effect/DimensionMaster/Meshes/fm_h_swing_05.wmodel"),
    ("base", "Effect/DimensionMaster/Textures/FX_TEX_05/fx_k_auraline_16.dds"),
    ("noise", "Effect/DimensionMaster/Textures/FX_TEX_05/fx_k_caustictile_01.dds"),
    ("mask", "Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_002.dds"),
)
EXPECTED_MATERIAL_PATH = "fx_m_mi_k_00.fx_mi.fx_k_me_makeflow_03_05_tr"
EXPECTED_PARENT_MATERIAL_PATH = "fx_m_mi_k_00.fx_m.fx_k_me_makeflow_03_tr"
EXPECTED_RUNTIME_SHADER_PROFILE = "effect.ue3.grouped-translucent.v1"
EXPECTED_MATERIAL_SHA256 = (
    "4f379c76dea44d7ddb73397585579faa0c13e4adab492745f92dffb7bcefd80d"
)

EXPECTED_A_CUE_LINE = (
    '"pc_sp_m_00_sk_sk_willowrend" EFFECT startms=0 '
    'payload="effect.dimensionmaster.skill.2050210.unified" effectref=asset '
    'anchor="root" follow=follow orientation=action_facing stop=natural '
    "px=0 py=0 pz=0 rx=0 ry=0 rz=0 sx=1 sy=1 sz=1"
)


class MaterializationError(RuntimeError):
    """Raised when any immutable or current-state contract has drifted."""


def _reject_duplicate_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise MaterializationError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def parse_json_text(text: str, label: str) -> dict[str, Any]:
    try:
        value = json.loads(
            text,
            object_pairs_hook=_reject_duplicate_keys,
            parse_constant=lambda token: (_ for _ in ()).throw(
                MaterializationError(f"non-finite JSON token in {label}: {token}")
            ),
        )
    except json.JSONDecodeError as error:
        raise MaterializationError(f"invalid JSON {label}: {error}") from error
    if not isinstance(value, dict):
        raise MaterializationError(f"JSON root must be an object: {label}")
    return value


def load_json(path: pathlib.Path) -> dict[str, Any]:
    try:
        return parse_json_text(path.read_text(encoding="utf-8-sig"), str(path))
    except OSError as error:
        raise MaterializationError(f"cannot read {path}: {error}") from error


def _require_finite(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise MaterializationError(f"{label} must be numeric")
    result = float(value)
    if not math.isfinite(result):
        raise MaterializationError(f"{label} must be finite")
    return result


def _binding_pairs(value: Any, label: str) -> tuple[tuple[str, str], ...]:
    if not isinstance(value, list):
        raise MaterializationError(f"{label} must be an array")
    pairs: list[tuple[str, str]] = []
    for row in value:
        if not isinstance(row, dict):
            raise MaterializationError(f"{label} row must be an object")
        pair = (str(row.get("slotId", "")), str(row.get("assetId", "")))
        if not all(pair) or pair in pairs:
            raise MaterializationError(f"{label} has an invalid/duplicate binding")
        pairs.append(pair)
    return tuple(pairs)


def _target_receipt(receipt: dict[str, Any]) -> dict[str, Any]:
    if (
        receipt.get("schema") != "lostark.four-class-track-a-restoration-receipt"
        or receipt.get("formatVersion") != 2
    ):
        raise MaterializationError("Track A restoration receipt identity changed")
    targets = receipt.get("targets")
    if not isinstance(targets, list):
        raise MaterializationError("Track A restoration receipt targets are missing")
    matches = [
        row
        for row in targets
        if isinstance(row, dict)
        and row.get("targetEffectAssetId") == TARGET_EFFECT_ID
    ]
    if len(matches) != 1:
        raise MaterializationError("2050210 target receipt is missing/ambiguous")
    target = matches[0]
    expected_identity = {
        "characterClass": CHARACTER_CLASS,
        "skillId": SKILL_ID,
        "stageIndex": 0,
        "stageClipIndex": 0,
        "clip": "pc_sp_m_00_sk_sk_willowrend",
        "targetPath": (
            "Data/Effects/Authored/"
            "effect.dimensionmaster.skill.2050210.unified.effect.json"
        ),
    }
    for field, expected in expected_identity.items():
        if target.get(field) != expected:
            raise MaterializationError(f"2050210 receipt {field} changed")
    return target


def _selected_receipt_rows(target_receipt: dict[str, Any]) -> list[dict[str, Any]]:
    rows = target_receipt.get("particleRows")
    if not isinstance(rows, list):
        raise MaterializationError("2050210 receipt particleRows are missing")
    selected = [
        row
        for row in rows
        if isinstance(row, dict)
        and (
            row.get("sourceElementId") == SOURCE_BASE_ID
            or str(row.get("sourceElementId", "")).startswith(SOURCE_BASE_ID + ".")
        )
    ]
    if [row.get("sourceElementId") for row in selected] != list(SOURCE_ELEMENT_IDS):
        raise MaterializationError("MakeFlow receipt occurrence set/order changed")
    return selected


def _validate_imported_document(source: dict[str, Any]) -> dict[str, dict[str, Any]]:
    if (
        source.get("schema") != "lostark.effect-authoring"
        or source.get("version") != 12
        or source.get("effectAssetId") != SOURCE_EFFECT_ID
    ):
        raise MaterializationError("2050210 imported document identity changed")
    elements = source.get("elements")
    if not isinstance(elements, list) or len(elements) != 117:
        raise MaterializationError("2050210 imported denominator changed")
    by_id: dict[str, dict[str, Any]] = {}
    for element in elements:
        if not isinstance(element, dict):
            raise MaterializationError("2050210 imported element is invalid")
        element_id = str(element.get("id", ""))
        if not element_id or element_id in by_id:
            raise MaterializationError("2050210 imported element ID is missing/duplicate")
        by_id[element_id] = element
    if any(element_id not in by_id for element_id in SOURCE_ELEMENT_IDS):
        raise MaterializationError("MakeFlow imported occurrence is missing")
    return by_id


def _validate_tuned_provenance(
    target_elements: list[dict[str, Any]], source_by_id: dict[str, dict[str, Any]]
) -> None:
    tuned = next(
        (row for row in target_elements if row.get("id") == TUNED_REFERENCE_ID), None
    )
    if tuned is None:
        raise MaterializationError("PROJECT_TUNED MakeFlow reference row is missing")
    source_identity = TUNED_FIELD_PROVENANCE["sourceIdentity"]
    if not str(tuned.get("sourceNode", "")).endswith(
        "|element:" + str(source_identity["sourceElementId"])
    ):
        raise MaterializationError("PROJECT_TUNED row source identity changed")
    tuned_start = tuned.get("detail", {}).get("timing", {}).get(
        "startDelaySeconds"
    )
    expected_tuned = TUNED_FIELD_PROVENANCE["projectTunedFields"][
        "detail.timing.startDelaySeconds"
    ]["value"]
    if _require_finite(tuned_start, "PROJECT_TUNED start") != expected_tuned:
        raise MaterializationError("PROJECT_TUNED timing changed")
    source_start = source_by_id[SOURCE_ELEMENT_IDS[1]].get("detail", {}).get(
        "timing", {}
    ).get("startDelaySeconds")
    if _require_finite(source_start, "source event-030 start") != source_identity[
        "sourceStartDelaySeconds"
    ]:
        raise MaterializationError("source identity timing changed")


def build_source_rows(
    source: dict[str, Any], receipt: dict[str, Any]
) -> list[dict[str, Any]]:
    source_by_id = _validate_imported_document(source)
    receipt_rows = _selected_receipt_rows(_target_receipt(receipt))
    source_elements = list(source["elements"])
    try:
        canonical_materials = load_canonical_materials(
            CHARACTER_CLASS, SOURCE_EFFECT_ID, source_elements
        )
    except (KeyError, RuntimeError, ValueError) as error:
        raise MaterializationError(
            f"cannot join canonical 2050210 materials: {error}"
        ) from error

    projected: list[dict[str, Any]] = []
    identities = zip(
        SOURCE_ELEMENT_IDS,
        SOURCE_EVENT_IDS,
        SOURCE_START_SECONDS,
        SOURCE_LOCAL_POSITIONS,
        SOURCE_TARGET_IDS,
    )
    for index, (source_id, event_id, start, position, expected_target_id) in enumerate(
        identities
    ):
        source_element = source_by_id[source_id]
        receipt_row = receipt_rows[index]
        stable_id = stable_particle_element_id(
            character_class=CHARACTER_CLASS,
            skill_id=SKILL_ID,
            source_effect_id=SOURCE_EFFECT_ID,
            source_element_id=source_id,
            source_event_id=event_id,
            target_effect_id=TARGET_EFFECT_ID,
        )
        if stable_id != expected_target_id or receipt_row.get(
            "targetElementId"
        ) != stable_id:
            raise MaterializationError(f"{source_id}: stable identity drifted")
        if receipt_row.get("sourceEventId") != event_id:
            raise MaterializationError(f"{source_id}: source event identity drifted")

        detail = copy.deepcopy(source_element.get("detail"))
        if not isinstance(detail, dict):
            raise MaterializationError(f"{source_id}: imported Detail is missing")
        transform = detail.get("transform")
        timing = detail.get("timing")
        mesh = detail.get("mesh")
        if not all(isinstance(value, dict) for value in (transform, timing, mesh)):
            raise MaterializationError(f"{source_id}: imported Detail shape changed")
        if _require_finite(timing.get("startDelaySeconds"), source_id) != start:
            raise MaterializationError(f"{source_id}: source cadence changed")
        if tuple(transform.get("position", ())) != position:
            raise MaterializationError(f"{source_id}: source local pose changed")
        if canonical_sha256(detail) != receipt_row.get(
            "sourceDetailCanonicalSha256"
        ):
            raise MaterializationError(f"{source_id}: source Detail receipt drifted")
        mesh["modelPreScale"] = 0.01
        if canonical_sha256(detail) != receipt_row.get(
            "targetDetailCanonicalSha256"
        ):
            raise MaterializationError(f"{source_id}: target Detail receipt drifted")

        source_recipe = source_element.get("sourceRecipe")
        if canonical_sha256(source_recipe) != receipt_row.get(
            "sourceRecipeCanonicalSha256"
        ):
            raise MaterializationError(f"{source_id}: source recipe receipt drifted")
        normalized_recipe = normalized_source_recipe(source_recipe)
        if canonical_sha256(normalized_recipe) != receipt_row.get(
            "normalizedRecipeCanonicalSha256"
        ):
            raise MaterializationError(f"{source_id}: normalized recipe drifted")

        source_bindings = _binding_pairs(
            source_element.get("resources"), f"{source_id} imported resources"
        )
        receipt_source_bindings = _binding_pairs(
            receipt_row.get("sourceBindings"), f"{source_id} receipt source bindings"
        )
        target_bindings = _binding_pairs(
            receipt_row.get("targetBindings"), f"{source_id} receipt target bindings"
        )
        if (
            source_bindings != EXPECTED_SOURCE_BINDINGS
            or receipt_source_bindings != source_bindings
            or target_bindings != source_bindings
        ):
            raise MaterializationError(f"{source_id}: resource identity drifted")
        material = copy.deepcopy(canonical_materials.get(source_id))
        if not isinstance(material, dict):
            raise MaterializationError(f"{source_id}: canonical material is missing")
        profile = material.get("sourceProfile")
        if not isinstance(profile, dict) or profile.get("enabled") is not True:
            raise MaterializationError(f"{source_id}: canonical SourceProfile changed")
        material["templateId"] = "effect.source_material"
        if (
            material.get("sourceMaterialPath") != EXPECTED_MATERIAL_PATH
            or profile.get("parentMaterialPath") != EXPECTED_PARENT_MATERIAL_PATH
            or profile.get("runtimeShaderProfileId") != EXPECTED_RUNTIME_SHADER_PROFILE
            or receipt_row.get("materialTemplateId") != "effect.source_material"
            or receipt_row.get("runtimeShaderProfileId")
            != EXPECTED_RUNTIME_SHADER_PROFILE
            or receipt_row.get("materialResolutionStatus")
            != "RESOLVED_EXACT_SOURCE_PACKAGE"
            or receipt_row.get("sourceProfileReady") is not True
            or receipt_row.get("portable") is not True
            or receipt_row.get("failClosedReasons") != []
            or canonical_sha256(material) != EXPECTED_MATERIAL_SHA256
        ):
            raise MaterializationError(f"{source_id}: source material contract drifted")

        attachment = copy.deepcopy(source_element.get("actionCueAttachment"))
        if not isinstance(attachment, dict):
            raise MaterializationError(f"{source_id}: source attachment is missing")
        attachment.setdefault("snapshotRootSourceBasisYawDegrees", 0.0)
        if (
            attachment.get("enabled") is not True
            or attachment.get("follow") is not False
            or attachment.get("runtimeAnchorSlotId") != "root"
        ):
            raise MaterializationError(f"{source_id}: inner snapshot contract drifted")

        source_presentation = copy.deepcopy(source_element.get("sourcePresentation"))
        if not isinstance(source_presentation, dict):
            raise MaterializationError(f"{source_id}: source presentation is missing")
        source_presentation["sourceEventId"] = event_id

        element = copy.deepcopy(source_element)
        element["id"] = stable_id
        element["displayName"] = f"{event_id} {source_id.rsplit('.', 1)[-1]}"
        element["groupId"] = "authored.source-particle"
        element["sourceNode"] = (
            f"authored-source-particle:{TARGET_EFFECT_ID}|source:{SOURCE_EFFECT_ID}|"
            f"element:{source_id}"
        )
        element["kind"] = "particle"
        element["resources"] = [
            {"slotId": slot_id, "assetId": asset_id}
            for slot_id, asset_id in target_bindings
        ]
        element["material"] = material
        element["actionCueAttachment"] = attachment
        element["transformInheritance"] = {
            "enabled": False,
            "masterElementId": "",
        }
        element["detail"] = detail
        element["sourceRecipe"] = normalized_recipe
        element["sourcePresentation"] = source_presentation
        element.pop("authoringOverrides", None)
        if canonical_sha256(element) != SOURCE_PROJECTED_SHA256[index]:
            raise MaterializationError(f"{source_id}: projected source row drifted")
        projected.append(element)
    return projected


def _validate_target_header(document: dict[str, Any]) -> None:
    if (
        document.get("schema") != "lostark.effect-authoring"
        or document.get("version") != 13
        or document.get("effectAssetId") != TARGET_EFFECT_ID
    ):
        raise MaterializationError("2050210 Product document identity changed")
    header = {key: value for key, value in document.items() if key != "elements"}
    if canonical_sha256(header) != EXPECTED_HEADER_SHA256:
        raise MaterializationError("2050210 Product document header drifted")


def validate_target_state(
    document: dict[str, Any],
    source_by_id: dict[str, dict[str, Any]],
    projected_rows: list[dict[str, Any]],
) -> str:
    _validate_target_header(document)
    elements = document.get("elements")
    expected_counts = {
        len(EXPECTED_TUNED_ROWS),
        len(EXPECTED_TUNED_ROWS) + len(projected_rows) - 1,
    }
    if not isinstance(elements, list) or len(elements) not in expected_counts:
        raise MaterializationError("unknown/partial 2050210 Product denominator")
    ids = [row.get("id") for row in elements if isinstance(row, dict)]
    if len(ids) != len(elements) or len(ids) != len(set(ids)):
        raise MaterializationError("2050210 Product IDs are invalid/duplicated")

    for index, (expected_id, expected_sha256) in enumerate(EXPECTED_TUNED_ROWS):
        element = elements[index]
        if element.get("id") != expected_id or canonical_sha256(
            element
        ) != expected_sha256:
            raise MaterializationError(
                f"current tuned row drifted at index {index}: {expected_id}"
            )
    _validate_tuned_provenance(elements[: len(EXPECTED_TUNED_ROWS)], source_by_id)

    tail = elements[len(EXPECTED_TUNED_ROWS) :]
    if not tail:
        return "baseline"
    admitted_rows = projected_rows[1:]
    if tail != admitted_rows:
        raise MaterializationError("selectively projected MakeFlow rows drifted")
    visual_rows = [
        next(row for row in elements if row.get("id") == TUNED_REFERENCE_ID),
        *tail,
    ]
    if (
        len(visual_rows) != SOURCE_OCCURRENCE_ADMISSION_RECEIPT["visualCardinality"]
        or any(row.get("visible") is not True for row in visual_rows)
        or tuple(
            row.get("detail", {}).get("timing", {}).get("startDelaySeconds")
            for row in visual_rows
        )
        != SOURCE_START_SECONDS
        or SOURCE_TARGET_IDS[0] in ids
    ):
        raise MaterializationError("MakeFlow visual cadence/cardinality changed")
    duplicate_policy = SOURCE_OCCURRENCE_ADMISSION_RECEIPT[
        "duplicateSourceNodePolicy"
    ]
    duplicate_rows = [
        row
        for row in visual_rows
        if str(row.get("sourceNode", "")).endswith(
            "|element:" + str(duplicate_policy["sourceElementId"])
        )
    ]
    if (
        [row.get("id") for row in duplicate_rows]
        != [TUNED_REFERENCE_ID, SOURCE_TARGET_IDS[1]]
        or duplicate_policy.get("allowed") is not True
    ):
        raise MaterializationError("explicit duplicate sourceNode policy changed")
    return "materialized"


def build_document(
    target: dict[str, Any], source: dict[str, Any], receipt: dict[str, Any]
) -> dict[str, Any]:
    source_by_id = _validate_imported_document(source)
    projected_rows = build_source_rows(source, receipt)
    state = validate_target_state(target, source_by_id, projected_rows)
    if state == "materialized":
        return copy.deepcopy(target)
    result = copy.deepcopy(target)
    result["elements"].extend(copy.deepcopy(projected_rows[1:]))
    if validate_target_state(result, source_by_id, projected_rows) != "materialized":
        raise MaterializationError("2050210 staged transaction did not materialize")
    return result


def _elements_array_bounds(text: str) -> tuple[int, int]:
    match = re.search(r'"elements"\s*:\s*\[', text)
    if match is None:
        raise MaterializationError("elements array is missing")
    opening = text.find("[", match.start())
    depth = 0
    in_string = False
    escaped = False
    for index in range(opening, len(text)):
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
        elif character == "[":
            depth += 1
        elif character == "]":
            depth -= 1
            if depth == 0:
                return opening + 1, index
    raise MaterializationError("elements array is unterminated")


def render_materialized_text(
    original_text: str,
    original_document: dict[str, Any],
    result_document: dict[str, Any],
) -> str:
    if original_document == result_document:
        return original_text
    baseline_count = len(EXPECTED_TUNED_ROWS)
    if (
        len(original_document.get("elements", [])) != baseline_count
        or result_document.get("elements", [])[:baseline_count]
        != original_document["elements"]
        or len(result_document.get("elements", []))
        != baseline_count + len(SOURCE_ELEMENT_IDS) - 1
    ):
        raise MaterializationError("layout-preserving append received unknown state")
    array_start, array_end = _elements_array_bounds(original_text)
    array_text = original_text[array_start:array_end]
    body_end = len(array_text.rstrip())
    if body_end == 0 or array_text[:body_end][-1] != "}":
        raise MaterializationError("baseline elements array layout changed")
    rendered_rows = []
    for row in result_document["elements"][baseline_count:]:
        rendered = json.dumps(row, ensure_ascii=False, indent=2, allow_nan=False)
        rendered_rows.append("\n".join("    " + line for line in rendered.splitlines()))
    migrated_array = (
        array_text[:body_end]
        + ",\n"
        + ",\n".join(rendered_rows)
        + array_text[body_end:]
    )
    result = original_text[:array_start] + migrated_array + original_text[array_end:]
    if parse_json_text(result, "staged 2050210 output") != result_document:
        raise MaterializationError("layout-preserving JSON round-trip changed output")
    return result


def validate_animation_event_text(text: str) -> None:
    lines = text.splitlines()
    if not lines:
        raise MaterializationError("DimensionMaster animevents is empty")
    header = re.fullmatch(
        r'LOSTARK_ANIM_EVENTS\s+6\s+"DimensionMaster"\s+(\d+)', lines[0]
    )
    if header is None or int(header.group(1)) != len(lines) - 1:
        raise MaterializationError("DimensionMaster animevents header/count changed")
    cues = [
        line
        for line in lines[1:]
        if line.startswith('"pc_sp_m_00_sk_sk_willowrend" EFFECT ')
        and f'payload="{TARGET_EFFECT_ID}"' in line
    ]
    if cues != [EXPECTED_A_CUE_LINE]:
        raise MaterializationError(
            "A cue must be one root FOLLOW + action_facing orientation occurrence"
        )


def _atomic_replace(
    target_path: pathlib.Path, text: str, expected_document: dict[str, Any], *, bom: bool
) -> None:
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{target_path.name}.", suffix=".tmp", dir=target_path.parent
    )
    temporary_path = pathlib.Path(temporary_name)
    try:
        with os.fdopen(descriptor, "wb") as stream:
            if bom:
                stream.write(codecs.BOM_UTF8)
            stream.write(text.encode("utf-8"))
            stream.flush()
            os.fsync(stream.fileno())
        staged_text = temporary_path.read_text(encoding="utf-8-sig")
        staged = parse_json_text(staged_text, str(temporary_path))
        if staged != expected_document:
            raise MaterializationError("temporary JSON round-trip changed transaction")
        os.replace(temporary_path, target_path)
    finally:
        if temporary_path.exists():
            temporary_path.unlink()


def run(
    *,
    write: bool,
    target_path: pathlib.Path = TARGET_PATH,
    source_path: pathlib.Path = IMPORTED_PATH,
    receipt_path: pathlib.Path = RESTORATION_RECEIPT_PATH,
    animation_events_path: pathlib.Path = ANIMATION_EVENTS_PATH,
) -> bool:
    raw = target_path.read_bytes()
    bom = raw.startswith(codecs.BOM_UTF8)
    original_text = raw.decode("utf-8-sig")
    target = parse_json_text(original_text, str(target_path))
    source = load_json(source_path)
    receipt = load_json(receipt_path)
    try:
        animation_text = animation_events_path.read_text(encoding="utf-8-sig")
    except OSError as error:
        raise MaterializationError(
            f"cannot read {animation_events_path}: {error}"
        ) from error
    validate_animation_event_text(animation_text)
    result = build_document(target, source, receipt)
    rendered = render_materialized_text(original_text, target, result)
    changed = rendered != original_text
    if changed and not write:
        raise MaterializationError(
            "2050210 target requires selective materialization; rerun with --write"
        )
    if changed:
        _atomic_replace(target_path, rendered, result, bom=bom)
    return changed


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Selectively materialize DimensionMaster A MakeFlow occurrences"
    )
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--write", action="store_true")
    arguments = parser.parse_args()
    changed = run(write=arguments.write)
    if changed:
        print(f"materialized three missing source-exact MakeFlow rows: {TARGET_PATH}")
    else:
        print("check passed: 9 tuned rows + 3 source-exact rows = 4 visible slashes")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (MaterializationError, OSError, KeyError, TypeError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        raise SystemExit(1)
