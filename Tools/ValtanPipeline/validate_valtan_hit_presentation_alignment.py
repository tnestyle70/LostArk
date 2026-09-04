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
    exception_ids: set[str] = set()
    for index, row in enumerate(document["exceptions"]):
        context = f"hit alignment allowlist exceptions[{index}]"
        _exact_fields(row, ALLOWLIST_FIELDS, context)
        exception_id = _stable(row["exceptionId"], f"{context}.exceptionId")
        if exception_id in exception_ids:
            raise ContractError(f"duplicate allowlist exceptionId: {exception_id}")
        exception_ids.add(exception_id)
        rule = row["rule"]
        if rule not in ALLOWED_EXCEPTION_RULES:
            raise ContractError(f"{context}.rule is invalid: {rule!r}")
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
        if rule == "EXTERNAL_V2_BINDING_SCOPE":
            binding_id = _stable(binding_id, f"{context}.bindingId")
            if offsets:
                raise ContractError(f"{context} external binding exception cannot contain hit offsets")
            if binding_id in by_binding:
                raise ContractError(f"duplicate external binding exception: {binding_id}")
            by_binding[binding_id] = row
        else:
            if binding_id is not None:
                raise ContractError(f"{context} sound-track exception bindingId must be null")
            if not offsets:
                raise ContractError(f"{context} sound-track exception needs exact hit offsets")
            if scope in by_stage:
                raise ContractError(f"duplicate sound-track exception scope: {scope}")
            by_stage[scope] = row
    return by_binding, by_stage


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
        _exact_fields(cue, cue_fields, context)
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


def _validate_combat_objects(
        combat_objects: dict[str, Any],
        combat_sounds: dict[str, Any],
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
        _exact_fields(cue, cue_fields, context)
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
    if set(hits) != sound_keys:
        missing = sorted(set(hits) - sound_keys)
        stale = sorted(sound_keys - set(hits))
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
    external_allowlist, sound_track_allowlist = _validate_allowlist(allowlist)
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
    combat_hits, combat_sound_count = _validate_combat_objects(
        combat_objects, combat_object_sounds
    )
    used_exceptions: set[str] = set()

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

    impact_sounds = _validate_pattern_sounds(
        pattern_sounds, stages, occurrences, clip_templates
    )
    stage_hit_points = 0
    sound_aligned_points = 0
    sound_track_exceptions = 0
    for scope, stage_info in stages.items():
        offsets = stage_info["hitOffsetsMs"]
        if not offsets:
            continue
        stage_hit_points += len(offsets)
        candidates = impact_sounds.get(scope, [])
        unmatched = [
            offset for offset in offsets
            if not any(abs(candidate["wallMs"] - offset) <= TICK_TOLERANCE_MS
                       for candidate in candidates)
        ]
        sound_aligned_points += len(offsets) - len(unmatched)
        if not unmatched:
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
        "externalBindings": external_bindings,
        "stageHitPoints": stage_hit_points,
        "soundAlignedPoints": sound_aligned_points,
        "soundTrackExceptions": sound_track_exceptions,
        "combatObjectHits": len(combat_hits),
        "combatObjectSoundCues": combat_sound_count,
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
        f"{stats['combatObjectHits']} combat-object hits)."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
