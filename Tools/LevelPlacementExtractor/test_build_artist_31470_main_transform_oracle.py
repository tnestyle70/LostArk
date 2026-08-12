#!/usr/bin/env python3
"""Focused and mutation tests for the Artist 31470 G02 transform oracle."""

from __future__ import annotations

import copy
import unittest

import build_artist_31470_main_transform_oracle as oracle


def reseal_row(row: dict) -> None:
    row.pop("rowSha256", None)
    oracle.seal_row(row)


def reseal_receipt(receipt: dict) -> None:
    receipt.pop("receiptSha256", None)
    oracle.seal_receipt(receipt)


class BasisConjugationTests(unittest.TestCase):
    def test_three_axis_conjugation_preserves_rotated_point_and_rejects_swizzle(self) -> None:
        canary = oracle.build_basis_canary()
        self.assertLessEqual(canary["pointConsistencyMaxAbsError"], 1.0e-12)
        self.assertGreater(canary["conjugationVsNaiveMaxAbsDelta"], 0.1)
        self.assertTrue(canary["componentSwizzleRejected"])
        self.assertEqual(canary["clientPointByBasis"], [0.37, 1.19, 0.61])

    def test_row_vector_cue_then_root_order_is_not_commutative(self) -> None:
        cue = oracle.srt_matrix((3.0, 3.0, 3.0), (0.0, 0.0, 0.0), (1.0, 0.0, 1.0))
        root = oracle.srt_matrix(
            oracle.ROOT_SCALE,
            oracle.ROOT_ROTATION_DEGREES,
            oracle.ROOT_POSITION,
        )
        local = (0.23, -0.41, 0.79)
        intended = oracle.transform_point3(local, oracle.mat4_mul(cue, root))
        reversed_order = oracle.transform_point3(local, oracle.mat4_mul(root, cue))
        self.assertGreater(
            max(abs(intended[index] - reversed_order[index]) for index in range(3)),
            1.0,
        )


class TrackedReceiptTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.tracked = oracle.read_json_strict(oracle.DEFAULT_OUTPUT)
        oracle.validate_receipt(cls.tracked)
        cls.rebuilt = oracle.build_receipt()

    def mutate(self, callback) -> dict:
        value = copy.deepcopy(self.tracked)
        callback(value)
        reseal_receipt(value)
        return value

    def test_tracked_receipt_is_current_against_all_five_input_groups(self) -> None:
        self.assertEqual(
            oracle.encoded_receipt(self.rebuilt),
            oracle.encoded_receipt(self.tracked),
        )

    def test_mutation_rejects_basis_axis_sign_even_after_reseal(self) -> None:
        mutated = self.mutate(
            lambda value: value["coordinateContract"]["sourceToClientBasisRows"][1].__setitem__(2, 1.0)
        )
        with self.assertRaisesRegex(ValueError, "source-to-client basis changed"):
            oracle.validate_receipt(mutated)

    def test_mutation_rejects_resealed_component_swizzle_rotation(self) -> None:
        def change(value: dict) -> None:
            canary = value["basisConjugationCanary"]
            canary["clientRotationRowsByConjugation"] = copy.deepcopy(
                canary["naiveComponentSwizzleRotationRows"]
            )

        mutated = self.mutate(change)
        with self.assertRaisesRegex(ValueError, "basis canary clientRotationRowsByConjugation changed"):
            oracle.validate_receipt(mutated)

    def test_mutation_rejects_double_geometry_prescale_after_row_reseal(self) -> None:
        def change(value: dict) -> None:
            row = value["occurrences"][0]
            row["applicationCounts"]["geometryPreScale"] = 2
            reseal_row(row)

        mutated = self.mutate(change)
        with self.assertRaisesRegex(ValueError, "main transform owner/admission changed"):
            oracle.validate_receipt(mutated)

    def test_mutation_rejects_double_cue_scale_after_row_reseal(self) -> None:
        def change(value: dict) -> None:
            row = value["occurrences"][1]
            row["applicationCounts"]["cueScale"] = 2
            reseal_row(row)

        mutated = self.mutate(change)
        with self.assertRaisesRegex(ValueError, "main transform owner/admission changed"):
            oracle.validate_receipt(mutated)

    def test_mutation_rejects_coordinated_live_follow_after_all_row_reseals(self) -> None:
        def change(value: dict) -> None:
            cue = value["actionCueProjection"]
            attachment = cue["programProjection"]["actionCueAttachment"]
            attachment["follow"] = True
            attachment["sourceAnchorSlotId"] = "b_wp_1"
            attachment["runtimeAnchorSlotId"] = "weapon.source"
            attachment["runtimeBoneName"] = "b_wp_1"
            cue["programProjectionSha256"] = oracle.canonical_sha256(
                cue["programProjection"]
            )
            reseal_row(cue)
            for row in value["occurrences"]:
                row["attachment"] = copy.deepcopy(attachment)
                reseal_row(row)

        mutated = self.mutate(change)
        with self.assertRaisesRegex(ValueError, "main action cue projection changed"):
            oracle.validate_receipt(mutated)

    def test_mutation_rejects_raw_pivot_promotion_after_row_reseal(self) -> None:
        def change(value: dict) -> None:
            row = value["geometry"][0]
            row["rawUpkToGltfPivot"] = {
                "status": "RESOLVED",
                "admitted": True,
                "reason": "MUTATED_WITHOUT_AUTHORITY",
            }
            reseal_row(row)

        mutated = self.mutate(change)
        with self.assertRaisesRegex(ValueError, "main WModel transform/pivot boundary changed"):
            oracle.validate_receipt(mutated)

    def test_mutation_rejects_late_historical_root_promotion(self) -> None:
        def change(value: dict) -> None:
            value["fixedFixture"]["capturedRoot"]["lateInitialSeekHistory"] = {
                "status": "RESOLVED",
                "admitted": True,
                "reason": "MUTATED_WITHOUT_ACTOR_ROOT_HISTORY",
            }
            value["admission"]["lateHistoricalRootReplay"] = True

        mutated = self.mutate(change)
        with self.assertRaisesRegex(ValueError, "captured root fixture changed"):
            oracle.validate_receipt(mutated)

    def test_mutation_rejects_world_matrix_and_ndc_drift_after_row_reseal(self) -> None:
        def change(value: dict) -> None:
            row = value["occurrences"][2]
            row["matrices"]["runtimeWorldWithoutBodyPresentationYaw"][0][0] += 0.25
            row["fixedViewProjectionPoint"]["runtimeWorld"]["ndcPoint"][0] -= 0.5
            reseal_row(row)

        mutated = self.mutate(change)
        with self.assertRaisesRegex(ValueError, "runtime world changed"):
            oracle.validate_receipt(mutated)

    def test_mutation_rejects_playable_yaw_promotion(self) -> None:
        def change(value: dict) -> None:
            value["fixedFixture"]["playablePresentationYawDiagnostic"][
                "selectedForRuntimeWorld"
            ] = True
            value["admission"]["playablePresentationYawRelation"] = True

        mutated = self.mutate(change)
        with self.assertRaisesRegex(ValueError, "playable presentation yaw boundary changed"):
            oracle.validate_receipt(mutated)


if __name__ == "__main__":
    unittest.main()
