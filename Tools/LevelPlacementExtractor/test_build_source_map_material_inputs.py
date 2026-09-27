import json
import tempfile
import unittest
from pathlib import Path

from build_source_map_material_inputs import derive_lighting_evidence


class LightingEvidenceDerivationTests(unittest.TestCase):
    """lightingEvidence must be measured, never assumed.

    build_source_map_materials.py requires the value to be source-bound or source-absent and
    guards source-absent against carrying bindings. That guard only means something when the
    value comes from the component lighting receipts instead of a hardcoded default.
    """

    def build(self, assets, components):
        directory = Path(self.enterContext(tempfile.TemporaryDirectory()))
        inventory = directory / "admitted.inventory.json"
        inventory.write_text(
            json.dumps({"areaId": "TEST", "assets": assets}), encoding="utf-8"
        )
        receipt = directory / "PKG.component-lighting.json"
        receipt.write_text(
            json.dumps({"logicalPackage": "PKG", "components": components}),
            encoding="utf-8",
        )
        return derive_lighting_evidence(inventory, [receipt])

    @staticmethod
    def component(mesh, status):
        return {"sourceMesh": {"objectPath": mesh}, "status": status}

    def test_rnm_component_makes_the_asset_source_bound(self):
        evidence, unresolved, summary = self.build(
            [{"assetId": "A", "fullPath": "pkg.mesh.one"}],
            {"p:1": self.component("pkg.mesh.one", "RNM_TEXTURE_LIGHTMAP")},
        )
        self.assertEqual(evidence, {"A": "source-bound"})
        self.assertEqual(unresolved, {})
        self.assertEqual((summary["sourceBound"], summary["sourceAbsent"]), (1, 0))
        self.assertEqual(summary["unresolved"], 0)

    def test_only_no_lod_components_make_the_asset_source_absent(self):
        evidence, unresolved, summary = self.build(
            [{"assetId": "A", "fullPath": "pkg.mesh.one"}],
            {
                "p:1": self.component("pkg.mesh.one", "NO_LOD_LIGHTING_DATA"),
                "p:2": self.component("pkg.mesh.one", "NO_LOD_LIGHTING_DATA"),
            },
        )
        self.assertEqual(evidence, {"A": "source-absent"})
        self.assertEqual(unresolved, {})
        self.assertEqual(summary["sourceAbsent"], 1)

    def test_one_rnm_component_outweighs_no_lod_siblings(self):
        evidence, _, _ = self.build(
            [{"assetId": "A", "fullPath": "pkg.mesh.one"}],
            {
                "p:1": self.component("pkg.mesh.one", "NO_LOD_LIGHTING_DATA"),
                "p:2": self.component("pkg.mesh.one", "RNM_TEXTURE_LIGHTMAP"),
            },
        )
        self.assertEqual(evidence, {"A": "source-bound"})

    def test_undecided_status_is_unresolved_not_source_absent(self):
        evidence, unresolved, summary = self.build(
            [{"assetId": "A", "fullPath": "pkg.mesh.one"}],
            {"p:1": self.component("pkg.mesh.one", "UNSUPPORTED_NATIVE_LAYOUT")},
        )
        self.assertEqual(evidence, {})
        self.assertIn("A", unresolved)
        self.assertIn("undecided", unresolved["A"])
        self.assertEqual(summary["unresolved"], 1)

    def test_asset_without_any_component_is_unresolved(self):
        evidence, unresolved, summary = self.build(
            [{"assetId": "A", "fullPath": "pkg.mesh.missing"}],
            {"p:1": self.component("pkg.mesh.other", "RNM_TEXTURE_LIGHTMAP")},
        )
        self.assertEqual(evidence, {})
        self.assertIn("no component lighting record", unresolved["A"])
        self.assertEqual(summary["unresolved"], 1)

    def test_mesh_join_ignores_case_and_covers_override_variants(self):
        evidence, unresolved, _ = self.build(
            [
                {"assetId": "A_OVR_1", "fullPath": "PKG.Mesh.One"},
                {"assetId": "A_OVR_2", "fullPath": "pkg.mesh.ONE"},
            ],
            {"p:1": self.component("pkg.mesh.one", "RNM_TEXTURE_LIGHTMAP")},
        )
        self.assertEqual(
            evidence, {"A_OVR_1": "source-bound", "A_OVR_2": "source-bound"}
        )
        self.assertEqual(unresolved, {})

    def test_asset_without_full_path_is_not_claimed(self):
        evidence, unresolved, summary = self.build(
            [{"assetId": "A"}],
            {"p:1": self.component("pkg.mesh.one", "RNM_TEXTURE_LIGHTMAP")},
        )
        self.assertEqual(evidence, {})
        self.assertEqual(unresolved, {})
        self.assertEqual(summary["inventoryAssets"], 0)

    def test_summary_records_the_receipts_it_measured(self):
        _, _, summary = self.build(
            [{"assetId": "A", "fullPath": "pkg.mesh.one"}],
            {"p:1": self.component("pkg.mesh.one", "RNM_TEXTURE_LIGHTMAP")},
        )
        self.assertEqual(summary["componentLightingPackages"], ["PKG"])
        self.assertEqual(summary["distinctSourceMeshes"], 1)


if __name__ == "__main__":
    unittest.main()
