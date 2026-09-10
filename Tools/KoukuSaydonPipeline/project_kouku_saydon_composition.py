#!/usr/bin/env python3
"""Validate the Gate 1 KoukuSaydon composition and project its Product views.

The composition document is the only directly-authored owner.  Runtime-facing
encounter and animation binding documents are deterministic projections of the
ready rows. The saved hierarchy remains visible even when a row cannot execute.
"""

from __future__ import annotations

import argparse
import copy
import json
import math
import os
from pathlib import Path
import re
import shutil
import shlex
import sys
import struct
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
from Tools.ModelAssetConverter import verify_dimensionmaster_summon_bind_pose as wmodel_pose

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
FORMAT_VERSION = 3
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
    "nextFolderOrdinal", "nextBundleOrdinal", "folders", "bundles",
}
PATTERN_OPTIONAL_KEYS = {
    "nextLogicOccurrenceOrdinal",
    "logicOccurrences",
    "nextSummonOccurrenceOrdinal",
    "summonOccurrences",
    "nextWorldOccurrenceOrdinal",
    "worldOccurrences",
    "nextSceneProfileOccurrenceOrdinal",
    "sceneProfileOccurrences", "resetBossToSpawn", "resetBossYawDegrees", "bossMotion", "animationRootVerticalScale",
    "presentationOccurrences", "nextPresentationOccurrenceOrdinal", "gateId", "targetBossPlacementId", "folderId",
}
LOGIC_KEYS = {"logicId", "displayName", "logicType"}
# Typed judgement values live on the DURATION definition, typed outcome values
# on the RESULT definition. Which value keys a kind may carry is exact.
LOGIC_KIND_VALUE_KEYS = {
    "ROULETTE_CARD_MATCH": {
        "sectorCount", "sectorSymbols", "centerX", "centerZ", "outerRadiusM",
        "worldSequenceInstanceId", "regionIds",
    },
    "GAZE_REAL_BOSS": {"halfAngleDegrees", "maxDistanceM", "insideOutcome"},
    "POSE_INPUT": {"poseIndex"},
    "STAGGER_WINDOW": {"threshold", "shieldArcDegrees", "endsPatternOnSuccess", "normalYawOffsetDegrees"},
    "AREA_OVERLAP": {"insideOutcome"},
    "OBJECT_OVERLAP": {"targetWorldInstanceId", "targetRadiusM", "insideOutcome"},
    "EXTERNAL_SIGNAL": {"endsPatternOnSuccess"},
    "COUNTER_WINDOW": {"endsPatternOnSuccess"},
    "ATTACHMENT_HOLD": set(),
}
LOGIC_DURATION_VALUE_KEYS = {"judgementKind"} | set().union(*LOGIC_KIND_VALUE_KEYS.values())
LOGIC_RESULT_VALUE_KEYS = {"outcomeKind", "percent", "durationMs", "followupPatternId", "targetWorldInstanceId", "motionInstanceId",
                           "contactMotions", "targetLogicOccurrenceId", "contactTargetWorldOccurrenceId", "sceneProfileId", "effectResourceId", "lightResourceId", "effectDelayMs", "attachmentSlot", "gripLocalOffset", "pushRangeM", "pushMs"}
LOGIC_TRIGGER_VALUE_KEYS = {"triggerKind", "hudMode", "teleportPosition", "clonePatternId", "clockHours", "faceCenterYawOffsetDegrees",
                            "targetWorldOccurrenceIds", "targetRadiusM", "contactGroupId", "contactPriority", "bossChargeDistanceM", "chargeYawOffsetDegrees", "rearmOnExit", "repeatAfterKnockback"}
LOGIC_OPTIONAL_KEYS = LOGIC_DURATION_VALUE_KEYS | LOGIC_RESULT_VALUE_KEYS | LOGIC_TRIGGER_VALUE_KEYS
JUDGEMENT_KINDS = set(LOGIC_KIND_VALUE_KEYS)
# End-tick kinds judge once when the window closes: Success or Fail, never
# Timeout. The boss-level stagger window has no wrong answer, so no Fail.
END_TICK_KINDS = {"GAZE_REAL_BOSS"}
OUTCOME_KINDS = {
    "INSTANT_DEATH", "MAX_HP_PERCENT_DAMAGE", "MADNESS_GAUGE_ADD_PERCENT",
    "CLOWN_TRANSFORM", "FEAR", "FOLLOWUP_PATTERN", "PLAY_WORLD_OBJECT_MOTION",
    "PLAY_CONTACT_WORLD_OBJECT_MOTION", "COMPLETE_LOGIC_WINDOW", "CAPTURE_PLAYER",
    # Hangs the player on the region that judged them and drags them with it.
    "GRAB_TO_WORLD_OBJECT",
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
    "holdLogicOccurrenceId",
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
MAX_WORLD_OCCURRENCES_PER_PATTERN = 128
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
            if logic.get("insideOutcome", "SUCCESS") not in {"SUCCESS", "FAIL"}:
                raise CompositionError(f"{context} insideOutcome must be SUCCESS or FAIL")
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
        elif kind == "OBJECT_OVERLAP":
            if logic.get("insideOutcome", "SUCCESS") not in {"SUCCESS", "FAIL"}:
                raise CompositionError(f"{context} insideOutcome must be SUCCESS or FAIL")
            definition["targetWorldInstanceId"] = _stable_id(logic.get("targetWorldInstanceId", ""), f"{context} targetWorldInstanceId")
            definition["targetRadiusM"] = _number(logic.get("targetRadiusM", 0), f"{context} targetRadiusM", .01, 1000)
        elif kind == "AREA_OVERLAP":
            if logic.get("insideOutcome", "SUCCESS") not in {"SUCCESS", "FAIL"}:
                raise CompositionError(f"{context} insideOutcome must be SUCCESS or FAIL")
        elif kind in {"EXTERNAL_SIGNAL", "COUNTER_WINDOW"}:
            definition["endsPatternOnSuccess"] = _boolean(
                logic.get("endsPatternOnSuccess", False), f"{context} endsPatternOnSuccess")
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
            if extra - {"bossChargeDistanceM", "chargeYawOffsetDegrees", "rearmOnExit", "repeatAfterKnockback"} != {"triggerKind"}:
                raise CompositionError(f"{context} ENTER_AREA carries unrelated values")
            definition["bossChargeDistanceM"] = _number(logic.get("bossChargeDistanceM", 0), f"{context} bossChargeDistanceM", 0, 1000)
            definition["chargeYawOffsetDegrees"] = _number(logic.get("chargeYawOffsetDegrees", 0), f"{context} chargeYawOffsetDegrees", -360, 360)
            if definition["chargeYawOffsetDegrees"] and not definition["bossChargeDistanceM"]:
                raise CompositionError(f"{context} charge yaw requires positive charge distance")
            definition["rearmOnExit"] = _boolean(logic.get("rearmOnExit", False), f"{context} rearmOnExit")
            definition["repeatAfterKnockback"] = _boolean(logic.get("repeatAfterKnockback", False), f"{context} repeatAfterKnockback")
            if definition["rearmOnExit"] and definition["repeatAfterKnockback"]:
                raise CompositionError(f"{context} choose one contact repeat policy")
        elif kind == "OBJECT_CONTACT":
            required = {"triggerKind", "targetWorldOccurrenceIds", "targetRadiusM"}
            if not required <= extra or extra - required - {"contactGroupId", "contactPriority"}:
                raise CompositionError(f"{context} OBJECT_CONTACT values are incomplete or unrelated")
            targets = _array(logic["targetWorldOccurrenceIds"], f"{context} targetWorldOccurrenceIds", 64)
            for target in targets:
                _stable_id(target, f"{context} targetWorldOccurrenceId")
            if not targets or len(set(targets)) != len(targets):
                raise CompositionError(f"{context} OBJECT_CONTACT requires 1..64 distinct target WORLD occurrences")
            _number(logic["targetRadiusM"], f"{context} targetRadiusM", .01, 1000)
            if "contactGroupId" in logic:
                _stable_id(logic["contactGroupId"], f"{context} contactGroupId")
            _integer(logic.get("contactPriority", 0), f"{context} contactPriority", 0, 1000)
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
        if kind not in {"CLOWN_TRANSFORM", "FEAR"} and duration_ms != 0:
            raise CompositionError(f"{context} {kind} does not take a durationMs")
        if extra & {"pushRangeM", "pushMs"}:
            if kind != "MAX_HP_PERCENT_DAMAGE":
                raise CompositionError(f"{context} only MAX_HP_PERCENT_DAMAGE owns push values")
            if ("pushRangeM" in logic) != ("pushMs" in logic):
                raise CompositionError(f"{context} pushRangeM and pushMs must be supplied together")
            push_range = _number(logic.get("pushRangeM", 0), f"{context} pushRangeM", 0, 20)
            push_ms = _integer(logic.get("pushMs", 0), f"{context} pushMs", 0, MAX_TIMELINE_MS)
            if (push_range == 0) != (push_ms == 0):
                raise CompositionError(f"{context} pushRangeM and pushMs must both be zero or positive")
            definition["pushRangeM"] = push_range
            definition["pushMs"] = push_ms
        if kind == "FEAR":
            if duration_ms == 0:
                raise CompositionError(f"{context} FEAR requires durationMs > 0")
            for field in ("sceneProfileId", "effectResourceId", "lightResourceId"):
                if logic.get(field, ""):
                    _stable_id(logic[field], f"{context} {field}")
                elif not isinstance(logic.get(field, ""), str):
                    raise CompositionError(f"{context} {field} must be text")
            delay = _integer(logic.get("effectDelayMs", 0), f"{context} effectDelayMs", 0, duration_ms - 1)
            if delay and not logic.get("effectResourceId", ""):
                raise CompositionError(f"{context} effectDelayMs requires an Effect resource")
        elif extra & {"sceneProfileId", "effectResourceId", "lightResourceId", "effectDelayMs"}:
            raise CompositionError(f"{context} only FEAR owns presentation values")
        if kind == "CAPTURE_PLAYER":
            if logic.get("attachmentSlot") != "BOSS_LEFT_HAND":
                raise CompositionError(f"{context} CAPTURE_PLAYER requires BOSS_LEFT_HAND")
            grip = logic.get("gripLocalOffset")
            _exact_keys(grip, {"forwardM", "upM", "rightM"}, f"{context} gripLocalOffset")
            for axis in ("forwardM", "upM", "rightM"):
                _number(grip[axis], f"{context} gripLocalOffset.{axis}", -10, 10)
        elif extra & {"attachmentSlot", "gripLocalOffset"}:
            raise CompositionError(f"{context} only CAPTURE_PLAYER owns attachment values")
        if (kind == "FOLLOWUP_PATTERN") != bool(followup):
            raise CompositionError(f"{context} FOLLOWUP_PATTERN requires exactly a followupPatternId")
        if followup:
            _stable_id(followup, f"{context} followupPatternId")
        definition["percent"] = percent
        definition["durationMs"] = duration_ms
        definition["followupPatternId"] = followup
        target = logic.get("targetWorldInstanceId", "")
        motion = logic.get("motionInstanceId", "")
        if not isinstance(target, str) or not isinstance(motion, str):
            raise CompositionError(f"{context} World Object motion IDs must be text")
        if kind == "PLAY_WORLD_OBJECT_MOTION":
            _stable_id(target, f"{context} targetWorldInstanceId")
            _stable_id(motion, f"{context} motionInstanceId")
            definition["targetWorldInstanceId"] = target
            definition["motionInstanceId"] = motion
        elif target or motion:
            raise CompositionError(f"{context} World Object motion IDs require PLAY_WORLD_OBJECT_MOTION")
        if kind == "PLAY_CONTACT_WORLD_OBJECT_MOTION":
            mappings = _array(logic.get("contactMotions", []), f"{context} contactMotions", 64)
            mapped = set()
            for mapping in mappings:
                _exact_keys(mapping, {"targetWorldOccurrenceId", "motionInstanceId"}, f"{context} contact motion")
                target_id = _stable_id(mapping["targetWorldOccurrenceId"], f"{context} contact motion target")
                _stable_id(mapping["motionInstanceId"], f"{context} contact motion instance")
                if target_id in mapped:
                    raise CompositionError(f"{context} repeats a contact motion target")
                mapped.add(target_id)
            if not mappings:
                raise CompositionError(f"{context} requires 1..64 contactMotions")
        elif "contactMotions" in logic:
            raise CompositionError(f"{context} contactMotions requires PLAY_CONTACT_WORLD_OBJECT_MOTION")
        if kind == "COMPLETE_LOGIC_WINDOW":
            _stable_id(logic.get("targetLogicOccurrenceId", ""), f"{context} targetLogicOccurrenceId")
            contact_target = logic.get("contactTargetWorldOccurrenceId", "")
            if not isinstance(contact_target, str):
                raise CompositionError(f"{context} contactTargetWorldOccurrenceId must be text")
            if contact_target:
                _stable_id(contact_target, f"{context} contactTargetWorldOccurrenceId")
        elif {"targetLogicOccurrenceId", "contactTargetWorldOccurrenceId"} & extra:
            raise CompositionError(f"{context} window signal values require COMPLETE_LOGIC_WINDOW")
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
    optional_keys: set[str] | None = None,
) -> list[dict[str, Any]]:
    next_ordinal = _integer(
        pattern.get(ordinal_key, 1), f"{context} {ordinal_key}", 1, MAX_ORDINAL
    )
    boxes = _array(pattern.get(list_key, []), f"{context} {list_key}", maximum)
    box_re = re.compile(rf"^{re.escape(pattern_id)}\.{suffix}\.([1-9][0-9]*)$")
    box_ids: set[str] = set()
    for index, box in enumerate(boxes):
        box_context = f"{context}.{list_key}[{index}]"
        _keys(box, box_keys, optional_keys or set(), box_context)
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



# Stable Gate placement contracts mirror the Workbench and the existing Area bosses.
GATE_TARGETS = {
    ("GATE1", "MN_RPCZ_00"): ("boss.kakulsaydon.g1.kouku", "BOSS_KAKULSAYDON_G1_KOUKU"),
    ("GATE1", "MN_RPCT_05"): ("boss.kakulsaydon.g1.saydon", "BOSS_KAKULSAYDON_G1_SAYDON"),
    ("GATE2", "MN_RPCZ_00"): ("boss.kakulsaydon.g2.kouku", "BOSS_KAKULSAYDON_G2_KOUKU"),
    ("GATE2", "MN_RPCT_06"): ("boss.kakulsaydon.g2.big-saydon", "BOSS_KAKULSAYDON_G2_BIG_SAYDON"),
    ("GATE3", "MN_RPCT_05"): ("boss.kakulsaydon.g3.saydon", "BOSS_KAKULSAYDON_G3_SAYDON"),
    ("BINGO", "MN_RPCT_05"): ("boss.kakulsaydon.bingo.saydon", "BOSS_KAKULSAYDON_BINGO_SAYDON"),
}
GATE_IDS = {"GATE1", "GATE2", "GATE3", "BINGO"}
FOLDER_KEYS = {"folderId", "gateId", "displayName"}
BUNDLE_KEYS = {"bundleId", "gateId", "folderId", "displayName", "authoringStatus", "nextMemberOrdinal",
               "nextSceneProfileOccurrenceOrdinal", "nextPresentationOccurrenceOrdinal", "members",
               "sceneProfileOccurrences", "presentationOccurrences"}


def _pattern_target_metadata(pattern: dict[str, Any]) -> dict[str, Any]:
    actor = _pattern_actor_profile_id(pattern)
    # Only the two user-confirmed v2 Gate 2 drafts migrate away from Gate 1.
    legacy_gate = "GATE2" if pattern["patternId"] in {"KAKULSAYDON_G1_PATTERN_8", "KAKULSAYDON_G1_PATTERN_9"} else "GATE1"
    gate = _stable_id(pattern.get("gateId", legacy_gate), "pattern gateId")
    target = _stable_id(pattern.get("targetBossPlacementId", GATE_TARGETS.get((gate, actor), ("", ""))[0]), "pattern targetBossPlacementId")
    return {"gateId": gate, "targetBossPlacementId": target, "actorProfileId": actor}


def _validate_gate_target(pattern: dict[str, Any]) -> None:
    metadata = _pattern_target_metadata(pattern)
    expected = GATE_TARGETS.get((metadata["gateId"], metadata["actorProfileId"]))
    if not expected or metadata["targetBossPlacementId"] != expected[0]:
        raise CompositionError(f"Pattern Gate/target/model mismatch: {pattern.get('patternId')}")


def _pattern_duration(pattern: dict[str, Any]) -> int:
    return sum(stage["durationMs"] for stage in pattern["stages"])


def _bundle_duration(document: dict[str, Any], bundle: dict[str, Any]) -> int:
    patterns = {p["patternId"]: p for p in document["patterns"]}
    return max([0, *[m["startOffsetMs"] + _pattern_duration(patterns[m["patternId"]]) for m in bundle["members"]],
                *[_integer(r.get("startMs"), "bundle scene start", 0, MAX_TIMELINE_MS) + _integer(r.get("durationMs"), "bundle scene duration", 1, MAX_TIMELINE_MS) for r in bundle.get("sceneProfileOccurrences", [])],
                *[_integer(r.get("startMs"), "bundle presentation start", 0, MAX_TIMELINE_MS) + _integer(r.get("durationMs"), "bundle presentation duration", 1, MAX_TIMELINE_MS) for r in bundle.get("presentationOccurrences", [])]])


def load_camera_shots(root: Path) -> dict[str, Any]:
    path = root / "Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json"
    if not path.is_file():
        return {}
    document = load_json(path)
    if document.get("schema") != "lostark.camera-shots" or document.get("formatVersion") != 1 or document.get("areaId") != "LV_LUT_MIDNIGHTC_ED":
        raise CompositionError("Invalid KoukuSaydon Area Camera document")
    result = {}
    for shot in _array(document.get("shots"), "Camera shots", 64):
        identity = _stable_id(shot.get("shotId"), "Camera shotId")
        if identity in result: raise CompositionError("Duplicate Camera shotId")
        _integer(shot.get("blendInMs"), "Camera blendInMs", 0, 10000)
        _integer(shot.get("blendOutMs"), "Camera blendOutMs", 0, 10000)
        result[identity] = shot
    return result


def _camera_return_ms(resource: dict[str, Any], shots: dict[str, Any]) -> int:
    return shots.get(resource.get("assetId"), {}).get("blendOutMs", 0) if resource.get("kind") == "CAMERA" else 0


def _validate_hierarchy(document: dict[str, Any], resources: dict[str, Any], scene_ids: set[str], camera_shots: dict[str, Any] | None = None) -> None:
    camera_shots = camera_shots or {}
    version = document["formatVersion"]
    if version >= 3 and not {"nextFolderOrdinal", "nextBundleOrdinal", "folders", "bundles"} <= document.keys():
        raise CompositionError("Composition v3 requires hierarchy collections and counters")
    folder_next = _integer(document.get("nextFolderOrdinal", 1), "nextFolderOrdinal", 1, MAX_ORDINAL)
    bundle_next = _integer(document.get("nextBundleOrdinal", 1), "nextBundleOrdinal", 1, MAX_ORDINAL)
    folders = {}
    for row in _array(document.get("folders", []), "folders", 4096):
        _exact_keys(row, FOLDER_KEYS, "folder")
        identity = _stable_id(row["folderId"], "folderId")
        _stable_id(row["gateId"], "folder gateId")
        match = re.fullmatch(r"kakulsaydon\.folder\.([1-9][0-9]*)", identity)
        if not match or int(match[1]) >= folder_next or identity in folders or row["gateId"] not in GATE_IDS:
            raise CompositionError("Invalid folder identity, Gate or next ordinal")
        _display_name(row["displayName"], "folder displayName")
        folders[identity] = row
    patterns = {p["patternId"]: p for p in document["patterns"]}
    for pattern in patterns.values():
        if "folderId" not in pattern:
            continue
        folder_id = _stable_id(pattern["folderId"], "pattern folderId")
        folder = folders.get(folder_id)
        if not folder or folder["gateId"] != _pattern_target_metadata(pattern)["gateId"]:
            raise CompositionError(f"Pattern {pattern['patternId']} requires a same-Gate Parent: {folder_id}")
    logics = {l["logicId"]: l for l in document.get("logics", [])}
    worlds = {w["worldId"]: w for w in document.get("worlds", [])}
    bundle_ids = set()
    for bundle in _array(document.get("bundles", []), "bundles", 4096):
        _exact_keys(bundle, BUNDLE_KEYS, "bundle")
        identity = _stable_id(bundle["bundleId"], "bundleId")
        match = re.fullmatch(r"kakulsaydon\.bundle\.([1-9][0-9]*)", identity)
        if not match or int(match[1]) >= bundle_next or identity in bundle_ids:
            raise CompositionError("Invalid or duplicate bundle identity/ordinal")
        bundle_ids.add(identity)
        _display_name(bundle["displayName"], "bundle displayName")
        _stable_id(bundle["gateId"], "bundle gateId")
        folder = folders.get(_stable_id(bundle["folderId"], "bundle folderId"))
        if bundle["gateId"] not in GATE_IDS or not folder or folder["gateId"] != bundle["gateId"]:
            raise CompositionError("Bundle requires a same-Gate folder")
        if _string(bundle["authoringStatus"], "bundle authoringStatus") not in AUTHORING_STATUSES:
            raise CompositionError("Invalid bundle authoringStatus")
        product = bundle["authoringStatus"] == "PRODUCT"
        members = _array(bundle["members"], "bundle members", 8)
        if product and not members:
            raise CompositionError("PRODUCT bundle must have at least one member")
        next_member = _integer(bundle["nextMemberOrdinal"], "nextMemberOrdinal", 1, MAX_ORDINAL)
        ids, used_patterns, used_targets = set(), set(), set()
        global_windows = []
        global_owners = {"SCENE_PROFILE": set(), "CAMERA": set()}
        followup_global_owners = {"SCENE_PROFILE": set(), "CAMERA": set()}
        def collect_global_owners(pattern, owner, followup=False):
            kinds = set()
            if pattern.get("sceneProfileOccurrences"): kinds.add("SCENE_PROFILE")
            if any(resources.get(row["resourceId"], {}).get("kind") == "CAMERA" for row in pattern.get("presentationOccurrences", [])):
                kinds.add("CAMERA")
            for kind in kinds:
                global_owners[kind].add(owner)
                if followup: followup_global_owners[kind].add(owner)
        stateful_members = set()
        world_owners = {}
        def collect_windows(pattern, offset, owner):
            # Only member start is quantized; Camera/Scene rows retain raw ms. Compare in ms * 30 units.
            offset_units = ((offset * FIXED_TICK_HZ + 999) // 1000) * 1000
            for box in pattern.get("sceneProfileOccurrences", []):
                global_windows.append(("SCENE_PROFILE", offset_units + box["startMs"] * FIXED_TICK_HZ,
                                       offset_units + (box["startMs"] + box["durationMs"]) * FIXED_TICK_HZ, owner))
            for box in pattern.get("presentationOccurrences", []):
                if resources.get(box["resourceId"], {}).get("kind") == "CAMERA":
                    global_windows.append(("CAMERA", offset_units + box["startMs"] * FIXED_TICK_HZ,
                                           offset_units + (box["startMs"] + box["durationMs"] + _camera_return_ms(resources[box["resourceId"]], camera_shots)) * FIXED_TICK_HZ, owner))
        for member in members:
            _exact_keys(member, {"memberId", "patternId", "startOffsetMs"}, "bundle member")
            member_id = _stable_id(member["memberId"], "memberId")
            match = re.fullmatch(re.escape(identity) + r"\.member\.([1-9][0-9]*)", member_id)
            pattern = patterns.get(_stable_id(member["patternId"], "member patternId"))
            _integer(member["startOffsetMs"], "member startOffsetMs", 0, MAX_TIMELINE_MS)
            if not match or int(match[1]) >= next_member or member_id in ids or member["patternId"] in used_patterns or not pattern:
                raise CompositionError("Bundle member has invalid/duplicate identity or missing pattern")
            ids.add(member_id); used_patterns.add(member["patternId"])
            _validate_gate_target(pattern)
            target = _pattern_target_metadata(pattern)
            if target["gateId"] != bundle["gateId"] or target["targetBossPlacementId"] in used_targets:
                raise CompositionError("Bundle members must have distinct bosses in the same Gate")
            used_targets.add(target["targetBossPlacementId"])
            if product and pattern["authoringStatus"] != "PRODUCT":
                raise CompositionError("PRODUCT bundle cannot refer to a DRAFT child")
            collect_windows(pattern, member["startOffsetMs"], member_id)
            collect_global_owners(pattern, member_id)
            # Follow-up stays owned by this member; never switches Gate or actor.
            pending, visited = [pattern], set()
            while pending:
                current = pending.pop()
                if current["patternId"] in visited: continue
                visited.add(current["patternId"])
                if _pattern_target_metadata(current) != target:
                    raise CompositionError("Bundle FOLLOWUP_PATTERN must retain Gate and target boss")
                if product and current["authoringStatus"] != "PRODUCT":
                    raise CompositionError("Bundle follow-up must be PRODUCT")
                for box in current.get("logicOccurrences", []):
                    if not box.get("enabled", True): continue
                    logic = logics.get(box["logicId"], {})
                    if logic.get("judgementKind") in {"POSE_INPUT", "ROULETTE_CARD_MATCH"} or logic.get("triggerKind") == "HUD_ENTER":
                        stateful_members.add(member_id)
                    for slot in OUTCOME_SLOTS:
                        for result_id in outcome_logic_ids(box, slot):
                            result = logics.get(result_id, {})
                            if result.get("outcomeKind") == "CLOWN_TRANSFORM": stateful_members.add(member_id)
                            followup = result.get("followupPatternId")
                            if followup:
                                if followup not in patterns: raise CompositionError("Bundle has missing follow-up")
                                collect_global_owners(patterns[followup], member_id, followup=True)
                                pending.append(patterns[followup])
                for box in current.get("worldOccurrences", []):
                    instance = worlds[box["worldId"]]["sequenceInstanceId"]
                    if instance in world_owners and world_owners[instance] != member_id:
                        raise CompositionError("Bundle members cannot share a live WORLD sequence instance")
                    world_owners[instance] = member_id
        if len(stateful_members) > 1:
            raise CompositionError("Bundle has multiple owners of shared player mode (POSE/ROULETTE/HUD/CLOWN)")
        for row in _array(bundle["sceneProfileOccurrences"], "bundle scene rows", 16):
            _exact_keys(row, SCENE_PROFILE_OCCURRENCE_KEYS, "bundle Scene Profile occurrence")
            _stable_id(row["occurrenceId"], "bundle Scene Profile occurrenceId")
            _stable_id(row["sceneProfileId"], "bundle sceneProfileId")
        for row in _array(bundle["presentationOccurrences"], "bundle Camera rows", 16):
            _keys(row, PRESENTATION_OCCURRENCE_REQUIRED, set(PRESENTATION_OCCURRENCE_DEFAULTS), "bundle presentation occurrence")
            _stable_id(row["occurrenceId"], "bundle presentation occurrenceId")
            _stable_id(row["resourceId"], "bundle presentation resourceId")
        span = _bundle_duration(document, bundle)
        if span > MAX_TIMELINE_MS: raise CompositionError("Bundle exceeds 600 seconds")
        # Existing occurrence readers own the common Camera/Scene value contract.
        fake = {"patternId": identity, "nextPresentationOccurrenceOrdinal": bundle["nextPresentationOccurrenceOrdinal"],
                "presentationOccurrences": bundle["presentationOccurrences"]}
        if len(_array(bundle["presentationOccurrences"], "bundle Camera rows", 16)) > 16: raise CompositionError("Too many Camera rows")
        for row in bundle["presentationOccurrences"]:
            resource = resources.get(row.get("resourceId"), {})
            if resource.get("kind") != "CAMERA" or row.get("anchorKind", "BOSS") != "BOSS" or any(row.get(k, "") for k in ("bone", "worldId", "worldOccurrenceId", "logicOccurrenceId", "regionId")):
                raise CompositionError("Bundle common presentation supports unbound Camera only")
        _validate_presentation_occurrences(fake, resources, span, {})
        next_scene = _integer(bundle["nextSceneProfileOccurrenceOrdinal"], "bundle scene ordinal", 1, MAX_ORDINAL)
        scene_occurrence_ids = set()
        for row in _array(bundle["sceneProfileOccurrences"], "bundle scene rows", 16):
            _exact_keys(row, SCENE_PROFILE_OCCURRENCE_KEYS, "bundle Scene Profile occurrence")
            match = re.fullmatch(re.escape(identity) + r"\.sceneprofile\.([1-9][0-9]*)", row["occurrenceId"])
            if not match or int(match[1]) >= next_scene or row["occurrenceId"] in scene_occurrence_ids or row["sceneProfileId"] not in scene_ids:
                raise CompositionError("Invalid bundle Scene Profile identity or reference")
            scene_occurrence_ids.add(row["occurrenceId"])
            start = _integer(row["startMs"], "bundle scene start", 0, MAX_TIMELINE_MS)
            duration = _integer(row["durationMs"], "bundle scene duration", 1, MAX_TIMELINE_MS)
            _integer(row["blendMs"], "bundle scene blend", 0, MAX_TIMELINE_MS)
            if start + duration > MAX_TIMELINE_MS: raise CompositionError("Bundle scene exceeds maximum lifetime")
        collect_windows(bundle, 0, identity)
        collect_global_owners(bundle, identity)
        for kind, owners in followup_global_owners.items():
            if owners and len(global_owners[kind]) > 1:
                raise CompositionError(f"Bundle conditional follow-up global {kind} conflicts with another owner; its start time is not fixed")
        for index, (kind, start, end, owner) in enumerate(global_windows):
            for other_kind, other_start, other_end, other_owner in global_windows[index + 1:]:
                if kind == other_kind and owner != other_owner and start < other_end and other_start < end:
                    raise CompositionError(f"Bundle overlaps global {kind} owners: {owner}/{other_owner}")


def _project_folders(document: dict[str, Any]) -> list[dict[str, Any]]:
    used = {b["folderId"] for b in document.get("bundles", []) if b["authoringStatus"] == "PRODUCT"}
    used.update(_stable_id(p["folderId"], "pattern folderId") for p in document["patterns"]
                if p["authoringStatus"] == "PRODUCT" and "folderId" in p)
    return [dict(row) for row in document.get("folders", []) if row["folderId"] in used]


def _project_bundles(document: dict[str, Any], presentation: bool = False) -> list[dict[str, Any]]:
    patterns = {p["patternId"]: p for p in document["patterns"]}
    result = []
    for bundle in document.get("bundles", []):
        if bundle["authoringStatus"] != "PRODUCT": continue
        row = {key: bundle[key] for key in ("bundleId", "gateId", "folderId", "displayName")}
        row["durationMs"] = _bundle_duration(document, bundle)
        row["members"] = [{**member, **{k: v for k, v in _pattern_target_metadata(patterns[member["patternId"]]).items() if k != "gateId"}}
                          for member in bundle["members"]]
        if presentation:
            fake = {"patternId": bundle["bundleId"], "actorProfileId": "MN_RPCT_05", "stages": [],
                    "sceneProfileOccurrences": bundle["sceneProfileOccurrences"], "presentationOccurrences": bundle["presentationOccurrences"]}
            row["presentationOccurrences"] = _project_pattern_presentation(document, fake)["presentationOccurrences"]
        result.append(row)
    return result


def _validate_contact_links(document, pattern):
    logics = {row["logicId"]: row for row in document.get("logics", [])}
    boxes = {row["occurrenceId"]: row for row in pattern.get("logicOccurrences", [])}
    worlds = {row["occurrenceId"]: row for row in pattern.get("worldOccurrences", [])}
    world_definitions = {row["worldId"]: row for row in document.get("worlds", [])}
    instance_counts = {}
    for cue in worlds.values():
        instance_id = world_definitions[cue["worldId"]]["sequenceInstanceId"]
        instance_counts[instance_id] = instance_counts.get(instance_id, 0) + 1
    groups = {}
    for box in boxes.values():
        logic = logics[box["logicId"]]
        for slot in OUTCOME_SLOTS:
            for result_id in outcome_logic_ids(box, slot):
                result = logics[result_id]
                if result.get("outcomeKind") == "PLAY_WORLD_OBJECT_MOTION" and instance_counts.get(result["targetWorldInstanceId"], 0) > 1:
                    raise CompositionError("PLAY_WORLD_OBJECT_MOTION target is ambiguous; repeated WORLD placements require contact occurrence binding")
        if logic.get("triggerKind") != "OBJECT_CONTACT":
            continue
        for collider in pattern.get("presentationOccurrences", []):
            if collider.get("logicOccurrenceId") != box["occurrenceId"]:
                continue
            if collider.get("anchorKind", "BOSS") == "BOSS" and not collider.get("followBoss", True):
                raise CompositionError("OBJECT_CONTACT Collider must follow its boss anchor")
            if collider.get("anchorKind", "BOSS") == "WORLD" and (collider.get("bone") or collider.get("boneTarget", "BODY") != "BODY"):
                raise CompositionError("OBJECT_CONTACT WORLD Collider cannot use a body or weapon bone")
        candidates = set(logic["targetWorldOccurrenceIds"])
        for target in candidates:
            cue = worlds.get(target)
            if cue is None or cue["startMs"] > box["startMs"] or cue["startMs"] + cue["durationMs"] < box["startMs"] + box["durationMs"]:
                raise CompositionError("OBJECT_CONTACT target WORLD lifetime must contain the whole trigger window")
        group_id = logic.get("contactGroupId", "")
        if group_id and box.get("enabled", True):
            timing, priorities = groups.setdefault((group_id, math.ceil(box["startMs"] * FIXED_TICK_HZ / 1000)),
                                                   ((box["startMs"], box["durationMs"]), set()))
            priority = logic.get("contactPriority", 0)
            if timing != (box["startMs"], box["durationMs"]) or priority in priorities:
                raise CompositionError("OBJECT_CONTACT group needs identical timing and distinct priorities")
            priorities.add(priority)
        for result_id in outcome_logic_ids(box, "Success"):
            result = logics[result_id]
            if result.get("outcomeKind") == "PLAY_CONTACT_WORLD_OBJECT_MOTION":
                if {row["targetWorldOccurrenceId"] for row in result["contactMotions"]} != candidates:
                    raise CompositionError("OBJECT_CONTACT contactMotions must map every candidate exactly once")
            elif result.get("outcomeKind") == "COMPLETE_LOGIC_WINDOW":
                target = boxes.get(result["targetLogicOccurrenceId"])
                if target is None or not target.get("enabled", True) or logics[target["logicId"]].get("judgementKind") != "EXTERNAL_SIGNAL":
                    raise CompositionError("COMPLETE_LOGIC_WINDOW must target an enabled same-pattern EXTERNAL_SIGNAL window")
                if target["startMs"] > box["startMs"] or target["startMs"] + target["durationMs"] < box["startMs"] + box["durationMs"]:
                    raise CompositionError("COMPLETE_LOGIC_WINDOW target must contain the whole trigger window")
                if result.get("contactTargetWorldOccurrenceId") and result["contactTargetWorldOccurrenceId"] not in candidates:
                    raise CompositionError("COMPLETE_LOGIC_WINDOW contact filter must name a contact candidate")


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
    sequences = None
    motion_results = [row for row in logic_defs.values() if row.get("kind") == "PLAY_WORLD_OBJECT_MOTION"]
    if motion_results:
        sequences = load_world_sequences(root, document["areaId"])
        if sequences.get("areaId") != document["areaId"]:
            raise CompositionError("World Object motions must belong to the composition Area")
        for result in motion_results:
            _validate_world_object_motion_result(result, sequences)
    summon_ids, _ = _validate_catalog(
        document, "nextSummonOrdinal", "summons", "summonId", GENERATED_SUMMON_RE,
        SUMMON_KEYS, MAX_SUMMONS, "kakulsaydon.g1.summon",
    )
    world_ids, worlds_by_id = _validate_catalog(
        document, "nextWorldOrdinal", "worlds", "worldId", GENERATED_WORLD_RE,
        WORLD_KEYS, MAX_WORLDS, "kakulsaydon.g1.world", optional_keys={"positionOffset", "anchorKind", "anchorPosition", "companionEffectResourceId", "objectResourceId"},
    )
    for world_id, world in worlds_by_id.items():
        _stable_id(world["sequenceInstanceId"], f"world {world_id} sequenceInstanceId")
        if "objectResourceId" in world:
            if sequences is None:
                sequences = load_world_sequences(root, document["areaId"])
            _validate_world_object_reference(world, sequences)
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
    play_all = _array(
        document["playAllPatternIds"], "playAllPatternIds", MAX_PATTERNS
    )
    presentation_resources = _validate_presentation_resources(document)
    _project_fear_presentations(document)
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
        _number(pattern.get("animationRootVerticalScale", 1.0), f"{context} animationRootVerticalScale", 0.0, 1.0)
        if "resetBossYawDegrees" in pattern:
            _number(pattern["resetBossYawDegrees"], f"{context} resetBossYawDegrees", -360, 360)
            if not pattern.get("resetBossToSpawn", False):
                raise CompositionError(f"{context} resetBossYawDegrees requires resetBossToSpawn")
        actor_profile_id = ""
        if version >= 2:
            actor_profile_id = _stable_id(pattern["actorProfileId"], f"{context} actorProfileId")
            if resolve_actor_profile_id(actor_profile_id) != actor_profile_id:
                raise CompositionError(f"unknown physical actorProfileId: {actor_profile_id}")
        if version >= 3:
            if "gateId" not in pattern or "targetBossPlacementId" not in pattern:
                raise CompositionError(f"{context} requires explicit Gate and target boss")
            _validate_gate_target(pattern)
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
        capture_grip = None
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
                    if owner["type"] != "DURATION" and owner.get("kind") not in {"ENTER_AREA", "OBJECT_CONTACT"}:
                        raise CompositionError(
                            f"{box_context} outcomes are only valid on a DURATION logic box"
                        )
            kind = owner.get("kind")
            hold_id = box.get("holdLogicOccurrenceId", "")
            if not isinstance(hold_id, str):
                raise CompositionError(f"{box_context} holdLogicOccurrenceId must be text")
            if hold_id:
                hold = next((row for row in logic_occurrences if isinstance(row, dict) and row.get("occurrenceId") == hold_id), None)
                if kind != "ENTER_AREA" or hold is None or logic_defs.get(hold["logicId"], {}).get("kind") != "ATTACHMENT_HOLD":
                    raise CompositionError(f"{box_context} Hold must name a same-pattern ATTACHMENT_HOLD")
                hold_start = _integer(hold.get("startMs"), f"{box_context} Hold startMs", 0, MAX_TIMELINE_MS)
                hold_duration = _integer(hold.get("durationMs"), f"{box_context} Hold durationMs", 1, MAX_TIMELINE_MS)
                capture_start = _integer(box["startMs"], f"{box_context} startMs", 0, MAX_TIMELINE_MS)
                capture_duration = _integer(box["durationMs"], f"{box_context} durationMs", 1, MAX_TIMELINE_MS)
                if enabled and status == "PRODUCT" and (not hold.get("enabled", True) or
                        hold_start > capture_start or hold_start + hold_duration < capture_start + capture_duration):
                    raise CompositionError(f"{box_context} Hold must be enabled and cover the complete Trigger window")
            if kind == "ATTACHMENT_HOLD" and any(outcomes.values()):
                raise CompositionError(f"{box_context} ATTACHMENT_HOLD has no outcomes")
            if kind == "ATTACHMENT_HOLD" and any(row.get("logicOccurrenceId") == box_id and
                    presentation_resources[row["resourceId"]]["kind"] == "COLLIDER" for row in pattern.get("presentationOccurrences", [])):
                raise CompositionError(f"{box_context} ATTACHMENT_HOLD has no Collider")
            if sum(logic_defs[target].get("kind") == "CAPTURE_PLAYER" for target in outcomes["Success"]) > 1:
                raise CompositionError(f"{box_context} allows at most one CAPTURE_PLAYER result")
            if kind in END_TICK_KINDS and outcomes["Timeout"]:
                raise CompositionError(
                    f"{box_context} {kind} judges once at the window end and has no Timeout slot"
                )
            if kind in {"STAGGER_WINDOW", "COUNTER_WINDOW", "ENTER_AREA", "EXTERNAL_SIGNAL", "OBJECT_CONTACT"} and outcomes["Fail"]:
                raise CompositionError(
                    f"{box_context} {kind} has no wrong answer and no Fail slot"
                )
            if kind == "OBJECT_CONTACT" and outcomes["Timeout"]:
                raise CompositionError(f"{box_context} OBJECT_CONTACT has no Timeout results")
            if kind == "OBJECT_CONTACT" and sum(logic_defs[target].get("kind") == "PLAY_CONTACT_WORLD_OBJECT_MOTION"
                                                for target in outcomes["Success"]) > 1:
                raise CompositionError(f"{box_context} OBJECT_CONTACT takes at most one contact motion result")
            for slot, targets in outcomes.items():
                for target in targets:
                    result_kind = logic_defs[target].get("kind")
                    if result_kind == "CAPTURE_PLAYER":
                        if kind != "ENTER_AREA" or slot != "Success":
                            raise CompositionError(f"{box_context} CAPTURE_PLAYER belongs to ENTER_AREA Success only")
                        if len(targets) != 1:
                            raise CompositionError(f"{box_context} CAPTURE_PLAYER must be the only Success result")
                        if enabled and status == "PRODUCT":
                            if not hold_id:
                                raise CompositionError(f"{box_context} CAPTURE_PLAYER requires a Hold window")
                            source_logic = next(row for row in document["logics"] if row["logicId"] == target)
                            grip = source_logic["gripLocalOffset"]
                            if capture_grip is not None and capture_grip != grip:
                                raise CompositionError(f"{context} requires one consistent capture grip offset")
                            capture_grip = grip
                    contact_result = result_kind in {"PLAY_CONTACT_WORLD_OBJECT_MOTION", "COMPLETE_LOGIC_WINDOW"}
                    if result_kind is not None and contact_result != (kind == "OBJECT_CONTACT"):
                        raise CompositionError(f"{box_context} OBJECT_CONTACT requires contact motion or window signal results")
                    if result_kind == "GRAB_TO_WORLD_OBJECT" and kind != "ENTER_AREA":
                        raise CompositionError(
                            f"{box_context} GRAB_TO_WORLD_OBJECT is only valid on an ENTER_AREA box, "
                            "whose regions ride a World Object transform track"
                        )
                    if result_kind == "FOLLOWUP_PATTERN":
                        if kind not in {"STAGGER_WINDOW", "COUNTER_WINDOW", "EXTERNAL_SIGNAL"}:
                            raise CompositionError(
                                f"{box_context} FOLLOWUP_PATTERN is only valid on a STAGGER_WINDOW, COUNTER_WINDOW or EXTERNAL_SIGNAL box"
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
            if enabled and status == "PRODUCT" and owner.get("repeatAfterKnockback", False):
                hits = [logic_defs[target] for target in outcomes["Success"]]
                if len(hits) != 1 or hits[0].get("kind") != "MAX_HP_PERCENT_DAMAGE" or hits[0].get("pushRangeM", 0) <= 0 or hits[0].get("pushMs", 0) <= 0:
                    raise CompositionError(f"{box_context} repeat after knockback requires one damage Success with positive knockback")
            if enabled and status == "PRODUCT" and (owner["type"] == "DURATION" or kind in {"ENTER_AREA", "OBJECT_CONTACT"}):
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
            "worldId", world_ids, optional_keys={"placement"},
        )
        world_instances: set[str] = set()
        for box in world_boxes:
            _number(box["playbackSpeed"], f"{context} world box playbackSpeed", 0.05, 16.0)
            instance_id = worlds_by_id[box["worldId"]]["sequenceInstanceId"]
            world_instances.add(instance_id)
            if "placement" in box:
                if sequences is None:
                    sequences = load_world_sequences(root, document["areaId"])
                _validate_world_occurrence_placement(box, worlds_by_id[box["worldId"]], sequences)
        _validate_contact_links(document, pattern)
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
            _exact_keys(stage, STAGE_KEYS | ({"retargetOnEnter"} if "retargetOnEnter" in stage else set()), stage_context)
            retarget = _boolean(stage.get("retargetOnEnter", False), f"{stage_context} retargetOnEnter")
            if retarget and "bossMotion" in pattern:
                raise CompositionError(f"{stage_context} retargetOnEnter cannot share bossMotion yaw")
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
                _exact_keys(occurrence, OCCURRENCE_KEYS | ({"blendInMs"} if "blendInMs" in occurrence else set()), occurrence_context)
                _integer(occurrence.get("blendInMs", 0), f"{occurrence_context} blendInMs", 0, 1000)
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
                _animation_blend_source(pattern, occurrence)
                if status == "PRODUCT" and (
                    start_offset != 0
                    or source_start != 0
                    or play_rate < 0.1
                    or play_rate > 4.0
                ):
                    raise CompositionError(
                        "PRODUCT animation must match the current runtime policy "
                        f"(stage entry, sourceStartMs 0, playRate 0.1..4): {occurrence_id}"
                    )

        charges = sorted((box["startMs"], box["startMs"] + box["durationMs"])
                         for box in pattern.get("logicOccurrences", [])
                         if box.get("enabled", True) and logic_defs[box["logicId"]].get("bossChargeDistanceM", 0) > 0)
        if charges and ("bossMotion" in pattern or any(left[1] > right[0] for left, right in zip(charges, charges[1:]))):
            raise CompositionError("Boss charge windows cannot overlap or share absolute bossMotion")
        if charges:
            stage_start = 0
            for stage in pattern["stages"]:
                if stage.get("retargetOnEnter", False) and any(start <= stage_start < end for start, end in charges):
                    raise CompositionError("Boss charge direction cannot retarget during its window")
                stage_start += stage["durationMs"]
        if "bossMotion" in pattern:
            motion = pattern["bossMotion"]
            _exact_keys(motion, {"startMs", "endMs", "startPosition", "endPosition", "yawDegrees"}, f"{context} bossMotion")
            start = _integer(motion["startMs"], f"{context} bossMotion.startMs", 0, 600000)
            end = _integer(motion["endMs"], f"{context} bossMotion.endMs", 1, pattern_duration_ms)
            _vector3(motion["startPosition"], f"{context} bossMotion.startPosition", -100000, 100000)
            _vector3(motion["endPosition"], f"{context} bossMotion.endPosition", -100000, 100000)
            _number(motion["yawDegrees"], f"{context} bossMotion.yawDegrees", -360, 360)
            if start >= end or motion["startPosition"][1] != motion["endPosition"][1]:
                raise CompositionError(f"{context} bossMotion requires an ordered interval and equal base Y")
            if pattern.get("resetBossToSpawn", False) or "resetBossYawDegrees" in pattern:
                raise CompositionError(f"{context} bossMotion cannot also reset boss to spawn")
            if any(box.get("enabled", True) and logic_defs[box["logicId"]].get("kind") == "REAL_GAZE_TELEPORT" for box in logic_occurrences):
                raise CompositionError(f"{context} bossMotion cannot also teleport the boss")
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

    _validate_hierarchy(document, presentation_resources, scene_profile_ids,
                        load_camera_shots(root) if any(row.get("kind") == "CAMERA" for row in presentation_resources.values()) else {})

    if play_all != product_ids:
        raise CompositionError(
            "playAllPatternIds must equal PRODUCT patternIds in authored order"
        )
    for box_id, target in followup_targets:
        if target not in pattern_ids:
            raise CompositionError(
                f"FOLLOWUP_PATTERN names an unknown pattern: {box_id} -> {target}"
            )


def _publication_candidate(source: dict[str, Any], pattern_ids: set[str],
                           bundle_ids: set[str] | None = None) -> dict[str, Any]:
    # Status is a legacy editing field. Only the private validation copy changes;
    # category, timings, lanes, and the persisted document remain authored values.
    candidate = copy.deepcopy(source)
    candidate["patterns"] = [p for p in candidate["patterns"] if p["patternId"] in pattern_ids]
    candidate["bundles"] = [b for b in candidate.get("bundles", []) if b["bundleId"] in (bundle_ids or set())]
    for row in [*candidate["patterns"], *candidate["bundles"]]:
        row["authoringStatus"] = "PRODUCT"
    candidate["playAllPatternIds"] = [p["patternId"] for p in candidate["patterns"]]
    return candidate


def _saved_pattern_inventory(source: dict[str, Any], root: Path) -> dict[str, Any]:
    """Reject ambiguous saved identities globally; incomplete row bodies stay visible."""
    patterns = _array(source.get("patterns"), "composition patterns", MAX_PATTERNS)
    bundles = _array(source.get("bundles", []), "composition bundles", 4096)
    # Existing validation owns the root, shared definitions, folders and counters.
    # Row bodies are validated separately so one unfinished row cannot hide others.
    skeleton = copy.deepcopy(source)
    skeleton.update(patterns=[], bundles=[], playAllPatternIds=[])
    validate_document(skeleton, root)
    inventory: dict[str, Any] = {"folders": copy.deepcopy(source.get("folders", [])), "patterns": [], "bundles": []}
    folders = {f["folderId"]: f for f in inventory["folders"]}
    pattern_by_id: dict[str, dict[str, Any]] = {}
    for pattern in patterns:
        if not isinstance(pattern, dict):
            raise CompositionError("saved Pattern must be an object")
        identity = _stable_id(pattern.get("patternId"), "saved patternId")
        if identity in pattern_by_id:
            raise CompositionError("duplicate saved patternId: " + identity)
        ordinal = GENERATED_PATTERN_RE.fullmatch(identity)
        if ordinal and int(ordinal[1]) >= source["nextPatternOrdinal"]:
            raise CompositionError("saved patternId is ahead of nextPatternOrdinal: " + identity)
        name = _display_name(pattern.get("displayName"), "saved Pattern displayName")
        category = _string(pattern.get("category"), "saved Pattern category")
        if category not in PATTERN_CATEGORIES or pattern.get("authoringStatus") not in AUTHORING_STATUSES:
            raise CompositionError("saved Pattern category/status is invalid: " + identity)
        if source["formatVersion"] >= 3 and not {"gateId", "actorProfileId", "targetBossPlacementId"} <= pattern.keys():
            raise CompositionError("saved Pattern requires explicit Gate, actor and target: " + identity)
        metadata = _pattern_target_metadata(pattern)
        _stable_id(metadata["actorProfileId"], "saved Pattern actorProfileId")
        if metadata["gateId"] not in GATE_IDS:
            raise CompositionError("saved Pattern Gate is invalid: " + identity)
        row = {"patternId": identity, "displayName": name, "category": category, **metadata, "unavailableReason": ""}
        if "folderId" in pattern:
            folder_id = _stable_id(pattern["folderId"], "saved Pattern folderId")
            if folder_id not in folders or folders[folder_id]["gateId"] != metadata["gateId"]:
                raise CompositionError("saved Pattern needs a same-Gate Parent: " + identity)
            row["folderId"] = folder_id
        inventory["patterns"].append(row)
        pattern_by_id[identity] = row
    saved_order = _array(source["playAllPatternIds"], "saved playAllPatternIds", MAX_PATTERNS)
    if any(not isinstance(identity, str) or identity not in pattern_by_id for identity in saved_order) or len(set(saved_order)) != len(saved_order):
        raise CompositionError("saved playAllPatternIds must refer to distinct saved Patterns")
    bundle_ids: set[str] = set()
    for bundle in bundles:
        if not isinstance(bundle, dict):
            raise CompositionError("saved Bundle must be an object")
        identity = _stable_id(bundle.get("bundleId"), "saved bundleId")
        ordinal = re.fullmatch(r"kakulsaydon\.bundle\.([1-9][0-9]*)", identity)
        if not ordinal or int(ordinal[1]) >= source.get("nextBundleOrdinal", 1) or identity in bundle_ids:
            raise CompositionError("saved Bundle identity/ordinal is invalid: " + identity)
        bundle_ids.add(identity)
        gate = _stable_id(bundle.get("gateId"), "saved Bundle gateId")
        folder_id = _stable_id(bundle.get("folderId"), "saved Bundle folderId")
        if gate not in GATE_IDS or folder_id not in folders or folders[folder_id]["gateId"] != gate:
            raise CompositionError("saved Bundle needs a same-Gate Parent: " + identity)
        if bundle.get("authoringStatus") not in AUTHORING_STATUSES:
            raise CompositionError("saved Bundle status is invalid: " + identity)
        row = {"bundleId": identity, "gateId": gate, "folderId": folder_id,
               "displayName": _display_name(bundle.get("displayName"), "saved Bundle displayName"),
               "members": [], "unavailableReason": ""}
        member_ids: set[str] = set()
        member_patterns: set[str] = set()
        next_member = _integer(bundle.get("nextMemberOrdinal"), "saved Bundle nextMemberOrdinal", 1, MAX_ORDINAL)
        for member in _array(bundle.get("members"), "saved Bundle members", 8):
            _exact_keys(member, {"memberId", "patternId", "startOffsetMs"}, "saved Bundle member")
            member_id = _stable_id(member["memberId"], "saved memberId")
            match = re.fullmatch(re.escape(identity) + r"\.member\.([1-9][0-9]*)", member_id)
            target = _stable_id(member["patternId"], "saved member patternId")
            if not match or int(match[1]) >= next_member or member_id in member_ids or target in member_patterns:
                raise CompositionError("saved Bundle member identity is invalid: " + identity)
            if target not in pattern_by_id or pattern_by_id[target]["gateId"] != gate:
                raise CompositionError("saved Bundle child must exist in the same Gate: " + identity)
            _integer(member["startOffsetMs"], "saved member startOffsetMs", 0, MAX_TIMELINE_MS)
            member_ids.add(member_id)
            member_patterns.add(target)
            row["members"].append(dict(member))
        inventory["bundles"].append(row)
    return inventory


def _pattern_dependencies(source: dict[str, Any], pattern: dict[str, Any]) -> set[str]:
    definitions = {row["logicId"]: row for row in source.get("logics", [])}
    result: set[str] = set()
    for box in _array(pattern.get("logicOccurrences", []), "Pattern logicOccurrences", MAX_LOGIC_OCCURRENCES_PER_PATTERN):
        if not isinstance(box, dict):
            raise CompositionError("Pattern Logic occurrence must be an object")
        if not isinstance(box.get("logicId"), str):
            raise CompositionError("Pattern Logic occurrence logicId must be text")
        # validate_publishable also requires disabled outcome references to resolve.
        for slot in OUTCOME_SLOTS:
            for logic_id in outcome_logic_ids(box, slot):
                target = definitions.get(logic_id, {}).get("followupPatternId", "")
                if target:
                    result.add(target)
        if box.get("enabled", True):
            target = definitions.get(box.get("logicId"), {}).get("clonePatternId", "")
            if target:
                result.add(target)
    return result


def prepare_publication(source: dict[str, Any], root: Path = REPOSITORY_ROOT) -> tuple[dict[str, Any], dict[str, Any]]:
    """Admit complete dependency closures, preserving every saved tree row."""
    inventory = _saved_pattern_inventory(source, root)
    patterns = {row["patternId"]: row for row in source["patterns"]}
    reasons: dict[str, str] = {}
    ready: set[str] = set()
    for row in inventory["patterns"]:
        identity = row["patternId"]
        try:
            closure: set[str] = set()
            pending = [identity]
            while pending:
                current = pending.pop()
                if current in closure:
                    continue
                if current not in patterns:
                    raise CompositionError("required Pattern is missing: " + current)
                closure.add(current)
                pending.extend(_pattern_dependencies(source, patterns[current]) - closure)
            candidate = _publication_candidate(source, closure)
            validate_document(candidate, root)
            validate_publishable(candidate, root)
            projected_outputs(candidate, root)
            ready.add(identity)
        except CompositionError as error:
            reasons[identity] = str(error)
        row["unavailableReason"] = reasons.get(identity, "")
    # Every admitted closure was validated, so its dependencies are also admitted.
    product = _publication_candidate(source, ready)
    validate_document(product, root)
    validate_publishable(product, root)
    projected_outputs(product, root)
    ready_bundles: set[str] = set()
    for row in inventory["bundles"]:
        try:
            for member in row["members"]:
                if member["patternId"] not in ready:
                    raise CompositionError("Bundle child is unavailable: " + member["patternId"] + ": " + reasons[member["patternId"]])
            candidate = _publication_candidate(source, ready, {row["bundleId"]})
            validate_document(candidate, root)
            validate_publishable(candidate, root)
            projected_outputs(candidate, root)
            ready_bundles.add(row["bundleId"])
        except CompositionError as error:
            row["unavailableReason"] = str(error)
    product = _publication_candidate(source, ready, ready_bundles)
    validate_document(product, root)
    validate_publishable(product, root)
    projected_outputs(product, root)
    return product, inventory


def load_and_validate(root: Path = REPOSITORY_ROOT) -> dict[str, Any]:
    return prepare_publication(load_json(root / SOURCE_PATH), root)[0]


def validate_publishable(document: dict[str, Any], root: Path = REPOSITORY_ROOT) -> None:
    _join_light_resources(document, root)
    resources = {row["resourceId"]: row for row in document.get("presentationResources", [])}
    camera_shots = None
    for owner in [*document["patterns"], *document.get("bundles", [])]:
        if owner.get("authoringStatus") != "PRODUCT": continue
        for box in owner.get("presentationOccurrences", []):
            resource = resources[box["resourceId"]]
            if resource["kind"] != "CAMERA": continue
            if camera_shots is None: camera_shots = load_camera_shots(root)
            shot = camera_shots.get(resource["assetId"])
            if shot is None: raise CompositionError("PRODUCT Camera shotId is absent from the Area source: " + resource["assetId"])
            if box["durationMs"] < shot["blendInMs"]:
                raise CompositionError("Camera box entry plus hold is shorter than the shot blend-in")
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
        **({"actions": [{"trigger": "ENTER", "kind": "RETARGET_RANDOM_ALIVE",
                        "targetId": "boss.target.pattern", "value": 1, "durationMs": 0}]}
           if stage.get("retargetOnEnter", False) else {}),
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


def _validate_world_object_motion_result(result, sequences):
    instances = {row["instanceId"]: row for row in sequences.get("instances", [])}
    objects = {row["objectId"]: row for row in sequences.get("objectResources", [])}
    templates = {row["sequenceId"]: row for row in sequences.get("templates", [])}
    bindings = []
    for key in ("targetWorldInstanceId", "motionInstanceId"):
        instance = instances.get(result[key])
        instance_bindings = instance.get("bindings", []) if instance else []
        if len(instance_bindings) != 1 or instance_bindings[0].get("targetKind") != "OBJECT_RESOURCE":
            raise CompositionError(f"{key} must reference one World Object instance in this Area")
        if not instance.get("enabled", True):
            raise CompositionError(f"{key} World Object instance is disabled")
        binding = instance_bindings[0]
        if binding.get("targetId") not in objects or instance.get("templateId") not in templates:
            raise CompositionError(f"{key} World Object or saved Motion template is missing")
        bindings.append(binding)
    if bindings[0]["targetId"] != bindings[1]["targetId"]:
        raise CompositionError("targetWorldInstanceId and motionInstanceId must bind the same World Object")
    if bindings[0].get("slotId") != bindings[1].get("slotId"):
        raise CompositionError("targetWorldInstanceId and motionInstanceId must bind the same slotId")
    motion = instances[result["motionInstanceId"]]
    return motion, templates[motion["templateId"]]


def _require_single_static_object_motion(template, context):
    motion = template.get("objectMotion", {})
    if motion.get("count", 1) != 1 or motion.get("spreadDegrees", 0) != 0 or any(
        any(float(value) != 0 for value in motion.get(key, [0, 0, 0]))
        for key in ("velocity", "acceleration", "angularVelocityDegrees", "revolutionDegreesPerSecond", "revolutionOffset")
    ):
        raise CompositionError(f"{context} requires count 1 and zero physical Object Motion")


def _validate_fixed_target_motion_chain(result, sequences, target_pose, slot_id):
    current = dict(result)
    visited = set()
    for _ in range(32):
        motion_id = current["motionInstanceId"]
        if motion_id in visited:
            raise CompositionError("OBJECT_OVERLAP Result motion chain contains a cycle")
        visited.add(motion_id)
        motion, template = _validate_world_object_motion_result(current, sequences)
        _require_single_static_object_motion(template, "OBJECT_OVERLAP Result motion")
        tracks = [track for track in template.get("tracks", []) if track["slotId"] == slot_id]
        if len(tracks) != 1 or not tracks[0].get("keys") or any(
            {key:value for key,value in pose.items() if key != "timeMs"} != target_pose
            for pose in tracks[0]["keys"]
        ):
            raise CompositionError("OBJECT_OVERLAP Result motion transform must preserve fixed target geometry")
        end = motion.get("motionEnd", "STOP")
        if end == "NEXT":
            current["motionInstanceId"] = motion.get("nextMotionId", "")
            continue
        if end not in {"LOOP", "HOLD"}:
            raise CompositionError("OBJECT_OVERLAP Result motion chain must finish with LOOP or HOLD to keep its target visible")
        return
    raise CompositionError("OBJECT_OVERLAP Result motion chain exceeds 32 instances")


def _project_object_overlap_target(document, pattern, box, logic, logics, sequences):
    target = logic["targetWorldInstanceId"]
    worlds = {w["worldId"]: w for w in document.get("worlds", [])}
    cues = [(w, worlds[w["worldId"]]) for w in pattern.get("worldOccurrences", [])
            if worlds[w["worldId"]]["sequenceInstanceId"] == target]
    if len(cues) != 1:
        raise CompositionError("OBJECT_OVERLAP needs exactly one target WORLD occurrence")
    cue, world = cues[0]
    projected, first, binding = _project_fixed_object_target(cue, world, box, logic["targetRadiusM"], sequences)
    for slot in OUTCOME_SLOTS:
        for result_id in outcome_logic_ids(box, slot):
            result = logics[result_id]
            if result.get("outcomeKind") != "PLAY_WORLD_OBJECT_MOTION" or result.get("targetWorldInstanceId") != target:
                raise CompositionError("OBJECT_OVERLAP Result must apply a motion to the same target World Object")
            _validate_fixed_target_motion_chain(result, sequences, first, binding["slotId"])
    return projected


def _validate_world_object_reference(world, sequences):
    """Validate Object identity without rebinding an existing box to a changed default."""
    object_id = _stable_id(world["objectResourceId"], "WORLD objectResourceId")
    resources = [row for row in sequences.get("objectResources", []) if row.get("objectId") == object_id]
    if len(resources) != 1:
        raise CompositionError(f"WORLD objectResourceId is missing or duplicated: {object_id}")
    resource = resources[0]
    alias = resource.get("sequenceInstanceId", "")
    def state(instance_id, context):
        _stable_id(instance_id, context)
        matches = [row for row in sequences.get("instances", []) if row.get("instanceId") == instance_id]
        if len(matches) != 1 or matches[0].get("enabled", True) is not True:
            raise CompositionError(f"{context} must reference an enabled saved Object state")
        instance = matches[0]
        if sum(row.get("sequenceId") == instance.get("templateId") for row in sequences.get("templates", [])) != 1:
            raise CompositionError(f"{context} saved Motion template is missing or duplicated")
        bindings = instance.get("bindings", [])
        if alias:
            if instance_id != alias:
                raise CompositionError(f"{context} must preserve the Object sequence alias")
        elif (len(bindings) != 1 or bindings[0].get("targetKind") != "OBJECT_RESOURCE" or
              bindings[0].get("targetId") != object_id):
            raise CompositionError(f"{context} must bind exactly the same Object")
        return instance
    selected = state(world["sequenceInstanceId"], "WORLD stored initial state")
    default = resource.get("defaultMotionInstanceId", "")
    if not isinstance(default, str):
        raise CompositionError("Object defaultMotionInstanceId must be a string")
    if default:
        state(default, "Object defaultMotionInstanceId")
    return selected


def _validate_world_occurrence_placement(cue, world, sequences):
    placement = cue["placement"]
    _exact_keys(placement, {"position", "rotationDegrees", "scale"}, "WORLD occurrence placement")
    _vector3(placement["position"], "WORLD placement position", -100000, 100000)
    _vector3(placement["rotationDegrees"], "WORLD placement rotationDegrees", -36000, 36000)
    _vector3(placement["scale"], "WORLD placement scale", .001, 1000)
    instance = next((row for row in sequences["instances"] if row["instanceId"] == world["sequenceInstanceId"]), None)
    bindings = instance.get("bindings", []) if instance else []
    if len(bindings) != 1 or bindings[0].get("targetKind") != "OBJECT_RESOURCE":
        raise CompositionError("WORLD placement requires exactly one World Object resource")
    if instance.get("enabled", True) is not True:
        raise CompositionError("WORLD placement initial Object state is disabled")
    resource = next((row for row in sequences.get("objectResources", []) if row["objectId"] == bindings[0]["targetId"]), None)
    anchor = instance.get("anchorKind", "WORLD")
    if resource is None or anchor not in {"WORLD", "PLAYER", "BOSS"} or resource.get("anchorKind", "WORLD") != anchor:
        raise CompositionError("WORLD placement requires a matching WORLD/PLAYER/BOSS Object anchor")
    if "walkableSurface" in instance:
        raise CompositionError("WORLD Object placement cannot replace a Map walkable surface")
    return placement


def _world_placement_point(placement, point):
    pitch, yaw, roll = [math.radians(value) * .5 for value in placement["rotationDegrees"]]
    sp, cp, sy, cy, sr, cr = math.sin(pitch), math.cos(pitch), math.sin(yaw), math.cos(yaw), math.sin(roll), math.cos(roll)
    quaternion = (cr*sp*cy + sr*cp*sy, cr*cp*sy - sr*sp*cy, sr*cp*cy - cr*sp*sy, cr*cp*cy + sr*sp*sy)
    matrix = wmodel_pose.affine_matrix(placement["scale"], quaternion, placement["position"])
    return list(wmodel_pose.transform_point(tuple(point), matrix))


def _project_world_placement(cue, world, sequences):
    if "placement" not in cue:
        return {}
    placement = _validate_world_occurrence_placement(cue, world, sequences)
    return {"placement": {key: list(value) for key, value in placement.items()}, "positionOffset": [0.0, 0.0, 0.0],
            "anchorKind": "NONE", "anchorPosition": [0.0, 0.0, 0.0]}


def _project_fixed_object_target(cue, world, box, radius, sequences, context="OBJECT_OVERLAP"):
    target = world["sequenceInstanceId"]
    if cue["startMs"] > box["startMs"] or cue["startMs"]+cue["durationMs"] < box["startMs"]+box["durationMs"]:
        raise CompositionError(f"{context} target WORLD lifetime must contain the whole window")
    if "placement" not in cue and world.get("anchorKind", "NONE") != "NONE":
        raise CompositionError(f"{context} target needs an absolute WORLD anchor")
    instance = next((r for r in sequences["instances"] if r["instanceId"] == target), None)
    bindings = instance.get("bindings", []) if instance else []
    if len(bindings) != 1 or bindings[0].get("targetKind") != "OBJECT_RESOURCE":
        raise CompositionError(f"{context} target must be one World Object instance")
    template = next((r for r in sequences["templates"] if r["sequenceId"] == instance["templateId"]), None)
    resource = next((r for r in sequences.get("objectResources", []) if r["objectId"] == bindings[0]["targetId"]), None)
    if template is None or resource is None or not instance.get("enabled", True):
        raise CompositionError(f"{context} target is missing or disabled")
    if instance.get("anchorKind", "WORLD") != "WORLD" or resource.get("anchorKind", "WORLD") != "WORLD" or instance.get("startDelayMs", 0) != 0:
        raise CompositionError(f"{context} target requires a fixed WORLD anchor without start delay")
    if instance.get("motionEnd", "STOP") not in {"LOOP", "HOLD"}:
        raise CompositionError(f"{context} target needs LOOP or HOLD to stay visible for its WORLD lifetime; STOP/NEXT are unsupported")
    _require_single_static_object_motion(template, f"{context} target")
    tracks = [t for t in template.get("tracks", []) if t["slotId"] == bindings[0]["slotId"]]
    if len(tracks) != 1 or not tracks[0].get("keys"):
        raise CompositionError(f"{context} target needs one static transform track")
    keys = tracks[0]["keys"]
    first = {k:v for k,v in keys[0].items() if k != "timeMs"}
    if not first.get("visible", True) or any({k:v for k,v in key.items() if k != "timeMs"} != first for key in keys):
        raise CompositionError(f"{context} target transform must remain static and visible")
    position = [float(a)+float(b)+float(c) for a,b,c in zip(instance.get("position", [0,0,0]),
                first.get("positionOffset", [0,0,0]), world.get("positionOffset", [0,0,0]))]
    if "placement" in cue:
        position = _world_placement_point(_validate_world_occurrence_placement(cue, world, sequences), first.get("positionOffset", [0, 0, 0]))
    if any(not math.isfinite(value) or abs(value) > 100000 for value in position):
        raise CompositionError("World Object target position is outside runtime bounds")
    return ({"targetWorldInstanceId": target, "targetWorldX": position[0], "targetWorldZ": position[2],
             "targetRadiusM": radius}, first, bindings[0])


def _project_object_contact_targets(document, pattern, box, logic, logics, sequences):
    worlds = {row["worldId"]: row for row in document.get("worlds", [])}
    cues = {row["occurrenceId"]: row for row in pattern.get("worldOccurrences", [])}
    targets = []
    for occurrence_id in logic["targetWorldOccurrenceIds"]:
        cue = cues.get(occurrence_id)
        if cue is None:
            raise CompositionError("OBJECT_CONTACT target must name a same-pattern WORLD occurrence")
        world = worlds[cue["worldId"]]
        target, _, _ = _project_fixed_object_target(cue, world, box, logic["targetRadiusM"], sequences, "OBJECT_CONTACT")
        targets.append({"targetWorldOccurrenceId": occurrence_id, **target})
    by_id = {row["targetWorldOccurrenceId"]: row for row in targets}
    for result_id in outcome_logic_ids(box, "Success"):
        result = logics[result_id]
        if result.get("outcomeKind") != "PLAY_CONTACT_WORLD_OBJECT_MOTION":
            continue
        if {row["targetWorldOccurrenceId"] for row in result["contactMotions"]} != set(by_id):
            raise CompositionError("OBJECT_CONTACT contactMotions must map every candidate exactly once")
        for mapping in result["contactMotions"]:
            # Contact is consumed once per card occurrence; its response may flip,
            # fly away or disappear without changing an unhit card's fixed target.
            _validate_world_object_motion_result({
                "targetWorldInstanceId": by_id[mapping["targetWorldOccurrenceId"]]["targetWorldInstanceId"],
                "motionInstanceId": mapping["motionInstanceId"]}, sequences)
    return {"contactTargets": targets, "contactGroupId": logic.get("contactGroupId", ""),
            "contactPriority": logic.get("contactPriority", 0)}


def _load_region_world(root: Path, area: str, sequences: dict[str, Any], world: dict[str, Any], cue=None):
    instance = next((r for r in sequences["instances"] if r["instanceId"] == world["sequenceInstanceId"]), None)
    if instance is None or len(instance.get("bindings", [])) != 1:
        raise CompositionError("Collider WORLD anchor needs exactly one bound placement")
    binding = instance["bindings"][0]
    template = next(r for r in sequences["templates"] if r["sequenceId"] == instance["templateId"])
    track = next((r for r in template["tracks"] if r["slotId"] == binding["slotId"]), None)
    if track is None or not track["keys"]:
        raise CompositionError("Collider WORLD has no transform track")
    if binding["targetKind"] == "OBJECT_RESOURCE":
        _require_single_static_object_motion(template, "Collider WORLD source")
        resource = next((r for r in sequences.get("objectResources", []) if r["objectId"] == binding["targetId"]), None)
        if resource is None or instance.get("anchorKind", "WORLD") != "WORLD" or resource.get("anchorKind", "WORLD") != "WORLD":
            raise CompositionError("Collider World Object must have a fixed WORLD anchor")
        position = instance.get("position", [0,0,0])
        _vector3(position, "Collider World Object position", -100000, 100000)
        scale = resource.get("scale", [1,1,1])
        _vector3(scale, "Collider World Object scale", .0001, 10000)
        if cue is not None and "placement" in cue:
            placement = _validate_world_occurrence_placement(cue, world, sequences)
            if placement["rotationDegrees"][0] != 0 or placement["rotationDegrees"][2] != 0 or abs(placement["scale"][0] - placement["scale"][2]) > .0001:
                raise CompositionError("XZ WORLD Collider placement needs yaw-only rotation and uniform X/Z scale")
            return instance, template, track, list(placement["position"]), placement["rotationDegrees"][1], [a*b for a,b in zip(scale,placement["scale"])]
        return instance, template, track, position, 0.0, scale
    if binding["targetKind"] != "MAP_PLACEMENT":
        raise CompositionError("Collider WORLD requires a placement or World Object")
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


def _project_walkable_surface(root, area, sequences, world, box):
    instance = next((row for row in sequences["instances"] if row["instanceId"] == world["sequenceInstanceId"]), None)
    if instance is None or "walkableSurface" not in instance:
        return {}
    if "placement" in box:
        raise CompositionError("WORLD Object placement cannot replace a Map walkable surface")
    surface = instance["walkableSurface"]
    if not isinstance(surface, dict) or set(surface) != {"radiusM", "localHeightM"}:
        raise CompositionError("Walkable surface requires radiusM and localHeightM")
    radius = _number(surface["radiusM"], "walkable surface radiusM", .001, 1000)
    height = _number(surface["localHeightM"], "walkable surface localHeightM", -10000, 10000)
    if len(instance.get("bindings", [])) != 1 or instance["bindings"][0]["targetKind"] != "MAP_PLACEMENT" or instance.get("motionEnd", "STOP") != "STOP":
        raise CompositionError("Walkable surface requires one static Map placement and STOP")
    instance, template, track, position, yaw, scale = _load_region_world(root, area, sequences, world)
    if not instance.get("enabled", True) or template.get("animationTracks"):
        raise CompositionError("Walkable surface source is disabled or animated")
    if min(scale) <= 0 or abs(scale[0] - scale[2]) > .00001:
        raise CompositionError("Walkable surface placement scale must be positive and uniform in X/Z")
    first = track["keys"][0]
    for key in track["keys"]:
        q = key["rotationQuaternion"]
        if abs(q[0]) > .00001 or abs(q[2]) > .00001 or key["positionOffset"] != first["positionOffset"] or key["scaleMultiplier"] != first["scaleMultiplier"]:
            raise CompositionError("Walkable surface supports fixed position/scale and Y rotation only")
    multiplier = first["scaleMultiplier"]
    _vector3(multiplier, "walkable surface scale", .000001, 100000)
    if abs(multiplier[0] - multiplier[2]) > .00001:
        raise CompositionError("Walkable surface scale must be uniform in X/Z")
    offset = _rotate_y(first["positionOffset"], yaw)
    center = [position[0] + offset[0], position[2] + offset[2]]
    projected_height = position[1] + offset[1] + height * scale[1] * multiplier[1]
    projected_radius = radius * scale[0] * multiplier[0]
    if projected_radius > 1000 or any(not math.isfinite(v) or abs(v) > 100000 for v in [*center, projected_height]):
        raise CompositionError("Projected walkable surface is outside runtime bounds")
    speed = float(instance.get("playbackSpeed", 1)) * float(box["playbackSpeed"])
    delay = instance.get("startDelayMs", 0)
    cue_end = math.ceil(box["durationMs"] * FIXED_TICK_HZ / 1000)
    windows = []
    keys = track["keys"]
    for index, key in enumerate(keys):
        if not key.get("visible", True):
            continue
        begin = min(cue_end, math.ceil((delay + key["timeMs"] / speed) * FIXED_TICK_HZ / 1000))
        end = cue_end if index + 1 == len(keys) else min(cue_end, math.ceil((delay + keys[index + 1]["timeMs"] / speed) * FIXED_TICK_HZ / 1000))
        if begin >= end:
            continue
        if windows and windows[-1]["endTick"] == begin:
            windows[-1]["endTick"] = end
        else:
            windows.append({"startTick": begin, "endTick": end})
    if not windows or len(windows) > 32:
        raise CompositionError("Walkable surface must have 1..32 visible windows inside its WORLD cue")
    return {"walkableSurface": {"centerX": center[0], "centerZ": center[1], "heightY": projected_height,
                                  "radiusM": projected_radius, "windows": windows}}


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
    instance, template, track, position, yaw, scale = _load_region_world(root,area,sequences,world,box)
    if not instance.get("enabled", True):
        raise CompositionError("WORLD Trigger anchor instance is disabled")
    if min(scale) <= 0 or abs(scale[0]-scale[2]) > 0.0001:
        raise CompositionError("WORLD Trigger parent scale must be positive and uniform in X/Z")
    if "placement" not in box:
        position = [a+b for a,b in zip(position,world.get("positionOffset",[0,0,0]))]
    if "placement" not in box and world.get("anchorKind", "NONE") == "BOSS_SPAWN":
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
        key_position = list(key["positionOffset"])
        if "placement" in box:
            key_position = [a*b for a,b in zip(key_position, box["placement"]["scale"])]
        _vector3(key_position,"WORLD Trigger placed key position",-100000,100000)
        keys.append({"timeMs":time,"positionOffset":key_position,
                     "rotationY":q[1]/length,"rotationW":q[3]/length,
                     "scaleMultiplier":list(key["scaleMultiplier"]),"visible":_boolean(key.get("visible",True),"WORLD Trigger visible")})
    return {"startMs":box["startMs"],"startDelayMs":instance.get("startDelayMs",0),"durationMs":template["durationMs"],
            "playbackSpeed":float(box["playbackSpeed"])*float(instance.get("playbackSpeed",1)),
            "interpolation":template.get("interpolation","LINEAR"),"baselinePosition":position,
            "baselineYawDegrees":yaw,"baselineScale":scale,"keys":keys}


def _sample_region_world(root: Path, area: str, sequences: dict[str, Any], world: dict[str, Any],
                         box: dict[str, Any], end_ms: int) -> tuple[list[float], float, list[float]]:
    instance, template, track, position, base_yaw, scale = _load_region_world(root,area,sequences,world,box)
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
    key_position = vector("positionOffset")
    if "placement" in box:
        key_position = [a*b for a,b in zip(key_position, box["placement"]["scale"])]
    offset = _rotate_y(key_position, base_yaw)
    legacy_offset = [0,0,0] if "placement" in box else world.get("positionOffset", [0,0,0])
    position = [a+b+c for a,b,c in zip(position, offset, legacy_offset)]
    scale = [a*b for a,b in zip(scale, vector("scaleMultiplier"))]
    qa, qb = first["rotationQuaternion"], second["rotationQuaternion"]
    if any(abs(q[i]) > 0.00001 for q in (qa,qb) for i in (0,2)):
        raise CompositionError("XZ gameplay collider WORLD track must rotate around Y")
    yaw = base_yaw + _slerp_yaw(qa,qb,factor)
    if "placement" not in box and world.get("anchorKind", "NONE") == "BOSS_SPAWN":
        position = [a-b for a,b in zip(position,world.get("anchorPosition",[0,0,0]))]
    return position, yaw, scale


def _load_bone_bake_actor(pattern, root, cache):
    metadata = _pattern_target_metadata(pattern)
    expected = GATE_TARGETS.get((metadata["gateId"], metadata["actorProfileId"]))
    if not expected or metadata["targetBossPlacementId"] != expected[0]:
        raise CompositionError("Bone Collider has an unknown Gate/actor target")
    cache_key = (str(root.resolve()), expected[1])
    if cache_key in cache:
        return cache[cache_key]
    actors = [row for row in load_json(root / BOSS_CATALOG_PATH).get("bosses", []) if row.get("archetypeId") == expected[1]]
    if len(actors) != 1 or actors[0].get("bodyModel") != REFERENCE_MODEL_ASSET_IDS[metadata["actorProfileId"]] + ".wmodel":
        raise CompositionError("Bone Collider requires the exact boss catalog model")
    actor = actors[0]
    resources = (root / "Client/Bin/Resources").resolve()
    def load_model(asset_id):
        if not isinstance(asset_id, str) or not asset_id or "\\" in asset_id or ":" in asset_id or ".." in Path(asset_id).parts:
            raise CompositionError("Bone Collider model needs a Resources-relative asset ID")
        path = (resources / asset_id).resolve()
        if not path.is_relative_to(resources) or path.suffix.lower() != ".wmodel":
            raise CompositionError("Bone Collider model escapes Resources or has an unsupported format")
        try:
            model = wmodel_pose.read_wmodel(path)
        except (OSError, ValueError, IndexError, KeyError, struct.error) as error:
            raise CompositionError(f"Cannot read Bone Collider model {asset_id}: {error}") from error
        if not model.skeleton_bones or len({row.name for row in model.skeleton_bones}) != len(model.skeleton_bones):
            raise CompositionError("Bone Collider model has an empty or ambiguous skeleton")
        return model
    body = load_model(actor["bodyModel"])
    weapon = load_model(actor["weaponModel"]) if actor.get("weaponModel") else None
    body_scale = _number(actor.get("bodyModelPreScale"), "Bone Collider body scale", .000001, 100)
    body_pre = wmodel_pose.affine_matrix((body_scale,) * 3, (0, 0, 0, 1), (0, 0, 0))
    weapon_pre = None
    if weapon:
        scale = _number(actor.get("weaponModelPreScale"), "Bone Collider weapon scale", .000001, 100)
        angles = actor.get("weaponModelPreRotationDegrees", [0, 0, 0])
        _vector3(angles, "Bone Collider weapon rotation", -360, 360)
        pitch, yaw, roll = [math.radians(value) * .5 for value in angles]
        sp, cp, sy, cy, sr, cr = math.sin(pitch), math.cos(pitch), math.sin(yaw), math.cos(yaw), math.sin(roll), math.cos(roll)
        # DirectXMath XMQuaternionRotationRollPitchYaw (row-vector convention).
        quaternion = (cr*sp*cy + sr*cp*sy, cr*cp*sy - sr*sp*cy,
                      sr*cp*cy - cr*sp*sy, cr*cp*cy + sr*sp*sy)
        weapon_pre = wmodel_pose.affine_matrix((scale,) * 3, quaternion, (0, 0, 0))
    value = {"body": body, "weapon": weapon, "bodyPre": body_pre, "weaponPre": weapon_pre,
             "actorProfileId": metadata["actorProfileId"], "poses": {}, "validatedClips": set()}
    cache[cache_key] = value
    return value


def _animation_blend_source(pattern, occurrence):
    previous = None
    elapsed = 0
    for stage in pattern["stages"]:
        for row in sorted(stage["animationOccurrences"], key=lambda value: value["startOffsetMs"]):
            start = elapsed + row["startOffsetMs"]
            if row["occurrenceId"] == occurrence["occurrenceId"]:
                if not row.get("blendInMs", 0):
                    return None
                if not previous or row["blendInMs"] > row["playMs"]:
                    raise CompositionError("Animation blend requires a previous occurrence and must fit the current window")
                prior, prior_start = previous
                if prior_start + prior["playMs"] != start or prior["endPolicy"] != "EXACT":
                    raise CompositionError("Animation blend requires an adjacent EXACT previous occurrence")
                return prior, (prior["sourceStartMs"] + prior["playMs"] * prior["playRate"]) / 1000
            previous = row, start
        elapsed += stage["durationMs"]
    raise CompositionError("Animation blend occurrence is not in its Pattern")


def _blend_local_matrix(left, right, alpha):
    def decompose(matrix):
        scale = [math.sqrt(sum(matrix[row * 4 + col] ** 2 for col in range(3))) for row in range(3)]
        if min(scale) <= 1e-8:
            raise CompositionError("Animation blend cannot decompose a singular bone pose")
        rows = [[matrix[row * 4 + col] / scale[row] for col in range(3)] for row in range(3)]
        determinant = sum(rows[0][i] * (rows[1][(i+1)%3] * rows[2][(i+2)%3] - rows[1][(i+2)%3] * rows[2][(i+1)%3]) for i in range(3))
        if determinant < 0:
            axis = max(range(3), key=lambda i: scale[i]); scale[axis] *= -1
            rows[axis] = [-value for value in rows[axis]]
        # Convert row-vector rotation to the conventional column-vector matrix.
        m = [[rows[col][row] for col in range(3)] for row in range(3)]
        trace = sum(m[i][i] for i in range(3))
        if trace > 0:
            t = math.sqrt(trace + 1) * 2
            q = [(m[2][1]-m[1][2])/t, (m[0][2]-m[2][0])/t, (m[1][0]-m[0][1])/t, t/4]
        else:
            i = max(range(3), key=lambda v: m[v][v]); j=(i+1)%3; k=(i+2)%3
            t = math.sqrt(max(0, 1+m[i][i]-m[j][j]-m[k][k])) * 2
            if t <= 1e-8: raise CompositionError("Animation blend has an invalid rotation")
            q = [0.,0.,0.,0.]; q[i]=t/4; q[j]=(m[j][i]+m[i][j])/t; q[k]=(m[k][i]+m[i][k])/t; q[3]=(m[k][j]-m[j][k])/t
        length = math.sqrt(sum(value*value for value in q)); q=[value/length for value in q]
        return scale, q, matrix[12:15]
    a, b = decompose(left), decompose(right)
    rotation = wmodel_pose.sample_quaternion([(0., *a[1]), (1., *b[1])], alpha)
    return wmodel_pose.affine_matrix(tuple(x+(y-x)*alpha for x,y in zip(a[0],b[0])), rotation,
                                    tuple(x+(y-x)*alpha for x,y in zip(a[2],b[2])))


def _sample_bone_bake_pose(actor, part, clip_name, source_seconds, root_vertical_scale=1.0, blend=None, local_only=False):
    model = actor[part]
    if model is None:
        raise CompositionError("Bone Collider actor has no weapon model")
    clips = [row for row in model.animations if row.name == clip_name] if clip_name else []
    if clip_name and len(clips) != 1:
        raise CompositionError(f"Bone Collider has a missing or ambiguous clip: {clip_name}")
    animation = clips[0] if clips else None
    if animation and (not math.isfinite(animation.ticks_per_second) or animation.ticks_per_second <= 0 or
                      not math.isfinite(animation.duration_ticks) or animation.duration_ticks <= 0):
        raise CompositionError("Bone Collider clip has invalid native timing")
    if animation and (part, clip_name) not in actor["validatedClips"]:
        # Match WAnimationReader admission and its normalized quaternion keys.
        for channel in animation.channels:
            for keys in (channel.position_keys, channel.rotation_keys, channel.scale_keys):
                previous = -1.0
                for row in keys:
                    if any(not math.isfinite(value) for value in row) or not previous <= row[0] <= animation.duration_ticks + .001 or row[0] < 0:
                        raise CompositionError("Bone Collider clip has invalid or unordered native keys")
                    previous = row[0]
            normalized = []
            for time, *quaternion in channel.rotation_keys:
                length = math.sqrt(sum(value * value for value in quaternion))
                if length <= .000001:
                    raise CompositionError("Bone Collider clip has an invalid quaternion key")
                normalized.append((time, *(value / length for value in quaternion)))
            channel.rotation_keys = normalized
        actor["validatedClips"].add((part, clip_name))
    ticks = min(source_seconds * animation.ticks_per_second, animation.duration_ticks) if animation else 0
    key = (part, clip_name, ticks, root_vertical_scale if part == "body" else 1.0, blend, local_only)
    if key in actor["poses"]:
        return actor["poses"][key]
    local = [list(bone.transform) for bone in model.skeleton_bones]
    if animation:
        for channel in animation.channels:
            local[channel.bone_index] = wmodel_pose.affine_matrix(
                wmodel_pose.sample_vector(channel.scale_keys, ticks, (1, 1, 1)),
                wmodel_pose.sample_quaternion(channel.rotation_keys, ticks),
                wmodel_pose.sample_vector(channel.position_keys, ticks, (0, 0, 0)))
    if local_only:
        return local
    if blend is not None:
        from_clip, from_seconds, alpha = blend
        prior = _sample_bone_bake_pose(actor, part, from_clip, from_seconds, local_only=True)
        local = [_blend_local_matrix(a, b, alpha) for a, b in zip(prior, local)]
    if part == "body":
        roots = [i for i, bone in enumerate(model.skeleton_bones) if bone.name == "b_root"]
        if len(roots) != 1:
            raise CompositionError("Bone Collider body lacks the b_root suppression anchor")
        # CNpc/CModel suppress local X/Y root translation and preserve vertical Z.
        for axis in (12, 13):
            local[roots[0]][axis] = model.skeleton_bones[roots[0]].transform[axis]
        rest_z = model.skeleton_bones[roots[0]].transform[14]
        local[roots[0]][14] = rest_z + (local[roots[0]][14] - rest_z) * root_vertical_scale
    combined = wmodel_pose.combined_transforms(model.skeleton_bones, local)
    pre = actor[part + "Pre"]
    combined = [wmodel_pose.matrix_multiply(matrix, pre) for matrix in combined]
    if any(not math.isfinite(value) for matrix in combined for value in matrix):
        raise CompositionError("Bone Collider sampled a non-finite transform")
    if len(actor["poses"]) >= 128:
        actor["poses"].pop(next(iter(actor["poses"])))
    actor["poses"][key] = combined
    return combined


def _bone_bake_stage_origins(pattern):
    elapsed_ticks = 0
    for index, stage in enumerate(pattern["stages"]):
        # Brain counts the initial entry tick, then advances subsequent stages
        # from the replicated action-start tick. Keep its existing schedule.
        yield stage, max(0, elapsed_ticks - (1 if index else 0)) * 1000 / FIXED_TICK_HZ
        elapsed_ticks += math.ceil(stage["durationMs"] * FIXED_TICK_HZ / 1000)


def _animation_holds_window_end(stage, animation):
    return (animation["startOffsetMs"] + animation["playMs"] < stage["durationMs"] or
            animation["endPolicy"] == "HOLD_LAST_POSE")


def _bone_bake_clip_sample(pattern, pattern_ms):
    stages = list(_bone_bake_stage_origins(pattern))
    selected = [(stage, start) for stage, start in stages if start <= pattern_ms + 1e-8]
    if not selected:
        raise CompositionError("Bone Collider samples before its first stage")
    stage, stage_start = selected[-1]
    matches = []
    for animation in stage["animationOccurrences"]:
        start = stage_start + animation["startOffsetMs"]
        end = start + animation["playMs"]
        # An external deadline may retain the last action after its stage ends.
        held_end = stage is stages[-1][0] and animation["startOffsetMs"] + animation["playMs"] == stage["durationMs"]
        holds_window = _animation_holds_window_end(stage, animation)
        held_gap = holds_window and len(stage["animationOccurrences"]) == 1
        if start <= pattern_ms + 1e-8 and (pattern_ms < end - 1e-8 or held_end or held_gap):
            age = pattern_ms - start
            if holds_window:
                age = min(age, animation["playMs"])
            matches.append((animation, age))
    if len(matches) != 1:
        raise CompositionError("Bone Collider samples an animation gap or overlapping clip windows")
    animation, age = matches[0]
    return animation, (animation["sourceStartMs"] + age * animation["playRate"]) / 1000


def _project_bone_collider_track(pattern, logic_box, collider, root, cache):
    if collider["anchorKind"] != "BOSS" or not collider["followBoss"] or not collider["bone"]:
        raise CompositionError("Bone Collider requires a following BOSS and a named bone")
    actor = _load_bone_bake_actor(pattern, root, cache)
    part = "weapon" if collider.get("boneTarget", "BODY") == "WEAPON" else "body"
    model = actor[part]
    if model is None:
        raise CompositionError("Bone Collider actor has no weapon model")
    bone_indices = [i for i, row in enumerate(model.skeleton_bones) if row.name == collider["bone"]]
    if len(bone_indices) != 1:
        raise CompositionError(f"Bone Collider bone is missing or ambiguous: {collider['bone']}")
    socket_indices = [i for i, row in enumerate(actor["body"].skeleton_bones) if row.name == "b_wp_1"]
    if part == "weapon" and (actor["actorProfileId"] != "MN_RPCT_06" or len(socket_indices) != 1):
        raise CompositionError("Weapon Bone Collider requires the supported Saydon hammer socket")
    start, duration = logic_box["startMs"], logic_box["durationMs"]
    end = start + duration
    sample_times = {0: float(start), duration: float(end)}
    for stage, stage_start in _bone_bake_stage_origins(pattern):
        for animation in stage["animationOccurrences"]:
            for edge in (stage_start + animation["startOffsetMs"], stage_start + animation["startOffsetMs"] + animation["playMs"]):
                for absolute in (math.floor(edge - 1e-7), math.ceil(edge)):
                    if start <= absolute <= end:
                        sample_times[absolute - start] = float(absolute)
    for tick in range(math.ceil(start * FIXED_TICK_HZ / 1000), math.floor(end * FIXED_TICK_HZ / 1000) + 1):
        absolute = tick * 1000 / FIXED_TICK_HZ
        local = absolute - start
        # The wire keys use integer milliseconds. Bracket a fractional fixed
        # tick with its identical pose, including stage-transition ticks.
        for value in (math.floor(local + 1e-8), math.ceil(local - 1e-8)):
            if 0 <= value <= duration:
                sample_times[value] = absolute
    if len(sample_times) > 4096:
        raise CompositionError("Bone Collider trigger needs more than 4096 baked keys; shorten its active window")
    keys = []
    for local_ms, pattern_ms in sorted(sample_times.items()):
        animation, seconds = _bone_bake_clip_sample(pattern, pattern_ms)
        if resolve_actor_profile_id(animation["profileId"]) != actor["actorProfileId"]:
            raise CompositionError("Bone Collider animation belongs to a different actor body")
        body_clip = animation["runtimeClip"]
        body_matches = [row for row in actor["body"].animations if row.name == body_clip]
        if len(body_matches) != 1:
            raise CompositionError(f"Bone Collider body clip is missing or ambiguous: {body_clip}")
        if not math.isfinite(body_matches[0].duration_ticks) or body_matches[0].duration_ticks <= 0 or not math.isfinite(body_matches[0].ticks_per_second) or body_matches[0].ticks_per_second <= 0:
            raise CompositionError("Bone Collider body clip has invalid native timing")
        native_seconds = body_matches[0].duration_ticks / body_matches[0].ticks_per_second
        if animation["endPolicy"] == "LOOP_TO_WINDOW":
            seconds %= native_seconds
        else:
            # Product plays the action without looping. A held final stage
            # keeps advancing its source clock until CAnimation's native end.
            seconds = min(seconds, native_seconds)
        blend = None
        previous = _animation_blend_source(pattern, animation)
        if previous:
            previous_animation, previous_seconds = previous
            current_start = next(origin + row["startOffsetMs"] for stage, origin in _bone_bake_stage_origins(pattern)
                                 for row in stage["animationOccurrences"] if row["occurrenceId"] == animation["occurrenceId"])
            alpha = min(1.0, max(0.0, (pattern_ms - current_start) / animation["blendInMs"]))
            if alpha < 1.0:
                blend = previous_animation["runtimeClip"], previous_seconds, alpha
        body_pose = _sample_bone_bake_pose(actor, "body", body_clip, seconds, pattern.get("animationRootVerticalScale", 1.0), blend)
        matrix = body_pose[bone_indices[0]] if part == "body" else None
        if part == "weapon":
            prefix = "mn_rpct_06_sk.ao_"
            if not body_clip.startswith(prefix):
                raise CompositionError("Weapon Bone Collider has an unsupported body clip family")
            suffix = body_clip[len(prefix):]
            if suffix.startswith(("att_battle_1_", "att_battle_3_")):
                suffix = suffix[:11] + "0" + suffix[11:]
            weapon_clip = "wprpct06_" + suffix
            if not any(row.name == weapon_clip for row in model.animations):
                weapon_clip = ""  # The live hammer uses its loaded rest pose for unmapped body clips.
            weapon_blend = None
            if blend:
                suffix = blend[0][len(prefix):]
                if suffix.startswith(("att_battle_1_", "att_battle_3_")):
                    suffix = suffix[:11] + "0" + suffix[11:]
                previous_weapon_clip = "wprpct06_" + suffix
                if not any(row.name == previous_weapon_clip for row in model.animations): previous_weapon_clip = ""
                weapon_blend = previous_weapon_clip, blend[1], blend[2]
            weapon_pose = _sample_bone_bake_pose(actor, "weapon", weapon_clip, seconds, blend=weapon_blend)
            matrix = wmodel_pose.matrix_multiply(weapon_pose[bone_indices[0]], body_pose[socket_indices[0]])
        position = matrix[12:15]
        _vector3(position, "Bone Collider boss-local tip", -100000, 100000)
        keys.append({"timeMs": local_ms, "positionOffset": position, "rotationY": 0.0, "rotationW": 1.0,
                     "scaleMultiplier": [1.0, 1.0, 1.0], "visible": True})
    return {"startMs": start, "startDelayMs": 0, "durationMs": duration, "playbackSpeed": 1.0,
            "interpolation": "LINEAR", "baselinePosition": [0.0, 0.0, 0.0], "baselineYawDegrees": 0.0,
            "baselineScale": [1.0, 1.0, 1.0], "keys": keys}


def _project_collider_regions(document, pattern, logic_box, logic, sequences, root, bone_cache=None):
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
        if kind == "OBJECT_CONTACT" and row["anchorKind"] == "BOSS" and not row["followBoss"]:
            raise CompositionError("OBJECT_CONTACT Collider must follow its boss anchor")
        if kind == "OBJECT_CONTACT" and row["anchorKind"] == "WORLD" and (row["bone"] or row.get("boneTarget", "BODY") != "BODY"):
            raise CompositionError("OBJECT_CONTACT WORLD Collider cannot use a body or weapon bone")
        if row["rotationDegrees"][0] != 0 or row["rotationDegrees"][2] != 0:
            raise CompositionError("XZ gameplay Collider supports only Y rotation")
        position, yaw, scale = list(row["positionOffset"]), row["rotationDegrees"][1], list(row["scale"])
        anchor = "BOSS_CURRENT"
        world_track = None
        if kind == "OBJECT_CONTACT" and row["bone"]:
            try:
                world_track = _project_bone_collider_track(pattern, logic_box, row, root, bone_cache if bone_cache is not None else {})
            except (ValueError, IndexError, ZeroDivisionError) as error:
                raise CompositionError(f"Cannot bake Bone Collider {row['occurrenceId']}: {error}") from error
        if row["anchorKind"] == "WORLD":
            world = next((w for w in document.get("worlds",[]) if w["worldId"] == row["worldId"]),None)
            world_box = _resolve_collider_world_occurrence(pattern, row)
            if world is None or world_box is None:
                raise CompositionError("Collider WORLD needs a definition and same-pattern WORLD box")
            end = logic_box["startMs"]+logic_box["durationMs"]
            if world_box["startMs"] > logic_box["startMs"] or world_box["startMs"]+world_box["durationMs"] < end:
                raise CompositionError("Collider judgement must remain inside its WORLD box lifetime")
            if kind in {"ENTER_AREA", "OBJECT_OVERLAP", "OBJECT_CONTACT"}:
                world_track = _project_region_world_track(root,document["areaId"],sequences,world,world_box)
            else:
                origin, world_yaw, world_scale = _sample_region_world(root,document["areaId"],sequences,world,world_box,end)
                position = [a+b for a,b in zip(origin,_rotate_y([a*b for a,b in zip(position,world_scale)],world_yaw))]
                yaw += world_yaw
                scale = [a*b for a,b in zip(scale,world_scale)]
            anchor = "BOSS_SPAWN" if "placement" not in world_box and world.get("anchorKind","NONE") == "BOSS_SPAWN" else "WORLD"
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
        if logic["outcomeKind"] == "FEAR":
            projected[-1]["presentationId"] = logic["logicId"]
        elif logic["outcomeKind"] == "MAX_HP_PERCENT_DAMAGE" and logic.get("pushRangeM", 0):
            projected[-1]["pushRangeM"] = float(logic["pushRangeM"])
            projected[-1]["pushMs"] = int(logic["pushMs"])
        elif logic["outcomeKind"] == "CAPTURE_PLAYER":
            projected[-1]["attachmentSlot"] = logic["attachmentSlot"]
            projected[-1]["gripLocalOffset"] = dict(logic["gripLocalOffset"])
        elif logic["outcomeKind"] == "PLAY_WORLD_OBJECT_MOTION":
            projected[-1]["targetWorldInstanceId"] = logic["targetWorldInstanceId"]
            projected[-1]["motionInstanceId"] = logic["motionInstanceId"]
        elif logic["outcomeKind"] == "PLAY_CONTACT_WORLD_OBJECT_MOTION":
            projected[-1]["contactMotions"] = [dict(row) for row in logic["contactMotions"]]
        elif logic["outcomeKind"] == "COMPLETE_LOGIC_WINDOW":
            projected[-1]["targetLogicOccurrenceId"] = logic["targetLogicOccurrenceId"]
            projected[-1]["contactTargetWorldOccurrenceId"] = logic.get("contactTargetWorldOccurrenceId", "")
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
        **({"holdLogicOccurrenceId": box["holdLogicOccurrenceId"]} if box.get("holdLogicOccurrenceId") else {}),
        **({"bossChargeDistanceM": logic["bossChargeDistanceM"]} if logic.get("bossChargeDistanceM", 0) else {}),
        **({"chargeYawOffsetDegrees": logic["chargeYawOffsetDegrees"]} if logic.get("chargeYawOffsetDegrees", 0) else {}),
        **({"rearmOnExit": True} if logic.get("rearmOnExit", False) else {}),
        **({"repeatAfterKnockback": True} if logic.get("repeatAfterKnockback", False) else {}),
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
        "endsPatternOnSuccess": bool(logic.get("endsPatternOnSuccess", False)) if kind in {"STAGGER_WINDOW", "COUNTER_WINDOW", "EXTERNAL_SIGNAL"} else False,
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
    bone_cache: dict[Any, Any] = {}
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
            for world_id in {box["worldId"] for box in world_boxes}:
                if "objectResourceId" in worlds[world_id]:
                    _validate_world_object_reference(worlds[world_id], world_sequences)
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
            if logic["logicType"] != "DURATION" and kind not in {"ENTER_AREA", "OBJECT_CONTACT"}:
                continue
            window = _project_logic_window(box, logic, logics, 0.0)
            linked_shields = kind == "STAGGER_WINDOW" and any(
                row.get("logicOccurrenceId") == box["occurrenceId"]
                for row in source.get("presentationOccurrences", []))
            if kind in {"ROULETTE_CARD_MATCH", "AREA_OVERLAP", "ENTER_AREA", "OBJECT_OVERLAP", "OBJECT_CONTACT"} or linked_shields:
                if world_sequences is None:
                    world_sequences = load_world_sequences(root, document["areaId"])
                window["cardRegions"] = _project_collider_regions(document, source, box, logic, world_sequences, root, bone_cache)
            if kind == "OBJECT_OVERLAP":
                window.update(_project_object_overlap_target(document, source, box, logic, logics, world_sequences))
            elif kind == "OBJECT_CONTACT":
                window.update(_project_object_contact_targets(document, source, box, logic, logics, world_sequences))
            logic_windows.append(window)
        mechanic_triggers = []
        for box in source.get("logicOccurrences", []):
            if not box.get("enabled", True):
                continue
            logic = logics[box["logicId"]]
            if logic["logicType"] != "TRIGGER" or "triggerKind" not in logic or logic["triggerKind"] in {"ENTER_AREA", "OBJECT_CONTACT"}:
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
                **_pattern_target_metadata(source),
                **({"folderId": source["folderId"]} if "folderId" in source else {}),
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
                **({"resetBossYawDegrees": float(source["resetBossYawDegrees"])}
                   if "resetBossYawDegrees" in source else {}),
                **({"bossMotion": copy.deepcopy(source["bossMotion"])} if "bossMotion" in source else {}),
                "logicWindows": logic_windows,
                "mechanicTriggers": mechanic_triggers,
                "worldSequences": [
                    {
                        "sequenceInstanceId": worlds[box["worldId"]]["sequenceInstanceId"],
                        "occurrenceId": box["occurrenceId"],
                        "startMs": box["startMs"],
                        "durationMs": box["durationMs"],
                        "playbackSpeed": float(box["playbackSpeed"]),
                        "positionOffset": list(worlds[box["worldId"]].get("positionOffset", [0.0, 0.0, 0.0])),
                        "anchorKind": worlds[box["worldId"]].get("anchorKind", "NONE"),
                        "anchorPosition": list(worlds[box["worldId"]].get("anchorPosition", [0.0, 0.0, 0.0])),
                        **_project_world_placement(box, worlds[box["worldId"]], world_sequences),
                        **_project_walkable_surface(root, document["areaId"], world_sequences, worlds[box["worldId"]], box),
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
        "folders": _project_folders(document),
        "bundles": _project_bundles(document),
    }


PRESENTATION_RESOURCE_REQUIRED = {"resourceId", "displayName", "kind", "assetId"}
PRESENTATION_RESOURCE_DEFAULTS = {
    "resourceKind": "GROUP", "elementId": "", "durationMs": 1000, "shape": "BOX", "colliderKind": "GEOMETRY",
    "halfExtents": [1.0, 1.0, 1.0], "radiusM": 3.0, "halfAngleDegrees": 45.0,
    "defaultAnchorKind": "BOSS",
}
PRESENTATION_OCCURRENCE_REQUIRED = {"occurrenceId", "resourceId", "startMs", "durationMs"}
PRESENTATION_OCCURRENCE_DEFAULTS = {
    "positionOffset": [0.0, 0.0, 0.0], "rotationDegrees": [0.0, 0.0, 0.0], "scale": [1.0, 1.0, 1.0],
    "fadeInMs": 0, "fadeOutMs": 0, "dissolveStart": 0.85, "dissolveEnd": 1.0,
    "volume": 1.0, "followBoss": True, "bone": "", "boneTarget": "BODY", "brightnessMultiplier": 1.0,
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
        if normalized["resourceKind"] not in ({"GROUP", "LEAF", "V1_EFFECT", "V1_ELEMENT"} if kind == "EFFECT" else {"", "GROUP", "LEAF"}):
            raise CompositionError("presentation resourceKind is unsupported")
        if normalized["resourceKind"] == "V1_ELEMENT":
            _stable_id(normalized["elementId"], "V1 Element ID")
        elif normalized["elementId"] != "":
            raise CompositionError("Only V1_ELEMENT owns elementId")
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


def _resolve_collider_world_occurrence(pattern, collider):
    candidates = [row for row in pattern.get("worldOccurrences", []) if row["worldId"] == collider.get("worldId")]
    identity = collider.get("worldOccurrenceId", "")
    if identity:
        candidates = [row for row in candidates if row["occurrenceId"] == identity]
    if len(candidates) != 1:
        raise CompositionError("WORLD Collider needs one exact same-pattern WORLD occurrence; select its occurrenceId when the World definition is reused")
    return candidates[0]


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
            kind = resources[box["resourceId"]]["kind"]
            if owner is None or kind not in {"EFFECT", "COLLIDER"}:
                raise CompositionError("worldOccurrenceId requires an EFFECT/COLLIDER and a same-pattern World box")
            if kind == "EFFECT":
                if worlds[owner["worldId"]].get("companionEffectResourceId", "") != box["resourceId"]:
                    raise CompositionError("linked Effect must match its World companionEffectResourceId")
                if world_occurrence_id in linked_world_ids:
                    raise CompositionError("a World box can have at most one linked companion Effect")
                linked_world_ids.add(world_occurrence_id)
            elif normalized["anchorKind"] != "WORLD" or normalized["worldId"] != owner["worldId"]:
                raise CompositionError("WORLD Collider occurrenceId must match its World definition and anchor")
        if resources[box["resourceId"]]["kind"] == "COLLIDER" and normalized["anchorKind"] == "WORLD":
            _resolve_collider_world_occurrence(pattern, normalized)
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
        if not isinstance(normalized["boneTarget"], str) or normalized["boneTarget"] not in {"BODY", "WEAPON"}:
            raise CompositionError("presentation boneTarget must be BODY or WEAPON")
        if normalized["boneTarget"] == "WEAPON" and (resources[box["resourceId"]]["kind"] not in {"COLLIDER", "EFFECT"} or
                normalized["anchorKind"] != "BOSS" or not normalized["bone"] or not normalized["followBoss"]):
            raise CompositionError("WEAPON boneTarget requires a following BOSS Collider/Effect and a named bone")


def _project_pattern_presentation(document: dict[str, Any], pattern: dict[str, Any]) -> dict[str, Any]:
    resources = {resource["resourceId"]: resource for resource in document.get("presentationResources", [])}
    occurrences = []
    for box in pattern.get("presentationOccurrences", []):
        resource = {**PRESENTATION_RESOURCE_DEFAULTS, **resources[box["resourceId"]]}
        occurrences.append({
            **{key: value for key, value in resource.items() if key not in {"displayName", "durationMs", "defaultAnchorKind"}},
            "resourceDurationMs": resource["durationMs"], **{key: value for key, value in PRESENTATION_OCCURRENCE_DEFAULTS.items() if key not in {"brightnessMultiplier", "boneTarget"}}, **box,
            **({"brightnessMultiplier": box.get("brightnessMultiplier", 1.0)} if resource["kind"] == "LIGHT" else {}),
            "worldSequenceInstanceId": next((w["sequenceInstanceId"] for w in document.get("worlds", []) if w["worldId"] == box.get("worldId", "")), ""),
            **({"worldOccurrenceId": _resolve_collider_world_occurrence(pattern, box)["occurrenceId"]}
               if resource["kind"] == "COLLIDER" and box.get("anchorKind", "BOSS") == "WORLD" else {}),
        })
    profiles = {profile["sceneProfileId"]: profile for profile in document.get("sceneProfiles", [])}
    for box in pattern.get("sceneProfileOccurrences", []):
        occurrences.append({
            **{key: value for key, value in PRESENTATION_RESOURCE_DEFAULTS.items() if key not in {"durationMs", "defaultAnchorKind"}},
            **{key: value for key, value in PRESENTATION_OCCURRENCE_DEFAULTS.items() if key not in {"brightnessMultiplier", "boneTarget"}},
            "occurrenceId": box["occurrenceId"], "resourceId": box["sceneProfileId"],
            "kind": "SCENE_PROFILE", "worldSequenceInstanceId": "", "assetId": profiles[box["sceneProfileId"]]["renderingProfileId"],
            "resourceDurationMs": box["durationMs"], "startMs": box["startMs"], "durationMs": box["durationMs"],
            "fadeInMs": box["blendMs"],
        })
    worlds = {world["worldId"]: world for world in document.get("worlds", [])}
    emission_anchors = [
        {"occurrenceId": box["occurrenceId"], "startMs": box["startMs"],
         "positionOffset": list(worlds[box["worldId"]].get("positionOffset", [0.0, 0.0, 0.0])),
         "anchorPosition": list(worlds[box["worldId"]].get("anchorPosition", [0.0, 0.0, 0.0]))}
        for box in pattern.get("worldOccurrences", [])
        if "bossMotion" in pattern and "placement" not in box
        and worlds[box["worldId"]].get("objectResourceId")
        and worlds[box["worldId"]].get("anchorKind", "NONE") == "BOSS_SPAWN"
    ]
    return {"patternId": pattern["patternId"], **_pattern_target_metadata(pattern), "durationMs": sum(stage["durationMs"] for stage in pattern["stages"]),
            **({"bossMotion": copy.deepcopy(pattern["bossMotion"])} if "bossMotion" in pattern else {}),
            **({"animationRootVerticalScale": pattern["animationRootVerticalScale"]} if pattern.get("animationRootVerticalScale", 1.0) != 1.0 else {}),
            **({"worldEmissionAnchors": emission_anchors} if emission_anchors else {}),
            "presentationOccurrences": occurrences}


def _join_light_resources(document: dict[str, Any], root: Path) -> int | None:
    resources = {r["resourceId"]: r for r in document.get("presentationResources", []) if r["kind"] == "LIGHT"}
    used = {resources[box["resourceId"]]["assetId"] for pattern in document["patterns"]
            if pattern.get("authoringStatus") == "PRODUCT"
            for box in pattern.get("presentationOccurrences", []) if box["resourceId"] in resources}
    used.update(resources[logic["lightResourceId"]]["assetId"] for logic in document.get("logics", [])
                if logic.get("outcomeKind") == "FEAR" and logic.get("lightResourceId") in resources)
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


def _project_fear_presentations(document: dict[str, Any]) -> list[dict[str, Any]]:
    profiles = {row["sceneProfileId"]: row for row in document.get("sceneProfiles", [])}
    resources = {row["resourceId"]: row for row in document.get("presentationResources", [])}
    result = []
    for logic in document.get("logics", []):
        if logic.get("outcomeKind") != "FEAR":
            continue
        scene_id = logic.get("sceneProfileId", "")
        effect_id = logic.get("effectResourceId", "")
        light_id = logic.get("lightResourceId", "")
        if scene_id and scene_id not in profiles:
            raise CompositionError(f"FEAR references missing Scene Profile: {scene_id}")
        if effect_id and (effect_id not in resources or resources[effect_id]["kind"] != "EFFECT"):
            raise CompositionError(f"FEAR references missing Effect resource: {effect_id}")
        if light_id and (light_id not in resources or resources[light_id]["kind"] != "LIGHT" or
                         resources[light_id].get("defaultAnchorKind", "BOSS") != "PLAYER"):
            raise CompositionError(f"FEAR requires a PLAYER Light resource: {light_id}")
        result.append({
            "presentationId": logic["logicId"],
            "sceneProfileId": profiles[scene_id]["renderingProfileId"] if scene_id else "",
            "effectResource": {**PRESENTATION_RESOURCE_DEFAULTS, **resources[effect_id],
                               "resourceDurationMs": resources[effect_id].get("durationMs", 1000)} if effect_id else None,
            "lightResource": {**PRESENTATION_RESOURCE_DEFAULTS, **resources[light_id],
                              "resourceDurationMs": resources[light_id].get("durationMs", 1000)} if light_id else None,
            "effectDelayMs": logic.get("effectDelayMs", 0),
            "durationMs": logic["durationMs"],
        })
    return result


def _project_attachment_grips(document: dict[str, Any]) -> list[dict[str, Any]]:
    definitions = {row["logicId"]: row for row in document.get("logics", [])}
    result = []
    for pattern in document["patterns"]:
        if pattern["authoringStatus"] != "PRODUCT":
            continue
        selected = None
        for box in pattern.get("logicOccurrences", []):
            if not box.get("enabled", True) or definitions[box["logicId"]].get("triggerKind") != "ENTER_AREA":
                continue
            for target in outcome_logic_ids(box, "Success"):
                logic = definitions[target]
                if logic.get("outcomeKind") != "CAPTURE_PLAYER":
                    continue
                row = {"patternId": pattern["patternId"], "attachmentSlot": logic["attachmentSlot"],
                       "gripLocalOffset": [logic["gripLocalOffset"][axis] for axis in ("forwardM", "upM", "rightM")]}
                if selected is not None and row != selected:
                    raise CompositionError(f"{pattern['patternId']} requires one consistent capture grip offset")
                selected = row
        if selected is not None:
            result.append(selected)
    return result


def project_presentation(document: dict[str, Any], root: Path = REPOSITORY_ROOT) -> dict[str, Any]:
    light_revision = _join_light_resources(document, root)
    bindings: list[dict[str, Any]] = []
    for pattern in document["patterns"]:
        if pattern["authoringStatus"] != "PRODUCT":
            continue
        definitions = {row["logicId"]: row for row in document.get("logics", [])}
        contact_ids = {row["occurrenceId"] for row in pattern.get("logicOccurrences", [])
                       if row.get("enabled", True) and definitions[row["logicId"]].get("triggerKind") == "OBJECT_CONTACT"}
        collider_ids = {row["resourceId"] for row in document.get("presentationResources", []) if row["kind"] == "COLLIDER"}
        unblended_bone_contact = any(row["resourceId"] in collider_ids and row.get("logicOccurrenceId") in contact_ids and
            row.get("anchorKind", "BOSS") == "BOSS" and row.get("bone") for row in pattern.get("presentationOccurrences", []))
        for stage in pattern["stages"]:
            for occurrence in stage["animationOccurrences"]:
                previous = _animation_blend_source(pattern, occurrence)
                transition = {"blendInMs": occurrence["blendInMs"], "blendFromClip": previous[0]["runtimeClip"],
                              "blendFromSourceMs": previous[1] * 1000} if previous else {}
                bindings.append(
                    {
                        "actionId": stage["actionId"],
                        "occurrenceId": occurrence["occurrenceId"],
                        "clip": occurrence["runtimeClip"],
                        **transition,
                        "startOffsetMs": occurrence["startOffsetMs"],
                        "sourceStartMs": occurrence["sourceStartMs"],
                        "playMs": occurrence["playMs"],
                        "playRate": occurrence["playRate"],
                        "endPolicy": occurrence["endPolicy"],
                        **({"holdAtWindowEnd": True} if _animation_holds_window_end(stage, occurrence) else {}),
                        **({"unblendedBoneContact": True} if unblended_bone_contact else {}),
                        **({"animationRootVerticalScale": pattern["animationRootVerticalScale"]} if pattern.get("animationRootVerticalScale", 1.0) != 1.0 else {}),
                    }
                )
    return {
        "schema": "lostark.kouku-saydon-pattern-bindings",
        "formatVersion": 1,
        **({"lightResourceRevision": light_revision} if light_revision is not None else {}),
        "bossArchetypeId": document["bossArchetypeId"],
        "sourceRevision": document["revision"],
        "bindings": bindings,
        "fearPresentations": _project_fear_presentations(document),
        "attachmentGrips": _project_attachment_grips(document),
        "folders": _project_folders(document),
        "bundles": _project_bundles(document, presentation=True),
        "patterns": [_project_pattern_presentation(document, pattern) for pattern in document["patterns"]
                     if pattern["authoringStatus"] == "PRODUCT"],
    }


def serialize_json(document: dict[str, Any]) -> bytes:
    return (
        json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    ).encode("utf-8")


def projected_outputs(document: dict[str, Any], root: Path = REPOSITORY_ROOT,
                      pattern_inventory: dict[str, Any] | None = None) -> dict[Path, bytes]:
    encounter = project_encounter(document, root)
    if pattern_inventory is not None:
        encounter["patternInventory"] = pattern_inventory
    return {
        ENCOUNTER_PATH: serialize_json(encounter),
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
    source = load_json(root / SOURCE_PATH)
    document, inventory = prepare_publication(source, root)
    outputs = projected_outputs(document, root, inventory)
    if mode == "publish":
        publish_outputs(root, outputs)
    else:
        validate_outputs(root, outputs)
    return {
        "compositionId": COMPOSITION_ID,
        "sourceRevision": document["revision"],
        "productPatternCount": len(document["playAllPatternIds"]),
        "savedPatternCount": len(inventory["patterns"]),
        "savedBundleCount": len(inventory["bundles"]),
        "productBundleCount": len(document["bundles"]),
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
