#!/usr/bin/env python3
"""Checked-in DimensionMaster presentation source-count contract."""

from __future__ import annotations

import json
import unittest
from collections import Counter
from pathlib import Path


BASE_SKILL_IDS = (
    2050010,
    2050100,
    2050120,
    2050160,
    2050180,
    2050210,
    2050220,
    2050230,
    2050240,
    2050500,
    2050520,
)


class DimensionMasterPresentationSourceContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.root = Path(__file__).resolve().parents[2]

    def read_json(self, path: Path) -> dict:
        return json.loads(path.read_text(encoding="utf-8-sig"))

    def test_base11_light_and_screen_post_counts_are_stable(self) -> None:
        kind_counts: Counter[str] = Counter()
        subtype_counts: Counter[str] = Counter()
        for skill_id in BASE_SKILL_IDS:
            document = self.read_json(
                self.root / "Data" / "Effects" / "Authored" /
                f"effect.dimensionmaster.skill.{skill_id}.effect.json"
            )
            kind_counts.update(
                str(element.get("kind") or "")
                for element in document.get("elements", [])
            )
            for element in document.get("elements", []):
                if element.get("kind") != "screenPost":
                    continue
                system_id = str(element.get("sourceNode") or "").split(
                    "|", 1
                )[0].casefold()
                if "rgbnoise" in system_id:
                    subtype_counts["RGB_NOISE"] += 1
                elif "par_j_zoomblur" in system_id:
                    subtype_counts["ZOOM_J"] += 1
                elif "par_c_zoomblur" in system_id:
                    subtype_counts["ZOOM_C"] += 1
                elif "filmnoise" in system_id:
                    subtype_counts["FILM"] += 1
                else:
                    self.fail(f"unknown screen-post source system: {system_id}")

        self.assertEqual(kind_counts["light"], 23)
        self.assertEqual(kind_counts["screenPost"], 38)
        self.assertEqual(subtype_counts, {
            "RGB_NOISE": 26,
            "ZOOM_J": 9,
            "ZOOM_C": 2,
            "FILM": 1,
        })

    def test_disabled_d_zoom_c_occurrence_is_not_materialized(self) -> None:
        recipe = self.read_json(
            self.root / "Data" / "Effects" / "Imported" /
            "DimensionMaster" / "Converted" /
            "skill.2050240.action-cue-recipe.json"
        )
        source_occurrences = [
            cue
            for cue in recipe.get("cues", [])
            if str(cue.get("sourceType") or "").casefold()
            == "playparticleeffect"
            and "par_c_zoomblur_02" in str(
                cue.get("typedPayload", {}).get("sourceParticleSystem") or ""
            ).casefold()
        ]
        self.assertEqual(
            Counter(bool(cue.get("executionEnabled")) for cue in source_occurrences),
            {True: 1, False: 1},
        )

        document = self.read_json(
            self.root / "Data" / "Effects" / "Authored" /
            "effect.dimensionmaster.skill.2050240.effect.json"
        )
        emitted = [
            element
            for element in document.get("elements", [])
            if element.get("kind") == "screenPost"
            and "par_c_zoomblur_02" in str(
                element.get("sourceNode") or ""
            ).casefold()
        ]
        self.assertEqual(len(emitted), 1)


if __name__ == "__main__":
    unittest.main()
