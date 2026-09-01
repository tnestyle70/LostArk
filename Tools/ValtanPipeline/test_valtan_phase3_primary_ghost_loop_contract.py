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
PACKET_HEADER_PATH = REPOSITORY_ROOT / "Shared/Public/Network/PacketMessages.h"
PACKET_RUNTIME_PATH = REPOSITORY_ROOT / "Shared/Private/Network/PacketMessages.cpp"
VALTAN_HEADER_PATH = REPOSITORY_ROOT / "Client/Public/Valtan.h"
VALTAN_RUNTIME_PATH = REPOSITORY_ROOT / "Client/Private/Valtan.cpp"
SERVER_WORLD_ENTITY_PATH = REPOSITORY_ROOT / "Server/Public/ServerWorldEntity.h"


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

    def test_portal_once_is_one_atomic_four_corner_edge_volley(self) -> None:
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
        self.assertEqual(
            "NEXT_RADIAL_SLOT", charge["spawn"]["direction"]["kind"]
        )
        self.assertEqual("LINEAR", charge["movement"]["kind"])
        self.assertAlmostEqual(8.8, charge["movement"]["speedMps"], places=4)
        self.assertAlmostEqual(44.0, charge["movement"]["maximumDistanceM"], places=4)

        corners = []
        for ordinal in range(4):
            radians = math.radians(
                layout["startAngleDegrees"] + layout["angleStepDegrees"] * ordinal
            )
            corners.append(
                (
                    math.sin(radians) * layout["radiusM"],
                    math.cos(radians) * layout["radiusM"],
                )
            )
        for ordinal, start in enumerate(corners):
            end = corners[(ordinal + 1) % len(corners)]
            self.assertAlmostEqual(
                44.0,
                math.hypot(end[0] - start[0], end[1] - start[1]),
                places=4,
            )

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
        self.assertIn(
            "BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NEXT_RADIAL_SLOT", runtime
        )
        self.assertIn("const std::uint32_t nextOrdinal = (ordinal + 1u) % count", runtime)
        self.assertIn("routeX * inverseRouteLength", runtime)
        self.assertIn("routeZ * inverseRouteLength", runtime)

    def test_hidden_primary_ghost_uses_one_known_flag_and_render_only_gate(self) -> None:
        packet_header = PACKET_HEADER_PATH.read_text(encoding="utf-8")
        packet_runtime = PACKET_RUNTIME_PATH.read_text(encoding="utf-8")
        valtan_header = VALTAN_HEADER_PATH.read_text(encoding="utf-8")
        valtan_runtime = VALTAN_RUNTIME_PATH.read_text(encoding="utf-8")

        self.assertIn("GHOST_HIDDEN = 1u << 4", packet_header)
        self.assertIn(
            "BOSS_COMBAT_STATE_FLAG::GHOST_HIDDEN", packet_runtime
        )
        self.assertIn("isInvulnerable && snapshot.iGameplayPhase >= 3u", packet_runtime)
        self.assertIn("m_isGhostPresentationHidden", valtan_header)
        self.assertIn(
            "if (!m_isGhostPresentationHidden)\n\t\t__super::Late_Update(fTimeDelta);",
            valtan_runtime,
        )
        self.assertIn("Client::CEffectV2Runtime::Tick(", valtan_runtime)
        self.assertIn("m_isGhostPresentationHidden = isGhostHidden;", valtan_runtime)
        self.assertIn("CEffectV2Runtime::Set_Ignored(", valtan_runtime)
        self.assertIn(
            "m_isGhostPresentationHidden ||\n\t\t!m_hasNetworkTransformState",
            valtan_runtime,
        )

    def test_primary_relocation_is_nav_admitted_atomic_and_one_tick(self) -> None:
        room = GAME_ROOM_PATH.read_text(encoding="utf-8")
        world_entity = SERVER_WORLD_ENTITY_PATH.read_text(encoding="utf-8")

        self.assertIn("Begin_ValtanGhostRelocation(", room)
        self.assertIn("attempt < 128u && !foundSpawn", room)
        self.assertIn("hasSpawnClearance(spawn)", room)
        self.assertIn("SERVER_BOSS_COMBAT_FLAG::INVULNERABLE, true", room)
        self.assertIn("SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN, true", room)
        self.assertIn("boss.bGhostRepositionPending = true;", room)
        self.assertIn(
            "Add_ServerTicksSkippingReservedZero(serverTick, 1u)", room
        )
        self.assertIn("updateValtanBrain = false;", room)
        self.assertIn("SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN, false", room)
        self.assertIn("SERVER_BOSS_COMBAT_FLAG::INVULNERABLE, false", room)
        self.assertIn("bool bGhostRepositionPending = false;", world_entity)
        self.assertIn("std::uint32_t iGhostReappearTick = 0u;", world_entity)

    def test_portal_off_navigation_exception_is_exactly_phase_three_square(self) -> None:
        room = GAME_ROOM_PATH.read_text(encoding="utf-8")

        self.assertIn("phaseThreePortalSquareMayStartOffNavigation", room)
        self.assertIn(
            "BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NEXT_RADIAL_SLOT ==", room
        )
        self.assertIn('"VALTAN_GHOST_PORTAL_ONCE" == patternId', room)
        self.assertIn("boss.bGhostPhasePatternLoopActive && 3u == boss.iPhase", room)
        self.assertIn("!phaseThreePortalSquareMayStartOffNavigation", room)


if __name__ == "__main__":
    unittest.main()
