#!/usr/bin/env python3
"""Validate and publish the shared boss-composition scheduling read model.

The current Valtan split documents remain the writable owners while its
composition is in SHADOW state.  This pipeline performs a strict join and
publishes one immutable read model whose revision covers every scheduling
owner, including Pattern Sound and Effect V2 bindings.  KakulSaydon remains
REFERENCE_ONLY until a gameplay authoring/Product pair exists.
"""

from __future__ import annotations

import argparse
import contextlib
import copy
import hashlib
import json
import math
import os
import re
import sys
import time
import uuid
from pathlib import Path, PurePosixPath
from typing import Any, Iterable, Mapping


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
if str(REPOSITORY_ROOT) not in sys.path:
    sys.path.insert(0, str(REPOSITORY_ROOT))

from Tools.ValtanPipeline import valtan_tuning_pipeline as valtan  # noqa: E402
from Tools.EffectToolV2 import effect_v2_binding_pipeline as effect_v2_pipeline  # noqa: E402
from Tools.KakulSaydonPipeline import (  # noqa: E402
    build_kakul_animation_reference as kakul_animation,
)


BOSS_SCHEMA = "lostark.boss-composition"
BOSS_PRODUCT_SCHEMA = "lostark.boss-composition-product"
SEQUENCER_SCHEMA = "lostark.arena-sequencer"
SEQUENCER_PRODUCT_SCHEMA = "lostark.arena-sequencer-product"
RECEIPT_SCHEMA = "lostark.composition-publish-receipt"
FORMAT_VERSION = 1
UINT32_MAX = (1 << 32) - 1
MAX_ARENA_DURATION_MS = 3_600_000
MAX_CUE_TIME_MS = 600_000
MAX_DOCUMENT_BYTES = 32 * 1024 * 1024
MAX_SOURCE_DOCUMENTS = 64
MAX_PATTERNS = 1024
MAX_REFERENCE_PROFILES = 64
MAX_TRACKS = 8192
MAX_STABLE_ID_BYTES = 255
WORLD_SEQUENCE_MAX_DURATION_MS = 600_000
WORLD_SEQUENCE_MAX_TEMPLATES = 256
WORLD_SEQUENCE_MAX_INSTANCES = 2048
WORLD_SEQUENCE_MAX_TRACKS = 32
WORLD_SEQUENCE_MAX_KEYS = 256
WORLD_SEQUENCE_MAX_COMPONENT = 100_000.0
WORLD_SEQUENCE_MIN_SCALE = 0.000001
CAMERA_SHOT_MAX_COUNT = 64
CAMERA_SHOT_MAX_BLEND_MS = 10_000
CAMERA_SHOT_MAX_PRIORITY = 1_000
CAMERA_SHOT_MAX_HALF_EXTENT = 1_000.0
CAMERA_SHOT_MAX_COORDINATE = 100_000.0
CAMERA_TRACK_MAX_DURATION_MS = 120_000
CAMERA_TRACK_MAX_KEYFRAMES = 64
PUBLISH_LOCK_TIMEOUT_SECONDS = 120.0
PUBLISH_RECEIPT_REL = "Composition.publish.receipt.json"
PUBLISH_JOURNAL_NAME = ".composition-publish.journal.json"
PUBLISH_JOURNAL_SCHEMA = "lostark.composition-publish-journal"
SOUND_CATALOG_REL = "Data/Sound/CharacterSoundCatalog.json"

VALTAN_SOURCE_DOCUMENTS = {
    "GAMEPLAY": valtan.GAMEPLAY_AUTHORING_REL,
    "PRESENTATION": valtan.PRESENTATION_AUTHORING_REL,
    "COMBAT_OBJECTS": valtan.COMBAT_AUTHORING_REL,
    "WORLD_EVENT_SETS": valtan.WORLD_SET_REL,
    "ANIMATION_BINDINGS": valtan.BINDINGS_REL,
    "EFFECT_V1_CUES": valtan.CUES_REL,
    "EFFECT_V1_ALIASES": valtan.EFFECT_V1_ALIASES_REL,
    "EFFECT_V2_BINDINGS": valtan.EFFECT_V2_BINDINGS_REL,
    "PATTERN_SOUND_CUES": valtan.PATTERN_SOUND_CUES_REL,
    "PATTERN_SHAKE_CUES": valtan.SHAKE_CUES_REL,
    "COMBAT_OBJECT_SOUND_CUES": valtan.COMBAT_OBJECT_SOUND_CUES_REL,
}

VALTAN_ARENA_SOURCE_DOCUMENTS = {
    "BOSS_COMPOSITION": "Data/Compositions/Bosses/Valtan.bosscomposition.json",
    "MAP_EFFECTS": (
        "Data/Maps/Authoring/LV_LUT_HEARTRB_ED/"
        "LV_LUT_HEARTRB_ED.mapeffects.json"
    ),
    "MAP_LIGHTS": (
        "Data/Maps/Authoring/LV_LUT_HEARTRB_ED/"
        "LV_LUT_HEARTRB_ED.maplights.json"
    ),
}

KAKUL_ARENA_SOURCE_DOCUMENTS = {
    "BOSS_COMPOSITION": (
        "Data/Compositions/Bosses/KakulSaydon.bosscomposition.json"
    ),
    "WORLD_SEQUENCES": (
        "Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/"
        "LV_LUT_MIDNIGHTC_ED.worldsequences.json"
    ),
    "CAMERA_SHOTS": (
        "Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/"
        "LV_LUT_MIDNIGHTC_ED.camerashots.json"
    ),
    "SCENE_PROFILES": "Data/Rendering/Authored/RenderingProfiles.json",
}

ARENA_CONTRACT_BY_BOSS_ID = {
    "boss.composition.valtan": {
        "sequencerId": "arena.sequencer.valtan",
        "areaId": "LV_LUT_HEARTRB_ED",
        "sources": VALTAN_ARENA_SOURCE_DOCUMENTS,
    },
    "boss.composition.kakulsaydon": {
        "sequencerId": "arena.sequencer.kakulsaydon",
        "areaId": "LV_LUT_MIDNIGHTC_ED",
        "sources": KAKUL_ARENA_SOURCE_DOCUMENTS,
    },
}

BOSS_AUTHORING = (
    "Data/Compositions/Bosses/Valtan.bosscomposition.json",
    "Data/Compositions/Bosses/KakulSaydon.bosscomposition.json",
)
SEQUENCER_AUTHORING = (
    "Data/Compositions/Sequences/ValtanArena.sequencer.json",
    "Data/Compositions/Sequences/KakulSaydonArena.sequencer.json",
)
ALL_AUTHORING = BOSS_AUTHORING + SEQUENCER_AUTHORING

PRODUCT_RELATIVE_PATHS = {
    BOSS_AUTHORING[0]: "Bosses/Valtan.bosscomposition.json",
    BOSS_AUTHORING[1]: "Bosses/KakulSaydon.bosscomposition.json",
    SEQUENCER_AUTHORING[0]: "Sequences/ValtanArena.sequencer.json",
    SEQUENCER_AUTHORING[1]: "Sequences/KakulSaydonArena.sequencer.json",
}

STATUS_VALUES = frozenset(("SHADOW", "REFERENCE_ONLY", "AUTHORITATIVE"))
TRACK_KINDS = frozenset(
    (
        "WORLD_SEQUENCE",
        "SCENE_PROFILE",
        "CAMERA_SHOT",
        "ACTOR_PATTERN",
        "EFFECT",
        "SOUND",
        "SCREEN_POST",
        "LIGHT",
        "PLAYER_SPAWN",
        "UI",
        "WORLD_EVENT",
    )
)
ADMITTED_TRACK_KINDS = frozenset(("WORLD_SEQUENCE", "CAMERA_SHOT", "ACTOR_PATTERN"))
COMPOSITION_ID_RE = re.compile(r"^boss\.composition\.[a-z0-9][a-z0-9.-]*$")
SEQUENCER_ID_RE = re.compile(r"^arena\.sequencer\.[a-z0-9][a-z0-9.-]*$")
TRACK_ID_RE = re.compile(r"^track\.[a-z0-9][a-z0-9._-]*$")
SOURCE_ROLE_RE = re.compile(r"^[A-Z][A-Z0-9_]*$")


class CompositionError(RuntimeError):
    """A stable, user-facing validation or transaction failure."""


def _reject_duplicate_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise CompositionError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def read_json(path: Path) -> dict[str, Any]:
    try:
        payload_bytes = path.read_bytes()
        if len(payload_bytes) > MAX_DOCUMENT_BYTES:
            raise CompositionError(
                f"JSON document exceeds {MAX_DOCUMENT_BYTES} bytes: {path}"
            )
        payload = json.loads(
            payload_bytes.decode("utf-8-sig"),
            object_pairs_hook=_reject_duplicate_keys,
        )
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise CompositionError(f"cannot read JSON {path}: {exc}") from exc
    if not isinstance(payload, dict):
        raise CompositionError(f"JSON root must be an object: {path}")
    return payload


def _require_exact_fields(
    value: Mapping[str, Any], required: Iterable[str], optional: Iterable[str], context: str
) -> None:
    required_set = set(required)
    allowed = required_set | set(optional)
    missing = sorted(required_set - set(value))
    unknown = sorted(set(value) - allowed)
    if missing or unknown:
        raise CompositionError(
            f"{context} field mismatch: missing={missing} unknown={unknown}"
        )


def _require_string(value: Any, context: str, *, allow_empty: bool = False) -> str:
    if not isinstance(value, str) or (not allow_empty and not value):
        raise CompositionError(f"{context} must be a non-empty string")
    return value


def _require_stable_id(value: Any, context: str) -> str:
    result = _require_string(value, context)
    if (
        len(result.encode("utf-8")) > MAX_STABLE_ID_BYTES
        or re.fullmatch(r"[A-Za-z0-9_.-]+", result) is None
    ):
        raise CompositionError(f"{context} is not a bounded stable ID")
    return result


def _require_owner_stable_id(value: Any, context: str, maximum_bytes: int) -> str:
    result = _require_stable_id(value, context)
    if len(result.encode("utf-8")) > maximum_bytes:
        raise CompositionError(
            f"{context} exceeds its owner stable-ID limit of {maximum_bytes} bytes"
        )
    return result


def _require_finite_number(
    value: Any,
    context: str,
    *,
    minimum: float | None = None,
    maximum: float | None = None,
) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise CompositionError(f"{context} must be a finite number")
    result = float(value)
    if not math.isfinite(result):
        raise CompositionError(f"{context} must be a finite number")
    if minimum is not None and result < minimum:
        raise CompositionError(f"{context} is below {minimum}")
    if maximum is not None and result > maximum:
        raise CompositionError(f"{context} exceeds {maximum}")
    return result


def _require_float3(
    value: Any,
    context: str,
    *,
    maximum_magnitude: float,
    positive: bool = False,
) -> list[float]:
    if not isinstance(value, list) or len(value) != 3:
        raise CompositionError(f"{context} must be a float3")
    result = [
        _require_finite_number(
            component,
            f"{context}[{ordinal}]",
            minimum=(0.0 if positive else -maximum_magnitude),
            maximum=maximum_magnitude,
        )
        for ordinal, component in enumerate(value)
    ]
    if positive and any(component <= 0.0 for component in result):
        raise CompositionError(f"{context} components must be greater than zero")
    return result


def _require_nonnegative_int(value: Any, context: str) -> int:
    if (
        isinstance(value, bool)
        or not isinstance(value, int)
        or value < 0
        or value > UINT32_MAX
    ):
        raise CompositionError(
            f"{context} must be a non-negative uint32 integer"
        )
    return value


def _require_positive_int(value: Any, context: str) -> int:
    result = _require_nonnegative_int(value, context)
    if result == 0:
        raise CompositionError(f"{context} must be greater than zero")
    return result


def _canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def _sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _safe_source_path(root: Path, raw_path: Any, context: str) -> tuple[str, Path]:
    relative = _require_string(raw_path, context)
    raw_parts = relative.split("/")
    if (
        "\\" in relative
        or ":" in relative
        or len(relative.encode("utf-8")) > 1024
        or any(part in ("", ".", "..") for part in raw_parts)
    ):
        raise CompositionError(f"{context} must use repository-relative '/' separators")
    pure = PurePosixPath(relative)
    if pure.is_absolute() or ".." in pure.parts or not pure.parts or pure.parts[0] != "Data":
        raise CompositionError(f"{context} escapes the repository Data root: {relative}")
    resolved_root = root.resolve()
    resolved = (resolved_root / Path(*pure.parts)).resolve()
    try:
        resolved.relative_to(resolved_root)
    except ValueError as exc:
        raise CompositionError(f"{context} escapes repository root: {relative}") from exc
    if not resolved.is_file():
        raise CompositionError(f"{context} does not exist: {relative}")
    return pure.as_posix(), resolved


def _validate_source_documents(
    root: Path, rows: Any, context: str
) -> tuple[dict[str, str], list[str]]:
    if (
        not isinstance(rows, list)
        or not rows
        or len(rows) > MAX_SOURCE_DOCUMENTS
    ):
        raise CompositionError(
            f"{context} must be a non-empty array <= {MAX_SOURCE_DOCUMENTS} rows"
        )
    by_role: dict[str, str] = {}
    paths: list[str] = []
    for index, row in enumerate(rows):
        row_context = f"{context}[{index}]"
        if not isinstance(row, dict):
            raise CompositionError(f"{row_context} must be an object")
        _require_exact_fields(row, ("role", "path"), (), row_context)
        role = _require_string(row["role"], f"{row_context}.role")
        if len(role.encode("utf-8")) > 127 or not SOURCE_ROLE_RE.fullmatch(role):
            raise CompositionError(f"{row_context}.role is not a stable role: {role}")
        path, _ = _safe_source_path(root, row["path"], f"{row_context}.path")
        if role in by_role:
            raise CompositionError(f"duplicate source role: {role}")
        if path in paths:
            raise CompositionError(f"duplicate source path: {path}")
        by_role[role] = path
        paths.append(path)
    return by_role, paths


def _validate_common_root(
    document: Mapping[str, Any], context: str, expected_schema: str
) -> str:
    if document.get("schema") != expected_schema:
        raise CompositionError(f"{context}.schema must be {expected_schema}")
    format_version = document.get("formatVersion")
    if (
        isinstance(format_version, bool)
        or not isinstance(format_version, int)
        or format_version != FORMAT_VERSION
    ):
        raise CompositionError(f"{context}.formatVersion must be {FORMAT_VERSION}")
    status = document.get("status")
    if status not in STATUS_VALUES:
        raise CompositionError(f"{context}.status is invalid: {status}")
    _require_positive_int(document.get("revision"), f"{context}.revision")
    display_name = _require_string(
        document.get("displayName"), f"{context}.displayName"
    )
    if len(display_name.encode("utf-8")) > 1024 or any(
        ord(character) < 0x20 and character != "\t" for character in display_name
    ):
        raise CompositionError(f"{context}.displayName is invalid")
    _require_stable_id(document.get("areaId"), f"{context}.areaId")
    return str(status)


def _valtan_join(root: Path, sources: Mapping[str, str]) -> dict[str, Any]:
    required = {
        "GAMEPLAY",
        "PRESENTATION",
        "COMBAT_OBJECTS",
        "WORLD_EVENT_SETS",
        "EFFECT_V1_CUES",
        "EFFECT_V2_BINDINGS",
        "PATTERN_SOUND_CUES",
    }
    missing = sorted(required - set(sources))
    if missing:
        raise CompositionError(f"Valtan composition is missing source roles: {missing}")
    try:
        return valtan.join_v2_authoring(
            read_json(root / sources["GAMEPLAY"]),
            read_json(root / sources["PRESENTATION"]),
            read_json(root / sources["WORLD_EVENT_SETS"]),
            read_json(root / sources["COMBAT_OBJECTS"]),
        )
    except (CompositionError, valtan.PipelineError) as exc:
        raise CompositionError(f"Valtan split join failed: {exc}") from exc


def _valtan_identity(joined: Mapping[str, Any]) -> tuple[int, int, str]:
    identity = [
        {
            "patternId": pattern["patternId"],
            "stages": [
                {"stageId": stage["stageId"], "actionId": stage["actionId"]}
                for stage in pattern["stages"]
            ],
        }
        for pattern in joined["patterns"]
    ]
    return (
        len(identity),
        sum(len(row["stages"]) for row in identity),
        _sha256_bytes(_canonical_bytes(identity)),
    )


def _cue_scope(pattern_id: str, stage: Mapping[str, Any]) -> dict[str, Any]:
    return {
        "patternId": pattern_id,
        "stageId": stage["stageId"],
        "actionId": stage["actionId"],
    }


def _root_anchor() -> dict[str, Any]:
    return {
        "kind": "BOSS_ROOT",
        "slotId": "root",
        "followPolicy": "FOLLOW",
        "rotationBasis": "BOSS_YAW",
        "localTransform": {
            "translation": [0.0, 0.0, 0.0],
            "rotation": [0.0, 0.0, 0.0],
            "scale": [1.0, 1.0, 1.0],
        },
    }


def _clock(
    *, basis: str, start_ms: int, repeat_policy: str, clip_occurrence_id: Any = None
) -> dict[str, Any]:
    result: dict[str, Any] = {
        "basis": basis,
        "startMs": start_ms,
        "repeatPolicy": repeat_policy.upper(),
    }
    if isinstance(clip_occurrence_id, str) and clip_occurrence_id:
        result["clipOccurrenceId"] = clip_occurrence_id
    return result


def _normalized_animation_cues(
    pattern_id: str, stage: Mapping[str, Any]
) -> list[dict[str, Any]]:
    result = []
    animation = stage["animation"]
    timing = _animation_occurrence_timing(stage)
    for occurrence in animation.get("occurrences", []):
        occurrence_id = occurrence["clipOccurrenceId"]
        start_ms, wall_ms, _source_start, _play_rate = timing[occurrence_id]
        payload = copy.deepcopy(occurrence)
        payload["wallDurationMs"] = wall_ms
        result.append(
            {
                "cueId": "cue.animation." + occurrence_id,
                "kind": "ANIMATION",
                "scope": _cue_scope(pattern_id, stage),
                "clock": _clock(
                    basis="STAGE",
                    start_ms=start_ms,
                    repeat_policy=(
                        "EACH_LOOP"
                        if occurrence.get("repeatUntilStageEnd")
                        else "ONCE"
                    ),
                ),
                "stopPolicy": (
                    "STAGE_END"
                    if occurrence.get("repeatUntilStageEnd")
                    or animation["endPolicy"] == "LOOP_TO_STAGE_END"
                    else "NATURAL"
                ),
                "anchor": _root_anchor(),
                "payload": payload,
            }
        )
    return result


def _animation_occurrence_timing(
    stage: Mapping[str, Any]
) -> dict[str, tuple[int, int, int, float]]:
    result: dict[str, tuple[int, int, int, float]] = {}
    cursor_ms = 0
    for occurrence in stage["animation"].get("occurrences", []):
        play_ms = occurrence["playMs"]
        play_rate = occurrence["playRate"]
        wall_ms = (
            max(0, stage["durationMs"] - cursor_ms)
            if play_ms == 0
            else int(math.floor((play_ms / play_rate) + 0.5))
        )
        result[occurrence["clipOccurrenceId"]] = (
            cursor_ms,
            wall_ms,
            occurrence["sourceStartMs"],
            play_rate,
        )
        cursor_ms += wall_ms
    return result


def _resolve_clip_source_ms(
    stage: Mapping[str, Any], clip_occurrence_id: str, source_ms: int
) -> int:
    timing = _animation_occurrence_timing(stage).get(clip_occurrence_id)
    if timing is None:
        raise CompositionError(
            f"unknown clipOccurrenceId in normalized cue: {clip_occurrence_id}"
        )
    occurrence_start, _wall_ms, source_start, play_rate = timing
    local_source_ms = max(0, source_ms - source_start)
    local_wall_ms = int(math.floor((local_source_ms / play_rate) + 0.5))
    return min(stage["durationMs"], occurrence_start + local_wall_ms)


def _normalized_v1_cues(
    pattern_id: str, stage: Mapping[str, Any]
) -> list[dict[str, Any]]:
    result = []
    for cue in stage["effectCues"]:
        clip_id = cue.get("clipOccurrenceId")
        stage_clock = cue.get("timingBasis") == "STAGE_CLOCK"
        source_clock_start = (
            cue.get("stageOffsetMs", 0)
            if stage_clock
            else cue.get("sourceStartMs", 0)
        )
        start_ms = (
            source_clock_start
            if stage_clock
            else _resolve_clip_source_ms(stage, clip_id, source_clock_start)
        )
        local = cue.get("localTransform", {})
        anchor = {
            "kind": "BOSS_SLOT",
            "slotId": cue.get("anchorSlotId", "root"),
            "followPolicy": str(cue.get("followPolicy", "follow")).upper(),
            "rotationBasis": "BOSS_YAW",
            "localTransform": {
                "translation": copy.deepcopy(local.get("position", [0, 0, 0])),
                "rotation": copy.deepcopy(local.get("rotationDegrees", [0, 0, 0])),
                "scale": copy.deepcopy(local.get("scale", [1, 1, 1])),
            },
        }
        payload = {
            key: copy.deepcopy(value)
            for key, value in cue.items()
            if key
            not in {
                "cueId",
                "occurrenceId",
                "clipOccurrenceId",
                "sourceStartMs",
                "sourceEndMs",
                "timingBasis",
                "stageOffsetMs",
                "anchorSlotId",
                "followPolicy",
                "stopPolicy",
                "repeatPolicy",
                "localTransform",
            }
        }
        payload["sourceClock"] = {
            "basis": "STAGE" if stage_clock else "CLIP_OCCURRENCE",
            "startMs": source_clock_start,
        }
        if clip_id:
            payload["sourceClock"]["clipOccurrenceId"] = clip_id
        normalized = {
            "cueId": cue["cueId"],
            "kind": "EFFECT_V1",
            "scope": _cue_scope(pattern_id, stage),
            "clock": _clock(
                basis="STAGE",
                start_ms=start_ms,
                repeat_policy=cue["repeatPolicy"],
                clip_occurrence_id=clip_id,
            ),
            "stopPolicy": str(cue["stopPolicy"]).upper(),
            "anchor": anchor,
            "payload": payload,
        }
        if cue.get("sourceEndMs") is not None:
            normalized["endMs"] = _resolve_clip_source_ms(
                stage, clip_id, cue["sourceEndMs"]
            )
        result.append(normalized)
    return result


def _normalized_camera_cues(
    pattern_id: str, stage: Mapping[str, Any]
) -> list[dict[str, Any]]:
    result = []
    for cue in stage["cameraInvocations"]:
        payload = copy.deepcopy(cue)
        start_ms = payload.pop("startOffsetMs")
        duration = payload.get("durationMs")
        normalized = {
            "cueId": cue["cameraInvocationId"],
            "kind": "CAMERA",
            "scope": _cue_scope(pattern_id, stage),
            "clock": _clock(basis="STAGE", start_ms=start_ms, repeat_policy="ONCE"),
            "stopPolicy": (
                "EXPLICIT" if cue["durationPolicy"] == "EXPLICIT" else "STAGE_END"
            ),
            "anchor": {"kind": "CAMERA_RIG"},
            "payload": payload,
        }
        if isinstance(duration, int):
            normalized["endMs"] = start_ms + duration
        result.append(normalized)
    return result


def _scope_key(value: Mapping[str, Any]) -> tuple[Any, Any, Any]:
    scope = value.get("scope", value)
    return (scope.get("patternId"), scope.get("stageId"), scope.get("actionId"))


def _normalized_external_cue(
    row: Mapping[str, Any],
    kind: str,
    *,
    stage: Mapping[str, Any] | None = None,
    detached: bool = False,
) -> dict[str, Any]:
    scope_value = row.get("scope", row)
    scope = {
        "patternId": scope_value.get("patternId"),
        "stageId": scope_value.get("stageId"),
        "actionId": scope_value.get("actionId"),
    }
    if kind == "EFFECT_V2":
        clock_value = row["clock"]
        source_basis = clock_value["basis"]
        source_start = clock_value["startMs"]
        clip_id = clock_value.get("clipOccurrenceId")
        start_ms = (
            _resolve_clip_source_ms(stage, clip_id, source_start)
            if stage is not None and source_basis == "CLIP_OCCURRENCE"
            else source_start
        )
        anchor = copy.deepcopy(row["anchor"])
        anchor["kind"] = "BOSS_SLOT"
        result = {
            "cueId": row["bindingId"],
            "kind": kind,
            "scope": scope,
            "clock": _clock(
                basis="STAGE" if stage is not None else source_basis,
                start_ms=start_ms,
                repeat_policy=clock_value["repeatPolicy"],
                clip_occurrence_id=clock_value.get("clipOccurrenceId"),
            ),
            "stopPolicy": row["stopPolicy"],
            "anchor": anchor,
            "payload": {
                "resource": copy.deepcopy(row["resource"]),
                "sourceClock": copy.deepcopy(clock_value),
            },
        }
    else:
        clip_id = row.get("clipOccurrenceId")
        source_start = row["startMs"]
        start_ms = (
            _resolve_clip_source_ms(stage, clip_id, source_start)
            if stage is not None and clip_id
            else source_start
        )
        result = {
            "cueId": row["bindingId"],
            "kind": kind,
            "scope": scope,
            "clock": _clock(
                basis=("STAGE" if stage is not None else (
                    "CLIP_OCCURRENCE" if clip_id else "STAGE"
                )),
                start_ms=start_ms,
                repeat_policy=row["repeatPolicy"],
                clip_occurrence_id=clip_id,
            ),
            "stopPolicy": "NATURAL",
            "anchor": _root_anchor(),
            "payload": {
                key: copy.deepcopy(value)
                for key, value in row.items()
                if key
                not in {
                    "bindingId",
                    "occurrenceId",
                    "patternId",
                    "stageId",
                    "actionId",
                    "clipOccurrenceId",
                    "repeatPolicy",
                    "startMs",
                }
            },
        }
        result["payload"]["sourceClock"] = {
            "basis": "CLIP_OCCURRENCE" if clip_id else "STAGE",
            "startMs": source_start,
        }
        if clip_id:
            result["payload"]["sourceClock"]["clipOccurrenceId"] = clip_id
    if detached:
        result["detachedReason"] = "OUTSIDE_MANAGED_PATTERN_GRAPH"
    return result


def _hit_start_ms(hit: Mapping[str, Any]) -> int:
    activation = hit.get("activation")
    if isinstance(activation, dict):
        for field in ("startMs", "atMs", "firstOffsetMs"):
            if isinstance(activation.get(field), int):
                return activation[field]
    schedule = hit.get("schedule")
    if isinstance(schedule, dict):
        if isinstance(schedule.get("firstOffsetMs"), int):
            return schedule["firstOffsetMs"]
        offsets = schedule.get("offsetsMs")
        if isinstance(offsets, list) and offsets:
            return min(value for value in offsets if isinstance(value, int))
    return 0


def _normalized_gameplay_cues(
    pattern_id: str, stage: Mapping[str, Any]
) -> list[dict[str, Any]]:
    result = []
    hit = stage["hit"]
    if hit["shape"]["kind"] != "NONE":
        result.append(
            {
                "cueId": f"cue.hit.{pattern_id.lower()}.{stage['stageId'].lower()}",
                "kind": "HIT",
                "scope": _cue_scope(pattern_id, stage),
                "clock": _clock(
                    basis="STAGE",
                    start_ms=_hit_start_ms(hit),
                    repeat_policy="ONCE",
                ),
                "stopPolicy": "STAGE_END",
                "anchor": {"kind": "BOSS_BODY"},
                "payload": copy.deepcopy(hit),
            }
        )
    for ordinal, event in enumerate(stage["events"]):
        event_id = event.get("eventId") or (
            f"cue.logic.{pattern_id.lower()}.{stage['stageId'].lower()}.{ordinal:02d}"
        )
        event_kind = str(event.get("kind", "LOGIC"))
        cue_kind = (
            "COMBAT_OBJECT"
            if event_kind.startswith("SPAWN_COMBAT_OBJECT")
            else "LOGIC"
        )
        start_ms = event.get("atMs") if isinstance(event.get("atMs"), int) else 0
        result.append(
            {
                "cueId": event_id,
                "kind": cue_kind,
                "scope": _cue_scope(pattern_id, stage),
                "clock": _clock(
                    basis="STAGE", start_ms=start_ms, repeat_policy="ONCE"
                ),
                "stopPolicy": "STAGE_END",
                "anchor": {"kind": "SERVER_RESOLVED"},
                "payload": copy.deepcopy(event),
            }
        )
    return result


def _normalized_detached_v1_cue(row: Mapping[str, Any]) -> dict[str, Any]:
    context = "detached Valtan Effect V1 cue"
    cue_id = _require_string(row.get("bindingId"), f"{context}.bindingId")
    scope = {
        "patternId": _require_string(row.get("patternId"), f"{cue_id}.patternId"),
        "stageId": _require_string(row.get("stageId"), f"{cue_id}.stageId"),
        "actionId": _require_string(row.get("actionId"), f"{cue_id}.actionId"),
    }
    stage_clock = row.get("timingBasis") == "STAGE_CLOCK"
    source_start = row.get("stageOffsetMs") if stage_clock else row.get("sourceStartMs")
    source_start = _require_nonnegative_int(source_start, f"{cue_id}.sourceStartMs")
    clip_id = None if stage_clock else _require_string(
        row.get("clipOccurrenceId"), f"{cue_id}.clipOccurrenceId"
    )
    local = row.get("localTransform")
    if not isinstance(local, dict):
        raise CompositionError(f"{cue_id}.localTransform must be an object")
    payload = {
        "effectAssetId": _require_string(
            row.get("effectAssetId"), f"{cue_id}.effectAssetId"
        ),
        "sourceClock": {
            "basis": "STAGE" if stage_clock else "CLIP_OCCURRENCE",
            "startMs": source_start,
        },
    }
    if clip_id is not None:
        payload["sourceClock"]["clipOccurrenceId"] = clip_id
    if "scalePolicy" in row:
        payload["scalePolicy"] = copy.deepcopy(row["scalePolicy"])
    result = {
        "cueId": cue_id,
        "kind": "EFFECT_V1",
        "scope": scope,
        "clock": _clock(
            basis="STAGE" if stage_clock else "CLIP_OCCURRENCE",
            start_ms=source_start,
            repeat_policy=_require_string(
                row.get("repeatPolicy"), f"{cue_id}.repeatPolicy"
            ),
            clip_occurrence_id=clip_id,
        ),
        "stopPolicy": _require_string(
            row.get("stopPolicy"), f"{cue_id}.stopPolicy"
        ).upper(),
        "anchor": {
            "kind": "BOSS_SLOT",
            "slotId": _require_string(
                row.get("anchorSlotId"), f"{cue_id}.anchorSlotId"
            ),
            "followPolicy": _require_string(
                row.get("followPolicy"), f"{cue_id}.followPolicy"
            ).upper(),
            "rotationBasis": "BOSS_YAW",
            "localTransform": {
                "translation": copy.deepcopy(local.get("position")),
                "rotation": copy.deepcopy(local.get("rotationDegrees")),
                "scale": copy.deepcopy(local.get("scale")),
            },
        },
        "payload": payload,
        "detachedReason": "OUTSIDE_MANAGED_PATTERN_GRAPH",
    }
    source_end = row.get("sourceEndMs")
    if source_end is not None:
        result["endMs"] = _require_nonnegative_int(
            source_end, f"{cue_id}.sourceEndMs"
        )
    return result


def _load_valtan_sound_events(root: Path) -> set[str]:
    catalog = read_json(root / SOUND_CATALOG_REL)
    _require_exact_fields(
        catalog, ("formatVersion", "classes"), (), "Character Sound catalog"
    )
    if (
        isinstance(catalog["formatVersion"], bool)
        or catalog["formatVersion"] != 1
        or not isinstance(catalog["classes"], dict)
    ):
        raise CompositionError("Character Sound catalog header/version is invalid")
    valtan_events = catalog["classes"].get("Valtan")
    if not isinstance(valtan_events, dict):
        raise CompositionError("Character Sound catalog has no Valtan event bank")
    result: set[str] = set()
    for event_id, variants in valtan_events.items():
        stable_event_id = _require_string(
            event_id, "Character Sound catalog Valtan eventId"
        )
        if stable_event_id in result:
            raise CompositionError(
                f"duplicate Character Sound catalog Valtan eventId: {stable_event_id}"
            )
        if not isinstance(variants, list) or any(
            not isinstance(path, str) or not path for path in variants
        ):
            raise CompositionError(
                f"Character Sound catalog Valtan event variants are invalid: {stable_event_id}"
            )
        result.add(stable_event_id)
    return result


def _expected_valtan_sound_bank(event_id: str) -> str | None:
    if event_id.startswith("G_Voltan1_"):
        return "S_Mob_G_Voltan1"
    if event_id.startswith("G_Voltan2_"):
        return "S_Mob_G_Voltan2"
    return None


def _validate_pattern_sound_document(
    document: Mapping[str, Any],
    animation_bindings: Mapping[str, Any],
    sound_events: set[str],
) -> None:
    _require_exact_fields(
        document,
        ("schema", "formatVersion", "ownerArchetypeId", "cues"),
        (),
        "Valtan Pattern Sound source",
    )
    if (
        document.get("schema") != "lostark.valtan-pattern-sound-cues"
        or document.get("formatVersion") != 1
        or isinstance(document.get("formatVersion"), bool)
        or document.get("ownerArchetypeId") != "BOSS_VALTAN"
    ):
        raise CompositionError("Valtan Pattern Sound source header/version is invalid")
    bindings = animation_bindings.get("bindings")
    if not isinstance(bindings, list):
        raise CompositionError("Valtan Animation bindings must be an array")
    clips_by_action: dict[str, dict[str, Mapping[str, Any]]] = {}
    for ordinal, binding in enumerate(bindings):
        if not isinstance(binding, dict):
            raise CompositionError(
                f"Valtan Animation bindings[{ordinal}] must be an object"
            )
        action_id = _require_string(
            binding.get("actionId"), f"Valtan Animation bindings[{ordinal}].actionId"
        )
        if action_id in clips_by_action:
            raise CompositionError(f"duplicate Animation actionId: {action_id}")
        clips = binding.get("clips")
        if not isinstance(clips, list):
            raise CompositionError(f"Animation binding {action_id}.clips must be an array")
        indexed: dict[str, Mapping[str, Any]] = {}
        for clip_ordinal, clip in enumerate(clips):
            if not isinstance(clip, dict):
                raise CompositionError(
                    f"Animation binding {action_id}.clips[{clip_ordinal}] must be an object"
                )
            clip_id = _require_string(
                clip.get("clipOccurrenceId"),
                f"Animation binding {action_id}.clips[{clip_ordinal}].clipOccurrenceId",
            )
            if clip_id in indexed:
                raise CompositionError(f"duplicate Animation clipOccurrenceId: {clip_id}")
            indexed[clip_id] = clip
        clips_by_action[action_id] = indexed

    cues = document.get("cues")
    if not isinstance(cues, list) or not cues or len(cues) > 1024:
        raise CompositionError("Valtan Pattern Sound cues must contain 1..1024 rows")
    binding_ids: set[str] = set()
    occurrence_ids: set[str] = set()
    for ordinal, row in enumerate(cues):
        context = f"Valtan Pattern Sound cues[{ordinal}]"
        if not isinstance(row, dict):
            raise CompositionError(f"{context} must be an object")
        _require_exact_fields(
            row,
            (
                "bindingId",
                "occurrenceId",
                "patternId",
                "stageId",
                "actionId",
                "clipOccurrenceId",
                "soundBank",
                "soundEvent",
                "repeatPolicy",
                "startMs",
            ),
            (),
            context,
        )
        binding_id = _require_string(row["bindingId"], f"{context}.bindingId")
        occurrence_id = _require_string(
            row["occurrenceId"], f"{context}.occurrenceId"
        )
        if binding_id in binding_ids:
            raise CompositionError(f"duplicate Pattern Sound bindingId: {binding_id}")
        if occurrence_id in occurrence_ids:
            raise CompositionError(
                f"duplicate Pattern Sound occurrenceId: {occurrence_id}"
            )
        binding_ids.add(binding_id)
        occurrence_ids.add(occurrence_id)
        for field in ("patternId", "stageId", "soundBank", "soundEvent"):
            _require_string(row[field], f"{binding_id}.{field}")
        if row["soundEvent"] not in sound_events:
            raise CompositionError(
                f"{binding_id}.soundEvent does not resolve in the Valtan Sound catalog: "
                f"{row['soundEvent']}"
            )
        expected_bank = _expected_valtan_sound_bank(row["soundEvent"])
        if expected_bank is None or row["soundBank"] != expected_bank:
            raise CompositionError(
                f"{binding_id}.soundBank does not match {row['soundEvent']}"
            )
        action_id = _require_string(row["actionId"], f"{binding_id}.actionId")
        clip_id = _require_string(
            row["clipOccurrenceId"], f"{binding_id}.clipOccurrenceId"
        )
        clip = clips_by_action.get(action_id, {}).get(clip_id)
        if clip is None:
            raise CompositionError(
                f"{binding_id} does not resolve an Animation clip occurrence"
            )
        repeat_policy = row["repeatPolicy"]
        if repeat_policy not in ("once", "each_loop"):
            raise CompositionError(f"{binding_id}.repeatPolicy is invalid")
        if repeat_policy == "each_loop" and clip.get("loop") is not True:
            raise CompositionError(f"{binding_id} each_loop targets a non-loop clip")
        start_ms = _require_nonnegative_int(row["startMs"], f"{binding_id}.startMs")
        if start_ms > 60_000:
            raise CompositionError(f"{binding_id}.startMs exceeds 60000")
        source_start_ms = clip.get("sourceStartMs")
        play_ms = clip.get("playMs")
        if (
            isinstance(source_start_ms, bool)
            or not isinstance(source_start_ms, int)
            or source_start_ms < 0
            or isinstance(play_ms, bool)
            or not isinstance(play_ms, int)
            or play_ms < 0
        ):
            raise CompositionError(
                f"{binding_id} references an invalid Animation source window"
            )
        if start_ms < source_start_ms or (
            play_ms != 0 and start_ms >= source_start_ms + play_ms
        ):
            raise CompositionError(
                f"{binding_id}.startMs is outside its Animation clip occurrence"
            )


def _validate_combat_object_sound_document(
    document: Mapping[str, Any],
    sound_events: set[str],
    combat_objects: Mapping[str, Any],
) -> None:
    _require_exact_fields(
        document,
        ("schema", "formatVersion", "ownerArchetypeId", "cues"),
        (),
        "Valtan Combat Object Sound source",
    )
    if (
        document.get("schema") != "lostark.valtan-combat-object-sound-cues"
        or isinstance(document.get("formatVersion"), bool)
        or document.get("formatVersion") != 1
        or document.get("ownerArchetypeId") != "BOSS_VALTAN"
    ):
        raise CompositionError(
            "Valtan Combat Object Sound source header/version is invalid"
        )
    rows = document.get("cues")
    if not isinstance(rows, list) or len(rows) > 256:
        raise CompositionError(
            "Valtan Combat Object Sound cues must be an array <= 256 rows"
        )
    binding_ids: set[str] = set()
    target_index: dict[str, tuple[set[str], set[str]]] = {}
    for object_ordinal, combat_object in enumerate(combat_objects.get("objects", [])):
        if not isinstance(combat_object, dict):
            raise CompositionError(
                f"Valtan combat objects[{object_ordinal}] must be an object"
            )
        archetype_id = _require_string(
            combat_object.get("combatObjectArchetypeId"),
            f"Valtan combat objects[{object_ordinal}].combatObjectArchetypeId",
        )
        if archetype_id in target_index:
            raise CompositionError(f"duplicate combatObjectArchetypeId: {archetype_id}")
        target_index[archetype_id] = (
            {
                row.get("hitId")
                for row in combat_object.get("hits", [])
                if isinstance(row, dict) and isinstance(row.get("hitId"), str)
            },
            {
                row.get("presentationEventId")
                for row in combat_object.get("presentationEvents", [])
                if isinstance(row, dict)
                and isinstance(row.get("presentationEventId"), str)
            },
        )
    for ordinal, row in enumerate(rows):
        context = f"Valtan Combat Object Sound cues[{ordinal}]"
        if not isinstance(row, dict):
            raise CompositionError(f"{context} must be an object")
        _require_exact_fields(
            row,
            (
                "bindingId",
                "combatObjectArchetypeId",
                "soundBank",
                "soundEvent",
            ),
            ("hitId", "presentationEventId"),
            context,
        )
        binding_id = _require_string(row["bindingId"], f"{context}.bindingId")
        if binding_id in binding_ids:
            raise CompositionError(
                f"duplicate Combat Object Sound bindingId: {binding_id}"
            )
        binding_ids.add(binding_id)
        for field in ("combatObjectArchetypeId", "soundBank", "soundEvent"):
            _require_string(row[field], f"{binding_id}.{field}")
        targets = target_index.get(row["combatObjectArchetypeId"])
        if targets is None:
            raise CompositionError(
                f"{binding_id}.combatObjectArchetypeId does not resolve"
            )
        target_fields = [
            field
            for field in ("hitId", "presentationEventId")
            if field in row
        ]
        if len(target_fields) != 1:
            raise CompositionError(
                f"{binding_id} requires exactly one hitId/presentationEventId"
            )
        _require_string(row[target_fields[0]], f"{binding_id}.{target_fields[0]}")
        target_set = targets[0] if target_fields[0] == "hitId" else targets[1]
        if row[target_fields[0]] not in target_set:
            raise CompositionError(
                f"{binding_id}.{target_fields[0]} does not resolve in its combat object"
            )
        if row["soundEvent"] not in sound_events:
            raise CompositionError(
                f"{binding_id}.soundEvent does not resolve in the Valtan Sound catalog: "
                f"{row['soundEvent']}"
            )
        if row["soundBank"] != "S_Mob_G_Voltan2" or not row[
            "soundEvent"
        ].startswith("G_Voltan2_"):
            raise CompositionError(
                f"{binding_id} Combat Object Sound bank/event contract is invalid"
            )


def _validate_v1_effect_owners(
    root: Path,
    document: Mapping[str, Any],
    *,
    animation_bindings: Mapping[str, Any] | None = None,
    joined: Mapping[str, Any] | None = None,
) -> list[str]:
    _require_exact_fields(
        document,
        ("schema", "formatVersion", "ownerArchetypeId", "cues"),
        (),
        "Valtan Effect V1 source",
    )
    if (
        document.get("schema") != "lostark.valtan-pattern-effect-cues"
        or isinstance(document.get("formatVersion"), bool)
        or document.get("formatVersion") != 4
        or document.get("ownerArchetypeId") != "BOSS_VALTAN"
    ):
        raise CompositionError("Valtan Effect V1 source header/version is invalid")
    rows = document.get("cues")
    if not isinstance(rows, list) or not rows or len(rows) > 512:
        raise CompositionError("Valtan Effect V1 cues must contain 1..512 rows")
    if animation_bindings is None:
        animation_bindings = read_json(root / valtan.BINDINGS_REL)
    if joined is None:
        joined = _valtan_join(root, VALTAN_SOURCE_DOCUMENTS)

    action_bindings: dict[str, Mapping[str, Any]] = {}
    for ordinal, binding in enumerate(animation_bindings.get("bindings", [])):
        if not isinstance(binding, dict):
            raise CompositionError(
                f"Valtan Animation bindings[{ordinal}] must be an object"
            )
        action_id = _require_owner_stable_id(
            binding.get("actionId"),
            f"Valtan Animation bindings[{ordinal}].actionId",
            160,
        )
        if action_id in action_bindings:
            raise CompositionError(f"duplicate Valtan Animation actionId: {action_id}")
        if not isinstance(binding.get("clips"), list):
            raise CompositionError(f"Valtan Animation {action_id}.clips must be an array")
        action_bindings[action_id] = binding

    stage_index: dict[tuple[str, str], tuple[Mapping[str, Any], Mapping[str, Any]]] = {}
    joined_pattern_ids: set[str] = set()
    for pattern in joined.get("patterns", []):
        if not isinstance(pattern, dict) or not isinstance(pattern.get("stages"), list):
            raise CompositionError("Valtan joined Pattern graph is malformed")
        pattern_id = _require_owner_stable_id(
            pattern.get("patternId"), "Valtan joined patternId", 160
        )
        joined_pattern_ids.add(pattern_id)
        for stage in pattern["stages"]:
            if not isinstance(stage, dict):
                raise CompositionError(f"Valtan {pattern_id} Stage is malformed")
            stage_id = _require_owner_stable_id(
                stage.get("stageId"), f"Valtan {pattern_id}.stageId", 160
            )
            key = (pattern_id, stage_id)
            if key in stage_index:
                raise CompositionError(f"duplicate Valtan Pattern/Stage identity: {key}")
            stage_index[key] = (pattern, stage)

    catalog = read_json(root / valtan.EFFECT_CATALOG_REL)
    referenced_paths: set[str] = set()
    binding_ids: set[str] = set()
    occurrence_ids: set[str] = set()
    action_occurrence_tuples: set[tuple[str, str, str]] = set()
    managed_scale_patterns = {
        "VALTAN_WHIRLWIND",
        "VALTAN_DASH_CHARGE",
        "VALTAN_FOUR_SLASH",
        "VALTAN_FIST_IN_OUT",
        "VALTAN_HIGH_JUMP",
        "VALTAN_FLOOR_WIPE_130",
        "VALTAN_ARENA_BREAK_109",
    }
    for ordinal, row in enumerate(rows):
        context = f"Valtan Effect V1 cues[{ordinal}]"
        if not isinstance(row, dict):
            raise CompositionError(f"{context} must be an object")
        stage_clock = row.get("timingBasis") == "STAGE_CLOCK"
        required = {
            "bindingId",
            "occurrenceId",
            "patternId",
            "stageId",
            "actionId",
            "effectAssetId",
            "anchorSlotId",
            "followPolicy",
            "stopPolicy",
            "repeatPolicy",
            "localTransform",
        }
        required.update(
            ("timingBasis", "stageOffsetMs")
            if stage_clock
            else ("clipOccurrenceId", "sourceStartMs", "sourceEndMs")
        )
        _require_exact_fields(row, required, ("scalePolicy",), context)
        binding_id = _require_owner_stable_id(
            row.get("bindingId"), f"{context}.bindingId", 160
        )
        if binding_id in binding_ids:
            raise CompositionError(f"duplicate Effect V1 bindingId: {binding_id}")
        binding_ids.add(binding_id)
        occurrence_id = _require_owner_stable_id(
            row.get("occurrenceId"), f"{binding_id}.occurrenceId", 160
        )
        if occurrence_id in occurrence_ids:
            raise CompositionError(f"duplicate Effect V1 occurrenceId: {occurrence_id}")
        occurrence_ids.add(occurrence_id)
        pattern_id = _require_owner_stable_id(
            row.get("patternId"), f"{binding_id}.patternId", 160
        )
        stage_id = _require_owner_stable_id(
            row.get("stageId"), f"{binding_id}.stageId", 160
        )
        action_id = _require_owner_stable_id(
            row.get("actionId"), f"{binding_id}.actionId", 160
        )
        effect_asset_id = _require_owner_stable_id(
            row.get("effectAssetId"), f"{binding_id}.effectAssetId", 160
        )
        anchor_slot_id = _require_owner_stable_id(
            row.get("anchorSlotId"), f"{binding_id}.anchorSlotId", 160
        )
        follow_policy = row.get("followPolicy")
        stop_policy = row.get("stopPolicy")
        repeat_policy = row.get("repeatPolicy")
        if follow_policy not in ("follow", "snapshot"):
            raise CompositionError(f"{binding_id}.followPolicy is invalid")
        if stop_policy not in ("natural", "cue_end"):
            raise CompositionError(f"{binding_id}.stopPolicy is invalid")
        if repeat_policy not in ("once", "each_loop"):
            raise CompositionError(f"{binding_id}.repeatPolicy is invalid")

        local_transform = row.get("localTransform")
        if not isinstance(local_transform, dict):
            raise CompositionError(f"{binding_id}.localTransform must be an object")
        _require_exact_fields(
            local_transform,
            ("position", "rotationDegrees", "scale"),
            (),
            f"{binding_id}.localTransform",
        )
        _require_float3(
            local_transform["position"],
            f"{binding_id}.localTransform.position",
            maximum_magnitude=100000.0,
        )
        _require_float3(
            local_transform["rotationDegrees"],
            f"{binding_id}.localTransform.rotationDegrees",
            maximum_magnitude=360000.0,
        )
        _require_float3(
            local_transform["scale"],
            f"{binding_id}.localTransform.scale",
            maximum_magnitude=1000.0,
            positive=True,
        )

        scale_policy = row.get("scalePolicy")
        if scale_policy is None:
            if pattern_id in managed_scale_patterns:
                raise CompositionError(
                    f"{binding_id} managed Pattern requires explicit scalePolicy"
                )
        else:
            if not isinstance(scale_policy, dict):
                raise CompositionError(f"{binding_id}.scalePolicy must be an object")
            kind = scale_policy.get("kind")
            if kind == "OWNER_RELATIVE":
                _require_exact_fields(
                    scale_policy, ("kind",), (), f"{binding_id}.scalePolicy"
                )
            elif kind in ("GAMEPLAY_FOOTPRINT", "ARENA_ABSOLUTE"):
                _require_exact_fields(
                    scale_policy,
                    ("kind", "worldScale"),
                    (),
                    f"{binding_id}.scalePolicy",
                )
                _require_float3(
                    scale_policy["worldScale"],
                    f"{binding_id}.scalePolicy.worldScale",
                    maximum_magnitude=1000.0,
                    positive=True,
                )
            else:
                raise CompositionError(f"{binding_id}.scalePolicy.kind is invalid")

        owner = stage_index.get((pattern_id, stage_id))
        if owner is None and pattern_id in joined_pattern_ids:
            raise CompositionError(
                f"{binding_id} does not resolve its exact Pattern/Stage/action tuple"
            )
        if owner is not None and owner[1].get("actionId") != action_id:
            raise CompositionError(
                f"{binding_id} does not resolve its exact Pattern/Stage/action tuple"
            )
        pattern, stage = owner if owner is not None else ({}, {})
        stage_duration = (
            _require_positive_int(
                stage.get("durationMs"), f"{binding_id}.Stage.durationMs"
            )
            if owner is not None
            else MAX_CUE_TIME_MS
        )
        animation = action_bindings.get(action_id)
        if animation is None:
            raise CompositionError(f"{binding_id}.actionId has no Animation binding")

        if anchor_slot_id.startswith("pattern.target."):
            if (
                anchor_slot_id != "pattern.target.snapshot"
                or follow_policy != "snapshot"
                or pattern.get("targetPolicy")
                not in (
                    "LOCK_NEAREST_ON_START",
                    "LOCK_RANDOM_ALIVE_ON_START",
                    "LOCK_RANDOM_ALIVE_BEHIND_ON_START",
                )
            ):
                raise CompositionError(
                    f"{binding_id} target anchor does not match its Server target policy"
                )
        if anchor_slot_id.startswith("arena.center"):
            motion = pattern.get("serverMotion")
            fixed_center = anchor_slot_id == "arena.center" and follow_policy == "snapshot"
            fixed_facing = (
                anchor_slot_id == "arena.center.facing"
                and follow_policy == "snapshot"
                and pattern.get("targetPolicy") == "LOCK_RANDOM_ALIVE_ON_START"
                and pattern.get("aimPolicy") == "LOCK_FACING_ON_START"
            )
            target_follow = (
                anchor_slot_id == "arena.center.target-follow"
                and follow_policy == "follow"
                and pattern.get("targetPolicy") == "LOCK_RANDOM_ALIVE_ON_START"
                and pattern.get("aimPolicy") == "TRACK_TARGET_EACH_TICK"
            )
            if (
                not isinstance(motion, dict)
                or motion.get("kind") != "LEAP_TO_ANCHOR"
                or motion.get("moveToAnchorBeforeTakeoff") is not True
                or not (fixed_center or fixed_facing or target_follow)
            ):
                raise CompositionError(
                    f"{binding_id} arena-center anchor contract is invalid"
                )

        if stage_clock:
            start_ms = _require_nonnegative_int(
                row.get("stageOffsetMs"), f"{binding_id}.stageOffsetMs"
            )
            if start_ms >= stage_duration:
                raise CompositionError(f"{binding_id}.stageOffsetMs is outside its Stage")
            if stop_policy != "natural" or repeat_policy != "once":
                raise CompositionError(
                    f"{binding_id} STAGE_CLOCK requires natural/once policies"
                )
            tuple_key = (action_id, "STAGE_CLOCK", occurrence_id)
        else:
            clip_id = _require_owner_stable_id(
                row.get("clipOccurrenceId"), f"{binding_id}.clipOccurrenceId", 160
            )
            clips = [
                clip
                for clip in animation["clips"]
                if isinstance(clip, dict) and clip.get("clipOccurrenceId") == clip_id
            ]
            if len(clips) != 1 or animation.get("suppressAnimation") is True:
                raise CompositionError(
                    f"{binding_id}.clipOccurrenceId does not resolve its Animation"
                )
            clip = clips[0]
            start_ms = _require_nonnegative_int(
                row.get("sourceStartMs"), f"{binding_id}.sourceStartMs"
            )
            clip_start = _require_nonnegative_int(
                clip.get("sourceStartMs"), f"{binding_id}.clip.sourceStartMs"
            )
            clip_play = _require_nonnegative_int(
                clip.get("playMs"), f"{binding_id}.clip.playMs"
            )
            segment_end = clip_start + clip_play
            if start_ms < clip_start or (clip_play != 0 and start_ms >= segment_end):
                raise CompositionError(f"{binding_id}.sourceStartMs is outside its clip")
            if repeat_policy == "each_loop" and clip.get("loop") is not True:
                raise CompositionError(
                    f"{binding_id}.repeatPolicy targets a non-loop Animation"
                )
            source_end = row.get("sourceEndMs")
            if stop_policy == "natural":
                if source_end is not None:
                    raise CompositionError(
                        f"{binding_id}.sourceEndMs must be null for natural stopPolicy"
                    )
            else:
                end_ms = _require_nonnegative_int(
                    source_end, f"{binding_id}.sourceEndMs"
                )
                if end_ms <= start_ms or (clip_play != 0 and end_ms > segment_end):
                    raise CompositionError(
                        f"{binding_id}.sourceEndMs is outside its clip window"
                    )
            tuple_key = (action_id, clip_id, occurrence_id)
        if tuple_key in action_occurrence_tuples:
            raise CompositionError(
                f"duplicate Effect V1 action/clock/occurrence tuple: {tuple_key}"
            )
        action_occurrence_tuples.add(tuple_key)

        try:
            catalog_row = valtan.validate_canonical_authored_effect_asset(
                root, catalog, effect_asset_id, binding_id
            )
        except valtan.PipelineError as exc:
            raise CompositionError(f"Valtan Effect V1 source is invalid: {exc}") from exc
        referenced_paths.add("Data/" + catalog_row["authoringPath"])
    return sorted(referenced_paths)


def _validate_v1_alias_owners(
    root: Path,
    document: Mapping[str, Any],
    cue_document: Mapping[str, Any] | None = None,
) -> list[str]:
    _require_exact_fields(
        document,
        ("schema", "formatVersion", "ownerArchetypeId", "aliases"),
        (),
        "Valtan Effect V1 alias source",
    )
    if (
        document["schema"] != "lostark.valtan-pattern-effect-v1-aliases"
        or isinstance(document["formatVersion"], bool)
        or document["formatVersion"] != 1
        or document["ownerArchetypeId"] != "BOSS_VALTAN"
        or not isinstance(document["aliases"], list)
        or len(document["aliases"]) > 512
    ):
        raise CompositionError("Valtan Effect V1 alias source contract is invalid")
    catalog = read_json(root / valtan.EFFECT_CATALOG_REL)
    if cue_document is None:
        cue_document = read_json(root / valtan.CUES_REL)
    cue_source_ids = {
        row.get("effectAssetId")
        for row in cue_document.get("cues", [])
        if isinstance(row, dict) and isinstance(row.get("effectAssetId"), str)
    }
    source_ids: set[str] = set()
    target_ids: set[str] = set()
    referenced_paths: set[str] = set()
    for ordinal, row in enumerate(document["aliases"]):
        context = f"Valtan Effect V1 aliases[{ordinal}]"
        if not isinstance(row, dict):
            raise CompositionError(f"{context} must be an object")
        _require_exact_fields(row, ("effectAssetId", "v1EffectAssetId"), (), context)
        source_id = _require_string(row["effectAssetId"], f"{context}.effectAssetId")
        target_id = _require_string(row["v1EffectAssetId"], f"{context}.v1EffectAssetId")
        if (
            source_id == target_id
            or not target_id.endswith(".v1.unified")
            or source_id not in cue_source_ids
        ):
            raise CompositionError(
                f"{context} must map one cue source to a distinct .v1.unified target"
            )
        if source_id in source_ids or target_id in target_ids:
            raise CompositionError(f"duplicate Effect V1 alias identity: {source_id}")
        source_ids.add(source_id)
        target_ids.add(target_id)
        for effect_id in (source_id, target_id):
            try:
                catalog_row = valtan.validate_canonical_authored_effect_asset(
                    root, catalog, effect_id, context
                )
            except valtan.PipelineError as exc:
                raise CompositionError(
                    f"Valtan Effect V1 alias source is invalid: {exc}"
                ) from exc
            referenced_paths.add("Data/" + catalog_row["authoringPath"])
    return sorted(referenced_paths)


def _validate_pattern_shake_document(
    document: Mapping[str, Any], animation_bindings: Mapping[str, Any]
) -> None:
    _require_exact_fields(
        document,
        ("schema", "formatVersion", "ownerArchetypeId", "cues"),
        (),
        "Valtan Pattern Shake source",
    )
    if (
        document["schema"] != "lostark.valtan-pattern-shake-cues"
        or isinstance(document["formatVersion"], bool)
        or document["formatVersion"] != 1
        or document["ownerArchetypeId"] != "BOSS_VALTAN"
        or not isinstance(document["cues"], list)
        or len(document["cues"]) > 1024
    ):
        raise CompositionError("Valtan Pattern Shake source contract is invalid")
    occurrence_index: dict[tuple[str, str], Mapping[str, Any]] = {}
    for binding in animation_bindings.get("bindings", []):
        if not isinstance(binding, dict) or not isinstance(binding.get("clips"), list):
            raise CompositionError("Valtan Animation binding is invalid")
        action_id = _require_string(binding.get("actionId"), "Animation actionId")
        for clip in binding["clips"]:
            if not isinstance(clip, dict):
                raise CompositionError("Valtan Animation clip occurrence is invalid")
            clip_id = _require_string(
                clip.get("clipOccurrenceId"), "Animation clipOccurrenceId"
            )
            key = (action_id, clip_id)
            if key in occurrence_index:
                raise CompositionError(f"duplicate Animation occurrence identity: {key}")
            occurrence_index[key] = clip
    binding_ids: set[str] = set()
    occurrence_ids: set[str] = set()
    for ordinal, row in enumerate(document["cues"]):
        context = f"Valtan Pattern Shake cues[{ordinal}]"
        if not isinstance(row, dict):
            raise CompositionError(f"{context} must be an object")
        _require_exact_fields(
            row,
            (
                "bindingId",
                "occurrenceId",
                "patternId",
                "stageId",
                "actionId",
                "clipOccurrenceId",
                "repeatPolicy",
                "startMs",
                "shake",
            ),
            (),
            context,
        )
        binding_id = _require_string(row["bindingId"], f"{context}.bindingId")
        occurrence_id = _require_string(row["occurrenceId"], f"{binding_id}.occurrenceId")
        if binding_id in binding_ids or occurrence_id in occurrence_ids:
            raise CompositionError(f"duplicate Pattern Shake identity: {binding_id}")
        binding_ids.add(binding_id)
        occurrence_ids.add(occurrence_id)
        for field in ("patternId", "stageId", "actionId", "clipOccurrenceId", "shake"):
            _require_string(row[field], f"{binding_id}.{field}")
        if row["repeatPolicy"] not in ("once", "each_loop"):
            raise CompositionError(f"{binding_id}.repeatPolicy is invalid")
        clip = occurrence_index.get((row["actionId"], row["clipOccurrenceId"]))
        if clip is None:
            # Historical, unmanaged rows remain visible as detached inventory.
            # A row whose Pattern/Stage scope is managed is resolved again by
            # the normalized graph projector and still fails closed there.
            continue
        start_ms = _require_nonnegative_int(row["startMs"], f"{binding_id}.startMs")
        source_start = clip.get("sourceStartMs")
        play_ms = clip.get("playMs")
        if (
            isinstance(source_start, bool)
            or not isinstance(source_start, int)
            or isinstance(play_ms, bool)
            or not isinstance(play_ms, int)
            or start_ms < source_start
            or (play_ms != 0 and start_ms >= source_start + play_ms)
        ):
            raise CompositionError(
                f"{binding_id}.startMs is outside its Animation occurrence"
            )


def _validate_valtan_camera_cue(
    row: Mapping[str, Any],
    context: str,
    *,
    maximum_duration_ms: int,
    scene_ids: set[str],
    allow_tracking: bool,
) -> str:
    base_fields = {
        "cueId",
        "durationMs",
        "interpolation",
        "easing",
        "shakeAmplitude",
        "shakeDurationMs",
        "keyframes",
    }
    if allow_tracking:
        base_fields.update(("patternId", "stageId"))
    optional = (
        ("trackingMode", "trackingOrigin", "transitionInMs", "transitionOutMs")
        if allow_tracking
        else ()
    )
    _require_exact_fields(row, base_fields, optional, context)
    cue_id = _require_owner_stable_id(row["cueId"], f"{context}.cueId", 128)
    duration_ms = _require_positive_int(row["durationMs"], f"{cue_id}.durationMs")
    if duration_ms > maximum_duration_ms:
        raise CompositionError(
            f"{cue_id}.durationMs exceeds its owner limit of {maximum_duration_ms}"
        )
    if row["interpolation"] not in ("LINEAR", "CATMULL_ROM"):
        raise CompositionError(f"{cue_id}.interpolation is invalid")
    if row["easing"] not in ("LINEAR", "SMOOTHSTEP", "HOLD"):
        raise CompositionError(f"{cue_id}.easing is invalid")
    shake_amplitude = _require_finite_number(
        row["shakeAmplitude"],
        f"{cue_id}.shakeAmplitude",
        minimum=0.0,
        maximum=2.0,
    )
    shake_duration = _require_nonnegative_int(
        row["shakeDurationMs"], f"{cue_id}.shakeDurationMs"
    )
    if shake_duration > 1000 or shake_duration > duration_ms:
        raise CompositionError(f"{cue_id}.shakeDurationMs is invalid")
    if (shake_amplitude > 0.0) != (shake_duration != 0):
        raise CompositionError(f"{cue_id} shake amplitude/duration pair is invalid")

    if allow_tracking:
        has_tracking_mode = "trackingMode" in row
        has_tracking_origin = "trackingOrigin" in row
        if has_tracking_mode != has_tracking_origin:
            raise CompositionError(
                f"{cue_id} must author trackingMode and trackingOrigin together"
            )
        if has_tracking_mode:
            if row["trackingMode"] not in (
                "BOSS_XZ",
                "BOSS_FACING",
                "PLAYER_BOSS_FRAME",
            ):
                raise CompositionError(f"{cue_id}.trackingMode is invalid")
            _require_float3(
                row["trackingOrigin"],
                f"{cue_id}.trackingOrigin",
                maximum_magnitude=100000.0,
            )
        for field in ("transitionInMs", "transitionOutMs"):
            if field not in row:
                continue
            transition_ms = _require_nonnegative_int(row[field], f"{cue_id}.{field}")
            if transition_ms > 1000:
                raise CompositionError(f"{cue_id}.{field} exceeds 1000")
            if field == "transitionInMs" and transition_ms > duration_ms:
                raise CompositionError(f"{cue_id}.transitionInMs exceeds durationMs")

    keyframes = row["keyframes"]
    if not isinstance(keyframes, list) or not 2 <= len(keyframes) <= 64:
        raise CompositionError(f"{cue_id}.keyframes must contain 2..64 rows")
    previous_time: int | None = None
    for ordinal, keyframe in enumerate(keyframes):
        frame_context = f"{cue_id}.keyframes[{ordinal}]"
        if not isinstance(keyframe, dict):
            raise CompositionError(f"{frame_context} must be an object")
        _require_exact_fields(
            keyframe,
            ("sceneId", "timeMs", "eye", "lookAt", "fovYDegrees"),
            (),
            frame_context,
        )
        scene_id = _require_owner_stable_id(
            keyframe["sceneId"], f"{frame_context}.sceneId", 128
        )
        if scene_id in scene_ids:
            raise CompositionError(f"duplicate Valtan camera sceneId: {scene_id}")
        scene_ids.add(scene_id)
        time_ms = _require_nonnegative_int(
            keyframe["timeMs"], f"{frame_context}.timeMs"
        )
        if time_ms > duration_ms:
            raise CompositionError(f"{frame_context}.timeMs exceeds durationMs")
        if previous_time is None and time_ms != 0:
            raise CompositionError(f"{cue_id} first keyframe must start at 0 ms")
        if previous_time is not None and time_ms <= previous_time:
            raise CompositionError(f"{cue_id} keyframe times must strictly increase")
        eye = _require_float3(
            keyframe["eye"], f"{frame_context}.eye", maximum_magnitude=100000.0
        )
        look_at = _require_float3(
            keyframe["lookAt"],
            f"{frame_context}.lookAt",
            maximum_magnitude=100000.0,
        )
        if sum((eye[index] - look_at[index]) ** 2 for index in range(3)) <= 0.000001:
            raise CompositionError(f"{frame_context} eye and lookAt must differ")
        _require_finite_number(
            keyframe["fovYDegrees"],
            f"{frame_context}.fovYDegrees",
            minimum=10.0,
            maximum=120.0,
        )
        previous_time = time_ms
    if previous_time != duration_ms:
        raise CompositionError(f"{cue_id} final keyframe must match durationMs")
    return cue_id


def _load_valtan_camera_index(
    root: Path, joined: Mapping[str, Any] | None = None
) -> dict[str, tuple[str, str]]:
    document = read_json(root / valtan.CAMERA_REL)
    _require_exact_fields(
        document,
        ("schema", "formatVersion", "encounterId", "provenance", "cues", "deathCue"),
        (),
        "Valtan Cinematic Camera source",
    )
    if (
        document["schema"] != "lostark.encounter-cinematic-camera"
        or isinstance(document["formatVersion"], bool)
        or document["formatVersion"] != 6
        or document["encounterId"] != "ENCOUNTER_VALTAN"
        or document["provenance"] != "PROJECT_AUTHORED"
        or not isinstance(document["cues"], list)
        or not 1 <= len(document["cues"]) <= 32
        or not isinstance(document["deathCue"], dict)
    ):
        raise CompositionError("Valtan Cinematic Camera source contract is invalid")
    joined_stages: dict[tuple[str, str], Mapping[str, Any]] = {}
    if joined is not None:
        for pattern in joined.get("patterns", []):
            if not isinstance(pattern, dict) or not isinstance(pattern.get("stages"), list):
                raise CompositionError("Valtan joined camera Pattern graph is malformed")
            pattern_id = _require_owner_stable_id(
                pattern.get("patternId"), "Valtan camera joined patternId", 128
            )
            for stage in pattern["stages"]:
                if not isinstance(stage, dict):
                    raise CompositionError("Valtan camera joined Stage is malformed")
                stage_id = _require_owner_stable_id(
                    stage.get("stageId"), "Valtan camera joined stageId", 128
                )
                joined_stages[(pattern_id, stage_id)] = stage
    result: dict[str, tuple[str, str]] = {}
    stage_tuples: set[tuple[str, str]] = set()
    scene_ids: set[str] = set()
    for ordinal, row in enumerate(document["cues"]):
        context = f"Valtan Cinematic Camera cues[{ordinal}]"
        if not isinstance(row, dict):
            raise CompositionError(f"{context} must be an object")
        pattern_id = _require_owner_stable_id(
            row.get("patternId"), f"{context}.patternId", 128
        )
        stage_id = _require_owner_stable_id(
            row.get("stageId"), f"{context}.stageId", 128
        )
        stage_tuple = (pattern_id, stage_id)
        if stage_tuple in stage_tuples:
            raise CompositionError(
                f"duplicate Valtan camera Pattern/Stage tuple: {stage_tuple}"
            )
        stage_tuples.add(stage_tuple)
        maximum_duration = 60_000
        if joined is not None:
            stage = joined_stages.get(stage_tuple)
            if stage is None:
                raise CompositionError(
                    f"Valtan camera cue names unknown Pattern/Stage: {stage_tuple}"
                )
            maximum_duration = _require_positive_int(
                stage.get("durationMs"), f"{context}.Stage.durationMs"
            )
        cue_id = _validate_valtan_camera_cue(
            row,
            context,
            maximum_duration_ms=maximum_duration,
            scene_ids=scene_ids,
            allow_tracking=True,
        )
        if cue_id in result:
            raise CompositionError(f"duplicate Valtan camera cueId: {cue_id}")
        result[cue_id] = stage_tuple
    death_cue = document["deathCue"]
    if death_cue:
        death_cue_id = _validate_valtan_camera_cue(
            death_cue,
            "Valtan Cinematic Camera deathCue",
            maximum_duration_ms=60_000,
            scene_ids=scene_ids,
            allow_tracking=False,
        )
        if death_cue_id in result:
            raise CompositionError(f"duplicate Valtan camera cueId: {death_cue_id}")
    return result


def _valtan_transitive_source_paths(
    root: Path, sources: Mapping[str, str]
) -> list[str]:
    paths = {
        valtan.LEGACY_REL,
        valtan.EFFECT_CATALOG_REL,
        valtan.CAMERA_REL,
        valtan.ENCOUNTER_REL,
        SOUND_CATALOG_REL,
    }
    effect_v1 = read_json(root / sources["EFFECT_V1_CUES"])
    paths.update(_validate_v1_effect_owners(root, effect_v1))
    aliases = read_json(root / sources["EFFECT_V1_ALIASES"])
    paths.update(_validate_v1_alias_owners(root, aliases))
    for directory, suffix in (
        (root / "Data/Effects/V2/Authored", "*.effectv2.json"),
        (root / "Data/Effects/V2/Groups", "*.effectv2group.json"),
    ):
        for path in sorted(directory.glob(suffix)):
            try:
                paths.add(path.resolve().relative_to(root.resolve()).as_posix())
            except ValueError as exc:
                raise CompositionError(
                    f"Effect V2 source path escapes repository root: {path}"
                ) from exc
    return sorted(paths)


def _validate_projected_cue_invariants(
    patterns: list[dict[str, Any]], detached: list[dict[str, Any]]
) -> None:
    allowed_kinds = {
        "ANIMATION",
        "EFFECT_V1",
        "EFFECT_V2",
        "SOUND",
        "CAMERA",
        "CAMERA_SHAKE",
        "HIT",
        "COMBAT_OBJECT",
        "COMBAT_OBJECT_SOUND",
        "LOGIC",
    }
    allowed_bases = {"STAGE", "CLIP_OCCURRENCE", "COMBAT_OBJECT_EVENT"}
    allowed_repeats = {"ONCE", "EACH_LOOP"}
    allowed_stops = {
        "NATURAL",
        "STAGE_END",
        "PATTERN_END",
        "EXPLICIT",
        "CLIP_OCCURRENCE_END",
        "CUE_END",
    }
    cue_ids: set[str] = set()

    def validate(cue: Mapping[str, Any], stage_duration_ms: int | None) -> None:
        cue_id = _require_string(cue.get("cueId"), "normalized cue.cueId")
        if cue_id in cue_ids:
            raise CompositionError(f"duplicate normalized cueId: {cue_id}")
        cue_ids.add(cue_id)
        if cue.get("kind") not in allowed_kinds:
            raise CompositionError(f"{cue_id}.kind is invalid")
        for field in ("scope", "clock", "anchor", "payload"):
            if not isinstance(cue.get(field), dict):
                raise CompositionError(f"{cue_id}.{field} must be an object")
        clock = cue["clock"]
        basis = clock.get("basis")
        if basis not in allowed_bases:
            raise CompositionError(f"{cue_id}.clock.basis is invalid")
        if clock.get("repeatPolicy") not in allowed_repeats:
            raise CompositionError(f"{cue_id}.clock.repeatPolicy is invalid")
        start_ms = clock.get("startMs")
        if basis != "COMBAT_OBJECT_EVENT":
            start_ms = _require_nonnegative_int(start_ms, f"{cue_id}.clock.startMs")
            if start_ms > MAX_CUE_TIME_MS:
                raise CompositionError(
                    f"{cue_id}.clock.startMs exceeds {MAX_CUE_TIME_MS}"
                )
            if stage_duration_ms is not None and start_ms > stage_duration_ms:
                raise CompositionError(f"{cue_id} starts after its scoped Stage")
        if cue.get("stopPolicy") not in allowed_stops:
            raise CompositionError(f"{cue_id}.stopPolicy is invalid")
        if "endMs" in cue:
            end_ms = _require_nonnegative_int(cue["endMs"], f"{cue_id}.endMs")
            if stage_duration_ms is not None and end_ms > stage_duration_ms:
                raise CompositionError(f"{cue_id} ends after its scoped Stage")
            if isinstance(start_ms, int) and end_ms < start_ms:
                raise CompositionError(f"{cue_id}.endMs precedes startMs")

    for pattern in patterns:
        for stage in pattern["stages"]:
            for cue in stage["cues"]:
                validate(cue, stage["durationMs"])
    for cue in detached:
        validate(cue, None)


def project_valtan_shadow_graph(
    joined: Mapping[str, Any],
    effect_v1: Mapping[str, Any],
    pattern_sound: Mapping[str, Any],
    effect_v2: Mapping[str, Any],
    pattern_shake: Mapping[str, Any] | None,
    combat_object_sound: Mapping[str, Any] | None,
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    sound_rows = pattern_sound.get("cues", [])
    effect_rows = effect_v2.get("bindings", [])
    shake_rows = [] if pattern_shake is None else pattern_shake.get("cues", [])
    external_by_scope: dict[
        tuple[Any, Any, Any], list[tuple[str, Mapping[str, Any]]]
    ] = {}
    for rows, kind in (
        (effect_rows, "EFFECT_V2"),
        (sound_rows, "SOUND"),
        (shake_rows, "CAMERA_SHAKE"),
    ):
        if not isinstance(rows, list):
            raise CompositionError(f"Valtan {kind} source rows must be an array")
        for row in rows:
            if not isinstance(row, dict):
                raise CompositionError(f"Valtan {kind} source row must be an object")
            external_by_scope.setdefault(_scope_key(row), []).append((kind, row))
    patterns: list[dict[str, Any]] = []
    managed_keys: set[tuple[Any, Any, Any]] = set()
    attached_v1_ids: set[str] = set()
    for pattern in joined["patterns"]:
        projected_pattern = {
            key: copy.deepcopy(value)
            for key, value in pattern.items()
            if key != "stages"
        }
        projected_stages = []
        for stage in pattern["stages"]:
            key = (pattern["patternId"], stage["stageId"], stage["actionId"])
            managed_keys.add(key)
            cues = []
            cues.extend(_normalized_animation_cues(pattern["patternId"], stage))
            v1_cues = _normalized_v1_cues(pattern["patternId"], stage)
            cues.extend(v1_cues)
            for cue in v1_cues:
                cue_id = cue["cueId"]
                if cue_id in attached_v1_ids:
                    raise CompositionError(f"duplicate managed Effect V1 cueId: {cue_id}")
                attached_v1_ids.add(cue_id)
            cues.extend(
                _normalized_external_cue(row, kind, stage=stage)
                for kind, row in external_by_scope.get(key, [])
            )
            cues.extend(_normalized_camera_cues(pattern["patternId"], stage))
            cues.extend(_normalized_gameplay_cues(pattern["patternId"], stage))
            cues.sort(
                key=lambda row: (
                    row["clock"]["startMs"], row["kind"], row["cueId"]
                )
            )
            projected_stage = {
                "stageId": stage["stageId"],
                "actionId": stage["actionId"],
                "sequenceRole": stage["sequenceRole"],
                "stageKind": stage["stageKind"],
                "durationMs": stage["durationMs"],
                "flow": {
                    "defaultNextActionId": copy.deepcopy(
                        stage["defaultNextActionId"]
                    ),
                    "branches": copy.deepcopy(stage["branches"]),
                },
                "motion": copy.deepcopy(stage["motion"]),
                "hit": copy.deepcopy(stage["hit"]),
                "events": copy.deepcopy(stage["events"]),
                "cues": cues,
            }
            for optional in (
                "bodyVisibility",
                "partDamagePolicy",
                "counterProxy",
                "bossResponse",
                "verticalOffsetM",
            ):
                if optional in stage:
                    projected_stage[optional] = copy.deepcopy(stage[optional])
            projected_stages.append(projected_stage)
        projected_pattern["stages"] = projected_stages
        patterns.append(projected_pattern)
    detached = []
    for key, cues in external_by_scope.items():
        if key not in managed_keys:
            for kind, row in cues:
                detached.append(_normalized_external_cue(row, kind, detached=True))
    effect_v1_rows = effect_v1.get("cues", [])
    if not isinstance(effect_v1_rows, list):
        raise CompositionError("Valtan Effect V1 Product cues must be an array")
    product_v1_ids: set[str] = set()
    for row in effect_v1_rows:
        if not isinstance(row, dict):
            raise CompositionError("Valtan Effect V1 Product cue must be an object")
        cue_id = _require_string(row.get("bindingId"), "Effect V1 bindingId")
        if cue_id in product_v1_ids:
            raise CompositionError(f"duplicate Effect V1 Product bindingId: {cue_id}")
        product_v1_ids.add(cue_id)
        if cue_id not in attached_v1_ids:
            detached.append(_normalized_detached_v1_cue(row))
    if not attached_v1_ids.issubset(product_v1_ids):
        missing = sorted(attached_v1_ids - product_v1_ids)
        raise CompositionError(
            f"managed Effect V1 cues are missing from Product projection: {missing}"
        )
    if combat_object_sound is not None:
        rows = combat_object_sound.get("cues", [])
        if not isinstance(rows, list):
            raise CompositionError("Valtan combat-object Sound cues must be an array")
        for row in rows:
            detached.append(
                {
                    "cueId": row["bindingId"],
                    "kind": "COMBAT_OBJECT_SOUND",
                    "scope": {
                        "combatObjectArchetypeId": row["combatObjectArchetypeId"]
                    },
                    "clock": {"basis": "COMBAT_OBJECT_EVENT", "repeatPolicy": "ONCE"},
                    "stopPolicy": "NATURAL",
                    "anchor": {"kind": "COMBAT_OBJECT"},
                    "payload": {
                        key: copy.deepcopy(value)
                        for key, value in row.items()
                        if key not in {"bindingId", "combatObjectArchetypeId"}
                    },
                    "detachedReason": "COMBAT_OBJECT_LIFETIME_SCOPE",
                }
            )
    detached.sort(key=lambda row: (row["kind"], row["cueId"]))
    return patterns, detached


def _project_valtan_document_graph(
    root: Path, sources: Mapping[str, str], joined: Mapping[str, Any]
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    effect_v1 = read_json(root / sources["EFFECT_V1_CUES"])
    pattern_sound = read_json(root / sources["PATTERN_SOUND_CUES"])
    effect_v2 = read_json(root / sources["EFFECT_V2_BINDINGS"])
    gameplay = read_json(root / sources["GAMEPLAY"])
    animation_bindings = read_json(root / sources["ANIMATION_BINDINGS"])
    sound_events = _load_valtan_sound_events(root)
    _validate_v1_effect_owners(
        root,
        effect_v1,
        animation_bindings=animation_bindings,
        joined=joined,
    )
    _validate_v1_alias_owners(
        root,
        read_json(root / sources["EFFECT_V1_ALIASES"]),
        effect_v1,
    )
    _validate_pattern_sound_document(
        pattern_sound, animation_bindings, sound_events
    )
    try:
        effect_v2 = effect_v2_pipeline.validate_binding_document(
            root,
            effect_v2,
            gameplay,
            animation_bindings,
            read_json(root / valtan.LEGACY_REL),
        )
    except effect_v2_pipeline.BindingContractError as exc:
        raise CompositionError(f"Valtan Effect V2 source is invalid: {exc}") from exc
    pattern_shake = (
        read_json(root / sources["PATTERN_SHAKE_CUES"])
        if "PATTERN_SHAKE_CUES" in sources
        else None
    )
    if pattern_shake is not None:
        _validate_pattern_shake_document(pattern_shake, animation_bindings)
    combat_sound = (
        read_json(root / sources["COMBAT_OBJECT_SOUND_CUES"])
        if "COMBAT_OBJECT_SOUND_CUES" in sources
        else None
    )
    if combat_sound is not None:
        _validate_combat_object_sound_document(
            combat_sound,
            sound_events,
            read_json(root / sources["COMBAT_OBJECTS"]),
        )
    camera_index = _load_valtan_camera_index(
        root, read_json(root / valtan.ENCOUNTER_REL)
    )
    for pattern in joined["patterns"]:
        for stage in pattern["stages"]:
            for invocation in stage["cameraInvocations"]:
                cue_id = invocation["cameraCueId"]
                if camera_index.get(cue_id) != (
                    pattern["patternId"],
                    stage["stageId"],
                ):
                    raise CompositionError(
                        "Valtan camera invocation does not resolve its exact Pattern/Stage: "
                        f"{cue_id}"
                    )
    result = project_valtan_shadow_graph(
        joined, effect_v1, pattern_sound, effect_v2, pattern_shake, combat_sound
    )
    _validate_projected_cue_invariants(*result)
    return result


def _validate_valtan_coverage(
    root: Path, coverage: Mapping[str, Any], sources: Mapping[str, str]
) -> dict[str, Any]:
    _require_exact_fields(
        coverage,
        (
            "kind",
            "expectedPatternCount",
            "expectedStageCount",
            "expectedIdentitySha256",
        ),
        (),
        "boss.coverage",
    )
    if coverage["kind"] != "VALTAN_SPLIT_JOIN":
        raise CompositionError("Valtan boss.coverage.kind must be VALTAN_SPLIT_JOIN")
    joined = _valtan_join(root, sources)
    actual = _valtan_identity(joined)
    expected = (
        _require_positive_int(
            coverage["expectedPatternCount"], "boss.coverage.expectedPatternCount"
        ),
        _require_positive_int(
            coverage["expectedStageCount"], "boss.coverage.expectedStageCount"
        ),
        _require_string(
            coverage["expectedIdentitySha256"],
            "boss.coverage.expectedIdentitySha256",
        ),
    )
    if actual != expected:
        raise CompositionError(
            "Valtan composition coverage drift: "
            f"expected={expected} actual={actual}"
        )
    return joined


def _kakul_identity(
    root: Path, profiles: Any, source_paths: set[str]
) -> tuple[list[dict[str, Any]], tuple[int, int, str]]:
    if (
        not isinstance(profiles, list)
        or not profiles
        or len(profiles) > MAX_REFERENCE_PROFILES
    ):
        raise CompositionError(
            f"boss.coverage.profiles must contain 1..{MAX_REFERENCE_PROFILES} rows"
        )
    resolved_profiles: list[dict[str, Any]] = []
    identity: list[dict[str, Any]] = []
    seen_profiles: set[str] = set()
    for index, row in enumerate(profiles):
        context = f"boss.coverage.profiles[{index}]"
        if not isinstance(row, dict):
            raise CompositionError(f"{context} must be an object")
        _require_exact_fields(
            row,
            (
                "profileId",
                "actionReferencePath",
                "actionBindingPath",
                "patternBindingPath",
                "expectedActionCount",
                "expectedReferenceRevision",
            ),
            (),
            context,
        )
        profile_id = _require_stable_id(row["profileId"], f"{context}.profileId")
        if profile_id in seen_profiles:
            raise CompositionError(f"duplicate Kakul profileId: {profile_id}")
        seen_profiles.add(profile_id)
        resolved_paths: dict[str, str] = {}
        for field in ("actionReferencePath", "actionBindingPath", "patternBindingPath"):
            path, _ = _safe_source_path(root, row[field], f"{context}.{field}")
            if path not in source_paths:
                raise CompositionError(f"{context}.{field} is not in sourceDocuments")
            resolved_paths[field] = path
        reference = read_json(root / resolved_paths["actionReferencePath"])
        action_binding = read_json(root / resolved_paths["actionBindingPath"])
        pattern_binding = read_json(root / resolved_paths["patternBindingPath"])
        try:
            kakul_animation.validate_reference_document(reference)
            kakul_animation.validate_authored_document(action_binding, reference)
            kakul_animation.validate_pattern_document(pattern_binding, reference)
        except kakul_animation.BuildError as exc:
            raise CompositionError(
                f"{context} Kakul reference-only document is invalid: {exc}"
            ) from exc
        if reference.get("profileId") != profile_id:
            raise CompositionError(f"{context} action reference profileId mismatch")
        if action_binding.get("profileId") != profile_id:
            raise CompositionError(f"{context} action binding profileId mismatch")
        if pattern_binding.get("profileId") != profile_id:
            raise CompositionError(f"{context} pattern binding profileId mismatch")
        if reference.get("referenceRevision") != row["expectedReferenceRevision"]:
            raise CompositionError(f"{context} reference revision drift")
        actions = reference.get("actions")
        if not isinstance(actions, list):
            raise CompositionError(f"{context} action reference has no actions array")
        expected_count = _require_positive_int(
            row["expectedActionCount"], f"{context}.expectedActionCount"
        )
        if len(actions) != expected_count:
            raise CompositionError(
                f"{context} action count drift: expected={expected_count} actual={len(actions)}"
            )
        action_ids = [action.get("sourceActionId") for action in actions]
        if any(isinstance(value, bool) or not isinstance(value, int) for value in action_ids):
            raise CompositionError(f"{context} contains an invalid sourceActionId")
        if len(action_ids) != len(set(action_ids)):
            raise CompositionError(f"{context} contains duplicate sourceActionId values")
        identity.append(
            {
                "profileId": profile_id,
                "referenceRevision": reference["referenceRevision"],
                "sourceActionIds": action_ids,
            }
        )
        resolved_profiles.append(
            {
                "profileId": profile_id,
                "actionReference": reference,
                "actionBindings": action_binding,
                "patternBindings": pattern_binding,
            }
        )
    identity.sort(key=lambda row: row["profileId"])
    result = (
        len(identity),
        sum(len(row["sourceActionIds"]) for row in identity),
        _sha256_bytes(_canonical_bytes(identity)),
    )
    resolved_profiles.sort(key=lambda row: row["profileId"])
    return resolved_profiles, result


def _validate_kakul_coverage(
    root: Path, coverage: Mapping[str, Any], source_paths: set[str]
) -> list[dict[str, Any]]:
    _require_exact_fields(
        coverage,
        (
            "kind",
            "expectedProfileCount",
            "expectedActionCount",
            "expectedIdentitySha256",
            "profiles",
        ),
        (),
        "boss.coverage",
    )
    if coverage["kind"] != "KAKUL_ACTION_REFERENCE":
        raise CompositionError(
            "KakulSaydon boss.coverage.kind must be KAKUL_ACTION_REFERENCE"
        )
    resolved, actual = _kakul_identity(root, coverage["profiles"], source_paths)
    expected = (
        _require_positive_int(
            coverage["expectedProfileCount"], "boss.coverage.expectedProfileCount"
        ),
        _require_positive_int(
            coverage["expectedActionCount"], "boss.coverage.expectedActionCount"
        ),
        _require_string(
            coverage["expectedIdentitySha256"],
            "boss.coverage.expectedIdentitySha256",
        ),
    )
    if actual != expected:
        raise CompositionError(
            "KakulSaydon composition coverage drift: "
            f"expected={expected} actual={actual}"
        )
    return resolved


def validate_boss_document(
    root: Path, document: Mapping[str, Any], context: str
) -> dict[str, Any]:
    _require_exact_fields(
        document,
        (
            "schema",
            "formatVersion",
            "compositionId",
            "status",
            "revision",
            "displayName",
            "bossArchetypeId",
            "encounterId",
            "areaId",
            "sourceDocuments",
            "coverage",
            "patterns",
        ),
        (),
        context,
    )
    status = _validate_common_root(document, context, BOSS_SCHEMA)
    composition_id = _require_stable_id(
        document["compositionId"], f"{context}.compositionId"
    )
    if not COMPOSITION_ID_RE.fullmatch(composition_id):
        raise CompositionError(f"{context}.compositionId is invalid: {composition_id}")
    for field in ("bossArchetypeId", "encounterId"):
        value = document[field]
        if value is not None:
            _require_stable_id(value, f"{context}.{field}")
        if status == "AUTHORITATIVE" and value is None:
            raise CompositionError(f"{context}.{field} is required for AUTHORITATIVE")
    sources, source_paths = _validate_source_documents(
        root, document["sourceDocuments"], f"{context}.sourceDocuments"
    )
    coverage = document["coverage"]
    if not isinstance(coverage, dict):
        raise CompositionError(f"{context}.coverage must be an object")
    if composition_id == "boss.composition.valtan":
        if status != "SHADOW":
            raise CompositionError("Valtan composition remains SHADOW during split-owner migration")
        if document["bossArchetypeId"] != "BOSS_VALTAN":
            raise CompositionError("Valtan bossArchetypeId must be BOSS_VALTAN")
        if document["encounterId"] != "ENCOUNTER_VALTAN":
            raise CompositionError("Valtan encounterId must be ENCOUNTER_VALTAN")
        if document["areaId"] != "LV_LUT_HEARTRB_ED":
            raise CompositionError("Valtan areaId must be LV_LUT_HEARTRB_ED")
        if sources != VALTAN_SOURCE_DOCUMENTS:
            raise CompositionError(
                "Valtan source role/path closure is invalid: "
                f"expected={VALTAN_SOURCE_DOCUMENTS} actual={sources}"
            )
        joined = _validate_valtan_coverage(root, coverage, sources)
        pattern_rows = document["patterns"]
        if not isinstance(pattern_rows, list) or len(pattern_rows) > MAX_PATTERNS:
            raise CompositionError(
                f"{context}.patterns must be an array <= {MAX_PATTERNS} rows"
            )
        pattern_ids: list[str] = []
        for index, row in enumerate(pattern_rows):
            row_context = f"{context}.patterns[{index}]"
            if not isinstance(row, dict):
                raise CompositionError(f"{row_context} must be an object")
            _require_exact_fields(row, ("patternId",), (), row_context)
            pattern_ids.append(
                _require_stable_id(row["patternId"], f"{row_context}.patternId")
            )
        joined_ids = [row["patternId"] for row in joined["patterns"]]
        if pattern_ids != joined_ids:
            raise CompositionError("Valtan composition pattern index order/closure drift")
        normalized_patterns, detached_cues = _project_valtan_document_graph(
            root, sources, joined
        )
        resolved = {
            "joinedPatternMaster": joined,
            "unifiedPatternGraph": normalized_patterns,
            "detachedCues": detached_cues,
        }
        for role, key in (
            ("PATTERN_SOUND_CUES", "patternSoundCues"),
            ("EFFECT_V1_CUES", "effectV1Cues"),
            ("EFFECT_V2_BINDINGS", "effectV2Bindings"),
            ("PATTERN_SHAKE_CUES", "patternShakeCues"),
            ("COMBAT_OBJECT_SOUND_CUES", "combatObjectSoundCues"),
        ):
            if role in sources:
                resolved[key] = read_json(root / sources[role])
        source_paths = sorted(
            set(source_paths) | set(_valtan_transitive_source_paths(root, sources))
        )
        return {"sources": sources, "sourcePaths": source_paths, "resolved": resolved}
    if composition_id == "boss.composition.kakulsaydon":
        if status != "REFERENCE_ONLY":
            raise CompositionError(
                "KakulSaydon composition must remain REFERENCE_ONLY until gameplay Product exists"
            )
        if document["bossArchetypeId"] is not None or document["encounterId"] is not None:
            raise CompositionError(
                "KakulSaydon must not invent boss/encounter IDs while REFERENCE_ONLY"
            )
        if document["areaId"] != "LV_LUT_MIDNIGHTC_ED":
            raise CompositionError("KakulSaydon areaId must be LV_LUT_MIDNIGHTC_ED")
        if document["patterns"] != []:
            raise CompositionError(
                "KakulSaydon patterns must remain empty while REFERENCE_ONLY"
            )
        profiles = _validate_kakul_coverage(root, coverage, set(source_paths))
        expected_sources: dict[str, str] = {}
        for profile in coverage["profiles"]:
            profile_id = profile["profileId"]
            expected_sources[f"ACTION_REFERENCE_{profile_id}"] = profile[
                "actionReferencePath"
            ]
            expected_sources[f"ACTION_BINDING_{profile_id}"] = profile[
                "actionBindingPath"
            ]
            expected_sources[f"PATTERN_BINDING_{profile_id}"] = profile[
                "patternBindingPath"
            ]
        if sources != expected_sources:
            raise CompositionError(
                "KakulSaydon source role/path closure is invalid: "
                f"expected={expected_sources} actual={sources}"
            )
        return {
            "sources": sources,
            "sourcePaths": source_paths,
            "resolved": {"referenceProfiles": profiles},
        }
    raise CompositionError(f"unsupported compositionId: {composition_id}")


def _require_bounded_display_text(value: Any, context: str, maximum_bytes: int) -> str:
    result = _require_string(value, context)
    if len(result.encode("utf-8")) > maximum_bytes or any(
        ord(character) < 0x20 or ord(character) == 0x7F for character in result
    ):
        raise CompositionError(
            f"{context} must be bounded UTF-8 display text <= {maximum_bytes} bytes"
        )
    return result


def _validate_world_sequence_source(
    document: Mapping[str, Any], expected_area_id: str
) -> set[str]:
    """Validate the complete format-v2 Map owner without publishing the Map domain."""
    context = "World Sequence source"
    _require_exact_fields(
        document,
        ("schema", "formatVersion", "areaId", "revision", "templates", "instances"),
        (),
        context,
    )
    if (
        document["schema"] != "lostark.world-sequences"
        or isinstance(document["formatVersion"], bool)
        or document["formatVersion"] != 2
        or document["areaId"] != expected_area_id
    ):
        raise CompositionError(f"{context} header/area is invalid")
    _require_positive_int(document["revision"], f"{context}.revision")
    templates = document["templates"]
    instances = document["instances"]
    if (
        not isinstance(templates, list)
        or len(templates) > WORLD_SEQUENCE_MAX_TEMPLATES
        or not isinstance(instances, list)
        or len(instances) > WORLD_SEQUENCE_MAX_INSTANCES
    ):
        raise CompositionError(f"{context} exceeds its array limits")

    template_slots: dict[str, dict[str, str]] = {}
    for template_ordinal, template in enumerate(templates):
        template_context = f"{context}.templates[{template_ordinal}]"
        if not isinstance(template, dict):
            raise CompositionError(f"{template_context} must be an object")
        _require_exact_fields(
            template,
            (
                "sequenceId",
                "displayName",
                "category",
                "durationMs",
                "interpolation",
                "tracks",
                "animationTracks",
            ),
            (),
            template_context,
        )
        sequence_id = _require_owner_stable_id(
            template["sequenceId"], f"{template_context}.sequenceId", 128
        )
        if sequence_id in template_slots:
            raise CompositionError(
                f"duplicate World Sequence templateId: {sequence_id}"
            )
        _require_bounded_display_text(
            template["displayName"], f"{template_context}.displayName", 128
        )
        _require_bounded_display_text(
            template["category"], f"{template_context}.category", 64
        )
        duration_ms = _require_positive_int(
            template["durationMs"], f"{template_context}.durationMs"
        )
        if duration_ms > WORLD_SEQUENCE_MAX_DURATION_MS:
            raise CompositionError(
                f"{template_context}.durationMs exceeds "
                f"{WORLD_SEQUENCE_MAX_DURATION_MS}"
            )
        if template["interpolation"] not in ("LINEAR", "SMOOTH_STEP"):
            raise CompositionError(f"{template_context}.interpolation is invalid")
        transform_tracks = template["tracks"]
        animation_tracks = template["animationTracks"]
        if not isinstance(transform_tracks, list) or not isinstance(
            animation_tracks, list
        ):
            raise CompositionError(
                f"{template_context} track collections must be arrays"
            )
        total_tracks = len(transform_tracks) + len(animation_tracks)
        if total_tracks < 1 or total_tracks > WORLD_SEQUENCE_MAX_TRACKS:
            raise CompositionError(
                f"{template_context} track count must be between 1 and "
                f"{WORLD_SEQUENCE_MAX_TRACKS}"
            )

        # A slot may carry both a transform track and a clip chain so one
        # binding can walk an animated prop while it plays. Duplicates are a
        # conflict only within the same kind.
        slots: dict[str, str] = {}
        transform_slots: set[str] = set()
        for track_ordinal, track in enumerate(transform_tracks):
            track_context = f"{template_context}.tracks[{track_ordinal}]"
            if not isinstance(track, dict):
                raise CompositionError(f"{track_context} must be an object")
            _require_exact_fields(track, ("slotId", "keys"), (), track_context)
            slot_id = _require_owner_stable_id(
                track["slotId"], f"{track_context}.slotId", 128
            )
            if slot_id in transform_slots:
                raise CompositionError(
                    f"duplicate World Sequence slotId: {sequence_id}/{slot_id}"
                )
            transform_slots.add(slot_id)
            slots[slot_id] = "MAP_PLACEMENT"
            keys = track["keys"]
            if (
                not isinstance(keys, list)
                or len(keys) < 2
                or len(keys) > WORLD_SEQUENCE_MAX_KEYS
            ):
                raise CompositionError(
                    f"{track_context}.keys must contain 2..{WORLD_SEQUENCE_MAX_KEYS} rows"
                )
            previous_time = -1
            for key_ordinal, key in enumerate(keys):
                key_context = f"{track_context}.keys[{key_ordinal}]"
                if not isinstance(key, dict):
                    raise CompositionError(f"{key_context} must be an object")
                _require_exact_fields(
                    key,
                    (
                        "timeMs",
                        "positionOffset",
                        "rotationQuaternion",
                        "scaleMultiplier",
                        "visible",
                    ),
                    (),
                    key_context,
                )
                time_ms = _require_nonnegative_int(
                    key["timeMs"], f"{key_context}.timeMs"
                )
                if time_ms <= previous_time or time_ms > duration_ms:
                    raise CompositionError(
                        f"{key_context}.timeMs must increase within durationMs"
                    )
                previous_time = time_ms
                _require_float3(
                    key["positionOffset"],
                    f"{key_context}.positionOffset",
                    maximum_magnitude=WORLD_SEQUENCE_MAX_COMPONENT,
                )
                quaternion = key["rotationQuaternion"]
                if not isinstance(quaternion, list) or len(quaternion) != 4:
                    raise CompositionError(
                        f"{key_context}.rotationQuaternion must be a float4"
                    )
                quaternion_values = [
                    _require_finite_number(
                        component,
                        f"{key_context}.rotationQuaternion[{axis}]",
                        minimum=-WORLD_SEQUENCE_MAX_COMPONENT,
                        maximum=WORLD_SEQUENCE_MAX_COMPONENT,
                    )
                    for axis, component in enumerate(quaternion)
                ]
                quaternion_length = math.sqrt(
                    sum(component * component for component in quaternion_values)
                )
                if (
                    abs(quaternion_length - 1.0) > 0.001
                    or quaternion_values[3] < 0.0
                ):
                    raise CompositionError(
                        f"{key_context}.rotationQuaternion must be normalized "
                        "with non-negative w"
                    )
                scale = _require_float3(
                    key["scaleMultiplier"],
                    f"{key_context}.scaleMultiplier",
                    maximum_magnitude=WORLD_SEQUENCE_MAX_COMPONENT,
                    positive=True,
                )
                if any(value < WORLD_SEQUENCE_MIN_SCALE for value in scale):
                    raise CompositionError(
                        f"{key_context}.scaleMultiplier is below runtime epsilon"
                    )
                if not isinstance(key["visible"], bool):
                    raise CompositionError(f"{key_context}.visible must be boolean")
            if keys[0]["timeMs"] != 0 or keys[-1]["timeMs"] != duration_ms:
                raise CompositionError(
                    f"{track_context} must span the whole template duration"
                )

        # An animation slot may carry an ordered clip chain, so its rows are
        # checked against that slot's previous start. A slot still may not be
        # both a transform and an animation slot.
        animation_slot_starts: dict[str, int] = {}
        for track_ordinal, track in enumerate(animation_tracks):
            track_context = f"{template_context}.animationTracks[{track_ordinal}]"
            if not isinstance(track, dict):
                raise CompositionError(f"{track_context} must be an object")
            _require_exact_fields(
                track,
                ("slotId", "clipName", "playbackRate", "loop", "holdLastFrame"),
                ("startMs",),
                track_context,
            )
            slot_id = _require_owner_stable_id(
                track["slotId"], f"{track_context}.slotId", 128
            )
            start_ms = 0
            if "startMs" in track:
                start_ms = _require_nonnegative_int(
                    track["startMs"], f"{track_context}.startMs"
                )
            if start_ms >= duration_ms:
                raise CompositionError(
                    f"{track_context}.startMs must be inside the template duration"
                )
            if slot_id in animation_slot_starts:
                if start_ms <= animation_slot_starts[slot_id]:
                    raise CompositionError(
                        f"World Sequence animation chain must advance: "
                        f"{sequence_id}/{slot_id}"
                    )
            else:
                if start_ms != 0:
                    raise CompositionError(
                        f"World Sequence animation chain must start at 0ms: "
                        f"{sequence_id}/{slot_id}"
                    )
                slots[slot_id] = "DEPLOY_PLACEMENT"
            animation_slot_starts[slot_id] = start_ms
            _require_bounded_display_text(
                track["clipName"], f"{track_context}.clipName", 128
            )
            _require_finite_number(
                track["playbackRate"],
                f"{track_context}.playbackRate",
                minimum=0.05,
                maximum=8.0,
            )
            if not isinstance(track["loop"], bool) or not isinstance(
                track["holdLastFrame"], bool
            ):
                raise CompositionError(
                    f"{track_context}.loop/holdLastFrame must be boolean"
                )
        template_slots[sequence_id] = slots

    instance_ids: set[str] = set()
    for instance_ordinal, instance in enumerate(instances):
        instance_context = f"{context}.instances[{instance_ordinal}]"
        if not isinstance(instance, dict):
            raise CompositionError(f"{instance_context} must be an object")
        _require_exact_fields(
            instance,
            (
                "instanceId",
                "templateId",
                "enabled",
                "startDelayMs",
                "playbackSpeed",
                "bindings",
            ),
            (),
            instance_context,
        )
        instance_id = _require_owner_stable_id(
            instance["instanceId"], f"{instance_context}.instanceId", 128
        )
        if instance_id in instance_ids:
            raise CompositionError(
                f"duplicate World Sequence instanceId: {instance_id}"
            )
        instance_ids.add(instance_id)
        template_id = _require_owner_stable_id(
            instance["templateId"], f"{instance_context}.templateId", 128
        )
        slots = template_slots.get(template_id)
        if slots is None:
            raise CompositionError(
                f"{instance_context}.templateId does not resolve: {template_id}"
            )
        if not isinstance(instance["enabled"], bool):
            raise CompositionError(f"{instance_context}.enabled must be boolean")
        start_delay_ms = _require_nonnegative_int(
            instance["startDelayMs"], f"{instance_context}.startDelayMs"
        )
        if start_delay_ms > WORLD_SEQUENCE_MAX_DURATION_MS:
            raise CompositionError(
                f"{instance_context}.startDelayMs exceeds "
                f"{WORLD_SEQUENCE_MAX_DURATION_MS}"
            )
        _require_finite_number(
            instance["playbackSpeed"],
            f"{instance_context}.playbackSpeed",
            minimum=0.05,
            maximum=8.0,
        )
        bindings = instance["bindings"]
        if not isinstance(bindings, list) or len(bindings) != len(slots):
            raise CompositionError(
                f"{instance_context}.bindings count does not match its template"
            )
        bound_slots: set[str] = set()
        bound_targets: set[tuple[str, str]] = set()
        for binding_ordinal, binding in enumerate(bindings):
            binding_context = f"{instance_context}.bindings[{binding_ordinal}]"
            if not isinstance(binding, dict):
                raise CompositionError(f"{binding_context} must be an object")
            _require_exact_fields(
                binding, ("slotId", "targetKind", "targetId"), (), binding_context
            )
            slot_id = _require_owner_stable_id(
                binding["slotId"], f"{binding_context}.slotId", 128
            )
            if slot_id in bound_slots or slot_id not in slots:
                raise CompositionError(
                    f"{binding_context}.slotId is duplicate or unresolved: {slot_id}"
                )
            bound_slots.add(slot_id)
            target_kind = binding["targetKind"]
            if target_kind not in ("MAP_PLACEMENT", "DEPLOY_PLACEMENT"):
                raise CompositionError(f"{binding_context}.targetKind is invalid")
            if target_kind != slots[slot_id]:
                raise CompositionError(
                    f"{binding_context}.targetKind does not match slot kind"
                )
            target_id = _require_string(
                binding["targetId"], f"{binding_context}.targetId"
            )
            if (
                len(target_id) > 20
                or not target_id.isascii()
                or not target_id.isdecimal()
            ):
                raise CompositionError(
                    f"{binding_context}.targetId must be an unsigned integer string"
                )
            numeric_target_id = int(target_id)
            if numeric_target_id == 0 or numeric_target_id > (1 << 64) - 1:
                raise CompositionError(f"{binding_context}.targetId is out of range")
            target_key = (target_kind, target_id)
            if target_key in bound_targets:
                raise CompositionError(
                    f"{binding_context} duplicates a placement target"
                )
            bound_targets.add(target_key)
    return instance_ids


def _validate_camera_shot_source(
    document: Mapping[str, Any],
    expected_area_id: str,
    world_instance_ids: set[str],
) -> set[str]:
    """Validate the complete Kakul camera owner contract and linked sequence IDs."""
    context = "Camera Shot source"
    _require_exact_fields(
        document,
        ("schema", "formatVersion", "areaId", "revision", "shots"),
        (),
        context,
    )
    if (
        document["schema"] != "lostark.camera-shots"
        or isinstance(document["formatVersion"], bool)
        or document["formatVersion"] != 1
        or document["areaId"] != expected_area_id
    ):
        raise CompositionError(f"{context} header/area is invalid")
    _require_positive_int(document["revision"], f"{context}.revision")
    shots = document["shots"]
    if not isinstance(shots, list) or len(shots) > CAMERA_SHOT_MAX_COUNT:
        raise CompositionError(
            f"{context}.shots must be an array <= {CAMERA_SHOT_MAX_COUNT} rows"
        )
    shot_ids: set[str] = set()
    for shot_ordinal, shot in enumerate(shots):
        shot_context = f"{context}.shots[{shot_ordinal}]"
        if not isinstance(shot, dict):
            raise CompositionError(f"{shot_context} must be an object")
        _require_exact_fields(
            shot,
            (
                "shotId",
                "sequenceInstanceId",
                "box",
                "eye",
                "lookAt",
                "fovYDegrees",
                "blendInMs",
                "blendOutMs",
                "priority",
            ),
            ("cameraTrack", "follow"),
            shot_context,
        )
        shot_id = _require_owner_stable_id(
            shot["shotId"], f"{shot_context}.shotId", 128
        )
        if shot_id in shot_ids:
            raise CompositionError(f"duplicate Camera Shot shotId: {shot_id}")
        shot_ids.add(shot_id)
        instance_id = _require_string(
            shot["sequenceInstanceId"],
            f"{shot_context}.sequenceInstanceId",
            allow_empty=True,
        )
        if instance_id:
            _require_owner_stable_id(
                instance_id, f"{shot_context}.sequenceInstanceId", 128
            )
            if instance_id not in world_instance_ids:
                raise CompositionError(
                    f"{shot_context}.sequenceInstanceId does not resolve: "
                    f"{instance_id}"
                )
        box = shot["box"]
        if not isinstance(box, dict):
            raise CompositionError(f"{shot_context}.box must be an object")
        _require_exact_fields(
            box, ("center", "halfExtents", "yawDegrees"), (), f"{shot_context}.box"
        )
        _require_float3(
            box["center"],
            f"{shot_context}.box.center",
            maximum_magnitude=CAMERA_SHOT_MAX_COORDINATE,
        )
        _require_float3(
            box["halfExtents"],
            f"{shot_context}.box.halfExtents",
            maximum_magnitude=CAMERA_SHOT_MAX_HALF_EXTENT,
            positive=True,
        )
        _require_finite_number(
            box["yawDegrees"],
            f"{shot_context}.box.yawDegrees",
            minimum=-360.0,
            maximum=360.0,
        )
        eye = _require_float3(
            shot["eye"],
            f"{shot_context}.eye",
            maximum_magnitude=CAMERA_SHOT_MAX_COORDINATE,
        )
        look_at = _require_float3(
            shot["lookAt"],
            f"{shot_context}.lookAt",
            maximum_magnitude=CAMERA_SHOT_MAX_COORDINATE,
        )
        if sum((look_at[axis] - eye[axis]) ** 2 for axis in range(3)) <= 0.000001:
            raise CompositionError(f"{shot_context}.eye and lookAt must differ")
        fov = _require_finite_number(
            shot["fovYDegrees"], f"{shot_context}.fovYDegrees"
        )
        if fov <= 1.0 or fov >= 179.0:
            raise CompositionError(
                f"{shot_context}.fovYDegrees must be between 1 and 179"
            )
        for field, maximum in (
            ("blendInMs", CAMERA_SHOT_MAX_BLEND_MS),
            ("blendOutMs", CAMERA_SHOT_MAX_BLEND_MS),
            ("priority", CAMERA_SHOT_MAX_PRIORITY),
        ):
            value = _require_nonnegative_int(shot[field], f"{shot_context}.{field}")
            if value > maximum:
                raise CompositionError(
                    f"{shot_context}.{field} exceeds {maximum}"
                )
        if "cameraTrack" in shot:
            _validate_camera_track(shot["cameraTrack"], f"{shot_context}.cameraTrack")
        if "follow" in shot:
            _validate_camera_follow(shot["follow"], f"{shot_context}.follow")
    return shot_ids


def _validate_camera_track(track: Any, context: str) -> None:
    """Validate the optional keyframed track exactly as the product level reads it.

    A shot without a track keeps its single authored pose; with one the level
    samples it on the bound sequence's clock, so a track this owner admits but
    the level rejects would strand the cutscene on a stale frame.
    """
    if not isinstance(track, dict):
        raise CompositionError(f"{context} must be an object")
    _require_exact_fields(
        track, ("durationMs", "interpolation", "easing", "keyframes"), (), context
    )
    duration_ms = _require_positive_int(track["durationMs"], f"{context}.durationMs")
    if duration_ms > CAMERA_TRACK_MAX_DURATION_MS:
        raise CompositionError(
            f"{context}.durationMs exceeds {CAMERA_TRACK_MAX_DURATION_MS}"
        )
    if track["interpolation"] not in ("LINEAR", "CATMULL_ROM"):
        raise CompositionError(f"{context}.interpolation is unknown")
    if track["easing"] not in ("LINEAR", "SMOOTHSTEP", "HOLD"):
        raise CompositionError(f"{context}.easing is unknown")
    keyframes = track["keyframes"]
    if (
        not isinstance(keyframes, list)
        or len(keyframes) < 2
        or len(keyframes) > CAMERA_TRACK_MAX_KEYFRAMES
    ):
        raise CompositionError(
            f"{context}.keyframes must be 2 to {CAMERA_TRACK_MAX_KEYFRAMES} rows"
        )
    scene_ids: set[str] = set()
    previous_time_ms = -1
    for ordinal, keyframe in enumerate(keyframes):
        key_context = f"{context}.keyframes[{ordinal}]"
        if not isinstance(keyframe, dict):
            raise CompositionError(f"{key_context} must be an object")
        _require_exact_fields(
            keyframe,
            ("sceneId", "timeMs", "eye", "lookAt", "fovYDegrees"),
            (),
            key_context,
        )
        scene_id = _require_owner_stable_id(
            keyframe["sceneId"], f"{key_context}.sceneId", 128
        )
        if scene_id in scene_ids:
            raise CompositionError(f"duplicate camera keyframe sceneId: {scene_id}")
        scene_ids.add(scene_id)
        time_ms = _require_nonnegative_int(keyframe["timeMs"], f"{key_context}.timeMs")
        if time_ms > duration_ms:
            raise CompositionError(f"{key_context}.timeMs exceeds the track duration")
        if ordinal == 0:
            if time_ms != 0:
                raise CompositionError(f"{context} must start at 0ms")
        elif time_ms <= previous_time_ms:
            raise CompositionError(f"{context}.keyframes must advance in time")
        previous_time_ms = time_ms
        eye = _require_float3(
            keyframe["eye"],
            f"{key_context}.eye",
            maximum_magnitude=CAMERA_SHOT_MAX_COORDINATE,
        )
        look_at = _require_float3(
            keyframe["lookAt"],
            f"{key_context}.lookAt",
            maximum_magnitude=CAMERA_SHOT_MAX_COORDINATE,
        )
        if sum((look_at[axis] - eye[axis]) ** 2 for axis in range(3)) <= 0.000001:
            raise CompositionError(f"{key_context} has no view direction")
        fov = _require_finite_number(
            keyframe["fovYDegrees"], f"{key_context}.fovYDegrees"
        )
        if fov <= 1.0 or fov >= 179.0:
            raise CompositionError(
                f"{key_context}.fovYDegrees must be between 1 and 179"
            )
    if previous_time_ms != duration_ms:
        raise CompositionError(f"{context} must end at its duration")


def _validate_camera_follow(follow: Any, context: str) -> None:
    """A side scrolling stage slides one framing with the player instead of
    pinning it, so that shot carries two offsets and no fixed world pose."""
    if not isinstance(follow, dict):
        raise CompositionError(f"{context} must be an object")
    _require_exact_fields(follow, ("eyeOffset", "lookAtOffset"), (), context)
    eye_offset = _require_float3(
        follow["eyeOffset"],
        f"{context}.eyeOffset",
        maximum_magnitude=CAMERA_SHOT_MAX_COORDINATE,
    )
    look_at_offset = _require_float3(
        follow["lookAtOffset"],
        f"{context}.lookAtOffset",
        maximum_magnitude=CAMERA_SHOT_MAX_COORDINATE,
    )
    if (
        sum((look_at_offset[axis] - eye_offset[axis]) ** 2 for axis in range(3))
        <= 0.000001
    ):
        raise CompositionError(f"{context} offsets coincide")


def _load_ref_ids(
    root: Path, sources: Mapping[str, str], expected_area_id: str
) -> dict[str, set[str]]:
    result: dict[str, set[str]] = {}
    world_instance_ids: set[str] = set()
    if "WORLD_SEQUENCES" in sources:
        world_instance_ids = _validate_world_sequence_source(
            read_json(root / sources["WORLD_SEQUENCES"]), expected_area_id
        )
        result["WORLD_SEQUENCE"] = world_instance_ids
    if "CAMERA_SHOTS" in sources:
        result["CAMERA_SHOT"] = _validate_camera_shot_source(
            read_json(root / sources["CAMERA_SHOTS"]),
            expected_area_id,
            world_instance_ids,
        )
    return result


def validate_sequencer_document(
    root: Path,
    document: Mapping[str, Any],
    context: str,
    boss_documents: Mapping[str, Mapping[str, Any]],
) -> dict[str, Any]:
    _require_exact_fields(
        document,
        (
            "schema",
            "formatVersion",
            "sequencerId",
            "status",
            "revision",
            "displayName",
            "areaId",
            "bossCompositionId",
            "durationMs",
            "sourceDocuments",
            "tracks",
        ),
        (),
        context,
    )
    status = _validate_common_root(document, context, SEQUENCER_SCHEMA)
    if status != "SHADOW":
        raise CompositionError(
            f"{context}.status must remain SHADOW before Server-trigger adapters are closed"
        )
    sequencer_id = _require_stable_id(
        document["sequencerId"], f"{context}.sequencerId"
    )
    if not SEQUENCER_ID_RE.fullmatch(sequencer_id):
        raise CompositionError(f"{context}.sequencerId is invalid: {sequencer_id}")
    boss_id = _require_stable_id(
        document["bossCompositionId"], f"{context}.bossCompositionId"
    )
    arena_contract = ARENA_CONTRACT_BY_BOSS_ID.get(boss_id)
    if arena_contract is None:
        raise CompositionError(f"{context} has no fixed Arena contract for {boss_id}")
    if (
        sequencer_id != arena_contract["sequencerId"]
        or document["areaId"] != arena_contract["areaId"]
    ):
        raise CompositionError(
            f"{context} fixed sequencerId/areaId/bossCompositionId contract is invalid"
        )
    boss = boss_documents.get(boss_id)
    if boss is None:
        raise CompositionError(f"{context} references unknown boss composition: {boss_id}")
    if boss.get("areaId") != document["areaId"]:
        raise CompositionError(f"{context} boss composition areaId mismatch")
    duration_ms = _require_nonnegative_int(document["durationMs"], f"{context}.durationMs")
    if duration_ms > MAX_ARENA_DURATION_MS:
        raise CompositionError(
            f"{context}.durationMs exceeds {MAX_ARENA_DURATION_MS}"
        )
    sources, source_paths = _validate_source_documents(
        root, document["sourceDocuments"], f"{context}.sourceDocuments"
    )
    if sources != arena_contract["sources"]:
        raise CompositionError(
            f"{context} source role/path closure is invalid: "
            f"expected={arena_contract['sources']} actual={sources}"
        )
    linked_boss = read_json(root / sources["BOSS_COMPOSITION"])
    if linked_boss.get("compositionId") != boss_id:
        raise CompositionError(
            f"{context}.BOSS_COMPOSITION does not contain {boss_id}"
        )
    ref_ids = _load_ref_ids(root, sources, document["areaId"])
    tracks = document["tracks"]
    if not isinstance(tracks, list) or len(tracks) > MAX_TRACKS:
        raise CompositionError(
            f"{context}.tracks must be an array <= {MAX_TRACKS} rows"
        )
    if tracks and duration_ms == 0:
        raise CompositionError(
            f"{context}.durationMs may be zero only when tracks is empty"
        )
    seen: set[str] = set()
    intervals: dict[str, list[tuple[int, int, str]]] = {
        "SCENE_PROFILE": [],
        "CAMERA_SHOT": [],
    }
    for index, track in enumerate(tracks):
        track_context = f"{context}.tracks[{index}]"
        if not isinstance(track, dict):
            raise CompositionError(f"{track_context} must be an object")
        _require_exact_fields(
            track,
            ("trackId", "kind", "startMs", "payload"),
            ("endMs",),
            track_context,
        )
        track_id = _require_stable_id(track["trackId"], f"{track_context}.trackId")
        if not TRACK_ID_RE.fullmatch(track_id):
            raise CompositionError(f"{track_context}.trackId is invalid: {track_id}")
        if track_id in seen:
            raise CompositionError(f"duplicate trackId: {track_id}")
        seen.add(track_id)
        kind = _require_stable_id(track["kind"], f"{track_context}.kind")
        if kind not in TRACK_KINDS:
            raise CompositionError(f"{track_context}.kind is invalid: {kind}")
        if kind not in ADMITTED_TRACK_KINDS:
            raise CompositionError(
                f"{track_context}.kind is not implemented in formatVersion 1: {kind}"
            )
        start_ms = _require_nonnegative_int(track["startMs"], f"{track_context}.startMs")
        end_ms = None
        if "endMs" in track:
            end_ms = _require_nonnegative_int(
                track["endMs"], f"{track_context}.endMs"
            )
            if end_ms <= start_ms:
                raise CompositionError(f"{track_context}.endMs must be greater than startMs")
        if start_ms > duration_ms or (end_ms is not None and end_ms > duration_ms):
            raise CompositionError(f"{track_context} exceeds durationMs")
        payload = track["payload"]
        if not isinstance(payload, dict):
            raise CompositionError(f"{track_context}.payload must be an object")
        if kind == "WORLD_SEQUENCE":
            _require_exact_fields(
                payload, ("instanceId",), (), f"{track_context}.payload"
            )
            instance_id = _require_stable_id(
                payload.get("instanceId"), f"{track_context}.payload.instanceId"
            )
            if instance_id not in ref_ids.get(kind, set()):
                raise CompositionError(f"{track_context} unknown world sequence: {instance_id}")
        elif kind == "CAMERA_SHOT":
            _require_exact_fields(
                payload, ("shotId",), (), f"{track_context}.payload"
            )
            shot_id = _require_stable_id(
                payload.get("shotId"), f"{track_context}.payload.shotId"
            )
            if shot_id not in ref_ids.get(kind, set()):
                raise CompositionError(f"{track_context} unknown camera shot: {shot_id}")
        elif kind == "ACTOR_PATTERN":
            _require_exact_fields(
                payload,
                ("bossCompositionId", "patternId"),
                (),
                f"{track_context}.payload",
            )
            referenced_boss = _require_stable_id(
                payload.get("bossCompositionId"),
                f"{track_context}.payload.bossCompositionId",
            )
            if referenced_boss != boss_id:
                raise CompositionError(f"{track_context} bossCompositionId mismatch")
            pattern_id = _require_stable_id(
                payload.get("patternId"), f"{track_context}.payload.patternId"
            )
            boss_pattern_ids = {
                row.get("patternId")
                for row in boss.get("patterns", [])
                if isinstance(row, dict)
            }
            if pattern_id not in boss_pattern_ids:
                raise CompositionError(
                    f"{track_context} references unknown boss pattern: {pattern_id}"
                )
        if kind in intervals and end_ms is not None:
            intervals[kind].append((start_ms, end_ms, track_id))
    for kind, rows in intervals.items():
        rows.sort()
        for previous, current in zip(rows, rows[1:]):
            if current[0] < previous[1]:
                raise CompositionError(
                    f"{context} overlapping {kind} tracks: {previous[2]} and {current[2]}"
                )
    return {"sources": sources, "sourcePaths": source_paths}


def _file_identity(root: Path, relative: str) -> dict[str, Any]:
    path = root / relative
    try:
        payload = path.read_bytes()
    except OSError as exc:
        raise CompositionError(f"cannot snapshot source {relative}: {exc}") from exc
    return {"path": relative, "sha256": _sha256_bytes(payload), "bytes": len(payload)}


def source_manifest(root: Path, paths: Iterable[str]) -> dict[str, Any]:
    entries = [_file_identity(root, path) for path in sorted(set(paths))]
    manifest_id = _sha256_bytes(
        b"".join(
            f"{row['path']}\0{row['sha256']}\n".encode("utf-8") for row in entries
        )
    )
    return {
        "schema": "lostark.composition-source-manifest",
        "formatVersion": 1,
        "sourceManifestId": manifest_id,
        "files": entries,
    }


def load_and_validate_all(root: Path) -> dict[str, dict[str, Any]]:
    documents = {relative: read_json(root / relative) for relative in ALL_AUTHORING}
    expected_fixed_identities = {
        BOSS_AUTHORING[0]: ("compositionId", "boss.composition.valtan"),
        BOSS_AUTHORING[1]: ("compositionId", "boss.composition.kakulsaydon"),
        SEQUENCER_AUTHORING[0]: ("sequencerId", "arena.sequencer.valtan"),
        SEQUENCER_AUTHORING[1]: ("sequencerId", "arena.sequencer.kakulsaydon"),
    }
    for relative, (field, expected) in expected_fixed_identities.items():
        if documents[relative].get(field) != expected:
            raise CompositionError(
                f"fixed authoring path {relative} must contain {field}={expected}"
            )
    boss_documents: dict[str, dict[str, Any]] = {}
    validations: dict[str, dict[str, Any]] = {}
    for relative in BOSS_AUTHORING:
        document = documents[relative]
        validation = validate_boss_document(root, document, relative)
        boss_documents[str(document["compositionId"])] = document
        validations[relative] = validation
    if len(boss_documents) != len(BOSS_AUTHORING):
        raise CompositionError("duplicate boss compositionId")
    sequencer_ids: set[str] = set()
    for relative in SEQUENCER_AUTHORING:
        document = documents[relative]
        validation = validate_sequencer_document(
            root, document, relative, boss_documents
        )
        sequencer_id = str(document["sequencerId"])
        if sequencer_id in sequencer_ids:
            raise CompositionError(f"duplicate sequencerId: {sequencer_id}")
        sequencer_ids.add(sequencer_id)
        validations[relative] = validation
    return {
        relative: {"document": documents[relative], **validations[relative]}
        for relative in ALL_AUTHORING
    }


def build_products(root: Path) -> tuple[dict[str, bytes], dict[str, Any]]:
    validated = load_and_validate_all(root)
    all_paths = set(ALL_AUTHORING)
    for value in validated.values():
        all_paths.update(value["sourcePaths"])
    before = source_manifest(root, all_paths)
    # Parse again after the pinned read-set has been captured.  This closes the
    # otherwise small race where an owner could change between the first parse
    # and the manifest snapshot while still producing a self-consistent, but
    # stale, resolved Product.
    validated = load_and_validate_all(root)
    confirmed_paths = set(ALL_AUTHORING)
    for value in validated.values():
        confirmed_paths.update(value["sourcePaths"])
    if confirmed_paths != all_paths:
        raise CompositionError(
            "composition source-document closure changed during validate/stage"
        )
    products: dict[str, bytes] = {}
    product_rows: list[dict[str, Any]] = []
    for source_relative in ALL_AUTHORING:
        value = validated[source_relative]
        document = value["document"]
        manifest = source_manifest(
            root, (source_relative, *value["sourcePaths"])
        )
        if document["schema"] == BOSS_SCHEMA:
            product = {
                "schema": BOSS_PRODUCT_SCHEMA,
                "formatVersion": 1,
                "compositionId": document["compositionId"],
                "sourceRevision": document["revision"],
                "compositionRevision": manifest["sourceManifestId"],
                "status": document["status"],
                "runtimeEligible": document["status"] == "AUTHORITATIVE",
                "displayName": document["displayName"],
                "bossArchetypeId": document["bossArchetypeId"],
                "encounterId": document["encounterId"],
                "areaId": document["areaId"],
                "sourceManifest": manifest,
                "coverage": copy.deepcopy(document["coverage"]),
                "patternIndex": copy.deepcopy(document["patterns"]),
                "resolved": value["resolved"],
            }
        else:
            product = {
                "schema": SEQUENCER_PRODUCT_SCHEMA,
                "formatVersion": 1,
                "sequencerId": document["sequencerId"],
                "sourceRevision": document["revision"],
                "compositionRevision": manifest["sourceManifestId"],
                "status": document["status"],
                "runtimeEligible": document["status"] == "AUTHORITATIVE",
                "displayName": document["displayName"],
                "areaId": document["areaId"],
                "bossCompositionId": document["bossCompositionId"],
                "durationMs": document["durationMs"],
                "sourceManifest": manifest,
                "tracks": copy.deepcopy(document["tracks"]),
            }
        relative = PRODUCT_RELATIVE_PATHS[source_relative]
        payload = (json.dumps(product, ensure_ascii=False, indent=2) + "\n").encode(
            "utf-8"
        )
        products[relative] = payload
        product_rows.append(
            {"path": relative, "sha256": _sha256_bytes(payload), "bytes": len(payload)}
        )
    after = source_manifest(root, all_paths)
    if before != after:
        raise CompositionError("composition sources changed during validate/stage")
    receipt = {
        "schema": RECEIPT_SCHEMA,
        "formatVersion": 1,
        "sourceManifest": before,
        "products": sorted(product_rows, key=lambda row: row["path"]),
    }
    receipt_payload = (json.dumps(receipt, ensure_ascii=False, indent=2) + "\n").encode(
        "utf-8"
    )
    products["Composition.publish.receipt.json"] = receipt_payload
    return products, receipt


def sync_valtan_shadow(root: Path) -> dict[str, Any]:
    """Refresh the derived SHADOW graph without changing its split owners."""

    relative = BOSS_AUTHORING[0]
    path = root / relative
    baseline = path.read_bytes()
    document = read_json(path)
    if document.get("schema") != BOSS_SCHEMA:
        raise CompositionError(f"{relative}.schema must be {BOSS_SCHEMA}")
    if document.get("compositionId") != "boss.composition.valtan":
        raise CompositionError(f"{relative} has the wrong compositionId")
    if document.get("status") != "SHADOW":
        raise CompositionError("only a SHADOW Valtan composition may be synchronized")
    sources, source_paths = _validate_source_documents(
        root, document.get("sourceDocuments"), f"{relative}.sourceDocuments"
    )
    source_before = source_manifest(root, source_paths)
    joined = _valtan_join(root, sources)
    candidate = project_valtan_shadow_index(document, joined, root)
    pattern_count, stage_count, _identity = _valtan_identity(joined)
    # Validate the candidate against the same physical split-owner closure
    # before any descriptor byte is staged or replaced.
    validate_boss_document(root, candidate, relative)
    if source_manifest(root, source_paths) != source_before:
        raise CompositionError(
            "Valtan split owners changed while the shadow index was staged"
        )
    payload = (
        baseline
        if candidate == document
        else (json.dumps(candidate, ensure_ascii=False, indent=2) + "\n").encode(
            "utf-8"
        )
    )
    if path.read_bytes() != baseline:
        raise CompositionError("Valtan composition changed while shadow graph was staged")
    if payload != baseline:
        stage = path.with_name(f".{path.name}.stage.{uuid.uuid4().hex}")
        try:
            with stage.open("xb") as stream:
                stream.write(payload)
                stream.flush()
                os.fsync(stream.fileno())
            os.replace(stage, path)
        finally:
            stage.unlink(missing_ok=True)
    validate_boss_document(root, read_json(path), relative)
    return {
        "compositionId": candidate["compositionId"],
        "revision": candidate["revision"],
        "patternCount": pattern_count,
        "stageCount": stage_count,
        "changed": payload != baseline,
    }


def project_valtan_shadow_index(
    document: Mapping[str, Any],
    joined: Mapping[str, Any],
    root: Path | None = None,
) -> dict[str, Any]:
    """Project only the stable source index owned by the SHADOW descriptor."""
    try:
        return valtan.project_valtan_composition_shadow_index(
            document, joined, root
        )
    except valtan.PipelineError as exc:
        raise CompositionError(str(exc)) from exc


def _sha256_file(path: Path) -> str:
    try:
        return _sha256_bytes(path.read_bytes())
    except OSError as exc:
        raise CompositionError(f"cannot hash transaction file {path}: {exc}") from exc


def _publish_lock_path(root: Path, output_root: Path) -> Path:
    identity = str(output_root.resolve())
    if os.name == "nt":
        identity = identity.casefold()
    key = _sha256_bytes(identity.encode("utf-8"))
    return root.resolve() / "out/CompositionPipeline/locks" / f"{key}.lock"


@contextlib.contextmanager
def _exclusive_publish_lock(
    root: Path, output_root: Path, timeout_seconds: float
) -> Iterable[None]:
    if (
        isinstance(timeout_seconds, bool)
        or not isinstance(timeout_seconds, (int, float))
        or not math.isfinite(float(timeout_seconds))
        or timeout_seconds < 0
    ):
        raise CompositionError("publish lock timeout must be a finite non-negative number")
    lock_path = _publish_lock_path(root, output_root)
    lock_path.parent.mkdir(parents=True, exist_ok=True)
    stream = lock_path.open("a+b")
    acquired = False
    deadline = time.monotonic() + float(timeout_seconds)
    try:
        stream.seek(0, os.SEEK_END)
        if stream.tell() == 0:
            stream.write(b"\0")
            stream.flush()
            os.fsync(stream.fileno())
        while True:
            try:
                stream.seek(0)
                if os.name == "nt":
                    import msvcrt

                    msvcrt.locking(stream.fileno(), msvcrt.LK_NBLCK, 1)
                else:
                    import fcntl

                    fcntl.flock(stream.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                acquired = True
                break
            except (OSError, BlockingIOError):
                if time.monotonic() >= deadline:
                    raise CompositionError(
                        f"composition publish output is locked: {output_root}"
                    )
                time.sleep(min(0.025, max(0.0, deadline - time.monotonic())))
        yield
    finally:
        if acquired:
            try:
                stream.seek(0)
                if os.name == "nt":
                    import msvcrt

                    msvcrt.locking(stream.fileno(), msvcrt.LK_UNLCK, 1)
                else:
                    import fcntl

                    fcntl.flock(stream.fileno(), fcntl.LOCK_UN)
            except OSError:
                pass
        stream.close()


def _journal_sha256(document: Mapping[str, Any]) -> str:
    value = copy.deepcopy(dict(document))
    value["journalSha256"] = ""
    return _sha256_bytes(_canonical_bytes(value))


def _write_publish_journal(path: Path, document: Mapping[str, Any]) -> dict[str, Any]:
    sealed = copy.deepcopy(dict(document))
    sealed["journalSha256"] = ""
    sealed["journalSha256"] = _journal_sha256(sealed)
    payload = (json.dumps(sealed, ensure_ascii=False, indent=2) + "\n").encode("utf-8")
    stage = path.with_name(f".{path.name}.stage.{uuid.uuid4().hex}")
    try:
        with stage.open("xb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(stage, path)
    finally:
        stage.unlink(missing_ok=True)
    return sealed


def _transaction_paths(
    output_root: Path, transaction_id: str, relative: str
) -> tuple[Path, Path, Path]:
    pure = PurePosixPath(relative)
    if pure.is_absolute() or any(part in ("", ".", "..") for part in pure.parts):
        raise CompositionError(f"publish journal has unsafe Product path: {relative}")
    destination = output_root / Path(*pure.parts)
    stage = destination.with_name(f".{destination.name}.stage.{transaction_id}")
    backup = destination.with_name(f".{destination.name}.rollback.{transaction_id}")
    return destination, stage, backup


def _read_publish_journal(output_root: Path) -> dict[str, Any] | None:
    path = output_root / PUBLISH_JOURNAL_NAME
    if not path.exists():
        return None
    document = read_json(path)
    _require_exact_fields(
        document,
        (
            "schema",
            "formatVersion",
            "transactionId",
            "outputRoot",
            "sourceManifestId",
            "state",
            "products",
            "journalSha256",
        ),
        (),
        "Composition publish journal",
    )
    transaction_id = document["transactionId"]
    if (
        document["schema"] != PUBLISH_JOURNAL_SCHEMA
        or document["formatVersion"] != 1
        or isinstance(document["formatVersion"], bool)
        or not isinstance(transaction_id, str)
        or re.fullmatch(r"[0-9a-f]{32}", transaction_id) is None
        or document["outputRoot"] != str(output_root)
        or re.fullmatch(r"[0-9a-f]{64}", str(document["sourceManifestId"])) is None
        or document["state"] not in ("PREPARED", "COMMITTED")
        or re.fullmatch(r"[0-9a-f]{64}", str(document["journalSha256"])) is None
        or document["journalSha256"] != _journal_sha256(document)
    ):
        raise CompositionError("Composition publish journal contract is invalid")
    expected_relatives = sorted(PRODUCT_RELATIVE_PATHS.values()) + [PUBLISH_RECEIPT_REL]
    rows = document["products"]
    if not isinstance(rows, list) or len(rows) != len(expected_relatives):
        raise CompositionError("Composition publish journal Product closure is invalid")
    seen: set[str] = set()
    for ordinal, row in enumerate(rows):
        context = f"Composition publish journal.products[{ordinal}]"
        if not isinstance(row, dict):
            raise CompositionError(f"{context} must be an object")
        _require_exact_fields(
            row,
            (
                "relativePath",
                "expectedSha256",
                "expectedBytes",
                "hadDestination",
                "previousSha256",
            ),
            (),
            context,
        )
        relative = _require_string(row["relativePath"], f"{context}.relativePath")
        _transaction_paths(output_root, transaction_id, relative)
        if relative in seen:
            raise CompositionError(f"duplicate publish journal Product: {relative}")
        seen.add(relative)
        if re.fullmatch(r"[0-9a-f]{64}", str(row["expectedSha256"])) is None:
            raise CompositionError(f"{context}.expectedSha256 is invalid")
        _require_nonnegative_int(row["expectedBytes"], f"{context}.expectedBytes")
        if not isinstance(row["hadDestination"], bool):
            raise CompositionError(f"{context}.hadDestination must be bool")
        previous = row["previousSha256"]
        if row["hadDestination"]:
            if not isinstance(previous, str) or re.fullmatch(r"[0-9a-f]{64}", previous) is None:
                raise CompositionError(f"{context}.previousSha256 is invalid")
        elif previous is not None:
            raise CompositionError(f"{context}.previousSha256 must be null")
    actual_relatives = [row["relativePath"] for row in rows]
    if actual_relatives != expected_relatives:
        raise CompositionError(
            "Composition publish journal Product order/closure is invalid"
        )
    return document


def _cleanup_committed_publish(
    output_root: Path, journal: Mapping[str, Any]
) -> None:
    transaction_id = str(journal["transactionId"])
    errors: list[str] = []
    for row in journal["products"]:
        destination, stage, backup = _transaction_paths(
            output_root, transaction_id, row["relativePath"]
        )
        if (
            not destination.is_file()
            or _sha256_file(destination) != row["expectedSha256"]
            or destination.stat().st_size != row["expectedBytes"]
        ):
            raise CompositionError(
                "committed Composition Product generation is incomplete: "
                f"{row['relativePath']}"
            )
        for artifact in (stage, backup):
            try:
                artifact.unlink(missing_ok=True)
            except OSError as exc:
                errors.append(f"{artifact}: {exc}")
    if errors:
        raise CompositionError(
            "composition Products committed, but transaction cleanup is incomplete: "
            + "; ".join(errors)
        )
    try:
        (output_root / PUBLISH_JOURNAL_NAME).unlink(missing_ok=True)
    except OSError as exc:
        raise CompositionError(
            "composition Products committed, but publish journal cleanup is "
            f"incomplete: {exc}"
        ) from exc


def _rollback_prepared_publish(
    output_root: Path, journal: Mapping[str, Any]
) -> None:
    transaction_id = str(journal["transactionId"])
    errors: list[str] = []
    for row in reversed(journal["products"]):
        destination, stage, backup = _transaction_paths(
            output_root, transaction_id, row["relativePath"]
        )
        try:
            if backup.exists():
                if not row["hadDestination"]:
                    raise CompositionError(f"unexpected rollback backup: {backup}")
                if _sha256_file(backup) != row["previousSha256"]:
                    raise CompositionError(f"rollback backup hash mismatch: {backup}")
                if destination.exists():
                    if _sha256_file(destination) != row["expectedSha256"]:
                        raise CompositionError(
                            f"unexpected Product bytes during rollback: {destination}"
                        )
                    destination.unlink()
                os.replace(backup, destination)
            elif row["hadDestination"]:
                if (
                    not destination.is_file()
                    or _sha256_file(destination) != row["previousSha256"]
                ):
                    raise CompositionError(
                        f"previous Product is unavailable during rollback: {destination}"
                    )
            elif destination.exists():
                if _sha256_file(destination) != row["expectedSha256"]:
                    raise CompositionError(
                        f"unexpected new Product bytes during rollback: {destination}"
                    )
                destination.unlink()
            stage.unlink(missing_ok=True)
        except (OSError, CompositionError) as exc:
            errors.append(str(exc))
    if errors:
        raise CompositionError("composition Product rollback incomplete: " + "; ".join(errors))
    try:
        (output_root / PUBLISH_JOURNAL_NAME).unlink(missing_ok=True)
    except OSError as exc:
        raise CompositionError(
            f"composition Product rollback journal cleanup failed: {exc}"
        ) from exc


def _recover_publish_journal(output_root: Path) -> None:
    journal = _read_publish_journal(output_root)
    if journal is None:
        return
    transaction_id = str(journal["transactionId"])
    receipt = journal["products"][-1]
    receipt_destination, _stage, receipt_backup = _transaction_paths(
        output_root, transaction_id, receipt["relativePath"]
    )
    receipt_matches = (
        receipt_destination.is_file()
        and _sha256_file(receipt_destination) == receipt["expectedSha256"]
    )
    receipt_was_promoted = receipt_matches and (
        journal["state"] == "COMMITTED"
        or receipt_backup.exists()
        or not receipt["hadDestination"]
    )
    if receipt_was_promoted:
        _cleanup_committed_publish(output_root, journal)
    else:
        _rollback_prepared_publish(output_root, journal)


def publish_products(
    root: Path,
    output_root: Path,
    *,
    fail_at: str = "",
    lock_timeout_seconds: float = PUBLISH_LOCK_TIMEOUT_SECONDS,
) -> dict[str, Any]:
    output_root = output_root.resolve()
    output_root.mkdir(parents=True, exist_ok=True)
    with _exclusive_publish_lock(root, output_root, lock_timeout_seconds):
        _recover_publish_journal(output_root)
        products, receipt = build_products(root)
        source_paths = [row["path"] for row in receipt["sourceManifest"]["files"]]
        expected_source_manifest = receipt["sourceManifest"]
        transaction_id = uuid.uuid4().hex
        ordered_relatives = sorted(
            relative for relative in products if relative != PUBLISH_RECEIPT_REL
        ) + [PUBLISH_RECEIPT_REL]
        rows: list[dict[str, Any]] = []
        for relative in ordered_relatives:
            payload = products[relative]
            destination, stage, _backup = _transaction_paths(
                output_root, transaction_id, relative
            )
            destination.parent.mkdir(parents=True, exist_ok=True)
            with stage.open("xb") as stream:
                stream.write(payload)
                stream.flush()
                os.fsync(stream.fileno())
            had_destination = destination.is_file()
            rows.append(
                {
                    "relativePath": relative,
                    "expectedSha256": _sha256_bytes(payload),
                    "expectedBytes": len(payload),
                    "hadDestination": had_destination,
                    "previousSha256": (
                        _sha256_file(destination) if had_destination else None
                    ),
                }
            )
        journal_path = output_root / PUBLISH_JOURNAL_NAME
        journal = _write_publish_journal(
            journal_path,
            {
                "schema": PUBLISH_JOURNAL_SCHEMA,
                "formatVersion": 1,
                "transactionId": transaction_id,
                "outputRoot": str(output_root),
                "sourceManifestId": expected_source_manifest["sourceManifestId"],
                "state": "PREPARED",
                "products": rows,
                "journalSha256": "",
            },
        )
        committed = False
        try:
            if fail_at == "after-stage":
                raise CompositionError("injected failure after stage")
            if source_manifest(root, source_paths) != expected_source_manifest:
                raise CompositionError(
                    "composition sources changed before Product commit"
                )
            for index, row in enumerate(rows[:-1]):
                destination, stage, backup = _transaction_paths(
                    output_root, transaction_id, row["relativePath"]
                )
                if row["hadDestination"]:
                    os.replace(destination, backup)
                os.replace(stage, destination)
                if fail_at == "after-first-promote" and index == 0:
                    raise CompositionError("injected failure after first promote")
                if fail_at == "interrupt-after-first-promote" and index == 0:
                    raise KeyboardInterrupt("injected publish interruption")
            # The optimistic source CAS is deliberately before the receipt.
            # Consumers may therefore use the receipt as the single visible
            # commit marker for a fully promoted and validated generation.
            if source_manifest(root, source_paths) != expected_source_manifest:
                raise CompositionError(
                    "composition sources changed during Product commit"
                )
            receipt_row = rows[-1]
            receipt_destination, receipt_stage, receipt_backup = _transaction_paths(
                output_root, transaction_id, receipt_row["relativePath"]
            )
            if receipt_row["hadDestination"]:
                os.replace(receipt_destination, receipt_backup)
            os.replace(receipt_stage, receipt_destination)
            committed = True
            journal["state"] = "COMMITTED"
            journal = _write_publish_journal(journal_path, journal)
            _cleanup_committed_publish(output_root, journal)
            return receipt
        except Exception as original:
            if committed:
                raise
            try:
                _rollback_prepared_publish(output_root, journal)
            except CompositionError as rollback_error:
                raise CompositionError(
                    f"{original}; {rollback_error}"
                ) from original
            raise


def _default_output_root(root: Path) -> Path:
    return root / "Client/Bin/DataFiles/Compositions"


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repository-root", type=Path, default=REPOSITORY_ROOT)
    subparsers = parser.add_subparsers(dest="command", required=True)
    subparsers.add_parser("validate")
    subparsers.add_parser("sync-valtan-shadow")
    publish = subparsers.add_parser("publish")
    publish.add_argument("--output-root", type=Path)
    publish.add_argument(
        "--fail-at", choices=("", "after-stage", "after-first-promote"), default=""
    )
    args = parser.parse_args(argv)
    root = args.repository_root.resolve()
    try:
        if args.command == "sync-valtan-shadow":
            result = sync_valtan_shadow(root)
            print(
                "Valtan composition shadow sync passed: "
                f"patterns={result['patternCount']} stages={result['stageCount']} "
                f"revision={result['revision']} changed={str(result['changed']).lower()}"
            )
            return 0
        if args.command == "validate":
            validated = load_and_validate_all(root)
            valtan_patterns = validated[BOSS_AUTHORING[0]]["resolved"][
                "joinedPatternMaster"
            ]["patterns"]
            kakul_profiles = validated[BOSS_AUTHORING[1]]["resolved"][
                "referenceProfiles"
            ]
            kakul_actions = sum(
                len(row["actionReference"]["actions"]) for row in kakul_profiles
            )
            print(
                "Composition validation passed: "
                f"Valtan patterns={len(valtan_patterns)}, "
                f"Kakul profiles={len(kakul_profiles)} actions={kakul_actions}, "
                f"arena sequencers={len(SEQUENCER_AUTHORING)}"
            )
            return 0
        output_root = args.output_root or _default_output_root(root)
        receipt = publish_products(root, output_root, fail_at=args.fail_at)
        print(
            "Composition publish passed: "
            f"sourceManifestId={receipt['sourceManifest']['sourceManifestId']}"
        )
        return 0
    except CompositionError as exc:
        print(f"Composition pipeline failed: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
