from __future__ import annotations

import copy
import json
from pathlib import Path
import sys
import unittest


TOOLS = Path(__file__).resolve().parent
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import build_valtan_front_back_front_family_inventory as subject


class ValtanFrontBackFrontFamilyInventoryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.inventory = subject.build_inventory()

    def test_exact_source_and_product_denominators_are_sealed(self) -> None:
        summary = self.inventory["summary"]
        self.assertEqual(summary["carrierCount"], 20)
        self.assertEqual(summary["spriteCarrierCount"], 15)
        self.assertEqual(summary["meshCarrierCount"], 3)
        self.assertEqual(summary["localDecalCarrierCount"], 2)
        self.assertEqual(summary["sourceWaveElementCount"], 100)
        self.assertEqual(summary["existingSourceWaveCueCount"], 4)
        self.assertEqual(summary["unprojectedImpactCueCount"], 4)
        self.assertEqual(summary["currentAdmittedFamilyCarrierCount"], 0)
        self.assertEqual(summary["blockedFamilyCarrierCount"], 20)
        self.assertEqual(summary["externalModuleReferenceCount"], 149)
        self.assertEqual(summary["externalModulePackageCount"], 31)
        self.assertEqual(summary["carrierWithExternalModuleDependencyCount"], 20)

        source_system = self.inventory["sourceSystem"]
        self.assertEqual(
            source_system["carrierShapeCounts"],
            {"sprite": 15, "mesh": 3, "decal": 2},
        )
        self.assertEqual(
            [row["sourceOrder"] for row in source_system["carriers"]],
            list(range(20)),
        )

    def test_family_witnesses_do_not_claim_product_admission(self) -> None:
        rows = {
            row["childMaterialPath"]: row
            for row in self.inventory["sourceSystem"]["carriers"]
        }
        spritewave = rows["fx_m_mi_m_00.fx_mi.fx_m_pa_spritewave_01_7_tr"]
        self.assertEqual(
            spritewave["familyCandidate"]["family"], "SPRITEWAVE_01_V2"
        )
        self.assertEqual(
            spritewave["familyCandidate"]["disposition"],
            "UV_PHASE_V2_EVALUATOR_REQUIRED",
        )
        self.assertIn(
            spritewave["familyCandidate"]["matchBasis"],
            {
                "EXACT_PARENT_MATERIAL",
                "PACKAGE_QUALIFIED_PARENT_EQUIVALENCE_REVIEW",
            },
        )
        self.assertEqual(
            spritewave["productAdmission"],
            "BLOCKED_PENDING_SOURCE_AND_FAMILY_CLOSURE",
        )

        makeflow = rows["fx_m_mi_k_00.fx_mi.fx_k_me_makeflow_03_27_tr"]
        self.assertEqual(makeflow["rendererShape"], "mesh")
        self.assertEqual(makeflow["meshObjectPath"], "fx_sm_00.fm_d_plane_003")
        self.assertEqual(makeflow["familyCandidate"]["family"], "MAKEFLOW_03_V2")

        watertrail = rows[
            "fx_m_mi_m_00.fx_mi.fx_m_me_watertrail_01_46_tr"
        ]
        self.assertEqual(watertrail["familyCandidate"]["family"], "WATERTRAIL")
        self.assertEqual(
            watertrail["meshObjectPath"], "fx_sm_01.fm_m_sphere_006"
        )

    def test_local_decal_variants_are_split_and_fail_closed(self) -> None:
        rows = {
            row["childMaterialPath"]: row
            for row in self.inventory["sourceSystem"]["carriers"]
        }
        unresolved = rows["fx_m_mi_04.fx_mi.fx_d_de_unlit_01_02_tr"]
        self.assertEqual(unresolved["rendererShape"], "decal")
        self.assertIsNone(unresolved["parentMaterialPath"])
        self.assertEqual(
            unresolved["familyCandidate"]["disposition"],
            "FAIL_CLOSE_PARENT_MATERIAL_UNDECLARED",
        )
        self.assertIn(
            "PARENT_MATERIAL_UNDECLARED",
            unresolved["materialEvidence"]["blockers"],
        )

        ground = rows["fx_m_mi_n_00.fx_mi.fx_n_de_ground_04_30_tr"]
        self.assertEqual(ground["rendererShape"], "decal")
        self.assertEqual(ground["familyCandidate"]["family"], "GROUND_DECAL_04")
        self.assertEqual(
            ground["familyCandidate"]["disposition"],
            "NEW_TYPED_LOCAL_DECAL_EVALUATOR_REQUIRED",
        )
        self.assertEqual(ground["materialEvidence"]["blockers"], [])

    def test_missing_portable_modules_remain_explicit(self) -> None:
        carriers = self.inventory["sourceSystem"]["carriers"]
        self.assertTrue(
            all(
                row["currentSourceAdapterDisposition"]
                == "UNRESOLVED_RUNTIME_ADAPTER"
                for row in carriers
            )
        )
        self.assertTrue(
            any(
                "particlemodulelifetime"
                in row["missingCanonicalPortableModules"]
                for row in carriers
            )
        )
        self.assertTrue(
            any(
                "particlemodulespawn" in row["missingCanonicalPortableModules"]
                for row in carriers
            )
        )
        decal_rows = [row for row in carriers if row["rendererShape"] == "decal"]
        self.assertEqual(len(decal_rows), 2)
        self.assertTrue(
            all(
                any(
                    "efparticlemoduletypedatadecal" in blocker
                    for blocker in row["currentSourceAdapterBlockers"]
                )
                for row in decal_rows
            )
        )

    def test_external_module_closure_is_exact_and_never_synthesized(self) -> None:
        source_system = self.inventory["sourceSystem"]
        closure = source_system["externalModuleClosure"]
        self.assertEqual(
            closure["status"], "BLOCKED_EXTERNAL_GRAPH_PACKAGES_NOT_LOADED"
        )
        self.assertEqual(closure["referenceCount"], 149)
        self.assertEqual(closure["packageCount"], 31)
        self.assertEqual(closure["syntheticDefaultPolicy"], "FORBIDDEN")

        rows = {
            row["childMaterialPath"]: row
            for row in source_system["carriers"]
        }
        spritewave = rows["fx_m_mi_m_00.fx_mi.fx_m_pa_spritewave_01_7_tr"]
        self.assertEqual(spritewave["externalModuleClosure"]["referenceCount"], 7)
        self.assertIn(
            "bfx_high_00", spritewave["externalModuleClosure"]["packageCounts"]
        )
        self.assertEqual(
            {
                dependency["property"]
                for dependency in spritewave["externalModuleClosure"]["dependencies"]
            },
            {"modules", "spawnmodule"},
        )
        self.assertTrue(
            any(
                dependency["moduleClassCandidate"] == "particlemodulelifetime"
                for dependency in spritewave["externalModuleClosure"]["dependencies"]
            )
        )

        ground = rows["fx_m_mi_n_00.fx_mi.fx_n_de_ground_04_30_tr"]
        self.assertEqual(ground["externalModuleClosure"]["referenceCount"], 2)
        self.assertEqual(
            {
                dependency["moduleClassCandidate"]
                for dependency in ground["externalModuleClosure"]["dependencies"]
            },
            {"particlemodulesize", "particlemodulespawn"},
        )
        self.assertTrue(
            all(
                row["externalModuleClosure"]["referenceCount"] > 0
                for row in source_system["carriers"]
            )
        )

    def test_timing_keeps_one_clip_and_four_plus_four_occurrences(self) -> None:
        timing = self.inventory["timing"]
        self.assertEqual(
            [row["sourceStartMs"] for row in timing["existingSourceWaveCues"]],
            [1169, 2253, 3224, 4220],
        )
        self.assertEqual(len(timing["unprojectedImpactOccurrences"]), 4)
        self.assertEqual(
            {
                round(row["sourceTimeSeconds"], 6)
                for row in timing["unprojectedImpactOccurrences"]
            },
            {1.174037, 2.225867, 3.214799, 4.206632},
        )
        self.assertEqual(timing["futureOrderedCueCount"], 8)
        self.assertEqual(
            timing["fourthBeatGameplayDisposition"],
            "AUXILIARY_PRESENTATION_ONLY",
        )

    def test_duplicate_carrier_material_binding_is_rejected(self) -> None:
        catalog = subject.load_json(subject.SOURCE_RESOURCE_CATALOG)
        mutated = copy.deepcopy(catalog)
        system = next(
            row
            for row in mutated["sourceSystems"]
            if row["sourceAsset"].casefold() == subject.SOURCE_SYSTEM_ID
        )
        material_binding = next(
            row
            for row in system["graph"]["resourceBindings"]
            if row.get("role") == "material"
        )
        duplicate = copy.deepcopy(material_binding)
        duplicate["objectPath"] = "fx_fake.fx_mi.conflicting_material"
        system["graph"]["resourceBindings"].append(duplicate)
        with self.assertRaisesRegex(subject.InventoryError, "not singular"):
            subject.build_inventory(source_catalog=mutated)

    def test_missing_material_evidence_is_rejected(self) -> None:
        evidence = subject.load_json(subject.SOURCE_MATERIAL_EVIDENCE)
        mutated = copy.deepcopy(evidence)
        child = "fx_m_mi_m_00.fx_mi.fx_m_pa_spritewave_01_7_tr"
        mutated["materials"] = [
            row
            for row in mutated["materials"]
            if row["sourceMaterialPath"].casefold() != child
        ]
        with self.assertRaisesRegex(subject.InventoryError, "no material evidence"):
            subject.build_inventory(material_evidence=mutated)

    def test_output_is_deterministic_and_json_serializable(self) -> None:
        rebuilt = subject.build_inventory()
        self.assertEqual(
            subject.canonical_bytes(self.inventory),
            subject.canonical_bytes(rebuilt),
        )
        payload = subject.pretty_bytes(rebuilt)
        self.assertEqual(json.loads(payload), rebuilt)


if __name__ == "__main__":
    unittest.main()
