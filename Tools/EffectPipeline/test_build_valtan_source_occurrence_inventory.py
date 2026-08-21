from __future__ import annotations

import copy
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).with_name("build_valtan_source_occurrence_inventory.py")
SCHEMA = SCRIPT.parent / "Schemas/lostark.valtan-source-occurrence-inventory.schema.json"
SPEC = importlib.util.spec_from_file_location("valtan_source_inventory", SCRIPT)
assert SPEC and SPEC.loader
INVENTORY = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(INVENTORY)


class ValtanSourceOccurrenceInventoryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.document = INVENTORY.build_inventory()
        cls.payload = INVENTORY.pretty_json_bytes(cls.document)

    def test_first_inventory_is_31_of_33_and_never_counts_branch_upper_bound(self) -> None:
        summary = self.document["summary"]
        self.assertEqual(summary["encounterPatternCount"], 33)
        self.assertEqual(summary["actionBindingPatternCount"], 31)
        self.assertEqual(summary["missingActionBindingPatternCount"], 2)
        self.assertGreater(summary["branchCandidateCount"], 0)
        self.assertEqual(summary["sourceSequenceCandidateCount"], 227)
        self.assertEqual(
            summary["branchCandidateCount"],
            summary["sourceSequenceCandidateCount"],
        )
        self.assertGreater(summary["branchCarrierUpperBound"], 0)
        self.assertEqual(summary["reviewedSelectedBranchCount"], 0)
        self.assertEqual(summary["reviewedSelectedSequenceCount"], 0)
        self.assertEqual(summary["completionCarrierDenominator"], 0)
        self.assertEqual(summary["encounterStageActionCount"], 127)
        self.assertEqual(summary["patternBindingActionCount"], 124)
        self.assertEqual(summary["patternBindingGapCount"], 3)
        self.assertEqual(summary["patternBindingGapProposalCount"], 3)
        self.assertEqual(summary["droppedOccurrenceCount"], 0)
        self.assertEqual(summary["duplicateOccurrenceCount"], 0)
        self.assertEqual(summary["droppedCarrierCount"], 0)
        self.assertEqual(summary["duplicateCarrierCount"], 0)
        self.assertEqual(summary["sourcePrimitiveDecodedCarrierCount"], 1044)
        self.assertEqual(summary["portableModuleReadyCarrierCount"], 904)
        self.assertEqual(summary["drawableRuntimeReadyCarrierCount"], 551)
        self.assertEqual(summary["missingRuntimeResourceCarrierCount"], 350)
        self.assertEqual(
            summary["portableRuntimeAdapterBlockedCarrierCount"], 119
        )
        self.assertEqual(summary["runtimeResourceBoundCarrierCount"], 708)
        self.assertEqual(summary["runtimeResourceBindingCount"], 1611)
        runtime_cook = self.document["sources"]["runtimeCookReceipt"]
        self.assertEqual(runtime_cook["assetCount"], 398)
        self.assertEqual(runtime_cook["verifiedRuntimeFileCount"], 398)
        self.assertEqual(runtime_cook["failureCount"], 0)
        missing = {
            row["patternId"]: row["status"]
            for row in self.document["coverage"]["missingActionBindingPatterns"]
        }
        self.assertEqual(missing, INVENTORY.MISSING_ACTION_BINDING_POLICIES)
        proposals = self.document["bindingGapProposals"]
        self.assertEqual(len(proposals), 3)
        self.assertEqual(
            {row["actionId"] for row in proposals},
            {
                "valtan.mechanic.arena-floor-84.windup",
                "valtan.mechanic.arena-floor-84.impact",
                "valtan.mechanic.arena-floor-84.recovery",
            },
        )
        for row in proposals:
            self.assertEqual(
                row["status"],
                "PROPOSED_SOURCE_REVIEWED_DELTA_NOT_ACCEPTED",
            )
            self.assertEqual(
                row["proposedClip"]["mappingBasis"],
                "SOURCE_REVIEWED_DELTA",
            )

        dash = self.document["sourceActionEvidenceProposals"]
        self.assertEqual(len(dash), 1)
        self.assertEqual(dash[0]["patternId"], "VALTAN_DASH_CHARGE")
        self.assertEqual(dash[0]["currentSourceActionIds"], [420604])
        self.assertEqual(dash[0]["proposedSourceActionId"], 400424)
        self.assertEqual(dash[0]["sequenceIndex"], 0)
        self.assertEqual(dash[0]["mappingBasis"], "SOURCE_REVIEWED_DELTA")
        self.assertEqual(
            dash[0]["status"],
            "PROPOSED_SOURCE_REVIEWED_DELTA_NOT_ACCEPTED",
        )
        self.assertGreaterEqual(len(dash[0]["patternPreviewReferences"]), 1)
        self.assertEqual(
            {
                row["confidence"]
                for row in dash[0]["patternPreviewReferences"]
            },
            {"USER_CONFIRMED_FAMILY"},
        )
        self.assertEqual(
            dash[0]["proposedSourceAction"]["stages"][0][
                "animationClips"
            ],
            ["Att_Battle_4_01"],
        )

    def test_reviewed_visual_signature_equivalence_keeps_every_sequence(self) -> None:
        reviews = {
            row["patternId"]: row
            for row in self.document["sourceVisualSignatureReviews"]
        }
        expected = {
            "VALTAN_SWING": [(420601, 0), (420660, 0)],
            "VALTAN_IMPRISON_ROAR": [(420603, 0), (420603, 2), (420603, 3)],
            "VALTAN_PARRY": [(420606, 0), (420606, 2), (420606, 3)],
            "VALTAN_STOMP": [(420611, 0), (420611, 2), (420611, 3)],
        }
        for pattern_id, identities in expected.items():
            review = reviews[pattern_id]
            self.assertEqual(
                review["status"], "SOURCE_VISUAL_SIGNATURE_EQUIVALENT"
            )
            candidates = review["equivalentCandidates"]
            self.assertEqual(
                [
                    (row["sourceActionId"], row["sequenceIndex"])
                    for row in candidates
                ],
                identities,
            )
            self.assertEqual(review["canonicalCandidate"], candidates[0])
            self.assertEqual(
                len(
                    {
                        row["sourceVisualFamilySignatureSha256"]
                        for row in candidates
                    }
                ),
                1,
            )
            for row in candidates:
                self.assertRegex(
                    row["sourceSequencePathSha256"], r"^[0-9a-f]{64}$"
                )
                self.assertTrue(row["sourceVisualFamilyMemberSha256s"])

        stomp = reviews["VALTAN_STOMP"]
        self.assertEqual(
            stomp["admissionDisposition"],
            "SOURCE_TIMING_REVIEW_REQUIRED",
        )
        bind = reviews["VALTAN_BIND_CHARGE_SMASH"]
        self.assertEqual(
            bind["status"], "AMBIGUOUS_SOURCE_VISUAL_SIGNATURE"
        )
        self.assertIsNone(bind["canonicalCandidate"])
        self.assertGreater(
            len(bind["candidateVisualSignatureSha256s"]), 1
        )
        self.assertEqual(
            bind["admissionDisposition"],
            "UNRESOLVED_BRANCH_SELECTION",
        )

    def test_additional_twelve_pattern_audit_is_fail_closed(self) -> None:
        audit = self.document["additionalSourceSelectionAudit"]
        self.assertEqual(
            [row["patternId"] for row in audit],
            INVENTORY.ADDITIONAL_SOURCE_SELECTION_REVIEW_ORDER,
        )
        safe = {
            row["patternId"]: row
            for row in audit
            if row["eligibilityDisposition"]
            == "SAFE_EXACT_EQUIVALENT_FULL_SEQUENCE_JOIN"
        }
        self.assertEqual(
            set(safe),
            {"VALTAN_SWING", "VALTAN_IMPRISON_ROAR", "VALTAN_PARRY"},
        )
        # setUpClass intentionally builds without a selection manifest.
        self.assertTrue(
            all(
                row["selectionStatus"] == "UNRESOLVED_BRANCH_SELECTION"
                and row["selectedBranchId"] is None
                and row["unresolvedReason"]
                for row in safe.values()
            )
        )
        unresolved = [
            row
            for row in audit
            if row["eligibilityDisposition"]
            != "SAFE_EXACT_EQUIVALENT_FULL_SEQUENCE_JOIN"
        ]
        self.assertEqual(len(unresolved), 9)
        self.assertTrue(
            all(
                row["selectedBranchId"] is None
                and row["unresolvedReason"]
                for row in unresolved
            )
        )

    def test_checked_selection_manifest_admits_only_three_safe_followups(self) -> None:
        selection_path = (
            INVENTORY.ROOT
            / "Data/Effects/Imported/Valtan/"
            "Valtan.priority-source-sequence-selections.v1.json"
        )
        manifest = INVENTORY.load_selection_manifest(selection_path)
        selected = INVENTORY.build_inventory(
            {"reviewedBranchSelections": manifest["selections"]}
        )
        audit = {
            row["patternId"]: row
            for row in selected["additionalSourceSelectionAudit"]
        }
        selected_followups = {
            pattern_id
            for pattern_id, row in audit.items()
            if row["selectionStatus"] == "REVIEWED_SELECTED"
        }
        self.assertEqual(
            selected_followups,
            {"VALTAN_SWING", "VALTAN_IMPRISON_ROAR", "VALTAN_PARRY"},
        )
        for pattern_id in selected_followups:
            self.assertIsNone(audit[pattern_id]["unresolvedReason"])
            self.assertEqual(
                audit[pattern_id]["selectedBranchId"],
                audit[pattern_id]["candidateBranchId"],
            )
        self.assertEqual(
            selected["summary"]["reviewedSelectedBranchCount"], 24
        )
        self.assertEqual(
            selected["summary"]["completionCarrierDenominator"], 630
        )

    def test_branch_stage_paths_and_full_keys_are_not_clip_deduplicated(self) -> None:
        portal = [
            row
            for row in self.document["branches"]
            if row["patternId"] == "VALTAN_PORTAL_RUSH"
            and row["sourceActionId"] == 420622
        ]
        self.assertEqual([row["sourceStageStartIndex"] for row in portal], [0, 8, 15])
        self.assertEqual([row["sourceStageEndIndex"] for row in portal], [7, 14, 21])
        self.assertEqual([row["sequenceIndex"] for row in portal], [1, 2, 3])
        self.assertEqual(
            [row["recommendationStatus"] for row in portal],
            [
                "UNIQUE_BEST_CANDIDATE",
                "ALTERNATE_BRANCH_CANDIDATE",
                "ALTERNATE_BRANCH_CANDIDATE",
            ],
        )
        occurrences = [
            row
            for row in self.document["occurrences"]
            if row["patternId"] == "VALTAN_PORTAL_RUSH"
            and row["sourceActionId"] == 420622
        ]
        branch_ids = {row["branchId"] for row in occurrences}
        self.assertEqual(branch_ids, {row["branchId"] for row in portal})
        keys = [row["fullKey"] for row in occurrences]
        self.assertEqual(len(keys), len(set(keys)))
        for row in occurrences:
            self.assertRegex(row["fullKey"], r"^occurrence-key\.[0-9a-f]{64}$")
            self.assertTrue(row["branchId"])
            self.assertTrue(row["sourceStagePath"])
            self.assertTrue(row["notifyId"])
            self.assertEqual(
                row["branchSelectionStatus"], "UNRESOLVED_BRANCH_SELECTION"
            )

    def test_clipseq_paths_cross_main_resets_without_splitting(self) -> None:
        expected = {
            ("VALTAN_FOUR_SLASH", 420609, 3): [8, 9, 10],
            ("VALTAN_DOWN_SMASH", 420602, 3): [14, 15, 16],
            ("VALTAN_EARTHQUAKE_SMASH", 420605, 3): [18, 19, 20, 21],
            ("VALTAN_GROUND_WAVE_SMASH", 420615, 5): [29, 30, 31, 32, 33, 34],
        }
        for identity, source_stage_path in expected.items():
            pattern_id, source_action_id, sequence_index = identity
            matches = [
                row
                for row in self.document["branches"]
                if row["patternId"] == pattern_id
                and row["sourceActionId"] == source_action_id
                and row["sequenceIndex"] == sequence_index
            ]
            self.assertEqual(len(matches), 1, identity)
            self.assertEqual(matches[0]["sourceStagePath"], source_stage_path)
            self.assertRegex(
                matches[0]["sourceSequencePathSha256"], r"^[0-9a-f]{64}$"
            )

    def test_core_closure_keeps_every_selected_lod_emitter_and_module_occurrence(self) -> None:
        carrier_keys = []
        dispositions = set()
        for system in self.document["sourceSystems"]:
            self.assertEqual(system["droppedCarrierCount"], 0)
            self.assertEqual(system["duplicateCarrierCount"], 0)
            self.assertEqual(
                len(system["carriers"]),
                system["selectedLodCarrierCount"]
                + system["preservedUnresolvedEmitterCount"],
            )
            for carrier in system["carriers"]:
                carrier_keys.append(carrier["carrierKey"])
                dispositions.add(carrier["disposition"])
                module_rows = carrier["orderedModuleOccurrences"]
                self.assertEqual(
                    [row["ordinal"] for row in module_rows],
                    list(range(len(module_rows))),
                )
                self.assertEqual(
                    carrier["moduleOrderSha256"],
                    INVENTORY.canonical_sha256(module_rows),
                )
                if carrier["disposition"] == "EXECUTABLE_CORE":
                    self.assertIsNotNone(carrier["sourceRecipeSummary"])
                    self.assertRegex(
                        carrier["sourceRecipeSha256"], r"^[0-9a-f]{64}$"
                    )
                    self.assertRegex(
                        carrier["portableSourceRecipeSha256"],
                        r"^[0-9a-f]{64}$",
                    )
                    self.assertEqual(
                        carrier["conversionStatus"],
                        "PORTABLE_RUNTIME_CARRIER_READY",
                    )
                    self.assertGreater(
                        carrier["runtimeResourceBindingCount"], 0
                    )
                    self.assertNotIn("sourceRecipe", carrier)
                    self.assertNotIn("elementSeed", carrier)
                else:
                    self.assertNotIn("elementSeed", carrier)
        self.assertEqual(len(carrier_keys), len(set(carrier_keys)))
        self.assertIn("EXECUTABLE_CORE", dispositions)
        self.assertIn("DEFERRED_LIGHT", dispositions)
        self.assertIn("DEFERRED_GENERIC_DUST", dispositions)
        self.assertIn("UNRESOLVED_RUNTIME_ADAPTER", dispositions)

    def test_animation_trail_and_ribbon_are_not_admitted_as_core(self) -> None:
        summary = self.document["summary"]
        self.assertEqual(summary["animationTrailOccurrenceCount"], 259)
        self.assertEqual(summary["trailGhostOccurrenceCount"], 100)
        self.assertEqual(summary["ribbonCarrierCount"], 6)
        self.assertEqual(summary["ribbonSourceSystemCount"], 6)
        self.assertEqual(summary["ribbonReferencedOccurrenceCount"], 109)
        self.assertEqual(summary["ribbonBlockedOccurrenceCount"], 27)
        self.assertEqual(summary["unresolvedRuntimeAdapterCarrierCount"], 125)
        self.assertEqual(
            summary["unresolvedRuntimeAdapterOccurrenceCount"], 1931
        )
        for system in self.document["sourceSystems"]:
            for carrier in system["carriers"]:
                if carrier["runtimeAdapterType"] == "RIBBON":
                    self.assertEqual(
                        carrier["disposition"], "UNRESOLVED_RUNTIME_ADAPTER"
                    )
                    self.assertIn(
                        "RIBBON_RUNTIME_ADAPTER_UNAVAILABLE",
                        carrier["conversionBlockers"],
                    )

    def test_portable_runtime_admission_is_carrier_specific_and_fail_closed(self) -> None:
        portal_system = next(
            row
            for row in self.document["sourceSystems"]
            if row["sourceSystemId"]
            == "fx_mn_rpbf_00_n.par_n_rpbf_potal_02_01"
        )
        dispositions = {
            row["disposition"] for row in portal_system["carriers"]
        }
        self.assertIn("EXECUTABLE_CORE", dispositions)
        self.assertIn("MISSING_RUNTIME_RESOURCE", dispositions)
        for carrier in portal_system["carriers"]:
            if carrier["disposition"] == "MISSING_RUNTIME_RESOURCE":
                self.assertEqual(
                    carrier["conversionStatus"], "MISSING_RUNTIME_RESOURCE"
                )
                self.assertIn(
                    "DRAWABLE_BASE_OR_MESH_RUNTIME_BINDING_MISSING",
                    carrier["conversionBlockers"],
                )
            if carrier["disposition"] == "EXECUTABLE_CORE":
                self.assertEqual(
                    carrier["conversionStatus"],
                    "PORTABLE_RUNTIME_CARRIER_READY",
                )

    def test_unresolved_decal_and_effect_payloads_are_preserved(self) -> None:
        unresolved = [
            row
            for row in self.document["occurrences"]
            if row["category"] in {"decal", "unresolved_effect"}
            and row["disposition"] == "UNRESOLVED_SOURCE_PAYLOAD"
        ]
        self.assertEqual(
            len(unresolved), self.document["summary"]["unresolvedDecalOrEffectCount"]
        )
        self.assertGreater(len(unresolved), 0)
        for row in unresolved:
            self.assertIsNone(row["assetReference"])
            self.assertEqual(row["expandedCarrierCount"], 0)
            self.assertEqual(
                row["expandedCarrierFullKeysSha256"],
                INVENTORY.canonical_sha256([]),
            )

    def test_reachability_is_separate_from_source_disposition(self) -> None:
        reachability = self.document["summary"][
            "sourceOccurrenceReachabilityCounts"
        ]
        self.assertGreater(reachability["UNRESOLVED_BRANCH_SELECTION"], 0)
        self.assertNotIn("UNREACHABLE_SOURCE_OCCURRENCE", reachability)
        self.assertNotIn("REACHABLE_REVIEWED", reachability)
        dispositions = self.document["summary"][
            "sourceOccurrenceDispositionCounts"
        ]
        for required in (
            "EXECUTABLE_CORE",
            "DEFERRED_LIGHT",
            "DEFERRED_GENERIC_DUST",
            "UNRESOLVED_SOURCE_PAYLOAD",
        ):
            self.assertGreater(dispositions[required], 0)

    def test_selected_branch_requires_exact_source_to_clip_occurrence_mapping(self) -> None:
        branch_id = (
            "valtan_portal_rush.source-420622.mn_rpbf_00."
            "sequence-001.stages-000-007"
        )
        source_sequence_sha = next(
            row["sourceSequencePathSha256"]
            for row in self.document["branches"]
            if row["branchId"] == branch_id
        )
        mappings = [
            (0, "valtan.attack.portal-rush.portal.clip.01", "REACHABLE"),
            (1, "valtan.attack.portal-rush.rushes.clip.01", "REACHABLE"),
            (
                5,
                "valtan.attack.portal-rush.finish.clip.01",
                "SOURCE_TIMING_REVIEW_REQUIRED",
            ),
            (6, "valtan.attack.portal-rush.recovery.clip.01", "REACHABLE"),
        ]
        previous = {
            "reviewedBranchSelections": [
                {
                    "patternId": "VALTAN_PORTAL_RUSH",
                    "sourceActionId": 420622,
                    "profileId": "MN_RPBF_00",
                    "sequenceIndex": 1,
                    "sourceSequencePathSha256": source_sequence_sha,
                    "branchId": branch_id,
                    "status": "REVIEWED_SELECTED",
                    "reviewBasis": "unit-test exact Portal source stage review",
                    "stageMappings": [
                        {
                            "sourceStageIndex": stage,
                            "sourceClipOrdinal": 0,
                            "clipOccurrenceId": occurrence_id,
                            "timingDisposition": timing,
                        }
                        for stage, occurrence_id, timing in mappings
                    ],
                }
            ]
        }
        selected = INVENTORY.build_inventory(previous)
        portal = [
            row
            for row in selected["occurrences"]
            if row["branchId"] == branch_id
        ]
        reachable = [
            row
            for row in portal
            if row["reachabilityDisposition"] == "REACHABLE_REVIEWED"
        ]
        self.assertGreater(len(reachable), 0)
        self.assertEqual(
            {row["sourceStageIndex"] for row in reachable}, {0, 1, 6}
        )
        for row in reachable:
            self.assertEqual(row["timingDisposition"], "REACHABLE")
            self.assertIsNotNone(row["clipOccurrenceId"])
            self.assertIn(
                row["clipOccurrenceId"], row["candidateClipOccurrenceIds"]
            )
        unresolved = [
            row
            for row in portal
            if row["reachabilityDisposition"]
            == "UNRESOLVED_CLIP_OCCURRENCE_MAPPING"
        ]
        self.assertGreater(len(unresolved), 0)
        timing_review = [
            row
            for row in portal
            if row["reachabilityDisposition"]
            == "SOURCE_TIMING_REVIEW_REQUIRED"
        ]
        self.assertGreater(len(timing_review), 0)
        self.assertEqual(
            {row["sourceStageIndex"] for row in timing_review}, {5}
        )
        self.assertGreater(
            selected["summary"]["completionCarrierDenominator"], 0
        )

        selected_without_mappings = copy.deepcopy(previous)
        selected_without_mappings["reviewedBranchSelections"][0][
            "stageMappings"
        ] = []
        unresolved_document = INVENTORY.build_inventory(
            selected_without_mappings
        )
        self.assertEqual(
            unresolved_document["summary"]["completionCarrierDenominator"],
            0,
        )
        self.assertGreater(
            unresolved_document["summary"][
                "sourceOccurrenceReachabilityCounts"
            ]["UNRESOLVED_CLIP_OCCURRENCE_MAPPING"],
            0,
        )

    def test_reviewed_mapping_rejects_same_name_guess_to_wrong_clip(self) -> None:
        branch_id = (
            "valtan_portal_rush.source-420622.mn_rpbf_00."
            "sequence-001.stages-000-007"
        )
        source_sequence_sha = next(
            row["sourceSequencePathSha256"]
            for row in self.document["branches"]
            if row["branchId"] == branch_id
        )
        previous = {
            "reviewedBranchSelections": [
                {
                    "patternId": "VALTAN_PORTAL_RUSH",
                    "sourceActionId": 420622,
                    "profileId": "MN_RPBF_00",
                    "sequenceIndex": 1,
                    "sourceSequencePathSha256": source_sequence_sha,
                    "branchId": branch_id,
                    "status": "REVIEWED_SELECTED",
                    "reviewBasis": "negative fixture",
                    "stageMappings": [
                        {
                            "sourceStageIndex": 0,
                            "sourceClipOrdinal": 0,
                            "clipOccurrenceId": (
                                "valtan.attack.portal-rush.rushes.clip.01"
                            ),
                            "timingDisposition": "REACHABLE",
                        }
                    ],
                }
            ]
        }
        with self.assertRaises(INVENTORY.InventoryError):
            INVENTORY.build_inventory(previous)

    def test_rebuild_is_byte_deterministic(self) -> None:
        rebuilt = INVENTORY.build_inventory(copy.deepcopy(self.document))
        self.assertEqual(INVENTORY.pretty_json_bytes(rebuilt), self.payload)

    def test_check_is_read_only_and_detects_drift(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "inventory.json"
            path.write_bytes(self.payload)
            before = (path.stat().st_mtime_ns, path.stat().st_size)
            INVENTORY.check_exact(path, self.payload)
            after = (path.stat().st_mtime_ns, path.stat().st_size)
            self.assertEqual(after, before)
            with self.assertRaises(INVENTORY.InventoryError):
                INVENTORY.check_exact(path, self.payload + b" ")

    def test_reconcile_is_missing_only_and_reports_legacy_rows(self) -> None:
        legacy = {
            "id": "par_old.em00",
            "sourceNode": "legacy",
            "detail": {
                "transform": {"scale": [1.0, 1.0, 1.0]},
                "particle": {
                    "maxParticles": 8,
                    "burstCount": 8,
                    "lifeTimeSeconds": [0.6, 1.0],
                },
            },
            "sourceRecipe": {"enabled": False},
        }
        existing_source = {
            "id": "existing",
            "sourceNode": "source-key-1",
            "material": {"sourceMaterialPath": "fx.material"},
            "sourceRecipe": {"enabled": True, "modules": []},
        }
        existing = {
            "effectAssetId": "effect.test",
            "elements": [legacy, existing_source],
        }
        candidate_existing = copy.deepcopy(existing_source)
        candidate_existing["detail"] = {"color": {"multiply": [9, 9, 9, 9]}}
        candidate_new = copy.deepcopy(existing_source)
        candidate_new["id"] = "new"
        candidate_new["sourceNode"] = "source-key-2"
        plan = INVENTORY.reconcile_effect_document(
            existing, [candidate_existing, candidate_new]
        )
        self.assertEqual(plan["preservedExistingElementCount"], 2)
        self.assertEqual([row["id"] for row in plan["addElements"]], ["new"])
        self.assertEqual(plan["deleteElements"], [])
        self.assertEqual(plan["sourceRebaseRequired"], [])
        self.assertEqual(plan["legacyGenericRetireCandidates"], ["par_old.em00"])
        self.assertEqual(existing["elements"], [legacy, existing_source])

    def test_reconcile_fails_closed_on_compiler_owned_source_drift(self) -> None:
        existing = {
            "effectAssetId": "effect.test",
            "elements": [
                {
                    "id": "existing",
                    "sourceNode": "source-key",
                    "material": {"sourceMaterialPath": "fx.material"},
                    "sourceRecipe": {"enabled": True, "modules": [1]},
                }
            ],
        }
        candidate = copy.deepcopy(existing["elements"][0])
        candidate["sourceRecipe"]["modules"] = [2]
        plan = INVENTORY.reconcile_effect_document(existing, [candidate])
        self.assertEqual(plan["addElements"], [])
        self.assertEqual(plan["sourceRebaseRequired"], ["source-key"])
        self.assertEqual(plan["deleteElements"], [])

    def test_schema_is_valid_json_and_names_the_same_contract(self) -> None:
        schema = json.loads(SCHEMA.read_text(encoding="utf-8"))
        self.assertEqual(
            schema["properties"]["schema"]["const"], self.document["schema"]
        )
        self.assertEqual(
            schema["properties"]["formatVersion"]["const"],
            self.document["formatVersion"],
        )


if __name__ == "__main__":
    unittest.main()
