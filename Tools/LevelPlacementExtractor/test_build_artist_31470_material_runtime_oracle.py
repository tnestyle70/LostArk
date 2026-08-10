#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from build_artist_31470_material_runtime_oracle import (
    DEFAULT_HLSL,
    DEFAULT_MATERIAL_CONTRACT,
    DEFAULT_OUTPUT,
    DEFAULT_RENDER_RECEIPT,
    DEFAULT_SHADER_RECEIPT,
    DEFAULT_SOURCE_VALUE_ACQUISITION_RECEIPT,
    ORACLE_INPUTS,
    build_material_feasibility_matrices,
    build_receipt,
    build_recipe_numeric_samples,
    build_recipe_operands,
    canonical_sha256,
    evaluate_cpu,
    feature_names,
    read_json,
    seal_receipt,
    tracked_text_sha256,
    validate_runtime_receipt,
    validate_runtime_receipt_source_bindings,
    validate_runtime_receipt_tracked_sources,
)
import build_artist_31470_material_source_value_acquisition as acquisition
from verify_artist_31470_material_runtime_oracle_hlsl import (
    DEFAULT_D3DCOMPILER,
    run_hlsl_oracle,
    run_warp_state_provider_oracle,
)


class Artist31470MaterialRuntimeOracleTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.material = read_json(DEFAULT_MATERIAL_CONTRACT)
        cls.render = read_json(DEFAULT_RENDER_RECEIPT)
        cls.shader = read_json(DEFAULT_SHADER_RECEIPT)
        cls.source_value_acquisition = read_json(
            DEFAULT_SOURCE_VALUE_ACQUISITION_RECEIPT
        )
        cls.receipt = read_json(DEFAULT_OUTPUT)

    def rebuild(self, *, render: dict | None = None) -> dict:
        return build_receipt(
            copy.deepcopy(self.material),
            copy.deepcopy(render if render is not None else self.render),
            copy.deepcopy(self.shader),
            copy.deepcopy(self.source_value_acquisition),
            DEFAULT_MATERIAL_CONTRACT,
            DEFAULT_RENDER_RECEIPT,
            DEFAULT_SHADER_RECEIPT,
            DEFAULT_SOURCE_VALUE_ACQUISITION_RECEIPT,
            DEFAULT_HLSL,
            copy.deepcopy(self.receipt["sourceRevisionShaderCacheAcquisition"]),
            copy.deepcopy(self.receipt["hlslVerification"]),
            copy.deepcopy(self.receipt["warpStateProviderVerification"]),
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
        self.assertEqual(summary["materialFeasibilityRowCount"], 255)
        self.assertEqual(summary["materialFeasibilityReadyCount"], 0)
        self.assertEqual(summary["materialFeasibilityBlockedCount"], 255)
        self.assertEqual(summary["sourceExactEvaluatorCount"], 0)
        self.assertEqual(summary["productRecipeCount"], 0)
        self.assertEqual(summary["productOccurrenceCount"], 0)
        self.assertFalse(self.receipt["admission"]["productAdmission"])
        self.assertTrue(self.receipt["admission"]["evidenceIntegrityAdmission"])
        self.assertFalse(self.receipt["admission"]["executionReadinessAdmission"])
        matrices = self.receipt["materialFeasibilityMatrices"]
        self.assertEqual(len(matrices["renderStateRows"]), 89)
        self.assertEqual(len(matrices["staticPermutationRows"]), 94)
        self.assertEqual(len(matrices["strictSamplerRows"]), 72)
        matrix_summary = matrices["summary"]
        self.assertEqual(matrix_summary["staticExactGuidJoinCount"], 66)
        self.assertEqual(
            matrix_summary["staticOverrideTrueSourceValueAcquiredCount"], 23
        )
        self.assertEqual(
            matrix_summary["staticNonoverrideSemanticsUnverifiedCount"], 43
        )
        self.assertEqual(matrix_summary["staticNoExactGuidEntryCount"], 28)
        self.assertEqual(
            matrix_summary["strictSamplerRejectedLegacyExactRowCount"], 4
        )
        self.assertEqual(
            matrix_summary["strictSamplerSourceTextureEvidenceRowCount"], 72
        )

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
        state_replay = run_warp_state_provider_oracle()
        self.assertEqual(
            state_replay, self.receipt["warpStateProviderVerification"]
        )
        self.assertEqual(state_replay["pilotCount"], 4)

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

    def test_coordinated_upstream_receipt_reseals_are_rejected(self) -> None:
        with self.subTest(receipt="render"), tempfile.TemporaryDirectory() as root:
            mutated_render = copy.deepcopy(self.render)
            mutated_render["skillId"] = 999
            seal_receipt(mutated_render)
            render_path = Path(root) / "mutated-render.json"
            render_path.write_text(
                json.dumps(
                    mutated_render,
                    ensure_ascii=False,
                    indent=2,
                    allow_nan=False,
                )
                + "\n",
                encoding="utf-8",
                newline="\n",
            )
            downstream = copy.deepcopy(self.receipt)
            downstream["sourceEvidence"]["renderReceiptSha256"] = mutated_render[
                "receiptSha256"
            ]
            downstream["sourceEvidence"][
                "renderReceiptTrackedTextSha256"
            ] = tracked_text_sha256(render_path)
            seal_receipt(downstream)
            validate_runtime_receipt(downstream)
            with self.assertRaisesRegex(ValueError, "render receipt root identity"):
                validate_runtime_receipt_source_bindings(
                    downstream,
                    self.material,
                    mutated_render,
                    self.shader,
                    self.source_value_acquisition,
                    material_contract_path=DEFAULT_MATERIAL_CONTRACT,
                    render_receipt_path=render_path,
                )

        with self.subTest(receipt="shader-cache"), tempfile.TemporaryDirectory() as root:
            mutated_shader = copy.deepcopy(self.shader)
            mutated_shader["summary"]["exactMaterialShaderMapJoinCount"] = 1
            seal_receipt(mutated_shader)
            shader_path = Path(root) / "mutated-shader.json"
            shader_path.write_text(
                json.dumps(
                    mutated_shader,
                    ensure_ascii=False,
                    indent=2,
                    allow_nan=False,
                )
                + "\n",
                encoding="utf-8",
                newline="\n",
            )
            downstream = copy.deepcopy(self.receipt)
            downstream["sourceEvidence"]["shaderCacheReceiptSha256"] = (
                mutated_shader["receiptSha256"]
            )
            downstream["sourceEvidence"][
                "shaderCacheReceiptTrackedTextSha256"
            ] = tracked_text_sha256(shader_path)
            downstream["materialFeasibilityMatrices"] = (
                build_material_feasibility_matrices(
                    self.material,
                    mutated_shader,
                    downstream["warpStateProviderVerification"],
                    self.source_value_acquisition,
                )
            )
            seal_receipt(downstream)
            with self.assertRaisesRegex(ValueError, "approved semantic"):
                validate_runtime_receipt(downstream)
            with self.assertRaisesRegex(ValueError, "denominator changed"):
                validate_runtime_receipt_source_bindings(
                    downstream,
                    self.material,
                    self.render,
                    mutated_shader,
                    self.source_value_acquisition,
                    material_contract_path=DEFAULT_MATERIAL_CONTRACT,
                    render_receipt_path=DEFAULT_RENDER_RECEIPT,
                )

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
        with self.assertRaisesRegex(ValueError, "HLSL replay binding"):
            validate_runtime_receipt(mutated)
        with self.assertRaisesRegex(ValueError, "typed input is not source-bound"):
            validate_runtime_receipt_source_bindings(
                mutated,
                self.material,
                self.render,
                self.shader,
                self.source_value_acquisition,
            )

    def test_occurrence_cannot_bind_another_recipe_sha_or_evaluator(self) -> None:
        mutated = copy.deepcopy(self.receipt)
        occurrence = mutated["occurrenceBindings"][0]
        other = next(
            recipe
            for recipe in mutated["materialRecipeBindings"]
            if recipe["recipeId"] != occurrence["materialRecipeId"]
        )
        occurrence["materialBindingSha256"] = other["bindingSha256"]
        occurrence["evaluatorId"] = other["evaluatorId"]
        occurrence["evaluatorVersion"] = other["evaluatorVersion"]
        occurrence.pop("bindingSha256")
        occurrence["bindingSha256"] = canonical_sha256(occurrence)
        seal_receipt(mutated)
        with self.assertRaisesRegex(ValueError, "exact recipe binding"):
            validate_runtime_receipt(mutated)

    def test_raw_expression_feature_mask_coordinated_reseal_is_rejected(self) -> None:
        mutated = copy.deepcopy(self.receipt)
        family = mutated["familyEvaluators"][0]
        family["featureMask"] ^= 1
        family["features"] = feature_names(family["featureMask"])
        family.pop("evaluatorSha256")
        family["evaluatorSha256"] = canonical_sha256(family)
        seal_receipt(mutated)
        with self.assertRaisesRegex(ValueError, "raw-expression-bound"):
            validate_runtime_receipt_source_bindings(
                mutated,
                self.material,
                self.render,
                self.shader,
                self.source_value_acquisition,
            )

    def test_input_and_static_order_owner_reseal_is_rejected(self) -> None:
        for collection, error in (
            ("orderedInputBindings", "typed input is not source-bound"),
            ("orderedStaticSwitchBindings", "static switch is not source-bound"),
        ):
            with self.subTest(collection=collection):
                mutated = copy.deepcopy(self.receipt)
                recipe = next(
                    row
                    for row in mutated["materialRecipeBindings"]
                    if row[collection]
                )
                recipe[collection][0]["sourceSectionIndex"] += 1
                recipe.pop("bindingSha256")
                recipe["bindingSha256"] = canonical_sha256(recipe)
                seal_receipt(mutated)
                with self.assertRaisesRegex(ValueError, error):
                    validate_runtime_receipt_source_bindings(
                        mutated,
                        self.material,
                        self.render,
                        self.shader,
                        self.source_value_acquisition,
                    )

    def test_material_feasibility_denominator_and_owner_reseal_are_rejected(self) -> None:
        for collection in (
            "renderStateRows",
            "staticPermutationRows",
            "strictSamplerRows",
        ):
            with self.subTest(collection=collection):
                shrunk = copy.deepcopy(self.receipt)
                shrunk["materialFeasibilityMatrices"][collection].pop()
                seal_receipt(shrunk)
                with self.assertRaisesRegex(ValueError, "feasibility denominator"):
                    validate_runtime_receipt(shrunk)

        for owner_field in ("owner", "finalRuntimeOwner"):
            with self.subTest(owner_field=owner_field):
                ownerless = copy.deepcopy(self.receipt)
                ownerless["materialFeasibilityMatrices"]["renderStateRows"][0][
                    owner_field
                ] = ""
                seal_receipt(ownerless)
                with self.assertRaisesRegex(ValueError, "owner or decision"):
                    validate_runtime_receipt(ownerless)

        owner_swapped = copy.deepcopy(self.receipt)
        owner_swapped["materialFeasibilityMatrices"]["renderStateRows"][0][
            "finalRuntimeOwner"
        ] = "FORGED_OWNER"
        seal_receipt(owner_swapped)
        with self.assertRaisesRegex(ValueError, "matrices are not source-bound"):
            validate_runtime_receipt_source_bindings(
                owner_swapped,
                self.material,
                self.render,
                self.shader,
                self.source_value_acquisition,
            )

    def test_source_archive_projection_and_tracked_hash_reseal_are_rejected(self) -> None:
        archive_mutated = copy.deepcopy(self.receipt)
        archive_mutated["sourceRevisionShaderCacheAcquisition"][
            "inventoryProjectionSha256"
        ] = "0" * 64
        seal_receipt(archive_mutated)
        with self.assertRaisesRegex(ValueError, "deep projection"):
            validate_runtime_receipt(archive_mutated)

        tracked_mutated = copy.deepcopy(self.receipt)
        tracked_mutated["sourceEvidence"][
            "materialContractTrackedTextSha256"
        ] = "0" * 64
        seal_receipt(tracked_mutated)
        validate_runtime_receipt(tracked_mutated)
        with self.assertRaisesRegex(ValueError, "tracked source changed"):
            validate_runtime_receipt_tracked_sources(
                tracked_mutated,
                DEFAULT_MATERIAL_CONTRACT,
                DEFAULT_RENDER_RECEIPT,
                DEFAULT_SHADER_RECEIPT,
                DEFAULT_SOURCE_VALUE_ACQUISITION_RECEIPT,
                DEFAULT_HLSL,
            )

    def test_actual_warp_rejects_nan_positive_and_negative_infinity(self) -> None:
        source = DEFAULT_HLSL.read_text(encoding="utf-8")
        needle = (
            "    Outputs[DispatchThreadId.x] = EvaluateReconstructedMaterial(\n"
            "        Inputs[DispatchThreadId.x]);"
        )
        self.assertIn(needle, source)
        for label, bits in (
            ("nan", "0x7fc00000u"),
            ("positive-infinity", "0x7f800000u"),
            ("negative-infinity", "0xff800000u"),
        ):
            with self.subTest(label=label), tempfile.TemporaryDirectory() as root:
                replacement = (
                    "    float NonFinite = asfloat(" + bits + ");\n"
                    "    Outputs[DispatchThreadId.x] = float4("
                    "NonFinite, NonFinite, NonFinite, NonFinite);"
                )
                mutated_hlsl = Path(root) / "NonFiniteMaterialOracle.hlsl"
                mutated_hlsl.write_text(
                    source.replace(needle, replacement),
                    encoding="utf-8",
                    newline="\n",
                )
                with self.assertRaisesRegex(RuntimeError, "non-finite lane"):
                    run_hlsl_oracle(
                        self.receipt, mutated_hlsl, DEFAULT_D3DCOMPILER
                    )

    def test_evaluator_and_hlsl_coordinated_reseals_are_rejected(self) -> None:
        evaluator_mutations = {
            "source-exact": lambda contract: contract.update(
                sourceExact=True, fidelity="SOURCE_EXACT"
            ),
            "empty-operation-order": lambda contract: contract.update(
                operationOrder=[]
            ),
            "reversed-operation-order": lambda contract: contract.update(
                operationOrder=list(reversed(contract["operationOrder"]))
            ),
            "bool-tolerance": lambda contract: contract.update(
                numericTolerance=True
            ),
            "string-nan-tolerance": lambda contract: contract.update(
                numericTolerance="NaN"
            ),
            "bool-input": lambda contract: contract["inputSamples"][0].update(
                time=True
            ),
        }
        for label, mutate in evaluator_mutations.items():
            with self.subTest(evaluator=label):
                mutated = copy.deepcopy(self.receipt)
                mutate(mutated["evaluatorContract"])
                seal_receipt(mutated)
                with self.assertRaisesRegex(ValueError, "evaluator contract"):
                    validate_runtime_receipt(mutated)

        hlsl_mutations = {
            "zero-samples": lambda hlsl: hlsl.update(sampleCount=0),
            "oversized-error": lambda hlsl: hlsl.update(maxAbsoluteError=999.0),
            "zero-compiler": lambda hlsl: hlsl["compiler"].update(
                rawSha256="0" * 64
            ),
            "zero-dxbc": lambda hlsl: hlsl.update(compiledDxbcSha256="0" * 64),
            "zero-input": lambda hlsl: hlsl.update(inputBytesSha256="0" * 64),
            "zero-output": lambda hlsl: hlsl.update(
                outputFloat32BytesSha256="0" * 64
            ),
            "zero-hlsl-source": lambda hlsl: hlsl.update(
                hlslTrackedTextSha256="0" * 64
            ),
            "zero-replay": lambda hlsl: hlsl.update(
                replayBindingSha256="0" * 64
            ),
        }
        for label, mutate in hlsl_mutations.items():
            with self.subTest(hlsl=label):
                mutated = copy.deepcopy(self.receipt)
                mutate(mutated["hlslVerification"])
                seal_receipt(mutated)
                with self.assertRaisesRegex(ValueError, "Material HLSL"):
                    validate_runtime_receipt(mutated)

    def test_cpu_numeric_lanes_reject_bool_nonfinite_and_f32_overflow(self) -> None:
        direct_mutations = {
            "bool-time": lambda sample: sample.update(time=True),
            "positive-infinity": lambda sample: sample["texture0"].__setitem__(
                0, float("inf")
            ),
            "float32-overflow": lambda sample: sample["texture0"].__setitem__(
                0, 3.5e38
            ),
        }
        for label, mutate in direct_mutations.items():
            with self.subTest(direct=label):
                sample = copy.deepcopy(ORACLE_INPUTS[0])
                mutate(sample)
                with self.assertRaises(ValueError):
                    evaluate_cpu(1, sample)

        family_mutated = copy.deepcopy(self.receipt)
        family = family_mutated["familyEvaluators"][0]
        family["sampleRows"][2]["expectedFloat4"][3] = False
        family.pop("evaluatorSha256")
        family["evaluatorSha256"] = canonical_sha256(family)
        seal_receipt(family_mutated)
        with self.assertRaisesRegex(ValueError, "exact JSON number"):
            validate_runtime_receipt(family_mutated)

        recipe_mutated = copy.deepcopy(self.receipt)
        recipe = recipe_mutated["materialRecipeBindings"][0]
        recipe["numericBindingSamples"][0]["expectedFloat4"][3] = False
        recipe.pop("bindingSha256")
        recipe["bindingSha256"] = canonical_sha256(recipe)
        seal_receipt(recipe_mutated)
        with self.assertRaisesRegex(ValueError, "exact JSON number"):
            validate_runtime_receipt(recipe_mutated)

    def test_acquisition_promotion_reseal_is_rejected_at_every_runtime_boundary(
        self,
    ) -> None:
        mutated_acquisition = copy.deepcopy(self.source_value_acquisition)
        row = next(
            row
            for row in mutated_acquisition["matrices"]["strictSamplerRows"]
            if row["matrixRowId"]
            == "material-feasibility-sampler-316a56b9a4bd256c"
        )
        row["fullDescriptorSourceExact"] = True
        row["sourceValueAcquired"] = True
        row["sourceValueDecision"] = "SOURCE_EXACT_FULL_DESCRIPTOR"
        row["strictReauditDecision"] = "PASS"
        row["executionReady"] = True
        mutated_acquisition["summary"]["strictSamplerSourceValueAcquiredCount"] = 1
        mutated_acquisition["summary"]["strictExecutionReadyCount"] = 1
        mutated_acquisition["summary"]["strictSamplerRowSetSha256"] = (
            acquisition.canonical_sha256(
                mutated_acquisition["matrices"]["strictSamplerRows"]
            )
        )
        mutated_acquisition.pop("receiptSha256")
        mutated_acquisition["receiptSha256"] = acquisition.canonical_sha256(
            mutated_acquisition
        )

        with self.assertRaisesRegex(ValueError, "sampler approved semantic"):
            validate_runtime_receipt_source_bindings(
                self.receipt,
                self.material,
                self.render,
                self.shader,
                mutated_acquisition,
            )

        with tempfile.TemporaryDirectory() as root:
            acquisition_path = Path(root) / "promoted-acquisition.json"
            acquisition_path.write_text(
                json.dumps(
                    mutated_acquisition,
                    ensure_ascii=False,
                    indent=2,
                    allow_nan=False,
                )
                + "\n",
                encoding="utf-8",
                newline="\n",
            )
            downstream = copy.deepcopy(self.receipt)
            downstream["sourceEvidence"][
                "sourceValueAcquisitionReceiptSha256"
            ] = mutated_acquisition["receiptSha256"]
            downstream["sourceEvidence"][
                "sourceValueAcquisitionTrackedTextSha256"
            ] = tracked_text_sha256(acquisition_path)
            seal_receipt(downstream)
            with self.assertRaisesRegex(ValueError, "sampler approved semantic"):
                validate_runtime_receipt_tracked_sources(
                    downstream,
                    DEFAULT_MATERIAL_CONTRACT,
                    DEFAULT_RENDER_RECEIPT,
                    DEFAULT_SHADER_RECEIPT,
                    acquisition_path,
                    DEFAULT_HLSL,
                )

    def test_static_semantic_forges_are_rejected_at_runtime_boundaries(
        self,
    ) -> None:
        cases = (
            (
                "override23-value",
                "material-feasibility-static-104ba0eb7fef8369",
                lambda row: row["micNativeSelection"]["entry"].update(
                    value=False
                ),
                "MIC value decoded/raw semantics",
            ),
            (
                "nonoverride43-override",
                "material-feasibility-static-25e206c498b44f77",
                lambda row: row["micNativeSelection"]["entry"].update(
                    bOverride=True
                ),
                "MIC bOverride decoded/raw semantics",
            ),
            (
                "unmatched28-parent-default",
                "material-feasibility-static-1b69b57952caaa03",
                lambda row: row["parentExpression"][
                    "defaultValueProperty"
                ].update(value=False),
                "parent default decoded/raw semantics",
            ),
            (
                "unmatched28-forged-exact-count",
                "material-feasibility-static-1b69b57952caaa03",
                lambda row: row["micNativeSelection"].update(
                    exactNameAndGuidMatchCount=1
                ),
                "MIC entry owner/offset",
            ),
        )
        for label, row_id, mutate, expected_error in cases:
            with self.subTest(label=label):
                mutated_acquisition = copy.deepcopy(
                    self.source_value_acquisition
                )
                row = next(
                    row
                    for row in mutated_acquisition["matrices"][
                        "staticPermutationRows"
                    ]
                    if row["matrixRowId"] == row_id
                )
                mutate(row)
                mutated_static_sha = acquisition.canonical_sha256(
                    mutated_acquisition["matrices"]["staticPermutationRows"]
                )
                mutated_acquisition["summary"]["staticRowSetSha256"] = (
                    mutated_static_sha
                )
                mutated_acquisition.pop("receiptSha256")
                mutated_acquisition["receiptSha256"] = (
                    acquisition.canonical_sha256(mutated_acquisition)
                )
                with mock.patch.object(
                    acquisition,
                    "APPROVED_STATIC_ROW_SET_SHA256",
                    mutated_static_sha,
                ):
                    with self.assertRaisesRegex(ValueError, expected_error):
                        build_receipt(
                            copy.deepcopy(self.material),
                            copy.deepcopy(self.render),
                            copy.deepcopy(self.shader),
                            copy.deepcopy(mutated_acquisition),
                            DEFAULT_MATERIAL_CONTRACT,
                            DEFAULT_RENDER_RECEIPT,
                            DEFAULT_SHADER_RECEIPT,
                            DEFAULT_SOURCE_VALUE_ACQUISITION_RECEIPT,
                            DEFAULT_HLSL,
                            copy.deepcopy(
                                self.receipt[
                                    "sourceRevisionShaderCacheAcquisition"
                                ]
                            ),
                            copy.deepcopy(self.receipt["hlslVerification"]),
                            copy.deepcopy(
                                self.receipt["warpStateProviderVerification"]
                            ),
                        )
                    with self.assertRaisesRegex(ValueError, expected_error):
                        validate_runtime_receipt_source_bindings(
                            self.receipt,
                            self.material,
                            self.render,
                            self.shader,
                            mutated_acquisition,
                        )

                    with tempfile.TemporaryDirectory() as root:
                        acquisition_path = (
                            Path(root) / f"forged-static-{label}.json"
                        )
                        acquisition_path.write_text(
                            json.dumps(
                                mutated_acquisition,
                                ensure_ascii=False,
                                indent=2,
                                allow_nan=False,
                            )
                            + "\n",
                            encoding="utf-8",
                            newline="\n",
                        )
                        downstream = copy.deepcopy(self.receipt)
                        downstream["sourceEvidence"][
                            "sourceValueAcquisitionReceiptSha256"
                        ] = mutated_acquisition["receiptSha256"]
                        downstream["sourceEvidence"][
                            "sourceValueAcquisitionTrackedTextSha256"
                        ] = tracked_text_sha256(acquisition_path)
                        seal_receipt(downstream)
                        with self.assertRaisesRegex(
                            ValueError, expected_error
                        ):
                            validate_runtime_receipt_tracked_sources(
                                downstream,
                                DEFAULT_MATERIAL_CONTRACT,
                                DEFAULT_RENDER_RECEIPT,
                                DEFAULT_SHADER_RECEIPT,
                                acquisition_path,
                                DEFAULT_HLSL,
                            )

    def test_warp_capture_and_product_summary_reseals_are_rejected(self) -> None:
        state_value = copy.deepcopy(self.receipt)
        state_value["warpStateProviderVerification"]["pilots"][0][
            "actualStateOutputs"
        ][0]["SrcBlend"] = 999
        state_value["warpStateProviderVerification"][
            "pilotProjectionSha256"
        ] = canonical_sha256(
            state_value["warpStateProviderVerification"]["pilots"]
        )
        seal_receipt(state_value)
        with self.assertRaisesRegex(ValueError, "WARP state output replay"):
            validate_runtime_receipt(state_value)

        state_tolerance = copy.deepcopy(self.receipt)
        state_tolerance["warpStateProviderVerification"]["pilots"][0][
            "numericTolerance"
        ] = False
        state_tolerance["warpStateProviderVerification"][
            "pilotProjectionSha256"
        ] = canonical_sha256(
            state_tolerance["warpStateProviderVerification"]["pilots"]
        )
        seal_receipt(state_tolerance)
        with self.assertRaisesRegex(ValueError, "WARP state pilot metadata"):
            validate_runtime_receipt(state_tolerance)

        capture = copy.deepcopy(self.receipt)
        capture["controlledCaptureAssessment"].update(
            available=True,
            uncontrolledInstalledGameProcessUsed=True,
            decision="SOURCE_EXACT_CAPTURE_AVAILABLE",
        )
        seal_receipt(capture)
        with self.assertRaisesRegex(ValueError, "controlled-capture"):
            validate_runtime_receipt(capture)

        summary_mutated = copy.deepcopy(self.receipt)
        summary_mutated["summary"].update(
            runtimeHandlerConsumedRecipeCount=27,
            runtimeHandlerConsumedOccurrenceCount=34,
            productRecipeCount=27,
            productOccurrenceCount=34,
        )
        seal_receipt(summary_mutated)
        with self.assertRaisesRegex(ValueError, "consumption summary"):
            validate_runtime_receipt(summary_mutated)

    def test_product_and_runtime_handler_admission_cannot_be_opened(self) -> None:
        for path in (
            ("admission", "productAdmission"),
            ("admission", "materialRuntimeHandlerConsumptionAdmission"),
            ("admission", "rendererConsumptionAdmission"),
            ("admission", "executionReadinessAdmission"),
        ):
            mutated = copy.deepcopy(self.receipt)
            mutated[path[0]][path[1]] = True
            seal_receipt(mutated)
            with self.assertRaisesRegex(ValueError, "admission"):
                validate_runtime_receipt(mutated)

    def test_runtime_root_summary_family_and_occurrence_cannot_be_resealed(self) -> None:
        cases = (
            "root-extra",
            "source-extra",
            "summary-extra",
            "admission-extra",
            "reconstructed-count",
            "input-kind-count",
            "input-role-count",
            "empty-admission-blockers",
            "family-hlsl-false",
            "family-arithmetic-false",
            "family-empty-evidence-blockers",
            "family-empty-runtime-blockers",
            "family-extra",
            "occurrence-extra",
        )
        for label in cases:
            with self.subTest(label=label):
                mutated = copy.deepcopy(self.receipt)
                if label == "root-extra":
                    mutated["forgedProductAdmission"] = True
                elif label == "source-extra":
                    mutated["sourceEvidence"]["forgedProductAdmission"] = True
                elif label == "summary-extra":
                    mutated["summary"]["forgedProductAdmission"] = True
                elif label == "admission-extra":
                    mutated["admission"]["forgedProductAdmission"] = True
                elif label == "reconstructed-count":
                    mutated["summary"][
                        "reconstructedNumericallyVerifiedEvaluatorCount"
                    ] = 0
                elif label == "input-kind-count":
                    mutated["summary"]["inputKindCounts"]["scalar"] = 0
                elif label == "input-role-count":
                    mutated["summary"]["inputRoleCounts"]["ALPHA_SCALAR"] = 0
                elif label == "empty-admission-blockers":
                    mutated["admission"]["blockers"] = []
                elif label.startswith("family-"):
                    family = mutated["familyEvaluators"][0]
                    if label == "family-hlsl-false":
                        family["hlslNumericOracleVerified"] = False
                    elif label == "family-arithmetic-false":
                        family["arithmeticEvaluationAdmission"] = False
                    elif label == "family-empty-evidence-blockers":
                        family["evidenceBlockers"] = []
                    elif label == "family-empty-runtime-blockers":
                        family["runtimeBlockers"] = []
                    else:
                        family["forgedProductAdmission"] = True
                    family.pop("evaluatorSha256")
                    family["evaluatorSha256"] = canonical_sha256(family)
                else:
                    occurrence = mutated["occurrenceBindings"][0]
                    occurrence["forgedProductAdmission"] = True
                    occurrence.pop("bindingSha256")
                    occurrence["bindingSha256"] = canonical_sha256(
                        occurrence
                    )
                seal_receipt(mutated)
                with self.assertRaises(ValueError):
                    validate_runtime_receipt(mutated)

    def test_coordinated_external_capture_acquisition_reseal_is_rejected(self) -> None:
        mutated_acquisition = copy.deepcopy(self.source_value_acquisition)
        search = mutated_acquisition["externalArtifactSearch"]
        search["scopeBoundary"]["globalExhaustionClaim"] = True
        search["scopeBoundary"]["volumeShadowCopy"].update(
            status="EXHAUSTED_NO_PROVIDER", admissionInput=True
        )
        search["controlledRuntimeCapture"].update(
            safeProviderAvailable=True,
            sourceRevisionRuntimeBundleAvailable=True,
            sourceRevisionDebugOrCaptureApiAvailable=True,
            currentInstalledProcessIsSourceRevisionAuthenticated=True,
            decision="SOURCE_EXACT_CAPTURE_AVAILABLE",
        )
        mutated_acquisition.pop("receiptSha256")
        mutated_acquisition["receiptSha256"] = acquisition.canonical_sha256(
            mutated_acquisition
        )

        with self.assertRaisesRegex(ValueError, "external artifact"):
            build_receipt(
                copy.deepcopy(self.material),
                copy.deepcopy(self.render),
                copy.deepcopy(self.shader),
                copy.deepcopy(mutated_acquisition),
                DEFAULT_MATERIAL_CONTRACT,
                DEFAULT_RENDER_RECEIPT,
                DEFAULT_SHADER_RECEIPT,
                DEFAULT_SOURCE_VALUE_ACQUISITION_RECEIPT,
                DEFAULT_HLSL,
                copy.deepcopy(
                    self.receipt["sourceRevisionShaderCacheAcquisition"]
                ),
                copy.deepcopy(self.receipt["hlslVerification"]),
                copy.deepcopy(self.receipt["warpStateProviderVerification"]),
            )
        with self.assertRaisesRegex(ValueError, "external artifact"):
            validate_runtime_receipt_source_bindings(
                self.receipt,
                self.material,
                self.render,
                self.shader,
                mutated_acquisition,
            )

        with tempfile.TemporaryDirectory() as root:
            acquisition_path = Path(root) / "forged-external-acquisition.json"
            acquisition_path.write_text(
                json.dumps(
                    mutated_acquisition,
                    ensure_ascii=False,
                    indent=2,
                    allow_nan=False,
                )
                + "\n",
                encoding="utf-8",
                newline="\n",
            )
            downstream = copy.deepcopy(self.receipt)
            downstream["sourceEvidence"][
                "sourceValueAcquisitionReceiptSha256"
            ] = mutated_acquisition["receiptSha256"]
            downstream["sourceEvidence"][
                "sourceValueAcquisitionTrackedTextSha256"
            ] = tracked_text_sha256(acquisition_path)
            seal_receipt(downstream)
            with self.assertRaisesRegex(ValueError, "external artifact"):
                validate_runtime_receipt_tracked_sources(
                    downstream,
                    DEFAULT_MATERIAL_CONTRACT,
                    DEFAULT_RENDER_RECEIPT,
                    DEFAULT_SHADER_RECEIPT,
                    acquisition_path,
                    DEFAULT_HLSL,
                )

    def test_shader_cache_candidate_cannot_be_laundered(self) -> None:
        mutated = copy.deepcopy(self.receipt)
        acquisition = mutated["sourceRevisionShaderCacheAcquisition"]
        acquisition["shaderCacheNameCandidateCount"] = 1
        acquisition["decision"] = (
            "SOURCE_REVISION_SHADER_CACHE_CANDIDATE_REQUIRES_FULL_DECODE"
        )
        seal_receipt(mutated)
        with self.assertRaisesRegex(ValueError, "deep projection"):
            validate_runtime_receipt(mutated)


if __name__ == "__main__":
    unittest.main()
