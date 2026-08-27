#!/usr/bin/env python3

import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

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

    def test_build_preserves_explicit_chain_with_large_intermediate_excursion(self) -> None:
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
            ],
        }

        with tempfile.TemporaryDirectory() as temporary_directory:
            repo_root = Path(temporary_directory)
            encounter_path = (
                repo_root / "Data/Encounters/Valtan/ValtanEncounter.json")
            bindings_path = (
                repo_root / "Data/Animation/Authored/Valtan" /
                "Valtan.patternbindings.json")
            encounter_path.parent.mkdir(parents=True)
            bindings_path.parent.mkdir(parents=True)
            encounter_path.write_text(json.dumps(encounter), encoding="utf-8")
            bindings_path.write_text(json.dumps(bindings), encoding="utf-8")

            curves = {
                "first": self.curve,
                "returning": returning_curve,
                "static": static_curve,
                "single-returning": single_returning_curve,
            }
            with mock.patch.object(
                    rootmotion, "read_root_curves", return_value=curves):
                document, notes = rootmotion.build(
                    repo_root, repo_root / "Resources")

        self.assertEqual([], notes)
        self.assertEqual(1, len(document["patterns"]))
        pattern_ids = [pattern["patternId"] for pattern in document["patterns"]]
        self.assertEqual(
            "TEST_RETURNING_CHAIN", document["patterns"][0]["patternId"])
        self.assertNotIn("TEST_STATIC_CHAIN", pattern_ids)
        self.assertNotIn("TEST_SINGLE_RETURNING_CLIP", pattern_ids)
        stage = document["patterns"][0]["stages"][0]
        self.assertAlmostEqual(0.0, stage["samples"][-1]["forward"], places=6)
        self.assertGreater(
            max(abs(sample["forward"]) for sample in stage["samples"]),
            rootmotion.MINIMUM_TRAVEL_METRES,
        )

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
            "valtan.mechanic.floor-wipe-130.interval",
            "valtan.mechanic.arena-break-109.drop",
            "valtan.mechanic.arena-break-109.impact-hold",
            "valtan.mechanic.arena-break-109.wide-reveal",
        ], explicit_multi_actions)


if __name__ == "__main__":
    unittest.main()
