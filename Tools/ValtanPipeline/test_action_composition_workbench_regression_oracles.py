#!/usr/bin/env python3
"""Focused regression oracles for the independent Action Composition Workbench.

These tests deliberately combine physical authoring fixtures with narrow source
contracts.  The fixture checks prove the indexed animation/pattern data is real;
the source contracts pin the UI-only state transitions that cannot be invoked by
the headless Product harness.
"""

from __future__ import annotations

import json
import math
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for cursor in range(opening, len(source)):
        if source[cursor] == "{":
            depth += 1
        elif source[cursor] == "}":
            depth -= 1
            if depth == 0:
                return source[opening : cursor + 1]
    raise AssertionError(f"unterminated function: {signature}")


def cpp_round_positive(value: float) -> int:
    """std::llround for the positive clocks admitted by this contract."""
    return math.floor(value + 0.5)


def wall_contract_accepts(
    policy: str,
    duration_ms: int,
    slots: list[tuple[int, float, bool]],
) -> bool:
    known = sum(cpp_round_positive(play_ms / rate) for play_ms, rate, _ in slots if play_ms)
    unknown = [slot for slot in slots if slot[0] == 0]
    loops = [slot for slot in slots if slot[2]]
    if policy == "EXACT":
        if unknown or loops or abs(known - duration_ms) > 2 or not slots:
            return False
        last_wall = cpp_round_positive(slots[-1][0] / slots[-1][1])
        return last_wall + duration_ms - known > 0
    if policy == "HOLD_LAST_POSE":
        return (
            len(unknown) <= 1
            and not loops
            and known < duration_ms + 2
            and (not unknown or known < duration_ms)
        )
    if policy == "LOOP_TO_STAGE_END":
        return len(unknown) == 1 and len(loops) == 1 and unknown[0][2] and known < duration_ms
    return False


def source_dependency_window_accepts(
    source_start_ms: int,
    play_ms: int,
    dependency_start_ms: int,
    dependency_end_ms: int | None = None,
    *,
    loop: bool = False,
    requires_loop: bool = False,
) -> bool:
    """Executable mirror of the pre-draft Effect/Shake source-window gate."""

    if dependency_start_ms < source_start_ms:
        return False
    if play_ms:
        source_end_ms = source_start_ms + play_ms
        if dependency_start_ms >= source_end_ms:
            return False
        if dependency_end_ms is not None and dependency_end_ms > source_end_ms:
            return False
    if dependency_end_ms is not None and dependency_end_ms <= dependency_start_ms:
        return False
    return not requires_loop or loop


def fit_hold_chain_to_stage(cuts_ms: list[int], stage_ms: int) -> list[int]:
    """Executable mirror of the Workbench's preserve-edges HOLD fitter."""

    if sum(cuts_ms) <= stage_ms:
        return cuts_ms.copy()
    if len(cuts_ms) < 3:
        raise ValueError("no deterministic middle HOLD window")
    middle_count = len(cuts_ms) - 2
    edge_ms = cuts_ms[0] + cuts_ms[-1]
    if edge_ms + middle_count > stage_ms:
        raise ValueError("no positive middle HOLD window")
    fitted = cuts_ms.copy()
    remaining_budget = stage_ms - edge_ms
    remaining_requested = sum(cuts_ms[1:-1])
    for index in range(1, len(cuts_ms) - 1):
        remaining_count = len(cuts_ms) - index - 2
        allocation = remaining_budget
        if remaining_count:
            proportional = (
                1
                if not remaining_requested
                else remaining_budget * cuts_ms[index] // remaining_requested
            )
            allocation = max(
                1, min(proportional, remaining_budget - remaining_count)
            )
        fitted[index] = allocation
        remaining_budget -= allocation
        remaining_requested -= min(remaining_requested, cuts_ms[index])
    return fitted


class ActionCompositionWorkbenchRegressionOracles(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.animation_cpp = read("Client/Private/Animation_Tool.cpp")
        cls.animation_h = read("Client/Public/Animation_Tool.h")
        cls.workbench_h = read("Client/Public/ActionCompositionWorkbench.h")
        cls.workbench_cpp = read("Client/Private/ActionCompositionWorkbench.cpp")
        cls.blueprint_cpp = read(
            "Client/Private/ActionCompositionWorkbench_Blueprint.cpp"
        )
        cls.timeline_h = read("Client/Public/ActionPresentationTimeline.h")
        cls.timeline_cpp = read("Client/Private/ActionPresentationTimeline.cpp")
        cls.balance_cpp = read("Client/Private/BalanceTool.cpp")
        cls.boss_cpp = read("Client/Private/BossTool.cpp")
        cls.main_cpp = read("Client/Private/MainApp.cpp")
        cls.valtan_cpp = read("Client/Private/Valtan.cpp")
        cls.presentation = json.loads(read("Data/Valtan/Valtan.presentation.json"))
        cls.gameplay = json.loads(read("Data/Valtan/Valtan.gameplay.json"))

    def test_create_pattern_tab_latches_its_canonical_load_before_reload(self) -> None:
        creator = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanCompositionPatternCreator()",
        )
        guard = creator.index("if (!m_bValtanPatternMasterLoadAttempted)")
        latch = creator.index(
            "m_bValtanPatternMasterLoadAttempted = true;", guard
        )
        reload_call = creator.index("Reload_ValtanPatternMaster()", guard)
        self.assertLess(guard, latch)
        self.assertLess(latch, reload_call)

    def test_sequence_intake_routes_to_creator_then_back_to_created_pattern(self) -> None:
        browser = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_SequenceBrowser(",
        )
        patterns = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_PatternsWindow(",
        )
        render = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render()",
        )
        intake = browser.index("Stage_ValtanCompositionIntakeSequence(")
        route_to_creator = browser.index("m_iRequestedPatternTab = 1", intake)
        self.assertLess(intake, route_to_creator)
        self.assertIn("ImGuiTabItemFlags_SetSelected", patterns)
        self.assertIn('"Create New Pattern"', patterns)
        self.assertIn("m_iRequestedPatternTab = -1", patterns)

        created = render.index("Consume_ValtanCompositionPatternCreated(")
        route_to_browser = render.index("m_iRequestedPatternTab = 0", created)
        reload_canonical = render.index("Reload_Canonical()", created)
        self.assertLess(route_to_browser, reload_canonical)

    def test_details_owns_a_deferred_pattern_save_without_stale_frame_views(self) -> None:
        details = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_Details(",
        )
        render = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render()",
        )
        self.assertIn("UNSAVED PATTERN DRAFT", details)
        self.assertIn('ImGui::Button("Save & Apply##CompositionDetails")', details)
        self.assertIn("m_bSavePatternRequested = true", details)

        resources = render.index("Render_ResourcesWindow(")
        consume = render.index("if (m_bSavePatternRequested)")
        save = render.index("Save_Publish_Reload()", consume)
        self.assertLess(resources, consume)
        self.assertLess(consume, save)
        self.assertNotIn("Render_", render[save:])

        self.assertIn("LAST SAVE: SAVED - APPLY STATUS BELOW", details)
        self.assertIn("LAST SAVE & APPLY: FAILED", details)
        self.assertIn("m_strPatternSaveStatus", details)
        self.assertIn("m_bPatternSaveSucceeded = Save_Publish_Reload()", render)

    def test_sound_save_admission_uses_the_complete_canonical_graph(self) -> None:
        """Non-playable legacy rows still own admitted Pattern Sound cues."""

        sound_rows = json.loads(
            read("Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json")
        )["cues"]
        encounter_patterns = json.loads(
            read("Data/Encounters/Valtan/ValtanEncounter.json")
        )["patterns"]
        sound_pattern_ids = {row["patternId"] for row in sound_rows}
        encounter_ids = {row["patternId"] for row in encounter_patterns}
        managed_ids = {
            row["patternId"]
            for row in self.gameplay["patterns"]
        }
        self.assertTrue(sound_pattern_ids <= encounter_ids)
        self.assertTrue(sound_pattern_ids - managed_ids)
        self.assertTrue(
            any(
                row["patternId"] == "VALTAN_BACKSTEP_ATTACK"
                and row["stageId"] == "SWEEP"
                for row in sound_rows
            )
        )
        self.assertTrue(
            any(
                row["patternId"] == "VALTAN_BACKSTEP_ATTACK"
                for row in encounter_patterns
            )
        )
        self.assertNotIn("VALTAN_BACKSTEP_ATTACK", managed_ids)

        complete = function_body(
            self.workbench_cpp,
            "Collect_CanonicalPatternsForDependencyValidation() const",
        )
        self.assertIn("m_CanonicalView.Gimmicks", complete)
        self.assertIn("m_CanonicalView.Rotation", complete)
        self.assertNotIn("m_PlayableInventory.Contains", complete)

        for signature in (
            "Validate_ManualStageTopologySoundDependencies(",
            "bool_t Client::CActionCompositionWorkbench::Save_Publish_Reload()",
        ):
            body = function_body(self.workbench_cpp, signature)
            self.assertIn(
                "Collect_CanonicalPatternsForDependencyValidation()", body
            )

    def test_native_model_window_admission_precedes_every_animation_mutation(self) -> None:
        for token in (
            "Resolve_ValtanCompositionNativeClipDurationMs",
            "Validate_ValtanCompositionAnimationStageMutation",
            "Validate_ValtanCompositionAnimationGraphMutations",
        ):
            self.assertIn(token, self.animation_h)
            self.assertIn(token, self.animation_cpp)
        self.assertIn("Validate_AuthoredSourceWindow", self.timeline_h)
        strict = function_body(
            self.timeline_cpp,
            "bool Client::CActionPresentationTimeline::Validate_AuthoredSourceWindow(",
        )
        self.assertIn("std::floor(fRemainingMs + 0.5)", strict)
        self.assertNotIn("std::min", strict)

        wrapper = function_body(
            self.workbench_cpp,
            "bool_t SetValtanStageDraftWithSoundDependencyAdmission(",
        )
        self.assertLess(
            wrapper.index("Validate_ValtanCompositionAnimationStageMutation"),
            wrapper.index("Validate_ValtanCompositionPatternSoundStageDependencies"),
        )
        self.assertLess(
            wrapper.index("Validate_ValtanCompositionPatternSoundStageDependencies"),
            wrapper.index("Set_ValtanStageDraft"),
        )

        legacy = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Render_ValtanStageDraftInspector(",
        )
        self.assertLess(
            legacy.index("Validate_ValtanCompositionAnimationStageMutation"),
            legacy.index("Validate_ValtanCompositionPatternSoundStageDependencies"),
        )
        self.assertLess(
            legacy.index("Validate_ValtanCompositionPatternSoundStageDependencies"),
            legacy.index("Set_ValtanStageDraft"),
        )

        save = function_body(
            self.workbench_cpp,
            "bool_t Client::CActionCompositionWorkbench::Save_Publish_Reload()",
        )
        self.assertLess(
            save.index("Validate_ValtanCompositionAnimationGraphMutations"),
            save.index("Validate_ValtanCompositionPatternSoundGraphDependencies"),
        )
        self.assertLess(
            save.index("Validate_ValtanCompositionPatternSoundGraphDependencies"),
            save.index("Save_ValtanCanonicalProduct"),
        )

    def test_extracted_hold_chain_fits_the_existing_stage_with_exact_slots(self) -> None:
        apply = function_body(
            self.workbench_cpp,
            "bool_t Client::CActionCompositionWorkbench::Apply_SelectedSequenceToStage(",
        )
        self.assertLess(
            apply.index('"WAIT" == Stage.strSequenceRole'),
            apply.index("Get_ValtanStageDraft"),
        )
        self.assertIn("must remain Animation NONE", apply)
        self.assertIn("Resolve_ValtanCompositionNativeClipDurationMs", apply)
        self.assertIn("FitCompositionSequenceCutsToStage(", apply)
        self.assertIn('"HOLD" != Selected->strMode', apply)
        self.assertIn("bDeterministicHoldChain", apply)
        self.assertIn("3u == Selected->Clips.size()", apply)
        self.assertIn('"start" == ClipReplacementRole(', apply)
        self.assertIn('"loop" == ClipReplacementRole(', apply)
        self.assertIn('"end" == ClipReplacementRole(', apply)
        self.assertIn("while (iRemainingMs > NativeDurationsMs[iClip])", apply)
        self.assertIn('"loop" != ClipReplacementRole(', apply)
        self.assertIn("start/end/one-shot clips are never repeated", apply)
        self.assertIn("Slot.repeatUntilStageEnd = false", apply)
        self.assertIn('"EXACT" : "HOLD_LAST_POSE"', apply)
        self.assertNotIn("Draft.durationMs = static_cast<uint32_t>(iDurationMs);\n\tif (!Set", apply)
        self.assertNotIn("Insert_ValtanManualStageAfter", apply)

        fitted = fit_hold_chain_to_stage([1833, 3000, 2000], 5000)
        self.assertEqual([1833, 1167, 2000], fitted)
        self.assertEqual(5000, sum(fitted))
        self.assertTrue(all(value <= native for value, native in zip(
            fitted, [1833, 1333, 2000]
        )))

    def test_no_model_sequence_intake_is_a_strict_265_row_join(self) -> None:
        sequence_lines = read(
            "Data/Animation/Reference/Valtan/Valtan.clipseq"
        ).splitlines()
        cut_lines = read(
            "Data/Animation/Reference/Valtan/Valtan.clipcuts"
        ).splitlines()
        self.assertEqual('LOSTARK_CLIP_SEQ 2 "Valtan" 265', sequence_lines[0])
        self.assertEqual('LOSTARK_CLIP_CUTS 1 "Valtan" 265', cut_lines[0])

        sequence_pattern = re.compile(
            r'^(\d+) "([^"]+)" seq=(\d+) mode=(SEQUENCE|HOLD|COMBO) clips="([^"]+)"$'
        )
        cut_pattern = re.compile(r'^(\d+) seq=(\d+) cuts="([^"]+)"$')
        sequences: dict[tuple[int, int], tuple[str, list[str]]] = {}
        cuts: dict[tuple[int, int], list[float]] = {}
        for line in sequence_lines[1:]:
            match = sequence_pattern.fullmatch(line)
            self.assertIsNotNone(match, line)
            assert match is not None
            key = (int(match.group(1)), int(match.group(3)))
            self.assertNotIn(key, sequences)
            clips = match.group(5).split(",")
            self.assertTrue(all(re.fullmatch(r"[A-Za-z0-9_.-]+", clip) for clip in clips))
            sequences[key] = (match.group(4), clips)
        skipped_cut_count = 0
        for line in cut_lines[1:]:
            match = cut_pattern.fullmatch(line)
            self.assertIsNotNone(match, line)
            assert match is not None
            key = (int(match.group(1)), int(match.group(2)))
            self.assertNotIn(key, cuts)
            row = [float(value) for value in match.group(3).split(",")]
            self.assertTrue(all(math.isfinite(value) and 0.0 <= value <= 600.0 for value in row))
            skipped_cut_count += sum(value == 0.0 for value in row)
            cuts[key] = row
        self.assertEqual(265, len(sequences))
        self.assertGreater(skipped_cut_count, 0)
        self.assertEqual(sequences.keys(), cuts.keys())
        for key, (_mode, clips) in sequences.items():
            self.assertEqual(len(clips), len(cuts[key]), key)

        loader = function_body(
            self.animation_cpp,
            "bool_t Load_ValtanCompositionSequenceLibrary(",
        )
        getter = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Get_ValtanCompositionSequences(",
        )
        workbench_reload = function_body(
            self.workbench_cpp,
            "bool_t Client::CActionCompositionWorkbench::Reload_AnimationSequences()",
        )
        for body in (loader, getter, workbench_reload):
            self.assertNotIn("Resolve_Model()", body)
            self.assertNotIn("Stage_ValtanCompositionPreview", body)
        self.assertIn('Resolve(L"Animation/Reference/Valtan")', loader)
        self.assertIn("iCutCount != iSequenceCount", loader)
        self.assertIn("Cuts->second.size() != Sequence.Clips.size()", loader)
        self.assertIn("if (Cuts->second[iClip] <= 0.f)", loader)
        self.assertIn("TimedClips.push_back", loader)

    def test_animation_wall_policy_matches_runtime_integer_budget(self) -> None:
        gameplay = {row["patternId"]: row for row in self.gameplay["patterns"]}
        policies: set[str] = set()
        stage_count = 0
        for pattern in self.presentation["patterns"]:
            gameplay_stages = {
                row["stageId"]: row
                for row in gameplay[pattern["patternId"]]["stages"]
            }
            for stage in pattern["stages"]:
                animation = stage["animation"]
                if animation.get("mode", "CLIP_SEQUENCE") == "NONE":
                    continue
                policy = animation["endPolicy"]
                policies.add(policy)
                slots = [
                    (
                        row["playMs"],
                        row["playRate"],
                        row["repeatUntilStageEnd"],
                    )
                    for row in animation["occurrences"]
                ]
                self.assertTrue(
                    wall_contract_accepts(
                        policy,
                        gameplay_stages[stage["stageId"]]["durationMs"],
                        slots,
                    ),
                    f'{pattern["patternId"]}/{stage["stageId"]}',
                )
                stage_count += 1
        self.assertGreater(stage_count, 100)
        self.assertEqual(
            {"EXACT", "HOLD_LAST_POSE", "LOOP_TO_STAGE_END"}, policies
        )

        self.assertTrue(wall_contract_accepts("EXACT", 100, [(100, 1.0, False)]))
        self.assertTrue(wall_contract_accepts("HOLD_LAST_POSE", 150, [(100, 1.0, False)]))
        self.assertTrue(
            wall_contract_accepts(
                "LOOP_TO_STAGE_END", 150, [(100, 1.0, False), (0, 1.0, True)]
            )
        )
        self.assertFalse(wall_contract_accepts("EXACT", 150, [(100, 1.0, False)]))
        self.assertFalse(
            wall_contract_accepts(
                "HOLD_LAST_POSE", 150, [(100, 1.0, False), (0, 1.0, True)]
            )
        )
        # The previous float-sum validator accepted 32 * 0.6ms ~= 19ms,
        # while the runtime assigned 32 * round(0.6ms) = 32ms. It must reject.
        self.assertFalse(
            wall_contract_accepts("EXACT", 19, [(6, 10.0, False)] * 32)
        )

        setter = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Set_ValtanStageDraft(",
        )
        self.assertIn("uint64_t iKnownWallMs", setter)
        self.assertIn("std::llround", setter)
        self.assertNotIn("double fKnownWallMs", setter)
        self.assertIn("candidate.animationSlots.back().playMs", setter)
        self.assertIn("static_cast<int64_t>(iKnownWallMs) > 0", setter)

    def test_animation_occurrence_timing_is_a_typed_sequencer_edit(self) -> None:
        apply_timing = function_body(
            self.workbench_cpp,
            "bool_t Client::CActionCompositionWorkbench::Apply_AnimationOccurrenceTiming(",
        )
        inline_editor = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_SelectedAnimationTiming(",
        )
        timeline = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_Timeline(",
        )
        build_timeline = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Build_Timeline(",
        )
        wrapper = function_body(
            self.workbench_cpp,
            "bool_t SetValtanStageDraftWithSoundDependencyAdmission(",
        )

        for token in (
            '"WAIT" == Stage.strSequenceRole',
            "1u != iMatchCount",
            "Slot.clipOccurrenceId == strClipOccurrenceId",
            "Found->sourceStartMs = iSourceStartMs",
            "Found->playMs = iPlayMs",
            "SetValtanStageDraftWithSoundDependencyAdmission",
        ):
            self.assertIn(token, apply_timing)
        for forbidden in (
            "Save_Valtan",
            "Draft.durationMs =",
            "Draft.animationEndPolicy =",
            "Found->clipOccurrenceId =",
            "Found->mappingBasis =",
            "Found->repeatUntilStageEnd =",
        ):
            self.assertNotIn(forbidden, apply_timing)

        self.assertIn('InputInt(\n\t\t"Source Start (ms)"', inline_editor)
        self.assertIn('InputInt(\n\t\t"Play Duration (ms)"', inline_editor)
        self.assertGreaterEqual(
            inline_editor.count("ImGui::IsItemDeactivatedAfterEdit()"),
            2,
            "Animation timing owner edits must commit after each scalar interaction ends",
        )
        self.assertIn("Apply_AnimationOccurrenceTiming", inline_editor)
        self.assertIn("StageDraft.animationEditable && !Slot.repeatUntilStageEnd", build_timeline)
        self.assertIn("DETAIL_OWNER::ANIMATION == Item.eOwner", timeline)
        self.assertIn("m_strTimelineTrimStableId == Item.strStableId", timeline)
        self.assertIn("iNewWallMs) *\n\t\t\t\t\t\t\t\tSlot->playRate", timeline)
        self.assertIn("Apply_AnimationOccurrenceTiming", timeline)

        effect_validation = wrapper.index(
            '"Effect occurrence " + Cue.strOccurrenceId'
        )
        shake_validation = wrapper.index(
            '"Pattern Shake occurrence " + Cue.strOccurrenceId'
        )
        typed_commit = wrapper.rindex("Set_ValtanStageDraft")
        self.assertLess(effect_validation, shake_validation)
        self.assertLess(shake_validation, typed_commit)

        self.assertTrue(source_dependency_window_accepts(100, 200, 100))
        self.assertTrue(source_dependency_window_accepts(100, 200, 299, 300))
        self.assertFalse(source_dependency_window_accepts(100, 200, 99))
        self.assertFalse(source_dependency_window_accepts(100, 200, 300))
        self.assertFalse(source_dependency_window_accepts(100, 200, 250, 301))
        self.assertFalse(
            source_dependency_window_accepts(
                100, 0, 150, loop=False, requires_loop=True
            )
        )
        self.assertTrue(
            source_dependency_window_accepts(
                100, 0, 150, loop=True, requires_loop=True
            )
        )

    def test_stale_preserved_renders_only_the_pinned_canonical_generation(self) -> None:
        render = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render()",
        )
        build_timeline = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Build_Timeline(",
        )
        gameplay_details = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_GameplayStageDetails(",
        )
        animation_details = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_AnimationStageDetails(",
        )
        inline_timing = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_SelectedAnimationTiming(",
        )
        details = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_Details(",
        )
        browser = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_Browser(",
        )

        effective_ready = render.index("bool_t bEffectivePatternReady =")
        effective_lookup = render.index("Get_ValtanPatternDraft", effective_ready)
        self.assertIn(
            "ADMISSION_STATE::ADMITTED == m_eAdmission",
            render[effective_ready:effective_lookup],
        )
        draft_dirty = render.index("m_bAuthoringDraftDirty =")
        self.assertIn(
            "ADMISSION_STATE::ADMITTED == m_eAdmission",
            render[draft_dirty:effective_ready],
        )

        timeline_lookup = build_timeline.index("Get_ValtanStageDraft")
        self.assertIn(
            "ADMISSION_STATE::ADMITTED == m_eAdmission",
            build_timeline[:timeline_lookup],
        )
        for body in (gameplay_details, animation_details, inline_timing):
            stale_gate = body.index(
                "ADMISSION_STATE::ADMITTED != m_eAdmission"
            )
            draft_lookup = body.index("Get_ValtanStageDraft")
            self.assertLess(stale_gate, draft_lookup)
            self.assertIn("return;", body[stale_gate:draft_lookup])
        self.assertIn("Pinned canonical generation (read only)", gameplay_details)
        self.assertIn("Pinned canonical Animation generation", animation_details)
        self.assertIn("Pinned source %u ms", inline_timing)

        for body in (build_timeline, details):
            sound_lookup = body.index(
                "Get_ValtanCompositionPatternSoundDraft"
            )
            self.assertIn(
                "ADMISSION_STATE::ADMITTED != m_eAdmission",
                body[:sound_lookup],
            )
        self.assertNotIn("Get_ValtanStageDurationDraft", browser)
        self.assertIn("std::to_string(Stage.iDurationMs)", browser)

    def test_product_display_and_full_source_join_have_separate_admission(self) -> None:
        reload_body = function_body(
            self.workbench_cpp,
            "bool_t Client::CActionCompositionWorkbench::Reload_Canonical()",
        )
        render = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render()",
        )

        product_load = reload_body.index("CValtanPatternTree::Load_WhileAdmitted")
        inventory = reload_body.index("Build_PlayablePatternInventory")
        source_reload = reload_body.index("Reload_ValtanSource")
        authoring_revision = reload_body.index("Get_ValtanAuthoringState")
        canonical_revision = reload_body.index("Get_ValtanCanonicalSourceRevision")
        generation_join = reload_body.index(
            "Verify_ValtanCanonicalSourceRevision_WhileAdmitted"
        )
        sound_join = reload_body.index("Reload_ValtanCompositionPatternSounds")
        final_generation_check = reload_body.rindex("Validate_StillCurrent")
        product_commit = reload_body.rindex("m_CanonicalView = std::move(Staged)")
        authoring_pin = reload_body.index(
            "m_strPinnedAuthoringSourceRevision = std::move(AuthoringRevision)"
        )
        canonical_pin = reload_body.index(
            "m_strPinnedCanonicalSourceRevision ="
        )
        admitted = reload_body.rindex("m_eAdmission = ADMISSION_STATE::ADMITTED")
        self.assertLess(product_load, inventory)
        self.assertLess(inventory, source_reload)
        self.assertLess(source_reload, authoring_revision)
        self.assertLess(authoring_revision, canonical_revision)
        self.assertLess(canonical_revision, generation_join)
        self.assertLess(generation_join, sound_join)
        self.assertLess(sound_join, final_generation_check)
        self.assertLess(final_generation_check, product_commit)
        self.assertLess(product_commit, authoring_pin)
        self.assertLess(authoring_pin, canonical_pin)
        self.assertLess(canonical_pin, admitted)

        self.assertIn("PreserveOrCommitReadOnlyProduct", reload_body)
        read_only = function_body(reload_body, "[this, &CanonicalAdmission")
        read_only_shake_load = read_only.index(
            "CValtanPatternShakeCueDocument::Load_Source"
        )
        read_only_sound_load = read_only.index(
            "CValtanCombatObjectSoundCueDocument::Load_Source"
        )
        read_only_generation_check = read_only.index("Validate_StillCurrent")
        read_only_product_commit = read_only.index(
            "m_CanonicalView = std::move(Staged)"
        )
        self.assertLess(read_only_shake_load, read_only_generation_check)
        self.assertLess(read_only_sound_load, read_only_generation_check)
        self.assertLess(
            read_only_generation_check,
            read_only_product_commit,
        )
        self.assertLess(
            read_only_product_commit,
            read_only.index("m_PatternShakes = std::move(StagedPatternShakes)"),
        )
        self.assertLess(
            read_only_product_commit,
            read_only.index(
                "m_CombatObjectSounds = std::move(StagedCombatObjectSounds)"
            ),
        )
        self.assertLess(
            read_only.index("0u != m_CanonicalView.Get_PatternCount()"),
            read_only.index("if (!bPreserved)"),
        )
        self.assertIn("PRODUCT_ONLY / canonical Product read admission", read_only)
        self.assertIn("previous display snapshot was preserved", read_only)
        self.assertIn("m_eAdmission = ADMISSION_STATE::STALE_PRESERVED", read_only)
        self.assertNotIn("m_strPinnedAuthoringSourceRevision =", read_only)
        self.assertIn("m_strPinnedAuthoringSourceRevision.clear()", read_only)
        self.assertIn("FULL_JOIN / canonical Product", reload_body)
        full_join_shake_load = reload_body.rindex(
            "CValtanPatternShakeCueDocument::Load_Source"
        )
        full_join_sound_load = reload_body.rindex(
            "CValtanCombatObjectSoundCueDocument::Load_Source"
        )
        self.assertLess(full_join_shake_load, final_generation_check)
        self.assertLess(full_join_sound_load, final_generation_check)
        self.assertLess(
            final_generation_check,
            reload_body.rindex(
                "m_PatternShakes = std::move(StagedPatternShakes)"
            ),
        )
        self.assertNotIn(
            "Load_Source(\n\t\tm_PatternShakes", reload_body
        )
        self.assertNotIn(
            "Load_Source(\n\t\t\tm_CombatObjectSounds", reload_body
        )

        effective_ready = render.index("bool_t bEffectivePatternReady =")
        effective_lookup = render.index("Get_ValtanPatternDraft", effective_ready)
        self.assertIn(
            "ADMISSION_STATE::ADMITTED == m_eAdmission",
            render[effective_ready:effective_lookup],
        )
        for pin in (
            "m_strPinnedAuthoringSourceRevision",
            "m_strPinnedCanonicalSourceRevision",
        ):
            self.assertIn(pin, render)

    def test_sound_owner_reload_discard_and_retry_require_fresh_admission(self) -> None:
        details = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_Details(",
        )
        sound_start = details.index(
            "if (DETAIL_OWNER::SOUND == m_eDetailOwner)"
        )
        sound = details[sound_start:]
        self.assertIn(
            "ADMISSION_STATE::ADMITTED == m_eAdmission && bMutationAdmitted",
            sound,
        )
        runtime_apply = function_body(sound, "[this](std::string& strOutStatus)")
        self.assertIn("ADMISSION_STATE::ADMITTED != m_eAdmission", runtime_apply)
        self.assertIn("STALE_PRESERVED is display-only", runtime_apply)
        self.assertIn(
            "const bool_t bSoundSourceCommitAdmitted =\n"
            "\t\t\tbFreshSoundOwnerAdmitted &&",
            sound,
        )
        sound_commit_gate = sound.index(
            "const bool_t bSoundSourceCommitAdmitted ="
        )
        self.assertIn(
            "Can_CommitValtanCompositionPatternSoundGeneration",
            sound[sound_commit_gate : sound_commit_gate + 400],
        )
        for label in (
            "Reload Sound Owner",
            "Confirm Discard + Reload Sound Owner",
            "Retry Apply Saved Sound",
        ):
            label_at = sound.index(label)
            guarded = sound.rfind("bSoundSourceCommitAdmitted", 0, label_at)
            self.assertNotEqual(-1, guarded, label)
            self.assertLess(label_at - guarded, 260, label)

    def test_dirty_reload_preserves_current_admission_and_draft(self) -> None:
        reload_body = function_body(
            self.workbench_cpp,
            "bool_t Client::CActionCompositionWorkbench::Reload_Canonical()",
        )
        dirty_start = reload_body.index("if (m_pBalanceTool->Is_ValtanDraftDirty())")
        dirty_end = reload_body.index("\n\t\t}", dirty_start)
        dirty_branch = reload_body[dirty_start:dirty_end]
        self.assertIn("return false", dirty_branch)
        self.assertIn("current admitted draft remains editable", dirty_branch)
        for forbidden in (
            "m_eAdmission =",
            "m_CanonicalView =",
            "Reload_ValtanSource",
        ):
            self.assertNotIn(forbidden, dirty_branch)
        self.assertLess(dirty_start, reload_body.index("Reload_ValtanSource"))

    def test_capture_collider_remove_is_blocked_and_attachment_is_serialized(self) -> None:
        capture_rows = []
        for pattern in self.gameplay["patterns"]:
            for stage in pattern["stages"]:
                hit = stage.get("hit")
                if isinstance(hit, dict) and hit.get("playerResponse") == "CAPTURE":
                    capture_rows.append((pattern["patternId"], stage["stageId"], hit))
        self.assertGreaterEqual(len(capture_rows), 3)
        for pattern_id, stage_id, hit in capture_rows:
            self.assertNotEqual("NONE", hit["shape"]["kind"], (pattern_id, stage_id))
            self.assertEqual("BOSS_LEFT_HAND", hit["attachmentSlot"])
            self.assertEqual(0.0, hit["pushRangeM"])
            self.assertEqual(0, hit["pushMs"])
            self.assertFalse(hit["knockdown"])
            self.assertEqual(0, hit["downMs"])

        setter = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Set_ValtanStageDraft(",
        )
        capture_reject = setter.index(
            '"CAPTURE" == current.playerResponse && "NONE" != current.hitShape'
        )
        none_commit = setter.index('stage->strHitShape = candidate.hitShape')
        self.assertLess(capture_reject, none_commit)
        self.assertIn("Geometry-only removal is blocked", setter)
        serializer_start = self.balance_cpp.index(
            'hit << ", \\"playerResponse\\": \\"CAPTURE\\""'
        )
        serializer = self.balance_cpp[serializer_start : serializer_start + 260]
        self.assertIn('\\"attachmentSlot\\"', serializer)
        self.assertIn("stage.strAttachmentSlot", serializer)

    def test_grab_release_direction_matches_projector_and_server_contract(self) -> None:
        setter = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Set_ValtanStageDraft(",
        )
        details = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_GameplayStageDetails(",
        )
        self.assertIn("const bool launchDirectionValid", setter)
        self.assertIn(
            '"OPPOSITE_KNOCKBACK" == draftAction.strReleaseMode &&', setter
        )
        self.assertIn("0.f == draftAction.fYawOffsetDegrees", setter)
        self.assertIn(
            '"ARENA_EJECTION" == draftAction.strReleaseMode &&', setter
        )
        self.assertIn("bHoldRelease", details)
        self.assertIn("bArenaEjection", details)
        self.assertIn("ImGui::BeginDisabled(!bArenaEjection)", details)
        self.assertIn(
            "Use ARENA_EJECTION for an authored rotation offset", details
        )

    def test_collider_damage_profile_is_part_of_the_typed_hit_change_set(self) -> None:
        setter = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Set_ValtanStageDraft(",
        )
        hit_change_start = setter.index("const bool hitChanged =")
        hit_change_end = setter.index(";", hit_change_start)
        hit_change = setter[hit_change_start:hit_change_end]
        self.assertIn(
            "candidate.damageProfileId != current.damageProfileId",
            hit_change,
            "a Damage Profile-only edit must not be reported as an unchanged Collider",
        )
        self.assertIn(
            "stage->strServerDamageProfileId = candidate.damageProfileId",
            setter,
        )
        unchanged_gate = setter.index(
            "if (!stageKindChanged && !durationChanged && !hitChanged"
        )
        self.assertLess(
            setter.index("candidate.damageProfileId != current.damageProfileId"),
            unchanged_gate,
        )
        self.assertLess(
            unchanged_gate,
            setter.index("stage->strServerDamageProfileId = candidate.damageProfileId"),
        )

        self.assertIn("if (hitChanged && !current.hitEditable)", setter)
        self.assertNotIn("hitContractTransition", setter)
        self.assertIn("Add/remove is available only to a manual audition Stage", setter)

    def test_local_preview_stages_draft_animation_and_hit_before_play(self) -> None:
        stage = function_body(
            self.valtan_cpp,
            "bool_t CValtan::Stage_LocalPatternAuthoringPreview(",
        )
        apply_sample = function_body(
            self.valtan_cpp,
            "bool_t CValtan::Apply_PatternPresentationSample(",
        )
        start = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Start_ValtanPatternMasterPreview(",
        )
        self.assertIn("Stage.ClipOccurrences", stage)
        self.assertIn("Stage.iHitCount", stage)
        self.assertIn("StagedBindings.emplace(Stage.strActionId", stage)
        self.assertIn("StagedHitAreas.emplace(Stage.strActionId", stage)
        self.assertLess(
            stage.index("StagedBindings.emplace"),
            stage.index("m_LocalPreviewClipByActionId = std::move(StagedBindings)"),
        )
        self.assertIn(
            "m_bLocalPatternAuthoringPreview ?\n\t\t\tm_LocalPreviewClipByActionId : m_PatternClipByActionId",
            apply_sample,
        )
        self.assertLess(
            start.index("Stage_LocalPatternAuthoringPreview"),
            start.index("m_ValtanPatternMasterPlaylist = std::move"),
        )

    def test_legacy_offline_preview_is_display_only_when_admission_is_stale(self) -> None:
        start = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Start_ValtanPatternMasterPreview(",
        )
        self.assertIn("VALTAN_PATTERN_MASTER_ADMISSION_STATE::ADMITTED", start)
        self.assertIn("display-only", start)
        self.assertLess(
            start.index("VALTAN_PATTERN_MASTER_ADMISSION_STATE::ADMITTED"),
            start.index("Build_ValtanPatternMasterTimeline"),
        )

    def test_create_success_is_a_one_shot_canonical_refresh(self) -> None:
        poll = function_body(
            self.animation_cpp,
            "void Client::CAnimation_Tool::Poll_ValtanPatternCreateCommand()",
        )
        consume = function_body(
            self.animation_cpp,
            "bool_t Client::CAnimation_Tool::Consume_ValtanCompositionPatternCreated(",
        )
        render = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render()",
        )
        self.assertNotIn("Save_ValtanProduct", poll)
        for prerequisite in (
            "bBalanceReloaded",
            "bIntakeReloaded",
            "bAnimationAdmitted",
            "bBossReloaded",
            "bSelected",
        ):
            self.assertIn(prerequisite, poll)
        self.assertLess(
            poll.index("const bool_t bReloadClosureAdmitted"),
            poll.index("m_bValtanCompositionPatternCreatedPending = true"),
        )
        self.assertIn("m_bValtanCompositionPatternCreatedPending = false", consume)
        self.assertIn("m_strValtanCompositionPatternCreatedId.clear()", consume)
        self.assertIn("Consume_ValtanCompositionPatternCreated", render)
        self.assertLess(
            render.index("Consume_ValtanCompositionPatternCreated"),
            render.index("Select_Pattern(*pCreated)"),
        )
        self.assertIn("m_eAdmission = ADMISSION_STATE::STALE_PRESERVED", render)

    def test_save_publishes_candidate_before_any_local_consumer_reload(self) -> None:
        save = function_body(
            self.workbench_cpp,
            "bool_t Client::CActionCompositionWorkbench::Save_Publish_Reload()",
        )
        canonical = save.index("Save_ValtanCanonicalProduct")
        boss_reload = save.index("Reload_CanonicalGraph")
        workbench_reload = save.index("if (!Reload_Canonical())")
        candidate_publish = save.index("Save_ValtanProduct")
        self.assertLess(canonical, candidate_publish)
        self.assertLess(candidate_publish, boss_reload)
        self.assertLess(boss_reload, workbench_reload)
        self.assertIn("SAVE & APPLY: data files saved", save)
        self.assertIn("Server apply status follows", save)
        self.assertIn("exact saved revision is active", save)
        self.assertIn("presentation changes can require re-entry", save)

        balance_save = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Save_ValtanProduct(",
        )
        apply_failure = balance_save.index("if (!Apply_ValtanRevision(stepStatus))")
        rerecord = balance_save.index(
            "Record_GameplaySourceActivationExpectation(", apply_failure
        )
        failure_status = balance_save.index(
            "Live apply was not submitted", apply_failure
        )
        self.assertLess(apply_failure, rerecord)
        self.assertLess(rerecord, failure_status)

    def test_workbench_frame_gate_is_memory_only_and_gap_is_typed(self) -> None:
        toolbar = function_body(
            self.workbench_cpp,
            "bool_t Client::CActionCompositionWorkbench::Render_Toolbar(",
        )
        observe = function_body(
            self.boss_cpp,
            "bool_t Client::CBossTool::Observe_ServerActivePatternRevision(",
        )
        exact = function_body(
            self.boss_cpp,
            "bool_t Client::CBossTool::Get_ServerActivePatternRevision(",
        )
        self.assertIn("Observe_ServerActivePatternRevision(", toolbar)
        self.assertNotIn("Get_ServerActivePatternRevision(", toolbar)
        self.assertNotIn("Is_CurrentPresentationBaselineIntact", observe)
        self.assertNotIn("Acquire_Receipt", observe)
        self.assertIn("Is_CurrentPresentationBaselineIntact", exact)

        gap = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_SelectedStageGapControl(",
        )
        for token in (
            '"Selected Stage Gap (ms)"',
            "ComputeExactAnimationWallMs",
            "Draft.durationMs",
            '"EXACT" : "HOLD_LAST_POSE"',
            "SetValtanStageDraftWithSoundDependencyAdmission",
            "Invalidate_TimelineCache()",
        ):
            self.assertIn(token, gap)
        timeline = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_Timeline(",
        )
        self.assertIn("Render_SelectedStageGapControl", timeline)

    def test_workbench_is_seven_independent_unconstrained_domain_windows(self) -> None:
        render = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render()",
        )
        renderers = (
            "Render_PatternsWindow",
            "Render_PreviewWindow",
            "Render_SequencerWindow",
            "Render_DetailsWindow",
            "Render_ResourcesWindow",
            "Render_SessionWindow",
            "Render_BossPatternWindow",
        )
        for renderer in renderers:
            self.assertIn(renderer, self.workbench_h)

            self.assertEqual(
                1,
                render.count(renderer + "("),
                f"Render must dispatch the independent {renderer} exactly once",
            )
            source = (
                self.blueprint_cpp
                if renderer == "Render_BossPatternWindow"
                else self.workbench_cpp
            )
            body = function_body(
                source, f"Client::CActionCompositionWorkbench::{renderer}("
            )
            self.assertIn("ImGui::Begin(", body, renderer)
            self.assertIn("ImGui::End();", body, renderer)

        for monolithic_layout_token in (
            "##ActionCompositionOuterLayout",
            "##CompositionMainColumn",
            "##CompositionBrowserPreview",
            "##CompositionDetails",
            "ImGui::BeginChild(",
            "ImGui::BeginTable(",
            "ImGuiTableColumnFlags_WidthFixed",
            "m_fDetailsWidth",
            "m_fBrowserWidth",
            "fMainHeight * 0.54f",
            "280.f, 680.f",
        ):
            self.assertNotIn(monolithic_layout_token, render)

        for hard_constraint in (
            "ImGui::SetNextWindowSizeConstraints",
            "ImGuiWindowFlags_AlwaysAutoResize",
            "ImGuiWindowFlags_NoResize",
        ):
            self.assertNotIn(hard_constraint, self.workbench_cpp)
        self.assertIn("ImGuiCond_FirstUseEver", self.workbench_cpp)
        self.assertIn("ImGuiCond_FirstUseEver", self.blueprint_cpp)

    def test_lane_plus_opens_one_large_typed_resource_picker(self) -> None:
        request = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Request_LaneAuthoring(",
        )
        resources = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_ResourcesWindow(",
        )
        for token in (
            "m_bResourcesWindowExpandRequested = true",
            "m_bResourcesWindowFocusRequested = true",
            "m_strResourceTargetPatternId = Pattern.strPatternId",
            "m_strResourceTargetStageId = Stage.strStageId",
        ):
            self.assertIn(token, request)
        for token in (
            "ImGui::SetNextWindowFocus()",
            "ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always)",
            'ImGui::Button("Append V1 Effect to Pattern Draft")',
            'ImGui::Button("Append + Save V2 Stage Binding")',
            'ImGui::Button("Append Selected Sound to Stage")',
            "Render_SequenceBrowser(",
            'ImGui::SeparatorText("Sound Event Tree")',
            "RenderResourceTree(",
        ):
            self.assertIn(token, resources)
        self.assertNotIn(
            "Resolve_ValtanCompositionPatternSoundWindow(", resources
        )

    def test_boss_pattern_graph_is_queue_only_and_generation_cached(self) -> None:
        render = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render()",
        )
        session = render.index("Render_SessionWindow(")
        graph = render.index("Render_BossPatternWindow(")
        patterns = render.index("Render_PatternsWindow(")
        deferred_save = render.index("if (m_bSavePatternRequested)")
        self.assertLess(session, graph)
        self.assertLess(graph, patterns)
        self.assertLess(patterns, deferred_save)

        window = function_body(
            self.blueprint_cpp,
            "void Client::CActionCompositionWorkbench::Render_BossPatternWindow(",
        )
        self.assertIn("m_bSavePatternRequested = true", window)
        self.assertNotIn("Save_Publish_Reload", window)
        attempted = window.index("m_bBossPatternGraphAttempted = true")
        project = window.index("CActionCompositionGraphModel::Project(")
        self.assertLess(attempted, project)
        for forbidden in (
            "std::filesystem",
            "ifstream",
            "DataJson",
            "CProjectDataRoot",
            "Reload_Canonical",
            "Get_ValtanPatternDraft",
            "CEffectCatalog",
        ):
            self.assertNotIn(forbidden, window)

    def test_boss_pattern_selection_reuses_details_and_preview_only_routes(self) -> None:
        window = function_body(
            self.blueprint_cpp,
            "void Client::CActionCompositionWorkbench::Render_BossPatternWindow(",
        )
        self.assertIn("Select_Stage(*pPattern, Stage", window)
        self.assertIn('Stage.strStageId + "/branch/" + Edge.strOutcome', window)
        self.assertIn("m_BossPatternOutcomeOverrides", window)
        self.assertIn("++m_iBossPatternRouteGeneration", window)
        self.assertNotIn("m_iPlayheadMs =", window)
        self.assertIn("Effect/Animation/Sound/Camera stay on Sequencer lanes", window)

        topology = function_body(
            self.blueprint_cpp,
            "Render_BossPatternStageTopologyControls(",
        )
        for typed_writer in (
            "Can_Edit_ValtanManualStageTopology",
            "Insert_ValtanManualStageAfter",
            "Move_ValtanManualStage",
            "Remove_ValtanManualStage",
            "Validate_ManualStageTopologySoundDependencies",
        ):
            self.assertIn(typed_writer, topology)

    def test_boss_pattern_window_is_opt_in_with_explicit_entry_points(self) -> None:
        browser = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_Browser(",
        )
        menu = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_WindowMenu()",
        )
        open_valtan = function_body(
            self.workbench_cpp,
            "bool_t Client::CActionCompositionWorkbench::Open_Valtan()",
        )
        self.assertIn('ImGui::Button("Open Boss Pattern")', browser)
        self.assertIn('ImGui::MenuItem("Boss Pattern"', menu)
        self.assertIn("m_bBossPatternWindowVisible = true", menu)
        self.assertNotIn("m_bBossPatternWindowVisible", open_valtan)
        self.assertIn(
            "Composition Boss Pattern###CompositionBossPatternWindow",
            self.blueprint_cpp,
        )

    def test_sequencer_renders_the_seven_typed_categories_once_each(self) -> None:
        lane_enum = re.search(
            r"enum class TIMELINE_LANE\s*:\s*uint8_t\s*\{(?P<body>.*?)\};",
            self.workbench_h,
            re.DOTALL,
        )
        self.assertIsNotNone(lane_enum)
        assert lane_enum is not None
        enum_entries = re.findall(r"\b([A-Z][A-Z_]*)\s*,?", lane_enum.group("body"))
        self.assertEqual(
            [
                "STAGE",
                "ANIMATION",
                "EFFECT",
                "SOUND",
                "LOGIC",
                "COLLIDER",
                "CAMERA",
                "COUNT",
            ],
            enum_entries,
        )

        timeline = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_Timeline(",
        )
        lane_order = re.search(
            r"TIMELINE_LANE_ORDER\s*=\s*\{(?P<body>.*?)\};",
            timeline,
            re.DOTALL,
        )
        self.assertIsNotNone(lane_order)
        assert lane_order is not None
        self.assertEqual(
            [
                "STAGE",
                "ANIMATION",
                "EFFECT",
                "SOUND",
                "LOGIC",
                "COLLIDER",
                "CAMERA",
            ],
            re.findall(
                r"TIMELINE_LANE::([A-Z_]+)", lane_order.group("body")
            ),
        )
        lane_labels = function_body(
            self.workbench_cpp,
            "const char_t* Client::CActionCompositionWorkbench::Lane_Label(",
        )
        for label in (
            "Stage",
            "Animation",
            "Effect",
            "Sound",
            "Logic",
            "Collider",
            "Camera",
        ):
            self.assertEqual(1, lane_labels.count(f'return "{label}"'))
        self.assertRegex(
            timeline,
            r"for\s*\([^\n]*TIMELINE_LANE_ORDER[^\n]*\)",
        )
        self.assertEqual(
            1, timeline.count("ImGui::TextUnformatted(Lane_Label(eLane))")
        )
        self.assertNotIn("Item.strLane", timeline)

    def test_timeline_cache_build_is_generation_driven_not_per_frame(self) -> None:
        render = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render()",
        )
        self.assertNotIn("Build_Timeline(", render)
        self.assertIn("Ensure_TimelineCache(", render)

        cache = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Ensure_TimelineCache(",
        )
        self.assertIn("Get_ValtanDraftGeneration", cache)
        self.assertIn("m_strTimelineCachePatternId", cache)
        self.assertIn("m_iTimelineCacheDraftGeneration", cache)
        build_at = cache.index("Build_Timeline(")
        cache_gate = cache[:build_at]
        self.assertIn("m_strTimelineCachePatternId", cache_gate)
        self.assertIn("m_iTimelineCacheDraftGeneration", cache_gate)
        self.assertIn("return;", cache_gate)

    def test_resource_catalogs_load_only_in_the_open_resource_domain(self) -> None:
        open_valtan = function_body(
            self.workbench_cpp,
            "bool_t Client::CActionCompositionWorkbench::Open_Valtan()",
        )
        reload_canonical = function_body(
            self.workbench_cpp,
            "bool_t Client::CActionCompositionWorkbench::Reload_Canonical()",
        )
        render = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render()",
        )
        for body in (open_valtan, reload_canonical, render):
            self.assertNotIn("Reload_AnimationSequences(", body)
            self.assertNotIn("Reload_SemanticValtanEffects(", body)

        sequence_browser = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_SequenceBrowser(",
        )
        automatic_animation = sequence_browser.index(
            "if (!m_bAnimationSequenceLoadAttempted)"
        )
        first_animation_load = sequence_browser.index(
            "Reload_AnimationSequences(", automatic_animation
        )
        refresh_animation = sequence_browser.index(
            'ImGui::Button("Reload Animation Sequences")'
        )
        second_animation_load = sequence_browser.index(
            "Reload_AnimationSequences(", refresh_animation
        )
        self.assertLess(automatic_animation, first_animation_load)
        self.assertLess(refresh_animation, second_animation_load)

        resources_signature = (
            "void Client::CActionCompositionWorkbench::Render_ResourcesWindow("
        )
        self.assertIn(resources_signature, self.workbench_cpp)
        resources = function_body(self.workbench_cpp, resources_signature)
        self.assertIn("&m_bResourcesWindowVisible", resources)
        animation_tab = resources.index('"Animation", nullptr,')
        animation_browser = resources.index("Render_SequenceBrowser(", animation_tab)
        self.assertLess(animation_tab, animation_browser)
        self.assertEqual(
            2,
            self.workbench_cpp.count("Render_SequenceBrowser("),
            "the Animation resource browser must have one definition and one Resource-window call",
        )
        effect_tab = resources.index('"Effect", nullptr,')
        lazy_effect = resources.index(
            "if (!m_bSemanticValtanEffectLoadAttempted)", effect_tab
        )
        first_effect_load = resources.index(
            "Reload_SemanticValtanEffects(", lazy_effect
        )
        refresh_effect = resources.index(
            'ImGui::Button("Refresh Effect Catalog")', first_effect_load
        )
        second_effect_load = resources.index(
            "Reload_SemanticValtanEffects(", refresh_effect
        )
        self.assertLess(effect_tab, lazy_effect)
        self.assertLess(lazy_effect, first_effect_load)
        self.assertLess(refresh_effect, second_effect_load)
        effect_reload = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Reload_SemanticValtanEffects()",
        )
        self.assertIn(
            "m_bSemanticValtanEffectLoadAttempted = true", effect_reload
        )

    def test_workbench_next_and_flow_routes_use_the_existing_server_owners(self) -> None:
        toolbar = function_body(
            self.workbench_cpp,
            "bool_t Client::CActionCompositionWorkbench::Render_Toolbar(",
        )
        queue = function_body(
            self.boss_cpp,
            "bool_t Client::CBossTool::Queue_NextServerPattern(",
        )
        open_flow = function_body(
            self.main_cpp,
            "bool_t CMainApp::Debug_OpenBossPatternFlow(",
        )
        self.assertIn('ImGui::Button("Queue as Next")', toolbar)
        self.assertIn("Queue_NextServerPattern", toolbar)
        self.assertIn('ImGui::Button("Pattern Flow...")', toolbar)
        self.assertIn("Debug_OpenBossPatternFlow", toolbar)

        self.assertIn("Acquire_ServerPlaybackAdmission", queue)
        self.assertIn("Can_QueueNextPattern", queue)
        self.assertIn("m_bNextPatternInventoryReady", queue)
        self.assertIn("Queue_NextPattern", queue)
        self.assertLess(
            queue.index("Acquire_ServerPlaybackAdmission"),
            queue.index("Queue_NextPattern"),
        )
        self.assertIn("m_bRepeat = false", queue)

        self.assertIn("EnsureDebugTool(DEBUG_TOOL::BOSS)", open_flow)
        self.assertIn("Open_PatternFlow", open_flow)
        self.assertNotIn("CValtanPatternFlowDocument", toolbar)

    def test_reload_save_end_the_frame_and_sound_save_balances_disabled_stack(self) -> None:
        toolbar = function_body(
            self.workbench_cpp,
            "bool_t Client::CActionCompositionWorkbench::Render_Toolbar(",
        )
        session_signature = (
            "bool_t Client::CActionCompositionWorkbench::Render_SessionWindow("
        )
        self.assertIn(session_signature, self.workbench_cpp)
        session = function_body(self.workbench_cpp, session_signature)
        open_valtan = function_body(
            self.workbench_cpp,
            "bool_t Client::CActionCompositionWorkbench::Open_Valtan()",
        )
        details = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_Details(",
        )

        self.assertIn('ImGui::Button("Reload Canonical")', toolbar)
        self.assertIn("return true", toolbar)
        self.assertIn("bCanonicalViewMayHaveChanged", toolbar)
        self.assertIn("Render_Toolbar", session)
        self.assertIn("ImGui::End();", session)
        self.assertIn("ADMISSION_STATE::ADMITTED != m_eAdmission", open_valtan)

        sound_button = details.index('ImGui::Button("Save Sound Owner")')
        sound_end_disabled = details.index("ImGui::EndDisabled();", sound_button)
        sound_return = details.index("if (bSoundSaveRequested)", sound_button)
        self.assertLess(sound_end_disabled, sound_return)

    def test_manual_pattern_stage_role_is_a_typed_detail_not_an_id_rewrite(self) -> None:
        details = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_GameplayStageDetails(",
        )
        setter = function_body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Set_ValtanStageDraft(",
        )
        self.assertIn("Draft.stageKindEditable", details)
        self.assertIn('"Stage Role"', details)
        for kind in ('"ACTIVE"', '"WINDUP"', '"GROGGY"'):
            self.assertIn(kind, details)
            self.assertIn(kind, setter)
        self.assertIn("pattern->bManualServerAudition", setter)
        self.assertIn("stage->strStageKind = candidate.stageKind", setter)
        self.assertNotIn("stage->strStageId =", setter)
        self.assertNotIn("stage->strActionId =", setter)

    def test_timeline_trim_never_rebuilds_the_vector_being_iterated(self) -> None:
        timeline = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_Timeline(",
        )
        self.assertIn(
            "const std::vector<TIMELINE_ITEM>& TimelineItems = m_TimelineItems",
            timeline,
        )
        self.assertIn("iItem < TimelineItems.size()", timeline)
        self.assertIn("const TIMELINE_ITEM& Item = TimelineItems[iItem]", timeline)
        self.assertIn("Apply_AnimationOccurrenceTiming", timeline)
        self.assertIn(
            "Invalidate_TimelineCache()",
            timeline,
            "successful trim should invalidate the joined view and rebuild on the next frame",
        )
        self.assertNotIn(
            "Build_Timeline(*pPattern)",
            timeline,
            "the lane vector must remain stable throughout the render pass",
        )

    def test_lane_add_routes_to_typed_details_and_lazy_resource_domains(self) -> None:
        timeline = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_Timeline(",
        )
        request = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Request_LaneAuthoring(",
        )
        resources = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_ResourcesWindow(",
        )
        self.assertIn('ImGui::SmallButton("+##LaneAuthoring")', timeline)
        self.assertIn("Request_LaneAuthoring(eLane", timeline)
        for lane in (
            "STAGE",
            "ANIMATION",
            "EFFECT",
            "SOUND",
            "LOGIC",
            "COLLIDER",
            "CAMERA",
        ):
            self.assertIn(f"TIMELINE_LANE::{lane}", request)
        self.assertIn("m_bResourceDomainSelectionRequested = true", request)
        self.assertIn("ImGuiTabItemFlags_SetSelected", resources)
        self.assertIn('"Open Counter -> Groggy Detail"', resources)
        self.assertIn('"Open Motion Detail"', resources)
        self.assertIn('"Open Grab Release Detail"', resources)

    def test_pattern_browser_is_the_complete_play_inventory_projection(self) -> None:
        collect = function_body(
            self.workbench_cpp,
            "Client::CActionCompositionWorkbench::Collect_Patterns() const",
        )
        self.assertEqual(
            2,
            collect.count("m_PlayableInventory.Contains(Pattern.strPatternId)"),
            "both canonical Pattern categories must be filtered by the admitted Boss inventory",
        )
        for category in ("m_CanonicalView.Gimmicks", "m_CanonicalView.Rotation"):
            category_at = collect.index(category)
            inventory_at = collect.index(
                "m_PlayableInventory.Contains(Pattern.strPatternId)", category_at
            )
            append_at = collect.index("Patterns.push_back(&Pattern)", inventory_at)
            self.assertLess(category_at, inventory_at)
            self.assertLess(inventory_at, append_at)

    def test_selection_is_local_and_complete_play_is_an_explicit_command(self) -> None:
        select_pattern = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Select_Pattern(",
        )
        select_stage = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Select_Stage(",
        )
        for selection in (select_pattern, select_stage):
            self.assertNotIn("Debug_SelectCompletePlayPattern", selection)
            self.assertNotIn("Debug_CompletePlaySelected", selection)

        self.assertNotIn("Invalidate_EffectivePatternCache", select_pattern)
        self.assertNotIn("Build_Timeline(", select_pattern)
        self.assertIn("Invalidate_TimelineCache()", select_pattern)

        toolbar = function_body(
            self.workbench_cpp,
            "bool_t Client::CActionCompositionWorkbench::Render_Toolbar(",
        )
        command_edge = toolbar.index(
            'ImGui::Button("Play Saved Active Revision on Server Valtan")'
        )
        select_edge = toolbar.index("Debug_SelectCompletePlayPattern", command_edge)
        play_edge = toolbar.index("Debug_CompletePlaySelected", select_edge)
        self.assertLess(command_edge, select_edge)
        self.assertLess(select_edge, play_edge)
        sequence_browser = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_SequenceBrowser(",
        )
        resource_command = sequence_browser.index(
            '"Play Owning Saved Active Revision on Server Valtan"'
        )
        resource_select = sequence_browser.index(
            "Debug_SelectCompletePlayPattern", resource_command
        )
        resource_play = sequence_browser.index(
            "Debug_CompletePlaySelected", resource_select
        )
        self.assertLess(resource_command, resource_select)
        self.assertLess(resource_select, resource_play)
        self.assertEqual(2, self.workbench_cpp.count("Debug_SelectCompletePlayPattern"))

    def test_resource_searches_cache_indices_and_clip_only_visible_rows(self) -> None:
        for member in (
            "m_FilteredAnimationSequenceIndices",
            "m_bAnimationSequenceFilterDirty",
            "m_strAnimationSequenceFilterQuery",
            "m_FilteredEffectAssetIndices",
            "m_bEffectFilterDirty",
            "m_strEffectFilterQuery",
        ):
            self.assertIn(member, self.workbench_h)

        animation_reload = function_body(
            self.workbench_cpp,
            "bool_t Client::CActionCompositionWorkbench::Reload_AnimationSequences()",
        )
        self.assertIn("m_FilteredAnimationSequenceIndices.clear()", animation_reload)
        self.assertIn("m_bAnimationSequenceFilterDirty = true", animation_reload)
        animation = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_SequenceBrowser(",
        )
        animation_gate = animation.index("if (m_bAnimationSequenceFilterDirty")
        animation_filter = animation.index("ContainsInsensitive", animation_gate)
        animation_commit = animation.index(
            "m_bAnimationSequenceFilterDirty = false", animation_filter
        )
        animation_status = animation.index('"%zu matching source Sequences"')
        selected_sequence = animation.index(
            'ImGui::SeparatorText("Selected Sequence")'
        )
        preview_sequence = animation.index(
            'ImGui::Button("Preview Sequence on Arena Clone")'
        )
        animation_tree = animation.index(
            'ImGui::SeparatorText("Animation Sequence Tree")',
            animation_commit,
        )
        self.assertLess(animation_gate, animation_filter)
        self.assertLess(animation_filter, animation_commit)
        self.assertLess(animation_commit, animation_status)
        self.assertLess(animation_status, selected_sequence)
        self.assertLess(selected_sequence, preview_sequence)
        self.assertLess(preview_sequence, animation_tree)
        preview_call = animation.index(
            "if (m_pAnimationTool->Preview_ValtanCompositionSequence(",
            preview_sequence,
        )
        preview_claim = animation.index(
            "m_bPreviewOwnerClaimRequested = true", preview_call
        )
        self.assertLess(
            preview_call,
            preview_claim,
        )
        self.assertNotIn(
            "m_bPreviewOwnerClaimRequested = true",
            animation[preview_sequence:preview_call],
        )
        for selected_action in (
            'ImGui::Button("Replace Stage Slots")',
            'ImGui::Button("Append to Stage Slots")',
            'ImGui::Button("Use for Create New Pattern")',
        ):
            self.assertLess(animation.index(selected_action), animation_tree)
        self.assertIn(
            "m_AnimationSequences[iSequence]", animation[animation_tree:]
        )
        self.assertIn("RenderResourceTree(", animation[animation_tree:])
        self.assertIn("std::to_string(Sequence.iSequenceIndex)", animation)
        self.assertNotIn("std::vector<std::size_t> Filtered", animation)

        effect_reload = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Reload_SemanticValtanEffects()",
        )
        self.assertIn("m_FilteredEffectAssetIndices.clear()", effect_reload)
        self.assertIn("m_bEffectFilterDirty = true", effect_reload)
        resources = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_ResourcesWindow(",
        )
        effect_tab = resources.index('"Effect", nullptr,')
        effect_gate = resources.index("if (m_bEffectFilterDirty", effect_tab)
        effect_filter = resources.index("ContainsInsensitive", effect_gate)
        effect_commit = resources.index("m_bEffectFilterDirty = false", effect_filter)
        effect_tree = resources.index(
            'ImGui::SeparatorText("All Effect Resources")', effect_commit
        )
        self.assertLess(effect_gate, effect_filter)
        self.assertLess(effect_filter, effect_commit)
        self.assertLess(effect_commit, effect_tree)
        self.assertIn(
            "for (const std::size_t i : m_FilteredEffectAssetIndices)",
            resources[effect_filter:effect_commit],
        )
        self.assertIn("RenderResourceTree(", resources[effect_tree:])
        self.assertIn("m_EffectV2DocumentResourceTree", resources[effect_tree:])
        self.assertIn("m_EffectV2GroupResourceTree", resources[effect_tree:])
        self.assertNotIn("std::vector<std::size_t> Filtered", resources)

    def test_source_sequence_server_owner_index_is_primary_only_and_cached(
        self,
    ) -> None:
        index = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Ensure_SourceSequenceOwnerIndex()",
        )
        generation_gate = index.index(
            "m_iSourceSequenceOwnerIndexGeneration =="
        )
        full_inventory_scan = index.index("Collect_Patterns()")
        generation_commit = index.index(
            "m_iSourceSequenceOwnerIndexGeneration = m_iCanonicalDisplayGeneration"
        )
        self.assertLess(generation_gate, full_inventory_scan)
        self.assertLess(full_inventory_scan, generation_commit)
        self.assertIn('if ("PRIMARY" != Source.strRole)', index)

        lookup = function_body(
            self.workbench_cpp,
            "Client::CActionCompositionWorkbench::Find_SourceSequenceOwners(",
        )
        self.assertIn("std::lower_bound", lookup)

        animation = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_SequenceBrowser(",
        )
        self.assertIn("Ensure_SourceSequenceOwnerIndex()", animation)
        self.assertIn("Find_SourceSequenceOwners(", animation)
        self.assertNotIn("Collect_Patterns()", animation)
        self.assertNotIn("PresentationSources", animation)

        reload_canonical = function_body(
            self.workbench_cpp,
            "bool_t Client::CActionCompositionWorkbench::Reload_Canonical()",
        )
        self.assertEqual(2, reload_canonical.count("++m_iCanonicalDisplayGeneration"))
        self.assertEqual(2, reload_canonical.count("Invalidate_SourceSequenceOwnerIndex()"))

        # This exact source is PRIMARY for real owners but only REFERENCE for
        # the terrain-destruction Pattern.  Reverse mapping must never promote
        # that provenance row into a Server Play owner.
        source_action_id = 420624
        sequence_index = 1
        reference_pattern_id = "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK"
        rows = {
            pattern["patternId"]: pattern["presentationSources"]
            for pattern in self.presentation["patterns"]
        }
        self.assertTrue(
            any(
                source["sourceActionId"] == source_action_id
                and source["sequenceIndex"] == sequence_index
                and source["role"] == "REFERENCE"
                for source in rows[reference_pattern_id]
            )
        )
        primary_owner_ids = {
            pattern_id
            for pattern_id, sources in rows.items()
            if any(
                source["sourceActionId"] == source_action_id
                and source["sequenceIndex"] == sequence_index
                and source["role"] == "PRIMARY"
                for source in sources
            )
        }
        self.assertTrue(primary_owner_ids)
        self.assertNotIn(reference_pattern_id, primary_owner_ids)

    def test_saved_source_owner_server_play_ignores_unrelated_balance_draft(
        self,
    ) -> None:
        animation = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_SequenceBrowser(",
        )
        gate_start = animation.index("const bool_t bCanPlayServerPattern")
        gate_end = animation.index("ImGui::BeginDisabled", gate_start)
        gate = animation[gate_start:gate_end]
        self.assertNotIn("m_bAuthoringDraftDirty", gate)
        self.assertNotIn("bMutationAdmitted", gate)
        self.assertIn("bServerRevisionAdmitted", gate)
        self.assertIn("bSoundRuntimeReady", gate)
        self.assertIn("SAVED ACTIVE REVISION", animation)
        self.assertIn("Unsaved local Pattern drafts are not sent", animation)

    def test_timeline_clocks_are_bounded_and_edits_commit_at_interaction_edges(self) -> None:
        timeline = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_Timeline(",
        )
        self.assertRegex(
            self.workbench_cpp,
            r"(?:TIMELINE[^\n=]*MAX|MAX[^\n=]*TIMELINE)[^\n=]*=\s*600000u?",
            "the visible/editable Pattern clock needs one explicit ten-minute cap",
        )
        canvas_at = timeline.index("const float fCanvasWidth")
        child_at = timeline.index("ImGui::BeginChild", canvas_at)
        canvas_clock = timeline[canvas_at:child_at]
        self.assertNotIn(
            "m_iTimelineDurationMs",
            canvas_clock,
            "canvas width must consume the already capped clock",
        )
        ruler_at = timeline.index('"##TimelineRuler"')
        ruler_end = timeline.index("const std::vector<TIMELINE_ITEM>& TimelineItems", ruler_at)
        ruler = timeline[ruler_at:ruler_end]
        self.assertRegex(ruler, r"for\s*\(\s*(?:const\s+)?uint64_t\s+")
        self.assertNotRegex(ruler, r"for\s*\(\s*(?:const\s+)?uint32_t\s+")
        self.assertNotIn("<= m_iTimelineDurationMs", ruler)

        pattern_duration = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_PatternDurationControl(",
        )
        pattern_input = pattern_duration.index("ImGui::InputInt(")
        pattern_commit_edge = pattern_duration.index(
            "ImGui::IsItemDeactivatedAfterEdit()", pattern_input
        )
        pattern_mutation = pattern_duration.index(
            "SetValtanStageDraftWithSoundDependencyAdmission", pattern_commit_edge
        )
        self.assertLess(pattern_input, pattern_commit_edge)
        self.assertLess(pattern_commit_edge, pattern_mutation)

        stage_gap = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_SelectedStageGapControl(",
        )
        gap_input = stage_gap.index("ImGui::InputInt(")
        gap_commit_edge = stage_gap.index(
            "ImGui::IsItemDeactivatedAfterEdit()", gap_input
        )
        gap_mutation = stage_gap.index(
            "SetValtanStageDraftWithSoundDependencyAdmission", gap_commit_edge
        )
        self.assertLess(gap_input, gap_commit_edge)
        self.assertLess(gap_commit_edge, gap_mutation)

        self.assertIn("ImGui::IsMouseReleased(ImGuiMouseButton_Left)", timeline)
        typed_drag_commits = (
            "SetValtanStageDraftWithSoundDependencyAdmission(",
            "Apply_AnimationOccurrenceTiming(",
            "Apply_EffectOccurrenceTiming(",
            "Apply_PatternSoundOccurrenceTiming(",
        )
        for token in typed_drag_commits:
            for match in re.finditer(re.escape(token), timeline):
                latest_release = timeline.rfind(
                    "ImGui::IsMouseReleased(ImGuiMouseButton_Left)", 0, match.start()
                )
                latest_down = timeline.rfind(
                    "ImGui::IsMouseDown(ImGuiMouseButton_Left)", 0, match.start()
                )
                self.assertGreater(
                    latest_release,
                    latest_down,
                    f"{token} must commit on release, not mutate the owner every drag frame",
                )

    def test_scalar_duration_edit_never_uses_unsupported_enter_flag(self) -> None:
        duration = function_body(
            self.workbench_cpp,
            "void Client::CActionCompositionWorkbench::Render_PatternDurationControl(",
        )
        self.assertIn("ImGui::InputInt(", duration)
        self.assertIn("ImGui::IsItemDeactivatedAfterEdit()", duration)
        self.assertNotIn("ImGuiInputTextFlags_EnterReturnsTrue", duration)


if __name__ == "__main__":
    unittest.main()
