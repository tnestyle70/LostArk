from __future__ import annotations

import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROBE = ROOT / "Tools/RenderingPipeline/ProductEffectShaderWarpProbe.cpp"
CLOSURE = ROOT / "Tools/Build/Test-CompiledShaderClosure.ps1"
RETIRED_SOURCES = (
    "Tools/EffectRenderContractHarness/Default/EffectRenderContractHarness.vcxproj",
    "Tools/EffectRenderContractHarness/Default/EffectRenderContractHarness.vcxproj.filters",
    "Tools/EffectRenderContractHarness/Private/EffectRenderContractHarness.cpp",
    "Tools/EffectRenderContractHarness/Run-EffectRenderContractHarness.ps1",
    "Tools/EffectRenderContractHarness/Test-EffectRenderResourceRoot.ps1",
    "Tools/EffectRenderContractHarness/test_effect_render_harness_contract.py",
)


class ProductEffectShaderWarpContractTests(unittest.TestCase):
    def test_probe_executes_both_product_effect_shaders_on_warp(self) -> None:
        source = PROBE.read_text(encoding="utf-8")
        for token in (
            "D3D_DRIVER_TYPE_WARP",
            "D3DX11CreateEffectFromMemory",
            "Shader_VtxEffectMeshPreview.cso",
            "Shader_EffectMeshV2.cso",
            '"OpaqueBackDepthWrite"',
            '"AlphaNoDepth"',
            "CopyResource",
            "D3D11_MAP_READ",
            "compiled effect produced zero visible WARP pixels",
            "assetPathBoundaryValidated",
        ):
            self.assertIn(token, source)

    def test_compiled_shader_closure_builds_runs_and_cleans_the_probe(self) -> None:
        closure = CLOSURE.read_text(encoding="utf-8-sig")
        for token in (
            "Invoke-ProductEffectShaderWarpProbe",
            "ProductEffectShaderWarpProbe.cpp",
            "Effects11d.lib",
            "Effects11.lib",
            "v1LitPixels",
            "v2LitPixels",
            "assetPathBoundaryValidated",
            "Remove-Item -LiteralPath $resolvedProbeRoot -Recurse -Force",
            "invalid primary cannot fall back",
            "invalid shared cannot fall back",
            "Product Effect resource-root cases : 8",
        ):
            self.assertIn(token, closure)
        self.assertNotIn("EffectRenderContractHarness", closure)

    def test_broad_effect_render_project_is_physically_retired(self) -> None:
        for relative in RETIRED_SOURCES:
            self.assertFalse((ROOT / relative).exists(), relative)


if __name__ == "__main__":
    unittest.main()
