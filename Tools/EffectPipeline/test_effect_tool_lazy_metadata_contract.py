#!/usr/bin/env python3
"""Focused source contract for Effect Tool lazy document admission."""

from __future__ import annotations

import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SOURCE_PATH = ROOT / "Client" / "Private" / "Effect_Tool.cpp"
SOURCE = SOURCE_PATH.read_text(encoding="utf-8")


def function_body(signature: str) -> str:
    """Return one C++ function body while ignoring braces in comments/strings."""

    start = SOURCE.index(signature)
    opening = SOURCE.index("{", start)
    depth = 0
    state = "code"
    index = opening
    while index < len(SOURCE):
        char = SOURCE[index]
        next_char = SOURCE[index + 1] if index + 1 < len(SOURCE) else ""
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
                    return SOURCE[opening : index + 1]
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


class EffectToolLazyMetadataContractTests(unittest.TestCase):
    def test_first_render_bootstrap_is_metadata_only(self) -> None:
        body = function_body(
            "void Client::CEffect_Tool::Initialize_CatalogMetadataView()"
        )
        self.assertNotIn("Refresh_DataFiles(", body)
        self.assertNotIn("Refresh_AllEffects(", body)
        self.assertNotIn("CEffectDocumentCodec::Load", body)
        self.assertIn("document decode is deferred until Open or Play", body)

    def test_valtan_workspace_open_does_not_decode_the_corpus(self) -> None:
        body = function_body(
            "bool_t Client::CEffect_Tool::Open_ValtanAllEffectsWorkspace()"
        )
        self.assertIn("Initialize_CatalogMetadataView();", body)
        self.assertNotIn("Refresh_DataFiles(", body)
        self.assertNotIn("Refresh_AllEffects(", body)
        self.assertNotIn("CEffectDocumentCodec::Load", body)

    def test_product_deep_link_decodes_only_after_exact_tuple_resolution(self) -> None:
        body = function_body(
            "bool_t Client::CEffect_Tool::Open_ValtanProductEffect("
        )
        self.assertIn("Initialize_CatalogMetadataView();", body)
        self.assertNotIn("Refresh_DataFiles(", body)
        self.assertNotIn("Refresh_AllEffects(", body)
        self.assertLess(
            body.index("pCue->strEffectAssetId != Request.strEffectAssetId"),
            body.index("Try_OpenValtanStandaloneEffect("),
        )

    def test_pattern_binding_refresh_joins_metadata_without_document_decode(self) -> None:
        body = function_body(
            "bool_t Client::CEffect_Tool::Refresh_ValtanPatternAuthoringEffects()"
        )
        self.assertNotIn("CEffectDocumentCodec::Load", body)
        self.assertIn("m_DirectAuthoredEditableEntries.find(", body)
        self.assertIn("bExactAuthoredMetadata", body)

    def test_manual_data_refresh_indexes_metadata_without_document_decode(self) -> None:
        body = function_body("bool_t Client::CEffect_Tool::Refresh_DataFiles()")
        self.assertNotIn("CEffectDocumentCodec::Load", body)
        self.assertIn(
            "Open or Play decodes this exact document on demand", body
        )
        self.assertIn("Refresh_DirectAuthoredEditableIndex(Staged)", body)

    def test_exact_selected_load_path_still_decodes_one_staged_document(self) -> None:
        body = function_body(
            "bool_t Client::CEffect_Tool::Try_LoadDocumentPathStaged("
        )
        self.assertIn("EFFECT_DOCUMENT_DESC Staged", body)
        self.assertIn("CEffectDocumentCodec::Load(Path, Staged, Error)", body)
        self.assertIn("The previous Current Effect was preserved", body)

    def test_closed_pattern_node_skips_runtime_row_projection(self) -> None:
        body = function_body(
            "void Client::CEffect_Tool::Render_ValtanPatternNode("
        )
        tree = body.index("const bool_t bPatternOpen = ImGui::TreeNodeEx")
        closed_return = body.index("if (!bPatternOpen)")
        projection = body.index(
            "std::vector<RUNTIME_VALTAN_EFFECT_ROW> RuntimeRows"
        )
        self.assertLess(tree, closed_return)
        self.assertLess(closed_return, projection)

    def test_closed_independent_node_skips_owner_timeline_projection(self) -> None:
        body = function_body(
            "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode("
        )
        tree = body.index("const bool_t bOpen = ImGui::TreeNodeEx")
        closed_return = body.index("if (!bOpen)")
        projection = body.index("Build_ValtanAuthoringTimeline(")
        self.assertLess(tree, closed_return)
        self.assertLess(closed_return, projection)

    def test_failed_direct_index_stage_preserves_admitted_cache(self) -> None:
        body = function_body(
            "bool_t Client::CEffect_Tool::Refresh_DirectAuthoredEditableIndex("
        )
        build = body.index("CEffectDirectAuthoredSourceIndex::Build(")
        preserve = body.index("return PreservePrevious(SourceIndexStatus)")
        commit = body.index(
            "m_DirectAuthoredEditableEntries = std::move(StagedEntries)"
        )
        self.assertLess(build, preserve)
        self.assertLess(preserve, commit)


if __name__ == "__main__":
    unittest.main()
