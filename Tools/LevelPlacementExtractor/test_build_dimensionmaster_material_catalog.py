#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import json
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest


MODULE_PATH = Path(__file__).with_name(
    "build_dimensionmaster_material_catalog.py"
)
SPEC = importlib.util.spec_from_file_location(
    "build_dimensionmaster_material_catalog", MODULE_PATH
)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class BuildDimensionMasterMaterialCatalogTests(unittest.TestCase):
    def test_deduplicates_materials_and_excludes_alt_v_by_default(self) -> None:
        with TemporaryDirectory() as temporary:
            root = Path(temporary)
            player_skills = root / "PlayerSkills.json"
            bindings = root / "DimensionMaster.skillbindings.json"
            imported = root / "Imported"
            imported.mkdir()
            player_skills.write_text(json.dumps({"skills": [
                {
                    "characterClass": "DIMENSIONMASTER",
                    "skillId": 10,
                    "effectId": "effect.dimensionmaster.skill.10",
                    "inputSlot": "Q",
                },
                {
                    "characterClass": "DIMENSIONMASTER",
                    "skillId": 20,
                    "effectId": "effect.dimensionmaster.skill.20",
                    "inputSlot": "ALT_V",
                },
            ]}), encoding="utf-8")
            bindings.write_text(json.dumps({
                "characterClass": "DIMENSIONMASTER",
                "bindings": [
                    {"skillId": 10, "clips": ["q"]},
                    {"skillId": 20, "clips": ["alt_v"]},
                ],
            }), encoding="utf-8")
            (imported / "skill.10.source-receipt.json").write_text(json.dumps({
                "schema": "lostark.effect-source-receipt",
                "characterClass": "DIMENSIONMASTER",
                "skillId": 10,
                "materialParameterBindings": [
                    {
                        "sourceMaterialPath": "fx_pkg.fx_mi.shared",
                        "sourcePhysicalPackage": "fx_pkg.upk",
                        "parent": "fx_m.parent",
                    },
                    {
                        "sourceMaterialPath": "fx_pkg.fx_mi.shared",
                        "sourcePhysicalPackage": "fx_pkg.upk",
                        "parent": "fx_m.parent",
                    },
                    {"sourceMaterialPath": "EngineMaterials.DefaultParticle"},
                ],
            }), encoding="utf-8")
            (imported / "skill.20.source-receipt.json").write_text(json.dumps({
                "schema": "lostark.effect-source-receipt",
                "characterClass": "DIMENSIONMASTER",
                "skillId": 20,
                "materialParameterBindings": [{
                    "sourceMaterialPath": "fx_pkg.fx_mi.alt_v",
                }],
            }), encoding="utf-8")

            catalog = MODULE.build_catalog(imported, player_skills, bindings)

            self.assertFalse(catalog["includeAltV"])
            self.assertEqual([10], [row["skillId"] for row in catalog["skills"]])
            self.assertEqual(1, catalog["summary"]["materialCandidateCount"])
            self.assertEqual(
                "fx_pkg.fx_mi.shared",
                catalog["unresolvedMaterialBindings"][0]["sourceMaterialPath"],
            )
            self.assertEqual([10], catalog["unresolvedMaterialBindings"][0]["skillIds"])
            self.assertEqual(
                1, catalog["summary"]["excludedEngineBuiltinMaterialBindingCount"]
            )

            with_alt_v = MODULE.build_catalog(
                imported, player_skills, bindings, include_alt_v=True
            )
            self.assertEqual(2, with_alt_v["summary"]["materialCandidateCount"])
            self.assertEqual(
                [10, 20], [row["skillId"] for row in with_alt_v["skills"]]
            )


if __name__ == "__main__":
    unittest.main()
