#!/usr/bin/env python3
"""Publish the four-class Authored approximation set into product source truth.

This tool never invents an Effect approximation.  It consumes the completed
stage-local Authored documents and their hash-pinned approximation receipts,
validates the current combat skill/animation scope, and transactionally replaces:

* the four classes' in-scope ``effectref=asset`` animation cues;
* ``Data/Effects/EffectCatalog.json``; and
* the deterministic product-rollout receipt.

A multi-clip stage cannot run as one Effect at the first clip's fixed play rate.
For those stages the publisher also creates deterministic ``.clipN`` Authored
documents.  They are lossless clip projections, not new approximations: Element
semantics are copied byte-for-byte and only ``startDelaySeconds`` is rebased by
the exact source clip offset.  The stage document is the one-time projection
source and every generated file is hash-pinned in the rollout receipt.  After a
Product document is hand-authored in F1, a normal write fails closed instead of
overwriting it; replacing that seed requires the explicit
``--migrate-managed-projections`` migration switch.

Assemblies, WFX components, and the runtime catalog remain owned by
``build_effect_components.py`` and ``Publish-Effects.ps1``.
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
import shlex
import tempfile
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
DATA_ROOT = REPOSITORY_ROOT / "Data"
AUTHORED_ROOT = DATA_ROOT / "Effects/Authored"
GENERATED_CORRECTION_ROOT = DATA_ROOT / "Effects/AuthoredCorrections/Generated"
CATALOG_PATH = DATA_ROOT / "Effects/EffectCatalog.json"
PLAYER_SKILLS_PATH = DATA_ROOT / "Balance/PlayerSkills.json"
ROLLOUT_RECEIPT_PATH = (
    GENERATED_CORRECTION_ROOT / "FourClassCombat.authored-product-rollout.json"
)
APPROXIMATION_COVERAGE_PATH = (
    GENERATED_CORRECTION_ROOT / "FourClassCombat.authored-approximation-coverage.json"
)
RUNTIME_CATALOG_PATH = (
    REPOSITORY_ROOT / "Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json"
)

MANIFEST_SCHEMA = "lostark.combat-effect-source-stage-manifest"
MANIFEST_VERSION = 1
APPROXIMATION_RECEIPT_SCHEMA = "lostark.effect-authored-approximation-receipt"
APPROXIMATION_RECEIPT_VERSION = 1
APPROXIMATION_REUSE_RECEIPT_SCHEMA = (
    "lostark.effect-authored-approximation-reuse-receipt"
)
APPROXIMATION_REUSE_RECEIPT_VERSION = 1
ROLLOUT_SCHEMA = "lostark.four-class-authored-product-rollout"
ROLLOUT_VERSION = 2
APPROXIMATION_COVERAGE_SCHEMA = "lostark.effect-authored-approximation-coverage"
APPROXIMATION_COVERAGE_VERSION = 1
EFFECT_DOCUMENT_SCHEMA = "lostark.effect-authoring"
EFFECT_DOCUMENT_VERSION = 12
STABLE_ID = re.compile(r"^[A-Za-z0-9_.-]+$")

EXPECTED_SKILL_COUNT = 51
EXPECTED_STAGE_COUNT = 74
EXPECTED_CLIP_OCCURRENCE_COUNT = 113
EXPECTED_EFFECT_BEARING_STAGE_COUNT = 73
EXPECTED_INTENTIONALLY_SILENT_STAGE_COUNT = 1
EXPECTED_VISUAL_CLIP_OCCURRENCE_COUNT = 102
EXPECTED_SILENT_CLIP_OCCURRENCE_COUNT = 11
EXPECTED_DERIVED_CLIP_TARGET_COUNT = 48
EXPECTED_RETAINED_STAGE_TARGET_COUNT = 53
EXPECTED_STAGE_SOURCE_TARGET_COUNT = 72
EXPECTED_PRODUCT_TARGET_COUNT = 101
EXPECTED_PRODUCT_CUE_COUNT = 101
EXPECTED_TRIMMED_ELEMENT_COUNT = 0
EXPECTED_UNMAPPED_ELEMENT_COUNT = 0
EXPECTED_SILENT_IDENTITY = ("ARTIST", 31210, 1)
PROTECTED_STAGE_IDENTITY = ("DIMENSIONMASTER", 2050210, 0)
PROTECTED_TARGET_ID = "effect.dimensionmaster.skill.2050210.authored-baseline"
PROTECTED_MATERIALIZATION_PATH = (
    DATA_ROOT
    / "Effects/AuthoredCorrections/DimensionMaster/"
    "effect.dimensionmaster.skill.2050210.authored-baseline.materialization.json"
)

CLASS_CONTRACTS: tuple[dict[str, Any], ...] = (
    {
        "animationAssetId": "DimensionMaster",
        "characterClass": "DIMENSIONMASTER",
        "classToken": "dimensionmaster",
        "combatSlots": frozenset(
            {"LMB", "Q", "W", "E", "R", "A", "S", "D", "F", "T", "V", "ALT_V"}
        ),
    },
    {
        "animationAssetId": "LanceMaster",
        "characterClass": "LANCE_MASTER",
        "classToken": "lancemaster",
        "combatSlots": frozenset(
            {"LMB", "Q", "W", "E", "R", "A", "S", "T", "V", "ALT_V"}
        ),
    },
    {
        "animationAssetId": "Artist",
        "characterClass": "ARTIST",
        "classToken": "artist",
        "combatSlots": frozenset(
            {"LMB", "Q", "W", "E", "R", "A", "S", "V", "ALT_V"}
        ),
    },
    {
        "animationAssetId": "Warlord",
        "characterClass": "WARLORD",
        "classToken": "warlord",
        "combatSlots": frozenset(
            {"LMB", "Q", "W", "E", "R", "A", "S", "D", "F", "T", "X", "V", "ALT_V"}
        ),
    },
)


def _load_json(path: Path, label: str) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ValueError(f"{label} could not be read: {path}: {error}") from error
    if not isinstance(value, dict):
        raise ValueError(f"{label} must be a JSON object: {path}")
    return value


def _canonical_json(value: Any) -> str:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    )


def _sha256_json(value: Any) -> str:
    return hashlib.sha256(_canonical_json(value).encode("utf-8")).hexdigest()


def _sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _json_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2) + "\n"
    ).encode("utf-8")


def _data_relative(path: Path) -> str:
    resolved = path.resolve()
    try:
        relative = resolved.relative_to(DATA_ROOT.resolve())
    except ValueError as error:
        raise ValueError(f"Path escaped Data: {path}") from error
    return relative.as_posix()


def _repository_relative(path: Path) -> str:
    resolved = path.resolve()
    try:
        relative = resolved.relative_to(REPOSITORY_ROOT.resolve())
    except ValueError as error:
        raise ValueError(f"Path escaped repository: {path}") from error
    return relative.as_posix()


def _resolve_data_path(value: Any, label: str) -> Path:
    if not isinstance(value, str) or not value or "\\" in value:
        raise ValueError(f"{label} must be a Data-relative POSIX path")
    relative = value.removeprefix("Data/")
    candidate = (DATA_ROOT / relative).resolve()
    try:
        candidate.relative_to(DATA_ROOT.resolve())
    except ValueError as error:
        raise ValueError(f"{label} escaped Data: {value}") from error
    if not candidate.is_file():
        raise ValueError(f"{label} does not exist: {value}")
    return candidate


def _binding_clip(value: Any) -> dict[str, Any]:
    if isinstance(value, str):
        if not value:
            raise ValueError("Animation binding contains an empty clip")
        return {"clip": value, "playMs": None, "playRate": 1.0}
    if not isinstance(value, dict):
        raise ValueError("Animation binding has an unsupported clip shape")
    clip = value.get("clip")
    if not isinstance(clip, str) or not clip:
        raise ValueError("Animation binding object contains no clip")
    play_ms = value.get("playMs")
    if play_ms is not None:
        if isinstance(play_ms, bool) or not isinstance(play_ms, int) or play_ms <= 0:
            raise ValueError(f"Animation binding playMs is invalid: {clip}")
    play_rate = value.get("playRate", 1.0)
    if (
        isinstance(play_rate, bool)
        or not isinstance(play_rate, (int, float))
        or not math.isfinite(float(play_rate))
        or float(play_rate) <= 0.0
    ):
        raise ValueError(f"Animation binding playRate is invalid: {clip}")
    return {
        "clip": clip,
        "playMs": play_ms,
        "playRate": float(play_rate),
    }


def _flatten_binding_clip(value: Any) -> list[dict[str, Any]]:
    if isinstance(value, list):
        if not value:
            raise ValueError("Animation binding contains an empty stage")
        result: list[dict[str, Any]] = []
        for nested in value:
            result.extend(_flatten_binding_clip(nested))
        return result
    return [_binding_clip(value)]


def _binding_stages(raw_clips: Any) -> list[list[dict[str, Any]]]:
    if not isinstance(raw_clips, list) or not raw_clips:
        raise ValueError("Animation binding must contain clips")
    nested = [isinstance(value, list) for value in raw_clips]
    if any(nested) and not all(nested):
        raise ValueError("Animation binding mixes staged and sequential clips")
    if all(nested):
        return [_flatten_binding_clip(value) for value in raw_clips]
    result: list[dict[str, Any]] = []
    for value in raw_clips:
        result.extend(_flatten_binding_clip(value))
    return [result]


def _target_id(
    class_token: str, skill_id: int, stage_index: int, stage_count: int
) -> str:
    if stage_count == 1:
        return f"effect.{class_token}.skill.{skill_id}.authored-baseline"
    return f"effect.{class_token}.skill.{skill_id}.ba{stage_index + 1}"


def _is_intentionally_silent(stage: dict[str, Any]) -> bool:
    decision = stage.get("completionDecision")
    return (
        isinstance(decision, dict)
        and decision.get("decision") == "sourceIntentionallySilent"
    )


def _scope_player_skills() -> dict[str, dict[int, dict[str, Any]]]:
    root = _load_json(PLAYER_SKILLS_PATH, "PlayerSkills")
    rows = root.get("skills")
    if not isinstance(rows, list):
        raise ValueError("PlayerSkills.skills must be an array")
    result: dict[str, dict[int, dict[str, Any]]] = {}
    for contract in CLASS_CONTRACTS:
        stable_class = contract["characterClass"]
        combat_slots = contract["combatSlots"]
        by_id: dict[int, dict[str, Any]] = {}
        for raw_row in rows:
            if not isinstance(raw_row, dict):
                continue
            if raw_row.get("characterClass") != stable_class:
                continue
            if raw_row.get("inputSlot") not in combat_slots:
                continue
            if str(raw_row.get("setsStance") or "NONE").upper() != "NONE":
                continue
            skill_id = int(raw_row.get("skillId", 0))
            if skill_id <= 0 or skill_id in by_id:
                raise ValueError(f"Duplicate or invalid combat skill: {stable_class}/{skill_id}")
            if stable_class != "DIMENSIONMASTER" and raw_row.get("effectId") not in (None, ""):
                raise ValueError(
                    f"Non-DimensionMaster gameplay effectId must remain blank: {stable_class}/{skill_id}"
                )
            by_id[skill_id] = raw_row
        result[stable_class] = by_id
    return result


def _load_scope() -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    player_skills = _scope_player_skills()
    stages: list[dict[str, Any]] = []
    source_inputs: list[dict[str, Any]] = [
        {
            "path": _repository_relative(PLAYER_SKILLS_PATH),
            "sha256": _sha256_file(PLAYER_SKILLS_PATH),
        }
    ]
    skill_count = 0
    clip_occurrence_count = 0
    for contract in CLASS_CONTRACTS:
        asset_id = contract["animationAssetId"]
        stable_class = contract["characterClass"]
        class_token = contract["classToken"]
        manifest_path = (
            DATA_ROOT
            / "Effects/Imported"
            / asset_id
            / f"{asset_id}.combat-source-stage-manifest.json"
        )
        binding_path = (
            DATA_ROOT
            / "Animation/Authored"
            / asset_id
            / f"{asset_id}.skillbindings.json"
        )
        manifest = _load_json(manifest_path, f"{asset_id} source-stage manifest")
        bindings = _load_json(binding_path, f"{asset_id} skillbindings")
        if (
            manifest.get("schema") != MANIFEST_SCHEMA
            or manifest.get("version") != MANIFEST_VERSION
            or manifest.get("animationAssetId") != asset_id
            or manifest.get("characterClass") != stable_class
        ):
            raise ValueError(f"Source-stage manifest identity mismatch: {asset_id}")
        if (
            bindings.get("animationAssetId") != asset_id
            or bindings.get("characterClass") != stable_class
        ):
            raise ValueError(f"Skillbinding identity mismatch: {asset_id}")
        binding_rows = bindings.get("bindings")
        if not isinstance(binding_rows, list):
            raise ValueError(f"Skillbindings are invalid: {asset_id}")
        binding_by_skill: dict[int, list[list[dict[str, Any]]]] = {}
        for raw_binding in binding_rows:
            if not isinstance(raw_binding, dict):
                raise ValueError(f"Skillbinding row is invalid: {asset_id}")
            skill_id = int(raw_binding.get("skillId", 0))
            if skill_id in binding_by_skill:
                raise ValueError(f"Duplicate skillbinding: {asset_id}/{skill_id}")
            binding_by_skill[skill_id] = _binding_stages(raw_binding.get("clips"))

        raw_skills = manifest.get("skills")
        if not isinstance(raw_skills, list):
            raise ValueError(f"Manifest skills are invalid: {asset_id}")
        manifest_skill_ids = {
            int(row.get("productSkillId", 0))
            for row in raw_skills
            if isinstance(row, dict)
        }
        expected_skill_ids = set(player_skills[stable_class])
        if manifest_skill_ids != expected_skill_ids:
            raise ValueError(
                f"Current combat skill coverage mismatch for {asset_id}: "
                f"missing={sorted(expected_skill_ids - manifest_skill_ids)}, "
                f"extra={sorted(manifest_skill_ids - expected_skill_ids)}"
            )
        seen_skill_ids: set[int] = set()
        for raw_skill in raw_skills:
            if not isinstance(raw_skill, dict):
                raise ValueError(f"Manifest skill row is invalid: {asset_id}")
            skill_id = int(raw_skill.get("productSkillId", 0))
            if skill_id in seen_skill_ids:
                raise ValueError(f"Duplicate manifest skill: {asset_id}/{skill_id}")
            seen_skill_ids.add(skill_id)
            raw_stages = raw_skill.get("stages")
            if not isinstance(raw_stages, list) or not raw_stages:
                raise ValueError(f"Manifest skill has no stages: {asset_id}/{skill_id}")
            binding_stages = binding_by_skill.get(skill_id)
            if binding_stages is None:
                raise ValueError(f"Manifest skill has no binding: {asset_id}/{skill_id}")
            skill_contract = player_skills[stable_class][skill_id]
            manifest_clip_stages: list[list[str]] = []
            stage_count = len(raw_stages)
            if len(binding_stages) != stage_count:
                raise ValueError(
                    f"Manifest/binding stage count mismatch: {asset_id}/{skill_id}"
                )
            for expected_stage_index, raw_stage in enumerate(raw_stages):
                if not isinstance(raw_stage, dict):
                    raise ValueError(f"Manifest stage is invalid: {asset_id}/{skill_id}")
                stage_index = int(raw_stage.get("stageIndex", -1))
                if stage_index != expected_stage_index:
                    raise ValueError(f"Manifest stage order drifted: {asset_id}/{skill_id}")
                raw_clips = raw_stage.get("clips")
                if not isinstance(raw_clips, list) or not raw_clips:
                    raise ValueError(f"Manifest stage has no clips: {asset_id}/{skill_id}/{stage_index}")
                clips: list[str] = []
                manifest_clips: list[dict[str, Any]] = []
                for expected_clip_index, raw_clip in enumerate(raw_clips):
                    if not isinstance(raw_clip, dict):
                        raise ValueError("Manifest clip occurrence must be an object")
                    if int(raw_clip.get("stageClipIndex", -1)) != expected_clip_index:
                        raise ValueError("Manifest stage clip order drifted")
                    clip = raw_clip.get("clip")
                    if not isinstance(clip, str) or not clip:
                        raise ValueError("Manifest clip occurrence has no clip")
                    clips.append(clip)
                    manifest_clips.append(raw_clip)
                binding_stage = binding_stages[stage_index]
                if clips != [row["clip"] for row in binding_stage]:
                    raise ValueError(
                        f"Manifest/binding clip mismatch: "
                        f"{asset_id}/{skill_id}/{stage_index}"
                    )
                stage_timeline_offset = raw_stage.get("timelineOffsetSeconds")
                if (
                    isinstance(stage_timeline_offset, bool)
                    or not isinstance(stage_timeline_offset, (int, float))
                    or not math.isfinite(float(stage_timeline_offset))
                    or float(stage_timeline_offset) < 0.0
                ):
                    raise ValueError(
                        f"Manifest stage timeline offset is invalid: "
                        f"{asset_id}/{skill_id}/{stage_index}"
                    )
                is_hold_loop_stage = (
                    str(skill_contract.get("skillKind") or "") == "HOLD"
                    and stage_count == 3
                    and stage_index == 1
                )
                if is_hold_loop_stage and len(clips) != 1:
                    raise ValueError(
                        f"HOLD loop stage must own one clip: {asset_id}/{skill_id}"
                    )
                clip_bindings: list[dict[str, Any]] = []
                previous_source_end = 0.0
                for clip_index, (manifest_clip, binding_clip) in enumerate(
                    zip(manifest_clips, binding_stage, strict=True)
                ):
                    timeline_offset = manifest_clip.get("timelineOffsetSeconds")
                    source_length = manifest_clip.get("lengthSeconds")
                    if (
                        isinstance(timeline_offset, bool)
                        or not isinstance(timeline_offset, (int, float))
                        or not math.isfinite(float(timeline_offset))
                        or isinstance(source_length, bool)
                        or not isinstance(source_length, (int, float))
                        or not math.isfinite(float(source_length))
                        or float(source_length) <= 0.0
                    ):
                        raise ValueError(
                            f"Manifest clip timing is invalid: "
                            f"{asset_id}/{skill_id}/{stage_index}/{clip_index}"
                        )
                    source_offset = (
                        float(timeline_offset) - float(stage_timeline_offset)
                    )
                    if (
                        source_offset < -1e-6
                        or abs(source_offset - previous_source_end) > 1e-4
                    ):
                        raise ValueError(
                            f"Manifest clip timeline is not contiguous: "
                            f"{asset_id}/{skill_id}/{stage_index}/{clip_index}"
                        )
                    source_offset = max(0.0, source_offset)
                    source_length_value = float(source_length)
                    play_ms = binding_clip["playMs"]
                    playable_source_duration = source_length_value
                    if play_ms is not None:
                        playable_source_duration = min(
                            source_length_value, float(play_ms) * 0.001
                        )
                    clip_bindings.append(
                        {
                            "clip": binding_clip["clip"],
                            "stageClipIndex": clip_index,
                            "sourceTimelineOffsetSeconds": source_offset,
                            "sourceLengthSeconds": source_length_value,
                            "playMs": play_ms,
                            "playRate": binding_clip["playRate"],
                            "playableSourceDurationSeconds": playable_source_duration,
                            "runtimeLoop": is_hold_loop_stage,
                        }
                    )
                    previous_source_end = source_offset + source_length_value
                manifest_clip_stages.append(clips)
                clip_occurrence_count += len(clips)
                stages.append(
                    {
                        "animationAssetId": asset_id,
                        "characterClass": stable_class,
                        "classToken": class_token,
                        "productSkillId": skill_id,
                        "stageIndex": stage_index,
                        "stageCount": stage_count,
                        "stageId": raw_stage.get("stageId"),
                        "clips": clips,
                        "clipBindings": clip_bindings,
                        "sourceStageTimelineOffsetSeconds": float(
                            stage_timeline_offset
                        ),
                        "sourceEventIds": copy.deepcopy(
                            raw_stage.get("sourceEventIds", [])
                        ),
                        "silent": _is_intentionally_silent(raw_stage),
                        "nominalTargetEffectAssetId": _target_id(
                            class_token, skill_id, stage_index, stage_count
                        ),
                        "manifestPath": manifest_path,
                    }
                )
            if manifest_clip_stages != [
                [row["clip"] for row in stage] for stage in binding_stages
            ]:
                raise ValueError(
                    f"Manifest/binding stage partition mismatch: {asset_id}/{skill_id}: "
                    f"{manifest_clip_stages!r} != {binding_stages!r}"
                )
            skill_count += 1
        source_inputs.extend(
            (
                {
                    "path": _repository_relative(manifest_path),
                    "sha256": _sha256_file(manifest_path),
                },
                {
                    "path": _repository_relative(binding_path),
                    "sha256": _sha256_file(binding_path),
                },
            )
        )

    if (
        skill_count != EXPECTED_SKILL_COUNT
        or len(stages) != EXPECTED_STAGE_COUNT
        or clip_occurrence_count != EXPECTED_CLIP_OCCURRENCE_COUNT
    ):
        raise ValueError(
            "Four-class scope count mismatch: "
            f"skills={skill_count}, stages={len(stages)}, clips={clip_occurrence_count}"
        )
    return stages, source_inputs


def _validate_effect_document(path: Path, expected_id: str) -> dict[str, Any]:
    document = _load_json(path, "Authored Effect")
    if (
        document.get("schema") != EFFECT_DOCUMENT_SCHEMA
        or document.get("version") != EFFECT_DOCUMENT_VERSION
        or document.get("effectAssetId") != expected_id
        or path.name != f"{expected_id}.effect.json"
    ):
        raise ValueError(f"Authored Effect identity mismatch: {path}")
    elements = document.get("elements")
    if not isinstance(elements, list) or not elements:
        raise ValueError(f"Authored Effect has no elements: {expected_id}")
    groups: dict[str, dict[str, int]] = {}
    for element in elements:
        if not isinstance(element, dict):
            raise ValueError(f"Authored Effect element is invalid: {expected_id}")
        kind = element.get("kind")
        if kind not in {"mesh", "sprite"}:
            raise ValueError(f"Product Authored Effect contains {kind}: {expected_id}")
        resources = element.get("resources")
        if not isinstance(resources, list):
            raise ValueError(f"Product Authored Effect resources are invalid: {expected_id}")
        slot_counts: dict[str, int] = {}
        for resource in resources:
            if not isinstance(resource, dict):
                raise ValueError(
                    f"Product Authored Effect resource is invalid: {expected_id}"
                )
            slot_id = resource.get("slotId")
            asset_id = resource.get("assetId")
            if (
                not isinstance(slot_id, str)
                or not STABLE_ID.fullmatch(slot_id)
                or not isinstance(asset_id, str)
                or not asset_id
            ):
                raise ValueError(
                    f"Product Authored Effect resource identity is invalid: {expected_id}"
                )
            slot_counts[slot_id] = slot_counts.get(slot_id, 0) + 1
        if expected_id != PROTECTED_TARGET_ID:
            if kind == "mesh":
                use_model_material = element.get("detail", {}).get("mesh", {}).get(
                    "useModelMaterial"
                )
                if (
                    slot_counts.get("meshModel", 0) != 1
                    or not isinstance(use_model_material, bool)
                    or (
                        use_model_material is False
                        and slot_counts.get("base", 0) != 1
                    )
                ):
                    raise ValueError(
                        f"Approximation Mesh is not drawable: {expected_id}/"
                        f"{element.get('id')}"
                    )
            elif slot_counts.get("base", 0) != 1:
                raise ValueError(
                    f"Approximation Sprite is not drawable: {expected_id}/"
                    f"{element.get('id')}"
                )
        recipe = element.get("sourceRecipe")
        if not isinstance(recipe, dict) or recipe.get("enabled") is not False:
            raise ValueError(f"Product Authored Effect enables sourceRecipe: {expected_id}")
        attachment = element.get("actionCueAttachment")
        socket_transform = (
            attachment.get("socketLocalTransform")
            if isinstance(attachment, dict)
            else None
        )
        if (
            not isinstance(attachment, dict)
            or attachment.get("enabled") is not True
            or attachment.get("follow") is not False
            or attachment.get("sourceAnchorSlotId") != "root"
            or attachment.get("runtimeAnchorSlotId") != "root"
            or not isinstance(socket_transform, dict)
            or socket_transform.get("position") != [0.0, 0.0, 0.0]
            or socket_transform.get("rotationDegrees") != [0.0, 0.0, 0.0]
            or socket_transform.get("scale") != [1.0, 1.0, 1.0]
        ):
            raise ValueError(f"Product Authored Effect is not a root snapshot: {expected_id}")
        group_id = element.get("groupId")
        if not isinstance(group_id, str) or not group_id:
            raise ValueError(f"Product Authored Effect has no groupId: {expected_id}")
        counts = groups.setdefault(group_id, {"mesh": 0, "sprite": 0})
        counts[kind] += 1
        if kind == "sprite":
            detail = element.get("detail")
            sprite = detail.get("sprite") if isinstance(detail, dict) else None
            if not isinstance(sprite, dict) or float(
                sprite.get("billboardRollDegrees", 0.0)
            ) != -90.0:
                raise ValueError(f"Product Sprite is not authored at -90 degrees: {expected_id}")
    for group_id, counts in groups.items():
        if counts["mesh"] > 5 or counts["sprite"] > 3:
            raise ValueError(
                f"Authored occurrence exceeds Mesh/Sprite budget: "
                f"{expected_id}/{group_id}={counts}"
            )
    return document


def _normalized_runtime_document(document: dict[str, Any]) -> dict[str, Any]:
    """Drop authoring-only identity while retaining every runtime visual field."""
    normalized = {
        "schema": document.get("schema"),
        "version": document.get("version"),
        "particleSystem": copy.deepcopy(document.get("particleSystem")),
        "modelCues": copy.deepcopy(document.get("modelCues")),
        "elements": [],
    }
    for raw_element in document.get("elements", []):
        element = copy.deepcopy(raw_element)
        for field in ("id", "displayName", "groupId", "sourceNode"):
            element.pop(field, None)
        normalized["elements"].append(element)
    return normalized


def _validate_receipt_material_contract(
    document: dict[str, Any], receipt: dict[str, Any], expected_id: str
) -> None:
    selected: dict[str, dict[str, Any]] = {}
    for occurrence in receipt.get("occurrences", []):
        if not isinstance(occurrence, dict):
            raise ValueError("Approximation receipt occurrence is invalid")
        candidates = occurrence.get("candidates")
        if not isinstance(candidates, list):
            raise ValueError("Approximation receipt candidates are invalid")
        for candidate in candidates:
            if (
                not isinstance(candidate, dict)
                or candidate.get("selectionDecision") != "selected"
            ):
                continue
            element_id = candidate.get("targetElementId")
            if (
                not isinstance(element_id, str)
                or not STABLE_ID.fullmatch(element_id)
                or element_id in selected
            ):
                raise ValueError("Selected material receipt identity is invalid")
            selected[element_id] = candidate
    elements = document.get("elements", [])
    document_ids = {str(element.get("id") or "") for element in elements}
    if set(selected) != document_ids:
        raise ValueError(
            f"Selected material receipt coverage drifted: {expected_id}"
        )
    for element in elements:
        element_id = str(element["id"])
        candidate = selected[element_id]
        decision = candidate.get("materialDecision")
        material = element.get("material")
        provenance = candidate.get("materialProvenance")
        drawable = candidate.get("standaloneDrawableResourceDecision")
        source_profile = (
            material.get("sourceProfile")
            if isinstance(material, dict)
            else None
        )
        if not isinstance(drawable, dict):
            raise ValueError(
                f"Approximation drawable receipt is missing: "
                f"{expected_id}/{element_id}"
            )
        target_resources = drawable.get("targetResourceBindings")
        target_use_model_material = drawable.get("targetUseModelMaterial")
        drawable_decision = drawable.get("decision")
        if (
            not isinstance(material, dict)
            or material.get("templateId") != "effect.standard"
            or not isinstance(source_profile, dict)
            or not isinstance(provenance, dict)
            or not isinstance(target_resources, list)
            or _canonical_json(target_resources)
            != _canonical_json(element.get("resources"))
            or drawable.get("targetResourceBindingsSha256")
            != _sha256_json(target_resources)
            or not isinstance(target_use_model_material, bool)
            or target_use_model_material
            is not element.get("detail", {}).get("mesh", {}).get(
                "useModelMaterial"
            )
        ):
            raise ValueError(
                f"Approximation material/receipt evidence drifted: "
                f"{expected_id}/{element_id}"
            )
        if decision == "acceptedStandardApproximation":
            expected_material = copy.deepcopy(provenance)
            expected_material["templateId"] = "effect.standard"
            expected_material["sourceProfile"] = {"enabled": False}
            if (
                source_profile != {"enabled": False}
                or _canonical_json(expected_material) != _canonical_json(material)
            ):
                raise ValueError(
                    f"Accepted standard material re-enabled source profile: "
                    f"{expected_id}/{element_id}"
                )
        elif decision == "sourceMaterialPreserved":
            if (
                _canonical_json(provenance) != _canonical_json(material)
                or source_profile.get("enabled") is not True
                or source_profile.get("runtimeShaderProfileId")
                != "effect.ue3.grouped-translucent.v1"
                or source_profile.get("semanticStatus")
                != "reconstructed_profile"
                or source_profile.get("productAdmissionStatus")
                != "ADMITTED_RECONSTRUCTED_PROFILE"
            ):
                raise ValueError(
                    f"Preserved source material is not executable: "
                    f"{expected_id}/{element_id}"
                )
        else:
            raise ValueError(
                f"Unknown approximation material decision: "
                f"{expected_id}/{element_id}/{decision}"
            )
        if drawable_decision in {
            "acceptedElementTextureApproximation",
            "acceptedGroupTextureApproximation",
        }:
            source_resources = drawable.get("sourceResourceBindings")
            promoted = drawable.get("promotedSourceResource")
            model_resource = drawable.get("modelResource")
            model_info = drawable.get("modelInfoEvidence")
            preflight = drawable.get("runtimeDrawablePreflight")
            source_resource_projection = (
                [
                    {
                        "slotId": row.get("slotId"),
                        "assetId": row.get("assetId"),
                    }
                    for row in source_resources
                    if isinstance(row, dict)
                ]
                if isinstance(source_resources, list)
                else []
            )
            base_rows = [
                row for row in target_resources
                if isinstance(row, dict) and row.get("slotId") == "base"
            ]
            mesh_rows = [
                row for row in target_resources
                if isinstance(row, dict) and row.get("slotId") == "meshModel"
            ]
            expected_scope = (
                "sourceElement"
                if drawable_decision == "acceptedElementTextureApproximation"
                else "sourceCascadeGroup"
            )
            if (
                not isinstance(source_resources, list)
                or len(source_resource_projection) != len(source_resources)
                or drawable.get("sourceResourceBindingsSha256")
                != _sha256_json(source_resource_projection)
                or drawable.get("sourceSlots")
                != [row["slotId"] for row in source_resource_projection]
                or "base" in drawable.get("sourceSlots", [])
                or len(base_rows) != 1
                or len(mesh_rows) != 1
                or target_use_model_material is not False
                or drawable.get("disposition") != "acceptedApproximation"
                or drawable.get("carrierScope") != expected_scope
                or not isinstance(promoted, dict)
                or not isinstance(model_resource, dict)
                or model_resource not in source_resources
                or not isinstance(model_info, dict)
                or not isinstance(preflight, dict)
                or promoted.get("sourceGroupId") != drawable.get("sourceGroupId")
                or promoted.get("assetId") != base_rows[0].get("assetId")
                or promoted.get("slotId") in {None, "meshModel"}
                or re.fullmatch(
                    r"[0-9a-f]{64}", str(promoted.get("resourceSha256") or "")
                ) is None
                or re.fullmatch(
                    r"[0-9a-f]{64}",
                    str(promoted.get("sourceDocumentSha256") or ""),
                ) is None
                or model_resource.get("assetId") != mesh_rows[0].get("assetId")
                or model_info.get("decision")
                != "embeddedFirstBaseTextureAbsent"
                or model_info.get("modelAssetId") != model_resource.get("assetId")
                or model_info.get("modelResourceSha256")
                != model_resource.get("resourceSha256")
                or model_info.get("embeddedFirstBaseTexturePresent") is not False
                or model_info.get("firstBaseTexture") != ""
                or preflight.get("result") != "executableBaseOverride"
                or preflight.get("rendererPath")
                != "Effect_DocumentRenderer.Mesh.BaseOverride"
                or preflight.get("targetUseModelMaterial") is not False
                or preflight.get("baseResource") != base_rows[0]
                or preflight.get("baseResourceSha256")
                != promoted.get("resourceSha256")
                or (
                    drawable_decision == "acceptedElementTextureApproximation"
                    and promoted.get("sourceElementId")
                    != candidate.get("sourceElementId")
                )
            ):
                raise ValueError(
                    f"Mesh Base approximation evidence drifted: "
                    f"{expected_id}/{element_id}/{drawable_decision}"
                )
        allowed_drawable_decisions = {
            "acceptedModelMaterialApproximation": "mesh",
            "meshElementMaterialBasePreserved": "mesh",
            "acceptedElementTextureApproximation": "mesh",
            "acceptedGroupTextureApproximation": "mesh",
            "acceptedSourceTextureBaseAlias": "sprite",
            "spriteBaseResourcePreserved": "sprite",
        }
        if allowed_drawable_decisions.get(drawable_decision) != element.get("kind"):
            raise ValueError(
                f"Unknown or mismatched drawable resource decision: "
                f"{expected_id}/{element_id}/{drawable_decision}"
            )


def _load_approximation_receipts() -> dict[tuple[str, int, int], dict[str, Any]]:
    receipts: dict[tuple[str, int, int], dict[str, Any]] = {}
    if not GENERATED_CORRECTION_ROOT.is_dir():
        raise ValueError("Generated Authored approximation receipt root is missing")
    for path in sorted(
        GENERATED_CORRECTION_ROOT.rglob("*.approximation-receipt.json")
    ):
        receipt = _load_json(path, "Authored approximation receipt")
        schema_version = (receipt.get("schema"), receipt.get("version"))
        if schema_version not in {
            (APPROXIMATION_RECEIPT_SCHEMA, APPROXIMATION_RECEIPT_VERSION),
            (
                APPROXIMATION_REUSE_RECEIPT_SCHEMA,
                APPROXIMATION_REUSE_RECEIPT_VERSION,
            ),
        }:
            raise ValueError(f"Unexpected approximation receipt: {path}")
        identity = (
            str(receipt.get("characterClass") or ""),
            int(receipt.get("productSkillId", 0)),
            int(receipt.get("stageIndex", -1)),
        )
        if identity in receipts:
            raise ValueError(f"Duplicate approximation receipt identity: {identity}")
        receipts[identity] = {"path": path, "document": receipt}
    return receipts


def _attach_documents(stages: list[dict[str, Any]]) -> None:
    receipts = _load_approximation_receipts()
    expected_receipt_identities: set[tuple[str, int, int]] = set()
    silent_identities: set[tuple[str, int, int]] = set()
    for stage in stages:
        identity = (
            stage["characterClass"],
            stage["productSkillId"],
            stage["stageIndex"],
        )
        target_id = stage["nominalTargetEffectAssetId"]
        nominal_path = AUTHORED_ROOT / f"{target_id}.effect.json"
        if stage["silent"]:
            silent_identities.add(identity)
            if nominal_path.exists() or identity in receipts:
                raise ValueError(
                    f"Intentionally-silent stage has stale product output: {identity}"
                )
            continue
        if identity == PROTECTED_STAGE_IDENTITY:
            if target_id != PROTECTED_TARGET_ID:
                raise ValueError("Protected DimensionMaster target identity drifted")
            document = _validate_effect_document(nominal_path, target_id)
            stage["documentPath"] = nominal_path
            stage["document"] = document
            stage["documentFileSha256"] = _sha256_file(nominal_path)
            stage["normalizedRuntimeSha256"] = _sha256_json(
                _normalized_runtime_document(document)
            )
            stage["sourceStatus"] = "preservedExisting"
            continue
        expected_receipt_identities.add(identity)
        receipt_row = receipts.get(identity)
        if receipt_row is None:
            raise ValueError(f"Effect-bearing stage has no approximation receipt: {identity}")
        receipt = receipt_row["document"]
        if receipt.get("schema") == APPROXIMATION_REUSE_RECEIPT_SCHEMA:
            if nominal_path.exists():
                raise ValueError(f"Reused product stage has a stale nominal document: {identity}")
            if receipt.get("nominalTargetEffectAssetId") != target_id:
                raise ValueError(f"Reuse receipt nominal target drifted: {identity}")
            nominal_authoring_path = receipt.get("nominalTargetAuthoringPath")
            if not isinstance(nominal_authoring_path, str):
                raise ValueError(f"Reuse receipt nominal path is invalid: {identity}")
            expected_nominal_relative = _data_relative(nominal_path)
            if nominal_authoring_path.removeprefix("Data/") != expected_nominal_relative:
                raise ValueError(f"Reuse receipt nominal path drifted: {identity}")
            reused_target = receipt.get("reusesProductTarget")
            if not isinstance(reused_target, str) or not STABLE_ID.fullmatch(reused_target):
                raise ValueError(f"Reuse receipt product target is invalid: {identity}")
            document_path = _resolve_data_path(
                receipt.get("reusedTargetAuthoringPath"),
                f"Reuse targetAuthoringPath {identity}",
            )
            document = _validate_effect_document(document_path, reused_target)
            normalized_hash = _sha256_json(_normalized_runtime_document(document))
            equivalence = receipt.get("sourceCarrierEquivalence")
            nominal_output = receipt.get("nominalOutputEvidence")
            if (
                not isinstance(equivalence, dict)
                or equivalence.get("normalizedContentSha256")
                != receipt.get("normalizedSourceCarrierSha256")
                or not isinstance(nominal_output, dict)
                or nominal_output.get("particleCount") != 0
                or nominal_output.get("elementCount") != len(document["elements"])
                or nominal_output.get("meshCount")
                != sum(element.get("kind") == "mesh" for element in document["elements"])
                or nominal_output.get("spriteCount")
                != sum(element.get("kind") == "sprite" for element in document["elements"])
                or receipt.get("normalizedAuthoredContentSha256") != normalized_hash
                or receipt.get("reusedTargetDocumentSha256") != _sha256_json(document)
            ):
                raise ValueError(f"Approximation reuse equivalence drifted: {identity}")
            _validate_receipt_material_contract(document, receipt, reused_target)
            stage["documentPath"] = document_path
            stage["document"] = document
            stage["documentFileSha256"] = _sha256_file(document_path)
            stage["normalizedRuntimeSha256"] = normalized_hash
            stage["sourceStatus"] = "reusesProductTarget"
            stage["productTargetEffectAssetId"] = reused_target
            stage["reusesProductTarget"] = reused_target
            stage["nominalAuthoringPath"] = nominal_authoring_path.removeprefix(
                "Data/"
            )
            stage["nominalDocumentCanonicalSha256"] = nominal_output.get(
                "documentSha256"
            )
            stage["approximationReceiptPath"] = receipt_row["path"]
            stage["approximationReceiptSha256"] = _sha256_file(
                receipt_row["path"]
            )
            stage["approximationReceiptDocument"] = receipt
            continue
        if receipt.get("targetEffectAssetId") != target_id:
            raise ValueError(f"Approximation receipt target drifted: {identity}")
        document_path = _resolve_data_path(
            receipt.get("targetAuthoringPath"),
            f"Approximation targetAuthoringPath {identity}",
        )
        if document_path.resolve() != nominal_path.resolve():
            raise ValueError(f"Approximation target path drifted: {identity}")
        document = _validate_effect_document(document_path, target_id)
        output = receipt.get("output")
        if not isinstance(output, dict) or output.get("particleCount") != 0:
            raise ValueError(f"Approximation receipt particle count drifted: {identity}")
        actual_mesh_count = sum(
            element.get("kind") == "mesh" for element in document["elements"]
        )
        actual_sprite_count = sum(
            element.get("kind") == "sprite" for element in document["elements"]
        )
        if (
            output.get("elementCount") != len(document["elements"])
            or output.get("meshCount") != actual_mesh_count
            or output.get("spriteCount") != actual_sprite_count
        ):
            raise ValueError(f"Approximation receipt output counts drifted: {identity}")
        if output.get("documentSha256") != _sha256_json(document):
            raise ValueError(f"Approximation receipt document hash drifted: {identity}")
        _validate_receipt_material_contract(document, receipt, target_id)
        stage["documentPath"] = document_path
        stage["document"] = document
        stage["documentFileSha256"] = _sha256_file(document_path)
        stage["normalizedRuntimeSha256"] = _sha256_json(
            _normalized_runtime_document(document)
        )
        stage["sourceStatus"] = "generatedApproximation"
        stage["approximationReceiptPath"] = receipt_row["path"]
        stage["approximationReceiptSha256"] = _sha256_file(receipt_row["path"])
        stage["approximationReceiptDocument"] = receipt

    if silent_identities != {EXPECTED_SILENT_IDENTITY}:
        raise ValueError(f"Intentionally-silent stage identity drifted: {silent_identities}")
    if set(receipts) != expected_receipt_identities:
        raise ValueError(
            "Approximation receipt coverage mismatch: "
            f"missing={sorted(expected_receipt_identities - set(receipts))}, "
            f"extra={sorted(set(receipts) - expected_receipt_identities)}"
        )


def _validate_approximation_coverage(stages: list[dict[str, Any]]) -> dict[str, Any]:
    coverage = _load_json(APPROXIMATION_COVERAGE_PATH, "Authored approximation coverage")
    if (
        coverage.get("schema") != APPROXIMATION_COVERAGE_SCHEMA
        or coverage.get("version") != APPROXIMATION_COVERAGE_VERSION
    ):
        raise ValueError("Authored approximation coverage schema/version drifted")
    summary = coverage.get("summary")
    expected_summary = {
        "stageCount": EXPECTED_STAGE_COUNT,
        "effectBearingStageCount": EXPECTED_EFFECT_BEARING_STAGE_COUNT,
        "intentionallySilentStageCount": EXPECTED_INTENTIONALLY_SILENT_STAGE_COUNT,
        "blockedStageCount": 0,
        "uniqueProductTargetCount": EXPECTED_STAGE_SOURCE_TARGET_COUNT,
        "generatedApproximationTargetCount": 71,
        "protectedProductTargetCount": 1,
        "reusedProductStageCount": 1,
        "selectionReceiptCount": 72,
    }
    if not isinstance(summary, dict) or any(
        summary.get(name) != value for name, value in expected_summary.items()
    ):
        raise ValueError(f"Authored approximation coverage counts drifted: {summary}")
    raw_rows = coverage.get("stages")
    if not isinstance(raw_rows, list) or len(raw_rows) != len(stages):
        raise ValueError("Authored approximation coverage stage list drifted")
    by_identity: dict[tuple[str, int, int], dict[str, Any]] = {}
    for row in raw_rows:
        if not isinstance(row, dict):
            raise ValueError("Authored approximation coverage row is invalid")
        identity = (
            str(row.get("characterClass") or ""),
            int(row.get("productSkillId", 0)),
            int(row.get("stageIndex", -1)),
        )
        if identity in by_identity:
            raise ValueError(f"Duplicate Authored coverage stage: {identity}")
        by_identity[identity] = row
    expected_identities = {
        (stage["characterClass"], stage["productSkillId"], stage["stageIndex"])
        for stage in stages
    }
    if set(by_identity) != expected_identities:
        raise ValueError("Authored approximation coverage identities drifted")
    for stage in stages:
        identity = (
            stage["characterClass"],
            stage["productSkillId"],
            stage["stageIndex"],
        )
        row = by_identity[identity]
        if stage["silent"]:
            if (
                row.get("decision") != "sourceIntentionallySilent"
                or row.get("nominalTargetEffectAssetId")
                != stage["nominalTargetEffectAssetId"]
            ):
                raise ValueError(f"Silent coverage decision drifted: {identity}")
            continue
        if identity == PROTECTED_STAGE_IDENTITY:
            if (
                row.get("decision") != "protectedProductTarget"
                or row.get("targetEffectAssetId") != PROTECTED_TARGET_ID
            ):
                raise ValueError(f"Protected coverage decision drifted: {identity}")
            continue
        if stage.get("sourceStatus") == "reusesProductTarget":
            if (
                row.get("decision") != "reusesProductTarget"
                or row.get("nominalTargetEffectAssetId")
                != stage["nominalTargetEffectAssetId"]
                or row.get("reusesProductTarget")
                != stage["reusesProductTarget"]
                or row.get("normalizedAuthoredContentSha256")
                != stage["normalizedRuntimeSha256"]
            ):
                raise ValueError(f"Reuse coverage decision drifted: {identity}")
        elif (
            row.get("decision") != "authoredProductTarget"
            or row.get("targetEffectAssetId")
            != stage["nominalTargetEffectAssetId"]
        ):
            raise ValueError(f"Authored coverage decision drifted: {identity}")
        expected_receipt_path = _data_relative(stage["approximationReceiptPath"])
        if str(row.get("receiptPath") or "").removeprefix("Data/") != expected_receipt_path:
            raise ValueError(f"Authored coverage receipt path drifted: {identity}")
    return coverage


def _selected_source_evidence(
    stage: dict[str, Any],
) -> dict[str, dict[str, Any]]:
    receipt = stage.get("approximationReceiptDocument")
    if not isinstance(receipt, dict):
        return {}
    result: dict[str, dict[str, Any]] = {}
    stage_offset = stage["sourceStageTimelineOffsetSeconds"]
    for occurrence in receipt.get("occurrences", []):
        if not isinstance(occurrence, dict):
            raise ValueError("Approximation receipt occurrence is invalid")
        occurrence_event_values = [occurrence.get("sourceEventId")]
        occurrence_source_ids = occurrence.get("sourceEventIds", [])
        if not isinstance(occurrence_source_ids, list):
            raise ValueError("Approximation receipt sourceEventIds is invalid")
        occurrence_event_values.extend(occurrence_source_ids)
        candidates = occurrence.get("candidates", [])
        if not isinstance(candidates, list):
            raise ValueError("Approximation receipt candidates are invalid")
        for candidate in candidates:
            if (
                not isinstance(candidate, dict)
                or candidate.get("selectionDecision") != "selected"
            ):
                continue
            element_id = candidate.get("targetElementId")
            source_time = candidate.get("sourceTimeSeconds")
            if (
                not isinstance(element_id, str)
                or not STABLE_ID.fullmatch(element_id)
                or isinstance(source_time, bool)
                or not isinstance(source_time, (int, float))
                or not math.isfinite(float(source_time))
            ):
                raise ValueError("Selected source ownership evidence is invalid")
            stage_local_source_time = float(source_time) - stage_offset
            if stage_local_source_time < -1e-5:
                raise ValueError(
                    f"Selected source time precedes its stage: {element_id}"
                )
            stage_local_source_time = max(0.0, stage_local_source_time)
            receipt_stage_local = occurrence.get("stageLocalTimeSeconds")
            if (
                isinstance(receipt_stage_local, bool)
                or not isinstance(receipt_stage_local, (int, float))
                or not math.isfinite(float(receipt_stage_local))
                or abs(float(receipt_stage_local) - stage_local_source_time) > 1e-4
            ):
                raise ValueError(
                    f"Selected source global/stage-local timing drifted: {element_id}"
                )
            event_values = list(occurrence_event_values)
            event_values.extend(
                (candidate.get("sourceEventId"), candidate.get("occurrenceEventId"))
            )
            source_event_ids: list[str] = []
            for value in event_values:
                if not isinstance(value, str) or not value:
                    continue
                if not STABLE_ID.fullmatch(value):
                    raise ValueError(f"Selected source event ID is invalid: {value}")
                if value not in source_event_ids:
                    source_event_ids.append(value)
            occurrence_key = occurrence.get("occurrenceKey")
            if not isinstance(occurrence_key, str) or not occurrence_key:
                raise ValueError(
                    f"Selected source ownership has no occurrence key: {element_id}"
                )
            if element_id in result:
                raise ValueError(
                    f"Selected target Element has duplicate ownership: {element_id}"
                )
            result[element_id] = {
                "targetElementId": element_id,
                "sourceTimeSeconds": float(source_time),
                "stageLocalSourceTimeSeconds": stage_local_source_time,
                "occurrenceKey": occurrence_key,
                "sourceEventIds": source_event_ids,
            }
    return result


def _locate_element_clip(
    stage: dict[str, Any], source_delay: float
) -> tuple[str, int | None]:
    if not math.isfinite(source_delay) or source_delay < 0.0:
        return "unmapped", None
    bindings = stage["clipBindings"]
    epsilon = 1e-5
    for index, binding in enumerate(bindings):
        start = binding["sourceTimelineOffsetSeconds"]
        length = binding["sourceLengthSeconds"]
        source_end = start + length
        is_last = index + 1 == len(bindings)
        inside_source_clip = (
            source_delay >= start - epsilon
            and (
                source_delay < source_end - epsilon
                or (is_last and source_delay <= source_end + epsilon)
            )
        )
        if not inside_source_clip:
            continue
        local_delay = source_delay - start
        playable = binding["playableSourceDurationSeconds"]
        is_trimmed = (
            playable < length - epsilon and local_delay >= playable - epsilon
        )
        return ("trimmed" if is_trimmed else "visual"), index
    return "unmapped", None


def _validate_lossless_clip_projection(
    source_document: dict[str, Any],
    derived_document: dict[str, Any],
    selected_source_elements: list[dict[str, Any]],
) -> None:
    source_by_id = {
        str(element.get("id") or ""): element for element in selected_source_elements
    }
    projected = copy.deepcopy(derived_document)
    projected["effectAssetId"] = source_document["effectAssetId"]
    projected["displayName"] = source_document["displayName"]
    for element in projected["elements"]:
        source = source_by_id.get(str(element.get("id") or ""))
        if source is None:
            raise ValueError("Clip projection introduced an unknown Element")
        element["detail"]["timing"]["startDelaySeconds"] = source["detail"][
            "timing"
        ]["startDelaySeconds"]
    expected = copy.deepcopy(source_document)
    expected["elements"] = copy.deepcopy(selected_source_elements)
    if _canonical_json(projected) != _canonical_json(expected):
        raise ValueError(
            "Clip projection changed Element semantics beyond startDelaySeconds"
        )


def _derive_clip_products(
    stages: list[dict[str, Any]],
) -> tuple[dict[Path, bytes], dict[str, int]]:
    desired_documents: dict[Path, bytes] = {}
    visual_occurrences = 0
    silent_occurrences = 0
    derived_targets = 0
    retained_targets = 0
    trimmed_elements = 0
    unmapped_elements = 0
    for stage in stages:
        clip_products: list[dict[str, Any]] = []
        for binding in stage["clipBindings"]:
            clip_products.append(
                {
                    "animationAssetId": stage["animationAssetId"],
                    "characterClass": stage["characterClass"],
                    "productSkillId": stage["productSkillId"],
                    "stageIndex": stage["stageIndex"],
                    **copy.deepcopy(binding),
                    "status": (
                        "sourceIntentionallySilent"
                        if stage["silent"]
                        else "noSelectedCarrier"
                    ),
                    "sourceEventIds": [],
                    "elementCount": 0,
                    "productTargetEffectAssetId": None,
                }
            )
        stage["clipProducts"] = clip_products
        if stage["silent"]:
            silent_occurrences += len(clip_products)
            continue

        document = stage["document"]
        elements = document.get("elements")
        if not isinstance(elements, list) or not elements:
            raise ValueError("Effect-bearing stage has no Authored Elements")
        elements_by_clip: dict[int, list[dict[str, Any]]] = {
            index: [] for index in range(len(clip_products))
        }
        selected_source_evidence = _selected_source_evidence(stage)
        document_element_ids: set[str] = set()
        for element in elements:
            element_id = str(element.get("id") or "")
            if not STABLE_ID.fullmatch(element_id) or element_id in document_element_ids:
                raise ValueError("Authored Element ID is invalid or duplicated")
            document_element_ids.add(element_id)
            timing = element.get("detail", {}).get("timing", {})
            raw_delay = timing.get("startDelaySeconds")
            if (
                isinstance(raw_delay, bool)
                or not isinstance(raw_delay, (int, float))
            ):
                raise ValueError("Authored Element startDelaySeconds is invalid")
            ownership = selected_source_evidence.get(element_id)
            if selected_source_evidence and ownership is None:
                raise ValueError(
                    f"Authored Element has no selected source ownership: {element_id}"
                )
            owner_time = (
                ownership["stageLocalSourceTimeSeconds"]
                if ownership is not None
                else float(raw_delay)
            )
            disposition, clip_index = _locate_element_clip(stage, owner_time)
            if disposition == "trimmed":
                trimmed_elements += 1
                continue
            if disposition == "unmapped" or clip_index is None:
                unmapped_elements += 1
                continue
            elements_by_clip[clip_index].append(element)

        if selected_source_evidence and set(selected_source_evidence) != document_element_ids:
            raise ValueError(
                "Selected source ownership and Authored Element coverage differ: "
                f"missing={sorted(document_element_ids - set(selected_source_evidence))}, "
                f"extra={sorted(set(selected_source_evidence) - document_element_ids)}"
            )

        visual_stage_clip_count = sum(bool(rows) for rows in elements_by_clip.values())
        if visual_stage_clip_count == 0:
            raise ValueError("Effect-bearing stage has no playable visual clip")
        for clip_index, source_elements in elements_by_clip.items():
            product = clip_products[clip_index]
            if not source_elements:
                continue
            visual_occurrences += 1
            binding = stage["clipBindings"][clip_index]
            source_offset = binding["sourceTimelineOffsetSeconds"]
            source_target = stage["productTargetEffectAssetId"]
            if len(stage["clips"]) == 1:
                target = source_target
                document_path = stage["documentPath"]
                product_document = document
                document_bytes = document_path.read_bytes()
                document_file_sha256 = stage["documentFileSha256"]
                retained_targets += 1
                derived = False
            else:
                target = f"{source_target}.clip{clip_index + 1}"
                if not STABLE_ID.fullmatch(target):
                    raise ValueError(f"Derived clip target ID is invalid: {target}")
                document_path = AUTHORED_ROOT / f"{target}.effect.json"
                product_document = copy.deepcopy(document)
                product_document["effectAssetId"] = target
                product_document["displayName"] = target
                product_document["elements"] = copy.deepcopy(source_elements)
                for element in product_document["elements"]:
                    timing = element["detail"]["timing"]
                    rebased_delay = (
                        float(timing["startDelaySeconds"]) - source_offset
                    )
                    if rebased_delay < -1e-5:
                        raise ValueError(
                            f"Clip projection produced a negative local delay: {target}"
                        )
                    timing["startDelaySeconds"] = (
                        0.0 if abs(rebased_delay) <= 1e-5 else rebased_delay
                    )
                _validate_lossless_clip_projection(
                    document, product_document, source_elements
                )
                document_bytes = _json_bytes(product_document)
                document_file_sha256 = _sha256_bytes(document_bytes)
                desired_documents[document_path] = document_bytes
                derived_targets += 1
                derived = True
            element_ids = {
                str(element.get("id") or "") for element in source_elements
            }
            source_ownership = [
                copy.deepcopy(selected_source_evidence[element_id])
                for element_id in sorted(element_ids)
                if element_id in selected_source_evidence
            ]
            source_event_ids: list[str] = []
            for ownership in source_ownership:
                for source_event_id in ownership["sourceEventIds"]:
                    if source_event_id not in source_event_ids:
                        source_event_ids.append(source_event_id)
            if (
                len(source_ownership) != len(element_ids)
                and "approximationReceiptDocument" in stage
            ):
                raise ValueError(
                    f"Visual clip has incomplete source ownership: {target}"
                )
            product.update(
                {
                    "status": "visualBearing",
                    "sourceEventIds": source_event_ids,
                    "sourceElementOwnership": source_ownership,
                    "sourceElementOwnershipSha256": _sha256_json(
                        source_ownership
                    ),
                    "elementCount": len(source_elements),
                    "elementIds": sorted(element_ids),
                    "sourceElementsSha256": _sha256_json(source_elements),
                    "sourceStageTargetEffectAssetId": source_target,
                    "sourceStageAuthoringPath": _data_relative(
                        stage["documentPath"]
                    ),
                    "sourceStageDocumentFileSha256": stage[
                        "documentFileSha256"
                    ],
                    "productTargetEffectAssetId": target,
                    "productAuthoringPath": _data_relative(document_path),
                    "productDocumentFileSha256": document_file_sha256,
                    "normalizedRuntimeSha256": _sha256_json(
                        _normalized_runtime_document(product_document)
                    ),
                    "derivedClipProjection": derived,
                    "documentPath": document_path,
                    "document": product_document,
                }
            )
            if "approximationReceiptPath" in stage:
                product["approximationReceipt"] = {
                    "path": _repository_relative(
                        stage["approximationReceiptPath"]
                    ),
                    "sha256": stage["approximationReceiptSha256"],
                }
        silent_occurrences += sum(
            product["status"] != "visualBearing" for product in clip_products
        )

    counts = {
        "visualClipOccurrenceCount": visual_occurrences,
        "silentClipOccurrenceCount": silent_occurrences,
        "derivedClipTargetOccurrenceCount": derived_targets,
        "retainedStageTargetOccurrenceCount": retained_targets,
        "trimmedElementCount": trimmed_elements,
        "unmappedElementCount": unmapped_elements,
    }
    expected = {
        "visualClipOccurrenceCount": EXPECTED_VISUAL_CLIP_OCCURRENCE_COUNT,
        "silentClipOccurrenceCount": EXPECTED_SILENT_CLIP_OCCURRENCE_COUNT,
        "derivedClipTargetOccurrenceCount": EXPECTED_DERIVED_CLIP_TARGET_COUNT,
        "retainedStageTargetOccurrenceCount": 54,
        "trimmedElementCount": EXPECTED_TRIMMED_ELEMENT_COUNT,
        "unmappedElementCount": EXPECTED_UNMAPPED_ELEMENT_COUNT,
    }
    if counts != expected:
        raise ValueError(f"Clip projection counts drifted: {counts} != {expected}")
    return desired_documents, counts


def _resolve_repeated_clips(stages: list[dict[str, Any]]) -> list[dict[str, Any]]:
    occurrences: dict[tuple[str, str], list[dict[str, Any]]] = {}
    for stage in stages:
        for product in stage["clipProducts"]:
            occurrences.setdefault(
                (stage["animationAssetId"], product["clip"]), []
            ).append({"stage": stage, "product": product})

    evidence: list[dict[str, Any]] = []
    for (asset_id, clip), rows in sorted(occurrences.items()):
        if len(rows) < 2:
            continue
        visual_rows = [
            row for row in rows if row["product"]["status"] == "visualBearing"
        ]
        if len(visual_rows) != len(rows):
            raise ValueError(
                f"Repeated clip mixes visual and silent product semantics: "
                f"{asset_id}/{clip}"
            )
        normalized_hashes = {
            row["product"]["normalizedRuntimeSha256"] for row in visual_rows
        }
        if len(normalized_hashes) != 1:
            detail = [
                (
                    row["stage"]["productSkillId"],
                    row["stage"]["stageIndex"],
                    row["product"]["normalizedRuntimeSha256"],
                )
                for row in visual_rows
            ]
            raise ValueError(
                f"P0 repeated clip has different Authored runtime content: "
                f"{asset_id}/{clip}: {detail}"
            )
        canonical = visual_rows[0]
        canonical_target = canonical["product"]["productTargetEffectAssetId"]
        for row in visual_rows[1:]:
            stage = row["stage"]
            product = row["product"]
            declared_reuse = stage.get("reusesProductTarget")
            if declared_reuse != canonical_target:
                raise ValueError(
                    f"Repeated clip reuse points at the wrong product target: "
                    f"{asset_id}/{clip}: {declared_reuse} != {canonical_target}"
                )
            product["productTargetEffectAssetId"] = canonical_target
            product["productAuthoringPath"] = canonical["product"][
                "productAuthoringPath"
            ]
            product["productDocumentFileSha256"] = canonical["product"][
                "productDocumentFileSha256"
            ]
            product["documentPath"] = canonical["product"]["documentPath"]
            product["document"] = canonical["product"]["document"]
            product["reusesProductTarget"] = canonical_target
        evidence.append(
            {
                "animationAssetId": asset_id,
                "clip": clip,
                "occurrenceCount": len(rows),
                "resolution": "sharedEquivalentProductTarget",
                "normalizedRuntimeSha256": next(iter(normalized_hashes)),
                "productTargetEffectAssetId": canonical_target,
                "stageIdentities": [
                    {
                        "characterClass": row["stage"]["characterClass"],
                        "productSkillId": row["stage"]["productSkillId"],
                        "stageIndex": row["stage"]["stageIndex"],
                        "stageClipIndex": row["product"]["stageClipIndex"],
                        "sourceStageTargetEffectAssetId": row["product"][
                            "sourceStageTargetEffectAssetId"
                        ],
                    }
                    for row in visual_rows
                ],
            }
        )
    return evidence


def _parse_animevent_line(line: str, line_number: int) -> tuple[list[str], dict[str, str]]:
    try:
        tokens = shlex.split(line, posix=True)
    except ValueError as error:
        raise ValueError(f"Animevent syntax is invalid at line {line_number}") from error
    attributes: dict[str, str] = {}
    if len(tokens) >= 3:
        for token in tokens[2:]:
            if "=" not in token or token.startswith("="):
                continue
            key, value = token.split("=", 1)
            if key in attributes:
                raise ValueError(f"Animevent has duplicate field at line {line_number}")
            attributes[key] = value
    return tokens, attributes


def _desired_animevent_bytes(
    contract: dict[str, Any],
    stages: list[dict[str, Any]],
) -> tuple[Path, bytes, int]:
    asset_id = contract["animationAssetId"]
    path = (
        DATA_ROOT
        / "Animation/Authored"
        / asset_id
        / f"{asset_id}.animevents"
    )
    lines = path.read_text(encoding="utf-8-sig").splitlines()
    if not lines:
        raise ValueError(f"Animevent document is empty: {asset_id}")
    try:
        header = shlex.split(lines[0], posix=True)
    except ValueError as error:
        raise ValueError(f"Animevent header is invalid: {asset_id}") from error
    if (
        len(header) != 4
        or header[0] != "LOSTARK_ANIM_EVENTS"
        or header[2] != asset_id
    ):
        raise ValueError(f"Animevent identity mismatch: {asset_id}")
    version = int(header[1])
    event_lines = [line for line in lines[1:] if line.strip()]
    if int(header[3]) != len(event_lines):
        raise ValueError(f"Animevent row count mismatch: {asset_id}")
    scope_clips = {
        clip
        for stage in stages
        if stage["animationAssetId"] == asset_id
        for clip in stage["clips"]
    }
    preserved_lines: list[str] = []
    for line_number, line in enumerate(event_lines, start=2):
        tokens, attributes = _parse_animevent_line(line, line_number)
        current_scope_product = (
            len(tokens) >= 3
            and tokens[1] == "EFFECT"
            and tokens[0] in scope_clips
            and attributes.get("effectref") == "asset"
        )
        if not current_scope_product:
            preserved_lines.append(line)

    desired_cues: dict[str, str] = {}
    for stage in stages:
        if stage["animationAssetId"] != asset_id:
            continue
        for product in stage["clipProducts"]:
            if product["status"] != "visualBearing":
                continue
            clip = product["clip"]
            target = product["productTargetEffectAssetId"]
            previous = desired_cues.setdefault(clip, target)
            if previous != target:
                raise ValueError(
                    f"One clip owns multiple product targets: {asset_id}/{clip}: "
                    f"{previous}, {target}"
                )
    cue_lines = [
        (
            f'"{clip}" EFFECT startms=0 payload="{target}" effectref=asset '
            'anchor="root" follow=follow stop=natural '
            'px=0 py=0 pz=0 rx=0 ry=0 rz=0 sx=1 sy=1 sz=1'
        )
        for clip, target in sorted(desired_cues.items())
    ]
    output_events = preserved_lines + cue_lines
    output_lines = [
        f'LOSTARK_ANIM_EVENTS {version} "{asset_id}" {len(output_events)}',
        *output_events,
    ]
    return path, ("\n".join(output_lines) + "\n").encode("utf-8"), len(cue_lines)


def _index_product_targets(
    products: list[dict[str, Any]],
) -> dict[str, dict[str, Any]]:
    """Fail closed when a target or its clip owner is ambiguous."""
    product_target_stages: dict[str, dict[str, Any]] = {}
    target_cue_owners: dict[str, tuple[str, str]] = {}
    for product in products:
        target = product["productTargetEffectAssetId"]
        if not isinstance(target, str) or not STABLE_ID.fullmatch(target):
            raise ValueError(f"Invalid product target ID: {target}")
        if "reusesProductTarget" not in product:
            previous_product = product_target_stages.setdefault(target, product)
            if previous_product is not product:
                raise ValueError(f"Duplicate canonical product target: {target}")
        owner = (product["animationAssetId"], product["clip"])
        previous_owner = target_cue_owners.setdefault(target, owner)
        if previous_owner != owner:
            raise ValueError(f"Product target has multiple cue owners: {target}")
    return product_target_stages


def _desired_protected_product_gate(
    catalog_bytes: bytes, dimensionmaster_cue_bytes: bytes
) -> bytes:
    document = _load_json(
        PROTECTED_MATERIALIZATION_PATH,
        "protected DimensionMaster materialization",
    )
    if (
        document.get("schema") != "lostark.effect-authored-materialization"
        or document.get("version") != 1
        or document.get("materializationId")
        != "dimensionmaster.skill.2050210.authored-baseline"
        or document.get("characterClass") != "DIMENSIONMASTER"
        or document.get("skillId") != 2050210
        or document.get("status") != "preserveExisting"
    ):
        raise ValueError("Protected DimensionMaster materialization identity drifted")
    stages = document.get("stages")
    if not isinstance(stages, list) or len(stages) != 1:
        raise ValueError("Protected DimensionMaster product gate stage drifted")
    stage = stages[0]
    if (
        not isinstance(stage, dict)
        or stage.get("stageIndex") != 0
        or stage.get("clip") != "pc_sp_m_00_sk_sk_willowrend"
        or stage.get("status") != "preserveExisting"
        or stage.get("targetEffectAssetId") != PROTECTED_TARGET_ID
        or stage.get("targetAuthoringPath")
        != f"Effects/Authored/{PROTECTED_TARGET_ID}.effect.json"
    ):
        raise ValueError("Protected DimensionMaster product gate owner drifted")
    gate = stage.get("productGate")
    if not isinstance(gate, dict):
        raise ValueError("Protected DimensionMaster product gate is missing")
    catalog = gate.get("catalog")
    animation_cue = gate.get("animationCue")
    expected_document_hash = _sha256_file(
        AUTHORED_ROOT / f"{PROTECTED_TARGET_ID}.effect.json"
    )
    expected_kind_counts = {"mesh": 20, "sprite": 4, "particle": 0}
    if (
        gate.get("schema") != "lostark.effect-authored-product-gate"
        or gate.get("version") != 1
        or gate.get("documentSha256") != expected_document_hash
        or gate.get("expectedElementCount") != 24
        or gate.get("expectedOccurrenceCount") != 4
        or gate.get("expectedKindCounts") != expected_kind_counts
        or gate.get("spriteBillboardRollDegrees") != -90.0
        or gate.get("innerAnchorPolicy")
        != {
            "provenance": "AUTHORED_POLICY",
            "runtimeAnchorSlotId": "root",
            "follow": False,
        }
        or not isinstance(catalog, dict)
        or catalog.get("path") != "Effects/EffectCatalog.json"
        or not isinstance(animation_cue, dict)
        or animation_cue.get("path")
        != "Animation/Authored/DimensionMaster/DimensionMaster.animevents"
        or animation_cue.get("clip") != "pc_sp_m_00_sk_sk_willowrend"
        or animation_cue.get("startMilliseconds") != 0
        or animation_cue.get("stop") != "natural"
        or animation_cue.get("outerAnchorPolicy")
        != {
            "provenance": "AUTHORED_POLICY",
            "runtimeAnchorSlotId": "root",
            "follow": True,
        }
    ):
        raise ValueError("Protected DimensionMaster product gate semantics drifted")
    catalog["sha256"] = _sha256_bytes(catalog_bytes)
    animation_cue["sha256"] = _sha256_bytes(dimensionmaster_cue_bytes)
    return _json_bytes(document)


def _clip_product_receipt(product: dict[str, Any]) -> dict[str, Any]:
    row: dict[str, Any] = {
        "clip": product["clip"],
        "stageClipIndex": product["stageClipIndex"],
        "sourceTimelineOffsetSeconds": product["sourceTimelineOffsetSeconds"],
        "sourceLengthSeconds": product["sourceLengthSeconds"],
        "playMs": product["playMs"],
        "playRate": product["playRate"],
        "playableSourceDurationSeconds": product[
            "playableSourceDurationSeconds"
        ],
        "runtimeLoop": product["runtimeLoop"],
        "status": product["status"],
        "sourceEventIds": copy.deepcopy(product["sourceEventIds"]),
        "elementCount": product["elementCount"],
    }
    if product["status"] != "visualBearing":
        return row
    row.update(
        {
            "elementIds": copy.deepcopy(product["elementIds"]),
            "sourceElementsSha256": product["sourceElementsSha256"],
            "sourceElementOwnership": copy.deepcopy(
                product["sourceElementOwnership"]
            ),
            "sourceElementOwnershipSha256": product[
                "sourceElementOwnershipSha256"
            ],
            "sourceStageTargetEffectAssetId": product[
                "sourceStageTargetEffectAssetId"
            ],
            "sourceStageAuthoringPath": product["sourceStageAuthoringPath"],
            "sourceStageDocumentFileSha256": product[
                "sourceStageDocumentFileSha256"
            ],
            "productTargetEffectAssetId": product[
                "productTargetEffectAssetId"
            ],
            "productAuthoringPath": product["productAuthoringPath"],
            "productDocumentFileSha256": product[
                "productDocumentFileSha256"
            ],
            "normalizedRuntimeSha256": product["normalizedRuntimeSha256"],
            "derivedClipProjection": product["derivedClipProjection"],
        }
    )
    if "reusesProductTarget" in product:
        row["reusesProductTarget"] = product["reusesProductTarget"]
    if "approximationReceipt" in product:
        row["approximationReceipt"] = copy.deepcopy(
            product["approximationReceipt"]
        )
    return row


def _product_target_receipt(product: dict[str, Any]) -> dict[str, Any]:
    row = {
        "effectAssetId": product["productTargetEffectAssetId"],
        "authoringPath": product["productAuthoringPath"],
        "documentFileSha256": product["productDocumentFileSha256"],
        "animationAssetId": product["animationAssetId"],
        "characterClass": product["characterClass"],
        "productSkillId": product["productSkillId"],
        "stageIndex": product["stageIndex"],
        "stageClipIndex": product["stageClipIndex"],
        "clip": product["clip"],
        "derivedClipProjection": product["derivedClipProjection"],
        "sourceStageTargetEffectAssetId": product[
            "sourceStageTargetEffectAssetId"
        ],
        "sourceStageAuthoringPath": product["sourceStageAuthoringPath"],
        "sourceStageDocumentFileSha256": product[
            "sourceStageDocumentFileSha256"
        ],
        "sourceElementsSha256": product["sourceElementsSha256"],
        "sourceElementOwnership": copy.deepcopy(
            product["sourceElementOwnership"]
        ),
        "sourceElementOwnershipSha256": product[
            "sourceElementOwnershipSha256"
        ],
        "sourceEventIds": copy.deepcopy(product["sourceEventIds"]),
        "clipBinding": {
            "sourceTimelineOffsetSeconds": product[
                "sourceTimelineOffsetSeconds"
            ],
            "sourceLengthSeconds": product["sourceLengthSeconds"],
            "playMs": product["playMs"],
            "playRate": product["playRate"],
            "playableSourceDurationSeconds": product[
                "playableSourceDurationSeconds"
            ],
            "runtimeLoop": product["runtimeLoop"],
        },
    }
    if "approximationReceipt" in product:
        row["approximationReceipt"] = copy.deepcopy(
            product["approximationReceipt"]
        )
    return row


_MANAGED_CLIP_PROJECTION_NAME = re.compile(
    r"^effect\.(?:artist|dimensionmaster|lancemaster|warlord)\.skill\."
    r"[1-9][0-9]*\.(?:authored-baseline|ba[1-9][0-9]*)\."
    r"clip[1-9][0-9]*\.effect\.json$"
)


def _managed_projection_deletes(
    desired_documents: dict[Path, bytes],
    allow_managed_drift: bool = False,
) -> set[Path]:
    desired_paths = {path.resolve() for path in desired_documents}
    previous_managed: set[Path] = set()
    if ROLLOUT_RECEIPT_PATH.is_file():
        previous = _load_json(ROLLOUT_RECEIPT_PATH, "previous product rollout")
        if previous.get("schema") != ROLLOUT_SCHEMA:
            raise ValueError("Previous product rollout schema drifted")
        version = previous.get("version")
        if version not in {1, ROLLOUT_VERSION}:
            raise ValueError("Previous product rollout version is unsupported")
        if version == ROLLOUT_VERSION:
            product_rows = previous.get("productTargets")
            if not isinstance(product_rows, list):
                raise ValueError("Previous product rollout targets are invalid")
            for row in product_rows:
                if not isinstance(row, dict) or row.get(
                    "derivedClipProjection"
                ) is not True:
                    continue
                path = (DATA_ROOT / str(row.get("authoringPath") or "")).resolve()
                try:
                    path.relative_to(AUTHORED_ROOT.resolve())
                except ValueError as error:
                    raise ValueError(
                        "Previous managed clip projection escaped Authored root"
                    ) from error
                if not _MANAGED_CLIP_PROJECTION_NAME.fullmatch(path.name):
                    raise ValueError(
                        f"Previous managed clip projection has an invalid path: {path}"
                    )
                expected_file_sha = row.get("documentFileSha256")
                if (
                    not isinstance(expected_file_sha, str)
                    or re.fullmatch(r"[0-9a-f]{64}", expected_file_sha) is None
                ):
                    raise ValueError(
                        "Previous managed clip projection has no valid file hash: "
                        f"{path}"
                    )
                if (
                    path.is_file()
                    and _sha256_file(path) != expected_file_sha
                    and not allow_managed_drift
                ):
                    raise ValueError(
                        "Managed Authored clip projection was edited after its "
                        "one-time seed; refusing to overwrite it without "
                        f"--migrate-managed-projections: {path}"
                    )
                previous_managed.add(path)

    discovered = {
        path.resolve()
        for path in AUTHORED_ROOT.glob("*.clip*.effect.json")
        if _MANAGED_CLIP_PROJECTION_NAME.fullmatch(path.name)
    }
    unmanaged = discovered - desired_paths - previous_managed
    if unmanaged:
        raise ValueError(
            "Unmanaged Authored clip projection exists: "
            + ", ".join(_repository_relative(path) for path in sorted(unmanaged))
        )
    return previous_managed - desired_paths


def _build_desired(
    allow_managed_projection_migration: bool = False,
) -> tuple[dict[Path, bytes], set[Path], dict[str, Any]]:
    stages, source_inputs = _load_scope()
    _attach_documents(stages)
    _validate_approximation_coverage(stages)
    source_inputs.append(
        {
            "path": _repository_relative(APPROXIMATION_COVERAGE_PATH),
            "sha256": _sha256_file(APPROXIMATION_COVERAGE_PATH),
        }
    )
    effect_bearing = [stage for stage in stages if not stage["silent"]]
    silent = [stage for stage in stages if stage["silent"]]
    if (
        len(effect_bearing) != EXPECTED_EFFECT_BEARING_STAGE_COUNT
        or len(silent) != EXPECTED_INTENTIONALLY_SILENT_STAGE_COUNT
    ):
        raise ValueError(
            f"Effect-bearing/silent stage count mismatch: "
            f"{len(effect_bearing)}/{len(silent)}"
        )
    for stage in effect_bearing:
        stage.setdefault(
            "productTargetEffectAssetId", stage["nominalTargetEffectAssetId"]
        )
    desired_documents, clip_counts = _derive_clip_products(stages)
    repeated_clip_evidence = _resolve_repeated_clips(stages)

    visual_products = [
        product
        for stage in stages
        for product in stage["clipProducts"]
        if product["status"] == "visualBearing"
    ]
    product_targets = _index_product_targets(visual_products)
    if len(product_targets) != EXPECTED_PRODUCT_TARGET_COUNT:
        raise ValueError(
            f"Unique product target count mismatch: {len(product_targets)}"
        )
    retained_target_count = sum(
        product["derivedClipProjection"] is False
        for product in product_targets.values()
    )
    derived_target_count = sum(
        product["derivedClipProjection"] is True
        for product in product_targets.values()
    )
    if (
        retained_target_count != EXPECTED_RETAINED_STAGE_TARGET_COUNT
        or derived_target_count != EXPECTED_DERIVED_CLIP_TARGET_COUNT
    ):
        raise ValueError(
            "Retained/derived target count mismatch: "
            f"{retained_target_count}/{derived_target_count}"
        )

    catalog_entries = []
    for target, product in sorted(product_targets.items()):
        document_path = product["documentPath"]
        document = product["document"]
        if document.get("effectAssetId") != target:
            raise ValueError(f"Canonical product document ID mismatch: {target}")
        catalog_entries.append(
            {
                "effectAssetId": target,
                "authoringPath": _data_relative(document_path),
            }
        )
    catalog = {"formatVersion": 1, "effects": catalog_entries}
    catalog_bytes = _json_bytes(catalog)

    cue_count = 0
    desired_files: dict[Path, bytes] = dict(desired_documents)
    desired_files[CATALOG_PATH] = catalog_bytes
    dimensionmaster_cue_path: Path | None = None
    for contract in CLASS_CONTRACTS:
        path, content, class_cue_count = _desired_animevent_bytes(
            contract, stages
        )
        desired_files[path] = content
        cue_count += class_cue_count
        if contract["animationAssetId"] == "DimensionMaster":
            dimensionmaster_cue_path = path
    if cue_count != EXPECTED_PRODUCT_CUE_COUNT:
        raise ValueError(f"Product cue count mismatch: {cue_count}")
    if dimensionmaster_cue_path is None:
        raise ValueError("DimensionMaster product cue output is missing")
    product_gate_bytes = _desired_protected_product_gate(
        catalog_bytes, desired_files[dimensionmaster_cue_path]
    )
    desired_files[PROTECTED_MATERIALIZATION_PATH] = product_gate_bytes

    stage_rows: list[dict[str, Any]] = []
    for stage in stages:
        row: dict[str, Any] = {
            "animationAssetId": stage["animationAssetId"],
            "characterClass": stage["characterClass"],
            "productSkillId": stage["productSkillId"],
            "stageIndex": stage["stageIndex"],
            "stageId": stage["stageId"],
            "boundClips": stage["clips"],
            "nominalTargetEffectAssetId": stage[
                "nominalTargetEffectAssetId"
            ],
            "sourceEventIds": copy.deepcopy(stage["sourceEventIds"]),
            "clipProducts": [
                _clip_product_receipt(product)
                for product in stage["clipProducts"]
            ],
        }
        if stage["silent"]:
            row.update(
                {
                    "status": "sourceIntentionallySilent",
                }
            )
        else:
            row.update(
                {
                    "status": "effectBearing",
                    "sourceStatus": stage["sourceStatus"],
                    "sourceStageTargetEffectAssetId": stage[
                        "productTargetEffectAssetId"
                    ],
                    "sourceStageAuthoringPath": _data_relative(
                        stage["documentPath"]
                    ),
                    "sourceStageDocumentFileSha256": stage[
                        "documentFileSha256"
                    ],
                    "sourceStageNormalizedRuntimeSha256": stage[
                        "normalizedRuntimeSha256"
                    ],
                }
            )
            row["nominalAuthoringPath"] = stage.get(
                "nominalAuthoringPath", _data_relative(stage["documentPath"])
            )
            if "nominalDocumentCanonicalSha256" in stage:
                row["nominalDocumentCanonicalSha256"] = stage[
                    "nominalDocumentCanonicalSha256"
                ]
            else:
                row["nominalDocumentFileSha256"] = stage["documentFileSha256"]
            if "reusesProductTarget" in stage:
                row["reusesProductTarget"] = stage["reusesProductTarget"]
            if "approximationReceiptPath" in stage:
                row["approximationReceipt"] = {
                    "path": _repository_relative(stage["approximationReceiptPath"]),
                    "sha256": stage["approximationReceiptSha256"],
                }
        stage_rows.append(row)

    receipt = {
        "schema": ROLLOUT_SCHEMA,
        "version": ROLLOUT_VERSION,
        "sourceInputs": sorted(source_inputs, key=lambda row: row["path"]),
        "summary": {
            "skillCount": EXPECTED_SKILL_COUNT,
            "stageCount": len(stages),
            "clipOccurrenceCount": sum(len(stage["clips"]) for stage in stages),
            "effectBearingStageCount": len(effect_bearing),
            "sourceIntentionallySilentStageCount": len(silent),
            "blockedStageCount": 0,
            "generatedApproximationStageCount": sum(
                stage.get("sourceStatus") == "generatedApproximation"
                for stage in effect_bearing
            ),
            "preservedExistingStageCount": sum(
                stage.get("sourceStatus") == "preservedExisting"
                for stage in effect_bearing
            ),
            "reusedProductTargetStageCount": sum(
                "reusesProductTarget" in stage for stage in effect_bearing
            ),
            **clip_counts,
            "derivedClipTargetCount": derived_target_count,
            "retainedStageTargetCount": retained_target_count,
            "productTargetCount": len(product_targets),
            "productCueCount": cue_count,
        },
        "repeatedClipEvidence": repeated_clip_evidence,
        "protectedProductGate": {
            "path": _repository_relative(PROTECTED_MATERIALIZATION_PATH),
            "sha256": _sha256_bytes(product_gate_bytes),
            "catalogSha256": _sha256_bytes(catalog_bytes),
            "animationCueSha256": _sha256_bytes(
                desired_files[dimensionmaster_cue_path]
            ),
        },
        "stages": stage_rows,
        "productTargets": [
            _product_target_receipt(product)
            for _, product in sorted(product_targets.items())
        ],
    }
    desired_files[ROLLOUT_RECEIPT_PATH] = _json_bytes(receipt)
    delete_paths = _managed_projection_deletes(
        desired_documents,
        allow_managed_drift=allow_managed_projection_migration,
    )
    return desired_files, delete_paths, receipt


def _write_transactionally(
    desired_files: dict[Path, bytes], delete_paths: set[Path] | None = None
) -> None:
    delete_paths = set() if delete_paths is None else set(delete_paths)
    overlap = set(desired_files).intersection(delete_paths)
    if overlap:
        raise ValueError(f"A transactional path cannot be written and deleted: {overlap}")
    temporary_paths: list[tuple[Path, Path]] = []
    backups: list[tuple[Path, Path]] = []
    committed: list[Path] = []
    try:
        for target, content in sorted(
            desired_files.items(), key=lambda item: str(item[0])
        ):
            target.parent.mkdir(parents=True, exist_ok=True)
            descriptor, temporary_name = tempfile.mkstemp(
                prefix=f".{target.name}.", suffix=".tmp", dir=target.parent
            )
            temporary = Path(temporary_name)
            with os.fdopen(descriptor, "wb") as stream:
                stream.write(content)
                stream.flush()
                os.fsync(stream.fileno())
            temporary_paths.append((temporary, target))
        affected_targets = sorted(
            set(desired_files).union(delete_paths), key=str
        )
        for target in affected_targets:
            if target.exists():
                descriptor, backup_name = tempfile.mkstemp(
                    prefix=f".{target.name}.", suffix=".bak", dir=target.parent
                )
                os.close(descriptor)
                backup = Path(backup_name)
                backup.unlink()
                os.replace(target, backup)
                backups.append((backup, target))
        for temporary, target in temporary_paths:
            os.replace(temporary, target)
            committed.append(target)
    except BaseException:
        for target in reversed(committed):
            if target.exists():
                target.unlink()
        for backup, target in reversed(backups):
            if backup.exists():
                os.replace(backup, target)
        raise
    finally:
        for temporary, _ in temporary_paths:
            if temporary.exists():
                temporary.unlink()
        for backup, _ in backups:
            if backup.exists():
                backup.unlink()


def _check_desired(
    desired_files: dict[Path, bytes], delete_paths: set[Path] | None = None
) -> None:
    delete_paths = set() if delete_paths is None else set(delete_paths)
    mismatches = []
    for path, expected in desired_files.items():
        actual = path.read_bytes() if path.is_file() else None
        if actual != expected:
            mismatches.append(_repository_relative(path))
    for path in sorted(delete_paths):
        if path.exists():
            mismatches.append(_repository_relative(path))
    if mismatches:
        raise ValueError(
            "Four-class product source publish is stale: " + ", ".join(mismatches)
        )


def _check_runtime(receipt: dict[str, Any]) -> dict[str, int]:
    def require_runtime_display_name(value: Any, context: str) -> None:
        if (
            not isinstance(value, str)
            or not value.strip()
            or len(value.encode("utf-8")) > 64
        ):
            raise ValueError(
                f"Runtime {context} displayName must be 1-64 UTF-8 bytes"
            )

    runtime = _load_json(RUNTIME_CATALOG_PATH, "runtime Effect catalog")
    if runtime.get("formatVersion") != 2:
        raise ValueError("Runtime Effect catalog formatVersion must be 2")
    expected_ids = {
        row["effectAssetId"] for row in receipt.get("productTargets", [])
    }
    runtime_effects = runtime.get("effects")
    runtime_components = runtime.get("components")
    if not isinstance(runtime_effects, list) or not isinstance(runtime_components, list):
        raise ValueError("Runtime Effect catalog arrays are invalid")
    actual_ids = {
        str(row.get("effectAssetId") or "")
        for row in runtime_effects
        if isinstance(row, dict)
    }
    if len(actual_ids) != len(runtime_effects) or actual_ids != expected_ids:
        raise ValueError(
            "Runtime Effect target coverage mismatch: "
            f"missing={sorted(expected_ids - actual_ids)}, "
            f"extra={sorted(actual_ids - expected_ids)}"
        )
    expected_by_id = {
        row["effectAssetId"]: row for row in receipt["productTargets"]
    }
    for row in runtime_effects:
        effect_id = row["effectAssetId"]
        expected = expected_by_id[effect_id]
        if row.get("contentSha256") != expected["documentFileSha256"]:
            raise ValueError(f"Runtime Effect content hash drifted: {effect_id}")
        assembly = row.get("assembly")
        if not isinstance(assembly, dict) or assembly.get("effectAssetId") != effect_id:
            raise ValueError(f"Runtime Effect assembly identity drifted: {effect_id}")
        require_runtime_display_name(
            assembly.get("displayName"), f"Effect Assembly {effect_id}"
        )
    for component in runtime_components:
        if not isinstance(component, dict):
            raise ValueError("Runtime Effect component is invalid")
        source = component.get("source")
        source_id = source.get("effectAssetId") if isinstance(source, dict) else None
        if source_id not in expected_ids:
            raise ValueError(f"Runtime component escaped product targets: {source_id}")
        component_id = str(component.get("componentAssetId") or "")
        require_runtime_display_name(
            component.get("displayName"), f"Effect Component {component_id}"
        )
        document = component.get("document")
        if not isinstance(document, dict):
            raise ValueError(f"Runtime component Document is invalid: {component_id}")
        require_runtime_display_name(
            document.get("displayName"),
            f"Effect Component Document {component_id}",
        )
    return {
        "runtimeEffectCount": len(runtime_effects),
        "runtimeComponentCount": len(runtime_components),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    parser.add_argument(
        "--check-runtime",
        action="store_true",
        help="Also require the generated runtime catalog to match the 101 product targets.",
    )
    parser.add_argument(
        "--migrate-managed-projections",
        action="store_true",
        help=(
            "Explicitly replace edited generated .clipN seed documents. "
            "Normal writes fail closed so F1-authored Product documents are preserved."
        ),
    )
    args = parser.parse_args()
    if args.migrate_managed_projections and not args.write:
        parser.error("--migrate-managed-projections requires --write")

    desired_files, delete_paths, receipt = _build_desired(
        allow_managed_projection_migration=args.migrate_managed_projections,
    )
    if args.write:
        _write_transactionally(desired_files, delete_paths)
        _check_desired(desired_files, delete_paths)
    else:
        _check_desired(desired_files, delete_paths)
    result = copy.deepcopy(receipt["summary"])
    if args.check_runtime:
        result.update(_check_runtime(receipt))
    print(json.dumps(result, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
