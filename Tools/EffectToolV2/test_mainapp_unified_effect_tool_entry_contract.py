from __future__ import annotations
import sys as _cpp_domain_sys
from pathlib import Path as _CppDomainPath
_cpp_domain_sys.path.insert(0, str(_CppDomainPath(__file__).resolve().parents[1] / "Build"))
from cpp_source_domains import read_source_text


import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MAIN_HEADER = ROOT / "Client/Public/MainApp.h"
MAIN_SOURCE = ROOT / "Client/Private/MainApp.cpp"


def read(path: Path) -> str:
    return read_source_text(path, encoding="utf-8-sig")


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


class MainAppSplitEffectToolEntryContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.header = read(MAIN_HEADER)
        cls.source = read(MAIN_SOURCE)

    def test_effect_versions_have_independent_lazy_entries(self) -> None:
        self.assertIn("EFFECT_V2,", self.header)
        ensure = function_body(
            self.source, "HRESULT CMainApp::EnsureDebugTool("
        )
        aliases = ensure[:ensure.index("switch (eTool)")]
        self.assertNotIn("DEBUG_TOOL::EFFECT_V2 == eTool", aliases)
        self.assertIn("case DEBUG_TOOL::EFFECT:", ensure)
        self.assertIn("case DEBUG_TOOL::EFFECT_V2:", ensure)

    def test_opening_each_version_constructs_only_its_own_effect_backend(self) -> None:
        ensure = function_body(
            self.source, "HRESULT CMainApp::EnsureDebugTool("
        )
        effect_case = ensure.split("case DEBUG_TOOL::EFFECT:", 1)[1].split("case DEBUG_TOOL::", 1)[0]
        v2_case = ensure.split("case DEBUG_TOOL::EFFECT_V2:", 1)[1].split("case DEBUG_TOOL::", 1)[0]
        self.assertIn("make_unique<CEffect_Tool>(", effect_case)
        self.assertIn("m_pEffectTool->Configure_AuthoringWorkspace(", effect_case)
        self.assertNotIn("m_pEffectToolV2", effect_case)
        self.assertIn("make_unique<CEffect_Tool_V2>(m_pDevice, m_pContext)", v2_case)
        self.assertNotIn("make_unique<CEffect_Tool>(", v2_case)
        self.assertNotIn("m_pEffectTool->", v2_case)
        self.assertIn("SetDebugToolVisible(eTool, true)", ensure)

    def test_visibility_render_and_close_edges_are_independent(self) -> None:
        render = function_body(self.source, "HRESULT CMainApp::Render()")
        effect_render = function_body(render, "if (IsDebugToolVisible(DEBUG_TOOL::EFFECT)")
        v2_render = function_body(render, "if (IsDebugToolVisible(DEBUG_TOOL::EFFECT_V2)")
        self.assertIn("m_pEffectTool->Render();", effect_render)
        self.assertNotIn("m_pEffectToolV2->Render();", effect_render)
        self.assertIn("m_pEffectToolV2->Render();", v2_render)
        self.assertNotIn("m_pEffectTool->Render();", v2_render)

        visibility = function_body(
            self.source, "void CMainApp::SetDebugToolVisible("
        )
        for canonicalizer in (visibility, function_body(
            self.source, "bool_t CMainApp::IsDebugToolVisible(")):
            self.assertNotIn("DEBUG_TOOL::EFFECT_V2 == eTool", canonicalizer)
        self.assertNotIn("m_DebugToolVisible[static_cast<size_t>(DEBUG_TOOL::EFFECT_V2)] = false", visibility)
        self.assertRegex(visibility, r"DEBUG_TOOL::EFFECT\s*==\s*eCanonicalTool[^;]*m_pEffectTool->Deactivate_AuthoringWorkspace\(\)")
        self.assertRegex(visibility, r"DEBUG_TOOL::EFFECT_V2\s*==\s*eCanonicalTool[^;]*m_pEffectToolV2->Deactivate\(\)")
        close_all = function_body(self.source, "void CMainApp::CloseAllDebugTools()")
        self.assertIn("SetDebugToolVisible(eTool, false)", close_all)
        self.assertNotIn("DEBUG_TOOL::EFFECT_V2 != eTool", close_all)

    def test_f1_has_separate_versioned_buttons_and_focus_options(self) -> None:
        developer_tools = function_body(
            self.source, "void CMainApp::RenderDeveloperTools()"
        )
        for tool, label in (("EFFECT", "Effect Tool V1"), ("EFFECT_V2", "Effect Tool V2")):
            self.assertEqual(1, developer_tools.count(f'toolCell("{label}", DEBUG_TOOL::{tool});'))
            self.assertEqual(1, developer_tools.count(f'{{ DEBUG_TOOL::{tool}, "{label}" }}'))
        self.assertNotIn('toolCell("Effect Tool",', developer_tools)
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
        self.assertEqual([("EFFECT", "Effect Tool V1"), ("EFFECT_V2", "Effect Tool V2")],
                         [row for row in entries if row[0] in ("EFFECT", "EFFECT_V2")])

    def test_resource_inventory_preserves_each_effect_authoring_owner(self) -> None:
        refresh = function_body(self.source, "void CMainApp::RefreshDebugResourceFiles()")
        roots = re.findall(
            r'\{\s*"Effect Resource",\s*"(Resources|Data)",\s*(.*?),'
            r'\s*"([^"]+)",\s*DEBUG_TOOL::(\w+)\s*\}', refresh, re.DOTALL)
        self.assertEqual(4, len(roots))
        self.assertEqual({"EFFECT", "EFFECT_V2"}, {row[3] for row in roots})
        self.assertEqual({"Resources/Effect", "Data/Effects/Authored",
                          "Data/Effects/Assemblies", "Data/Effects/V2"}, {row[2] for row in roots})
        self.assertIn("CRuntimeAssetRoot::Get_ResourceRoot()", refresh)
        self.assertIn("CProjectDataRoot::Get()", refresh)
        self.assertIn("file.eTool = root.eTool", refresh)
        self.assertIn("file.strRelativePath = std::string(root.pStablePrefix)", refresh)
        open_file = function_body(self.source, "void CMainApp::OpenDebugResourceFile(")
        self.assertIn("const DEBUG_RESOURCE_FILE& file = m_DebugResourceFiles[iFile]", open_file)
        self.assertIn("EnsureDebugTool(file.eTool)", open_file)

    def test_v2_data_files_route_to_v2_while_v1_roots_keep_v1(self) -> None:
        refresh = function_body(
            self.source, "void CMainApp::RefreshDebugResourceFiles()"
        )
        self.assertEqual(refresh.count('{ "Effect Resource",'), 4)
        for prefix in (
            '"Resources/Effect", DEBUG_TOOL::EFFECT',
            '"Data/Effects/Authored", DEBUG_TOOL::EFFECT',
            '"Data/Effects/Assemblies", DEBUG_TOOL::EFFECT',
            '"Data/Effects/V2", DEBUG_TOOL::EFFECT_V2',
        ):
            self.assertIn(prefix, refresh)

    def test_retired_composition_alias_does_not_collapse_the_v2_entry(self) -> None:
        ensure = function_body(self.source, "HRESULT CMainApp::EnsureDebugTool(")
        self.assertRegex(ensure, r"if \(DEBUG_TOOL::EFFECT_COMPOSITION == eTool\)\s*return EnsureDebugTool\(DEBUG_TOOL::EFFECT\);")
        self.assertIn("case DEBUG_TOOL::EFFECT_V2:", ensure)
        self.assertNotIn("m_pEffectCompositionWorkbench", self.header + self.source)
        developer_tools = function_body(self.source, "void CMainApp::RenderDeveloperTools()")
        self.assertNotIn('toolCell("Effect Composition Workbench"', developer_tools)

    def test_cpu_v2_pane_reuses_existing_resource_and_detail_editors(self) -> None:
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
        pane = read(ROOT / "Client/Private/EffectAuthoringV2Pane.cpp")
        self.assertIn("m_Editor.Render_CompositionResources(*document)", pane)
        self.assertIn("m_Editor.Render_DraftDetail(*document)", pane)
        pane_open = function_body(pane, "bool CEffectAuthoringV2Pane::Open(")
        self.assertIn("m_Edit.Load(", pane_open)
        for forbidden in ("Spawn_Preview", "Prewarm", "Add_GameObject", "Reload_BossValtan"):
            self.assertNotIn(forbidden, pane_open)

    def test_both_effect_previews_release_before_level_transition_and_shutdown(self) -> None:
        transition = function_body(self.source, "void CMainApp::Apply_LevelRequest()")
        self.assertLess(transition.index("m_pEffectTool->Deactivate_AuthoringWorkspace()"), transition.index("Start_Level("))
        self.assertLess(transition.index("m_pEffectToolV2->Deactivate()"), transition.index("Start_Level("))
        shutdown = function_body(self.source, "void CMainApp::Free()")
        self.assertLess(shutdown.index("m_pEffectTool->Deactivate_AuthoringWorkspace()"), shutdown.index("m_pKoukuPresentationPlayer.reset()"))
        self.assertLess(shutdown.index("m_pEffectToolV2->Deactivate()"), shutdown.index("m_pKoukuPresentationPlayer.reset()"))

    def test_v2_keeps_cpu_and_native_drafts_across_its_own_visibility_edge(self) -> None:
        source = read(ROOT / "Client/Private/Effect_Tool_V2.cpp")
        header = read(ROOT / "Client/Public/Effect_Tool_V2.h")
        deactivate = function_body(source, "void Client::CEffect_Tool_V2::Deactivate()")
        self.assertIn("m_pAuthoringSequencer->Stop()", deactivate)
        self.assertLess(deactivate.index("Capture_PreviewDocument(nativeDraft)"),
                        deactivate.index("pPreview->Finish()"))
        self.assertLess(deactivate.index("m_PreservedNativeDraft = std::move(nativeDraft)"),
                        deactivate.index("m_pPreview.reset()"))
        for forbidden in ("m_pAuthoringPane.reset", "m_PreservedNativeDraft.reset", "m_Group ="):
            self.assertNotIn(forbidden, deactivate)
        capture = function_body(source, "bool Client::CEffect_Tool_V2::Capture_PreviewDocument(")
        for token in ("preview->Params()", "preview->PivotWorld()", "preview->Part_Visible(part)",
                      "preview->Part_BaseAssetId(part)", "document.strAnimationClip"):
            self.assertIn(token, capture)
        restore = function_body(source, "bool Client::CEffect_Tool_V2::Restore_NativeDraft()")
        self.assertIn("if (!Spawn_Preview(draft.Desc, draft.Parts, draft.strAnimationClip))", restore)
        self.assertLess(restore.index("m_eType = draft.eType"), restore.index("Spawn_Preview("))
        failed_restore = function_body(restore, "if (!Spawn_Preview(")
        self.assertIn("m_eType = previousType", failed_restore)
        self.assertIn("return false;", failed_restore)
        self.assertNotIn("m_PreservedNativeDraft.reset()", failed_restore)
        self.assertLess(restore.index("Spawn_Preview("), restore.index("m_PreservedNativeDraft.reset()"))
        self.assertIn("void Activate() { m_bNativeRestorePending = true; }", header)
        ensure = function_body(self.source, "HRESULT CMainApp::EnsureDebugTool(")
        v2_case = ensure.split("case DEBUG_TOOL::EFFECT_V2:", 1)[1].split("case DEBUG_TOOL::", 1)[0]
        self.assertIn("m_pEffectToolV2->Activate()", v2_case)
        render = function_body(source, "void Client::CEffect_Tool_V2::Render()")
        self.assertIn("Restore_NativeDraft()", render)

    def test_v2_renders_its_cpu_editor_sequencer_and_native_controls(self) -> None:
        source = read(ROOT / "Client/Private/Effect_Tool_V2.cpp")
        configure = function_body(source, "void Client::CEffect_Tool_V2::Configure_AuthoringWorkspace(")
        self.assertIn("make_unique<CEffectAuthoringV2Pane>(*this)", configure)
        self.assertIn('"effect.sequence.v2.default"', configure)
        self.assertIn("Set_V2SnapshotProvider(", configure)
        self.assertIn("m_pAuthoringPane->Snapshot(selected, snapshot, error)", configure)
        workspace = function_body(source, "void Client::CEffect_Tool_V2::Render_AuthoringWorkspace()")
        for token in ('"Effect Tool V2###EffectToolV2"', "m_pAuthoringPane->Render_ToolContents()",
                      "m_pAuthoringPane->Render_DetailContents()", "m_pAuthoringPane->Render_ResourceContents()",
                      "m_pAuthoringSequencer->Render_ModelView()",
                      'Render_Sequencer("Sequencer V2###EffectAuthoringV2")'):
            self.assertIn(token, workspace)
        render = function_body(source, "void Client::CEffect_Tool_V2::Render()")
        for token in ("Render_AuthoringWorkspace()", '"Effect Resource Library"',
                      "Render_TuningPanel()", "Render_AttachWindow()", "Render_GroupWindow()"):
            self.assertIn(token, render)

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
        capture = function_body(legacy, "bool Client::CEffect_Tool_V2::Capture_PreviewDocument(")
        self.assertIn("document.strDisplayName = m_strLoadedDocumentDisplayName", capture)
        for token in ("Capture_PreviewDocument(Document)",
                      "CEffectV2Document::Parse_Document(strBytes, Validated, strError)",
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

    def test_v1_workspace_does_not_own_or_switch_to_a_v2_editor(self) -> None:
        v1_header = read(ROOT / "Client/Public/Effect_Tool.h")
        v1_source = read(ROOT / "Client/Private/Effect_Tool.cpp")
        workspace = read(ROOT / "Client/Private/Effect_Tool_Workspace.cpp")
        for forbidden in ("m_pAuthoringV2", "m_pLegacyV2", "m_bLegacyV2Window",
                          "m_bAuthoringV2Selected", "Render_AuthoringOwnerSelector"):
            self.assertNotIn(forbidden, v1_header + v1_source + workspace)
        configure = function_body(workspace, "void CEffect_Tool::Configure_AuthoringWorkspace(")
        self.assertNotIn("CEffect_Tool_V2", configure)
        self.assertIn("Set_V1Callbacks(", configure)
        self.assertIn("Set_V1AnchorProvider(", configure)


if __name__ == "__main__":
    unittest.main()
