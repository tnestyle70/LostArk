#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("build_imported_effect_drafts.py")
SPEC = importlib.util.spec_from_file_location("build_imported_effect_drafts", MODULE_PATH)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class BuildImportedEffectDraftsTests(unittest.TestCase):
    def test_mesh_and_material_textures_become_review_candidates(self) -> None:
        source = {
            "characterClass": "TEST",
            "skillId": 1,
            "inputSlot": "Q",
            "timeline": {"durationSeconds": 1, "clips": [], "events": []},
            "unsupportedUnresolved": [
                {
                    "sourceObjectPath": "shared.module",
                    "reason": "external_graph_package_not_loaded",
                }
            ],
        }
        graph = {
            "characterClass": "TEST",
            "skillId": 1,
            "sourceSystems": [
                {
                    "sourceSystemId": "test.par",
                    "sourceAsset": "TEST.par",
                    "logicalPackage": "TEST",
                    "resourceBindings": [
                        {"role": "mesh", "objectPath": "fx_sm.mesh"},
                        {"role": "material", "objectPath": "fx_m.mat"},
                    ],
                    "summary": {
                        "featureClasses": ["ParticleModuleTypeDataMesh"]
                    },
                }
            ],
            "materialParameterBindings": [
                {
                    "sourceMaterialPath": "fx_m.mat",
                    "textures": [{"name": "noise_map", "texture": "fx_tex.noise"}],
                }
            ],
        }
        cook = {
            "assets": [
                {
                    "sourceAssetPath": "fx_sm.mesh",
                    "runtimeAssetId": "Effect/Test/Meshes/mesh.wmodel",
                },
                {
                    "sourceAssetPath": "fx_tex.noise",
                    "runtimeAssetId": "Effect/Test/Textures/noise.dds",
                },
            ]
        }
        draft = MODULE.build_draft(source, graph, cook)
        system = draft["sourceSystems"][0]
        self.assertEqual(["mesh", "particle"], system["candidateKinds"])
        self.assertEqual(
            ["mesh_model", "noise"],
            [row["suggestedSlot"] for row in system["resourceCandidates"]],
        )
        self.assertEqual(
            "NOT_PUBLISHABLE_SOURCE_CONVERSION_DRAFT",
            draft["runtimeDocumentStatus"],
        )
        self.assertEqual(
            source["unsupportedUnresolved"], draft["unsupportedOrUnresolved"]
        )


if __name__ == "__main__":
    unittest.main()
