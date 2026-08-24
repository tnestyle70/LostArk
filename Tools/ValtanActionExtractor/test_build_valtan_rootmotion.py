#!/usr/bin/env python3

import json
import sys
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

    def test_current_explicit_multi_clip_migration_is_dash_windup_only(self) -> None:
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

        self.assertEqual(
            ["valtan.attack.dash-charge.windup"], explicit_multi_actions)


if __name__ == "__main__":
    unittest.main()
