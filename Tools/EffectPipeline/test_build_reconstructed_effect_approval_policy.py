from __future__ import annotations

import copy
import subprocess
import sys
import tempfile
import unittest
from collections import Counter
from pathlib import Path
from typing import Any, Callable

sys.path.insert(0, str(Path(__file__).resolve().parent))

import build_reconstructed_effect_approval_policy as builder


ROOT = Path(__file__).resolve().parents[2]
POLICY_SOURCE = ROOT / builder.POLICY_SOURCE_PATH
POLICY_SCHEMA = ROOT / builder.POLICY_SCHEMA_PATH
RECEIPT = ROOT / builder.OUTPUT_PATH


class ReconstructedEffectApprovalPolicyTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.policy_source = builder.load_strict_json_object(POLICY_SOURCE)
        cls.policy_schema = builder.load_strict_json_object(POLICY_SCHEMA)
        cls.receipt = builder.load_strict_json_object(RECEIPT)
        cls.expected = builder.build_receipt(
            ROOT, cls.policy_source, cls.policy_schema
        )
        _, cls.upstream = builder.verify_frozen_inputs(
            ROOT, cls.policy_source["frozenInputs"]
        )

    @staticmethod
    def reseal(receipt: dict[str, Any]) -> None:
        unsigned = copy.deepcopy(receipt)
        unsigned.pop("receiptSha256", None)
        receipt["receiptSha256"] = builder.canonical_sha256(unsigned)

    def assert_receipt_mutation_rejected(
        self,
        mutation: Callable[[dict[str, Any]], None],
        pattern: str = "differs|self-hash|schema|formatVersion",
        *,
        reseal: bool = True,
    ) -> None:
        changed = copy.deepcopy(self.receipt)
        mutation(changed)
        if reseal:
            self.reseal(changed)
        with self.assertRaisesRegex(ValueError, pattern):
            builder.validate_receipt_against_expected(changed, self.expected)

    def assert_build_source_mutation_rejected(
        self,
        mutation: Callable[[dict[str, Any]], None],
        pattern: str = "V1|changed|differs|policy",
    ) -> None:
        changed = copy.deepcopy(self.policy_source)
        mutation(changed)
        with self.assertRaisesRegex(ValueError, pattern):
            builder.build_receipt(ROOT, changed, self.policy_schema)

    def test_baseline_is_deterministic_and_fail_closed(self) -> None:
        builder.validate_receipt_against_expected(self.receipt, self.expected)
        self.assertEqual(self.receipt, self.expected)
        admission = self.receipt["admissionPolicy"]
        self.assertTrue(admission["policyRouteApproved"])
        self.assertFalse(admission["sourceExactAdmission"])
        self.assertFalse(admission["executionAdmission"])
        self.assertFalse(admission["productAdmission"])

    def test_exact_frozen_lane_identities_are_joined(self) -> None:
        rows = {row["laneId"]: row for row in self.receipt["frozenInputs"]}
        self.assertEqual(set(rows), builder.EXPECTED_LANES)
        self.assertEqual(
            rows["SOURCE"]["commitId"],
            "7da937aeaa34c088c694e8eb4f53ff1f7f848ef3",
        )
        self.assertEqual(
            rows["MATERIAL"]["commitId"],
            "3ba493de5fde8d058ddee7e0fa0e6c3e466faa43",
        )
        self.assertEqual(
            rows["GEOMETRY"]["commitId"],
            "0aca792819fdda3f541bb7cec7451c5ed93c6467",
        )
        self.assertEqual(
            rows["RUNTIME_FOUNDATION"]["commitId"],
            "38ebe7cf7dceb5054bde93812907173cc0f98c67",
        )

    def test_source_29_rows_are_owned_by_seven_families(self) -> None:
        rows = self.receipt["sourceRows"]
        families = self.receipt["sourceExecutionFamilies"]
        self.assertEqual(len(rows), 29)
        self.assertEqual(len(families), 7)
        self.assertEqual(len({row["moduleOccurrenceId"] for row in rows}), 29)
        self.assertEqual(sum(row["moduleOccurrenceCount"] for row in families), 29)
        self.assertTrue(all(not row["sourceExact"] for row in rows))
        self.assertTrue(all(not row["executionAdmission"] for row in rows))

    def test_material_255_rows_have_corrected_denominators(self) -> None:
        rows = self.receipt["materialRows"]
        counts = Counter(row["domain"] for row in rows)
        self.assertEqual(len(rows), 260)
        self.assertEqual(counts, {
            "RENDER_STATE": 89,
            "STATIC_PERMUTATION": 94,
            "SAMPLER": 77,
        })
        self.assertEqual(len({row["upstreamMatrixRowId"] for row in rows}), 260)
        self.assertTrue(all(not row["sourceExact"] for row in rows))
        self.assertTrue(all(not row["executionAdmission"] for row in rows))

    def test_material_rows_route_to_minimal_evidence_families(self) -> None:
        counts = Counter(row["policyFamilyId"] for row in self.receipt["materialRows"])
        self.assertEqual(counts["RECONSTRUCTED_APPROVED_CURRENT_CDO_RENDER_STATE_V1"], 25)
        self.assertEqual(counts["RECONSTRUCTED_APPROVED_ARTIST_RENDER_POLICY_V1"], 64)
        self.assertEqual(counts["RECONSTRUCTED_APPROVED_STATIC_EXPLICIT_SELECTION_V1"], 23)
        self.assertEqual(counts["RECONSTRUCTED_APPROVED_STATIC_NONOVERRIDE_SELECTION_V1"], 43)
        self.assertEqual(counts["RECONSTRUCTED_APPROVED_PARENT_DEFAULT_SELECTION_V1"], 28)
        self.assertEqual(counts["RECONSTRUCTED_APPROVED_CURRENT_TEXTURE2D_EVIDENCE_V1"], 74)
        self.assertEqual(counts["RECONSTRUCTED_APPROVED_ROLE_SAMPLER_POLICY_V1"], 3)

    def test_four_former_exact_samplers_are_reclassified_one_plus_three(self) -> None:
        all_sampler_rows = [
            row for row in self.receipt["materialRows"] if row["domain"] == "SAMPLER"
        ]
        rows = [
            row for row in all_sampler_rows
            if row["previousSamplerAdmission"] == "SOURCE_EXACT_SAMPLER"
        ]
        self.assertEqual(len(all_sampler_rows), 77)
        self.assertTrue(all(not row["fullDescriptorSourceExact"]
                            for row in all_sampler_rows))
        self.assertTrue(all(row["evidenceFidelity"] != "SOURCE_EXACT_SAMPLER"
                            for row in all_sampler_rows))
        counts = Counter(row["evidenceFidelity"] for row in rows)
        self.assertEqual(len(rows), 4)
        self.assertEqual(counts, {
            "SOURCE_EXACT_TEXTURE_BINDING_PARTIAL_SAMPLER_TAGS": 1,
            "SOURCE_EXACT_TEXTURE_BINDING_SAMPLER_DEFAULT_UNPROVEN": 3,
        })
        self.assertTrue(all(not row["fullDescriptorSourceExact"] for row in rows))
        self.assertEqual(self.receipt["summary"]["samplerFullSourceExactCount"], 0)
        self.assertEqual(self.receipt["summary"]["forbiddenFullFidelityLabelCount"], 0)

    def test_arithmetic_23_preserves_numeric_proof_and_source_blockers(self) -> None:
        rows = self.receipt["materialArithmeticRows"]
        self.assertEqual(len(rows), 23)
        self.assertTrue(all(row["cpuNumericOracleVerified"] for row in rows))
        self.assertTrue(all(row["hlslNumericOracleVerified"] for row in rows))
        self.assertTrue(all(not row["sourceExact"] for row in rows))
        self.assertTrue(all(row["preservedEvidenceBlockers"] for row in rows))
        self.assertTrue(all(not row["executionAdmission"] for row in rows))

    def test_geometry_seven_is_dependency_only(self) -> None:
        rows = self.receipt["geometryRows"]
        self.assertEqual(len(rows), 7)
        self.assertEqual(len({row["assetId"] for row in rows}), 7)
        self.assertTrue(all(row["geometryPreScale"] == 0.01 for row in rows))
        self.assertTrue(all(not row["sourceExact"] for row in rows))
        self.assertTrue(all(not row["executionAdmission"] for row in rows))

    def test_manual_validation_requires_human_eye_without_capture_oracle(self) -> None:
        manual = self.receipt["manualValidation"]
        self.assertEqual(manual["requiredOccurrenceCount"], 35)
        self.assertEqual(manual["completedOccurrenceCount"], 0)
        self.assertFalse(manual["automatedScreenshotOrImageOracleAllowed"])
        self.assertFalse(manual["captureArtifactRequired"])
        self.assertEqual(manual["status"], "NOT_STARTED")

    def test_policy_source_format_version_rejects_bool_float_and_string(self) -> None:
        for invalid in (True, 1.0, "1"):
            with self.subTest(invalid=invalid):
                changed = copy.deepcopy(self.policy_source)
                changed["formatVersion"] = invalid
                with self.assertRaisesRegex(ValueError, "integer 1"):
                    builder.validate_policy_source(changed)

    def test_policy_source_cannot_grant_source_execution_or_product(self) -> None:
        fields = (
            "sourceFidelityPromotionAllowed",
            "runtimeExecutionGranted",
            "productAdmissionGranted",
        )
        for field in fields:
            with self.subTest(field=field):
                changed = copy.deepcopy(self.policy_source)
                changed["approvalDecision"][field] = True
                with self.assertRaisesRegex(ValueError, "must be False"):
                    builder.validate_policy_source(changed)

    def test_policy_source_cannot_restore_old_sampler_denominator(self) -> None:
        changed = copy.deepcopy(self.policy_source)
        changed["denominators"]["materialSamplerRowCount"] = 68
        changed["denominators"]["materialExecutionRowCount"] = 251
        with self.assertRaisesRegex(ValueError, "V1 denominators changed"):
            builder.build_receipt(ROOT, changed, self.policy_schema)

    def test_actual_build_rejects_effect_occurrence_denominator_trivialization(self) -> None:
        self.assert_build_source_mutation_rejected(
            lambda row: row["denominators"].__setitem__("effectOccurrenceCount", 1),
            "V1 denominators changed",
        )

    def test_actual_build_rejects_policy_and_target_aliases(self) -> None:
        mutations = (
            lambda row: row.__setitem__("policyId", "artist.31470.f.alias-v1"),
            lambda row: row["target"].__setitem__("characterClass", "WARLORD"),
            lambda row: row["target"].__setitem__("skillId", 31471),
            lambda row: row["target"].__setitem__("inputSlot", "Q"),
            lambda row: row["target"].__setitem__(
                "effectAssetId", "effect.artist.skill.31470.alias"
            ),
        )
        for index, mutation in enumerate(mutations):
            with self.subTest(index=index):
                self.assert_build_source_mutation_rejected(mutation, "V1 policy id|V1 target")

    def test_actual_build_rejects_global_gate_and_rollback_trivialization(self) -> None:
        mutations = (
            (
                lambda row: row.__setitem__(
                    "requiredGlobalGates",
                    ["DEBUG_RELEASE_BUILD_HARNESS_AND_FOCUSED_PROJECT_AUDIT_PASS"],
                ),
                "V1 global admission gates changed",
            ),
            (
                lambda row: row.__setitem__(
                    "rollbackConditions", ["PARTIAL_OR_STALE_PRODUCT_STAGE"]
                ),
                "V1 rollback contract changed",
            ),
        )
        for mutation, pattern in mutations:
            with self.subTest(pattern=pattern):
                self.assert_build_source_mutation_rejected(mutation, pattern)

    def test_actual_build_rejects_coordinated_source_family_swap(self) -> None:
        def mutate(row: dict[str, Any]) -> None:
            first = row["sourceFamilyPolicies"][0]
            second = row["sourceFamilyPolicies"][1]
            first["policyFamilyId"], second["policyFamilyId"] = (
                second["policyFamilyId"], first["policyFamilyId"]
            )
            first["requiredOracleIds"], second["requiredOracleIds"] = (
                second["requiredOracleIds"], first["requiredOracleIds"]
            )

        self.assert_build_source_mutation_rejected(mutate, "Source family/oracle mapping changed")

    def test_actual_build_rejects_coordinated_render_rule_swap(self) -> None:
        def mutate(row: dict[str, Any]) -> None:
            first = row["materialRowRules"][0]
            second = row["materialRowRules"][1]
            first["policyFamilyId"], second["policyFamilyId"] = (
                second["policyFamilyId"], first["policyFamilyId"]
            )
            first["evidenceFidelity"], second["evidenceFidelity"] = (
                second["evidenceFidelity"], first["evidenceFidelity"]
            )

        self.assert_build_source_mutation_rejected(
            mutate, "V1 Material rule/fidelity mapping changed: RENDER_STATE"
        )

    def test_actual_build_rejects_real_descendant_frozen_commit(self) -> None:
        def mutate(row: dict[str, Any]) -> None:
            lane = next(item for item in row["frozenInputs"]
                        if item["laneId"] == "RUNTIME_FOUNDATION")
            lane["commitId"] = "e303dbd0bc6002e80c9a6fb09e0d6633d17434d0"
            lane["treeId"] = "20f338059edb79f2679fc460030d9a2f4c585bb4"
            blobs = {
                "Client/Public/Effect_RuntimeAuthority.h":
                    "62aefde63475999c955cbd37fc61ab0a1dc96002",
                "Client/Private/Effect_RuntimeAuthority.cpp":
                    "d003a41caf29c0b65c4739ae1257ee75f46cfb85",
                "Client/Public/Effect_Catalog.h":
                    "4c9e9ce42e064fab445ac05e76e697fa50e4b8fe",
                "Client/Private/Effect_Catalog.cpp":
                    "fc76075c8c7d80e5e18719d68c789d5430c390d8",
                "Tools/ProjectAudit/Test-EffectRuntimeAuthority.ps1":
                    "b8e06f79b95f1e05d42e23bdb5dcfc87979813cc",
            }
            for artifact in lane["requiredSourceArtifacts"]:
                artifact["blobId"] = blobs[artifact["path"]]

        self.assert_build_source_mutation_rejected(
            mutate, "frozen lane commit/tree/path/blob table changed"
        )

    def test_actual_build_rejects_forbidden_sampler_fidelity_promotion(self) -> None:
        def mutate(row: dict[str, Any]) -> None:
            rule = next(
                item for item in row["materialRowRules"]
                if item["evidenceFidelity"] ==
                "SOURCE_EXACT_TEXTURE_BINDING_PARTIAL_SAMPLER_TAGS"
            )
            rule["evidenceFidelity"] = "SOURCE_EXACT_SAMPLER"

        self.assert_build_source_mutation_rejected(
            mutate, "V1 Material rule/fidelity mapping changed: SAMPLER"
        )

    def test_actual_build_rejects_nested_schema_weakening_and_keyword_injection(self) -> None:
        weakened = copy.deepcopy(self.policy_schema)
        weakened["$defs"]["materialRow"]["additionalProperties"] = True
        with self.assertRaisesRegex(ValueError, "reject additional properties"):
            builder.build_receipt(ROOT, self.policy_source, weakened)
        missing_required = copy.deepcopy(self.policy_schema)
        missing_required["$defs"]["materialRow"]["required"].remove("sourceExact")
        with self.assertRaisesRegex(ValueError, "required/properties differ"):
            builder.build_receipt(ROOT, self.policy_source, missing_required)
        injected = copy.deepcopy(self.policy_schema)
        injected["$defs"]["materialRow"]["properties"]["sourceExact"]["default"] = False
        with self.assertRaisesRegex(ValueError, "unsupported or weakening"):
            builder.build_receipt(ROOT, self.policy_source, injected)

    def test_policy_row_binding_covers_family_and_policy_semantics(self) -> None:
        source_receipt = self.upstream["lostark.effect-source-oracle-acquisition"]
        baseline_rows, _ = builder.build_source_rows(
            source_receipt,
            self.policy_source["sourceFamilyPolicies"],
            29,
            builder.V1_POLICY_SOURCE_CANONICAL_SHA256,
            builder.V1_POLICY_SCHEMA_CANONICAL_SHA256,
        )
        changed_rows, _ = builder.build_source_rows(
            source_receipt,
            self.policy_source["sourceFamilyPolicies"],
            29,
            "0" * 64,
            builder.V1_POLICY_SCHEMA_CANONICAL_SHA256,
        )
        self.assertNotEqual(
            [row["policyBindingSha256"] for row in baseline_rows],
            [row["policyBindingSha256"] for row in changed_rows],
        )
        self.assertNotEqual(
            [row["policyRowId"] for row in baseline_rows],
            [row["policyRowId"] for row in changed_rows],
        )
        material_receipt = self.upstream[
            "lostark.artist-31470-material-source-value-acquisition-receipt"
        ]
        baseline_material, _ = builder.build_material_rows(
            material_receipt,
            self.policy_source["materialExecutionFamilies"],
            self.policy_source["materialRowRules"],
            self.policy_source["denominators"],
            builder.V1_POLICY_SOURCE_CANONICAL_SHA256,
            builder.V1_POLICY_SCHEMA_CANONICAL_SHA256,
        )
        changed_material, _ = builder.build_material_rows(
            material_receipt,
            self.policy_source["materialExecutionFamilies"],
            self.policy_source["materialRowRules"],
            self.policy_source["denominators"],
            "0" * 64,
            builder.V1_POLICY_SCHEMA_CANONICAL_SHA256,
        )
        self.assertNotEqual(
            [row["policyBindingSha256"] for row in baseline_material],
            [row["policyBindingSha256"] for row in changed_material],
        )

    def test_policy_source_cannot_drop_source_exact_forbidden_claim(self) -> None:
        changed = copy.deepcopy(self.policy_source)
        changed["forbiddenClaims"].remove("SOURCE_EXACT")
        with self.assertRaisesRegex(ValueError, "SOURCE_EXACT must be forbidden"):
            builder.validate_policy_source(changed)

    def test_frozen_commit_tree_blob_and_receipt_hash_mutations_fail(self) -> None:
        mutations = (
            lambda row: row["frozenInputs"][0].__setitem__("commitId", "0" * 40),
            lambda row: row["frozenInputs"][0].__setitem__("treeId", "0" * 40),
            lambda row: row["frozenInputs"][0]["receiptArtifacts"][0].__setitem__(
                "blobId", "0" * 40
            ),
            lambda row: row["frozenInputs"][0]["receiptArtifacts"][0].__setitem__(
                "receiptSha256", "0" * 64
            ),
        )
        for index, mutation in enumerate(mutations):
            with self.subTest(index=index):
                changed = copy.deepcopy(self.policy_source)
                mutation(changed)
                with self.assertRaises(ValueError):
                    builder.build_receipt(ROOT, changed, self.policy_schema)

    def test_material_rule_ambiguity_and_gap_are_rejected(self) -> None:
        acquisition = self.upstream[
            "lostark.artist-31470-material-source-value-acquisition-receipt"
        ]
        duplicated = copy.deepcopy(self.policy_source["materialRowRules"])
        duplicated.append(copy.deepcopy(duplicated[0]))
        with self.assertRaisesRegex(ValueError, "exactly one"):
            builder.build_material_rows(
                acquisition,
                self.policy_source["materialExecutionFamilies"],
                duplicated,
                self.policy_source["denominators"],
                builder.V1_POLICY_SOURCE_CANONICAL_SHA256,
                builder.V1_POLICY_SCHEMA_CANONICAL_SHA256,
            )
        missing = copy.deepcopy(self.policy_source["materialRowRules"])
        missing.pop(0)
        with self.assertRaisesRegex(ValueError, "exactly one"):
            builder.build_material_rows(
                acquisition,
                self.policy_source["materialExecutionFamilies"],
                missing,
                self.policy_source["denominators"],
                builder.V1_POLICY_SOURCE_CANONICAL_SHA256,
                builder.V1_POLICY_SCHEMA_CANONICAL_SHA256,
            )

    def test_source_row_cannot_be_removed(self) -> None:
        self.assert_receipt_mutation_rejected(lambda row: row["sourceRows"].pop())

    def test_material_row_cannot_be_removed(self) -> None:
        self.assert_receipt_mutation_rejected(lambda row: row["materialRows"].pop())

    def test_source_evidence_blocker_cannot_be_removed(self) -> None:
        self.assert_receipt_mutation_rejected(
            lambda row: row["sourceRows"][0]["preservedEvidenceBlockers"].clear()
        )

    def test_source_family_cannot_be_reassigned(self) -> None:
        replacement = next(
            row["policyFamilyId"] for row in self.receipt["sourceRows"]
            if row["policyFamilyId"] != self.receipt["sourceRows"][0]["policyFamilyId"]
        )
        self.assert_receipt_mutation_rejected(
            lambda row: row["sourceRows"][0].__setitem__(
                "policyFamilyId", replacement
            )
        )

    def test_material_recipe_and_occurrence_ownership_cannot_move(self) -> None:
        def mutate(row: dict[str, Any]) -> None:
            row["materialRows"][0]["materialRecipeId"] = row["materialRows"][1][
                "materialRecipeId"
            ]
            row["materialRows"][0]["materialOccurrenceIds"] = copy.deepcopy(
                row["materialRows"][1]["materialOccurrenceIds"]
            )

        self.assert_receipt_mutation_rejected(mutate)

    def test_former_sampler_cannot_be_laundered_to_source_exact(self) -> None:
        index = next(
            i for i, row in enumerate(self.receipt["materialRows"])
            if row["previousSamplerAdmission"] == "SOURCE_EXACT_SAMPLER"
        )
        self.assert_receipt_mutation_rejected(
            lambda row: row["materialRows"][index].__setitem__(
                "evidenceFidelity", "SOURCE_EXACT_SAMPLER"
            )
        )

    def test_source_exact_flags_cannot_be_opened(self) -> None:
        mutations = (
            lambda row: row["sourceRows"][0].__setitem__("sourceExact", True),
            lambda row: row["materialRows"][0].__setitem__("sourceExact", True),
            lambda row: row["materialArithmeticRows"][0].__setitem__("sourceExact", True),
            lambda row: row["geometryRows"][0].__setitem__("sourceExact", True),
            lambda row: row["fidelityPolicy"].__setitem__("sourceExactAdmission", True),
        )
        for index, mutation in enumerate(mutations):
            with self.subTest(index=index):
                self.assert_receipt_mutation_rejected(mutation)

    def test_execution_and_product_flags_cannot_be_opened(self) -> None:
        mutations = (
            lambda row: row["sourceRows"][0].__setitem__("executionAdmission", True),
            lambda row: row["materialRows"][0].__setitem__("executionAdmission", True),
            lambda row: row["admissionPolicy"].__setitem__("executionAdmission", True),
            lambda row: row["admissionPolicy"].__setitem__("productAdmission", True),
            lambda row: row["summary"].__setitem__("productAdmissionCount", 35),
        )
        for index, mutation in enumerate(mutations):
            with self.subTest(index=index):
                self.assert_receipt_mutation_rejected(mutation)

    def test_geometry_prescale_cannot_be_changed(self) -> None:
        self.assert_receipt_mutation_rejected(
            lambda row: row["geometryRows"][0].__setitem__("geometryPreScale", 1.0)
        )

    def test_manual_validation_cannot_be_fabricated(self) -> None:
        def mutate(row: dict[str, Any]) -> None:
            row["manualValidation"]["completedOccurrenceCount"] = 35
            row["manualValidation"]["status"] = "PASS"
            row["summary"]["manualValidationCount"] = 35

        self.assert_receipt_mutation_rejected(mutate)

    def test_frozen_input_identity_cannot_be_resealed_in_receipt(self) -> None:
        self.assert_receipt_mutation_rejected(
            lambda row: row["frozenInputs"][0].__setitem__("commitId", "0" * 40)
        )

    def test_receipt_self_hash_mutation_fails_before_snapshot_compare(self) -> None:
        self.assert_receipt_mutation_rejected(
            lambda row: row["summary"].__setitem__("sourceExecutionRowCount", 28),
            pattern="self-hash",
            reseal=False,
        )

    def test_tracked_lf_crlf_output_comparison_is_stable(self) -> None:
        payload = builder.pretty_json_bytes(self.receipt)
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "receipt.json"
            path.write_bytes(payload.replace(b"\n", b"\r\n"))
            self.assertTrue(builder.generated_matches(path, payload))
            path.write_bytes(payload.replace(b"sourceExactCount\": 0", b"sourceExactCount\": 1"))
            self.assertFalse(builder.generated_matches(path, payload))

    def test_duplicate_json_key_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "duplicate JSON key"):
            builder.parse_strict_json_bytes(b'{"formatVersion":1,"formatVersion":1}', "fixture")

    def test_schema_root_mutation_is_rejected(self) -> None:
        changed = copy.deepcopy(self.policy_schema)
        changed["additionalProperties"] = True
        with self.assertRaisesRegex(ValueError, "reject additional"):
            builder.validate_policy_schema_document(changed)

    def test_checked_receipt_conforms_to_closed_json_schema_contract(self) -> None:
        builder.validate_policy_schema_document(self.policy_schema)
        self.assertEqual(
            set(self.policy_schema["required"]),
            set(self.policy_schema["properties"]),
        )
        self.assertEqual(
            set(self.receipt),
            set(self.policy_schema["required"]),
        )
        try:
            import jsonschema
        except ImportError:
            jsonschema = None
        if jsonschema is not None:
            jsonschema.Draft202012Validator.check_schema(self.policy_schema)
            jsonschema.validate(self.receipt, self.policy_schema)

    def test_actual_cli_check_passes(self) -> None:
        completed = subprocess.run(
            [
                "python",
                "-B",
                str(ROOT / "Tools/EffectPipeline/build_reconstructed_effect_approval_policy.py"),
                "--check",
            ],
            cwd=ROOT,
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        self.assertEqual(completed.returncode, 0, completed.stdout)
        self.assertIn("source=29 material=260 sampler=77 geometry=7", completed.stdout)


if __name__ == "__main__":
    unittest.main()
