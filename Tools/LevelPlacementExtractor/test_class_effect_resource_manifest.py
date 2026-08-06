#!/usr/bin/env python3

from __future__ import annotations

import csv
import json
import tempfile
import unittest
from pathlib import Path

from build_class_effect_resource_manifest import build_manifest


class ClassEffectResourceManifestTests(unittest.TestCase):
    def test_merges_direct_and_material_parameter_resources(self) -> None:
        with tempfile.TemporaryDirectory() as raw_root:
            root = Path(raw_root)
            inventory = root / "inventory.csv"
            with inventory.open("w", encoding="utf-8", newline="") as output:
                writer = csv.DictWriter(
                    output,
                    fieldnames=("logical_name", "physical_file", "byte_size"),
                )
                writer.writeheader()
                writer.writerows(
                    [
                        {"logical_name": "FX_SM", "physical_file": "mesh.upk", "byte_size": 1},
                        {"logical_name": "FX_TEX", "physical_file": "tex.upk", "byte_size": 1},
                    ]
                )
            graph = root / "skill.1.normalized-effect-graph.json"
            graph.write_text(
                json.dumps(
                    {
                        "characterClass": "TEST",
                        "skillId": 1,
                        "sourceSystems": [
                            {
                                "sourceAsset": "FX_A.Par_A",
                                "resourceBindings": [
                                    {"role": "mesh", "objectPath": "FX_SM.Mesh_A"}
                                ],
                            }
                        ],
                        "materialParameterBindings": [
                            {
                                "sourceMaterialPath": "FX_M.Mat_A",
                                "resolutionStatus": "RESOLVED_UNIQUE_PATH",
                                "sourcePhysicalPackage": "material.upk",
                                "textures": [
                                    {"name": "Base", "texture": "FX_TEX.Tex_A"}
                                ],
                            }
                        ],
                    }
                ),
                encoding="utf-8",
            )
            result = build_manifest([graph], inventory, "TEST")

        by_path = {row["sourceAssetPath"]: row for row in result["assets"]}
        self.assertEqual("mesh.upk", by_path["FX_SM.Mesh_A"]["physicalPackage"])
        self.assertEqual("material.upk", by_path["FX_M.Mat_A"]["physicalPackage"])
        self.assertEqual("tex.upk", by_path["FX_TEX.Tex_A"]["physicalPackage"])
        self.assertEqual(0, result["summary"]["unresolvedSourcePackageCount"])

    def test_class_mismatch_fails(self) -> None:
        with tempfile.TemporaryDirectory() as raw_root:
            root = Path(raw_root)
            inventory = root / "inventory.csv"
            inventory.write_text("logical_name,physical_file,byte_size\n", encoding="utf-8")
            graph = root / "graph.json"
            graph.write_text(
                json.dumps({"characterClass": "OTHER", "skillId": 1}), encoding="utf-8"
            )
            with self.assertRaisesRegex(ValueError, "class mismatch"):
                build_manifest([graph], inventory, "TEST")


if __name__ == "__main__":
    unittest.main()
