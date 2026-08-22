#!/usr/bin/env python3
"""Historical SafeReviewedGaps and Carrier V1 successor contracts."""

from __future__ import annotations

import copy
import json
from pathlib import Path
import sys
import unittest
from unittest import mock

TOOLS = Path(__file__).resolve().parent
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import apply_valtan_safe_reviewed_gaps as applicator


SCHEMA_PATH = (
    applicator.ROOT
    / "Tools/EffectPipeline/Schemas/"
    "lostark.valtan-safe-reviewed-gap-application-receipt.schema.json"
)


class ValtanSafeReviewedGapHistoricalTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.state, cls.writes, cls.receipt = applicator.expected_application()
        cls.successor = applicator.validate_carrier_v1_successor()
        cls.cues = applicator.read_json(applicator.candidates.CUES_PATH)
        cls.catalog = applicator.read_json(applicator.candidates.CATALOG_PATH)

    def test_safe_gap_is_historical_and_never_rewrites_current_product(self) -> None:
        self.assertEqual("CARRIER_V1_SUCCESSOR_HISTORICAL", self.state)
        self.assertEqual({applicator.RECEIPT_PATH}, set(self.writes))
        self.assertEqual(
            applicator.RECEIPT_PATH.read_bytes(),
            self.writes[applicator.RECEIPT_PATH],
        )
        self.assertNotIn(applicator.candidates.CUES_PATH, self.writes)
        self.assertNotIn(applicator.candidates.CATALOG_PATH, self.writes)

    def test_historical_receipt_keeps_its_original_denominators(self) -> None:
        applicator.validate_historical_receipt(self.receipt)
        self.assertEqual(106, self.receipt["canonicalCueDocument"]["cueCount"])
        self.assertEqual(315, self.receipt["canonicalCatalogDocument"]["effectCount"])
        self.assertEqual(160, self.receipt["summary"]["coreProjectionCount"])
        self.assertEqual(167, self.receipt["summary"]["addedElementCount"])
        self.assertEqual(7, len(self.receipt["trailProjections"]))

    def test_historical_input_artifacts_remain_byte_identical(self) -> None:
        for key in ("candidateManifest", "drawableProof"):
            identity = self.receipt[key]
            path = applicator.ROOT / identity["path"]
            self.assertTrue(path.is_file())
            self.assertEqual(identity["rawSha256"], applicator.raw_sha256(path))
            self.assertEqual(
                identity["artifactSha256"],
                applicator.read_json(path)["artifactSha256"],
            )

    def test_carrier_v1_is_the_only_current_clip_owner_successor(self) -> None:
        summary = self.successor["summary"]
        self.assertEqual(660, summary["reviewedCoreProjectionCount"])
        self.assertEqual(657, summary["materializedProjectionCount"])
        self.assertEqual(46, summary["finalValtanCatalogCount"])
        self.assertEqual(44, summary["finalBossRootCueCount"])
        self.assertEqual(
            0,
            self.successor["productReset"]["duplicateClipOccurrenceOwnerCount"],
        )

        cue_ids = {row["bindingId"] for row in self.cues["cues"]}
        effect_ids = {row["effectAssetId"] for row in self.catalog["effects"]}
        self.assertTrue(
            set(applicator.SAFE_GAP_PREDECESSOR_BINDING_IDS).isdisjoint(cue_ids)
        )
        self.assertTrue(
            set(applicator.SAFE_GAP_PREDECESSOR_EFFECT_IDS).isdisjoint(effect_ids)
        )

    def test_each_predecessor_has_an_explicit_successor_or_retirement(self) -> None:
        mappings = {
            row["retiredBindingId"]: row
            for row in self.successor["retiredOwnerSuccessorMappings"]
        }
        dispositions = {
            key: mappings[key]["disposition"]
            for key in applicator.SAFE_GAP_PREDECESSOR_BINDING_IDS
        }
        self.assertEqual(
            "RETIRED_NO_EXACT_REVIEWED_CARRIER_OWNER",
            dispositions["cue.valtan.jump-spin.spin.trails"],
        )
        for key in (
            "cue.valtan.backstep.windup.trails",
            "cue.valtan.four-slash.active.clip-02",
            "cue.valtan.swing.active.clip-02",
        ):
            self.assertEqual(
                "REPLACED_BY_EXACT_CARRIER_V1_CLIP_OWNER",
                dispositions[key],
            )

    def test_tampered_historical_seal_fails_closed(self) -> None:
        tampered = copy.deepcopy(self.receipt)
        tampered["summary"]["coreProjectionCount"] = 159
        with self.assertRaisesRegex(applicator.ApplyError, "receipt"):
            applicator.validate_historical_receipt(tampered)

    def test_tampered_successor_mapping_fails_closed(self) -> None:
        tampered = copy.deepcopy(self.successor)
        row = next(
            row
            for row in tampered["retiredOwnerSuccessorMappings"]
            if row["retiredBindingId"] == "cue.valtan.four-slash.active.clip-02"
        )
        row["replacementEffectAssetId"] = "effect.valtan.missing"
        real_read = applicator.read_json

        def fake_read(path: Path) -> dict:
            if path == applicator.CARRIER_V1_RECEIPT_PATH:
                return tampered
            return real_read(path)

        with mock.patch.object(applicator, "read_json", side_effect=fake_read):
            with self.assertRaisesRegex(applicator.ApplyError, "not live"):
                applicator.validate_carrier_v1_successor()

    def test_historical_receipt_still_matches_its_original_schema(self) -> None:
        schema = json.loads(SCHEMA_PATH.read_text(encoding="utf-8"))
        try:
            import jsonschema
        except ImportError:
            self.skipTest("jsonschema is not installed")
        jsonschema.Draft202012Validator.check_schema(schema)
        jsonschema.validate(self.receipt, schema)


if __name__ == "__main__":
    unittest.main()
