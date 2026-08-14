from __future__ import annotations

import copy
import importlib.util
from pathlib import Path
import unittest


MODULE_PATH = Path(__file__).with_name("build_effect_derived_artifact.py")
SPEC = importlib.util.spec_from_file_location("effect_derived_artifact", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class OccurrenceTuningTests(unittest.TestCase):
    def setUp(self) -> None:
        self.program = {
            "target": {"runtimeCatalogAssetId": "effect.test.skill.1"},
            "emitters": [
                {"emitterId": "system::cue::emitter-a", "rowSha256": "a" * 64},
                {"emitterId": "system::cue::emitter-b", "rowSha256": "b" * 64},
            ],
        }
        self.tuning = {
            "schema": "lostark.effect-occurrence-tuning",
            "formatVersion": 1,
            "effectAssetId": "effect.test.skill.1",
            "entries": [
                {
                    "occurrenceId": "system::cue::emitter-a",
                    "sourceOccurrenceRowSha256": "a" * 64,
                    "provenance": "PROJECT_TUNED",
                    "effectiveLocalTransform": {
                        "position": [1.0, 2.0, 3.0],
                        "rotationDegrees": [0.0, 90.0, 0.0],
                        "scale": [1.0, 1.0, 1.0],
                    },
                }
            ],
        }

    def test_good_tuning_is_admitted(self) -> None:
        MODULE.validate_occurrence_tuning(self.tuning, self.program)

    def test_duplicate_occurrence_is_rejected(self) -> None:
        tuning = copy.deepcopy(self.tuning)
        tuning["entries"].append(copy.deepcopy(tuning["entries"][0]))
        with self.assertRaisesRegex(MODULE.ContractError, "unique and sorted"):
            MODULE.validate_occurrence_tuning(tuning, self.program)

    def test_stale_source_row_is_rejected(self) -> None:
        tuning = copy.deepcopy(self.tuning)
        tuning["entries"][0]["sourceOccurrenceRowSha256"] = "c" * 64
        with self.assertRaisesRegex(MODULE.ContractError, "stale"):
            MODULE.validate_occurrence_tuning(tuning, self.program)

    def test_unknown_occurrence_is_rejected(self) -> None:
        tuning = copy.deepcopy(self.tuning)
        tuning["entries"][0]["occurrenceId"] = "system::cue::missing"
        with self.assertRaisesRegex(MODULE.ContractError, "unknown"):
            MODULE.validate_occurrence_tuning(tuning, self.program)

    def test_unsafe_source_path_is_rejected(self) -> None:
        with self.assertRaisesRegex(MODULE.ContractError, "unsafe|normalized"):
            MODULE._validate_occurrence_tuning_source_path(
                "Effects/AuthoredCorrections/../escape.occurrence-tuning.json"
            )


if __name__ == "__main__":
    unittest.main()
