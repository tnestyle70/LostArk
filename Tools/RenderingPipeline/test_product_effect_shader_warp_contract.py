from __future__ import annotations

import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROBE = ROOT / "Tools/RenderingPipeline/ProductEffectShaderWarpProbe.cpp"
CLOSURE = ROOT / "Tools/Build/Test-CompiledShaderClosure.ps1"
MATERIAL_FAMILIES = (
    ROOT / "Client/Bin/ShaderFiles/Shader_EffectUe3MaterialFamilies.hlsli"
)
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
            "Create_Deterministic_Glass_Fixture",
            '"g_RuntimeMaterialV2Opcode", 1004',
            '"g_RuntimeMaterialV2ParticleColorConsumedMask"',
            "GLASS_V1_VALID",
            "GLASS_V1_INVALID_MASK",
            "GLASS_V1_INVALID_SCALAR_NAN",
            "GLASS_V1_INVALID_ZERO_NORMAL",
            "GLASS_V1_INVALID_CAMERA_NAN",
            "glass opcode 1004 invalid fixture did not fail closed",
            "glassRt0Pixels",
            "glassRt1Pixels",
            "invalidNanScalarGlassRt0Pixels",
            "invalidZeroNormalGlassRt0Pixels",
            "invalidNanCameraGlassRt0Pixels",
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

    def test_glass_finite_guard_survives_release_fxc_optimization(self) -> None:
        source = MATERIAL_FAMILIES.read_text(encoding="utf-8-sig")
        for token in (
            "EffectProjectTunedGlassFinite1",
            "EffectProjectTunedGlassFinite2",
            "EffectProjectTunedGlassFinite3",
            "EffectProjectTunedGlassFinite4",
            "asuint(value) & 0x7f800000u",
            "EffectProjectTunedGlassFinite3(cameraPosition)",
        ):
            self.assertIn(token, source)

    def test_broad_effect_render_project_is_physically_retired(self) -> None:
        for relative in RETIRED_SOURCES:
            self.assertFalse((ROOT / relative).exists(), relative)


if __name__ == "__main__":
    unittest.main()
