import copy
import json
import math
import re
import subprocess
import tempfile
from pathlib import Path
import unittest

from Tools.KoukuSaydonPipeline import project_kouku_saydon_composition as subject
from Tools.KoukuSaydonPipeline import test_project_kouku_saydon_composition as fixtures


class ResultTuningContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        fixtures.KoukuSaydonCompositionProjectionTests.setUpClass()
        cls.fixture = fixtures.KoukuSaydonCompositionProjectionTests()

    def validate_logic(self, **values):
        row = dict(logicId="kakulsaydon.g1.logic.1", displayName="Tuning", **values)
        return subject._validate_logic_definition(row, "test", 2)[1]

    def test_soldier_defaults_counts_fractional_radius_and_bounds(self):
        defaults = dict(logicType="TRIGGER", triggerKind="CARD_RAIN_SOLDIERS")
        result = self.validate_logic(**defaults)
        self.assertEqual(([1, 1, 1], 3.0, 6.0), tuple(result[k] for k in
                         ("soldierCounts", "spawnRadiusMinM", "spawnRadiusMaxM")))
        values = dict(defaults, soldierCounts=[2, 0, 5], spawnRadiusMinM=2.125, spawnRadiusMaxM=6.375)
        result = self.validate_logic(**values)
        self.assertEqual(([2, 0, 5], 2.125, 6.375), tuple(result[k] for k in
                         ("soldierCounts", "spawnRadiusMinM", "spawnRadiusMaxM")))
        for change in (dict(soldierCounts=[0, 0, 0]), dict(soldierCounts=[32, 32, 1]),
                       dict(soldierCounts=[1, True, 1]), dict(soldierCounts=[1, 1]),
                       dict(spawnRadiusMinM=-1), dict(spawnRadiusMaxM=1),
                       dict(spawnRadiusMaxM=math.nan), dict(spawnRadiusMaxM=101),
                       dict(triggerKind="BINGO_DETONATION")):
            with self.subTest(change=change), self.assertRaises(subject.CompositionError):
                self.validate_logic(**dict(values, **change))

    def test_duration_pulse_projects_result_values_and_preserves_input(self):
        document = self.fixture.reenter_damage_document()
        duration, result = document["logics"][-2:]
        duration.pop("triggerKind")
        duration.pop("rearmOnExit")
        duration.update(logicType="DURATION", judgementKind="AREA_OVERLAP", repeatIntervalMs=100)
        result.update(outcomeKind="FIXED_DAMAGE", damageAmount=100, percent=0,
                      pushRangeM=3.125, pushMs=350)
        before = copy.deepcopy(document)
        self.fixture.validate(document)
        window = self.fixture.first_product(subject.project_encounter(document))["logicWindows"][0]
        self.assertEqual(("AREA_OVERLAP", 100), (window["kind"], window["repeatIntervalMs"]))
        damage = window["onSuccess"][0]
        self.assertEqual(("FIXED_DAMAGE", 100, 3.125, 350),
                         tuple(damage[k] for k in ("kind", "damageAmount", "pushRangeM", "pushMs")))
        self.assertEqual(before, document)
        for value in (-1, True, 1, 33, 600001, math.nan):
            with self.subTest(interval=value), self.assertRaises(subject.CompositionError):
                self.validate_logic(logicType="DURATION", judgementKind="AREA_OVERLAP", repeatIntervalMs=value)
        self.assertEqual(0, self.validate_logic(logicType="DURATION", judgementKind="AREA_OVERLAP")["repeatIntervalMs"])

    def test_card_rain_scale_and_radius_keep_fractional_values(self):
        document = self.fixture.random_volley_document()
        logic = document["logics"][0]
        logic.update(fixedSelectionGroupId="", trackingPresentationOccurrenceId="",
                     randomAnchorKind="BOSS", randomArenaRadiusM=12.125,
                     randomScaleMin=.125, randomScaleMax=.875)
        self.fixture.validate(document)
        rows, _, _ = subject._project_showtime_targets(document, self.fixture.first_product(document))
        self.assertEqual((12.125, .125, .875), tuple(rows[0][k] for k in
                         ("randomArenaRadiusM", "randomScaleMin", "randomScaleMax")))

    def test_cross_direction_child_accepts_only_contact_gameplay(self):
        document, parent, definition = self.fixture.cross_direction_document()
        child = next(row for row in document["patterns"] if row["patternId"] == definition["directionPatternIds"][0])
        contact = dict(logicId="test.contact", displayName="Contact", logicType="DURATION",
                       judgementKind="AREA_OVERLAP", repeatIntervalMs=100)
        result = dict(logicId="test.damage", displayName="Damage", logicType="RESULT",
                      outcomeKind="FIXED_DAMAGE", damageAmount=100, percent=0, durationMs=0)
        document["logics"].extend([contact, result])
        child["logicOccurrences"] = [dict(occurrenceId="test.contact.box", logicId=contact["logicId"],
            startMs=0, durationMs=100, onSuccessLogicIds=[result["logicId"]])]
        subject._validate_cross_direction(document, parent)
        result["outcomeKind"] = "INSTANT_DEATH"
        with self.assertRaises(subject.CompositionError):
            subject._validate_cross_direction(document, parent)
        result["outcomeKind"] = "FIXED_DAMAGE"
        contact["judgementKind"] = "BOSS_TRACK_TARGET"
        with self.assertRaises(subject.CompositionError):
            subject._validate_cross_direction(document, parent)

    def test_authored_madness_replaces_only_its_full_lifetime_world(self):
        world = dict(occurrenceId="world.1", startMs=100, durationMs=1000)
        document = dict(presentationResources=[dict(resourceId="collider", kind="COLLIDER")], logics=[
            dict(logicId="contact", judgementKind="AREA_OVERLAP", repeatIntervalMs=100),
            dict(logicId="gauge", outcomeKind="MADNESS_GAUGE_ADD_PERCENT", percent=2)])
        window = dict(occurrenceId="contact.1", logicId="contact", startMs=100, durationMs=1000,
                      onSuccessLogicIds=["gauge"])
        collider = dict(resourceId="collider", logicOccurrenceId="contact.1", anchorKind="WORLD",
                        worldOccurrenceId="world.1")
        pattern = dict(logicOccurrences=[window], presentationOccurrences=[collider])
        self.assertTrue(subject._world_has_authored_madness(document, pattern, world))
        window["durationMs"] = 999
        self.assertFalse(subject._world_has_authored_madness(document, pattern, world))
        window["durationMs"] = 1000
        collider["worldOccurrenceId"] = "world.other"
        self.assertFalse(subject._world_has_authored_madness(document, pattern, world))

    def test_world_bone_collider_uses_installed_mouth_pose_and_full_local_trs(self):
        root = subject.REPOSITORY_ROOT
        document = json.loads((root / subject.SOURCE_PATH).read_text(encoding="utf-8-sig"))
        sequences = subject.load_world_sequences(root, document["areaId"])
        world = next(row for row in document["worlds"] if row["worldId"] == "kakulsaydon.g1.world.22")
        box = dict(occurrenceId="test.world", worldId=world["worldId"], startMs=0, durationMs=17314,
                   playbackSpeed=1, placement=dict(position=[0,0,0], rotationDegrees=[0,0,0], scale=[1,1,1]))
        collider = dict(occurrenceId="test.collider", bone="b_mouth_f", boneRotation="BONE",
                        positionOffset=[.1,-4.57,0], rotationDegrees=[90,0,0], worldEmissionIndex=0)
        track = subject._project_world_bone_collider_track(root, sequences, world, box, collider)
        first = track["keys"][0]
        # Installed mouth's -Y flame axis points forward at birth; its raw +Z does not.
        self.assertGreater(first["positionOffset"][2], 7.0)
        self.assertLess(abs(math.degrees(2*math.atan2(first["rotationY"], first["rotationW"]))), 10.0)
        headings = [2*math.atan2(key["rotationY"], key["rotationW"]) for key in track["keys"]]
        self.assertGreater(max(headings) - min(headings), 5.0)
        self.assertTrue(all(math.isfinite(value) for key in track["keys"] for value in key["positionOffset"]))
        collider["bone"] = "missing.bone"
        with self.assertRaises(subject.CompositionError):
            subject._project_world_bone_collider_track(root, sequences, world, box, collider)

    def test_instant_facing_bingo_and_soldier_trigger_projection(self):
        document = self.fixture.reenter_damage_document()
        pattern = self.fixture.first_product(document)
        pattern["presentationOccurrences"] = []
        window = pattern["logicOccurrences"][0]
        window.pop("onSuccessLogicIds")
        definition = document["logics"][-2]
        definition.pop("rearmOnExit")
        for kind in ("BOSS_TRACK_TARGET", "BOSS_RANDOM_TARGET", "BINGO_DETONATION", "CARD_RAIN_SOLDIERS"):
            definition["triggerKind"] = kind
            self.fixture.validate(document)
            projected = self.fixture.first_product(subject.project_encounter(document))["mechanicTriggers"][0]
            self.assertEqual(kind, projected["kind"])
            self.assertEqual(34 if kind == "BOSS_TRACK_TARGET" else 1000, projected["durationMs"])
            if kind == "CARD_RAIN_SOLDIERS":
                self.assertEqual([1, 1, 1], projected["soldierCounts"])
                self.assertEqual((3.0, 6.0), (projected["spawnRadiusMinM"], projected["spawnRadiusMaxM"]))

    def test_bingo_completed_lines_and_timed_player_invulnerability(self):
        definition = self.validate_logic(logicType="DURATION", judgementKind="BINGO_COMPLETED_LINES")
        self.assertEqual(3, definition["threshold"])
        window = subject._project_logic_window(dict(occurrenceId="test", startMs=0, durationMs=32628),
            dict(judgementKind="BINGO_COMPLETED_LINES", threshold=3), {}, 0.0)
        self.assertEqual(("BINGO_COMPLETED_LINES", 3), (window["kind"], window["threshold"]))
        self.validate_logic(logicType="RESULT", outcomeKind="PLAYER_INVULNERABILITY", durationMs=30000)
        for threshold in (0, 11, True):
            with self.subTest(threshold=threshold), self.assertRaises(subject.CompositionError):
                self.validate_logic(logicType="DURATION", judgementKind="BINGO_COMPLETED_LINES", threshold=threshold)
        for duration in (0, 600001, True):
            with self.subTest(duration=duration), self.assertRaises(subject.CompositionError):
                self.validate_logic(logicType="RESULT", outcomeKind="PLAYER_INVULNERABILITY", durationMs=duration)


    def test_bootstrap_bone_yaw_preserves_scale_clock_and_unit_quaternion(self):
        publisher = (subject.REPOSITORY_ROOT / "Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1").read_text(encoding="utf-8-sig")
        definitions = [re.search(r"(?ms)^function " + name + r"\b.*?^\}", publisher).group(0)
                       for name in ("Assert-ExactProperties", "Assert-JsonInteger", "Assert-JsonNumber",
                                    "Format-InvariantFloat", "Format-InvariantSignedFloat", "Format-JsonSignedNumbers")]
        start = publisher.index("\t\t\tif ($hasWorldTrack) {\n\t\t\t\t$track =")
        end = publisher.index("\n\t\t}\n\t\tforeach ($slotName", start)
        script = "$ErrorActionPreference='Stop'\n" + "\n".join(definitions)
        script += "\n$region=Get-Content -Raw (Join-Path $PSScriptRoot 'region.json') | ConvertFrom-Json\n"
        script += "$hasWorldTrack=$true; $windowKind='AREA_OVERLAP'; $window=[pscustomobject]@{windowId='test';startMs=10;durationMs=100}\n"
        script += "$koukuPattern=[pscustomobject]@{patternId='test'}; $koukuEncounterDocument=[pscustomobject]@{encounterId='test'}\n"
        script += "$patternRows=[Collections.Generic.List[string]]::new()\n" + publisher[start:end]
        script += "\nConvertTo-Json -InputObject @($patternRows) -Compress\n"
        track = dict(startMs=10, startDelayMs=0, durationMs=100, playbackSpeed=1, interpolation="LINEAR",
                     baselinePosition=[0,0,0], baselineYawDegrees=0, baselineScale=[1,1,1], keys=[
                         dict(timeMs=t, positionOffset=[1,2,3], rotationY=math.sin(yaw/2),
                              rotationW=math.cos(yaw/2), scaleMultiplier=[1,1,1], visible=True)
                         for t,yaw in ((0,0.5),(100,1.5))])
        region = dict(regionId="native.bone", anchorKind="BOSS_CURRENT", shape="BOX", worldTrack=track)
        with tempfile.TemporaryDirectory() as temporary:
            folder=Path(temporary); path=folder/"validate.ps1"; source=folder/"region.json"
            path.write_text(script, encoding="utf-8-sig")
            def run(value):
                source.write_text(json.dumps(value),encoding="utf-8")
                return subprocess.run(["powershell","-NoProfile","-ExecutionPolicy","Bypass","-File",str(path)],
                                      capture_output=True,text=True,timeout=30)
            result=run(region)
            self.assertEqual(0,result.returncode,result.stderr)
            rows=[r.split("\t") for r in json.loads(result.stdout)]
            self.assertEqual(2,sum(r[0]=="PATTERNLOGICREGIONWORLDKEY" for r in rows))
            for invalid in ("rotation","scale","hidden","baseline","clock","endpoint"):
                value=copy.deepcopy(region); current=value["worldTrack"]
                if invalid=="rotation": current["keys"][0]["rotationY"]=2
                elif invalid=="scale": current["keys"][0]["scaleMultiplier"]=[2,2,2]
                elif invalid=="hidden": current["keys"][0]["visible"]=False
                elif invalid=="baseline": current["baselineYawDegrees"]=5
                elif invalid=="clock": current["startDelayMs"]=1
                else: current["keys"].pop()
                with self.subTest(invalid=invalid):
                    self.assertNotEqual(0,run(value).returncode)


if __name__ == "__main__":
    unittest.main()
