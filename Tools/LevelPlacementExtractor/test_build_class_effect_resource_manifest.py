#!/usr/bin/env python3

from __future__ import annotations

import csv
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name(
    "build_class_effect_resource_manifest.py"
)
SPEC = importlib.util.spec_from_file_location(
    "build_class_effect_resource_manifest", MODULE_PATH
)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class BuildClassEffectResourceManifestTests(unittest.TestCase):
    def test_parent_named_texture_defaults_inherit_child_skill_id(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            graph_path = root / "skill.2050210.normalized-effect-graph.json"
            graph_path.write_text(
                json.dumps({
                    "skillId": 2050210,
                    "characterClass": "DIMENSIONMASTER",
                    "sourceSystems": [],
                    "materialParameterBindings": [{
                        "resolutionStatus": "RESOLVED_EXACT_SOURCE_PACKAGE",
                        "sourceMaterialPath": (
                            "fx_m_mi_j_00.fx_mi.fx_j_me_localcrack_01_04_tr"
                        ),
                        "sourcePhysicalPackage": "materials.upk",
                        "parent": "fx_m.fx_j_me_localcrack_01_tr",
                        "textures": [{
                            "name": "normal_tex",
                            "texture": "fx_tex_06.fx_j_normal_bc5_09",
                        }],
                    }],
                }),
                encoding="utf-8",
            )
            other_graph_path = (
                root / "skill.2050240.normalized-effect-graph.json"
            )
            other_graph_path.write_text(
                json.dumps({
                    "skillId": 2050240,
                    "characterClass": "DIMENSIONMASTER",
                    "sourceSystems": [{
                        "sourceAsset": "fx_other.system",
                        "resourceBindings": [{
                            "role": "texture",
                            "objectPath": (
                                "fx_tex_00.fx_b_atypical_004_cube"
                            ),
                        }, {
                            "role": "texture",
                            "objectPath": "fx_tex_04.fx_h_atypical_01_1",
                        }],
                    }],
                    "materialParameterBindings": [],
                }),
                encoding="utf-8",
            )
            inventory_path = root / "inventory.csv"
            with inventory_path.open("w", encoding="utf-8", newline="") as out:
                writer = csv.DictWriter(
                    out, fieldnames=("logical_name", "physical_file")
                )
                writer.writeheader()
                writer.writerows((
                    {"logical_name": "fx_m_mi_j_00",
                     "physical_file": "materials.upk"},
                    {"logical_name": "fx_m", "physical_file": "materials.upk"},
                    {"logical_name": "fx_tex_00", "physical_file": "t0.upk"},
                    {"logical_name": "fx_tex_04", "physical_file": "t4.upk"},
                    {"logical_name": "fx_tex_06", "physical_file": "t6.upk"},
                ))
            material_map_path = root / "material-map.json"
            material_map_path.write_text(
                json.dumps({
                    "materials": {
                        "fx_mi.fx_j_me_localcrack_01_04_tr": [{
                            "material_path": (
                                "fx_m_mi_j_00.fx_mi."
                                "fx_j_me_localcrack_01_04_tr"
                            ),
                            "source_file": "materials.upk",
                            "parent": (
                                "fx_m_mi_j_00.fx_m."
                                "fx_j_me_localcrack_01_tr"
                            ),
                            "textures": [{
                                "name": "normal_tex",
                                "texture": "fx_tex_06.fx_j_normal_bc5_09",
                            }],
                        }],
                        "catalog.local-crack-defaults": [{
                            "material_path": "catalog.reflection",
                            "source_file": "materials.upk",
                            "parent": "catalog.parent",
                            "textures": [{
                                "name": "dependency",
                                "texture": (
                                    "fx_tex_00.fx_b_atypical_004_cube"
                                ),
                            }, {
                                "name": "dependency",
                                "texture": "fx_tex_04.fx_h_atypical_01_1",
                            }, {
                                "name": "dependency",
                                "texture": "fx_tex_00.fx_unused_parent_default",
                            }],
                        }],
                    },
                }),
                encoding="utf-8",
            )
            parent_evidence_path = root / "parent-evidence.json"
            parent_evidence_path.write_text(
                json.dumps({
                    "materials": {
                        "evidence-only": [{
                            "material_path": "evidence-only.material",
                            "source_file": "materials.upk",
                            "parent": "catalog.parent",
                            "textures": [{
                                "name": "dependency",
                                "texture": "fx_tex.fx_e_adba",
                            }],
                        }],
                    },
                    "parentMaterialEvidence": {
                        "local-crack": {
                            "parentMaterialPath": (
                                "fx_m_mi_j_00.fx_m."
                                "fx_j_me_localcrack_01_tr"
                            ),
                            "materialEvidence": {
                                "collectedTextureParameters": [{
                                    "name": "normal_tex",
                                    "texture": "fx_j_normal_bc5_09",
                                }, {
                                    "name": "refle_tex",
                                    "texture": "fx_b_atypical_004_cube",
                                }, {
                                    "name": "dissolve_tex",
                                    "texture": "fx_h_atypical_01_1",
                                }, {
                                    "name": "unused_tex",
                                    "texture": "fx_unused_parent_default",
                                }, {
                                    "name": "evidence_only_tex",
                                    "texture": "fx_e_adba",
                                }]
                            },
                        }
                    },
                }),
                encoding="utf-8",
            )

            legacy_manifest = MODULE.build_manifest(
                [graph_path, other_graph_path],
                inventory_path,
                "DIMENSIONMASTER",
                material_map_path,
            )
            legacy_by_path = {
                row["sourceAssetPath"].casefold(): row
                for row in legacy_manifest["assets"]
            }
            self.assertEqual(
                [2050210],
                legacy_by_path[
                    "fx_tex_06.fx_j_normal_bc5_09"
                ]["skillIds"],
            )

            manifest = MODULE.build_manifest(
                [graph_path, other_graph_path],
                inventory_path,
                "DIMENSIONMASTER",
                material_map_path,
                parent_evidence_path,
            )
            by_path = {
                row["sourceAssetPath"].casefold(): row
                for row in manifest["assets"]
            }
            self.assertEqual(
                legacy_manifest["summary"], manifest["summary"]
            )
            self.assertNotIn("fx_tex.fx_e_adba", by_path)
            self.assertNotIn(
                "fx_tex_00.fx_unused_parent_default", by_path
            )
            self.assertEqual(
                [2050210],
                by_path[
                    "fx_tex_06.fx_j_normal_bc5_09"
                ]["skillIds"],
            )
            for source_path in (
                "fx_tex_00.fx_b_atypical_004_cube",
                "fx_tex_04.fx_h_atypical_01_1",
            ):
                with self.subTest(source_path=source_path):
                    row = by_path[source_path]
                    self.assertEqual([2050210, 2050240], row["skillIds"])
                    self.assertEqual(["texture"], row["roles"])
                    self.assertEqual(
                        "RESOLVED_SOURCE_PACKAGE", row["resolutionStatus"]
                    )

    def test_parent_identity_match_uses_dot_boundary(self) -> None:
        self.assertTrue(MODULE.parent_identity_matches(
            "fx_m_mi_j_00.fx_m.fx_j_me_localcrack_01_tr",
            "fx_m.fx_j_me_localcrack_01_tr",
        ))
        self.assertFalse(MODULE.parent_identity_matches(
            "fx_m.fx_j_me_localcrack_01_tr_variant",
            "fx_m.fx_j_me_localcrack_01_tr",
        ))


if __name__ == "__main__":
    unittest.main()
