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


if __name__ == "__main__":
    unittest.main()
