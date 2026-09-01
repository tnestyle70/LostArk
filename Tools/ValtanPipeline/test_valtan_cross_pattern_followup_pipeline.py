#!/usr/bin/env python3
"""Focused contracts for Valtan stage-result cross-pattern follow-ups."""

from __future__ import annotations

import copy
import pathlib
import sys
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import valtan_tuning_pipeline as pipeline


ROOT = pathlib.Path(__file__).resolve().parents[2]
PUBLISHER = ROOT / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1"


def pattern(master: dict, pattern_id: str) -> dict:
    return next(row for row in master["patterns"] if row["patternId"] == pattern_id)


def stage(owner: dict, stage_id: str) -> dict:
    return next(row for row in owner["stages"] if row["stageId"] == stage_id)


class ValtanCrossPatternFollowupPipelineTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.docs = pipeline.load_pipeline_documents(ROOT)
        cls.master = pipeline.join_v2_authoring(
            cls.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            cls.docs[pipeline.PRESENTATION_AUTHORING_REL],
            cls.docs[pipeline.WORLD_SET_REL],
            cls.docs[pipeline.COMBAT_AUTHORING_REL],
        )

    def validate(self, candidate: dict) -> None:
        pipeline.validate_v2_master(
            candidate,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )

    def test_repository_followups_survive_split_join_and_product_projection(self) -> None:
        dash = pattern(self.master, "VALTAN_DASH_CHARGE")
        charge = stage(dash, "CHARGE")
        self.assertEqual(
            [
                {
                    "outcome": "WALL_CONTACT",
                    "nextActionId": None,
                    "nextPatternId": "VALTAN_DASH_CHARGE_GROGGY",
                },
                {
                    "outcome": "TIMEOUT",
                    "nextActionId": None,
                    "nextPatternId": "VALTAN_DASH_CHARGE_GROGGY",
                },
            ],
            charge["branches"],
        )
        groggy = pattern(self.master, "VALTAN_DASH_CHARGE_GROGGY")
        self.assertEqual(
            {
                "outcome": "PART_DESTROYED",
                "nextActionId": None,
                "nextPatternId": "VALTAN_PART_BREAK",
            },
            stage(groggy, "GROGGY")["branches"][0],
        )

        gameplay, presentation = pipeline.split_v2_authoring(
            copy.deepcopy(self.master),
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        rejoined = pipeline.join_v2_authoring(
            gameplay,
            presentation,
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        projected = pipeline.compile_pattern_product(
            rejoined, pattern(rejoined, "VALTAN_DASH_CHARGE")
        )
        self.assertEqual(
            charge["branches"], stage(projected, "CHARGE")["branches"]
        )

    def test_followup_rejects_conflicting_missing_or_selectable_targets(self) -> None:
        invalid = copy.deepcopy(self.master)
        counter = stage(pattern(invalid, "VALTAN_COUNTER"), "STEP_02")
        counter_branch = next(
            row for row in counter["branches"] if row["outcome"] == "COUNTER_HIT"
        )
        counter_branch["nextActionId"] = "valtan.sequence.counter.step-03"
        with self.assertRaisesRegex(pipeline.PipelineError, "must choose"):
            self.validate(invalid)

        invalid = copy.deepcopy(self.master)
        branch = stage(pattern(invalid, "VALTAN_DASH_CHARGE"), "CHARGE")[
            "branches"
        ][0]
        branch["nextPatternId"] = "VALTAN_MISSING_FOLLOWUP"
        with self.assertRaisesRegex(pipeline.PipelineError, "target is missing"):
            self.validate(invalid)

        invalid = copy.deepcopy(self.master)
        branch = stage(pattern(invalid, "VALTAN_DASH_CHARGE"), "CHARGE")[
            "branches"
        ][0]
        branch["nextPatternId"] = "VALTAN_WHIRLWIND"
        with self.assertRaisesRegex(pipeline.PipelineError, "AUDITION_ONLY"):
            self.validate(invalid)

    def test_followup_target_must_be_zero_bar_and_untargeted(self) -> None:
        invalid = copy.deepcopy(self.master)
        target = pattern(invalid, "VALTAN_DASH_CHARGE_GROGGY")
        target["eligibility"]["minimumHealthBarInclusive"] = 1
        with self.assertRaisesRegex(pipeline.PipelineError, "zero selection/health"):
            self.validate(invalid)

        invalid = copy.deepcopy(self.master)
        target = pattern(invalid, "VALTAN_DASH_CHARGE_GROGGY")
        target["targetPolicy"] = "LOCK_NEAREST_ON_START"
        target["aimPolicy"] = "LOCK_FACING_ON_START"
        with self.assertRaisesRegex(pipeline.PipelineError, "untargeted"):
            self.validate(invalid)

    def append_followup_chain(self, master, edge_count: int) -> None:
        template = copy.deepcopy(pattern(master, "VALTAN_PART_BREAK"))
        root = copy.deepcopy(template)
        root["patternId"] = "VALTAN_FOLLOWUP_DEPTH_ROOT"
        root["displayName"] = "Follow-up depth root"
        root["actionId"] = "valtan.reaction.followup-depth-root"
        root["entryActionId"] = root["actionId"] + ".active"
        root_stage = root["stages"][0]
        root_stage["actionId"] = root["entryActionId"]
        root_stage["animation"]["occurrences"][0][
            "clipOccurrenceId"
        ] = root["actionId"] + ".clip.01"
        root_stage["branches"] = [
            {"outcome": "TIMEOUT", "nextActionId": None}
        ]
        master["patterns"].append(root)
        master["decisionModel"]["manualAuditions"].append(
            {
                "patternId": root["patternId"],
                "sourceChainId": "derived.followup-depth-root",
                "authoringPhase": 1,
                "admissionState": pipeline.DERIVED_SERVER_PATTERN,
            }
        )
        previous = root_stage
        for ordinal in range(edge_count):
            pattern_id = f"VALTAN_FOLLOWUP_DEPTH_{ordinal:02d}"
            action_root = f"valtan.reaction.followup-depth-{ordinal:02d}"
            owner = copy.deepcopy(template)
            owner["patternId"] = pattern_id
            owner["displayName"] = f"Follow-up depth {ordinal}"
            owner["actionId"] = action_root
            owner["entryActionId"] = action_root + ".active"
            owner_stage = owner["stages"][0]
            owner_stage["actionId"] = owner["entryActionId"]
            owner_stage["animation"]["occurrences"][0][
                "clipOccurrenceId"
            ] = action_root + ".clip.01"
            owner_stage["branches"] = [
                {"outcome": "TIMEOUT", "nextActionId": None}
            ]
            master["patterns"].append(owner)
            master["decisionModel"]["manualAuditions"].append(
                {
                    "patternId": pattern_id,
                    "sourceChainId": f"derived.followup-depth-{ordinal:02d}",
                    "authoringPhase": 1,
                    "admissionState": pipeline.DERIVED_SERVER_PATTERN,
                }
            )
            previous["branches"][0]["nextPatternId"] = pattern_id
            previous = owner_stage

    def test_followup_graph_rejects_cycles_and_excessive_depth(self) -> None:
        invalid = copy.deepcopy(self.master)
        terminal = stage(pattern(invalid, "VALTAN_PART_BREAK"), "PART_BREAK")
        terminal["branches"][0]["nextPatternId"] = (
            "VALTAN_DASH_CHARGE_GROGGY"
        )
        with self.assertRaisesRegex(pipeline.PipelineError, "contains a cycle"):
            self.validate(invalid)

        invalid = copy.deepcopy(self.master)
        self.append_followup_chain(
            invalid, pipeline.PATTERN_FOLLOWUP_MAX_DEPTH + 1
        )
        with self.assertRaisesRegex(pipeline.PipelineError, "maximum depth"):
            self.validate(invalid)

    def test_followup_graph_accepts_native_boundary_depth(self) -> None:
        boundary = copy.deepcopy(self.master)
        self.append_followup_chain(
            boundary, pipeline.PATTERN_FOLLOWUP_MAX_DEPTH
        )
        self.validate(boundary)

    def test_publisher_uses_dedicated_followup_row_and_v29(self) -> None:
        source = PUBLISHER.read_text(encoding="utf-8")
        self.assertIn("'PATTERNSTAGEFOLLOWUP'", source)
        self.assertIn("$gameplayBootstrapVersion = if ($rotationFormatVersion -eq 4) { 29 }", source)
        self.assertIn("follow-up target must be an untargeted AUDITION_ONLY pattern", source)
        self.assertIn("Pattern follow-up graph exceeds maximum depth", source)
        self.assertEqual(29, pipeline.GAMEPLAY_BOOTSTRAP_VERSION)


if __name__ == "__main__":
    unittest.main()
