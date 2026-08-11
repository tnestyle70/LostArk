#!/usr/bin/env python3
from __future__ import annotations

import copy
import math
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from build_artist_31470_reconstructed_source_capability import (
    ACQUISITION_PATH,
    CLASS_CONTRACTS,
    CUSTOM_PATH,
    DECAL_SOURCE_CLASS,
    EVIDENCE_BLOCKERS,
    EXPECTED_DECAL_NEAR_PLANE,
    EXPECTED_FAMILY_COUNTS,
    IMPLEMENTATION_VERSIONS,
    OUTPUT_PATH,
    SOURCE_PATH,
    SOURCE_RECEIPT_PATH,
    build_family_policies,
    build_occurrence_rows,
    build_receipt,
    canonical_sha256,
    json_bytes,
    tracked_json_sha256,
    validate_receipt,
)
from effect_source_contract_io import load_strict_json_object


class ReconstructedSourceCapabilityTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.root = Path(__file__).resolve().parents[2]
        cls.source = load_strict_json_object(cls.root / SOURCE_PATH)
        cls.custom = load_strict_json_object(cls.root / CUSTOM_PATH)
        cls.acquisition = load_strict_json_object(cls.root / ACQUISITION_PATH)
        cls.source_receipt = load_strict_json_object(cls.root / SOURCE_RECEIPT_PATH)
        cls.receipt = build_receipt(
            cls.root, cls.source, cls.custom, cls.acquisition, cls.source_receipt
        )

    def _reseal(self, value: dict) -> None:
        value.pop("receiptSha256", None)
        value["receiptSha256"] = canonical_sha256(value)

    def _assert_invalid(self, value: dict) -> None:
        self._reseal(value)
        with self.assertRaises(ValueError):
            validate_receipt(
                value,
                self.root,
                self.source,
                self.custom,
                self.acquisition,
                self.source_receipt,
            )

    @staticmethod
    def _blocked_module(source: dict, source_class: str) -> tuple[dict, dict]:
        for occurrence in source["occurrences"]:
            for module in occurrence["modules"]:
                if module["decision"] == "BLOCKED" and module["exactSourceClass"] == source_class:
                    return occurrence, module
        raise AssertionError(f"blocked module not found: {source_class}")

    @staticmethod
    def _descriptor(module: dict, property_path: str) -> dict:
        matches = [
            row["descriptor"] for row in module["typedPayload"]["distributions"]
            if row["descriptor"]["propertyPath"] == property_path
        ]
        if len(matches) != 1:
            raise AssertionError(f"descriptor not unique: {property_path}")
        return matches[0]

    @staticmethod
    def _adapter(module: dict, property_path: str) -> dict:
        matches = [
            row for row in module["distributionAdapters"]
            if row.get("distributionId", "").endswith("::distribution:" + property_path)
        ]
        if len(matches) != 1:
            raise AssertionError(f"adapter not unique: {property_path}")
        return matches[0]

    @staticmethod
    def _output_for(rows: list[dict], module_id: str) -> list[dict]:
        matches = [row for row in rows if row["moduleOccurrenceId"] == module_id]
        if len(matches) != 1:
            raise AssertionError(f"row not unique: {module_id}")
        return [sample["output"] for sample in matches[0]["numericSamples"]]

    def test_01_denominators_and_family_mapping_are_exact(self) -> None:
        summary = self.receipt["summary"]
        self.assertEqual(summary["policyFamilyCount"], 7)
        self.assertEqual(summary["moduleOccurrenceCount"], 29)
        self.assertEqual(summary["familyOccurrenceCounts"], EXPECTED_FAMILY_COUNTS)
        self.assertEqual(summary["numericSampleCount"], 87)
        blocked_ids = {
            module["moduleOccurrenceId"]
            for occurrence in self.source["occurrences"]
            for module in occurrence["modules"]
            if module["decision"] == "BLOCKED"
        }
        self.assertEqual(
            {row["moduleOccurrenceId"] for row in self.receipt["occurrences"]},
            blocked_ids,
        )

    def test_02_family_specs_have_closed_variants_and_implementation_hash(self) -> None:
        policies = build_family_policies(self.root, self.source)
        self.assertEqual(len(policies), 7)
        owned_variants = set()
        for policy in policies:
            self.assertIs(type(policy["implementationVersion"]), int)
            self.assertEqual(
                policy["implementationVersion"],
                IMPLEMENTATION_VERSIONS[policy["policyFamilyId"]],
            )
            self.assertEqual(len(policy["implementationSha256"]), 64)
            self.assertEqual(
                canonical_sha256(policy["semanticContract"]),
                policy["familySemanticImplementationSha256"],
            )
            self.assertFalse(policy["semanticContract"]["genericFallbackAllowed"])
            for binding in policy["semanticContract"]["variantBindings"]:
                self.assertEqual(binding["inputSchema"]["type"], "OBJECT")
                self.assertFalse(binding["inputSchema"]["additionalFieldsAllowed"])
                self.assertEqual(binding["outputSchema"]["type"], "OBJECT")
                self.assertFalse(binding["outputSchema"]["additionalFieldsAllowed"])
                owned_variants.add(binding["variant"])
        self.assertEqual(owned_variants, {row["variant"] for row in CLASS_CONTRACTS.values()})

    def test_03_all_ready_rows_have_three_finite_samples(self) -> None:
        for row in self.receipt["occurrences"]:
            self.assertEqual(row["capabilityDecision"], "READY_FOR_RECONSTRUCTED_REVIEW")
            self.assertEqual(len(row["numericSamples"]), 3)
            for sample in row["numericSamples"]:
                self.assertEqual(len(sample["randomUnits"]), 4)
                self.assertTrue(all(math.isfinite(value) for value in sample["randomUnits"]))
                self.assertEqual(canonical_sha256(sample["typedInputs"]), sample["typedInputSha256"])
                self.assertEqual(canonical_sha256(sample["output"]), sample["outputSha256"])

    def test_04_current_evidence_is_never_promoted(self) -> None:
        for row in self.receipt["occurrences"]:
            self.assertEqual(
                row["sourceEvidenceFidelity"],
                "CURRENT_REVISION_CROSS_REVISION_EVIDENCE",
            )
            self.assertFalse(row["sourceExact"])
            self.assertFalse(row["currentEvidencePromotedToSourceExact"])
            self.assertTrue(set(EVIDENCE_BLOCKERS).issubset(row["preservedEvidenceBlockers"]))
        self.assertEqual(self.receipt["summary"]["sourceExactCount"], 0)
        self.assertEqual(self.receipt["summary"]["currentEvidencePromotedCount"], 0)

    def test_05_upstream_custom_and_acquisition_blockers_are_preserved(self) -> None:
        union = set(self.receipt["blockerUnion"])
        self.assertIn("EXACT_NATIVE_PARTICLE_OUTPUT_ORACLE_REQUIRED", union)
        self.assertIn("SOURCE_ERA_NATIVE_HANDLER_IDENTITY_UNPINNED", union)
        self.assertIn("EXACT_NATIVE_DISTRIBUTION_OUTPUT_ORACLE_REQUIRED", union)
        self.assertIn("SOURCE_ERA_DISTRIBUTION_EVALUATOR_IDENTITY_UNPINNED", union)
        self.assertIn("SOURCE_ERA_ACTUAL_OUTPUT_PROVIDER_NOT_ACQUIRED", union)
        self.assertIn("SOURCE_29_EXECUTION_READINESS_BLOCKED", union)

    def test_06_execution_and_product_remain_false(self) -> None:
        self.assertFalse(self.receipt["runtimeExecutionAdmission"]["allowed"])
        self.assertFalse(self.receipt["productAdmission"]["allowed"])
        self.assertEqual(self.receipt["summary"]["runtimeExecutionAdmissionCount"], 0)
        self.assertEqual(self.receipt["summary"]["productAdmissionCount"], 0)
        self.assertTrue(all(not row["runtimeExecutionAdmission"] for row in self.receipt["occurrences"]))
        self.assertTrue(all(not row["productAdmission"] for row in self.receipt["occurrences"]))

    def test_07_build_is_deterministic(self) -> None:
        rebuilt = build_receipt(
            self.root, self.source, self.custom, self.acquisition, self.source_receipt
        )
        self.assertEqual(json_bytes(self.receipt), json_bytes(rebuilt))

    def test_08_strict_loader_rejects_duplicate_keys(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "duplicate.json"
            path.write_bytes(b'{"schema":"x","schema":"y"}\n')
            with self.assertRaises(ValueError):
                load_strict_json_object(path)

    def test_09_exact_json_integer_version_is_required(self) -> None:
        mutated = copy.deepcopy(self.receipt)
        mutated["formatVersion"] = True
        self._assert_invalid(mutated)
        mutated = copy.deepcopy(self.receipt)
        mutated["formatVersion"] = 1.0
        self._assert_invalid(mutated)

    def test_10_frozen_raw_and_self_identity_mutation_is_rejected(self) -> None:
        mutated = copy.deepcopy(self.receipt)
        mutated["frozenSourceInputs"][0]["canonicalTextSha256"] = "00" * 32
        self._assert_invalid(mutated)
        mutated = copy.deepcopy(self.receipt)
        mutated["frozenSourceInputs"][1]["selfSha256"] = "11" * 32
        self._assert_invalid(mutated)

    def test_11_tool_and_implementation_hash_mutation_is_rejected(self) -> None:
        mutated = copy.deepcopy(self.receipt)
        mutated["toolIdentity"]["canonicalTextSha256"] = "22" * 32
        self._assert_invalid(mutated)
        mutated = copy.deepcopy(self.receipt)
        mutated["familyPolicies"][0]["semanticContract"]["algorithm"] += "_FORGED"
        mutated["familyPolicies"][0]["familySemanticImplementationSha256"] = canonical_sha256(
            mutated["familyPolicies"][0]["semanticContract"]
        )
        implementation = {
            "implementationId": mutated["familyPolicies"][0]["implementationId"],
            "implementationVersion": mutated["familyPolicies"][0]["implementationVersion"],
            "familySemanticImplementationSha256": mutated["familyPolicies"][0]["familySemanticImplementationSha256"],
            "semanticContract": copy.deepcopy(mutated["familyPolicies"][0]["semanticContract"]),
        }
        mutated["familyPolicies"][0]["implementationSha256"] = canonical_sha256(implementation)
        self._assert_invalid(mutated)

    def test_12_missing_or_nonfinite_numeric_sample_is_rejected(self) -> None:
        mutated = copy.deepcopy(self.receipt)
        mutated["occurrences"][0]["numericSamples"].pop()
        self._assert_invalid(mutated)
        mutated = copy.deepcopy(self.receipt)
        sample = mutated["occurrences"][0]["numericSamples"][0]
        sample["output"]["forged"] = float("inf")
        sample["outputSha256"] = canonical_sha256(sample["output"])
        self._assert_invalid(mutated)

    def test_13_blocker_loss_and_admission_flip_are_rejected(self) -> None:
        mutated = copy.deepcopy(self.receipt)
        mutated["occurrences"][0]["preservedEvidenceBlockers"].remove(
            "SOURCE_EXACT_NOT_CLAIMED"
        )
        self._assert_invalid(mutated)
        mutated = copy.deepcopy(self.receipt)
        mutated["occurrences"][0]["runtimeExecutionAdmission"] = True
        self._assert_invalid(mutated)
        mutated = copy.deepcopy(self.receipt)
        mutated["productAdmission"]["allowed"] = True
        self._assert_invalid(mutated)

    def test_14_unknown_class_and_duplicate_occurrence_fail_closed(self) -> None:
        mutated_source = copy.deepcopy(self.source)
        _, module = self._blocked_module(mutated_source, "particlemodulelifetime_seeded")
        module["exactSourceClass"] = "forged_unknown_module"
        with self.assertRaises(ValueError):
            build_occurrence_rows(
                mutated_source,
                build_family_policies(self.root, self.source),
                self.custom,
                self.acquisition,
            )
        mutated = copy.deepcopy(self.receipt)
        mutated["occurrences"][1]["moduleOccurrenceId"] = mutated["occurrences"][0]["moduleOccurrenceId"]
        self._assert_invalid(mutated)

    def test_15_all_seven_family_properties_affect_numeric_output(self) -> None:
        baseline_rows = build_occurrence_rows(
            self.source,
            build_family_policies(self.root, self.source),
            self.custom,
            self.acquisition,
        )
        cases: list[tuple[str, str, callable, bool]] = []

        def seeded_mutation(source: dict, module: dict) -> None:
            descriptor = self._descriptor(module, "lifetime")
            descriptor["lookupTable"] = [value + 0.125 for value in descriptor["lookupTable"]]

        cases.append(("particlemodulelifetime_seeded", "source.reconstructed.seeded.v1", seeded_mutation, False))

        def cylinder_mutation(source: dict, module: dict) -> None:
            literal = next(row for row in module["typedPayload"]["literals"] if row["propertyPath"] == "benabled")
            literal["value"] = not literal["value"]

        cases.append(("efparticlemodulelocationprimitivecylinderspin", "source.reconstructed.cylinder-spin.v1", cylinder_mutation, False))

        def ground_mutation(source: dict, module: dict) -> None:
            adapter = self._adapter(module, "skiplocation")
            adapter["numericOracleSamples"][0]["value"] = [0.0]

        cases.append(("efparticlemodulelocationonground", "source.reconstructed.ground.v1", ground_mutation, False))

        def decal_mutation(source: dict, module: dict) -> None:
            literal = next(row for row in module["typedPayload"]["literals"] if row["propertyPath"] == "nearplane")
            literal["value"] = float(literal["value"]) + 25.0

        cases.append(("efparticlemoduletypedatadecal", "source.reconstructed.decal.v1", decal_mutation, True))

        def light_mutation(source: dict, module: dict) -> None:
            field = next(row for row in source["pointLightAdapter"]["fields"] if row["fieldPath"] == "brightness")
            field["value"] = float(field["value"]) + 1.0

        cases.append(("efparticlemoduletypedatalight", "source.reconstructed.light.v1", light_mutation, False))

        def velocity_mutation(source: dict, module: dict) -> None:
            descriptor = self._descriptor(module, "veloverlife")
            descriptor["lookupTable"] = [value * 1.5 for value in descriptor["lookupTable"]]

        cases.append(("efparticlemodulevelocityoverlifetime", "source.reconstructed.velocity.v1", velocity_mutation, False))

        def vector_mutation(source: dict, module: dict) -> None:
            adapter = self._adapter(module, "startrotation")
            field = next(row for row in adapter["currentRevisionFields"] if row["fieldPath"] == "constant")
            field["value"]["z"] = float(field["value"]["z"]) + 0.25

        cases.append(("particlemodulemeshrotation", "source.reconstructed.ef-vector-multiply.v1", vector_mutation, False))

        changed_families = set()
        for source_class, family_id, mutator, mutation_must_reject in cases:
            mutated_source = copy.deepcopy(self.source)
            _, module = self._blocked_module(mutated_source, source_class)
            module_id = module["moduleOccurrenceId"]
            baseline_output = self._output_for(baseline_rows, module_id)
            mutator(mutated_source, module)
            if mutation_must_reject:
                with self.assertRaises(ValueError):
                    build_occurrence_rows(
                        mutated_source,
                        build_family_policies(self.root, self.source),
                        self.custom,
                        self.acquisition,
                    )
            else:
                mutated_rows = build_occurrence_rows(
                    mutated_source,
                    build_family_policies(self.root, self.source),
                    self.custom,
                    self.acquisition,
                )
                self.assertNotEqual(
                    baseline_output, self._output_for(mutated_rows, module_id)
                )
            changed_families.add(family_id)
        self.assertEqual(changed_families, set(EXPECTED_FAMILY_COUNTS))

    def test_16_coordinated_resealed_row_mutation_is_rejected(self) -> None:
        mutated = copy.deepcopy(self.receipt)
        sample = mutated["occurrences"][0]["numericSamples"][0]
        sample["typedInputs"]["fixedSeed"] += 1
        sample["typedInputSha256"] = canonical_sha256(sample["typedInputs"])
        sample["output"]["coordinatedForgedValue"] = 3.0
        sample["outputSha256"] = canonical_sha256(sample["output"])
        self._assert_invalid(mutated)

    def test_17_checked_in_receipt_matches_generator(self) -> None:
        checked_in = load_strict_json_object(self.root / OUTPUT_PATH)
        self.assertEqual(json_bytes(checked_in), json_bytes(self.receipt))

    def test_18_cli_check_passes(self) -> None:
        process = subprocess.run(
            [
                sys.executable,
                "-B",
                "Tools/LevelPlacementExtractor/build_artist_31470_reconstructed_source_capability.py",
                "--check",
            ],
            cwd=self.root,
            check=False,
            capture_output=True,
            text=True,
        )
        self.assertEqual(process.returncode, 0, process.stdout + process.stderr)

    def test_19_recursive_variant_schema_rejects_key_and_type_attacks(self) -> None:
        mutated = copy.deepcopy(self.receipt)
        sample = mutated["occurrences"][0]["numericSamples"][0]
        sample["typedInputs"]["unknownGenericBagField"] = 1.0
        sample["typedInputSha256"] = canonical_sha256(sample["typedInputs"])
        self._assert_invalid(mutated)

        mutated = copy.deepcopy(self.receipt)
        sample = mutated["occurrences"][0]["numericSamples"][0]
        sample["typedInputs"]["time"] = 0
        sample["typedInputSha256"] = canonical_sha256(sample["typedInputs"])
        self._assert_invalid(mutated)

        mutated = copy.deepcopy(self.receipt)
        sample = mutated["occurrences"][0]["numericSamples"][0]
        literal = next(iter(sample["typedInputs"]["sourceLiterals"].values()))
        literal["present"] = False
        literal["value"] = 1.0
        sample["typedInputSha256"] = canonical_sha256(sample["typedInputs"])
        self._assert_invalid(mutated)

    def test_20_reviewer_field_attacks_change_reconstructed_output(self) -> None:
        policies = build_family_policies(self.root, self.source)
        baseline = build_occurrence_rows(
            self.source, policies, self.custom, self.acquisition
        )

        def assert_literal_changes(source_class: str, property_path: str) -> None:
            source = copy.deepcopy(self.source)
            candidates = []
            for occurrence in source["occurrences"]:
                for module in occurrence["modules"]:
                    if module["decision"] == "BLOCKED" and module["exactSourceClass"] == source_class:
                        if any(row["propertyPath"] == property_path for row in module["typedPayload"]["literals"]):
                            candidates.append(module)
            self.assertTrue(candidates, property_path)
            module = candidates[0]
            module_id = module["moduleOccurrenceId"]
            literal = next(row for row in module["typedPayload"]["literals"] if row["propertyPath"] == property_path)
            self.assertIs(type(literal["value"]), bool)
            literal["value"] = not literal["value"]
            mutated = build_occurrence_rows(source, policies, self.custom, self.acquisition)
            self.assertNotEqual(
                self._output_for(baseline, module_id),
                self._output_for(mutated, module_id),
                property_path,
            )

        source = copy.deepcopy(self.source)
        _, color_module = self._blocked_module(source, "particlemodulecolorscaleoverlife")
        color_id = color_module["moduleOccurrenceId"]
        alpha_descriptor = self._descriptor(color_module, "alphascaleoverlife")
        if alpha_descriptor["lookupTable"]:
            alpha_descriptor["lookupTable"] = [value + 0.25 for value in alpha_descriptor["lookupTable"]]
        else:
            alpha_descriptor["defaultMinimum"][0] += 0.25
        mutated = build_occurrence_rows(source, policies, self.custom, self.acquisition)
        self.assertNotEqual(self._output_for(baseline, color_id), self._output_for(mutated, color_id))

        assert_literal_changes("efparticlemodulelocationprimitivecylinderspin", "surfaceonly")
        assert_literal_changes("efparticlemodulelocationprimitivecylinderspin", "positive_y")
        assert_literal_changes("particlemodulelocationprimitivecylinder_seeded", "negative_z")
        assert_literal_changes("particlemodulelocationprimitivecylinder_seeded", "positive_z")

    def test_21_property_consumption_and_default_provenance_are_complete(self) -> None:
        summary = self.receipt["summary"]
        self.assertEqual(summary["sourcePropertyRowCount"], 148)
        self.assertEqual(summary["policyInputConsumedPropertyCount"], 117)
        self.assertEqual(summary["preservedIrrelevantPropertyCount"], 31)
        self.assertEqual(summary["unconsumedPropertyCount"], 0)
        for row in self.receipt["occurrences"]:
            for property_row in row["propertyConsumption"]:
                if property_row["capabilityConsumptionDecision"] == "PRESERVED_VERIFIED_IRRELEVANT":
                    self.assertTrue(property_row["irrelevanceOracleId"])
                else:
                    self.assertTrue(property_row["outputDependencyRequired"])
            policy = next(
                policy for policy in self.receipt["familyPolicies"]
                if policy["policyFamilyId"] == row["policyFamilyId"]
            )
            binding = next(
                binding for binding in policy["semanticContract"]["variantBindings"]
                if binding["variant"] == row["variant"]
            )
            for default in binding["explicitDefaults"]:
                self.assertEqual(
                    default["provenance"],
                    "RECONSTRUCTED_APPROVED_V1_POLICY_DEFAULT_NOT_SOURCE_EVIDENCE",
                )
                self.assertFalse(default["sourceExact"])

    def test_22_missing_source_era_identity_is_explicitly_unpinned(self) -> None:
        summary = self.receipt["summary"]
        self.assertEqual(summary["distributionBindingCount"], 65)
        self.assertEqual(summary["sourceEraIdentityPinnedDistributionCount"], 0)
        self.assertEqual(summary["sourceEraIdentityUnpinnedDistributionCount"], 65)
        self.assertEqual(summary["sourceEraIdentityFieldMissingDistributionCount"], 60)
        bindings = [
            binding for row in self.receipt["occurrences"]
            for binding in row["distributionBindings"]
        ]
        self.assertTrue(all(binding["sourceEraIdentityPinned"] is False for binding in bindings))
        self.assertEqual(sum(not binding["sourceEraIdentityFieldPresent"] for binding in bindings), 60)

        mutated = copy.deepcopy(self.receipt)
        binding = mutated["occurrences"][0]["distributionBindings"][0]
        binding["sourceEraIdentityPinned"] = True
        self._assert_invalid(mutated)

    def test_23_global_id_and_distribution_owner_attacks_are_rejected(self) -> None:
        mutated = copy.deepcopy(self.receipt)
        first = mutated["occurrences"][0]["distributionBindings"][0]
        second = mutated["occurrences"][1]["distributionBindings"][0]
        second["distributionId"] = first["distributionId"]
        self._assert_invalid(mutated)

        mutated = copy.deepcopy(self.receipt)
        first_property = mutated["occurrences"][0]["propertyConsumption"][0]
        mutated["occurrences"][1]["propertyConsumption"][0]["propertyId"] = first_property["propertyId"]
        self._assert_invalid(mutated)

        mutated = copy.deepcopy(self.receipt)
        mutated["occurrences"][0]["occurrenceCompositeId"] = "foreign::occurrence"
        self._assert_invalid(mutated)

    def test_24_tracked_json_lf_crlf_parity_and_bom_rejection(self) -> None:
        source_bytes = (self.root / SOURCE_PATH).read_bytes()
        text = source_bytes.decode("utf-8")
        lf = text.replace("\r\n", "\n").replace("\r", "\n").encode("utf-8")
        crlf = lf.decode("utf-8").replace("\n", "\r\n").encode("utf-8")
        with tempfile.TemporaryDirectory() as directory:
            directory_path = Path(directory)
            lf_path = directory_path / "lf.json"
            crlf_path = directory_path / "crlf.json"
            bom_path = directory_path / "bom.json"
            semantic_path = directory_path / "semantic.json"
            lf_path.write_bytes(lf)
            crlf_path.write_bytes(crlf)
            bom_path.write_bytes(b"\xef\xbb\xbf" + lf)
            semantic_path.write_bytes(lf.replace(b'"skillId": 31470', b'"skillId": 31471', 1))
            self.assertEqual(tracked_json_sha256(lf_path), tracked_json_sha256(crlf_path))
            self.assertNotEqual(tracked_json_sha256(lf_path), tracked_json_sha256(semantic_path))
            with self.assertRaises(ValueError):
                tracked_json_sha256(bom_path)

    def test_25_dependency_and_default_identity_attacks_are_rejected(self) -> None:
        mutated = copy.deepcopy(self.receipt)
        mutated["executionDependencies"][1]["canonicalTextSha256"] = "33" * 32
        mutated["executionDependencySetSha256"] = canonical_sha256(
            mutated["executionDependencies"]
        )
        self._assert_invalid(mutated)

        mutated = copy.deepcopy(self.receipt)
        binding = mutated["familyPolicies"][0]["semanticContract"]["variantBindings"][0]
        binding["explicitDefaults"][0]["provenance"] = "SOURCE_EXACT"
        mutated["familyPolicies"][0]["familySemanticImplementationSha256"] = canonical_sha256(
            mutated["familyPolicies"][0]["semanticContract"]
        )
        implementation = {
            "implementationId": mutated["familyPolicies"][0]["implementationId"],
            "implementationVersion": mutated["familyPolicies"][0]["implementationVersion"],
            "familySemanticImplementationSha256": mutated["familyPolicies"][0]["familySemanticImplementationSha256"],
            "semanticContract": copy.deepcopy(mutated["familyPolicies"][0]["semanticContract"]),
        }
        mutated["familyPolicies"][0]["implementationSha256"] = canonical_sha256(implementation)
        self._assert_invalid(mutated)

    def test_26_decal_current_cdo_defaults_and_outputs_are_exact(self) -> None:
        policy = next(
            row for row in self.receipt["familyPolicies"]
            if row["policyFamilyId"] == "source.reconstructed.decal.v1"
        )
        self.assertEqual(policy["implementationVersion"], 2)
        self.assertEqual(
            policy["semanticContract"]["algorithm"],
            "EXPLICIT_DECAL_FRUSTUM_DESCRIPTOR_FROM_SOURCE_NEAR_PLANE_AND_CURRENT_EFGAME_CDO_DEFAULTS",
        )
        binding = next(
            row for row in policy["semanticContract"]["variantBindings"]
            if row["variant"] == "EF_DECAL_DESCRIPTOR"
        )
        defaults = {
            row["fieldPath"]: row["value"]
            for row in binding["explicitDefaults"]
            if row["fieldPath"].startswith("decal.")
        }
        self.assertEqual(defaults, {
            "decal.blendRange": [100.0, 100.0],
            "decal.defaultSize": [50.0, 50.0],
            "decal.farPlane": 300.0,
            "decal.supports3dDrawMode": True,
            "decal.yawOnly": True,
        })
        self.assertEqual(
            set(binding["inputSchema"]["fields"]),
            {
                "time", "fixedSeed", "randomUnits", "sourceLiterals",
                "evaluatedDistributions", "nearPlane", "farPlane",
                "defaultSize", "blendRange", "yawOnly",
                "supports3dDrawMode",
            },
        )
        self.assertEqual(
            set(binding["outputSchema"]["fields"]),
            {"variant", "frustum", "yawOnly", "supports3dDrawMode"},
        )

        rows = [
            row for row in self.receipt["occurrences"]
            if row["exactSourceClass"] == DECAL_SOURCE_CLASS
        ]
        self.assertEqual(len(rows), 3)
        expected_default_sha = canonical_sha256(binding["explicitDefaults"])
        for row in rows:
            self.assertEqual(row["explicitDefaultsSha256"], expected_default_sha)
            self.assertFalse(row["sourceExact"])
            self.assertTrue(set(EVIDENCE_BLOCKERS).issubset(row["preservedEvidenceBlockers"]))
            near_literal = next(
                literal for literal in row["sourceLiteralBindings"]
                if literal["propertyPath"] == "nearplane"
            )
            self.assertIs(type(near_literal["value"]), float)
            self.assertEqual(near_literal["value"], EXPECTED_DECAL_NEAR_PLANE)
            for sample in row["numericSamples"]:
                typed_inputs = sample["typedInputs"]
                output = sample["output"]
                self.assertEqual(typed_inputs["nearPlane"], -300.0)
                self.assertEqual(typed_inputs["farPlane"], 300.0)
                self.assertEqual(typed_inputs["defaultSize"], [50.0, 50.0])
                self.assertEqual(typed_inputs["blendRange"], [100.0, 100.0])
                self.assertIs(typed_inputs["yawOnly"], True)
                self.assertIs(typed_inputs["supports3dDrawMode"], True)
                self.assertEqual(
                    output["frustum"],
                    [-300.0, 300.0, 50.0, 50.0, 100.0, 100.0],
                )
                self.assertIs(output["yawOnly"], True)
                self.assertIs(output["supports3dDrawMode"], True)

    def test_27_decal_cdo_and_implicit_default_mutations_fail_closed(self) -> None:
        mutations = []

        def root_yaw(source: dict) -> None:
            source["currentRevisionDefaultEvidence"]["decal"]["values"][
                "bonlycalcrotationyaw"
            ] = False

        mutations.append(("root-yaw", root_yaw))

        def cdo_support(source: dict) -> None:
            source["currentRevisionDefaultEvidence"]["classDefaultObjects"][
                "efParticleModuleTypeDataDecal"
            ]["properties"]["bsupported3ddrawmode"]["value"] = False

        mutations.append(("cdo-support", cdo_support))

        def occurrence_yaw(source: dict) -> None:
            _, module = self._blocked_module(source, DECAL_SOURCE_CLASS)
            module["implicitDefaults"][0]["values"]["bonlycalcrotationyaw"] = False

        mutations.append(("occurrence-yaw", occurrence_yaw))

        def occurrence_provenance(source: dict) -> None:
            _, module = self._blocked_module(source, DECAL_SOURCE_CLASS)
            module["implicitDefaults"][0]["provenance"] = "FORGED"

        mutations.append(("occurrence-provenance", occurrence_provenance))

        def near_plane(source: dict) -> None:
            _, module = self._blocked_module(source, DECAL_SOURCE_CLASS)
            literal = next(
                row for row in module["typedPayload"]["literals"]
                if row["propertyPath"] == "nearplane"
            )
            literal["value"] = -299.0

        mutations.append(("near-plane", near_plane))

        def far_plane(source: dict) -> None:
            source["currentRevisionDefaultEvidence"]["decal"]["values"][
                "farplane"
            ] = 301.0

        mutations.append(("far-plane", far_plane))

        for name, mutate in mutations:
            with self.subTest(name=name):
                source = copy.deepcopy(self.source)
                mutate(source)
                with self.assertRaises(ValueError):
                    build_family_policies(self.root, source)

    def test_28_coordinated_resealed_decal_yaw_attack_is_rejected(self) -> None:
        mutated = copy.deepcopy(self.receipt)
        policy = next(
            row for row in mutated["familyPolicies"]
            if row["policyFamilyId"] == "source.reconstructed.decal.v1"
        )
        binding = next(
            row for row in policy["semanticContract"]["variantBindings"]
            if row["variant"] == "EF_DECAL_DESCRIPTOR"
        )
        yaw_default = next(
            row for row in binding["explicitDefaults"]
            if row["fieldPath"] == "decal.yawOnly"
        )
        yaw_default["value"] = False
        policy["familySemanticImplementationSha256"] = canonical_sha256(
            policy["semanticContract"]
        )
        implementation = {
            "implementationId": policy["implementationId"],
            "implementationVersion": policy["implementationVersion"],
            "familySemanticImplementationSha256": policy[
                "familySemanticImplementationSha256"
            ],
            "semanticContract": copy.deepcopy(policy["semanticContract"]),
        }
        policy["implementationSha256"] = canonical_sha256(implementation)
        explicit_defaults_sha = canonical_sha256(binding["explicitDefaults"])
        for row in mutated["occurrences"]:
            if row["exactSourceClass"] != DECAL_SOURCE_CLASS:
                continue
            row["implementationSha256"] = policy["implementationSha256"]
            row["familySemanticImplementationSha256"] = policy[
                "familySemanticImplementationSha256"
            ]
            row["explicitDefaultsSha256"] = explicit_defaults_sha
            for sample in row["numericSamples"]:
                sample["typedInputs"]["yawOnly"] = False
                sample["typedInputSha256"] = canonical_sha256(sample["typedInputs"])
                sample["output"]["yawOnly"] = False
                sample["outputSha256"] = canonical_sha256(sample["output"])
        self._assert_invalid(mutated)

    def test_29_implicit_default_overlap_matrix_has_no_other_conflict(self) -> None:
        implicit_counts: dict[str, int] = {}
        for occurrence in self.source["occurrences"]:
            for module in occurrence["modules"]:
                if module["decision"] != "BLOCKED":
                    continue
                rows = module.get("implicitDefaults") or []
                if rows:
                    implicit_counts[module["exactSourceClass"]] = (
                        implicit_counts.get(module["exactSourceClass"], 0) + len(rows)
                    )
        self.assertEqual(implicit_counts, {
            DECAL_SOURCE_CLASS: 3,
            "efparticlemoduletypedatalight": 1,
        })

        policies = build_family_policies(self.root, self.source)
        light_policy = next(
            row for row in policies
            if row["policyFamilyId"] == "source.reconstructed.light.v1"
        )
        light_binding = light_policy["semanticContract"]["variantBindings"][0]
        self.assertFalse(any(
            row["fieldPath"].startswith("light.")
            for row in light_binding["explicitDefaults"]
        ))
        light_row = next(
            row for row in self.receipt["occurrences"]
            if row["exactSourceClass"] == "efparticlemoduletypedatalight"
        )
        expected_fields = {
            row["fieldPath"]: row["value"]
            for row in self.source["pointLightAdapter"]["fields"]
        }
        for sample in light_row["numericSamples"]:
            self.assertEqual(sample["typedInputs"]["pointLightFields"], expected_fields)


if __name__ == "__main__":
    unittest.main(verbosity=2)
