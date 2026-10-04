"""Publish the new Landscape family through the real atomic Area publisher."""
from __future__ import annotations

import copy
import unittest

import test_map_effect_presentation_contract as map_contract

Fixture = map_contract.Fixture
POWERSHELL = map_contract.POWERSHELL


@unittest.skipUnless(POWERSHELL, "PowerShell is required")
class LandscapeMaterialPublishTests(unittest.TestCase):
    def prepare(self, *, sharded=False):
        fixture = Fixture()
        path = map_contract.MapMaterialPublishContractTests().install_materials(fixture, sharded=sharded)
        document = fixture.read_json(path)
        document["formatVersion"] = 2
        document["materials"] = [{
            "assetId": "FIXTURE", "materialName": "Floor",
            "sourceMaterial": "source.landscape", "family": "bg-source-landscape-opaque",
            "castsShadow": True,
            "sourceLandscape": {
                "grid": [-124, 62, 62, 31],
                "weightmapScaleBias": [1 / 64, 1 / 64, 0.5 / 64, 0.5 / 64],
                "heightmapScaleBias": [1 / 64, 1 / 64, 0, 0],
                "weightmaps": ["Map/reflection.dds"],
                "heightmapTexture": "Map/reflection.dds",
                "layers": [{
                    "layerIndex": 5, "uv": [3.6, -1.2, 0, 0],
                    "diffuse": [1, 1, 1, 0.8], "specular": [1, 1, 1, 1],
                    "factors": [0.25, 0.6, 60, 0], "weight": [0, 1, 1, 0],
                    "diffuseTexture": "Map/reflection.dds", "diffuseColorSpace": "srgb",
                    "normalTexture": "Map/reflection.dds",
                }],
            },
        }]
        fixture.write_json(path, document)
        return fixture, path, document

    def test_publish_keeps_source_bindings_in_single_and_sharded_area(self):
        for sharded in (False, True):
            with self.subTest(sharded=sharded):
                fixture, path, document = self.prepare(sharded=sharded)
                try:
                    result = fixture.publish()
                    self.assertEqual(0, result.returncode, result.stdout)
                    self.assertEqual(document, fixture.read_json(fixture.runtime_map / path.name))
                finally:
                    fixture.close()

    def test_bad_source_bindings_preserve_previously_published_area(self):
        fixture, path, document = self.prepare(sharded=True)
        try:
            result = fixture.publish()
            self.assertEqual(0, result.returncode, result.stdout)
            before = fixture.snapshot_runtime()
            mutations = {
                "missing_weight": lambda row: row["sourceLandscape"].update(weightmaps=["Map/missing.dds"]),
                "escaped_height": lambda row: row["sourceLandscape"].update(heightmapTexture="../outside.dds"),
                "duplicate_layer": lambda row: row["sourceLandscape"]["layers"].append(copy.deepcopy(row["sourceLandscape"]["layers"][0])),
                "bad_channel": lambda row: row["sourceLandscape"]["layers"][0].update(weight=[0, 4, 1, 0]),
                "bad_grid": lambda row: row["sourceLandscape"].update(grid=[0, 0, 63, 31]),
                "non_integer_grid": lambda row: row["sourceLandscape"].update(grid=[0.5, 0, 62, 31]),
                "unknown_input": lambda row: row["sourceLandscape"]["layers"][0].update(mipBias=-10),
                "bad_normal": lambda row: row["sourceLandscape"]["layers"][0].update(factors=[0.25, 17, 60, 0]),
                "color_space_case": lambda row: row["sourceLandscape"]["layers"][0].update(diffuseColorSpace="sRGB"),
                "wrong_render_pass": lambda row: row.update(renderMode="translucent"),
            }
            for name, mutate in mutations.items():
                with self.subTest(name=name):
                    invalid = copy.deepcopy(document)
                    mutate(invalid["materials"][0])
                    fixture.write_json(path, invalid)
                    result = fixture.publish()
                    self.assertNotEqual(0, result.returncode, result.stdout)
                    self.assertEqual(before, fixture.snapshot_runtime())
        finally:
            fixture.close()

    def test_distinct_layers_cannot_alias_the_same_paint_channel(self):
        fixture, path, document = self.prepare()
        try:
            second = copy.deepcopy(document["materials"][0]["sourceLandscape"]["layers"][0])
            second["layerIndex"] = 2
            document["materials"][0]["sourceLandscape"]["layers"].append(second)
            fixture.write_json(path, document)
            before = fixture.snapshot_runtime()
            result = fixture.publish()
            self.assertNotEqual(0, result.returncode, result.stdout)
            self.assertIn("duplicate landscape weight channel", result.stdout)
            self.assertEqual(before, fixture.snapshot_runtime())
        finally:
            fixture.close()

    def test_native_lighting_roundtrip_and_invalid_inputs_preserve_area(self):
        fixture, path, document = self.prepare(sharded=True)
        try:
            document["materials"][0]["bakedLighting"] = {
                "averageTexture": "Map/reflection.dds",
                "directionalTexture": "Map/reflection.dds",
                "colorSpace": "linear",
                "staticShadow": {
                    "texture": "Map/reflection.dds", "lightGuid": "79ad76d6208c814790c50f5f74b7b8a0",
                    "lightChannel": 1, "penumbraWidth": 0.05,
                    "penumbraBasis": "PROJECT_ADAPTER", "shadowExponent": 2,
                },
            }
            document["placementLighting"] = [{
                "sourcePlacementId": "baseline.A", "assetId": "FIXTURE",
                "coordinateScale": [0.953125, 0.953125],
                "coordinateBias": [0.015372984111309052] * 2,
                "averageScale": [1, 1, 1], "directionalScale": [1.5683544, 1.9274721, 1.6980708],
                "shadowCoordinateScale": [0.953125, 0.953125],
                "shadowCoordinateBias": [0.015372984111309052] * 2,
            }]
            fixture.write_json(path, document)
            result = fixture.publish()
            self.assertEqual(0, result.returncode, result.stdout)
            self.assertEqual(document, fixture.read_json(fixture.runtime_map / path.name))
            before = fixture.snapshot_runtime()
            mutations = {
                "missing_rnm": lambda doc: doc["materials"][0]["bakedLighting"].update(averageTexture="Map/missing.dds"),
                "escaped_shadow": lambda doc: doc["materials"][0]["bakedLighting"]["staticShadow"].update(texture="../outside.dds"),
                "unknown_baked_field": lambda doc: doc["materials"][0]["bakedLighting"].update(mipBias=-2),
                "missing_shadow_coordinate": lambda doc: doc["placementLighting"][0].pop("shadowCoordinateBias"),
                "wrong_source": lambda doc: doc["placementLighting"][0].update(sourcePlacementId="absent.source"),
                "outside_atlas": lambda doc: doc["placementLighting"][0].update(coordinateScale=[1, 1]),
            }
            for name, mutate in mutations.items():
                with self.subTest(name=name):
                    invalid = copy.deepcopy(document)
                    mutate(invalid)
                    fixture.write_json(path, invalid)
                    result = fixture.publish()
                    self.assertNotEqual(0, result.returncode, result.stdout)
                    self.assertEqual(before, fixture.snapshot_runtime())
        finally:
            fixture.close()


if __name__ == "__main__":
    unittest.main()
