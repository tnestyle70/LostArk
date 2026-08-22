#!/usr/bin/env python3

from __future__ import annotations

import unittest

import replay_ue3_glasshole02_runtime_rt0 as runtime


class Glasshole02RuntimeRt0Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.report = runtime.build_report()

    def test_particle_vertex_bridge_closes_structure_not_raw_spatial_parity(self) -> None:
        vertex = self.report["runtimeShader"]["vertex"]
        self.assertTrue(vertex["vtxEffectParticleInputSignatureClosed"])
        self.assertTrue(vertex["offsetCenterInstanceWorldAdapterContract"])
        self.assertTrue(
            vertex["psaRectangleIndependentWorldXYScaleAdapterContract"]
        )
        self.assertTrue(vertex["dynamicParameterForwardedWithoutRemap"])
        self.assertFalse(vertex["rawSourceVertexSpatialParity"])
        outputs = {
            (row["semanticName"].upper(), row["semanticIndex"])
            for row in vertex["outputSignature"]
        }
        self.assertEqual(
            outputs,
            {
                ("TEXCOORD", 10),
                ("TEXCOORD", 11),
                ("TEXCOORD", 0),
                ("TEXCOORD", 1),
                ("TEXCOORD", 2),
                ("TEXCOORD", 4),
                ("TEXCOORD", 6),
                ("TEXCOORD", 5),
                ("SV_POSITION", 0),
            },
        )

    def test_runtime_pixel_shader_is_rt0_only_and_keeps_source_resources(self) -> None:
        shader = self.report["runtimeShader"]
        self.assertGreater(shader["fx11Debug"]["compiledByteSize"], 0)
        self.assertGreater(shader["fx11Release"]["compiledByteSize"], 0)
        pixel = shader["pixel"]
        self.assertEqual(pixel["instructionCount"], 173)
        self.assertEqual(
            pixel["constantBufferFloat4Counts"], {"0": 22, "1": 9, "2": 2}
        )
        self.assertEqual(pixel["textureRegisters"], list(range(8)))
        self.assertEqual(pixel["samplerRegisters"], list(range(8)))
        self.assertTrue(pixel["rt0Only"])
        self.assertEqual(len(pixel["outputSignature"]), 1)
        self.assertTrue(
            shader["carrier"]["rawPixelSignatureClosure"]["pass"]
        )
        self.assertTrue(
            shader["carrier"]["runtimePixelSignatureClosure"]["pass"]
        )

    def test_exact_ast_cb0_changes_with_the_same_runtime_local_time(self) -> None:
        contract = self.report["runtimeUniformExpressionContract"]
        self.assertEqual(
            contract["localTimeIdentifier"],
            "g_Glasshole02LocalTimeSeconds",
        )
        self.assertEqual(contract["materialConstantIdentifier"], "g_Ue3Glasshole02CB0")
        self.assertEqual(contract["evaluationTimesSeconds"], [0.0, 0.25, 0.6])
        self.assertEqual(
            contract["astTimeSensitiveCb0Slots"],
            list(runtime.EXPECTED_AST_TIME_SENSITIVE_CB0_SLOTS),
        )
        self.assertEqual(
            contract["numericallyChangedCb0Slots"],
            list(runtime.EXPECTED_NUMERICALLY_CHANGED_CB0_SLOTS),
        )
        self.assertTrue(contract["staticTimeZeroCb0Rejected"])
        self.assertEqual(contract["signedPeriodicNegativeProbe"], -0.25)

    def test_target_depth_projection_matches_engine_view_depth_lane(self) -> None:
        depth = self.report["targetDepthContract"]
        self.assertEqual(depth["runtimeRenderTarget"], "Target_Depth")
        self.assertEqual(
            depth["shaderResourceIdentifier"],
            "g_Ue3Glasshole02SceneDepth",
        )
        self.assertLessEqual(
            depth["maximumAbsoluteError"],
            runtime.DEPTH_EQUIVALENCE_TOLERANCE,
        )
        self.assertTrue(depth["pass"])

    def test_raw_oracle_and_runtime_wrapper_match_at_all_time_cases(self) -> None:
        comparison = self.report["warpComparison"]
        self.assertEqual(comparison["caseCount"], 3)
        self.assertEqual(
            [row["localTimeSeconds"] for row in comparison["cases"]],
            [0.0, 0.25, 0.6],
        )
        self.assertLessEqual(
            comparison["maximumAbsoluteError"], runtime.FLOAT_TOLERANCE
        )
        for row in comparison["cases"]:
            self.assertTrue(row["pass"])
            self.assertTrue(row["runtimeUndeclaredTargetsRemainSentinel"])

    def test_focused_gate_does_not_overclaim_product_or_visual(self) -> None:
        decision = self.report["decision"]
        self.assertTrue(decision["runtimeRt0ShaderCompiled"])
        self.assertTrue(decision["runtimeRt0RawOracleParity"])
        self.assertTrue(decision["runtimeVertexBridgeStructureClosed"])
        self.assertFalse(decision["rawSourceVertexSpatialParity"])
        self.assertTrue(decision["runtimeTimeVaryingCb0ContractClosed"])
        self.assertFalse(decision["runtimeRendererBindingAdmission"])
        self.assertFalse(decision["productAdmission"])
        self.assertFalse(decision["visualAdmission"])


if __name__ == "__main__":
    unittest.main()
