from __future__ import annotations

import unittest
import xml.etree.ElementTree as ET
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROJECT = (
    ROOT
    / "Tools"
    / "EffectRenderContractHarness"
    / "Default"
    / "EffectRenderContractHarness.vcxproj"
)
SOURCE = (
    ROOT
    / "Tools"
    / "EffectRenderContractHarness"
    / "Private"
    / "EffectRenderContractHarness.cpp"
)
RUNNER = (
    ROOT
    / "Tools"
    / "EffectRenderContractHarness"
    / "Run-EffectRenderContractHarness.ps1"
)
MSBUILD = {"m": "http://schemas.microsoft.com/developer/msbuild/2003"}


class EffectRenderHarnessContractTests(unittest.TestCase):
    def test_project_compiles_only_the_focused_probe(self) -> None:
        project = ET.parse(PROJECT).getroot()
        compile_paths = [
            node.attrib["Include"]
            for node in project.findall(".//m:ClCompile", MSBUILD)
            if "Include" in node.attrib
        ]
        self.assertEqual(
            [r"..\Private\EffectRenderContractHarness.cpp"], compile_paths
        )
        self.assertEqual([], project.findall(".//m:ProjectReference", MSBUILD))
        text = PROJECT.read_text(encoding="utf-8")
        self.assertNotIn(r"Client\Private", text)
        self.assertNotIn("Engine.lib", text)
        self.assertIn("Effects11d.lib", text)
        self.assertIn("Effects11.lib", text)

    def test_probe_draws_both_product_effect_generations_on_warp(self) -> None:
        source = SOURCE.read_text(encoding="utf-8")
        for token in (
            "D3D_DRIVER_TYPE_WARP",
            "D3DX11CreateEffectFromMemory",
            "Shader_VtxEffectMeshPreview.cso",
            "Shader_EffectMeshV2.cso",
            "v1LitPixels",
            "v2LitPixels",
            "CopyResource",
            "D3D11_MAP_READ",
        ):
            self.assertIn(token, source)

    def test_runner_joins_current_v1_and_v2_validators_before_gpu_probe(self) -> None:
        runner = RUNNER.read_text(encoding="utf-8")
        self.assertIn("Validate-EffectSources.ps1", runner)
        self.assertIn("validate_effect_v2.py", runner)
        self.assertIn("-ResourceRoot", runner)
        self.assertIn("AllowLocalEffectResources", runner)
        self.assertIn("-AllowLocalResources", runner)


if __name__ == "__main__":
    unittest.main()
