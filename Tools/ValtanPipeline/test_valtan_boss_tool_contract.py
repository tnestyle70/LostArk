#!/usr/bin/env python3
"""Static admission for the minimal Server-authoritative Valtan Boss Tool."""

from __future__ import annotations

import pathlib
import json
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
BOSS_CPP = ROOT / "Client/Private/BossTool.cpp"
BOSS_H = ROOT / "Client/Public/BossTool.h"
BALANCE_CPP = ROOT / "Client/Private/BalanceTool.cpp"
BALANCE_H = ROOT / "Client/Public/BalanceTool.h"
EFFECT_CPP = ROOT / "Client/Private/Effect_Tool.cpp"
MAIN_CPP = ROOT / "Client/Private/MainApp.cpp"
MAIN_H = ROOT / "Client/Public/MainApp.h"
LEVEL_CPP = ROOT / "Client/Private/Level_ValtanArena.cpp"
TREE_CPP = ROOT / "Client/Private/ValtanPatternTree.cpp"
TREE_H = ROOT / "Client/Public/ValtanPatternTree.h"
HUD_H = ROOT / "Client/Public/CombatHUDViewModel.h"
NETWORK_CPP = ROOT / "Client/Private/NetworkManager.cpp"
NETWORK_H = ROOT / "Client/Public/NetworkManager.h"
SERVER_ROOM = ROOT / "Server/Private/GameRoom.cpp"
SERVER_TESTS = ROOT / "Server/Private/ServerGameplayContractTests.cpp"
PROJECT = ROOT / "Client/Default/Client.vcxproj"
FILTERS = ROOT / "Client/Default/Client.vcxproj.filters"
GAMEPLAY = ROOT / "Data/Valtan/Valtan.gameplay.json"

EXPECTED_CORE_PATTERN_IDS = [
    "VALTAN_WHIRLWIND",
    "VALTAN_FOUR_SLASH",
    "VALTAN_HIGH_JUMP",
    "VALTAN_DASH_CHARGE",
    "VALTAN_FLOOR_WIPE_130",
    "VALTAN_ARENA_BREAK_109",
    "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
    "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK",
]

EXPECTED_TERRAIN_CORE_CONTRACTS = {
    "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK": {
        "healthBar": 84,
        "impactActionId": "valtan.mechanic.terrain-destruction-3.impact",
        "worldEventSetId":
            "worldeventset.valtan.terrain-destruction-3.floor84",
    },
    "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK": {
        "healthBar": 30,
        "impactActionId": "valtan.mechanic.terrain-destruction-9.impact",
        "worldEventSetId":
            "worldeventset.valtan.terrain-destruction-9.floor30",
    },
}


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for cursor in range(opening, len(source)):
        token = source[cursor]
        if token == "{":
            depth += 1
        elif token == "}":
            depth -= 1
            if depth == 0:
                return source[opening : cursor + 1]
    raise AssertionError(f"unterminated function body: {signature}")


class ValtanBossToolContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.boss_cpp = BOSS_CPP.read_text(encoding="utf-8")
        cls.boss_h = BOSS_H.read_text(encoding="utf-8")
        cls.balance_cpp = BALANCE_CPP.read_text(encoding="utf-8")
        cls.balance_h = BALANCE_H.read_text(encoding="utf-8")
        cls.effect_cpp = EFFECT_CPP.read_text(encoding="utf-8")
        cls.main_cpp = MAIN_CPP.read_text(encoding="utf-8")
        cls.main_h = MAIN_H.read_text(encoding="utf-8")
        cls.level_cpp = LEVEL_CPP.read_text(encoding="utf-8")
        cls.tree_cpp = TREE_CPP.read_text(encoding="utf-8")
        cls.tree_h = TREE_H.read_text(encoding="utf-8")
        cls.hud_h = HUD_H.read_text(encoding="utf-8")
        cls.network_cpp = NETWORK_CPP.read_text(encoding="utf-8")
        cls.network_h = NETWORK_H.read_text(encoding="utf-8")
        cls.server_room = SERVER_ROOM.read_text(encoding="utf-8")
        cls.server_tests = SERVER_TESTS.read_text(encoding="utf-8")
        cls.project = PROJECT.read_text(encoding="utf-8")
        cls.filters = FILTERS.read_text(encoding="utf-8")
        cls.gameplay = json.loads(GAMEPLAY.read_text(encoding="utf-8"))

    def test_project_and_f1_hub_register_one_boss_tool(self) -> None:
        for source in (self.project, self.filters):
            self.assertEqual(1, source.count("BossTool.h"))
            self.assertEqual(1, source.count("BossTool.cpp"))
        self.assertIn("BOSS", self.main_h)
        self.assertIn("unique_ptr<CBossTool>", self.main_h)
        ensure = function_body(
            self.main_cpp,
            "HRESULT CMainApp::EnsureDebugTool(const DEBUG_TOOL eTool)",
        )
        self.assertIn("case DEBUG_TOOL::BOSS", ensure)
        self.assertIn("m_pBossTool->Open()", ensure)
        self.assertEqual(1, self.main_cpp.count('toolButton("Boss Tool"'))

    def test_tool_reuses_product_views_and_typed_server_commands(self) -> None:
        for marker in (
            "CValtanPatternTree::Load",
            "CValtanPatternAuditionService::Get().Submit",
            "CCombatHUDViewModel::Get().Get_Boss",
            "CEffectDocumentCodec::Load",
            "CValtanCinematicCameraDocument",
            "Request_RevivePlayer",
        ):
            self.assertIn(marker, self.boss_cpp + self.boss_h)
        for forbidden in (
            "Send_ValtanPattern",
            "PlayAnimation",
            "Play_Animation",
            "Spawn_Effect",
            "Create_Effect",
            "CDataJson::Parse",
        ):
            self.assertNotIn(forbidden, self.boss_cpp)

    def test_default_screen_keeps_only_the_user_workflow(self) -> None:
        render = function_body(self.boss_cpp, "void Client::CBossTool::Render()")
        for marker in (
            '"Valtan Boss Tool"',
            'BeginTabBar("##bossToolTabs")',
            'BeginTabItem("Boss Verification")',
            'BeginTabItem("Pattern Flow")',
            "Render_BossVerificationTab()",
            "Render_PatternFlowTab()",
        ):
            self.assertIn(marker, render)
        self.assertLess(
            render.index('BeginTabItem("Boss Verification")'),
            render.index('BeginTabItem("Pattern Flow")'),
        )
        verification = function_body(
            self.boss_cpp,
            "void Client::CBossTool::Render_BossVerificationTab()",
        )
        for marker in (
            "Render_LiveSummary()",
            "Render_ActionBar()",
            "Render_PatternList()",
            "Render_SelectedPattern()",
        ):
            self.assertIn(marker, verification)
        for marker in (
            'ImGui::Button("Play Selected")',
            'ImGui::Checkbox("Repeat"',
            'ImGui::Button("Stop After Current")',
            'ImGui::CollapsingHeader("Why / Advanced diagnostics")',
            '"Animation"',
            '"Effect"',
            '"Camera"',
            '"Hit / Motion"',
            '"World"',
            '"Next"',
        ):
            self.assertIn(marker, self.boss_cpp)
        self.assertNotIn("InputFloat", self.boss_cpp)
        self.assertNotIn("SliderFloat", self.boss_cpp)

    def test_initial_selection_is_empty_and_live_follow_is_exact(self) -> None:
        reload_body = function_body(
            self.boss_cpp,
            "bool_t Client::CBossTool::Reload_Graph()",
        )
        self.assertNotIn("m_Graph.Gimmicks.front().strPatternId", reload_body)
        self.assertNotIn("m_Graph.Rotation.front().strPatternId", reload_body)
        sync = function_body(
            self.boss_cpp,
            "void Client::CBossTool::Synchronize_LiveSelection()",
        )
        self.assertIn("Find_AuditionPattern(Boss.strPatternId)", sync)
        self.assertIn("Find_LiveStage(*pPattern)", sync)
        submit = function_body(
            self.boss_cpp,
            "bool_t Client::CBossTool::Submit_SelectedPattern()",
        )
        self.assertIn("m_bFollowLive = true", submit)
        find_live = function_body(
            self.boss_cpp,
            "const Client::VALTAN_STAGE_VIEW* Client::CBossTool::Find_LiveStage(",
        )
        self.assertIn("Candidate.strActionId == Boss.strActionId", find_live)
        self.assertNotIn("iPatternStageIndex", find_live)
        self.assertIn("No pattern matches this search.", self.boss_cpp)

    def test_selector_is_exactly_the_all_effects_eight_plus_twenty(self) -> None:
        inventory = function_body(
            self.tree_cpp,
            "bool_t Client::CValtanPatternTree::Build_ToolAuditionInventory(",
        )
        for marker in (
            "VALTAN_TOOL_AUDITION_CORE_PATTERN_IDS",
            "View.ManualAuditions",
            "CORE_PATTERN_COUNT",
            "ANIMATOR_PATTERN_COUNT",
            "TOTAL_PATTERN_COUNT",
            "UniquePatternIds",
            "pPattern->bManualServerAudition",
        ):
            self.assertIn(marker, inventory)

        core_source = self.tree_cpp[
            self.tree_cpp.index("VALTAN_TOOL_AUDITION_CORE_PATTERN_IDS") :
            self.tree_cpp.index("bool Read_TextDocument")
        ]
        positions = [core_source.index(f'"{pattern_id}"')
                     for pattern_id in EXPECTED_CORE_PATTERN_IDS]
        self.assertEqual(sorted(positions), positions)

        manual_ids = [
            row["patternId"]
            for row in self.gameplay["decisionModel"]["manualAuditions"]
        ]
        visible_ids = EXPECTED_CORE_PATTERN_IDS + manual_ids
        self.assertEqual(20, len(manual_ids))
        self.assertEqual(28, len(visible_ids))
        self.assertEqual(28, len(set(visible_ids)))
        for excluded in (
            "VALTAN_SWING",
            "VALTAN_FIST_IN_OUT",
            "VALTAN_ARMOR_BREAK_OPENING",
        ):
            self.assertNotIn(excluded, visible_ids)

        pattern_list = function_body(
            self.boss_cpp,
            "void Client::CBossTool::Render_PatternList()",
        )
        for marker in (
            "m_AuditionInventory.CorePatternIds",
            "m_AuditionInventory.AnimatorPatternIds",
            '"CORE SERVER PATTERNS (8)"',
            '"ANIMATOR PATTERNS (20)"',
            "Find_AuditionPattern",
        ):
            self.assertIn(marker, pattern_list)
        for forbidden in (
            "m_Graph.Gimmicks",
            "m_Graph.Rotation",
            "m_Graph.LegacyRotations",
        ):
            self.assertNotIn(forbidden, pattern_list)
        self.assertIn(
            "CValtanPatternTree::Build_ToolAuditionInventory",
            self.effect_cpp,
        )

    def test_new_core_rows_keep_the_exact_server_audition_tuples(self) -> None:
        scripted_ids = self.gameplay["decisionModel"]["scriptedSequence"][
            "patternIds"
        ]
        patterns = {
            row["patternId"]: row for row in self.gameplay["patterns"]
        }
        mechanics = {
            row["patternId"]: row
            for row in self.gameplay["decisionModel"]["mechanics"]
        }

        for pattern_id, expected in EXPECTED_TERRAIN_CORE_CONTRACTS.items():
            with self.subTest(pattern_id=pattern_id):
                self.assertIn(pattern_id, scripted_ids)
                mechanic = mechanics[pattern_id]
                self.assertEqual(
                    {
                        "kind": "HEALTH_BAR_CROSSING",
                        "healthBar": expected["healthBar"],
                    },
                    mechanic["trigger"],
                )
                self.assertEqual(1, mechanic["triggerOrder"])
                self.assertTrue(mechanic["oncePerEncounter"])

                stages = patterns[pattern_id]["stages"]
                self.assertEqual(4, len(stages))
                impact = stages[3]
                self.assertEqual("IMPACT", impact["stageId"])
                self.assertEqual(
                    expected["impactActionId"], impact["actionId"]
                )
                self.assertEqual(
                    [
                        (
                            "ENTER",
                            "TRIGGER_WORLD_EVENT_SET",
                            expected["worldEventSetId"],
                        )
                    ],
                    [
                        (
                            event["trigger"],
                            event["kind"],
                            event["worldEventSetId"],
                        )
                        for event in impact["events"]
                    ],
                )

        self.assertLess(
            scripted_ids.index("VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK"),
            scripted_ids.index("VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK"),
        )

        boss_submit = function_body(
            self.boss_cpp,
            "bool_t Client::CBossTool::Submit_SelectedPattern()",
        )
        effect_submit = function_body(
            self.effect_cpp,
            "bool_t Client::CEffect_Tool::Try_PlayValtanServerPattern(",
        )
        self.assertIn("m_strSelectedPatternId", boss_submit)
        self.assertIn("Pattern.strPatternId", effect_submit)

    def test_hidden_graph_patterns_cannot_reenter_selection_or_repeat(self) -> None:
        for signature in (
            "bool_t Client::CBossTool::Submit_SelectedPattern()",
            "void Client::CBossTool::Render_ActionBar()",
            "void Client::CBossTool::Render_SelectedPattern()",
            "void Client::CBossTool::Synchronize_LiveSelection()",
        ):
            body = function_body(self.boss_cpp, signature)
            self.assertIn("Find_AuditionPattern", body, signature)

        reload_body = function_body(
            self.boss_cpp,
            "bool_t Client::CBossTool::Reload_Graph()",
        )
        self.assertIn("Build_ToolAuditionInventory", reload_body)
        self.assertIn("m_strSelectedPatternId.clear()", reload_body)
        self.assertIn("m_strSelectedStageId.clear()", reload_body)
        self.assertIn("m_bRepeat = false", reload_body)
        self.assertIn("m_strRepeatPatternId.clear()", reload_body)

        update = function_body(
            self.boss_cpp,
            "void Client::CBossTool::Update(const bool_t bToolVisible)",
        )
        self.assertLess(
            update.index("Find_AuditionPattern(m_strRepeatPatternId)"),
            update.rindex("Submit_SelectedPattern()"),
        )

        live_summary = function_body(
            self.boss_cpp,
            "void Client::CBossTool::Render_LiveSummary()",
        )
        self.assertIn("Find_Pattern(Boss.strPatternId)", live_summary)
        self.assertNotIn("Find_AuditionPattern(Boss.strPatternId)", live_summary)
        self.assertIn("[live only; outside All Effects list]", live_summary)
        self.assertIn("pPattern->strDisplayName", live_summary)
        self.assertIn("ImGui::TextWrapped", live_summary)
        self.assertNotIn("ImGui::SameLine()", live_summary)

        pattern_list = function_body(
            self.boss_cpp,
            "void Client::CBossTool::Render_PatternList()",
        )
        self.assertIn("Repeat stopped after selecting a different Pattern", pattern_list)
        self.assertIn("m_strRepeatPatternId.clear()", pattern_list)

    def test_repeat_stops_when_hidden_or_closed_and_waits_for_revive(self) -> None:
        update = function_body(
            self.boss_cpp,
            "void Client::CBossTool::Update(const bool_t bToolVisible)",
        )
        self.assertIn("if (!bToolVisible || !m_bOpen)", update)
        self.assertIn("m_strRepeatPatternId.clear()", update)
        self.assertIn("0u == Player.iCurrentHp", update)
        self.assertIn("Revive the player to continue Repeat", update)
        self.assertLess(
            update.index("0u == Player.iCurrentHp"),
            update.rindex("Submit_SelectedPattern()"),
        )

    def test_context_actions_keep_immediate_feedback_visible(self) -> None:
        action_bar = function_body(
            self.boss_cpp,
            "void Client::CBossTool::Render_ActionBar()",
        )
        self.assertIn("m_strActionFeedback", action_bar)
        self.assertIn("Repeat stopped.", action_bar)
        self.assertIn("Request_RevivePlayer(m_strActionFeedback)", action_bar)
        self.assertIn("if (m_strActionFeedback.empty())", action_bar)
        revive = function_body(
            self.boss_cpp,
            "bool_t Client::CBossTool::Request_RevivePlayer(",
        )
        self.assertIn("Revive requested.", revive)

    def test_legacy_arena_panel_is_not_a_second_visible_replay_path(self) -> None:
        update = function_body(
            self.level_cpp,
            "void CLevel_ValtanArena::Update(f32_t fTimeDelta)",
        )
        self.assertNotIn("Render_AuditionPanel();", update)
        self.assertIn("Update_AuditionTransaction();", update)

    def test_boss_and_effect_share_replay_but_boss_alone_controls_it(self) -> None:
        for source in (self.balance_cpp, self.balance_h):
            self.assertNotIn("Play Server Pattern", source)
            self.assertNotIn("Play Server", source)
            self.assertNotIn("Try_PlayValtanServerPattern", source)
            self.assertNotIn("CValtanPatternAuditionService", source)
            self.assertNotIn("Request_RevivePlayer", source)
        for marker in (
            "CValtanPatternAuditionService::Get().Submit",
            'ImGui::SmallButton("Play Server")',
            'ImGui::SmallButton("Play Server Owner")',
            "VALTAN_EFFECT_TOOL_AUDITION_CONSUMER_ID",
        ):
            self.assertIn(marker, self.effect_cpp)
        for forbidden in (
            "Request_RevivePlayer",
            "Stop After Current",
            "Send_ValtanPatternAuditionById",
            "Try_Consume_ValtanPatternAuditionByIdResult",
        ):
            self.assertNotIn(forbidden, self.effect_cpp)
        self.assertIn("Repeat and Revive remain in Boss Tool", self.balance_cpp)
        self.assertIn("Boss Tool owns Repeat, Revive, and diagnostics", self.effect_cpp)
        action_bar = function_body(
            self.boss_cpp,
            "void Client::CBossTool::Render_ActionBar()",
        )
        self.assertEqual(1, action_bar.count("Request_RevivePlayer"))

    def test_camera_and_live_freshness_are_preserved_in_the_shared_view(self) -> None:
        for marker in (
            "VALTAN_CAMERA_INVOCATION_VIEW",
            "strCameraInvocationId",
            "strCameraCueId",
            "iStartOffsetMs",
            "strDurationPolicy",
            "CameraInvocations",
        ):
            self.assertIn(marker, self.tree_h + self.tree_cpp)
        for marker in (
            "Is_ExactCameraInvocation",
            '"Tuple mismatch: "',
            '"join %s | frame %s | origin %s"',
            "BOSS_FACING",
            "PLAYER_BOSS_FRAME",
        ):
            self.assertIn(marker, self.boss_cpp)
        for marker in (
            "iServerTick",
            "iActionStartTick",
            "PinnedDefinitionRevision",
        ):
            self.assertIn(marker, self.hud_h)
            self.assertIn(marker, self.boss_cpp)
        self.assertIn("presentation UNAVAILABLE", self.boss_cpp)
        self.assertIn("Connections are unverified", self.boss_cpp)
        for marker in (
            "Is_CurrentPresentationBaselineIntact",
            "Workspace presentation changed after world entry",
        ):
            self.assertIn(marker, self.network_h + self.network_cpp)
        self.assertIn("workspace changed; restart/publish", self.boss_cpp)
        for marker in (
            "CEffectCatalog::Find_Loaded",
            "CEffectDocumentCodec::Serialize",
            "LOCAL UNVERIFIED",
            "NEXT-SPAWN MATCHED - replay required",
            "STALE: Effect source or next-spawn catalog changed.",
            "m_ResourceSearchDocumentGenerations",
            "Gameplay rows: LOCAL AUTHORING - Server parity unverified.",
        ):
            self.assertIn(marker, self.boss_cpp)
        self.assertNotIn('"runtime matched"', self.boss_cpp)

    def test_server_holds_completed_stable_id_pattern_until_next_request(self) -> None:
        completion = function_body(
            self.server_room,
            "bool LostArk::Server::CGameRoom::Refresh_ValtanPatternIdAuditionState(",
        )
        self.assertIn(
            "boss->bAutomaticPatternSequenceAuditionOverride = true",
            completion,
        )
        self.assertIn(
            "boss->bAutomaticPatternSequenceAuditionHold = true",
            completion,
        )
        self.assertIn(
            "Hold a completed stable-ID audition idle without advancing the Product rotation",
            self.server_tests,
        )


if __name__ == "__main__":
    unittest.main()
