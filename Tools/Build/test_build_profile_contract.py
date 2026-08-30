#!/usr/bin/env python3
"""Fast structural gate for the product-first build surface.

The source tree may keep focused diagnostic projects, but a normal solution
build and the Product profile must never rebuild them.  Effect authoring JSON
is discovered by domain tools and must not be expanded into thousands of
MSBuild project items.
"""

from __future__ import annotations

from pathlib import Path
import re
import unittest
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parents[2]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8-sig")


class BuildProfileContractTests(unittest.TestCase):
    def test_solution_default_build_contains_only_four_product_projects(self) -> None:
        solution = read("Framework.sln")
        product_guids = {
            "1A5672C6-54B4-4779-99E7-84EF85171388",  # Client
            "922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7",  # Engine
            "F4CCF815-6D51-412F-A76E-84D2F1D05571",  # Shared
            "053788E7-C377-4811-8AFE-BC23D9BE4AE7",  # Server
        }
        build_rows = re.findall(
            r"\{([0-9A-F-]{36})\}\.[^\r\n]+\.Build\.0\s*=", solution
        )
        self.assertEqual(16, len(build_rows))
        self.assertEqual(product_guids, set(build_rows))
        self.assertNotIn('= "EffectRenderContractHarness"', solution)
        self.assertNotIn('= "ValtanFourPlayerHarness"', solution)
        self.assertNotIn('= "MapFrustumContractHarness"', solution)

    def test_product_runner_is_explicit_and_has_no_retired_harness(self) -> None:
        runner = read("Tools/Build/Invoke-BuildAndRegression.ps1")
        self.assertRegex(runner, r"\[string\]\$Profile\s*=\s*'Core'")
        self.assertIn("[ValidateSet('Product', 'Core', 'FullDiagnostic')]", runner)
        for retired in (
            "EffectRenderContractHarness",
            "ValtanFourPlayerHarness",
            "Run-ValtanFourPlayerHarness",
            "MapFrustumContractHarness",
            "Test-BernFrustumCullingContract",
        ):
            self.assertNotIn(retired, runner)
        self.assertIn("Tools\\MapPipeline\\Test-MapWaterRenderContract.ps1", runner)

        product_projects = (
            "'Engine\\Default\\Engine.vcxproj'",
            "'Shared\\Default\\Shared.vcxproj'",
            "'Server\\Default\\Server.vcxproj'",
            "'Client\\Default\\Client.vcxproj'",
        )
        positions = [runner.index(project) for project in product_projects]
        self.assertEqual(sorted(positions), positions)
        self.assertIn("/p:BuildProjectReferences=false", runner)

    def test_effect_json_is_not_materialized_as_msbuild_items(self) -> None:
        project_path = ROOT / "Client/Default/Client.vcxproj"
        filters_path = ROOT / "Client/Default/Client.vcxproj.filters"
        ET.parse(project_path)
        ET.parse(filters_path)
        project = read("Client/Default/Client.vcxproj")
        filters = read("Client/Default/Client.vcxproj.filters")
        for source in (project, filters):
            self.assertNotIn("GeneratedEffectData", source)
            self.assertNotRegex(
                source,
                r'<None Include="\.\.\\\.\.\\Data\\Effects\\',
            )
        for retired_script in (
            "Tools/EffectPipeline/Sync-EffectDataProject.ps1",
            "Tools/EffectPipeline/Test-EffectDataProjectRegistration.ps1",
        ):
            self.assertFalse((ROOT / retired_script).exists())

    def test_valtan_four_player_duplicate_is_physically_retired(self) -> None:
        for relative in (
            "Tools/ValtanFourPlayerHarness",
            "Tools/Network/Run-ValtanFourPlayerHarness.ps1",
        ):
            self.assertFalse((ROOT / relative).exists())

    def test_map_frustum_harness_is_physically_retired(self) -> None:
        for relative in (
            "Tools/MapFrustumContractHarness/Default/MapFrustumContractHarness.vcxproj",
            "Tools/MapFrustumContractHarness/Default/MapFrustumContractHarness.vcxproj.filters",
            "Tools/MapFrustumContractHarness/Private/MapFrustumContractHarness.cpp",
            "Tools/MapFrustumContractHarness/Run-MapFrustumContractHarness.ps1",
            "Tools/ProjectAudit/Test-BernFrustumCullingContract.ps1",
            "Tools/ProjectAudit/Test-MapWaterRenderContract.ps1",
        ):
            self.assertFalse((ROOT / relative).exists(), relative)
        self.assertTrue(
            (ROOT / "Tools/MapPipeline/Test-MapWaterRenderContract.ps1").is_file()
        )

    def test_product_projects_enable_parallel_x64_compilation(self) -> None:
        for relative in (
            "Engine/Default/Engine.vcxproj",
            "Shared/Default/Shared.vcxproj",
            "Server/Default/Server.vcxproj",
            "Client/Default/Client.vcxproj",
        ):
            ET.parse(ROOT / relative)
            project = read(relative)
            self.assertGreaterEqual(
                project.count(
                    "<MultiProcessorCompilation>true</MultiProcessorCompilation>"
                ),
                2,
                relative,
            )

    def test_effect_render_is_a_focused_warp_probe_not_a_product_recompile(self) -> None:
        project_path = (
            ROOT
            / "Tools/EffectRenderContractHarness/Default/EffectRenderContractHarness.vcxproj"
        )
        project = ET.parse(project_path).getroot()
        namespace = {"m": "http://schemas.microsoft.com/developer/msbuild/2003"}
        compile_paths = [
            node.attrib["Include"]
            for node in project.findall(".//m:ClCompile", namespace)
            if "Include" in node.attrib
        ]
        self.assertEqual(
            [r"..\Private\EffectRenderContractHarness.cpp"], compile_paths
        )
        self.assertEqual([], project.findall(".//m:ProjectReference", namespace))

        source = read(
            "Tools/EffectRenderContractHarness/Private/EffectRenderContractHarness.cpp"
        )
        self.assertLess(len(source.splitlines()), 1200)
        for token in (
            "D3D_DRIVER_TYPE_WARP",
            "Shader_VtxEffectMeshPreview.cso",
            "Shader_EffectMeshV2.cso",
            "D3D11_MAP_READ",
        ):
            self.assertIn(token, source)

        solution = read("Framework.sln")
        runner = read("Tools/Build/Invoke-BuildAndRegression.ps1")
        self.assertNotIn("EffectRenderContractHarness", solution)
        self.assertNotIn("EffectRenderContractHarness", runner)

    def test_action_presentation_harness_compile_items_have_filters(self) -> None:
        project_root = ET.parse(
            ROOT
            / "Tools/ActionPresentationTimelineHarness/Default/ActionPresentationTimelineHarness.vcxproj"
        ).getroot()
        filters_root = ET.parse(
            ROOT
            / "Tools/ActionPresentationTimelineHarness/Default/ActionPresentationTimelineHarness.vcxproj.filters"
        ).getroot()
        namespace = {"m": "http://schemas.microsoft.com/developer/msbuild/2003"}
        project_sources = {
            node.attrib["Include"]
            for node in project_root.findall(".//m:ClCompile", namespace)
            if "Include" in node.attrib
        }
        filtered_sources = {
            node.attrib["Include"]
            for node in filters_root.findall(".//m:ClCompile", namespace)
            if "Include" in node.attrib
        }
        self.assertEqual(project_sources, filtered_sources)


if __name__ == "__main__":
    unittest.main()
