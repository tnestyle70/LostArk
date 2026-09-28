#!/usr/bin/env python3
"""Fail-closed Valtan hit, V2 attack-effect, and impact-sound alignment.

This validator joins the split gameplay/presentation authoring clocks.  Clip
events use source time, so their stage-wall time is calculated as::

    occurrence_wall_start + (event_source_ms - source_start_ms) / play_rate

It intentionally does not infer gameplay from visuals.  Gameplay hits and
grab-damage actions remain authoritative; attack-role V2 bindings and impact
sounds must resolve back to those authored contracts.
"""

from __future__ import annotations

import argparse
import json
import math
import re
import sys
from collections import defaultdict
from pathlib import Path
from typing import Any

if __package__:
    from . import validate_valtan_clip_template_parity as clip_parity
else:
    import validate_valtan_clip_template_parity as clip_parity


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
ROLE_LEDGER_PATH = Path("Data/Effects/V2/EffectRoles.json")
ALLOWLIST_PATH = Path("Data/Valtan/Valtan.hitalignment-allowlist.json")
GAMEPLAY_PATH = Path("Data/Valtan/Valtan.gameplay.json")
PRESENTATION_PATH = Path("Data/Valtan/Valtan.presentation.json")
V2_BINDINGS_PATH = Path("Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json")
SOUND_CUES_PATH = Path("Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json")
COMBAT_OBJECTS_PATH = Path("Data/Valtan/Valtan.combatobjects.json")
COMBAT_OBJECT_SOUND_CUES_PATH = Path(
    "Data/Animation/Authored/Valtan/Valtan.combatobjectsoundcues.json"
)
BOSS_CATALOG_PATH = Path("Data/Actors/BossCatalog.json")
CLIP_TEMPLATES_PATH = Path("Data/Valtan/Valtan.cliptemplates.json")

TICK_TOLERANCE_MS = 33.0
STABLE_ID = re.compile(r"^[A-Za-z0-9_.-]{1,240}$")
ROLE_ROOT_FIELDS = {"schema", "formatVersion", "ownerArchetypeId", "resources"}
ROLE_FIELDS = {"kind", "id", "role", "alignmentPolicy"}
ALLOWLIST_ROOT_FIELDS = {
    "schema", "formatVersion", "ownerArchetypeId", "exceptions",
}
ALLOWLIST_FIELDS = {
    "exceptionId", "rule", "patternId", "stageId", "actionId",
    "bindingId", "expectedHitOffsetsMs", "reason",
}
ALLOWED_EXCEPTION_RULES = {
    "EXTERNAL_V2_BINDING_SCOPE",
    "STAGE_HIT_SOUND_TRACK",
    "PROJECT_AUTHORED_PRESENTATION_ONLY",
    "PROJECT_AUTHORED_SOUND_TIMING",
    "COMBAT_OBJECT_HIT_PATTERN_SOUND",
}
ATTACK_POLICIES = {
    "BINDING_START",
    "CLIP_TEMPLATE",
    "CLIP_TEMPLATE_OR_BINDING_START",
    "STAGE_DAMAGE_ACTION",
    "COMBAT_OBJECT_HIT",
}
NON_ATTACK_ROLES = {"TELEGRAPH", "STATE"}
DOWNSTREAM_DAMAGE_ACTIONS = {
    "DAMAGE_GRABBED_PLAYERS",
    "EXECUTE_GRABBED_PLAYERS",
}


class ContractError(ValueError):
    """Raised when the authored hit/presentation contract does not close."""


def _load(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ContractError(f"cannot read JSON {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise ContractError(f"JSON root must be an object: {path}")
    return value


def _exact_fields(value: Any, fields: set[str], context: str) -> dict[str, Any]:
    if not isinstance(value, dict) or set(value) != fields:
        actual = sorted(value) if isinstance(value, dict) else type(value).__name__
        raise ContractError(f"{context} fields must be {sorted(fields)}, got {actual}")
    return value


def _stable(value: Any, context: str) -> str:
    if not isinstance(value, str) or not STABLE_ID.fullmatch(value):
        raise ContractError(f"{context} must be a stable ID")
    return value


def _integer(value: Any, context: str, *, minimum: int = 0) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < minimum:
        raise ContractError(f"{context} must be an integer >= {minimum}")
    return value


def _positive_number(value: Any, context: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ContractError(f"{context} must be a positive finite number")
    result = float(value)
    if not math.isfinite(result) or result <= 0.0:
        raise ContractError(f"{context} must be a positive finite number")
    return result


def _validate_role_ledger(document: dict[str, Any]) -> dict[tuple[str, str], dict[str, Any]]:
    _exact_fields(document, ROLE_ROOT_FIELDS, "effect role ledger root")
    if document["schema"] != "lostark.effect-v2-role-ledger":
        raise ContractError("effect role ledger schema is invalid")
    if document["formatVersion"] != 1 or document["ownerArchetypeId"] != "BOSS_VALTAN":
        raise ContractError("effect role ledger identity/version is invalid")
    if not isinstance(document["resources"], list) or not document["resources"]:
        raise ContractError("effect role ledger resources must be a non-empty array")

    roles: dict[tuple[str, str], dict[str, Any]] = {}
    for index, row in enumerate(document["resources"]):
        context = f"effect role ledger resources[{index}]"
        _exact_fields(row, ROLE_FIELDS, context)
        kind = row["kind"]
        if kind not in {"GROUP", "LEAF"}:
            raise ContractError(f"{context}.kind is invalid")
        resource_id = _stable(row["id"], f"{context}.id")
        role = row["role"]
        policy = row["alignmentPolicy"]
        if role == "ATTACK":
            if policy not in ATTACK_POLICIES:
                raise ContractError(f"{context} ATTACK policy is invalid: {policy!r}")
        elif role in NON_ATTACK_ROLES:
            if policy != "NONE":
                raise ContractError(f"{context} {role} resource must use NONE policy")
        else:
            raise ContractError(f"{context}.role is invalid: {role!r}")
        key = (kind, resource_id)
        if key in roles:
            raise ContractError(f"duplicate effect role resource: {key}")
        roles[key] = row
    return roles


def _validate_allowlist(document: dict[str, Any]) -> tuple[
        dict[str, dict[str, Any]],
        dict[tuple[str, str, str], dict[str, Any]],
        dict[str, dict[str, Any]],
        dict[tuple[str, str, str], dict[str, Any]],
        dict[tuple[str, str], dict[str, Any]],
]:
    _exact_fields(document, ALLOWLIST_ROOT_FIELDS, "hit alignment allowlist root")
    if document["schema"] != "lostark.valtan-hit-presentation-alignment-allowlist":
        raise ContractError("hit alignment allowlist schema is invalid")
    if document["formatVersion"] != 1 or document["ownerArchetypeId"] != "BOSS_VALTAN":
        raise ContractError("hit alignment allowlist identity/version is invalid")
    if not isinstance(document["exceptions"], list):
        raise ContractError("hit alignment allowlist exceptions must be an array")

    by_binding: dict[str, dict[str, Any]] = {}
    by_stage: dict[tuple[str, str, str], dict[str, Any]] = {}
    presentation_only: dict[str, dict[str, Any]] = {}
    authored_sounds: dict[tuple[str, str, str], dict[str, Any]] = {}
    combat_pattern_sounds: dict[tuple[str, str], dict[str, Any]] = {}
    exception_ids: set[str] = set()
    for index, row in enumerate(document["exceptions"]):
        context = f"hit alignment allowlist exceptions[{index}]"
        rule = row.get("rule") if isinstance(row, dict) else None
        if not isinstance(rule, str) or rule not in ALLOWED_EXCEPTION_RULES:
            raise ContractError(f"{context}.rule is invalid: {rule!r}")
        extra_fields: set[str] = set()
        if rule == "PROJECT_AUTHORED_PRESENTATION_ONLY":
            extra_fields = {"resource", "clock", "mappingBasis"}
        elif rule == "PROJECT_AUTHORED_SOUND_TIMING":
            extra_fields = {"expectedSoundCues", "expectedAnimation"}
            if "expectedSoundStage" in row:
                extra_fields.add("expectedSoundStage")
        elif rule == "COMBAT_OBJECT_HIT_PATTERN_SOUND":
            extra_fields = {"combatObjectArchetypeId", "hitId", "spawnEventId",
                            "expectedSoundCue", "expectedAnimation"}
        _exact_fields(row, ALLOWLIST_FIELDS | extra_fields, context)
        exception_id = _stable(row["exceptionId"], f"{context}.exceptionId")
        if exception_id in exception_ids:
            raise ContractError(f"duplicate allowlist exceptionId: {exception_id}")
        exception_ids.add(exception_id)
        scope = tuple(_stable(row[field], f"{context}.{field}") for field in (
            "patternId", "stageId", "actionId"
        ))
        offsets = row["expectedHitOffsetsMs"]
        if (not isinstance(offsets, list) or
                any(isinstance(value, bool) or not isinstance(value, int) or value < 0
                    for value in offsets) or
                offsets != sorted(set(offsets))):
            raise ContractError(f"{context}.expectedHitOffsetsMs must be sorted unique integers")
        reason = row["reason"]
        if not isinstance(reason, str) or len(reason.strip()) < 20:
            raise ContractError(f"{context}.reason must explain the exception")

        binding_id = row["bindingId"]
        if rule == "COMBAT_OBJECT_HIT_PATTERN_SOUND":
            _stable(binding_id, f"{context}.bindingId")
            key = tuple(_stable(row[field], f"{context}.{field}") for field in
                        ("combatObjectArchetypeId", "hitId"))
            _stable(row["spawnEventId"], f"{context}.spawnEventId")
            if key in combat_pattern_sounds:
                raise ContractError(f"duplicate combat-object pattern-sound alias: {key}")
            if (len(offsets) != 1 or not isinstance(row["expectedSoundCue"], dict) or
                    not isinstance(row["expectedAnimation"], dict) or
                    row["expectedSoundCue"].get("bindingId") != binding_id):
                raise ContractError(f"{context} needs one exact hit, sound cue, and animation")
            combat_pattern_sounds[key] = row
            continue
        if rule in {"EXTERNAL_V2_BINDING_SCOPE", "PROJECT_AUTHORED_PRESENTATION_ONLY"}:
            binding_id = _stable(binding_id, f"{context}.bindingId")
            if offsets:
                raise ContractError(f"{context} binding exception cannot contain hit offsets")
            if binding_id in by_binding or binding_id in presentation_only:
                raise ContractError(f"duplicate binding exception: {binding_id}")
            if rule == "EXTERNAL_V2_BINDING_SCOPE":
                by_binding[binding_id] = row
                continue
            resource = _exact_fields(row["resource"], {"kind", "id"}, f"{context}.resource")
            if resource["kind"] not in {"GROUP", "LEAF"}:
                raise ContractError(f"{context}.resource.kind is invalid")
            _stable(resource["id"], f"{context}.resource.id")
            clock = _exact_fields(
                row["clock"], {"basis", "clipOccurrenceId", "startMs", "repeatPolicy"},
                f"{context}.clock",
            )
            _integer(clock["startMs"], f"{context}.clock.startMs")
            if (clock["basis"] != "STAGE" or clock["clipOccurrenceId"] is not None or
                    clock["repeatPolicy"] != "ONCE"):
                raise ContractError(f"{context} presentation-only clock must be STAGE/ONCE")
            if row["mappingBasis"] != "PROJECT_AUTHORED":
                raise ContractError(f"{context} presentation-only mappingBasis is invalid")
            presentation_only[binding_id] = row
        else:
            if binding_id is not None:
                raise ContractError(f"{context} sound-track exception bindingId must be null")
            if not offsets:
                raise ContractError(f"{context} sound-track exception needs exact hit offsets")
            if scope in by_stage or scope in authored_sounds:
                raise ContractError(f"duplicate sound-track exception scope: {scope}")
            if rule == "STAGE_HIT_SOUND_TRACK":
                by_stage[scope] = row
                continue
            if not isinstance(row["expectedAnimation"], dict):
                raise ContractError(f"{context}.expectedAnimation must be an object")
            cues = row["expectedSoundCues"]
            if not isinstance(cues, list) or not cues or any(not isinstance(cue, dict) for cue in cues):
                raise ContractError(f"{context}.expectedSoundCues must be a non-empty object array")
            cue_ids = [_stable(cue.get("bindingId"), f"{context}.expectedSoundCues.bindingId")
                       for cue in cues]
            if len(cue_ids) != len(set(cue_ids)):
                raise ContractError(f"{context}.expectedSoundCues has duplicate binding IDs")
            if "expectedSoundStage" in row:
                source = _exact_fields(row["expectedSoundStage"],
                    {"stageId", "actionId", "expectedAnimation"}, f"{context}.expectedSoundStage")
                _stable(source["stageId"], f"{context}.expectedSoundStage.stageId")
                _stable(source["actionId"], f"{context}.expectedSoundStage.actionId")
                if not isinstance(source["expectedAnimation"], dict):
                    raise ContractError(f"{context} preceding animation must be an object")
            authored_sounds[scope] = row
    return by_binding, by_stage, presentation_only, authored_sounds, combat_pattern_sounds


def _validate_presentation_only_binding(
        binding: dict[str, Any], exception: dict[str, Any],
        stage_info: dict[str, Any],
        occurrences: dict[tuple[str, str, str, str], dict[str, Any]],
) -> None:
    scope = tuple(binding["scope"][field] for field in ("patternId", "stageId", "actionId"))
    expected_scope = tuple(exception[field] for field in ("patternId", "stageId", "actionId"))
    if (scope != expected_scope or binding["resource"] != exception["resource"] or
            binding["clock"] != exception["clock"]):
        raise ContractError(f"presentation-only binding receipt drift: {binding['bindingId']}")
    if stage_info["hitOffsetsMs"] or any(
            event.get("kind") in DOWNSTREAM_DAMAGE_ACTIONS
            for event in stage_info["stage"].get("events", []) if isinstance(event, dict)):
        raise ContractError(f"presentation-only binding stage now has damage: {binding['bindingId']}")
    stage_occurrences = [row for key, row in occurrences.items() if key[:3] == scope]
    if not stage_occurrences or any(
            row["row"].get("mappingBasis") != exception["mappingBasis"] for row in stage_occurrences):
        raise ContractError(f"presentation-only binding needs PROJECT_AUTHORED clips: {binding['bindingId']}")
    if binding["clock"]["startMs"] >= stage_info["stage"]["durationMs"]:
        raise ContractError(f"presentation-only binding exceeds stage duration: {binding['bindingId']}")


def _stage_hit_offsets(stage: dict[str, Any], context: str) -> list[int]:
    hit = stage.get("hit")
    if not isinstance(hit, dict):
        raise ContractError(f"{context}.hit must be an object")
    shape = hit.get("shape")
    if not isinstance(shape, dict) or not isinstance(shape.get("kind"), str):
        raise ContractError(f"{context}.hit.shape is invalid")
    if shape["kind"] == "NONE":
        if "schedule" in hit or "activation" in hit:
            raise ContractError(f"{context} NONE hit cannot have a schedule/activation")
        return []
    if ("schedule" in hit) == ("activation" in hit):
        raise ContractError(f"{context} damaging hit needs exactly one schedule or activation")

    if "schedule" in hit:
        schedule = hit["schedule"]
        if not isinstance(schedule, dict):
            raise ContractError(f"{context}.hit.schedule must be an object")
        kind = schedule.get("kind")
        if kind == "INTERVAL":
            if set(schedule) != {"kind", "count", "firstOffsetMs", "intervalMs"}:
                raise ContractError(f"{context}.hit.schedule INTERVAL fields are invalid")
            count = _integer(schedule["count"], f"{context}.hit.schedule.count", minimum=1)
            first = _integer(schedule["firstOffsetMs"], f"{context}.hit.schedule.firstOffsetMs")
            interval = _integer(schedule["intervalMs"], f"{context}.hit.schedule.intervalMs")
            if count > 1 and interval == 0:
                raise ContractError(f"{context}.hit.schedule repeated interval must be positive")
            offsets = [first + interval * index for index in range(count)]
        elif kind == "EXPLICIT_OFFSETS":
            if set(schedule) != {"kind", "offsetsMs"}:
                raise ContractError(f"{context}.hit.schedule EXPLICIT_OFFSETS fields are invalid")
            offsets = schedule["offsetsMs"]
            if (not isinstance(offsets, list) or not offsets or
                    any(isinstance(value, bool) or not isinstance(value, int) or value < 0
                        for value in offsets) or
                    offsets != sorted(set(offsets))):
                raise ContractError(f"{context}.hit.schedule offsets are invalid")
            offsets = list(offsets)
        else:
            raise ContractError(f"{context}.hit.schedule kind is unsupported: {kind!r}")
    else:
        activation = hit["activation"]
        if (not isinstance(activation, dict) or activation.get("kind") != "ACTIVE_WINDOW" or
                set(activation) != {"kind", "startMs", "lifetimeMs", "perTargetPolicy"}):
            raise ContractError(f"{context}.hit.activation is unsupported or incomplete")
        offsets = [_integer(activation["startMs"], f"{context}.hit.activation.startMs")]
        _integer(activation["lifetimeMs"], f"{context}.hit.activation.lifetimeMs", minimum=1)
    duration = _integer(stage.get("durationMs"), f"{context}.durationMs", minimum=1)
    if any(offset >= duration for offset in offsets):
        raise ContractError(f"{context}.hit offsets must lie inside the stage")
    return offsets


def _build_source_indexes(gameplay: dict[str, Any], presentation: dict[str, Any]) -> tuple[
        dict[tuple[str, str, str], dict[str, Any]],
        dict[tuple[str, str, str], int],
        dict[tuple[str, str, str, str], dict[str, Any]],
]:
    if (gameplay.get("schema") != "lostark.valtan-gameplay-authoring" or
            gameplay.get("formatVersion") != 1 or
            gameplay.get("bossArchetypeId") != "BOSS_VALTAN"):
        raise ContractError("gameplay split source identity/version is invalid")
    if (presentation.get("schema") != "lostark.valtan-pattern-presentation-authoring" or
            presentation.get("formatVersion") != 1 or
            presentation.get("bossArchetypeId") != "BOSS_VALTAN"):
        raise ContractError("presentation split source identity/version is invalid")

    stages: dict[tuple[str, str, str], dict[str, Any]] = {}
    stage_ordinals: dict[tuple[str, str, str], int] = {}
    patterns = gameplay.get("patterns")
    if not isinstance(patterns, list):
        raise ContractError("gameplay patterns must be an array")
    for pattern_index, pattern in enumerate(patterns):
        pattern_id = _stable(pattern.get("patternId"), f"gameplay patterns[{pattern_index}].patternId")
        if not isinstance(pattern.get("stages"), list):
            raise ContractError(f"gameplay pattern {pattern_id} stages must be an array")
        for stage_index, stage in enumerate(pattern["stages"]):
            stage_id = _stable(stage.get("stageId"), f"{pattern_id} stages[{stage_index}].stageId")
            action_id = _stable(stage.get("actionId"), f"{pattern_id}.{stage_id}.actionId")
            key = (pattern_id, stage_id, action_id)
            if key in stages:
                raise ContractError(f"duplicate gameplay stage scope: {key}")
            offsets = _stage_hit_offsets(stage, ".".join(key))
            stages[key] = {"stage": stage, "hitOffsetsMs": offsets}
            stage_ordinals[key] = stage_index

    presentation_keys: set[tuple[str, str, str]] = set()
    occurrences: dict[tuple[str, str, str, str], dict[str, Any]] = {}
    global_occurrence_ids: set[str] = set()
    presentation_patterns = presentation.get("patterns")
    if not isinstance(presentation_patterns, list):
        raise ContractError("presentation patterns must be an array")
    for pattern_index, pattern in enumerate(presentation_patterns):
        pattern_id = _stable(
            pattern.get("patternId"), f"presentation patterns[{pattern_index}].patternId"
        )
        if not isinstance(pattern.get("stages"), list):
            raise ContractError(f"presentation pattern {pattern_id} stages must be an array")
        for stage_index, stage in enumerate(pattern["stages"]):
            stage_id = _stable(stage.get("stageId"), f"{pattern_id} presentation stageId")
            action_id = _stable(stage.get("actionId"), f"{pattern_id}.{stage_id} presentation actionId")
            stage_key = (pattern_id, stage_id, action_id)
            if stage_key in presentation_keys:
                raise ContractError(f"duplicate presentation stage scope: {stage_key}")
            presentation_keys.add(stage_key)
            animation = stage.get("animation")
            if not isinstance(animation, dict):
                raise ContractError(f"presentation stage animation is invalid: {stage_key}")
            if stage_key in stages:
                stages[stage_key]["animation"] = animation
            occurrence_rows = animation.get("occurrences", [])
            if animation.get("mode") == "NONE":
                occurrence_rows = []
            if not isinstance(occurrence_rows, list):
                raise ContractError(f"presentation occurrences must be an array: {stage_key}")
            wall_start = 0.0
            for occurrence_index, occurrence in enumerate(occurrence_rows):
                context = f"{'.'.join(stage_key)} occurrences[{occurrence_index}]"
                occurrence_id = _stable(occurrence.get("clipOccurrenceId"), f"{context}.clipOccurrenceId")
                if occurrence_id in global_occurrence_ids:
                    raise ContractError(f"duplicate clipOccurrenceId: {occurrence_id}")
                global_occurrence_ids.add(occurrence_id)
                _stable(occurrence.get("clip"), f"{context}.clip")
                source_start = _integer(occurrence.get("sourceStartMs"), f"{context}.sourceStartMs")
                play_ms = _integer(occurrence.get("playMs"), f"{context}.playMs")
                play_rate = _positive_number(occurrence.get("playRate"), f"{context}.playRate")
                key = (*stage_key, occurrence_id)
                occurrences[key] = {
                    "row": occurrence,
                    "wallStartMs": wall_start,
                    "sourceStartMs": source_start,
                    "sourceEndMs": source_start + play_ms,
                    "playRate": play_rate,
                }
                if play_ms:
                    wall_start += play_ms / play_rate

    if set(stages) != presentation_keys:
        missing = sorted(set(stages) - presentation_keys)
        extra = sorted(presentation_keys - set(stages))
        raise ContractError(
            f"gameplay/presentation stage scope drift; missing={missing[:3]}, extra={extra[:3]}"
        )
    return stages, stage_ordinals, occurrences


def _event_wall_ms(occurrence: dict[str, Any], source_ms: Any, context: str) -> float:
    source_ms = _integer(source_ms, context)
    source_start = occurrence["sourceStartMs"]
    source_end = occurrence["sourceEndMs"]
    if source_ms < source_start:
        raise ContractError(f"{context} precedes occurrence sourceStartMs")
    if source_end > source_start and source_ms >= source_end:
        raise ContractError(f"{context} lies outside the occurrence source segment")
    return occurrence["wallStartMs"] + (source_ms - source_start) / occurrence["playRate"]


def _validate_bindings(document: dict[str, Any]) -> tuple[
        list[dict[str, Any]], set[tuple[str, str]]
]:
    if set(document) != {"schema", "formatVersion", "archetypeId", "bindings"}:
        raise ContractError("V2 binding root fields are invalid")
    if (document["schema"] != "lostark.effect-v2-bindings" or
            document["formatVersion"] != 2 or document["archetypeId"] != "BOSS_VALTAN"):
        raise ContractError("V2 binding identity/version is invalid")
    if not isinstance(document["bindings"], list):
        raise ContractError("V2 bindings must be an array")
    ids: set[str] = set()
    resources: set[tuple[str, str]] = set()
    for index, binding in enumerate(document["bindings"]):
        context = f"V2 bindings[{index}]"
        _exact_fields(
            binding,
            {"bindingId", "resource", "scope", "clock", "anchor", "stopPolicy"},
            context,
        )
        binding_id = _stable(binding["bindingId"], f"{context}.bindingId")
        if binding_id in ids:
            raise ContractError(f"duplicate V2 bindingId: {binding_id}")
        ids.add(binding_id)
        resource = _exact_fields(binding["resource"], {"kind", "id"}, f"{context}.resource")
        if resource["kind"] not in {"GROUP", "LEAF"}:
            raise ContractError(f"{context}.resource.kind is invalid")
        resources.add((resource["kind"], _stable(resource["id"], f"{context}.resource.id")))
        scope = _exact_fields(
            binding["scope"], {"patternId", "stageId", "actionId"}, f"{context}.scope"
        )
        for field in ("patternId", "stageId", "actionId"):
            _stable(scope[field], f"{context}.scope.{field}")
        clock = _exact_fields(
            binding["clock"],
            {"basis", "clipOccurrenceId", "startMs", "repeatPolicy"},
            f"{context}.clock",
        )
        _integer(clock["startMs"], f"{context}.clock.startMs")
        if clock["basis"] == "STAGE":
            if clock["clipOccurrenceId"] is not None:
                raise ContractError(f"{context} STAGE clock clipOccurrenceId must be null")
        elif clock["basis"] == "CLIP_OCCURRENCE":
            _stable(clock["clipOccurrenceId"], f"{context}.clock.clipOccurrenceId")
        else:
            raise ContractError(f"{context}.clock.basis is invalid")
    return document["bindings"], resources


def _binding_wall_ms(
        binding: dict[str, Any],
        occurrences: dict[tuple[str, str, str, str], dict[str, Any]],
) -> float:
    clock = binding["clock"]
    if clock["basis"] == "STAGE":
        return float(clock["startMs"])
    scope = binding["scope"]
    occurrence_key = (
        scope["patternId"], scope["stageId"], scope["actionId"],
        clock["clipOccurrenceId"],
    )
    occurrence = occurrences.get(occurrence_key)
    if occurrence is None:
        raise ContractError(
            f"V2 binding clip occurrence does not resolve: {binding['bindingId']}"
        )
    return _event_wall_ms(
        occurrence, clock["startMs"], f"{binding['bindingId']}.clock.startMs"
    )


def _template_effect_index(document: dict[str, Any]) -> dict[str, list[dict[str, Any]]]:
    if (document.get("schema") != "lostark.valtan-clip-templates" or
            document.get("formatVersion") != 1 or
            document.get("ownerArchetypeId") != "BOSS_VALTAN"):
        raise ContractError("clip-template identity/version is invalid")
    templates = document.get("templates")
    if not isinstance(templates, list) or not templates:
        raise ContractError("clip templates must be a non-empty array")
    result: dict[str, list[dict[str, Any]]] = {}
    for template_index, template in enumerate(templates):
        clip = _stable(template.get("clip"), f"clip templates[{template_index}].clip")
        if clip in result:
            raise ContractError(f"duplicate clip template: {clip}")
        effects = template.get("effects")
        if not isinstance(effects, list) or not effects:
            raise ContractError(f"clip template {clip} effects must be non-empty")
        checked: list[dict[str, Any]] = []
        for effect_index, effect in enumerate(effects):
            context = f"clip template {clip} effects[{effect_index}]"
            if not isinstance(effect, dict):
                raise ContractError(f"{context} must be an object")
            if effect.get("resourceKind") not in {"GROUP", "LEAF"}:
                raise ContractError(f"{context}.resourceKind is invalid")
            _stable(effect.get("resourceId"), f"{context}.resourceId")
            _integer(effect.get("clipMs"), f"{context}.clipMs")
            checked.append(effect)
        result[clip] = checked
    return result


def _binding_has_template_contract(
        binding: dict[str, Any],
        binding_wall_ms: float,
        occurrences: dict[tuple[str, str, str, str], dict[str, Any]],
        template_effects: dict[str, list[dict[str, Any]]],
) -> bool:
    scope = binding["scope"]
    stage_prefix = (scope["patternId"], scope["stageId"], scope["actionId"])
    candidates = [
        (key, occurrence) for key, occurrence in occurrences.items()
        if key[:3] == stage_prefix and (
            binding["clock"]["basis"] != "CLIP_OCCURRENCE" or
            key[3] == binding["clock"]["clipOccurrenceId"]
        )
    ]
    resource = binding["resource"]
    for _, occurrence in candidates:
        clip = occurrence["row"]["clip"]
        for effect in template_effects.get(clip, []):
            if (effect["resourceKind"] != resource["kind"] or
                    effect["resourceId"] != resource["id"]):
                continue
            try:
                effect_wall = _event_wall_ms(
                    occurrence, effect["clipMs"],
                    f"clip template {clip} effect {effect['resourceId']}",
                )
            except ContractError:
                continue
            if math.isclose(effect_wall, binding_wall_ms, rel_tol=0.0, abs_tol=1e-6):
                if (binding["clock"]["basis"] == "CLIP_OCCURRENCE" and
                        effect["clipMs"] != binding["clock"]["startMs"]):
                    continue
                return True
    return False


def _validated_effect_timing_overrides(clip_templates, template_effects, stages,
                                       occurrences, bindings) -> dict[str, float]:
    """Reuse parity's exact binding proof, retaining the original gameplay clock."""
    result: dict[str, float] = {}
    try:
        clip_parity.validate_template_document(clip_templates)
        for row in clip_templates["allowlist"]:
            override = row.get(clip_parity.EFFECT_TIMING_OVERRIDE_FIELD)
            if override is None:
                continue
            key = tuple(row[field] for field in
                        ("patternId", "stageId", "actionId", "clipOccurrenceId"))
            occurrence = occurrences.get(key)
            if (occurrence is None or key[:3] not in stages or
                    override["templateEffect"] not in
                    template_effects.get(occurrence["row"]["clip"], [])):
                raise ContractError(f"effect timing override owner/template drift: {key}")
            binding_id = override["bindingId"]
            if binding_id in result:
                raise ContractError(f"effect timing override reuses one binding: {binding_id}")
            result[binding_id] = clip_parity.validate_effect_timing_override(
                override, key, occurrence["row"], occurrence["wallStartMs"],
                stages[key[:3]]["stage"], bindings
            )
    except clip_parity.ContractError as exc:
        raise ContractError(str(exc)) from exc
    return result


def _validate_pattern_sounds(
        document: dict[str, Any],
        stages: dict[tuple[str, str, str], dict[str, Any]],
        occurrences: dict[tuple[str, str, str, str], dict[str, Any]],
        clip_templates: dict[str, Any],
) -> dict[tuple[str, str, str], list[dict[str, Any]]]:
    if set(document) != {"schema", "formatVersion", "ownerArchetypeId", "cues"}:
        raise ContractError("pattern sound root fields are invalid")
    if (document["schema"] != "lostark.valtan-pattern-sound-cues" or
            document["formatVersion"] != 1 or document["ownerArchetypeId"] != "BOSS_VALTAN"):
        raise ContractError("pattern sound identity/version is invalid")
    if not isinstance(document["cues"], list):
        raise ContractError("pattern sound cues must be an array")

    template_sound_keys: set[tuple[str, str, int]] = set()
    for template in clip_templates.get("templates", []):
        clip = template.get("clip")
        for sound in template.get("sounds", []):
            if (isinstance(clip, str) and isinstance(sound, dict) and
                    isinstance(sound.get("soundEvent"), str) and
                    isinstance(sound.get("clipMs"), int) and
                    not isinstance(sound.get("clipMs"), bool)):
                template_sound_keys.add((clip, sound["soundEvent"], sound["clipMs"]))

    result: dict[tuple[str, str, str], list[dict[str, Any]]] = defaultdict(list)
    binding_ids: set[str] = set()
    occurrence_ids: set[str] = set()
    cue_fields = {
        "bindingId", "occurrenceId", "patternId", "stageId", "actionId",
        "clipOccurrenceId", "soundBank", "soundEvent", "repeatPolicy", "startMs",
    }
    for index, cue in enumerate(document["cues"]):
        context = f"pattern sound cues[{index}]"
        optional_fields = {"playbackOffsetMs", "playbackDurationMs"}
        present_optional = set(cue) & optional_fields if isinstance(cue, dict) else set()
        _exact_fields(cue, cue_fields | present_optional, context)
        for field, minimum in (("playbackOffsetMs", 0), ("playbackDurationMs", 1)):
            if field in cue:
                value = _integer(cue[field], f"{context}.{field}", minimum=minimum)
                if value > 600000:
                    raise ContractError(f"{context}.{field} must be <= 600000")
        binding_id = _stable(cue["bindingId"], f"{context}.bindingId")
        occurrence_id = _stable(cue["occurrenceId"], f"{context}.occurrenceId")
        if binding_id in binding_ids or occurrence_id in occurrence_ids:
            raise ContractError(f"duplicate pattern sound binding/occurrence ID: {binding_id}")
        binding_ids.add(binding_id)
        occurrence_ids.add(occurrence_id)
        scope = tuple(_stable(cue[field], f"{context}.{field}") for field in (
            "patternId", "stageId", "actionId"
        ))
        clip_occurrence_id = _stable(cue["clipOccurrenceId"], f"{context}.clipOccurrenceId")
        _stable(cue["soundBank"], f"{context}.soundBank")
        sound_event = _stable(cue["soundEvent"], f"{context}.soundEvent")
        start_ms = _integer(cue["startMs"], f"{context}.startMs")
        if scope not in stages:
            continue
        occurrence = occurrences.get((*scope, clip_occurrence_id))
        if occurrence is None:
            raise ContractError(f"pattern sound cue occurrence does not resolve: {binding_id}")
        wall_ms = _event_wall_ms(occurrence, start_ms, f"{binding_id}.startMs")
        clip = occurrence["row"]["clip"]
        is_impact = (
            "_Shot" in sound_event or
            "_ProjExp" in sound_event or
            (clip, sound_event, start_ms) in template_sound_keys
        )
        if is_impact:
            result[scope].append({
                "wallMs": wall_ms,
                "bindingId": binding_id,
                "soundEvent": sound_event,
            })
    return result


def _preceding_exact_stage(scope, stages):
    """Resolve one finite predecessor by authority edges, never document order."""
    pattern = {key: info for key, info in stages.items() if key[0] == scope[0]}
    if scope not in pattern or pattern[scope]["stage"].get("branches"):
        return None
    actions = {key[2]: key for key in pattern}
    if len(actions) != len(pattern):
        return None
    predecessors = [key for key, info in pattern.items()
                    if info["stage"].get("defaultNextActionId") == scope[2]]
    if len(predecessors) != 1 or predecessors[0] == scope:
        return None
    previous = predecessors[0]
    if (pattern[previous]["stage"].get("branches") or
            any(branch.get("nextActionId") in {previous[2], scope[2]}
                for info in pattern.values() for branch in info["stage"].get("branches", []))):
        return None
    visited = set()
    cursor = previous
    while cursor is not None:
        if cursor in visited:
            return None
        visited.add(cursor)
        cursor = actions.get(pattern[cursor]["stage"].get("defaultNextActionId"))
    for key in (previous, scope):
        info = pattern[key]
        animation = info.get("animation", {})
        rows = animation.get("occurrences", [])
        if (animation.get("endPolicy") != "EXACT" or animation.get("repeatCount") != 1 or
                not rows or any(row.get("repeatUntilStageEnd") for row in rows)):
            return None
        duration = sum(row["playMs"] / row["playRate"] for row in rows)
        if not math.isclose(duration, info["stage"]["durationMs"], abs_tol=1e-6):
            return None
    return previous


def _stage_impact_candidates(scope, stages, impact_sounds):
    candidates = list(impact_sounds.get(scope, []))
    previous = _preceding_exact_stage(scope, stages)
    if previous is not None:
        duration = stages[previous]["stage"]["durationMs"]
        candidates.extend(dict(cue, wallMs=cue["wallMs"] - duration)
                          for cue in impact_sounds.get(previous, []))
    return candidates


def _validate_authored_sound_source(receipt, scope, stages, pattern_sounds, impact_sounds):
    offsets = stages[scope]["hitOffsetsMs"]
    if receipt["expectedHitOffsetsMs"] != offsets:
        raise ContractError(f"authored sound exception hit offsets are stale: {receipt['exceptionId']}")
    if (json.dumps(stages[scope]["animation"], sort_keys=True) !=
            json.dumps(receipt["expectedAnimation"], sort_keys=True)):
        raise ContractError(f"authored sound animation receipt drift: {receipt['exceptionId']}")
    sound_scope = scope
    if "expectedSoundStage" in receipt:
        source = receipt["expectedSoundStage"]
        sound_scope = (scope[0], source["stageId"], source["actionId"])
        if (_preceding_exact_stage(scope, stages) != sound_scope or
                any(tuple(cue[field] for field in ("patternId", "stageId", "actionId")) == scope
                    for cue in pattern_sounds["cues"]) or
                json.dumps(stages[sound_scope]["animation"], sort_keys=True) !=
                json.dumps(source["expectedAnimation"], sort_keys=True)):
            raise ContractError(f"authored sound preceding-stage receipt drift: {receipt['exceptionId']}")
    source_cues = [cue for cue in pattern_sounds["cues"] if
                   tuple(cue[field] for field in ("patternId", "stageId", "actionId")) == sound_scope]
    by_id = lambda cue: cue["bindingId"]
    actual_payload = json.dumps(sorted(source_cues, key=by_id), sort_keys=True)
    expected_payload = json.dumps(sorted(receipt["expectedSoundCues"], key=by_id), sort_keys=True)
    if actual_payload != expected_payload:
        raise ContractError(f"authored sound receipt drift: {receipt['exceptionId']}")
    if not impact_sounds.get(sound_scope):
        raise ContractError(f"authored sound timing still needs an impact sound: {receipt['exceptionId']}")


def _validate_combat_pattern_sound_aliases(
        aliases: dict[tuple[str, str], dict[str, Any]],
        combat_objects: dict[str, Any], pattern_sounds: dict[str, Any],
        stages: dict[tuple[str, str, str], dict[str, Any]],
        occurrences: dict[tuple[str, str, str, str], dict[str, Any]],
) -> set[tuple[str, str]]:
    """Prove one existing pattern sound already covers one owned object hit.

    This audit alias never changes runtime playback. It is limited to a single
    static object spawned on ENTER, a single timed pulse in that same stage,
    and the exact saved sound/animation payload; no approximate-time waiver.
    """
    covered: set[tuple[str, str]] = set()
    used_bindings: set[str] = set()
    for key, receipt in aliases.items():
        label = receipt["exceptionId"]
        scope = tuple(receipt[field] for field in ("patternId", "stageId", "actionId"))
        info = stages.get(scope)
        rows = [row for row in combat_objects.get("objects", [])
                if row.get("combatObjectArchetypeId") == key[0]]
        if info is None or len(rows) != 1:
            raise ContractError(f"combat pattern-sound alias owner is missing: {label}")
        if info["stage"].get("branches"):
            raise ContractError(f"combat pattern-sound alias stage has conditional branches: {label}")
        owner = rows[0]
        hits = [hit for hit in owner.get("hits", []) if hit.get("hitId") == key[1]]
        if (owner.get("kind") != "FIXED_AREA" or
                owner.get("movement") != {"kind": "STATIC"} or len(hits) != 1):
            raise ContractError(f"combat pattern-sound alias hit owner drift: {label}")
        hit = hits[0]
        expected_ms = receipt["expectedHitOffsetsMs"][0]
        if (hit.get("trigger") != {"kind": "TIMED", "atMs": expected_ms} or
                hit.get("repeat") != {"count": 1, "intervalMs": 0} or
                expected_ms >= info["stage"]["durationMs"] or
                expected_ms >= owner.get("lifetimeMs", 0)):
            raise ContractError(f"combat pattern-sound alias timed hit drift: {label}")
        # Search all stage owners: a second spawn would need its own sound and
        # cannot silently borrow a cue from this one scope.
        spawns = [(candidate_scope, event) for candidate_scope, candidate in stages.items()
                  for event in candidate["stage"].get("events", [])
                  if event.get("combatObjectArchetypeId") == key[0]]
        expected_spawn = dict(eventId=receipt["spawnEventId"], trigger="ENTER",
                              kind="SPAWN_COMBAT_OBJECT", combatObjectArchetypeId=key[0], count=1)
        if len(spawns) != 1 or spawns[0] != (scope, expected_spawn):
            raise ContractError(f"combat pattern-sound alias spawn owner/clock drift: {label}")
        cues = [cue for cue in pattern_sounds.get("cues", [])
                if cue.get("bindingId") == receipt["bindingId"]]
        if (len(cues) != 1 or json.dumps(cues[0], sort_keys=True) !=
                json.dumps(receipt["expectedSoundCue"], sort_keys=True) or
                json.dumps(info["animation"], sort_keys=True) !=
                json.dumps(receipt["expectedAnimation"], sort_keys=True)):
            raise ContractError(f"combat pattern-sound alias source receipt drift: {label}")
        cue = cues[0]
        cue_scope = tuple(cue.get(field) for field in ("patternId", "stageId", "actionId"))
        occurrence = occurrences.get((*scope, cue.get("clipOccurrenceId")))
        if (cue_scope != scope or occurrence is None or cue.get("repeatPolicy") != "once" or
                cue.get("timingBasis", "CLIP_OCCURRENCE") != "CLIP_OCCURRENCE" or
                abs(_event_wall_ms(occurrence, cue["startMs"], label) - expected_ms) > 1e-6):
            raise ContractError(f"combat pattern-sound alias cue wall clock drift: {label}")
        if cue["bindingId"] in used_bindings:
            raise ContractError(f"combat pattern-sound alias reuses one sound: {label}")
        used_bindings.add(cue["bindingId"])
        covered.add(key)
    return covered


def _validate_combat_objects(
        combat_objects: dict[str, Any],
        combat_sounds: dict[str, Any],
        pattern_sound_keys: set[tuple[str, str]] | None = None,
) -> tuple[dict[tuple[str, str], dict[str, Any]], int]:
    if (combat_objects.get("schema") != "lostark.valtan-combat-object-authoring" or
            combat_objects.get("formatVersion") != 1 or
            combat_objects.get("encounterId") != "ENCOUNTER_VALTAN"):
        raise ContractError("combat-object source identity/version is invalid")
    objects = combat_objects.get("objects")
    if not isinstance(objects, list):
        raise ContractError("combat objects must be an array")
    hits: dict[tuple[str, str], dict[str, Any]] = {}
    object_ids: set[str] = set()
    for object_index, row in enumerate(objects):
        object_id = _stable(
            row.get("combatObjectArchetypeId"),
            f"combat objects[{object_index}].combatObjectArchetypeId",
        )
        if object_id in object_ids:
            raise ContractError(f"duplicate combat object archetype: {object_id}")
        object_ids.add(object_id)
        object_hits = row.get("hits", [])
        if not isinstance(object_hits, list):
            raise ContractError(f"combat object {object_id} hits must be an array")
        for hit_index, hit in enumerate(object_hits):
            if not isinstance(hit, dict):
                raise ContractError(f"combat object {object_id} hit[{hit_index}] is invalid")
            hit_id = _stable(hit.get("hitId"), f"combat object {object_id} hitId")
            key = (object_id, hit_id)
            if key in hits:
                raise ContractError(f"duplicate combat-object hit key: {key}")
            hits[key] = hit

    if set(combat_sounds) != {"schema", "formatVersion", "ownerArchetypeId", "cues"}:
        raise ContractError("combat-object sound root fields are invalid")
    if (combat_sounds["schema"] != "lostark.valtan-combat-object-sound-cues" or
            combat_sounds["formatVersion"] != 1 or
            combat_sounds["ownerArchetypeId"] != "BOSS_VALTAN"):
        raise ContractError("combat-object sound identity/version is invalid")
    if not isinstance(combat_sounds["cues"], list):
        raise ContractError("combat-object sound cues must be an array")
    sound_keys: set[tuple[str, str]] = set()
    binding_ids: set[str] = set()
    cue_fields = {
        "bindingId", "combatObjectArchetypeId", "hitId", "soundBank", "soundEvent",
    }
    for cue_index, cue in enumerate(combat_sounds["cues"]):
        context = f"combat-object sound cues[{cue_index}]"
        optional_fields = {"playbackOffsetMs"} if "playbackOffsetMs" in cue else set()
        _exact_fields(cue, cue_fields | optional_fields, context)
        if "playbackOffsetMs" in cue and _integer(cue["playbackOffsetMs"], f"{context}.playbackOffsetMs") > 600000:
            raise ContractError(f"{context}.playbackOffsetMs must be <= 600000")
        binding_id = _stable(cue["bindingId"], f"{context}.bindingId")
        if binding_id in binding_ids:
            raise ContractError(f"duplicate combat-object sound bindingId: {binding_id}")
        binding_ids.add(binding_id)
        key = (
            _stable(cue["combatObjectArchetypeId"], f"{context}.combatObjectArchetypeId"),
            _stable(cue["hitId"], f"{context}.hitId"),
        )
        if key in sound_keys:
            raise ContractError(f"duplicate combat-object sound hit key: {key}")
        sound_keys.add(key)
        _stable(cue["soundBank"], f"{context}.soundBank")
        _stable(cue["soundEvent"], f"{context}.soundEvent")
    aliases = pattern_sound_keys or set()
    if sound_keys & aliases:
        raise ContractError("combat pattern-sound alias duplicates runtime object audio")
    covered_keys = sound_keys | aliases
    if set(hits) != covered_keys:
        missing = sorted(set(hits) - covered_keys)
        stale = sorted(covered_keys - set(hits))
        raise ContractError(
            f"combat-object hit/sound key drift; missing={missing}, stale={stale}"
        )
    return hits, len(sound_keys)


def _catalog_v2_groups(document: dict[str, Any]) -> list[dict[str, Any]]:
    if document.get("schema") != "lostark.boss-catalog":
        raise ContractError("boss catalog schema is invalid")
    bosses = document.get("bosses")
    if not isinstance(bosses, list):
        raise ContractError("boss catalog bosses must be an array")
    valtan = [row for row in bosses if row.get("archetypeId") == "BOSS_VALTAN"]
    if len(valtan) != 1:
        raise ContractError("boss catalog must contain exactly one BOSS_VALTAN row")
    visuals = valtan[0].get("combatObjectVisuals")
    if not isinstance(visuals, list):
        raise ContractError("BOSS_VALTAN combatObjectVisuals must be an array")
    groups: list[dict[str, Any]] = []
    for visual_index, visual in enumerate(visuals):
        group = visual.get("effectV2Group")
        if group is None:
            continue
        if not isinstance(group, dict):
            raise ContractError(f"boss catalog V2 group[{visual_index}] must be an object")
        group_id = _stable(group.get("groupId"), f"boss catalog V2 group[{visual_index}].groupId")
        raw_server_hit_id = group.get("serverHitId")
        server_hit_id = (
            None
            if raw_server_hit_id is None
            else _stable(
                raw_server_hit_id,
                f"boss catalog V2 group[{visual_index}].serverHitId",
            )
        )
        object_id = _stable(
            visual.get("combatObjectArchetypeId"),
            f"boss catalog V2 group[{visual_index}].combatObjectArchetypeId",
        )
        groups.append({
            "resourceKey": ("GROUP", group_id),
            "groupId": group_id,
            "combatObjectArchetypeId": object_id,
            "serverHitId": server_hit_id,
        })
    return groups


def _is_high_frequency_track(offsets: list[int]) -> bool:
    if len(offsets) < 5:
        return False
    gaps = [right - left for left, right in zip(offsets, offsets[1:])]
    return bool(gaps) and min(gaps) > 0 and max(gaps) <= 100


def validate_alignment(
        role_ledger: dict[str, Any],
        allowlist: dict[str, Any],
        gameplay: dict[str, Any],
        presentation: dict[str, Any],
        v2_bindings: dict[str, Any],
        pattern_sounds: dict[str, Any],
        combat_objects: dict[str, Any],
        combat_object_sounds: dict[str, Any],
        boss_catalog: dict[str, Any],
        clip_templates: dict[str, Any],
) -> dict[str, int]:
    """Validate already-loaded documents and return compact coverage counts."""
    roles = _validate_role_ledger(role_ledger)
    (external_allowlist, sound_track_allowlist, presentation_only_allowlist,
     authored_sound_allowlist, combat_pattern_sound_aliases) = _validate_allowlist(allowlist)
    stages, stage_ordinals, occurrences = _build_source_indexes(gameplay, presentation)
    bindings, binding_resources = _validate_bindings(v2_bindings)
    catalog_groups = _catalog_v2_groups(boss_catalog)
    catalog_resources = {row["resourceKey"] for row in catalog_groups}
    expected_role_resources = binding_resources | catalog_resources
    if set(roles) != expected_role_resources:
        missing = sorted(expected_role_resources - set(roles))
        stale = sorted(set(roles) - expected_role_resources)
        raise ContractError(f"effect role coverage drift; missing={missing}, stale={stale}")

    template_effects = _template_effect_index(clip_templates)
    effect_timing_overrides = _validated_effect_timing_overrides(
        clip_templates, template_effects, stages, occurrences, bindings
    )
    used_effect_timing_overrides: set[str] = set()
    alias_keys = _validate_combat_pattern_sound_aliases(
        combat_pattern_sound_aliases, combat_objects, pattern_sounds, stages, occurrences
    )
    combat_hits, combat_sound_count = _validate_combat_objects(
        combat_objects, combat_object_sounds, alias_keys
    )
    used_exceptions: set[str] = {
        row["exceptionId"] for row in combat_pattern_sound_aliases.values()
    }

    combat_v2_contracts = 0
    for resource_key, role in roles.items():
        if role["alignmentPolicy"] != "COMBAT_OBJECT_HIT":
            continue
        matching_groups = [row for row in catalog_groups if row["resourceKey"] == resource_key]
        if len(matching_groups) != 1:
            raise ContractError(
                f"COMBAT_OBJECT_HIT resource needs one exact BossCatalog mapping: {resource_key}"
            )
        mapping = matching_groups[0]
        if mapping["serverHitId"] is None:
            raise ContractError(
                "COMBAT_OBJECT_HIT BossCatalog mapping needs serverHitId: "
                f"{resource_key}"
            )
        hit_key = (mapping["combatObjectArchetypeId"], mapping["serverHitId"])
        if hit_key not in combat_hits:
            raise ContractError(f"BossCatalog serverHitId does not resolve: {hit_key}")
        combat_v2_contracts += 1

    attack_bindings = 0
    aligned_attack_bindings = 0
    template_delegations = 0
    external_bindings = 0
    presentation_only_bindings = 0
    pattern_stage_order: dict[str, list[tuple[str, str, str]]] = defaultdict(list)
    for key, ordinal in stage_ordinals.items():
        pattern_stage_order[key[0]].append((key, ordinal))

    for binding in bindings:
        binding_id = binding["bindingId"]
        scope_row = binding["scope"]
        scope = (scope_row["patternId"], scope_row["stageId"], scope_row["actionId"])
        if scope not in stages:
            exception = external_allowlist.get(binding_id)
            if exception is None:
                raise ContractError(
                    f"V2 binding scope is outside split authoring without exact exception: {binding_id}"
                )
            exception_scope = (
                exception["patternId"], exception["stageId"], exception["actionId"]
            )
            if exception_scope != scope:
                raise ContractError(f"external V2 binding exception scope drift: {binding_id}")
            used_exceptions.add(exception["exceptionId"])
            external_bindings += 1
            continue

        resource_key = (binding["resource"]["kind"], binding["resource"]["id"])
        role = roles[resource_key]
        if role["role"] != "ATTACK":
            continue
        presentation_exception = presentation_only_allowlist.get(binding_id)
        if presentation_exception is not None:
            _validate_presentation_only_binding(
                binding, presentation_exception, stages[scope], occurrences
            )
            used_exceptions.add(presentation_exception["exceptionId"])
            presentation_only_bindings += 1
            continue
        attack_bindings += 1
        policy = role["alignmentPolicy"]
        if policy == "COMBAT_OBJECT_HIT":
            raise ContractError(
                f"COMBAT_OBJECT_HIT resource must come from BossCatalog, not stage binding: {binding_id}"
            )
        if policy == "STAGE_DAMAGE_ACTION":
            current_ordinal = stage_ordinals[scope]
            matching_actions: list[tuple[str, str]] = []
            for candidate_scope, ordinal in pattern_stage_order[scope[0]]:
                if ordinal < current_ordinal:
                    continue
                for event in stages[candidate_scope]["stage"].get("events", []):
                    if isinstance(event, dict) and event.get("kind") in DOWNSTREAM_DAMAGE_ACTIONS:
                        matching_actions.append((candidate_scope[1], event["kind"]))
            if not matching_actions:
                raise ContractError(
                    f"STAGE_DAMAGE_ACTION has no downstream grabbed-player damage: {binding_id}"
                )
            aligned_attack_bindings += 1
            continue

        binding_wall = _binding_wall_ms(binding, occurrences)
        if binding_id in effect_timing_overrides:
            template_wall = effect_timing_overrides[binding_id]
            if (policy not in {"BINDING_START", "CLIP_TEMPLATE", "CLIP_TEMPLATE_OR_BINDING_START"} or
                    not any(abs(template_wall - offset) <= TICK_TOLERANCE_MS
                            for offset in stages[scope]["hitOffsetsMs"])):
                raise ContractError(f"effect timing override lost its template hit: {binding_id}")
            used_effect_timing_overrides.add(binding_id)
            aligned_attack_bindings += 1
            continue
        template_match = _binding_has_template_contract(
            binding, binding_wall, occurrences, template_effects
        )
        if policy == "CLIP_TEMPLATE":
            if not template_match:
                raise ContractError(
                    f"CLIP_TEMPLATE binding lacks exact occurrence/resource contract: {binding_id}"
                )
            template_delegations += 1
            aligned_attack_bindings += 1
            continue
        if policy == "CLIP_TEMPLATE_OR_BINDING_START" and template_match:
            template_delegations += 1
            aligned_attack_bindings += 1
            continue
        if policy not in {"BINDING_START", "CLIP_TEMPLATE_OR_BINDING_START"}:
            raise ContractError(f"unsupported stage attack policy {policy}: {binding_id}")
        hit_offsets = stages[scope]["hitOffsetsMs"]
        if not any(abs(binding_wall - offset) <= TICK_TOLERANCE_MS for offset in hit_offsets):
            raise ContractError(
                f"attack binding has no hit within {int(TICK_TOLERANCE_MS)}ms: "
                f"{binding_id} wall={binding_wall:.3f}, hits={hit_offsets}"
            )
        aligned_attack_bindings += 1

    if set(effect_timing_overrides) != used_effect_timing_overrides:
        raise ContractError("stale hit-alignment effect timing overrides")

    impact_sounds = _validate_pattern_sounds(
        pattern_sounds, stages, occurrences, clip_templates
    )
    stage_hit_points = 0
    sound_aligned_points = 0
    sound_track_exceptions = 0
    authored_sound_exceptions = 0
    for scope, stage_info in stages.items():
        offsets = stage_info["hitOffsetsMs"]
        if not offsets:
            continue
        stage_hit_points += len(offsets)
        candidates = _stage_impact_candidates(scope, stages, impact_sounds)
        unmatched = [
            offset for offset in offsets
            if not any(abs(candidate["wallMs"] - offset) <= TICK_TOLERANCE_MS
                       for candidate in candidates)
        ]
        sound_aligned_points += len(offsets) - len(unmatched)
        if not unmatched:
            continue
        authored_exception = authored_sound_allowlist.get(scope)
        if authored_exception is not None:
            _validate_authored_sound_source(
                authored_exception, scope, stages, pattern_sounds, impact_sounds
            )
            used_exceptions.add(authored_exception["exceptionId"])
            authored_sound_exceptions += 1
            continue
        exception = sound_track_allowlist.get(scope)
        if exception is None:
            available = [
                f"{candidate['wallMs']:.3f}:{candidate['soundEvent']}"
                for candidate in candidates
            ]
            raise ContractError(
                f"stage hit lacks impact sound within {int(TICK_TOLERANCE_MS)}ms: "
                f"scope={scope}, missing={unmatched}, available={available}"
            )
        if exception["expectedHitOffsetsMs"] != offsets:
            raise ContractError(
                f"sound-track exception offsets are stale: {exception['exceptionId']}"
            )
        if not _is_high_frequency_track(offsets):
            raise ContractError(
                f"sound-track exception is only valid for <=100ms high-frequency tracks: "
                f"{exception['exceptionId']}"
            )
        if not candidates:
            raise ContractError(
                f"continuous hit track still needs a representative impact sound: "
                f"{exception['exceptionId']}"
            )
        used_exceptions.add(exception["exceptionId"])
        sound_track_exceptions += 1

    all_exception_ids = {
        row["exceptionId"] for row in allowlist["exceptions"]
    }
    stale_exceptions = sorted(all_exception_ids - used_exceptions)
    if stale_exceptions:
        raise ContractError(f"stale hit-alignment allowlist exceptions: {stale_exceptions}")

    return {
        "roleResources": len(roles),
        "v2Bindings": len(bindings),
        "attackBindings": attack_bindings,
        "alignedAttackBindings": aligned_attack_bindings,
        "templateDelegations": template_delegations,
        "effectTimingOverrides": len(used_effect_timing_overrides),
        "externalBindings": external_bindings,
        "presentationOnlyBindings": presentation_only_bindings,
        "authoredSoundExceptions": authored_sound_exceptions,
        "stageHitPoints": stage_hit_points,
        "soundAlignedPoints": sound_aligned_points,
        "soundTrackExceptions": sound_track_exceptions,
        "combatObjectHits": len(combat_hits),
        "combatObjectSoundCues": combat_sound_count,
        "combatObjectPatternSoundAliases": len(alias_keys),
        "combatV2Contracts": combat_v2_contracts,
    }


def validate_repository(repository_root: Path) -> dict[str, int]:
    return validate_alignment(
        _load(repository_root / ROLE_LEDGER_PATH),
        _load(repository_root / ALLOWLIST_PATH),
        _load(repository_root / GAMEPLAY_PATH),
        _load(repository_root / PRESENTATION_PATH),
        _load(repository_root / V2_BINDINGS_PATH),
        _load(repository_root / SOUND_CUES_PATH),
        _load(repository_root / COMBAT_OBJECTS_PATH),
        _load(repository_root / COMBAT_OBJECT_SOUND_CUES_PATH),
        _load(repository_root / BOSS_CATALOG_PATH),
        _load(repository_root / CLIP_TEMPLATES_PATH),
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repository-root", type=Path, default=REPOSITORY_ROOT)
    parser.add_argument(
        "--check", action="store_true",
        help="Validate without writing (the validator is always read-only).",
    )
    args = parser.parse_args(argv)
    try:
        stats = validate_repository(args.repository_root.resolve())
    except ContractError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    print(
        "PASS: Valtan hit/presentation alignment "
        f"({stats['roleResources']} roles, {stats['attackBindings']} attack bindings, "
        f"{stats['templateDelegations']} template-delegated, "
        f"{stats['stageHitPoints']} stage hit points / "
        f"{stats['soundAlignedPoints']} individually sound-aligned, "
        f"{stats['soundTrackExceptions']} continuous-track exceptions, "
        f"{stats['externalBindings']} counted external V2 bindings, "
        f"{stats['presentationOnlyBindings']} exact presentation-only bindings, "
        f"{stats['authoredSoundExceptions']} exact authored sound receipts, "
        f"{stats['combatObjectHits']} combat-object hits)."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
