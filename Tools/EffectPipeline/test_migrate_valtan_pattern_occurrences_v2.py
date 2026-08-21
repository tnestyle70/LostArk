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

    def test_checked_in_baseline_identity_is_valid(self) -> None:
        self.assertEqual(
            MIGRATION.validate_v2(
                self.bindings,
                self.cues,
                require_migration_denominator=False,
            ),
            (130, 137, 106),
        )
        self.assertEqual(99, self.receipt["outputs"]["cueOccurrenceCount"])
        MIGRATION.check_receipt(self.receipt, self.bindings, self.cues)

    def test_new_v2_rows_do_not_invalidate_the_migrated_identity_subset(
        self,
    ) -> None:
        bindings = copy.deepcopy(self.bindings)
        cues = copy.deepcopy(self.cues)
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
        cues["cues"].append(
            {
                **copy.deepcopy(cues["cues"][0]),
                "bindingId": "cue.valtan.test.added",
                "occurrenceId": "cue.valtan.test.added.occurrence.01",
            }
        )
        self.assertEqual(
            MIGRATION.validate_v2(
                bindings,
                cues,
                require_migration_denominator=False,
            ),
            (131, 138, 107),
        )
        MIGRATION.check_receipt(self.receipt, bindings, cues)

    def test_rebinding_a_migrated_cue_is_rejected(self) -> None:
        cues = copy.deepcopy(self.cues)
        cues["cues"][0]["effectAssetId"] = "effect.valtan.invalid.rebind"
        with self.assertRaisesRegex(
            MIGRATION.MigrationError,
            "removed or rebound",
        ):
            MIGRATION.check_receipt(self.receipt, self.bindings, cues)

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
            MIGRATION.check_receipt(receipt, self.bindings, self.cues)

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
            "restored or rebound",
        ):
            MIGRATION.check_receipt(self.receipt, self.bindings, restored)

    def test_reordering_migrated_clip_occurrences_is_rejected(self) -> None:
        bindings = copy.deepcopy(self.bindings)
        multi = next(
            row for row in bindings["bindings"] if len(row["clips"]) > 1
        )
        multi["clips"].reverse()
        with self.assertRaisesRegex(MIGRATION.MigrationError, "order changed"):
            MIGRATION.check_receipt(self.receipt, bindings, self.cues)


if __name__ == "__main__":
    unittest.main()
