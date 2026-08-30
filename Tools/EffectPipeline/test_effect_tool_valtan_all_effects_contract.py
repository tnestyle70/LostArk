from __future__ import annotations

import json
import re
import unittest
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
EFFECT_TOOL_CPP = REPOSITORY_ROOT / "Client/Private/Effect_Tool.cpp"
EFFECT_TOOL_HEADER = REPOSITORY_ROOT / "Client/Public/Effect_Tool.h"
PATTERN_TREE_CPP = REPOSITORY_ROOT / "Client/Private/ValtanPatternTree.cpp"
CHARACTER_PREVIEW_CPP = (
    REPOSITORY_ROOT / "Client/Private/CharacterPreviewPanel.cpp"
)
OWNERSHIP_CPP = (
    REPOSITORY_ROOT
    / "Client/Private/ValtanPatternAuthoringEffectDocument.cpp"
)
OWNERSHIP_HEADER = (
    REPOSITORY_ROOT
    / "Client/Public/ValtanPatternAuthoringEffectDocument.h"
)
OWNERSHIP_JSON = (
    REPOSITORY_ROOT / "Data/Effects/ValtanPatternAuthoringEffects.json"
)
GAMEPLAY_JSON = REPOSITORY_ROOT / "Data/Valtan/Valtan.gameplay.json"
PATTERN_PRODUCT_JSON = REPOSITORY_ROOT / "Data/Valtan/Valtan.pattern.json"
PRESENTATION_JSON = REPOSITORY_ROOT / "Data/Valtan/Valtan.presentation.json"
PRODUCT_CUES_JSON = (
    REPOSITORY_ROOT
    / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
)
EFFECT_CATALOG_JSON = REPOSITORY_ROOT / "Data/Effects/EffectCatalog.json"
CLIENT_PROJECT = REPOSITORY_ROOT / "Client/Default/Client.vcxproj"
CLIENT_FILTERS = REPOSITORY_ROOT / "Client/Default/Client.vcxproj.filters"

REQUIRED_INDEPENDENT_EFFECT_ASSETS = {
    "valtan.independent-effect.donut-in-out":
        "effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01",
    "valtan.independent-effect.target-axe": "effect.valtan.sky-axe.active",
}



def source_section(source: str, start: str, end: str) -> str:
    start_index = source.index(start)
    end_index = source.index(end, start_index)
    return source[start_index:end_index]


class EffectToolValtanAllEffectsContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.cpp = EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        cls.header = EFFECT_TOOL_HEADER.read_text(encoding="utf-8")
        cls.pattern_tree_cpp = PATTERN_TREE_CPP.read_text(encoding="utf-8")
        cls.character_preview = CHARACTER_PREVIEW_CPP.read_text(
            encoding="utf-8"
        )
        cls.ownership_cpp = OWNERSHIP_CPP.read_text(encoding="utf-8")
        cls.ownership_header = OWNERSHIP_HEADER.read_text(encoding="utf-8")
        cls.gameplay = json.loads(GAMEPLAY_JSON.read_text(encoding="utf-8"))
        cls.pattern_product = json.loads(
            PATTERN_PRODUCT_JSON.read_text(encoding="utf-8")
        )
        cls.presentation = json.loads(
            PRESENTATION_JSON.read_text(encoding="utf-8")
        )
        cls.product_cues = json.loads(
            PRODUCT_CUES_JSON.read_text(encoding="utf-8")
        )
        cls.effect_catalog = json.loads(
            EFFECT_CATALOG_JSON.read_text(encoding="utf-8")
        )

    def test_boss_product_open_obeys_the_existing_unlink_lock(self) -> None:
        opening = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Open_ValtanProductEffect(",
            "void Client::CEffect_Tool::Update(",
        )
        guard = source_section(
            opening,
            "if (m_ValtanPatternProductUnlinkOperation.has_value())",
            "m_bAllEffectsValtanBossSelected = true;",
        )
        self.assertIn("return Reject(", guard)
        self.assertIn("unlink transaction", guard)
        self.assertLess(opening.index(guard), opening.index("Refresh_DataFiles()"))
        self.assertLess(opening.index(guard), opening.index("Refresh_ValtanPatternTree()"))

    def test_boss_product_open_revalidates_identity_before_loading(self) -> None:
        opening = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Open_ValtanProductEffect(",
            "void Client::CEffect_Tool::Update(",
        )
        for field in ("PatternId", "StageId", "CueOccurrenceId", "EffectAssetId"):
            self.assertIn(f"Request.str{field}.empty()", opening)
        for failure in (
            "if (!Refresh_DataFiles())",
            "if (!Refresh_ValtanPatternTree())",
            "!Is_ValtanAllEffectsPattern(*pPattern)",
            "1u != iStageMatchCount",
            "1u != iCueMatchCount",
            "pCue->strPatternId != pPattern->strPatternId",
            "pCue->strStageId != pStage->strStageId",
            "pCue->strActionId != pStage->strActionId",
            "pCue->strEffectAssetId != Request.strEffectAssetId",
        ):
            self.assertIn(failure, opening)
            self.assertLess(
                opening.index(failure),
                opening.index("Resolve_DirectAuthoredEditablePath("),
            )
        self.assertIn("if (nullptr == pPath)", opening)
        self.assertIn("return Reject(std::move(PathStatus));", opening)
        self.assertIn("m_PendingDocumentLoad->Path == *pPath", opening)
        for forbidden in (
            "Discard_ActiveDocument(", "Try_SaveDocument(",
            "m_ActiveDocument =", "strV1EffectAssetId",
        ):
            self.assertNotIn(forbidden, opening)

    def test_boss_product_open_keeps_stage_clock_and_clip_preview_distinct(self) -> None:
        opening = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Open_ValtanProductEffect(",
            "void Client::CEffect_Tool::Update(",
        )
        standalone = source_section(opening, "if (pCue->bUsesStageClock)", "\n\telse\n")
        self.assertIn("Try_OpenValtanStandaloneEffect(", standalone)
        self.assertNotIn("Build_ValtanProductPreview(", standalone)
        self.assertIn("1u != iClipMatchCount", opening)
        for preview_path in ("NORMAL", "WALL_GROGGY", "PART_BREAK"):
            self.assertIn(f"VALTAN_PATTERN_PREVIEW_PATH::{preview_path}", opening)
        self.assertIn("Build_ValtanProductPreview(", opening)
        self.assertIn("return Reject(std::move(PreviewError));", opening)
        self.assertIn(
            "Try_OpenValtanAuthoredEffect(*pPath,Request.strEffectAssetId,Preview,false)",
            re.sub(r"\s+", "", opening),
        )
        for forbidden in ("Try_Play", "Stage_WorldPreview(", "Play_ValtanStageSequence("):
            self.assertNotIn(forbidden, opening)
        helper = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_OpenValtanAuthoredEffect(\n"
            "\tconst std::filesystem::path& Path,\n"
            "\tconst std::string& strEffectAssetId,\n"
            "\tconst VALTAN_PRODUCT_PREVIEW& Preview,",
            "bool_t Client::CEffect_Tool::Refresh_ValtanPatternTree()",
        )
        self.assertIn("m_PendingDocumentLoad->ValtanProductPreview = Preview;", helper)
        self.assertLess(helper.index("Try_LoadDocumentPath("), helper.index("Play_ValtanProductCue(Preview)"))
        self.assertIn("if (bAnimationReady && !bQueuePlayCompleteAfterLoad)", helper)
        self.assertIn("Set_SynchronizedAnimationPaused(true);", helper)

    def test_preview_labels_distinguish_effect_time_and_animation_time(self) -> None:
        self.assertIn("Timeline %.3f / %.3f s", self.cpp)
        self.assertIn("Effect local: %.3f s | Effect 0 = Timeline %.3f s", self.cpp)
        timing = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_TimingDetail(",
            "void Client::CEffect_Tool::Render_SizeDetail(",
        )
        for token in (
            "Start Delay is relative to Effect 0",
            "SourceRecipe.fEmitterDelaySeconds",
            "Detail.Timing.fStartDelaySeconds + fNativeEmitterDelay",
            "Resolve_EffectTimelineTime(fEmitterStart)",
            "Native emitter delay: +%.3f s",
            "Scaling Lerp finishes before the native emitter starts",
            "Particle lifetime: %.3f - %.3f s after spawn",
        ):
            self.assertIn(token, timing)
        self.assertNotIn("Start Delay positions this Element in the clip", timing)

    def test_valtan_preview_duration_includes_natural_cue_tail(self) -> None:
        duration = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Recalculate_PreviewDuration(\n    const EFFECT_DOCUMENT_DESC& Document)",
            "bool_t Client::CEffect_Tool::Has_UnsavedWork() const",
        )
        valtan = source_section(
            duration,
            "else if (m_ValtanProductPreview.has_value())",
            "else if (0u != m_iValtanWorldOwnerStageDurationMs)",
        )
        for token in (
            "CActionPresentationTimeline::Resolve_CuePreviewDuration(",
            "m_ValtanProductPreview->iOwningClipTimelineOffsetMs",
            "m_ValtanProductPreview->Cue.bHasSourceEnd",
            "m_fPreviewDurationSeconds, fEffectDurationSeconds",
            "m_fPreviewDurationSeconds = fCuePreviewDuration",
            "rejected invalid cue timing",
        ):
            self.assertIn(token, valtan)

    def test_lerp_restart_waits_for_successful_draft_staging(self) -> None:
        lerp = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_LerpDetail(",
            "void Client::CEffect_Tool::Render_AssemblyHierarchy(",
        )
        self.assertIn("m_bDetailDraftPreviewRestartRequested = true;", lerp)
        self.assertNotIn("m_fPreviewTimeSeconds =", lerp)
        self.assertNotIn("m_bPreviewPlaying =", lerp)

        stage = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Stage_DetailDraftPreview()",
            "bool_t Client::CEffect_Tool::Stage_ModelCueDraftPreview()",
        )
        failure = source_section(
            stage,
            "if (!Stage_WorldPreview(Staged))",
            "if (m_bDetailDraftPreviewRestartRequested)",
        )
        for token in (
            "m_fPreviewDurationSeconds = fPreviousDuration;",
            "m_fPreviewTimeSeconds = fPreviousTime;",
            "return false;",
        ):
            self.assertIn(token, failure)
        self.assertNotIn("Start_WorldPreviewFromBeginning", failure)
        self.assertLess(
            stage.index("m_bDetailDraftPreviewRestartRequested = false;"),
            stage.index("Start_WorldPreviewFromBeginning();"),
        )
        detail = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_EffectDetailWindow()",
            "void Client::CEffect_Tool::Render_SelectedVisualProgramEvidence()",
        )
        self.assertLess(
            detail.index("Render_Detail(*m_DetailDraft, bChanged);"),
            detail.index("Stage_DetailDraftPreview();"),
        )
        reset = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Reset_DetailDraft()",
            "void Client::CEffect_Tool::Recalculate_PreviewDuration()",
        )
        self.assertIn("m_bDetailDraftPreviewRestartRequested = false;", reset)
        revert = source_section(
            detail, 'if (ImGui::Button("Revert Detail"))', "ImGui::EndDisabled();"
        )
        self.assertIn("m_bDetailDraftPreviewRestartRequested = false;", revert)

        restart = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Start_WorldPreviewFromBeginning()",
            "void Client::CEffect_Tool::Synchronize_LoadedSkillPreview()",
        )
        self.assertIn("m_fPreviewTimeSeconds = 0.f;", restart)
        self.assertIn("Restart_SynchronizedAnimationSequence();", restart)
        self.assertIn("Resolve_EffectSampleTime(m_fPreviewTimeSeconds)", restart)

    def test_particle_audition_restarts_animation_only_after_staging(self) -> None:
        audition = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_AuditionParticleSystem()",
            "bool_t Client::CEffect_Tool::Try_AuditionSelectedElement()",
        )
        failure = source_section(
            audition,
            "if (!Stage_WorldPreview(Staged))",
            "Restart_SynchronizedAnimationSequence();",
        )
        for token in (
            "m_ePreviewFilter = ePreviousFilter;",
            "m_fPreviewTimeSeconds = fPreviousTime;",
            "m_fPreviewDurationSeconds = fPreviousDuration;",
            "m_bPreviewPlaying = bPreviousPlaying;",
            "return false;",
        ):
            self.assertIn(token, failure)
        self.assertLess(
            audition.index("Restart_SynchronizedAnimationSequence();"),
            audition.index("pObject->Reset();"),
        )
        self.assertIn("Resolve_EffectSampleTime(m_fPreviewTimeSeconds)", audition)
        self.assertIn("Set_SynchronizedAnimationPaused(true);", audition)
        animation_restart = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Restart_SynchronizedAnimationSequence()",
            "void Client::CEffect_Tool::Seek_SynchronizedAnimationSequence(",
        )
        self.assertIn("m_iSynchronizedAnimationClipIndex = 0u;", animation_restart)
        self.assertIn("m_iSynchronizedAnimationLoopEpoch = 0u;", animation_restart)
        self.assertIn("Start_SynchronizedAnimationClip(0u, false)", animation_restart)

    def test_uv_keyframe_restart_keeps_the_previous_pause_intent(self) -> None:
        uv = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_UVKeyframes(",
            "void Client::CEffect_Tool::Render_TimingDetail(",
        )
        committed = uv[uv.index("if (bCommitted)"):]
        self.assertLess(
            uv.index("Try_CommitDocument(std::move(Staged))"),
            uv.index("Start_WorldPreviewFromBeginning();"),
        )
        self.assertLess(
            committed.index("const bool_t bWasPlaying = m_bPreviewPlaying;"),
            committed.index("Start_WorldPreviewFromBeginning();"),
        )
        paused = committed[committed.index("if (!bWasPlaying)"):]
        self.assertIn("m_bPreviewPlaying = false;", paused)
        self.assertIn("Set_SynchronizedAnimationPaused(true);", paused)
        self.assertNotIn("m_fPreviewTimeSeconds =", uv)

    def test_selected_audition_commits_play_intent_before_animation_seek(self) -> None:
        audition = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_AuditionSelectedElement()",
            "bool_t Client::CEffect_Tool::Refresh_ResourceCatalog()",
        )
        failure = source_section(
            audition,
            "if (!Stage_WorldPreview(AuditionDocument))",
            "m_bPreviewPlaying = true;",
        )
        for token in (
            "m_fPreviewTimeSeconds = fPreviousTime;",
            "m_bPreviewPlaying = bPreviousPlaying;",
            "return false;",
        ):
            self.assertIn(token, failure)
        seek = "Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds);"
        self.assertLess(audition.index("m_bPreviewPlaying = true;"), audition.index(seek))
        self.assertEqual(1, audition.count("m_bPreviewPlaying = true;"))
        self.assertIn("Resolve_EffectTimelineTime(", audition)
        self.assertIn("Resolve_EffectSampleTime(m_fPreviewTimeSeconds)", audition)
        self.assertIn("Set_SynchronizedAnimationPaused(true);", audition)
        sampler = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Seek_SynchronizedAnimationSequence(",
            "void Client::CEffect_Tool::Set_SynchronizedAnimationPaused(",
        )
        self.assertIn("CActionPresentationTimeline::Resolve_PreviewSequenceSample(", sampler)
        self.assertIn("Sample.iClipIndex, bHoldingEndPose || !m_bPreviewPlaying", sampler)
        self.assertLess(sampler.index("Resolve_PreviewSequenceSample("), sampler.index("Start_SynchronizedAnimationClip("))
        self.assertIn("Set_AnimPaused(bHoldingEndPose || !m_bPreviewPlaying)", sampler)

    def test_source_size_help_matches_the_existing_positive_scaling_controls(self) -> None:
        size_start = self.cpp.index("void Client::CEffect_Tool::Render_SizeDetail(")
        size_end = self.cpp.index("void Client::CEffect_Tool::", size_start + 1)
        size = self.cpp[size_start:size_end]
        self.assertRegex(
            size,
            r'DragFloat\("Size x", &Tuning\.fSize,\s*0\.01f, 0\.01f, 16\.f',
        )
        self.assertIn("Size x is a constant source multiplier", size)
        self.assertIn("zero is invalid", size)
        self.assertIn("editor minimum 0.001", size)
        self.assertIn("Linear Lerp > Lerp Scaling", size)
        self.assertIn("longer than the native emitter delay", size)
        transform = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_TransformDetail(",
            "void Client::CEffect_Tool::Render_ColorDetail(",
        )
        self.assertIn('"Scaling", Detail.Transform.vScale, 0.01f, 0.001f, 100.f', transform)
        self.assertIn('RenderLerpToggle("Lerp Scaling", Lerp.bScale);', self.cpp)
        self.assertIn('"Scaling End", Lerp.vEndScale, 0.01f, 0.001f, 100.f', self.cpp)

    def test_independent_effects_and_authored_pattern_inventory_have_distinct_owners(self) -> None:
        self.assertNotIn("VALTAN_ALL_EFFECTS_INDEPENDENT_EFFECT_IDS", self.cpp)
        authored_ids = [row["patternId"] for row in self.gameplay["patterns"]]
        manual_ids = [row["patternId"] for row in self.gameplay["decisionModel"]["manualAuditions"]]
        self.assertEqual(len(authored_ids), len(set(authored_ids)))
        self.assertEqual(len(manual_ids), len(set(manual_ids)))
        self.assertLessEqual(set(manual_ids), set(authored_ids))
        self.assertTrue({
            "VALTAN_GHOST_RESPAWN_AUDITION",
            "VALTAN_GHOST_DEATH_AUDITION",
        }.issubset(set(manual_ids)))
        self.assertNotIn("VALTAN_TOOL_AUDITION_CORE_PATTERN_IDS", self.pattern_tree_cpp)
        self.assertNotIn("exact 27-pattern", self.cpp)
        self.assertNotIn("27 Patterns", self.cpp)

        independent_rows = self.pattern_product["independentEffects"]
        independent = {
            row["independentEffectId"]: row
            for row in independent_rows
        }
        self.assertTrue(independent_rows)
        self.assertEqual(len(independent_rows), len(independent))
        self.assertLessEqual(set(REQUIRED_INDEPENDENT_EFFECT_ASSETS), set(independent))
        for independent_id, effect_asset_id in REQUIRED_INDEPENDENT_EFFECT_ASSETS.items():
            self.assertEqual(
                effect_asset_id, independent[independent_id]["effectAssetId"]
            )
        self.assertIn("VALTAN_FIST_IN_OUT", authored_ids)

    def test_tree_uses_authored_manual_order_and_no_legacy_fallback(self) -> None:
        tree = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
            "void Client::CEffect_Tool::Render_AllEffectsWindow()",
        )
        for token in (
            "m_ValtanPatternTree.IndependentEffects",
            "IndependentRows.reserve(m_ValtanPatternTree.IndependentEffects.size())",
            "m_ValtanToolAuditionInventory.CorePatternIds",
            "m_ValtanToolAuditionInventory.AnimatorPatternIds",
            "CORE SERVER PATTERNS",
            "ANIMATOR PATTERNS",
            "PATTERN-OWNED INDEPENDENT EFFECT (",
            "m_ValtanToolAuditionInventory.Get_PatternCount()",
            "m_ValtanToolAuditionInventory.DerivedPatternIds",
            "no legacy or replacement row was substituted",
        ):
            self.assertIn(token, tree)
        for fixed_surface in (
            "2u != IndependentRows.size()",
            "%zu/2 Effects",
            "PATTERN-OWNED INDEPENDENT EFFECT (2)",
        ):
            self.assertNotIn(fixed_surface, tree)
        self.assertIn(
            "CValtanPatternTree::Build_PlayablePatternInventory",
            self.cpp,
        )
        for retired_surface in (
            "CounterReactionLayers",
            "iIntroRotationIndex",
            "LegacyRotations",
            "Render_ValtanStageRow",
            'Render_ValtanPatternNode(Pattern, "Gimmick"',
        ):
            self.assertNotIn(retired_surface, tree)

    def test_published_pattern_effects_are_editable_and_keep_server_play(self) -> None:
        pattern = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_ValtanPatternNode(",
            "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
        )
        for token in (
            "m_strSelectedValtanPatternId = Pattern.strPatternId",
            'ImGui::SmallButton("Play Server")',
            "Try_PlayValtanServerPattern(Pattern)",
            "Pattern.Stages",
            "Stage.ProductCues",
            "RUNTIME_VALTAN_EFFECT_ROW",
            "Runtime Product Effects",
            'ImGui::SmallButton("Open Editor")',
            "Build_ValtanProductPreview",
            "Try_OpenValtanAuthoredEffect",
            "Try_PlayValtanSavedUnifiedEffect",
            "Try_OpenValtanSavedReferenceEffect",
            "iReferenceEffectStartMs",
            "Unpublished Pattern Draft",
            "Play Effect + Pattern",
            "Try_OpenValtanPatternDraftEffect",
        ):
            self.assertIn(token, pattern)
        self.assertNotIn("Play Draft Effect Only", pattern)
        self.assertIn(
            "return Row.ProductSources.empty() &&",
            pattern,
            "an independent Effect must still appear below the pattern that consumes it",
        )
        for retired_projection in (
            'Label += " | Effect 1"',
            "Effect | (not created)",
            "DRAFT_ATTACHED##aggregate-effect",
        ):
            self.assertNotIn(retired_projection, pattern)

    def test_pattern_draft_keeps_pattern_identity_and_timeline_through_play(self) -> None:
        create = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_CreateValtanPatternEffect(",
            "void Client::CEffect_Tool::Render_ValtanPatternNode(",
        )
        prepare = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Prepare_ActiveValtanPatternDraftTimeline(",
            "bool_t Client::CEffect_Tool::Try_OpenValtanPatternDraftEffect(",
        )
        open_draft = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_OpenValtanPatternDraftEffect(",
            "bool_t Client::CEffect_Tool::Prepare_ValtanStandaloneEffectTarget(",
        )
        pending = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Execute_PendingDocumentLoad(",
            "bool_t Client::CEffect_Tool::Refresh_AllEffects(",
        )
        restart = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Start_WorldPreviewFromBeginning()",
            "void Client::CEffect_Tool::Synchronize_LoadedSkillPreview()",
        )
        synchronize = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Synchronize_LoadedSkillPreview()",
            "bool_t Client::CEffect_Tool::Start_SynchronizedAnimationClip(",
        )

        for token in (
            "VALTAN_PATTERN_DRAFT",
            "strValtanPatternId",
            "eValtanPatternPreviewPath",
            "m_strActiveValtanPatternDraftId",
            "m_eActiveValtanPatternDraftPreviewPath",
        ):
            self.assertIn(token, self.header)
        for token in (
            "Build_ValtanAuthoringTimeline",
            "Play_ValtanStageSequence",
            "m_iValtanWorldOwnerStageDurationMs = iTimelineDurationMs",
            "m_iValtanReferenceEffectStartMs = 0u",
            "Recalculate_PreviewDuration()",
            "Set_SynchronizedAnimationPaused(bPaused)",
        ):
            self.assertIn(token, prepare)
        for forbidden in (
            "m_ePreviewPivotKind =",
            "m_strPreviewAnchorSlotId.clear()",
        ):
            self.assertNotIn(
                forbidden,
                prepare,
                "Play/re-sync must preserve the Model View authoring pivot",
            )
        for token in (
            "EFFECT_DOCUMENT_PREVIEW_INTENT::VALTAN_PATTERN_DRAFT",
            "bPreserveAuthoringPivot",
            "m_PendingDocumentLoad->strValtanPatternId = Pattern.strPatternId",
            "m_PendingDocumentLoad->eValtanPatternPreviewPath",
            "m_PendingDocumentLoad->bPlayCompleteAfterLoad = bPlayAfterOpen",
            "m_strActiveValtanPatternDraftId = Pattern.strPatternId",
            "if (!bPreserveAuthoringPivot)",
            "m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT",
            "m_strPreviewAnchorSlotId.clear()",
            "Prepare_ActiveValtanPatternDraftTimeline(true)",
            "Try_PlayActiveUnifiedEffect()",
        ):
            self.assertIn(token, open_draft)
        self.assertLess(
            open_draft.index("if (!bAlreadyActive)"),
            open_draft.index("Refresh_UnifiedEffectCache"),
            "an active in-memory draft must not be rejected by the empty disk shell",
        )
        self.assertIn(
            "else if (bPlayAfterOpen && !m_bActiveDocumentDrawable)",
            open_draft,
        )
        for token in (
            "Pending.strValtanPatternId",
            "Pending.eValtanPatternPreviewPath",
            "m_strActiveValtanPatternDraftId = Pending.strValtanPatternId",
            "m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT",
            "m_strPreviewAnchorSlotId.clear()",
            "Prepare_ActiveValtanPatternDraftTimeline(true)",
            "Pending.bPlayCompleteAfterLoad",
            "Try_PlayActiveUnifiedEffect()",
        ):
            self.assertIn(token, pending)
        pending_continuations = pending[
            pending.index("const auto CompleteValtanPreviewPartial"):
        ]
        self.assertLess(
            pending_continuations.index(
                "EFFECT_DOCUMENT_PREVIEW_INTENT::VALTAN_PATTERN_DRAFT"
            ),
            pending_continuations.index(
                "EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT"
            ),
        )
        self.assertIn(
            "Prepare_ActiveValtanPatternDraftTimeline(false)", restart
        )
        self.assertIn(
            "Prepare_ActiveValtanPatternDraftTimeline(true)", synchronize
        )
        self.assertIn("Try_OpenValtanPatternDraftEffect", create)
        self.assertNotIn("Try_OpenValtanStandaloneEffect", create)

    def test_server_play_is_arena_only_and_reuses_typed_audition(self) -> None:
        resolver = source_section(
            self.cpp,
            "const char_t* Resolve_ValtanServerPatternBossPlacement(",
            "const char_t* Tool_PlayerStanceLabel(",
        )
        can_play = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Can_PlayValtanServerPattern(",
            "bool_t Client::CEffect_Tool::Try_PlayValtanServerPattern(",
        )
        play = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_PlayValtanServerPattern(",
            "void Client::CEffect_Tool::Update_ValtanServerPatternAudition()",
        )
        update = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Update_ValtanServerPatternAudition()",
            "bool_t Client::CEffect_Tool::Try_CreateValtanPatternEffect(",
        )
        self.assertIn("LEVEL::VALTAN_ARENA", resolver)
        self.assertIn("boss.valtan.center", self.cpp)
        self.assertNotIn("CHARACTER_SELECT", resolver + can_play + play)
        self.assertIn("CNetworkManager::Get().Is_Connected()", can_play)
        for gate in (
            "CCombatHUDViewModel::Get().Get_Boss()",
            "CCombatHUDViewModel::Get().Get_Player()",
            "Boss.iCurrentHp",
            "Player.iCurrentHp",
            "Player.isCombatReady",
        ):
            self.assertIn(gate, can_play)
        self.assertIn("CValtanPatternAuditionService::Get().Submit", play)
        self.assertIn("VALTAN_EFFECT_TOOL_AUDITION_CONSUMER_ID", play)
        self.assertIn(
            "VALTAN_EFFECT_TOOL_AUDITION_CONSUMER_ID !=",
            update,
            "Effect Tool must not display another consumer's lifecycle as its own",
        )
        for local_preview in (
            "Play_ValtanAuthoringTimeline",
            "Play_ValtanStageSequence",
            "Start_Animation",
            "Clear_ProductCuePreview",
            "Reset_SynchronizedAnimationSequence",
            "Release_WorldPreview",
            "Send_ValtanPatternAuditionById",
            "Try_Consume_ValtanPatternAuditionByIdResult",
            "Request_RevivePlayer",
        ):
            self.assertNotIn(local_preview, play + update)
        self.assertNotIn(
            "CValtanPatternAuditionService::Get().Update();",
            play + update,
        )

    def test_all_authored_pattern_names_are_canonical_korean(self) -> None:
        patterns = {
            row["patternId"]: row for row in self.gameplay["patterns"]
        }
        visible_pattern_ids = [row["patternId"] for row in self.gameplay["patterns"]]
        self.assertEqual(len(visible_pattern_ids), len(set(visible_pattern_ids)))
        for pattern_id in visible_pattern_ids:
            display_name = patterns[pattern_id]["displayName"]
            with self.subTest(pattern_id=pattern_id):
                self.assertNotIn("[P2 Animation]", display_name)
                self.assertTrue(
                    any("가" <= character <= "힣" for character in display_name),
                    display_name,
                )

        pattern_node = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_ValtanPatternNode(",
            "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
        )
        self.assertIn(
            "Pattern.strPatternId : Pattern.strDisplayName",
            pattern_node,
            "the Korean display name must be the primary visible Pattern label",
        )

    def test_independent_rows_split_local_owner_preview_from_server_logic(self) -> None:
        independent = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
            "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
        )
        standalone_open = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_OpenValtanStandaloneEffect(",
            "bool_t Client::CEffect_Tool::Try_PlayValtanStandaloneEffect(",
        )
        standalone_target = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Prepare_ValtanStandaloneEffectTarget(",
            "bool_t Client::CEffect_Tool::Try_OpenValtanStandaloneEffect(",
        )
        synchronize = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Synchronize_LoadedSkillPreview()",
            "bool_t Client::CEffect_Tool::Start_SynchronizedAnimationClip(",
        )
        update = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Update(const f32_t fTimeDelta)",
            "void Client::CEffect_Tool::Render()",
        )
        restart = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Start_WorldPreviewFromBeginning()",
            "void Client::CEffect_Tool::Synchronize_LoadedSkillPreview()",
        )
        pending = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Execute_PendingDocumentLoad(",
            "bool_t Client::CEffect_Tool::Refresh_AllEffects(",
        )
        for token in (
            "Build_ValtanAuthoringTimeline",
            "Build_ValtanProductPreview",
            "PatternStagePreview",
            "pStageClockCue",
            "Try_OpenValtanAuthoredEffect",
            "Try_OpenValtanSavedReferenceEffect",
            "Try_PlayValtanSavedUnifiedEffect",
            "Try_OpenValtanStandaloneEffect",
            "Try_PlayValtanStandaloneEffect",
            "iEffectStartMs",
            'ImGui::SmallButton("Play Server Owner")',
            "Try_PlayValtanServerPattern(*pOwnerPattern)",
            "actual target tracking and hit timing require Play Server",
        ):
            self.assertIn(token, independent)
        self.assertIn(
            "iOwnerTimelineDurationMs, true, iEffectStartMs",
            independent,
            "combat-object local playback must wait for its Server owner stage",
        )
        for token in (
            "EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT",
            "Clear_ProductCuePreview()",
            "Reset_SynchronizedAnimationSequence()",
            "pModel->Set_AnimPaused(true)",
            "CAnimationTargetService::Resolve_Boss()",
        ):
            self.assertIn(token, standalone_open + standalone_target)
        self.assertLess(
            standalone_target.index("pModel->Set_AnimPaused(true)"),
            standalone_target.index("Reset_SynchronizedAnimationSequence()"),
        )
        self.assertIn("m_eActiveDocumentPreviewIntent", synchronize)
        self.assertIn("Prepare_ValtanStandaloneEffectTarget()", synchronize)
        for invariant in (
            "bStandaloneValtanEffectActive",
            "(!bStaticAreaEffectActive && Has_UnsavedWork()) ||",
            "CAnimationTargetService::Resolve_Boss()",
            "Prepare_ValtanStandaloneEffectTarget()",
            "Release_WorldPreview(true)",
        ):
            self.assertIn(invariant, update)
        self.assertIn(
            "EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT", restart
        )
        self.assertIn("Prepare_ValtanStandaloneEffectTarget()", restart)
        self.assertIn("LEVEL::VALTAN_ARENA", self.character_preview)
        self.assertIn("bValtanArenaBossPreview", self.character_preview)
        self.assertIn("Pending.ePreviewIntent", pending)
        self.assertIn("Try_PlayActiveUnifiedEffect()", pending)

    def test_open_editor_stages_owner_timeline_paused_at_zero(self) -> None:
        saved_reference = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_OpenValtanSavedReferenceEffect(",
            "bool_t Client::CEffect_Tool::Try_OpenValtanAuthoredEffect(",
        )
        authored = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_OpenValtanAuthoredEffect(\n\tconst std::filesystem::path& Path,\n\tconst std::string& strEffectAssetId,\n\tconst VALTAN_PRODUCT_PREVIEW& Preview,",
            "bool_t Client::CEffect_Tool::Refresh_ValtanPatternTree()",
        )
        pending = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Execute_PendingDocumentLoad(",
            "bool_t Client::CEffect_Tool::Refresh_AllEffects(",
        )
        self.assertIn("if (!bQueuePlayCompleteAfterLoad)", saved_reference)
        self.assertIn(
            "bAnimationReady && !bQueuePlayCompleteAfterLoad", authored
        )
        for section in (saved_reference, authored):
            self.assertIn("Set_SynchronizedAnimationPaused(true)", section)
        self.assertGreaterEqual(
            pending.count("Set_SynchronizedAnimationPaused(true)"),
            2,
            "Save/Discard + Open must preserve the same paused owner timeline",
        )

    def test_authoritative_valtan_effect_inventory_and_timing_are_intact(self) -> None:
        cues = self.product_cues["cues"]
        cue_asset_ids = [row["effectAssetId"] for row in cues]
        split_cues = [
            cue
            for pattern in self.presentation["patterns"]
            for stage in pattern["stages"]
            for cue in stage["effectCues"]
        ]
        managed_pattern_ids = {
            pattern["patternId"] for pattern in self.presentation["patterns"]
        }
        managed_product_cues = [
            cue for cue in cues if cue["patternId"] in managed_pattern_ids
        ]
        self.assertEqual(
            {cue["cueId"] for cue in split_cues},
            {cue["bindingId"] for cue in managed_product_cues},
        )
        self.assertEqual(len(split_cues), len(managed_product_cues))
        unlinked_recovery_cue = (
            "cue.valtan.carrier-v1.attack.four-slash.recovery.clip-01"
        )
        self.assertNotIn(
            unlinked_recovery_cue,
            {cue["bindingId"] for cue in cues},
        )

        valtan_catalog = {
            row["effectAssetId"]: row
            for row in self.effect_catalog["effects"]
            if row["effectAssetId"].startswith("effect.valtan.")
        }
        self.assertGreaterEqual(len(valtan_catalog), 58)
        retired_recovery_id = (
            "effect.valtan.carrier-v1.attack.four-slash.recovery.clip-01"
        )
        self.assertNotIn(retired_recovery_id, valtan_catalog)
        self.assertFalse(
            (
                REPOSITORY_ROOT
                / f"Data/Effects/Authored/{retired_recovery_id}.effect.json"
            ).exists()
        )
        for effect_asset_id in set(cue_asset_ids) | {
            "effect.valtan.sky-axe.active",
        }:
            with self.subTest(effect_asset_id=effect_asset_id):
                self.assertIn(effect_asset_id, valtan_catalog)
                authoring_path = REPOSITORY_ROOT / "Data" / valtan_catalog[
                    effect_asset_id
                ]["authoringPath"]
                self.assertTrue(authoring_path.is_file(), authoring_path)
                document = json.loads(authoring_path.read_text(encoding="utf-8"))
                self.assertEqual(effect_asset_id, document["effectAssetId"])
                self.assertGreater(len(document["elements"]), 0)

        fist = next(
            row for row in self.presentation["patterns"]
            if row["patternId"] == "VALTAN_FIST_IN_OUT"
        )
        self.assertEqual(1, len(fist["stages"]))
        fist_stage = fist["stages"][0]
        self.assertEqual("NONE", fist_stage["animation"]["mode"])
        self.assertEqual([], fist_stage["effectCues"])
        self.assertFalse(any(row["patternId"] == "VALTAN_FIST_IN_OUT" for row in cues))
        independent = next(
            row for row in self.presentation["independentEffects"]
            if row["independentEffectId"] == "valtan.independent-effect.donut-in-out"
        )
        self.assertEqual("SERVER_COMBAT_OBJECT", independent["ownership"])
        owner = next(row for row in self.gameplay["patterns"] if row["patternId"] == "VALTAN_FIST_IN_OUT")
        spawn = next(
            event for stage in owner["stages"] for event in stage["events"]
            if event["eventId"] == independent["spawnEventId"]
        )
        self.assertEqual("SPAWN_COMBAT_OBJECT", spawn["kind"])
        self.assertEqual("combatobject.valtan.fist-in-out.donut", spawn["combatObjectArchetypeId"])

    def test_takeoff_excludes_late_legacy_rows_and_decal_owners_allow_user_edits(
        self,
    ) -> None:
        authored_root = REPOSITORY_ROOT / "Data/Effects/Authored"
        takeoff = json.loads((
            authored_root /
            "effect.valtan.carrier-v1.attack.high-jump.takeoff.clip-01.effect.json"
        ).read_text(encoding="utf-8"))
        self.assertEqual(
            ["source.f4617c98d44349eec51d"],
            [row["id"] for row in takeoff["elements"]],
        )

        texture_id = (
            "Effect/Valtan/Textures/FX_TEX_HIGH_01/fx_e_decal_007_2.dds"
        )
        owners = set()
        for path in authored_root.glob("*.effect.json"):
            document = json.loads(path.read_text(encoding="utf-8"))
            if any(
                resource.get("assetId") == texture_id
                for element in document.get("elements", [])
                for resource in element.get("resources", [])
            ):
                owners.add(document["effectAssetId"])
        # The editable floor-wipe document may remove or replace this decal.
        # Keep the other exact owners and reject unrelated ownership drift.
        optional_floor_wipe_owner = (
            "effect.valtan.carrier-v1.mechanic.floor-wipe-130.second-smash.clip-01"
        )
        self.assertEqual(
            {
                "effect.valtan.carrier-v1.attack.swing.active.clip-02",
                "effect.valtan.carrier-v1.mechanic.four-pillars-105.target-cone.clip-01",
            },
            owners - {optional_floor_wipe_owner},
        )

    def test_new_effect_is_a_two_document_cas_transaction(self) -> None:
        create = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_CreateValtanPatternEffect(",
            "void Client::CEffect_Tool::Render_ValtanPatternNode(",
        )
        for token in (
            "EFFECT_AUTHORING_FORMAT_VERSION",
            "Build_ValtanPatternAggregateEffectAssetId",
            "DRAFT_ATTACHED",
            "CEffectDocumentCodec::Save_AtomicIfUnchanged",
            "CValtanPatternAuthoringEffectDocument::Save_AtomicIfUnchanged",
            "m_strValtanPatternAuthoringEffectsBaseline",
            "CValtanPatternAuthoringEffectTransaction Transaction",
            "CurrentOwnershipBaseline",
            "CAuthoritativeProductSourceReadLocks",
            "Try_LockAndInspectAuthoritativeProductOwnership",
            "CEffectCatalog::Contains",
            "Remove_EffectDocumentIfCanonical",
            "Refresh_ValtanPatternAuthoringEffects()",
        ):
            self.assertIn(token, create)
        self.assertLess(
            create.index("CEffectDocumentCodec::Save_AtomicIfUnchanged"),
            create.index(
                "CValtanPatternAuthoringEffectDocument::Save_AtomicIfUnchanged"
            ),
        )
        for forbidden in (
            "EffectCatalog.json",
            "Valtan.gameplay.json",
            "Valtan.presentation.json",
            "Valtan.patternbindings.json",
            "CValtanPatternAuditionService",
        ):
            self.assertNotIn(forbidden, create)

    def test_existing_aggregate_is_opened_without_duplicate_creation(self) -> None:
        pattern = next(
            row for row in self.gameplay["patterns"]
            if row["patternId"] == "VALTAN_CHARGE"
        )
        aggregate_id = "effect." + pattern["actionId"]
        registered = next(
            row for row in self.effect_catalog["effects"]
            if row["effectAssetId"] == aggregate_id
        )
        preserved_path = REPOSITORY_ROOT / "Data" / registered["authoringPath"]
        preserved = json.loads(preserved_path.read_text(encoding="utf-8"))
        self.assertEqual(aggregate_id, preserved["effectAssetId"])
        ownership = json.loads(OWNERSHIP_JSON.read_text(encoding="utf-8"))
        self.assertFalse(any(
            row["patternId"] == pattern["patternId"]
            and row["effectAssetId"] == aggregate_id
            and row["state"] == "DRAFT_ATTACHED"
            for row in ownership["bindings"]
        ))

        tree = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
            "void Client::CEffect_Tool::Render_AllEffectsWindow()",
        )
        for token in (
            "std::filesystem::exists(AggregateEffectPath, AggregatePathError)",
            "CEffectCatalog::Contains(Aggregate.strEffectAssetId)",
            "m_DirectAuthoredEditableEntries.contains(Aggregate.strEffectAssetId)",
            "!Has_UnsavedWork() && !bExistingAggregate",
            "!AggregateEffectPath.empty() && !AggregatePathError",
            'ImGui::Button("Open Existing Effect")',
            "Try_OpenExistingValtanPatternEffect(*pSelectedPattern)",
        ):
            self.assertIn(token, tree)
        after_open = tree[tree.index("Try_OpenExistingValtanPatternEffect("):]
        self.assertLess(
            after_open.index("if (bOpenExistingRequested)\n\t\t\treturn;"),
            after_open.index("Can_DeleteSelectedValtanPatternEffect"),
            "Product Open refreshes the tree; the frame must stop using its old pointers",
        )

    def test_selected_pattern_identity_uses_the_shared_tree_summary(self) -> None:
        tree = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
            "void Client::CEffect_Tool::Render_AllEffectsWindow()",
        )
        selected = tree[
            tree.index('ImGui::SeparatorText("Pattern Authoring")'):
            tree.index("std::filesystem::path AggregateEffectPath")
        ]
        self.assertIn("pSelectedPattern->strDisplayName.c_str()", selected)
        self.assertIn(
            "CValtanPatternTree::Build_PatternIdentitySummary(*pSelectedPattern)",
            selected,
        )
        self.assertNotIn("VALTAN_STRUGGLING", selected)
        self.assertNotIn("VALTAN_FRONT_BACK_FRONT", selected)

    def test_existing_open_validates_before_product_or_authoring_only_open(self) -> None:
        existing = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_OpenExistingValtanPatternEffect(",
            "bool_t Client::CEffect_Tool::Matches_ValtanPatternSearch(",
        )
        for token in (
            "m_ValtanPatternProductUnlinkOperation.has_value()",
            "m_strSelectedValtanPatternId != Pattern.strPatternId",
            "Path.lexically_normal()",
            "CEffectDocumentCodec::Load(Path, ExistingDocument, Error)",
            "ExistingDocument.strEffectAssetId != Aggregate.strEffectAssetId",
            "if (iProductCueCount > 1u)",
            "if (1u == iProductCueCount)",
            "Request.strStageId = Stage.strStageId",
            "Request.strCueOccurrenceId = Cue.strOccurrenceId",
            "return Open_ValtanProductEffect(Request)",
            "Try_OpenValtanStandaloneEffect(Path, Aggregate.strEffectAssetId)",
            "This Pattern remains unlinked",
        ):
            self.assertIn(token, existing)
        self.assertLess(
            existing.index("CEffectDocumentCodec::Load"),
            existing.index("Open_ValtanProductEffect(Request)"),
        )
        self.assertLess(
            existing.index("ExistingDocument.strEffectAssetId !="),
            existing.index("Try_OpenValtanStandaloneEffect"),
        )
        for forbidden in (
            "Save_Atomic", "std::filesystem::remove", "New_Effect",
            "Bindings.push_back", ".authored-copy", "Try_CreateValtanPatternEffect",
        ):
            self.assertNotIn(forbidden, existing)

    def test_existing_authoring_open_preserves_failures_and_pending_identity(self) -> None:
        standalone = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_OpenValtanStandaloneEffect(",
            "bool_t Client::CEffect_Tool::Try_PlayValtanStandaloneEffect(",
        )
        self.assertIn("m_strPreviewStatus = Cache.strStatus", standalone)
        self.assertIn("m_strPreviewStatus = m_strDocumentStatus", standalone)
        self.assertIn("m_strPreviewStatus = m_strPreviewAnimationStatus", standalone)
        staged_load = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_LoadDocumentPathStaged(",
            "bool_t Client::CEffect_Tool::Execute_PendingDocumentLoad(",
        )
        for precondition in (
            "Has_UnsavedWork()",
            "CEffectDocumentCodec::Load(Path, Staged, Error)",
            "Staged.strEffectAssetId != strSelectionId",
            "!Prepare_ValtanStandaloneEffectTarget()",
        ):
            self.assertLess(
                staged_load.index(precondition),
                staged_load.index("m_ActiveDocument = std::move(Staged)"),
            )
        existing = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_OpenExistingValtanPatternEffect(",
            "bool_t Client::CEffect_Tool::Matches_ValtanPatternSearch(",
        )
        self.assertIn(
            "m_PendingDocumentLoad->strValtanPatternId = Aggregate.strPatternId",
            existing,
        )
        pending = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Execute_PendingDocumentLoad(",
            "bool_t Client::CEffect_Tool::Refresh_DataFiles()",
        )
        self.assertLess(
            pending.index("Open Existing Effect failed; the current Effect is unchanged"),
            pending.index("const auto CompleteValtanPreviewPartial"),
        )
        self.assertIn("m_strSelectedValtanPatternId = Pending.strValtanPatternId", pending)
        self.assertIn("Product connections were not changed", pending)
        self.assertNotIn("Save_Atomic", pending)
        modal = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_PendingDocumentLoadModal()",
            "void Client::CEffect_Tool::Render_EffectTypeSelector()",
        )
        cancel = modal[modal.index('if (ImGui::Button("Cancel"))'):]
        self.assertLess(
            cancel.index("Cancelled Open Existing Effect"),
            cancel.index("m_PendingDocumentLoad.reset()"),
        )
        self.assertNotIn("m_ActiveDocument =", cancel)

    def test_pattern_draft_recovers_after_model_view_target_replacement(self) -> None:
        update = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Update(const f32_t fTimeDelta)",
            "void Client::CEffect_Tool::Render()",
        )
        for token in (
            "bValtanPatternDraftActive",
            "bValtanPatternDraftTimelineRebound",
            "bValtanPatternDraftTargetInvalid",
            "m_iSynchronizedAnimationTargetGeneration !=",
            "CAnimationTargetService::Resolve_TargetGeneration()",
            "Prepare_ActiveValtanPatternDraftTimeline(!m_bPreviewPlaying)",
            "bool_t bSeekAfterLoop = bValtanPatternDraftTimelineRebound",
            "bSeekAfterLoop = bSeekAfterLoop ||",
            "Seek_WorldPreviewWithSourceAnchorHistory",
            "pObject->Set_SampleTime(fEffectSampleSeconds)",
        ):
            self.assertIn(token, update)
        self.assertLess(
            update.index("m_iSynchronizedAnimationTargetGeneration !="),
            update.index("Update_SynchronizedAnimationSequence()"),
            "target replacement must rebuild the Pattern timeline before generic update resets it",
        )
        self.assertLess(
            update.index("bool_t bSeekAfterLoop = bValtanPatternDraftTimelineRebound"),
            update.index("pObject->Advance_Preview(fSequentialAdvance, Root)"),
            "target replacement must force a zero-time full seek before sequential Effect advance",
        )

    def test_new_effect_reports_auto_open_failure_without_rolling_back_commit(self) -> None:
        create = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_CreateValtanPatternEffect(",
            "bool_t Client::CEffect_Tool::Can_DeleteSelectedValtanPatternEffect(",
        )
        self.assertNotIn("(void)Try_OpenValtanPatternDraftEffect", create)
        self.assertIn("if (!Try_OpenValtanPatternDraftEffect", create)
        self.assertIn("files remain committed", create)
        self.assertLess(
            create.rindex(
                "CValtanPatternAuthoringEffectDocument::Save_AtomicIfUnchanged"
            ),
            create.index("if (!Try_OpenValtanPatternDraftEffect"),
        )

    def test_delete_effect_is_selected_row_scoped_and_confirmed(self) -> None:
        pattern = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_ValtanPatternNode(",
            "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
        )
        tree = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
            "void Client::CEffect_Tool::Render_AllEffectsWindow()",
        )
        self.assertIn("m_SelectedValtanPatternEffect", pattern)
        self.assertIn("Selection.CueIds", pattern)
        self.assertIn("VALTAN_PATTERN_EFFECT_SELECTION_KIND::PRODUCT_CUE_LINK", pattern)
        self.assertIn("VALTAN_PATTERN_EFFECT_SELECTION_KIND::DRAFT_ATTACHED", pattern)
        self.assertIn('ImGui::Button("Delete Effect")', tree)
        self.assertIn('ImGui::Button("Confirm Delete")', tree)
        self.assertIn("m_PendingValtanPatternEffectDeletion", tree)
        self.assertIn("bDeleteAttemptedThisFrame", tree)
        delete = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_DeleteSelectedValtanPatternEffect()",
            "void Client::CEffect_Tool::Render_ValtanPatternNode(",
        )
        for token in (
            "Pending.eKind != Current.eKind",
            "Pending.strPatternId != Current.strPatternId",
            "Pending.strEffectAssetId != Current.strEffectAssetId",
            "Pending.CueIds != Current.CueIds",
            "nothing was deleted",
        ):
            self.assertIn(token, delete)
        self.assertLess(
            tree.index('ImGui::Button("Create Effect")'),
            tree.index('ImGui::Button("Delete Effect")'),
        )

    def test_draft_delete_uses_sidecar_cas_and_exact_file_delete(self) -> None:
        draft_delete = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_DeleteValtanPatternDraftEffect(",
            "bool_t Client::CEffect_Tool::Try_UnlinkValtanPatternProductEffect(",
        )
        for token in (
            "Build_ValtanPatternAggregateEffectAssetId",
            "Build_AuthoringPath",
            "CEffectDocumentCodec::Load",
            "CEffectCatalog::Contains",
            "Try_LockAndInspectAuthoritativeProductOwnership",
            "Count_ProductCueMappings",
            "CValtanPatternAuthoringEffectDocument::Save_AtomicIfUnchanged",
            "Remove_EffectDocumentIfCanonical",
            "StagedOwnershipCanonical",
            "Discard_ActiveDocument",
        ):
            self.assertIn(token, draft_delete)
        self.assertLess(
            draft_delete.index(
                "CValtanPatternAuthoringEffectDocument::Save_AtomicIfUnchanged"
            ),
            draft_delete.index("Remove_EffectDocumentIfCanonical"),
        )
        self.assertGreaterEqual(
            draft_delete.count(
                "CValtanPatternAuthoringEffectDocument::Save_AtomicIfUnchanged"
            ),
            2,
            "file-delete failure must CAS-restore the sidecar ownership",
        )
        for token in (
            "CAuthoritativeProductSourceReadLocks",
            "FILE_SHARE_READ",
            "Try_ContainsSourceRegistrationFresh",
            "CValtanPatternTree::Load",
            "Count_ValtanProductReferences",
            "Valtan.gameplay.json",
            "Valtan.presentation.json",
            "ValtanEncounter.json",
            "ValtanPatternRotations.json",
            "Valtan.patternbindings.json",
            "Valtan.patterneffectcues.json",
            "Valtan.patterneffects.json",
            "BossCatalog.json",
            "ValtanCombatObjects.json",
            "complete authoritative Product read set",
        ):
            self.assertIn(token, self.cpp)

    def test_product_delete_unlinks_only_selected_pattern_and_preserves_asset(self) -> None:
        unlink = source_section(
            self.cpp,
            "bool_t Client::CEffect_Tool::Try_UnlinkValtanPatternProductEffect(",
            "bool_t Client::CEffect_Tool::Try_DeleteSelectedValtanPatternEffect(",
        )
        for token in (
            "Remove-ValtanPatternEffectLink.ps1",
            "-Mode Apply",
            "-PatternId",
            "-EffectAssetId",
            "-CueIds",
            "Start_OwnedToolProcess",
            "m_ValtanPatternProductUnlinkOperation",
            "the process will not be killed on timeout",
        ):
            self.assertIn(token, unlink)
        for forbidden in (
            "EffectCatalog.json",
            "std::filesystem::remove",
            "DeleteFileW",
            "Remove_EffectDocumentIfCanonical",
            "Run_OwnedToolProcess",
            "TerminateProcess",
        ):
            self.assertNotIn(forbidden, unlink)
        poll = source_section(
            self.cpp,
            "void Client::CEffect_Tool::Update_ValtanPatternProductEffectUnlink()",
            "bool_t Client::CEffect_Tool::Try_CreateValtanPatternEffect(",
        )
        for token in (
            "WaitForSingleObject(Operation.hProcess, 0u)",
            "WAIT_OBJECT_0 != iWait",
            "still running after 180 seconds",
            "It was not terminated",
            "All Effects remains locked",
            "Refresh_ValtanPatternTree()",
            "Refresh_DataFiles()",
            "does not assume that source or Product was preserved",
            "shared Effect asset, authored file, and other Pattern links were preserved",
            "Restart the Server and re-enter Valtan Arena",
        ):
            self.assertIn(token, poll)
        self.assertNotIn("TerminateProcess", poll)
        destructor = source_section(
            self.cpp,
            "Client::CEffect_Tool::~CEffect_Tool()",
            "void Client::CEffect_Tool::Update(const f32_t fTimeDelta)",
        )
        self.assertIn("CloseHandle", destructor)
        self.assertNotIn("TerminateProcess", destructor)

    def test_sidecar_is_effect_domain_only_and_registered_in_client(self) -> None:
        document = json.loads(OWNERSHIP_JSON.read_text(encoding="utf-8"))
        self.assertEqual(
            {"schema", "formatVersion", "bossArchetypeId", "bindings"},
            set(document),
        )
        self.assertEqual(
            "lostark.valtan-pattern-authoring-effects", document["schema"]
        )
        self.assertEqual(1, document["formatVersion"])
        self.assertEqual("BOSS_VALTAN", document["bossArchetypeId"])
        self.assertIsInstance(document["bindings"], list)
        self.assertEqual([], document["bindings"])
        seen_patterns: set[str] = set()
        seen_effects: set[str] = set()
        for binding in document["bindings"]:
            self.assertEqual(
                {"patternId", "effectAssetId", "authoringPath", "state"},
                set(binding),
            )
            pattern_id = binding["patternId"]
            effect_id = binding["effectAssetId"]
            self.assertRegex(pattern_id, r"^[A-Z0-9_]+$")
            self.assertRegex(effect_id, r"^[a-z0-9._-]+$")
            self.assertEqual(
                f"Effects/Authored/{effect_id}.effect.json",
                binding["authoringPath"],
            )
            self.assertEqual("DRAFT_ATTACHED", binding["state"])
            self.assertNotIn(pattern_id, seen_patterns)
            self.assertNotIn(effect_id, seen_effects)
            seen_patterns.add(pattern_id)
            seen_effects.add(effect_id)
        combined = self.ownership_header + self.ownership_cpp
        for token in (
            "Effects/ValtanPatternAuthoringEffects.json",
            "Effects/Authored/",
            "DRAFT_ATTACHED",
            "Save_AtomicIfUnchanged",
            "changed on disk; refresh before creating",
            "TRANSACTION_MUTEX_NAME",
            "WaitForSingleObject",
            "BeforeReplaceCanonical",
            "MoveFileExW",
        ):
            self.assertIn(token, combined)
        for forbidden in (
            "strStageId",
            "strClipName",
            "iDurationMs",
            "strAnimation",
            "EffectCatalog",
        ):
            self.assertNotIn(forbidden, self.ownership_header)

        project = CLIENT_PROJECT.read_text(encoding="utf-8")
        filters = CLIENT_FILTERS.read_text(encoding="utf-8")
        for text in (project, filters):
            self.assertIn("ValtanPatternAuthoringEffectDocument.h", text)
            self.assertIn("ValtanPatternAuthoringEffectDocument.cpp", text)


if __name__ == "__main__":
    unittest.main(verbosity=2)
