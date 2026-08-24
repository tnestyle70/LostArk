#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import os
import sys
import tempfile
import unittest
from collections import Counter
from pathlib import Path
from unittest import mock

TOOLS = Path(__file__).resolve().parent
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import materialize_valtan_carrier_v1 as materializer
import valtan_carrier_v1_successor_lineage as successor_lineage


class ValtanCarrierV1MaterializerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.state, cls.outputs, cls.receipt = materializer.build_outputs()
        cls.successor_receipt = materializer._load_successor_receipt()
        cls.historical_receipt = materializer.read_json(
            materializer.RECEIPT_PATH
        )
        cls.additive_receipt = materializer.read_json(
            materializer.ADDITIVE_RECEIPT_PATH
        )
        assert (
            cls.receipt["additiveReviewedOwnerAppend"]
            == cls.additive_receipt
        )
        cls.catalog = json.loads(
            cls.outputs[materializer.CATALOG_PATH].decode("utf-8")
        )
        cls.cues = json.loads(cls.outputs[materializer.CUE_PATH].decode("utf-8"))
        cls.documents = {}
        document_rows = list(cls.receipt["outputs"]["targetDocuments"])
        document_rows.extend(
            cls.receipt["additiveReviewedOwnerAppend"]["targetDocuments"]
        )
        for row in document_rows:
            path = materializer.ROOT / row["path"]
            cls.documents[row["effectAssetId"]] = json.loads(
                cls.outputs[path].decode("utf-8")
                if path in cls.outputs
                else path.read_text(encoding="utf-8")
            )
        cls.protected_path = (
            materializer.AUTHORED_ROOT
            / f"{materializer.PROTECTED_EFFECT_ID}.effect.json"
        )
        cls.protected = json.loads(
            cls.outputs[cls.protected_path].decode("utf-8")
            if cls.protected_path in cls.outputs
            else cls.protected_path.read_text(encoding="utf-8")
        )
        cls.successor_target_ids = set(
            successor_lineage.successor_documents(cls.successor_receipt)
        ).intersection(cls.documents)
        cls.additive_target_ids = {
            str(row["effectAssetId"])
            for row in cls.receipt["additiveReviewedOwnerAppend"][
                "targetDocuments"
            ]
        }

    def test_exact_reviewed_denominator_and_decal_expansion(self) -> None:
        summary = self.receipt["summary"]
        for key, expected in materializer.EXPECTED.items():
            self.assertEqual(expected, summary[key], key)
        self.assertEqual(materializer.BASE_SUMMARY, summary)
        additive = self.receipt["additiveReviewedOwnerAppend"]
        self.assertEqual(
            "lostark.valtan-carrier-v1-four-pillars-additive-receipt",
            additive["schema"],
        )
        self.assertNotIn(
            "additiveReviewedOwnerAppend", self.historical_receipt
        )
        self.assertEqual(
            materializer.BASE_RECEIPT_CANONICAL_SHA256,
            materializer.canonical_sha256(self.historical_receipt),
        )
        self.assertNotIn(materializer.RECEIPT_PATH, self.outputs)
        self.assertIn(materializer.ADDITIVE_RECEIPT_PATH, self.outputs)
        self.assertEqual(
            {
                "successorBase": 54,
                "additiveDelta": 2,
                "final": 56,
            },
            {
                key: additive["liveProductUnion"]["valtanCatalogRows"][key]
                for key in ("successorBase", "additiveDelta", "final")
            },
        )
        self.assertEqual(
            {
                "successorBase": 47,
                "additiveDelta": 2,
                "final": 49,
            },
            {
                key: additive["liveProductUnion"]["bossRootCueRows"][key]
                for key in ("successorBase", "additiveDelta", "final")
            },
        )
        derived = additive["historicalDerivedCounts"]
        self.assertEqual(
            {"base": 669, "delta": 26, "historicalDerived": 695,
             "meaning": "RECEIPT_ARITHMETIC_ONLY_NOT_CURRENT_PHYSICAL_PRODUCT_COUNT"},
            derived["carrierProductRowCount"],
        )
        self.assertEqual(48, derived["carrierCatalogOwnerCount"]["historicalDerived"])
        self.assertEqual(46, derived["bossRootCueCount"]["historicalDerived"])
        self.assertEqual(46, len(self.documents))
        successor_entries = successor_lineage.successor_documents(
            self.successor_receipt
        )
        expected_live_count = 657 - sum(
            successor_entries[effect_id]["historicalBaseline"]["elementCount"]
            for effect_id in self.successor_target_ids
        ) + sum(
            successor_entries[effect_id]["finalDocument"]["elementCount"]
            for effect_id in self.successor_target_ids
        ) + sum(
            int(row["elementCount"])
            for row in additive["targetDocuments"]
        )
        self.assertEqual(
            expected_live_count,
            sum(len(row["elements"]) for row in self.documents.values()),
        )
        self.assertEqual(669, summary["finalValtanProductRowCount"])
        self.assertEqual(
            materializer.BASE_DECAL_EXPANSION["newClipOccurrenceIds"],
            self.receipt["decalExpansion"]["newClipOccurrenceIds"],
        )
        self.assertEqual(
            materializer.BASE_DECAL_EXPANSION["newPatternIds"],
            self.receipt["decalExpansion"]["newPatternIds"],
        )
        self.assertEqual(
            [
                "valtan.mechanic.four-pillars-105.takeoff.clip.01",
                "valtan.mechanic.four-pillars-105.target-cone.clip.01",
            ],
            additive["decalAppend"]["clipOccurrenceIds"],
        )
        self.assertEqual(2, additive["decalAppend"]["exactProjectionCount"])

        ledger = self.receipt["reviewedProjectionLedger"]
        source_only = self.receipt["reviewedSourceOnlyOccurrences"]
        core = [row for row in ledger if row["disposition"] == "EXECUTABLE_CORE"]
        self.assertEqual(1577, len(ledger))
        self.assertEqual(197, len(source_only))
        self.assertEqual(660, len(core))
        self.assertEqual(45, len({row["clipOccurrenceId"] for row in core}))
        self.assertEqual(24, len({row["patternId"] for row in core}))
        self.assertEqual(
            {"decal": 32, "mesh": 173, "sprite": 455},
            dict(sorted(Counter(row["rendererShape"] for row in core).items())),
        )
        delta = additive["deltaEvidence"]
        self.assertEqual(54, delta["reviewedProjectionLedger"]["count"])
        self.assertEqual(26, delta["reviewedProjectionLedger"]["executableCoreCount"])
        self.assertEqual(9, delta["reviewedSourceOnlyOccurrences"]["count"])
        self.assertEqual(26, delta["sourceElements"]["count"])
        self.assertEqual(2, delta["clipGroups"]["count"])
        self.assertTrue(
            all(
                row["occurrenceFullKey"]
                and row["carrierKey"]
                and row["rendererShape"]
                and "runtimeResources" in row
                and "materialObjectPaths" in row
                for row in ledger
            )
        )

    def test_all_materialized_rows_use_exact_carrier_and_common_alpha(self) -> None:
        effect_ids = {
            "effect.valtan.carrier-v1.mechanic.four-pillars-105.takeoff.clip-01",
            "effect.valtan.carrier-v1.mechanic.four-pillars-105.target-cone.clip-01",
        }
        by_source = {
            element["sourceNode"]: element
            for effect_id, document in self.documents.items()
            if effect_id in effect_ids
            for element in document["elements"]
        }
        _, materialized, blockers = materializer._build_projections(
            materializer._load_inventory()
        )
        ledger = {
            row.get("sourceNode"): row
            for row in blockers["reviewedProjectionLedger"]
            if row.get("sourceNode")
        }
        self.assertEqual(26, len(by_source))
        shapes = Counter()
        sources = [
            materializer._incremental_source_element(row)
            for row in materialized
        ]
        self.assertEqual(26, len(sources))
        for source in sources:
            element = by_source[source["sourceNode"]]
            material = element["material"]
            self.assertEqual("effect.standard", material["templateId"])
            self.assertEqual(
                "alpha_two_sided_depth_read", material["renderProfile"]
            )
            expected_profile = copy.deepcopy(
                source["originalMaterial"]["sourceProfile"]
            )
            expected_profile["enabled"] = False
            self.assertEqual(expected_profile, material["sourceProfile"])
            self.assertEqual(
                source["originalMaterial"]["sourceMaterialPath"],
                material["sourceMaterialPath"],
            )
            self.assertTrue(element["sourceRecipe"]["enabled"])
            self.assertEqual(
                materializer.canonical_sha256(element["resources"]),
                materializer.canonical_sha256(
                    ledger[source["sourceNode"]]["runtimeResources"]
                ),
            )
            shapes[source["rendererShape"]] += 1
            mesh_models = [
                row
                for row in element["resources"]
                if row.get("slotId") == "meshModel"
            ]
            if source["rendererShape"] == "mesh":
                self.assertEqual("particle", element["kind"])
                self.assertEqual(1, len(mesh_models))
                self.assertEqual(0.01, element["detail"]["mesh"]["modelPreScale"])
            elif source["rendererShape"] == "decal":
                self.assertEqual("decal", element["kind"])
                self.assertEqual("decal", element["inventoryRendererShape"])
                self.assertEqual("decal", element["sourceRecipe"]["rendererShape"])
                self.assertFalse(mesh_models)
                self.assertEqual(
                    1,
                    sum(row.get("slotId") == "base" for row in element["resources"]),
                )
                self.assertNotIn("modelPreScale", element["detail"].get("mesh", {}))
            else:
                self.assertFalse(mesh_models)
                self.assertNotIn("modelPreScale", element["detail"].get("mesh", {}))
        self.assertEqual(
            {"decal": 2, "mesh": 8, "sprite": 16},
            dict(sorted(shapes.items())),
        )

    def test_historical_materialized_rows_keep_successor_lineage(self) -> None:
        by_source = {
            element["sourceNode"]: element
            for effect_id, document in self.documents.items()
            if effect_id not in self.successor_target_ids
            and effect_id not in self.additive_target_ids
            for element in document["elements"]
        }
        ledger = {
            row.get("sourceNode"): row
            for row in self.receipt["reviewedProjectionLedger"]
            if row.get("sourceNode")
        }
        managed_sources = [
            row
            for row in self.receipt["sourceElements"]
            if row["effectAssetId"] not in self.successor_target_ids
        ]
        self.assertEqual(577, len(managed_sources))
        self.assertEqual(577, len(by_source))
        shapes = Counter()
        for source in managed_sources:
            element = by_source[source["sourceNode"]]
            material = element["material"]
            self.assertEqual("effect.standard", material["templateId"])
            self.assertEqual(
                "alpha_two_sided_depth_read", material["renderProfile"]
            )
            expected_profile = copy.deepcopy(
                source["originalMaterial"]["sourceProfile"]
            )
            expected_profile["enabled"] = False
            self.assertEqual(expected_profile, material["sourceProfile"])
            self.assertEqual(
                source["originalMaterial"]["sourceMaterialPath"],
                material["sourceMaterialPath"],
            )
            self.assertTrue(element["sourceRecipe"]["enabled"])
            self.assertEqual(
                materializer.canonical_sha256(element["resources"]),
                materializer.canonical_sha256(
                    ledger[source["sourceNode"]]["runtimeResources"]
                ),
            )
            shapes[source["rendererShape"]] += 1
            mesh_models = [
                row
                for row in element["resources"]
                if row.get("slotId") == "meshModel"
            ]
            if source["rendererShape"] == "mesh":
                self.assertEqual("particle", element["kind"])
                self.assertEqual(1, len(mesh_models))
                self.assertEqual(
                    0.01, element["detail"]["mesh"]["modelPreScale"]
                )
            elif source["rendererShape"] == "decal":
                self.assertEqual("decal", element["kind"])
                self.assertEqual("decal", element["inventoryRendererShape"])
                self.assertEqual(
                    "decal", element["sourceRecipe"]["rendererShape"]
                )
                self.assertFalse(mesh_models)
                self.assertEqual(
                    1,
                    sum(
                        row.get("slotId") == "base"
                        for row in element["resources"]
                    ),
                )
                self.assertNotIn(
                    "modelPreScale", element["detail"].get("mesh", {})
                )
            else:
                self.assertFalse(mesh_models)
                self.assertNotIn(
                    "modelPreScale", element["detail"].get("mesh", {})
                )
        expected_shapes = Counter(
            row["rendererShape"] for row in managed_sources
        )
        self.assertEqual(
            dict(sorted(expected_shapes.items())),
            dict(sorted(shapes.items())),
        )

    def test_one_product_owner_per_clip_and_explicit_exceptions(self) -> None:
        base_catalog, base_cues = (
            materializer._validate_successor_base_product(
                self.successor_receipt, self.catalog, self.cues
            )
        )
        historical_cues = successor_lineage.project_historical_cues(
            base_cues, self.successor_receipt
        )
        carrier_cues = [
            row
            for row in historical_cues["cues"]
            if str(row.get("bindingId") or "").startswith(
                "cue.valtan.carrier-v1."
            )
        ]
        self.assertEqual(43, len(carrier_cues))
        self.assertEqual(43, len({row["clipOccurrenceId"] for row in carrier_cues}))
        historical_owners = Counter(
            row["clipOccurrenceId"] for row in historical_cues["cues"]
        )
        owners = Counter(row["clipOccurrenceId"] for row in self.cues["cues"])
        self.assertEqual(2, owners[materializer.PROTECTED_CLIP_ID])
        self.assertTrue(
            all(
                value == 1
                for clip_id, value in owners.items()
                if clip_id != materializer.PROTECTED_CLIP_ID
            )
        )
        live_binding_ids = {
            str(row["bindingId"]) for row in self.cues["cues"]
        }
        self.assertTrue(
            {
                str(row["bindingId"])
                for row in self.receipt["additiveReviewedOwnerAppend"][
                    "cueRows"
                ]
            }.issubset(live_binding_ids)
        )
        self.assertEqual(
            0, historical_owners[materializer.RED_BLADE_CLIP_ID]
        )
        self.assertTrue(
            all(
                row["sourceStartMs"] == 0
                and row["sourceEndMs"] is None
                and row["stopPolicy"] == "natural"
                for row in carrier_cues
            )
        )
        four_pillars = {
            "valtan.mechanic.four-pillars-105.takeoff.clip.01": (
                "TAKEOFF",
                "valtan.mechanic.four-pillars-105.takeoff",
                3,
            ),
            "valtan.mechanic.four-pillars-105.target-cone.clip.01": (
                "TARGET_CONE",
                "valtan.mechanic.four-pillars-105.target-cone",
                23,
            ),
        }
        for clip_id, (stage_id, action_id, element_count) in four_pillars.items():
            self.assertEqual(1, owners[clip_id])
            cue = next(
                row for row in self.cues["cues"]
                if row["clipOccurrenceId"] == clip_id
            )
            self.assertEqual("VALTAN_FOUR_PILLARS_105", cue["patternId"])
            self.assertEqual(stage_id, cue["stageId"])
            self.assertEqual(action_id, cue["actionId"])
            self.assertNotIn("high-jump", cue["effectAssetId"])
            self.assertEqual(
                element_count,
                len(self.documents[cue["effectAssetId"]]["elements"]),
            )

        historical_catalog = successor_lineage.project_historical_catalog(
            base_catalog, self.successor_receipt
        )
        historical_catalog_ids = {
            row["effectAssetId"]
            for row in historical_catalog["effects"]
            if row["effectAssetId"].startswith("effect.valtan.")
        }
        catalog_ids = {
            row["effectAssetId"]
            for row in base_catalog["effects"]
            if row["effectAssetId"].startswith("effect.valtan.")
        }
        self.assertEqual(54, len(catalog_ids))
        self.assertEqual(46, len(historical_catalog_ids))
        self.assertIn(materializer.RED_BLADE_EFFECT_ID, catalog_ids)
        self.assertIn(materializer.PROTECTED_EFFECT_ID, catalog_ids)
        self.assertIn("effect.valtan.sky-axe.active", catalog_ids)
        self.assertNotIn(materializer.WATERTRAIL_CANARY_EFFECT_ID, catalog_ids)
        self.assertTrue(
            all(
                effect_id.startswith("effect.valtan.carrier-v1.")
                or effect_id.endswith(".v1.unified")
                or effect_id
                in {
                    materializer.RED_BLADE_EFFECT_ID,
                    materializer.PROTECTED_EFFECT_ID,
                    "effect.valtan.sky-axe.active",
                }
                for effect_id in historical_catalog_ids
            )
        )
        successors = self.receipt["retiredOwnerSuccessorMappings"]
        self.assertEqual(105, len(successors))
        self.assertEqual(
            105, len({row["retiredBindingId"] for row in successors})
        )
        cue_ids = {row["bindingId"] for row in historical_cues["cues"]}
        replaced = [
            row
            for row in successors
            if row["disposition"]
            == "REPLACED_BY_EXACT_CARRIER_V1_CLIP_OWNER"
        ]
        blocked = [
            row
            for row in successors
            if row["disposition"]
            == "RETIRED_NO_EXACT_REVIEWED_CARRIER_OWNER"
        ]
        self.assertEqual(48, len(replaced))
        self.assertEqual(57, len(blocked))
        self.assertEqual(42, len({row["replacementBindingId"] for row in replaced}))
        self.assertTrue(
            all(
                row["replacementBindingId"] in cue_ids
                and row["replacementEffectAssetId"] in catalog_ids
                for row in replaced
            )
        )
        self.assertTrue(
            all(
                row["replacementBindingId"] is None
                and row["replacementEffectAssetId"] is None
                for row in blocked
            )
        )
        additive_successors = self.receipt["additiveReviewedOwnerAppend"][
            "successorMappings"
        ]
        live_cue_ids = {row["bindingId"] for row in self.cues["cues"]}
        live_catalog_ids = {
            row["effectAssetId"] for row in self.catalog["effects"]
        }
        self.assertEqual(2, len(additive_successors))
        self.assertTrue(
            all(
                row["disposition"]
                == "REPLACED_BY_EXACT_CARRIER_V1_CLIP_OWNER"
                and row["replacementBindingId"] in live_cue_ids
                and row["replacementEffectAssetId"] in live_catalog_ids
                for row in additive_successors
            )
        )

    def test_every_retired_document_is_an_empty_evidence_shell(self) -> None:
        migration = self.receipt["legacyMigration"]
        rows = migration["documents"]
        self.assertEqual(3032, sum(row["strictLegacyRemovedCount"] for row in rows))
        self.assertEqual(539, sum(row["exactSourceMovedCount"] for row in rows))
        retired = set(migration["retiredEffectAssetIds"])
        self.assertEqual(105, len(retired))
        by_effect = {row["effectAssetId"]: row for row in rows}
        for effect_id in retired:
            row = by_effect[effect_id]
            path = materializer.ROOT / row["path"]
            shell = json.loads(
                self.outputs[path].decode("utf-8")
                if path in self.outputs
                else path.read_text(encoding="utf-8")
            )
            self.assertEqual(effect_id, shell["effectAssetId"])
            self.assertEqual([], shell["elements"])
            self.assertRegex(row["preimageCanonicalSha256"], r"^[0-9a-f]{64}$")
            self.assertRegex(row["preimageByteSha256"], r"^[0-9a-f]{64}$")

    def test_protected_whirlwind_aliases_and_wmodel_scale_are_sealed(self) -> None:
        proofs = self.receipt["protectedWhirlwindExactAliasProof"]
        self.assertEqual(3, len(proofs))
        self.assertTrue(all(row["joinStatus"].startswith("EXACT_") for row in proofs))
        elements = {row["id"]: row for row in self.protected["elements"]}
        mesh_proofs = [row for row in proofs if row["rendererShape"] == "mesh"]
        self.assertEqual(2, len(mesh_proofs))
        for proof in mesh_proofs:
            element = elements[proof["protectedElementId"]]
            self.assertEqual(0.01, element["detail"]["mesh"]["modelPreScale"])
            self.assertEqual(
                1,
                sum(row.get("slotId") == "meshModel" for row in element["resources"]),
            )

    def test_tuning_merge_preserves_profile_evidence_and_reseals_identity(self) -> None:
        _, materialized, _ = materializer._build_projections(
            materializer._load_inventory()
        )
        row = next(row for row in materialized if row["rendererShape"] == "mesh")
        exact = copy.deepcopy(row["element"])
        exact["material"]["sourceProfile"] = {
            "enabled": True,
            "parentMaterialPath": "exact.parent",
            "textures": [{"slotId": "base", "objectPath": "exact.texture"}],
            "scalars": [{"name": "dissolve", "value": 0.25}],
            "dynamicParameterSemantics": ["x", "y", "z", "w"],
        }
        existing = copy.deepcopy(exact)
        existing["visible"] = False
        existing["detail"]["transform"]["position"] = [3.0, 4.0, 5.0]
        existing["detail"]["timing"]["startDelaySeconds"] = 999.0
        existing["detail"]["mesh"].pop("modelPreScale", None)
        existing["material"]["sourceMaterialPath"] = "wrong.material"
        result = materializer._common_translucent_element(exact, existing)
        self.assertFalse(result["visible"])
        self.assertEqual([3.0, 4.0, 5.0], result["detail"]["transform"]["position"])
        self.assertEqual(
            row["sourceTimeSeconds"], result["detail"]["timing"]["startDelaySeconds"]
        )
        self.assertEqual(0.01, result["detail"]["mesh"]["modelPreScale"])
        expected_profile = copy.deepcopy(exact["material"]["sourceProfile"])
        expected_profile["enabled"] = False
        self.assertEqual(expected_profile, result["material"]["sourceProfile"])
        self.assertEqual(
            row["originalMaterial"]["sourceMaterialPath"],
            result["material"]["sourceMaterialPath"],
        )

    def test_staged_applied_receipt_is_repeatable_and_ledger_sealed(self) -> None:
        core, materialized, blockers = materializer._build_projections(
            materializer._load_inventory()
        )
        state, writes, _ = materializer._build_additive_outputs(
            core,
            materialized,
            blockers,
            self.successor_receipt,
        )
        self.assertEqual("APPLIED", state)
        self.assertEqual({}, materializer._changed_outputs(writes))
        drifted = copy.deepcopy(blockers)
        drifted["reviewedProjectionLedger"][0]["conversionStatus"] += ".drift"
        with self.assertRaisesRegex(
            materializer.MaterializeError,
            "100-bar additive receipt proof drifted",
        ):
            materializer._build_additive_outputs(
                core,
                materialized,
                drifted,
                self.successor_receipt,
            )

    def test_additive_append_fails_closed_on_base_product_drift(self) -> None:
        target_effect_ids = {
            row["effectAssetId"]
            for row in self.receipt["additiveReviewedOwnerAppend"]
            ["targetDocuments"]
        }
        catalog = materializer.read_json(materializer.CATALOG_PATH)
        cues = materializer.read_json(materializer.CUE_PATH)
        _, materialized, blockers = materializer._build_projections(
            materializer._load_inventory()
        )
        groups = {}
        for row in materialized:
            groups.setdefault(row["clipOccurrenceId"], []).append(row)
        additive_delta = {
            "reviewedProjectionLedger": sorted(
                copy.deepcopy(blockers["reviewedProjectionLedger"]),
                key=lambda row: (row["occurrenceFullKey"], row["carrierKey"]),
            ),
            "reviewedSourceOnlyOccurrences": sorted(
                copy.deepcopy(blockers["reviewedSourceOnlyOccurrences"]),
                key=lambda row: row["occurrenceFullKey"],
            ),
            "sourceElements": sorted(
                (
                    materializer._incremental_source_element(row)
                    for row in materialized
                ),
                key=lambda row: row["fullSourceKey"],
            ),
            "clipGroups": sorted(
                (
                    materializer._incremental_clip_group(group)
                    for group in groups.values()
                ),
                key=lambda row: row["clipOccurrenceId"],
            ),
        }

        drifted_header = copy.deepcopy(self.receipt)
        drifted_header["policy"] += "; DRIFT"
        with self.assertRaisesRegex(
            materializer.MaterializeError,
            "pre-existing carrier receipt full evidence drifted",
        ):
            materializer._validate_and_project_base_receipt_evidence(
                drifted_header,
                self.successor_receipt,
                target_effect_ids,
                catalog,
                cues,
                additive_delta,
                blockers,
            )

        drifted_catalog = copy.deepcopy(catalog)
        base_catalog_id = self.receipt["outputs"]["targetDocuments"][0][
            "effectAssetId"
        ]
        base_catalog_row = next(
            row
            for row in drifted_catalog["effects"]
            if row["effectAssetId"] == base_catalog_id
        )
        base_catalog_row["authoringPath"] += ".drift"
        with self.assertRaisesRegex(
            materializer.MaterializeError,
            "catalog.*drifted",
        ):
            materializer._validate_and_project_base_receipt_evidence(
                copy.deepcopy(self.historical_receipt),
                self.successor_receipt,
                target_effect_ids,
                drifted_catalog,
                cues,
                additive_delta,
                blockers,
            )

        drifted_cues = copy.deepcopy(cues)
        base_cue = next(
            row
            for row in drifted_cues["cues"]
            if row["effectAssetId"] == base_catalog_id
        )
        base_cue["sourceStartMs"] += 1
        with self.assertRaisesRegex(
            materializer.MaterializeError,
            "cue.*drifted",
        ):
            materializer._validate_and_project_base_receipt_evidence(
                copy.deepcopy(self.historical_receipt),
                self.successor_receipt,
                target_effect_ids,
                catalog,
                drifted_cues,
                additive_delta,
                blockers,
            )

        base_document_row = self.receipt["outputs"]["targetDocuments"][0]
        base_document_path = materializer.ROOT / base_document_row["path"]
        drifted_document = materializer.read_json(base_document_path)
        drifted_document["elements"][0]["visible"] = not drifted_document[
            "elements"
        ][0]["visible"]
        real_read_json = materializer.read_json

        def read_with_document_drift(path: Path) -> dict:
            if path.resolve() == base_document_path.resolve():
                return copy.deepcopy(drifted_document)
            return real_read_json(path)

        with mock.patch.object(
            materializer, "read_json", side_effect=read_with_document_drift
        ):
            with self.assertRaisesRegex(
                materializer.MaterializeError,
                "pre-existing carrier target document drifted",
            ):
                materializer._validate_and_project_base_receipt_evidence(
                    copy.deepcopy(self.historical_receipt),
                    self.successor_receipt,
                    target_effect_ids,
                    catalog,
                    cues,
                    additive_delta,
                    blockers,
                )

    def test_temp_transaction_second_build_is_applied_and_idempotent(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            temp_root = Path(temporary).resolve()
            staged: dict[Path, bytes] = {}
            for path, payload in self.outputs.items():
                relative = path.resolve().relative_to(materializer.ROOT.resolve())
                staged[temp_root / relative] = payload

            required = [
                materializer.RECEIPT_PATH,
                materializer.SUCCESSOR_RECEIPT_PATH,
                materializer.ADDITIVE_RECEIPT_PATH,
                materializer.LEGACY_INVENTORY_PATH,
                materializer.AUTHORED_ROOT
                / f"{materializer.PROTECTED_EFFECT_ID}.effect.json",
                materializer.AUTHORED_ROOT
                / "effect.valtan.sky-axe.active.effect.json",
                *(
                    materializer.ROOT / row["path"]
                    for row in self.receipt["outputs"]["targetDocuments"]
                ),
                *(
                    materializer.ROOT / row["path"]
                    for row in self.receipt["legacyMigration"]["documents"]
                ),
                *(
                    materializer.authored_path(row)
                    for row in self.catalog["effects"]
                    if row["effectAssetId"].startswith("effect.valtan.")
                ),
            ]
            for path in required:
                target = temp_root / path.resolve().relative_to(
                    materializer.ROOT.resolve()
                )
                if target not in staged:
                    staged[target] = path.read_bytes()
            materializer._atomic_replace(staged)

            temp_catalog = temp_root / "Data/Effects/EffectCatalog.json"
            temp_cues = (
                temp_root
                / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
            )
            temp_authored = temp_root / "Data/Effects/Authored"
            temp_receipt = (
                temp_root
                / "Data/Effects/Imported/Valtan/CarrierV1"
                / "Valtan.carrier-v1-materialization-receipt.v1.json"
            )
            temp_successor_receipt = (
                temp_root
                / "Data/Effects/Imported/Valtan/CarrierV1"
                / "Valtan.carrier-v1-successor-lineage-receipt.v1.json"
            )
            temp_additive_receipt = (
                temp_root
                / "Data/Effects/Imported/Valtan/CarrierV1"
                / "Valtan.carrier-v1-four-pillars-additive-receipt.v1.json"
            )
            temp_legacy = (
                temp_root
                / "Data/Effects/Imported/Valtan/CarrierV1"
                / "Valtan.legacy-v0-carrier-migration-inventory.v1.json"
            )
            with mock.patch.multiple(
                materializer,
                ROOT=temp_root,
                CATALOG_PATH=temp_catalog,
                CUE_PATH=temp_cues,
                AUTHORED_ROOT=temp_authored,
                RECEIPT_PATH=temp_receipt,
                SUCCESSOR_RECEIPT_PATH=temp_successor_receipt,
                ADDITIVE_RECEIPT_PATH=temp_additive_receipt,
                LEGACY_INVENTORY_PATH=temp_legacy,
            ):
                state, writes, _ = materializer.build_outputs()
                self.assertEqual("APPLIED", state)
                self.assertEqual({}, materializer._changed_outputs(writes))
                self.assertEqual(0, materializer.main(["--mode", "check"]))
            temp_receipt_document = json.loads(
                temp_receipt.read_text(encoding="utf-8")
            )
            self.assertEqual(
                materializer.BASE_SUMMARY,
                temp_receipt_document["summary"],
            )
            self.assertNotIn(
                "additiveReviewedOwnerAppend", temp_receipt_document
            )
            temp_additive_document = json.loads(
                temp_additive_receipt.read_text(encoding="utf-8")
            )
            self.assertEqual(
                48,
                temp_additive_document
                ["historicalDerivedCounts"]["carrierCatalogOwnerCount"]
                ["historicalDerived"],
            )

    def test_atomic_failure_restores_every_target(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            first = root / "first.json"
            second = root / "nested" / "second.json"
            first.write_bytes(b"before")
            calls = 0

            def fail_second(source: str, target: str) -> None:
                nonlocal calls
                calls += 1
                if calls == 2:
                    raise OSError("injected replace failure")
                os.replace(source, target)

            with self.assertRaises(materializer.MaterializeError):
                materializer._atomic_replace(
                    {first: b"after", second: b"created"}, replace=fail_second
                )
            self.assertEqual(b"before", first.read_bytes())
            self.assertFalse(second.exists())

    def test_legacy_hash_and_material_ordinal_drift_fail_closed(self) -> None:
        _, authorizations = materializer._load_legacy_authorizations()
        self.assertEqual(3032, len(authorizations))
        (effect_id, element_index), authorization = next(iter(authorizations.items()))
        catalog = {
            row["effectAssetId"]: row
            for row in json.loads(
                materializer.CATALOG_PATH.read_text(encoding="utf-8")
            )["effects"]
        }
        if effect_id in catalog:
            document = json.loads(
                (materializer.ROOT / "Data" / catalog[effect_id]["authoringPath"])
                .read_text(encoding="utf-8")
            )
            element = copy.deepcopy(document["elements"][element_index])
            materializer._validate_legacy_preimage_row(authorization, element)
            element["material"]["sourceMaterialPath"] += ".drift"
        else:
            element = {
                "id": authorization["elementId"],
                "material": {
                    "sourceMaterialPath": authorization[
                        "legacySourceMaterialPath"
                    ]
                    + ".drift"
                },
            }
        with self.assertRaises(materializer.MaterializeError):
            materializer._validate_legacy_preimage_row(authorization, element)

        join_drift = copy.deepcopy(authorization)
        join_drift["materialOrdinalJoinStatus"] = "AMBIGUOUS"
        join_drift["sourceSystemJoinStatus"] = "AMBIGUOUS"
        with self.assertRaises(materializer.MaterializeError):
            materializer._legacy_authorization_key(join_drift)


if __name__ == "__main__":
    unittest.main()
