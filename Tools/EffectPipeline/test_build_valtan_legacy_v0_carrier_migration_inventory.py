from __future__ import annotations

import copy
import json
import importlib.util
import tempfile
import unittest
from collections import Counter
from pathlib import Path
from unittest import mock


SCRIPT = Path(__file__).with_name(
    "build_valtan_legacy_v0_carrier_migration_inventory.py"
)
SPEC = importlib.util.spec_from_file_location(
    "valtan_legacy_v0_carrier_migration_inventory", SCRIPT
)
assert SPEC and SPEC.loader
builder = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(builder)


class ValtanLegacyV0CarrierMigrationInventoryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.checked_in = builder.read_json(builder.OUTPUT_PATH)
        cls.historical = builder.MATERIALIZATION_RECEIPT_PATH.is_file()
        if cls.historical:
            cls.receipt = builder.read_json(
                builder.MATERIALIZATION_RECEIPT_PATH
            )
            builder.validate_sealed_historical_preimage(
                cls.checked_in, cls.receipt
            )
            cls.document = cls.checked_in
        else:
            cls.receipt = None
            cls.document = builder.build_inventory()

    def test_checked_in_output_is_deterministic_and_current(self) -> None:
        self.assertEqual(self.checked_in, self.document)
        if self.historical:
            self.assertTrue(builder.sealed_historical_preimage_is_applied())
        else:
            builder.check_output(self.document)
        self.assertEqual(
            builder.serialized(self.document), builder.OUTPUT_PATH.read_bytes()
        )

    def test_product_legacy_and_preserved_denominators_are_sealed(self) -> None:
        summary = self.document["summary"]
        self.assertEqual(97, summary["productOwnerDocumentCount"])
        self.assertEqual(96, summary["bossPatternCueOwnerDocumentCount"])
        self.assertEqual(1, summary["combatObjectOwnerDocumentCount"])
        self.assertEqual(3343, summary["productElementCount"])
        self.assertEqual(3032, summary["legacyAggregateElementCount"])
        self.assertEqual(311, summary["preservedNonLegacyElementCount"])
        self.assertEqual(54, summary["pureLegacyDocumentCount"])
        self.assertEqual(43, summary["mixedLegacyDocumentCount"])
        self.assertEqual(0, summary["failedButRemovedElementCount"])

        classifications = Counter(
            row["documentClassification"] for row in self.document["documents"]
        )
        self.assertEqual(
            {
                "PURE_LEGACY_V0": 54,
                "MIXED_LEGACY_V0_AND_PRESERVED_NONLEGACY": 43,
            },
            dict(classifications),
        )
        self.assertEqual(
            3343,
            sum(row["productElementCount"] for row in self.document["documents"]),
        )
        self.assertEqual(
            311,
            sum(
                row["preservedNonLegacyElementCount"]
                for row in self.document["documents"]
            ),
        )

    def test_every_legacy_row_is_fail_closed_not_an_exact_occurrence(self) -> None:
        rows = self.document["legacyRowAudit"]
        self.assertEqual(3032, len(rows))
        self.assertEqual(len(rows), len({row["auditId"] for row in rows}))
        self.assertEqual(
            {"UNIQUE_SOURCE_SYSTEM_CANDIDATE_ONLY"},
            {row["sourceSystemJoinStatus"] for row in rows},
        )
        self.assertTrue(
            all(row["migrationDisposition"] == "BLOCKED_SOURCE_BRANCH" for row in rows)
        )
        self.assertTrue(all(row["exactCarrierKey"] is None for row in rows))
        self.assertTrue(
            all(row["exactReviewedSourceOccurrenceId"] is None for row in rows)
        )
        self.assertTrue(
            all(
                "LEGACY_EMITTER_ORDINAL_IS_NOT_A_CARRIER_KEY"
                in row["blockingReasons"]
                for row in rows
            )
        )
        summary = self.document["summary"]
        self.assertEqual(0, summary["unknownSourceSystemJoinCount"])
        self.assertEqual(0, summary["duplicateSourceSystemJoinCount"])
        self.assertEqual(3032, summary["exactMaterialOrdinalJoinCount"])
        self.assertEqual(0, summary["blockedLegacyIdentityDriftCount"])
        self.assertEqual(0, summary["exactReviewedSourceOccurrenceJoinCount"])
        self.assertEqual(
            {"EXACT_MATERIAL_ORDINAL_JOIN"},
            {row["materialOrdinalJoinStatus"] for row in rows},
        )
        self.assertTrue(
            all(
                row["candidateMaterialObjectPath"].casefold()
                == row["legacySourceMaterialPath"].casefold()
                for row in rows
            )
        )

    def test_source_material_path_drift_is_fail_closed(self) -> None:
        source_catalog = builder.read_json(builder.SOURCE_CATALOG_PATH)
        catalog_system = next(
            row
            for row in source_catalog["sourceSystems"]
            if row["objectName"] == "par_o_rpbf_atk_08_03"
        )
        audit = next(
            row
            for row in self.document["legacyRowAudit"]
            if row["effectAssetId"] == "effect.valtan.red-blade-wave.active"
            and row["legacyEmitterOrdinal"] == 0
        )
        element = {
            "material": {
                "sourceMaterialPath": audit["legacySourceMaterialPath"]
            }
        }
        exact = builder.material_ordinal_join(element, catalog_system, 0)
        self.assertEqual("EXACT_MATERIAL_ORDINAL_JOIN", exact["materialOrdinalJoinStatus"])
        element["material"]["sourceMaterialPath"] += ".drift"
        proof = builder.material_ordinal_join(element, catalog_system, 0)
        self.assertEqual(
            "SOURCE_MATERIAL_PATH_MISMATCH", proof["materialOrdinalJoinStatus"]
        )
        self.assertEqual(
            "BLOCKED_LEGACY_IDENTITY_DRIFT", proof["migrationDisposition"]
        )
        self.assertEqual(
            "LEGACY_SOURCE_MATERIAL_PATH_MISMATCH", proof["blockingReason"]
        )
        self.assertIsNotNone(proof["candidateMaterialObjectPath"])

    def test_material_ordinal_out_of_range_is_fail_closed(self) -> None:
        source_catalog = builder.read_json(builder.SOURCE_CATALOG_PATH)
        catalog_system = next(
            row
            for row in source_catalog["sourceSystems"]
            if row["objectName"] == "par_o_rpbf_atk_08_03"
        )
        audit = next(
            row
            for row in self.document["legacyRowAudit"]
            if row["effectAssetId"] == "effect.valtan.red-blade-wave.active"
        )
        proof = builder.material_ordinal_join(
            {
                "material": {
                    "sourceMaterialPath": audit["legacySourceMaterialPath"]
                }
            },
            catalog_system,
            999,
        )
        self.assertEqual(
            "MATERIAL_ORDINAL_OUT_OF_RANGE", proof["materialOrdinalJoinStatus"]
        )
        self.assertEqual(
            "BLOCKED_LEGACY_IDENTITY_DRIFT", proof["migrationDisposition"]
        )
        self.assertEqual(
            "LEGACY_MATERIAL_ORDINAL_OUT_OF_RANGE", proof["blockingReason"]
        )
        self.assertIsNone(proof["candidateMaterialObjectPath"])

    def test_raw_aggregate_denominator_is_labeled_and_never_product_exact(self) -> None:
        candidates = self.document["legacyClipAggregateCandidates"]
        self.assertEqual(3403, len(candidates))
        self.assertEqual(
            len(candidates), len({row["candidateId"] for row in candidates})
        )
        self.assertEqual(
            {builder.DENOMINATOR_LABEL},
            {row["denominatorLabel"] for row in candidates},
        )
        self.assertTrue(all(row["productOccurrenceId"] is None for row in candidates))
        self.assertEqual(
            {"NOT_ESTABLISHED_LEGACY_CLIP_AGGREGATE"},
            {row["exactProductOccurrenceJoinStatus"] for row in candidates},
        )
        self.assertEqual(
            {"BLOCKED_SOURCE_BRANCH"},
            {row["migrationDisposition"] for row in candidates},
        )
        self.assertEqual(
            {"sprite": 2914, "mesh": 394, "decal": 89, "light": 6},
            dict(Counter(row["rendererShape"] for row in candidates)),
        )

    def test_source_system_and_exact_carrier_shapes_are_deduplicated(self) -> None:
        systems = self.document["sourceSystems"]
        self.assertEqual(115, len(systems))
        self.assertEqual(
            115, len({row["sourceSystemId"] for row in systems})
        )
        carriers = [carrier for row in systems for carrier in row["carriers"]]
        self.assertEqual(934, len(carriers))
        self.assertEqual(934, len({row["carrierKey"] for row in carriers}))
        self.assertEqual(
            {"sprite": 741, "mesh": 162, "decal": 29, "light": 2},
            dict(Counter(row["rendererShape"] for row in carriers)),
        )
        for system in systems:
            self.assertEqual(0, system["droppedCarrierCount"])
            self.assertEqual(0, system["duplicateCarrierCount"])
            self.assertEqual(
                len(system["carriers"]),
                len({row["sourceOrder"] for row in system["carriers"]}),
            )

    def test_document_source_systems_are_deduplicated_before_expansion(self) -> None:
        for row in self.document["documents"]:
            system_ids = row["deduplicatedSourceSystemIds"]
            self.assertEqual(len(system_ids), len(set(system_ids)))
            self.assertEqual(row["deduplicatedSourceSystemCount"], len(system_ids))
            self.assertEqual("BLOCKED_SOURCE_BRANCH", row["migrationDisposition"])
            self.assertEqual(0, row["exactReviewedSourceOccurrenceJoinCount"])

    def test_red_blade_active_is_a_world_root_combat_object_owner(self) -> None:
        row = next(
            row
            for row in self.document["documents"]
            if row["effectAssetId"] == "effect.valtan.red-blade-wave.active"
        )
        owner = row["productOwner"]
        self.assertEqual("COMBAT_OBJECT", owner["ownerKind"])
        self.assertEqual("WORLD_ROOT", owner["ownerRoot"])
        self.assertEqual(
            "combatobject.valtan.red-blade-wave.projectile",
            owner["combatObjectArchetypeId"],
        )
        self.assertEqual(
            "valtan.attack.red-blade-wave.active", owner["ownerStageActionId"]
        )
        self.assertEqual(
            "valtan.attack.red-blade-wave.active.clip.01",
            owner["ownerClipOccurrenceId"],
        )
        self.assertEqual("mesh_att_battle_12_10", owner["ownerClip"])
        self.assertIsNone(owner["occurrenceId"])
        self.assertEqual(15, row["productElementCount"])
        self.assertEqual(10, row["legacyAggregateElementCount"])
        self.assertEqual(5, row["preservedNonLegacyElementCount"])
        self.assertEqual(14, row["rawAggregateCarrierCandidateCount"])
        self.assertEqual({"sprite": 14}, row["rawAggregateRendererShapeCounts"])
        candidates = [
            candidate
            for candidate in self.document["legacyClipAggregateCandidates"]
            if candidate["effectAssetId"] == row["effectAssetId"]
        ]
        self.assertEqual(14, len(candidates))
        self.assertTrue(all(candidate["productOccurrenceId"] is None for candidate in candidates))
        self.assertTrue(
            all(
                candidate["ownerStageActionId"]
                == "valtan.attack.red-blade-wave.active"
                and candidate["ownerClip"] == "mesh_att_battle_12_10"
                and candidate["migrationDisposition"] == "BLOCKED_SOURCE_BRANCH"
                for candidate in candidates
            )
        )

    def test_every_declared_input_hash_matches_its_active_or_sealed_contract(self) -> None:
        if self.historical:
            sealed = self.receipt["legacyMigration"]["sealedInventory"]
            self.assertEqual(
                builder.canonical_sha256(self.document),
                sealed["canonicalSha256"],
            )
            migrations = {
                row["effectAssetId"]: row
                for row in self.receipt["legacyMigration"]["documents"]
            }
            for row in self.document["sources"]["productEffectDocuments"]:
                self.assertEqual(
                    row["sha256"],
                    migrations[row["effectAssetId"]]["preimageByteSha256"],
                )
        else:
            for row in self.document["sources"]["repository"]:
                path = builder.ROOT / Path(row["path"])
                self.assertTrue(path.is_file(), row["path"])
                self.assertEqual(row["sha256"], builder.sha256_file(path))
            for row in self.document["sources"]["productEffectDocuments"]:
                path = builder.ROOT / Path(row["path"])
                self.assertTrue(path.is_file(), row["path"])
                self.assertEqual(row["sha256"], builder.sha256_file(path))

    def test_schema_seals_the_same_contract_constants(self) -> None:
        schema_path = (
            builder.ROOT
            / "Tools/EffectPipeline/Schemas"
            / "lostark.valtan-legacy-v0-carrier-migration-inventory.schema.json"
        )
        schema = json.loads(schema_path.read_text(encoding="utf-8"))
        self.assertEqual(
            "lostark.valtan-legacy-v0-carrier-migration-inventory.schema.json",
            schema["$id"],
        )
        self.assertEqual(
            builder.SCHEMA_ID, schema["properties"]["schema"]["const"]
        )
        summary = schema["$defs"]["summary"]["properties"]
        self.assertEqual(97, summary["productOwnerDocumentCount"]["const"])
        self.assertEqual(3032, summary["legacyAggregateElementCount"]["const"])
        self.assertEqual(
            3403, summary["rawAggregateCarrierCandidateCount"]["const"]
        )
        self.assertEqual(934, summary["uniqueSourceCarrierCount"]["const"])
        self.assertEqual(3032, summary["exactMaterialOrdinalJoinCount"]["const"])
        self.assertEqual(0, summary["blockedLegacyIdentityDriftCount"]["const"])

    def test_atomic_write_is_idempotent_and_rolls_back_replace_failure(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            output_path = Path(directory) / "inventory.json"
            self.assertTrue(builder.write_output(self.document, output_path))
            first = output_path.read_bytes()
            self.assertFalse(builder.write_output(self.document, output_path))
            self.assertEqual(first, output_path.read_bytes())

            modified = dict(self.document)
            modified["completionPolicy"] = self.document["completionPolicy"] + ";TEST"
            with mock.patch.object(builder.os, "replace", side_effect=OSError("boom")):
                with self.assertRaises(OSError):
                    builder.write_output(modified, output_path)
            self.assertEqual(first, output_path.read_bytes())
            self.assertFalse(
                output_path.with_name(output_path.name + ".staging").exists()
            )


if __name__ == "__main__":
    unittest.main()
