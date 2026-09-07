#!/usr/bin/env python3
"""Validate the Gate 1 KoukuSaydon composition and project its Product views.

The composition document is the only directly-authored owner.  Runtime-facing
encounter and animation binding documents are deterministic projections of the
rows whose authoringStatus is PRODUCT.
"""

from __future__ import annotations

import argparse
import json
import math
import os
from pathlib import Path
import re
import shutil
import shlex
import sys
import tempfile
import uuid
from typing import Any, Iterable

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
if __package__ in (None, ""):
    sys.path.insert(0, str(REPOSITORY_ROOT))
from Tools.RenderingPipeline.light_resources_pipeline import (
    LightValidationError, validate_resources as validate_light_resources,
    validate_map_lights_v2,
)

SOURCE_PATH = Path("Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json")
ENCOUNTER_PATH = Path(
    "Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json"
)
PRESENTATION_PATH = Path(
    "Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json"
)
LIGHT_RESOURCES_PATH = Path("Data/Rendering/Authored/LightResources.json")
REFERENCE_ROOT = Path("Data/Animation/Reference/KoukuSaydon")
REFERENCE_MODEL_ASSET_IDS = {
    "MN_RPCT_05": "Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05",
    "MN_RPCT_06": "Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06",
    "MN_RPCT_07": "Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05",
    "MN_RPCZ_00": "Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00",
}

SCHEMA = "lostark.kouku-saydon-composition"
FORMAT_VERSION = 2
COMPOSITION_ID = "boss.composition.kakulsaydon.gate1"
ENCOUNTER_ID = "ENCOUNTER_KAKULSAYDON_G1"
BOSS_ARCHETYPE_ID = "BOSS_KAKULSAYDON_G1_KOUKU"
BOSS_PLACEMENT_ID = "boss.kakulsaydon.g1.kouku"
AREA_ID = "LV_LUT_MIDNIGHTC_ED"
FIXED_TICK_HZ = 30

ROOT_KEYS = {
    "schema",
    "formatVersion",
    "revision",
    "compositionId",
    "encounterId",
    "bossArchetypeId",
    "bossPlacementId",
    "areaId",
    "fixedTickHz",
    "nextPatternOrdinal",
    "playAllPatternIds",
    "patterns",
}
PATTERN_KEYS = {
    "patternId",
    "actorProfileId",
    "displayName",
    "authoringStatus",
    "category",
    "nextStageOrdinal",
    "nextAnimationOrdinal",
    "stages",
}
STAGE_KEYS = {
    "stageId",
    "actionId",
    "stageKind",
    "durationMs",
    "animationOccurrences",
}
OCCURRENCE_KEYS = {
    "occurrenceId",
    "profileId",
    "sourceActionId",
    "sourceStageId",
    "sourceSlotId",
    "referenceRevision",
    "runtimeClip",
    "startOffsetMs",
    "sourceStartMs",
    "playMs",
    "playRate",
    "endPolicy",
}
REFERENCE_ROOT_KEYS = {
    "schema",
    "formatVersion",
    "authority",
    "profileId",
    "modelAssetId",
    "sourceEvidenceSha256",
    "referenceRevision",
    "actions",
}

# Catalog keys are optional on read so a document written before a catalog
# existed still opens; the editor always writes them.
ROOT_OPTIONAL_KEYS = {
    "nextLogicOrdinal", "logics", "nextSummonOrdinal", "summons",
    "nextWorldOrdinal", "worlds", "nextSceneProfileOrdinal", "sceneProfiles",
    "madnessPolicy", "presentationResources", "nextPresentationResourceOrdinal",
}
PATTERN_OPTIONAL_KEYS = {
    "nextLogicOccurrenceOrdinal",
    "logicOccurrences",
    "nextSummonOccurrenceOrdinal",
    "summonOccurrences",
    "nextWorldOccurrenceOrdinal",
    "worldOccurrences",
    "nextSceneProfileOccurrenceOrdinal",
    "sceneProfileOccurrences", "resetBossToSpawn",
    "presentationOccurrences", "nextPresentationOccurrenceOrdinal",
}
LOGIC_KEYS = {"logicId", "displayName", "logicType"}
# Typed judgement values live on the DURATION definition, typed outcome values
# on the RESULT definition. Which value keys a kind may carry is exact.
LOGIC_KIND_VALUE_KEYS = {
    "ROULETTE_CARD_MATCH": {
        "sectorCount", "sectorSymbols", "centerX", "centerZ", "outerRadiusM",
        "worldSequenceInstanceId", "regionIds",
    },
    "GAZE_REAL_BOSS": {"halfAngleDegrees", "maxDistanceM"},
    "POSE_INPUT": {"poseIndex"},
    "STAGGER_WINDOW": {"threshold", "shieldArcDegrees", "endsPatternOnSuccess", "normalYawOffsetDegrees"},
    "AREA_OVERLAP": {"insideOutcome"},
}
LOGIC_DURATION_VALUE_KEYS = {"judgementKind"} | set().union(*LOGIC_KIND_VALUE_KEYS.values())
LOGIC_RESULT_VALUE_KEYS = {"outcomeKind", "percent", "durationMs", "followupPatternId"}
LOGIC_TRIGGER_VALUE_KEYS = {"triggerKind", "hudMode", "teleportPosition", "clonePatternId", "clockHours", "faceCenterYawOffsetDegrees"}
LOGIC_OPTIONAL_KEYS = LOGIC_DURATION_VALUE_KEYS | LOGIC_RESULT_VALUE_KEYS | LOGIC_TRIGGER_VALUE_KEYS
JUDGEMENT_KINDS = set(LOGIC_KIND_VALUE_KEYS)
# End-tick kinds judge once when the window closes: Success or Fail, never
# Timeout. The boss-level stagger window has no wrong answer, so no Fail.
END_TICK_KINDS = {"GAZE_REAL_BOSS"}
OUTCOME_KINDS = {
    "INSTANT_DEATH", "MAX_HP_PERCENT_DAMAGE", "MADNESS_GAUGE_ADD_PERCENT",
    "CLOWN_TRANSFORM", "FOLLOWUP_PATTERN",
}
PERCENT_OUTCOME_KINDS = {"MAX_HP_PERCENT_DAMAGE", "MADNESS_GAUGE_ADD_PERCENT"}
CARD_SYMBOLS = ("HEART", "SPADE", "CLUB", "DIAMOND")
OUTCOME_SLOTS = ("Success", "Fail", "Timeout")
MAX_OUTCOMES_PER_SLOT = 4
DANCE_POSE_COUNT = 4
LOGIC_OCCURRENCE_KEYS = {"occurrenceId", "logicId", "startMs", "durationMs"}
# Outcome slots are optional on read; the singular keys are the pre-list form
# and read as a one-entry list.
LOGIC_OCCURRENCE_OPTIONAL_KEYS = {
    "enabled",
    "onSuccessLogicId", "onTimeoutLogicId",
    "onSuccessLogicIds", "onFailLogicIds", "onTimeoutLogicIds",
}
# A Summon is named only today; what it spawns is a later definition field.
SUMMON_KEYS = {"summonId", "displayName"}
SUMMON_OCCURRENCE_KEYS = {"occurrenceId", "summonId", "startMs", "durationMs"}
GENERATED_SUMMON_RE = re.compile(r"^kakulsaydon\.g1\.summon\.([1-9][0-9]*)$")
MAX_SUMMONS = 4096
MAX_SUMMON_OCCURRENCES_PER_PATTERN = 1024
# A World names one authored world sequence instance of the Area; its box
# plays that instance on the pattern clock at the box speed.
WORLD_KEYS = {"worldId", "displayName", "sequenceInstanceId"}
WORLD_OCCURRENCE_KEYS = {"occurrenceId", "worldId", "startMs", "durationMs", "playbackSpeed"}
GENERATED_WORLD_RE = re.compile(r"^kakulsaydon\.g1\.world\.([1-9][0-9]*)$")
MAX_WORLDS = 4096
MAX_WORLD_OCCURRENCES_PER_PATTERN = 16
# A Scene Profile names one rendering profile; its box applies that profile on
# the pattern clock for the box lifetime.
SCENE_PROFILE_KEYS = {"sceneProfileId", "displayName", "renderingProfileId"}
SCENE_PROFILE_OCCURRENCE_KEYS = {
    "occurrenceId", "sceneProfileId", "startMs", "durationMs", "blendMs",
}
GENERATED_SCENE_PROFILE_RE = re.compile(r"^kakulsaydon\.g1\.sceneprofile\.([1-9][0-9]*)$")
MAX_SCENE_PROFILES = 4096
MAX_SCENE_PROFILE_OCCURRENCES_PER_PATTERN = 16
MADNESS_POLICY_KEYS = {"maximum", "clownHoldMs"}
DEFAULT_MADNESS_POLICY = {"maximum": 100, "clownHoldMs": 15000}
WORLD_SEQUENCE_PATH = Path("Data/Maps/Authoring/{area}/{area}.worldsequences.json")
# A roulette judgement must end while the wheel stands still: the yaw over the
# last 300 ms before the window end may not move more than this.
ROULETTE_HOLD_MS = 300
ROULETTE_HOLD_TOLERANCE_DEGREES = 0.05

AUTHORING_STATUSES = {"DRAFT", "PRODUCT"}
PATTERN_CATEGORIES = {"NORMAL", "MECHANIC"}
STAGE_KINDS = {"WINDUP", "ACTIVE", "RECOVERY"}
END_POLICIES = {"EXACT", "HOLD_LAST_POSE", "LOOP_TO_WINDOW"}
LOGIC_TYPES = {"DURATION", "TRIGGER", "RESULT"}
STABLE_ID_RE = re.compile(r"^[A-Za-z0-9_.-]{1,128}$")
SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
GENERATED_PATTERN_RE = re.compile(r"^KAKULSAYDON_G1_PATTERN_([1-9][0-9]*)$")
GENERATED_STAGE_RE = re.compile(r"^STAGE_([1-9][0-9]*)$")
GENERATED_LOGIC_RE = re.compile(r"^kakulsaydon\.g1\.logic\.([1-9][0-9]*)$")
MAX_ORDINAL = 1_000_000
MAX_TIMELINE_MS = 600_000
MAX_PATTERNS = 4096
MAX_PRODUCT_PATTERNS = 64
MAX_DRAFT_STAGES = 1024
MAX_PRODUCT_STAGES = 64
MAX_OCCURRENCES = 4096
MAX_LOGICS = 4096
MAX_LOGIC_OCCURRENCES_PER_PATTERN = 1024
MAX_PRODUCT_LOGIC_WINDOWS = 64


class CompositionError(ValueError):
    """Raised when source or projected Product violates the strict contract."""


def _reject_duplicate_pairs(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise CompositionError(f"duplicate JSON property: {key}")
        result[key] = value
    return result


def load_json(path: Path) -> dict[str, Any]:
    try:
        text = path.read_text(encoding="utf-8")
    except (OSError, UnicodeError) as error:
        raise CompositionError(f"cannot read UTF-8 JSON {path}: {error}") from error
    try:
        value = json.loads(text, object_pairs_hook=_reject_duplicate_pairs)
    except (json.JSONDecodeError, CompositionError) as error:
        raise CompositionError(f"invalid JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise CompositionError(f"JSON root must be an object: {path}")
    return value


def _exact_keys(value: Any, expected: set[str], context: str) -> None:
    if not isinstance(value, dict) or set(value) != expected:
        actual = sorted(value) if isinstance(value, dict) else type(value).__name__
        raise CompositionError(
            f"{context} fields are invalid; expected={sorted(expected)!r} "
            f"actual={actual!r}"
        )


def _keys(value: Any, required: set[str], optional: set[str], context: str) -> None:
    if not isinstance(value, dict):
        raise CompositionError(f"{context} must be an object, got {type(value).__name__}")
    actual = set(value)
    missing = required - actual
    unknown = actual - required - optional
    if missing or unknown:
        raise CompositionError(
            f"{context} fields are invalid; missing={sorted(missing)!r} "
            f"unknown={sorted(unknown)!r}"
        )


def _integer(value: Any, context: str, minimum: int, maximum: int) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise CompositionError(f"{context} must be an integer")
    if not minimum <= value <= maximum:
        raise CompositionError(f"{context} is out of range: {value}")
    return value


def _number(value: Any, context: str, minimum: float, maximum: float) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise CompositionError(f"{context} must be a number")
    result = float(value)
    if not math.isfinite(result) or not minimum <= result <= maximum:
        raise CompositionError(f"{context} is out of range: {value}")
    return result


def _boolean(value: Any, context: str) -> bool:
    if not isinstance(value, bool):
        raise CompositionError(f"{context} must be a boolean")
    return value


def _string(value: Any, context: str, maximum: int = 128) -> str:
    if not isinstance(value, str) or not value or len(value) > maximum:
        raise CompositionError(f"{context} must be a non-empty string")
    return value


def _display_name(value: Any, context: str) -> str:
    result = _string(value, context, 255)
    if len(result.encode("utf-8")) > 255 or any(ord(character) < 0x20 for character in result):
        raise CompositionError(
            f"{context} must be 1..255 UTF-8 bytes without control characters"
        )
    return result


def _stable_id(value: Any, context: str) -> str:
    result = _string(value, context)
    if STABLE_ID_RE.fullmatch(result) is None:
        raise CompositionError(f"{context} is not a stable ID: {result!r}")
    return result


def _array(value: Any, context: str, maximum: int) -> list[Any]:
    if not isinstance(value, list) or len(value) > maximum:
        raise CompositionError(f"{context} must be an array of at most {maximum} rows")
    return value


def resolve_actor_profile_id(source_profile_id: str) -> str:
    if source_profile_id not in REFERENCE_MODEL_ASSET_IDS:
        return ""
    return "MN_RPCT_05" if source_profile_id == "MN_RPCT_07" else source_profile_id


BOSS_CATALOG_PATH = Path("Data/Actors/BossCatalog.json")
ARENA_BOSS_ARCHETYPE_PREFIX = "BOSS_KAKULSAYDON_"


def arena_boss_archetypes_by_profile(root: Path = REPOSITORY_ROOT) -> dict[str, list[str]]:
    """Physical actor body -> KoukuSaydon arena boss archetypes presenting on it.

    A PRODUCT pattern plays only on a live arena boss whose catalog body is the
    pattern's actor body, so the join is read from the boss catalog instead of
    being authored a second time in the composition."""
    catalog = load_json(root / BOSS_CATALOG_PATH)
    result: dict[str, set[str]] = {}
    for boss in catalog.get("bosses", []):
        if not isinstance(boss, dict):
            continue
        archetype_id = boss.get("archetypeId")
        body_model = boss.get("bodyModel")
        if not isinstance(archetype_id, str) or not isinstance(body_model, str):
            continue
        if not archetype_id.startswith(ARENA_BOSS_ARCHETYPE_PREFIX):
            continue
        for profile_id, model_asset_id in REFERENCE_MODEL_ASSET_IDS.items():
            if resolve_actor_profile_id(profile_id) != profile_id:
                continue
            if body_model == model_asset_id + ".wmodel":
                result.setdefault(profile_id, set()).add(archetype_id)
    return {profile_id: sorted(ids) for profile_id, ids in result.items()}


def outcome_logic_ids(box: dict[str, Any], slot: str) -> list[str]:
    """The RESULT logic IDs wired to one outcome slot, list or legacy singular."""
    plural = box.get(f"on{slot}LogicIds")
    if plural is not None:
        if not isinstance(plural, list) or not all(isinstance(item, str) for item in plural):
            raise CompositionError(f"on{slot}LogicIds must be a list of text")
        return [item for item in plural if item]
    singular = box.get(f"on{slot}LogicId", "")
    if not isinstance(singular, str):
        raise CompositionError(f"on{slot}LogicId must be text")
    return [singular] if singular else []


def _validate_logic_definition(
    logic: dict[str, Any], context: str, next_logic: int
) -> tuple[str, dict[str, Any]]:
    _keys(logic, LOGIC_KEYS, LOGIC_OPTIONAL_KEYS, context)
    logic_id = _stable_id(logic["logicId"], f"{context} logicId")
    generated_logic = GENERATED_LOGIC_RE.fullmatch(logic_id)
    if generated_logic is None or int(generated_logic.group(1)) >= next_logic:
        raise CompositionError(
            f"logicId must use kakulsaydon.g1.logic.<N> below nextLogicOrdinal: {logic_id}"
        )
    _display_name(logic["displayName"], f"{context} displayName")
    logic_type = logic["logicType"]
    if logic_type not in LOGIC_TYPES:
        raise CompositionError(f"{context} logicType is invalid: {logic_type!r}")
    extra = set(logic) - LOGIC_KEYS
    definition: dict[str, Any] = {"type": logic_type, "kind": None}
    if logic_type == "DURATION":
        if extra - LOGIC_DURATION_VALUE_KEYS:
            raise CompositionError(f"{context} carries RESULT values on a DURATION logic")
        kind = logic.get("judgementKind")
        if kind is None:
            if extra:
                raise CompositionError(f"{context} carries judgement values without a judgementKind")
            return logic_id, definition
        if kind not in JUDGEMENT_KINDS:
            raise CompositionError(f"{context} judgementKind is invalid: {kind!r}")
        allowed = LOGIC_KIND_VALUE_KEYS[kind]
        if (extra - {"judgementKind"}) - allowed:
            raise CompositionError(
                f"{context} carries values another judgementKind owns: "
                f"{sorted((extra - {'judgementKind'}) - allowed)!r}"
            )
        definition["kind"] = kind
        if kind == "ROULETTE_CARD_MATCH" and "regionIds" in logic:
            regions = _array(logic["regionIds"], f"{context} regionIds", 8)
            if len(set(regions)) != len(regions):
                raise CompositionError(f"{context} repeats a regionId")
            for region_id in regions:
                _stable_id(region_id, f"{context} regionId")
            definition["regionIds"] = regions
        elif kind == "ROULETTE_CARD_MATCH":
            sector_count = _integer(logic.get("sectorCount", 0), f"{context} sectorCount", 2, 64)
            symbols = _array(logic.get("sectorSymbols", []), f"{context} sectorSymbols", 64)
            if len(symbols) != sector_count or any(symbol not in CARD_SYMBOLS for symbol in symbols):
                raise CompositionError(
                    f"{context} sectorSymbols must list exactly sectorCount card symbols"
                )
            definition["sectorCount"] = sector_count
            definition["sectorSymbols"] = list(symbols)
            definition["centerX"] = _number(logic.get("centerX", 0.0), f"{context} centerX", -100000.0, 100000.0)
            definition["centerZ"] = _number(logic.get("centerZ", 0.0), f"{context} centerZ", -100000.0, 100000.0)
            definition["outerRadiusM"] = _number(logic.get("outerRadiusM", 0.0), f"{context} outerRadiusM", 0.1, 1000.0)
            definition["worldSequenceInstanceId"] = _stable_id(
                logic.get("worldSequenceInstanceId", ""), f"{context} worldSequenceInstanceId"
            )
        elif kind == "GAZE_REAL_BOSS":
            definition["halfAngleDegrees"] = _number(
                logic.get("halfAngleDegrees", 0.0), f"{context} halfAngleDegrees", 1.0, 180.0
            )
            definition["maxDistanceM"] = _number(
                logic.get("maxDistanceM", 0.0), f"{context} maxDistanceM", 0.0, 1000.0
            )
        elif kind == "POSE_INPUT":
            definition["poseIndex"] = _integer(
                logic.get("poseIndex", -1), f"{context} poseIndex", 0, DANCE_POSE_COUNT - 1
            )
        elif kind == "AREA_OVERLAP":
            if logic.get("insideOutcome", "SUCCESS") not in {"SUCCESS", "FAIL"}:
                raise CompositionError(f"{context} insideOutcome must be SUCCESS or FAIL")
        elif kind == "STAGGER_WINDOW":
            definition["normalYawOffsetDegrees"] = _number(logic.get("normalYawOffsetDegrees", 0), f"{context} normalYawOffsetDegrees", -360, 360)
            definition["threshold"] = _integer(
                logic.get("threshold", 0), f"{context} threshold", 1, 2**32 - 1
            )
            definition["shieldArcDegrees"] = _number(
                logic.get("shieldArcDegrees", 0.0), f"{context} shieldArcDegrees", 0.0, 360.0
            )
            definition["endsPatternOnSuccess"] = _boolean(
                logic.get("endsPatternOnSuccess", False), f"{context} endsPatternOnSuccess"
            )
        return logic_id, definition
    if logic_type == "TRIGGER" and extra:
        kind = logic.get("triggerKind")
        if kind == "HUD_ENTER":
            if extra != {"triggerKind", "hudMode"} or logic["hudMode"] not in {
                "NONE", "POLYMORPH", "MARIO", "DANCE", "MAZE"
            }:
                raise CompositionError(f"{context} HUD_ENTER requires a supported hudMode")
        elif kind == "REAL_GAZE_TELEPORT":
            if extra - {"faceCenterYawOffsetDegrees"} != {"triggerKind", "teleportPosition", "clonePatternId", "clockHours"}:
                raise CompositionError(f"{context} REAL_GAZE_TELEPORT values are incomplete")
            _number(logic.get("faceCenterYawOffsetDegrees", 0), f"{context} faceCenterYawOffsetDegrees", -360, 360)
            position = logic["teleportPosition"]
            if not isinstance(position, list) or len(position) != 3:
                raise CompositionError(f"{context} teleportPosition needs X/Y/Z")
            for value in position:
                _number(value, f"{context} teleportPosition", -100000.0, 100000.0)
            _stable_id(logic["clonePatternId"], f"{context} clonePatternId")
            hours = logic["clockHours"]
            if not isinstance(hours, list) or len(hours) != 3 or len(set(hours)) != 3:
                raise CompositionError(f"{context} clockHours needs three different hours")
            for hour in hours:
                _integer(hour, f"{context} clockHours", 2, 12)
        elif kind == "ENTER_AREA":
            if extra != {"triggerKind"}:
                raise CompositionError(f"{context} ENTER_AREA carries unrelated values")
        else:
            raise CompositionError(f"{context} triggerKind is unsupported")
        definition["kind"] = kind
        return logic_id, definition
    if logic_type == "RESULT":
        if extra - LOGIC_RESULT_VALUE_KEYS:
            raise CompositionError(f"{context} carries judgement values on a RESULT logic")
        kind = logic.get("outcomeKind")
        if kind is None:
            if extra:
                raise CompositionError(f"{context} carries outcome values without an outcomeKind")
            return logic_id, definition
        if kind not in OUTCOME_KINDS:
            raise CompositionError(f"{context} outcomeKind is invalid: {kind!r}")
        definition["kind"] = kind
        percent = _integer(logic.get("percent", 0), f"{context} percent", 0, 100)
        duration_ms = _integer(logic.get("durationMs", 0), f"{context} durationMs", 0, MAX_TIMELINE_MS)
        followup = logic.get("followupPatternId", "")
        if not isinstance(followup, str):
            raise CompositionError(f"{context} followupPatternId must be text")
        if kind in PERCENT_OUTCOME_KINDS and percent == 0:
            raise CompositionError(f"{context} {kind} requires a percent of 1..100")
        if kind not in PERCENT_OUTCOME_KINDS and percent != 0:
            raise CompositionError(f"{context} {kind} does not take a percent")
        if kind != "CLOWN_TRANSFORM" and duration_ms != 0:
            raise CompositionError(f"{context} {kind} does not take a durationMs")
        if (kind == "FOLLOWUP_PATTERN") != bool(followup):
            raise CompositionError(f"{context} FOLLOWUP_PATTERN requires exactly a followupPatternId")
        if followup:
            _stable_id(followup, f"{context} followupPatternId")
        definition["percent"] = percent
        definition["durationMs"] = duration_ms
        definition["followupPatternId"] = followup
        return logic_id, definition
    if extra:
        raise CompositionError(f"{context} TRIGGER logic carries no values")
    return logic_id, definition


def _validate_catalog(
    document: dict[str, Any],
    ordinal_key: str,
    list_key: str,
    id_key: str,
    generated_re: re.Pattern[str],
    exact_keys: set[str],
    maximum: int,
    context: str,
    optional_keys: set[str] | None = None,
) -> tuple[set[str], dict[str, dict[str, Any]]]:
    next_ordinal = _integer(document.get(ordinal_key, 1), ordinal_key, 1, MAX_ORDINAL)
    rows = _array(document.get(list_key, []), f"composition {list_key}", maximum)
    ids: set[str] = set()
    by_id: dict[str, dict[str, Any]] = {}
    for index, row in enumerate(rows):
        row_context = f"{list_key}[{index}]"
        _keys(row, exact_keys, optional_keys or set(), row_context)
        row_id = _stable_id(row[id_key], f"{row_context} {id_key}")
        generated = generated_re.fullmatch(row_id)
        if generated is None or int(generated.group(1)) >= next_ordinal:
            raise CompositionError(
                f"{id_key} must use {context}.<N> below {ordinal_key}: {row_id}"
            )
        if row_id in ids:
            raise CompositionError(f"duplicate {id_key}: {row_id}")
        ids.add(row_id)
        _display_name(row["displayName"], f"{row_context} displayName")
        by_id[row_id] = row
    return ids, by_id


def _validate_boxes(
    pattern: dict[str, Any],
    pattern_id: str,
    context: str,
    ordinal_key: str,
    list_key: str,
    box_keys: set[str],
    suffix: str,
    maximum: int,
    reference_key: str,
    known_ids: set[str],
) -> list[dict[str, Any]]:
    next_ordinal = _integer(
        pattern.get(ordinal_key, 1), f"{context} {ordinal_key}", 1, MAX_ORDINAL
    )
    boxes = _array(pattern.get(list_key, []), f"{context} {list_key}", maximum)
    box_re = re.compile(rf"^{re.escape(pattern_id)}\.{suffix}\.([1-9][0-9]*)$")
    box_ids: set[str] = set()
    for index, box in enumerate(boxes):
        box_context = f"{context}.{list_key}[{index}]"
        _exact_keys(box, box_keys, box_context)
        box_id = _stable_id(box["occurrenceId"], f"{box_context} occurrenceId")
        match = box_re.fullmatch(box_id)
        if match is None:
            raise CompositionError(
                f"{suffix} occurrenceId must use {pattern_id}.{suffix}.<N>: {box_id}"
            )
        if int(match.group(1)) >= next_ordinal:
            raise CompositionError(
                f"{suffix} occurrenceId is ahead of {ordinal_key}: {box_id}"
            )
        if box_id in box_ids:
            raise CompositionError(f"duplicate {suffix} occurrenceId: {box_id}")
        box_ids.add(box_id)
        if box[reference_key] not in known_ids:
            raise CompositionError(
                f"{suffix} box references an unknown {reference_key}: {box_id}"
            )
        start_ms = _integer(box["startMs"], f"{box_context} startMs", 0, MAX_TIMELINE_MS)
        duration_ms = _integer(box["durationMs"], f"{box_context} durationMs", 1, MAX_TIMELINE_MS)
        if start_ms + duration_ms > MAX_TIMELINE_MS:
            raise CompositionError(f"{suffix} box exceeds 600 seconds: {box_id}")
    return boxes


def validate_document(document: dict[str, Any], root: Path = REPOSITORY_ROOT) -> None:
    """Check persisted identities and timing; reference/oracle metadata is advisory."""

    _keys(document, ROOT_KEYS, ROOT_OPTIONAL_KEYS, "composition")
    version = _integer(document["formatVersion"], "composition formatVersion", 1, FORMAT_VERSION)
    exact_values = {
        "schema": SCHEMA,
        "compositionId": COMPOSITION_ID,
        "encounterId": ENCOUNTER_ID,
        "bossArchetypeId": BOSS_ARCHETYPE_ID,
        "bossPlacementId": BOSS_PLACEMENT_ID,
        "areaId": AREA_ID,
        "fixedTickHz": FIXED_TICK_HZ,
    }
    for field, expected in exact_values.items():
        if document[field] != expected:
            raise CompositionError(
                f"composition {field} must be {expected!r}, got {document[field]!r}"
            )
    _integer(document["revision"], "composition revision", 1, 2**32 - 1)
    next_pattern = _integer(
        document["nextPatternOrdinal"], "nextPatternOrdinal", 1, MAX_ORDINAL
    )
    if "madnessPolicy" in document:
        _exact_keys(document["madnessPolicy"], MADNESS_POLICY_KEYS, "madnessPolicy")
        _integer(document["madnessPolicy"]["maximum"], "madnessPolicy maximum", 1, 1_000_000)
        _integer(document["madnessPolicy"]["clownHoldMs"], "madnessPolicy clownHoldMs", 0, MAX_TIMELINE_MS)
    next_logic = _integer(
        document.get("nextLogicOrdinal", 1), "nextLogicOrdinal", 1, MAX_ORDINAL
    )
    logics = _array(document.get("logics", []), "composition logics", MAX_LOGICS)
    logic_ids: set[str] = set()
    logic_defs: dict[str, dict[str, Any]] = {}
    for logic_index, logic in enumerate(logics):
        logic_id, definition = _validate_logic_definition(
            logic, f"logics[{logic_index}]", next_logic
        )
        if logic_id in logic_ids:
            raise CompositionError(f"duplicate logicId: {logic_id}")
        logic_ids.add(logic_id)
        logic_defs[logic_id] = definition
    summon_ids, _ = _validate_catalog(
        document, "nextSummonOrdinal", "summons", "summonId", GENERATED_SUMMON_RE,
        SUMMON_KEYS, MAX_SUMMONS, "kakulsaydon.g1.summon",
    )
    world_ids, worlds_by_id = _validate_catalog(
        document, "nextWorldOrdinal", "worlds", "worldId", GENERATED_WORLD_RE,
        WORLD_KEYS, MAX_WORLDS, "kakulsaydon.g1.world", optional_keys={"positionOffset", "anchorKind", "anchorPosition", "companionEffectResourceId"},
    )
    for world_id, world in worlds_by_id.items():
        _stable_id(world["sequenceInstanceId"], f"world {world_id} sequenceInstanceId")
        offset = world.get("positionOffset", [0.0, 0.0, 0.0])
        if not isinstance(offset, list) or len(offset) != 3:
            raise CompositionError(f"world {world_id} positionOffset needs X/Y/Z")
        for value in offset:
            _number(value, f"world {world_id} positionOffset", -100000.0, 100000.0)
        if world.get("anchorKind", "NONE") not in {"NONE", "BOSS_SPAWN"}:
            raise CompositionError(f"world {world_id} anchorKind is unsupported")
        _vector3(world.get("anchorPosition", [0, 0, 0]), f"world {world_id} anchorPosition", -100000, 100000)
    scene_profile_ids, scene_profiles_by_id = _validate_catalog(
        document, "nextSceneProfileOrdinal", "sceneProfiles", "sceneProfileId",
        GENERATED_SCENE_PROFILE_RE, SCENE_PROFILE_KEYS, MAX_SCENE_PROFILES,
        "kakulsaydon.g1.sceneprofile",
    )
    for profile_id, profile in scene_profiles_by_id.items():
        _stable_id(profile["renderingProfileId"], f"sceneProfile {profile_id} renderingProfileId")
    patterns = _array(document["patterns"], "composition patterns", MAX_PATTERNS)
    if not patterns:
        raise CompositionError("composition must contain at least one pattern")
    play_all = _array(
        document["playAllPatternIds"], "playAllPatternIds", MAX_PATTERNS
    )
    presentation_resources = _validate_presentation_resources(document)
    for world_id, world in worlds_by_id.items():
        companion = world.get("companionEffectResourceId", "")
        if companion != "":
            _stable_id(companion, f"world {world_id} companionEffectResourceId")
            if companion not in presentation_resources or presentation_resources[companion]["kind"] != "EFFECT":
                raise CompositionError("world companionEffectResourceId must reference an EFFECT resource")
    pattern_ids: set[str] = set()
    product_action_ids: set[str] = set()
    action_ids: set[str] = set()
    occurrence_ids: set[str] = set()
    product_ids: list[str] = []
    followup_targets: list[tuple[str, str]] = []

    for pattern_index, pattern in enumerate(patterns):
        context = f"patterns[{pattern_index}]"
        _keys(
            pattern,
            PATTERN_KEYS - {"actorProfileId"} if version == 1 else PATTERN_KEYS,
            PATTERN_OPTIONAL_KEYS,
            context,
        )
        _boolean(pattern.get("resetBossToSpawn", False), f"{context} resetBossToSpawn")
        actor_profile_id = ""
        if version == FORMAT_VERSION:
            actor_profile_id = _stable_id(pattern["actorProfileId"], f"{context} actorProfileId")
            if resolve_actor_profile_id(actor_profile_id) != actor_profile_id:
                raise CompositionError(f"unknown physical actorProfileId: {actor_profile_id}")
        pattern_id = _stable_id(pattern["patternId"], f"{context} patternId")
        if pattern_id in pattern_ids:
            raise CompositionError(f"duplicate patternId: {pattern_id}")
        pattern_ids.add(pattern_id)
        generated_pattern = GENERATED_PATTERN_RE.fullmatch(pattern_id)
        if generated_pattern and int(generated_pattern.group(1)) >= next_pattern:
            raise CompositionError(f"patternId is ahead of nextPatternOrdinal: {pattern_id}")
        _display_name(pattern["displayName"], f"{context} displayName")
        status = pattern["authoringStatus"]
        if status not in AUTHORING_STATUSES:
            raise CompositionError(f"{context} authoringStatus is invalid: {status!r}")
        if pattern["category"] not in PATTERN_CATEGORIES:
            raise CompositionError(f"{context} category is invalid: {pattern['category']!r}")
        next_logic_occurrence = _integer(
            pattern.get("nextLogicOccurrenceOrdinal", 1),
            f"{context} nextLogicOccurrenceOrdinal",
            1,
            MAX_ORDINAL,
        )
        logic_occurrences = _array(
            pattern.get("logicOccurrences", []),
            f"{context} logicOccurrences",
            MAX_LOGIC_OCCURRENCES_PER_PATTERN,
        )
        logic_box_re = re.compile(rf"^{re.escape(pattern_id)}\.logic\.([1-9][0-9]*)$")
        logic_box_ids: set[str] = set()
        product_window_count = 0
        roulette_instances: list[str] = []
        for box_index, box in enumerate(logic_occurrences):
            box_context = f"{context}.logicOccurrences[{box_index}]"
            _keys(box, LOGIC_OCCURRENCE_KEYS, LOGIC_OCCURRENCE_OPTIONAL_KEYS, box_context)
            enabled = _boolean(box.get("enabled", True), f"{box_context} enabled")
            box_id = _stable_id(box["occurrenceId"], f"{box_context} occurrenceId")
            logic_box_match = logic_box_re.fullmatch(box_id)
            if logic_box_match is None:
                raise CompositionError(
                    f"logic occurrenceId must use {pattern_id}.logic.<N>: {box_id}"
                )
            if int(logic_box_match.group(1)) >= next_logic_occurrence:
                raise CompositionError(
                    f"logic occurrenceId is ahead of nextLogicOccurrenceOrdinal: {box_id}"
                )
            if box_id in logic_box_ids:
                raise CompositionError(f"duplicate Logic occurrenceId: {box_id}")
            logic_box_ids.add(box_id)
            if box["logicId"] not in logic_ids:
                raise CompositionError(f"logic box references an unknown logicId: {box_id}")
            owner = logic_defs[box["logicId"]]
            outcomes = {slot: outcome_logic_ids(box, slot) for slot in OUTCOME_SLOTS}
            for slot, targets in outcomes.items():
                if len(targets) > MAX_OUTCOMES_PER_SLOT:
                    raise CompositionError(
                        f"{box_context} on{slot} wires more than {MAX_OUTCOMES_PER_SLOT} RESULT logics"
                    )
                for target in targets:
                    if logic_defs.get(target, {}).get("type") != "RESULT":
                        raise CompositionError(
                            f"{box_context} on{slot}LogicIds must name a RESULT logic: {target!r}"
                        )
                    if owner["type"] != "DURATION" and owner.get("kind") != "ENTER_AREA":
                        raise CompositionError(
                            f"{box_context} outcomes are only valid on a DURATION logic box"
                        )
            kind = owner.get("kind")
            if kind in END_TICK_KINDS and outcomes["Timeout"]:
                raise CompositionError(
                    f"{box_context} {kind} judges once at the window end and has no Timeout slot"
                )
            if kind == "STAGGER_WINDOW" and outcomes["Fail"]:
                raise CompositionError(
                    f"{box_context} STAGGER_WINDOW has no wrong answer and no Fail slot"
                )
            for slot, targets in outcomes.items():
                for target in targets:
                    result_kind = logic_defs[target].get("kind")
                    if result_kind == "FOLLOWUP_PATTERN":
                        if kind != "STAGGER_WINDOW":
                            raise CompositionError(
                                f"{box_context} FOLLOWUP_PATTERN is only valid on a STAGGER_WINDOW box"
                            )
                        followup_targets.append((box_id, logic_defs[target]["followupPatternId"]))
            logic_start_ms = _integer(
                box["startMs"], f"{box_context} startMs", 0, MAX_TIMELINE_MS
            )
            logic_duration_ms = _integer(
                box["durationMs"], f"{box_context} durationMs", 1, MAX_TIMELINE_MS
            )
            if logic_start_ms + logic_duration_ms > MAX_TIMELINE_MS:
                raise CompositionError(f"Logic box exceeds 600 seconds: {box_id}")
            if enabled and status == "PRODUCT" and owner["type"] == "DURATION":
                if kind is None:
                    raise CompositionError(
                        "PRODUCT pattern owns a DURATION logic box without a judgementKind: "
                        f"{box_id}"
                    )
                for targets in outcomes.values():
                    for target in targets:
                        if logic_defs[target].get("kind") is None:
                            raise CompositionError(
                                "PRODUCT pattern wires a RESULT logic without an outcomeKind: "
                                f"{box_id} -> {target}"
                            )
                product_window_count += 1
                if kind == "ROULETTE_CARD_MATCH" and "worldSequenceInstanceId" in owner:
                    roulette_instances.append(owner["worldSequenceInstanceId"])
        if product_window_count > MAX_PRODUCT_LOGIC_WINDOWS:
            raise CompositionError(
                f"PRODUCT pattern exceeds {MAX_PRODUCT_LOGIC_WINDOWS} judgement windows: {pattern_id}"
            )
        _validate_boxes(
            pattern, pattern_id, context, "nextSummonOccurrenceOrdinal", "summonOccurrences",
            SUMMON_OCCURRENCE_KEYS, "summon", MAX_SUMMON_OCCURRENCES_PER_PATTERN,
            "summonId", summon_ids,
        )
        world_boxes = _validate_boxes(
            pattern, pattern_id, context, "nextWorldOccurrenceOrdinal", "worldOccurrences",
            WORLD_OCCURRENCE_KEYS, "world", MAX_WORLD_OCCURRENCES_PER_PATTERN,
            "worldId", world_ids,
        )
        world_instances: set[str] = set()
        for box in world_boxes:
            _number(box["playbackSpeed"], f"{context} world box playbackSpeed", 0.05, 16.0)
            instance_id = worlds_by_id[box["worldId"]]["sequenceInstanceId"]
            if instance_id in world_instances:
                raise CompositionError(
                    f"{context} plays world sequence instance {instance_id} from two boxes"
                )
            world_instances.add(instance_id)
        for instance_id in roulette_instances:
            if instance_id not in world_instances:
                raise CompositionError(
                    f"PRODUCT roulette window needs a World box playing {instance_id}: {pattern_id}"
                )
        scene_boxes = _validate_boxes(
            pattern, pattern_id, context, "nextSceneProfileOccurrenceOrdinal",
            "sceneProfileOccurrences", SCENE_PROFILE_OCCURRENCE_KEYS, "sceneprofile",
            MAX_SCENE_PROFILE_OCCURRENCES_PER_PATTERN, "sceneProfileId", scene_profile_ids,
        )
        for box in scene_boxes:
            _integer(box["blendMs"], f"{context} scene profile box blendMs", 0, MAX_TIMELINE_MS)
        next_stage = _integer(
            pattern["nextStageOrdinal"], f"{context} nextStageOrdinal", 1, MAX_ORDINAL
        )
        next_animation = _integer(
            pattern["nextAnimationOrdinal"],
            f"{context} nextAnimationOrdinal",
            1,
            MAX_ORDINAL,
        )
        stages = _array(pattern["stages"], f"{context} stages",
                        MAX_PRODUCT_STAGES if status == "PRODUCT" else MAX_DRAFT_STAGES)
        if status == "PRODUCT":
            # Which physical bodies may publish is a catalog join; validate_publishable
            # applies it at publish time so this pure check never reads a file.
            product_ids.append(pattern_id)
            if pattern["category"] != "MECHANIC":
                raise CompositionError(
                    f"PRODUCT pattern must use MECHANIC category: {pattern_id}"
                )
            product_action_id = pattern_id.lower().replace("_", ".")
            if product_action_id in product_action_ids:
                raise CompositionError(
                    f"PRODUCT patterns derive duplicate actionId: {product_action_id}"
                )
            product_action_ids.add(product_action_id)
            if len(product_ids) > MAX_PRODUCT_PATTERNS:
                raise CompositionError(
                    f"composition exceeds {MAX_PRODUCT_PATTERNS} PRODUCT patterns"
                )
            if not stages:
                raise CompositionError(f"PRODUCT pattern has no stages: {pattern_id}")
        stage_ids: set[str] = set()
        occurrence_ordinal_re = re.compile(
            rf"^{re.escape(pattern_id)}\.animation\.([1-9][0-9]*)$"
        )
        pattern_duration_ms = 0
        for stage_index, stage in enumerate(stages):
            stage_context = f"{context}.stages[{stage_index}]"
            _exact_keys(stage, STAGE_KEYS, stage_context)
            stage_id = _stable_id(stage["stageId"], f"{stage_context} stageId")
            action_id = _stable_id(stage["actionId"], f"{stage_context} actionId")
            if stage_id in stage_ids:
                raise CompositionError(f"duplicate stageId in {pattern_id}: {stage_id}")
            if action_id in action_ids:
                raise CompositionError(f"duplicate actionId: {action_id}")
            stage_ids.add(stage_id)
            action_ids.add(action_id)
            generated_stage = GENERATED_STAGE_RE.fullmatch(stage_id)
            if generated_stage and int(generated_stage.group(1)) >= next_stage:
                raise CompositionError(
                    f"stageId is ahead of nextStageOrdinal: {pattern_id}/{stage_id}"
                )
            if stage["stageKind"] not in STAGE_KINDS:
                raise CompositionError(
                    f"{stage_context} stageKind is invalid: {stage['stageKind']!r}"
                )
            duration_ms = _integer(
                stage["durationMs"], f"{stage_context} durationMs", 1, MAX_TIMELINE_MS
            )
            pattern_duration_ms += duration_ms
            if pattern_duration_ms > MAX_TIMELINE_MS:
                raise CompositionError(f"Pattern exceeds 600 seconds: {pattern_id}")
            occurrences = _array(
                stage["animationOccurrences"],
                f"{stage_context} animationOccurrences",
                MAX_OCCURRENCES,
            )
            if status == "PRODUCT" and len(occurrences) != 1:
                raise CompositionError(
                    f"PRODUCT stage must contain exactly one animation occurrence: "
                    f"{pattern_id}/{stage_id}"
                )
            for occurrence_index, occurrence in enumerate(occurrences):
                occurrence_context = (
                    f"{stage_context}.animationOccurrences[{occurrence_index}]"
                )
                _exact_keys(occurrence, OCCURRENCE_KEYS, occurrence_context)
                occurrence_id = _stable_id(
                    occurrence["occurrenceId"], f"{occurrence_context} occurrenceId"
                )
                occurrence_match = occurrence_ordinal_re.fullmatch(occurrence_id)
                if occurrence_match is None:
                    raise CompositionError(
                        f"occurrenceId must use {pattern_id}.animation.<N>: {occurrence_id}"
                    )
                if int(occurrence_match.group(1)) >= next_animation:
                    raise CompositionError(
                        f"occurrenceId is ahead of nextAnimationOrdinal: {occurrence_id}"
                    )
                if occurrence_id in occurrence_ids:
                    raise CompositionError(f"duplicate occurrenceId: {occurrence_id}")
                occurrence_ids.add(occurrence_id)
                profile_id = _stable_id(
                    occurrence["profileId"], f"{occurrence_context} profileId"
                )
                occurrence_actor = resolve_actor_profile_id(profile_id)
                if not occurrence_actor:
                    raise CompositionError(f"unknown animation profile: {profile_id}")
                if version == 1 and not actor_profile_id:
                    actor_profile_id = occurrence_actor
                if occurrence_actor != actor_profile_id:
                    raise CompositionError(f"animation profile does not match Pattern actorProfileId: {occurrence_id}")
                source_action_id = _integer(occurrence["sourceActionId"], f"{occurrence_context} sourceActionId", 0, 2**32 - 1)
                _stable_id(occurrence["runtimeClip"], f"{occurrence_context} runtimeClip")
                _stable_id(occurrence["sourceStageId"], f"{occurrence_context} sourceStageId")
                _stable_id(occurrence["sourceSlotId"], f"{occurrence_context} sourceSlotId")
                # Referenced default action 0 is distinct from a physical RAW clip.
                if occurrence["sourceStageId"] == "RAW" and source_action_id != 0:
                    raise CompositionError("physical RAW clips must use sourceActionId 0")
                if not isinstance(occurrence["referenceRevision"], str):
                    raise CompositionError("referenceRevision must be text")
                if occurrence["sourceStageId"] != "RAW" and not SHA256_RE.fullmatch(occurrence["referenceRevision"]):
                    raise CompositionError("referenced animation revision must be a lowercase SHA-256")
                start_offset = _integer(
                    occurrence["startOffsetMs"],
                    f"{occurrence_context} startOffsetMs",
                    0,
                    MAX_TIMELINE_MS,
                )
                source_start = _integer(
                    occurrence["sourceStartMs"],
                    f"{occurrence_context} sourceStartMs",
                    0,
                    MAX_TIMELINE_MS,
                )
                play_ms = _integer(
                    occurrence["playMs"],
                    f"{occurrence_context} playMs",
                    1,
                    MAX_TIMELINE_MS,
                )
                play_rate = _number(
                    occurrence["playRate"],
                    f"{occurrence_context} playRate",
                    0.01,
                    16.0,
                )
                end_policy = occurrence["endPolicy"]
                if end_policy not in END_POLICIES:
                    raise CompositionError(
                        f"{occurrence_context} endPolicy is invalid: {end_policy!r}"
                    )
                if start_offset + play_ms > duration_ms:
                    raise CompositionError(
                        f"animation occurrence exceeds its stage: {occurrence_id}"
                    )
                if status == "PRODUCT" and (
                    start_offset != 0
                    or source_start != 0
                    or play_ms != duration_ms
                    or play_rate < 0.1
                    or play_rate > 4.0
                    or end_policy != "EXACT"
                ):
                    raise CompositionError(
                        "PRODUCT animation must match the current runtime policy "
                        f"(whole stage, sourceStartMs 0, playRate 0.1..4, "
                        f"EXACT): {occurrence_id}"
                    )

        _validate_presentation_occurrences(pattern, presentation_resources, pattern_duration_ms, worlds_by_id)
        for box in logic_occurrences:
            if box["startMs"] + box["durationMs"] > pattern_duration_ms:
                raise CompositionError(
                    f"Logic box exceeds the Pattern lifetime: {box['occurrenceId']}"
                )
        for box in world_boxes:
            if box["startMs"] > pattern_duration_ms:
                raise CompositionError(
                    f"World box starts after the Pattern lifetime: {box['occurrenceId']}"
                )
        for box in scene_boxes:
            if box["startMs"] + box["durationMs"] > pattern_duration_ms:
                raise CompositionError(
                    f"Scene profile box exceeds the Pattern lifetime: {box['occurrenceId']}"
                )

    if play_all != product_ids:
        raise CompositionError(
            "playAllPatternIds must equal PRODUCT patternIds in authored order"
        )
    for box_id, target in followup_targets:
        if target not in pattern_ids:
            raise CompositionError(
                f"FOLLOWUP_PATTERN names an unknown pattern: {box_id} -> {target}"
            )


def load_and_validate(root: Path = REPOSITORY_ROOT) -> dict[str, Any]:
    document = load_json(root / SOURCE_PATH)
    product = dict(document)
    product["patterns"] = [p for p in document.get("patterns", []) if isinstance(p, dict) and p.get("authoringStatus") == "PRODUCT"]
    product["playAllPatternIds"] = [p.get("patternId") for p in product["patterns"]]
    validate_document(product, root)
    return product


def validate_publishable(document: dict[str, Any], root: Path = REPOSITORY_ROOT) -> None:
    _join_light_resources(document, root)
    if not document["playAllPatternIds"]:
        raise CompositionError(
            "composition must retain at least one PRODUCT pattern before publish"
        )
    # A PRODUCT pattern plays on a live arena boss whose catalog body is the
    # pattern's actor body. The join is read here, at publish time, so the pure
    # document validation above never touches the catalog.
    arena_archetypes_by_profile = arena_boss_archetypes_by_profile(root)
    product_ids = set(document["playAllPatternIds"])
    for pattern in document["patterns"]:
        if pattern.get("authoringStatus") != "PRODUCT":
            continue
        actor_profile_id = _pattern_actor_profile_id(pattern)
        if actor_profile_id not in arena_archetypes_by_profile:
            raise CompositionError(
                "PRODUCT actorProfileId has no KoukuSaydon arena boss body: "
                f"{pattern['patternId']} ({actor_profile_id or 'unknown'})"
            )
    # A follow-up the Server begins must itself be a published Product.
    logics = {logic["logicId"]: logic for logic in document.get("logics", [])}
    for pattern in document["patterns"]:
        if pattern.get("authoringStatus") != "PRODUCT":
            continue
        for box in pattern.get("logicOccurrences", []):
            for slot in OUTCOME_SLOTS:
                for target in outcome_logic_ids(box, slot):
                    followup = logics.get(target, {}).get("followupPatternId", "")
                    if followup and followup not in product_ids:
                        raise CompositionError(
                            "FOLLOWUP_PATTERN must name a PRODUCT pattern: "
                            f"{box['occurrenceId']} -> {followup}"
                        )


def _pattern_action_id(pattern_id: str) -> str:
    return pattern_id.lower().replace("_", ".")


def _project_stage(stage: dict[str, Any]) -> dict[str, Any]:
    return {
        "stageId": stage["stageId"],
        "actionId": stage["actionId"],
        "stageKind": stage["stageKind"],
        "durationMs": stage["durationMs"],
        "hitShape": "NONE",
        "hitOuterRadius": 0.0,
        "hitInnerRadius": 0.0,
        "hitAngleDegrees": 0.0,
        "hitLength": 0.0,
        "hitHalfWidth": 0.0,
        "hitCount": 0,
        "hitIntervalMs": 0,
        "hitDelayMs": 0,
        "serverDamageProfileId": "",
        "pushRangeM": 0.0,
        "pushMs": 0,
        "knockdown": False,
        "downMs": 0,
    }


def _pattern_actor_profile_id(source: dict[str, Any]) -> str:
    actor_profile_id = source.get("actorProfileId")
    if isinstance(actor_profile_id, str) and actor_profile_id:
        return actor_profile_id
    for stage in source["stages"]:
        for occurrence in stage["animationOccurrences"]:
            resolved = resolve_actor_profile_id(occurrence["profileId"])
            if resolved:
                return resolved
    return ""


def _quaternion_yaw_degrees(quaternion: list[float]) -> float:
    x, y, z, w = (float(component) for component in quaternion)
    yaw = math.degrees(math.atan2(2.0 * (w * y + x * z), 1.0 - 2.0 * (y * y + z * z)))
    return yaw % 360.0


def _sample_track_yaw(track: dict[str, Any], time_ms: float) -> tuple[float, bool]:
    """Yaw and visibility of a world sequence track at a sequence-local time."""
    keys = track["keys"]
    if not keys:
        raise CompositionError("world sequence track has no keys")
    if time_ms <= keys[0]["timeMs"]:
        return _quaternion_yaw_degrees(keys[0]["rotationQuaternion"]), bool(keys[0].get("visible", True))
    for previous, following in zip(keys, keys[1:]):
        if previous["timeMs"] <= time_ms <= following["timeMs"]:
            span = following["timeMs"] - previous["timeMs"]
            alpha = 0.0 if span <= 0 else (time_ms - previous["timeMs"]) / span
            first = [float(c) for c in previous["rotationQuaternion"]]
            second = [float(c) for c in following["rotationQuaternion"]]
            if sum(a * b for a, b in zip(first, second)) < 0.0:
                second = [-c for c in second]
            blended = [a + (b - a) * alpha for a, b in zip(first, second)]
            length = math.sqrt(sum(c * c for c in blended)) or 1.0
            blended = [c / length for c in blended]
            return _quaternion_yaw_degrees(blended), bool(previous.get("visible", True))
    return _quaternion_yaw_degrees(keys[-1]["rotationQuaternion"]), bool(keys[-1].get("visible", True))


def load_world_sequences(root: Path, area_id: str) -> dict[str, Any]:
    return load_json(root / Path(str(WORLD_SEQUENCE_PATH).format(area=area_id)))


def derive_roulette_stop_yaw(
    sequences: dict[str, Any],
    instance_id: str,
    sequence_local_ms: float,
    context: str,
) -> float:
    """Yaw of the wheel at the judgement time, refused unless the wheel holds still."""
    instance = next(
        (row for row in sequences.get("instances", []) if row.get("instanceId") == instance_id),
        None,
    )
    if instance is None:
        raise CompositionError(f"{context} world sequence instance is unknown: {instance_id}")
    template = next(
        (row for row in sequences.get("templates", []) if row.get("sequenceId") == instance.get("templateId")),
        None,
    )
    if template is None or not instance.get("bindings"):
        raise CompositionError(f"{context} world sequence instance has no template or binding: {instance_id}")
    slot_id = instance["bindings"][0].get("slotId")
    track = next((row for row in template.get("tracks", []) if row.get("slotId") == slot_id), None)
    if track is None:
        raise CompositionError(f"{context} world sequence template has no transform track: {instance_id}")
    duration_ms = int(template.get("durationMs", 0))
    if sequence_local_ms < ROULETTE_HOLD_MS or sequence_local_ms > duration_ms:
        raise CompositionError(
            f"{context} roulette window ends outside the wheel sequence (local {sequence_local_ms:.0f} ms of {duration_ms})"
        )
    yaw_end, visible = _sample_track_yaw(track, sequence_local_ms)
    yaw_before, _ = _sample_track_yaw(track, sequence_local_ms - ROULETTE_HOLD_MS)
    difference = abs(((yaw_end - yaw_before) + 180.0) % 360.0 - 180.0)
    if not visible or difference > ROULETTE_HOLD_TOLERANCE_DEGREES:
        raise CompositionError(
            f"{context} roulette window does not end on a stop of the wheel "
            f"(local {sequence_local_ms:.0f} ms, yaw {yaw_before:.2f} -> {yaw_end:.2f})"
        )
    return round(yaw_end, 3)



def _rotate_y(vector: list[float], yaw: float) -> list[float]:
    angle = math.radians(yaw)
    return [math.cos(angle)*vector[0]+math.sin(angle)*vector[2], vector[1],
            -math.sin(angle)*vector[0]+math.cos(angle)*vector[2]]


def _load_region_world(root: Path, area: str, sequences: dict[str, Any], world: dict[str, Any]):
    instance = next((r for r in sequences["instances"] if r["instanceId"] == world["sequenceInstanceId"]), None)
    if instance is None or len(instance.get("bindings", [])) != 1:
        raise CompositionError("Collider WORLD anchor needs exactly one bound placement")
    binding = instance["bindings"][0]
    if binding["targetKind"] != "MAP_PLACEMENT":
        raise CompositionError("Collider WORLD anchor currently requires MAP_PLACEMENT")
    template = next(r for r in sequences["templates"] if r["sequenceId"] == instance["templateId"])
    track = next((r for r in template["tracks"] if r["slotId"] == binding["slotId"]), None)
    if track is None or not track["keys"]:
        raise CompositionError("Collider WORLD has no transform track")
    placement = None
    path = root / f"Data/Maps/Authoring/{area}/{area}.mapplacements"
    for line in path.read_text("utf-8").splitlines()[1:]:
        if line.split(" ", 1)[0] == str(binding["targetId"]):
            placement = shlex.split(line)
            break
    if placement is None:
        raise CompositionError("Collider WORLD bound placement is missing")
    position = [float(v) for v in placement[5:8]]
    base_quat = [float(v) for v in placement[8:12]]
    if abs(base_quat[0]) > 0.00001 or abs(base_quat[2]) > 0.00001:
        raise CompositionError("XZ gameplay collider WORLD anchor must be upright")
    base_yaw = _quaternion_yaw_degrees(base_quat)
    scale = [float(v) for v in placement[12:15]]
    return instance, template, track, position, base_yaw, scale


def _slerp_yaw(first, second, factor):
    dot = sum(a*b for a,b in zip(first,second))
    if dot < 0:
        second = [-v for v in second]
        dot = -dot
    dot = max(-1.0,min(1.0,dot))
    a,b = 1-factor,factor
    if dot < 0.9995:
        omega = math.acos(dot)
        a,b = math.sin((1-factor)*omega)/math.sin(omega),math.sin(factor*omega)/math.sin(omega)
    q = [x*a+y*b for x,y in zip(first,second)]
    length = math.sqrt(sum(v*v for v in q))
    return _quaternion_yaw_degrees([v/length for v in q])


def _project_region_world_track(root, area, sequences, world, box):
    instance, template, track, position, yaw, scale = _load_region_world(root,area,sequences,world)
    if not instance.get("enabled", True):
        raise CompositionError("WORLD Trigger anchor instance is disabled")
    if min(scale) <= 0 or abs(scale[0]-scale[2]) > 0.0001:
        raise CompositionError("WORLD Trigger parent scale must be positive and uniform in X/Z")
    position = [a+b for a,b in zip(position,world.get("positionOffset",[0,0,0]))]
    if world.get("anchorKind", "NONE") == "BOSS_SPAWN":
        position = [a-b for a,b in zip(position,world.get("anchorPosition",[0,0,0]))]
    keys = []
    previous = -1
    for key in _array(track["keys"],"WORLD Trigger transform keys",4096):
        time = _integer(key["timeMs"],"WORLD Trigger key timeMs",0,template["durationMs"])
        if time <= previous: raise CompositionError("WORLD Trigger keys must have increasing times")
        previous = time
        q = key["rotationQuaternion"]
        if len(q) != 4 or any(not math.isfinite(v) for v in q) or abs(q[0]) > 0.00001 or abs(q[2]) > 0.00001:
            raise CompositionError("WORLD Trigger must rotate around Y")
        length = math.sqrt(sum(v*v for v in q))
        if length <= 0.000001: raise CompositionError("WORLD Trigger quaternion is empty")
        _vector3(key["positionOffset"],"WORLD Trigger key position",-100000,100000)
        _vector3(key["scaleMultiplier"],"WORLD Trigger key scale",0,100000)
        if abs(key["scaleMultiplier"][0]-key["scaleMultiplier"][2]) > 0.0001:
            raise CompositionError("WORLD Trigger animated scale must be uniform in X/Z")
        keys.append({"timeMs":time,"positionOffset":list(key["positionOffset"]),
                     "rotationY":q[1]/length,"rotationW":q[3]/length,
                     "scaleMultiplier":list(key["scaleMultiplier"]),"visible":_boolean(key.get("visible",True),"WORLD Trigger visible")})
    return {"startMs":box["startMs"],"startDelayMs":instance.get("startDelayMs",0),"durationMs":template["durationMs"],
            "playbackSpeed":float(box["playbackSpeed"])*float(instance.get("playbackSpeed",1)),
            "interpolation":template.get("interpolation","LINEAR"),"baselinePosition":position,
            "baselineYawDegrees":yaw,"baselineScale":scale,"keys":keys}


def _sample_region_world(root: Path, area: str, sequences: dict[str, Any], world: dict[str, Any],
                         box: dict[str, Any], end_ms: int) -> tuple[list[float], float, list[float]]:
    instance, template, track, position, base_yaw, scale = _load_region_world(root,area,sequences,world)
    # Same ceil-to-fixed-tick edges as LogicRuntime and World cue admission.
    local = ((math.ceil(end_ms*30/1000)-math.ceil(box["startMs"]*30/1000))*1000/30
             - instance.get("startDelayMs", 0)) * box["playbackSpeed"] * instance.get("playbackSpeed", 1)
    local = max(0, min(local, template["durationMs"]))
    keys = track["keys"]
    first, second, factor = keys[-1], keys[-1], 0.0
    if local <= keys[0]["timeMs"]:
        first = second = keys[0]
    else:
        for left, right in zip(keys, keys[1:]):
            if left["timeMs"] <= local < right["timeMs"]:
                first, second = left, right
                factor = (local-left["timeMs"])/(right["timeMs"]-left["timeMs"])
                break
    if not first.get("visible", True):
        raise CompositionError("Collider WORLD anchor is hidden at judgement")
    if template.get("interpolation") == "SMOOTH_STEP":
        factor = factor*factor*(3-2*factor)
    def vector(key):
        return [float(a)+(float(b)-float(a))*factor for a,b in zip(first[key],second[key])]
    offset = _rotate_y(vector("positionOffset"), base_yaw)
    position = [a+b+c for a,b,c in zip(position, offset, world.get("positionOffset", [0,0,0]))]
    scale = [a*b for a,b in zip(scale, vector("scaleMultiplier"))]
    qa, qb = first["rotationQuaternion"], second["rotationQuaternion"]
    if any(abs(q[i]) > 0.00001 for q in (qa,qb) for i in (0,2)):
        raise CompositionError("XZ gameplay collider WORLD track must rotate around Y")
    yaw = base_yaw + _slerp_yaw(qa,qb,factor)
    if world.get("anchorKind", "NONE") == "BOSS_SPAWN":
        position = [a-b for a,b in zip(position,world.get("anchorPosition",[0,0,0]))]
    return position, yaw, scale


def _project_collider_regions(document, pattern, logic_box, logic, sequences, root):
    resources = {r["resourceId"]:{**PRESENTATION_RESOURCE_DEFAULTS,**r} for r in document.get("presentationResources",[])}
    kind = logic.get("judgementKind",logic.get("triggerKind"))
    candidates = [{**PRESENTATION_OCCURRENCE_DEFAULTS,**r} for r in pattern.get("presentationOccurrences",[])
                  if resources[r["resourceId"]]["kind"] == "COLLIDER"]
    if kind == "ROULETTE_CARD_MATCH":
        ids = logic.get("regionIds",[])
        if len(ids) != 8 or len(set(ids)) != 8:
            raise CompositionError("PRODUCT roulette needs eight explicit regionIds; legacy guessed sectors cannot publish")
        selected = []
        for region_id in ids:
            matches = [r for r in candidates if r["regionId"] == region_id]
            if len(matches) != 1:
                raise CompositionError(f"roulette regionId must name exactly one Collider occurrence: {region_id}")
            selected.append(matches[0])
        if {(r["cardSymbol"],r["cardColor"]) for r in selected} != {(symbol,color) for symbol in CARD_SYMBOLS for color in ("RED","BLACK")}:
            raise CompositionError("roulette regions need all eight distinct suit/color combinations")
    else:
        selected = [r for r in candidates if r["logicOccurrenceId"] == logic_box["occurrenceId"]]
        if not selected or len(selected) > 64:
            raise CompositionError("AREA_OVERLAP/ENTER_AREA needs 1..64 linked Collider occurrences")
    result = []
    for row in selected:
        resource = resources[row["resourceId"]]
        if row["logicOccurrenceId"] != logic_box["occurrenceId"]:
            raise CompositionError("roulette Collider must link the judging Logic occurrence")
        if kind == "ROULETTE_CARD_MATCH" and resource["colliderKind"] != "ROULETTE_CARD_REGION":
            raise CompositionError("roulette needs ROULETTE_CARD_REGION resources")
        if kind == "STAGGER_WINDOW" and (resource["shape"] != "SECTOR" or
                                         row["anchorKind"] != "BOSS" or row["bone"] or not row["followBoss"]):
            raise CompositionError("Shield reflection requires following boss-pivot SECTOR colliders without a bone")
        if row["rotationDegrees"][0] != 0 or row["rotationDegrees"][2] != 0:
            raise CompositionError("XZ gameplay Collider supports only Y rotation")
        position, yaw, scale = list(row["positionOffset"]), row["rotationDegrees"][1], list(row["scale"])
        anchor = "BOSS_CURRENT"
        world_track = None
        if row["anchorKind"] == "WORLD":
            world = next((w for w in document.get("worlds",[]) if w["worldId"] == row["worldId"]),None)
            world_box = next((w for w in pattern.get("worldOccurrences",[]) if w["worldId"] == row["worldId"]),None)
            if world is None or world_box is None:
                raise CompositionError("Collider WORLD needs a definition and same-pattern WORLD box")
            end = logic_box["startMs"]+logic_box["durationMs"]
            if world_box["startMs"] > logic_box["startMs"] or world_box["startMs"]+world_box["durationMs"] < end:
                raise CompositionError("Collider judgement must remain inside its WORLD box lifetime")
            if kind == "ENTER_AREA":
                world_track = _project_region_world_track(root,document["areaId"],sequences,world,world_box)
            else:
                origin, world_yaw, world_scale = _sample_region_world(root,document["areaId"],sequences,world,world_box,end)
                position = [a+b for a,b in zip(origin,_rotate_y([a*b for a,b in zip(position,world_scale)],world_yaw))]
                yaw += world_yaw
                scale = [a*b for a,b in zip(scale,world_scale)]
            anchor = "BOSS_SPAWN" if world.get("anchorKind","NONE") == "BOSS_SPAWN" else "WORLD"
        if min(scale) <= 0:
            raise CompositionError("Gameplay collider scale must be positive")
        if resource["shape"] == "SECTOR" and abs(scale[0]-scale[2]) > 0.0001:
            raise CompositionError("Circular SECTOR requires equal X/Z scale")
        result.append({"regionId":row["regionId"] or row["occurrenceId"],"shape":resource["shape"],"anchorKind":anchor,
                       "center":position,"yawDegrees":yaw,"halfExtents":[a*b for a,b in zip(resource["halfExtents"],scale)],
                       "radiusM":resource["radiusM"]*(max(scale[0],scale[2]) if resource["shape"] == "CIRCLE" else scale[0]),"halfAngleDegrees":resource["halfAngleDegrees"],
                       "cardSymbol":row["cardSymbol"] if kind == "ROULETTE_CARD_MATCH" else "NONE",
                       "cardColor":row["cardColor"] if kind == "ROULETTE_CARD_MATCH" else "NONE"})
        if world_track is not None:
            result[-1]["worldTrack"] = world_track
    return result


def _project_outcomes(logics: dict[str, dict[str, Any]], targets: list[str]) -> list[dict[str, Any]]:
    projected: list[dict[str, Any]] = []
    for target in targets:
        logic = logics[target]
        projected.append({
            "kind": logic["outcomeKind"],
            "percent": int(logic.get("percent", 0)),
            "durationMs": int(logic.get("durationMs", 0)),
            "patternId": logic.get("followupPatternId", ""),
        })
    return projected


def _project_logic_window(
    box: dict[str, Any],
    logic: dict[str, Any],
    logics: dict[str, dict[str, Any]],
    stop_yaw_degrees: float,
) -> dict[str, Any]:
    kind = logic.get("judgementKind", logic.get("triggerKind"))
    return {
        "windowId": box["occurrenceId"],
        "kind": kind,
        "startMs": box["startMs"],
        "durationMs": box["durationMs"],
        "sectorCount": int(logic.get("sectorCount", 0)) if kind == "ROULETTE_CARD_MATCH" else 0,
        "sectorSymbols": list(logic.get("sectorSymbols", [])) if kind == "ROULETTE_CARD_MATCH" else [],
        "centerX": float(logic.get("centerX", 0.0)) if kind == "ROULETTE_CARD_MATCH" else 0.0,
        "centerZ": float(logic.get("centerZ", 0.0)) if kind == "ROULETTE_CARD_MATCH" else 0.0,
        "outerRadiusM": float(logic.get("outerRadiusM", 0.0)) if kind == "ROULETTE_CARD_MATCH" else 0.0,
        "stopYawDegrees": stop_yaw_degrees,
        "normalYawOffsetDegrees": float(logic.get("normalYawOffsetDegrees", 0.0)),
        "insideOutcome": logic.get("insideOutcome", "SUCCESS"),
        "cardRegions": [],
        "halfAngleDegrees": float(logic.get("halfAngleDegrees", 0.0)) if kind == "GAZE_REAL_BOSS" else 0.0,
        "maxDistanceM": float(logic.get("maxDistanceM", 0.0)) if kind == "GAZE_REAL_BOSS" else 0.0,
        "poseIndex": int(logic.get("poseIndex", 0)) if kind == "POSE_INPUT" else 0,
        "threshold": int(logic.get("threshold", 0)) if kind == "STAGGER_WINDOW" else 0,
        "shieldArcDegrees": float(logic.get("shieldArcDegrees", 0.0)) if kind == "STAGGER_WINDOW" else 0.0,
        "endsPatternOnSuccess": bool(logic.get("endsPatternOnSuccess", False)) if kind == "STAGGER_WINDOW" else False,
        "onSuccess": _project_outcomes(logics, outcome_logic_ids(box, "Success")),
        "onFail": _project_outcomes(logics, outcome_logic_ids(box, "Fail")),
        "onTimeout": _project_outcomes(logics, outcome_logic_ids(box, "Timeout")),
    }


def project_encounter(document: dict[str, Any], root: Path = REPOSITORY_ROOT) -> dict[str, Any]:
    arena_archetypes_by_profile = arena_boss_archetypes_by_profile(root)
    logics = {logic["logicId"]: logic for logic in document.get("logics", [])}
    worlds = {world["worldId"]: world for world in document.get("worlds", [])}
    scene_profiles = {row["sceneProfileId"]: row for row in document.get("sceneProfiles", [])}
    world_sequences: dict[str, Any] | None = None
    patterns: list[dict[str, Any]] = []
    for source in document["patterns"]:
        if source["authoringStatus"] != "PRODUCT":
            continue
        boss_archetype_ids = list(
            arena_archetypes_by_profile.get(_pattern_actor_profile_id(source), [])
        )
        source_action_ids = list(
            dict.fromkeys(
                occurrence["sourceActionId"]
                for stage in source["stages"]
                for occurrence in stage["animationOccurrences"]
                if not (occurrence["sourceActionId"] == 0 and occurrence["sourceStageId"] == "RAW")
            )
        )
        world_boxes = source.get("worldOccurrences", [])
        world_by_instance = {
            worlds[box["worldId"]]["sequenceInstanceId"]: box for box in world_boxes
        }
        if world_boxes:
            if world_sequences is None:
                world_sequences = load_world_sequences(root, document["areaId"])
            instances_by_id = {row["instanceId"]: row for row in world_sequences.get("instances", [])}
            templates_by_id = {row["sequenceId"]: row for row in world_sequences.get("templates", [])}
            objects_by_id = {row["objectId"]: row for row in world_sequences.get("objectResources", [])}
            for instance_id in world_by_instance:
                instance = instances_by_id.get(instance_id)
                if instance is None or instance.get("templateId") not in templates_by_id:
                    raise CompositionError(f"{source['patternId']} references an unknown saved World Object state: {instance_id}")
                for binding in instance.get("bindings", []):
                    if binding.get("targetKind") != "OBJECT_RESOURCE":
                        continue
                    resource = objects_by_id.get(binding.get("targetId"))
                    if resource is None or not resource.get("modelAssetId") or resource.get("sequenceInstanceId"):
                        raise CompositionError(f"{source['patternId']} World Object state has an unresolved model resource: {instance_id}")
        logic_windows: list[dict[str, Any]] = []
        for box in source.get("logicOccurrences", []):
            if not box.get("enabled", True):
                continue
            logic = logics[box["logicId"]]
            kind = logic.get("judgementKind", logic.get("triggerKind"))
            if logic["logicType"] != "DURATION" and kind != "ENTER_AREA":
                continue
            window = _project_logic_window(box, logic, logics, 0.0)
            linked_shields = kind == "STAGGER_WINDOW" and any(
                row.get("logicOccurrenceId") == box["occurrenceId"]
                for row in source.get("presentationOccurrences", []))
            if kind in {"ROULETTE_CARD_MATCH", "AREA_OVERLAP", "ENTER_AREA"} or linked_shields:
                if world_sequences is None:
                    world_sequences = load_world_sequences(root, document["areaId"])
                window["cardRegions"] = _project_collider_regions(document, source, box, logic, world_sequences, root)
            logic_windows.append(window)
        mechanic_triggers = []
        for box in source.get("logicOccurrences", []):
            if not box.get("enabled", True):
                continue
            logic = logics[box["logicId"]]
            if logic["logicType"] != "TRIGGER" or "triggerKind" not in logic or logic["triggerKind"] == "ENTER_AREA":
                continue
            clone_id = logic.get("clonePatternId", "")
            if clone_id and not any(p["patternId"] == clone_id and p["authoringStatus"] == "PRODUCT"
                                    for p in document["patterns"]):
                raise CompositionError(f"{box['occurrenceId']} clone pattern must be PRODUCT: {clone_id}")
            mechanic_triggers.append({
                "triggerId": box["occurrenceId"], "kind": logic["triggerKind"],
                "startMs": box["startMs"], "durationMs": box["durationMs"],
                "hudMode": logic.get("hudMode", "NONE"),
                "teleportPosition": logic.get("teleportPosition", [0.0, 0.0, 0.0]),
                "clonePatternId": clone_id, "clockHours": logic.get("clockHours", []),
                "faceCenterYawOffsetDegrees": float(logic.get("faceCenterYawOffsetDegrees", 0.0)),
            })
        patterns.append(
            {
                "patternId": source["patternId"],
                "category": source["category"],
                "minimumPhase": 1,
                "maximumPhase": 1,
                "targetPolicy": "NONE",
                "aimPolicy": "NONE",
                "displayName": source["displayName"],
                "actionId": _pattern_action_id(source["patternId"]),
                "sourceActionIds": source_action_ids,
                # Arena boss bodies this pattern's clips belong to; the Server
                # audition plays it only on a live boss of one of them.
                "bossArchetypeIds": boss_archetype_ids,
                "selectionMode": "AUDITION_ONLY",
                "minimumHealthBar": 0,
                "maximumHealthBar": 0,
                "triggerHealthBar": 0,
                "triggerOrder": 0,
                "armorRequirement": "ANY",
                "phaseRequirement": "ANY",
                "invulnerableWhileRunning": False,
                "selectionWeight": 0,
                "maximumConsecutiveUses": 0,
                # The v33 PATTERN row requires an ordered range even though an
                # untargeted audition never consumes it.
                "minimumRange": 0.0,
                "maximumRange": 1.0,
                "stages": [_project_stage(stage) for stage in source["stages"]],
                # Pattern-clock lanes beside the stages: judgement windows and
                # the presentation cues the Server broadcasts.
                "resetBossToSpawn": source.get("resetBossToSpawn", False),
                "logicWindows": logic_windows,
                "mechanicTriggers": mechanic_triggers,
                "worldSequences": [
                    {
                        "sequenceInstanceId": worlds[box["worldId"]]["sequenceInstanceId"],
                        "startMs": box["startMs"],
                        "durationMs": box["durationMs"],
                        "playbackSpeed": float(box["playbackSpeed"]),
                        "positionOffset": list(worlds[box["worldId"]].get("positionOffset", [0.0, 0.0, 0.0])),
                        "anchorKind": worlds[box["worldId"]].get("anchorKind", "NONE"),
                        "anchorPosition": list(worlds[box["worldId"]].get("anchorPosition", [0.0, 0.0, 0.0])),
                    }
                    for box in world_boxes
                ],
                "sceneProfiles": [
                    {
                        "renderingProfileId": scene_profiles[box["sceneProfileId"]]["renderingProfileId"],
                        "startMs": box["startMs"],
                        "durationMs": box["durationMs"],
                        "blendMs": box["blendMs"],
                    }
                    for box in source.get("sceneProfileOccurrences", [])
                ],
            }
        )
    return {
        "schema": "lostark.encounter-profile",
        "formatVersion": 4,
        "encounterId": document["encounterId"],
        "bossArchetypeId": document["bossArchetypeId"],
        "authority": "server",
        "fixedTickHz": document["fixedTickHz"],
        "sourceRevision": document["revision"],
        "madnessPolicy": dict(document.get("madnessPolicy", DEFAULT_MADNESS_POLICY)),
        "playAllPatternIds": list(document["playAllPatternIds"]),
        "patterns": patterns,
    }


PRESENTATION_RESOURCE_REQUIRED = {"resourceId", "displayName", "kind", "assetId"}
PRESENTATION_RESOURCE_DEFAULTS = {
    "resourceKind": "GROUP", "durationMs": 1000, "shape": "BOX", "colliderKind": "GEOMETRY",
    "halfExtents": [1.0, 1.0, 1.0], "radiusM": 3.0, "halfAngleDegrees": 45.0,
    "defaultAnchorKind": "BOSS",
}
PRESENTATION_OCCURRENCE_REQUIRED = {"occurrenceId", "resourceId", "startMs", "durationMs"}
PRESENTATION_OCCURRENCE_DEFAULTS = {
    "positionOffset": [0.0, 0.0, 0.0], "rotationDegrees": [0.0, 0.0, 0.0], "scale": [1.0, 1.0, 1.0],
    "fadeInMs": 0, "fadeOutMs": 0, "dissolveStart": 0.85, "dissolveEnd": 1.0,
    "volume": 1.0, "followBoss": True, "bone": "", "brightnessMultiplier": 1.0,
    "regionId": "", "cardSymbol": "NONE", "cardColor": "NONE",
    "anchorKind": "BOSS", "worldId": "", "logicOccurrenceId": "", "debugRender": True, "worldOccurrenceId": "",
}


def _vector3(value: Any, context: str, minimum: float, maximum: float) -> None:
    if not isinstance(value, list) or len(value) != 3:
        raise CompositionError(f"{context} needs three coordinates")
    for coordinate in value:
        _number(coordinate, context, minimum, maximum)


def _validate_presentation_resources(document: dict[str, Any]) -> dict[str, dict[str, Any]]:
    resources = _array(document.get("presentationResources", []), "presentationResources", 4096)
    next_id = _integer(document.get("nextPresentationResourceOrdinal", 1), "nextPresentationResourceOrdinal", 1, 1000000)
    result = {}
    for resource in resources:
        _keys(resource, PRESENTATION_RESOURCE_REQUIRED, set(PRESENTATION_RESOURCE_DEFAULTS), "presentation resource")
        resource_id = _stable_id(resource["resourceId"], "presentation resourceId")
        if resource_id in result:
            raise CompositionError(f"duplicate presentation resourceId: {resource_id}")
        generated = re.fullmatch(r"kakulsaydon\.g1\.presentation\.([1-9][0-9]*)", resource_id)
        if generated and int(generated.group(1)) >= next_id:
            raise CompositionError("presentation resourceId is ahead of nextPresentationResourceOrdinal")
        _display_name(resource["displayName"], "presentation displayName")
        normalized = {**PRESENTATION_RESOURCE_DEFAULTS, **resource}
        kind = normalized["kind"]
        if kind not in {"EFFECT", "SOUND", "CAMERA", "COLLIDER", "LIGHT"}:
            raise CompositionError("presentation resource kind is unsupported")
        asset = normalized["assetId"]
        if kind == "COLLIDER":
            if asset != "":
                raise CompositionError("COLLIDER assetId must be empty")
        elif kind in {"CAMERA", "LIGHT"}:
            _stable_id(asset, f"{kind} assetId")
        else:
            _string(asset, "presentation assetId", 512)
            if asset.startswith("/") or "\\" in asset or ":" in asset or ".." in asset.split("/"):
                raise CompositionError("presentation assetId must be Resources-relative")
            if kind == "SOUND" and not asset.startswith("Sound/"):
                raise CompositionError("SOUND assetId must begin with Sound/")
        if normalized["defaultAnchorKind"] not in {"MAP", "PLAYER", "BOSS"}:
            raise CompositionError("presentation defaultAnchorKind is unsupported")
        if kind == "LIGHT" and normalized["resourceKind"] != "":
            raise CompositionError("LIGHT resourceKind must be empty")
        if normalized["resourceKind"] not in ({"GROUP", "LEAF"} if kind == "EFFECT" else {"", "GROUP", "LEAF"}):
            raise CompositionError("presentation resourceKind is unsupported")
        if normalized["colliderKind"] not in {"GEOMETRY", "ROULETTE_CARD_REGION"}:
            raise CompositionError("presentation colliderKind is unsupported")
        if normalized["shape"] not in {"BOX", "SECTOR", "CIRCLE"}:
            raise CompositionError("presentation collider shape is unsupported")
        _integer(normalized["durationMs"], "presentation durationMs", 1, MAX_TIMELINE_MS)
        _vector3(normalized["halfExtents"], "collider halfExtents", 0.001, 100000)
        _number(normalized["radiusM"], "collider radiusM", 0.001, 100000)
        _number(normalized["halfAngleDegrees"], "collider halfAngleDegrees", 0.001, 180)
        result[resource_id] = normalized
    return result


def _validate_presentation_occurrences(pattern: dict[str, Any], resources: dict[str, Any], duration: int, worlds: dict[str, Any]) -> None:
    boxes = _array(pattern.get("presentationOccurrences", []), "presentationOccurrences", 1024)
    next_id = _integer(pattern.get("nextPresentationOccurrenceOrdinal", 1), "nextPresentationOccurrenceOrdinal", 1, 1000000)
    ids = set()
    linked_world_ids = set()
    prefix = re.escape(pattern["patternId"]) + r"\.presentation\.([1-9][0-9]*)"
    for box in boxes:
        _keys(box, PRESENTATION_OCCURRENCE_REQUIRED, set(PRESENTATION_OCCURRENCE_DEFAULTS), "presentation occurrence")
        generated = re.fullmatch(prefix, box["occurrenceId"])
        if not generated or int(generated.group(1)) >= next_id or box["occurrenceId"] in ids:
            raise CompositionError("presentation occurrenceId must belong to this pattern and its next ordinal")
        ids.add(box["occurrenceId"])
        if box["resourceId"] not in resources:
            raise CompositionError("presentation occurrence names an unknown resourceId")
        normalized = {**PRESENTATION_OCCURRENCE_DEFAULTS, **box}
        world_occurrence_id = normalized["worldOccurrenceId"]
        if world_occurrence_id != "":
            _stable_id(world_occurrence_id, "presentation worldOccurrenceId")
            owner = next((row for row in pattern.get("worldOccurrences", []) if row["occurrenceId"] == world_occurrence_id), None)
            if owner is None or resources[box["resourceId"]]["kind"] != "EFFECT":
                raise CompositionError("worldOccurrenceId requires an EFFECT and a same-pattern World box")
            if worlds[owner["worldId"]].get("companionEffectResourceId", "") != box["resourceId"]:
                raise CompositionError("linked Effect must match its World companionEffectResourceId")
            if world_occurrence_id in linked_world_ids:
                raise CompositionError("a World box can have at most one linked companion Effect")
            linked_world_ids.add(world_occurrence_id)
        if normalized["cardSymbol"] not in {"NONE", *CARD_SYMBOLS} or normalized["cardColor"] not in {"NONE", "RED", "BLACK"}:
            raise CompositionError("collider card mapping is invalid")
        light = resources[box["resourceId"]]["kind"] == "LIGHT"
        if normalized["anchorKind"] not in ({"BOSS", "PLAYER", "MAP"} if light else {"BOSS", "WORLD"}):
            raise CompositionError("presentation anchorKind is invalid")
        if light and (normalized["scale"] != [1.0, 1.0, 1.0] or normalized["worldId"] or
                      normalized["logicOccurrenceId"] or
                      (normalized["anchorKind"] != "BOSS" and normalized["bone"]) or
                      (normalized["anchorKind"] == "PLAYER" and not normalized["followBoss"])):
            raise CompositionError("LIGHT requires unit scale, a direct anchor and Character follow without a bone")
        if not light and normalized["brightnessMultiplier"] != 1.0:
            raise CompositionError("brightnessMultiplier belongs only to LIGHT")
        for field in ("regionId", "worldId", "logicOccurrenceId"):
            if normalized[field]: _stable_id(normalized[field], f"collider {field}")
        if normalized["logicOccurrenceId"]:
            owner = next((row for row in pattern.get("logicOccurrences", []) if row["occurrenceId"] == normalized["logicOccurrenceId"]), None)
            if owner is None:
                raise CompositionError("collider logicOccurrenceId names an unknown Logic box")
            if owner["startMs"] != normalized["startMs"] or owner["durationMs"] != normalized["durationMs"]:
                raise CompositionError("linked Collider and Logic windows must have identical timing")
        start = _integer(normalized["startMs"], "presentation startMs", 0, MAX_TIMELINE_MS)
        play = _integer(normalized["durationMs"], "presentation durationMs", 1, MAX_TIMELINE_MS)
        if start + play > duration:
            raise CompositionError("presentation occurrence exceeds the Pattern lifetime")
        for key in ("positionOffset", "rotationDegrees"):
            _vector3(normalized[key], f"presentation {key}", -100000, 100000)
        _vector3(normalized["scale"], "presentation scale", 0.001, 100000)
        fade_in = _integer(normalized["fadeInMs"], "presentation fadeInMs", 0, play)
        fade_out = _integer(normalized["fadeOutMs"], "presentation fadeOutMs", 0, play)
        if fade_in + fade_out > play:
            raise CompositionError("presentation fades exceed the occurrence duration")
        dissolve_start = _number(normalized["dissolveStart"], "presentation dissolveStart", 0, 1)
        dissolve_end = _number(normalized["dissolveEnd"], "presentation dissolveEnd", 0, 1)
        if dissolve_start >= dissolve_end:
            raise CompositionError("presentation dissolve interval is empty or reversed")
        _number(normalized["volume"], "presentation volume", 0, 1)
        _number(normalized["brightnessMultiplier"], "presentation brightnessMultiplier", 0, 16)
        _boolean(normalized["followBoss"], "presentation followBoss")
        _boolean(normalized["debugRender"], "presentation debugRender")
        if not isinstance(normalized["bone"], str) or len(normalized["bone"]) > 128 or "\0" in normalized["bone"]:
            raise CompositionError("presentation bone is invalid")


def _project_pattern_presentation(document: dict[str, Any], pattern: dict[str, Any]) -> dict[str, Any]:
    resources = {resource["resourceId"]: resource for resource in document.get("presentationResources", [])}
    occurrences = []
    for box in pattern.get("presentationOccurrences", []):
        resource = {**PRESENTATION_RESOURCE_DEFAULTS, **resources[box["resourceId"]]}
        occurrences.append({
            **{key: value for key, value in resource.items() if key not in {"displayName", "durationMs", "defaultAnchorKind"}},
            "resourceDurationMs": resource["durationMs"], **{key: value for key, value in PRESENTATION_OCCURRENCE_DEFAULTS.items() if key != "brightnessMultiplier"}, **box,
            **({"brightnessMultiplier": box.get("brightnessMultiplier", 1.0)} if resource["kind"] == "LIGHT" else {}),
            "worldSequenceInstanceId": next((w["sequenceInstanceId"] for w in document.get("worlds", []) if w["worldId"] == box.get("worldId", "")), ""),
        })
    profiles = {profile["sceneProfileId"]: profile for profile in document.get("sceneProfiles", [])}
    for box in pattern.get("sceneProfileOccurrences", []):
        occurrences.append({
            **{key: value for key, value in PRESENTATION_RESOURCE_DEFAULTS.items() if key not in {"durationMs", "defaultAnchorKind"}},
            **{key: value for key, value in PRESENTATION_OCCURRENCE_DEFAULTS.items() if key != "brightnessMultiplier"},
            "occurrenceId": box["occurrenceId"], "resourceId": box["sceneProfileId"],
            "kind": "SCENE_PROFILE", "worldSequenceInstanceId": "", "assetId": profiles[box["sceneProfileId"]]["renderingProfileId"],
            "resourceDurationMs": box["durationMs"], "startMs": box["startMs"], "durationMs": box["durationMs"],
            "fadeInMs": box["blendMs"],
        })
    return {"patternId": pattern["patternId"], "durationMs": sum(stage["durationMs"] for stage in pattern["stages"]),
            "presentationOccurrences": occurrences}


def _join_light_resources(document: dict[str, Any], root: Path) -> int | None:
    resources = {r["resourceId"]: r for r in document.get("presentationResources", []) if r["kind"] == "LIGHT"}
    used = {resources[box["resourceId"]]["assetId"] for pattern in document["patterns"]
            if pattern.get("authoringStatus") == "PRODUCT"
            for box in pattern.get("presentationOccurrences", []) if box["resourceId"] in resources}
    if not used:
        return None
    try:
        catalog = validate_light_resources(load_json(root / LIGHT_RESOURCES_PATH))
    except LightValidationError as error:
        raise CompositionError(f"Light Resource catalog is invalid: {error}") from error
    revision = catalog["revision"]
    ids = {light["lightResourceId"] for light in catalog["lights"]}
    map_catalog_path = root / "Data/Maps/MapCatalog.json"
    if map_catalog_path.is_file():
        map_catalog = load_json(map_catalog_path)
        area = next((row for row in map_catalog.get("areas", []) if row.get("id") == document["areaId"]), {})
        source = area.get("sourceLights", "")
        if bool(source) != bool(area.get("lights", "")):
            raise CompositionError("Map light source/runtime pair is incomplete")
        if source:
            if not isinstance(source, str) or not source.startswith("Data/Maps/Authoring/") or ":" in source or "\\" in source or ".." in source.split("/"):
                raise CompositionError("Map light source path is invalid")
            map_lights = load_json(root / source)
            if map_lights.get("schema") != "lostark.map-light-presentation" or map_lights.get("areaId") != document["areaId"]:
                raise CompositionError("Map Light catalog schema or area is incompatible")
            # Legacy v1 source remains a read-only Area layer, never an Append resource.
            if map_lights.get("formatVersion") == 2:
                try:
                    validate_map_lights_v2(map_lights, document["areaId"])
                except LightValidationError as error:
                    raise CompositionError(f"Map Light catalog is invalid: {error}") from error
                for light in _array(map_lights.get("lights"), "Map lights", 64):
                    identity = _stable_id(light.get("lightId"), "Map lightId")
                    if identity in ids:
                        raise CompositionError(f"duplicate lightResourceId across Map and catalog: {identity}")
                    ids.add(identity)
    missing = sorted(used - ids)
    if missing:
        raise CompositionError("LIGHT references missing Light Resources: " + ", ".join(missing))
    return revision


def project_presentation(document: dict[str, Any], root: Path = REPOSITORY_ROOT) -> dict[str, Any]:
    light_revision = _join_light_resources(document, root)
    bindings: list[dict[str, Any]] = []
    for pattern in document["patterns"]:
        if pattern["authoringStatus"] != "PRODUCT":
            continue
        for stage in pattern["stages"]:
            for occurrence in stage["animationOccurrences"]:
                bindings.append(
                    {
                        "actionId": stage["actionId"],
                        "occurrenceId": occurrence["occurrenceId"],
                        "clip": occurrence["runtimeClip"],
                        "startOffsetMs": occurrence["startOffsetMs"],
                        "sourceStartMs": occurrence["sourceStartMs"],
                        "playMs": occurrence["playMs"],
                        "playRate": occurrence["playRate"],
                        "endPolicy": occurrence["endPolicy"],
                    }
                )
    return {
        "schema": "lostark.kouku-saydon-pattern-bindings",
        "formatVersion": 1,
        **({"lightResourceRevision": light_revision} if light_revision is not None else {}),
        "bossArchetypeId": document["bossArchetypeId"],
        "sourceRevision": document["revision"],
        "bindings": bindings,
        "patterns": [_project_pattern_presentation(document, pattern) for pattern in document["patterns"]
                     if pattern["authoringStatus"] == "PRODUCT"],
    }


def serialize_json(document: dict[str, Any]) -> bytes:
    return (
        json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    ).encode("utf-8")


def projected_outputs(document: dict[str, Any], root: Path = REPOSITORY_ROOT) -> dict[Path, bytes]:
    return {
        ENCOUNTER_PATH: serialize_json(project_encounter(document, root)),
        PRESENTATION_PATH: serialize_json(project_presentation(document, root)),
    }


def validate_outputs(root: Path, expected: dict[Path, bytes]) -> None:
    for relative, content in expected.items():
        path = root / relative
        try:
            actual = path.read_bytes()
        except OSError as error:
            raise CompositionError(f"missing projected Product: {relative}") from error
        if actual != content:
            raise CompositionError(f"projected Product is stale: {relative}")
        # Reparse with duplicate-key rejection after byte parity so a writer
        # regression cannot be hidden by Python's permissive default parser.
        load_json(path)


def publish_outputs(root: Path, outputs: dict[Path, bytes]) -> None:
    transaction_id = uuid.uuid4().hex
    staged: dict[Path, Path] = {}
    backups: dict[Path, Path | None] = {}
    promoted: list[Path] = []
    preserved_backups: set[Path] = set()
    try:
        for relative, content in outputs.items():
            destination = root / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            descriptor, temporary_name = tempfile.mkstemp(
                prefix=f".{destination.name}.staging.{transaction_id}.",
                dir=destination.parent,
            )
            temporary = Path(temporary_name)
            with os.fdopen(descriptor, "wb") as stream:
                stream.write(content)
                stream.flush()
                os.fsync(stream.fileno())
            load_json(temporary)
            staged[destination] = temporary
            if destination.exists():
                backup = destination.with_name(
                    f".{destination.name}.rollback.{transaction_id}"
                )
                shutil.copy2(destination, backup)
                backups[destination] = backup
            else:
                backups[destination] = None
        for destination, temporary in staged.items():
            os.replace(temporary, destination)
            promoted.append(destination)
        validate_outputs(root, outputs)
    except Exception as publish_error:
        rollback_failures: list[str] = []
        for destination in reversed(promoted):
            backup = backups.get(destination)
            try:
                if backup is None:
                    destination.unlink(missing_ok=True)
                elif backup.exists():
                    os.replace(backup, destination)
            except OSError as rollback_error:
                if backup is not None and backup.exists():
                    preserved_backups.add(backup)
                    recovery = str(backup)
                else:
                    recovery = f"no recovery backup for {destination}"
                rollback_failures.append(f"{recovery} ({rollback_error})")
        if rollback_failures:
            raise CompositionError(
                "KoukuSaydon Product publish failed and rollback was incomplete; "
                "recovery backup(s) preserved: " + "; ".join(rollback_failures)
            ) from publish_error
        raise
    finally:
        for temporary in staged.values():
            temporary.unlink(missing_ok=True)
        for backup in backups.values():
            if backup is not None and backup not in preserved_backups:
                backup.unlink(missing_ok=True)


def run(root: Path, mode: str) -> dict[str, Any]:
    document = load_and_validate(root)
    validate_publishable(document, root)
    outputs = projected_outputs(document, root)
    if mode == "publish":
        publish_outputs(root, outputs)
    else:
        validate_outputs(root, outputs)
    return {
        "compositionId": COMPOSITION_ID,
        "sourceRevision": document["revision"],
        "productPatternCount": len(document["playAllPatternIds"]),
        "productStageCount": sum(
            len(pattern["stages"])
            for pattern in document["patterns"]
            if pattern["authoringStatus"] == "PRODUCT"
        ),
        "outputCount": len(outputs),
    }


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repository-root", type=Path, default=REPOSITORY_ROOT
    )
    parser.add_argument(
        "--mode", choices=("validate", "publish"), default="validate"
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="alias for --mode validate",
    )
    return parser


def main(arguments: Iterable[str] | None = None) -> int:
    options = _parser().parse_args(arguments)
    mode = "validate" if options.check else options.mode
    try:
        summary = run(options.repository_root.resolve(), mode)
    except CompositionError as error:
        print(f"KoukuSaydon composition {mode} failed: {error}", file=sys.stderr)
        return 1
    print(json.dumps(summary, ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
