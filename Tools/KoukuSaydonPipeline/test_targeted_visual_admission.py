"""Run the owner publisher's actual PowerShell targeted-visual admission."""

import copy
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


def visual():
    return {"clientVisualId": "kouku.showtime.fixed." + "a" * 64,
            "combatObjectArchetypeId": "combatobject.kouku.showtime.fixed",
            "durationMs": 10, "loop": False, "resources": [{}], "occurrences": [{}],
            "selectedFlightMs": 5, "selectedFlightArcHeightM": 2.5,
            "selectedFlightSourceOffset": [.2, 1.0990578, 0]}


@unittest.skipUnless(shutil.which("powershell"), "owner validator requires PowerShell")
class TargetedVisualAdmissionTests(unittest.TestCase):
    def run_cases(self, cases):
        source = (ROOT / "Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1").read_text(encoding="utf-8-sig")
        names = ("Assert-ExactProperties", "Assert-StableId", "Assert-JsonString",
                 "Assert-JsonInteger", "Assert-JsonNumber", "Get-KoukuTargetedVisualIndex")
        definitions = [re.search(r"(?ms)^function " + name + r"\b.*?^\}", source).group(0) for name in names]
        script = "$ErrorActionPreference='Stop'\n$stableIdPattern='^[A-Za-z0-9_.-]{1,128}$'\n" + "\n".join(definitions)
        script += """
$cases=Get-Content -Raw -Encoding UTF8 (Join-Path $PSScriptRoot 'cases.json') | ConvertFrom-Json
$results=@(foreach ($case in $cases) {
    $value=$case.visual
    switch ($case.inject) {
        'arcNan' { $value.selectedFlightArcHeightM=[double]::NaN }
        'arcInfinity' { $value.selectedFlightArcHeightM=[double]::PositiveInfinity }
        'offsetNan' { $value.selectedFlightSourceOffset[1]=[double]::NaN }
        'offsetInfinity' { $value.selectedFlightSourceOffset[1]=[double]::NegativeInfinity }
    }
    $bindings=[pscustomobject]@{schema='lostark.kouku-saydon-pattern-bindings';formatVersion=1;
        sourceRevision=2349;targetedCombatVisuals=@($value)}
    try { $index=Get-KoukuTargetedVisualIndex $bindings 2349; [pscustomobject]@{accepted=$true;count=$index.Count} }
    catch { [pscustomobject]@{accepted=$false;error=$_.Exception.Message} }
})
ConvertTo-Json -InputObject $results -Compress
"""
        with tempfile.TemporaryDirectory() as directory:
            folder = Path(directory)
            (folder / "cases.json").write_text(json.dumps(cases), encoding="utf-8")
            path = folder / "validate.ps1"
            path.write_text(script, encoding="utf-8-sig")
            result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(path)],
                                    capture_output=True, text=True, timeout=30)
        self.assertEqual(0, result.returncode, result.stderr)
        outcomes = json.loads(result.stdout)
        self.assertEqual(len(cases), len(outcomes))
        for case, outcome in zip(cases, outcomes):
            with self.subTest(case=case):
                self.assertEqual(case["accepted"], outcome["accepted"], outcome)
                if case["accepted"]:
                    self.assertEqual(1, outcome["count"])

    def test_finite_flight_boundaries_and_legacy_groups_are_accepted(self):
        rows = [visual(), dict(visual(), selectedFlightMs=1, selectedFlightArcHeightM=0,
                               selectedFlightSourceOffset=[-1000, 0, 1000]),
                dict(visual(), selectedFlightMs=9, selectedFlightArcHeightM=1000)]
        legacy = {key: value for key, value in visual().items() if not key.startswith("selectedFlight")}
        rows.append(legacy)
        rows.append(dict(legacy, clientVisualId="kouku.showtime.tracking." + "b" * 64,
                         combatObjectArchetypeId="combatobject.kouku.showtime.tracking", loop=True))
        self.run_cases([{"visual": row, "accepted": True} for row in rows])

    def test_partial_fields_wrong_types_bounds_and_tracking_are_rejected(self):
        rows = []
        for field in ("selectedFlightMs", "selectedFlightArcHeightM", "selectedFlightSourceOffset"):
            row = visual()
            row.pop(field)
            rows.append(row)
        invalid = {"selectedFlightMs": [None, True, "5", 1.5, 0, -1, 10, 11],
                   "selectedFlightArcHeightM": [None, True, "2.5", -0.01, 1000.01],
                   "selectedFlightSourceOffset": [None, 1, [0, 0], [0, 0, 0, 0], [True, 0, 0],
                                                   ["0", 0, 0], [0, -1000.01, 0], [0, 0, 1000.01]]}
        for field, values in invalid.items():
            for value in values:
                rows.append(dict(visual(), **{field: value}))
        rows.append(dict(visual(), clientVisualId="kouku.showtime.tracking." + "b" * 64,
                         combatObjectArchetypeId="combatobject.kouku.showtime.tracking", loop=True))
        rows.append(dict(visual(), unexpectedFlightField=1))
        rows.append(dict(visual(), durationMs=1, selectedFlightMs=1))
        self.run_cases([{"visual": row, "accepted": False} for row in rows])

    def test_nonfinite_arc_and_source_coordinates_are_rejected(self):
        self.run_cases([{"visual": copy.deepcopy(visual()), "accepted": False, "inject": inject}
                        for inject in ("arcNan", "arcInfinity", "offsetNan", "offsetInfinity")])


if __name__ == "__main__":
    unittest.main()
