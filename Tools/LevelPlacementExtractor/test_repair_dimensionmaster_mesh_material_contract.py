#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import json
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest


MODULE_PATH = Path(__file__).with_name(
    "repair_dimensionmaster_mesh_material_contract.py"
)
SPEC = importlib.util.spec_from_file_location(
    "repair_dimensionmaster_mesh_material_contract", MODULE_PATH
)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


def document(use_model_material: bool = True) -> dict:
    return {
        "schema": "lostark.effect-authoring",
        "version": 12,
        "effectAssetId": "effect.dimensionmaster.skill.10",
        "unrelated": {"mustRemain": [1, 2, 3]},
        "elements": [{
            "id": "mesh",
            "kind": "particle",
            "detail": {"mesh": {
                "useModelMaterial": use_model_material,
            }},
            "sourceRecipe": {
                "rendererShape": "mesh",
                "modules": [{
                    "className": "ParticleModuleTypeDataMesh",
                    "literals": [{
                        "propertyPath": "bOverrideMaterial",
                        "value": True,
                    }],
                }],
            },
        }],
    }


class RepairDimensionMasterMeshMaterialContractTests(unittest.TestCase):
    def test_stage_and_promote_change_only_material_ownership(self) -> None:
        with TemporaryDirectory() as temporary:
            root = Path(temporary)
            source_root = root / "source"
            staged_root = root / "staged"
            source_root.mkdir()
            source_path = source_root / (
                "effect.dimensionmaster.skill.10.effect.json"
            )
            source_path.write_text(json.dumps(document()), encoding="utf-8")
            receipt_path = root / "receipt.json"

            receipt = MODULE.stage_repairs(
                [(10, "effect.dimensionmaster.skill.10")],
                source_root,
                staged_root,
                receipt_path,
            )
            self.assertEqual(1, receipt["correctionCount"])
            staged = json.loads(
                Path(receipt["documents"][0]["stagedPath"])
                .read_text(encoding="utf-8")
            )
            self.assertFalse(
                staged["elements"][0]["detail"]["mesh"]["useModelMaterial"]
            )
            self.assertEqual([1, 2, 3], staged["unrelated"]["mustRemain"])

            MODULE.promote_staged(receipt)
            promoted = json.loads(source_path.read_text(encoding="utf-8"))
            self.assertFalse(
                promoted["elements"][0]["detail"]["mesh"]
                ["useModelMaterial"]
            )
            self.assertEqual([1, 2, 3], promoted["unrelated"]["mustRemain"])

    def test_stale_source_blocks_promotion(self) -> None:
        with TemporaryDirectory() as temporary:
            root = Path(temporary)
            source_root = root / "source"
            staged_root = root / "staged"
            source_root.mkdir()
            source_path = source_root / (
                "effect.dimensionmaster.skill.10.effect.json"
            )
            source_path.write_text(json.dumps(document()), encoding="utf-8")
            receipt = MODULE.stage_repairs(
                [(10, "effect.dimensionmaster.skill.10")],
                source_root,
                staged_root,
                root / "receipt.json",
            )
            changed = document()
            changed["unrelated"]["mustRemain"] = [9]
            source_path.write_text(json.dumps(changed), encoding="utf-8")

            with self.assertRaisesRegex(ValueError, "stale source"):
                MODULE.promote_staged(receipt)
            preserved = json.loads(source_path.read_text(encoding="utf-8"))
            self.assertEqual([9], preserved["unrelated"]["mustRemain"])
            self.assertTrue(
                preserved["elements"][0]["detail"]["mesh"]
                ["useModelMaterial"]
            )


if __name__ == "__main__":
    unittest.main()
