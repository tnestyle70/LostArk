#!/usr/bin/env python3
"""Mutation tests for Artist F RECONSTRUCTED_APPROVED_V1 Material policy."""

from __future__ import annotations

import copy
import hashlib
import json
import math
import subprocess
import struct
import sys
import tempfile
import unittest
from pathlib import Path

from build_artist_31470_material_reconstructed_policy import (
    DEFAULT_ACQUISITION_RECEIPT,
    DEFAULT_HLSL,
    DEFAULT_MATERIAL_CONTRACT,
    DEFAULT_OUTPUT,
    DEFAULT_RUNTIME_RECEIPT,
    DEFAULT_VERIFIER,
    DIRECT_IMPORT_DEPENDENCY_PATHS,
    ROOT,
    canonical_sha256,
    direct_import_closure,
    read_json,
    resolve_current_texture_filter_candidate,
    validate_direct_import_closure,
    validate_policy_receipt,
)
from artist_31470_material_reconstructed_policy_approval import require_approved_receipt
from verify_artist_31470_material_reconstructed_policy_hlsl import oracle_input_bytes


def reseal_row(row: dict) -> None:
    row.pop("rowSha256", None)
    row["rowSha256"] = canonical_sha256(row)


def reseal_receipt(receipt: dict) -> None:
    receipt.pop("receiptSha256", None)
    receipt["receiptSha256"] = canonical_sha256(receipt)


def reseal_warp_rows(receipt: dict) -> None:
    rows = receipt["warpDescriptorVerification"]["rowResults"]
    payload = json.dumps(rows, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")
    receipt["warpDescriptorVerification"]["rowResultsSha256"] = hashlib.sha256(payload).hexdigest()


class ReconstructedMaterialPolicyTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.runtime = read_json(DEFAULT_RUNTIME_RECEIPT)
        cls.acquisition = read_json(DEFAULT_ACQUISITION_RECEIPT)
        cls.contract = read_json(DEFAULT_MATERIAL_CONTRACT)
        cls.receipt = read_json(DEFAULT_OUTPUT)

    def validate(self, receipt: dict) -> None:
        validate_policy_receipt(
            receipt,
            self.runtime,
            self.acquisition,
            self.contract,
            hlsl_path=DEFAULT_HLSL,
            verifier_path=DEFAULT_VERIFIER,
        )

    def assert_rejected(self, receipt: dict) -> None:
        with self.assertRaises((ValueError, TypeError, OverflowError)):
            self.validate(receipt)

    def run_cli(self, receipt_path: Path, *, deep: bool) -> subprocess.CompletedProcess[str]:
        command = [
            sys.executable,
            "-B",
            str(ROOT / "Tools/LevelPlacementExtractor/build_artist_31470_material_reconstructed_policy.py"),
            "--output",
            str(receipt_path),
        ]
        command.extend(["--run-hlsl", "--check"] if deep else ["--shallow-check"])
        return subprocess.run(
            command,
            cwd=ROOT,
            text=True,
            capture_output=True,
            timeout=90,
            check=False,
        )

    def test_baseline_receipt_is_valid(self) -> None:
        self.validate(copy.deepcopy(self.receipt))

    def test_denominators_and_policy_boundary(self) -> None:
        render = self.receipt["renderStatePolicies"]
        static = self.receipt["staticPermutationPolicies"]
        sampler = self.receipt["samplerPolicies"]
        self.assertEqual((len(render), len(static), len(sampler)), (89, 94, 72))
        rows = render + static + sampler
        self.assertEqual([row["policyOrder"] for row in rows], list(range(255)))
        self.assertTrue(all(row["sourceExact"] is False for row in rows))
        self.assertTrue(all(row["evidenceBlockers"]["sourceValueAcquisition"] for row in rows))
        self.assertTrue(all(row["evidenceBlockers"]["runtimeOracle"] for row in rows))
        self.assertEqual(self.receipt["admission"]["policySelection"], {"ready": True, "rowCount": 255})
        self.assertEqual(self.receipt["admission"]["runtimeConsumer"], {"ready": False, "rowCount": 0})
        self.assertFalse(self.receipt["admission"]["product"])

    def test_selected_value_denominators(self) -> None:
        render_values: dict[tuple[str, object], int] = {}
        for row in self.receipt["renderStatePolicies"]:
            key = (row["fieldName"], row["selectedValue"]["value"])
            render_values[key] = render_values.get(key, 0) + 1
        self.assertEqual(render_values[("bdisabledepthtest", False)], 26)
        self.assertEqual(render_values[("buseonelayerdistortion", False)], 26)
        self.assertEqual(render_values[("opacitymaskclipvalue", 0.33329999446868896)], 26)
        self.assertEqual(render_values[("twosided", False)], 9)
        self.assertEqual(render_values[("lightingmodel", "mlm_unlit")], 2)

        static = self.receipt["staticPermutationPolicies"]
        self.assertEqual(sum(row["selectedValue"]["value"] is True for row in static), 85)
        self.assertEqual(sum(row["selectedValue"]["value"] is False for row in static), 9)
        self.assertEqual(
            sum(
                row["providerBasis"]["basisId"]
                == "SOURCE_ARCHIVE_MIC_EXACT_OVERRIDE_RETAINED_AS_RECONSTRUCTION_POLICY"
                for row in static
            ),
            23,
        )

        samplers = self.receipt["samplerPolicies"]
        self.assertEqual(sum(row["selectedDescriptor"]["addressU"]["ue3"] == "ta_wrap" for row in samplers), 63)
        self.assertEqual(sum(row["selectedDescriptor"]["addressU"]["ue3"] == "ta_clamp" for row in samplers), 9)
        self.assertEqual(sum(row["selectedDescriptor"]["sRgb"] is True for row in samplers), 67)
        self.assertEqual(sum(row["selectedDescriptor"]["sRgb"] is False for row in samplers), 5)
        self.assertTrue(all(row["selectedDescriptor"]["filter"]["ue3"] == "tf_linear" for row in samplers))

    def test_every_row_has_explicit_typed_policy_and_oracle(self) -> None:
        rows = (
            self.receipt["renderStatePolicies"]
            + self.receipt["staticPermutationPolicies"]
            + self.receipt["samplerPolicies"]
        )
        for row in rows:
            self.assertTrue("selectedValue" in row or "selectedDescriptor" in row)
            self.assertIn("basisId", row["providerBasis"])
            self.assertEqual(row["implementation"]["implementationVersion"], 1)
            self.assertEqual(row["numericOracle"]["numericTolerance"], 0.0)
            self.assertTrue(all(math.isfinite(value) for value in row["numericOracle"]["expectedFloat4"]))
        self.assertEqual(self.receipt["hlslVerification"]["sampleCount"], 255)
        self.assertEqual(self.receipt["warpDescriptorVerification"]["descriptorRowCount"], 107)
        self.assertEqual(self.receipt["warpDescriptorVerification"]["srvColorSpaceRowCount"], 72)

    def test_row_reorder_and_ordinal_reseal_is_rejected(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        rows = candidate["renderStatePolicies"]
        rows[0], rows[1] = rows[1], rows[0]
        rows[0]["policyOrder"], rows[1]["policyOrder"] = 0, 1
        reseal_row(rows[0])
        reseal_row(rows[1])
        reseal_receipt(candidate)
        self.assert_rejected(candidate)

    def test_recipe_field_occurrence_owner_swap_reseal_is_rejected(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        first, second = candidate["samplerPolicies"][:2]
        for key in ("materialRecipeId", "materialOccurrenceIds", "fieldId", "bindingOriginAndOwner"):
            first[key], second[key] = copy.deepcopy(second[key]), copy.deepcopy(first[key])
        reseal_row(first)
        reseal_row(second)
        reseal_receipt(candidate)
        self.assert_rejected(candidate)

    def test_selected_render_value_coordinated_reseal_is_rejected(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        row = next(row for row in candidate["renderStatePolicies"] if row["fieldName"] == "bdisabledepthtest")
        row["selectedValue"]["value"] = True
        row["numericOracle"]["expectedFloat4"] = [1.0, 0.0, 1.0, 1.0]
        reseal_row(row)
        reseal_receipt(candidate)
        self.assert_rejected(candidate)

    def test_static_override_parent_selection_swap_is_rejected(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        override = next(
            row for row in candidate["staticPermutationPolicies"]
            if row["providerBasis"]["basisId"]
            == "SOURCE_ARCHIVE_MIC_EXACT_OVERRIDE_RETAINED_AS_RECONSTRUCTION_POLICY"
        )
        parent = next(
            row for row in candidate["staticPermutationPolicies"]
            if row["providerBasis"]["basisId"] == "SOURCE_ARCHIVE_PARENT_STATIC_DEFAULT_POLICY"
        )
        override["selectedValue"], parent["selectedValue"] = parent["selectedValue"], override["selectedValue"]
        override["providerBasis"], parent["providerBasis"] = parent["providerBasis"], override["providerBasis"]
        reseal_row(override)
        reseal_row(parent)
        reseal_receipt(candidate)
        self.assert_rejected(candidate)

    def test_sampler_explicit_clamp_to_wrap_is_rejected(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        row = next(row for row in candidate["samplerPolicies"] if row["selectedDescriptor"]["addressU"]["ue3"] == "ta_clamp")
        row["selectedDescriptor"]["addressU"] = {"ue3": "ta_wrap", "d3d11": 1}
        row["numericOracle"]["expectedFloat4"][1] = 1.0
        reseal_row(row)
        reseal_receipt(candidate)
        self.assert_rejected(candidate)

    def test_sampler_explicit_srgb_false_to_true_is_rejected(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        row = next(
            row for row in candidate["samplerPolicies"]
            if row["selectedDescriptor"]["sRgb"] is False
            and row["providerBasis"]["sRgb"]["basisId"].startswith("SOURCE_ARCHIVE_EXPLICIT")
        )
        row["selectedDescriptor"]["sRgb"] = True
        row["selectedDescriptor"]["srvColorSpace"] = "SRGB"
        row["numericOracle"]["expectedFloat4"][3] = 1.0
        reseal_row(row)
        reseal_receipt(candidate)
        self.assert_rejected(candidate)

    def test_sampler_descriptor_field_mutations_are_rejected(self) -> None:
        mutations = (
            ("filter", {"ue3": "tf_point", "d3d11": 0}),
            ("addressW", {"ue3": "ta_clamp", "d3d11": 3}),
            ("maxAnisotropy", 16),
            ("borderColor", [1.0, 0.0, 0.0, 0.0]),
            ("maxLOD", 1.0),
        )
        for field, value in mutations:
            with self.subTest(field=field):
                candidate = copy.deepcopy(self.receipt)
                row = candidate["samplerPolicies"][0]
                row["selectedDescriptor"][field] = value
                reseal_row(row)
                reseal_receipt(candidate)
                self.assert_rejected(candidate)

    def test_unknown_lod_group_is_rejected(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        row = candidate["samplerPolicies"][0]
        row["selectedDescriptor"]["lodGroup"] = "texturegroup_fabricated"
        reseal_row(row)
        reseal_receipt(candidate)
        self.assert_rejected(candidate)

    def test_provider_and_implementation_swap_are_rejected(self) -> None:
        for key in ("providerBasis", "implementation"):
            with self.subTest(key=key):
                candidate = copy.deepcopy(self.receipt)
                first, second = candidate["renderStatePolicies"][:2]
                first[key], second[key] = copy.deepcopy(second[key]), copy.deepcopy(first[key])
                reseal_row(first)
                reseal_row(second)
                reseal_receipt(candidate)
                self.assert_rejected(candidate)

    def test_blocker_removal_and_source_exact_promotion_are_rejected(self) -> None:
        for mutation in ("blocker", "sourceExact"):
            with self.subTest(mutation=mutation):
                candidate = copy.deepcopy(self.receipt)
                row = candidate["renderStatePolicies"][0]
                if mutation == "blocker":
                    row["evidenceBlockers"]["runtimeOracle"].pop()
                else:
                    row["sourceExact"] = True
                reseal_row(row)
                reseal_receipt(candidate)
                self.assert_rejected(candidate)

    def test_bool_as_integer_is_rejected(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        row = next(row for row in candidate["renderStatePolicies"] if row["selectedValue"]["type"] == "BOOL")
        row["selectedValue"]["value"] = 0
        reseal_row(row)
        reseal_receipt(candidate)
        self.assert_rejected(candidate)

    def test_nan_and_infinity_are_rejected(self) -> None:
        for value in (float("nan"), float("inf"), float("-inf")):
            with self.subTest(value=value):
                candidate = copy.deepcopy(self.receipt)
                candidate["renderStatePolicies"][0]["numericOracle"]["expectedFloat4"][0] = value
                self.assert_rejected(candidate)

    def test_hlsl_output_row_permutation_is_rejected(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        rows = candidate["hlslVerification"]["rowResults"]
        rows[0], rows[1] = rows[1], rows[0]
        candidate["hlslVerification"]["rowResultsSha256"] = hashlib.sha256(
            json.dumps(rows, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")
        ).hexdigest()
        reseal_receipt(candidate)
        self.assert_rejected(candidate)

    def test_warp_expected_actual_coordinated_mutation_is_rejected(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        row = candidate["warpDescriptorVerification"]["rowResults"][0]
        row["expectedDescriptor"]["DepthEnable"] = False
        row["actualDescriptor"]["DepthEnable"] = False
        reseal_warp_rows(candidate)
        reseal_receipt(candidate)
        self.assert_rejected(candidate)

    def test_warp_stale_digest_coordinated_mutation_is_rejected(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        row = candidate["warpDescriptorVerification"]["rowResults"][0]
        row["expectedDescriptor"]["DepthEnable"] = False
        row["actualDescriptor"]["DepthEnable"] = False
        # Deliberately retain the original stored rowResultsSha256.
        reseal_receipt(candidate)
        self.assert_rejected(candidate)

    def test_warp_descriptor_extra_and_missing_fields_are_rejected(self) -> None:
        for mutation in ("extra", "missing"):
            with self.subTest(mutation=mutation):
                candidate = copy.deepcopy(self.receipt)
                row = candidate["warpDescriptorVerification"]["rowResults"][0]
                if mutation == "extra":
                    row["expectedDescriptor"]["Forged"] = 1
                    row["actualDescriptor"]["Forged"] = 1
                else:
                    row["expectedDescriptor"].pop("DepthFunc")
                    row["actualDescriptor"].pop("DepthFunc")
                reseal_receipt(candidate)
                self.assert_rejected(candidate)

    def test_warp_srv_stale_digest_mutation_is_rejected(self) -> None:
        candidate = copy.deepcopy(self.receipt)
        row = candidate["warpDescriptorVerification"]["srvRowResults"][0]
        row["expectedSrv"]["Format"] = 28
        row["actualSrv"]["Format"] = 28
        reseal_receipt(candidate)
        self.assert_rejected(candidate)

    def test_oracle_top_level_metadata_reseal_mutations_are_rejected(self) -> None:
        mutations = (
            lambda value: value["hlslVerification"].__setitem__("numericTolerance", 123.0),
            lambda value: value["hlslVerification"].__setitem__("maxAbsoluteError", 123.0),
            lambda value: value["warpDescriptorVerification"].__setitem__("numericTolerance", 123.0),
            lambda value: value["hlslVerification"]["compiler"].__setitem__("fileName", "forged.dll"),
            lambda value: value["hlslVerification"].__setitem__("entryPoint", "forged"),
            lambda value: value["hlslVerification"].__setitem__("targetProfile", "cs_4_0"),
            lambda value: value["warpDescriptorVerification"].__setitem__("featureLevel", 0),
            lambda value: value["hlslVerification"].__setitem__("extra", True),
            lambda value: value["warpDescriptorVerification"].__setitem__("extra", True),
        )
        for mutate in mutations:
            with self.subTest(mutate=mutate):
                candidate = copy.deepcopy(self.receipt)
                mutate(candidate)
                reseal_receipt(candidate)
                self.assert_rejected(candidate)

    def test_approval_projection_pins_fidelity_evidence_and_admission(self) -> None:
        mutations = (
            lambda value: value["renderStatePolicies"][0].__setitem__("policyFidelity", "FORGED"),
            lambda value: value["renderStatePolicies"][0]["upstreamEvidence"].__setitem__(
                "runtimeOracleRowSha256", "0" * 64
            ),
            lambda value: value["renderStatePolicies"][0].__setitem__("runtimeConsumerAdmission", True),
            lambda value: value["admission"].__setitem__("product", True),
            lambda value: value["sourceEvidence"]["runtimeOracle"].__setitem__("receiptSha256", "0" * 64),
        )
        for mutate in mutations:
            with self.subTest(mutate=mutate):
                candidate = copy.deepcopy(self.receipt)
                mutate(candidate)
                with self.assertRaises(ValueError):
                    require_approved_receipt(candidate)

    def test_current_texture_filter_candidate_is_compositionally_bound(self) -> None:
        self.assertEqual(
            resolve_current_texture_filter_candidate(self.acquisition)["fields"]["filter"]["property"]["value"],
            "TF_Linear",
        )
        mutations = ("missing", "duplicate", "value", "record", "serial", "admissibility")
        for mutation in mutations:
            with self.subTest(mutation=mutation):
                candidate = copy.deepcopy(self.acquisition)
                rows = candidate["externalArtifactSearch"]["currentRevisionCandidates"]["classDefaultObjects"]
                target = next(row for row in rows if row.get("candidateId") == "current-default-texture")
                if mutation == "missing":
                    rows.remove(target)
                elif mutation == "duplicate":
                    rows.append(copy.deepcopy(target))
                elif mutation == "value":
                    target["fields"]["filter"]["property"]["value"] = "TF_Point"
                elif mutation == "record":
                    target["fields"]["filter"]["property"]["recordSha256"] = "0" * 64
                elif mutation == "serial":
                    target["export"]["serialSha256"] = "0" * 64
                else:
                    target["admissibleAsSourceEra"] = True
                with self.assertRaises(ValueError):
                    resolve_current_texture_filter_candidate(candidate)

    def test_sampler_hlsl_input_uses_primitive_policy_fields(self) -> None:
        payload, expected, row_ids = oracle_input_bytes(self.receipt)
        first_sampler_index = 89 + 94
        header = struct.unpack_from("<4I", payload, first_sampler_index * 128)
        value0 = struct.unpack_from("<4f", payload, first_sampler_index * 128 + 16)
        row = self.receipt["samplerPolicies"][0]
        self.assertEqual(
            header,
            (
                3,
                row["selectedDescriptor"]["filter"]["d3d11"],
                row["selectedDescriptor"]["addressU"]["d3d11"],
                row["selectedDescriptor"]["addressV"]["d3d11"],
            ),
        )
        self.assertEqual(value0, (float(row["selectedDescriptor"]["sRgb"]), 0.0, 0.0, 0.0))
        self.assertNotEqual(list(value0), expected[first_sampler_index])
        self.assertEqual(row_ids[first_sampler_index], row["policyRowId"])

    def test_runtime_renderer_product_promotion_is_rejected(self) -> None:
        for target in ("runtimeConsumer", "rendererConsumer", "product"):
            with self.subTest(target=target):
                candidate = copy.deepcopy(self.receipt)
                if target == "product":
                    candidate["admission"][target] = True
                else:
                    candidate["admission"][target] = {"ready": True, "rowCount": 255}
                reseal_receipt(candidate)
                self.assert_rejected(candidate)

    def test_format_version_bool_and_float_are_rejected(self) -> None:
        for value in (True, 1.0, "1"):
            with self.subTest(value=value):
                candidate = copy.deepcopy(self.receipt)
                candidate["formatVersion"] = value
                reseal_receipt(candidate)
                self.assert_rejected(candidate)

    def test_coordinated_upstream_reseal_is_rejected_by_frozen_pin(self) -> None:
        acquisition = copy.deepcopy(self.acquisition)
        acquisition["matrices"]["renderStateRows"][0]["remainingBlockers"].append("FORGED_BLOCKER")
        acquisition.pop("receiptSha256")
        acquisition["receiptSha256"] = canonical_sha256(acquisition)
        with self.assertRaises(ValueError):
            validate_policy_receipt(
                copy.deepcopy(self.receipt),
                self.runtime,
                acquisition,
                self.contract,
                hlsl_path=DEFAULT_HLSL,
                verifier_path=DEFAULT_VERIFIER,
            )

    def test_actual_cli_rejects_duplicate_keys_and_bom_shallow_and_deep(self) -> None:
        source = DEFAULT_OUTPUT.read_text(encoding="utf-8")
        mutations = {
            "root": source.replace(
                '  "schema":',
                '  "schema": "FORGED",\n  "schema":',
                1,
            ).encode("utf-8"),
            "row": source.replace(
                '      "policyRowId":',
                '      "policyRowId": "FORGED",\n      "policyRowId":',
                1,
            ).encode("utf-8"),
            "descriptor": source.replace(
                '      "selectedDescriptor": {',
                '      "selectedDescriptor": {\n        "type": "FORGED",',
                1,
            ).encode("utf-8"),
            "oracle": source.replace(
                '  "hlslVerification": {',
                '  "hlslVerification": {\n    "backend": "FORGED",',
                1,
            ).encode("utf-8"),
            "bom": b"\xef\xbb\xbf" + source.encode("utf-8"),
        }
        self.assertNotEqual(mutations["root"], source.encode("utf-8"))
        self.assertNotEqual(mutations["row"], source.encode("utf-8"))
        self.assertNotEqual(mutations["descriptor"], source.encode("utf-8"))
        self.assertNotEqual(mutations["oracle"], source.encode("utf-8"))
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            for name, payload in mutations.items():
                path = root / f"{name}.json"
                path.write_bytes(payload)
                for deep in (False, True):
                    with self.subTest(name=name, deep=deep):
                        result = self.run_cli(path, deep=deep)
                        self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
                        failure_text = result.stdout + result.stderr
                        if name == "bom":
                            self.assertIn("JSON must be UTF-8 without BOM", failure_text)
                        else:
                            self.assertIn("duplicate JSON key", failure_text)

    def test_actual_cli_lf_crlf_receipt_parity_shallow_and_deep(self) -> None:
        source = DEFAULT_OUTPUT.read_bytes().replace(b"\r\n", b"\n").replace(b"\r", b"\n")
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            paths = {
                "lf": root / "lf.json",
                "crlf": root / "crlf.json",
            }
            paths["lf"].write_bytes(source)
            paths["crlf"].write_bytes(source.replace(b"\n", b"\r\n"))
            for name, path in paths.items():
                for deep in (False, True):
                    with self.subTest(name=name, deep=deep):
                        result = self.run_cli(path, deep=deep)
                        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_direct_import_closure_is_pinned_and_actual_hash_mutation_rejected(self) -> None:
        evidence = self.receipt["sourceEvidence"]["directImportClosure"]
        self.assertEqual(evidence, direct_import_closure())

        candidate = copy.deepcopy(self.receipt)
        row = candidate["sourceEvidence"]["directImportClosure"]["dependencies"][0]
        row["canonicalTextSha256"] = "0" * 64
        candidate["sourceEvidence"]["directImportClosure"]["projectionSha256"] = canonical_sha256(
            candidate["sourceEvidence"]["directImportClosure"]["dependencies"]
        )
        reseal_receipt(candidate)
        self.assert_rejected(candidate)

        dependency_id = "RUNTIME_WARP_SUPPORT"
        original = DIRECT_IMPORT_DEPENDENCY_PATHS[dependency_id].read_bytes()
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            crlf = root / "crlf.py"
            crlf.write_bytes(original.replace(b"\r\n", b"\n").replace(b"\r", b"\n").replace(b"\n", b"\r\n"))
            crlf_paths = dict(DIRECT_IMPORT_DEPENDENCY_PATHS)
            crlf_paths[dependency_id] = crlf
            validate_direct_import_closure(evidence, crlf_paths)

            mutated = root / "mutated.py"
            mutated.write_bytes(original + b"\nDEPENDENCY_MUTATION = True\n")
            mutated_paths = dict(DIRECT_IMPORT_DEPENDENCY_PATHS)
            mutated_paths[dependency_id] = mutated
            with self.assertRaises(ValueError):
                validate_direct_import_closure(evidence, mutated_paths)


if __name__ == "__main__":
    unittest.main(verbosity=2)
