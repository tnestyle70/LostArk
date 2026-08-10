#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import unittest
from pathlib import Path

from build_artist_31470_material_runtime_oracle import (
    DEFAULT_HLSL,
    DEFAULT_MATERIAL_CONTRACT,
    DEFAULT_OUTPUT,
    DEFAULT_RENDER_RECEIPT,
    DEFAULT_SHADER_RECEIPT,
    build_receipt,
    build_recipe_numeric_samples,
    build_recipe_operands,
    canonical_sha256,
    read_json,
    seal_receipt,
    validate_runtime_receipt,
    validate_runtime_receipt_source_bindings,
)
from verify_artist_31470_material_runtime_oracle_hlsl import (
    DEFAULT_D3DCOMPILER,
    run_hlsl_oracle,
)


class Artist31470MaterialRuntimeOracleTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.material = read_json(DEFAULT_MATERIAL_CONTRACT)
        cls.render = read_json(DEFAULT_RENDER_RECEIPT)
        cls.shader = read_json(DEFAULT_SHADER_RECEIPT)
        cls.receipt = read_json(DEFAULT_OUTPUT)

    def rebuild(self, *, render: dict | None = None) -> dict:
        return build_receipt(
            copy.deepcopy(self.material),
            copy.deepcopy(render if render is not None else self.render),
            copy.deepcopy(self.shader),
            DEFAULT_MATERIAL_CONTRACT,
            DEFAULT_RENDER_RECEIPT,
            DEFAULT_SHADER_RECEIPT,
            DEFAULT_HLSL,
            copy.deepcopy(self.receipt["sourceRevisionShaderCacheAcquisition"]),
            copy.deepcopy(self.receipt["hlslVerification"]),
        )

    def test_baseline_denominators_and_fail_closed_product(self) -> None:
        validate_runtime_receipt(self.receipt)
        summary = self.receipt["summary"]
        self.assertEqual(summary["materialFamilyCount"], 23)
        self.assertEqual(summary["materialRecipeBindingCount"], 27)
        self.assertEqual(summary["materialOccurrenceBindingCount"], 34)
        self.assertEqual(summary["inputBindingCount"], 729)
        self.assertEqual(summary["staticSwitchBindingCount"], 94)
        self.assertEqual(summary["totalTypedFieldBindingCount"], 823)
        self.assertEqual(summary["renderStateBindingCount"], 162)
        self.assertEqual(summary["resolvedRenderStateBindingCount"], 73)
        self.assertEqual(summary["unresolvedRenderStateBindingCount"], 89)
        self.assertEqual(summary["familyNumericSampleCount"], 92)
        self.assertEqual(summary["recipeNumericSampleCount"], 108)
        self.assertEqual(summary["hlslSampleCount"], 200)
        self.assertEqual(summary["sourceExactEvaluatorCount"], 0)
        self.assertEqual(summary["productRecipeCount"], 0)
        self.assertEqual(summary["productOccurrenceCount"], 0)
        self.assertFalse(self.receipt["admission"]["productAdmission"])

    def test_checked_receipt_rebuilds_from_pinned_sources(self) -> None:
        self.assertEqual(self.rebuild(), self.receipt)

    def test_source_archive_acquisition_denominator_is_fixed(self) -> None:
        acquisition = self.receipt["sourceRevisionShaderCacheAcquisition"]
        self.assertEqual(acquisition["fileCount"], 1813)
        self.assertEqual(acquisition["uniquePackageContentCount"], 624)
        self.assertEqual(acquisition["duplicateContentFileCount"], 1189)
        self.assertEqual(acquisition["shaderCacheNameCandidateCount"], 0)
        self.assertEqual(
            acquisition["decision"],
            "SOURCE_REVISION_SHADER_CACHE_NOT_PRESENT_IN_SCANNED_ARCHIVE",
        )

    def test_actual_warp_hlsl_replay_matches_stored_receipt(self) -> None:
        replay = run_hlsl_oracle(
            self.receipt, DEFAULT_HLSL, DEFAULT_D3DCOMPILER
        )
        self.assertEqual(replay, self.receipt["hlslVerification"])
        self.assertEqual(replay["sampleCount"], 200)
        self.assertLessEqual(replay["maxAbsoluteError"], 2.0e-5)

    def test_resealed_render_projection_mutation_is_rejected_by_pin(self) -> None:
        mutated = copy.deepcopy(self.render)
        mutated["graphExpressions"][0]["projection"]["parameterName"] = (
            "forged_parameter"
        )
        seal_receipt(mutated)
        with self.assertRaisesRegex(
            ValueError, "does not pin the supplied render receipt"
        ):
            self.rebuild(render=mutated)

    def test_family_feature_mutation_is_rejected(self) -> None:
        mutated = copy.deepcopy(self.receipt)
        family = mutated["familyEvaluators"][0]
        family["featureMask"] ^= 1
        family["features"] = ["FORGED"]
        family.pop("evaluatorSha256")
        family["evaluatorSha256"] = canonical_sha256(family)
        seal_receipt(mutated)
        with self.assertRaisesRegex(ValueError, "feature mask"):
            validate_runtime_receipt(mutated)

    def test_coordinated_recipe_value_reseal_does_not_match_sources(self) -> None:
        mutated = copy.deepcopy(self.receipt)
        recipe = mutated["materialRecipeBindings"][0]
        scalar = next(
            field
            for field in recipe["orderedInputBindings"]
            if field["fieldKind"] == "scalar"
        )
        scalar["typedValue"] = float(scalar["typedValue"]) + 17.0
        scalar["typedValueSha256"] = canonical_sha256(scalar["typedValue"])
        operands = build_recipe_operands(
            recipe["orderedInputBindings"], recipe["recipeFeatureMask"]
        )
        recipe["runtimeOperandBindings"] = operands
        recipe["numericBindingSamples"] = build_recipe_numeric_samples(
            recipe["recipeFeatureMask"], operands
        )
        recipe.pop("bindingSha256")
        recipe["bindingSha256"] = canonical_sha256(recipe)
        for occurrence in mutated["occurrenceBindings"]:
            if occurrence["materialRecipeId"] != recipe["recipeId"]:
                continue
            occurrence["materialBindingSha256"] = recipe["bindingSha256"]
            occurrence.pop("bindingSha256")
            occurrence["bindingSha256"] = canonical_sha256(occurrence)
        seal_receipt(mutated)
        validate_runtime_receipt(mutated)
        with self.assertRaisesRegex(ValueError, "typed input is not source-bound"):
            validate_runtime_receipt_source_bindings(mutated, self.material)

    def test_product_and_runtime_handler_admission_cannot_be_opened(self) -> None:
        for path in (
            ("admission", "productAdmission"),
            ("admission", "materialRuntimeHandlerConsumptionAdmission"),
        ):
            mutated = copy.deepcopy(self.receipt)
            mutated[path[0]][path[1]] = True
            seal_receipt(mutated)
            with self.assertRaisesRegex(ValueError, "admission"):
                validate_runtime_receipt(mutated)

    def test_shader_cache_candidate_cannot_be_laundered(self) -> None:
        mutated = copy.deepcopy(self.receipt)
        acquisition = mutated["sourceRevisionShaderCacheAcquisition"]
        acquisition["shaderCacheNameCandidateCount"] = 1
        acquisition["decision"] = (
            "SOURCE_REVISION_SHADER_CACHE_CANDIDATE_REQUIRES_FULL_DECODE"
        )
        seal_receipt(mutated)
        with self.assertRaisesRegex(ValueError, "ShaderCache decision"):
            validate_runtime_receipt(mutated)


if __name__ == "__main__":
    unittest.main()
