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


ANIMATIONLESS_SKELETON_CONTRACTS: dict[str, dict[str, Any]] = {
    "VALTAN_STAGGER_SLOT": {
        "displayName": "무력화 패턴 (애니메이션 슬롯 미지정)",
        "sourceChainId": "derived.stagger-slot",
        "sourceActionId": 420642,
        "sourceSequenceIndex": 1,
        "actionId": "valtan.authoring.stagger-slot",
        "targetPolicy": "NONE",
        "aimPolicy": "NONE",
    },
    "VALTAN_BIND_SLOT": {
        "displayName": "속박 패턴 (애니메이션 슬롯 미지정)",
        "sourceChainId": "derived.bind-slot",
        "sourceActionId": 420623,
        "sourceSequenceIndex": 1,
        "actionId": "valtan.authoring.bind-slot",
        "targetPolicy": "LOCK_RANDOM_ALIVE_ON_START",
        "aimPolicy": "LOCK_FACING_ON_START",
    },
    "VALTAN_SILENCE_SLOT": {
        "displayName": "침묵 패턴 (애니메이션 슬롯 미지정)",
        "sourceChainId": "derived.silence-slot",
        "sourceActionId": 400440,
        "sourceSequenceIndex": 0,
        "actionId": "valtan.authoring.silence-slot",
        "targetPolicy": "NONE",
        "aimPolicy": "NONE",
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
    flow_pattern_ids: frozenset[str]
    animationless_skeleton_ids: frozenset[str]


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


def _skeleton_eligibility() -> dict[str, Any]:
    return {
        "armorRequirement": "ANY",
        "phaseRequirement": "ANY",
        "minimumGameplayPhase": 1,
        "maximumGameplayPhase": 3,
        "minimumHealthBarInclusive": 0,
        "maximumHealthBarInclusive": 0,
        "minimumRangeM": 0.0,
        "maximumRangeM": 1.0,
        "cooldownPolicy": "DERIVED_SOURCE_ACTION",
        "selectionCooldownMs": None,
        "cooldownGroupId": None,
        "repeatPolicy": {
            "kind": "SOFT_AVOID_UNLESS_ONLY_ELIGIBLE",
            "limit": 0,
        },
    }


def _expected_gameplay_skeleton(
    pattern_id: str, contract: dict[str, Any]
) -> dict[str, Any]:
    stage_action_id = f'{contract["actionId"]}.step-01'
    return {
        "patternId": pattern_id,
        "displayName": contract["displayName"],
        "category": "NORMAL",
        "compatibilitySelectionWeight": 0,
        "actionId": contract["actionId"],
        "entryActionId": stage_action_id,
        "targetPolicy": contract["targetPolicy"],
        "aimPolicy": contract["aimPolicy"],
        "eligibility": _skeleton_eligibility(),
        "invulnerableWhileRunning": False,
        "sourceActionIds": [contract["sourceActionId"]],
        "serverMotion": None,
        "reactions": [],
        "stages": [
            {
                "stageId": "STEP_01",
                "actionId": stage_action_id,
                "stageKind": "ACTIVE",
                "durationMs": 5000,
                "defaultNextActionId": None,
                "hit": {"shape": {"kind": "NONE"}},
                "motion": None,
                "events": [],
                "branches": [],
            }
        ],
    }


def _expected_presentation_skeleton(
    pattern_id: str, contract: dict[str, Any]
) -> dict[str, Any]:
    stage_action_id = f'{contract["actionId"]}.step-01'
    return {
        "patternId": pattern_id,
        "sourceSequenceIndex": contract["sourceSequenceIndex"],
        "presentationSources": [
            {
                "sourceActionId": contract["sourceActionId"],
                "sequenceIndex": contract["sourceSequenceIndex"],
                "role": "PRIMARY",
            }
        ],
        "stages": [
            {
                "stageId": "STEP_01",
                "actionId": stage_action_id,
                "sequenceRole": "STEP",
                "animation": {"mode": "NONE"},
                "effectCues": [],
                "cameraInvocations": [],
            }
        ],
    }


def _expected_encounter_skeleton(
    pattern_id: str, contract: dict[str, Any]
) -> dict[str, Any]:
    stage_action_id = f'{contract["actionId"]}.step-01'
    return {
        "patternId": pattern_id,
        "category": "NORMAL",
        "minimumPhase": 1,
        "maximumPhase": 3,
        "targetPolicy": contract["targetPolicy"],
        "aimPolicy": contract["aimPolicy"],
        "displayName": contract["displayName"],
        "actionId": contract["actionId"],
        "sourceActionIds": [contract["sourceActionId"]],
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
        "minimumRange": 0.0,
        "maximumRange": 1.0,
        "stages": [
            {
                "stageId": "STEP_01",
                "actionId": stage_action_id,
                "stageKind": "ACTIVE",
                "durationMs": 5000,
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
        ],
    }


def _validate_animationless_skeletons(
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
            ("STEP_01", "SET_STAGGER_GAUGE", 100, 5000),
            ("GROGGY", "SET_BOSS_FLAG", 1, 3000),
            ("RECOVERY", None, 0, 1000),
        ),
        "VALTAN_BIND_SLOT": (
            ("STEP_01", "SET_PLAYER_BIND", 10000, 5000),
        ),
        "VALTAN_SILENCE_SLOT": (
            ("STEP_01", "SET_PLAYER_SILENCE", 1, 5000),
        ),
    }

    for pattern_id, contract in ANIMATIONLESS_SKELETON_CONTRACTS.items():
        expected_manual = {
            "patternId": pattern_id,
            "sourceChainId": contract["sourceChainId"],
            "authoringPhase": 1,
            "admissionState": "DERIVED_SERVER_PATTERN",
        }
        if manual_by_id.get(pattern_id) != expected_manual:
            raise CoverageError(
                f"animationless skeleton manual owner differs: {pattern_id}"
            )
        gameplay_row = gameplay_by_id.get(pattern_id)
        presentation_row = presentation_by_id.get(pattern_id)
        encounter_row = encounter_by_id.get(pattern_id)
        if not isinstance(gameplay_row, dict) or not isinstance(presentation_row, dict):
            raise CoverageError(f"status Pattern split owner is missing: {pattern_id}")
        if not isinstance(encounter_row, dict):
            raise CoverageError(f"status Pattern Product row is missing: {pattern_id}")
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
                    raise CoverageError(f"status recovery must be event-free: {pattern_id}")
            elif not isinstance(enter, dict) or enter.get("kind") != expected_kind:
                raise CoverageError(f"status Pattern typed ENTER differs: {pattern_id}")
            elif expected_kind == "SET_STAGGER_GAUGE" and enter.get("value") != expected_value:
                raise CoverageError(f"status stagger capacity differs: {pattern_id}")
            elif expected_kind == "SET_PLAYER_BIND" and (
                    enter.get("heightM") != expected_value / 1000 or
                    enter.get("durationMs") != expected_duration):
                raise CoverageError(f"status bind contract differs: {pattern_id}")
            elif expected_kind == "SET_PLAYER_SILENCE" and (
                    enter.get("durationMs") != expected_duration):
                raise CoverageError(f"status silence contract differs: {pattern_id}")
        for stage in presentation_stages:
            if stage.get("animation") != {"mode": "NONE"} or stage.get("effectCues"):
                raise CoverageError(f"status Pattern must remain animationless: {pattern_id}")
            binding = binding_by_action.get(stage["actionId"])
            if binding is not None and binding != {
                "actionId": stage["actionId"], "playbackMode": "NONE", "clips": []
            }:
                raise CoverageError(f"status Pattern binding is not NONE: {pattern_id}")
        if any(
            isinstance(row, dict) and row.get("patternId") == pattern_id
            for row in cue_rows
        ):
            raise CoverageError(
                f"animationless skeleton unexpectedly owns an Effect cue: {pattern_id}"
            )
        if (
            pattern_id in promoted_pattern_ids
            or contract["sourceChainId"] in promoted_chain_ids
        ):
            raise CoverageError(
                f"derived skeleton leaked into manual animation promotion: {pattern_id}"
            )

    return frozenset(ANIMATIONLESS_SKELETON_CONTRACTS)


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
    flow = _load_object(root / "Data/Encounters/Valtan/ValtanBossAuditionFlows.json")
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

    animationless_skeleton_ids = _validate_animationless_skeletons(
        gameplay,
        gameplay_by_id,
        presentation_by_id,
        encounter_by_id,
        bindings,
        effect_cues,
        promotion_manifest,
    )

    flows = flow.get("flows")
    if not isinstance(flows, list) or not flows:
        raise CoverageError("ValtanBossAuditionFlows.flows must be a non-empty array")
    flow_pattern_ids: set[str] = set()
    for flow_ordinal, flow_row in enumerate(flows):
        if not isinstance(flow_row, dict):
            raise CoverageError(f"flow[{flow_ordinal}] must be an object")
        rows = flow_row.get("nodes")
        row_label = "nodes"
        if not isinstance(rows, list):
            rows = flow_row.get("slots")
            row_label = "slots"
        if not isinstance(rows, list):
            raise CoverageError(f"flow[{flow_ordinal}] has no nodes or slots")
        for slot_ordinal, slot in enumerate(rows):
            if not isinstance(slot, dict) or not isinstance(slot.get("patternId"), str):
                raise CoverageError(
                    f"flow[{flow_ordinal}].{row_label}[{slot_ordinal}] has no patternId"
                )
            pattern_id = slot["patternId"]
            if pattern_id not in product_ids:
                raise CoverageError(
                    f"saved Flow admits non-Product pattern {pattern_id}"
                )
            flow_pattern_ids.add(pattern_id)

    unexpected_flow_skeletons = animationless_skeleton_ids & flow_pattern_ids
    if unexpected_flow_skeletons:
        raise CoverageError(
            "animationless skeleton entered the saved Flow: "
            + ", ".join(sorted(unexpected_flow_skeletons))
        )

    _validate_runtime_inventory_source(root)
    return CoverageReport(
        product_count=len(product_ids),
        encounter_count=len(encounter_ids),
        product_ids=product_ids,
        encounter_ids=encounter_ids,
        flow_pattern_ids=frozenset(flow_pattern_ids),
        animationless_skeleton_ids=animationless_skeleton_ids,
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
    for pattern_id in ANIMATIONLESS_SKELETON_CONTRACTS:
        print(f"  DERIVED_ANIMATIONLESS_SKELETON: {pattern_id}")
    print(f"  SAVED_FLOW_PRODUCT_IDS: {len(report.flow_pattern_ids)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
