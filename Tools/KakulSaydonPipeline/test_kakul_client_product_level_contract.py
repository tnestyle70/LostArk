#!/usr/bin/env python3
"""Focused source contract for the KakulSaydon Client product level."""

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


class KakulClientProductLevelContractTests(unittest.TestCase):
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

    def test_loader_stages_only_map_and_server_player_bundle(self) -> None:
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
        for forbidden in (
            "Ready_ValtanPresentation",
            "Ready_DeployPropArea",
            "Ensure_Prototypes",
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
            "BossTool",
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
            'ImGui::BeginTabItem("KoukuSaton")',
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


if __name__ == "__main__":
    unittest.main()
