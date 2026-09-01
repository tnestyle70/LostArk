import json
import math
import unittest
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
GAMEPLAY_PATH = REPOSITORY_ROOT / "Data/Valtan/Valtan.gameplay.json"
COMBAT_OBJECTS_PATH = REPOSITORY_ROOT / "Data/Valtan/Valtan.combatobjects.json"
BOSS_CATALOG_PATH = REPOSITORY_ROOT / "Data/Actors/BossCatalog.json"
GAME_ROOM_PATH = REPOSITORY_ROOT / "Server/Private/GameRoom.cpp"
COMBAT_RUNTIME_PATH = REPOSITORY_ROOT / "Server/Private/CombatObjectRuntime.cpp"


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


class ValtanPhase3PrimaryGhostLoopContractTests(unittest.TestCase):
    def test_respawn_enters_phase_three_and_finale_owns_exact_primary_loop(self) -> None:
        gameplay = load_json(GAMEPLAY_PATH)
        patterns = {row["patternId"]: row for row in gameplay["patterns"]}
        respawn = patterns["VALTAN_GHOST_RESPAWN_AUDITION"]
        phase_events = [
            event
            for stage in respawn["stages"]
            for event in stage["events"]
            if event["kind"] == "SET_GAMEPLAY_PHASE"
        ]
        self.assertEqual(
            [("STEP_01", 3)],
            [
                (stage["stageId"], event["gameplayPhase"])
                for stage in respawn["stages"]
                for event in stage["events"]
                if event["kind"] == "SET_GAMEPLAY_PHASE"
            ],
        )
        self.assertEqual(1, len(phase_events))
        self.assertEqual(
            [
                "VALTAN_SIX_PIZZA_106",
                "VALTAN_GROUND_ROAR",
                "VALTAN_STAGGER_SLOT",
                "VALTAN_BIND_SLOT",
                "VALTAN_SILENCE_SLOT",
                "VALTAN_TRIPLE_COUNTER",
            ],
            patterns["VALTAN_GHOST_FINALE"]["finale"]["ghostPatternIds"],
        )

    def test_portal_once_is_one_atomic_four_corner_inward_volley(self) -> None:
        gameplay = load_json(GAMEPLAY_PATH)
        portal = next(
            row
            for row in gameplay["patterns"]
            if row["patternId"] == "VALTAN_GHOST_PORTAL_ONCE"
        )
        self.assertEqual("NONE", portal["targetPolicy"])
        self.assertEqual("NONE", portal["aimPolicy"])
        self.assertEqual(1, len(portal["stages"]))
        event = portal["stages"][0]["events"][0]
        self.assertEqual("SPAWN_COMBAT_OBJECT_VOLLEY", event["kind"])
        self.assertEqual("BOSS_RELATIVE", event["volleyPolicy"])
        self.assertEqual(4, event["countPerResolvedTarget"])
        self.assertEqual(4, event["maximumTotalObjects"])
        self.assertFalse(event["allowOverlap"])
        self.assertEqual(
            {
                "kind": "INTERVAL",
                "count": 1,
                "firstOffsetMs": 0,
                "intervalMs": 0,
            },
            event["spawnSchedule"],
        )
        layout = event["layout"]
        self.assertEqual("RADIAL_AROUND_BOSS", layout["kind"])
        self.assertAlmostEqual(math.sqrt(22.0**2 + 22.0**2), layout["radiusM"], places=5)
        self.assertEqual(45.0, layout["startAngleDegrees"])
        self.assertEqual(90.0, layout["angleStepDegrees"])

        objects = {
            row["combatObjectArchetypeId"]: row
            for row in load_json(COMBAT_OBJECTS_PATH)["objects"]
        }
        charge = objects["combatobject.valtan.ghost.portal-charge"]
        self.assertEqual("MISSILE", charge["kind"])
        self.assertEqual(5000, charge["lifetimeMs"])
        self.assertEqual("RADIAL_INWARD", charge["spawn"]["direction"]["kind"])
        self.assertEqual("LINEAR", charge["movement"]["kind"])
        self.assertAlmostEqual(62.2254, charge["movement"]["maximumDistanceM"], places=4)

    def test_primary_valtan_reuses_portal_effect_mapping(self) -> None:
        bosses = {
            row["archetypeId"]: row
            for row in load_json(BOSS_CATALOG_PATH)["bosses"]
        }
        mappings = {
            row["combatObjectArchetypeId"]: row["effectAssetId"]
            for row in bosses["BOSS_VALTAN"]["combatObjectVisuals"]
        }
        self.assertEqual(
            "effect.valtan.project-tuned.sequence.warp.portal",
            mappings["combatobject.valtan.ghost.portal-charge"],
        )

    def test_server_uses_primary_identity_loop_and_five_second_auxiliary_clock(self) -> None:
        room = GAME_ROOM_PATH.read_text(encoding="utf-8")
        runtime = COMBAT_RUNTIME_PATH.read_text(encoding="utf-8")
        self.assertIn("Activate_ValtanGhostPhaseLoop", room)
        self.assertIn("boss.GhostPhasePatternSequence", room)
        self.assertIn("boss.bGhostPhasePatternLoopActive = true", room)
        self.assertIn("entity.iRotationStepIndex = 0u", room)
        self.assertIn("5u * SERVER_TICK_HZ", room)
        self.assertIn("Update_ValtanGhostPortalScheduler", room)
        self.assertIn("synthetic.fPositionX = boss.fSpawnPositionX", room)
        self.assertIn("synthetic.fPositionZ = boss.fSpawnPositionZ", room)
        self.assertIn("owner.bGhostPhasePatternLoopActive", room)
        self.assertIn("BOSS_COMBAT_OBJECT_DIRECTION_POLICY::RADIAL_INWARD", runtime)
        self.assertIn("-radialDirectionX", runtime)
        self.assertIn("-radialDirectionZ", runtime)


if __name__ == "__main__":
    unittest.main()
