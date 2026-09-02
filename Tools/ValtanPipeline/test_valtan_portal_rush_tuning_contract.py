#!/usr/bin/env python3
from __future__ import annotations

import copy
import json
import math
import struct
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
    "distanceM": 16.0,
}
BASELINE_DURATION_MS = 1800
BASELINE_OFFSETS = list(range(500, 1300, 50))


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
                    self.assertEqual(BASELINE_DURATION_MS, stage["durationMs"])
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
                    self.assertEqual(16, len(offsets))
                    self.assertEqual(
                        500.0,
                        stage["durationMs"]
                        - stage["motion"]["retargetDelayMs"]
                        - stage["motion"]["distanceM"]
                        / stage["motion"]["speedMps"]
                        * 1000.0,
                    )

        presentation = next(
            row
            for row in self.docs[pipeline.PRESENTATION_AUTHORING_REL]["patterns"]
            if row["patternId"] == "VALTAN_WARP"
        )
        rush_presentation = presentation["stages"][1:9]
        self.assertEqual(8, len(rush_presentation))
        for index, stage in enumerate(rush_presentation, start=2):
            animation = stage["animation"]
            self.assertEqual("LOOP_TO_STAGE_END", animation["endPolicy"])
            self.assertEqual(1, len(animation["occurrences"]))
            occurrence = animation["occurrences"][0]
            self.assertEqual(
                f"valtan.sequence.warp.step-{index:02d}.clip-01",
                occurrence["clipOccurrenceId"],
            )
            self.assertEqual(0, occurrence["playMs"])
            self.assertTrue(occurrence["repeatUntilStageEnd"])
            self.assertEqual(
                {"hiddenFromMs": 1300, "hiddenToMs": 1800},
                stage["bodyVisibility"],
            )
            self.assertEqual(1300, stage["effectCues"][0]["sourceStartMs"])

    def test_each_rush_boundary_is_a_root_snapshot_not_a_predicted_endpoint(self) -> None:
        authored = pipeline.read_json(
            self.root
            / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
        )
        warp_cues = [row for row in authored["cues"] if row["patternId"] == "VALTAN_WARP"]
        self.assertEqual(9, len(warp_cues))
        self.assertEqual(
            {f"STEP_{index:02d}" for index in range(2, 11)},
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
            self.assertEqual("root", cue["anchorSlotId"])
            self.assertEqual("snapshot", cue["followPolicy"])
            self.assertEqual([0.0, 0.0, 0.0], cue["localTransform"]["position"])

    def test_all_eight_rush_legs_project_one_derived_gap_contract(self) -> None:
        operations: list[dict] = []
        source_legs = self.warp_legs(self.joined_master())
        expected_offsets = list(range(600, 1100, 50))
        for index, source_stage in enumerate(source_legs, start=2):
            stage_id = f"STEP_{index:02d}"
            hit = copy.deepcopy(source_stage["hit"])
            hit["schedule"]["offsetsMs"] = expected_offsets
            operations.extend(
                [
                    {
                        "op": "SET_STAGE_DURATION",
                        "patternId": "VALTAN_WARP",
                        "stageId": stage_id,
                        "durationMs": 1350,
                    },
                    {
                        "op": "SET_STAGE_PORTAL_RUSH_MOTION",
                        "patternId": "VALTAN_WARP",
                        "stageId": stage_id,
                        "retargetDelayMs": 600,
                        "speedMps": 16.0,
                        "distanceM": 8.0,
                    },
                    {
                        "op": "SET_STAGE_HIT",
                        "patternId": "VALTAN_WARP",
                        "stageId": stage_id,
                        "hit": hit,
                    },
                ]
            )
        candidate = self.apply(operations)
        product_outputs = pipeline.project_v2_products(self.root, self.docs, candidate)
        product = json.loads(product_outputs[pipeline.ENCOUNTER_REL])

        for owner in (candidate, product):
            with self.subTest(owner=owner.get("schema")):
                for stage in self.warp_legs(owner):
                    self.assertEqual(1350, stage["durationMs"])
                    self.assertEqual(
                        {
                            "kind": "PORTAL_TARGET_RUSH",
                            "retargetDelayMs": 600,
                            "speedMps": 16.0,
                            "distanceM": 8.0,
                        },
                        stage["motion"],
                    )
                    offsets = (
                        stage["hit"]["schedule"]["offsetsMs"]
                        if "hit" in stage
                        else stage["hitOffsetsMs"]
                    )
                    self.assertEqual(expected_offsets, offsets)
                    if "bodyVisibility" in stage:
                        self.assertEqual(
                            {"hiddenFromMs": 1100, "hiddenToMs": 1350},
                            stage["bodyVisibility"],
                        )

    def test_each_warp_leg_clock_is_independently_authored(self) -> None:
        gameplay = copy.deepcopy(self.docs[pipeline.GAMEPLAY_AUTHORING_REL])
        self.warp_legs(gameplay)[0]["durationMs"] = 1801
        pipeline.validate_gameplay_authoring(gameplay)

    def test_float_storage_is_canonical_before_50ms_boundary_sampling(self) -> None:
        def float32(value: float) -> float:
            return struct.unpack("<f", struct.pack("<f", value))[0]

        exact_storage = float32(0.15)
        exact_bits = struct.unpack("<I", struct.pack("<f", exact_storage))[0]
        lower_storage = struct.unpack(
            "<f", struct.pack("<I", exact_bits - 1)
        )[0]
        for distance_m, expected_duration, expected_offsets in (
            (lower_storage, 550, [500]),
            (exact_storage, 551, [500, 550]),
        ):
            travel_ms = distance_m / float32(3.0) * 1000.0
            self.assertEqual(expected_duration, math.ceil(500 + travel_ms))
            self.assertEqual(
                expected_offsets,
                list(range(500, math.ceil(500 + travel_ms), 50)),
            )

    def test_overrun_wrong_owner_and_stale_pre_delay_hits_fail_closed(self) -> None:
        overrun = {
            "op": "SET_STAGE_PORTAL_RUSH_MOTION",
            "patternId": "VALTAN_WARP",
            "stageId": "STEP_02",
            "retargetDelayMs": 500,
            "speedMps": 20.0,
            "distanceM": 37.0,
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
            pipeline.DraftPatchError, "requires an existing PORTAL_TARGET_RUSH Stage"
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
        self.assertIn("-ceq 'PORTAL_TARGET_RUSH'", publisher)

        animation_tool = (self.root / "Client/Private/Animation_Tool.cpp").read_text(
            encoding="utf-8-sig"
        )
        composition = (
            self.root / "Client/Private/ActionCompositionWorkbench.cpp"
        ).read_text(encoding="utf-8-sig")
        balance = (self.root / "Client/Private/BalanceTool.cpp").read_text(
            encoding="utf-8-sig"
        )
        header = (self.root / "Client/Public/BalanceTool.h").read_text(
            encoding="utf-8-sig"
        )
        self.assertIn("Retarget / wait delay ms", animation_tool)
        self.assertIn("Rush speed m/s", animation_tool)
        self.assertIn("Rush distance m", animation_tool)
        self.assertIn("Portal Gap After Rush (ms)", animation_tool)
        self.assertIn(
            "!Draft.durationEditable || Draft.portalRushMotionEditable",
            animation_tool,
        )
        self.assertIn("Binding %s | occurrence %s", animation_tool)
        self.assertIn("Normalize_ValtanPortalRushDraft", animation_tool)
        self.assertNotIn("const auto NormalizePortalRush", animation_tool)
        self.assertIn("Warp Rush - All 8 Legs", composition)
        self.assertIn("Delay Before Rush (ms)##WarpAllLegs", composition)
        self.assertIn("Rush Speed (m/s)##WarpAllLegs", composition)
        self.assertIn("Rush Distance (m)##WarpAllLegs", composition)
        self.assertIn(
            "Portal Gap After Rush (ms)##WarpAllLegs", composition
        )
        self.assertIn('"PORTAL_TARGET_RUSH" == strMotionKind', composition)
        self.assertIn('Stage.strStageId + "/motion/delay"', composition)
        self.assertIn('Stage.strStageId + "/motion/travel"', composition)
        self.assertIn('Stage.strStageId + "/motion/gap"', composition)
        self.assertIn('"Target Delay | "', composition)
        self.assertIn('"Rush | "', composition)
        self.assertIn('"Gap | "', composition)
        self.assertIn("fDistanceM / fSpeedMps * 1000.0", composition)
        self.assertIn("iStageDurationMs - iTravelEndMs", composition)
        self.assertIn("else if (!bWarpLegClockOwned)", composition)
        self.assertIn(
            "Distance endpoint currently bypasses navigation clamp",
            composition,
        )
        self.assertIn("Normalize_ValtanPortalRushDraft", composition)
        self.assertIn("struct VALTAN_WARP_RUSH_EDIT final", header)
        self.assertIn("Get_ValtanWarpRushDraft", header)
        self.assertIn("Set_ValtanWarpRushDraft", header)
        setter_start = balance.index(
            "bool Client::CBalanceTool::Set_ValtanWarpRushDraft("
        )
        setter_end = balance.index(
            "bool Client::CBalanceTool::Get_ValtanPatternDraft(", setter_start
        )
        setter = balance[setter_start:setter_end]
        self.assertIn("VALTAN_PATTERN_TREE_VIEW staged = m_valtanPatternTree", setter)
        self.assertEqual(1, setter.count("m_valtanPatternTree = std::move(staged)"))
        self.assertLess(
            setter.index("for (const char* const stageId"),
            setter.index("m_valtanPatternTree = std::move(staged)"),
        )
        self.assertIn("stage->HitOffsetsMs = draft.hitOffsetsMs", setter)
        self.assertIn("stage->iHitCount = draft.hitCount", setter)
        self.assertIn("stage->iDurationMs = draft.durationMs", setter)
        self.assertIn("RetargetPortalRushLoopToStageEnd", setter)
        self.assertIn("rush.trailingGapMs", setter)
        normalizer_start = balance.index(
            "bool Client::CBalanceTool::Normalize_ValtanPortalRushDraft("
        )
        normalizer_end = balance.index(
            "bool Client::CBalanceTool::Get_ValtanWarpRushDraft(",
            normalizer_start,
        )
        normalizer = balance[normalizer_start:normalizer_end]
        self.assertLess(
            normalizer.index("static_cast<float>(stage.portalDistanceM)"),
            normalizer.index("const double travelMs"),
        )
        self.assertIn("std::ceil(totalMs)", normalizer)
        self.assertIn("stage.hitOffsetsMs.size() < 64u", normalizer)
        self.assertIn("totalMs > 120000.0", normalizer)
        self.assertIn("SET_STAGE_PORTAL_RUSH_MOTION", balance)


if __name__ == "__main__":
    unittest.main()
