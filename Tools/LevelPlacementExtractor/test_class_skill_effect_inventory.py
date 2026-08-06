#!/usr/bin/env python3

from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from build_class_skill_effect_inventory import build_inventory


class ClassSkillEffectInventoryTests(unittest.TestCase):
    def write_inputs(self, root: Path, *, duplicate: bool = False) -> tuple[Path, Path]:
        bindings = root / "bindings.json"
        animnotify = root / "source.animnotify"
        rows = [
            {"skillId": 10, "clips": ["clip_a", "clip_b"]},
            {"skillId": 10 if duplicate else 20, "clips": ["clip_c"]},
        ]
        bindings.write_text(
            json.dumps(
                {
                    "animationAssetId": "Test",
                    "characterClass": "TEST",
                    "bindings": rows,
                }
            ),
            encoding="utf-8",
        )
        animnotify.write_text(
            '\n'.join(
                [
                    '"clip_a" skill=10 len=1.0000 name="A"',
                    '  n t=0.2500 d=0.0000 kind=EFFECT src=PlayParticleEffect asset="FX_A.Par_A" label="" win=NONE',
                    '"clip_b" skill=10 len=2.0000 name="B"',
                    '  n t=0.5000 d=0.1000 kind=EFFECT src=TrailGhostEffect asset="" label="" win=NONE',
                    '"clip_c" skill=20 len=0.5000 name="C"',
                    '  n t=0.1000 d=0.2000 kind=SHAKE src=ViewShake asset="" label="" win=NONE',
                ]
            ) + '\n',
            encoding="utf-8",
        )
        return bindings, animnotify

    def test_builds_global_timeline_and_exact_package_inventory(self) -> None:
        with tempfile.TemporaryDirectory() as raw_root:
            bindings, animnotify = self.write_inputs(Path(raw_root))
            result = build_inventory(bindings, animnotify)

        self.assertEqual(2, result["summary"]["skillCount"])
        self.assertEqual(["FX_A"], result["logicalPackages"])
        self.assertEqual(["FX_A.Par_A"], result["particleSystems"])
        self.assertEqual(3.0, result["skills"][0]["durationSeconds"])
        self.assertEqual(
            "UNSUPPORTED_SOURCE_NOTIFY",
            result["skills"][0]["sourceEvents"][1]["extractionStatus"],
        )
        self.assertFalse(result["ownership"]["reconstructCombinedMeshAsEffectElement"])

    def test_missing_bound_clip_fails(self) -> None:
        with tempfile.TemporaryDirectory() as raw_root:
            bindings, animnotify = self.write_inputs(Path(raw_root))
            document = json.loads(bindings.read_text(encoding="utf-8"))
            document["bindings"][0]["clips"] = ["absent"]
            bindings.write_text(json.dumps(document), encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "absent from animnotify"):
                build_inventory(bindings, animnotify)

    def test_duplicate_skill_fails(self) -> None:
        with tempfile.TemporaryDirectory() as raw_root:
            bindings, animnotify = self.write_inputs(Path(raw_root), duplicate=True)
            with self.assertRaisesRegex(ValueError, "duplicate skillId"):
                build_inventory(bindings, animnotify)


if __name__ == "__main__":
    unittest.main()
