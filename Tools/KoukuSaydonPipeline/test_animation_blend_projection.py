import copy
from pathlib import Path
from types import SimpleNamespace
import unittest
from unittest import mock

from Tools.KoukuSaydonPipeline import project_kouku_saydon_composition as subject


def fixture():
    animations = [dict(occurrenceId=f"P.animation.{i + 1}", sourceActionId=0,
        sourceStageId="RAW", sourceSlot="clip", profileId="MN_RPCT_05",
        runtimeClip=name, startOffsetMs=offset, sourceStartMs=source,
        sourceEndMs=0, playMs=play, playRate=1.0, endPolicy="HOLD_LAST_POSE")
        for i, (name, offset, source, play) in enumerate((("a", 0, 0, 992), ("b", 30, 200, 970), ("c", 0, 0, 1000)))]
    pattern = dict(patternId="P", actorProfileId="MN_RPCT_05", gateId="GATE1",
        targetBossPlacementId="boss.kakulsaydon.g1.saydon", authoringStatus="PRODUCT",
        category="MECHANIC", displayName="Blend", nextStageOrdinal=4, nextAnimationOrdinal=4,
        nextPatternOccurrenceOrdinal=1, stages=[dict(stageId=f"STAGE_{i + 1}",
            actionId=f"P_ACTION_{i + 1}", stageKind="RECOVERY", durationMs=1000,
            animationOccurrences=[row]) for i, row in enumerate(animations)],
        logicOccurrences=[dict(occurrenceId="P.logic.1", logicId="kakulsaydon.g1.logic.52",
            startMs=800, durationMs=400)])
    logic = dict(logicId="kakulsaydon.g1.logic.52", displayName="Blend", logicType="TRIGGER", triggerKind="ANIMATION_BLEND")
    return dict(patterns=[pattern], logics=[logic], presentationResources=[], playAllPatternIds=["P"], revision=1,
        bossArchetypeId=subject.BOSS_ARCHETYPE_ID, bossPlacementId=subject.BOSS_PLACEMENT_ID,
        encounterId=subject.ENCOUNTER_ID, areaId=subject.AREA_ID, fixedTickHz=subject.FIXED_TICK_HZ), pattern


class AnimationBlendProjectionTests(unittest.TestCase):
    def test_exact_pair_timing_and_crop_descriptor(self):
        document, pattern = fixture()
        before = copy.deepcopy(document)
        windows = subject.resolve_animation_blend_windows(document, pattern)
        self.assertEqual(1, len(windows))
        window = windows[0]
        self.assertEqual({"logicOccurrenceId", "startMs", "durationMs", "source", "target"}, set(window))
        self.assertEqual((800, 400), (window["startMs"], window["durationMs"]))
        self.assertEqual(("a", "b"), (window["source"]["runtimeClip"], window["target"]["runtimeClip"]))
        self.assertEqual((1000, 1030, 200), (window["target"]["poseStartMs"], window["target"]["startMs"], window["target"]["sourceStartMs"]))
        self.assertEqual(before, document)
        for t, alpha in ((800, 0), (900, .25), (1000, .5), (1100, .75)):
            self.assertEqual(alpha, subject.sample_animation_blend_window(windows, t)[1])
        self.assertIsNone(subject.sample_animation_blend_window(windows, 799))
        self.assertIsNone(subject.sample_animation_blend_window(windows, 1200))

    def test_boundary_at_window_end_and_disabled_window(self):
        document, pattern = fixture()
        pattern["logicOccurrences"][0].update(startMs=700, durationMs=300)
        self.assertEqual(1, len(subject.resolve_animation_blend_windows(document, pattern)))
        pattern["logicOccurrences"][0].update(enabled=False, durationMs=90000)
        self.assertEqual([], subject.resolve_animation_blend_windows(document, pattern))

    def test_window_rejects_ambiguity_other_owner_and_legacy_blend(self):
        changes = (lambda p: p["logicOccurrences"][0].update(durationMs=0),
            lambda p: p["logicOccurrences"][0].update(durationMs=1001),
            lambda p: p["logicOccurrences"][0].update(startMs=1100, durationMs=100),
            lambda p: p["logicOccurrences"][0].update(startMs=1000, durationMs=1000),
            lambda p: p["stages"][1]["animationOccurrences"][0].update(profileId="MN_RPCZ_00"),
            lambda p: p["stages"][1]["animationOccurrences"][0].update(blendInMs=100),
            lambda p: p["logicOccurrences"].append(dict(p["logicOccurrences"][0], occurrenceId="P.logic.2")))
        for change in changes:
            document, pattern = fixture(); change(pattern); before = copy.deepcopy(document)
            with self.subTest(change=change), self.assertRaises(subject.CompositionError):
                subject.resolve_animation_blend_windows(document, pattern)
            self.assertEqual(before, document)

    def test_trigger_has_no_gameplay_parameters(self):
        document, _ = fixture()
        row = document["logics"][0]
        self.assertEqual("ANIMATION_BLEND", subject._validate_logic_definition(row, "blend", 53)[1]["kind"])
        for key, value in (("hudMode", "NONE"), ("bossChargeDistanceM", 1), ("radiusM", 3)):
            with self.assertRaises(subject.CompositionError):
                subject._validate_logic_definition(dict(row, **{key: value}), "blend", 53)

    def test_product_windows_intersect_both_semantic_actions_without_server_trigger(self):
        document, pattern = fixture()
        # Projection-only filesystem dependencies are independent of the derived clocks.
        with mock.patch.object(subject, "_project_pattern_root_motion", return_value={}), \
             mock.patch.object(subject, "_join_light_resources", return_value=None), \
             mock.patch.object(subject, "_project_fear_presentations", return_value=[]), \
             mock.patch.object(subject, "_project_attachment_grips", return_value=[]), \
             mock.patch.object(subject, "_project_folders", return_value=[]), \
             mock.patch.object(subject, "_project_bundles", return_value=[]), \
             mock.patch.object(subject, "_load_bone_bake_actor", return_value={}), \
             mock.patch.object(subject, "_clip_native_ms", return_value=2000), \
             mock.patch.object(subject, "arena_boss_archetypes_by_profile", return_value={}), \
             mock.patch.object(subject, "_project_stage", return_value={}):
            output = subject.project_presentation(document)
            self.assertEqual([1, 1, 0], [len(row.get("animationBlendWindows", [])) for row in output["bindings"]])
            self.assertEqual(output["bindings"][0]["animationBlendWindows"], output["patterns"][0]["animationBlendWindows"])
            encounter = subject.project_encounter(document)["patterns"][0]
            self.assertEqual([], encounter["mechanicTriggers"])
            self.assertTrue(encounter["fixedTimeline"])
            pattern["logicOccurrences"][0]["enabled"] = False
            self.assertNotIn("fixedTimeline", subject.project_encounter(document)["patterns"][0])

    def test_bone_bake_uses_upcoming_first_pose_and_current_source_time(self):
        document, pattern = fixture()
        windows = subject.resolve_animation_blend_windows(document, pattern)
        model = SimpleNamespace(skeleton_bones=[SimpleNamespace(name="tip")],
            animations=[SimpleNamespace(name=name, duration_ticks=2000, ticks_per_second=1000) for name in ("a", "b", "c")])
        actor = dict(actorProfileId="MN_RPCT_05", body=model, weapon=None)
        samples = []
        def pose(actor, part, clip, seconds, *args, **kwargs):
            blend = args[1] if len(args) > 1 else kwargs.get("blend")
            samples.append((clip, seconds, blend))
            return [[1., 0, 0, 0, 0, 1., 0, 0, 0, 0, 1., 0, 0, 0, 0, 1.]]
        with mock.patch.object(subject, "_load_bone_bake_actor", return_value=actor), \
             mock.patch.object(subject, "_sample_bone_bake_pose", side_effect=pose):
            track = subject._build_bone_collider_track(pattern, dict(startMs=800, durationMs=400),
                dict(bone="tip", anchorKind="BOSS", followBoss=True), Path.cwd(), {}, False, windows)
        self.assertIn(0, [key["timeMs"] for key in track["keys"]])
        self.assertIn(400, [key["timeMs"] for key in track["keys"]])
        early = [row for row in samples if row[2] and row[2][2] < .5]
        self.assertTrue(early)
        self.assertTrue(all(row[0] == "b" and abs(row[1] - .2) < 1e-8 and row[2][0] == "a" for row in early))
        self.assertGreater(max(row[2][1] for row in early), min(row[2][1] for row in early))

    def test_weapon_bake_receives_the_same_pair_clock_and_alpha_as_body(self):
        document, pattern = fixture()
        pattern["actorProfileId"] = "MN_RPCT_06"
        body_names = [f"mn_rpct_06_sk.ao_att_battle_2_0{i + 1}" for i in range(3)]
        weapon_names = [f"wprpct06_att_battle_2_0{i + 1}" for i in range(3)]
        for stage, name in zip(pattern["stages"], body_names):
            stage["animationOccurrences"][0].update(profileId="MN_RPCT_06", runtimeClip=name)
        def model(names, bone):
            return SimpleNamespace(skeleton_bones=[SimpleNamespace(name=bone)],
                animations=[SimpleNamespace(name=name, duration_ticks=2000, ticks_per_second=1000) for name in names])
        actor = dict(actorProfileId="MN_RPCT_06", body=model(body_names, "b_wp_1"), weapon=model(weapon_names, "tip"))
        samples = []
        def pose(actor, part, clip, seconds, root_scale=1, blend=None):
            samples.append((part, clip, seconds, blend))
            return [[1., 0, 0, 0, 0, 1., 0, 0, 0, 0, 1., 0, 0, 0, 0, 1.]]
        with mock.patch.object(subject, "_load_bone_bake_actor", return_value=actor), \
             mock.patch.object(subject, "_sample_bone_bake_pose", side_effect=pose):
            subject._build_bone_collider_track(pattern, dict(startMs=800, durationMs=400),
                dict(bone="tip", anchorKind="BOSS", followBoss=True, boneTarget="WEAPON"), Path.cwd(), {}, False,
                subject.resolve_animation_blend_windows(document, pattern))
        active = 0
        for body, weapon in zip(samples[::2], samples[1::2]):
            self.assertEqual(("body", "weapon"), (body[0], weapon[0]))
            self.assertEqual(body[2], weapon[2])
            if body[3]:
                active += 1
                self.assertEqual((body_names[1], weapon_names[1]), (body[1], weapon[1]))
                self.assertEqual((body_names[0], weapon_names[0]), (body[3][0], weapon[3][0]))
                self.assertEqual(body[3][1:], weapon[3][1:])
        self.assertGreater(active, 1)

    def test_typed_windows_keep_exact_stage_clock_outside_blend(self):
        _, pattern = fixture()
        pattern["stages"][0]["durationMs"] = 992
        exact = list(subject._bone_bake_stage_origins(pattern, True))
        legacy = list(subject._bone_bake_stage_origins(pattern))
        self.assertEqual([0, 992, 1992], [origin for _, origin in exact])
        self.assertNotEqual([origin for _, origin in exact], [origin for _, origin in legacy])
        row, seconds = subject._bone_bake_clip_sample(pattern, 1992, True)
        self.assertEqual(("c", 0), (row["runtimeClip"], seconds))

    def motion_fixture(self):
        document, child = fixture()
        child["bossMotion"] = dict(startMs=100, endMs=800, startPosition=[0, 0, -5], endPosition=[0, 0, 0], yawDegrees=0)
        parent = {**copy.deepcopy(child), "patternId": "Parent", "stages": [], "logicOccurrences": [],
            "durationMs": 4000, "nextPatternOccurrenceOrdinal": 2,
            "patternOccurrences": [dict(occurrenceId="Parent.pattern.1", patternId="P", startMs=500, durationMs=3000, repeat=False)]}
        parent.pop("bossMotion")
        document["patterns"].append(parent)
        return document, parent, child

    def test_full_child_motion_and_blend_are_shifted_without_editing_child(self):
        document, parent, child = self.motion_fixture(); before = copy.deepcopy(document)
        expanded = subject.expand_pattern_document(document, "Parent")
        result = next(row for row in expanded["patterns"] if row["patternId"] == "Parent")
        self.assertEqual(dict(child["bossMotion"], startMs=600, endMs=1300), result["bossMotion"])
        windows = subject.resolve_animation_blend_windows(expanded, result)
        self.assertEqual((1300, 400), (windows[0]["startMs"], windows[0]["durationMs"]))
        self.assertEqual("Parent.pattern.1.r0.logic.1", windows[0]["logicOccurrenceId"])
        self.assertEqual(1530, windows[0]["target"]["startMs"])
        self.assertEqual(before, document)

    def test_child_motion_rejects_repeat_trim_second_owner_and_parent_movement(self):
        for mode in ("repeat", "trim", "parent_motion", "second_child", "reset", "animation", "retarget", "charge"):
            document, parent, child = self.motion_fixture()
            if mode == "repeat": parent["patternOccurrences"][0]["repeat"] = True
            if mode == "trim": parent["patternOccurrences"][0]["durationMs"] -= 1
            if mode == "parent_motion": parent["bossMotion"] = child["bossMotion"]
            if mode == "second_child":
                parent["durationMs"] = 8000; parent["nextPatternOccurrenceOrdinal"] = 3
                parent["patternOccurrences"].append(dict(parent["patternOccurrences"][0], occurrenceId="Parent.pattern.2", startMs=4000))
            if mode == "reset": parent["resetBossToSpawn"] = True
            if mode == "animation": parent["stages"] = [dict(child["stages"][0], durationMs=100, animationOccurrences=[dict(child["stages"][0]["animationOccurrences"][0], playMs=100)])]
            if mode == "retarget": parent["stages"] = [dict(child["stages"][0], durationMs=100, animationOccurrences=[], retargetOnEnter=True)]
            if mode == "charge":
                document["logics"].append(dict(logicId="charge", triggerKind="ENTER_AREA", bossChargeDistanceM=5))
                parent["logicOccurrences"] = [dict(occurrenceId="Parent.logic.1", logicId="charge", startMs=0, durationMs=100)]
            before = copy.deepcopy(document)
            with self.subTest(mode=mode), self.assertRaises(subject.CompositionError):
                subject.expand_pattern_document(document, "Parent")
            self.assertEqual(before, document)


if __name__ == "__main__":
    unittest.main()
