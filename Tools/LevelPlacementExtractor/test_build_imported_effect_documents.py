#!/usr/bin/env python3
"""Regression tests for the executable imported Effect document bridge."""

from __future__ import annotations

import unittest

from build_imported_effect_documents import (
    SourceIndex,
    SourceObject,
    build_distribution_recipe,
    build_document,
    choose_resources,
    default_detail,
    flatten_source_properties,
    mesh_uses_model_material,
    screen_post_presentation,
    promote_document,
    selected_lod_partitions,
    table_vector_samples,
    texture_slot,
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
    def test_texture_roles_do_not_promote_normal_or_alpha_to_base(self) -> None:
        self.assertIsNone(texture_slot("normal_tex"))
        self.assertIsNone(texture_slot("cracknormal_tex"))
        self.assertEqual("mask", texture_slot("alpha_texture2"))
        self.assertEqual("mask", texture_slot("opacity_tex"))
        self.assertEqual("emissive", texture_slot("emissive_tex_02"))
        self.assertEqual("base", texture_slot("refle_tex"))

    def test_material_resources_keep_source_semantics_without_slot_spill(self) -> None:
        material_path = "fx_m_mi_s_00.fx_mi.fx_s_me_localcrack_01_01_tr"
        normal_path = "fx_tex_06.fx_j_normal_bc5_09"
        reflection_path = "fx_tex_06.fx_s_atypical_006"
        duplicate_normal_path = "fx_tex_06.fx_j_normal_bc5_10"
        system = {
            "resourceBindings": [{
                "sourceNodeId": "required",
                "role": "material",
                "objectPath": material_path,
            }],
        }
        graph = {
            "materialParameterBindings": [{
                "sourceMaterialPath": material_path,
                "textures": [
                    {"name": "normal_tex", "texture": normal_path},
                    {"name": "refle_tex", "texture": reflection_path},
                    {"name": "detail_normal", "texture": duplicate_normal_path},
                ],
            }],
            "runtimeResourceBindings": [
                {"resolutionStatus": "RESOLVED_RUNTIME_ASSET",
                 "sourceObjectPath": normal_path,
                 "assetId": "Effect/Test/normal.dds"},
                {"resolutionStatus": "RESOLVED_RUNTIME_ASSET",
                 "sourceObjectPath": reflection_path,
                 "assetId": "Effect/Test/reflection.dds"},
                {"resolutionStatus": "RESOLVED_RUNTIME_ASSET",
                 "sourceObjectPath": duplicate_normal_path,
                 "assetId": "Effect/Test/detail-normal.dds"},
            ],
        }

        resources, receipt, _materials = choose_resources(
            system, {"required"}, graph
        )
        slots = {row["slotId"]: row["assetId"] for row in resources}
        self.assertEqual("Effect/Test/reflection.dds", slots["base"])
        self.assertNotIn("noise", slots)
        self.assertNotIn("mask", slots)
        self.assertNotIn("emissive", slots)
        self.assertTrue(any(
            row["parameterName"] == "normal_tex"
            and row["status"]
            == "FALLBACK_BLOCKED_UNRESOLVED_TEXTURE_ROLE"
            for row in receipt
        ))

    def test_reference_array_object_paths_keep_distinct_indices(self) -> None:
        index = SourceIndex({"nodes": [], "edges": []}, {"packages": []})
        module = SourceObject(
            key="test.meshmaterial",
            source_id="module",
            class_name="particlemodulemeshmaterial",
            object_path="test.particlemodulemeshmaterial_0",
            properties={"meshmaterials": value("arrayproperty", [-10, -20])},
            reference_paths=[
                ("meshmaterials", "fx_mi.first"),
                ("meshmaterials", "fx_mi.second"),
            ],
        )

        literals, _distributions = flatten_source_properties(index, module)
        by_path = {row["propertyPath"]: row["value"] for row in literals}

        self.assertEqual("fx_mi.first", by_path["meshmaterials[0].objectpath"])
        self.assertEqual("fx_mi.second", by_path["meshmaterials[1].objectpath"])
        self.assertEqual(len(literals), len(by_path))

    def test_local_vector_field_occurrence_keeps_source_identity_and_asset(self) -> None:
        graph = {
            "sourceSystems": [{
                "sourceSystemId": "test.vector.system",
                "rootNodeId": "system",
                "unresolvedExternalReferences": [{
                    "sourceNodeId": "vector-module",
                    "referenceIndex": 0,
                    "property": "vectorfield",
                    "packageIndex": -7,
                    "objectPath": "FX_O_W_01.FX_O_VectorField_01",
                }],
            }],
            "nodes": [
                {
                    "nodeId": "system", "package": "TEST", "exportIndex": 0,
                    "className": "particlesystem", "objectPath": "system",
                    "properties": {},
                },
                {
                    "nodeId": "emitter", "package": "TEST", "exportIndex": 1,
                    "className": "particlespriteemitter", "objectPath": "emitter",
                    "properties": {},
                },
                {
                    "nodeId": "lod", "package": "TEST", "exportIndex": 2,
                    "className": "particlelodlevel", "objectPath": "lod",
                    "properties": {},
                },
                {
                    "nodeId": "vector-module", "package": "TEST", "exportIndex": 3,
                    "className": "particlemodulelocalvectorfield",
                    "objectPath": "emitter.particlemodulelocalvectorfield_0",
                    "properties": {"vectorfield": value("objectproperty", -7)},
                },
            ],
            "edges": [
                {
                    "sourceNodeId": "system", "targetNodeId": "emitter",
                    "property": "emitters", "referenceIndex": 0,
                },
                {
                    "sourceNodeId": "emitter", "targetNodeId": "lod",
                    "property": "lodlevels", "referenceIndex": 0,
                },
                {
                    "sourceNodeId": "lod", "targetNodeId": "vector-module",
                    "property": "modules", "referenceIndex": 4,
                },
            ],
        }
        index = SourceIndex(graph, {"packages": []})
        partitions = selected_lod_partitions(graph, index, {"packages": []})
        self.assertEqual(1, len(partitions))
        module = partitions[0][3][0]

        literals, _distributions = flatten_source_properties(index, module, {
            "modules": {},
            "vectorFields": [{
                "sourceObjectPath": "fx_o_w_01.fx_o_vectorfield_01",
                "assetId": (
                    "Effect/DimensionMaster/VectorFields/"
                    "fx_o_w_01.2fdb8d454138eac6.wvectorfield"
                ),
            }],
        })
        by_path = {row["propertyPath"]: row for row in literals}
        self.assertEqual(
            "FX_O_W_01.FX_O_VectorField_01",
            by_path["vectorfield.objectpath"]["value"],
        )
        self.assertEqual(
            "Effect/DimensionMaster/VectorFields/"
            "fx_o_w_01.2fdb8d454138eac6.wvectorfield",
            by_path["vectorfield.assetid"]["value"],
        )

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

    def test_mesh_model_material_follows_typedata_override(self) -> None:
        override_module = SourceObject(
            key="mesh-override",
            source_id="mesh-override",
            class_name="ParticleModuleTypeDataMesh",
            object_path="FX_TEST.MeshOverride",
            properties={
                "boverridematerial": {"type": "boolean", "value": True},
            },
        )
        mappings = []
        self.assertFalse(mesh_uses_model_material([override_module], mappings))
        self.assertEqual("EXACT", mappings[0]["status"])
        self.assertIn("inverse", mappings[0]["note"])

        model_module = SourceObject(
            key="mesh-model",
            source_id="mesh-model",
            class_name="ParticleModuleTypeDataMesh",
            object_path="FX_TEST.MeshModel",
            properties={},
        )
        mappings = []
        self.assertTrue(mesh_uses_model_material([model_module], mappings))
        self.assertEqual("SOURCE_CLASS_DEFAULT", mappings[0]["status"])

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
                if "typedatalight" in type_class.casefold():
                    point_light = f"{prefix}:pointlight"
                    add_node(
                        point_light,
                        "pointlightcomponent",
                        f"{prefix}.pointlight",
                        {"brightness": value("floatproperty", 10.0)},
                    )
                    add_edge(
                        type_data,
                        "pointlightcomponent",
                        point_light,
                        prefix,
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
        add_system(
            "fx_post.fx_par.par_j_rgbnoise_01", None, "none", ""
        )
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
                    for index, name in enumerate((
                        "sprite_system",
                        "mesh_system",
                        "light_system",
                        "fx_post.fx_par.par_j_rgbnoise_01",
                    ))
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
        self.assertEqual(document["version"], 12)
        self.assertEqual(document["particleSystem"], {
            "uniformScaleMultiplier": 1.0,
            "yawOffsetDegrees": 0.0,
            "directionYawDegrees": 0.0,
            "initialSpeedMultiplier": 1.0,
        })
        self.assertEqual(document["modelCues"], [])
        self.assertIn("F 2050500", document["displayName"])
        self.assertEqual(conversion["summary"]["sourceEmitterPartitionCount"], 4)
        self.assertEqual(conversion["summary"]["graphEmitterPartitionCount"], 5)
        self.assertEqual(
            conversion["summary"]["inactiveSourceSystemEmitterPartitionCount"],
            1,
        )
        self.assertEqual(conversion["summary"]["convertedEmitterCount"], 4)
        self.assertEqual(conversion["summary"]["unsupportedEmitterCount"], 0)
        self.assertEqual(len(document["elements"]), 5)
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
        light = next(
            row for row in document["elements"]
            if row["groupId"] == "light_system"
        )
        screen_post = next(
            row for row in document["elements"]
            if row["groupId"] == "fx_post.fx_par.par_j_rgbnoise_01"
        )
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
        self.assertFalse(sprite["sourcePresentation"]["enabled"])
        self.assertEqual(
            sprite["sourcePresentation"]["schema"],
            "lostark.effect-source-presentation",
        )
        self.assertEqual(
            repeated_sprite["sourcePresentation"]["sourceOccurrenceIndex"],
            1,
        )
        self.assertEqual(
            repeated_sprite["sourcePresentation"]["sourceEventId"],
            "source-event-repeat",
        )
        self.assertEqual(mesh["kind"], "particle")
        self.assertTrue(any(row["slotId"] == "meshModel" for row in mesh["resources"]))
        self.assertFalse(mesh["detail"]["particle"]["billboard"])
        self.assertEqual(light["kind"], "light")
        self.assertTrue(light["detail"]["light"]["enabled"])
        self.assertEqual(light["detail"]["light"]["intensity"], 10.0)
        self.assertEqual(
            light["detail"]["light"]["profileId"],
            "light.point.reconstructed.v1",
        )
        self.assertEqual(light["sourcePresentation"]["status"], "unresolved")
        self.assertTrue(any(
            row["name"] == "radiusUeUnits"
            and row["status"] == "unresolved_class_default"
            for row in light["sourcePresentation"]["parameters"]
        ))
        self.assertEqual(screen_post["kind"], "screenPost")
        self.assertTrue(screen_post["detail"]["screenPost"]["enabled"])
        self.assertEqual(
            screen_post["detail"]["screenPost"]["profileId"],
            "screen.rgb-noise.reconstructed.v1",
        )
        self.assertEqual(
            screen_post["sourcePresentation"]["status"], "reconstructed"
        )
        sprite_conversion = next(
            row for row in conversion["elementConversions"]
            if row.get("sourceSystemId") == "sprite_system"
        )
        self.assertTrue(any(
            row["className"] == "particlemodulecameraoffset"
            for row in sprite_conversion["unrepresentedModules"]
        ))

    def test_screen_post_preserves_dynamic_parameter_provenance(self) -> None:
        detail = default_detail()
        recipe = {
            "modules": [{
                "className": "particlemoduleparameterdynamic",
                "objectPath": "FX_Post.ParameterDynamic_0",
                "literals": [
                    {
                        "propertyPath": "dynamicparams[0].paramname",
                        "value": "powerx",
                    },
                    {
                        "propertyPath": "dynamicparams[1].paramname",
                        "value": "rgb_str",
                    },
                ],
                "distributions": [
                    {
                        "propertyPath": "dynamicparams[0].paramvalue",
                        "lookupTable": [1.0, 5.0, 5.0, 1.0],
                    },
                    {
                        "propertyPath": "dynamicparams[1].paramvalue",
                        "lookupTable": [1.0, 1.0, 1.0, 1.0],
                    },
                ],
            }],
        }
        source = screen_post_presentation(
            "FX_Post.FX_Par.Par_J_RGBNoise_01",
            recipe,
            detail,
            {
                "sourceActionCueId": "cue-1",
                "eventId": "event-1",
                "globalTimeSeconds": 0.75,
            },
            "FX_Post.Emitter_0",
        )
        self.assertEqual(source["status"], "reconstructed")
        self.assertEqual(source["sourceActionCueId"], "cue-1")
        self.assertEqual(source["sourceTimeSeconds"], 0.75)
        self.assertEqual(detail["screenPost"]["intensity"], 1.0)
        self.assertEqual(detail["screenPost"]["secondaryIntensity"], 0.0)
        by_name = {row["name"]: row for row in source["parameters"]}
        self.assertEqual(by_name["powerx"]["status"], "source_distribution")
        self.assertEqual(by_name["powerx"]["numberValue"], 5.0)

    def test_film_noise_without_source_gain_is_fail_closed(self) -> None:
        detail = default_detail()
        source = screen_post_presentation(
            "FX_Post.FX_Par.Par_J_FilmNoise_01",
            {"modules": []},
            detail,
            {
                "sourceActionCueId": "cue-film",
                "eventId": "event-film",
                "globalTimeSeconds": 1.0,
            },
            "FX_Post.FilmNoiseEmitter_0",
        )
        self.assertFalse(source["enabled"])
        self.assertEqual(source["status"], "unresolved")
        self.assertFalse(detail["screenPost"]["enabled"])
        self.assertEqual(detail["screenPost"]["intensity"], 0.0)
        self.assertEqual(detail["screenPost"]["secondaryIntensity"], 0.0)

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
        self.assertEqual(promoted["version"], 12)
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
