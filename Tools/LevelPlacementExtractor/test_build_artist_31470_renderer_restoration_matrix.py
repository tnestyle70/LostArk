#!/usr/bin/env python3
"""Focused tests for the Artist F renderer-restoration coverage matrix.

The synthetic tests deliberately avoid opening either full RefShaderCache.  The
tracked-receipt tests exercise the shallow, self-contained validation boundary
and then re-seal mutations so a plain digest mismatch cannot satisfy them.
"""

from __future__ import annotations

import copy
import hashlib
import json
import math
import struct
import tempfile
import unittest
from pathlib import Path
from typing import Any, Callable

import build_artist_31470_renderer_restoration_matrix as matrix


def fake_dxbc(payload: bytes = b"\x00\x00\x00\x00") -> bytes:
    total_size = 36 + 8 + len(payload)
    return (
        b"DXBC"
        + (b"\x00" * 16)
        + struct.pack("<III", 1, total_size, 1)
        + struct.pack("<I", 36)
        + b"SHDR"
        + struct.pack("<I", len(payload))
        + payload
    )


def literal_lz4(payload: bytes) -> bytes:
    output = bytearray([min(len(payload), 15) << 4])
    remainder = len(payload) - 15
    if remainder >= 0:
        while remainder >= 255:
            output.append(255)
            remainder -= 255
        output.append(remainder)
    output.extend(payload)
    return bytes(output)


class MemoryReader:
    def __init__(self, payload: bytes) -> None:
        self.payload = payload

    def read_logical_range(self, offset: int, size: int) -> bytes:
        result = self.payload[offset : offset + size]
        if len(result) != size:
            raise ValueError("synthetic logical range is truncated")
        return result


def resolve_payload(payload: bytes) -> dict[str, Any]:
    compressed = literal_lz4(payload)
    return matrix.resolve_code_candidate(
        {"reader": MemoryReader(compressed)},
        {
            "codePosition": {
                "codeIndex": 7,
                "compressedLogicalOffset": 0,
                "compressedByteSize": len(compressed),
                "uncompressedByteSize": len(payload),
            },
            "codeIndexCandidate": 7,
            "opaqueDescriptorTailU32": [7, 0, 0],
        },
    )


def recipe_fixture(
    recipe_id: str,
    *,
    static: bool,
    material_class: str = "materialinstanceconstant",
) -> dict[str, Any]:
    static_set = None
    status = "MIC_NATIVE_STATIC_KEY_ABSENT"
    if static:
        static_set = {
            "baseMaterialIdHex": "11" * 16,
            "engineEqualitySha256": "22" * 32,
        }
        status = "STATIC_PARAMETER_SET_AVAILABLE"
    elif material_class == "material":
        status = "DIRECT_MATERIAL_HAS_NO_STATIC_PARAMETER_SET"
    return {
        "recipeId": recipe_id,
        "sourceMaterialPath": f"fixture.{recipe_id}",
        "rendererShapes": ["SpriteParticle"],
        "arithmeticFamilyId": "fixture-family",
        "materialIdentity": {
            "materialClass": material_class,
            "logicalPackage": "fixture",
            "physicalPackage": "fixture.upk",
            "physicalPackageSha256": "33" * 32,
            "materialObjectPath": f"fixture.{recipe_id}",
            "materialSerialSha256": "44" * 32,
        },
        "occurrenceIds": [],
        "sourceInputCounts": {
            "scalarOverrides": 0,
            "vectorOverrides": 0,
            "textureOverrides": 0,
            "parentDefaults": 0,
        },
        "renderStateEvidence": {
            "partialCullExact": False,
            "fullCullModeExact": False,
            "fullRenderStateExact": False,
        },
        "staticParameterSet": static_set,
        "staticLookupStatus": status,
    }


def missing_map_row(recipe_id: str, status: str = "MAP_CONTEXT_MISSING") -> dict[str, Any]:
    return {
        "recipeId": recipe_id,
        "search": {
            "exactMapContextCount": 0,
            "mapContextStatus": status,
        },
        "parseStatus": status,
        "parseError": None,
        "materialMapCandidate": None,
    }


def cohort(cohort_id: str, *rows: dict[str, Any]) -> dict[str, Any]:
    return {"cohortId": cohort_id, "maps": list(rows)}


def reseal_row(row: dict[str, Any]) -> None:
    row.pop("rowSha256", None)
    matrix.seal_row(row)


def reseal_receipt(receipt: dict[str, Any]) -> None:
    receipt.pop("receiptSha256", None)
    matrix.seal_receipt(receipt)


def different_hex(value: str) -> str:
    replacement = "0" * len(value)
    return "f" * len(value) if replacement == value else replacement


class StrictJsonAndDxbcTests(unittest.TestCase):
    def test_strict_json_rejects_duplicate_keys_and_nonfinite_constants(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "fixture.json"
            path.write_text('{"schema":"a","schema":"b"}', encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "duplicate JSON key"):
                matrix.read_json_strict(path)

            for token in ("NaN", "Infinity", "-Infinity"):
                path.write_text(f'{{"value":{token}}}', encoding="utf-8")
                with self.subTest(token=token), self.assertRaisesRegex(
                    ValueError, "non-finite JSON constant"
                ):
                    matrix.read_json_strict(path)

        with self.assertRaises(ValueError):
            matrix.canonical_bytes({"value": math.nan})

    def test_single_and_bundled_dxbc_are_distinguished_without_real_cache(self) -> None:
        first = fake_dxbc(b"\x01\x02\x03\x04")
        second = fake_dxbc(b"\x05\x06\x07\x08")

        single = resolve_payload(b"prefix" + first + b"suffix")
        self.assertEqual(single["status"], "CANDIDATE_SINGLE_DXBC")
        self.assertEqual(len(single["dxbcContainers"]), 1)
        self.assertEqual(
            single["dxbcContainers"][0]["dxbcSha256"],
            hashlib.sha256(first).hexdigest(),
        )

        bundled = resolve_payload(first + b"opaque-separator" + second)
        self.assertEqual(bundled["status"], "CANDIDATE_DXBC_BUNDLE")
        self.assertEqual(len(bundled["dxbcContainers"]), 2)
        self.assertEqual(
            [row["offsetInDecompressedRecord"] for row in bundled["dxbcContainers"]],
            [0, len(first) + len(b"opaque-separator")],
        )

    def test_unresolved_descriptor_is_not_promoted_to_embedded_code(self) -> None:
        candidate = matrix.resolve_code_candidate(
            {"reader": MemoryReader(b"")},
            {
                "codePosition": None,
                "codeIndexCandidate": 31,
                "opaqueDescriptorTailU32": [31, 2, 1],
            },
        )
        self.assertEqual(candidate["status"], "UNRESOLVED_CODE_MAPPING")
        self.assertIsNone(candidate["candidatePosition"])
        self.assertEqual(candidate["dxbcContainers"], [])


class SyntheticCoverageTests(unittest.TestCase):
    def test_missing_same_distribution_map_becomes_explicit_cache_block(self) -> None:
        recipe = recipe_fixture("recipe-static", static=True)
        official = cohort(
            "OFFICIAL_SAME_DISTRIBUTION_V974",
            missing_map_row(recipe["recipeId"]),
        )
        installed = cohort(
            "CURRENT_INSTALLED_CACHE",
            missing_map_row(recipe["recipeId"]),
        )

        row = matrix.combine_recipe_coverage([recipe], official, installed)[0]

        self.assertEqual(row["disposition"], "CACHE_MISSING_BLOCK")
        self.assertEqual(row["cacheJoins"], [])
        self.assertEqual(
            row["registerClosureStatus"],
            "UNAVAILABLE_WITHOUT_OFFICIAL_SHADER_MAP",
        )
        self.assertIn("OFFICIAL_MAP_CONTEXT_MISSING", row["blockers"])
        self.assertFalse(row["selectedVfPassAdmission"])
        self.assertFalse(row["productAdmission"])

    def test_direct_material_and_mic_without_key_stay_reconstructed_only(self) -> None:
        recipes = [
            recipe_fixture(
                "recipe-direct-material", static=False, material_class="material"
            ),
            recipe_fixture(
                "recipe-mic-no-key",
                static=False,
                material_class="materialinstanceconstant",
            ),
        ]
        official = cohort("OFFICIAL_SAME_DISTRIBUTION_V974")
        installed = cohort("CURRENT_INSTALLED_CACHE")

        rows = matrix.combine_recipe_coverage(recipes, official, installed)

        self.assertEqual(
            [row["disposition"] for row in rows],
            ["RECONSTRUCTED_ONLY", "RECONSTRUCTED_ONLY"],
        )
        self.assertEqual([row["cacheJoins"] for row in rows], [[], []])
        self.assertEqual(
            {row["staticLookupStatus"] for row in rows},
            {
                "DIRECT_MATERIAL_HAS_NO_STATIC_PARAMETER_SET",
                "MIC_NATIVE_STATIC_KEY_ABSENT",
            },
        )
        for row in rows:
            self.assertIn(row["staticLookupStatus"], row["blockers"])
            self.assertEqual(
                row["registerClosureStatus"],
                "UNAVAILABLE_WITHOUT_AUTHENTICATED_SHADER_MAP",
            )


class TrackedReceiptMutationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.receipt = matrix.read_json_strict(matrix.DEFAULT_OUTPUT)
        # Every mutation test is meaningful only after the checked-in receipt
        # passes the same self-contained shallow validator used by ProjectAudit.
        matrix.validate_receipt(cls.receipt)

    def fresh_receipt(self) -> dict[str, Any]:
        return copy.deepcopy(self.receipt)

    def assert_resealed_rejected(
        self,
        mutate: Callable[[dict[str, Any]], None],
        pattern: str,
    ) -> None:
        forged = self.fresh_receipt()
        mutate(forged)
        reseal_receipt(forged)
        with self.assertRaisesRegex((ValueError, KeyError, TypeError), pattern):
            matrix.validate_receipt(forged)

    def test_tracked_receipt_is_canonical_and_shallow_valid(self) -> None:
        raw = matrix.DEFAULT_OUTPUT.read_bytes()
        self.assertFalse(raw.startswith(b"\xef\xbb\xbf"))
        self.assertNotIn(b"\r", raw)
        self.assertTrue(raw.endswith(b"\n"))
        self.assertFalse(raw.endswith(b"\n\n"))
        parsed = json.loads(raw.decode("utf-8"))
        self.assertEqual(parsed, self.receipt)
        matrix.validate_receipt(self.receipt)

    def test_coordinated_base_material_identity_reseal_is_rejected(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            recipe = next(row for row in receipt["recipeCoverage"] if row["staticParameterSet"])
            static_set = recipe["staticParameterSet"]
            replacement = different_hex(static_set["baseMaterialIdHex"])
            static_set["baseMaterialIdHex"] = replacement
            static_set["normalized"]["baseMaterialIdHex"] = replacement
            static_set["engineEquivalent"]["baseMaterialIdHex"] = replacement
            static_set["serializedSemanticSha256"] = matrix.canonical_sha256(
                static_set["normalized"]
            )
            static_set["engineEqualitySha256"] = matrix.canonical_sha256(
                static_set["engineEquivalent"]
            )
            reseal_row(recipe)
            receipt["sourceStaticIdentityProjectionSha256"] = matrix.canonical_sha256(
                matrix.source_static_identity_projection(receipt["recipeCoverage"])
            )

        self.assert_resealed_rejected(mutate, "source static identity projection")

    def test_coordinated_static_switch_identity_reseal_is_rejected(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            recipe = next(
                row
                for row in receipt["recipeCoverage"]
                if row["staticParameterSet"]
                and row["staticParameterSet"]["normalized"]["staticSwitchParameters"]
            )
            static_set = recipe["staticParameterSet"]
            normalized_switch = static_set["normalized"]["staticSwitchParameters"][0]
            matching_engine_switch = next(
                row
                for row in static_set["engineEquivalent"]["staticSwitchParameters"]
                if row["parameterNameComparisonCasefold"]
                == normalized_switch["parameterNameComparisonCasefold"]
                and row["parameterNameNumber"] == normalized_switch["parameterNameNumber"]
                and row["expressionGuidHex"] == normalized_switch["expressionGuidHex"]
            )
            normalized_switch["value"] = not normalized_switch["value"]
            matching_engine_switch["value"] = normalized_switch["value"]
            static_set["serializedSemanticSha256"] = matrix.canonical_sha256(
                static_set["normalized"]
            )
            static_set["engineEqualitySha256"] = matrix.canonical_sha256(
                static_set["engineEquivalent"]
            )
            reseal_row(recipe)
            receipt["sourceStaticIdentityProjectionSha256"] = matrix.canonical_sha256(
                matrix.source_static_identity_projection(receipt["recipeCoverage"])
            )

        self.assert_resealed_rejected(mutate, "source static identity projection")

    def test_resealed_vertex_factory_selection_admission_is_rejected(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            recipe = next(
                row
                for row in receipt["recipeCoverage"]
                if row["recipeId"] == "material-recipe-03cc03b86c1a4c8f"
            )
            vertex_factory = recipe["cacheJoins"][0]["candidateVertexFactories"][0]
            vertex_factory["selectionAdmission"] = True
            reseal_row(recipe)

        self.assert_resealed_rejected(mutate, "candidate vertex factory was selected")

    def test_resealed_pass_classification_mutation_is_rejected(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            recipe = next(row for row in receipt["recipeCoverage"] if row["cacheJoins"])
            reference = recipe["cacheJoins"][0]["candidateVertexFactories"][0][
                "shaderReferences"
            ][0]
            expected = matrix.classify_shader_pass(reference["shaderType"])
            reference["passFamilyCandidate"] = (
                "DISTORTION" if expected != "DISTORTION" else "BASE_PASS"
            )
            reseal_row(recipe)

        self.assert_resealed_rejected(mutate, "shader pass classification")

    def test_resealed_shader_object_id_mutation_is_rejected(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            recipe = next(row for row in receipt["recipeCoverage"] if row["cacheJoins"])
            join = recipe["cacheJoins"][0]
            reference = join["candidateVertexFactories"][0]["shaderReferences"][0]
            candidate_ids = {row["shaderIdHex"] for row in join["shaderObjectCandidates"]}
            replacement = different_hex(reference["shaderIdHex"])
            if replacement in candidate_ids:
                replacement = "e" * len(reference["shaderIdHex"])
            reference["shaderIdHex"] = replacement
            reseal_row(recipe)

        self.assert_resealed_rejected(mutate, "shader reference candidate is absent")

    def test_resealed_main_golden_dxbc_mutation_is_rejected(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            selected = next(
                row
                for row in receipt["mainGoldenCrossCheck"]["rows"]
                if row["selectedOriginalDxbc"]
            )["selectedOriginalDxbc"][0]
            selected["dxbcSha256"] = different_hex(selected["dxbcSha256"])

        self.assert_resealed_rejected(
            mutate, "main golden (source|DXBC) projection"
        )

    def test_coordinated_main_golden_and_candidate_dxbc_reseal_is_rejected(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            golden = next(
                row
                for row in receipt["mainGoldenCrossCheck"]["rows"]
                if row["selectedOriginalDxbc"]
            )
            selected = golden["selectedOriginalDxbc"][0]
            recipe = next(
                row
                for row in receipt["recipeCoverage"]
                if row["recipeId"] == golden["recipeId"]
            )
            join = next(
                row for row in recipe["cacheJoins"] if row["cohortId"] == golden["cohortId"]
            )
            candidate = next(
                row
                for row in join["shaderObjectCandidates"]
                if row["shaderIdHex"] == selected["shaderIdHex"]
            )
            container = next(
                row
                for row in candidate["dxbcContainers"]
                if row["dxbcSha256"] == selected["dxbcSha256"]
            )
            replacement = different_hex(selected["dxbcSha256"])
            selected["dxbcSha256"] = replacement
            container["dxbcSha256"] = replacement
            reseal_row(recipe)

        self.assert_resealed_rejected(mutate, "main golden|DXBC|projection")


if __name__ == "__main__":
    unittest.main()
