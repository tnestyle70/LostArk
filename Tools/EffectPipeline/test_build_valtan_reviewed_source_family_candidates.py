#!/usr/bin/env python3
"""Historical reviewed-source candidates after the Carrier V1 reset."""

from __future__ import annotations

import copy
import json
import sys
import unittest
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import build_valtan_reviewed_source_family_candidates as subject
import validate_boss_pattern_effects as schema_validator


SCHEMA_PATH = (
    subject.ROOT
    / "Tools/EffectPipeline/Schemas/"
    "lostark.valtan-reviewed-source-family-candidates.schema.json"
)


class ValtanReviewedSourceFamilyHistoricalTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.files, cls.receipt = subject.build_candidates()
        cls.inventory = subject.load_inventory(subject.SELECTION_PATH)
        cls.successor = subject.validate_carrier_v1_successor(cls.receipt)
        cls.catalog = subject.read_json(subject.CATALOG_PATH)
        cls.cues = subject.read_json(subject.CUE_PATH)

    def test_builder_is_a_historical_witness_and_has_no_product_outputs(self) -> None:
        receipt_path = subject.OUTPUT_ROOT / subject.RECEIPT_NAME
        self.assertEqual(
            {receipt_path.relative_to(subject.ROOT).as_posix()}, set(self.files)
        )
        self.assertEqual(
            receipt_path.read_bytes(),
            self.files[receipt_path.relative_to(subject.ROOT).as_posix()],
        )
        self.assertNotIn(
            subject.CATALOG_PATH.relative_to(subject.ROOT).as_posix(), self.files
        )
        self.assertNotIn(
            subject.CUE_PATH.relative_to(subject.ROOT).as_posix(), self.files
        )

    def test_historical_candidate_denominators_remain_sealed(self) -> None:
        summary = self.receipt["summary"]
        self.assertEqual(628, summary["reachableCoreProjectionCount"])
        self.assertEqual(36, summary["candidateDocumentCount"])
        self.assertEqual(279, summary["candidateElementCount"])
        self.assertEqual(160, summary["missingCueProjectionCount"])
        self.assertEqual(100, summary["multipleCueProjectionCount"])
        self.assertEqual(0, summary["deletedElementCount"])
        subject.validate_historical_candidate_outputs(self.receipt, self.inventory)

    def test_all_historical_candidate_documents_keep_their_byte_identity(self) -> None:
        self.assertEqual(36, len(self.receipt["documents"]))
        for row in self.receipt["documents"]:
            path = subject.ROOT / row["candidateDocumentPath"]
            self.assertTrue(path.is_file(), path)
            self.assertEqual(
                row["candidateDocumentSha256"],
                subject.source_inventory.sha256_file(path),
            )

    def test_safe_gap_160_pairs_are_a_subset_of_the_expanded_inventory(self) -> None:
        manifest = subject.read_json(subject.SAFE_GAP_MANIFEST_PATH)
        subject.validate_safe_gap_core_projection_identity(self.inventory, manifest)
        receipt_pairs = {
            (row["occurrenceFullKey"], row["carrierKey"])
            for row in manifest["coreProjections"]
        }
        core_clips = {
            row["clipOccurrenceId"]
            for row in manifest["candidateDocuments"]
            if row["coreProjectionCount"] > 0
        }
        systems = {
            system["sourceSystemId"]: {
                carrier["carrierKey"]
                for carrier in system["carriers"]
                if carrier["disposition"] == "EXECUTABLE_CORE"
            }
            for system in self.inventory["sourceSystems"]
        }
        current_pairs = {
            (occurrence["fullKey"], carrier_key)
            for occurrence in self.inventory["occurrences"]
            if occurrence["reachabilityDisposition"] == "REACHABLE_REVIEWED"
            and occurrence["clipOccurrenceId"] in core_clips
            for carrier_key in systems.get(occurrence["sourceSystemId"], set())
        }
        self.assertEqual(160, len(receipt_pairs))
        self.assertTrue(receipt_pairs.issubset(current_pairs))
        self.assertGreater(len(current_pairs), len(receipt_pairs))

    def test_carrier_v1_successor_uses_exact_sprite_mesh_and_decal_shapes(self) -> None:
        summary = self.successor["summary"]
        self.assertEqual(660, summary["reviewedCoreProjectionCount"])
        self.assertEqual(455, summary["reviewedCoreSpriteProjectionCount"])
        self.assertEqual(173, summary["reviewedCoreMeshProjectionCount"])
        self.assertEqual(32, summary["reviewedCoreDecalProjectionCount"])
        self.assertEqual(657, summary["materializedProjectionCount"])
        self.assertEqual(
            0,
            self.successor["productReset"]["duplicateClipOccurrenceOwnerCount"],
        )

    def test_historical_product_owners_are_retired_except_red_blade(self) -> None:
        historical_ids = {
            row["effectAssetId"] for row in self.receipt["documents"]
        }
        live_ids = {row["effectAssetId"] for row in self.catalog["effects"]}
        self.assertEqual(
            {"effect.valtan.red-blade-wave.active"},
            historical_ids.intersection(live_ids),
        )
        live_cue_effects = {row["effectAssetId"] for row in self.cues["cues"]}
        self.assertNotIn("effect.valtan.red-blade-wave.active", live_cue_effects)

        mappings = {
            row["retiredEffectAssetId"]
            for row in self.successor["retiredOwnerSuccessorMappings"]
        }
        self.assertTrue(
            historical_ids.difference({"effect.valtan.red-blade-wave.active"})
            .issubset(mappings)
        )

    def test_unrelated_live_successor_does_not_reseal_history(self) -> None:
        catalog = copy.deepcopy(self.catalog)
        catalog["effects"].append(
            {
                "effectAssetId": "effect.valtan.four-slash.active.clip-02",
                "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
                "authoringPath": (
                    "Effects/Authored/"
                    "effect.valtan.four-slash.active.clip-02.effect.json"
                ),
            }
        )
        subject.validate_carrier_v1_successor(
            self.receipt,
            cue_document=self.cues,
            catalog_document=catalog,
        )

    def test_restoring_an_exact_historical_owner_fails_closed(self) -> None:
        catalog = copy.deepcopy(self.catalog)
        catalog["effects"].append(
            {
                "effectAssetId": "effect.valtan.four-slash.active",
                "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
                "authoringPath": (
                    "Effects/Authored/effect.valtan.four-slash.active.effect.json"
                ),
            }
        )
        with self.assertRaisesRegex(
            subject.CandidateError, "historical reviewed owner was restored"
        ):
            subject.validate_carrier_v1_successor(
                self.receipt,
                cue_document=self.cues,
                catalog_document=catalog,
            )

    def test_historical_receipt_matches_its_original_schema(self) -> None:
        schema = json.loads(SCHEMA_PATH.read_text(encoding="utf-8"))
        schema_validator.validate_schema_instance(self.receipt, schema)
        try:
            import jsonschema
        except ImportError:
            self.skipTest("jsonschema is not installed")
        jsonschema.Draft202012Validator.check_schema(schema)
        jsonschema.validate(self.receipt, schema)


if __name__ == "__main__":
    unittest.main()
