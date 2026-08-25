import json
import re
import unittest
from pathlib import Path


class DimensionMaster2050010AutomaticSequenceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.repository_root = Path(__file__).resolve().parents[2]

    def load_json(self, relative_path: str) -> dict:
        return json.loads(
            (self.repository_root / relative_path).read_text(encoding="utf-8")
        )

    def load_effect(self, stage: int) -> dict:
        return self.load_json(
            "Data/Effects/Authored/"
            f"effect.dimensionmaster.skill.2050010.ba{stage}.unified.effect.json"
        )

    def test_ba_sequence_groups_the_double_thrust_then_swing(self) -> None:
        document = self.load_json(
            "Data/Animation/Authored/DimensionMaster/"
            "DimensionMaster.skillbindings.json"
        )
        binding = next(
            row for row in document["bindings"] if row["skillId"] == 2050010
        )

        self.assertEqual(
            binding["clips"],
            [
                [
                    {
                        "clip": "pc_sp_m_00_sk_att_battle_1_01",
                        "playMs": 1800,
                        "playRate": 2.0,
                    }
                ],
                ["pc_sp_m_00_sk_att_battle_1_03"],
            ],
        )
        self.assertEqual(
            binding["clips"][0][0]["playMs"]
            / binding["clips"][0][0]["playRate"],
            900.0,
        )

        ba2 = self.load_effect(2)
        visible_start_wall_seconds = [
            element["detail"]["timing"]["startDelaySeconds"]
            / binding["clips"][0][0]["playRate"]
            for element in ba2["elements"]
            if element["visible"]
        ]
        self.assertEqual(len(visible_start_wall_seconds), 2)
        self.assertLess(max(visible_start_wall_seconds), 0.9)

    def test_server_stages_advance_automatically_after_each_full_motion(self) -> None:
        document = self.load_json("Data/Balance/PlayerSkills.json")
        skill = next(row for row in document["skills"] if row["skillId"] == 2050010)
        stages = skill["comboStages"]

        self.assertEqual(skill["actionDurationMs"], 900)
        self.assertEqual(skill["hitTimeMs"], 50)
        self.assertEqual(
            [stage["actionDurationMs"] for stage in stages],
            [900, 1067],
        )
        self.assertEqual(
            [stage["hitTimeMs"] for stage in stages],
            [50, 28],
        )
        self.assertEqual(
            [stage["comboAdvanceMs"] for stage in stages],
            [900, 1067],
        )
        self.assertEqual(
            [stage["inputOpenMs"] for stage in stages],
            [0, 0],
        )
        self.assertEqual(
            [stage["inputCloseMs"] for stage in stages],
            [0, 0],
        )
        self.assertTrue(
            all(
                stage["comboAdvanceMs"] == stage["actionDurationMs"]
                for stage in stages
            )
        )

    def test_product_effects_follow_the_confirmed_two_animation_groups(self) -> None:
        event_path = (
            self.repository_root
            / "Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents"
        )
        marker = 'payload="effect.dimensionmaster.skill.2050010.ba'
        cue_pattern = re.compile(
            r'^"([^"]+)" EFFECT startms=(\d+) payload="([^"]+)" '
            r'effectref=asset anchor="root" follow=follow '
            r'orientation=action_facing stop=natural '
        )
        cues = []
        for line in event_path.read_text(encoding="utf-8").splitlines():
            if marker not in line:
                continue
            match = cue_pattern.match(line)
            self.assertIsNotNone(match, line)
            cues.append((match.group(1), int(match.group(2)), match.group(3)))

        self.assertEqual(
            cues,
            [
                (
                    "pc_sp_m_00_sk_att_battle_1_01",
                    0,
                    "effect.dimensionmaster.skill.2050010.ba2.unified",
                ),
                (
                    "pc_sp_m_00_sk_att_battle_1_03",
                    0,
                    "effect.dimensionmaster.skill.2050010.ba3.unified",
                ),
            ],
        )

    def test_authored_ba_assets_are_catalogued_without_pinning_payloads(self) -> None:
        catalog = self.load_json("Data/Effects/EffectCatalog.json")
        rows = {row["effectAssetId"]: row for row in catalog["effects"]}

        for stage in range(1, 5):
            effect_asset_id = (
                f"effect.dimensionmaster.skill.2050010.ba{stage}.unified"
            )
            authoring_path = (
                "Effects/Authored/"
                f"effect.dimensionmaster.skill.2050010.ba{stage}.unified.effect.json"
            )
            self.assertIn(effect_asset_id, rows)
            self.assertEqual(rows[effect_asset_id]["payloadKind"],
                             "DIRECT_AUTHORED_DOCUMENT_V13")
            self.assertEqual(rows[effect_asset_id]["authoringPath"], authoring_path)

            effect = self.load_effect(stage)
            self.assertEqual(effect["schema"], "lostark.effect-authoring")
            self.assertEqual(effect["version"], 13)
            self.assertEqual(effect["effectAssetId"], effect_asset_id)

    def test_root_motion_matches_full_stage_wall_durations(self) -> None:
        document = self.load_json(
            "Data/Animation/RootMotion/DimensionMaster.rootmotion.json"
        )
        skill = next(
            row for row in document["skills"] if row["skillId"] == 2050010
        )

        self.assertEqual(
            [stage["durationMs"] for stage in skill["stages"]],
            [900, 1067],
        )
        expected_end_forward = [0.9298, 0.2802]
        expected_sample_counts = [55, 33]
        for stage_index, (stage, expected_forward, expected_count) in enumerate(
            zip(skill["stages"], expected_end_forward, expected_sample_counts)
        ):
            self.assertEqual(stage["stageIndex"], stage_index)
            self.assertEqual(len(stage["samples"]), expected_count)
            times = [sample["timeMs"] for sample in stage["samples"]]
            self.assertEqual(times[0], 0)
            self.assertEqual(times[-1], stage["durationMs"])
            self.assertTrue(all(left < right for left, right in zip(times, times[1:])))
            self.assertAlmostEqual(
                stage["samples"][-1]["forward"], expected_forward
            )


if __name__ == "__main__":
    unittest.main()
