#!/usr/bin/env python3

from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from export_effect_resources import export_requests, matching_outputs


class ExportEffectResourcesTests(unittest.TestCase):
    def test_groups_only_mesh_and_texture_requests(self) -> None:
        result = export_requests(
            {
                "assets": [
                    {
                        "sourceAssetPath": "FX_SM.Group.Mesh_A",
                        "logicalPackage": "FX_SM",
                        "roles": ["mesh"],
                        "skillIds": [1],
                    },
                    {
                        "sourceAssetPath": "FX_TEX.Tex_A",
                        "logicalPackage": "FX_TEX",
                        "roles": ["texture"],
                        "skillIds": [1],
                    },
                    {
                        "sourceAssetPath": "FX_M.Mat_A",
                        "logicalPackage": "FX_M",
                        "roles": ["material"],
                        "skillIds": [1],
                    },
                ]
            }
        )
        self.assertEqual(["FX_SM", "FX_TEX"], list(result))
        self.assertEqual("Mesh_A", result["FX_SM"][0]["objectName"])

    def test_output_matching_is_case_insensitive_and_keeps_sidecars(self) -> None:
        with tempfile.TemporaryDirectory() as raw_root:
            root = Path(raw_root)
            package = root / "Package" / "StaticMesh3"
            package.mkdir(parents=True)
            (package / "Mesh_A.gltf").write_text("{}", encoding="utf-8")
            (package / "mesh_a_0.bin").write_bytes(b"x")
            (package / "Other.gltf").write_text("{}", encoding="utf-8")
            matches = matching_outputs(root, "mesh_a")
        self.assertEqual(["Mesh_A.gltf", "mesh_a_0.bin"], [p.name for p in matches])

    def test_output_matching_can_be_scoped_to_logical_package(self) -> None:
        with tempfile.TemporaryDirectory() as raw_root:
            root = Path(raw_root)
            for package_name in ("FX_SM_00", "FX_SM_01"):
                package = root / package_name / "StaticMesh3"
                package.mkdir(parents=True)
                (package / "same.gltf").write_text("{}", encoding="utf-8")
            matches = matching_outputs(root, "same", "FX_SM_01")
        self.assertEqual(
            ["FX_SM_01/StaticMesh3/same.gltf"],
            [path.relative_to(root).as_posix() for path in matches],
        )


if __name__ == "__main__":
    unittest.main()
