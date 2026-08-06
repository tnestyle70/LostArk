#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("cook_effect_runtime_resources.py")
SPEC = importlib.util.spec_from_file_location("cook_effect_runtime_resources", MODULE_PATH)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class CookEffectRuntimeResourcesTests(unittest.TestCase):
    def test_select_export_prefers_dds_over_tga(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "asset.tga").write_bytes(b"tga")
            (root / "asset.dds").write_bytes(b"dds")
            row = {
                "outputs": [
                    {"relativePath": "asset.tga"},
                    {"relativePath": "asset.dds"},
                ]
            }
            selected = MODULE.select_export(row, root, (".dds", ".tga"))
            self.assertEqual(root / "asset.dds", selected)

    def test_duplicate_runtime_name_is_rejected(self) -> None:
        assets = [
            {
                "sourceAssetPath": "package_a.same",
                "objectName": "same",
                "roles": ["texture"],
            },
            {
                "sourceAssetPath": "package_b.same",
                "objectName": "same",
                "roles": ["texture"],
            },
        ]
        with self.assertRaisesRegex(ValueError, "runtime name collision"):
            MODULE.validate_unique_names(assets)

    def test_package_scoped_duplicate_runtime_name_is_allowed(self) -> None:
        assets = [
            {
                "sourceAssetPath": "package_a.same",
                "logicalPackage": "package_a",
                "objectName": "same",
                "roles": ["mesh"],
            },
            {
                "sourceAssetPath": "package_b.same",
                "logicalPackage": "package_b",
                "objectName": "same",
                "roles": ["mesh"],
            },
        ]
        MODULE.validate_unique_names(assets, {"mesh"})

    def test_tga_conversion_writes_dds(self) -> None:
        from PIL import Image

        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            source = root / "source.tga"
            destination = root / "destination.dds"
            Image.new("RGBA", (2, 2), (1, 2, 3, 255)).save(source)
            MODULE.convert_tga_to_dds(source, destination)
            with Image.open(destination) as image:
                self.assertEqual((2, 2), image.size)


if __name__ == "__main__":
    unittest.main()
