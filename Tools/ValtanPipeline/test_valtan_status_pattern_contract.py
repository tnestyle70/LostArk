#!/usr/bin/env python3
from __future__ import annotations

import copy
import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PIPELINE_ROOT = ROOT / "Tools/ValtanPipeline"
if str(PIPELINE_ROOT) not in sys.path:
    sys.path.insert(0, str(PIPELINE_ROOT))

import valtan_tuning_pipeline as pipeline  # noqa: E402


def read_text(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8-sig")


class ValtanStatusPatternContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.gameplay = json.loads(read_text("Data/Valtan/Valtan.gameplay.json"))
        cls.presentation = json.loads(
            read_text("Data/Valtan/Valtan.presentation.json")
        )
        cls.gameplay_by_id = {
            row["patternId"]: row for row in cls.gameplay["patterns"]
        }
        cls.presentation_by_id = {
            row["patternId"]: row for row in cls.presentation["patterns"]
        }

    def test_split_authoring_validates_without_projecting_product(self) -> None:
        documents = pipeline.load_pipeline_documents(ROOT)
        joined = pipeline.join_v2_authoring(
            documents[pipeline.GAMEPLAY_AUTHORING_REL],
            documents[pipeline.PRESENTATION_AUTHORING_REL],
            documents[pipeline.WORLD_SET_REL],
            documents[pipeline.COMBAT_AUTHORING_REL],
            documents.get(pipeline.SAVED_FLOW_REL),
        )
        pipeline.validate_v2_master(
            joined,
            documents[pipeline.WORLD_SET_REL],
            documents[pipeline.COMBAT_AUTHORING_REL],
        )

    def test_stagger_break_and_timeout_are_exact_typed_edges(self) -> None:
        pattern = self.gameplay_by_id["VALTAN_STAGGER_SLOT"]
        self.assertEqual("NONE", pattern["targetPolicy"])
        self.assertEqual(["STEP_01", "GROGGY", "RECOVERY"],
                         [row["stageId"] for row in pattern["stages"]])
        active, groggy, recovery = pattern["stages"]
        self.assertEqual(5000, active["durationMs"])
        self.assertEqual(
            [("STAGGER_BROKEN", "valtan.authoring.stagger-slot.groggy"),
             ("TIMEOUT", "valtan.authoring.stagger-slot.recovery")],
            [(row["outcome"], row["nextActionId"])
             for row in active["branches"]],
        )
        self.assertNotIn("targetActionId", json.dumps(active))
        self.assertEqual(
            [("ENTER", 100), ("EXIT", 0)],
            [(row["trigger"], row["value"]) for row in active["events"]],
        )
        self.assertEqual("GROGGY", groggy["stageKind"])
        self.assertEqual(3000, groggy["durationMs"])
        self.assertEqual("RECOVERY", recovery["stageKind"])
        self.assertEqual(1000, recovery["durationMs"])

    def test_bind_and_silence_default_to_five_seconds(self) -> None:
        bind = self.gameplay_by_id["VALTAN_BIND_SLOT"]
        self.assertEqual("LOCK_RANDOM_ALIVE_ON_START", bind["targetPolicy"])
        bind_events = bind["stages"][0]["events"]
        self.assertEqual(
            [("ENTER", "SET_PLAYER_BIND", 10.0, 5000),
             ("EXIT", "SET_PLAYER_BIND", 0.0, 0)],
            [(row["trigger"], row["kind"], row["heightM"], row["durationMs"])
             for row in bind_events],
        )
        silence_events = self.gameplay_by_id["VALTAN_SILENCE_SLOT"]["stages"][0]["events"]
        self.assertEqual(
            [("ENTER", "SET_PLAYER_SILENCE", 5000),
             ("EXIT", "SET_PLAYER_SILENCE", 0)],
            [(row["trigger"], row["kind"], row["durationMs"])
             for row in silence_events],
        )

    def test_status_presentation_is_animationless_and_does_not_invent_effect(self) -> None:
        for pattern_id in (
            "VALTAN_STAGGER_SLOT", "VALTAN_BIND_SLOT", "VALTAN_SILENCE_SLOT"
        ):
            with self.subTest(pattern_id=pattern_id):
                stages = self.presentation_by_id[pattern_id]["stages"]
                self.assertEqual(
                    [row["stageId"] for row in self.gameplay_by_id[pattern_id]["stages"]],
                    [row["stageId"] for row in stages],
                )
                for stage in stages:
                    self.assertEqual({"mode": "NONE"}, stage["animation"])
                    self.assertEqual([], stage["effectCues"])

    def test_invalid_status_clear_and_clock_fail_closed(self) -> None:
        documents = pipeline.load_pipeline_documents(ROOT)
        gameplay = documents[pipeline.GAMEPLAY_AUTHORING_REL]
        invalid = copy.deepcopy(gameplay)
        bind = next(row for row in invalid["patterns"]
                    if row["patternId"] == "VALTAN_BIND_SLOT")
        bind["stages"][0]["events"][0]["durationMs"] = 4999
        with self.assertRaises(pipeline.PipelineError):
            pipeline.join_v2_authoring(
                invalid, documents[pipeline.PRESENTATION_AUTHORING_REL],
                documents[pipeline.WORLD_SET_REL],
                documents[pipeline.COMBAT_AUTHORING_REL], None)
        invalid = copy.deepcopy(gameplay)
        silence = next(row for row in invalid["patterns"]
                       if row["patternId"] == "VALTAN_SILENCE_SLOT")
        silence["stages"][0]["events"][1]["durationMs"] = 1
        with self.assertRaises(pipeline.PipelineError):
            pipeline.join_v2_authoring(
                invalid, documents[pipeline.PRESENTATION_AUTHORING_REL],
                documents[pipeline.WORLD_SET_REL],
                documents[pipeline.COMBAT_AUTHORING_REL], None)

    def test_server_wire_hud_and_composition_consumers_are_connected(self) -> None:
        player = read_text("Server/Public/ServerPlayer.h")
        room = read_text("Server/Private/GameRoom.cpp")
        catalog = read_text("Server/Private/GameplayCatalog.cpp")
        packet_h = read_text("Shared/Public/Network/PacketMessages.h")
        packet_cpp = read_text("Shared/Private/Network/PacketMessages.cpp")
        protocol = read_text("Shared/Public/Network/PacketType.h")
        harness = read_text(
            "Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp"
        )
        hud = read_text("Client/Private/CombatHUDViewModel.cpp")
        main_app = read_text("Client/Private/MainApp.cpp")
        composition = read_text("Client/Private/ActionCompositionWorkbench.cpp")
        for marker in (
            "bPatternBound", "iPatternBindEndTick", "fPatternBindRestoreX",
            "iSilenceEndTick", "iSilenceDurationTicks",
        ):
            self.assertIn(marker, player)
        for marker in (
            "isPatternBound", "iPatternBindEndTick", "iSilenceEndTick",
            "iSilenceDurationTicks",
        ):
            self.assertIn(marker, packet_h)
        for marker in (
            "SET_PLAYER_BIND", "SET_PLAYER_SILENCE",
            "Cancel_PlayerActionForPatternStatus",
            "Restore_PatternBoundPlayer", "ownsLivePatternOccurrence",
            "Clear_SilenceStatus", "Has_ReachedServerTick",
        ):
            self.assertIn(marker, room)
        self.assertIn('"player.status.bind" == targetId', catalog)
        self.assertIn('"player.status.silence" == targetId', catalog)
        self.assertIn("writer.Write_U8(player.isPatternBound ? 1u : 0u)", packet_cpp)
        self.assertIn("player.isPatternBound = 0u != rawPatternBound", packet_cpp)
        self.assertIn("NETWORK_PROTOCOL_VERSION = 51", protocol)
        self.assertIn("Pattern-Bound Player Snapshot Round Trip", harness)
        self.assertIn("iSilenceDurationTicks", harness)
        self.assertIn("m_Player.iSilenceEndTick > state.iCooldownEndTick", hud)
        self.assertIn("boss.iMaximumStagger - (std::min)", main_app)
        for marker in (
            "Stagger Gauge | ", "Bind | +", "Silence | ",
            "Status & Gauge (Non-spatial)",
            "Selected Status / Gauge Box", "Stage window: %u ms",
            "Status value and ENTER/EXIT timing remain read-only",
        ):
            self.assertIn(marker, composition)


if __name__ == "__main__":
    unittest.main()
