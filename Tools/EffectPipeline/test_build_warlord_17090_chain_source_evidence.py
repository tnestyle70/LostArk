#!/usr/bin/env python3
"""Focused source-side tests for the Warlord 17090 chain boundary."""

from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import sys
import unittest


MODULE_PATH = Path(__file__).with_name(
    "build_warlord_17090_chain_source_evidence.py"
)
SPEC = importlib.util.spec_from_file_location(
    "build_warlord_17090_chain_source_evidence", MODULE_PATH
)
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class Warlord17090ChainSourceEvidenceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        root = MODULE.REPO_ROOT
        cls.imported = MODULE.read_json(
            root / "Data/Effects/Imported/Warlord/CurrentCombat/Converted/"
            "effect.warlord.skill.17090.imported.effect.json"
        )
        cls.authored = MODULE.read_json(
            root / "Data/Effects/Authored/"
            "effect.warlord.skill.17090.unified.effect.json"
        )
        cls.material_evidence = MODULE.read_json(
            root / "Data/Effects/Imported/FourClassCombat/"
            "FourClassCombat.source-material-evidence.json"
        )
        cls.raw_inventory = MODULE.read_json(
            root / "Data/Effects/Imported/RawResourceInventory/"
            "R8.raw-resource-inventory-v1.json"
        )

    def test_exact_twelve_chain_rows_preserve_dynamic_parameter_evidence(self) -> None:
        rows = MODULE.collect_chain_rows(self.imported, self.authored)
        self.assertEqual(12, len(rows))
        self.assertEqual(
            {"fm_d_berchain_06.wmodel": 8, "fm_d_berchain_07.wmodel": 4},
            {
                name: sum(Path(row["meshAssetId"]).name == name for row in rows)
                for name in MODULE.CHAIN_MODELS
            },
        )
        self.assertTrue(all(row["modelPreScale"] == 0.01 for row in rows))
        self.assertTrue(all(
            tuple(item["name"] for item in row["dynamicParameters"])
            == MODULE.DYNAMIC_PARAMETER_NAMES
            for row in rows
        ))
        self.assertTrue(all(
            row["dynamicParameters"][0]["maximum"] == 50.0
            for row in rows
        ))

    def test_parent_reference_is_not_an_exact_base_lane(self) -> None:
        boundary = MODULE.collect_material_boundary(self.material_evidence)
        self.assertEqual("BLEND_Masked", boundary["renderState"]["blendMode"])
        self.assertTrue(boundary["renderState"]["twoSided"])
        self.assertEqual(0, boundary["collectedTextureParameterCount"])
        self.assertEqual(9, len(boundary["referencedTextureObjectNames"]))
        self.assertIn("fx_d_grid_016", boundary["referencedTextureObjectNames"])
        self.assertEqual(
            MODULE.TYPED_REASON,
            "SOURCE_MASKED_WPO_ARITHMETIC_UNAVAILABLE",
        )

    def test_raw_inventory_owns_both_exact_chain_meshes(self) -> None:
        rows = [
            MODULE.find_raw_mesh_row(self.raw_inventory, name)
            for name in MODULE.CHAIN_MODELS
        ]
        self.assertEqual(
            {"fx_sm_01.fm_d_berchain_06", "fx_sm_01.fm_d_berchain_07"},
            {row["sourceAssetPath"] for row in rows},
        )
        self.assertTrue(all(
            sum(payload.get("kind") == "WMODEL" for payload in row["payloads"])
            == 1
            for row in rows
        ))

    def test_written_receipt_keeps_gpu_witness_unmet(self) -> None:
        receipt = MODULE.read_json(
            MODULE.REPO_ROOT / "Data/Effects/Imported/Warlord/CurrentCombat/"
            "skill.17090.chain-source-evidence.json"
        )
        self.assertEqual(MODULE.TYPED_REASON, receipt["classification"]["typedReason"])
        self.assertFalse(receipt["classification"]["exactRuntimeEligible"])
        self.assertFalse(
            receipt["erroneousAutoBinding"]["isExactBaseSemantic"]
        )
        self.assertEqual(
            "NOT_RUN_NO_TYPED_EVALUATOR",
            receipt["requiredGpuWitness"]["status"],
        )
        self.assertEqual(12, receipt["requiredGpuWitness"]["rowCount"])
        self.assertEqual(12, len(receipt["chainRows"]))
        self.assertEqual(9, len(receipt["textures"]))
        self.assertTrue(all(
            row["sourceExportByteEquivalent"] for row in receipt["textures"]
        ))
        self.assertEqual(
            "COOKED_PARTIAL",
            receipt["material"]["cookedGraph"]["summary"]["topologyStatus"],
        )
        self.assertEqual(
            0,
            receipt["material"]["cookedGraph"]["outputs"]
            ["opacitymask"]["packageIndex"],
        )
        self.assertEqual(
            0,
            receipt["material"]["cookedGraph"]["outputs"]
            ["worldpositionoffset"]["packageIndex"],
        )


if __name__ == "__main__":
    unittest.main()
