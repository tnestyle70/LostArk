#!/usr/bin/env python3
"""Focused contract for clip-bound Effect V2 rows in Composition.

This test intentionally reads only source/data.  It fixes the Workbench join
that maps one model-clip binding to each matching Pattern clip occurrence;
rendering continues to consume the already-loaded immutable V2 snapshot.
"""

from __future__ import annotations

import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
WORKBENCH = ROOT / "Client/Private/ActionCompositionWorkbench.cpp"
HEADER = ROOT / "Client/Public/ActionCompositionWorkbench.h"
PRESENTATION = ROOT / "Data/Valtan/Valtan.presentation.json"
BINDINGS = ROOT / "Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json"


def source_slice(source: str, begin: str, end: str) -> str:
    start = source.index(begin)
    finish = source.index(end, start)
    return source[start:finish]


class ActionCompositionEffectV2ClipProjectionContract(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.source = WORKBENCH.read_text(encoding="utf-8-sig")
        cls.header = HEADER.read_text(encoding="utf-8-sig")

    def test_stable_identity_owns_stage_or_clip_and_projected_occurrence(self) -> None:
        stable = source_slice(
            self.source,
            "std::string BuildEffectV2BindingStableId(",
            "const Client::EFFECT_V2_BINDING* ResolveEffectV2Binding(",
        )
        self.assertIn('Binding.strStage.empty() ? "clip/" : "stage/"', stable)
        self.assertIn("Binding.strClip", stable)
        self.assertIn("strClipOccurrenceId", stable)
        self.assertIn('StableId << "/occurrence/"', stable)

        resolver = source_slice(
            self.source,
            "const Client::EFFECT_V2_BINDING* ResolveEffectV2Binding(",
            "float ResolveTimelineDisplayWidthPx(",
        )
        self.assertIn("BuildEffectV2BindingStableId(", resolver)
        self.assertIn("strClipOccurrenceId", resolver)
        self.assertNotIn("Binding.strStage != strStageActionId", resolver)
        self.assertIn("strEffectV2ClipOccurrenceId", self.header)

    def test_timeline_projects_clip_source_clock_without_file_scans(self) -> None:
        timeline = source_slice(
            self.source,
            "void Client::CActionCompositionWorkbench::Build_Timeline(",
            "void Client::CActionCompositionWorkbench::Pack_TimelineSubrows(",
        )
        self.assertIn("Binding.strStage != Stage.strActionId", timeline)
        self.assertIn("Binding.strClip != strClipName", timeline)
        self.assertIn("StageDraft.animationSlots", timeline)
        self.assertIn("Stage.ClipOccurrences", timeline)
        self.assertIn("iSourceMs - iSourceStartMs", timeline)
        self.assertIn("fPlayRate", timeline)
        self.assertIn("bRepeatUntilStageEnd ?", timeline)
        self.assertIn("iStageDurationMs", timeline)
        self.assertIn('strLabel += " [each loop]"', timeline)
        self.assertNotIn("Reload_BossValtan", timeline)
        self.assertNotIn("recursive_directory_iterator", timeline)
        self.assertNotIn("ifstream", timeline)

    def test_duplicate_delete_move_and_box_detail_resolve_exact_clip_key(self) -> None:
        duplicate = source_slice(
            self.source,
            "bool_t Client::CActionCompositionWorkbench::Duplicate_SelectedTimelineBox(",
            "bool_t Client::CActionCompositionWorkbench::Delete_SelectedTimelineBox(",
        )
        delete = source_slice(
            self.source,
            "bool_t Client::CActionCompositionWorkbench::Delete_SelectedTimelineBox(",
            "bool_t Client::CActionCompositionWorkbench::Apply_AnimationOccurrenceTiming(",
        )
        details = source_slice(
            self.source,
            "void Client::CActionCompositionWorkbench::Render_Details(",
            "bool_t Client::CActionCompositionWorkbench::Validate_TimelineDependencyWindows(",
        )
        timeline_render = source_slice(
            self.source,
            "void Client::CActionCompositionWorkbench::Render_Timeline(",
            "void Client::CActionCompositionWorkbench::Render_SemanticLinkedRows(",
        )
        for block in (duplicate, delete, details, timeline_render):
            self.assertIn("strEffectV2ClipOccurrenceId", block)
            self.assertIn("ResolveEffectV2Binding(", block)
        self.assertIn("EFFECT_V2_STAGE_BINDING_KEY::From_Binding", details)
        self.assertIn('SeparatorText("Selected Effect V2 Box")', details)
        self.assertIn('"Clip source start (ms)"', details)
        self.assertIn("Item.fEffectV2ClipPlayRate", timeline_render)

    def test_valtan_struggling_v2_groups_have_exact_clip_join_targets(self) -> None:
        presentation = json.loads(PRESENTATION.read_text(encoding="utf-8-sig"))
        bindings = json.loads(BINDINGS.read_text(encoding="utf-8-sig"))["bindings"]
        struggling = next(
            pattern
            for pattern in presentation["patterns"]
            if pattern["patternId"] == "VALTAN_STRUGGLING"
        )
        occurrences = {
            occurrence["clip"]: (stage["stageId"], stage["animation"]["endPolicy"], occurrence)
            for stage in struggling["stages"]
            for occurrence in stage["animation"]["occurrences"]
        }
        occurrences_by_id = {
            occurrence["clipOccurrenceId"]: occurrence
            for stage in struggling["stages"]
            for occurrence in stage["animation"]["occurrences"]
        }
        expected_occurrences = {
            "mesh_att_battle_19_01": ("STEP_04", "EXACT", 5000, False),
            "mesh_att_battle_19_02": ("STEP_05", "EXACT", 1500, False),
            "mesh_att_battle_19_03": ("STEP_06", "LOOP_TO_STAGE_END", 0, True),
            "mesh_att_battle_19_04": ("STEP_07", "EXACT", 2600, False),
        }
        for clip, expected in expected_occurrences.items():
            stage_id, end_policy, occurrence = occurrences[clip]
            self.assertEqual(expected[0], stage_id)
            self.assertEqual(expected[1], end_policy)
            self.assertEqual(expected[2], occurrence["playMs"])
            self.assertEqual(expected[3], occurrence["repeatUntilStageEnd"])

        joined_rows = sorted(
            (
                occurrences_by_id[row["clock"]["clipOccurrenceId"]]["clip"],
                row["resource"]["id"],
                row["clock"]["startMs"],
            )
            for row in bindings
            if row["scope"]["patternId"] == "VALTAN_STRUGGLING"
            and row["resource"]["kind"] == "GROUP"
            and row["clock"]["basis"] == "CLIP_OCCURRENCE"
            and row["clock"]["clipOccurrenceId"] in occurrences_by_id
            and occurrences_by_id[row["clock"]["clipOccurrenceId"]]["clip"]
            in expected_occurrences
        )
        self.assertEqual(
            sorted([
                ("mesh_att_battle_19_01", "boss.valtan.impact", 1233),
                ("mesh_att_battle_19_01", "boss.valtan.impact", 2233),
                ("mesh_att_battle_19_01", "boss.valtan.impact", 3233),
                ("mesh_att_battle_19_01", "boss.valtan.impact", 4200),
                ("mesh_att_battle_19_02", "boss.valtan.pounding.chase", 0),
                ("mesh_att_battle_19_03", "boss.valtan.pounding", 200),
                ("mesh_att_battle_19_03", "boss.valtan.pounding", 400),
                ("mesh_att_battle_19_04", "boss.valtan.twohand", 1033),
            ]),
            joined_rows,
        )
        for clip, _group, start_ms in joined_rows:
            occurrence = occurrences[clip][2]
            if occurrence["repeatUntilStageEnd"]:
                continue
            self.assertLess(start_ms, occurrence["playMs"])


if __name__ == "__main__":
    unittest.main()
