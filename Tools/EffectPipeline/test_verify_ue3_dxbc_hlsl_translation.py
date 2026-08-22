#!/usr/bin/env python3
"""Focused tests for numeric comparison in the DXBC/HLSL WARP gate."""

from __future__ import annotations

import math
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from verify_ue3_dxbc_hlsl_translation import compare  # noqa: E402


def pixel(value: float) -> list[list[float]]:
    return [[value, 0.0, 0.0, 0.0]]


class NonFiniteComparisonTest(unittest.TestCase):
    def test_original_nan_and_translated_finite_is_a_mismatch(self) -> None:
        result = compare(pixel(math.nan), pixel(1.0))
        self.assertEqual(len(result["mismatches"]), 1)
        self.assertEqual(result["mismatches"][0]["reason"],
                         "NON_FINITE_MISMATCH")
        self.assertEqual(result["mismatches"][0]["original"], "NaN")

    def test_original_finite_and_translated_nan_is_a_mismatch(self) -> None:
        result = compare(pixel(1.0), pixel(math.nan))
        self.assertEqual(len(result["mismatches"]), 1)
        self.assertEqual(result["mismatches"][0]["translated"], "NaN")

    def test_opposite_infinities_are_a_mismatch(self) -> None:
        result = compare(pixel(math.inf), pixel(-math.inf))
        self.assertEqual(len(result["mismatches"]), 1)
        self.assertEqual(result["mismatches"][0]["original"], "+Infinity")
        self.assertEqual(result["mismatches"][0]["translated"], "-Infinity")

    def test_matching_non_finite_values_are_accepted(self) -> None:
        self.assertEqual(compare(pixel(math.nan), pixel(math.nan))["mismatches"], [])
        self.assertEqual(compare(pixel(math.inf), pixel(math.inf))["mismatches"], [])
        self.assertEqual(compare(pixel(-math.inf), pixel(-math.inf))["mismatches"], [])


if __name__ == "__main__":
    unittest.main()
