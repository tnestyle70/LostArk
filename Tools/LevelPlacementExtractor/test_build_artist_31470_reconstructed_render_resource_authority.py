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


def reseal_publisher_entry(entry: dict) -> None:
    link = entry["reconstructedRuntimeProgram"]
    receipt = entry["publishReceipt"]
    receipt["reconstructedRuntimeProgramSha256"] = builder.canonical_sha256(link)
    receipt.pop("receiptSha256", None)
    receipt["receiptSha256"] = builder.canonical_sha256(receipt)
    entry["publishReceiptSha256"] = builder.canonical_sha256(receipt)


class ReconstructedRenderResourceAuthorityTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        _, cls.program = builder._read_program()
        cls.approval_raw, cls.approval = builder._read_approval(cls.program)
        cls.expected, cls.payloads = builder.build_receipt(
            cls.program,
            cls.approval,
            _inputs_already_validated=True,
        )
        (
            _,
            cls.publisher_catalog,
            cls.publisher_entry,
            cls.publisher_expected_entry,
        ) = builder._read_publisher_runtime_authority()
        cls.serialized = builder.serialized_receipt(cls.expected)

    def receipt_copy(self) -> dict:
        return copy.deepcopy(self.expected)

    def assertPureRejects(self, receipt: dict, **kwargs: object) -> None:
        with self.assertRaises(
            (ValueError, KeyError, TypeError, builder.program_module.ContractError)
        ):
            builder.validate_receipt(
                receipt,
                require_independent_approval=False,
                **kwargs,
            )

    def test_01_exact_denominators_and_derived_formats(self) -> None:
        summary = self.expected["summary"]
        self.assertEqual(48, summary["textureResourceCount"])
        self.assertEqual(72, summary["textureBindingCount"])
        self.assertEqual(
            {
                "BC1_SRGB": 35,
                "BC3_LINEAR": 1,
                "BC3_SRGB": 8,
                "BC5_LINEAR": 4,
            },
            {row["classification"]: row["count"] for row in summary["resourceFormatCounts"]},
        )
        self.assertEqual(
            {72: 58, 77: 1, 78: 9, 83: 4},
            {row["dxgiFormat"]: row["count"] for row in summary["bindingSrvDxgiFormatCounts"]},
        )
        self.assertEqual(
            {"LINEAR": 5, "SRGB": 67},
            {
                row["colorSpacePolicy"]: row["count"]
                for row in summary["bindingColorSpaceCounts"]
            },
        )
        self.assertEqual(27, summary["recipeTextureBindingCount"])
        self.assertEqual(57, summary["rendererSlotBindingCount"])
        self.assertEqual(3, summary["ambiguousRendererDecisionCount"])
        self.assertEqual(46, summary["renderStateDescriptorCount"])

    def test_02_canonical_receipt_and_independent_pins_accept(self) -> None:
        builder.validate_receipt(self.expected)
        self.assertEqual(
            authority_module.APPROVED_DECISION_PROJECTION_SHA256,
            self.expected["decisionProjectionSha256"],
        )
        self.assertEqual(
            authority_module.APPROVED_RECEIPT_PROJECTION_SHA256,
            self.expected["receiptSha256"],
        )

    def test_03_serialization_is_lf_utf8_and_contains_no_absolute_path(self) -> None:
        self.assertFalse(self.serialized.startswith(b"\xef\xbb\xbf"))
        self.assertNotIn(b"\r", self.serialized)
        self.assertNotIn(b"C:\\", self.serialized)
        self.assertNotIn(b"C:/", self.serialized)
        self.assertNotIn(b"Users", self.serialized)
        self.assertEqual(self.serialized, builder.DEFAULT_OUTPUT.read_bytes())

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

    def test_25_publisher_runtime_authority_tuple_is_exact(self) -> None:
        evidence = self.expected["sourceEvidence"]["publisherRuntimeCatalogAuthority"]
        self.assertEqual(26_255_931, evidence["currentCheckoutByteCount"])
        self.assertEqual(
            "bf0807ec1b4d975c988ed7e8bb204c6b1713218968be76ea6accb6340e714d29",
            evidence["currentCheckoutRawSha256"],
        )
        self.assertEqual(10, evidence["outerKeyCount"])
        self.assertEqual(16, evidence["linkKeyCount"])
        self.assertEqual(25, evidence["receiptKeyCount"])
        self.assertEqual(3, evidence["toolDependencyCount"])
        self.assertEqual(
            "74175fe1e41b22ae593a9d1ff92027606bc0b31d62d17927ef6ac5673dd4a7a2",
            evidence["linkCanonicalSha256"],
        )
        self.assertEqual(
            "5c91709f2f0ec855c54c94e6dad5bcd7ed048c6133ca9a9af7d4873f20da1bd3",
            evidence["receiptSelfSha256"],
        )
        self.assertEqual(
            "92c883f78d88018a50d8dec09eb6fb155974bec4b3756a796b3499fc2f839d94",
            evidence["outerPublishReceiptSha256"],
        )
        builder.publisher_module.validate_reconstructed_runtime_entry(
            self.publisher_entry
        )
        builder.strict_ordered_equal(
            self.publisher_entry,
            self.publisher_expected_entry,
            "testPublisherExpected",
        )

    def test_26_publisher_outer_coordinated_reseal_rejects(self) -> None:
        forged = copy.deepcopy(self.publisher_entry)
        forged["artifactRevision"] = 2
        forged["publishReceipt"]["artifactRevision"] = 2
        reseal_publisher_entry(forged)
        self.assertPureRejects(self.expected, supplied_publisher_entry=forged)

    def test_27_publisher_link_coordinated_reseal_rejects(self) -> None:
        forged = copy.deepcopy(self.publisher_entry)
        forged["reconstructedRuntimeProgram"]["programVersion"] = 2
        forged["publishReceipt"]["programVersion"] = 2
        reseal_publisher_entry(forged)
        self.assertPureRejects(self.expected, supplied_publisher_entry=forged)

    def test_28_publisher_receipt_coordinated_reseal_rejects(self) -> None:
        forged = copy.deepcopy(self.publisher_entry)
        forged["publishReceipt"]["receiptRole"] += "_FORGED"
        reseal_publisher_entry(forged)
        self.assertPureRejects(self.expected, supplied_publisher_entry=forged)

    def test_29_publisher_tool_coordinated_reseal_rejects(self) -> None:
        forged = copy.deepcopy(self.publisher_entry)
        forged["publishReceipt"]["toolDependencies"][1]["sha256"] = "0" * 64
        reseal_publisher_entry(forged)
        self.assertPureRejects(self.expected, supplied_publisher_entry=forged)

    def test_30_publisher_catalog_a_b_coordinated_reseal_rejects(self) -> None:
        forged_catalog = copy.deepcopy(self.publisher_catalog)
        forged = forged_catalog["effects"][builder.RUNTIME_ENTRY_EFFECT_INDEX]
        alternate_id = "effect.artist.skill.31470.b"
        forged["effectAssetId"] = alternate_id
        forged["reconstructedRuntimeProgram"]["effectAssetId"] = alternate_id
        forged["publishReceipt"]["effectAssetId"] = alternate_id
        reseal_publisher_entry(forged)
        raw = json.dumps(
            forged_catalog,
            ensure_ascii=False,
            separators=(",", ":"),
            allow_nan=False,
        ).encode("utf-8")
        self.assertPureRejects(self.expected, supplied_runtime_catalog_bytes=raw)

    def test_31_publisher_read_trace_and_current_tools_are_mandatory(self) -> None:
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
        actual_catalog_bytes = original_read_bytes(builder.DEFAULT_RUNTIME_CATALOG)
        forged_catalog = copy.deepcopy(self.publisher_catalog)
        forged_catalog["components"][0] = {
            "forged": "SECOND_PATH_READ_MUST_NEVER_BECOME_THE_VALIDATED_OBJECT"
        }
        forged_catalog_bytes = json.dumps(
            forged_catalog,
            ensure_ascii=False,
            separators=(",", ":"),
            allow_nan=False,
        ).encode("utf-8")
        original_parse = builder._parse_json_object_bytes
        original_validate_catalog = builder.publisher_module.validate_runtime_catalog
        saved_catalog_cache = builder._PUBLISHER_CATALOG_CACHE
        saved_expected_cache = builder._PUBLISHER_EXPECTED_ENTRY_CACHE

        def invoke_split_read() -> tuple[
            tuple[bytes, dict, dict, dict], list[Path], int, list[dict], list[dict]
        ]:
            reads: list[Path] = []
            runtime_catalog_read_count = 0
            parsed_catalogs: list[dict] = []
            publicly_validated_catalogs: list[dict] = []

            def split_read_bytes(path: Path) -> bytes:
                nonlocal runtime_catalog_read_count
                resolved = path.resolve()
                reads.append(resolved)
                if resolved == builder.DEFAULT_RUNTIME_CATALOG.resolve():
                    runtime_catalog_read_count += 1
                    if runtime_catalog_read_count == 1:
                        return actual_catalog_bytes
                    return forged_catalog_bytes
                return original_read_bytes(path)

            def traced_parse(raw: bytes, label: str) -> dict:
                value = original_parse(raw, label)
                if label == "current publisher runtime catalog":
                    parsed_catalogs.append(value)
                return value

            def traced_validate_catalog(value: dict) -> None:
                publicly_validated_catalogs.append(value)
                original_validate_catalog(value)

            with (
                mock.patch.object(Path, "read_bytes", split_read_bytes),
                mock.patch.object(builder, "_parse_json_object_bytes", traced_parse),
                mock.patch.object(
                    builder.publisher_module,
                    "validate_runtime_catalog",
                    traced_validate_catalog,
                ),
                mock.patch.object(
                    builder.publisher_module,
                    "load_json",
                    side_effect=AssertionError(
                        "publisher catalog path was reopened after exact-byte hashing"
                    ),
                ),
            ):
                authority = builder._read_publisher_runtime_authority()
            return (
                authority,
                reads,
                runtime_catalog_read_count,
                parsed_catalogs,
                publicly_validated_catalogs,
            )

        try:
            builder._PUBLISHER_CATALOG_CACHE = None
            builder._PUBLISHER_EXPECTED_ENTRY_CACHE = None
            cold, cold_reads, cold_count, parsed, publicly_validated = invoke_split_read()
            cold_raw, cold_catalog, cold_entry, cold_expected = cold
            self.assertEqual(1, cold_count)
            self.assertEqual(actual_catalog_bytes, cold_raw)
            self.assertEqual(1, len(parsed))
            self.assertEqual(1, len(publicly_validated))
            self.assertIs(cold_catalog, parsed[0])
            self.assertIs(cold_catalog, publicly_validated[0])
            self.assertIs(cold_entry, cold_catalog["effects"][builder.RUNTIME_ENTRY_EFFECT_INDEX])
            self.assertNotEqual(
                forged_catalog["components"][0],
                cold_catalog["components"][0],
            )

            cached, cached_reads, cached_count, cached_parsed, cached_validated = (
                invoke_split_read()
            )
            cached_raw, cached_catalog, cached_entry, cached_expected = cached
            self.assertEqual(1, cached_count)
            self.assertEqual(actual_catalog_bytes, cached_raw)
            self.assertEqual([], cached_parsed)
            self.assertEqual([], cached_validated)
            self.assertIs(cached_catalog, cold_catalog)
            self.assertIs(cached_entry, cold_entry)
            self.assertIs(cached_expected, cold_expected)

            for reads in (cold_reads, cached_reads):
                self.assertEqual(
                    1,
                    reads.count(builder.DEFAULT_RUNTIME_CATALOG.resolve()),
                )
                for relative, _blob_id in builder.PUBLISHER_TOOL_BLOBS:
                    self.assertIn((builder.ROOT / relative).resolve(), reads)
            current_tools = builder.publisher_module._current_reconstructed_tool_dependencies()
            self.assertEqual(
                current_tools,
                cached_entry["publishReceipt"]["toolDependencies"],
            )
        finally:
            builder._PUBLISHER_CATALOG_CACHE = saved_catalog_cache
            builder._PUBLISHER_EXPECTED_ENTRY_CACHE = saved_expected_cache


if __name__ == "__main__":
    unittest.main(verbosity=2)
