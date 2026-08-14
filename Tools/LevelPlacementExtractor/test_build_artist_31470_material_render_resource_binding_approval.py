#!/usr/bin/env python3
from __future__ import annotations

import copy
import hashlib
import json
import math
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock


SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

import artist_31470_material_render_resource_binding_approval as approval
import build_artist_31470_material_render_resource_binding_approval as binding


def reseal_row(row: dict) -> None:
    row.pop("rowSha256", None)
    row["rowSha256"] = binding.canonical_sha256(row)


def reseal_receipt(receipt: dict) -> None:
    receipt.pop("receiptSha256", None)
    receipt["receiptSha256"] = binding.canonical_sha256(receipt)


class MaterialRenderResourceBindingApprovalTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.temporary = tempfile.TemporaryDirectory()
        root = Path(cls.temporary.name)
        cls.program_path = root / "program.json"
        cls.output_path = root / "approval.json"
        cls.program = binding.program_module.build_program()
        cls.program_path.write_bytes(
            binding.program_module.output_bytes(cls.program)
        )
        cls.receipt = binding.build_receipt(cls.program)
        cls.output_path.write_bytes(binding.serialized_receipt(cls.receipt))
        cls.inputs = {row["fieldId"]: row for row in cls.program["materialInputs"]}
        cls.texture_bindings = {
            row["bindingId"]: row for row in cls.program["materialTextureBindings"]
        }
        cls.policies = {
            row["policyRowId"]: row for row in cls.program["materialPolicyRows"]
        }

    @classmethod
    def tearDownClass(cls) -> None:
        cls.temporary.cleanup()

    def validate(self, receipt: dict, *, require_approval: bool = True) -> None:
        binding.validate_receipt(
            receipt,
            self.program,
            program_path=self.program_path,
            _program_already_validated=True,
            require_approval=require_approval,
        )

    def assert_invalid(self, receipt: dict, *, pure: bool = False) -> None:
        with self.assertRaises((ValueError, KeyError, TypeError)):
            self.validate(receipt, require_approval=not pure)

    def test_checked_receipt_is_exact_and_deterministic(self) -> None:
        raw = self.output_path.read_bytes()
        self.assertEqual(len(raw), 378_236)
        self.assertEqual(
            hashlib.sha256(raw).hexdigest(),
            "a73a4e36e5860dc37961a236270c4ca3245025711f05e64a56503cd839b6cd74",
        )
        self.assertFalse(raw.startswith(b"\xef\xbb\xbf"))
        self.assertNotIn(b"\r", raw)
        self.assertEqual(
            self.receipt["receiptSha256"],
            "9ca692c688c3987746ab811e4f2504d7186b2905efa7fa8b3446e8c4bf053ac6",
        )
        self.assertEqual(
            approval.decision_projection_sha256(self.receipt),
            "1b0e8e224b7b1b98b1606f123423ff1bca287271d6f24deda3a16c880c89994d",
        )
        self.validate(self.receipt, require_approval=False)
        rebuilt = binding.build_receipt(
            self.program, _program_already_validated=True
        )
        self.assertEqual(rebuilt, self.receipt)
        self.assertEqual(
            binding.serialized_receipt(rebuilt), self.output_path.read_bytes()
        )

    def test_denominators_and_all_admission_gates(self) -> None:
        summary = self.receipt["summary"]
        self.assertEqual(summary["recipeTextureBindingCount"], 27)
        self.assertEqual(summary["rendererSlotBindingCount"], 57)
        self.assertEqual(summary["ambiguousRendererDecisionCount"], 3)
        self.assertEqual(summary["renderStateDescriptorCount"], 46)
        self.assertEqual(summary["blendDescriptorCount"], 27)
        self.assertEqual(summary["twoSidedRasterDescriptorCount"], 18)
        self.assertEqual(summary["disableDepthDescriptorCount"], 1)
        for section in (
            "neutralProviders", "recipeTextureBindings", "rendererSlotBindings",
            "renderStateDescriptors",
        ):
            for row in self.receipt[section]:
                self.assertFalse(row["sourceExact"])
                self.assertTrue(row["requiresAutomatedWARPProbe"])
                self.assertTrue(row["requiresManualEyeValidation"])
                self.assertFalse(row["runtimeExecutionAdmission"])
                self.assertFalse(row["product"])
        self.assertFalse(self.receipt["admission"]["sourceExact"])
        self.assertFalse(self.receipt["admission"]["runtimeExecutionAdmission"])
        self.assertFalse(self.receipt["admission"]["product"])

    def test_three_ambiguities_are_exact_independent_choices(self) -> None:
        rows = {
            row["textureResourceId"]: row
            for row in self.receipt["rendererSlotBindings"]
            if row["candidateCount"] == 2
        }
        self.assertEqual(set(rows), set(approval.APPROVED_AMBIGUOUS_RENDERER_BINDINGS))
        for resource_id, expected_field_id in approval.APPROVED_AMBIGUOUS_RENDERER_BINDINGS.items():
            row = rows[resource_id]
            self.assertEqual(row["selectedMaterialInputFieldId"], expected_field_id)
            self.assertEqual(row["decisionBasis"], "EXPLICIT_INDEPENDENT_AMBIGUITY_APPROVAL")

    def test_each_ambiguous_reverse_choice_resealed_is_rejected(self) -> None:
        for source in self.receipt["rendererSlotBindings"]:
            if source["candidateCount"] != 2:
                continue
            forged = copy.deepcopy(self.receipt)
            row = forged["rendererSlotBindings"][source["order"]]
            alternate = next(
                candidate for candidate in row["candidates"]
                if candidate["materialInputFieldId"] != row["selectedMaterialInputFieldId"]
            )
            texture_binding = self.texture_bindings[alternate["textureBindingId"]]
            policy = self.policies[texture_binding["samplerPolicyRowId"]]
            row["selectedMaterialInputFieldId"] = alternate["materialInputFieldId"]
            row["selectedMaterialInputRowSha256"] = alternate["materialInputRowSha256"]
            row["selectedNormalizedParameterName"] = alternate["normalizedParameterName"]
            row["selectedTextureBindingId"] = alternate["textureBindingId"]
            row["selectedTextureBindingRowSha256"] = alternate["textureBindingRowSha256"]
            row["selectedSamplerPolicyRowId"] = texture_binding["samplerPolicyRowId"]
            row["selectedSamplerPolicyRowSha256"] = policy["rowSha256"]
            row["decisionBasis"] = "EXPLICIT_INDEPENDENT_AMBIGUITY_APPROVAL"
            reseal_row(row)
            reseal_receipt(forged)
            self.assert_invalid(forged, pure=True)

    def test_unique_renderer_choice_coordinated_reseal_is_rejected(self) -> None:
        forged = copy.deepcopy(self.receipt)
        row = next(row for row in forged["rendererSlotBindings"] if row["candidateCount"] == 1)
        row["selectedNormalizedParameterName"] += "_forged"
        reseal_row(row)
        reseal_receipt(forged)
        self.assert_invalid(forged, pure=True)

    def test_all_recipes_have_explicit_texture0_texture1_and_fallback(self) -> None:
        self.assertEqual(len(self.receipt["recipeTextureBindings"]), 27)
        for row in self.receipt["recipeTextureBindings"]:
            self.assertIn(
                row["texture0Provider"]["providerKind"],
                {"MATERIAL_TEXTURE_BINDING", "NEUTRAL_CONSTANT"},
            )
            self.assertIn(
                row["texture1Provider"]["providerKind"],
                {"MATERIAL_TEXTURE_BINDING", "NEUTRAL_CONSTANT"},
            )
            fallback = row["neutralFallbackDecision"]
            self.assertEqual(
                fallback["neutralProviderApplicationPolicy"],
                "ONLY_WHEN_THIS_APPROVAL_EXPLICITLY_SELECTS_NEUTRAL",
            )
            self.assertEqual(
                fallback["materialBindingFailurePolicy"],
                "FAIL_CLOSED_TRANSACTION_ROLLBACK",
            )

    def test_no_texture_recipes_use_semantically_valid_explicit_neutrals(self) -> None:
        rows = [
            row for row in self.receipt["recipeTextureBindings"]
            if not row["candidateTextureBindingIds"]
        ]
        self.assertEqual(len(rows), 3)
        for row in rows:
            self.assertEqual(
                row["texture0Provider"]["neutralProviderId"],
                binding.NEUTRAL_BASE_WHITE,
            )
            if row["distortionOperationEnabled"] and row["distortionStrengthF32"] != 0.0:
                expected = binding.NEUTRAL_DISTORTION_HALF
            elif row["secondTextureOperationEnabled"]:
                expected = binding.NEUTRAL_SECOND_WHITE
            else:
                expected = binding.NEUTRAL_UNUSED_ZERO
            self.assertEqual(row["texture1Provider"]["neutralProviderId"], expected)

    def test_self_default_sprite_and_ribbon_recipes_use_distinct_runtime_resources(self) -> None:
        expected = {
            "material-recipe-aa19ee0d487380ca": (
                "Effect/Artist/Textures/fx_d_noise_003.dds",
                "Effect/Artist/Textures/fx_c_noise_002.dds",
            ),
            "material-recipe-b508403ea55fedbc": (
                "Effect/Artist/Textures/fx_i_atypical_03_ycl.dds",
                "Effect/Artist/Textures/fx_b_atypical_004.dds",
            ),
        }
        rows = {row["recipeId"]: row for row in self.receipt["recipeTextureBindings"]}
        for recipe_id, assets in expected.items():
            row = rows[recipe_id]
            self.assertEqual(
                row["texture0Provider"]["providerKind"],
                "MATERIAL_TEXTURE_BINDING",
            )
            self.assertEqual(
                row["texture1Provider"]["providerKind"],
                "MATERIAL_TEXTURE_BINDING",
            )
            self.assertEqual(row["texture0Provider"]["runtimeAssetId"], assets[0])
            self.assertEqual(row["texture1Provider"]["runtimeAssetId"], assets[1])
            self.assertNotEqual(
                row["texture0Provider"]["runtimeAssetId"].casefold(),
                row["texture1Provider"]["runtimeAssetId"].casefold(),
            )

    def test_neutral_provider_semantics_are_numeric_and_explicit(self) -> None:
        providers = {
            row["neutralProviderId"]: row for row in self.receipt["neutralProviders"]
        }
        self.assertEqual(providers[binding.NEUTRAL_BASE_WHITE]["rgbaF32"], [1.0] * 4)
        self.assertEqual(providers[binding.NEUTRAL_SECOND_WHITE]["secondaryMultiplyFactor"], 1.0)
        half = providers[binding.NEUTRAL_DISTORTION_HALF]
        self.assertEqual(half["rgbaF32"], [0.5, 0.5, 0.5, 1.0])
        self.assertEqual(half["signedDistortionOffset"], 0.0)
        self.assertEqual(half["secondaryMultiplyFactor"], 0.75)

    def test_neutral_provider_coordinated_reseal_is_rejected(self) -> None:
        forged = copy.deepcopy(self.receipt)
        row = next(
            row for row in forged["neutralProviders"]
            if row["neutralProviderId"] == binding.NEUTRAL_DISTORTION_HALF
        )
        row["rgbaF32"][0] = 0.6
        row["signedDistortionOffset"] = 0.2
        reseal_row(row)
        reseal_receipt(forged)
        self.assert_invalid(forged, pure=True)

    def test_recipe_provider_coordinated_reseal_is_rejected(self) -> None:
        forged = copy.deepcopy(self.receipt)
        row = next(
            row for row in forged["recipeTextureBindings"]
            if row["texture0Provider"]["providerKind"] == "MATERIAL_TEXTURE_BINDING"
            and row["texture1Provider"]["providerKind"] == "MATERIAL_TEXTURE_BINDING"
        )
        row["texture0Provider"], row["texture1Provider"] = (
            row["texture1Provider"], row["texture0Provider"]
        )
        reseal_row(row)
        reseal_receipt(forged)
        self.assert_invalid(forged, pure=True)

    def test_blend_descriptors_match_explicit_effect_states(self) -> None:
        rows = [
            row for row in self.receipt["renderStateDescriptors"]
            if row["descriptorKind"] == "D3D11_BLEND_DESC"
        ]
        self.assertEqual(len(rows), 27)
        self.assertEqual(
            {row["implementationStateName"] for row in rows},
            {"BS_EffectOpaque", "BS_EffectAlpha", "BS_EffectAdditive"},
        )
        for row in rows:
            descriptor = row["expectedDescriptor"]
            self.assertTrue(descriptor["IndependentBlendEnable"])
            self.assertEqual(len(descriptor["RenderTarget"]), 8)
            self.assertTrue(descriptor["RenderTarget"][1]["BlendEnable"])
            self.assertEqual(descriptor["RenderTarget"][1]["RenderTargetWriteMask"], 3)

    def test_raster_and_depth_descriptors_are_complete(self) -> None:
        raster = [
            row for row in self.receipt["renderStateDescriptors"]
            if row["descriptorKind"] == "D3D11_RASTERIZER_DESC"
        ]
        depth = [
            row for row in self.receipt["renderStateDescriptors"]
            if row["descriptorKind"] == "D3D11_DEPTH_STENCIL_DESC"
        ]
        self.assertEqual(len(raster), 18)
        self.assertTrue(all(row["expectedDescriptor"]["CullMode"] == 1 for row in raster))
        self.assertEqual(len(depth), 1)
        self.assertFalse(depth[0]["expectedDescriptor"]["DepthEnable"])
        self.assertEqual(depth[0]["expectedDescriptor"]["DepthWriteMask"], 0)

    def test_descriptor_coordinated_reseal_is_rejected(self) -> None:
        forged = copy.deepcopy(self.receipt)
        row = next(
            row for row in forged["renderStateDescriptors"]
            if row["descriptorKind"] == "D3D11_BLEND_DESC"
        )
        row["expectedDescriptor"]["RenderTarget"][0]["DestBlend"] = 2
        reseal_row(row)
        reseal_receipt(forged)
        self.assert_invalid(forged, pure=True)

    def test_bool_for_integer_and_nonfinite_float_are_rejected(self) -> None:
        forged = copy.deepcopy(self.receipt)
        forged["summary"]["recipeTextureBindingCount"] = False
        reseal_receipt(forged)
        self.assert_invalid(forged, pure=True)

        forged = copy.deepcopy(self.receipt)
        forged["neutralProviders"][0]["rgbaF32"][0] = math.nan
        self.assert_invalid(forged, pure=True)

    def test_missing_extra_and_key_reorder_are_rejected(self) -> None:
        missing = copy.deepcopy(self.receipt)
        missing.pop("summary")
        reseal_receipt(missing)
        self.assert_invalid(missing, pure=True)

        extra = copy.deepcopy(self.receipt)
        extra["forged"] = False
        reseal_receipt(extra)
        self.assert_invalid(extra, pure=True)

        reordered = copy.deepcopy(self.receipt)
        schema = reordered.pop("schema")
        reordered = {"formatVersion": reordered.pop("formatVersion"), "schema": schema, **reordered}
        reseal_receipt(reordered)
        self.assert_invalid(reordered, pure=True)

    def test_row_key_reorder_with_valid_canonical_seal_is_rejected(self) -> None:
        forged = copy.deepcopy(self.receipt)
        original = forged["recipeTextureBindings"][0]
        unsigned = {key: value for key, value in reversed(list(original.items())) if key != "rowSha256"}
        unsigned["rowSha256"] = binding.canonical_sha256(unsigned)
        forged["recipeTextureBindings"][0] = unsigned
        reseal_receipt(forged)
        self.assert_invalid(forged, pure=True)

    def test_duplicate_json_keys_are_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "duplicate.json"
            path.write_text('{"schema":"a","schema":"b"}', encoding="utf-8")
            with self.assertRaises(ValueError):
                binding.strict_io.load_strict_json_object(path)

    def test_independent_pin_is_required_beyond_pure_validation(self) -> None:
        self.validate(self.receipt, require_approval=False)
        with (
            mock.patch.object(
                approval,
                "APPROVED_DECISION_PROJECTION_SHA256",
                approval.decision_projection_sha256(self.receipt),
            ),
            mock.patch.object(
                approval,
                "APPROVED_RECEIPT_PROJECTION_SHA256",
                approval.receipt_projection_sha256(self.receipt),
            ),
        ):
            self.validate(self.receipt)
        with mock.patch.object(
            approval, "APPROVED_DECISION_PROJECTION_SHA256", "0" * 64
        ):
            self.assert_invalid(self.receipt)

    def test_current_authority_bytes_are_not_replaced_by_supplied_objects(self) -> None:
        original = binding.strict_io.tracked_text_sha256

        def forged_hash(path: Path) -> str:
            if path.name == "skill.31470.material-runtime-oracle.receipt.json":
                return "0" * 64
            return original(path)

        with mock.patch.object(binding.strict_io, "tracked_text_sha256", side_effect=forged_hash):
            self.assert_invalid(self.receipt, pure=True)

    def test_action_time_io_and_transaction_rollback_are_closed(self) -> None:
        contract = self.receipt["approvalContract"]
        blockers = self.receipt["blockerProjection"]
        self.assertFalse(contract["runtimeNameOrRoleHeuristicsAllowed"])
        self.assertFalse(contract["actionTimeIoAllowed"])
        self.assertEqual(contract["transactionPolicy"], "PARSE_VALIDATE_STAGE_COMMIT_OR_ROLLBACK")
        self.assertEqual(
            blockers["bindingFailureBehavior"],
            "ROLLBACK_PRESERVE_PREVIOUS_RESOURCE_SET",
        )
        self.assertFalse(blockers["partialCommitAllowed"])


if __name__ == "__main__":
    unittest.main()
