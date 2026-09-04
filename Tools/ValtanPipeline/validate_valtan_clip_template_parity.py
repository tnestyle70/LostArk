#!/usr/bin/env python3
"""Fail-closed parity validator for Valtan clip-local attack templates.

The template document is deliberately small: it does not author gameplay.  It
states the hit, V2 effect, and impact-sound contract shared by every occurrence
of a reviewed animation clip.  This validator joins that contract to the split
Valtan authoring documents and rejects an unreviewed occurrence or drift in any
of the three consumers.
"""

from __future__ import annotations

import argparse
import json
import math
import re
import sys
from pathlib import Path
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
TEMPLATE_PATH = Path("Data/Valtan/Valtan.cliptemplates.json")
GAMEPLAY_PATH = Path("Data/Valtan/Valtan.gameplay.json")
PRESENTATION_PATH = Path("Data/Valtan/Valtan.presentation.json")
V2_BINDINGS_PATH = Path("Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json")
SOUND_CUES_PATH = Path("Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json")

STABLE_ID = re.compile(r"^[A-Za-z0-9_.-]{1,200}$")
ROOT_FIELDS = {
    "schema", "formatVersion", "ownerArchetypeId", "tickToleranceMs",
    "templates", "allowlist",
}
TEMPLATE_FIELDS = {"templateId", "clip", "hits", "effects", "sounds"}
HIT_FIELDS = {
    "clipMs", "shape", "serverDamageProfileId", "pushRangeM", "pushMs",
    "knockdown", "downMs",
}
EFFECT_FIELDS = {
    "resourceKind", "resourceId", "clipMs", "anchorSlotId",
}
SOUND_FIELDS = {"soundEvent", "clipMs"}
ALLOWLIST_FIELDS = {
    "patternId", "stageId", "actionId", "clipOccurrenceId", "waivers",
    "reason",
}
EXTRA_HIT_OFFSETS_FIELD = "extraHitOffsetsMs"
ALLOWED_WAIVERS = {
    "HIT", "HIT_SHAPE", "HIT_RESPONSE", "EFFECT", "SOUND", "EXTRA_HIT",
}
SHAPE_FIELDS = {
    "CIRCLE": {"kind", "outerRadiusM"},
    "CONE": {"kind", "angleDegrees", "lengthM"},
    "BOX": {"kind", "forwardOffsetM", "halfWidthM", "halfLengthM"},
    "CROSS": {"kind", "lengthM", "halfWidthM"},
}


class ContractError(ValueError):
    """Raised when a clip-template contract does not resolve exactly."""


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


def _number(value: Any, context: str, *, positive: bool = False) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ContractError(f"{context} must be a finite number")
    result = float(value)
    if not math.isfinite(result) or (positive and result <= 0.0):
        raise ContractError(f"{context} must be {'positive and ' if positive else ''}finite")
    return result


def _validate_shape(shape: Any, context: str) -> dict[str, Any]:
    if not isinstance(shape, dict):
        raise ContractError(f"{context} must be an object")
    kind = shape.get("kind")
    expected = SHAPE_FIELDS.get(kind)
    if expected is None or set(shape) != expected:
        raise ContractError(f"{context} has unsupported or incomplete shape {kind!r}")
    for field, value in shape.items():
        if field != "kind":
            _number(value, f"{context}.{field}", positive=True)
    return shape


def validate_template_document(document: dict[str, Any]) -> None:
    _exact_fields(document, ROOT_FIELDS, "clip template root")
    if document["schema"] != "lostark.valtan-clip-templates":
        raise ContractError("clip template schema is invalid")
    if document["formatVersion"] != 1 or document["ownerArchetypeId"] != "BOSS_VALTAN":
        raise ContractError("clip template identity/version is invalid")
    _integer(document["tickToleranceMs"], "tickToleranceMs", minimum=1)
    if not isinstance(document["templates"], list) or not document["templates"]:
        raise ContractError("templates must be a non-empty array")
    if not isinstance(document["allowlist"], list):
        raise ContractError("allowlist must be an array")

    template_ids: set[str] = set()
    clips: set[str] = set()
    for template_index, template in enumerate(document["templates"]):
        context = f"templates[{template_index}]"
        _exact_fields(template, TEMPLATE_FIELDS, context)
        template_id = _stable(template["templateId"], f"{context}.templateId")
        clip = _stable(template["clip"], f"{context}.clip")
        if template_id in template_ids or clip in clips:
            raise ContractError(f"duplicate templateId or clip at {context}")
        template_ids.add(template_id)
        clips.add(clip)
        for collection in ("hits", "effects", "sounds"):
            if not isinstance(template[collection], list):
                raise ContractError(f"{context}.{collection} must be an array")
        if not template["hits"] or not template["effects"] or not template["sounds"]:
            raise ContractError(f"{context} must declare hit, effect, and sound contracts")

        hit_keys: set[tuple[Any, ...]] = set()
        for index, hit in enumerate(template["hits"]):
            hit_context = f"{context}.hits[{index}]"
            _exact_fields(hit, HIT_FIELDS, hit_context)
            clip_ms = _integer(hit["clipMs"], f"{hit_context}.clipMs")
            shape = _validate_shape(hit["shape"], f"{hit_context}.shape")
            _stable(hit["serverDamageProfileId"], f"{hit_context}.serverDamageProfileId")
            _number(hit["pushRangeM"], f"{hit_context}.pushRangeM")
            _integer(hit["pushMs"], f"{hit_context}.pushMs")
            if not isinstance(hit["knockdown"], bool):
                raise ContractError(f"{hit_context}.knockdown must be boolean")
            _integer(hit["downMs"], f"{hit_context}.downMs")
            key = (clip_ms, json.dumps(shape, sort_keys=True))
            if key in hit_keys:
                raise ContractError(f"duplicate hit contract in {context}")
            hit_keys.add(key)

        effect_keys: set[tuple[str, str, int]] = set()
        for index, effect in enumerate(template["effects"]):
            effect_context = f"{context}.effects[{index}]"
            _exact_fields(effect, EFFECT_FIELDS, effect_context)
            kind = effect["resourceKind"]
            if kind not in {"GROUP", "LEAF"}:
                raise ContractError(f"{effect_context}.resourceKind is invalid")
            resource_id = _stable(effect["resourceId"], f"{effect_context}.resourceId")
            clip_ms = _integer(effect["clipMs"], f"{effect_context}.clipMs")
            _stable(effect["anchorSlotId"], f"{effect_context}.anchorSlotId")
            key = (kind, resource_id, clip_ms)
            if key in effect_keys:
                raise ContractError(f"duplicate effect contract in {context}")
            effect_keys.add(key)

        sound_keys: set[tuple[str, int]] = set()
        for index, sound in enumerate(template["sounds"]):
            sound_context = f"{context}.sounds[{index}]"
            _exact_fields(sound, SOUND_FIELDS, sound_context)
            event = _stable(sound["soundEvent"], f"{sound_context}.soundEvent")
            clip_ms = _integer(sound["clipMs"], f"{sound_context}.clipMs")
            key = (event, clip_ms)
            if key in sound_keys:
                raise ContractError(f"duplicate sound contract in {context}")
            sound_keys.add(key)

    allowlist_keys: set[tuple[str, str, str, str]] = set()
    for index, row in enumerate(document["allowlist"]):
        context = f"allowlist[{index}]"
        has_extra_hit_offsets = (
            isinstance(row, dict) and EXTRA_HIT_OFFSETS_FIELD in row
        )
        _exact_fields(
            row,
            ALLOWLIST_FIELDS |
            ({EXTRA_HIT_OFFSETS_FIELD} if has_extra_hit_offsets else set()),
            context,
        )
        key = tuple(_stable(row[field], f"{context}.{field}") for field in (
            "patternId", "stageId", "actionId", "clipOccurrenceId"
        ))
        if key in allowlist_keys:
            raise ContractError(f"duplicate allowlist occurrence: {key}")
        allowlist_keys.add(key)
        waivers = row["waivers"]
        if (not isinstance(waivers, list) or not waivers or
                any(value not in ALLOWED_WAIVERS for value in waivers) or
                len(waivers) != len(set(waivers))):
            raise ContractError(f"{context}.waivers are invalid")
        if ("EXTRA_HIT" in waivers) != has_extra_hit_offsets:
            raise ContractError(
                f"{context}.EXTRA_HIT requires exact extraHitOffsetsMs"
            )
        if has_extra_hit_offsets:
            offsets = row[EXTRA_HIT_OFFSETS_FIELD]
            if (not isinstance(offsets, list) or not offsets or
                    any(isinstance(value, bool) or not isinstance(value, int) or
                        value < 0 for value in offsets) or
                    offsets != sorted(set(offsets))):
                raise ContractError(
                    f"{context}.extraHitOffsetsMs must be sorted unique offsets"
                )
        reason = row["reason"]
        if not isinstance(reason, str) or len(reason.strip()) < 20:
            raise ContractError(f"{context}.reason must explain the exception")


def _stage_hit_offsets(stage: dict[str, Any]) -> list[int]:
    hit = stage.get("hit")
    if not isinstance(hit, dict) or hit.get("shape", {}).get("kind") == "NONE":
        return []
    schedule = hit.get("schedule")
    if isinstance(schedule, dict):
        kind = schedule.get("kind")
        if kind == "EXPLICIT_OFFSETS":
            return list(schedule.get("offsetsMs", []))
        if kind == "INTERVAL":
            count = schedule.get("count", 0)
            first = schedule.get("firstOffsetMs", 0)
            interval = schedule.get("intervalMs", 0)
            if all(isinstance(value, int) and not isinstance(value, bool)
                   for value in (count, first, interval)):
                return [first + interval * index for index in range(count)]
    activation = hit.get("activation")
    if isinstance(activation, dict) and activation.get("kind") == "ACTIVE_WINDOW":
        start = activation.get("startMs")
        return [start] if isinstance(start, int) and not isinstance(start, bool) else []
    return []


def _shape_matches(actual: dict[str, Any], expected: dict[str, Any]) -> bool:
    if set(actual) != set(expected):
        return False
    for field, value in expected.items():
        if isinstance(value, (int, float)) and not isinstance(value, bool):
            if not math.isclose(float(actual[field]), float(value), rel_tol=0.0, abs_tol=1e-6):
                return False
        elif actual[field] != value:
            return False
    return True


def _response_matches(actual: dict[str, Any], expected: dict[str, Any]) -> bool:
    fields = ("serverDamageProfileId", "pushRangeM", "pushMs", "knockdown", "downMs")
    for field in fields:
        if field not in actual:
            return False
        left, right = actual[field], expected[field]
        if isinstance(right, (int, float)) and not isinstance(right, bool):
            if not math.isclose(float(left), float(right), rel_tol=0.0, abs_tol=1e-6):
                return False
        elif left != right:
            return False
    return True


def _event_stage_ms(occurrence: dict[str, Any], occurrence_start_ms: int,
                    clip_ms: int) -> int | None:
    source_start = occurrence.get("sourceStartMs", 0)
    play_ms = occurrence.get("playMs", 0)
    play_rate = occurrence.get("playRate", 1.0)
    if (isinstance(source_start, bool) or not isinstance(source_start, int) or
            isinstance(play_ms, bool) or not isinstance(play_ms, int) or
            isinstance(play_rate, bool) or not isinstance(play_rate, (int, float)) or
            play_rate <= 0):
        raise ContractError(f"invalid occurrence timing: {occurrence.get('clipOccurrenceId')}")
    if clip_ms < source_start:
        return None
    source_delta = clip_ms - source_start
    # playMs is a source-clip span.  Reject contacts outside that source span,
    # then convert the accepted source delta to the stage wall clock.
    if play_ms and source_delta >= play_ms:
        return None
    local_ms = int(round(source_delta / float(play_rate)))
    return occurrence_start_ms + local_ms


def _occurrence_wall_duration_ms(occurrence: dict[str, Any]) -> int:
    occurrence_id = occurrence.get("clipOccurrenceId")
    play_ms = occurrence.get("playMs", 0)
    play_rate = occurrence.get("playRate", 1.0)
    if (isinstance(play_ms, bool) or not isinstance(play_ms, int) or play_ms < 0 or
            isinstance(play_rate, bool) or not isinstance(play_rate, (int, float)) or
            play_rate <= 0):
        raise ContractError(f"invalid occurrence timing: {occurrence_id}")
    return int(round(play_ms / float(play_rate)))


def validate_parity(template_document: dict[str, Any], gameplay: dict[str, Any],
                    presentation: dict[str, Any], v2_bindings: dict[str, Any],
                    sound_cues: dict[str, Any]) -> dict[str, int]:
    """Validate loaded documents and return compact coverage statistics."""
    validate_template_document(template_document)
    if gameplay.get("schema") != "lostark.valtan-gameplay-authoring":
        raise ContractError("gameplay source schema is invalid")
    if presentation.get("schema") != "lostark.valtan-pattern-presentation-authoring":
        raise ContractError("presentation source schema is invalid")
    if v2_bindings.get("schema") != "lostark.effect-v2-bindings":
        raise ContractError("V2 binding schema is invalid")
    if sound_cues.get("schema") != "lostark.valtan-pattern-sound-cues":
        raise ContractError("pattern sound schema is invalid")

    gameplay_stages: dict[tuple[str, str, str], dict[str, Any]] = {}
    for pattern in gameplay.get("patterns", []):
        pattern_id = pattern.get("patternId")
        for stage in pattern.get("stages", []):
            key = (pattern_id, stage.get("stageId"), stage.get("actionId"))
            if key in gameplay_stages:
                raise ContractError(f"duplicate gameplay stage tuple: {key}")
            gameplay_stages[key] = stage

    occurrences: list[tuple[tuple[str, str, str], dict[str, Any], int]] = []
    occurrence_ids: set[str] = set()
    for pattern in presentation.get("patterns", []):
        pattern_id = pattern.get("patternId")
        for stage in pattern.get("stages", []):
            key = (pattern_id, stage.get("stageId"), stage.get("actionId"))
            if key not in gameplay_stages:
                raise ContractError(f"presentation stage does not resolve to gameplay: {key}")
            offset = 0
            for occurrence in stage.get("animation", {}).get("occurrences", []):
                occurrence_id = occurrence.get("clipOccurrenceId")
                if occurrence_id in occurrence_ids:
                    raise ContractError(f"duplicate clipOccurrenceId: {occurrence_id}")
                occurrence_ids.add(occurrence_id)
                occurrences.append((key, occurrence, offset))
                offset += _occurrence_wall_duration_ms(occurrence)

    templates = {row["clip"]: row for row in template_document["templates"]}
    waivers: dict[tuple[str, str, str, str], set[str]] = {}
    waiver_reasons: dict[tuple[str, str, str, str], str] = {}
    extra_hit_offsets: dict[tuple[str, str, str, str], list[int]] = {}
    for row in template_document["allowlist"]:
        key = (row["patternId"], row["stageId"], row["actionId"], row["clipOccurrenceId"])
        if row["clipOccurrenceId"] not in occurrence_ids:
            raise ContractError(f"allowlist occurrence does not exist: {key}")
        waivers[key] = set(row["waivers"])
        waiver_reasons[key] = row["reason"]
        if EXTRA_HIT_OFFSETS_FIELD in row:
            extra_hit_offsets[key] = list(row[EXTRA_HIT_OFFSETS_FIELD])

    binding_rows = v2_bindings.get("bindings", [])
    sound_rows = sound_cues.get("cues", [])
    used_waivers: set[tuple[tuple[str, str, str, str], str]] = set()
    covered_occurrences = 0
    checked_hits = 0
    checked_effects = 0
    checked_sounds = 0

    def waive_or_fail(occurrence_key: tuple[str, str, str, str], waiver: str,
                      message: str) -> None:
        if waiver in waivers.get(occurrence_key, set()):
            used_waivers.add((occurrence_key, waiver))
            return
        raise ContractError(f"{message}: {occurrence_key}")

    for stage_key, occurrence, occurrence_start in occurrences:
        template = templates.get(occurrence.get("clip"))
        if template is None:
            continue
        pattern_id, stage_id, action_id = stage_key
        occurrence_id = occurrence.get("clipOccurrenceId")
        occurrence_key = (pattern_id, stage_id, action_id, occurrence_id)
        stage = gameplay_stages[stage_key]
        stage_offsets = _stage_hit_offsets(stage)
        tolerance = template_document["tickToleranceMs"]
        covered_occurrences += 1
        expected_hit_stage_offsets: list[int] = []

        for expected_hit in template["hits"]:
            expected_ms = _event_stage_ms(occurrence, occurrence_start, expected_hit["clipMs"])
            if expected_ms is None:
                waive_or_fail(occurrence_key, "HIT", "truncated occurrence lacks HIT waiver")
                continue
            expected_hit_stage_offsets.append(expected_ms)
            nearest = min(stage_offsets, key=lambda value: abs(value - expected_ms), default=None)
            if nearest is None or abs(nearest - expected_ms) > tolerance:
                waive_or_fail(occurrence_key, "HIT", f"missing hit near stage {expected_ms}ms")
                continue
            checked_hits += 1
            actual_hit = stage.get("hit", {})
            if not _shape_matches(actual_hit.get("shape", {}), expected_hit["shape"]):
                waive_or_fail(occurrence_key, "HIT_SHAPE", "hit shape differs from clip template")
            if not _response_matches(actual_hit, expected_hit):
                waive_or_fail(occurrence_key, "HIT_RESPONSE", "hit response differs from clip template")

        occurrence_end = occurrence_start + _occurrence_wall_duration_ms(occurrence)
        occurrence_hit_offsets = [
            value for value in stage_offsets
            if occurrence_start <= value < occurrence_end
        ]
        unexpected_hit_offsets = [
            value for value in occurrence_hit_offsets
            if not any(abs(value - expected) <= tolerance
                       for expected in expected_hit_stage_offsets)
        ]
        if unexpected_hit_offsets:
            if ("EXTRA_HIT" not in waivers.get(occurrence_key, set()) or
                    extra_hit_offsets.get(occurrence_key) != unexpected_hit_offsets):
                raise ContractError(
                    "unreviewed extra hit offsets "
                    f"{unexpected_hit_offsets}: {occurrence_key}"
                )
            used_waivers.add((occurrence_key, "EXTRA_HIT"))

        for expected_effect in template["effects"]:
            expected_ms = _event_stage_ms(occurrence, occurrence_start, expected_effect["clipMs"])
            if expected_ms is None:
                waive_or_fail(occurrence_key, "EFFECT", "truncated occurrence lacks EFFECT waiver")
                continue
            matches = []
            for binding in binding_rows:
                scope = binding.get("scope", {})
                resource = binding.get("resource", {})
                clock = binding.get("clock", {})
                if (scope.get("patternId"), scope.get("stageId"), scope.get("actionId")) != stage_key:
                    continue
                if (resource.get("kind"), resource.get("id")) != (
                        expected_effect["resourceKind"], expected_effect["resourceId"]):
                    continue
                basis = clock.get("basis")
                clip_match = (
                    basis == "CLIP_OCCURRENCE" and
                    clock.get("clipOccurrenceId") == occurrence_id and
                    clock.get("startMs") == expected_effect["clipMs"]
                )
                stage_match = (
                    basis == "STAGE" and occurrence_start == 0 and
                    clock.get("clipOccurrenceId") is None and
                    clock.get("startMs") == expected_ms
                )
                if clip_match or stage_match:
                    matches.append(binding)
            if len(matches) != 1:
                waive_or_fail(occurrence_key, "EFFECT", "effect template must resolve exactly once")
            else:
                anchor = matches[0].get("anchor", {})
                if anchor.get("slotId") != expected_effect["anchorSlotId"]:
                    waive_or_fail(occurrence_key, "EFFECT", "effect anchor differs from clip template")
                else:
                    checked_effects += 1

        for expected_sound in template["sounds"]:
            expected_ms = _event_stage_ms(occurrence, occurrence_start, expected_sound["clipMs"])
            if expected_ms is None:
                waive_or_fail(occurrence_key, "SOUND", "truncated occurrence lacks SOUND waiver")
                continue
            matches = [row for row in sound_rows if (
                row.get("patternId"), row.get("stageId"), row.get("actionId"),
                row.get("clipOccurrenceId"), row.get("soundEvent")
            ) == (
                pattern_id, stage_id, action_id, occurrence_id,
                expected_sound["soundEvent"],
            ) and isinstance(row.get("startMs"), int) and
                abs(row["startMs"] - expected_sound["clipMs"]) <= tolerance]
            if len(matches) != 1:
                waive_or_fail(occurrence_key, "SOUND", "sound template must resolve exactly once")
            else:
                checked_sounds += 1

    if not covered_occurrences:
        raise ContractError("no templated clip occurrence was found")
    for key, declared in waivers.items():
        for waiver in declared:
            if (key, waiver) not in used_waivers:
                raise ContractError(
                    f"stale allowlist waiver {waiver}: {key}; {waiver_reasons[key]}"
                )

    return {
        "templates": len(templates),
        "occurrences": covered_occurrences,
        "hits": checked_hits,
        "effects": checked_effects,
        "sounds": checked_sounds,
        "waivers": len(used_waivers),
    }


def validate_repository(repository_root: Path) -> dict[str, int]:
    root = repository_root.resolve()
    return validate_parity(
        _load(root / TEMPLATE_PATH),
        _load(root / GAMEPLAY_PATH),
        _load(root / PRESENTATION_PATH),
        _load(root / V2_BINDINGS_PATH),
        _load(root / SOUND_CUES_PATH),
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repository-root", type=Path, default=REPOSITORY_ROOT)
    parser.add_argument("--check", action="store_true",
                        help="Validate only (the validator never writes files).")
    args = parser.parse_args(argv)
    try:
        stats = validate_repository(args.repository_root)
    except ContractError as exc:
        print(f"Valtan clip template parity: FAIL: {exc}", file=sys.stderr)
        return 1
    print(
        "Valtan clip template parity: PASS "
        f"({stats['templates']} templates, {stats['occurrences']} occurrences, "
        f"{stats['hits']} hits, {stats['effects']} effects, "
        f"{stats['sounds']} sounds, {stats['waivers']} reviewed waivers)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
