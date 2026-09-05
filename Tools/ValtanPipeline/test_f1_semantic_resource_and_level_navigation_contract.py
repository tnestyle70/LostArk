"""Static regression for the compact F1 semantic sources and typed Level routes."""

from __future__ import annotations

import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8-sig")


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unterminated function: {signature}")


class F1SemanticResourceAndLevelNavigationContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.main_cpp = read("Client/Private/MainApp.cpp")
        cls.main_h = read("Client/Public/MainApp.h")
        cls.character_select_cpp = read(
            "Client/Private/Level_CharacterSelect.cpp"
        )
        cls.character_select_h = read(
            "Client/Public/Level_CharacterSelect.h"
        )

    def test_authoring_sources_are_semantic_and_open_real_owners(self) -> None:
        refresh = function_body(
            self.main_cpp, "void CMainApp::RefreshDebugAuthoringSources()"
        )
        open_source = function_body(
            self.main_cpp, "void CMainApp::OpenDebugAuthoringSource("
        )
        for token in (
            '"Valtan Action Composition"',
            '"Data/Valtan/Valtan.gameplay.json"',
            '"Valtan Animation & Sequence Source"',
            '"Data/Valtan/Valtan.presentation.json"',
            '"Valtan Effect Resources"',
            '"Data/Effects"',
            "effect.valtan.*",
            "boss.valtan.*",
            'L"Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json"',
            '"Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json"',
            '"Data/Balance/BossProfiles.json"',
            '"Data/Encounters/Valtan/ValtanEncounter.json"',
        ):
            self.assertIn(token, refresh)
        for token in (
            "EnsureDebugTool(source.eTool)",
            "EnsureDebugTool(DEBUG_TOOL::VALTAN_ACTION_WORKBENCH)",
            "m_pValtanActionWorkbench->Open_Valtan()",
            "m_pValtanActionWorkbench->Has_DisplaySnapshot()",
            "canonical Product snapshot in read-only mode",
            "canonical Product admission was rejected",
            "Open_ValtanAllEffectsWorkspace()",
            "Select its semantic row to edit in Detail",
        ):
            self.assertIn(token, open_source)
        for forbidden in ("copy_file", "ofstream", "remove(", "rename("):
            self.assertNotIn(forbidden, refresh + open_source)

        self.assertNotIn(
            "recursive_directory_iterator",
            refresh,
            "opening F1 must not recursively enumerate Data to count rows",
        )

    def test_f1_has_no_raw_resource_index(self) -> None:
        render = function_body(
            self.main_cpp, "void CMainApp::RenderDebugResourceFiles()"
        )
        for token in (
            'BeginChild(\n\t\t"CanonicalAuthoringSources", ImVec2(0.f, 300.f)',
            "Detailed Pattern, Sequence, Effect, Sound and Resource rows load inside the selected Tool",
            "F1 does not build a raw physical-file index",
        ):
            self.assertIn(token, render)
        for forbidden in (
            "Diagnostics / Raw File Index",
            "Build Raw Index",
            "RefreshDebugResourceFiles()",
            "m_DebugResourceFiles",
            "recursive_directory_iterator",
        ):
            self.assertNotIn(forbidden, render)

    def test_f1_complete_play_does_no_disk_admission_per_frame(self) -> None:
        render = function_body(
            self.main_cpp, "void CMainApp::RenderCompletePlayControls()"
        )
        self.assertIn("Load Complete Play Inventory", render)
        self.assertIn("Inventory is loaded only on request", render)
        self.assertEqual(1, render.count("RefreshCompletePlayPatternOptions()"))
        for forbidden in (
            "Can_Play_ServerPattern",
            "make_unique<CBalanceTool>",
            "ImGuiTreeNodeFlags_DefaultOpen",
        ):
            self.assertNotIn(forbidden, render)

    def test_raw_open_status_does_not_claim_arbitrary_asset_editing(self) -> None:
        open_raw = function_body(
            self.main_cpp, "void CMainApp::OpenDebugResourceFile("
        )
        for token in (
            "Read-only raw path selected",
            "this arbitrary file was not loaded",
            "m_pValtanActionWorkbench->Open_Valtan()",
            "Open_KoukuSaydonProfile",
        ):
            self.assertIn(token, open_raw)

    def test_level_navigation_uses_typed_existing_routes(self) -> None:
        request = function_body(
            self.main_cpp, "bool_t CMainApp::RequestDebugLevelNavigation("
        )
        render = function_body(
            self.main_cpp, "void CMainApp::RenderDebugLevelNavigation()"
        )
        for token in (
            "CLevel_Lobby::Submit_ProductCommand(stage)",
            "Debug_Request_ProductStage(stage)",
            "CLobbyCommandService::Request(stage, token)",
            "CLevelTransitionService::Request_Load(",
            "Debug_Request_KakulSaydonArena()",
            "LOBBY_STAGE::CHARACTER_SELECT",
            "press KoukuSaydon again after admission",
        ):
            self.assertIn(token, request)
        for forbidden in (
            "Change_Level",
            "CNetworkManager",
            "Send_DebugEnterKakulSaydonArena",
            "LOBBY_STAGE::KAKUL",
        ):
            self.assertNotIn(forbidden, request)
        for token in (
            'SeparatorText("Level Navigation")',
            '"Current: %s"',
            '"| Pending: %s"',
            'LEVEL::LOBBY, "Lobby"',
            'LEVEL::CHARACTER_SELECT, "Character Select"',
            'LEVEL::BERN, "Bern"',
            'LEVEL::VALTAN_ARENA, "Valtan"',
            'LEVEL::KAKULSAYDON_ARENA, "KoukuSaydon"',
            "ImGui::BeginDisabled(disable)",
            "timed out after 15 seconds and was unlocked",
            "m_bDebugLevelNavigationDeadlineActive = false",
        ):
            self.assertIn(token, render)

    def test_kakul_wrapper_is_the_character_select_sink_owner(self) -> None:
        wrapper = function_body(
            self.character_select_cpp,
            "bool_t CLevel_CharacterSelect::Debug_Request_KakulSaydonArena()",
        )
        for token in (
            "MODE::SERVER_ARENA",
            "CLevelTransitionService::Is_Pending()",
            "m_pWorldEntityCommandSink->Request_EnterKakulSaydonArena(",
            "m_iNextKakulArenaRequestSequence",
        ):
            self.assertIn(token, wrapper)
        self.assertNotIn("CNetworkManager", wrapper)
        panel = function_body(
            self.character_select_cpp,
            "void CLevel_CharacterSelect::Render_SelectionPanel()",
        )
        self.assertIn("Debug_Request_KakulSaydonArena()", panel)

    def test_level_panel_precedes_resource_files(self) -> None:
        render = function_body(
            self.main_cpp, "void CMainApp::RenderDeveloperTools()"
        )
        self.assertLess(
            render.index("RenderDebugLevelNavigation();"),
            render.index("RenderDebugResourceFiles();"),
        )
        for token in (
            "RenderDebugLevelNavigation",
            "RequestDebugLevelNavigation",
            "m_eDebugLevelNavigationTarget",
            "m_DebugAuthoringSources",
        ):
            self.assertIn(token, self.main_h)

    def test_declared_canonical_sources_exist(self) -> None:
        for relative in (
            "Data/Valtan/Valtan.gameplay.json",
            "Data/Valtan/Valtan.presentation.json",
            "Data/Effects/EffectCatalog.json",
            "Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json",
            "Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json",
            "Data/Balance/BossProfiles.json",
            "Data/Encounters/Valtan/ValtanEncounter.json",
        ):
            self.assertTrue((ROOT / relative).is_file(), relative)


if __name__ == "__main__":
    unittest.main()
