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

    def test_split_rejects_catch_timeout_default_edge_drift(self) -> None:
        documents = pipeline.load_pipeline_documents(ROOT)
        invalid = copy.deepcopy(documents[pipeline.GAMEPLAY_AUTHORING_REL])
        catch = next(
            row for row in invalid["patterns"]
            if row["patternId"] == "VALTAN_CATCH_BREATH"
        )
        grab = next(row for row in catch["stages"] if row["stageId"] == "STEP_02")
        self.assertIsNone(next(
            branch["nextActionId"] for branch in grab["branches"]
            if branch["outcome"] == "TIMEOUT"
        ))
        grab["defaultNextActionId"] = "valtan.sequence.catch-breath.step-03"
        with self.assertRaisesRegex(
            pipeline.PipelineError,
            "defaultNextActionId must match explicit TIMEOUT",
        ):
            pipeline.validate_gameplay_authoring(invalid)
        with self.assertRaisesRegex(
            pipeline.PipelineError,
            "defaultNextActionId must match explicit TIMEOUT",
        ):
            pipeline.join_v2_authoring(
                invalid,
                documents[pipeline.PRESENTATION_AUTHORING_REL],
                documents[pipeline.WORLD_SET_REL],
                documents[pipeline.COMBAT_AUTHORING_REL],
            )

    def test_magic_orb_damage_threshold_and_wipe_are_exact_typed_edges(self) -> None:
        pattern = self.gameplay_by_id["VALTAN_STAGGER_SLOT"]
        self.assertEqual("마력구 파괴 패턴", pattern["displayName"])
        self.assertEqual("NONE", pattern["targetPolicy"])
        self.assertEqual(3.0, pattern["verticalOffsetM"])
        self.assertEqual(["CHANNEL", "FINAL_ATTACK"],
                         [row["stageId"] for row in pattern["stages"]])
        channel, final_attack = pattern["stages"]
        self.assertEqual(12000, channel["durationMs"])
        self.assertEqual(
            {"kind": "ACCUMULATED_HEALTH_DAMAGE", "threshold": 1000},
            channel["bossResponse"],
        )
        self.assertEqual(
            [
                ("HEALTH_DAMAGE_THRESHOLD_REACHED", None,
                 "VALTAN_GROGGY_FOLLOWUP"),
                ("TIMEOUT", "valtan.authoring.stagger-slot.final-attack", None),
            ],
            [
                (row["outcome"], row["nextActionId"], row.get("nextPatternId"))
                for row in channel["branches"]
            ],
        )
        self.assertEqual(3000, final_attack["durationMs"])
        self.assertEqual(
            {
                "kind": "INTERVAL",
                "count": 1,
                "firstOffsetMs": 2900,
                "intervalMs": 0,
            },
            final_attack["hit"]["schedule"],
        )
        self.assertEqual(100.0, final_attack["hit"]["shape"]["outerRadiusM"])
        self.assertEqual(
            "damage.valtan.omnidirectional-wipe-130",
            final_attack["hit"]["serverDamageProfileId"],
        )

        groggy = self.gameplay_by_id["VALTAN_GROGGY_FOLLOWUP"]
        self.assertEqual("NONE", groggy["targetPolicy"])
        self.assertEqual(["GROGGY"], [row["stageId"] for row in groggy["stages"]])
        groggy_stage = groggy["stages"][0]
        self.assertEqual("GROGGY", groggy_stage["stageKind"])
        self.assertEqual(6833, groggy_stage["durationMs"])
        self.assertEqual(
            "DESTROY_FIRST_ELIGIBLE", groggy_stage["partDamagePolicy"]
        )
        self.assertEqual(
            {
                "PART_DESTROYED": "VALTAN_PART_BREAK",
                "TIMEOUT": None,
            },
            {
                row["outcome"]: row.get("nextPatternId")
                for row in groggy_stage["branches"]
            },
        )
        self.assertEqual(
            [("ENTER", True), ("EXIT", False)],
            [
                (row["trigger"], row["enabled"])
                for row in groggy_stage["events"]
                if row["kind"] == "SET_BOSS_FLAG"
                and row["flagId"] == "boss.flag.groggy"
            ],
        )

    def test_bind_clock_and_nonblocking_silence_deadline_are_authored(self) -> None:
        bind = self.gameplay_by_id["VALTAN_BIND_SLOT"]
        self.assertEqual("속박 패턴", bind["displayName"])
        self.assertEqual("LOCK_RANDOM_ALIVE_ON_START", bind["targetPolicy"])
        self.assertEqual([420623, 400442], bind["sourceActionIds"])
        self.assertEqual(
            [("STEP_01", 5000), ("RECOVERY", 3533)],
            [(row["stageId"], row["durationMs"]) for row in bind["stages"]],
        )
        bind_events = bind["stages"][0]["events"]
        self.assertEqual(
            [("ENTER", "SET_PLAYER_BIND", 5.0, 5000),
             ("EXIT", "SET_PLAYER_BIND", 0.0, 0)],
            [(row["trigger"], row["kind"], row["heightM"], row["durationMs"])
             for row in bind_events],
        )
        silence = self.gameplay_by_id["VALTAN_SILENCE_SLOT"]
        self.assertEqual("침묵 패턴", silence["displayName"])
        self.assertEqual(
            [("STEP_01", 2633), ("SILENCE_APPLY", 100)],
            [(row["stageId"], row["durationMs"]) for row in silence["stages"]],
        )
        self.assertEqual([], silence["stages"][0]["events"])
        silence_events = silence["stages"][1]["events"]
        self.assertEqual(
            [("ENTER", "SET_PLAYER_SILENCE", 5000)],
            [(row["trigger"], row["kind"], row["durationMs"])
             for row in silence_events],
        )

    def test_server_catalog_treats_silence_as_deadline_latched_not_exit_closed(self) -> None:
        catalog = read_text("Server/Private/GameplayCatalog.cpp")
        start = catalog.index("bool IsStatefulBossPatternStageAction(")
        end = catalog.index("bool HasFiniteBossStageGraph", start)
        lifetime_classifier = catalog[start:end]
        self.assertIn("SET_PLAYER_SILENCE != kind", lifetime_classifier)

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
                ("mesh_att_battle_5_01_end", 900),
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
        bind_recovery = bind["stages"][1]["animation"]
        self.assertEqual("EXACT", bind_recovery["endPolicy"])
        self.assertEqual(
            [("mesh_att_battle_5_01_end", 900, 3533)],
            [
                (row["clip"], row["sourceStartMs"], row["playMs"])
                for row in bind_recovery["occurrences"]
            ],
        )

        magic_stages = {
            stage["stageId"]: stage
            for stage in self.presentation_by_id["VALTAN_STAGGER_SLOT"]["stages"]
        }
        self.assertEqual(
            ["mesh_att_battle_17_start", "mesh_att_battle_17_loop"],
            [row["clip"] for row in magic_stages["CHANNEL"]["animation"]["occurrences"]],
        )
        self.assertTrue(
            magic_stages["CHANNEL"]["animation"]["occurrences"][-1][
                "repeatUntilStageEnd"
            ]
        )
        self.assertEqual(
            [("mesh_att_battle_17_end", 3000)],
            [
                (row["clip"], row["playMs"])
                for row in magic_stages["FINAL_ATTACK"]["animation"]["occurrences"]
            ],
        )

        groggy_occurrences = self.presentation_by_id[
            "VALTAN_GROGGY_FOLLOWUP"
        ]["stages"][0]["animation"]["occurrences"]
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
            "VALTAN_STAGGER_SLOT", "VALTAN_GROGGY_FOLLOWUP",
            "VALTAN_BIND_SLOT", "VALTAN_SILENCE_SLOT",
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
        bind["stages"][0]["events"][0]["durationMs"] = 4999
        with self.assertRaises(pipeline.PipelineError):
            pipeline.join_v2_authoring(
                invalid, documents[pipeline.PRESENTATION_AUTHORING_REL],
                documents[pipeline.WORLD_SET_REL],
                documents[pipeline.COMBAT_AUTHORING_REL])
        invalid = copy.deepcopy(gameplay)
        silence = next(row for row in invalid["patterns"]
                       if row["patternId"] == "VALTAN_SILENCE_SLOT")
        silence["stages"][1]["events"][0]["trigger"] = "EXIT"
        with self.assertRaises(pipeline.PipelineError):
            pipeline.join_v2_authoring(
                invalid, documents[pipeline.PRESENTATION_AUTHORING_REL],
                documents[pipeline.WORLD_SET_REL],
                documents[pipeline.COMBAT_AUTHORING_REL])

    def test_stage_duration_patch_preserves_deadline_latched_silence(self) -> None:
        documents = pipeline.load_pipeline_documents(ROOT)
        source_revision = pipeline.source_manifest(ROOT)["sourceManifestId"]
        master = pipeline.join_v2_authoring(
            documents[pipeline.GAMEPLAY_AUTHORING_REL],
            documents[pipeline.PRESENTATION_AUTHORING_REL],
            documents[pipeline.WORLD_SET_REL],
            documents[pipeline.COMBAT_AUTHORING_REL],
        )

        for pattern_id, stage_id, status_kind, duration_ms, resize_animation in (
            ("VALTAN_BIND_SLOT", "STEP_01", "SET_PLAYER_BIND", 8000, True),
            ("VALTAN_SILENCE_SLOT", "SILENCE_APPLY",
             "SET_PLAYER_SILENCE", 2000, False),
        ):
            with self.subTest(pattern_id=pattern_id):
                current_stage = next(
                    stage
                    for pattern in master["patterns"]
                    if pattern["patternId"] == pattern_id
                    for stage in pattern["stages"]
                    if stage["stageId"] == stage_id
                )
                operations = [{
                    "op": "SET_STAGE_DURATION",
                    "patternId": pattern_id,
                    "stageId": stage_id,
                    "durationMs": duration_ms,
                }]
                if resize_animation:
                    resized_animation = copy.deepcopy(current_stage["animation"])
                    resized_animation["occurrences"][-1]["playMs"] += (
                        duration_ms - current_stage["durationMs"]
                    )
                    operations.append({
                        "op": "SET_STAGE_ANIMATION",
                        "patternId": pattern_id,
                        "stageId": stage_id,
                        "animation": resized_animation,
                    })
                candidate, _, _, operation_count = pipeline.apply_draft_patch(
                    master,
                    copy.deepcopy(documents[pipeline.BOSS_PROFILES_REL]),
                    copy.deepcopy(documents[pipeline.DAMAGE_REL]),
                    {
                        "schema": pipeline.DRAFT_PATCH_SCHEMA,
                        "formatVersion": 1,
                        "sourceRevision": source_revision,
                        "operations": operations,
                    },
                    source_revision,
                    documents[pipeline.WORLD_SET_REL],
                    documents[pipeline.COMBAT_AUTHORING_REL],
                    repository_root=ROOT,
                )
                self.assertEqual(len(operations), operation_count)
                patched_pattern = next(
                    row for row in candidate["patterns"]
                    if row["patternId"] == pattern_id
                )
                patched_stage = next(
                    row for row in patched_pattern["stages"]
                    if row["stageId"] == stage_id
                )
                self.assertEqual(duration_ms, patched_stage["durationMs"])
                if resize_animation:
                    self.assertEqual(
                        duration_ms,
                        sum(
                            occurrence["playMs"]
                            for occurrence in patched_stage["animation"]["occurrences"]
                        ),
                    )
                else:
                    self.assertEqual({"mode": "NONE"}, patched_stage["animation"])
                expected_events = (
                    [("ENTER", duration_ms), ("EXIT", 0)]
                    if status_kind == "SET_PLAYER_BIND"
                    else [("ENTER", 5000)]
                )
                self.assertEqual(expected_events, [
                    (event["trigger"], event["durationMs"])
                    for event in patched_stage["events"]
                    if event["kind"] == status_kind
                ])

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
        silence_deadline_clear = room[
            room.index("if (0u != player.iSilenceEndTick"):
            room.index("#ifdef _DEBUG", room.index("if (0u != player.iSilenceEndTick"))
        ]
        self.assertIn("Has_ReachedServerTick", silence_deadline_clear)
        self.assertNotIn("ownsLivePatternOccurrence", silence_deadline_clear)
        self.assertIn('"player.status.bind" == targetId', catalog)
        self.assertIn('"player.status.silence" == targetId', catalog)
        self.assertIn("writer.Write_U8(player.isPatternBound ? 1u : 0u)", packet_cpp)
        self.assertIn("player.isPatternBound = 0u != rawPatternBound", packet_cpp)
        self.assertIn("NETWORK_PROTOCOL_VERSION = 53", protocol)
        self.assertIn("Pattern-Bound Player Snapshot Round Trip", harness)
        self.assertIn("iSilenceDurationTicks", harness)
        skill_build = hud[
            hud.index("void Client::CCombatHUDViewModel::Build_PlayerSkills"):
            hud.index("void Client::CCombatHUDViewModel::Apply_Boss")
        ]
        self.assertNotIn("iSilenceEndTick", skill_build)
        self.assertIn("boss.iMaximumStagger - (std::min)", main_app)
        for marker in (
            "Stagger Gauge | ", "Bind | +", "Silence | ",
            "Status & Gauge (Non-spatial)",
            "Selected Status / Gauge Box", "Stage window: %u ms",
            "Status value and ENTER/EXIT timing remain read-only",
        ):
            self.assertIn(marker, composition)

    def test_single_player_bind_remains_a_live_boss_targetless_occurrence(self) -> None:
        brain = read_text("Server/Private/ValtanBrain.cpp")
        bind_guard = brain[
            brain.index("bool PatternOwnsBindLifecycle"):
            brain.index("const BOSS_PATTERN_STAGE_DEFINITION* FindStageByActionId")
        ]
        for marker in (
            "BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_BIND",
            "BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER",
            "BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT",
            "bool CanContinueTargetlessOwnedBindPattern",
            "player.bPatternBound",
            "player.iPatternBindOwnerNetEntityId == boss.iNetEntityId",
            "player.iPatternBindSequence == boss.iPatternSequence",
        ):
            self.assertIn(marker, bind_guard)
        self.assertNotIn("VALTAN_BIND_SLOT", bind_guard)

        update = brain[brain.index("void LostArk::Server::CValtanBrain::Update("):]
        self.assertIn(
            "CanContinueTargetlessOwnedBindPattern(boss, players, *patterns)",
            update,
        )
        self.assertGreaterEqual(
            update.count("continueTargetlessOwnedBindPattern"),
            2,
        )
        self.assertNotIn("pauseOrderedStepForRevive", update)

    def test_silence_uses_a_dedicated_r_mask_without_mutating_cooldown_or_icon(self) -> None:
        main_app = read_text("Client/Private/MainApp.cpp")
        hud_layout = json.loads(read_text("Data/UI/HUD/HUD_Layout.json"))

        cooldown_start = main_app.index("void CMainApp::Update_SkillCooldowns()")
        cooldown_end = main_app.index(
            "void CMainApp::RenderSkillCooldownText()", cooldown_start
        )
        cooldown = main_app[cooldown_start:cooldown_end]
        for marker in (
            "float4_t(0.f, 0.f, 0.f, 150.f / 255.f)",
            'string("Skill_") + pInputSlot + "_Cooldown"',
            "Set_SlotTint(strOverlaySlot, vCooldownTint)",
            "Set_SlotArcRatio(strOverlaySlot, fFraction)",
            "Set_SlotVisible(strOverlaySlot, true)",
        ):
            self.assertIn(marker, cooldown)
        self.assertNotIn("iSilenceEndTick", cooldown)
        self.assertNotIn("Skill_R_SilenceMask", cooldown)
        self.assertNotIn("5000", cooldown)
        self.assertNotIn("150u", cooldown)

        for marker in (
            "Is_ServerDeadlinePending(Player.iServerTick, Player.iSilenceEndTick)",
            "Update_SilenceRSlotMask(m_pHUDRuntimeView.get(), player)",
            'Set_SlotVisible("Skill_R_SilenceMask", bSilenced)',
        ):
            self.assertIn(marker, main_app)
        silence_mask = main_app[
            main_app.index("void Update_SilenceRSlotMask"):
            main_app.index("CMainApp* CMainApp::s_pActiveInstance")
        ]
        self.assertNotIn("Skill_R_Icon", silence_mask)
        self.assertNotIn("Skill_R_Cooldown", silence_mask)
        self.assertNotIn("Set_SlotTint", silence_mask)
        self.assertNotIn("for (", silence_mask)

        slots_by_id = {slot["id"]: slot for slot in hud_layout["slots"]}
        for input_slot in ("Q", "W", "E", "R", "A", "S", "D", "F", "T", "V"):
            layer = slots_by_id[f"Skill_{input_slot}_Cooldown"]["layers"][0]
            self.assertEqual("UI/Common/White1x1.png", layer["path"])
            self.assertEqual([0, 0, 0, 150 / 255], layer["tint"])

        r_icon = slots_by_id["Skill_R_Icon"]
        r_mask = slots_by_id["Skill_R_SilenceMask"]
        self.assertEqual(r_icon["rect"], r_mask["rect"])
        self.assertEqual(
            "UI/ItemUpgrade/buildup_lock_icon.png",
            r_mask["layers"][0]["path"],
        )
        self.assertEqual([1, 1, 1, 1], r_mask["layers"][0]["tint"])


if __name__ == "__main__":
    unittest.main()
