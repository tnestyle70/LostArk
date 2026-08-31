#!/usr/bin/env python3
from __future__ import annotations

import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
WORKBENCH_H = ROOT / "Client/Public/ActionCompositionWorkbench.h"
WORKBENCH_CPP = ROOT / "Client/Private/ActionCompositionWorkbench.cpp"


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

    def test_timeline_rows_keep_clip_qualified_effect_and_point_sound_semantics(self) -> None:
        timeline = body(
            self.source,
            "void Client::CActionCompositionWorkbench::Build_Timeline(",
        )
        self.assertIn("!Cue.bUsesStageClock", timeline)
        self.assertIn('Cue.strStopPolicy + "/" +', timeline)
        self.assertIn('Cue.strRepeatPolicy + "]"', timeline)
        self.assertIn(
            "DETAIL_OWNER::SOUND, TIMELINE_LANE::SOUND", timeline
        )
        self.assertIn("iStageStartMs + iLocalMs, iStageStartMs + iLocalMs, true", timeline)
        self.assertIn('Cue.eRepeatPolicy ? " [each_loop]" : " [once]"', timeline)

    def test_dependency_preflight_is_full_join_and_fail_closed_for_all_three_lanes(self) -> None:
        validator = body(
            self.source,
            "bool_t Client::CActionCompositionWorkbench::Validate_TimelineDependencyWindows(",
        )
        for token in (
            "ADMISSION_STATE::ADMITTED != m_eAdmission",
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
            "bool_t Client::CActionCompositionWorkbench::Apply_EffectOccurrenceTiming(",
        )
        self.assertIn("VALTAN_PRODUCT_EFFECT_CUE_VIEW Candidate = Current", patch)
        self.assertIn("Is_PatternSoundDraftDirty", patch)
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
        self.assertIn("Save + Validate + Publish was not invoked", patch)

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
            "void Client::CActionCompositionWorkbench::Render_Timeline(",
        )
        for token in (
            "const std::vector<TIMELINE_ITEM>& TimelineItems = m_TimelineItems",
            "bPatternMutationAdmitted",
            "bSoundMoveAdmitted",
            "!m_bAuthoringDraftDirty",
            "m_iTimelineMoveSourceStartMs",
            "m_iTimelineMoveSourceEndMs",
            "Apply_EffectOccurrenceTiming",
            "Resolve_ValtanCompositionPatternSoundWindow",
            "Apply_PatternSoundOccurrenceTiming",
        ):
            self.assertIn(token, timeline)
        self.assertIn('"cue_end" == pEffectCue->strStopPolicy', timeline)
        self.assertIn('"once" == pEffectCue->strRepeatPolicy', timeline)
        self.assertNotIn("Save_Publish_Reload(", timeline)
        self.assertNotIn("Save_ValtanCompositionPatternSounds", timeline)
        self.assertNotIn("Retry_ValtanCompositionPatternSoundRuntimeApply", timeline)

        self.assertIn("bool_t bMutationAdmitted,", self.header)
        self.assertIn("bool_t bPatternMutationAdmitted);", self.header)

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
