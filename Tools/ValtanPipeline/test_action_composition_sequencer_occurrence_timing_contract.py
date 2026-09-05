#!/usr/bin/env python3
from __future__ import annotations

import unittest
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
WORKBENCH_H = ROOT / "Client/Public/ValtanActionWorkbench.h"
WORKBENCH_CPP = ROOT / "Client/Private/ValtanActionWorkbench.cpp"
BALANCE_H = ROOT / "Client/Public/BalanceTool.h"
BALANCE_CPP = ROOT / "Client/Private/BalanceTool.cpp"


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unterminated function: {signature}")


def effect_right_trim_admitted(*, has_end: bool, stop: str, repeat: str) -> bool:
    return has_end and stop == "cue_end" and repeat == "once"


class ActionCompositionSequencerOccurrenceTimingContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.header = WORKBENCH_H.read_text(encoding="utf-8")
        cls.source = WORKBENCH_CPP.read_text(encoding="utf-8")
        cls.balance_header = BALANCE_H.read_text(encoding="utf-8")
        cls.balance_source = BALANCE_CPP.read_text(encoding="utf-8")

    def test_timeline_rows_keep_clip_qualified_effect_and_point_sound_semantics(self) -> None:
        timeline = body(
            self.source,
            "void Client::CValtanActionWorkbench::Build_Timeline(",
        )
        self.assertIn("!Cue.bUsesStageClock", timeline)
        self.assertIn('Cue.strStopPolicy + "/" +', timeline)
        self.assertIn('Cue.strRepeatPolicy + "]"', timeline)
        self.assertIn(
            "DETAIL_OWNER::SOUND, TIMELINE_LANE::SOUND", timeline
        )
        self.assertIn("iStageStartMs + iLocalMs, iStageStartMs + iLocalMs, true", timeline)
        self.assertIn("0u, SOUND_EVENT_MINIMUM_WIDTH_PX", timeline)
        self.assertIn("SOUND_EVENT_MINIMUM_WIDTH_PX = 180.f", self.source)
        self.assertIn('Cue.eRepeatPolicy ? " [each_loop]" : " [once]"', timeline)

    def test_dependency_preflight_is_full_join_and_fail_closed_for_all_three_lanes(self) -> None:
        validator = body(
            self.source,
            "bool_t Client::CValtanActionWorkbench::Validate_TimelineDependencyWindows(",
        )
        for token in (
            "VALTAN_VIEW_ADMISSION::ADMITTED != m_eAdmission",
            "Get_ValtanCompositionPatternSoundDraft",
            "!m_bPatternShakesReady",
            "for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Existing",
            '"Effect occurrence " + pCandidate->strOccurrenceId',
            "for (const VALTAN_PATTERN_SOUND_CUE& Existing",
            '"Pattern Sound occurrence " + pCandidate->strOccurrenceId',
            "for (const VALTAN_PATTERN_SHAKE_CUE& Cue",
            '"Pattern Shake occurrence " + Cue.strOccurrenceId',
            "1u != iEffectOverrideMatches",
            "1u != iSoundOverrideMatches",
        ):
            self.assertIn(token, validator)

    def test_effect_timing_patch_preserves_identity_policy_and_requires_explicit_save(self) -> None:
        patch = body(
            self.source,
            "bool_t Client::CValtanActionWorkbench::Apply_EffectOccurrenceTiming(",
        )
        self.assertIn("VALTAN_PRODUCT_EFFECT_CUE_VIEW Candidate = Current", patch)
        self.assertNotIn("Is_PatternSoundDraftDirty", patch)
        self.assertIn("Candidate.iSourceStartMs = iSourceStartMs", patch)
        self.assertIn(
            "Candidate.iSourceEndMs = Candidate.bHasSourceEnd ? iSourceEndMs : 0u",
            patch,
        )
        self.assertIn("Validate_TimelineDependencyWindows", patch)
        self.assertIn("Update_ValtanStageEffectCue", patch)
        for preserved in (
            "Candidate.strBindingId =",
            "Candidate.strOccurrenceId =",
            "Candidate.strClipOccurrenceId =",
            "Candidate.strEffectAssetId =",
            "Candidate.strAnchorSlotId =",
            "Candidate.strStopPolicy =",
            "Candidate.strRepeatPolicy =",
        ):
            self.assertNotIn(preserved, patch)
        self.assertNotIn("Save_Publish_Reload(", patch)
        self.assertIn("use Save when the timing is ready", patch)

    def test_sound_timing_patch_changes_only_point_start_in_separate_owner_draft(self) -> None:
        patch = body(
            self.source,
            "Apply_PatternSoundOccurrenceTiming(",
        )
        self.assertIn("VALTAN_PATTERN_SOUND_CUE Candidate = Current", patch)
        self.assertIn("Candidate.iStartMs = iSourceStartMs", patch)
        self.assertIn("Validate_TimelineDependencyWindows", patch)
        self.assertIn("Patch_ValtanCompositionPatternSound", patch)
        self.assertIn("Current.strSoundEvent", patch)
        self.assertIn("Current.eRepeatPolicy", patch)
        for forbidden in (
            "Candidate.strBindingId =",
            "Candidate.strOccurrenceId =",
            "Candidate.strClipOccurrenceId =",
            "Candidate.strSoundEvent =",
            "Candidate.eRepeatPolicy =",
            "Save_ValtanCompositionPatternSounds",
            "Retry_ValtanCompositionPatternSoundRuntimeApply",
        ):
            self.assertNotIn(forbidden, patch)
        self.assertIn("Save/Apply was not invoked", patch)

    def test_drag_uses_dual_owner_gates_and_frame_local_rows(self) -> None:
        timeline = body(
            self.source,
            "void Client::CValtanActionWorkbench::Render_Timeline(",
        )
        for token in (
            "const std::vector<TIMELINE_ITEM>& TimelineItems = m_TimelineItems",
            "bPatternMutationAdmitted",
            "bSoundMoveAdmitted",
            "m_iTimelineMoveSourceStartMs",
            "m_iTimelineMoveSourceEndMs",
            "Apply_EffectOccurrenceTiming",
            "Resolve_ValtanCompositionPatternSoundWindow",
            "Apply_PatternSoundOccurrenceTiming",
            "bAnimationMove",
            "Transfer_AnimationOccurrence",
        ):
            self.assertIn(token, timeline)
        self.assertNotIn(
            "bSoundMoveAdmitted = bMutationAdmitted &&\n\t\t!m_bAuthoringDraftDirty",
            timeline,
        )
        self.assertIn('"cue_end" == pEffectCue->strStopPolicy', timeline)
        self.assertIn('"once" == pEffectCue->strRepeatPolicy', timeline)
        self.assertNotIn("Save_Publish_Reload(", timeline)
        self.assertNotIn("Save_ValtanCompositionPatternSounds", timeline)
        self.assertNotIn("Retry_ValtanCompositionPatternSoundRuntimeApply", timeline)

        self.assertIn("bool_t bMutationAdmitted,", self.header)
        self.assertIn("bool_t bPatternMutationAdmitted);", self.header)

    def test_animation_drag_routes_across_stage_clocks_as_one_transaction(self) -> None:
        timeline = body(
            self.source,
            "void Client::CValtanActionWorkbench::Render_Timeline(",
        )
        for token in (
            "pTargetStageItem",
            "TIMELINE_LANE::STAGE",
            "pTargetStage->strStageId",
            "Transfer_AnimationOccurrence(",
        ):
            self.assertIn(token, timeline)

        transfer = body(
            self.source,
            "bool_t Client::CValtanActionWorkbench::Transfer_AnimationOccurrence(",
        )
        for token in (
            "SourceDraft.animationSlots.erase(Found)",
            "TargetDraft.animationSlots.insert(",
            "bLinkedEffect",
            "bLinkedEffectV2",
            "bLinkedSound",
            "bLinkedShake",
            "SetValtanStageDraftWithSoundDependencyAdmission(",
            "Set_ValtanAnimationTransferDrafts(",
        ):
            self.assertIn(token, transfer)
        self.assertLess(
            transfer.index("SetValtanStageDraftWithSoundDependencyAdmission("),
            transfer.index("Set_ValtanAnimationTransferDrafts("),
        )

        self.assertIn("Set_ValtanAnimationTransferDrafts", self.balance_header)
        atomic = body(
            self.balance_source,
            "bool Client::CBalanceTool::Set_ValtanAnimationTransferDrafts(",
        )
        for token in (
            "const VALTAN_PATTERN_VIEW PatternBefore = *pPattern",
            "Set_ValtanStageDraft(",
            "*pRollbackPattern = PatternBefore",
            "m_valtanDraftGeneration = iDraftGenerationBefore",
        ):
            self.assertIn(token, atomic)
        self.assertLess(
            atomic.index("const VALTAN_PATTERN_VIEW PatternBefore = *pPattern"),
            atomic.index("Set_ValtanStageDraft("),
        )

    def test_short_boxes_pack_labels_except_collider_semantic_width(self) -> None:
        timeline = body(
            self.source,
            "void Client::CValtanActionWorkbench::Render_Timeline(",
        )
        self.assertIn(
            "ResolveTimelineDisplayWidthPx(\n\t\t\t\tItem.strLabel, Item.fMinimumDisplayWidthPx)",
            timeline,
        )
        self.assertIn("bColliderTimelineItem", timeline)
        self.assertIn("TIMELINE_POINT_MINIMUM_WIDTH_PX", timeline)
        self.assertIn("fSemanticEndX", timeline)
        self.assertIn("fStartX + fDisplayWidthPx", timeline)
        self.assertNotIn("fStartX + Item.fMinimumDisplayWidthPx", timeline)
        self.assertIn("EFFECT_V2_LEAF_MINIMUM_WIDTH_PX = 180.f", self.source)
        self.assertIn("EFFECT_V2_GROUP_MINIMUM_WIDTH_PX = 240.f", self.source)

    def test_selected_box_toolbar_dispatches_typed_duplicate_and_delete(self) -> None:
        timeline = body(
            self.source,
            "void Client::CValtanActionWorkbench::Render_Timeline(",
        )
        self.assertIn('ImGui::Button("Duplicate Box")', timeline)
        self.assertIn('ImGui::Button("Delete Box")', timeline)
        self.assertIn("Duplicate_SelectedTimelineBox(", timeline)
        self.assertIn("Delete_SelectedTimelineBox(", timeline)

        duplicate = body(
            self.source,
            "bool_t Client::CValtanActionWorkbench::Duplicate_SelectedTimelineBox(",
        )
        for token in (
            "Duplicate_AnimationOccurrence(",
            "Add_ValtanStageEffectCue(",
            "Add_ValtanCompositionPatternSound(",
        ):
            self.assertIn(token, duplicate)

        delete = body(
            self.source,
            "bool_t Client::CValtanActionWorkbench::Delete_SelectedTimelineBox(",
        )
        for token in (
            "Remove_AnimationOccurrence(",
            "Remove_ValtanStageEffectCue(",
            "Remove_ValtanCompositionPatternSound(",
        ):
            self.assertIn(token, delete)

    def test_drawn_timeline_uses_the_same_preview_branch_clock(self) -> None:
        timeline = body(
            self.source,
            "void Client::CValtanActionWorkbench::Build_Timeline(",
        )
        self.assertIn("Build_PreviewStagePath(", timeline)
        self.assertIn("for (const VALTAN_STAGE_VIEW* const pStage : PreviewStages)", timeline)
        self.assertNotIn("for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)", timeline)
        self.assertIn("m_eTimelineCachePreviewPath", self.header)

        render = body(
            self.source,
            "void Client::CValtanActionWorkbench::Render_Timeline(",
        )
        self.assertIn('ImGui::Button("Play Selected Stage (All Slots)")', render)
        self.assertIn("Build_PreviewStagePath(", render)

    def test_sequencer_exposes_the_common_preview_transport(self) -> None:
        timeline = body(
            self.source,
            "void Client::CValtanActionWorkbench::Render_Timeline(",
        )
        self.assertIn(
            'Preview.bPlaying && !Preview.bPaused ? "Pause" : "Play"',
            timeline,
        )
        self.assertIn("Play_EffectivePreview(", timeline)
        self.assertIn('ImGui::Button("Stop")', timeline)
        self.assertIn("Stop_ValtanCompositionPattern(", timeline)
        self.assertTrue(
            'ImGui::Button("Restart")' in timeline
            or 'ImGui::Button("Restart Preview")' in timeline,
            "the Sequencer must expose a Restart transport action",
        )

    def test_ruler_active_drag_scrubs_the_effective_arena_preview(self) -> None:
        timeline = body(
            self.source,
            "void Client::CValtanActionWorkbench::Render_Timeline(",
        )
        ruler_start = timeline.index('"##TimelineRuler"')
        ruler_end = timeline.index(
            "const std::vector<TIMELINE_ITEM>& TimelineItems", ruler_start
        )
        ruler = timeline[ruler_start:ruler_end]
        self.assertTrue(
            "ImGui::IsItemActive()" in ruler
            or "ImGui::IsMouseDragging(" in ruler
            or "ImGui::IsMouseDown(" in ruler,
            "the ruler must keep sampling the cursor while the left-button drag is active",
        )
        self.assertIn("m_iPlayheadMs", ruler)
        self.assertIn("Seek_EffectivePreview(", ruler)

    def test_selected_sequence_actions_are_available_above_the_timeline(self) -> None:
        timeline = body(
            self.source,
            "void Client::CValtanActionWorkbench::Render_Timeline(",
        )
        self.assertIn('ImGui::Button("Replace Stage Slots")', timeline)
        self.assertIn('ImGui::Button("Append to Stage Slots")', timeline)
        self.assertGreaterEqual(
            timeline.count("Apply_SelectedSequenceToStage("),
            2,
            "Replace and Append must both reuse the typed Stage-slot mutation path",
        )
        self.assertRegex(
            timeline,
            re.compile(
                r"Apply_SelectedSequenceToStage\(\s*\*pPattern,\s*\*\w+,\s*false\s*\)"
            ),
        )
        self.assertRegex(
            timeline,
            re.compile(
                r"Apply_SelectedSequenceToStage\(\s*\*pPattern,\s*\*\w+,\s*true\s*\)"
            ),
        )

    def test_successful_play_and_seek_request_composition_preview_ownership(self) -> None:
        play = body(
            self.source,
            "bool_t Client::CValtanActionWorkbench::Play_EffectivePreview(",
        )
        self.assertIn("m_bPreviewOwnerClaimRequested = true", play)
        play_call = play.index("Play_ValtanCompositionDraftPattern(")
        play_claim = play.index(
            "m_bPreviewOwnerClaimRequested = true", play_call
        )
        self.assertLess(play_call, play_claim)
        self.assertLess(play_claim, play.rindex("return true"))

        seek = body(
            self.source,
            "bool_t Client::CValtanActionWorkbench::Seek_EffectivePreview(",
        )
        self.assertIn("m_bPreviewOwnerClaimRequested = true", seek)
        seek_call = seek.index("Seek_ValtanCompositionPattern(")
        seek_claim = seek.index(
            "m_bPreviewOwnerClaimRequested = true", seek_call
        )
        self.assertLess(seek_call, seek_claim)

    def test_natural_and_each_loop_effects_never_expose_right_trim(self) -> None:
        self.assertTrue(
            effect_right_trim_admitted(has_end=True, stop="cue_end", repeat="once")
        )
        self.assertFalse(
            effect_right_trim_admitted(has_end=False, stop="natural", repeat="once")
        )
        self.assertFalse(
            effect_right_trim_admitted(
                has_end=True, stop="cue_end", repeat="each_loop"
            )
        )
        self.assertFalse(
            effect_right_trim_admitted(
                has_end=False, stop="natural", repeat="each_loop"
            )
        )


if __name__ == "__main__":
    unittest.main()
