#!/usr/bin/env python3
"""Unit tests for the Artist F original-DXBC replay contract."""

from __future__ import annotations

import copy
import unittest

from replay_artist_31470_main_original_dxbc import (
    FLOAT_TOLERANCE,
    base_fixtures,
    replay_cases,
    sprite_opacity_oracle,
    water_opacity_oracle,
)


class OpacityOracleTests(unittest.TestCase):
    def test_all_fixed_fixtures_match_the_reviewed_closed_forms(self) -> None:
        for fixture in replay_cases():
            oracle = (
                water_opacity_oracle(fixture)
                if fixture["family"] == "#9/#10_WATERTRAIL"
                else sprite_opacity_oracle(fixture)
            )
            self.assertLessEqual(
                abs(oracle - fixture["expectedAlpha"]),
                FLOAT_TOLERANCE,
                fixture["caseId"],
            )

    def test_water_uses_noise_r_and_rejects_gba_as_opacity_inputs(self) -> None:
        carrier, water, textures, _, _ = base_fixtures()
        fixture = {
            "cb0": water,
            "carrier": carrier,
            "textures": textures,
        }
        fixture["cb0"][15][0] = 0.1
        baseline = water_opacity_oracle(fixture)
        gba = copy.deepcopy(fixture)
        gba["textures"][0]["pixels"][0][1:] = [0.91, 0.73, 0.19]
        self.assertEqual(baseline, water_opacity_oracle(gba))
        red = copy.deepcopy(fixture)
        red["textures"][0]["pixels"][0][0] = 0.0
        self.assertNotEqual(baseline, water_opacity_oracle(red))

    def test_sprite_rgb_exponent_is_not_in_the_opacity_slice(self) -> None:
        carrier, _, _, sprite, textures = base_fixtures()
        fixture = {
            "cb0": sprite,
            "carrier": carrier,
            "textures": textures,
        }
        baseline = sprite_opacity_oracle(fixture)
        fixture["cb0"][22][3] = 7.0
        self.assertEqual(baseline, sprite_opacity_oracle(fixture))


if __name__ == "__main__":
    unittest.main()
