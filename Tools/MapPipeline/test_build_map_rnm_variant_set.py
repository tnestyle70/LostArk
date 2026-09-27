from pathlib import Path
import copy
import tempfile
import unittest

import build_map_rnm_variant_set as builder


def catalog_row(asset_id, model=None):
    return [asset_id, asset_id.lower(),
            model or ("Map/AREA/" + asset_id + "/" + asset_id + ".wmodel"),
            "Prototype_Component_Model_" + asset_id, "1", "1", "1", "Origin", "staticmesh",
            "pkg", "UE3 ImportTable exact: pkg.mesh." + asset_id.lower(), "Opaque", "Back",
            "1", "1", "0", "0", "1", "1", "1", "50", "1", "1", "1", "1", "1"]


def placement_row(source_id, asset_id):
    return ["1", source_id, "AREA_SL01", "actor", asset_id,
            "0", "0", "0", "0", "0", "0", "1", "1", "1", "1", "1"]


def material_row(asset_id, slot="SLOT_000_mi"):
    return dict(assetId=asset_id, materialName=slot, sourceMaterial="pkg.mat." + slot,
                family="bg-source-opaque-masked", diffuseTexture="Map/AREA/d.dds")


def rnm_row(source_id, asset_id, average, directional, scale=None, bias=None):
    return dict(sourcePlacementId=source_id, assetId=asset_id, average=average,
                directional=directional,
                coordinateScale=scale or [0.5, 0.5], coordinateBias=bias or [0.25, 0.25],
                averageScale=[1.0, 1.0, 1.0], directionalScale=[2.0, 2.0, 2.0])


class RnmVariantSetTests(unittest.TestCase):
    def build(self, catalog, placements, materials, rnm, absent_ids=frozenset()):
        built = builder.build_variant_set(
            catalog_rows=catalog, placement_rows=placements, materials=materials,
            rnm=rnm, lightmap_directory="Map/Lighting/AREA")
        problems = builder.validate_set(
            catalog_rows=catalog, catalog_additions=built["catalogAdditions"],
            placement_rows=built["placements"], materials=built["materials"],
            placement_lighting=built["placementLighting"],
            baked_assets=built["bakedAssets"], source_absent_ids=set(absent_ids),
            resources_root=None)
        return built, problems

    def test_distinct_ovr_assets_keep_their_own_atlas_pair(self):
        """Two assets that differ only by OVR suffix must not share a pair."""
        assets = ["MESH_A_OVR_1111", "MESH_A_OVR_2222"]
        catalog = [catalog_row(a) for a in assets]
        placements = [placement_row("s1", assets[0]), placement_row("s2", assets[1])]
        materials = [material_row(a) for a in assets]
        rnm = [rnm_row("s1", assets[0], "P.avg_1", "P.dir_1"),
               rnm_row("s2", assets[1], "P.avg_2", "P.dir_2")]
        built, problems = self.build(catalog, placements, materials, rnm)
        self.assertEqual(problems, [])
        self.assertEqual(built["catalogAdditions"], [])
        self.assertEqual(built["repointed"], {})
        pairs = {row["assetId"]: row["bakedLighting"]["averageTexture"]
                 for row in built["materials"]}
        self.assertEqual(pairs[assets[0]], "Map/Lighting/AREA/p_avg_1.dds")
        self.assertEqual(pairs[assets[1]], "Map/Lighting/AREA/p_avg_2.dds")

    def test_multiple_atlas_pairs_produce_one_variant_each(self):
        """The base id keeps the most common pair and every other pair becomes a variant."""
        asset = "MESH_B_OVR_3333"
        catalog = [catalog_row(asset)]
        placements = [placement_row("s%d" % n, asset) for n in range(1, 5)]
        materials = [material_row(asset)]
        rnm = [rnm_row("s1", asset, "P.avg_1", "P.dir_1"),
               rnm_row("s2", asset, "P.avg_1", "P.dir_1"),
               rnm_row("s3", asset, "P.avg_2", "P.dir_2"),
               rnm_row("s4", asset, "P.avg_3", "P.dir_3")]
        built, problems = self.build(catalog, placements, materials, rnm)
        self.assertEqual(problems, [])
        self.assertEqual(len(built["catalogAdditions"]), 2)
        self.assertEqual(len(built["repointed"]), 2)
        self.assertNotIn("s1", built["repointed"])
        self.assertNotIn("s2", built["repointed"])
        for row in built["catalogAdditions"]:
            self.assertEqual(row[2], catalog[0][2])
            self.assertEqual(row[3], "Prototype_Component_Model_" + row[0])
        base_rows = [r for r in built["materials"] if r["assetId"] == asset]
        self.assertEqual(len(base_rows), 1)
        self.assertEqual(base_rows[0]["bakedLighting"]["averageTexture"],
                         "Map/Lighting/AREA/p_avg_1.dds")

    def test_variant_ids_are_stable_and_pair_specific(self):
        first = builder.variant_asset_id("MESH", "P.avg_1", "P.dir_1")
        self.assertEqual(first, builder.variant_asset_id("MESH", "P.avg_1", "P.dir_1"))
        self.assertNotEqual(first, builder.variant_asset_id("MESH", "P.avg_2", "P.dir_1"))
        self.assertNotEqual(first, builder.variant_asset_id("OTHER", "P.avg_1", "P.dir_1"))
        self.assertTrue(first.startswith("MESH_LM_"))

    def test_every_slot_of_a_multi_slot_asset_is_carried_into_the_variant(self):
        asset = "MESH_C_OVR_4444"
        catalog = [catalog_row(asset)]
        placements = [placement_row("s1", asset), placement_row("s2", asset)]
        materials = [material_row(asset, "SLOT_000_a"), material_row(asset, "SLOT_001_b")]
        rnm = [rnm_row("s1", asset, "P.avg_1", "P.dir_1"),
               rnm_row("s2", asset, "P.avg_2", "P.dir_2")]
        built, problems = self.build(catalog, placements, materials, rnm)
        self.assertEqual(problems, [])
        variant = built["catalogAdditions"][0][0]
        slots = sorted(r["materialName"] for r in built["materials"] if r["assetId"] == variant)
        self.assertEqual(slots, ["SLOT_000_a", "SLOT_001_b"])
        self.assertEqual(len(built["materials"]), 4)
        for row in built["materials"]:
            self.assertIn("bakedLighting", row)

    def test_placement_without_a_component_record_gets_no_lighting(self):
        """A placement absent from the RNM input is left alone rather than guessed at."""
        asset = "MESH_D_OVR_5555"
        catalog = [catalog_row(asset)]
        placements = [placement_row("s1", asset), placement_row("s2", asset)]
        materials = [material_row(asset)]
        rnm = [rnm_row("s1", asset, "P.avg_1", "P.dir_1")]
        built, problems = self.build(catalog, placements, materials, rnm)
        self.assertEqual(problems, [])
        self.assertEqual([i["sourcePlacementId"] for i in built["placementLighting"]], ["s1"])
        self.assertEqual(built["placements"][1][4], asset)

    def test_source_absent_placement_is_never_given_a_pair(self):
        asset = "MESH_E_OVR_6666"
        catalog = [catalog_row(asset)]
        placements = [placement_row("s1", asset), placement_row("s2", asset)]
        materials = [material_row(asset)]
        rnm = [rnm_row("s1", asset, "P.avg_1", "P.dir_1")]
        built, problems = self.build(catalog, placements, materials, rnm, absent_ids={"s2"})
        self.assertEqual(problems, [])
        self.assertNotIn("s2", [i["sourcePlacementId"] for i in built["placementLighting"]])

    def test_validator_rejects_rnm_on_a_source_absent_placement(self):
        asset = "MESH_F_OVR_7777"
        catalog = [catalog_row(asset)]
        placements = [placement_row("s1", asset)]
        materials = [material_row(asset)]
        rnm = [rnm_row("s1", asset, "P.avg_1", "P.dir_1")]
        _, problems = self.build(catalog, placements, materials, rnm, absent_ids={"s1"})
        self.assertTrue(any("no source lighting" in p for p in problems), problems)

    def test_validator_rejects_a_duplicate_source_placement_id(self):
        asset = "MESH_G_OVR_8888"
        catalog = [catalog_row(asset)]
        placements = [placement_row("s1", asset)]
        materials = [material_row(asset)]
        rnm = [rnm_row("s1", asset, "P.avg_1", "P.dir_1"),
               rnm_row("s1", asset, "P.avg_1", "P.dir_1")]
        _, problems = self.build(catalog, placements, materials, rnm)
        self.assertTrue(any("duplicate placementLighting source" in p for p in problems), problems)

    def test_validator_rejects_a_bad_lightmap_path(self):
        asset = "MESH_H_OVR_9999"
        catalog = [catalog_row(asset)]
        placements = [placement_row("s1", asset)]
        materials = [material_row(asset)]
        rnm = [rnm_row("s1", asset, "P.avg_1", "P.dir_1")]
        built = builder.build_variant_set(catalog_rows=catalog, placement_rows=placements,
                                          materials=materials, rnm=rnm,
                                          lightmap_directory="Map/Lighting/AREA")
        for bad in ("../escape/x.dds", "C:/abs/x.dds", "Map/Lighting/AREA/x.png"):
            broken = copy.deepcopy(built["materials"])
            broken[0]["bakedLighting"]["averageTexture"] = bad
            problems = builder.validate_set(
                catalog_rows=catalog, catalog_additions=built["catalogAdditions"],
                placement_rows=built["placements"], materials=broken,
                placement_lighting=built["placementLighting"],
                baked_assets=built["bakedAssets"], source_absent_ids=set(),
                resources_root=None)
            self.assertTrue(any("Resources-relative DDS" in p for p in problems),
                            (bad, problems))

    def test_validator_reports_a_missing_lightmap_file(self):
        asset = "MESH_I_OVR_AAAA"
        catalog = [catalog_row(asset)]
        placements = [placement_row("s1", asset)]
        materials = [material_row(asset)]
        rnm = [rnm_row("s1", asset, "P.avg_1", "P.dir_1")]
        built = builder.build_variant_set(catalog_rows=catalog, placement_rows=placements,
                                          materials=materials, rnm=rnm,
                                          lightmap_directory="Map/Lighting/AREA")
        with tempfile.TemporaryDirectory() as directory:
            problems = builder.validate_set(
                catalog_rows=catalog, catalog_additions=built["catalogAdditions"],
                placement_rows=built["placements"], materials=built["materials"],
                placement_lighting=built["placementLighting"],
                baked_assets=built["bakedAssets"], source_absent_ids=set(),
                resources_root=Path(directory))
            self.assertTrue(any("lightmap is missing" in p for p in problems), problems)

    def test_validator_rejects_out_of_atlas_coordinates(self):
        asset = "MESH_J_OVR_BBBB"
        catalog = [catalog_row(asset)]
        placements = [placement_row("s1", asset)]
        materials = [material_row(asset)]
        rnm = [rnm_row("s1", asset, "P.avg_1", "P.dir_1", scale=[0.9, 0.5], bias=[0.5, 0.1])]
        _, problems = self.build(catalog, placements, materials, rnm)
        self.assertTrue(any("atlas bounds exceeded" in p for p in problems), problems)

    def test_asset_without_a_material_row_is_excluded_and_others_survive(self):
        """A failing asset must not remove or alter the rows of a healthy one."""
        good, orphan = "MESH_K_OVR_CCCC", "MESH_L_OVR_DDDD"
        catalog = [catalog_row(good), catalog_row(orphan)]
        placements = [placement_row("s1", good), placement_row("s2", orphan)]
        materials = [material_row(good)]
        rnm = [rnm_row("s1", good, "P.avg_1", "P.dir_1"),
               rnm_row("s2", orphan, "P.avg_2", "P.dir_2")]
        built, problems = self.build(catalog, placements, materials, rnm)
        self.assertEqual(problems, [])
        self.assertEqual(built["plan"]["excludedNoMaterialRow"], [orphan])
        self.assertEqual([i["sourcePlacementId"] for i in built["placementLighting"]], ["s1"])
        self.assertEqual(len(built["materials"]), 1)
        self.assertEqual(built["materials"][0]["assetId"], good)
        self.assertEqual(built["materials"][0]["diffuseTexture"], "Map/AREA/d.dds")
        self.assertEqual(built["placements"][1][4], orphan)

    def test_variant_base_missing_from_catalog_fails_without_partial_output(self):
        asset = "MESH_M_OVR_EEEE"
        placements = [placement_row("s1", asset), placement_row("s2", asset)]
        materials = [material_row(asset)]
        rnm = [rnm_row("s1", asset, "P.avg_1", "P.dir_1"),
               rnm_row("s2", asset, "P.avg_2", "P.dir_2")]
        with self.assertRaises(builder.VariantSetError):
            builder.build_variant_set(catalog_rows=[], placement_rows=placements,
                                      materials=materials, rnm=rnm,
                                      lightmap_directory="Map/Lighting/AREA")

    def test_input_documents_are_not_mutated(self):
        asset = "MESH_N_OVR_FFFF"
        catalog = [catalog_row(asset)]
        placements = [placement_row("s1", asset), placement_row("s2", asset)]
        materials = [material_row(asset)]
        rnm = [rnm_row("s1", asset, "P.avg_1", "P.dir_1"),
               rnm_row("s2", asset, "P.avg_2", "P.dir_2")]
        before = copy.deepcopy((catalog, placements, materials, rnm))
        builder.build_variant_set(catalog_rows=catalog, placement_rows=placements,
                                  materials=materials, rnm=rnm,
                                  lightmap_directory="Map/Lighting/AREA")
        self.assertEqual(before, (catalog, placements, materials, rnm))


if __name__ == "__main__":
    unittest.main(verbosity=2)
