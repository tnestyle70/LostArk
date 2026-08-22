#!/usr/bin/env python3

from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path
from unittest import mock


TOOLS = Path(__file__).resolve().parent
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import build_valtan_source_timing_delta_proposals as proposals
import build_valtan_portal_rush_imported_canary as portal_canary


class ValtanSourceTimingDeltaProposalTests(unittest.TestCase):
    def test_portal_finish_compresses_full_source_segment(self) -> None:
        document = proposals.build_proposals()
        self.assertEqual(0, document["summary"]["canonicalDataMutationCount"])
        self.assertEqual(1, len(document["proposals"]))
        row = document["proposals"][0]
        self.assertEqual(
            "SOURCE_REVIEWED_DELTA",
            row["proposedBindingClip"]["mappingBasis"],
        )
        self.assertFalse(row["proposedBindingClip"]["loop"])
        self.assertAlmostEqual(
            1666.7 / 600.0,
            row["proposedBindingClip"]["playRate"],
        )
        self.assertEqual(1666, row["reviewOnlySourceWindow"]["sourceEndMs"])
        self.assertIsNone(row["currentProductCue"])
        self.assertIsNone(row["proposedProductCue"])
        self.assertIn("BLOCKED_NO_EXACT", row["status"])
        self.assertIn("DO_NOT_RESTORE_LEGACY_CUE", row["productCueDisposition"])
        self.assertEqual(
            "RETIRED_NO_EXACT_REVIEWED_CARRIER_OWNER",
            row["historicalCueOwner"]["disposition"],
        )
        self.assertIsNone(row["historicalCueOwner"]["replacementBindingId"])
        self.assertLessEqual(
            row["wallProjection"]["effectiveFullSegmentWallDurationMs"],
            row["serverStageDurationMs"],
        )
        self.assertTrue(
            row["wallProjection"]["allReviewedNotifySamplesFitStage"]
        )

    def test_dash_400424_remains_explicit_cross_stage_review(self) -> None:
        document = proposals.build_proposals()
        self.assertFalse(
            any(
                row.get("patternId") == "VALTAN_DASH_CHARGE"
                for row in document["proposals"]
            )
        )
        self.assertEqual(
            "CROSS_STAGE_SOURCE_TIMING_SPLIT_REVIEW_REQUIRED",
            document["deferredExplicitReviews"][0]["status"],
        )

    def test_invalid_rate_loop_and_cue_segment_are_rejected(self) -> None:
        document = proposals.build_proposals()
        row = document["proposals"][0]
        binding = copy.deepcopy(row["proposedBindingClip"])
        cue = copy.deepcopy(row["reviewOnlySourceWindow"])
        binding["playRate"] = 0.01
        with self.assertRaises(proposals.TimingProposalError):
            proposals.validate_transform(1666.7, 600, binding, cue)
        binding = copy.deepcopy(row["proposedBindingClip"])
        binding["loop"] = True
        with self.assertRaises(proposals.TimingProposalError):
            proposals.validate_transform(1666.7, 600, binding, cue)
        binding = copy.deepcopy(row["proposedBindingClip"])
        cue["sourceEndMs"] = 1667
        with self.assertRaises(proposals.TimingProposalError):
            proposals.validate_transform(1666.7, 600, binding, cue)

    def test_output_is_deterministic(self) -> None:
        self.assertEqual(proposals.build_proposals(), proposals.build_proposals())

    def test_successor_contract_drift_fails_closed(self) -> None:
        receipt = portal_canary.read_json(portal_canary.CARRIER_V1_RECEIPT_PATH)
        finish = next(
            row
            for row in receipt["retiredOwnerSuccessorMappings"]
            if row["retiredBindingId"] == proposals.PORTAL_RETIRED_BINDING_ID
        )
        finish["replacementBindingId"] = "cue.valtan.illegal-revival"
        actual_read_json = portal_canary.read_json

        def tampered_read_json(path: Path) -> dict:
            if path == portal_canary.CARRIER_V1_RECEIPT_PATH:
                return copy.deepcopy(receipt)
            return actual_read_json(path)

        with mock.patch.object(
            portal_canary, "read_json", side_effect=tampered_read_json
        ):
            with self.assertRaises(proposals.TimingProposalError):
                proposals.build_proposals()


if __name__ == "__main__":
    unittest.main()
