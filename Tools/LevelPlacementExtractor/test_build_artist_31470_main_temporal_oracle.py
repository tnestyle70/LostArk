#!/usr/bin/env python3

from __future__ import annotations

import copy
import importlib.util
import unittest
from pathlib import Path


HERE = Path(__file__).resolve().parent
MODULE_PATH = HERE / "build_artist_31470_main_temporal_oracle.py"
SPEC = importlib.util.spec_from_file_location("artist_31470_temporal_oracle", MODULE_PATH)
assert SPEC and SPEC.loader
oracle = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(oracle)


def reseal(receipt: dict) -> None:
    for row in receipt["occurrences"]:
        for sample in row["samples"]:
            lookup = sample["sourceLookup"]
            lookup_payload = copy.deepcopy(lookup)
            lookup_payload.pop("lookupSha256", None)
            lookup["lookupSha256"] = oracle.digest(lookup_payload)
            sample_payload = copy.deepcopy(sample)
            sample_payload.pop("sampleSha256", None)
            sample["sampleSha256"] = oracle.digest(sample_payload)
        row_payload = copy.deepcopy(row)
        row_payload.pop("rowSha256", None)
        row["rowSha256"] = oracle.digest(row_payload)
    receipt.pop("receiptSha256", None)
    receipt["receiptSha256"] = oracle.digest(receipt)


class TemporalOracleTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.tracked = oracle.read_json(oracle.OUTPUT)

    def assert_deep_rejected(self, mutation) -> None:
        value = copy.deepcopy(self.tracked)
        mutation(value)
        reseal(value)
        with self.assertRaises(oracle.OracleError):
            oracle.validate_against_inputs(value)

    def test_tracked_receipt_is_current_and_valid(self) -> None:
        oracle.validate_against_inputs(self.tracked)

    def test_generator_is_deterministic(self) -> None:
        self.assertEqual(oracle.canonical_bytes(self.tracked), oracle.canonical_bytes(oracle.build_receipt()))

    def test_unsealed_byte_mutation_is_rejected(self) -> None:
        value = copy.deepcopy(self.tracked)
        value["summary"]["sampleCount"] = 14
        with self.assertRaises(oracle.OracleError):
            oracle.validate_receipt(value)

    def test_seed_mutation_is_rejected_after_reseal(self) -> None:
        self.assert_deep_rejected(lambda value: value["occurrences"][0]["rng"].__setitem__("emitterSeed", 7))

    def test_emitter_order_mutation_is_rejected_after_reseal(self) -> None:
        self.assert_deep_rejected(lambda value: value["occurrences"][1].__setitem__("order", 99))

    def test_lifetime_mutation_is_rejected_after_reseal(self) -> None:
        self.assert_deep_rejected(lambda value: value["occurrences"][2]["lifetime"].__setitem__("seconds", 0.81))

    def test_lookup_output_mutation_is_rejected_against_inputs(self) -> None:
        self.assert_deep_rejected(lambda value: value["occurrences"][2]["samples"][2]["sourceLookup"]["dynamicParameter"]["value"].__setitem__(0, 123.0))

    def test_material_clock_origin_mutation_is_rejected(self) -> None:
        self.assert_deep_rejected(lambda value: value["occurrences"][0]["samples"][1]["materialCurrentTime"].__setitem__("projectedLocalSeconds", 0.0))

    def test_straight_seek_divergence_is_rejected(self) -> None:
        self.assert_deep_rejected(lambda value: value["occurrences"][0]["samples"][1]["fixed60Hz"].__setitem__("directSeekReplaySha256", "0" * 64))

    def test_terminal_tail_leak_is_rejected(self) -> None:
        self.assert_deep_rejected(lambda value: value["occurrences"][0]["samples"][4]["sourceLifetimeBoundary"].__setitem__("renderEligible", True))

    def test_default_provenance_promotion_is_rejected(self) -> None:
        self.assert_deep_rejected(lambda value: value["occurrences"][0]["schedule"].__setitem__("sourceExact", True))

    def test_product_admission_promotion_is_rejected(self) -> None:
        self.assert_deep_rejected(lambda value: value["admission"].__setitem__("productAdmission", True))


if __name__ == "__main__":
    unittest.main()
