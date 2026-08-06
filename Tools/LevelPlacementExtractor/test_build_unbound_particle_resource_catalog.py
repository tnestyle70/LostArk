#!/usr/bin/env python3

from __future__ import annotations

import csv
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("build_unbound_particle_resource_catalog.py")
SPEC = importlib.util.spec_from_file_location(
    "build_unbound_particle_resource_catalog", MODULE_PATH
)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class UnboundParticleResourceCatalogTests(unittest.TestCase):
    def test_particle_root_and_external_mesh_are_cataloged(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            graph_path = root / "FX_TEST.particle-graph.json"
            graph_path.write_text(
                json.dumps(
                    {
                        "package": "FX_TEST",
                        "objects": [
                            {
                                "exportIndex": 0,
                                "className": "ParticleSystem",
                                "objectName": "par_test",
                                "objectPath": "par_test",
                                "properties": {},
                                "references": [
                                    {
                                        "property": "Emitters",
                                        "packageIndex": 2,
                                        "objectPath": "par_test.emitter_0",
                                    }
                                ],
                            },
                            {
                                "exportIndex": 1,
                                "className": "ParticleSpriteEmitter",
                                "objectName": "emitter_0",
                                "objectPath": "par_test.emitter_0",
                                "properties": {},
                                "references": [
                                    {
                                        "property": "Mesh",
                                        "packageIndex": -1,
                                        "objectPath": "fx_sm_00.fm_test",
                                    }
                                ],
                            },
                        ],
                    }
                ),
                encoding="utf-8",
            )
            inventory = root / "inventory.csv"
            with inventory.open("w", encoding="utf-8", newline="") as stream:
                writer = csv.DictWriter(
                    stream, fieldnames=["logical_name", "physical_file"]
                )
                writer.writeheader()
                writer.writerow(
                    {"logical_name": "fx_sm_00", "physical_file": "mesh.upk"}
                )
                writer.writerow(
                    {"logical_name": "fx_test", "physical_file": "test.upk"}
                )

            document = MODULE.build_catalog(
                [graph_path], inventory, "WARLORD", None
            )
            self.assertEqual(1, document["summary"]["sourceSystemCount"])
            self.assertEqual("UNBOUND_SOURCE_SYSTEM", document["sourceSystems"][0]["resolutionStatus"])
            self.assertEqual("fx_sm_00.fm_test", document["assets"][0]["sourceAssetPath"])
            self.assertEqual(["mesh"], document["assets"][0]["roles"])

    def test_particle_sound_direct_material_metadata_is_preserved(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "basebuff.json"
            path.write_text(
                json.dumps(
                    {
                        "profileId": "VALTAN_BASEBUFF",
                        "particleSystems": [],
                        "materials": [
                            {
                                "sourceAsset": "FX_MAT.M_Expost",
                                "actionNames": ["SpecialSkill_PP"],
                                "occurrenceCount": 1,
                            }
                        ],
                        "meshes": [
                            {
                                "sourceAsset": "FX_SM.Mesh_Test",
                                "classNames": ["StaticMesh"],
                                "actionIds": [17250],
                                "actionNames": ["Guardian"],
                                "occurrenceCount": 1,
                            }
                        ],
                    }
                ),
                encoding="utf-8",
            )
            systems, materials, resources, receipts = (
                MODULE.load_action_source_metadata([path])
            )
            self.assertEqual({}, systems)
            self.assertEqual(
                {"SpecialSkill_PP"},
                materials["fx_mat.m_expost"]["actionNames"],
            )
            self.assertEqual("mesh", resources["mesh:fx_sm.mesh_test"]["role"])
            self.assertEqual(
                {17250}, resources["mesh:fx_sm.mesh_test"]["actionIds"]
            )
            self.assertEqual(1, len(receipts))


if __name__ == "__main__":
    unittest.main()
