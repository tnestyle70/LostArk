#!/usr/bin/env python3
"""Regression tests for the executable imported Effect document bridge."""

from __future__ import annotations

import unittest

from build_imported_effect_documents import build_document
from extract_ue3_particle_module_closure import obfuscate_package_name


def value(property_type: str, payload):
    return {"type": property_type, "structType": None, "value": payload}


def raw_float(*samples: float):
    return value(
        "structproperty",
        {
            "size": 64,
            "properties": {
                "distribution": value("objectproperty", 0),
                "lookuptable": value("arrayproperty", list(samples)),
            },
        },
    )


def raw_vector(*samples: float):
    return value(
        "structproperty",
        {
            "size": 96,
            "properties": {
                "distribution": value("objectproperty", 0),
                "lookuptable": value("arrayproperty", list(samples)),
            },
        },
    )


class ImportedEffectDocumentTests(unittest.TestCase):
    def test_lostark_package_name_resolution_is_stable(self) -> None:
        self.assertEqual(obfuscate_package_name("FX_PC_SWP_00"), "YGI3SWD3SH9W3D11G9KMHCS9M")
        self.assertEqual(obfuscate_package_name("FX_PC_SWP_01"), "YGI3SWD3SH9W3D18G9KMHCS9M")
        self.assertEqual(obfuscate_package_name("FX_CM_02"), "XFH2RCA2R0EF0CYE90QX0CMQ")

    def test_emitter_partition_maps_particle_mesh_and_unsupported_light(self) -> None:
        nodes = []
        edges = []
        systems = []

        def add_node(node_id: str, class_name: str, object_path: str, properties=None):
            nodes.append(
                {
                    "nodeId": node_id,
                    "package": "TEST_FX",
                    "exportIndex": len(nodes),
                    "className": class_name,
                    "objectName": object_path.rsplit(".", 1)[-1],
                    "objectPath": object_path,
                    "properties": properties or {},
                }
            )

        def add_edge(source: str, property_name: str, target: str, system_id: str):
            edges.append(
                {
                    "sourceNodeId": source,
                    "referenceIndex": len(edges),
                    "property": property_name,
                    "packageIndex": 1,
                    "objectPath": target,
                    "targetNodeId": target,
                    "sourceSystemId": system_id,
                }
            )

        def add_system(prefix: str, type_class: str | None, role: str, object_path: str):
            root = f"{prefix}:root"
            emitter = f"{prefix}:emitter"
            lod = f"{prefix}:lod"
            required = f"{prefix}:required"
            spawn = f"{prefix}:spawn"
            lifetime = f"{prefix}:lifetime"
            velocity = f"{prefix}:velocity"
            size = f"{prefix}:size"
            location = f"{prefix}:location"
            add_node(root, "particlesystem", f"{prefix}.system")
            add_node(emitter, "particlespriteemitter", f"{prefix}.emitter")
            add_node(lod, "particlelodlevel", f"{prefix}.lod", {"peakactiveparticles": value("intproperty", 12)})
            add_node(required, "particlemodulerequired", f"{prefix}.required", {"buselocalspace": value("boolproperty", True), "emitterduration": value("floatproperty", 2.0)})
            add_node(spawn, "particlemodulespawn", f"{prefix}.spawn", {"rate": raw_float(10.0), "burstlist": value("arrayproperty", [])})
            add_node(lifetime, "particlemodulelifetime", f"{prefix}.lifetime", {"lifetime": raw_float(0.5, 1.0)})
            add_node(velocity, "particlemodulevelocity", f"{prefix}.velocity", {"startvelocity": raw_vector(-100.0, 0.0, 0.0, 100.0, 200.0, 0.0)})
            add_node(size, "particlemodulesize", f"{prefix}.size", {"startsize": raw_vector(50.0, 80.0, 0.0)})
            add_node(location, "particlemodulelocation", f"{prefix}.location", {"startlocation": raw_vector(-100.0, -50.0, 0.0, 100.0, 50.0, 0.0)})
            add_edge(root, "emitters[0]", emitter, prefix)
            add_edge(emitter, "lodlevels[0]", lod, prefix)
            for property_name, target in (
                ("requiredmodule", required),
                ("spawnmodule", spawn),
                ("modules[0]", lifetime),
                ("modules[1]", velocity),
                ("modules[2]", size),
                ("modules[3]", location),
            ):
                add_edge(lod, property_name, target, prefix)
            bindings = [
                {
                    "sourceNodeId": required,
                    "referenceIndex": 0,
                    "property": "material",
                    "packageIndex": -1,
                    "objectPath": f"test.material.{prefix}",
                    "role": "material",
                }
            ]
            if type_class:
                type_data = f"{prefix}:typedata"
                add_node(type_data, type_class, f"{prefix}.typedata")
                add_edge(lod, "typedatamodule", type_data, prefix)
                if "mesh" in type_class.casefold():
                    bindings.append(
                        {
                            "sourceNodeId": type_data,
                            "referenceIndex": 0,
                            "property": "mesh",
                            "packageIndex": -2,
                            "objectPath": object_path,
                            "role": "mesh",
                        }
                    )
            systems.append(
                {
                    "sourceSystemId": prefix,
                    "sourceAsset": f"TEST.{prefix}",
                    "logicalPackage": "TEST_FX",
                    "objectName": prefix,
                    "objectPath": prefix,
                    "rootNodeId": root,
                    "nodeIds": [],
                    "resourceBindings": bindings,
                    "unresolvedExternalReferences": [],
                    "summary": {},
                }
            )

        add_system("sprite_system", None, "texture", "test.texture.base")
        add_system("mesh_system", "particlemoduletypedatamesh", "mesh", "test.mesh.portal")
        add_system("light_system", "efparticlemoduletypedatalight", "none", "")

        graph = {
            "characterClass": "DimensionMaster",
            "skillId": 2050500,
            "sourceSystems": systems,
            "nodes": nodes,
            "edges": edges,
            "materialParameterBindings": [
                {
                    "sourceMaterialPath": "test.material.sprite_system",
                    "textures": [{"name": "emissive_tex", "texture": "test.texture.base"}],
                    "scalars": [],
                    "vectors": [],
                },
                {
                    "sourceMaterialPath": "test.material.mesh_system",
                    "textures": [],
                    "scalars": [],
                    "vectors": [],
                },
            ],
            "runtimeResourceBindings": [
                {"role": "texture", "sourceObjectPath": "test.texture.base", "resolutionStatus": "RESOLVED_RUNTIME_ASSET", "assetId": "Effect/DimensionMaster/Textures/FX_TEX_00/test.dds"},
                {"role": "mesh", "sourceObjectPath": "test.mesh.portal", "resolutionStatus": "RESOLVED_RUNTIME_ASSET", "assetId": "Effect/DimensionMaster/Meshes/test.wmodel"},
                {"role": "texture", "sourceObjectPath": "fx_tex_00.fx_a_blankwhite_01", "resolutionStatus": "RESOLVED_RUNTIME_ASSET", "assetId": "Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_blankwhite_01.dds"},
            ],
        }
        receipt = {
            "characterClass": "DimensionMaster",
            "skillId": 2050500,
            "inputSlot": "F",
            "timeline": {
                "events": [
                    {"resolutionStatus": "RESOLVED_PARTICLE_GRAPH", "sourceSystemId": name, "globalTimeSeconds": index * 0.25, "durationSeconds": 1.0}
                    for index, name in enumerate(("sprite_system", "mesh_system", "light_system"))
                ]
            },
            "unsupportedUnresolved": [],
        }
        closure = {
            "skillId": 2050500,
            "packages": [],
            "summary": {"packageCount": 0, "requestCount": 0, "unresolvedRequestCount": 0},
        }

        document, conversion = build_document(receipt, graph, closure)
        self.assertEqual(conversion["summary"]["sourceEmitterPartitionCount"], 3)
        self.assertEqual(conversion["summary"]["convertedEmitterCount"], 2)
        self.assertEqual(conversion["summary"]["unsupportedEmitterCount"], 1)
        self.assertEqual(len(document["elements"]), 2)
        sprite = next(row for row in document["elements"] if row["groupId"] == "sprite_system")
        mesh = next(row for row in document["elements"] if row["groupId"] == "mesh_system")
        self.assertEqual(sprite["kind"], "particle")
        self.assertEqual(sprite["resources"][0]["slotId"], "base")
        self.assertEqual(sprite["detail"]["particle"]["initialPositionMin"], [-1.0, -0.5, 0.0])
        self.assertEqual(sprite["detail"]["particle"]["initialPositionMax"], [1.0, 0.5, 0.0])
        self.assertEqual(mesh["kind"], "particle")
        self.assertTrue(any(row["slotId"] == "meshModel" for row in mesh["resources"]))
        self.assertFalse(mesh["detail"]["particle"]["billboard"])


if __name__ == "__main__":
    unittest.main()
