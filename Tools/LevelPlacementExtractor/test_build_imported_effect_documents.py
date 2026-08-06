#!/usr/bin/env python3
"""Regression tests for the executable imported Effect document bridge."""

from __future__ import annotations

import unittest

from build_imported_effect_documents import (
    SourceIndex,
    build_distribution_recipe,
    build_document,
    promote_document,
    table_vector_samples,
)
from extract_ue3_particle_module_closure import obfuscate_package_name


def value(property_type: str, payload):
    return {"type": property_type, "structType": None, "value": payload}


def raw_float(*samples: float):
    cooked = [min(samples), max(samples), *samples]
    return value(
        "structproperty",
        {
            "size": 64,
            "properties": {
                "distribution": value("objectproperty", 0),
                "lookuptable": value("arrayproperty", cooked),
            },
        },
    )


def raw_vector(*samples: float):
    cooked = [min(samples), max(samples), *samples]
    return value(
        "structproperty",
        {
            "size": 96,
            "properties": {
                "distribution": value("objectproperty", 0),
                "lookuptable": value("arrayproperty", cooked),
            },
        },
    )


class ImportedEffectDocumentTests(unittest.TestCase):
    def test_cooked_vector_lookup_skips_range_cache_and_uses_three_float_stride(self) -> None:
        raw = {
            "lookuptable": value(
                "arrayproperty",
                [0.0, 500.0, 500.0, 0.0, 0.0, 500.0, 0.0, 0.0],
            )
        }
        self.assertEqual(
            table_vector_samples(raw, 1),
            [[500.0, 0.0, 0.0], [500.0, 0.0, 0.0]],
        )

    def test_raw_distribution_type_forwards_xyz_random_lock(self) -> None:
        raw_wrapper = value(
            "structproperty",
            {
                "size": 96,
                "properties": {
                    "distribution": value("objectproperty", 0),
                    "type": value("byteproperty", 4),
                    "op": value("byteproperty", 2),
                    "lookuptablechunksize": value("byteproperty", 6),
                    "lookuptablenumelements": value("byteproperty", 2),
                    "lookuptable": value(
                        "arrayproperty",
                        [6.0, 7.0, 6.0, 6.0, 6.0, 7.0, 7.0, 7.0],
                    ),
                },
            },
        )
        graph = {
            "nodes": [
                {
                    "nodeId": "module",
                    "package": "TEST",
                    "className": "particlemodulesize",
                    "objectPath": "particlemodulesize_0",
                    "properties": {"startsize": raw_wrapper},
                }
            ],
            "edges": [],
        }
        index = SourceIndex(graph, {"packages": []})
        module = index.get_id("module")
        self.assertIsNotNone(module)
        recipe = build_distribution_recipe(
            index, module, "startsize", raw_wrapper
        )
        self.assertEqual(recipe["operation"], 2)
        self.assertEqual(recipe["randomLockAxes"], 4)
        self.assertEqual(recipe["lookupTableChunkSize"], 6)
        self.assertEqual(recipe["lookupTableNumElements"], 2)

    def test_cooked_distribution_rejects_noncanonical_stride(self) -> None:
        raw_wrapper = value(
            "structproperty",
            {
                "size": 96,
                "properties": {
                    "distribution": value("objectproperty", 0),
                    "op": value("byteproperty", 1),
                    "lookuptablechunksize": value("byteproperty", 4),
                    "lookuptablenumelements": value("byteproperty", 1),
                    "lookuptable": value(
                        "arrayproperty",
                        [0.0, 1.0, 1.0, 0.0, 0.0, 99.0],
                    ),
                },
            },
        )
        graph = {
            "nodes": [
                {
                    "nodeId": "module",
                    "package": "TEST",
                    "className": "particlemodulesize",
                    "objectPath": "particlemodulesize_0",
                    "properties": {"startsize": raw_wrapper},
                }
            ],
            "edges": [],
        }
        index = SourceIndex(graph, {"packages": []})
        module = index.get_id("module")
        self.assertIsNotNone(module)
        with self.assertRaisesRegex(ValueError, "malformed UE3 cooked"):
            build_distribution_recipe(index, module, "startsize", raw_wrapper)

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
            camera_offset = f"{prefix}:cameraoffset"
            add_node(root, "particlesystem", f"{prefix}.system")
            add_node(emitter, "particlespriteemitter", f"{prefix}.emitter")
            add_node(lod, "particlelodlevel", f"{prefix}.lod", {"peakactiveparticles": value("intproperty", 12)})
            required_properties = {
                "buselocalspace": value("boolproperty", True),
                "emitterduration": value("floatproperty", 2.0),
            }
            if prefix == "sprite_system":
                required_properties.update({
                    "subimages_horizontal": value("intproperty", 8),
                    "subimages_vertical": value("intproperty", 4),
                })
            add_node(required, "particlemodulerequired", f"{prefix}.required", required_properties)
            add_node(spawn, "particlemodulespawn", f"{prefix}.spawn", {"rate": raw_float(10.0), "burstlist": value("arrayproperty", [])})
            add_node(lifetime, "particlemodulelifetime", f"{prefix}.lifetime", {"lifetime": raw_float(0.5, 5.0)})
            add_node(velocity, "particlemodulevelocity", f"{prefix}.velocity", {"startvelocity": raw_vector(-100.0, 0.0, 0.0, 100.0, 200.0, 0.0)})
            add_node(size, "particlemodulesize", f"{prefix}.size", {"startsize": raw_vector(50.0, 80.0, 0.0)})
            add_node(location, "particlemodulelocation", f"{prefix}.location", {"startlocation": raw_vector(-100.0, -50.0, 0.0, 100.0, 50.0, 0.0)})
            add_node(camera_offset, "particlemodulecameraoffset", f"{prefix}.cameraoffset")
            add_edge(root, "emitters[0]", emitter, prefix)
            add_edge(emitter, "lodlevels[0]", lod, prefix)
            for property_name, target in (
                ("requiredmodule", required),
                ("spawnmodule", spawn),
                ("modules[0]", lifetime),
                ("modules[1]", velocity),
                ("modules[2]", size),
                ("modules[3]", location),
                ("modules[4]", camera_offset),
            ):
                add_edge(lod, property_name, target, prefix)
            if prefix == "sprite_system":
                color = f"{prefix}:color"
                color_life = f"{prefix}:colorlife"
                subuv = f"{prefix}:subuv"
                add_node(color, "particlemodulecolor", f"{prefix}.color", {
                    "startcolor": raw_vector(10.0, 90.0, 10.0),
                    "startalpha": raw_float(1.0),
                })
                add_node(
                    color_life,
                    "particlemodulecoloroverlife",
                    f"{prefix}.colorlife",
                    {
                        "coloroverlife": raw_vector(1.0, 2.0, 3.0),
                        "alphaoverlife": raw_float(1.0, 0.0),
                    },
                )
                add_node(subuv, "particlemodulesubuv", f"{prefix}.subuv", {
                    "subimageindex": raw_float(0.0, 31.0),
                })
                add_edge(lod, "modules[5]", color, prefix)
                add_edge(lod, "modules[6]", color_life, prefix)
                add_edge(lod, "modules[7]", subuv, prefix)
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
        add_system("inactive_system", None, "texture", "test.texture.base")

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
                    "scalars": [
                        {"name": "fx_panning_intensity", "value": 100.0},
                        {"name": "emissive_intensity", "value": 2.0},
                        {"name": "distortion_intensity", "value": 3.0},
                    ],
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
                    {
                        "resolutionStatus": "RESOLVED_PARTICLE_GRAPH",
                        "sourceSystemId": name,
                        "globalTimeSeconds": index * 0.25,
                        "durationSeconds": 1.0,
                        **({
                            "actionCuePayload": {
                                "particleDataDecoded": True,
                                "attachment": {
                                    "mode": "SNAPSHOT_ROOT",
                                    "sourceAnchorNames": [],
                                    "runtimeAnchorSlotId": "root",
                                    "runtimeBoneName": "",
                                    "socketLocalTransform": {
                                        "position": [0.0, 0.0, 0.0],
                                        "rotationDegrees": [0.0, 0.0, 0.0],
                                        "scale": [1.0, 1.0, 1.0],
                                    },
                                },
                                "localTransform": {
                                    "position": [1.0, 2.0, 3.0],
                                    "rotationDegrees": [0.0, 0.0, 15.0],
                                    "scale": [2.0, 2.0, 2.0],
                                },
                            }
                        } if name == "sprite_system" else {}),
                    }
                    for index, name in enumerate(("sprite_system", "mesh_system", "light_system"))
                ] + [
                    {
                        "eventId": "source-event-repeat",
                        "resolutionStatus": "RESOLVED_PARTICLE_GRAPH",
                        "sourceSystemId": "sprite_system",
                        "globalTimeSeconds": 1.25,
                        "durationSeconds": 0.5,
                        "actionCuePayload": {
                            "particleDataDecoded": True,
                            "attachment": {
                                "mode": "SNAPSHOT_ROOT",
                                "sourceAnchorNames": [],
                                "runtimeAnchorSlotId": "root",
                                "runtimeBoneName": "",
                                "socketLocalTransform": {
                                    "position": [0.0, 0.0, 0.0],
                                    "rotationDegrees": [0.0, 0.0, 0.0],
                                    "scale": [1.0, 1.0, 1.0],
                                },
                            },
                            "localTransform": {
                                "position": [4.0, 5.0, 6.0],
                                "rotationDegrees": [0.0, 0.0, 30.0],
                                "scale": [3.0, 3.0, 3.0],
                            },
                        },
                    }
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
        self.assertEqual(document["version"], 10)
        self.assertEqual(document["particleSystem"], {
            "uniformScaleMultiplier": 1.0,
            "yawOffsetDegrees": 0.0,
            "directionYawDegrees": 0.0,
            "initialSpeedMultiplier": 1.0,
        })
        self.assertEqual(document["modelCues"], [])
        self.assertIn("F 2050500", document["displayName"])
        self.assertEqual(conversion["summary"]["sourceEmitterPartitionCount"], 3)
        self.assertEqual(conversion["summary"]["graphEmitterPartitionCount"], 4)
        self.assertEqual(
            conversion["summary"]["inactiveSourceSystemEmitterPartitionCount"],
            1,
        )
        self.assertEqual(conversion["summary"]["convertedEmitterCount"], 3)
        self.assertEqual(conversion["summary"]["unsupportedEmitterCount"], 0)
        self.assertEqual(len(document["elements"]), 4)
        self.assertFalse(any(
            row["groupId"] == "inactive_system"
            for row in document["elements"]
        ))
        sprite = next(row for row in document["elements"] if row["groupId"] == "sprite_system")
        repeated_sprite = next(
            row for row in document["elements"]
            if row["groupId"] == "sprite_system.source-event-repeat"
        )
        mesh = next(row for row in document["elements"] if row["groupId"] == "mesh_system")
        self.assertEqual(sprite["kind"], "particle")
        self.assertTrue(sprite["sourceRecipe"]["enabled"])
        self.assertGreater(len(sprite["sourceRecipe"]["modules"]), 0)
        self.assertEqual(sprite["resources"][0]["slotId"], "base")
        self.assertEqual(sprite["detail"]["particle"]["initialPositionMin"], [-1.0, -0.5, 0.0])
        self.assertEqual(sprite["detail"]["particle"]["initialPositionMax"], [1.0, 0.5, 0.0])
        self.assertEqual(sprite["detail"]["transform"]["position"], [1.0, 2.0, 3.0])
        self.assertEqual(sprite["detail"]["transform"]["scale"], [2.0, 2.0, 2.0])
        self.assertEqual(sprite["detail"]["timing"]["lifeTimeSeconds"], 2.0)
        self.assertEqual(sprite["detail"]["particle"]["lifeTimeSeconds"], [0.5, 5.0])
        self.assertEqual(sprite["detail"]["color"]["emissiveIntensity"], 2.0)
        self.assertEqual(sprite["detail"]["color"]["distortionIntensity"], 3.0)
        self.assertEqual(sprite["detail"]["color"]["multiply"], [1.0, 1.0, 1.0, 1.0])
        self.assertFalse(sprite["detail"]["linearLerp"]["colorMultiply"])
        self.assertEqual(sprite["detail"]["linearLerp"]["endColorMultiply"], [1.0, 1.0, 1.0, 1.0])
        self.assertFalse(sprite["detail"]["uv"]["sequence"])
        self.assertEqual(sprite["detail"]["uv"]["tileColumns"], 1)
        self.assertEqual(sprite["detail"]["uv"]["tileRows"], 1)
        self.assertEqual(
            repeated_sprite["detail"]["timing"]["startDelaySeconds"], 1.25
        )
        self.assertEqual(
            repeated_sprite["detail"]["transform"]["position"],
            [4.0, 5.0, 6.0],
        )
        self.assertEqual(
            repeated_sprite["detail"]["transform"]["scale"],
            [3.0, 3.0, 3.0],
        )
        self.assertEqual(mesh["kind"], "particle")
        self.assertTrue(any(row["slotId"] == "meshModel" for row in mesh["resources"]))
        self.assertFalse(mesh["detail"]["particle"]["billboard"])
        sprite_conversion = next(
            row for row in conversion["elementConversions"]
            if row.get("sourceSystemId") == "sprite_system"
        )
        self.assertTrue(any(
            row["className"] == "particlemodulecameraoffset"
            for row in sprite_conversion["unrepresentedModules"]
        ))

    def test_promotion_requires_matching_skill_id_and_preserves_elements(self) -> None:
        imported = {
            "schema": "lostark.effect-authoring",
            "version": 7,
            "effectAssetId": "effect.dimensionmaster.skill.2050500.imported",
            "displayName": "Imported",
            "modelCues": [],
            "elements": [{"id": "portal"}],
        }
        cue = {"cueId": "dimension_summon"}
        promoted = promote_document(
            imported,
            "effect.dimensionmaster.skill.2050500",
            "Boundary Break",
            [cue],
        )
        self.assertEqual(promoted["version"], 10)
        self.assertEqual(promoted["particleSystem"]["uniformScaleMultiplier"], 1.0)
        self.assertEqual(promoted["effectAssetId"],
                         "effect.dimensionmaster.skill.2050500")
        self.assertEqual(promoted["elements"], imported["elements"])
        self.assertEqual(promoted["modelCues"], [cue])
        self.assertEqual(imported["modelCues"], [])
        with self.assertRaises(ValueError):
            promote_document(imported,
                             "effect.dimensionmaster.skill.2050510", "Wrong")


if __name__ == "__main__":
    unittest.main()
