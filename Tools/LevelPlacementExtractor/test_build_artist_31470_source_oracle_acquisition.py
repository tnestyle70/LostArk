from __future__ import annotations

import copy
import unittest
from pathlib import Path
from typing import Any, Callable

import build_artist_31470_source_oracle_acquisition as builder
from build_artist_31470_source_execution_semantics import canonical_sha256, json_bytes
from effect_source_contract_io import load_strict_json_object


ROOT = Path(__file__).resolve().parents[2]
SOURCE_EXECUTION = ROOT / builder.SOURCE_EXECUTION_PATH
CUSTOM_HANDLER = ROOT / builder.CUSTOM_HANDLER_PATH
SOURCE_RECEIPT = ROOT / builder.SOURCE_RECEIPT_PATH
RECEIPT = ROOT / builder.OUTPUT_PATH


class Artist31470SourceOracleAcquisitionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.source_execution = load_strict_json_object(SOURCE_EXECUTION)
        cls.custom_handler = load_strict_json_object(CUSTOM_HANDLER)
        cls.source_receipt = load_strict_json_object(SOURCE_RECEIPT)
        cls.receipt = load_strict_json_object(RECEIPT)

    @staticmethod
    def reseal(receipt: dict[str, Any]) -> None:
        unsigned = copy.deepcopy(receipt)
        unsigned.pop("receiptSha256", None)
        receipt["receiptSha256"] = canonical_sha256(unsigned)

    def assert_mutation_rejected(
        self,
        mutation: Callable[[dict[str, Any]], None],
        pattern: str = "changed|differs|promoted|gate|contract|boundary",
    ) -> None:
        changed = copy.deepcopy(self.receipt)
        mutation(changed)
        self.reseal(changed)
        with self.assertRaisesRegex(ValueError, pattern):
            builder.validate_receipt(
                changed,
                self.source_execution,
                self.custom_handler,
                self.source_receipt,
                ROOT,
            )

    def test_baseline_is_deterministic_and_blocked(self) -> None:
        builder.validate_receipt(
            self.receipt,
            self.source_execution,
            self.custom_handler,
            self.source_receipt,
            ROOT,
        )
        rebuilt = builder.build_receipt(
            ROOT, self.source_execution, self.custom_handler, self.source_receipt
        )
        self.assertEqual(json_bytes(self.receipt), json_bytes(rebuilt))
        self.assertEqual(self.receipt["blockerDelta"], {
            "beforeBlockedModuleOccurrenceCount": 29,
            "afterBlockedModuleOccurrenceCount": 29,
            "resolvedModuleOccurrenceCount": 0,
            "beforeActualOutputOracleCount": 0,
            "afterActualOutputOracleCount": 0,
            "ownerlessBlockerCount": 0,
        })

    def test_all_29_occurrences_are_owned_by_15_classes_and_7_families(self) -> None:
        classes = self.receipt["sourceBlockerClassRows"]
        families = self.receipt["nativeFamilyClusters"]
        occurrence_ids = [
            occurrence_id
            for row in classes for occurrence_id in row["moduleOccurrenceIds"]
        ]
        self.assertEqual(len(classes), 15)
        self.assertEqual(len(families), 7)
        self.assertEqual(len(occurrence_ids), 29)
        self.assertEqual(len(set(occurrence_ids)), 29)
        self.assertTrue(all(row["owner"]["role"] == "SOURCE_SPECIALIST"
                            for row in classes))
        self.assertTrue(all(row["sourceEraProviderId"] is None
                            and row["actualOutputOracleCount"] == 0
                            for row in classes))

    def test_format_version_bool_is_rejected(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row.__setitem__("formatVersion", True), "version changed"
        )

    def test_class_denominator_cannot_shrink(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["sourceBlockerClassRows"].pop(),
            "class rows were promoted or changed",
        )

    def test_class_provider_cannot_be_fabricated(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["sourceBlockerClassRows"][0].__setitem__(
                "sourceEraProviderId", "fabricated.provider"
            ),
            "class rows were promoted or changed",
        )

    def test_family_pilot_cannot_be_fabricated(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["nativeFamilyClusters"][0].__setitem__(
                "standaloneActualOutputPilotCount", 1
            ),
            "native family acquisition gate changed",
        )

    def test_current_surface_cannot_be_laundered_as_source_era(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["currentCallableSurface"]["standardSeeded"].__setitem__(
                "fidelity", "SOURCE_EXACT"
            )
        )

    def test_vss_cannot_be_marked_exhausted(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            vss = next(row for row in receipt["auditedSearchRoots"]
                       if row["auditId"] == "local.vss.v1")
            vss["status"] = "EXHAUSTED"
            vss["providerCount"] = 0

        self.assert_mutation_rejected(mutate, "VSS was laundered")

    def test_search_result_cannot_acquire_untracked_provider(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["auditedSearchRoots"][0].__setitem__("providerCount", 1)
        )

    def test_unreachable_map_cannot_be_promoted(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["rejectedUnreachableCandidate"].__setitem__(
                "decision", "ACCEPTED_SOURCE_PROVIDER"
            )
        )

    def test_source_package_revision_cannot_be_laundered(self) -> None:
        target_index = next(
            index for index, row in enumerate(
                self.receipt["sourcePackageRevisionComparisons"]
            ) if row["logicalPackage"] == "FX_PC_SDM_07"
        )
        self.assert_mutation_rejected(
            lambda row: row["sourcePackageRevisionComparisons"][target_index].__setitem__(
                "sameRevision", True
            )
        )

    def test_source_exact_branch_cannot_be_promoted(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["fidelityBranches"]["sourceExact"].__setitem__(
                "decision", "READY"
            ),
            "fidelity branch approval boundary changed",
        )

    def test_reconstruction_cannot_be_approved_inside_receipt(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["fidelityBranches"][
                "reconstructedNumericallyVerifiedMaximum"
            ].__setitem__("approved", True),
            "fidelity branch approval boundary changed",
        )

    def test_external_artifact_contract_cannot_accept_current_only(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["externalArtifactIntakeContract"]["rejectedEvidence"].remove(
                "current-only binary or package identity"
            ),
            "external artifact intake contract changed",
        )

    def test_external_artifact_contract_cannot_drop_mutated_output(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["externalArtifactIntakeContract"][
                "fixtureFieldsRequired"
            ].remove("expected mutated numeric post-state"),
            "external artifact intake contract changed",
        )

    def test_blocker_delta_cannot_claim_resolution(self) -> None:
        def mutate(receipt: dict[str, Any]) -> None:
            receipt["blockerDelta"]["resolvedModuleOccurrenceCount"] = 1
            receipt["blockerDelta"]["afterBlockedModuleOccurrenceCount"] = 28

        self.assert_mutation_rejected(mutate, "blocker delta changed")

    def test_next_stage_cannot_be_opened(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["nextStageAdmission"].__setitem__("allowed", True),
            "next-stage gate changed",
        )

    def test_product_cannot_be_opened(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["productAdmission"].__setitem__("allowed", True),
            "granted Product admission",
        )

    def test_tracked_input_identity_cannot_change(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["repoArtifactInputs"][0].__setitem__(
                "rawSha256", "0" * 64
            )
        )

    def test_tool_identity_cannot_change(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row["toolIdentity"].__setitem__(
                "canonicalTextSha256", "0" * 64
            )
        )

    def test_plan_identity_cannot_change(self) -> None:
        self.assert_mutation_rejected(
            lambda row: row.__setitem__("canonicalPlanCommit", "0" * 40)
        )


if __name__ == "__main__":
    unittest.main()
