#!/usr/bin/env python3
"""Static orchestration contract for Action Presentation Workbench.

The Workbench may compose domain owners, but it must not create a second
network, boss, effect, sound, or arena runtime.  These checks keep the first
integrated slice on stable Product identities and typed Server commands.
"""

from __future__ import annotations

import json
import os
from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[2]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


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


class ActionPresentationWorkbenchContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.animation_h = read("Client/Public/Animation_Tool.h")
        cls.animation_cpp = read("Client/Private/Animation_Tool.cpp")
        cls.animation_binding_h = read(
            "Client/Public/AnimationSkillBindingDocument.h"
        )
        cls.animation_binding_cpp = read(
            "Client/Private/AnimationSkillBindingDocument.cpp"
        )
        cls.animation_binding_harness = read(
            "Tools/ValtanPatternAuditionServiceHarness/Private/"
            "ValtanPatternAnimationBindingDocumentContractTests.cpp"
        )
        cls.balance_h = read("Client/Public/BalanceTool.h")
        cls.balance_cpp = read("Client/Private/BalanceTool.cpp")
        cls.valtan_tree_h = read("Client/Public/ValtanPatternTree.h")
        cls.valtan_tree_cpp = read("Client/Private/ValtanPatternTree.cpp")
        cls.boss_h = read("Client/Public/BossTool.h")
        cls.boss_cpp = read("Client/Private/BossTool.cpp")
        cls.level_h = read("Client/Public/Level_ValtanArena.h")
        cls.level_cpp = read("Client/Private/Level_ValtanArena.cpp")
        cls.client_replication_h = read("Client/Public/ClientReplication.h")
        cls.client_replication_cpp = read("Client/Private/ClientReplication.cpp")
        cls.effect_v2_cpp = read("Client/Private/Effect_Tool_V2.cpp")
        cls.effect_v1_cpp = read("Client/Private/Effect_Tool.cpp")
        cls.main_cpp = read("Client/Private/MainApp.cpp")
        cls.main_h = read("Client/Public/MainApp.h")
        cls.packet_h = read("Shared/Public/Network/PacketMessages.h")
        cls.packet_cpp = read("Shared/Private/Network/PacketMessages.cpp")
        cls.packet_type_h = read("Shared/Public/Network/PacketType.h")
        cls.room_cpp = read("Server/Private/GameRoom.cpp")
        cls.combat_runtime_cpp = read("Server/Private/CombatObjectRuntime.cpp")
        cls.valtan_cpp = read("Client/Private/Valtan.cpp")
        cls.valtan_h = read("Client/Public/Valtan.h")
        cls.combat_sound_document_h = read(
            "Client/Public/ValtanCombatObjectSoundCueDocument.h"
        )
        cls.combat_sound_document_cpp = read(
            "Client/Private/ValtanCombatObjectSoundCueDocument.cpp"
        )
        cls.pattern_sound_document_cpp = read(
            "Client/Private/ValtanPatternSoundCueDocument.cpp"
        )
        cls.pattern_sound_harness = read(
            "Tools/ValtanPatternAuditionServiceHarness/Private/"
            "ValtanPatternSoundCueDocumentContractTests.cpp"
        )

    def test_primary_window_has_one_workbench_identity(self) -> None:
        self.assertRegex(
            self.main_cpp,
            r'toolButton\(\s*"Action Presentation Workbench"\s*,\s*'
            r"DEBUG_TOOL::ANIMATION",
        )
        self.assertRegex(
            self.animation_cpp,
            r'ImGui::Begin\(\s*"Action Presentation Workbench"\s*,',
        )
        self.assertNotRegex(
            self.main_cpp,
            r'toolButton\(\s*"Animation Tool"\s*,',
        )

    def test_workbench_layout_resists_narrow_ini_and_keeps_kakul_detail_visible(
        self,
    ) -> None:
        render = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render()",
        )
        for token in (
            "WORKBENCH_DEFAULT_WIDTH",
            "WORKBENCH_DEFAULT_HEIGHT",
            "ImGui::GetMainViewport()",
            "ImGui::SetNextWindowSizeConstraints",
            "m_bResetWorkbenchLayoutRequested",
            "ImGuiCond_Always",
            'ImGui::SmallButton("Reset Workbench Layout")',
        ):
            self.assertIn(token, render)
        self.assertRegex(
            self.animation_cpp,
            r"WORKBENCH_DEFAULT_WIDTH\s*=\s*1180\.f",
        )
        self.assertRegex(
            self.animation_cpp,
            r"WORKBENCH_DEFAULT_HEIGHT\s*=\s*760\.f",
        )
        self.assertIn(
            "bool_t m_bResetWorkbenchLayoutRequested = false;",
            self.animation_h,
        )

        kakul = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_KakulActionBindings(",
        )
        for token in (
            "KAKUL_ACTION_LIST_MIN_WIDTH",
            "KAKUL_ACTION_DETAIL_MIN_WIDTH",
            'ImGui::InvisibleButton("##KakulActionSplitter"',
            "ImGui::GetIO().MouseDelta.x",
            "m_fKakulActionListWidth",
            '"##KakulActionDetail"',
        ):
            self.assertIn(token, kakul)

    def test_valtan_workbench_has_stable_three_plus_one_pane_shell(self) -> None:
        for token in (
            "enum class VALTAN_WORKBENCH_SELECTION_KIND",
            "m_eValtanWorkbenchSelection",
            "m_strValtanWorkbenchPatternId",
            "m_strValtanWorkbenchStageId",
        ):
            self.assertIn(token, self.animation_h)

        workbench = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPatternMaster(",
        )
        for token in (
            '"##ValtanWorkbenchMain"',
            "ImGuiTableFlags_Resizable",
            '"Master / Outliner"',
            '"Preview / Transport"',
            '"Persistent Detail"',
            '"##ValtanWorkbenchOutliner"',
            '"##ValtanWorkbenchPreview"',
            '"##ValtanWorkbenchDetail"',
            '"##ValtanWorkbenchDataFiles"',
            "Pattern.strPatternId",
            "Stage.strStageId",
            "m_strValtanWorkbenchPatternId",
            "m_strValtanWorkbenchStageId",
            "Render_ValtanStageDraftInspector(",
            "Data/Animation/Authored/Valtan/Valtan.patternbindings.json",
            "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json",
            "Data/Animation/Authored/Valtan/Valtan.patterneffectv1aliases.json",
            "Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json",
            "Data/Animation/Authored/Valtan/Valtan.patternshakecues.json",
            "Data/Encounters/Valtan/ValtanCinematicCamera.json",
            "Data/Encounters/Valtan/ValtanWorldEvents.json",
            "m_hasEffectToolOpenRequest",
            "m_hasCameraToolOpenRequest",
        ):
            self.assertIn(token, workbench)
        self.assertRegex(
            workbench,
            r'ImGui::BeginTable\(\s*"##ValtanWorkbenchMain"\s*,\s*3\s*,',
        )
        self.assertNotRegex(
            workbench,
            r'InputTextMultiline\([^\n]*(?:JSON|Json|json)',
        )

        lanes = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPresentationLanes(",
        )
        self.assertIn("strStageFilter", lanes)
        self.assertIn("Stage.strStageId != strStageFilter", lanes)
        self.assertRegex(
            workbench,
            r"Render_ValtanPresentationLanes\(\s*\*pSelected,\s*"
            r"pSelectedStage->strStageId\s*\)",
        )

    def test_persistent_detail_uses_typed_valtan_animation_binding_owner(
        self,
    ) -> None:
        for token in (
            "enum class VALTAN_WORKBENCH_DETAIL_OWNER",
            "m_eValtanWorkbenchDetailOwner",
            "m_ValtanPatternAnimationBindingDraft",
            "m_bValtanPatternAnimationBindingDirty",
            "Render_ValtanAnimationBindingInspector",
        ):
            self.assertIn(token, self.animation_h)

        inspector = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Render_ValtanAnimationBindingInspector(",
        )
        loader = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Load_ValtanAnimationBindingDraft(",
        )
        self.assertIn(
            "CValtanPatternAnimationBindingDocument::Load_ForAuthoring(", loader
        )
        self.assertIn(
            "m_strValtanPatternAnimationBindingBaselineSourceBytes", loader
        )
        for token in (
            "CValtanPatternAnimationBindingDocument::Save_Atomic(",
            '"Add Sequence Slot"',
            '"Set Blank / NONE Draft"',
            "bSuppressAnimation",
            "strClipOccurrenceId",
            "strMappingBasis = \"PROJECT_AUTHORED\"",
            '"Counter true',
            '"Counter false',
            "Groggy transition sequence slot",
            '"MISSING CONTRACT',
        ):
            self.assertIn(token, inspector)

        workbench = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPatternMaster(",
        )
        self.assertIn(
            "Render_ValtanAnimationBindingInspector(", workbench
        )
        self.assertIn("VALTAN_WORKBENCH_DETAIL_OWNER::ANIMATION", workbench)
        self.assertIn("m_bValtanPatternAnimationBindingDirty", workbench)
        self.assertIn(
            "EDITABLE IN PERSISTENT DETAIL / typed owner Save", workbench
        )

    def test_persistent_detail_exposes_first_ui_admission_rows(self) -> None:
        inspector = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanStageDraftInspector(",
        )
        for token in (
            '"Server wall / blank timeline ms"',
            '"VALTAN_HIGH_JUMP"',
            '"AIRBORNE"',
            '"RELEASE_GRABBED_PLAYERS"',
            '"Typed row %zu',
            '"Release yaw offset deg"',
            "yawOffsetDegrees=%.3f",
            '"Set 180 deg Draft"',
            '"Saved yawOffsetDegrees',
        ):
            self.assertIn(token, inspector)

        counter = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanCounterWindowInspector(",
        )
        for token in ('"Counterable true', '"Counterable false'):
            self.assertIn(token, counter)

        sound = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Render_ValtanPatternSoundInspector(",
        )
        for token in (
            "Cue.strBindingId",
            "Cue.strOccurrenceId",
            "Cue.strSoundBank",
            "Cue.strSoundEvent",
        ):
            self.assertIn(token, sound)

        workbench = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPatternMaster(",
        )
        self.assertIn("Render_ValtanPatternSoundInspector(", workbench)
        self.assertRegex(
            workbench,
            r"if \(Render_ValtanPatternSoundInspector\([\s\S]*?"
            r"bReloadPatternMasterAfterSave = true;",
        )
        self.assertRegex(
            workbench,
            r'Pattern\.strDisplayName\s*\+\s*" \| "\s*\+\s*'
            r'Pattern\.strPatternId',
        )

    def test_counter_and_pattern_sound_use_landed_typed_owner_apis(self) -> None:
        for token in (
            "Render_ValtanCounterWindowInspector",
            "Render_ValtanPatternSoundInspector",
            "m_bValtanPatternSoundCuesDirty",
        ):
            self.assertIn(token, self.animation_h)

        counter = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanCounterWindowInspector(",
        )
        for token in (
            "Get_ValtanCounterWindowDraft(",
            "Set_ValtanCounterWindowDraft(",
            '"Counter Enabled"',
            '"GROGGY"',
            "successStageId",
            "successActionId",
            '"Counterable ENTER=true / EXIT=false"',
            '"Groggy ENTER=true / EXIT=false"',
            '"TIMEOUT/default branch',
        ):
            self.assertIn(token, counter)

        sound = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Render_ValtanPatternSoundInspector(",
        )
        for token in (
            '"Save Pattern Sound"',
            "CValtanPatternSoundCueDocument::Save_Atomic(",
            '"Sound Event"',
            "CSoundCueCatalog::Collect_EventNames(\"Valtan\")",
            "Cue.strSoundBank",
            "Cue.strSoundEvent",
            '"startMs"',
            '"once"',
            '"each_loop"',
            "m_bValtanPatternSoundCuesDirty = true",
            "CValtanPatternSoundCueDocument::Add_AuthoringRow(",
            "CValtanPatternSoundCueDocument::Remove_AuthoringRow(",
            '"Add Exact Pattern Sound Row"',
            '"Remove Exact Pattern Sound Row"',
            "CreatedRowId.strBindingId",
            "PendingRemoveRowId.strOccurrenceId",
            "ClipSourceDurationSecondsByName",
            "CActionPresentationTimeline::Resolve_ClipDuration(",
            "CActionPresentationTimeline::Resolve_CueWallOffset(",
            "fEffectiveSourceEndMilliseconds",
            "fRemainingStageWallSeconds",
            '"Runtime-equivalent source window:',
        ):
            self.assertIn(token, sound)
        duration_lookup = function_body(
            self.animation_cpp,
            "CollectModelClipSourceDurationSeconds(",
        )
        for token in (
            "pModel->Get_NumAnimations()",
            "pModel->Get_AnimationName(",
            "pModel->Get_AnimationTickPerSecond(",
            "pModel->Get_AnimationProgress(",
        ):
            self.assertIn(token, duration_lookup)

        self.assertRegex(
            sound,
            r"CValtanPatternSoundCueDocument::Save_Atomic\(\s*"
            r"m_ValtanPatternSoundCues\s*,\s*"
            r"ClipSourceDurationSecondsByName\s*,",
        )

        reload_sound = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Reload_ValtanPatternSoundCues(",
        )
        self.assertIn(
            "CValtanPatternSoundCueDocument::Load_ForAuthoring(", reload_sound
        )
        self.assertIn(
            "m_strValtanPatternSoundCueBaselineSourceBytes", reload_sound
        )
        self.assertIn("m_bValtanPatternSoundCuesDirty = false", reload_sound)
        self.assertIn("m_bValtanPatternSoundCuesDirty", function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Is_AnyDocumentDirty() const",
        ))

        self.assertIn("Reload_PatternSoundCues", self.valtan_h)
        runtime_reload = function_body(
            self.valtan_cpp,
            "bool_t CValtan::Reload_PatternSoundCues(",
        )
        self.assertIn("m_PatternSoundCuesByActionId = std::move(Staged)", runtime_reload)

    def test_pattern_sound_save_locks_exact_join_dependencies_through_commit(
        self,
    ) -> None:
        save = function_body(
            self.pattern_sound_document_cpp,
            "bool_t Client::CValtanPatternSoundCueDocument::Save_Atomic(",
        )
        for token in (
            "SOUND_JOINED_OWNER_SOURCE_SNAPSHOT",
            "Resolve_EncounterPath()",
            'CValtanPatternAnimationBindingDocument::Resolve_Path("Valtan")',
            "DependencySnapshots[0u].SourceBytes",
            "DependencySnapshots[1u].SourceBytes",
            "SOUND_JOINED_OWNER_COMMIT_GUARD CommitGuard",
            "CommitGuard.Lock_AndVerify(",
        ):
            self.assertIn(token, save)
        for token in (
            "FILE_SHARE_READ",
            "Encounter/Animation bytes changed while staging",
            "SOUND_AUTHORING_SAVE_MUTEX",
        ):
            self.assertIn(token, self.pattern_sound_document_cpp)
        hook = "LOSTARK_TEST_VALTAN_SOUND_MUTATE_DEPENDENCY_BEFORE_COMMIT"
        self.assertIn(hook, self.pattern_sound_document_cpp)
        self.assertIn(hook, self.pattern_sound_harness)
        self.assertIn("mid-save Animation dependency mutation", self.pattern_sound_harness)
        self.assertIn("mid-save Encounter dependency mutation", self.pattern_sound_harness)

    def test_valtan_animation_binding_save_is_cas_and_dependency_closed(self) -> None:
        for token in (
            "Load_ForAuthoring(",
            "expectedBaselineSourceBytes",
            "outCommittedSourceBytes",
        ):
            self.assertIn(token, self.animation_binding_h)

        save = function_body(
            self.animation_binding_cpp,
            "bool_t Client::CValtanPatternAnimationBindingDocument::Save_Atomic(",
        )
        for token in (
            "admittedSourceBytes != expectedBaselineSourceBytes",
            "commitSourceBytes != expectedBaselineSourceBytes",
            "Validate_BossPatternSaveClosure(",
            "baselineDocument",
            "clipSourceDurationSecondsByName",
            "std::array<JOINED_OWNER_SOURCE_SNAPSHOT, 4u> dependencySnapshots",
            "dependencyCommitGuard.Lock_AndVerify(",
            "outCommittedSourceBytes = serialized",
        ):
            self.assertIn(token, save)

        closure = function_body(
            self.animation_binding_cpp,
            "bool_t Validate_BossPatternSaveClosure(",
        )
        for token in (
            "Validate_RequiredActions(",
            "Validate_ChangedBindingModelTimings(",
            "encounterSourceBytes",
            "encounterPath, std::move(encounterSourceBytes)",
            'asset + ".patterneffectcues.json"',
            'asset + ".patternsoundcues.json"',
            'asset + ".patternshakecues.json"',
        ):
            self.assertIn(token, closure)

        for token in (
            '"missing Encounter-required action"',
            '"cue-linked occurrence delete"',
            '"cue-linked action NONE"',
            '"cue-linked timing window"',
            '"missing current model duration changed the file or draft"',
            '"stale CAS Save overwrote an external source mutation"',
            '"dependent Sound owner"',
            '"Encounter owner"',
            '" commit-time CAS did not reject/retain an external mutation"',
        ):
            self.assertIn(token, self.animation_binding_harness)

    def test_animation_save_reloads_runtime_then_defers_joined_view_refresh(
        self,
    ) -> None:
        self.assertIn("Reload_PatternBindings", self.valtan_h)
        runtime_reload = function_body(
            self.valtan_cpp,
            "bool_t CValtan::Reload_PatternBindings(",
        )
        for token in (
            "CValtanPatternAnimationBindingDocument::Load(",
            "Build_PatternTimeline(",
            "staged.emplace(",
            "m_PatternClipByActionId = std::move(staged)",
            "m_iPatternPresentationClipOccurrenceIndex",
        ):
            self.assertIn(token, runtime_reload)
        self.assertLess(
            runtime_reload.index("Build_PatternTimeline("),
            runtime_reload.index("m_PatternClipByActionId = std::move(staged)"),
        )
        self.assertNotIn("m_PatternClipByActionId.clear", runtime_reload)

        inspector = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Render_ValtanAnimationBindingInspector(",
        )
        for token in (
            "if (!Boss->Reload_PatternPresentationAuthoring(PreviewStatus))",
            '"Secondary preview joined reload rejected: "',
        ):
            self.assertIn(token, inspector)
        self.assertIn("bReloadJoinedWorkbenchAfterSave = true", inspector)

        joined_reload = function_body(
            self.valtan_cpp,
            "bool_t CValtan::Reload_PatternPresentationAuthoring(",
        )
        for token in (
            "PreviousBindings",
            "PreviousEffectCues",
            "PreviousSoundCues",
            "PreviousShakeCues",
            "Reload_PatternBindings(StepStatus)",
            "Reload_PatternEffectCues(StepStatus)",
            "Reload_PatternSoundCues(StepStatus)",
            "Reload_PatternShakeCues(StepStatus)",
            "RestorePrevious()",
            "m_PatternClipByActionId = PreviousBindings",
            "m_PatternEffectCuesByActionId = PreviousEffectCues",
            "m_PatternSoundCuesByActionId = PreviousSoundCues",
            "m_PatternShakeCuesByActionId = PreviousShakeCues",
        ):
            self.assertIn(token, joined_reload)

        workbench = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPatternMaster(",
        )
        save_call = workbench.index("Render_ValtanAnimationBindingInspector(")
        deferred_reload = workbench.rindex("if (bReloadPatternMasterAfterSave)")
        self.assertLess(save_call, deferred_reload)
        deferred = workbench[deferred_reload:]
        self.assertIn("if (Reload_ValtanPatternMaster())", deferred)
        self.assertIn("Joined Workbench reload rejected", deferred)
        self.assertIn("previous admitted view preserved", deferred)
        self.assertIn("ReloadDiagnostic", deferred)

    def test_saved_presentation_reloads_authoritative_primary_valtan_and_gates_play(
        self,
    ) -> None:
        for token in (
            "Reload_PrimaryValtanPresentationAuthoring",
            "Reload_PrimaryValtanCombatObjectSoundCues",
            "Can_Play_PrimaryValtanPresentation",
        ):
            self.assertIn(token, self.client_replication_h)
            self.assertIn(token, self.level_h)
        self.assertIn(
            "CPrimaryValtanPresentationFreshnessGate", self.client_replication_h
        )

        joined_reload = function_body(
            self.client_replication_cpp,
            "bool_t Client::CClientReplication::Reload_PrimaryValtanPresentationAuthoring(",
        )
        for token in (
            '"BOSS_VALTAN"',
            "INVALID_NET_ENTITY_ID",
            "Reload_PatternPresentationAuthoring(",
            "m_PrimaryValtanJoinedPresentationFreshness.Reject(",
        ):
            self.assertIn(token, joined_reload)

        combat_reload = function_body(
            self.client_replication_cpp,
            "bool_t Client::CClientReplication::Reload_PrimaryValtanCombatObjectSoundCues(",
        )
        self.assertIn("Reload_CombatObjectSoundCues(", combat_reload)
        self.assertIn(
            "m_PrimaryValtanCombatObjectSoundFreshness.Reject(", combat_reload
        )

        can_play = function_body(
            self.boss_cpp,
            "bool_t Client::CBossTool::Can_Play_ServerPattern(",
        )
        self.assertIn("Can_Play_PrimaryValtanPresentation(", can_play)

        animation_inspector = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Render_ValtanAnimationBindingInspector(",
        )
        sound_inspector = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Render_ValtanPatternSoundInspector(",
        )
        workbench = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPatternMaster(",
        )
        self.assertIn("Reload_PrimaryValtanPresentationAuthoring(", animation_inspector)
        self.assertIn("Reload_PrimaryValtanPresentationAuthoring(", sound_inspector)
        self.assertIn("Reload_PrimaryValtanCombatObjectSoundCues(", workbench)
        self.assertIn("VerifyAuthoritativeFreshnessGate", self.animation_binding_harness)

    def test_every_boss_server_pattern_start_entry_checks_presentation_freshness(
        self,
    ) -> None:
        def require_gate_before(body: str, submission: str) -> None:
            gate = body.index("Can_Play_ServerPattern(")
            submit = body.index(submission)
            self.assertLess(gate, submit)

        direct = function_body(
            self.boss_cpp,
            "bool_t Client::CBossTool::Submit_SelectedPattern()",
        )
        require_gate_before(direct, "CValtanPatternAuditionService::Get().Submit(")

        isolated = function_body(
            self.boss_cpp,
            "bool_t Client::CBossTool::Preview_SelectedFlowSlotIsolated()",
        )
        require_gate_before(isolated, "CValtanPatternAuditionService::Get().Submit(")

        flow_start = function_body(
            self.boss_cpp,
            "bool_t Client::CBossTool::Start_Flow(",
        )
        require_gate_before(flow_start, "CValtanPatternFlowService::Get().Start(")

        next_picker = function_body(
            self.boss_cpp,
            "void Client::CBossTool::Render_NextPatternPicker()",
        )
        require_gate_before(next_picker, "Service.Queue_NextPattern(")

        next_card = function_body(
            self.boss_cpp,
            "void Client::CBossTool::Render_NextPatternCard()",
        )
        next_gate = next_card.index("Can_Play_ServerPattern(")
        self.assertLess(next_card.index("Service.Clear_NextPattern("), next_gate)
        self.assertLess(next_gate, next_card.index("Service.Retry_NextPatternCommand("))

        flow_detail = function_body(
            self.boss_cpp,
            "void Client::CBossTool::Render_FlowSelectedSlot()",
        )
        flow_gate = flow_detail.index("Can_Play_ServerPattern(")
        self.assertLess(flow_detail.index("Stop_AfterCurrent("), flow_gate)
        self.assertLess(flow_gate, flow_detail.index("FlowService.Retry_Start("))

    def test_primary_valtan_freshness_latches_across_despawn_until_reset_or_reload(
        self,
    ) -> None:
        despawn = function_body(
            self.client_replication_cpp,
            "bool Client::CClientReplication::Apply_WorldEntityDespawn(",
        )
        self.assertNotIn(
            "m_PrimaryValtanJoinedPresentationFreshness.Admit(", despawn
        )
        self.assertNotIn(
            "m_PrimaryValtanCombatObjectSoundFreshness.Admit(", despawn
        )
        self.assertIn("A despawn must not erase a reload rejection", despawn)

        reset = function_body(
            self.client_replication_cpp,
            "void Client::CClientReplication::Reset_World(",
        )
        self.assertIn(
            "m_PrimaryValtanJoinedPresentationFreshness.Admit(", reset
        )
        self.assertIn(
            "m_PrimaryValtanCombatObjectSoundFreshness.Admit(", reset
        )
        self.assertIn(
            "despawn/no-consumer lifecycle cleared a rejected freshness gate",
            self.animation_binding_harness,
        )

    def test_joined_tracks_are_one_full_width_sequencer_between_main_and_files(
        self,
    ) -> None:
        workbench = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPatternMaster(",
        )
        main = workbench.index('"##ValtanWorkbenchMain"')
        sequencer = workbench.index('"##ValtanWorkbenchSequencer"')
        data_files = workbench.index('"##ValtanWorkbenchDataFiles"')
        self.assertLess(main, sequencer)
        self.assertLess(sequencer, data_files)
        self.assertEqual(1, workbench.count("Render_ValtanPresentationLanes("))
        self.assertIn('"Sequencer / Joined Tracks"', workbench)
        self.assertIn('"Joined Tracks", "READ-ONLY COMPOSITE', workbench)
        self.assertNotIn("Codec READY / no Product owner path bound", workbench)
        self.assertNotIn("No selected .sequence.json owner", workbench)

        lanes = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPresentationLanes(",
        )
        for token in (
            '"##JoinedTrackSegments"',
            '"Animation"',
            '"Effect"',
            '"Sound"',
            '"Camera"',
            '"World"',
        ):
            self.assertIn(token, lanes)

    def test_joined_lanes_expose_sound_assets_and_combat_hit_gap(self) -> None:
        lane = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPresentationLanes(",
        )
        for token in (
            "Stage.ClipOccurrences",
            "Stage.ProductCues",
            "Stage.CombatObjectEffects",
            "Stage.CameraInvocations",
            "m_ValtanPatternShakeCues",
            "Pattern.WorldEventTriggerRefs",
            "CombatObject.HitIds",
            "CombatObject.HitOffsetsMs",
            "m_ValtanCombatObjectSoundCues",
            'CSoundCueCatalog::Find_Variants("Valtan"',
            "Preview_ValtanSoundAsset",
            "COVERAGE GAP: %zu Server combat-object hit(s)",
            'ImGui::SeparatorText("Camera / Shake")',
            'ImGui::SeparatorText("World Event / Runtime UI")',
        ):
            self.assertIn(token, lane)

        preview = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Preview_ValtanSoundAsset(",
        )
        self.assertRegex(
            preview,
            r"CRuntimeAssetRoot::Resolve\(\s*strResourceAssetId\s*\)",
        )
        self.assertLess(
            preview.index("CRuntimeAssetRoot::Resolve"),
            preview.index("CGameInstance::Get().Play_Sound"),
        )
        for forbidden in ("C:\\Users\\", "Client/Bin/Resources", "..\\"):
            self.assertNotIn(forbidden, preview)

    def test_high_jump_impact_sound_joins_the_server_hit_identity(self) -> None:
        product = json.loads(
            read("Data/Encounters/Valtan/ValtanCombatObjects.json")
        )
        bindings = json.loads(
            read(
                "Data/Animation/Authored/Valtan/"
                "Valtan.combatobjectsoundcues.json"
            )
        )
        sound_catalog = json.loads(
            read("Data/Sound/CharacterSoundCatalog.json")
        )
        resource_root = Path(
            os.environ.get(
                "LOSTARK_RESOURCE_ROOT",
                ROOT / "Client" / "Bin" / "Resources",
            )
        )
        product_sources = {
            (entry["combatObjectArchetypeId"], hit["hitId"])
            for entry in product["objects"]
            for hit in entry["hits"]
        }
        self.assertEqual("lostark.valtan-combat-object-sound-cues", bindings["schema"])
        self.assertGreater(len(bindings["cues"]), 0)
        bound_sources: set[tuple[str, str]] = set()
        for cue in bindings["cues"]:
            source = (cue["combatObjectArchetypeId"], cue["hitId"])
            self.assertIn(source, product_sources)
            self.assertNotIn(source, bound_sources)
            bound_sources.add(source)
            variants = sound_catalog["classes"]["Valtan"][cue["soundEvent"]]
            self.assertGreater(len(variants), 0)
            for asset_id in variants:
                self.assertTrue(asset_id.startswith("Sound/Valtan/"))
                self.assertTrue(asset_id.endswith(".wav"))
                self.assertNotIn("..", Path(asset_id).parts)
                self.assertTrue(
                    (resource_root / Path(asset_id)).is_file(),
                    f"missing physical sound dependency: {asset_id}",
                )

        for token in (
            "S2C_COMBAT_OBJECT_PRESENTATION_EVENT",
            "NETWORK_PROTOCOL_VERSION = 47;",
        ):
            self.assertIn(token, self.packet_type_h)
        for token in (
            "strHitId",
            "PendingPresentationEvents",
        ):
            self.assertIn(token, self.combat_runtime_cpp)
        for token in (
            "Apply_CombatObjectPresentationEvent",
            "m_CombatObjectSoundCuesBySource",
            "Play_Sound",
        ):
            self.assertIn(token, self.valtan_cpp)

    def test_trash_capture_preimpact_has_the_authored_slam_sound(self) -> None:
        sounds = json.loads(read(
            "Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json"
        ))["cues"]
        bindings = json.loads(read(
            "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
        ))["bindings"]
        catalog = json.loads(read("Data/Sound/CharacterSoundCatalog.json"))
        expected_patterns = {
            "VALTAN_TRASH", "VALTAN_TRASH_CATCH_IF",
            "VALTAN_TRASH_CATCH_SUCCESS",
        }
        slam = [
            row for row in sounds
            if row["stageId"] == "CATCH_PRE_IMPACT"
            and row["soundEvent"] == "G_Voltan2_Attack12_Shot3"
        ]
        self.assertEqual(expected_patterns, {row["patternId"] for row in slam})
        self.assertEqual(3, len(slam))
        clips = {
            clip["clipOccurrenceId"]: clip
            for binding in bindings
            for clip in binding.get("clips", [])
            if isinstance(clip, dict)
        }
        for cue in slam:
            clip = clips[cue["clipOccurrenceId"]]
            self.assertEqual(1200, cue["startMs"] - clip["sourceStartMs"])
            self.assertLess(cue["startMs"],
                            clip["sourceStartMs"] + clip["playMs"])
        variants = catalog["classes"]["Valtan"]["G_Voltan2_Attack12_Shot3"]
        self.assertEqual([
            "Sound/Valtan/g_voltan2_attack12_shot3__382941727.wav"
        ], variants)
        self.assertTrue((ROOT / "Client/Bin/Resources" / variants[0]).is_file())

    def test_only_one_user_facing_save_drives_explicit_internal_stages(self) -> None:
        workbench = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPatternMaster(",
        )
        self.assertEqual(1, len(re.findall(r'ImGui::Button\(\s*"Save"', workbench)))
        for forbidden_label in (
            '"Validate Joined"',
            '"Save Domain"',
            '"Publish Candidate"',
            '"Apply Revision"',
            '"Apply Hot Reload"',
            '"Save Everything"',
        ):
            self.assertNotIn(forbidden_label, workbench)
        self.assertIn("m_pBalanceTool->Save_ValtanProduct", workbench)
        for sound_save_token in (
            "Validate_SourceDraft",
            "Begin_SourceReplacement",
            "Commit_SourceReplacement",
            "Rollback_SourceReplacement",
            "Reload_CombatObjectSoundCues",
            "m_bValtanCombatObjectSoundCuesDirty",
        ):
            self.assertIn(sound_save_token, workbench)
        sound_begin = workbench.index("Begin_SourceReplacement")
        product_save = workbench.index("m_pBalanceTool->Save_ValtanProduct")
        sound_rollback = workbench.index("Rollback_SourceReplacement")
        sound_commit = workbench.index("Commit_SourceReplacement")
        self.assertLess(sound_begin, product_save)
        self.assertLess(product_save, sound_rollback)
        self.assertLess(sound_rollback, sound_commit)
        self.assertIn(
            "gameplay Product/runtime and Sound source were preserved",
            workbench,
        )

        reload_guard = re.search(
            r"ImGui::BeginDisabled\((?P<guard>.*?)\);\s*"
            r'if \(ImGui::SmallButton\("Reload Valtan Pattern Master"\)\)',
            workbench,
            re.S,
        )
        self.assertIsNotNone(reload_guard)
        self.assertIn(
            "m_bValtanCombatObjectSoundCuesDirty",
            reload_guard.group("guard"),
        )

        save = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Save_ValtanProduct(",
        )
        validation = save.index("Validate_ValtanDraft")
        authoring = save.index("Save_ValtanAuthoring")
        product = save.index("Publish_ValtanCandidate")
        self.assertLess(validation, authoring)
        self.assertLess(authoring, product)
        self.assertIn("Apply_ValtanRevision", save)
        apply_revision = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Apply_ValtanRevision(",
        )
        self.assertIn("RequestValtanHotReload", apply_revision)

        sound_reload = function_body(
            self.valtan_cpp,
            "bool_t CValtan::Reload_CombatObjectSoundCues(",
        )
        self.assertNotIn(
            "m_iLastCombatObjectPresentationEventSequence = 0u", sound_reload
        )
        for token in ("MoveFileExW", "MOVEFILE_REPLACE_EXISTING"):
            self.assertIn(token, self.combat_sound_document_cpp)

    def test_joined_save_sound_replacement_keeps_exact_recovery_bytes(self) -> None:
        for declaration in (
            "Begin_SourceReplacement",
            "Commit_SourceReplacement",
            "Rollback_SourceReplacement",
        ):
            self.assertIn(declaration, self.combat_sound_document_h)

        begin = function_body(
            self.combat_sound_document_cpp,
            "bool_t Client::CValtanCombatObjectSoundCueDocument::Begin_SourceReplacement(",
        )
        for token in (
            "Validate_SourceDraft",
            "Write_StagedSource",
            "CopyFileW",
            "MoveFileExW",
            "MOVEFILE_REPLACE_EXISTING",
            "MOVEFILE_WRITE_THROUGH",
            "transaction.bActive = true",
        ):
            self.assertIn(token, begin)
        self.assertLess(begin.index("CopyFileW"), begin.index("MoveFileExW"))

        rollback = function_body(
            self.combat_sound_document_cpp,
            "bool_t Client::CValtanCombatObjectSoundCueDocument::Rollback_SourceReplacement(",
        )
        for token in (
            "transaction.Rollback.c_str()",
            "transaction.Destination.c_str()",
            "MOVEFILE_REPLACE_EXISTING",
            "MOVEFILE_WRITE_THROUGH",
            "DeleteFileW",
        ):
            self.assertIn(token, rollback)

        standalone_save = function_body(
            self.combat_sound_document_cpp,
            "bool_t Client::CValtanCombatObjectSoundCueDocument::Save_Source(",
        )
        self.assertLess(
            standalone_save.index("Begin_SourceReplacement"),
            standalone_save.index("Commit_SourceReplacement"),
        )

    def test_workbench_uses_stable_domain_boundaries_not_network(self) -> None:
        combined = self.animation_h + "\n" + self.animation_cpp
        for forbidden in (
            '#include "NetworkManager.h"',
            '#include "ValtanPatternAuditionService.h"',
            "CNetworkManager::",
            "CValtanPatternAuditionService::",
            "Send_ValtanAudition",
        ):
            self.assertNotIn(forbidden, combined)
        for route in (
            "CMainApp::Get_Active",
            "Debug_SelectCompletePlayPattern",
            "Debug_CompletePlaySelected",
            "m_pBossTool->Set_ServerArenaPreset",
            "m_pBalanceTool->Set_ValtanStageDraft",
            "Consume_EffectToolOpenRequest",
            "Consume_CameraToolOpenRequest",
        ):
            self.assertIn(route, combined)
        self.assertIn(
            "m_pAnimationTool->Consume_CameraToolOpenRequest",
            self.main_cpp,
        )

    def test_typed_stage_inspector_reuses_balance_draft_and_keeps_clocks_separate(
        self,
    ) -> None:
        for token in (
            "struct PATTERN_STAGE_EDIT final",
            "hitOuterRadius",
            "hitInnerRadius",
            "hitAngleDegrees",
            "hitLength",
            "hitHalfWidth",
            "hitCount",
            "hitIntervalMs",
            "hitDelayMs",
            "damageProfileId",
            "pushRangeM",
            "pushMs",
            "knockdown",
            "downMs",
            "Get_ValtanStageDraft",
            "Set_ValtanStageDraft",
        ):
            self.assertIn(token, self.balance_h)

        setter = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Set_ValtanStageDraft(",
        )
        for token in (
            "FindValtanPattern",
            "FindValtanStage",
            "DamageProfile, response, and explicit-offset ownership are read-only",
            "Manual Server audition duration is locked",
            "IsValtanStageGeometryValid",
            "finalIntervalHit",
            "captureReactionValid",
            "MarkDirty(true)",
        ):
            self.assertIn(token, setter)
        for forbidden in ("ofstream", "CNetworkManager", "Send_"):
            self.assertNotIn(forbidden, setter)
        self.assertIn(
            'readBoundedDouble(*hit, "pushRangeM", -20.0, 20.0',
            self.balance_cpp,
        )

        inspector = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanStageDraftInspector(",
        )
        for token in (
            "Get_ValtanStageDraft",
            "Set_ValtanStageDraft",
            'ImGui::SeparatorText("Server Stage Clock")',
            'ImGui::SeparatorText("Stage Identity / Motion")',
            'ImGui::SeparatorText("Server Stage Actions")',
            'ImGui::SeparatorText("Branches / Counter Proxy")',
            'ImGui::SeparatorText("Effect Cue Details")',
            'ImGui::SeparatorText("Server Hit / Collider")',
            'ImGui::SeparatorText("Server Hit Schedule")',
            'ImGui::SeparatorText("Server Player Reaction")',
            "DamageProfile (read-only)",
            "AWAY_FROM_HIT_SOURCE",
            "TOWARD_HIT_SOURCE",
            "derived speed %.3f m/s",
            "AIRBORNE duration is the boss stage/blank wall-clock",
            '"Release yaw offset deg"',
            '"Local Y rotation deg"',
            '"arena.center.facing"',
        ):
            self.assertIn(token, inspector)
        for forbidden in ("CDataJson", "CNetworkManager", "ofstream"):
            self.assertNotIn(forbidden, inspector)

        for token in (
            "SET_STAGE_GRABBED_RELEASE",
            "SET_EFFECT_CUE_LOCAL_YAW",
            "fYawOffsetDegrees",
            "strOccurrenceId",
        ):
            self.assertIn(token, self.balance_cpp + self.valtan_tree_h)
        self.assertNotRegex(
            inspector,
            r"Collider NONE[\s\S]{0,180}return;",
            "event-only stages must continue to render Action/Effect families",
        )

        for token in (
            "strDirectionPolicy",
            "fSpeedMps",
            "fMaximumDistanceM",
            "iLifetimeMs",
        ):
            self.assertIn(token, self.valtan_tree_h)
            self.assertIn(token, self.valtan_tree_cpp)
        lane = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPresentationLanes(",
        )
        for token in (
            "Product clock (read-only): life %u ms",
            "combat-object +%u ms (local clock; stage duration %u ms)",
            "Separate clocks: AIRBORNE stage %u ms | axe lifetime %u ms | first axe-local hit atMs %u",
        ):
            self.assertIn(token, lane)

    def test_effect_v2_server_play_reuses_shared_complete_play_owner(self) -> None:
        play = function_body(
            self.effect_v2_cpp,
            "bool_t Client::CEffect_Tool_V2::Try_PlayValtanServerPattern(",
        )
        for token in (
            "CMainApp::Get_Active",
            "Debug_SelectCompletePlayPattern",
            "Debug_CompletePlaySelected",
            "Pattern.strPatternId",
        ):
            self.assertIn(token, play)
        for forbidden in (
            "CNetworkManager::",
            "CValtanPatternAuditionService::Get().Submit",
            "Send_ValtanAudition",
            "Set_Visible",
        ):
            self.assertNotIn(forbidden, play)
        self.assertIn(
            'ImGui::Button("Complete Play (Server/Arena)")',
            self.effect_v2_cpp,
        )

    def test_arena_presets_cross_the_single_level_request_owner(self) -> None:
        for token in (
            "VALTAN_ARENA_PRESET",
            "SET_ARENA_PRESET",
        ):
            self.assertIn(token, self.packet_h)
            self.assertIn(token, self.packet_cpp)

        boss_route = function_body(
            self.boss_cpp,
            "bool_t Client::CBossTool::Set_ServerArenaPreset(",
        )
        self.assertIn("CLevel_ValtanArena::Get_Active", boss_route)
        self.assertIn("arena->Set_ArenaPreset", boss_route)
        self.assertNotIn("CNetworkManager", boss_route)

        level_route = function_body(
            self.level_cpp,
            "bool_t CLevel_ValtanArena::Set_ArenaPreset(",
        )
        self.assertIn("Submit_Audition", level_route)
        self.assertIn("SET_ARENA_PRESET", level_route)

        server_route = function_body(
            self.room_cpp,
            "LostArk::Server::CGameRoom::Evaluate_ValtanAudition(",
        )
        for token in (
            "Resolve_ValtanArenaPreset",
            "Reset_ValtanAuditionState",
            "Prepare_ValtanTimelineArenaState",
            "Commit_WorldDestructionTransaction",
            "bAutomaticPatternSequenceAuditionHold",
        ):
            self.assertIn(token, server_route)

        for label in (
            '"Fresh / All Walls"',
            '"Circle / Walls Gone"',
            '"Break 3 O\'Clock"',
            '"Break 9 O\'Clock"',
            '"Break 3 + 9 O\'Clock"',
        ):
            self.assertIn(label, self.animation_cpp)

        active_state = function_body(
            self.level_cpp,
            "CLevel_ValtanArena::Get_ArenaActiveState() const",
        )
        for token in (
            "Get_WorldDestructionGroupStates",
            "OUTER_RING_GROUP_PREFIX",
            "THREE_OCLOCK_GROUP_PREFIX",
            "NINE_OCLOCK_GROUP_PREFIX",
            "Get_WorldDestructionDiagnostics",
            "iActiveWallCollisionCount",
            "iActiveNavBlockerRegionCount",
            "iNavigationRevision",
            "Get_ActiveActorCount",
        ):
            self.assertIn(token, active_state)

        global_controls = function_body(
            self.main_cpp,
            "void CMainApp::RenderServerArenaActiveControls()",
        )
        for token in (
            "Get_ServerArenaActiveState",
            "Set_ServerArenaPreset",
            "Active boxes are replicated facts",
            "VALTAN_ARENA_PRESET::FRESH",
            "VALTAN_ARENA_PRESET::CIRCLE_WALLS_GONE",
            "VALTAN_ARENA_PRESET::THREE_OCLOCK_BROKEN",
            "VALTAN_ARENA_PRESET::NINE_OCLOCK_BROKEN",
            "VALTAN_ARENA_PRESET::BOTH_SIDES_BROKEN",
            "active collision",
            "active nav regions",
        ):
            self.assertIn(token, global_controls)
        self.assertNotIn("Set_ServerArenaChannelActive", global_controls)
        self.assertNotIn("Set_Visible", global_controls)

    def test_debug_workspace_is_non_exclusive(self) -> None:
        ensure = function_body(
            self.main_cpp,
            "HRESULT CMainApp::EnsureDebugTool(",
        )
        for forbidden in (
            "DEBUG_TOOL::MAP != eTool",
            "DEBUG_TOOL::CAMERA != eTool",
            "m_eActiveDebugTool",
        ):
            self.assertNotIn(forbidden, ensure)
        self.assertIn("SetDebugToolVisible(eTool, true)", ensure)

        render = function_body(self.main_cpp, "HRESULT CMainApp::Render()")
        for tool in (
            "MAP",
            "ANIMATION",
            "EFFECT",
            "EFFECT_V2",
            "RENDERING",
            "UI",
            "BALANCE",
            "BOSS",
            "CAMERA",
        ):
            self.assertIn(
                f"IsDebugToolVisible(DEBUG_TOOL::{tool})",
                render,
            )
        self.assertNotIn("switch (m_eActiveDebugTool)", render)

        for relative, token in (
            ("Client/Private/MapTool.cpp", "Debug_CompletePlaySelected"),
            ("Client/Private/HUDLayoutTool.cpp", "Debug_CompletePlaySelected"),
            ("Client/Private/CameraTool.cpp", "Debug_CompletePlaySelected"),
        ):
            self.assertIn(token, read(relative), relative)

        complete_play = function_body(
            self.main_cpp,
            "bool_t CMainApp::Debug_CompletePlaySelected(",
        )
        self.assertIn("m_pBossTool->Play_ServerPattern", complete_play)
        self.assertNotIn("CNetworkManager", complete_play)

        for token in (
            "Debug_SelectCompletePlayPattern",
            "Debug_GetSelectedCompletePlayPatternId",
            "m_strCompletePlayPatternId",
        ):
            self.assertIn(token, self.main_h + self.main_cpp)
        self.assertNotIn("m_iCompletePlayPattern", self.main_h + self.main_cpp)

        for source in (
            self.animation_cpp,
            self.effect_v1_cpp,
            self.effect_v2_cpp,
            self.boss_cpp,
            read("Client/Private/MapTool.cpp"),
            read("Client/Private/HUDLayoutTool.cpp"),
            read("Client/Private/CameraTool.cpp"),
        ):
            if "Complete Play" in source:
                self.assertIn("Debug_CompletePlaySelected", source)

        for source in (
            self.animation_cpp,
            self.effect_v1_cpp,
            self.effect_v2_cpp,
        ):
            self.assertIn("Debug_SelectCompletePlayPattern", source)

    def test_effect_v2_hide_and_level_change_release_preview_not_draft(self) -> None:
        deactivate = function_body(
            self.effect_v2_cpp,
            "void Client::CEffect_Tool_V2::Deactivate()",
        )
        for token in (
            "Stop_ValtanTimeline()",
            "Clear_FollowTarget()",
            "Remove_GameObject_from_Layer",
            "Despawn_Target()",
        ):
            self.assertIn(token, deactivate)
        for forbidden in (
            "m_SlotBindings.clear",
            "m_Documents.clear",
            "m_szEffectId[0]",
        ):
            self.assertNotIn(forbidden, deactivate)
        self.assertIn("m_pEffectToolV2->Deactivate()", self.main_cpp)
        self.assertIn("m_pEffectToolV2->On_LevelChanged()", self.main_cpp)

        main_update = function_body(self.main_cpp, "void CMainApp::Update(")
        for token in (
            "m_eDebugInputOwner",
            "DEBUG_TOOL::MAP == m_eDebugInputOwner",
            "DEBUG_TOOL::ANIMATION == m_eDebugInputOwner",
            "DEBUG_TOOL::CAMERA == m_eDebugInputOwner",
        ):
            self.assertIn(token, main_update)
        self.assertIn("Explicit viewport/preview owner", self.main_cpp)
        map_update = function_body(
            read("Client/Private/MapTool.cpp"),
            "void Client::CMapTool::Update(",
        )
        self.assertIn("bAllowWorldInput && isMapAuthoringLevel", map_update)

    def test_complete_play_preserves_server_arena_and_uses_scroll_selection(
        self,
    ) -> None:
        controls = function_body(
            self.main_cpp,
            "void CMainApp::RenderCompletePlayControls()",
        )
        self.assertRegex(
            controls,
            r'ImGui::BeginChild\(\s*"CompletePlayPatternInventory"',
        )
        for token in (
            "ImGuiListClipper",
            "ImGui::Selectable",
            "Debug_CompletePlaySelected",
        ):
            self.assertIn(token, controls)
        for forbidden in (
            "BeginCombo",
            "Set_ServerArenaPreset",
            "VALTAN_ARENA_PRESET::FRESH",
        ):
            self.assertNotIn(forbidden, controls)

        pattern_play = function_body(self.room_cpp, "if (isPatternIdPlay)")
        self.assertIn("Reset_ValtanBossOnlyAuditionState", pattern_play)
        self.assertNotIn("Reset_ValtanAuditionState", pattern_play)
        for preserved_owner in (
            "m_WorldDestructionRuntime",
            "m_EncounterPropRuntime",
            "m_ServerCollisionSystem.Reset_RuntimeStates",
            "m_ServerNavigation.Reset_RuntimeBlockers",
        ):
            self.assertNotIn(preserved_owner, pattern_play)

    def test_workbench_exposes_applied_sources_and_valtan_balance_owner(
        self,
    ) -> None:
        workbench = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPatternMaster(",
        )
        for token in (
            '"Applied Product Sources / Editability"',
            "Data/Valtan/Valtan.gameplay.json",
            "Data/Balance/BossProfiles.json and DamageProfiles.json",
            "Data/Valtan/Valtan.presentation.json",
            '"Open Valtan Balance / Gameplay"',
            "m_pBalanceTool->Open_Valtan()",
            "Animation Sequence Intake is an offline reviewed source",
        ):
            self.assertIn(token, workbench)

        self.assertIn("void Open_Valtan();", self.balance_h)
        open_valtan = function_body(
            self.balance_cpp,
            "void Client::CBalanceTool::Open_Valtan()",
        )
        for token in ("Open();", "m_showPlayers = false", '"BOSS_VALTAN"'):
            self.assertIn(token, open_valtan)

    def test_high_jump_axe_count_uses_the_typed_product_draft(self) -> None:
        inspector = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanStageDraftInspector(",
        )
        for token in (
            '"VALTAN_HIGH_JUMP"',
            '"AIRBORNE"',
            '"Axes per alive player"',
            "Get_ValtanHighJumpAxeCountDraft",
            "Set_ValtanHighJumpAxeCountDraft",
            '"PER_ALIVE_PLAYER draft %u | saved %u',
        ):
            self.assertIn(token, inspector)
        for forbidden in (
            "m_valtanAxeVolley",
            "SET_AXE_VOLLEY",
            "Data/Valtan/Valtan.gameplay.json",
        ):
            self.assertNotIn(forbidden, inspector)

    def test_resource_files_is_a_read_only_orchestration_index(self) -> None:
        refresh = function_body(
            self.main_cpp,
            "void CMainApp::RefreshDebugResourceFiles()",
        )
        render = function_body(
            self.main_cpp,
            "void CMainApp::RenderDebugResourceFiles()",
        )
        open_file = function_body(
            self.main_cpp,
            "void CMainApp::OpenDebugResourceFile(",
        )
        for token in (
            "CRuntimeAssetRoot::Get_ResourceRoot",
            "CProjectDataRoot::Get",
            '"DataFiles/Map"',
            '"Data/Effects/V2"',
            '"Resources/Sound"',
            '"KoukuSaton"',
            '"Resources/Map/LV_LUT_MIDNIGHTC_ED/"',
            '"Resources/Character/MN_RPCT_05/"',
        ):
            self.assertIn(token, refresh)
        for forbidden in (
            "copy_file",
            "copy(",
            "rename(",
            "remove(",
            "ofstream",
        ):
            self.assertNotIn(forbidden, refresh + open_file)
        self.assertIn("ImGuiListClipper", render)
        self.assertIn("OpenDebugResourceFile(iFile)", render)
        self.assertIn("EnsureDebugTool(file.eTool)", open_file)
        self.assertIn("DEBUG_TOOL::ANIMATION", open_file)
        self.assertIn("DEBUG_TOOL::BOSS", open_file)


if __name__ == "__main__":
    unittest.main()
