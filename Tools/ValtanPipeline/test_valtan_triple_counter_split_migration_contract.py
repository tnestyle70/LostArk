from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/ValtanPipeline"))

import valtan_tuning_pipeline as pipeline  # noqa: E402


PATTERN_ID = "VALTAN_TRIPLE_COUNTER"
SETUP = "valtan.reactive.triple-counter.setup"
FIRST = "valtan.reactive.triple-counter.first"
FIRST_FAIL = "valtan.reactive.triple-counter.first-fail"
SECOND = "valtan.reactive.triple-counter.second"
SECOND_FAIL = "valtan.reactive.triple-counter.second-fail"
THIRD = "valtan.reactive.triple-counter.third"
THIRD_FAIL = "valtan.reactive.triple-counter.third-fail"


def read_json(relative: str) -> dict:
    return json.loads((ROOT / relative).read_text(encoding="utf-8"))


class ValtanTripleCounterSplitMigrationContract(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.gameplay = read_json("Data/Valtan/Valtan.gameplay.json")
        cls.presentation = read_json("Data/Valtan/Valtan.presentation.json")
        cls.sound = read_json(
            "Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json"
        )
        cls.legacy = read_json("Data/Valtan/Valtan.legacy-compatibility.json")
        cls.gameplay_pattern = next(
            row for row in cls.gameplay["patterns"] if row["patternId"] == PATTERN_ID
        )
        cls.presentation_pattern = next(
            row for row in cls.presentation["patterns"] if row["patternId"] == PATTERN_ID
        )
        cls.stages = {
            row["stageId"]: row for row in cls.gameplay_pattern["stages"]
        }

    def test_legacy_and_reference_only_owners_are_removed(self) -> None:
        self.assertNotIn(
            PATTERN_ID,
            {row["patternId"] for row in self.legacy["patternEntries"]},
        )
        reaction_ids = {
            row["reactionLayerId"]
            for row in self.gameplay["counterReactionLayers"]
        }
        self.assertFalse(
            {
                "valtan.reaction-layer.triple-counter.first",
                "valtan.reaction-layer.triple-counter.second",
                "valtan.reaction-layer.triple-counter.third",
            }
            & reaction_ids
        )

    def test_rotation_ownership_stays_core_and_workbench_details_admit_it(self) -> None:
        manual_pattern_ids = {
            row["patternId"]
            for row in self.gameplay["decisionModel"]["manualAuditions"]
        }
        self.assertNotIn(PATTERN_ID, manual_pattern_ids)

        selection_sets = self.gameplay["decisionModel"]["selectionSets"]
        self.assertEqual(
            12,
            sum(len(row["candidates"]) for row in selection_sets),
        )
        self.assertEqual(
            [
                ("selectionset.valtan.160.130", 4),
                ("selectionset.valtan.130.109", 4),
            ],
            [
                (selection_set["selectionSetId"], candidate["weight"])
                for selection_set in selection_sets
                for candidate in selection_set["candidates"]
                if candidate["patternId"] == PATTERN_ID
            ],
        )

        pattern_tree = (ROOT / "Client/Private/ValtanPatternTree.cpp").read_text(
            encoding="utf-8", errors="replace"
        )
        workbench = (
            ROOT / "Client/Private/ActionCompositionWorkbench.cpp"
        ).read_text(encoding="utf-8", errors="replace")
        balance_tool = (ROOT / "Client/Private/BalanceTool.cpp").read_text(
            encoding="utf-8", errors="replace"
        )
        self.assertIn(
            "Staged.CorePatternIds.push_back(pPattern->strPatternId);",
            pattern_tree,
        )
        self.assertIn(
            "if (m_PlayableInventory.Contains(Pattern.strPatternId))",
            workbench,
        )
        # Stage logic and collider tuning intentionally have separate Details
        # tabs while retaining the same typed gameplay owner.
        self.assertIn(
            'OwnerButton("Stage / Logic", DETAIL_OWNER::GAMEPLAY_STAGE,',
            workbench,
        )
        self.assertIn(
            'OwnerButton("Collider", DETAIL_OWNER::GAMEPLAY_STAGE,',
            workbench,
        )
        self.assertIn(
            "draft.colliderTuneAdmitted = !isWaitStage && hasCollider;",
            balance_tool,
        )

    def test_three_counter_windows_and_slam_failure_dag_are_exact(self) -> None:
        self.assertEqual(SETUP, self.gameplay_pattern["entryActionId"])
        self.assertEqual(
            ["SETUP", "COUNTER_1", "FAIL_1", "COUNTER_2", "FAIL_2",
             "COUNTER_3", "FAIL_3"],
            [row["stageId"] for row in self.gameplay_pattern["stages"]],
        )
        setup = self.stages["SETUP"]
        self.assertEqual(("WINDUP", 2000, SETUP, FIRST), (
            setup["stageKind"], setup["durationMs"], setup["actionId"],
            setup["defaultNextActionId"],
        ))
        self.assertEqual(
            [{"outcome": "TIMEOUT", "nextActionId": FIRST}],
            setup["branches"],
        )
        self.assertNotIn("counterProxy", setup)
        self.assertFalse(setup["events"])

        expected_windows = {
            "COUNTER_1": (FIRST, FIRST_FAIL),
            "COUNTER_2": (SECOND, SECOND_FAIL),
            "COUNTER_3": (THIRD, THIRD_FAIL),
        }
        for stage_id, (action_id, timeout) in expected_windows.items():
            with self.subTest(stage_id=stage_id):
                stage = self.stages[stage_id]
                self.assertEqual("WINDUP", stage["stageKind"])
                self.assertEqual(1800, stage["durationMs"])
                self.assertEqual(action_id, stage["actionId"])
                self.assertEqual(timeout, stage["defaultNextActionId"])
                self.assertEqual(
                    [
                        {
                            "outcome": "COUNTER_HIT",
                            "nextActionId": None,
                            "nextPatternId": "VALTAN_GROGGY_FOLLOWUP",
                        },
                        {"outcome": "TIMEOUT", "nextActionId": timeout},
                    ],
                    stage["branches"],
                )
                self.assertEqual(
                    {
                        "kind": "BOSS_FORWARD_ARC",
                        "forwardOffsetM": 0.0,
                        "rightOffsetM": 0.0,
                        "radiusM": 0.0,
                        "arcDegrees": 180.0,
                    },
                    stage["counterProxy"],
                )
                flag_edges = [
                    (event["trigger"], event["enabled"])
                    for event in stage["events"]
                    if event["kind"] == "SET_BOSS_FLAG"
                    and event["flagId"] == "boss.flag.counterable"
                ]
                self.assertEqual([("ENTER", True), ("EXIT", False)], flag_edges)

        expected_failures = {
            "FAIL_1": (FIRST_FAIL, SECOND),
            "FAIL_2": (SECOND_FAIL, THIRD),
            "FAIL_3": (THIRD_FAIL, None),
        }
        for stage_id, (action_id, next_action_id) in expected_failures.items():
            with self.subTest(stage_id=stage_id):
                stage = self.stages[stage_id]
                self.assertEqual("ACTIVE", stage["stageKind"])
                self.assertEqual(1667, stage["durationMs"])
                self.assertEqual(action_id, stage["actionId"])
                self.assertEqual(next_action_id, stage["defaultNextActionId"])
                self.assertEqual(12.0, stage["hit"]["shape"]["outerRadiusM"])
                self.assertEqual(
                    {"kind": "EXPLICIT_OFFSETS", "offsetsMs": [900]},
                    stage["hit"]["schedule"],
                )
                self.assertEqual(
                    "damage.valtan.triple-counter",
                    stage["hit"]["serverDamageProfileId"],
                )
                self.assertEqual(
                    [{"outcome": "TIMEOUT", "nextActionId": next_action_id}],
                    stage["branches"],
                )
                self.assertNotIn("counterProxy", stage)
                self.assertFalse(stage["events"])

    def test_14_01_then_three_exact_14_02_14_03_pairs_are_authored(self) -> None:
        presentation_stages = {
            row["stageId"]: row for row in self.presentation_pattern["stages"]
        }
        self.assertEqual(set(self.stages), set(presentation_stages))
        setup = presentation_stages["SETUP"]["animation"]
        self.assertEqual("EXACT", setup["endPolicy"])
        self.assertEqual(
            [("valtan.reactive.triple-counter.setup.clip.01",
              "mesh_att_battle_14_01", 2000, False)],
            [(row["clipOccurrenceId"], row["clip"], row["playMs"],
              row["repeatUntilStageEnd"]) for row in setup["occurrences"]],
        )
        for root, counter_stage_id, fail_stage_id in (
            ("first", "COUNTER_1", "FAIL_1"),
            ("second", "COUNTER_2", "FAIL_2"),
            ("third", "COUNTER_3", "FAIL_3"),
        ):
            with self.subTest(counter_stage_id=counter_stage_id):
                counter_animation = presentation_stages[counter_stage_id]["animation"]
                self.assertEqual("EXACT", counter_animation["endPolicy"])
                self.assertEqual(
                    [
                        (f"valtan.reactive.triple-counter.{root}.clip.01",
                         "mesh_att_battle_14_02", 1000, False),
                        (f"valtan.reactive.triple-counter.{root}.clip.02",
                         "mesh_att_battle_14_02", 800, False),
                    ],
                    [(row["clipOccurrenceId"], row["clip"], row["playMs"],
                      row["repeatUntilStageEnd"])
                     for row in counter_animation["occurrences"]],
                )
                fail_animation = presentation_stages[fail_stage_id]["animation"]
                self.assertEqual("EXACT", fail_animation["endPolicy"])
                self.assertEqual(
                    [(f"valtan.reactive.triple-counter.{root}-fail.clip.01",
                      "mesh_att_battle_14_03", 1667, False)],
                    [(row["clipOccurrenceId"], row["clip"], row["playMs"],
                      row["repeatUntilStageEnd"])
                     for row in fail_animation["occurrences"]],
                )
        self.assertTrue(all(
            not row["effectCues"] for row in presentation_stages.values()
        ))

    def test_pattern_sound_dependencies_follow_the_exact_occurrences(self) -> None:
        cues = [
            row for row in self.sound["cues"]
            if row["patternId"] == PATTERN_ID
        ]
        self.assertFalse(any(row["stageId"] == "RECOVERY" for row in cues))
        cue_occurrences = {row["clipOccurrenceId"] for row in cues}
        self.assertTrue({
            "valtan.reactive.triple-counter.setup.clip.01",
            "valtan.reactive.triple-counter.first.clip.01",
            "valtan.reactive.triple-counter.first.clip.02",
            "valtan.reactive.triple-counter.first-fail.clip.01",
            "valtan.reactive.triple-counter.second.clip.01",
            "valtan.reactive.triple-counter.second.clip.02",
            "valtan.reactive.triple-counter.second-fail.clip.01",
            "valtan.reactive.triple-counter.third.clip.01",
            "valtan.reactive.triple-counter.third.clip.02",
            "valtan.reactive.triple-counter.third-fail.clip.01",
        }.issubset(cue_occurrences))
        bindings = {
            row["actionId"]: row
            for row in read_json(
                "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
            )["bindings"]
        }
        clip_windows = {
            clip["clipOccurrenceId"]: clip["playMs"]
            for action_id in {
                SETUP, FIRST, FIRST_FAIL, SECOND, SECOND_FAIL, THIRD, THIRD_FAIL
            }
            for clip in bindings[action_id]["clips"]
        }
        self.assertTrue(all(
            row["clipOccurrenceId"] in clip_windows
            and (
                clip_windows[row["clipOccurrenceId"]] == 0
                or row["startMs"] < clip_windows[row["clipOccurrenceId"]]
            )
            for row in cues
        ))

    def test_split_join_and_native_catalog_source_admit_forward_counter_targets(self) -> None:
        docs = pipeline.load_pipeline_documents(ROOT)
        joined = pipeline.join_v2_authoring(
            docs[pipeline.GAMEPLAY_AUTHORING_REL],
            docs[pipeline.PRESENTATION_AUTHORING_REL],
            docs[pipeline.WORLD_SET_REL],
            docs[pipeline.COMBAT_AUTHORING_REL],
        )
        pipeline.validate_v2_master(
            joined,
            docs[pipeline.WORLD_SET_REL],
            docs[pipeline.COMBAT_AUTHORING_REL],
        )
        source = (ROOT / "Server/Private/GameplayCatalog.cpp").read_text(
            encoding="utf-8", errors="replace"
        )
        for token in (
            "counterTargetIsForward",
            "timeoutTargetIsForward",
            "counterTargetKindIsSupported",
            "BOSS_PATTERN_STAGE_KIND::WINDUP",
            "BOSS_PATTERN_STAGE_KIND::GROGGY",
            "BOSS_PATTERN_STAGE_KIND::RECOVERY",
            "!counterTargetIsGroggy ||",
        ):
            self.assertIn(token, source)


if __name__ == "__main__":
    unittest.main()
