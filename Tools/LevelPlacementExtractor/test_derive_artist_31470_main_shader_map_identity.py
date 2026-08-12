#!/usr/bin/env python3

from __future__ import annotations

import copy
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace

from derive_artist_31470_main_shader_map_identity import (
    canonical_json_sha256,
    engine_equivalent_static_parameter_set,
    normalized_static_parameter_set,
)
from extract_ue3_placements import (
    CompressedChunk,
    ExtractionError,
    LostArkPackageRangeReader,
)


def static_set_fixture() -> dict:
    return {
        "baseMaterialIdHex": "00112233445566778899aabbccddeeff",
        "staticSwitchParameters": [
            {
                "parameterName": "Use_Dissolve",
                "entryOffset": 128,
                "value": True,
                "bOverride": False,
                "expressionGuidHex": "ffeeddccbbaa99887766554433221100",
            },
            {
                "parameterName": "Use_Mesh",
                "entryOffset": 160,
                "value": True,
                "bOverride": True,
                "expressionGuidHex": "0123456789abcdeffedcba9876543210",
            },
        ],
        "staticComponentMaskParameters": [],
        "normalParameters": [],
        "terrainLayerWeightParameters": [],
        "offset": 108,
        "byteSize": 96,
        "rawSha256": "not-a-cross-package-key",
        "semanticSha256": "legacy-includes-entry-offset",
        "endOffset": 204,
    }


class NormalizedStaticParameterSetTests(unittest.TestCase):
    def test_offsets_case_and_nonsemantic_hashes_do_not_change_key(self) -> None:
        left = static_set_fixture()
        right = copy.deepcopy(left)
        right["offset"] = 4000
        right["endOffset"] = 4096
        right["rawSha256"] = "different-package-local-bytes"
        right["semanticSha256"] = "different-legacy-digest"
        right["staticSwitchParameters"][0]["entryOffset"] = 4020
        right["staticSwitchParameters"][1]["entryOffset"] = 4052
        right["staticSwitchParameters"][0]["parameterName"] = "use_dissolve"
        right["staticSwitchParameters"][1]["parameterName"] = "USE_MESH"

        left_key = normalized_static_parameter_set(left)
        right_key = normalized_static_parameter_set(right)
        self.assertEqual(left_key, right_key)
        self.assertEqual(canonical_json_sha256(left_key), canonical_json_sha256(right_key))
        self.assertEqual(
            left_key["staticSwitchParameters"][0]["parameterNameNumber"], 0
        )

    def test_array_order_value_and_guid_remain_identity_fields(self) -> None:
        baseline = normalized_static_parameter_set(static_set_fixture())
        reordered = static_set_fixture()
        reordered["staticSwitchParameters"].reverse()
        changed_value = static_set_fixture()
        changed_value["staticSwitchParameters"][0]["value"] = False
        changed_guid = static_set_fixture()
        changed_guid["staticSwitchParameters"][0]["expressionGuidHex"] = "00" * 16

        self.assertNotEqual(baseline, normalized_static_parameter_set(reordered))
        self.assertNotEqual(baseline, normalized_static_parameter_set(changed_value))
        self.assertNotEqual(baseline, normalized_static_parameter_set(changed_guid))

    def test_engine_equality_ignores_order_override_and_legacy_normal(self) -> None:
        baseline = static_set_fixture()
        baseline["normalParameters"] = [
            {
                "parameterName": "LegacyNormal",
                "entryOffset": 300,
                "valueOrdinalCandidate": 7,
                "overrideOrdinalCandidate": 1,
                "expressionGuidHex": "11" * 16,
            }
        ]
        equivalent = copy.deepcopy(baseline)
        equivalent["staticSwitchParameters"].reverse()
        for row in equivalent["staticSwitchParameters"]:
            row["bOverride"] = not row["bOverride"]
        equivalent["normalParameters"][0]["valueOrdinalCandidate"] = 999

        self.assertEqual(
            engine_equivalent_static_parameter_set(baseline),
            engine_equivalent_static_parameter_set(equivalent),
        )
        self.assertNotEqual(
            normalized_static_parameter_set(baseline),
            normalized_static_parameter_set(equivalent),
        )

    def test_engine_equality_keeps_switch_mask_and_terrain_values(self) -> None:
        baseline = static_set_fixture()
        baseline["staticComponentMaskParameters"] = [
            {
                "parameterName": "Mask",
                "r": True,
                "g": False,
                "b": True,
                "a": False,
                "bOverride": True,
                "expressionGuidHex": "22" * 16,
            }
        ]
        baseline["terrainLayerWeightParameters"] = [
            {
                "parameterName": "Terrain",
                "valueOrdinalCandidate": 3,
                "overrideOrdinalCandidate": 0,
                "expressionGuidHex": "33" * 16,
            }
        ]
        changed_switch = copy.deepcopy(baseline)
        changed_switch["staticSwitchParameters"][0]["value"] = False
        changed_mask = copy.deepcopy(baseline)
        changed_mask["staticComponentMaskParameters"][0]["a"] = True
        changed_terrain = copy.deepcopy(baseline)
        changed_terrain["terrainLayerWeightParameters"][0]["valueOrdinalCandidate"] = 4

        key = engine_equivalent_static_parameter_set(baseline)
        self.assertNotEqual(key, engine_equivalent_static_parameter_set(changed_switch))
        self.assertNotEqual(key, engine_equivalent_static_parameter_set(changed_mask))
        self.assertNotEqual(key, engine_equivalent_static_parameter_set(changed_terrain))


class LogicalPackageRangeReaderTests(unittest.TestCase):
    def test_uncompressed_ranges_cross_direct_and_chunk_boundaries(self) -> None:
        direct = b"abcdefghijklmnop"
        first = b"qrstuvwxyzABCDEF"
        second = b"GHIJKLMNOPQRSTUV"
        physical = direct + (b"\x00" * 16) + first + second
        chunks = (
            CompressedChunk(16, 16, 32, 16, 0),
            CompressedChunk(32, 16, 48, 16, 0),
        )
        summary = SimpleNamespace(chunks=chunks)
        expected = direct + first + second
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "fixture.upk"
            path.write_bytes(physical)
            reader = LostArkPackageRangeReader(path, summary=summary)
            self.assertEqual(reader.read_logical_range(0, len(expected)), expected)
            self.assertEqual(reader.read_logical_range(12, 24), expected[12:36])

    def test_uncovered_logical_gap_is_fail_closed(self) -> None:
        physical = (b"A" * 16) + (b"\x00" * 16) + (b"B" * 16) + (b"C" * 16)
        summary = SimpleNamespace(
            chunks=(
                CompressedChunk(16, 16, 32, 16, 0),
                CompressedChunk(48, 16, 48, 16, 0),
            )
        )
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "gap.upk"
            path.write_bytes(physical)
            reader = LostArkPackageRangeReader(path, summary=summary)
            with self.assertRaisesRegex(ExtractionError, "uncovered byte"):
                reader.read_logical_range(0, 64)


if __name__ == "__main__":
    unittest.main()
