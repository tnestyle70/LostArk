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
        self.assertIn("CLevel_Lobby::Can_SubmitProductCommand", main)

    def test_character_select_visible_panel_is_debug_only_but_modal_host_is_common(self) -> None:
        header = read("Client/Public/Level_CharacterSelect.h")
        source = read("Client/Private/Level_CharacterSelect.cpp")

        self.assertRegex(
            header,
            r"void Render_CreateCharacterProductInputHost\(\);\s+"
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

        slots = [slot["id"] for slot in recovery["slots"]]
        for slot_id in (
            "LoadingRecovery_Panel",
            "LoadingRecovery_Message",
            "LoadingRecovery_RetryButton",
        ):
            self.assertEqual(slots.count(slot_id), 1)


if __name__ == "__main__":
    unittest.main()
