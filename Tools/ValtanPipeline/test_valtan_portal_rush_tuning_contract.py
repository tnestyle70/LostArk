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
PORTAL_DISTANCE_M = 16.0
PORTAL_TRAVEL_MS = 1300
PORTAL_SPEED_MPS = 12.3076925
FIRST_LEG_DELAY_MS = 300
REPEAT_LEG_DELAY_MS = 600
FIRST_LEG_DURATION_MS = FIRST_LEG_DELAY_MS + PORTAL_TRAVEL_MS
REPEAT_LEG_DURATION_MS = REPEAT_LEG_DELAY_MS + PORTAL_TRAVEL_MS


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
                for index, stage in enumerate(legs):
                    expected_delay_ms = (
                        FIRST_LEG_DELAY_MS if index == 0 else REPEAT_LEG_DELAY_MS
                    )
                    expected_duration_ms = (
                        FIRST_LEG_DURATION_MS
                        if index == 0
                        else REPEAT_LEG_DURATION_MS
                    )
                    self.assertEqual(expected_duration_ms, stage["durationMs"])
                    self.assertEqual(
                        {
                            "kind": "PORTAL_TARGET_RUSH",
                            "retargetDelayMs": expected_delay_ms,
                            "speedMps": PORTAL_SPEED_MPS,
                            "distanceM": PORTAL_DISTANCE_M,
                        },
                        stage["motion"],
                    )
                    if "hit" in stage:
                        schedule = stage["hit"]["schedule"]
                        self.assertEqual("EXPLICIT_OFFSETS", schedule["kind"])
                        offsets = schedule["offsetsMs"]
                    else:
                        offsets = stage["hitOffsetsMs"]
                        self.assertEqual(len(offsets), stage["hitCount"])
                    self.assertEqual(
                        list(range(expected_delay_ms, expected_duration_ms, 50)),
                        offsets,
                    )
                    self.assertGreaterEqual(
                        min(offsets), stage["motion"]["retargetDelayMs"]
                    )
                    self.assertEqual(26, len(offsets))
                    self.assertAlmostEqual(
                        0.0,
                        stage["durationMs"]
                        - stage["motion"]["retargetDelayMs"]
                        - stage["motion"]["distanceM"]
                        / stage["motion"]["speedMps"]
                        * 1000.0,
                        delta=0.001,
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
                {
                    "hiddenFromMs": 0,
                    "hiddenToMs": (
                        FIRST_LEG_DELAY_MS if index == 2 else REPEAT_LEG_DELAY_MS
                    ),
                },
                stage["bodyVisibility"],
            )
            self.assertEqual([], stage["effectCues"])
        self.assertEqual(
            {"hiddenFromMs": 0, "hiddenToMs": 300},
            presentation["stages"][9]["bodyVisibility"],
        )

    def test_first_rush_spawns_at_stage_edge_and_repeats_wait_for_handoff(self) -> None:
        bindings = json.loads(
            (self.root / "Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json")
            .read_text(encoding="utf-8")
        )
        warp_bindings = [
            row
            for row in bindings["bindings"]
            if row["scope"]["patternId"] == "VALTAN_WARP"
        ]
        self.assertEqual(16, len(warp_bindings))
        self.assertEqual(
            {f"STEP_{index:02d}" for index in range(2, 10)},
            {row["scope"]["stageId"] for row in warp_bindings},
        )
        for index in range(2, 10):
            stage_id = f"STEP_{index:02d}"
            stage_rows = [
                row for row in warp_bindings if row["scope"]["stageId"] == stage_id
            ]
            self.assertEqual(2, len(stage_rows))
            expected_start_ms = 0 if index == 2 else 300
            self.assertEqual(
                [expected_start_ms, expected_start_ms],
                [row["clock"]["startMs"] for row in stage_rows],
            )
            for row in stage_rows:
                self.assertEqual(
                    {"kind": "GROUP", "id": "boss.valtan.portal"},
                    row["resource"],
                )
                self.assertEqual("STAGE", row["clock"]["basis"])
                self.assertIsNone(row["clock"]["clipOccurrenceId"])
                self.assertEqual("ONCE", row["clock"]["repeatPolicy"])
                self.assertIn(
                    row["anchor"]["slotId"],
                    {"portal.rush.start", "portal.rush.end"},
                )
                self.assertEqual(
                    "SNAPSHOT_AT_START", row["anchor"]["followPolicy"]
                )
                self.assertEqual("TARGET_YAW", row["anchor"]["rotationBasis"])
                self.assertEqual(
                    [0.0, 0.0, 0.0],
                    row["anchor"]["localTransform"]["rotation"],
                )
                self.assertEqual(
                    [1.0, 1.0, 1.0],
                    row["anchor"]["localTransform"]["scale"],
                )
                self.assertEqual("NATURAL", row["stopPolicy"])
            rows_by_suffix = {
                row["bindingId"].rsplit(".", 1)[-1]: row for row in stage_rows
            }
            self.assertEqual(
                [0.0, 0.0, 0.0],
                rows_by_suffix["portal-0-start"]["anchor"]["localTransform"][
                    "translation"
                ],
            )
            self.assertEqual(
                "portal.rush.start",
                rows_by_suffix["portal-0-start"]["anchor"]["slotId"],
            )
            self.assertEqual(
                [0.0, 0.0, 0.0],
                rows_by_suffix["portal-1-end"]["anchor"]["localTransform"][
                    "translation"
                ],
            )
            self.assertEqual(
                "portal.rush.end",
                rows_by_suffix["portal-1-end"]["anchor"]["slotId"],
            )

    def test_portal_group_holds_then_dissolves_for_one_rush_stage(self) -> None:
        group = json.loads(
            (self.root / "Data/Effects/V2/Groups/boss.valtan.portal.effectv2group.json")
            .read_text(encoding="utf-8")
        )
        self.assertEqual(1900, group["durationMs"])
        self.assertEqual(2, len(group["children"]))
        for child in group["children"]:
            self.assertEqual(0, child["startMs"])
            self.assertEqual(1900, child["durationMs"])
            self.assertEqual(
                [1.0, 1.0, 1.0], child["localTransform"]["scale"]
            )
        expected_positions = {
            "black_1": [-1.0, 3.0, 0.0],
            "cyan_1": [-0.95, 3.0, 0.0],
        }
        expected_scales = {
            "black_1": ([4.0, 6.0, 2.0], [2.0, 2.0, 2.0]),
            "cyan_1": ([4.8, 7.2, 2.0], [2.0, 2.0, 2.0]),
        }
        for suffix, expected_position in expected_positions.items():
            leaf = json.loads(
                (
                    self.root
                    / f"Data/Effects/V2/Authored/boss.valtan.portal.{suffix}.effectv2.json"
                ).read_text(encoding="utf-8")
            )
            self.assertEqual(1.9, leaf["params"]["lifetime"])
            self.assertEqual(
                expected_position, leaf["params"]["position"]["start"]
            )
            self.assertEqual(
                expected_scales[suffix][0], leaf["params"]["scale"]["start"]
            )
            self.assertEqual(
                expected_scales[suffix][1], leaf["params"]["scale"]["end"]
            )
            self.assertEqual(0.0, leaf["params"]["dissolveInEnd"])
            self.assertAlmostEqual(
                1600.0 / 1900.0,
                leaf["params"]["dissolveStart"],
                places=7,
            )
            self.assertAlmostEqual(
                1600.0,
                leaf["params"]["lifetime"]
                * 1000.0
                * leaf["params"]["dissolveStart"],
                delta=0.001,
            )
            self.assertFalse(leaf["params"]["loop"])

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
        projected_cues = json.loads(product_outputs[pipeline.CUES_REL])

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
                            {"hiddenFromMs": 0, "hiddenToMs": 600},
                            stage["bodyVisibility"],
                        )
                    else:
                        self.assertEqual("lostark.encounter-profile", owner.get("schema"))

        for stage in self.warp_legs(candidate):
            self.assertEqual([], stage["effectCues"])
        projected_rush_cues = [
            cue
            for cue in projected_cues["cues"]
            if cue["patternId"] == "VALTAN_WARP"
            and cue["stageId"] in {f"STEP_{index:02d}" for index in range(2, 10)}
        ]
        self.assertEqual([], projected_rush_cues)

    def test_each_warp_leg_clock_is_independently_authored(self) -> None:
        gameplay = copy.deepcopy(self.docs[pipeline.GAMEPLAY_AUTHORING_REL])
        self.warp_legs(gameplay)[0]["durationMs"] = 1601
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
            "retargetDelayMs": 300,
            "speedMps": 20.0,
            "distanceM": 37.0,
        }
        with self.assertRaisesRegex(
            pipeline.DraftPatchError, "delay plus travel exceeds"
        ):
            self.apply([overrun])

        no_trailing_gap = copy.deepcopy(overrun)
        no_trailing_gap["distanceM"] = 26.0
        exact_fit_hit = copy.deepcopy(self.warp_legs(self.joined_master())[0]["hit"])
        exact_fit_hit["schedule"]["offsetsMs"] = list(range(300, 1600, 50))
        exact_fit = self.apply(
            [
                no_trailing_gap,
                {
                    "op": "SET_STAGE_HIT",
                    "patternId": "VALTAN_WARP",
                    "stageId": "STEP_02",
                    "hit": exact_fit_hit,
                },
            ]
        )
        self.assertEqual(
            {"hiddenFromMs": 0, "hiddenToMs": 300},
            self.warp_legs(exact_fit)[0]["bodyVisibility"],
        )
        self.assertEqual([], self.warp_legs(exact_fit)[0]["effectCues"])

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
        configure_start = brain.index("CValtanBrain::Configure_PortalMotion")
        configure_end = brain.index(
            "CValtanBrain::Lock_PortalTargetRushAtStageStart", configure_start
        )
        self.assertNotIn(
            "LockPortalTargetRushAtStageStart(boss)",
            brain[configure_start:configure_end],
        )

        game_room = (self.root / "Server/Private/GameRoom.cpp").read_text(
            encoding="utf-8-sig"
        )
        commit_start = game_room.index("Commit_BossPatternPlayerStageActions(")
        commit_end = game_room.index("Drain_BossCombatEvents", commit_start)
        commit = game_room[commit_start:commit_end]
        self.assertLess(
            commit.index("RETARGET_RANDOM_ALIVE"),
            commit.index("Lock_PortalTargetRushAtStageStart"),
        )

        client = (self.root / "Client/Private/Valtan.cpp").read_text(
            encoding="utf-8-sig"
        )
        apply_start = client.index("CValtan::Apply_NetworkState")
        apply_end = client.index("CValtan::", apply_start + 1)
        apply_network_state = client[apply_start:apply_end]
        self.assertIn('"VALTAN_WARP" == patternId', apply_network_state)
        self.assertIn("bWarpPortalRushStage", apply_network_state)
        self.assertIn("m_PortalRushRoute = PortalRushRoute", apply_network_state)
        self.assertLess(
            apply_network_state.index("m_PortalRushRoute = PortalRushRoute"),
            apply_network_state.index("CEffectV2Runtime::Sync_Stage"),
        )
        self.assertIn("m_LocalPreviewPortalRushDistanceByActionId", client)
        self.assertIn("Try_Get_PortalRushAnchorMatrices", client)
        self.assertIn("m_LocalPreviewBodyVisibilityByActionId", client)
        self.assertIn("BodyVisibilityByActionId.find(StageActionId)", client)
        self.assertIn("m_isPatternBodyHidden = bPatternBodyHidden", client)
        self.assertIn("m_LocalPreviewBodyVisibilityByActionId.clear()", client)

        shared_header = (
            self.root / "Shared/Public/Network/PacketMessages.h"
        ).read_text(encoding="utf-8-sig")
        shared_source = (
            self.root / "Shared/Private/Network/PacketMessages.cpp"
        ).read_text(encoding="utf-8-sig")
        effect_object = (
            self.root / "Client/Private/EffectV2_Object.cpp"
        ).read_text(encoding="utf-8-sig")
        self.assertIn("PORTAL_RUSH_ROUTE_SNAPSHOT", shared_header)
        self.assertIn("PortalRushRoute.fStartX", shared_source)
        self.assertIn("PortalRushRoute.fEndZ", shared_source)
        self.assertIn('"portal.rush.start" == strBone', effect_object)
        self.assertIn('"portal.rush.end" == strBone', effect_object)

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
        self.assertIn("Portal lead before rush ms", animation_tool)
        self.assertIn("Rush speed m/s", animation_tool)
        self.assertIn("Rush distance m", animation_tool)
        self.assertIn("Next portal offset after arrival ms", animation_tool)
        self.assertIn(
            "!Draft.durationEditable || Draft.portalRushMotionEditable",
            animation_tool,
        )
        self.assertIn("Binding %s | occurrence %s", animation_tool)
        self.assertIn("Normalize_ValtanPortalRushDraft", animation_tool)
        self.assertNotIn("const auto NormalizePortalRush", animation_tool)
        self.assertIn("Warp Rush - All 8 Legs", composition)
        self.assertIn("Portal Lead Before Rush (ms)##WarpAllLegs", composition)
        self.assertIn("Rush Speed (m/s)##WarpAllLegs", composition)
        self.assertIn("Rush Distance (m)##WarpAllLegs", composition)
        self.assertIn(
            "Next Portal Offset After Arrival (ms)##WarpAllLegs", composition
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
        self.assertIn(
            "stage->iBodyHiddenToMs = draft.portalRetargetDelayMs", setter
        )
        self.assertIn(
            "recovery->iBodyHiddenToMs = rush.trailingGapMs", setter
        )
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
