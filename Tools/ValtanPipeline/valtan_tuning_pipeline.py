#!/usr/bin/env python3
"""Valtan G01/G02 authoring migration and candidate publication pipeline.

The shipping Client still consumes products projected from the format-v1-era
contract.  This module never overwrites Data/Valtan/Valtan.pattern.json.  The
physical v2 authoring sources are gameplay/presentation documents that join to
one strict in-memory v2 IR before projection into an immutable candidate.
"""

from __future__ import annotations

import argparse
import base64
import copy
import ctypes
import hashlib
import json
import math
import os
import re
import shutil
import stat
import subprocess
import sys
import tempfile
import uuid
from collections import Counter
from pathlib import Path
from typing import Any, Iterable, Mapping, Sequence


MASTER_REL = "Data/Valtan/Valtan.pattern.json"
GAMEPLAY_AUTHORING_REL = "Data/Valtan/Valtan.gameplay.json"
SAVED_FLOW_REL = "Data/Encounters/Valtan/ValtanBossAuditionFlows.json"
SAVED_FLOW_MAX_SLOTS = 255
DEFAULT_SAVED_FLOW_ID = "flow.valtan.boss-tool.default"
SAVED_FLOW_MAX_BYTES = 256 * 1024
OPTIONAL_ENTRY_PATTERN_ID = "VALTAN_ENTRANCE_CINEMATIC"
PRESENTATION_AUTHORING_REL = "Data/Valtan/Valtan.presentation.json"
DEBUG_PRESENTATION_REL = "Data/Valtan/Valtan.presentation.debug.json"
ANIMATION_PROMOTION_MANIFEST_REL = (
    "Data/Valtan/Valtan.animation-chain-promotions.json"
)
COMBAT_AUTHORING_REL = "Data/Valtan/Valtan.combatobjects.json"
WORLD_SET_REL = "Data/Valtan/Valtan.worldeventsets.json"
LEGACY_REL = "Data/Valtan/Valtan.legacy-compatibility.json"
ENCOUNTER_REL = "Data/Encounters/Valtan/ValtanEncounter.json"
ROTATIONS_REL = "Data/Encounters/Valtan/ValtanPatternRotations.json"
COMBAT_PRODUCT_REL = "Data/Encounters/Valtan/ValtanCombatObjects.json"
CAMERA_REL = "Data/Encounters/Valtan/ValtanCinematicCamera.json"
WORLD_PRODUCT_REL = "Data/Encounters/Valtan/ValtanWorldEvents.json"
BINDINGS_REL = "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
CUES_REL = "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
BOSS_CATALOG_REL = "Data/Actors/BossCatalog.json"
BOSS_PROFILES_REL = "Data/Balance/BossProfiles.json"
DAMAGE_REL = "Data/Balance/DamageProfiles.json"
EFFECT_CATALOG_REL = "Data/Effects/EffectCatalog.json"
PROVENANCE_REL = "Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json"
GAMEPLAY_BOOTSTRAP_REL = "Runtime/Gameplay/Gameplay.bootstrap"
GAMEPLAY_BOOTSTRAP_VERSION = 26

SCRIPTED_SEQUENCE_MODE = "ORDERED_ONCE_THEN_IDLE"

ANIMATION_MODE_CLIP_SEQUENCE = "CLIP_SEQUENCE"
ANIMATION_MODE_NONE = "NONE"
CUE_TIMING_BASIS_CLIP_OCCURRENCE = "CLIP_OCCURRENCE"
CUE_TIMING_BASIS_STAGE_CLOCK = "STAGE_CLOCK"

REPOSITORY_PRODUCT_ARTIFACTS = (
    ENCOUNTER_REL,
    ROTATIONS_REL,
    COMBAT_PRODUCT_REL,
    WORLD_PRODUCT_REL,
    BINDINGS_REL,
    CUES_REL,
    PROVENANCE_REL,
)

AUTHORING_ARTIFACTS = (
    GAMEPLAY_AUTHORING_REL,
    PRESENTATION_AUTHORING_REL,
    BOSS_PROFILES_REL,
    DAMAGE_REL,
    COMBAT_AUTHORING_REL,
    WORLD_SET_REL,
    LEGACY_REL,
    PROVENANCE_REL,
    SAVED_FLOW_REL,
)
LEGACY_AUTHORING_ARTIFACTS = tuple(
    relative for relative in AUTHORING_ARTIFACTS if relative != SAVED_FLOW_REL
)

WORLD_SET_ID = "worldeventset.valtan.arena-break-109.outer-wall"
WORLD_PATTERN_ID = "VALTAN_ARENA_BREAK_109"
WORLD_STAGE_ID = "IMPACT"
WORLD_TRIGGER_KIND = "STAGE_ENTER"
WORLD_EVENT_ID = "event.valtan.arena-break-109.impact.trigger-outer-wall"
WORLD_SET_OWNERS = {
    WORLD_SET_ID: (WORLD_PATTERN_ID, WORLD_STAGE_ID, WORLD_TRIGGER_KIND),
    "worldeventset.valtan.terrain-destruction-3.floor84": (
        "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
        "IMPACT",
        WORLD_TRIGGER_KIND,
    ),
    "worldeventset.valtan.terrain-destruction-9.floor30": (
        "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK",
        "IMPACT",
        WORLD_TRIGGER_KIND,
    ),
}
WORLD_SET_LEGACY_OWNERS = {
    "worldeventset.valtan.terrain-destruction-3.floor84": (
        "VALTAN_ARENA_BREAK_84",
        "IMPACT",
        WORLD_TRIGGER_KIND,
    ),
    "worldeventset.valtan.terrain-destruction-9.floor30": (
        "VALTAN_ARENA_BREAK_33",
        "LANDING",
        WORLD_TRIGGER_KIND,
    ),
}
MANAGED_WORLD_SET_OWNERS = dict(WORLD_SET_OWNERS)

# These two sealed v1 rows are retained only as migration evidence after their
# Product mechanics were replaced by the split-authoring 3/9-o'clock patterns.
# They must remain byte-identical in the compatibility receipt, but they are no
# longer expected to exist in the current Encounter Product.
ARCHIVED_LEGACY_PATTERN_IDS = frozenset(
    ("VALTAN_ARENA_BREAK_84", "VALTAN_ARENA_BREAK_33")
)

# The frozen v1 fixture cannot own these later split-authoring replacements,
# but the current Product already does.  Treat them as migration-managed while
# validating legacy preservation so the old 84/33 receipt remains sealed
# evidence instead of being mistaken for a second live mechanic owner.
CURRENT_MIGRATION_MANAGED_PATTERN_IDS = frozenset(
    (
        "VALTAN_ENTRANCE_CINEMATIC",
        "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
        "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK",
    )
)

MANAGED_PATTERN_IDS = (
    "VALTAN_WHIRLWIND",
    "VALTAN_DASH_CHARGE",
    "VALTAN_FOUR_SLASH",
    "VALTAN_FIST_IN_OUT",
    "VALTAN_HIGH_JUMP",
    "VALTAN_FLOOR_WIPE_130",
    "VALTAN_ARENA_BREAK_109",
)
MANAGED_ROTATION_IDS = frozenset(
    ("rotation.valtan.160.130", "rotation.valtan.130.109")
)
MANUAL_SERVER_AUDITION = "MANUAL_SERVER_AUDITION"
ANIMATION_INTAKE_ONLY = "ANIMATION_INTAKE_ONLY"
DERIVED_SERVER_PATTERN = "DERIVED_SERVER_PATTERN"
AUDITION_ONLY = "AUDITION_ONLY"
ALLOWED_MAPPING_BASES = frozenset(
    (
        "CURRENT_PRODUCT_BASELINE",
        "PATTERN_PR_REFERENCE",
        "ANIMATION_PR_127",
        "SOURCE_REVIEWED_DELTA",
        "PROJECT_AUTHORED",
        "LEGACY_V1_MIGRATION",
    )
)

OWNER_RELATIVE = "OWNER_RELATIVE"
GAMEPLAY_FOOTPRINT = "GAMEPLAY_FOOTPRINT"
ARENA_ABSOLUTE = "ARENA_ABSOLUTE"
WORLD_SCALE_POLICY_KINDS = frozenset((GAMEPLAY_FOOTPRINT, ARENA_ABSOLUTE))
MANAGED_CUE_SCALE_POLICIES = {
    "cue.valtan.carrier-v1.attack.whirlwind.recovery.clip-01": GAMEPLAY_FOOTPRINT,
    "cue.valtan.whirlwind.active": GAMEPLAY_FOOTPRINT,
    "cue.valtan.project-tuned.attack.dash-charge.windup-telegraph": GAMEPLAY_FOOTPRINT,
    "cue.valtan.project-tuned.attack.dash-charge.active-shield": OWNER_RELATIVE,
    "cue.valtan.carrier-v1.attack.four-slash.active.clip-01": GAMEPLAY_FOOTPRINT,
    "cue.valtan.carrier-v1.attack.four-slash.active.clip-02": GAMEPLAY_FOOTPRINT,
    "cue.valtan.carrier-v1.attack.four-slash.recovery.clip-01": OWNER_RELATIVE,
    "cue.valtan.carrier-v1.attack.fist-in-out.inner.clip-01": GAMEPLAY_FOOTPRINT,
    "cue.valtan.carrier-v1.attack.high-jump.takeoff.clip-01": OWNER_RELATIVE,
    "cue.valtan.carrier-v1.attack.high-jump.land.clip-01": GAMEPLAY_FOOTPRINT,
    "cue.valtan.carrier-v1.mechanic.floor-wipe-130.windup.clip-01": GAMEPLAY_FOOTPRINT,
    "cue.valtan.carrier-v1.mechanic.floor-wipe-130.second-smash.clip-01": GAMEPLAY_FOOTPRINT,
    "cue.valtan.carrier-v1.mechanic.arena-break-109.takeoff.clip-01": ARENA_ABSOLUTE,
    "cue.valtan.carrier-v1.mechanic.arena-break-109.impact.clip-01": ARENA_ABSOLUTE,
    "cue.valtan.carrier-v1.mechanic.arena-break-109.roar-recovery.clip-01": OWNER_RELATIVE,
    "cue.valtan.phase2.four.slashes": GAMEPLAY_FOOTPRINT,
    **{
        f"cue.valtan.phase2.warp.step-{leg:02d}.composite": OWNER_RELATIVE
        for leg in range(2, 10)
    },
    **{
        f"cue.valtan.finale.warp.step-{leg:02d}.composite": OWNER_RELATIVE
        for leg in range(2, 10)
    },
    "cue.valtan.phase2.terrain.four-axe.01": GAMEPLAY_FOOTPRINT,
    "cue.valtan.phase2.terrain.four-axe.02": GAMEPLAY_FOOTPRINT,
    "cue.valtan.phase2.terrain.four-axe.03": GAMEPLAY_FOOTPRINT,
    "cue.valtan.phase2.terrain.four-axe.04": GAMEPLAY_FOOTPRINT,
    "cue.valtan.requested.20260827.terrain-3.semicircle": GAMEPLAY_FOOTPRINT,
    "cue.valtan.requested.20260827.terrain-9.semicircle": GAMEPLAY_FOOTPRINT,
    "cue.valtan.requested.20260827.six-pizza.composite": GAMEPLAY_FOOTPRINT,
    "cue.valtan.requested.20260827.attack-whirlwind.composite": GAMEPLAY_FOOTPRINT,
    "cue.valtan.requested.20260827.charge.axe-follow": OWNER_RELATIVE,
    "cue.valtan.requested.20260827.charge2.red-fan": GAMEPLAY_FOOTPRINT,
    "cue.valtan.requested.20260827.roar-charge.composite": OWNER_RELATIVE,
    "cue.valtan.requested.20260827.three.composite": GAMEPLAY_FOOTPRINT,
    "cue.valtan.requested.20260827.front-back-front.electric-fan": GAMEPLAY_FOOTPRINT,
    "cue.valtan.requested.20260827.counter.cyan-roar-ring": GAMEPLAY_FOOTPRINT,
    "cue.valtan.requested.20260827.trash.composite": OWNER_RELATIVE,
    "cue.valtan.requested.20260827.trash-catch-success.composite": OWNER_RELATIVE,
    "cue.valtan.requested.20260827.trash-catch-fail.composite": OWNER_RELATIVE,
    "cue.valtan.requested.20260827.trash-catch-if.composite": OWNER_RELATIVE,
    "cue.valtan.requested.20260827.catch-breath.composite": OWNER_RELATIVE,
    "cue.valtan.requested.20260827.struggling.composite": OWNER_RELATIVE,
}
MIGRATION_MANAGED_CUE_IDS = frozenset(
    (
        "cue.valtan.carrier-v1.attack.whirlwind.recovery.clip-01",
        "cue.valtan.whirlwind.active",
        "cue.valtan.project-tuned.attack.dash-charge.windup-telegraph",
        "cue.valtan.project-tuned.attack.dash-charge.active-shield",
        "cue.valtan.carrier-v1.attack.four-slash.active.clip-01",
        "cue.valtan.carrier-v1.attack.four-slash.active.clip-02",
        "cue.valtan.carrier-v1.attack.four-slash.recovery.clip-01",
        "cue.valtan.carrier-v1.attack.fist-in-out.inner.clip-01",
        "cue.valtan.carrier-v1.attack.high-jump.takeoff.clip-01",
        "cue.valtan.carrier-v1.attack.high-jump.land.clip-01",
        "cue.valtan.carrier-v1.mechanic.floor-wipe-130.windup.clip-01",
        "cue.valtan.carrier-v1.mechanic.floor-wipe-130.second-smash.clip-01",
        "cue.valtan.carrier-v1.mechanic.arena-break-109.takeoff.clip-01",
        "cue.valtan.carrier-v1.mechanic.arena-break-109.impact.clip-01",
        "cue.valtan.carrier-v1.mechanic.arena-break-109.roar-recovery.clip-01",
    )
)
EXPECTED_MANAGED_SCALE_POLICY_COUNTS = Counter(
    MANAGED_CUE_SCALE_POLICIES.values()
)

STABLE_ID_RE = re.compile(r"^[A-Za-z0-9_.-]{1,160}$")


class PipelineError(RuntimeError):
    error_code = "VALIDATION_FAILED"

    def as_error(self) -> dict[str, Any]:
        return {
            "document": "",
            "path": "",
            "patternId": "",
            "stageId": "",
            "field": "",
            "errorCode": self.error_code,
            "message": str(self),
        }


class InjectedFailure(PipelineError):
    error_code = "FAILURE_INJECTED"


class RuntimeProjectionError(PipelineError):
    error_code = "RUNTIME_PROJECTION_UNSUPPORTED"

    def __init__(
        self,
        message: str,
        *,
        pattern_id: str = "",
        stage_id: str = "",
        field: str = "",
    ) -> None:
        super().__init__(message)
        self.pattern_id = pattern_id
        self.stage_id = stage_id
        self.field = field

    def as_error(self) -> dict[str, Any]:
        return {
            "document": GAMEPLAY_AUTHORING_REL,
            "path": "patterns[].stages[].events[]",
            "patternId": self.pattern_id,
            "stageId": self.stage_id,
            "field": self.field,
            "errorCode": self.error_code,
            "message": str(self),
        }


class DraftPatchError(PipelineError):
    """A stable-ID draft operation failed without mutating repository sources."""

    error_code = "DRAFT_PATCH_INVALID"

    def __init__(
        self,
        message: str,
        *,
        document: str = "",
        path: str = "",
        pattern_id: str = "",
        stage_id: str = "",
        field: str = "",
        error_code: str | None = None,
    ) -> None:
        super().__init__(message)
        self.document = document
        self.path = path
        self.pattern_id = pattern_id
        self.stage_id = stage_id
        self.field = field
        if error_code is not None:
            self.error_code = error_code

    def as_error(self) -> dict[str, Any]:
        return {
            "document": self.document,
            "path": self.path,
            "patternId": self.pattern_id,
            "stageId": self.stage_id,
            "field": self.field,
            "errorCode": self.error_code,
            "message": str(self),
        }


def _reject_duplicate_pairs(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise PipelineError(f"duplicate JSON property: {key}")
        result[key] = value
    return result


def _reject_nonfinite_constant(value: str) -> Any:
    raise PipelineError(f"non-finite JSON number is forbidden: {value}")


def read_json(path: Path) -> Any:
    data = path.read_bytes()
    if data.startswith(b"\xef\xbb\xbf"):
        raise PipelineError(f"UTF-8 BOM is forbidden: {path}")
    try:
        text = data.decode("utf-8", errors="strict")
        return json.loads(
            text,
            object_pairs_hook=_reject_duplicate_pairs,
            parse_constant=_reject_nonfinite_constant,
        )
    except (UnicodeError, json.JSONDecodeError) as exc:
        raise PipelineError(f"invalid UTF-8 JSON: {path}: {exc}") from exc


def read_text(path: Path) -> str:
    data = path.read_bytes()
    if data.startswith(b"\xef\xbb\xbf"):
        raise PipelineError(f"UTF-8 BOM is forbidden: {path}")
    try:
        return data.decode("utf-8", errors="strict")
    except UnicodeError as exc:
        raise PipelineError(f"invalid UTF-8 text: {path}: {exc}") from exc


def json_text(value: Any) -> str:
    return json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n"


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def source_text_identity(path: Path) -> tuple[str, int]:
    """Hash source JSON with platform line endings normalized to canonical LF."""

    normalized = (
        read_text(path).replace("\r\n", "\n").replace("\r", "\n").encode("utf-8")
    )
    return sha256_bytes(normalized), len(normalized)


def canonical_hash(value: Any) -> str:
    return sha256_bytes(canonical_bytes(value))


def exact(value: Mapping[str, Any], fields: Iterable[str], context: str) -> None:
    expected = set(fields)
    actual = set(value.keys()) if isinstance(value, dict) else set()
    if actual != expected:
        raise PipelineError(
            f"{context} fields mismatch: expected={sorted(expected)} actual={sorted(actual)}"
        )


def stable_id(value: Any, context: str, *, allow_empty: bool = False) -> str:
    if not isinstance(value, str):
        raise PipelineError(f"{context} must be a string")
    if allow_empty and value == "":
        return value
    if not STABLE_ID_RE.fullmatch(value):
        raise PipelineError(f"{context} is not a stable ID: {value!r}")
    return value


def mapping_basis(value: Any, context: str) -> str:
    result = stable_id(value, context)
    if result not in ALLOWED_MAPPING_BASES:
        raise PipelineError(
            f"{context} is not in the allowed mappingBasis vocabulary: {result}"
        )
    return result


def integer(value: Any, context: str, minimum: int = 0, maximum: int = 2**31 - 1) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise PipelineError(f"{context} must be an integer")
    if value < minimum or value > maximum:
        raise PipelineError(f"{context} out of range: {value}")
    return value


def number(value: Any, context: str, minimum: float, maximum: float) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise PipelineError(f"{context} must be a number")
    result = float(value)
    if not math.isfinite(result) or result < minimum or result > maximum:
        raise PipelineError(f"{context} out of range: {value}")
    return result


def boolean(value: Any, context: str) -> bool:
    if not isinstance(value, bool):
        raise PipelineError(f"{context} must be a boolean")
    return value


def unique_index(rows: Sequence[dict[str, Any]], key: str, context: str) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for ordinal, row in enumerate(rows):
        identity = stable_id(row.get(key), f"{context}[{ordinal}].{key}")
        if identity in result:
            raise PipelineError(f"duplicate {context} {key}: {identity}")
        result[identity] = row
    return result


def validate_cue_animation_join(
    cue: dict[str, Any],
    occurrences_by_id: Mapping[str, dict[str, Any]],
    context: str,
) -> dict[str, Any]:
    clip_occurrence_id = stable_id(
        cue.get("clipOccurrenceId"), f"{context}.clipOccurrenceId"
    )
    occurrence = occurrences_by_id.get(clip_occurrence_id)
    if occurrence is None:
        raise PipelineError(
            f"{context} does not resolve its saved animation occurrence: "
            f"{clip_occurrence_id}"
        )

    cue_basis = mapping_basis(cue.get("mappingBasis"), f"{context}.mappingBasis")
    occurrence_basis = mapping_basis(
        occurrence.get("mappingBasis"),
        f"{context} occurrence.mappingBasis",
    )
    if cue_basis != occurrence_basis:
        raise PipelineError(
            f"{context} mappingBasis does not match its saved animation occurrence"
        )

    cue_start = integer(
        cue.get("sourceStartMs"), f"{context}.sourceStartMs", 0, 600000
    )
    occurrence_start = integer(
        occurrence.get("sourceStartMs"),
        f"{context} occurrence.sourceStartMs",
        0,
        600000,
    )
    if cue_start < occurrence_start:
        raise PipelineError(
            f"{context} starts before its saved animation occurrence"
        )

    cue_end = cue.get("sourceEndMs")
    if cue_end is not None:
        cue_end = integer(cue_end, f"{context}.sourceEndMs", 0, 600000)
        if cue_end <= cue_start:
            raise PipelineError(f"{context} source window is not positive")

    occurrence_play_ms = integer(
        occurrence.get("playMs"),
        f"{context} occurrence.playMs",
        0,
        600000,
    )
    if occurrence_play_ms:
        occurrence_end = occurrence_start + occurrence_play_ms
        if cue_start >= occurrence_end or (
            cue_end is not None and cue_end > occurrence_end
        ):
            raise PipelineError(
                f"{context} source window escapes its saved animation occurrence"
            )
    return occurrence


def validate_cue_scale_policy(scale_policy: Any, context: str) -> str:
    if not isinstance(scale_policy, dict):
        raise PipelineError(f"{context}.scalePolicy must be an object")
    kind = scale_policy.get("kind")
    if kind == OWNER_RELATIVE:
        exact(scale_policy, ("kind",), f"{context}.scalePolicy")
        return kind
    if kind not in WORLD_SCALE_POLICY_KINDS:
        raise PipelineError(f"{context}.scalePolicy.kind is unsupported: {kind!r}")
    exact(scale_policy, ("kind", "worldScale"), f"{context}.scalePolicy")
    world_scale = scale_policy["worldScale"]
    if not isinstance(world_scale, list) or len(world_scale) != 3:
        raise PipelineError(f"{context}.scalePolicy.worldScale must be float3")
    normalized = [
        number(
            component,
            f"{context}.scalePolicy.worldScale[{ordinal}]",
            0.000001,
            1000.0,
        )
        for ordinal, component in enumerate(world_scale)
    ]
    if normalized != [1.5, 1.5, 1.5]:
        raise PipelineError(
            f"{context}.scalePolicy.worldScale must preserve the 1.5 gameplay footprint"
        )
    return kind


def repo_path(root: Path, relative: str) -> Path:
    if not isinstance(relative, str) or not relative or "\\" in relative:
        raise PipelineError(f"invalid repository-relative path: {relative!r}")
    candidate = (root / relative).resolve()
    resolved_root = root.resolve()
    try:
        candidate.relative_to(resolved_root)
    except ValueError as exc:
        raise PipelineError(f"path escapes repository root: {relative}") from exc
    return candidate


def staging_root(root: Path, path: Path, context: str) -> Path:
    resolved = path.resolve()
    resolved_repository = root.resolve()
    if resolved == resolved_repository:
        raise DraftPatchError(
            f"{context} must not be the repository root",
            document=context,
            path=str(resolved),
            error_code="OUTPUT_ROOT_FORBIDDEN",
        )
    data_root = (resolved_repository / "Data").resolve()
    try:
        resolved.relative_to(data_root)
    except ValueError:
        pass
    else:
        raise DraftPatchError(
            f"{context} must not write inside repository Data",
            document=context,
            path=str(resolved),
            error_code="OUTPUT_ROOT_FORBIDDEN",
        )
    return resolved


def _is_reparse_point(path: Path) -> bool:
    """Return True for symlinks and Windows junction/other reparse entries."""

    try:
        metadata = path.lstat()
    except FileNotFoundError:
        return False
    if stat.S_ISLNK(metadata.st_mode):
        return True
    file_attributes = getattr(metadata, "st_file_attributes", 0)
    reparse_attribute = getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0x400)
    return bool(file_attributes & reparse_attribute)


def _assert_transaction_path(
    transaction_root: Path,
    path: Path,
    context: str,
) -> None:
    """Reject lexical escapes and every existing reparse component below a root.

    ``Path.resolve`` on the requested output root is not enough: a cooperative
    root can later acquire a ``revisions`` junction that redirects an otherwise
    valid lexical child.  Durable transaction cleanup must never follow it.
    """

    try:
        relative = path.relative_to(transaction_root)
    except ValueError as exc:
        raise PipelineError(f"{context} escapes its transaction root") from exc
    if relative == Path("."):
        raise PipelineError(f"{context} must be below its transaction root")
    if _is_reparse_point(transaction_root) or not transaction_root.is_dir():
        raise PipelineError(f"{context} transaction root is not a regular directory")
    resolved_root = transaction_root.resolve(strict=True)
    current = transaction_root
    parts = relative.parts
    for ordinal, part in enumerate(parts):
        current = current / part
        if _is_reparse_point(current):
            raise PipelineError(f"{context} traverses a reparse point: {current}")
        if not current.exists():
            continue
        if ordinal + 1 < len(parts) and not current.is_dir():
            raise PipelineError(f"{context} ancestor is not a directory: {current}")
        try:
            current.resolve(strict=True).relative_to(resolved_root)
        except ValueError as exc:
            raise PipelineError(f"{context} resolves outside its transaction root") from exc


def _ensure_transaction_directory(
    transaction_root: Path,
    path: Path,
    context: str,
) -> None:
    _assert_transaction_path(transaction_root, path, context)
    if not path.exists():
        path.mkdir()
    _assert_transaction_path(transaction_root, path, context)
    if not path.is_dir():
        raise PipelineError(f"{context} is not a regular directory")


def require_documents(root: Path, relatives: Iterable[str]) -> dict[str, Any]:
    documents: dict[str, Any] = {}
    for relative in relatives:
        path = repo_path(root, relative)
        if not path.is_file():
            raise PipelineError(f"missing document: {relative}")
        documents[relative] = read_json(path)
    return documents


def world_managed_bindings(world: dict[str, Any]) -> list[dict[str, Any]]:
    return [
        binding
        for binding in world["bindings"]
        if binding.get("patternId") == WORLD_PATTERN_ID
        and binding.get("stageId") == WORLD_STAGE_ID
        and binding.get("triggerKind") == WORLD_TRIGGER_KIND
        and binding.get("enabled") is True
    ]


def world_set_source_bindings(
    world: dict[str, Any], set_id: str
) -> list[dict[str, Any]]:
    """Resolve the flat rows owned by one typed Valtan event set."""

    if set_id == WORLD_SET_ID:
        return world_managed_bindings(world)
    group_marker = {
        "worldeventset.valtan.terrain-destruction-3.floor84":
            "destroyable.group.valtan.floor84.",
        "worldeventset.valtan.terrain-destruction-9.floor30":
            "destroyable.group.valtan.floor30.",
    }.get(set_id)
    if group_marker is None:
        raise PipelineError(f"unsupported Valtan world event set: {set_id}")
    mutations = unique_index(world["mutations"], "mutationId", "world mutations")
    result = []
    for binding in world["bindings"]:
        mutation = mutations.get(binding.get("mutationId"))
        if (
            binding.get("enabled") is True
            and mutation is not None
            and str(mutation.get("groupId", "")).startswith(group_marker)
        ):
            result.append(binding)
    return result


# The 109 collapse owns the outer ring plus the interior walls that are still
# standing when it fires, so a member ID keeps its own family segment instead of
# forcing every wall into the outer-ring namespace. wall159 is listed before wall
# only for readability; the two prefixes cannot both match one group ID.
WORLD_MEMBER_GROUP_PREFIXES = (
    "destroyable.group.valtan.outerwall109.",
    "destroyable.group.valtan.wall159.",
    "destroyable.group.valtan.wall.",
)
_WORLD_GROUP_NAMESPACE = "destroyable.group.valtan."


def _world_member_id(group_id: str) -> str:
    for marker in WORLD_MEMBER_GROUP_PREFIXES:
        if group_id.startswith(marker):
            family = marker[len(_WORLD_GROUP_NAMESPACE) : -1]
            return f"worldeventmember.valtan.{family}." + group_id[len(marker) :]
    raise PipelineError(f"109 managed group has unexpected ID: {group_id}")


def build_world_event_sets(world: dict[str, Any]) -> dict[str, Any]:
    exact(
        world,
        ("schema", "formatVersion", "areaId", "encounterId", "provenance", "groups", "mutations", "bindings"),
        "ValtanWorldEvents root",
    )
    if (
        world["schema"] != "lostark.world-destruction-events"
        or world["formatVersion"] != 1
        or world["areaId"] != "LV_LUT_HEARTRB_ED"
        or world["encounterId"] != "ENCOUNTER_VALTAN"
    ):
        raise PipelineError("ValtanWorldEvents header mismatch")
    groups = unique_index(world["groups"], "groupId", "groups")
    mutations = unique_index(world["mutations"], "mutationId", "mutations")
    managed = world_managed_bindings(world)
    if len(managed) != 97:
        raise PipelineError(f"initial 109 migration requires 97 enabled bindings, got {len(managed)}")
    members: list[dict[str, Any]] = []
    placement_ids: set[str] = set()
    for ordinal, binding in enumerate(managed):
        exact(
            binding,
            ("bindingId", "mutationId", "patternId", "stageId", "triggerKind", "offsetMs", "receiverCollisionId", "enabled"),
            f"managed world binding[{ordinal}]",
        )
        mutation_id = stable_id(binding["mutationId"], f"managed binding[{ordinal}].mutationId")
        mutation = mutations.get(mutation_id)
        if mutation is None or mutation.get("targetState") != "DESPAWNED":
            raise PipelineError(f"managed binding mutation is missing or not DESPAWNED: {mutation_id}")
        group_id = stable_id(mutation.get("groupId"), f"mutation {mutation_id}.groupId")
        group = groups.get(group_id)
        if group is None:
            raise PipelineError(f"managed mutation group is missing: {group_id}")
        group_placements = group.get("memberPlacementIds")
        nav_regions = group.get("navigationRegionIds")
        if not isinstance(group_placements, list) or not group_placements:
            raise PipelineError(f"managed group has no placement closure: {group_id}")
        if not isinstance(nav_regions, list):
            raise PipelineError(f"managed group navigation closure is not a list: {group_id}")
        for placement_id in group_placements:
            stable_id(placement_id, f"group {group_id} placementId")
            if placement_id in placement_ids:
                raise PipelineError(f"109 placement is owned by multiple managed groups: {placement_id}")
            placement_ids.add(placement_id)
        members.append(
            {
                "memberId": _world_member_id(group_id),
                "bindingId": binding["bindingId"],
                "groupId": group_id,
                "mutationId": mutation_id,
                "offsetMs": binding["offsetMs"],
                "receiverCollisionId": binding["receiverCollisionId"],
                "enabled": binding["enabled"],
            }
        )
    if len(placement_ids) != 135:
        raise PipelineError(f"initial 109 migration requires 135 unique placements, got {len(placement_ids)}")
    result = {
        "schema": "lostark.valtan-world-event-set-authoring",
        "formatVersion": 1,
        "areaId": world["areaId"],
        "encounterId": world["encounterId"],
        "sets": [{"worldEventSetId": WORLD_SET_ID, "members": members}],
    }
    validate_world_event_sets(result, world, migration_fixture=True)
    return result


def validate_world_event_sets(
    document: dict[str, Any], world: dict[str, Any], *, migration_fixture: bool = False
) -> None:
    exact(document, ("schema", "formatVersion", "areaId", "encounterId", "sets"), "world-event-set root")
    if (
        document["schema"] != "lostark.valtan-world-event-set-authoring"
        or document["formatVersion"] != 1
        or document["areaId"] != "LV_LUT_HEARTRB_ED"
        or document["encounterId"] != "ENCOUNTER_VALTAN"
    ):
        raise PipelineError("world-event-set header mismatch")
    groups = unique_index(world["groups"], "groupId", "world groups")
    mutations = unique_index(world["mutations"], "mutationId", "world mutations")
    binding_by_id = unique_index(world["bindings"], "bindingId", "world bindings")
    set_ids: set[str] = set()
    member_ids: set[str] = set()
    binding_ids: set[str] = set()
    placement_ids: set[str] = set()
    for set_ordinal, event_set in enumerate(document["sets"]):
        exact(event_set, ("worldEventSetId", "members"), f"sets[{set_ordinal}]")
        set_id = stable_id(event_set["worldEventSetId"], f"sets[{set_ordinal}].worldEventSetId")
        if set_id in set_ids:
            raise PipelineError(f"duplicate worldEventSetId: {set_id}")
        set_ids.add(set_id)
        if not isinstance(event_set["members"], list) or not event_set["members"]:
            raise PipelineError(f"world event set has no members: {set_id}")
        for member_ordinal, member in enumerate(event_set["members"]):
            context = f"set {set_id} member[{member_ordinal}]"
            exact(
                member,
                ("memberId", "bindingId", "groupId", "mutationId", "offsetMs", "receiverCollisionId", "enabled"),
                context,
            )
            member_id = stable_id(member["memberId"], f"{context}.memberId")
            binding_id = stable_id(member["bindingId"], f"{context}.bindingId")
            group_id = stable_id(member["groupId"], f"{context}.groupId")
            mutation_id = stable_id(member["mutationId"], f"{context}.mutationId")
            if member_id in member_ids or binding_id in binding_ids:
                raise PipelineError(f"duplicate member/binding identity in world event sets: {member_id}/{binding_id}")
            member_ids.add(member_id)
            binding_ids.add(binding_id)
            integer(member["offsetMs"], f"{context}.offsetMs", 0, 600000)
            stable_id(member["receiverCollisionId"], f"{context}.receiverCollisionId", allow_empty=True)
            boolean(member["enabled"], f"{context}.enabled")
            mutation = mutations.get(mutation_id)
            if mutation is None or mutation.get("groupId") != group_id:
                raise PipelineError(f"mutation/group mismatch for {member_id}")
            group = groups.get(group_id)
            if group is None:
                raise PipelineError(f"missing group for {member_id}: {group_id}")
            placements = group.get("memberPlacementIds")
            nav_regions = group.get("navigationRegionIds")
            # A member with no placement has nothing to remove, so that stays
            # fatal. An empty navigation closure does not: the destruction
            # publisher authors none for a wall with a cliff behind it, because
            # breaking that wall must not turn the drop into walkable floor.
            if not isinstance(placements, list) or not placements or not isinstance(nav_regions, list):
                raise PipelineError(f"group closure is incomplete for {member_id}")
            for placement_id in placements:
                if placement_id in placement_ids:
                    raise PipelineError(f"world event sets reuse placement: {placement_id}")
                placement_ids.add(placement_id)
            source = binding_by_id.get(binding_id)
            if source is None:
                raise PipelineError(f"world event member binding does not exist: {binding_id}")
            expected_owner = WORLD_SET_OWNERS.get(set_id)
            source_owner = (
                source.get("patternId"),
                source.get("stageId"),
                source.get("triggerKind"),
            )
            accepted_owners = {expected_owner}
            legacy_owner = WORLD_SET_LEGACY_OWNERS.get(set_id)
            if legacy_owner is not None and not migration_fixture:
                accepted_owners.add(legacy_owner)
            if expected_owner is None or source_owner not in accepted_owners:
                raise PipelineError(
                    f"world event member owner drifted from its flat binding: {binding_id}"
                )
            if (
                source.get("mutationId") != mutation_id
                or source.get("offsetMs") != member["offsetMs"]
                or source.get("receiverCollisionId") != member["receiverCollisionId"]
                or source.get("enabled") != member["enabled"]
            ):
                raise PipelineError(f"world event member drifted from its flat binding: {binding_id}")
    expected_set_ids = (
        {WORLD_SET_ID} if migration_fixture else set(WORLD_SET_OWNERS)
    )
    if set_ids != expected_set_ids:
        raise PipelineError(
            "Valtan world event set closure drift: "
            f"expected={sorted(expected_set_ids)} actual={sorted(set_ids)}"
        )
    current_ids = {
        row["bindingId"]
        for set_id in expected_set_ids
        for row in world_set_source_bindings(world, set_id)
    }
    if binding_ids != current_ids:
        raise PipelineError(
            "world event set membership must equal the current managed flat binding IDs"
        )
    if migration_fixture and (len(binding_ids) != 97 or len(placement_ids) != 135):
        raise PipelineError("initial 109 migration fixture must close 97 bindings / 135 placements")


def _shape_from_flat(row: dict[str, Any], prefix: str = "hit") -> dict[str, Any]:
    kind = row[f"{prefix}Shape"]
    if kind == "NONE":
        return {"kind": "NONE"}
    if kind == "CIRCLE":
        return {"kind": kind, "outerRadiusM": row[f"{prefix}OuterRadius"]}
    if kind == "RING":
        return {
            "kind": kind,
            "innerRadiusM": row[f"{prefix}InnerRadius"],
            "outerRadiusM": row[f"{prefix}OuterRadius"],
        }
    if kind == "CONE":
        return {
            "kind": kind,
            "angleDegrees": row[f"{prefix}AngleDegrees"],
            "lengthM": row[f"{prefix}Length"],
        }
    if kind in ("BOX", "CROSS", "SIX_DIRECTIONS"):
        return {
            "kind": kind,
            "lengthM": row[f"{prefix}Length"],
            "halfWidthM": row[f"{prefix}HalfWidth"],
        }
    raise PipelineError(f"unsupported hit shape: {kind}")


def _flat_from_shape(shape: dict[str, Any]) -> dict[str, Any]:
    kind = shape["kind"]
    result = {
        "hitShape": kind,
        "hitOuterRadius": 0.0,
        "hitInnerRadius": 0.0,
        "hitAngleDegrees": 0.0,
        "hitLength": 0.0,
        "hitHalfWidth": 0.0,
    }
    if kind == "CIRCLE":
        result["hitOuterRadius"] = shape["outerRadiusM"]
    elif kind == "RING":
        result["hitInnerRadius"] = shape["innerRadiusM"]
        result["hitOuterRadius"] = shape["outerRadiusM"]
    elif kind == "CONE":
        result["hitAngleDegrees"] = shape["angleDegrees"]
        result["hitLength"] = shape["lengthM"]
    elif kind in ("BOX", "CROSS", "SIX_DIRECTIONS"):
        result["hitLength"] = shape["lengthM"]
        result["hitHalfWidth"] = shape["halfWidthM"]
    elif kind != "NONE":
        raise PipelineError(f"unsupported hit shape: {kind}")
    return result


def build_combat_authoring(product: dict[str, Any]) -> dict[str, Any]:
    exact(product, ("schema", "formatVersion", "encounterId", "objects"), "combat-object Product root")
    if (
        product["schema"] != "lostark.valtan-combat-objects"
        or product["formatVersion"] != 1
        or product["encounterId"] != "ENCOUNTER_VALTAN"
    ):
        raise PipelineError("combat-object Product header mismatch")
    objects: list[dict[str, Any]] = []
    for object_ordinal, source in enumerate(product["objects"]):
        archetype = stable_id(source["combatObjectArchetypeId"], f"combat object[{object_ordinal}] archetype")
        if source["originPolicy"] == "LOCKED_TARGET_PER_ALIVE_PLAYER":
            origin = {"kind": "RESOLVED_VOLLEY_POSITION"}
        elif source["originPolicy"] == "BOSS_POSITION":
            origin = {
                "kind": "BOSS_POSITION",
                "forwardOffsetM": source["offsetForwardM"],
                "rightOffsetM": source["offsetRightM"],
            }
        else:
            raise PipelineError(f"unsupported combat origin policy: {source['originPolicy']}")
        direction = {"kind": source["directionPolicy"]}
        if source["kind"] == "FIXED_AREA":
            movement = {"kind": "STATIC"}
        elif source["kind"] == "MISSILE":
            movement = {
                "kind": "LINEAR",
                "speedMps": source["speedMps"],
                "maximumDistanceM": source["maximumDistanceM"],
            }
        else:
            raise PipelineError(f"unsupported combat object kind: {source['kind']}")
        hits: list[dict[str, Any]] = []
        for hit_ordinal, hit in enumerate(source["hits"]):
            trigger = {"kind": hit["trigger"]}
            if hit["trigger"] == "TIMED":
                trigger["atMs"] = hit["atMs"]
            elif hit["trigger"] != "CONTACT":
                raise PipelineError(f"unsupported combat hit trigger: {hit['trigger']}")
            hit_id = "hit." + archetype[len("combatobject.") :] + f".{hit_ordinal + 1:02d}"
            hits.append(
                {
                    "hitId": hit_id,
                    "trigger": trigger,
                    "repeat": {"count": hit["repeatCount"], "intervalMs": hit["repeatIntervalMs"]},
                    "shape": _shape_from_flat(hit),
                    "serverDamageProfileId": hit["serverDamageProfileId"],
                    "pushRangeM": hit["pushRangeM"],
                    "pushMs": hit["pushMs"],
                    "knockdown": hit["knockdown"],
                    "downMs": hit["downMs"],
                }
            )
        objects.append(
            {
                "combatObjectArchetypeId": archetype,
                "kind": source["kind"],
                "spawn": {"origin": origin, "direction": direction},
                "movement": movement,
                "hits": hits,
            }
        )
    result = {
        "schema": "lostark.valtan-combat-object-authoring",
        "formatVersion": 1,
        "encounterId": product["encounterId"],
        "objects": objects,
    }
    validate_combat_authoring(result)
    return result


def _validate_shape(shape: dict[str, Any], context: str) -> None:
    kind = shape.get("kind")
    fields_by_kind = {
        "NONE": ("kind",),
        "CIRCLE": ("kind", "outerRadiusM"),
        "RING": ("kind", "innerRadiusM", "outerRadiusM"),
        "CONE": ("kind", "angleDegrees", "lengthM"),
        "BOX": ("kind", "lengthM", "halfWidthM"),
        "CROSS": ("kind", "lengthM", "halfWidthM"),
        "SIX_DIRECTIONS": ("kind", "lengthM", "halfWidthM"),
    }
    if kind not in fields_by_kind:
        raise PipelineError(f"{context} has unsupported shape kind: {kind}")
    exact(shape, fields_by_kind[kind], context)
    for field, value in shape.items():
        if field != "kind":
            number(value, f"{context}.{field}", 0.0, 100000.0)
    if kind == "RING" and shape["innerRadiusM"] >= shape["outerRadiusM"]:
        raise PipelineError(f"{context} ring radii are inverted")


def _pattern_stage_fields(stage: Mapping[str, Any], *, joined: bool) -> tuple[str, ...]:
    fields = (
        "stageId",
        "sequenceRole",
        "actionId",
        "stageKind",
        "durationMs",
        "defaultNextActionId",
        "hit",
        "motion",
        "events",
        "branches",
        "animation",
        "effectCues",
        "cameraInvocations",
    ) if joined else (
        "stageId",
        "actionId",
        "stageKind",
        "durationMs",
        "defaultNextActionId",
        "hit",
        "motion",
        "events",
        "branches",
    )
    for optional in ("partDamagePolicy", "counterProxy"):
        if optional in stage:
            fields += (optional,)
    return fields


def _has_closed_stage_flag(stage: Mapping[str, Any], flag_id: str) -> bool:
    events = stage.get("events")
    if not isinstance(events, list):
        return False
    return any(
        event.get("kind") == "SET_BOSS_FLAG"
        and event.get("trigger") == "ENTER"
        and event.get("flagId") == flag_id
        and event.get("enabled") is True
        for event in events
        if isinstance(event, dict)
    ) and any(
        event.get("kind") == "SET_BOSS_FLAG"
        and event.get("trigger") == "EXIT"
        and event.get("flagId") == flag_id
        and event.get("enabled") is False
        for event in events
        if isinstance(event, dict)
    )


def _validate_pattern_stage_extensions(
    stage: Mapping[str, Any], context: str
) -> None:
    motion = stage.get("motion")
    if motion is not None:
        if not isinstance(motion, dict):
            raise PipelineError(f"{context}.motion must be an object")
        if motion.get("kind") == "PORTAL_CROSS_ARENA":
            exact(motion, ("kind", "cornerIndex", "halfExtentsM"), f"{context}.motion")
            integer(motion["cornerIndex"], f"{context}.motion.cornerIndex", 0, 3)
            if not isinstance(motion["halfExtentsM"], list) or len(motion["halfExtentsM"]) != 2:
                raise PipelineError(f"{context}.motion.halfExtentsM requires X/Z")
            for value in motion["halfExtentsM"]:
                number(value, f"{context}.motion.halfExtentsM", 1, 100)
        elif motion.get("kind") == "PORTAL_TARGET_RUSH":
            exact(motion, ("kind",), f"{context}.motion")
        elif motion.get("kind") == "FORWARD":
            exact(motion, ("kind", "distance"), f"{context}.motion")
            number(motion["distance"], f"{context}.motion.distance", 0.000001, 1000)
        else:
            raise PipelineError(f"{context}.motion kind is unsupported")
    if any(branch.get("outcome") == "NAVIGATION_BLOCKED" for branch in stage.get("branches", [])):
        if stage.get("hit", {}).get("playerResponse") != "CAPTURE":
            raise PipelineError(f"{context} navigation-blocked outcome requires a capture rush")
    part_damage_policy = stage.get("partDamagePolicy", "NORMAL")
    if part_damage_policy not in ("NORMAL", "DESTROY_FIRST_ELIGIBLE"):
        raise PipelineError(f"{context}.partDamagePolicy is unsupported")
    if part_damage_policy == "DESTROY_FIRST_ELIGIBLE":
        has_destroyed_branch = any(
            branch.get("outcome") == "PART_DESTROYED"
            for branch in stage.get("branches", [])
            if isinstance(branch, dict)
        )
        if not has_destroyed_branch or not _has_closed_stage_flag(
            stage, "boss.flag.groggy"
        ):
            raise PipelineError(
                f"{context} instant part destruction requires a closed groggy "
                "window and PART_DESTROYED branch"
            )
    proxy = stage.get("counterProxy")
    if proxy is None:
        return
    if not isinstance(proxy, dict):
        raise PipelineError(f"{context}.counterProxy must be an object")
    exact(
        proxy,
        ("space", "forwardOffsetM", "rightOffsetM", "radiusM"),
        f"{context}.counterProxy",
    )
    if proxy["space"] != "BOSS_LOCAL":
        raise PipelineError(f"{context}.counterProxy space is unsupported")
    number(proxy["forwardOffsetM"], f"{context}.counterProxy.forwardOffsetM", -20, 20)
    number(proxy["rightOffsetM"], f"{context}.counterProxy.rightOffsetM", -20, 20)
    number(proxy["radiusM"], f"{context}.counterProxy.radiusM", 0.1, 20)
    has_counter_branch = any(
        branch.get("outcome") == "COUNTER_HIT"
        for branch in stage.get("branches", [])
        if isinstance(branch, dict)
    )
    if not has_counter_branch or not _has_closed_stage_flag(
        stage, "boss.flag.counterable"
    ):
        raise PipelineError(
            f"{context} counter proxy requires a closed counterable window and "
            "COUNTER_HIT branch"
        )


def _validate_pattern_target_aim(pattern: Mapping[str, Any], context: str) -> None:
    target = pattern.get("targetPolicy")
    aim = pattern.get("aimPolicy")
    locked_targets = {
        "LOCK_NEAREST_ON_START",
        "LOCK_RANDOM_ALIVE_ON_START",
        "LOCK_RANDOM_ALIVE_BEHIND_ON_START",
    }
    coherent = (
        (target == "NONE" and aim in ("NONE", "FACE_MOTION_ANCHOR"))
        or (target == "NEAREST_EACH_TICK" and aim == "TRACK_TARGET_EACH_TICK")
        or (
            target in locked_targets
            and aim in ("LOCK_FACING_ON_START", "TRACK_TARGET_EACH_TICK")
        )
    )
    if not coherent:
        raise PipelineError(
            f"{context} target/aim policy is unsupported or incoherent: "
            f"{target}/{aim}"
        )


def validate_combat_authoring(document: dict[str, Any]) -> None:
    exact(document, ("schema", "formatVersion", "encounterId", "objects"), "combat authoring root")
    if (
        document["schema"] != "lostark.valtan-combat-object-authoring"
        or document["formatVersion"] != 1
        or document["encounterId"] != "ENCOUNTER_VALTAN"
    ):
        raise PipelineError("combat authoring header mismatch")
    archetypes: set[str] = set()
    hit_ids: set[str] = set()
    for object_ordinal, obj in enumerate(document["objects"]):
        context = f"combat authoring object[{object_ordinal}]"
        object_fields = ("combatObjectArchetypeId", "kind", "spawn", "movement", "hits")
        if "lifetimeMs" in obj:
            object_fields += ("lifetimeMs",)
            integer(obj["lifetimeMs"], f"{context}.lifetimeMs", 1, 600000)
        exact(obj, object_fields, context)
        archetype = stable_id(obj["combatObjectArchetypeId"], f"{context}.combatObjectArchetypeId")
        if archetype in archetypes:
            raise PipelineError(f"duplicate combat object archetype: {archetype}")
        archetypes.add(archetype)
        if obj["kind"] not in ("FIXED_AREA", "MISSILE"):
            raise PipelineError(f"{context}.kind is unsupported")
        exact(obj["spawn"], ("origin", "direction"), f"{context}.spawn")
        origin = obj["spawn"]["origin"]
        if origin.get("kind") == "RESOLVED_VOLLEY_POSITION":
            exact(origin, ("kind",), f"{context}.spawn.origin")
        elif origin.get("kind") == "BOSS_POSITION":
            exact(origin, ("kind", "forwardOffsetM", "rightOffsetM"), f"{context}.spawn.origin")
            number(origin["forwardOffsetM"], f"{context}.forwardOffsetM", -1000.0, 1000.0)
            number(origin["rightOffsetM"], f"{context}.rightOffsetM", -1000.0, 1000.0)
        else:
            raise PipelineError(f"{context} origin kind is unsupported")
        direction = obj["spawn"]["direction"]
        exact(direction, ("kind",), f"{context}.spawn.direction")
        if direction["kind"] not in ("NONE", "PATTERN_FACING_AT_SPAWN"):
            raise PipelineError(f"{context} direction kind is unsupported")
        movement = obj["movement"]
        if movement.get("kind") == "STATIC":
            exact(movement, ("kind",), f"{context}.movement")
        elif movement.get("kind") == "LINEAR":
            exact(movement, ("kind", "speedMps", "maximumDistanceM"), f"{context}.movement")
            number(movement["speedMps"], f"{context}.speedMps", 0.000001, 10000.0)
            number(movement["maximumDistanceM"], f"{context}.maximumDistanceM", 0.000001, 10000.0)
        else:
            raise PipelineError(f"{context} movement kind is unsupported")
        if not isinstance(obj["hits"], list) or not obj["hits"]:
            raise PipelineError(f"{context} must own at least one hit")
        for hit_ordinal, hit in enumerate(obj["hits"]):
            hit_context = f"{context}.hits[{hit_ordinal}]"
            exact(
                hit,
                ("hitId", "trigger", "repeat", "shape", "serverDamageProfileId", "pushRangeM", "pushMs", "knockdown", "downMs"),
                hit_context,
            )
            hit_id = stable_id(hit["hitId"], f"{hit_context}.hitId")
            if hit_id in hit_ids:
                raise PipelineError(f"duplicate combat hitId: {hit_id}")
            hit_ids.add(hit_id)
            trigger = hit["trigger"]
            if trigger.get("kind") == "TIMED":
                exact(trigger, ("kind", "atMs"), f"{hit_context}.trigger")
                integer(trigger["atMs"], f"{hit_context}.trigger.atMs", 0, 600000)
            elif trigger.get("kind") == "CONTACT":
                exact(trigger, ("kind",), f"{hit_context}.trigger")
            else:
                raise PipelineError(f"{hit_context} trigger kind is unsupported")
            exact(hit["repeat"], ("count", "intervalMs"), f"{hit_context}.repeat")
            count = integer(hit["repeat"]["count"], f"{hit_context}.repeat.count", 1, 64)
            interval = integer(hit["repeat"]["intervalMs"], f"{hit_context}.repeat.intervalMs", 0, 600000)
            if count == 1 and interval != 0:
                raise PipelineError(f"{hit_context} single hit must have intervalMs 0")
            if count > 1 and interval == 0:
                raise PipelineError(f"{hit_context} repeated hit requires a positive interval")
            if "lifetimeMs" in obj and trigger.get("kind") == "TIMED" and (
                trigger["atMs"] + (count - 1) * interval >= obj["lifetimeMs"]
            ):
                raise PipelineError(f"{hit_context} timed hit escapes object lifetime")
            _validate_shape(hit["shape"], f"{hit_context}.shape")
            stable_id(hit["serverDamageProfileId"], f"{hit_context}.serverDamageProfileId")
            number(hit["pushRangeM"], f"{hit_context}.pushRangeM", 0.0, 100000.0)
            integer(hit["pushMs"], f"{hit_context}.pushMs", 0, 600000)
            boolean(hit["knockdown"], f"{hit_context}.knockdown")
            integer(hit["downMs"], f"{hit_context}.downMs", 0, 600000)


def build_legacy_manifest(root: Path, docs: dict[str, Any]) -> dict[str, Any]:
    master = docs[MASTER_REL]
    managed = {row["patternId"] for row in master["patterns"]}
    encounter = docs[ENCOUNTER_REL]
    managed.update(
        row["patternId"]
        for row in encounter["patterns"]
        if row.get("selectionMode") == AUDITION_ONLY
    )
    bindings = docs[BINDINGS_REL]
    cues = docs[CUES_REL]
    rotations = docs[ROTATIONS_REL]
    combat = docs[COMBAT_PRODUCT_REL]
    binding_ordinals = {row["actionId"]: index for index, row in enumerate(bindings["bindings"])}
    cue_ordinals = {row["bindingId"]: index for index, row in enumerate(cues["cues"])}
    pattern_entries: list[dict[str, Any]] = []
    for encounter_ordinal, pattern in enumerate(encounter["patterns"]):
        pattern_id = pattern["patternId"]
        if pattern_id in managed:
            continue
        action_ids = [stage["actionId"] for stage in pattern["stages"]]
        pattern_bindings: list[dict[str, Any]] = []
        for action_id in action_ids:
            matches = [row for row in bindings["bindings"] if row["actionId"] == action_id]
            if len(matches) != 1:
                raise PipelineError(f"legacy action does not have exact animation binding: {action_id}")
            row = matches[0]
            pattern_bindings.append(
                {
                    "bindingOrdinal": binding_ordinals[action_id],
                    "bindingRowSha256": canonical_hash(row),
                    "runtimeBinding": copy.deepcopy(row),
                }
            )
        pattern_cues = []
        for cue in cues["cues"]:
            if cue["patternId"] == pattern_id:
                pattern_cues.append(
                    {
                        "cueOrdinal": cue_ordinals[cue["bindingId"]],
                        "cueRowSha256": canonical_hash(cue),
                        "runtimeCue": copy.deepcopy(cue),
                    }
                )
        pattern_entries.append(
            {
                "patternId": pattern_id,
                "encounterOrdinal": encounter_ordinal,
                "encounterRowSha256": canonical_hash(pattern),
                "runtimePattern": copy.deepcopy(pattern),
                "animationBindings": pattern_bindings,
                "effectCues": pattern_cues,
            }
        )
    shared_rotations = []
    for ordinal, rotation in enumerate(rotations["rotations"]):
        rotation_pattern_ids = rotation.get("patternIds")
        if rotation_pattern_ids is None:
            rotation_pattern_ids = [
                row["patternId"] for row in rotation.get("candidates", [])
            ]
        shared_rotations.append(
            {
                "rotationId": rotation["rotationId"],
                "rotationOrdinal": ordinal,
                "rotationRowSha256": canonical_hash(rotation),
                "selectionMode": rotation["selectionMode"],
                "fromHealthBar": rotation["fromHealthBar"],
                "toHealthBar": rotation["toHealthBar"],
                "patternRefs": [
                    {
                        "patternId": pattern_id,
                        "ownership": "MANAGED" if pattern_id in managed else "LEGACY",
                    }
                    for pattern_id in rotation_pattern_ids
                ],
            }
        )
    legacy_owners = []
    for ordinal, obj in enumerate(combat["objects"]):
        if obj["ownerPatternId"] not in managed:
            legacy_owners.append(
                {
                    "combatObjectArchetypeId": obj["combatObjectArchetypeId"],
                    "ownerPatternId": obj["ownerPatternId"],
                    "ownerStageActionId": obj["ownerStageActionId"],
                    "lifeMs": obj["lifeMs"],
                    "originalOrdinal": ordinal,
                }
            )
    receipt_paths = (ENCOUNTER_REL, ROTATIONS_REL, COMBAT_PRODUCT_REL, BINDINGS_REL, CUES_REL)
    result = {
        "schema": "lostark.valtan-legacy-compatibility",
        "formatVersion": 1,
        "encounterId": "ENCOUNTER_VALTAN",
        "patternEntries": pattern_entries,
        "sharedRotationRows": shared_rotations,
        "legacyCombatObjectOwners": legacy_owners,
        "initialMigrationReceipt": {
            "sources": [
                {"path": relative, "sha256": sha256_file(repo_path(root, relative))}
                for relative in sorted(receipt_paths)
            ]
        },
    }
    validate_legacy_manifest(result, managed)
    return result


def validate_legacy_manifest(document: dict[str, Any], managed: set[str]) -> None:
    exact(
        document,
        ("schema", "formatVersion", "encounterId", "patternEntries", "sharedRotationRows", "legacyCombatObjectOwners", "initialMigrationReceipt"),
        "legacy compatibility root",
    )
    if (
        document["schema"] != "lostark.valtan-legacy-compatibility"
        or document["formatVersion"] != 1
        or document["encounterId"] != "ENCOUNTER_VALTAN"
    ):
        raise PipelineError("legacy compatibility header mismatch")
    legacy_ids: set[str] = set()
    encounter_ordinals: set[int] = set()
    binding_ordinals: set[int] = set()
    cue_ordinals: set[int] = set()
    for entry_ordinal, entry in enumerate(document["patternEntries"]):
        context = f"legacy patternEntries[{entry_ordinal}]"
        exact(
            entry,
            ("patternId", "encounterOrdinal", "encounterRowSha256", "runtimePattern", "animationBindings", "effectCues"),
            context,
        )
        pattern_id = stable_id(entry["patternId"], f"{context}.patternId")
        if pattern_id in managed or pattern_id in legacy_ids:
            raise PipelineError(f"legacy pattern ownership is invalid: {pattern_id}")
        legacy_ids.add(pattern_id)
        ordinal = integer(entry["encounterOrdinal"], f"{context}.encounterOrdinal", 0, 4096)
        if ordinal in encounter_ordinals:
            raise PipelineError(f"duplicate legacy encounter ordinal: {ordinal}")
        encounter_ordinals.add(ordinal)
        if entry["runtimePattern"].get("patternId") != pattern_id or canonical_hash(entry["runtimePattern"]) != entry["encounterRowSha256"]:
            raise PipelineError(f"sealed legacy runtime pattern hash mismatch: {pattern_id}")
        action_ids = {stage["actionId"] for stage in entry["runtimePattern"]["stages"]}
        seen_actions: set[str] = set()
        for binding in entry["animationBindings"]:
            exact(binding, ("bindingOrdinal", "bindingRowSha256", "runtimeBinding"), f"{context}.animationBinding")
            binding_ordinal = integer(binding["bindingOrdinal"], f"{context}.bindingOrdinal", 0, 16384)
            if binding_ordinal in binding_ordinals:
                raise PipelineError(f"duplicate sealed binding ordinal: {binding_ordinal}")
            binding_ordinals.add(binding_ordinal)
            action_id = binding["runtimeBinding"].get("actionId")
            if action_id not in action_ids or action_id in seen_actions:
                raise PipelineError(f"legacy binding closure mismatch: {pattern_id}/{action_id}")
            seen_actions.add(action_id)
            if canonical_hash(binding["runtimeBinding"]) != binding["bindingRowSha256"]:
                raise PipelineError(f"sealed legacy binding hash mismatch: {action_id}")
        if seen_actions != action_ids:
            raise PipelineError(f"legacy binding closure is partial: {pattern_id}")
        for cue in entry["effectCues"]:
            exact(cue, ("cueOrdinal", "cueRowSha256", "runtimeCue"), f"{context}.effectCue")
            cue_ordinal = integer(cue["cueOrdinal"], f"{context}.cueOrdinal", 0, 16384)
            if cue_ordinal in cue_ordinals:
                raise PipelineError(f"duplicate sealed cue ordinal: {cue_ordinal}")
            cue_ordinals.add(cue_ordinal)
            if cue["runtimeCue"].get("patternId") != pattern_id or canonical_hash(cue["runtimeCue"]) != cue["cueRowSha256"]:
                raise PipelineError(f"sealed legacy cue mismatch: {pattern_id}")
    rotation_ids: set[str] = set()
    rotation_ordinals: set[int] = set()
    for row_ordinal, row in enumerate(document["sharedRotationRows"]):
        context = f"legacy sharedRotationRows[{row_ordinal}]"
        exact(
            row,
            ("rotationId", "rotationOrdinal", "rotationRowSha256", "selectionMode", "fromHealthBar", "toHealthBar", "patternRefs"),
            context,
        )
        rotation_id = stable_id(row["rotationId"], f"{context}.rotationId")
        ordinal = integer(row["rotationOrdinal"], f"{context}.rotationOrdinal", 0, 4096)
        if rotation_id in rotation_ids or ordinal in rotation_ordinals:
            raise PipelineError(f"duplicate shared rotation identity: {rotation_id}/{ordinal}")
        rotation_ids.add(rotation_id)
        rotation_ordinals.add(ordinal)
        runtime_rotation = {
            "rotationId": rotation_id,
            "selectionMode": row["selectionMode"],
            "fromHealthBar": row["fromHealthBar"],
            "toHealthBar": row["toHealthBar"],
            "patternIds": [ref["patternId"] for ref in row["patternRefs"]],
        }
        if (
            rotation_id not in MANAGED_ROTATION_IDS
            and canonical_hash(runtime_rotation) != row["rotationRowSha256"]
        ):
            raise PipelineError(f"sealed shared rotation hash mismatch: {rotation_id}")
        for ref in row["patternRefs"]:
            exact(ref, ("patternId", "ownership"), f"{context}.patternRef")
            pattern_id = stable_id(ref["patternId"], f"{context}.patternRef.patternId")
            expected = "MANAGED" if pattern_id in managed else "LEGACY"
            if ref["ownership"] != expected or (expected == "LEGACY" and pattern_id not in legacy_ids):
                raise PipelineError(f"shared rotation ownership mismatch: {rotation_id}/{pattern_id}")
    owner_archetypes: set[str] = set()
    for owner_ordinal, owner in enumerate(document["legacyCombatObjectOwners"]):
        context = f"legacy combat owner[{owner_ordinal}]"
        exact(
            owner,
            ("combatObjectArchetypeId", "ownerPatternId", "ownerStageActionId", "lifeMs", "originalOrdinal"),
            context,
        )
        archetype = stable_id(owner["combatObjectArchetypeId"], f"{context}.combatObjectArchetypeId")
        if archetype in owner_archetypes or owner["ownerPatternId"] not in legacy_ids:
            raise PipelineError(f"legacy combat owner is invalid: {archetype}")
        owner_archetypes.add(archetype)
        stable_id(owner["ownerStageActionId"], f"{context}.ownerStageActionId")
        integer(owner["lifeMs"], f"{context}.lifeMs", 1, 600000)
        integer(owner["originalOrdinal"], f"{context}.originalOrdinal", 0, 4096)
    exact(document["initialMigrationReceipt"], ("sources",), "initialMigrationReceipt")
    receipt_paths: list[str] = []
    for source in document["initialMigrationReceipt"]["sources"]:
        exact(source, ("path", "sha256"), "initialMigrationReceipt source")
        if not isinstance(source["path"], str) or not re.fullmatch(r"[0-9a-f]{64}", source["sha256"]):
            raise PipelineError("initialMigrationReceipt source is malformed")
        receipt_paths.append(source["path"])
    if receipt_paths != sorted(receipt_paths) or len(receipt_paths) != len(set(receipt_paths)):
        raise PipelineError("initialMigrationReceipt sources must be unique and sorted")


def validate_legacy_products(
    document: dict[str, Any],
    docs: dict[str, Any],
    managed: set[str],
) -> None:
    """Prove the sealed unmanaged rows still match the current transitional Products."""

    encounter_rows = docs[ENCOUNTER_REL]["patterns"]
    product_managed = set(managed)
    product_audition_ids = {
        stable_id(row.get("patternId"), "Product AUDITION_ONLY patternId")
        for row in encounter_rows
        if row.get("selectionMode") == AUDITION_ONLY
    }
    stale_audition_ids = product_audition_ids - product_managed
    if stale_audition_ids:
        raise PipelineError(
            "Product AUDITION_ONLY ownership drift: "
            + ", ".join(sorted(stale_audition_ids))
        )
    validate_legacy_manifest(document, product_managed)
    binding_rows = docs[BINDINGS_REL]["bindings"]
    cue_rows = docs[CUES_REL]["cues"]
    cue_rows_by_id: dict[str, dict[str, Any]] = {}
    for cue_row in cue_rows:
        binding_id = stable_id(
            cue_row.get("bindingId"), "Product effect cue bindingId"
        )
        if binding_id in cue_rows_by_id:
            raise PipelineError(
                f"duplicate Product effect cue bindingId: {binding_id}"
            )
        cue_rows_by_id[binding_id] = cue_row
    rotation_rows = docs[ROTATIONS_REL]["rotations"]
    combat_rows = docs[COMBAT_PRODUCT_REL]["objects"]
    sealed_legacy_ids = {row["patternId"] for row in document["patternEntries"]}
    if not ARCHIVED_LEGACY_PATTERN_IDS.issubset(sealed_legacy_ids):
        raise PipelineError("archived legacy terrain mechanics left the sealed receipt")
    active_sealed_legacy_ids = sealed_legacy_ids - ARCHIVED_LEGACY_PATTERN_IDS
    actual_legacy_ids = {
        row["patternId"]
        for row in encounter_rows
        if row["patternId"] not in product_managed
    }
    if active_sealed_legacy_ids != actual_legacy_ids:
        raise PipelineError("legacy compatibility pattern ownership drift")
    archived_ordinals = sorted(
        entry["encounterOrdinal"]
        for entry in document["patternEntries"]
        if entry["patternId"] in ARCHIVED_LEGACY_PATTERN_IDS
    )
    for entry in document["patternEntries"]:
        if entry["patternId"] in ARCHIVED_LEGACY_PATTERN_IDS:
            continue
        ordinal = entry["encounterOrdinal"] - sum(
            archived < entry["encounterOrdinal"]
            for archived in archived_ordinals
        )
        if ordinal >= len(encounter_rows) or encounter_rows[ordinal] != entry["runtimePattern"]:
            raise PipelineError(f"sealed legacy encounter row drift: {entry['patternId']}")
        for binding in entry["animationBindings"]:
            binding_ordinal = binding["bindingOrdinal"]
            if (
                binding_ordinal >= len(binding_rows)
                or binding_rows[binding_ordinal] != binding["runtimeBinding"]
            ):
                raise PipelineError(
                    f"sealed legacy animation row drift: "
                    f"{binding['runtimeBinding'].get('actionId')}"
                )
        for cue in entry["effectCues"]:
            runtime_cue = cue["runtimeCue"]
            binding_id = stable_id(
                runtime_cue.get("bindingId"), "sealed legacy effect cue bindingId"
            )
            if cue_rows_by_id.get(binding_id) != runtime_cue:
                raise PipelineError(
                    f"sealed legacy effect cue drift: {binding_id}"
                )
    if len(document["sharedRotationRows"]) != len(rotation_rows):
        raise PipelineError("sealed rotation row count drift")
    if GAMEPLAY_AUTHORING_REL in docs:
        gameplay_decision = docs[GAMEPLAY_AUTHORING_REL]["decisionModel"]
    else:
        migration_master = docs[MASTER_REL]
        weights = {
            row["patternId"]: row["selectionWeight"]
            for row in migration_master["patterns"]
        }
        migration_sets = []
        migration_windows = []
        for range_row in migration_master["normalSelection"]["ranges"]:
            suffix = range_row["rotationId"].removeprefix("rotation.valtan.")
            set_id = "selectionset.valtan." + suffix
            product_rotation = next(
                (
                    row
                    for row in rotation_rows
                    if row.get("rotationId") == range_row["rotationId"]
                ),
                {},
            )
            product_enabled = {
                row.get("patternId"): row.get("enabled")
                for row in product_rotation.get("candidates", [])
            }
            migration_candidates = [
                {
                    "patternId": pattern_id,
                    "weight": weights[pattern_id],
                    "enabled": product_enabled.get(pattern_id, True),
                }
                for pattern_id in migration_master["normalSelection"]["patternIds"]
            ]
            migration_sets.append(
                {
                    "selectionSetId": set_id,
                    "mode": "WEIGHTED_POOL",
                    "candidates": copy.deepcopy(migration_candidates),
                }
            )
            migration_windows.append(
                {
                    "windowId": "window.valtan.phase1." + suffix,
                    "gameplayPhase": 1,
                    "maximumHealthBarInclusive": range_row["fromHealthBar"],
                    "minimumHealthBarExclusive": range_row["toHealthBar"],
                    "selectionSetId": set_id,
                    "compatibilityRotationId": range_row["rotationId"],
                }
            )
        gameplay_decision = {
            "selectionSets": migration_sets,
            "selectionWindows": migration_windows,
        }
    windows_by_rotation = {
        row["compatibilityRotationId"]: row
        for row in gameplay_decision["selectionWindows"]
    }
    sets_by_id = {
        row["selectionSetId"]: row
        for row in gameplay_decision["selectionSets"]
    }
    for sealed in document["sharedRotationRows"]:
        ordinal = sealed["rotationOrdinal"]
        if ordinal >= len(rotation_rows):
            raise PipelineError(f"sealed rotation ordinal is missing: {ordinal}")
        actual = rotation_rows[ordinal]
        rotation_id = sealed["rotationId"]
        if actual.get("rotationId") != rotation_id:
            raise PipelineError(f"sealed shared rotation row drift: {sealed['rotationId']}")
        if rotation_id in MANAGED_ROTATION_IDS:
            window = windows_by_rotation.get(rotation_id)
            selection_set = (
                sets_by_id.get(window["selectionSetId"])
                if window is not None
                else None
            )
            expected = None if selection_set is None else {
                "rotationId": rotation_id,
                "selectionMode": selection_set["mode"],
                "fromHealthBar": window["maximumHealthBarInclusive"],
                "toHealthBar": window["minimumHealthBarExclusive"],
                "windowId": window["windowId"],
                "gameplayPhase": window["gameplayPhase"],
                "selectionSetId": selection_set["selectionSetId"],
                "candidates": copy.deepcopy(selection_set["candidates"]),
            }
            if actual != expected:
                raise PipelineError(
                    f"managed rotation split projection drift: {rotation_id}"
                )
        elif canonical_hash(actual) != sealed["rotationRowSha256"]:
            raise PipelineError(f"sealed shared rotation row drift: {rotation_id}")
    for owner in document["legacyCombatObjectOwners"]:
        ordinal = owner["originalOrdinal"]
        if ordinal >= len(combat_rows):
            raise PipelineError(f"sealed legacy combat owner ordinal is missing: {ordinal}")
        actual = combat_rows[ordinal]
        for field in (
            "combatObjectArchetypeId",
            "ownerPatternId",
            "ownerStageActionId",
            "lifeMs",
        ):
            if actual.get(field) != owner[field]:
                raise PipelineError(
                    f"sealed legacy combat owner drift: "
                    f"{owner['combatObjectArchetypeId']}/{field}"
                )


def _event_id(pattern_id: str, stage_id: str, kind: str, ordinal: int) -> str:
    stem = pattern_id.lower().replace("_", "-")
    stage = stage_id.lower().replace("_", "-")
    token = kind.lower().replace("_", "-")
    return f"event.{stem}.{stage}.{token}.{ordinal + 1:02d}"


def _migrate_hit(stage: dict[str, Any]) -> dict[str, Any]:
    shape = _shape_from_flat(stage)
    if shape["kind"] == "NONE":
        return {"shape": shape}
    if stage["hitOffsetsMs"]:
        schedule = {"kind": "EXPLICIT_OFFSETS", "offsetsMs": copy.deepcopy(stage["hitOffsetsMs"])}
    else:
        schedule = {
            "kind": "INTERVAL",
            "count": stage["hitCount"],
            "firstOffsetMs": stage["hitDelayMs"],
            "intervalMs": stage["hitIntervalMs"],
        }
    return {
        "shape": shape,
        "schedule": schedule,
        "serverDamageProfileId": stage["serverDamageProfileId"],
        "pushRangeM": stage["pushRangeM"],
        "pushMs": stage["pushMs"],
        "knockdown": stage["knockdown"],
        "downMs": stage["downMs"],
    }


def _migrate_action(
    pattern_id: str, stage_id: str, action: dict[str, Any], ordinal: int
) -> dict[str, Any]:
    if action["kind"] == "SET_BOSS_FLAG":
        return {
            "eventId": _event_id(pattern_id, stage_id, action["kind"], ordinal),
            "trigger": action["trigger"],
            "kind": "SET_BOSS_FLAG",
            "flagId": action["targetId"],
            "enabled": action["value"] != 0,
        }
    if action["kind"] == "SPAWN_COMBAT_OBJECT":
        event_id = (
            "event.valtan.high-jump.airborne.spawn-target-axe"
            if action["targetId"] == "combatobject.valtan.high-jump.target-axe"
            else _event_id(pattern_id, stage_id, action["kind"], ordinal)
        )
        return {
            "eventId": event_id,
            "trigger": action["trigger"],
            "kind": "SPAWN_COMBAT_OBJECT_VOLLEY",
            "combatObjectArchetypeId": action["targetId"],
            "volleyPolicy": "PER_ALIVE_PLAYER",
            "countPerResolvedTarget": action["value"],
            "layout": {"kind": "TARGET_CENTER"},
            "spawnSchedule": {
                "kind": "INTERVAL",
                "count": 3,
                "firstOffsetMs": 0,
                "intervalMs": 1333,
            },
            "arenaRandom": {
                "kind": "RANDOM_NAVIGABLE_CIRCLE",
                "anchor": "BOSS_SPAWN_POSITION",
                "count": 4,
                "radiusM": 14.0,
                "heightToleranceM": 1.0,
            },
            "allowOverlap": False,
            "maximumTotalObjects": 36,
        }
    raise PipelineError(f"v1 action has no v2 migration: {action['kind']}")


def _migrate_effect_cue(
    effect_ref: dict[str, Any],
    cue_by_id: dict[str, dict[str, Any]],
    occurrences_by_id: Mapping[str, dict[str, Any]],
    context: str,
    live_split_cue_ids: set[str] | None = None,
) -> dict[str, Any] | None:
    cue_id = effect_ref["refId"]
    cue = cue_by_id.get(cue_id)
    if cue is None:
        if live_split_cue_ids is not None and cue_id not in live_split_cue_ids:
            return None
        raise PipelineError(f"managed cue is missing from Product: {cue_id}")
    projection = effect_ref["cueProjection"]
    # The frozen v1 fixture owns its legacy clip projection.  A current Product
    # cue may have intentionally moved to a stage-clock independent Effect, but
    # that must not rewrite the historical migration input into the new timing
    # model.  Rehydrate the v1 clip cue from its sealed projection in that case.
    cue_uses_stage_clock = (
        cue.get("timingBasis") == CUE_TIMING_BASIS_STAGE_CLOCK
    )
    if not cue_uses_stage_clock and (
        cue["clipOccurrenceId"] != projection["clipOccurrenceId"]
        or cue["sourceStartMs"] != projection["sourceStartMs"]
        or cue["sourceEndMs"] != projection["sourceEndMs"]
    ):
        raise PipelineError(f"managed cue owner/source window drift: {cue_id}")
    clip_occurrence_id = projection["clipOccurrenceId"]
    source_start_ms = projection["sourceStartMs"]
    source_end_ms = projection["sourceEndMs"]
    occurrence = occurrences_by_id.get(clip_occurrence_id)
    if occurrence is None:
        raise PipelineError(
            f"managed cue does not resolve its saved animation occurrence: {cue_id}"
        )
    scale_policy = cue.get("scalePolicy")
    if scale_policy is None:
        scale_kind = MANAGED_CUE_SCALE_POLICIES.get(cue_id)
        if scale_kind is None:
            raise PipelineError(f"managed cue has no scale-policy migration: {cue_id}")
        scale_policy = {"kind": scale_kind}
        if scale_kind in WORLD_SCALE_POLICY_KINDS:
            scale_policy["worldScale"] = [1.5, 1.5, 1.5]
    validate_cue_scale_policy(scale_policy, context)
    migrated = {
        "cueId": cue_id,
        "occurrenceId": cue["occurrenceId"],
        "effectAssetId": cue["effectAssetId"],
        "clipOccurrenceId": clip_occurrence_id,
        "sourceStartMs": source_start_ms,
        "sourceEndMs": source_end_ms,
        "anchorSlotId": cue["anchorSlotId"],
        "followPolicy": cue["followPolicy"],
        "stopPolicy": cue["stopPolicy"],
        "repeatPolicy": cue["repeatPolicy"],
        "localTransform": copy.deepcopy(cue["localTransform"]),
        "scalePolicy": copy.deepcopy(scale_policy),
        "mappingBasis": occurrence["mappingBasis"],
    }
    validate_cue_animation_join(migrated, occurrences_by_id, context)
    return migrated


def migrate_v1_to_v2(root: Path, docs: dict[str, Any]) -> dict[str, Any]:
    master = docs[MASTER_REL]
    exact(
        master,
        ("schema", "formatVersion", "bossArchetypeId", "encounterId", "scope", "previewPaths", "retiredPatternIds", "normalSelection", "counterReactionLayers", "independentEffects", "patterns"),
        "v1 master root",
    )
    if master["schema"] != "lostark.valtan-pattern-master" or master["formatVersion"] != 1:
        raise PipelineError("migration input must be Valtan pattern master v1")
    managed = {pattern["patternId"] for pattern in master["patterns"]}
    if managed != set(MANAGED_PATTERN_IDS):
        raise PipelineError("initial v1 migration fixture does not contain the seven managed patterns")
    # The frozen v1 migration fixture predates split manualAuditions. Current
    # Product audition rows are later managed additions, not legacy seal rows.
    migration_product_managed = (
        managed
        | CURRENT_MIGRATION_MANAGED_PATTERN_IDS
        | {
            row["patternId"]
            for row in docs[ENCOUNTER_REL]["patterns"]
            if row.get("selectionMode") == AUDITION_ONLY
        }
    )
    validate_legacy_products(
        docs[LEGACY_REL], docs, migration_product_managed
    )
    cue_by_id = unique_index(docs[CUES_REL]["cues"], "bindingId", "effect cues")
    live_split_cue_ids: set[str] | None = None
    live_presentation_path = root / PRESENTATION_AUTHORING_REL
    if live_presentation_path.is_file():
        live_presentation = read_json(live_presentation_path)
        live_split_cue_ids = {
            stable_id(cue.get("cueId"), "live split cueId")
            for pattern in live_presentation.get("patterns", [])
            for stage in pattern.get("stages", [])
            for cue in stage.get("effectCues", [])
        }
    camera_by_id = unique_index(docs[CAMERA_REL]["cues"], "cueId", "camera cues")
    product_rotations_by_id = {
        row["rotationId"]: row for row in docs[ROTATIONS_REL]["rotations"]
    }
    selection_sets = []
    selection_windows = []
    for range_row in master["normalSelection"]["ranges"]:
        suffix = range_row["rotationId"][len("rotation.valtan.") :]
        set_id = "selectionset.valtan." + suffix
        product_enabled = {
            row["patternId"]: row["enabled"]
            for row in product_rotations_by_id[range_row["rotationId"]]["candidates"]
        }
        set_candidates = []
        for pattern_id in master["normalSelection"]["patternIds"]:
            pattern = next(
                row for row in master["patterns"]
                if row["patternId"] == pattern_id
            )
            set_candidates.append(
                {
                    "patternId": pattern_id,
                    "weight": pattern["selectionWeight"],
                    "enabled": product_enabled[pattern_id],
                }
            )
        selection_sets.append(
            {"selectionSetId": set_id, "mode": "WEIGHTED_POOL", "candidates": copy.deepcopy(set_candidates)}
        )
        selection_windows.append(
            {
                "windowId": "window.valtan.phase1." + suffix,
                "gameplayPhase": 1,
                "maximumHealthBarInclusive": range_row["fromHealthBar"],
                "minimumHealthBarExclusive": range_row["toHealthBar"],
                "selectionSetId": set_id,
                "compatibilityRotationId": range_row["rotationId"],
            }
        )
    mechanics = []
    for pattern in master["patterns"]:
        if pattern["selectionMode"] == "HEALTH_BAR":
            mechanics.append(
                {
                    "mechanicId": "mechanic." + pattern["patternId"].lower().replace("_", "-"),
                    "patternId": pattern["patternId"],
                    "trigger": {"kind": "HEALTH_BAR_CROSSING", "healthBar": pattern["triggerHealthBar"]},
                    "triggerOrder": pattern["triggerOrder"],
                    "oncePerEncounter": True,
                    "failurePolicy": "ABORT_ENCOUNTER_REQUIRE_RESET",
                }
            )
    migrated_patterns = []
    cue_owners: dict[str, tuple[str, str]] = {}
    spawn_event_by_archetype: dict[str, str] = {}
    for pattern in master["patterns"]:
        pattern_id = pattern["patternId"]
        migrated_stages = []
        for stage_ordinal, stage in enumerate(pattern["stages"]):
            events = [
                _migrate_action(pattern_id, stage["stageId"], action, action_ordinal)
                for action_ordinal, action in enumerate(stage["actions"])
            ]
            for event in events:
                if event["kind"] == "SPAWN_COMBAT_OBJECT_VOLLEY":
                    spawn_event_by_archetype[event["combatObjectArchetypeId"]] = event["eventId"]
            if (
                pattern_id == WORLD_PATTERN_ID
                and stage["stageId"] == WORLD_STAGE_ID
                and pattern["worldEventTriggerRefs"]
            ):
                events.append(
                    {
                        "eventId": "event.valtan.arena-break-109.impact.set-gameplay-phase-2",
                        "trigger": "ENTER",
                        "kind": "SET_GAMEPLAY_PHASE",
                        "gameplayPhase": 2,
                    }
                )
                events.append(
                    {
                        "eventId": WORLD_EVENT_ID,
                        "trigger": "ENTER",
                        "kind": "TRIGGER_WORLD_EVENT_SET",
                        "worldEventSetId": WORLD_SET_ID,
                    }
                )
            effect_cues = []
            occurrences_by_id = unique_index(
                stage["animation"]["occurrences"],
                "clipOccurrenceId",
                f"{pattern_id}/{stage['stageId']} animation occurrences",
            )
            for effect_ref in stage["effectRefs"]:
                if effect_ref["refType"] == "CUE_BINDING":
                    migrated_cue = _migrate_effect_cue(
                        effect_ref,
                        cue_by_id,
                        occurrences_by_id,
                        f"{pattern_id}/{stage['stageId']} managed cue",
                        live_split_cue_ids,
                    )
                    if migrated_cue is None:
                        continue
                    effect_cues.append(migrated_cue)
                    cue_owners[migrated_cue["cueId"]] = (pattern_id, stage["stageId"])
            camera_invocations = []
            for camera_id in pattern["cameraCueIds"]:
                camera = camera_by_id.get(camera_id)
                if camera is not None and camera.get("stageId") == stage["stageId"]:
                    camera_invocations.append(
                        {
                            "cameraInvocationId": camera_id + ".invocation",
                            "cameraCueId": camera_id,
                            "trigger": "ENTER",
                            "startOffsetMs": 0,
                            "durationPolicy": "EXPLICIT",
                            "durationMs": camera["durationMs"],
                        }
                    )
            timeout_branches = [row for row in stage["branches"] if row["outcome"] == "TIMEOUT"]
            if timeout_branches:
                default_next = timeout_branches[0]["nextActionId"]
            elif stage_ordinal + 1 < len(pattern["stages"]):
                default_next = pattern["stages"][stage_ordinal + 1]["actionId"]
            else:
                default_next = None
            migrated_stages.append(
                {
                    "stageId": stage["stageId"],
                    "sequenceRole": stage["sequenceRole"],
                    "actionId": stage["actionId"],
                    "stageKind": stage["stageKind"],
                    "durationMs": (
                        6500
                        if pattern_id == "VALTAN_HIGH_JUMP"
                        and stage["stageId"] == "AIRBORNE"
                        else stage["durationMs"]
                    ),
                    "defaultNextActionId": default_next,
                    "hit": _migrate_hit(stage),
                    "motion": copy.deepcopy(stage["motion"]),
                    "events": events,
                    "branches": copy.deepcopy(stage["branches"]),
                    "animation": copy.deepcopy(stage["animation"]),
                    "effectCues": effect_cues,
                    "cameraInvocations": camera_invocations,
                }
            )
        migrated_patterns.append(
            {
                "patternId": pattern_id,
                "displayName": pattern["displayName"],
                "category": pattern["category"],
                "compatibilitySelectionWeight": pattern["selectionWeight"],
                "actionId": pattern["actionId"],
                "entryActionId": pattern["stages"][0]["actionId"],
                "targetPolicy": pattern["targetPolicy"],
                "aimPolicy": pattern["aimPolicy"],
                "eligibility": {
                    "armorRequirement": pattern["armorRequirement"],
                    "phaseRequirement": pattern["phaseRequirement"],
                    "minimumGameplayPhase": pattern["minimumPhase"],
                    "maximumGameplayPhase": pattern["maximumPhase"],
                    "minimumHealthBarInclusive": pattern["minimumHealthBar"],
                    "maximumHealthBarInclusive": pattern["maximumHealthBar"],
                    "minimumRangeM": pattern["minimumRange"],
                    "maximumRangeM": pattern["maximumRange"],
                    "cooldownPolicy": "DERIVED_SOURCE_ACTION",
                    "selectionCooldownMs": None,
                    "cooldownGroupId": None,
                    "repeatPolicy": {
                        "kind": "SOFT_AVOID_UNLESS_ONLY_ELIGIBLE",
                        "limit": pattern["maximumConsecutiveUses"],
                    },
                },
                "invulnerableWhileRunning": pattern["invulnerableWhileRunning"],
                "sourceActionIds": copy.deepcopy(pattern["sourceActionIds"]),
                "sourceSequenceIndex": pattern["sourceSequenceIndex"],
                "presentationSources": copy.deepcopy(pattern["presentationSources"]),
                "serverMotion": copy.deepcopy(pattern["serverMotion"]),
                "reactions": copy.deepcopy(pattern["reactions"]),
                "stages": migrated_stages,
            }
        )
    independent_effects = []
    for entry in master["independentEffects"]:
        if entry["ownership"] == "SERVER_PATTERN_STAGE":
            cue_id = entry["effectCueBindingId"]
            owner_pattern = next(row for row in migrated_patterns if row["patternId"] == entry["ownerPatternId"])
            owner_stage = next(row for row in owner_pattern["stages"] if row["stageId"] == entry["ownerStageId"])
            if cue_id not in cue_owners:
                occurrences_by_id = unique_index(
                    owner_stage["animation"]["occurrences"],
                    "clipOccurrenceId",
                    f"{entry['ownerPatternId']}/{entry['ownerStageId']} animation occurrences",
                )
                cue = _migrate_effect_cue(
                    {"refId": cue_id, "cueProjection": entry["cueProjection"]},
                    cue_by_id,
                    occurrences_by_id,
                    f"{entry['ownerPatternId']}/{entry['ownerStageId']} managed cue",
                    live_split_cue_ids,
                )
                if cue is None:
                    continue
                owner_stage["effectCues"].append(cue)
                cue_owners[cue_id] = (entry["ownerPatternId"], entry["ownerStageId"])
            independent_effects.append(
                {
                    "independentEffectId": entry["independentEffectId"],
                    "displayName": entry["displayName"],
                    "ownership": "SERVER_PATTERN_STAGE",
                    "cueId": cue_id,
                }
            )
        elif entry["ownership"] == "SERVER_COMBAT_OBJECT":
            archetype = entry["combatObjectArchetypeId"]
            event_id = spawn_event_by_archetype.get(archetype)
            if event_id is None:
                raise PipelineError(f"combat independent effect has no migrated spawn event: {archetype}")
            independent_effects.append(
                {
                    "independentEffectId": entry["independentEffectId"],
                    "displayName": entry["displayName"],
                    "ownership": "SERVER_COMBAT_OBJECT",
                    "spawnEventId": event_id,
                }
            )
        else:
            raise PipelineError(f"unsupported independent effect ownership: {entry['ownership']}")
    preview_paths = copy.deepcopy(master["previewPaths"])
    preview_paths.update(
        {
            "combatObjectAuthoring": COMBAT_AUTHORING_REL,
            "worldEventSets": WORLD_SET_REL,
            "legacyCompatibility": LEGACY_REL,
        }
    )
    result = {
        "schema": "lostark.valtan-pattern-master",
        "formatVersion": 2,
        "bossArchetypeId": master["bossArchetypeId"],
        "encounterId": master["encounterId"],
        "scope": master["scope"],
        "previewPaths": preview_paths,
        "retiredPatternIds": copy.deepcopy(master["retiredPatternIds"]),
        "decisionModel": {
            "scriptedSequence": None,
            "selectionSets": selection_sets,
            "selectionWindows": selection_windows,
            "mechanics": mechanics,
            "manualAuditions": [],
        },
        "counterReactionLayers": copy.deepcopy(master["counterReactionLayers"]),
        "independentEffects": independent_effects,
        "patterns": migrated_patterns,
    }
    validate_v2_master(
        result,
        build_world_event_sets(docs[WORLD_PRODUCT_REL]),
        docs[COMBAT_AUTHORING_REL],
        migration_fixture=True,
    )
    return result


def _event_fields(kind: str) -> tuple[str, ...]:
    common = ("eventId", "trigger", "kind")
    return {
        "SET_BOSS_FLAG": common + ("flagId", "enabled"),
        "SPAWN_COMBAT_OBJECT": common + ("combatObjectArchetypeId", "count"),
        "SPAWN_COMBAT_OBJECT_VOLLEY": common
        + (
            "combatObjectArchetypeId",
            "volleyPolicy",
            "countPerResolvedTarget",
            "layout",
            "spawnSchedule",
            "arenaRandom",
            "allowOverlap",
            "maximumTotalObjects",
        ),
        "SET_GAMEPLAY_PHASE": common + ("gameplayPhase",),
        "TRIGGER_WORLD_EVENT_SET": common + ("worldEventSetId",),
        "RETARGET_RANDOM_ALIVE": common,
        "RETURN_TO_ARENA_CENTER": common,
        "DAMAGE_GRABBED_PLAYERS": common + ("damageProfileId",),
        "EXECUTE_GRABBED_PLAYERS": common,
        "RELEASE_GRABBED_PLAYERS": common
        + ("releaseMode", "speedMps", "durationMs", "yawOffsetDegrees"),
    }.get(kind, ())


def _validate_volley_spawn_schedule(
    schedule: Any, stage_duration_ms: int, event_id: str
) -> tuple[int, int]:
    context = f"event {event_id}.spawnSchedule"
    if not isinstance(schedule, dict):
        raise PipelineError(f"{context} must be an object")
    exact(schedule, ("kind", "count", "firstOffsetMs", "intervalMs"), context)
    if schedule["kind"] != "INTERVAL":
        raise PipelineError(f"unsupported volley spawn schedule: {event_id}")
    count = integer(schedule["count"], f"{context}.count", 1, 8)
    first_offset_ms = integer(
        schedule["firstOffsetMs"], f"{context}.firstOffsetMs", 0, 600000
    )
    interval_ms = integer(
        schedule["intervalMs"], f"{context}.intervalMs", 0, 600000
    )
    if first_offset_ms != 0:
        raise PipelineError(f"volley spawn schedule must start at stage ENTER: {event_id}")
    if count > 1 and interval_ms == 0:
        raise PipelineError(f"repeating volley spawn schedule has zero interval: {event_id}")
    if count == 1 and interval_ms != 0:
        raise PipelineError(
            f"single volley spawn schedule has non-zero interval: {event_id}"
        )
    final_offset_ms = first_offset_ms + (count - 1) * interval_ms
    if final_offset_ms >= stage_duration_ms:
        raise PipelineError(
            f"volley spawn schedule exceeds its stage duration: {event_id}"
        )
    return count, interval_ms


def _validate_volley_arena_random(arena_random: Any, event_id: str) -> int:
    context = f"event {event_id}.arenaRandom"
    if not isinstance(arena_random, dict):
        raise PipelineError(f"{context} must be an object")
    exact(
        arena_random,
        ("kind", "anchor", "count", "radiusM", "heightToleranceM"),
        context,
    )
    if (
        arena_random["kind"] != "RANDOM_NAVIGABLE_CIRCLE"
        or arena_random["anchor"] != "BOSS_SPAWN_POSITION"
    ):
        raise PipelineError(f"unsupported volley arena-random policy: {event_id}")
    count = integer(arena_random["count"], f"{context}.count", 1, 32)
    number(arena_random["radiusM"], f"{context}.radiusM", 0.01, 1000)
    number(
        arena_random["heightToleranceM"],
        f"{context}.heightToleranceM",
        0.01,
        1000,
    )
    return count


def _validate_manual_auditions(
    decision_model: dict[str, Any],
    pattern_by_id: Mapping[str, dict[str, Any]],
) -> set[str]:
    rows = decision_model.get("manualAuditions")
    if not isinstance(rows, list):
        raise PipelineError("decisionModel.manualAuditions must be an array")

    pattern_ids: set[str] = set()
    source_chain_ids: set[str] = set()
    for ordinal, row in enumerate(rows):
        context = f"manualAuditions[{ordinal}]"
        if not isinstance(row, dict):
            raise PipelineError(f"{context} must be an object")
        exact(
            row,
            ("patternId", "sourceChainId", "authoringPhase", "admissionState"),
            context,
        )
        pattern_id = stable_id(row["patternId"], f"{context}.patternId")
        source_chain_id = stable_id(
            row["sourceChainId"], f"{context}.sourceChainId"
        )
        integer(row["authoringPhase"], f"{context}.authoringPhase", 1, 3)
        if row["admissionState"] not in {
            MANUAL_SERVER_AUDITION,
            DERIVED_SERVER_PATTERN,
        }:
            raise PipelineError(
                f"{context}.admissionState is unsupported"
            )
        if pattern_id not in pattern_by_id:
            raise PipelineError(f"manual audition pattern is missing: {pattern_id}")
        if pattern_id in pattern_ids:
            raise PipelineError(f"manual audition pattern is duplicated: {pattern_id}")
        if source_chain_id in source_chain_ids:
            raise PipelineError(
                f"manual audition sourceChainId is duplicated: {source_chain_id}"
            )
        pattern_ids.add(pattern_id)
        source_chain_ids.add(source_chain_id)
    return pattern_ids


def read_saved_flow_document(root: Path) -> dict[str, Any]:
    """Read the one repository-relative Flow input, never a caller-supplied path."""

    path = repo_path(root, SAVED_FLOW_REL)
    try:
        raw = path.read_bytes()
        if not raw or len(raw) > SAVED_FLOW_MAX_BYTES:
            raise PipelineError("saved Flow document is empty or exceeds 256 KiB")
        return json.loads(
            raw.decode("utf-8"),
            object_pairs_hook=_reject_duplicate_pairs,
            parse_constant=_reject_nonfinite_constant,
        )
    except PipelineError:
        raise
    except (OSError, UnicodeError, ValueError) as exc:
        raise PipelineError(f"cannot read saved Flow document: {exc}") from exc


def validate_saved_flow_document(
    document: dict[str, Any],
    admitted_pattern_ids: Iterable[str],
    *,
    require_nonempty: bool = True,
) -> dict[str, Any]:
    """Match the Boss Tool's stable slot contract without sorting or deduplicating."""

    exact(document, ("schema", "formatVersion", "flows"), "saved Flow document")
    if document["schema"] != "lostark.valtan-boss-audition-flows":
        raise PipelineError("saved Flow schema is invalid")
    integer(document["formatVersion"], "saved Flow formatVersion", 1, 1)
    flows = document["flows"]
    if not isinstance(flows, list) or len(flows) != 1:
        raise PipelineError("saved Flow must contain exactly one default flow")
    flow = flows[0]
    exact(
        flow,
        ("flowId", "nextSlotOrdinal", "interStepPursuitMs", "slots"),
        "saved Flow definition",
    )
    if flow["flowId"] != DEFAULT_SAVED_FLOW_ID:
        raise PipelineError("saved Flow flowId is not the supported default flow")
    next_ordinal = integer(
        flow["nextSlotOrdinal"], "saved Flow nextSlotOrdinal", 1, 1000000
    )
    integer(flow["interStepPursuitMs"], "saved Flow interStepPursuitMs", 100, 10000)
    slots = flow["slots"]
    if (
        not isinstance(slots, list)
        or len(slots) > SAVED_FLOW_MAX_SLOTS
        or (require_nonempty and not slots)
    ):
        raise PipelineError(
            f"saved Flow requires 1..{SAVED_FLOW_MAX_SLOTS} slots for Product playback"
        )
    admitted_rows = list(admitted_pattern_ids)
    admitted = set(admitted_rows)
    if (
        not admitted
        or len(admitted) != len(admitted_rows)
        or any(
            not isinstance(value, str)
            or re.fullmatch(r"[A-Za-z0-9_.-]{1,128}", value) is None
            for value in admitted
        )
    ):
        raise PipelineError("saved Flow admitted inventory is invalid or duplicated")
    slot_ids: set[str] = set()
    maximum_ordinal = 0
    for ordinal, slot in enumerate(slots):
        context = f"saved Flow slots[{ordinal}]"
        exact(slot, ("slotId", "patternId"), context)
        slot_id = slot["slotId"]
        match = re.fullmatch(
            re.escape(DEFAULT_SAVED_FLOW_ID) + r"\.slot\.([0-9]{6})",
            slot_id if isinstance(slot_id, str) else "",
        )
        if (
            match is None
            or len(slot_id) > 128
            or int(match.group(1)) == 0
            or slot_id in slot_ids
        ):
            raise PipelineError(f"{context}.slotId is malformed or duplicated")
        pattern_id = slot["patternId"]
        if not isinstance(pattern_id, str) or pattern_id not in admitted:
            raise PipelineError(f"{context}.patternId is not in the Boss Tool inventory")
        if pattern_id == OPTIONAL_ENTRY_PATTERN_ID and ordinal != 0:
            raise PipelineError("the optional entry cinematic must occur exactly once at the first step")
        slot_ids.add(slot_id)
        maximum_ordinal = max(maximum_ordinal, int(match.group(1)))
    if next_ordinal <= maximum_ordinal:
        raise PipelineError("saved Flow nextSlotOrdinal would reuse a stable slot ID")
    return flow


def resolve_gameplay_flow_reference(
    gameplay: dict[str, Any],
    flow_document: dict[str, Any] | None = None,
) -> dict[str, Any]:
    """Resolve the tagged physical source to the existing inline sequence IR."""

    decision = gameplay.get("decisionModel") if isinstance(gameplay, dict) else None
    sequence = decision.get("scriptedSequence") if isinstance(decision, dict) else None
    if not isinstance(sequence, dict) or "flowId" not in sequence:
        return gameplay
    exact(sequence, ("sequenceId", "mode", "flowId"), "scriptedSequence Flow reference")
    stable_id(sequence["sequenceId"], "scriptedSequence.sequenceId")
    if sequence["mode"] != SCRIPTED_SEQUENCE_MODE:
        raise PipelineError("scriptedSequence.mode must be ORDERED_ONCE_THEN_IDLE")
    if sequence["flowId"] != DEFAULT_SAVED_FLOW_ID:
        raise PipelineError("scriptedSequence flowId is not the supported default flow")
    if flow_document is None:
        raise PipelineError("scriptedSequence Flow reference requires its snapshot-local Flow document")
    patterns = gameplay.get("patterns")
    if not isinstance(patterns, list) or not patterns or any(
        not isinstance(pattern, dict) for pattern in patterns
    ):
        raise PipelineError("saved Flow requires authored pattern definitions")
    pattern_by_id = unique_index(patterns, "patternId", "gameplay patterns")
    _validate_manual_auditions(decision, pattern_by_id)
    # Match the Client's strictly joined split-owned inventory. Grouping is
    # presentation metadata, never another list of IDs or a fixed count gate.
    # Full gameplay/presentation/owner validation still follows this resolve.
    flow = validate_saved_flow_document(flow_document, pattern_by_id)
    resolved = copy.deepcopy(gameplay)
    resolved["decisionModel"]["scriptedSequence"] = {
        "sequenceId": sequence["sequenceId"],
        "mode": sequence["mode"],
        "interStepPursuitMs": flow["interStepPursuitMs"],
        "patternIds": [slot["patternId"] for slot in flow["slots"]],
    }
    return resolved


def _restore_gameplay_flow_reference(
    gameplay: dict[str, Any],
    source_gameplay: dict[str, Any],
    flow_document: dict[str, Any] | None,
) -> dict[str, Any]:
    """Keep immutable authoring on the same reference plus Flow source closure."""

    sequence = source_gameplay["decisionModel"]["scriptedSequence"]
    if not isinstance(sequence, dict) or "flowId" not in sequence:
        return gameplay
    restored = copy.deepcopy(gameplay)
    restored["decisionModel"]["scriptedSequence"] = copy.deepcopy(sequence)
    resolved = resolve_gameplay_flow_reference(restored, flow_document)
    if resolved["decisionModel"]["scriptedSequence"] != gameplay["decisionModel"]["scriptedSequence"]:
        raise PipelineError("saved Flow source does not match the immutable authoring sequence")
    return restored


def _snapshot_saved_flow_text(root: Path, sources: dict[str, Any]) -> str | None:
    entry = next((row for row in sources["files"] if row["path"] == SAVED_FLOW_REL), None)
    if entry is None:
        return None
    text = read_text(repo_path(root, SAVED_FLOW_REL))
    raw = text.encode("utf-8")
    if sha256_bytes(raw) != entry["sha256"] or len(raw) != entry["bytes"]:
        raise PipelineError("saved Flow changed while its immutable source was staged")
    return text


def _validate_scripted_sequence(
    decision_model: dict[str, Any],
    pattern_by_id: Mapping[str, dict[str, Any]],
) -> tuple[str, ...]:
    sequence = decision_model.get("scriptedSequence")
    if sequence is None:
        return ()
    if not isinstance(sequence, dict):
        raise PipelineError("decisionModel.scriptedSequence must be an object or null")
    exact(
        sequence,
        ("sequenceId", "mode", "interStepPursuitMs", "patternIds"),
        "decisionModel.scriptedSequence",
    )
    stable_id(sequence["sequenceId"], "scriptedSequence.sequenceId")
    if sequence["mode"] != SCRIPTED_SEQUENCE_MODE:
        raise PipelineError(
            "scriptedSequence.mode must be ORDERED_ONCE_THEN_IDLE"
        )
    integer(
        sequence["interStepPursuitMs"],
        "scriptedSequence.interStepPursuitMs",
        100,
        10000,
    )
    rows = sequence["patternIds"]
    if (
        not isinstance(rows, list)
        or not rows
        or len(rows) > SAVED_FLOW_MAX_SLOTS
    ):
        raise PipelineError("scriptedSequence.patternIds count is invalid")
    pattern_ids = tuple(
        stable_id(value, "scriptedSequence.patternId") for value in rows
    )
    for pattern_id in pattern_ids:
        if pattern_id not in pattern_by_id:
            raise PipelineError(
                f"scriptedSequence names no managed pattern: {pattern_id}"
            )
    if OPTIONAL_ENTRY_PATTERN_ID in pattern_ids and (
        pattern_ids[0] != OPTIONAL_ENTRY_PATTERN_ID
        or pattern_ids.count(OPTIONAL_ENTRY_PATTERN_ID) != 1
    ):
        raise PipelineError("the optional entry cinematic must occur exactly once at the first step")
    return pattern_ids


def validate_manual_audition_animation_lineage(
    master: dict[str, Any],
    debug_presentation: dict[str, Any],
    promotion_manifest: dict[str, Any],
) -> None:
    """Keep phase-2/3 Server auditions on the reviewed animation intake.

    Debug intake may leave target IDs and native clip lengths unresolved, but a
    promoted MANUAL_SERVER_AUDITION must preserve the exact clip order, source
    offsets, mapping basis, and playback rates.  The joined Product remains the
    owner of explicit stage walls and loop/hold policy.
    """

    exact(
        debug_presentation,
        (
            "schema",
            "formatVersion",
            "bossArchetypeId",
            "encounterId",
            "chains",
        ),
        "Valtan debug presentation root",
    )
    if (
        debug_presentation["schema"]
        != "lostark.valtan-pattern-presentation-debug"
        or debug_presentation["formatVersion"] != 1
        or debug_presentation["bossArchetypeId"] != master["bossArchetypeId"]
        or debug_presentation["encounterId"] != master["encounterId"]
    ):
        raise PipelineError("Valtan debug presentation header mismatch")

    exact(
        promotion_manifest,
        (
            "schema",
            "formatVersion",
            "bossArchetypeId",
            "encounterId",
            "sourceDocument",
            "presentationProfile",
            "clipAliases",
            "animationIntakeOnly",
            "patterns",
        ),
        "Valtan animation promotion manifest root",
    )
    if (
        promotion_manifest["schema"]
        != "lostark.valtan-animation-chain-promotions"
        or promotion_manifest["formatVersion"] != 2
        or promotion_manifest["bossArchetypeId"] != master["bossArchetypeId"]
        or promotion_manifest["encounterId"] != master["encounterId"]
    ):
        raise PipelineError("Valtan animation promotion manifest header mismatch")

    chains = unique_index(
        debug_presentation["chains"], "chainId", "Valtan debug chains"
    )
    patterns = unique_index(master["patterns"], "patternId", "v2 patterns")
    manual_rows = [
        row
        for row in master["decisionModel"]["manualAuditions"]
        if row["admissionState"] == MANUAL_SERVER_AUDITION
    ]

    promotion_rows = promotion_manifest["patterns"]
    intake_rows = promotion_manifest["animationIntakeOnly"]
    if not isinstance(promotion_rows, list):
        raise PipelineError("Valtan animation promotion patterns must be an array")
    if not isinstance(intake_rows, list):
        raise PipelineError("Valtan animation intake-only rows must be an array")

    promotion_lineage: list[tuple[str, str, int, str]] = []
    declared_chain_ids: list[str] = []
    declared_pattern_ids: set[str] = set()
    declared_chain_id_set: set[str] = set()
    promotion_required = {
        "sourceChainId",
        "patternId",
        "displayName",
        "authoringPhase",
        "admissionState",
    }
    promotion_optional = {"targetPolicy", "aimPolicy", "sourceActionId"}
    for ordinal, row in enumerate(promotion_rows):
        context = f"animation promotion patterns[{ordinal}]"
        if not isinstance(row, dict):
            raise PipelineError(f"{context} must be an object")
        actual_fields = set(row)
        if (
            not promotion_required.issubset(actual_fields)
            or actual_fields - promotion_required - promotion_optional
        ):
            raise PipelineError(f"{context} fields mismatch")
        if ("targetPolicy" in row) != ("aimPolicy" in row):
            raise PipelineError(f"{context} target/aim policy must be paired")
        if "sourceActionId" in row:
            integer(row["sourceActionId"], f"{context}.sourceActionId", 1)
        pattern_id = stable_id(row["patternId"], f"{context}.patternId")
        chain_id = stable_id(row["sourceChainId"], f"{context}.sourceChainId")
        phase = integer(
            row["authoringPhase"], f"{context}.authoringPhase", 1, 3
        )
        if (
            not isinstance(row["displayName"], str)
            or not row["displayName"].strip()
            or row["admissionState"] != MANUAL_SERVER_AUDITION
        ):
            raise PipelineError(f"{context} metadata is invalid")
        if pattern_id in declared_pattern_ids or chain_id in declared_chain_id_set:
            raise PipelineError(f"{context} identity is duplicated")
        declared_pattern_ids.add(pattern_id)
        declared_chain_id_set.add(chain_id)
        declared_chain_ids.append(chain_id)
        promotion_lineage.append(
            (pattern_id, chain_id, phase, row["admissionState"])
        )

    intake_chain_ids: list[str] = []
    for ordinal, row in enumerate(intake_rows):
        context = f"animationIntakeOnly[{ordinal}]"
        if not isinstance(row, dict):
            raise PipelineError(f"{context} must be an object")
        exact(
            row,
            ("sourceChainId", "displayName", "authoringPhase", "admissionState"),
            context,
        )
        chain_id = stable_id(row["sourceChainId"], f"{context}.sourceChainId")
        integer(row["authoringPhase"], f"{context}.authoringPhase", 1, 3)
        if (
            not isinstance(row["displayName"], str)
            or not row["displayName"].strip()
            or row["admissionState"] != ANIMATION_INTAKE_ONLY
        ):
            raise PipelineError(f"{context} metadata is invalid")
        if chain_id in declared_chain_id_set:
            raise PipelineError(f"{context} identity is duplicated")
        declared_chain_id_set.add(chain_id)
        declared_chain_ids.append(chain_id)
        intake_chain_ids.append(chain_id)

    debug_chain_ids = [row["chainId"] for row in debug_presentation["chains"]]
    promoted_chain_ids = [row["sourceChainId"] for row in promotion_rows]
    promoted_chain_id_set = set(promoted_chain_ids)
    intake_chain_id_set = set(intake_chain_ids)
    if (
        declared_chain_id_set != set(debug_chain_ids)
        or promoted_chain_ids
        != [value for value in debug_chain_ids if value in promoted_chain_id_set]
        or intake_chain_ids
        != [value for value in debug_chain_ids if value in intake_chain_id_set]
    ):
        raise PipelineError(
            "phase-2/3 animation intake must exact-join promotion plus "
            "intake-only declarations: "
            f"declared={declared_chain_ids} debug={debug_chain_ids}"
        )

    manual_lineage = [
        (
            row["patternId"],
            row["sourceChainId"],
            row["authoringPhase"],
            row["admissionState"],
        )
        for row in manual_rows
    ]
    if manual_lineage != promotion_lineage:
        raise PipelineError(
            "manualAuditions must exact-join promoted animation lineage: "
            f"manual={manual_lineage} promoted={promotion_lineage}"
        )

    for chain_id in intake_chain_ids:
        chain = chains[chain_id]
        if chain.get("targetPatternId") or chain.get("targetStageId"):
            raise PipelineError(
                f"animation intake-only chain targets must remain empty: {chain_id}"
            )

    def product_clip_name(value: str) -> str:
        return "mesh_" + value if value.startswith("att_") else value

    for row in manual_rows:
        pattern_id = row["patternId"]
        chain_id = row["sourceChainId"]
        pattern = patterns[pattern_id]
        chain = chains[chain_id]
        context = f"manual audition {pattern_id}/{chain_id}"
        exact(
            chain,
            ("chainId", "targetPatternId", "targetStageId", "animation"),
            context,
        )
        target_pattern_id = chain["targetPatternId"]
        target_stage_id = chain["targetStageId"]
        if not isinstance(target_pattern_id, str) or not isinstance(
            target_stage_id, str
        ):
            raise PipelineError(f"{context} target IDs must be strings")
        if bool(target_pattern_id) != bool(target_stage_id):
            raise PipelineError(f"{context} target IDs must be empty or paired")
        if target_pattern_id:
            stage_ids = {stage["stageId"] for stage in pattern["stages"]}
            if target_pattern_id != pattern_id or target_stage_id not in stage_ids:
                raise PipelineError(f"{context} target IDs do not join Product")

        animation = chain["animation"]
        exact(
            animation,
            ("endPolicy", "repeatCount", "occurrences"),
            f"{context}.animation",
        )
        if animation["endPolicy"] != "NATIVE_CLIP_LENGTHS":
            raise PipelineError(f"{context} debug endPolicy is unsupported")
        occurrences = animation["occurrences"]
        repeat_count = integer(
            animation["repeatCount"], f"{context}.repeatCount", 1, 256
        )
        if not isinstance(occurrences, list) or repeat_count != len(occurrences):
            raise PipelineError(f"{context} debug occurrence count drift")

        product_occurrences = [
            occurrence
            for stage in pattern["stages"]
            for occurrence in stage["animation"]["occurrences"]
        ]
        if pattern_id == "VALTAN_TRASH":
            # The reviewed intake still owns the eight setup/rush clips. The
            # Product graph now owns real capture outcomes, not three separate
            # ordered audition rows. Validate its source slices explicitly.
            extensions = {
                "CATCH_COUNTER": [("mesh_att_battle_13_05-1", 0, 200)],
                "CATCH_PRE_IMPACT": [("mesh_att_battle_13_05-1", 200, 1300)],
                "CATCH_SLAM": [("mesh_att_battle_13_05-1", 1500, 1500)],
                "EXECUTE_TAIL": [("mesh_att_battle_13_05-1", 1500, 1500)],
                "RUSH_MISS": [("mesh_att_battle_13_05-2", 0, 1000)],
                "GROGGY": [
                    ("mesh_abn_groggy_1_start", 0, 1833),
                    ("mesh_abn_groggy_1_loop", 0, 600),
                    ("mesh_abn_groggy_1_end", 0, 2000),
                ],
            }
            expected_ids = [f"STEP_{i:02d}" for i in range(1, 9)] + list(extensions)
            if [stage["stageId"] for stage in pattern["stages"]] != expected_ids:
                raise PipelineError(f"{context} capture branch stage inventory drift")
            if 420631 not in pattern["sourceActionIds"]:
                raise PipelineError(f"{context} real groggy source action is missing")
            for branch_stage in pattern["stages"][8:]:
                actual = [
                    (clip["clip"], clip["sourceStartMs"], clip["playMs"])
                    for clip in branch_stage["animation"]["occurrences"]
                ]
                if actual != extensions[branch_stage["stageId"]] or any(
                    clip["playRate"] != 1.0 or clip["repeatUntilStageEnd"]
                    for clip in branch_stage["animation"]["occurrences"]
                ):
                    raise PipelineError(f"{context} capture branch source slice drift")
            product_occurrences = product_occurrences[:len(occurrences)]
        trash_fragments = {
            "VALTAN_TRASH_CATCH_IF": ["STEP_07", "STEP_08", "CATCH_COUNTER", "CATCH_PRE_IMPACT",
                                      "CATCH_SLAM", "EXECUTE_TAIL", "RUSH_MISS", "GROGGY"],
            "VALTAN_TRASH_CATCH_SUCCESS": ["CATCH_COUNTER", "CATCH_PRE_IMPACT", "CATCH_SLAM", "EXECUTE_TAIL"],
            "VALTAN_TRASH_CATCH_FAIL": ["RUSH_MISS"],
        }
        if pattern_id in trash_fragments:
            # These are now finite subgraphs of the reviewed main capture
            # pattern, not an unrelated replay of its old animation-only rows.
            expected_ids = trash_fragments[pattern_id]
            if [stage["stageId"] for stage in pattern["stages"]] != expected_ids:
                raise PipelineError(f"{context} shared capture fragment stage inventory drift")
            shared = {stage["stageId"]: stage for stage in patterns["VALTAN_TRASH"]["stages"]}
            for fragment in pattern["stages"]:
                source_fragment = shared[fragment["stageId"]]
                def slices(value: dict[str, Any]) -> list[tuple[Any, ...]]:
                    return [(clip["clip"], clip["sourceStartMs"], clip["playMs"],
                             clip["playRate"], clip["repeatUntilStageEnd"])
                            for clip in value["animation"]["occurrences"]]
                if slices(fragment) != slices(source_fragment) or fragment["durationMs"] != source_fragment["durationMs"]:
                    raise PipelineError(f"{context} shared capture fragment source slice drift")
            continue
        if len(product_occurrences) != len(occurrences):
            raise PipelineError(f"{context} Product occurrence count drift")
        for ordinal, (source, product) in enumerate(
            zip(occurrences, product_occurrences)
        ):
            occurrence_context = f"{context}.occurrences[{ordinal}]"
            exact(
                source,
                (
                    "clipOccurrenceId",
                    "clip",
                    "mappingBasis",
                    "sourceStartMs",
                    "playMs",
                    "playRate",
                    "repeatUntilStageEnd",
                ),
                occurrence_context,
            )
            source_clip = stable_id(source["clip"], f"{occurrence_context}.clip")
            mapping_basis(
                source["mappingBasis"], f"{occurrence_context}.mappingBasis"
            )
            source_start_ms = integer(
                source["sourceStartMs"],
                f"{occurrence_context}.sourceStartMs",
                0,
                600000,
            )
            integer(
                source["playMs"], f"{occurrence_context}.playMs", 0, 600000
            )
            play_rate = number(
                source["playRate"],
                f"{occurrence_context}.playRate",
                0.000001,
                1000,
            )
            boolean(
                source["repeatUntilStageEnd"],
                f"{occurrence_context}.repeatUntilStageEnd",
            )
            if (
                product_clip_name(source_clip) != product["clip"]
                or source["mappingBasis"] != product["mappingBasis"]
                or source_start_ms != product["sourceStartMs"]
                or play_rate != product["playRate"]
            ):
                raise PipelineError(
                    f"{occurrence_context} no longer matches joined Product"
                )


def validate_v2_master(
    master: dict[str, Any],
    world_sets: dict[str, Any],
    combat_authoring: dict[str, Any],
    *,
    migration_fixture: bool = False,
) -> None:
    exact(
        master,
        ("schema", "formatVersion", "bossArchetypeId", "encounterId", "scope", "previewPaths", "retiredPatternIds", "decisionModel", "counterReactionLayers", "independentEffects", "patterns"),
        "v2 master root",
    )
    if (
        master["schema"] != "lostark.valtan-pattern-master"
        or master["formatVersion"] != 2
        or master["bossArchetypeId"] != "BOSS_VALTAN"
        or master["encounterId"] != "ENCOUNTER_VALTAN"
        or master["scope"] != "PHASE_ONE"
    ):
        raise PipelineError("v2 master header mismatch")
    for name, relative in master["previewPaths"].items():
        if not isinstance(name, str) or not isinstance(relative, str) or not relative or "\\" in relative or relative.startswith("/") or ".." in relative.split("/"):
            raise PipelineError(f"v2 preview path is illegal: {name}={relative!r}")
    exact(
        master["decisionModel"],
        (
            "scriptedSequence",
            "selectionSets",
            "selectionWindows",
            "mechanics",
            "manualAuditions",
        ),
        "decisionModel",
    )
    pattern_by_id = unique_index(master["patterns"], "patternId", "v2 patterns")
    retired_rows = master["retiredPatternIds"]
    if not isinstance(retired_rows, list):
        raise PipelineError("retiredPatternIds must be an array")
    retired_ids = [stable_id(value, "retiredPatternIds") for value in retired_rows]
    if len(retired_ids) != len(set(retired_ids)):
        raise PipelineError("retiredPatternIds contains a duplicate")
    if set(retired_ids) & set(pattern_by_id):
        raise PipelineError("retiredPatternIds cannot contain an active pattern")
    manual_patterns = _validate_manual_auditions(
        master["decisionModel"], pattern_by_id
    )
    scripted_pattern_ids = _validate_scripted_sequence(
        master["decisionModel"], pattern_by_id
    )
    set_ids: set[str] = set()
    candidate_patterns: set[str] = set()
    for set_ordinal, selection_set in enumerate(master["decisionModel"]["selectionSets"]):
        exact(selection_set, ("selectionSetId", "mode", "candidates"), f"selectionSets[{set_ordinal}]")
        set_id = stable_id(selection_set["selectionSetId"], f"selectionSets[{set_ordinal}].selectionSetId")
        if set_id in set_ids or selection_set["mode"] != "WEIGHTED_POOL":
            raise PipelineError(f"invalid selection set: {set_id}")
        set_ids.add(set_id)
        if not isinstance(selection_set["candidates"], list) or not selection_set["candidates"]:
            raise PipelineError(f"selection set has no candidates: {set_id}")
        candidates: set[str] = set()
        enabled_weight = 0
        for candidate in selection_set["candidates"]:
            exact(candidate, ("patternId", "weight", "enabled"), f"selection set {set_id} candidate")
            pattern_id = stable_id(candidate["patternId"], f"selection set {set_id} patternId")
            if pattern_id in candidates or pattern_id not in pattern_by_id:
                raise PipelineError(f"invalid selection candidate: {set_id}/{pattern_id}")
            candidates.add(pattern_id)
            # enabled is the only disable switch.  A disabled candidate keeps
            # its last positive weight so re-enabling cannot silently invent a
            # new value and weight=0 never becomes a second disable meaning.
            weight = integer(candidate["weight"], f"selection set {set_id} weight", 1, 100000)
            enabled = boolean(candidate["enabled"], f"selection set {set_id} enabled")
            if enabled:
                enabled_weight += weight
            candidate_patterns.add(pattern_id)
        if selection_set["mode"] == "WEIGHTED_POOL" and enabled_weight <= 0:
            raise PipelineError(f"weighted selection set has no enabled positive weight: {set_id}")
    window_ids: set[str] = set()
    rotation_ids: set[str] = set()
    window_set_ids: set[str] = set()
    previous_minimum_by_phase: dict[int, int] = {}
    for window_ordinal, window in enumerate(master["decisionModel"]["selectionWindows"]):
        exact(
            window,
            ("windowId", "gameplayPhase", "maximumHealthBarInclusive", "minimumHealthBarExclusive", "selectionSetId", "compatibilityRotationId"),
            f"selectionWindows[{window_ordinal}]",
        )
        window_id = stable_id(window["windowId"], f"selectionWindows[{window_ordinal}].windowId")
        rotation_id = stable_id(window["compatibilityRotationId"], f"selectionWindows[{window_ordinal}].compatibilityRotationId")
        set_id = stable_id(window["selectionSetId"], f"selectionWindows[{window_ordinal}].selectionSetId")
        if (
            window_id in window_ids
            or rotation_id in rotation_ids
            or set_id in window_set_ids
            or set_id not in set_ids
        ):
            raise PipelineError(f"selection window references missing set: {window['selectionSetId']}")
        window_ids.add(window_id)
        rotation_ids.add(rotation_id)
        window_set_ids.add(set_id)
        phase = integer(window["gameplayPhase"], "selection window gameplayPhase", 1, 3)
        maximum = integer(window["maximumHealthBarInclusive"], "selection window maximum", 1, 1000)
        minimum = integer(window["minimumHealthBarExclusive"], "selection window minimum", 0, 999)
        previous_minimum = previous_minimum_by_phase.get(phase)
        if maximum <= minimum or (
            previous_minimum is not None and maximum != previous_minimum
        ):
            raise PipelineError(f"selection window is empty/duplicate: {window['windowId']}")
        previous_minimum_by_phase[phase] = minimum
    if window_set_ids != set_ids:
        raise PipelineError("selection windows do not cover every selection set exactly once")
    mechanic_patterns: set[str] = set()
    mechanic_ids: set[str] = set()
    mechanic_triggers: set[tuple[int, int]] = set()
    previous_mechanic_health_bar: int | None = None
    previous_mechanic_order = 0
    for mechanic in master["decisionModel"]["mechanics"]:
        exact(
            mechanic,
            ("mechanicId", "patternId", "trigger", "triggerOrder", "oncePerEncounter", "failurePolicy"),
            "decision mechanic",
        )
        mechanic_id = stable_id(mechanic["mechanicId"], "mechanicId")
        pattern_id = stable_id(mechanic["patternId"], "mechanic.patternId")
        if (
            mechanic_id in mechanic_ids
            or pattern_id in mechanic_patterns
            or pattern_id not in pattern_by_id
        ):
            raise PipelineError(f"mechanic pattern is duplicate/missing: {pattern_id}")
        mechanic_ids.add(mechanic_id)
        mechanic_patterns.add(pattern_id)
        exact(mechanic["trigger"], ("kind", "healthBar"), f"mechanic {pattern_id}.trigger")
        if mechanic["trigger"]["kind"] != "HEALTH_BAR_CROSSING":
            raise PipelineError(f"unsupported mechanic trigger: {pattern_id}")
        health_bar = integer(
            mechanic["trigger"]["healthBar"],
            f"mechanic {pattern_id}.healthBar",
            1,
            1000,
        )
        trigger_order = integer(
            mechanic["triggerOrder"],
            f"mechanic {pattern_id}.triggerOrder",
            1,
            100000,
        )
        if (health_bar, trigger_order) in mechanic_triggers:
            raise PipelineError(
                f"mechanic trigger health/order is duplicated: "
                f"{health_bar}/{trigger_order}"
            )
        if previous_mechanic_health_bar is not None and (
            health_bar > previous_mechanic_health_bar
            or (
                health_bar == previous_mechanic_health_bar
                and trigger_order <= previous_mechanic_order
            )
        ):
            raise PipelineError(
                "mechanics require descending health bars with ascending "
                "same-bar triggerOrder"
            )
        mechanic_triggers.add((health_bar, trigger_order))
        previous_mechanic_health_bar = health_bar
        previous_mechanic_order = trigger_order
        if not boolean(
            mechanic["oncePerEncounter"],
            f"mechanic {pattern_id}.oncePerEncounter",
        ):
            raise PipelineError(f"mechanic must be oncePerEncounter: {pattern_id}")
        if mechanic["failurePolicy"] != "ABORT_ENCOUNTER_REQUIRE_RESET":
            raise PipelineError(f"unsupported mechanic failure policy: {pattern_id}")
    phase_one_windows = [
        window
        for window in master["decisionModel"]["selectionWindows"]
        if window["gameplayPhase"] == 1
    ]
    arena_break = [
        mechanic
        for mechanic in master["decisionModel"]["mechanics"]
        if mechanic["patternId"] == "VALTAN_ARENA_BREAK_109"
    ]
    if (
        not phase_one_windows
        or len(arena_break) != 1
        or arena_break[0]["trigger"]["healthBar"]
        != phase_one_windows[-1]["minimumHealthBarExclusive"]
    ):
        raise PipelineError(
            "VALTAN_ARENA_BREAK_109 healthBar must equal the final phase-1 "
            "selection-window boundary; use a future atomic phase-boundary operation"
        )
    ownership_overlap = (
        (candidate_patterns & mechanic_patterns)
        | (candidate_patterns & manual_patterns)
        | (mechanic_patterns & manual_patterns)
    )
    if ownership_overlap:
        raise PipelineError(
            "decision model pattern ownership overlaps: "
            + ", ".join(sorted(ownership_overlap))
        )
    decision_owned_patterns = (
        candidate_patterns | mechanic_patterns | manual_patterns
    )
    # The reviewed entrance remains a loadable definition when a custom Flow
    # omits it. It is not automatically prepended and is never a weighted owner.
    scripted_entry_only_patterns = (
        {OPTIONAL_ENTRY_PATTERN_ID}
        if OPTIONAL_ENTRY_PATTERN_ID in pattern_by_id
        and OPTIONAL_ENTRY_PATTERN_ID not in decision_owned_patterns
        else set()
    )
    if (
        decision_owned_patterns | scripted_entry_only_patterns
        != set(pattern_by_id)
    ):
        raise PipelineError(
            "decision model must own every managed pattern exactly once; only "
            "VALTAN_ENTRANCE_CINEMATIC may be a dormant entry-only definition"
        )
    event_ids: set[str] = set()
    expected_cue_policies = {
        cue_id: scale_kind
        for cue_id, scale_kind in MANAGED_CUE_SCALE_POLICIES.items()
        if not migration_fixture or cue_id in MIGRATION_MANAGED_CUE_IDS
    }
    cue_ids: set[str] = set()
    scale_policy_counts: Counter[str] = Counter()
    spawn_events: set[str] = set()
    world_events: list[tuple[str, str, str]] = []
    gameplay_phase_events: list[tuple[str, str, str, int]] = []
    combat_archetypes = {row["combatObjectArchetypeId"] for row in combat_authoring["objects"]}
    for pattern_id, pattern in pattern_by_id.items():
        exact(
            pattern,
            ("patternId", "displayName", "category", "compatibilitySelectionWeight", "actionId", "entryActionId", "targetPolicy", "aimPolicy", "eligibility", "invulnerableWhileRunning", "sourceActionIds", "sourceSequenceIndex", "presentationSources", "serverMotion", "reactions", "stages") + (("finale",) if "finale" in pattern else ()),
            f"pattern {pattern_id}",
        )
        _validate_pattern_target_aim(pattern, f"pattern {pattern_id}")
        _validate_finale(pattern, pattern_by_id)
        compatibility_weight = integer(
            pattern["compatibilitySelectionWeight"],
            f"pattern {pattern_id}.compatibilitySelectionWeight",
            0,
            100000,
        )
        is_non_rotation = (
            pattern_id in mechanic_patterns or pattern_id in manual_patterns
        )
        if (is_non_rotation and compatibility_weight != 0) or (
            not is_non_rotation and compatibility_weight == 0
        ):
            raise PipelineError(
                "compatibilitySelectionWeight must be positive only for normal "
                f"patterns: {pattern_id}"
            )
        stable_id(pattern["actionId"], f"pattern {pattern_id}.actionId")
        stable_id(pattern["entryActionId"], f"pattern {pattern_id}.entryActionId")
        exact(
            pattern["eligibility"],
            ("armorRequirement", "phaseRequirement", "minimumGameplayPhase", "maximumGameplayPhase", "minimumHealthBarInclusive", "maximumHealthBarInclusive", "minimumRangeM", "maximumRangeM", "cooldownPolicy", "selectionCooldownMs", "cooldownGroupId", "repeatPolicy"),
            f"pattern {pattern_id}.eligibility",
        )
        eligibility = pattern["eligibility"]
        if eligibility["cooldownPolicy"] != "DERIVED_SOURCE_ACTION" or eligibility["selectionCooldownMs"] is not None or eligibility["cooldownGroupId"] is not None:
            raise PipelineError(f"initial migration only admits derived cooldown: {pattern_id}")
        exact(eligibility["repeatPolicy"], ("kind", "limit"), f"pattern {pattern_id}.repeatPolicy")
        if eligibility["repeatPolicy"]["kind"] != "SOFT_AVOID_UNLESS_ONLY_ELIGIBLE":
            raise PipelineError(f"initial migration only admits soft repeat: {pattern_id}")
        integer(eligibility["repeatPolicy"]["limit"], f"pattern {pattern_id}.repeat limit", 0, 64)
        minimum_range = number(
            eligibility["minimumRangeM"], f"pattern {pattern_id}.minimumRangeM", 0, 1000
        )
        maximum_range = number(
            eligibility["maximumRangeM"], f"pattern {pattern_id}.maximumRangeM", 0, 1000
        )
        if minimum_range > maximum_range:
            raise PipelineError(f"pattern range is inverted: {pattern_id}")
        stage_ids = unique_index(pattern["stages"], "stageId", f"pattern {pattern_id} stages")
        action_ids = {stage["actionId"] for stage in pattern["stages"]}
        if pattern["entryActionId"] not in action_ids:
            raise PipelineError(f"pattern entryActionId is missing: {pattern_id}")
        for stage_id, stage in stage_ids.items():
            exact(
                stage,
                _pattern_stage_fields(stage, joined=True),
                f"pattern {pattern_id} stage {stage_id}",
            )
            _validate_pattern_stage_extensions(
                stage, f"pattern {pattern_id} stage {stage_id}"
            )
            duration = integer(stage["durationMs"], f"{pattern_id}/{stage_id}.durationMs", 1, 600000)
            if stage["defaultNextActionId"] is not None and stage["defaultNextActionId"] not in action_ids:
                raise PipelineError(f"defaultNextActionId is missing: {pattern_id}/{stage_id}")
            grab_impacts = [event for event in stage["events"] if isinstance(event, dict) and event.get("kind") in (
                "DAMAGE_GRABBED_PLAYERS", "EXECUTE_GRABBED_PLAYERS"
            )]
            if grab_impacts and (len(stage["events"]) != 1 or stage["hit"]["shape"]["kind"] != "NONE"):
                raise PipelineError(f"grabbed-player impact must own its stage transaction: {pattern_id}/{stage_id}")
            for branch in stage["branches"]:
                exact(branch, ("outcome", "nextActionId"), f"{pattern_id}/{stage_id}.branch")
                if branch["nextActionId"] is not None and branch["nextActionId"] not in action_ids:
                    raise PipelineError(f"branch target is missing: {pattern_id}/{stage_id}")
            hit = stage["hit"]
            if hit["shape"]["kind"] == "NONE":
                exact(hit, ("shape",), f"{pattern_id}/{stage_id}.hit")
            else:
                hit_fields = (
                    "shape",
                    "schedule",
                    "serverDamageProfileId",
                    "pushRangeM",
                    "pushMs",
                    "knockdown",
                    "downMs",
                )
                has_player_response = (
                    "playerResponse" in hit or "attachmentSlot" in hit
                )
                if has_player_response:
                    hit_fields += ("playerResponse", "attachmentSlot")
                exact(
                    hit,
                    hit_fields,
                    f"{pattern_id}/{stage_id}.hit",
                )
                schedule = hit["schedule"]
                if schedule["kind"] == "EXPLICIT_OFFSETS":
                    exact(schedule, ("kind", "offsetsMs"), f"{pattern_id}/{stage_id}.schedule")
                    if (
                        not isinstance(schedule["offsetsMs"], list)
                        or not 1 <= len(schedule["offsetsMs"]) <= 64
                    ):
                        raise PipelineError(
                            f"explicit hit offsets require 1..64 entries: {pattern_id}/{stage_id}"
                        )
                    previous = -1
                    for offset in schedule["offsetsMs"]:
                        offset_value = integer(offset, f"{pattern_id}/{stage_id}.offset", 0, duration - 1)
                        if offset_value <= previous:
                            raise PipelineError(f"hit offsets must be strictly increasing: {pattern_id}/{stage_id}")
                        previous = offset_value
                elif schedule["kind"] == "INTERVAL":
                    exact(schedule, ("kind", "count", "firstOffsetMs", "intervalMs"), f"{pattern_id}/{stage_id}.schedule")
                    count = integer(schedule["count"], "interval count", 1, 64)
                    first = integer(schedule["firstOffsetMs"], "interval first", 0, duration - 1)
                    interval = integer(schedule["intervalMs"], "intervalMs", 0, duration)
                    if count > 1 and interval == 0:
                        raise PipelineError(
                            f"multi-hit interval must be positive: {pattern_id}/{stage_id}"
                        )
                    if first + (count - 1) * interval >= duration:
                        raise PipelineError(f"interval hit escapes stage: {pattern_id}/{stage_id}")
                else:
                    raise PipelineError(f"unsupported hit schedule: {pattern_id}/{stage_id}")
                damage_id = stable_id(
                    hit["serverDamageProfileId"],
                    f"{pattern_id}/{stage_id}.serverDamageProfileId",
                )
                if not damage_id.startswith("damage.valtan."):
                    raise PipelineError(
                        f"Valtan stage damage is outside its namespace: {pattern_id}/{stage_id}"
                    )
                number(hit["pushRangeM"], f"{pattern_id}/{stage_id}.pushRangeM", 0, 100000)
                integer(hit["pushMs"], f"{pattern_id}/{stage_id}.pushMs", 0, 600000)
                boolean(hit["knockdown"], f"{pattern_id}/{stage_id}.knockdown")
                integer(hit["downMs"], f"{pattern_id}/{stage_id}.downMs", 0, 600000)
                if has_player_response:
                    if (
                        hit["playerResponse"] != "CAPTURE"
                        or hit["attachmentSlot"] != "BOSS_LEFT_HAND"
                        or hit["pushRangeM"] != 0
                        or hit["pushMs"] != 0
                        or hit["knockdown"] is not False
                        or hit["downMs"] != 0
                    ):
                        raise PipelineError(
                            "capture hit requires the typed boss-left-hand slot "
                            f"without push/knockdown: {pattern_id}/{stage_id}"
                        )
            _validate_shape(hit["shape"], f"{pattern_id}/{stage_id}.shape")
            animation = stage["animation"]
            animation_mode = animation.get(
                "mode", ANIMATION_MODE_CLIP_SEQUENCE
            ) if isinstance(animation, dict) else None
            if animation_mode == ANIMATION_MODE_NONE:
                exact(animation, ("mode",), f"{pattern_id}/{stage_id}.animation")
                occurrences_by_id: dict[str, dict[str, Any]] = {}
            elif animation_mode == ANIMATION_MODE_CLIP_SEQUENCE:
                animation_fields = ("endPolicy", "repeatCount", "occurrences")
                if "mode" in animation:
                    animation_fields = ("mode",) + animation_fields
                exact(animation, animation_fields, f"{pattern_id}/{stage_id}.animation")
                if animation["endPolicy"] not in ("EXACT", "HOLD_LAST_POSE", "LOOP_TO_STAGE_END"):
                    raise PipelineError(f"invalid animation end policy: {pattern_id}/{stage_id}")
                integer(animation["repeatCount"], f"{pattern_id}/{stage_id}.repeatCount", 1, 32)
                if not animation["occurrences"]:
                    raise PipelineError(f"stage has no animation occurrence: {pattern_id}/{stage_id}")
                known_wall = 0.0
                loops = 0
                zero_play = 0
                for occurrence in animation["occurrences"]:
                    exact(
                        occurrence,
                        ("clipOccurrenceId", "clip", "mappingBasis", "sourceStartMs", "playMs", "playRate", "repeatUntilStageEnd"),
                        f"{pattern_id}/{stage_id}.occurrence",
                    )
                    stable_id(
                        occurrence["clip"],
                        f"{pattern_id}/{stage_id}.occurrence.clip",
                    )
                    mapping_basis(
                        occurrence["mappingBasis"],
                        f"{pattern_id}/{stage_id}.occurrence.mappingBasis",
                    )
                    integer(
                        occurrence["sourceStartMs"],
                        f"{pattern_id}/{stage_id}.occurrence.sourceStartMs",
                        0,
                        600000,
                    )
                    play_ms = integer(occurrence["playMs"], "occurrence playMs", 0, 600000)
                    play_rate = number(occurrence["playRate"], "occurrence playRate", 0.000001, 1000)
                    if play_ms == 0:
                        zero_play += 1
                    else:
                        known_wall += play_ms / play_rate
                    if boolean(occurrence["repeatUntilStageEnd"], "repeatUntilStageEnd"):
                        loops += 1
                if animation["endPolicy"] == "EXACT" and (zero_play or loops or abs(known_wall - duration) > 2.0):
                    raise PipelineError(f"EXACT animation budget mismatch: {pattern_id}/{stage_id}")
                if animation["endPolicy"] == "LOOP_TO_STAGE_END" and (zero_play != 1 or loops != 1 or known_wall >= duration):
                    raise PipelineError(f"LOOP animation budget mismatch: {pattern_id}/{stage_id}")
                if animation["endPolicy"] == "HOLD_LAST_POSE" and (loops or known_wall >= duration + 2.0):
                    raise PipelineError(f"HOLD animation budget mismatch: {pattern_id}/{stage_id}")
                occurrences_by_id = unique_index(
                    animation["occurrences"],
                    "clipOccurrenceId",
                    f"{pattern_id}/{stage_id} animation occurrences",
                )
            else:
                raise PipelineError(
                    f"unsupported animation mode: {pattern_id}/{stage_id}"
                )
            for event in stage["events"]:
                fields = _event_fields(event.get("kind"))
                if not fields:
                    raise PipelineError(f"unsupported typed event: {pattern_id}/{stage_id}/{event.get('kind')}")
                exact(event, fields, f"{pattern_id}/{stage_id}.event")
                event_id = stable_id(event["eventId"], f"{pattern_id}/{stage_id}.eventId")
                if event_id in event_ids or event["trigger"] not in ("ENTER", "EXIT"):
                    raise PipelineError(f"duplicate/invalid event: {event_id}")
                event_ids.add(event_id)
                if event["kind"] == "SET_BOSS_FLAG":
                    stable_id(event["flagId"], f"event {event_id}.flagId")
                    boolean(event["enabled"], f"event {event_id}.enabled")
                elif event["kind"] == "SPAWN_COMBAT_OBJECT":
                    archetype = stable_id(event["combatObjectArchetypeId"], f"event {event_id}.archetype")
                    if archetype not in combat_archetypes or event["trigger"] != "ENTER":
                        raise PipelineError(f"spawn event has unresolved archetype/trigger: {event_id}")
                    integer(event["count"], f"event {event_id}.count", 1, 1)
                    definition = next(row for row in combat_authoring["objects"]
                                      if row["combatObjectArchetypeId"] == archetype)
                    if "lifetimeMs" not in definition:
                        raise PipelineError(f"independent spawn requires explicit lifetimeMs: {event_id}")
                    spawn_events.add(event_id)
                elif event["kind"] == "SPAWN_COMBAT_OBJECT_VOLLEY":
                    archetype = stable_id(event["combatObjectArchetypeId"], f"event {event_id}.archetype")
                    if archetype not in combat_archetypes or event["volleyPolicy"] != "PER_ALIVE_PLAYER":
                        raise PipelineError(f"spawn event has unresolved/unsupported archetype: {event_id}")
                    count = integer(event["countPerResolvedTarget"], f"event {event_id}.count", 1, 8)
                    boolean(event["allowOverlap"], f"event {event_id}.allowOverlap")
                    spawn_count, _ = _validate_volley_spawn_schedule(
                        event["spawnSchedule"], duration, event_id
                    )
                    arena_random_count = _validate_volley_arena_random(
                        event["arenaRandom"], event_id
                    )
                    maximum_total_objects = integer(
                        event["maximumTotalObjects"],
                        f"event {event_id}.maximumTotalObjects",
                        1,
                        64,
                    )
                    minimum_total_objects = count + arena_random_count
                    if maximum_total_objects < minimum_total_objects:
                        raise PipelineError(
                            "volley maximumTotalObjects cannot admit one wave with "
                            f"one resolved target plus its arena-random objects: {event_id}"
                        )
                    if not isinstance(event["layout"], dict):
                        raise PipelineError(f"volley layout must be an object: {event_id}")
                    if event["allowOverlap"] is not False:
                        raise PipelineError(f"current Server volley contract forbids overlap: {event_id}")
                    if event["layout"].get("kind") == "TARGET_CENTER":
                        exact(event["layout"], ("kind",), f"event {event_id}.layout")
                        if count != 1:
                            raise PipelineError(f"TARGET_CENTER only admits count 1: {event_id}")
                    elif event["layout"].get("kind") == "RADIAL_AROUND_TARGET":
                        exact(
                            event["layout"],
                            ("kind", "radiusM", "startAngleDegrees", "angleStepDegrees"),
                            f"event {event_id}.layout",
                        )
                        if count < 2:
                            raise PipelineError(f"RADIAL_AROUND_TARGET requires count 2..8: {event_id}")
                        number(event["layout"]["radiusM"], f"event {event_id}.radiusM", 0.01, 1000)
                        number(
                            event["layout"]["startAngleDegrees"],
                            f"event {event_id}.startAngleDegrees",
                            -360000,
                            360000,
                        )
                        step = number(
                            event["layout"]["angleStepDegrees"],
                            f"event {event_id}.angleStepDegrees",
                            0.000001,
                            360,
                        )
                        if step * count > 360.000001:
                            raise PipelineError(f"radial volley wraps onto itself: {event_id}")
                    else:
                        raise PipelineError(f"unsupported volley layout: {event_id}")
                    spawn_events.add(event_id)
                elif event["kind"] == "SET_GAMEPLAY_PHASE":
                    gameplay_phase = integer(
                        event["gameplayPhase"], f"event {event_id}.gameplayPhase", 2, 3
                    )
                    gameplay_phase_events.append(
                        (pattern_id, stage_id, event["trigger"], gameplay_phase)
                    )
                elif event["kind"] == "TRIGGER_WORLD_EVENT_SET":
                    stable_id(event["worldEventSetId"], f"event {event_id}.worldEventSetId")
                    world_events.append((pattern_id, stage_id, event["worldEventSetId"]))
                elif event["kind"] in ("RETARGET_RANDOM_ALIVE", "RETURN_TO_ARENA_CENTER"):
                    if event["trigger"] != "ENTER":
                        raise PipelineError(
                            f"boss retarget event must run on ENTER: {event_id}"
                        )
                elif event["kind"] == "RELEASE_GRABBED_PLAYERS":
                    speed_mps = number(
                        event["speedMps"], f"event {event_id}.speedMps", 0, 50
                    )
                    release_duration_ms = integer(
                        event["durationMs"],
                        f"event {event_id}.durationMs",
                        0,
                        5000,
                    )
                    yaw_offset_degrees = number(
                        event["yawOffsetDegrees"],
                        f"event {event_id}.yawOffsetDegrees",
                        -180,
                        180,
                    )
                    hold = (
                        event["releaseMode"] == "HOLD"
                        and speed_mps == 0
                        and release_duration_ms == 0
                        and yaw_offset_degrees == 0
                    )
                    knockback = (
                        event["releaseMode"] in ("OPPOSITE_KNOCKBACK", "ARENA_EJECTION")
                        and speed_mps > 0
                        and release_duration_ms > 0
                        and (
                            event["releaseMode"] == "ARENA_EJECTION"
                            or yaw_offset_degrees == 0
                        )
                    )
                    if event["trigger"] not in ("ENTER", "EXIT") or not (
                        hold or knockback
                    ):
                        raise PipelineError(
                            f"grabbed-player release event is invalid: {event_id}"
                        )
                elif event["kind"] in (
                    "DAMAGE_GRABBED_PLAYERS", "EXECUTE_GRABBED_PLAYERS"
                ):
                    if event["trigger"] != "ENTER":
                        raise PipelineError(
                            f"grabbed-player impact must run on ENTER: {event_id}"
                        )
                    if event["kind"] == "DAMAGE_GRABBED_PLAYERS":
                        stable_id(event["damageProfileId"], f"{event_id}.damageProfileId")
                        if not event["damageProfileId"].startswith("damage."):
                            raise PipelineError(f"invalid grabbed damage profile: {event_id}")
            for cue in stage["effectCues"]:
                cue_timing_basis = cue.get(
                    "timingBasis", CUE_TIMING_BASIS_CLIP_OCCURRENCE
                ) if isinstance(cue, dict) else None
                cue_common_fields = (
                    "cueId", "occurrenceId", "effectAssetId",
                    "anchorSlotId", "followPolicy", "stopPolicy",
                    "repeatPolicy", "localTransform", "scalePolicy",
                )
                if cue_timing_basis == CUE_TIMING_BASIS_STAGE_CLOCK:
                    exact(
                        cue,
                        cue_common_fields + ("timingBasis", "stageOffsetMs"),
                        f"{pattern_id}/{stage_id}.effectCue",
                    )
                    if animation_mode != ANIMATION_MODE_NONE:
                        raise PipelineError(
                            "stage-clock cue requires NONE animation mode: "
                            f"{pattern_id}/{stage_id}"
                        )
                    integer(
                        cue["stageOffsetMs"],
                        f"{pattern_id}/{stage_id}.effectCue.stageOffsetMs",
                        0,
                        duration - 1,
                    )
                    if cue["repeatPolicy"] != "once":
                        raise PipelineError(
                            f"stage-clock cue must use once repeatPolicy: {pattern_id}/{stage_id}"
                        )
                elif cue_timing_basis == CUE_TIMING_BASIS_CLIP_OCCURRENCE:
                    cue_fields = cue_common_fields + (
                        "clipOccurrenceId", "sourceStartMs", "sourceEndMs",
                        "mappingBasis",
                    )
                    if "timingBasis" in cue:
                        cue_fields += ("timingBasis",)
                    exact(
                        cue,
                        cue_fields,
                        f"{pattern_id}/{stage_id}.effectCue",
                    )
                    validate_cue_animation_join(
                        cue,
                        occurrences_by_id,
                        f"{pattern_id}/{stage_id} cue {cue.get('cueId', '')}",
                    )
                else:
                    raise PipelineError(
                        f"unsupported Effect cue timing basis: {pattern_id}/{stage_id}"
                    )
                cue_id = stable_id(cue["cueId"], f"{pattern_id}/{stage_id}.cueId")
                anchor_slot = stable_id(cue["anchorSlotId"], f"{cue_id}.anchorSlotId")
                if anchor_slot.startswith("arena.center"):
                    motion = pattern.get("serverMotion")
                    if (anchor_slot not in ("arena.center", "arena.center.facing")
                            or not isinstance(motion, dict)
                            or motion.get("kind") != "LEAP_TO_ANCHOR"
                            or motion.get("moveToAnchorBeforeTakeoff") is not True
                            or cue["followPolicy"] != "snapshot"):
                        raise PipelineError(f"{cue_id} arena center anchor requires a fixed center approach")
                    if (anchor_slot == "arena.center.facing"
                            and (pattern["aimPolicy"] != "LOCK_FACING_ON_START"
                                 or pattern["targetPolicy"] != "LOCK_RANDOM_ALIVE_ON_START")):
                        raise PipelineError(f"{cue_id} facing anchor requires one locked random target")
                if cue_id in cue_ids:
                    raise PipelineError(f"duplicate managed cue: {cue_id}")
                scale_kind = validate_cue_scale_policy(
                    cue["scalePolicy"],
                    f"{pattern_id}/{stage_id} cue {cue_id}",
                )
                expected_scale_kind = expected_cue_policies.get(cue_id)
                if scale_kind != expected_scale_kind:
                    raise PipelineError(
                        f"managed cue scalePolicy migration mismatch: {cue_id} "
                        f"expected={expected_scale_kind} actual={scale_kind}"
                    )
                scale_policy_counts[scale_kind] += 1
                cue_ids.add(cue_id)
            for invocation in stage["cameraInvocations"]:
                exact(
                    invocation,
                    ("cameraInvocationId", "cameraCueId", "trigger", "startOffsetMs", "durationPolicy", "durationMs"),
                    f"{pattern_id}/{stage_id}.cameraInvocation",
                )
                if invocation["durationPolicy"] != "EXPLICIT":
                    raise PipelineError("initial camera migration uses EXPLICIT duration")
                integer(invocation["durationMs"], "camera invocation durationMs", 1, duration)
    if not cue_ids.issubset(expected_cue_policies):
        raise PipelineError("managed cue scalePolicy migration table coverage drift")
    live_expected_scale_policy_counts = Counter(
        expected_cue_policies[cue_id] for cue_id in cue_ids
    )
    if scale_policy_counts != live_expected_scale_policy_counts:
        raise PipelineError(
            "managed cue scalePolicy coverage drift: "
            f"expected={dict(live_expected_scale_policy_counts)} "
            f"actual={dict(scale_policy_counts)}"
        )
    expected_world_set_owners = (
        {WORLD_SET_ID: WORLD_SET_OWNERS[WORLD_SET_ID]}
        if migration_fixture
        else MANAGED_WORLD_SET_OWNERS
    )
    expected_world_events = {
        (pattern_id, stage_id, set_id)
        for set_id, (pattern_id, stage_id, _trigger_kind) in
        expected_world_set_owners.items()
    }
    if set(world_events) != expected_world_events or len(world_events) != len(
        expected_world_events
    ):
        raise PipelineError(
            "v2 master world-set invocation closure drift: "
            f"expected={sorted(expected_world_events)} actual={sorted(world_events)}"
        )
    expected_phase_event = [(WORLD_PATTERN_ID, WORLD_STAGE_ID, "ENTER", 2)]
    if gameplay_phase_events != expected_phase_event:
        raise PipelineError(
            "v2 master must own exactly one 109 IMPACT/ENTER gameplay phase event: "
            f"{gameplay_phase_events}"
        )
    independent_ids: set[str] = set()
    referenced_cues: set[str] = set()
    referenced_spawns: set[str] = set()
    for entry in master["independentEffects"]:
        common = ("independentEffectId", "displayName", "ownership")
        if entry.get("ownership") == "SERVER_PATTERN_STAGE":
            exact(entry, common + ("cueId",), "independent SERVER_PATTERN_STAGE")
            cue_id = entry["cueId"]
            if cue_id not in cue_ids or cue_id in referenced_cues:
                raise PipelineError(f"independent cue is unresolved/duplicate: {cue_id}")
            referenced_cues.add(cue_id)
        elif entry.get("ownership") == "SERVER_COMBAT_OBJECT":
            exact(entry, common + ("spawnEventId",), "independent SERVER_COMBAT_OBJECT")
            event_id = entry["spawnEventId"]
            if event_id not in spawn_events or event_id in referenced_spawns:
                raise PipelineError(f"independent spawn is unresolved/duplicate: {event_id}")
            referenced_spawns.add(event_id)
        else:
            raise PipelineError("independent effect ownership is unsupported")
        independent_id = stable_id(entry["independentEffectId"], "independentEffectId")
        if independent_id in independent_ids:
            raise PipelineError(f"duplicate independentEffectId: {independent_id}")
        independent_ids.add(independent_id)


def _validate_finite_pattern_graph(pattern: Mapping[str, Any]) -> None:
    """Require every stage to terminate; looping finales repeat occurrences, not edges."""
    stages = unique_index(pattern["stages"], "actionId", "finite pattern stages")
    active: set[str] = set()
    completed: set[str] = set()

    def visit(action_id: str) -> None:
        if action_id in active:
            raise PipelineError(f"pattern {pattern['patternId']} finite stage graph contains a cycle")
        if action_id in completed:
            return
        stage = stages.get(action_id)
        if stage is None:
            raise PipelineError(f"pattern {pattern['patternId']} finite stage graph has a dangling action")
        active.add(action_id)
        successors = [stage["defaultNextActionId"]] + [branch["nextActionId"] for branch in stage["branches"]]
        for target in successors:
            if target is not None:
                visit(target)
        active.remove(action_id)
        completed.add(action_id)

    for action_id in stages:
        visit(action_id)


def _validate_finale(pattern: Mapping[str, Any], patterns: Mapping[str, Any]) -> None:
    if pattern["patternId"] in ("VALTAN_TRASH", "VALTAN_TRASH_CATCH_IF",
                                 "VALTAN_TRASH_CATCH_SUCCESS", "VALTAN_TRASH_CATCH_FAIL"):
        _validate_finite_pattern_graph(pattern)
    if "finale" not in pattern:
        return
    context = f"pattern {pattern['patternId']}.finale"
    finale = pattern["finale"]
    if not isinstance(finale, dict):
        raise PipelineError(f"{context} must be an object")
    exact(finale, ("kind", "ghostArchetypeId", "ghostPatternIds",
                   "spawnHalfExtentsM", "maximumActiveGhosts"), context)
    if finale["kind"] != "GHOST_PORTAL_LOOP":
        raise PipelineError(f"{context} kind is unsupported")
    _validate_finite_pattern_graph(pattern)
    if stable_id(finale["ghostArchetypeId"], context) != "BOSS_VALTAN_GHOST":
        raise PipelineError(f"{context} requires the dependent ghost archetype")
    integer(finale["maximumActiveGhosts"], f"{context}.maximumActiveGhosts", 1, 1)
    extents = finale["spawnHalfExtentsM"]
    if not isinstance(extents, list) or len(extents) != 2:
        raise PipelineError(f"{context}.spawnHalfExtentsM requires X/Z")
    for value in extents:
        number(value, f"{context}.spawnHalfExtentsM", 1, 100)
    children = finale["ghostPatternIds"]
    if not isinstance(children, list) or len(children) != 3:
        raise PipelineError(f"{context}.ghostPatternIds requires three finite attacks")
    for child_id in children:
        stable_id(child_id, f"{context}.ghostPatternIds")
    if len(set(children)) != 3:
        raise PipelineError(f"{context}.ghostPatternIds has duplicate IDs")
    for child_id in children:
        stable_id(child_id, f"{context}.ghostPatternIds")
        child = patterns.get(child_id)
        if child is None or child_id == pattern["patternId"] or "finale" in child:
            raise PipelineError(f"{context} has unresolved/recursive child {child_id}")
        _validate_finite_pattern_graph(child)
        if any(event.get("kind") == "TRIGGER_WORLD_EVENT_SET"
               for stage in child["stages"] for event in stage["events"]):
            raise PipelineError(f"{context} cannot delegate terrain destruction")
    if children != ["VALTAN_WHIRLWIND", "VALTAN_FOUR_SLASH", "VALTAN_SEQUENCE_FOUR"]:
        raise PipelineError(f"{context} requires the reviewed whirlwind/four-slash/four-direction order")
    if pattern.get("invulnerableWhileRunning"):
        raise PipelineError(f"{context} must remain damageable until owner death")


def validate_gameplay_authoring(
    document: dict[str, Any],
    flow_document: dict[str, Any] | None = None,
) -> None:
    document = resolve_gameplay_flow_reference(document, flow_document)
    exact(
        document,
        (
            "schema",
            "formatVersion",
            "bossArchetypeId",
            "encounterId",
            "scope",
            "previewPaths",
            "retiredPatternIds",
            "decisionModel",
            "counterReactionLayers",
            "patterns",
        ),
        "Valtan gameplay authoring root",
    )
    if (
        document["schema"] != "lostark.valtan-gameplay-authoring"
        or document["formatVersion"] != 1
        or document["bossArchetypeId"] != "BOSS_VALTAN"
        or document["encounterId"] != "ENCOUNTER_VALTAN"
        or document["scope"] != "PHASE_ONE"
    ):
        raise PipelineError("Valtan gameplay authoring header mismatch")
    patterns = unique_index(document["patterns"], "patternId", "gameplay patterns")
    decision_model = document.get("decisionModel")
    exact(
        decision_model,
        (
            "scriptedSequence",
            "selectionSets",
            "selectionWindows",
            "mechanics",
            "manualAuditions",
        ),
        "Valtan gameplay decisionModel",
    )
    manual_patterns = _validate_manual_auditions(decision_model, patterns)
    _validate_scripted_sequence(decision_model, patterns)
    mechanics = decision_model.get("mechanics")
    if not isinstance(mechanics, list):
        raise PipelineError("Valtan gameplay decisionModel.mechanics must be an array")
    mechanic_patterns = {
        stable_id(row.get("patternId"), "gameplay mechanic patternId")
        for row in mechanics
        if isinstance(row, dict)
    }
    if len(mechanic_patterns) != len(mechanics):
        raise PipelineError("Valtan gameplay mechanic pattern ownership is invalid")
    for pattern_id, pattern in patterns.items():
        exact(
            pattern,
            (
                "patternId",
                "displayName",
                "category",
                "compatibilitySelectionWeight",
                "actionId",
                "entryActionId",
                "targetPolicy",
                "aimPolicy",
                "eligibility",
                "invulnerableWhileRunning",
                "sourceActionIds",
                "serverMotion",
                "reactions",
                "stages",
            ) + (("finale",) if "finale" in pattern else ()),
            f"gameplay pattern {pattern_id}",
        )
        _validate_finale(pattern, patterns)
        _validate_pattern_target_aim(
            pattern, f"gameplay pattern {pattern_id}"
        )
        compatibility_weight = integer(
            pattern["compatibilitySelectionWeight"],
            f"pattern {pattern_id}.compatibilitySelectionWeight",
            0,
            100000,
        )
        is_non_rotation = (
            pattern_id in mechanic_patterns or pattern_id in manual_patterns
        )
        if (is_non_rotation and compatibility_weight != 0) or (
            not is_non_rotation and compatibility_weight == 0
        ):
            raise PipelineError(
                "compatibilitySelectionWeight must be positive only for normal "
                f"patterns: {pattern_id}"
            )
        stable_id(pattern["actionId"], f"gameplay pattern {pattern_id}.actionId")
        if not isinstance(pattern["sourceActionIds"], list) or not pattern["sourceActionIds"]:
            raise PipelineError(f"gameplay pattern sourceActionIds is empty: {pattern_id}")
        source_action_ids = [
            integer(value, f"gameplay pattern {pattern_id}.sourceActionIds", 1)
            for value in pattern["sourceActionIds"]
        ]
        if len(source_action_ids) != len(set(source_action_ids)):
            raise PipelineError(f"gameplay pattern sourceActionIds is duplicate: {pattern_id}")
        stages = unique_index(pattern["stages"], "stageId", f"gameplay pattern {pattern_id} stages")
        server_motion = pattern["serverMotion"]
        if server_motion is not None:
            exact(
                server_motion,
                (
                    "kind",
                    "anchorId",
                    "landingPosition",
                    "apexHeight",
                    "travelStageId",
                    "takeoffStartMs",
                    "takeoffEndMs",
                    "travelStartMs",
                    "travelEndMs",
                ) + (("moveToAnchorBeforeTakeoff",) if "moveToAnchorBeforeTakeoff" in server_motion else ()),
                f"gameplay pattern {pattern_id}.serverMotion",
            )
            if "moveToAnchorBeforeTakeoff" in server_motion:
                boolean(server_motion["moveToAnchorBeforeTakeoff"],
                        f"gameplay pattern {pattern_id}.serverMotion.moveToAnchorBeforeTakeoff")
                if server_motion["moveToAnchorBeforeTakeoff"] and server_motion["takeoffStartMs"] <= 0:
                    raise PipelineError(f"center approach requires a positive anticipation window: {pattern_id}")
            if server_motion["kind"] not in {"LEAP_TO_ANCHOR", "LEAP_TO_TARGET"}:
                raise PipelineError(
                    f"unsupported serverMotion kind: {pattern_id}"
                )
            stable_id(
                server_motion["anchorId"],
                f"gameplay pattern {pattern_id}.serverMotion.anchorId",
            )
            travel_stage_id = stable_id(
                server_motion["travelStageId"],
                f"gameplay pattern {pattern_id}.serverMotion.travelStageId",
            )
            if travel_stage_id not in stages or not pattern["stages"]:
                raise PipelineError(
                    f"serverMotion travel stage is missing: {pattern_id}/{travel_stage_id}"
                )
            if (
                pattern["stages"][0].get("actionId")
                != pattern["entryActionId"]
            ):
                raise PipelineError(
                    f"serverMotion first stage must own entryActionId: {pattern_id}"
                )
            travel_stage_index = next(
                (
                    index
                    for index, stage in enumerate(pattern["stages"])
                    if stage.get("stageId") == travel_stage_id
                ),
                -1,
            )
            if travel_stage_index <= 0:
                raise PipelineError(
                    f"serverMotion travel stage must follow the entry stage: "
                    f"{pattern_id}/{travel_stage_id}"
                )
            landing = server_motion["landingPosition"]
            if not isinstance(landing, list) or len(landing) != 3:
                raise PipelineError(
                    f"serverMotion landingPosition is invalid: {pattern_id}"
                )
            for coordinate in landing:
                number(coordinate, f"gameplay pattern {pattern_id}.landingPosition", -100000, 100000)
            number(
                server_motion["apexHeight"],
                f"gameplay pattern {pattern_id}.apexHeight",
                0.000001,
                200,
            )
            takeoff_start = integer(
                server_motion["takeoffStartMs"],
                f"gameplay pattern {pattern_id}.takeoffStartMs",
                0,
            )
            takeoff_end = integer(
                server_motion["takeoffEndMs"],
                f"gameplay pattern {pattern_id}.takeoffEndMs",
                1,
            )
            travel_start = integer(
                server_motion["travelStartMs"],
                f"gameplay pattern {pattern_id}.travelStartMs",
                0,
            )
            travel_end = integer(
                server_motion["travelEndMs"],
                f"gameplay pattern {pattern_id}.travelEndMs",
                1,
            )
            takeoff_duration = integer(
                pattern["stages"][0]["durationMs"],
                f"gameplay pattern {pattern_id} TAKEOFF durationMs",
                1,
            )
            travel_duration = integer(
                stages[travel_stage_id]["durationMs"],
                f"gameplay pattern {pattern_id} travel durationMs",
                1,
            )
            if (
                takeoff_start >= takeoff_end
                or takeoff_end > takeoff_duration
                or travel_start >= travel_end
                or travel_end > travel_duration
            ):
                raise PipelineError(
                    f"serverMotion travel window is outside its stage: {pattern_id}"
                )
        action_ids: set[str] = set()
        for stage_id, stage in stages.items():
            exact(
                stage,
                _pattern_stage_fields(stage, joined=False),
                f"gameplay pattern {pattern_id} stage {stage_id}",
            )
            _validate_pattern_stage_extensions(
                stage, f"gameplay pattern {pattern_id} stage {stage_id}"
            )
            action_id = stable_id(
                stage["actionId"],
                f"gameplay pattern {pattern_id} stage {stage_id}.actionId",
            )
            if action_id in action_ids:
                raise PipelineError(
                    f"gameplay stage actionId is duplicate: {pattern_id}/{action_id}"
                )
            action_ids.add(action_id)
            hit = stage["hit"]
            if not isinstance(hit, dict):
                raise PipelineError(
                    f"gameplay stage hit must be an object: {pattern_id}/{stage_id}"
                )
            has_player_response = "playerResponse" in hit
            has_attachment_slot = "attachmentSlot" in hit
            if has_player_response != has_attachment_slot:
                raise PipelineError(
                    "capture hit requires playerResponse and attachmentSlot together: "
                    f"{pattern_id}/{stage_id}"
                )
            if has_player_response and (
                hit["playerResponse"] != "CAPTURE"
                or hit["attachmentSlot"] != "BOSS_LEFT_HAND"
            ):
                raise PipelineError(
                    "capture hit requires the typed boss-left-hand slot: "
                    f"{pattern_id}/{stage_id}"
                )

    warp = patterns.get("VALTAN_WARP")
    if warp is not None:
        warp_stages = {stage["stageId"]: stage for stage in warp["stages"]}
        for leg in range(8):
            stage_id = f"STEP_{leg + 2:02d}"
            stage = warp_stages.get(stage_id)
            if stage is None or stage.get("motion") != {"kind": "PORTAL_TARGET_RUSH"}:
                raise PipelineError(
                    f"VALTAN_WARP requires target-rush motion: {stage_id}"
                )

    finale = patterns.get("VALTAN_GHOST_FINALE")
    if finale is not None:
        finale_stages = {stage["stageId"]: stage for stage in finale["stages"]}
        for leg in range(8):
            stage_id = f"STEP_{leg + 2:02d}"
            expected_motion = {
                "kind": "PORTAL_CROSS_ARENA",
                "cornerIndex": leg % 4,
                "halfExtentsM": [22.0, 22.0],
            }
            stage = finale_stages.get(stage_id)
            if stage is None or stage.get("motion") != expected_motion:
                raise PipelineError(
                    f"VALTAN_GHOST_FINALE requires corner portal motion: {stage_id}"
                )


def validate_presentation_authoring(document: dict[str, Any]) -> None:
    exact(
        document,
        (
            "schema",
            "formatVersion",
            "bossArchetypeId",
            "encounterId",
            "scope",
            "patterns",
            "independentEffects",
        ),
        "Valtan presentation authoring root",
    )
    if (
        document["schema"] != "lostark.valtan-pattern-presentation-authoring"
        or document["formatVersion"] != 1
        or document["bossArchetypeId"] != "BOSS_VALTAN"
        or document["encounterId"] != "ENCOUNTER_VALTAN"
        or document["scope"] != "PHASE_ONE"
    ):
        raise PipelineError("Valtan presentation authoring header mismatch")
    patterns = unique_index(
        document["patterns"], "patternId", "presentation patterns"
    )
    for pattern_id, pattern in patterns.items():
        exact(
            pattern,
            (
                "patternId",
                "sourceSequenceIndex",
                "presentationSources",
                "stages",
            ),
            f"presentation pattern {pattern_id}",
        )
        integer(
            pattern["sourceSequenceIndex"],
            f"presentation pattern {pattern_id}.sourceSequenceIndex",
            0,
            4096,
        )
        if not isinstance(pattern["presentationSources"], list):
            raise PipelineError(
                f"presentationSources must be an array: {pattern_id}"
            )
        for source_ordinal, source in enumerate(pattern["presentationSources"]):
            exact(
                source,
                ("sourceActionId", "sequenceIndex", "role"),
                f"presentation pattern {pattern_id} source[{source_ordinal}]",
            )
            integer(
                source["sourceActionId"],
                f"presentation pattern {pattern_id} sourceActionId",
                1,
            )
            integer(
                source["sequenceIndex"],
                f"presentation pattern {pattern_id} sequenceIndex",
                0,
                4096,
            )
            stable_id(
                source["role"], f"presentation pattern {pattern_id} source role"
            )
        stages = unique_index(
            pattern["stages"], "stageId", f"presentation pattern {pattern_id} stages"
        )
        action_ids: set[str] = set()
        for stage_id, stage in stages.items():
            exact(
                stage,
                (
                    "stageId",
                    "actionId",
                    "sequenceRole",
                    "animation",
                    "effectCues",
                    "cameraInvocations",
                ),
                f"presentation pattern {pattern_id} stage {stage_id}",
            )
            action_id = stable_id(
                stage["actionId"],
                f"presentation pattern {pattern_id} stage {stage_id}.actionId",
            )
            if action_id in action_ids:
                raise PipelineError(
                    f"presentation stage actionId is duplicate: {pattern_id}/{action_id}"
                )
            action_ids.add(action_id)


def _validate_gameplay_only_events(gameplay: dict[str, Any]) -> None:
    volley_owners = []
    phase_owners = []
    for pattern in gameplay["patterns"]:
        for stage in pattern["stages"]:
            for event in stage["events"]:
                if event.get("kind") == "SPAWN_COMBAT_OBJECT_VOLLEY":
                    volley_owners.append(
                        (
                            pattern["patternId"],
                            stage["stageId"],
                            event.get("trigger"),
                            event.get("combatObjectArchetypeId"),
                        )
                    )
                elif event.get("kind") == "SET_GAMEPLAY_PHASE":
                    phase_owners.append(
                        (
                            pattern["patternId"],
                            stage["stageId"],
                            event.get("trigger"),
                            event.get("gameplayPhase"),
                        )
                    )
    if volley_owners != [
        (
            "VALTAN_HIGH_JUMP",
            "AIRBORNE",
            "ENTER",
            "combatobject.valtan.high-jump.target-axe",
        )
    ]:
        raise PipelineError(
            "Valtan gameplay authoring must own exactly one "
            "HIGH_JUMP/AIRBORNE mixed axe volley: "
            f"{volley_owners}"
        )
    if phase_owners != [(WORLD_PATTERN_ID, WORLD_STAGE_ID, "ENTER", 2)]:
        raise PipelineError(
            "Valtan gameplay authoring must own exactly one ARENA_BREAK_109/IMPACT phase event: "
            f"{phase_owners}"
        )


def split_v2_authoring(
    master: dict[str, Any],
    world_sets: dict[str, Any],
    combat_authoring: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any]]:
    """Split the strict internal v2 IR into its two physical authoring roles."""

    validate_v2_master(master, world_sets, combat_authoring)
    gameplay_patterns = []
    presentation_patterns = []
    for pattern in master["patterns"]:
        gameplay_stages = []
        presentation_stages = []
        for stage in pattern["stages"]:
            gameplay_stage = {
                "stageId": stage["stageId"],
                "actionId": stage["actionId"],
                "stageKind": stage["stageKind"],
                "durationMs": stage["durationMs"],
                "defaultNextActionId": copy.deepcopy(
                    stage["defaultNextActionId"]
                ),
                "hit": copy.deepcopy(stage["hit"]),
                "motion": copy.deepcopy(stage["motion"]),
                "events": copy.deepcopy(stage["events"]),
                "branches": copy.deepcopy(stage["branches"]),
            }
            for optional in ("partDamagePolicy", "counterProxy"):
                if optional in stage:
                    gameplay_stage[optional] = copy.deepcopy(stage[optional])
            gameplay_stages.append(gameplay_stage)
            presentation_stages.append(
                {
                    "stageId": stage["stageId"],
                    "actionId": stage["actionId"],
                    "sequenceRole": stage["sequenceRole"],
                    "animation": copy.deepcopy(stage["animation"]),
                    "effectCues": copy.deepcopy(stage["effectCues"]),
                    "cameraInvocations": copy.deepcopy(
                        stage["cameraInvocations"]
                    ),
                }
            )
        gameplay_patterns.append(
            {
                "patternId": pattern["patternId"],
                "displayName": pattern["displayName"],
                "category": pattern["category"],
                "compatibilitySelectionWeight": pattern[
                    "compatibilitySelectionWeight"
                ],
                "actionId": pattern["actionId"],
                "entryActionId": pattern["entryActionId"],
                "targetPolicy": pattern["targetPolicy"],
                "aimPolicy": pattern["aimPolicy"],
                "eligibility": copy.deepcopy(pattern["eligibility"]),
                "invulnerableWhileRunning": pattern["invulnerableWhileRunning"],
                "sourceActionIds": copy.deepcopy(pattern["sourceActionIds"]),
                "serverMotion": copy.deepcopy(pattern["serverMotion"]),
                "reactions": copy.deepcopy(pattern["reactions"]),
                "stages": gameplay_stages,
            }
        )
        if "finale" in pattern:
            gameplay_patterns[-1]["finale"] = copy.deepcopy(pattern["finale"])
        presentation_patterns.append(
            {
                "patternId": pattern["patternId"],
                "sourceSequenceIndex": pattern["sourceSequenceIndex"],
                "presentationSources": copy.deepcopy(
                    pattern["presentationSources"]
                ),
                "stages": presentation_stages,
            }
        )
    gameplay = {
        "schema": "lostark.valtan-gameplay-authoring",
        "formatVersion": 1,
        "bossArchetypeId": master["bossArchetypeId"],
        "encounterId": master["encounterId"],
        "scope": master["scope"],
        "previewPaths": copy.deepcopy(master["previewPaths"]),
        "retiredPatternIds": copy.deepcopy(master["retiredPatternIds"]),
        "decisionModel": copy.deepcopy(master["decisionModel"]),
        "counterReactionLayers": copy.deepcopy(master["counterReactionLayers"]),
        "patterns": gameplay_patterns,
    }
    presentation = {
        "schema": "lostark.valtan-pattern-presentation-authoring",
        "formatVersion": 1,
        "bossArchetypeId": master["bossArchetypeId"],
        "encounterId": master["encounterId"],
        "scope": master["scope"],
        "patterns": presentation_patterns,
        "independentEffects": copy.deepcopy(master["independentEffects"]),
    }
    validate_gameplay_authoring(gameplay)
    validate_presentation_authoring(presentation)
    _validate_gameplay_only_events(gameplay)
    return gameplay, presentation


def join_v2_authoring(
    gameplay: dict[str, Any],
    presentation: dict[str, Any],
    world_sets: dict[str, Any],
    combat_authoring: dict[str, Any],
    flow_document: dict[str, Any] | None = None,
) -> dict[str, Any]:
    """Strictly join the two physical authoring roles into the internal v2 IR."""

    gameplay = resolve_gameplay_flow_reference(gameplay, flow_document)
    validate_gameplay_authoring(gameplay)
    validate_presentation_authoring(presentation)
    for field in ("bossArchetypeId", "encounterId", "scope"):
        if gameplay[field] != presentation[field]:
            raise PipelineError(f"split authoring root identity mismatch: {field}")
    gameplay_pattern_ids = [row["patternId"] for row in gameplay["patterns"]]
    presentation_pattern_ids = [
        row["patternId"] for row in presentation["patterns"]
    ]
    if gameplay_pattern_ids != presentation_pattern_ids:
        raise PipelineError(
            "split authoring patternId order/closure mismatch: "
            f"gameplay={gameplay_pattern_ids} presentation={presentation_pattern_ids}"
        )
    joined_patterns = []
    for gameplay_pattern, presentation_pattern in zip(
        gameplay["patterns"], presentation["patterns"]
    ):
        pattern_id = gameplay_pattern["patternId"]
        primary_sources = [
            row
            for row in presentation_pattern["presentationSources"]
            if row["role"] == "PRIMARY"
        ]
        expected_primary = (
            gameplay_pattern["sourceActionIds"][0],
            presentation_pattern["sourceSequenceIndex"],
        )
        if len(primary_sources) != 1 or (
            primary_sources[0]["sourceActionId"],
            primary_sources[0]["sequenceIndex"],
        ) != expected_primary:
            raise PipelineError(
                f"split authoring primary presentation source mismatch: {pattern_id}"
            )
        gameplay_stage_ids = [row["stageId"] for row in gameplay_pattern["stages"]]
        presentation_stage_ids = [
            row["stageId"] for row in presentation_pattern["stages"]
        ]
        if gameplay_stage_ids != presentation_stage_ids:
            raise PipelineError(
                f"split authoring stageId order/closure mismatch: {pattern_id}"
            )
        joined_stages = []
        for gameplay_stage, presentation_stage in zip(
            gameplay_pattern["stages"], presentation_pattern["stages"]
        ):
            stage_id = gameplay_stage["stageId"]
            if gameplay_stage["actionId"] != presentation_stage["actionId"]:
                raise PipelineError(
                    f"split authoring actionId mismatch: {pattern_id}/{stage_id}"
                )
            joined_stage = {
                "stageId": stage_id,
                "sequenceRole": presentation_stage["sequenceRole"],
                "actionId": gameplay_stage["actionId"],
                "stageKind": gameplay_stage["stageKind"],
                "durationMs": gameplay_stage["durationMs"],
                "defaultNextActionId": copy.deepcopy(
                    gameplay_stage["defaultNextActionId"]
                ),
                "hit": copy.deepcopy(gameplay_stage["hit"]),
                "motion": copy.deepcopy(gameplay_stage["motion"]),
                "events": copy.deepcopy(gameplay_stage["events"]),
                "branches": copy.deepcopy(gameplay_stage["branches"]),
                "animation": copy.deepcopy(presentation_stage["animation"]),
                "effectCues": copy.deepcopy(
                    presentation_stage["effectCues"]
                ),
                "cameraInvocations": copy.deepcopy(
                    presentation_stage["cameraInvocations"]
                ),
            }
            for optional in ("partDamagePolicy", "counterProxy"):
                if optional in gameplay_stage:
                    joined_stage[optional] = copy.deepcopy(
                        gameplay_stage[optional]
                    )
            joined_stages.append(joined_stage)
        joined_patterns.append(
            {
                "patternId": pattern_id,
                "displayName": gameplay_pattern["displayName"],
                "category": gameplay_pattern["category"],
                "compatibilitySelectionWeight": gameplay_pattern[
                    "compatibilitySelectionWeight"
                ],
                "actionId": gameplay_pattern["actionId"],
                "entryActionId": gameplay_pattern["entryActionId"],
                "targetPolicy": gameplay_pattern["targetPolicy"],
                "aimPolicy": gameplay_pattern["aimPolicy"],
                "eligibility": copy.deepcopy(gameplay_pattern["eligibility"]),
                "invulnerableWhileRunning": gameplay_pattern[
                    "invulnerableWhileRunning"
                ],
                "sourceActionIds": copy.deepcopy(
                    gameplay_pattern["sourceActionIds"]
                ),
                "sourceSequenceIndex": presentation_pattern[
                    "sourceSequenceIndex"
                ],
                "presentationSources": copy.deepcopy(
                    presentation_pattern["presentationSources"]
                ),
                "serverMotion": copy.deepcopy(gameplay_pattern["serverMotion"]),
                "reactions": copy.deepcopy(gameplay_pattern["reactions"]),
                "stages": joined_stages,
            }
        )
        if "finale" in gameplay_pattern:
            joined_patterns[-1]["finale"] = copy.deepcopy(gameplay_pattern["finale"])
    joined = {
        "schema": "lostark.valtan-pattern-master",
        "formatVersion": 2,
        "bossArchetypeId": gameplay["bossArchetypeId"],
        "encounterId": gameplay["encounterId"],
        "scope": gameplay["scope"],
        "previewPaths": copy.deepcopy(gameplay["previewPaths"]),
        "retiredPatternIds": copy.deepcopy(gameplay["retiredPatternIds"]),
        "decisionModel": copy.deepcopy(gameplay["decisionModel"]),
        "counterReactionLayers": copy.deepcopy(
            gameplay["counterReactionLayers"]
        ),
        "independentEffects": copy.deepcopy(presentation["independentEffects"]),
        "patterns": joined_patterns,
    }
    _validate_gameplay_only_events(gameplay)
    validate_v2_master(joined, world_sets, combat_authoring)
    return joined


def _compile_hit(hit: dict[str, Any]) -> dict[str, Any]:
    result = _flat_from_shape(hit["shape"])
    if hit["shape"]["kind"] == "NONE":
        result.update(
            {
                "hitCount": 0,
                "hitIntervalMs": 0,
                "hitDelayMs": 0,
                "hitOffsetsMs": [],
                "serverDamageProfileId": "",
                "pushRangeM": 0.0,
                "pushMs": 0,
                "knockdown": False,
                "downMs": 0,
            }
        )
        return result
    schedule = hit["schedule"]
    if schedule["kind"] == "EXPLICIT_OFFSETS":
        offsets = copy.deepcopy(schedule["offsetsMs"])
        hit_offsets = offsets
        hit_delay = 0
        hit_count = len(offsets)
        hit_interval = 0
    else:
        hit_count = schedule["count"]
        hit_delay = schedule["firstOffsetMs"]
        hit_interval = schedule["intervalMs"]
        hit_offsets = []
    result.update(
        {
            "hitCount": hit_count,
            "hitIntervalMs": hit_interval,
            "hitDelayMs": hit_delay,
            "hitOffsetsMs": hit_offsets,
            "serverDamageProfileId": hit["serverDamageProfileId"],
            "pushRangeM": hit["pushRangeM"],
            "pushMs": hit["pushMs"],
            "knockdown": hit["knockdown"],
            "downMs": hit["downMs"],
        }
    )
    if "playerResponse" in hit:
        result["playerResponse"] = hit["playerResponse"]
        result["attachmentSlot"] = hit["attachmentSlot"]
    return result


def _compile_event(
    event: dict[str, Any], pattern_id: str, stage_id: str
) -> dict[str, Any] | None:
    if event["kind"] == "SET_BOSS_FLAG":
        return {
            "trigger": event["trigger"],
            "kind": "SET_BOSS_FLAG",
            "targetId": event["flagId"],
            "value": 1 if event["enabled"] else 0,
            "durationMs": 0,
        }
    if event["kind"] == "SPAWN_COMBAT_OBJECT":
        return {
            "trigger": event["trigger"],
            "kind": "SPAWN_COMBAT_OBJECT",
            "targetId": event["combatObjectArchetypeId"],
            "value": event["count"],
            "durationMs": 0,
        }
    if event["kind"] == "SPAWN_COMBAT_OBJECT_VOLLEY":
        layout = event["layout"]
        spawn_schedule = event["spawnSchedule"]
        arena_random = event["arenaRandom"]
        if layout["kind"] == "TARGET_CENTER":
            product_layout = "SINGLE"
            radius_m = 0
            start_angle_degrees = 0
            angle_step_degrees = 0
        elif layout["kind"] == "RADIAL_AROUND_TARGET":
            product_layout = "RADIAL"
            radius_m = layout["radiusM"]
            start_angle_degrees = layout["startAngleDegrees"]
            angle_step_degrees = layout["angleStepDegrees"]
        else:
            raise PipelineError(
                f"volley layout cannot compile to v18 Product: {layout.get('kind')}"
            )
        return {
            "trigger": event["trigger"],
            "kind": "SPAWN_COMBAT_OBJECT_VOLLEY",
            "targetId": event["combatObjectArchetypeId"],
            "targetingPolicy": event["volleyPolicy"],
            "countPerResolvedTarget": event["countPerResolvedTarget"],
            "layout": product_layout,
            "radiusM": radius_m,
            "startAngleDegrees": start_angle_degrees,
            "angleStepDegrees": angle_step_degrees,
            "allowOverlap": event["allowOverlap"],
            "maximumTotalObjects": event["maximumTotalObjects"],
            "spawnCount": spawn_schedule["count"],
            "spawnIntervalMs": spawn_schedule["intervalMs"],
            "arenaRandomCount": arena_random["count"],
            "arenaRandomRadiusM": arena_random["radiusM"],
            "arenaHeightToleranceM": arena_random["heightToleranceM"],
            "arenaAnchorPolicy": arena_random["anchor"],
        }
    if event["kind"] == "SET_GAMEPLAY_PHASE":
        return {
            "trigger": event["trigger"],
            "kind": "SET_GAMEPLAY_PHASE",
            "targetId": "boss.phase.gameplay",
            "value": event["gameplayPhase"],
            "durationMs": 0,
        }
    if event["kind"] == "TRIGGER_WORLD_EVENT_SET":
        return None
    if event["kind"] in ("RETARGET_RANDOM_ALIVE", "RETURN_TO_ARENA_CENTER"):
        return {
            "trigger": event["trigger"],
            "kind": event["kind"],
            "targetId": "boss.arena.center" if event["kind"] == "RETURN_TO_ARENA_CENTER" else "boss.target.pattern",
            "value": 1,
            "durationMs": 0,
        }
    if event["kind"] == "RELEASE_GRABBED_PLAYERS":
        return {
            "trigger": event["trigger"],
            "kind": "RELEASE_GRABBED_PLAYERS",
            "targetId": "boss.attachment.left-hand",
            "releaseMode": event["releaseMode"],
            "speedMps": event["speedMps"],
            "durationMs": event["durationMs"],
            "yawOffsetDegrees": event["yawOffsetDegrees"],
        }
    if event["kind"] in ("DAMAGE_GRABBED_PLAYERS", "EXECUTE_GRABBED_PLAYERS"):
        return {
            "trigger": event["trigger"],
            "kind": event["kind"],
            "targetId": event.get("damageProfileId", "boss.attachment.left-hand"),
            "value": 0,
            "durationMs": 0,
        }
    raise PipelineError(f"event cannot compile to current Product: {event['kind']}")


def compile_pattern_product(
    master: dict[str, Any],
    pattern: dict[str, Any],
) -> dict[str, Any]:
    mechanic_by_pattern = {
        row["patternId"]: row for row in master["decisionModel"]["mechanics"]
    }
    manual_pattern_ids = {
        row["patternId"] for row in master["decisionModel"]["manualAuditions"]
    }
    eligibility = pattern["eligibility"]
    mechanic = mechanic_by_pattern.get(pattern["patternId"])
    is_manual_audition = pattern["patternId"] in manual_pattern_ids
    projected_stages = []
    for stage in pattern["stages"]:
        projected = {
            "stageId": stage["stageId"],
            "actionId": stage["actionId"],
            "stageKind": stage["stageKind"],
            "durationMs": stage["durationMs"],
        }
        projected.update(_compile_hit(stage["hit"]))
        if projected.get("hitOffsetsMs") == []:
            del projected["hitOffsetsMs"]
        if stage["motion"] is not None:
            projected["motion"] = copy.deepcopy(stage["motion"])
        actions = [
            compiled
            for event in stage["events"]
            if (
                compiled := _compile_event(
                    event, pattern["patternId"], stage["stageId"]
                )
            )
            is not None
        ]
        if actions:
            projected["actions"] = actions
        if stage["branches"]:
            projected["branches"] = copy.deepcopy(stage["branches"])
        for optional in ("partDamagePolicy", "counterProxy"):
            if optional in stage:
                projected[optional] = copy.deepcopy(stage[optional])
        projected_stages.append(projected)
    if is_manual_audition:
        selection_fields = {
            "selectionMode": AUDITION_ONLY,
            "minimumPhase": eligibility["minimumGameplayPhase"],
            "maximumPhase": eligibility["maximumGameplayPhase"],
            "minimumHealthBar": 0,
            "maximumHealthBar": 0,
            "triggerHealthBar": 0,
            "triggerOrder": 0,
            "armorRequirement": "ANY",
            "phaseRequirement": "ANY",
            "selectionWeight": 0,
            "maximumConsecutiveUses": 0,
            "minimumRange": eligibility["minimumRangeM"],
            "maximumRange": eligibility["maximumRangeM"],
        }
    else:
        selection_fields = {
            "selectionMode": "HEALTH_BAR" if mechanic else "NORMAL",
            "minimumPhase": eligibility["minimumGameplayPhase"],
            "maximumPhase": eligibility["maximumGameplayPhase"],
            "minimumHealthBar": eligibility["minimumHealthBarInclusive"],
            "maximumHealthBar": eligibility["maximumHealthBarInclusive"],
            "triggerHealthBar": mechanic["trigger"]["healthBar"] if mechanic else 0,
            "triggerOrder": mechanic["triggerOrder"] if mechanic else 0,
            "armorRequirement": eligibility["armorRequirement"],
            "phaseRequirement": eligibility["phaseRequirement"],
            # Encounter v4's flat weight remains an explicit legacy/global fallback.
            # Per-window authoring must never rewrite it through an arbitrary set.
            "selectionWeight": 0
            if mechanic
            else integer(
                pattern["compatibilitySelectionWeight"],
                f"compatibility selection weight {pattern['patternId']}",
                1,
                100000,
            ),
            "maximumConsecutiveUses": eligibility["repeatPolicy"]["limit"],
            "minimumRange": eligibility["minimumRangeM"],
            "maximumRange": eligibility["maximumRangeM"],
        }
    result = {
        "patternId": pattern["patternId"],
        "category": pattern["category"],
        "minimumPhase": selection_fields["minimumPhase"],
        "maximumPhase": selection_fields["maximumPhase"],
        "targetPolicy": pattern["targetPolicy"],
        "aimPolicy": pattern["aimPolicy"],
        "displayName": pattern["displayName"],
        "actionId": pattern["actionId"],
        "sourceActionIds": copy.deepcopy(pattern["sourceActionIds"]),
        "selectionMode": selection_fields["selectionMode"],
        "minimumHealthBar": selection_fields["minimumHealthBar"],
        "maximumHealthBar": selection_fields["maximumHealthBar"],
        "triggerHealthBar": selection_fields["triggerHealthBar"],
        "triggerOrder": selection_fields["triggerOrder"],
        "armorRequirement": selection_fields["armorRequirement"],
        "phaseRequirement": selection_fields["phaseRequirement"],
        "invulnerableWhileRunning": pattern["invulnerableWhileRunning"],
        "selectionWeight": selection_fields["selectionWeight"],
        "maximumConsecutiveUses": selection_fields["maximumConsecutiveUses"],
        "minimumRange": selection_fields["minimumRange"],
        "maximumRange": selection_fields["maximumRange"],
    }
    if pattern["serverMotion"] is not None:
        result["serverMotion"] = copy.deepcopy(pattern["serverMotion"])
    if "finale" in pattern:
        result["finale"] = copy.deepcopy(pattern["finale"])
    result["stages"] = projected_stages
    return result


def compile_binding(stage: dict[str, Any]) -> dict[str, Any]:
    animation = stage["animation"]
    if animation.get("mode", ANIMATION_MODE_CLIP_SEQUENCE) == ANIMATION_MODE_NONE:
        return {
            "actionId": stage["actionId"],
            "playbackMode": ANIMATION_MODE_NONE,
            "clips": [],
        }
    return {
        "actionId": stage["actionId"],
        "clips": [
            {
                "clipOccurrenceId": occurrence["clipOccurrenceId"],
                "clip": occurrence["clip"],
                "mappingBasis": occurrence["mappingBasis"],
                "sourceStartMs": occurrence["sourceStartMs"],
                "playMs": occurrence["playMs"],
                "playRate": occurrence["playRate"],
                "loop": occurrence["repeatUntilStageEnd"],
            }
            for occurrence in animation["occurrences"]
        ],
    }


def compile_cue(pattern_id: str, stage: dict[str, Any], cue: dict[str, Any]) -> dict[str, Any]:
    if cue.get(
        "timingBasis", CUE_TIMING_BASIS_CLIP_OCCURRENCE
    ) == CUE_TIMING_BASIS_STAGE_CLOCK:
        return {
            "bindingId": cue["cueId"],
            "occurrenceId": cue["occurrenceId"],
            "patternId": pattern_id,
            "stageId": stage["stageId"],
            "actionId": stage["actionId"],
            "timingBasis": CUE_TIMING_BASIS_STAGE_CLOCK,
            "stageOffsetMs": cue["stageOffsetMs"],
            "effectAssetId": cue["effectAssetId"],
            "anchorSlotId": cue["anchorSlotId"],
            "followPolicy": cue["followPolicy"],
            "stopPolicy": cue["stopPolicy"],
            "repeatPolicy": cue["repeatPolicy"],
            "localTransform": copy.deepcopy(cue["localTransform"]),
            "scalePolicy": copy.deepcopy(cue["scalePolicy"]),
        }
    return {
        "bindingId": cue["cueId"],
        "occurrenceId": cue["occurrenceId"],
        "patternId": pattern_id,
        "stageId": stage["stageId"],
        "actionId": stage["actionId"],
        "clipOccurrenceId": cue["clipOccurrenceId"],
        "effectAssetId": cue["effectAssetId"],
        "anchorSlotId": cue["anchorSlotId"],
        "followPolicy": cue["followPolicy"],
        "stopPolicy": cue["stopPolicy"],
        "repeatPolicy": cue["repeatPolicy"],
        "sourceStartMs": cue["sourceStartMs"],
        "sourceEndMs": cue["sourceEndMs"],
        "localTransform": copy.deepcopy(cue["localTransform"]),
        "scalePolicy": copy.deepcopy(cue["scalePolicy"]),
    }


def _array_span(text: str, key: str) -> tuple[int, int, list[tuple[int, int]]]:
    match = re.search(r'"' + re.escape(key) + r'"\s*:', text)
    if not match:
        raise PipelineError(f"JSON array property not found: {key}")
    start = text.find("[", match.end())
    if start < 0:
        raise PipelineError(f"JSON array opening bracket not found: {key}")
    in_string = False
    escaped = False
    object_depth = 0
    array_depth = 1
    object_start: int | None = None
    spans: list[tuple[int, int]] = []
    index = start + 1
    while index < len(text):
        char = text[index]
        if in_string:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == '"':
                in_string = False
            index += 1
            continue
        if char == '"':
            in_string = True
        elif char == "[":
            array_depth += 1
        elif char == "]":
            array_depth -= 1
            if array_depth == 0:
                if object_depth != 0:
                    raise PipelineError(f"unbalanced object in array: {key}")
                return start, index, spans
        elif char == "{":
            if array_depth == 1 and object_depth == 0:
                object_start = index
            object_depth += 1
        elif char == "}":
            object_depth -= 1
            if object_depth < 0:
                raise PipelineError(f"unbalanced object close in array: {key}")
            if array_depth == 1 and object_depth == 0:
                if object_start is None:
                    raise PipelineError(f"missing object start in array: {key}")
                spans.append((object_start, index + 1))
                object_start = None
        index += 1
    raise PipelineError(f"unterminated JSON array: {key}")


def replace_rows_same_ordinal(
    text: str, array_key: str, id_key: str, replacements: Mapping[str, dict[str, Any]]
) -> str:
    _, _, spans = _array_span(text, array_key)
    parsed_rows = [json.loads(text[start:end], object_pairs_hook=_reject_duplicate_pairs) for start, end in spans]
    current_ids = [row.get(id_key) for row in parsed_rows]
    missing = sorted(set(replacements) - set(current_ids))
    if missing:
        raise PipelineError(f"{array_key} replacement would insert missing IDs before G09: {missing}")
    output = text
    for (start, end), row in reversed(list(zip(spans, parsed_rows))):
        identity = row.get(id_key)
        if identity not in replacements:
            continue
        line_start = output.rfind("\n", 0, start) + 1
        prefix = output[line_start:start]
        replacement = json.dumps(replacements[identity], ensure_ascii=False, indent=2)
        replacement = replacement.replace("\n", "\n" + prefix)
        output = output[:start] + replacement + output[end:]
    projected = json.loads(output, object_pairs_hook=_reject_duplicate_pairs)
    projected_rows = projected[array_key]
    if [row[id_key] for row in projected_rows] != current_ids:
        raise PipelineError(f"{array_key} stable ID/ordinal sequence changed")
    return output


def replace_or_append_rows(
    text: str,
    array_key: str,
    id_key: str,
    replacements: Mapping[str, dict[str, Any]],
) -> str:
    """Replace known rows in place and append newly managed rows deterministically.

    Existing stable-ID ordinals and raw bytes outside replaced managed rows remain
    untouched.  Mapping insertion order is the canonical append order, which is
    supplied by the joined pattern/stage authoring order.
    """

    _, _, spans = _array_span(text, array_key)
    parsed_rows = [
        json.loads(text[start:end], object_pairs_hook=_reject_duplicate_pairs)
        for start, end in spans
    ]
    current_ids = [row.get(id_key) for row in parsed_rows]
    if len(current_ids) != len(set(current_ids)):
        raise PipelineError(f"{array_key} contains duplicate {id_key} values")
    for identity, replacement in replacements.items():
        if replacement.get(id_key) != identity:
            raise PipelineError(
                f"{array_key} replacement identity mismatch: {identity}"
            )

    output = text
    for (start, end), row in reversed(list(zip(spans, parsed_rows))):
        identity = row.get(id_key)
        if identity not in replacements:
            continue
        line_start = output.rfind("\n", 0, start) + 1
        prefix = output[line_start:start]
        replacement = json.dumps(replacements[identity], ensure_ascii=False, indent=2)
        replacement = replacement.replace("\n", "\n" + prefix)
        output = output[:start] + replacement + output[end:]

    missing_ids = [identity for identity in replacements if identity not in current_ids]
    if missing_ids:
        _, _, projected_spans = _array_span(output, array_key)
        newline = "\r\n" if "\r\n" in output else "\n"
        if not projected_spans:
            raise PipelineError(
                f"{array_key} cannot establish append indentation from an empty Product array"
            )
        last_start, last_end = projected_spans[-1]
        line_start = output.rfind("\n", 0, last_start) + 1
        prefix = output[line_start:last_start]
        appended_rows = []
        for identity in missing_ids:
            serialized = json.dumps(
                replacements[identity], ensure_ascii=False, indent=2
            )
            appended_rows.append(serialized.replace("\n", newline + prefix))
        insertion = "," + newline + prefix + ("," + newline + prefix).join(
            appended_rows
        )
        output = output[:last_end] + insertion + output[last_end:]

    projected = json.loads(output, object_pairs_hook=_reject_duplicate_pairs)
    projected_ids = [row[id_key] for row in projected[array_key]]
    expected_ids = current_ids + missing_ids
    if projected_ids != expected_ids:
        raise PipelineError(
            f"{array_key} existing stable-ID ordinals or deterministic append order changed"
        )
    return output


def replace_append_or_remove_rows(
    text: str,
    array_key: str,
    id_key: str,
    replacements: Mapping[str, dict[str, Any]],
    removed_ids: set[str],
) -> str:
    """Replace/append managed rows and retire only explicitly owned old IDs.

    The transitional Products still contain sealed legacy rows.  Rebuilding the
    whole array would erase their reviewed raw JSON, while plain
    replace-or-append leaves removed managed stage actions behind.  This helper
    retains each untouched raw object byte-for-byte and removes only the IDs
    whose previous managed pattern topology proves ownership for.
    """

    array_start, array_end, spans = _array_span(text, array_key)
    if not spans:
        raise PipelineError(f"{array_key} cannot retire rows from an empty array")
    parsed_rows = [
        json.loads(text[start:end], object_pairs_hook=_reject_duplicate_pairs)
        for start, end in spans
    ]
    current_ids = [row.get(id_key) for row in parsed_rows]
    if len(current_ids) != len(set(current_ids)):
        raise PipelineError(f"{array_key} contains duplicate {id_key} values")
    if removed_ids & set(replacements):
        raise PipelineError(f"{array_key} cannot replace and remove the same ID")
    unknown_removed = removed_ids - set(current_ids)
    if unknown_removed:
        raise PipelineError(
            f"{array_key} removal names missing IDs: {sorted(unknown_removed)}"
        )
    for identity, replacement in replacements.items():
        if replacement.get(id_key) != identity:
            raise PipelineError(
                f"{array_key} replacement identity mismatch: {identity}"
            )

    first_start, _ = spans[0]
    _, last_end = spans[-1]
    leading = text[array_start + 1:first_start]
    trailing = text[last_end:array_end]
    separator = (
        text[spans[0][1]:spans[1][0]]
        if len(spans) > 1
        else ",\n"
    )
    line_start = text.rfind("\n", 0, first_start) + 1
    prefix = text[line_start:first_start]
    current_id_set = set(current_ids)
    projected_rows: list[str] = []
    projected_ids: list[str] = []
    for (start, end), row in zip(spans, parsed_rows):
        identity = row.get(id_key)
        if identity in removed_ids:
            continue
        if identity in replacements:
            serialized = json.dumps(
                replacements[identity], ensure_ascii=False, indent=2
            )
            projected_rows.append(serialized.replace("\n", "\n" + prefix))
        else:
            projected_rows.append(text[start:end])
        projected_ids.append(identity)
    for identity, replacement in replacements.items():
        if identity in current_id_set:
            continue
        serialized = json.dumps(replacement, ensure_ascii=False, indent=2)
        projected_rows.append(serialized.replace("\n", "\n" + prefix))
        projected_ids.append(identity)

    output = (
        text[:array_start + 1]
        + leading
        + separator.join(projected_rows)
        + trailing
        + text[array_end:]
    )
    projected = json.loads(output, object_pairs_hook=_reject_duplicate_pairs)
    if [row[id_key] for row in projected[array_key]] != projected_ids:
        raise PipelineError(
            f"{array_key} managed removal changed an unrelated stable-ID ordinal"
        )
    return output


def replace_root_format_version(
    text: str,
    expected_schema: str,
    admitted_versions: set[int],
    projected_version: int,
) -> str:
    document = json.loads(text, object_pairs_hook=_reject_duplicate_pairs)
    if (
        not isinstance(document, dict)
        or document.get("schema") != expected_schema
        or document.get("formatVersion") not in admitted_versions
    ):
        raise PipelineError("Product header is not admitted for format projection")
    current_version = document["formatVersion"]
    if current_version == projected_version:
        return text
    match = re.search(
        rf'("formatVersion"\s*:\s*){current_version}(?=\s*,)',
        text,
    )
    if match is None:
        raise PipelineError("Product root formatVersion token was not found")
    return (
        text[: match.start()]
        + match.group(1)
        + str(projected_version)
        + text[match.end() :]
    )


def _assert_unmanaged_raw_rows_preserved(
    before: str, after: str, array_key: str, id_key: str, managed_ids: set[str]
) -> None:
    before_start, before_end, before_spans = _array_span(before, array_key)
    after_start, after_end, after_spans = _array_span(after, array_key)
    if before[: before_start + 1] != after[: after_start + 1] or before[before_end:] != after[after_end:]:
        raise PipelineError(f"{array_key} bytes outside the array changed")
    before_rows = {}
    for start, end in before_spans:
        raw = before[start:end]
        row = json.loads(raw)
        before_rows[row[id_key]] = raw
    after_rows = {}
    for start, end in after_spans:
        raw = after[start:end]
        row = json.loads(raw)
        after_rows[row[id_key]] = raw
    for identity, raw in before_rows.items():
        if identity not in managed_ids and after_rows.get(identity) != raw:
            raise PipelineError(f"unmanaged {array_key} raw row changed: {identity}")


def _compile_rotations(master: dict[str, Any], legacy: dict[str, Any]) -> list[dict[str, Any]]:
    rows = []
    windows_by_rotation = {
        window["compatibilityRotationId"]: window
        for window in master["decisionModel"]["selectionWindows"]
    }
    sets = {
        row["selectionSetId"]: row for row in master["decisionModel"]["selectionSets"]
    }
    for sealed in sorted(legacy["sharedRotationRows"], key=lambda row: row["rotationOrdinal"]):
        rotation_id = sealed["rotationId"]
        if rotation_id in windows_by_rotation:
            window = windows_by_rotation[rotation_id]
            selection_set = sets[window["selectionSetId"]]
            row = {
                "rotationId": rotation_id,
                "selectionMode": selection_set["mode"],
                "fromHealthBar": window["maximumHealthBarInclusive"],
                "toHealthBar": window["minimumHealthBarExclusive"],
                "windowId": window["windowId"],
                "gameplayPhase": window["gameplayPhase"],
                "selectionSetId": selection_set["selectionSetId"],
                "candidates": copy.deepcopy(selection_set["candidates"]),
            }
        else:
            row = {
                "rotationId": rotation_id,
                "selectionMode": sealed["selectionMode"],
                "fromHealthBar": sealed["fromHealthBar"],
                "toHealthBar": sealed["toHealthBar"],
                "patternIds": [ref["patternId"] for ref in sealed["patternRefs"]],
            }
        rows.append(row)
    arena_break = next(
        (
            mechanic
            for mechanic in master["decisionModel"]["mechanics"]
            if mechanic["patternId"] == "VALTAN_ARENA_BREAK_109"
        ),
        None,
    )
    legacy_rows = [
        row for row in rows if row["rotationId"] not in windows_by_rotation
    ]
    if (
        arena_break is None
        or not legacy_rows
        or arena_break["trigger"]["healthBar"]
        != legacy_rows[0]["fromHealthBar"] + 1
    ):
        raise PipelineError(
            "VALTAN_ARENA_BREAK_109 healthBar drifted from the first legacy "
            "rotation boundary; phase topology must be changed atomically"
        )
    return rows


def _boss_visuals(boss_catalog: dict[str, Any]) -> dict[str, str]:
    bosses = [row for row in boss_catalog["bosses"] if row["archetypeId"] == "BOSS_VALTAN"]
    if len(bosses) != 1:
        raise PipelineError("BossCatalog must contain exactly one BOSS_VALTAN")
    result: dict[str, str] = {}
    for row in bosses[0]["combatObjectVisuals"]:
        archetype = row["combatObjectArchetypeId"]
        if archetype in result:
            raise PipelineError(f"duplicate BossCatalog combat visual: {archetype}")
        result[archetype] = row["clientVisualId"]
    return result


def _compile_combat_products(
    master: dict[str, Any], companion: dict[str, Any], legacy: dict[str, Any], boss_catalog: dict[str, Any]
) -> list[dict[str, Any]]:
    managed_owners: dict[str, tuple[str, str, int]] = {}
    for pattern in master["patterns"]:
        for stage in pattern["stages"]:
            for event in stage["events"]:
                if event["kind"] in ("SPAWN_COMBAT_OBJECT", "SPAWN_COMBAT_OBJECT_VOLLEY"):
                    archetype = event["combatObjectArchetypeId"]
                    if archetype in managed_owners:
                        raise PipelineError(f"combat object has multiple managed owners: {archetype}")
                    managed_owners[archetype] = (pattern["patternId"], stage["actionId"], stage["durationMs"])
    legacy_owners = {
        row["combatObjectArchetypeId"]: row for row in legacy["legacyCombatObjectOwners"]
    }
    visuals = _boss_visuals(boss_catalog)
    rows: list[tuple[int, dict[str, Any]]] = []
    for companion_ordinal, obj in enumerate(companion["objects"]):
        archetype = obj["combatObjectArchetypeId"]
        if archetype in managed_owners:
            owner_pattern, owner_action, life_ms = managed_owners[archetype]
            ordinal = companion_ordinal
        elif archetype in legacy_owners:
            sealed = legacy_owners[archetype]
            owner_pattern = sealed["ownerPatternId"]
            owner_action = sealed["ownerStageActionId"]
            life_ms = sealed["lifeMs"]
            ordinal = sealed["originalOrdinal"]
        else:
            raise PipelineError(f"combat object has no exact managed/legacy owner: {archetype}")
        life_ms = obj.get("lifetimeMs", life_ms)
        client_visual = visuals.get(archetype)
        if client_visual is None:
            raise PipelineError(f"combat object has no BossCatalog visual: {archetype}")
        origin = obj["spawn"]["origin"]
        if origin["kind"] == "RESOLVED_VOLLEY_POSITION":
            origin_policy = "LOCKED_TARGET_PER_ALIVE_PLAYER"
            offset_forward = 0.0
            offset_right = 0.0
        else:
            origin_policy = "BOSS_POSITION"
            offset_forward = origin["forwardOffsetM"]
            offset_right = origin["rightOffsetM"]
        movement = obj["movement"]
        speed = movement.get("speedMps", 0.0)
        distance = movement.get("maximumDistanceM", 0.0)
        hits = []
        for hit in obj["hits"]:
            flat = _flat_from_shape(hit["shape"])
            flat.update(
                {
                    "hitId": hit["hitId"],
                    "trigger": hit["trigger"]["kind"],
                    "atMs": hit["trigger"].get("atMs", 0),
                    "repeatCount": hit["repeat"]["count"],
                    "repeatIntervalMs": hit["repeat"]["intervalMs"],
                    "serverDamageProfileId": hit["serverDamageProfileId"],
                    "pushRangeM": hit["pushRangeM"],
                    "pushMs": hit["pushMs"],
                    "knockdown": hit["knockdown"],
                    "downMs": hit["downMs"],
                }
            )
            hits.append(flat)
        rows.append(
            (
                ordinal,
                {
                    "combatObjectArchetypeId": archetype,
                    "clientVisualId": client_visual,
                    "ownerPatternId": owner_pattern,
                    "ownerStageActionId": owner_action,
                    "kind": obj["kind"],
                    "originPolicy": origin_policy,
                    "directionPolicy": obj["spawn"]["direction"]["kind"],
                    "offsetForwardM": offset_forward,
                    "offsetRightM": offset_right,
                    "speedMps": speed,
                    "maximumDistanceM": distance,
                    "lifeMs": life_ms,
                    "hits": hits,
                },
            )
        )
    ordinals = [ordinal for ordinal, _ in rows]
    if len(ordinals) != len(set(ordinals)):
        raise PipelineError("combat-object Product ordinals collide")
    return [row for _, row in sorted(rows, key=lambda item: item[0])]


def _world_owner(
    master: dict[str, Any], world_event_set_id: str
) -> tuple[str, str, str]:
    owners = []
    for pattern in master["patterns"]:
        for stage in pattern["stages"]:
            for event in stage["events"]:
                if (
                    event["kind"] == "TRIGGER_WORLD_EVENT_SET"
                    and event["worldEventSetId"] == world_event_set_id
                ):
                    owners.append((pattern["patternId"], stage["stageId"], event["trigger"]))
    expected = MANAGED_WORLD_SET_OWNERS.get(world_event_set_id)
    if expected is None or owners != [(expected[0], expected[1], "ENTER")]:
        raise PipelineError(
            f"world set owner is not exact for {world_event_set_id}: {owners}"
        )
    return expected


def _preserve_equal_json_values(source: Any, projected: Any) -> Any:
    """Retain source JSON number shapes for fields a draft did not change."""

    if isinstance(source, dict) and isinstance(projected, dict):
        if source.keys() != projected.keys():
            return projected
        return {
            key: _preserve_equal_json_values(source[key], projected[key])
            for key in projected
        }
    if isinstance(source, list) and isinstance(projected, list):
        if len(source) != len(projected):
            return projected
        return [
            _preserve_equal_json_values(source_value, projected_value)
            for source_value, projected_value in zip(source, projected)
        ]
    source_is_number = isinstance(source, (int, float)) and not isinstance(source, bool)
    projected_is_number = isinstance(projected, (int, float)) and not isinstance(projected, bool)
    if source_is_number and projected_is_number and source == projected:
        return copy.deepcopy(source)
    return projected


def validate_full_product_mechanic_trigger_closure(encounter: dict[str, Any]) -> None:
    """No managed edit may collide with a sealed legacy health mechanic."""

    seen: dict[tuple[int, int], str] = {}
    for row in encounter["patterns"]:
        if row.get("selectionMode") != "HEALTH_BAR":
            continue
        pattern_id = stable_id(row.get("patternId"), "Product mechanic patternId")
        key = (
            integer(row.get("triggerHealthBar"), f"{pattern_id}.triggerHealthBar", 1, 1000),
            integer(row.get("triggerOrder"), f"{pattern_id}.triggerOrder", 1, 100000),
        )
        previous = seen.get(key)
        if previous is not None:
            raise PipelineError(
                "Product health mechanic trigger tuple is duplicated: "
                f"{key[0]}/{key[1]} ({previous}, {pattern_id})"
            )
        seen[key] = pattern_id


def project_v2_products(
    root: Path,
    docs: dict[str, Any],
    master: dict[str, Any],
    *,
    migration_fixture: bool = False,
) -> dict[str, str]:
    projection_world_sets = (
        build_world_event_sets(docs[WORLD_PRODUCT_REL])
        if migration_fixture
        else docs[WORLD_SET_REL]
    )
    validate_v2_master(
        master,
        projection_world_sets,
        docs[COMBAT_AUTHORING_REL],
        migration_fixture=migration_fixture,
    )
    if (
        not migration_fixture
        and master["decisionModel"]["scriptedSequence"] is None
    ):
        raise PipelineError(
            "current Valtan projection requires decisionModel.scriptedSequence"
        )
    validate_decision_model_against_boss_profiles(
        master, docs[BOSS_PROFILES_REL]
    )
    managed_pattern_ids = {row["patternId"] for row in master["patterns"]}
    fixture_audition_ids = (
        {
            row["patternId"]
            for row in docs[ENCOUNTER_REL]["patterns"]
            if row.get("selectionMode") == AUDITION_ONLY
        }
        if migration_fixture
        else set()
    )
    fixture_legacy_managed_ids = fixture_audition_ids | (
        CURRENT_MIGRATION_MANAGED_PATTERN_IDS
        if migration_fixture
        else set()
    )
    source_patterns = {
        row["patternId"]: row for row in docs[ENCOUNTER_REL]["patterns"]
    }
    source_retired_pattern_ids = (
        set(master["retiredPatternIds"]) & set(source_patterns)
    )
    validate_legacy_products(
        docs[LEGACY_REL], docs,
        managed_pattern_ids | source_retired_pattern_ids | fixture_legacy_managed_ids,
    )
    managed_patterns = {
        row["patternId"]: compile_pattern_product(master, row)
        for row in master["patterns"]
    }
    managed_patterns = {
        identity: (
            _preserve_equal_json_values(source_patterns[identity], row)
            if identity in source_patterns
            else row
        )
        for identity, row in managed_patterns.items()
    }
    managed_bindings = {
        stage["actionId"]: compile_binding(stage)
        for pattern in master["patterns"]
        for stage in pattern["stages"]
    }
    managed_cues = {
        cue["cueId"]: compile_cue(pattern["patternId"], stage, cue)
        for pattern in master["patterns"]
        for stage in pattern["stages"]
        for cue in stage["effectCues"]
    }
    rotations = _compile_rotations(master, docs[LEGACY_REL])
    managed_rotation_ids = {
        window["compatibilityRotationId"] for window in master["decisionModel"]["selectionWindows"]
    }
    rotation_replacements = {row["rotationId"]: row for row in rotations if row["rotationId"] in managed_rotation_ids}
    combat_authoring = docs[COMBAT_AUTHORING_REL]
    if migration_fixture:
        # The sealed v1 fixture predates independent donut ownership. Preserve
        # newer Product rows untouched, instead of making historical migration
        # invent a spawn owner or delete a current hazard.
        fixture_owners = {
            event["combatObjectArchetypeId"]
            for pattern in master["patterns"] for stage in pattern["stages"]
            for event in stage["events"]
            if event["kind"] in ("SPAWN_COMBAT_OBJECT", "SPAWN_COMBAT_OBJECT_VOLLEY")
        } | {row["combatObjectArchetypeId"] for row in docs[LEGACY_REL]["legacyCombatObjectOwners"]}
        combat_authoring = {**combat_authoring, "objects": [
            row for row in combat_authoring["objects"]
            if row["combatObjectArchetypeId"] in fixture_owners
        ]}
    combat_rows = _compile_combat_products(
        master, combat_authoring, docs[LEGACY_REL], docs[BOSS_CATALOG_REL]
    )
    source_combat_rows = {
        row["combatObjectArchetypeId"]: row
        for row in docs[COMBAT_PRODUCT_REL]["objects"]
    }
    combat_rows = [
        _preserve_equal_json_values(
            source_combat_rows.get(row["combatObjectArchetypeId"], {}), row
        )
        for row in combat_rows
    ]
    world_replacements = {}
    for event_set in projection_world_sets["sets"]:
        if event_set["worldEventSetId"] not in MANAGED_WORLD_SET_OWNERS:
            continue
        world_owner = _world_owner(master, event_set["worldEventSetId"])
        for member in event_set["members"]:
            world_replacements[member["bindingId"]] = {
                "bindingId": member["bindingId"],
                "mutationId": member["mutationId"],
                "patternId": world_owner[0],
                "stageId": world_owner[1],
                "triggerKind": world_owner[2],
                "offsetMs": member["offsetMs"],
                "receiverCollisionId": member["receiverCollisionId"],
                "enabled": member["enabled"],
            }
    source_texts = {
        relative: read_text(repo_path(root, relative))
        for relative in (
            ENCOUNTER_REL,
            BINDINGS_REL,
            CUES_REL,
            ROTATIONS_REL,
            COMBAT_PRODUCT_REL,
            WORLD_PRODUCT_REL,
        )
    }
    source_managed_action_ids = {
        stage["actionId"]
        for pattern_id, pattern in source_patterns.items()
        if pattern_id in managed_pattern_ids | source_retired_pattern_ids
        for stage in pattern["stages"]
    }
    removed_managed_action_ids = (
        source_managed_action_ids - set(managed_bindings)
    )
    # Binding ordinals are part of the sealed legacy compatibility receipt.
    # Removing a managed stage row would shift every later sealed row, so a
    # retired action becomes an explicit inert NONE tombstone at the same
    # ordinal. The encounter no longer emits that action and the empty chain also
    # prevents sound/root-motion projection from treating it as playable.
    ordinal_safe_bindings = dict(managed_bindings)
    for action_id in removed_managed_action_ids:
        ordinal_safe_bindings[action_id] = {
            "actionId": action_id,
            "playbackMode": ANIMATION_MODE_NONE,
            "clips": [],
        }
    source_cue_document = json.loads(
        source_texts[CUES_REL], object_pairs_hook=_reject_duplicate_pairs
    )
    source_managed_cue_ids = {
        row["bindingId"]
        for row in source_cue_document["cues"]
        if row.get("patternId") in managed_pattern_ids | set(master["retiredPatternIds"])
    }
    removed_managed_cue_ids = source_managed_cue_ids - set(managed_cues)
    binding_rows_output = replace_append_or_remove_rows(
        source_texts[BINDINGS_REL],
        "bindings",
        "actionId",
        ordinal_safe_bindings,
        set(),
    )
    cue_rows_output = replace_append_or_remove_rows(
        source_texts[CUES_REL],
        "cues",
        "bindingId",
        managed_cues,
        removed_managed_cue_ids,
    )
    # The frozen v1 migration is a compatibility proof, not an authority that
    # may downgrade a current cue Product.  In particular, the live donut cue
    # is now a v4 stage-clock independent Effect while the fixture still owns
    # the historical clip projection used to prove that old input migrates.
    if migration_fixture:
        cue_rows_output = source_texts[CUES_REL]
    rotation_output = replace_rows_same_ordinal(
        source_texts[ROTATIONS_REL],
        "rotations",
        "rotationId",
        rotation_replacements,
    )
    rotation_document = json.loads(
        rotation_output, object_pairs_hook=_reject_duplicate_pairs
    )
    if not migration_fixture:
        rotation_document["formatVersion"] = 4
        rotation_document["scriptedSequence"] = copy.deepcopy(
            master["decisionModel"]["scriptedSequence"]
        )
    outputs = {
        ENCOUNTER_REL: replace_append_or_remove_rows(
            source_texts[ENCOUNTER_REL], "patterns", "patternId",
            managed_patterns, source_retired_pattern_ids,
        ),
        BINDINGS_REL: replace_root_format_version(
            binding_rows_output,
            "lostark.valtan-pattern-bindings",
            {2, 3},
            2 if migration_fixture else 3,
        ),
        CUES_REL: replace_root_format_version(
            cue_rows_output,
            "lostark.valtan-pattern-effect-cues",
            {2, 3, 4},
            4,
        ),
        ROTATIONS_REL: json_text(rotation_document),
        COMBAT_PRODUCT_REL: replace_or_append_rows(
            source_texts[COMBAT_PRODUCT_REL],
            "objects",
            "combatObjectArchetypeId",
            {row["combatObjectArchetypeId"]: row for row in combat_rows},
        ),
        WORLD_PRODUCT_REL: replace_rows_same_ordinal(
            source_texts[WORLD_PRODUCT_REL], "bindings", "bindingId", world_replacements
        ),
    }
    projected_encounter = json.loads(
        outputs[ENCOUNTER_REL], object_pairs_hook=_reject_duplicate_pairs
    )
    projected_audition_ids = {
        row["patternId"]
        for row in projected_encounter["patterns"]
        if row.get("selectionMode") == AUDITION_ONLY
    }
    manual_audition_ids = {
        row["patternId"] for row in master["decisionModel"]["manualAuditions"]
    }
    expected_audition_ids = manual_audition_ids | fixture_audition_ids
    if projected_audition_ids != expected_audition_ids:
        raise PipelineError(
            "projected Product AUDITION_ONLY ownership does not exact-join "
            "decisionModel.manualAuditions"
        )
    validate_full_product_mechanic_trigger_closure(
        json.loads(outputs[ENCOUNTER_REL], object_pairs_hook=_reject_duplicate_pairs)
    )
    _assert_unmanaged_raw_rows_preserved(
        source_texts[ENCOUNTER_REL],
        outputs[ENCOUNTER_REL],
        "patterns",
        "patternId",
        set(managed_patterns) | source_retired_pattern_ids,
    )
    _assert_unmanaged_raw_rows_preserved(
        source_texts[BINDINGS_REL],
        binding_rows_output,
        "bindings",
        "actionId",
        set(ordinal_safe_bindings),
    )
    _assert_unmanaged_raw_rows_preserved(
        source_texts[CUES_REL],
        cue_rows_output,
        "cues",
        "bindingId",
        set(managed_cues) | removed_managed_cue_ids,
    )
    _assert_unmanaged_raw_rows_preserved(
        source_texts[WORLD_PRODUCT_REL],
        outputs[WORLD_PRODUCT_REL],
        "bindings",
        "bindingId",
        set(world_replacements),
    )
    before_world = json.loads(source_texts[WORLD_PRODUCT_REL])
    after_world = json.loads(outputs[WORLD_PRODUCT_REL])
    if (
        canonical_hash(before_world["groups"]) != canonical_hash(after_world["groups"])
        or canonical_hash(before_world["mutations"]) != canonical_hash(after_world["mutations"])
        or canonical_hash(before_world["provenance"]) != canonical_hash(after_world["provenance"])
    ):
        raise PipelineError("world groups/mutations/provenance changed during transitional projection")
    before_bindings = before_world["bindings"]
    after_bindings = after_world["bindings"]
    if len(before_bindings) != len(after_bindings):
        raise PipelineError("world projection changed the flat binding count")
    unmanaged_before = [row for row in before_bindings if row["bindingId"] not in world_replacements]
    unmanaged_after = [row for row in after_bindings if row["bindingId"] not in world_replacements]
    if unmanaged_before != unmanaged_after:
        raise PipelineError("an unmanaged World/Map binding changed")
    for relative, text in outputs.items():
        try:
            json.loads(text, object_pairs_hook=_reject_duplicate_pairs)
        except json.JSONDecodeError as exc:
            raise PipelineError(f"projected JSON failed round-trip: {relative}: {exc}") from exc
    return outputs


def build_repository_product_projection(
    root: Path,
) -> tuple[dict[str, Any], dict[str, Any], dict[str, str]]:
    """Project the checked-in split authoring head into its runtime Products."""

    docs = load_pipeline_documents(root)
    validate_world_event_sets(
        docs[WORLD_SET_REL], docs[WORLD_PRODUCT_REL], migration_fixture=False
    )
    validate_combat_authoring(docs[COMBAT_AUTHORING_REL])
    validate_balance_documents(docs[BOSS_PROFILES_REL], docs[DAMAGE_REL])
    joined = join_v2_authoring(
        docs[GAMEPLAY_AUTHORING_REL],
        docs[PRESENTATION_AUTHORING_REL],
        docs[WORLD_SET_REL],
        docs[COMBAT_AUTHORING_REL],
    )
    validate_manual_audition_animation_lineage(
        joined,
        read_json(repo_path(root, DEBUG_PRESENTATION_REL)),
        read_json(repo_path(root, ANIMATION_PROMOTION_MANIFEST_REL)),
    )
    managed = {row["patternId"] for row in joined["patterns"]}
    validate_legacy_manifest(docs[LEGACY_REL], managed)
    outputs = project_v2_products(root, docs, joined)
    balance_outputs = project_balance_products(
        root, docs[BOSS_PROFILES_REL], docs[DAMAGE_REL]
    )
    provenance_inputs = {**outputs, **balance_outputs}
    outputs[PROVENANCE_REL] = project_provenance_receipt(
        root, provenance_inputs
    )
    return docs, joined, outputs


def stage_repository_product_projection(
    root: Path, output_root: Path
) -> dict[str, Any]:
    """Materialize a validated projection below an isolated external root."""

    resolved_output = staging_root(root, output_root, "OutputRoot")
    if resolved_output.exists():
        if _is_reparse_point(resolved_output) or not resolved_output.is_dir():
            raise PipelineError("OutputRoot must be a regular directory")
        if any(resolved_output.iterdir()):
            raise PipelineError("OutputRoot must be empty")
    else:
        resolved_output.mkdir(parents=True)

    before = source_manifest(root)
    _, _, outputs = build_repository_product_projection(root)
    if source_manifest(root)["sourceManifestId"] != before["sourceManifestId"]:
        raise PipelineError("Valtan sources changed during Product projection")

    artifacts = []
    for relative in REPOSITORY_PRODUCT_ARTIFACTS:
        text = outputs[relative]
        parsed = json.loads(text, object_pairs_hook=_reject_duplicate_pairs)
        if not isinstance(parsed, dict):
            raise PipelineError(f"projected Product root is not an object: {relative}")
        destination = resolved_output / relative
        _write_fsync(destination, text.encode("utf-8"))
        artifacts.append(
            {
                "path": relative,
                "sha256": sha256_file(destination),
                "bytes": destination.stat().st_size,
            }
        )
    return {
        "outputRoot": str(resolved_output),
        "sourceManifestId": before["sourceManifestId"],
        "projectedArtifacts": len(artifacts),
        "files": artifacts,
    }


def load_pipeline_documents(
    root: Path,
    *,
    include_companions: bool = True,
    include_split_authoring: bool = True,
    include_migration_fixture: bool = False,
) -> dict[str, Any]:
    relatives = [
        ENCOUNTER_REL,
        ROTATIONS_REL,
        COMBAT_PRODUCT_REL,
        CAMERA_REL,
        WORLD_PRODUCT_REL,
        BINDINGS_REL,
        CUES_REL,
        BOSS_CATALOG_REL,
        BOSS_PROFILES_REL,
        DAMAGE_REL,
        EFFECT_CATALOG_REL,
    ]
    if include_migration_fixture:
        relatives.append(MASTER_REL)
    if include_split_authoring:
        relatives.extend((GAMEPLAY_AUTHORING_REL, PRESENTATION_AUTHORING_REL))
    if include_companions:
        relatives.extend((COMBAT_AUTHORING_REL, WORLD_SET_REL, LEGACY_REL))
    docs = require_documents(root, relatives)
    if include_split_authoring:
        sequence = docs[GAMEPLAY_AUTHORING_REL].get("decisionModel", {}).get("scriptedSequence")
        if isinstance(sequence, dict) and "flowId" in sequence:
            docs[SAVED_FLOW_REL] = read_saved_flow_document(root)
            docs[GAMEPLAY_AUTHORING_REL] = resolve_gameplay_flow_reference(
                docs[GAMEPLAY_AUTHORING_REL], docs[SAVED_FLOW_REL]
            )
    return docs


def source_manifest(root: Path) -> dict[str, Any]:
    paths = (
        GAMEPLAY_AUTHORING_REL,
        PRESENTATION_AUTHORING_REL,
        COMBAT_AUTHORING_REL,
        WORLD_SET_REL,
        LEGACY_REL,
        ENCOUNTER_REL,
        ROTATIONS_REL,
        COMBAT_PRODUCT_REL,
        CAMERA_REL,
        WORLD_PRODUCT_REL,
        BINDINGS_REL,
        CUES_REL,
        BOSS_CATALOG_REL,
        BOSS_PROFILES_REL,
        DAMAGE_REL,
        EFFECT_CATALOG_REL,
        DEBUG_PRESENTATION_REL,
        ANIMATION_PROMOTION_MANIFEST_REL,
        PROVENANCE_REL,
    )
    source_gameplay = read_json(repo_path(root, GAMEPLAY_AUTHORING_REL))
    sequence = source_gameplay.get("decisionModel", {}).get("scriptedSequence")
    references_flow = isinstance(sequence, dict) and "flowId" in sequence
    if references_flow:
        paths += (SAVED_FLOW_REL,)

    def snapshot_entries() -> list[dict[str, Any]]:
        entries = []
        for relative in sorted(paths):
            if relative == SAVED_FLOW_REL:
                flow_bytes = repo_path(root, relative).read_bytes()
                sha256, byte_count = sha256_bytes(flow_bytes), len(flow_bytes)
            else:
                sha256, byte_count = source_text_identity(repo_path(root, relative))
            entries.append(
                {
                    "path": relative,
                    "sha256": sha256,
                    "bytes": byte_count,
                }
            )
        return entries

    entries = snapshot_entries()
    split_documents = require_documents(
        root,
        (
            GAMEPLAY_AUTHORING_REL,
            PRESENTATION_AUTHORING_REL,
            WORLD_SET_REL,
            COMBAT_AUTHORING_REL,
        ),
    )
    joined = join_v2_authoring(
        split_documents[GAMEPLAY_AUTHORING_REL],
        split_documents[PRESENTATION_AUTHORING_REL],
        split_documents[WORLD_SET_REL],
        split_documents[COMBAT_AUTHORING_REL],
        read_saved_flow_document(root) if references_flow else None,
    )
    if entries != snapshot_entries():
        raise PipelineError("Valtan source files changed during strict split join")
    manifest_hash = sha256_bytes(
        "".join(f"{row['path']}\0{row['sha256']}\n" for row in entries).encode("utf-8")
    )
    return {
        "schema": "lostark.valtan-tuning-source-manifest",
        "formatVersion": 1,
        "sourceManifestId": manifest_hash,
        "gameplaySourceVersion": split_documents[GAMEPLAY_AUTHORING_REL][
            "formatVersion"
        ],
        "presentationSourceVersion": split_documents[PRESENTATION_AUTHORING_REL][
            "formatVersion"
        ],
        "joinedSourceVersion": joined["formatVersion"],
        "splitJoinValidated": True,
        "files": entries,
    }


DRAFT_PATCH_SCHEMA = "lostark.valtan-tuning-draft-patch"
DRAFT_PATCH_OPERATIONS = {
    "SET_PATTERN_WEIGHT": ("op", "selectionSetId", "patternId", "value"),
    "SET_PATTERN_ENABLED": ("op", "selectionSetId", "patternId", "value"),
    "SET_MECHANIC_TRIGGER": (
        "op",
        "mechanicId",
        "patternId",
        "healthBar",
        "triggerOrder",
    ),
    "SET_PATTERN_REPEAT_LIMIT": ("op", "patternId", "value"),
    "SET_PATTERN_RANGE": ("op", "patternId", "minimumRangeM", "maximumRangeM"),
    "SET_STAGE_DURATION": ("op", "patternId", "stageId", "durationMs"),
    "SET_STAGE_HIT": ("op", "patternId", "stageId", "hit"),
    "SET_STAGE_GRABBED_RELEASE": (
        "op",
        "patternId",
        "stageId",
        "releaseMode",
        "speedMps",
        "durationMs",
        "yawOffsetDegrees",
    ),
    "SET_EFFECT_CUE_LOCAL_YAW": (
        "op",
        "patternId",
        "stageId",
        "occurrenceId",
        "localYawDegrees",
    ),
    "SET_AXE_VOLLEY": (
        "op",
        "patternId",
        "stageId",
        "eventId",
        "countPerResolvedTarget",
        "layout",
        "spawnSchedule",
        "arenaRandom",
        "allowOverlap",
        "maximumTotalObjects",
    ),
    "SET_BOSS_BASE_FIELD": ("op", "bossArchetypeId", "field", "value"),
    "SET_DAMAGE_RATE": ("op", "damageProfileId", "value"),
}

BOSS_BASE_FIELD_LIMITS: dict[str, tuple[str, float, float]] = {
    "maximumHp": ("integer", 1, 2**31 - 1),
    "maximumHealthBars": ("integer", 1, 1000),
    "attackPower": ("integer", 0, 2**31 - 1),
    "collisionRadius": ("number", 0.01, 1000),
    "engageDistance": ("number", 0, 10000),
    "moveSpeed": ("number", 0, 1000),
}

CANDIDATE_APPLY_CLASSES = frozenset(
    {"HOT_RELOAD", "ENCOUNTER_RESET", "SERVER_RESTART"}
)


def classify_candidate_apply_class(
    runtime_boss_profiles: dict[str, Any],
    candidate_boss_profiles: dict[str, Any],
) -> str:
    """Return the strongest activation class required by the final candidate state.

    Boss base values are copied into a live Valtan body, so changing any supported
    base field cannot be admitted by the existing tick-boundary HOT_RELOAD path.
    Classification intentionally compares the fully materialized candidate against
    the repository/runtime baseline rather than inspecting the current draft ops;
    a zero-op publish of a saved authoring head must retain this requirement.
    """

    runtime_rows = unique_index(
        runtime_boss_profiles.get("bosses"),
        "archetypeId",
        "runtime BossProfiles bosses",
    )
    candidate_rows = unique_index(
        candidate_boss_profiles.get("bosses"),
        "archetypeId",
        "candidate BossProfiles bosses",
    )
    runtime_valtan = runtime_rows.get("BOSS_VALTAN")
    candidate_valtan = candidate_rows.get("BOSS_VALTAN")
    if runtime_valtan is None or candidate_valtan is None:
        raise PipelineError("apply-class comparison requires BOSS_VALTAN in both profiles")
    for field in BOSS_BASE_FIELD_LIMITS:
        if field not in runtime_valtan or field not in candidate_valtan:
            raise PipelineError(
                f"apply-class comparison is missing BOSS_VALTAN.{field}"
            )
        if runtime_valtan[field] != candidate_valtan[field]:
            return "ENCOUNTER_RESET"
    return "HOT_RELOAD"


def validate_balance_documents(
    boss_profiles: dict[str, Any], damage_profiles: dict[str, Any]
) -> None:
    exact(boss_profiles, ("schema", "formatVersion", "bosses"), "BossProfiles root")
    if boss_profiles["schema"] != "lostark.boss-profiles" or boss_profiles["formatVersion"] != 4:
        raise PipelineError("BossProfiles header mismatch")
    bosses = unique_index(boss_profiles["bosses"], "archetypeId", "BossProfiles bosses")
    valtan = bosses.get("BOSS_VALTAN")
    if valtan is None:
        raise PipelineError("BossProfiles is missing BOSS_VALTAN")
    if valtan.get("encounterId") != "ENCOUNTER_VALTAN":
        raise PipelineError("BOSS_VALTAN encounterId mismatch")
    phase_policy = valtan.get("phasePolicy")
    if not isinstance(phase_policy, dict):
        raise PipelineError("BOSS_VALTAN phasePolicy must be a tagged object")
    exact(phase_policy, ("kind",), "BOSS_VALTAN phasePolicy")
    if phase_policy["kind"] != "AUTHORED_PATTERN_EVENT":
        raise PipelineError("BOSS_VALTAN must use AUTHORED_PATTERN_EVENT phasePolicy")
    for field, (kind, minimum, maximum) in BOSS_BASE_FIELD_LIMITS.items():
        if kind == "integer":
            integer(valtan.get(field), f"BOSS_VALTAN.{field}", int(minimum), int(maximum))
        else:
            number(valtan.get(field), f"BOSS_VALTAN.{field}", minimum, maximum)

    exact(damage_profiles, ("schema", "formatVersion", "profiles"), "DamageProfiles root")
    if (
        damage_profiles["schema"] != "lostark.damage-profiles"
        or damage_profiles["formatVersion"] != 2
    ):
        raise PipelineError("DamageProfiles header mismatch")
    damage_rows = unique_index(
        damage_profiles["profiles"], "damageProfileId", "DamageProfiles profiles"
    )
    for damage_id, row in damage_rows.items():
        exact(row, ("damageProfileId", "damageRatePercent"), f"damage profile {damage_id}")
        integer(row["damageRatePercent"], f"damage profile {damage_id}.damageRatePercent", 0, 2**31 - 1)


def validate_decision_model_against_boss_profiles(
    master: dict[str, Any], boss_profiles: dict[str, Any]
) -> None:
    bosses = unique_index(
        boss_profiles.get("bosses"), "archetypeId", "BossProfiles bosses"
    )
    valtan = bosses.get("BOSS_VALTAN")
    if valtan is None:
        raise PipelineError("decision validation requires BOSS_VALTAN")
    maximum_health_bars = integer(
        valtan.get("maximumHealthBars"),
        "BOSS_VALTAN.maximumHealthBars",
        1,
        1000,
    )
    for window in master["decisionModel"]["selectionWindows"]:
        if (
            window["maximumHealthBarInclusive"] > maximum_health_bars
            or window["minimumHealthBarExclusive"] >= maximum_health_bars
        ):
            raise PipelineError(
                "selection window exceeds BOSS_VALTAN.maximumHealthBars: "
                + window["windowId"]
            )
    for mechanic in master["decisionModel"]["mechanics"]:
        if mechanic["trigger"]["healthBar"] > maximum_health_bars:
            raise PipelineError(
                "mechanic trigger exceeds BOSS_VALTAN.maximumHealthBars: "
                + mechanic["mechanicId"]
            )


def _draft_error(
    message: str,
    *,
    operation_ordinal: int | None = None,
    pattern_id: str = "",
    stage_id: str = "",
    field: str = "",
    error_code: str | None = None,
) -> DraftPatchError:
    path = ""
    if operation_ordinal is not None:
        path = f"operations[{operation_ordinal}]"
        if field:
            path += f".{field}"
    return DraftPatchError(
        message,
        document="draftPatch",
        path=path,
        pattern_id=pattern_id,
        stage_id=stage_id,
        field=field,
        error_code=error_code,
    )


def _draft_pattern(
    master: dict[str, Any], pattern_id: Any, ordinal: int
) -> dict[str, Any]:
    identity = stable_id(pattern_id, f"operations[{ordinal}].patternId")
    matches = [row for row in master["patterns"] if row["patternId"] == identity]
    if len(matches) != 1:
        raise _draft_error(
            f"draft patch patternId does not resolve exactly once: {identity}",
            operation_ordinal=ordinal,
            pattern_id=identity,
            field="patternId",
            error_code="STABLE_ID_NOT_FOUND",
        )
    return matches[0]


def _draft_stage(
    pattern: dict[str, Any], stage_id: Any, ordinal: int
) -> dict[str, Any]:
    identity = stable_id(stage_id, f"operations[{ordinal}].stageId")
    matches = [row for row in pattern["stages"] if row["stageId"] == identity]
    if len(matches) != 1:
        raise _draft_error(
            f"draft patch stageId does not resolve exactly once: {identity}",
            operation_ordinal=ordinal,
            pattern_id=pattern["patternId"],
            stage_id=identity,
            field="stageId",
            error_code="STABLE_ID_NOT_FOUND",
        )
    return matches[0]


def _validate_volley_layout(layout: Any, count: int, context: str) -> dict[str, Any]:
    if not isinstance(layout, dict):
        raise DraftPatchError(f"{context} must be an object", field="layout")
    kind = layout.get("kind")
    if kind == "TARGET_CENTER":
        exact(layout, ("kind",), context)
        if count != 1:
            raise DraftPatchError("TARGET_CENTER only admits count 1", field="layout")
    elif kind == "RADIAL_AROUND_TARGET":
        exact(layout, ("kind", "radiusM", "startAngleDegrees", "angleStepDegrees"), context)
        if count < 2:
            raise DraftPatchError("RADIAL_AROUND_TARGET requires count 2..8", field="layout")
        number(layout["radiusM"], f"{context}.radiusM", 0.01, 1000)
        number(layout["startAngleDegrees"], f"{context}.startAngleDegrees", -360000, 360000)
        step = number(layout["angleStepDegrees"], f"{context}.angleStepDegrees", 0.000001, 360)
        if step * count > 360.000001:
            raise DraftPatchError("radial volley wraps onto itself", field="layout")
    else:
        raise DraftPatchError(f"unsupported volley layout: {kind!r}", field="layout")
    return copy.deepcopy(layout)


def apply_draft_patch(
    master: dict[str, Any],
    boss_profiles: dict[str, Any],
    damage_profiles: dict[str, Any],
    draft_patch: dict[str, Any],
    source_revision: str,
    world_sets: dict[str, Any],
    combat_authoring: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any], int]:
    """Apply bounded stable-ID operations to copies of the staged authoring views."""

    exact(draft_patch, ("schema", "formatVersion", "sourceRevision", "operations"), "draft patch root")
    if draft_patch["schema"] != DRAFT_PATCH_SCHEMA or draft_patch["formatVersion"] != 1:
        raise _draft_error("draft patch header mismatch", error_code="SCHEMA_MISMATCH")
    supplied_revision = draft_patch["sourceRevision"]
    if not isinstance(supplied_revision, str) or not re.fullmatch(r"[0-9a-f]{64}", supplied_revision):
        raise _draft_error(
            "draft patch sourceRevision must be a lowercase SHA-256",
            field="sourceRevision",
            error_code="SOURCE_REVISION_INVALID",
        )
    if supplied_revision != source_revision:
        raise _draft_error(
            "draft patch sourceRevision precondition failed",
            field="sourceRevision",
            error_code="SOURCE_REVISION_MISMATCH",
        )
    operations = draft_patch["operations"]
    if not isinstance(operations, list):
        raise _draft_error("draft patch operations must be an array", field="operations")

    patched_master = copy.deepcopy(master)
    patched_bosses = copy.deepcopy(boss_profiles)
    patched_damage = copy.deepcopy(damage_profiles)
    validate_balance_documents(patched_bosses, patched_damage)
    boss_by_id = unique_index(patched_bosses["bosses"], "archetypeId", "BossProfiles bosses")
    damage_by_id = unique_index(patched_damage["profiles"], "damageProfileId", "DamageProfiles profiles")
    touched: set[tuple[Any, ...]] = set()

    for ordinal, operation in enumerate(operations):
        if not isinstance(operation, dict):
            raise _draft_error("draft operation must be an object", operation_ordinal=ordinal)
        kind = operation.get("op")
        fields = DRAFT_PATCH_OPERATIONS.get(kind)
        if fields is None:
            raise _draft_error(
                f"unsupported draft operation: {kind!r}",
                operation_ordinal=ordinal,
                field="op",
                error_code="OPERATION_UNSUPPORTED",
            )
        try:
            exact(operation, fields, f"operations[{ordinal}]")
        except PipelineError as exc:
            raise _draft_error(str(exc), operation_ordinal=ordinal) from exc

        if kind in ("SET_PATTERN_WEIGHT", "SET_PATTERN_ENABLED"):
            set_id = stable_id(operation["selectionSetId"], f"operations[{ordinal}].selectionSetId")
            pattern_id = stable_id(operation["patternId"], f"operations[{ordinal}].patternId")
            target = (kind, set_id, pattern_id)
            selection_sets = [
                row
                for row in patched_master["decisionModel"]["selectionSets"]
                if row["selectionSetId"] == set_id
            ]
            candidates = (
                [row for row in selection_sets[0]["candidates"] if row["patternId"] == pattern_id]
                if len(selection_sets) == 1
                else []
            )
            if len(candidates) != 1:
                raise _draft_error(
                    f"selectionSetId/patternId does not resolve exactly once: {set_id}/{pattern_id}",
                    operation_ordinal=ordinal,
                    pattern_id=pattern_id,
                    field="selectionSetId",
                    error_code="STABLE_ID_NOT_FOUND",
                )
            if kind == "SET_PATTERN_WEIGHT":
                candidates[0]["weight"] = integer(
                    operation["value"],
                    f"operations[{ordinal}].value",
                    1,
                    100000,
                )
            else:
                candidates[0]["enabled"] = boolean(
                    operation["value"], f"operations[{ordinal}].value"
                )
        elif kind == "SET_MECHANIC_TRIGGER":
            mechanic_id = stable_id(
                operation["mechanicId"],
                f"operations[{ordinal}].mechanicId",
            )
            pattern_id = stable_id(
                operation["patternId"],
                f"operations[{ordinal}].patternId",
            )
            mechanics = [
                row
                for row in patched_master["decisionModel"]["mechanics"]
                if row["mechanicId"] == mechanic_id
                and row["patternId"] == pattern_id
            ]
            if len(mechanics) != 1:
                raise _draft_error(
                    f"mechanicId/patternId does not resolve exactly once: "
                    f"{mechanic_id}/{pattern_id}",
                    operation_ordinal=ordinal,
                    pattern_id=pattern_id,
                    field="mechanicId",
                    error_code="STABLE_ID_NOT_FOUND",
                )
            target = (kind, mechanic_id, pattern_id)
            mechanics[0]["trigger"]["healthBar"] = integer(
                operation["healthBar"],
                f"operations[{ordinal}].healthBar",
                1,
                1000,
            )
            mechanics[0]["triggerOrder"] = integer(
                operation["triggerOrder"],
                f"operations[{ordinal}].triggerOrder",
                1,
                100000,
            )
        elif kind == "SET_PATTERN_REPEAT_LIMIT":
            pattern = _draft_pattern(patched_master, operation["patternId"], ordinal)
            target = (kind, pattern["patternId"])
            pattern["eligibility"]["repeatPolicy"]["limit"] = integer(
                operation["value"], f"operations[{ordinal}].value", 0, 64
            )
        elif kind == "SET_PATTERN_RANGE":
            pattern = _draft_pattern(patched_master, operation["patternId"], ordinal)
            target = (kind, pattern["patternId"])
            minimum = number(
                operation["minimumRangeM"], f"operations[{ordinal}].minimumRangeM", 0, 1000
            )
            maximum = number(
                operation["maximumRangeM"], f"operations[{ordinal}].maximumRangeM", 0, 1000
            )
            if minimum > maximum:
                raise _draft_error(
                    "minimumRangeM must not exceed maximumRangeM",
                    operation_ordinal=ordinal,
                    pattern_id=pattern["patternId"],
                    field="minimumRangeM",
                )
            pattern["eligibility"]["minimumRangeM"] = minimum
            pattern["eligibility"]["maximumRangeM"] = maximum
        elif kind == "SET_STAGE_DURATION":
            pattern = _draft_pattern(patched_master, operation["patternId"], ordinal)
            stage = _draft_stage(pattern, operation["stageId"], ordinal)
            target = (kind, pattern["patternId"], stage["stageId"])
            stage["durationMs"] = integer(
                operation["durationMs"], f"operations[{ordinal}].durationMs", 1, 600000
            )
        elif kind == "SET_STAGE_HIT":
            pattern = _draft_pattern(patched_master, operation["patternId"], ordinal)
            stage = _draft_stage(pattern, operation["stageId"], ordinal)
            target = (kind, pattern["patternId"], stage["stageId"])
            if not isinstance(operation["hit"], dict):
                raise _draft_error(
                    "hit must be an object",
                    operation_ordinal=ordinal,
                    pattern_id=pattern["patternId"],
                    stage_id=stage["stageId"],
                    field="hit",
                )
            stage["hit"] = copy.deepcopy(operation["hit"])
        elif kind == "SET_STAGE_GRABBED_RELEASE":
            pattern = _draft_pattern(patched_master, operation["patternId"], ordinal)
            stage = _draft_stage(pattern, operation["stageId"], ordinal)
            events = [
                row
                for row in stage["events"]
                if row.get("kind") == "RELEASE_GRABBED_PLAYERS"
            ]
            if len(events) != 1:
                raise _draft_error(
                    "stage must resolve exactly one grabbed-player release event",
                    operation_ordinal=ordinal,
                    pattern_id=pattern["patternId"],
                    stage_id=stage["stageId"],
                    field="stageId",
                    error_code="STABLE_ID_NOT_FOUND",
                )
            event = events[0]
            release_mode = operation["releaseMode"]
            if release_mode not in ("HOLD", "OPPOSITE_KNOCKBACK", "ARENA_EJECTION"):
                raise _draft_error(
                    "unsupported grabbed-player releaseMode",
                    operation_ordinal=ordinal,
                    pattern_id=pattern["patternId"],
                    stage_id=stage["stageId"],
                    field="releaseMode",
                )
            speed_mps = number(
                operation["speedMps"],
                f"operations[{ordinal}].speedMps",
                0,
                50,
            )
            duration_ms = integer(
                operation["durationMs"],
                f"operations[{ordinal}].durationMs",
                0,
                5000,
            )
            yaw_offset_degrees = number(
                operation["yawOffsetDegrees"],
                f"operations[{ordinal}].yawOffsetDegrees",
                -180,
                180,
            )
            hold = release_mode == "HOLD" and speed_mps == 0 and duration_ms == 0
            launch = (
                release_mode in ("OPPOSITE_KNOCKBACK", "ARENA_EJECTION")
                and speed_mps > 0
                and duration_ms > 0
                and (release_mode == "ARENA_EJECTION" or yaw_offset_degrees == 0)
            )
            if not (hold or launch) or (hold and yaw_offset_degrees != 0):
                raise _draft_error(
                    "grabbed-player release values are incoherent",
                    operation_ordinal=ordinal,
                    pattern_id=pattern["patternId"],
                    stage_id=stage["stageId"],
                    field="releaseMode",
                )
            target = (kind, pattern["patternId"], stage["stageId"])
            event["releaseMode"] = release_mode
            event["speedMps"] = speed_mps
            event["durationMs"] = duration_ms
            event["yawOffsetDegrees"] = yaw_offset_degrees
        elif kind == "SET_EFFECT_CUE_LOCAL_YAW":
            pattern = _draft_pattern(patched_master, operation["patternId"], ordinal)
            stage = _draft_stage(pattern, operation["stageId"], ordinal)
            occurrence_id = stable_id(
                operation["occurrenceId"],
                f"operations[{ordinal}].occurrenceId",
            )
            cues = [
                row
                for row in stage["effectCues"]
                if row.get("occurrenceId") == occurrence_id
            ]
            if len(cues) != 1:
                raise _draft_error(
                    "effect cue occurrenceId must resolve exactly once in its stage",
                    operation_ordinal=ordinal,
                    pattern_id=pattern["patternId"],
                    stage_id=stage["stageId"],
                    field="occurrenceId",
                    error_code="STABLE_ID_NOT_FOUND",
                )
            cue = cues[0]
            transform = cue.get("localTransform")
            rotation = transform.get("rotationDegrees") if isinstance(transform, dict) else None
            if not isinstance(rotation, list) or len(rotation) != 3:
                raise _draft_error(
                    "effect cue localTransform.rotationDegrees is invalid",
                    operation_ordinal=ordinal,
                    pattern_id=pattern["patternId"],
                    stage_id=stage["stageId"],
                    field="localYawDegrees",
                )
            local_yaw = number(
                operation["localYawDegrees"],
                f"operations[{ordinal}].localYawDegrees",
                -180,
                180,
            )
            target = (kind, occurrence_id)
            rotation[1] = local_yaw
        elif kind == "SET_AXE_VOLLEY":
            pattern = _draft_pattern(patched_master, operation["patternId"], ordinal)
            stage = _draft_stage(pattern, operation["stageId"], ordinal)
            event_id = stable_id(operation["eventId"], f"operations[{ordinal}].eventId")
            events = [row for row in stage["events"] if row["eventId"] == event_id]
            if len(events) != 1 or events[0].get("kind") != "SPAWN_COMBAT_OBJECT_VOLLEY":
                raise _draft_error(
                    f"volley eventId does not resolve exactly once: {event_id}",
                    operation_ordinal=ordinal,
                    pattern_id=pattern["patternId"],
                    stage_id=stage["stageId"],
                    field="eventId",
                    error_code="STABLE_ID_NOT_FOUND",
                )
            event = events[0]
            if event["combatObjectArchetypeId"] != "combatobject.valtan.high-jump.target-axe":
                raise _draft_error(
                    "SET_AXE_VOLLEY may edit only the authored high-jump axe event",
                    operation_ordinal=ordinal,
                    pattern_id=pattern["patternId"],
                    stage_id=stage["stageId"],
                    field="eventId",
                )
            target = (kind, pattern["patternId"], stage["stageId"], event_id)
            count = integer(
                operation["countPerResolvedTarget"],
                f"operations[{ordinal}].countPerResolvedTarget",
                1,
                8,
            )
            event["countPerResolvedTarget"] = count
            event["layout"] = _validate_volley_layout(
                operation["layout"], count, f"operations[{ordinal}].layout"
            )
            try:
                _validate_volley_spawn_schedule(
                    operation["spawnSchedule"], stage["durationMs"], event_id
                )
            except PipelineError as exc:
                raise _draft_error(
                    str(exc),
                    operation_ordinal=ordinal,
                    pattern_id=pattern["patternId"],
                    stage_id=stage["stageId"],
                    field="spawnSchedule",
                ) from exc
            event["spawnSchedule"] = copy.deepcopy(operation["spawnSchedule"])
            try:
                _validate_volley_arena_random(operation["arenaRandom"], event_id)
            except PipelineError as exc:
                raise _draft_error(
                    str(exc),
                    operation_ordinal=ordinal,
                    pattern_id=pattern["patternId"],
                    stage_id=stage["stageId"],
                    field="arenaRandom",
                ) from exc
            event["arenaRandom"] = copy.deepcopy(operation["arenaRandom"])
            event["allowOverlap"] = boolean(
                operation["allowOverlap"], f"operations[{ordinal}].allowOverlap"
            )
            if event["allowOverlap"]:
                raise _draft_error(
                    "Valtan axe volley does not admit overlapping placements",
                    operation_ordinal=ordinal,
                    pattern_id=pattern["patternId"],
                    stage_id=stage["stageId"],
                    field="allowOverlap",
                )
            event["maximumTotalObjects"] = integer(
                operation["maximumTotalObjects"],
                f"operations[{ordinal}].maximumTotalObjects",
                count + event["arenaRandom"]["count"],
                64,
            )
        elif kind == "SET_BOSS_BASE_FIELD":
            boss_id = stable_id(operation["bossArchetypeId"], f"operations[{ordinal}].bossArchetypeId")
            field = operation["field"]
            if boss_id != "BOSS_VALTAN" or field not in BOSS_BASE_FIELD_LIMITS:
                raise _draft_error(
                    f"boss base field is outside the Valtan draft allowlist: {boss_id}/{field}",
                    operation_ordinal=ordinal,
                    field="field",
                    error_code="FIELD_NOT_ALLOWED",
                )
            boss = boss_by_id.get(boss_id)
            if boss is None:
                raise _draft_error(
                    f"bossArchetypeId does not resolve: {boss_id}",
                    operation_ordinal=ordinal,
                    field="bossArchetypeId",
                    error_code="STABLE_ID_NOT_FOUND",
                )
            target = (kind, boss_id, field)
            value_kind, minimum, maximum = BOSS_BASE_FIELD_LIMITS[field]
            boss[field] = (
                integer(operation["value"], f"operations[{ordinal}].value", int(minimum), int(maximum))
                if value_kind == "integer"
                else number(operation["value"], f"operations[{ordinal}].value", minimum, maximum)
            )
        else:
            damage_id = stable_id(
                operation["damageProfileId"], f"operations[{ordinal}].damageProfileId"
            )
            if not damage_id.startswith("damage.valtan."):
                raise _draft_error(
                    f"damage profile is outside the Valtan draft allowlist: {damage_id}",
                    operation_ordinal=ordinal,
                    field="damageProfileId",
                    error_code="FIELD_NOT_ALLOWED",
                )
            row = damage_by_id.get(damage_id)
            if row is None:
                raise _draft_error(
                    f"damageProfileId does not resolve: {damage_id}",
                    operation_ordinal=ordinal,
                    field="damageProfileId",
                    error_code="STABLE_ID_NOT_FOUND",
                )
            target = (kind, damage_id)
            row["damageRatePercent"] = integer(
                operation["value"], f"operations[{ordinal}].value", 0, 2**31 - 1
            )

        if target in touched:
            raise _draft_error(
                f"duplicate draft target: {target}",
                operation_ordinal=ordinal,
                error_code="DUPLICATE_TARGET",
            )
        touched.add(target)

    try:
        # Draft patches are also exercised against the frozen v1 migration
        # fixture.  That fixture intentionally has no current scripted sequence
        # and owns only MIGRATION_MANAGED_CUE_IDS; validating it as the live
        # split-authoring master makes later Product cue additions look like
        # missing migration coverage.
        migration_fixture = (
            patched_master["decisionModel"].get("scriptedSequence") is None
        )
        validate_v2_master(
            patched_master,
            world_sets,
            combat_authoring,
            migration_fixture=migration_fixture,
        )
        validate_balance_documents(patched_bosses, patched_damage)
        validate_decision_model_against_boss_profiles(
            patched_master, patched_bosses
        )
        known_damage = {row["damageProfileId"] for row in patched_damage["profiles"]}
        for pattern in patched_master["patterns"]:
            for stage in pattern["stages"]:
                damage_id = stage["hit"].get("serverDamageProfileId")
                if damage_id is not None:
                    if not isinstance(damage_id, str) or not damage_id.startswith("damage.valtan."):
                        raise PipelineError(
                            f"Valtan stage hit references damage outside its allowlist: "
                            f"{pattern['patternId']}/{stage['stageId']}/{damage_id}"
                        )
                    if damage_id not in known_damage:
                        raise PipelineError(
                            f"stage hit references missing DamageProfiles row: "
                            f"{pattern['patternId']}/{stage['stageId']}/{damage_id}"
                        )
    except DraftPatchError:
        raise
    except PipelineError as exc:
        raise _draft_error(str(exc), error_code="CANDIDATE_VALIDATION_FAILED") from exc
    return patched_master, patched_bosses, patched_damage, len(operations)


def project_balance_products(
    root: Path, boss_profiles: dict[str, Any], damage_profiles: dict[str, Any]
) -> dict[str, str]:
    validate_balance_documents(boss_profiles, damage_profiles)
    source_bosses = read_text(repo_path(root, BOSS_PROFILES_REL))
    source_damage = read_text(repo_path(root, DAMAGE_REL))
    boss_rows = {
        row["archetypeId"]: row
        for row in boss_profiles["bosses"]
        if row["archetypeId"] == "BOSS_VALTAN"
    }
    damage_rows = {
        row["damageProfileId"]: row
        for row in damage_profiles["profiles"]
        if row["damageProfileId"].startswith("damage.valtan.")
    }
    projected_bosses = replace_rows_same_ordinal(
        source_bosses, "bosses", "archetypeId", boss_rows
    )
    projected_damage = replace_rows_same_ordinal(
        source_damage, "profiles", "damageProfileId", damage_rows
    )
    _assert_unmanaged_raw_rows_preserved(
        source_damage,
        projected_damage,
        "profiles",
        "damageProfileId",
        set(damage_rows),
    )
    return {
        BOSS_PROFILES_REL: projected_bosses,
        DAMAGE_REL: projected_damage,
    }


def _receipt_path_value(value: Any, field_path: str) -> Any:
    current = value
    for token in field_path.split("."):
        if token == "length":
            if not isinstance(current, list):
                raise PipelineError(f"provenance length target is not an array: {field_path}")
            current = len(current)
            continue
        match = re.fullmatch(r"([A-Za-z][A-Za-z0-9]*)(?:\[([0-9]+)\])?", token)
        if match is None or not isinstance(current, dict) or match.group(1) not in current:
            raise PipelineError(f"provenance target path does not resolve: {field_path}")
        current = current[match.group(1)]
        if match.group(2) is not None:
            index = int(match.group(2))
            if not isinstance(current, list) or index >= len(current):
                raise PipelineError(f"provenance target index does not resolve: {field_path}")
            current = current[index]
    return current


def _receipt_entry_value(document: dict[str, Any], entry: dict[str, Any]) -> Any:
    target_document = entry["targetDocument"]
    target_id = entry["targetId"]
    target_field = entry["targetField"]
    base: Any = document
    if target_document == BOSS_PROFILES_REL:
        prefix = "boss:"
        rows, key = document["bosses"], "archetypeId"
    elif target_document == DAMAGE_REL:
        prefix = "damage:"
        rows, key = document["profiles"], "damageProfileId"
    elif target_document == COMBAT_PRODUCT_REL and target_id.startswith("combat-object:"):
        prefix = "combat-object:"
        rows, key = document["objects"], "combatObjectArchetypeId"
    elif target_document == ENCOUNTER_REL and target_id.startswith("pattern:"):
        prefix = "pattern:"
        rows, key = document["patterns"], "patternId"
        # The receipt keeps the live-pattern ordinal in its human-readable
        # field label, while the Product may interleave AUDITION_ONLY rows.
        # Resolve the owner by stable patternId, then address the row-local
        # property instead of treating that ordinal as a storage identity.
        target_field = re.sub(r"^patterns\[[0-9]+\]\.", "", target_field)
    else:
        prefix = ""
        rows, key = [], ""
    if prefix:
        identity = target_id[len(prefix) :]
        matches = [row for row in rows if row.get(key) == identity]
        if len(matches) != 1:
            raise PipelineError(f"provenance targetId does not resolve exactly once: {target_id}")
        base = matches[0]
    return copy.deepcopy(_receipt_path_value(base, target_field))


def project_provenance_receipt(root: Path, projected_outputs: Mapping[str, str]) -> str:
    """Synchronize changed projected fields to an explicit PROJECT_TUNED receipt."""

    source_text = read_text(repo_path(root, PROVENANCE_REL))
    receipt = json.loads(source_text, object_pairs_hook=_reject_duplicate_pairs)
    exact(
        receipt,
        (
            "schema",
            "formatVersion",
            "sourceBuildId",
            "referenceSkillLevel",
            "extractorSha256",
            "sourceFiles",
            "coverage",
            "entries",
        ),
        "balance provenance receipt",
    )
    if receipt["schema"] != "lostark.balance-provenance-receipt":
        raise PipelineError("balance provenance receipt schema mismatch")
    projected_documents = {
        relative: json.loads(text, object_pairs_hook=_reject_duplicate_pairs)
        for relative, text in projected_outputs.items()
        if relative in (ENCOUNTER_REL, COMBAT_PRODUCT_REL, BOSS_PROFILES_REL, DAMAGE_REL)
    }
    changed = 0
    seen: set[tuple[str, str, str]] = set()
    for entry in receipt["entries"]:
        key = (entry.get("targetDocument"), entry.get("targetId"), entry.get("targetField"))
        if key in seen:
            raise PipelineError(f"duplicate balance provenance entry: {key}")
        seen.add(key)
        document = projected_documents.get(entry.get("targetDocument"))
        if document is None:
            continue
        result_value = _receipt_entry_value(document, entry)
        if entry.get("resultValue") == result_value:
            continue
        entry["basis"] = "PROJECT_TUNED"
        entry["source"] = {
            "type": "project-policy",
            "policyId": "balance-tool-authored-override-v1",
        }
        entry["sourceValue"] = copy.deepcopy(result_value)
        entry["transform"] = "Balance Tool authored override"
        entry["resultValue"] = copy.deepcopy(result_value)
        entry["note"] = (
            "Changed through the F1 Balance Tool; re-export official sources to restore "
            "an official basis."
        )
        changed += 1
    if receipt["coverage"].get("fieldEntryCount") != len(receipt["entries"]):
        raise PipelineError("balance provenance fieldEntryCount drift")
    for entry in receipt["entries"]:
        document = projected_documents.get(entry["targetDocument"])
        if document is not None and entry["resultValue"] != _receipt_entry_value(document, entry):
            raise PipelineError(
                f"balance provenance result mismatch after synchronization: "
                f"{entry['targetId']}/{entry['targetField']}"
            )
    return json_text(receipt) if changed else source_text


def _write_fsync(path: Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("wb") as stream:
        stream.write(data)
        stream.flush()
        os.fsync(stream.fileno())


def _fsync_directory(path: Path) -> None:
    if os.name == "nt":
        return
    descriptor = os.open(path, os.O_RDONLY)
    try:
        os.fsync(descriptor)
    finally:
        os.close(descriptor)


def _artifact_manifest(stage: Path) -> list[dict[str, Any]]:
    entries = []
    for path in sorted(row for row in stage.rglob("*") if row.is_file()):
        relative = path.relative_to(stage).as_posix()
        entries.append({"path": relative, "sha256": sha256_file(path), "bytes": path.stat().st_size})
    return entries


def _manifest_hash(entries: Sequence[dict[str, Any]]) -> str:
    return sha256_bytes(
        "".join(f"{row['path']}\0{row['sha256']}\n" for row in entries).encode("utf-8")
    )


def _powershell_executable() -> str:
    """Resolve the checked-in publisher host without relying on a shell."""

    candidates = ("powershell.exe", "powershell", "pwsh.exe", "pwsh")
    for candidate in candidates:
        resolved = shutil.which(candidate)
        if resolved is not None:
            return resolved
    raise PipelineError("PowerShell is required to publish the gameplay bootstrap")


def _publish_gameplay_bootstrap(
    root: Path,
    output_root: Path,
    *,
    input_overlay_root: Path | None,
) -> Path:
    """Run the canonical gameplay publisher against a staged overlay.

    Publish mode performs the same validation as Validate mode before its
    transactional write, so a successful return is both publisher admission
    and materialization of the exact bytes the Server loader consumes.
    """

    script = repo_path(root, "Tools/GameplayPipeline/Publish-GameplayBalance.ps1")
    command = [
        _powershell_executable(),
        "-NoProfile",
        "-ExecutionPolicy",
        "Bypass",
        "-File",
        str(script),
        "-Mode",
        "Publish",
        "-OutputRoot",
        str(output_root),
        "-SkipValtanSplitProjection",
    ]
    if input_overlay_root is not None:
        command.extend(("-InputOverlayRoot", str(input_overlay_root)))
    completed = subprocess.run(
        command,
        cwd=root,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        timeout=180,
        creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
        check=False,
    )
    if completed.returncode != 0:
        detail = (completed.stderr.strip() or completed.stdout.strip())[-4000:]
        raise PipelineError(
            "gameplay publisher rejected the staged Valtan candidate"
            + (f": {detail}" if detail else "")
        )
    bootstrap = output_root / "Gameplay.bootstrap"
    if not bootstrap.is_file():
        raise PipelineError("gameplay publisher succeeded without Gameplay.bootstrap")
    return bootstrap


def _parse_gameplay_bootstrap(path: Path) -> tuple[int, list[str]]:
    data = path.read_bytes()
    if data.startswith(b"\xef\xbb\xbf"):
        raise PipelineError(f"Gameplay.bootstrap must not contain a UTF-8 BOM: {path}")
    try:
        lines = data.decode("utf-8", errors="strict").splitlines()
    except UnicodeDecodeError as exc:
        raise PipelineError(f"Gameplay.bootstrap is not strict UTF-8: {path}") from exc
    if not lines:
        raise PipelineError(f"Gameplay.bootstrap is empty: {path}")
    header = lines[0].split("\t")
    if len(header) != 3 or header[0] != "LOSTARK_GAMEPLAY_BOOTSTRAP":
        raise PipelineError(f"Gameplay.bootstrap header is invalid: {path}")
    try:
        version = int(header[1])
        declared_count = int(header[2])
    except ValueError as exc:
        raise PipelineError(f"Gameplay.bootstrap header numbers are invalid: {path}") from exc
    rows = lines[1:]
    if (
        version != GAMEPLAY_BOOTSTRAP_VERSION
        or declared_count != len(rows)
        or not rows
        or len(rows) > 4096
        or any(not row for row in rows)
    ):
        raise PipelineError(
            f"Gameplay.bootstrap version/count is invalid: version={version} "
            f"declared={declared_count} actual={len(rows)}"
        )
    return version, rows


def _is_valtan_gameplay_row(row: str) -> bool:
    fields = row.split("\t")
    if len(fields) < 2:
        return False
    kind, owner = fields[0], fields[1]
    if kind == "DAMAGE":
        return owner.startswith("damage.valtan.")
    if kind in ("BOSS", "BOSSARMOR", "BOSSPART"):
        return owner in ("BOSS_VALTAN", "BOSS_VALTAN_GHOST")
    if kind in ("BOSSCOMBATOBJECT", "BOSSCOMBATOBJECTHIT"):
        return owner == "ENCOUNTER_VALTAN"
    if kind == "ENCOUNTERINTRO" or kind.startswith("PATTERN"):
        return owner == "ENCOUNTER_VALTAN"
    return kind.startswith("VALTANTIMELINE")


def validate_valtan_only_bootstrap_diff(
    baseline_path: Path,
    candidate_path: Path,
) -> dict[str, Any]:
    """Reject any staged bootstrap delta outside the Valtan-owned row space."""

    baseline_version, baseline_rows = _parse_gameplay_bootstrap(baseline_path)
    candidate_version, candidate_rows = _parse_gameplay_bootstrap(candidate_path)
    if candidate_version != baseline_version:
        raise PipelineError("candidate Gameplay.bootstrap version drifted from baseline")
    baseline = Counter(baseline_rows)
    candidate = Counter(candidate_rows)
    removed = list((baseline - candidate).elements())
    added = list((candidate - baseline).elements())
    forbidden = sorted(
        {row for row in (*removed, *added) if not _is_valtan_gameplay_row(row)}
    )
    if forbidden:
        sample = " | ".join(forbidden[:3])
        raise PipelineError(
            "candidate Gameplay.bootstrap changed rows outside VALTAN_BOSS: " + sample
        )
    return {
        "formatVersion": candidate_version,
        "baselineRowCount": len(baseline_rows),
        "candidateRowCount": len(candidate_rows),
        "removedValtanRows": len(removed),
        "addedValtanRows": len(added),
        "baselineSha256": sha256_file(baseline_path),
        "candidateSha256": sha256_file(candidate_path),
    }


def _preserve_byte_identical_client_products(
    root: Path,
    outputs: dict[str, str],
) -> None:
    """Keep aliasable presentation bytes exact, not merely JSON-equivalent."""

    for relative in (BINDINGS_REL, CUES_REL, COMBAT_PRODUCT_REL, WORLD_PRODUCT_REL):
        projected = json.loads(outputs[relative], object_pairs_hook=_reject_duplicate_pairs)
        source_path = repo_path(root, relative)
        source = read_json(source_path)
        if projected != source:
            raise PipelineError(
                "candidate requires a new client presentation generation: " + relative
            )
        outputs[relative] = read_text(source_path)


def _client_presentation_compatibility(
    root: Path,
    stage: Path,
    client_artifacts: Sequence[tuple[str, str]],
) -> dict[str, Any]:
    lanes = ["ANIMATION", "EFFECT", "COMBAT_VISUAL", "CAMERA", "WORLD_EVENT_SET"]
    artifacts = []
    covered_lanes: set[str] = set()
    for lane, relative in sorted(client_artifacts, key=lambda row: (row[0], row[1])):
        if lane not in lanes:
            raise PipelineError("client presentation artifact has an unknown lane: " + lane)
        staged_path = stage / relative
        source_path = repo_path(root, relative)
        staged_sha = sha256_file(staged_path)
        source_sha = sha256_file(source_path)
        if staged_path.read_bytes() != source_path.read_bytes() or staged_sha != source_sha:
            raise PipelineError(
                "BYTE_IDENTICAL_TO_ACTIVE presentation artifact changed: " + relative
            )
        artifacts.append(
            {
                "lane": lane,
                "path": relative,
                "sha256": staged_sha,
                "bytes": staged_path.stat().st_size,
                "repositorySourceSha256": source_sha,
            }
        )
        covered_lanes.add(lane)
    missing_lanes = sorted(set(lanes) - covered_lanes)
    if missing_lanes:
        raise PipelineError(
            "BYTE_IDENTICAL_TO_ACTIVE lacks required presentation lanes: "
            + ", ".join(missing_lanes)
        )
    return {
        "mode": "BYTE_IDENTICAL_TO_ACTIVE",
        "requiredLanes": lanes,
        "artifacts": artifacts,
    }


def validate_candidate_revision_manifest(stage: Path, manifest: dict[str, Any]) -> None:
    """Strictly admit the one manifest shape consumed by Client and Server 2PC."""

    exact(
        manifest,
        (
            "schema",
            "formatVersion",
            "revisionId",
            "revisionIdentity",
            "sourceManifestId",
            "authoringBaseRevision",
            "artifactSetId",
            "draftPatchOperationCount",
            "allowedDomains",
            "requiredPresentationLanes",
            "clientPresentationCompatibility",
            "serverGameplayBootstrap",
            "applyClass",
            "runtimeActivation",
            "serverSubmanifestSha256",
            "clientSubmanifestSha256",
            "authoringSubmanifestSha256",
            "artifacts",
        ),
        "candidate revision manifest",
    )
    if (
        manifest["schema"] != "lostark.valtan-tuning-revision-manifest"
        or manifest["formatVersion"] != 1
        or manifest["allowedDomains"] != ["VALTAN_BOSS"]
        or manifest["applyClass"] not in CANDIDATE_APPLY_CLASSES
        or manifest["runtimeActivation"] != "SERVER_2PC_TICK_BOUNDARY"
    ):
        raise PipelineError("candidate revision activation contract is invalid")
    for field in (
        "revisionId",
        "sourceManifestId",
        "authoringBaseRevision",
        "artifactSetId",
        "serverSubmanifestSha256",
        "clientSubmanifestSha256",
        "authoringSubmanifestSha256",
    ):
        if not isinstance(manifest[field], str) or not re.fullmatch(
            r"[0-9a-f]{64}", manifest[field]
        ):
            raise PipelineError(f"candidate revision {field} is not a SHA-256")
    integer(
        manifest["draftPatchOperationCount"],
        "candidate revision draftPatchOperationCount",
        0,
        100000,
    )

    identity = manifest["revisionIdentity"]
    exact(
        identity,
        (
            "kind",
            "algorithm",
            "identityPayloadPath",
            "serverBootstrapContentRevision",
        ),
        "candidate revision identity",
    )
    if (
        identity["kind"] != "PARENT_MANIFEST"
        or identity["algorithm"]
        != "SHA256_CANONICAL_JSON_WITH_EMPTY_REVISION_ID"
        or identity["identityPayloadPath"] != "revision-identity.json"
        or not isinstance(identity["serverBootstrapContentRevision"], str)
        or not re.fullmatch(r"[0-9a-f]{64}", identity["serverBootstrapContentRevision"])
    ):
        raise PipelineError("candidate parent revision identity is invalid")

    required_lanes = [
        "ANIMATION",
        "EFFECT",
        "COMBAT_VISUAL",
        "CAMERA",
        "WORLD_EVENT_SET",
    ]
    if manifest["requiredPresentationLanes"] != required_lanes:
        raise PipelineError("candidate required presentation lanes are invalid")
    compatibility = manifest["clientPresentationCompatibility"]
    exact(
        compatibility,
        ("mode", "requiredLanes", "artifacts"),
        "candidate client presentation compatibility",
    )
    if (
        compatibility["mode"] != "BYTE_IDENTICAL_TO_ACTIVE"
        or compatibility["requiredLanes"] != required_lanes
        or not isinstance(compatibility["artifacts"], list)
        or not compatibility["artifacts"]
    ):
        raise PipelineError("candidate client presentation compatibility is invalid")
    covered_lanes: set[str] = set()
    compatibility_paths: set[str] = set()
    for row in compatibility["artifacts"]:
        exact(
            row,
            ("lane", "path", "sha256", "bytes", "repositorySourceSha256"),
            "candidate client presentation artifact",
        )
        if (
            row["lane"] not in required_lanes
            or not isinstance(row["path"], str)
            or not row["path"]
            or "\\" in row["path"]
            or row["path"].startswith("/")
            or ".." in row["path"].split("/")
            or row["path"] in compatibility_paths
            or not isinstance(row["sha256"], str)
            or not re.fullmatch(r"[0-9a-f]{64}", row["sha256"])
            or not isinstance(row["repositorySourceSha256"], str)
            or not re.fullmatch(
                r"[0-9a-f]{64}", row["repositorySourceSha256"]
            )
        ):
            raise PipelineError("candidate presentation artifact lane is invalid")
        integer(
            row["bytes"],
            "candidate presentation artifact bytes",
            0,
            2**63 - 1,
        )
        if row["sha256"] != row["repositorySourceSha256"]:
            raise PipelineError("candidate presentation alias hash is not byte-identical")
        covered_lanes.add(row["lane"])
        compatibility_paths.add(row["path"])
    if covered_lanes != set(required_lanes):
        raise PipelineError("candidate presentation lane coverage is incomplete")

    bootstrap = manifest["serverGameplayBootstrap"]
    exact(
        bootstrap,
        (
            "path",
            "formatVersion",
            "baselineRowCount",
            "candidateRowCount",
            "removedValtanRows",
            "addedValtanRows",
            "baselineSha256",
            "candidateSha256",
        ),
        "candidate server gameplay bootstrap",
    )
    if (
        bootstrap["path"] != GAMEPLAY_BOOTSTRAP_REL
        or bootstrap["formatVersion"] != GAMEPLAY_BOOTSTRAP_VERSION
        or bootstrap["candidateSha256"]
        != identity["serverBootstrapContentRevision"]
    ):
        raise PipelineError("candidate server gameplay bootstrap identity is invalid")
    for field in ("baselineSha256", "candidateSha256"):
        if not isinstance(bootstrap[field], str) or not re.fullmatch(
            r"[0-9a-f]{64}", bootstrap[field]
        ):
            raise PipelineError(f"candidate server gameplay {field} is invalid")
    for field in (
        "baselineRowCount",
        "candidateRowCount",
        "removedValtanRows",
        "addedValtanRows",
    ):
        integer(bootstrap[field], f"candidate server gameplay {field}", 0, 4096)

    artifacts = manifest["artifacts"]
    if not isinstance(artifacts, list) or not artifacts:
        raise PipelineError("candidate revision artifact list is empty")
    seen_paths: set[str] = set()
    artifact_by_path: dict[str, dict[str, Any]] = {}
    for row in artifacts:
        exact(row, ("path", "sha256", "bytes"), "candidate revision artifact")
        relative = row["path"]
        if (
            not isinstance(relative, str)
            or not relative
            or "\\" in relative
            or relative.startswith("/")
            or ".." in relative.split("/")
            or relative in seen_paths
            or relative in ("revision-identity.json", "revision-manifest.json")
            or not isinstance(row["sha256"], str)
            or not re.fullmatch(r"[0-9a-f]{64}", row["sha256"])
        ):
            raise PipelineError("candidate revision artifact path is invalid")
        integer(row["bytes"], "candidate revision artifact bytes", 0, 2**63 - 1)
        seen_paths.add(relative)
        artifact_by_path[relative] = row
        artifact_path = stage / relative
        _assert_transaction_path(
            stage, artifact_path, "candidate revision artifact"
        )
        if (
            not artifact_path.is_file()
            or _is_reparse_point(artifact_path)
            or sha256_file(artifact_path) != row["sha256"]
            or artifact_path.stat().st_size != row["bytes"]
        ):
            raise PipelineError("candidate revision artifact hash/size mismatch: " + relative)
    if manifest["artifactSetId"] != _manifest_hash(artifacts):
        raise PipelineError("candidate revision artifactSetId mismatch")
    authoring_gameplay_path = stage / "Authoring/Valtan.gameplay.json"
    if authoring_gameplay_path.is_file():
        authoring_gameplay = read_json(authoring_gameplay_path)
        sequence = authoring_gameplay.get("decisionModel", {}).get("scriptedSequence")
        if isinstance(sequence, dict) and "flowId" in sequence:
            if SAVED_FLOW_REL not in artifact_by_path:
                raise PipelineError("candidate Flow reference has no immutable Flow artifact")
            resolved_gameplay = resolve_gameplay_flow_reference(
                authoring_gameplay, read_saved_flow_document(stage)
            )
            if read_json(stage / ROTATIONS_REL).get("scriptedSequence") != (
                resolved_gameplay["decisionModel"]["scriptedSequence"]
            ):
                raise PipelineError("candidate saved Flow order does not match its Product sequence")
    for row in compatibility["artifacts"]:
        artifact = artifact_by_path.get(row["path"])
        if (
            artifact is None
            or artifact["sha256"] != row["sha256"]
            or artifact["bytes"] != row["bytes"]
        ):
            raise PipelineError(
                "candidate presentation alias is not bound to its immutable artifact"
            )
    for field, relative in (
        ("serverSubmanifestSha256", "_manifest/server.json"),
        ("clientSubmanifestSha256", "_manifest/client.json"),
        ("authoringSubmanifestSha256", "_manifest/authoring.json"),
    ):
        if manifest[field] != sha256_file(stage / relative):
            raise PipelineError(f"candidate revision {field} mismatch")

    identity_payload = copy.deepcopy(manifest)
    identity_payload["revisionId"] = ""
    expected_payload = canonical_bytes(identity_payload)
    identity_path = stage / identity["identityPayloadPath"]
    if (
        not identity_path.is_file()
        or _is_reparse_point(identity_path)
        or identity_path.read_bytes() != expected_payload
        or sha256_bytes(expected_payload) != manifest["revisionId"]
    ):
        raise PipelineError("candidate revision identity payload mismatch")


def _failure(point: str | None, expected: str) -> None:
    if point == expected:
        raise InjectedFailure(f"failure injection: {expected}")


HARD_CRASH_EXIT_CODE = 86


def _hard_crash(point: str | None, expected: str) -> None:
    if point == expected:
        os._exit(HARD_CRASH_EXIT_CODE)


def _transaction_contract(kind: str) -> dict[str, str]:
    if kind == "authoring":
        return {
            "lockName": ".save.lock",
            "lockSchema": "lostark.valtan-tuning-authoring-save-lock",
            "journalName": ".save-journal.json",
            "journalSchema": "lostark.valtan-tuning-authoring-save-journal",
            "pointerName": "current-authoring.json",
            "pointerSchema": "lostark.valtan-tuning-authoring-pointer",
            "pointerStagePrefix": ".current-authoring.stage.",
            "manifestName": "authoring-manifest.json",
        }
    if kind == "candidate":
        return {
            "lockName": ".publish.lock",
            "lockSchema": "lostark.valtan-tuning-publish-lock",
            "journalName": ".publish-journal.json",
            "journalSchema": "lostark.valtan-tuning-publish-journal",
            "pointerName": "current-candidate.json",
            "pointerSchema": "lostark.valtan-tuning-candidate-pointer",
            "pointerStagePrefix": ".current-candidate.stage.",
            "manifestName": "revision-manifest.json",
        }
    raise PipelineError(f"unknown durable transaction kind: {kind}")


def _parse_json_bytes(data: bytes, context: str) -> Any:
    if data.startswith(b"\xef\xbb\xbf"):
        raise PipelineError(f"UTF-8 BOM is forbidden: {context}")
    try:
        return json.loads(
            data.decode("utf-8", errors="strict"),
            object_pairs_hook=_reject_duplicate_pairs,
        )
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise PipelineError(f"invalid strict JSON: {context}") from exc


def _validate_pointer_bytes(data: bytes, kind: str) -> dict[str, Any]:
    contract = _transaction_contract(kind)
    pointer = _parse_json_bytes(data, f"{kind} pointer")
    exact(
        pointer,
        ("schema", "formatVersion", "revisionId", "manifest", "activeRuntimeChanged"),
        f"{kind} pointer",
    )
    revision_id = pointer["revisionId"]
    if (
        pointer["schema"] != contract["pointerSchema"]
        or pointer["formatVersion"] != 1
        or not isinstance(revision_id, str)
        or not re.fullmatch(r"[0-9a-f]{64}", revision_id)
        or pointer["manifest"]
        != f"revisions/{revision_id}/{contract['manifestName']}"
        or pointer["activeRuntimeChanged"] is not False
    ):
        raise PipelineError(f"{kind} pointer contract is invalid")
    return pointer


def _encode_previous_pointer(data: bytes | None, kind: str) -> dict[str, str] | None:
    if data is None:
        return None
    _validate_pointer_bytes(data, kind)
    return {
        "sha256": sha256_bytes(data),
        "bytesBase64": base64.b64encode(data).decode("ascii"),
    }


def _decode_previous_pointer(value: Any, kind: str) -> bytes | None:
    if value is None:
        return None
    exact(value, ("sha256", "bytesBase64"), f"{kind} journal previousPointer")
    if (
        not isinstance(value["sha256"], str)
        or not re.fullmatch(r"[0-9a-f]{64}", value["sha256"])
        or not isinstance(value["bytesBase64"], str)
    ):
        raise PipelineError(f"{kind} journal previousPointer fields are invalid")
    try:
        data = base64.b64decode(value["bytesBase64"], validate=True)
    except (ValueError, base64.binascii.Error) as exc:
        raise PipelineError(f"{kind} journal previousPointer base64 is invalid") from exc
    if sha256_bytes(data) != value["sha256"]:
        raise PipelineError(f"{kind} journal previousPointer hash mismatch")
    _validate_pointer_bytes(data, kind)
    return data


def _process_is_alive(pid: int) -> bool:
    if pid == os.getpid():
        return True
    if os.name == "nt":
        kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
        open_process = kernel32.OpenProcess
        open_process.argtypes = (ctypes.c_uint32, ctypes.c_int, ctypes.c_uint32)
        open_process.restype = ctypes.c_void_p
        get_exit_code = kernel32.GetExitCodeProcess
        get_exit_code.argtypes = (ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint32))
        get_exit_code.restype = ctypes.c_int
        close_handle = kernel32.CloseHandle
        close_handle.argtypes = (ctypes.c_void_p,)
        close_handle.restype = ctypes.c_int
        handle = open_process(0x1000, 0, pid)
        if not handle:
            # Access denied means the owner may still be alive; only an invalid
            # or already-gone PID is safe to classify as stale.
            return ctypes.get_last_error() not in (87, 1168)
        try:
            exit_code = ctypes.c_uint32()
            return bool(get_exit_code(handle, ctypes.byref(exit_code))) and exit_code.value == 259
        finally:
            close_handle(handle)
    try:
        os.kill(pid, 0)
        return True
    except ProcessLookupError:
        return False
    except PermissionError:
        return True


def _rewrite_open_file(descriptor: int, data: bytes) -> None:
    os.lseek(descriptor, 0, os.SEEK_SET)
    os.ftruncate(descriptor, 0)
    offset = 0
    while offset < len(data):
        offset += os.write(descriptor, data[offset:])
    os.fsync(descriptor)


def _acquire_transaction_lock(
    transaction_root: Path,
    kind: str,
) -> tuple[int, dict[str, Any], str]:
    contract = _transaction_contract(kind)
    path = transaction_root / contract["lockName"]
    _assert_transaction_path(transaction_root, path, f"{kind} transaction lock")
    transaction_id = uuid.uuid4().hex
    document = {
        "schema": contract["lockSchema"],
        "formatVersion": 1,
        "rootPath": str(transaction_root),
        "journalPath": str(transaction_root / contract["journalName"]),
        "pid": os.getpid(),
        "transactionId": transaction_id,
        "journalFileSha256": "",
    }
    try:
        descriptor = os.open(path, os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600)
    except FileExistsError as exc:
        raise PipelineError(f"{kind} transaction lock is already held: {path}") from exc
    try:
        _rewrite_open_file(descriptor, json_text(document).encode("utf-8"))
        _fsync_directory(transaction_root)
        return descriptor, document, transaction_id
    except Exception:
        os.close(descriptor)
        path.unlink(missing_ok=True)
        raise


def _seal_journal(document: dict[str, Any]) -> dict[str, Any]:
    sealed = copy.deepcopy(document)
    sealed["journalSha256"] = ""
    sealed["journalSha256"] = sha256_bytes(canonical_bytes(sealed))
    return sealed


def _write_atomic_journal(
    journal_path: Path,
    document: dict[str, Any],
    *,
    crash_at: str | None = None,
    crash_point: str | None = None,
) -> dict[str, Any]:
    sealed = _seal_journal(document)
    transaction_id = sealed["transactionId"]
    journal_stage = journal_path.with_name(
        journal_path.name + ".stage." + transaction_id
    )
    if journal_stage.exists() or _is_reparse_point(journal_stage):
        raise PipelineError("durable journal stage already exists")
    _write_fsync(journal_stage, json_text(sealed).encode("utf-8"))
    if crash_point is not None:
        _hard_crash(crash_at, crash_point)
    if journal_path.exists() and (
        _is_reparse_point(journal_path) or not journal_path.is_file()
    ):
        raise PipelineError("durable journal is not a regular file")
    os.replace(journal_stage, journal_path)
    _fsync_directory(journal_path.parent)
    return sealed


def _validate_authoring_artifact(directory: Path, revision_id: str) -> dict[str, Any]:
    if _is_reparse_point(directory) or not directory.is_dir():
        raise PipelineError("authoring transaction artifact is not a regular directory")
    manifest_path = directory / "authoring-manifest.json"
    if _is_reparse_point(manifest_path) or not manifest_path.is_file():
        raise PipelineError("authoring transaction manifest is not a regular file")
    manifest = read_json(manifest_path)
    exact(
        manifest,
        (
            "schema",
            "formatVersion",
            "revisionId",
            "baseRevision",
            "repositorySourceRevision",
            "artifactSetId",
            "draftPatchOperationCount",
            "artifacts",
        ),
        "recovery authoring manifest",
    )
    if (
        manifest["schema"] != "lostark.valtan-tuning-authoring-manifest"
        or manifest["formatVersion"] != 1
        or manifest["revisionId"] != revision_id
    ):
        raise PipelineError("recovery authoring manifest header mismatch")
    for field in ("revisionId", "baseRevision", "repositorySourceRevision", "artifactSetId"):
        if not isinstance(manifest[field], str) or not re.fullmatch(
            r"[0-9a-f]{64}", manifest[field]
        ):
            raise PipelineError(f"recovery authoring manifest {field} is invalid")
    integer(
        manifest["draftPatchOperationCount"],
        "recovery authoring draftPatchOperationCount",
        0,
        100000,
    )
    artifacts = manifest["artifacts"]
    if not isinstance(artifacts, list):
        raise PipelineError("recovery authoring artifacts are invalid")
    paths: set[str] = set()
    for row in artifacts:
        exact(row, ("path", "sha256", "bytes"), "recovery authoring artifact")
        relative = row["path"]
        artifact = directory / relative
        _assert_transaction_path(
            directory, artifact, "authoring revision artifact"
        )
        if (
            relative not in AUTHORING_ARTIFACTS
            or relative in paths
            or not isinstance(row["sha256"], str)
            or not re.fullmatch(r"[0-9a-f]{64}", row["sha256"])
            or isinstance(row["bytes"], bool)
            or not isinstance(row["bytes"], int)
            or row["bytes"] < 0
            or not artifact.is_file()
            or _is_reparse_point(artifact)
            or sha256_file(artifact) != row["sha256"]
            or artifact.stat().st_size != row["bytes"]
        ):
            raise PipelineError("recovery authoring artifact hash/path mismatch")
        paths.add(relative)
    expected_paths = set(
        AUTHORING_ARTIFACTS if SAVED_FLOW_REL in paths else LEGACY_AUTHORING_ARTIFACTS
    )
    if paths != expected_paths or _manifest_hash(artifacts) != manifest["artifactSetId"]:
        raise PipelineError("recovery authoring artifact set mismatch")
    resolve_gameplay_flow_reference(
        read_json(directory / GAMEPLAY_AUTHORING_REL),
        read_saved_flow_document(directory) if SAVED_FLOW_REL in paths else None,
    )
    expected_revision = sha256_bytes(
        (
            manifest["repositorySourceRevision"]
            + "\n"
            + manifest["baseRevision"]
            + "\n"
            + manifest["artifactSetId"]
            + "\n"
        ).encode("utf-8")
    )
    if expected_revision != revision_id:
        raise PipelineError("recovery authoring revision hash mismatch")
    return manifest


def _validate_transaction_artifact(
    directory: Path,
    revision_id: str,
    kind: str,
) -> dict[str, Any]:
    if kind == "authoring":
        return _validate_authoring_artifact(directory, revision_id)
    if _is_reparse_point(directory) or not directory.is_dir():
        raise PipelineError("candidate transaction artifact is not a regular directory")
    manifest_path = directory / "revision-manifest.json"
    if _is_reparse_point(manifest_path) or not manifest_path.is_file():
        raise PipelineError("candidate transaction manifest is not a regular file")
    manifest = read_json(manifest_path)
    if manifest.get("revisionId") != revision_id:
        raise PipelineError("recovery candidate revision ID mismatch")
    validate_candidate_revision_manifest(directory, manifest)
    return manifest


def _validate_bound_transaction_artifact(
    directory: Path,
    journal: dict[str, Any],
    kind: str,
) -> dict[str, Any]:
    manifest = _validate_transaction_artifact(
        directory, journal["revisionId"], kind
    )
    source_field = (
        "repositorySourceRevision" if kind == "authoring" else "sourceManifestId"
    )
    if manifest.get(source_field) != journal["repositorySourceRevision"]:
        raise PipelineError(
            f"{kind} durable journal source revision is not bound to its artifact"
        )
    return manifest


def _expected_pointer(kind: str, revision_id: str) -> dict[str, Any]:
    contract = _transaction_contract(kind)
    return {
        "schema": contract["pointerSchema"],
        "formatVersion": 1,
        "revisionId": revision_id,
        "manifest": f"revisions/{revision_id}/{contract['manifestName']}",
        "activeRuntimeChanged": False,
    }


def _validate_lock_document(
    lock_path: Path,
    transaction_root: Path,
    kind: str,
) -> dict[str, Any]:
    contract = _transaction_contract(kind)
    document = read_json(lock_path)
    exact(
        document,
        (
            "schema",
            "formatVersion",
            "rootPath",
            "journalPath",
            "pid",
            "transactionId",
            "journalFileSha256",
        ),
        f"{kind} durable lock",
    )
    if (
        document["schema"] != contract["lockSchema"]
        or document["formatVersion"] != 1
        or document["rootPath"] != str(transaction_root)
        or document["journalPath"]
        != str(transaction_root / contract["journalName"])
        or isinstance(document["pid"], bool)
        or not isinstance(document["pid"], int)
        or document["pid"] <= 0
        or document["pid"] > 2**31 - 1
        or not isinstance(document["transactionId"], str)
        or not re.fullmatch(r"[0-9a-f]{32}", document["transactionId"])
        # New locks remain immutable and leave this field empty.  A valid hash
        # is accepted only to recover transactions left by the earlier mutable
        # lock format; it is advisory because exact binding created an
        # unrecoverable journal/lock update window.
        or not isinstance(document["journalFileSha256"], str)
        or (
            document["journalFileSha256"] != ""
            and not re.fullmatch(r"[0-9a-f]{64}", document["journalFileSha256"])
        )
    ):
        raise PipelineError(f"{kind} durable lock contract is invalid")
    return document


def _validate_journal_document(
    journal_path: Path,
    transaction_root: Path,
    kind: str,
) -> tuple[dict[str, Any], Path, Path, Path, bytes | None]:
    contract = _transaction_contract(kind)
    raw_document = journal_path.read_bytes()
    document = read_json(journal_path)
    if raw_document != json_text(document).encode("utf-8"):
        raise PipelineError(f"{kind} durable journal bytes are not canonical")
    exact(
        document,
        (
            "schema",
            "formatVersion",
            "transactionId",
            "stagePath",
            "targetPath",
            "pointerStagePath",
            "revisionId",
            "repositorySourceRevision",
            "targetExisted",
            "previousPointer",
            "state",
            "journalSha256",
        ),
        f"{kind} durable journal",
    )
    transaction_id = document["transactionId"]
    revision_id = document["revisionId"]
    if (
        document["schema"] != contract["journalSchema"]
        or document["formatVersion"] != 1
        or not isinstance(transaction_id, str)
        or not re.fullmatch(r"[0-9a-f]{32}", transaction_id)
        or not isinstance(revision_id, str)
        or not re.fullmatch(r"[0-9a-f]{64}", revision_id)
        or not isinstance(document["repositorySourceRevision"], str)
        or not re.fullmatch(r"[0-9a-f]{64}", document["repositorySourceRevision"])
        or not isinstance(document["targetExisted"], bool)
        or document["state"] not in ("STAGED", "PROMOTED")
        or not isinstance(document["journalSha256"], str)
        or not re.fullmatch(r"[0-9a-f]{64}", document["journalSha256"])
    ):
        raise PipelineError(f"{kind} durable journal contract is invalid")
    hash_input = copy.deepcopy(document)
    hash_input["journalSha256"] = ""
    if sha256_bytes(canonical_bytes(hash_input)) != document["journalSha256"]:
        raise PipelineError(f"{kind} durable journal self-hash mismatch")
    stage = transaction_root / f".stage.{transaction_id}"
    target = transaction_root / "revisions" / revision_id
    pointer_stage = transaction_root / (
        contract["pointerStagePrefix"] + transaction_id
    )
    if (
        document["stagePath"] != str(stage)
        or document["targetPath"] != str(target)
        or document["pointerStagePath"] != str(pointer_stage)
    ):
        raise PipelineError(f"{kind} durable journal path escapes its transaction root")
    for candidate, context in (
        (stage, f"{kind} durable stage"),
        (target, f"{kind} durable revision"),
        (pointer_stage, f"{kind} durable pointer stage"),
    ):
        _assert_transaction_path(transaction_root, candidate, context)
    previous_pointer = _decode_previous_pointer(document["previousPointer"], kind)
    return document, stage, target, pointer_stage, previous_pointer


def _recover_durable_transaction(
    repository_root: Path,
    transaction_root: Path,
    kind: str,
) -> None:
    contract = _transaction_contract(kind)
    if _is_reparse_point(transaction_root):
        raise PipelineError(f"{kind} transaction root must not be a reparse point")
    if not transaction_root.exists():
        return
    if not transaction_root.is_dir():
        raise PipelineError(f"{kind} transaction root is not a regular directory")
    lock_path = transaction_root / contract["lockName"]
    journal_path = transaction_root / contract["journalName"]
    _assert_transaction_path(transaction_root, lock_path, f"{kind} durable lock")
    _assert_transaction_path(transaction_root, journal_path, f"{kind} durable journal")
    if not lock_path.exists() and not journal_path.exists():
        return

    lock: dict[str, Any] | None = None
    if lock_path.exists():
        if _is_reparse_point(lock_path) or not lock_path.is_file():
            raise PipelineError(f"{kind} durable lock is not a regular file")
        lock = _validate_lock_document(lock_path, transaction_root, kind)
        if _process_is_alive(lock["pid"]):
            raise PipelineError(f"{kind} transaction lock is owned by a live process")

    if not journal_path.exists():
        if lock is None:
            raise PipelineError(f"{kind} durable transaction state is missing")
        transaction_id = lock["transactionId"]
        transient_paths = (
            transaction_root / f".stage.{transaction_id}",
            transaction_root / (contract["pointerStagePrefix"] + transaction_id),
            transaction_root / f".baseline.{transaction_id}",
            transaction_root
            / (contract["journalName"] + ".stage." + transaction_id),
        )
        for transient in transient_paths:
            _assert_transaction_path(
                transaction_root, transient, f"{kind} orphan transient"
            )
            if not transient.exists():
                continue
            if transient.is_dir():
                shutil.rmtree(transient)
            elif transient.is_file():
                transient.unlink()
            else:
                raise PipelineError(
                    f"{kind} orphan transient is not a regular file/directory"
                )
        lock_path.unlink()
        _fsync_directory(transaction_root)
        return
    if _is_reparse_point(journal_path) or not journal_path.is_file():
        raise PipelineError(f"{kind} durable journal is not a regular file")

    journal, stage, target, pointer_stage, previous_pointer = (
        _validate_journal_document(journal_path, transaction_root, kind)
    )
    if lock is not None and lock["transactionId"] != journal["transactionId"]:
        raise PipelineError(f"{kind} durable lock/journal transaction mismatch")
    journal_stage = transaction_root / (
        contract["journalName"] + ".stage." + journal["transactionId"]
    )
    _assert_transaction_path(
        transaction_root, journal_stage, f"{kind} durable journal stage"
    )
    if journal_stage.exists() and (
        _is_reparse_point(journal_stage) or not journal_stage.is_file()
    ):
        raise PipelineError(f"{kind} durable journal stage is not a regular file")
    pointer_path = transaction_root / contract["pointerName"]
    current_pointer = _pointer_bytes(pointer_path)
    if current_pointer is not None:
        _validate_pointer_bytes(current_pointer, kind)
    expected_pointer = json_text(
        _expected_pointer(kind, journal["revisionId"])
    ).encode("utf-8")

    stage_exists = stage.exists()
    target_exists = target.exists()
    if journal["state"] == "STAGED":
        if pointer_stage.exists():
            raise PipelineError(f"{kind} STAGED journal unexpectedly owns a pointer stage")
        if current_pointer != previous_pointer:
            raise PipelineError(f"{kind} STAGED recovery pointer changed externally")
        if stage_exists:
            _validate_bound_transaction_artifact(stage, journal, kind)
        if target_exists:
            _validate_bound_transaction_artifact(target, journal, kind)
        if journal["targetExisted"] and not target_exists:
            raise PipelineError(f"{kind} STAGED recovery lost its pre-existing target")
        if not journal["targetExisted"] and stage_exists and target_exists:
            raise PipelineError(f"{kind} STAGED recovery has both stage and new target")
        if stage_exists:
            shutil.rmtree(stage)
        if target_exists and not journal["targetExisted"]:
            shutil.rmtree(target)
    else:
        if stage_exists or not target_exists:
            raise PipelineError(f"{kind} PROMOTED recovery artifact state is invalid")
        _validate_bound_transaction_artifact(target, journal, kind)
        if current_pointer not in (previous_pointer, expected_pointer):
            raise PipelineError(f"{kind} PROMOTED recovery pointer changed externally")
        if pointer_stage.exists():
            if _is_reparse_point(pointer_stage) or not pointer_stage.is_file():
                raise PipelineError(f"{kind} recovery pointer stage is not a regular file")
            staged_pointer = pointer_stage.read_bytes()
            _validate_pointer_bytes(staged_pointer, kind)
            if staged_pointer != expected_pointer:
                raise PipelineError(f"{kind} recovery pointer stage content mismatch")
        source_unchanged = (
            source_manifest(repository_root)["sourceManifestId"]
            == journal["repositorySourceRevision"]
        )
        if source_unchanged:
            recovery_pointer = transaction_root / (
                contract["pointerStagePrefix"] + "recovery-" + uuid.uuid4().hex
            )
            _write_fsync(recovery_pointer, expected_pointer)
            os.replace(recovery_pointer, pointer_path)
        else:
            _restore_pointer(pointer_path, previous_pointer)
            if not journal["targetExisted"]:
                shutil.rmtree(target)
        if pointer_stage.exists():
            pointer_stage.unlink()

    if journal_stage.exists():
        journal_stage.unlink()
    if lock_path.exists():
        lock_path.unlink()
    journal_path.unlink()
    _fsync_directory(transaction_root)


def _pointer_bytes(pointer_path: Path) -> bytes | None:
    if _is_reparse_point(pointer_path):
        raise PipelineError("transaction pointer must not be a reparse point")
    if not pointer_path.exists():
        return None
    if not pointer_path.is_file():
        raise PipelineError("transaction pointer is not a regular file")
    return pointer_path.read_bytes()


def _restore_pointer(pointer_path: Path, previous: bytes | None) -> None:
    if previous is None:
        if pointer_path.exists() or _is_reparse_point(pointer_path):
            pointer_path.unlink()
    else:
        temp = pointer_path.with_name(pointer_path.name + ".restore." + uuid.uuid4().hex)
        _write_fsync(temp, previous)
        os.replace(temp, pointer_path)


def load_authoring_revision(
    root: Path,
    authoring_root: Path,
    revision_id: str,
    current_sources: dict[str, Any],
    docs: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any]]:
    if not isinstance(revision_id, str) or not re.fullmatch(r"[0-9a-f]{64}", revision_id):
        raise DraftPatchError(
            "authoring revision must be a lowercase SHA-256",
            document="authoringRevision",
            field="revisionId",
            error_code="SOURCE_REVISION_INVALID",
        )
    resolved_root = staging_root(root, authoring_root, "AuthoringRoot")
    revision_root = resolved_root / "revisions" / revision_id
    _assert_transaction_path(
        resolved_root, revision_root, "saved authoring revision"
    )
    manifest_path = revision_root / "authoring-manifest.json"
    if not manifest_path.is_file():
        raise DraftPatchError(
            f"saved authoring revision does not exist: {revision_id}",
            document="authoringRevision",
            field="revisionId",
            error_code="STABLE_ID_NOT_FOUND",
        )
    _validate_authoring_artifact(revision_root, revision_id)
    manifest = read_json(manifest_path)
    exact(
        manifest,
        (
            "schema",
            "formatVersion",
            "revisionId",
            "baseRevision",
            "repositorySourceRevision",
            "artifactSetId",
            "draftPatchOperationCount",
            "artifacts",
        ),
        "saved authoring manifest",
    )
    if (
        manifest["schema"] != "lostark.valtan-tuning-authoring-manifest"
        or manifest["formatVersion"] != 1
        or manifest["revisionId"] != revision_id
    ):
        raise PipelineError("saved authoring manifest header mismatch")
    if manifest["repositorySourceRevision"] != current_sources["sourceManifestId"]:
        raise DraftPatchError(
            "repository source changed after this authoring revision was saved",
            document="authoringRevision",
            field="repositorySourceRevision",
            error_code="SOURCE_REVISION_MISMATCH",
        )
    artifacts = manifest["artifacts"]
    if not isinstance(artifacts, list):
        raise PipelineError("saved authoring artifacts must be an array")
    actual_paths: list[str] = []
    for ordinal, entry in enumerate(artifacts):
        exact(entry, ("path", "sha256", "bytes"), f"saved authoring artifact[{ordinal}]")
        relative = entry["path"]
        if relative not in AUTHORING_ARTIFACTS or relative in actual_paths:
            raise PipelineError(f"saved authoring artifact path is invalid/duplicate: {relative}")
        artifact_path = revision_root / relative
        if (
            not artifact_path.is_file()
            or _is_reparse_point(artifact_path)
            or sha256_file(artifact_path) != entry["sha256"]
            or artifact_path.stat().st_size != entry["bytes"]
        ):
            raise PipelineError(f"saved authoring artifact hash/size mismatch: {relative}")
        actual_paths.append(relative)
    expected_paths = (
        AUTHORING_ARTIFACTS if SAVED_FLOW_REL in actual_paths else LEGACY_AUTHORING_ARTIFACTS
    )
    if tuple(sorted(actual_paths)) != tuple(sorted(expected_paths)):
        raise PipelineError("saved authoring artifact set is incomplete")
    if _manifest_hash(artifacts) != manifest["artifactSetId"]:
        raise PipelineError("saved authoring artifact-set hash mismatch")
    expected_revision = sha256_bytes(
        (
            manifest["repositorySourceRevision"]
            + "\n"
            + manifest["baseRevision"]
            + "\n"
            + manifest["artifactSetId"]
            + "\n"
        ).encode("utf-8")
    )
    if expected_revision != revision_id:
        raise PipelineError("saved authoring revision hash mismatch")
    master = join_v2_authoring(
        read_json(revision_root / GAMEPLAY_AUTHORING_REL),
        read_json(revision_root / PRESENTATION_AUTHORING_REL),
        read_json(revision_root / WORLD_SET_REL),
        read_json(revision_root / COMBAT_AUTHORING_REL),
        read_saved_flow_document(revision_root) if SAVED_FLOW_REL in actual_paths else None,
    )
    bosses = read_json(revision_root / BOSS_PROFILES_REL)
    damage = read_json(revision_root / DAMAGE_REL)
    validate_balance_documents(bosses, damage)
    expected_receipt = json.loads(
        project_provenance_receipt(
            root, project_balance_products(root, bosses, damage)
        ),
        object_pairs_hook=_reject_duplicate_pairs,
    )
    if read_json(revision_root / PROVENANCE_REL) != expected_receipt:
        raise PipelineError("saved authoring balance provenance receipt drift")
    return master, bosses, damage


def _validated_authoring_head(
    root: Path,
    authoring_root: Path,
    current_sources: dict[str, Any],
    docs: dict[str, Any],
) -> str:
    pointer_bytes = _pointer_bytes(authoring_root / "current-authoring.json")
    if pointer_bytes is None:
        return current_sources["sourceManifestId"]
    revision_id = _validate_pointer_bytes(pointer_bytes, "authoring")["revisionId"]
    load_authoring_revision(
        root, authoring_root, revision_id, current_sources, docs
    )
    return revision_id


def resolve_authoring_base(
    root: Path,
    authoring_root: Path | None,
    revision_id: str,
    current_sources: dict[str, Any],
    docs: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any]]:
    if revision_id == current_sources["sourceManifestId"]:
        return (
            join_v2_authoring(
                docs[GAMEPLAY_AUTHORING_REL],
                docs[PRESENTATION_AUTHORING_REL],
                docs[WORLD_SET_REL],
                docs[COMBAT_AUTHORING_REL],
            ),
            copy.deepcopy(docs[BOSS_PROFILES_REL]),
            copy.deepcopy(docs[DAMAGE_REL]),
        )
    if authoring_root is None:
        raise DraftPatchError(
            "draft sourceRevision is not the repository source; AuthoringRoot is required",
            document="draftPatch",
            field="sourceRevision",
            error_code="SOURCE_REVISION_MISMATCH",
        )
    return load_authoring_revision(root, authoring_root, revision_id, current_sources, docs)


def save_authoring(
    root: Path,
    authoring_root: Path,
    draft_patch: dict[str, Any],
    *,
    fail_at: str | None = None,
    crash_at: str | None = None,
) -> dict[str, Any]:
    """Persist a v2 authoring revision without replacing the format-v1 repository source."""

    authoring_root = staging_root(root, authoring_root, "AuthoringRoot")
    authoring_root.mkdir(parents=True, exist_ok=True)
    _recover_durable_transaction(root, authoring_root, "authoring")
    lock_path = authoring_root / ".save.lock"
    lock_fd, _lock_document, transaction_id = _acquire_transaction_lock(
        authoring_root, "authoring"
    )
    pointer_path = authoring_root / "current-authoring.json"
    journal_path = authoring_root / ".save-journal.json"
    stage: Path | None = None
    promoted: Path | None = None
    pointer_stage: Path | None = None
    promoted_was_new = False
    previous_pointer: bytes | None = None
    previous_pointer_captured = False
    try:
        previous_pointer = _pointer_bytes(pointer_path)
        previous_head_revision: str | None = None
        if previous_pointer is not None:
            previous_head_revision = _validate_pointer_bytes(
                previous_pointer, "authoring"
            )["revisionId"]
        previous_pointer_captured = True
        current_sources = source_manifest(root)
        docs = load_pipeline_documents(root)
        base_revision = draft_patch.get("sourceRevision") if isinstance(draft_patch, dict) else ""
        expected_head_revision = (
            previous_head_revision or current_sources["sourceManifestId"]
        )
        if base_revision != expected_head_revision:
            raise DraftPatchError(
                "draft sourceRevision does not match the current saved authoring head",
                document="draftPatch",
                field="sourceRevision",
                error_code="SOURCE_REVISION_MISMATCH",
            )
        base_master, base_bosses, base_damage = resolve_authoring_base(
            root, authoring_root, base_revision, current_sources, docs
        )
        master, bosses, damage, operation_count = apply_draft_patch(
            base_master,
            base_bosses,
            base_damage,
            draft_patch,
            base_revision,
            docs[WORLD_SET_REL],
            docs[COMBAT_AUTHORING_REL],
        )
        stage = authoring_root / (".stage." + transaction_id)
        _assert_transaction_path(authoring_root, stage, "authoring save stage")
        stage.mkdir()
        gameplay, presentation = split_v2_authoring(
            master, docs[WORLD_SET_REL], docs[COMBAT_AUTHORING_REL]
        )
        gameplay = _restore_gameplay_flow_reference(
            gameplay, read_json(repo_path(root, GAMEPLAY_AUTHORING_REL)),
            docs.get(SAVED_FLOW_REL),
        )
        values = {
            GAMEPLAY_AUTHORING_REL: gameplay,
            PRESENTATION_AUTHORING_REL: presentation,
            BOSS_PROFILES_REL: bosses,
            DAMAGE_REL: damage,
            COMBAT_AUTHORING_REL: docs[COMBAT_AUTHORING_REL],
            WORLD_SET_REL: docs[WORLD_SET_REL],
            LEGACY_REL: docs[LEGACY_REL],
        }
        for relative, value in values.items():
            _write_fsync(stage / relative, json_text(value).encode("utf-8"))
        saved_flow_text = _snapshot_saved_flow_text(root, current_sources)
        if saved_flow_text is not None:
            _write_fsync(stage / SAVED_FLOW_REL, saved_flow_text.encode("utf-8"))
        balance_outputs = project_balance_products(root, bosses, damage)
        _write_fsync(
            stage / PROVENANCE_REL,
            project_provenance_receipt(root, balance_outputs).encode("utf-8"),
        )
        _failure(fail_at, "after_stage")
        join_v2_authoring(
            read_json(stage / GAMEPLAY_AUTHORING_REL),
            read_json(stage / PRESENTATION_AUTHORING_REL),
            read_json(stage / WORLD_SET_REL),
            read_json(stage / COMBAT_AUTHORING_REL),
            read_saved_flow_document(stage) if saved_flow_text is not None else None,
        )
        validate_balance_documents(
            read_json(stage / BOSS_PROFILES_REL), read_json(stage / DAMAGE_REL)
        )
        _failure(fail_at, "after_validate")
        artifacts = _artifact_manifest(stage)
        artifact_set_id = _manifest_hash(artifacts)
        revision_id = sha256_bytes(
            (
                current_sources["sourceManifestId"]
                + "\n"
                + base_revision
                + "\n"
                + artifact_set_id
                + "\n"
            ).encode("utf-8")
        )
        manifest = {
            "schema": "lostark.valtan-tuning-authoring-manifest",
            "formatVersion": 1,
            "revisionId": revision_id,
            "baseRevision": base_revision,
            "repositorySourceRevision": current_sources["sourceManifestId"],
            "artifactSetId": artifact_set_id,
            "draftPatchOperationCount": operation_count,
            "artifacts": artifacts,
        }
        _write_fsync(stage / "authoring-manifest.json", json_text(manifest).encode("utf-8"))
        _failure(fail_at, "after_revision_manifest")
        promoted = authoring_root / "revisions" / revision_id
        _ensure_transaction_directory(
            authoring_root, promoted.parent, "authoring revisions directory"
        )
        _assert_transaction_path(
            authoring_root, promoted, "authoring promoted revision"
        )
        journal = {
            "schema": "lostark.valtan-tuning-authoring-save-journal",
            "formatVersion": 1,
            "transactionId": transaction_id,
            "stagePath": str(stage),
            "targetPath": str(promoted),
            "pointerStagePath": str(
                authoring_root / (".current-authoring.stage." + transaction_id)
            ),
            "revisionId": revision_id,
            "repositorySourceRevision": current_sources["sourceManifestId"],
            "targetExisted": promoted.exists(),
            "previousPointer": _encode_previous_pointer(
                previous_pointer, "authoring"
            ),
            "state": "STAGED",
            "journalSha256": "",
        }
        journal = _write_atomic_journal(
            journal_path,
            journal,
            crash_at=crash_at,
            crash_point="after_staged_journal_temp",
        )
        _hard_crash(crash_at, "after_journal_staged")
        _failure(fail_at, "before_promote")
        if promoted.exists():
            existing_manifest = _validate_authoring_artifact(
                promoted, revision_id
            )
            if existing_manifest != manifest:
                raise PipelineError(f"immutable authoring revision collision: {revision_id}")
            shutil.rmtree(stage)
            stage = None
        else:
            os.replace(stage, promoted)
            stage = None
            promoted_was_new = True
            _fsync_directory(promoted.parent)
        journal["state"] = "PROMOTED"
        journal = _write_atomic_journal(
            journal_path,
            journal,
            crash_at=crash_at,
            crash_point="after_promoted_journal_temp",
        )
        _hard_crash(crash_at, "after_promote")
        _failure(fail_at, "after_promote")
        if source_manifest(root) != current_sources:
            raise PipelineError("repository source changed during authoring save")
        pointer = {
            "schema": "lostark.valtan-tuning-authoring-pointer",
            "formatVersion": 1,
            "revisionId": revision_id,
            "manifest": f"revisions/{revision_id}/authoring-manifest.json",
            "activeRuntimeChanged": False,
        }
        pointer_stage = authoring_root / (
            ".current-authoring.stage." + transaction_id
        )
        _write_fsync(pointer_stage, json_text(pointer).encode("utf-8"))
        _failure(fail_at, "before_pointer")
        os.replace(pointer_stage, pointer_path)
        pointer_stage = None
        _fsync_directory(authoring_root)
        _hard_crash(crash_at, "after_pointer")
        _failure(fail_at, "after_pointer")
        journal_path.unlink(missing_ok=True)
        _fsync_directory(authoring_root)
        _hard_crash(crash_at, "after_journal_unlink")
        return pointer
    except Exception:
        if pointer_stage is not None and pointer_stage.exists():
            _assert_transaction_path(
                authoring_root, pointer_stage, "authoring pointer-stage cleanup"
            )
            pointer_stage.unlink()
        if stage is not None and stage.exists():
            _assert_transaction_path(authoring_root, stage, "authoring stage cleanup")
            shutil.rmtree(stage, ignore_errors=True)
        if promoted_was_new and promoted is not None and promoted.exists():
            _assert_transaction_path(
                authoring_root, promoted, "authoring revision cleanup"
            )
            shutil.rmtree(promoted, ignore_errors=True)
        if previous_pointer_captured:
            _restore_pointer(pointer_path, previous_pointer)
        journal_path.unlink(missing_ok=True)
        raise
    finally:
        os.close(lock_fd)
        lock_path.unlink(missing_ok=True)


def publish_candidate(
    root: Path,
    candidate_root: Path,
    *,
    expected_source_manifest: dict[str, Any] | None = None,
    draft_patch: dict[str, Any] | None = None,
    authoring_root: Path | None = None,
    authoring_revision: str | None = None,
    fail_at: str | None = None,
    crash_at: str | None = None,
) -> dict[str, Any]:
    if authoring_root is not None:
        authoring_root = staging_root(root, authoring_root, "AuthoringRoot")
        authoring_root.mkdir(parents=True, exist_ok=True)
        _recover_durable_transaction(root, authoring_root, "authoring")
    candidate_root = staging_root(root, candidate_root, "CandidateRoot")
    candidate_root.mkdir(parents=True, exist_ok=True)
    _recover_durable_transaction(root, candidate_root, "candidate")
    lock_path = candidate_root / ".publish.lock"
    lock_fd, _lock_document, transaction_id = _acquire_transaction_lock(
        candidate_root, "candidate"
    )
    pointer_path = candidate_root / "current-candidate.json"
    journal_path = candidate_root / ".publish-journal.json"
    stage: Path | None = None
    baseline_runtime_root: Path | None = None
    promoted: Path | None = None
    pointer_stage: Path | None = None
    promoted_was_new = False
    previous_pointer: bytes | None = None
    previous_pointer_captured = False
    authoring_lock_fd: int | None = None
    authoring_lock_path: Path | None = None
    try:
        if authoring_root is not None:
            authoring_lock_path = authoring_root / ".save.lock"
            authoring_lock_fd, _authoring_lock_document, _ = (
                _acquire_transaction_lock(authoring_root, "authoring")
            )
        previous_pointer = _pointer_bytes(pointer_path)
        if previous_pointer is not None:
            _validate_pointer_bytes(previous_pointer, "candidate")
        previous_pointer_captured = True
        current_sources = source_manifest(root)
        if expected_source_manifest is not None and current_sources != expected_source_manifest:
            raise PipelineError("source hash precondition failed; authoring changed externally")
        docs = load_pipeline_documents(root)
        validate_world_event_sets(
            docs[WORLD_SET_REL],
            docs[WORLD_PRODUCT_REL],
            migration_fixture=False,
        )
        validate_combat_authoring(docs[COMBAT_AUTHORING_REL])
        repository_v2 = join_v2_authoring(
            docs[GAMEPLAY_AUTHORING_REL],
            docs[PRESENTATION_AUTHORING_REL],
            docs[WORLD_SET_REL],
            docs[COMBAT_AUTHORING_REL],
        )
        validate_legacy_manifest(
            docs[LEGACY_REL],
            {row["patternId"] for row in repository_v2["patterns"]},
        )
        requested_base_revision = authoring_revision
        if requested_base_revision is None and draft_patch is not None:
            requested_base_revision = (
                draft_patch.get("sourceRevision")
                if isinstance(draft_patch, dict)
                else ""
            )
        if requested_base_revision is None:
            requested_base_revision = current_sources["sourceManifestId"]
        if authoring_root is not None:
            current_head = _validated_authoring_head(
                root, authoring_root, current_sources, docs
            )
            if requested_base_revision != current_head:
                raise DraftPatchError(
                    "candidate base revision does not match the current saved authoring head",
                    document="draftPatch",
                    field="sourceRevision",
                    error_code="SOURCE_REVISION_MISMATCH",
                )
        base_revision = current_sources["sourceManifestId"]
        if authoring_revision is not None:
            base_revision = authoring_revision
            v2, candidate_bosses, candidate_damage = resolve_authoring_base(
                root, authoring_root, base_revision, current_sources, docs
            )
        else:
            v2 = repository_v2
            candidate_bosses = docs[BOSS_PROFILES_REL]
            candidate_damage = docs[DAMAGE_REL]
        operation_count = 0
        if draft_patch is not None:
            patch_revision = draft_patch.get("sourceRevision") if isinstance(draft_patch, dict) else ""
            if authoring_revision is not None and patch_revision != authoring_revision:
                raise DraftPatchError(
                    "DraftPatch sourceRevision does not match AuthoringRevision",
                    document="draftPatch",
                    field="sourceRevision",
                    error_code="SOURCE_REVISION_MISMATCH",
                )
            if authoring_revision is None and patch_revision != base_revision:
                base_revision = patch_revision
                v2, candidate_bosses, candidate_damage = resolve_authoring_base(
                    root, authoring_root, base_revision, current_sources, docs
                )
            v2, candidate_bosses, candidate_damage, operation_count = apply_draft_patch(
                v2,
                candidate_bosses,
                candidate_damage,
                draft_patch,
                base_revision,
                docs[WORLD_SET_REL],
                docs[COMBAT_AUTHORING_REL],
            )
        apply_class = classify_candidate_apply_class(
            docs[BOSS_PROFILES_REL], candidate_bosses
        )
        outputs = project_v2_products(root, docs, v2)
        outputs.update(project_balance_products(root, candidate_bosses, candidate_damage))
        outputs[PROVENANCE_REL] = project_provenance_receipt(root, outputs)
        _preserve_byte_identical_client_products(root, outputs)
        stage = candidate_root / (".stage." + transaction_id)
        _assert_transaction_path(candidate_root, stage, "candidate publish stage")
        stage.mkdir()
        candidate_gameplay, candidate_presentation = split_v2_authoring(
            v2, docs[WORLD_SET_REL], docs[COMBAT_AUTHORING_REL]
        )
        candidate_gameplay = _restore_gameplay_flow_reference(
            candidate_gameplay, read_json(repo_path(root, GAMEPLAY_AUTHORING_REL)),
            docs.get(SAVED_FLOW_REL),
        )
        authoring_outputs = {
            "Authoring/Valtan.gameplay.json": json_text(candidate_gameplay),
            "Authoring/Valtan.presentation.json": json_text(candidate_presentation),
            "Authoring/Valtan.combatobjects.json": json_text(docs[COMBAT_AUTHORING_REL]),
            "Authoring/Valtan.worldeventsets.json": json_text(docs[WORLD_SET_REL]),
            "Authoring/Valtan.legacy-compatibility.json": json_text(docs[LEGACY_REL]),
        }
        saved_flow_text = _snapshot_saved_flow_text(root, current_sources)
        if saved_flow_text is not None:
            authoring_outputs[SAVED_FLOW_REL] = saved_flow_text
        if draft_patch is not None:
            authoring_outputs["Authoring/Valtan.tuning-draft-patch.json"] = json_text(draft_patch)
        for relative, text in {**authoring_outputs, **outputs}.items():
            _write_fsync(stage / relative, text.encode("utf-8"))
        for relative in (CAMERA_REL, EFFECT_CATALOG_REL, BOSS_CATALOG_REL):
            _write_fsync(
                stage / relative,
                read_text(repo_path(root, relative)).encode("utf-8"),
            )
        _failure(fail_at, "after_stage")
        staged_world = read_json(stage / WORLD_PRODUCT_REL)
        if len(staged_world["groups"]) != 105 or len(staged_world["mutations"]) != 105 or len(staged_world["bindings"]) != 224:
            raise PipelineError("staged world Product count/closure drift")
        baseline_runtime_root = candidate_root / (".baseline." + transaction_id)
        _assert_transaction_path(
            candidate_root, baseline_runtime_root, "candidate baseline stage"
        )
        baseline_bootstrap = _publish_gameplay_bootstrap(
            root,
            baseline_runtime_root,
            input_overlay_root=None,
        )
        candidate_bootstrap = _publish_gameplay_bootstrap(
            root,
            stage / "Runtime/Gameplay",
            input_overlay_root=stage,
        )
        bootstrap_compatibility = validate_valtan_only_bootstrap_diff(
            baseline_bootstrap,
            candidate_bootstrap,
        )
        shutil.rmtree(baseline_runtime_root)
        baseline_runtime_root = None
        _failure(fail_at, "after_validate")
        server_paths = (
            ENCOUNTER_REL,
            ROTATIONS_REL,
            COMBAT_PRODUCT_REL,
            WORLD_PRODUCT_REL,
            BOSS_PROFILES_REL,
            DAMAGE_REL,
            GAMEPLAY_BOOTSTRAP_REL,
        )
        client_artifacts = (
            ("ANIMATION", BINDINGS_REL),
            ("EFFECT", CUES_REL),
            ("EFFECT", EFFECT_CATALOG_REL),
            ("COMBAT_VISUAL", COMBAT_PRODUCT_REL),
            ("COMBAT_VISUAL", BOSS_CATALOG_REL),
            ("CAMERA", CAMERA_REL),
            ("WORLD_EVENT_SET", WORLD_PRODUCT_REL),
        )
        client_paths = tuple(relative for _, relative in client_artifacts)
        client_compatibility = _client_presentation_compatibility(
            root,
            stage,
            client_artifacts,
        )
        authoring_paths = tuple(sorted((*authoring_outputs, PROVENANCE_REL)))
        submanifests = {
            "_manifest/server.json": {
                "schema": "lostark.valtan-tuning-server-submanifest",
                "formatVersion": 1,
                "artifacts": [
                    {"path": relative, "sha256": sha256_file(stage / relative)} for relative in sorted(server_paths)
                ],
            },
            "_manifest/client.json": {
                "schema": "lostark.valtan-tuning-client-submanifest",
                "formatVersion": 1,
                "artifacts": [
                    {"path": relative, "sha256": sha256_file(stage / relative)} for relative in sorted(client_paths)
                ],
            },
            "_manifest/authoring.json": {
                "schema": "lostark.valtan-tuning-authoring-submanifest",
                "formatVersion": 1,
                "artifacts": [
                    {"path": relative, "sha256": sha256_file(stage / relative)} for relative in authoring_paths
                ],
            },
        }
        for relative, value in submanifests.items():
            _write_fsync(stage / relative, json_text(value).encode("utf-8"))
        artifacts = _artifact_manifest(stage)
        artifact_set_id = _manifest_hash(artifacts)
        revision_manifest = {
            "schema": "lostark.valtan-tuning-revision-manifest",
            "formatVersion": 1,
            "revisionId": "",
            "revisionIdentity": {
                "kind": "PARENT_MANIFEST",
                "algorithm": "SHA256_CANONICAL_JSON_WITH_EMPTY_REVISION_ID",
                "identityPayloadPath": "revision-identity.json",
                "serverBootstrapContentRevision": bootstrap_compatibility[
                    "candidateSha256"
                ],
            },
            "sourceManifestId": current_sources["sourceManifestId"],
            "authoringBaseRevision": base_revision,
            "artifactSetId": artifact_set_id,
            "draftPatchOperationCount": operation_count,
            "allowedDomains": ["VALTAN_BOSS"],
            "requiredPresentationLanes": client_compatibility["requiredLanes"],
            "clientPresentationCompatibility": client_compatibility,
            "serverGameplayBootstrap": {
                "path": GAMEPLAY_BOOTSTRAP_REL,
                **bootstrap_compatibility,
            },
            "applyClass": apply_class,
            "runtimeActivation": "SERVER_2PC_TICK_BOUNDARY",
            "serverSubmanifestSha256": sha256_file(stage / "_manifest/server.json"),
            "clientSubmanifestSha256": sha256_file(stage / "_manifest/client.json"),
            "authoringSubmanifestSha256": sha256_file(stage / "_manifest/authoring.json"),
            "artifacts": artifacts,
        }
        identity_payload = canonical_bytes(revision_manifest)
        revision_id = sha256_bytes(identity_payload)
        revision_manifest["revisionId"] = revision_id
        _write_fsync(stage / "revision-identity.json", identity_payload)
        validate_candidate_revision_manifest(stage, revision_manifest)
        _write_fsync(stage / "revision-manifest.json", json_text(revision_manifest).encode("utf-8"))
        _failure(fail_at, "after_revision_manifest")
        promoted = candidate_root / "revisions" / revision_id
        _ensure_transaction_directory(
            candidate_root, promoted.parent, "candidate revisions directory"
        )
        _assert_transaction_path(
            candidate_root, promoted, "candidate promoted revision"
        )
        journal = {
            "schema": "lostark.valtan-tuning-publish-journal",
            "formatVersion": 1,
            "transactionId": transaction_id,
            "stagePath": str(stage),
            "targetPath": str(promoted),
            "pointerStagePath": str(
                candidate_root / (".current-candidate.stage." + transaction_id)
            ),
            "revisionId": revision_id,
            "repositorySourceRevision": current_sources["sourceManifestId"],
            "targetExisted": promoted.exists(),
            "previousPointer": _encode_previous_pointer(
                previous_pointer, "candidate"
            ),
            "state": "STAGED",
            "journalSha256": "",
        }
        journal = _write_atomic_journal(
            journal_path,
            journal,
            crash_at=crash_at,
            crash_point="after_staged_journal_temp",
        )
        _hard_crash(crash_at, "after_journal_staged")
        _failure(fail_at, "before_promote")
        if promoted.exists():
            existing = read_json(promoted / "revision-manifest.json")
            identity_path = promoted / "revision-identity.json"
            if (
                existing != revision_manifest
                or not identity_path.is_file()
                or sha256_file(identity_path) != revision_id
                or identity_path.read_bytes() != identity_payload
            ):
                raise PipelineError(f"immutable candidate collision: {revision_id}")
            validate_candidate_revision_manifest(promoted, existing)
            shutil.rmtree(stage)
            stage = None
        else:
            os.replace(stage, promoted)
            stage = None
            promoted_was_new = True
            _fsync_directory(promoted.parent)
        journal["state"] = "PROMOTED"
        journal = _write_atomic_journal(
            journal_path,
            journal,
            crash_at=crash_at,
            crash_point="after_promoted_journal_temp",
        )
        _hard_crash(crash_at, "after_promote")
        _failure(fail_at, "after_promote")
        if source_manifest(root) != current_sources:
            raise PipelineError("source changed during candidate publication")
        pointer = {
            "schema": "lostark.valtan-tuning-candidate-pointer",
            "formatVersion": 1,
            "revisionId": revision_id,
            "manifest": f"revisions/{revision_id}/revision-manifest.json",
            "activeRuntimeChanged": False,
        }
        pointer_stage = candidate_root / (
            ".current-candidate.stage." + transaction_id
        )
        _write_fsync(pointer_stage, json_text(pointer).encode("utf-8"))
        _failure(fail_at, "before_pointer")
        os.replace(pointer_stage, pointer_path)
        pointer_stage = None
        _fsync_directory(candidate_root)
        _hard_crash(crash_at, "after_pointer")
        _failure(fail_at, "after_pointer")
        journal_path.unlink(missing_ok=True)
        _fsync_directory(candidate_root)
        _hard_crash(crash_at, "after_journal_unlink")
        return pointer
    except Exception:
        if pointer_stage is not None and pointer_stage.exists():
            _assert_transaction_path(
                candidate_root, pointer_stage, "candidate pointer-stage cleanup"
            )
            pointer_stage.unlink()
        if stage is not None and stage.exists():
            _assert_transaction_path(candidate_root, stage, "candidate stage cleanup")
            shutil.rmtree(stage, ignore_errors=True)
        if baseline_runtime_root is not None and baseline_runtime_root.exists():
            _assert_transaction_path(
                candidate_root,
                baseline_runtime_root,
                "candidate baseline cleanup",
            )
            shutil.rmtree(baseline_runtime_root, ignore_errors=True)
        if promoted_was_new and promoted is not None and promoted.exists():
            _assert_transaction_path(
                candidate_root, promoted, "candidate revision cleanup"
            )
            shutil.rmtree(promoted, ignore_errors=True)
        if previous_pointer_captured:
            _restore_pointer(pointer_path, previous_pointer)
        journal_path.unlink(missing_ok=True)
        raise
    finally:
        if authoring_lock_fd is not None:
            os.close(authoring_lock_fd)
            if authoring_lock_path is not None:
                authoring_lock_path.unlink(missing_ok=True)
        os.close(lock_fd)
        lock_path.unlink(missing_ok=True)


def _require_saved_flow_revision(root: Path, expected_revision: str) -> None:
    if not isinstance(expected_revision, str) or re.fullmatch(r"[0-9a-f]{64}", expected_revision) is None:
        raise PipelineError("expectedFlowRevision must be a lowercase SHA-256")
    if sha256_file(repo_path(root, SAVED_FLOW_REL)) != expected_revision:
        raise PipelineError("saved Flow revision changed; reload the saved Flow before publishing")


def publish_saved_flow(
    root: Path,
    candidate_root: Path,
    expected_flow_revision: str,
    *,
    fail_at: str | None = None,
) -> dict[str, Any]:
    """Project the saved canonical Flow, then build an inactive immutable candidate."""

    _require_saved_flow_revision(root, expected_flow_revision)
    source_gameplay = read_json(repo_path(root, GAMEPLAY_AUTHORING_REL))
    sequence = source_gameplay.get("decisionModel", {}).get("scriptedSequence")
    if not isinstance(sequence, dict) or sequence.get("flowId") != DEFAULT_SAVED_FLOW_ID:
        raise PipelineError("Product scriptedSequence does not reference the saved default Flow")
    # Validate the reference, inventory, and full joined source before a Product
    # publisher may replace any generated document. Saved Balance overlays are
    # intentionally neither selected nor discarded by this Flow-only command.
    source_manifest(root)
    command = [
        _powershell_executable(), "-NoProfile", "-ExecutionPolicy", "Bypass",
        "-File", str(repo_path(root, "Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1")),
        "-Mode", "PublishV2", "-RepositoryRoot", str(root),
        "-ExpectedFlowRevision", expected_flow_revision,
    ]
    # The editor owns this asynchronous process and reports long-running work
    # as unconfirmed. subprocess.run(timeout=...) would kill the inner Product
    # publisher before its durable commit/rollback can finish.
    try:
        completed = subprocess.run(
            command, cwd=root, capture_output=True, text=True,
            encoding="utf-8", errors="replace", check=False,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
        )
    except OSError as exc:
        raise PipelineError(f"saved Flow Product publisher did not complete: {exc}") from exc
    if completed.returncode != 0:
        detail = (completed.stderr or completed.stdout).strip()[-4000:]
        raise PipelineError("saved Flow Product publication failed" + (f": {detail}" if detail else ""))
    _require_saved_flow_revision(root, expected_flow_revision)
    current_sources = source_manifest(root)
    pointer = publish_candidate(
        root, candidate_root, expected_source_manifest=current_sources, fail_at=fail_at
    )
    _require_saved_flow_revision(root, expected_flow_revision)
    manifest = _validate_transaction_artifact(
        staging_root(root, candidate_root, "CandidateRoot") / "revisions" / pointer["revisionId"],
        pointer["revisionId"], "candidate",
    )
    flow_artifact = next(
        (row for row in manifest["artifacts"] if row["path"] == SAVED_FLOW_REL), None
    )
    if flow_artifact is None or flow_artifact["sha256"] != expected_flow_revision:
        raise PipelineError("published candidate does not contain the requested saved Flow revision")
    return {
        "sourceRevision": current_sources["sourceManifestId"],
        "candidateRevision": pointer["revisionId"],
        "flowRevision": expected_flow_revision,
        "applyClass": manifest["applyClass"],
        "splitJoinValidated": True,
        "pointer": pointer,
    }


def emit_bootstrap_patch(root: Path) -> str:
    docs = load_pipeline_documents(
        root,
        include_companions=False,
        include_split_authoring=False,
        include_migration_fixture=True,
    )
    world_sets = build_world_event_sets(docs[WORLD_PRODUCT_REL])
    combat = build_combat_authoring(docs[COMBAT_PRODUCT_REL])
    docs[WORLD_SET_REL] = world_sets
    docs[COMBAT_AUTHORING_REL] = combat
    legacy = build_legacy_manifest(root, docs)
    files = {
        COMBAT_AUTHORING_REL: json_text(combat),
        WORLD_SET_REL: json_text(world_sets),
        LEGACY_REL: json_text(legacy),
    }
    lines = ["*** Begin Patch"]
    for relative, text in files.items():
        path = repo_path(root, relative)
        if path.exists():
            if read_json(path) != json.loads(text, object_pairs_hook=_reject_duplicate_pairs):
                raise PipelineError(f"bootstrap target exists with different content: {relative}")
            continue
        lines.append(f"*** Add File: {relative}")
        lines.extend("+" + line for line in text.splitlines())
    lines.append("*** End Patch")
    return "\n".join(lines) + "\n"


def validate_repository(root: Path) -> dict[str, Any]:
    docs, v2, product_outputs = build_repository_product_projection(root)
    outputs = dict(product_outputs)
    outputs.update(
        project_balance_products(root, docs[BOSS_PROFILES_REL], docs[DAMAGE_REL])
    )
    for relative, projected_text in outputs.items():
        projected = json.loads(
            projected_text, object_pairs_hook=_reject_duplicate_pairs
        )
        current = read_json(repo_path(root, relative))
        if current != projected:
            raise PipelineError(
                "split authoring Product drift; run PublishV2: " + relative
            )
    return {
        "schema": "lostark.valtan-tuning-pipeline-validation",
        "formatVersion": 1,
        "gameplaySourceVersion": docs[GAMEPLAY_AUTHORING_REL]["formatVersion"],
        "presentationSourceVersion": docs[PRESENTATION_AUTHORING_REL]["formatVersion"],
        "joinedSourceVersion": v2["formatVersion"],
        "managedPatterns": len(v2["patterns"]),
        "legacyPatterns": len(docs[LEGACY_REL]["patternEntries"]),
        "worldMembers": len(docs[WORLD_SET_REL]["sets"][0]["members"]),
        "combatObjects": len(docs[COMBAT_AUTHORING_REL]["objects"]),
        "projectedArtifacts": len(outputs),
        "sourceManifestId": source_manifest(root)["sourceManifestId"],
    }


def source_manifest_with_authoring(
    root: Path,
    authoring_root: Path,
) -> tuple[dict[str, Any], dict[str, Any], str]:
    """Recover and validate the effective authoring base exposed to the tool."""

    resolved_authoring_root = staging_root(root, authoring_root, "AuthoringRoot")
    _recover_durable_transaction(root, resolved_authoring_root, "authoring")
    repository_manifest = source_manifest(root)
    authoring_revision: str | None = None
    pointer_bytes = _pointer_bytes(
        resolved_authoring_root / "current-authoring.json"
    )
    if pointer_bytes is not None:
        pointer = _validate_pointer_bytes(pointer_bytes, "authoring")
        authoring_revision = pointer["revisionId"]
        load_authoring_revision(
            root,
            resolved_authoring_root,
            authoring_revision,
            repository_manifest,
            load_pipeline_documents(root),
        )
    payload = copy.deepcopy(repository_manifest)
    payload["authoringRevision"] = authoring_revision
    return (
        repository_manifest,
        payload,
        authoring_revision or repository_manifest["sourceManifestId"],
    )


def validate_draft_patch(
    root: Path,
    draft_patch: dict[str, Any],
    authoring_root: Path | None = None,
) -> dict[str, Any]:
    if authoring_root is not None:
        authoring_root = staging_root(root, authoring_root, "AuthoringRoot")
        _recover_durable_transaction(root, authoring_root, "authoring")
    docs = load_pipeline_documents(root)
    current_sources = source_manifest(root)
    base_revision = draft_patch.get("sourceRevision") if isinstance(draft_patch, dict) else ""
    if authoring_root is not None:
        current_head = _validated_authoring_head(
            root, authoring_root, current_sources, docs
        )
        if base_revision != current_head:
            raise DraftPatchError(
                "draft sourceRevision does not match the current saved authoring head",
                document="draftPatch",
                field="sourceRevision",
                error_code="SOURCE_REVISION_MISMATCH",
            )
    migrated, base_bosses, base_damage = resolve_authoring_base(
        root, authoring_root, base_revision, current_sources, docs
    )
    candidate, candidate_bosses, candidate_damage, operation_count = apply_draft_patch(
        migrated,
        base_bosses,
        base_damage,
        draft_patch,
        base_revision,
        docs[WORLD_SET_REL],
        docs[COMBAT_AUTHORING_REL],
    )
    projected = project_v2_products(root, docs, candidate)
    projected.update(project_balance_products(root, candidate_bosses, candidate_damage))
    projected[PROVENANCE_REL] = project_provenance_receipt(root, projected)
    return {
        "operationCount": operation_count,
        "baseRevision": base_revision,
        "authoringRevision": (
            base_revision
            if base_revision != current_sources["sourceManifestId"]
            else None
        ),
        "repositorySourceRevision": current_sources["sourceManifestId"],
        "gameplaySourceVersion": docs[GAMEPLAY_AUTHORING_REL]["formatVersion"],
        "presentationSourceVersion": docs[PRESENTATION_AUTHORING_REL]["formatVersion"],
        "joinedSourceVersion": candidate["formatVersion"],
        "projectedArtifacts": len(projected),
        "runtimeProjectionSupported": True,
    }


def command_result(
    command: str,
    *,
    ok: bool,
    source_revision: str | None,
    candidate_revision: str | None,
    errors: Sequence[dict[str, Any]] = (),
    payload: Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    return {
        "schema": "lostark.valtan-tuning-command-result",
        "formatVersion": 1,
        "command": command,
        "ok": ok,
        "sourceRevision": source_revision,
        "candidateRevision": candidate_revision,
        "errors": list(errors),
        "payload": dict(payload or {}),
    }


def _default_root() -> Path:
    return Path(__file__).resolve().parents[2]


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repository-root", type=Path, default=_default_root())
    subparsers = parser.add_subparsers(dest="command", required=True)
    subparsers.add_parser("emit-bootstrap-patch")
    migrate_parser = subparsers.add_parser("migrate-preview")
    migrate_parser.add_argument("--output", type=Path, required=True)
    migrate_parser.add_argument("--draft-patch", type=Path)
    subparsers.add_parser("validate")
    product_parser = subparsers.add_parser("project-products")
    product_parser.add_argument("--output-root", type=Path, required=True)
    draft_parser = subparsers.add_parser("validate-draft")
    draft_parser.add_argument("--draft-patch", type=Path)
    draft_parser.add_argument("--authoring-root", type=Path)
    save_parser = subparsers.add_parser("save-authoring")
    save_parser.add_argument("--authoring-root", type=Path, required=True)
    save_parser.add_argument("--draft-patch", type=Path)
    save_parser.add_argument(
        "--fail-at",
        choices=("after_stage", "after_validate", "after_revision_manifest", "before_promote", "after_promote", "before_pointer", "after_pointer"),
    )
    save_parser.add_argument(
        "--crash-at",
        choices=(
            "after_staged_journal_temp",
            "after_journal_staged",
            "after_promoted_journal_temp",
            "after_promote",
            "after_pointer",
            "after_journal_unlink",
        ),
    )
    source_parser = subparsers.add_parser("source-manifest")
    source_parser.add_argument("--output", type=Path)
    source_parser.add_argument("--repository-only", action="store_true")
    source_parser.add_argument(
        "--authoring-root",
        type=Path,
        default=Path("Intermediate/ValtanTuningAuthoring"),
    )
    publish_parser = subparsers.add_parser("publish-candidate")
    publish_parser.add_argument("--candidate-root", type=Path, required=True)
    publish_parser.add_argument("--expected-source-manifest", type=Path)
    publish_parser.add_argument("--draft-patch", type=Path)
    publish_parser.add_argument("--authoring-root", type=Path)
    publish_parser.add_argument("--authoring-revision")
    publish_parser.add_argument(
        "--fail-at",
        choices=("after_stage", "after_validate", "after_revision_manifest", "before_promote", "after_promote", "before_pointer", "after_pointer"),
    )
    publish_parser.add_argument(
        "--crash-at",
        choices=(
            "after_staged_journal_temp",
            "after_journal_staged",
            "after_promoted_journal_temp",
            "after_promote",
            "after_pointer",
            "after_journal_unlink",
        ),
    )
    flow_publish_parser = subparsers.add_parser("publish-saved-flow")
    flow_publish_parser.add_argument("--candidate-root", type=Path, required=True)
    flow_publish_parser.add_argument("--expected-flow-revision", required=True)
    flow_publish_parser.add_argument(
        "--fail-at",
        choices=("after_stage", "after_validate", "after_revision_manifest", "before_promote", "after_promote", "before_pointer", "after_pointer"),
    )
    args = parser.parse_args(argv)
    root = args.repository_root.resolve()
    command_name = args.command.replace("-", "_").upper()
    source_revision: str | None = None
    try:
        if args.command == "emit-bootstrap-patch":
            sys.stdout.write(emit_bootstrap_patch(root))
            return 0

        source_manifest_payload: dict[str, Any] | None = None
        if args.command == "source-manifest":
            if args.repository_only:
                current_sources = source_manifest(root)
                source_manifest_payload = {**current_sources, "authoringRevision": None}
                source_revision = current_sources["sourceManifestId"]
            else:
                source_authoring_root = args.authoring_root
                if not source_authoring_root.is_absolute():
                    source_authoring_root = root / source_authoring_root
                current_sources, source_manifest_payload, source_revision = (
                    source_manifest_with_authoring(root, source_authoring_root)
                )
        else:
            current_sources = source_manifest(root)
            source_revision = current_sources["sourceManifestId"]
        payload: dict[str, Any]
        candidate_revision: str | None = None
        if args.command == "migrate-preview":
            docs = load_pipeline_documents(
                root,
                include_split_authoring=False,
                include_migration_fixture=True,
            )
            migrated = migrate_v1_to_v2(root, docs)
            operation_count = 0
            if args.draft_patch:
                draft_patch = read_json(args.draft_patch.resolve())
                migrated, _, _, operation_count = apply_draft_patch(
                    migrated,
                    docs[BOSS_PROFILES_REL],
                    docs[DAMAGE_REL],
                    draft_patch,
                    source_revision,
                    docs[WORLD_SET_REL],
                    docs[COMBAT_AUTHORING_REL],
                )
            output_path = args.output.resolve()
            if output_path == repo_path(root, MASTER_REL):
                raise DraftPatchError(
                    "migrate-preview refuses to overwrite the format-v1 runtime-compatible master",
                    document=MASTER_REL,
                    path=str(output_path),
                    error_code="SOURCE_OVERWRITE_FORBIDDEN",
                )
            staging_root(root, output_path.parent, "V2OutputPath")
            output_path.parent.mkdir(parents=True, exist_ok=True)
            _write_fsync(output_path, json_text(migrated).encode("utf-8"))
            payload = {
                "outputPath": str(output_path),
                "migrationFixtureSourceVersion": docs[MASTER_REL]["formatVersion"],
                "joinedPreviewVersion": migrated["formatVersion"],
                "operationCount": operation_count,
                "runtimeProductChanged": False,
            }
        elif args.command == "validate":
            payload = validate_repository(root)
        elif args.command == "project-products":
            output_root = args.output_root
            if not output_root.is_absolute():
                output_root = root / output_root
            payload = stage_repository_product_projection(root, output_root)
        elif args.command == "validate-draft":
            if args.draft_patch is None:
                raise DraftPatchError(
                    "validate-draft requires DraftPatchPath",
                    document="draftPatch",
                    field="path",
                    error_code="DRAFT_PATCH_REQUIRED",
                )
            draft_patch = read_json(args.draft_patch.resolve())
            payload = validate_draft_patch(
                root,
                draft_patch,
                args.authoring_root.resolve() if args.authoring_root else None,
            )
            source_revision = draft_patch["sourceRevision"]
        elif args.command == "save-authoring":
            if args.draft_patch is None:
                raise DraftPatchError(
                    "save-authoring requires DraftPatchPath",
                    document="draftPatch",
                    field="path",
                    error_code="DRAFT_PATCH_REQUIRED",
                )
            draft_patch = read_json(args.draft_patch.resolve())
            pointer = save_authoring(
                root,
                args.authoring_root,
                draft_patch,
                fail_at=args.fail_at,
                crash_at=args.crash_at,
            )
            source_revision = draft_patch["sourceRevision"]
            payload = {
                "authoringRevision": pointer["revisionId"],
                "pointer": pointer,
            }
        elif args.command == "source-manifest":
            if args.output:
                args.output.parent.mkdir(parents=True, exist_ok=True)
                _write_fsync(
                    args.output, json_text(current_sources).encode("utf-8")
                )
            if source_manifest_payload is None:
                raise PipelineError("source manifest payload was not initialized")
            payload = source_manifest_payload
        elif args.command == "publish-saved-flow":
            published = publish_saved_flow(
                root, args.candidate_root, args.expected_flow_revision, fail_at=args.fail_at
            )
            source_revision = published.pop("sourceRevision")
            candidate_revision = published.pop("candidateRevision")
            payload = published
        elif args.command == "publish-candidate":
            expected = read_json(args.expected_source_manifest) if args.expected_source_manifest else None
            draft_patch = read_json(args.draft_patch.resolve()) if args.draft_patch else None
            pointer = publish_candidate(
                root,
                args.candidate_root,
                expected_source_manifest=expected,
                draft_patch=draft_patch,
                authoring_root=args.authoring_root,
                authoring_revision=args.authoring_revision,
                fail_at=args.fail_at,
                crash_at=args.crash_at,
            )
            if draft_patch is not None:
                source_revision = draft_patch["sourceRevision"]
            elif args.authoring_revision is not None:
                source_revision = args.authoring_revision
            candidate_revision = pointer["revisionId"]
            published_manifest = _validate_transaction_artifact(
                staging_root(root, args.candidate_root, "CandidateRoot")
                / "revisions"
                / candidate_revision,
                candidate_revision,
                "candidate",
            )
            payload = {
                "authoringRevision": (
                    source_revision
                    if source_revision != current_sources["sourceManifestId"]
                    else None
                ),
                "applyClass": published_manifest["applyClass"],
                "pointer": pointer,
            }
        else:
            raise PipelineError(f"unhandled command: {args.command}")
        print(
            json.dumps(
                command_result(
                    command_name,
                    ok=True,
                    source_revision=source_revision,
                    candidate_revision=candidate_revision,
                    payload=payload,
                ),
                ensure_ascii=False,
                sort_keys=True,
            )
        )
        return 0
    except Exception as exc:
        error = exc if isinstance(exc, PipelineError) else PipelineError(str(exc))
        if source_revision is None:
            try:
                source_revision = source_manifest(root)["sourceManifestId"]
            except Exception:
                pass
        print(
            json.dumps(
                command_result(
                    command_name,
                    ok=False,
                    source_revision=source_revision,
                    candidate_revision=None,
                    errors=(error.as_error(),),
                ),
                ensure_ascii=False,
                sort_keys=True,
            ),
            file=sys.stderr,
        )
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
