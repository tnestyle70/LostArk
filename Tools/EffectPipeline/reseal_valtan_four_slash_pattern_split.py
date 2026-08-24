#!/usr/bin/env python3
"""Reseal the exact Carrier V1 owners moved by the Valtan pattern split.

This is a one-way, idempotent successor migration.  It accepts only the sealed
Carrier/migration preimage or the exact sealed successor.  The historical
``baselineIdentity`` remains unchanged; three explicit old-to-new owner rows
carry the only admitted identity movement.
"""

from __future__ import annotations

import argparse
import copy
import importlib.util
import os
from pathlib import Path
import sys
from typing import Any, Callable


MIGRATION_SCRIPT = Path(__file__).with_name(
    "migrate_valtan_pattern_occurrences_v2.py"
)
MIGRATION_SPEC = importlib.util.spec_from_file_location(
    "valtan_pattern_occurrence_migration_for_owner_reseal",
    MIGRATION_SCRIPT,
)
if MIGRATION_SPEC is None or MIGRATION_SPEC.loader is None:  # pragma: no cover
    raise RuntimeError("Unable to load the Valtan occurrence migration module.")
migration = importlib.util.module_from_spec(MIGRATION_SPEC)
MIGRATION_SPEC.loader.exec_module(migration)


CARRIER_RECEIPT_PATH = migration.CARRIER_V1_RECEIPT_PATH
MIGRATION_RECEIPT_PATH = migration.RECEIPT_PATH
BINDINGS_PATH = migration.BINDINGS_PATH
CUES_PATH = migration.CUES_PATH
ACTION_BINDINGS_PATH = (
    migration.REPOSITORY_ROOT
    / "Data/Animation/Authored/Valtan/Valtan.actionbindings.json"
)

EXPECTED_OLD_CARRIER_RECEIPT_CANONICAL_SHA256 = (
    "a1a0515a6072c52097bef9ada0ab681fe8c8c46c4d64d902817d4e2e4e826e00"
)
EXPECTED_OWNER_RESEALED_CARRIER_RECEIPT_CANONICAL_SHA256 = (
    "ddb22908985fc2ec93a551f81b41ab77ef06d5dd330fc3a19daf51428a8a16d8"
)
EXPECTED_NEW_CARRIER_RECEIPT_CANONICAL_SHA256 = (
    "92a102cb0b25d0d669014fdce0c4a4e08912b68530960b533769d7d10442cc99"
)
EXPECTED_OLD_MIGRATION_RECEIPT_CANONICAL_SHA256 = (
    "30a1a5510ebf9059574cc3cec4145bc10d8bcaeb1c60c49a2a2e74d3fb0159d1"
)
EXPECTED_OWNER_RESEALED_MIGRATION_RECEIPT_CANONICAL_SHA256 = (
    "d239bd03687b383950bcbcfe0073c5176065398a95e8ca0c4f95d87af89d3efa"
)
EXPECTED_NEW_MIGRATION_RECEIPT_CANONICAL_SHA256 = (
    "92d79f185361b060a03e207ccded4645f3726f65021fadbb42cdf1cd70880e6f"
)
EXPECTED_BASELINE_IDENTITY_CANONICAL_SHA256 = (
    "3bf15319d5ec09c4d04042dfb932e37a07242372e76ec00edb943abc7d3a58eb"
)
EXPECTED_CURRENT_COUNTS = (131, 137, 44)


class ResealError(RuntimeError):
    pass


def repository_path(path: Path) -> str:
    return path.resolve().relative_to(
        migration.REPOSITORY_ROOT.resolve()
    ).as_posix()


def _unique_index(
    rows: Any,
    field: str,
    label: str,
) -> dict[str, dict[str, Any]]:
    if not isinstance(rows, list) or any(
        not isinstance(row, dict) for row in rows
    ):
        raise ResealError(f"{label} rows are invalid.")
    indexed = {row.get(field): row for row in rows}
    if None in indexed or len(indexed) != len(rows):
        raise ResealError(f"{label} identity is missing or duplicated.")
    return indexed


def derive_four_slash_source_sequence_path(
    action_bindings: dict[str, Any],
) -> tuple[str, str, list[dict[str, Any]]]:
    if (
        action_bindings.get("schema")
        != "lostark.valtan-action-effect-bindings"
        or action_bindings.get("formatVersion") != 1
        or action_bindings.get("bossArchetypeId") != "BOSS_VALTAN"
    ):
        raise ResealError("Valtan actionbindings header drifted.")
    patterns = _unique_index(
        action_bindings.get("patterns"), "patternId", "actionbinding pattern"
    )
    pattern = patterns.get("VALTAN_FOUR_SLASH")
    if (
        not isinstance(pattern, dict)
        or pattern.get("sourceActionIds") != [
            migration.FOUR_SLASH_SOURCE_ACTION_ID
        ]
    ):
        raise ResealError("Four-slash source action owner drifted.")
    source_actions = _unique_index(
        pattern.get("sourceActions"),
        "sourceActionId",
        "four-slash source action",
    )
    source_action = source_actions.get(migration.FOUR_SLASH_SOURCE_ACTION_ID)
    if (
        not isinstance(source_action, dict)
        or source_action.get("profileId") != "MN_RPBF_00"
    ):
        raise ResealError("Four-slash source action profile drifted.")
    stages = _unique_index(
        source_action.get("stages"),
        "stageIndex",
        "four-slash source stage",
    )
    selected_stages = [stages.get(index) for index in (8, 9, 10)]
    if any(not isinstance(stage, dict) for stage in selected_stages):
        raise ResealError("Four-slash source stages 8-10 are missing.")
    sequence_path = [
        {
            "sourceStageIndex": int(stage["stageIndex"]),
            "sourceClipOrdinal": clip_ordinal,
            "clip": clip,
        }
        for stage in selected_stages
        for clip_ordinal, clip in enumerate(stage.get("animationClips", []))
    ]
    expected_path = [
        {
            "sourceStageIndex": 8,
            "sourceClipOrdinal": 0,
            "clip": "Att_Battle_10_01",
        },
        {
            "sourceStageIndex": 9,
            "sourceClipOrdinal": 0,
            "clip": "Att_Battle_10_02",
        },
        {
            "sourceStageIndex": 10,
            "sourceClipOrdinal": 0,
            "clip": "Idle_Battle_1",
        },
    ]
    sequence_sha256 = migration.canonical_sha256(sequence_path)
    branch_id = (
        "valtan_four_slash.source-"
        f"{source_action['sourceActionId']}.mn_rpbf_00."
        "sequence-003.stages-008-010"
    )
    if (
        sequence_path != expected_path
        or branch_id != migration.FOUR_SLASH_SOURCE_BRANCH_ID
        or sequence_sha256
        != migration.FOUR_SLASH_SOURCE_SEQUENCE_CANONICAL_SHA256
    ):
        raise ResealError(
            "Four-slash current source sequence branch/hash drifted."
        )
    return branch_id, sequence_sha256, sequence_path


def _validate_current_split(
    bindings: dict[str, Any],
    cues: dict[str, Any],
    action_bindings: dict[str, Any],
) -> None:
    derive_four_slash_source_sequence_path(action_bindings)
    counts = migration.validate_v2(
        bindings,
        cues,
        require_migration_denominator=False,
    )
    if counts != EXPECTED_CURRENT_COUNTS:
        raise ResealError(
            f"Current Valtan successor denominator drifted: {counts}"
        )
    if migration.canonical_sha256(cues) != (
        migration.FOUR_SLASH_NEW_CUE_CANONICAL_SHA256
    ):
        raise ResealError("Current split cue document hash drifted.")

    binding_rows = _unique_index(
        bindings.get("bindings"), "actionId", "pattern binding"
    )
    if "valtan.attack.four-slash.active" in binding_rows:
        raise ResealError("The retired combined active action was restored.")
    expected = {
        "valtan.attack.triple-slash.active": {
            "actionId": "valtan.attack.triple-slash.active",
            "clips": [
                {
                    "clipOccurrenceId": (
                        "valtan.attack.four-slash.active.clip.01"
                    ),
                    "clip": "mesh_att_battle_10_01",
                    "mappingBasis": "ANIMATION_PR_127",
                    "sourceStartMs": 0,
                    "playMs": 0,
                    "playRate": 1.0,
                    "loop": False,
                }
            ],
        },
        "valtan.attack.rotation-slash.active": {
            "actionId": "valtan.attack.rotation-slash.active",
            "clips": [
                {
                    "clipOccurrenceId": (
                        "valtan.attack.four-slash.active.clip.02"
                    ),
                    "clip": "mesh_att_battle_10_02",
                    "mappingBasis": "ANIMATION_PR_127",
                    "sourceStartMs": 0,
                    "playMs": 0,
                    "playRate": 1.0,
                    "loop": False,
                }
            ],
        },
    }
    for action_id, expected_row in expected.items():
        if binding_rows.get(action_id) != expected_row:
            raise ResealError(
                f"Split pattern binding is missing or rebound: {action_id}"
            )


def _validate_old_owner(
    row: dict[str, Any],
    transfer: dict[str, Any],
    *,
    group: bool,
) -> None:
    owner = transfer["oldOwner"]
    stage_field = "semanticStageId" if group else "stageId"
    action_field = "gameplayActionId" if group else "actionId"
    if (
        row.get("patternId") != owner["patternId"]
        or row.get(stage_field) != owner["stageId"]
        or row.get(action_field) != owner["actionId"]
        or row.get("clipOccurrenceId") != transfer["clipOccurrenceId"]
    ):
        raise ResealError(
            "Carrier preimage owner is not the sealed old owner: "
            f"{transfer['replacementBindingId']}"
        )


def _apply_new_owner(
    row: dict[str, Any],
    transfer: dict[str, Any],
    *,
    group: bool,
) -> None:
    owner = transfer["newOwner"]
    row["patternId"] = owner["patternId"]
    row["semanticStageId" if group else "stageId"] = owner["stageId"]
    row["gameplayActionId" if group else "actionId"] = owner["actionId"]


def _apply_owner_reseal(carrier_receipt: dict[str, Any]) -> None:
    proof = migration.expected_four_slash_owner_reseal_proof()
    clip_groups = carrier_receipt.get("clipGroups")
    if not isinstance(clip_groups, list):
        raise ResealError("Carrier clip group rows are invalid.")
    groups = _unique_index(
        [row for row in clip_groups if row.get("cueBindingId") is not None],
        "cueBindingId",
        "Carrier clip group",
    )
    mappings = _unique_index(
        carrier_receipt.get("retiredOwnerSuccessorMappings"),
        "retiredBindingId",
        "Carrier successor mapping",
    )
    for transfer in proof["ownerTransfers"]:
        replacement_id = transfer["replacementBindingId"]
        retired_id = transfer["retiredBindingId"]
        group = groups.get(replacement_id)
        mapping = mappings.get(retired_id)
        if (
            not isinstance(group, dict)
            or not isinstance(mapping, dict)
            or group.get("effectAssetId") != transfer["effectAssetId"]
            or mapping.get("replacementBindingId") != replacement_id
            or mapping.get("replacementEffectAssetId")
            != transfer["effectAssetId"]
        ):
            raise ResealError(
                f"Carrier owner transfer join is missing: {replacement_id}"
            )
        _validate_old_owner(group, transfer, group=True)
        _validate_old_owner(mapping, transfer, group=False)
        _apply_new_owner(group, transfer, group=True)
        _apply_new_owner(mapping, transfer, group=False)

    outputs = carrier_receipt.get("outputs")
    cue_output = (outputs or {}).get("cues")
    if (
        not isinstance(cue_output, dict)
        or cue_output.get("canonicalSha256")
        != migration.FOUR_SLASH_OLD_CUE_CANONICAL_SHA256
    ):
        raise ResealError("Carrier cue preimage seal drifted.")
    cue_output["canonicalSha256"] = (
        migration.FOUR_SLASH_NEW_CUE_CANONICAL_SHA256
    )
    carrier_receipt["fourSlashPatternSplitOwnerReseal"] = copy.deepcopy(
        proof
    )


def _apply_clip01_screen_post_overlay(
    carrier_receipt: dict[str, Any],
) -> None:
    if "clip01ScreenPostSuccessorOverlay" in carrier_receipt:
        raise ResealError("Carrier ScreenPost overlay proof already exists.")
    carrier_receipt["clip01ScreenPostSuccessorOverlay"] = copy.deepcopy(
        migration.expected_four_slash_clip01_screen_post_overlay()
    )


def build_resealed_receipts(
    carrier_receipt: dict[str, Any],
    migration_receipt: dict[str, Any],
    bindings: dict[str, Any],
    cues: dict[str, Any],
    action_bindings: dict[str, Any],
) -> tuple[str, dict[str, Any], dict[str, Any]]:
    _validate_current_split(bindings, cues, action_bindings)
    baseline_identity = migration_receipt.get("baselineIdentity")
    if (
        not isinstance(baseline_identity, dict)
        or migration.canonical_sha256(baseline_identity)
        != EXPECTED_BASELINE_IDENTITY_CANONICAL_SHA256
    ):
        raise ResealError("Historical migration baselineIdentity drifted.")
    preserved_baseline_identity = copy.deepcopy(baseline_identity)

    carrier_hash = migration.canonical_sha256(carrier_receipt)
    migration_hash = migration.canonical_sha256(migration_receipt)
    if carrier_hash == EXPECTED_OLD_CARRIER_RECEIPT_CANONICAL_SHA256:
        if migration_hash != EXPECTED_OLD_MIGRATION_RECEIPT_CANONICAL_SHA256:
            raise ResealError("Carrier/migration receipt state is mixed.")
        state = "SEALED_PREIMAGE"
        target_carrier = copy.deepcopy(carrier_receipt)
        _apply_owner_reseal(target_carrier)
        _apply_clip01_screen_post_overlay(target_carrier)
    elif (
        carrier_hash
        == EXPECTED_OWNER_RESEALED_CARRIER_RECEIPT_CANONICAL_SHA256
    ):
        if (
            migration_hash
            != EXPECTED_OWNER_RESEALED_MIGRATION_RECEIPT_CANONICAL_SHA256
        ):
            raise ResealError("Carrier/migration receipt state is mixed.")
        state = "OWNER_RESEALED_SUCCESSOR"
        target_carrier = copy.deepcopy(carrier_receipt)
        _apply_clip01_screen_post_overlay(target_carrier)
    elif carrier_hash == EXPECTED_NEW_CARRIER_RECEIPT_CANONICAL_SHA256:
        if migration_hash != EXPECTED_NEW_MIGRATION_RECEIPT_CANONICAL_SHA256:
            raise ResealError("Carrier/migration receipt state is mixed.")
        state = "RESEALED_SUCCESSOR"
        target_carrier = copy.deepcopy(carrier_receipt)
    else:
        raise ResealError(
            "Carrier receipt is neither the sealed preimage nor successor."
        )

    if migration.canonical_sha256(target_carrier) != (
        EXPECTED_NEW_CARRIER_RECEIPT_CANONICAL_SHA256
    ):
        raise ResealError("Carrier successor receipt hash drifted.")
    target_migration = copy.deepcopy(migration_receipt)
    target_migration["carrierV1SuccessorContract"] = (
        migration.build_carrier_v1_successor_contract(
            target_carrier,
            cues,
            preserved_baseline_identity["cues"],
        )
    )
    if target_migration.get("baselineIdentity") != preserved_baseline_identity:
        raise ResealError("Historical baselineIdentity was mutated by reseal.")
    migration.check_receipt(
        target_migration,
        bindings,
        cues,
        target_carrier,
    )
    if migration.canonical_sha256(target_migration) != (
        EXPECTED_NEW_MIGRATION_RECEIPT_CANONICAL_SHA256
    ):
        raise ResealError("Migration successor receipt hash drifted.")
    return state, target_carrier, target_migration


def build_outputs() -> tuple[str, dict[Path, bytes]]:
    state, carrier_receipt, migration_receipt = build_resealed_receipts(
        migration.read_json(CARRIER_RECEIPT_PATH),
        migration.read_json(MIGRATION_RECEIPT_PATH),
        migration.read_json(BINDINGS_PATH),
        migration.read_json(CUES_PATH),
        migration.read_json(ACTION_BINDINGS_PATH),
    )
    return state, {
        CARRIER_RECEIPT_PATH: migration.pretty_bytes(carrier_receipt),
        MIGRATION_RECEIPT_PATH: migration.pretty_bytes(migration_receipt),
    }


def _changed_outputs(writes: dict[Path, bytes]) -> dict[Path, bytes]:
    return {
        path: payload
        for path, payload in writes.items()
        if not path.is_file() or path.read_bytes() != payload
    }


def _atomic_replace(
    writes: dict[Path, bytes],
    replace: Callable[[str, str], None] = os.replace,
) -> None:
    if not writes:
        return
    ordered = sorted(writes.items(), key=lambda row: str(row[0]))
    before = {
        path: path.read_bytes() if path.is_file() else None
        for path, _ in ordered
    }
    staged: dict[Path, Path] = {}
    committed: list[Path] = []
    try:
        for ordinal, (path, payload) in enumerate(ordered):
            path.parent.mkdir(parents=True, exist_ok=True)
            temporary = path.with_name(
                f"{path.name}.four-slash-reseal.{os.getpid()}.{ordinal}.staging"
            )
            temporary.write_bytes(payload)
            staged[path] = temporary
        for path, _ in ordered:
            replace(str(staged[path]), str(path))
            committed.append(path)
    except Exception as error:
        rollback_errors = []
        for ordinal, path in enumerate(reversed(committed)):
            try:
                previous = before[path]
                if previous is None:
                    path.unlink(missing_ok=True)
                else:
                    recovery = path.with_name(
                        f"{path.name}.four-slash-reseal.rollback."
                        f"{os.getpid()}.{ordinal}"
                    )
                    recovery.write_bytes(previous)
                    os.replace(str(recovery), str(path))
            except Exception as rollback_error:  # pragma: no cover
                rollback_errors.append(f"{path}: {rollback_error}")
        suffix = (
            "; rollback errors: " + "; ".join(rollback_errors)
            if rollback_errors
            else ""
        )
        raise ResealError(
            f"Four-slash owner reseal transaction failed: {error}{suffix}"
        ) from error
    finally:
        for temporary in staged.values():
            temporary.unlink(missing_ok=True)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--write", action="store_true")
    args = parser.parse_args(argv)

    live_bindings = migration.read_json(BINDINGS_PATH)
    live_cues = migration.read_json(CUES_PATH)
    if any(
        isinstance(row, dict)
        and row.get("bindingId")
        == "cue.valtan.carrier-v1.attack.four-slash.active.clip-01"
        and row.get("patternId") == "VALTAN_FOUR_SLASH"
        for row in live_cues.get("cues", [])
    ):
        migration.validate_rejoined_four_slash_successor(
            live_bindings, live_cues
        )
        print(
            "Valtan four-slash split-owner reseal retired: current product "
            "is the validated VALTAN_FOUR_SLASH successor; changed=0"
        )
        return 0

    state, writes = build_outputs()
    changed = _changed_outputs(writes)
    if args.check:
        if changed:
            paths = "\n  ".join(
                repository_path(path) for path in sorted(changed)
            )
            raise ResealError(
                "Four-slash successor receipts are stale; run --write:\n  "
                + paths
            )
    else:
        _atomic_replace(changed)
        verified_state, verified_writes = build_outputs()
        if (
            verified_state != "RESEALED_SUCCESSOR"
            or _changed_outputs(verified_writes)
        ):
            raise ResealError("Four-slash successor receipt readback failed.")
    print(
        "Valtan four-slash owner reseal "
        f"{'check' if args.check else 'write'}: "
        f"state={state} changed={len(changed)} transfers=3"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (ResealError, migration.MigrationError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
