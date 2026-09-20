import copy
from types import SimpleNamespace
import unittest
from unittest.mock import patch

from Tools.KoukuSaydonPipeline import world_object_collider as subject


class ObjectColliderTests(unittest.TestCase):
    def fixture(self, behavior="DAMAGE"):
        row={"colliderTrackId":"collider.tip","slotId":"object","startMs":0,"durationMs":600,
             "positionOffset":[0,0,0],"halfExtents":[.5,.2,1],"yawDegrees":0,"behavior":behavior,
             "damagePercent":20 if behavior=="DAMAGE" else 0,"gripLocalOffset":[0,0,0]}
        keys=[{"timeMs":t,"positionOffset":[0,0,0],"rotationQuaternion":[0,0,0,1],"scaleMultiplier":[1,1,1],"visible":True} for t in (0,1000)]
        sequence={"sequenceId":"sequence.test","durationMs":1000,"interpolation":"LINEAR",
                  "tracks":[{"slotId":"object","keys":keys}],"colliderTracks":[row],"objectMotion":{"velocity":[2,0,0]}}
        instance={"instanceId":"motion.test","templateId":"sequence.test","anchorKind":"WORLD","position":[10,0,0],"bindings":[{"slotId":"object","targetKind":"OBJECT_RESOURCE","targetId":"object.test"}]}
        sequences={"templates":[sequence],"instances":[instance],"objectResources":[{"objectId":"object.test","modelAssetId":"unused.wmodel","scale":[1,1,1]}]}
        worlds={"world.test":{"worldId":"world.test","sequenceInstanceId":"motion.test"}}
        boxes=[{"occurrenceId":"world.occurrence","worldId":"world.test","startMs":0,"durationMs":3000,"playbackSpeed":1}]
        return sequences,worlds,boxes

    def bake(self, fixture, **kwargs):
        def no_model(*args): raise AssertionError("Unattached collider should not load a model")
        return subject.bake_windows(*fixture,no_model,**kwargs)

    def test_linear_blade_and_no_collider_compatibility(self):
        f=self.fixture();rows=self.bake(f)
        self.assertEqual(len(rows),1)
        self.assertEqual(rows[0]["region"]["worldTrack"]["keys"][0]["positionOffset"],[10,0,0])
        self.assertAlmostEqual(rows[0]["region"]["worldTrack"]["keys"][-1]["positionOffset"][0],11.2)
        del f[0]["templates"][0]["colliderTracks"]
        self.assertEqual(self.bake(f),[])

    def test_finite_loop_and_unscaled_delay(self):
        f=self.fixture();f[0]["instances"][0].update(motionEnd="LOOP",startDelayMs=100,playbackSpeed=2)
        rows=self.bake(f)
        self.assertEqual([r["startMs"] for r in rows],[100,600,1100,1600,2100,2600])
        self.assertEqual([r["durationMs"] for r in rows],[300]*6)
        self.assertEqual(len({r["occurrenceId"] for r in rows}),6)

    def test_emission_delay_and_visible_span_clock(self):
        f=self.fixture();s=f[0]["templates"][0]
        s["effectTracks"]=[{}]
        s["objectMotion"]["emissions"]=[{"positionOffset":[0,0,0],"yawDegrees":0,"startDelayMs":0},
            {"positionOffset":[5,0,0],"yawDegrees":90,"startDelayMs":700}]
        rows=self.bake(f)
        self.assertEqual([r["startMs"] for r in rows],[0,700])
        self.assertEqual(rows[1]["durationMs"],600)
        self.assertEqual(rows[1]["region"]["yawDegrees"],90)

    def test_hook_capture_and_carry_are_distinct(self):
        rows=self.bake(self.fixture("HOOK_CAPTURE"))
        self.assertEqual(rows[0]["durationMs"],600)
        self.assertEqual(rows[0]["region"]["worldTrack"]["durationMs"],1000)
        self.assertIn("gripPosition",rows[0]["region"]["worldTrack"]["keys"][0])

    def test_hide_then_reappear_creates_independent_capture(self):
        f=self.fixture("HOOK_CAPTURE");s=f[0]["templates"][0];s["colliderTracks"][0]["durationMs"]=1000
        base=s["tracks"][0]["keys"][0]
        s["tracks"][0]["keys"]=[{**base,"timeMs":t,"visible":visible} for t,visible in [(0,True),(400,False),(401,True),(1000,False)]]
        rows=self.bake(f)
        self.assertEqual([(r["startMs"],r["durationMs"]) for r in rows],[(0,400),(401,599)])
        self.assertNotEqual(rows[0]["occurrenceId"],rows[1]["occurrenceId"])

    def test_floor_offset_ignores_visual_spin(self):
        f=self.fixture();s=f[0]["templates"][0];s["colliderTracks"][0]["positionOffset"]=[1,0,0]
        baseline=self.bake(f)
        s["objectMotion"]["angularVelocityDegrees"]=[1440,0,0]
        for key in s["tracks"][0]["keys"]:key["rotationQuaternion"]=subject.rotation([0,0,90])
        actual=self.bake(f)
        self.assertEqual(actual,baseline)

    def test_unsupported_random_spread_is_explicit(self):
        f=self.fixture();f[0]["templates"][0]["objectMotion"]["spreadDegrees"]=20
        with self.assertRaisesRegex(subject.ColliderBakeError,"zero spread"):self.bake(f)

    def test_invalid_rows_are_rejected(self):
        for change in ({"damagePercent":20.5},{"halfExtents":[0,1,1]},{"durationMs":1001},{"attachmentBone":"b_hook"},{"slotId":"missing"}):
            f=self.fixture();f[0]["templates"][0]["colliderTracks"][0].update(change)
            with self.subTest(change=change),self.assertRaises(subject.ColliderBakeError):self.bake(f)

    def test_window_budget_is_bounded(self):
        f=self.fixture();f[0]["templates"][0]["objectMotion"].update(count=129)
        with self.assertRaisesRegex(subject.ColliderBakeError,"128-window"):self.bake(f)

    def test_non_tick_aligned_birth_samples_actual_server_time(self):
        f=self.fixture();f[0]["templates"][0]["colliderTracks"][0]["startMs"]=80
        row=self.bake(f)[0]
        self.assertEqual(row["startMs"],80)
        self.assertAlmostEqual(row["region"]["worldTrack"]["keys"][0]["positionOffset"][0],10.2)

    def test_next_motion_retains_initial_object_binding_and_position(self):
        f=self.fixture();initial=f[0]["instances"][0]
        initial.update(motionEnd="NEXT",nextMotionId="motion.next")
        f[0]["instances"].append({**initial,"instanceId":"motion.next","position":[999,0,0],"motionEnd":"STOP"})
        rows=self.bake(f)
        self.assertEqual([r["startMs"] for r in rows],[0,1000])
        self.assertEqual(rows[1]["region"]["worldTrack"]["keys"][0]["positionOffset"],[10,0,0])

    def test_born_object_with_effects_completes_after_world_birth_deadline(self):
        for end in ("STOP","HOLD"):
            f=self.fixture();s=f[0]["templates"][0];s["effectTracks"]=[{}];s["colliderTracks"][0]["durationMs"]=1000
            f[0]["instances"][0]["motionEnd"]=end;f[2][0]["durationMs"]=500
            with self.subTest(end=end):self.assertEqual(self.bake(f)[0]["durationMs"],1000)

    def test_pattern_end_closes_effect_tail_damage_and_hook_carry(self):
        for behavior in ("DAMAGE", "HOOK_CAPTURE"):
            for end in ("STOP", "HOLD", "LOOP"):
                with self.subTest(behavior=behavior, end=end):
                    f=self.fixture(behavior);s=f[0]["templates"][0]
                    s["effectTracks"]=[{}];s["colliderTracks"][0]["durationMs"]=1000
                    f[0]["instances"][0]["motionEnd"]=end
                    f[2][0].update(startMs=200,durationMs=500)
                    rows=self.bake(f,pattern_end_ms=800)
                    self.assertEqual([(r["startMs"],r["durationMs"]) for r in rows],[(200,600)])
                    track=rows[0]["region"]["worldTrack"]
                    self.assertEqual(track["durationMs"],600)
                    self.assertEqual(track["keys"][-1]["timeMs"],600)
                    self.assertFalse(track["keys"][-1]["visible"])
                    self.assertEqual(self.bake(f,pattern_end_ms=200),[])

    def test_hook_empty_bone_uses_full_visual_root_basis(self):
        f=self.fixture("HOOK_CAPTURE");s=f[0]["templates"][0];s["colliderTracks"][0]["gripLocalOffset"]=[1,0,0]
        for key in s["tracks"][0]["keys"]:key["rotationQuaternion"]=subject.rotation([0,90,0])
        grip=self.bake(f)[0]["region"]["worldTrack"]["keys"][0]["gripPosition"]
        self.assertAlmostEqual(grip[0],10);self.assertAlmostEqual(grip[2],-1)

    def test_hook_model_selection_is_reused_only_within_one_bake(self):
        fixture = self.fixture("HOOK_CAPTURE")
        fixture[0]["templates"][0]["colliderTracks"][0]["attachmentBone"] = "b_tip"
        model = SimpleNamespace(skeleton_bones=[SimpleNamespace(name="b_tip", parent=-1,
            transform=subject.matrix(position=(0, 0, 100)))], animations=[])
        calls = []
        def load(asset, clips):
            calls.append((asset, tuple(clips)))
            return model
        first = subject.bake_windows(*fixture, load)
        self.assertEqual([("unused.wmodel", ())], calls)
        self.assertEqual([10, 0, 1], first[0]["region"]["worldTrack"]["keys"][0]["gripPosition"])
        self.assertEqual(first, subject.bake_windows(*fixture, load))
        self.assertEqual(2, len(calls))
        model.skeleton_bones[0].name = "missing"
        with self.assertRaisesRegex(subject.ColliderBakeError, "absent or ambiguous"):
            subject.bake_windows(*fixture, load)

    def test_shared_snapshot_reuses_poses_without_aliasing_other_sequences(self):
        fixture = self.fixture("HOOK_CAPTURE")
        fixture[0]["templates"][0]["colliderTracks"][0]["attachmentBone"] = "b_tip"
        model = SimpleNamespace(skeleton_bones=[SimpleNamespace(name="b_tip", parent=-1,
            transform=subject.matrix(position=(0, 0, 100)))], animations=[])
        calls = []
        def load(asset, clips):
            calls.append((asset, tuple(clips)))
            return model
        cache = {}
        with patch.object(subject.wm, "combined_transforms", wraps=subject.wm.combined_transforms) as poses:
            first = subject.bake_windows(*fixture, load, sampling_cache=cache)
            count = poses.call_count
            self.assertGreater(count, 0)
            self.assertEqual(first, subject.bake_windows(*fixture, load, sampling_cache=cache))
            self.assertEqual(count, poses.call_count)
            other = copy.deepcopy(fixture)
            self.assertEqual(first, subject.bake_windows(*other, load, sampling_cache=cache))
            self.assertGreater(poses.call_count, count)
            self.assertEqual(1, len(calls))
            self.assertIs(fixture[0], cache["sequences"][id(fixture[0])])


if __name__=="__main__":unittest.main()
