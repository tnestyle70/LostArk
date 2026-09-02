#!/usr/bin/env python3
"""Validate the requested Valtan pattern coverage against its real owners.

This validator deliberately distinguishes the split Product source admitted by
Complete Play from the larger Encounter/reference catalog.  A pattern existing
in ``ValtanEncounter.json`` is not, by itself, a playable Product pattern.
"""

from __future__ import annotations

import argparse
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any


STATUS_PATTERN_CONTRACTS: dict[str, dict[str, Any]] = {
    "VALTAN_STAGGER_SLOT": {
        "displayName": "마력구 파괴 패턴",
        "sourceChainId": "derived.stagger-slot",
        "sourceActionIds": (420617,),
        "sourceSequenceIndex": 1,
        "presentationSources": (
            (420617, 1, "PRIMARY"),
        ),
        "actionId": "valtan.authoring.stagger-slot",
        "targetPolicy": "NONE",
        "aimPolicy": "NONE",
        "scriptedSequenceMember": True,
    },
    "VALTAN_GROGGY_FOLLOWUP": {
        "displayName": "발탄 공용 그로기 후속",
        "sourceChainId": "derived.valtan-groggy-followup",
        "authoringPhase": 3,
        "sourceActionIds": (420618,),
        "sourceSequenceIndex": 0,
        "presentationSources": (
            (420618, 0, "PRIMARY"),
        ),
        "actionId": "valtan.followup.groggy",
        "targetPolicy": "NONE",
        "aimPolicy": "NONE",
        "scriptedSequenceMember": False,
    },
    "VALTAN_BIND_SLOT": {
        "displayName": "속박 패턴",
        "sourceChainId": "derived.bind-slot",
        "sourceActionIds": (420623, 400442),
        "sourceSequenceIndex": 1,
        "presentationSources": (
            (420623, 1, "PRIMARY"),
            (400442, 0, "REFERENCE_400442_0"),
        ),
        "actionId": "valtan.authoring.bind-slot",
        "targetPolicy": "LOCK_RANDOM_ALIVE_ON_START",
        "aimPolicy": "LOCK_FACING_ON_START",
        "scriptedSequenceMember": True,
    },
    "VALTAN_SILENCE_SLOT": {
        "displayName": "침묵 패턴",
        "sourceChainId": "derived.silence-slot",
        "sourceActionIds": (400440, 400437),
        "sourceSequenceIndex": 0,
        "presentationSources": (
            (400440, 0, "PRIMARY"),
            (400437, 0, "REFERENCE_400437_0"),
        ),
        "actionId": "valtan.authoring.silence-slot",
        "targetPolicy": "NONE",
        "aimPolicy": "NONE",
        "scriptedSequenceMember": True,
    },
}

REQUESTED_PRODUCT_IDS = (
    "VALTAN_HIGH_JUMP",
    "VALTAN_SIX_PIZZA_106",
    "VALTAN_WARP",
    "VALTAN_TRASH",
    "VALTAN_CATCH_BREATH",
    "VALTAN_STRUGGLING",
    "VALTAN_DASH_CHARGE",
    "VALTAN_GROUND_ROAR",
    "VALTAN_TRIPLE_COUNTER",
    "VALTAN_STAGGER_SLOT",
    "VALTAN_GROGGY_FOLLOWUP",
    "VALTAN_BIND_SLOT",
    "VALTAN_SILENCE_SLOT",
)

REQUESTED_REFERENCE_ONLY_IDS = (
    "VALTAN_MAGIC_ORB_STAGGER_76",
    "VALTAN_FOUR_PILLARS_105",
)

class CoverageError(RuntimeError):
    """Raised when Product/reference/runtime admission no longer exact-joins."""


@dataclass(frozen=True)
class CoverageReport:
    product_count: int
    encounter_count: int
    product_ids: frozenset[str]
    encounter_ids: frozenset[str]
    scripted_pattern_ids: frozenset[str]
    status_pattern_ids: frozenset[str]


def _load_object(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise CoverageError(f"failed to load {path}: {error}") from error
    if not isinstance(value, dict):
        raise CoverageError(f"{path} must contain one JSON object")
    return value


def _index_patterns(document: dict[str, Any], label: str) -> dict[str, dict[str, Any]]:
    rows = document.get("patterns")
    if not isinstance(rows, list):
        raise CoverageError(f"{label}.patterns must be an array")
    indexed: dict[str, dict[str, Any]] = {}
    for ordinal, row in enumerate(rows):
        if not isinstance(row, dict):
            raise CoverageError(f"{label}.patterns[{ordinal}] must be an object")
        pattern_id = row.get("patternId")
        if not isinstance(pattern_id, str) or not pattern_id:
            raise CoverageError(f"{label}.patterns[{ordinal}] has no stable patternId")
        if pattern_id in indexed:
            raise CoverageError(f"{label} duplicates patternId {pattern_id}")
        indexed[pattern_id] = row
    return indexed


def _stage_identity(row: dict[str, Any], owner: str) -> list[tuple[str, str]]:
    stages = row.get("stages")
    if not isinstance(stages, list) or not stages:
        raise CoverageError(f"{owner} has no stages")
    identities: list[tuple[str, str]] = []
    for ordinal, stage in enumerate(stages):
        if not isinstance(stage, dict):
            raise CoverageError(f"{owner}.stages[{ordinal}] must be an object")
        stage_id = stage.get("stageId")
        action_id = stage.get("actionId")
        if not isinstance(stage_id, str) or not stage_id:
            raise CoverageError(f"{owner}.stages[{ordinal}] has no stageId")
        if not isinstance(action_id, str) or not action_id:
            raise CoverageError(f"{owner}.stages[{ordinal}] has no actionId")
        identities.append((stage_id, action_id))
    if len(identities) != len(set(identities)):
        raise CoverageError(f"{owner} duplicates a stage/action identity")
    return identities


def _validate_status_patterns(
    gameplay: dict[str, Any],
    gameplay_by_id: dict[str, dict[str, Any]],
    presentation_by_id: dict[str, dict[str, Any]],
    encounter_by_id: dict[str, dict[str, Any]],
    bindings: dict[str, Any],
    effect_cues: dict[str, Any],
    promotion_manifest: dict[str, Any],
) -> frozenset[str]:
    decision_model = gameplay.get("decisionModel")
    manual_rows = (
        decision_model.get("manualAuditions")
        if isinstance(decision_model, dict)
        else None
    )
    if not isinstance(manual_rows, list):
        raise CoverageError("Valtan.gameplay manualAuditions must be an array")
    manual_by_id: dict[str, dict[str, Any]] = {}
    for ordinal, row in enumerate(manual_rows):
        if not isinstance(row, dict) or not isinstance(row.get("patternId"), str):
            raise CoverageError(f"manualAuditions[{ordinal}] has no patternId")
        if row["patternId"] in manual_by_id:
            raise CoverageError(
                f'manualAuditions duplicates patternId {row["patternId"]}'
            )
        manual_by_id[row["patternId"]] = row

    binding_rows = bindings.get("bindings")
    if not isinstance(binding_rows, list):
        raise CoverageError("Valtan.patternbindings.bindings must be an array")
    binding_by_action: dict[str, dict[str, Any]] = {}
    for ordinal, row in enumerate(binding_rows):
        if not isinstance(row, dict) or not isinstance(row.get("actionId"), str):
            raise CoverageError(f"Valtan.patternbindings[{ordinal}] has no actionId")
        if row["actionId"] in binding_by_action:
            raise CoverageError(
                f'Valtan.patternbindings duplicates actionId {row["actionId"]}'
            )
        binding_by_action[row["actionId"]] = row

    cue_rows = effect_cues.get("cues")
    if not isinstance(cue_rows, list):
        raise CoverageError("Valtan.patterneffectcues.cues must be an array")
    promoted_rows = promotion_manifest.get("patterns")
    if not isinstance(promoted_rows, list):
        raise CoverageError("Valtan animation promotion patterns must be an array")
    promoted_pattern_ids = {
        row.get("patternId") for row in promoted_rows if isinstance(row, dict)
    }
    promoted_chain_ids = {
        row.get("sourceChainId") for row in promoted_rows if isinstance(row, dict)
    }

    expected_status_events = {
        "VALTAN_STAGGER_SLOT": (
            ("CHANNEL", None, 0, 12000),
            ("FINAL_ATTACK", None, 0, 3000),
        ),
        "VALTAN_GROGGY_FOLLOWUP": (
            ("GROGGY", "SET_BOSS_FLAG", True, 6833),
        ),
        "VALTAN_BIND_SLOT": (
            ("STEP_01", "SET_PLAYER_BIND", 5000, 5000),
            ("RECOVERY", None, 0, 3533),
        ),
        "VALTAN_SILENCE_SLOT": (
            ("STEP_01", None, 0, 2633),
            ("SILENCE_APPLY", "SET_PLAYER_SILENCE", 1, 100),
        ),
    }

    expected_animation_occurrences = {
        ("VALTAN_STAGGER_SLOT", "CHANNEL"): {
            "endPolicy": "LOOP_TO_STAGE_END",
            "occurrences": (
                ("mesh_att_battle_17_start", 0, 2000, False),
                ("mesh_att_battle_17_loop", 0, 0, True),
            ),
        },
        ("VALTAN_STAGGER_SLOT", "FINAL_ATTACK"): {
            "endPolicy": "EXACT",
            "occurrences": (("mesh_att_battle_17_end", 0, 3000, False),),
        },
        ("VALTAN_GROGGY_FOLLOWUP", "GROGGY"): {
            "endPolicy": "EXACT",
            "occurrences": (
                ("mesh_abn_groggy_1_start", 0, 1833, False),
                ("mesh_abn_groggy_1_loop", 0, 1333, False),
                ("mesh_abn_groggy_1_loop", 0, 1333, False),
                ("mesh_abn_groggy_1_loop", 0, 334, False),
                ("mesh_abn_groggy_1_end", 0, 2000, False),
            ),
        },
        ("VALTAN_BIND_SLOT", "STEP_01"): {
            "endPolicy": "EXACT",
            "occurrences": (
                ("mesh_att_battle_5_01_start", 0, 1400, False),
                ("mesh_att_battle_5_01_loop", 0, 900, False),
                ("mesh_att_battle_5_01_loop", 0, 900, False),
                ("mesh_att_battle_5_01_loop", 0, 900, False),
                ("mesh_att_battle_5_01_end", 0, 900, False),
            ),
        },
        ("VALTAN_BIND_SLOT", "RECOVERY"): {
            "endPolicy": "EXACT",
            "occurrences": (("mesh_att_battle_5_01_end", 900, 3533, False),),
        },
        ("VALTAN_SILENCE_SLOT", "STEP_01"): {
            "endPolicy": "EXACT",
            "occurrences": (
                ("mesh_evt1_att_battle_5_01_end", 0, 2633, False),
            ),
        },
    }

    for pattern_id, contract in STATUS_PATTERN_CONTRACTS.items():
        expected_manual = {
            "patternId": pattern_id,
            "sourceChainId": contract["sourceChainId"],
            "authoringPhase": contract.get("authoringPhase", 1),
            "admissionState": "DERIVED_SERVER_PATTERN",
        }
        if manual_by_id.get(pattern_id) != expected_manual:
            raise CoverageError(
                f"status Pattern manual owner differs: {pattern_id}"
            )
        gameplay_row = gameplay_by_id.get(pattern_id)
        presentation_row = presentation_by_id.get(pattern_id)
        encounter_row = encounter_by_id.get(pattern_id)
        if not isinstance(gameplay_row, dict) or not isinstance(presentation_row, dict):
            raise CoverageError(f"status Pattern split owner is missing: {pattern_id}")
        if not isinstance(encounter_row, dict):
            raise CoverageError(f"status Pattern Product row is missing: {pattern_id}")
        if (gameplay_row.get("displayName") != contract["displayName"] or
                encounter_row.get("displayName") != contract["displayName"]):
            raise CoverageError(f"status Pattern display name differs: {pattern_id}")
        expected_source_action_ids = contract["sourceActionIds"]
        if (tuple(gameplay_row.get("sourceActionIds", ())) !=
                expected_source_action_ids or
                tuple(encounter_row.get("sourceActionIds", ())) !=
                expected_source_action_ids):
            raise CoverageError(
                f"status Pattern source action closure differs: {pattern_id}"
            )
        presentation_sources = presentation_row.get("presentationSources")
        if (not isinstance(presentation_sources, list) or
                len(presentation_sources) != len(contract["presentationSources"]) or
                any(not isinstance(row, dict) for row in presentation_sources) or
                presentation_row.get("sourceSequenceIndex") !=
                contract["sourceSequenceIndex"] or
                tuple(
                    (
                        row.get("sourceActionId"),
                        row.get("sequenceIndex"),
                        row.get("role"),
                    )
                    for row in presentation_sources
                    if isinstance(row, dict)
                ) != contract["presentationSources"]):
            raise CoverageError(
                f"status Pattern presentation source closure differs: {pattern_id}"
            )
        if (gameplay_row.get("targetPolicy") != contract["targetPolicy"] or
                gameplay_row.get("aimPolicy") != contract["aimPolicy"]):
            raise CoverageError(f"status Pattern target contract differs: {pattern_id}")
        gameplay_stages = gameplay_row.get("stages")
        presentation_stages = presentation_row.get("stages")
        expected_rows = expected_status_events[pattern_id]
        if (not isinstance(gameplay_stages, list) or
                [row.get("stageId") for row in gameplay_stages] !=
                [row[0] for row in expected_rows]):
            raise CoverageError(f"status Pattern Stage closure differs: {pattern_id}")
        if (not isinstance(presentation_stages, list) or
                _stage_identity(gameplay_row, pattern_id) !=
                _stage_identity(presentation_row, pattern_id)):
            raise CoverageError(f"status Pattern presentation join differs: {pattern_id}")
        for stage, (_, expected_kind, expected_value, expected_duration) in zip(
                gameplay_stages, expected_rows):
            if stage.get("durationMs") != expected_duration:
                raise CoverageError(f"status Pattern duration differs: {pattern_id}")
            enter = next((event for event in stage.get("events", [])
                          if event.get("trigger") == "ENTER"), None)
            if expected_kind is None:
                if stage.get("events"):
                    raise CoverageError(f"status stage must be event-free: {pattern_id}")
            elif not isinstance(enter, dict) or enter.get("kind") != expected_kind:
                raise CoverageError(f"status Pattern typed ENTER differs: {pattern_id}")
            elif expected_kind == "SET_PLAYER_BIND" and (
                    enter.get("heightM") != expected_value / 1000 or
                    enter.get("durationMs") != expected_duration):
                raise CoverageError(f"status bind contract differs: {pattern_id}")
            elif expected_kind == "SET_PLAYER_SILENCE" and (
                    enter.get("durationMs") != 5000 or
                    len(stage.get("events", [])) != 1):
                raise CoverageError(f"status silence contract differs: {pattern_id}")
            elif expected_kind == "SET_BOSS_FLAG" and (
                    enter.get("flagId") != "boss.flag.groggy" or
                    enter.get("enabled") is not expected_value):
                raise CoverageError(f"status groggy contract differs: {pattern_id}")

        if pattern_id == "VALTAN_STAGGER_SLOT":
            channel, final_attack = gameplay_stages
            channel_branches = channel.get("branches")
            if not isinstance(channel_branches, list) or len(channel_branches) != 2:
                raise CoverageError("magic-orb response branches differ")
            success, timeout = channel_branches
            if ("verticalOffsetM" in gameplay_row or
                    channel.get("verticalOffsetM") != 0.5 or
                    "verticalOffsetM" in final_attack or
                    channel.get("bossResponse") != {
                        "kind": "ACCUMULATED_HEALTH_DAMAGE",
                        "threshold": 1000,
                    } or
                    success != {
                        "outcome": "HEALTH_DAMAGE_THRESHOLD_REACHED",
                        "nextActionId": None,
                        "nextPatternId": "VALTAN_GROGGY_FOLLOWUP",
                    } or
                    timeout != {
                        "outcome": "TIMEOUT",
                        "nextActionId": "valtan.authoring.stagger-slot.final-attack",
                    } or
                    final_attack.get("hit", {}).get("shape") != {
                        "kind": "CIRCLE", "outerRadiusM": 100.0
                    } or
                    final_attack.get("hit", {}).get("schedule") != {
                        "kind": "INTERVAL", "count": 1,
                        "firstOffsetMs": 2900, "intervalMs": 0,
                    } or
                    final_attack.get("hit", {}).get("serverDamageProfileId") !=
                    "damage.valtan.omnidirectional-wipe-130"):
                raise CoverageError("magic-orb damage response or wipe contract differs")
        gameplay_stage_by_id = {stage["stageId"]: stage for stage in gameplay_stages}
        for stage in presentation_stages:
            stage_id = stage["stageId"]
            expected_animation = expected_animation_occurrences.get(
                (pattern_id, stage_id)
            )
            animation = stage.get("animation")
            if expected_animation is None:
                if animation != {"mode": "NONE"}:
                    raise CoverageError(
                        f"status Pattern unexpected animation: {pattern_id}/{stage_id}"
                    )
            else:
                expected_occurrences = expected_animation["occurrences"]
                expected_end_policy = expected_animation["endPolicy"]
                occurrences = animation.get("occurrences") if isinstance(animation, dict) else None
                if (not isinstance(occurrences, list) or
                        animation.get("endPolicy") != expected_end_policy or
                        animation.get("repeatCount") != 1 or
                        tuple(
                            (
                                row.get("clip"), row.get("sourceStartMs"),
                                row.get("playMs"), row.get("repeatUntilStageEnd"),
                            )
                            for row in occurrences
                        ) != expected_occurrences or
                        any(
                            row.get("mappingBasis") != "PROJECT_AUTHORED" or
                            row.get("playRate") != 1
                            for row in occurrences
                        ) or (
                        expected_end_policy == "EXACT" and
                        sum(row.get("playMs", 0) for row in occurrences) !=
                        gameplay_stage_by_id[stage_id]["durationMs"])):
                    raise CoverageError(
                        f"status Pattern selected animation differs: {pattern_id}/{stage_id}"
                    )
                if expected_end_policy == "LOOP_TO_STAGE_END" and (
                        not occurrences[-1].get("repeatUntilStageEnd") or
                        sum(row.get("playMs", 0) for row in occurrences[:-1]) >=
                        gameplay_stage_by_id[stage_id]["durationMs"]):
                    raise CoverageError(
                        f"status Pattern loop animation differs: {pattern_id}/{stage_id}"
                    )
            if stage.get("effectCues"):
                raise CoverageError(f"status Pattern unexpectedly owns an Effect cue: {pattern_id}")
            binding = binding_by_action.get(stage["actionId"])
            if expected_animation is None:
                if binding is not None and binding != {
                    "actionId": stage["actionId"], "playbackMode": "NONE", "clips": []
                }:
                    raise CoverageError(f"status Pattern binding is not NONE: {pattern_id}")
            elif (not isinstance(binding, dict) or
                  tuple(
                      (
                          row.get("clip"), row.get("sourceStartMs"),
                          row.get("playMs"), row.get("loop"),
                      )
                      for row in binding.get("clips", [])
                  ) != expected_occurrences or
                  any(
                      row.get("mappingBasis") != "PROJECT_AUTHORED" or
                      row.get("playRate") != 1
                      for row in binding.get("clips", [])
                  )):
                raise CoverageError(
                    f"status Pattern binding differs: {pattern_id}/{stage_id}"
                )
        if any(
            isinstance(row, dict) and row.get("patternId") == pattern_id
            for row in cue_rows
        ):
            raise CoverageError(
                f"status Pattern unexpectedly owns an Effect cue: {pattern_id}"
            )
        if (
            pattern_id in promoted_pattern_ids
            or contract["sourceChainId"] in promoted_chain_ids
        ):
            raise CoverageError(
                f"derived status Pattern leaked into manual animation promotion: {pattern_id}"
            )

    return frozenset(STATUS_PATTERN_CONTRACTS)


def _validate_runtime_inventory_source(root: Path) -> None:
    source_path = root / "Client/Private/ValtanPatternTree.cpp"
    try:
        source = source_path.read_text(encoding="utf-8-sig")
    except (OSError, UnicodeError) as error:
        raise CoverageError(f"failed to read {source_path}: {error}") from error

    playable_begin = source.find("CValtanPatternTree::Build_PlayablePatternInventory(")
    next_begin = source.find("CValtanPatternTree::Build_NextPatternInventory(", playable_begin)
    summary_begin = source.find("CValtanPatternTree::Build_PatternIdentitySummary(", next_begin)
    if min(playable_begin, next_begin, summary_begin) < 0:
        raise CoverageError("runtime playable/Next inventory functions are missing")

    playable = source[playable_begin:next_begin]
    next_inventory = source[next_begin:summary_begin]
    for marker in (
        "if (!Pattern.bAuthoringMasterManaged)",
        "Pattern.bManualServerAudition",
        "StagedPatterns.push_back(&Pattern)",
        "OutInventory = std::move(Staged)",
    ):
        if marker not in playable:
            raise CoverageError(
                f"Build_PlayablePatternInventory lost Product admission marker: {marker}"
            )
    for pattern_id in REQUESTED_REFERENCE_ONLY_IDS:
        if pattern_id in playable:
            raise CoverageError(
                f"Build_PlayablePatternInventory hard-codes reference row {pattern_id}"
            )
    if "Build_PlayablePatternInventory(View, Inventory, strOutError)" not in next_inventory:
        raise CoverageError("Next no longer derives from Complete Play admission")


def validate(root: Path) -> CoverageReport:
    root = root.resolve()
    gameplay = _load_object(root / "Data/Valtan/Valtan.gameplay.json")
    presentation = _load_object(root / "Data/Valtan/Valtan.presentation.json")
    encounter = _load_object(root / "Data/Encounters/Valtan/ValtanEncounter.json")
    rotations = _load_object(
        root / "Data/Encounters/Valtan/ValtanPatternRotations.json"
    )
    bindings = _load_object(
        root / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
    )
    effect_cues = _load_object(
        root / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
    )
    promotion_manifest = _load_object(
        root / "Data/Valtan/Valtan.animation-chain-promotions.json"
    )

    gameplay_by_id = _index_patterns(gameplay, "Valtan.gameplay")
    presentation_by_id = _index_patterns(presentation, "Valtan.presentation")
    encounter_by_id = _index_patterns(encounter, "ValtanEncounter")
    product_ids = frozenset(gameplay_by_id)
    encounter_ids = frozenset(encounter_by_id)

    if not product_ids:
        raise CoverageError("split Product has no patterns")
    if not encounter_ids:
        raise CoverageError("Encounter/reference catalog has no patterns")
    if frozenset(presentation_by_id) != product_ids:
        raise CoverageError("gameplay/presentation split Product identity sets differ")
    if not product_ids <= encounter_ids:
        missing = sorted(product_ids - encounter_ids)
        raise CoverageError(f"Product rows are missing from Encounter projection: {missing}")

    for pattern_id in sorted(product_ids):
        gameplay_identity = _stage_identity(
            gameplay_by_id[pattern_id], f"Valtan.gameplay/{pattern_id}"
        )
        presentation_identity = _stage_identity(
            presentation_by_id[pattern_id], f"Valtan.presentation/{pattern_id}"
        )
        encounter_identity = _stage_identity(
            encounter_by_id[pattern_id], f"ValtanEncounter/{pattern_id}"
        )
        if gameplay_identity != presentation_identity:
            raise CoverageError(
                f"gameplay/presentation stage identity differs for {pattern_id}"
            )
        if gameplay_identity != encounter_identity:
            raise CoverageError(
                f"split Product/Encounter stage identity differs for {pattern_id}"
            )

    for pattern_id in REQUESTED_PRODUCT_IDS:
        if pattern_id not in product_ids:
            raise CoverageError(f"requested Product pattern is not playable: {pattern_id}")
    for pattern_id in REQUESTED_REFERENCE_ONLY_IDS:
        if pattern_id not in encounter_ids:
            raise CoverageError(f"requested reference pattern is missing: {pattern_id}")
        if pattern_id in product_ids:
            raise CoverageError(
                f"reference-only pattern was silently admitted to Product: {pattern_id}"
            )

    status_pattern_ids = _validate_status_patterns(
        gameplay,
        gameplay_by_id,
        presentation_by_id,
        encounter_by_id,
        bindings,
        effect_cues,
        promotion_manifest,
    )

    decision_model = gameplay.get("decisionModel")
    sequence = (
        decision_model.get("scriptedSequence")
        if isinstance(decision_model, dict)
        else None
    )
    if not isinstance(sequence, dict) or set(sequence) != {
        "sequenceId",
        "mode",
        "interStepPursuitMs",
        "patternIds",
    }:
        raise CoverageError(
            "Valtan.gameplay scriptedSequence must be the inline canonical contract"
        )
    rows = sequence.get("patternIds")
    if not isinstance(rows, list) or not rows:
        raise CoverageError("Valtan.gameplay scriptedSequence.patternIds is empty")
    scripted_pattern_ids: set[str] = set()
    for ordinal, pattern_id in enumerate(rows):
        if not isinstance(pattern_id, str) or pattern_id not in product_ids:
            raise CoverageError(
                "canonical scriptedSequence admits non-Product pattern at "
                f"index {ordinal}: {pattern_id!r}"
            )
        scripted_pattern_ids.add(pattern_id)
    if rotations.get("scriptedSequence") != sequence:
        raise CoverageError(
            "generated ValtanPatternRotations scriptedSequence differs from canonical gameplay"
        )

    expected_scripted_status_patterns = frozenset(
        pattern_id
        for pattern_id, contract in STATUS_PATTERN_CONTRACTS.items()
        if contract["scriptedSequenceMember"]
    )
    actual_scripted_status_patterns = status_pattern_ids & scripted_pattern_ids
    if actual_scripted_status_patterns != expected_scripted_status_patterns:
        raise CoverageError(
            "derived status Pattern scriptedSequence membership differs: "
            f"expected={sorted(expected_scripted_status_patterns)} "
            f"actual={sorted(actual_scripted_status_patterns)}"
        )

    _validate_runtime_inventory_source(root)
    return CoverageReport(
        product_count=len(product_ids),
        encounter_count=len(encounter_ids),
        product_ids=product_ids,
        encounter_ids=encounter_ids,
        scripted_pattern_ids=frozenset(scripted_pattern_ids),
        status_pattern_ids=status_pattern_ids,
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--root",
        type=Path,
        default=Path(__file__).resolve().parents[2],
        help="repository root",
    )
    args = parser.parse_args()
    try:
        report = validate(args.root)
    except CoverageError as error:
        print(f"FAIL: {error}")
        return 1

    print(
        f"Valtan requested-pattern coverage: Product {report.product_count} / "
        f"Encounter {report.encounter_count}"
    )
    for pattern_id in REQUESTED_PRODUCT_IDS:
        print(f"  PRODUCT_SPLIT_PLAYABLE: {pattern_id}")
    for pattern_id in REQUESTED_REFERENCE_ONLY_IDS:
        print(f"  ENCOUNTER_REFERENCE_ONLY: {pattern_id}")
    for pattern_id in STATUS_PATTERN_CONTRACTS:
        print(f"  DERIVED_STATUS_PATTERN: {pattern_id}")
    print(
        "  CANONICAL_SCRIPTED_SEQUENCE_PRODUCT_IDS: "
        f"{len(report.scripted_pattern_ids)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
