from __future__ import annotations

import copy
import json
import tempfile
import unittest
from pathlib import Path
from unittest import mock

import build_effect_tuple_cohort_inventory as inventory


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
ARTIFACT_PATH = REPOSITORY_ROOT / inventory.OUTPUT_RELATIVE_PATH


class EffectTupleCohortInventoryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.document = inventory.build_inventory(REPOSITORY_ROOT)

    def _rehash(self) -> None:
        self.document.pop("artifactSha256", None)
        self.document["artifactSha256"] = inventory.canonical_sha256(self.document)

    def _assert_mutation_rejected(self, mutate, restore) -> None:
        mutate()
        self._rehash()
        try:
            with self.assertRaises(inventory.InventoryError):
                inventory.validate_inventory(self.document)
        finally:
            restore()
            self._rehash()
            inventory.validate_inventory(self.document)

    def _assert_copy_mutation_rejected(self, mutate) -> None:
        document = copy.deepcopy(self.document)
        mutate(document)
        document.pop("artifactSha256", None)
        document["artifactSha256"] = inventory.canonical_sha256(document)
        with self.assertRaises(inventory.InventoryError):
            inventory.validate_inventory(document)

    def test_real_build_is_byte_current_and_conservative(self) -> None:
        self.assertEqual(
            inventory.pretty_json_bytes(self.document),
            ARTIFACT_PATH.read_bytes(),
        )
        summary = self.document["summary"]
        self.assertEqual(summary["occurrenceCount"], 7566)
        self.assertEqual(
            summary["programStatusCounts"],
            {
                "BOUNDED_SOURCE_PROFILE_ONLY": 525,
                "DXBC_FAMILY_REPRESENTATIVE_ONLY": 3671,
                "DXBC_OCCURRENCE_EXACT": 2148,
                "DXBC_OCCURRENCE_EXACT_UNTRANSLATED": 8,
                "NOT_APPLICABLE_PRESENTATION": 109,
                "NO_PROGRAM_EVIDENCE": 1055,
                "TYPED_RUNTIME_PROGRAM_DECLARED": 50,
            },
        )
        self.assertEqual(
            summary["layoutStatusCounts"],
            {
                "EXACT_VARIANT_NATIVE_WIRE_ONLY_REQUIRES_PACKET_TRANSLATION": 20,
                "NAMED_NATIVE_WIRE_ONLY_REQUIRES_COUNT_EXTENSION": 3153,
                "NAMED_NATIVE_WIRE_ONLY_WITHIN_COUNT_CAPS": 2430,
                "NOT_APPLICABLE_PRESENTATION": 109,
                "SOURCE_NAMES_ONLY": 673,
                "TYPED_PACKET_CLOSED": 50,
                "UNRESOLVED": 1131,
            },
        )
        self.assertEqual(summary["cohortCount"], 340)
        self.assertEqual(
            summary["cohortKindCounts"],
            {"NATIVE_EVIDENCE_COHORT": 321, "TYPED_EXECUTION_COHORT": 19},
        )
        self.assertEqual(
            summary["adapterStatusCounts"],
            {
                "PRESENTATION_SEPARATE": 109,
                "RENDER_PROFILE_STATIC_CANDIDATE": 7385,
                "TYPED_STATIC_DISPATCH_CANDIDATE": 50,
                "UNRESOLVED": 22,
            },
        )
        self.assertEqual(summary["runtimeVerifiedCohortCount"], 0)
        self.assertEqual(
            summary["runtimeDescriptorExpansionEligibleCohortCount"], 0
        )
        self.assertTrue(
            all(cohort["runtimeVerified"] is False for cohort in self.document["cohorts"])
        )
        self.assertTrue(
            all(
                cohort["runtimeDescriptorExpansionEligible"] is False
                for cohort in self.document["cohorts"]
            )
        )

    def test_runtime_catalog_v4_material_registry_uses_publisher_contract(self) -> None:
        catalog_path = REPOSITORY_ROOT / inventory.RUNTIME_CATALOG_PATH
        catalog = json.loads(catalog_path.read_text(encoding="utf-8"))
        inventory._validate_runtime_catalog_contract(catalog, catalog_path)

        legacy_identity = copy.deepcopy(catalog)
        legacy_identity["formatVersion"] = 3
        with self.assertRaises(inventory.InventoryError):
            inventory._validate_runtime_catalog_contract(legacy_identity, catalog_path)

        wrong_registry_identity = copy.deepcopy(catalog)
        wrong_registry_identity["materialPrograms"]["schema"] = "lostark.wrong"
        with self.assertRaises(inventory.InventoryError):
            inventory._validate_runtime_catalog_contract(
                wrong_registry_identity, catalog_path
            )

        hidden_root_field = copy.deepcopy(catalog)
        hidden_root_field["runtimeAdmission"] = True
        with self.assertRaises(inventory.InventoryError):
            inventory._validate_runtime_catalog_contract(hidden_root_field, catalog_path)

    def test_content_addressed_registry_is_order_independent(self) -> None:
        payloads = [
            {"kind": "TEST", "value": 1},
            {"kind": "TEST", "value": 2},
            {"kind": "TEST", "value": 1},
        ]
        forward = inventory.VariantRegistry("test.")
        reverse = inventory.VariantRegistry("test.")
        for payload in payloads:
            forward.add(payload)
        for payload in reversed(payloads):
            reverse.add(payload)
        self.assertEqual(forward.rows("testId"), reverse.rows("testId"))

    def test_descriptor_value_change_does_not_change_tuple_identity(self) -> None:
        authored = json.loads(
            (
                REPOSITORY_ROOT
                / "Data/Effects/Authored/effect.artist.skill.31470.unified.effect.json"
            ).read_text(encoding="utf-8")
        )
        execution = next(
            element["material"]["execution"]
            for element in authored["elements"]
            if element["material"]["execution"].get("enabled") is True
            and element["material"]["execution"]["scalars"]
        )
        changed = copy.deepcopy(execution)
        changed["scalars"][0]["value"] += 0.125
        before = inventory._typed_material_axes(execution)
        after = inventory._typed_material_axes(changed)
        self.assertEqual(before[0], after[0])
        self.assertEqual(before[1], after[1])
        self.assertEqual(before[3], after[3])
        self.assertNotEqual(before[2], after[2])

    def test_typed_codec_rejects_register_and_mask_drift(self) -> None:
        authored = json.loads(
            (
                REPOSITORY_ROOT
                / "Data/Effects/Authored/effect.artist.skill.31470.unified.effect.json"
            ).read_text(encoding="utf-8")
        )
        execution = next(
            element["material"]["execution"]
            for element in authored["elements"]
            if element["material"]["execution"].get("enabled") is True
        )
        bad_register = copy.deepcopy(execution)
        bad_register["textureLanes"][0]["samplerRegister"] = 4
        with self.assertRaises(inventory.InventoryError):
            inventory._typed_material_axes(bad_register)
        bad_mask = copy.deepcopy(execution)
        bad_mask["dynamicSuppressedMask"] = bad_mask["dynamicConsumedMask"]
        if bad_mask["dynamicConsumedMask"] == 0:
            bad_mask["dynamicConsumedMask"] = 1
            bad_mask["dynamicSuppressedMask"] = 1
        with self.assertRaises(inventory.InventoryError):
            inventory._typed_material_axes(bad_mask)
        discarded_standard_color = copy.deepcopy(execution)
        discarded_standard_color["standardColor"] = [1.0, 1.0, 1.0, 1.0]
        with self.assertRaises(inventory.InventoryError):
            inventory._typed_material_axes(discarded_standard_color)

    def test_representative_program_cannot_be_promoted(self) -> None:
        occurrence = next(
            row
            for row in self.document["occurrences"]
            if row["program"]["status"] == "DXBC_FAMILY_REPRESENTATIVE_ONLY"
        )
        original = occurrence["program"]["programCandidateId"]
        candidate = self.document["programCandidates"][0]["programCandidateId"]
        self._assert_mutation_rejected(
            lambda: occurrence["program"].__setitem__("programCandidateId", candidate),
            lambda: occurrence["program"].__setitem__("programCandidateId", original),
        )

        def mutate_exact_family_carrier(document) -> None:
            exact_occurrence = next(
                row
                for row in document["occurrences"]
                if row["program"]["status"] == "DXBC_OCCURRENCE_EXACT"
                and next(
                    evidence
                    for evidence in document["programEvidence"]
                    if evidence["programEvidenceId"]
                    == row["program"]["programEvidenceId"]
                )["kind"]
                == "SOURCE_FAMILY_PROGRAM_EVIDENCE"
            )
            old_id = exact_occurrence["program"]["programEvidenceId"]
            evidence = next(
                row
                for row in document["programEvidence"]
                if row["programEvidenceId"] == old_id
            )
            evidence["sourceCarrier"] = (
                "mesh" if evidence["sourceCarrier"] != "mesh" else "sprite"
            )
            payload = dict(evidence)
            payload.pop("programEvidenceId")
            new_id = "program-evidence." + inventory.canonical_sha256(payload)
            evidence["programEvidenceId"] = new_id
            for row in document["occurrences"]:
                if row["program"]["programEvidenceId"] == old_id:
                    row["program"]["programEvidenceId"] = new_id

        self._assert_copy_mutation_rejected(mutate_exact_family_carrier)

    def test_exact_source_family_rejects_blocked_cooked_status_after_readdressing(self) -> None:
        def mutate(document) -> None:
            exact_occurrence = next(
                occurrence
                for occurrence in document["occurrences"]
                if occurrence["program"]["status"] == "DXBC_OCCURRENCE_EXACT"
                and next(
                    evidence
                    for evidence in document["programEvidence"]
                    if evidence["programEvidenceId"]
                    == occurrence["program"]["programEvidenceId"]
                )["kind"]
                == "SOURCE_FAMILY_PROGRAM_EVIDENCE"
            )
            old_id = exact_occurrence["program"]["programEvidenceId"]
            evidence = next(
                row
                for row in document["programEvidence"]
                if row["programEvidenceId"] == old_id
            )
            evidence["cookedStatus"] = "BLOCKED"
            evidence["cookedDxbcSha256"] = None
            payload = dict(evidence)
            payload.pop("programEvidenceId")
            new_id = "program-evidence." + inventory.canonical_sha256(payload)
            evidence["programEvidenceId"] = new_id
            for occurrence in document["occurrences"]:
                if occurrence["program"]["programEvidenceId"] == old_id:
                    occurrence["program"]["programEvidenceId"] = new_id

        self._assert_copy_mutation_rejected(mutate)

    def test_exact_variant_overlay_preserves_translation_and_admission_boundaries(self) -> None:
        exact_wire = [
            row
            for row in self.document["occurrences"]
            if row["layout"]["status"]
            == "EXACT_VARIANT_NATIVE_WIRE_ONLY_REQUIRES_PACKET_TRANSLATION"
        ]
        self.assertEqual(len(exact_wire), 20)
        self.assertEqual(
            sum(
                row["program"]["status"]
                == "DXBC_OCCURRENCE_EXACT_UNTRANSLATED"
                for row in exact_wire
            ),
            8,
        )
        for row in exact_wire:
            self.assertIsNone(row["layout"]["layoutCandidateId"])
            if row["program"]["status"] == "DXBC_OCCURRENCE_EXACT_UNTRANSLATED":
                self.assertIsNone(row["program"]["programCandidateId"])
                self.assertIn("LITERAL_TRANSLATION_MISSING", row["blockers"])
                self.assertIsNone(row["tupleCohortId"])
        mismatches = [
            row
            for row in self.document["occurrences"]
            if "EXACT_VARIANT_RENDERER_KIND_MISMATCH" in row["blockers"]
        ]
        self.assertEqual(len(mismatches), 22)
        self.assertEqual(
            sum(row["program"]["status"] == "DXBC_OCCURRENCE_EXACT" for row in mismatches),
            12,
        )
        self.assertEqual(
            sum(row["program"]["status"] == "DXBC_FAMILY_REPRESENTATIVE_ONLY" for row in mismatches),
            10,
        )
        exact_layout_rows = [
            row
            for row in self.document["layoutEvidence"]
            if row["kind"] == "EXACT_VARIANT_NATIVE_WIRE_EVIDENCE"
        ]
        self.assertTrue(exact_layout_rows)
        self.assertTrue(
            all(
                row["runtimePacketTopologyMaterialized"] is False
                and row["sourceExactNativeScalarGroupPacking"] is False
                and row["sourceExactSampler"] is False
                and row["sourceValueReplay"] is False
                and row["actualVfPass"] is False
                and row["runtimeAdmission"] is False
                and row["visualAdmission"] is False
                for row in exact_layout_rows
            )
        )

    def test_disabled_profile_keeps_program_layout_evidence_but_not_values(self) -> None:
        row = next(
            occurrence
            for occurrence in self.document["occurrences"]
            if occurrence["sourceProfileEnabled"] is False
            and occurrence["program"]["status"] == "DXBC_OCCURRENCE_EXACT"
            and occurrence["descriptor"]["status"] == "RESOURCE_ONLY_NO_MATERIAL_VALUES"
        )
        self.assertIsNotNone(row["program"]["programCandidateId"])
        self.assertIsNotNone(row["program"]["programEvidenceId"])
        self.assertIsNotNone(row["layout"]["layoutEvidenceId"])
        self.assertNotEqual(row["descriptor"]["status"], "SOURCE_VALUES_PRESENT_UNPACKED")

        self._assert_copy_mutation_rejected(
            lambda document: next(
                occurrence
                for occurrence in document["occurrences"]
                if occurrence["descriptor"]["status"]
                == "SOURCE_VALUES_PRESENT_UNPACKED"
            ).__setitem__("sourceProfileEnabled", False)
        )

    def test_exact_native_wire_rejects_nested_admission_after_readdressing(self) -> None:
        def mutate(document) -> None:
            evidence = next(
                row
                for row in document["layoutEvidence"]
                if row["kind"] == "EXACT_VARIANT_NATIVE_WIRE_EVIDENCE"
            )
            old_id = evidence["layoutEvidenceId"]
            evidence["nativeBindingWire"]["shaderObject"]["visualAdmission"] = True
            evidence["nativeBindingWireSha256"] = inventory.canonical_sha256(
                evidence["nativeBindingWire"]
            )
            payload = dict(evidence)
            payload.pop("layoutEvidenceId")
            new_id = "layout-evidence." + inventory.canonical_sha256(payload)
            evidence["layoutEvidenceId"] = new_id
            for occurrence in document["occurrences"]:
                if occurrence["layout"]["layoutEvidenceId"] == old_id:
                    occurrence["layout"]["layoutEvidenceId"] = new_id
            cohort_id_map = {}
            for cohort in document["cohorts"]:
                if cohort["layoutIdentityId"] != old_id:
                    continue
                old_cohort_id = cohort["tupleCohortId"]
                cohort["layoutIdentityId"] = new_id
                identity = {
                    "programCandidateId": cohort["programCandidateId"],
                    "layoutIdentityId": cohort["layoutIdentityId"],
                    "adapterCandidateId": cohort["adapterCandidateId"],
                }
                new_cohort_id = "cohort." + inventory.canonical_sha256(identity)
                cohort["tupleCohortId"] = new_cohort_id
                cohort_id_map[old_cohort_id] = new_cohort_id
            for occurrence in document["occurrences"]:
                if occurrence["tupleCohortId"] in cohort_id_map:
                    occurrence["tupleCohortId"] = cohort_id_map[
                        occurrence["tupleCohortId"]
                    ]
            artist_canary = document["canaries"]["artistFHorizontalGolden"]
            if artist_canary["tupleCohortId"] in cohort_id_map:
                artist_canary["tupleCohortId"] = cohort_id_map[
                    artist_canary["tupleCohortId"]
                ]

        self._assert_copy_mutation_rejected(mutate)

    def test_untranslated_exact_program_cannot_use_a_fabricated_translation(self) -> None:
        def mutate(document) -> None:
            occurrence_rows = [
                row
                for row in document["occurrences"]
                if row["program"]["status"] == "DXBC_OCCURRENCE_EXACT_UNTRANSLATED"
            ]
            old_evidence_id = occurrence_rows[0]["program"]["programEvidenceId"]
            evidence = next(
                row
                for row in document["programEvidence"]
                if row["programEvidenceId"] == old_evidence_id
            )
            evidence["literalTranslationAvailable"] = True
            evidence_payload = dict(evidence)
            evidence_payload.pop("programEvidenceId")
            new_evidence_id = "program-evidence." + inventory.canonical_sha256(
                evidence_payload
            )
            evidence["programEvidenceId"] = new_evidence_id
            fake_candidate_payload = {
                "kind": "DXBC_LITERAL_TRANSLATION",
                "dxbcSha256": evidence["dxbcSha256"],
                "hlslSha256": "0" * 64,
                "functionName": "FakeUntrackedTranslation",
            }
            fake_candidate_id = "program." + inventory.canonical_sha256(
                fake_candidate_payload
            )
            document["programCandidates"].append(
                {"programCandidateId": fake_candidate_id, **fake_candidate_payload}
            )
            for occurrence in occurrence_rows:
                occurrence["program"] = {
                    "status": "DXBC_OCCURRENCE_EXACT",
                    "programCandidateId": fake_candidate_id,
                    "programEvidenceId": new_evidence_id,
                }
            count = len(occurrence_rows)
            summary = document["summary"]["programStatusCounts"]
            summary["DXBC_OCCURRENCE_EXACT_UNTRANSLATED"] -= count
            summary["DXBC_OCCURRENCE_EXACT"] += count

        self._assert_copy_mutation_rejected(mutate)

    def test_exact_program_and_layout_variant_keys_must_match(self) -> None:
        def mutate(document) -> None:
            occurrence = next(
                row
                for row in document["occurrences"]
                if row["program"]["status"] == "DXBC_OCCURRENCE_EXACT_UNTRANSLATED"
            )
            old_id = occurrence["layout"]["layoutEvidenceId"]
            evidence = next(
                row
                for row in document["layoutEvidence"]
                if row["layoutEvidenceId"] == old_id
            )
            evidence["variantKeySha256"] = "0" * 64
            payload = dict(evidence)
            payload.pop("layoutEvidenceId")
            new_id = "layout-evidence." + inventory.canonical_sha256(payload)
            evidence["layoutEvidenceId"] = new_id
            for row in document["occurrences"]:
                if row["layout"]["layoutEvidenceId"] == old_id:
                    row["layout"]["layoutEvidenceId"] = new_id

        self._assert_copy_mutation_rejected(mutate)
    def test_child_parent_blocked_receipts_are_preserved_for_all_occurrences(self) -> None:
        rows = [
            row
            for row in self.document["occurrences"]
            if row["sourceParentResolution"] == "CHILD_PARENT_BLOCKED"
        ]
        self.assertEqual(len(rows), 558)
        self.assertEqual(sum(row["carrier"] == "PRESENTATION" for row in rows), 50)
        self.assertTrue(
            all(
                row["sourceParentRowSha256"]
                and row["sourceParentBlocker"]
                and "CHILD_PARENT_RESOLUTION_BLOCKED" in row["blockers"]
                for row in rows
            )
        )

        self.assertEqual(
            sum(
                row["program"]["status"]
                in ("DXBC_OCCURRENCE_EXACT", "DXBC_OCCURRENCE_EXACT_UNTRANSLATED")
                for row in rows
            ),
            0,
        )
        self.assertEqual(
            sum(
                row["program"]["programCandidateId"] is not None
                for row in rows
                if row["program"]["status"] != "TYPED_RUNTIME_PROGRAM_DECLARED"
            ),
            0,
        )

    def test_unknown_foreign_key_and_tuple_backref_are_rejected(self) -> None:
        occurrence = next(
            row for row in self.document["occurrences"] if row["compositionVariantIds"]
        )
        original_composition = list(occurrence["compositionVariantIds"])
        self._assert_mutation_rejected(
            lambda: occurrence.__setitem__(
                "compositionVariantIds", ["composition." + "0" * 64]
            ),
            lambda: occurrence.__setitem__(
                "compositionVariantIds", original_composition
            ),
        )
        cohort_occurrence = next(
            row for row in self.document["occurrences"] if row["tupleCohortId"]
        )
        original_tuple = cohort_occurrence["tupleCohortId"]
        self._assert_mutation_rejected(
            lambda: cohort_occurrence.__setitem__(
                "tupleCohortId", "cohort." + "0" * 64
            ),
            lambda: cohort_occurrence.__setitem__("tupleCohortId", original_tuple),
        )

    def test_runtime_and_product_overclaims_are_rejected(self) -> None:
        cohort = self.document["cohorts"][0]
        self._assert_mutation_rejected(
            lambda: cohort.__setitem__("runtimeVerified", True),
            lambda: cohort.__setitem__("runtimeVerified", False),
        )
        original_policy = self.document["policies"]["productAdmission"]
        self._assert_mutation_rejected(
            lambda: self.document["policies"].__setitem__(
                "productAdmission", "PROVEN"
            ),
            lambda: self.document["policies"].__setitem__(
                "productAdmission", original_policy
            ),
        )
        self._assert_copy_mutation_rejected(
            lambda document: document.__setitem__("runtimeVerified", True)
        )
        self._assert_copy_mutation_rejected(
            lambda document: document["occurrences"][0].__setitem__(
                "productAdmission", "ADMITTED"
            )
        )
        self._assert_copy_mutation_rejected(
            lambda document: document["canaries"]["artistFHorizontalGolden"].__setitem__(
                "visualPass", True
            )
        )

    def test_typed_descriptor_topology_mutation_is_rejected_after_readdressing(self) -> None:
        def mutate(document) -> None:
            descriptor = next(
                row
                for row in document["descriptorVariants"]
                if row["kind"] == "TYPED_RUNTIME_VALUES" and row["textureLanes"]
            )
            old_id = descriptor["descriptorVariantId"]
            descriptor["textureLanes"] = descriptor["textureLanes"][1:]
            payload = dict(descriptor)
            payload.pop("descriptorVariantId")
            new_id = "descriptor." + inventory.canonical_sha256(payload)
            descriptor["descriptorVariantId"] = new_id
            for occurrence in document["occurrences"]:
                if occurrence["descriptor"]["descriptorVariantId"] == old_id:
                    occurrence["descriptor"]["descriptorVariantId"] = new_id

        self._assert_copy_mutation_rejected(mutate)

    def test_typed_program_layout_adapter_mismatch_is_rejected_after_readdressing(self) -> None:
        def mutate(document) -> None:
            target_occurrence = next(
                row
                for row in document["occurrences"]
                if row["effectAssetId"] == "effect.artist.skill.31490.unified"
                and row["program"]["status"] == "TYPED_RUNTIME_PROGRAM_DECLARED"
            )
            old_program_id = target_occurrence["program"]["programCandidateId"]
            program = next(
                row
                for row in document["programCandidates"]
                if row["programCandidateId"] == old_program_id
            )
            program["backend"] = (
                "runtimeMaterialV2"
                if program["backend"] != "runtimeMaterialV2"
                else "artistVisualV4"
            )
            program_payload = dict(program)
            program_payload.pop("programCandidateId")
            new_program_id = "program." + inventory.canonical_sha256(program_payload)
            program["programCandidateId"] = new_program_id

            evidence_id_map = {}
            for evidence in document["programEvidence"]:
                if evidence.get("programCandidateId") != old_program_id:
                    continue
                old_evidence_id = evidence["programEvidenceId"]
                evidence["programCandidateId"] = new_program_id
                evidence_payload = dict(evidence)
                evidence_payload.pop("programEvidenceId")
                new_evidence_id = "program-evidence." + inventory.canonical_sha256(
                    evidence_payload
                )
                evidence["programEvidenceId"] = new_evidence_id
                evidence_id_map[old_evidence_id] = new_evidence_id

            for occurrence in document["occurrences"]:
                if occurrence["program"]["programCandidateId"] == old_program_id:
                    occurrence["program"]["programCandidateId"] = new_program_id
                    occurrence["program"]["programEvidenceId"] = evidence_id_map[
                        occurrence["program"]["programEvidenceId"]
                    ]

            cohort_id_map = {}
            for cohort in document["cohorts"]:
                if cohort["programCandidateId"] != old_program_id:
                    continue
                old_cohort_id = cohort["tupleCohortId"]
                cohort["programCandidateId"] = new_program_id
                identity = {
                    "programCandidateId": cohort["programCandidateId"],
                    "layoutIdentityId": cohort["layoutIdentityId"],
                    "adapterCandidateId": cohort["adapterCandidateId"],
                }
                new_cohort_id = "cohort." + inventory.canonical_sha256(identity)
                cohort["tupleCohortId"] = new_cohort_id
                cohort_id_map[old_cohort_id] = new_cohort_id
            for occurrence in document["occurrences"]:
                if occurrence["tupleCohortId"] in cohort_id_map:
                    occurrence["tupleCohortId"] = cohort_id_map[
                        occurrence["tupleCohortId"]
                    ]

        self._assert_copy_mutation_rejected(mutate)

    def test_typed_layout_cannot_be_grafted_onto_a_source_tuple(self) -> None:
        def mutate(document) -> None:
            reference_counts = {}
            for occurrence in document["occurrences"]:
                evidence_id = occurrence["layout"]["layoutEvidenceId"]
                if evidence_id is not None:
                    reference_counts[evidence_id] = reference_counts.get(evidence_id, 0) + 1
            occurrence = next(
                row
                for row in document["occurrences"]
                if row["program"]["status"] == "DXBC_FAMILY_REPRESENTATIVE_ONLY"
                and row["layout"]["status"] == "NAMED_NATIVE_WIRE_ONLY_WITHIN_COUNT_CAPS"
                and reference_counts[row["layout"]["layoutEvidenceId"]] > 1
            )
            old_status = occurrence["layout"]["status"]
            occurrence["layout"] = {
                "status": "TYPED_PACKET_CLOSED",
                "layoutCandidateId": document["layoutCandidates"][0]["layoutCandidateId"],
                "layoutEvidenceId": None,
            }
            document["summary"]["layoutStatusCounts"][old_status] -= 1
            document["summary"]["layoutStatusCounts"]["TYPED_PACKET_CLOSED"] += 1

        self._assert_copy_mutation_rejected(mutate)

    def test_composition_hidden_admission_and_missing_product_refs_are_rejected(self) -> None:
        def add_hidden_admission(document) -> None:
            variant = document["compositionVariants"][0]
            old_id = variant["compositionVariantId"]
            variant["semantic"]["visualAdmission"] = True
            new_id = "composition." + inventory.canonical_sha256(variant["semantic"])
            variant["compositionVariantId"] = new_id
            for occurrence in document["occurrences"]:
                occurrence["compositionVariantIds"] = [
                    new_id if value == old_id else value
                    for value in occurrence["compositionVariantIds"]
                ]

        self._assert_copy_mutation_rejected(add_hidden_admission)

        def retarget_semantic_asset(document) -> None:
            variant = document["compositionVariants"][0]
            old_id = variant["compositionVariantId"]
            old_asset = variant["semantic"]["effectAssetId"]
            variant["semantic"]["effectAssetId"] = (
                "effect.artist.skill.31470.unified"
                if old_asset != "effect.artist.skill.31470.unified"
                else "effect.warlord.skill.17040.unified"
            )
            new_id = "composition." + inventory.canonical_sha256(variant["semantic"])
            variant["compositionVariantId"] = new_id
            for occurrence in document["occurrences"]:
                occurrence["compositionVariantIds"] = [
                    new_id if value == old_id else value
                    for value in occurrence["compositionVariantIds"]
                ]

        self._assert_copy_mutation_rejected(retarget_semantic_asset)

        def make_character_timing_negative(document) -> None:
            variant = next(
                row
                for row in document["compositionVariants"]
                if row["semantic"]["kind"] == "CHARACTER_ANIMATION_CUE"
            )
            old_id = variant["compositionVariantId"]
            variant["semantic"]["startMs"] = -1
            new_id = "composition." + inventory.canonical_sha256(variant["semantic"])
            variant["compositionVariantId"] = new_id
            for occurrence in document["occurrences"]:
                occurrence["compositionVariantIds"] = [
                    new_id if value == old_id else value
                    for value in occurrence["compositionVariantIds"]
                ]

        self._assert_copy_mutation_rejected(make_character_timing_negative)
        self._assert_copy_mutation_rejected(
            lambda document: next(
                row
                for row in document["occurrences"]
                if row["scopeBits"]["productConsumed"]
            ).__setitem__("compositionVariantIds", [])
        )

    def test_composition_projection_change_is_nonfatal_snapshot_drift(self) -> None:
        document = copy.deepcopy(self.document)
        variant = next(
            row
            for row in document["compositionVariants"]
            if row["semantic"]["kind"] == "CHARACTER_ANIMATION_CUE"
        )
        old_id = variant["compositionVariantId"]
        variant["semantic"]["startMs"] += 1
        new_id = "composition." + inventory.canonical_sha256(variant["semantic"])
        variant["compositionVariantId"] = new_id
        for occurrence in document["occurrences"]:
            occurrence["compositionVariantIds"] = [
                new_id if value == old_id else value
                for value in occurrence["compositionVariantIds"]
            ]
        projection_sha = inventory.canonical_sha256(document["compositionVariants"])
        document["summary"]["compositionProjectionSha256"] = projection_sha
        product_canary = document["canaries"]["productConsumerRuntimeSnapshot"]
        product_canary["compositionProjectionSha256"] = projection_sha
        product_canary["status"] = "DRIFTED_FROM_CURRENT_REGRESSION_SNAPSHOT"
        document.pop("artifactSha256", None)
        document["artifactSha256"] = inventory.canonical_sha256(document)
        inventory.validate_inventory(document)

    def test_named_layout_evidence_cannot_cross_program_families(self) -> None:
        def mutate(document) -> None:
            usage = {}
            for occurrence in document["occurrences"]:
                evidence_id = occurrence["layout"]["layoutEvidenceId"]
                if evidence_id:
                    usage[evidence_id] = usage.get(evidence_id, 0) + 1
            evidence_by_id = {
                row["layoutEvidenceId"]: row for row in document["layoutEvidence"]
            }
            target = next(
                occurrence
                for occurrence in document["occurrences"]
                if occurrence["tupleCohortId"] is None
                and occurrence["layout"]["status"]
                in (
                    "NAMED_NATIVE_WIRE_ONLY_WITHIN_COUNT_CAPS",
                    "NAMED_NATIVE_WIRE_ONLY_REQUIRES_COUNT_EXTENSION",
                )
                and usage[occurrence["layout"]["layoutEvidenceId"]] > 1
            )
            current = evidence_by_id[target["layout"]["layoutEvidenceId"]]
            replacement = next(
                row
                for row in document["layoutEvidence"]
                if row["kind"] == "NAMED_NATIVE_WIRE_EVIDENCE"
                and row["withinCountCaps"] == current["withinCountCaps"]
                and row["parentMaterialPath"] != current["parentMaterialPath"]
            )
            target["layout"]["layoutEvidenceId"] = replacement["layoutEvidenceId"]

        self._assert_copy_mutation_rejected(mutate)

    def test_published_element_order_is_part_of_occurrence_exactness(self) -> None:
        current_order = ["a", "b"]
        current_hashes = {"a": "1", "b": "2"}
        self.assertEqual(
            inventory._published_element_exact_projection(
                current_order, current_hashes, ["a", "b"], current_hashes
            ),
            {"a": True, "b": True},
        )
        self.assertEqual(
            inventory._published_element_exact_projection(
                current_order, current_hashes, ["b", "a"], current_hashes
            ),
            {"a": False, "b": False},
        )
        self.assertEqual(
            inventory._published_element_exact_projection(
                current_order, current_hashes, ["a"], {"a": "1"}
            ),
            {"a": True, "b": False},
        )

    def test_occurrence_level_publish_status_is_not_inherited_from_a_sibling(self) -> None:
        def mutate(document) -> None:
            occurrence = next(
                row
                for row in document["occurrences"]
                if row["publishedElementExact"] is True
                and row["productStatus"] == "PRODUCT_JOIN_CLOSED"
            )
            occurrence["productStatus"] = "PUBLISHED_ELEMENT_STALE"
            occurrence["blockers"] = sorted(
                set(occurrence["blockers"]) | {"PUBLISHED_ELEMENT_STALE"}
            )

        self._assert_copy_mutation_rejected(mutate)

    def test_runtime_scope_cannot_be_synthesized_without_published_ownership(self) -> None:
        def mutate(document) -> None:
            seed = next(
                row
                for row in document["occurrences"]
                if row["compositionStatus"] == "CATALOG_DECLARED_ONLY"
            )
            asset = seed["effectAssetId"]
            rows = [row for row in document["occurrences"] if row["effectAssetId"] == asset]
            borrowed_path = next(
                row["publishedDocumentPath"]
                for row in document["occurrences"]
                if row["publishedDocumentPath"] is not None
            )
            for row in rows:
                row["scopeBits"]["runtimePublished"] = True
                row["compositionStatus"] = "RUNTIME_PUBLISHED_WITHOUT_CONSUMER"
                row["productStatus"] = "RUNTIME_PUBLISHED_UNCONSUMED"
                row["publishedDocumentPath"] = borrowed_path
                row["publishedElementExact"] = True
            count = len(rows)
            summary = document["summary"]
            summary["compositionStatusCounts"]["CATALOG_DECLARED_ONLY"] -= count
            summary["compositionStatusCounts"]["RUNTIME_PUBLISHED_WITHOUT_CONSUMER"] = (
                summary["compositionStatusCounts"].get(
                    "RUNTIME_PUBLISHED_WITHOUT_CONSUMER", 0
                )
                + count
            )
            summary["productStatusCounts"]["CATALOG_NOT_PUBLISHED"] -= count
            summary["productStatusCounts"]["RUNTIME_PUBLISHED_UNCONSUMED"] = (
                summary["productStatusCounts"].get(
                    "RUNTIME_PUBLISHED_UNCONSUMED", 0
                )
                + count
            )
            summary["runtimePublishedAssetCount"] += 1
            summary["scopedRuntimeCatalogAssetCount"] += 1
            summary["runtimePublishedOccurrenceCount"] += count
            summary["runtimeAssetSetEqualsProductConsumerSet"] = False
            canary = document["canaries"]["productConsumerRuntimeSnapshot"]
            canary["runtimePublishedAssetCount"] += 1
            canary["runtimePublishedOccurrenceCount"] += count
            canary["runtimeAssetSetEqualsProductConsumerSet"] = False
            canary["status"] = "DRIFTED_FROM_CURRENT_REGRESSION_SNAPSHOT"

        self._assert_copy_mutation_rejected(mutate)

    def test_published_input_role_cannot_be_grafted_onto_authored_input(self) -> None:
        self._assert_copy_mutation_rejected(
            lambda document: next(
                row
                for row in document["inputs"]
                if row["roles"] == ["TARGET_AUTHORED_DOCUMENT"]
            )["roles"].append("PUBLISHED_EFFECT_DOCUMENT")
        )

    def test_scope_review_and_blocker_bucket_drift_are_rejected(self) -> None:
        occurrence = next(
            row
            for row in self.document["occurrences"]
            if row["compositionStatus"] == "AUTHORED_ONLY"
        )
        self._assert_mutation_rejected(
            lambda: occurrence["scopeBits"].__setitem__("productConsumed", True),
            lambda: occurrence["scopeBits"].__setitem__("productConsumed", False),
        )
        original_review = occurrence["legacyGoldenReview"]["status"]
        self._assert_mutation_rejected(
            lambda: occurrence["legacyGoldenReview"].__setitem__(
                "status", "SUPER_APPROVED"
            ),
            lambda: occurrence["legacyGoldenReview"].__setitem__(
                "status", original_review
            ),
        )
        bucket = self.document["blockerBuckets"][0]
        original_count = bucket["occurrenceCount"]
        self._assert_mutation_rejected(
            lambda: bucket.__setitem__("occurrenceCount", original_count + 1),
            lambda: bucket.__setitem__("occurrenceCount", original_count),
        )

    def test_strict_json_rejects_duplicate_keys_and_nonfinite_numbers(self) -> None:
        with self.assertRaises(inventory.InventoryError):
            inventory._decode_json(b'{"x":1,"x":2}', "duplicate fixture")
        with self.assertRaises(inventory.InventoryError):
            inventory._decode_json(b'{"x":NaN}', "nonfinite fixture")

    def test_input_snapshot_detects_change_before_commit(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            source = root / "input.json"
            source.write_bytes(b'{"value":1}\n')
            parsed = {"value": 1}
            document = {
                "inputs": [
                    {
                        "path": "input.json",
                        "roles": ["FIXTURE"],
                        "rawSha256": inventory._sha256_bytes(source.read_bytes()),
                        "byteSize": len(source.read_bytes()),
                        "canonicalJsonSha256": inventory.canonical_sha256(parsed),
                    }
                ]
            }
            inventory.validate_input_snapshot(document, root)
            source.write_bytes(b'{"value":2}\n')
            with self.assertRaises(inventory.InventoryError):
                inventory.validate_input_snapshot(document, root)

    def test_input_snapshot_detects_discovered_set_membership_change(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            source = root / "input.json"
            source.write_bytes(b'{"value":1}\n')
            parsed = {"value": 1}
            document = {
                "inputs": [
                    {
                        "path": "input.json",
                        "roles": ["FIXTURE"],
                        "rawSha256": inventory._sha256_bytes(source.read_bytes()),
                        "byteSize": len(source.read_bytes()),
                        "canonicalJsonSha256": inventory.canonical_sha256(parsed),
                    }
                ]
            }
            inventory.validate_input_snapshot(document, root)
            authored = root / inventory.AUTHORED_DIRECTORY
            authored.mkdir(parents=True)
            (authored / "effect.artist.fixture.effect.json").write_text(
                '{"effectAssetId":"effect.artist.fixture"}\n', encoding="utf-8"
            )
            with self.assertRaises(inventory.InventoryError):
                inventory.validate_input_snapshot(document, root)

    def test_atomic_replace_failure_preserves_existing_output(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            target = Path(temporary_directory) / "inventory.json"
            sentinel = b"existing-output-must-survive\n"
            target.write_bytes(sentinel)
            with mock.patch.object(
                inventory.os, "replace", side_effect=OSError("injected replace failure")
            ):
                with self.assertRaises(OSError):
                    inventory.write_inventory(
                        self.document,
                        target,
                        REPOSITORY_ROOT,
                    )
            self.assertEqual(target.read_bytes(), sentinel)
            self.assertEqual(
                list(target.parent.glob(target.name + ".*.tmp")),
                [],
            )


if __name__ == "__main__":
    unittest.main()
