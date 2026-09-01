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
        bindings = json.loads(
            read_text("Data/Animation/Authored/Valtan/Valtan.patternbindings.json")
        )
        cls.binding_by_action = {
            row["actionId"]: row for row in bindings["bindings"]
        }

    def test_split_authoring_validates_without_projecting_product(self) -> None:
        documents = pipeline.load_pipeline_documents(ROOT)
        joined = pipeline.join_v2_authoring(
            documents[pipeline.GAMEPLAY_AUTHORING_REL],
            documents[pipeline.PRESENTATION_AUTHORING_REL],
            documents[pipeline.WORLD_SET_REL],
            documents[pipeline.COMBAT_AUTHORING_REL],
        )
        pipeline.validate_v2_master(
            joined,
            documents[pipeline.WORLD_SET_REL],
            documents[pipeline.COMBAT_AUTHORING_REL],
        )

    def test_stagger_break_and_timeout_are_exact_typed_edges(self) -> None:
        pattern = self.gameplay_by_id["VALTAN_STAGGER_SLOT"]
        self.assertEqual("마력구파괴 패턴", pattern["displayName"])
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
        self.assertEqual(6833, groggy["durationMs"])
        self.assertEqual("RECOVERY", recovery["stageKind"])
        self.assertEqual(1000, recovery["durationMs"])

    def test_bind_and_silence_follow_their_authored_stage_clocks(self) -> None:
        bind = self.gameplay_by_id["VALTAN_BIND_SLOT"]
        self.assertEqual("속박 패턴", bind["displayName"])
        self.assertEqual("LOCK_RANDOM_ALIVE_ON_START", bind["targetPolicy"])
        self.assertEqual([420623, 400442], bind["sourceActionIds"])
        self.assertEqual(8533, bind["stages"][0]["durationMs"])
        bind_events = bind["stages"][0]["events"]
        self.assertEqual(
            [("ENTER", "SET_PLAYER_BIND", 10.0, 8533),
             ("EXIT", "SET_PLAYER_BIND", 0.0, 0)],
            [(row["trigger"], row["kind"], row["heightM"], row["durationMs"])
             for row in bind_events],
        )
        silence = self.gameplay_by_id["VALTAN_SILENCE_SLOT"]
        self.assertEqual("침묵 패턴", silence["displayName"])
        silence_events = silence["stages"][0]["events"]
        self.assertEqual(
            [("ENTER", "SET_PLAYER_SILENCE", 2633),
             ("EXIT", "SET_PLAYER_SILENCE", 0)],
            [(row["trigger"], row["kind"], row["durationMs"])
             for row in silence_events],
        )

    def test_status_presentation_owns_only_the_selected_animation_stages(self) -> None:
        bind = self.presentation_by_id["VALTAN_BIND_SLOT"]
        self.assertEqual(
            [
                {"sourceActionId": 420623, "sequenceIndex": 1, "role": "PRIMARY"},
                {
                    "sourceActionId": 400442,
                    "sequenceIndex": 0,
                    "role": "REFERENCE_400442_0",
                },
            ],
            bind["presentationSources"],
        )
        bind_animation = bind["stages"][0]["animation"]
        self.assertEqual("EXACT", bind_animation["endPolicy"])
        self.assertEqual(
            [
                ("mesh_att_battle_5_01_start", 1400),
                ("mesh_att_battle_5_01_loop", 900),
                ("mesh_att_battle_5_01_loop", 900),
                ("mesh_att_battle_5_01_loop", 900),
                ("mesh_att_battle_5_01_end", 4433),
            ],
            [(row["clip"], row["playMs"])
             for row in bind_animation["occurrences"]],
        )
        bind_binding = self.binding_by_action[
            "valtan.authoring.bind-slot.step-01"
        ]
        self.assertEqual(
            [(row["clip"], row["playMs"])
             for row in bind_animation["occurrences"]],
            [(row["clip"], row["playMs"])
             for row in bind_binding["clips"]],
        )

        stagger_stages = {
            stage["stageId"]: stage
            for stage in self.presentation_by_id["VALTAN_STAGGER_SLOT"]["stages"]
        }
        self.assertEqual({"mode": "NONE"}, stagger_stages["STEP_01"]["animation"])
        self.assertEqual({"mode": "NONE"}, stagger_stages["RECOVERY"]["animation"])
        groggy_occurrences = stagger_stages["GROGGY"]["animation"]["occurrences"]
        self.assertEqual(
            ["mesh_abn_groggy_1_start", "mesh_abn_groggy_1_loop",
             "mesh_abn_groggy_1_loop", "mesh_abn_groggy_1_loop",
             "mesh_abn_groggy_1_end"],
            [row["clip"] for row in groggy_occurrences],
        )
        self.assertEqual(6833, sum(row["playMs"] for row in groggy_occurrences))

        silence_stage = self.presentation_by_id["VALTAN_SILENCE_SLOT"]["stages"][0]
        self.assertEqual(
            ["mesh_evt1_att_battle_5_01_end"],
            [row["clip"] for row in silence_stage["animation"]["occurrences"]],
        )
        self.assertEqual(
            2633,
            sum(row["playMs"] for row in silence_stage["animation"]["occurrences"]),
        )

        for pattern_id in (
            "VALTAN_STAGGER_SLOT", "VALTAN_BIND_SLOT", "VALTAN_SILENCE_SLOT"
        ):
            stages = self.presentation_by_id[pattern_id]["stages"]
            self.assertEqual(
                [row["stageId"] for row in self.gameplay_by_id[pattern_id]["stages"]],
                [row["stageId"] for row in stages],
            )
            self.assertTrue(all(stage["effectCues"] == [] for stage in stages))

    def test_pattern_display_name_patch_is_typed_and_fail_closed(self) -> None:
        documents = pipeline.load_pipeline_documents(ROOT)
        source_revision = pipeline.source_manifest(ROOT)["sourceManifestId"]
        master = pipeline.join_v2_authoring(
            documents[pipeline.GAMEPLAY_AUTHORING_REL],
            documents[pipeline.PRESENTATION_AUTHORING_REL],
            documents[pipeline.WORLD_SET_REL],
            documents[pipeline.COMBAT_AUTHORING_REL],
        )

        def apply(pattern_id: str, display_name: str) -> dict:
            candidate, _, _, operation_count = pipeline.apply_draft_patch(
                master,
                copy.deepcopy(documents[pipeline.BOSS_PROFILES_REL]),
                copy.deepcopy(documents[pipeline.DAMAGE_REL]),
                {
                    "schema": pipeline.DRAFT_PATCH_SCHEMA,
                    "formatVersion": 1,
                    "sourceRevision": source_revision,
                    "operations": [{
                        "op": "SET_PATTERN_DISPLAY_NAME",
                        "patternId": pattern_id,
                        "displayName": display_name,
                    }],
                },
                source_revision,
                documents[pipeline.WORLD_SET_REL],
                documents[pipeline.COMBAT_AUTHORING_REL],
                repository_root=ROOT,
            )
            self.assertEqual(1, operation_count)
            return candidate

        candidate = apply("VALTAN_SILENCE_SLOT", "침묵 패턴")
        renamed = next(row for row in candidate["patterns"]
                       if row["patternId"] == "VALTAN_SILENCE_SLOT")
        self.assertEqual("침묵 패턴", renamed["displayName"])

        original = copy.deepcopy(master)
        with self.assertRaises(pipeline.PipelineError):
            apply("VALTAN_SILENCE_SLOT", " ")
        with self.assertRaises(pipeline.PipelineError):
            apply("VALTAN_UNKNOWN_SLOT", "알 수 없음")
        self.assertEqual(original, master)

    def test_invalid_status_clear_and_clock_fail_closed(self) -> None:
        documents = pipeline.load_pipeline_documents(ROOT)
        gameplay = documents[pipeline.GAMEPLAY_AUTHORING_REL]
        invalid = copy.deepcopy(gameplay)
        bind = next(row for row in invalid["patterns"]
                    if row["patternId"] == "VALTAN_BIND_SLOT")
        bind["stages"][0]["events"][0]["durationMs"] = 8532
        with self.assertRaises(pipeline.PipelineError):
            pipeline.join_v2_authoring(
                invalid, documents[pipeline.PRESENTATION_AUTHORING_REL],
                documents[pipeline.WORLD_SET_REL],
                documents[pipeline.COMBAT_AUTHORING_REL])
        invalid = copy.deepcopy(gameplay)
        silence = next(row for row in invalid["patterns"]
                       if row["patternId"] == "VALTAN_SILENCE_SLOT")
        silence["stages"][0]["events"][1]["durationMs"] = 1
        with self.assertRaises(pipeline.PipelineError):
            pipeline.join_v2_authoring(
                invalid, documents[pipeline.PRESENTATION_AUTHORING_REL],
                documents[pipeline.WORLD_SET_REL],
                documents[pipeline.COMBAT_AUTHORING_REL])

    def test_stage_duration_patch_cascades_whole_stage_status_clock(self) -> None:
        documents = pipeline.load_pipeline_documents(ROOT)
        source_revision = pipeline.source_manifest(ROOT)["sourceManifestId"]
        master = pipeline.join_v2_authoring(
            documents[pipeline.GAMEPLAY_AUTHORING_REL],
            documents[pipeline.PRESENTATION_AUTHORING_REL],
            documents[pipeline.WORLD_SET_REL],
            documents[pipeline.COMBAT_AUTHORING_REL],
        )

        for pattern_id, status_kind, duration_ms in (
            ("VALTAN_BIND_SLOT", "SET_PLAYER_BIND", 8000),
            ("VALTAN_SILENCE_SLOT", "SET_PLAYER_SILENCE", 2000),
        ):
            with self.subTest(pattern_id=pattern_id):
                current_stage = next(
                    stage
                    for pattern in master["patterns"]
                    if pattern["patternId"] == pattern_id
                    for stage in pattern["stages"]
                    if stage["stageId"] == "STEP_01"
                )
                resized_animation = copy.deepcopy(current_stage["animation"])
                resized_animation["occurrences"][-1]["playMs"] += (
                    duration_ms - current_stage["durationMs"]
                )
                candidate, _, _, operation_count = pipeline.apply_draft_patch(
                    master,
                    copy.deepcopy(documents[pipeline.BOSS_PROFILES_REL]),
                    copy.deepcopy(documents[pipeline.DAMAGE_REL]),
                    {
                        "schema": pipeline.DRAFT_PATCH_SCHEMA,
                        "formatVersion": 1,
                        "sourceRevision": source_revision,
                        "operations": [
                            {
                                "op": "SET_STAGE_DURATION",
                                "patternId": pattern_id,
                                "stageId": "STEP_01",
                                "durationMs": duration_ms,
                            },
                            {
                                "op": "SET_STAGE_ANIMATION",
                                "patternId": pattern_id,
                                "stageId": "STEP_01",
                                "animation": resized_animation,
                            },
                        ],
                    },
                    source_revision,
                    documents[pipeline.WORLD_SET_REL],
                    documents[pipeline.COMBAT_AUTHORING_REL],
                    repository_root=ROOT,
                )
                self.assertEqual(2, operation_count)
                patched_pattern = next(
                    row for row in candidate["patterns"]
                    if row["patternId"] == pattern_id
                )
                patched_stage = patched_pattern["stages"][0]
                self.assertEqual(duration_ms, patched_stage["durationMs"])
                self.assertEqual(
                    duration_ms,
                    sum(
                        occurrence["playMs"]
                        for occurrence in patched_stage["animation"]["occurrences"]
                    ),
                )
                self.assertEqual(
                    [("ENTER", duration_ms), ("EXIT", 0)],
                    [
                        (event["trigger"], event["durationMs"])
                        for event in patched_stage["events"]
                        if event["kind"] == status_kind
                    ],
                )

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
        self.assertIn("NETWORK_PROTOCOL_VERSION = 52", protocol)
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

    def test_silence_tints_existing_cooldown_and_icon_slots(self) -> None:
        main_app = read_text("Client/Private/MainApp.cpp")
        runtime_h = read_text("Client/Public/UILayoutRuntime.h")
        runtime_cpp = read_text("Client/Private/UILayoutRuntime.cpp")
        sprite_h = read_text("Client/Public/UI_Sprite.h")
        sprite_cpp = read_text("Client/Private/UI_Sprite.cpp")
        hud_layout = json.loads(read_text("Data/UI/HUD/HUD_Layout.json"))

        cooldown_start = main_app.index("void CMainApp::Update_SkillCooldowns()")
        cooldown_end = main_app.index(
            "void CMainApp::RenderSkillCooldownText()", cooldown_start
        )
        cooldown = main_app[cooldown_start:cooldown_end]
        for marker in (
            "Is_ServerDeadlinePending(player.iServerTick, player.iSilenceEndTick)",
            "float4_t(0.85f, 0.04f, 0.04f, 0.7f)",
            "float4_t(0.f, 0.f, 0.f, 150.f / 255.f)",
            'string("Skill_") + pInputSlot + "_Cooldown"',
            "Set_SlotTint(strOverlaySlot, vCooldownTint)",
            "Set_SlotArcRatio(strOverlaySlot, fFraction)",
            "Set_SlotVisible(strOverlaySlot, true)",
        ):
            self.assertIn(marker, cooldown)
        self.assertNotIn("5000", cooldown)
        self.assertNotIn("150u", cooldown)

        for marker in (
            "Is_ServerDeadlinePending(Player.iServerTick, Player.iSilenceEndTick)",
            "Apply_SilenceQuickSlotTint(m_pHUDRuntimeView.get(), player)",
            "float4_t(1.f, 0.2f, 0.2f, 1.f)",
            'string("Skill_") + pInputSlot + "_Icon"',
            '"Skill_Z", "Skill_X", "Yin_Skill_Z", "Yin_Skill_X"',
            "Set_SlotTintMultiplier(pSlotId, vIconTint)",
        ):
            self.assertIn(marker, main_app)

        self.assertIn("Set_SlotTintMultiplier", runtime_h)
        self.assertIn("pKeySprite->Set_TintMultiplier", runtime_cpp)
        self.assertIn("m_vTintMultiplier", sprite_h)
        self.assertIn("m_vTint.x * m_vTintMultiplier.x", sprite_cpp)
        self.assertIn("m_vTint.w * m_vTintMultiplier.w", sprite_cpp)

        slots_by_id = {slot["id"]: slot for slot in hud_layout["slots"]}
        for input_slot in ("Q", "W", "E", "R", "A", "S", "D", "F", "T", "V"):
            layer = slots_by_id[f"Skill_{input_slot}_Cooldown"]["layers"][0]
            self.assertEqual("UI/Common/White1x1.png", layer["path"])
            self.assertEqual([0, 0, 0, 150 / 255], layer["tint"])


if __name__ == "__main__":
    unittest.main()
