#!/usr/bin/env python3

from __future__ import annotations

from pathlib import Path
import unittest


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
EFFECT_TOOL_CPP = REPOSITORY_ROOT / "Client/Private/Effect_Tool.cpp"
EFFECT_TOOL_HEADER = REPOSITORY_ROOT / "Client/Public/Effect_Tool.h"
EFFECT_CODEC_CPP = REPOSITORY_ROOT / "Client/Private/Effect_DocumentCodec.cpp"


def function_slice(text: str, signature: str, next_signature: str) -> str:
    start = text.index(signature)
    end = text.index(next_signature, start + len(signature))
    return text[start:end]


class EffectToolSavedElementCloneTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.cpp = EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        cls.header = EFFECT_TOOL_HEADER.read_text(encoding="utf-8")
        cls.codec = EFFECT_CODEC_CPP.read_text(encoding="utf-8")

    def test_refresh_stages_parsed_authored_documents_for_tree_projection(self) -> None:
        refresh = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Refresh_DataFiles()",
            "bool_t Client::CEffect_Tool::Try_AppendSavedElementToActiveDocument(",
        )
        for token in (
            "pParsedDocument",
            "bDocumentParseAttempted",
            "strDocumentParseStatus",
        ):
            self.assertIn(token, self.header)
        for token in (
            "EFFECT_DOCUMENT_SOURCE::AUTHORED == eSource",
            "CEffectDocumentCodec::Load(",
            "ParsedDocument.strEffectAssetId != EffectAssetId",
            "std::make_shared<const EFFECT_DOCUMENT_DESC>",
            "m_DataFiles = std::move(Staged)",
        ):
            self.assertIn(token, refresh)
        self.assertLess(
            refresh.index("std::make_shared<const EFFECT_DOCUMENT_DESC>"),
            refresh.index("m_DataFiles = std::move(Staged)"),
        )

    def test_saved_effect_rows_project_effect_family_element_tree(self) -> None:
        render = function_slice(
            self.cpp,
            "void Client::CEffect_Tool::Render_DataFilesWindow()",
            "bool_t Client::CEffect_Tool::Try_CreateDocument()",
        )
        ordered = (
            'ImGui::SeparatorText("Saved Skill Effects")',
            "const auto RenderSavedEffectRow",
            "DataFile.pParsedDocument",
            "EFFECT_AUTHORING_FAMILY::END",
            "Resolve_AuthoringFamily(Element)",
            "FriendlyAuthoringElementLabel(",
            "m_strSelectedDataFileElementId =",
        )
        for token in ordered:
            self.assertIn(token, render)
        positions = [render.index(token) for token in ordered]
        self.assertEqual(positions, sorted(positions))
        self.assertIn('ImGui::Button("Load Saved Effect for Editing")', render)
        self.assertIn('ImGui::Button("Load Saved Element for Editing")', render)

    def test_click_reparses_exact_source_and_never_opens_or_saves_it(self) -> None:
        append = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_AppendSavedElementToActiveDocument(",
            "bool_t Client::CEffect_Tool::Try_SelectProductCue(",
        )
        required = (
            "EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource",
            "EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource",
            "Has_UnappliedDetailDraft()",
            "CEffectDocumentCodec::Load(Path, SourceDocument, Error)",
            "SourceDocument.strEffectAssetId != strExpectedEffectAssetId",
            "iExactSourceCount",
            "1u != iExactSourceCount",
        )
        for token in required:
            self.assertIn(token, append)
        self.assertNotIn("Try_LoadDocumentPath", append)
        self.assertNotIn("Try_LoadDocument(", append)
        self.assertNotIn("Save_Atomic", append)
        self.assertNotIn("Try_Save", append)

    def test_foreign_particle_system_context_is_fail_closed(self) -> None:
        append = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_AppendSavedElementToActiveDocument(",
            "bool_t Client::CEffect_Tool::Try_SelectProductCue(",
        )
        particle_fields = (
            "fUniformScaleMultiplier",
            "fYawOffsetDegrees",
            "fDirectionYawDegrees",
            "fInitialSpeedMultiplier",
        )
        for field in particle_fields:
            self.assertGreaterEqual(append.count(field), 2)
        rejection = append.index(
            "Saved Element copy rejected different Effect-level Particle System controls"
        )
        generic_copy = append.index(
            "CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy("
        )
        self.assertLess(rejection, generic_copy)

    def test_append_allocates_unique_id_and_commits_only_staged_document(self) -> None:
        append = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_AppendSavedElementToActiveDocument(",
            "bool_t Client::CEffect_Tool::Try_SelectProductCue(",
        )
        ordered = (
            "CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(",
            "EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument",
            'const std::string Prefix = "authored.copy."',
            "128u - Prefix.size() - Suffix.size()",
            "std::none_of(Staged.Elements.begin(), Staged.Elements.end()",
            "Staged.Elements.push_back(std::move(AppendedElement))",
            "Try_CommitDocument(std::move(Staged))",
            "m_strSelectedElementId = CopyElementId",
        )
        for token in ordered:
            self.assertIn(token, append)
        positions = [append.index(token) for token in ordered]
        self.assertEqual(positions, sorted(positions))
        self.assertNotIn("m_ActiveDocument =", append)
        self.assertIn("no file was saved", append)

    def test_failed_append_preserves_current_and_success_remains_dirty_unsaved(self) -> None:
        append = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_AppendSavedElementToActiveDocument(",
            "bool_t Client::CEffect_Tool::Try_SelectProductCue(",
        )
        commit = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_CommitDocument(",
            "bool_t Client::CEffect_Tool::Try_SetPreviewFilter(",
        )
        commit_call = "Try_CommitDocument(std::move(Staged))"
        self.assertEqual(append.count(commit_call), 1)
        self.assertNotIn("m_ActiveDocument =", append)
        self.assertNotIn("Save_Atomic", append)
        self.assertNotIn("Try_Save", append)
        self.assertIn("if (!CEffectDocumentCodec::Validate(Staged, Error))", commit)
        self.assertLess(
            commit.index("if (!CEffectDocumentCodec::Validate(Staged, Error))"),
            commit.index("m_ActiveDocument = std::move(Staged)"),
        )
        self.assertIn(
            '"Change rejected; active Document and preview were preserved: "',
            commit,
        )
        self.assertGreaterEqual(commit.count("m_bDocumentDirty = true"), 2)
        self.assertLess(
            append.index(commit_call),
            append.index("m_strSelectedElementId = CopyElementId"),
        )

    def test_effect_asset_transitions_clear_the_saved_element_half_of_selection(self) -> None:
        create_element = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_CreateMeshEffect(",
            "bool_t Client::CEffect_Tool::Try_UseSelectedElementAsAuthoringPreset()",
        )
        save = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_SaveDocument()",
            "bool_t Client::CEffect_Tool::Try_SaveDocumentAs(",
        )
        save_as = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_SaveDocumentAs(",
            "bool_t Client::CEffect_Tool::\n\tTry_SaveSelectedAdapterElementAsGenericAuthoredCopy(",
        )
        load = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_LoadDocumentPathStaged(",
            "bool_t Client::CEffect_Tool::Execute_PendingDocumentLoad(",
        )
        for transition in (create_element, save, save_as, load):
            asset_selection = transition.index(
                "m_strSelectedDataFileAssetId ="
            )
            element_clear = transition.index(
                "m_strSelectedDataFileElementId.clear()",
                asset_selection,
            )
            self.assertLess(asset_selection, element_clear)

    def test_generic_copy_clears_source_ownership_but_preserves_element_payload(self) -> None:
        generic = function_slice(
            self.codec,
            "bool_t Client::CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(",
            "void Client::CEffectDocumentCodec::Record_AuthoringResourceOverride(",
        )
        self.assertIn("Candidate.Elements.push_back(*First)", generic)
        for token in (
            "Lowered.strSourceNode.clear()",
            "Lowered.Renderer = {}",
            "Lowered.ActionCueAttachment = {}",
            "Lowered.TransformInheritance = {}",
            "Lowered.SourceRecipe = {}",
            "Lowered.SourcePresentation = {}",
        ):
            self.assertIn(token, generic)


if __name__ == "__main__":
    unittest.main()
