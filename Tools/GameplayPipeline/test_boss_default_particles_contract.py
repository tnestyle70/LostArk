"""Exercise the publisher's actual PowerShell validators without publishing data."""
import copy
import json
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


@unittest.skipUnless(shutil.which("powershell"), "PowerShell is required")
class BossDefaultParticlesContractTests(unittest.TestCase):
    def test_saved_valtan_particles_and_invalid_payloads(self):
        catalog = json.loads((ROOT / "Data/Actors/BossCatalog.json").read_text(encoding="utf-8-sig"))
        saved = [boss for boss in catalog["bosses"] if "defaultParticles" in boss]
        self.assertTrue(saved)
        cases = [{"name": boss["archetypeId"], "boss": boss, "accept": True} for boss in saved]
        baseline = saved[0]

        def add(name, mutate, accept=False):
            boss = copy.deepcopy(baseline)
            mutate(boss)
            cases.append({"name": name, "boss": boss, "accept": accept})

        add("empty", lambda b: b.update(defaultParticles=[]), True)
        add("null", lambda b: b.update(defaultParticles=None))
        add("not-array", lambda b: b.update(defaultParticles={}))
        add("too-many", lambda b: b.update(defaultParticles=b["defaultParticles"] * 17))
        add("wrong-consumer", lambda b: b.update(clientPresentationId="boss.kakulsaydon.g1.kouku.client.v1"))
        add("duplicate", lambda b: b["defaultParticles"].append(copy.deepcopy(b["defaultParticles"][0])))
        add("unknown-field", lambda b: b["defaultParticles"][0].update(unexpected=0))
        add("missing-field", lambda b: b["defaultParticles"][0].pop("scale"))
        add("bad-id", lambda b: b["defaultParticles"][0].update(effectAssetId="../bad"))
        add("empty-bone", lambda b: b["defaultParticles"][0].update(boneName=""))
        add("bone-byte-limit", lambda b: b["defaultParticles"][0].update(boneName="\uac00" * 43))
        add("vector-size", lambda b: b["defaultParticles"][0].update(position=[1, 2]))
        add("string-number", lambda b: b["defaultParticles"][0].update(position=["1", 2, 3]))
        add("bool-number", lambda b: b["defaultParticles"][0].update(scale=[True, 1, 1]))
        add("position-range", lambda b: b["defaultParticles"][0].update(position=[1000.01, 0, 0]))
        add("rotation-range", lambda b: b["defaultParticles"][0].update(rotationDegrees=[0, -360.01, 0]))
        add("zero-scale", lambda b: b["defaultParticles"][0].update(scale=[0, 1, 1]))
        add("scale-range", lambda b: b["defaultParticles"][0].update(scale=[1, 100.01, 1]))
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            (directory / "cases.json").write_text(json.dumps(cases, ensure_ascii=True), encoding="utf-8")
            script = directory / "run.ps1"
            script.write_text(r'''
param([string]$Publisher, [string]$Cases)
$ErrorActionPreference = 'Stop'
$tokens = $null
$errors = $null
$ast = [Management.Automation.Language.Parser]::ParseFile($Publisher, [ref]$tokens, [ref]$errors)
if ($errors.Count) { throw ($errors | Out-String) }
$names = @('Assert-ExactProperties', 'Assert-StableId', 'Assert-JsonString', 'Assert-JsonNumber', 'Assert-BossDefaultParticles')
$functions = @($ast.FindAll({ param($node) $node -is [Management.Automation.Language.FunctionDefinitionAst] -and $node.Name -in $names }, $true))
if ($functions.Count -ne $names.Count) { throw 'Missing actual publisher function.' }
foreach ($function in $functions) { Invoke-Expression $function.Extent.Text }
$stableIdPattern = '^[A-Za-z0-9_.-]{1,128}$'
$fixtureRows = Get-Content -LiteralPath $Cases -Raw -Encoding UTF8 | ConvertFrom-Json
foreach ($case in $fixtureRows) {
    $accepted = $true
    try { Assert-BossDefaultParticles $case.boss } catch { $accepted = $false }
    if ($accepted -ne $case.accept) { throw "Unexpected admission: $($case.name), accepted=$accepted" }
}
Write-Output "PASS $($fixtureRows.Count) actual publisher default-particle cases"
''', encoding="utf-8")
            result = subprocess.run([
                "powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(script),
                "-Publisher", str(ROOT / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1"),
                "-Cases", str(directory / "cases.json"),
            ], capture_output=True, text=True, errors="replace", timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            print(result.stdout.strip())


if __name__ == "__main__":
    unittest.main()
