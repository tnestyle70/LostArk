#!/usr/bin/env python3
from __future__ import annotations

import copy
import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/ValtanPipeline"))
import valtan_tuning_pipeline as pipeline  # noqa: E402


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


def function_slice(text: str, signature: str, next_signature: str) -> str:
    start = text.index(signature)
    end = text.index(next_signature, start + len(signature))
    return text[start:end]


class ValtanPatternTargetEffectAnchorContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.docs = pipeline.load_pipeline_documents(ROOT)
        cls.presentation = cls.docs[pipeline.PRESENTATION_AUTHORING_REL]
        cls.client_replication = read("Client/Private/ClientReplication.cpp")
        cls.valtan_h = read("Client/Public/Valtan.h")
        cls.valtan_cpp = read("Client/Private/Valtan.cpp")
        cls.cue_cpp = read("Client/Private/ValtanPatternEffectCueDocument.cpp")
        cls.tree_cpp = read("Client/Private/ValtanPatternTree.cpp")
        cls.cue_authoring_cpp = read(
            "Client/Private/ValtanPatternEffectCueAuthoring.cpp"
        )
        cls.workbench_cpp = read("Client/Private/ActionCompositionWorkbench.cpp")
        cls.six_pizza_effect = json.loads(read(
            "Data/Effects/Authored/"
            "effect.valtan.project-tuned.sequence.six-pizza-106.effect.json"
        ))

    def test_six_pizza_source_and_projection_use_exact_target_follow_contract(self) -> None:
        source_gameplay_pattern = next(
            row for row in self.docs[pipeline.GAMEPLAY_AUTHORING_REL]["patterns"]
            if row["patternId"] == "VALTAN_SIX_PIZZA_106"
        )
        source_presentation_pattern = next(
            row for row in self.presentation["patterns"]
            if row["patternId"] == "VALTAN_SIX_PIZZA_106"
        )
        source_cues = [
            cue for stage in source_presentation_pattern["stages"]
            for cue in stage["effectCues"]
        ]
        master = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        joined_pattern = next(
            row for row in master["patterns"]
            if row["patternId"] == "VALTAN_SIX_PIZZA_106"
        )
        projected = pipeline.project_v2_products(ROOT, self.docs, master)
        product_pattern = next(
            row for row in json.loads(projected[pipeline.ENCOUNTER_REL])["patterns"]
            if row["patternId"] == "VALTAN_SIX_PIZZA_106"
        )
        product_cues = [
            cue for cue in json.loads(projected[pipeline.CUES_REL])["cues"]
            if cue["patternId"] == "VALTAN_SIX_PIZZA_106"
        ]
        self.assertEqual(1, len(source_cues))
        self.assertEqual(1, len(product_cues))
        for pattern in (
            source_gameplay_pattern, joined_pattern, product_pattern
        ):
            self.assertEqual(
                ("LOCK_RANDOM_ALIVE_ON_START", "TRACK_TARGET_EACH_TICK"),
                (pattern["targetPolicy"], pattern["aimPolicy"]),
            )
        for cue in source_cues + product_cues:
            self.assertEqual("arena.center.target-follow", cue["anchorSlotId"])
            self.assertEqual("follow", cue["followPolicy"])

    def test_high_jump_keeps_target_landing_but_places_takeoff_and_land_at_fixed_center(self) -> None:
        gameplay_pattern = next(
            row for row in self.docs[pipeline.GAMEPLAY_AUTHORING_REL]["patterns"]
            if row["patternId"] == "VALTAN_HIGH_JUMP"
        )
        presentation_pattern = next(
            row for row in self.presentation["patterns"]
            if row["patternId"] == "VALTAN_HIGH_JUMP"
        )
        self.assertEqual("LEAP_TO_TARGET", gameplay_pattern["serverMotion"]["kind"])
        source_cues = {
            stage["stageId"]: stage["effectCues"][0]
            for stage in presentation_pattern["stages"]
            if stage["stageId"] in {"TAKEOFF", "LAND"}
        }
        self.assertEqual({"TAKEOFF", "LAND"}, set(source_cues))
        for cue in source_cues.values():
            self.assertEqual("arena.center", cue["anchorSlotId"])
            self.assertEqual("snapshot", cue["followPolicy"])

        master = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        joined_pattern = next(
            row for row in master["patterns"]
            if row["patternId"] == "VALTAN_HIGH_JUMP"
        )
        self.assertEqual("LEAP_TO_TARGET", joined_pattern["serverMotion"]["kind"])
        outputs = pipeline.project_v2_products(ROOT, self.docs, master)
        product_cues = [
            cue for cue in json.loads(outputs[pipeline.CUES_REL])["cues"]
            if cue["patternId"] == "VALTAN_HIGH_JUMP"
            and cue["stageId"] in {"TAKEOFF", "LAND"}
        ]
        self.assertEqual(2, len(product_cues))
        for cue in product_cues:
            self.assertEqual("arena.center", cue["anchorSlotId"])
            self.assertEqual("snapshot", cue["followPolicy"])

        invalid = copy.deepcopy(master)
        invalid_pattern = next(
            row for row in invalid["patterns"]
            if row["patternId"] == "VALTAN_HIGH_JUMP"
        )
        invalid_cue = next(
            cue for stage in invalid_pattern["stages"]
            for cue in stage["effectCues"]
        )
        invalid_cue["anchorSlotId"] = "arena.center.facing"
        with self.assertRaises(pipeline.PipelineError):
            pipeline.validate_v2_master(
                invalid,
                self.docs[pipeline.WORLD_SET_REL],
                self.docs[pipeline.COMBAT_AUTHORING_REL],
            )

    def test_split_pipeline_rejects_fixed_unknown_or_untracked_target_follow_anchor(self) -> None:
        joined = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        mutations = (
            lambda pattern, cue: cue.update(followPolicy="snapshot"),
            lambda pattern, cue: cue.update(anchorSlotId="arena.center.facing"),
            lambda pattern, cue: cue.update(anchorSlotId="arena.center.unknown"),
            lambda pattern, cue: pattern.update(targetPolicy="NEAREST_EACH_TICK"),
            lambda pattern, cue: pattern.update(aimPolicy="LOCK_FACING_ON_START"),
            lambda pattern, cue: pattern.update(targetPolicy="NONE"),
            lambda pattern, cue: pattern.update(aimPolicy="NONE"),
        )
        for mutate in mutations:
            invalid = copy.deepcopy(joined)
            pattern = next(
                row for row in invalid["patterns"]
                if row["patternId"] == "VALTAN_SIX_PIZZA_106"
            )
            cue = next(
                cue for stage in pattern["stages"]
                for cue in stage["effectCues"]
            )
            mutate(pattern, cue)
            with self.assertRaises(pipeline.PipelineError):
                pipeline.validate_v2_master(
                    invalid,
                    self.docs[pipeline.WORLD_SET_REL],
                    self.docs[pipeline.COMBAT_AUTHORING_REL],
                )

    def test_six_pizza_static_sectors_inherit_the_mutable_composite_root(self) -> None:
        static_sector_ids = {
            "requested.20260827.six-pizza.sector.yellow-05",
            "authored.copy.authored.copy.authored.copy.sprite_particle_8.1.1.1",
            "authored.copy.authored.copy.authored.copy.authored.copy."
            "sprite_particle_8.1.1.1.1",
        }
        elements = {
            row["id"]: row for row in self.six_pizza_effect["elements"]
            if row["id"] in static_sector_ids
        }
        self.assertEqual(static_sector_ids, set(elements))
        self.assertEqual({11.0, 23.0}, {
            row["detail"]["timing"]["startDelaySeconds"]
            for row in elements.values()
        })
        for element_id, element in elements.items():
            with self.subTest(element=element_id):
                self.assertIs(
                    True,
                    element["detail"]["particle"]["localSpace"],
                    "static sectors must observe later updates to their shared root",
                )

        overlay = next(
            row for row in self.six_pizza_effect["elements"]
            if row["id"] ==
            "requested.20260827.six-pizza.sector.red-roar-overlay"
        )
        self.assertEqual("decal", overlay["kind"])
        self.assertEqual(
            (19.5, 0.779999971),
            (
                overlay["detail"]["timing"]["startDelaySeconds"],
                overlay["detail"]["timing"]["lifeTimeSeconds"],
            ),
        )
        self.assertEqual(
            [0, -182.75, 0],
            overlay["detail"]["transform"]["rotationDegrees"],
        )
        self.assertIs(False, overlay["actionCueAttachment"]["enabled"])
        self.assertIs(False, overlay["transformInheritance"]["enabled"])

    def test_client_resolves_target_pose_from_the_same_snapshot_player_rows(self) -> None:
        helper = function_slice(
            self.client_replication,
            "CValtan::PATTERN_TARGET_SNAPSHOT_POSE Resolve_ValtanPatternTargetSnapshotPose(",
            "bool Client::CClientReplication::Initialize(",
        )
        apply = function_slice(
            self.client_replication,
            "bool Client::CClientReplication::Apply_WorldSnapshot(",
            "Client::CClientReplication::CHARACTER_REPLACE_RESULT",
        )
        for token in (
            "std::span<const LostArk::Shared::PLAYER_SNAPSHOT>",
            "player.iNetEntityId != iTargetNetEntityId",
            "player.fPositionX",
            "player.fYawDegrees",
            "std::isfinite",
        ):
            self.assertIn(token, helper)
        self.assertNotIn("m_Registry", helper)
        self.assertIn("Resolve_ValtanPatternTargetSnapshotPose(", apply)
        self.assertIn("snapshot.Players.data()", apply)
        self.assertIn("snapshot.Players.size()", apply)
        self.assertIn("entity.iPatternTargetNetEntityId", apply)
        self.assertIn("PatternTargetPose,", apply)
        self.assertIn("entity.PortalRushRoute)", apply)

    def test_valtan_reuses_one_world_root_handle_across_server_yaw_ticks(self) -> None:
        apply = function_slice(
            self.valtan_cpp,
            "bool_t CValtan::Apply_NetworkState(",
            "unique_ptr<CValtan> CValtan::Create(",
        )
        spawn = function_slice(
            self.valtan_cpp,
            "void CValtan::Spawn_DuePatternEffectCues(",
            "void CValtan::Update_PatternTargetFollowEffectRoots(",
        )
        update = function_slice(
            self.valtan_cpp,
            "void CValtan::Update_PatternTargetFollowEffectRoots(",
            "void CValtan::Detach_PatternTargetFollowEffectRoots(",
        )
        detach = function_slice(
            self.valtan_cpp,
            "void CValtan::Detach_PatternTargetFollowEffectRoots(",
            "void CValtan::Load_PatternSoundCues(",
        )
        for token in (
            "m_iServerPatternTargetPoseSequence",
            "m_iServerPatternTargetNetEntityId",
            "m_bServerPatternTargetIdentityStable",
            "m_bHasServerPatternTargetSnapshotPose",
        ):
            self.assertIn(token, self.valtan_h)
            self.assertIn(token, apply + spawn + update)
        for token in (
            "PATTERN_TARGET_FOLLOW_EFFECT_ROOT",
            "m_PatternTargetFollowEffectRoots",
            "m_fServerPatternCurrentYawDegrees",
            "Update_PatternTargetFollowEffectRoots()",
            "Detach_PatternTargetFollowEffectRoots()",
        ):
            self.assertIn(token, self.valtan_h)
        for token in (
            "Is_ArenaCenterTargetFollowCueAnchor(Cue.strAnchorSlotId)",
            "m_fServerPatternCurrentYawDegrees",
            "VALTAN_TARGET_FOLLOW_ROOT_YAW_OFFSET_DEGREES",
            "CEffectPresentationService::Build_CueScalePolicyRoot(",
            "CEffectPresentationService::Spawn_WorldRoot(",
            "FollowRoot.iWorldRootHandle = handle.iValue",
            "FollowRoot.iPatternSequence = m_iServerPatternSequence",
            "FollowRoot.iTargetNetEntityId =",
            "m_PatternTargetFollowEffectRoots.push_back(",
        ):
            self.assertIn(token, spawn)
        for token in (
            "FollowRoot->iPatternSequence == m_iServerPatternSequence",
            "FollowRoot->iTargetNetEntityId ==",
            '"arena.center.target-follow"',
            "m_fServerPatternCurrentYawDegrees",
            "VALTAN_TARGET_FOLLOW_ROOT_YAW_OFFSET_DEGREES",
            "CEffectPresentationService::Update_WorldRoot(",
            "EFFECT_WORLD_ROOT_HANDLE{ FollowRoot->iWorldRootHandle }, Root",
        ):
            self.assertIn(token, update)
        self.assertNotIn("CEffectPresentationService::Spawn_WorldRoot(", update)
        self.assertNotIn("Stop_WorldRoot", detach)
        self.assertIn("m_PatternTargetFollowEffectRoots.clear()", detach)
        self.assertIn(
            "VALTAN_TARGET_FOLLOW_ROOT_YAW_OFFSET_DEGREES = 180.f",
            self.valtan_cpp,
        )

        current_yaw_assignment = (
            "m_fServerPatternCurrentYawDegrees = yawDegrees;"
        )
        self.assertIn(current_yaw_assignment, apply)
        self.assertIn("Update_PatternTargetFollowEffectRoots();", apply)
        self.assertLess(
            apply.index(current_yaw_assignment),
            apply.index("Update_PatternTargetFollowEffectRoots();"),
            "the accepted tick yaw must be staged before updating the existing root",
        )

    def test_runtime_parsers_reserve_snapshot_and_target_follow_anchor_contracts(self) -> None:
        for text in (self.cue_cpp, self.tree_cpp):
            self.assertIn('"pattern.target.snapshot"', text)
            self.assertIn('starts_with("pattern.target.")', text)
            self.assertIn('"arena.center.target-follow"', text)
            self.assertIn('"LOCK_RANDOM_ALIVE_ON_START"', text)
            self.assertIn('"TRACK_TARGET_EACH_TICK"', text)
            self.assertIn('"follow"', text)
            self.assertIn('"snapshot"', text)
            self.assertIn('"LEAP_TO_TARGET"', text)
        self.assertIn('"LEAP_TO_TARGET"', self.cue_authoring_cpp)
        self.assertIn('"LEAP_TO_TARGET"', self.workbench_cpp)


if __name__ == "__main__":
    unittest.main()
