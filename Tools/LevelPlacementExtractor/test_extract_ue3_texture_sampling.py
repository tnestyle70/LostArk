#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("extract_ue3_texture_sampling.py")
SPEC = importlib.util.spec_from_file_location(
    "extract_ue3_texture_sampling", MODULE_PATH
)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class ExtractUe3TextureSamplingTests(unittest.TestCase):
    def test_serialized_and_class_default_sampling_are_separate(self) -> None:
        properties = {
            "AddressX": {"value": "TA_Clamp"},
            "sRGB": {"value": False},
        }
        row = MODULE.sampling_row(
            "fx_tex.fx_mask", "source.upk", "abc", properties
        )
        self.assertEqual("clamp", row["addressU"])
        self.assertEqual("wrap", row["addressV"])
        self.assertEqual("linear", row["colorSpace"])
        self.assertEqual("serialized_property", row["addressUEvidence"])
        self.assertEqual("ue3_class_default", row["addressVEvidence"])
        self.assertEqual("serialized_property", row["colorSpaceEvidence"])

    def test_missing_srgb_uses_ue3_texture_class_default(self) -> None:
        row = MODULE.sampling_row(
            "fx_tex.fx_b_atypical_004_cube", "source.upk", "abc", {}
        )
        self.assertEqual("wrap", row["addressU"])
        self.assertEqual("wrap", row["addressV"])
        self.assertEqual("srgb", row["colorSpace"])
        self.assertEqual(
            "ue3_texture_class_default", row["colorSpaceEvidence"]
        )

    def test_skill_filter_consumes_manifest_ownership_not_cube_name(self) -> None:
        manifest = {
            "assets": [{
                "sourceAssetPath": "fx_tex_00.fx_b_atypical_004_cube",
                "physicalPackage": "textures.upk",
                "roles": ["texture"],
                "skillIds": [2050210],
            }, {
                "sourceAssetPath": "fx_tex_00.unrelated",
                "physicalPackage": "textures.upk",
                "roles": ["texture"],
                "skillIds": [2050240],
            }]
        }
        selected = MODULE.selected_texture_assets(manifest, {2050210})
        self.assertEqual(1, len(selected))
        self.assertEqual(
            "fx_tex_00.fx_b_atypical_004_cube",
            selected[0]["sourceAssetPath"],
        )

    def test_unknown_address_mode_fails_closed(self) -> None:
        with self.assertRaises(ValueError):
            MODULE.normalize_address("TA_Mirror")


if __name__ == "__main__":
    unittest.main()
