#!/usr/bin/env python3
"""Static admission for Balance authoring and the shared Valtan replay seam."""

from __future__ import annotations

import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
BALANCE_CPP = ROOT / "Client/Private/BalanceTool.cpp"
BALANCE_H = ROOT / "Client/Public/BalanceTool.h"
AUDITION_CPP = ROOT / "Client/Private/ValtanPatternAuditionService.cpp"
AUDITION_H = ROOT / "Client/Public/ValtanPatternAuditionService.h"
EFFECT_CPP = ROOT / "Client/Private/Effect_Tool.cpp"
BOSS_CPP = ROOT / "Client/Private/BossTool.cpp"
PROJECT = ROOT / "Client/Default/Client.vcxproj"
FILTERS = ROOT / "Client/Default/Client.vcxproj.filters"
NETWORK_CPP = ROOT / "Client/Private/NetworkManager.cpp"
NETWORK_H = ROOT / "Client/Public/NetworkManager.h"
MAIN_APP_CPP = ROOT / "Client/Private/MainApp.cpp"
SERVER_GAME_ROOM_CPP = ROOT / "Server/Private/GameRoom.cpp"
SERVER_PROJECT = ROOT / "Server/Default/Server.vcxproj"
GAMEPLAY_PUBLISHER = ROOT / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1"
VALTAN_PROJECTOR = ROOT / "Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1"


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


class ValtanBalanceToolContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.balance_cpp = BALANCE_CPP.read_text(encoding="utf-8")
        cls.balance_h = BALANCE_H.read_text(encoding="utf-8")
        cls.audition_cpp = AUDITION_CPP.read_text(encoding="utf-8")
        cls.audition_h = AUDITION_H.read_text(encoding="utf-8")
        cls.effect_cpp = EFFECT_CPP.read_text(encoding="utf-8")
        cls.boss_cpp = BOSS_CPP.read_text(encoding="utf-8")
        cls.project = PROJECT.read_text(encoding="utf-8")
        cls.filters = FILTERS.read_text(encoding="utf-8")
        cls.network_cpp = NETWORK_CPP.read_text(encoding="utf-8")
        cls.network_h = NETWORK_H.read_text(encoding="utf-8")
        cls.main_app_cpp = MAIN_APP_CPP.read_text(encoding="utf-8")
        cls.server_game_room_cpp = SERVER_GAME_ROOM_CPP.read_text(encoding="utf-8")
        cls.server_project = SERVER_PROJECT.read_text(encoding="utf-8")
        cls.gameplay_publisher = GAMEPLAY_PUBLISHER.read_text(encoding="utf-8")
        cls.valtan_projector = VALTAN_PROJECTOR.read_text(encoding="utf-8")

    def test_balance_tool_selection_reopens_and_focuses_existing_window(self) -> None:
        open_body = function_body(
            self.balance_cpp,
            "void Client::CBalanceTool::Open()",
        )
        self.assertIn("m_open = true", open_body)
        self.assertIn("m_focusPending = true", open_body)
        render_body = function_body(
            self.balance_cpp,
            "void Client::CBalanceTool::Render()",
        )
        collapsed = render_body.index("ImGui::SetNextWindowCollapsed(false")
        focused = render_body.index("ImGui::SetNextWindowFocus()")
        begin = render_body.index('ImGui::Begin("LostArk Balance Tool"')
        self.assertLess(collapsed, begin)
        self.assertLess(focused, begin)
        ensure_body = function_body(
            self.main_app_cpp,
            "HRESULT CMainApp::EnsureDebugTool(const DEBUG_TOOL eTool)",
        )
        self.assertIn("m_pBalanceTool->Open();", ensure_body)

    def test_valtan_revive_restores_immediate_boss_target_admission(self) -> None:
        self.assertNotIn("Request_RevivePlayer", self.balance_cpp)
        self.assertIn('ImGui::Button("Revive Player")', self.boss_cpp)
        self.assertIn("Request_RevivePlayer", self.boss_cpp)
        revive_body = function_body(
            self.server_game_room_cpp,
            "void LostArk::Server::CGameRoom::Handle_RevivePlayer(",
        )
        self.assertIn("player.isCombatReady = true", revive_body)
        self.assertLess(
            revive_body.index("player.isCombatReady = true"),
            revive_body.index("m_ServerTriggerSystem.Remove_Player"),
        )

    def test_generated_encounter_v4_is_read_only(self) -> None:
        self.assertIn('encounterFormatVersion != 4u', self.balance_cpp)
        self.assertIn('Generated Encounter v4 is read-only', self.balance_cpp)
        save = self.balance_cpp.index("bool Client::CBalanceTool::Save(")
        block = self.balance_cpp.index("Save blocked:", save)
        writes = self.balance_cpp.index("const std::vector<WRITE> writes", save)
        self.assertLess(block, writes)
        self.assertIn("if (nullptr == readOnlyCapture)", self.balance_cpp[save:block])

    def test_authoritative_master_and_legacy_product_are_separated(self) -> None:
        self.assertIn("CValtanPatternTree::Load(stagedTree", self.balance_cpp)
        self.assertIn("managedPatternIds.contains(row.patternId)", self.balance_cpp)
        self.assertIn("Legacy Product patterns (read-only until Promote)", self.balance_cpp)
        self.assertIn("VALTAN_PATTERN_TREE_VIEW m_valtanPatternTree", self.balance_h)
        self.assertIn("std::vector<LEGACY_PATTERN_SUMMARY>", self.balance_h)

        reload_authoring = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::ReloadValtanPatternAuthoring(",
        )
        self.assertIn("if (pattern.bAuthoringMasterManaged)", reload_authoring)
        self.assertIn("CountManagedValtanPatterns(patternTree)", reload_authoring)

        restore_authoring = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::RestoreValtanSavedAuthoring(",
        )
        self.assertIn("!targetPattern->bAuthoringMasterManaged", restore_authoring)
        self.assertIn("CountManagedValtanPatterns(patternTree)", restore_authoring)

        render_authoring = function_body(
            self.balance_cpp,
            "void Client::CBalanceTool::RenderValtanPatternAuthoring()",
        )
        self.assertIn("Managed manual Server auditions", render_authoring)
        self.assertIn("!pattern.bAuthoringMasterManaged", render_authoring)
        self.assertIn('pattern, "MANUAL AUDITION", index', render_authoring)

        live_verification = function_body(
            self.balance_cpp,
            "void Client::CBalanceTool::RenderLiveVerification()",
        )
        self.assertIn(
            "nullptr != selected && selected->bAuthoringMasterManaged",
            live_verification,
        )
        self.assertIn("Selected legacy pattern", live_verification)

        draft = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::BuildValtanDraftPatch(",
        )
        self.assertIn("if (!pattern.bAuthoringMasterManaged)", draft)

    def test_manual_audition_actual_valtan_path_locks_selection_and_wall_clock(self) -> None:
        managed_render = function_body(
            self.balance_cpp,
            "void Client::CBalanceTool::RenderValtanManagedPattern(",
        )
        for marker in (
            "const bool_t bManualAudition = pattern.bManualServerAudition",
            "Manual Server audition | phase %u | source chain %s",
            "automatic selection disabled",
            "AUDITION_ONLY keeps selection, repeat, and target range read-only",
            "locked to the promoted animation occurrence wall-clock",
            "Server replay is available in Boss Tool and Effect Tool; Repeat and Revive remain in Boss Tool",
            'EditFloat("Hit outer radius m"',
            "Pattern Presentation references (read-only)",
        ):
            self.assertIn(marker, managed_render)

        draft = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::BuildValtanDraftPatch(",
        )
        for marker in (
            "Manual Server audition selection/repeat/range is locked",
            "Manual Server audition stage duration is locked to the animation wall-clock",
            "SET_STAGE_HIT",
        ):
            self.assertIn(marker, draft)

    def test_gameplay_and_presentation_source_roles_are_explicit_and_fail_closed(self) -> None:
        for marker in (
            "Data/Valtan/Valtan.gameplay.json",
            "Data/Valtan/Valtan.presentation.json",
            "Server Gameplay canonical (editable)",
            "Pattern Presentation references (read-only)",
            "patternId + stageId + actionId",
            "SPLIT_SOURCE_INCOMPLETE",
            "SPLIT_SOURCE_UNVERIFIED",
            "JOINED_VALIDATED",
            "splitJoinValidated",
            "No joined revision is claimed",
        ):
            self.assertIn(marker, self.balance_cpp + self.balance_h)
        self.assertNotIn(
            "Valtan.pattern.json is the authoring master",
            self.balance_cpp,
        )
        self.assertIn(
            "VALTAN_SOURCE_JOIN_STATUS m_valtanSourceJoin",
            self.balance_h,
        )
        self.assertIn(
            "Command blocked: Server Gameplay and Pattern Presentation split",
            self.balance_cpp,
        )
        self.assertIn("The draft remains ", self.balance_cpp)
        self.assertIn('"unadmitted."', self.balance_cpp)

    def test_pattern_detail_lanes_and_phase_contract_are_visible(self) -> None:
        for marker in (
            "Server Gameplay canonical stage timeline (editable)",
            "Pattern Presentation references (read-only)",
            "Animation %s",
            "Effect %s",
            "Server combat-object spawn %s",
            "Combat-object presentation %s",
            "Camera cues:",
            "World event projection (read-only before G09)",
            "109 IMPACT/ENTER event",
        ):
            self.assertIn(marker, self.balance_cpp)
        self.assertIn('phasePolicyKind == "AUTHORED_PATTERN_EVENT"', self.balance_cpp)
        self.assertNotIn("boss.phaseTwoHpPercent", self.balance_cpp)

    def test_single_audition_service_owns_queue_for_two_ui_submitters(self) -> None:
        self.assertIn("class CValtanPatternAuditionService", self.audition_h)
        self.assertIn("Try_Consume_ValtanPatternAuditionByIdResult", self.audition_cpp)
        self.assertIn('"Boss Tool"', self.boss_cpp)
        self.assertIn("CValtanPatternAuditionService::Get().Submit", self.boss_cpp)
        self.assertIn('"Effect Tool"', self.effect_cpp)
        self.assertIn("CValtanPatternAuditionService::Get().Submit", self.effect_cpp)
        self.assertNotIn("CValtanPatternAuditionService", self.balance_cpp)
        self.assertNotIn("Request_RevivePlayer", self.balance_cpp)
        self.assertNotIn("Request_RevivePlayer", self.effect_cpp)
        self.assertNotIn("Try_Consume_ValtanPatternAuditionByIdResult", self.effect_cpp)
        self.assertNotIn("m_PendingValtanServerPatternRequest", self.effect_cpp)
        for state in ("REQUEST_PENDING", "QUEUED", "ACTIVE", "COMPLETED", "ABORTED"):
            self.assertIn(state, self.audition_h)

    def test_main_app_owns_per_frame_audition_queue_update(self) -> None:
        network_update = self.main_app_cpp.index(
            "CNetworkManager::Get().Update();"
        )
        service_update = self.main_app_cpp.index(
            "CValtanPatternAuditionService::Get().Update();",
            network_update,
        )
        engine_update = self.main_app_cpp.index(
            "CGameInstance::Get().Update_Engine", service_update
        )
        self.assertLess(network_update, service_update)
        self.assertLess(service_update, engine_update)
        managed_render = function_body(
            self.balance_cpp,
            "void Client::CBalanceTool::RenderValtanManagedPattern(",
        )
        pattern_render = function_body(
            self.balance_cpp,
            "void Client::CBalanceTool::RenderValtanPatternAuthoring()",
        )
        self.assertNotIn("auditionService.Update()", managed_render)
        self.assertNotIn("service.Update()", pattern_render)
        self.assertNotIn(
            "CValtanPatternAuditionService::Get().Update();",
            self.effect_cpp,
        )

    def test_play_server_pattern_uses_actual_stable_id_server_path(self) -> None:
        self.assertIn(
            "VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID",
            self.network_cpp,
        )
        for marker in (
            "VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID == request.eOperation",
            "boss->PendingPatternIds.push_back(request.strPatternId)",
            "m_GameplayCatalog.Get_ActiveRevision()",
        ):
            self.assertIn(marker, self.server_game_room_cpp)

    def test_unsaved_reload_requires_explicit_discard(self) -> None:
        self.assertIn("Discard unsaved Balance draft?", self.balance_cpp)
        self.assertIn("m_reloadConfirmationOpen", self.balance_h)

    def test_new_service_is_registered(self) -> None:
        for text in (self.project, self.filters):
            self.assertIn("ValtanPatternAuditionService.h", text)
            self.assertIn("ValtanPatternAuditionService.cpp", text)

    def test_valtan_draft_uses_revision_guarded_stable_id_operations(self) -> None:
        self.assertIn("lostark.valtan-tuning-draft-patch", self.balance_cpp)
        self.assertIn("sourceRevision", self.balance_cpp)
        for operation in (
            "SET_BOSS_BASE_FIELD",
            "SET_DAMAGE_RATE",
            "SET_PATTERN_WEIGHT",
            "SET_PATTERN_REPEAT_LIMIT",
            "SET_PATTERN_RANGE",
            "SET_STAGE_DURATION",
            "SET_STAGE_HIT",
            "SET_AXE_VOLLEY",
        ):
            self.assertIn(operation, self.balance_cpp)
        self.assertIn("BuildValtanDraftPatch", self.balance_h)
        self.assertIn("m_valtanSourceRevision", self.balance_h)

    def test_authoring_candidate_and_typed_hot_reload_are_distinct(self) -> None:
        for command in ("ValidateDraft", "SaveAuthoring", "PublishCandidate"):
            self.assertIn(command, self.balance_cpp)
        self.assertIn('ImGui::Button("Apply Hot Reload")', self.balance_cpp)

    def test_candidate_apply_class_is_strict_and_blocks_hot_reload(self) -> None:
        self.assertIn("std::string applyClass;", self.balance_cpp)
        self.assertIn("bool hasApplyClassField = false;", self.balance_cpp)
        self.assertIn('payload->Find("applyClass")', self.balance_cpp)
        self.assertIn(
            'output.command != "PUBLISH_CANDIDATE"', self.balance_cpp
        )
        self.assertIn(
            'm_valtanCandidateApplyClass != "HOT_RELOAD"', self.balance_cpp
        )
        self.assertIn(
            'm_valtanCandidateApplyClass == "HOT_RELOAD"', self.balance_cpp
        )
        self.assertIn("m_valtanCandidateApplyClass.clear();", self.balance_cpp)
        self.assertIn("Apply class: %s", self.balance_cpp)
        self.assertIn("m_valtanCandidateApplyClass", self.balance_h)
        self.assertIn("RequestValtanHotReload", self.balance_cpp)
        self.assertIn("Send_DataRevisionPrepareRequest", self.balance_cpp)
        self.assertIn("GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK", self.balance_cpp)
        self.assertIn("const bool canApply", self.balance_cpp)
        self.assertIn("!m_dirty", self.balance_cpp)
        self.assertIn("ServerActiveRevision.Is_Valid()", self.balance_cpp)
        self.assertIn("Runtime active pointer is unchanged", self.balance_cpp)
        self.assertIn("m_valtanAuthoringRevision", self.balance_h)
        self.assertIn("m_valtanCandidateRevision", self.balance_h)

    def test_client_alias_ready_requires_byte_identical_all_lane_artifacts(self) -> None:
        self.assertIn("ValidateByteIdenticalCandidatePresentation", self.network_cpp)
        self.assertIn('"BYTE_IDENTICAL_TO_ACTIVE"', self.network_cpp)
        for lane in (
            "ANIMATION",
            "EFFECT",
            "COMBAT_VISUAL",
            "CAMERA",
            "WORLD_EVENT_SET",
        ):
            self.assertIn(f'"{lane}"', self.network_cpp)
        for field in (
            'artifact.Find("sha256")',
            'artifact.Find("repositorySourceSha256")',
            'ReadExactUnsigned(artifact, "bytes"',
        ):
            self.assertIn(field, self.network_cpp)
        self.assertIn("candidateSha != shaValue->Get_String()", self.network_cpp)
        self.assertIn(
            "baseline->second->strSha256 != sourceShaValue->Get_String()",
            self.network_cpp,
        )
        self.assertIn("response.strReason = staged ? std::string{}", self.network_cpp)
        self.assertIn("MAX_PRESENTATION_ALIAS_GENERATIONS", self.network_h)

    def test_server_decision_trace_is_typed_bounded_and_visible(self) -> None:
        for marker in (
            "Send_ValtanDecisionTraceQuery",
            "S2C_VALTAN_DECISION_TRACE_RESPONSE",
            "m_ValtanDecisionTraceState",
            "iSubmittedAfterTraceSequence",
        ):
            self.assertIn(marker, self.network_cpp + self.network_h)
        for marker in (
            'SeparatorText("Server decision trace")',
            "DescribeValtanDecisionExclusions",
            "iAuthoredWeight",
            "iEffectiveWeight",
            "iCooldownRemainingTicks",
            "iRandomTicket",
            "LatestDefinitionRevision",
        ):
            self.assertIn(marker, self.balance_cpp)

    def test_world_entry_aliases_are_revalidated_and_abort_is_identity_scoped(self) -> None:
        self.assertIn("AdmitDebugPresentationRevisionAtWorldEntry", self.network_cpp)
        self.assertIn('L"Gameplay.bootstrap"', self.network_cpp)
        self.assertNotIn("retainedAliases", self.network_cpp)
        entry_start = self.network_cpp.index("case PACKET_TYPE::S2C_ENTER_ACCEPTED:")
        entry_end = self.network_cpp.index(
            "case PACKET_TYPE::S2C_ENTER_REJECTED:", entry_start
        )
        entry = self.network_cpp[entry_start:entry_end]
        self.assertIn("stagedPresentationAliases", entry)
        self.assertIn("if (!admitEntryRevision", entry)
        self.assertLess(
            entry.index("if (!admitEntryRevision"),
            entry.index("Reset_WorldInboundState();"),
        )
        self.assertLess(
            entry.index("Fail_Protocol(WSAEINVAL);"),
            entry.index("m_hasPendingEnterAccepted = true;"),
        )
        self.assertIn("hasOutstandingPrepareRequest", self.network_h)
        self.assertIn("const bool matchesStaged", self.network_cpp)
        self.assertIn("const bool matchesOutstanding", self.network_cpp)
        self.assertIn("if (matchesStaged)", self.network_cpp)
        self.assertIn(
            "result.ActiveRevision !=\n\t\t\t\tm_GameplayRevisionState.ServerActiveRevision",
            self.network_cpp,
        )
        self.assertIn("Prune_PresentationAliases", self.network_cpp)
        self.assertIn("ManifestMatchesRevisionIdentity", self.network_cpp)
        self.assertIn("HashBytesSha256(identityText", self.network_cpp)
        snapshot_start = self.network_cpp.index(
            "case PACKET_TYPE::S2C_WORLD_SNAPSHOT:"
        )
        snapshot_end = self.network_cpp.index(
            "case PACKET_TYPE::S2C_WORLD_DESTRUCTION_FULL_SYNC:", snapshot_start
        )
        snapshot = self.network_cpp[snapshot_start:snapshot_end]
        self.assertLess(
            snapshot.index(
                "snapshot.ActiveGameplayRevision !=\n\t\t\t\tm_GameplayRevisionState.ServerActiveRevision"
            ),
            snapshot.index("Record_WorldRevisionSet("),
        )

    def test_revision_stage_survives_overlap_and_stale_commit(self) -> None:
        stage = function_body(
            self.network_cpp,
            "bool CNetworkManager::Stage_ByteIdenticalPresentationAlias(",
        )
        self.assertNotIn("Discard_StagedPresentationAlias", stage)
        self.assertIn("const bool isExactRetransmit", stage)
        self.assertIn("Exact revision prepare retransmit is already staged", stage)
        self.assertIn("Overlapping or stale revision prepare was rejected", stage)
        self.assertLess(
            stage.index("if (m_GameplayRevisionState.hasStagedPresentationAlias)"),
            stage.index("ValidateByteIdenticalCandidatePresentation"),
        )

        commit = function_body(
            self.network_cpp,
            "bool CNetworkManager::Commit_StagedPresentationAlias(",
        )
        stale_commit = commit[
            commit.index("if (!isAlreadyActiveIdempotentCommit &&") :
            commit.index("if (!isAlreadyActiveIdempotentCommit &&\n\t\t!Is_PresentationRevisionAvailable")
        ]
        self.assertIn("return false;", stale_commit)
        self.assertNotIn("Discard_StagedPresentationAlias", stale_commit)

        self.assertIn("const bool matchesRejectedPrepare", commit)
        self.assertIn("hasRejectedPrepareAwaitingAbort", commit)
        self.assertIn("RejectedPrepareBaseRevision ==", commit)
        self.assertIn("RejectedPrepareCandidateRevision ==", commit)
        self.assertIn(
            "!matchesStaged && !matchesOutstanding && !matchesRejectedPrepare",
            commit,
        )
        self.assertIn(
            "m_GameplayRevisionState.hasRejectedPrepareAwaitingAbort = false;",
            commit,
        )

        handler_start = self.network_cpp.index(
            "case PACKET_TYPE::S2C_DATA_REVISION_RESULT:"
        )
        handler_end = self.network_cpp.index(
            "case PACKET_TYPE::S2C_CHARACTER_CLASS_CHANGE_RESULT:", handler_start
        )
        handler = self.network_cpp[handler_start:handler_end]
        failure = handler.index("if (!presentationCommitted)")
        fail_close = handler.index("Fail_Protocol(WSAEINVAL);", failure)
        active_write = handler.index("if (DATA_REVISION_RESULT::COMMITTED")
        self.assertLess(failure, fail_close)
        self.assertLess(fail_close, active_write)

    def test_pipeline_result_is_parsed_as_a_structured_contract(self) -> None:
        self.assertIn('"lostark.valtan-tuning-command-result"', self.balance_cpp)
        for field in (
            'ReadString(root, "command", output.command)',
            'root.Find("sourceRevision")',
            'root.Find("candidateRevision")',
            'Field(root, "errors", DATA_JSON_TYPE::ARRAY)',
            'Field(root, "payload", DATA_JSON_TYPE::OBJECT)',
        ):
            self.assertIn(field, self.balance_cpp)

    def test_saved_authoring_head_is_resumed_through_strict_pipeline_admission(self) -> None:
        for marker in (
            "QueryValtanSourceRevision",
            "RestoreValtanSavedAuthoring",
            "Intermediate/ValtanTuningAuthoring/current-authoring.json",
            "lostark.valtan-tuning-authoring-pointer",
            "Data/Valtan/Valtan.gameplay.json",
            "Data/Valtan/Valtan.presentation.json",
            "Load_FromAuthoringPaths",
            "VALTAN_PATTERN_TREE_LOAD_POLICY::RESTORE_AUTHORING_SNAPSHOT",
            "lostark.valtan-gameplay-authoring",
            "Data/Balance/BossProfiles.json",
            "Data/Balance/DamageProfiles.json",
            'L"-Mode ValidateDraft -DraftPatchPath',
            '\\"operations\\": []',
            'validationResult.command != "VALIDATE_DRAFT"',
            "ResolveFixedValtanAuthoringPath",
            "FILE_ATTRIBUTE_REPARSE_POINT",
        ):
            self.assertIn(marker, self.balance_cpp)
        self.assertNotIn("Data/Valtan/Valtan.pattern.v2.json", self.balance_cpp)
        self.assertIn("m_valtanAuthoringRevision = std::move(valtanAuthoringRevision)", self.balance_cpp)
        self.assertIn("m_loadedValtanPatternTree = m_valtanPatternTree", self.balance_cpp)

    def test_saved_authoring_overlay_commits_only_after_every_staged_check(self) -> None:
        reload_body = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Reload()",
        )
        query = reload_body.index("QueryValtanSourceRevision")
        restore = reload_body.index("RestoreValtanSavedAuthoring")
        commit = reload_body.index("m_players = std::move(players)")
        self.assertLess(query, restore)
        self.assertLess(restore, commit)
        restore_body = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::RestoreValtanSavedAuthoring(",
        )
        for marker in (
            "savedBossIds.insert",
            "savedDamageIds.insert",
            "patternTree = std::move(savedPatternTree)",
            "typed sets/windows/mechanics",
            "savedPatternIds.insert",
            "savedStageIds.insert",
            "FindValtanPattern(patternTree, patternId)",
            "FindValtanStage(*targetPattern, stageId)",
            "ReadTextFile(pointerPath) != admittedPointerBytes",
        ):
            self.assertIn(marker, restore_body)
        strict_join = restore_body.index("Load_FromAuthoringPaths(")
        stage_joined_tree = restore_body.index(
            "patternTree = std::move(savedPatternTree)", strict_join
        )
        gameplay_restore = restore_body.index(
            "ReadAbsoluteJsonObject(gameplayPath", stage_joined_tree
        )
        pointer_cas = restore_body.index(
            "ReadTextFile(pointerPath) != admittedPointerBytes", gameplay_restore
        )
        self.assertLess(strict_join, stage_joined_tree)
        self.assertLess(stage_joined_tree, gameplay_restore)
        self.assertLess(gameplay_restore, pointer_cas)
        self.assertIn("gameplayPath, presentationPath", restore_body)
        self.assertIn(
            "VALTAN_PATTERN_TREE_LOAD_POLICY::RESTORE_AUTHORING_SNAPSHOT",
            restore_body,
        )
        self.assertNotIn("ReadAbsoluteJsonObject(presentationPath", restore_body)
        self.assertNotIn("Valtan.pattern.v2.json", restore_body)

    def test_per_window_weight_enabled_and_mechanic_ops_are_typed(self) -> None:
        for marker in (
            "Managed selection windows (next Server decision)",
            "Post-109 legacy rotations (read-only)",
            "SET_PATTERN_WEIGHT",
            "SET_PATTERN_ENABLED",
            "SET_MECHANIC_TRIGGER",
            "selectionSet.strSelectionSetId",
            "mechanic.strMechanicId",
            "Same-bar trigger order",
            "Compatibility fallback is read-only",
            "Phase boundary health is read-only until windows and legacy rotation topology can change atomically.",
            '"VALTAN_ARENA_BREAK_109" == mechanic.strPatternId',
        ):
            self.assertIn(marker, self.balance_cpp)
        draft = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::BuildValtanDraftPatch(",
        )
        self.assertNotIn(
            "for (const VALTAN_NORMAL_SELECTION_RANGE_VIEW& range", draft
        )
        self.assertIn("candidate.iWeight != loadedCandidate->iWeight", draft)
        self.assertIn("candidate.bEnabled != loadedCandidate->bEnabled", draft)

    def test_all_valtan_v2_authoring_sources_are_project_data_items(self) -> None:
        for relative in (
            "Data\\Valtan\\Valtan.pattern.json",
            "Data\\Valtan\\Valtan.gameplay.json",
            "Data\\Valtan\\Valtan.presentation.json",
            "Data\\Valtan\\Valtan.combatobjects.json",
            "Data\\Valtan\\Valtan.worldeventsets.json",
            "Data\\Valtan\\Valtan.legacy-compatibility.json",
        ):
            self.assertIn(relative, self.project)
            self.assertIn(relative, self.filters)

    def test_split_products_drive_client_and_server_builds(self) -> None:
        self.assertIn('Name="ValidateValtanSplitProducts"', self.project)
        self.assertIn('-Mode ValidateV2', self.project)
        self.assertIn('Publish-GameplayBalance.ps1', self.server_project)
        self.assertIn("-Mode ValidateV2", self.gameplay_publisher)
        self.assertNotIn("if ($Mode -eq 'Publish') { 'PublishV2' }", self.gameplay_publisher)
        self.assertIn('project-products', self.valtan_projector)
        self.assertNotIn('ValtanChargeImpactActions.json', self.gameplay_publisher)
        self.assertIn("[string]$_.outcome -ceq 'WALL_CONTACT'", self.gameplay_publisher)
        self.assertIn("[string]$motion.kind -cne 'FORWARD'", self.gameplay_publisher)
        self.assertIn("stageKind -cne 'GROGGY'", self.gameplay_publisher)


if __name__ == "__main__":
    unittest.main(verbosity=2)
