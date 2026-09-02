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
        cls.valtan_view_admission_h = read(
            "Client/Public/ValtanViewAdmission.h"
        )
        cls.animation_cpp = read("Client/Private/Animation_Tool.cpp")
        cls.composition_h = read(
            "Client/Public/ActionCompositionWorkbench.h"
        )
        cls.composition_cpp = read(
            "Client/Private/ActionCompositionWorkbench.cpp"
        )
        cls.composition_blueprint_cpp = read(
            "Client/Private/ActionCompositionWorkbench_Blueprint.cpp"
        )
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
        cls.character_preview_cpp = read(
            "Client/Private/CharacterPreviewPanel.cpp"
        )
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
        developer_tools = function_body(
            self.main_cpp,
            "void CMainApp::RenderDeveloperTools()",
        )
        composition = function_body(
            self.composition_cpp,
            "void Client::CActionCompositionWorkbench::Render()",
        )
        animation = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render()",
        )
        self.assertRegex(
            developer_tools,
            r'toolCell\(\s*"Action Composition Workbench"\s*,\s*'
            r"DEBUG_TOOL::COMPOSITION",
        )
        self.assertRegex(
            developer_tools,
            r'toolCell\(\s*"Animation Clip Tool"\s*,\s*'
            r"DEBUG_TOOL::ANIMATION",
        )
        self.assertIn('"##DeveloperToolLaunchGrid"', developer_tools)
        self.assertIn("iToolButtonColumns", developer_tools)
        for window in (
            '"Composition Patterns###CompositionPatternsWindow"',
            '"Composition Preview###CompositionPreviewWindow"',
            '"Composition Sequencer###CompositionSequencerWindowResizableV3"',
            '"Box Detail###CompositionDetailsWindow"',
            '"Composition Resources###CompositionResourcesWindowResizableV2"',
            '"Server Replay###CompositionSessionWindow"',
        ):
            self.assertIn(window, self.composition_cpp)
        self.assertIn(
            '"Animation Clip Tool###AnimationClipToolResizableV1"',
            animation,
        )
        self.assertRegex(
            self.main_h,
            r"enum class DEBUG_TOOL[\s\S]*?COMPOSITION,[\s\S]*?ANIMATION,",
        )
        self.assertNotRegex(
            developer_tools,
            r'"Action Composition Workbench"\s*,\s*DEBUG_TOOL::ANIMATION',
        )
        self.assertNotRegex(
            developer_tools,
            r'"Animation Clip Tool"\s*,\s*DEBUG_TOOL::COMPOSITION',
        )

    def test_f1_has_no_hard_bounds_and_owner_windows_keep_layout_contracts(
        self,
    ) -> None:
        developer_tools = function_body(
            self.main_cpp,
            "void CMainApp::RenderDeveloperTools()",
        )
        animation = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render()",
        )
        composition = function_body(
            self.composition_cpp,
            "void Client::CActionCompositionWorkbench::Render()",
        )
        for token in (
            "ImGui::GetMainViewport()",
            "ImGui::SetNextWindowSize(vDefaultSize, ImGuiCond_FirstUseEver)",
            '"LostArk Developer Tools###LostArkDeveloperToolsResizableV1"',
        ):
            self.assertIn(token, developer_tools)
        self.assertNotIn("ImGuiWindowFlags_AlwaysAutoResize", developer_tools)
        self.assertNotIn("ImGui::SetNextWindowSizeConstraints", developer_tools)

        for token in (
            "WORKBENCH_DEFAULT_WIDTH",
            "WORKBENCH_DEFAULT_HEIGHT",
            "ImGui::GetMainViewport()",
            "m_bResetWorkbenchLayoutRequested",
            "ImGuiCond_Always",
            'ImGui::SmallButton("Reset Animation Tool Layout")',
            '"Animation Clip Tool###AnimationClipToolResizableV1"',
            "ImGui::SetNextWindowSizeConstraints",
        ):
            self.assertIn(token, animation)
        self.assertRegex(
            self.animation_cpp,
            r"WORKBENCH_DEFAULT_WIDTH\s*=\s*900\.f",
        )
        self.assertRegex(
            self.animation_cpp,
            r"WORKBENCH_DEFAULT_HEIGHT\s*=\s*700\.f",
        )
        self.assertIn(
            "bool_t m_bResetWorkbenchLayoutRequested = false;",
            self.animation_h,
        )
        self.assertNotIn("ImGuiWindowFlags_AlwaysAutoResize", animation)

        for token in (
            "BuildCompositionDefaultLayout",
            "ImGui::GetMainViewport()",
            "m_bResetLayoutRequested",
            "ImGuiCond_FirstUseEver",
        ):
            self.assertIn(token, self.composition_cpp)
        for window_signature in (
            "void Client::CActionCompositionWorkbench::Render_PatternsWindow(",
            "void Client::CActionCompositionWorkbench::Render_PreviewWindow(",
            "void Client::CActionCompositionWorkbench::Render_SequencerWindow(",
            "void Client::CActionCompositionWorkbench::Render_DetailsWindow(",
            "void Client::CActionCompositionWorkbench::Render_ResourcesWindow(",
        ):
            self.assertNotIn(
                "ImGuiWindowFlags_AlwaysAutoResize",
                function_body(self.composition_cpp, window_signature),
            )
        self.assertNotIn(
            "ImGui::SetNextWindowSizeConstraints",
            self.composition_cpp,
            "Composition must not turn its default size into a hard bound",
        )
        for token in (
            "COMPOSITION_RESOURCES_DEFAULT_HEIGHT_SCALE = 2.f",
            "COMPOSITION_SEQUENCER_DEFAULT_HEIGHT_SCALE = 2.f",
            "COMPOSITION_LEFT_COLUMN_WIDTH_RATIO = 0.18f",
            "COMPOSITION_RIGHT_COLUMN_WIDTH_RATIO = 0.20f",
            "TIMELINE_ROW_HEIGHT = 48.f",
            "TIMELINE_BLOCK_VERTICAL_PADDING = 6.f",
            "TIMELINE_CANVAS_MINIMUM_HEIGHT = 420.f",
            "leftBottomHeight * COMPOSITION_RESOURCES_DEFAULT_HEIGHT_SCALE",
            "sequencerHeight * COMPOSITION_SEQUENCER_DEFAULT_HEIGHT_SCALE",
            '"Composition Resources###CompositionResourcesWindowResizableV2"',
            '"Composition Sequencer###CompositionSequencerWindowResizableV3"',
        ):
            self.assertIn(token, self.composition_cpp)
        column_ratios = re.search(
            r"COMPOSITION_LEFT_COLUMN_WIDTH_RATIO\s*=\s*([0-9.]+)f;"
            r"[\s\S]*?COMPOSITION_RIGHT_COLUMN_WIDTH_RATIO\s*=\s*([0-9.]+)f;",
            self.composition_cpp,
        )
        self.assertIsNotNone(column_ratios)
        left_ratio, right_ratio = map(float, column_ratios.groups())
        self.assertAlmostEqual(0.18, left_ratio)
        self.assertAlmostEqual(0.20, right_ratio)
        self.assertAlmostEqual(0.62, 1.0 - left_ratio - right_ratio)
        self.assertNotIn(
            '"Composition Resources###CompositionResourcesWindow"',
            self.composition_cpp,
        )
        self.assertNotIn(
            '"Composition Sequencer###CompositionSequencerWindow"',
            self.composition_cpp,
        )
        self.assertIn("bool_t m_bResetLayoutRequested = false;", self.composition_h)

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
            "Render_ValtanSelectedResourceUsage(*pSelected, pSelectedStage);",
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

    def test_valtan_arena_without_model_still_enters_data_only_master(self) -> None:
        render = function_body(
            self.composition_cpp,
            "void Client::CActionCompositionWorkbench::Render()",
        )
        preview = function_body(
            self.composition_cpp,
            "void Client::CActionCompositionWorkbench::Render_Preview(",
        )
        for token in (
            "Reload_Canonical()",
            "Normalize_Selection()",
            "Find_SelectedPattern()",
            "Ensure_TimelineCache(pPattern)",
            "Render_SequencerWindow(",
            "Render_PatternsWindow(",
            "Render_PreviewWindow(",
            "Render_DetailsWindow(",
            "Render_ResourcesWindow(",
        ):
            self.assertIn(token, render)
        self.assertNotIn("Render_SessionWindow(", render)
        self.assertLess(
            render.index("Reload_Canonical()"),
            render.index("Render_SequencerWindow("),
            "canonical data admission must not wait for a preview model",
        )
        self.assertNotIn("Resolve_Model", render)
        self.assertNotIn("CAnimationTargetService", render)
        self.assertIn('Preview.bModelReady ? "READY" : "DATA-ONLY"', preview)
        self.assertIn(
            "ImGui::BeginDisabled(nullptr == m_pAnimationTool", preview
        )
        self.assertNotIn(
            "if (nullptr == m_pAnimationTool)\n\t\treturn",
            render,
            "a missing preview owner may disable transport, not erase data panels",
        )

    def test_first_dirty_balance_open_preserves_draft_and_stages_product_browser(
        self,
    ) -> None:
        reload_canonical = function_body(
            self.composition_cpp,
            "bool_t Client::CActionCompositionWorkbench::Reload_Canonical()",
        )
        for token in (
            "const bool_t bBalanceDraftDirty",
            "const bool_t bHasDisplaySnapshot",
            "if (bBalanceDraftDirty && bHasDisplaySnapshot)",
            "if (bBalanceDraftDirty)",
            "The draft was left untouched",
            "only the last saved canonical Product was staged for read-only browsing",
        ):
            self.assertIn(token, reload_canonical)
        first_open_fallback = reload_canonical[
            reload_canonical.rindex("if (bBalanceDraftDirty)") :
            reload_canonical.index("std::string AuthoringStatus;")
        ]
        self.assertIn("PreserveOrCommitReadOnlyProduct(", first_open_fallback)
        self.assertNotIn("Reload_ValtanSource", first_open_fallback)
        self.assertNotIn("Discard", first_open_fallback)
        self.assertLess(
            reload_canonical.index("Build_PlayablePatternInventory("),
            reload_canonical.rindex("if (bBalanceDraftDirty)"),
            "the saved Product and playable inventory must stage before the dirty-draft read-only commit",
        )
        self.assertLess(
            reload_canonical.rindex("if (bBalanceDraftDirty)"),
            reload_canonical.index("Reload_ValtanSource("),
            "a dirty Balance draft must return through Product-only admission before source reload can replace it",
        )

    def test_patterns_window_exposes_canonical_admission_status_and_reload(self) -> None:
        patterns_window = function_body(
            self.composition_cpp,
            "void Client::CActionCompositionWorkbench::Render_PatternsWindow(",
        )
        for token in (
            'ImGui::TextDisabled("Canonical admission: %s", Admission_Label())',
            'ImGui::SmallButton("Reload Canonical")',
            "(void)Reload_Canonical();",
            'ImGui::TextWrapped("Canonical status: %s", m_strStatus.c_str())',
        ):
            self.assertIn(token, patterns_window)
        self.assertLess(
            patterns_window.index("Admission_Label()"),
            patterns_window.index('ImGui::BeginTabBar("##CompositionPatternDomainTabs")'),
        )

    def test_workbench_can_reclaim_preview_owner_after_domain_deep_link(
        self,
    ) -> None:
        preview = function_body(
            self.composition_cpp,
            "void Client::CActionCompositionWorkbench::Render_Preview(",
        )
        consume = function_body(
            self.composition_cpp,
            "bool_t Client::CActionCompositionWorkbench::"
            "Consume_PreviewOwnerClaimRequest()",
        )
        main_update = function_body(self.main_cpp, "void CMainApp::Update(")

        for token in (
            "Consume_PreviewOwnerClaimRequest",
            "Set_PreviewOwnerActive",
            "m_bPreviewOwnerClaimRequested",
            "m_bPreviewOwnerActive",
        ):
            self.assertIn(token, self.composition_h)
        for token in (
            '"Viewport preview owner: %s"',
            '"Claim Preview Owner"',
            "m_bPreviewOwnerClaimRequested = true",
            "m_bPreviewOwnerActive",
        ):
            self.assertIn(token, preview)
        self.assertIn("m_bPreviewOwnerClaimRequested = false", consume)

        for token in (
            "Consume_PreviewOwnerClaimRequest()",
            "IsDebugToolVisible(DEBUG_TOOL::COMPOSITION)",
            "m_eDebugInputOwner = DEBUG_TOOL::COMPOSITION",
            "m_eDebugWindowFocusPending = DEBUG_TOOL::COMPOSITION",
            "Set_PreviewOwnerActive(",
        ):
            self.assertIn(token, main_update)
        self.assertLess(
            main_update.index("Consume_PreviewOwnerClaimRequest()"),
            main_update.index("m_pAnimationTool->Update("),
            "MainApp must restore Composition ownership before Animation Tool update",
        )
        self.assertNotIn("m_eDebugInputOwner", self.composition_cpp)

    def test_resources_omits_redundant_selected_semantic_links_without_raw_scan(
        self,
    ) -> None:
        linked = function_body(
            self.composition_cpp,
            "void Client::CActionCompositionWorkbench::Render_SemanticLinkedRows(",
        )
        resources = function_body(
            self.composition_cpp,
            "void Client::CActionCompositionWorkbench::Render_ResourcesWindow(",
        )

        for category in (
            '"Animation Sequence Slots"',
            '"Exact Effect Invocations"',
            '"Pattern Sound Events"',
            '"Camera / Light References"',
            '"World / Light Event References"',
            '"Server Collider Owner"',
        ):
            self.assertIn(category, linked)
        for identity in (
            "Clip.strClipOccurrenceId",
            "Clip.strClipName",
            "Cue.strOccurrenceId",
            "Cue.strEffectAssetId",
            "Cue.strSoundEvent",
            "Cue.ResolvedAssetIds.size()",
            '"UNRESOLVED"',
            "Camera.strCameraInvocationId",
            "Camera.strCameraCueId",
            "Event.strTriggerKind",
            "pStage->strHitShape",
            "pStage->strServerDamageProfileId",
        ):
            self.assertIn(identity, linked)
        for routing in (
            "DETAIL_OWNER::ANIMATION",
            "DETAIL_OWNER::EFFECT",
            "DETAIL_OWNER::SOUND",
            "DETAIL_OWNER::CAMERA",
            "DETAIL_OWNER::WORLD",
            "DETAIL_OWNER::GAMEPLAY_STAGE",
            "m_eDetailOwner = eOwner",
            "m_strSelectedStableId = strStableId",
        ):
            self.assertIn(routing, linked)
        for forbidden in (
            "filesystem",
            "recursive_directory_iterator",
            "directory_iterator",
            "RefreshDebugResourceFiles",
        ):
            self.assertNotIn(forbidden, linked)

        self.assertNotIn("Render_SemanticLinkedRows(", resources)
        self.assertIn('"##CompositionResourceDomainTabs"', resources)

    def test_pattern_details_exposes_core_tuning_and_truthful_create_boundary(
        self,
    ) -> None:
        details = function_body(
            self.composition_cpp,
            "void Client::CActionCompositionWorkbench::Render_Details(",
        )
        gameplay = function_body(
            self.composition_cpp,
            "void Client::CActionCompositionWorkbench::"
            "Render_GameplayStageDetails(",
        )

        for overview in (
            '"New Pattern Authoring Coverage"',
            '"Sequence slots:',
            '"Internal gap:',
            '"Server collider:',
            '"Counter -> Groggy:',
            '"Grab release action creation: unavailable in this revision.',
        ):
            self.assertIn(overview, details)
        for existing_vertical_slice in (
            '"Replace Stage Slots"',
            '"Append to Stage Slots"',
            '"WAIT / GAP##BossPatternAdd"',
            '"Add Server Collider"',
            '"Counter Enabled"',
            '"Counter Hit -> Groggy"',
        ):
            self.assertIn(
                existing_vertical_slice,
                self.composition_cpp + self.composition_blueprint_cpp,
            )

        for release_mode in (
            '"Release Mode"',
            '"HOLD"',
            '"OPPOSITE_KNOCKBACK"',
            '"ARENA_EJECTION"',
            '"Release Velocity (m/s)"',
            '"Release Duration (ms)"',
            '"Release Rotation / Yaw Offset (deg)"',
        ):
            self.assertIn(release_mode, gameplay)
        self.assertIn("Action.strReleaseMode = ReleaseModes", gameplay)
        self.assertIn("Action.fSpeedMps = 10.f", gameplay)
        self.assertIn("Action.iDurationMs = 500u", gameplay)
        self.assertIn("Action.fYawOffsetDegrees = 0.f", gameplay)
        self.assertIn("SetValtanStageDraftWithSoundDependencyAdmission(", gameplay)
        self.assertNotIn(
            "Draft.actions.push_back",
            gameplay,
            "Workbench must not fake release-action creation through the read-only inventory",
        )

    def test_effect_selected_collider_routes_to_server_gameplay_owner(self) -> None:
        details = function_body(
            self.composition_cpp,
            "void Client::CActionCompositionWorkbench::Render_Details(",
        )
        effect_owner = function_body(
            details,
            "if (DETAIL_OWNER::EFFECT == m_eDetailOwner)",
        )
        for token in (
            "bHasServerCollider",
            "bManualColliderAddAdmitted",
            '"Add Manual Audition Server Collider / Hit Schedule"',
            "m_eDetailOwner = DETAIL_OWNER::GAMEPLAY_STAGE",
            "m_strSelectedStableId = pStage->strStageId",
            "no geometry is inferred from the Effect",
        ):
            self.assertIn(token, effect_owner)

    def test_canonical_failure_keeps_shell_data_files_and_diagnostic(self) -> None:
        workbench = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPatternMaster(",
        )
        not_ready = function_body(workbench, "if (!bReady)")
        self.assertIn("m_strValtanPatternMasterStatus", not_ready)
        self.assertNotIn(
            "return;",
            not_ready,
            "canonical admission failure must flow into the persistent shell",
        )
        for token in (
            '"##ValtanWorkbenchMain"',
            '"##ValtanWorkbenchOutliner"',
            '"##ValtanWorkbenchDetail"',
            '"##ValtanWorkbenchSequencer"',
            '"##ValtanWorkbenchDataFiles"',
        ):
            self.assertIn(token, workbench)

        data_files = workbench[workbench.index('"##ValtanWorkbenchDataFiles"') :]
        self.assertIn(
            "m_strValtanPatternMasterStatus",
            data_files,
            "Data Files must retain the exact canonical join diagnostic",
        )

    def test_model_capability_guards_local_preview_and_typed_clip_owners(self) -> None:
        workbench = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPatternMaster(",
        )
        self.assertIn(
            "const bool_t bHasPreviewModel = nullptr != pModel;",
            workbench,
        )

        offline_guard = re.search(
            r"ImGui::BeginDisabled\(\s*!bHasPreviewModel\s*\|\|\s*!bMutationAdmitted\s*\);"
            r"(?P<body>[\s\S]*?)ImGui::EndDisabled\(\);",
            workbench,
        )
        self.assertIsNotNone(offline_guard)
        assert offline_guard is not None
        self.assertIn('ImGui::Button("Pattern Offline")', offline_guard.group("body"))
        self.assertNotIn(
            'ImGui::Button("Complete Play (Server/Arena)")',
            offline_guard.group("body"),
            "Server Complete Play must not depend on a local preview model",
        )
        self.assertIn(
            "Can_MutateValtanView(m_eValtanPatternMasterAdmission)",
            function_body(
                self.animation_cpp,
                "bool_t Client::CAnimation_Tool::Start_ValtanPatternMasterPreview(",
            ),
            "the backend preview adapter must also reject stale-preserved data",
        )

        animation_owner_start = workbench.index(
            "VALTAN_WORKBENCH_DETAIL_OWNER::ANIMATION =="
        )
        animation_owner_end = workbench.index(
            "VALTAN_WORKBENCH_DETAIL_OWNER::EFFECT ==",
            animation_owner_start,
        )
        animation_owner = workbench[animation_owner_start:animation_owner_end]
        self.assertIn("bHasPreviewModel", animation_owner)
        self.assertIn("Render_ValtanAnimationBindingInspector(", animation_owner)

        sound_owner_start = workbench.index(
            "VALTAN_WORKBENCH_DETAIL_OWNER::SOUND =="
        )
        sound_owner_end = workbench.index(
            "VALTAN_WORKBENCH_DETAIL_OWNER::CAMERA ==",
            sound_owner_start,
        )
        sound_owner = workbench[sound_owner_start:sound_owner_end]
        self.assertIn("bHasPreviewModel", sound_owner)
        self.assertIn("Render_ValtanPatternSoundInspector(", sound_owner)

        self.assertIn('ImGui::Button("Complete Play (Server/Arena)")', workbench)
        self.assertIn("Debug_CompletePlaySelected", workbench)
        self.assertIn("Set_ServerArenaPreset", workbench)

    def test_valtan_arena_requests_only_the_exact_admitted_preview(self) -> None:
        render = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render()",
        )
        exact_request = re.search(
            r"ETOUI\(LEVEL::VALTAN_ARENA\)[\s\S]*?"
            r'Select_TargetAsset\(\s*"Valtan"\s*\)',
            render,
        )
        self.assertIsNotNone(
            exact_request,
            "Valtan Arena Workbench must request the admitted Valtan preview by asset ID",
        )
        self.assertNotIn("CAnimationTargetService::Bind_Preview", render)

        select_asset = function_body(
            self.character_preview_cpp,
            "bool_t Client::CCharacterPreviewPanel::Select_Asset(",
        )
        for token in (
            "bValtanArenaBossPreview",
            'asset.pBossArchetypeId } == "BOSS_VALTAN"',
            "CValtanPresentationAssetService::Ensure_Prototypes(",
            "CAnimationTargetService::Bind_Preview(",
            'TEXT("Layer_AnimationPreview")',
            "desc.fCollisionRadius = 0.f",
            "desc.isServerAuthoritative = false",
        ):
            self.assertIn(token, select_asset)

        placement = function_body(
            self.level_cpp,
            "bool_t CLevel_ValtanArena::Try_Get_AuthoringPreviewPlacement(",
        )
        for token in (
            "m_Replication.Get_LocalCharacter()",
            "Try_SampleTargetGround(",
            "STATE::RIGHT",
            "Get_ValtanPresentationState()",
        ):
            self.assertIn(token, placement)
        self.assertNotIn("Set_State(", placement)
        self.assertNotIn("CNetworkManager", placement)

        select_target = function_body(
            self.character_preview_cpp,
            "bool_t Client::CCharacterPreviewPanel::Select_TargetAsset(",
        )
        self.assertIn("Try_Get_AuthoringPreviewPlacement(", select_target)
        self.assertIn("Target=ARENA CLONE", select_target)
        self.assertIn("Server Valtan=UNCHANGED", select_target)

        sequence_preview = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Preview_ValtanCompositionSequence(",
        )
        for token in (
            "Target=ARENA CLONE",
            "Get_CurrentAnimIndex()",
            "Server Valtan=UNCHANGED",
        ):
            self.assertIn(token, sequence_preview)

        preview_state = function_body(
            self.animation_cpp,
            "Client::CAnimation_Tool::Get_ValtanCompositionPreviewState() const",
        )
        for token in (
            "bSourceSequencePlaying",
            "m_iValtanPatternPreviewItem",
            "Get_CurrentAnimIndex()",
            "strSourceSequenceStatus",
        ):
            self.assertIn(token, preview_state)

        model_stage = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Stage_ValtanCompositionPreview(",
        )
        self.assertNotIn("Reload_ValtanPatternMaster", model_stage)
        self.assertNotIn("m_eValtanPatternMasterAdmission", model_stage)
        self.assertIn('Select_TargetAsset("Valtan")', model_stage)

        draft_pattern = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Play_ValtanCompositionDraftPattern(",
        )
        self.assertIn("Reload_ValtanPatternMaster()", draft_pattern)

        sequence_browser = function_body(
            self.composition_cpp,
            "void Client::CActionCompositionWorkbench::Render_SequenceBrowser(",
        )
        preview_button = sequence_browser.index(
            'ImGui::Button("Preview Sequence on Arena Clone")'
        )
        preview_gate = sequence_browser.rfind(
            "ImGui::BeginDisabled(", 0, preview_button
        )
        self.assertNotIn(
            "m_eAdmission",
            sequence_browser[preview_gate:preview_button],
        )

    def test_valtan_arena_auto_preview_is_identity_and_generation_driven(self) -> None:
        render = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render()",
        )
        for token in (
            "CAnimationTargetService::Resolve_AssetName()",
            "CAnimationTargetService::Resolve_Boss()",
            "CAnimationTargetService::Resolve_TargetGeneration()",
            "m_iValtanAutoPreviewAttemptGeneration",
            "m_iValtanAutoPreviewSuccessGeneration",
        ):
            self.assertIn(token, render)
        self.assertRegex(
            render,
            r'"Valtan"\s*==\s*strResolvedAssetName[\s\S]*?'
            r'nullptr\s*!=\s*pResolvedValtanBoss',
        )
        stage_preview = function_body(
            render, "const auto TryStageValtanPreview = [&]()"
        )
        self.assertIn('Select_TargetAsset("Valtan")', stage_preview)
        self.assertLess(
            stage_preview.index('Select_TargetAsset("Valtan")'),
            stage_preview.index("m_iValtanAutoPreviewSuccessGeneration"),
        )
        self.assertNotIn(
            "nullptr == Resolve_Model()",
            render,
            "a scene player model must not suppress the exact Valtan preview request",
        )
        self.assertRegex(
            render,
            r"m_iValtanAutoPreviewAttemptGeneration\s*=\s*"
            r"iResolvedTargetGeneration;[\s\S]*?TryStageValtanPreview\(\);",
        )

    def test_valtan_dirty_workspace_uses_only_exact_preview_model(self) -> None:
        composition_play = function_body(
            self.composition_cpp,
            "bool_t Client::CActionCompositionWorkbench::Play_EffectivePreview(",
        )
        draft_play = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Play_ValtanCompositionDraftPattern(",
        )
        start_preview = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Start_ValtanPatternMasterPreview(",
        )
        local_stage = function_body(
            self.valtan_cpp,
            "bool_t CValtan::Stage_LocalPatternAuthoringPreview(",
        )
        self.assertIn(
            "Play_ValtanCompositionDraftPattern(\n\t\t\tPattern, m_ePreviewPath, status)",
            composition_play,
        )
        self.assertNotIn(
            "Play_ValtanCompositionPattern(",
            composition_play,
            "the effective dirty draft must not fall back to the saved Product view",
        )
        for token in (
            "m_ValtanCompositionDraftPreview = Pattern",
            "m_bValtanCompositionDraftPreviewReady = true",
            "Start_ValtanPatternMasterPreview(\n\t\tpModel, m_ValtanCompositionDraftPreview, ePath)",
        ):
            self.assertIn(token, draft_play)
        for token in (
            "CAnimationTargetService::Resolve_Boss()",
            "PreviewBoss->Get_BodyModel() != pModel",
            "PreviewBoss->Stage_LocalPatternAuthoringPreview(Pattern, Status)",
        ):
            self.assertIn(token, start_preview)
        self.assertIn("m_LocalPreviewClipByActionId", local_stage)
        self.assertIn("m_LocalPreviewHitAreaByActionId", local_stage)
        self.assertIn("m_bLocalPatternAuthoringPreview = true", local_stage)
        self.assertNotIn(
            "m_PatternClipByActionId =",
            local_stage,
            "local authoring preview must not overwrite the admitted Product bindings",
        )

    def test_valtan_auto_preview_latch_resets_on_every_level_instance(self) -> None:
        self.assertIn("void On_LevelChanged();", self.animation_h)
        changed = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::On_LevelChanged()",
        )
        for token in (
            "m_iValtanAutoPreviewAttemptGeneration = 0u;",
            "m_iValtanAutoPreviewSuccessGeneration = 0u;",
            "m_bValtanWorkspaceTabInitialized = false;",
        ):
            self.assertIn(token, changed)
        self.assertIn("m_pAnimationTool->On_LevelChanged();", self.main_cpp)

    def test_valtan_canonical_admission_distinguishes_stale_preserved(self) -> None:
        for token in (
            '#include "ValtanViewAdmission.h"',
            "VALTAN_VIEW_ADMISSION",
            "m_eValtanPatternMasterAdmission",
        ):
            self.assertIn(token, self.animation_h)
        for token in (
            "enum class VALTAN_VIEW_ADMISSION",
            "UNLOADED",
            "ADMITTED",
            "STALE_PRESERVED",
            "REJECTED",
            "Can_DisplayValtanView(",
            "Can_MutateValtanView(",
            "VALTAN_VIEW_ADMISSION::STALE_PRESERVED == eAdmission",
            "VALTAN_VIEW_ADMISSION::ADMITTED == eAdmission",
        ):
            self.assertIn(token, self.valtan_view_admission_h)
        reload_master = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Reload_ValtanPatternMaster()",
        )
        self.assertIn("STALE_PRESERVED", reload_master)
        self.assertIn("REJECTED", reload_master)
        self.assertIn("ADMITTED", reload_master)
        workbench = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPatternMaster(",
        )
        self.assertIn("ValtanPatternMasterAdmissionLabel(", workbench)

    def test_valtan_three_pane_tables_scroll_in_narrow_layouts(self) -> None:
        self.assertIn("WORKBENCH_THREE_PANE_INNER_WIDTH", self.animation_cpp)
        unavailable = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPatternMasterUnavailableShell(",
        )
        ready = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPatternMaster(",
        )
        for body in (unavailable, ready):
            self.assertIn("ImGuiTableFlags_ScrollX", body)
            self.assertIn("WORKBENCH_THREE_PANE_INNER_WIDTH", body)

    def test_valtan_workspace_defaults_to_pattern_then_gates_source_tools(self) -> None:
        tabs = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanWorkspaceTabs(",
        )
        pattern = 'ImGui::BeginTabItem("Pattern Workbench"'
        source = 'ImGui::BeginTabItem("Animation Clips / Sequence Intake"'
        self.assertIn(pattern, tabs)
        self.assertIn(source, tabs)
        self.assertLess(tabs.index(pattern), tabs.index(source))
        self.assertIn("ImGuiTabItemFlags_SetSelected", tabs)
        self.assertIn("Render_ValtanPatternMaster(pModel);", tabs)
        self.assertIn("Render_ValtanAnimationSourceWorkspace(pModel);", tabs)

        preview = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPatternPreview(",
        )
        self.assertNotIn("Render_ValtanPatternMaster(", preview)
        source_body = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanAnimationSourceWorkspace(",
        )
        for token in (
            "Render_Playback(pModel);",
            "Render_ValtanPatternPreview(pModel);",
            "Render_NotifyReference(pModel);",
            "Render_HitAreaWires(pModel);",
            "Render_SkillReference(pModel, true);",
            "Render_AnimationList(pModel);",
            "Render_ValtanCustomChainWindow(pModel);",
        ):
            self.assertIn(token, source_body)
        master = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPatternMaster(",
        )
        self.assertNotIn("Render_SkillReference(", master)

        render = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render()",
        )
        collapsed = function_body(render, "if (bValtanWorkspaceMode)")
        self.assertIn('ImGui::CollapsingHeader("Target / Model View")', collapsed)

    def test_selected_pattern_resources_are_semantic_and_focus_typed_detail(self) -> None:
        resources = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanSelectedResourceUsage(",
        )
        ordered_slots = (
            "Gameplay Source / Product",
            "Collider / Hit",
            "Motion / Flow",
            "Animation Chain",
            "Animation Clips",
            "Effect Cues / Assets",
            "Sound Cues / Assets",
            "Camera / World",
        )
        positions = []
        for slot in ordered_slots:
            self.assertIn(slot, resources)
            positions.append(resources.index(slot))
        self.assertEqual(positions, sorted(positions))
        for token in (
            "Pattern.strPatternId",
            "pStage->strStageId",
            "pStage->ClipOccurrences",
            "pStage->ProductCues",
            "m_ValtanPatternSoundCues.Cues",
            "m_eValtanWorkbenchDetailOwner = eOwner;",
            "m_bValtanWorkbenchFocusDetailRequested = true;",
            'ImGui::CollapsingHeader("Raw owner index / diagnostics")',
        ):
            self.assertIn(token, resources)

    def test_persistent_detail_reads_projected_animation_product_without_saving_it(
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
            "CValtanPatternAnimationBindingDocument::Load(", loader
        )
        self.assertIn(
            "m_strValtanPatternAnimationBindingBaselineSourceBytes.clear()", loader
        )
        active_inspector = inspector.split("#if 0", 1)[0]
        for token in (
            "AUTHORING OWNER: Data/Valtan/Valtan.presentation.json",
            "READ-ONLY GENERATED PRODUCT:",
            "Sequence rows are read-only until a typed presentation-source adapter stages them",
            "Stage.ClipOccurrences",
            "strClipOccurrenceId",
            "ProductBinding->Clips.size()",
        ):
            self.assertIn(token, active_inspector)
        for forbidden in (
            "CValtanPatternAnimationBindingDocument::Save_Atomic(",
            '"Save Animation Bindings"',
            '"Add Sequence Slot"',
            '"Set Blank / NONE Draft"',
        ):
            self.assertNotIn(forbidden, active_inspector)

        workbench = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanPatternMaster(",
        )
        self.assertIn(
            "Render_ValtanAnimationBindingInspector(", workbench
        )
        self.assertIn("VALTAN_WORKBENCH_DETAIL_OWNER::ANIMATION", workbench)
        self.assertIn("m_bValtanPatternAnimationBindingDirty", workbench)
        resources = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanSelectedResourceUsage(",
        )
        self.assertIn(
            "Data/Animation/Authored/Valtan/Valtan.patternbindings.json",
            resources,
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
            "Save_ValtanCompositionPatternSounds(SaveStatus)",
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
        self.assertNotIn(
            "CValtanPatternSoundCueDocument::Save_Atomic(",
            sound,
            "the inspector must delegate commit to the typed Sound owner API",
        )
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

        save_sound = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Save_ValtanCompositionPatternSounds(",
        )
        for token in (
            "m_bValtanPatternSoundCuesReady",
            "m_bValtanPatternSoundCuesDirty",
            "CollectModelClipSourceDurationSeconds(Resolve_Model())",
            "CValtanPatternSoundCueDocument::Save_Atomic(",
            "m_strValtanPatternSoundCueBaselineSourceBytes",
            "Reload_ValtanPatternSoundCues()",
            "Pattern Sound saved and loaded.",
        ):
            self.assertIn(token, save_sound)
        self.assertNotIn(
            "Apply_ValtanCompositionPatternSoundsToActiveConsumers(",
            save_sound,
            "source Save must not race the exact-revision runtime admission gate",
        )
        self.assertRegex(
            save_sound,
            r"CValtanPatternSoundCueDocument::Save_Atomic\(\s*"
            r"m_ValtanPatternSoundCues\s*,\s*Durations\s*,\s*"
            r"m_strValtanPatternSoundCueBaselineSourceBytes",
        )
        apply_sound = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Apply_ValtanCompositionPatternSoundsToActiveConsumers(",
        )
        self.assertIn(
            "Reload_PrimaryValtanPresentationAuthoring(",
            apply_sound,
        )
        self.assertIn("ExpectedRevision, ArenaStatus", apply_sound)
        self.assertIn(
            "Boss->Reload_PatternPresentationAuthoring(PreviewStatus)",
            apply_sound,
        )

        composition_gameplay = function_body(
            self.composition_cpp,
            "void Client::CActionCompositionWorkbench::Render_GameplayStageDetails(",
        )
        for token in (
            "Get_ValtanCounterWindowDraft(",
            "Set_ValtanCounterWindowDraft(",
            "Counter.successStageId",
            "Counter.successActionId",
        ):
            self.assertIn(token, composition_gameplay)
        for token in (
            "Patch_ValtanCompositionPatternSound(",
            "Prepare_ValtanCompositionPatternSoundSave(",
            "Save_ValtanCompositionProduct(",
            "Accept_ValtanCompositionPatternSoundSave(",
            "Resolve_ValtanCompositionPatternSoundWindow(",
        ):
            self.assertIn(token, self.composition_cpp)
        composition_save = function_body(
            self.composition_cpp,
            "bool_t Client::CActionCompositionWorkbench::Save_Reload()",
        )
        self.assertNotIn(
            "Save_ValtanCompositionPatternSounds(", composition_save
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
            "bool_t CValtan::Reload_PatternSoundCues_WhileAdmitted(",
        )
        self.assertIn("m_PatternSoundCuesByActionId = std::move(Staged)", runtime_reload)
        public_reload = function_body(
            self.valtan_cpp,
            "bool_t CValtan::Reload_PatternSoundCues(",
        )
        self.assertIn(
            "Reload_PatternPresentationAuthoring(strOutStatus)", public_reload
        )

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

    def test_valtan_animation_binding_product_save_is_fail_closed(self) -> None:
        self.assertIn("read-only generated Product", self.animation_binding_h)
        save = function_body(
            self.animation_binding_cpp,
            "bool_t Client::CValtanPatternAnimationBindingDocument::Save_Atomic(",
        )
        active_save = save.split("#if 0", 1)[0]
        for token in (
            "outCommittedSourceBytes.clear()",
            "read-only generated Product",
            "Data/Valtan/Valtan.presentation.json",
            "return false;",
        ):
            self.assertIn(token, active_save)
        for forbidden in (
            "Read_BinaryText(",
            "Make_BossPatternTemporaryPath(",
            "Commit_BossPatternTemporary(",
            "outCommittedSourceBytes = serialized",
        ):
            self.assertNotIn(forbidden, active_save)
        for token in (
            "!CValtanPatternAnimationBindingDocument::Save_Atomic(",
            "ReadText(source) == sourceText",
            "rejectedCommittedBytes.empty()",
            'status.find("read-only generated Product")',
        ):
            self.assertIn(token, self.animation_binding_harness)

    def test_generated_animation_product_reload_is_transactional_and_inspector_is_read_only(
        self,
    ) -> None:
        self.assertIn("Reload_PatternBindings", self.valtan_h)
        runtime_reload = function_body(
            self.valtan_cpp,
            "bool_t CValtan::Reload_PatternBindings_WhileAdmitted(",
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
            "AUTHORING OWNER: Data/Valtan/Valtan.presentation.json",
            "READ-ONLY GENERATED PRODUCT: Data/Animation/Authored/Valtan/Valtan.patternbindings.json",
            "Reload Read-only Animation Product",
            "Sequence rows are read-only until a typed presentation-source adapter stages them",
        ):
            self.assertIn(token, inspector)
        for forbidden in (
            "CValtanPatternAnimationBindingDocument::Save_Atomic(",
            "Reload_PrimaryValtanPresentationAuthoring(",
            "Reload_PatternPresentationAuthoring(",
            "bReloadJoinedWorkbenchAfterSave = true",
        ):
            self.assertNotIn(forbidden, inspector)

        joined_reload = function_body(
            self.valtan_cpp,
            "bool_t CValtan::Reload_PatternPresentationAuthoring_Impl(",
        )
        for token in (
            "PreviousBindings",
            "PreviousEffectCues",
            "PreviousSoundCues",
            "PreviousCombatObjectSoundCues",
            "PreviousShakeCues",
            "CValtanCanonicalProductReadAdmission CanonicalAdmission",
            "CValtanPresentationGenerationReadAdmission GenerationAdmission",
            "GenerationAdmission.Acquire_Receipt(",
            "VALTAN_CANONICAL_READ_DIAGNOSTIC CanonicalDiagnostic",
            "CanonicalAdmission.Acquire(CanonicalDiagnostic)",
            "Reload_PatternBindings_WhileAdmitted(StepStatus)",
            "Reload_PatternEffectCues_WhileAdmitted(StepStatus)",
            "Reload_PatternSoundCues_WhileAdmitted(StepStatus)",
            "Reload_CombatObjectSoundCues_WhileAdmitted(StepStatus)",
            "Reload_PatternShakeCues_WhileAdmitted(StepStatus)",
            "GenerationAdmission.Validate_StillCurrent(StepStatus)",
            "CanonicalAdmission.Validate_StillCurrent(StepStatus)",
            "StagedBindings",
            "StagedCombatObjectSoundCues",
            "RestorePrevious()",
            "m_PatternClipByActionId = PreviousBindings",
            "m_PatternEffectCuesByActionId = PreviousEffectCues",
            "m_PatternSoundCuesByActionId = PreviousSoundCues",
            "m_CombatObjectSoundCuesBySource = PreviousCombatObjectSoundCues",
            "m_PatternShakeCuesByActionId = PreviousShakeCues",
            "CurrentPresentationReceipt",
            "std::move(CurrentPresentationReceipt)",
        ):
            self.assertIn(token, joined_reload)
        self.assertNotIn(
            "GenerationAdmission.Acquire_ExactReceipt(", joined_reload
        )

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
        self.assertIn(
            "Try_Get_ValtanPresentationGenerationReceipt(", combat_reload
        )
        self.assertIn("ExpectedRevision, PresentationReceipt", combat_reload)
        self.assertIn("Reload_PatternPresentationAuthoring(", combat_reload)
        self.assertNotIn("Reload_CombatObjectSoundCues(", combat_reload)
        self.assertIn(
            "m_PrimaryValtanCombatObjectSoundFreshness.Reject(", combat_reload
        )
        self.assertIn(
            "m_PrimaryValtanJoinedPresentationFreshness.Admit(", combat_reload
        )
        self.assertIn(
            "m_PrimaryValtanCombatObjectSoundFreshness.Admit(", combat_reload
        )

        playback_admission = function_body(
            self.boss_cpp,
            "bool_t Client::CBossTool::Acquire_ServerPlaybackAdmission(",
        )
        self.assertIn(
            "Can_Play_PrimaryValtanPresentation(", playback_admission
        )
        self.assertIn(
            "Get_PrimaryValtanPatternSoundSourceReceipt(", playback_admission
        )
        self.assertIn("SoundAdmission.Acquire(", playback_admission)

        animation_inspector = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Render_ValtanAnimationBindingInspector(",
        )
        sound_inspector = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Render_ValtanPatternSoundInspector(",
        )
        sound_save = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Save_ValtanCompositionPatternSounds(",
        )
        canonical_save = function_body(
            self.composition_cpp,
            "bool_t Client::CActionCompositionWorkbench::Save_Reload()",
        )
        toolbar = function_body(
            self.composition_cpp,
            "bool_t Client::CActionCompositionWorkbench::Render_Toolbar(",
        )
        self.assertIn(
            "READ-ONLY GENERATED PRODUCT: Data/Animation/Authored/Valtan/Valtan.patternbindings.json",
            animation_inspector,
        )
        self.assertNotIn(
            "Reload_PrimaryValtanPresentationAuthoring(", animation_inspector
        )
        self.assertIn("Save_ValtanCompositionPatternSounds(SaveStatus)", sound_inspector)
        self.assertNotIn(
            "Reload_PrimaryValtanPresentationAuthoring(",
            sound_inspector,
            "the inspector delegates runtime freshness to the typed Save owner",
        )
        self.assertNotIn(
            "Apply_ValtanCompositionPatternSoundsToActiveConsumers(",
            sound_save,
        )
        sound_apply = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Apply_ValtanCompositionPatternSoundsToActiveConsumers(",
        )
        self.assertIn("Reload_PrimaryValtanPresentationAuthoring(", sound_apply)
        sound_retry = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Retry_ValtanCompositionPatternSoundRuntimeApply(",
        )
        self.assertIn(
            "Apply_ValtanCompositionPatternSoundsToActiveConsumers(",
            sound_retry,
        )
        transaction_edges = (
            "Prepare_ValtanCompositionPatternSoundSave(",
            "Prepare_BossValtanBindingDraftSave(",
            "Save_ValtanCompositionProduct(",
            "Accept_ValtanCompositionPatternSoundSave(",
            "Accept_BossValtanBindingDraftSave(",
            "Reload_CanonicalGraph(ToolReloadStatus)",
            "Reload_Canonical()",
        )
        for token in transaction_edges:
            self.assertIn(token, canonical_save)
        transaction_positions = [
            canonical_save.index(token) for token in transaction_edges
        ]
        self.assertEqual(transaction_positions, sorted(transaction_positions))
        self.assertEqual(
            1, canonical_save.count("Save_ValtanCompositionProduct(")
        )
        self.assertNotIn("Save_ValtanProduct(", canonical_save)
        self.assertNotIn(
            "Save_ValtanCompositionPatternSounds(", canonical_save
        )
        for token in (
            "patternSoundBaselineBytes",
            "patternSoundCandidateBytes",
            "effectV2BaselineBytes",
            "effectV2CandidateBytes",
            'm_strStatus = "Nothing was saved. "',
        ):
            self.assertIn(token, canonical_save)
        self.assertIn("Observe_ServerActivePatternRevision(", toolbar)
        self.assertNotIn("Get_ServerActivePatternRevision(", toolbar)
        self.assertIn("ExpectedServerRevision", toolbar)
        self.assertIn("VerifyAuthoritativeFreshnessGate", self.animation_binding_harness)

    def test_boss_pattern_start_entries_use_typed_services_and_saved_flow_reset(
        self,
    ) -> None:
        def require_gate_before(body: str, submission: str) -> None:
            gate = body.index("Acquire_ServerPlaybackAdmission(")
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
        first_slot = flow_start.index("pFlow->Slots.front().strSlotId")
        delegate = flow_start.index(
            "Start_FlowAtSlot(StartSlotId, pRequiredDefinitionRevision)"
        )
        self.assertLess(first_slot, delegate)
        flow_submit = function_body(
            self.boss_cpp,
            "bool_t Client::CBossTool::Start_FlowAtSlot(",
        )
        self.assertIn("CValtanPatternFlowService::Get().Start(", flow_submit)
        restart_saved = self.boss_cpp[
            self.boss_cpp.index(
                "bool_t Client::CBossTool::Restart_SavedFlow()"
            ) :
            self.boss_cpp.index(
                "bool_t Client::CBossTool::Request_RevivePlayer("
            )
        ]
        self.assertLess(
            restart_saved.index("Reload_FlowDocument()"),
            restart_saved.index("return Start_Flow("),
        )
        self.assertIn(
            "bHasSavedGameplayExpectation ? &SavedCandidateRevision : nullptr",
            restart_saved,
        )

        server_start = function_body(
            self.room_cpp,
            "LostArk::Server::CGameRoom::Evaluate_ValtanPatternFlowStart(",
        )
        reset = server_start.index(
            "Reset_ValtanAuditionState(*boss, resetTick, resetStatus)"
        )
        commit = server_start.index(
            "m_ValtanPatternFlowAudition = std::move(stagedFlow)"
        )
        lifecycle = server_start.index("Queue_ValtanPatternFlowLifecycle(")
        self.assertLess(reset, commit)
        self.assertLess(commit, lifecycle)

        next_picker = function_body(
            self.boss_cpp,
            "void Client::CBossTool::Render_NextPatternPicker()",
        )
        self.assertIn("Queue_NextServerPattern(", next_picker)
        self.assertNotIn("Service.Queue_NextPattern(", next_picker)

        next_card = function_body(
            self.boss_cpp,
            "void Client::CBossTool::Render_NextPatternCard()",
        )
        next_gate = next_card.index("Acquire_ServerPlaybackAdmission(")
        self.assertLess(next_card.index("Service.Clear_NextPattern("), next_gate)
        self.assertLess(next_gate, next_card.index("Service.Retry_NextPatternCommand("))

        flow_detail = function_body(
            self.boss_cpp,
            "void Client::CBossTool::Render_FlowSelectedSlot()",
        )
        self.assertIn("Stop_AfterCurrent(", flow_detail)
        self.assertNotIn("Acquire_ServerPlaybackAdmission(", flow_detail)
        self.assertNotIn("FlowService.Retry_Start(", flow_detail)
        self.assertIn("FlowService.Retry_Start(", restart_saved)

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
            "m_PrimaryValtanJoinedPresentationFreshness.Reject(", reset
        )
        self.assertIn(
            "m_PrimaryValtanCombatObjectSoundFreshness.Reject(", reset
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
        self.assertIn(
            "Render_ValtanSelectedResourceUsage(*pSelected, pSelectedStage);",
            workbench,
        )
        self.assertNotIn('"Joined Tracks", "READ-ONLY COMPOSITE', workbench)
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
            "NETWORK_PROTOCOL_VERSION = 53;",
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
        self.assertEqual(
            1,
            len(re.findall(
                r'ImGui::Button\(\s*"Save & Apply##ValtanPatternMaster"',
                workbench,
            )),
        )
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
        self.assertIn("m_bValtanCombatObjectSoundCuesDirty", workbench)
        self.assertIn("legacy direct source replacement was retired", workbench)
        for unsafe_route in (
            "Begin_SourceReplacement",
            "Commit_SourceReplacement",
            "Rollback_SourceReplacement",
        ):
            self.assertNotIn(unsafe_route, workbench)
        sound_block = workbench.index(
            "if (m_bValtanCombatObjectSoundCuesDirty)"
        )
        product_save = workbench.index("m_pBalanceTool->Save_ValtanProduct")
        self.assertLess(sound_block, product_save)

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
        canonical = save.index("Save_ValtanCanonicalProduct")
        product = save.index("Publish_ValtanCandidate")
        self.assertLess(validation, product)
        self.assertLess(canonical, product)
        self.assertNotIn("Save_ValtanAuthoring", save)
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

    def test_legacy_combat_sound_direct_save_is_hard_rejected(self) -> None:
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
        self.assertIn("Direct combat-object Sound source replacement is retired", begin)
        self.assertLess(begin.index("return false;"), begin.index("#if 0"))

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
            "stable identity, DamageProfile, response, and motion kind are read-only",
            "MANUAL_SERVER_AUDITION Stage admits ACTIVE, WINDUP, or GROGGY",
            "stageKindChanged",
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
            '"arena.center.target-follow"',
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
        self.assertNotIn(
            "IsDebugToolVisible(DEBUG_TOOL::EFFECT_V2)", render
        )
        self.assertIn("m_pEffectToolV2->Render()", render)
        self.assertIn("m_pBossTool->Render()", render)
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

        toolbar = function_body(
            self.composition_cpp,
            "bool_t Client::CActionCompositionWorkbench::Render_Toolbar(",
        )
        complete_play = function_body(
            toolbar, 'if (ImGui::Button("Play on Server"'
        )
        restart = function_body(
            toolbar, 'if (ImGui::Button("Restart"'
        )
        fresh_arena = function_body(
            toolbar, 'if (ImGui::Button("Restore Arena"'
        )
        self.assertIn("Debug_CompletePlaySelected(Status)", complete_play)
        self.assertIn("Restart_ServerPattern", restart)
        for preserved_command in (complete_play, restart):
            self.assertNotIn("Set_ServerArenaPreset", preserved_command)
            self.assertNotIn("VALTAN_ARENA_PRESET::FRESH", preserved_command)
        self.assertIn("Set_ServerArenaPreset", fresh_arena)
        self.assertIn("VALTAN_ARENA_PRESET::FRESH", fresh_arena)

        reset_contract = self.room_cpp.index(
            "Both supported worlds use this boss-only reset"
        )
        pattern_start = self.room_cpp.rfind(
            "if (isPatternIdCommand)", 0, reset_contract
        )
        pattern_end = self.room_cpp.index(
            "SET_ARENA_PRESET is the only consumer", reset_contract
        )
        self.assertGreaterEqual(pattern_start, 0)
        pattern_play = self.room_cpp[pattern_start:pattern_end]
        self.assertIn("Build_ValtanBossOnlyAuditionReset", pattern_play)
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

    def test_f1_resources_are_semantic_owner_entry_points_only(self) -> None:
        semantic_refresh = function_body(
            self.main_cpp,
            "void CMainApp::RefreshDebugAuthoringSources()",
        )
        semantic_open = function_body(
            self.main_cpp,
            "void CMainApp::OpenDebugAuthoringSource(",
        )
        raw_refresh = function_body(
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
            '"Valtan Action Composition"',
            '"Data/Valtan/Valtan.gameplay.json"',
            '"Data/Valtan/Valtan.presentation.json"',
            '"Valtan Effect Resources"',
            '"Data/Effects"',
            '"Valtan Pattern Sound Cues"',
            '"Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json"',
            '"Valtan Encounter Runtime"',
            "DEBUG_TOOL::COMPOSITION",
        ):
            self.assertIn(token, semantic_refresh)
        self.assertIn("EnsureDebugTool(source.eTool)", semantic_open)
        self.assertIn("EnsureDebugTool(DEBUG_TOOL::COMPOSITION)", semantic_open)
        self.assertIn("m_pActionCompositionWorkbench->Open_Valtan()", semantic_open)

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
            self.assertIn(token, raw_refresh)
        for forbidden in (
            "copy_file",
            "copy(",
            "rename(",
            "remove(",
            "ofstream",
        ):
            self.assertNotIn(
                forbidden,
                semantic_refresh + semantic_open + raw_refresh + open_file,
            )
        self.assertIn('ImGui::SeparatorText("Authoring Sources")', render)
        self.assertIn("F1 does not build a raw physical-file index", render)
        self.assertIn("load inside the selected Tool, never in F1", render)
        self.assertNotIn("recursive_directory_iterator", semantic_refresh)
        for forbidden in (
            "Diagnostics / Raw File Index",
            "Build Raw Index",
            "RefreshDebugResourceFiles()",
            "OpenDebugResourceFile(iFile)",
            "m_DebugResourceFiles",
        ):
            self.assertNotIn(forbidden, render)
        self.assertIn("EnsureDebugTool(file.eTool)", open_file)

    def test_composition_first_open_admits_boss_server_inventory(self) -> None:
        ensure = function_body(
            self.main_cpp,
            "HRESULT CMainApp::EnsureDebugTool(const DEBUG_TOOL eTool)",
        )
        composition = ensure[
            ensure.index("case DEBUG_TOOL::COMPOSITION:") :
            ensure.index("case DEBUG_TOOL::ANIMATION:")
        ]
        self.assertIn("m_pBossTool->Reload_CanonicalGraph", composition)
        self.assertLess(
            composition.index("m_pBossTool->Reload_CanonicalGraph"),
            composition.index("m_pActionCompositionWorkbench->Open_Valtan"),
        )

    def test_native_master_parser_accepts_canonical_sequence_zero(self) -> None:
        parser = function_body(
            self.valtan_tree_cpp,
            "bool_t Parse_MasterPattern(",
        )
        self.assertNotIn("0.0 == pSourceSequence->Get_Number()", parser)
        self.assertNotIn(
            '0.0 == Source.Find("sequenceIndex")->Get_Number()', parser
        )
        self.assertGreaterEqual(parser.count("4096.0 <"), 2)


if __name__ == "__main__":
    unittest.main()
