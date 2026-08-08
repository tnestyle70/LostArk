#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
from tempfile import TemporaryDirectory
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
            [{
                "name": "power",
                "group": "",
                "defaultValue": None,
                "value": 7.0,
                "valueSource": "INSTANCE_OVERRIDE_WITHOUT_PARENT_DECLARATION",
            }],
            identity["sourceParameters"]["scalars"],
        )

    def test_parent_defaults_groups_merge_case_insensitive_instance_overrides(self):
        effect = {
            "effectAssetId": "effect.test",
            "elements": [{
                "id": "p0", "kind": "particle", "resources": [],
                "material": {
                    "templateId": "effect.source_material",
                    "sourceMaterialPath": "fx_pkg.fx_mi.child",
                },
                "sourceRecipe": {"modules": []},
            }],
        }
        graph = {
            "materialParameterBindings": [{
                "sourceMaterialPath": "fx_pkg.fx_mi.child",
                "sourcePhysicalPackage": "fx.upk",
                "parent": "fx_m.parent",
            }],
            "runtimeResourceBindings": [{
                "sourceObjectPath": "fx_tex.fx_base_override",
                "assetId": "Effect/base.dds",
                "resolutionStatus": "RESOLVED_RUNTIME_ASSET",
            }],
        }
        material_map = {"materials": {"fx_mi.child": [{
            "source_file": "fx.upk",
            "material_path": "fx_mi.child",
            "parent": "fx_m.parent",
            "textures": [{
                "name": "BASE_TEX", "texture": "fx_tex.fx_base_override",
            }],
            "scalars": [{"name": "emissive_power", "value": 4.0}],
            "vectors": [{
                "name": "TINT", "value": {"r": 0.5, "g": 1.0, "b": 2.0, "a": 1.0},
            }],
            "static_switches": [{"name": "Use_Distortion", "value": True}],
            "materialEvidence": {
                "collectedTextureParameters": [{
                    "name": "base_tex", "group": "base", "texture": "fx_tex.fx_base_default",
                }],
                "collectedScalarParameters": [{
                    "name": "Emissive_Power", "group": "emissive", "value": 1.0,
                }],
                "collectedVectorParameters": [{
                    "name": "tint", "group": "color",
                    "value": {"r": 1.0, "g": 1.0, "b": 1.0, "a": 1.0},
                }],
                "collectedStaticSwitchParameters": [{
                    "name": "use_distortion", "group": "distortion", "value": False,
                }],
            },
        }]}}

        contract, _ = MODULE.build_contract(
            effect, graph, {"elementConversions": []}, {"assets": []}, material_map
        )
        identity = contract["materialIdentities"][0]
        self.assertEqual(
            "effect.ue3.grouped-translucent.v1",
            identity["runtimeShaderProfileId"],
        )
        self.assertEqual(
            [{
                "name": "Emissive_Power", "group": "emissive",
                "defaultValue": 1.0, "value": 4.0,
                "valueSource": "INSTANCE_OVERRIDE",
            }],
            identity["sourceParameters"]["scalars"],
        )
        self.assertEqual(
            "fx_tex.fx_base_default",
            identity["sourceParameters"]["textures"][0]["defaultValue"],
        )
        self.assertEqual(
            "fx_tex.fx_base_override",
            identity["sourceParameters"]["textures"][0]["texture"],
        )
        upgraded = MODULE.upgrade_effect_document(effect, contract)
        self.assertEqual(
            [{
                "name": "tint", "group": "color",
                "value": [0.5, 1.0, 2.0, 1.0],
            }],
            upgraded["elements"][0]["material"]["sourceProfile"]["vectors"],
        )
        self.assertNotIn(
            "sourceParameters",
            upgraded["elements"][0]["material"]["sourceProfile"],
        )

    def test_unknown_or_missing_grouped_resource_fails_closed(self):
        selection = MODULE.grouped_translucent_selection(
            "", "", [], [{
                "parameterName": "base_tex", "parentGroup": "base",
                "status": "MISSING_RUNTIME_ASSET",
            }],
        )
        self.assertEqual(
            (
                "effect.ue3.fallback-blocked.v1",
                "MISSING_GROUPED_TRANSPARENT_RUNTIME_RESOURCE",
            ), selection,
        )
        self.assertEqual(
            ("effect.ue3.fallback-blocked.v1", "UNKNOWN_GROUPED_TRANSPARENT_INPUT"),
            MODULE.grouped_translucent_selection("", "", [], []),
        )

    def test_safe_existing_carrier_enables_bounded_grouped_reconstruction(self):
        self.assertEqual(
            ("effect.ue3.grouped-translucent.v1", None),
            MODULE.grouped_translucent_selection(
                "", "", [], [], True,
                [{"slotId": "base", "assetId": "Effect/safe_glow.dds"}],
            ),
        )
        self.assertEqual(
            (
                "effect.ue3.fallback-blocked.v1",
                "UNKNOWN_GROUPED_TRANSPARENT_INPUT",
            ),
            MODULE.grouped_translucent_selection(
                "", "", [], [], True,
                [{"slotId": "noise", "assetId": "Effect/noise.dds"}],
            ),
        )
        self.assertEqual(
            (
                "effect.ue3.fallback-blocked.v1",
                "UNKNOWN_GROUPED_TRANSPARENT_INPUT",
            ),
            MODULE.grouped_translucent_selection(
                "", "", [], [], True,
                [{
                    "slotId": "base",
                    "assetId": "Effect/fx_a_blankwhite_01.dds",
                }],
            ),
        )
        self.assertEqual(
            ("effect.ue3.grouped-translucent.v1", None),
            MODULE.grouped_translucent_selection(
                "", "", [], [], True,
                [
                    {
                        "slotId": "base",
                        "assetId": "Effect/fx_a_blankwhite_01.dds",
                    },
                    {"slotId": "mask", "assetId": "Effect/alpha_mask.dds"},
                ],
            ),
        )
        self.assertEqual(
            (
                "effect.ue3.fallback-blocked.v1",
                "UNRESOLVED_OR_AMBIGUOUS_MATERIAL_EXPORT",
            ),
            MODULE.grouped_translucent_selection(
                "", "UNRESOLVED_OR_AMBIGUOUS_MATERIAL_EXPORT", [], [], True,
                [{"slotId": "base", "assetId": "Effect/unsafe_guess.dds"}],
            ),
        )

    def test_existing_finite_profile_precedes_parent_props_fallback(self):
        self.assertEqual(
            ("effect.ue3.aura.v1", None),
            MODULE.grouped_translucent_selection(
                "effect.ue3.aura.v1", "MISSING_PARENT_PROPS", [], []
            ),
        )

    def test_package_prefixed_parent_uses_existing_one_layer_distortion_profile(self):
        self.assertEqual(
            "effect.ue3.one-layer-distortion.v1",
            MODULE.runtime_shader_profile_id(
                "fx_mastermaterial.fx_mm.fx_mm_onelayerdistortion_02_ad",
                "fx_mastermaterial.fx_mi.fx_mm_onelayerdistortion_02_01_ad",
            ),
        )
        self.assertEqual(
            "",
            MODULE.runtime_shader_profile_id(
                "fx_mastermaterial.fx_mm.not_fx_mm_onelayerdistortion_02_ad",
                "",
            ),
        )
        self.assertEqual(
            (
                "effect.ue3.fallback-blocked.v1",
                "MISSING_EXISTING_FINITE_PROFILE_REQUIRED_RUNTIME_RESOURCE",
            ),
            MODULE.grouped_translucent_selection(
                "effect.ue3.aura.v1", "MISSING_PARENT_PROPS", [], [], False
            ),
        )

    def test_dimensionmaster_exact_parent_profiles_are_finite_and_bounded(self):
        cases = [
            (
                "fx_m_mi_02.fx_m.fx_f_pa_shine_01_0_tr",
                "effect.ue3.shine.v1",
            ),
            (
                "fx_m_mi_j_00.fx_m.fx_j_pa_blacklineaura_01_tr",
                "effect.ue3.blackline-aura.v1",
            ),
            (
                "fx_m_mi_j_00.fx_m.fx_j_pa_linearflow_02_tr",
                "effect.ue3.linearflow-02.v1",
            ),
            (
                "fx_m_mi_j_00.fx_m.fx_j_pa_slice_01_tr",
                "effect.ue3.slice.v1",
            ),
            (
                "fx_m_mi_m_00.fx_m.fx_m_pa_missiletrail_01_tr",
                "effect.ue3.missiletrail-01.v1",
            ),
            (
                "fx_m_mi_j_00.fx_m.fx_j_me_localcrack_01_tr",
                "effect.ue3.local-crack.v1",
            ),
            (
                "bfx_m_mi_00.bfx_m.bfx_i_pa_glow_01_ad",
                "effect.ue3.procedural-center-glow.v1",
            ),
        ]
        for parent, expected in cases:
            with self.subTest(parent=parent):
                self.assertEqual(
                    expected,
                    MODULE.runtime_shader_profile_id(parent, ""),
                )
                self.assertEqual(
                    (expected, None),
                    MODULE.grouped_translucent_selection(
                        expected, "MISSING_PARENT_PROPS", [], []
                    ),
                )

        self.assertEqual(
            "",
            MODULE.runtime_shader_profile_id(
                "fx_m_mi_02.fx_m.not_fx_f_pa_shine_01_0_tr", ""
            ),
        )
        self.assertEqual(
            "",
            MODULE.runtime_shader_profile_id(
                "fx_m_mi_j_00.fx_m.fx_j_me_localcrack_01_tr_variant", ""
            ),
        )

    def test_dimensionmaster_finite_profiles_require_runtime_carriers(self):
        self.assertTrue(MODULE.finite_profile_runtime_resource_contract_satisfied(
            "effect.ue3.shine.v1",
            [
                {"slotId": "base", "assetId": "Effect/shine.dds"},
                {"slotId": "mask", "assetId": "Effect/mask.dds"},
            ],
        ))
        self.assertFalse(MODULE.finite_profile_runtime_resource_contract_satisfied(
            "effect.ue3.shine.v1",
            [{"slotId": "base", "assetId": "Effect/fx_a_blankwhite_01.dds"}],
        ))
        blackline_textures = {
            "textures": [
                {"name": name, "assetId": f"Effect/{name}.dds"}
                for name in (
                    "diffuse_tex", "flow_tex", "mask_a_tex", "mask_b_tex",
                    "dissolve_tex",
                )
            ]
        }
        self.assertTrue(MODULE.finite_profile_runtime_resource_contract_satisfied(
            "effect.ue3.blackline-aura.v1", [], blackline_textures,
        ))
        self.assertTrue(MODULE.finite_profile_runtime_resource_contract_satisfied(
            "effect.ue3.procedural-center-glow.v1", [],
        ))
        local_crack_textures = {
            "textures": [{
                "name": name,
                "assetId": f"Effect/{name}.dds",
                "addressU": "wrap",
                "addressV": "wrap",
                "colorSpace": color_space,
                "samplingEvidence": "ue3_property_or_class_default.v1",
            } for name, color_space in (
                ("normal_tex", "linear"),
                ("refle_tex", "srgb"),
                ("dissolve_tex", "srgb"),
            )]
        }
        local_crack_resources = [
            {"slotId": "meshModel", "assetId": "Effect/crack.wmodel"},
        ]
        self.assertTrue(MODULE.finite_profile_runtime_resource_contract_satisfied(
            "effect.ue3.local-crack.v1",
            local_crack_resources,
            local_crack_textures,
        ))
        local_crack_textures["textures"][1]["samplingEvidence"] = (
            "legacy_default"
        )
        self.assertFalse(MODULE.finite_profile_runtime_resource_contract_satisfied(
            "effect.ue3.local-crack.v1",
            local_crack_resources,
            local_crack_textures,
        ))
        local_crack_textures["textures"][1]["samplingEvidence"] = (
            "ue3_property_or_class_default.v1"
        )
        local_crack_textures["textures"].pop()
        self.assertFalse(MODULE.finite_profile_runtime_resource_contract_satisfied(
            "effect.ue3.local-crack.v1",
            local_crack_resources,
            local_crack_textures,
        ))
        self.assertTrue(MODULE.finite_profile_runtime_resource_contract_satisfied(
            "effect.ue3.slice.v1",
            [{"slotId": "base", "assetId": "Effect/slice.dds"}],
        ))
        self.assertFalse(MODULE.finite_profile_runtime_resource_contract_satisfied(
            "effect.ue3.slice.v1",
            [{"slotId": "noise", "assetId": "Effect/slice.dds"}],
        ))
        missile_resources = [
            {"slotId": "meshModel", "assetId": "Effect/trail.wmodel"},
            {"slotId": "base", "assetId": "Effect/trail.dds"},
            {"slotId": "mask", "assetId": "Effect/trail.dds"},
            {"slotId": "noise", "assetId": "Effect/noise.dds"},
            {"slotId": "dissolve", "assetId": "Effect/dissolve.dds"},
        ]
        self.assertTrue(MODULE.finite_profile_runtime_resource_contract_satisfied(
            "effect.ue3.missiletrail-01.v1", missile_resources,
        ))
        self.assertFalse(MODULE.finite_profile_runtime_resource_contract_satisfied(
            "effect.ue3.missiletrail-01.v1", missile_resources[:-1],
        ))

        linearflow_textures = {
            "textures": [
                {"name": name, "assetId": f"Effect/{name}.dds"}
                for name in (
                    "diff_tex", "diff_noise_tex", "a_mask_tex",
                    "a_noise_01_tex", "b_mask_tex", "b_noise_01_tex",
                    "dissolve_tex",
                )
            ]
        }
        self.assertTrue(MODULE.finite_profile_runtime_resource_contract_satisfied(
            "effect.ue3.linearflow-02.v1", [], linearflow_textures,
        ))
        linearflow_textures["textures"].pop()
        self.assertFalse(MODULE.finite_profile_runtime_resource_contract_satisfied(
            "effect.ue3.linearflow-02.v1", [], linearflow_textures,
        ))

    def test_local_crack_legacy_sampling_downgrades_to_fallback_blocked(self):
        effect = {
            "effectAssetId": "effect.test.local-crack",
            "elements": [{
                "id": "crack",
                "kind": "particle",
                "resources": [{
                    "slotId": "meshModel",
                    "assetId": "Effect/crack.wmodel",
                }],
                "material": {
                    "sourceMaterialPath": "fx_mi.fx_local_crack",
                },
            }],
        }
        texture_rows = [{
            "name": name,
            "texture": f"fx_tex.{name}",
            "sourceObjectPath": f"fx_tex.{name}",
            "assetId": f"Effect/{name}.dds",
            "addressU": "wrap",
            "addressV": "wrap",
            "colorSpace": color_space,
            "samplingEvidence": evidence,
        } for name, color_space, evidence in (
            ("normal_tex", "linear", "ue3_property_or_class_default.v1"),
            ("refle_tex", "srgb", "legacy_default"),
            ("dissolve_tex", "srgb", "ue3_property_or_class_default.v1"),
        )]
        contract = {"materialIdentities": [{
            "sourceMaterialPath": "fx_mi.fx_local_crack",
            "profileId": "ue3.material.local-crack",
            "runtimeShaderProfileId": "effect.ue3.local-crack.v1",
            "parentMaterialPath": "fx_m.fx_local_crack",
            "sourceParameters": {
                "textures": texture_rows,
                "scalars": [],
                "vectors": [],
                "staticSwitches": [],
            },
            "renderState": {},
        }]}
        upgraded = MODULE.upgrade_effect_document(effect, contract)
        self.assertEqual(
            "effect.ue3.fallback-blocked.v1",
            upgraded["elements"][0]["material"]["sourceProfile"][
                "runtimeShaderProfileId"
            ],
        )
        texture_rows[1]["samplingEvidence"] = (
            "ue3_property_or_class_default.v1"
        )
        upgraded = MODULE.upgrade_effect_document(effect, contract)
        self.assertEqual(
            "effect.ue3.local-crack.v1",
            upgraded["elements"][0]["material"]["sourceProfile"][
                "runtimeShaderProfileId"
            ],
        )

    def test_direct_parent_graph_evidence_adds_only_named_texture_defaults(self):
        selected = {
            "materialEvidence": {
                "collectedTextureParameters": [
                    {
                        "name": "dissolve_tex",
                        "group": "00_dissolve",
                        "texture": "fx_h_atypical_01_1",
                    }
                ]
            }
        }
        graph = {
            "materials": [{
                "materialPath": "fx_m.fx_j_pa_linearflow_02_tr",
                "namedTextures": [
                    {
                        "name": "diff_noise_tex",
                        "group": "01_diffuse",
                        "sourceObjectPath": "fx_tex_00.fx_bg_dustpanner_01",
                    },
                    {
                        "name": "dissolve_tex",
                        "group": "00_dissolve",
                        "sourceObjectPath": "fx_tex_04.fx_h_atypical_01_1",
                    },
                ],
                "summary": {
                    "topologyStatus": "COOKED_PARTIAL",
                    "runtimeExactEligible": False,
                },
            }]
        }
        merged = MODULE.merge_parent_graph_texture_defaults(
            selected,
            "fx_m_mi_j_00.fx_m.fx_j_pa_linearflow_02_tr",
            graph,
        )
        textures = merged["materialEvidence"]["collectedTextureParameters"]
        self.assertEqual(2, len(textures))
        self.assertEqual("diff_noise_tex", textures[1]["name"])
        self.assertEqual(
            "COOKED_PARTIAL",
            merged["materialEvidence"]["cookedGraphTopologyStatus"],
        )
        self.assertFalse(
            merged["materialEvidence"]["cookedGraphRuntimeExactEligible"]
        )
        self.assertEqual(1, len(
            selected["materialEvidence"]["collectedTextureParameters"]
        ))

    def test_existing_finite_profile_required_asset_is_checked_at_resource_root(self):
        bindings = [{"assetId": "Effect/Test/required.dds"}]
        with TemporaryDirectory() as temporary:
            root = Path(temporary)
            self.assertFalse(MODULE.runtime_binding_files_available(bindings, root))
            required = root / "Effect" / "Test"
            required.mkdir(parents=True)
            (required / "required.dds").write_bytes(b"dds")
            self.assertTrue(MODULE.runtime_binding_files_available(bindings, root))

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
            ["dissolve", "uv_pan", "unbound", "unbound"],
            MODULE.dynamic_parameter_semantics(element),
        )
        self.assertEqual(
            "uv_pan", MODULE.classify_dynamic_parameter("alpha_pan")
        )
        self.assertEqual(
            "unbound", MODULE.classify_dynamic_parameter("alpha_rot_time")
        )
        self.assertEqual(
            "dissolve", MODULE.classify_dynamic_parameter("alpha_dissolve")
        )
        missile_element = {
            "sourceRecipe": {
                "modules": [{
                    "className": "ParticleModuleParameterDynamic",
                    "literals": [
                        {"propertyPath": "DynamicParams[0].ParamName",
                         "value": "alpha_pan"},
                        {"propertyPath": "DynamicParams[1].ParamName",
                         "value": "uv_noise_velue"},
                        {"propertyPath": "DynamicParams[2].ParamName",
                         "value": "uv_noise_pan"},
                        {"propertyPath": "DynamicParams[3].ParamName",
                         "value": "alpha_dissolve"},
                    ],
                }]
            }
        }
        self.assertEqual(
            ["missile_alpha_pan", "missile_noise_strength",
             "missile_noise_pan", "missile_dissolve"],
            MODULE.runtime_profile_dynamic_parameter_semantics(
                "effect.ue3.missiletrail-01.v1", missile_element
            ),
        )

    def test_parent_texture_groups_replace_name_heuristic_slots(self):
        selected = {
            "textures": [
                {"name": "21.map_c", "texture": "fx_tex.fx_alpha"},
                {"name": "02.map_e", "texture": "fx_tex.fx_emission_a"},
                {"name": "12.map_f", "texture": "fx_tex.fx_emission_b"},
                {"name": "06.map", "texture": "fx_tex.fx_uv_noise"},
            ],
            "materialEvidence": {
                "collectedTextureParameters": [
                    {"name": "21.map_c", "group": "01_alpha"},
                    {"name": "02.map_e", "group": "02_emission"},
                    {"name": "12.map_f", "group": "02_emission"},
                    {"name": "06.map", "group": "08_uvdistort"},
                ]
            },
        }
        graph = {
            "runtimeResourceBindings": [
                {
                    "sourceObjectPath": "fx_tex.fx_alpha",
                    "assetId": "Effect/alpha.dds",
                    "resolutionStatus": "RESOLVED_RUNTIME_ASSET",
                },
                {
                    "sourceObjectPath": "fx_tex.fx_emission_a",
                    "assetId": "Effect/emission_a.dds",
                    "resolutionStatus": "RESOLVED_RUNTIME_ASSET",
                },
                {
                    "sourceObjectPath": "fx_tex.fx_emission_b",
                    "assetId": "Effect/emission_b.dds",
                    "resolutionStatus": "RESOLVED_RUNTIME_ASSET",
                },
                {
                    "sourceObjectPath": "fx_tex.fx_uv_noise",
                    "assetId": "Effect/uv_noise.dds",
                    "resolutionStatus": "RESOLVED_RUNTIME_ASSET",
                },
            ]
        }

        bindings, diagnostics = MODULE.role_resolved_runtime_bindings(
            selected, graph
        )
        self.assertEqual(
            [
                ("mask", "Effect/alpha.dds"),
                ("noise", "Effect/uv_noise.dds"),
                ("base", "Effect/emission_a.dds"),
                ("emissive", "Effect/emission_b.dds"),
            ],
            [(row["slotId"], row["assetId"]) for row in bindings],
        )
        self.assertTrue(all(
            row["status"] == "PARENT_GROUP_RUNTIME_ASSET"
            for row in diagnostics
        ))

    def test_normal_bump_roles_fail_closed_except_evidence_backed_noise(self):
        selected = {
            "textures": [
                {
                    "name": "normal_tex",
                    "texture": "fx_tex.fx_generic_data",
                },
                {
                    "name": "uv_noise_tex",
                    "texture": "fx_tex.fx_distortion_normal",
                },
            ],
            "materialEvidence": {
                "collectedTextureParameters": [
                    {"name": "normal_tex", "group": "01_alpha"},
                    {"name": "uv_noise_tex", "group": "08_uvdistort"},
                ],
            },
        }
        graph = {"runtimeResourceBindings": [
            {
                "sourceObjectPath": "fx_tex.fx_generic_data",
                "assetId": "Effect/fx_generic_data.dds",
                "resolutionStatus": "RESOLVED_RUNTIME_ASSET",
            },
            {
                "sourceObjectPath": "fx_tex.fx_distortion_normal",
                "assetId": "Effect/fx_distortion_normal.dds",
                "resolutionStatus": "RESOLVED_RUNTIME_ASSET",
            },
        ]}

        bindings, diagnostics = MODULE.role_resolved_runtime_bindings(
            selected, graph
        )
        self.assertEqual(
            [("noise", "Effect/fx_distortion_normal.dds")],
            [(row["slotId"], row["assetId"]) for row in bindings],
        )
        by_name = {row["parameterName"]: row for row in diagnostics}
        self.assertEqual(
            "BLOCKED_NORMAL_BUMP_NON_NOISE_ROLE",
            by_name["normal_tex"]["status"],
        )
        self.assertEqual(
            "EVIDENCE_BACKED_NOISE_DISTORTION",
            by_name["uv_noise_tex"]["normalBumpPolicy"],
        )

        current = [
            {"slotId": slot, "assetId": "Effect/fx_generic_data.dds"}
            for slot in ("base", "mask", "emissive", "dissolve", "noise")
        ] + [{
            "slotId": "noise",
            "assetId": "Effect/fx_distortion_normal.dds",
        }]
        blocked = MODULE.blocked_normal_bump_runtime_bindings(
            current, graph, diagnostics
        )
        self.assertEqual(
            {"base", "mask", "emissive", "dissolve", "noise"},
            {row["slotId"] for row in blocked},
        )
        self.assertFalse(any(
            row["assetId"] == "Effect/fx_distortion_normal.dds"
            for row in blocked
        ))

        effect = {
            "effectAssetId": "effect.test",
            "elements": [{
                "id": "p0", "kind": "particle",
                "resources": current + [{
                    "slotId": "base", "assetId": "Effect/safe_color.dds",
                }],
                "material": {"sourceMaterialPath": "fx_mi.child"},
                "sourceRecipe": {"modules": []},
            }],
        }
        contract = {"materialIdentities": [{
            "sourceMaterialPath": "fx_mi.child",
            "profileId": "ue3.material.test",
            "runtimeShaderProfileId": "effect.ue3.fallback-blocked.v1",
            "parentMaterialPath": "fx_m.parent",
            "sourceParameters": {},
            "requiredRuntimeBindings": [],
            "roleResolvedRuntimeBindings": [],
            "blockedRuntimeAssetIds": [],
            "blockedRuntimeBindings": blocked,
        }]}
        upgraded = MODULE.upgrade_effect_document(effect, contract)
        resources = upgraded["elements"][0]["resources"]
        self.assertEqual(
            [
                {"slotId": "noise", "assetId": "Effect/fx_distortion_normal.dds"},
                {"slotId": "base", "assetId": "Effect/safe_color.dds"},
            ],
            resources,
        )

    def test_reflection_cubemap_is_not_promoted_to_generic_base(self):
        selected = {
            "textures": [{
                "name": "refle_tex",
                "texture": "fx_tex.fx_reflection_cube",
            }],
            "materialEvidence": {
                "collectedTextureParameters": [{
                    "name": "refle_tex",
                    "group": "reflection",
                }],
            },
        }
        graph = {"runtimeResourceBindings": [{
            "sourceObjectPath": "fx_tex.fx_reflection_cube",
            "assetId": "Effect/fx_reflection_cube.dds",
            "resolutionStatus": "RESOLVED_RUNTIME_ASSET",
        }]}
        bindings, diagnostics = MODULE.role_resolved_runtime_bindings(
            selected, graph
        )
        self.assertEqual([], bindings)
        self.assertEqual(
            "UNSUPPORTED_REFLECTION_RUNTIME_RESOURCE",
            diagnostics[0]["status"],
        )
        self.assertEqual(
            ["Effect/fx_reflection_cube.dds"],
            MODULE.blocked_runtime_asset_ids(
                selected, graph, set(), diagnostics
            ),
        )

    def test_grouped_occurrence_without_required_emission_carrier_blocks(self):
        effect = {
            "effectAssetId": "effect.test",
            "elements": [{
                "id": "p0", "kind": "particle",
                "resources": [{
                    "slotId": "mask", "assetId": "Effect/mask.dds",
                }],
                "material": {"sourceMaterialPath": "fx_mi.child"},
                "sourceRecipe": {"modules": []},
            }],
        }
        identity = {
            "sourceMaterialPath": "fx_mi.child",
            "profileId": "ue3.material.test",
            "runtimeShaderProfileId": "effect.ue3.grouped-translucent.v1",
            "parentMaterialPath": "fx_m.parent",
            "sourceParameters": {
                "scalars": [{
                    "name": "emission_strength",
                    "group": "emission",
                    "value": 2.0,
                }],
            },
            "requiredRuntimeBindings": [],
            "roleResolvedRuntimeBindings": [],
            "blockedRuntimeAssetIds": [],
            "blockedRuntimeBindings": [],
        }
        upgraded = MODULE.upgrade_effect_document(
            effect, {"materialIdentities": [identity]}
        )
        profile = upgraded["elements"][0]["material"]["sourceProfile"]
        self.assertEqual(
            "effect.ue3.fallback-blocked.v1",
            profile["runtimeShaderProfileId"],
        )

        effect["elements"][0]["resources"].append({
            "slotId": "emissive", "assetId": "Effect/emissive.dds",
        })
        upgraded = MODULE.upgrade_effect_document(
            effect, {"materialIdentities": [identity]}
        )
        self.assertEqual(
            "effect.ue3.grouped-translucent.v1",
            upgraded["elements"][0]["material"]["sourceProfile"]
            ["runtimeShaderProfileId"],
        )

    def test_partial_parent_role_keeps_non_conflicting_safe_carriers(self):
        effect = {
            "effectAssetId": "effect.test",
            "elements": [{
                "id": "p0", "kind": "particle",
                "resources": [
                    {"slotId": "base", "assetId": "Effect/base.dds"},
                    {"slotId": "mask", "assetId": "Effect/mask.dds"},
                    {"slotId": "noise", "assetId": "Effect/old_noise.dds"},
                ],
                "material": {"sourceMaterialPath": "fx_mi.child"},
                "sourceRecipe": {"modules": []},
            }],
        }
        contract = {"materialIdentities": [{
            "sourceMaterialPath": "fx_mi.child",
            "profileId": "ue3.material.test",
            "runtimeShaderProfileId": "effect.ue3.grouped-translucent.v1",
            "parentMaterialPath": "fx_m.parent",
            "sourceParameters": {},
            "requiredRuntimeBindings": [],
            "roleResolvedRuntimeBindings": [{
                "slotId": "noise", "assetId": "Effect/source_noise.dds",
            }],
            "blockedRuntimeAssetIds": [],
            "blockedRuntimeBindings": [],
        }]}
        upgraded = MODULE.upgrade_effect_document(effect, contract)
        self.assertEqual(
            [
                {"slotId": "base", "assetId": "Effect/base.dds"},
                {"slotId": "mask", "assetId": "Effect/mask.dds"},
                {"slotId": "noise", "assetId": "Effect/source_noise.dds"},
            ],
            upgraded["elements"][0]["resources"],
        )

    def test_parent_default_texture_resolves_from_unique_manifest_suffix(self):
        selected = {
            "materialEvidence": {
                "collectedTextureParameters": [{
                    "name": "base_tex", "group": "base",
                    "texture": "fx_tex_00.fx_a_glow_001",
                }]
            }
        }
        manifest = {"assets": [{
            "sourceAssetPath": "fx_tex_00.fx_a_glow_001",
            "logicalPackage": "fx_tex_00",
            "roles": ["texture"],
            "resolutionStatus": "RESOLVED_SOURCE_PACKAGE",
        }]}
        bindings, diagnostics = MODULE.role_resolved_runtime_bindings(
            selected, {"runtimeResourceBindings": []}, manifest
        )
        self.assertEqual(
            [("base", "Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_glow_001.dds")],
            [(row["slotId"], row["assetId"]) for row in bindings],
        )
        self.assertEqual("RESOURCE_MANIFEST_SUFFIX", diagnostics[0]["resolutionMethod"])

    def test_ambiguous_manifest_texture_stays_unresolved(self):
        asset_id, status = MODULE.manifest_texture_runtime_asset(
            {"assets": [
                {
                    "sourceAssetPath": "fx_tex_00.fx_shared",
                    "logicalPackage": "fx_tex_00", "roles": ["texture"],
                    "resolutionStatus": "RESOLVED_SOURCE_PACKAGE",
                },
                {
                    "sourceAssetPath": "fx_tex_01.fx_shared",
                    "logicalPackage": "fx_tex_01", "roles": ["texture"],
                    "resolutionStatus": "RESOLVED_SOURCE_PACKAGE",
                },
            ]},
            "fx_unknown.fx_shared",
        )
        self.assertIsNone(asset_id)
        self.assertEqual("AMBIGUOUS_MANIFEST_TEXTURE", status)

    def test_unresolved_full_card_material_is_emitter_fail_closed(self):
        effect = {
            "effectAssetId": "effect.test",
            "elements": [{
                "id": "p0",
                "kind": "particle",
                "resources": [{"slotId": "base", "assetId": "Effect/white.dds"}],
                "material": {
                    "templateId": "effect.standard",
                    "sourceMaterialPath": "fx_mi.fx.card",
                },
                "sourceRecipe": {"modules": []},
            }],
        }
        graph = {"materialParameterBindings": [{
            "sourceMaterialPath": "fx_mi.fx.card",
            "sourcePhysicalPackage": "fx.upk",
            "parent": "fx_m.card",
        }]}
        material_map = {"materials": {"fx.card": [{
            "source_file": "fx.upk",
            "material_path": "fx_mi.fx.card",
            "parent": "fx_m.card",
            "fallbackBlockedReason": "UNRESOLVED_FULL_OPAQUE_CARD",
        }]}}

        contract, result = MODULE.build_contract(
            effect, graph, {"elementConversions": []}, {"assets": []},
            material_map,
        )
        self.assertEqual([], result["failures"])
        identity = contract["materialIdentities"][0]
        self.assertEqual(
            "effect.ue3.fallback-blocked.v1",
            identity["runtimeShaderProfileId"],
        )
        upgraded = MODULE.upgrade_effect_document(effect, contract)
        self.assertEqual(
            "effect.ue3.fallback-blocked.v1",
            upgraded["elements"][0]["material"]["sourceProfile"]
            ["runtimeShaderProfileId"],
        )

    def test_parent_render_state_selects_one_sided_translucent_profile(self):
        effect = {
            "effectAssetId": "effect.test",
            "elements": [{
                "id": "p0",
                "kind": "particle",
                "resources": [],
                "material": {
                    "templateId": "effect.standard",
                    "sourceMaterialPath": "fx_mi.fx.one_sided",
                },
                "sourceRecipe": {"modules": []},
            }],
        }
        graph = {"materialParameterBindings": [{
            "sourceMaterialPath": "fx_mi.fx.one_sided",
            "sourcePhysicalPackage": "fx.upk",
            "parent": "fx_m.one_sided",
        }]}
        material_map = {"materials": {"fx.one_sided": [{
            "source_file": "fx.upk",
            "material_path": "fx_mi.fx.one_sided",
            "parent": "fx_m.one_sided",
            "materialEvidence": {"renderState": {
                "blendMode": "BLEND_Translucent",
                "twoSided": False,
            }},
        }]}}

        contract, result = MODULE.build_contract(
            effect, graph, {"elementConversions": []}, {"assets": []},
            material_map,
        )
        self.assertEqual([], result["failures"])
        upgraded = MODULE.upgrade_effect_document(effect, contract)
        self.assertEqual(
            "alpha_one_sided_depth_read",
            upgraded["elements"][0]["material"]["renderProfile"],
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
        self.assertEqual(12, upgraded["version"])
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
