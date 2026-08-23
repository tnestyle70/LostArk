import json
import unittest
from pathlib import Path


class DimensionMaster2050010StageSplitTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.repository_root = Path(__file__).resolve().parents[2]

    def load_json(self, relative_path: str) -> dict:
        return json.loads(
            (self.repository_root / relative_path).read_text(encoding="utf-8")
        )

    def test_ba1_and_ba2_keep_only_their_intended_visual_occurrences(self) -> None:
        ba1 = self.load_json(
            "Data/Effects/Authored/"
            "effect.dimensionmaster.skill.2050010.ba1.unified.effect.json"
        )
        ba2 = self.load_json(
            "Data/Effects/Authored/"
            "effect.dimensionmaster.skill.2050010.ba2.unified.effect.json"
        )

        self.assertEqual(
            [element["id"] for element in ba1["elements"]],
            ["authored.source-particle.16456efaedcffa790f2fb3ee"],
        )
        self.assertAlmostEqual(
            ba1["elements"][0]["detail"]["timing"]["startDelaySeconds"],
            0.2,
        )

        self.assertEqual(
            [element["id"] for element in ba2["elements"]],
            [
                "authored.source-particle.2dfd37aef91a841b74d3298c",
                "authored.source-particle.a11249aa4c7265b3c8a60357",
                "authored.source-particle.2518c186ad5c92b0247e344c",
                "authored.source-particle.7823110ed221a2329cb06536",
                "authored.source-particle.58c2cafaa13ac2ddea5fa5ab",
                "authored.source-particle.c1fe3322c2f504747145816b",
                "authored.source-particle.6efeefc6e80a915b806ce89c",
                "authored.source-particle.13024baa3f1d365c06d69d32",
            ],
        )
        self.assertTrue(
            all(
                abs(
                    element["detail"]["timing"]["startDelaySeconds"] - 0.1
                )
                < 1e-6
                for element in ba2["elements"]
            )
        )
        self.assertTrue(
            all(
                "normalatk_signal" not in element["sourceNode"].lower()
                for element in ba2["elements"]
            )
        )

    def test_root_motion_windows_match_the_four_server_stage_durations(self) -> None:
        document = self.load_json(
            "Data/Animation/RootMotion/DimensionMaster.rootmotion.json"
        )
        skill = next(
            row for row in document["skills"] if row["skillId"] == 2050010
        )

        self.assertEqual(
            [stage["durationMs"] for stage in skill["stages"]],
            [276, 269, 494, 1267],
        )
        expected_end_forward = [0.1055, 0.3031, 0.2802, 1.0404]
        for stage_index, (stage, expected_forward) in enumerate(
            zip(skill["stages"], expected_end_forward)
        ):
            self.assertEqual(stage["stageIndex"], stage_index)
            times = [sample["timeMs"] for sample in stage["samples"]]
            self.assertEqual(times[0], 0)
            self.assertEqual(times[-1], stage["durationMs"])
            self.assertTrue(all(left < right for left, right in zip(times, times[1:])))
            self.assertAlmostEqual(
                stage["samples"][-1]["forward"], expected_forward
            )

        self.assertAlmostEqual(
            skill["stages"][0]["samples"][-1]["forward"]
            + skill["stages"][1]["samples"][-1]["forward"],
            0.4086,
        )


if __name__ == "__main__":
    unittest.main()
