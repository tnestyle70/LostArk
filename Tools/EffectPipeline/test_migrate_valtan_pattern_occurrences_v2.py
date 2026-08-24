#!/usr/bin/env python3
"""Regression tests for the one-shot Valtan occurrence v2 migration."""

from __future__ import annotations

import copy
import importlib.util
from pathlib import Path
import unittest


SCRIPT = Path(__file__).with_name("migrate_valtan_pattern_occurrences_v2.py")
SPEC = importlib.util.spec_from_file_location("valtan_occurrence_migration", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
MIGRATION = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MIGRATION)


class ValtanOccurrenceMigrationReceiptTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.bindings = MIGRATION.read_json(MIGRATION.BINDINGS_PATH)
        cls.cues = MIGRATION.read_json(MIGRATION.CUES_PATH)
        cls.receipt = MIGRATION.read_json(MIGRATION.RECEIPT_PATH)
        cls.carrier_receipt = MIGRATION.read_json(
            MIGRATION.CARRIER_V1_RECEIPT_PATH
        )

    def test_checked_in_baseline_identity_is_valid(self) -> None:
        self.assertEqual(
            MIGRATION.validate_v2(
                self.bindings,
                self.cues,
                require_migration_denominator=False,
            ),
            (131, 137, 44),
        )
        self.assertEqual(99, self.receipt["outputs"]["cueOccurrenceCount"])
        self.assertEqual(
            99, len(self.receipt["baselineIdentity"]["cues"])
        )
        MIGRATION.check_receipt(
            self.receipt,
            self.bindings,
            self.cues,
            self.carrier_receipt,
        )

    def test_new_binding_rows_do_not_invalidate_the_migrated_identity_subset(
        self,
    ) -> None:
        bindings = copy.deepcopy(self.bindings)
        bindings["bindings"].append(
            {
                "actionId": "valtan.test.added.action",
                "clips": [
                    {
                        "clipOccurrenceId": "valtan.test.added.action.clip.01",
                        "clip": "mesh_idle_battle_1",
                        "mappingBasis": "PROJECT_AUTHORED",
                        "sourceStartMs": 0,
                        "playMs": 0,
                        "playRate": 1.0,
                        "loop": True,
                    }
                ],
            }
        )
        self.assertEqual(
            MIGRATION.validate_v2(
                bindings,
                self.cues,
                require_migration_denominator=False,
            ),
            (132, 138, 44),
        )
        MIGRATION.check_receipt(
            self.receipt,
            bindings,
            self.cues,
            self.carrier_receipt,
        )

    def test_arbitrary_extra_cue_is_rejected(self) -> None:
        cues = copy.deepcopy(self.cues)
        cues["cues"].append(
            {
                **copy.deepcopy(cues["cues"][0]),
                "bindingId": "cue.valtan.test.added",
                "occurrenceId": "cue.valtan.test.added.occurrence.01",
            }
        )
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "Unexpected v2 denominators",
        ):
            MIGRATION.validate_v2(
                self.bindings,
                cues,
                require_migration_denominator=False,
            )
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "cue denominator drifted",
        ):
            MIGRATION.check_receipt(
                self.receipt,
                self.bindings,
                cues,
                self.carrier_receipt,
            )

    def test_rebinding_a_migrated_cue_is_rejected(self) -> None:
        cues = copy.deepcopy(self.cues)
        cues["cues"][0]["effectAssetId"] = "effect.valtan.invalid.rebind"
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "canonical hash drifted|rebound",
        ):
            MIGRATION.check_receipt(
                self.receipt,
                self.bindings,
                cues,
                self.carrier_receipt,
            )

    def test_only_the_exact_combat_object_transfer_may_retire_a_baseline_cue(
        self,
    ) -> None:
        receipt = copy.deepcopy(self.receipt)
        receipt["retiredBaselineCues"][0]["effectAssetId"] = (
            "effect.valtan.invalid"
        )
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "retirement ledger drifted",
        ):
            MIGRATION.check_receipt(
                receipt,
                self.bindings,
                self.cues,
                self.carrier_receipt,
            )

        restored = copy.deepcopy(self.cues)
        baseline = next(
            row
            for row in self.receipt["baselineIdentity"]["cues"]
            if row["occurrenceId"]
            == "cue.valtan.red-blade-wave.active.occurrence.01"
        )
        restored["cues"].append(
            {
                **baseline,
                "anchorSlotId": "root",
                "followPolicy": "follow",
                "stopPolicy": "cue_end",
                "repeatPolicy": "once",
                "sourceStartMs": 0,
                "sourceEndMs": 1000,
                "localTransform": {
                    "position": [0, 0, 0],
                    "rotationDegrees": [0, 0, 0],
                    "scale": [1, 1, 1],
                },
            }
        )
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "cue denominator drifted",
        ):
            MIGRATION.check_receipt(
                self.receipt,
                self.bindings,
                restored,
                self.carrier_receipt,
            )

    def test_reordering_migrated_clip_occurrences_is_rejected(self) -> None:
        bindings = copy.deepcopy(self.bindings)
        multi = next(
            row for row in bindings["bindings"] if len(row["clips"]) > 1
        )
        multi["clips"].reverse()
        with self.assertRaisesRegex(MIGRATION.MigrationError, "order changed"):
            MIGRATION.check_receipt(
                self.receipt,
                bindings,
                self.cues,
                self.carrier_receipt,
            )

    def test_current_cue_metadata_drift_is_rejected_by_canonical_hash(
        self,
    ) -> None:
        cues = copy.deepcopy(self.cues)
        cues["cues"][0]["localTransform"]["position"][0] = 0.25
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "current cue canonical hash drifted",
        ):
            MIGRATION.check_receipt(
                self.receipt,
                self.bindings,
                cues,
                self.carrier_receipt,
            )

    def test_carrier_receipt_metadata_and_migration_hash_drift_fail_closed(
        self,
    ) -> None:
        carrier_receipt = copy.deepcopy(self.carrier_receipt)
        carrier_receipt["blocked"]["productAdmission"] = True
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "successor contract seal drifted",
        ):
            MIGRATION.check_receipt(
                self.receipt,
                self.bindings,
                self.cues,
                carrier_receipt,
            )

        migration_receipt = copy.deepcopy(self.receipt)
        migration_receipt["carrierV1SuccessorContract"][
            "currentCueCanonicalSha256"
        ] = "0" * 64
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "successor contract seal drifted",
        ):
            MIGRATION.check_receipt(
                migration_receipt,
                self.bindings,
                self.cues,
                self.carrier_receipt,
            )

    def test_successor_mapping_and_current_owner_sets_are_exact(self) -> None:
        contract = MIGRATION.build_carrier_v1_successor_contract(
            self.carrier_receipt,
            self.cues,
            self.receipt["baselineIdentity"]["cues"],
        )
        self.assertEqual(
            self.receipt["carrierV1SuccessorContract"], contract
        )
        self.assertEqual(105, contract["retiredCueCount"])
        self.assertEqual(48, contract["replacementMappingCount"])
        self.assertEqual(57, contract["retiredWithoutSuccessorCount"])
        self.assertEqual(96, contract["carrierRetiredBaselineCueCount"])
        self.assertEqual(1, contract["survivingBaselineCueCount"])
        self.assertEqual(3, contract["fourSlashOwnerTransferCount"])
        self.assertEqual(
            MIGRATION.FOUR_SLASH_SOURCE_BRANCH_ID,
            contract["fourSlashSourceBranchId"],
        )
        self.assertEqual(
            MIGRATION.FOUR_SLASH_OLD_CUE_CANONICAL_SHA256,
            contract["fourSlashOldCueCanonicalSha256"],
        )
        self.assertEqual(
            MIGRATION.FOUR_SLASH_NEW_CUE_CANONICAL_SHA256,
            contract["fourSlashNewCueCanonicalSha256"],
        )

    def test_only_the_three_exact_owner_transfers_are_admitted(self) -> None:
        carrier_receipt = copy.deepcopy(self.carrier_receipt)
        carrier_receipt["fourSlashPatternSplitOwnerReseal"][
            "sourceSequenceCanonicalSha256"
        ] = "0" * 64
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "owner reseal proof drifted",
        ):
            MIGRATION.build_carrier_v1_successor_contract(
                carrier_receipt,
                self.cues,
                self.receipt["baselineIdentity"]["cues"],
            )

        carrier_receipt = copy.deepcopy(self.carrier_receipt)
        unapproved = next(
            row
            for row in carrier_receipt["retiredOwnerSuccessorMappings"]
            if row["replacementBindingId"] is not None
            and row["replacementBindingId"]
            not in {
                transfer["replacementBindingId"]
                for transfer in MIGRATION.FOUR_SLASH_OWNER_TRANSFERS
            }
        )
        unapproved["patternId"] = "VALTAN_ROTATION_SLASH"
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "successor cue is missing or rebound",
        ):
            MIGRATION.build_carrier_v1_successor_contract(
                carrier_receipt,
                self.cues,
                self.receipt["baselineIdentity"]["cues"],
            )

    def test_split_binding_owner_and_loop_are_exact(self) -> None:
        bindings = copy.deepcopy(self.bindings)
        triple = next(
            row
            for row in bindings["bindings"]
            if row["actionId"] == "valtan.attack.triple-slash.active"
        )
        triple["clips"][0]["loop"] = True
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "clip split is missing or rebound",
        ):
            MIGRATION.check_receipt(
                self.receipt,
                bindings,
                self.cues,
                self.carrier_receipt,
            )


class ValtanRejoinedFourSlashSuccessorTests(unittest.TestCase):
    """Current-product admission after the historical split was retired."""

    @classmethod
    def setUpClass(cls) -> None:
        cls.bindings = MIGRATION.read_json(MIGRATION.BINDINGS_PATH)
        cls.cues = MIGRATION.read_json(MIGRATION.CUES_PATH)

    def test_checked_in_rejoined_successor_is_exact(self) -> None:
        self.assertEqual(
            (131, 141, 47),
            MIGRATION.validate_rejoined_four_slash_successor(
                self.bindings,
                self.cues,
            ),
        )

    def test_rejoined_cue_owner_drift_fails_closed(self) -> None:
        cues = copy.deepcopy(self.cues)
        row = next(
            cue
            for cue in cues["cues"]
            if cue.get("bindingId")
            == "cue.valtan.carrier-v1.attack.four-slash.active.clip-02"
        )
        row["stageId"] = "SLASHES"
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "Rejoined four-slash cue is missing or rebound",
        ):
            MIGRATION.validate_rejoined_four_slash_successor(
                self.bindings,
                cues,
            )

    def test_retired_split_owner_cannot_return(self) -> None:
        cues = copy.deepcopy(self.cues)
        row = next(
            cue
            for cue in cues["cues"]
            if cue.get("bindingId")
            == "cue.valtan.carrier-v1.attack.four-slash.active.clip-01"
        )
        row["patternId"] = "VALTAN_TRIPLE_SLASH"
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "Rejoined four-slash cue is missing or rebound|Retired split",
        ):
            MIGRATION.validate_rejoined_four_slash_successor(
                self.bindings,
                cues,
            )


if __name__ == "__main__":
    unittest.main()
