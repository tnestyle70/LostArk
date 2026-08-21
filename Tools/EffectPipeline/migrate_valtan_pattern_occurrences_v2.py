#!/usr/bin/env python3
"""Migrate Valtan animation bindings and Effect cues to occurrence format v2.

The migration is deliberately mechanical.  It preserves the merged clip order,
records row-level Git provenance, and attaches every legacy stage cue to the
first persisted clip occurrence.  A legacy cue end that extends beyond that
clip's natural source duration becomes ``natural``: Valtan already stops every
owned occurrence at the Server stage edge, while an out-of-segment v2 cue end
would be an invalid source-time claim.

This tool never derives a product animation sequence from the source extraction.
Reviewed source-driven changes use a separate SOURCE_REVIEWED_DELTA edit.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
VALTAN_ROOT = REPOSITORY_ROOT / "Data/Animation/Authored/Valtan"
BINDINGS_PATH = VALTAN_ROOT / "Valtan.patternbindings.json"
CUES_PATH = VALTAN_ROOT / "Valtan.patterneffectcues.json"
ANIM_NOTIFY_PATH = (
    REPOSITORY_ROOT / "Data/Animation/Reference/Valtan/Valtan.animnotify"
)
RECEIPT_PATH = VALTAN_ROOT / "Valtan.pattern-occurrence-v2-migration.receipt.json"

ANIMATION_PR_127_ACTIONS = frozenset(
    {
        "valtan.attack.down-smash.active",
        "valtan.attack.down-smash.recovery",
        "valtan.attack.down-smash.windup",
        "valtan.attack.earthquake-smash.delayed",
        "valtan.attack.earthquake-smash.impact",
        "valtan.attack.earthquake-smash.recovery",
        "valtan.attack.earthquake-smash.windup",
        "valtan.attack.fist-in-out.outer",
        "valtan.attack.fist-in-out.recovery",
        "valtan.attack.four-slash.active",
        "valtan.attack.four-slash.recovery",
        "valtan.attack.four-slash.windup",
        "valtan.attack.front-back-front.active",
        "valtan.attack.front-back-front.recovery",
        "valtan.attack.front-back-front.windup",
        "valtan.attack.ground-wave-smash.active",
        "valtan.attack.ground-wave-smash.windup",
        "valtan.attack.high-jump.recovery",
        "valtan.attack.swing.active",
        "valtan.attack.swing.recovery",
        "valtan.attack.swing.windup",
    }
)

PATTERN_PR_REFERENCE_ACTIONS = frozenset(
    {
        "valtan.attack.dash-charge.groggy",
        "valtan.attack.dash-charge.part-break",
        "valtan.mechanic.armor-break-opening.part-break",
    }
)

EXPECTED_BINDING_COUNT = 124
EXPECTED_CLIP_OCCURRENCE_COUNT = 128
EXPECTED_CUE_COUNT = 99
STABLE_TOKEN = re.compile(r"^[A-Za-z0-9_.-]+$")
ANIM_NOTIFY_ROW = re.compile(
    r'^"(?P<clip>[^"]+)".*\slen=(?P<seconds>[0-9]+(?:\.[0-9]+)?)'
)


class MigrationError(RuntimeError):
    pass


def read_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8-sig") as handle:
        value = json.load(handle)
    if not isinstance(value, dict):
        raise MigrationError(f"Expected an object document: {path}")
    return value


def pretty_bytes(value: dict[str, Any]) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2) + "\n"
    ).encode("utf-8")


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def require_token(value: Any, label: str) -> str:
    if not isinstance(value, str) or not STABLE_TOKEN.fullmatch(value):
        raise MigrationError(f"Invalid stable token for {label}: {value!r}")
    return value


def mapping_basis(action_id: str) -> str:
    if action_id in ANIMATION_PR_127_ACTIONS:
        return "ANIMATION_PR_127"
    if action_id in PATTERN_PR_REFERENCE_ACTIONS:
        return "PATTERN_PR_REFERENCE"
    return "CURRENT_PRODUCT_BASELINE"


def clip_occurrence_id(action_id: str, ordinal: int) -> str:
    return f"{action_id}.clip.{ordinal:02d}"


def normalize_legacy_clips(value: Any, action_id: str) -> list[str]:
    if isinstance(value, str):
        clips = [value]
    elif isinstance(value, list):
        clips = value
    else:
        raise MigrationError(f"Invalid legacy clip chain: {action_id}")
    if not clips or len(clips) > 16:
        raise MigrationError(f"Invalid legacy clip count: {action_id}")
    return [require_token(clip, f"{action_id} clip") for clip in clips]


def migrate_bindings(document: dict[str, Any]) -> dict[str, Any]:
    if document.get("schema") != "lostark.valtan-pattern-bindings":
        raise MigrationError("Unexpected Valtan pattern binding schema.")
    if document.get("formatVersion") != 1:
        raise MigrationError("Binding migration requires formatVersion 1 input.")
    rows = document.get("bindings")
    if not isinstance(rows, list) or len(rows) != EXPECTED_BINDING_COUNT:
        raise MigrationError("Unexpected Valtan pattern binding denominator.")

    migrated: list[dict[str, Any]] = []
    claimed_actions: set[str] = set()
    claimed_occurrences: set[str] = set()
    for row in rows:
        if not isinstance(row, dict) or set(row) != {"actionId", "clip"}:
            raise MigrationError("Legacy pattern binding has unexpected fields.")
        action_id = require_token(row.get("actionId"), "actionId")
        if action_id in claimed_actions:
            raise MigrationError(f"Duplicate actionId: {action_id}")
        claimed_actions.add(action_id)
        clips = normalize_legacy_clips(row.get("clip"), action_id)
        migrated_clips: list[dict[str, Any]] = []
        for index, clip in enumerate(clips, start=1):
            occurrence_id = clip_occurrence_id(action_id, index)
            if occurrence_id in claimed_occurrences:
                raise MigrationError(f"Duplicate clip occurrence: {occurrence_id}")
            claimed_occurrences.add(occurrence_id)
            migrated_clips.append(
                {
                    "clipOccurrenceId": occurrence_id,
                    "clip": clip,
                    "mappingBasis": mapping_basis(action_id),
                    "sourceStartMs": 0,
                    "playMs": 0,
                    "playRate": 1.0,
                    # This explicitly preserves the v1 runtime's last-clip loop.
                    "loop": index == len(clips),
                }
            )
        migrated.append({"actionId": action_id, "clips": migrated_clips})

    if len(claimed_occurrences) != EXPECTED_CLIP_OCCURRENCE_COUNT:
        raise MigrationError("Unexpected Valtan clip occurrence denominator.")
    return {
        "schema": document["schema"],
        "formatVersion": 2,
        "bossArchetypeId": document.get("bossArchetypeId"),
        "bindings": migrated,
    }


def load_clip_durations_ms() -> dict[str, int]:
    durations: dict[str, int] = {}
    for line in ANIM_NOTIFY_PATH.read_text(encoding="utf-8-sig").splitlines():
        match = ANIM_NOTIFY_ROW.match(line)
        if match is None:
            continue
        # Cue times are integer milliseconds.  Flooring prevents a 1.4667 s
        # source clip from admitting a 1467 ms cue end that the runtime model
        # timeline must reject as out of segment.
        milliseconds = int(float(match.group("seconds")) * 1000.0 + 1.0e-9)
        clip = match.group("clip")
        durations[clip] = max(milliseconds, durations.get(clip, 0))
    if not durations:
        raise MigrationError("Valtan animnotify yielded no clip durations.")
    return durations


def migrate_cues(
    document: dict[str, Any],
    bindings: dict[str, Any],
) -> tuple[dict[str, Any], list[dict[str, Any]]]:
    if document.get("schema") != "lostark.valtan-pattern-effect-cues":
        raise MigrationError("Unexpected Valtan cue schema.")
    if document.get("formatVersion") != 1:
        raise MigrationError("Cue migration requires formatVersion 1 input.")
    rows = document.get("cues")
    if not isinstance(rows, list) or len(rows) != EXPECTED_CUE_COUNT:
        raise MigrationError("Unexpected Valtan cue denominator.")

    binding_by_action = {
        row["actionId"]: row for row in bindings["bindings"]
    }
    clip_durations = load_clip_durations_ms()
    migrated: list[dict[str, Any]] = []
    promoted_natural: list[dict[str, Any]] = []
    occurrence_ids: set[str] = set()

    legacy_fields = {
        "bindingId",
        "patternId",
        "stageId",
        "actionId",
        "effectAssetId",
        "anchorSlotId",
        "followPolicy",
        "stopPolicy",
        "startMs",
        "endMs",
        "localTransform",
    }
    for row in rows:
        if not isinstance(row, dict) or set(row) != legacy_fields:
            raise MigrationError("Legacy Valtan cue has unexpected fields.")
        binding_id = require_token(row.get("bindingId"), "bindingId")
        action_id = require_token(row.get("actionId"), "cue actionId")
        action_binding = binding_by_action.get(action_id)
        if action_binding is None:
            raise MigrationError(f"Cue action has no animation binding: {action_id}")
        first_clip = action_binding["clips"][0]
        source_start_ms = row.get("startMs")
        if not isinstance(source_start_ms, int) or source_start_ms < 0:
            raise MigrationError(f"Invalid cue start: {binding_id}")
        duration_ms = clip_durations.get(first_clip["clip"])
        if duration_ms is None or source_start_ms >= duration_ms:
            raise MigrationError(
                f"Cue start is outside the first persisted clip: {binding_id}"
            )

        stop_policy = row.get("stopPolicy")
        source_end_ms = row.get("endMs")
        if stop_policy == "natural":
            if source_end_ms is not None:
                raise MigrationError(f"Natural legacy cue has an end: {binding_id}")
        elif stop_policy == "cue_end":
            if (
                not isinstance(source_end_ms, int)
                or source_end_ms <= source_start_ms
            ):
                raise MigrationError(f"Invalid legacy cue end: {binding_id}")
            if source_end_ms > duration_ms:
                promoted_natural.append(
                    {
                        "bindingId": binding_id,
                        "legacyEndMs": source_end_ms,
                        "clipDurationMs": duration_ms,
                        "reason": "LEGACY_STAGE_WALL_END_OUTSIDE_SOURCE_CLIP",
                    }
                )
                stop_policy = "natural"
                source_end_ms = None
        else:
            raise MigrationError(f"Invalid legacy stop policy: {binding_id}")

        occurrence_id = f"{binding_id}.occurrence.01"
        if occurrence_id in occurrence_ids:
            raise MigrationError(f"Duplicate cue occurrence: {occurrence_id}")
        occurrence_ids.add(occurrence_id)
        migrated.append(
            {
                "bindingId": binding_id,
                "occurrenceId": occurrence_id,
                "patternId": row["patternId"],
                "stageId": row["stageId"],
                "actionId": action_id,
                "clipOccurrenceId": first_clip["clipOccurrenceId"],
                "effectAssetId": row["effectAssetId"],
                "anchorSlotId": row["anchorSlotId"],
                "followPolicy": row["followPolicy"],
                "stopPolicy": stop_policy,
                "repeatPolicy": "once",
                "sourceStartMs": source_start_ms,
                "sourceEndMs": source_end_ms,
                "localTransform": row["localTransform"],
            }
        )

    return (
        {
            "schema": document["schema"],
            "formatVersion": 2,
            "ownerArchetypeId": document.get("ownerArchetypeId"),
            "cues": migrated,
        },
        promoted_natural,
    )


def validate_v2(
    bindings: dict[str, Any],
    cues: dict[str, Any],
    *,
    require_migration_denominator: bool,
) -> tuple[int, int, int]:
    if bindings.get("formatVersion") != 2 or cues.get("formatVersion") != 2:
        raise MigrationError("Expected v2 Valtan documents.")
    binding_rows = bindings.get("bindings")
    cue_rows = cues.get("cues")
    if not isinstance(binding_rows, list) or not isinstance(cue_rows, list):
        raise MigrationError("Expected Valtan v2 row arrays.")
    occurrence_count = sum(len(row.get("clips", [])) for row in binding_rows)
    counts = (len(binding_rows), occurrence_count, len(cue_rows))
    expected = (
        EXPECTED_BINDING_COUNT,
        EXPECTED_CLIP_OCCURRENCE_COUNT,
        EXPECTED_CUE_COUNT,
    )
    if (require_migration_denominator and counts != expected) or (
        not require_migration_denominator
        and any(actual < baseline for actual, baseline in zip(counts, expected))
    ):
        raise MigrationError(f"Unexpected v2 denominators: {counts}")
    return counts


def write_atomic(path: Path, payload: bytes) -> None:
    staging = path.with_suffix(path.suffix + ".staging")
    staging.write_bytes(payload)
    staging.replace(path)


def build_receipt(
    legacy_binding_sha: str,
    legacy_cue_sha: str,
    bindings: dict[str, Any],
    cues: dict[str, Any],
    binding_payload: bytes,
    cue_payload: bytes,
    promoted_natural: list[dict[str, Any]],
) -> dict[str, Any]:
    binding_identity = [
        {
            "actionId": row["actionId"],
            "clips": [
                {
                    "clipOccurrenceId": clip["clipOccurrenceId"],
                    "clip": clip["clip"],
                }
                for clip in row["clips"]
            ],
        }
        for row in bindings["bindings"]
    ]
    cue_identity_fields = (
        "bindingId",
        "occurrenceId",
        "patternId",
        "stageId",
        "actionId",
        "clipOccurrenceId",
        "effectAssetId",
    )
    cue_identity = [
        {field: row[field] for field in cue_identity_fields}
        for row in cues["cues"]
    ]
    return {
        "schema": "lostark.valtan-pattern-occurrence-v2-migration-receipt",
        "formatVersion": 2,
        "basis": {
            "animationSequenceMerge": "459da808",
            "patternReferenceMerge": "0b70f06e",
            "implementationBaseline": "dd382bf1c817eca3314067fbfac3628b47b617b2",
        },
        "inputs": {
            "legacyPatternBindingsSha256": legacy_binding_sha,
            "legacyPatternEffectCuesSha256": legacy_cue_sha,
        },
        "outputs": {
            "migrationBaselinePatternBindingsSha256": sha256_bytes(
                binding_payload
            ),
            "migrationBaselinePatternEffectCuesSha256": sha256_bytes(
                cue_payload
            ),
            "bindingCount": EXPECTED_BINDING_COUNT,
            "clipOccurrenceCount": EXPECTED_CLIP_OCCURRENCE_COUNT,
            "cueOccurrenceCount": EXPECTED_CUE_COUNT,
        },
        "baselineIdentity": {
            "bindings": binding_identity,
            "cues": cue_identity,
        },
        "legacyCueEndPromotions": promoted_natural,
        "policies": {
            "sourceSequenceInsertion": "FORBIDDEN_WITHOUT_SOURCE_REVIEWED_DELTA",
            "lastClipLoop": "EXPLICIT_V1_RUNTIME_PRESERVATION",
            "legacyCueRepeat": "once",
            "outOfSegmentCueEnd": "natural_until_server_stage_edge",
        },
    }


def check_receipt(
    receipt: dict[str, Any],
    bindings: dict[str, Any],
    cues: dict[str, Any],
) -> None:
    if receipt.get("schema") != (
        "lostark.valtan-pattern-occurrence-v2-migration-receipt"
    ) or receipt.get("formatVersion") != 2:
        raise MigrationError("Unexpected Valtan occurrence migration receipt.")
    outputs = receipt.get("outputs")
    if not isinstance(outputs, dict):
        raise MigrationError("Occurrence migration receipt has no outputs.")
    baseline_identity = receipt.get("baselineIdentity")
    if not isinstance(baseline_identity, dict):
        raise MigrationError("Occurrence migration receipt has no identity set.")
    baseline_bindings = baseline_identity.get("bindings")
    baseline_cues = baseline_identity.get("cues")
    if (
        not isinstance(baseline_bindings, list)
        or not isinstance(baseline_cues, list)
        or len(baseline_bindings) != outputs.get("bindingCount")
        or len(baseline_cues) != outputs.get("cueOccurrenceCount")
    ):
        raise MigrationError("Occurrence migration identity denominator drifted.")

    current_bindings = {
        row.get("actionId"): row
        for row in bindings.get("bindings", [])
        if isinstance(row, dict)
    }
    baseline_clip_count = 0
    for baseline in baseline_bindings:
        if not isinstance(baseline, dict):
            raise MigrationError("Invalid baseline binding identity row.")
        action_id = baseline.get("actionId")
        current = current_bindings.get(action_id)
        baseline_clips = baseline.get("clips")
        if not isinstance(current, dict) or not isinstance(baseline_clips, list):
            raise MigrationError(
                f"Migrated binding identity was removed: {action_id}"
            )
        current_clips = current.get("clips")
        if not isinstance(current_clips, list):
            raise MigrationError(f"Migrated binding clips are invalid: {action_id}")
        current_by_occurrence = {
            clip.get("clipOccurrenceId"): clip
            for clip in current_clips
            if isinstance(clip, dict)
        }
        current_order = [
            clip.get("clipOccurrenceId")
            for clip in current_clips
            if isinstance(clip, dict)
        ]
        baseline_order: list[str] = []
        for baseline_clip in baseline_clips:
            if not isinstance(baseline_clip, dict):
                raise MigrationError("Invalid baseline clip identity row.")
            occurrence_id = baseline_clip.get("clipOccurrenceId")
            current_clip = current_by_occurrence.get(occurrence_id)
            if (
                not isinstance(current_clip, dict)
                or current_clip.get("clip") != baseline_clip.get("clip")
            ):
                raise MigrationError(
                    "Migrated clip identity was removed or rebound: "
                    f"{occurrence_id}"
                )
            baseline_order.append(occurrence_id)
            baseline_clip_count += 1
        projected_order = [
            occurrence_id
            for occurrence_id in current_order
            if occurrence_id in set(baseline_order)
        ]
        if projected_order != baseline_order:
            raise MigrationError(
                f"Migrated clip order changed: {action_id}"
            )
    if baseline_clip_count != outputs.get("clipOccurrenceCount"):
        raise MigrationError("Occurrence migration clip denominator drifted.")

    cue_identity_fields = (
        "bindingId",
        "occurrenceId",
        "patternId",
        "stageId",
        "actionId",
        "clipOccurrenceId",
        "effectAssetId",
    )
    current_cues = {
        row.get("occurrenceId"): row
        for row in cues.get("cues", [])
        if isinstance(row, dict)
    }
    for baseline in baseline_cues:
        if not isinstance(baseline, dict):
            raise MigrationError("Invalid baseline cue identity row.")
        occurrence_id = baseline.get("occurrenceId")
        current = current_cues.get(occurrence_id)
        if not isinstance(current, dict) or any(
            current.get(field) != baseline.get(field)
            for field in cue_identity_fields
        ):
            raise MigrationError(
                "Migrated cue identity was removed or rebound: "
                f"{occurrence_id}"
            )


def upgrade_v1_receipt(
    receipt: dict[str, Any],
    bindings: dict[str, Any],
    cues: dict[str, Any],
) -> dict[str, Any]:
    if receipt.get("schema") != (
        "lostark.valtan-pattern-occurrence-v2-migration-receipt"
    ) or receipt.get("formatVersion") != 1:
        raise MigrationError("Receipt refresh requires the v1 migration receipt.")
    validate_v2(bindings, cues, require_migration_denominator=True)
    refreshed = build_receipt(
        receipt.get("inputs", {}).get("legacyPatternBindingsSha256", ""),
        receipt.get("inputs", {}).get("legacyPatternEffectCuesSha256", ""),
        bindings,
        cues,
        pretty_bytes(bindings),
        pretty_bytes(cues),
        receipt.get("legacyCueEndPromotions", []),
    )
    refreshed["basis"] = receipt.get("basis", refreshed["basis"])
    refreshed["policies"] = receipt.get("policies", refreshed["policies"])
    return refreshed


def main() -> int:
    parser = argparse.ArgumentParser()
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--refresh-receipt", action="store_true")
    args = parser.parse_args()

    bindings = read_json(BINDINGS_PATH)
    cues = read_json(CUES_PATH)
    if args.write:
        legacy_binding_payload = BINDINGS_PATH.read_bytes()
        legacy_cue_payload = CUES_PATH.read_bytes()
        migrated_bindings = migrate_bindings(bindings)
        migrated_cues, promoted = migrate_cues(cues, migrated_bindings)
        validate_v2(
            migrated_bindings,
            migrated_cues,
            require_migration_denominator=True,
        )
        binding_payload = pretty_bytes(migrated_bindings)
        cue_payload = pretty_bytes(migrated_cues)
        receipt = build_receipt(
            sha256_bytes(legacy_binding_payload),
            sha256_bytes(legacy_cue_payload),
            migrated_bindings,
            migrated_cues,
            binding_payload,
            cue_payload,
            promoted,
        )
        write_atomic(BINDINGS_PATH, binding_payload)
        write_atomic(CUES_PATH, cue_payload)
        write_atomic(RECEIPT_PATH, pretty_bytes(receipt))
        print(
            "Migrated Valtan occurrence v2: "
            f"{EXPECTED_BINDING_COUNT} bindings / "
            f"{EXPECTED_CLIP_OCCURRENCE_COUNT} clips / "
            f"{EXPECTED_CUE_COUNT} cues / "
            f"{len(promoted)} legacy cue ends promoted to natural."
        )
        return 0

    if not RECEIPT_PATH.is_file():
        raise MigrationError(f"Missing migration receipt: {RECEIPT_PATH}")
    if args.refresh_receipt:
        refreshed = upgrade_v1_receipt(
            read_json(RECEIPT_PATH), bindings, cues
        )
        write_atomic(RECEIPT_PATH, pretty_bytes(refreshed))
        print(
            "Refreshed Valtan occurrence migration receipt to identity-subset "
            "format v2."
        )
        return 0

    counts = validate_v2(
        bindings,
        cues,
        require_migration_denominator=False,
    )
    check_receipt(
        read_json(RECEIPT_PATH),
        bindings,
        cues,
    )
    print(
        "Valtan occurrence v2 check passed: "
        f"{counts[0]} bindings / {counts[1]} clips / {counts[2]} cues."
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except MigrationError as error:
        print(f"ERROR: {error}")
        raise SystemExit(1)
