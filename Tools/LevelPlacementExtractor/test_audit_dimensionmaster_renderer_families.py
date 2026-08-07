#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import json
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest


MODULE_PATH = Path(__file__).with_name(
    "audit_dimensionmaster_renderer_families.py"
)
SPEC = importlib.util.spec_from_file_location(
    "audit_dimensionmaster_renderer_families", MODULE_PATH
)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


def resource(slot: str, asset: str) -> dict[str, str]:
    return {"slotId": slot, "assetId": asset}


def mesh_element(
    element_id: str,
    model: str,
    start: float,
    use_model_material: bool,
    override: bool | None,
) -> dict:
    literals = []
    if override is not None:
        literals.append({
            "propertyPath": "bOverrideMaterial",
            "kind": "boolean",
            "value": override,
        })
    return {
        "id": element_id,
        "kind": "particle",
        "resources": [resource("meshModel", model)],
        "material": {"sourceProfile": {
            "runtimeShaderProfileId": "effect.ue3.grouped-translucent.v1",
        }},
        "detail": {
            "mesh": {"useModelMaterial": use_model_material},
            "timing": {"startDelaySeconds": start},
            "transform": {
                "position": [start, 0.0, 0.0],
                "rotationDegrees": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        },
        "sourceRecipe": {
            "rendererShape": "mesh",
            "modules": [
                {
                    "className": "ParticleModuleTypeDataMesh",
                    "literals": literals,
                    "distributions": [],
                },
                {
                    "className": "ParticleModuleMeshRotation",
                    "literals": [],
                    "distributions": [{
                        "propertyPath": "StartRotation",
                        "componentCount": 3,
                        "operation": 1,
                        "randomLockAxes": 0,
                        "lookupTable": [-0.3, 0.5, 0.0],
                        "keys": [],
                    }],
                },
            ],
        },
    }


def sprite_element(element_id: str, profile: str, with_base: bool) -> dict:
    return {
        "id": element_id,
        "kind": "particle",
        "resources": (
            [resource("base", "Effect/Test/base.dds")] if with_base else []
        ),
        "material": {"sourceProfile": {
            "runtimeShaderProfileId": profile,
        }},
        "detail": {
            "mesh": {"useModelMaterial": False},
            "timing": {"startDelaySeconds": 0.0},
            "transform": {
                "position": [0.0, 0.0, 0.0],
                "rotationDegrees": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        },
        "sourceRecipe": {"rendererShape": "sprite", "modules": []},
    }


class DimensionMasterRendererFamilyAuditTests(unittest.TestCase):
    def test_classifies_mesh_sprite_and_bound_skeletal_cue(self) -> None:
        with TemporaryDirectory() as temporary:
            root = Path(temporary)
            authored = root / "Authored"
            converted = root / "Converted"
            resources = root / "Resources"
            authored.mkdir()
            converted.mkdir()
            (resources / "Effect/Test").mkdir(parents=True)
            (resources / "Character/Test").mkdir(parents=True)
            (resources / "Effect/Test/swing.wmodel").write_bytes(b"mesh")
            (resources / "Effect/Test/model.wmodel").write_bytes(b"model")
            (resources / "Effect/Test/base.dds").write_bytes(b"texture")
            (resources / "Character/Test/summon.wmodel").write_bytes(b"summon")

            effect_asset_id = "effect.dimensionmaster.skill.10"
            document = {
                "schema": "lostark.effect-authoring",
                "version": 12,
                "effectAssetId": effect_asset_id,
                "modelCues": [{
                    "cueId": "summon",
                    "modelAssetId": "Character/Test/summon.wmodel",
                    "clipName": "summon_clip",
                }],
                "elements": [
                    mesh_element(
                        "mesh.base", "Effect/Test/swing.wmodel", 0.25,
                        False, True,
                    ),
                    mesh_element(
                        "mesh.base.event_source-event-1",
                        "Effect/Test/swing.wmodel", 0.6, False, True,
                    ),
                    mesh_element(
                        "mesh.model", "Effect/Test/model.wmodel", 0.0,
                        True, None,
                    ),
                    sprite_element(
                        "sprite.texture", "effect.ue3.grouped-translucent.v1",
                        True,
                    ),
                    sprite_element(
                        "sprite.procedural",
                        "effect.ue3.procedural-center-glow.v1", False,
                    ),
                    sprite_element(
                        "sprite.blocked", "effect.ue3.fallback-blocked.v1",
                        False,
                    ),
                    {"id": "light", "kind": "light"},
                    {"id": "post", "kind": "screenpost"},
                    {"id": "decal", "kind": "decal"},
                ],
            }
            (authored / f"{effect_asset_id}.effect.json").write_text(
                json.dumps(document), encoding="utf-8"
            )
            recipe = {
                "schema": "lostark.effect-action-cue-recipe",
                "formatVersion": 2,
                "skillId": 10,
                "cues": [{
                    "cueId": "source-summon",
                    "sourceType": "PlaySkeletalMesh",
                    "globalTimeSeconds": 0.0,
                    "sourceExecutionStatus": "RUNTIME_MODEL_BINDING_REQUIRED",
                    "typedPayload": {
                        "sourceCueName": "SK_Summon",
                        "sourceSkeletalMesh": "Pkg.Mesh.Summon_SK",
                        "sourceAnimSet": "Pkg.Ani.Summon_Ani",
                        "sourceMaterialInstances": ["Pkg.Mat.Summon_MI"],
                    },
                }],
            }
            (converted / "skill.10.action-cue-recipe.json").write_text(
                json.dumps(recipe), encoding="utf-8"
            )
            bindings_path = root / "bindings.json"
            bindings_path.write_text(json.dumps({
                "schema": "lostark.effect-model-cue-runtime-bindings",
                "formatVersion": 1,
                "bindings": [{
                    "skillId": 10,
                    "source": {
                        "cueName": "SK_Summon",
                        "skeletalMesh": "Pkg.Mesh.Summon_SK",
                        "animSet": "Pkg.Ani.Summon_Ani",
                    },
                    "runtime": {
                        "cueId": "summon",
                        "modelAssetId": "Character/Test/summon.wmodel",
                        "clipName": "summon_clip",
                        "assetPreTransform": {},
                    },
                }],
            }), encoding="utf-8")

            result = MODULE.build_renderer_family_audit(
                [{
                    "skillId": 10,
                    "inputSlot": "Q",
                    "effectAssetId": effect_asset_id,
                }],
                authored,
                converted,
                bindings_path,
                resources,
            )

            self.assertEqual(3, result["summary"]["staticMeshLayerCount"])
            self.assertEqual(3, result["summary"]["spriteLayerCount"])
            self.assertEqual(
                2,
                result["summary"]["staticMeshMaterialOverrideCarrierCount"],
            )
            self.assertEqual(
                1,
                result["summary"]["staticMeshEmbeddedMaterialCandidateCount"],
            )
            self.assertEqual(1, result["summary"]["boundSkeletalModelCueCount"])
            self.assertEqual(1, result["summary"]["lightCount"])
            self.assertEqual(1, result["summary"]["screenPostCount"])
            self.assertEqual(1, result["summary"]["decalCount"])
            skill = result["skills"][0]
            self.assertEqual(1, skill["summary"]["repeatedStaticMeshGroupCount"])
            repeated = skill["repeatedStaticMeshGroups"][0]
            self.assertEqual([0.25, 0.6], [
                row["startDelaySeconds"] for row in repeated["instances"]
            ])
            self.assertEqual(
                "BOUND_SKELETAL_MODEL_CUE",
                skill["skeletalModelCues"][0]["classification"],
            )

    def test_rejects_mesh_shape_without_mesh_binding(self) -> None:
        element = sprite_element(
            "broken", "effect.ue3.grouped-translucent.v1", True
        )
        element["sourceRecipe"]["rendererShape"] = "mesh"
        with self.assertRaisesRegex(ValueError, "shape and meshModel"):
            MODULE.classify_cascade_layer(element)

    def test_isolates_override_and_model_material_contradiction(self) -> None:
        element = mesh_element(
            "broken", "Effect/Test/mesh.wmodel", 0.0, True, True
        )
        self.assertEqual(
            "STATIC_MESH_CONTRACT_CONTRADICTION",
            MODULE.classify_cascade_layer(element),
        )


if __name__ == "__main__":
    unittest.main()
