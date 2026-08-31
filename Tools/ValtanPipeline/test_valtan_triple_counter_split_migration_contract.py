from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/ValtanPipeline"))

import valtan_tuning_pipeline as pipeline  # noqa: E402


PATTERN_ID = "VALTAN_TRIPLE_COUNTER"
FIRST = "valtan.reactive.triple-counter.first"
FIRST_FAIL = "valtan.reactive.triple-counter.first-fail"
SECOND = "valtan.reactive.triple-counter.second"
SECOND_FAIL = "valtan.reactive.triple-counter.second-fail"
THIRD = "valtan.reactive.triple-counter.third"
THIRD_FAIL = "valtan.reactive.triple-counter.third-fail"
RECOVERY = "valtan.reactive.triple-counter.recovery"


def read_json(relative: str) -> dict:
    return json.loads((ROOT / relative).read_text(encoding="utf-8"))


class ValtanTripleCounterSplitMigrationContract(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.gameplay = read_json("Data/Valtan/Valtan.gameplay.json")
        cls.presentation = read_json("Data/Valtan/Valtan.presentation.json")
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

    def test_three_counter_windows_and_failure_dag_are_exact(self) -> None:
        expected = {
            "COUNTER_1": (1800, FIRST, SECOND, FIRST_FAIL),
            "COUNTER_2": (1600, SECOND, THIRD, SECOND_FAIL),
            "COUNTER_3": (1400, THIRD, RECOVERY, THIRD_FAIL),
        }
        for stage_id, (duration_ms, action_id, success, timeout) in expected.items():
            with self.subTest(stage_id=stage_id):
                stage = self.stages[stage_id]
                self.assertEqual("WINDUP", stage["stageKind"])
                self.assertEqual(duration_ms, stage["durationMs"])
                self.assertEqual(action_id, stage["actionId"])
                self.assertEqual(timeout, stage["defaultNextActionId"])
                self.assertEqual(
                    [
                        {"outcome": "COUNTER_HIT", "nextActionId": success},
                        {"outcome": "TIMEOUT", "nextActionId": timeout},
                    ],
                    stage["branches"],
                )
                self.assertEqual(
                    {
                        "space": "BOSS_LOCAL",
                        "forwardOffsetM": 1.0,
                        "rightOffsetM": 0.0,
                        "radiusM": 2.25,
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

        for stage_id, radius_m, damage, target in (
            ("FAIL_1", 18.0, "damage.valtan.triple-counter", SECOND),
            ("FAIL_2", 18.0, "damage.valtan.triple-counter", THIRD),
            ("FAIL_3", 100.0, "damage.valtan.omnidirectional-wipe-130", RECOVERY),
        ):
            with self.subTest(stage_id=stage_id):
                stage = self.stages[stage_id]
                self.assertEqual(radius_m, stage["hit"]["shape"]["outerRadiusM"])
                self.assertEqual(damage, stage["hit"]["serverDamageProfileId"])
                self.assertEqual(target, stage["defaultNextActionId"])
                self.assertEqual(
                    [{"outcome": "TIMEOUT", "nextActionId": target}],
                    stage["branches"],
                )
        self.assertEqual(1200, self.stages["RECOVERY"]["durationMs"])
        self.assertIsNone(self.stages["RECOVERY"]["defaultNextActionId"])
        self.assertEqual(
            [{"outcome": "TIMEOUT", "nextActionId": None}],
            self.stages["RECOVERY"]["branches"],
        )

    def test_legacy_occurrences_and_single_carrier_cue_are_preserved(self) -> None:
        expected = [
            ("COUNTER_1", FIRST, "mesh_abn_groggy_1_start"),
            ("FAIL_1", FIRST_FAIL, "mesh_abn_groggy_1_loop"),
            ("COUNTER_2", SECOND, "mesh_abn_groggy_1_loop"),
            ("FAIL_2", SECOND_FAIL, "mesh_abn_groggy_1_loop"),
            ("COUNTER_3", THIRD, "mesh_abn_groggy_1_loop"),
            ("FAIL_3", THIRD_FAIL, "mesh_abn_groggy_1_loop"),
            ("RECOVERY", RECOVERY, "mesh_abn_groggy_1_end"),
        ]
        presentation_stages = {
            row["stageId"]: row for row in self.presentation_pattern["stages"]
        }
        for stage_id, action_id, clip in expected:
            occurrence = presentation_stages[stage_id]["animation"]["occurrences"][0]
            self.assertEqual(f"{action_id}.clip.01", occurrence["clipOccurrenceId"])
            self.assertEqual(clip, occurrence["clip"])
            self.assertEqual("CURRENT_PRODUCT_BASELINE", occurrence["mappingBasis"])
            self.assertTrue(occurrence["repeatUntilStageEnd"])
        cues = presentation_stages["COUNTER_1"]["effectCues"]
        self.assertEqual(1, len(cues))
        self.assertEqual(
            "cue.valtan.carrier-v1.reactive.triple-counter.first.clip-01",
            cues[0]["cueId"],
        )
        self.assertEqual("OWNER_RELATIVE", cues[0]["scalePolicy"]["kind"])

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
