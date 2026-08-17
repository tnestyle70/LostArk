#!/usr/bin/env python3
"""Focused contract tests for exact UE3 material-family projection."""

from __future__ import annotations

import importlib.util
import json
import tempfile
import unittest
from collections import Counter
from pathlib import Path


SCRIPT_PATH = Path(__file__).resolve().with_name(
    "project_ue3_material_families.py")
SPEC = importlib.util.spec_from_file_location(
    "project_ue3_material_families", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
projector = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(projector)


class AdmissionTests(unittest.TestCase):
    def setUp(self) -> None:
        self.family = {
            "familyId": "fixture",
            "runtimeShaderProfileId": "effect.ue3.fixture.v1",
            "profileId": "ue3.material.fixture",
            "parentMaterialPath": "fixture.parent",
            "carrier": {"elementKind": "particle", "rendererShape": "sprite"},
            "renderProfile": "alpha_one_sided_depth_read",
            "textureRoles": [
                {"lane": 0, "name": "base_tex", "required": True},
                {"lane": 1, "name": "optional_tex", "required": False},
            ],
        }
        self.identity = {
            "sourceMaterialPath": "fixture.child",
            "profileId": "ue3.material.fixture",
            "parentMaterialPath": "fixture.parent",
            "sourceParameters": {"textures": [
                {
                    "name": "base_tex", "group": "base",
                    "sourceObjectPath": "fx_tex.fixture",
                    "assetId": "Effect/Fixture/fixture.dds",
                    "addressU": "wrap", "addressV": "wrap",
                    "colorSpace": "srgb", "samplingEvidence": "fixture",
                },
                {
                    "name": "optional_tex", "group": "",
                    "sourceObjectPath": "", "assetId": "",
                },
            ]},
        }
        self.element = {
            "id": "fixture.element",
            "kind": "particle",
            "sourceRecipe": {"rendererShape": "sprite"},
            "material": {
                "sourceMaterialPath": "fixture.child",
                "renderProfile": "alpha_one_sided_depth_read",
                "sourceProfile": {
                    "enabled": True,
                    "profileId": "ue3.material.fixture",
                    "runtimeShaderProfileId": "effect.ue3.grouped-translucent.v1",
                    "parentMaterialPath": "fixture.parent",
                    "textures": [],
                    "scalars": [{"name": "keep", "value": 7}],
                },
            },
        }

    def test_optional_unresolved_lane_is_not_synthesized(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            texture = root / "Client/Bin/Resources/Effect/Fixture/fixture.dds"
            texture.parent.mkdir(parents=True)
            texture.write_bytes(b"fixture")
            projected, blockers = projector.admit_occurrence(
                self.element, self.family,
                {("fixture.child", "ue3.material.fixture"): self.identity},
                {}, root)
        self.assertEqual(blockers, [])
        self.assertIsNotNone(projected)
        self.assertEqual(
            [row["name"] for row in projected["textures"]], ["base_tex"])
        self.assertEqual(projected["scalars"], [{"name": "keep", "value": 7}])

    def test_parent_match_does_not_bypass_exact_render_state(self) -> None:
        element = json.loads(json.dumps(self.element))
        element["material"]["renderProfile"] = "additive_one_sided_depth_read"
        projected, blockers = projector.admit_occurrence(
            element, self.family,
            {("fixture.child", "ue3.material.fixture"): self.identity},
            {}, Path("."))
        self.assertIsNone(projected)
        self.assertIn("RENDER_PROFILE_MISMATCH", blockers)

    def test_parent_match_does_not_bypass_sprite_carrier(self) -> None:
        element = json.loads(json.dumps(self.element))
        element["sourceRecipe"]["rendererShape"] = "mesh"
        projected, blockers = projector.admit_occurrence(
            element, self.family,
            {("fixture.child", "ue3.material.fixture"): self.identity},
            {}, Path("."))
        self.assertIsNone(projected)
        self.assertIn("RENDERER_SHAPE_MISMATCH", blockers)

    def test_existing_typed_runtime_profile_is_not_overwritten(self) -> None:
        element = json.loads(json.dumps(self.element))
        element["material"]["sourceProfile"]["runtimeShaderProfileId"] = (
            "effect.ue3.manual-specialized.v1")
        projected, blockers = projector.admit_occurrence(
            element, self.family,
            {("fixture.child", "ue3.material.fixture"): self.identity},
            {}, Path("."))
        self.assertIsNone(projected)
        self.assertIn("CURRENT_RUNTIME_PROFILE_MISMATCH", blockers)


class ProductProjectionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.plan = projector.build_plan(projector.REPOSITORY_ROOT)

    def test_registry_ids_indices_and_role_order_are_locked(self) -> None:
        projector.validate_registry(self.plan["registry"])
        actual = {
            row["runtimeShaderProfileId"]: (
                row["nativeProfileIndex"], row["renderProfile"],
                [role["name"] for role in row["textureRoles"]],
                [role["required"] for role in row["textureRoles"]])
            for row in self.plan["registry"]["families"]
        }
        self.assertEqual(actual["effect.ue3.slice.v1"][:2],
                         (12, "alpha_one_sided_depth_read"))
        self.assertEqual(actual["effect.ue3.glasshole-02.v1"], (
            29, "alpha_two_sided_depth_read",
            ["aura_texture", "cracknormal_tex", "in_hole_texture"],
            [True, True, True]))
        self.assertEqual(actual["effect.ue3.fluidninja-01.v1"], (
            30, "alpha_two_sided_depth_read",
            ["diff_tex", "flow_1_tex", "flow_2_tex", "mask_tex",
             "opacity_tex"], [True, True, True, True, True]))
        self.assertEqual(actual["effect.ue3.customparticle-01.v1"], (
            31, "additive_one_sided_depth_read",
            ["diff_tex", "a_noise_01_tex"], [True, False]))
        self.assertEqual(actual["effect.ue3.crackholev2-01.v1"], (
            32, "alpha_one_sided_depth_read",
            ["01.map_e", "06.map_f", "06.map", "mask_noisemap",
             "mask_tex_l", "mask_tex_r"],
            [True, True, True, True, True, True]))

    def test_product_denominator_and_family_census_are_exact(self) -> None:
        self.assertEqual(self.plan["documentCount"], 101)
        self.assertEqual(self.plan["elementCount"], 2945)
        self.assertEqual(self.plan["rejections"], [])
        self.assertEqual(self.plan["pendingDocumentCount"], 0)
        self.assertEqual(self.plan["pendingOccurrenceCount"], 0)
        self.assertEqual(Counter(
            row["familyId"] for row in self.plan["occurrences"]), {
                "slice-01": 7,
                "glasshole-02": 8,
                "fluidninja-01": 1,
                "customparticle-01": 1,
                "crackholev2-01": 1,
            })

    def test_dimensionmaster_w_keeps_21_elements_and_admits_six(self) -> None:
        path = (projector.REPOSITORY_ROOT / "Data/Effects/Authored/"
                "effect.dimensionmaster.skill.2050120.clip3.unified.effect.json")
        document = projector.read_json(path)
        self.assertEqual(len(document["elements"]), 21)
        targets = {
            "authored.source-particle.445823f15363035cab17dc91": (
                "effect.ue3.slice.v1", ["slice_flow_texture"]),
            "authored.source-particle.d4f1be245a9ac057550a4229": (
                "effect.ue3.crackholev2-01.v1",
                ["01.map_e", "06.map_f", "06.map", "mask_noisemap",
                 "mask_tex_l", "mask_tex_r"]),
            "authored.source-particle.40e1b48e2f0f88dcfeff1549": (
                "effect.ue3.glasshole-02.v1",
                ["aura_texture", "cracknormal_tex", "in_hole_texture"]),
            "authored.source-particle.5fb1a1b067b955f128fd25e0": (
                "effect.ue3.fluidninja-01.v1",
                ["diff_tex", "flow_1_tex", "flow_2_tex", "mask_tex",
                 "opacity_tex"]),
            "authored.source-particle.dc486c58e3b368e683fa8f03": (
                "effect.ue3.customparticle-01.v1", ["diff_tex"]),
            "authored.source-particle.0a9e81858cb8604c16deeb56": (
                "effect.ue3.glasshole-02.v1",
                ["aura_texture", "cracknormal_tex", "in_hole_texture"]),
        }
        by_id = {row["id"]: row for row in document["elements"]}
        for element_id, (runtime_profile, roles) in targets.items():
            profile = by_id[element_id]["material"]["sourceProfile"]
            self.assertEqual(profile["runtimeShaderProfileId"], runtime_profile)
            self.assertEqual([row["name"] for row in profile["textures"]], roles)
        scale = by_id[
            "authored.source-particle.445823f15363035cab17dc91"
        ]["detail"]["particle"]["sourceScale"]
        self.assertEqual(scale, {
            "count": 1.42999995, "size": 0.25, "lifeTime": 1.64999998})


if __name__ == "__main__":
    unittest.main(verbosity=2)
