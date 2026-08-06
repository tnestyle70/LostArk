from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

import build_bern_castle_foliage_supplement as supplement


def foliage_item(item_id: str, export_index: int, path: str) -> dict:
    return {
        "id": item_id,
        "type": "foliage",
        "source": {
            "level": "LV_BER_BERNCASTLE_T_SL00",
            "component": {
                "exportIndex": export_index,
                "objectPath": f"theworld.persistentlevel.component_{export_index}",
            },
        },
        "references": [
            {
                "property": "staticmesh",
                "role": "staticMesh",
                "index": -7,
                "class": "StaticMesh",
                "objectPath": path,
                "runtimeAvailability": "missing",
            }
        ],
        "missingReferences": [
            {"kind": "missingRuntimeStaticMesh", "objectPath": path}
        ],
    }


class FoliageSupplementTests(unittest.TestCase):
    def test_inventory_uses_exact_manifest_reference_and_deduplicates(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path = "pkg.mesh.tree_sm"
            nonstatic = root / "nonstatic.json"
            base = root / "base.json"
            nonstatic.write_text(
                json.dumps(
                    {
                        "schemaVersion": 1,
                        "areaId": "LV_BER_BERNCASTLE",
                        "items": [
                            foliage_item("item-a", 10, path),
                            foliage_item("item-b", 11, path),
                        ],
                    }
                ),
                encoding="utf-8",
            )
            base.write_text(
                json.dumps({"assetCount": 0, "assets": []}), encoding="utf-8"
            )
            result = supplement.build_inventory(nonstatic, base, 1, 2)
            self.assertEqual(result["assetCount"], 1)
            self.assertEqual(result["usageCount"], 2)
            self.assertEqual(result["assets"][0]["logicalPackage"], "PKG")
            self.assertEqual(result["assets"][0]["objectGroup"], "mesh")
            self.assertEqual(result["assets"][0]["objectName"], "tree_sm")

    def test_base_manifest_overlap_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path = "pkg.mesh.tree_sm"
            item = foliage_item("item-a", 10, path)
            nonstatic = root / "nonstatic.json"
            base = root / "base.json"
            nonstatic.write_text(
                json.dumps(
                    {"schemaVersion": 1, "areaId": "LV_BER_BERNCASTLE", "items": [item]}
                ),
                encoding="utf-8",
            )
            base.write_text(
                json.dumps(
                    {
                        "assetCount": 1,
                        "assets": [{"assetId": "old", "fullPath": path}],
                    }
                ),
                encoding="utf-8",
            )
            with self.assertRaisesRegex(supplement.SupplementError, "already exists"):
                supplement.build_inventory(nonstatic, base, 1, 1)

    def test_non_static_reference_class_must_be_static_mesh(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            item = foliage_item("item-a", 10, "pkg.mesh.tree_sm")
            item["references"][0]["class"] = "Texture2D"
            nonstatic = root / "nonstatic.json"
            base = root / "base.json"
            nonstatic.write_text(
                json.dumps(
                    {"schemaVersion": 1, "areaId": "LV_BER_BERNCASTLE", "items": [item]}
                ),
                encoding="utf-8",
            )
            base.write_text(json.dumps({"assetCount": 0, "assets": []}), encoding="utf-8")
            with self.assertRaisesRegex(supplement.SupplementError, "not StaticMesh"):
                supplement.build_inventory(nonstatic, base, 1, 1)


if __name__ == "__main__":
    unittest.main()
