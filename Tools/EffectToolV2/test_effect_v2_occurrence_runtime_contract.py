"""Focused contract for Effect V2 Stage/clip-occurrence runtime clocks."""

from __future__ import annotations

import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


class EffectV2OccurrenceRuntimeContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.runtime_h = (ROOT / "Client/Public/EffectV2_Runtime.h").read_text(
            encoding="utf-8"
        )
        cls.runtime_cpp = (ROOT / "Client/Private/EffectV2_Runtime.cpp").read_text(
            encoding="utf-8"
        )
        cls.valtan_cpp = (ROOT / "Client/Private/Valtan.cpp").read_text(
            encoding="utf-8"
        )

    def test_runtime_consumes_a_typed_occurrence_wall_map(self) -> None:
        for token in (
            "EFFECT_V2_CLIP_OCCURRENCE_CLOCK",
            "fStageWallStartSeconds",
            "fSourceStartSeconds",
            "fSourceDurationSeconds",
            "fLoopWallDurationSeconds",
            "fPlaybackRate",
            "bLoop",
        ):
            self.assertIn(token, self.runtime_h)
        self.assertIn("Find_OccurrenceClock", self.runtime_cpp)
        self.assertIn("Resolve_StageSpawnClock", self.runtime_cpp)
        self.assertIn("Pending.Binding.strClipOccurrenceId", self.runtime_cpp)

    def test_each_loop_is_epoch_driven_and_bounded(self) -> None:
        for token in (
            "iNextLoopEpoch",
            "EFFECT_V2_REPEAT_POLICY::EACH_LOOP",
            "MAX_EPOCHS_PER_SYNC = 256u",
            "Resolve_LastDueLoopEpoch",
            "iLastDueEpoch",
        ):
            self.assertIn(token, self.runtime_cpp)
        self.assertNotIn("bool_t bSpawned", self.runtime_cpp)
        self.assertIn("EACH_LOOP requires a looping clip occurrence", self.runtime_cpp)

    def test_occurrence_stop_and_late_snapshot_are_explicit(self) -> None:
        self.assertIn(
            "EFFECT_V2_STOP_POLICY::CLIP_OCCURRENCE_END", self.runtime_cpp
        )
        self.assertIn("fOccurrenceEnd", self.runtime_cpp)
        self.assertIn("fAgeSeconds - Clock.fStartSeconds", self.runtime_cpp)
        self.assertIn("Reset_StageLane(State)", self.runtime_cpp)

    def test_valtan_builds_clocks_from_the_same_animation_chain(self) -> None:
        for token in (
            "Build_EffectV2OccurrenceClocks(",
            "Build_PatternTimeline(pModel, Clips, Timings)",
            "Clips[iClip].strClipOccurrenceId",
            "Build_EffectV2OccurrenceClocksForAction(",
            "m_LocalPreviewClipByActionId : m_PatternClipByActionId",
            "m_PatternClipByActionId,",
            "EffectV2Clocks,",
        ):
            self.assertIn(token, self.valtan_cpp)

    def test_every_each_loop_binding_joins_one_looping_occurrence(self) -> None:
        bindings = json.loads(
            (
                ROOT
                / "Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json"
            ).read_text(encoding="utf-8")
        )["bindings"]
        animation = json.loads(
            (
                ROOT
                / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
            ).read_text(encoding="utf-8")
        )["bindings"]
        by_action = {row["actionId"]: row for row in animation}
        each_loop = [
            row
            for row in bindings
            if row["clock"]["repeatPolicy"] == "EACH_LOOP"
        ]
        self.assertGreater(len(each_loop), 0)
        for row in each_loop:
            action = by_action[row["scope"]["actionId"]]
            matches = [
                clip
                for clip in action["clips"]
                if clip["clipOccurrenceId"] == row["clock"]["clipOccurrenceId"]
            ]
            self.assertEqual(1, len(matches), row["bindingId"])
            self.assertTrue(matches[0]["loop"], row["bindingId"])


if __name__ == "__main__":
    unittest.main()
