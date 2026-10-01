"""Exercise actual boss publication and Retail generation without changing runtime files."""
import copy
import importlib.util
import json
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch


ROOT = Path(__file__).resolve().parents[2]


class BossDamageReferenceHpTests(unittest.TestCase):
    def test_retail_regeneration_keeps_actual_hp_separate_from_damage_basis(self):
        path = ROOT / "Tools/GameplayPipeline/build_retail_balance_profile.py"
        spec = importlib.util.spec_from_file_location("retail_profile", path)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        bosses = json.loads((ROOT / "Data/Balance/Profiles/Retail.balanceprofile.json").read_text("utf-8-sig"))["bosses"]
        fixtures = {row["sourceNpcId"]: {
            "npcId": row["sourceNpcId"],
            "maximumHp": row.get("damageReferenceHp", row["maximumHp"]),
            "attackPower": row["attackPower"] * 10,
            "maximumHealthBars": row["maximumHealthBars"],
            "staggerGaugeMaximum": row["staggerGaugeMaximum"],
        } for row in bosses}
        with patch.object(module, "resolve_npc", side_effect=lambda balances, stats, key: fixtures[key]):
            generated = module.build_bosses(None, None, bosses)
        self.assertEqual(generated, bosses)
        valtan = next(row for row in generated if row["archetypeId"] == "BOSS_VALTAN")
        self.assertEqual((valtan["maximumHp"], valtan["damageReferenceHp"], valtan["maximumHealthBars"]),
                         (2100000000, 741285439, 160))
        self.assertTrue(all("damageReferenceHp" not in row for row in generated if row is not valtan))

    @unittest.skipUnless(shutil.which("powershell"), "PowerShell is required")
    def test_actual_boss_publisher_optional_trailing_field_and_invalid_values(self):
        boss = json.loads((ROOT / "Data/Balance/BossProfiles.json").read_text("utf-8-sig"))["bosses"][0]
        profile = json.loads((ROOT / "Data/Balance/Profiles/Retail.balanceprofile.json").read_text("utf-8-sig"))["bosses"][0]
        cases = [{"name": "legacy", "boss": boss, "profile": None, "accept": True, "count": 11, "hp": boss["maximumHp"]},
                 {"name": "retail-valtan", "boss": boss, "profile": profile, "accept": True,
                  "count": 12, "hp": 2100000000, "reference": 741285439}]
        for location in ("boss", "profile"):
            for name, value in (("zero", 0), ("negative", -1), ("overflow", 4294967296),
                                ("float", 1.5), ("bool", True), ("string", "3"), ("null", None)):
                case = copy.deepcopy(cases[1])
                case.update(name=f"{location}-{name}", accept=False)
                if location == "boss":
                    case["profile"] = None
                case[location]["damageReferenceHp"] = value
                cases.append(case)
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            (directory / "cases.json").write_text(json.dumps(cases), "utf-8")
            script = directory / "run.ps1"
            script.write_text(r'''
param([string]$Root, [string]$Cases)
$ErrorActionPreference = 'Stop'
. (Join-Path $Root 'Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1')
$source = [IO.File]::ReadAllText((Join-Path $Root 'Tools/GameplayPipeline/Publish-GameplayBalance.ps1'))
$start = $source.IndexOf("`$bossDocument = Read-JsonDocument 'Data/Balance/BossProfiles.json'")
$end = $source.IndexOf('$koukuBossProfiles =', $start)
if ($start -lt 0 -or $end -lt 0) { throw 'Actual boss publication block is missing.' }
$publication = [scriptblock]::Create($source.Substring($start, $end - $start))
function Read-JsonDocument([string]$Path) {
    return [pscustomobject]@{ schema='lostark.boss-profiles'; formatVersion=4; bosses=@($script:inputBoss) }
}
foreach ($case in (Get-Content -LiteralPath $Cases -Raw -Encoding UTF8 | ConvertFrom-Json)) {
    $script:inputBoss = $case.boss
    $balanceProfileBosses = @{}
    if ($null -ne $case.profile) { $balanceProfileBosses[[string]$case.profile.archetypeId] = $case.profile }
    $accepted = $true
    try { . $publication } catch { $accepted = $false }
    if ($accepted -ne $case.accept) { throw "Unexpected admission: $($case.name), accepted=$accepted" }
    if ($accepted) {
        $row = @($bossRows | Where-Object { $_.StartsWith("BOSS`t") })[0].Split("`t")
        if ($row.Count -ne $case.count -or [uint32]$row[3] -ne $case.hp -or [uint32]$row[4] -ne 160) {
            throw "Invalid row: $($case.name)"
        }
        if ($row.Count -eq 12 -and [uint32]$row[11] -ne $case.reference) { throw 'Damage basis was not emitted last.' }
    }
}
Write-Output 'PASS 16 actual publisher cases: legacy, Retail, invalid optional values'
''', "utf-8")
            result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(script),
                                     "-Root", str(ROOT), "-Cases", str(directory / "cases.json")],
                                    capture_output=True, text=True, errors="replace", timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            print(result.stdout.strip())


if __name__ == "__main__":
    unittest.main()
