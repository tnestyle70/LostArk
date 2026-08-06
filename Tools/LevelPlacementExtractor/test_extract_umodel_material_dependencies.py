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


if __name__ == "__main__":
    unittest.main()
