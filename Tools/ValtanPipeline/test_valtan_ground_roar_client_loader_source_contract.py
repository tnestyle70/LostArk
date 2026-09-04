from __future__ import annotations

import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PATTERN_TREE_CPP = ROOT / "Client" / "Private" / "ValtanPatternTree.cpp"


def _slice(source: str, begin: str, end: str) -> str:
    start = source.index(begin)
    stop = source.index(end, start)
    return source[start:stop]


class ValtanGroundRoarClientLoaderSourceContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        source = PATTERN_TREE_CPP.read_text(encoding="utf-8")
        cls.product_reader = _slice(
            source,
            "bool_t Read_StageActions(",
            "bool_t Read_StageBranches(",
        )
        cls.authoring_projection = _slice(
            source,
            "bool_t Build_SplitEventProjection(",
            "bool_t Parse_SplitMasterDocument(",
        )

    def test_product_reader_keeps_per_alive_player_contract(self) -> None:
        for marker in (
            '"PER_ALIVE_PLAYER" == strTargeting',
            'const bool_t bNoArenaSupplement',
            '"NONE" == pArenaAnchor->Get_String()',
            '0.0 == Value.Find("arenaRandomCount")->Get_Number()',
            'const bool_t bArenaSupplement',
            '"BOSS_SPAWN_POSITION" == pArenaAnchor->Get_String()',
            'Value.Find("arenaRandomCount")->Get_Number() > 0.0',
            'Value.Find("arenaRandomRadiusM")->Get_Number() > 0.0',
            'Value.Find("arenaHeightToleranceM")->Get_Number() > 0.0',
            '(bNoArenaSupplement || bArenaSupplement)',
        ):
            self.assertIn(marker, self.product_reader)

    def test_product_reader_admits_tunable_boss_radial_projection(self) -> None:
        for marker in (
            '"BOSS_RELATIVE" == strTargeting',
            '"ENTER" != Read_String(Value, "trigger")',
            '"RADIAL" == pLayout->Get_String() && fCount >= 2.0',
            "fRadius > 0.0",
            "fAngleStep > 0.0",
            "fAngleStep * fCount <= 360.000001",
            'Value.Find("maximumTotalObjects")->Get_Number() >= fCount',
            '1.0 == Value.Find("spawnCount")->Get_Number()',
            'Value.Find("firstSpawnOffsetMs")->Get_Number()',
            '0.0 == Value.Find("spawnIntervalMs")->Get_Number()',
            '0.0 == Value.Find("arenaRandomCount")->Get_Number()',
            '0.0 == Value.Find("arenaRandomRadiusM")->Get_Number()',
            '0.0 == Value.Find("arenaHeightToleranceM")->Get_Number()',
            '"NONE" == pArenaAnchor->Get_String()',
            "!bPerAlivePlayerContract && !bBossRelativeContract",
        ):
            self.assertIn(marker, self.product_reader)
        for forbidden in ("2.25 == fRadius", "90.0 == fAngleStep"):
            self.assertNotIn(forbidden, self.product_reader)

    def test_authoring_projection_keeps_per_alive_player_contract(self) -> None:
        for marker in (
            '"PER_ALIVE_PLAYER" == strVolleyPolicy',
            '"RADIAL_AROUND_TARGET" == strLayoutKind',
            'const bool_t bNoArenaSupplement',
            'Has_ExactProperties(*pArenaRandom, { "kind" })',
            '"NONE" == Read_String(*pArenaRandom, "kind")',
            'const bool_t bArenaSupplement',
            '"RANDOM_NAVIGABLE_CIRCLE" ==',
            '"BOSS_SPAWN_POSITION" == Read_String(*pArenaRandom, "anchor")',
            "fArenaRandomCount > 0.0",
            'pArenaRandom->Find("radiusM")->Get_Number() > 0.0',
            'pArenaRandom->Find("heightToleranceM")->Get_Number() > 0.0',
            '(bNoArenaSupplement || bArenaSupplement)',
        ):
            self.assertIn(marker, self.authoring_projection)

    def test_authoring_projection_admits_tunable_ground_roar_contract(self) -> None:
        for marker in (
            '"BOSS_RELATIVE" == strVolleyPolicy',
            '"RADIAL_AROUND_BOSS" == strLayoutKind',
            '"angleStepDegrees", "mappingBasis"',
            '"PROJECT_TUNED" == Read_String(*pLayout, "mappingBasis")',
            "fCount < 2.0",
            'pLayout->Find("radiusM")->Get_Number() <= 0.0',
            'pLayout->Find("angleStepDegrees")->Get_Number() <= 0.0',
            'pLayout->Find("angleStepDegrees")->Get_Number() * fCount >',
            'Has_ExactProperties(*pArenaRandom, { "kind" })',
            '"NONE" == Read_String(*pArenaRandom, "kind")',
            '1.0 == pSpawnSchedule->Find("count")->Get_Number()',
            'pSpawnSchedule->Find("firstOffsetMs")->Get_Number() +',
            '0.0 == pSpawnSchedule->Find("intervalMs")->Get_Number()',
            'Action.emplace("firstSpawnOffsetMs"',
            'Event.Find("maximumTotalObjects")->Get_Number() >= fCount',
            "nullptr == pAllowOverlap || pAllowOverlap->Get_Boolean()",
            "!bPerAliveArenaContract && !bBossRelativeContract",
        ):
            self.assertIn(marker, self.authoring_projection)
        for forbidden in (
            "4.0 != fCount",
            '2.25 != pLayout->Find("radiusM")->Get_Number()',
            '90.0 != pLayout->Find("angleStepDegrees")->Get_Number()',
        ):
            self.assertNotIn(forbidden, self.authoring_projection)

    def test_both_readers_reject_unknown_policy_before_projection(self) -> None:
        self.assertIn(
            "(!bPerAlivePlayer && !bBossRelative && !bArenaCenter)",
            self.product_reader,
        )
        self.assertIn(
            "(!bPerAlivePlayer && !bBossRelative && !bArenaCenter)",
            self.authoring_projection,
        )


if __name__ == "__main__":
    unittest.main()
