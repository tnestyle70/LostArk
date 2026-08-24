#!/usr/bin/env python3

from __future__ import annotations

import json
from pathlib import Path
import unittest


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
CPP_PATH = REPOSITORY_ROOT / "Client/Private/Animation_Tool.cpp"
HEADER_PATH = REPOSITORY_ROOT / "Client/Public/Animation_Tool.h"
MASTER_PATH = REPOSITORY_ROOT / "Data/Valtan/Valtan.pattern.json"

EXPECTED_PATTERN_ORDER = (
    "VALTAN_WHIRLWIND",
    "VALTAN_DASH_CHARGE",
    "VALTAN_FOUR_SLASH",
    "VALTAN_FIST_IN_OUT",
    "VALTAN_HIGH_JUMP",
    "VALTAN_FLOOR_WIPE_130",
    "VALTAN_ARENA_BREAK_109",
)


def function_slice(text: str, signature: str, next_signature: str) -> str:
    start = text.index(signature)
    end = text.index(next_signature, start + len(signature))
    return text[start:end]


class AnimationToolValtanPatternMasterContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.cpp = CPP_PATH.read_text(encoding="utf-8")
        cls.header = HEADER_PATH.read_text(encoding="utf-8")
        cls.master = json.loads(MASTER_PATH.read_text(encoding="utf-8"))

    def test_master_contains_the_seven_animation_tool_patterns(self) -> None:
        self.assertEqual("lostark.valtan-pattern-master", self.master["schema"])
        self.assertEqual(1, self.master["formatVersion"])
        self.assertEqual(
            list(EXPECTED_PATTERN_ORDER),
            [pattern["patternId"] for pattern in self.master["patterns"]],
        )

    def test_animation_tool_loads_the_shared_typed_projection(self) -> None:
        combined = self.header + "\n" + self.cpp
        for token in (
            '#include "ValtanPatternTree.h"',
            "VALTAN_PATTERN_TREE_VIEW m_ValtanPatternMasterView;",
            "CValtanPatternTree::Load(Staged, Status)",
            "bAuthoringMasterManaged",
            "expected exactly 7 managed patterns",
            '"Valtan Pattern Master (Authoritative)"',
            "Data/Valtan/Valtan.pattern.json",
        ):
            self.assertIn(token, combined)

    def test_complete_timeline_uses_server_wall_and_source_clocks(self) -> None:
        build = function_slice(
            self.cpp,
            "bool_t Client::CAnimation_Tool::Build_ValtanPatternMasterTimeline(",
            "bool_t Client::CAnimation_Tool::Start_ValtanPatternMasterPreview(",
        )
        apply_pose = function_slice(
            self.cpp,
            "bool_t Client::CAnimation_Tool::Apply_ValtanPatternMasterPose(",
            "bool_t Client::CAnimation_Tool::Activate_ValtanPatternMasterItem(",
        )
        for token in (
            "Clip.iAuthoringWallMs",
            "Clip.iSourceStartMs",
            "Clip.iPlayMs",
            "Clip.fPlayRate",
            "Clip.bLoop",
            "iStageAnimationWallMs != Stage.iDurationMs",
            "Stage.iAuthoringRepeatCount",
            "iPlayableOccurrenceCount",
        ):
            self.assertIn(token, build)
        for token in (
            "CActionPresentationTimeline::Resolve_Sample",
            "Item.iSourceStartMs",
            "Item.iPlayMs",
            "Item.fPlayRate",
            "Item.bRepeatUntilStageEnd",
            "Set_AnimTrackPosition",
            "Play_Animation(0.f)",
        ):
            self.assertIn(token, apply_pose)

    def test_dash_paths_come_from_the_admitted_branch_graph(self) -> None:
        build = function_slice(
            self.cpp,
            "bool_t Client::CAnimation_Tool::Build_ValtanPatternMasterTimeline(",
            "bool_t Client::CAnimation_Tool::Start_ValtanPatternMasterPreview(",
        )
        self.assertIn("CValtanPatternTree::Build_PreviewStagePath(", build)
        self.assertIn("VALTAN_PATTERN_PREVIEW_PATH", self.header)
        self.assertNotIn('{ "WINDUP", "CHARGE", "RECOVERY" }', build)
        self.assertIn('"Dash authoring path##ValtanPatternMaster"', self.cpp)

    def test_source_provenance_and_end_policy_are_visible(self) -> None:
        self.assertIn("pSelected->PresentationSources", self.cpp)
        self.assertIn("Source.iSourceActionId", self.cpp)
        self.assertIn("Source.iSequenceIndex", self.cpp)
        self.assertIn("Stage.strAnimationEndPolicy", self.cpp)
        policies = {
            stage["animation"]["endPolicy"]
            for pattern in self.master["patterns"]
            for stage in pattern["stages"]
        }
        self.assertEqual(
            {"EXACT", "HOLD_LAST_POSE", "LOOP_TO_STAGE_END"}, policies
        )

    def test_seek_and_one_active_preview_are_explicit(self) -> None:
        start_master = function_slice(
            self.cpp,
            "bool_t Client::CAnimation_Tool::Start_ValtanPatternMasterPreview(",
            "bool_t Client::CAnimation_Tool::Apply_ValtanPatternMasterPose(",
        )
        start_source = function_slice(
            self.cpp,
            "bool_t Client::CAnimation_Tool::Start_ValtanSequencePreview(",
            "bool_t Client::CAnimation_Tool::Start_ValtanPatternPreview(",
        )
        self.assertLess(
            start_master.index("Build_ValtanPatternMasterTimeline("),
            start_master.index("Reset_ValtanPatternPreviewState("),
            "master must stage and validate before replacing the current pose",
        )
        self.assertIn("Reset_ValtanPatternMasterPreviewState(", start_source)
        for token in (
            "Seek_ValtanPatternMasterPreview(",
            '"##ValtanPatternMasterTimeline"',
            "m_bValtanPatternMasterPaused",
            "Stop_ValtanPatternMasterPreview(",
            '"mesh_idle_battle_1"',
        ):
            self.assertIn(token, self.cpp)
        seek = function_slice(
            self.cpp,
            "bool_t Client::CAnimation_Tool::Seek_ValtanPatternMasterPreview(",
            "void Client::CAnimation_Tool::Advance_ValtanPatternMasterPreview(",
        )
        self.assertIn("std::nextafter(fLocalSeconds, 0.f)", seek)

    def test_historical_reference_is_retained_and_demoted(self) -> None:
        for token in (
            '"Secondary / Read-only Source Reference (1-67)"',
            "Valtan.patternpreview.json",
            "Valtan.clipseq",
            '"Valtan Source Reference (Read-only)"',
            "CValtanPatternPreviewDocument::Load(",
            "Start_ValtanSequencePreview(",
        ):
            self.assertIn(token, self.cpp)


if __name__ == "__main__":
    unittest.main()
