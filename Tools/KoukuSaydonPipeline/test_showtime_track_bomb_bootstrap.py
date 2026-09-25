"""Focused tests of the canonical PowerShell tracking-bomb writer."""
import copy
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class TrackBombBootstrapTests(unittest.TestCase):
    def test_exact_join_rejects_bad_owner_geometry_and_visuals(self):
        body_id = "kouku.showtime.fixed." + "1" * 64
        blast_id = "kouku.showtime.fixed." + "2" * 64
        baseline = {
            "name": "valid", "valid": True,
            "pattern": {"patternId": "pattern.test", "logicWindows": [],
                        "mechanicTriggers": [{"triggerId": "track.test", "kind": "BOSS_TRACK_TARGET",
                                              "startMs": 500, "durationMs": 1000}],
                        "trackBombs": [{"occurrenceId": "track.test", "bodyStartMs": 100,
                                        "bodyVisualId": body_id, "explosionVisualId": blast_id,
                                        "explosionLifetimeMs": 3300, "position": [-5.47, 1.32, 942.33],
                                        "fan": {"positionOffset": [0, 0, 0], "yawDegrees": 90,
                                                "radiusXM": 24.2, "radiusZM": 13.75,
                                                "halfAngleDegrees": 22.5}}]},
            "visuals": [{"clientVisualId": body_id, "combatObjectArchetypeId": "combatobject.kouku.showtime.fixed",
                         "loop": False, "durationMs": 1400},
                        {"clientVisualId": blast_id, "combatObjectArchetypeId": "combatobject.kouku.showtime.fixed",
                         "loop": False, "durationMs": 3300}],
        }
        cases = [baseline]

        def bad(name, change):
            case = copy.deepcopy(baseline)
            case.update(name=name, valid=False)
            change(case)
            cases.append(case)

        bomb = lambda c: c["pattern"]["trackBombs"][0]
        owner = lambda c: c["pattern"]["mechanicTriggers"][0]
        bad("duplicate bomb", lambda c: c["pattern"]["trackBombs"].append(copy.deepcopy(bomb(c))))
        bad("missing owner", lambda c: c["pattern"].update(mechanicTriggers=[]))
        bad("ambiguous owner", lambda c: c["pattern"]["mechanicTriggers"].append(copy.deepcopy(owner(c))))
        bad("different mechanic", lambda c: owner(c).update(kind="BOSS_RANDOM_TARGET"))
        bad("logic owner collision", lambda c: c["pattern"]["logicWindows"].append({"windowId": "track.test"}))
        bad("late body", lambda c: bomb(c).update(bodyStartMs=1500))
        bad("fractional birth", lambda c: bomb(c).update(bodyStartMs=100.5))
        bad("owner exceeds pattern", lambda c: owner(c).update(durationMs=600000))
        bad("same two visuals", lambda c: bomb(c).update(explosionVisualId=body_id))
        bad("missing visual", lambda c: c.update(visuals=c["visuals"][:1]))
        bad("wrong visual role", lambda c: c["visuals"][0].update(combatObjectArchetypeId="combatobject.kouku.showtime.tracking"))
        bad("looping body", lambda c: c["visuals"][0].update(loop=True))
        bad("stale body lifetime", lambda c: c["visuals"][0].update(durationMs=1399))
        bad("stale explosion lifetime", lambda c: c["visuals"][1].update(durationMs=3299))
        bad("short position", lambda c: bomb(c).update(position=[0, 0]))
        bad("Boolean coordinate", lambda c: bomb(c).update(position=[True, 0, 0]))
        bad("position overflow", lambda c: bomb(c).update(position=[100001, 0, 0]))
        bad("zero radius", lambda c: bomb(c)["fan"].update(radiusXM=0))
        bad("negative radius", lambda c: bomb(c)["fan"].update(radiusZM=-1))
        bad("radius overflow", lambda c: bomb(c)["fan"].update(radiusXM=100001))
        bad("angle overflow", lambda c: bomb(c)["fan"].update(halfAngleDegrees=181))
        bad("full angle forbidden", lambda c: bomb(c)["fan"].update(halfAngleDegrees=180))
        bad("zero angle", lambda c: bomb(c)["fan"].update(halfAngleDegrees=0))
        bad("infinite geometry", lambda c: c.update(specialMutation="infinity"))
        bad("NaN geometry", lambda c: c.update(specialMutation="nan"))
        bad("wrong outer field", lambda c: bomb(c).update(typo=1))
        bad("wrong fan field", lambda c: bomb(c)["fan"].update(typo=1))
        bad("not array", lambda c: c["pattern"].update(trackBombs={}))
        bad("unbounded count", lambda c: c["pattern"].update(trackBombs=[copy.deepcopy(bomb(c))] * 65))

        bad("instant trigger owner", lambda c: (owner(c).update(durationMs=34), c["visuals"][0].update(durationMs=434)))
        bad("fan positive offset overflow", lambda c: bomb(c)["fan"].update(positionOffset=[1000.001, 0, 0]))
        bad("fan negative offset overflow", lambda c: bomb(c)["fan"].update(positionOffset=[0, -1000.001, 0]))
        bad("fan finite axis overflow", lambda c: bomb(c)["fan"].update(radiusXM=1000.001))
        bad("positive yaw overflow", lambda c: bomb(c)["fan"].update(yawDegrees=360.001))
        bad("negative yaw overflow", lambda c: bomb(c)["fan"].update(yawDegrees=-360.001))
        edge = copy.deepcopy(baseline)
        edge.update(name="server accepted bounds")
        owner(edge).update(durationMs=35)
        edge["visuals"][0].update(durationMs=435)
        bomb(edge).update(position=[-100000, 100000, 0])
        bomb(edge)["fan"].update(positionOffset=[-1000, 1000, 0], radiusXM=1000, radiusZM=1000,
                                 yawDegrees=-360, halfAngleDegrees=179.999)
        cases.append(edge)

        script = r"""
param([string]$Root)
$ErrorActionPreference='Stop'
. (Join-Path $Root 'Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1')
[Threading.Thread]::CurrentThread.CurrentCulture = [Globalization.CultureInfo]::GetCultureInfo('de-DE')
$cases = Get-Content -Raw -Encoding UTF8 (Join-Path $PSScriptRoot 'cases.json') | ConvertFrom-Json
$results = [Collections.Generic.List[object]]::new()
foreach ($case in $cases) {
    $visuals = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    foreach ($visual in $case.visuals) { $visuals.Add($visual.clientVisualId,$visual) }
    if ($case.specialMutation -ceq 'infinity') { $case.pattern.trackBombs[0].fan.radiusXM = [double]::PositiveInfinity }
    if ($case.specialMutation -ceq 'nan') { $case.pattern.trackBombs[0].fan.yawDegrees = [double]::NaN }
    try {
        $rows = @(New-KoukuTrackBombRows $case.pattern 'encounter.test' 6000 $visuals)
        $results.Add([pscustomobject]@{name=$case.name;accepted=$true;rows=$rows})
    } catch {
        $results.Add([pscustomobject]@{name=$case.name;accepted=$false;error=$_.Exception.Message})
    }
}
$owner = "PATTERNMECHANICTRIGGER`tencounter.test`tpattern.test`ttrack.test`tBOSS_TRACK_TARGET`t500`t1000"
$child = $results[0].rows[0]
$sorted = @($child,$owner) | Sort-Object { Get-BootstrapRowSortKey $_ }
$actorAllowed = Test-KoukuActorMechanicTriggers $cases[0].pattern
ConvertTo-Json -InputObject @{cases=$results.ToArray();sort=$sorted;actorAllowed=$actorAllowed} -Depth 10 -Compress
"""
        with tempfile.TemporaryDirectory(prefix="track-bomb-", dir=ROOT / "out") as temporary:
            path = Path(temporary)
            (path / "cases.json").write_text(json.dumps(cases), encoding="utf-8")
            (path / "run.ps1").write_text(script, encoding="utf-8-sig")
            result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass",
                                     "-File", str(path / "run.ps1"), "-Root", str(ROOT)],
                                    capture_output=True, text=True, timeout=30)
        self.assertEqual(0, result.returncode, result.stderr)
        output = json.loads(result.stdout)
        self.assertEqual(len(cases), len(output["cases"]))
        for source, observed in zip(cases, output["cases"]):
            self.assertEqual(source["valid"], observed["accepted"], observed)
        fields = output["cases"][0]["rows"][0].split("\t")
        self.assertEqual(18, len(fields))
        self.assertEqual(["PATTERNTRACKBOMB", "encounter.test", "pattern.test", "track.test", "100",
                          body_id, blast_id, "3300", "-5.47", "1.32", "942.33", "0", "0", "0",
                          "90", "24.2", "13.75", "22.5"], fields)
        self.assertTrue(output["sort"][0].startswith("PATTERNMECHANICTRIGGER\t"))
        self.assertTrue(output["sort"][1].startswith("PATTERNTRACKBOMB\t"))
        self.assertFalse(output["actorAllowed"])


if __name__ == "__main__":
    unittest.main()
