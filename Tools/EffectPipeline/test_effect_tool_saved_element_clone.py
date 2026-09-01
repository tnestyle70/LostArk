#!/usr/bin/env python3

from __future__ import annotations

from pathlib import Path
import unittest


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
EFFECT_TOOL_CPP = REPOSITORY_ROOT / "Client/Private/Effect_Tool.cpp"
EFFECT_TOOL_HEADER = REPOSITORY_ROOT / "Client/Public/Effect_Tool.h"
EFFECT_CODEC_CPP = REPOSITORY_ROOT / "Client/Private/Effect_DocumentCodec.cpp"
EFFECT_PLAYBACK_CPP = REPOSITORY_ROOT / "Client/Private/Effect_Playback.cpp"


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
        cls.playback = EFFECT_PLAYBACK_CPP.read_text(encoding="utf-8")

    def test_duplicate_button_uses_all_marks_and_reports_the_count(self) -> None:
        start = self.cpp.index("const EFFECT_ELEMENT_DESC* pSelectedForDuplicate")
        render = self.cpp[start : self.cpp.index("pSelectedForSeed", start)]
        for token in (
            "m_MarkedElementIds.empty()",
            "std::all_of(m_ActiveDocument->Elements.begin()",
            "m_MarkedElementIds.contains(Element.strElementId)",
            '"Duplicate " + std::to_string(m_MarkedElementIds.size()) + " Marked"',
            "ImGui::SmallButton(DuplicateLabel.c_str())",
            "then Delete or Duplicate all marked",
            "Copies stay marked and preserve timing",
        ):
            self.assertIn(token, render)

    def test_duplicate_selection_commits_once_before_replacing_marks_or_detail(self) -> None:
        duplicate = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_DuplicateSelectedElement()",
            "bool_t Client::CEffect_Tool::Try_DeleteSelectedElement()",
        )
        for token in (
            "Has_UnappliedDetailDraft()",
            "m_MarkedElementIds.empty() && m_strSelectedElementId.empty()",
            "std::vector<std::string>(m_MarkedElementIds.begin(), m_MarkedElementIds.end())",
            "CEffectDocumentCodec::Build_DuplicatedAuthoredElements(",
            "DuplicateIds.find(m_strSelectedElementId)",
            "CopiedMarks.insert(CopyId)",
            "m_strPreviewIsolationElementId = strPreviousIsolationElement",
        ):
            self.assertIn(token, duplicate)
        commit = "Try_CommitDocument(std::move(Staged))"
        self.assertEqual(duplicate.count(commit), 1)
        for mutation in (
            "Reset_DetailDraft()",
            "m_MarkedElementIds = std::move(CopiedMarks)",
            "m_strSelectedElementId = DuplicateId",
            "Start_WorldPreviewFromBeginning()",
        ):
            self.assertLess(duplicate.index(commit), duplicate.index(mutation))
        self.assertNotIn("m_ActiveDocument =", duplicate)
        self.assertNotIn("Save_Atomic", duplicate)
        self.assertNotIn("Try_Save", duplicate)
        self.assertNotIn("m_MarkedElementIds.clear()", duplicate)

    def test_duplicate_stage_remaps_internal_masters_and_preserves_outputs_on_failure(self) -> None:
        duplicate = function_slice(
            self.codec,
            "bool_t Client::CEffectDocumentCodec::Build_DuplicatedAuthoredElements(",
            "bool_t Client::CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(",
        )
        for token in (
            "Validate(SourceDocument, strOutError)",
            "!Targets.insert(ElementId).second",
            "UsedIds.contains(DuplicateId)",
            "UsedIds.insert(DuplicateId)",
            "DuplicateIds.size() != Targets.size()",
            "EFFECT_ELEMENT_DESC Duplicate = Element",
            "Resolve_EffectPortableOriginElementId(Element)",
            "Duplicate.SourcePresentation = {}",
            "MasterCopy = DuplicateIds.find(",
            "Duplicate.TransformInheritance.strMasterElementId = MasterCopy->second",
        ):
            self.assertIn(token, duplicate)
        commit = "Validate(Staged, strOutError)"
        for output in (
            "InOutDocument = std::move(Staged)",
            "OutDuplicatedElementIds = std::move(DuplicateIds)",
        ):
            self.assertLess(duplicate.index(commit), duplicate.index(output))
        self.assertNotIn(".fStartDelaySeconds =", duplicate)

    def test_refresh_indexes_metadata_and_defers_exact_document_decode(self) -> None:
        refresh = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Refresh_DataFiles()",
            "bool_t Client::CEffect_Tool::Ensure_DataFileDocumentParsed(",
        )
        for token in (
            "pParsedDocument",
            "bDocumentParseAttempted",
            "strDocumentParseStatus",
        ):
            self.assertIn(token, self.header)
        for token in (
            "EFFECT_DOCUMENT_SOURCE::AUTHORED == eSource",
            '"Metadata indexed; Open or Play decodes this exact document on demand."',
            "Refresh_DirectAuthoredEditableIndex(Staged)",
            "m_DataFiles = std::move(Staged)",
        ):
            self.assertIn(token, refresh)
        self.assertNotIn("CEffectDocumentCodec::Load(", refresh)

        explicit_load = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_LoadDocumentPathStaged(",
            "bool_t Client::CEffect_Tool::Execute_PendingDocumentLoad(",
        )
        self.assertIn("CEffectDocumentCodec::Load(Path, Staged, Error)", explicit_load)
        self.assertIn("Staged.strEffectAssetId != strSelectionId", explicit_load)

    def test_expanded_saved_effect_lazily_loads_physical_element_tree(self) -> None:
        ensure = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Ensure_DataFileDocumentParsed(",
            "bool_t Client::CEffect_Tool::Try_AppendSavedElementToActiveDocument(",
        )
        ordered = (
            "Entry.bDocumentParseAttempted = true",
            "CEffectDocumentCodec::Load(Entry.Path, Staged, Error)",
            "Staged.strEffectAssetId != Entry.strAssetId",
            "std::make_shared<const EFFECT_DOCUMENT_DESC>(std::move(Staged))",
        )
        for token in ordered:
            self.assertIn(token, ensure)
        self.assertEqual(
            [ensure.index(token) for token in ordered],
            sorted(ensure.index(token) for token in ordered),
        )
        self.assertIn("if (Entry.bDocumentParseAttempted)", ensure)

        render = function_slice(
            self.cpp,
            "void Client::CEffect_Tool::Render_DataFilesWindow()",
            "bool_t Client::CEffect_Tool::Try_CreateDocument()",
        )
        self.assertLess(
            render.index("if (!bEffectOpen)"),
            render.index("Ensure_DataFileDocumentParsed(DataFile)"),
        )
        self.assertLess(
            render.index("Ensure_DataFileDocumentParsed(DataFile)"),
            render.index("if (nullptr == DataFile.pParsedDocument)"),
        )
        self.assertIn('RowLabel += "###saved-" + DataFile.strAssetId', render)
        self.assertNotIn('RowLabel += "##saved-" + DataFile.strAssetId', render)

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
            "void Client::CEffect_Tool::Reset_BufferedComboAudition()",
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
            "void Client::CEffect_Tool::Reset_BufferedComboAudition()",
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
        portable_copy = append.index(
            "CEffectDocumentCodec::Build_PortableAuthoredElementStartingCopy("
        )
        self.assertLess(rejection, portable_copy)
        for token in (
            "bTargetUsesParticleSystem",
            "bUsesParticleSystem && bTargetUsesParticleSystem",
            "bUsesParticleSystem && !bTargetUsesParticleSystem",
            "Staged.ParticleSystem = SourceSystem",
        ):
            self.assertIn(token, append)

    def test_tool_delegates_saved_element_portability_before_commit(self) -> None:
        append = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_AppendSavedElementToActiveDocument(",
            "void Client::CEffect_Tool::Reset_BufferedComboAudition()",
        )
        required = (
            "CEffectDocumentCodec::Build_PortableAuthoredElementStartingCopy(",
            "PortableCopy.Elements.size()",
            '"Saved Element copy rejected: " + Error',
        )
        for token in required:
            self.assertIn(token, append)
        ordered = (
            "CEffectDocumentCodec::Build_PortableAuthoredElementStartingCopy(",
            "EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument",
            "Staged.Elements.push_back(std::move(AppendedElement))",
            "Try_CommitDocument(std::move(Staged))",
        )
        positions = [append.index(token) for token in ordered]
        self.assertEqual(positions, sorted(positions))

    def test_append_allocates_unique_id_and_commits_only_staged_document(self) -> None:
        append = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_AppendSavedElementToActiveDocument(",
            "void Client::CEffect_Tool::Reset_BufferedComboAudition()",
        )
        ordered = (
            "CEffectDocumentCodec::Build_PortableAuthoredElementStartingCopy(",
            "EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument",
            'const std::string Prefix = "authored.copy."',
            "128u - Prefix.size() - Suffix.size()",
            "std::none_of(Staged.Elements.begin(), Staged.Elements.end()",
            "Staged.Elements.push_back(std::move(AppendedElement))",
            "Try_CommitDocument(std::move(Staged))",
            "m_strSelectedElementId = CopyElementId",
            "if (m_bActiveDocumentDrawable)",
            "Start_WorldPreviewFromBeginning();",
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
            "void Client::CEffect_Tool::Reset_BufferedComboAudition()",
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

    def test_successful_append_restarts_player_and_valtan_preview_from_zero(self) -> None:
        append = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_AppendSavedElementToActiveDocument(",
            "void Client::CEffect_Tool::Reset_BufferedComboAudition()",
        )
        restart = function_slice(
            self.cpp,
            "void Client::CEffect_Tool::Start_WorldPreviewFromBeginning()",
            "void Client::CEffect_Tool::Synchronize_LoadedSkillPreview()",
        )
        self.assertEqual(1, append.count("Start_WorldPreviewFromBeginning();"))
        self.assertLess(
            append.index("Try_CommitDocument(std::move(Staged))"),
            append.index("Start_WorldPreviewFromBeginning();"),
        )
        self.assertLess(
            append.index("m_strSelectedElementId = CopyElementId"),
            append.index("Start_WorldPreviewFromBeginning();"),
        )
        valtan_begin = restart.index(
            "if (m_bValtanBossPatternTransformHistoryRequired)"
        )
        generic_begin = restart.index("float4x4_t TargetRoot{};", valtan_begin)
        valtan_restart = restart[valtan_begin:generic_begin]
        ordered_valtan_restart = (
            "pObject->Reset();",
            "Seek_ValtanBossPatternTransformHistory(",
            "pObject->Set_Visible(true);",
        )
        for token in ordered_valtan_restart:
            self.assertIn(token, valtan_restart)
        self.assertEqual(
            sorted(valtan_restart.index(token) for token in ordered_valtan_restart),
            [valtan_restart.index(token) for token in ordered_valtan_restart],
        )
        self.assertGreater(
            restart.index("Resolve_PreviewRoot(TargetRoot)", generic_begin),
            valtan_begin,
        )
        history_seek = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Seek_ValtanBossPatternTransformHistory(",
            "void Client::CEffect_Tool::Reset_ValtanBossPatternTransformHistory()",
        )
        self.assertIn(
            "pObject->Set_SampleTimeWithTransformHistory(", history_seek
        )

    def test_append_stages_complete_scope_and_rebinds_current_document_timeline(self) -> None:
        append = function_slice(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_AppendSavedElementToActiveDocument(",
            "void Client::CEffect_Tool::Reset_BufferedComboAudition()",
        )
        commit = "Try_CommitDocument(std::move(Staged))"
        scope_tokens = (
            "const EFFECT_PREVIEW_FILTER ePreviousPreviewFilter",
            "const std::string strPreviousIsolationElement",
            "const std::string strPreviousIsolationGroup",
            "const std::string strPreviousIsolationModelCue",
            "const EFFECT_AUTHORING_FAMILY ePreviousIsolationAuthoringFamily",
            "m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE",
            "m_strPreviewIsolationElementId.clear()",
            "m_strPreviewIsolationGroupId.clear()",
            "m_strPreviewIsolationModelCueId.clear()",
            "m_ePreviewIsolationAuthoringFamily = EFFECT_AUTHORING_FAMILY::END",
        )
        for token in scope_tokens:
            self.assertIn(token, append)
            self.assertLess(append.index(token), append.index(commit))
        rollback = append[append.index(f"if (!{commit})") :]
        for token in (
            "m_ePreviewFilter = ePreviousPreviewFilter",
            "m_strPreviewIsolationElementId = strPreviousIsolationElement",
            "m_strPreviewIsolationGroupId = strPreviousIsolationGroup",
            "m_strPreviewIsolationModelCueId = strPreviousIsolationModelCue",
            "ePreviousIsolationAuthoringFamily",
        ):
            self.assertIn(token, rollback)
        ordered_success = (
            commit,
            "bForeignPlayerProduct",
            "bForeignValtanProduct",
            "m_SourcePreviewDocument.reset()",
            "m_strSelectedElementId = CopyElementId",
            "Recalculate_PreviewDuration(*m_ActiveDocument)",
            "Synchronize_LoadedSkillPreview()",
            "Start_WorldPreviewFromBeginning()",
        )
        positions = [append.index(token) for token in ordered_success]
        self.assertEqual(positions, sorted(positions))

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
            "bool_t Client::CEffectDocumentCodec::Build_PortableAuthoredElementStartingCopy(",
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
        emission_fallback = (
            "Lowered.eKind == EFFECT_ELEMENT_KIND::PARTICLE",
            "Lowered.Detail.Particle.fSpawnRatePerSecond <= 0.f",
            "0u == Lowered.Detail.Particle.iBurstCount",
            "Lowered.Detail.Particle.iMaxParticles = (std::max)(",
            "Lowered.Detail.Particle.iBurstCount = 1u;",
        )
        for token in emission_fallback:
            self.assertIn(token, generic)
        recipe_clear = generic.index("Lowered.SourceRecipe = {}")
        self.assertLess(
            recipe_clear,
            generic.index(emission_fallback[0], recipe_clear),
        )
        size_normalization = (
            'Lowered.SourceRecipe.strRendererShape == "mesh"',
            "std::string_view SourceClass = Module.strClassName",
            'SourceClass.starts_with("efparticlemodule")',
            'SourceClass.ends_with("_seeded")',
            'SourceClass != "particlemodulesize"',
            'Distribution.strPropertyPath == "startsize"',
            "iSourceStartSizeCandidateCount",
            "Distribution.iOperation == 1u",
            "CEffectDistribution::Validate(",
            "CEffectDistribution::Evaluate(",
            "bLegacyGeometryScaledSize",
            "1.f / fModelPreScale",
            "Lowered.Detail.Particle.vStartSize.x *= fDimensionlessSizeScale",
            "Lowered.Detail.Particle.vEndSize.y *= fDimensionlessSizeScale",
            "Lowered.Detail.Particle.SourceScale.fSize",
            "Lowered.Detail.Particle.vStartSize.x *= fSourceSizeScale",
            "Lowered.Detail.Particle.SourceScale.fSize = 1.f",
        )
        for token in size_normalization:
            self.assertIn(token, generic)
        self.assertLess(
            generic.index('Distribution.strPropertyPath == "startsize"'),
            generic.index("Lowered.SourceRecipe = {}"),
        )
        self.assertLess(
            generic.index("Lowered.SourceRecipe = {}"),
            generic.index("bLegacyGeometryScaledSize"),
        )

    def test_portable_copy_restores_exact_detail_and_family_carriers(self) -> None:
        portable = function_slice(
            self.codec,
            "bool_t Client::CEffectDocumentCodec::Build_PortableAuthoredElementStartingCopy(",
            "void Client::CEffectDocumentCodec::Record_AuthoringResourceOverride(",
        )
        required = (
            "SourceDocument.bSourceContract",
            "!Source->bVisible",
            "Source->eKind == EFFECT_ELEMENT_KIND::TRAIL",
            "Source->TransformInheritance.bEnabled",
            "Source->SourcePresentation.bEnabled",
            "Source->ActionCueAttachment.bFollow",
            "Candidate.Elements.push_back(*Source)",
            "Portable.Renderer = {}",
            "Portable.TransformInheritance = {}",
            "Portable.SourceRecipe = {}",
            "Portable.SourcePresentation = {}",
            "Portable.Detail = Source->Detail",
            "Portable.ActionCueAttachment = Source->ActionCueAttachment",
            'AuthoredCopyPrefix = "authored-copy:"',
            "Source->strSourceNode.starts_with(AuthoredCopyPrefix)",
            "std::string(AuthoredCopyPrefix) + Source->strElementId",
            "Apply_PortableAuthoredParticleRuntimeCarrier(",
            "Apply_PortableAuthoredDecalRuntimeCarrier(",
            "Portable.SourceRecipe.fEmitterDelaySeconds =",
            "Source->SourceRecipe.fEmitterDelaySeconds",
            "Validate(Staged, strOutError)",
            "Serialize(Staged) != Canonical",
        )
        for token in required:
            self.assertIn(token, portable)
        self.assertLess(
            portable.index("Portable.Detail = Source->Detail"),
            portable.index("Apply_PortableAuthoredParticleRuntimeCarrier("),
        )
        self.assertLess(
            portable.index("Apply_PortableAuthoredParticleRuntimeCarrier("),
            portable.index("Portable.SourceRecipe.fEmitterDelaySeconds ="),
        )
        self.assertNotIn("OwnerNamespace", portable)

    def test_portable_copy_fails_closed_on_nonportable_visibility_and_history(self) -> None:
        portable = function_slice(
            self.codec,
            "bool_t Client::CEffectDocumentCodec::Build_PortableAuthoredElementStartingCopy(",
            "void Client::CEffectDocumentCodec::Record_AuthoringResourceOverride(",
        )
        for token in (
            "cannot turn an invisible source Element into a visible occurrence",
            "cannot detach Trail transform history",
            "cannot detach a FOLLOW attachment from its owner animation history",
            "load the complete Effect instead",
        ):
            self.assertIn(token, portable)

        carrier = function_slice(
            self.codec,
            "bool_t ApplyPortableAuthoredEmitterRuntimeCarrier(",
            "bool_t Client::CEffectDocumentCodec::\n\tApply_PortableAuthoredParticleRuntimeCarrier(",
        )
        self.assertIn("HasPortableAuthoredAutonomousEmission(Staged)", carrier)
        self.assertIn("no autonomous positive Burst or Rate", carrier)
        self.assertIn("SpawnPerUnit/event/history-only emitters", carrier)

        autonomous = function_slice(
            self.codec,
            "bool_t HasPortableAuthoredAutonomousEmission(",
            "bool_t IsPortableVectorFieldAssetId(",
        )
        for token in (
            "Burst.iCountMaximum > 0u",
            'FindDistribution("rate")',
            'FindDistribution("ratescale")',
            "IsPortableNullCdoDistribution(*pRateScale)",
            "CEffectDistribution::Evaluate(",
        ):
            self.assertIn(token, autonomous)

    def test_portable_decal_and_random_identity_are_runtime_admitted(self) -> None:
        for token in (
            '"particlemoduletypedatadecal"',
            'Module.strClassName != "efparticlemoduletypedatadecal"',
            "Apply_PortableAuthoredDecalRuntimeCarrier(",
            "bDecalCardinalityValid",
        ):
            self.assertIn(token, self.codec)
        for token in (
            "bool_t Is_PortableAuthoredDecalCarrier(",
            "bool_t Is_PortableAuthoredEmitterCarrier(",
            "bSourceVisualProgramActive ||",
            "Is_PortableAuthoredDecalCarrier(Element)",
            "Hash_RuntimeRandomIdentity(Element)",
            "Resolve_EffectPortableOriginElementId(Element)",
        ):
            self.assertIn(token, self.playback)
        self.assertEqual(self.playback.count("Hash_RuntimeRandomIdentity(Element)"), 3)

    def test_portable_origin_identity_reaches_exact_material_contracts(self) -> None:
        header = (REPOSITORY_ROOT / "Client/Public/Effect_AuthoringDocument.h").read_text(
            encoding="utf-8-sig"
        )
        renderer = (REPOSITORY_ROOT / "Client/Private/Effect_DocumentRenderer.cpp").read_text(
            encoding="utf-8-sig"
        )
        material = (REPOSITORY_ROOT / "Client/Public/Effect_MaterialTemplate.h").read_text(
            encoding="utf-8-sig"
        )
        for token in (
            "EFFECT_PORTABLE_AUTHORED_COPY_PREFIX",
            "Resolve_EffectPortableOriginElementId(",
            "Is_EffectSourceIdentityOrPortableCopy(",
        ):
            self.assertIn(token, header)
        self.assertGreaterEqual(
            renderer.count("Resolve_EffectPortableOriginElementId(Element)"), 5
        )
        self.assertGreaterEqual(
            renderer.count("Is_EffectSourceIdentityOrPortableCopy("), 5
        )
        self.assertIn("Is_EffectSourceIdentityOrPortableCopy(Element,", material)
        self.assertIn("WARLORD_CHAIN_ORIGIN_ELEMENT_IDS", self.playback)
        self.assertIn(
            "Resolve_EffectPortableOriginElementId(Element)", self.codec
        )


if __name__ == "__main__":
    unittest.main()
