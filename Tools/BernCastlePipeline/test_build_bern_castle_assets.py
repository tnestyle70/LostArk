from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

import build_bern_castle_assets as pipeline


class BernCastlePipelineTests(unittest.TestCase):
    def test_asset_id_uses_existing_sha1_contract(self):
        self.assertEqual(
            pipeline.stable_asset_id(
                "lv_navimesh.mesh.lv_common_mesh_cul_box_8",
                "lv_common_mesh_cul_box_8",
            ),
            "MAP_1E5F55FC0524_LV_COMMON_MESH_CUL_BOX_8",
        )

    def test_inventory_deduplicates_assets_and_filters_level_prefix(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            document = {
                "schemaVersion": 1,
                "propertyErrors": [],
                "placements": [
                    {
                        "placementId": "main:1",
                        "levelPackage": "LV_BER_BERNCASTLE_T_SL00",
                        "asset": {
                            "objectPath": "bg_test_a.mesh.bg_test_box_sm",
                            "objectName": "bg_test_box_sm",
                        },
                    },
                    {
                        "placementId": "main:2",
                        "levelPackage": "LV_BER_BERNCASTLE_T_SL00",
                        "asset": {
                            "objectPath": "BG_TEST_A.MESH.BG_TEST_BOX_SM",
                            "objectName": "BG_TEST_BOX_SM",
                        },
                    },
                    {
                        "placementId": "fav:1",
                        "levelPackage": "LV_BER_BERNCASTLE_FAV_PS",
                        "asset": {
                            "objectPath": "bg_fav.mesh.bg_fav_sm",
                            "objectName": "bg_fav_sm",
                        },
                    },
                ],
            }
            path = root / "source.placements.json"
            path.write_text(json.dumps(document), encoding="utf-8")
            result = pipeline.inventory_assets(
                [root], "LV_BER_BERNCASTLE_T_", 1, 2
            )
            self.assertEqual(result["assetCount"], 1)
            self.assertEqual(result["placementCount"], 2)
            self.assertEqual(
                result["assets"][0]["fullPath"],
                "bg_test_a.mesh.bg_test_box_sm",
            )

    def test_parse_material_props_reads_texture_parameter(self):
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "sample.props.txt"
            path.write_text(
                "Parent = MaterialInstanceConstant'base.parent_mi'\n"
                "TextureParameterValues[1] =\n{\n"
                " ParameterValue = Texture2D'tex.sample_d'\n"
                " ParameterName = texture_diffuse\n}\n",
                encoding="utf-8",
            )
            parent, values = pipeline.parse_material_props(path)
            self.assertEqual(parent, "base.parent_mi")
            self.assertEqual(values["texture_diffuse"], "sample_d")


if __name__ == "__main__":
    unittest.main()
