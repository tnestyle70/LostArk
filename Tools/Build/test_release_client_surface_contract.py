"""Source/data contract for Release level diagnostics and product input paths."""

from __future__ import annotations

import json
import pathlib
import re
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8-sig")


class ReleaseClientSurfaceContractTests(unittest.TestCase):
    def test_lobby_panel_is_debug_only_and_product_buttons_cover_four_commands(self) -> None:
        header = read("Client/Public/Level_Lobby.h")
        source = read("Client/Private/Level_Lobby.cpp")
        main = read("Client/Private/MainApp.cpp")
        layout = json.loads(read("Data/UI/Lobby/Lobby_Layout.json"))

        self.assertRegex(
            header,
            r"#ifdef _DEBUG\s+void Render_StagePanel\(\);\s+#endif",
        )
        render = re.search(
            r"HRESULT CLevel_Lobby::Render\(\)\s*\{(?P<body>.*?)\n\}",
            source,
            re.S,
        )
        self.assertIsNotNone(render)
        self.assertRegex(
            render.group("body"),
            r"(?s)#ifdef _DEBUG.*Render_StagePanel\(\);.*#endif",
        )

        expected = {
            "Lobby_TestButton": "LOBBY_STAGE::TEST",
            "Lobby_CreateCharacterButton": "LOBBY_STAGE::CHARACTER_SELECT",
            "Lobby_ValtanButton": "LOBBY_STAGE::VALTAN",
            "Lobby_BernButton": "LOBBY_STAGE::BERN",
        }
        slot_ids = [slot["id"] for slot in layout["slots"]]
        for slot_id, stage in expected.items():
            self.assertEqual(slot_ids.count(slot_id), 1)
            self.assertIn(slot_id, main)
            self.assertIn(stage, main)
        self.assertEqual(slot_ids.count("Lobby_StatusText"), 1)
        self.assertIn("CLevel_Lobby::Submit_ProductCommand", main)
        self.assertIn("CLevel_Lobby::Get_ProductStatus", main)
        self.assertIn("Resolve_LobbyProductButtonRects", main)
        self.assertIn("Is_ValidProductRect", main)
        self.assertIn("LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE", source)

    def test_character_select_visible_panel_is_debug_only_but_modal_host_is_common(self) -> None:
        header = read("Client/Public/Level_CharacterSelect.h")
        source = read("Client/Private/Level_CharacterSelect.cpp")

        self.assertRegex(
            header,
            r"void Render_CreateCharacterProductInputHost\(\);\s+"
            r"void Render_ProductStatus\(\);\s+"
            r"bool_t Is_ProductPointerHovered\(\) const;\s+"
            r"#ifdef _DEBUG\s+void Render_SelectionPanel\(\);\s+#endif",
        )
        render = re.search(
            r"HRESULT CLevel_CharacterSelect::Render\(\)\s*\{(?P<body>.*?)\n\}",
            source,
            re.S,
        )
        self.assertIsNotNone(render)
        body = render.group("body")
        self.assertRegex(
            body,
            r"(?s)#ifdef _DEBUG.*Render_SelectionPanel\(\);.*#endif",
        )
        self.assertIn("Render_CreateCharacterProductInputHost();", body)
        self.assertIn("Render_ProductStatus();", body)
        self.assertIn('"##CharacterSelectProductInputHost"', source)

        host = re.search(
            r"void CLevel_CharacterSelect::Render_CreateCharacterProductInputHost\(\)"
            r"\s*\{(?P<body>.*?)\n\}",
            source,
            re.S,
        )
        self.assertIsNotNone(host)
        self.assertIn("m_hasCreateCharacterButtonClick = false", host.group("body"))
        self.assertIn("Open_CreateCharacterModal();", host.group("body"))
        self.assertIn("Render_CreateCharacterModal();", host.group("body"))

        panel = re.search(
            r"void CLevel_CharacterSelect::Render_SelectionPanel\(\)"
            r"\s*\{(?P<body>.*?)\n\}\n#endif",
            source,
            re.S,
        )
        self.assertIsNotNone(panel)
        self.assertNotIn("m_hasCreateCharacterButtonClick = false", panel.group("body"))
        self.assertNotIn("Open_CreateCharacterModal();", panel.group("body"))

        layout = json.loads(read("Data/UI/ClassSelect/ClassSelect_Layout.json"))
        slot_ids = [slot["id"] for slot in layout["slots"]]
        self.assertEqual(slot_ids.count("CharacterSelect_StatusText"), 1)
        self.assertEqual(slot_ids.count("CreateCharacterModal_StatusText"), 1)
        self.assertIn("CLevel_CharacterSelect::Render_ProductStatus", source)
        self.assertIn("CreateCharacterModal_StatusText", source)
        self.assertIn("Has_CompleteProductButtonSlots", source)
        self.assertIn("Resolve_ProductButtonRect", source)
        self.assertIn("Is_ProductPointerHovered()", source)
        self.assertIn("SetMouseButtonBlocked(DIM::LB, true)", source)
        self.assertIn("SPAWN_HIT_ORDER", source)
        self.assertIn('m_strStatus = "Enter a 1-32 byte nickname, then confirm."', source)

    def test_valtan_raid_clear_release_path_uses_reliable_death_and_consumes_input(self) -> None:
        header = read("Client/Public/Level_ValtanArena.h")
        source = read("Client/Private/Level_ValtanArena.cpp")
        replication = read("Client/Private/ClientReplication.cpp")
        view_model = read("Client/Private/CombatHUDViewModel.cpp")
        server = read("Server/Private/GameRoom.cpp")
        layout = json.loads(read("Data/UI/RaidClear/RaidClear_Layout.json"))

        self.assertIn("LOSTARK_RAID_CLEAR_TEST_MODE", source)
        self.assertIn("Is_RaidClearTestModeEnabled()", source)
        self.assertIn("Update_DeadScene(bool_t isBlockedByRaidClear);", header)

        update_start = source.index("void CLevel_ValtanArena::Update(f32_t fTimeDelta)")
        raid_clear_update = source.index("Update_RaidClear(fTimeDelta);", update_start)
        controller_update = source.index("\n\tm_PlayerController.Update(", update_start)
        self.assertLess(raid_clear_update, controller_update)
        update_slice = source[raid_clear_update:controller_update]
        self.assertIn("SetMouseButtonBlocked(DIM::LB, true)", update_slice)
        self.assertIn("SetMouseButtonBlocked(DIM::RB, true)", update_slice)
        self.assertIn("cameraAcceptsGameplay && !isRaidClearActive", source)

        despawn_start = replication.index(
            "bool Client::CClientReplication::Apply_WorldEntityDespawn("
        )
        despawn_end = replication.index("\n}", despawn_start)
        self.assertIn("Set_BossDeadRaw(true);", replication[despawn_start:despawn_end])

        reset_start = view_model.index(
            "void Client::CCombatHUDViewModel::Reset_RuntimeState()"
        )
        reset_end = view_model.index("\n}", reset_start)
        reset_body = view_model[reset_start:reset_end]
        self.assertIn("m_bBossDeadRaw = false;", reset_body)
        self.assertIn("m_RaidClearTextRects = {};", reset_body)

        return_start = server.index(
            "void LostArk::Server::CGameRoom::Handle_ReturnToBern("
        )
        return_end = server.index("\n}", return_start)
        return_body = server[return_start:return_end]
        self.assertIn("m_bValtanRaidCleared", return_body)
        self.assertIn("Is_RaidClearTestModeEnabled()", return_body)

        slot_ids = [slot["id"] for slot in layout["slots"]]
        self.assertEqual(slot_ids.count("RaidClear_ReturnButton"), 1)

    def test_loading_release_has_product_recovery_without_visible_imgui_windows(self) -> None:
        header = read("Client/Public/Level_Loading.h")
        source = read("Client/Private/Level_Loading.cpp")
        recovery = json.loads(read("Data/UI/Loading/LoadingRecovery.json"))

        self.assertRegex(
            header,
            r"#ifdef _DEBUG\s+void Render_LoadingProgressDiagnostics\(\);\s+#endif",
        )
        self.assertIn("void Render_LoadingRecoveryProduct();", header)
        self.assertNotIn('ImGui::Begin(\n\t\t\t"Loading recovery"', source)
        self.assertIn("Render_LoadingRecoveryProduct();", source)
        self.assertIn('"Loading progress"', source)
        self.assertIn("DEFAULT_RETRY_RECT", source)
        self.assertIn("AddRectFilled", source)
        self.assertIn("std::isfinite(AuthoredRect.fWidth)", source)

        slots = [slot["id"] for slot in recovery["slots"]]
        for slot_id in (
            "LoadingRecovery_Panel",
            "LoadingRecovery_Message",
            "LoadingRecovery_RetryButton",
        ):
            self.assertEqual(slots.count(slot_id), 1)


if __name__ == "__main__":
    unittest.main()
