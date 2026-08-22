#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import re
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import materialize_ue3_glasshole02_runtime_canary_contract as canary


class Glasshole02RuntimeCanaryContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.generated = canary.build_receipt()

    def test_tracked_receipt_is_current_and_exactly_one_occurrence(self) -> None:
        tracked = canary.read_json(canary.DEFAULT_OUTPUT)
        canary.validate_receipt(tracked)
        self.assertEqual(tracked, self.generated)
        self.assertEqual(tracked["subject"]["occurrenceCount"], 1)
        self.assertEqual(tracked["subject"]["occurrenceId"], canary.OCCURRENCE_ID)

    def test_seven_dds_plus_scene_depth_and_eight_samplers_are_closed(self) -> None:
        resources = self.generated["resourceBindings"]
        self.assertEqual(resources["materialDdsSlotCount"], 7)
        self.assertEqual(resources["requiredSourceTextureMask"], 0x7F)
        self.assertEqual(resources["declaredTextureRegisters"], list(range(8)))
        self.assertEqual(resources["declaredSamplerRegisters"], list(range(8)))
        self.assertEqual(resources["engineSceneDepth"]["textureRegister"], "t2")
        self.assertEqual(resources["engineSceneDepth"]["samplerRegister"], "s0")
        self.assertEqual(resources["previewSamplerCandidateCount"], 8)
        self.assertTrue(all(not row["sourceExact"] for row in resources["previewSamplerCandidates"]))

    def test_sampler_provenance_keeps_unresolved_axes_open(self) -> None:
        resources = self.generated["resourceBindings"]
        self.assertEqual(resources["sourceExactColorSpaceBindingCount"], 7)
        self.assertEqual(resources["sourceExactFilterSelectorBindingCount"], 7)
        self.assertEqual(resources["sourceExactHardwareFilterBindingCount"], 0)
        self.assertFalse(resources["fullSourceExactSampler"])
        self.assertTrue(any(not row["sourceExactAddressU"] for row in resources["materialDdsSlots"]))
        self.assertTrue(all(not row["sourceExactHardwareFilter"] for row in resources["materialDdsSlots"]))

    def test_rt0_only_shader_and_rt1_sentinel_are_required(self) -> None:
        self.assertEqual(self.generated["runtimeShader"]["pixelOutputRegisters"], [0])
        rt = self.generated["rtContract"]
        self.assertEqual(rt["rt1BoundSentinel"], -99.0)
        self.assertTrue(rt["offlineRawAndTranslationLeaveRt1SentinelUntouched"])
        self.assertTrue(rt["runtimePixelShaderCannotWriteRt1"])
        self.assertTrue(rt["rt1SentinelRequiredAfterCanaryDraw"])

    def test_exact_uniform_ast_proves_cb0_changes_with_local_time(self) -> None:
        time_cb0 = self.generated["timeVaryingCb0"]
        self.assertEqual(time_cb0["evaluationTimesSeconds"], [0.0, 0.25, 0.6])
        self.assertEqual(time_cb0["astTimeDependentCb0Slots"], [2, 3, 7, 8, 10, 11, 13, 15, 17, 19, 20, 21])
        self.assertEqual(time_cb0["valueChangingCb0SlotsAtSealedTimes"], [2, 3, 10, 11, 13, 17, 19, 20, 21])
        hashes = [row["nativeCb0MaterialRowsSemanticSha256"] for row in time_cb0["evaluations"]]
        self.assertEqual(len(set(hashes)), 3)
        self.assertFalse(time_cb0["staticCb0PacketAllowed"])
        cb0 = self.generated["runtimeCb0Contract"]
        self.assertEqual(cb0["rendererOwnedC0"]["slot"], 0)
        self.assertFalse(cb0["rendererOwnedC0"]["sourceExactMaterialAstParity"])
        self.assertEqual(cb0["exactMaterialAstParitySlots"], list(range(1, 22)))
        self.assertFalse(cb0["fullCb0SourceExact"])
        self.assertEqual(cb0["cppZeroTailSlots"], [3, 10, 11, 13])

    def test_activation_is_default_off_product_off_and_fail_closed(self) -> None:
        activation = self.generated["activation"]
        self.assertFalse(activation["defaultEnabled"])
        self.assertFalse(activation["productEnabled"])
        self.assertTrue(activation["failClosed"])
        self.assertFalse(activation["fallbackToApproximationOnCanaryFailure"])
        self.assertEqual(
            self.generated["admission"]["rendererContractStatus"],
            "CONTRACT_STAGED_AWAITING_CPP_RUNTIME_HARNESS",
        )
        self.assertFalse(self.generated["admission"]["rendererRuntimeAdmission"])

    def test_validation_rejects_forged_product_and_sampler_admission(self) -> None:
        for path in (("activation", "productEnabled"), ("admission", "sourceExactFullSampler"), ("resourceBindings", "fullSourceExactSampler")):
            forged = copy.deepcopy(self.generated)
            forged[path[0]][path[1]] = True
            canary.seal(forged)
            with self.assertRaises(ValueError):
                canary.validate_receipt(forged)

    def test_validation_rejects_mask_or_rt1_contract_mutation(self) -> None:
        for section, field, value in (("resourceBindings", "requiredSourceTextureMask", 0x7), ("runtimeShader", "pixelOutputRegisters", [0, 1]), ("rtContract", "rt1SentinelRequiredAfterCanaryDraw", False)):
            forged = copy.deepcopy(self.generated)
            forged[section][field] = value
            canary.seal(forged)
            with self.assertRaises(ValueError):
                canary.validate_receipt(forged)

    def test_validation_rejects_static_cb0_packet(self) -> None:
        forged = copy.deepcopy(self.generated)
        first_hash = forged["timeVaryingCb0"]["evaluations"][0]["nativeCb0MaterialRowsSemanticSha256"]
        for row in forged["timeVaryingCb0"]["evaluations"]:
            row["nativeCb0MaterialRowsSemanticSha256"] = first_hash
        canary.seal(forged)
        with self.assertRaisesRegex(ValueError, "static CB0"):
            canary.validate_receipt(forged)

    def test_renderer_local_time_bind_cannot_be_satisfied_by_a_comment(self) -> None:
        renderer = canary.DEFAULT_RENDERER.read_text(encoding="utf-8-sig")
        header = canary.DEFAULT_RENDERER_HEADER.read_text(encoding="utf-8-sig")
        mutated, count = re.subn(
            r'("g_Glasshole02LocalTimeSeconds"\s*,\s*)&fLocalTimeSeconds',
            r"\1nullptr /* fake: Bind_RawValue(\"g_Glasshole02LocalTimeSeconds\", &fLocalTimeSeconds, sizeof(fLocalTimeSeconds)) */",
            renderer,
            count=1,
        )
        self.assertEqual(count, 1)
        with self.assertRaisesRegex(ValueError, "Bind_RawValue"):
            canary.validate_renderer_runtime_seams(mutated, header)

    def test_renderer_structural_join_rejects_pin_or_depth_mutation(self) -> None:
        renderer = canary.DEFAULT_RENDERER.read_text(encoding="utf-8-sig")
        slots = self.generated["resourceBindings"]["materialDdsSlots"]
        expected = {
            row["gameTimeSeconds"]: row["runtimeCb0Rows"]
            for row in self.generated["runtimeCb0Contract"]["cppSelfTestSemanticHashes"]
        }
        canary.validate_renderer_source_join(renderer, slots, expected)
        mutations = (
            renderer.replace(slots[0]["runtimeDdsSha256"], "0" * 64, 1),
            renderer.replace(
                'TEXT("Target_Depth"), m_pGlasshole02TranslatedCanaryShader',
                'TEXT("Broken_Depth"), m_pGlasshole02TranslatedCanaryShader',
                1,
            ),
            renderer.replace('S("alpha_tile_x", 1.f)', 'S("wrong_lane", 1.f)', 1),
        )
        for mutated in mutations:
            with self.assertRaises(ValueError):
                canary.validate_renderer_source_join(mutated, slots, expected)

    def test_named_scalar_and_vector_runtime_mapping_is_sealed(self) -> None:
        mapping = self.generated["runtimeInputMapping"]
        self.assertEqual(mapping["scalarParameterCount"], 32)
        self.assertEqual(
            [[lane["parameterName"] for lane in row["lanes"]] for row in mapping["parameterRows"]],
            canary.GLASSHOLE_SCALAR_LANES,
        )
        self.assertEqual(
            [(row["runtimeField"], row["parameterName"]) for row in mapping["vectors"]],
            [("vSourceVector0", "aura_color"), ("vSourceVector1", "in_hole_color")],
        )

    def test_renderer_structural_join_rejects_cb0_tail_mutation(self) -> None:
        renderer = canary.DEFAULT_RENDERER.read_text(encoding="utf-8-sig")
        slots = self.generated["resourceBindings"]["materialDdsSlots"]
        expected = {
            row["gameTimeSeconds"]: row["runtimeCb0Rows"]
            for row in self.generated["runtimeCb0Contract"]["cppSelfTestSemanticHashes"]
        }
        mutated = renderer.replace(
            "{ 0.f, 0.03125f, 0.f, 0.f }",
            "{ 0.f, 0.03125f, 0.03125f, 0.03125f }",
            1,
        )
        self.assertNotEqual(mutated, renderer)
        with self.assertRaisesRegex(ValueError, "full CB0 self-test"):
            canary.validate_renderer_source_join(mutated, slots, expected)

    def test_renderer_structural_join_rejects_clear_reset_mutation(self) -> None:
        renderer = canary.DEFAULT_RENDERER.read_text(encoding="utf-8-sig")
        slots = self.generated["resourceBindings"]["materialDdsSlots"]
        expected = {
            row["gameTimeSeconds"]: row["runtimeCb0Rows"]
            for row in self.generated["runtimeCb0Contract"]["cppSelfTestSemanticHashes"]
        }
        anchor = "m_bAuthoringGlasshole02TranslatedCanaryEnabled = false;"
        offset = renderer.rfind(anchor)
        self.assertGreaterEqual(offset, 0)
        mutated = renderer[:offset] + "/* removed canary reset */" + renderer[offset + len(anchor):]
        with self.assertRaisesRegex(ValueError, "Clear"):
            canary.validate_renderer_source_join(mutated, slots, expected)

    def test_tool_frontend_structural_contract_rejects_gate_and_rollback_mutations(self) -> None:
        tool = canary.DEFAULT_EFFECT_TOOL.read_text(encoding="utf-8-sig")
        header = canary.DEFAULT_EFFECT_TOOL_HEADER.read_text(encoding="utf-8-sig")
        canary.validate_tool_frontend_seams(tool, header)

        mutations = (
            (
                tool.replace(
                    'ImGui::Checkbox("Enable Translated Glasshole02 Canary"',
                    'ImGui::Checkbox("Enable Unsealed Canary"',
                    1,
                ),
                header,
            ),
            (
                tool.replace(
                    "!bAllowReadOnlySourceProjection && !Has_ProductCuePreview()",
                    "bAllowReadOnlySourceProjection && !Has_ProductCuePreview()",
                    1,
                ),
                header,
            ),
            (
                tool.replace(
                    "else if (!bGlasshole02TranslatedCanaryStage &&",
                    "else if (bGlasshole02TranslatedCanaryStage &&",
                    1,
                ),
                header,
            ),
            (
                tool.replace(
                    "const std::string CanaryFailure = m_strPreviewStatus;\n"
                    "\t\tm_bGlasshole02TranslatedCanaryEnabled = false;",
                    "const std::string CanaryFailure = m_strPreviewStatus;",
                    1,
                ),
                header,
            ),
            (
                tool.replace(
                    "Reset_Glasshole02TranslatedCanarySelection(strReason);",
                    "Reset_Glasshole02TranslatedCanarySelection(\"mutated\");",
                    1,
                ),
                header,
            ),
            (
                tool,
                header.replace(
                    "m_bGlasshole02TranslatedCanaryEnabled = false;",
                    "m_bGlasshole02TranslatedCanaryEnabled = true;",
                    1,
                ),
            ),
        )
        for mutated_tool, mutated_header in mutations:
            self.assertTrue(mutated_tool != tool or mutated_header != header)
            with self.assertRaises(ValueError):
                canary.validate_tool_frontend_seams(
                    mutated_tool, mutated_header
                )

    def test_tool_frontend_receipt_stays_contract_only(self) -> None:
        frontend = self.generated["toolFrontendContract"]
        self.assertTrue(frontend["separateFlagDefaultOff"])
        self.assertTrue(frontend["separateCheckbox"])
        self.assertTrue(frontend["rawCookedCanaryMutuallyExclusive"])
        self.assertTrue(frontend["authoredExactOccurrenceOnly"])
        self.assertTrue(frontend["productAndReadOnlyOffBeforeStage"])
        self.assertTrue(frontend["translatedExecutionEnabledBeforeDocumentStage"])
        self.assertTrue(frontend["initialStageFailureFlagOffReleaseOrdinaryRestage"])
        self.assertTrue(frontend["productReadOnlyStageDisablesTranslatedExecution"])
        self.assertTrue(frontend["documentChangeReset"])
        self.assertFalse(frontend["frontendRuntimeAdmission"])
        self.assertFalse(self.generated["admission"]["rendererRuntimeAdmission"])

    def test_json_round_trip_is_stable(self) -> None:
        encoded = json.dumps(self.generated, ensure_ascii=False, sort_keys=True, separators=(",", ":"), allow_nan=False)
        self.assertEqual(json.loads(encoded), self.generated)


if __name__ == "__main__":
    unittest.main()
