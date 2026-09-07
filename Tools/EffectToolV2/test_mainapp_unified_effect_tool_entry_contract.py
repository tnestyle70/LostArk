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
        # Other compatibility aliases may share this canonicalizer too. The
        # retired Effect enum must still resolve to EFFECT, never its own slot.
        for canonicalizer in (visibility, function_body(
            self.source, "bool_t CMainApp::IsDebugToolVisible(")):
            self.assertRegex(canonicalizer,
                             r"DEBUG_TOOL::EFFECT_V2\s*==\s*eTool\s*\?\s*DEBUG_TOOL::EFFECT\s*:")
        self.assertIn("m_DebugToolVisible[static_cast<size_t>(DEBUG_TOOL::EFFECT_V2)] = false", visibility)
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
        options = re.search(
            r"std::array<std::pair<DEBUG_TOOL,\s*const char_t\*>,\s*(\d+)>"
            r"\s*TOOL_FOCUS_OPTIONS\s*=\s*\{\{(.*?)\}\};",
            developer_tools, re.DOTALL,
        )
        self.assertIsNotNone(options)
        entries = re.findall(r'\{\s*DEBUG_TOOL::(\w+),\s*"([^"]+)"\s*\}', options.group(2))
        self.assertEqual(int(options.group(1)), len(entries))
        self.assertEqual(len(entries), len({tool for tool, _label in entries}))
        self.assertEqual(len(entries), len({label for _tool, label in entries}))
        self.assertEqual([("EFFECT", "Effect Tool")], [row for row in entries if row[0] == "EFFECT"])
        self.assertNotIn("EFFECT_V2", {tool for tool, _label in entries})

    def test_resource_inventory_and_file_open_preserve_one_effect_authoring_owner(self) -> None:
        # The old source summary panel was removed. Its current consumer is the
        # Resources/Data file inventory, which carries the same typed owner all
        # the way from each canonical path to EnsureDebugTool.
        refresh = function_body(self.source, "void CMainApp::RefreshDebugResourceFiles()")
        roots = re.findall(
            r'\{\s*"Effect Resource",\s*"(Resources|Data)",\s*(.*?),'
            r'\s*"([^"]+)",\s*DEBUG_TOOL::(\w+)\s*\}', refresh, re.DOTALL)
        self.assertEqual(4, len(roots))
        self.assertEqual({"EFFECT"}, {row[3] for row in roots})
        self.assertEqual({"Resources/Effect", "Data/Effects/Authored",
                          "Data/Effects/Assemblies", "Data/Effects/V2"}, {row[2] for row in roots})
        self.assertIn("CRuntimeAssetRoot::Get_ResourceRoot()", refresh)
        self.assertIn("CProjectDataRoot::Get()", refresh)
        self.assertIn("file.eTool = root.eTool", refresh)
        self.assertIn("file.strRelativePath = std::string(root.pStablePrefix)", refresh)
        open_file = function_body(self.source, "void CMainApp::OpenDebugResourceFile(")
        self.assertIn("const DEBUG_RESOURCE_FILE& file = m_DebugResourceFiles[iFile]", open_file)
        self.assertIn("EnsureDebugTool(file.eTool)", open_file)
        for forbidden in ('"Effect V1"', '"Effect V2"', "DEBUG_TOOL::EFFECT_V2"):
            self.assertNotIn(forbidden, refresh + open_file)

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

    def test_effect_composition_is_an_independent_lazy_entry_using_the_existing_resource_editor(self) -> None:
        ensure = function_body(self.source, "HRESULT CMainApp::EnsureDebugTool(")
        composition = ensure.split("case DEBUG_TOOL::EFFECT_COMPOSITION:", 1)[1].split(
            "case DEBUG_TOOL::WORLD_OBJECT:", 1)[0]
        for token in ("if (!m_pEffectToolV2)", "make_unique<CEffect_Tool_V2>",
                      "if (!m_pEffectCompositionWorkbench)", "*m_pEffectToolV2)",
                      "m_pEffectCompositionWorkbench->Open()"):
            self.assertIn(token, composition)
        for forbidden in ("EnsureAnimationPreviewBackend", "make_unique<CEffect_Tool>",
                          "Reload_BossValtan", "Open_ValtanAllEffectsWorkspace"):
            self.assertNotIn(forbidden, composition)
        tool = read(ROOT / "Client/Private/Effect_Tool_V2.cpp")
        constructor = function_body(tool, "Client::CEffect_Tool_V2::CEffect_Tool_V2(")
        for forbidden in ("Scan_Resources", "Request_Preview", "Prewarm", "Add_GameObject"):
            self.assertNotIn(forbidden, constructor)
        picker = function_body(tool, "void Client::CEffect_Tool_V2::Render_CompositionResources(")
        for token in ("if (!m_bScanned) Scan_Resources()", "Render_SlotCards()",
                      "Render_ResourceBrowser()", "document.Desc.strMeshAssetId = slots",
                      "document.Desc.TextureAssetIds[i] = slots", "= previousBindings",
                      "m_eType = previousType", "m_eSelectedSlot = previousSlot"):
            self.assertIn(token, picker)
        workbench = read(ROOT / "Client/Private/EffectCompositionWorkbench.cpp")
        self.assertIn("m_ResourceEditor.Render_CompositionResources(*document)", workbench)
        self.assertIn("m_ResourceEditor.Render_DraftDetail(*document)", workbench)
        inventory = function_body(workbench, "void CEffectCompositionWorkbench::Refresh_Inventory()")
        self.assertIn("Read_Inventory(staged, status)", inventory)
        self.assertNotIn("Reload_BossValtan", inventory)
        developer_tools = function_body(self.source, "void CMainApp::RenderDeveloperTools()")
        self.assertEqual(1, developer_tools.count(
            'toolCell("Effect Composition Workbench", DEBUG_TOOL::EFFECT_COMPOSITION);'))
        self.assertEqual(1, developer_tools.count(
            '{ DEBUG_TOOL::EFFECT_COMPOSITION, "Effect Composition Workbench" }'))

    def test_effect_composition_preview_releases_before_shared_editor_or_level_owner(self) -> None:
        update = function_body(self.source, "void CMainApp::Update(")
        self.assertRegex(update, r"m_pEffectCompositionWorkbench->Update\(fTimeDelta,\s*m_bDeveloperToolsVisible\s*&&"
                         r"\s*IsDebugToolVisible\(DEBUG_TOOL::EFFECT_COMPOSITION\)\s*&&"
                         r"\s*DEBUG_TOOL::EFFECT_COMPOSITION\s*==\s*m_eDebugInputOwner")
        render = function_body(self.source, "HRESULT CMainApp::Render()")
        self.assertIn("IsDebugToolVisible(DEBUG_TOOL::EFFECT_COMPOSITION) && m_pEffectCompositionWorkbench", render)
        self.assertIn("m_pEffectCompositionWorkbench->Render()", render)
        visibility = function_body(self.source, "void CMainApp::SetDebugToolVisible(")
        self.assertRegex(visibility, r"DEBUG_TOOL::EFFECT_COMPOSITION\s*==\s*eCanonicalTool"
                         r"[^;]*m_pEffectCompositionWorkbench->Deactivate\(\)")
        transition = function_body(self.source, "void CMainApp::Apply_LevelRequest()")
        self.assertLess(transition.index("m_pEffectCompositionWorkbench->Deactivate()"), transition.index("Start_Level("))
        shutdown = function_body(self.source, "void CMainApp::Free()")
        self.assertLess(shutdown.index("m_pEffectCompositionWorkbench->Deactivate()"), shutdown.index("m_pKoukuPresentationPlayer.reset()"))
        self.assertLess(shutdown.index("m_pEffectCompositionWorkbench.reset()"), shutdown.index("m_pEffectToolV2.reset()"))
        workbench = read(ROOT / "Client/Private/EffectCompositionWorkbench.cpp")
        stop = function_body(workbench, "void CEffectCompositionWorkbench::Stop()")
        self.assertIn("CEffectV2Runtime::Stop_Group(m_Handle)", stop)
        self.assertIn("m_Model.Stop()", stop)
        self.assertIn("m_PlaySnapshot.reset()", stop)
        self.assertNotIn("m_Edit =", stop)
        self.assertNotIn("m_Edit.Clear", stop)

    def test_effect_composition_save_validates_cpu_drafts_without_gpu_or_animation_admission(self) -> None:
        session = read(ROOT / "Client/Private/EffectEditingSession.cpp")
        save = function_body(session, "bool CEffectEditingSession::Save(")
        first_write = save.index("CEffectV2Document::Write_AtomicFile(")
        for token in ("CEffectV2Document::Parse_Document(", "CEffectV2Document::Parse_Group(",
                      "current != write.before"):
            self.assertLess(save.index(token), first_write)
        self.assertIn("current != write.after", save)
        self.assertIn("write.before, error", save)
        self.assertIn("m_GroupBaseline = groupText", save)
        self.assertIn("CEffectV2Runtime::Invalidate_Caches()", save)
        self.assertLess(first_write, save.index("m_Dirty = false"))
        for forbidden in ("Play_Group(", "Prewarm(", "Add_GameObject", "->Clone(",
                          "EnsureDebugTool(", "Reload_BossValtan", "Animation_Count(",
                          "m_pDevice", "m_pContext"):
            self.assertNotIn(forbidden, save)

        legacy = read(ROOT / "Client/Private/Effect_Tool_V2.cpp")
        legacy_save = function_body(legacy, "bool_t Client::CEffect_Tool_V2::Save_Document()")
        legacy_write = legacy_save.index("CEffectV2Document::Write_AtomicFile(")
        self.assertIn("Document.strDisplayName = m_strLoadedDocumentDisplayName", legacy_save)
        for token in ("CEffectV2Document::Parse_Document(strBytes, Validated, strError)",
                      "Read_DocumentSource(Path, strCurrentBytes, bExists, strError)"):
            self.assertLess(legacy_save.index(token), legacy_write)
        loaded = function_body(legacy_save, "if (strEffectId == m_strLoadedDocumentId)")
        stale = function_body(loaded, "if (!bExists || strCurrentBytes != m_strLoadedDocumentBytes)")
        self.assertIn("return false;", stale)
        self.assertIn("return false;", function_body(legacy_save, "else if (bExists)"))
        self.assertLess(legacy_save.index("else if (bExists)"), legacy_write)
        for token in ("m_strLoadedDocumentId = strEffectId", "m_strLoadedDocumentBytes = strBytes",
                      "m_strLoadedDocumentDisplayName = Document.strDisplayName"):
            self.assertLess(legacy_write, legacy_save.index(token))
        legacy_load = function_body(legacy, "bool_t Client::CEffect_Tool_V2::Load_Document(")
        stages = [legacy_load.index(token) for token in (
            "CEffectV2Document::Is_ValidEffectId(strEffectId)", "Read_DocumentSource(",
            "CEffectV2Document::Parse_Document(strBytes, Document, strError)",
            "Document.strEffectId != strEffectId", "Spawn_Preview(",
            "m_strLoadedDocumentId = strEffectId")]
        self.assertEqual(stages, sorted(stages))
        failed_spawn = function_body(legacy_load, "if (!Spawn_Preview(")
        self.assertIn("return false;", failed_spawn)
        for token in ("m_strLoadedDocumentId", "m_strLoadedDocumentDisplayName", "m_strLoadedDocumentBytes"):
            self.assertNotIn(token, failed_spawn)
        self.assertIn("m_strLoadedDocumentDisplayName = Document.strDisplayName", legacy_load)
        self.assertIn("m_strLoadedDocumentBytes = std::move(strBytes)", legacy_load)
        reader = function_body(legacy, "bool_t Client::CEffect_Tool_V2::Read_DocumentSource(")
        self.assertIn("std::ifstream Input(Path, std::ios::binary)", reader)
        self.assertIn("strOutBytes.assign(std::istreambuf_iterator<char>(Input)", reader)
        self.assertIn("return false;", function_body(reader, "if (Input.bad())"))

    def test_mainapp_has_no_user_facing_versioned_effect_label(self) -> None:
        versioned_labels = re.findall(
            r'"[^"\n]*(?:Effect V1|Effect V2|Effect Tool v2)[^"\n]*"',
            self.source,
            flags=re.IGNORECASE,
        )
        self.assertEqual(versioned_labels, [])


if __name__ == "__main__":
    unittest.main()
