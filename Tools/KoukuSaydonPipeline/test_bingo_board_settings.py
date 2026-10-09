"""Bingo authoring settings through the shared publication/draft row writer."""
import copy
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

from Tools.KoukuSaydonPipeline import project_kouku_saydon_composition as subject


ROOT = Path(__file__).resolve().parents[2]


class BingoBoardSettingsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = subject.load_json(ROOT / subject.SOURCE_PATH)

    def fixture(self):
        document = copy.deepcopy(self.source)
        identity = "KAKULSAYDON_G1_PATTERN_501"
        logic = dict(logicId="kakulsaydon.g1.logic.1", displayName="Fixture board",
                     logicType="DURATION", judgementKind="BINGO_BOARD")
        box = dict(occurrenceId=identity + ".logic.1", logicId=logic["logicId"],
                   startMs=0, durationMs=34, enabled=True,
                   onSuccessLogicIds=[], onFailLogicIds=[], onTimeoutLogicIds=[])
        pattern = dict(patternId=identity, actorProfileId="MN_RPCT_05", gateId="BINGO",
                       targetBossPlacementId="boss.kakulsaydon.bingo.saydon", durationMs=34,
                       displayName="Fixture board control", authoringStatus="PRODUCT", category="MECHANIC",
                       nextStageOrdinal=2, nextAnimationOrdinal=1, nextLogicOccurrenceOrdinal=2,
                       stages=[dict(stageId="STAGE_1", actionId=identity + ".stage.1", stageKind="ACTIVE",
                                    durationMs=34, animationOccurrences=[])], logicOccurrences=[box],
                       summonOccurrences=[], worldOccurrences=[], sceneProfileOccurrences=[],
                       presentationOccurrences=[], resetBossToSpawn=False,
                       animationRootVerticalScale=0, animationRootHorizontalScale=0)
        document["patterns"] = [pattern]
        document["logics"] = [logic]
        document["nextLogicOrdinal"] = 2
        document["playAllPatternIds"] = [pattern["patternId"]]
        document["folders"], document["bundles"] = [], []
        document.pop("patternFlows", None)
        return document, pattern, box, logic

    def test_legacy_defaults_and_explicit_window_project_without_mutation(self):
        document, pattern, box, logic = self.fixture()
        before = copy.deepcopy(document)
        encounter = subject.project_encounter(document)
        trigger = encounter["patterns"][0]["mechanicTriggers"][0]
        self.assertEqual(subject.BINGO_BOARD_DEFAULTS,
                         {key: trigger[key] for key in subject.BINGO_BOARD_DEFAULTS})
        self.assertTrue(trigger["bingoControlOnly"])
        self.assertEqual((0, 34), (trigger["startMs"], trigger["durationMs"]))
        self.assertEqual(2, len(trigger["hammerHalfExtentsM"]))
        self.assertEqual([], encounter["patterns"][0]["logicWindows"])
        self.assertEqual(before, document)

        pattern["patternId"] = "KAKULSAYDON_G1_PATTERN_502"
        document["playAllPatternIds"] = [pattern["patternId"]]
        box["occurrenceId"] = pattern["patternId"] + ".logic.1"
        pattern["stages"][0]["actionId"] = pattern["patternId"] + ".stage.1"
        pattern["durationMs"] = 90000
        box.update(startMs=1500, durationMs=80000)
        settings = dict(bingoActiveMode="WINDOW", bingoFirstBombDelayMs=0,
                        bingoBombIntervalMs=1000, bingoBombMarkMs=500,
                        bingoBombDropDelayMs=250, bingoBombFuseMs=250,
                        bingoInitialMarkedCells=25)
        logic.update(settings)
        _, validated = subject._validate_logic_definition(logic, "Bingo fixture", document["nextLogicOrdinal"])
        self.assertEqual(settings, {key: validated[key] for key in settings})
        before = copy.deepcopy(document)
        trigger = subject.project_encounter(document)["patterns"][0]["mechanicTriggers"][0]
        self.assertEqual(settings, {key: trigger[key] for key in settings})
        self.assertEqual((1500, 80000), (trigger["startMs"], trigger["durationMs"]))
        self.assertTrue(trigger["bingoControlOnly"])
        self.assertEqual(before, document)
        box["enabled"] = False
        self.assertEqual([], subject.project_encounter(document)["patterns"][0]["mechanicTriggers"])

    def test_invalid_definition_values_and_capacity_are_rejected(self):
        document, _, _, logic = self.fixture()
        cases = [("bingoActiveMode", value) for value in (None, True, "window", "OTHER")]
        for key in subject.BINGO_BOARD_DEFAULTS:
            if key == "bingoActiveMode":
                continue
            cases.extend((key, value) for value in (None, True, 1.5, "1000", -1, 600001))
        cases += [("bingoBombIntervalMs", 0), ("bingoBombMarkMs", 0),
                  ("bingoBombDropDelayMs", 0), ("bingoBombFuseMs", 249),
                  ("bingoBombFuseMs", 80001), ("bingoInitialMarkedCells", 26),
                  ("bingoBombIntervalMs", 2999)]
        for field, value in cases:
            with self.subTest(field=field, value=value):
                invalid = dict(logic, **{field: value})
                before = copy.deepcopy(invalid)
                with self.assertRaises(subject.CompositionError):
                    subject._validate_logic_definition(invalid, "Bingo fixture", document["nextLogicOrdinal"])
                self.assertEqual(before, invalid)
        for settings in (dict(bingoFirstBombDelayMs=0, bingoInitialMarkedCells=0),
                         dict(bingoBombIntervalMs=3000),
                         dict(bingoBombIntervalMs=600000, bingoBombMarkMs=600000,
                              bingoBombDropDelayMs=600000, bingoBombFuseMs=80000)):
            subject._validate_logic_definition(dict(logic, **settings), "Bingo fixture", document["nextLogicOrdinal"])
        with self.assertRaises(subject.CompositionError):
            subject._validate_logic_definition(dict(logic, judgementKind="AREA_OVERLAP", bingoBombFuseMs=4000),
                                               "wrong owner", document["nextLogicOrdinal"])
        with self.assertRaisesRegex(subject.CompositionError, "30 Hz"):
            subject._validate_logic_definition(dict(logic, bingoBombIntervalMs=100, bingoBombMarkMs=1,
                                               bingoBombDropDelayMs=1, bingoBombFuseMs=398),
                                               "rounded capacity", document["nextLogicOrdinal"])

    def test_disabled_long_window_projects_short_empty_carriers_and_reenables_without_source_changes(self):
        document, pattern, box, logic = self.fixture()
        pattern["durationMs"] = 90000
        box.update(startMs=1500, durationMs=80000, enabled=False)
        logic["bingoActiveMode"] = "WINDOW"
        before = copy.deepcopy(document)
        encounter = subject.project_encounter(document)["patterns"][0]
        presentation = subject.project_presentation(document)["patterns"][0]
        self.assertEqual(34, sum(row["durationMs"] for row in encounter["stages"]))
        self.assertNotIn("timelineDurationMs", encounter)
        self.assertEqual([], encounter["mechanicTriggers"])
        self.assertEqual(34, presentation["durationMs"])
        self.assertEqual([], presentation["presentationOccurrences"])
        self.assertEqual(before, document)
        box["enabled"] = True
        enabled = copy.deepcopy(document)
        encounter = subject.project_encounter(document)["patterns"][0]
        trigger = encounter["mechanicTriggers"][0]
        self.assertEqual((1500, 80000), (trigger["startMs"], trigger["durationMs"]))
        self.assertTrue(trigger["bingoControlOnly"])
        self.assertEqual(90000, subject.project_presentation(document)["patterns"][0]["durationMs"])
        self.assertEqual(enabled, document)

    def test_control_marker_requires_an_independent_empty_actor_timeline(self):
        document, pattern, _, _ = self.fixture()
        logics = {row["logicId"]: row for row in document["logics"]}
        self.assertTrue(subject._is_bingo_control_pattern(pattern, logics))
        mutations = [lambda p: p.update(gateId="GATE3"),
                     lambda p: p.update(patternOccurrences=[{}]),
                     lambda p: p.update(bossMotion={"keys": [{}]}),
                     lambda p: p.update(resetBossToSpawn=True),
                     lambda p: p.update(resetBossYawDegrees=0),
                     lambda p: p.update(enterCombatOnFinish=True),
                     lambda p: p["logicOccurrences"].append(copy.deepcopy(p["logicOccurrences"][0])),
                     lambda p: p["stages"][0]["animationOccurrences"].append({"runtimeClip": "raw.idle"})]
        mutations.append(lambda p: p["stages"][0]["animationOccurrences"].append({"enabled": False}))
        mutations += [lambda p, lane=lane: p[lane].append({}) for lane in
                      ("summonOccurrences", "worldOccurrences", "sceneProfileOccurrences", "presentationOccurrences")]
        for mutation in mutations:
            candidate = copy.deepcopy(pattern)
            mutation(candidate)
            self.assertFalse(subject._is_bingo_control_pattern(candidate, logics))
        candidate = copy.deepcopy(pattern)
        candidate["logicOccurrences"][0]["enabled"] = False
        self.assertFalse(subject._is_bingo_control_pattern(candidate, logics))
        self.assertTrue(subject._is_bingo_control_pattern(candidate, logics, include_disabled=True))
        candidate["logicOccurrences"].append(dict(logicId="other.logic", enabled=True))
        self.assertFalse(subject._is_bingo_control_pattern(candidate, logics, include_disabled=True))
        logics[pattern["logicOccurrences"][0]["logicId"]]["logicType"] = "TRIGGER"
        self.assertFalse(subject._is_bingo_control_pattern(pattern, logics))

    def test_shared_bootstrap_writer_preserves_settings_and_rejects_malformed_rows(self):
        document, source_pattern, box, logic = self.fixture()
        baseline = subject.project_encounter(document)
        cases = [{"name": "defaults", "valid": True, "document": baseline}]

        def changed(name, valid, change):
            case = copy.deepcopy(cases[0])
            case.update(name=name, valid=valid)
            change(case["document"]["patterns"][0])
            cases.append(case)

        owner = lambda p: p["mechanicTriggers"][0]
        changed("legacy fields absent", True,
                lambda p: [owner(p).pop(key) for key in (*subject.BINGO_BOARD_DEFAULTS, "bingoControlOnly")])
        changed("window", True, lambda p: (p.update(timelineDurationMs=90000),
                owner(p).update(bingoActiveMode="WINDOW", startMs=1500, durationMs=80000,
                                bingoFirstBombDelayMs=0, bingoBombIntervalMs=1000, bingoBombMarkMs=500,
                                bingoBombDropDelayMs=250, bingoBombFuseMs=250, bingoInitialMarkedCells=25)))
        for field, value in (("bingoActiveMode", "bad"), ("bingoBombFuseMs", 249),
                             ("bingoBombFuseMs", 80001), ("bingoBombDropDelayMs", 0),
                             ("bingoInitialMarkedCells", 26), ("bingoBombIntervalMs", 2999),
                             ("bingoFirstBombDelayMs", None), ("bingoBombMarkMs", True),
                             ("bingoBombIntervalMs", 20000.5), ("bingoControlOnly", "true")):
            changed(field + repr(value), False, lambda p, f=field, v=value: owner(p).update({f: v}))
        changed("unknown field", False, lambda p: owner(p).update(bingoTypo=1))
        changed("rounded capacity", False, lambda p: owner(p).update(bingoBombIntervalMs=100,
                bingoBombMarkMs=1, bingoBombDropDelayMs=1, bingoBombFuseMs=398))
        changed("duplicate owner", False, lambda p: p["mechanicTriggers"].append(copy.deepcopy(owner(p))))
        changed("wrong owner", False, lambda p: (owner(p).update(kind="BINGO_DETONATION"), owner(p).pop("hammerHalfExtentsM")))
        changed("control motion", False, lambda p: p.update(resetBossToSpawn=True))
        changed("control wrong gate", False, lambda p: p.update(gateId="GATE3"))
        changed("control attack", False, lambda p: p["stages"][0].update(hitCount=1))
        changed("control root motion", False, lambda p: p["stages"][0].update(rootMotionSamples=[]))
        source_pattern["durationMs"] = 90000
        box.update(startMs=1500, durationMs=80000)
        box["enabled"] = False
        cases.append({"name": "disabled board", "valid": True, "document": subject.project_encounter(document)})

        script = r"""
param([string]$Root)
$ErrorActionPreference='Stop'
. (Join-Path $Root 'Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1')
[Threading.Thread]::CurrentThread.CurrentCulture = [Globalization.CultureInfo]::GetCultureInfo('de-DE')
$bosses = Get-Content -Raw -Encoding UTF8 (Join-Path $Root 'Data/Balance/BossProfiles.json') | ConvertFrom-Json
$cases = Get-Content -Raw -Encoding UTF8 (Join-Path $PSScriptRoot 'cases.json') | ConvertFrom-Json
$results = [Collections.Generic.List[object]]::new()
foreach ($case in $cases) {
    $rows = [Collections.Generic.List[string]]::new()
    try {
        Add-KoukuBootstrapRows -Encounter $case.document -BossProfiles $bosses -Rows $rows
        $sorted = @($rows | Sort-Object { Get-BootstrapRowSortKey $_ })
        $results.Add([pscustomobject]@{name=$case.name;accepted=$true;rows=$sorted})
    } catch { $results.Add([pscustomobject]@{name=$case.name;accepted=$false;error=$_.Exception.Message}) }
}
ConvertTo-Json -InputObject $results.ToArray() -Depth 10 -Compress
"""
        with tempfile.TemporaryDirectory(prefix="bingo-settings-", dir=ROOT / "out") as temporary:
            path = Path(temporary)
            (path / "cases.json").write_text(json.dumps(cases), encoding="utf-8")
            (path / "run.ps1").write_text(script, encoding="utf-8-sig")
            result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass",
                                     "-File", str(path / "run.ps1"), "-Root", str(ROOT)],
                                    capture_output=True, text=True, timeout=60)
        self.assertEqual(0, result.returncode, result.stderr)
        output = json.loads(result.stdout)
        self.assertEqual(len(cases), len(output))
        for source, observed in zip(cases, output):
            self.assertEqual(source["valid"], observed["accepted"], observed)
        for observed in output[:3]:
            rows = observed["rows"]
            base = next(i for i, row in enumerate(rows) if row.startswith("PATTERNMECHANICTRIGGER\t"))
            settings = next(i for i, row in enumerate(rows) if row.startswith("PATTERNBINGOBOARD\t"))
            self.assertLess(base, settings)
            self.assertEqual(11, len(rows[settings].split("\t")))
            controls = [i for i, row in enumerate(rows) if row.startswith("PATTERNBINGOCONTROL\t")]
            self.assertEqual(0 if observed["name"] == "legacy fields absent" else 1, len(controls))
            for control in controls:
                self.assertLess(base, control)
                self.assertEqual(4, len(rows[control].split("\t")))
            self.assertEqual(1, sum(row.startswith("PATTERNBINGOHAMMER\t") for row in rows))
            expected = ["WINDOW", "0", "1000", "500", "250", "250", "25"] if observed["name"] == "window" else ["ENCOUNTER", "30000", "20000", "6000", "2000", "4000", "2"]
            self.assertEqual(expected, rows[settings].split("\t")[4:])
        self.assertFalse(any(row.startswith("PATTERNBINGO") for row in output[-1]["rows"]))


if __name__ == "__main__":
    unittest.main()
