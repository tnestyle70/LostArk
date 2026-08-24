import json
import unittest
from pathlib import Path


STAGE_FIELDS = {
    "actionDurationMs",
    "hitTimeMs",
    "comboAdvanceMs",
    "inputOpenMs",
    "inputCloseMs",
}


class PlayerSkillCatalogComboTimingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.repository_root = Path(__file__).resolve().parents[2]
        cls.skills = json.loads(
            (cls.repository_root / "Data/Balance/PlayerSkills.json").read_text(
                encoding="utf-8"
            )
        )["skills"]

    def test_all_stage_rows_match_the_published_timing_contract(self) -> None:
        for skill in self.skills:
            stages = skill["comboStages"]
            kind = skill["skillKind"]
            for stage in stages:
                self.assertEqual(set(stage), STAGE_FIELDS, skill["skillId"])
                for field in STAGE_FIELDS:
                    value = stage[field]
                    self.assertIs(type(value), int, (skill["skillId"], field))
                    self.assertGreaterEqual(value, 0)
                    self.assertLessEqual(value, 0xFFFFFFFF)
                self.assertGreater(stage["actionDurationMs"], 0)
                self.assertLessEqual(stage["hitTimeMs"], stage["comboAdvanceMs"])
                self.assertLessEqual(
                    stage["comboAdvanceMs"], stage["actionDurationMs"]
                )

            if kind == "COMBO":
                self.assertGreaterEqual(len(stages), 2)
                self.assertLessEqual(len(stages), 8)
                for stage in stages[:-1]:
                    self.assertLess(stage["inputOpenMs"], stage["inputCloseMs"])
                    self.assertLessEqual(
                        stage["inputCloseMs"], stage["actionDurationMs"]
                    )
                self.assertEqual(
                    stages[-1]["comboAdvanceMs"], stages[-1]["actionDurationMs"]
                )
                self.assertEqual(stages[-1]["inputOpenMs"], 0)
                self.assertEqual(stages[-1]["inputCloseMs"], 0)
            elif kind == "COUNTER":
                self.assertEqual(len(stages), 2)
                guard, counter = stages
                self.assertEqual(guard["hitTimeMs"], 0)
                self.assertLess(guard["inputOpenMs"], guard["inputCloseMs"])
                self.assertLessEqual(
                    guard["inputCloseMs"], guard["actionDurationMs"]
                )
                self.assertGreater(counter["hitTimeMs"], 0)
                self.assertEqual(counter["inputOpenMs"], 0)
                self.assertEqual(counter["inputCloseMs"], 0)
            elif kind == "HOLD":
                self.assertEqual(len(stages), 3)
                self.assertTrue(all(stage["inputOpenMs"] == 0 for stage in stages))
                self.assertTrue(all(stage["inputCloseMs"] == 0 for stage in stages))
                self.assertTrue(all(stage["hitTimeMs"] == 0 for stage in stages[:-1]))
                self.assertGreater(stages[-1]["hitTimeMs"], 0)
            else:
                self.assertEqual(stages, [], skill["skillId"])

    def test_client_catalog_preserves_each_stage_before_atomic_commit(self) -> None:
        header = (
            self.repository_root / "Client/Public/PlayerSkillCatalog.h"
        ).read_text(encoding="utf-8")
        source = (
            self.repository_root / "Client/Private/PlayerSkillCatalog.cpp"
        ).read_text(encoding="utf-8")

        self.assertIn("struct PLAYER_COMBO_STAGE_TIMING", header)
        for member in (
            "iActionDurationMs",
            "iHitTimeMs",
            "iComboAdvanceMs",
            "iInputOpenMs",
            "iInputCloseMs",
        ):
            self.assertIn(member, header)
        self.assertIn(
            "std::vector<PLAYER_COMBO_STAGE_TIMING> ComboStages;", header
        )

        parse_at = source.index("definition.ComboStages.push_back(stage);")
        stage_at = source.index("skills.push_back(std::move(definition));")
        commit_at = source.index("g_Skills = std::move(skills);")
        self.assertLess(parse_at, stage_at)
        self.assertLess(stage_at, commit_at)


if __name__ == "__main__":
    unittest.main()
