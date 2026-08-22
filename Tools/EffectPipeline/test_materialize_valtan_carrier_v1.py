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


class ValtanCarrierV1MaterializerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.state, cls.outputs, cls.receipt = materializer.build_outputs()
        cls.catalog = json.loads(
            cls.outputs[materializer.CATALOG_PATH].decode("utf-8")
        )
        cls.cues = json.loads(cls.outputs[materializer.CUE_PATH].decode("utf-8"))
        cls.documents = {}
        for row in cls.receipt["outputs"]["targetDocuments"]:
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

    def test_exact_reviewed_denominator_and_decal_expansion(self) -> None:
        summary = self.receipt["summary"]
        for key, expected in materializer.EXPECTED.items():
            self.assertEqual(expected, summary[key], key)
        self.assertEqual(44, len(self.documents))
        self.assertEqual(
            657, sum(len(row["elements"]) for row in self.documents.values())
        )
        self.assertEqual(669, summary["finalValtanProductRowCount"])
        self.assertEqual(
            materializer.DECAL_EXPANDED_CLIP_IDS,
            self.receipt["decalExpansion"]["newClipOccurrenceIds"],
        )
        self.assertEqual(
            materializer.DECAL_EXPANDED_PATTERN_IDS,
            self.receipt["decalExpansion"]["newPatternIds"],
        )

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
        by_source = {
            element["sourceNode"]: element
            for document in self.documents.values()
            for element in document["elements"]
        }
        ledger = {
            row.get("sourceNode"): row
            for row in self.receipt["reviewedProjectionLedger"]
            if row.get("sourceNode")
        }
        self.assertEqual(657, len(by_source))
        shapes = Counter()
        for source in self.receipt["sourceElements"]:
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
            {"decal": 32, "mesh": 171, "sprite": 454},
            dict(sorted(shapes.items())),
        )

    def test_one_product_owner_per_clip_and_explicit_exceptions(self) -> None:
        carrier_cues = [
            row
            for row in self.cues["cues"]
            if str(row.get("bindingId") or "").startswith(
                "cue.valtan.carrier-v1."
            )
        ]
        self.assertEqual(43, len(carrier_cues))
        self.assertEqual(43, len({row["clipOccurrenceId"] for row in carrier_cues}))
        owners = Counter(row["clipOccurrenceId"] for row in self.cues["cues"])
        self.assertTrue(all(value == 1 for value in owners.values()))
        self.assertEqual(0, owners[materializer.RED_BLADE_CLIP_ID])
        self.assertTrue(
            all(
                row["sourceStartMs"] == 0
                and row["sourceEndMs"] is None
                and row["stopPolicy"] == "natural"
                for row in carrier_cues
            )
        )

        catalog_ids = {
            row["effectAssetId"] for row in self.catalog["effects"]
            if row["effectAssetId"].startswith("effect.valtan.")
        }
        self.assertEqual(46, len(catalog_ids))
        self.assertIn(materializer.RED_BLADE_EFFECT_ID, catalog_ids)
        self.assertIn(materializer.PROTECTED_EFFECT_ID, catalog_ids)
        self.assertIn("effect.valtan.sky-axe.active", catalog_ids)
        self.assertNotIn(materializer.WATERTRAIL_CANARY_EFFECT_ID, catalog_ids)
        self.assertTrue(
            all(
                effect_id.startswith("effect.valtan.carrier-v1.")
                or effect_id
                in {
                    materializer.RED_BLADE_EFFECT_ID,
                    materializer.PROTECTED_EFFECT_ID,
                    "effect.valtan.sky-axe.active",
                }
                for effect_id in catalog_ids
            )
        )
        successors = self.receipt["retiredOwnerSuccessorMappings"]
        self.assertEqual(105, len(successors))
        self.assertEqual(
            105, len({row["retiredBindingId"] for row in successors})
        )
        cue_ids = {row["bindingId"] for row in self.cues["cues"]}
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
        _, _, blockers = materializer._build_projections(
            materializer._load_inventory()
        )
        sky_path = materializer.AUTHORED_ROOT / "effect.valtan.sky-axe.active.effect.json"
        sky = json.loads(sky_path.read_text(encoding="utf-8"))
        materializer._validate_existing_receipt(
            self.receipt,
            self.documents,
            self.protected,
            sky,
            blockers,
            self.catalog,
            self.cues,
        )
        unrelated_catalog = copy.deepcopy(self.catalog)
        unrelated_catalog["effects"].append(
            {
                "effectAssetId": "effect.unrelated.concurrent-session",
                "authoringPath": (
                    "Effects/Authored/"
                    "effect.unrelated.concurrent-session.effect.json"
                ),
            }
        )
        materializer._validate_existing_receipt(
            self.receipt,
            self.documents,
            self.protected,
            sky,
            blockers,
            unrelated_catalog,
            self.cues,
        )
        drifted_catalog = copy.deepcopy(self.catalog)
        next(
            row
            for row in drifted_catalog["effects"]
            if row["effectAssetId"] == materializer.PROTECTED_EFFECT_ID
        )["authoringPath"] += ".drift"
        with self.assertRaises(materializer.MaterializeError):
            materializer._validate_existing_receipt(
                self.receipt,
                self.documents,
                self.protected,
                sky,
                blockers,
                drifted_catalog,
                self.cues,
            )
        drifted = copy.deepcopy(blockers)
        drifted["reviewedProjectionLedger"][0]["conversionStatus"] += ".drift"
        with self.assertRaises(materializer.MaterializeError):
            materializer._validate_existing_receipt(
                self.receipt,
                self.documents,
                self.protected,
                sky,
                drifted,
                self.catalog,
                self.cues,
            )

    def test_temp_transaction_second_build_is_applied_and_idempotent(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            temp_root = Path(temporary).resolve()
            staged: dict[Path, bytes] = {}
            for path, payload in self.outputs.items():
                relative = path.resolve().relative_to(materializer.ROOT.resolve())
                staged[temp_root / relative] = payload

            required = [
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
                LEGACY_INVENTORY_PATH=temp_legacy,
            ):
                state, writes, _ = materializer.build_outputs()
                self.assertEqual("APPLIED", state)
                self.assertEqual({}, materializer._changed_outputs(writes))
                self.assertEqual(0, materializer.main(["--mode", "check"]))
            legacy = materializer.legacy_inventory_builder
            with mock.patch.multiple(
                legacy,
                ROOT=temp_root,
                OUTPUT_PATH=temp_legacy,
                MATERIALIZATION_RECEIPT_PATH=temp_receipt,
                EFFECT_CATALOG_PATH=temp_catalog,
                CUE_PATH=temp_cues,
            ):
                self.assertTrue(legacy.sealed_historical_preimage_is_applied())
                self.assertEqual(0, legacy.main(["--check"]))

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
