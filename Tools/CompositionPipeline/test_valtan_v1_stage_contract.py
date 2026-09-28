#!/usr/bin/env python3
"""Effect V1 stage clocks and target anchors must match the canonical owner."""
from __future__ import annotations

import copy
import json
import tempfile
import unittest
from pathlib import Path

from Tools.CompositionPipeline import composition_pipeline as pipeline
from Tools.ValtanPipeline import valtan_tuning_pipeline as valtan


class ValtanV1StageContractTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.asset = "effect.valtan.contract"
        relative = f"Effects/Authored/{self.asset}.effect.json"
        catalog = self.root / valtan.EFFECT_CATALOG_REL
        catalog.parent.mkdir(parents=True)
        catalog.write_text(json.dumps({"formatVersion": 1, "effects": [{
            "effectAssetId": self.asset,
            "payloadKind": valtan.DIRECT_AUTHORED_EFFECT_KIND,
            "authoringPath": relative,
        }]}), encoding="utf-8")
        source = self.root / "Data" / relative
        source.parent.mkdir(parents=True)
        source.write_text(json.dumps({"schema": "lostark.effect-authoring",
            "version": 15, "effectAssetId": self.asset, "elements": []}), encoding="utf-8")
        self.stage = {"stageId": "TRACK", "actionId": "action.track", "durationMs": 1000,
                      "aim": {"targetPolicy": "NEAREST_EACH_TICK"}}
        self.pattern = {"patternId": "VALTAN_CONTRACT", "targetPolicy": "NONE",
            "aimPolicy": "LOCK_FACING_ON_START", "stages": [
                {"stageId": "CENTER", "actionId": "action.center", "durationMs": 1000,
                 "motion": {"kind": "TO_ARENA_CENTER"}}, self.stage]}
        self.bindings = {"bindings": [{"actionId": "action.track", "clips": [
            {"clipOccurrenceId": "clip.track", "sourceStartMs": 100,
             "playMs": 1000, "playRate": 1.0, "loop": False}]}]}
        self.row = {"bindingId": "cue.valtan.contract", "occurrenceId": "occ.contract",
            "patternId": "VALTAN_CONTRACT", "stageId": "TRACK", "actionId": "action.track",
            "effectAssetId": self.asset, "timingBasis": "STAGE_CLOCK", "stageOffsetMs": 100,
            "stageEndMs": 1500, "anchorSlotId": "root", "followPolicy": "follow",
            "stopPolicy": "cue_end", "repeatPolicy": "once", "playbackOffsetMs": 73,
            "localTransform": {"position": [0, 0, 0], "rotationDegrees": [0, 0, 0],
                               "scale": [1, 1, 1]}, "scalePolicy": {"kind": "OWNER_RELATIVE"}}

    def validate(self, row: dict | None = None) -> None:
        document = {"schema": "lostark.valtan-pattern-effect-cues", "formatVersion": 4,
                    "ownerArchetypeId": "BOSS_VALTAN", "cues": [row or self.row]}
        before = copy.deepcopy((document, self.pattern, self.bindings))
        try:
            pipeline._validate_v1_effect_owners(self.root, document,
                animation_bindings=self.bindings, joined={"patterns": [self.pattern]})
        finally:
            self.assertEqual(before, (document, self.pattern, self.bindings))

    def test_bounded_stage_tail_survives_joined_and_detached_projection(self) -> None:
        for end_ms in (101, 1500, 600000):
            self.row["stageEndMs"] = end_ms
            self.validate()
            stage = dict(self.stage, effectCues=[dict(self.row, cueId=self.row["bindingId"])])
            before = copy.deepcopy(stage)
            for normalized in (pipeline._normalized_v1_cues("VALTAN_CONTRACT", stage)[0],
                               pipeline._normalized_detached_v1_cue(self.row)):
                self.assertEqual(normalized["clock"]["basis"], "STAGE")
                self.assertEqual(normalized["endMs"], end_ms)
                self.assertEqual(normalized["stopPolicy"], "CUE_END")
                self.assertEqual(normalized["payload"]["sourceClock"],
                                 {"basis": "STAGE", "startMs": 100, "endMs": end_ms})
                self.assertEqual(normalized["payload"]["playbackOffsetMs"], 73)
                self.assertNotIn("stageEndMs", normalized["payload"])
                pipeline._validate_projected_cue_invariants(
                    [{"stages": [{"durationMs": 1000, "cues": [normalized]}]}], [])
            self.assertEqual(stage, before)

    def test_landing_snapshot_requires_exact_server_leap_and_snapshot_follow(self) -> None:
        self.row.update(anchorSlotId="pattern.landing.snapshot", followPolicy="snapshot")
        for kind in ("LEAP_TO_TARGET", "LEAP_TO_ANCHOR"):
            self.pattern["serverMotion"] = {"kind": kind}
            self.validate()
        for mutation in (
            {"anchorSlotId": "pattern.landing.unknown"},
            {"followPolicy": "follow"},
        ):
            with self.subTest(mutation=mutation), self.assertRaises(pipeline.CompositionError):
                self.validate(dict(self.row, **mutation))
        for motion in (None, {"kind": "FORWARD"}):
            self.pattern["serverMotion"] = motion
            with self.subTest(motion=motion), self.assertRaises(pipeline.CompositionError):
                self.validate()

    def test_natural_stage_clock_accepts_only_absent_or_null_end(self) -> None:
        self.row["stopPolicy"] = "natural"
        for present in (False, True):
            self.row.pop("stageEndMs", None)
            if present:
                self.row["stageEndMs"] = None
            self.validate()
            stage = dict(self.stage, effectCues=[dict(self.row, cueId=self.row["bindingId"])])
            for cue in (pipeline._normalized_v1_cues("VALTAN_CONTRACT", stage)[0],
                        pipeline._normalized_detached_v1_cue(self.row)):
                self.assertNotIn("endMs", cue)
                self.assertNotIn("endMs", cue["payload"]["sourceClock"])
        self.row["stageEndMs"] = 1500
        with self.assertRaisesRegex(pipeline.CompositionError, "natural stopPolicy"):
            self.validate()

    def test_stage_end_bounds_types_and_policy_pairing_remain_strict(self) -> None:
        for end in (None, -1, 0, 99, 100, 600001, True, 1500.0, "1500"):
            with self.subTest(end=end), self.assertRaises(pipeline.CompositionError):
                self.validate(dict(self.row, stageEndMs=end))
        for fields in ({"repeatPolicy": "each_loop"}, {"stageOffsetMs": 1000},
                       {"stageOffsetMs": -1}, {"stageOffsetMs": True},
                       {"sourceEndMs": 1500}, {"unownedField": 0}):
            with self.subTest(fields=fields), self.assertRaises(pipeline.CompositionError):
                self.validate(dict(self.row, **fields))

    def test_clip_clock_does_not_admit_stage_end(self) -> None:
        row = dict(self.row)
        for field in ("timingBasis", "stageOffsetMs", "stageEndMs"):
            row.pop(field)
        row.update(clipOccurrenceId="clip.track", sourceStartMs=100, sourceEndMs=1500)
        self.validate(row)
        with self.assertRaisesRegex(pipeline.CompositionError, "stageEndMs"):
            self.validate(dict(row, stageEndMs=1500))

    def test_nearest_target_follow_requires_center_at_or_before_owning_stage(self) -> None:
        self.row["anchorSlotId"] = "arena.center.target-follow"
        self.validate()
        self.pattern["stages"][0].pop("motion")
        with self.assertRaisesRegex(pipeline.CompositionError, "arena-center"):
            self.validate()
        self.stage["motion"] = {"kind": "TO_ARENA_CENTER"}
        self.validate()
        self.stage.pop("motion")
        self.pattern["stages"].append({"stageId": "LATER", "actionId": "action.later",
                                      "motion": {"kind": "TO_ARENA_CENTER"}})
        with self.assertRaisesRegex(pipeline.CompositionError, "arena-center"):
            self.validate()

    def test_nearest_target_follow_does_not_relax_aim_or_snapshot_contract(self) -> None:
        self.row["anchorSlotId"] = "arena.center.target-follow"
        for policy in (None, "LOCK_FACING_ON_START", "NEAREST_ON_START"):
            self.stage["aim"] = {} if policy is None else {"targetPolicy": policy}
            with self.subTest(policy=policy), self.assertRaisesRegex(
                    pipeline.CompositionError, "arena-center"):
                self.validate()
        self.stage["aim"] = {"targetPolicy": "NEAREST_EACH_TICK"}
        for fields in ({"followPolicy": "snapshot"},
                       {"anchorSlotId": "arena.center", "followPolicy": "snapshot"},
                       {"anchorSlotId": "arena.center.facing", "followPolicy": "snapshot"}):
            with self.subTest(fields=fields), self.assertRaisesRegex(
                    pipeline.CompositionError, "arena-center"):
                self.validate(dict(self.row, **fields))

    def test_legacy_locked_leap_target_follow_remains_admitted(self) -> None:
        self.row["anchorSlotId"] = "arena.center.target-follow"
        self.pattern["stages"][0].pop("motion")
        self.stage.pop("aim")
        self.pattern.update(targetPolicy="LOCK_RANDOM_ALIVE_ON_START",
            aimPolicy="TRACK_TARGET_EACH_TICK", serverMotion={"kind": "LEAP_TO_ANCHOR",
            "moveToAnchorBeforeTakeoff": True})
        self.validate()
        self.pattern["serverMotion"]["moveToAnchorBeforeTakeoff"] = False
        with self.assertRaisesRegex(pipeline.CompositionError, "arena-center"):
            self.validate()


if __name__ == "__main__":
    unittest.main()
