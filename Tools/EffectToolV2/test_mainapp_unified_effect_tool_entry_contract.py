from __future__ import annotations

import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MAIN_HEADER = ROOT / "Client/Public/MainApp.h"
MAIN_SOURCE = ROOT / "Client/Private/MainApp.cpp"


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8-sig")


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and next_char == "/":
                state = "line_comment"
                index += 2
                continue
            if char == "/" and next_char == "*":
                state = "block_comment"
                index += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[start : index + 1]
        elif state == "line_comment":
            if char == "\n":
                state = "code"
        elif state == "block_comment":
            if char == "*" and next_char == "/":
                state = "code"
                index += 2
                continue
        elif state in {"string", "character"}:
            if char == "\\":
                index += 2
                continue
            terminator = '"' if state == "string" else "'"
            if char == terminator:
                state = "code"
        index += 1
    raise AssertionError(f"unterminated function: {signature}")


class MainAppUnifiedEffectToolEntryContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.header = read(MAIN_HEADER)
        cls.source = read(MAIN_SOURCE)

    def test_compatibility_enum_routes_to_the_single_effect_entry(self) -> None:
        self.assertIn("EFFECT_V2,", self.header)
        self.assertIn("Compatibility-only route", self.header)
        ensure = function_body(
            self.source, "HRESULT CMainApp::EnsureDebugTool("
        )
        alias = "if (DEBUG_TOOL::EFFECT_V2 == eTool)"
        self.assertIn(alias, ensure)
        self.assertIn("return EnsureDebugTool(DEBUG_TOOL::EFFECT);", ensure)
        self.assertLess(ensure.index(alias), ensure.index("switch (eTool)"))
        self.assertNotIn("case DEBUG_TOOL::EFFECT_V2:", ensure)

    def test_single_effect_open_prepares_both_existing_backends(self) -> None:
        ensure = function_body(
            self.source, "HRESULT CMainApp::EnsureDebugTool("
        )
        effect_case = ensure[
            ensure.index("case DEBUG_TOOL::EFFECT:") :
            ensure.index("case DEBUG_TOOL::RENDERING:")
        ]
        for token in (
            "make_unique<CEffect_Tool>(",
            "make_unique<CEffect_Tool_V2>(m_pDevice, m_pContext)",
            "Open_ValtanAllEffectsWorkspace()",
        ):
            self.assertIn(token, effect_case)
        self.assertLess(
            effect_case.index("make_unique<CEffect_Tool>("),
            effect_case.index("make_unique<CEffect_Tool_V2>"),
        )
        self.assertLess(
            effect_case.index("make_unique<CEffect_Tool_V2>"),
            ensure.index("SetDebugToolVisible(eTool, true)"),
        )

    def test_one_visibility_edge_renders_and_deactivates_the_pair(self) -> None:
        render = function_body(self.source, "HRESULT CMainApp::Render()")
        self.assertIn("IsDebugToolVisible(DEBUG_TOOL::EFFECT)", render)
        self.assertNotIn("IsDebugToolVisible(DEBUG_TOOL::EFFECT_V2)", render)
        effect_render = render[
            render.index("if (IsDebugToolVisible(DEBUG_TOOL::EFFECT))") :
            render.index("if (IsDebugToolVisible(DEBUG_TOOL::RENDERING))")
        ]
        self.assertIn("m_pEffectTool->Render();", effect_render)
        self.assertIn("m_pEffectToolV2->Render();", effect_render)

        visibility = function_body(
            self.source, "void CMainApp::SetDebugToolVisible("
        )
        self.assertIn(
            "DEBUG_TOOL::EFFECT_V2 == eTool ?\n\t\tDEBUG_TOOL::EFFECT : eTool",
            visibility,
        )
        self.assertIn("DEBUG_TOOL::EFFECT == eCanonicalTool", visibility)
        self.assertIn("m_pEffectToolV2->Deactivate();", visibility)

    def test_f1_has_one_effect_button_and_one_focus_option(self) -> None:
        developer_tools = function_body(
            self.source, "void CMainApp::RenderDeveloperTools()"
        )
        self.assertEqual(
            developer_tools.count(
                'toolCell("Effect Tool", DEBUG_TOOL::EFFECT);'
            ),
            1,
        )
        self.assertEqual(
            developer_tools.count(
                '{ DEBUG_TOOL::EFFECT, "Effect Tool" }'
            ),
            1,
        )
        self.assertNotIn("DEBUG_TOOL::EFFECT_V2", developer_tools)
        self.assertNotIn("Effect Tool v2", developer_tools)
        self.assertIn(
            "std::array<std::pair<DEBUG_TOOL, const char_t*>, 10>",
            developer_tools,
        )

    def test_authoring_source_is_one_effect_resource_owner(self) -> None:
        refresh = function_body(
            self.source, "void CMainApp::RefreshDebugAuthoringSources()"
        )
        self.assertEqual(refresh.count('"Effect Resource"'), 1)
        for token in (
            '"Valtan Effect Resources"',
            '"Data/Effects"',
            "effect.valtan.*",
            "boss.valtan.*",
            'sourceExists(L"Effects/EffectCatalog.json")',
            'sourceExists(L"Effects/V2/Authored")',
            'sourceExists(L"Effects/V2/Groups")',
            'sourceExists(L"Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json")',
            '"Open Effect Tool", DEBUG_TOOL::EFFECT',
        ):
            self.assertIn(token, refresh)
        for forbidden in ('"Effect V1"', '"Effect V2"', "DEBUG_TOOL::EFFECT_V2"):
            self.assertNotIn(forbidden, refresh)

    def test_data_files_use_one_effect_resource_category_and_owner(self) -> None:
        refresh = function_body(
            self.source, "void CMainApp::RefreshDebugResourceFiles()"
        )
        self.assertEqual(refresh.count('{ "Effect Resource",'), 4)
        for prefix in (
            '"Resources/Effect", DEBUG_TOOL::EFFECT',
            '"Data/Effects/Authored", DEBUG_TOOL::EFFECT',
            '"Data/Effects/Assemblies", DEBUG_TOOL::EFFECT',
            '"Data/Effects/V2", DEBUG_TOOL::EFFECT',
        ):
            self.assertIn(prefix, refresh)
        self.assertNotIn('"Effect V1"', refresh)
        self.assertNotIn('"Effect V2"', refresh)
        self.assertNotIn("DEBUG_TOOL::EFFECT_V2", refresh)

    def test_mainapp_has_no_user_facing_versioned_effect_label(self) -> None:
        versioned_labels = re.findall(
            r'"[^"\n]*(?:Effect V1|Effect V2|Effect Tool v2)[^"\n]*"',
            self.source,
            flags=re.IGNORECASE,
        )
        self.assertEqual(versioned_labels, [])


if __name__ == "__main__":
    unittest.main()
