#!/usr/bin/env python3

from __future__ import annotations

import copy
import hashlib
import json
import tempfile
import unittest
from pathlib import Path

try:
    from . import materialize_warlord_17140_wpo_sinwave_canary as canary
except ImportError:
    import materialize_warlord_17140_wpo_sinwave_canary as canary


class WarlordWpoSinWaveCanaryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.product = canary.read_json(canary.PRODUCT)
        cls.candidate = canary.build_candidate(copy.deepcopy(cls.product))

    def test_product_is_byte_frozen_at_56_rows(self) -> None:
        self.assertEqual(canary.raw_sha256(canary.PRODUCT), canary.PRODUCT_BYTE_SHA256)
        self.assertEqual(len(self.product["elements"]), 56)

    def test_tracked_candidate_and_receipt_are_current(self) -> None:
        canary.run("check")
        self.assertEqual(canary.read_json(canary.CANDIDATE), self.candidate)
        receipt = canary.read_json(canary.RECEIPT)
        sealed = copy.deepcopy(receipt)
        claimed = sealed.pop("receiptSha256")
        self.assertEqual(claimed, canary.canonical_sha256(sealed))
        self.assertEqual(receipt["provenance"], "PROJECT_RECONSTRUCTED")
        self.assertFalse(receipt["admission"]["productCue"])
        self.assertFalse(receipt["admission"]["visual"])

    def test_only_material_execution_side_differs_from_product_rows(self) -> None:
        product_by_id = {row["id"]: row for row in self.product["elements"]}
        for row in self.candidate["elements"]:
            original = product_by_id[row["id"]]
            for key in set(original) | set(row):
                if key != "material":
                    self.assertEqual(original.get(key), row.get(key), f"{row['id']}/{key}")
            self.assertEqual(row["material"]["sourceMaterialPath"], canary.CHILD)
            self.assertEqual(row["material"]["sourceProfile"]["parentMaterialPath"], canary.PARENT)

    def test_second_occurrence_is_data_only_packet_reuse(self) -> None:
        first, second = self.candidate["elements"]
        self.assertEqual(first["material"]["execution"], second["material"]["execution"])
        self.assertEqual(first["material"]["execution"]["opcode"], 22)
        receipt = canary.read_json(canary.RECEIPT)
        self.assertEqual(receipt["packet"]["secondOccurrenceCppChanges"], 0)
        self.assertEqual(receipt["packet"]["secondOccurrenceHlslChanges"], 0)

    def test_only_two_proven_lanes_are_consumed(self) -> None:
        execution = self.candidate["elements"][0]["material"]["execution"]
        self.assertEqual(
            [(row["role"], row["assetId"], row["sourceChannel"])
             for row in execution["textureLanes"]],
            [
                ("alpha_mask_21_map_c", canary.THUNDER, "R"),
                ("emission_02_map_e", canary.EMISSION, "RGB"),
            ],
        )
        receipt = canary.read_json(canary.RECEIPT)
        pending = receipt["family"]["pendingEvidence"]
        self.assertEqual([row["sourceParameter"] for row in pending[:3]],
                         list(canary.PENDING_TEXTURES))
        self.assertTrue(all(row["status"] == "PENDING_EVIDENCE" for row in pending))

    def test_candidate_is_cataloged_but_has_no_product_cue(self) -> None:
        catalog = canary.read_json(canary.CATALOG)
        matches = [row for row in catalog["effects"]
                   if row["effectAssetId"] == canary.CANDIDATE_EFFECT_ID]
        self.assertEqual(len(matches), 1)
        self.assertEqual(matches[0]["payloadKind"], "DIRECT_AUTHORED_DOCUMENT_V13")
        animevents = (canary.ROOT / "Data/Animation/Authored/Warlord/Warlord.animevents").read_text(
            encoding="utf-8-sig"
        )
        self.assertNotIn(canary.CANDIDATE_EFFECT_ID, animevents)
        self.assertIn(canary.PRODUCT_EFFECT_ID, animevents)

    def test_disposition_waits_for_user_review(self) -> None:
        disposition = canary.read_json(canary.RECEIPT)["disposition"]
        self.assertEqual(disposition["carrierDisposition"], "USER_REVIEW_PENDING")
        self.assertEqual(disposition["candidateAction"], "ADD_OR_REPLACE_PENDING")
        self.assertFalse(disposition["terminalKeepReplaceAddRetireAssigned"])

    def test_candidate_validation_rejects_carrier_child_and_packet_drift(self) -> None:
        mutations = []
        forged = copy.deepcopy(self.candidate)
        forged["elements"][0]["resources"][0]["assetId"] = "forged.wmodel"
        mutations.append(forged)
        forged = copy.deepcopy(self.candidate)
        forged["elements"][0]["material"]["sourceMaterialPath"] = "forged.child"
        mutations.append(forged)
        forged = copy.deepcopy(self.candidate)
        forged["elements"][1]["material"]["execution"]["opcode"] = 17
        mutations.append(forged)
        for mutation in mutations:
            with self.assertRaises(RuntimeError):
                canary.validate_candidate(mutation, self.product)

    def test_write_is_deterministic_in_memory(self) -> None:
        rebuilt = canary.build_candidate(copy.deepcopy(self.product))
        self.assertEqual(canary.pretty_bytes(rebuilt), canary.pretty_bytes(self.candidate))
        self.assertEqual(
            hashlib.sha256(canary.pretty_bytes(rebuilt)).hexdigest(),
            hashlib.sha256(canary.pretty_bytes(self.candidate)).hexdigest(),
        )


if __name__ == "__main__":
    unittest.main()
