#!/usr/bin/env python3
"""Static admission for Balance authoring and the shared Valtan replay seam."""

from __future__ import annotations

import pathlib
import json
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
BALANCE_CPP = ROOT / "Client/Private/BalanceTool.cpp"
BALANCE_H = ROOT / "Client/Public/BalanceTool.h"
AUDITION_CPP = ROOT / "Client/Private/ValtanPatternAuditionService.cpp"
AUDITION_H = ROOT / "Client/Public/ValtanPatternAuditionService.h"
TUNING_COMMAND_CPP = ROOT / "Client/Private/ValtanTuningCommandService.cpp"
EFFECT_CPP = ROOT / "Client/Private/Effect_Tool.cpp"
BOSS_CPP = ROOT / "Client/Private/BossTool.cpp"
PROJECT = ROOT / "Client/Default/Client.vcxproj"
FILTERS = ROOT / "Client/Default/Client.vcxproj.filters"
NETWORK_CPP = ROOT / "Client/Private/NetworkManager.cpp"
NETWORK_H = ROOT / "Client/Public/NetworkManager.h"
PRESENTATION_ADMISSION_CPP = (
    ROOT / "Client/Private/ValtanPresentationGenerationAdmission.cpp"
)
PRESENTATION_ADMISSION_NATIVE_TESTS = (
    ROOT
    / "Tools/ValtanPatternAuditionServiceHarness/Private/"
    "ValtanPresentationGenerationAdmissionContractTests.cpp"
)
MAIN_APP_CPP = ROOT / "Client/Private/MainApp.cpp"
SERVER_GAME_ROOM_CPP = ROOT / "Server/Private/GameRoom.cpp"
SERVER_PROJECT = ROOT / "Server/Default/Server.vcxproj"
GAMEPLAY_PUBLISHER = ROOT / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1"
VALTAN_PROJECTOR = ROOT / "Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1"
BUILD_DOMAINS = ROOT / "Tools/Build/BuildDomains.json"


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
        cls.tuning_command_cpp = TUNING_COMMAND_CPP.read_text(encoding="utf-8")
        cls.effect_cpp = EFFECT_CPP.read_text(encoding="utf-8")
        cls.boss_cpp = BOSS_CPP.read_text(encoding="utf-8")
        cls.project = PROJECT.read_text(encoding="utf-8")
        cls.filters = FILTERS.read_text(encoding="utf-8")
        cls.network_cpp = NETWORK_CPP.read_text(encoding="utf-8")
        cls.network_h = NETWORK_H.read_text(encoding="utf-8")
        cls.presentation_admission_cpp = PRESENTATION_ADMISSION_CPP.read_text(
            encoding="utf-8"
        )
        cls.presentation_admission_native_tests = (
            PRESENTATION_ADMISSION_NATIVE_TESTS.read_text(encoding="utf-8")
        )
        cls.main_app_cpp = MAIN_APP_CPP.read_text(encoding="utf-8")
        cls.server_game_room_cpp = SERVER_GAME_ROOM_CPP.read_text(encoding="utf-8")
        cls.server_project = SERVER_PROJECT.read_text(encoding="utf-8")
        cls.gameplay_publisher = GAMEPLAY_PUBLISHER.read_text(encoding="utf-8")
        cls.valtan_projector = VALTAN_PROJECTOR.read_text(encoding="utf-8")
        cls.build_domains = json.loads(BUILD_DOMAINS.read_text(encoding="utf-8"))

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
        self.assertIn(
            "CValtanPatternTree::Load_WhileAdmitted(", self.balance_cpp
        )
        self.assertIn("managedPatternIds.contains(row.patternId)", self.balance_cpp)
        self.assertIn("Legacy Product patterns (read-only until Promote)", self.balance_cpp)
        self.assertIn("VALTAN_PATTERN_TREE_VIEW m_valtanPatternTree", self.balance_h)
        self.assertIn("std::vector<LEGACY_PATTERN_SUMMARY>", self.balance_h)

        reload_authoring = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::ReloadValtanPatternAuthoring(",
        )
        self.assertIn("canonicalAdmission, stagedTree", reload_authoring)
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

    def test_manual_audition_locks_selection_but_exposes_typed_composition_clock(self) -> None:
        managed_render = function_body(
            self.balance_cpp,
            "void Client::CBalanceTool::RenderValtanManagedPattern(",
        )
        for marker in (
            "const bool_t bManualAudition = pattern.bManualServerAudition",
            "Manual Server audition | phase %u | source chain %s",
            "automatic selection disabled",
            "AUDITION_ONLY keeps selection, repeat, and target range read-only",
            "edit Stage kind, Sequence slots, and gap in Action Composition Workbench",
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
            "SET_STAGE_KIND",
            "SET_STAGE_DURATION",
            "SET_STAGE_ANIMATION",
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

    def test_single_audition_service_is_submitted_only_by_shared_boss_owner(self) -> None:
        self.assertIn("class CValtanPatternAuditionService", self.audition_h)
        self.assertIn("Try_Consume_ValtanPatternAuditionByIdResult", self.audition_cpp)
        self.assertIn('"Boss Tool"', self.boss_cpp)
        self.assertIn("CValtanPatternAuditionService::Get().Submit", self.boss_cpp)
        self.assertIn('"Effect Tool"', self.effect_cpp)
        self.assertNotIn("CValtanPatternAuditionService::Get().Submit", self.effect_cpp)
        self.assertIn("Debug_SelectCompletePlayPattern", self.effect_cpp)
        self.assertIn("Debug_CompletePlaySelected", self.effect_cpp)
        self.assertIn("m_pBossTool->Play_ServerPattern", self.main_app_cpp)
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

    def test_pattern_validation_and_round_trip_do_not_require_a_fixed_inventory_size(self) -> None:
        validate = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::ValidateDraft(std::string& status) const",
        )
        for forbidden in (
            "m_players.size() != 6u",
            "m_skills.size() != 94u",
            "m_damageProfiles.size() != 108u",
            "m_bosses.size() != 1u",
            "livePatternCount",
            "33u",
        ):
            self.assertNotIn(forbidden, validate)
        for required in (
            "m_players.empty()",
            "m_skills.empty()",
            "m_damageProfiles.empty()",
            "m_bosses.empty()",
            "m_patterns.empty()",
            "playerClasses.insert(player.characterClass)",
            "playerClasses.contains(skill.characterClass)",
            "damageProfileIds.insert(damage.damageProfileId)",
            "bossArchetypeIds.insert(boss.archetypeId)",
            "m_bosses.end() == encounterBoss",
        ):
            self.assertIn(required, validate)
        self.assertIn("patternIds.insert(pattern.patternId)", validate)
        round_trip = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Run_ReadOnlyRoundTripContractTest(",
        )
        for forbidden in ("26u != safetyTool.m_legacyPatterns.size()", "33u != livePatternCount"):
            self.assertNotIn(forbidden, round_trip)
        self.assertIn(
            "CValtanPatternTree::Build_PlayablePatternInventory(",
            round_trip,
        )
        self.assertIn("playableInventory.Get_PatternCount()", round_trip)
        self.assertNotIn(
            "NormalSelection.PatternIds.size() +",
            round_trip,
        )
        self.assertIn(
            "managedPatternCount + safetyTool.m_legacyPatterns.size()",
            round_trip,
        )
        self.assertIn(
            "patterns->Get_Array().size() != tool.m_patterns.size()",
            round_trip,
        )
        for dynamic_collection in (
            "players->Get_Array().size() != tool.m_players.size()",
            "skills->Get_Array().size() != tool.m_skills.size()",
            "damageProfiles->Get_Array().size() != tool.m_damageProfiles.size()",
            "bosses->Get_Array().size() != tool.m_bosses.size()",
        ):
            self.assertIn(dynamic_collection, round_trip)
        self.assertNotIn("players->Get_Array().size() != 6u", round_trip)
        self.assertNotIn("skills->Get_Array().size() != 94u", round_trip)

        save_command = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::RunValtanDraftCommand(",
        )
        self.assertNotIn("result.artifactCount < 9u", save_command)
        self.assertNotIn("!result.hasArtifactCountField", save_command)
        self.assertIn("result.hasOperationCountField", save_command)
        self.assertIn("result.hasChangedCountField", save_command)

    def test_authoring_candidate_and_typed_hot_reload_are_distinct(self) -> None:
        for command in ("ValidateDraft", "SaveAuthoring", "PublishCandidate"):
            self.assertIn(command, self.balance_cpp)
        save_product = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Save_ValtanProduct(std::string& status)",
        )
        for command in (
            "Validate_ValtanDraft",
            "Save_ValtanCanonicalProduct",
            "Publish_ValtanCandidate",
            "Apply_ValtanRevision",
        ):
            self.assertIn(command, save_product)
        self.assertNotIn("Save_ValtanAuthoring", save_product)
        self.assertNotIn('ImGui::Button("Apply Hot Reload")', self.balance_cpp)

    def test_canonical_commit_receipt_is_not_reclassified_as_nothing_saved(self) -> None:
        command = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::RunValtanDraftCommand(",
        )
        commit = command[command.index('if (0 == std::wcscmp(mode, L"CommitCanonicalDraft"))') :]
        reopen_failure = commit[
            commit.index("if (!Reload())") :
            commit.index("if (m_valtanSourceRevision != committedRevision")
        ]
        self.assertIn("COMMIT_SUCCEEDED_REOPEN_FAILED", reopen_failure)
        self.assertIn("return true;", reopen_failure)
        self.assertNotIn("return false;", reopen_failure)
        revision_mismatch = commit[
            commit.index("if (m_valtanSourceRevision != committedRevision") :
            commit.index('status = "COMMITTED_AND_RELOADED')
        ]
        self.assertIn("COMMIT_SUCCEEDED_REOPEN_FAILED", revision_mismatch)
        self.assertIn("return true;", revision_mismatch)
        self.assertIn("COMMITTED_AND_RELOADED", commit)

    def test_clean_saved_source_has_typed_publish_apply_retry_without_recommit(self) -> None:
        retry = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Retry_ValtanProductPublishApply(",
        )
        self.assertIn("m_valtanCommittedRevisionPendingReopen", retry)
        self.assertIn("m_valtanCommittedReopenDraftGeneration", retry)
        self.assertLess(
            retry.index("m_valtanDraftGeneration !="),
            retry.index("Reload()"),
        )
        self.assertIn("else if (m_dirty)", retry)
        self.assertIn("Save_ValtanProduct(RetryStatus)", retry)
        self.assertIn("without another canonical commit", retry)
        self.assertNotIn("Save_ValtanCanonicalProduct", retry)
        self.assertNotIn("RunValtanDraftCommand", retry)

        command = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::RunValtanDraftCommand(",
        )
        commit = command[
            command.index('if (0 == std::wcscmp(mode, L"CommitCanonicalDraft"))') :
        ]
        self.assertLess(
            commit.index("m_valtanCommittedRevisionPendingReopen = committedRevision"),
            commit.index("if (!Reload())"),
        )
        self.assertLess(
            commit.index("Record_GameplaySourceActivationExpectation"),
            commit.index("if (!Reload())"),
        )
        self.assertIn('{}, "NOT_ACTIVATED"', commit)
        reload_body = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Reload()",
        )
        self.assertIn("m_valtanCommittedRevisionPendingReopen.clear()", reload_body)
        self.assertIn("m_valtanCommittedReopenDraftGeneration = 0u", reload_body)

        save_product = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Save_ValtanProduct(std::string& status)",
        )
        self.assertIn(
            "m_dirty && !m_valtanCommittedRevisionPendingReopen.empty()",
            save_product,
        )
        self.assertIn("will not repeat an already committed", save_product)
        self.assertIn("COMMIT_SUCCEEDED_REOPEN_FAILED:", save_product)
        render = function_body(
            self.balance_cpp,
            "void Client::CBalanceTool::Render()",
        )
        self.assertIn(
            'ImGui::Button("Retry Product Publish / Apply##ValtanBalance")',
            render,
        )
        self.assertIn("canRetryProduct", render)

    def test_candidate_apply_class_is_strict_and_blocks_hot_reload(self) -> None:
        self.assertIn("std::string applyClass;", self.balance_cpp)
        self.assertIn("bool hasApplyClassField = false;", self.balance_cpp)
        self.assertIn('payload->Find("applyClass")', self.balance_cpp)
        self.assertIn(
            'output.command != "PUBLISH_CANDIDATE"', self.balance_cpp
        )
        request_body = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::RequestValtanHotReload(",
        )
        self.assertIn("CValtanTuningCommandService::Get().ApplyCandidate(", request_body)
        self.assertIn("m_valtanCandidateRevision, m_valtanCandidateApplyClass", request_body)
        apply_body = function_body(
            self.tuning_command_cpp,
            "bool Client::CValtanTuningCommandService::ApplyCandidate(",
        )
        self.assertIn('ApplyClass != "HOT_RELOAD"', apply_body)
        self.assertIn('m_Snapshot.strApplyClass != "HOT_RELOAD"', self.tuning_command_cpp)
        self.assertIn(
            'm_valtanCandidateApplyClass == "HOT_RELOAD"', self.balance_cpp
        )
        self.assertIn("m_valtanCandidateApplyClass.clear();", self.balance_cpp)
        self.assertIn("Runtime activation: %s", self.balance_cpp)
        self.assertIn("m_valtanCandidateApplyClass", self.balance_h)
        self.assertIn("RequestValtanHotReload", self.balance_cpp)
        self.assertNotIn("Send_DataRevisionPrepareRequest", request_body)
        self.assertIn("Send_DataRevisionPrepareRequest", self.tuning_command_cpp)
        self.assertIn("GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK", self.tuning_command_cpp)
        self.assertIn("Has_PendingCommand()", self.balance_cpp)
        self.assertIn("!m_dirty", self.balance_cpp)
        self.assertIn("ServerActiveRevision.Is_Valid()", self.balance_cpp)

    def test_saved_gameplay_source_blocks_replay_until_exact_server_candidate(self) -> None:
        save_product = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Save_ValtanProduct(std::string& status)",
        )
        replay_gate = function_body(
            self.boss_cpp,
            "bool_t Client::CBossTool::Get_ServerActivePatternRevision(",
        )
        activation_gate = function_body(
            self.tuning_command_cpp,
            "Is_LatestGameplaySourceServerActive(std::string& strOutStatus) const",
        )
        exact_activation_gate = function_body(
            self.tuning_command_cpp,
            "Try_GetLatestGameplaySourceServerActiveRevision(",
        )
        reload_balance = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Reload()",
        )
        construct_balance = function_body(
            self.balance_cpp,
            "Client::CBalanceTool::CBalanceTool()",
        )

        self.assertEqual(
            3,
            save_product.count("Record_GameplaySourceActivationExpectation"),
            "publish failure, candidate publication, and apply preflight failure must each refresh the exact activation expectation",
        )
        self.assertIn("hadDirtyDraft", save_product)
        self.assertIn("m_valtanCandidateRevision", save_product)
        self.assertIn("m_valtanCandidateApplyClass", save_product)
        self.assertNotIn("Is_LatestGameplaySourceServerActive", replay_gate)
        self.assertIn("ServerActiveRevision", replay_gate)
        self.assertIn("activeRevision.Is_Valid()", replay_gate)
        self.assertIn(
            "Try_GetLatestGameplaySourceServerActiveRevision", activation_gate
        )
        self.assertIn("Read_RevisionObservation", exact_activation_gate)
        self.assertIn(
            "Observation.ServerActiveRevision == CandidateRevision",
            exact_activation_gate,
        )
        self.assertIn("outRevision = CandidateRevision", exact_activation_gate)
        self.assertIn("no admitted Product candidate", exact_activation_gate)
        self.assertIn("Has_GameplaySourceActivationExpectation", reload_balance)
        self.assertIn("A saved Valtan authoring head was resumed", reload_balance)
        self.assertIn("if (!Reload()", construct_balance)
        self.assertIn("Record_GameplaySourceActivationExpectation", construct_balance)
        self.assertIn("Runtime active pointer is unchanged", self.balance_cpp)
        self.assertIn("m_valtanAuthoringRevision", self.balance_h)
        self.assertIn("m_valtanCandidateRevision", self.balance_h)

    def test_balance_reload_preserves_rows_but_revokes_mutation_until_commit(self) -> None:
        reload_balance = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Reload()",
        )
        admission_gate = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Require_ValtanAuthoringAdmission(",
        )
        self.assertIn('#include "ValtanViewAdmission.h"', self.balance_h)
        self.assertIn("m_eValtanViewAdmission", self.balance_h)
        revoke = (
            "m_eValtanViewAdmission = bHadDisplayableValtanView ?"
        )
        self.assertIn(revoke, reload_balance)
        self.assertLess(
            reload_balance.index(revoke),
            reload_balance.index("CanonicalAdmission.Acquire("),
        )
        self.assertIn(
            "m_eValtanViewAdmission =\n\t\tVALTAN_SOURCE_JOIN_STATE::JOINED_VALIDATED",
            reload_balance,
        )
        self.assertIn(
            "Can_MutateValtanView(m_eValtanViewAdmission)", admission_gate
        )
        self.assertIn("m_valtanSourceJoin.state", admission_gate)
        self.assertIn("m_valtanSourceRevision.empty()", admission_gate)

    def test_high_jump_axe_count_has_one_typed_normalization_boundary(self) -> None:
        getter = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Get_ValtanHighJumpAxeCountDraft(",
        )
        setter = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Set_ValtanHighJumpAxeCountDraft(",
        )
        for token in (
            '"VALTAN_HIGH_JUMP"',
            '"AIRBORNE"',
            '"event.valtan.high-jump.airborne.spawn-target-axe"',
            "m_valtanSourceRevision.empty()",
        ):
            self.assertIn(token, getter)
        for token in (
            "countPerAlivePlayer < 1u || countPerAlivePlayer > 8u",
            'candidate.layoutKind = "TARGET_CENTER"',
            'candidate.layoutKind = "RADIAL_AROUND_TARGET"',
            "360.0 /",
            "candidate.arenaRandomCount",
            "requiredCapacity > 64u",
            "MarkDirty(true)",
        ):
            self.assertIn(token, setter)
        render = function_body(
            self.balance_cpp,
            "void Client::CBalanceTool::RenderValtanManagedPattern(",
        )
        self.assertIn("Set_ValtanHighJumpAxeCountDraft", render)
        self.assertIn("SET_AXE_VOLLEY", self.balance_cpp)

    def test_client_current_generation_requires_exact_all_lane_artifacts(self) -> None:
        self.assertIn("ValidateCurrentCandidatePresentationGeneration", self.network_cpp)
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

    def test_live_apply_preflights_current_saved_generation_before_prepare(self) -> None:
        for token in (
            "PRESENTATION_CANDIDATE_PREFLIGHT_RESULT",
            "CURRENT_GENERATION_READY",
            "Preflight_PresentationCandidate",
        ):
            self.assertIn(token, self.network_h)
        validator = function_body(
            self.network_cpp,
            "bool ValidateCurrentCandidatePresentationGeneration(",
        )
        self.assertNotIn("requiresReentry", validator)
        self.assertIn("firstCurrentMismatch", validator)
        self.assertNotIn('"REENTRY_REQUIRED:', validator)
        self.assertIn("current saved typed", validator)
        self.assertLess(
            validator.index("candidateSha != shaValue->Get_String()"),
            validator.index("firstCurrentMismatch = relative"),
        )
        preflight = function_body(
            self.network_cpp,
            "CNetworkManager::Preflight_PresentationCandidate(",
        )
        self.assertIn("CapturePresentationArtifactBaseline(", preflight)
        self.assertIn("ValidateCurrentCandidatePresentationGeneration(", preflight)
        self.assertNotIn(
            "m_GameplayRevisionState.PresentationArtifactBaseline", preflight
        )

        submit = function_body(
            self.tuning_command_cpp,
            "bool Client::CValtanTuningCommandService::Submit_Candidate(",
        )
        self.assertIn("Preflight_PresentationCandidate(", submit)
        self.assertIn("return Reject(std::move(strOutStatus));", submit)
        self.assertLess(
            submit.index("Preflight_PresentationCandidate("),
            submit.index("Send_PrepareRequest(Request)"),
        )

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

    def test_world_entry_pins_server_revisions_without_physical_receipt_gate(self) -> None:
        self.assertNotIn("AdmitDebugPresentationRevisionAtWorldEntry", self.network_cpp)
        self.assertNotIn("retainedAliases", self.network_cpp)
        entry_start = self.network_cpp.index("case PACKET_TYPE::S2C_ENTER_ACCEPTED:")
        entry_end = self.network_cpp.index(
            "case PACKET_TYPE::S2C_ENTER_REJECTED:", entry_start
        )
        entry = self.network_cpp[entry_start:entry_end]
        self.assertIn("CapturePresentationArtifactBaseline(", entry)
        baseline_capture = function_body(
            entry, "if (!hasPresentationArtifactBaseline)"
        )
        self.assertNotIn("Fail_Protocol(", baseline_capture)
        self.assertNotIn("return;", baseline_capture)
        self.assertNotIn("CLIENT_ENTRY_PRESENTATION_BASELINE_FAILED", entry)
        self.assertIn('"presentation.baseline-unavailable"', baseline_capture)
        self.assertIn("stagedPresentationArtifactBaseline.clear()", baseline_capture)
        self.assertIn("stagedPresentationReceipt = {}", baseline_capture)
        decode_failure = function_body(entry, "if (!Read_Message(reader, accepted)")
        self.assertIn("Fail_Protocol(", decode_failure)
        self.assertIn("CLIENT_MESSAGE_DECODE_FAILED", decode_failure)
        self.assertIn("return;", decode_failure)
        self.assertIn("stagedPresentationAliases", entry)
        self.assertIn("const auto admitEntryRevision", entry)
        self.assertIn("if (!admitEntryRevision", entry)
        self.assertIn("if (!revision.Is_Valid())", entry)
        self.assertNotIn("ValidateCurrentCandidatePresentationGeneration", entry)
        self.assertNotIn("PresentationGenerationId", entry)
        self.assertNotIn("strSha256", entry)
        self.assertNotIn("iBytes", entry)
        self.assertNotIn("Gameplay.bootstrap", entry)
        self.assertNotIn(
            "Release Client received pinned presentation revisions", entry
        )
        self.assertNotIn(
            "Release world-entry revision does not match packaged", entry
        )
        self.assertIn(
            "stagedPresentationReceipt.ServerGameplayRevision =", entry
        )
        self.assertIn(
            "m_GameplayRevisionState.hasPresentationArtifactBaseline =\n"
            "\t\t\thasPresentationArtifactBaseline;",
            entry,
        )
        self.assertIn("m_GameplayRevisionState.isPresentationIsolated = true", entry)
        self.assertIn("Reload the presentation sources", entry)
        self.assertIn(
            "hasPendingEntryPresentationBaselineRecovery", entry
        )
        self.assertIn(
            "stagedPresentationDiagnostic.Is_AutomaticRetryable()", entry
        )
        self.assertNotIn("status.find(\"Win32 33\")", self.network_cpp)
        self.assertIn(
            "ENTRY_PRESENTATION_BASELINE_RETRY_MILLISECONDS", entry
        )
        self.assertLess(
            entry.index("if (!admitEntryRevision"),
            entry.index("Reset_WorldInboundState();"),
        )
        admission_failure = function_body(entry, "if (!admitEntryRevision(")
        self.assertIn("Fail_Protocol(", admission_failure)
        self.assertIn("WSAEINVAL", admission_failure)
        self.assertIn("CLIENT_ENTRY_PRESENTATION_REVISION_FAILED", admission_failure)
        self.assertIn("entryAdmissionFailure", admission_failure)
        self.assertIn("return;", admission_failure)
        self.assertLess(
            entry.index("CLIENT_ENTRY_PRESENTATION_REVISION_FAILED"),
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
        revision_failure = function_body(snapshot, "if (snapshot.ActiveGameplayRevision !=")
        self.assertIn("Fail_Protocol(", revision_failure)
        self.assertIn("CLIENT_INVALID_SERVER_RESPONSE", revision_failure)
        self.assertIn("return;", revision_failure)
        self.assertLess(
            snapshot.index("if (snapshot.ActiveGameplayRevision !="),
            snapshot.index("Record_WorldRevisionSet("),
        )

    def test_presentation_reopen_uses_current_typed_closure_and_transaction_guard(self) -> None:
        acquire = function_body(
            self.presentation_admission_cpp,
            "bool Client::CValtanPresentationGenerationReadAdmission::Acquire_ReceiptFromRoot(",
        )
        self.assertIn("if (!ExpectedServerRevision.Is_Valid())", acquire)
        self.assertIn("(void)ExpectedReceipt", acquire)
        self.assertIn(
            "physical.ServerGameplayRevision = ExpectedServerRevision", acquire
        )
        self.assertIn("m_pState->Receipt = physical", acquire)
        for stale_gate in (
            "ExpectedReceipt.Is_Valid()",
            "ExpectedReceipt.ServerGameplayRevision",
            "ExpectedReceipt.PresentationGenerationId",
            "sameInventory",
            "current.strRelativePath == expected.strRelativePath",
        ):
            self.assertNotIn(stale_gate, acquire)

        validate_wrapper = function_body(
            self.presentation_admission_cpp,
            "Validate_StillCurrent(std::string& strOutStatus) const",
        )
        self.assertIn("Validate_StillCurrent(Diagnostic)", validate_wrapper)
        validate = function_body(
            self.presentation_admission_cpp,
            "Validate_StillCurrent(\n"
            "\t\tVALTAN_CANONICAL_READ_DIAGNOSTIC& OutDiagnostic) const",
        )
        self.assertIn(
            "m_pState->CanonicalAdmission->Validate_StillCurrent", validate
        )
        self.assertIn("physical != m_pState->Receipt", validate)
        self.assertIn("GENERATION_CHANGED", validate)

        exact = function_body(
            self.presentation_admission_cpp,
            "bool Client::CValtanPresentationGenerationReadAdmission::\n"
            "\tAcquire_ExactReceiptFromRoot(",
        )
        self.assertIn("ExpectedReceipt.Is_Valid()", exact)
        self.assertIn(
            "ExpectedReceipt.ServerGameplayRevision != ExpectedServerRevision",
            exact,
        )
        self.assertIn("physical != ExpectedReceipt", exact)
        self.assertIn("m_pState.reset()", exact)

        native = self.presentation_admission_native_tests
        for marker in (
            "stale world-entry generation/inventory blocked the current typed closure",
            "changed local presentation bytes were compared with the world-entry receipt",
            "concurrent presentation write passed the transactional currentness check",
        ):
            self.assertIn(marker, native)

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
            stage.index("ValidateCurrentCandidatePresentationGeneration"),
        )
        self.assertIn("StagedPresentationReceipt", stage)
        self.assertIn("m_pStagedPresentationAdmission", stage)

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
        self.assertIn("AvailablePresentationReceipts.push_back", commit)
        discard = function_body(
            self.network_cpp,
            "void CNetworkManager::Discard_StagedPresentationAlias() noexcept",
        )
        self.assertIn("StagedPresentationReceipt = {}", discard)
        self.assertIn("m_pStagedPresentationAdmission.reset()", discard)

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
        valtan_domain = next(
            row for row in self.build_domains["domains"]
            if row["id"] == "valtan.product"
        )
        self.assertEqual("validation", valtan_domain["kind"])
        self.assertEqual(
            ["Product", "Core", "FullDiagnostic"],
            valtan_domain["profiles"],
        )
        self.assertIn(
            "Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1",
            valtan_domain["action"]["arguments"],
        )
        self.assertIn("Validate", valtan_domain["action"]["arguments"])
        gameplay_domain = next(
            row for row in self.build_domains["domains"]
            if row["id"] == "gameplay.balance"
        )
        self.assertEqual("publisher", gameplay_domain["kind"])
        self.assertIn(
            "Tools/GameplayPipeline/Publish-GameplayBalance.ps1",
            gameplay_domain["action"]["arguments"],
        )
        self.assertIn("-SkipValtanSplitProjection", gameplay_domain["action"]["arguments"])
        self.assertIn("-Mode Validate", self.gameplay_publisher)
        self.assertNotIn("if ($Mode -eq 'Publish') { 'PublishV2' }", self.gameplay_publisher)
        self.assertIn('project-products', self.valtan_projector)
        self.assertNotIn('ValtanChargeImpactActions.json', self.gameplay_publisher)
        self.assertIn("[string]$_.outcome -ceq 'WALL_CONTACT'", self.gameplay_publisher)
        self.assertIn("[string]$motion.kind -cne 'FORWARD'", self.gameplay_publisher)
        self.assertIn("stageKind -cne 'GROGGY'", self.gameplay_publisher)


if __name__ == "__main__":
    unittest.main(verbosity=2)
