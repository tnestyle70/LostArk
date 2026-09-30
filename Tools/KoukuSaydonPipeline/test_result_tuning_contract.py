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
from Tools.KoukuSaydonPipeline import world_object_collider as native_collider


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

    def test_soldier_combat_overrides_are_optional_bounded_and_trigger_owned(self):
        defaults = dict(logicType="TRIGGER", triggerKind="CARD_RAIN_SOLDIERS")
        row = self.validate_logic(**defaults)
        self.assertEqual((0, 0), (row["soldierMaxHp"], row["soldierDamage"]))
        row = self.validate_logic(**defaults, soldierMaxHp=69000, soldierDamage=13200)
        self.assertEqual((69000, 13200), (row["soldierMaxHp"], row["soldierDamage"]))
        for key in ("soldierMaxHp", "soldierDamage"):
            for value in (-1, True, 1.5, 2000000001):
                with self.subTest(key=key, value=value), self.assertRaises(subject.CompositionError):
                    self.validate_logic(**defaults, **{key: value})
            with self.assertRaises(subject.CompositionError):
                self.validate_logic(logicType="TRIGGER", triggerKind="BINGO_DETONATION", **{key: 1})

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


class DollEffectColliderContractTests(unittest.TestCase):
    """Installed mouth poses must meet the visible flame, including its parent TRS."""

    @classmethod
    def setUpClass(cls):
        cls.root = subject.REPOSITORY_ROOT
        cls.document = subject.load_json(cls.root / subject.SOURCE_PATH)
        cls.sequences = subject.load_world_sequences(cls.root, cls.document["areaId"])
        cls.effect = subject.load_json(cls.root / "Data/Effects/Authored/effect.kouku.gate3.doll.flame.shared.effect.json")
        worlds = {row["worldId"]: row for row in cls.document["worlds"]}
        cls.cases, cls.balls = [], []
        for pattern in cls.document["patterns"]:
            for box in pattern.get("worldOccurrences", []):
                world = worlds[box["worldId"]]
                identity = world["sequenceInstanceId"]
                colliders = [row for row in pattern.get("presentationOccurrences", [])
                             if row.get("worldOccurrenceId") == box["occurrenceId"] and row.get("logicOccurrenceId")]
                if identity == "world.object.instance.kouku.odd_doll.large.att_battle_2_01":
                    cls.cases.extend((pattern, world, box, row) for row in colliders if row.get("bone"))
                elif identity == "world.object.instance.kouku.mario_circus_ball.aura":
                    cls.balls.append((pattern, world, box, colliders))
        cls.models = {}

    @staticmethod
    def multiply(*matrices):
        result = matrices[0]
        for value in matrices[1:]:
            result = native_collider.wm.matrix_multiply(result, value)
        return result

    def flame_frame(self, sequences, world, box, collider, elapsed_ms):
        """Independent Client element/socket/bone/Effect/Object matrix construction."""
        instance = next(row for row in sequences["instances"] if row["instanceId"] == world["sequenceInstanceId"])
        template = next(row for row in sequences["templates"] if row["sequenceId"] == instance["templateId"])
        binding = instance["bindings"][0]
        resource = next(row for row in sequences["objectResources"] if row["objectId"] == binding["targetId"])
        effect = next(row for row in template["effectTracks"] if row["effectTrackId"] == "effect.doll.flame")
        age = elapsed_ms * box["playbackSpeed"] * instance.get("playbackSpeed", 1) % template["durationMs"]

        def load_model(asset, clips):
            key = (asset, tuple(clips))
            if key not in self.models:
                self.models[key] = native_collider.wm.read_wmodel(self.root / "Client/Bin/Resources" / asset,
                    include_geometry=False, animation_names=set(clips))
            return self.models[key]

        bone = native_collider.sample_bone(template, resource, binding["slotId"], age, collider["bone"], load_model)
        # Client CModel keeps its .01 pre-transform on both basis and translation.
        for axis in (0, 4, 8):
            for component in range(3):
                bone[axis + component] *= resource["modelPreScale"]
        element = next(row for row in self.effect["elements"] if
            row.get("actionCueAttachment", {}).get("runtimeBoneName") == collider["bone"] and
            row["sourceNode"].endswith("par_g_rpct_05_fire_01_loc_int.particlespriteemitter_12"))
        socket, local = element["actionCueAttachment"]["socketLocalTransform"], element["detail"]["transform"]
        matrix, rotation = native_collider.matrix, native_collider.rotation
        placement = box["placement"]
        parent = self.multiply(matrix(effect["scale"], rotation(effect["rotationDegrees"]), effect["positionOffset"]),
            matrix(resource["scale"]), matrix(placement["scale"], rotation(placement["rotationDegrees"]), placement["position"]))
        frame = self.multiply(matrix(quat=rotation(local["rotationDegrees"])),
            matrix(socket["scale"], rotation(socket["rotationDegrees"]), socket["position"]), bone, parent)
        center = native_collider.wm.transform_point(
            tuple(value / resource["modelPreScale"] for value in collider["positionOffset"]), self.multiply(bone, parent))
        length = math.hypot(frame[8], frame[10])
        return frame[12:15], (frame[8] / length, frame[10] / length), center

    @staticmethod
    def inside_box(key, half_extents, point):
        yaw = 2 * math.atan2(key["rotationY"], key["rotationW"])
        dx, dz = point[0] - key["positionOffset"][0], point[1] - key["positionOffset"][2]
        x, z = math.cos(yaw) * dx - math.sin(yaw) * dz, math.sin(yaw) * dx + math.cos(yaw) * dz
        return (abs(x) <= half_extents[0] * key["scaleMultiplier"][0] and
                abs(z) <= half_extents[2] * key["scaleMultiplier"][2])

    def test_saved_twenty_mouth_colliders_touch_flame_axis_and_reject_old_axis(self):
        self.assertEqual(20, len(self.cases))
        self.assertEqual({34, 88, 89, 91, 92, 93}, {int(row[0]["patternId"].rsplit("_", 1)[1]) for row in self.cases})
        resources = {row["resourceId"]: row for row in self.document["presentationResources"]}
        with subject._publication_session():
            for pattern, world, box, saved in self.cases:
                collider = dict(saved, worldEffectTrackId="effect.doll.flame")
                current = subject._project_world_bone_collider_track(self.root, self.sequences, world, box, collider)
                previous = subject._project_world_bone_collider_track(self.root, self.sequences, world, box,
                    dict(collider, worldEffectTrackId=""))
                self.assertEqual((box["startMs"], box["durationMs"]), (current["startMs"], current["durationMs"]))
                half = [a * b for a, b in zip(resources[collider["resourceId"]]["halfExtents"], collider["scale"])]
                for time in (2000, 4000, 8000, 12000, 16000, 19314, 36628):
                    if time >= box["durationMs"]:
                        continue
                    with self.subTest(collider=collider["occurrenceId"], time=time):
                        key = min(current["keys"], key=lambda row: abs(row["timeMs"] - time))
                        old = next(row for row in previous["keys"] if row["timeMs"] == key["timeMs"])
                        origin, forward, center = self.flame_frame(self.sequences, world, box, collider, key["timeMs"])
                        self.assertLess(math.dist(center, key["positionOffset"]), .015)
                        yaw = 2 * math.atan2(key["rotationY"], key["rotationW"])
                        self.assertGreater(math.sin(yaw) * forward[0] + math.cos(yaw) * forward[1], .99999)
                        flame_point = (origin[0] + 5 * forward[0], origin[2] + 5 * forward[1])
                        self.assertTrue(self.inside_box(key, half, flame_point), "Visible flame centerline must charge madness")
                        self.assertFalse(self.inside_box(old, half, flame_point), "Fixture must reproduce the previous missing hit")
                        old_yaw = 2 * math.atan2(old["rotationY"], old["rotationW"])
                        old_point = (box["placement"]["position"][0] + 5 * math.sin(old_yaw),
                                     box["placement"]["position"][2] + 5 * math.cos(old_yaw))
                        self.assertTrue(self.inside_box(old, half, old_point))
                        self.assertFalse(self.inside_box(key, half, old_point), "The previous empty-space hit must disappear")

    def test_effect_rotation_changes_follow_saved_track_and_empty_binding_stays_unchanged(self):
        _, world, box, saved = self.cases[0]
        box = copy.deepcopy(dict(box, startMs=0, durationMs=5000))
        box["placement"].update(rotationDegrees=[0, 23, 0], scale=[1.25, 1.25, 1.25])
        sequences = copy.deepcopy(self.sequences)
        instance = next(row for row in sequences["instances"] if row["instanceId"] == world["sequenceInstanceId"])
        template = next(row for row in sequences["templates"] if row["sequenceId"] == instance["templateId"])
        effect = next(row for row in template["effectTracks"] if row["effectTrackId"] == "effect.doll.flame")
        collider = dict(saved, worldEffectTrackId="effect.doll.flame")
        plain = dict(saved, worldEffectTrackId="")
        with subject._publication_session():
            before = subject._project_world_bone_collider_track(self.root, sequences, world, box, collider)
            unchanged = subject._project_world_bone_collider_track(self.root, sequences, world, box, plain)
            effect["rotationDegrees"][1] = 37
            effect["scale"] = [2, 2, 2]
            effect["positionOffset"] = [.25, .5, -.75]
            after = subject._project_world_bone_collider_track(self.root, sequences, world, box, collider)
            self.assertEqual(unchanged, subject._project_world_bone_collider_track(self.root, sequences, world, box, plain))
        for target in (2000, 4000):
            old = min(before["keys"], key=lambda row: abs(row["timeMs"] - target))
            key = next(row for row in after["keys"] if row["timeMs"] == old["timeMs"])
            for previous_scale, current_scale in zip(old["scaleMultiplier"], key["scaleMultiplier"]):
                self.assertAlmostEqual(previous_scale * 2, current_scale, places=7)
            difference = math.degrees(2 * math.atan2(key["rotationY"], key["rotationW"]) -
                2 * math.atan2(old["rotationY"], old["rotationW"]))
            self.assertAlmostEqual(-53, (difference + 180) % 360 - 180, places=5)
            _, forward, center = self.flame_frame(sequences, world, box, collider, key["timeMs"])
            self.assertLess(math.dist(center, key["positionOffset"]), .015)
            yaw = 2 * math.atan2(key["rotationY"], key["rotationW"])
            self.assertGreater(math.sin(yaw) * forward[0] + math.cos(yaw) * forward[1], .99999)

    def test_effect_binding_rejects_missing_duplicate_slot_and_nonfollowing_track(self):
        _, world, saved_box, saved = self.cases[0]
        box = dict(saved_box, durationMs=100)
        for invalid in ("missing", "duplicate", "slot", "follow", "bone", "rotation", "kind"):
            sequences = copy.deepcopy(self.sequences)
            instance = next(row for row in sequences["instances"] if row["instanceId"] == world["sequenceInstanceId"])
            template = next(row for row in sequences["templates"] if row["sequenceId"] == instance["templateId"])
            effect = next(row for row in template["effectTracks"] if row["effectTrackId"] == "effect.doll.flame")
            collider = dict(saved, worldEffectTrackId="effect.doll.flame")
            if invalid == "missing": collider["worldEffectTrackId"] = "missing.effect"
            elif invalid == "duplicate": template["effectTracks"].append(copy.deepcopy(effect))
            elif invalid == "slot": effect["slotId"] = "other.slot"
            elif invalid == "follow": effect["followObject"] = False
            elif invalid == "bone": effect["bone"] = "b_mouth_f"
            elif invalid == "rotation": effect["inheritObjectRotation"] = False
            else: effect["resourceKind"] = "V2_EFFECT"
            before = copy.deepcopy((sequences, collider))
            with self.subTest(invalid=invalid), self.assertRaises(subject.CompositionError):
                subject._project_world_bone_collider_track(self.root, sequences, world, box, collider)
            self.assertEqual(before, (sequences, collider))

    def test_effect_binding_rejects_unsupported_collider_attachment_without_mutation(self):
        _, world, saved_box, saved = self.cases[0]
        for change in (dict(anchorKind="MAP"), dict(bone=""), dict(boneTarget="WEAPON"),
                       dict(followBoss=False), dict(boneRotation="TARGET_YAW")):
            collider = dict(saved, worldEffectTrackId="effect.doll.flame", **change)
            before = copy.deepcopy(collider)
            with self.subTest(change=change), self.assertRaises(subject.CompositionError):
                subject._project_world_bone_collider_track(self.root, self.sequences, world,
                    dict(saved_box, durationMs=100), collider)
            self.assertEqual(before, collider)

    def test_ten_circus_ball_regions_remain_identical_to_published_geometry(self):
        self.assertEqual(10, len(self.balls))
        document = copy.deepcopy(self.document)
        mouths = {row[3]["occurrenceId"] for row in self.cases}
        for pattern in document["patterns"]:
            for row in pattern.get("presentationOccurrences", []):
                if row["occurrenceId"] in mouths:
                    row["worldEffectTrackId"] = "effect.doll.flame"
        published = subject.load_json(self.root / subject.ENCOUNTER_PATH)
        products = {row["patternId"]: row for row in published["patterns"]}
        definitions = {row["logicId"]: row for row in document["logics"]}
        for saved_pattern, _, _, colliders in self.balls:
            pattern = next(row for row in document["patterns"] if row["patternId"] == saved_pattern["patternId"])
            self.assertEqual(1, len(colliders))
            logic_id = colliders[0]["logicOccurrenceId"]
            window = next(row for row in pattern["logicOccurrences"] if row["occurrenceId"] == logic_id)
            with self.subTest(window=logic_id):
                projected = subject._project_collider_regions(document, pattern, window,
                    definitions[window["logicId"]], self.sequences, self.root)
                expected = next(row["cardRegions"] for row in products[pattern["patternId"]]["logicWindows"]
                    if row["windowId"] == logic_id)
                self.assertEqual(subject.serialize_json(expected), subject.serialize_json(projected))


if __name__ == "__main__":
    unittest.main()
