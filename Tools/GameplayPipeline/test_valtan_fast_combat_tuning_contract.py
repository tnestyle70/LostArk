from __future__ import annotations

import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PLAYER_PROFILES = ROOT / "Data/Balance/PlayerProfiles.json"
PLAYER_SKILLS = ROOT / "Data/Balance/PlayerSkills.json"
DAMAGE_PROFILES = ROOT / "Data/Balance/DamageProfiles.json"
BOSS_PROFILES = ROOT / "Data/Balance/BossProfiles.json"

EXPECTED_CLASSES = {
    "LANCE_MASTER",
    "GUNSLINGER",
    "SLAYER",
    "ARTIST",
    "DIMENSIONMASTER",
    "WARLORD",
}


def load(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


class ValtanFastCombatTuningContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.players = load(PLAYER_PROFILES)["players"]
        cls.skills = load(PLAYER_SKILLS)["skills"]
        cls.damage_profiles = {
            row["damageProfileId"]: row["damageRatePercent"]
            for row in load(DAMAGE_PROFILES)["profiles"]
        }
        cls.bosses = {
            row["archetypeId"]: row
            for row in load(BOSS_PROFILES)["bosses"]
        }

    def test_primary_valtan_hp_is_tenfold_without_scaling_the_ghost(self) -> None:
        self.assertEqual(600000, self.bosses["BOSS_VALTAN"]["maximumHp"])
        self.assertEqual(160, self.bosses["BOSS_VALTAN"]["maximumHealthBars"])
        self.assertEqual(60000, self.bosses["BOSS_VALTAN_GHOST"]["maximumHp"])

    def test_every_non_basic_attack_has_a_three_second_cooldown(self) -> None:
        non_basic_attacks = [
            row for row in self.skills if row["inputSlot"] != "LMB"
        ]
        self.assertEqual(87, len(non_basic_attacks))
        self.assertTrue(
            all(row["cooldownMs"] == 3000 for row in non_basic_attacks)
        )

        basic_attacks = [
            row for row in self.skills if row["inputSlot"] == "LMB"
        ]
        self.assertEqual(7, len(basic_attacks))
        self.assertTrue(all(row["cooldownMs"] == 0 for row in basic_attacks))

    def test_player_attack_power_is_one_thousand(self) -> None:
        self.assertEqual(
            EXPECTED_CLASSES,
            {row["characterClass"] for row in self.players},
        )
        self.assertTrue(all(row["attackPower"] == 1000 for row in self.players))

        for skill in self.skills:
            damage_profile_id = skill["serverDamageProfileId"]
            if not damage_profile_id:
                continue
            rate = self.damage_profiles[damage_profile_id]
            tuned_damage = 1000 * rate // 100
            self.assertEqual(rate * 10, tuned_damage)


if __name__ == "__main__":
    unittest.main()
