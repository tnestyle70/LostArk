#!/usr/bin/env python3

import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import build_valtan_rootmotion as rootmotion


class ExplicitClipSegmentTests(unittest.TestCase):
    def setUp(self) -> None:
        # One raw metre over one second after UNIT_TO_METRES is applied.
        self.curve = (
            1000.0,
            1000.0,
            [
                (0.0, 0.0, 0.0, 0.0),
                (1000.0, 100.0, 0.0, 0.0),
            ],
        )
        self.curves = {
            "first": self.curve,
            "second": self.curve,
            "third": self.curve,
        }

    def test_explicit_slices_restart_source_and_accumulate_wall_segments(self) -> None:
        clips = [
            {
                "clip": "first",
                "sourceStartMs": 0,
                "playMs": 200,
                "playRate": 1.0,
                "loop": False,
            },
            {
                "clip": "second",
                "sourceStartMs": 100,
                "playMs": 400,
                "playRate": 2.0,
                "loop": False,
            },
            {
                "clip": "third",
                "sourceStartMs": 0,
                "playMs": 600,
                "playRate": 1.0,
                "loop": False,
            },
        ]

        segments = rootmotion.build_explicit_clip_segments(clips, self.curves)

        self.assertIsNotNone(segments)
        assert segments is not None
        self.assertEqual([200.0, 200.0, 600.0], [
            segment.wall_duration_ms for segment in segments])
        expected_forward = {
            0.0: 0.0,
            199.0: 0.199,
            200.0: 0.2,
            300.0: 0.4,
            400.0: 0.6,
            1000.0: 1.2,
            1200.0: 1.2,
        }
        for time_ms, expected in expected_forward.items():
            with self.subTest(time_ms=time_ms):
                forward, lateral = rootmotion.sample_explicit_clip_segments(
                    segments, time_ms)
                self.assertAlmostEqual(expected, forward, places=6)
                self.assertEqual(0.0, lateral)

    def test_natural_multi_clip_chain_keeps_legacy_bake_path(self) -> None:
        clips = [
            {
                "clip": "first",
                "sourceStartMs": 0,
                "playMs": 0,
                "playRate": 1.0,
                "loop": False,
            },
            {
                "clip": "second",
                "sourceStartMs": 0,
                "playMs": 400,
                "playRate": 1.0,
                "loop": False,
            },
        ]

        self.assertIsNone(
            rootmotion.build_explicit_clip_segments(clips, self.curves))

    def test_build_preserves_explicit_chain_and_skips_portal_transform(self) -> None:
        returning_curve = (
            1000.0,
            1000.0,
            [
                (0.0, 0.0, 0.0, 0.0),
                (1000.0, -100.0, 0.0, 0.0),
            ],
        )
        static_curve = (
            1000.0,
            1000.0,
            [
                (0.0, 0.0, 0.0, 0.0),
                (1000.0, 0.0, 0.0, 0.0),
            ],
        )
        single_returning_curve = (
            1000.0,
            1000.0,
            [
                (0.0, 0.0, 0.0, 0.0),
                (500.0, 100.0, 0.0, 0.0),
                (1000.0, 0.0, 0.0, 0.0),
            ],
        )
        encounter = {
            "bossArchetypeId": "BOSS_TEST",
            "patterns": [
                {
                    "patternId": "TEST_RETURNING_CHAIN",
                    "stages": [
                        {
                            "stageId": "ACTIVE",
                            "actionId": "test.returning-chain.active",
                            "durationMs": 2000,
                        },
                    ],
                },
                {
                    "patternId": "TEST_STATIC_CHAIN",
                    "stages": [
                        {
                            "stageId": "ACTIVE",
                            "actionId": "test.static-chain.active",
                            "durationMs": 2000,
                        },
                    ],
                },
                {
                    "patternId": "TEST_SINGLE_RETURNING_CLIP",
                    "stages": [
                        {
                            "stageId": "ACTIVE",
                            "actionId": "test.single-returning-clip.active",
                            "durationMs": 1000,
                        },
                    ],
                },
                {
                    "patternId": "TEST_PORTAL",
                    "stages": [
                        {
                            "stageId": "ACTIVE",
                            "actionId": "test.portal.active",
                            "durationMs": 1000,
                            "motion": {
                                "kind": "PORTAL_CROSS_ARENA",
                                "cornerIndex": 0,
                                "halfExtentsM": [22.0, 22.0],
                            },
                        },
                    ],
                },
                {
                    "patternId": "TEST_PORTAL_TARGET",
                    "stages": [
                        {
                            "stageId": "ACTIVE",
                            "actionId": "test.portal-target.active",
                            "durationMs": 1000,
                            "motion": {"kind": "PORTAL_TARGET_RUSH"},
                        },
                    ],
                },
            ],
        }
        bindings = {
            "bindings": [
                {
                    "actionId": "test.returning-chain.active",
                    "clips": [
                        {
                            "clip": "first",
                            "sourceStartMs": 0,
                            "playMs": 1000,
                            "playRate": 1.0,
                            "loop": False,
                        },
                        {
                            "clip": "returning",
                            "sourceStartMs": 0,
                            "playMs": 1000,
                            "playRate": 1.0,
                            "loop": False,
                        },
                    ],
                },
                {
                    "actionId": "test.static-chain.active",
                    "clips": [
                        {
                            "clip": "static",
                            "sourceStartMs": 0,
                            "playMs": 1000,
                            "playRate": 1.0,
                            "loop": False,
                        },
                        {
                            "clip": "static",
                            "sourceStartMs": 0,
                            "playMs": 1000,
                            "playRate": 1.0,
                            "loop": False,
                        },
                    ],
                },
                {
                    "actionId": "test.single-returning-clip.active",
                    "clips": [
                        {
                            "clip": "single-returning",
                            "sourceStartMs": 0,
                            "playMs": 1000,
                            "playRate": 1.0,
                            "loop": False,
                        },
                    ],
                },
                {
                    "actionId": "test.portal.active",
                    "clips": [{
                        "clip": "first", "sourceStartMs": 0,
                        "playMs": 1000, "playRate": 1.0, "loop": False,
                    }],
                },
                {
                    "actionId": "test.portal-target.active",
                    "clips": [{
                        "clip": "first", "sourceStartMs": 0,
                        "playMs": 1000, "playRate": 1.0, "loop": False,
                    }],
                },
            ],
        }

        with tempfile.TemporaryDirectory() as temporary_directory:
            repo_root = Path(temporary_directory)
            curves = {
                "first": self.curve,
                "returning": returning_curve,
                "static": static_curve,
                "single-returning": single_returning_curve,
            }
            document, notes = rootmotion.build(
                repo_root,
                encounter_document=encounter,
                bindings_document=bindings,
                curves=curves,
            )

        self.assertEqual([
            "TEST_PORTAL/ACTIVE: kept portal transform motion",
        ], notes)
        self.assertEqual(2, len(document["patterns"]))
        pattern_ids = [pattern["patternId"] for pattern in document["patterns"]]
        self.assertEqual(
            "TEST_RETURNING_CHAIN", document["patterns"][0]["patternId"])
        self.assertNotIn("TEST_STATIC_CHAIN", pattern_ids)
        self.assertNotIn("TEST_SINGLE_RETURNING_CLIP", pattern_ids)
        self.assertNotIn("TEST_PORTAL", pattern_ids)
        self.assertIn("TEST_PORTAL_TARGET", pattern_ids)
        stage = document["patterns"][0]["stages"][0]
        self.assertAlmostEqual(0.0, stage["samples"][-1]["forward"], places=6)
        self.assertGreater(
            max(abs(sample["forward"]) for sample in stage["samples"]),
            rootmotion.MINIMUM_TRAVEL_METRES,
        )
        target = next(pattern for pattern in document["patterns"]
                      if pattern["patternId"] == "TEST_PORTAL_TARGET")
        self.assertAlmostEqual(
            1.0, target["stages"][0]["samples"][-1]["forward"], places=6)

    def test_current_explicit_multi_clip_migrations_are_allowlisted(self) -> None:
        repo_root = Path(__file__).resolve().parents[2]
        document = json.loads((
            repo_root /
            "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
        ).read_text(encoding="utf-8"))

        explicit_multi_actions = [
            binding["actionId"]
            for binding in document["bindings"]
            if len(binding["clips"]) > 1 and all(
                float(clip.get("playMs", 0)) > 0.0
                for clip in binding["clips"])
        ]

        self.assertEqual([
            "valtan.attack.dash-charge.windup",
            "valtan.attack.dash-charge.part-break",
            "valtan.attack.dash-charge.recovery",
            "valtan.reactive.triple-counter.first",
            "valtan.reactive.triple-counter.second",
            "valtan.reactive.triple-counter.third",
            "valtan.mechanic.floor-wipe-130.interval",
            "valtan.mechanic.arena-break-109.drop",
            "valtan.mechanic.arena-break-109.impact-hold",
            "valtan.mechanic.arena-break-109.wide-reveal",
            "valtan.sequence.counter.step-02",
            "valtan.sequence.center-trash-rush-if.groggy",
            "valtan.sequence.rush-if.groggy",
            "valtan.sequence.sequence.400440.0.step-01",
            "valtan.authoring.bind-slot.step-01",
            "valtan.followup.groggy.active",
            "valtan.reaction.part-break.recovery",
        ], explicit_multi_actions)


class CurrentValtanRootMotionClosureTests(unittest.TestCase):
    def test_navigation_blocked_capture_stages_have_server_motion(self) -> None:
        repo_root = Path(__file__).resolve().parents[2]
        encounter = json.loads((
            repo_root / "Data/Encounters/Valtan/ValtanEncounter.json"
        ).read_text(encoding="utf-8"))
        root_motion = json.loads((
            repo_root / "Data/Animation/RootMotion/Valtan.rootmotion.json"
        ).read_text(encoding="utf-8"))

        baked_stages = {
            (pattern["patternId"], stage["stageId"]): stage
            for pattern in root_motion["patterns"]
            for stage in pattern["stages"]
        }
        missing = []
        checked = []
        for pattern in encounter["patterns"]:
            for stage_index, stage in enumerate(pattern["stages"]):
                outcomes = {
                    branch["outcome"] for branch in stage.get("branches", [])
                }
                if "NAVIGATION_BLOCKED" not in outcomes:
                    continue
                if stage.get("playerResponse") != "CAPTURE":
                    continue
                checked.append((pattern["patternId"], stage["stageId"]))
                has_forward_motion = (
                    stage.get("motion", {}).get("kind") == "FORWARD"
                )
                baked = baked_stages.get(
                    (pattern["patternId"], stage["stageId"])
                )
                if not has_forward_motion and baked is None:
                    missing.append((pattern["patternId"], stage["stageId"]))
                    continue
                if has_forward_motion:
                    continue
                self.assertEqual(stage_index, int(baked["stageIndex"]))
                self.assertEqual("mesh_att_battle_13_04", baked["clip"])
                self.assertEqual(667, int(baked["durationMs"]))
                self.assertEqual(22, len(baked["samples"]))
                self.assertAlmostEqual(
                    7.4608, float(baked["samples"][-1]["forward"]), places=4
                )

        self.assertEqual(6, len(checked))
        self.assertEqual([], missing)


if __name__ == "__main__":
    unittest.main()
