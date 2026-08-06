#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("build_effect_source_material_contract.py")
SPEC = importlib.util.spec_from_file_location(
    "build_effect_source_material_contract", MODULE_PATH
)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class BuildEffectSourceMaterialContractTests(unittest.TestCase):
    def test_duplicate_material_path_uses_exact_source_package_evidence(self):
        effect = {
            "effectAssetId": "effect.test",
            "elements": [{
                "id": "p0",
                "kind": "particle",
                "resources": [],
                "material": {
                    "templateId": "effect.source_material",
                    "sourceMaterialPath": "fx_pkg.fx_mi.shared",
                },
            }],
        }
        graph = {"materialParameterBindings": [{
            "sourceMaterialPath": "fx_pkg.fx_mi.shared",
            "sourcePhysicalPackage": "RIGHT.upk",
            "parent": "fx_m.right",
            "scalars": [{"name": "power", "value": 7.0}],
        }]}
        material_map = {"materials": {"fx_mi.shared": [
            {
                "source_file": "WRONG.upk",
                "material_path": "fx_mi.shared",
                "parent": "fx_m.wrong",
            },
            {
                "source_file": "RIGHT.upk",
                "material_path": "fx_mi.shared",
                "parent": "fx_m.right",
                "scalars": [{"name": "power", "value": 7.0}],
            },
        ]}}

        contract, result = MODULE.build_contract(
            effect, graph, {"elementConversions": []}, {"assets": []},
            material_map,
        )

        self.assertEqual([], result["failures"])
        identity = contract["materialIdentities"][0]
        self.assertEqual("RIGHT.upk", identity["sourcePhysicalPackage"])
        self.assertEqual("fx_m.right", identity["parentMaterialPath"])
        self.assertEqual(
            [{"name": "power", "value": 7.0}],
            identity["sourceParameters"]["scalars"],
        )

    def test_dynamic_parameter_names_become_typed_reconstruction_semantics(self):
        element = {
            "sourceRecipe": {
                "modules": [
                    {
                        "className": "ParticleModuleParameterDynamic",
                        "literals": [
                            {
                                "propertyPath": "DynamicParams[0].ParamName",
                                "value": "AlphaDissolve[0-1]",
                            },
                            {
                                "propertyPath": "DynamicParams[1].ParamName",
                                "value": "Diff_Panner",
                            },
                        ],
                    }
                ]
            }
        }
        self.assertEqual(
            ["opacity", "uv_pan", "unbound", "unbound"],
            MODULE.dynamic_parameter_semantics(element),
        )

    def test_particle_materials_group_by_parent_without_hiding_occurrences(self):
        effect = {
            "effectAssetId": "effect.test",
            "elements": [
                {
                    "id": "p0",
                    "kind": "particle",
                    "resources": [],
                    "material": {
                        "templateId": "effect.source_material",
                        "sourceMaterialPath": "fx_mi.fx.child_a",
                    },
                },
                {
                    "id": "p1",
                    "kind": "particle",
                    "resources": [{"slotId": "base", "assetId": "Effect/T.dds"}],
                    "material": {
                        "templateId": "effect.standard",
                        "sourceMaterialPath": "fx_mi.fx.child_b",
                    },
                },
            ],
        }
        graph = {
            "materialParameterBindings": [
                {
                    "sourceMaterialPath": "fx_mi.fx.child_a",
                    "sourcePhysicalPackage": "fx.upk",
                    "parent": "fx_m.parent",
                },
                {
                    "sourceMaterialPath": "fx_mi.fx.child_b",
                    "sourcePhysicalPackage": "fx.upk",
                    "parent": "fx_m.parent",
                },
            ]
        }
        receipt = {
            "elementConversions": [
                {
                    "targetKind": "particle",
                    "resourceMappings": [
                        {"status": "PARAMETER_NAME_HEURISTIC"}
                    ],
                }
            ]
        }
        manifest = {
            "assets": [
                {
                    "sourceAssetPath": "fx_m.parent",
                    "physicalPackage": "fx.upk",
                }
            ]
        }
        material_map = {
            "materials": {
                "fx.child_a": [
                    {
                        "source_file": "fx.upk",
                        "material_path": "fx.child_a",
                        "parent": "fx_m.parent",
                    }
                ],
                "fx.child_b": [
                    {
                        "source_file": "fx.upk",
                        "material_path": "fx.child_b",
                        "parent": "fx_m.parent",
                    }
                ],
            }
        }

        contract, result = MODULE.build_contract(
            effect, graph, receipt, manifest, material_map
        )
        self.assertEqual(2, result["summary"]["particleElementCount"])
        self.assertEqual(2, result["summary"]["materialIdentityCount"])
        self.assertEqual(1, result["summary"]["parentProfileGroupCount"])
        self.assertEqual(1, result["summary"]["pendingRuntimeOccurrenceCount"])
        self.assertEqual(1, result["summary"]["parameterNameHeuristicCount"])
        self.assertEqual(
            "fx.upk", contract["materialIdentities"][0]["parentSourcePhysicalPackage"]
        )
        upgraded = MODULE.upgrade_effect_document(effect, contract)
        self.assertEqual(11, upgraded["version"])
        self.assertTrue(
            upgraded["elements"][0]["material"]["sourceProfile"]["enabled"]
        )
        self.assertEqual(
            "reconstructed_profile",
            upgraded["elements"][1]["material"]["sourceProfile"][
                "semanticStatus"
            ],
        )


if __name__ == "__main__":
    unittest.main()
