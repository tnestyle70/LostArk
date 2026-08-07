#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("extract_umodel_material_dependencies.py")
SPEC = importlib.util.spec_from_file_location(
    "extract_umodel_material_dependencies", MODULE_PATH
)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class ExtractUmodelMaterialDependenciesTests(unittest.TestCase):
    def test_props_parameters_are_parsed(self) -> None:
        document = MODULE.parse_props(
            """
Parent = Material3'fx_m.fx_parent'
ScalarParameterValues[1] =
{
    ScalarParameterValues[0] =
    {
        ParameterValue = 2.5
        ParameterName = intensity
    }
}
TextureParameterValues[1] =
{
    TextureParameterValues[0] =
    {
        ParameterValue = Texture2D'fx_mask'
        ParameterName = mask
    }
}
VectorParameterValues[1] =
{
    VectorParameterValues[0] =
    {
        ParameterValue = { R=1, G=0.5, B=0.25, A=1 }
        ParameterName = color
    }
}
"""
        )
        self.assertEqual("fx_m.fx_parent", document["parent"])
        self.assertEqual(2.5, document["scalar"][0]["value"])
        self.assertEqual("fx_mask", document["texture"][0]["value"])
        self.assertEqual(0.5, document["vector"][0]["value"]["g"])

    def test_static_switch_and_material_dump_are_explicit_evidence(self) -> None:
        document = MODULE.parse_props(
            """
StaticSwitchParameters[1] =
{
    StaticSwitchParameters[0] =
    {
        Value = true
        ParameterName = usedynamicparam
    }
}
"""
        )
        self.assertTrue(document["static_switch"][0]["value"])

        evidence = MODULE.parse_material_dump(
            """
BlendMode = BLEND_Additive
LightingModel = MLM_Unlit
TwoSided = true
bUsesDistortion = false
ReferencedTextures[0] = Texture2D'fx_tex_00.fx_a_noise_002'
Expressions[48] =
Expressions[0] = 0
Expressions[1] = MaterialExpressionScalarParameter'power'
"""
        )
        self.assertEqual("BLEND_Additive", evidence["renderState"]["blendMode"])
        self.assertTrue(evidence["renderState"]["twoSided"])
        self.assertEqual(
            ["fx_tex_00.fx_a_noise_002"], evidence["referencedTextures"]
        )
        self.assertEqual(2, evidence["expressionCoverage"]["entryCount"])
        self.assertEqual(1, evidence["expressionCoverage"]["nonNullCount"])

    def test_direct_material_dump_preserves_exact_defaults_and_provenance(self) -> None:
        import tempfile

        with tempfile.TemporaryDirectory() as temporary:
            dump_path = Path(temporary) / "direct.umodel-dump.txt"
            dump_text = """
ClassName: Material3 ObjectName: direct_glow
TwoSided = false
bDisableDepthTest = false
BlendMode = BLEND_Additive (3)
Expressions[48] =
Expressions[4] = MaterialExpressionScalarParameter'power'
CollectedScalarParameters[2] =
{
    CollectedScalarParameters[0] = { Value=2, Name=centerglow_str, Group=None }
    CollectedScalarParameters[1] = { Value=25, Name=centerglow_power, Group=None }
}
"""
            dump_path.write_text(dump_text, encoding="utf-8")
            candidate = MODULE.build_direct_material_dump_candidate(
                "bfx_m.bfx.direct_glow", "source.upk", dump_path,
                dump_text,
            )
            self.assertIsNotNone(candidate)
            assert candidate is not None
            self.assertEqual("RESOLVED_UMODEL_DUMP", candidate["resolutionStatus"])
            self.assertEqual("SOURCE_MATERIAL_DUMP", candidate["materialEvidenceStatus"])
            self.assertEqual(
                [
                    {"name": "centerglow_str", "value": 2.0},
                    {"name": "centerglow_power", "value": 25.0},
                ],
                candidate["scalars"],
            )
            self.assertEqual(
                MODULE.sha256_file(dump_path),
                candidate["materialEvidenceSha256"],
            )
            self.assertEqual(
                1,
                candidate["materialEvidence"]["expressionCoverage"][
                    "entryCount"
                ],
            )

            mismatch = MODULE.build_direct_material_dump_candidate(
                "bfx_m.bfx.other", "source.upk", dump_path, dump_text
            )
            self.assertIsNone(mismatch)

    def test_collected_texture_parameter_roles_are_explicit_evidence(self) -> None:
        evidence = MODULE.parse_material_dump(
            """
bDisableDepthTest = false
CollectedTextureParameters[3] =
{
    CollectedTextureParameters[0] =
    {
        Texture = Texture2D'fx_tex.fx_alpha'
        Name = alpha_texture2
        Group = alpha
    }
    CollectedTextureParameters[1] = { Texture=Texture2D'fx_tex.fx_emissive', Name=emissive_tex_01, Group=emissive }
    CollectedTextureParameters[2] = { Texture=Texture2D'fx_tex.fx_noise', Name=uv_noise_texture, Group=uv_noise }
}
"""
        )
        self.assertFalse(evidence["renderState"]["disableDepthTest"])
        self.assertEqual(
            [
                {
                    "texture": "fx_tex.fx_alpha",
                    "name": "alpha_texture2",
                    "group": "alpha",
                },
                {
                    "texture": "fx_tex.fx_emissive",
                    "name": "emissive_tex_01",
                    "group": "emissive",
                },
                {
                    "texture": "fx_tex.fx_noise",
                    "name": "uv_noise_texture",
                    "group": "uv_noise",
                },
            ],
            evidence["collectedTextureParameters"],
        )

    def test_parent_parameter_defaults_and_groups_are_explicit_evidence(self) -> None:
        evidence = MODULE.parse_material_dump(
            """
CollectedScalarParameters[2] =
{
    CollectedScalarParameters[0] = { Value=1.25, Name=uv_panning_x, Group=panning }
    CollectedScalarParameters[1] =
    {
        Value = 0.5
        Name = emissive_power
        Group = emissive
    }
}
CollectedVectorParameters[1] =
{
    CollectedVectorParameters[0] =
    {
        Value = { R=1, G=0.5, B=0.25, A=1 }
        Name = tint
        Group = color
    }
}
CollectedStaticSwitchParameters[1] = { Value=true, Name=use_distortion, Group=distortion }
"""
        )
        self.assertEqual(
            [
                {"name": "uv_panning_x", "group": "panning", "value": 1.25},
                {"name": "emissive_power", "group": "emissive", "value": 0.5},
            ],
            evidence["collectedScalarParameters"],
        )
        self.assertEqual(
            [{
                "name": "tint", "group": "color",
                "value": {"r": 1.0, "g": 0.5, "b": 0.25, "a": 1.0},
            }],
            evidence["collectedVectorParameters"],
        )
        self.assertEqual(
            [{
                "name": "use_distortion", "group": "distortion",
                "value": True,
            }],
            evidence["collectedStaticSwitchParameters"],
        )

    def test_parent_package_resolves_without_exported_parent_mat(self) -> None:
        import tempfile

        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            material = root / "MaterialInstanceConstant"
            material.mkdir()
            (material / "child.props.txt").write_text(
                "Parent = Material3'fx_parent.fx_material'\n",
                encoding="utf-8",
            )
            candidate = MODULE.build_candidate(
                "fx_child.fx_mi.child",
                "child.upk",
                root,
                inventory={"fx_parent": "parent.upk"},
            )
            self.assertEqual("parent.upk", candidate["parent_source_file"])

    def test_material_evidence_comes_from_parent_props_not_console_log(self) -> None:
        import tempfile

        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            instance = root / "MaterialInstanceConstant"
            parent = root / "Material3"
            instance.mkdir()
            parent.mkdir()
            (instance / "child.props.txt").write_text(
                "Parent = Material3'fx_parent.fx_material'\n",
                encoding="utf-8",
            )
            (parent / "fx_material.props.txt").write_text(
                """
BlendMode = BLEND_Translucent
LightingModel = MLM_Unlit
TwoSided = false
ReferencedTextures[0] = Texture2D'fx_tex.fx_alpha'
""",
                encoding="utf-8",
            )
            candidate = MODULE.build_candidate(
                "fx_child.fx_mi.child",
                "child.upk",
                root,
                umodel_log="BlendMode = BLEND_Opaque\nTwoSided = true\n",
                inventory={"fx_parent": "parent.upk"},
            )

            evidence = candidate["materialEvidence"]
            self.assertEqual(
                "BLEND_Translucent", evidence["renderState"]["blendMode"]
            )
            self.assertFalse(evidence["renderState"]["twoSided"])
            self.assertEqual(
                ["fx_tex.fx_alpha"], evidence["referencedTextures"]
            )
            self.assertEqual(
                "SOURCE_MATERIAL_PROPS", candidate["materialEvidenceStatus"]
            )
            self.assertEqual(
                "Material3/fx_material.props.txt",
                candidate["materialEvidencePropsFile"],
            )
            self.assertEqual(
                MODULE.sha256_file(parent / "fx_material.props.txt"),
                candidate["materialEvidencePropsSha256"],
            )
            self.assertEqual(
                MODULE.sha256_file(instance / "child.props.txt"),
                candidate["propsFileSha256"],
            )

    def test_material_evidence_follows_instance_parent_chain(self) -> None:
        import tempfile

        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            child = root / "fx_child" / "MaterialInstanceConstant"
            middle = root / "fx_middle" / "MaterialInstanceConstant"
            parent = root / "fx_parent" / "Material3"
            child.mkdir(parents=True)
            middle.mkdir(parents=True)
            parent.mkdir(parents=True)
            child_props = child / "child.props.txt"
            child_props.write_text(
                "Parent = MaterialInstanceConstant'fx_middle.mi.middle'\n",
                encoding="utf-8",
            )
            (middle / "middle.props.txt").write_text(
                "Parent = Material3'fx_parent.fx_material'\n",
                encoding="utf-8",
            )
            parent_props = parent / "fx_material.props.txt"
            parent_props.write_text(
                "BlendMode = BLEND_Translucent\n", encoding="utf-8"
            )

            selected, count = MODULE.select_material_evidence_props(
                root, child_props, "fx_middle.mi.middle"
            )
            self.assertEqual(parent_props, selected)
            self.assertEqual(1, count)

    def test_material_evidence_parent_cycle_fails_closed(self) -> None:
        import tempfile

        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            child = root / "fx_child" / "MaterialInstanceConstant"
            middle = root / "fx_middle" / "MaterialInstanceConstant"
            child.mkdir(parents=True)
            middle.mkdir(parents=True)
            child_props = child / "child.props.txt"
            child_props.write_text(
                "Parent = MaterialInstanceConstant'fx_middle.mi.middle'\n",
                encoding="utf-8",
            )
            (middle / "middle.props.txt").write_text(
                "Parent = MaterialInstanceConstant'fx_child.mi.child'\n",
                encoding="utf-8",
            )

            selected, count = MODULE.select_material_evidence_props(
                root, child_props, "fx_middle.mi.middle"
            )
            self.assertIsNone(selected)
            self.assertEqual(0, count)


if __name__ == "__main__":
    unittest.main()
