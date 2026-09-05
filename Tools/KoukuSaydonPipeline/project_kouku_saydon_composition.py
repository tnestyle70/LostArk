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
import sys
import tempfile
import uuid
from typing import Any, Iterable

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
SOURCE_PATH = Path("Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json")
ENCOUNTER_PATH = Path(
    "Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json"
)
PRESENTATION_PATH = Path(
    "Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json"
)
REFERENCE_ROOT = Path("Data/Animation/Reference/KoukuSaydon")
REFERENCE_MODEL_ASSET_IDS = {
    "MN_RPCT_05": "Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05",
    "MN_RPCT_06": "Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06",
    "MN_RPCT_07": "Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05",
    "MN_RPCZ_00": "Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00",
}

SCHEMA = "lostark.kouku-saydon-composition"
FORMAT_VERSION = 1
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

AUTHORING_STATUSES = {"DRAFT", "PRODUCT"}
PATTERN_CATEGORIES = {"NORMAL", "MECHANIC"}
STAGE_KINDS = {"WINDUP", "ACTIVE", "RECOVERY"}
END_POLICIES = {"EXACT", "HOLD_LAST_POSE", "LOOP_TO_WINDOW"}
STABLE_ID_RE = re.compile(r"^[A-Za-z0-9_.-]{1,128}$")
SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
GENERATED_PATTERN_RE = re.compile(r"^KAKULSAYDON_G1_PATTERN_([1-9][0-9]*)$")
GENERATED_STAGE_RE = re.compile(r"^STAGE_([1-9][0-9]*)$")
MAX_ORDINAL = 1_000_000
MAX_TIMELINE_MS = 600_000
MAX_PATTERNS = 4096
MAX_PRODUCT_PATTERNS = 64
MAX_STAGES = 64
MAX_OCCURRENCES = 4096


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


def validate_document(document: dict[str, Any], root: Path = REPOSITORY_ROOT) -> None:
    """Check persisted identities and timing; reference/oracle metadata is advisory."""

    _exact_keys(document, ROOT_KEYS, "composition")
    exact_values = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
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
    patterns = _array(document["patterns"], "composition patterns", MAX_PATTERNS)
    if not patterns:
        raise CompositionError("composition must contain at least one pattern")
    play_all = _array(
        document["playAllPatternIds"], "playAllPatternIds", MAX_PATTERNS
    )
    pattern_ids: set[str] = set()
    product_action_ids: set[str] = set()
    action_ids: set[str] = set()
    occurrence_ids: set[str] = set()
    product_ids: list[str] = []

    for pattern_index, pattern in enumerate(patterns):
        context = f"patterns[{pattern_index}]"
        _exact_keys(pattern, PATTERN_KEYS, context)
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
        next_stage = _integer(
            pattern["nextStageOrdinal"], f"{context} nextStageOrdinal", 1, MAX_ORDINAL
        )
        next_animation = _integer(
            pattern["nextAnimationOrdinal"],
            f"{context} nextAnimationOrdinal",
            1,
            MAX_ORDINAL,
        )
        stages = _array(pattern["stages"], f"{context} stages", MAX_STAGES)
        if status == "PRODUCT":
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
                if status == "PRODUCT" and profile_id != "MN_RPCZ_00":
                    raise CompositionError(
                        f"PRODUCT boss animation must use MN_RPCZ_00: {occurrence_id}"
                    )
                if profile_id not in REFERENCE_MODEL_ASSET_IDS:
                    raise CompositionError(f"unknown animation profile: {profile_id}")
                source_action_id = _integer(occurrence["sourceActionId"], f"{occurrence_context} sourceActionId", 0, 2**32 - 1)
                _stable_id(occurrence["runtimeClip"], f"{occurrence_context} runtimeClip")
                _stable_id(occurrence["sourceStageId"], f"{occurrence_context} sourceStageId")
                _stable_id(occurrence["sourceSlotId"], f"{occurrence_context} sourceSlotId")
                if source_action_id == 0 and occurrence["sourceStageId"] != "RAW":
                    raise CompositionError("sourceActionId 0 is reserved for physical RAW clips")
                if not isinstance(occurrence["referenceRevision"], str):
                    raise CompositionError("referenceRevision must be text")
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

    if play_all != product_ids:
        raise CompositionError(
            "playAllPatternIds must equal PRODUCT patternIds in authored order"
        )


def load_and_validate(root: Path = REPOSITORY_ROOT) -> dict[str, Any]:
    document = load_json(root / SOURCE_PATH)
    product = dict(document)
    product["patterns"] = [p for p in document.get("patterns", []) if isinstance(p, dict) and p.get("authoringStatus") == "PRODUCT"]
    product["playAllPatternIds"] = [p.get("patternId") for p in product["patterns"]]
    validate_document(product, root)
    return product


def validate_publishable(document: dict[str, Any]) -> None:
    if not document["playAllPatternIds"]:
        raise CompositionError(
            "composition must retain at least one PRODUCT pattern before publish"
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


def project_encounter(document: dict[str, Any]) -> dict[str, Any]:
    patterns: list[dict[str, Any]] = []
    for source in document["patterns"]:
        if source["authoringStatus"] != "PRODUCT":
            continue
        source_action_ids = list(
            dict.fromkeys(
                occurrence["sourceActionId"]
                for stage in source["stages"]
                for occurrence in stage["animationOccurrences"]
                if occurrence["sourceActionId"] != 0
            )
        )
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
        "playAllPatternIds": list(document["playAllPatternIds"]),
        "patterns": patterns,
    }


def project_presentation(document: dict[str, Any]) -> dict[str, Any]:
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
        "bossArchetypeId": document["bossArchetypeId"],
        "sourceRevision": document["revision"],
        "bindings": bindings,
    }


def serialize_json(document: dict[str, Any]) -> bytes:
    return (
        json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    ).encode("utf-8")


def projected_outputs(document: dict[str, Any]) -> dict[Path, bytes]:
    return {
        ENCOUNTER_PATH: serialize_json(project_encounter(document)),
        PRESENTATION_PATH: serialize_json(project_presentation(document)),
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
    validate_publishable(document)
    outputs = projected_outputs(document)
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
