#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import math
import sys
import tempfile
import unittest
from pathlib import Path


TOOLS = Path(__file__).resolve().parent
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import build_valtan_reviewed_source_family_candidates as candidates
import build_valtan_source_occurrence_inventory as inventory
import validate_boss_pattern_effects as schema_validator


SCHEMA_PATH = (
    TOOLS
    / "Schemas/lostark.valtan-reviewed-source-family-candidates.schema.json"
)


class ValtanReviewedSourceFamilyCandidateTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.inventory = candidates.load_inventory(candidates.SELECTION_PATH)
        cls.files, cls.receipt = candidates.build_candidates(
            inventory_document=cls.inventory
        )

    def test_canonical_reviewed_projection_denominator_is_fixed(self) -> None:
        summary = self.receipt["summary"]
        for field, expected in candidates.EXPECTED_COUNTS.items():
            self.assertEqual(expected, summary[field], field)
        self.assertEqual(
            summary["reachableCoreProjectionCount"],
            summary["admittedCoreProjectionCount"]
            + summary["protectedCanaryProjectionCount"]
            + summary["missingCueProjectionCount"]
            + summary["multipleCueProjectionCount"]
            + summary["cueIdentityMismatchProjectionCount"]
            + summary["negativeTimingProjectionCount"]
            + summary["outsideCueWindowProjectionCount"]
            + summary["effectAssetReuseDivergenceProjectionCount"],
        )
        self.assertEqual(24, len(self.receipt["patternCoverage"]))
        self.assertEqual(37, len(self.receipt["documents"]))

    def test_every_element_delay_is_exactly_cue_local(self) -> None:
        for row in self.receipt["documents"]:
            document = json.loads(
                self.files[row["candidateDocumentPath"]].decode("utf-8")
            )
            candidates.validate_candidate_document(
                document, row["effectAssetId"]
            )
            by_source = {
                element["sourceNode"]: element
                for element in document["elements"]
            }
            self.assertEqual(
                row["candidateElementCount"], len(row["sourceElementKeys"])
            )
            for source_key in row["sourceElementKeys"]:
                expected = source_key["sourceTimeSeconds"] - (
                    source_key["cueSourceStartMs"] / 1000.0
                )
                self.assertGreaterEqual(expected, 0.0)
                self.assertTrue(math.isfinite(expected))
                self.assertEqual(
                    expected, source_key["elementStartDelaySeconds"]
                )
                element = by_source[source_key["sourceNode"]]
                self.assertEqual(
                    expected,
                    element["detail"]["timing"]["startDelaySeconds"],
                )
                cue_end = source_key["cueSourceEndMs"]
                if cue_end is not None:
                    self.assertLess(
                        source_key["sourceTimeSeconds"], cue_end / 1000.0
                    )

    def test_front_back_front_keeps_four_visual_waves_and_system_groups(self) -> None:
        row = next(
            document
            for document in self.receipt["documents"]
            if document["effectAssetId"]
            == "effect.valtan.front-back-front.active"
        )
        visual_groups = row["visualTimingGroups"]
        notify_system_groups = row["notifySystemTimingGroups"]
        self.assertEqual(4, len(visual_groups))
        self.assertEqual(12, len(notify_system_groups))
        self.assertEqual(
            [
                1.1699999570846558,
                2.2536299228668213,
                3.224159002304077,
                4.220053195953369,
            ],
            sorted(group["sourceTimeSeconds"] for group in visual_groups),
        )
        self.assertEqual(
            [25, 25, 25, 25],
            sorted(group["elementCount"] for group in visual_groups),
        )
        self.assertTrue(
            all(
                len(group["notifySystemTimingGroupIds"]) == 3
                and len(group["notifyIds"]) == 3
                and len(group["sourceSystemIds"]) == 3
                for group in visual_groups
            )
        )
        self.assertEqual(
            {8, 9},
            {group["elementCount"] for group in notify_system_groups},
        )

    def test_three_safe_followup_selection_outcomes_are_explicit(self) -> None:
        coverage = {
            row["patternId"]: row
            for row in self.receipt["patternCoverage"]
        }
        imprison = coverage["VALTAN_IMPRISON_ROAR"]
        self.assertEqual(
            imprison["status"], "IMMUTABLE_CANDIDATES_EMITTED"
        )
        self.assertEqual(imprison["reachableCoreProjectionCount"], 30)
        self.assertEqual(imprison["admittedCoreProjectionCount"], 30)
        self.assertEqual(imprison["candidateEffectAssetCount"], 3)

        parry = coverage["VALTAN_PARRY"]
        self.assertEqual(parry["status"], "IMMUTABLE_CANDIDATES_EMITTED")
        self.assertEqual(parry["reachableCoreProjectionCount"], 16)
        self.assertEqual(parry["admittedCoreProjectionCount"], 16)
        self.assertEqual(parry["candidateEffectAssetCount"], 2)

        swing = coverage["VALTAN_SWING"]
        self.assertEqual(
            swing["status"],
            "ALL_CORE_PROJECTIONS_EXPLICITLY_BLOCKED_OR_EXCLUDED",
        )
        self.assertEqual(swing["reachableCoreProjectionCount"], 148)
        self.assertEqual(swing["missingCueProjectionCount"], 141)
        self.assertEqual(swing["negativeTimingProjectionCount"], 7)
        self.assertEqual(swing["candidateEffectAssetCount"], 0)

    def test_negative_and_explicit_end_starts_are_rejected(self) -> None:
        occurrence = {"sourceTimeSeconds": 1.0}
        cue = {"sourceStartMs": 1000, "sourceEndMs": 1200}
        self.assertEqual(
            ("ADMITTED", 0.0),
            candidates.cue_local_start_delay_seconds(occurrence, cue),
        )
        occurrence["sourceTimeSeconds"] = 0.999
        disposition, delay = candidates.cue_local_start_delay_seconds(
            occurrence, cue
        )
        self.assertEqual("NEGATIVE_BEFORE_CUE_START", disposition)
        self.assertLess(delay, 0.0)
        occurrence["sourceTimeSeconds"] = 1.2
        self.assertEqual(
            "OUTSIDE_EXPLICIT_CUE_WINDOW",
            candidates.cue_local_start_delay_seconds(occurrence, cue)[0],
        )
        self.assertEqual(
            86, len(self.receipt["rejectedTimingProjections"])
        )
        self.assertTrue(
            all(
                row["disposition"] == "NEGATIVE_BEFORE_CUE_START"
                for row in self.receipt["rejectedTimingProjections"]
            )
        )

    def test_whirlwind_active_is_a_byte_protected_exclusion(self) -> None:
        protected = self.receipt["protectedCanaries"]
        self.assertEqual(1, len(protected))
        row = protected[0]
        self.assertEqual(
            "effect.valtan.pattern.420633.active", row["effectAssetId"]
        )
        path = candidates.ROOT / row["authoredDocumentPath"]
        self.assertEqual(row["authoredDocumentSha256"], inventory.sha256_file(path))
        self.assertEqual(3, row["sourceProjectionCount"])
        self.assertNotIn(
            row["effectAssetId"],
            {document["effectAssetId"] for document in self.receipt["documents"]},
        )
        self.assertFalse(
            any(
                row["effectAssetId"] in relative_path
                for relative_path in self.files
            )
        )

    def test_reconcile_is_report_only_and_preserves_every_existing_row(self) -> None:
        summary = self.receipt["summary"]
        self.assertEqual(0, summary["deletedElementCount"])
        self.assertEqual(0, summary["sourceRebaseRequiredCount"])
        self.assertEqual(1093, summary["legacyGenericRetireCandidateCount"])
        self.assertEqual(4, summary["preservedExistingSourceOrImportedRowCount"])
        for row in self.receipt["documents"]:
            plan = row["reconcile"]
            self.assertEqual([], plan["deleteElements"])
            self.assertEqual(
                plan["existingElementCount"],
                plan["preservedExistingElementCount"],
            )
            self.assertEqual(
                "REPORT_ONLY_UNVERIFIED_DEFAULT_SIGNATURE_NO_DELETE",
                plan["legacyRetirementDisposition"],
            )
            self.assertEqual(
                plan["existingElementCount"],
                len(plan["preservedExistingRows"]),
            )

    def test_user_tuning_is_preserved_and_source_rebase_is_visible(self) -> None:
        document_row = self.receipt["documents"][0]
        candidate = json.loads(
            self.files[document_row["candidateDocumentPath"]].decode("utf-8")
        )
        existing = {
            "effectAssetId": document_row["effectAssetId"],
            "elements": [copy.deepcopy(candidate["elements"][0])],
        }
        existing["elements"][0]["detail"]["color"][
            "emissiveIntensity"
        ] = 7.0
        plan = inventory.reconcile_effect_document(
            existing, candidate["elements"]
        )
        self.assertEqual(1, plan["preservedExistingElementCount"])
        self.assertEqual([], plan["deleteElements"])
        self.assertEqual(
            7.0,
            existing["elements"][0]["detail"]["color"][
                "emissiveIntensity"
            ],
        )
        self.assertEqual([], plan["sourceRebaseRequired"])

        existing["elements"][0]["sourceRecipe"]["emitterLoopCount"] += 1
        rebase = candidates.compact_reconcile_plan(
            existing,
            candidate,
            candidates.OUTPUT_ROOT
            / candidates.candidate_filename(document_row["effectAssetId"]),
            document_row["sourceElementKeys"],
        )
        self.assertEqual(
            "BLOCKED_SOURCE_REBASE_REQUIRED_NO_MUTATION",
            rebase["applyDisposition"],
        )
        self.assertEqual(1, len(rebase["sourceRebaseRequiredRows"]))
        self.assertEqual([], rebase["deleteElements"])

    def test_multiple_exact_clip_cues_are_not_arbitrarily_selected(self) -> None:
        cues = candidates.read_json(candidates.CUE_PATH)
        target = next(
            row
            for row in cues["cues"]
            if row["clipOccurrenceId"]
            == "valtan.attack.portal-rush.portal.clip.01"
        )
        duplicate = copy.deepcopy(target)
        duplicate["bindingId"] += ".ambiguity-test"
        duplicate["occurrenceId"] += ".ambiguity-test"
        cues["cues"].append(duplicate)
        _files, receipt = candidates.build_candidates(
            inventory_document=self.inventory,
            cue_document=cues,
            enforce_expected_counts=False,
        )
        blockers = [
            row
            for row in receipt["unresolvedCueJoins"]
            if row["reason"]
            == "MULTIPLE_V2_CUES_FOR_EXACT_CLIP_OCCURRENCE"
        ]
        self.assertEqual(1, len(blockers))
        self.assertEqual(14, blockers[0]["executableCoreProjectionCount"])
        self.assertEqual(14, receipt["summary"]["multipleCueProjectionCount"])
        self.assertNotIn(
            "effect.valtan.portal-rush.portal",
            {row["effectAssetId"] for row in receipt["documents"]},
        )

    def test_divergent_effect_asset_reuse_is_fail_visible(self) -> None:
        cues = candidates.read_json(candidates.CUE_PATH)
        portal_effect = "effect.valtan.portal-rush.portal"
        recovery = next(
            row
            for row in cues["cues"]
            if row["clipOccurrenceId"]
            == "valtan.attack.portal-rush.recovery.clip.01"
        )
        recovery["effectAssetId"] = portal_effect
        _files, receipt = candidates.build_candidates(
            inventory_document=self.inventory,
            cue_document=cues,
            enforce_expected_counts=False,
        )
        review = next(
            row
            for row in receipt["effectAssetReuseReviews"]
            if row["effectAssetId"] == portal_effect
        )
        self.assertEqual(
            "DIVERGENT_SOURCE_FAMILIES_NO_CANDIDATE_NO_APPLY",
            review["status"],
        )
        self.assertEqual(19, review["executableCoreProjectionCount"])
        self.assertEqual(
            19,
            receipt["summary"][
                "effectAssetReuseDivergenceProjectionCount"
            ],
        )
        self.assertNotIn(
            portal_effect,
            {row["effectAssetId"] for row in receipt["documents"]},
        )

    def test_build_is_byte_deterministic_from_the_same_pinned_inputs(self) -> None:
        second_files, second_receipt = candidates.build_candidates(
            inventory_document=self.inventory
        )
        self.assertEqual(self.files, second_files)
        self.assertEqual(self.receipt, second_receipt)

    def test_check_exact_is_read_only(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "candidate.json"
            payload = b'{"ok":true}\n'
            path.write_bytes(payload)
            before = (path.stat().st_mtime_ns, path.read_bytes())
            candidates.check_exact(path, payload)
            after = (path.stat().st_mtime_ns, path.read_bytes())
            self.assertEqual(before, after)
            with self.assertRaises(candidates.CandidateError):
                candidates.check_exact(path, b"{}\n")

    def test_receipt_schema_is_valid_json_and_matches_when_jsonschema_exists(self) -> None:
        schema = json.loads(SCHEMA_PATH.read_text(encoding="utf-8"))
        self.assertEqual(
            "lostark.valtan-reviewed-source-family-candidates.schema.json",
            schema["$id"],
        )
        self.assertEqual(
            candidates.EXPECTED_COUNTS["candidateDocumentCount"],
            schema["$defs"]["summary"]["properties"][
                "candidateDocumentCount"
            ]["const"],
        )
        schema_validator.validate_schema_instance(self.receipt, schema)
        try:
            import jsonschema
        except ImportError:
            return
        jsonschema.Draft202012Validator.check_schema(schema)
        jsonschema.validate(self.receipt, schema)


if __name__ == "__main__":
    unittest.main()
