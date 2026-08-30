#!/usr/bin/env python3
"""Static admission for the minimal Server-authoritative Valtan Boss Tool."""

from __future__ import annotations

import pathlib
import json
import re
import sys
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/ValtanPipeline"))
import valtan_tuning_pipeline as tuning_pipeline  # noqa: E402
BOSS_CPP = ROOT / "Client/Private/BossTool.cpp"
BOSS_H = ROOT / "Client/Public/BossTool.h"
BALANCE_CPP = ROOT / "Client/Private/BalanceTool.cpp"
BALANCE_H = ROOT / "Client/Public/BalanceTool.h"
EFFECT_CPP = ROOT / "Client/Private/Effect_Tool.cpp"
EFFECT_H = ROOT / "Client/Public/Effect_Tool.h"
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
        cls.effect_h = EFFECT_H.read_text(encoding="utf-8")
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
        cls.gameplay = tuning_pipeline.resolve_gameplay_flow_reference(
            json.loads(GAMEPLAY.read_text(encoding="utf-8")),
            json.loads((ROOT / tuning_pipeline.SAVED_FLOW_REL).read_text(encoding="utf-8")),
        )

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

    def test_linked_effect_request_contains_only_exact_stable_identity(self) -> None:
        request = function_body(
            self.effect_h,
            "struct EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST final",
        )
        self.assertEqual(
            [
                "strPatternId", "strStageId", "strCueOccurrenceId",
                "strEffectAssetId",
            ],
            re.findall(r"std::string\s+(\w+)\s*;", request),
        )
        self.assertEqual(4, request.count(";"))
        render = function_body(
            self.boss_cpp,
            "void Client::CBossTool::Render_ConnectionSummary(",
        )
        compact = re.sub(r"\s+", "", render)
        self.assertIn('ImGui::Button("EditLinkedEffect")', compact)
        for destination, source in (
            ("PatternId", "Pattern.strPatternId"),
            ("StageId", "Stage.strStageId"),
            ("CueOccurrenceId", "Cue.strOccurrenceId"),
            ("EffectAssetId", "Cue.strEffectAssetId"),
        ):
            assignment = f"m_strEffectToolOpen{destination}={source};"
            self.assertIn(assignment, compact)
            self.assertLess(
                compact.index(assignment),
                compact.index("m_hasEffectToolOpenRequest=true;"),
            )
        self.assertIn("Stage.ProductCues", render)
        self.assertNotIn("Open_ValtanProductEffect", render)

    def test_linked_effect_request_is_consumed_exactly_once(self) -> None:
        consume = function_body(
            self.boss_cpp,
            "bool_t Client::CBossTool::Consume_EffectToolOpenRequest(",
        )
        compact = re.sub(r"\s+", "", consume)
        self.assertIn("if(!m_hasEffectToolOpenRequest)returnfalse;", compact)
        for field in (
            "PatternId", "StageId", "CueOccurrenceId", "EffectAssetId",
        ):
            self.assertIn(
                f"outRequest.str{field}=std::move(m_strEffectToolOpen{field});",
                compact,
            )
            self.assertIn(f"m_strEffectToolOpen{field}.clear();", compact)
        self.assertTrue(compact.endswith("m_hasEffectToolOpenRequest=false;returntrue;}"))

    def test_main_app_routes_linked_effect_without_starting_gameplay(self) -> None:
        update = function_body(self.main_cpp, "void CMainApp::Update(")
        begin = update.index("EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST effectRequest;")
        route = update[begin:update.index("if (nullptr != m_pCameraTool)", begin)]
        self.assertEqual(2, route.count("Consume_EffectToolOpenRequest(effectRequest)"))
        self.assertLess(
            route.index("EnsureDebugTool(DEBUG_TOOL::EFFECT)"),
            route.index("Open_ValtanProductEffect(effectRequest)"),
        )
        self.assertIn("nullptr != m_pEffectTool", route)
        self.assertIn("m_strToolStatus = bOpened ?", route)
        for forbidden in ("Try_Play", "Submit_SelectedPattern", "Spawn_Effect"):
            self.assertNotIn(forbidden, route)

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
            'ImGui::Button("Complete Play Selected")',
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

    def test_selector_uses_authored_definitions_without_fixed_group_counts(self) -> None:
        inventory = function_body(
            self.tree_cpp,
            "bool_t Client::CValtanPatternTree::Build_PlayablePatternInventory(",
        )
        for marker in (
            "View.ManualAuditions", "bAuthoringMasterManaged", "ManualByPattern",
            "CorePatternIds", "AnimatorPatternIds", "DerivedPatternIds",
            "IdentityCounts", "StageIds", "ActionIds",
        ):
            self.assertIn(marker, inventory)
        for retired in (
            "VALTAN_TOOL_AUDITION_CORE_PATTERN_IDS", "CORE_PATTERN_COUNT",
            "ANIMATOR_PATTERN_COUNT", "BASE_PATTERN_COUNT", "TOTAL_PATTERN_COUNT",
        ):
            self.assertNotIn(retired, self.tree_cpp + self.tree_h)
        authored_ids = [row["patternId"] for row in self.gameplay["patterns"]]
        manual_ids = [row["patternId"] for row in self.gameplay["decisionModel"]["manualAuditions"]]
        self.assertEqual(len(authored_ids), len(set(authored_ids)))
        self.assertLessEqual(set(manual_ids), set(authored_ids))
        for included in (
            "VALTAN_FIST_IN_OUT",
            "VALTAN_ENTRANCE_CINEMATIC",
            "VALTAN_ENTRANCE_CINEMATIC_IDLE",
            "VALTAN_GHOST_FINALE",
        ):
            self.assertIn(included, authored_ids)
        for ghost_wrapper in (
            "VALTAN_GHOST_RESPAWN_AUDITION",
            "VALTAN_GHOST_DEATH_AUDITION",
        ):
            self.assertIn(ghost_wrapper, manual_ids)
        for legacy_only in ("VALTAN_SWING", "VALTAN_ARMOR_BREAK_OPENING"):
            self.assertNotIn(legacy_only, authored_ids)

        pattern_list = function_body(
            self.boss_cpp, "void Client::CBossTool::Render_PatternList()")
        for marker in (
            "m_AuditionInventory.CorePatternIds", "m_AuditionInventory.AnimatorPatternIds",
            "m_AuditionInventory.DerivedPatternIds", "CORE SERVER PATTERNS",
            "ANIMATOR PATTERNS", "Find_AuditionPattern",
        ):
            self.assertIn(marker, pattern_list)
        for forbidden in (
            "m_Graph.Gimmicks", "m_Graph.Rotation", "m_Graph.LegacyRotations",
            "CORE SERVER PATTERNS (8)", "ANIMATOR PATTERNS (20)",
        ):
            self.assertNotIn(forbidden, pattern_list)
        self.assertIn("CValtanPatternTree::Build_PlayablePatternInventory", self.effect_cpp)

    def test_selected_identity_reuses_the_shared_joined_pattern_summary(self) -> None:
        self.assertIn("static std::string Build_PatternIdentitySummary(", self.tree_h)
        summary = function_body(
            self.tree_cpp,
            "std::string Client::CValtanPatternTree::Build_PatternIdentitySummary(",
        )
        for field in (
            "Pattern.strPatternId", "Pattern.iAuthoringPhase",
            "Pattern.iMinimumPhase", "Pattern.iMaximumPhase",
            "Stage.ClipOccurrences", "Clip.iSourceStartMs", "Clip.iPlayMs",
            "Clip.fPlayRate", "Clip.iAuthoringWallMs",
        ):
            self.assertIn(field, summary)
        self.assertNotIn("strDisplayName", summary)
        self.assertNotIn("RuntimeClipNames", summary)
        self.assertNotIn("std::sort", summary)
        for signature in (
            "void Client::CBossTool::Render_SelectedPattern()",
            "void Client::CBossTool::Render_PatternList()",
        ):
            with self.subTest(signature=signature):
                selected = function_body(self.boss_cpp, signature)
                self.assertIn(
                    "CValtanPatternTree::Build_PatternIdentitySummary(*pPattern)",
                    selected,
                )
                self.assertIn("pPattern->strDisplayName", selected)

    def test_new_core_rows_keep_the_exact_server_audition_tuples(self) -> None:
        patterns = {
            row["patternId"]: row for row in self.gameplay["patterns"]
        }
        mechanics = {
            row["patternId"]: row
            for row in self.gameplay["decisionModel"]["mechanics"]
        }

        for pattern_id, expected in EXPECTED_TERRAIN_CORE_CONTRACTS.items():
            with self.subTest(pattern_id=pattern_id):
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
        self.assertIn("Build_PlayablePatternInventory", reload_body)
        self.assertIn("StagedAdmittedPatternIds", reload_body)
        self.assertIn("CValtanPatternFlowDocument StagedFlowDocument", reload_body)
        self.assertIn("StagedFlowDocument.Verify_SourceRevision", reload_body)
        self.assertIn("StagedGraph.strSavedFlowSourceRevision", reload_body)
        self.assertIn(
            "strSavedFlowSourceRevision", self.tree_h + self.tree_cpp
        )
        self.assertGreaterEqual(
            self.tree_cpp.count("CValtanPatternFlowDocument::MAX_SLOTS"), 2
        )
        self.assertIn("CValtanPatternFlowDocument::Validate(", reload_body)
        self.assertLess(
            reload_body.index("CValtanPatternFlowDocument::Validate("),
            reload_body.index("m_Graph = std::move(StagedGraph)"),
        )
        self.assertLess(
            reload_body.index("StagedGraph.strSavedFlowSourceRevision"),
            reload_body.index("m_Graph = std::move(StagedGraph)"),
        )
        self.assertLess(
            reload_body.index("m_FlowDocument = std::move(StagedFlowDocument)"),
            reload_body.index("m_bGraphReady = true"),
        )
        for marker in (
            "bSelectedIsEntry",
            "bFirstIsEntry",
            "bWouldCrossEntry",
        ):
            self.assertIn(marker, self.boss_cpp)
        self.assertIn("graph reload was not committed", reload_body)
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

    def test_boss_and_effect_share_replay_through_main_app_selection(self) -> None:
        for source in (self.balance_cpp, self.balance_h):
            self.assertNotIn("Play Server Pattern", source)
            self.assertNotIn("Play Server", source)
            self.assertNotIn("Try_PlayValtanServerPattern", source)
            self.assertNotIn("CValtanPatternAuditionService", source)
            self.assertNotIn("Request_RevivePlayer", source)
        for marker in (
            "Debug_SelectCompletePlayPattern",
            "Debug_CompletePlaySelected",
            'ImGui::SmallButton("Complete Play (Server/Arena)")',
            'ImGui::SmallButton("Complete Play Owner")',
        ):
            self.assertIn(marker, self.effect_cpp)
        self.assertNotIn(
            "CValtanPatternAuditionService::Get().Submit",
            self.effect_cpp,
        )
        self.assertNotIn(
            "VALTAN_EFFECT_TOOL_AUDITION_CONSUMER_ID",
            self.effect_cpp,
        )
        self.assertIn("m_pBossTool->Play_ServerPattern", self.main_cpp)
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

    def test_next_picker_is_independent_of_saved_flow_and_turns_repeat_off(self) -> None:
        picker = function_body(self.boss_cpp, "void Client::CBossTool::Render_NextPatternPicker()")
        for marker in (
            "m_NextPatternIds", "m_NextPatternSearch", "Contains_CaseInsensitive",
            "strDisplayName", "PatternId", "bAuthoringMasterManaged",
            "Queue_NextPattern", "compatibility manual",
        ):
            self.assertIn(marker, picker)
        for forbidden in ("Add_Slot", "m_FlowDocument", ".Submit(", "Send_", "CNetworkManager"):
            self.assertNotIn(forbidden, picker)
        self.assertLess(picker.index("m_bRepeat = false"), picker.index("Queue_NextPattern"))
        self.assertLess(picker.index("m_strRepeatPatternId.clear()"), picker.index("Queue_NextPattern"))
        admitted = function_body(self.boss_cpp, "std::vector<std::string> Client::CBossTool::Build_AdmittedPatternIds() const")
        self.assertNotIn("m_NextPatternIds", admitted)
        self.assertIn("m_AuditionInventory", admitted)

    def test_next_card_survives_graph_flow_and_selected_slot_failures(self) -> None:
        layout = function_body(self.boss_cpp, "void Client::CBossTool::Render_PatternFlowTab()")
        self.assertNotIn("return;", layout)
        self.assertIn("fColumnHeight", layout)
        self.assertLess(layout.index("##bossFlowSelectedPane"), layout.index("##bossNextPatternCard"))
        self.assertIn("ImGui::EndChild();", layout[layout.index("Render_FlowSelectedSlot();"):layout.index("##bossNextPatternCard")])
        card = function_body(self.boss_cpp, "void Client::CBossTool::Render_NextPatternCard()")
        for marker in ("Get_Snapshot", "Get_NextSnapshot", "Get_NextCommand",
                       "Clear_NextPattern", "Retry_NextPatternCommand", "UNCONFIRMED"):
            self.assertIn(marker, card)
        self.assertIn("Service.Can_QueueNextPattern", card)
        self.assertIn("!Next.Is_Live() || Next.bReservationConsumed ||", card)
        picker = function_body(self.boss_cpp, "void Client::CBossTool::Render_NextPatternPicker()")
        self.assertIn("Service.Can_QueueNextPattern", picker)
        for forbidden in ("m_FlowDocument", "Find_SelectedFlowSlot", "Is_Dirty", "Has_ExternalConflict"):
            self.assertNotIn(forbidden, card)
        reload_graph = function_body(self.boss_cpp, "bool_t Client::CBossTool::Reload_Graph()")
        self.assertNotIn("m_bNextPatternInventoryReady = false", reload_graph)
        self.assertIn("previous playable inventory preserved", reload_graph)
        self.assertLess(reload_graph.index("Build_NextPatternInventory("), reload_graph.index("m_Graph ="))
        self.assertLess(reload_graph.index("m_NextPatternIds ="), reload_graph.index("m_bNextPatternInventoryReady = true"))
        for function in ("Reload_FlowDocument", "Save_FlowDocument"):
            body = function_body(self.boss_cpp, "bool_t Client::CBossTool::" + function + "()")
            for forbidden in ("Queue_NextPattern", "Clear_NextPattern", ".Submit(", "Send_"):
                self.assertNotIn(forbidden, body)

    def test_next_ownership_guards_repeat_flow_and_other_tools(self) -> None:
        for signature in (
            "void Client::CBossTool::Update(const bool_t bToolVisible)",
            "void Client::CBossTool::Render_ActionBar()",
            "void Client::CBossTool::Render_FlowSelectedSlot()",
        ):
            self.assertIn("Has_PlaybackOwnership", function_body(self.boss_cpp, signature))
        action_bar = function_body(self.boss_cpp, "void Client::CBossTool::Render_ActionBar()")
        repeat_begin = action_bar.index('ImGui::Checkbox("Repeat"')
        self.assertIn("bNextOwnsPlayback", action_bar[:repeat_begin])
        service = (ROOT / "Client/Private/ValtanPatternAuditionService.cpp").read_text(encoding="utf-8")
        submit = function_body(service, "bool Client::CValtanPatternAuditionService::Submit(")
        self.assertIn("Has_PlaybackOwnership() || Is_FlowInFlight()", submit)
        flow = (ROOT / "Client/Private/ValtanPatternFlowService.cpp").read_text(encoding="utf-8")
        start = function_body(flow, "bool Client::CValtanPatternFlowService::Start(")
        self.assertIn("Has_PendingStart", start)
        self.assertIn("Has_PendingNextCommand", start)
        self.assertIn("VALTAN_PATTERN_FLOW_START_STATE::WAITING_VERDICT", start)
        self.assertIn("CValtanPatternAuditionService::Get().Update();", self.main_cpp)

    def test_next_wire_stays_behind_single_shared_service_queue(self) -> None:
        send = function_body(self.network_cpp, "bool CNetworkManager::Send_ValtanNextPatternCommand(")
        self.assertIn("Write_Message(payloadWriter, message)", send)
        for operation in ("PLAY_PATTERN_ID", "QUEUE_NEXT_PATTERN_ID", "QUEUE_NEXT_LIVE_PATTERN_ID", "CLEAR_NEXT_PATTERN_ID"):
            self.assertIn("VALTAN_AUDITION_OPERATION::" + operation + " == result.eOperation", self.network_cpp)
        service = (ROOT / "Client/Private/ValtanPatternAuditionService.cpp").read_text(encoding="utf-8")
        update = function_body(service, "void Client::CValtanPatternAuditionService::Update()")
        self.assertLess(update.index("if (!Is_Connected() || bWorldChanged)"), update.index("while (Consume_Result(Result))"))
        retry = function_body(service, "bool Client::CValtanPatternAuditionService::Retry_NextPatternCommand(")
        self.assertIn("Send_Request(m_NextCommand.Request)", retry)
        self.assertNotIn("Advance_RequestSequence", retry)

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
