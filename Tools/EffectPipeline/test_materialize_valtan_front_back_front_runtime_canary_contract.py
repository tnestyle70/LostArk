#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import materialize_valtan_front_back_front_runtime_canary_contract as canary


class ValtanFrontBackFrontRuntimeCanaryContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.generated = canary.build_receipt()

    @staticmethod
    def write_json(path: Path, value: object) -> None:
        path.write_text(
            json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n",
            encoding="utf-8",
            newline="\n",
        )

    def test_tracked_receipt_is_current(self) -> None:
        tracked = canary.read_json(canary.DEFAULT_OUTPUT)
        canary.validate_receipt(tracked)
        self.assertEqual(tracked, self.generated)
        self.assertEqual(tracked["subject"]["effectAssetId"], canary.EFFECT_ASSET_ID)
        self.assertEqual(tracked["subject"]["familyCount"], 3)
        self.assertEqual(tracked["subject"]["runtimeElementCount"], 9)

    def test_exact_authored_element_sets_are_sealed(self) -> None:
        actual = {
            family["targetId"]: [
                occurrence["runtimeElementId"]
                for occurrence in family["authoredOccurrences"]
            ]
            for family in self.generated["families"]
        }
        self.assertEqual(actual, canary.EXPECTED_RUNTIME_IDS)
        flattened = [element_id for rows in actual.values() for element_id in rows]
        self.assertEqual(len(flattened), 9)
        self.assertEqual(len(set(flattened)), 9)

    def test_three_exact_pixel_shader_identities_are_sealed(self) -> None:
        actual = {
            family["targetId"]: family["exactPixelShaderSha256"]
            for family in self.generated["families"]
        }
        self.assertEqual(actual, canary.EXPECTED_PIXEL_SHADERS)
        for family in self.generated["families"]:
            self.assertEqual(family["evidence"]["shaderMap"], "EXACT_MATERIAL_SHADER_MAP")
            self.assertEqual(family["evidence"]["nativeShaderObjectBinding"], "EXACT_NATIVE_SHADER_OBJECT_BINDING")
            self.assertEqual(family["evidence"]["pixelEquation"], "EXACT_PACKED_DXBC")
            self.assertEqual(family["evidence"]["hlslTranslation"], "TRANSLATED")

    def test_tool_only_fail_closed_rt0_adapter_runtime_is_not_product_admitted(self) -> None:
        self.assertEqual(self.generated["activation"], canary.EXPECTED_ACTIVATION)
        self.assertEqual(self.generated["carrierAdapter"], canary.EXPECTED_CARRIER_ADAPTER)
        self.assertEqual(self.generated["carrierAdapter"]["pixelOutputRegisters"], [0])
        admission = self.generated["admission"]
        self.assertTrue(admission["toolCanaryContract"])
        self.assertTrue(admission["rendererRuntimeAdmission"])
        self.assertFalse(admission["defaultRuntime"])
        self.assertFalse(admission["samplerExact"])
        self.assertFalse(admission["engineSceneCbExact"])
        self.assertFalse(admission["actualVfPass"])
        self.assertFalse(admission["product"])
        self.assertFalse(admission["visual"])
        self.assertEqual(
            admission["contractStatus"],
            "TOOL_RENDERER_RUNTIME_ADMITTED_BOUNDED_RT0",
        )

    def test_bounded_adapter_does_not_claim_source_mrt_or_vf_pass(self) -> None:
        adapter = self.generated["carrierAdapter"]
        self.assertFalse(adapter["rawSourceVertexShaderExecution"])
        self.assertFalse(adapter["rawSourceMrtExecution"])
        self.assertFalse(adapter["sourceVertexFactoryPassAdmission"])
        self.assertFalse(adapter["engineSceneCbExact"])
        for family in self.generated["families"]:
            self.assertEqual(family["boundedAdapterPixelOutputRegisters"], [0])
            self.assertTrue(family["admission"]["rendererRuntime"])
            self.assertFalse(family["admission"]["samplerExact"])
            self.assertFalse(family["admission"]["engineSceneCbExact"])
            self.assertFalse(family["admission"]["actualVfPass"])
            self.assertFalse(family["admission"]["product"])
            self.assertFalse(family["admission"]["visual"])

    def test_authored_identity_projection_preserves_tuning_boundary(self) -> None:
        authored_input = self.generated["inputs"]["authoredIdentity"]
        self.assertEqual(authored_input["effectAssetId"], canary.EFFECT_ASSET_ID)
        self.assertEqual(authored_input["selectedOccurrenceCount"], 9)
        self.assertIn("tuning-only fields", authored_input["role"])
        for family in self.generated["families"]:
            for occurrence in family["authoredOccurrences"]:
                self.assertFalse(occurrence["sourceProfileEnabled"])
                self.assertEqual(occurrence["modelPreScale"], 0.01)
                self.assertEqual(
                    occurrence["sourceMaterialPath"], family["sourceMaterialPath"]
                )

    def test_runtime_implementation_file_identities_are_sealed(self) -> None:
        targets = canary.read_json(canary.DEFAULT_TARGETS)
        expected_targets = targets["runtimeImplementationFiles"]
        actual_inputs = self.generated["inputs"]["runtimeImplementation"]
        self.assertEqual(
            [row["path"] for row in expected_targets],
            list(canary.EXPECTED_IMPLEMENTATION_FILES),
        )
        self.assertEqual(
            [row["path"] for row in actual_inputs],
            list(canary.EXPECTED_IMPLEMENTATION_FILES),
        )
        self.assertEqual(len(actual_inputs), 12)
        for expected, actual in zip(expected_targets, actual_inputs, strict=True):
            self.assertEqual(actual["path"], expected["path"])
            self.assertEqual(actual["role"], expected["role"])
            self.assertEqual(actual["sha256"], expected["sha256"])
            self.assertGreater(actual["byteSize"], 0)

    def test_runtime_regression_seams_are_sealed_and_truthful(self) -> None:
        self.assertEqual(
            self.generated["runtimeRegressionSeams"],
            canary.EXPECTED_RUNTIME_REGRESSION_SEAMS,
        )
        self.assertTrue(all(self.generated["runtimeRegressionSeams"].values()))
        self.assertFalse(self.generated["carrierAdapter"]["engineSceneCbExact"])
        self.assertFalse(self.generated["admission"]["engineSceneCbExact"])

    def test_runtime_regression_seam_validator_rejects_critical_mutations(self) -> None:
        runtime_header_path = canary.REPOSITORY_ROOT / "Client/Public/Effect_ValtanTranslatedCanaryRuntime.h"
        runtime_path = canary.REPOSITORY_ROOT / "Client/Private/Effect_ValtanTranslatedCanaryRuntime.cpp"
        renderer_path = canary.REPOSITORY_ROOT / "Client/Private/Effect_DocumentRenderer.cpp"
        tool_path = canary.REPOSITORY_ROOT / "Client/Private/Effect_Tool.cpp"
        runtime_header = runtime_header_path.read_text(encoding="utf-8-sig")
        runtime = runtime_path.read_text(encoding="utf-8-sig")
        renderer = renderer_path.read_text(encoding="utf-8-sig")
        tool = tool_path.read_text(encoding="utf-8-sig")
        mutations = (
            (
                runtime_header.replace(
                    "static constexpr bool_t ENGINE_SCENE_CB_EXACT = false;",
                    "static constexpr bool_t ENGINE_SCENE_CB_EXACT = true;",
                ),
                runtime,
                renderer,
                tool,
                "compile-time admission boundary",
            ),
            (
                runtime_header,
                runtime.replace(
                    "CB2[3] = { 0.f, 0.f, 0.f, 1.f };",
                    "CB2[3] = { 0.f, 0.f, 0.f, 0.f };",
                ),
                renderer,
                tool,
                "neutral cb2",
            ),
            (
                runtime_header,
                runtime.replace("pRect->Bind_Resources()", "S_OK"),
                renderer,
                tool,
                "resources are not bound",
            ),
            (
                runtime_header,
                runtime,
                renderer.replace(
                    "!m_bAuthoringValtanTranslatedCanaryEnabled &&\n\t\t\tElement.Material.Execution.bFailClosed",
                    "m_bAuthoringValtanTranslatedCanaryEnabled &&\n\t\t\tElement.Material.Execution.bFailClosed",
                ),
                tool,
                "ordinary fail-closed",
            ),
            (
                runtime_header,
                runtime,
                renderer.replace("if (iCount > 1u)", "if (1u != iCount)"),
                tool,
                "non-empty unique exact subset",
            ),
            (
                runtime_header,
                runtime,
                renderer,
                tool.replace("if (nullptr == pFound ||", "if (false ||"),
                "full-nine Authored precheck",
            ),
            (
                runtime_header,
                runtime,
                renderer,
                tool.replace(
                    "Build_PreviewDocument(Document, bValtanTranslatedCanaryStage)",
                    "Build_PreviewDocument(Document, true)",
                ),
                "exact Authored eligibility",
            ),
            (
                runtime_header,
                runtime,
                renderer,
                tool.replace("bounded engine scene CBs", "exact engine scene CBs"),
                "scene-CB boundary truthfully",
            ),
        )
        for runtime_header_mutated, runtime_mutated, renderer_mutated, tool_mutated, expected in mutations:
            with self.subTest(expected=expected):
                with self.assertRaisesRegex(ValueError, expected):
                    canary.validate_runtime_regression_seams(
                        runtime_header_mutated,
                        runtime_mutated,
                        renderer_mutated,
                        tool_mutated,
                    )

    def test_validation_rejects_open_activation_and_non_rt0_adapter(self) -> None:
        mutations = (
            ("activation", "defaultOff", False),
            ("activation", "failClosed", False),
            ("activation", "productAdmission", True),
            ("activation", "visualAdmission", True),
            ("activation", "actualVfPassAdmission", True),
            ("carrierAdapter", "pixelOutputRegisters", [0, 1]),
            ("carrierAdapter", "sourceVertexFactoryPassAdmission", True),
        )
        for section, field, value in mutations:
            forged = copy.deepcopy(self.generated)
            forged[section][field] = value
            canary.seal(forged)
            with self.assertRaises(ValueError):
                canary.validate_receipt(forged)
        for field, value in (
            ("familyId", "forged.family"),
            ("sourceMaterialPath", "forged.material"),
        ):
            forged = copy.deepcopy(self.generated)
            forged["families"][0][field] = value
            canary.seal(forged)
            with self.assertRaises(ValueError):
                canary.validate_receipt(forged)
        forged = copy.deepcopy(self.generated)
        forged["families"][0]["authoredOccurrences"][0]["sourceOccurrenceId"] = "forged.occurrence"
        canary.seal(forged)
        with self.assertRaises(ValueError):
            canary.validate_receipt(forged)
        forged = copy.deepcopy(self.generated)
        forged["admission"]["samplerExact"] = True
        canary.seal(forged)
        with self.assertRaises(ValueError):
            canary.validate_receipt(forged)
        forged = copy.deepcopy(self.generated)
        forged["admission"]["engineSceneCbExact"] = True
        canary.seal(forged)
        with self.assertRaises(ValueError):
            canary.validate_receipt(forged)
        forged = copy.deepcopy(self.generated)
        forged["families"][0]["admission"]["samplerExact"] = True
        canary.seal(forged)
        with self.assertRaises(ValueError):
            canary.validate_receipt(forged)
        forged = copy.deepcopy(self.generated)
        forged["inputs"]["runtimeImplementation"][0]["sha256"] = "0" * 64
        canary.seal(forged)
        with self.assertRaisesRegex(ValueError, "implementation receipt is stale"):
            canary.validate_receipt(forged)

    def test_build_rejects_authored_material_identity_mutation(self) -> None:
        authored = canary.read_json(canary.DEFAULT_AUTHORED)
        element = next(
            row
            for row in authored["elements"]
            if row.get("id") == "par_n_rpbf_atk_01_02.em14"
        )
        element["material"]["sourceMaterialPath"] = "forged.material"
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "authored.json"
            self.write_json(path, authored)
            with self.assertRaisesRegex(ValueError, "authored source material"):
                canary.build_receipt(authored_path=path)

    def test_build_rejects_model_prescale_regression(self) -> None:
        authored = canary.read_json(canary.DEFAULT_AUTHORED)
        element = next(
            row
            for row in authored["elements"]
            if row.get("id") == "par_n_rpbf_atk_04_12.em02"
        )
        element["detail"]["mesh"]["modelPreScale"] = 1.0
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "authored.json"
            self.write_json(path, authored)
            with self.assertRaisesRegex(ValueError, "modelPreScale"):
                canary.build_receipt(authored_path=path)

    def test_build_rejects_exact_program_shader_mutation(self) -> None:
        programs = canary.read_json(canary.DEFAULT_EXACT_PROGRAMS)
        program = next(
            row
            for row in programs["programs"]
            if row.get("targetId") == "valtan-front-back-front-crack-translucent"
        )
        program["dxbc"]["sha256"] = "0" * 64
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "programs.json"
            self.write_json(path, programs)
            with self.assertRaisesRegex(ValueError, "exact program DXBC"):
                canary.build_receipt(exact_programs_path=path)

    def test_targets_reject_duplicate_or_missing_runtime_elements(self) -> None:
        targets = canary.read_json(canary.DEFAULT_TARGETS)
        targets["families"][2]["runtimeElementIds"][-1] = targets["families"][2]["runtimeElementIds"][0]
        with self.assertRaisesRegex(ValueError, "runtime element set"):
            canary.validate_targets_document(targets)

    def test_build_rejects_runtime_implementation_hash_drift(self) -> None:
        targets = canary.read_json(canary.DEFAULT_TARGETS)
        targets["runtimeImplementationFiles"][1]["sha256"] = "0" * 64
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "targets.json"
            self.write_json(path, targets)
            with self.assertRaisesRegex(ValueError, "implementation hash drifted"):
                canary.build_receipt(targets_path=path)

    def test_descriptor_rejects_missing_runtime_implementation(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            missing = Path(temporary) / "missing-runtime-implementation.cpp"
            with self.assertRaisesRegex(ValueError, "required input is missing"):
                canary.descriptor(missing, "missing implementation canary")

    def test_build_is_deterministic_and_json_round_trip_is_stable(self) -> None:
        rebuilt = canary.build_receipt()
        self.assertEqual(rebuilt, self.generated)
        encoded = json.dumps(
            rebuilt,
            ensure_ascii=False,
            sort_keys=True,
            separators=(",", ":"),
            allow_nan=False,
        )
        self.assertEqual(json.loads(encoded), rebuilt)


if __name__ == "__main__":
    unittest.main()
