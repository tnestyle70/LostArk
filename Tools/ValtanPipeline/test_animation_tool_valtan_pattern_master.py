#!/usr/bin/env python3

from __future__ import annotations

import json
from pathlib import Path
import unittest


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
CPP_PATH = REPOSITORY_ROOT / "Client/Private/Animation_Tool.cpp"
HEADER_PATH = REPOSITORY_ROOT / "Client/Public/Animation_Tool.h"
MASTER_PATH = REPOSITORY_ROOT / "Data/Valtan/Valtan.pattern.json"
PRESENTATION_PATH = REPOSITORY_ROOT / "Data/Valtan/Valtan.presentation.json"
GAMEPLAY_PATH = REPOSITORY_ROOT / "Data/Valtan/Valtan.gameplay.json"

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
        cls.presentation = json.loads(PRESENTATION_PATH.read_text(encoding="utf-8"))
        cls.gameplay = json.loads(GAMEPLAY_PATH.read_text(encoding="utf-8"))

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
            "expected the 7 baseline managed patterns",
            '"Valtan Pattern Master (Authoritative)"',
            "Data/Valtan/Valtan.gameplay.json + Valtan.presentation.json",
        ):
            self.assertIn(token, combined)

    def test_recovered_floor_wipe_and_arena_break_sequences_are_exact(self) -> None:
        patterns = {
            row["patternId"]: row
            for row in self.presentation["patterns"]
        }
        floor = patterns["VALTAN_FLOOR_WIPE_130"]
        self.assertEqual(
            [
                "mesh_att_battle_5_02_loop",
                "mesh_att_battle_5_02_end",
                "mesh_att_battle_5_04",
                "mesh_att_battle_15_02",
                "mesh_att_battle_15_03",
                "mesh_att_battle_15_04",
            ],
            [
                occurrence["clip"]
                for stage in floor["stages"]
                for occurrence in stage["animation"]["occurrences"]
            ],
        )
        floor_stages = {
            stage["stageId"]: stage
            for stage in floor["stages"]
        }
        self.assertEqual(
            ("HOLD_LAST_POSE", [("mesh_att_battle_5_02_end", 534, False)]),
            (
                floor_stages["FIRST_SMASH"]["animation"]["endPolicy"],
                [
                    (
                        occurrence["clip"],
                        occurrence["playMs"],
                        occurrence["repeatUntilStageEnd"],
                    )
                    for occurrence in floor_stages["FIRST_SMASH"]["animation"]["occurrences"]
                ],
            ),
        )
        self.assertEqual(
            (
                "HOLD_LAST_POSE",
                [
                    ("mesh_att_battle_5_04", 500, False),
                    ("mesh_att_battle_15_02", 1000, False),
                ],
            ),
            (
                floor_stages["INTERVAL"]["animation"]["endPolicy"],
                [
                    (
                        occurrence["clip"],
                        occurrence["playMs"],
                        occurrence["repeatUntilStageEnd"],
                    )
                    for occurrence in floor_stages["INTERVAL"]["animation"]["occurrences"]
                ],
            ),
        )
        self.assertEqual(
            ("EXACT", [("mesh_att_battle_15_03", 500, False)]),
            (
                floor_stages["SECOND_SMASH"]["animation"]["endPolicy"],
                [
                    (
                        occurrence["clip"],
                        occurrence["playMs"],
                        occurrence["repeatUntilStageEnd"],
                    )
                    for occurrence in floor_stages["SECOND_SMASH"]["animation"]["occurrences"]
                ],
            ),
        )
        arena_break = patterns["VALTAN_ARENA_BREAK_109"]
        arena_gameplay = next(
            pattern
            for pattern in self.gameplay["patterns"]
            if pattern["patternId"] == "VALTAN_ARENA_BREAK_109"
        )
        self.assertEqual(
            {
                "kind": "LEAP_TO_ANCHOR",
                "anchorId": "anchor.valtan.arena-break-109.landing",
                "landingPosition": [156.03, 22.99751, -122.06],
                "apexHeight": 12.0,
                "travelStageId": "DROP",
                "takeoffStartMs": 0,
                "takeoffEndMs": 900,
                "travelStartMs": 0,
                "travelEndMs": 700,
            },
            arena_gameplay["serverMotion"],
        )
        self.assertEqual(
            [
                ("TAKEOFF", "valtan.mechanic.arena-break-109.takeoff", 900),
                ("DROP", "valtan.mechanic.arena-break-109.drop", 700),
                ("IMPACT", "valtan.mechanic.arena-break-109.impact", 400),
                (
                    "IMPACT_HOLD",
                    "valtan.mechanic.arena-break-109.impact-hold",
                    1100,
                ),
                (
                    "WIDE_REVEAL",
                    "valtan.mechanic.arena-break-109.wide-reveal",
                    2300,
                ),
                ("RECOVERY", "valtan.mechanic.arena-break-109.recovery", 870),
            ],
            [
                (stage["stageId"], stage["actionId"], stage["durationMs"])
                for stage in arena_gameplay["stages"]
            ],
        )
        self.assertEqual(
            [
                "mesh_att_battle_12_01",
                "mesh_att_battle_12_01",
                "mesh_att_battle_12_02",
                "mesh_att_battle_12_02",
                "mesh_att_battle_12_02",
                "mesh_att_battle_12_03",
                "mesh_att_battle_12_03",
                "mesh_evt1_att_battle_5_01_start",
                "mesh_evt1_att_battle_5_01_loop",
                "mesh_evt1_att_battle_5_01_end",
            ],
            [
                occurrence["clip"]
                for stage in arena_break["stages"]
                for occurrence in stage["animation"]["occurrences"]
            ],
        )
        wide_reveal = next(
            stage
            for stage in arena_break["stages"]
            if stage["stageId"] == "WIDE_REVEAL"
        )
        self.assertEqual("ROAR_SEQUENCE", wide_reveal["sequenceRole"])
        self.assertEqual(
            [
                "mesh_att_battle_12_03",
                "mesh_evt1_att_battle_5_01_start",
                "mesh_evt1_att_battle_5_01_loop",
            ],
            [
                occurrence["clip"]
                for occurrence in wide_reveal["animation"]["occurrences"]
            ],
        )

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
