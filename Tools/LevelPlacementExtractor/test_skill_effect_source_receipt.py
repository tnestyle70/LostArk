import json
import tempfile
import unittest
from pathlib import Path

from build_skill_effect_source_receipt import (
    build_timeline,
    collect_system_graph,
    find_particle_system,
    load_graphs,
    parse_animnotify,
    resolve_material_parameters,
    resolve_runtime_resource_bindings,
    source_asset_parts,
)


class SkillEffectSourceReceiptTests(unittest.TestCase):
    def test_animnotify_timeline_accumulates_bound_clip_lengths(self):
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "sample.animnotify"
            path.write_text(
                '\n'.join(
                    (
                        '"clip_a" skill=7 len=0.6000 name="A"',
                        '  n t=0.1000 d=0.0000 kind=EFFECT src=PlayParticleEffect asset="FX_A.Group.Par_A" label="" win=NONE',
                        '"clip_b" skill=7 len=2.4000 name="B"',
                        '  n t=1.0000 d=0.1000 kind=EFFECT src=Effect asset="" label="" win=NONE',
                    )
                )
                + '\n',
                encoding="utf-8",
            )
            catalog = parse_animnotify(path)
            clips, events, duration = build_timeline(["clip_a", "clip_b"], catalog, 7)

            self.assertEqual(3.0, duration)
            self.assertEqual(0.6, clips[1]["offsetSeconds"])
            self.assertEqual(1.6, events[1]["globalTimeSeconds"])

    def test_cross_package_graph_reference_keeps_exact_node_class(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            graph_a = {
                "package": "FX_A",
                "physicalPackage": "missing_a.upk",
                "summary": {},
                "objects": [
                    {
                        "exportIndex": 0,
                        "className": "ParticleSystem",
                        "objectName": "Par_A",
                        "objectPath": "Group.Par_A",
                        "properties": {},
                        "references": [
                            {
                                "property": "Emitters",
                                "packageIndex": -1,
                                "objectPath": "FX_B.Group.ParticleSpriteEmitter_1",
                            }
                        ],
                    }
                ],
            }
            graph_b = {
                "package": "FX_B",
                "physicalPackage": "missing_b.upk",
                "summary": {},
                "objects": [
                    {
                        "exportIndex": 0,
                        "className": "ParticleSpriteEmitter",
                        "objectName": "ParticleSpriteEmitter_1",
                        "objectPath": "Group.ParticleSpriteEmitter_1",
                        "properties": {},
                        "references": [
                            {
                                "property": "Material",
                                "packageIndex": -2,
                                "objectPath": "FX_M.Materials.MI_A",
                            }
                        ],
                    }
                ],
            }
            path_a = root / "a.json"
            path_b = root / "b.json"
            path_a.write_text(json.dumps(graph_a), encoding="utf-8")
            path_b.write_text(json.dumps(graph_b), encoding="utf-8")
            index = load_graphs([("FX_A", path_a), ("FX_B", path_b)])
            found = find_particle_system(index, "FX_A", "Group.Par_A", "Par_A")
            self.assertIsNotNone(found)
            package_key, system = found
            closure = collect_system_graph(index, package_key, system)

            self.assertEqual(2, closure["summary"]["nodeCount"])
            self.assertEqual(1, closure["summary"]["emitterCount"])
            self.assertEqual("material", closure["resourceBindings"][0]["role"])
            self.assertEqual(0, closure["summary"]["unresolvedExternalReferenceCount"])

    def test_source_asset_parts_preserve_group_path(self):
        self.assertEqual(
            ("FX_POST", "FX_Par.Par_J_RGBNoise_01", "Par_J_RGBNoise_01"),
            source_asset_parts("FX_POST.FX_Par.Par_J_RGBNoise_01"),
        )

    def test_material_resolution_prefers_manifest_physical_package(self):
        catalog = {
            "fx_mi.mi_test": [
                {
                    "source_file": "wrong.upk",
                    "material_path": "fx_mi.mi_test",
                    "class": "materialinstanceconstant",
                    "textures": [],
                    "scalars": [],
                    "vectors": [],
                },
                {
                    "source_file": "exact.upk",
                    "material_path": "fx_mi.mi_test",
                    "class": "materialinstanceconstant",
                    "textures": [{"name": "map_a", "texture": "fx_tex_00.tex_a"}],
                    "scalars": [],
                    "vectors": [],
                },
            ]
        }
        resolved = resolve_material_parameters(
            "FX_M_MI_00.fx_mi.mi_test", catalog, {"fx_m_mi_00": "exact.upk"}
        )

        self.assertEqual("RESOLVED_EXACT_SOURCE_PACKAGE", resolved["resolutionStatus"])
        self.assertEqual("exact.upk", resolved["sourcePhysicalPackage"])
        self.assertEqual("fx_tex_00.tex_a", resolved["textures"][0]["texture"])

    def test_runtime_resource_resolution_uses_exact_effect_paths(self):
        with tempfile.TemporaryDirectory() as temporary:
            resources = Path(temporary)
            mesh = resources / "Effect" / "DimensionMaster" / "Meshes" / "fm_ring.wmodel"
            texture = (
                resources
                / "Effect"
                / "DimensionMaster"
                / "Textures"
                / "FX_TEX_00"
                / "fx_ring.dds"
            )
            mesh.parent.mkdir(parents=True)
            texture.parent.mkdir(parents=True)
            mesh.write_bytes(b"mesh")
            texture.write_bytes(b"texture")
            systems = [
                {
                    "graph": {
                        "resourceBindings": [
                            {"role": "mesh", "objectPath": "fx_sm_00.fm_ring"}
                        ]
                    }
                }
            ]
            materials = [
                {
                    "resolutionStatus": "RESOLVED_UNIQUE_PATH",
                    "textures": [{"texture": "fx_tex_00.fx_ring"}],
                }
            ]
            rows = resolve_runtime_resource_bindings(
                systems, materials, resources, "Effect/DimensionMaster"
            )

            self.assertEqual(2, len(rows))
            self.assertTrue(all(row["resolutionStatus"] == "RESOLVED_RUNTIME_ASSET" for row in rows))
            self.assertEqual(
                {"Effect/DimensionMaster/Meshes/fm_ring.wmodel", "Effect/DimensionMaster/Textures/FX_TEX_00/fx_ring.dds"},
                {row["assetId"] for row in rows},
            )

    def test_runtime_resource_resolution_uses_cook_receipt_for_flat_textures(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            resources = root / "Resources"
            texture = (
                resources
                / "Effect"
                / "LanceMaster"
                / "Textures"
                / "fx_ring.dds"
            )
            texture.parent.mkdir(parents=True)
            texture.write_bytes(b"texture")
            receipt = root / "runtime-cook-receipt.json"
            receipt.write_text(
                json.dumps(
                    {
                        "assets": [
                            {
                                "sourceAssetPath": "fx_tex_00.fx_ring",
                                "runtimeAssetId": (
                                    "Effect/LanceMaster/Textures/fx_ring.dds"
                                ),
                            }
                        ]
                    }
                ),
                encoding="utf-8",
            )
            rows = resolve_runtime_resource_bindings(
                [],
                [
                    {
                        "resolutionStatus": "RESOLVED_UNIQUE_PATH",
                        "textures": [{"texture": "fx_tex_00.fx_ring"}],
                    }
                ],
                resources,
                "Effect/LanceMaster",
                receipt,
            )

            self.assertEqual(1, len(rows))
            self.assertEqual("RESOLVED_RUNTIME_ASSET", rows[0]["resolutionStatus"])
            self.assertEqual(
                "Effect/LanceMaster/Textures/fx_ring.dds",
                rows[0]["assetId"],
            )


if __name__ == "__main__":
    unittest.main()
