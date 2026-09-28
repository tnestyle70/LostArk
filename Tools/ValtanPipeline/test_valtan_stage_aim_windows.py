"""Stage aim windows retain legacy omission and freeze on an exclusive deadline."""
import copy
import unittest

from Tools.ValtanPipeline import valtan_tuning_pipeline as pipeline


class StageAimWindowTests(unittest.TestCase):
    def validate(self, aim, duration=1000, stage_kind="ACTIVE"):
        stage = dict(stageKind=stage_kind, durationMs=duration, branches=[])
        if aim is not ...:
            stage["aim"] = aim
        before = copy.deepcopy(stage)
        pipeline._validate_pattern_stage_extensions(stage, "test-stage")
        self.assertEqual(before, stage)

    def test_legacy_and_optional_end_response_combinations(self):
        self.validate(...)
        for policy in ("NEAREST_EACH_TICK", "PATTERN_TARGET"):
            for extras in ({}, {"endMs": 0}, {"endMs": 1000}, {"responseScale": 1 / 3},
                           {"endMs": 133, "responseScale": 1 / 3}):
                with self.subTest(policy=policy, extras=extras):
                    self.validate(dict(targetPolicy=policy, **extras))

    def test_invalid_aim_deadlines_are_rejected(self):
        for value in (-1, 1001, 0.5, True, None, "100"):
            with self.subTest(value=value), self.assertRaises(pipeline.PipelineError):
                self.validate(dict(targetPolicy="PATTERN_TARGET", endMs=value))

    def test_invalid_response_and_policy_are_rejected(self):
        for value in (0, -1, 10.01, float("inf"), float("nan"), True, None, "0.3"):
            with self.subTest(value=value), self.assertRaises(pipeline.PipelineError):
                self.validate(dict(targetPolicy="PATTERN_TARGET", responseScale=value))
        for aim in (None, [], "PATTERN_TARGET", {"targetPolicy": "NONE"},
                    {"targetPolicy": "PATTERN_TARGET", "untilMs": 500}):
            with self.subTest(aim=aim), self.assertRaises(pipeline.PipelineError):
                self.validate(aim)
        with self.assertRaises(pipeline.PipelineError):
            self.validate(dict(targetPolicy="PATTERN_TARGET"), stage_kind="WAIT")


if __name__ == "__main__":
    unittest.main()
