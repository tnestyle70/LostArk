#!/usr/bin/env python3
"""Focused source contract for the KoukuSaydon Client product level."""

from __future__ import annotations

from pathlib import Path
import re
import unittest
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parents[2]
AREA_ID = "LV_LUT_MIDNIGHTC_ED"
LEVEL = "KAKULSAYDON_ARENA"


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8-sig")


class KoukuSaydonClientProductLevelContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.level_h = read("Client/Public/Level_KakulSaydonArena.h")
        cls.level_cpp = read("Client/Private/Level_KakulSaydonArena.cpp")
        cls.main_h = read("Client/Public/MainApp.h")
        cls.main_cpp = read("Client/Private/MainApp.cpp")
        cls.loader_h = read("Client/Public/Loader.h")
        cls.loader_cpp = read("Client/Private/Loader.cpp")
        cls.registry = read("Client/Private/LevelRegistry.cpp")
        cls.transition = read("Client/Private/LevelTransitionService.cpp")
        cls.character_select = read("Client/Private/Level_CharacterSelect.cpp")
        cls.character_select_h = read("Client/Public/Level_CharacterSelect.h")
        cls.workbench_h = read("Client/Public/KoukuSaydonActionWorkbench.h")
        cls.workbench_cpp = read("Client/Private/KoukuSaydonActionWorkbench.cpp")
        cls.composition_cpp = read(
            "Client/Private/KoukuSaydonCompositionDocument.cpp"
        )
        cls.boss_tool_cpp = read("Client/Private/KoukuSaydonBossTool.cpp")
        cls.character_preview_cpp = read(
            "Client/Private/CharacterPreviewPanel.cpp"
        )

    def test_registry_owns_exact_product_identity(self) -> None:
        descriptor = re.search(
            rf"LEVEL::{LEVEL},\s*CLIENT_LEVEL_KIND::PRODUCT,\s*"
            rf'"raid\.kakul-saydon\.arena",\s*"{AREA_ID}",\s*'
            r'"scene\.development\.neutral\.v1",\s*MakeFullMapScope\(\),\s*'
            r"CreateKakulSaydonArena,\s*&CLoader::Ready_For_KakulSaydonArena",
            self.registry,
        )
        self.assertIsNotNone(descriptor)
        self.assertIn('#include "Level_KakulSaydonArena.h"', self.registry)
        self.assertIn("CLevel_KakulSaydonArena::Create", self.registry)

    def test_loader_stages_map_player_and_exact_server_boss_presentation(self) -> None:
        self.assertIn("HRESULT Ready_For_KakulSaydonArena();", self.loader_h)
        start = self.loader_cpp.index("HRESULT CLoader::Ready_For_KakulSaydonArena()")
        end = self.loader_cpp.index("HRESULT CLoader::Ready_For_Development()", start)
        body = self.loader_cpp[start:end]
        self.assertIn("Ready_MapArea", body)
        self.assertIn("Ready_Character_Rendering", body)
        self.assertIn(f"LEVEL::{LEVEL}", body)
        self.assertIn("CNpcPresentationAssetService::Begin_LevelLoad", body)
        self.assertIn("CNpcPlacementPresentationService::Begin_LevelLoad", body)
        self.assertIn("CMonsterPresentationAssetService::Begin_LevelLoad", body)
        self.assertEqual(
            1,
            body.count(
                "CKoukuSaydonPresentationAssetService::Ensure_Prototypes"
            ),
        )
        self.assertIn('"BOSS_KAKULSAYDON_G1_KOUKU"', body)
        for forbidden in (
            "Ready_ValtanPresentation",
            "Ready_DeployPropArea",
            "SpawnWorldEntity",
            "Play_Pattern",
        ):
            self.assertNotIn(forbidden, body)

    def test_level_consumes_server_replication_and_never_local_spawns(self) -> None:
        self.assertIn("CMapPlacementRuntime m_MapRuntime", self.level_h)
        self.assertIn("CClientReplication m_Replication", self.level_h)
        self.assertIn("CPlayerController m_PlayerController", self.level_h)
        self.assertIn("Get_Active()", self.level_h)
        self.assertIn("s_pActiveInstance", self.level_cpp)
        self.assertIn(AREA_ID, self.level_cpp)
        self.assertIn("m_MapRuntime.Load_Area", self.level_cpp)
        self.assertIn("m_Replication.Initialize", self.level_cpp)
        self.assertIn("m_Replication.Update", self.level_cpp)
        self.assertIn("CNetworkPlayerCommandSink", self.level_cpp)
        self.assertIn("Pump_ServerApprovedWorldTransfer", self.level_cpp)
        self.assertIn("Bind_CameraToLocalCharacter", self.level_cpp)
        self.assertEqual(1, self.level_cpp.count("Add_GameObject_to_Layer"))
        self.assertIn('TEXT("Prototype_GameObject_Camera_Free")', self.level_cpp)
        for forbidden in (
            "Prototype_GameObject_Character",
            "Prototype_GameObject_Valtan",
            "CValtan",
            "ValtanBossTool",
            "Play_Pattern",
        ):
            self.assertNotIn(forbidden, self.level_cpp)

    def test_stage_teleport_is_typed_and_fail_closed_without_markers(self) -> None:
        self.assertIn("shared_ptr<IWorldEntityCommandSink>", self.level_h)
        self.assertIn("Request_StageTeleport", self.level_h)
        start = self.level_cpp.index(
            "bool_t Client::CLevel_KakulSaydonArena::Request_StageTeleport"
        )
        end = self.level_cpp.index(
            "HRESULT Client::CLevel_KakulSaydonArena::Ready_Layer_Camera", start
        )
        body = self.level_cpp[start:end]
        empty_guard = body.index("m_StageMarkerPlacementIds.empty()")
        sink_call = body.index("m_pWorldEntityCommandSink->Request_StageTeleport")
        self.assertLess(empty_guard, sink_call)
        self.assertIn("m_StageMarkerPlacementIds.contains", body)
        self.assertNotIn("Set_State", body)
        self.assertNotIn("Set_Position", body)

    def test_f1_arena_tabs_submit_runtime_stage_marker_identity(self) -> None:
        self.assertIn('#include "Level_KakulSaydonArena.h"', self.main_cpp)
        start = self.main_cpp.index(
            "void CMainApp::RenderServerArenaActiveControls()"
        )
        end = self.main_cpp.index("void CMainApp::RenderDeveloperTools()", start)
        body = self.main_cpp[start:end]

        for token in (
            'ImGui::BeginTabBar("##ServerArenaActiveTabs")',
            'ImGui::BeginTabItem("Valtan")',
            'ImGui::BeginTabItem("KoukuSaydon")',
            "VALTAN_ARENA_PRESET::FRESH",
            "VALTAN_ARENA_PRESET::CIRCLE_WALLS_GONE",
            "VALTAN_ARENA_PRESET::THREE_OCLOCK_BROKEN",
            "VALTAN_ARENA_PRESET::NINE_OCLOCK_BROKEN",
            "VALTAN_ARENA_PRESET::BOTH_SIDES_BROKEN",
            "LEVEL::KAKULSAYDON_ARENA",
            "CLevel_KakulSaydonArena::Get_Active()",
            "pKakulArena->Get_StageMarkers()",
            "marker.strPlacementId",
            "pKakulArena->Request_StageTeleport",
            "m_strKakulStageTeleportStatus",
        ):
            self.assertIn(token, body)

        self.assertIn(
            "uint32_t m_iNextKakulStageTeleportRequestSequence = 1u;",
            self.main_h,
        )
        capture = body.index(
            "const std::uint32_t requestSequence ="
        )
        advance = body.index(
            "++m_iNextKakulStageTeleportRequestSequence", capture
        )
        submit = body.index("pKakulArena->Request_StageTeleport", capture)
        self.assertLess(capture, advance)
        self.assertLess(advance, submit)
        self.assertIn(
            "0u == m_iNextKakulStageTeleportRequestSequence", body
        )
        self.assertIn(
            "(std::numeric_limits<std::uint32_t>::max)()", body
        )

        for forbidden in (
            "stage.kakul.sl01",
            "stage.kakul.sl02",
            "stage.kakul.sl03",
            "stage.kakul.sl04",
            "stage.kakul.sl05",
            "Set_State",
            "Set_Position",
        ):
            self.assertNotIn(forbidden, body)

    def test_server_world_mapping_and_project_registration_are_exact(self) -> None:
        self.assertRegex(
            self.transition,
            rf"case WORLD_ID::{LEVEL}:\s*targetLevel = LEVEL::{LEVEL};",
        )
        project_path = ROOT / "Client/Default/Client.vcxproj"
        filters_path = ROOT / "Client/Default/Client.vcxproj.filters"
        ET.parse(project_path)
        ET.parse(filters_path)
        project = project_path.read_text(encoding="utf-8-sig")
        filters = filters_path.read_text(encoding="utf-8-sig")
        for name in ("Level_KakulSaydonArena.h", "Level_KakulSaydonArena.cpp"):
            self.assertEqual(1, project.count(name))
            self.assertEqual(1, filters.count(name))

    def test_character_select_consumes_transfer_before_replication(self) -> None:
        start = self.character_select.index(
            "void CLevel_CharacterSelect::Update_ServerArena()"
        )
        end = self.character_select.index(
            "void CLevel_CharacterSelect::Fail_ServerArena", start
        )
        body = self.character_select[start:end]
        transfer = body.index("Pump_ServerApprovedWorldTransfer")
        replication = body.index("m_Replication.Update")
        self.assertLess(transfer, replication)
        self.assertIn("LEVEL::CHARACTER_SELECT", body[:replication])

    def test_character_select_hands_live_socket_to_approved_target(self) -> None:
        self.assertIn(
            "bool_t m_preserveServerConnectionForTransfer = false;",
            self.character_select_h,
        )
        update_start = self.character_select.index(
            "void CLevel_CharacterSelect::Update_ServerArena()"
        )
        update_end = self.character_select.index(
            "void CLevel_CharacterSelect::Fail_ServerArena", update_start
        )
        update = self.character_select[update_start:update_end]
        self.assertRegex(
            update,
            r"REQUESTED\s*==\s*transferResult[\s\S]*"
            r"m_preserveServerConnectionForTransfer\s*=\s*true",
        )

        destructor_start = self.character_select.index(
            "CLevel_CharacterSelect::~CLevel_CharacterSelect()"
        )
        destructor_end = self.character_select.index(
            "HRESULT CLevel_CharacterSelect::Initialize()", destructor_start
        )
        destructor = self.character_select[destructor_start:destructor_end]
        self.assertRegex(
            destructor,
            r"if\s*\(!m_preserveServerConnectionForTransfer\)\s*"
            r"CNetworkManager::Get\(\)\.Close_ServerConnection\(\)",
        )

        return_start = self.character_select.index(
            "void CLevel_CharacterSelect::Return_ServerArenaToLobby"
        )
        return_end = self.character_select.index(
            "bool_t CLevel_CharacterSelect::Request_SelectedArenaSpawn", return_start
        )
        self.assertIn(
            "CNetworkManager::Get().Close_ServerConnection()",
            self.character_select[return_start:return_end],
        )

    def test_kouku_workbench_and_boss_tool_keep_exact_data_ownership(self) -> None:
        self.assertIn(
            "KoukuSaydon/Gate1/KoukuSaydonComposition.json",
            self.composition_cpp,
        )
        self.assertIn(
            'L"Encounters/KoukuSaydon/KoukuSaydonEncounter.json"',
            self.boss_tool_cpp,
        )
        for source in (self.workbench_h, self.workbench_cpp, self.boss_tool_cpp):
            self.assertNotIn('#include "Valtan', source)
            self.assertNotIn("CValtan", source)
        self.assertNotIn("CNetworkManager", self.workbench_cpp)

    def test_workbench_server_play_is_routed_through_the_kouku_boss_tool(self) -> None:
        for token in (
            'ImGui::Button("Publish All PRODUCT")',
            'ImGui::Button("Play Published Product (Server)")',
            "Consume_ServerPlayRequest",
        ):
            self.assertIn(token, self.workbench_cpp)

        consume = self.main_cpp.index(
            "m_pKoukuSaydonActionWorkbench->Consume_ServerPlayRequest"
        )
        ensure = self.main_cpp.index(
            "EnsureDebugTool(DEBUG_TOOL::KOUKU_SAYDON_BOSS)", consume
        )
        play = self.main_cpp.index(
            "m_pKoukuSaydonBossTool->Play_PatternById", ensure
        )
        self.assertLess(consume, ensure)
        self.assertLess(ensure, play)

    def test_composition_save_lock_is_crash_recoverable(self) -> None:
        lock_start = self.composition_cpp.index(
            "class COMPOSITION_WRITER_LOCK final"
        )
        lock_end = self.composition_cpp.index(
            "Client::CKoukuSaydonCompositionDocument::",
            lock_start,
        )
        lock = self.composition_cpp[lock_start:lock_end]
        destructor_start = lock.index("~COMPOSITION_WRITER_LOCK()")
        acquire_start = lock.index("bool_t Acquire", destructor_start)
        self.assertNotIn("DeleteFileW", lock[destructor_start:acquire_start])
        self.assertIn("FILE_FLAG_DELETE_ON_CLOSE", lock)
        self.assertIn("0u, nullptr, CREATE_NEW", lock)
        self.assertIn("DeleteFileW(m_Path.c_str())", lock)
        # G04: an empty or partly invalid composition still opens; the
        # projector, not the editor, rejects a PRODUCT set with no pattern.
        self.assertNotIn("document.Patterns.empty()", self.composition_cpp)

    def test_detail_combo_mutations_are_deferred_and_add_stage_stays_reachable(
        self,
    ) -> None:
        details_start = self.workbench_cpp.index(
            "void Client::CKoukuSaydonActionWorkbench::Render_Details()"
        )
        details_end = self.workbench_cpp.index(
            "void Client::CKoukuSaydonActionWorkbench::Render_ReloadConfirmation()",
            details_start,
        )
        details = self.workbench_cpp[details_start:details_end]
        for deferred_selection in (
            "requestedCategory",
            "requestedAuthoringStatus",
            "requestedStageKind",
            "requestedTargetStageId",
        ):
            self.assertIn(deferred_selection, details)
        add_stage = details.index('ImGui::Button("Add Stage")')
        selected_stage_guard = details.index("if (nullptr == stage)")
        self.assertLess(add_stage, selected_stage_guard)

    def test_raid_arenas_use_registered_local_bodies_and_replication_placement(self) -> None:
        select = _region(
            self.character_preview_cpp,
            "bool_t Client::CCharacterPreviewPanel::Select_Asset(",
            "void Client::CCharacterPreviewPanel::Render_Selector(",
        )
        for token in (
            "bRaidCompositionPreview",
            "LEVEL::KAKULSAYDON_ARENA",
            "LEVEL::VALTAN_ARENA",
            "Is_CompositionAnimationTargetAsset(asset.pAssetName)",
            "Try_ResolveRaidPreviewPlacement(",
            'TEXT("Layer_AnimationPreview")',
            "Server boss=UNCHANGED",
        ):
            self.assertIn(token, select)
        target_contract = read("Client/Public/CompositionAnimationResource.h")
        for asset in ("Valtan", "Valtan_Ghost_MN_RPBF_02", "MN_RPCT_00",
                      "MN_RPCT_05", "MN_RPCT_06", "MN_RPCZ_00"):
            self.assertIn(f'"{asset}"', target_contract)
        placement = _region(
            self.level_cpp,
            "bool_t Client::CLevel_KakulSaydonArena::Try_Get_AuthoringPreviewPlacement(",
            "bool_t Client::CLevel_KakulSaydonArena::Request_StageTeleport(",
        )
        self.assertIn("m_Replication.Get_LocalCharacter()", placement)
        self.assertIn("Try_SampleTargetGround(", placement)
        self.assertNotIn("Resolve_SceneCharacter", placement)
        self.assertNotIn("Set_State(", placement)
        self.assertIn(
            'constexpr const char_t* BOSS_BODY_PROFILE_ID = "MN_RPCZ_00";',
            self.workbench_cpp,
        )
        self.assertIn("Is_AppendAdmitted(source, outStatus)", self.workbench_cpp)


def _region(source: str, start: str, end: str) -> str:
    start_at = source.index(start)
    end_at = source.index(end, start_at + len(start))
    return source[start_at:end_at]


class KoukuSaydonSharedEditorContractTests(unittest.TestCase):
    """G05: one shared resource browser, typed transport, honest playback."""

    @classmethod
    def setUpClass(cls) -> None:
        cls.workbench_h = read("Client/Public/KoukuSaydonActionWorkbench.h")
        cls.workbench_cpp = read("Client/Private/KoukuSaydonActionWorkbench.cpp")
        cls.main_cpp = read("Client/Private/MainApp.cpp")
        cls.animation_h = read("Client/Public/Animation_Tool.h")
        cls.animation_cpp = read("Client/Private/Animation_Tool.cpp")
        cls.tree_h = read("Client/Public/CompositionResourceTree.h")
        cls.tree_cpp = read("Client/Private/CompositionResourceTree.cpp")
        cls.valtan_h = read("Client/Public/ValtanActionWorkbench.h")
        cls.valtan_cpp = read("Client/Private/ValtanActionWorkbench.cpp")
        cls.action_document_h = read(
            "Client/Public/KoukuSaydonAnimationActionDocument.h"
        )
        cls.action_document_cpp = read(
            "Client/Private/KoukuSaydonAnimationActionDocument.cpp"
        )
        cls.vcxproj = read("Client/Default/Client.vcxproj")
        cls.filters = read("Client/Default/Client.vcxproj.filters")

    def test_resource_tree_is_one_shared_component_over_per_boss_catalogs(self) -> None:
        self.assertIn("struct COMPOSITION_RESOURCE_TREE_NODE final", self.tree_h)
        self.assertNotIn("struct COMPOSITION_RESOURCE_TREE_NODE final", self.valtan_h)
        self.assertIn('#include "CompositionResourceTree.h"', self.valtan_h)
        self.assertIn('#include "CompositionResourceTree.h"', self.workbench_h)
        for token in (
            "void Client::InsertResourceTree(",
            "std::size_t Client::FinalizeResourceTree(",
            "void Client::RenderResourceTree(",
        ):
            self.assertIn(token, self.tree_cpp)
        self.assertNotIn("\tvoid InsertResourceTree(", self.valtan_cpp)
        self.assertNotIn("\tvoid RenderResourceTree(", self.valtan_cpp)
        self.assertIn("RenderResourceTree(", self.valtan_cpp)
        self.assertIn("RenderResourceTree(m_ResourceTree", self.workbench_cpp)
        for registration in (
            '<ClInclude Include="..\\Public\\CompositionResourceTree.h" />',
            '<ClCompile Include="..\\Private\\CompositionResourceTree.cpp" />',
        ):
            self.assertIn(registration, self.vcxproj)
        self.assertIn('Include="..\\Private\\CompositionResourceTree.cpp"', self.filters)
        self.assertIn('Include="..\\Public\\CompositionResourceTree.h"', self.filters)
        # Resources reads physical metadata without creating a model or entering
        # either boss's document admission. Failed files retain their prior rows.
        inventory = _region(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Read_CompositionAnimationResources(",
            "bool_t Client::CAnimation_Tool::Preview_CompositionAnimationResource(",
        )
        self.assertIn("Read_AnimationCatalog(", inventory)
        self.assertIn("previous rows retained", inventory)
        for forbidden in ("Select_TargetAsset", "CModel::Create", "Find_Boss(",
                          "Open_ValtanWorkspace", "Open_KoukuSaydonProfile"):
            self.assertNotIn(forbidden, inventory)
        backend = _region(
            self.main_cpp,
            "HRESULT CMainApp::EnsureAnimationPreviewBackend()",
            "HRESULT CMainApp::EnsureDebugTool(",
        )
        self.assertNotIn("SetDebugToolVisible", backend)
        self.assertNotIn("m_eDebugWindowFocusPending", backend)
        for source in (self.workbench_cpp, self.valtan_cpp):
            self.assertIn("CompositionTimeline::DrawRuler(", source)
            self.assertIn("CompositionTimeline::DrawBox(", source)
            self.assertIn("CompositionTimeline::HitBoxGesture(", source)

    def test_kouku_resources_browse_every_profile_but_bind_only_the_boss_body(self) -> None:
        rebuild = _region(
            self.workbench_cpp,
            "void Client::CKoukuSaydonActionWorkbench::Rebuild_ResourceTree()",
            "bool_t Client::CKoukuSaydonActionWorkbench::Append_ActionAsStages(",
        )
        self.assertIn(
            "CKoukuSaydonAnimationActionDocument::Resolve_ActionCategory(", rebuild
        )
        self.assertNotIn('"MN_RPCZ_00"', rebuild)
        self.assertIn(
            'constexpr const char_t* BOSS_BODY_PROFILE_ID = "MN_RPCZ_00";',
            self.workbench_cpp,
        )
        admitted = _region(
            self.workbench_cpp,
            "bool_t Client::CKoukuSaydonActionWorkbench::Is_AppendAdmitted(",
            "bool_t Client::CKoukuSaydonActionWorkbench::Resolve_NativeClipMs(",
        )
        self.assertIn("BOSS_BODY_PROFILE_ID", admitted)
        for token in (
            '"Composition Resources###KoukuCompositionResources"',
            'ImGui::Button("Preview Action")',
            'ImGui::Button("Append Action as Stages")',
            'ImGui::Button("Append Action to Selected Stage")',
            'ImGui::Button("Append as Stage")',
            'ImGui::Button("Add Animation Row")',
            "Is_AppendAdmitted(source, outStatus)",
        ):
            self.assertIn(token, self.workbench_cpp)
        self.assertIn("static const char_t* Resolve_ActionCategory(", self.action_document_h)
        self.assertIn(
            "Client::CKoukuSaydonAnimationActionDocument::Resolve_ActionCategory(",
            self.action_document_cpp,
        )
        resolver = _region(
            self.animation_cpp,
            "const char_t* Resolve_ActionCompositionCategory(",
            "\n\t}\n",
        )
        self.assertIn(
            "CKoukuSaydonAnimationActionDocument::Resolve_ActionCategory(", resolver
        )
        self.assertNotIn('"Large Kouku"', resolver)

    def test_transport_and_playback_are_typed_value_requests(self) -> None:
        for token in (
            "enum class KOUKU_PREVIEW_TRANSPORT",
            "struct KOUKU_PREVIEW_STATE final",
            "Consume_PreviewTransportRequest(",
            "Set_PreviewState(",
            "Set_AnimationPlayback(",
        ):
            self.assertIn(token, self.workbench_h)
        for token in (
            'ImGui::InvisibleButton("##KoukuRuler"',
            'ImGui::Button("Apply Playback")',
            'ImGui::Button("Stop")',
            "Normalize_EndPolicyForWindow(",
            "m_iPendingPreviewStartMs",
            "m_PreviewState.strPatternId == pattern->strPatternId",
        ):
            self.assertIn(token, self.workbench_cpp)
        consume = self.main_cpp.index(
            "m_pKoukuSaydonActionWorkbench->Consume_PreviewTransportRequest("
        )
        start = self.main_cpp.index(
            "m_pKoukuSaydonActionWorkbench->Consume_PatternPreviewRequest("
        )
        snapshot = self.main_cpp.index(
            "m_pKoukuSaydonActionWorkbench->Set_PreviewState(", consume
        )
        self.assertLess(start, consume)
        self.assertLess(consume, snapshot)
        self.assertIn("pattern, startClockMs, startPaused", self.main_cpp[start:consume])
        self.assertIn("pattern, m_strToolStatus, startClockMs, startPaused", self.main_cpp[start:consume])
        self.assertNotIn("m_bSeekToCursorAfterStart", self.workbench_cpp)
        for token in (
            "Set_KoukuCompositionPreviewPaused(",
            "Stop_KoukuCompositionPreview(",
            "Seek_KoukuCompositionPreview(",
            "Get_KoukuCompositionPreviewState()",
            "m_pKoukuSaydonActionWorkbench->Set_PreviewState(",
        ):
            self.assertIn(token, self.main_cpp[consume:])
        for token in (
            "KOUKU_COMPOSITION_PREVIEW_STATE",
            "Set_KoukuCompositionPreviewPaused(",
            "Seek_KoukuCompositionPreview(",
            "Stop_KoukuCompositionPreview(",
        ):
            self.assertIn(token, self.animation_h)
        self.assertNotIn("CAnimation_Tool", self.workbench_cpp)
        self.assertNotIn("CAnimation_Tool", self.workbench_h)

    def test_exact_window_past_native_clip_holds_instead_of_vanishing(self) -> None:
        start = self.animation_cpp.index(
            "bool_t Client::CAnimation_Tool::Start_PendingKoukuSaydonCompositionPreview("
        )
        body = self.animation_cpp[start:]
        body = body[: body.index("\nbool_t Client::CAnimation_Tool::", 10)]
        self.assertNotIn("trim exceeds native clip", body)
        self.assertIn("holds its last pose past the native clip end", body)
        self.assertIn('(row.strEndPolicy == "HOLD_LAST_POSE" || row.iSourceStartMs', body)
        playback = _region(
            self.workbench_cpp,
            "bool_t Client::CKoukuSaydonActionWorkbench::Set_AnimationPlayback(",
            "void Client::CKoukuSaydonActionWorkbench::Normalize_Selection()",
        )
        self.assertIn("EXACT cannot outrun the native clip", playback)
        for signature in (
            "bool_t Client::CKoukuSaydonActionWorkbench::Trim_Animation(",
            "bool_t Client::CKoukuSaydonActionWorkbench::Bind_Animation(",
            "bool_t Client::CKoukuSaydonActionWorkbench::Append_AnimationAsStage(",
        ):
            start_at = self.workbench_cpp.index(signature)
            self.assertIn(
                "Normalize_EndPolicyForWindow(",
                self.workbench_cpp[start_at : start_at + 3000],
            )


if __name__ == "__main__":
    unittest.main()
