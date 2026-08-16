#!/usr/bin/env python3
"""Focused contract tests for the complete four-class strict Track A surface."""

from __future__ import annotations

import base64
import copy
import hashlib
import importlib.util
import json
import os
import sys
import tempfile
import unittest
from collections import Counter
from pathlib import Path
from unittest import mock


SCRIPT_PATH = Path(__file__).with_name(
    "materialize_four_class_track_a_candidates.py"
)
sys.path.insert(0, str(SCRIPT_PATH.parent))
SPEC = importlib.util.spec_from_file_location("four_class_track_a", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class AuthoringApproximateAdmissionTests(unittest.TestCase):
    @staticmethod
    def _literal(property_path: str, value: object) -> dict[str, object]:
        return {"propertyPath": property_path, "value": value}

    @classmethod
    def _generator(cls, name: str, event_type: str = "epet_spawn") -> dict:
        return {
            "className": "particlemoduleeventgenerator",
            "literals": [
                cls._literal("events[0].type", event_type),
                cls._literal("events[0].customname", name),
            ],
        }

    @classmethod
    def _receiver(cls, name: str, event_type: str = "epet_spawn") -> dict:
        return {
            "className": "particlemoduleeventreceiverspawn",
            "literals": [
                cls._literal("eventgeneratortype", event_type),
                cls._literal("eventname", name),
            ],
        }

    @staticmethod
    def _element(
        element_id: str,
        modules: list[dict] | None = None,
        *,
        visible: bool = True,
        execution: dict | None = None,
    ) -> dict:
        material: dict[str, object] = {"templateId": "effect.source_material"}
        if execution is not None:
            material["execution"] = copy.deepcopy(execution)
        return {
            "id": element_id,
            "kind": "particle",
            "visible": visible,
            "material": material,
            "sourceRecipe": {
                "enabled": True,
                "modules": copy.deepcopy(modules or []),
            },
        }

    def test_approximate_eligibility_requires_profile_resources_and_modules(
        self,
    ) -> None:
        self.assertTrue(
            MODULE.authoring_approximate_admitted(
                ["NON_EXACT_NAMED_TEXTURE_ALIAS"],
                "sprite",
                [{"slotId": "base", "assetId": "Effect/exact.dds"}],
                True,
                MODULE.GROUPED_TRANSLUCENT_PROFILE,
            )
        )
        self.assertFalse(
            MODULE.authoring_approximate_admitted(
                ["SOURCE_PROFILE_NOT_COMPILED"],
                "sprite",
                [{"slotId": "base", "assetId": "Effect/exact.dds"}],
                True,
                MODULE.GROUPED_TRANSLUCENT_PROFILE,
            ),
            "an invalid/uncompiled profile must stay hard fail-closed",
        )
        self.assertFalse(
            MODULE.authoring_approximate_admitted(
                ["NON_EXACT_NAMED_TEXTURE_ALIAS"], "sprite", [], True,
                MODULE.GROUPED_TRANSLUCENT_PROFILE,
            ),
            "a texture-less carrier is not an approximate drawable",
        )
        self.assertFalse(
            MODULE.authoring_approximate_admitted(
                ["NON_EXACT_NAMED_TEXTURE_ALIAS"],
                "mesh",
                [{"slotId": "base", "assetId": "Effect/exact.dds"}],
                True,
                MODULE.GROUPED_TRANSLUCENT_PROFILE,
            ),
            "a mesh carrier still requires its exact WModel",
        )
        self.assertFalse(
            MODULE.authoring_approximate_admitted(
                ["SOURCE_PROFILE_GROUPED_RESOURCE_CONTRACT"],
                "sprite",
                [{"slotId": "base", "assetId": "Effect/exact.dds"}],
                True,
                MODULE.GROUPED_TRANSLUCENT_PROFILE,
            ),
            "an invalid profile resource contract cannot use the approximate gate",
        )
        self.assertFalse(
            MODULE.authoring_approximate_admitted(
                ["UNSUPPORTED_ORDINARY_RECIPE:unsupported module"],
                "sprite",
                [{"slotId": "base", "assetId": "Effect/exact.dds"}],
                True,
                MODULE.GROUPED_TRANSLUCENT_PROFILE,
            ),
            "an unsupported portable module cannot use the approximate gate",
        )
        self.assertFalse(
            MODULE.authoring_approximate_admitted(
                ["NON_EXACT_NAMED_TEXTURE_ALIAS"],
                "sprite",
                [{"slotId": "base", "assetId": "Effect/exact.dds"}],
                True,
                "effect.ue3.circle.v1",
            ),
            "a finite non-grouped profile cannot use the generic approximate gate",
        )

    def test_approximate_event_route_is_validated_and_closed(self) -> None:
        execution = copy.deepcopy(MODULE.AUTHORING_APPROXIMATE_EXECUTION)
        elements = [
            self._element(
                "authored.approx.generator",
                [self._generator("spawn_a")],
                execution=execution,
            ),
            self._element(
                "authored.approx.receiver",
                [self._receiver("spawn_a")],
                execution=execution,
            ),
        ]
        self.assertEqual(MODULE.validate_portable_event_route_closure(elements), 1)

        with self.assertRaisesRegex(
            MODULE.RestorationError, "same-document generator/receiver closure"
        ):
            MODULE.validate_portable_event_route_closure(elements[:1])

        invalid_module = copy.deepcopy(elements)
        invalid_module[0]["sourceRecipe"]["modules"][0] = self._generator(
            "spawn_a", "epet_death"
        )
        with self.assertRaisesRegex(
            MODULE.RestorationError, "event generator identity is invalid"
        ):
            MODULE.validate_portable_event_route_closure(invalid_module)

    def test_approximate_event_route_cycle_is_rejected(self) -> None:
        execution = copy.deepcopy(MODULE.AUTHORING_APPROXIMATE_EXECUTION)
        elements = [
            self._element(
                "authored.approx.a",
                [self._generator("a_to_b"), self._receiver("b_to_a")],
                execution=execution,
            ),
            self._element(
                "authored.approx.b",
                [self._receiver("a_to_b"), self._generator("b_to_a")],
                execution=execution,
            ),
        ]
        with self.assertRaisesRegex(MODULE.RestorationError, "cycle"):
            MODULE.validate_portable_event_route_closure(elements)

    def test_hard_fail_closed_is_hidden_evidence_not_an_activation(self) -> None:
        element = self._element(
            "authored.hard-locked",
            [self._generator("unmatched_is_ignored")],
            visible=False,
            execution=copy.deepcopy(MODULE.FAIL_CLOSED_EXECUTION),
        )
        self.assertFalse(MODULE.is_authoring_execution_target(element))
        self.assertEqual(MODULE.validate_portable_event_route_closure([element]), 0)

        element["visible"] = True
        with self.assertRaisesRegex(MODULE.RestorationError, "cannot be activated"):
            MODULE.is_authoring_execution_target(element)

    def test_invalid_approximate_execution_flag_combinations_are_rejected(
        self,
    ) -> None:
        valid_hidden = self._element(
            "authored.approx.hidden",
            visible=False,
            execution=copy.deepcopy(MODULE.AUTHORING_APPROXIMATE_EXECUTION),
        )
        self.assertTrue(MODULE.is_authoring_execution_target(valid_hidden))

        invalid_rows = [
            {
                "enabled": False,
                "failClosed": False,
                "authoringApproximate": True,
            },
            {
                "enabled": True,
                "failClosed": True,
                "authoringApproximate": True,
            },
            {
                "enabled": False,
                "failClosed": True,
                "authoringApproximate": False,
            },
        ]
        for index, execution in enumerate(invalid_rows):
            with self.subTest(index=index), self.assertRaises(
                MODULE.RestorationError
            ):
                MODULE.is_authoring_execution_target(
                    self._element(
                        f"authored.approx.invalid-{index}", execution=execution
                    )
                )


class DynamicParameterArithmeticBoundaryTests(unittest.TestCase):
    def test_cooked_partial_grouped_dissolve_is_unbound_by_exact_parent_evidence(
        self,
    ) -> None:
        identity = {
            "sourceMaterialPath": (
                "fx_m_mi_m_00.fx_mi.fx_m_pa_spritewave_01_7_tr"
            ),
            "sourcePhysicalPackage": "ZHJ4TC4PCK4PC4J22HIXEYUXEU.upk",
            "parentMaterialPath": "fx_m.fx_m_pa_spritewave_01_tr",
            "parentSourcePhysicalPackage": "ZHJ4TC4PCK4PC4J22HIXEYUXEU.upk",
            "profileId": "ue3.material.spritewave",
            "runtimeShaderProfileId": MODULE.GROUPED_TRANSLUCENT_PROFILE,
            "sourceParameters": {
                "textures": [], "scalars": [], "vectors": [],
                "staticSwitches": [],
            },
        }
        source_element = {
            "sourceRecipe": {
                "modules": [{
                    "className": "ParticleModuleParameterDynamic",
                    "literals": [{
                        "propertyPath": "DynamicParams[1].ParamName",
                        "value": "dissolve",
                    }],
                    # A normalized value is deliberately still downgraded: the
                    # lost parent arithmetic, not the observed range, is the
                    # exactness boundary.
                    "distributions": [{
                        "propertyPath": "DynamicParams[1].ParamValue",
                        "lookupTable": [0.5, 0.75],
                    }],
                }]
            }
        }
        profile = MODULE.material_contract_profile(identity, source_element)
        self.assertTrue(
            MODULE.source_dynamic_parameter_arithmetic_unavailable(identity)
        )
        self.assertEqual("unbound", profile["dynamicParameterSemantics"][1])
        self.assertTrue(
            MODULE.authoring_approximate_admitted(
                [MODULE.SOURCE_DYNAMIC_PARAMETER_ARITHMETIC_UNAVAILABLE],
                "sprite",
                [{"slotId": "base", "assetId": "Effect/exact.dds"}],
                True,
                MODULE.GROUPED_TRANSLUCENT_PROFILE,
            )
        )

        runtime_exact = copy.deepcopy(identity)
        runtime_exact["cookedGraphTopologyStatus"] = "COMPLETE"
        runtime_exact["cookedGraphRuntimeExactEligible"] = True
        self.assertEqual(
            "dissolve",
            MODULE.material_contract_profile(
                runtime_exact, source_element
            )["dynamicParameterSemantics"][1],
        )

        wrong_package = copy.deepcopy(identity)
        wrong_package["parentSourcePhysicalPackage"] = "OTHER.upk"
        self.assertEqual(
            "dissolve",
            MODULE.material_contract_profile(
                wrong_package, source_element
            )["dynamicParameterSemantics"][1],
        )

        non_grouped = copy.deepcopy(identity)
        non_grouped["runtimeShaderProfileId"] = "effect.ue3.circle.v1"
        self.assertEqual(
            "dissolve",
            MODULE.material_contract_profile(
                non_grouped, source_element
            )["dynamicParameterSemantics"][1],
        )


class StrictTypedMaterialProfileTests(unittest.TestCase):
    @staticmethod
    def _dynamic_source(names: list[str]) -> dict:
        return {
            "sourceRecipe": {
                "rendererShape": "mesh",
                "modules": [
                    {
                        "className": "ParticleModuleParameterDynamic",
                        "literals": [
                            {
                                "propertyPath": (
                                    f"DynamicParams[{index}].ParamName"
                                ),
                                "value": name,
                            }
                            for index, name in enumerate(names)
                        ],
                    }
                ],
            }
        }

    @staticmethod
    def _contract(
        identity: dict[str, str], parameters: dict[str, list[dict]]
    ) -> dict:
        return {
            **identity,
            "sourceEvidenceResolved": True,
            "productAdmissionStatus": "ADMITTED_RECONSTRUCTED_PROFILE",
            "runtimeShaderProfileId": MODULE.GROUPED_TRANSLUCENT_PROFILE,
            "sourceParameters": parameters,
        }

    @staticmethod
    def _material(identity: dict[str, str]) -> dict:
        return {
            "templateId": "effect.source_material",
            "sourceMaterialPath": identity["sourceMaterialPath"],
            "renderProfile": "alpha_two_sided_depth_read",
            "execution": {"failClosed": False, "sentinel": "unchanged"},
            "sourceProfile": {
                "enabled": True,
                "profileId": identity["profileId"],
                "runtimeShaderProfileId": MODULE.GROUPED_TRANSLUCENT_PROFILE,
                "parentMaterialPath": identity["parentMaterialPath"],
                "semanticStatus": "reconstructed_profile",
                "textures": [],
                "scalars": [],
                "vectors": [],
                "staticSwitches": [],
                "dynamicParameterSemantics": [
                    "uv_pan", "uv_pan", "uv_pan", "dissolve"
                ],
                "subUVMode": "none",
            },
        }

    def test_lance_exact_two_emissive_profile_preserves_chroma_inputs(
        self,
    ) -> None:
        identity = MODULE.STRICT_MISSILETRAIL_IDENTITY
        assets = MODULE.STRICT_MISSILETRAIL_RESOURCES
        texture_assets = {
            "alpha_tex": assets["mask"],
            "emissive_tex01": assets["base"],
            "emissive_tex02": assets["emissive"],
            "uv_dissolve_tex": assets["dissolve"],
            "uv_noise_tex": assets["noise"],
        }
        contract = self._contract(
            identity,
            {
                "textures": [
                    {
                        "name": name,
                        "sourceObjectPath": f"fx_tex.{name}",
                        "assetId": asset_id,
                    }
                    for name, asset_id in texture_assets.items()
                ],
                "scalars": [
                    {"name": "emissive_tex_strength", "value": 5000.0},
                    {"name": "emissive_tex_power", "value": 2.0},
                    {"name": "uvnoise_tex_01_texcoord_x", "value": 1.5},
                    {"name": "uvnoise_tex_01_texcoord_y", "value": 2.5},
                ],
                "vectors": [],
                "staticSwitches": [],
            },
        )
        source = self._dynamic_source(
            ["alpha_pan", "uv_noise_velue", "uv_noise_pan", "alpha_dissolve"]
        )
        material = self._material(identity)
        result = MODULE.apply_strict_typed_material_profile(
            material,
            contract,
            source,
            [
                {"slotId": slot, "assetId": asset_id}
                for slot, asset_id in assets.items()
            ],
        )
        self.assertEqual(result, MODULE.MISSILETRAIL_PROFILE)
        profile = material["sourceProfile"]
        self.assertEqual(
            profile["dynamicParameterSemantics"],
            [
                "missile_alpha_pan",
                "missile_noise_strength",
                "missile_noise_pan",
                "missile_dissolve",
            ],
        )
        self.assertEqual(
            {row["name"] for row in profile["textures"]},
            set(texture_assets),
        )
        scalars = {row["name"]: row["value"] for row in profile["scalars"]}
        self.assertEqual(scalars["emissive_tex_strength"], 5000.0)
        self.assertEqual(scalars["emissive_tex_power"], 2.0)
        self.assertEqual(material["execution"], {
            "failClosed": False, "sentinel": "unchanged"
        })
        resource_rows = [
            {"slotId": slot, "assetId": asset_id}
            for slot, asset_id in assets.items()
        ]
        self.assertEqual(
            MODULE.source_profile_readiness(material, resource_rows, "mesh")[:2],
            (True, True),
        )
        without_second_emissive = [
            row for row in resource_rows if row["slotId"] != "emissive"
        ]
        self.assertEqual(
            MODULE.source_profile_readiness(
                material, without_second_emissive, "mesh"
            )[:2],
            (False, True),
        )
        missing_named_lane = copy.deepcopy(material)
        missing_named_lane["sourceProfile"]["textures"] = [
            row
            for row in missing_named_lane["sourceProfile"]["textures"]
            if row["name"] != "emissive_tex02"
        ]
        self.assertEqual(
            MODULE.source_profile_readiness(
                missing_named_lane, resource_rows, "mesh"
            )[:2],
            (False, True),
        )

        # One shared exposure denominator preserves component ratios before
        # ParticleColor applies the source blue tint; per-channel saturation
        # would collapse this witness to white.
        lane = [0.8 * 0.9, 0.6 * 0.8, 0.4 * 0.7]
        radiance = [value**2.0 * 5000.0 for value in lane]
        denominator = 1.0 + max(radiance)
        particle_color = [0.2, 0.375, 0.5]
        output = [
            value / denominator * tint
            for value, tint in zip(radiance, particle_color)
        ]
        self.assertGreater(max(output) - min(output), 0.02)
        self.assertNotEqual(output[0], output[1])
        self.assertNotEqual(output[1], output[2])

    def test_watertrail_restores_only_named_main_noise_and_direct_dissolve(
        self,
    ) -> None:
        identity = MODULE.STRICT_WATERTRAIL_IDENTITY
        assets = MODULE.STRICT_WATERTRAIL_RESOURCES
        contract = self._contract(
            identity,
            {
                "textures": [
                    {
                        "name": "maintex",
                        "sourceObjectPath": "fx_tex_04.fx_h_wave_01",
                        "assetId": assets["base"],
                    },
                    {
                        "name": "uv_noise_tex",
                        "sourceObjectPath": "fx_tex_00.fx_a_noise_011",
                        "assetId": assets["noise"],
                    },
                    {
                        "name": "umodel_dependency",
                        "sourceObjectPath": "fx_tex_05.fx_m_fluid_004",
                        "assetId": "",
                    },
                ],
                "scalars": [
                    {"name": "main_tex_power", "value": 2.0},
                    {"name": "main_tex_power_multiply", "value": 0.01},
                    {"name": "maintex_desaturation", "value": 0.2},
                ],
                "vectors": [
                    {"name": "reflection_color", "value": [4.0, 3.0, 2.0, 1.0]}
                ],
                "staticSwitches": [],
            },
        )
        source = self._dynamic_source(
            ["alpha_pan", "uv_noise_pan", "dissolve", "noise_velue"]
        )
        material = self._material(identity)
        result = MODULE.apply_strict_typed_material_profile(
            material,
            contract,
            source,
            [
                {"slotId": slot, "assetId": asset_id}
                for slot, asset_id in assets.items()
            ],
        )
        self.assertEqual(result, MODULE.WATERTRAIL_PROFILE)
        profile = material["sourceProfile"]
        self.assertEqual(
            profile["dynamicParameterSemantics"],
            [
                "water_alpha_pan",
                "water_noise_pan",
                "water_dissolve",
                "water_noise_strength",
            ],
        )
        self.assertEqual(
            {row["name"] for row in profile["textures"]},
            MODULE.WATERTRAIL_NAMED_TEXTURES,
        )
        self.assertNotIn("emissive", {
            row["name"] for row in profile["textures"]
        })
        self.assertEqual(
            profile["vectors"][0]["value"], [4.0, 3.0, 2.0, 1.0]
        )
        self.assertEqual(material["execution"]["sentinel"], "unchanged")
        water_resource_rows = [
            {"slotId": slot, "assetId": asset_id}
            for slot, asset_id in assets.items()
        ]
        self.assertEqual(
            MODULE.source_profile_readiness(
                material, water_resource_rows, "mesh"
            )[:2],
            (True, True),
        )
        water_missing_lane = copy.deepcopy(material)
        water_missing_lane["sourceProfile"]["textures"] = [
            row
            for row in water_missing_lane["sourceProfile"]["textures"]
            if row["name"] != "uv_noise_tex"
        ]
        self.assertEqual(
            MODULE.source_profile_readiness(
                water_missing_lane, water_resource_rows, "mesh"
            )[:2],
            (False, True),
        )

        mismatched = self._material(identity)
        wrong_resources = [
            {"slotId": slot, "assetId": asset_id}
            for slot, asset_id in assets.items()
        ]
        wrong_resources[1]["assetId"] = "Effect/wrong.dds"
        self.assertEqual(
            MODULE.apply_strict_typed_material_profile(
                mismatched, contract, source, wrong_resources
            ),
            "",
        )
        self.assertEqual(
            mismatched["sourceProfile"]["runtimeShaderProfileId"],
            MODULE.GROUPED_TRANSLUCENT_PROFILE,
        )


class Warlord17090MeshProjectionTests(unittest.TestCase):
    @staticmethod
    def _canonical_material(source_element_id: str) -> dict:
        path = MODULE.ROOT / (
            "Data/Effects/Authored/"
            "effect.warlord.skill.17090.unified.effect.json"
        )
        document = json.loads(path.read_text(encoding="utf-8"))
        matches = [
            row
            for row in document["elements"]
            if str(row.get("sourceNode") or "").endswith(
                "|element:" + source_element_id
            )
        ]
        if len(matches) != 1:
            raise AssertionError(
                f"Warlord canonical material match count is {len(matches)}"
            )
        return copy.deepcopy(matches[0]["material"])

    @staticmethod
    def _recipe(literals: list[dict], modules: int = 1) -> dict:
        return {
            "rendererShape": "mesh",
            "modules": [
                {
                    "className": "particlemoduletypedatamesh",
                    "literals": copy.deepcopy(literals),
                }
                for _ in range(modules)
            ],
        }

    def test_typedata_rotation_literal_contract_and_order(self) -> None:
        literals = [
            {"propertyPath": "pitch", "kind": "number", "value": 45.0},
            {"propertyPath": "roll", "kind": "number", "value": 90.0},
        ]
        self.assertEqual(
            MODULE.source_type_data_mesh_rotation_degrees(
                self._recipe(literals)
            ),
            [90.0, 45.0, 0.0],
        )
        invalid = [
            self._recipe(literals, modules=0),
            self._recipe(literals, modules=2),
            self._recipe(literals + [copy.deepcopy(literals[0])]),
            self._recipe([
                {"propertyPath": "roll", "kind": "string", "value": 1.0}
            ]),
            self._recipe([
                {"propertyPath": "roll", "kind": "number", "value": float("nan")}
            ]),
        ]
        for recipe in invalid:
            with self.subTest(recipe=recipe), self.assertRaises(
                MODULE.RestorationError
            ):
                MODULE.source_type_data_mesh_rotation_degrees(recipe)

    def test_exact_mesh14_chain12_boundary_and_two_pass_stability(self) -> None:
        path = MODULE.ROOT / (
            "Data/Effects/Imported/Warlord/CurrentCombat/Converted/"
            "effect.warlord.skill.17090.imported.effect.json"
        )
        document = json.loads(path.read_text(encoding="utf-8"))
        meshes = [
            row
            for row in document["elements"]
            if row.get("sourceRecipe", {}).get("rendererShape") == "mesh"
        ]
        self.assertEqual(len(meshes), 14)
        chain_models: Counter[str] = Counter()
        chain_count = 0
        for index, element in enumerate(meshes):
            rotation = MODULE.source_type_data_mesh_rotation_degrees(
                element["sourceRecipe"]
            )
            detail = copy.deepcopy(element["detail"])
            detail["artistSentinel"] = index
            first_changed = MODULE.reassert_warlord_mesh_compiler_fields(
                detail, rotation, False
            )
            first = copy.deepcopy(detail)
            second_changed = MODULE.reassert_warlord_mesh_compiler_fields(
                detail, rotation, False
            )
            self.assertTrue(first_changed)
            self.assertFalse(second_changed)
            self.assertEqual(detail, first)
            self.assertEqual(detail["artistSentinel"], index)

            source_resources = element["resources"]
            assignment = MODULE.SourceParticleAssignment(
                "WARLORD", 17090, None, str(element["id"]),
                "source-event", index, f"target-{index}",
                MODULE.WARLORD_17090_TARGET_EFFECT_ID, 0.0,
            )
            if not MODULE.is_warlord_17090_chain_source(
                assignment, element, source_resources
            ):
                continue
            chain_count += 1
            location_modules = [
                row
                for row in element["sourceRecipe"]["modules"]
                if row.get("className") == "particlemodulelocationdirect"
            ]
            self.assertEqual(len(location_modules), 1)
            location_distributions = {
                row["propertyPath"]: row
                for row in location_modules[0]["distributions"]
            }
            self.assertEqual(
                location_distributions["location"]["lookupTable"],
                [-30.0, 35.0, 0.0, 35.0, 0.0, -20.0, -30.0, 0.0],
            )
            scale_factor = location_distributions["scalefactor"]
            self.assertEqual(scale_factor["componentCount"], 3)
            self.assertEqual(scale_factor["operation"], 1)
            self.assertEqual(scale_factor["sourceClass"], "")
            self.assertEqual(scale_factor["sourceObjectPath"], "")
            self.assertEqual(scale_factor["defaultMinimum"], [0.0] * 4)
            self.assertEqual(scale_factor["defaultMaximum"], [0.0] * 4)
            self.assertEqual(scale_factor["lookupTable"], [])
            self.assertEqual(scale_factor["keys"], [])
            self.assertEqual(
                {row["slotId"] for row in source_resources}, {"meshModel"}
            )
            model = source_resources[0]["assetId"]
            chain_models[model] += 1
            material = self._canonical_material(str(element["id"]))
            preview_resources = copy.deepcopy(source_resources)
            MODULE.apply_warlord_chain_preview_boundary(
                material, detail, preview_resources
            )
            first_preview = copy.deepcopy(preview_resources)
            MODULE.apply_warlord_chain_preview_boundary(
                material, detail, preview_resources
            )
            self.assertEqual(preview_resources, first_preview)
            self.assertEqual(material["templateId"], "effect.source_material")
            self.assertTrue(material["sourceProfile"]["enabled"])
            self.assertEqual(
                material["sourceProfile"]["runtimeShaderProfileId"],
                MODULE.GROUPED_TRANSLUCENT_PROFILE,
            )
            self.assertFalse(detail["mesh"]["useModelMaterial"])
            self.assertEqual(
                preview_resources,
                [
                    {"slotId": "meshModel", "assetId": model},
                    {
                        "slotId": "base",
                        "assetId": (
                            MODULE.WARLORD_CHAIN_PREVIEW_BASE_ALIAS_ASSET_ID
                        ),
                    },
                ],
            )
            self.assertEqual(
                MODULE.AUTHORING_APPROXIMATE_EXECUTION,
                {
                    "enabled": False,
                    "failClosed": True,
                    "authoringApproximate": True,
                },
            )
        self.assertEqual(chain_count, 12)
        self.assertEqual(
            sorted(chain_models.values()), [4, 8]
        )
        self.assertEqual(
            MODULE.WARLORD_CHAIN_APPROXIMATION_REASON,
            "SOURCE_MASKED_WPO_ARITHMETIC_UNAVAILABLE",
        )
        self.assertEqual(
            MODULE.strict_effect_resource_kind(
                MODULE.WARLORD_CHAIN_PREVIEW_BASE_ALIAS_ASSET_ID,
                "Warlord chain non-exact preview Base",
            ),
            "texture",
        )

    def test_preview_alias_path_kind_and_profile_fail_closed(self) -> None:
        imported_path = MODULE.ROOT / (
            "Data/Effects/Imported/Warlord/CurrentCombat/Converted/"
            "effect.warlord.skill.17090.imported.effect.json"
        )
        document = json.loads(imported_path.read_text(encoding="utf-8"))
        element = next(
            row
            for row in document["elements"]
            if row.get("material", {}).get("sourceMaterialPath")
            == MODULE.WARLORD_CHAIN_SOURCE_MATERIAL_PATH
        )
        material = self._canonical_material(str(element["id"]))
        detail = copy.deepcopy(element["detail"])
        source_resources = copy.deepcopy(element["resources"])

        invalid_asset_ids = (
            "../fx_d_atypical_028.dds",
            "Effect/Warlord/Textures/FX_TEX_02/missing.dds",
            "Effect/Warlord/Textures/FX_TEX_02/fx_d_atypical_028.wmodel",
        )
        with tempfile.TemporaryDirectory() as temporary:
            temporary_root = Path(temporary)
            wrong_kind = temporary_root / invalid_asset_ids[2]
            wrong_kind.parent.mkdir(parents=True, exist_ok=True)
            wrong_kind.write_bytes(b"not-a-texture")
            for asset_id in invalid_asset_ids:
                with (
                    self.subTest(asset_id=asset_id),
                    mock.patch.object(MODULE, "RESOURCE_ROOT", temporary_root),
                    mock.patch.object(
                        MODULE,
                        "WARLORD_CHAIN_PREVIEW_BASE_ALIAS_ASSET_ID",
                        asset_id,
                    ),
                    self.assertRaises(MODULE.RestorationError),
                ):
                    MODULE.apply_warlord_chain_preview_boundary(
                        copy.deepcopy(material),
                        copy.deepcopy(detail),
                        copy.deepcopy(source_resources),
                    )

        invalid_materials = []
        wrong_template = copy.deepcopy(material)
        wrong_template["templateId"] = "effect.standard"
        invalid_materials.append(wrong_template)
        wrong_profile = copy.deepcopy(material)
        wrong_profile["sourceProfile"]["runtimeShaderProfileId"] = (
            "effect.ue3.reconstructed-standard.v1"
        )
        invalid_materials.append(wrong_profile)
        for invalid_material in invalid_materials:
            with self.subTest(material=invalid_material), self.assertRaisesRegex(
                MODULE.RestorationError, "source profile is invalid"
            ):
                MODULE.apply_warlord_chain_preview_boundary(
                    invalid_material,
                    copy.deepcopy(detail),
                    copy.deepcopy(source_resources),
                )

        invalid_resources = copy.deepcopy(source_resources)
        invalid_resources.append({
            "slotId": "base",
            "assetId": "Effect/Warlord/Textures/FX_TEX_02/fx_d_grid_016.dds",
        })
        with self.assertRaisesRegex(
            MODULE.RestorationError, "resource identity/cardinality is invalid"
        ):
            MODULE.apply_warlord_chain_preview_boundary(
                copy.deepcopy(material),
                copy.deepcopy(detail),
                invalid_resources,
            )


class GenericAuthoringOwnershipTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.resource_root = Path(self.temporary.name) / "Resources"
        self.assets = {
            "compiler": "Effect/Test/compiler.dds",
            "compiler_new": "Effect/Test/compiler-new.dds",
            "artist": "Effect/Test/artist.dds",
            "compiler_noise": "Effect/Test/compiler-noise.dds",
            "compiler_noise_new": "Effect/Test/compiler-noise-new.dds",
            "artist_noise": "Effect/Test/artist-noise.dds",
            "compiler_model": "Effect/Test/compiler.wmodel",
            "artist_model": "Effect/Test/artist.wmodel",
        }
        for asset_id in self.assets.values():
            path = self.resource_root / asset_id
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b"g3-focused-fixture")
        self.resource_patch = mock.patch.object(
            MODULE, "RESOURCE_ROOT", self.resource_root
        )
        self.resource_patch.start()

    def tearDown(self) -> None:
        self.resource_patch.stop()
        self.temporary.cleanup()

    @staticmethod
    def _profile(
        opacity: float = 1.0,
        size: float = 2.0,
        tint: list[float] | None = None,
        named_texture: str | None = None,
    ) -> dict:
        tint_value = list(tint or [1.0, 1.0, 1.0, 1.0])
        return {
            "enabled": True,
            "profileId": "ue3.material.g3.fixture",
            "runtimeShaderProfileId": MODULE.GROUPED_TRANSLUCENT_PROFILE,
            "parentMaterialPath": "fx_m.g3_parent",
            "semanticStatus": "exact",
            "textures": (
                [{"name": "named.diffuse", "assetId": named_texture}]
                if named_texture is not None
                else []
            ),
            "scalars": [
                {"name": "opacity", "group": "opacity", "value": opacity},
                {"name": "size", "group": "shape", "value": size},
            ],
            "vectors": [
                {"name": "tint", "group": "color", "value": tint_value}
            ],
            "staticSwitches": [],
            "dynamicParameterSemantics": [
                "opacity",
                "unbound",
                "unbound",
                "unbound",
            ],
            "subUVMode": "none",
        }

    def _compiler_element(
        self,
        *,
        base: str | None = None,
        noise: str | None = None,
        lane: str | None = None,
        named_texture: str | None = None,
        opacity: float = 1.0,
        size: float = 2.0,
        tint: list[float] | None = None,
    ) -> dict:
        tint_value = list(tint or [1.0, 1.0, 1.0, 1.0])
        resources = []
        if base is not None:
            resources.append({"slotId": "base", "assetId": base})
        if noise is not None:
            resources.append({"slotId": "noise", "assetId": noise})
        execution = {
            "textureLanes": (
                [{"laneId": "lane.0", "assetId": lane}]
                if lane is not None
                else []
            ),
            "scalars": [
                {"name": "opacity", "packedIndex": 0, "value": opacity},
                {"name": "size", "packedIndex": 1, "value": size},
            ],
            "vectors": [
                {"name": "tint", "packedIndex": 0, "value": tint_value}
            ],
            "artistParameters": [
                {"name": "tint", "packedIndex": 0, "value": tint_value}
            ],
            "colors": [
                {"name": "tint", "packedIndex": 0, "value": tint_value}
            ],
        }
        return {
            "id": "particle.g3.target",
            "displayName": "Compiler display",
            "groupId": "compiler.group",
            "kind": "particle",
            "visible": True,
            "resources": resources,
            "material": {
                "templateId": "effect.source_material",
                "sourceMaterialPath": "fx_m.g3_instance",
                "renderProfile": "alpha_two_sided_depth_read",
                "sourceProfile": self._profile(
                    opacity, size, tint_value, named_texture
                ),
                "execution": execution,
            },
            "detail": {"transform": {"position": [0.0, 0.0, 0.0]}},
            "actionCueAttachment": {"enabled": False},
            "transformInheritance": {"enabled": False, "masterElementId": ""},
        }

    def _stable_with_overrides(self) -> dict:
        stable = self._compiler_element(
            base=self.assets["compiler"],
            noise=self.assets["compiler_noise"],
            lane=self.assets["compiler_noise"],
            named_texture=self.assets["compiler_noise"],
        )
        stable["displayName"] = "Artist display"
        stable["groupId"] = "artist.group"
        stable["visible"] = False
        stable["detail"]["transform"]["position"] = [1.0, 2.0, 3.0]
        stable["actionCueAttachment"] = {
            "enabled": True,
            "follow": True,
            "runtimeBoneName": "b_weapon_rhand",
        }
        stable["transformInheritance"] = {
            "enabled": True,
            "masterElementId": "particle.g3.master",
        }
        stable["resources"][0]["assetId"] = self.assets["artist"]
        stable["material"]["execution"]["textureLanes"][0]["assetId"] = (
            self.assets["artist_noise"]
        )
        stable["material"]["sourceProfile"]["textures"][0]["assetId"] = (
            self.assets["artist_noise"]
        )
        stable["material"]["sourceProfile"]["scalars"][0]["value"] = 0.4
        stable["material"]["sourceProfile"]["vectors"][0]["value"] = [
            0.1,
            0.2,
            0.3,
            0.4,
        ]
        stable["material"]["execution"]["scalars"][0]["value"] = 0.4
        for collection in ("vectors", "artistParameters", "colors"):
            stable["material"]["execution"][collection][0]["value"] = [
                0.1,
                0.2,
                0.3,
                0.4,
            ]
        stable["authoringOverrides"] = {
            "resources": [
                {
                    "slotId": "base",
                    "assetId": self.assets["artist"],
                    "compilerAssetId": self.assets["compiler"],
                },
                {
                    "slotId": "materialExecutionLane:lane.0",
                    "assetId": self.assets["artist_noise"],
                    "compilerAssetId": self.assets["compiler_noise"],
                },
                {
                    "slotId": "sourceMaterialTexture:named.diffuse",
                    "assetId": self.assets["artist_noise"],
                    "compilerAssetId": self.assets["compiler_noise"],
                },
            ],
            "scalars": [
                {"name": "opacity", "value": 0.4, "compilerValue": 1.0}
            ],
            "colors": [
                {
                    "name": "tint",
                    "value": [0.1, 0.2, 0.3, 0.4],
                    "compilerValue": [1.0, 1.0, 1.0, 1.0],
                }
            ],
        }
        return stable

    def test_resource_scalar_color_rebase_and_execution_effective_values(self) -> None:
        stable = self._stable_with_overrides()
        compiler = self._compiler_element(
            base=self.assets["compiler_new"],
            noise=self.assets["compiler_noise_new"],
            lane=self.assets["compiler_noise_new"],
            named_texture=self.assets["compiler_noise_new"],
            opacity=0.75,
            size=3.0,
            tint=[0.9, 0.8, 0.7, 0.6],
        )
        stable_before = copy.deepcopy(stable)
        compiler_before = copy.deepcopy(compiler)

        result, drops = MODULE.reapply_authoring_overrides(compiler, stable)

        self.assertEqual(drops, [])
        self.assertEqual(stable, stable_before)
        self.assertEqual(compiler, compiler_before)
        self.assertEqual(result["resources"][0]["assetId"], self.assets["artist"])
        self.assertEqual(
            result["material"]["execution"]["textureLanes"][0]["assetId"],
            self.assets["artist_noise"],
        )
        self.assertEqual(
            result["material"]["sourceProfile"]["textures"][0]["assetId"],
            self.assets["artist_noise"],
        )
        self.assertEqual(
            result["material"]["sourceProfile"]["scalars"][0]["value"], 0.4
        )
        self.assertEqual(
            result["material"]["sourceProfile"]["vectors"][0]["value"],
            [0.1, 0.2, 0.3, 0.4],
        )
        self.assertEqual(
            result["material"]["execution"]["scalars"][0]["value"], 0.4
        )
        for collection in ("vectors", "artistParameters", "colors"):
            self.assertEqual(
                result["material"]["execution"][collection][0]["value"],
                [0.1, 0.2, 0.3, 0.4],
            )
        overrides = result["authoringOverrides"]
        self.assertEqual(
            [row["compilerAssetId"] for row in overrides["resources"]],
            [
                self.assets["compiler_new"],
                self.assets["compiler_noise_new"],
                self.assets["compiler_noise_new"],
            ],
        )
        self.assertEqual(overrides["scalars"][0]["compilerValue"], 0.75)
        self.assertEqual(
            overrides["colors"][0]["compilerValue"], [0.9, 0.8, 0.7, 0.6]
        )
        profile = result["material"]["sourceProfile"]
        self.assertEqual(profile["profileId"], "ue3.material.g3.fixture")
        self.assertEqual(
            profile["runtimeShaderProfileId"], MODULE.GROUPED_TRANSLUCENT_PROFILE
        )
        self.assertEqual(profile["subUVMode"], "none")
        self.assertEqual(json.loads(MODULE.serialized(result)), result)
        repeated, repeated_drops = MODULE.reapply_authoring_overrides(
            compiler, result
        )
        self.assertEqual(repeated_drops, [])
        self.assertEqual(repeated, result)

        no_op_compiler = copy.deepcopy(compiler)
        no_op_compiler["material"]["sourceProfile"]["scalars"][0]["value"] = 0.4
        no_op_compiler["material"]["execution"]["scalars"][0]["value"] = 0.4
        no_op, _ = MODULE.reapply_authoring_overrides(no_op_compiler, stable)
        self.assertEqual(no_op["authoringOverrides"]["scalars"], [])

        execution_only_stable = copy.deepcopy(stable)
        execution_only_stable["material"]["sourceProfile"]["scalars"] = []
        execution_only_stable["material"]["sourceProfile"]["vectors"] = []
        execution_only_compiler = copy.deepcopy(compiler)
        execution_only_compiler["material"]["sourceProfile"]["scalars"] = []
        execution_only_compiler["material"]["sourceProfile"]["vectors"] = []
        execution_only, execution_only_drops = MODULE.reapply_authoring_overrides(
            execution_only_compiler, execution_only_stable
        )
        self.assertEqual(execution_only_drops, [])
        self.assertEqual(
            execution_only["material"]["execution"]["scalars"][0]["value"],
            0.4,
        )
        for collection in ("vectors", "artistParameters", "colors"):
            self.assertEqual(
                execution_only["material"]["execution"][collection][0]["value"],
                [0.1, 0.2, 0.3, 0.4],
            )
        self.assertEqual(
            execution_only["authoringOverrides"]["scalars"][0]["compilerValue"],
            0.75,
        )
        self.assertEqual(
            execution_only["authoringOverrides"]["colors"][0]["compilerValue"],
            [0.9, 0.8, 0.7, 0.6],
        )

    def test_invalid_override_path_type_duplicate_and_mismatch_roll_back(self) -> None:
        compiler = self._compiler_element(
            base=self.assets["compiler_new"],
            noise=self.assets["compiler_noise_new"],
            lane=self.assets["compiler_noise_new"],
            named_texture=self.assets["compiler_noise_new"],
        )

        cases: list[tuple[str, dict, str]] = []
        unsafe = self._stable_with_overrides()
        unsafe["authoringOverrides"]["resources"][0]["assetId"] = "../escape.dds"
        cases.append(("unsafe path", unsafe, "unsafe authoring override asset"))

        wrong_type = self._stable_with_overrides()
        wrong_type["resources"][0]["assetId"] = self.assets["artist_model"]
        wrong_type["authoringOverrides"]["resources"][0]["assetId"] = self.assets[
            "artist_model"
        ]
        cases.append(("wrong type", wrong_type, "resource type"))

        duplicate = self._stable_with_overrides()
        duplicate["authoringOverrides"]["resources"].append(
            copy.deepcopy(duplicate["authoringOverrides"]["resources"][0])
        )
        cases.append(("duplicate", duplicate, "duplicate/invalid"))

        duplicate_scalar = self._stable_with_overrides()
        duplicate_scalar["authoringOverrides"]["scalars"].append(
            copy.deepcopy(duplicate_scalar["authoringOverrides"]["scalars"][0])
        )
        cases.append(
            ("duplicate scalar", duplicate_scalar, "duplicate/invalid")
        )

        unknown = self._stable_with_overrides()
        unknown["authoringOverrides"]["resources"][0]["slotId"] = "missing"
        cases.append(("unknown target", unknown, "target is unknown"))

        unknown_named_texture = self._stable_with_overrides()
        unknown_named_texture["authoringOverrides"]["resources"][2]["slotId"] = (
            "sourceMaterialTexture:missing"
        )
        cases.append(
            (
                "unknown named texture",
                unknown_named_texture,
                "target is unknown",
            )
        )

        mismatch = self._stable_with_overrides()
        mismatch["material"]["sourceProfile"]["scalars"][0]["value"] = 0.5
        cases.append(("effective mismatch", mismatch, "mismatch"))

        mirror_mismatch = self._stable_with_overrides()
        mirror_mismatch["material"]["execution"]["scalars"][0]["value"] = 0.5
        cases.append(("mirror mismatch", mirror_mismatch, "mirror mismatch"))

        wrong_parameter_type = self._stable_with_overrides()
        wrong_parameter_type["material"]["sourceProfile"]["scalars"][0][
            "name"
        ] = "renamed.opacity"
        wrong_parameter_type["material"]["sourceProfile"]["vectors"].append(
            {
                "name": "opacity",
                "group": "wrong-type",
                "value": [0.4, 0.4, 0.4, 0.4],
            }
        )
        cases.append(
            ("parameter type", wrong_parameter_type, "parameter type mismatch")
        )

        no_op_resource = self._stable_with_overrides()
        no_op_resource["authoringOverrides"]["resources"][0][
            "compilerAssetId"
        ] = self.assets["artist"]
        cases.append(("no-op resource", no_op_resource, "is a no-op"))

        no_op_scalar = self._stable_with_overrides()
        no_op_scalar["authoringOverrides"]["scalars"][0]["compilerValue"] = 0.4
        cases.append(("no-op scalar", no_op_scalar, "is a no-op"))

        no_op_color = self._stable_with_overrides()
        no_op_color["authoringOverrides"]["colors"][0]["compilerValue"] = [
            0.1,
            0.2,
            0.3,
            0.4,
        ]
        cases.append(("no-op color", no_op_color, "is a no-op"))

        for name, stable, message in cases:
            with self.subTest(name=name):
                compiler_before = copy.deepcopy(compiler)
                stable_before = copy.deepcopy(stable)
                with self.assertRaisesRegex(MODULE.RestorationError, message):
                    MODULE.reapply_authoring_overrides(compiler, stable)
                self.assertEqual(compiler, compiler_before)
                self.assertEqual(stable, stable_before)

    @staticmethod
    def _material_projection() -> dict[str, object]:
        return {
            "canonicalMaterialJoined": False,
            "sharedMaterialContractResolved": False,
            "nonExactNamedTextureAliasCount": 0,
            "sourceDynamicParameterArithmeticUnavailable": False,
        }

    def _source_assignment(
        self,
        resources: list[dict[str, str]],
        profile: dict,
    ) -> MODULE.SourceParticleAssignment:
        element_id = "source.particle.g3"
        effect_id = "source.effect.g3"
        element = {
            "id": element_id,
            "displayName": "Source Particle",
            "groupId": "source.group",
            "sourceNode": "source.node",
            "visible": True,
            "kind": "particle",
            "resources": copy.deepcopy(resources),
            "material": {
                "templateId": "effect.source_material",
                "sourceMaterialPath": "fx_m.g3_instance",
                "renderProfile": "alpha_two_sided_depth_read",
                "sourceProfile": copy.deepcopy(profile),
            },
            "detail": {"transform": {"position": [0.0, 0.0, 0.0]}},
            "actionCueAttachment": {"enabled": False},
            "transformInheritance": {"enabled": False, "masterElementId": ""},
            "sourceRecipe": {
                "enabled": True,
                "rendererShape": "sprite",
                "modules": [
                    {"className": "particlemodulerequired", "distributions": []}
                ],
            },
            "sourcePresentation": {"enabled": False},
        }
        source_document = MODULE.SourceDocument(
            character_class="ARTIST",
            path=Path("synthetic.source.effect.json"),
            effect_id=effect_id,
            document={"elements": [element]},
            elements={element_id: element},
            canonical_materials={},
            generated_receipt_path=Path("synthetic.receipt.json"),
            timeline_events={},
            first_event_by_system={},
            material_resolution_by_path={
                "fx_m.g3_instance": MODULE.EXACT_SOURCE_MATERIAL_STATUS
            },
            material_physical_package_by_path={},
            runtime_texture_by_source_path={},
            manifest_texture_by_source_path={},
            canonical_v12_package_aliases={},
            shared_material_contracts={},
        )
        return MODULE.SourceParticleAssignment(
            character_class="ARTIST",
            skill_id=31000,
            source_document=source_document,
            source_element_id=element_id,
            source_event_id="source.event.g3",
            source_order=0,
            target_element_id="particle.g3.target",
            target_effect_id="effect.artist.skill.31000.unified",
            clip_timeline_offset_seconds=0.0,
        )

    def test_materialize_preserves_artist_fields_and_records_typed_vanishing(self) -> None:
        assignment = self._source_assignment(
            [
                {"slotId": "base", "assetId": self.assets["compiler"]},
                {
                    "slotId": "noise",
                    "assetId": self.assets["compiler_noise"],
                },
            ],
            self._profile(named_texture=self.assets["compiler_noise"]),
        )

        def canonical_material(source_document, source_element_id, _resources):
            return (
                copy.deepcopy(
                    source_document.elements[source_element_id]["material"]
                ),
                self._material_projection(),
            )

        with (
            mock.patch.object(
                MODULE, "canonical_particle_material", side_effect=canonical_material
            ),
            mock.patch.object(
                MODULE,
                "canonical_particle_detail",
                side_effect=lambda source, *_: copy.deepcopy(source["detail"]),
            ),
            mock.patch.object(
                MODULE,
                "normalized_source_recipe",
                side_effect=lambda value: copy.deepcopy(value),
            ),
            mock.patch.object(
                MODULE,
                "portable_recipe",
                side_effect=lambda value: copy.deepcopy(value),
            ),
            mock.patch.object(
                MODULE,
                "source_profile_readiness",
                return_value=(True, True, "READY"),
            ),
        ):
            pristine, _ = MODULE.materialize_source_particle(
                assignment, None, None
            )
            tuned = copy.deepcopy(pristine)
            tuned["displayName"] = "Artist Particle"
            tuned["groupId"] = "artist.group"
            tuned["visible"] = False
            tuned["detail"]["transform"]["position"] = [1.0, 2.0, 3.0]
            tuned["actionCueAttachment"] = {
                "enabled": True,
                "follow": True,
                "runtimeBoneName": "b_weapon_rhand",
            }
            tuned["transformInheritance"] = {
                "enabled": True,
                "masterElementId": "particle.g3.master",
            }
            tuned["resources"][0]["assetId"] = self.assets["artist"]
            tuned["resources"][1]["assetId"] = self.assets["artist_noise"]
            tuned["material"]["sourceProfile"]["scalars"][0]["value"] = 0.4
            tuned["material"]["sourceProfile"]["scalars"][1]["value"] = 1.5
            tuned["material"]["sourceProfile"]["vectors"][0]["value"] = [
                0.1,
                0.2,
                0.3,
                0.4,
            ]
            tuned["material"]["sourceProfile"]["textures"][0]["assetId"] = (
                self.assets["artist_noise"]
            )
            tuned["authoringOverrides"] = {
                "resources": [
                    {
                        "slotId": "base",
                        "assetId": self.assets["artist"],
                        "compilerAssetId": self.assets["compiler"],
                    },
                    {
                        "slotId": "noise",
                        "assetId": self.assets["artist_noise"],
                        "compilerAssetId": self.assets["compiler_noise"],
                    },
                    {
                        "slotId": "sourceMaterialTexture:named.diffuse",
                        "assetId": self.assets["artist_noise"],
                        "compilerAssetId": self.assets["compiler_noise"],
                    },
                ],
                "scalars": [
                    {"name": "opacity", "value": 0.4, "compilerValue": 1.0},
                    {"name": "size", "value": 1.5, "compilerValue": 2.0},
                ],
                "colors": [
                    {
                        "name": "tint",
                        "value": [0.1, 0.2, 0.3, 0.4],
                        "compilerValue": [1.0, 1.0, 1.0, 1.0],
                    }
                ],
            }

            rebound, receipt = MODULE.materialize_source_particle(
                assignment, None, tuned
            )
            for field in (
                "displayName",
                "groupId",
                "detail",
                "visible",
                "actionCueAttachment",
                "transformInheritance",
                "authoringOverrides",
            ):
                self.assertEqual(rebound[field], tuned[field])
            self.assertEqual(receipt["authoringOverrideDrops"], [])
            self.assertTrue(receipt["stableReimportPreserved"])
            self.assertEqual(json.loads(MODULE.serialized(rebound)), rebound)
            repeated, _ = MODULE.materialize_source_particle(
                assignment, None, rebound
            )
            self.assertEqual(repeated, rebound)

            refreshed_assignment = self._source_assignment(
                [
                    {
                        "slotId": "noise",
                        "assetId": self.assets["compiler_noise_new"],
                    }
                ],
                {
                    **self._profile(size=3.0),
                    "scalars": [
                        {"name": "size", "group": "shape", "value": 3.0}
                    ],
                    "vectors": [],
                },
            )
            refreshed, refreshed_receipt = MODULE.materialize_source_particle(
                refreshed_assignment, None, rebound
            )

        self.assertEqual(
            refreshed_receipt["authoringOverrideDrops"],
            [
                {
                    "targetKind": "resource",
                    "targetId": "base",
                    "reason": "RESOURCE_SLOT_VANISHED",
                },
                {
                    "targetKind": "resource",
                    "targetId": "sourceMaterialTexture:named.diffuse",
                    "reason": "RESOURCE_SLOT_VANISHED",
                },
                {
                    "targetKind": "scalar",
                    "targetId": "opacity",
                    "reason": "SCALAR_PARAMETER_VANISHED",
                },
                {
                    "targetKind": "color",
                    "targetId": "tint",
                    "reason": "COLOR_PARAMETER_VANISHED",
                },
            ],
        )
        self.assertEqual(
            refreshed["authoringOverrides"],
            {
                "resources": [
                    {
                        "slotId": "noise",
                        "assetId": self.assets["artist_noise"],
                        "compilerAssetId": self.assets["compiler_noise_new"],
                    }
                ],
                "scalars": [
                    {"name": "size", "value": 1.5, "compilerValue": 3.0}
                ],
                "colors": [],
            },
        )
        self.assertEqual(refreshed["displayName"], "Artist Particle")
        self.assertEqual(refreshed["groupId"], "artist.group")


class DimensionSummonMaskedModelCueTests(unittest.TestCase):
    @staticmethod
    def _record() -> object:
        return MODULE.CandidateRecord(
            character_class="DIMENSIONMASTER",
            skill_id=2050500,
            stage_index=0,
            stage_clip_index=0,
            clip=MODULE.DIMENSION_SUMMON_CLIP_NAME,
            target_effect_id=MODULE.DIMENSION_SUMMON_MODEL_CUE_TARGET,
            target_path=Path("unused.effect.json"),
            blueprint_path=Path("unused.blueprint.json"),
        )

    @staticmethod
    def _current(alpha_mode: str) -> dict:
        return {
            "modelCues": [
                {
                    "cueId": MODULE.DIMENSION_SUMMON_MODEL_CUE_ID,
                    "modelAssetId": MODULE.DIMENSION_SUMMON_MODEL_ASSET_ID,
                    "clipName": MODULE.DIMENSION_SUMMON_CLIP_NAME,
                    "alphaMode": alpha_mode,
                }
            ]
        }

    def test_exact_mask_contract_and_drift_rollback(self) -> None:
        current = self._current("MASKED")
        current_before = copy.deepcopy(current)
        staged = {"modelCues": [{"sentinel": "unchanged-until-commit"}]}
        rows = MODULE.materialize_document_model_cues(
            self._record(), current, staged
        )
        self.assertEqual(current, current_before)
        self.assertEqual(staged["modelCues"], current["modelCues"])
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]["alphaMode"], "MASKED")
        self.assertEqual(
            rows[0]["provenance"], "RAW_UE3_PARENT_BLEND_MASKED_EXACT"
        )
        contract = rows[0]["sourceMaskContract"]
        self.assertEqual(contract["blendMode"], "BLEND_Masked")
        self.assertFalse(contract["twoSided"])
        self.assertEqual(contract["opacityMaskClipValue"], 0.333)
        self.assertEqual(
            [row["belowCutoffPixelCount"] for row in contract["sections"]],
            [0, 0, 0, 643_895],
        )

        invalid = self._current("OPAQUE")
        invalid_before = copy.deepcopy(invalid)
        rollback = {"modelCues": [{"sentinel": "must-survive"}]}
        rollback_before = copy.deepcopy(rollback)
        with self.assertRaisesRegex(
            MODULE.RestorationError, "alphaMode drifted: OPAQUE"
        ):
            MODULE.materialize_document_model_cues(
                self._record(), invalid, rollback
            )
        self.assertEqual(invalid, invalid_before)
        self.assertEqual(rollback, rollback_before)


class FourClassTrackACandidateTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.batch = MODULE.load_json(MODULE.BATCH_PATH)
        cls.records = MODULE.candidate_records(cls.batch)
        cls.record_by_target = {
            row.target_effect_id: row for row in cls.records
        }
        cls.source_by_effect, cls.stage_effects = MODULE.load_source_index()
        (
            cls.source_particles,
            cls.particle_exclusions,
        ) = MODULE.load_source_particle_assignments(
            cls.records, cls.source_by_effect
        )
        cls.drawable_decisions = MODULE.load_drawable_resource_decisions(
            cls.source_by_effect
        )
        cls.source_decals = MODULE.load_source_decal_assignments(
            cls.records, cls.source_by_effect
        )
        cls.source_animation_trails = (
            MODULE.load_source_animation_trail_assignments(cls.records)
        )
        cls.animation_trail_template = (
            MODULE.load_animation_trail_authoring_template()
        )
        cls.outputs, cls.receipt = MODULE.build_projection()
        cls.documents = {
            record.target_effect_id: json.loads(cls.outputs[record.target_path])
            for record in cls.records
        }
        cls.receipt_by_target = {
            row["targetEffectAssetId"]: row
            for row in cls.receipt["targets"]
        }

    def test_real_projection_closes_the_full_source_denominator(self) -> None:
        counts = self.receipt["counts"]
        self.assertEqual(counts["targetCount"], 101)
        self.assertEqual(counts["sourceParticleCorpusCount"], 4846)
        self.assertEqual(counts["strictMappedParticleCount"], 4687)
        self.assertEqual(counts["excludedParticleCount"], 159)
        self.assertEqual(counts["legacySelectedParticleCount"], 2160)
        self.assertEqual(counts["legacySelectedExcludedCount"], 9)
        self.assertEqual(counts["legacySelectedRetargetedCount"], 2)
        self.assertEqual(counts["sourceDecalCount"], 79)
        self.assertEqual(counts["sourceAnimationTrailNotifyCount"], 8)
        self.assertEqual(counts["sourceAnimationTrailElementCount"], 11)
        self.assertEqual(counts["supplementalPreservedCount"], 11)
        self.assertEqual(counts["placeholderTrailExcludedCount"], 0)
        self.assertEqual(counts["outputElementCount"], 4777)
        self.assertEqual(
            counts["portableCount"] + counts["sourcePreservedDeferredCount"],
            4687,
        )
        independently_portable = 0
        for rows in self.source_particles.values():
            for assignment in rows:
                source = assignment.source_document.elements[
                    assignment.source_element_id
                ]
                try:
                    MODULE.portable_recipe(source["sourceRecipe"])
                except MODULE.PortableRecipeError:
                    pass
                else:
                    independently_portable += 1
        self.assertEqual(counts["portableCount"], independently_portable)
        self.assertEqual(len(self.outputs), 102)
        self.assertFalse(self.receipt["productMutation"])
        self.assertFalse(self.receipt["visualApproval"])

    def test_exclusion_receipt_is_exact_and_stable(self) -> None:
        self.assertEqual(len(self.particle_exclusions), 159)
        self.assertEqual(
            Counter(
                (row.character_class, row.skill_id, row.reason)
                for row in self.particle_exclusions
            ),
            Counter(
                {
                    ("ARTIST", 31210, "NO_CURRENT_101_TARGET"): 74,
                    (
                        "ARTIST",
                        31210,
                        "OUTSIDE_STAGE_WINDOW_OR_EVENT_SET",
                    ): 68,
                    ("ARTIST", 31430, "NO_EVENT_JOIN"): 1,
                    ("DIMENSIONMASTER", 2050540, "NO_EVENT_JOIN"): 15,
                    ("LANCE_MASTER", 34590, "NO_EVENT_JOIN"): 1,
                }
            ),
        )
        rows = self.receipt["particleExclusions"]
        self.assertEqual(len(rows), 159)
        self.assertEqual(len({row["exclusionId"] for row in rows}), 159)
        self.assertTrue(
            all(row["exclusionId"].startswith("excluded.source-particle.") for row in rows)
        )

    def test_every_strict_particle_has_one_stable_target_identity(self) -> None:
        output_particles = [
            element
            for document in self.documents.values()
            for element in document["elements"]
            if element["kind"] == "particle"
        ]
        self.assertEqual(len(output_particles), 4687)
        self.assertEqual(len({row["id"] for row in output_particles}), 4687)
        self.assertTrue(
            all(row["id"].startswith("authored.source-particle.") for row in output_particles)
        )
        output_source_nodes = {row["sourceNode"] for row in output_particles}
        self.assertEqual(len(output_source_nodes), 4687)
        excluded_identities = {
            (row.source_effect_id, row.source_element_id)
            for row in self.particle_exclusions
        }
        for rows in self.source_particles.values():
            for assignment in rows:
                self.assertNotIn(
                    (
                        assignment.source_document.effect_id,
                        assignment.source_element_id,
                    ),
                    excluded_identities,
                )
                target = self.documents[assignment.target_effect_id]
                self.assertIn(
                    assignment.target_element_id,
                    {element["id"] for element in target["elements"]},
                )

    def test_source_detail_is_canonical_clip_local_and_mesh_scale_is_once(self) -> None:
        for target_effect_id, assignments in self.source_particles.items():
            staged_by_id = {
                element["id"]: element
                for element in self.documents[target_effect_id]["elements"]
            }
            for assignment in assignments:
                source = assignment.source_document.elements[
                    assignment.source_element_id
                ]
                decision = self.drawable_decisions.get(
                    (
                        assignment.source_document.effect_id,
                        assignment.source_element_id,
                    )
                )
                expected_detail = MODULE.canonical_particle_detail(
                    source, assignment, decision
                )
                staged = staged_by_id[assignment.target_element_id]
                self.assertEqual(staged["detail"], expected_detail)
                self.assertAlmostEqual(
                    staged["detail"]["timing"]["startDelaySeconds"],
                    float(source["detail"]["timing"]["startDelaySeconds"])
                    - assignment.clip_timeline_offset_seconds,
                    places=6,
                )
                self.assertEqual(
                    staged["detail"]["transform"], source["detail"]["transform"]
                )
                if source["sourceRecipe"]["rendererShape"] == "mesh":
                    self.assertEqual(
                        staged["detail"]["mesh"]["modelPreScale"], 0.01
                    )

    def test_resources_are_source_exact_or_receipt_proven_base_only(self) -> None:
        supplemental_count = 0
        for target_receipt in self.receipt["targets"]:
            for row in target_receipt["particleRows"]:
                source_by_slot = {
                    binding["slotId"]: binding["assetId"]
                    for binding in row["sourceBindings"]
                }
                target_by_slot = {
                    binding["slotId"]: binding["assetId"]
                    for binding in row["targetBindings"]
                }
                for slot_id, asset_id in source_by_slot.items():
                    self.assertEqual(target_by_slot[slot_id], asset_id)
                supplemental = set(target_by_slot) - set(source_by_slot)
                self.assertTrue(supplemental <= {"base"})
                if supplemental:
                    supplemental_count += 1
                    self.assertNotEqual(row["drawableDecision"], "NONE")
                    decision = self.drawable_decisions[
                        (row["sourceEffectAssetId"], row["sourceElementId"])
                    ]
                    self.assertEqual(
                        tuple(target_by_slot.items()), decision.target_resources
                    )
        self.assertEqual(supplemental_count, 363)
        counts = self.receipt["counts"]
        self.assertEqual(counts["sourceResourceBindingCount"], 8509)
        self.assertEqual(counts["resourceBindingCount"], 8872)
        self.assertEqual(counts["receiptSupplementalBindingCount"], 363)

    def test_visible_particles_always_have_executable_evidence(self) -> None:
        row_by_id = {
            row["targetElementId"]: row
            for target in self.receipt["targets"]
            for row in target["particleRows"]
        }
        visible_count = 0
        for document in self.documents.values():
            for element in document["elements"]:
                if element["kind"] != "particle":
                    continue
                row = row_by_id[element["id"]]
                profile_enabled = bool(
                    element["material"].get("sourceProfile", {}).get(
                        "enabled"
                    )
                )
                if element["material"]["templateId"] == "effect.source_material":
                    self.assertTrue(profile_enabled)
                if row["failClosedReasons"]:
                    self.assertFalse(element["visible"])
                    self.assertEqual(
                        element["material"]["execution"],
                        MODULE.FAIL_CLOSED_EXECUTION,
                    )
                    if not profile_enabled:
                        self.assertEqual(
                            element["material"]["templateId"],
                            "effect.standard",
                        )
                else:
                    self.assertTrue(row["portable"])
                    self.assertTrue(row["hasSafeDrawable"])
                    self.assertNotIn("execution", element["material"])
                    self.assertTrue(row["runtimeShaderProfileId"])
                    self.assertTrue(profile_enabled)
                if element["visible"]:
                    visible_count += 1
                    self.assertEqual(
                        element["material"]["templateId"],
                        "effect.source_material",
                    )
                    self.assertTrue(row["portable"])
                    self.assertTrue(row["hasSafeDrawable"])
                    self.assertTrue(row["sourceProfileReady"])
        counts = self.receipt["counts"]
        self.assertEqual(visible_count, counts["drawableAdmittedCount"])
        self.assertEqual(counts["drawableAdmittedCount"], 3481)
        self.assertEqual(counts["portableFailClosedCount"], 1007)
        self.assertEqual(counts["missingSafeBaseCount"], 1230)
        self.assertEqual(counts["missingExactMeshModelCount"], 110)
        self.assertEqual(counts["missingExecutableDrawableCount"], 1069)
        self.assertEqual(counts["nonExactMaterialCount"], 90)

    def test_canonical_material_and_named_texture_coverage_is_exact(self) -> None:
        counts = self.receipt["counts"]
        self.assertEqual(counts["canonicalMaterialJoinedCount"], 784)
        self.assertEqual(counts["sharedMaterialContractMatchedCount"], 4637)
        self.assertEqual(counts["sharedMaterialContractResolvedCount"], 4637)
        self.assertEqual(counts["sharedMaterialProfileJoinedCount"], 3730)
        self.assertEqual(
            counts["sharedMaterialOccurrenceProfilePromotedCount"], 372
        )
        self.assertEqual(
            counts["sharedMaterialCanonicalFallbackReplacedCount"], 19
        )
        self.assertEqual(counts["sourceProfileEnabledCount"], 4562)
        self.assertEqual(counts["sourceProfileReadyCount"], 3672)
        self.assertEqual(counts["sourceProfileBlockedCount"], 890)
        self.assertEqual(counts["namedTextureCount"], 8484)
        self.assertEqual(counts["exactNamedTextureCount"], 6911)
        self.assertEqual(counts["rebasedNamedTextureCount"], 1967)
        self.assertEqual(counts["manifestNamedTextureCount"], 104)
        self.assertEqual(
            counts["canonicalV12PackageQualifiedAssetCount"], 20
        )
        self.assertEqual(
            counts["canonicalV12PackageQualifiedElementCount"], 20
        )
        self.assertEqual(counts["nonExactNamedTextureAliasCount"], 47)
        self.assertEqual(counts["nonExactNamedTextureAliasElementCount"], 30)
        self.assertEqual(counts["unresolvedNamedTextureCount"], 1469)
        self.assertEqual(
            counts["dimensionMasterBaseLessGroupedSpriteAdmittedCount"], 8
        )

        class_coverage: dict[str, Counter[str]] = {}
        map_b_assets: Counter[tuple[str, str]] = Counter()
        for target in self.receipt["targets"]:
            coverage = class_coverage.setdefault(
                target["characterClass"], Counter()
            )
            document_by_id = {
                element["id"]: element
                for element in self.documents[target["targetEffectAssetId"]][
                    "elements"
                ]
            }
            for row in target["particleRows"]:
                coverage["canonical"] += bool(row["canonicalMaterialJoined"])

                coverage["matched"] += bool(
                    row["sharedMaterialContractMatched"]
                )
                coverage["resolved"] += bool(
                    row["sharedMaterialContractResolved"]
                )
                coverage["joined"] += bool(row["sharedMaterialProfileJoined"])
                coverage["promoted"] += bool(
                    row["sharedMaterialOccurrenceProfilePromoted"]
                )
                coverage["replaced"] += bool(
                    row["sharedMaterialCanonicalFallbackReplaced"]
                )
                coverage["enabled"] += bool(row["runtimeShaderProfileId"])
                coverage["ready"] += bool(
                    row["runtimeShaderProfileId"] and row["sourceProfileReady"]
                )
                coverage["blocked"] += bool(
                    row["runtimeShaderProfileId"]
                    and not row["sourceProfileReady"]
                )
                element = document_by_id[row["targetElementId"]]
                profile = element["material"].get("sourceProfile", {})
                if profile.get("enabled"):
                    self.assertEqual(
                        element["material"]["templateId"],
                        "effect.source_material",
                    )
                for texture in profile.get("textures", []):
                    source_path = str(texture.get("sourceObjectPath", ""))
                    asset_id = str(texture.get("assetId", ""))
                    source_document = self.source_by_effect[
                        row["sourceEffectAssetId"]
                    ]
                    exact = source_document.runtime_texture_by_source_path.get(
                        source_path.casefold()
                    )
                    if exact is not None:
                        self.assertEqual(asset_id, exact)
                    if asset_id:
                        self.assertTrue(MODULE.resource_path(asset_id).is_file())
                    if texture.get("name") == "11.map_b":
                        map_b_assets[(source_path, asset_id)] += 1

        self.assertEqual(
            class_coverage,
            {
                "ARTIST": Counter(
                    canonical=0,
                    matched=639,
                    resolved=639,
                    joined=611,
                    promoted=27,
                    replaced=0,
                    enabled=638,
                    ready=447,
                    blocked=191,
                ),
                "DIMENSIONMASTER": Counter(
                    canonical=784,
                    matched=779,
                    resolved=779,
                    joined=0,
                    promoted=25,
                    replaced=19,
                    enabled=784,
                    ready=597,
                    blocked=187,
                ),
                "LANCE_MASTER": Counter(
                    canonical=0,
                    matched=1989,
                    resolved=1989,
                    joined=1904,
                    promoted=241,
                    replaced=0,
                    enabled=1925,
                    ready=1642,
                    blocked=283,
                ),
                "WARLORD": Counter(
                    canonical=0,
                    matched=1230,
                    resolved=1230,
                    joined=1215,
                    promoted=79,
                    replaced=0,
                    enabled=1215,
                    ready=986,
                    blocked=229,
                ),
            },
        )
        self.assertEqual(
            map_b_assets[
                (
                    "fx_tex_02.fx_d_environ_067",
                    "Effect/Artist/Textures/fx_d_environ_067.dds",
                )
            ],
            14,
        )
        self.assertEqual(
            map_b_assets[
                (
                    "fx_tex_02.fx_d_typical_006",
                    "Effect/Artist/Textures/fx_d_typical_006.dds",
                )
            ],
            3,
        )
        self.assertFalse(
            any(
                asset_id.startswith("Effect/DimensionMaster/")
                for _, asset_id in map_b_assets
            ),
            "cross-class named textures must be rebound to occurrence-local assets",
        )

        dm_a = self.receipt_by_target[
            "effect.dimensionmaster.skill.2050210.unified"
        ]
        self.assertEqual(dm_a["sourceProfileEnabledCount"], 100)
        self.assertEqual(dm_a["sourceProfileReadyCount"], 86)
        self.assertEqual(dm_a["sourceProfileBlockedCount"], 14)

    def test_object_name_only_named_texture_aliases_are_fail_closed(self) -> None:
        rows = [
            (target, row)
            for target in self.receipt["targets"]
            for row in target["particleRows"]
            if row["nonExactNamedTextureAliasCount"] > 0
        ]
        self.assertEqual(len(rows), 30)
        self.assertEqual(
            sum(row["nonExactNamedTextureAliasCount"] for _, row in rows),
            47,
        )
        for target, row in rows:
            self.assertEqual(target["characterClass"], "DIMENSIONMASTER")
            self.assertIn(
                "NON_EXACT_NAMED_TEXTURE_ALIAS", row["failClosedReasons"]
            )
            element = next(
                element
                for element in self.documents[target["targetEffectAssetId"]][
                    "elements"
                ]
                if element["id"] == row["targetElementId"]
            )
            self.assertFalse(element["visible"])
            self.assertEqual(
                element["material"]["execution"], MODULE.FAIL_CLOSED_EXECUTION
            )

    def test_engine_builtin_materials_remain_durably_fail_closed(self) -> None:
        builtin_rows = []
        for target in self.receipt["targets"]:
            for row in target["particleRows"]:
                source_element = self.source_by_effect[
                    row["sourceEffectAssetId"]
                ].elements[row["sourceElementId"]]
                source_path = str(
                    source_element.get("material", {}).get(
                        "sourceMaterialPath", ""
                    )
                )
                if not source_path.casefold().startswith("enginematerials."):
                    continue
                builtin_rows.append(row)
                self.assertIn(
                    "SOURCE_MATERIAL_ENGINE_BUILTIN_POLICY_NOT_AUTHORED",
                    row["failClosedReasons"],
                )
        self.assertEqual(len(builtin_rows), 15)

    def test_source_profile_readiness_matches_runtime_contract_edges(self) -> None:
        def material(profile_id: str, textures: list[dict[str, str]]) -> dict:
            return {
                "templateId": "effect.standard",
                "sourceProfile": {
                    "enabled": True,
                    "runtimeShaderProfileId": profile_id,
                    "textures": textures,
                    "scalars": [],
                    "vectors": [],
                },
            }

        ready, _, reason = MODULE.source_profile_readiness(
            material("effect.ue3.unknown.v1", []),
            [{"slotId": "base", "assetId": "Effect/safe.dds"}],
            "sprite",
        )
        self.assertFalse(ready)
        self.assertTrue(reason.startswith("SOURCE_PROFILE_UNSUPPORTED:"))

        ready, _, reason = MODULE.source_profile_readiness(
            material(MODULE.FALLBACK_BLOCKED_PROFILE, []), [], "sprite"
        )
        self.assertFalse(ready)
        self.assertEqual(reason, "SOURCE_PROFILE_FALLBACK_BLOCKED")

        ready, _, _ = MODULE.source_profile_readiness(
            material("effect.ue3.reconstructed-standard.v1", []),
            [{"slotId": "base", "assetId": "Effect/blankwhite.dds"}],
            "sprite",
        )
        self.assertFalse(ready)

        blackline_textures = [
            {"name": name, "assetId": f"Effect/{name}.dds"}
            for name in MODULE.BLACKLINE_NAMED_TEXTURES
        ]
        ready, owns, _ = MODULE.source_profile_readiness(
            material(MODULE.BLACKLINE_PROFILE, blackline_textures),
            [{"slotId": "meshModel", "assetId": "Effect/model.wmodel"}],
            "mesh",
        )
        self.assertTrue(ready)
        self.assertTrue(owns)

        local_crack_textures = [
            {
                "name": name,
                "assetId": f"Effect/{name}.dds",
                "samplingEvidence": "legacy_default",
            }
            for name in MODULE.LOCAL_CRACK_NAMED_TEXTURES
        ]
        ready, _, _ = MODULE.source_profile_readiness(
            material(MODULE.LOCAL_CRACK_PROFILE, local_crack_textures),
            [{"slotId": "meshModel", "assetId": "Effect/model.wmodel"}],
            "mesh",
        )
        self.assertFalse(ready)

    def test_material_identity_is_invariant_and_occurrence_overlay_is_local(self) -> None:
        identity = {
            "profileId": "ue3.material.test",
            "runtimeShaderProfileId": MODULE.GROUPED_TRANSLUCENT_PROFILE,
            "parentMaterialPath": "fx_m.test_parent",
            "sourceParameters": {
                "textures": [],
                "scalars": [{"name": "alpha", "value": 1.0}],
                "vectors": [],
                "staticSwitches": [],
            },
        }

        def element(parameter_name: str) -> dict:
            return {
                "sourceRecipe": {
                    "modules": [
                        {
                            "className": "ParticleModuleParameterDynamic",
                            "literals": [
                                {
                                    "propertyPath": "DynamicParams[0].ParamName",
                                    "value": parameter_name,
                                }
                            ],
                        }
                    ]
                }
            }

        opacity = MODULE.material_contract_profile(identity, element("opacity"))
        dissolve = MODULE.material_contract_profile(identity, element("dissolve"))
        self.assertEqual(
            {key: value for key, value in opacity.items() if key != "dynamicParameterSemantics"},
            {key: value for key, value in dissolve.items() if key != "dynamicParameterSemantics"},
        )
        self.assertEqual(opacity["dynamicParameterSemantics"][0], "opacity")
        self.assertEqual(dissolve["dynamicParameterSemantics"][0], "dissolve")
        self.assertNotIn("sourceResourceBindings", opacity)

    def test_exact_admitted_contract_replaces_only_matching_enabled_fallback(
        self,
    ) -> None:
        authored_profile = {
            "enabled": True,
            "profileId": "ue3.material.exact",
            "runtimeShaderProfileId": MODULE.FALLBACK_BLOCKED_PROFILE,
            "parentMaterialPath": "fx_m.parent",
            "semanticStatus": "artist_preserved",
            "textures": [],
            "scalars": [
                {"name": "alpha", "value": 0.75, "artistNote": "keep"}
            ],
            "vectors": [],
            "staticSwitches": [],
            "dynamicParameterSemantics": [
                "opacity",
                "unbound",
                "unbound",
                "unbound",
            ],
            "subUVMode": "none",
            "artistOnly": {"tuned": True},
        }
        admitted_contract = {
            "profileId": "ue3.material.exact",
            "runtimeShaderProfileId": MODULE.GROUPED_TRANSLUCENT_PROFILE,
            "parentMaterialPath": "fx_m.parent",
            "sourceEvidenceResolved": True,
            "productAdmissionStatus": "ADMITTED_RECONSTRUCTED_PROFILE",
            "sourceParameters": {
                "textures": [
                    {
                        "name": "01.map_a",
                        "texture": "fx_tex.exact",
                        "sourceObjectPath": "fx_tex.exact",
                        "assetId": "Effect/Test/exact.dds",
                    }
                ],
                "scalars": [{"name": "alpha", "value": 1.0}],
                "vectors": [],
                "staticSwitches": [],
            },
        }
        source_element = {"sourceRecipe": {"modules": []}}

        replaced, did_replace = (
            MODULE.exact_admitted_canonical_fallback_profile(
                authored_profile, admitted_contract, source_element
            )
        )
        self.assertTrue(did_replace)
        self.assertIsNotNone(replaced)
        assert replaced is not None
        self.assertEqual(
            replaced["runtimeShaderProfileId"],
            MODULE.GROUPED_TRANSLUCENT_PROFILE,
        )
        self.assertEqual(replaced["semanticStatus"], "artist_preserved")
        self.assertEqual(replaced["artistOnly"], {"tuned": True})
        self.assertEqual(replaced["scalars"][0]["value"], 1.0)
        self.assertEqual(replaced["scalars"][0]["artistNote"], "keep")
        self.assertEqual(len(replaced["textures"]), 1)

        negative_profiles = []
        non_fallback = copy.deepcopy(authored_profile)
        non_fallback["runtimeShaderProfileId"] = MODULE.GROUPED_TRANSLUCENT_PROFILE
        negative_profiles.append((non_fallback, admitted_contract))
        disabled = copy.deepcopy(authored_profile)
        disabled["enabled"] = False
        negative_profiles.append((disabled, admitted_contract))
        mismatched_profile = copy.deepcopy(admitted_contract)
        mismatched_profile["profileId"] = "ue3.material.other"
        negative_profiles.append((authored_profile, mismatched_profile))
        mismatched_parent = copy.deepcopy(admitted_contract)
        mismatched_parent["parentMaterialPath"] = "fx_m.other"
        negative_profiles.append((authored_profile, mismatched_parent))
        blocked_contract = copy.deepcopy(admitted_contract)
        blocked_contract["runtimeShaderProfileId"] = MODULE.FALLBACK_BLOCKED_PROFILE
        blocked_contract["productAdmissionStatus"] = "BLOCKED_FALLBACK_PROFILE"
        negative_profiles.append((authored_profile, blocked_contract))

        for existing, contract in negative_profiles:
            untouched, did_replace = (
                MODULE.exact_admitted_canonical_fallback_profile(
                    existing, contract, source_element
                )
            )
            self.assertFalse(did_replace)
            self.assertEqual(untouched, existing)

    def test_dimensionmaster_canonical_bridge_is_exact_and_class_scoped(self) -> None:
        canonical_count = sum(
            len(document.canonical_materials)
            for document in self.source_by_effect.values()
        )
        self.assertEqual(canonical_count, 799)
        dm_v11 = self.source_by_effect[
            "effect.dimensionmaster.skill.2050540.imported"
        ]
        self.assertTrue(dm_v11.canonical_materials)
        self.assertTrue(
            all(
                not document.canonical_materials
                for document in self.source_by_effect.values()
                if document.character_class != "DIMENSIONMASTER"
            )
        )

    def _dm_a_canonical_alias_fixture(self) -> tuple[
        list[dict], dict, dict, dict
    ]:
        source = self.source_by_effect[
            MODULE.DM_A_CANONICAL_SOURCE_EFFECT_ID
        ]
        return (
            copy.deepcopy(source.document["elements"]),
            MODULE.load_json(MODULE.DM_A_CANONICAL_V12_PATH),
            MODULE.load_json(
                MODULE.CLASS_RESOURCE_MANIFESTS["DIMENSIONMASTER"]
            ),
            MODULE.load_json(MODULE.DM_A_TEXTURE_SAMPLING_EVIDENCE_PATH),
        )

    def test_dm_a_canonical_v12_package_alias_is_exact_and_provenanced(
        self,
    ) -> None:
        source_elements, canonical, manifest, sampling = (
            self._dm_a_canonical_alias_fixture()
        )
        authority = (
            MODULE.build_dm_a_canonical_v12_package_alias_authority(
                "DIMENSIONMASTER",
                MODULE.DM_A_CANONICAL_SOURCE_EFFECT_ID,
                source_elements,
                canonical,
                MODULE.raw_sha256(MODULE.DM_A_CANONICAL_V12_PATH),
                manifest,
                sampling,
            )
        )
        self.assertEqual(len(authority), 20)
        self.assertTrue(
            all(
                row["provenance"]
                == MODULE.CANONICAL_V12_PACKAGE_QUALIFIED_ASSET_ID
                and row["canonicalRawSha256"]
                == MODULE.DM_A_CANONICAL_V12_RAW_SHA256
                and row["qualifiedSourceObjectPath"].casefold()
                == (
                    f"{row['logicalPackage']}.{row['objectName']}"
                ).casefold()
                and MODULE.resource_path(row["assetId"]).is_file()
                for row in authority.values()
            )
        )
        self.assertEqual(
            Counter(row["qualifiedSourceObjectPath"].casefold() for row in authority.values()),
            Counter(
                {
                    "fx_tex_04.fx_h_atypical_01_1": 16,
                    "fx_tex_05.fx_l_environment_001": 4,
                }
            ),
        )

        target = self.receipt_by_target[
            "effect.dimensionmaster.skill.2050210.unified"
        ]
        golden_rows = [
            row
            for row in target["particleRows"]
            if row["sourceElementId"].split(".event_source-event-", 1)[0]
            in MODULE.DM_A_GOLDEN_SOURCE_ELEMENT_BASE_IDS
        ]
        self.assertEqual(len(golden_rows), 24)
        self.assertEqual(sum(not row["failClosedReasons"] for row in golden_rows), 24)
        self.assertEqual(sum(bool(row["failClosedReasons"]) for row in golden_rows), 0)
        authority_rows = [
            row
            for row in golden_rows
            if row["canonicalV12PackageQualifiedAssetCount"]
        ]
        self.assertEqual(len(authority_rows), 20)
        self.assertTrue(
            all(
                row["canonicalV12PackageQualifiedAssetCount"] == 1
                and row["nonExactNamedTextureAliasCount"] == 0
                and row["canonicalV12PackageQualifiedAssets"][0][
                    "provenance"
                ]
                == MODULE.CANONICAL_V12_PACKAGE_QUALIFIED_ASSET_ID
                for row in authority_rows
            )
        )
        counts = self.receipt["counts"]
        self.assertEqual(counts["dimensionMaster2050210GoldenElementCount"], 24)
        self.assertEqual(counts["dimensionMaster2050210GoldenAdmittedCount"], 24)
        self.assertEqual(counts["dimensionMaster2050210GoldenFailClosedCount"], 0)
        self.assertFalse(self.receipt["productMutation"])

    def test_dm_a_canonical_v12_package_alias_rejects_all_identity_drift(
        self,
    ) -> None:
        def build(
            source_elements: list[dict],
            canonical: dict,
            manifest: dict,
            sampling: dict,
            *,
            raw_sha256: str = MODULE.DM_A_CANONICAL_V12_RAW_SHA256,
        ) -> dict:
            return MODULE.build_dm_a_canonical_v12_package_alias_authority(
                "DIMENSIONMASTER",
                MODULE.DM_A_CANONICAL_SOURCE_EFFECT_ID,
                source_elements,
                canonical,
                raw_sha256,
                manifest,
                sampling,
            )

        source, canonical, manifest, sampling = (
            self._dm_a_canonical_alias_fixture()
        )
        with self.assertRaisesRegex(MODULE.RestorationError, "raw SHA drifted"):
            build(
                source,
                canonical,
                manifest,
                sampling,
                raw_sha256="0" * 64,
            )

        source, canonical, manifest, sampling = (
            self._dm_a_canonical_alias_fixture()
        )
        group = (
            "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1."
            "particlespriteemitter_14"
        )
        for element in canonical["elements"]:
            if element.get("id") not in {
                group + suffix for suffix in MODULE.DM_A_GOLDEN_EVENT_SUFFIXES
            }:
                continue
            texture = next(
                row
                for row in element["material"]["sourceProfile"]["textures"]
                if row.get("name") == "dissolve_tex"
            )
            texture["assetId"] = (
                "Effect/DimensionMaster/Textures/FX_TEX_04/"
                "wrong_object.dds"
            )
        with self.assertRaisesRegex(
            MODULE.RestorationError, "package/object identity mismatch"
        ):
            build(source, canonical, manifest, sampling)

        for disposition in ("missing", "duplicate"):
            with self.subTest(manifest=disposition):
                source, canonical, manifest, sampling = (
                    self._dm_a_canonical_alias_fixture()
                )
                exact_rows = [
                    row
                    for row in manifest["assets"]
                    if row.get("sourceAssetPath", "").casefold()
                    == "fx_tex_04.fx_h_atypical_01_1"
                ]
                self.assertEqual(len(exact_rows), 1)
                if disposition == "missing":
                    manifest["assets"].remove(exact_rows[0])
                else:
                    manifest["assets"].append(copy.deepcopy(exact_rows[0]))
                with self.assertRaisesRegex(
                    MODULE.RestorationError, "manifest row is missing/duplicate"
                ):
                    build(source, canonical, manifest, sampling)

        for field, value in (
            ("physicalPackage", "WRONG_PACKAGE.upk"),
            ("physicalPackageSha256", "0" * 64),
        ):
            with self.subTest(sampling=field):
                source, canonical, manifest, sampling = (
                    self._dm_a_canonical_alias_fixture()
                )
                row = next(
                    row
                    for row in sampling["textures"]
                    if row.get("sourceObjectPath", "").casefold()
                    == "fx_tex_04.fx_h_atypical_01_1"
                )
                row[field] = value
                with self.assertRaisesRegex(
                    MODULE.RestorationError, "sampling package/SHA mismatch"
                ):
                    build(source, canonical, manifest, sampling)

        source, canonical, manifest, sampling = (
            self._dm_a_canonical_alias_fixture()
        )
        with tempfile.TemporaryDirectory() as directory:
            missing_dds = Path(directory) / "missing.dds"
            with mock.patch.object(
                MODULE, "resource_path", return_value=missing_dds
            ):
                with self.assertRaisesRegex(
                    MODULE.RestorationError, "canonical alias DDS is missing"
                ):
                    build(source, canonical, manifest, sampling)

        source, canonical, manifest, sampling = (
            self._dm_a_canonical_alias_fixture()
        )
        drift_id = group + ".event_source-event-030"
        next(
            element
            for element in canonical["elements"]
            if element.get("id") == drift_id
        )["material"]["sourceProfile"]["profileId"] = "drifted.profile"
        with self.assertRaisesRegex(
            MODULE.RestorationError, "profile drifted across event copies"
        ):
            build(source, canonical, manifest, sampling)

        source, canonical, manifest, sampling = (
            self._dm_a_canonical_alias_fixture()
        )
        next(
            element for element in source if element.get("id") == drift_id
        )["material"]["sourceMaterialPath"] = "fx_m.wrong_material"
        with self.assertRaisesRegex(
            MODULE.RestorationError, "material profile drifted across event copies"
        ):
            build(source, canonical, manifest, sampling)

        source, canonical, manifest, sampling = (
            self._dm_a_canonical_alias_fixture()
        )
        self.assertEqual(
            MODULE.build_dm_a_canonical_v12_package_alias_authority(
                "ARTIST",
                MODULE.DM_A_CANONICAL_SOURCE_EFFECT_ID,
                source,
                canonical,
                MODULE.DM_A_CANONICAL_V12_RAW_SHA256,
                manifest,
                sampling,
            ),
            {},
        )
        self.assertEqual(
            MODULE.build_dm_a_canonical_v12_package_alias_authority(
                "DIMENSIONMASTER",
                "effect.dimensionmaster.skill.2050220.imported",
                source,
                canonical,
                MODULE.DM_A_CANONICAL_V12_RAW_SHA256,
                manifest,
                sampling,
            ),
            {},
        )

    def test_missing_mesh_model_is_evidence_not_an_invented_placeholder(self) -> None:
        target = "effect.artist.skill.31200.unified"
        source_element_id = (
            "fx_pc_sdm_04.par_w_sdm_inkpaddle_02.particlespriteemitter_13"
        )
        row = next(
            row
            for row in self.receipt_by_target[target]["particleRows"]
            if row["sourceElementId"] == source_element_id
        )
        staged = next(
            element
            for element in self.documents[target]["elements"]
            if element["id"] == row["targetElementId"]
        )
        self.assertEqual(row["rendererShape"], "mesh")
        self.assertFalse(row["meshResourceValid"])
        self.assertIn("MISSING_EXACT_MESH_MODEL", row["failClosedReasons"])
        self.assertFalse(staged["visible"])
        self.assertFalse(any(r["slotId"] == "meshModel" for r in staged["resources"]))

    def test_decal_and_animation_trail_boundaries_remain_separate(self) -> None:
        decals = [
            element
            for document in self.documents.values()
            for element in document["elements"]
            if element["kind"] == "decal"
        ]
        trails = [
            element
            for document in self.documents.values()
            for element in document["elements"]
            if element["kind"] == "trail"
        ]
        self.assertEqual(len(decals), 79)
        self.assertEqual(
            sum(any(r["slotId"] == "base" for r in row["resources"]) for row in decals),
            46,
        )
        self.assertEqual(len(trails), 11)
        trail_rows = [
            row
            for target in self.receipt["targets"]
            for row in target["animationTrailRows"]
        ]
        self.assertEqual(len(trail_rows), 11)
        self.assertEqual(
            len(
                {
                    (
                        target["characterClass"],
                        target["skillId"],
                        row["sourceEventId"],
                    )
                    for target in self.receipt["targets"]
                    for row in target["animationTrailRows"]
                }
            ),
            8,
        )
        row_by_target_id = {
            row["targetElementId"]: row for row in trail_rows
        }
        for trail in trails:
            row = row_by_target_id[trail["id"]]
            self.assertTrue(trail["visible"])
            self.assertTrue(row["admitted"])
            self.assertEqual(row["failClosedReasons"], [])
            self.assertNotIn("execution", trail["material"])
            self.assertEqual(trail["sourcePresentation"]["status"], "reconstructed")
            self.assertEqual(
                trail["sourcePresentation"]["profileId"],
                "four-class.animation-trail-history.v1",
            )
            self.assertFalse(trail["sourceRecipe"]["enabled"])
            self.assertIn("particlemoduletypedataanimtrail", row["sourceTypedDataPath"])
            self.assertNotIn("particlemoduletypedataribbon", row["sourceTypedDataPath"])
            self.assertTrue(trail["actionCueAttachment"]["follow"])
            self.assertEqual(
                trail["actionCueAttachment"]["runtimeBoneName"],
                "b_weapon_rhand",
            )
            for binding in trail["resources"]:
                self.assertTrue(MODULE.resource_path(binding["assetId"]).is_file())

    def test_character_ghost_receipt_is_byte_exact_and_never_a_trail_element(
        self,
    ) -> None:
        rows = self.receipt["characterGhostCues"]
        self.assertEqual(self.receipt["counts"]["characterGhostCueCount"], 72)
        self.assertEqual(self.receipt["counts"]["characterGhostTargetCount"], 29)
        self.assertEqual(len(rows), 72)
        self.assertEqual(
            Counter(row["characterClass"] for row in rows),
            Counter(
                {
                    "ARTIST": 1,
                    "DIMENSIONMASTER": 41,
                    "LANCE_MASTER": 24,
                    "WARLORD": 6,
                }
            ),
        )
        self.assertEqual(
            len({row["targetEffectAssetId"] for row in rows}), 29
        )
        self.assertEqual(len({row["cueId"] for row in rows}), 72)
        joins = {
            (row["characterClass"], row["skillId"], row["sourceEventId"])
            for row in rows
        }
        self.assertEqual(len(joins), 72)
        self.assertTrue(
            all(row["semanticFamily"] == "CHARACTER_AFTERIMAGE" for row in rows)
        )
        self.assertTrue(all(not row["admitted"] for row in rows))
        self.assertTrue(
            all(
                row["admission"]
                == {
                    "admitted": False,
                    "failClosed": True,
                    "blocker": (
                        "POSE_RUNTIME_BODY_EQUIPMENT_SNAPSHOT_AND_GHOST_"
                        "MATERIAL_EXECUTION_UNAVAILABLE"
                    ),
                }
                for row in rows
            )
        )
        for row in rows:
            payload = row["serializedPayload"]
            decoded = base64.b64decode(payload["data"], validate=True)
            self.assertEqual(len(decoded), payload["byteSize"])
            self.assertEqual(hashlib.sha256(decoded).hexdigest(), payload["sha256"])
            artifact = row["sourceArtifact"]
            if artifact["kind"] == "EXISTING_ACTION_CUE_RECIPE":
                artifact_path = MODULE.repository_path(
                    artifact["path"], "character ghost recipe"
                )
            else:
                source = MODULE.CHARACTER_GHOST_ACTION_SOURCES[
                    row["characterClass"]
                ]
                self.assertEqual(
                    artifact["path"], f"XmlData/Action/{source['filename']}"
                )
                artifact_path = (
                    MODULE.CHARACTER_GHOST_ACTION_SOURCE_ROOT
                    / source["filename"]
                )
            self.assertEqual(MODULE.raw_sha256(artifact_path), artifact["rawSha256"])
            receipt_artifact = artifact["sourceReceipt"]
            receipt_path = MODULE.repository_path(
                receipt_artifact["path"], "character ghost source receipt"
            )
            self.assertEqual(
                MODULE.raw_sha256(receipt_path), receipt_artifact["rawSha256"]
            )

        element_payload = json.dumps(
            [
                element
                for document in self.documents.values()
                for element in document["elements"]
            ],
            ensure_ascii=False,
            sort_keys=True,
        )
        self.assertNotIn("TrailGhostEffect", element_payload)
        self.assertNotIn("CHARACTER_AFTERIMAGE", element_payload)
        self.assertEqual(
            sum(
                element["kind"] == "trail"
                for document in self.documents.values()
                for element in document["elements"]
            ),
            11,
        )

    def test_dimension_summon_model_cue_alpha_mode_is_exact_and_scoped(self) -> None:
        target = self.documents[MODULE.DIMENSION_SUMMON_MODEL_CUE_TARGET]
        cue = next(
            cue
            for cue in target["modelCues"]
            if cue["cueId"] == MODULE.DIMENSION_SUMMON_MODEL_CUE_ID
        )
        self.assertEqual(cue["modelAssetId"], MODULE.DIMENSION_SUMMON_MODEL_ASSET_ID)
        self.assertEqual(cue["alphaMode"], "MASKED")
        self.assertEqual(self.receipt["counts"]["modelCueAlphaModePinnedCount"], 1)
        pinned = [
            row
            for target_receipt in self.receipt["targets"]
            for row in target_receipt["modelCueRows"]
        ]
        self.assertEqual(
            pinned,
            [
                {
                    "cueId": MODULE.DIMENSION_SUMMON_MODEL_CUE_ID,
                    "modelAssetId": MODULE.DIMENSION_SUMMON_MODEL_ASSET_ID,
                    "clipName": MODULE.DIMENSION_SUMMON_CLIP_NAME,
                    "alphaMode": "MASKED",
                    "provenance": "RAW_UE3_PARENT_BLEND_MASKED_EXACT",
                    "policy": (
                        "STABLE_CUE_MODEL_AND_CLIP_ID_ONLY_NO_GLOBAL_DEFAULT"
                    ),
                    "sourceMaskContract": MODULE.dimension_summon_mask_contract(),
                }
            ],
        )

    def test_animation_trail_reimport_preserves_artist_tuning(self) -> None:
        assignment = next(
            row
            for rows in self.source_animation_trails.values()
            for row in rows
            if row.character_class == "LANCE_MASTER"
        )
        record = self.record_by_target[assignment.target_effect_id]
        pristine, _ = MODULE.materialize_source_animation_trail(
            record, assignment, self.animation_trail_template, None
        )
        pristine_reimport, pristine_receipt = (
            MODULE.materialize_source_animation_trail(
                record, assignment, self.animation_trail_template, pristine
            )
        )
        self.assertEqual(pristine_reimport, pristine)
        self.assertFalse(pristine_receipt["stableReimportPreserved"])
        tuned = copy.deepcopy(pristine)
        tuned["detail"]["transform"]["position"] = [0.1, 0.2, 0.3]
        tuned["detail"]["trail"]["startWidth"] = 0.77
        tuned["visible"] = False
        tuned["resources"][0]["assetId"] = (
            "Effect/LanceMaster/Textures/fx_m_trail_006.dds"
        )
        rebound, receipt = MODULE.materialize_source_animation_trail(
            record, assignment, self.animation_trail_template, tuned
        )
        self.assertEqual(rebound["detail"], tuned["detail"])
        self.assertFalse(rebound["visible"])
        self.assertEqual(
            next(
                row["assetId"]
                for row in rebound["resources"]
                if row["slotId"] == "base"
            ),
            tuned["resources"][0]["assetId"],
        )
        self.assertEqual(
            rebound["sourcePresentation"]["sourceEventId"],
            assignment.source_event_id,
        )
        self.assertTrue(receipt["stableReimportPreserved"])

    def test_stable_reimport_preserves_artist_fields_only_after_stable_id(self) -> None:
        assignment = next(
            row
            for rows in self.source_particles.values()
            for row in rows
            if row.source_document.elements[row.source_element_id]["sourceRecipe"].get(
                "rendererShape"
            )
            == "sprite"
            and any(
                binding.get("slotId") == "base"
                for binding in row.source_document.elements[row.source_element_id][
                    "resources"
                ]
            )
        )
        decision = self.drawable_decisions.get(
            (assignment.source_document.effect_id, assignment.source_element_id)
        )
        pristine, _ = MODULE.materialize_source_particle(
            assignment, decision, None
        )
        tuned = copy.deepcopy(pristine)
        tuned["detail"]["transform"]["position"] = [1.0, 2.0, 3.0]
        tuned["visible"] = False
        rebound, receipt = MODULE.materialize_source_particle(
            assignment, decision, tuned
        )
        self.assertEqual(
            rebound["detail"]["transform"]["position"], [1.0, 2.0, 3.0]
        )
        self.assertFalse(rebound["visible"])
        self.assertTrue(receipt["stableReimportPreserved"])

        old_identity = copy.deepcopy(tuned)
        old_identity["id"] = "authored.approx.old"
        with self.assertRaises(MODULE.RestorationError):
            MODULE.materialize_source_particle(assignment, decision, old_identity)

    def test_transaction_rolls_back_mid_commit(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_name:
            root = Path(temporary_name)
            first = root / "a.json"
            second = root / "b.json"
            created = root / "c.json"
            first.write_text('{"value":"old-a"}\n', encoding="utf-8")
            second.write_text('{"value":"old-b"}\n', encoding="utf-8")
            outputs = {
                first: '{"value":"new-a"}\n',
                second: '{"value":"new-b"}\n',
                created: '{"value":"new-c"}\n',
            }
            real_replace = MODULE.os.replace
            replace_count = 0

            def fail_fourth_replace(source: object, destination: object) -> None:
                nonlocal replace_count
                replace_count += 1
                if replace_count == 4:
                    raise OSError("fixture mid-commit failure")
                real_replace(source, destination)

            with mock.patch.object(
                MODULE.os, "replace", side_effect=fail_fourth_replace
            ):
                with self.assertRaises(OSError):
                    MODULE.commit_transaction(outputs)

            self.assertEqual(
                json.loads(first.read_text(encoding="utf-8")), {"value": "old-a"}
            )
            self.assertEqual(
                json.loads(second.read_text(encoding="utf-8")), {"value": "old-b"}
            )
            self.assertFalse(created.exists())

    def test_receipt_is_canonical_round_trip(self) -> None:
        receipt = copy.deepcopy(self.receipt)
        artifact_sha = receipt.pop("artifactSha256")
        self.assertEqual(artifact_sha, MODULE.canonical_sha256(receipt))
        serialized = MODULE.serialized({**receipt, "artifactSha256": artifact_sha})
        self.assertEqual(
            json.loads(serialized), {**receipt, "artifactSha256": artifact_sha}
        )


if __name__ == "__main__":
    unittest.main()
