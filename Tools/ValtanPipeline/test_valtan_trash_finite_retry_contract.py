#!/usr/bin/env python3
"""Focused source contract for Trash retry loops and finite audition fragments."""

from __future__ import annotations

import copy
import json
import sys
import unittest
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
PIPELINE_ROOT = ROOT / "Tools" / "ValtanPipeline"
sys.path.insert(0, str(PIPELINE_ROOT))

import author_valtan_phase_two_mechanics as author  # noqa: E402
import valtan_tuning_pipeline as tuning  # noqa: E402


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


GAMEPLAY = load_json(ROOT / "Data" / "Valtan" / "Valtan.gameplay.json")
PRESENTATION = load_json(
    ROOT / "Data" / "Valtan" / "Valtan.presentation.json"
)


def pattern(document: dict[str, Any], pattern_id: str) -> dict[str, Any]:
    matches = [
        row for row in document["patterns"] if row["patternId"] == pattern_id
    ]
    if len(matches) != 1:
        raise AssertionError(f"expected one {pattern_id}, found {len(matches)}")
    return matches[0]


def stage(pattern_row: dict[str, Any], stage_id: str) -> dict[str, Any]:
    matches = [row for row in pattern_row["stages"] if row["stageId"] == stage_id]
    if len(matches) != 1:
        raise AssertionError(
            f"expected one {pattern_row['patternId']}/{stage_id}, found {len(matches)}"
        )
    return matches[0]


class ValtanTrashFiniteRetryContractTests(unittest.TestCase):
    PATTERN_IDS = ("VALTAN_TRASH", "VALTAN_TRASH_CATCH_IF")

    def test_authoring_recipe_preserves_retry_and_counter_contract(self) -> None:
        authored_gameplay, _ = author.build("VALTAN_TRASH")
        # Saved timing and effect cues are user-owned. The recipe's generated
        # defaults need not overwrite them to preserve its retry graph contract.
        fields = ("stageId", "actionId", "stageKind", "defaultNextActionId",
                  "branches", "hit", "events", "counterProxy")
        for pattern_id in (*self.PATTERN_IDS, "VALTAN_TRASH_CATCH_SUCCESS",
                           "VALTAN_TRASH_CATCH_FAIL"):
            saved = pattern(GAMEPLAY, pattern_id)
            authored = pattern(authored_gameplay, pattern_id)
            self.assertEqual(saved["entryActionId"], authored["entryActionId"])
            self.assertEqual(
                [{key: row.get(key) for key in fields} for row in saved["stages"]],
                [{key: row.get(key) for key in fields} for row in authored["stages"]],
            )

    def test_only_three_exact_retry_backedges_are_allowed(self) -> None:
        pattern_by_id = {row["patternId"]: row for row in GAMEPLAY["patterns"]}
        retry_sources = {"RETRY_EXHAUSTED", "CATCH_SLAM", "EXECUTE_TAIL"}
        for pattern_id in self.PATTERN_IDS:
            gameplay = pattern(GAMEPLAY, pattern_id)
            tuning._validate_finite_pattern_graph(gameplay)
            tuning._validate_pattern_counter_groggy_contract(
                gameplay, pattern_by_id, f"pattern {pattern_id}"
            )
            positions = {row["actionId"]: index
                         for index, row in enumerate(gameplay["stages"])}
            retry_target = stage(gameplay, "RECHARGE_WAIT_02")["actionId"]
            found = set()
            for source_index, row in enumerate(gameplay["stages"]):
                targets = [row["defaultNextActionId"]] + [
                    branch["nextActionId"] for branch in row["branches"]
                ]
                for target in targets:
                    if target is None or positions[target] > source_index:
                        continue
                    self.assertIn(row["stageId"], retry_sources)
                    self.assertEqual(target, retry_target)
                    self.assertEqual(row["defaultNextActionId"], retry_target)
                    self.assertEqual(row["branches"], [
                        {"outcome": "TIMEOUT", "nextActionId": retry_target}
                    ])
                    found.add(row["stageId"])
            self.assertEqual(found, retry_sources)

    def test_unrelated_cycles_and_finite_fragment_cycles_are_rejected(self) -> None:
        for pattern_id in (*self.PATTERN_IDS, "VALTAN_TRASH_CATCH_SUCCESS",
                           "VALTAN_TRASH_CATCH_FAIL"):
            gameplay = copy.deepcopy(pattern(GAMEPLAY, pattern_id))
            tuning._validate_finite_pattern_graph(gameplay)
            first = gameplay["stages"][0]
            first["defaultNextActionId"] = first["actionId"]
            with self.assertRaises(tuning.PipelineError):
                tuning._validate_finite_pattern_graph(gameplay)
        for source_id in ("RETRY_EXHAUSTED", "CATCH_SLAM", "EXECUTE_TAIL"):
            for pattern_id in self.PATTERN_IDS:
                gameplay = copy.deepcopy(pattern(GAMEPLAY, pattern_id))
                row = stage(gameplay, source_id)
                target = stage(gameplay, "RETRY_WINDUP_02")["actionId"]
                row["defaultNextActionId"] = target
                row["branches"] = [{"outcome": "TIMEOUT", "nextActionId": target}]
                with self.assertRaises(tuning.PipelineError):
                    tuning._validate_finite_pattern_graph(gameplay)

    def test_no_capture_partial_capture_and_all_capture_retry_until_counter(self) -> None:
        # Traverse the actual saved outcome edges. Native lifecycle separately
        # proves that confirmed combat hits publish COUNTER_HIT at runtime.
        for pattern_id in self.PATTERN_IDS:
            gameplay = pattern(GAMEPLAY, pattern_id)
            by_action = {row["actionId"]: row for row in gameplay["stages"]}
            for capture in ("NONE", "PARTIAL", "ALL"):
                with self.subTest(pattern=pattern_id, capture=capture):
                    action = gameplay["entryActionId"]
                    retry_count = counter_count = 0
                    visited = []
                    for _ in range(128):
                        self.assertIsNotNone(action)
                        row = by_action[action]
                        visited.append(row["stageId"])
                        branches = {b["outcome"]: b["nextActionId"]
                                    for b in row["branches"]}
                        if row["stageId"] == "RECHARGE_WAIT_02":
                            retry_count += 1
                        outcome = "TIMEOUT"
                        if "COUNTER_HIT" in branches and retry_count >= 3:
                            outcome = "COUNTER_HIT"
                            counter_count += 1
                        elif capture != "NONE" and "ANY_PLAYER_GRABBED" in branches:
                            outcome = "ANY_PLAYER_GRABBED"
                        elif capture == "ALL" and "ALL_PLAYERS_GRABBED" in branches:
                            outcome = "ALL_PLAYERS_GRABBED"
                        action = branches.get(outcome, row["defaultNextActionId"])
                        if action is None:
                            break
                    self.assertIsNone(action, "counter must reach the finite GROGGY terminal")
                    self.assertEqual(counter_count, 1)
                    self.assertGreaterEqual(retry_count, 3)
                    self.assertEqual(visited[-1], "GROGGY")
                    expected_tail = {"NONE": "RETRY_EXHAUSTED", "PARTIAL": "CATCH_SLAM",
                                     "ALL": "EXECUTE_TAIL"}[capture]
                    self.assertGreaterEqual(visited.count(expected_tail), 2)

    def test_each_attempt_owns_one_counter_window_and_capture_rush(self) -> None:
        for pattern_id in self.PATTERN_IDS:
            gameplay = pattern(GAMEPLAY, pattern_id)
            action_root = gameplay["actionId"]
            attempts = (
                ("STEP_07", "STEP_08", "RUSH_MISS"),
                ("RETRY_WINDUP_02", "RETRY_RUSH_02", "RETRY_MISS_02"),
                ("RETRY_WINDUP_03", "RETRY_RUSH_03", "RETRY_EXHAUSTED"),
            )
            for windup_id, rush_id, miss_id in attempts:
                windup = stage(gameplay, windup_id)
                rush = stage(gameplay, rush_id)
                self.assertEqual(windup["stageKind"], "WINDUP")
                self.assertEqual(windup["durationMs"], 1000)
                self.assertEqual(
                    windup["counterProxy"],
                    {
                        "space": "BOSS_LOCAL",
                        "forwardOffsetM": 1.0,
                        "rightOffsetM": 0.0,
                        "radiusM": 2.25,
                    },
                )
                flags = [
                    event
                    for event in windup["events"]
                    if event.get("kind") == "SET_BOSS_FLAG"
                    and event.get("flagId") == "boss.flag.counterable"
                ]
                self.assertEqual(
                    {(event["trigger"], event["enabled"]) for event in flags},
                    {("ENTER", True), ("EXIT", False)},
                )
                branches = {
                    branch["outcome"]: branch["nextActionId"]
                    for branch in windup["branches"]
                }
                self.assertEqual(
                    branches,
                    {
                        "COUNTER_HIT": f"{action_root}.groggy",
                        "TIMEOUT": rush["actionId"],
                    },
                )

                hit = rush["hit"]
                self.assertEqual(hit["shape"], {"kind": "BOX", "lengthM": 6.0, "halfWidthM": 2.5})
                self.assertEqual(hit["playerResponse"], "CAPTURE")
                self.assertEqual(hit["attachmentSlot"], "BOSS_LEFT_HAND")
                rush_branches = {
                    branch["outcome"]: branch["nextActionId"]
                    for branch in rush["branches"]
                }
                self.assertEqual(
                    rush_branches,
                    {
                        "ANY_PLAYER_GRABBED": f"{action_root}.catch-counter",
                        "NAVIGATION_BLOCKED": stage(gameplay, miss_id)["actionId"],
                        "TIMEOUT": stage(gameplay, miss_id)["actionId"],
                    },
                )

    def test_misses_advance_then_repeat_the_counter_opportunity(self) -> None:
        for pattern_id in self.PATTERN_IDS:
            gameplay = pattern(GAMEPLAY, pattern_id)
            expected = (
                ("RUSH_MISS", "RECHARGE_WAIT_02"),
                ("RETRY_MISS_02", "RECHARGE_WAIT_03"),
                ("RETRY_EXHAUSTED", "RECHARGE_WAIT_02"),
            )
            for miss_id, next_id in expected:
                miss = stage(gameplay, miss_id)
                target = None if next_id is None else stage(gameplay, next_id)["actionId"]
                self.assertEqual(miss["defaultNextActionId"], target)
                self.assertEqual(
                    miss["branches"],
                    [{"outcome": "TIMEOUT", "nextActionId": target}],
                )

    def test_presentation_is_losslessly_joined_to_all_new_stages(self) -> None:
        expected_clips = {
            "RUSH_MISS": ("mesh_att_battle_13_05-2", 1000),
            "RECHARGE_WAIT_02": ("mesh_att_battle_13_02-1", 4100),
            "RETRY_WINDUP_02": ("mesh_att_battle_13_03", 1000),
            "RETRY_RUSH_02": ("mesh_att_battle_13_04", 667),
            "RETRY_MISS_02": ("mesh_att_battle_13_05-2", 1000),
            "RECHARGE_WAIT_03": ("mesh_att_battle_13_02-1", 4100),
            "RETRY_WINDUP_03": ("mesh_att_battle_13_03", 1000),
            "RETRY_RUSH_03": ("mesh_att_battle_13_04", 667),
            "RETRY_EXHAUSTED": ("mesh_att_battle_13_05-2", 1000),
        }
        for pattern_id in self.PATTERN_IDS:
            gameplay = pattern(GAMEPLAY, pattern_id)
            presentation = pattern(PRESENTATION, pattern_id)
            self.assertEqual(
                [row["stageId"] for row in gameplay["stages"]],
                [row["stageId"] for row in presentation["stages"]],
            )
            self.assertEqual(
                [row["actionId"] for row in gameplay["stages"]],
                [row["actionId"] for row in presentation["stages"]],
            )
            occurrence_ids: list[str] = []
            for stage_id, (clip, play_ms) in expected_clips.items():
                gameplay_stage = stage(gameplay, stage_id)
                presentation_stage = stage(presentation, stage_id)
                occurrences = presentation_stage["animation"]["occurrences"]
                self.assertEqual(len(occurrences), 1)
                self.assertEqual(occurrences[0]["clip"], clip)
                self.assertEqual(occurrences[0]["playMs"], play_ms)
                self.assertEqual(gameplay_stage["durationMs"], play_ms)
            for presentation_stage in presentation["stages"]:
                occurrence_ids.extend(
                    occurrence["clipOccurrenceId"]
                    for occurrence in presentation_stage["animation"]["occurrences"]
                )
            self.assertEqual(len(occurrence_ids), len(set(occurrence_ids)))

    def test_server_branch_consumer_is_pattern_agnostic(self) -> None:
        source = (ROOT / "Server" / "Private" / "ValtanBrain.cpp").read_text(
            encoding="utf-8"
        )
        start = source.index("bool ApplyStageBranch(")
        end = source.index("bool ApplyPublishedOutcomeBranch(", start)
        apply_stage_branch = source[start:end]
        self.assertIn(
            "FindStageByActionId(pattern, branch.strNextActionId, nextStageIndex)",
            apply_stage_branch,
        )
        self.assertIn(
            "EnterPatternStage(boss, *next, nextStageIndex, serverTick)",
            apply_stage_branch,
        )
        self.assertNotIn("VALTAN_TRASH", apply_stage_branch)


if __name__ == "__main__":
    unittest.main()
