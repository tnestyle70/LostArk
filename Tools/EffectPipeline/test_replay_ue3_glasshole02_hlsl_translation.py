#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import unittest

import replay_ue3_glasshole02_hlsl_translation as translation


class Glasshole02HlslTranslationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.generated = translation.build_receipt(
            translation.DEFAULT_RAW_DXBC,
            translation.DEFAULT_INCLUDE,
            translation.DEFAULT_SHADER,
            translation.DEFAULT_UNIFORM_RECEIPT,
            translation.DEFAULT_D3DCOMPILER,
        )

    def test_raw_and_translated_shader_abi_is_exactly_joined(self) -> None:
        abi = self.generated["shaderAbi"]
        self.assertEqual(abi["rawInstructionCount"], 198)
        self.assertEqual(abi["constantBufferFloat4Counts"], {"0": 22, "2": 4})
        self.assertEqual(abi["textureRegisters"], list(range(8)))
        self.assertEqual(abi["samplerRegisters"], list(range(8)))
        self.assertEqual(abi["centroidInputRegisters"], [6, 7])
        self.assertEqual(abi["declaredMrtRegisters"], [0, 2, 3, 4, 5])
        self.assertTrue(abi["carrier"]["signatureClosure"]["pass"])
        self.assertTrue(abi["registerAndSignatureParityAdmission"])

    def test_warp_cases_cover_spatial_inputs_and_all_declared_mrts(self) -> None:
        warp = self.generated["warpComparison"]
        self.assertEqual(warp["caseCount"], 13)
        self.assertGreaterEqual(warp["spatialPatternCaseCount"], 4)
        self.assertLessEqual(
            warp["maximumAbsoluteError"], translation.FLOAT_TOLERANCE
        )
        self.assertFalse(warp["sourceExactSamplerPolicyAdmission"])
        for row in warp["cases"]:
            self.assertTrue(row["comparisonAdmission"])
            self.assertEqual(row["rawDxbcMrt"][1], [translation.SENTINEL] * 4)
            self.assertEqual(
                row["translatedHlslMrt"][1], [translation.SENTINEL] * 4
            )
            self.assertEqual(
                sorted(row["targetMaximumAbsoluteError"]),
                ["rt0", "rt2", "rt3", "rt4", "rt5"],
            )
        sensitivity_targets = {
            row["sensitivityTargetRegister"]
            for row in warp["cases"]
            if row["expectedRelation"] == "SENSITIVE"
        }
        self.assertEqual(sensitivity_targets, {0, 2, 3})

    def test_tracked_receipt_is_current_and_keeps_admission_closed(self) -> None:
        tracked = translation.read_json(translation.DEFAULT_OUTPUT)
        translation.validate_receipt(tracked)
        self.assertEqual(tracked, self.generated)
        decision = tracked["decision"]
        self.assertTrue(decision["rawDxbcNumericOracle"])
        self.assertTrue(decision["translatedHlslFixedFixtureEquationParity"])
        self.assertFalse(decision["rawDxbcProductExecution"])
        self.assertFalse(decision["translatedHlslRuntimeAdmission"])
        self.assertFalse(decision["translatedHlslAuthoringCanaryAdmission"])
        self.assertFalse(decision["productAdmission"])
        self.assertFalse(decision["visualAdmission"])

    def test_validation_rejects_a_forged_product_admission(self) -> None:
        forged = copy.deepcopy(self.generated)
        forged["decision"]["productAdmission"] = True
        translation.seal(forged)
        with self.assertRaisesRegex(ValueError, "overclaims productAdmission"):
            translation.validate_receipt(forged)

    def test_receipt_is_json_round_trip_stable(self) -> None:
        encoded = json.dumps(
            self.generated,
            ensure_ascii=False,
            sort_keys=True,
            separators=(",", ":"),
            allow_nan=False,
        )
        self.assertEqual(json.loads(encoded), self.generated)


if __name__ == "__main__":
    unittest.main()
