import copy
import json
import math
import unittest
from pathlib import Path
from Tools.KoukuSaydonPipeline import prepare_mario_world_blades as subject
from Tools.KoukuSaydonPipeline import world_object_collider as collider

ROOT = Path(__file__).resolve().parents[2]
class MarioWorldBladeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # This one-time migration predates the later single-blade/22s authoring.
        # Keep its historical input independent of the live saved-tuning tests below.
        fixture = json.loads((Path(__file__).parent / "fixtures" /
            "mario_world_blades_migration_input.json").read_text(encoding="utf-8"))
        cls.action = fixture["action"]
        cls.world = fixture["worldSequences"]
    def test_scoped_prepare_preserves_libraries_placements_dolls_and_is_idempotent(self):
        a, w, report = subject.prepare(self.action, self.world)
        again = subject.prepare(a, w)
        self.assertEqual((a,w),again[:2])
        old = {x["patternId"]:x for x in self.action["patterns"]}
        for row in a["patterns"]:
            if row["patternId"] != subject.PHASE:self.assertEqual(row,old[row["patternId"]])
        phase = next(x for x in a["patterns"] if x["patternId"] == subject.PHASE)
        for row in old[subject.PHASE]["worldOccurrences"]:
            current = next(x for x in phase["worldOccurrences"] if x["occurrenceId"]==row["occurrenceId"])
            self.assertEqual(current["placement"],row["placement"])
        source={x["sequenceId"]:x for x in self.world["templates"]}
        for row in w["templates"]:
            if row["sequenceId"]==subject.PATH_TEMPLATE:continue
            expected=copy.deepcopy(source[row["sequenceId"]])
            if row["sequenceId"] in ("world.object.kouku.cutting_blade.state.1","world.object.kouku.cutting_blade.state.instant_death"):
                for effect in expected.get("effectTracks",[]):effect["inheritObjectRotation"]=False
            self.assertEqual(row,expected)
        self.assertEqual(report["emissionCount"],8)
        self.assertEqual(report["arrivalMs"],10380)
        self.assertAlmostEqual(report["actualSpeedMps"],2.,places=3)
        source_sequence=source["world.object.kouku.cutting_blade.state.instant_death"]
        candidate=next(x for x in w["templates"] if x["sequenceId"]==subject.PATH_TEMPLATE)
        self.assertLessEqual(report["transformKeyCount"],256)
        max_error=0.
        for time in range(candidate["durationMs"]+1):
            original=collider.sample_key(source_sequence,"object",time)
            sampled=collider.sample_key(candidate,"object",time)
            max_error=max(max_error,max(abs(a-b) for a,b in zip(original["scaleMultiplier"],sampled["scaleMultiplier"])))
            for original_component,sampled_component in zip(original["rotationQuaternion"],sampled["rotationQuaternion"]):
                self.assertAlmostEqual(original_component,sampled_component,places=12)
        self.assertLess(max_error,.0001)
    def test_all_eight_emissions_reach_the_same_displacement_and_bake_instant_death(self):
        a,w,report=subject.prepare(self.action,self.world)
        world=next(x for x in a["worlds"] if x["worldId"]==report["targetWorldId"])
        box=next(x for p in a["patterns"] if p["patternId"]==subject.PHASE for x in p["worldOccurrences"] if x["worldId"]==world["worldId"])
        instance=next(x for x in w["instances"] if x["instanceId"]==subject.PATH_INSTANCE)
        sequence=next(x for x in w["templates"] if x["sequenceId"]==subject.PATH_TEMPLATE)
        resource=next(x for x in w["objectResources"] if x["objectId"]==world["objectResourceId"])
        row=sequence["colliderTracks"][0]
        def no_model(*args):raise AssertionError("Unattached blade collider does not need a skeleton")
        for emitter in range(8):
            start=collider.sample_object(sequence,instance,resource,box,world,emitter,0,row,no_model)[0]
            end=collider.sample_object(sequence,instance,resource,box,world,emitter,report["arrivalMs"],row,no_model)[0]
            hold=collider.sample_object(sequence,instance,resource,box,world,emitter,11000,row,no_model)[0]
            standalone=collider.sample_object(sequence,instance,resource,{},world,emitter,report["arrivalMs"],row,no_model)[0]
            for axis in range(3):
                self.assertAlmostEqual(end[axis]-start[axis],subject.DESTINATION[axis]-subject.START[axis],places=5)
                self.assertAlmostEqual(end[axis],hold[axis],places=5)
                self.assertAlmostEqual(end[axis],standalone[axis],places=5)
        one_cycle=copy.deepcopy(box);one_cycle["durationMs"]=11000
        windows=collider.bake_windows(w,{world["worldId"]:world},[one_cycle],no_model)
        self.assertEqual(len(windows),8)
        for window in windows:
            self.assertEqual(window["behavior"],"INSTANT_DEATH")
            self.assertGreater(len(window["region"]["worldTrack"]["keys"]),2)
class SavedInstantBladeTimingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.action = json.loads((ROOT / subject.ACTION_PATH).read_text(encoding="utf-8-sig"))
        cls.sequences = json.loads((ROOT / subject.WORLD_PATH).read_text(encoding="utf-8-sig"))

    def saved_blade(self, ordinal):
        world = next(row for row in self.action["worlds"] if row["worldId"] == f"kakulsaydon.g1.world.{ordinal}")
        instance = next(row for row in self.sequences["instances"] if row["instanceId"] == world["sequenceInstanceId"])
        sequence = next(row for row in self.sequences["templates"] if row["sequenceId"] == instance["templateId"])
        resource = next(row for row in self.sequences["objectResources"] if row["objectId"] == world["objectResourceId"])
        return world, instance, sequence, resource

    @staticmethod
    def no_model(*args):
        raise AssertionError("The blade floor collider has no skeletal attachment")

    def test_saved_instant_blade_lifetimes_preserve_sound_starts_and_unrelated_blades(self):
        for ordinal in (38, 39):
            with self.subTest(world=ordinal):
                _, _, sequence, _ = self.saved_blade(ordinal)
                self.assertEqual(22000, sequence["durationMs"])
                self.assertEqual([1440, 0, 0], sequence["objectMotion"]["angularVelocityDegrees"])
                self.assertEqual((1, 1), (sequence["objectMotion"]["count"], len(sequence["objectMotion"]["emissions"])))
                for lane in ("effectTracks", "colliderTracks", "soundTracks"):
                    self.assertTrue(sequence[lane])
                    self.assertTrue(all(row["startMs"] + row["durationMs"] == 22000 for row in sequence[lane]))
                self.assertEqual([0, 2000, 3000], [row["startMs"] for row in sequence["soundTracks"]])
        for ordinal in (19, 48):
            self.assertEqual(11000, self.saved_blade(ordinal)[2]["durationMs"])
        for ordinal, occurrence in ((33, 5), (95, 1)):
            pattern = next(row for row in self.action["patterns"] if row["patternId"] == f"KAKULSAYDON_G1_PATTERN_{ordinal}")
            box = next(row for row in pattern["worldOccurrences"] if row["occurrenceId"].endswith(f".world.{occurrence}"))
            self.assertEqual(22000, box["durationMs"])
            self.assertEqual(27830 if ordinal == 33 else 22000, pattern["durationMs"])

    def test_actual_floor_sampler_moves_at_one_metre_per_second_and_reaches_saved_endpoint(self):
        for ordinal in (38, 39):
            world, instance, sequence, resource = self.saved_blade(ordinal)
            row = sequence["colliderTracks"][0]
            sample = lambda time: collider.sample_object(sequence, instance, resource, {}, world, 0, time, row, self.no_model)[0]
            with self.subTest(world=ordinal):
                start, one_second, two_seconds = sample(0), sample(1000), sample(2000)
                self.assertAlmostEqual(1, math.dist(start, one_second), places=4)
                self.assertAlmostEqual(1, math.dist(one_second, two_seconds), places=4)
                if ordinal == 38:
                    self.assertAlmostEqual(22, math.dist(start, sample(22000)), places=5)
                else:
                    keys = sequence["tracks"][0]["keys"]
                    arrival = next(key["timeMs"] for key in keys if key["positionOffset"] == keys[-1]["positionOffset"])
                    self.assertGreater(arrival, 19000)
                    self.assertLess(arrival, 22000)
                    self.assertEqual(sample(arrival), sample(22000))

    def test_published_instant_death_windows_cover_the_full_twenty_two_second_motion(self):
        for ordinal, pattern_id, occurrence in ((38, 95, 1), (39, 33, 5)):
            world, _, _, _ = self.saved_blade(ordinal)
            pattern = next(row for row in self.action["patterns"] if row["patternId"] == f"KAKULSAYDON_G1_PATTERN_{pattern_id}")
            box = next(row for row in pattern["worldOccurrences"] if row["occurrenceId"].endswith(f".world.{occurrence}"))
            windows = collider.bake_windows(self.sequences, {world["worldId"]: world}, [box], self.no_model,
                pattern_end_ms=pattern["durationMs"])
            with self.subTest(world=ordinal):
                self.assertEqual(1, len(windows))
                self.assertEqual("INSTANT_DEATH", windows[0]["behavior"])
                self.assertEqual((box["startMs"], 22000), (windows[0]["startMs"], windows[0]["durationMs"]))
                self.assertEqual(22000, windows[0]["region"]["worldTrack"]["keys"][-1]["timeMs"])


if __name__=="__main__":unittest.main()
