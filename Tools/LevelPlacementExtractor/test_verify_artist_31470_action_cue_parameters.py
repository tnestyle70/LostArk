#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import unittest
from pathlib import Path

from verify_artist_31470_action_cue_parameters import validate_recipe


REPO_ROOT = Path(__file__).resolve().parents[2]
RECIPE_PATH = (
    REPO_ROOT
    / "Data"
    / "Effects"
    / "Imported"
    / "Artist"
    / "skill.31470.action-cue-recipe.json"
)


class Artist31470ActionCueParameterTests(unittest.TestCase):
    def setUp(self) -> None:
        self.recipe = json.loads(RECIPE_PATH.read_text(encoding="utf-8-sig"))

    def test_current_recipe_preserves_all_source_occurrences_and_overrides(self) -> None:
        result = validate_recipe(self.recipe)
        self.assertEqual(result["verifiedLightOccurrenceCount"], 4)
        self.assertGreater(result["typedParticleParameterOverrideCount"], 25)

    def test_disabled_light_occurrence_cannot_be_promoted(self) -> None:
        changed = copy.deepcopy(self.recipe)
        cue = next(
            row for row in changed["cues"]
            if row["cueId"].endswith("notify-030")
        )
        cue["sourceOccurrence"]["enabled"] = True
        with self.assertRaisesRegex(ValueError, "enabled flag drift"):
            validate_recipe(changed)

    def test_explicit_size_scalar_cannot_be_overwritten_by_fallback(self) -> None:
        changed = copy.deepcopy(self.recipe)
        cue = next(
            row for row in changed["cues"]
            if row["cueId"].endswith("notify-029")
        )
        size = next(
            row for row in cue["typedPayload"]["parameterOverrides"]
            if row["name"] == "Size"
        )
        size["scalarValue"] = 100.0
        with self.assertRaisesRegex(ValueError, "expected 600.0"):
            validate_recipe(changed)


if __name__ == "__main__":
    unittest.main()
