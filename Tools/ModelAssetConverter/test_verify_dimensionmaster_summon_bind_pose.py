#!/usr/bin/env python3

from __future__ import annotations

import json
from pathlib import Path
import unittest

import verify_dimensionmaster_summon_bind_pose as verifier


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
RUNTIME_WMODEL = (
    REPOSITORY_ROOT
    / "Client/Bin/Resources/Character/DimensionMaster/DimensionMaster_DimensionSummon.wmodel"
)
RECEIPT = (
    REPOSITORY_ROOT
    / "Data/Effects/Imported/DimensionMaster/DimensionMaster.summon-bind-pose-repair.receipt.json"
)


class DimensionMasterSummonBindPoseTest(unittest.TestCase):
    def test_runtime_asset_matches_exact_receipt(self) -> None:
        receipt = json.loads(RECEIPT.read_text(encoding="utf-8"))
        witness = verifier.verify(RUNTIME_WMODEL)
        self.assertEqual(
            receipt["wmodelSha256"],
            witness["wmodelSha256"],
        )
        self.assertEqual(receipt["submeshCount"], witness["submeshCount"])
        self.assertEqual(
            receipt["cookedVertexCount"], witness["cookedVertexCount"]
        )
        self.assertEqual(
            receipt["sourceTopology"]["nonzeroInfluenceHistogram"][1],
            witness["sourceTopology"]["nonzeroInfluenceHistogram"][1],
        )
        self.assertAlmostEqual(
            witness["bindPose"]["maximumNormalizedIdentityError"],
            receipt["bindPose"]["maximumNormalizedIdentityError"],
            places=12,
        )

        self.assertEqual(
            [row["name"] for row in receipt["animations"]],
            [row["name"] for row in witness["animations"]],
        )
        for expected, actual in zip(
            receipt["animations"], witness["animations"], strict=True
        ):
            self.assertEqual(expected["durationTicks"], actual["durationTicks"])
            self.assertEqual(expected["ticksPerSecond"], actual["ticksPerSecond"])
            self.assertEqual(expected["channelCount"], actual["channelCount"])
            self.assertEqual(
                expected["movingSourceBones"], actual["movingSourceBones"]
            )
            for expected_sample, actual_sample in zip(
                expected["samples"], actual["samples"], strict=True
            ):
                self.assertEqual(expected_sample["timeTicks"], actual_sample["timeTicks"])
                for expected_value, actual_value in zip(
                    expected_sample["bounds"]["minimum"],
                    actual_sample["bounds"]["minimum"],
                    strict=True,
                ):
                    self.assertAlmostEqual(expected_value, actual_value, places=5)
                for expected_value, actual_value in zip(
                    expected_sample["bounds"]["maximum"],
                    actual_sample["bounds"]["maximum"],
                    strict=True,
                ):
                    self.assertAlmostEqual(expected_value, actual_value, places=5)
                self.assertAlmostEqual(
                    expected_sample["bounds"]["diagonal"],
                    actual_sample["bounds"]["diagonal"],
                    places=5,
                )

    def test_action_pose_baked_as_rest_is_rejected(self) -> None:
        model = verifier.read_wmodel(RUNTIME_WMODEL)
        clock = next(
            bone for bone in model.skeleton_bones if bone.name == "b_clock_17"
        )
        clock.transform[12] += 1.0
        with self.assertRaisesRegex(ValueError, "source bind/rest basis mismatch"):
            verifier.verify_bind_pose(model, 1e-3)

    def test_rest_mode_stationary_action_bake_is_rejected(self) -> None:
        model = verifier.read_wmodel(RUNTIME_WMODEL)
        animation = model.animations[0]
        for channel in animation.channels:
            for keys in (
                channel.position_keys,
                channel.rotation_keys,
                channel.scale_keys,
            ):
                if not keys:
                    continue
                source = keys[0]
                keys[:] = [tuple([key[0], *source[1:]]) for key in keys]
        with self.assertRaisesRegex(ValueError, "lost source clock motion"):
            verifier.verify_animation_motion(model, animation)


if __name__ == "__main__":
    unittest.main()
