#!/usr/bin/env python3
from __future__ import annotations

import copy
import json
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import valtan_tuning_pipeline as pipeline


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
BASELINE_MOTION = {
    "kind": "PORTAL_TARGET_RUSH",
    "retargetDelayMs": 500,
    "speedMps": 20.0,
    "distanceM": 8.0,
}
BASELINE_OFFSETS = list(range(500, 900, 50))


class ValtanPortalRushTuningContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.root = REPOSITORY_ROOT.resolve()
        cls.docs = pipeline.load_pipeline_documents(cls.root)
        cls.source_manifest = pipeline.source_manifest(cls.root)

    def joined_master(self) -> dict:
        return pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )

    @staticmethod
    def warp_legs(document: dict) -> list[dict]:
        warp = next(
            row for row in document["patterns"] if row["patternId"] == "VALTAN_WARP"
        )
        return warp["stages"][1:9]

    def apply(self, operations: list[dict]) -> dict:
        candidate, _, _, _ = pipeline.apply_draft_patch(
            self.joined_master(),
            self.docs[pipeline.BOSS_PROFILES_REL],
            self.docs[pipeline.DAMAGE_REL],
            {
                "schema": pipeline.DRAFT_PATCH_SCHEMA,
                "formatVersion": 1,
                "sourceRevision": self.source_manifest["sourceManifestId"],
                "operations": operations,
            },
            self.source_manifest["sourceManifestId"],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        return candidate

    def test_baseline_projects_losslessly_and_wait_has_no_hit(self) -> None:
        gameplay = self.docs[pipeline.GAMEPLAY_AUTHORING_REL]
        master = self.joined_master()
        product_outputs = pipeline.project_v2_products(self.root, self.docs, master)
        product = json.loads(product_outputs[pipeline.ENCOUNTER_REL])

        for owner in (gameplay, master, product):
            with self.subTest(owner=owner.get("schema")):
                legs = self.warp_legs(owner)
                self.assertEqual(8, len(legs))
                for stage in legs:
                    self.assertEqual(BASELINE_MOTION, stage["motion"])
                    if "hit" in stage:
                        schedule = stage["hit"]["schedule"]
                        self.assertEqual("EXPLICIT_OFFSETS", schedule["kind"])
                        offsets = schedule["offsetsMs"]
                    else:
                        offsets = stage["hitOffsetsMs"]
                        self.assertEqual(len(offsets), stage["hitCount"])
                    self.assertEqual(BASELINE_OFFSETS, offsets)
                    self.assertGreaterEqual(
                        min(offsets), stage["motion"]["retargetDelayMs"]
                    )

    def test_existing_portal_cue_owners_are_listed_without_synthetic_edges(self) -> None:
        authored = pipeline.read_json(
            self.root
            / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
        )
        warp_cues = [row for row in authored["cues"] if row["patternId"] == "VALTAN_WARP"]
        self.assertEqual(8, len(warp_cues))
        self.assertEqual(
            {f"STEP_{index:02d}" for index in range(2, 10)},
            {row["stageId"] for row in warp_cues},
        )
        for index, cue in enumerate(warp_cues, start=2):
            self.assertEqual(
                f"cue.valtan.phase2.warp.step-{index:02d}.composite",
                cue["bindingId"],
            )
            self.assertEqual(
                f"cue.valtan.phase2.warp.step-{index:02d}.composite.occurrence.01",
                cue["occurrenceId"],
            )
            self.assertEqual(
                "effect.valtan.project-tuned.sequence.warp.portal",
                cue["effectAssetId"],
            )

    def test_typed_draft_updates_motion_and_swept_schedule_atomically(self) -> None:
        source_stage = copy.deepcopy(self.warp_legs(self.joined_master())[0])
        source_stage["hit"]["schedule"]["offsetsMs"] = list(range(450, 900, 50))
        candidate = self.apply(
            [
                {
                    "op": "SET_STAGE_PORTAL_RUSH_MOTION",
                    "patternId": "VALTAN_WARP",
                    "stageId": "STEP_02",
                    "retargetDelayMs": 450,
                    "speedMps": 20.0,
                    "distanceM": 9.0,
                },
                {
                    "op": "SET_STAGE_HIT",
                    "patternId": "VALTAN_WARP",
                    "stageId": "STEP_02",
                    "hit": source_stage["hit"],
                },
            ]
        )
        stage = self.warp_legs(candidate)[0]
        self.assertEqual(
            {
                "kind": "PORTAL_TARGET_RUSH",
                "retargetDelayMs": 450,
                "speedMps": 20.0,
                "distanceM": 9.0,
            },
            stage["motion"],
        )
        self.assertEqual(list(range(450, 900, 50)), stage["hit"]["schedule"]["offsetsMs"])

    def test_overrun_wrong_owner_and_stale_pre_delay_hits_fail_closed(self) -> None:
        overrun = {
            "op": "SET_STAGE_PORTAL_RUSH_MOTION",
            "patternId": "VALTAN_WARP",
            "stageId": "STEP_02",
            "retargetDelayMs": 500,
            "speedMps": 20.0,
            "distanceM": 9.0,
        }
        with self.assertRaisesRegex(
            pipeline.DraftPatchError, "delay plus travel exceeds"
        ):
            self.apply([overrun])

        wrong_owner = copy.deepcopy(overrun)
        wrong_owner.update(
            patternId="VALTAN_GHOST_FINALE", stageId="STEP_02", distanceM=8.0
        )
        with self.assertRaisesRegex(
            pipeline.DraftPatchError, "only admits an existing VALTAN_WARP"
        ):
            self.apply([wrong_owner])

        stale_schedule = copy.deepcopy(overrun)
        stale_schedule["distanceM"] = 6.0
        stale_schedule["retargetDelayMs"] = 600
        with self.assertRaisesRegex(
            pipeline.DraftPatchError, "hit offsets must start at retargetDelayMs"
        ):
            self.apply([stale_schedule])

    def test_runtime_and_workbench_sources_keep_one_typed_owner(self) -> None:
        brain = (self.root / "Server/Private/ValtanBrain.cpp").read_text(
            encoding="utf-8-sig"
        )
        motion_function = brain.index("CValtanBrain::Try_BuildStageMotion")
        explicit_branch = brain.index("PORTAL_TARGET_RUSH", motion_function)
        root_motion_branch = brain.index("PatternStageRootMotion.empty", motion_function)
        self.assertLess(explicit_branch, root_motion_branch)
        self.assertIn("iPortalRushRetargetDelayMs", brain)
        self.assertIn("fPortalRushSpeedMps", brain)
        self.assertIn("fPortalRushDistanceM", brain)

        publisher = (
            self.root / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1"
        ).read_text(encoding="utf-8-sig")
        self.assertIn("$patternStageMotionKindByKey", publisher)
        self.assertIn("-cne 'PORTAL_TARGET_RUSH'", publisher)

        workbench = (self.root / "Client/Private/Animation_Tool.cpp").read_text(
            encoding="utf-8-sig"
        )
        balance = (self.root / "Client/Private/BalanceTool.cpp").read_text(
            encoding="utf-8-sig"
        )
        self.assertIn("Retarget / wait delay ms", workbench)
        self.assertIn("Rush speed m/s", workbench)
        self.assertIn("Rush distance m", workbench)
        self.assertIn("Binding %s | occurrence %s", workbench)
        self.assertIn("SET_STAGE_PORTAL_RUSH_MOTION", balance)


if __name__ == "__main__":
    unittest.main()
