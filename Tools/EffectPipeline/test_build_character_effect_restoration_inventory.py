#!/usr/bin/env python3
"""Focused tests for the family-first character Effect restoration inventory."""

from __future__ import annotations

import copy
import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock


SCRIPT = Path(__file__).with_name(
    "build_character_effect_restoration_inventory.py"
)
SPEC = importlib.util.spec_from_file_location(
    "character_effect_restoration_inventory", SCRIPT
)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class CharacterEffectRestorationInventoryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.inventory = MODULE.build_inventory()

    def mutation(self) -> dict:
        return copy.deepcopy(self.inventory)

    def test_current_canary_denominators_and_admission_partition(self) -> None:
        summary = self.inventory["summary"]
        self.assertEqual(summary["targetCount"], 3)
        self.assertEqual(summary["occurrenceCount"], 22)
        self.assertEqual(
            summary["runtimeAdmissionCounts"],
            {"ADMITTED": 22},
        )
        targets = {row["targetId"]: row for row in self.inventory["targets"]}
        artist = targets["target.artist.f.31470.golden"]
        dimension = targets["target.dimensionmaster.a.2050210.makeflow"]
        warlord = targets["target.warlord.w.17060.hemisphere"]
        self.assertEqual(artist["selectedOccurrenceCount"], 17)
        self.assertEqual(artist["currentAuthoredElementCount"], 17)
        self.assertEqual(artist["userReview"], "APPROVED")
        self.assertEqual(dimension["selectedOccurrenceCount"], 4)
        self.assertEqual(dimension["currentAuthoredElementCount"], 12)
        self.assertEqual(warlord["selectedOccurrenceCount"], 1)
        self.assertEqual(warlord["currentAuthoredElementCount"], 2)

        occurrences = {
            row["authoredNode"]["stableId"]: row
            for row in self.inventory["occurrences"]
        }
        self.assertEqual(
            occurrences[
                "authored.source-particle.a58f7a015a0bab4c53a664fd"
            ]["runtimeAdmission"],
            "ADMITTED",
        )
        self.assertEqual(
            occurrences[
                "authored.source-particle.635e153de978318aa446452e"
            ]["runtimeAdmission"],
            "ADMITTED",
        )
        for stable_id in (
            "authored.source-particle.53c11a9082f088279597515c",
            "authored.source-particle.83f005256fb59d87b99de92f",
            "authored.source-particle.ca6dc295e0267400d6968003",
        ):
            self.assertEqual(
                occurrences[stable_id]["runtimeAdmission"], "ADMITTED"
            )
            self.assertEqual(
                occurrences[stable_id]["productJoin"]["status"],
                "CLOSED",
            )

        dimension_rows = [
            row
            for row in self.inventory["occurrences"]
            if row["authoredNode"]["effectAssetId"]
            == "effect.dimensionmaster.skill.2050210.unified"
        ]
        dimension_starts = [
            next(
                composition["timing"]["startDelaySeconds"]
                for composition in self.inventory["compositionVariants"]
                if composition["compositionVariantId"]
                == row["familyTuple"]["compositionVariantId"]
            )
            for row in dimension_rows
        ]
        self.assertEqual(dimension_starts, [0.25, 0.6, 0.9, 1.3])

    def test_artist_f_golden_kind_backend_and_source_recipe_identity(self) -> None:
        artist = MODULE.read_json(
            MODULE.REPOSITORY_ROOT
            / "Data/Effects/Authored/effect.artist.skill.31470.unified.effect.json"
        )
        self.assertEqual(
            MODULE.canonical_sha256(artist),
            MODULE.ARTIST_F_GOLDEN_DOCUMENT_SHA256,
        )
        elements = artist["elements"]
        kind_counts: dict[str, int] = {}
        backend_counts: dict[str, int] = {}
        for element in elements:
            kind = element["kind"]
            kind_counts[kind] = kind_counts.get(kind, 0) + 1
            backend = element["material"]["execution"]["backend"]
            backend_counts[backend] = backend_counts.get(backend, 0) + 1
        self.assertEqual(kind_counts, {"trail": 1, "particle": 14, "decal": 2})
        self.assertEqual(
            backend_counts,
            {"runtimeMaterialV2": 8, "artistVisualV4": 7, "localDecal": 2},
        )
        self.assertEqual(
            sum(bool(row["sourceRecipe"]["enabled"]) for row in elements), 14
        )
        self.assertEqual(
            sum(len(row["sourceRecipe"]["modules"]) for row in elements), 165
        )

    def test_all_variant_ids_and_execution_closures_are_canonical_sha256(self) -> None:
        for rows, id_field, prefix in (
            (self.inventory["carrierVariants"], "carrierVariantId", "carrier"),
            (self.inventory["materialVariants"], "materialVariantId", "material"),
            (self.inventory["renderVariants"], "renderVariantId", "render"),
            (
                self.inventory["compositionVariants"],
                "compositionVariantId",
                "composition",
            ),
        ):
            for row in rows:
                payload = copy.deepcopy(row)
                variant_id = payload.pop(id_field)
                self.assertEqual(
                    variant_id,
                    f"{prefix}.{MODULE.canonical_sha256(payload)}",
                )
        for occurrence in self.inventory["occurrences"]:
            self.assertEqual(
                occurrence["executionClosureId"],
                MODULE.canonical_sha256(occurrence["familyTuple"]),
            )

    def test_rgba_lanes_seal_role_channel_color_space_sampler_and_policies(self) -> None:
        for material in self.inventory["materialVariants"]:
            self.assertIn(
                material["missingLanePolicy"]["mode"],
                {"FAIL_CLOSED", "EXPLICIT_CONSTANT"},
            )
            self.assertIn(
                material["coveragePolicy"]["kind"],
                {"TEXTURE_CHANNEL", "RECOVERED_EQUATION"},
            )
            self.assertIn(
                material["emissiveSourcePolicy"]["kind"],
                {"TEXTURE_LANE", "BASE_LUMINANCE", "CONSTANT", "NONE"},
            )
            for lane in material["textureLanes"]:
                self.assertTrue(lane["semanticRole"])
                self.assertIn(lane["sourceChannel"], MODULE.CHANNEL_VALUES)
                self.assertIn(lane["colorSpace"], MODULE.COLOR_SPACE_VALUES)
                self.assertEqual(
                    set(lane["sampler"]),
                    {
                        "filter",
                        "addressU",
                        "addressV",
                        "addressW",
                        "mipLodBias",
                        "maxAnisotropy",
                        "comparison",
                        "borderColor",
                        "minLod",
                        "maxLod",
                        "evidence",
                    },
                )

    def test_provenance_evidence_executor_admission_review_are_orthogonal(self) -> None:
        for occurrence in self.inventory["occurrences"]:
            self.assertIn(occurrence["provenance"], MODULE.PROVENANCE_VALUES)
            self.assertIn(occurrence["evidence"], MODULE.EVIDENCE_VALUES)
            self.assertIn(occurrence["runtimeExecutor"], MODULE.EXECUTOR_VALUES)
            self.assertIn(occurrence["runtimeAdmission"], MODULE.ADMISSION_VALUES)
            self.assertIn(occurrence["userReview"], MODULE.USER_REVIEW_VALUES)
            self.assertEqual(occurrence["failureScope"], "THIS_OCCURRENCE")
        dimension = next(
            row
            for row in self.inventory["occurrences"]
            if row["authoredNode"]["stableId"].endswith("a58f7a015a0bab4c53a664fd")
        )
        self.assertEqual(dimension["provenance"], "PROJECT_TUNED")
        self.assertEqual(dimension["evidence"], "PARTIAL")
        self.assertEqual(
            dimension["runtimeExecutor"], "TYPED_SOURCE_RECONSTRUCTION"
        )
        self.assertEqual(dimension["userReview"], "PENDING")
        dimension_source_exact = next(
            row
            for row in self.inventory["occurrences"]
            if row["authoredNode"]["stableId"].endswith(
                "53c11a9082f088279597515c"
            )
        )
        self.assertEqual(dimension_source_exact["provenance"], "SOURCE_EXACT")
        self.assertEqual(dimension_source_exact["evidence"], "PARTIAL")

    def test_dimension_a_retime_is_explicit_and_source_occurrence_is_evidence_only(self) -> None:
        self.assertEqual(
            self.inventory["summary"]["substitutionDuplicationReceiptCount"], 1
        )
        receipt = self.inventory["substitutionDuplicationReceipts"][0]
        self.assertEqual(receipt["targetId"], "target.dimensionmaster.a.2050210.makeflow")
        self.assertEqual(
            receipt["sourceStableId"],
            "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1."
            "particlespriteemitter_2.event_source-event-030",
        )
        self.assertAlmostEqual(receipt["sourceStartSeconds"], 0.6)
        self.assertEqual(receipt["targetCardinality"], 4)
        self.assertEqual(len(receipt["occurrenceDecisions"]), 2)
        decisions = {
            row["currentStableId"]: row
            for row in receipt["occurrenceDecisions"]
        }
        decision = decisions[
            "authored.source-particle.a58f7a015a0bab4c53a664fd"
        ]
        self.assertAlmostEqual(decision["authoredStartSeconds"], 0.25)
        self.assertEqual(decision["timingProvenance"], "PROJECT_TUNED")
        self.assertEqual(
            decision["sourceOccurrenceDisposition"],
            "EVIDENCE_ONLY_SUBSTITUTED_NOT_ADMITTED",
        )
        source_exact = decisions[
            "authored.source-particle.53c11a9082f088279597515c"
        ]
        self.assertAlmostEqual(source_exact["authoredStartSeconds"], 0.6)
        self.assertEqual(source_exact["timingProvenance"], "SOURCE_EXACT")
        self.assertEqual(
            source_exact["sourceOccurrenceDisposition"],
            "ADMITTED_SOURCE_OCCURRENCE",
        )

    def test_silent_duplicate_source_lineage_is_rejected(self) -> None:
        changed = self.mutation()
        source_occurrence = next(
            row
            for row in changed["occurrences"]
            if row["substitutionDuplicationReceiptId"] is not None
        )
        duplicate = copy.deepcopy(source_occurrence)
        duplicate["occurrenceId"] = "occurrence." + "f" * 64
        duplicate["authoredNode"]["stableId"] += ".silent-duplicate"
        changed["occurrences"].append(duplicate)
        with self.assertRaisesRegex(
            MODULE.InventoryError,
            "duplicate sourceNode exceeds exact receipt authorization",
        ):
            MODULE.validate_inventory(changed)

    def test_retimed_dimension_occurrence_cannot_claim_source_exact(self) -> None:
        changed = self.mutation()
        retimed = next(
            row
            for row in changed["occurrences"]
            if row["authoredNode"]["stableId"].endswith(
                "a58f7a015a0bab4c53a664fd"
            )
        )
        retimed["provenance"] = "SOURCE_EXACT"
        current_receipt = next(
            row
            for row in retimed["fieldReceipts"]
            if row["fieldGroup"] == "CURRENT_DIRECT_AUTHORED_CLOSURE"
        )
        current_receipt["provenance"] = "SOURCE_EXACT"
        with self.assertRaisesRegex(
            MODULE.InventoryError,
            "receipt does not exactly key current/source IDs",
        ):
            MODULE.validate_inventory(changed)

    def test_unpublished_occurrence_cannot_claim_product_admission(self) -> None:
        changed = self.mutation()
        unpublished = next(
            row
            for row in changed["occurrences"]
            if row["productJoin"]["status"] == "CLOSED"
        )
        unpublished["productJoin"]["status"] = "AUTHORED_NOT_PUBLISHED"
        unpublished["productJoin"]["publishedElementSha256"] = None
        unpublished["blockers"].append(
            "PUBLISHED_DIRECT_DOCUMENT_STALE_FOR_OCCURRENCE"
        )
        unpublished["runtimeAdmission"] = "ADMITTED"
        with self.assertRaisesRegex(
            MODULE.InventoryError,
            "unpublished authored occurrence must remain AUTHORING_ONLY",
        ):
            MODULE.validate_inventory(changed)

    def test_closed_product_join_requires_current_element_identity(self) -> None:
        changed = self.mutation()
        closed = next(
            row
            for row in changed["occurrences"]
            if row["productJoin"]["status"] == "CLOSED"
        )
        closed["productJoin"]["publishedElementSha256"] = "f" * 64
        with self.assertRaisesRegex(
            MODULE.InventoryError,
            "does not publish the current authored closure",
        ):
            MODULE.validate_inventory(changed)

    def test_non_presentation_carrier_rejects_null_material(self) -> None:
        changed = self.mutation()
        changed["occurrences"][0]["familyTuple"]["materialVariantId"] = None
        with self.assertRaisesRegex(
            MODULE.InventoryError, "materialVariantId=null is illegal"
        ):
            MODULE.validate_inventory(changed)

    def test_typedata_ribbon_cannot_silently_project_to_sprite(self) -> None:
        changed = self.mutation()
        sprite_occurrence = next(
            row
            for row in changed["occurrences"]
            if row["authoredNode"]["stableId"].startswith("sprite.")
        )
        carrier_id = sprite_occurrence["familyTuple"]["carrierVariantId"]
        carrier = next(
            row
            for row in changed["carrierVariants"]
            if row["carrierVariantId"] == carrier_id
        )
        carrier["sourceTypeDataClasses"].append("particlemoduletypedataribbon")
        with self.assertRaisesRegex(
            MODULE.InventoryError, "TypeDataRibbon cannot be projected"
        ):
            MODULE.validate_inventory(changed)

    def test_execution_closure_tuple_mismatch_is_rejected(self) -> None:
        changed = self.mutation()
        changed["occurrences"][0]["executionClosureId"] = "0" * 64
        with self.assertRaisesRegex(MODULE.InventoryError, "tuple mismatch"):
            MODULE.validate_inventory(changed)

    def test_unknown_variant_id_is_rejected(self) -> None:
        changed = self.mutation()
        changed["occurrences"][0]["familyTuple"]["carrierVariantId"] = (
            "carrier." + "0" * 64
        )
        with self.assertRaisesRegex(MODULE.InventoryError, "unknown carrierVariantId"):
            MODULE.validate_inventory(changed)

    def test_duplicate_registry_id_is_rejected(self) -> None:
        changed = self.mutation()
        changed["carrierVariants"].append(copy.deepcopy(changed["carrierVariants"][0]))
        with self.assertRaisesRegex(MODULE.InventoryError, "duplicate carrierVariantId"):
            MODULE.validate_inventory(changed)

    def test_dxt1_no_alpha_cannot_be_implicitly_admitted_as_base_alpha(self) -> None:
        changed = self.mutation()
        occurrence = next(
            row for row in changed["occurrences"] if row["runtimeAdmission"] == "ADMITTED"
        )
        material_id = occurrence["familyTuple"]["materialVariantId"]
        material = next(
            row
            for row in changed["materialVariants"]
            if row["materialVariantId"] == material_id
        )
        lane = material["textureLanes"][0]
        lane["storage"] = {"format": "DXT1", "alphaMode": "NONE"}
        material["coveragePolicy"] = {
            "kind": "TEXTURE_CHANNEL",
            "laneId": lane["laneId"],
            "channel": "A",
            "equationId": None,
            "opaqueAlphaBehavior": "REJECT_IF_NO_ALPHA",
        }
        with self.assertRaisesRegex(MODULE.InventoryError, "no-alpha texture"):
            MODULE.validate_inventory(changed)

    def test_positive_emissive_intensity_rejects_null_emissive_lane(self) -> None:
        changed = self.mutation()
        occurrence = next(
            row
            for row in changed["occurrences"]
            if row["runtimeAdmission"] == "ADMITTED"
            and row["materialInputs"]["emissiveIntensity"] > 0
        )
        material_id = occurrence["familyTuple"]["materialVariantId"]
        material = next(
            row
            for row in changed["materialVariants"]
            if row["materialVariantId"] == material_id
        )
        emissive_lane_id = material["emissiveSourcePolicy"]["laneId"]
        emissive_lane = next(
            row for row in material["textureLanes"] if row["laneId"] == emissive_lane_id
        )
        emissive_lane["assetId"] = None
        with self.assertRaisesRegex(MODULE.InventoryError, "emissive source lane is null"):
            MODULE.validate_inventory(changed)

    def test_implicit_default_srv_missing_lane_policy_is_rejected(self) -> None:
        changed = self.mutation()
        changed["materialVariants"][0]["missingLanePolicy"]["mode"] = (
            "IMPLICIT_DEFAULT_SRV"
        )
        with self.assertRaisesRegex(MODULE.InventoryError, "implicit default SRV"):
            MODULE.validate_inventory(changed)

    def test_material_policy_change_invalidates_content_addressed_variant(self) -> None:
        changed = self.mutation()
        changed["materialVariants"][0]["coveragePolicy"]["equationId"] += ".changed"
        with self.assertRaisesRegex(MODULE.InventoryError, "content hash mismatch"):
            MODULE.validate_inventory(changed)

    def test_atomic_replace_failure_preserves_previous_output_and_cleans_stage(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "inventory.json"
            previous = b"previous-valid-output\n"
            output.write_bytes(previous)
            with mock.patch.object(MODULE.os, "replace", side_effect=OSError("blocked")):
                with self.assertRaises(OSError):
                    MODULE.write_inventory_transactionally(self.inventory, output)
            self.assertEqual(output.read_bytes(), previous)
            self.assertEqual(list(Path(directory).glob("*.staging")), [])

    def test_validation_failure_rolls_back_before_staging(self) -> None:
        changed = self.mutation()
        changed["carrierVariants"].append(copy.deepcopy(changed["carrierVariants"][0]))
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "inventory.json"
            previous = b"previous-valid-output\n"
            output.write_bytes(previous)
            with self.assertRaises(MODULE.InventoryError):
                MODULE.write_inventory_transactionally(changed, output)
            self.assertEqual(output.read_bytes(), previous)
            self.assertEqual(list(Path(directory).glob("*.staging")), [])

    def test_check_mode_is_read_only_and_rejects_stale_valid_inventory(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "inventory.json"
            MODULE.write_inventory_transactionally(self.inventory, output)
            before = output.read_bytes()
            MODULE.check_inventory(self.inventory, output)
            self.assertEqual(output.read_bytes(), before)

            stale = self.mutation()
            stale["occurrences"][-1]["blockers"].append("SYNTHETIC_STALE_ROW")
            stale["artifactSha256"] = MODULE._artifact_sha256(stale)
            MODULE.validate_inventory(stale)
            output.write_bytes(MODULE.pretty_json_bytes(stale))
            stale_bytes = output.read_bytes()
            with self.assertRaisesRegex(MODULE.InventoryError, "output is stale"):
                MODULE.check_inventory(self.inventory, output)
            self.assertEqual(output.read_bytes(), stale_bytes)

    def test_strict_json_loader_rejects_duplicate_keys(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "duplicate.json"
            path.write_text('{"schema":"a","schema":"b"}', encoding="utf-8")
            with self.assertRaisesRegex(MODULE.InventoryError, "duplicate JSON key"):
                MODULE.read_json(path)


if __name__ == "__main__":
    unittest.main()
