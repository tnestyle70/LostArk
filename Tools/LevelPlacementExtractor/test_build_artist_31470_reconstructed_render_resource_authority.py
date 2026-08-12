#!/usr/bin/env python3
"""Strict regressions for the Artist 31470 render-resource authority."""

from __future__ import annotations

import copy
import hashlib
import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock


SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

import artist_31470_reconstructed_render_resource_authority as authority_module
import build_artist_31470_material_render_resource_binding_approval as approval_builder
import build_artist_31470_reconstructed_render_resource_authority as builder


def reseal_authority(receipt: dict) -> None:
    receipt["decisionProjectionSha256"] = authority_module.decision_projection_sha256(receipt)
    receipt.pop("receiptSha256", None)
    receipt["receiptSha256"] = builder.canonical_sha256(receipt)


def reseal_row_and_authority(receipt: dict, section: str, index: int) -> None:
    row = receipt[section][index]
    row.pop("rowSha256", None)
    row["rowSha256"] = builder.canonical_sha256(row)
    reseal_authority(receipt)


def reseal_material_approval(receipt: dict, section: str, index: int) -> None:
    row = receipt[section][index]
    row.pop("rowSha256", None)
    row["rowSha256"] = approval_builder.canonical_sha256(row)
    receipt.pop("receiptSha256", None)
    receipt["receiptSha256"] = approval_builder.canonical_sha256(receipt)


class ReconstructedRenderResourceAuthorityTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.temporary = tempfile.TemporaryDirectory()
        root = Path(cls.temporary.name)
        cls.program_path = root / "program.json"
        cls.approval_path = root / "approval.json"
        cls.output_path = root / "sidecar.json"
        cls.program = builder.program_module.build_program()
        cls.program_raw = builder.program_module.output_bytes(cls.program)
        cls.program_path.write_bytes(cls.program_raw)
        cls.approval = approval_builder.build_receipt(cls.program)
        approval_builder.validate_receipt(
            cls.approval,
            cls.program,
            _program_already_validated=True,
            require_approval=False,
        )
        cls.approval_raw = approval_builder.serialized_receipt(cls.approval)
        cls.approval_path.write_bytes(cls.approval_raw)
        cls.expected, cls.payloads = builder.build_receipt(
            cls.program,
            cls.approval,
            require_independent_approval=False,
            _inputs_already_validated=True,
        )
        cls.publisher_projection = builder._build_publisher_base_authority(
            cls.program
        )
        cls.serialized = builder.serialized_receipt(cls.expected)
        cls.output_path.write_bytes(cls.serialized)

    @classmethod
    def tearDownClass(cls) -> None:
        cls.temporary.cleanup()

    def receipt_copy(self) -> dict:
        return copy.deepcopy(self.expected)

    def assertPureRejects(self, receipt: dict, **kwargs: object) -> None:
        with self.assertRaises(
            (ValueError, KeyError, TypeError, builder.program_module.ContractError)
        ):
            builder.validate_receipt(
                receipt,
                program_path=self.program_path,
                approval_path=self.approval_path,
                require_independent_approval=False,
                **kwargs,
            )

    def test_01_exact_denominators_and_derived_formats(self) -> None:
        summary = self.expected["summary"]
        self.assertEqual(52, summary["textureResourceCount"])
        self.assertEqual(77, summary["textureBindingCount"])
        self.assertEqual(
            {
                "BC1_SRGB": 39,
                "BC3_LINEAR": 1,
                "BC3_SRGB": 8,
                "BC5_LINEAR": 4,
            },
            {row["classification"]: row["count"] for row in summary["resourceFormatCounts"]},
        )
        self.assertEqual(
            {72: 63, 77: 1, 78: 9, 83: 4},
            {row["dxgiFormat"]: row["count"] for row in summary["bindingSrvDxgiFormatCounts"]},
        )
        self.assertEqual(
            {"LINEAR": 5, "SRGB": 72},
            {
                row["colorSpacePolicy"]: row["count"]
                for row in summary["bindingColorSpaceCounts"]
            },
        )
        self.assertEqual(27, summary["recipeTextureBindingCount"])
        self.assertEqual(57, summary["rendererSlotBindingCount"])
        self.assertEqual(3, summary["ambiguousRendererDecisionCount"])
        self.assertEqual(46, summary["renderStateDescriptorCount"])

    def test_02_canonical_receipt_matches_independent_pins(self) -> None:
        builder.validate_receipt(
            self.expected,
            program_path=self.program_path,
            approval_path=self.approval_path,
            require_independent_approval=False,
        )
        authority_module.require_approved_receipt(self.expected)

    def test_03_serialization_is_lf_utf8_and_contains_no_absolute_path(self) -> None:
        self.assertFalse(self.serialized.startswith(b"\xef\xbb\xbf"))
        self.assertNotIn(b"\r", self.serialized)
        self.assertNotIn(b"C:\\", self.serialized)
        self.assertNotIn(b"C:/", self.serialized)
        self.assertNotIn(b"Users", self.serialized)
        self.assertEqual(self.serialized, self.output_path.read_bytes())

    def test_04_every_resource_matches_current_bytes_and_dds_header(self) -> None:
        resources = {row["runtimeAssetId"]: row for row in self.expected["textureResources"]}
        self.assertEqual(set(resources), set(self.payloads))
        for asset_id, raw in self.payloads.items():
            row = resources[asset_id]
            self.assertEqual(len(raw), row["byteCount"])
            self.assertEqual(hashlib.sha256(raw).hexdigest(), row["rawSha256"])
            self.assertEqual(builder.parse_dds(raw, asset_id), row["ddsHeader"])
            self.assertTrue(row["ddsHeader"]["payloadByteCountExact"])

    def test_05_prior_rgba_fixture_is_separate_from_actual_compressed_srv(self) -> None:
        for row in self.expected["textureBindings"]:
            fixture = row["priorPolicySrvFixture"]
            actual = row["actualDdsSrvDescriptor"]
            self.assertEqual(
                "MATERIAL_POLICY_1X1_RGBA8_SRV_ORACLE_NOT_ACTUAL_DDS_DESCRIPTOR",
                fixture["fixtureKind"],
            )
            self.assertIn(fixture["expectedSrv"]["Format"], {28, 29})
            self.assertIn(actual["Format"], {72, 77, 78, 83})
            self.assertNotEqual(fixture["expectedSrv"]["Format"], actual["Format"])

    def test_06_coordinated_dds_header_mutation_rejects(self) -> None:
        receipt = self.receipt_copy()
        receipt["textureResources"][0]["ddsHeader"]["width"] += 4
        reseal_row_and_authority(receipt, "textureResources", 0)
        self.assertPureRejects(receipt)

    def test_07_coordinated_actual_dds_identity_mutation_rejects(self) -> None:
        receipt = self.receipt_copy()
        receipt["textureResources"][0]["rawSha256"] = "0" * 64
        reseal_row_and_authority(receipt, "textureResources", 0)
        self.assertPureRejects(receipt)

    def test_08_coordinated_actual_srv_format_mutation_rejects(self) -> None:
        receipt = self.receipt_copy()
        receipt["textureBindings"][0]["actualDdsSrvDescriptor"]["Format"] = 77
        reseal_row_and_authority(receipt, "textureBindings", 0)
        self.assertPureRejects(receipt)

    def test_09_coordinated_color_policy_mutation_rejects(self) -> None:
        receipt = self.receipt_copy()
        receipt["textureBindings"][0]["colorSpacePolicy"] = "LINEAR"
        reseal_row_and_authority(receipt, "textureBindings", 0)
        self.assertPureRejects(receipt)

    def test_10_coordinated_binding_owner_mutation_rejects(self) -> None:
        receipt = self.receipt_copy()
        original = receipt["textureBindings"][0]["recipeId"]
        alternate = next(
            row["recipeId"] for row in receipt["textureBindings"]
            if row["recipeId"] != original
        )
        receipt["textureBindings"][0]["recipeId"] = alternate
        reseal_row_and_authority(receipt, "textureBindings", 0)
        self.assertPureRejects(receipt)

    def test_11_coordinated_recipe_choice_mutation_rejects(self) -> None:
        receipt = self.receipt_copy()
        provider = receipt["recipeTextureBindings"][0]["texture0Provider"]
        provider["runtimeAssetId"] = receipt["textureResources"][1]["runtimeAssetId"]
        reseal_row_and_authority(receipt, "recipeTextureBindings", 0)
        self.assertPureRejects(receipt)

    def test_12_coordinated_renderer_ambiguity_choice_mutation_rejects(self) -> None:
        receipt = self.receipt_copy()
        index = next(
            index
            for index, row in enumerate(receipt["rendererSlotBindings"])
            if row["candidateCount"] == 2
        )
        row = receipt["rendererSlotBindings"][index]
        alternate = next(
            candidate
            for candidate in row["candidates"]
            if candidate["materialInputFieldId"] != row["selectedMaterialInputFieldId"]
        )
        row["selectedMaterialInputFieldId"] = alternate["materialInputFieldId"]
        row["selectedMaterialInputRowSha256"] = alternate["materialInputRowSha256"]
        row["selectedNormalizedParameterName"] = alternate["normalizedParameterName"]
        row["selectedTextureBindingId"] = alternate["textureBindingId"]
        row["selectedTextureBindingRowSha256"] = alternate["textureBindingRowSha256"]
        reseal_row_and_authority(receipt, "rendererSlotBindings", index)
        self.assertPureRejects(receipt)

    def test_13_coordinated_d3d_descriptor_mutation_rejects(self) -> None:
        receipt = self.receipt_copy()
        row = next(
            row for row in receipt["renderStateDescriptors"]
            if row["descriptorKind"] == "D3D11_BLEND_DESC"
        )
        index = receipt["renderStateDescriptors"].index(row)
        row["expectedDescriptor"]["RenderTarget"][0]["BlendEnable"] = False
        reseal_row_and_authority(receipt, "renderStateDescriptors", index)
        self.assertPureRejects(receipt)

    def test_14_resource_reorder_with_root_reseal_rejects(self) -> None:
        receipt = self.receipt_copy()
        receipt["textureResources"][0], receipt["textureResources"][1] = (
            receipt["textureResources"][1], receipt["textureResources"][0]
        )
        reseal_authority(receipt)
        self.assertPureRejects(receipt)

    def test_15_binding_reorder_with_root_reseal_rejects(self) -> None:
        receipt = self.receipt_copy()
        receipt["textureBindings"][0], receipt["textureBindings"][1] = (
            receipt["textureBindings"][1], receipt["textureBindings"][0]
        )
        reseal_authority(receipt)
        self.assertPureRejects(receipt)

    def test_16_supplied_external_dds_mutation_rejects(self) -> None:
        supplied = dict(self.payloads)
        asset_id = next(iter(supplied))
        payload = bytearray(supplied[asset_id])
        payload[-1] ^= 0x01
        supplied[asset_id] = bytes(payload)
        self.assertPureRejects(self.expected, supplied_resource_payloads=supplied)

    def test_17_supplied_approval_coordinated_reseal_rejects(self) -> None:
        forged = copy.deepcopy(self.approval)
        index = next(
            index
            for index, row in enumerate(forged["rendererSlotBindings"])
            if row["candidateCount"] == 2
        )
        forged["rendererSlotBindings"][index]["rationale"] += " forged"
        reseal_material_approval(forged, "rendererSlotBindings", index)
        raw = (json.dumps(forged, ensure_ascii=False, indent=2) + "\n").encode("utf-8")
        self.assertPureRejects(self.expected, supplied_approval_bytes=raw)

    def test_18_supplied_current_tool_mutation_rejects(self) -> None:
        forged = builder.DEFAULT_TOOL.read_bytes() + b"\n# coordinated tool mutation\n"
        self.assertPureRejects(self.expected, supplied_tool_bytes=forged)

    def test_19_noncanonical_resource_root_rejects(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            self.assertPureRejects(self.expected, resources_root=Path(temp_dir))

    def test_20_coordinated_runtime_path_mutation_rejects(self) -> None:
        receipt = self.receipt_copy()
        receipt["textureResources"][0]["runtimeAssetId"] = "../forged.dds"
        reseal_row_and_authority(receipt, "textureResources", 0)
        self.assertPureRejects(receipt)

    def test_21_program_a_b_identity_mutation_rejects(self) -> None:
        forged = copy.deepcopy(self.program)
        forged["programSha256"] = "0" * 64
        self.assertPureRejects(self.expected, program=forged)

    def test_22_strict_recursive_bool_type_mutation_rejects(self) -> None:
        receipt = self.receipt_copy()
        receipt["admission"]["product"] = 0
        reseal_authority(receipt)
        self.assertPureRejects(receipt)

    def test_23_independent_pin_rejects_fully_resealed_decision(self) -> None:
        receipt = self.receipt_copy()
        receipt["blockerProjection"]["blockers"] = receipt["blockerProjection"]["blockers"][:-1]
        reseal_authority(receipt)
        with self.assertRaises(ValueError):
            authority_module.require_approved_receipt(receipt)

    def test_24_action_time_and_product_admission_remain_false(self) -> None:
        self.assertFalse(self.expected["authorityContract"]["actionTimeIoAllowed"])
        self.assertFalse(self.expected["authorityContract"]["runtimeExecutionAdmission"])
        self.assertFalse(self.expected["authorityContract"]["product"])
        self.assertFalse(self.expected["admission"]["runtimeExecutionAdmission"])
        self.assertFalse(self.expected["admission"]["product"])

    def test_25_publisher_base_projection_is_exact_and_self_reference_free(self) -> None:
        evidence = self.expected["sourceEvidence"]["publisherRuntimeCatalogAuthority"]
        self.assertEqual(
            "BASE_RUNTIME_ENTRY_PROJECTION_BEFORE_RENDER_RESOURCE_SIDECAR",
            evidence["authorityScope"],
        )
        self.assertFalse(evidence["runtimeCatalogBytesRead"])
        self.assertFalse(evidence["completedRuntimeEntryRead"])
        self.assertFalse(evidence["renderResourceSidecarRead"])
        self.assertTrue(evidence["selfReferenceExcluded"])
        self.assertEqual(
            tuple(builder.publisher_module.RECONSTRUCTED_BASE_AUTHORITY_PROJECTION_KEYS),
            tuple(evidence["projectionKeyOrder"]),
        )
        self.assertEqual(
            len(self.publisher_projection), evidence["projectionKeyCount"]
        )
        builder.strict_ordered_equal(
            evidence["baseProjection"],
            self.publisher_projection,
            "testPublisherBaseProjection",
        )
        self.assertEqual(
            builder.canonical_sha256(self.publisher_projection),
            evidence["projectionCanonicalSha256"],
        )
        builder.publisher_module.validate_reconstructed_base_authority_projection(
            evidence["baseProjection"]
        )

    def test_26_base_projection_program_identity_reseal_rejects(self) -> None:
        receipt = self.receipt_copy()
        authority = receipt["sourceEvidence"]["publisherRuntimeCatalogAuthority"]
        authority["baseProjection"]["programSha256"] = "0" * 64
        authority["projectionCanonicalSha256"] = builder.canonical_sha256(
            authority["baseProjection"]
        )
        reseal_authority(receipt)
        self.assertPureRejects(receipt)

    def test_27_base_projection_scope_reseal_rejects(self) -> None:
        receipt = self.receipt_copy()
        receipt["sourceEvidence"]["publisherRuntimeCatalogAuthority"][
            "authorityScope"
        ] += "_FORGED"
        reseal_authority(receipt)
        self.assertPureRejects(receipt)

    def test_28_catalog_or_completed_entry_dependency_reseal_rejects(self) -> None:
        for field in ("runtimeCatalogBytesRead", "completedRuntimeEntryRead",
                      "renderResourceSidecarRead"):
            receipt = self.receipt_copy()
            receipt["sourceEvidence"]["publisherRuntimeCatalogAuthority"][field] = True
            reseal_authority(receipt)
            self.assertPureRejects(receipt)

    def test_29_removed_catalog_and_entry_injection_api_rejects(self) -> None:
        self.assertPureRejects(
            self.expected,
            supplied_runtime_catalog_bytes=b"{}",
        )
        self.assertPureRejects(
            self.expected,
            supplied_publisher_entry={},
        )

    def test_30_public_projection_validator_rejects_mutations(self) -> None:
        for field, value in (
            ("programVersion", 2),
            ("sourceExact", 0),
            ("productAdmission", True),
        ):
            forged = copy.deepcopy(self.publisher_projection)
            forged[field] = value
            with self.assertRaises(builder.publisher_module.ContractError):
                builder.publisher_module.validate_reconstructed_base_authority_projection(
                    forged
                )

    def test_31_publisher_projection_never_reads_catalog_or_sidecar(self) -> None:
        with self.assertRaisesRegex(ValueError, "duplicate JSON key schema"):
            builder._parse_json_object_bytes(
                b'{"schema":1,"schema":2}',
                "publisher duplicate fixture",
            )
        with self.assertRaisesRegex(ValueError, "non-finite JSON token NaN"):
            builder._parse_json_object_bytes(
                b'{"schema":NaN}',
                "publisher non-finite fixture",
            )
        original_read_bytes = Path.read_bytes
        reads: list[Path] = []

        def traced_read_bytes(path: Path) -> bytes:
            reads.append(path.resolve())
            return original_read_bytes(path)

        with mock.patch.object(Path, "read_bytes", traced_read_bytes):
            actual = builder._build_publisher_base_authority(self.program)
        builder.strict_ordered_equal(
            actual, self.publisher_projection, "tracedPublisherProjection"
        )
        self.assertFalse(any(path.name == "EffectCatalog.runtime.json" for path in reads))
        self.assertFalse(any(
            path.name == "skill.31470.reconstructed-render-resource-authority.receipt.json"
            for path in reads
        ))


if __name__ == "__main__":
    unittest.main(verbosity=2)
