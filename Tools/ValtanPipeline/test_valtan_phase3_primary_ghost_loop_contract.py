import copy
import json
import math
import sys
import unittest
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPOSITORY_ROOT / "Tools/ValtanPipeline"))
import valtan_tuning_pipeline as pipeline

GAMEPLAY_PATH = REPOSITORY_ROOT / "Data/Valtan/Valtan.gameplay.json"
PRESENTATION_PATH = REPOSITORY_ROOT / "Data/Valtan/Valtan.presentation.json"
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
    def test_ghost_death_exit_skips_only_pursuit_and_projects_respawn_phase(self) -> None:
        gameplay = load_json(GAMEPLAY_PATH)
        sequence = gameplay["decisionModel"]["scriptedSequence"]["patternIds"]
        death_index = sequence.index("VALTAN_GHOST_DEATH_AUDITION")
        self.assertEqual(
            "VALTAN_GHOST_RESPAWN_AUDITION", sequence[death_index + 1]
        )
        patterns = {row["patternId"]: row for row in gameplay["patterns"]}
        death = patterns["VALTAN_GHOST_DEATH_AUDITION"]
        self.assertEqual(1, len(death["stages"]))
        self.assertIsNone(death["stages"][0]["defaultNextActionId"])
        self.assertEqual([], death["stages"][0]["branches"])
        self.assertEqual(
            [
                {
                    "eventId": "event.valtan.ghost-death.suppress-inter-step-pursuit",
                    "trigger": "EXIT",
                    "kind": "SUPPRESS_INTER_STEP_PURSUIT",
                }
            ],
            death["stages"][0]["events"],
        )

        documents = pipeline.load_pipeline_documents(REPOSITORY_ROOT)
        master = pipeline.join_v2_authoring(
            documents[pipeline.GAMEPLAY_AUTHORING_REL],
            documents[pipeline.PRESENTATION_AUTHORING_REL],
            documents[pipeline.WORLD_SET_REL],
            documents[pipeline.COMBAT_AUTHORING_REL],
        )
        products = pipeline.project_v2_products(
            REPOSITORY_ROOT, documents, master
        )
        encounter = json.loads(products[pipeline.ENCOUNTER_REL])
        projected = {row["patternId"]: row for row in encounter["patterns"]}
        self.assertEqual(
            [
                {
                    "trigger": "EXIT",
                    "kind": "SUPPRESS_INTER_STEP_PURSUIT",
                    "targetId": "boss.sequence.inter-step-pursuit",
                    "value": 0,
                    "durationMs": 0,
                }
            ],
            projected["VALTAN_GHOST_DEATH_AUDITION"]["stages"][0]["actions"],
        )
        self.assertEqual(
            [
                {
                    "trigger": "ENTER",
                    "kind": "SET_GAMEPLAY_PHASE",
                    "targetId": "boss.phase.gameplay",
                    "value": 3,
                    "durationMs": 0,
                }
            ],
            projected["VALTAN_GHOST_RESPAWN_AUDITION"]["stages"][0]["actions"],
        )

        for defect in ("wrong trigger", "wrong owner", "nonterminal branch"):
            with self.subTest(defect=defect):
                invalid = copy.deepcopy(gameplay)
                invalid_patterns = {
                    row["patternId"]: row for row in invalid["patterns"]
                }
                invalid_death = invalid_patterns["VALTAN_GHOST_DEATH_AUDITION"]
                if defect == "wrong trigger":
                    invalid_death["stages"][0]["events"][0]["trigger"] = "ENTER"
                elif defect == "wrong owner":
                    invalid_death["stages"][0]["events"] = []
                    invalid_patterns["VALTAN_GHOST_RESPAWN_AUDITION"]["stages"][0][
                        "events"
                    ].append(death["stages"][0]["events"][0])
                else:
                    invalid_death["stages"][0]["branches"] = [
                        {"outcome": "TIMEOUT", "nextActionId": None}
                    ]
                with self.assertRaisesRegex(
                    pipeline.PipelineError,
                    "inter-step pursuit suppression must be the terminal ghost-death EXIT event",
                ):
                    pipeline.join_v2_authoring(
                        invalid,
                        documents[pipeline.PRESENTATION_AUTHORING_REL],
                        documents[pipeline.WORLD_SET_REL],
                        documents[pipeline.COMBAT_AUTHORING_REL],
                    )

    def test_phase_three_swap_uses_restored_primary_ghost_resources(self) -> None:
        bosses = {
            row["archetypeId"]: row
            for row in load_json(BOSS_CATALOG_PATH)["bosses"]
        }
        ghost = bosses["BOSS_VALTAN_GHOST"]
        self.assertEqual(
            "Character/Valtan/Ghost/MN_RPBF_02.wmodel", ghost["bodyModel"]
        )
        self.assertEqual(
            "Character/Valtan/Ghost/MN_RPBF_02_AnimSet.wmodel",
            ghost["animationSetId"],
        )
        for asset_id in (ghost["bodyModel"], ghost["animationSetId"]):
            self.assertTrue(
                (REPOSITORY_ROOT / "Client/Bin/Resources" / asset_id).is_file(),
                asset_id,
            )
        runtime = VALTAN_RUNTIME_PATH.read_text(encoding="utf-8")
        self.assertIn("state.iGameplayPhase >= 3u", runtime)
        self.assertIn('"BOSS_VALTAN_GHOST" : "BOSS_VALTAN"', runtime)
        self.assertIn(
            "Replace_PresentationPartGroup(\n\t\t\t\t\tDesiredPresentationArchetype",
            runtime,
        )

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

    def test_portal_once_is_one_atomic_closed_equilateral_triangle_volley(self) -> None:
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
        self.assertEqual(3, event["countPerResolvedTarget"])
        self.assertEqual(3, event["maximumTotalObjects"])
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
        self.assertAlmostEqual(44.0 / math.sqrt(3.0), layout["radiusM"], places=12)
        self.assertEqual(30.0, layout["startAngleDegrees"])
        self.assertEqual(120.0, layout["angleStepDegrees"])

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

        vertices = []
        for ordinal in range(3):
            radians = math.radians(
                layout["startAngleDegrees"] + layout["angleStepDegrees"] * ordinal
            )
            vertices.append(
                (
                    math.sin(radians) * layout["radiusM"],
                    math.cos(radians) * layout["radiusM"],
                )
            )
        undirected_headings = []
        for ordinal, start in enumerate(vertices):
            end = vertices[(ordinal + 1) % len(vertices)]
            delta_x = end[0] - start[0]
            delta_z = end[1] - start[1]
            self.assertAlmostEqual(
                44.0,
                math.hypot(delta_x, delta_z),
                places=9,
            )
            heading = math.degrees(math.atan2(delta_x, delta_z)) % 180.0
            if math.isclose(heading, 180.0, rel_tol=0.0, abs_tol=1e-9):
                heading = 0.0
            undirected_headings.append(heading)
        for actual, expected in zip(
            sorted(undirected_headings), (0.0, 60.0, 120.0)
        ):
            self.assertAlmostEqual(expected, actual, places=9)

        independent = next(
            row
            for row in load_json(PRESENTATION_PATH)["independentEffects"]
            if row["independentEffectId"]
            == "valtan.independent-effect.ghost-portal-once"
        )
        self.assertEqual(
            "망령 포탈 돌진 1회 / 44m 정삼각형", independent["displayName"]
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

    def test_live_portal_triangle_geometry_and_name_drift_fail_closed(self) -> None:
        documents = pipeline.load_pipeline_documents(REPOSITORY_ROOT)
        invalid_geometry = copy.deepcopy(documents)
        portal = next(
            row
            for row in invalid_geometry[pipeline.GAMEPLAY_AUTHORING_REL]["patterns"]
            if row["patternId"] == "VALTAN_GHOST_PORTAL_ONCE"
        )
        portal["stages"][0]["events"][0]["layout"]["radiusM"] += 1.0
        with self.assertRaisesRegex(
            pipeline.PipelineError, "live ghost portal volley contract drifted"
        ):
            pipeline.join_v2_authoring(
                invalid_geometry[pipeline.GAMEPLAY_AUTHORING_REL],
                invalid_geometry[pipeline.PRESENTATION_AUTHORING_REL],
                invalid_geometry[pipeline.WORLD_SET_REL],
                invalid_geometry[pipeline.COMBAT_AUTHORING_REL],
            )

        invalid_name = copy.deepcopy(documents)
        independent = next(
            row
            for row in invalid_name[pipeline.PRESENTATION_AUTHORING_REL][
                "independentEffects"
            ]
            if row["independentEffectId"]
            == "valtan.independent-effect.ghost-portal-once"
        )
        independent["displayName"] = "stale square"
        with self.assertRaisesRegex(
            pipeline.PipelineError,
            "live ghost portal independent effect name drifted",
        ):
            pipeline.join_v2_authoring(
                invalid_name[pipeline.GAMEPLAY_AUTHORING_REL],
                invalid_name[pipeline.PRESENTATION_AUTHORING_REL],
                invalid_name[pipeline.WORLD_SET_REL],
                invalid_name[pipeline.COMBAT_AUTHORING_REL],
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
            "if (!m_isGhostPresentationHidden && !m_isPatternBodyHidden)\n"
            "\t\t__super::Late_Update(fTimeDelta);",
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

    def test_portal_off_navigation_exception_is_exactly_phase_three_triangle(self) -> None:
        room = GAME_ROOM_PATH.read_text(encoding="utf-8")

        self.assertIn("phaseThreePortalTriangleMayStartOffNavigation", room)
        self.assertIn(
            "BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NEXT_RADIAL_SLOT ==", room
        )
        self.assertIn('"VALTAN_GHOST_PORTAL_ONCE" == patternId', room)
        self.assertIn("boss.bGhostPhasePatternLoopActive && 3u == boss.iPhase", room)
        self.assertIn("!authoredVolleyMayStartOffNavigation", room)


if __name__ == "__main__":
    unittest.main()
