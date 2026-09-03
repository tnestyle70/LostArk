"""Resource-independent contracts for Composition -> All Effects refresh."""

from __future__ import annotations

import unittest
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
WORKBENCH_HEADER = REPOSITORY_ROOT / "Client/Public/ActionCompositionWorkbench.h"
WORKBENCH_SOURCE = REPOSITORY_ROOT / "Client/Private/ActionCompositionWorkbench.cpp"
EFFECT_HEADER = REPOSITORY_ROOT / "Client/Public/Effect_Tool.h"
EFFECT_SOURCE = REPOSITORY_ROOT / "Client/Private/Effect_Tool.cpp"
MAIN_SOURCE = REPOSITORY_ROOT / "Client/Private/MainApp.cpp"


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    state = "code"
    index = brace
    while index < len(source):
        char = source[index]
        pair = source[index : index + 2]
        if state == "code":
            if pair == "//":
                state = "line-comment"
                index += 2
                continue
            if pair == "/*":
                state = "block-comment"
                index += 2
                continue
            if char in {'"', "'"}:
                state = "string" if char == '"' else "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[start : index + 1]
        elif state == "line-comment":
            if char == "\n":
                state = "code"
        elif state == "block-comment":
            if pair == "*/":
                state = "code"
                index += 2
                continue
        else:
            if char == "\\":
                index += 2
                continue
            terminator = '"' if state == "string" else "'"
            if char == terminator:
                state = "code"
        index += 1
    raise AssertionError(f"unterminated C++ function: {signature}")


class EffectToolExactRevisionRefreshContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.workbench_header = WORKBENCH_HEADER.read_text(encoding="utf-8")
        cls.workbench_source = WORKBENCH_SOURCE.read_text(encoding="utf-8")
        cls.effect_header = EFFECT_HEADER.read_text(encoding="utf-8")
        cls.effect_source = EFFECT_SOURCE.read_text(encoding="utf-8")
        cls.main_source = MAIN_SOURCE.read_text(encoding="utf-8")

    def test_committed_save_queues_its_exact_receipt(self) -> None:
        mark = function_body(
            self.workbench_source,
            "void Client::CActionCompositionWorkbench::Mark_SourceCommitted(\n",
        )
        self.assertIn("const std::string& strExactSourceRevision", mark)
        self.assertIn(
            "m_strPostSaveRevision = strExactSourceRevision;", mark
        )
        self.assertNotIn("Get_ValtanPublishSourceRevision(", mark)
        self.assertIn(
            "m_strEffectGraphRefreshRevision = m_strPostSaveRevision;", mark
        )
        self.assertLess(
            mark.index("m_strEffectGraphRefreshRevision ="),
            mark.index("m_bEffectGraphRefreshRequested = true;"),
        )
        self.assertIn(
            "Consume_EffectGraphRefreshRequest(\n"
            "\t\tstd::string& strOutExpectedSourceRevision)",
            self.workbench_header,
        )

    def test_mainapp_delivers_without_a_composition_render_dependency(self) -> None:
        update = function_body(self.main_source, "void CMainApp::Update(")
        render = function_body(self.main_source, "HRESULT CMainApp::Render()")
        self.assertIn("Consume_EffectGraphRefreshRequest(", update)
        self.assertIn(
            "Request_ValtanGraphRefresh(\n\t\t\t\tExpectedValtanSourceRevision)",
            update,
        )
        self.assertLess(
            update.index("Consume_EffectGraphRefreshRequest("),
            update.index("m_pEffectTool->Update(fTimeDelta);"),
        )
        self.assertNotIn("Consume_EffectGraphRefreshRequest(", render)
        self.assertNotIn("Request_ValtanGraphRefresh(", render)

    def test_pattern_tree_is_the_gate_before_related_indexes_refresh(self) -> None:
        process = function_body(
            self.effect_source,
            "void Client::CEffect_Tool::Process_PendingValtanGraphRefresh()",
        )
        ordered = (
            "Refresh_ValtanPatternTreeForRevision(ExpectedRevision)",
            "Refresh_AllEffects(true)",
            "Refresh_DataFiles()",
            "Refresh_ValtanEffectResourceSnapshot()",
        )
        positions = [process.index(token) for token in ordered]
        self.assertEqual(positions, sorted(positions))
        self.assertIn("if (!Refresh_ValtanPatternTreeForRevision", process)
        self.assertIn("return;", process)

    def test_exact_revision_is_rechecked_before_tree_commit(self) -> None:
        wrapper = function_body(
            self.effect_source,
            "bool_t Client::CEffect_Tool::Refresh_ValtanPatternTreeForRevision(",
        )
        self.assertIn("Observe_ExpectedValtanSourceRevision(", wrapper)
        self.assertIn('"before parse"', wrapper)

        refresh = function_body(
            self.effect_source,
            "bool_t Client::CEffect_Tool::Refresh_ValtanPatternTree()",
        )
        guard = refresh.index(
            "Verify_ValtanCanonicalSourceRevision_WhileAdmitted("
        )
        commit = refresh.index("m_ValtanPatternTree = std::move(Staged);")
        self.assertLess(guard, commit)
        self.assertIn('"before commit"', refresh[guard:commit])
        self.assertLess(
            refresh.index("CanonicalAdmission.Validate_StillCurrent("), guard
        )

    def test_revision_mismatch_has_typed_stale_state_and_preserves_tree(self) -> None:
        observe = function_body(
            self.effect_source,
            "bool_t Client::CEffect_Tool::Observe_ExpectedValtanSourceRevision(",
        )
        for token in (
            "Get_ValtanPublishSourceRevision(",
            "Preserve_ValtanGraphForStaleRevision(",
        ):
            self.assertIn(token, observe)
        preserve = function_body(
            self.effect_source,
            "void Client::CEffect_Tool::Preserve_ValtanGraphForStaleRevision(",
        )
        for token in (
            "VALTAN_GRAPH_REFRESH_STATE::STALE_REVISION",
            "VALTAN_VIEW_ADMISSION::STALE_PRESERVED",
            '"STALE_REVISION: All Effects refresh preserved the previous Pattern tree',
        ):
            self.assertIn(token, preserve)
        for forbidden in (
            "m_ValtanPatternTree =",
            "m_ValtanToolAuditionInventory =",
            "m_ValtanUnifiedEffectCaches.clear()",
            "Stage_ValtanProductFallback(",
        ):
            self.assertNotIn(forbidden, preserve)
        self.assertIn("STALE_REVISION", self.effect_header)

    def test_effect_update_consumes_pending_refresh_while_render_stays_lazy(self) -> None:
        update = function_body(
            self.effect_source, "void Client::CEffect_Tool::Update("
        )
        render = function_body(
            self.effect_source, "void Client::CEffect_Tool::Render()"
        )
        self.assertIn("Process_PendingValtanGraphRefresh();", update)
        for forbidden in (
            "m_bValtanGraphRefreshRequested",
            "Refresh_AllEffects(true)",
            "Refresh_DataFiles()",
            "Refresh_ValtanEffectResourceSnapshot()",
        ):
            self.assertNotIn(forbidden, render)

    def test_first_effect_open_cannot_bypass_a_pending_exact_receipt(self) -> None:
        ensure = function_body(
            self.main_source, "HRESULT CMainApp::EnsureDebugTool("
        )
        effect_case = ensure[
            ensure.index("case DEBUG_TOOL::EFFECT:") :
            ensure.index("case DEBUG_TOOL::RENDERING:")
        ]
        deliver = effect_case.index("Consume_EffectGraphRefreshRequest(")
        open_workspace = effect_case.index("Open_ValtanAllEffectsWorkspace()")
        self.assertLess(deliver, open_workspace)

        open_all = function_body(
            self.effect_source,
            "bool_t Client::CEffect_Tool::Open_ValtanAllEffectsWorkspace()",
        )
        self.assertIn("Process_PendingValtanGraphRefresh();", open_all)
        self.assertIn("bHadPendingExactRefresh ?", open_all)
        self.assertIn("m_strCommittedValtanGraphRevision ==", open_all)

        exact_wrapper = function_body(
            self.effect_source,
            "bool_t Client::CEffect_Tool::Refresh_ValtanPatternTreeForRevision(",
        )
        self.assertIn(
            "m_bValtanPatternTreeReloadRetryPending = false;", exact_wrapper
        )


if __name__ == "__main__":
    unittest.main()
