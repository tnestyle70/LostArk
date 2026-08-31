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


EXPECTED_PRODUCT_COUNT = 33
EXPECTED_ENCOUNTER_COUNT = 57

REQUESTED_PRODUCT_IDS = (
    "VALTAN_HIGH_JUMP",
    "VALTAN_SIX_PIZZA_106",
    "VALTAN_WARP",
    "VALTAN_TRASH",
    "VALTAN_CATCH_BREATH",
    "VALTAN_STRUGGLING",
    "VALTAN_DASH_CHARGE",
)

REQUESTED_REFERENCE_ONLY_IDS = (
    "VALTAN_MAGIC_ORB_STAGGER_76",
    "VALTAN_TRIPLE_COUNTER",
    "VALTAN_FOUR_PILLARS_105",
)

# The requests describe these concepts, but no stable Pattern identity with
# either concept exists in Product or Encounter data yet.  Existing roar and
# pillar rows must not be silently relabelled as these new gameplay contracts.
REQUESTED_ABSENT_CONCEPTS = {
    "silence": ("VALTAN_SILENCE", "SILENCE"),
    "stone_creation_roar": ("VALTAN_STONE_CREATION_ROAR", "STONE"),
}


class CoverageError(RuntimeError):
    """Raised when Product/reference/runtime admission no longer exact-joins."""


@dataclass(frozen=True)
class CoverageReport:
    product_count: int
    encounter_count: int
    product_ids: frozenset[str]
    encounter_ids: frozenset[str]
    flow_pattern_ids: frozenset[str]


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

    gameplay_by_id = _index_patterns(gameplay, "Valtan.gameplay")
    presentation_by_id = _index_patterns(presentation, "Valtan.presentation")
    encounter_by_id = _index_patterns(encounter, "ValtanEncounter")
    product_ids = frozenset(gameplay_by_id)
    encounter_ids = frozenset(encounter_by_id)

    if len(product_ids) != EXPECTED_PRODUCT_COUNT:
        raise CoverageError(
            f"split Product count is {len(product_ids)}, expected {EXPECTED_PRODUCT_COUNT}"
        )
    if len(encounter_ids) != EXPECTED_ENCOUNTER_COUNT:
        raise CoverageError(
            f"Encounter/reference count is {len(encounter_ids)}, "
            f"expected {EXPECTED_ENCOUNTER_COUNT}"
        )
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

    all_ids = product_ids | encounter_ids
    for concept, (proposed_id, forbidden_token) in REQUESTED_ABSENT_CONCEPTS.items():
        if proposed_id in all_ids or any(forbidden_token in value for value in all_ids):
            raise CoverageError(
                f"requested absent concept {concept} unexpectedly owns a stable pattern identity"
            )

    flows = flow.get("flows")
    if not isinstance(flows, list) or not flows:
        raise CoverageError("ValtanBossAuditionFlows.flows must be a non-empty array")
    flow_pattern_ids: set[str] = set()
    for flow_ordinal, flow_row in enumerate(flows):
        if not isinstance(flow_row, dict) or not isinstance(flow_row.get("slots"), list):
            raise CoverageError(f"flow[{flow_ordinal}] has no slots")
        for slot_ordinal, slot in enumerate(flow_row["slots"]):
            if not isinstance(slot, dict) or not isinstance(slot.get("patternId"), str):
                raise CoverageError(
                    f"flow[{flow_ordinal}].slots[{slot_ordinal}] has no patternId"
                )
            pattern_id = slot["patternId"]
            if pattern_id not in product_ids:
                raise CoverageError(
                    f"saved Flow admits non-Product pattern {pattern_id}"
                )
            flow_pattern_ids.add(pattern_id)

    _validate_runtime_inventory_source(root)
    return CoverageReport(
        product_count=len(product_ids),
        encounter_count=len(encounter_ids),
        product_ids=product_ids,
        encounter_ids=encounter_ids,
        flow_pattern_ids=frozenset(flow_pattern_ids),
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
    for concept, (proposed_id, _) in REQUESTED_ABSENT_CONCEPTS.items():
        print(f"  NO_STABLE_PATTERN_YET: {concept} ({proposed_id})")
    print(f"  SAVED_FLOW_PRODUCT_IDS: {len(report.flow_pattern_ids)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
