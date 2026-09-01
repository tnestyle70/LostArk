import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def read(relative_path: str) -> str:
    return (ROOT / relative_path).read_text(encoding="utf-8-sig")


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unterminated function: {signature}")


class ValtanF1ArenaPreservationContractTests(unittest.TestCase):
    def test_complete_play_is_an_explicit_boss_only_server_action(self) -> None:
        main = read("Client/Private/MainApp.cpp")
        render = function_body(main, "void CMainApp::RenderCompletePlayControls()")
        submit = function_body(main, "bool_t CMainApp::Debug_CompletePlaySelected(")

        self.assertIn("Valtan Complete Play (Server Boss Replay)", render)
        self.assertIn("ImGui::Button(\"Complete Play##GlobalServerPattern\")", render)
        self.assertIn("Debug_CompletePlaySelected", render)
        self.assertIn("arena walls, floors, debris, collision, and Nav state are preserved", render)
        self.assertIn("Play_ServerPattern", submit)
        for forbidden in (
            "Set_ServerArenaPreset",
            "Submit_Audition",
            "Set_Visible",
            "Set_Position",
            "Set_WorldMatrix",
        ):
            self.assertNotIn(forbidden, render + submit)

    def test_complete_play_status_tracks_the_matching_server_lifecycle(self) -> None:
        main = read("Client/Private/MainApp.cpp")
        boss = read("Client/Private/BossTool.cpp")
        header = read("Client/Public/BossTool.h")
        render = function_body(main, "void CMainApp::RenderCompletePlayControls()")
        status = function_body(boss, "bool_t Client::CBossTool::Get_ServerPatternStatus(")

        self.assertIn("Get_ServerPatternStatus", header)
        self.assertIn("m_bCompletePlayStatusTracking", render)
        self.assertIn("Get_ServerPatternStatus", render)
        self.assertIn("Get_Snapshot", status)
        self.assertIn("CONSUMER_ID", status)
        self.assertIn("strPatternId", status)
        self.assertIn("Is_InFlight", status)

    def test_complete_play_waits_for_the_saved_server_revision(self) -> None:
        main = read("Client/Private/MainApp.cpp")
        boss = read("Client/Private/BossTool.cpp")
        render = function_body(main, "void CMainApp::RenderCompletePlayControls()")
        gate = function_body(
            boss, "bool_t Client::CBossTool::Get_ServerActivePatternRevision("
        )
        submit = function_body(
            boss, "bool_t Client::CBossTool::Submit_SelectedPattern()"
        )

        self.assertNotIn("Can_Play_ServerPattern", render)
        self.assertNotIn("make_unique<CBalanceTool>", render)
        self.assertIn("validated once when Complete Play is pressed", render)
        self.assertIn("Acquire_ServerPlaybackAdmission", submit)
        self.assertIn("Has_PendingCommand", gate)
        self.assertIn("strCandidateRevision", gate)
        self.assertIn("bCandidateIsServerActive", gate)
        self.assertIn("Server-active revision", gate)
        self.assertIn("ServerActiveRevision", gate)
        self.assertIn("expectedActiveRevision", submit)
        self.assertIn("CValtanPatternAuditionService::Get().Submit", submit)

    def test_arena_presets_are_five_explicit_buttons_with_clear_labels(self) -> None:
        main = read("Client/Private/MainApp.cpp")
        render = function_body(main, "void CMainApp::RenderServerArenaActiveControls()")

        self.assertIn("if (ImGui::SmallButton(label))", render)
        self.assertEqual(1, render.count("Set_ServerArenaPreset("))
        for label in (
            "Fresh / Restore Entire Arena##GlobalArenaPreset",
            "Circle / Remove All Walls##GlobalArenaPreset",
            "Break 3 O'Clock Floor##GlobalArenaPreset",
            "Break 9 O'Clock Floor##GlobalArenaPreset",
            "Final / Break 3 + 9 O'Clock Floors##GlobalArenaPreset",
        ):
            self.assertIn(label, render)
        for preset in (
            "FRESH",
            "CIRCLE_WALLS_GONE",
            "THREE_OCLOCK_BROKEN",
            "NINE_OCLOCK_BROKEN",
            "BOTH_SIDES_BROKEN",
        ):
            self.assertEqual(1, render.count(f"VALTAN_ARENA_PRESET::{preset}"))
        for forbidden in ("Set_Visible", "Set_Position", "Set_WorldMatrix"):
            self.assertNotIn(forbidden, render)

    def test_arena_status_polls_the_exact_server_transaction(self) -> None:
        main = read("Client/Private/MainApp.cpp")
        boss = read("Client/Private/BossTool.cpp")
        level = read("Client/Private/Level_ValtanArena.cpp")
        render = function_body(main, "void CMainApp::RenderServerArenaActiveControls()")
        pending = function_body(
            level, "bool_t CLevel_ValtanArena::Is_ArenaPresetRequestPending() const"
        )

        self.assertIn("m_bServerArenaPresetStatusTracking", render)
        self.assertIn("Get_ServerArenaPresetStatus", render)
        self.assertIn("Is_ServerArenaPresetPending", render)
        self.assertIn("Get_ArenaAuditionStatus", boss)
        self.assertIn("Is_ArenaPresetRequestPending", boss)
        self.assertIn("SET_ARENA_PRESET", pending)

    def test_active_state_is_read_only_replication_projection(self) -> None:
        level = read("Client/Private/Level_ValtanArena.cpp")
        active = function_body(
            level, "CLevel_ValtanArena::Get_ArenaActiveState() const"
        )

        self.assertIn("Is_WorldDestructionSynchronized", active)
        self.assertIn("Get_WorldDestructionGroupStates", active)
        self.assertIn("Get_WorldDestructionDiagnostics", active)
        for forbidden in ("Set_Visible", "Set_Position", "Set_WorldMatrix", "Send_"):
            self.assertNotIn(forbidden, active)

    def test_preset_route_delegates_to_the_level_transaction_owner_only(self) -> None:
        boss = read("Client/Private/BossTool.cpp")
        level = read("Client/Private/Level_ValtanArena.cpp")
        boss_submit = function_body(
            boss, "bool_t Client::CBossTool::Set_ServerArenaPreset("
        )
        level_submit = function_body(
            level, "bool_t CLevel_ValtanArena::Set_ArenaPreset("
        )

        self.assertIn("arena->Set_ArenaPreset", boss_submit)
        self.assertIn("Submit_Audition", level_submit)
        self.assertIn("SET_ARENA_PRESET", level_submit)
        for forbidden in (
            "Set_Visible",
            "Set_Position",
            "Set_WorldMatrix",
            "Send_ValtanAudition",
        ):
            self.assertNotIn(forbidden, boss_submit + level_submit)

    def test_hidden_legacy_panel_is_not_a_second_visible_command_path(self) -> None:
        level = read("Client/Private/Level_ValtanArena.cpp")
        header = read("Client/Public/Level_ValtanArena.h")
        update = function_body(level, "void CLevel_ValtanArena::Update(")
        main = read("Client/Private/MainApp.cpp")
        developer_tools = function_body(main, "void CMainApp::RenderDeveloperTools()")

        for retired in (
            "Render_AuditionPanel",
            "Load_AuditionTimeline",
            "Start_EnvironmentTimeline",
            "Start_AuthoredRotationPlayback",
            "Advance_EnvironmentTimeline",
            "AUDITION_TIMELINE_ROW",
            "ENVIRONMENT_TIMELINE_STEP",
        ):
            self.assertNotIn(retired, level + header)
        self.assertIn("Update_AuditionTransaction();", update)
        self.assertEqual(1, developer_tools.count("RenderCompletePlayControls();"))
        self.assertEqual(1, developer_tools.count("RenderServerArenaActiveControls();"))

    def test_server_rejects_revision_unaware_auditions_before_mutation(self) -> None:
        server = read("Server/Private/GameRoom.cpp")
        evaluate = function_body(
            server, "LostArk::Server::CGameRoom::Evaluate_ValtanAudition("
        )

        guard = evaluate.index("if (!isPatternIdCommand && !isArenaPreset)")
        receipt = evaluate.index("bool shouldStorePatternIdReceipt")
        mutation = evaluate.index("const auto evaluate = [&]()")
        self.assertLess(guard, receipt)
        self.assertLess(guard, mutation)
        self.assertIn("REJECTED_PATTERN_UNAVAILABLE", evaluate[guard:receipt])
        self.assertIn("SET_ARENA_PRESET", evaluate[:guard])

    def test_f1_root_window_has_no_hard_minimum_or_maximum(self) -> None:
        main = read("Client/Private/MainApp.cpp")
        developer_tools = function_body(main, "void CMainApp::RenderDeveloperTools()")

        self.assertIn("ImGui::SetNextWindowSize(vDefaultSize", developer_tools)
        self.assertNotIn("ImGui::SetNextWindowSizeConstraints", developer_tools)
        self.assertIn("LostArkDeveloperToolsResizableV1", developer_tools)
        self.assertNotIn("ImGuiWindowFlags_AlwaysAutoResize", developer_tools)
        self.assertIn('"##DeveloperToolLaunchGrid"', developer_tools)
        self.assertIn("iToolButtonColumns", developer_tools)
        self.assertIn("ImGui::TableNextColumn()", developer_tools)
        self.assertNotIn(
            'toolButton("Boss Tool", DEBUG_TOOL::BOSS, true);\n\tImGui::SameLine()',
            developer_tools,
        )

    def test_pattern_replay_never_resets_environment_owners(self) -> None:
        server = read("Server/Private/GameRoom.cpp")
        boss_only = function_body(
            server,
            "bool LostArk::Server::CGameRoom::Reset_ValtanBossOnlyAuditionState(",
        )
        evaluate = function_body(
            server, "LostArk::Server::CGameRoom::Evaluate_ValtanAudition("
        )
        pattern_start = evaluate.index("if (isPatternIdCommand)")
        pattern_end = evaluate.index(
            "SET_ARENA_PRESET is the only consumer", pattern_start
        )
        pattern_branch = evaluate[pattern_start:pattern_end]

        self.assertIn("Build_ValtanBossOnlyAuditionReset", pattern_branch)
        self.assertIn("Reset_ValtanBossOnlyAuditionState", pattern_branch)
        self.assertNotIn("Reset_ValtanAuditionState(", pattern_branch)
        self.assertIn("Build_ValtanBossOnlyAuditionReset", boss_only)
        self.assertIn("boss = std::move(stagedBoss)", boss_only)
        for forbidden in (
            "m_WorldDestructionRuntime",
            "m_EncounterPropRuntime",
            "Reset_WorldCollision",
            "Reset_Navigation",
            "Commit_WorldDestruction",
        ):
            self.assertNotIn(forbidden, boss_only)

    def test_explicit_pattern_restart_is_exact_and_arena_preserving(self) -> None:
        boss = read("Client/Private/BossTool.cpp")
        header = read("Client/Public/BossTool.h")
        service = read("Client/Private/ValtanPatternAuditionService.cpp")
        server = read("Server/Private/GameRoom.cpp")
        action_bar = function_body(boss, "void Client::CBossTool::Render_ActionBar()")
        restart = function_body(
            service,
            "bool Client::CValtanPatternAuditionService::Restart_ActivePattern(",
        )
        evaluate = function_body(
            server, "LostArk::Server::CGameRoom::Evaluate_ValtanAudition("
        )

        self.assertNotIn("Restart Pattern (Preserve Arena)", action_bar)
        self.assertNotIn("Restart_SelectedPattern()", action_bar)
        self.assertIn("Restart_SelectedPattern", header)
        self.assertIn("Restart_ServerPattern", header)
        restart_submit = function_body(
            boss, "bool_t Client::CBossTool::Restart_SelectedPattern()"
        )
        self.assertIn("Restart_ActivePattern", restart_submit)
        for marker in (
            "VALTAN_PATTERN_AUDITION_STATE::ACTIVE",
            "VALTAN_PATTERN_AUDITION_STATE::COMPLETED",
            "m_hasAuthoritativeLifecycle",
            "m_NextSnapshot.Is_Live()",
            "Has_PendingNextCommand()",
            "Is_FlowInFlight()",
            "Is_FlowStartPending()",
            "m_RestartFallback = m_Snapshot",
        ):
            self.assertIn(marker, restart)
        for marker in (
            "VALTAN_AUDITION_OPERATION::RESTART_PATTERN_ID",
            "iPredecessorRoomAuditionEpoch",
            "iPredecessorPatternSequence",
            "ExpectedDefinitionRevision",
            "REJECTED_OCCURRENCE_PRESERVED",
        ):
            self.assertIn(marker, evaluate)
        self.assertIn("Reset_ValtanBossOnlyAuditionState", evaluate)
        pattern_start = evaluate.index("if (isPatternIdCommand)")
        pattern_end = evaluate.index(
            "SET_ARENA_PRESET is the only consumer", pattern_start
        )
        self.assertNotIn("Reset_ValtanAuditionState(", evaluate[pattern_start:pattern_end])

    def test_boss_verification_default_restart_uses_one_saved_pattern_and_fresh_arena(self) -> None:
        boss = read("Client/Private/BossTool.cpp")
        action_bar = function_body(boss, "void Client::CBossTool::Render_ActionBar()")
        restart_single = function_body(
            boss,
            "bool_t Client::CBossTool::Restart_SavedSinglePatternFreshArena()",
        )
        restart_flow = function_body(
            boss,
            "bool_t Client::CBossTool::Restart_SavedFlow(\n"
            "\tconst bool_t bRequireSingleSavedPattern)",
        )
        server = read("Server/Private/GameRoom.cpp")
        flow_start = function_body(
            server, "LostArk::Server::CGameRoom::Evaluate_ValtanPatternFlowStart("
        )

        self.assertIn("Restart Saved Pattern (Fresh Arena)", action_bar)
        self.assertIn("Restart_SavedSinglePatternFreshArena", action_bar)
        self.assertIn("Get_SavedDefaultFlow", restart_single)
        self.assertIn("1u != iSavedSlotCount", restart_single)
        self.assertIn("exactly one saved scriptedSequence slot", restart_single)
        self.assertIn("Restart_SavedFlow(true)", restart_single)
        for marker in (
            "bRequireSingleSavedPattern",
            "Pending.Request.Slots.size() != 1u",
            "exact saved one-slot scriptedSequence request",
            "Reload_FlowDocument()",
            "Start_Flow(",
        ):
            self.assertIn(marker, restart_flow)
        self.assertIn("Reset_ValtanAuditionState(*boss", flow_start)
        self.assertIn("m_WorldDestructionRuntime", flow_start)
        self.assertIn("m_EncounterPropRuntime", flow_start)

    def test_wire_has_stable_pattern_and_explicit_arena_operations(self) -> None:
        packet = read("Shared/Public/Network/PacketMessages.h")
        self.assertIn("PLAY_PATTERN_ID", packet)
        self.assertIn("SET_ARENA_PRESET", packet)
        for preset in (
            "FRESH",
            "CIRCLE_WALLS_GONE",
            "THREE_OCLOCK_BROKEN",
            "NINE_OCLOCK_BROKEN",
            "BOTH_SIDES_BROKEN",
        ):
            self.assertIn(preset, packet)


if __name__ == "__main__":
    unittest.main()
