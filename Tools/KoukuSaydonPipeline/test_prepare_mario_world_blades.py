import copy
import json
import unittest
from pathlib import Path
from Tools.KoukuSaydonPipeline import prepare_mario_world_blades as subject
from Tools.KoukuSaydonPipeline import world_object_collider as collider

ROOT = Path(__file__).resolve().parents[2]
class MarioWorldBladeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.action = json.loads((ROOT/subject.ACTION_PATH).read_text(encoding="utf-8-sig"))
        cls.world = json.loads((ROOT/subject.WORLD_PATH).read_text(encoding="utf-8-sig"))
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
if __name__=="__main__":unittest.main()
