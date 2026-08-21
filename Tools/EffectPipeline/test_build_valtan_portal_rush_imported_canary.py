#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import sys
import tempfile
import unittest
from pathlib import Path


TOOLS = Path(__file__).resolve().parent
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import build_valtan_portal_rush_imported_canary as canary
import build_valtan_source_occurrence_inventory as inventory


class ValtanPortalRushImportedCanaryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.files, cls.receipt = canary.build_canary()

    def test_emits_one_independent_v13_document_per_reachable_clip(self) -> None:
        self.assertEqual(3, self.receipt["summary"]["candidateDocumentCount"])
        self.assertEqual(24, self.receipt["summary"]["executableElementCount"])
        self.assertEqual(
            set(canary.EXPECTED_CLIPS),
            {
                row["clipOccurrenceId"]
                for row in self.receipt["documents"]
            },
        )
        self.assertNotIn(
            canary.EXCLUDED_FINISH_CLIP,
            {
                row["clipOccurrenceId"]
                for row in self.receipt["documents"]
            },
        )
        for row in self.receipt["documents"]:
            document = json.loads(
                self.files[row["candidateDocumentPath"]].decode("utf-8")
            )
            canary.validate_candidate_document(
                document, row["effectAssetId"]
            )
            self.assertEqual(
                row["executableElementCount"], len(document["elements"])
            )
            self.assertEqual(
                row["candidateDocumentSha256"],
                inventory.sha256_bytes(
                    self.files[row["candidateDocumentPath"]]
                ),
            )

    def test_reconcile_is_missing_only_and_preserves_authored_rows(self) -> None:
        self.assertEqual(0, self.receipt["summary"]["deletedElementCount"])
        self.assertEqual(0, self.receipt["summary"]["sourceRebaseRequiredCount"])
        self.assertEqual(24, self.receipt["summary"]["missingOnlyAddElementCount"])
        self.assertEqual(47, self.receipt["summary"]["preservedAuthoredElementCount"])
        for row in self.receipt["documents"]:
            plan = row["reconcile"]
            self.assertEqual([], plan["deleteElements"])
            self.assertEqual(
                plan["existingElementCount"],
                plan["preservedExistingElementCount"],
            )
            self.assertEqual(
                row["executableElementCount"], len(plan["addElementRefs"])
            )
            self.assertEqual(
                "REPORT_ONLY_UNVERIFIED_DEFAULT_SIGNATURE_NO_DELETE",
                plan["legacyRetirementDisposition"],
            )

    def test_user_tuned_existing_source_row_is_never_overwritten(self) -> None:
        document_row = self.receipt["documents"][0]
        candidate = json.loads(
            self.files[document_row["candidateDocumentPath"]].decode("utf-8")
        )
        existing = {
            "effectAssetId": document_row["effectAssetId"],
            "elements": [copy.deepcopy(candidate["elements"][0])],
        }
        existing["elements"][0]["detail"]["color"]["emissiveIntensity"] = 7.0
        plan = inventory.reconcile_effect_document(
            existing, candidate["elements"]
        )
        self.assertEqual(1, plan["preservedExistingElementCount"])
        self.assertEqual([], plan["deleteElements"])
        self.assertEqual(
            len(candidate["elements"]) - 1, len(plan["addElements"])
        )
        self.assertEqual(
            7.0,
            existing["elements"][0]["detail"]["color"][
                "emissiveIntensity"
            ],
        )

    def test_build_is_byte_deterministic(self) -> None:
        second_files, second_receipt = canary.build_canary()
        self.assertEqual(self.files, second_files)
        self.assertEqual(self.receipt, second_receipt)

    def test_exact_check_does_not_mutate_the_file(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "candidate.json"
            payload = b'{"ok":true}\n'
            path.write_bytes(payload)
            before = (path.stat().st_mtime_ns, path.read_bytes())
            canary.check_exact(path, payload)
            after = (path.stat().st_mtime_ns, path.read_bytes())
            self.assertEqual(before, after)
            with self.assertRaises(canary.CanaryError):
                canary.check_exact(path, b"{}\n")


if __name__ == "__main__":
    unittest.main()
