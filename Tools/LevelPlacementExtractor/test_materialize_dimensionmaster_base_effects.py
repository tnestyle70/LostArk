#!/usr/bin/env python3

from __future__ import annotations

import unittest

from materialize_dimensionmaster_base_effects import build_combo_stage_document


class MaterializeDimensionMasterBaseEffectsTests(unittest.TestCase):
    def test_combo_stage_rebases_and_filters_element_clock(self) -> None:
        aggregate = {
            "effectAssetId": "effect.aggregate",
            "displayName": "aggregate",
            "modelCues": [{"cueId": "not-stage-owned"}],
            "elements": [
                {"id": "before", "detail": {"timing": {"startDelaySeconds": 3.9}}},
                {"id": "first", "detail": {"timing": {"startDelaySeconds": 4.1}}},
                {"id": "next", "detail": {"timing": {"startDelaySeconds": 5.5}}},
            ],
        }
        result = build_combo_stage_document(
            aggregate, "effect.ba2", "BA2", 4.0, 5.5
        )
        self.assertEqual("effect.ba2", result["effectAssetId"])
        self.assertEqual(["first"], [row["id"] for row in result["elements"]])
        self.assertAlmostEqual(
            0.1, result["elements"][0]["detail"]["timing"]["startDelaySeconds"]
        )
        self.assertEqual([], result["modelCues"])


if __name__ == "__main__":
    unittest.main()
