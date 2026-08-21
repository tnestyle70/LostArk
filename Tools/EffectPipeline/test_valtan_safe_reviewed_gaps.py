#!/usr/bin/env python3
"""Focused contract tests for the proof-gated Valtan safe-gap closure."""

from __future__ import annotations

from copy import deepcopy
import json
from pathlib import Path
import unittest

import apply_valtan_safe_reviewed_gaps as applicator
import build_valtan_safe_reviewed_gap_candidates as candidates
import build_valtan_safe_reviewed_gap_drawable_proof as drawable_proof


ROOT = Path(__file__).resolve().parents[2]
SCHEMA_ROOT = Path(__file__).parent / "Schemas"


def read_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


class ValtanSafeReviewedGapTests(unittest.TestCase):
    def setUp(self) -> None:
        self.manifest = read_json(candidates.MANIFEST_PATH)
        self.proof = read_json(drawable_proof.PROOF_PATH)
        self.receipt = read_json(applicator.RECEIPT_PATH)

    def test_candidate_manifest_is_exact_and_tamper_evident(self) -> None:
        candidates.validate_manifest(self.manifest)
        by_effect = {
            row["effectAssetId"]: (
                row["elementCount"],
                row["coreProjectionCount"],
                row["trailProjectionCount"],
            )
            for row in self.manifest["candidateDocuments"]
        }
        self.assertEqual(
            {
                "effect.valtan.backstep.windup.trails": (3, 0, 3),
                "effect.valtan.four-slash.active.clip-02": (20, 19, 1),
                "effect.valtan.jump-spin.spin.trails": (3, 0, 3),
                "effect.valtan.swing.active.clip-02": (141, 141, 0),
            },
            by_effect,
        )
        self.assertEqual(160, len(self.manifest["coreProjections"]))
        self.assertEqual(7, len(self.manifest["adapterProjections"]))
        self.assertEqual(6, len(self.manifest["preservedUnresolvedTrailRows"]))

        tampered = deepcopy(self.manifest)
        tampered["summary"]["elementCount"] = 166
        with self.assertRaises(candidates.SafeGapError):
            candidates.validate_manifest(tampered)

    def test_drawable_proof_rebuilds_exactly_and_rejects_denominator_drift(self) -> None:
        drawable_proof.validate_proof(self.proof)
        rebuilt = drawable_proof.build()
        self.assertEqual(self.proof, rebuilt)
        self.assertEqual(167, self.proof["summary"]["preparedElementCount"])
        self.assertEqual(167, self.proof["summary"]["drawnElementCount"])
        self.assertEqual(0, self.proof["summary"]["failedDrawCount"])

        tampered = deepcopy(self.proof)
        tampered["summary"]["drawnElementCount"] = 166
        drawable_proof.seal(tampered, "artifactSha256")
        with self.assertRaises(drawable_proof.ProofError):
            drawable_proof.validate_proof(tampered)

    def test_applied_state_is_exact_byte_idempotent_and_tamper_evident(self) -> None:
        state, writes, rebuilt_receipt = applicator.expected_application()
        self.assertEqual("APPLIED_EXACT", state)
        self.assertEqual(self.receipt, rebuilt_receipt)
        self.assertTrue(writes)
        for path, payload in writes.items():
            self.assertTrue(path.is_file(), path)
            self.assertEqual(payload, path.read_bytes(), path)

        tampered = deepcopy(self.receipt)
        tampered["summary"]["addedElementCount"] = 166
        applicator.seal(tampered, "artifactSha256")
        with self.assertRaises(applicator.ApplyError):
            applicator.validate_receipt(tampered)

    def test_canonical_cues_catalog_and_documents_close_only_four_slices(self) -> None:
        cues = read_json(candidates.CUES_PATH)
        catalog = read_json(candidates.CATALOG_PATH)
        cue_index = {row["bindingId"]: row for row in cues["cues"]}
        catalog_index = {row["effectAssetId"]: row for row in catalog["effects"]}
        self.assertEqual(108, len(cues["cues"]))
        self.assertEqual(315, len(catalog["effects"]))

        for row in self.manifest["candidateDocuments"]:
            cue = row["cue"]
            catalog_row = row["catalogRow"]
            self.assertEqual(cue, cue_index[cue["bindingId"]])
            self.assertEqual(catalog_row, catalog_index[row["effectAssetId"]])
            self.assertEqual("once", cue["repeatPolicy"])
            self.assertEqual(0, cue["sourceStartMs"])
            self.assertIsNone(cue["sourceEndMs"])
            canonical_path = ROOT / row["canonicalPath"]
            canonical = read_json(canonical_path)
            self.assertEqual(row["effectAssetId"], canonical["effectAssetId"])
            self.assertEqual(row["elementCount"], len(canonical["elements"]))
            self.assertEqual(row["elementIds"], [e["id"] for e in canonical["elements"]])
            self.assertEqual(row["canonicalSha256"], candidates.canonical_sha256(canonical))

    def test_candidate_proof_and_application_artifacts_match_json_schemas(self) -> None:
        try:
            import jsonschema
        except ImportError:
            self.skipTest("jsonschema is not installed")
        fixtures = (
            (
                self.manifest,
                "lostark.valtan-safe-reviewed-gap-candidates.schema.json",
            ),
            (
                self.proof,
                "lostark.valtan-safe-reviewed-gap-drawable-proof.schema.json",
            ),
            (
                self.receipt,
                "lostark.valtan-safe-reviewed-gap-application-receipt.schema.json",
            ),
        )
        for document, schema_name in fixtures:
            with self.subTest(schema=schema_name):
                schema = read_json(SCHEMA_ROOT / schema_name)
                jsonschema.Draft202012Validator.check_schema(schema)
                jsonschema.validate(document, schema)


if __name__ == "__main__":
    unittest.main()
