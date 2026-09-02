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
import subprocess
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
        self.assertNotIn('= "ActionPresentationTimelineHarness"', solution)

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
            "ActionPresentationTimelineHarness",
            "Run-ActionPresentationTimelineHarness",
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

    def test_core_runs_the_actual_valtan_canonical_loader_harness(self) -> None:
        runner = read("Tools/Build/Invoke-BuildAndRegression.ps1")
        core_build = runner.index("if ($includeCore) {")
        server_build = runner.index(
            "Invoke-MSBuildProject $msbuild 'Server\\Default\\Server.vcxproj'",
            core_build,
        )
        core_block = runner[core_build:server_build]
        self.assertIn(
            "Tools\\ValtanPatternAuditionServiceHarness\\Default\\ValtanPatternAuditionServiceHarness.vcxproj",
            core_block,
        )
        run = runner.index("& $valtanAuditionServiceHarnessExe")
        full_diagnostic_server = runner.index(
            "if ($includeFullDiagnostic) {", run
        )
        self.assertLess(run, full_diagnostic_server)
        self.assertIn("ValtanPatternAuditionServiceHarness failed.", runner[run:])

    def test_product_runner_rejects_locked_standard_exes_before_publish_or_build(self) -> None:
        runner = read("Tools/Build/Invoke-BuildAndRegression.ps1")
        guard = read("Tools/Build/ProductOutputGuard.psm1")
        self.assertIn("function Assert-ProductOutputsUnlocked", runner)
        self.assertIn("Assert-StandardProductOutputsNotRunning", runner)
        self.assertIn("product:output-lock-preflight", runner)
        self.assertIn("old EXE cannot", guard)
        preflight = runner.index("Assert-ProductOutputsUnlocked\n", runner.index("try {"))
        domain_publish = runner.index("Invoke-SelectedBuildDomains", preflight)
        first_msbuild = runner.index("Invoke-MSBuildProject $msbuild", domain_publish)
        self.assertLess(preflight, domain_publish)
        self.assertLess(domain_publish, first_msbuild)
        for token in (
            "'Debug', 'Release'",
            "Client",
            "Server",
            "[StringComparer]::OrdinalIgnoreCase",
        ):
            self.assertIn(token, guard)

    def test_core_runs_effect_v2_schema_transaction_and_runtime_gates(self) -> None:
        runner = read("Tools/Build/Invoke-BuildAndRegression.ps1")
        self.assertIn(
            "Effect Tool V2 binding schema, migration, and read-set gate", runner
        )
        self.assertIn(
            "Tools/EffectToolV2/test_effect_v2_binding_pipeline.py", runner
        )
        self.assertIn(
            "Effect Tool V2 catalog schema contract gate", runner
        )
        self.assertIn(
            "Tools/EffectToolV2/test_effect_v2_catalog_contract.py", runner
        )
        self.assertIn(
            "Effect Tool V2 occurrence runtime contract gate", runner
        )
        self.assertIn(
            "Tools/EffectToolV2/test_effect_v2_occurrence_runtime_contract.py",
            runner,
        )

    def test_core_runs_valtan_status_and_world_destruction_contract_gates(self) -> None:
        runner = read("Tools/Build/Invoke-BuildAndRegression.ps1")
        canary = runner.index("DimensionMaster glass/water Tool audition canary gates")
        core_start = runner.index("if ($includeCore) {", canary)
        core_end = runner.index("\n    }\n\n    Invoke-SelectedBuildDomains", core_start)
        core_block = runner[core_start:core_end]

        self.assertIn(
            "$includeCore = $Profile -in @('Core', 'FullDiagnostic')", runner
        )
        self.assertIn("Valtan status and response data-contract gate", core_block)
        self.assertIn(
            "Tools.ValtanPipeline.test_valtan_status_pattern_contract", core_block
        )
        self.assertIn(
            "& '.\\Tools\\WorldPipeline\\Publish-ValtanWorldDestruction.ps1'",
            core_block,
        )
        self.assertIn("-Mode ContractTest", core_block)
        self.assertIn("Valtan world destruction contract tests failed.", core_block)
        self.assertEqual(
            1, runner.count("Publish-ValtanWorldDestruction.ps1")
        )

    def test_full_diagnostic_runs_valtan_cross_product_cue_gate(self) -> None:
        runner = read("Tools/Build/Invoke-BuildAndRegression.ps1")
        gate = "Valtan CROSS Product cue and fixed-step rock wave gate"
        test_path = "Tools/EffectPipeline/test_valtan_cross_rock_wave_effect.py"
        gate_position = runner.index(gate)
        full_diagnostic_position = runner.rfind(
            "if ($includeFullDiagnostic) {", 0, gate_position
        )
        block_end = runner.index("\n\t}", gate_position)
        self.assertGreater(full_diagnostic_position, -1)
        self.assertLess(gate_position, block_end)
        self.assertIn(test_path, runner[gate_position:block_end])

    def test_full_diagnostic_runs_valtan_combat_object_hit_effect_gate(self) -> None:
        runner = read("Tools/Build/Invoke-BuildAndRegression.ps1")
        gate = "Valtan combat-object hit Effect presentation gate"
        test_path = (
            "Tools/ValtanPipeline/"
            "test_valtan_combat_object_hit_effect_presentation_contract.py"
        )
        gate_position = runner.index(gate)
        full_diagnostic_position = runner.rfind(
            "if ($includeFullDiagnostic) {", 0, gate_position
        )
        block_end = runner.index("\n\t}", gate_position)
        self.assertGreater(full_diagnostic_position, -1)
        self.assertLess(gate_position, block_end)
        self.assertIn(test_path, runner[gate_position:block_end])

    def test_product_output_guard_uses_real_process_fixture(self) -> None:
        result = subprocess.run(
            [
                "powershell.exe",
                "-NoProfile",
                "-ExecutionPolicy",
                "Bypass",
                "-File",
                str(ROOT / "Tools/Build/Test-ProductOutputGuard.ps1"),
            ],
            cwd=ROOT,
            text=True,
            capture_output=True,
            timeout=45,
            check=False,
        )
        self.assertEqual(
            0,
            result.returncode,
            f"stdout:\n{result.stdout}\nstderr:\n{result.stderr}",
        )
        self.assertRegex(
            result.stdout,
            r"ProductOutputGuard fixture: \d+/\d+ passed",
        )

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

    def test_effect_render_project_is_retired_and_warp_is_product_closure(self) -> None:
        for relative in (
            "Tools/EffectRenderContractHarness/Default/EffectRenderContractHarness.vcxproj",
            "Tools/EffectRenderContractHarness/Default/EffectRenderContractHarness.vcxproj.filters",
            "Tools/EffectRenderContractHarness/Private/EffectRenderContractHarness.cpp",
            "Tools/EffectRenderContractHarness/Run-EffectRenderContractHarness.ps1",
            "Tools/EffectRenderContractHarness/Test-EffectRenderResourceRoot.ps1",
            "Tools/EffectRenderContractHarness/test_effect_render_harness_contract.py",
        ):
            self.assertFalse((ROOT / relative).exists(), relative)
        source = read(
            "Tools/RenderingPipeline/ProductEffectShaderWarpProbe.cpp"
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
        shader_closure = read("Tools/Build/Test-CompiledShaderClosure.ps1")
        self.assertNotIn("EffectRenderContractHarness", solution)
        self.assertNotIn("EffectRenderContractHarness", runner)
        self.assertNotIn("EffectRenderContractHarness", shader_closure)
        self.assertIn("Invoke-ProductEffectShaderWarpProbe", shader_closure)

    def test_action_presentation_harness_is_partitioned_and_physically_retired(self) -> None:
        for relative in (
            "Tools/ActionPresentationTimelineHarness/Default/ActionPresentationTimelineHarness.vcxproj",
            "Tools/ActionPresentationTimelineHarness/Default/ActionPresentationTimelineHarness.vcxproj.filters",
            "Tools/ActionPresentationTimelineHarness/Private/ActionPresentationTimelineHarness.cpp",
            "Tools/ActionPresentationTimelineHarness/Private/ClientPartyRegression.cpp",
            "Tools/ActionPresentationTimelineHarness/Run-ActionPresentationTimelineHarness.ps1",
        ):
            self.assertFalse((ROOT / relative).exists(), relative)

        valtan_project_path = (
            ROOT
            / "Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj"
        )
        valtan_filters_path = valtan_project_path.with_suffix(".vcxproj.filters")
        character_project_path = (
            ROOT
            / "Tools/CharacterSelectIsolationHarness/Default/CharacterSelectIsolationHarness.vcxproj"
        )
        character_filters_path = character_project_path.with_suffix(".vcxproj.filters")
        namespace = {"m": "http://schemas.microsoft.com/developer/msbuild/2003"}
        for project_path, filters_path in (
            (valtan_project_path, valtan_filters_path),
            (character_project_path, character_filters_path),
        ):
            project_root = ET.parse(project_path).getroot()
            filters_root = ET.parse(filters_path).getroot()
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
            self.assertEqual(project_sources, filtered_sources, project_path)

        valtan_project = read(
            "Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj"
        )
        character_project = read(
            "Tools/CharacterSelectIsolationHarness/Default/CharacterSelectIsolationHarness.vcxproj"
        )
        self.assertIn("ValtanPresentationContractTests.cpp", valtan_project)
        self.assertIn("ClientPresentationPrimitiveContractTests.cpp", character_project)
        self.assertGreaterEqual(
            valtan_project.count("<MultiProcessorCompilation>true</MultiProcessorCompilation>"),
            1,
        )
        self.assertGreaterEqual(
            character_project.count("<MultiProcessorCompilation>true</MultiProcessorCompilation>"),
            4,
        )
        self.assertGreaterEqual(valtan_project.count("/MP /utf-8"), 1)
        self.assertGreaterEqual(character_project.count("/MP /utf-8"), 4)
        # The focused native suites consume the product Animation/Pattern Sound
        # parsers and the exact Encounter/cinematic loaders used by the canonical
        # Valtan graph.  Keeping these physical sources explicit prevents a mock
        # JSON projection from replacing the EXE admission path.
        native_contract_sources = (
            "ValtanPatternAuditionService.cpp",
            "ValtanPatternTree.cpp",
            "ActionCompositionGraphModel.cpp",
            "BossLogicFlowViewModel.cpp",
            "ValtanPresentationGenerationAdmission.cpp",
            "EffectV2_Document.cpp",
            "ValtanPatternFlowDocument.cpp",
            "ValtanPatternEffectCueDocument.cpp",
            "ValtanPatternEffectCueAuthoring.cpp",
            "Effect_DirectAuthoredSourceIndex.cpp",
            "ValtanPatternFlowService.cpp",
            "ValtanTuningCommandService.cpp",
            "AnimationSkillBindingDocument.cpp",
            "DataJson.cpp",
            "ProjectDataRoot.cpp",
            "RuntimeAssetRoot.cpp",
            "SoundCueCatalog.cpp",
            "ValtanCombatObjectSoundCueDocument.cpp",
            "ValtanPatternSoundCueDocument.cpp",
            "ActionPresentationTimeline.cpp",
            "CameraShakeService.cpp",
            "EncounterPatternReference.cpp",
            "ValtanCinematicCameraController.cpp",
            "ValtanCinematicCameraDocument.cpp",
        )
        for native_contract_source in native_contract_sources:
            self.assertIn(
                f"..\\..\\..\\Client\\Private\\{native_contract_source}",
                valtan_project,
            )
        self.assertEqual(
            len(native_contract_sources),
            valtan_project.count("..\\..\\..\\Client\\Private\\"),
        )
        self.assertEqual(1, character_project.count("..\\..\\..\\Client\\Private\\"))

        valtan_tests = read(
            "Tools/ValtanPatternAuditionServiceHarness/Private/ValtanPresentationContractTests.cpp"
        )
        party_tests = read(
            "Tools/CharacterSelectIsolationHarness/Private/ClientPresentationPrimitiveContractTests.cpp"
        )
        sound_tests = read("Tools/ValtanPipeline/test_valtan_pattern_sound_cue_contract.py")
        for assertion in (
            "VerifyFiniteDeathPresentationClock",
            "VerifyCameraShakeSpec",
            "VerifyAdjacentExplicitSourceWindows",
            "VerifyLegacyNaturalEndCompatibility",
            "VerifyClipOccurrenceTransitions",
            "VerifyCompletedAnimationClockRelease",
            "VerifyFiniteLoopAnimationClockRelease",
            "VerifyProductPreviewClock",
            "VerifyElementStartTimelineRoundTrip",
            "VerifyCuePreviewDuration",
            "VerifyNaturalProductPreviewDurationFloor",
            "VerifyMonsterActionOccurrenceProjection",
            "VerifyBoundedCameraTransitionSampler",
            "VerifyValtanCinematicTracking",
        ):
            self.assertIn(assertion, valtan_tests)
        for assertion in (
            "VerifyMousePressOwnership",
            "VerifyIndependentLoopedSound",
            "VerifyReplicatedPartyHealth",
            "VerifyPartyTransferNotice",
        ):
            self.assertIn(assertion, party_tests)
        self.assertIn("test_authored_rows_are_deterministic_extracted_joins", sound_tests)
        self.assertIn("test_invalid_identity_join_window_and_repeat_are_fail_closed", sound_tests)


if __name__ == "__main__":
    unittest.main()
