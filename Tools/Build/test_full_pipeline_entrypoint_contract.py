#!/usr/bin/env python3
"""Focused structural contracts for the repository-root full pipeline entry."""

from __future__ import annotations

import json
import pathlib
import subprocess
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
POWERSHELL_ENTRY = ROOT / "Tools/Build/Run-FullPipeline.ps1"
BUILD_RUNNER = ROOT / "Tools/Build/Invoke-BuildAndRegression.ps1"
BATCH_ENTRY = ROOT / "RunFullPipeline.bat"
DOMAIN_MANIFEST = ROOT / "Tools/Build/BuildDomains.json"


def read(path: pathlib.Path) -> str:
    return path.read_text(encoding="utf-8-sig")


class FullPipelineEntrypointContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.runner = read(POWERSHELL_ENTRY)
        cls.build_runner = read(BUILD_RUNNER)
        cls.batch = read(BATCH_ENTRY)
        cls.manifest = json.loads(read(DOMAIN_MANIFEST))

    def test_powershell_entrypoint_parses(self) -> None:
        command = (
            "$tokens=$null; $errors=$null; "
            "[void][Management.Automation.Language.Parser]::ParseFile("
            f"'{POWERSHELL_ENTRY.as_posix()}', [ref]$tokens, [ref]$errors); "
            "if ($errors.Count -ne 0) { $errors | ForEach-Object { "
            "[Console]::Error.WriteLine($_.Message) }; exit 1 }"
        )
        result = subprocess.run(
            ["powershell.exe", "-NoProfile", "-Command", command],
            cwd=ROOT,
            text=True,
            capture_output=True,
            timeout=30,
            check=False,
        )
        self.assertEqual(
            0,
            result.returncode,
            f"stdout:\n{result.stdout}\nstderr:\n{result.stderr}",
        )

    def test_step_result_cannot_overwrite_the_typed_scriptblock_parameter(self) -> None:
        # PowerShell variable names are case-insensitive, so `$body = & $Body`
        # attempts to assign a step result such as "PASS" back into the typed
        # [scriptblock] parameter.  Keep the result in a distinctly named
        # variable and do not call GetNewClosure on Invoke-PipelineStep output.
        self.assertIn("$bodyResult = & $Body", self.runner)
        self.assertNotIn("$body = & $Body", self.runner)
        self.assertNotIn("}.GetNewClosure()", self.runner)

    def test_publish_v2_precedes_receipted_valtan_validation(self) -> None:
        publish = self.runner.index(
            "Invoke-PipelineStep 'Valtan split Product PublishV2'"
        )
        validate = self.runner.index(
            "Invoke-PipelineStep 'Valtan split source Validate (post-condition)'"
        )
        self.assertLess(publish, validate)
        publish_block = self.runner[publish:validate]
        self.assertIn("Project-ValtanPatternMaster.ps1", publish_block)
        self.assertIn("-Mode PublishV2", publish_block)

        validate_end = self.runner.index("if ($DataOnly)", validate)
        validate_block = self.runner[validate:validate_end]
        self.assertIn(
            "Get-BuildDomainById $manifest 'valtan.product'",
            self.runner[publish:validate_end],
        )
        self.assertIn("Invoke-BuildDomain", validate_block)
        self.assertNotIn("Publish-ValtanTuningRuntimeSet.ps1", self.runner)

    def test_data_only_executes_the_complete_full_diagnostic_domain_set(self) -> None:
        domains = self.manifest["domains"]
        self.assertTrue(domains)
        full_diagnostic_ids = {
            row["id"]
            for row in domains
            if "FullDiagnostic" in row["profiles"]
        }
        self.assertEqual({row["id"] for row in domains}, full_diagnostic_ids)

        data_start = self.runner.index("if ($DataOnly)")
        build_start = self.runner.index("else {", data_start)
        data_block = self.runner[data_start:build_start]
        self.assertIn(
            "Get-BuildDomainsForProfile $manifest 'FullDiagnostic'", data_block
        )
        self.assertIn("Invoke-BuildDomain", data_block)
        self.assertIn("'valtan.product'", data_block)

    def test_build_mode_delegates_to_the_canonical_runner(self) -> None:
        data_start = self.runner.index("if ($DataOnly)")
        build_start = self.runner.index("else {", data_start)
        build_block = self.runner[build_start:]
        self.assertIn("Tools/Build/Invoke-BuildAndRegression.ps1", build_block)
        self.assertIn("-Configuration $Configuration", build_block)
        self.assertIn("-Profile $Profile", build_block)
        self.assertIn("-ResourceRoot $runtimeResourceRoot", build_block)
        self.assertNotIn("Invoke-BuildDomain", build_block)
        self.assertIn("$ErrorActionPreference = 'Continue'", build_block)
        self.assertIn("$buildExitCode = $global:LASTEXITCODE", build_block)
        self.assertIn("if ($buildExitCode -ne 0)", build_block)

        for duplicate_publisher in (
            "Publish-BalanceRuntimeSet.ps1",
            "Publish-GameplayBalance.ps1",
            "Publish-WorldGameplay.ps1",
            "Publish-ServerNavigation.ps1",
            "Publish-ValtanWorldDestruction.ps1",
            "Publish-ItemCatalog.ps1",
            "Publish-ValtanClearRewards.ps1",
        ):
            self.assertNotIn(duplicate_publisher, self.runner)

    def test_failures_have_stable_classes_and_exit_codes(self) -> None:
        expected_exit_codes = {
            "DOMAIN_VALIDATION": 2,
            "STALE_REVISION": 3,
            "OUTPUT_LOCKED": 4,
            "COMPILE": 5,
            "LINK": 6,
            "REGRESSION": 7,
        }
        for failure_class, exit_code in expected_exit_codes.items():
            self.assertIn(f"'{failure_class}' {{ return {exit_code} }}", self.runner)
        self.assertIn("FULL_PIPELINE_FAILURE_CLASS=$script:failureClass", self.runner)
        self.assertIn("exit (Get-FailureExitCode $script:failureClass)", self.runner)

    def test_exact_valtan_source_revision_is_guarded_and_receipted(self) -> None:
        self.assertIn("[string]$ExpectedValtanSourceRevision = ''", self.runner)
        self.assertIn("source-manifest --repository-only", self.runner)
        self.assertIn("function Assert-ValtanSourceRevision", self.runner)
        self.assertIn("'STALE_REVISION|split authoring Product drift", self.runner)
        for phase in (
            "'PublishV2 start'",
            "'PublishV2 completion'",
            "'valtan.product validation start'",
            "'valtan.product validation completion'",
            "'full pipeline receipt'",
        ):
            self.assertIn(phase, self.runner)
        self.assertIn(
            'Write-Host "FULL_PIPELINE_SOURCE_REVISION`t$pinnedValtanSourceRevision"',
            self.runner,
        )
        self.assertIn("'Full pipeline admission' '' $_.Exception.Message", self.runner)
        self.assertIn(
            "-ExpectedValtanSourceRevision $pinnedValtanSourceRevision",
            self.runner,
        )

        self.assertIn(
            "[string]$ExpectedValtanSourceRevision = ''", self.build_runner
        )
        self.assertIn(
            "source-manifest --repository-only", self.build_runner
        )
        self.assertIn(
            "function Assert-ExpectedValtanSourceRevision", self.build_runner
        )
        for phase in (
            "'build runner admission'",
            '"domain $($domain.id) start"',
            '"domain $($domain.id) completion"',
            "'product receipt start'",
            "'product receipt completion'",
            "'final evidence start'",
            "'final evidence completion'",
        ):
            self.assertIn(phase, self.build_runner)

    def test_batch_file_is_only_a_passthrough_with_exit_propagation(self) -> None:
        normalized = self.batch.replace("\r\n", "\n")
        self.assertEqual(1, normalized.count("Run-FullPipeline.ps1"))
        self.assertIn('powershell -NoProfile -ExecutionPolicy Bypass -File', normalized)
        self.assertIn("%*", normalized)
        self.assertIn('set "EXIT_CODE=%ERRORLEVEL%"', normalized)
        self.assertIn("exit /b %EXIT_CODE%", normalized)


if __name__ == "__main__":
    unittest.main(verbosity=2)
