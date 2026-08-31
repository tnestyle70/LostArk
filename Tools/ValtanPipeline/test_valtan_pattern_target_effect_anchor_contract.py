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
        cls.effect_product = json.loads(
            read("Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json")
        )
        cls.client_replication = read("Client/Private/ClientReplication.cpp")
        cls.valtan_h = read("Client/Public/Valtan.h")
        cls.valtan_cpp = read("Client/Private/Valtan.cpp")
        cls.cue_cpp = read("Client/Private/ValtanPatternEffectCueDocument.cpp")
        cls.tree_cpp = read("Client/Private/ValtanPatternTree.cpp")

    def test_six_pizza_source_and_product_use_exact_target_snapshot_anchor(self) -> None:
        source_pattern = next(
            row for row in self.presentation["patterns"]
            if row["patternId"] == "VALTAN_SIX_PIZZA_106"
        )
        source_cues = [
            cue for stage in source_pattern["stages"]
            for cue in stage["effectCues"]
        ]
        product_cues = [
            cue for cue in self.effect_product["cues"]
            if cue["patternId"] == "VALTAN_SIX_PIZZA_106"
        ]
        self.assertTrue(source_cues)
        self.assertTrue(product_cues)
        for cue in source_cues + product_cues:
            self.assertEqual("pattern.target.snapshot", cue["anchorSlotId"])
            self.assertEqual("snapshot", cue["followPolicy"])

    def test_split_pipeline_rejects_moving_unknown_or_unlocked_target_anchor(self) -> None:
        joined = pipeline.join_v2_authoring(
            self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL],
            self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL],
        )
        mutations = (
            lambda pattern, cue: cue.update(followPolicy="follow"),
            lambda pattern, cue: cue.update(anchorSlotId="pattern.target.unknown"),
            lambda pattern, cue: pattern.update(targetPolicy="NEAREST_EACH_TICK"),
            lambda pattern, cue: pattern.update(targetPolicy="NONE"),
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
        self.assertIn("PatternTargetPose)", apply)

    def test_valtan_latches_pose_to_sequence_and_identity_then_spawns_fixed_root(self) -> None:
        apply = function_slice(
            self.valtan_cpp,
            "bool_t CValtan::Apply_NetworkState(",
            "unique_ptr<CValtan> CValtan::Create(",
        )
        spawn = function_slice(
            self.valtan_cpp,
            "void CValtan::Spawn_DuePatternEffectCues(",
            "void CValtan::Load_PatternSoundCues(",
        )
        for token in (
            "m_iServerPatternTargetPoseSequence",
            "m_iServerPatternTargetNetEntityId",
            "m_bServerPatternTargetIdentityStable",
            "m_bHasServerPatternTargetSnapshotPose",
        ):
            self.assertIn(token, self.valtan_h)
            self.assertIn(token, apply + spawn)
        for token in (
            "Is_PatternTargetSnapshotCueAnchor(Cue.strAnchorSlotId)",
            "m_iServerPatternTargetPoseSequence == m_iServerPatternSequence",
            "XMMatrixRotationY(XMConvertToRadians(",
            "m_fServerPatternTargetSnapshotYawDegrees))",
            "m_vServerPatternTargetSnapshotPosition.x",
            "CEffectPresentationService::Build_CueScalePolicyRoot(",
            "CEffectPresentationService::Spawn_WorldRoot(",
            "Pattern-target snapshot cue has no admitted finite target pose.",
        ):
            self.assertIn(token, spawn)
        target_branch = spawn[
            spawn.index("if (Is_PatternTargetSnapshotCueAnchor"):
            spawn.index("else if (Is_ArenaCenterCueAnchor")
        ]
        self.assertNotIn("m_PatternArenaCenterAnchors", target_branch)
        self.assertNotIn("m_fServerPatternFacingYawDegrees", target_branch)
        self.assertNotIn("CEffectPresentationService::Spawn(Desc", target_branch)

    def test_runtime_parsers_reserve_target_anchor_and_require_locked_snapshot(self) -> None:
        for text in (self.cue_cpp, self.tree_cpp):
            self.assertIn('"pattern.target.snapshot"', text)
            self.assertIn('starts_with("pattern.target.")', text)
            self.assertIn('"LOCK_RANDOM_ALIVE_ON_START"', text)
            self.assertIn('"snapshot"', text)


if __name__ == "__main__":
    unittest.main()
