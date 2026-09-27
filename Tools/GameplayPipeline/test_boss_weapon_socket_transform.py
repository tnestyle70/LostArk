"""Run the real publisher's optional socket validator on valid and corrupt data."""
import copy
import json
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


@unittest.skipUnless(shutil.which("powershell"), "PowerShell is required")
class BossWeaponSocketTests(unittest.TestCase):
    def test_actual_publisher_admission(self):
        catalog = json.loads((ROOT / "Data/Actors/BossCatalog.json").read_text(encoding="utf-8-sig"))
        boss = next(row for row in catalog["bosses"] if row["archetypeId"] == "BOSS_VALTAN")
        cases = [{"name": "legacy-zero-default", "boss": copy.deepcopy(boss), "accept": True}]
        boss["weaponSocketTransform"] = {"positionMeters": [0, 0, 0], "rotationDegrees": [0, 0, 0]}

        def case(name, mutate, accept=False):
            row = copy.deepcopy(boss)
            mutate(row)
            cases.append({"name": name, "boss": row, "accept": accept})

        case("normal", lambda row: None, True)
        case("ghost", lambda row: row.update(archetypeId="BOSS_VALTAN_GHOST"), True)
        case("bounds", lambda row: row.update(weaponSocketTransform={
            "positionMeters": [-10, 10, 0], "rotationDegrees": [-360, 360, 0]}), True)
        case("owner", lambda row: row.update(archetypeId="BOSS_KAKULSAYDON_G1_KOUKU"))
        case("weaponless", lambda row: row.update(weaponModel=None))
        case("null", lambda row: row.update(weaponSocketTransform=None))
        case("missing-field", lambda row: row["weaponSocketTransform"].pop("positionMeters"))
        case("extra-field", lambda row: row["weaponSocketTransform"].update(scale=[1, 1, 1]))
        case("short-vector", lambda row: row["weaponSocketTransform"].update(positionMeters=[0, 0]))
        case("boolean", lambda row: row["weaponSocketTransform"].update(positionMeters=[True, 0, 0]))
        case("string", lambda row: row["weaponSocketTransform"].update(rotationDegrees=["1", 0, 0]))
        case("position-bound", lambda row: row["weaponSocketTransform"].update(positionMeters=[10.01, 0, 0]))
        case("rotation-bound", lambda row: row["weaponSocketTransform"].update(rotationDegrees=[0, 0, -360.01]))
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            (directory / "cases.json").write_text(json.dumps(cases), encoding="utf-8")
            script = directory / "test.ps1"
            script.write_text(r'''
param([string]$Publisher, [string]$Cases)
$ErrorActionPreference = 'Stop'
$tokens = $null
$errors = $null
$ast = [Management.Automation.Language.Parser]::ParseFile($Publisher, [ref]$tokens, [ref]$errors)
if ($errors.Count) { throw ($errors | Out-String) }
$names = @('Assert-ExactProperties','Assert-JsonNumber','Assert-BossWeaponSocketTransform')
$helperPath = Join-Path (Split-Path (Split-Path $Publisher)) 'KoukuSaydonPipeline/KoukuBootstrapRows.ps1'
$helperAst = [Management.Automation.Language.Parser]::ParseFile($helperPath, [ref]$tokens, [ref]$errors)
if ($errors.Count) { throw ($errors | Out-String) }
$functions = @($ast.FindAll({ param($node)
    $node -is [Management.Automation.Language.FunctionDefinitionAst] -and $node.Name -in $names
}, $true))
$functions += @($helperAst.FindAll({ param($node)
    $node -is [Management.Automation.Language.FunctionDefinitionAst] -and $node.Name -in $names
}, $true))
if ($functions.Count -ne $names.Count) { throw 'Missing actual publisher function.' }
foreach ($function in $functions) { Invoke-Expression $function.Extent.Text }
$rows = Get-Content -LiteralPath $Cases -Raw -Encoding UTF8 | ConvertFrom-Json
foreach ($case in $rows) {
    $accepted = $true
    try { Assert-BossWeaponSocketTransform $case.boss } catch { $accepted = $false }
    if ($accepted -ne $case.accept) { throw "Wrong result: $($case.name), accepted=$accepted" }
}
Write-Output "PASS $($rows.Count) actual publisher weapon socket cases"
''', encoding="utf-8")
            completed = subprocess.run([
                "powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(script),
                "-Publisher", str(ROOT / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1"),
                "-Cases", str(directory / "cases.json")], capture_output=True, text=True)
            self.assertEqual(completed.returncode, 0, completed.stdout + completed.stderr)
            print(completed.stdout.strip())


if __name__ == "__main__":
    unittest.main()
