#!/usr/bin/env python3
"""Focused contracts for build-domain fingerprint and freshness receipts."""

from __future__ import annotations

import json
import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MODULE = ROOT / "Tools/Build/BuildDomainPipeline.psm1"
MANIFEST = ROOT / "Tools/Build/BuildDomains.json"


def run(command: list[str], cwd: Path) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        command,
        cwd=cwd,
        check=True,
        capture_output=True,
        text=True,
        encoding="utf-8",
    )


def powershell(script: str, cwd: Path) -> str:
    result = subprocess.run(
        [
            "powershell.exe",
            "-NoProfile",
            "-ExecutionPolicy",
            "Bypass",
            "-Command",
            script,
        ],
        cwd=cwd,
        check=False,
        capture_output=True,
        text=True,
        encoding="utf-8",
    )
    if result.returncode != 0:
        raise AssertionError(
            f"PowerShell failed ({result.returncode}).\n"
            f"stdout:\n{result.stdout}\nstderr:\n{result.stderr}"
        )
    return result.stdout.strip()


class BuildDomainManifestContractTests(unittest.TestCase):
    def setUp(self) -> None:
        self.document = json.loads(MANIFEST.read_text(encoding="utf-8"))
        self.domains = {row["id"]: row for row in self.document["domains"]}

    def test_graph_has_one_cached_valtan_validator_and_preserves_effect_v2(self) -> None:
        self.assertEqual(self.document["schema"], "lostark.build-domain-graph")
        self.assertEqual(self.document["formatVersion"], 1)
        self.assertEqual(len(self.domains), len(self.document["domains"]))

        valtan = self.domains["valtan.product"]
        self.assertEqual(valtan["kind"], "validation")
        self.assertEqual(
            valtan["profiles"], ["Product", "Core", "FullDiagnostic"]
        )
        self.assertEqual(
            valtan["action"]["arguments"].count("Validate"), 1
        )
        self.assertIn(
            "Tools/ValtanPipeline/Format-PreservingJsonArray.ps1",
            valtan["tools"],
        )

        effect_v2 = self.domains["effect.v2"]
        self.assertEqual(effect_v2["profiles"], ["Core", "FullDiagnostic"])
        self.assertIn("Data/Effects/V2/**", effect_v2["inputs"])
        self.assertEqual(len(effect_v2["resourceReferenceFields"]), 6)
        self.assertIn("validate_effect_v2.py", json.dumps(effect_v2))
        for tool in (
            "Tools/EffectToolV2/effect_v2_binding_pipeline.py",
            "Tools/EffectToolV2/Schemas/lostark.effect-v2-bindings.v2.schema.json",
            "Tools/EffectToolV2/Schemas/lostark.effect-v2-binding-read-set.v1.schema.json",
            "Tools/EffectToolV2/Schemas/lostark.effect-v2-group.v2.schema.json",
        ):
            self.assertIn(tool, effect_v2["tools"])

    def test_publishers_have_owned_outputs_and_skip_duplicate_valtan_join(self) -> None:
        publishers = {
            domain_id
            for domain_id, domain in self.domains.items()
            if domain["kind"] == "publisher"
        }
        self.assertEqual(
            publishers,
            {
                "map.kakul",
                "world.gameplay",
                "navigation",
                "world.destruction",
                "gameplay.balance",
                "items.catalog",
                "valtan.rewards",
            },
        )
        for domain_id in publishers:
            domain = self.domains[domain_id]
            self.assertTrue(domain["outputs"], domain_id)
            self.assertTrue(domain["requiredOutputPatterns"], domain_id)
            self.assertIn("Publish", domain["action"]["arguments"])

        gameplay = self.domains["gameplay.balance"]
        self.assertIn("-SkipValtanSplitProjection", gameplay["action"]["arguments"])
        self.assertIn("Data/Effects/V2/**", gameplay["inputs"])

    def test_kakul_map_publisher_has_exact_clean_checkout_closure(self) -> None:
        domain = self.domains["map.kakul"]
        self.assertEqual(domain["kind"], "publisher")
        self.assertEqual(domain["profiles"], ["Product", "Core", "FullDiagnostic"])
        self.assertEqual(
            domain["inputs"],
            [
                "Data/Maps/Imported/LV_LUT_MIDNIGHTC_ED/**",
                "Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/**",
                "Data/Maps/MapCatalog.json",
            ],
        )
        self.assertEqual(
            domain["tools"], ["Tools/MapPipeline/Publish-MapAuthoring.ps1"]
        )
        expected_outputs = [
            "Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.mapassets",
            "Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.mapplacements",
            "Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.deployassets",
            "Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.deployplacements",
        ]
        self.assertEqual(domain["outputs"], expected_outputs)
        self.assertEqual(domain["requiredOutputPatterns"], expected_outputs)
        arguments = domain["action"]["arguments"]
        for token in (
            "Tools/MapPipeline/Publish-MapAuthoring.ps1",
            "LV_LUT_MIDNIGHTC_ED",
            "{repositoryRoot}",
            "Publish",
        ):
            self.assertIn(token, arguments)

    def test_client_owner_materializes_kakul_map_world_and_navigation(self) -> None:
        owner = (ROOT / "Tools/Build/Invoke-BuildDomainOwner.ps1").read_text(
            encoding="utf-8-sig"
        )
        client_block = owner.split("if ($Owner -eq 'Client') {", 1)[1].split(
            "else {", 1
        )[0]
        for domain_id in (
            "map.kakul",
            "world.gameplay",
            "navigation",
            "valtan.product",
        ):
            self.assertIn(f"'{domain_id}'", client_block)

        server_block = owner.split("else {", 1)[1].split("}", 1)[0]
        self.assertNotIn("'map.kakul'", server_block)

    def test_client_project_exposes_kakul_world_and_navigation_authoring(self) -> None:
        project = (ROOT / "Client/Default/Client.vcxproj").read_text(
            encoding="utf-8-sig"
        )
        filters = (ROOT / "Client/Default/Client.vcxproj.filters").read_text(
            encoding="utf-8-sig"
        )
        paths = (
            r"..\..\Data\Navigation\LV_LUT_MIDNIGHTC_ED.navsource",
            r"..\..\Data\Navigation\LV_LUT_MIDNIGHTC_ED.navpaint",
            r"..\..\Data\Worlds\LV_LUT_MIDNIGHTC_ED\Gameplay.world.json",
            r"..\..\Data\Worlds\LV_LUT_MIDNIGHTC_ED\StageMarkers.json",
        )
        for path in paths:
            self.assertEqual(project.count(f'<None Include="{path}" />'), 1, path)
            self.assertEqual(filters.count(f'<None Include="{path}">'), 1, path)

    def test_product_receipt_closes_exe_and_pdb_pairs(self) -> None:
        product = self.document["product"]
        outputs = set(product["outputs"])
        for owner, binary in (
            ("Engine", "Engine.dll"),
            ("Server", "Server.exe"),
            ("Client", "Client.exe"),
        ):
            root = f"{owner}/Bin/{{configuration}}"
            self.assertIn(f"{root}/{binary}", outputs)
            self.assertIn(f"{root}/{owner}.pdb", outputs)
        self.assertIn(
            "Shared/Bin/{configuration}/Shared.pdb",
            outputs,
        )
        for deployed in (
            "Client/Bin/{configuration}/Engine.dll",
            "Client/Bin/{configuration}/fmod.dll",
            "Client/Bin/{configuration}/{assimpRuntimeName}",
            "Client/Bin/{configuration}/PhysX_64.dll",
            "Client/Bin/{configuration}/PhysXCommon_64.dll",
            "Client/Bin/{configuration}/PhysXFoundation_64.dll",
            "EngineSDK/lib/{configuration}/Engine.lib",
        ):
            self.assertIn(deployed, outputs)
        self.assertIn(
            "Client/Bin/Debug/Engine.pdb",
            product["outputsByConfiguration"]["Debug"],
        )
        self.assertGreaterEqual(len(product["deploymentPairs"]), 9)

    def test_project_hooks_use_the_same_receipt_owner_without_boolean_bypass(self) -> None:
        server = (ROOT / "Server/Default/Server.vcxproj").read_text(
            encoding="utf-8-sig"
        )
        client = (ROOT / "Client/Default/Client.vcxproj").read_text(
            encoding="utf-8-sig"
        )
        helper = "Tools\\Build\\Invoke-BuildDomainOwner.ps1"
        self.assertIn(helper, server)
        self.assertIn(helper, client)
        self.assertIn("-Owner Server", server)
        self.assertIn("-Owner Client", client)
        self.assertNotIn("LostArkCentralBuildReceiptsVerified", server)
        self.assertNotIn("LostArkCentralBuildReceiptsVerified", client)

    def test_runner_owns_domains_freshness_evidence_and_client_sdk_copy(self) -> None:
        runner = (ROOT / "Tools/Build/Invoke-BuildAndRegression.ps1").read_text(
            encoding="utf-8-sig"
        )
        for token in (
            "BuildDomainPipeline.psm1",
            "BuildDomains.json",
            "Invoke-SelectedBuildDomains",
            "Test-BuildProductReceipt",
            "Write-BuildProductReceipt",
            "Write-BuildRunEvidence",
            "Enter-BuildExclusiveLock",
            "Assert-BuildRunStability",
        ):
            self.assertIn(token, runner)
        self.assertNotIn("LostArkCentralBuildReceiptsVerified", runner)
        self.assertIn(
            "-SkipBuild is supported only for Product",
            runner,
        )
        for duplicate in (
            "UpdateLib.bat",
            "Publish-BalanceRuntimeSet.ps1",
            "Publish-ServerNavigation.ps1",
            "Publish-ValtanWorldDestruction.ps1",
            "Project-ValtanPatternMaster.ps1",
        ):
            self.assertNotIn(duplicate, runner)

        client = (ROOT / "Client/Default/Client.vcxproj").read_text(
            encoding="utf-8-sig"
        )
        for copy_owner in (
            'Name="PrepareEngineSdk"',
            'Name="DeployClientCompiledShaders"',
            'Name="DeployClientRuntimeDependencies"',
        ):
            self.assertIn(copy_owner, client)


class BuildDomainReceiptBehaviorTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        (self.root / "Data").mkdir()
        (self.root / "Tools").mkdir()
        (self.root / "Product/Bin/Debug").mkdir(parents=True)
        (self.root / "Runtime").mkdir()
        (self.root / "Resources").mkdir()
        (self.root / "markers").mkdir()
        (self.root / ".gitignore").write_text(
            "receipts/\nevidence/\nconcurrent-receipts/\n"
            "counter.txt\ngenerated.txt\nmarkers/\n",
            encoding="utf-8",
        )
        (self.root / "Data/input.txt").write_text("alpha\n", encoding="utf-8")
        (self.root / "Product/source.cpp").parent.mkdir(exist_ok=True)
        (self.root / "Product/source.cpp").write_text("int value = 1;\n", encoding="utf-8")
        (self.root / "Product/Bin/Debug/Product.exe").write_bytes(b"exe-one")
        (self.root / "Product/Bin/Debug/Product.pdb").write_bytes(b"pdb-one")
        (self.root / "Runtime/source.dll").write_bytes(b"runtime-one")
        (self.root / "Product/Bin/Debug/runtime.dll").write_bytes(b"runtime-one")
        (self.root / "Tools/action.ps1").write_text(
            """param([string]$Root)
$markerRoot = Join-Path $Root 'markers'
[IO.Directory]::CreateDirectory($markerRoot) | Out-Null
Start-Sleep -Milliseconds 300
Set-Content -LiteralPath (Join-Path $markerRoot "$PID.marker") -Value 'ran'
$mutationRequest = Join-Path $Root 'mutate-during-action.txt'
if (Test-Path -LiteralPath $mutationRequest) {
    Set-Content -LiteralPath (Join-Path $Root 'Data/input.txt') -Value 'mutated'
}
$counterPath = Join-Path $Root 'counter.txt'
$count = if (Test-Path -LiteralPath $counterPath) {
    [int](Get-Content -LiteralPath $counterPath -Raw)
} else { 0 }
Set-Content -LiteralPath $counterPath -Value ($count + 1) -NoNewline
Copy-Item -LiteralPath (Join-Path $Root 'Data/input.txt') -Destination (Join-Path $Root 'generated.txt') -Force
Write-Output 'fixture action diagnostic'
""",
            encoding="utf-8",
        )
        self.manifest = {
            "schema": "lostark.build-domain-graph",
            "formatVersion": 1,
            "domains": [
                {
                    "id": "fixture.domain",
                    "kind": "publisher",
                    "profiles": ["Product"],
                    "inputs": ["Data/**"],
                    "tools": ["Tools/action.ps1"],
                    "outputs": ["generated.txt"],
                    "requiredOutputPatterns": ["generated.txt"],
                    "action": {
                        "executable": "powershell.exe",
                        "arguments": [
                            "-NoProfile",
                            "-ExecutionPolicy",
                            "Bypass",
                            "-File",
                            "Tools/action.ps1",
                            "{repositoryRoot}",
                        ],
                    },
                }
            ],
            "product": {
                "inputs": ["Product/source.cpp", "Runtime/source.dll"],
                "outputs": [
                    "Product/Bin/{configuration}/Product.exe",
                    "Product/Bin/{configuration}/Product.pdb",
                    "Product/Bin/{configuration}/runtime.dll",
                ],
                "deploymentPairs": [
                    {
                        "source": "Runtime/source.dll",
                        "destination": "Product/Bin/{configuration}/runtime.dll",
                    }
                ],
            },
        }
        self.manifest_path = self.root / "BuildDomains.json"
        self.manifest_path.write_text(
            json.dumps(self.manifest, indent=2) + "\n", encoding="utf-8"
        )
        run(["git", "init", "-q"], self.root)
        run(["git", "config", "user.email", "fixture@example.invalid"], self.root)
        run(["git", "config", "user.name", "Fixture"], self.root)
        run(["git", "add", "."], self.root)
        run(["git", "commit", "-qm", "fixture"], self.root)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def test_identical_fingerprint_executes_action_once_then_input_change_reexecutes(self) -> None:
        script = f"""
$ErrorActionPreference = 'Stop'
Import-Module '{MODULE.as_posix()}' -Force
$manifest = Read-BuildDomainManifest '{self.manifest_path.as_posix()}'
$domain = Get-BuildDomainById $manifest 'fixture.domain'
$receiptRoot = Join-Path '{self.root.as_posix()}' 'receipts'
$resourceRoot = Join-Path '{self.root.as_posix()}' 'Resources'
$first = Invoke-BuildDomain '{self.root.as_posix()}' $domain $resourceRoot $receiptRoot
$second = Invoke-BuildDomain '{self.root.as_posix()}' $domain $resourceRoot $receiptRoot
[IO.File]::WriteAllText((Join-Path '{self.root.as_posix()}' 'Data/input.txt'), "beta`n")
$third = Invoke-BuildDomain '{self.root.as_posix()}' $domain $resourceRoot $receiptRoot
[pscustomobject]@{{
    firstReused = $first.reused
    secondReused = $second.reused
    thirdReused = $third.reused
    count = [int](Get-Content -LiteralPath (Join-Path '{self.root.as_posix()}' 'counter.txt') -Raw)
}} | ConvertTo-Json -Compress
"""
        report = json.loads(powershell(script, self.root).splitlines()[-1])
        self.assertFalse(report["firstReused"])
        self.assertTrue(report["secondReused"])
        self.assertFalse(report["thirdReused"])
        self.assertEqual(report["count"], 2)

    def test_input_change_during_action_fails_without_writing_receipt(self) -> None:
        (self.root / "mutate-during-action.txt").write_text(
            "mutate\n", encoding="utf-8"
        )
        script = f"""
$ErrorActionPreference = 'Stop'
Import-Module '{MODULE.as_posix()}' -Force
$manifest = Read-BuildDomainManifest '{self.manifest_path.as_posix()}'
$domain = Get-BuildDomainById $manifest 'fixture.domain'
$receiptRoot = Join-Path '{self.root.as_posix()}' 'receipts'
$resourceRoot = Join-Path '{self.root.as_posix()}' 'Resources'
$failed = $false
$message = ''
try {{
    Invoke-BuildDomain '{self.root.as_posix()}' $domain $resourceRoot $receiptRoot |
        Out-Null
}}
catch {{
    $message = $_.Exception.Message
    $failed = $message -match 'source closure changed during action'
}}
$receiptPath = Join-Path $receiptRoot 'fixture.domain.receipt.json'
[pscustomobject]@{{
    failed = $failed
    message = $message
    receiptExists = Test-Path -LiteralPath $receiptPath
}} | ConvertTo-Json -Compress
"""
        report = json.loads(powershell(script, self.root).splitlines()[-1])
        self.assertTrue(report["failed"], report["message"])
        self.assertFalse(report["receiptExists"])

    def test_concurrent_identical_fingerprint_executes_action_once(self) -> None:
        command = f"""
$ErrorActionPreference = 'Stop'
Import-Module '{MODULE.as_posix()}' -Force
$manifest = Read-BuildDomainManifest '{self.manifest_path.as_posix()}'
$domain = Get-BuildDomainById $manifest 'fixture.domain'
$result = Invoke-BuildDomain '{self.root.as_posix()}' $domain `
    '{(self.root / 'Resources').as_posix()}' `
    '{(self.root / 'concurrent-receipts').as_posix()}'
$result | ConvertTo-Json -Compress
"""
        processes = [
            subprocess.Popen(
                [
                    "powershell.exe",
                    "-NoProfile",
                    "-ExecutionPolicy",
                    "Bypass",
                    "-Command",
                    command,
                ],
                cwd=self.root,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                encoding="utf-8",
            )
            for _ in range(2)
        ]
        outputs = [process.communicate(timeout=20) for process in processes]
        for process, (stdout, stderr) in zip(processes, outputs):
            self.assertEqual(process.returncode, 0, f"{stdout}\n{stderr}")
        results = [json.loads(stdout.splitlines()[-1]) for stdout, _ in outputs]
        self.assertEqual(sorted(row["reused"] for row in results), [False, True])
        self.assertEqual(len(list((self.root / "markers").glob("*.marker"))), 1)

    def test_exclusive_lock_wait_is_bounded(self) -> None:
        script = f"""
$ErrorActionPreference = 'Stop'
Import-Module '{MODULE.as_posix()}' -Force
$path = Join-Path '{self.root.as_posix()}' 'receipts/locks/bounded.lock'
$held = Enter-BuildExclusiveLock $path 1000 'fixture holder'
$timedOut = $false
try {{
    try {{
        $unexpected = Enter-BuildExclusiveLock $path 200 'fixture waiter'
        $unexpected.Dispose()
    }}
    catch {{
        $timedOut = $_.Exception.Message -match 'Timed out waiting'
    }}
}}
finally {{
    $held.Dispose()
}}
[pscustomobject]@{{ timedOut = $timedOut }} | ConvertTo-Json -Compress
"""
        report = json.loads(powershell(script, self.root).splitlines()[-1])
        self.assertTrue(report["timedOut"])

    def test_required_output_definition_change_invalidates_reuse(self) -> None:
        script = f"""
$ErrorActionPreference = 'Stop'
Import-Module '{MODULE.as_posix()}' -Force
$manifest = Read-BuildDomainManifest '{self.manifest_path.as_posix()}'
$domain = Get-BuildDomainById $manifest 'fixture.domain'
$receiptRoot = Join-Path '{self.root.as_posix()}' 'receipts'
$resourceRoot = Join-Path '{self.root.as_posix()}' 'Resources'
Invoke-BuildDomain '{self.root.as_posix()}' $domain $resourceRoot $receiptRoot | Out-Null
$domain.requiredOutputPatterns = @($domain.requiredOutputPatterns) + 'missing.txt'
$fresh = Test-BuildDomainReceipt '{self.root.as_posix()}' $domain $resourceRoot $receiptRoot
$failed = $false
try {{
    Invoke-BuildDomain '{self.root.as_posix()}' $domain $resourceRoot $receiptRoot | Out-Null
}}
catch {{
    $failed = $_.Exception.Message -match 'required output is missing'
}}
[pscustomobject]@{{ fresh = $fresh.Fresh; failed = $failed }} |
    ConvertTo-Json -Compress
"""
        report = json.loads(powershell(script, self.root).splitlines()[-1])
        self.assertFalse(report["fresh"])
        self.assertTrue(report["failed"])

    def test_product_receipt_rejects_stale_source_and_binary(self) -> None:
        script = f"""
$ErrorActionPreference = 'Stop'
Import-Module '{MODULE.as_posix()}' -Force
$manifest = Read-BuildDomainManifest '{self.manifest_path.as_posix()}'
$receiptRoot = Join-Path '{self.root.as_posix()}' 'receipts'
Write-BuildProductReceipt '{self.root.as_posix()}' $manifest Debug $receiptRoot | Out-Null
$fresh = Test-BuildProductReceipt '{self.root.as_posix()}' $manifest Debug $receiptRoot
[IO.File]::WriteAllText((Join-Path '{self.root.as_posix()}' 'Product/source.cpp'), 'int value = 2;')
$sourceStale = Test-BuildProductReceipt '{self.root.as_posix()}' $manifest Debug $receiptRoot
[IO.File]::WriteAllText((Join-Path '{self.root.as_posix()}' 'Product/source.cpp'), 'int value = 1;`n')
[IO.File]::WriteAllBytes((Join-Path '{self.root.as_posix()}' 'Product/Bin/Debug/Product.pdb'), [byte[]](1,2,3))
$binaryStale = Test-BuildProductReceipt '{self.root.as_posix()}' $manifest Debug $receiptRoot
[pscustomobject]@{{
    fresh = $fresh.Fresh
    sourceStale = $sourceStale.Fresh
    binaryStale = $binaryStale.Fresh
}} | ConvertTo-Json -Compress
"""
        report = json.loads(powershell(script, self.root).splitlines()[-1])
        self.assertTrue(report["fresh"])
        self.assertFalse(report["sourceStale"])
        self.assertFalse(report["binaryStale"])

    def test_product_receipt_rejects_stale_deployed_runtime(self) -> None:
        script = f"""
$ErrorActionPreference = 'Stop'
Import-Module '{MODULE.as_posix()}' -Force
$manifest = Read-BuildDomainManifest '{self.manifest_path.as_posix()}'
$receiptRoot = Join-Path '{self.root.as_posix()}' 'receipts'
Write-BuildProductReceipt '{self.root.as_posix()}' $manifest Debug $receiptRoot | Out-Null
[IO.File]::WriteAllBytes(
    (Join-Path '{self.root.as_posix()}' 'Product/Bin/Debug/runtime.dll'),
    [byte[]](9,8,7))
$stale = Test-BuildProductReceipt '{self.root.as_posix()}' $manifest Debug $receiptRoot
[pscustomobject]@{{ fresh = $stale.Fresh; reason = $stale.Reason }} |
    ConvertTo-Json -Compress
"""
        report = json.loads(powershell(script, self.root).splitlines()[-1])
        self.assertFalse(report["fresh"])
        self.assertIn("deployment", report["reason"])

    def test_dirty_identity_changes_when_same_path_content_changes(self) -> None:
        script = f"""
$ErrorActionPreference = 'Stop'
Import-Module '{MODULE.as_posix()}' -Force
$path = Join-Path '{self.root.as_posix()}' 'Data/input.txt'
[IO.File]::WriteAllText($path, 'first')
$first = Get-BuildGitIdentity '{self.root.as_posix()}'
[IO.File]::WriteAllText($path, 'second')
$second = Get-BuildGitIdentity '{self.root.as_posix()}'
[pscustomobject]@{{
    first = $first.dirtyIdentitySha256
    second = $second.dirtyIdentitySha256
}} | ConvertTo-Json -Compress
"""
        report = json.loads(powershell(script, self.root).splitlines()[-1])
        self.assertNotEqual(report["first"], report["second"])

    def test_postbuild_source_change_is_rejected_before_evidence(self) -> None:
        script = f"""
$ErrorActionPreference = 'Stop'
Import-Module '{MODULE.as_posix()}' -Force
$manifest = Read-BuildDomainManifest '{self.manifest_path.as_posix()}'
$domain = Get-BuildDomainById $manifest 'fixture.domain'
$receiptRoot = Join-Path '{self.root.as_posix()}' 'receipts'
$resourceRoot = Join-Path '{self.root.as_posix()}' 'Resources'
$domainResult = Invoke-BuildDomain '{self.root.as_posix()}' $domain $resourceRoot $receiptRoot
$startGit = Get-BuildGitIdentity '{self.root.as_posix()}'
$startSource = Get-BuildProductSourceFingerprint `
    '{self.root.as_posix()}' $manifest Debug
Write-BuildProductReceipt '{self.root.as_posix()}' $manifest Debug $receiptRoot | Out-Null
[IO.File]::WriteAllText(
    (Join-Path '{self.root.as_posix()}' 'Product/source.cpp'),
    'int value = 99;')
$failed = $false
try {{
    Assert-BuildRunStability '{self.root.as_posix()}' $manifest Debug `
        $resourceRoot $receiptRoot $startGit $startSource.sourceInputSha256 `
        @($domainResult) | Out-Null
}}
catch {{
    $failed = $_.Exception.Message -match 'identity changed|fingerprint changed'
}}
[pscustomobject]@{{ failed = $failed }} | ConvertTo-Json -Compress
"""
        report = json.loads(powershell(script, self.root).splitlines()[-1])
        self.assertTrue(report["failed"])

    def test_run_evidence_records_git_dirty_source_binary_and_step_hashes(self) -> None:
        script = f"""
$ErrorActionPreference = 'Stop'
Import-Module '{MODULE.as_posix()}' -Force
$manifest = Read-BuildDomainManifest '{self.manifest_path.as_posix()}'
$domain = Get-BuildDomainById $manifest 'fixture.domain'
$receiptRoot = Join-Path '{self.root.as_posix()}' 'receipts'
$resourceRoot = Join-Path '{self.root.as_posix()}' 'Resources'
$domainResult = Invoke-BuildDomain '{self.root.as_posix()}' $domain $resourceRoot $receiptRoot
$startGit = Get-BuildGitIdentity '{self.root.as_posix()}'
$startSource = Get-BuildProductSourceFingerprint '{self.root.as_posix()}' $manifest Debug
Write-BuildProductReceipt '{self.root.as_posix()}' $manifest Debug $receiptRoot | Out-Null
$stability = Assert-BuildRunStability '{self.root.as_posix()}' $manifest Debug `
    $resourceRoot $receiptRoot $startGit $startSource.sourceInputSha256 `
    @($domainResult)
$steps = @([pscustomobject]@{{ name = 'fixture'; elapsedMs = 17; result = 'PASS' }})
$path = Write-BuildRunEvidence '{self.root.as_posix()}' Debug Core $false `
    $steps @($domainResult) (Join-Path '{self.root.as_posix()}' 'evidence') `
    $stability '2026-08-30T00:00:00.0000000Z' 23
Get-Content -LiteralPath $path -Raw | ConvertFrom-Json |
    ConvertTo-Json -Depth 30 -Compress
"""
        report = json.loads(powershell(script, self.root).splitlines()[-1])
        self.assertEqual(report["schema"], "lostark.build-run-evidence")
        self.assertFalse(report["git"]["dirty"])
        self.assertRegex(report["git"]["head"], r"^[0-9a-f]{40}$")
        self.assertRegex(report["sourceInputSha256"], r"^[0-9a-f]{64}$")
        self.assertRegex(report["productReceiptSha256"], r"^[0-9a-f]{64}$")
        self.assertEqual(len(report["binaries"]), 3)
        for row in report["binaries"]:
            self.assertRegex(row["sha256"], r"^[0-9a-f]{64}$")
            self.assertIn("lastWriteTimeUtc", row)
        self.assertRegex(report["domains"][0]["receiptSha256"], r"^[0-9a-f]{64}$")
        self.assertIn("outputs", report["domains"][0])
        self.assertEqual(report["steps"][0]["elapsedMs"], 17)
        self.assertEqual(report["elapsedMs"], 23)


if __name__ == "__main__":
    unittest.main()
