import copy
import json
import math
import re
import subprocess
from pathlib import Path
import shutil
import tempfile
import unittest
from unittest import mock


from Tools.KoukuSaydonPipeline import project_kouku_saydon_composition as subject


ROOT = Path(__file__).resolve().parents[2]
WORLD_SEQUENCES = Path(str(subject.WORLD_SEQUENCE_PATH).format(area=subject.AREA_ID))
FIRST_PRODUCT_ID = "KAKULSAYDON_G1_PATTERN_1"
DRAFT_ID = "KAKULSAYDON_G1_PATTERN_3"
ROULETTE_ID = "KAKULSAYDON_G1_PATTERN_7"
DANCE_ID = "KAKULSAYDON_G1_PATTERN_6"
GAZE_ID = "KAKULSAYDON_G1_PATTERN_2"


def copy_repository_inputs(root: Path) -> None:
    """The files a temp-root projection reads beside the composition."""
    for relative in (subject.BOSS_CATALOG_PATH, subject.LIGHT_RESOURCES_PATH,
                     WORLD_SEQUENCES, Path(f"Data/Maps/Authoring/{subject.AREA_ID}/{subject.AREA_ID}.mapplacements")):
        destination = root / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(ROOT / relative, destination)


class KoukuSaydonCompositionProjectionTests(unittest.TestCase):
    def test_v1_source_anchor_animation_times_follow_stage_boundaries(self):
        document = {"presentationResources": [{"resourceId": "fx", "kind": "EFFECT",
            "resourceKind": "V1_EFFECT", "assetId": "effect.kouku.test.restore", "durationMs": 5000}]}
        pattern = {"patternId": "test", "gateId": "GATE1", "targetBossPlacementId": "test.boss",
            "actorProfileId": "MN_RPCT_05", "presentationOccurrences": [
                {"occurrenceId": "fx.1", "resourceId": "fx", "startMs": 0, "durationMs": 5000}],
            "stages": [{"durationMs": 1667, "animationOccurrences": [{"runtimeClip": "attack1",
                "startOffsetMs": 0, "sourceStartMs": 20, "playMs": 1500, "playRate": 1.1, "endPolicy": "EXACT"}]},
                {"durationMs": 3333, "animationOccurrences": [{"runtimeClip": "attack2", "startOffsetMs": 30,
                    "sourceStartMs": 40, "playMs": 3000, "playRate": 1.0, "endPolicy": "LOOP_TO_WINDOW", "blendInMs": 100}]}]}
        original = copy.deepcopy(pattern)
        result = subject._project_pattern_presentation(document, pattern)
        self.assertEqual([0, 1697], [row["startOffsetMs"] for row in result["sourceAnchorAnimations"]])
        self.assertEqual([0, 100], [row["blendInMs"] for row in result["sourceAnchorAnimations"]])
        self.assertEqual(40, result["sourceAnchorAnimations"][1]["sourceStartMs"])
        self.assertEqual("LOOP_TO_WINDOW", result["sourceAnchorAnimations"][1]["endPolicy"])
        self.assertEqual(original, pattern)
        document["presentationResources"][0]["resourceKind"] = "LEAF"
        self.assertNotIn("sourceAnchorAnimations", subject._project_pattern_presentation(document, pattern))

    @classmethod
    def setUpClass(cls):
        cls.hierarchy_document = subject.load_json(ROOT / subject.SOURCE_PATH)
        cls.document = copy.deepcopy(cls.hierarchy_document)
        # Existing lane tests own independent patterns, not the new saved bundle references.
        cls.document["folders"] = []
        cls.document["bundles"] = []
        for pattern in cls.document["patterns"]:
            pattern.pop("folderId", None)

    def test_animation_transition_projects_the_fixed_previous_endpoint(self):
        document = copy.deepcopy(self.document)
        for pattern in document["patterns"]:
            pattern["authoringStatus"] = "DRAFT"
        pattern = self.find(document, "KAKULSAYDON_G1_PATTERN_13")
        pattern["authoringStatus"] = "PRODUCT"
        clips = [row for stage in pattern["stages"] for row in stage["animationOccurrences"]]
        selected = next(row for row in clips if row["occurrenceId"].endswith(".animation.16"))
        selected["blendInMs"] = 100
        previous, seconds = subject._animation_blend_source(pattern, selected)
        self.assertTrue(previous["occurrenceId"].endswith(".animation.6"))
        self.assertEqual(1.0, seconds)
        row = next(row for row in subject.project_presentation(document)["bindings"]
                   if row["occurrenceId"] == selected["occurrenceId"])
        self.assertEqual(100, row["blendInMs"])
        self.assertEqual(previous["runtimeClip"], row["blendFromClip"])
        self.assertEqual(1000.0, row["blendFromSourceMs"])
        # The same clip can restart from a different source endpoint.
        selected["runtimeClip"] = previous["runtimeClip"]
        self.assertEqual(previous, subject._animation_blend_source(pattern, selected)[0])
        clips[0]["blendInMs"] = 100
        with self.assertRaisesRegex(subject.CompositionError, "previous occurrence"):
            subject._animation_blend_source(pattern, clips[0])
        clips[0].pop("blendInMs")
        previous["playMs"] -= 1
        with self.assertRaisesRegex(subject.CompositionError, "adjacent EXACT"):
            subject._animation_blend_source(pattern, selected)

    def test_bone_transition_interpolates_local_rotation_before_hierarchy(self):
        left = subject.wmodel_pose.affine_matrix((1, 1, 1), (0, 0, 0, 1), (0, 0, 0))
        right = subject.wmodel_pose.affine_matrix((2, 2, 2), (0, 0, 1, 0), (2, 4, 6))
        blended = subject._blend_local_matrix(left, right, 0.5)
        point = subject.wmodel_pose.transform_point((1, 0, 0), blended)
        for actual, expected in zip(point, (1, 3.5, 3)):
            self.assertAlmostEqual(expected, actual, places=6)
        for alpha, expected in ((0, left), (1, right)):
            for actual, value in zip(subject._blend_local_matrix(left, right, alpha), expected):
                self.assertAlmostEqual(value, actual, places=6)

    def validate(self, document):
        subject.validate_document(document, ROOT)

    @staticmethod
    def find(document, pattern_id):
        return next(
            pattern
            for pattern in document["patterns"]
            if pattern.get("patternId") == pattern_id
        )

    @classmethod
    def first_product(cls, document):
        return cls.find(document, FIRST_PRODUCT_ID)

    @classmethod
    def draft(cls, document):
        return cls.find(document, DRAFT_ID)

    @staticmethod
    def product_patterns(document):
        return [p for p in document["patterns"] if p["authoringStatus"] == "PRODUCT"]

    @classmethod
    def product_binding_count(cls, document):
        return sum(
            len(stage["animationOccurrences"])
            for pattern in cls.product_patterns(document)
            for stage in pattern["stages"]
        )

    @staticmethod
    def strip_lanes(pattern):
        """Drop every lane box of a pattern so a copy can live under another id."""
        for key in ("logicOccurrences", "summonOccurrences", "worldOccurrences",
                    "sceneProfileOccurrences", "presentationOccurrences"):
            pattern[key] = []
        for key in ("nextLogicOccurrenceOrdinal", "nextSummonOccurrenceOrdinal",
                    "nextWorldOccurrenceOrdinal", "nextSceneProfileOccurrenceOrdinal", "nextPresentationOccurrenceOrdinal"):
            pattern[key] = 1
        return pattern

    def surface_fixture(self):
        sequences = copy.deepcopy(subject.load_json(ROOT / WORLD_SEQUENCES))
        instance = next(row for row in sequences["instances"] if row["instanceId"] == "world.sequence.instance.8")
        instance["walkableSurface"] = {"radiusM": 2.5, "localHeightM": .026313}
        world = next(row for row in self.document["worlds"] if row["sequenceInstanceId"] == instance["instanceId"])
        pattern = self.find(self.document, ROULETTE_ID)
        box = copy.deepcopy(next(row for row in pattern["worldOccurrences"] if row["worldId"] == world["worldId"]))
        template = next(row for row in sequences["templates"] if row["sequenceId"] == instance["templateId"])
        return sequences, instance, template, world, box

    def test_walkable_surface_projects_placement_plane_and_last_hide(self):
        sequences, instance, template, world, box = self.surface_fixture()
        result = subject._project_walkable_surface(ROOT, subject.AREA_ID, sequences, world, box)["walkableSurface"]
        self.assertAlmostEqual(result["radiusM"], 10.0)
        self.assertAlmostEqual(result["heightY"], 1.89999998 + .026313 * 4)
        self.assertEqual(result["windows"][0]["startTick"], 0)
        expected_end = min(math.ceil(box["durationMs"] * .03),
            math.ceil(template["durationMs"] / box["playbackSpeed"] * .03))
        self.assertEqual(result["windows"][-1]["endTick"], expected_end)

    def test_walkable_surface_visibility_uses_delay_speed_and_tick_ceiling(self):
        sequences, instance, template, world, box = self.surface_fixture()
        first = template["tracks"][0]["keys"][0]
        template["tracks"][0]["keys"] = [dict(copy.deepcopy(first), timeMs=time, visible=visible)
            for time, visible in [(0, True), (100, False), (200, True), (300, False)]]
        template["durationMs"] = 300
        instance["startDelayMs"] = 50
        instance["playbackSpeed"] = 2
        box["playbackSpeed"] = 1
        box["durationMs"] = 1000
        result = subject._project_walkable_surface(ROOT, subject.AREA_ID, sequences, world, box)["walkableSurface"]
        self.assertEqual(result["windows"], [{"startTick": 2, "endTick": 3}, {"startTick": 5, "endTick": 6}])

    def test_walkable_surface_rejects_moving_tilted_or_invalid_circle(self):
        for field in ("position", "tilt", "radius"):
            sequences, instance, template, world, box = self.surface_fixture()
            if field == "position": template["tracks"][0]["keys"][-1]["positionOffset"][1] += .1
            elif field == "tilt": template["tracks"][0]["keys"][-1]["rotationQuaternion"][0] = .1
            else: instance["walkableSurface"]["radiusM"] = 0
            with self.subTest(field=field), self.assertRaises(subject.CompositionError):
                subject._project_walkable_surface(ROOT, subject.AREA_ID, sequences, world, box)

    def test_walkable_surface_bootstrap_uses_world_owner_and_rejects_invalid_windows(self):
        sequences, instance, template, world, box = self.surface_fixture()
        cue = dict(sequenceInstanceId=instance["instanceId"], occurrenceId=box["occurrenceId"],
                   startMs=box["startMs"], durationMs=box["durationMs"], playbackSpeed=box["playbackSpeed"],
                   positionOffset=world["positionOffset"], anchorKind=world["anchorKind"],
                   anchorPosition=world["anchorPosition"])
        cue.update(subject._project_walkable_surface(ROOT, subject.AREA_ID, sequences, world, box))
        publisher = (ROOT / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1").read_text(encoding="utf-8-sig")
        functions = []
        for name in ("Assert-ExactProperties", "Assert-StableId", "Assert-JsonString", "Assert-JsonInteger",
                     "Assert-JsonNumber", "Format-InvariantFloat", "Format-InvariantSignedFloat"):
            functions.append(re.search(r"(?ms)^function " + name + r"\b.*?^\}", publisher).group(0))
        start = publisher.index("\tforeach ($worldSequence in @($koukuPattern.worldSequences))")
        end = publisher.index("\tforeach ($sceneProfile in @($koukuPattern.sceneProfiles))", start)
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)
            path = folder / "cue.json"
            path.write_text(json.dumps(cue), encoding="utf-8")
            script = "$ErrorActionPreference='Stop'\n$stableIdPattern='^[A-Za-z0-9_.-]+$'\n" + "\n".join(functions)
            script += "\n$cue=Get-Content -Raw (Join-Path $PSScriptRoot 'cue.json') | ConvertFrom-Json\n"
            script += "$koukuEncounterDocument=@{encounterId='test.encounter'}; $koukuPattern=@{patternId='test.pattern';worldSequences=@($cue)}\n"
            script += "$koukuPatternDurationMs=600000; $patternRows=[Collections.Generic.List[string]]::new()\n"
            script += publisher[start:end] + "\nConvertTo-Json -InputObject @($patternRows) -Compress\n"
            check = folder / "check.ps1"; check.write_text(script, encoding="utf-8-sig")
            run = lambda: subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(check)], capture_output=True, text=True)
            result = run()
            self.assertEqual(0, result.returncode, result.stderr)
            rows = [line.split("\t") for line in json.loads(result.stdout)]
            self.assertEqual(["PATTERNWORLDSEQUENCE", "PATTERNWORLDSUPPORT"], [row[0] for row in rows])
            self.assertEqual(10, len(rows[1]))
            self.assertEqual(box["occurrenceId"], rows[1][3])
            self.assertEqual(10.0, float(rows[1][9]))
            cue["walkableSurface"]["windows"][0]["endTick"] = math.ceil(cue["durationMs"] * .03) + 1
            path.write_text(json.dumps(cue), encoding="utf-8")
            self.assertNotEqual(0, run().returncode)

    def object_overlap_document(self):
        document = copy.deepcopy(self.document)
        pattern = self.first_product(document)
        box = pattern["logicOccurrences"][0]
        target = "world.object.instance.kouku.card"
        logic = next(row for row in document["logics"] if row["logicId"] == box["logicId"])
        logic_id = logic["logicId"]
        logic.clear()
        logic.update(logicId=logic_id, displayName="Object overlap", logicType="DURATION",
                     judgementKind="OBJECT_OVERLAP", targetWorldInstanceId=target, targetRadiusM=1.25)
        result_id = f"kakulsaydon.g1.logic.{document['nextLogicOrdinal']}"
        document["nextLogicOrdinal"] += 1
        document["logics"].append(dict(logicId=result_id, displayName="Flip the same card", logicType="RESULT",
            outcomeKind="PLAY_WORLD_OBJECT_MOTION", targetWorldInstanceId=target,
            motionInstanceId="world.object.instance.kouku.card_flip"))
        box["onSuccessLogicIds"] = [result_id]
        box["onFailLogicIds"] = []
        box["onTimeoutLogicIds"] = []
        world_id = f"kakulsaydon.g1.world.{document['nextWorldOrdinal']}"
        document["nextWorldOrdinal"] += 1
        document["worlds"].append(dict(worldId=world_id, displayName="Target card", sequenceInstanceId=target,
                                       positionOffset=[100, 0, 900]))
        ordinal = pattern["nextWorldOccurrenceOrdinal"]
        pattern["nextWorldOccurrenceOrdinal"] += 1
        pattern["worldOccurrences"].append(dict(occurrenceId=f"{pattern['patternId']}.world.{ordinal}",
            worldId=world_id, startMs=box["startMs"], durationMs=box["durationMs"], playbackSpeed=1))
        return document

    def object_contact_document(self):
        document = self.object_overlap_document()
        pattern = self.first_product(document)
        box = pattern["logicOccurrences"][0]
        first = pattern["worldOccurrences"][-1]
        world = copy.deepcopy(next(row for row in document["worlds"] if row["worldId"] == first["worldId"]))
        world["worldId"] = f"kakulsaydon.g1.world.{document['nextWorldOrdinal']}"
        document["nextWorldOrdinal"] += 1
        world["positionOffset"][0] += 10
        document["worlds"].append(world)
        second = dict(first, occurrenceId=f"{pattern['patternId']}.world.{pattern['nextWorldOccurrenceOrdinal']}", worldId=world["worldId"])
        pattern["nextWorldOccurrenceOrdinal"] += 1
        pattern["worldOccurrences"].append(second)
        candidates = [first["occurrenceId"], second["occurrenceId"]]
        logic = next(row for row in document["logics"] if row["logicId"] == box["logicId"])
        logic_id = logic["logicId"]
        logic.clear()
        logic.update(logicId=logic_id, displayName="Contact each card", logicType="TRIGGER", triggerKind="OBJECT_CONTACT",
                     targetWorldOccurrenceIds=candidates, targetRadiusM=1.25, contactGroupId="hammer.impact", contactPriority=100)
        result = next(row for row in document["logics"] if row["logicId"] == box["onSuccessLogicIds"][0])
        result_id = result["logicId"]
        result.clear()
        result.update(logicId=result_id, displayName="Flip contacted card", logicType="RESULT", outcomeKind="PLAY_CONTACT_WORLD_OBJECT_MOTION",
                      contactMotions=[dict(targetWorldOccurrenceId=target, motionInstanceId="world.object.instance.kouku.card_flip") for target in candidates])
        signal_id = f"kakulsaydon.g1.logic.{document['nextLogicOrdinal']}"
        complete_id = f"kakulsaydon.g1.logic.{document['nextLogicOrdinal']+1}"
        followup_id = f"kakulsaydon.g1.logic.{document['nextLogicOrdinal']+2}"
        document["nextLogicOrdinal"] += 3
        signal_box = dict(occurrenceId=f"{pattern['patternId']}.logic.{pattern['nextLogicOccurrenceOrdinal']}",
                          logicId=signal_id, startMs=box["startMs"], durationMs=box["durationMs"],
                          onSuccessLogicIds=[followup_id], onFailLogicIds=[], onTimeoutLogicIds=["kakulsaydon.g1.logic.2"])
        pattern["nextLogicOccurrenceOrdinal"] += 1
        document["logics"].extend([
            dict(logicId=signal_id, displayName="Full deadline", logicType="DURATION", judgementKind="EXTERNAL_SIGNAL", endsPatternOnSuccess=True),
            dict(logicId=complete_id, displayName="Joker completes deadline", logicType="RESULT", outcomeKind="COMPLETE_LOGIC_WINDOW",
                 targetLogicOccurrenceId=signal_box["occurrenceId"], contactTargetWorldOccurrenceId=candidates[0]),
            dict(logicId=followup_id, displayName="Success followup", logicType="RESULT", outcomeKind="FOLLOWUP_PATTERN", followupPatternId=GAZE_ID),
        ])
        box["onSuccessLogicIds"].append(complete_id)
        box["durationMs"] = 100
        for collider in pattern["presentationOccurrences"]:
            if collider.get("logicOccurrenceId") == box["occurrenceId"]:
                collider["durationMs"] = box["durationMs"]
        pattern["logicOccurrences"].append(signal_box)
        return document

    def test_object_contact_projects_distinct_card_occurrences_and_full_deadline_signal(self):
        document = self.object_contact_document()
        self.validate(document)
        projected = self.first_product(subject.project_encounter(document))
        contact, signal = projected["logicWindows"]
        self.assertEqual("OBJECT_CONTACT", contact["kind"])
        self.assertEqual(2, len(contact["contactTargets"]))
        self.assertEqual(1, len({target["targetWorldInstanceId"] for target in contact["contactTargets"]}))
        self.assertEqual(2, len({target["targetWorldOccurrenceId"] for target in contact["contactTargets"]}))
        self.assertAlmostEqual(10, contact["contactTargets"][1]["targetWorldX"] - contact["contactTargets"][0]["targetWorldX"])
        self.assertEqual("PLAY_CONTACT_WORLD_OBJECT_MOTION", contact["onSuccess"][0]["kind"])
        self.assertEqual(signal["windowId"], contact["onSuccess"][1]["targetLogicOccurrenceId"])
        self.assertEqual("EXTERNAL_SIGNAL", signal["kind"])
        self.assertTrue(signal["endsPatternOnSuccess"])
        self.assertGreater(signal["durationMs"], contact["durationMs"])
        self.assertEqual("FOLLOWUP_PATTERN", signal["onSuccess"][0]["kind"])
        self.assertEqual([], contact["onTimeout"])
        self.assertEqual("INSTANT_DEATH", signal["onTimeout"][0]["kind"])

    def test_object_contact_rejects_missing_mapping_signal_and_short_target(self):
        for invalid in ("candidate", "mapping", "signal", "signal_kind", "signal_disabled", "filter", "lifetime", "timeout", "motion_twice"):
            document = self.object_contact_document()
            pattern = self.first_product(document)
            box, signal = pattern["logicOccurrences"]
            definitions = {row["logicId"]: row for row in document["logics"]}
            logic = definitions[box["logicId"]]
            motion, complete = [definitions[row] for row in box["onSuccessLogicIds"]]
            if invalid == "candidate": logic["targetWorldOccurrenceIds"][0] = "missing.world"
            elif invalid == "mapping": motion["contactMotions"].pop()
            elif invalid == "signal": complete["targetLogicOccurrenceId"] = "missing.logic"
            elif invalid == "signal_kind": complete["targetLogicOccurrenceId"] = box["occurrenceId"]
            elif invalid == "signal_disabled": signal["enabled"] = False
            elif invalid == "filter": complete["contactTargetWorldOccurrenceId"] = "missing.world"
            elif invalid == "lifetime": pattern["worldOccurrences"][0]["durationMs"] = 1
            elif invalid == "timeout": box["onTimeoutLogicIds"] = ["kakulsaydon.g1.logic.2"]
            else: box["onSuccessLogicIds"].append(box["onSuccessLogicIds"][0])
            with self.subTest(invalid=invalid), self.assertRaises(subject.CompositionError):
                self.validate(document)

    def test_object_contact_group_priority_and_repeated_strikes(self):
        document = self.object_contact_document()
        pattern = self.first_product(document)
        box = pattern["logicOccurrences"][0]
        sibling = dict(copy.deepcopy(box), occurrenceId=f"{pattern['patternId']}.logic.{pattern['nextLogicOccurrenceOrdinal']}")
        pattern["nextLogicOccurrenceOrdinal"] += 1
        definition = copy.deepcopy(next(row for row in document["logics"] if row["logicId"] == box["logicId"]))
        definition["logicId"] = f"kakulsaydon.g1.logic.{document['nextLogicOrdinal']}"
        document["nextLogicOrdinal"] += 1
        document["logics"].append(definition)
        sibling["logicId"] = definition["logicId"]
        pattern["logicOccurrences"].append(sibling)
        with self.assertRaisesRegex(subject.CompositionError, "distinct priorities"):
            self.validate(document)
        definition["contactPriority"] = 50
        self.validate(document)
        sibling["startMs"] += 1
        with self.assertRaisesRegex(subject.CompositionError, "identical timing"):
            self.validate(document)
        sibling["startMs"] += 100
        definition["contactPriority"] = 100
        self.validate(document)

    def test_object_contact_requires_static_target_and_same_object_motion(self):
        for invalid in ("moving_target", "wrong_object", "wrong_slot", "disabled_motion"):
            document = self.object_contact_document()
            sequences = copy.deepcopy(subject.load_json(ROOT / WORLD_SEQUENCES))
            target = next(row for row in sequences["instances"] if row["instanceId"] == "world.object.instance.kouku.card")
            motion = next(row for row in sequences["instances"] if row["instanceId"] == "world.object.instance.kouku.card_flip")
            if invalid == "moving_target":
                template = next(row for row in sequences["templates"] if row["sequenceId"] == target["templateId"])
                template["tracks"][0]["keys"][-1]["positionOffset"][0] += 1
            elif invalid == "wrong_object": motion["bindings"][0]["targetId"] = "missing.object"
            elif invalid == "wrong_slot": motion["bindings"][0]["slotId"] = "other.slot"
            else: motion["enabled"] = False
            with self.subTest(invalid=invalid), mock.patch.object(subject, "load_world_sequences", return_value=sequences), self.assertRaises(subject.CompositionError):
                subject.project_encounter(document)

    def bone_contact_document(self):
        document = self.object_contact_document()
        pattern = self.first_product(document)
        pattern.update(actorProfileId="MN_RPCT_06", gateId="GATE2", targetBossPlacementId="boss.kakulsaydon.g2.big-saydon")
        pattern.pop("folderId", None)
        stages = pattern["stages"][:2]
        for stage, suffix in zip(stages, ("8_01", "8_02")):
            stage["durationMs"] = 1001
            animation = stage["animationOccurrences"][0]
            animation.update(profileId="MN_RPCT_06", sourceActionId=0, sourceStageId="RAW", sourceSlotId="RAW",
                referenceRevision="", runtimeClip=f"mn_rpct_06_sk.ao_att_battle_{suffix}", startOffsetMs=0,
                sourceStartMs=0, playMs=1001, playRate=1, endPolicy="EXACT")
        pattern["stages"] = stages
        for box in pattern["logicOccurrences"] + pattern["worldOccurrences"]:
            box.update(startMs=0, durationMs=2002)
        pattern["logicOccurrences"][1]["onSuccessLogicIds"] = []
        contact = pattern["logicOccurrences"][0]
        colliders = [row for row in pattern["presentationOccurrences"] if row.get("logicOccurrenceId") == contact["occurrenceId"]]
        pattern["presentationOccurrences"] = colliders[:1]
        collider = pattern["presentationOccurrences"][0]
        collider.update(startMs=0, durationMs=2002, anchorKind="BOSS", followBoss=True, bone="b_rpct_01", boneTarget="WEAPON",
                        positionOffset=[1, 2, 3], rotationDegrees=[0, 27, 0], scale=[2, 2, 2])
        return document

    def placed_contact_document(self):
        document = self.object_contact_document()
        pattern = self.first_product(document)
        first, second = pattern["worldOccurrences"]
        second["worldId"] = first["worldId"]
        first["placement"] = dict(position=[10, 20, 30], rotationDegrees=[0, 90, 0], scale=[2, 3, 2])
        second["placement"] = dict(position=[30, 40, 50], rotationDegrees=[90, 0, 0], scale=[1, 2, 3])
        return document

    def object_placement_fixture(self):
        document = self.placed_contact_document()
        sequences = copy.deepcopy(subject.load_json(ROOT / WORLD_SEQUENCES))
        world_id = self.first_product(document)["worldOccurrences"][0]["worldId"]
        world = next(row for row in document["worlds"] if row["worldId"] == world_id)
        world["objectResourceId"] = "world.object.kouku.card"
        resource = next(row for row in sequences["objectResources"] if row["objectId"] == world["objectResourceId"])
        resource["defaultMotionInstanceId"] = "world.object.instance.kouku.card_flip"
        return document, sequences, world, resource

    def test_object_world_preserves_saved_initial_state_after_default_changes(self):
        document, sequences, world, resource = self.object_placement_fixture()
        before = copy.deepcopy(document)
        with mock.patch.object(subject, "load_world_sequences", return_value=sequences):
            self.validate(document)
            projected = self.first_product(subject.project_encounter(document))
            self.assertTrue(all(cue["sequenceInstanceId"] == world["sequenceInstanceId"] for cue in projected["worldSequences"]))
            self.assertNotEqual(resource["defaultMotionInstanceId"], world["sequenceInstanceId"])
            self.assertEqual(before, document)

    def test_object_world_rejects_missing_disabled_or_other_object_states(self):
        for invalid in ("object", "state", "disabled", "binding", "template", "default", "default_type", "default_other", "default_disabled"):
            document, sequences, world, resource = self.object_placement_fixture()
            selected = next(row for row in sequences["instances"] if row["instanceId"] == world["sequenceInstanceId"])
            if invalid == "object": world["objectResourceId"] = "missing.object"
            elif invalid == "state": world["sequenceInstanceId"] = "missing.state"
            elif invalid == "disabled": selected["enabled"] = False
            elif invalid == "binding": selected["bindings"][0]["targetId"] = "world.object.kouku.joker_card"
            elif invalid == "template": selected["templateId"] = "missing.template"
            elif invalid == "default": resource["defaultMotionInstanceId"] = "missing.default"
            elif invalid == "default_type": resource["defaultMotionInstanceId"] = None
            elif invalid == "default_other": resource["defaultMotionInstanceId"] = "world.object.instance.kouku.joker_card"
            else: next(row for row in sequences["instances"] if row["instanceId"] == resource["defaultMotionInstanceId"])["enabled"] = False
            with self.subTest(invalid=invalid), mock.patch.object(subject, "load_world_sequences", return_value=sequences):
                with self.assertRaises(subject.CompositionError): self.validate(document)
                with self.assertRaises(subject.CompositionError): subject.project_encounter(document)

    def test_seven_object_occurrences_preserve_placements_and_exact_contact_targets(self):
        document, sequences, world, resource = self.object_placement_fixture()
        pattern = self.first_product(document)
        original = copy.deepcopy(pattern["worldOccurrences"][0])
        joker_world = copy.deepcopy(world)
        joker_world.update(worldId=f"kakulsaydon.g1.world.{document['nextWorldOrdinal']}",
                           objectResourceId="world.object.kouku.joker_card", sequenceInstanceId="world.object.instance.kouku.joker_card")
        document["nextWorldOrdinal"] += 1
        document["worlds"].append(joker_world)
        pattern["worldOccurrences"] = []
        for index in range(7):
            cue = copy.deepcopy(original)
            cue.update(occurrenceId=f"{pattern['patternId']}.world.{pattern['nextWorldOccurrenceOrdinal']}",
                       worldId=(joker_world if index == 6 else world)["worldId"],
                       placement=dict(position=[10 * index, index, 100 + index], rotationDegrees=[0, index * 10, 0], scale=[1, 1 + index * .1, 1]))
            pattern["nextWorldOccurrenceOrdinal"] += 1
            pattern["worldOccurrences"].append(cue)
        contact = next(row for row in document["logics"] if row["logicId"] == pattern["logicOccurrences"][0]["logicId"])
        contact["targetWorldOccurrenceIds"] = [cue["occurrenceId"] for cue in pattern["worldOccurrences"]]
        motion = next(row for row in document["logics"] if row["logicId"] == pattern["logicOccurrences"][0]["onSuccessLogicIds"][0])
        motion["contactMotions"] = [dict(targetWorldOccurrenceId=cue["occurrenceId"], motionInstanceId=
            "world.object.instance.kouku.joker_card_flip" if index == 6 else "world.object.instance.kouku.card_flip")
            for index, cue in enumerate(pattern["worldOccurrences"])]
        signal = next(row for row in document["logics"] if row.get("outcomeKind") == "COMPLETE_LOGIC_WINDOW")
        signal["contactTargetWorldOccurrenceId"] = pattern["worldOccurrences"][-1]["occurrenceId"]
        for instance in sequences["instances"]:
            if instance["instanceId"] in {world["sequenceInstanceId"], joker_world["sequenceInstanceId"]}:
                instance["position"] = [9000, 9000, 9000]
                template = next(row for row in sequences["templates"] if row["sequenceId"] == instance["templateId"])
                for key in template["tracks"][0]["keys"]: key["positionOffset"] = [0, 0, 0]
        before = copy.deepcopy(document)
        with mock.patch.object(subject, "load_world_sequences", return_value=sequences):
            self.validate(document)
            product = self.first_product(subject.project_encounter(document))
        self.assertEqual(before, document)
        self.assertEqual(7, len(product["worldSequences"]))
        for cue, output, target in zip(pattern["worldOccurrences"], product["worldSequences"], product["logicWindows"][0]["contactTargets"]):
            self.assertEqual(cue["occurrenceId"], output["occurrenceId"])
            self.assertEqual(cue["placement"], output["placement"])
            self.assertEqual(cue["occurrenceId"], target["targetWorldOccurrenceId"])
            self.assertEqual(cue["placement"]["position"][0], target["targetWorldX"])
            self.assertEqual(cue["placement"]["position"][2], target["targetWorldZ"])
        self.assertEqual(motion["contactMotions"], product["logicWindows"][0]["onSuccess"][0]["contactMotions"])

    def test_world_placement_projects_independent_contact_centers_and_legacy_defaults(self):
        document = self.placed_contact_document()
        sequences = copy.deepcopy(subject.load_json(ROOT / WORLD_SEQUENCES))
        instance = next(row for row in sequences["instances"] if row["instanceId"] == "world.object.instance.kouku.card")
        instance["position"] = [1000, 1000, 1000]
        template = next(row for row in sequences["templates"] if row["sequenceId"] == instance["templateId"])
        for key in template["tracks"][0]["keys"]:
            key["positionOffset"] = [1, 2, 3]
        with mock.patch.object(subject, "load_world_sequences", return_value=sequences):
            self.validate(document)
            projected = self.first_product(subject.project_encounter(document))
            first, second = projected["logicWindows"][0]["contactTargets"]
            self.assertAlmostEqual(16, first["targetWorldX"])
            self.assertAlmostEqual(28, first["targetWorldZ"])
            self.assertAlmostEqual(31, second["targetWorldX"])
            self.assertAlmostEqual(54, second["targetWorldZ"])
            self.assertEqual([1.25, 1.25], [first["targetRadiusM"], second["targetRadiusM"]])
            for cue in projected["worldSequences"]:
                self.assertEqual([0, 0, 0], cue["positionOffset"])
                self.assertEqual([0, 0, 0], cue["anchorPosition"])
                self.assertEqual("NONE", cue["anchorKind"])
            for cue in self.first_product(document)["worldOccurrences"]:
                cue.pop("placement")
            legacy = self.first_product(subject.project_encounter(document))
            self.assertTrue(all("placement" not in cue for cue in legacy["worldSequences"]))
            self.assertAlmostEqual(1101, legacy["logicWindows"][0]["contactTargets"][0]["targetWorldX"])

    def test_anchored_object_placement_preserves_local_transform_without_fixed_collision_admission(self):
        for anchor in ("BOSS", "PLAYER"):
            sequences = copy.deepcopy(subject.load_json(ROOT / WORLD_SEQUENCES))
            instance = next(row for row in sequences["instances"] if row["instanceId"] == "world.object.instance.kouku.trumpet")
            resource = next(row for row in sequences["objectResources"] if row["objectId"] == instance["bindings"][0]["targetId"])
            instance["anchorKind"] = resource["anchorKind"] = anchor
            world = dict(sequenceInstanceId=instance["instanceId"], anchorKind="NONE", positionOffset=[0, 0, 0])
            cue = dict(startMs=0, durationMs=1000, placement=dict(position=[.3, .2, -.1], rotationDegrees=[90, 180, 45], scale=[2, 1, 3]))
            with self.subTest(anchor=anchor):
                projected = subject._project_world_placement(cue, world, sequences)
                self.assertEqual(cue["placement"], projected["placement"])
                self.assertEqual("NONE", projected["anchorKind"])
                with self.assertRaisesRegex(subject.CompositionError, "fixed WORLD anchor"):
                    subject._project_fixed_object_target(cue, world, dict(startMs=0, durationMs=1000), 1, sequences)
                resource["anchorKind"] = "WORLD"
                with self.assertRaises(subject.CompositionError):
                    subject._project_world_placement(cue, world, sequences)

    def test_world_placement_rejects_invalid_transform_and_non_object_anchor(self):
        for invalid in ("null", "field", "position", "rotation", "scale", "boolean", "binding", "anchor", "disabled"):
            document = self.placed_contact_document()
            cue = self.first_product(document)["worldOccurrences"][0]
            sequences = copy.deepcopy(subject.load_json(ROOT / WORLD_SEQUENCES))
            instance = next(row for row in sequences["instances"] if row["instanceId"] == "world.object.instance.kouku.card")
            if invalid == "null": cue["placement"] = None
            elif invalid == "field": cue["placement"]["unexpected"] = 1
            elif invalid == "position": cue["placement"]["position"][0] = 100001
            elif invalid == "rotation": cue["placement"]["rotationDegrees"][0] = 36001
            elif invalid == "scale": cue["placement"]["scale"][0] = 0
            elif invalid == "boolean": cue["placement"]["position"][0] = True
            elif invalid == "binding": instance["bindings"][0]["targetKind"] = "MAP_PLACEMENT"
            elif invalid == "disabled": instance["enabled"] = False
            else: instance["anchorKind"] = "PLAYER"
            with self.subTest(invalid=invalid), mock.patch.object(subject, "load_world_sequences", return_value=sequences), self.assertRaises(subject.CompositionError):
                self.validate(document)

    def test_world_placement_collider_resolves_exact_owner_and_same_track_transform(self):
        document = self.placed_contact_document()
        pattern = self.first_product(document)
        first, second = pattern["worldOccurrences"]
        second["placement"] = copy.deepcopy(first["placement"])
        second["placement"]["position"] = [40, 50, 60]
        contact = pattern["logicOccurrences"][0]
        collider = next(row for row in pattern["presentationOccurrences"] if row.get("logicOccurrenceId") == contact["occurrenceId"])
        pattern["presentationOccurrences"] = [collider]
        collider.update(anchorKind="WORLD", worldId=second["worldId"], worldOccurrenceId=second["occurrenceId"])
        sequences = copy.deepcopy(subject.load_json(ROOT / WORLD_SEQUENCES))
        instance = next(row for row in sequences["instances"] if row["instanceId"] == "world.object.instance.kouku.card")
        resource = next(row for row in sequences["objectResources"] if row["objectId"] == instance["bindings"][0]["targetId"])
        template = next(row for row in sequences["templates"] if row["sequenceId"] == instance["templateId"])
        for key in template["tracks"][0]["keys"]:
            key["positionOffset"] = [1, 2, 3]
        with mock.patch.object(subject, "load_world_sequences", return_value=sequences):
            self.validate(document)
            track = self.first_product(subject.project_encounter(document))["logicWindows"][0]["cardRegions"][0]["worldTrack"]
            self.assertEqual([40, 50, 60], track["baselinePosition"])
            self.assertEqual(90, track["baselineYawDegrees"])
            self.assertEqual([a*b for a,b in zip(resource["scale"], [2, 3, 2])], track["baselineScale"])
            self.assertEqual([2, 6, 6], track["keys"][0]["positionOffset"])
            collider["worldOccurrenceId"] = ""
            with self.assertRaisesRegex(subject.CompositionError, "exact same-pattern"):
                self.validate(document)
            pattern["worldOccurrences"].remove(first)
            # Resolve the legacy single-owner field to its stable occurrence in Product presentation.
            self.assertEqual(second["occurrenceId"], self.first_product(subject.project_presentation(document))["presentationOccurrences"][0]["worldOccurrenceId"])
            second["placement"]["rotationDegrees"][0] = 10
            with self.assertRaisesRegex(subject.CompositionError, "yaw-only"):
                subject._project_region_world_track(ROOT, subject.AREA_ID, sequences, next(row for row in document["worlds"] if row["worldId"] == second["worldId"]), second)

    def test_world_placement_bootstrap_sidecar_and_canonical_legacy_fields(self):
        projected = self.first_product(subject.project_encounter(self.placed_contact_document()))
        publisher = (ROOT / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1").read_text(encoding="utf-8-sig")
        functions = [re.search(r"(?ms)^function " + name + r"\b.*?^\}", publisher).group(0)
                     for name in ("Assert-ExactProperties", "Assert-StableId", "Assert-JsonString", "Assert-JsonInteger", "Assert-JsonNumber", "Format-InvariantFloat", "Format-InvariantSignedFloat")]
        start = publisher.index("\tforeach ($worldSequence in @($koukuPattern.worldSequences))")
        end = publisher.index("\tforeach ($sceneProfile in @($koukuPattern.sceneProfiles))", start)
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)
            script = "$ErrorActionPreference='Stop'\n$stableIdPattern='^[A-Za-z0-9_.-]+$'\n" + "\n".join(functions)
            script += "\n$koukuPattern=Get-Content -Raw (Join-Path $PSScriptRoot 'pattern.json') | ConvertFrom-Json\n"
            script += "$koukuPatternDurationMs=600000; $koukuEncounterDocument=@{encounterId='encounter.test'}; $patternRows=[Collections.Generic.List[string]]::new()\n"
            script += publisher[start:end] + "\nConvertTo-Json -InputObject @($patternRows) -Compress\n"
            check = folder / "check.ps1"; check.write_text(script, encoding="utf-8-sig")
            def run(value):
                (folder / "pattern.json").write_text(json.dumps(value), encoding="utf-8")
                return subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(check)], capture_output=True, text=True, timeout=30)
            result = run(projected)
            self.assertEqual(0, result.returncode, result.stderr)
            sidecars = [row.split("\t") for row in json.loads(result.stdout) if row.startswith("PATTERNWORLDPLACEMENT\t")]
            self.assertEqual([13, 13], [len(row) for row in sidecars])
            self.assertEqual([row["occurrenceId"] for row in projected["worldSequences"]], [row[3] for row in sidecars])
            for invalid in ("offset", "anchor", "scale", "rotation", "shape", "null", "surface"):
                value = copy.deepcopy(projected); cue = value["worldSequences"][0]
                if invalid == "offset": cue["positionOffset"][0] = 1
                elif invalid == "anchor": cue["anchorKind"] = "BOSS_SPAWN"
                elif invalid == "scale": cue["placement"]["scale"][0] = 1001
                elif invalid == "rotation": cue["placement"]["rotationDegrees"][0] = 36001
                elif invalid == "shape": cue["placement"]["position"].pop()
                elif invalid == "null": cue["placement"] = None
                else: cue["walkableSurface"] = {}
                with self.subTest(invalid=invalid):
                    self.assertNotEqual(0, run(value).returncode)

    def test_object_contact_rejects_invalid_collider_anchors(self):
        for invalid in ("world_bone", "followBoss", "boneTarget"):
            document = self.object_contact_document()
            pattern = self.first_product(document)
            contact = pattern["logicOccurrences"][0]
            collider = next(row for row in pattern["presentationOccurrences"] if row.get("logicOccurrenceId") == contact["occurrenceId"])
            if invalid == "world_bone": collider.update(anchorKind="WORLD", bone="hammer.bone")
            elif invalid == "followBoss": collider["followBoss"] = False
            else: collider["boneTarget"] = ["BODY"]
            with self.subTest(invalid=invalid), self.assertRaises(subject.CompositionError):
                self.validate(document)

    def test_bone_contact_bakes_real_hammer_and_body_with_target_yaw(self):
        document = self.bone_contact_document()
        self.validate(document)
        projected = self.first_product(subject.project_encounter(document))
        region = projected["logicWindows"][0]["cardRegions"][0]
        self.assertEqual("BOSS_CURRENT", region["anchorKind"])
        self.assertEqual([1, 2, 3], region["center"])
        self.assertEqual(27, region["yawDegrees"])
        track = region["worldTrack"]
        self.assertEqual([0, 2002], [track["keys"][0]["timeMs"], track["keys"][-1]["timeMs"]])
        self.assertGreater(len(track["keys"]), 60)
        positions = [key["positionOffset"] for key in track["keys"]]
        self.assertGreater(max(math.dist(positions[0], value) for value in positions), 1)
        self.assertTrue(all(key["rotationY"] == 0 and key["rotationW"] == 1 and key["scaleMultiplier"] == [1, 1, 1] for key in track["keys"]))
        collider = self.first_product(document)["presentationOccurrences"][0]
        collider.update(boneTarget="BODY", bone="b_wp_1")
        body = self.first_product(subject.project_encounter(document))["logicWindows"][0]["cardRegions"][0]["worldTrack"]
        self.assertNotEqual(track["keys"][0]["positionOffset"], body["keys"][0]["positionOffset"])
        bindings = subject.project_presentation(document)["bindings"]
        action_ids = {row["actionId"] for row in self.first_product(document)["stages"]}
        self.assertTrue(all(row.get("unblendedBoneContact", False) for row in bindings if row["actionId"] in action_ids))
        self.assertTrue(all("unblendedBoneContact" not in row for row in bindings if row["actionId"] not in action_ids))
        self.first_product(document)["logicOccurrences"][0]["enabled"] = False
        self.assertTrue(all("unblendedBoneContact" not in row for row in subject.project_presentation(document)["bindings"]))

    def test_bone_contact_uses_quantized_stage_origin_and_tick_brackets(self):
        document = self.bone_contact_document()
        pattern = self.first_product(document)
        # 1001 ms rounds up to 31 ticks; the second action starts at tick 30.
        before, seconds = subject._bone_bake_clip_sample(pattern, 999)
        after, zero = subject._bone_bake_clip_sample(pattern, 1000)
        self.assertTrue(before["runtimeClip"].endswith("8_01"))
        self.assertTrue(after["runtimeClip"].endswith("8_02"))
        self.assertEqual(0, zero)
        track = self.first_product(subject.project_encounter(document))["logicWindows"][0]["cardRegions"][0]["worldTrack"]
        keys = {key["timeMs"]: key["positionOffset"] for key in track["keys"]}
        self.assertEqual(keys[33], keys[34])
        self.assertEqual(keys[1066], keys[1067])
        self.assertNotEqual(keys[999], keys[1000])
        pattern["stages"][0]["durationMs"] = 1000
        pattern["stages"][0]["animationOccurrences"][0]["playMs"] = 1000
        shifted = self.first_product(subject.project_encounter(document))["logicWindows"][0]["cardRegions"][0]["worldTrack"]
        shifted_keys = {key["timeMs"]: key["positionOffset"] for key in shifted["keys"]}
        self.assertEqual(shifted_keys[966], shifted_keys[967])
        last, held_seconds = subject._bone_bake_clip_sample(pattern, 2000)
        self.assertTrue(last["runtimeClip"].endswith("8_02"))
        self.assertGreater(held_seconds, last["playMs"] / 1000)

    def test_bone_contact_rejects_missing_bone_clip_asset_and_sample_gap(self):
        for invalid in ("bone", "clip", "asset", "gap", "actor", "limit"):
            document = self.bone_contact_document()
            pattern = self.first_product(document)
            collider = pattern["presentationOccurrences"][0]
            animation = pattern["stages"][0]["animationOccurrences"][0]
            if invalid == "bone": collider["bone"] = "missing.hammer.tip"
            elif invalid == "clip": animation["runtimeClip"] = "missing.clip"
            elif invalid == "gap": animation["startOffsetMs"] = 10
            elif invalid == "actor": animation["profileId"] = "MN_RPCZ_00"
            elif invalid == "limit":
                for box in pattern["logicOccurrences"] + pattern["worldOccurrences"] + pattern["presentationOccurrences"]:
                    box["durationMs"] = 120000
            with self.subTest(invalid=invalid), self.assertRaises(subject.CompositionError):
                if invalid == "asset":
                    with mock.patch.object(subject.wmodel_pose, "read_wmodel", side_effect=OSError("missing asset")):
                        subject.project_encounter(document)
                else:
                    subject.project_encounter(document)

    def test_object_contact_bootstrap_owns_targets_motions_and_signal(self):
        projected = self.first_product(subject.project_encounter(self.object_contact_document()))
        publisher = (ROOT / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1").read_text(encoding="utf-8-sig")
        definitions = [re.search(r"(?ms)^function " + name + r"\b.*?^\}", publisher).group(0)
                       for name in ("Assert-ExactProperties", "Assert-StableId", "Assert-JsonString", "Assert-JsonInteger",
                                    "Assert-JsonNumber", "Format-InvariantFloat", "Format-InvariantSignedFloat")]
        start = publisher.index("\tif ($koukuPattern.logicWindows -isnot [Array]")
        end = publisher.index("\tif ($koukuPattern.mechanicTriggers", start)
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)
            source = folder / "pattern.json"
            script = "$ErrorActionPreference='Stop'\n$stableIdPattern='^[A-Za-z0-9_.-]+$'\n" + "\n".join(definitions)
            script += "\n$koukuPattern=Get-Content -Raw (Join-Path $PSScriptRoot 'pattern.json') | ConvertFrom-Json\n"
            script += "$koukuPatternDurationMs=600000; $koukuEncounterDocument=[pscustomobject]@{encounterId='encounter.test'}\n"
            script += "$patternRows=[Collections.Generic.List[string]]::new(); $koukuFollowupTargets=[Collections.Generic.List[string]]::new()\n"
            script += publisher[start:end] + "\nConvertTo-Json -InputObject @($patternRows) -Compress\n"
            path = folder / "validate.ps1"
            path.write_text(script, encoding="utf-8-sig")
            def run(value):
                source.write_text(json.dumps(value), encoding="utf-8")
                return subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(path)],
                                      capture_output=True, text=True, timeout=45)
            result = run(projected)
            self.assertEqual(0, result.returncode, result.stderr)
            rows = [row.split("\t") for row in json.loads(result.stdout)]
            targets = [row for row in rows if row[0] == "PATTERNLOGICCONTACTTARGET"]
            motions = [row for row in rows if row[0] == "PATTERNLOGICCONTACTMOTION"]
            signal = next(row for row in rows if row[0] == "PATTERNLOGICSIGNAL")
            self.assertEqual([9, 9], [len(row) for row in targets])
            self.assertEqual([8, 8], [len(row) for row in motions])
            self.assertEqual(8, len(signal))
            self.assertEqual({row[4] for row in targets}, {row[6] for row in motions})
            self.assertEqual(projected["logicWindows"][1]["windowId"], signal[6])
            self.assertEqual(targets[0][4], signal[7])
            self.assertEqual(["hammer.impact", "100"], next(row for row in rows if row[0] == "PATTERNLOGICCONTACTGROUP")[-2:])
            for invalid in ("target_owner", "target_lifetime", "mapping", "signal", "signal_lifetime", "timeout", "motion_twice", "priority", "group"):
                value = copy.deepcopy(projected)
                contact, deadline = value["logicWindows"]
                if invalid == "target_owner": contact["contactTargets"][0]["targetWorldInstanceId"] = "wrong.instance"
                elif invalid == "target_lifetime": value["worldSequences"][0]["durationMs"] = 1
                elif invalid == "mapping": contact["onSuccess"][0]["contactMotions"].pop()
                elif invalid == "signal": contact["onSuccess"][1]["targetLogicOccurrenceId"] = "missing.window"
                elif invalid == "signal_lifetime": deadline["durationMs"] = 1
                elif invalid == "timeout": contact["onTimeout"] = deadline["onTimeout"]
                elif invalid == "motion_twice": contact["onSuccess"].append(contact["onSuccess"][0])
                elif invalid == "priority": contact["contactPriority"] = True
                else:
                    sibling = copy.deepcopy(contact)
                    sibling["windowId"] += ".duplicate"
                    value["logicWindows"].append(sibling)
                with self.subTest(invalid=invalid):
                    self.assertNotEqual(0, run(value).returncode)
            ungrouped = copy.deepcopy(projected)
            ungrouped["logicWindows"][0]["contactGroupId"] = ""
            ungrouped["logicWindows"][0]["onSuccess"][1]["contactTargetWorldOccurrenceId"] = ""
            result = run(ungrouped)
            self.assertEqual(0, result.returncode, result.stderr)
            rows = [row.split("\t") for row in json.loads(result.stdout)]
            self.assertEqual("-", next(row for row in rows if row[0] == "PATTERNLOGICCONTACTGROUP")[-2])
            self.assertEqual("-", next(row for row in rows if row[0] == "PATTERNLOGICSIGNAL")[-1])
            capacity = copy.deepcopy(projected)
            while len(capacity["worldSequences"]) < 128:
                capacity["worldSequences"].append(dict(capacity["worldSequences"][-1], occurrenceId=f"world.capacity.{len(capacity['worldSequences'])}"))
            result = run(capacity)
            self.assertEqual(0, result.returncode, result.stderr)
            capacity["worldSequences"].append(dict(capacity["worldSequences"][-1], occurrenceId="world.capacity.129"))
            self.assertNotEqual(0, run(capacity).returncode)
            bone = self.first_product(subject.project_encounter(self.bone_contact_document()))
            result = run(bone)
            self.assertEqual(0, result.returncode, result.stderr)
            bone_rows = [row.split("\t") for row in json.loads(result.stdout)]
            self.assertTrue(any(row[0] == "PATTERNLOGICREGIONWORLDKEY" for row in bone_rows))
            for invalid in ("baseline", "clock", "rotation", "scale", "hidden", "endpoint"):
                value = copy.deepcopy(bone)
                track = value["logicWindows"][0]["cardRegions"][0]["worldTrack"]
                if invalid == "baseline": track["baselinePosition"][0] = 1
                elif invalid == "clock": track["startDelayMs"] = 1
                elif invalid == "rotation": track["keys"][0]["rotationY"] = .1
                elif invalid == "scale": track["keys"][0]["scaleMultiplier"] = [2, 2, 2]
                elif invalid == "hidden": track["keys"][0]["visible"] = False
                else: track["keys"].pop()
                with self.subTest(invalid=invalid):
                    self.assertNotEqual(0, run(value).returncode)

    def test_object_contact_world_capacity_allows_128_and_rejects_129(self):
        document = self.object_contact_document()
        pattern = self.first_product(document)
        while len(pattern["worldOccurrences"]) < 128:
            pattern["worldOccurrences"].append(dict(pattern["worldOccurrences"][-1],
                occurrenceId=f"{pattern['patternId']}.world.{pattern['nextWorldOccurrenceOrdinal']}"))
            pattern["nextWorldOccurrenceOrdinal"] += 1
        self.validate(document)
        self.assertEqual(128, len(self.first_product(subject.project_encounter(document))["worldSequences"]))
        pattern["worldOccurrences"].append(dict(pattern["worldOccurrences"][-1],
            occurrenceId=f"{pattern['patternId']}.world.{pattern['nextWorldOccurrenceOrdinal']}"))
        pattern["nextWorldOccurrenceOrdinal"] += 1
        with self.assertRaises(subject.CompositionError):
            self.validate(document)

    def test_object_contact_rejects_invalid_definition_values(self):
        for invalid in ("targets_type", "targets_empty", "target_type", "radius", "priority", "empty_group", "mapping_type", "signal_target_type"):
            document = self.object_contact_document()
            box = self.first_product(document)["logicOccurrences"][0]
            definitions = {row["logicId"]: row for row in document["logics"]}
            logic = definitions[box["logicId"]]
            motion, signal = [definitions[row] for row in box["onSuccessLogicIds"]]
            if invalid == "targets_type": logic["targetWorldOccurrenceIds"] = "card"
            elif invalid == "targets_empty": logic["targetWorldOccurrenceIds"] = []
            elif invalid == "target_type": logic["targetWorldOccurrenceIds"][0] = []
            elif invalid == "radius": logic["targetRadiusM"] = 0
            elif invalid == "priority": logic["contactPriority"] = True
            elif invalid == "empty_group": logic["contactGroupId"] = ""
            elif invalid == "mapping_type": motion["contactMotions"][0] = []
            else: signal["contactTargetWorldOccurrenceId"] = []
            with self.subTest(invalid=invalid), self.assertRaises(subject.CompositionError):
                self.validate(document)

    def test_repeated_card_placements_reject_legacy_instance_motion_binding(self):
        document = self.object_overlap_document()
        pattern = self.first_product(document)
        cue = dict(pattern["worldOccurrences"][-1], occurrenceId=f"{pattern['patternId']}.world.{pattern['nextWorldOccurrenceOrdinal']}")
        pattern["nextWorldOccurrenceOrdinal"] += 1
        pattern["worldOccurrences"].append(cue)
        with self.assertRaisesRegex(subject.CompositionError, "ambiguous"):
            self.validate(document)
        with self.assertRaisesRegex(subject.CompositionError, "exactly one target"):
            subject.project_encounter(document)

    def test_object_overlap_projects_same_card_motion_and_absolute_target_circle(self):
        document = self.object_overlap_document()
        self.validate(document)
        window = self.first_product(subject.project_encounter(document))["logicWindows"][0]
        self.assertEqual("OBJECT_OVERLAP", window["kind"])
        self.assertAlmostEqual(103.28999996, window["targetWorldX"])
        self.assertAlmostEqual(891.31000042, window["targetWorldZ"])
        self.assertEqual(1.25, window["targetRadiusM"])
        self.assertEqual("world.object.instance.kouku.card", window["onSuccess"][0]["targetWorldInstanceId"])
        self.assertEqual("world.object.instance.kouku.card_flip", window["onSuccess"][0]["motionInstanceId"])

    def test_object_overlap_fail_motion_emits_valid_publisher_rows(self):
        document = self.object_overlap_document()
        pattern = self.first_product(document)
        box = pattern["logicOccurrences"][0]
        logic = next(row for row in document["logics"] if row["logicId"] == box["logicId"])
        logic["insideOutcome"] = "FAIL"
        box["onFailLogicIds"] = box["onSuccessLogicIds"]
        box["onSuccessLogicIds"] = []
        self.validate(document)
        projected = self.first_product(subject.project_encounter(document))
        window = projected["logicWindows"][0]
        self.assertEqual("FAIL", window["insideOutcome"])
        self.assertEqual("PLAY_WORLD_OBJECT_MOTION", window["onFail"][0]["kind"])
        publisher = (ROOT / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1").read_text(encoding="utf-8-sig")
        definitions = []
        for name in ("Assert-ExactProperties", "Assert-StableId", "Assert-JsonString", "Assert-JsonInteger", "Assert-JsonNumber", "Format-InvariantFloat", "Format-InvariantSignedFloat"):
            definitions.append(re.search(r"(?ms)^function " + name + r"\b.*?^\}", publisher).group(0))
        start = publisher.index("\t$koukuWindowIds =")
        end = publisher.index("\tif ($koukuPattern.mechanicTriggers", start)
        validate_windows = publisher[start:end]
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)
            (folder / "pattern.json").write_text(json.dumps(projected), encoding="utf-8")
            script = "$ErrorActionPreference='Stop'\n$stableIdPattern='^[A-Za-z0-9_.-]+$'\n" + "\n".join(definitions)
            script += "\n$koukuPattern=Get-Content -Raw (Join-Path $PSScriptRoot 'pattern.json') | ConvertFrom-Json\n"
            script += "$koukuPatternDurationMs=600000; $koukuEncounterDocument=[pscustomobject]@{encounterId='encounter.test'}\n"
            script += "$patternRows=[Collections.Generic.List[string]]::new(); $koukuFollowupTargets=[Collections.Generic.List[string]]::new()\n"
            script += validate_windows
            script += "\nConvertTo-Json -InputObject @($patternRows) -Compress\n"
            path = folder / "validate.ps1"
            path.write_text(script, encoding="utf-8-sig")
            result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(path)],
                                    capture_output=True, text=True, timeout=45)
            self.assertEqual(0, result.returncode, result.stderr)
            rows = [row.split("\t") for row in json.loads(result.stdout)]
            logic_row = next(row for row in rows if row[0] == "PATTERNLOGIC")
            outcome_row = next(row for row in rows if row[0] == "PATTERNLOGICOUTCOME")
            self.assertEqual(26, len(logic_row))
            self.assertEqual(["world.object.instance.kouku.card", "103.28999996", "891.31000042", "1.25"], logic_row[-4:])
            self.assertEqual(12, len(outcome_row))
            self.assertEqual("FAIL", outcome_row[4])
            self.assertEqual(["world.object.instance.kouku.card", "world.object.instance.kouku.card_flip"], outcome_row[-2:])

    def test_object_overlap_rejects_missing_target_lifetime_and_different_object_motion(self):
        document = self.object_overlap_document()
        self.first_product(document)["worldOccurrences"][-1]["durationMs"] -= 1
        with self.assertRaisesRegex(subject.CompositionError, "lifetime"):
            subject.project_encounter(document)
        document = self.object_overlap_document()
        document["logics"][-1]["motionInstanceId"] = "world.object.instance.kouku.joker_card_flip"
        with self.assertRaisesRegex(subject.CompositionError, "same World Object"):
            self.validate(document)

        for case in ("disabled_target", "disabled_motion", "different_binding_slot"):
            with self.subTest(case=case):
                document = self.object_overlap_document()
                sequences = subject.load_world_sequences(ROOT, subject.AREA_ID)
                target = next(row for row in sequences["instances"] if row["instanceId"] == "world.object.instance.kouku.card")
                motion = next(row for row in sequences["instances"] if row["instanceId"] == "world.object.instance.kouku.card_flip")
                reason = "disabled"
                if case == "disabled_target":
                    target["enabled"] = False
                elif case == "disabled_motion":
                    motion["enabled"] = False
                else:
                    reason = "same slotId"
                    motion["bindings"][0]["slotId"] = "different_slot"
                    template = next(row for row in sequences["templates"] if row["sequenceId"] == motion["templateId"])
                    for track in template.get("tracks", []) + template.get("animationTracks", []):
                        track["slotId"] = "different_slot"
                with mock.patch.object(subject, "load_world_sequences", return_value=sequences):
                    with self.assertRaisesRegex(subject.CompositionError, reason):
                        self.validate(document)
                    with self.assertRaisesRegex(subject.CompositionError, reason):
                        subject.project_encounter(document)

    def test_object_overlap_rejects_moving_target_and_projects_object_source_world_track(self):
        document = self.object_overlap_document()
        sequences = subject.load_world_sequences(ROOT, subject.AREA_ID)
        instance = next(r for r in sequences["instances"] if r["instanceId"] == "world.object.instance.kouku.card")
        template = next(r for r in sequences["templates"] if r["sequenceId"] == instance["templateId"])
        template["objectMotion"]["velocity"][0] = 1
        with mock.patch.object(subject, "load_world_sequences", return_value=sequences):
            with self.assertRaisesRegex(subject.CompositionError, "zero physical Object Motion"):
                subject.project_encounter(document)
        template["objectMotion"]["velocity"][0] = 0
        for motion_end in ("STOP", "NEXT"):
            with self.subTest(motion_end=motion_end):
                invalid = copy.deepcopy(sequences)
                target = next(row for row in invalid["instances"] if row["instanceId"] == instance["instanceId"])
                target["motionEnd"] = motion_end
                with mock.patch.object(subject, "load_world_sequences", return_value=invalid):
                    with self.assertRaisesRegex(subject.CompositionError, "LOOP or HOLD"):
                        subject.project_encounter(document)
        for successor_case in ("valid_idle", "moving_successor", "disabled_successor"):
            with self.subTest(successor_case=successor_case):
                chain = copy.deepcopy(sequences)
                flip = next(row for row in chain["instances"] if row["instanceId"] == "world.object.instance.kouku.card_flip")
                hop = next(row for row in chain["instances"] if row["instanceId"] == "world.object.instance.kouku.card_hop")
                flip.update(motionEnd="NEXT", nextMotionId=hop["instanceId"])
                hop.update(motionEnd="NEXT", nextMotionId=instance["instanceId"])
                if successor_case == "moving_successor":
                    hop_template = next(row for row in chain["templates"] if row["sequenceId"] == hop["templateId"])
                    hop_template["tracks"][0]["keys"][-1]["positionOffset"][0] = 1
                elif successor_case == "disabled_successor":
                    hop["enabled"] = False
                with mock.patch.object(subject, "load_world_sequences", return_value=chain):
                    if successor_case == "valid_idle":
                        subject.project_encounter(document)
                    else:
                        with self.assertRaisesRegex(subject.CompositionError, "preserve fixed target geometry|disabled"):
                            subject.project_encounter(document)
        motion = next(row for row in sequences["instances"] if row["instanceId"] == "world.object.instance.kouku.card_flip")
        motion_template = next(row for row in sequences["templates"] if row["sequenceId"] == motion["templateId"])
        motion_template["tracks"][0]["keys"][-1]["positionOffset"][0] = 1
        with mock.patch.object(subject, "load_world_sequences", return_value=sequences):
            with self.assertRaisesRegex(subject.CompositionError, "preserve fixed target geometry"):
                subject.project_encounter(document)
        world = dict(sequenceInstanceId=instance["instanceId"], positionOffset=[100,0,900])
        track = subject._project_region_world_track(ROOT, subject.AREA_ID, sequences, world,
                                                    dict(startMs=0, playbackSpeed=1))
        self.assertEqual([103.28999996, 8.64000034, 891.31000042], track["baselinePosition"])

    def test_stagger_effects_expand_every_authored_disarm_child_without_a_duplicate_group(self):
        document = copy.deepcopy(self.document)
        pattern = self.first_product(document)
        resources = {row["resourceId"]: row for row in document["presentationResources"]}
        # Reusable group defaults do not override a saved occurrence's transform.
        override = next(row for row in pattern["presentationOccurrences"]
                        if resources[row["resourceId"]]["kind"] == "EFFECT")
        override.update(positionOffset=[1.25, .75, -2], rotationDegrees=[5, 90, -15], scale=[1.5, 2, .5])
        presentation = self.find(subject.project_presentation(document), FIRST_PRODUCT_ID)
        projected_effects = {row["occurrenceId"]: row for row in presentation["presentationOccurrences"]
                             if row["kind"] == "EFFECT"}
        effects = {row["occurrenceId"]: row for row in pattern["presentationOccurrences"]
                   if resources[row["resourceId"]]["kind"] == "EFFECT"}
        group = subject.load_json(ROOT / "Data/Effects/V2/Groups/boss.kouku.disarm.effectv2group.json")
        window = pattern["logicOccurrences"][0]
        self.assertEqual(len(group["children"]), len(effects))
        for child in group["children"]:
            ordinal = int(child["childId"].rsplit(".", 1)[1]) + 1
            row = effects[f"{FIRST_PRODUCT_ID}.presentation.{ordinal}"]
            resource = resources[row["resourceId"]]
            self.assertEqual(("LEAF", child["resource"]["id"]), (resource["resourceKind"], resource["assetId"]))
            projected_effect = projected_effects[row["occurrenceId"]]
            for field in ("positionOffset", "rotationDegrees", "scale"):
                self.assertEqual(row[field], projected_effect[field], (row["occurrenceId"], field))
            self.assertEqual(window["startMs"] + child["startMs"], row["startMs"])
            self.assertLessEqual(row["startMs"] + row["durationMs"], window["startMs"] + window["durationMs"])
            self.assertEqual(("BOSS", "", 0, 0), (row["anchorKind"], row["bone"], row["fadeInMs"], row["fadeOutMs"]))
            if resource["assetId"] == "boss.kouku.disarm.star.smoke_1":
                leaf = subject.load_json(ROOT / "Data/Effects/V2/Authored/boss.kouku.disarm.star.smoke_1.effectv2.json")
                emission = leaf["params"]["lifetime"]
                tail = max(leaf["params"]["particle"]["lifetime"])
                required_ms = round((emission + tail) * 1000 / leaf["params"]["playRate"])
                self.assertEqual(required_ms, row["durationMs"])
                self.assertFalse(leaf["params"]["loop"])
        encounter = subject.project_encounter(copy.deepcopy(self.document))
        projected = self.find(encounter, FIRST_PRODUCT_ID)
        shield = next(row for row in projected["logicWindows"] if row["windowId"] == window["occurrenceId"])
        authored_shields = [row for row in pattern["presentationOccurrences"]
                            if row.get("logicOccurrenceId") == window["occurrenceId"]
                            and resources[row["resourceId"]]["kind"] == "COLLIDER"]
        self.assertEqual(2, len(authored_shields))
        self.assertEqual([(row["positionOffset"], row["rotationDegrees"][1]) for row in authored_shields],
                         [(row["center"], row["yawDegrees"]) for row in shield["cardRegions"]])
        self.assertTrue(all(row["anchorKind"] == "BOSS_CURRENT" and row["shape"] == "SECTOR"
                            for row in shield["cardRegions"]))
        self.assertEqual(71.737272, shield["shieldArcDegrees"])
        for row in shield["cardRegions"]:
            self.assertEqual(35.868636, row["halfAngleDegrees"])
            self.assertEqual(2.148847, row["radiusM"])

    def test_stagger_reflection_rejects_a_non_sector_collider(self):
        document = copy.deepcopy(self.document)
        pattern = self.first_product(document)
        collider = next(row for row in pattern["presentationOccurrences"] if row.get("logicOccurrenceId"))
        resource = next(row for row in document["presentationResources"] if row["resourceId"] == collider["resourceId"])
        resource["shape"] = "BOX"
        with self.assertRaisesRegex(subject.CompositionError, "boss-pivot SECTOR"):
            subject.project_encounter(document)

    def test_stagger_reflection_rejects_a_detached_visual_collider(self):
        document = copy.deepcopy(self.document)
        pattern = self.first_product(document)
        collider = next(row for row in pattern["presentationOccurrences"] if row.get("logicOccurrenceId"))
        collider["followBoss"] = False
        with self.assertRaisesRegex(subject.CompositionError, "following boss-pivot SECTOR"):
            subject.project_encounter(document)

    def test_live_document_pins_the_gate1_saydon_products(self):
        self.validate(copy.deepcopy(self.document))
        self.assertGreaterEqual(self.document["revision"], 51)
        self.assertEqual(3, self.document["formatVersion"])
        self.assertEqual(
            ["KAKULSAYDON_G1_PATTERN_1", "KAKULSAYDON_G1_PATTERN_2", "KAKULSAYDON_G1_PATTERN_4",
             "KAKULSAYDON_G1_PATTERN_5", "KAKULSAYDON_G1_PATTERN_6", "KAKULSAYDON_G1_PATTERN_7"],
            self.document["playAllPatternIds"],
        )
        for pattern in self.product_patterns(self.document):
            self.assertEqual("MN_RPCT_05", pattern["actorProfileId"])
            self.assertEqual("MECHANIC", pattern["category"])
            for stage in pattern["stages"]:
                self.assertEqual(1, len(stage["animationOccurrences"]))
                self.assertEqual("EXACT", stage["animationOccurrences"][0]["endPolicy"])
        self.assertNotIn("KAKULSAYDON_G1_PIZZA", [p["patternId"] for p in self.document["patterns"]])
        self.assertEqual("DRAFT", self.draft(self.document)["authoringStatus"])
        self.assertEqual({"maximum": 100, "clownHoldMs": 15000}, self.document["madnessPolicy"])
        self.assertEqual(
            {"월드_룰렛": "world.sequence.instance.8", "월드_커튼": "world.sequence.instance.curtain_drop"},
            {row["displayName"]: row["sequenceInstanceId"] for row in self.document["worlds"]},
        )
        self.assertEqual(
            {"씬프로필_암전": "scene.kakulsaydon.find-true-dark.v1"},
            {row["displayName"]: row["renderingProfileId"] for row in self.document["sceneProfiles"]},
        )
        roulette = self.find(self.document, ROULETTE_ID)
        self.assertEqual(4, len(roulette["logicOccurrences"]))
        self.assertEqual(1, len(roulette["worldOccurrences"]))
        self.assertEqual(1, len(self.find(self.document, DANCE_ID)["worldOccurrences"]))
        self.assertEqual(1, len(self.find(self.document, GAZE_ID)["sceneProfileOccurrences"]))
        resources = {row["resourceId"]: row for row in self.document["presentationResources"]}
        for pattern_id in (GAZE_ID, DANCE_ID, ROULETTE_ID):
            with self.subTest(pattern=pattern_id):
                pattern = self.find(self.document, pattern_id)
                self.assertEqual(1, len(pattern["sceneProfileOccurrences"]))
                scene = pattern["sceneProfileOccurrences"][0]
                lights = [row for row in pattern["presentationOccurrences"]
                          if resources[row["resourceId"]]["kind"] == "LIGHT"]
                self.assertEqual(2, len(lights))
                self.assertEqual({("light.runtime.1", "PLAYER"), ("light.runtime.2", "BOSS")},
                                 {(resources[row["resourceId"]]["assetId"], row["anchorKind"]) for row in lights})
                for row in lights:
                    self.assertEqual((scene["startMs"], scene["durationMs"]),
                                     (row["startMs"], row["durationMs"]))
                    self.assertTrue(row["followBoss"])

    def test_rejects_unknown_fields_versions_ids_and_non_integer_revision(self):
        mutations = []
        extra = copy.deepcopy(self.document)
        extra["futureFamilies"] = []
        mutations.append(extra)
        wrong_version = copy.deepcopy(self.document)
        wrong_version["formatVersion"] = subject.FORMAT_VERSION + 1
        mutations.append(wrong_version)
        wrong_id = copy.deepcopy(self.document)
        wrong_id["bossPlacementId"] = "boss.kakulsaydon.g1.other"
        mutations.append(wrong_id)
        boolean_revision = copy.deepcopy(self.document)
        boolean_revision["revision"] = True
        mutations.append(boolean_revision)
        for mutation in mutations:
            with self.subTest(mutation=mutation):
                with self.assertRaises(subject.CompositionError):
                    self.validate(mutation)

    def test_rejects_duplicate_and_ahead_of_counter_identities(self):
        duplicate = copy.deepcopy(self.document)
        duplicate["patterns"].append(copy.deepcopy(self.first_product(duplicate)))
        duplicate["playAllPatternIds"].append(FIRST_PRODUCT_ID)
        with self.assertRaisesRegex(subject.CompositionError, "duplicate patternId"):
            self.validate(duplicate)

        pattern_counter = copy.deepcopy(self.document)
        ahead_id = f"KAKULSAYDON_G1_PATTERN_{pattern_counter['nextPatternOrdinal']}"
        self.draft(pattern_counter)["patternId"] = ahead_id
        with self.assertRaisesRegex(subject.CompositionError, "nextPatternOrdinal"):
            self.validate(pattern_counter)

        occurrence_counter = copy.deepcopy(self.document)
        self.first_product(occurrence_counter)["nextAnimationOrdinal"] = 2
        with self.assertRaisesRegex(subject.CompositionError, "nextAnimationOrdinal"):
            self.validate(occurrence_counter)

    def test_draft_may_be_empty_but_never_projects_or_replaces_product_inventory(self):
        document = copy.deepcopy(self.document)
        draft = self.strip_lanes(copy.deepcopy(self.first_product(document)))
        draft_ordinal = document["nextPatternOrdinal"]
        draft.update(
            patternId=f"KAKULSAYDON_G1_PATTERN_{draft_ordinal}",
            displayName="초안",
            authoringStatus="DRAFT",
            nextStageOrdinal=1,
            nextAnimationOrdinal=1,
            stages=[],
        )
        document["nextPatternOrdinal"] = draft_ordinal + 1
        document["patterns"].append(draft)
        self.validate(document)
        encounter = subject.project_encounter(document)
        presentation = subject.project_presentation(document)
        self.assertEqual(self.document["playAllPatternIds"], encounter["playAllPatternIds"])
        self.assertEqual(len(self.document["playAllPatternIds"]), len(encounter["patterns"]))
        self.assertEqual(self.product_binding_count(self.document), len(presentation["bindings"]))

        for pattern in self.product_patterns(document):
            pattern["authoringStatus"] = "DRAFT"
        document["playAllPatternIds"] = []
        self.validate(document)
        with self.assertRaisesRegex(subject.CompositionError, "at least one PRODUCT"):
            subject.validate_publishable(document)

    def test_product_inventory_is_bounded_to_downstream_catalog_capacity(self):
        document = copy.deepcopy(self.document)
        seed = self.strip_lanes(copy.deepcopy(self.first_product(document)))
        document["patterns"] = []
        document["playAllPatternIds"] = []
        for ordinal in range(1, subject.MAX_PRODUCT_PATTERNS + 2):
            pattern = copy.deepcopy(seed)
            pattern_id = f"KAKULSAYDON_G1_PATTERN_{ordinal}"
            pattern["patternId"] = pattern_id
            pattern["displayName"] = f"Product {ordinal}"
            pattern["nextStageOrdinal"] = len(seed["stages"]) + 1
            pattern["nextAnimationOrdinal"] = len(seed["stages"]) + 1
            for stage_index, stage in enumerate(pattern["stages"], 1):
                stage["stageId"] = f"STAGE_{stage_index}"
                stage["actionId"] = f"{pattern_id}.stage.{stage_index}"
                stage["animationOccurrences"][0]["occurrenceId"] = (
                    f"{pattern_id}.animation.{stage_index}"
                )
            document["patterns"].append(pattern)
            document["playAllPatternIds"].append(pattern_id)
        document["nextPatternOrdinal"] = subject.MAX_PRODUCT_PATTERNS + 2

        with self.assertRaisesRegex(subject.CompositionError, "64 PRODUCT patterns"):
            self.validate(document)

    def test_product_pattern_ids_cannot_collapse_to_one_runtime_action_id(self):
        document = copy.deepcopy(self.document)
        duplicate = self.strip_lanes(copy.deepcopy(self.first_product(document)))
        duplicate["patternId"] = "KAKULSAYDON.G1.PATTERN.1"
        duplicate["displayName"] = "Derived action collision"
        for ordinal, stage in enumerate(duplicate["stages"], 1):
            stage["actionId"] = f"kakulsaydon.g1.collision.stage.{ordinal}"
            stage["animationOccurrences"][0]["occurrenceId"] = (
                f"KAKULSAYDON.G1.PATTERN.1.animation.{ordinal}"
            )
        document["patterns"].append(duplicate)
        document["playAllPatternIds"].append("KAKULSAYDON.G1.PATTERN.1")

        with self.assertRaisesRegex(subject.CompositionError, "duplicate actionId"):
            self.validate(document)

    def test_normal_patterns_are_draft_only_in_the_animation_mvp(self):
        document = copy.deepcopy(self.document)
        self.first_product(document)["category"] = "NORMAL"
        with self.assertRaisesRegex(subject.CompositionError, "MECHANIC category"):
            self.validate(document)

    def test_display_name_uses_the_same_255_utf8_byte_ui_boundary(self):
        accepted = copy.deepcopy(self.document)
        accepted["patterns"][0]["displayName"] = "a" * 255
        self.validate(accepted)

        too_many_bytes = copy.deepcopy(self.document)
        too_many_bytes["patterns"][0]["displayName"] = "가" * 86
        with self.assertRaisesRegex(subject.CompositionError, "255 UTF-8 bytes"):
            self.validate(too_many_bytes)

        control = copy.deepcopy(self.document)
        control["patterns"][0]["displayName"] = "bad\nname"
        with self.assertRaisesRegex(subject.CompositionError, "control characters"):
            self.validate(control)

    def test_product_requires_one_animation_at_stage_entry(self):
        missing = copy.deepcopy(self.document)
        self.first_product(missing)["stages"][0]["animationOccurrences"] = []
        with self.assertRaisesRegex(subject.CompositionError, "exactly one"):
            self.validate(missing)

        offset = copy.deepcopy(self.document)
        occurrence = self.first_product(offset)["stages"][0]["animationOccurrences"][0]
        occurrence["startOffsetMs"] = 1
        occurrence["playMs"] -= 1
        with self.assertRaisesRegex(subject.CompositionError, "stage entry"):
            self.validate(offset)

    def test_product_short_clip_holds_its_endpoint_and_keeps_loop_policy(self):
        document = copy.deepcopy(self.document)
        pattern = self.first_product(document)
        stage = pattern["stages"][0]
        animation = stage["animationOccurrences"][0]
        stage["durationMs"] = 1000
        animation.update(startOffsetMs=0, sourceStartMs=0, playMs=600, playRate=1.5, endPolicy="EXACT")
        self.validate(document)
        binding = next(row for row in subject.project_presentation(document)["bindings"]
                       if row["occurrenceId"] == animation["occurrenceId"])
        self.assertTrue(binding["holdAtWindowEnd"])
        # At the authored cut and in the remaining stage gap the source is fixed.
        for clock_ms in (600, 700, 950):
            selected, seconds = subject._bone_bake_clip_sample(pattern, clock_ms)
            self.assertEqual(animation["occurrenceId"], selected["occurrenceId"])
            self.assertAlmostEqual(0.9, seconds)
        animation.update(playMs=1000, endPolicy="LOOP_TO_WINDOW")
        self.validate(document)
        binding = next(row for row in subject.project_presentation(document)["bindings"]
                       if row["occurrenceId"] == animation["occurrenceId"])
        self.assertEqual("LOOP_TO_WINDOW", binding["endPolicy"])
        self.assertNotIn("holdAtWindowEnd", binding)
        animation["endPolicy"] = "HOLD_LAST_POSE"
        self.validate(document)
        self.assertTrue(subject._animation_holds_window_end(stage, animation))

    def test_product_rejects_presentation_policies_the_client_cannot_run(self):
        variants = []
        source_offset = copy.deepcopy(self.document)
        stage = self.first_product(source_offset)["stages"][0]
        stage["durationMs"] = 100
        stage["animationOccurrences"][0]["playMs"] = 100
        stage["animationOccurrences"][0]["sourceStartMs"] = 1
        variants.append(source_offset)

        slow_rate = copy.deepcopy(self.document)
        stage = self.first_product(slow_rate)["stages"][0]
        stage["durationMs"] = 100
        stage["animationOccurrences"][0]["playMs"] = 100
        stage["animationOccurrences"][0]["playRate"] = 0.05
        variants.append(slow_rate)

        for mutation in variants:
            with self.subTest(mutation=mutation):
                with self.assertRaisesRegex(
                    subject.CompositionError, "current runtime policy"
                ):
                    self.validate(mutation)

    def test_reference_metadata_does_not_gate_composition(self):
        for key, value in (
            ("referenceRevision", "0" * 64),
            ("runtimeClip", "rpct00_idle_battle_1"),
            ("sourceSlotId", "animation-999"),
        ):
            document = copy.deepcopy(self.document)
            self.first_product(document)["stages"][0]["animationOccurrences"][0][key] = value
            with self.subTest(key=key), mock.patch.object(subject, "load_json") as read:
                self.validate(document)
                read.assert_not_called()

    @staticmethod
    def append_reference_sequence(pattern, profile_id, source_action_id=None):
        reference = subject.load_json(ROOT / subject.REFERENCE_ROOT / f"{profile_id}.actionreference.json")
        action = next(
            action for action in reference["actions"]
            if (source_action_id is None or action["sourceActionId"] == source_action_id)
            and any(stage["slots"] for stage in action["stages"])
        )
        ordinal = pattern["nextStageOrdinal"]
        pattern["nextStageOrdinal"] += 1
        stage = {
            "stageId": f"STAGE_{ordinal}",
            "actionId": f"{pattern['patternId']}.stage.{ordinal}",
            "stageKind": "ACTIVE", "durationMs": 0, "animationOccurrences": [],
        }
        for source_stage in action["stages"]:
            for slot in source_stage["slots"]:
                occurrence = {
                    "occurrenceId": f"{pattern['patternId']}.animation.{pattern['nextAnimationOrdinal']}",
                    "profileId": profile_id, "sourceActionId": action["sourceActionId"],
                    "sourceStageId": source_stage["stageId"], "sourceSlotId": slot["slotId"],
                    "referenceRevision": reference["referenceRevision"],
                    "runtimeClip": slot["runtimeClip"], "startOffsetMs": stage["durationMs"],
                    "sourceStartMs": slot["sourceStartMs"], "playMs": slot["playMs"],
                    "playRate": slot["playRate"],
                    "endPolicy": "LOOP_TO_WINDOW" if slot["loop"] else "EXACT",
                }
                pattern["nextAnimationOrdinal"] += 1
                stage["durationMs"] += slot["playMs"]
                stage["animationOccurrences"].append(occurrence)
        pattern["stages"].append(stage)
        return stage

    def test_known_actor_sequences_roundtrip_and_delete_without_retargeting(self):
        draft_index = next(
            index for index, pattern in enumerate(self.document["patterns"])
            if pattern["patternId"] == DRAFT_ID
        )
        for profile_id, action_id in (("MN_RPCT_05", 4219811), ("MN_RPCT_06", None),
                                      ("MN_RPCT_07", None), ("MN_RPCZ_00", 0)):
            with self.subTest(profile=profile_id):
                document = copy.deepcopy(self.document)
                pattern = self.draft(document)
                owner = subject.resolve_actor_profile_id(profile_id)
                pattern["actorProfileId"] = owner
                pattern["gateId"] = "GATE2" if owner == "MN_RPCT_06" else "GATE1"
                pattern["targetBossPlacementId"] = subject.GATE_TARGETS[(pattern["gateId"], owner)][0]
                stage = self.append_reference_sequence(pattern, profile_id, action_id)
                self.validate(document)
                reopened = json.loads(json.dumps(document, ensure_ascii=False))
                self.validate(reopened)
                self.assertEqual(document, reopened)
                self.assertTrue(all(row["profileId"] == profile_id for row in stage["animationOccurrences"]))
                if action_id == 4219811:
                    self.assertEqual(["rpct00_att_battle_12_06"],
                                     [row["runtimeClip"] for row in stage["animationOccurrences"]])
                if action_id == 0:
                    self.assertEqual(["stage-003", "stage-006"],
                                     [row["sourceStageId"] for row in stage["animationOccurrences"]])
                pattern["stages"][0]["animationOccurrences"].clear()
                self.validate(document)
                pattern["stages"].clear()
                self.validate(document)
                self.assertEqual(owner, pattern["actorProfileId"])
                others = [p for i, p in enumerate(self.document["patterns"]) if i != draft_index]
                self.assertEqual(others, [p for i, p in enumerate(document["patterns"]) if i != draft_index])

    def test_real_249_stage_saydon_action_fits_draft_but_product_stays_bounded(self):
        document = copy.deepcopy(self.document)
        draft = self.draft(document)
        draft["actorProfileId"] = "MN_RPCT_05"
        sequence = self.append_reference_sequence(draft, "MN_RPCT_05", 4219880)
        self.assertEqual(249, len(sequence["animationOccurrences"]))
        self.assertEqual(273134, sequence["durationMs"])
        self.assertEqual(249, len({row["sourceStageId"] for row in sequence["animationOccurrences"]}))
        draft["stages"] = []
        for ordinal, row in enumerate(sequence["animationOccurrences"], 1):
            row["startOffsetMs"] = 0
            draft["stages"].append({
                "stageId": f"STAGE_{ordinal}",
                "actionId": f"{draft['patternId']}.stage.{ordinal}",
                "stageKind": "ACTIVE", "durationMs": row["playMs"],
                "animationOccurrences": [row],
            })
        draft["nextStageOrdinal"] = 250
        self.validate(document)
        self.validate(json.loads(json.dumps(document)))
        product = self.first_product(document)
        template = product["stages"][0]
        product["stages"] = []
        for ordinal in range(1, 66):
            stage = copy.deepcopy(template)
            stage["stageId"] = f"STAGE_{ordinal}"
            stage["actionId"] = f"{product['patternId']}.stage.{ordinal}"
            stage["animationOccurrences"][0]["occurrenceId"] = f"{product['patternId']}.animation.{ordinal}"
            product["stages"].append(stage)
        product["nextStageOrdinal"] = product["nextAnimationOrdinal"] = 66
        with self.assertRaisesRegex(subject.CompositionError, "at most 64 rows"):
            self.validate(document)

    def test_actor_ownership_rejects_cross_model_unknown_and_nonphysical_owner(self):
        document = copy.deepcopy(self.document)
        pattern = self.draft(document)
        pattern["actorProfileId"] = "MN_RPCT_05"
        stage = self.append_reference_sequence(pattern, "MN_RPCT_05", 4219811)
        for owner in ("MN_RPCZ_00", "MN_RPCT_06", "MN_RPCT_07", "UNKNOWN", ""):
            with self.subTest(owner=owner):
                pattern["actorProfileId"] = owner
                with self.assertRaises(subject.CompositionError):
                    self.validate(document)
        pattern["actorProfileId"] = "MN_RPCT_05"
        stage["animationOccurrences"][0]["profileId"] = "UNKNOWN"
        with self.assertRaisesRegex(subject.CompositionError, "unknown animation profile"):
            self.validate(document)
        stage["animationOccurrences"][0]["profileId"] = "MN_RPCT_07"
        self.validate(document)

    def test_actor_without_arena_boss_body_cannot_publish(self):
        # Pure validation accepts any physical actor; publish joins the actor body
        # against Data/Actors/BossCatalog.json, so a body no arena boss presents
        # on is refused only there.
        document = copy.deepcopy(self.document)
        document["patterns"] = [self.strip_lanes(self.first_product(document))]
        document["playAllPatternIds"] = [FIRST_PRODUCT_ID]
        self.validate(document)
        catalog = subject.load_json(ROOT / subject.BOSS_CATALOG_PATH)
        self.assertIn("MN_RPCT_05", subject.arena_boss_archetypes_by_profile(ROOT))
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            copy_repository_inputs(root)
            catalog_path = root / subject.BOSS_CATALOG_PATH
            catalog["bosses"] = [
                boss for boss in catalog["bosses"]
                if boss["archetypeId"] == subject.BOSS_ARCHETYPE_ID
            ]
            catalog_path.write_text(json.dumps(catalog), encoding="utf-8")
            self.assertEqual(
                {"MN_RPCZ_00": [subject.BOSS_ARCHETYPE_ID]},
                subject.arena_boss_archetypes_by_profile(root),
            )
            with self.assertRaisesRegex(subject.CompositionError, "PRODUCT actorProfileId"):
                subject.validate_publishable(document, root)
            encounter = subject.project_encounter(document, root)
            self.assertEqual([], encounter["patterns"][0]["bossArchetypeIds"])
        encounter = subject.project_encounter(document, ROOT)
        self.assertEqual(
            subject.arena_boss_archetypes_by_profile(ROOT)["MN_RPCT_05"],
            encounter["patterns"][0]["bossArchetypeIds"],
        )

    def test_legacy_owner_derivation_accepts_alias_and_rejects_mixed_models(self):
        document = copy.deepcopy(self.document)
        draft = self.draft(document)
        stage = self.append_reference_sequence(draft, "MN_RPCT_05", 4219811)
        self.append_reference_sequence(draft, "MN_RPCT_07")
        document["formatVersion"] = 1
        for pattern in document["patterns"]:
            del pattern["actorProfileId"]
        self.validate(document)
        stage["animationOccurrences"][0]["profileId"] = "MN_RPCT_06"
        with self.assertRaisesRegex(subject.CompositionError, "does not match Pattern actorProfileId"):
            self.validate(document)
        draft["stages"].clear()
        self.validate(document)

    def test_v2_requires_explicit_owner_even_for_an_empty_pattern(self):
        document = copy.deepcopy(self.document)
        del self.draft(document)["actorProfileId"]
        with self.assertRaises(subject.CompositionError):
            self.validate(document)

    def test_referenced_action_zero_is_preserved_in_product_source_ids(self):
        document = copy.deepcopy(self.document)
        draft = self.draft(document)
        source_stage = self.append_reference_sequence(draft, "MN_RPCT_05", 0)
        row = copy.deepcopy(source_stage["animationOccurrences"][0])
        product = self.first_product(document)
        stage = product["stages"][0]
        row["occurrenceId"] = stage["animationOccurrences"][0]["occurrenceId"]
        row["startOffsetMs"] = 0
        row["endPolicy"] = "EXACT"
        stage["animationOccurrences"] = [row]
        stage["durationMs"] = row["playMs"]
        draft["stages"].clear()
        self.validate(document)
        self.assertEqual(
            [0] + sorted({s["animationOccurrences"][0]["sourceActionId"] for s in product["stages"][1:]}),
            subject.project_encounter(document)["patterns"][0]["sourceActionIds"],
        )
        row["referenceRevision"] = ""
        with self.assertRaisesRegex(subject.CompositionError, "lowercase SHA-256"):
            self.validate(document)

    def test_raw_stage_cannot_claim_a_nonzero_reference_action(self):
        document = copy.deepcopy(self.document)
        row = self.first_product(document)["stages"][0]["animationOccurrences"][0]
        row["sourceStageId"] = "RAW"
        with self.assertRaisesRegex(subject.CompositionError, "RAW clips must use sourceActionId 0"):
            self.validate(document)

    def test_raw_model_clips_publish_without_extracted_action_metadata(self):
        document = copy.deepcopy(self.document)
        for stage in self.first_product(document)["stages"]:
            row = stage["animationOccurrences"][0]
            row.update(sourceActionId=0, sourceStageId="RAW",
                       sourceSlotId=row["runtimeClip"], referenceRevision="")
        self.validate(document)
        self.assertEqual([], subject.project_encounter(document)["patterns"][0]["sourceActionIds"])
        self.assertEqual(
            self.product_binding_count(self.document),
            len(subject.project_presentation(document)["bindings"]),
        )

    def test_incomplete_row_is_visible_without_blocking_other_saved_patterns(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / subject.SOURCE_PATH
            source.parent.mkdir(parents=True)
            copy_repository_inputs(root)
            document = copy.deepcopy(self.document)
            self.draft(document)["stages"] = "broken"
            source.write_text(json.dumps(document), encoding="utf-8")
            result = subject.run(root, "publish")
            self.assertEqual(2, result["outputCount"])
            published = subject.load_json(root / subject.ENCOUNTER_PATH)
            self.assertTrue(self.find(published["patternInventory"], DRAFT_ID)["unavailableReason"])
            self.first_product(document)["stages"][0]["animationOccurrences"] = []
            source.write_text(json.dumps(document), encoding="utf-8")
            subject.run(root, "publish")
            published = subject.load_json(root / subject.ENCOUNTER_PATH)
            self.assertNotIn(FIRST_PRODUCT_ID, published["playAllPatternIds"])
            self.assertTrue(self.find(published["patternInventory"], FIRST_PRODUCT_ID)["unavailableReason"])
            before = (root / subject.ENCOUNTER_PATH).read_bytes()
            document["patterns"].append(copy.deepcopy(document["patterns"][0]))
            source.write_text(json.dumps(document), encoding="utf-8")
            with self.assertRaises(subject.CompositionError):
                subject.run(root, "publish")
            self.assertEqual(before, (root / subject.ENCOUNTER_PATH).read_bytes())

    def test_play_all_is_exact_product_order(self):
        missing = copy.deepcopy(self.document)
        missing["playAllPatternIds"] = []
        with self.assertRaisesRegex(subject.CompositionError, "PRODUCT patternIds"):
            self.validate(missing)
        unknown = copy.deepcopy(self.document)
        unknown["playAllPatternIds"] = ["KAKULSAYDON_G1_UNKNOWN"]
        with self.assertRaisesRegex(subject.CompositionError, "PRODUCT patternIds"):
            self.validate(unknown)

    def test_product_projection_is_minimal_and_timeout_only(self):
        encounter = subject.project_encounter(self.document)
        self.assertEqual(
            {
                "schema",
                "formatVersion",
                "encounterId",
                "bossArchetypeId",
                "authority",
                "fixedTickHz",
                "sourceRevision",
                "madnessPolicy",
                "playAllPatternIds",
                "patterns", "folders", "bundles",
            },
            set(encounter),
        )
        pattern = encounter["patterns"][0]
        self.assertEqual("AUDITION_ONLY", pattern["selectionMode"])
        self.assertNotIn("branches", pattern)
        self.assertNotIn("actions", pattern)
        for stage in pattern["stages"]:
            self.assertEqual("NONE", stage["hitShape"])
            self.assertEqual(0, stage["hitCount"])
            self.assertEqual("", stage["serverDamageProfileId"])
            self.assertNotIn("branches", stage)
            self.assertNotIn("actions", stage)

        presentation = subject.project_presentation(self.document)
        self.assertEqual(
            {
                "schema",
                "formatVersion",
                "bossArchetypeId",
                "sourceRevision",
                "lightResourceRevision",
                "bindings", "patterns", "folders", "bundles",
            },
            set(presentation),
        )
        self.assertEqual(self.product_binding_count(self.document), len(presentation["bindings"]))

    @classmethod
    def without_catalog_boxes(cls, document):
        """Drop live lane boxes so a test-owned catalog stays self-consistent."""
        for pattern in document["patterns"]:
            cls.strip_lanes(pattern)
        return document

    def test_logic_catalog_is_optional_on_read_and_untyped_logic_never_reaches_product(self):
        document = self.without_catalog_boxes(copy.deepcopy(self.document))
        document["nextLogicOrdinal"] = 2
        document["logics"] = [
            {
                "logicId": "kakulsaydon.g1.logic.1",
                "displayName": "방패 무력화",
                "logicType": "DURATION",
            }
        ]
        self.validate(copy.deepcopy(document))
        baseline = self.without_catalog_boxes(copy.deepcopy(self.document))
        self.assertEqual(
            subject.project_encounter(baseline),
            subject.project_encounter(copy.deepcopy(document)),
        )

        product = self.first_product(document)
        product["nextLogicOccurrenceOrdinal"] = 2
        product["presentationOccurrences"] = []
        product["logicOccurrences"] = [
            {
                "occurrenceId": f"{FIRST_PRODUCT_ID}.logic.1",
                "logicId": "kakulsaydon.g1.logic.1",
                "startMs": 0,
                "durationMs": 1000,
            }
        ]
        with self.assertRaisesRegex(subject.CompositionError, "without a judgementKind"):
            self.validate(copy.deepcopy(document))

        draft = copy.deepcopy(document)
        for pattern in self.product_patterns(draft):
            pattern["authoringStatus"] = "DRAFT"
        draft["playAllPatternIds"] = []
        self.validate(copy.deepcopy(draft))

        unknown_type = copy.deepcopy(draft)
        unknown_type["logics"][0]["logicType"] = "WINDOW"
        with self.assertRaisesRegex(subject.CompositionError, "logicType"):
            self.validate(unknown_type)
        ahead = copy.deepcopy(draft)
        ahead["nextLogicOrdinal"] = 1
        with self.assertRaisesRegex(subject.CompositionError, "nextLogicOrdinal"):
            self.validate(ahead)
        dangling = copy.deepcopy(draft)
        self.first_product(dangling)["logicOccurrences"][0]["logicId"] = "kakulsaydon.g1.logic.9"
        with self.assertRaisesRegex(subject.CompositionError, "unknown logicId"):
            self.validate(dangling)
        foreign_box = copy.deepcopy(draft)
        self.first_product(foreign_box)["logicOccurrences"][0]["occurrenceId"] = f"{FIRST_PRODUCT_ID}.animation.9"
        with self.assertRaisesRegex(subject.CompositionError, r"\.logic\.<N>"):
            self.validate(foreign_box)

    def draft_with_logic_box(self):
        document = copy.deepcopy(self.document)
        pattern = self.strip_lanes(self.first_product(document))
        document["patterns"] = [pattern]
        document["playAllPatternIds"] = []
        document["nextLogicOrdinal"] = 2
        document["logics"] = [{
            "logicId": "kakulsaydon.g1.logic.1",
            "displayName": "Logic window",
            "logicType": "DURATION",
        }]
        pattern["authoringStatus"] = "DRAFT"
        pattern["nextLogicOccurrenceOrdinal"] = 2
        pattern["presentationOccurrences"] = []
        pattern["logicOccurrences"] = [{
            "occurrenceId": pattern["patternId"] + ".logic.1",
            "logicId": "kakulsaydon.g1.logic.1",
            "startMs": 0,
            "durationMs": 1000,
        }]
        return document, pattern

    def test_logic_occurrences_reject_duplicate_and_invalid_ordinals(self):
        document, pattern = self.draft_with_logic_box()
        self.validate(copy.deepcopy(document))
        duplicate = copy.deepcopy(document)
        duplicate_pattern = self.first_product(duplicate)
        duplicate_pattern["logicOccurrences"].append(
            copy.deepcopy(duplicate_pattern["logicOccurrences"][0])
        )
        with self.assertRaisesRegex(subject.CompositionError, "duplicate Logic occurrenceId"):
            self.validate(duplicate)

        for ordinal in (0, 2, subject.MAX_ORDINAL):
            with self.subTest(ordinal=ordinal):
                invalid = copy.deepcopy(document)
                self.first_product(invalid)["logicOccurrences"][0]["occurrenceId"] = (
                    pattern["patternId"] + f".logic.{ordinal}"
                )
                with self.assertRaises(subject.CompositionError):
                    self.validate(invalid)
        for counter in (0, 1, subject.MAX_ORDINAL + 1):
            with self.subTest(counter=counter):
                invalid = copy.deepcopy(document)
                self.first_product(invalid)["nextLogicOccurrenceOrdinal"] = counter
                with self.assertRaisesRegex(subject.CompositionError, "nextLogicOccurrenceOrdinal"):
                    self.validate(invalid)

    def test_logic_windows_use_pattern_lifetime_and_bounded_stage_clock(self):
        document, pattern = self.draft_with_logic_box()
        lifetime_ms = sum(stage["durationMs"] for stage in pattern["stages"])
        pattern["logicOccurrences"][0].update(startMs=lifetime_ms - 1, durationMs=1)
        self.validate(copy.deepcopy(document))
        beyond_lifetime = copy.deepcopy(document)
        self.first_product(beyond_lifetime)["logicOccurrences"][0]["durationMs"] = 2
        with self.assertRaisesRegex(subject.CompositionError, "Pattern lifetime"):
            self.validate(beyond_lifetime)

        pattern["stages"] = pattern["stages"][:1]
        pattern["stages"][0]["durationMs"] = subject.MAX_TIMELINE_MS
        pattern["logicOccurrences"][0].update(
            startMs=subject.MAX_TIMELINE_MS - 1, durationMs=1
        )
        self.validate(copy.deepcopy(document))
        beyond_limit = copy.deepcopy(document)
        self.first_product(beyond_limit)["logicOccurrences"][0]["durationMs"] = 2
        with self.assertRaisesRegex(subject.CompositionError, "Logic box exceeds 600 seconds"):
            self.validate(beyond_limit)

        oversized, oversized_pattern = self.draft_with_logic_box()
        oversized_pattern["stages"][0]["durationMs"] = subject.MAX_TIMELINE_MS
        with self.assertRaisesRegex(subject.CompositionError, "Pattern exceeds 600 seconds"):
            self.validate(oversized)
        empty = copy.deepcopy(document)
        self.first_product(empty)["stages"] = []
        with self.assertRaisesRegex(subject.CompositionError, "Pattern lifetime"):
            self.validate(empty)

    def test_logic_occurrence_capacity_matches_client_1024_boxes(self):
        document, pattern = self.draft_with_logic_box()
        pattern["presentationOccurrences"] = []
        pattern["logicOccurrences"] = [{
            "occurrenceId": pattern["patternId"] + f".logic.{ordinal}",
            "logicId": "kakulsaydon.g1.logic.1",
            "startMs": 0,
            "durationMs": 1,
        } for ordinal in range(1, 1025)]
        pattern["nextLogicOccurrenceOrdinal"] = 1025
        self.validate(copy.deepcopy(document))
        extra = copy.deepcopy(pattern["logicOccurrences"][-1])
        extra["occurrenceId"] = pattern["patternId"] + ".logic.1025"
        pattern["logicOccurrences"].append(extra)
        pattern["nextLogicOccurrenceOrdinal"] = 1026
        with self.assertRaisesRegex(subject.CompositionError, "1024"):
            self.validate(document)

    def test_legacy_singular_outcomes_name_result_logics_on_duration_boxes_only(self):
        document = self.without_catalog_boxes(copy.deepcopy(self.document))
        document["nextLogicOrdinal"] = 4
        document["logics"] = [
            {"logicId": "kakulsaydon.g1.logic.1", "displayName": "방패 무력화", "logicType": "DURATION"},
            {"logicId": "kakulsaydon.g1.logic.2", "displayName": "그로기 패턴", "logicType": "RESULT"},
            {"logicId": "kakulsaydon.g1.logic.3", "displayName": "전원 전멸", "logicType": "RESULT"},
        ]
        for pattern in self.product_patterns(document):
            pattern["authoringStatus"] = "DRAFT"
        document["playAllPatternIds"] = []
        product = self.first_product(document)
        product["nextLogicOccurrenceOrdinal"] = 2
        product["presentationOccurrences"] = []
        product["logicOccurrences"] = [
            {
                "occurrenceId": f"{FIRST_PRODUCT_ID}.logic.1",
                "logicId": "kakulsaydon.g1.logic.1",
                "startMs": 0,
                "durationMs": 1000,
                "onSuccessLogicId": "kakulsaydon.g1.logic.2",
                "onTimeoutLogicId": "kakulsaydon.g1.logic.3",
            }
        ]
        self.validate(copy.deepcopy(document))
        box = product["logicOccurrences"][0]
        self.assertEqual(["kakulsaydon.g1.logic.2"], subject.outcome_logic_ids(box, "Success"))
        self.assertEqual([], subject.outcome_logic_ids(box, "Fail"))
        self.assertEqual(["kakulsaydon.g1.logic.3"], subject.outcome_logic_ids(box, "Timeout"))

        unwired = copy.deepcopy(document)
        box = self.first_product(unwired)["logicOccurrences"][0]
        del box["onSuccessLogicId"]
        box["onTimeoutLogicId"] = ""
        self.validate(unwired)

        not_result = copy.deepcopy(document)
        self.first_product(not_result)["logicOccurrences"][0]["onSuccessLogicId"] = "kakulsaydon.g1.logic.1"
        with self.assertRaisesRegex(subject.CompositionError, "RESULT logic"):
            self.validate(not_result)
        dangling = copy.deepcopy(document)
        self.first_product(dangling)["logicOccurrences"][0]["onTimeoutLogicId"] = "kakulsaydon.g1.logic.9"
        with self.assertRaisesRegex(subject.CompositionError, "RESULT logic"):
            self.validate(dangling)
        on_result_box = copy.deepcopy(document)
        self.first_product(on_result_box)["logicOccurrences"][0]["logicId"] = "kakulsaydon.g1.logic.2"
        with self.assertRaisesRegex(subject.CompositionError, "DURATION logic box"):
            self.validate(on_result_box)
        wrong_type = copy.deepcopy(document)
        self.first_product(wrong_type)["logicOccurrences"][0]["onSuccessLogicId"] = 7
        with self.assertRaisesRegex(subject.CompositionError, "must be text"):
            self.validate(wrong_type)
        wrong_list = copy.deepcopy(document)
        self.first_product(wrong_list)["logicOccurrences"][0]["onFailLogicIds"] = "kakulsaydon.g1.logic.3"
        with self.assertRaisesRegex(subject.CompositionError, "list of text"):
            self.validate(wrong_list)

    def test_fear_result_projects_only_stable_server_presentation_and_rejects_bad_references(self):
        document = copy.deepcopy(self.document)
        logic = next(row for row in document["logics"] if row["logicId"] == "kakulsaydon.g1.logic.2")
        logic.clear()
        logic.update(logicId="kakulsaydon.g1.logic.2", displayName="Fear", logicType="RESULT",
                     outcomeKind="FEAR", durationMs=3000,
                     sceneProfileId=document["sceneProfiles"][0]["sceneProfileId"])
        self.validate(document)
        server = subject._project_outcomes({logic["logicId"]: logic}, [logic["logicId"]])[0]
        self.assertEqual({"kind": "FEAR", "percent": 0, "durationMs": 3000,
                          "patternId": "", "presentationId": logic["logicId"]}, server)
        client = subject.project_presentation(document)["fearPresentations"][0]
        self.assertEqual(document["sceneProfiles"][0]["renderingProfileId"], client["sceneProfileId"])
        self.assertIsNone(client["effectResource"])
        for change in ({"durationMs": 0}, {"sceneProfileId": "missing"},
                       {"effectResourceId": "missing"}, {"effectDelayMs": 1000},
                       {"lightResourceId": "missing"}):
            invalid = copy.deepcopy(document)
            next(row for row in invalid["logics"] if row["logicId"] == logic["logicId"]).update(change)
            with self.subTest(change=change), self.assertRaises(subject.CompositionError):
                self.validate(invalid)

    def test_gaze_facing_outcome_projects_and_preserves_default(self):
        document = copy.deepcopy(self.document)
        logics = {row["logicId"]: row for row in document["logics"]}
        gaze = logics["kakulsaydon.g1.logic.16"]
        gaze.pop("insideOutcome", None)
        box = next(row for row in self.find(document, GAZE_ID)["logicOccurrences"]
                   if row["logicId"] == gaze["logicId"])
        self.assertEqual("SUCCESS", subject._project_logic_window(box, gaze, logics, 0)["insideOutcome"])
        gaze["insideOutcome"] = "FAIL"
        self.validate(document)
        projected = subject._project_logic_window(box, gaze, logics, 0)
        self.assertEqual("FAIL", projected["insideOutcome"])
        self.assertEqual("GAZE_REAL_BOSS", projected["kind"])
        self.assertEqual("INSTANT_DEATH", projected["onFail"][0]["kind"])
        gaze["insideOutcome"] = "TIMEOUT"
        with self.assertRaisesRegex(subject.CompositionError, "insideOutcome"):
            self.validate(document)

    def test_counter_window_projects_followup_and_refuses_per_player_fail(self):
        document = copy.deepcopy(self.document)
        logic = next(row for row in document["logics"] if row["logicId"] == "kakulsaydon.g1.logic.1")
        logic.clear()
        logic.update(logicId="kakulsaydon.g1.logic.1", displayName="Counter", logicType="DURATION",
                     judgementKind="COUNTER_WINDOW", endsPatternOnSuccess=True)
        pattern = self.first_product(document)
        pattern["presentationOccurrences"] = [row for row in pattern.get("presentationOccurrences", [])
                                              if not row.get("logicOccurrenceId")]
        self.validate(document)
        server = next(row for row in subject.project_encounter(document)["patterns"]
                      if row["patternId"] == FIRST_PRODUCT_ID)
        window = next(row for row in server["logicWindows"] if row["kind"] == "COUNTER_WINDOW")
        self.assertTrue(window["endsPatternOnSuccess"])
        self.assertEqual("FOLLOWUP_PATTERN", window["onSuccess"][0]["kind"])
        invalid = copy.deepcopy(document)
        next(row for row in self.first_product(invalid)["logicOccurrences"]
             if row["logicId"] == logic["logicId"])["onFailLogicIds"] = ["kakulsaydon.g1.logic.2"]
        with self.assertRaises(subject.CompositionError):
            self.validate(invalid)

    def test_typed_logic_definitions_follow_their_kind(self):
        logics = {logic["logicId"]: logic for logic in self.document["logics"]}
        self.assertEqual("STAGGER_WINDOW", logics["kakulsaydon.g1.logic.1"]["judgementKind"])
        self.assertEqual(1000, logics["kakulsaydon.g1.logic.1"]["threshold"])
        self.assertEqual("FOLLOWUP_PATTERN", logics["kakulsaydon.g1.logic.7"]["outcomeKind"])
        self.assertEqual("KAKULSAYDON_G1_PATTERN_4", logics["kakulsaydon.g1.logic.7"]["followupPatternId"])
        self.assertEqual(
            [1, 2, 3, 0],
            [logics[f"kakulsaydon.g1.logic.{n}"]["poseIndex"] for n in (10, 11, 12, 13)],
        )
        self.assertEqual(8, len(logics["kakulsaydon.g1.logic.14"]["regionIds"]))
        self.assertEqual("TRIGGER", logics["kakulsaydon.g1.logic.9"]["logicType"])

        def mutate(logic_id, **changes):
            document = copy.deepcopy(self.document)
            logic = next(row for row in document["logics"] if row["logicId"] == logic_id)
            for key, value in changes.items():
                if value is None:
                    logic.pop(key, None)
                else:
                    logic[key] = value
            return document

        rejected = {
            "unknown judgement kind": mutate("kakulsaydon.g1.logic.1", judgementKind="COIN_FLIP"),
            "values of another kind": mutate("kakulsaydon.g1.logic.1", poseIndex=1),
            "judgement values on a RESULT": mutate("kakulsaydon.g1.logic.2", threshold=5),
            "outcome values on a DURATION": mutate("kakulsaydon.g1.logic.1", percent=5),
            "unknown outcome kind": mutate("kakulsaydon.g1.logic.2", outcomeKind="HEAL"),
            "percent outcome without percent": mutate("kakulsaydon.g1.logic.4", percent=0),
            "percent on a death outcome": mutate("kakulsaydon.g1.logic.2", percent=10),
            "duration on a damage outcome": mutate("kakulsaydon.g1.logic.4", durationMs=100),
            "follow-up without a target": mutate("kakulsaydon.g1.logic.7", followupPatternId=""),
            "target on a non follow-up": mutate("kakulsaydon.g1.logic.2", followupPatternId="KAKULSAYDON_G1_PATTERN_4"),
            "follow-up naming an unknown pattern": mutate("kakulsaydon.g1.logic.7", followupPatternId="KAKULSAYDON_G1_PATTERN_99"),
            "pose index past the four poses": mutate("kakulsaydon.g1.logic.10", poseIndex=4),
            "duplicate region ID": mutate("kakulsaydon.g1.logic.14", regionIds=["same"] * 8),
            "invalid region ID": mutate("kakulsaydon.g1.logic.14", regionIds=["has space"]),
            "gaze cone wider than a half turn": mutate("kakulsaydon.g1.logic.16", halfAngleDegrees=181),
            "trigger with values": mutate("kakulsaydon.g1.logic.9", threshold=1),
        }
        for label, document in rejected.items():
            with self.subTest(label=label):
                with self.assertRaises(subject.CompositionError):
                    self.validate(document)
        # A typed value may be dropped again on a DRAFT-only definition.
        untyped = mutate("kakulsaydon.g1.logic.16", judgementKind=None, halfAngleDegrees=None, maxDistanceM=None)
        with self.assertRaisesRegex(subject.CompositionError, "without a judgementKind"):
            self.validate(untyped)
        self.find(untyped, GAZE_ID)["authoringStatus"] = "DRAFT"
        untyped["playAllPatternIds"].remove(GAZE_ID)
        self.validate(untyped)

    def test_outcome_slots_follow_kind_rules_and_capacity(self):
        document = copy.deepcopy(self.document)
        gaze_box = next(
            box for box in self.find(document, GAZE_ID)["logicOccurrences"]
            if box["logicId"] == "kakulsaydon.g1.logic.16"
        )
        self.assertEqual([], gaze_box["onTimeoutLogicIds"])
        self.assertEqual(["kakulsaydon.g1.logic.3"], gaze_box["onFailLogicIds"])
        stagger_box = self.first_product(document)["logicOccurrences"][0]
        self.assertEqual([], stagger_box["onFailLogicIds"])
        dance_boxes = self.find(document, DANCE_ID)["logicOccurrences"]
        for box in dance_boxes:
            self.assertEqual(box["onTimeoutLogicIds"], box["onFailLogicIds"])
        self.validate(copy.deepcopy(document))

        end_tick_timeout = copy.deepcopy(document)
        next(
            box for box in self.find(end_tick_timeout, GAZE_ID)["logicOccurrences"]
            if box["logicId"] == "kakulsaydon.g1.logic.16"
        )["onTimeoutLogicIds"] = ["kakulsaydon.g1.logic.3"]
        with self.assertRaisesRegex(subject.CompositionError, "no Timeout slot"):
            self.validate(end_tick_timeout)
        stagger_fail = copy.deepcopy(document)
        self.first_product(stagger_fail)["logicOccurrences"][0]["onFailLogicIds"] = ["kakulsaydon.g1.logic.2"]
        with self.assertRaisesRegex(subject.CompositionError, "no Fail slot"):
            self.validate(stagger_fail)
        too_many = copy.deepcopy(document)
        self.find(too_many, DANCE_ID)["logicOccurrences"][0]["onTimeoutLogicIds"] = [
            "kakulsaydon.g1.logic.4", "kakulsaydon.g1.logic.5", "kakulsaydon.g1.logic.6",
            "kakulsaydon.g1.logic.15", "kakulsaydon.g1.logic.8",
        ]
        with self.assertRaisesRegex(subject.CompositionError, "more than 4"):
            self.validate(too_many)
        followup_on_dance = copy.deepcopy(document)
        self.find(followup_on_dance, DANCE_ID)["logicOccurrences"][0]["onTimeoutLogicIds"] = ["kakulsaydon.g1.logic.7"]
        with self.assertRaisesRegex(subject.CompositionError, "FOLLOWUP_PATTERN is only valid"):
            self.validate(followup_on_dance)
        stacked = copy.deepcopy(document)
        self.find(stacked, DANCE_ID)["logicOccurrences"][0]["onTimeoutLogicIds"] = [
            "kakulsaydon.g1.logic.15", "kakulsaydon.g1.logic.5",
        ]
        self.validate(copy.deepcopy(stacked))
        window = subject.project_encounter(stacked)["patterns"][4]["logicWindows"][0]
        self.assertEqual(
            [("MADNESS_GAUGE_ADD_PERCENT", 50), ("MAX_HP_PERCENT_DAMAGE", 10)],
            [(row["kind"], row["percent"]) for row in window["onTimeout"]],
        )
        # A follow-up the Server starts must itself be a published Product.
        draft_followup = copy.deepcopy(document)
        self.find(draft_followup, "KAKULSAYDON_G1_PATTERN_4")["authoringStatus"] = "DRAFT"
        draft_followup["playAllPatternIds"].remove("KAKULSAYDON_G1_PATTERN_4")
        self.validate(copy.deepcopy(draft_followup))
        with self.assertRaisesRegex(subject.CompositionError, "must name a PRODUCT pattern"):
            subject.validate_publishable(draft_followup, ROOT)

    def test_summon_and_trigger_boxes_are_authoring_only_on_product(self):
        document = self.without_catalog_boxes(copy.deepcopy(self.document))
        # Untyped legacy TRIGGERs remain authoring-only; typed triggers project.
        trigger = next(row for row in document["logics"] if row["logicId"] == "kakulsaydon.g1.logic.9")
        for key in subject.LOGIC_TRIGGER_VALUE_KEYS:
            trigger.pop(key, None)
        document["nextSummonOrdinal"] = 2
        document["summons"] = [
            {"summonId": "kakulsaydon.g1.summon.1", "displayName": "가짜 세이튼 3"}
        ]
        self.validate(copy.deepcopy(document))
        baseline = subject.project_encounter(self.without_catalog_boxes(copy.deepcopy(self.document)))
        product = self.first_product(document)
        product["nextSummonOccurrenceOrdinal"] = 2
        product["summonOccurrences"] = [
            {
                "occurrenceId": f"{FIRST_PRODUCT_ID}.summon.1",
                "summonId": "kakulsaydon.g1.summon.1",
                "startMs": 0,
                "durationMs": 16334,
            }
        ]
        product["nextLogicOccurrenceOrdinal"] = 2
        product["presentationOccurrences"] = []
        product["logicOccurrences"] = [
            {
                "occurrenceId": f"{FIRST_PRODUCT_ID}.logic.1",
                "logicId": "kakulsaydon.g1.logic.9",
                "startMs": 0,
                "durationMs": 1000,
            }
        ]
        self.validate(copy.deepcopy(document))
        self.assertEqual(baseline, subject.project_encounter(copy.deepcopy(document)))

        ahead = copy.deepcopy(document)
        ahead["nextSummonOrdinal"] = 1
        with self.assertRaisesRegex(subject.CompositionError, "nextSummonOrdinal"):
            self.validate(ahead)
        dangling = copy.deepcopy(document)
        self.first_product(dangling)["summonOccurrences"][0]["summonId"] = "kakulsaydon.g1.summon.9"
        with self.assertRaisesRegex(subject.CompositionError, "unknown summonId"):
            self.validate(dangling)
        foreign_box = copy.deepcopy(document)
        self.first_product(foreign_box)["summonOccurrences"][0]["occurrenceId"] = f"{FIRST_PRODUCT_ID}.logic.9"
        with self.assertRaisesRegex(subject.CompositionError, r"\.summon\.<N>"):
            self.validate(foreign_box)
        typed = copy.deepcopy(document)
        typed["summons"][0]["summonType"] = "SPAWN"
        with self.assertRaisesRegex(subject.CompositionError, "summons\\[0\\]"):
            self.validate(typed)

    def test_product_world_state_requires_a_resolved_model_resource(self):
        document = copy.deepcopy(self.document)
        sequences = subject.load_world_sequences(ROOT, subject.AREA_ID)
        world_box = self.find(document, DANCE_ID)["worldOccurrences"][0]
        world = next(row for row in document["worlds"] if row["worldId"] == world_box["worldId"])
        instance = next(row for row in sequences["instances"] if row["instanceId"] == world["sequenceInstanceId"])
        instance["bindings"] = [{"slotId": "object", "targetKind": "OBJECT_RESOURCE", "targetId": "missing.object"}]
        with mock.patch.object(subject, "load_world_sequences", return_value=sequences):
            with self.assertRaisesRegex(subject.CompositionError, "unresolved model resource"):
                subject.project_encounter(document)
        sequences.setdefault("objectResources", []).append({"objectId": "missing.object", "modelAssetId": "Map/Test/card.wmodel"})
        with mock.patch.object(subject, "load_world_sequences", return_value=sequences):
            result = subject.project_encounter(document)
        projected = next(row for row in result["patterns"] if row["patternId"] == DANCE_ID)
        self.assertEqual(world_box["durationMs"], projected["worldSequences"][0]["durationMs"])

    def test_world_and_scene_profile_lanes_validate_and_project(self):
        document = copy.deepcopy(self.document)
        roulette = self.find(document, ROULETTE_ID)
        world_box = roulette["worldOccurrences"][0]
        world_definition = next(w for w in document["worlds"] if w["worldId"] == world_box["worldId"])
        encounter = subject.project_encounter(document)
        projected = next(p for p in encounter["patterns"] if p["patternId"] == ROULETTE_ID)
        self.assertEqual(
            [{"sequenceInstanceId": "world.sequence.instance.8", "occurrenceId": world_box["occurrenceId"], "startMs": world_box["startMs"],
              "durationMs": world_box["durationMs"],
              "playbackSpeed": world_box["playbackSpeed"], "positionOffset": world_definition.get("positionOffset", [0, 0, 0]),
              "anchorKind": world_definition.get("anchorKind", "NONE"),
              "anchorPosition": world_definition.get("anchorPosition", [0, 0, 0])}],
            projected["worldSequences"],
        )
        self.assertTrue(all(box.get("enabled", True) for box in roulette["logicOccurrences"]))
        self.assertEqual([8,8,8,1], [len(window["cardRegions"]) for window in projected["logicWindows"]])
        pairs = {(symbol,color) for symbol in subject.CARD_SYMBOLS for color in ("RED","BLACK")}
        for window in projected["logicWindows"][:3]:
            self.assertEqual(pairs, {(r["cardSymbol"],r["cardColor"]) for r in window["cardRegions"]})
            self.assertTrue(all(abs(r["radiusM"]-8.0)<0.001 and r["anchorKind"] == "BOSS_SPAWN" for r in window["cardRegions"]))
        gaze = next(p for p in encounter["patterns"] if p["patternId"] == GAZE_ID)
        self.assertEqual("scene.kakulsaydon.find-true-dark.v1", gaze["sceneProfiles"][0]["renderingProfileId"])
        dance = next(p for p in encounter["patterns"] if p["patternId"] == DANCE_ID)
        self.assertEqual("world.sequence.instance.curtain_drop",dance["worldSequences"][0]["sequenceInstanceId"])
        # A partial mapping fails projection and leaves the previous Product intact.
        unmapped = copy.deepcopy(document)
        next(l for l in unmapped["logics"] if l["logicId"] == "kakulsaydon.g1.logic.14")["regionIds"].pop()
        with self.assertRaisesRegex(subject.CompositionError,"eight explicit"):
            subject.project_encounter(unmapped)
        duplicated = copy.deepcopy(document)
        cards = self.find(duplicated,ROULETTE_ID)["presentationOccurrences"]
        cards[1]["cardSymbol"],cards[1]["cardColor"] = cards[0]["cardSymbol"],cards[0]["cardColor"]
        with self.assertRaisesRegex(subject.CompositionError,"eight distinct"):
            subject.project_encounter(duplicated)

        rejected = {}
        no_world = copy.deepcopy(document)
        self.find(no_world, ROULETTE_ID)["worldOccurrences"] = []
        with self.assertRaisesRegex(subject.CompositionError, "WORLD"):
            subject.project_encounter(no_world)
        twice = copy.deepcopy(document)
        twice_roulette = self.find(twice, ROULETTE_ID)
        second = copy.deepcopy(twice_roulette["worldOccurrences"][0])
        second["occurrenceId"] = f"{ROULETTE_ID}.world.2"
        twice_roulette["worldOccurrences"].append(second)
        twice_roulette["nextWorldOccurrenceOrdinal"] = 3
        rejected["same instance from two boxes"] = (twice, "from two boxes")
        late = copy.deepcopy(document)
        self.find(late, ROULETTE_ID)["worldOccurrences"][0]["startMs"] = 40000
        rejected["world box after the pattern"] = (late, "after the Pattern lifetime")
        slow = copy.deepcopy(document)
        self.find(slow, ROULETTE_ID)["worldOccurrences"][0]["playbackSpeed"] = 0.01
        rejected["playback speed below the floor"] = (slow, "playbackSpeed")
        unknown_world = copy.deepcopy(document)
        self.find(unknown_world, ROULETTE_ID)["worldOccurrences"][0]["worldId"] = "kakulsaydon.g1.world.9"
        rejected["unknown world"] = (unknown_world, "unknown worldId")
        long_scene = copy.deepcopy(document)
        self.find(long_scene, GAZE_ID)["sceneProfileOccurrences"][0]["durationMs"] += 1
        rejected["scene profile past the pattern"] = (long_scene, "exceeds the Pattern lifetime")
        unknown_scene = copy.deepcopy(document)
        self.find(unknown_scene, GAZE_ID)["sceneProfileOccurrences"][0]["sceneProfileId"] = "kakulsaydon.g1.sceneprofile.9"
        rejected["unknown scene profile"] = (unknown_scene, "unknown sceneProfileId")
        world_ahead = copy.deepcopy(document)
        world_ahead["nextWorldOrdinal"] = 1
        rejected["world catalog ahead of its counter"] = (world_ahead, "nextWorldOrdinal")
        bad_profile = copy.deepcopy(document)
        bad_profile["sceneProfiles"][0]["renderingProfileId"] = "has space"
        rejected["scene profile with an unstable rendering id"] = (bad_profile, "renderingProfileId")
        bad_policy = copy.deepcopy(document)
        bad_policy["madnessPolicy"]["maximum"] = 0
        rejected["madness maximum of zero"] = (bad_policy, "madnessPolicy maximum")
        for label, (mutated, message) in rejected.items():
            with self.subTest(label=label):
                with self.assertRaisesRegex(subject.CompositionError, message):
                    self.validate(mutated)

    def test_generic_presentation_resources_project_timed_instances(self):
        document = copy.deepcopy(self.document)
        # The user's saved transition value may vary; this fixture checks 500 ms propagation.
        self.find(document, GAZE_ID)["sceneProfileOccurrences"][0]["blendMs"] = 500
        document["presentationResources"] = []
        for world in document.get("worlds", []):
            world.pop("companionEffectResourceId", None)
        for candidate in document["patterns"]:
            candidate["presentationOccurrences"] = []
        pattern = self.first_product(document)
        pattern["presentationOccurrences"] = []
        for ordinal, (kind, asset) in enumerate((
            ("EFFECT", "Effect/V2/example.group.json"), ("SOUND", "Sound/example.wav"),
            ("CAMERA", "camera.shot.example"), ("COLLIDER", "")), 1):
            resource_id = f"kakulsaydon.g1.presentation.{ordinal}"
            document["presentationResources"].append({
                "resourceId": resource_id, "displayName": kind, "kind": kind, "assetId": asset,
            })
            pattern["presentationOccurrences"].append({
                "occurrenceId": f"{pattern['patternId']}.presentation.{ordinal}",
                "resourceId": resource_id, "startMs": 10, "durationMs": 500,
                "fadeInMs": 50, "fadeOutMs": 50, "positionOffset": [1, 2, 3],
            })
        document["nextPresentationResourceOrdinal"] = 5
        pattern["nextPresentationOccurrenceOrdinal"] = 5
        self.validate(copy.deepcopy(document))
        projected = next(p for p in subject.project_presentation(document)["patterns"]
                         if p["patternId"] == pattern["patternId"])
        rows = projected["presentationOccurrences"]
        self.assertEqual(["EFFECT", "SOUND", "CAMERA", "COLLIDER"], [row["kind"] for row in rows])
        self.assertTrue(all(row["durationMs"] == 500 and row["resourceDurationMs"] == 1000 for row in rows))
        self.assertEqual([1, 2, 3], rows[0]["positionOffset"])
        self.assertEqual([1.0, 1.0, 1.0], rows[3]["halfExtents"])
        rejected = copy.deepcopy(document)
        self.first_product(rejected)["presentationOccurrences"][0]["fadeOutMs"] = 500
        with self.assertRaisesRegex(subject.CompositionError, "fades exceed"):
            self.validate(rejected)
        rejected = copy.deepcopy(document)
        rejected["presentationResources"][1]["assetId"] = "Sound/../outside.wav"
        with self.assertRaisesRegex(subject.CompositionError, "Resources-relative"):
            self.validate(rejected)
        scenes = next(p for p in subject.project_presentation(document)["patterns"] if p["patternId"] == GAZE_ID)
        self.assertTrue(any(row["kind"] == "SCENE_PROFILE" and row["fadeInMs"] == 500
                            for row in scenes["presentationOccurrences"]))

    def light_document(self, anchor="PLAYER", asset="light.test.character"):
        document = copy.deepcopy(self.document)
        ordinal = document.get("nextPresentationResourceOrdinal", 1)
        document["nextPresentationResourceOrdinal"] = ordinal + 1
        resource_id = f"kakulsaydon.g1.presentation.{ordinal}"
        document.setdefault("presentationResources", []).append({
            "resourceId": resource_id, "displayName": "Character spotlight", "kind": "LIGHT",
            "assetId": asset, "resourceKind": "", "defaultAnchorKind": anchor, "durationMs": 500,
        })
        pattern = self.first_product(document)
        ordinal = pattern.get("nextPresentationOccurrenceOrdinal", 1)
        pattern["nextPresentationOccurrenceOrdinal"] = ordinal + 1
        box = {"occurrenceId": f"{pattern['patternId']}.presentation.{ordinal}",
               "resourceId": resource_id, "startMs": 0, "durationMs": 500,
               "anchorKind": anchor, "brightnessMultiplier": 2.5}
        pattern.setdefault("presentationOccurrences", []).append(box)
        return document, box

    @staticmethod
    def write_light_catalog(root, revision=1, lights=None):
        path = root / subject.LIGHT_RESOURCES_PATH
        path.parent.mkdir(parents=True, exist_ok=True)
        source = subject.load_json(ROOT / subject.LIGHT_RESOURCES_PATH)
        fixture = copy.deepcopy(next(row for row in source["lights"] if row["defaultAnchorKind"] == "PLAYER"))
        fixture["lightResourceId"] = "light.test.character"
        path.write_bytes(subject.serialize_json({
            "schema": "lostark.light-resources", "formatVersion": 1, "revision": revision,
            "nextLightResourceOrdinal": source["nextLightResourceOrdinal"],
            "lights": [*source["lights"], *(lights if lights is not None else [fixture])],
        }))

    def test_light_anchors_project_without_server_gameplay_rows(self):
        baseline = subject.project_encounter(self.document, ROOT)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.write_light_catalog(root)
            for anchor in ("PLAYER", "BOSS", "MAP"):
                with self.subTest(anchor=anchor):
                    document, box = self.light_document(anchor)
                    self.validate(document)
                    projected = subject.project_presentation(document, root)
                    row = next(row for pattern in projected["patterns"] for row in pattern["presentationOccurrences"]
                               if row["occurrenceId"] == box["occurrenceId"])
                    self.assertEqual(("LIGHT", anchor, 2.5), (row["kind"], row["anchorKind"], row["brightnessMultiplier"]))
                    self.assertEqual(baseline, subject.project_encounter(document, ROOT))

    def test_light_rejects_invalid_anchor_contract_before_projection(self):
        for changes in ({"bone": "bip001-head"}, {"followBoss": False}, {"scale": [2, 1, 1]},
                        {"anchorKind": "WORLD"}, {"brightnessMultiplier": float("nan")},
                        {"brightnessMultiplier": 17}, {"worldId": "world.bad"}):
            with self.subTest(changes=changes):
                document, box = self.light_document()
                box.update(changes)
                with self.assertRaises(subject.CompositionError):
                    self.validate(document)
        document, box = self.light_document()
        document["presentationResources"][-1].update(kind="CAMERA")
        with self.assertRaisesRegex(subject.CompositionError, "anchorKind"):
            self.validate(document)

    def test_light_catalog_changes_rejoin_without_composition_edits(self):
        document, _ = self.light_document()
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.write_light_catalog(root, revision=1)
            first = subject.project_presentation(document, root)
            self.write_light_catalog(root, revision=2)
            second = subject.project_presentation(document, root)
            self.assertEqual(first["patterns"], second["patterns"])
            self.assertEqual((1, 2), (first["lightResourceRevision"], second["lightResourceRevision"]))
            self.write_light_catalog(root, revision=3, lights=[])
            with self.assertRaisesRegex(subject.CompositionError, "missing Light Resources"):
                subject.project_presentation(document, root)

    def test_light_map_alias_joins_only_the_declared_area_source(self):
        document, _ = self.light_document("MAP", "light.map.stage")
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.write_light_catalog(root, lights=[])
            map_path = Path(f"Data/Maps/Authoring/{subject.AREA_ID}/{subject.AREA_ID}.maplights.json")
            (root / map_path).parent.mkdir(parents=True, exist_ok=True)
            source = subject.load_json(ROOT / subject.LIGHT_RESOURCES_PATH)["lights"][0]
            map_document = subject.load_json(ROOT / map_path)
            map_document["lights"] = [{**{key: value for key, value in source.items()
                                        if key not in {"lightResourceId", "defaultAnchorKind", "localOffset", "localRotationDegrees"}},
                                      "lightId": "light.map.stage", "groupId": "test", "enabled": True,
                                      "position": source["localOffset"], "rotationDegrees": source["localRotationDegrees"]}]
            (root / map_path).write_bytes(subject.serialize_json(map_document))
            with self.assertRaisesRegex(subject.CompositionError, "missing Light Resources"):
                subject.project_presentation(document, root)
            catalog = {"areas": [{"id": subject.AREA_ID, "sourceLights": map_path.as_posix(),
                                   "lights": f"Client/Bin/DataFiles/Map/{subject.AREA_ID}.maplights.json"}]}
            (root / "Data/Maps/MapCatalog.json").write_bytes(subject.serialize_json(catalog))
            self.assertEqual(1, subject.project_presentation(document, root)["lightResourceRevision"])
            template = copy.deepcopy(map_document["lights"][0])
            map_document["lights"].extend(
                {**copy.deepcopy(template), "lightId": f"light.map.extra.{ordinal}"}
                for ordinal in range(1, 512)
            )
            (root / map_path).write_bytes(subject.serialize_json(map_document))
            self.assertEqual(1, subject.project_presentation(document, root)["lightResourceRevision"])
            map_document["lights"].append({**template, "lightId": "light.map.overflow"})
            (root / map_path).write_bytes(subject.serialize_json(map_document))
            with self.assertRaisesRegex(subject.CompositionError, "0 to 512 entries"):
                subject.project_presentation(document, root)
            map_document["lights"] = []
            (root / map_path).write_bytes(subject.serialize_json(map_document))
            with self.assertRaisesRegex(subject.CompositionError, "missing Light Resources"):
                subject.project_presentation(document, root)

    def test_retarget_on_enter_projects_existing_stage_action_and_rejects_yaw_conflict(self):
        document = copy.deepcopy(self.document)
        pattern = self.first_product(document)
        stage = pattern["stages"][0]
        self.assertNotIn("actions", subject._project_stage(stage))
        stage["retargetOnEnter"] = True
        self.validate(document)
        self.assertEqual([{"trigger": "ENTER", "kind": "RETARGET_RANDOM_ALIVE",
                          "targetId": "boss.target.pattern", "value": 1, "durationMs": 0}],
                         subject._project_stage(stage)["actions"])
        for value in (None, 0, 1, "true", [], {}):
            rejected = copy.deepcopy(document)
            self.first_product(rejected)["stages"][0]["retargetOnEnter"] = value
            with self.subTest(value=value), self.assertRaisesRegex(subject.CompositionError, "retargetOnEnter"):
                self.validate(rejected)
        rejected = copy.deepcopy(document)
        self.first_product(rejected)["bossMotion"] = copy.deepcopy(
            self.find(document, "KAKULSAYDON_G1_PATTERN_8")["bossMotion"])
        with self.assertRaisesRegex(subject.CompositionError, "retargetOnEnter.*bossMotion"):
            self.validate(rejected)
        stage["retargetOnEnter"] = False
        self.validate(document)
        self.assertNotIn("actions", subject._project_stage(stage))

    def test_animation_root_vertical_scale_projects_only_its_actions(self):
        document = copy.deepcopy(self.document)
        pattern = self.find(document, "KAKULSAYDON_G1_PATTERN_8")
        pattern["authoringStatus"] = "PRODUCT"
        pattern["animationRootVerticalScale"] = .8
        document["playAllPatternIds"] = [p["patternId"] for p in document["patterns"] if p["authoringStatus"] == "PRODUCT"]
        self.validate(document)
        actions = {stage["actionId"] for stage in pattern["stages"]}
        product = subject.project_presentation(document)
        self.assertTrue(all(row["animationRootVerticalScale"] == .8 for row in product["bindings"] if row["actionId"] in actions))
        self.assertTrue(all("animationRootVerticalScale" not in row for row in product["bindings"] if row["actionId"] not in actions))
        self.assertEqual(.8, self.find(product, pattern["patternId"])["animationRootVerticalScale"])
        for scale in (None, True, "0.8", -.1, 1.1, float("nan"), float("inf")):
            pattern["animationRootVerticalScale"] = scale
            with self.subTest(scale=scale), self.assertRaisesRegex(subject.CompositionError, "animationRootVerticalScale"):
                self.validate(document)
        pattern.pop("animationRootVerticalScale")
        self.assertTrue(all("animationRootVerticalScale" not in row for row in subject.project_presentation(document)["bindings"]))

    def test_animation_root_vertical_scale_matches_original_rise_and_cache_isolation(self):
        pattern = self.find(self.document, "KAKULSAYDON_G1_PATTERN_8")
        actor = subject._load_bone_bake_actor(pattern, ROOT, {})
        root_index = next(i for i, bone in enumerate(actor["body"].skeleton_bones) if bone.name == "b_root")
        clip = "rpcz00_att_battle_7_01"
        def pose(seconds, scale):
            return subject._sample_bone_bake_pose(actor, "body", clip, seconds, scale)[root_index]
        original = pose(163 / 30, 1.0)
        lower = pose(163 / 30, .8)
        self.assertAlmostEqual(17.846225335, original[13], places=5)
        self.assertAlmostEqual(14.276980268, lower[13], places=5)
        self.assertEqual(original, pose(163 / 30, 1.0))
        self.assertEqual(lower, pose(163 / 30, .8))
        for before, after in zip(original[:13] + original[14:], lower[:13] + lower[14:]):
            self.assertAlmostEqual(before, after, places=5)  # Imported axis basis has sub-micrometre Z leakage.
        self.assertAlmostEqual(0.0, pose(6.1, .8)[13], places=5)
        self.assertAlmostEqual(0.0, pose(163 / 30, 0.0)[13], places=5)

    def test_boss_motion_projects_authoritative_and_presentation_origins(self):
        document = copy.deepcopy(self.document)
        pattern = self.find(document, "KAKULSAYDON_G1_PATTERN_8")
        pattern["authoringStatus"] = "PRODUCT"
        pattern["resetBossToSpawn"] = False
        pattern.pop("resetBossYawDegrees", None)
        motion = {"startMs": 1870, "endMs": 5780, "startPosition": [2.04, 10.56, 316.95],
                  "endPosition": [11.79, 10.56, 326.79], "yawDegrees": 314.7368}
        pattern["bossMotion"] = motion
        document["playAllPatternIds"] = [p["patternId"] for p in document["patterns"] if p["authoringStatus"] == "PRODUCT"]
        self.validate(copy.deepcopy(document))
        projected = next(p for p in subject.project_encounter(document)["patterns"] if p["patternId"] == "KAKULSAYDON_G1_PATTERN_8")
        self.assertEqual(motion, projected["bossMotion"])
        presentation = subject._project_pattern_presentation(document, pattern)
        self.assertEqual(motion, presentation["bossMotion"])
        box = pattern["worldOccurrences"][0]
        world = next(w for w in document["worlds"] if w["worldId"] == box["worldId"])
        world.update(anchorKind="BOSS_SPAWN", objectResourceId="test.object", anchorPosition=[1, 2, 3], positionOffset=[0, .5, 0])
        box.pop("placement", None)
        anchors = subject._project_pattern_presentation(document, pattern)["worldEmissionAnchors"]
        self.assertIn({"occurrenceId": box["occurrenceId"], "startMs": box["startMs"],
                       "anchorPosition": [1, 2, 3], "positionOffset": [0, .5, 0]}, anchors)
        projected["bossMotion"]["startPosition"][0] = 999
        self.assertEqual(2.04, pattern["bossMotion"]["startPosition"][0])

    def test_boss_motion_rejects_bad_intervals_height_and_two_position_writers(self):
        for field, value in (("startMs", True), ("startMs", 5780), ("endMs", 600000),
                             ("yawDegrees", float("nan")), ("yawDegrees", 361),
                             ("startPosition", [0, False, 0]), ("endPosition", [0, 11, 0])):
            document = copy.deepcopy(self.document)
            pattern = self.find(document, "KAKULSAYDON_G1_PATTERN_8")
            pattern["resetBossToSpawn"] = False
            pattern.pop("resetBossYawDegrees", None)
            pattern["bossMotion"] = {"startMs": 1870, "endMs": 5780, "startPosition": [2.04, 10.56, 316.95],
                                     "endPosition": [11.79, 10.56, 326.79], "yawDegrees": 314.7368}
            pattern["bossMotion"][field] = value
            with self.subTest(field=field, value=value), self.assertRaisesRegex(subject.CompositionError, "bossMotion"):
                self.validate(document)
        document = copy.deepcopy(self.document)
        pattern = self.find(document, "KAKULSAYDON_G1_PATTERN_8")
        pattern["resetBossToSpawn"] = True
        pattern["bossMotion"] = {"startMs": 0, "endMs": 100, "startPosition": [0, 0, 0],
                                 "endPosition": [1, 0, 1], "yawDegrees": 0}
        with self.assertRaisesRegex(subject.CompositionError, "bossMotion cannot also reset"):
            self.validate(document)

    def test_boss_spawn_reset_and_world_anchor_project(self):
        document = copy.deepcopy(self.document)
        pattern = self.find(document, ROULETTE_ID)
        pattern["resetBossToSpawn"] = True
        pattern["resetBossYawDegrees"] = 147.0
        world = next(w for w in document["worlds"] if w["worldId"] == pattern["worldOccurrences"][0]["worldId"])
        world.update(anchorKind="BOSS_SPAWN", anchorPosition=[-.319, 1.9, 737.531], positionOffset=[0, .58, 0])
        self.validate(copy.deepcopy(document))
        projected = next(p for p in subject.project_encounter(document)["patterns"] if p["patternId"] == ROULETTE_ID)
        self.assertTrue(projected["resetBossToSpawn"])
        self.assertEqual(147.0, projected["resetBossYawDegrees"])
        self.assertEqual("BOSS_SPAWN", projected["worldSequences"][0]["anchorKind"])
        self.assertEqual([-.319, 1.9, 737.531], projected["worldSequences"][0]["anchorPosition"])
        pattern["resetBossToSpawn"] = 1
        with self.assertRaisesRegex(subject.CompositionError, "resetBossToSpawn must be a boolean"):
            self.validate(document)

    def test_boss_spawn_reset_yaw_validates_and_absence_preserves_facing(self):
        document = copy.deepcopy(self.document)
        pattern = self.find(document, ROULETTE_ID)
        pattern["resetBossToSpawn"] = True
        for yaw in (0, -360, 360):
            pattern["resetBossYawDegrees"] = yaw
            self.validate(copy.deepcopy(document))
            projected = next(p for p in subject.project_encounter(document)["patterns"] if p["patternId"] == ROULETTE_ID)
            self.assertEqual(yaw, projected["resetBossYawDegrees"])
        for yaw in (None, True, "147", float("nan"), float("inf"), -361, 361):
            pattern["resetBossYawDegrees"] = yaw
            with self.subTest(yaw=yaw), self.assertRaisesRegex(subject.CompositionError, "resetBossYawDegrees"):
                self.validate(copy.deepcopy(document))
        pattern["resetBossYawDegrees"] = 147
        pattern["resetBossToSpawn"] = False
        with self.assertRaisesRegex(subject.CompositionError, "requires resetBossToSpawn"):
            self.validate(copy.deepcopy(document))
        pattern.pop("resetBossYawDegrees")
        self.validate(copy.deepcopy(document))
        projected = next(p for p in subject.project_encounter(document)["patterns"] if p["patternId"] == ROULETTE_ID)
        self.assertNotIn("resetBossYawDegrees", projected)

    def test_publish_is_deterministic_and_validate_detects_stale_product(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / subject.SOURCE_PATH
            source.parent.mkdir(parents=True)
            shutil.copy2(ROOT / subject.SOURCE_PATH, source)
            copy_repository_inputs(root)
            result = subject.run(root, "publish")
            self.assertEqual(2, result["outputCount"])
            product, inventory = subject.prepare_publication(subject.load_json(source), root)
            expected = subject.projected_outputs(product, root, inventory)
            subject.validate_outputs(root, expected)
            encounter = root / subject.ENCOUNTER_PATH
            encounter.write_text("{}\n", encoding="utf-8")
            with self.assertRaisesRegex(subject.CompositionError, "stale"):
                subject.validate_outputs(root, expected)

    def test_publish_rolls_back_both_products_after_post_commit_failure(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            outputs = {
                Path("Data/first.json"): b'{"revision": 2}\n',
                Path("Data/second.json"): b'{"revision": 2}\n',
            }
            baselines = {}
            for relative in outputs:
                destination = root / relative
                destination.parent.mkdir(parents=True, exist_ok=True)
                baseline = b'{"revision": 1}\n'
                destination.write_bytes(baseline)
                baselines[relative] = baseline

            with mock.patch.object(
                subject,
                "validate_outputs",
                side_effect=subject.CompositionError("forced verification failure"),
            ):
                with self.assertRaisesRegex(
                    subject.CompositionError, "forced verification failure"
                ):
                    subject.publish_outputs(root, outputs)

            for relative, baseline in baselines.items():
                self.assertEqual(baseline, (root / relative).read_bytes())
            self.assertEqual([], list(root.rglob("*.rollback.*")))

    def test_publish_preserves_backup_and_reports_path_when_rollback_fails(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            relative = Path("Data/product.json")
            destination = root / relative
            destination.parent.mkdir(parents=True)
            destination.write_bytes(b'{"revision": 1}\n')
            real_replace = subject.os.replace

            def fail_rollback(source, target):
                if ".rollback." in Path(source).name:
                    raise OSError("forced rollback failure")
                return real_replace(source, target)

            with mock.patch.object(
                subject,
                "validate_outputs",
                side_effect=subject.CompositionError("forced verification failure"),
            ), mock.patch.object(subject.os, "replace", side_effect=fail_rollback):
                with self.assertRaisesRegex(
                    subject.CompositionError,
                    r"rollback was incomplete; recovery backup\(s\) preserved: .*\.rollback\.",
                ):
                    subject.publish_outputs(
                        root, {relative: b'{"revision": 2}\n'}
                    )

            backups = list(destination.parent.glob(".product.json.rollback.*"))
            self.assertEqual(1, len(backups))
            self.assertEqual(b'{"revision": 1}\n', backups[0].read_bytes())
            self.assertEqual(b'{"revision": 2}\n', destination.read_bytes())

    def test_linked_collider_reuses_duration_trigger_and_result_windows(self):
        for logic_type, field, kind in (("DURATION", "judgementKind", "AREA_OVERLAP"),
                                        ("TRIGGER", "triggerKind", "ENTER_AREA")):
            with self.subTest(kind=kind):
                document = copy.deepcopy(self.document)
                pattern = self.strip_lanes(self.first_product(document))
                logic_id = f"kakulsaydon.g1.logic.{document['nextLogicOrdinal']}"
                document["nextLogicOrdinal"] += 1
                document["logics"].append({"logicId":logic_id,"displayName":"Geometry judgement","logicType":logic_type,field:kind})
                resource_id = f"kakulsaydon.g1.presentation.{document['nextPresentationResourceOrdinal']}"
                document["nextPresentationResourceOrdinal"] += 1
                document["presentationResources"].append({"resourceId":resource_id,"displayName":"Geometry","kind":"COLLIDER","assetId":"",
                    "shape":"BOX","halfExtents":[2,1,3]})
                box_id = pattern["patternId"]+".logic.1"
                pattern["nextLogicOccurrenceOrdinal"] = 2
                pattern["logicOccurrences"] = [{"occurrenceId":box_id,"logicId":logic_id,"startMs":0,"durationMs":1000,
                    "onSuccessLogicIds":["kakulsaydon.g1.logic.4"],"onTimeoutLogicIds":["kakulsaydon.g1.logic.5"]}]
                pattern["nextPresentationOccurrenceOrdinal"] = 2
                pattern["presentationOccurrences"] = [{"occurrenceId":pattern["patternId"]+".presentation.1","resourceId":resource_id,
                    "startMs":0,"durationMs":1000,"logicOccurrenceId":box_id,"positionOffset":[0,0,5],"rotationDegrees":[0,90,0]}]
                self.validate(document)
                result = next(p for p in subject.project_encounter(document)["patterns"] if p["patternId"] == FIRST_PRODUCT_ID)
                self.assertEqual(1,len(result["logicWindows"]))
                self.assertEqual([],result["mechanicTriggers"])
                window = result["logicWindows"][0]
                self.assertEqual(kind,window["kind"])
                self.assertEqual("BOSS_CURRENT",window["cardRegions"][0]["anchorKind"])
                self.assertEqual([2,1,3],window["cardRegions"][0]["halfExtents"])
                self.assertEqual(1,len(window["onSuccess"]))
                pattern["presentationOccurrences"][0]["durationMs"] = 999
                with self.assertRaisesRegex(subject.CompositionError,"identical timing"):
                    self.validate(document)

    def capture_fixture(self):
        document = copy.deepcopy(self.document)
        pattern = self.strip_lanes(self.first_product(document))
        ordinal = document["nextLogicOrdinal"]
        trigger_id, hold_id, result_id = [f"kakulsaydon.g1.logic.{ordinal + i}" for i in range(3)]
        document["nextLogicOrdinal"] += 3
        document["logics"].extend([
            dict(logicId=trigger_id, displayName="Grab contact", logicType="TRIGGER", triggerKind="ENTER_AREA"),
            dict(logicId=hold_id, displayName="Hold deadline", logicType="DURATION", judgementKind="ATTACHMENT_HOLD"),
            dict(logicId=result_id, displayName="Left hand", logicType="RESULT", outcomeKind="CAPTURE_PLAYER",
                 attachmentSlot="BOSS_LEFT_HAND", gripLocalOffset=dict(forwardM=.2, upM=-.1, rightM=.3)),
        ])
        resource_id = f"kakulsaydon.g1.presentation.{document['nextPresentationResourceOrdinal']}"
        document["nextPresentationResourceOrdinal"] += 1
        document["presentationResources"].append(dict(resourceId=resource_id, displayName="Grab fan", kind="COLLIDER",
                                                     assetId="", shape="SECTOR", radiusM=4, halfAngleDegrees=45))
        trigger_box, hold_box = [pattern["patternId"] + f".logic.{i}" for i in (1, 2)]
        pattern["nextLogicOccurrenceOrdinal"] = 3
        pattern["logicOccurrences"] = [
            dict(occurrenceId=trigger_box, logicId=trigger_id, startMs=100, durationMs=300,
                 holdLogicOccurrenceId=hold_box, onSuccessLogicIds=[result_id]),
            dict(occurrenceId=hold_box, logicId=hold_id, startMs=100, durationMs=900),
        ]
        pattern["nextPresentationOccurrenceOrdinal"] = 2
        pattern["presentationOccurrences"] = [dict(occurrenceId=pattern["patternId"] + ".presentation.1",
            resourceId=resource_id, startMs=100, durationMs=300, logicOccurrenceId=trigger_box, followBoss=True)]
        return document, pattern, document["logics"][-1]

    def test_capture_projects_independent_hold_deadline_and_single_pattern_grip(self):
        document, pattern, result = self.capture_fixture()
        self.validate(document)
        server = next(row for row in subject.project_encounter(document)["patterns"] if row["patternId"] == pattern["patternId"])
        trigger, hold = server["logicWindows"]
        self.assertEqual(hold["windowId"], trigger["holdLogicOccurrenceId"])
        self.assertEqual((100, 300, 100, 900), (trigger["startMs"], trigger["durationMs"], hold["startMs"], hold["durationMs"]))
        self.assertEqual("ATTACHMENT_HOLD", hold["kind"])
        self.assertEqual(([], [], [], []), (hold["cardRegions"], hold["onSuccess"], hold["onFail"], hold["onTimeout"]))
        self.assertEqual(result["gripLocalOffset"], trigger["onSuccess"][0]["gripLocalOffset"])
        grip = next(row for row in subject.project_presentation(document)["attachmentGrips"] if row["patternId"] == pattern["patternId"])
        self.assertEqual([.2, -.1, .3], grip["gripLocalOffset"])
        self.assertEqual("BOSS_LEFT_HAND", grip["attachmentSlot"])

    def test_capture_rejects_bad_hold_links_outcomes_regions_and_grip(self):
        for bad in ("missing", "wrong_kind", "late_start", "early_end", "disabled", "hold_outcome", "hold_collider",
                    "mixed_success", "timeout_capture", "bad_slot", "offset_range", "offset_schema", "missing_offset"):
            document, pattern, result = self.capture_fixture()
            trigger, hold = pattern["logicOccurrences"]
            if bad == "missing": trigger.pop("holdLogicOccurrenceId")
            elif bad == "wrong_kind": trigger["holdLogicOccurrenceId"] = trigger["occurrenceId"]
            elif bad == "late_start": hold["startMs"] = 101
            elif bad == "early_end": hold["durationMs"] = 299
            elif bad == "disabled": hold["enabled"] = False
            elif bad == "hold_outcome": hold["onSuccessLogicIds"] = [result["logicId"]]
            elif bad == "hold_collider":
                pattern["presentationOccurrences"][0].update(logicOccurrenceId=hold["occurrenceId"], durationMs=900)
            elif bad == "mixed_success": trigger["onSuccessLogicIds"].append("kakulsaydon.g1.logic.4")
            elif bad == "timeout_capture": trigger["onTimeoutLogicIds"] = trigger.pop("onSuccessLogicIds")
            elif bad == "bad_slot": result["attachmentSlot"] = "NONE"
            elif bad == "offset_range": result["gripLocalOffset"]["forwardM"] = 10.01
            elif bad == "offset_schema": result["gripLocalOffset"]["x"] = 0
            elif bad == "missing_offset": result.pop("gripLocalOffset")
            with self.subTest(bad=bad), self.assertRaises(subject.CompositionError):
                self.validate(document)

    def test_capture_draft_preserves_incomplete_wiring_but_rejects_conflicting_product_grips(self):
        document, pattern, result = self.capture_fixture()
        trigger = pattern["logicOccurrences"][0]
        pattern["authoringStatus"] = "DRAFT"
        pattern["logicOccurrences"][1]["startMs"] = 101
        document["playAllPatternIds"].remove(pattern["patternId"])
        self.validate(document)
        trigger.pop("holdLogicOccurrenceId")
        self.validate(document)
        document, pattern, result = self.capture_fixture()
        other = copy.deepcopy(result)
        other["logicId"] = f"kakulsaydon.g1.logic.{document['nextLogicOrdinal']}"
        document["nextLogicOrdinal"] += 1
        document["logics"].append(other)
        clone = copy.deepcopy(pattern["logicOccurrences"][0])
        clone.update(occurrenceId=pattern["patternId"] + ".logic.3", onSuccessLogicIds=[other["logicId"]])
        pattern["logicOccurrences"].append(clone)
        pattern["nextLogicOccurrenceOrdinal"] = 4
        # Two capture attempts may share one Hold and one grip.
        self.validate(document)
        self.assertEqual(1, sum(row["patternId"] == pattern["patternId"] for row in subject._project_attachment_grips(document)))
        other["gripLocalOffset"]["upM"] = 1
        with self.assertRaisesRegex(subject.CompositionError, "consistent capture grip"):
            self.validate(document)
        with self.assertRaisesRegex(subject.CompositionError, "consistent capture grip"):
            subject._project_attachment_grips(document)

    def reenter_damage_document(self):
        document = copy.deepcopy(self.document)
        pattern = self.strip_lanes(self.first_product(document))
        pattern["authoringStatus"] = "PRODUCT"
        document["playAllPatternIds"] = [row["patternId"] for row in self.product_patterns(document)]
        ordinal = document["nextLogicOrdinal"]
        trigger_id, result_id = (f"kakulsaydon.g1.logic.{ordinal + index}" for index in range(2))
        document["nextLogicOrdinal"] += 2
        document["logics"].extend([
            dict(logicId=trigger_id, displayName="Reenter hit", logicType="TRIGGER",
                 triggerKind="ENTER_AREA", rearmOnExit=True),
            dict(logicId=result_id, displayName="Hit and push", logicType="RESULT",
                 outcomeKind="MAX_HP_PERCENT_DAMAGE", percent=10, pushRangeM=2.0, pushMs=242),
        ])
        resource_id = f"kakulsaydon.g1.presentation.{document['nextPresentationResourceOrdinal']}"
        document["nextPresentationResourceOrdinal"] += 1
        document["presentationResources"].append(dict(resourceId=resource_id, displayName="Hit circle",
            kind="COLLIDER", assetId="", shape="CIRCLE", radiusM=3))
        window_id = pattern["patternId"] + ".logic.1"
        pattern["nextLogicOccurrenceOrdinal"] = pattern["nextPresentationOccurrenceOrdinal"] = 2
        pattern["logicOccurrences"] = [dict(occurrenceId=window_id, logicId=trigger_id,
            startMs=0, durationMs=1000, onSuccessLogicIds=[result_id])]
        pattern["presentationOccurrences"] = [dict(occurrenceId=pattern["patternId"] + ".presentation.1",
            resourceId=resource_id, startMs=0, durationMs=1000, logicOccurrenceId=window_id,
            anchorKind="BOSS", followBoss=True)]
        return document

    def test_elliptic_reverse_sector_and_forward_push_project_exact_axes(self):
        document = self.reenter_damage_document()
        resource = document["presentationResources"][-1]
        box = self.first_product(document)["presentationOccurrences"][0]
        box.update(scale=[0.2, 1, 4], rotationDegrees=[0, 725, 0])
        document["logics"][-1]["pushDirection"] = "BOSS_FORWARD"
        for shape, angle in (("SECTOR", 135), ("REVERSE_SECTOR", 0), ("REVERSE_SECTOR", 45), ("REVERSE_SECTOR", 180)):
            with self.subTest(shape=shape, halfAngle=angle):
                resource.update(shape=shape, halfAngleDegrees=angle)
                self.validate(document)
                window = self.first_product(subject.project_encounter(document))["logicWindows"][0]
                region = window["cardRegions"][0]
                self.assertEqual((shape, 725, angle), (region["shape"], region["yawDegrees"], region["halfAngleDegrees"]))
                self.assertAlmostEqual(.6, region["radiusXM"])
                self.assertEqual(12, region["radiusZM"])
                self.assertEqual("BOSS_FORWARD", window["onSuccess"][0]["pushDirection"])
        for invalid in ("PLAYER_FORWARD", "boss_forward", 1, None):
            document["logics"][-1]["pushDirection"] = invalid
            with self.subTest(direction=invalid), self.assertRaises(subject.CompositionError): self.validate(document)
        document["logics"][-1].update(pushDirection="BOSS_FORWARD", pushRangeM=0, pushMs=0)
        with self.assertRaises(subject.CompositionError): self.validate(document)

    def test_enter_area_rearm_and_damage_push_project_without_changing_legacy_defaults(self):
        document = self.reenter_damage_document()
        self.validate(document)
        window = self.first_product(subject.project_encounter(document))["logicWindows"][0]
        self.assertTrue(window["rearmOnExit"])
        self.assertEqual((10, 2.0, 242), tuple(window["onSuccess"][0][key]
            for key in ("percent", "pushRangeM", "pushMs")))
        self.assertEqual(("BOSS_CURRENT", 3), tuple(window["cardRegions"][0][key]
            for key in ("anchorKind", "radiusM")))
        trigger, result = document["logics"][-2:]
        for explicit_defaults in (False, True):
            if explicit_defaults:
                trigger["rearmOnExit"] = False
                result.update(pushRangeM=0, pushMs=0)
            else:
                trigger.pop("rearmOnExit", None)
                result.pop("pushRangeM", None)
                result.pop("pushMs", None)
            self.validate(document)
            legacy = self.first_product(subject.project_encounter(document))["logicWindows"][0]
            self.assertNotIn("rearmOnExit", legacy)
            self.assertNotIn("pushRangeM", legacy["onSuccess"][0])
            self.assertNotIn("pushMs", legacy["onSuccess"][0])
            self.assertEqual(10, legacy["onSuccess"][0]["percent"])

    def test_contact_repeats_after_knockback_with_one_positive_push_result(self):
        document = self.reenter_damage_document()
        trigger, result = document["logics"][-2:]
        trigger.pop("rearmOnExit")
        trigger["repeatAfterKnockback"] = True
        self.validate(document)
        window = self.first_product(subject.project_encounter(document))["logicWindows"][0]
        self.assertTrue(window["repeatAfterKnockback"])
        self.assertNotIn("rearmOnExit", window)
        for invalid in (dict(trigger, rearmOnExit=True), dict(trigger, repeatAfterKnockback=1),
                        dict(trigger, triggerKind="HUD_ENTER", hudMode="NONE")):
            with self.assertRaises(subject.CompositionError):
                subject._validate_logic_definition(invalid, "Continuous contact", document["nextLogicOrdinal"])
        result.update(pushRangeM=0, pushMs=0)
        with self.assertRaisesRegex(subject.CompositionError, "positive knockback"):
            self.validate(document)

    def test_rearm_and_push_reject_wrong_owner_type_bounds_and_unpaired_values(self):
        document = self.reenter_damage_document()
        trigger, result = document["logics"][-2:]
        invalid_rows = [dict(trigger, rearmOnExit=value) for value in (1, "true", None)]
        invalid_rows.append(dict(trigger, triggerKind="HUD_ENTER", hudMode="NONE"))
        invalid_rows.extend(dict(result, pushRangeM=value) for value in (-1, 20.01, float("inf"), float("nan"), True, "2"))
        invalid_rows.extend(dict(result, pushMs=value) for value in (-1, 600001, 0.5, True, "242"))
        invalid_rows.extend([dict(result, pushMs=0), dict(result, pushRangeM=0),
                             dict(result, outcomeKind="MADNESS_GAUGE_ADD_PERCENT")])
        for omitted in ("pushRangeM", "pushMs"):
            for other_value in (0, 2):
                invalid = dict(result, pushRangeM=other_value, pushMs=other_value)
                del invalid[omitted]
                invalid_rows.append(invalid)
        for row in invalid_rows:
            with self.subTest(row=row), self.assertRaises(subject.CompositionError):
                subject._validate_logic_definition(row, "Reentry/push fixture", document["nextLogicOrdinal"])
        for row in (dict(trigger, rearmOnExit=False), dict(result, pushRangeM=20, pushMs=600000),
                    dict(result, pushRangeM=0, pushMs=0)):
            subject._validate_logic_definition(row, "Reentry/push bounds", document["nextLogicOrdinal"])

    def test_rearm_and_push_bootstrap_sidecars_keep_existing_main_row_contract(self):
        projected = self.first_product(subject.project_encounter(self.reenter_damage_document()))
        publisher = (ROOT / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1").read_text(encoding="utf-8-sig")
        definitions = [re.search(r"(?ms)^function " + name + r"\b.*?^\}", publisher).group(0)
                       for name in ("Assert-ExactProperties", "Assert-StableId", "Assert-JsonString", "Assert-JsonInteger",
                                    "Assert-JsonNumber", "Format-InvariantFloat", "Format-InvariantSignedFloat")]
        start = publisher.index("\t$koukuWindowIds =")
        end = publisher.index("\tif ($koukuPattern.mechanicTriggers", start)
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)
            source = folder / "pattern.json"
            script = "$ErrorActionPreference='Stop'\n$stableIdPattern='^[A-Za-z0-9_.-]+$'\n" + "\n".join(definitions)
            script += "\n$koukuPattern=Get-Content -Raw (Join-Path $PSScriptRoot 'pattern.json') | ConvertFrom-Json\n"
            script += "$koukuPatternDurationMs=600000; $koukuEncounterDocument=[pscustomobject]@{encounterId='encounter.test'}\n"
            script += "$patternRows=[Collections.Generic.List[string]]::new(); $koukuFollowupTargets=[Collections.Generic.List[string]]::new()\n"
            script += publisher[start:end] + "\nConvertTo-Json -InputObject @($patternRows) -Compress\n"
            path = folder / "validate.ps1"
            path.write_text(script, encoding="utf-8-sig")
            def run(value):
                source.write_text(json.dumps(value), encoding="utf-8")
                return subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(path)],
                                      capture_output=True, text=True, timeout=45)
            result = run(projected)
            self.assertEqual(0, result.returncode, result.stderr)
            rows = [row.split("\t") for row in json.loads(result.stdout)]
            self.assertEqual(22, len(next(row for row in rows if row[0] == "PATTERNLOGIC")))
            self.assertEqual(10, len(next(row for row in rows if row[0] == "PATTERNLOGICOUTCOME")))
            rearm = next(row for row in rows if row[0] == "PATTERNLOGICREARM")
            push = next(row for row in rows if row[0] == "PATTERNLOGICPUSH")
            self.assertEqual((5, "ON_REENTER"), (len(rearm), rearm[-1]))
            self.assertEqual(8, len(push))
            self.assertEqual(["SUCCESS", "0", "2", "242"], push[-4:])
            self.assertEqual(rearm[1:4], push[1:4])
            forward = copy.deepcopy(projected)
            forward_window = forward["logicWindows"][0]
            forward_window["onSuccess"][0]["pushDirection"] = "BOSS_FORWARD"
            forward_region = forward_window["cardRegions"][0]
            forward_region.update(shape="REVERSE_SECTOR", halfAngleDegrees=0, radiusXM=.6, radiusZM=12)
            response = run(forward)
            self.assertEqual(0, response.returncode, response.stderr)
            forward_rows = [row.split("\t") for row in json.loads(response.stdout)]
            self.assertEqual("BOSS_FORWARD", next(row for row in forward_rows if row[0] == "PATTERNLOGICPUSH")[-1])
            self.assertEqual(9, len(next(row for row in forward_rows if row[0] == "PATTERNLOGICPUSH")))
            self.assertEqual(["0.6", "12"], next(row for row in forward_rows if row[0] == "PATTERNLOGICREGION")[-2:])
            self.assertEqual(21, len(next(row for row in forward_rows if row[0] == "PATTERNLOGICREGION")))
            for field, value in (("radiusXM", 0), ("radiusZM", -1), ("shape", "CIRCLE"), ("halfAngleDegrees", 180.1)):
                invalid = copy.deepcopy(forward)
                invalid["logicWindows"][0]["cardRegions"][0][field] = value
                with self.subTest(field=field): self.assertNotEqual(0, run(invalid).returncode)
            for value in ("PLAYER_FORWARD", 1, None):
                invalid = copy.deepcopy(forward)
                invalid["logicWindows"][0]["onSuccess"][0]["pushDirection"] = value
                with self.subTest(direction=value): self.assertNotEqual(0, run(invalid).returncode)
            for field, value in (("pushRangeM", -1), ("pushRangeM", 20.01), ("pushRangeM", 0),
                                 ("pushMs", 0), ("pushMs", 600001), ("pushMs", 0.5),
                                 ("kind", "MADNESS_GAUGE_ADD_PERCENT")):
                invalid = copy.deepcopy(projected)
                invalid["logicWindows"][0]["onSuccess"][0][field] = value
                with self.subTest(field=field, value=value):
                    self.assertNotEqual(0, run(invalid).returncode)
            for field, value in (("rearmOnExit", 1), ("kind", "AREA_OVERLAP")):
                invalid = copy.deepcopy(projected)
                invalid["logicWindows"][0][field] = value
                with self.subTest(field=field, value=value):
                    self.assertNotEqual(0, run(invalid).returncode)
            for omitted in ("pushRangeM", "pushMs"):
                invalid = copy.deepcopy(projected)
                invalid["logicWindows"][0]["onSuccess"][0].update(pushRangeM=0, pushMs=0)
                del invalid["logicWindows"][0]["onSuccess"][0][omitted]
                with self.subTest(omitted=omitted):
                    self.assertNotEqual(0, run(invalid).returncode)
            projected["logicWindows"][0]["rearmOnExit"] = False
            projected["logicWindows"][0]["onSuccess"][0].update(pushRangeM=0, pushMs=0)
            result = run(projected)
            self.assertEqual(0, result.returncode, result.stderr)
            self.assertFalse(any(row.startswith(("PATTERNLOGICREARM\t", "PATTERNLOGICPUSH\t"))
                                 for row in json.loads(result.stdout)))

    def test_enter_area_charge_uses_its_window_and_rejects_motion_conflicts(self):
        document = copy.deepcopy(self.document)
        pattern = self.strip_lanes(self.first_product(document))
        pattern.pop("bossMotion", None)
        for stage in pattern["stages"]:
            stage.pop("retargetOnEnter", None)
        logic_id = f"kakulsaydon.g1.logic.{document['nextLogicOrdinal']}"
        document["nextLogicOrdinal"] += 1
        document["logics"].append({"logicId": logic_id, "displayName": "Body charge", "logicType": "TRIGGER",
                                   "triggerKind": "ENTER_AREA", "bossChargeDistanceM": 7, "chargeYawOffsetDegrees": 90})
        resource_id = f"kakulsaydon.g1.presentation.{document['nextPresentationResourceOrdinal']}"
        document["nextPresentationResourceOrdinal"] += 1
        document["presentationResources"].append({"resourceId": resource_id, "displayName": "Body", "kind": "COLLIDER",
                                                  "assetId": "", "shape": "CIRCLE", "radiusM": 1})
        box_id = pattern["patternId"] + ".logic.1"
        pattern["nextLogicOccurrenceOrdinal"] = 2
        pattern["logicOccurrences"] = [{"occurrenceId": box_id, "logicId": logic_id, "startMs": 0, "durationMs": 1000}]
        pattern["nextPresentationOccurrenceOrdinal"] = 2
        pattern["presentationOccurrences"] = [{"occurrenceId": pattern["patternId"] + ".presentation.1", "resourceId": resource_id,
            "startMs": 0, "durationMs": 1000, "logicOccurrenceId": box_id, "anchorKind": "BOSS", "followBoss": True}]
        self.validate(document)
        row = next(row for row in subject.project_encounter(document)["patterns"] if row["patternId"] == FIRST_PRODUCT_ID)
        self.assertEqual((0, 1000, 7, 90), tuple(row["logicWindows"][0][key] for key in ("startMs", "durationMs", "bossChargeDistanceM", "chargeYawOffsetDegrees")))
        for distance, yaw in ((7, 361), (7, float("nan")), (0, 90)):
            invalid = copy.deepcopy(document)
            next(x for x in invalid["logics"] if x["logicId"] == logic_id).update(bossChargeDistanceM=distance, chargeYawOffsetDegrees=yaw)
            with self.subTest(distance=distance, yaw=yaw), self.assertRaises(subject.CompositionError):
                self.validate(invalid)
        self.assertEqual("BOSS_CURRENT", row["logicWindows"][0]["cardRegions"][0]["anchorKind"])
        for conflict in ("overlap", "bossMotion", "retarget"):
            invalid = copy.deepcopy(document)
            owner = self.first_product(invalid)
            if conflict == "overlap":
                owner["logicOccurrences"].append({**owner["logicOccurrences"][0], "occurrenceId": owner["patternId"] + ".logic.2"})
                owner["nextLogicOccurrenceOrdinal"] = 3
            elif conflict == "bossMotion":
                owner["bossMotion"] = {"startMs": 0, "endMs": 1000, "startPosition": [0, 0, 0], "endPosition": [0, 0, 7], "yawDegrees": 0}
            else:
                owner["stages"][0]["retargetOnEnter"] = True
            with self.subTest(conflict=conflict), self.assertRaisesRegex(subject.CompositionError, "[Cc]harge"):
                self.validate(invalid)
        invalid = copy.deepcopy(document)
        next(row for row in invalid["logics"] if row["logicId"] == logic_id)["bossChargeDistanceM"] = -1
        with self.assertRaises(subject.CompositionError):
            self.validate(invalid)

    def test_circle_fail_duration_preserves_authored_result_and_debug_render(self):
        document = copy.deepcopy(self.document)
        pattern = self.find(document,ROULETTE_ID)
        circle = next(row for row in pattern["presentationOccurrences"] if row["logicOccurrenceId"] == pattern["logicOccurrences"][-1]["occurrenceId"])
        circle["scale"] = [3,1,1]
        circle["debugRender"] = False
        self.validate(document)
        encounter = next(p for p in subject.project_encounter(document)["patterns"] if p["patternId"] == ROULETTE_ID)
        window = encounter["logicWindows"][-1]
        self.assertEqual(("AREA_OVERLAP","FAIL"),(window["kind"],window["insideOutcome"]))
        self.assertEqual("CIRCLE",window["cardRegions"][0]["shape"])
        self.assertAlmostEqual(24.0,window["cardRegions"][0]["radiusM"],places=3)
        self.assertEqual([50,50],[r["percent"] for r in window["onFail"]])
        flattened = next(p for p in subject.project_presentation(document)["patterns"] if p["patternId"] == ROULETTE_ID)
        self.assertFalse(next(r for r in flattened["presentationOccurrences"] if r["occurrenceId"] == circle["occurrenceId"])["debugRender"])
        invalid = copy.deepcopy(document)
        next(l for l in invalid["logics"] if l["logicId"] == pattern["logicOccurrences"][-1]["logicId"])["insideOutcome"] = "DAMAGE"
        with self.assertRaisesRegex(subject.CompositionError,"insideOutcome"):
            self.validate(invalid)

    def test_world_enter_trigger_projects_real_transform_keys_and_clock(self):
        document = copy.deepcopy(self.document)
        pattern = self.strip_lanes(self.first_product(document))
        logic_id = f"kakulsaydon.g1.logic.{document['nextLogicOrdinal']}"
        document["nextLogicOrdinal"] += 1
        document["logics"].append({"logicId":logic_id,"displayName":"WORLD entry","logicType":"TRIGGER","triggerKind":"ENTER_AREA"})
        resource_id = f"kakulsaydon.g1.presentation.{document['nextPresentationResourceOrdinal']}"
        document["nextPresentationResourceOrdinal"] += 1
        document["presentationResources"].append({"resourceId":resource_id,"displayName":"Trigger box","kind":"COLLIDER","assetId":"","resourceKind":"","shape":"BOX"})
        box_id = pattern["patternId"]+".logic.1"
        pattern["nextLogicOccurrenceOrdinal"] = pattern["nextPresentationOccurrenceOrdinal"] = pattern["nextWorldOccurrenceOrdinal"] = 2
        pattern["logicOccurrences"] = [{"occurrenceId":box_id,"logicId":logic_id,"startMs":200,"durationMs":1000,
            "onSuccessLogicIds":["kakulsaydon.g1.logic.4"]}]
        pattern["worldOccurrences"] = [{"occurrenceId":pattern["patternId"]+".world.1","worldId":"kakulsaydon.g1.world.1","startMs":200,"durationMs":1200,"playbackSpeed":2}]
        pattern["presentationOccurrences"] = [{"occurrenceId":pattern["patternId"]+".presentation.1","resourceId":resource_id,"startMs":200,"durationMs":1000,
            "logicOccurrenceId":box_id,"anchorKind":"WORLD","worldId":"kakulsaydon.g1.world.1","positionOffset":[0,0,1],"rotationDegrees":[0,90,0]}]
        self.validate(document)
        sequences = subject.load_world_sequences(ROOT,subject.AREA_ID)
        instance = next(r for r in sequences["instances"] if r["instanceId"] == "world.sequence.instance.8")
        instance["startDelayMs"] = 100
        template = next(r for r in sequences["templates"] if r["sequenceId"] == instance["templateId"])
        template["durationMs"] = 1100
        template["interpolation"] = "SMOOTH_STEP"
        keys = [{"timeMs":0,"positionOffset":[0,0,0],"rotationQuaternion":[0,0,0,1],"scaleMultiplier":[1,1,1],"visible":True},
                {"timeMs":1100,"positionOffset":[10,0,0],"rotationQuaternion":[0,0.7071067811865475,0,0.7071067811865475],"scaleMultiplier":[2,2,2],"visible":True}]
        template["tracks"][0]["keys"] = keys
        with mock.patch.object(subject,"load_world_sequences",return_value=sequences):
            output = next(r for r in subject.project_encounter(document)["patterns"] if r["patternId"] == FIRST_PRODUCT_ID)
        region = output["logicWindows"][0]["cardRegions"][0]
        self.assertEqual([0,0,1],region["center"])
        self.assertEqual(90,region["yawDegrees"])
        track = region["worldTrack"]
        self.assertEqual((200,100,1100,2.0,"SMOOTH_STEP"),(track["startMs"],track["startDelayMs"],track["durationMs"],track["playbackSpeed"],track["interpolation"]))
        self.assertEqual([4,4,4],track["baselineScale"])
        self.assertEqual([10,0,0],track["keys"][1]["positionOffset"])
        self.assertAlmostEqual(0.7071067811865475,track["keys"][1]["rotationY"])
        self.assertEqual("MAX_HP_PERCENT_DAMAGE",output["logicWindows"][0]["onSuccess"][0]["kind"])
        keys[1]["timeMs"] = 0
        with mock.patch.object(subject,"load_world_sequences",return_value=sequences):
            with self.assertRaisesRegex(subject.CompositionError,"increasing"):
                subject.project_encounter(document)

    def test_world_companion_effect_is_explicit_and_keeps_independent_timing(self):
        document = copy.deepcopy(self.document)
        pattern = self.strip_lanes(self.first_product(document))
        resource_id = f"kakulsaydon.g1.presentation.{document['nextPresentationResourceOrdinal']}"
        document["nextPresentationResourceOrdinal"] += 1
        document["presentationResources"].append({"resourceId":resource_id,"displayName":"Curtain companion",
            "kind":"EFFECT","assetId":"boss.kouku.curtain_1","resourceKind":"LEAF"})
        world_id = f"kakulsaydon.g1.world.{document['nextWorldOrdinal']}"
        document["nextWorldOrdinal"] += 1
        document["worlds"].append({"worldId":world_id,"displayName":"Curtain World","sequenceInstanceId":document["worlds"][0]["sequenceInstanceId"],
            "companionEffectResourceId":resource_id})
        world_box_id = pattern["patternId"]+".world.1"
        pattern["nextWorldOccurrenceOrdinal"] = pattern["nextPresentationOccurrenceOrdinal"] = 2
        pattern["worldOccurrences"] = [{"occurrenceId":world_box_id,"worldId":world_id,"startMs":0,"durationMs":1500,"playbackSpeed":1}]
        pattern["presentationOccurrences"] = [{"occurrenceId":pattern["patternId"]+".presentation.1","resourceId":resource_id,
            "startMs":100,"durationMs":900,"worldOccurrenceId":world_box_id}]
        self.validate(document)
        output = next(row for row in subject.project_presentation(document)["patterns"] if row["patternId"] == pattern["patternId"])
        self.assertEqual(1,len(output["presentationOccurrences"]))
        row = output["presentationOccurrences"][0]
        self.assertEqual((100,900,world_box_id),(row["startMs"],row["durationMs"],row["worldOccurrenceId"]))
        detached = copy.deepcopy(document)
        self.first_product(detached)["presentationOccurrences"] = []
        self.validate(detached)  # Existing World boxes are not auto-projected a second time.
        self.assertEqual([],next(row for row in subject.project_presentation(detached)["patterns"] if row["patternId"] == pattern["patternId"])["presentationOccurrences"])
        invalid = copy.deepcopy(document)
        self.first_product(invalid)["presentationOccurrences"][0]["worldOccurrenceId"] = pattern["patternId"]+".world.99"
        with self.assertRaisesRegex(subject.CompositionError,"same-pattern"):
            self.validate(invalid)
        invalid = copy.deepcopy(document)
        invalid["worlds"][-1]["companionEffectResourceId"] = "kakulsaydon.g1.presentation.999999"
        with self.assertRaisesRegex(subject.CompositionError,"EFFECT resource"):
            self.validate(invalid)
        invalid = copy.deepcopy(document)
        target = self.first_product(invalid)
        duplicate = copy.deepcopy(target["presentationOccurrences"][0])
        duplicate["occurrenceId"] = target["patternId"]+".presentation.2"
        target["nextPresentationOccurrenceOrdinal"] = 3
        target["presentationOccurrences"].append(duplicate)
        with self.assertRaisesRegex(subject.CompositionError,"at most one"):
            self.validate(invalid)

    def test_tracked_products_are_current(self):
        expected = subject.projected_outputs(self.document)
        subject.validate_outputs(ROOT, expected)


    def test_typed_trigger_projects_exact_target_and_rejects_missing_clone(self):
        document = self.without_catalog_boxes(copy.deepcopy(self.document))
        trigger = next(row for row in document["logics"] if row["logicId"] == "kakulsaydon.g1.logic.9")
        trigger.update(triggerKind="REAL_GAZE_TELEPORT", teleportPosition=[-6.36, 1.3, 937.92],
                       clonePatternId=FIRST_PRODUCT_ID, clockHours=[4, 7, 10])
        product = self.find(document, GAZE_ID)
        product["nextLogicOccurrenceOrdinal"] = 2
        product["presentationOccurrences"] = []
        product["logicOccurrences"] = [{"occurrenceId": GAZE_ID + ".logic.1",
            "logicId": trigger["logicId"], "startMs": 2000, "durationMs": 1000}]
        self.validate(document)
        projected = subject.project_encounter(document)
        row = next(p for p in projected["patterns"] if p["patternId"] == GAZE_ID)["mechanicTriggers"][0]
        self.assertEqual([-6.36, 1.3, 937.92], row["teleportPosition"])
        self.assertEqual([4, 7, 10], row["clockHours"])
        trigger["clonePatternId"] = "KAKULSAYDON_MISSING_CLONE"
        with self.assertRaisesRegex(subject.CompositionError, "clone pattern must be PRODUCT"):
            subject.project_encounter(document)
        trigger["clockHours"] = [5, 5, 10]
        with self.assertRaisesRegex(subject.CompositionError, "three different hours"):
            self.validate(document)

    def test_hud_enter_trigger_projects_mode_on_authored_start_tick(self):
        document = self.without_catalog_boxes(copy.deepcopy(self.document))
        trigger = next(row for row in document["logics"] if row["logicId"] == "kakulsaydon.g1.logic.9")
        for key in subject.LOGIC_TRIGGER_VALUE_KEYS:
            trigger.pop(key, None)
        trigger.update(triggerKind="HUD_ENTER", hudMode="MAZE")
        product = self.find(document, GAZE_ID)
        product["nextLogicOccurrenceOrdinal"] = 2
        product["presentationOccurrences"] = []
        product["logicOccurrences"] = [{"occurrenceId": GAZE_ID + ".logic.1",
            "logicId": trigger["logicId"], "startMs": 1333, "durationMs": 1000}]
        self.validate(document)
        row = next(p for p in subject.project_encounter(document)["patterns"]
                   if p["patternId"] == GAZE_ID)["mechanicTriggers"][0]
        self.assertEqual(("HUD_ENTER", "MAZE", 1333), (row["kind"], row["hudMode"], row["startMs"]))

    def test_scene_profile_reserved_blend_accepts_timeline_range_without_effect_envelope_bounds(self):
        document = copy.deepcopy(self.document)
        scene = self.find(document, GAZE_ID)["sceneProfileOccurrences"][0]
        scene["blendMs"] = 600000
        self.assertLess(scene["durationMs"], scene["blendMs"])
        self.validate(document)
        projected = next(row for row in subject.project_presentation(document)["patterns"]
                         if row["patternId"] == GAZE_ID)
        saved = next(row for row in projected["presentationOccurrences"]
                     if row["occurrenceId"] == scene["occurrenceId"])
        self.assertEqual(("SCENE_PROFILE", 600000, scene["durationMs"]),
                         (saved["kind"], saved["fadeInMs"], saved["durationMs"]))
        scene["blendMs"] = 600001
        with self.assertRaisesRegex(subject.CompositionError, "blendMs"):
            self.validate(document)
        scene["blendMs"] = 600000
        effect = next(row for row in self.first_product(document)["presentationOccurrences"]
                      if next(resource for resource in document["presentationResources"]
                              if resource["resourceId"] == row["resourceId"])["kind"] == "EFFECT")
        effect["fadeInMs"] = effect["durationMs"] + 1
        with self.assertRaisesRegex(subject.CompositionError, "fadeInMs"):
            self.validate(document)


    def direct_parent_document(self):
        document = copy.deepcopy(self.document)
        folder = {"folderId": f"kakulsaydon.folder.{document['nextFolderOrdinal']}",
                  "gateId": "GATE1", "displayName": "Independent outcomes"}
        document["nextFolderOrdinal"] += 1
        document["folders"].append(folder)
        self.first_product(document)["folderId"] = folder["folderId"]
        return document

    def test_direct_parent_pattern_projects_parent_without_creating_bundle_or_changing_playback(self):
        document = self.direct_parent_document()
        self.validate(document)
        before = subject.project_encounter(self.document)
        after = subject.project_encounter(document)
        expected = copy.deepcopy(before)
        expected["patterns"][0]["folderId"] = document["folders"][0]["folderId"]
        expected["folders"] = document["folders"]
        self.assertEqual(expected, after)
        self.assertEqual([], after["bundles"])
        self.assertEqual(before["playAllPatternIds"], after["playAllPatternIds"])
        self.assertTrue(all("folderId" not in row for row in before["patterns"]))

    def test_direct_parent_product_filter_keeps_referenced_folder_without_product_bundle(self):
        document = self.direct_parent_document()
        read_json = subject.load_json
        with mock.patch.object(subject, "load_json", side_effect=lambda path:
                               document if path == ROOT / subject.SOURCE_PATH else read_json(path)):
            product = subject.load_and_validate(ROOT)
        self.assertEqual(document["folders"], product["folders"])
        self.assertEqual(document["folders"][0]["folderId"], self.first_product(product)["folderId"])
        self.assertEqual([], product["bundles"])
        self.assertTrue(all(row["authoringStatus"] == "PRODUCT" for row in product["patterns"]))

    def test_direct_parent_rejects_invalid_missing_or_cross_gate_folder(self):
        for value in (None, [], True, "", "missing.parent"):
            with self.subTest(folderId=value):
                document = self.direct_parent_document()
                self.first_product(document)["folderId"] = value
                with self.assertRaises(subject.CompositionError):
                    self.validate(document)
        document = self.direct_parent_document()
        document["folders"][0]["gateId"] = "GATE2"
        with self.assertRaisesRegex(subject.CompositionError, "same-Gate Parent"):
            self.validate(document)

    def test_direct_parent_classification_survives_pattern_reuse_in_a_bundle(self):
        document = self.bundle_product_document()
        bundle = document["bundles"][0]
        pattern = self.find(document, bundle["members"][0]["patternId"])
        pattern["folderId"] = bundle["folderId"]
        self.validate(document)
        product = subject.project_encounter(document)
        projected = next(row for row in product["patterns"] if row["patternId"] == pattern["patternId"])
        self.assertEqual(bundle["folderId"], projected["folderId"])
        self.assertNotIn("folderId", product["bundles"][0]["members"][0])

    def bundle_product_document(self):
        document = copy.deepcopy(self.hierarchy_document)
        bundle = document["bundles"][0]
        # This fixture replaces the live clips, so their authored timing and
        # common camera lanes cannot be carried into the synthetic sequence.
        bundle["sceneProfileOccurrences"] = []
        bundle["presentationOccurrences"] = []
        bundle["nextSceneProfileOccurrenceOrdinal"] = 1
        bundle["nextPresentationOccurrenceOrdinal"] = 1
        for member in bundle["members"]:
            pattern = self.find(document, member["patternId"])
            self.strip_lanes(pattern)
            pattern.pop("bossMotion", None)
            pattern["stages"] = []
            stage = self.append_reference_sequence(pattern, pattern["actorProfileId"])
            clip = stage["animationOccurrences"][0]
            clip.update(startOffsetMs=0, endPolicy="EXACT")
            stage["animationOccurrences"] = [clip]
            stage["durationMs"] = clip["playMs"]
            pattern["authoringStatus"] = "PRODUCT"
        bundle["authoringStatus"] = "PRODUCT"
        document["playAllPatternIds"] = [p["patternId"] for p in document["patterns"] if p["authoringStatus"] == "PRODUCT"]
        return document

    def test_v3_seed_preserves_two_empty_gate2_drafts_and_has_no_published_bundle(self):
        document = copy.deepcopy(self.hierarchy_document)
        self.validate(document)
        self.assertEqual(3, document["formatVersion"])
        self.assertEqual(2, len(document["bundles"][0]["members"]))
        for member in document["bundles"][0]["members"]:
            pattern = self.find(document, member["patternId"])
            self.assertEqual(("GATE2", "DRAFT", []), (pattern["gateId"], pattern["authoringStatus"], pattern["stages"]))
        self.assertEqual([], subject.project_encounter(document)["bundles"])
        self.assertEqual([], subject.project_presentation(document)["folders"])
        self.assertEqual(document, json.loads(subject.serialize_json(document)))

    def test_v3_bundle_offsets_and_product_references_do_not_copy_child_timeline(self):
        document = self.bundle_product_document()
        document["bundles"][0]["members"][1]["startOffsetMs"] = 34
        before = copy.deepcopy(document["patterns"])
        self.validate(document)
        for product in (subject.project_encounter(document), subject.project_presentation(document)):
            bundle = product["bundles"][0]
            self.assertEqual(34, bundle["members"][1]["startOffsetMs"])
            self.assertEqual("boss.kakulsaydon.g2.big-saydon", bundle["members"][1]["targetBossPlacementId"])
            self.assertNotIn("stages", bundle["members"][0])
            self.assertEqual(subject._bundle_duration(document, document["bundles"][0]), bundle["durationMs"])
        self.assertEqual(before, document["patterns"])
        document["bundles"][0]["members"].clear()
        document["bundles"][0]["authoringStatus"] = "DRAFT"
        self.validate(document)
        self.assertEqual(before, document["patterns"])

    def test_v3_rejects_unknown_gate_target_cross_gate_and_duplicate_actor(self):
        for mutate in (
            lambda d: d["patterns"][0].update(gateId="GATE_UNKNOWN"),
            lambda d: d["patterns"][0].update(gateId=[]),
            lambda d: d["bundles"][0].update(authoringStatus=[]),
            lambda d: d["bundles"][0].update(folderId=[]),
            lambda d: d["bundles"][0]["members"][0].update(patternId=[]),
            lambda d: d["patterns"][0].update(targetBossPlacementId="boss.kakulsaydon.g3.saydon"),
            lambda d: d["bundles"][0].update(gateId="GATE1"),
            lambda d: d["bundles"][0]["members"][1].update(patternId=d["bundles"][0]["members"][0]["patternId"]),
            lambda d: d["bundles"][0]["members"][0].update(startOffsetMs=-1),
            lambda d: d["bundles"][0]["members"][0].update(startOffsetMs=True),
            lambda d: d["bundles"][0]["members"][0].update(patternId="missing.pattern"),
        ):
            document = copy.deepcopy(self.hierarchy_document)
            mutate(document)
            with self.assertRaises(subject.CompositionError): self.validate(document)

    def test_v3_empty_draft_and_shared_pattern_references_roundtrip(self):
        document = copy.deepcopy(self.hierarchy_document)
        duplicate = copy.deepcopy(document["bundles"][0])
        duplicate["bundleId"] = "kakulsaydon.bundle.2"
        for ordinal, member in enumerate(duplicate["members"], 1): member["memberId"] = f"kakulsaydon.bundle.2.member.{ordinal}"
        document["bundles"].append(duplicate); document["nextBundleOrdinal"] = 3
        self.validate(document)
        self.assertEqual(document, json.loads(subject.serialize_json(document)))
        document["bundles"].pop()
        document["bundles"][0]["members"] = []
        self.validate(document)
        document["bundles"][0]["authoringStatus"] = "PRODUCT"
        with self.assertRaisesRegex(subject.CompositionError, "at least one member"): self.validate(document)

    def test_v3_product_bundle_rejects_draft_child_and_dangling_delete(self):
        document = copy.deepcopy(self.hierarchy_document)
        document["bundles"][0]["authoringStatus"] = "PRODUCT"
        with self.assertRaisesRegex(subject.CompositionError, "DRAFT child"): self.validate(document)
        document["bundles"][0]["authoringStatus"] = "DRAFT"
        document["patterns"] = [p for p in document["patterns"] if p["patternId"] != "KAKULSAYDON_G1_PATTERN_8"]
        with self.assertRaisesRegex(subject.CompositionError, "missing pattern"): self.validate(document)

    def test_v3_common_scene_uses_reserved_blend_and_detects_child_conflict(self):
        document = self.bundle_product_document()
        bundle = document["bundles"][0]
        profile = document["sceneProfiles"][0]["sceneProfileId"]
        bundle["nextSceneProfileOccurrenceOrdinal"] = 2
        bundle["sceneProfileOccurrences"] = [dict(occurrenceId=bundle["bundleId"] + ".sceneprofile.1", sceneProfileId=profile,
                                                  startMs=0, durationMs=100, blendMs=600000)]
        self.validate(document)
        projected = subject.project_presentation(document)["bundles"][0]["presentationOccurrences"][0]
        self.assertEqual(("SCENE_PROFILE", 600000), (projected["kind"], projected["fadeInMs"]))
        child = self.find(document, bundle["members"][0]["patternId"])
        child["nextSceneProfileOccurrenceOrdinal"] = 2
        child["sceneProfileOccurrences"] = [dict(occurrenceId=child["patternId"] + ".sceneprofile.1", sceneProfileId=profile,
                                                 startMs=0, durationMs=100, blendMs=0)]
        with self.assertRaisesRegex(subject.CompositionError, "overlaps global SCENE_PROFILE"): self.validate(document)
        # 67 ms schedules at tick 3 (100 ms), so touching 100 ms boundaries do not overlap.
        bundle["members"][0]["startOffsetMs"] = 67
        self.validate(document)
        # Raw child [101, 201) misses common [202, 203), but quantized child [133 1/3, 233 1/3) overlaps.
        bundle["members"][0]["startOffsetMs"] = 1
        child["sceneProfileOccurrences"][0].update(startMs=100)
        bundle["sceneProfileOccurrences"][0].update(startMs=202, durationMs=1)
        with self.assertRaisesRegex(subject.CompositionError, "overlaps global SCENE_PROFILE"): self.validate(document)

    def test_v3_camera_common_rejects_unsupported_effect_and_malformed_duration(self):
        document = self.bundle_product_document()
        bundle = document["bundles"][0]
        effect = next(r["resourceId"] for r in document["presentationResources"] if r["kind"] == "EFFECT")
        bundle["nextPresentationOccurrenceOrdinal"] = 2
        bundle["presentationOccurrences"] = [dict(occurrenceId=bundle["bundleId"] + ".presentation.1", resourceId=effect,
                                                  startMs=0, durationMs=100)]
        with self.assertRaisesRegex(subject.CompositionError, "Camera only"): self.validate(document)
        bundle["presentationOccurrences"][0]["durationMs"] = "invalid"
        with self.assertRaises(subject.CompositionError): self.validate(document)
        for field in ("presentationOccurrences", "sceneProfileOccurrences"):
            for malformed in ([None], ["invalid"], [17], "invalid"):
                invalid = self.bundle_product_document()
                invalid["bundles"][0][field] = malformed
                with self.subTest(field=field, malformed=malformed), self.assertRaises(subject.CompositionError):
                    self.validate(invalid)

    def test_camera_bundle_conflict_includes_return_tail(self):
        document = self.bundle_product_document()
        bundle = document["bundles"][0]
        ordinal = document["nextPresentationResourceOrdinal"]
        document["nextPresentationResourceOrdinal"] += 1
        resource = dict(resourceId=f"kakulsaydon.g1.presentation.{ordinal}", displayName="Camera", kind="CAMERA", assetId="camera.test", resourceKind="")
        document["presentationResources"].append(resource)
        bundle["nextPresentationOccurrenceOrdinal"] = 2
        bundle["presentationOccurrences"] = [dict(occurrenceId=bundle["bundleId"] + ".presentation.1", resourceId=resource["resourceId"], startMs=0, durationMs=100)]
        child = self.find(document, bundle["members"][0]["patternId"])
        child["nextPresentationOccurrenceOrdinal"] = 2
        child["presentationOccurrences"] = [dict(occurrenceId=child["patternId"] + ".presentation.1", resourceId=resource["resourceId"], startMs=0, durationMs=100)]
        bundle["members"][0]["startOffsetMs"] = 100
        with mock.patch.object(subject, "load_camera_shots", return_value={"camera.test": {"blendInMs": 0, "blendOutMs": 50}}):
            with self.assertRaisesRegex(subject.CompositionError, "overlaps global CAMERA"): self.validate(document)
            # Offset 134 is quantized to 166 2/3 ms, safely past the [100,150) return tail.
            bundle["members"][0]["startOffsetMs"] = 134
            self.validate(document)
        with mock.patch.object(subject, "load_camera_shots", return_value={"camera.test": {"blendInMs": 101, "blendOutMs": 0}}):
            with self.assertRaisesRegex(subject.CompositionError, "shorter than.*blend-in"):
                subject.validate_publishable(document, ROOT)

    def test_v3_legacy_v2_has_no_hierarchy_dependency(self):
        document = copy.deepcopy(self.document)
        document["formatVersion"] = 2
        for key in ("folders", "bundles", "nextFolderOrdinal", "nextBundleOrdinal"): document.pop(key)
        for pattern in document["patterns"]:
            pattern.pop("gateId"); pattern.pop("targetBossPlacementId")
        self.validate(document)
        projected = subject.project_encounter(document)
        self.assertTrue(all(p["gateId"] == "GATE1" for p in projected["patterns"]))
        self.assertEqual([], projected["bundles"])


    def test_v3_multiple_shared_player_mode_owners_are_rejected(self):
        document = self.bundle_product_document()
        for member in document["bundles"][0]["members"]:
            child = self.find(document, member["patternId"])
            child["logicOccurrences"] = [dict(occurrenceId=child["patternId"] + ".logic.1", logicId="kakulsaydon.g1.logic.10",
                                               startMs=0, durationMs=1)]
            child["nextLogicOccurrenceOrdinal"] = 2
        with self.assertRaisesRegex(subject.CompositionError, "multiple owners of shared player mode"):
            self.validate(document)
        self.find(document, document["bundles"][0]["members"][1]["patternId"])["logicOccurrences"] = []
        self.validate(document)

    def test_v3_followup_must_stay_on_the_same_gate_and_boss(self):
        document = self.bundle_product_document()
        child = self.find(document, document["bundles"][0]["members"][0]["patternId"])
        child["logicOccurrences"] = [dict(occurrenceId=child["patternId"] + ".logic.1", logicId="kakulsaydon.g1.logic.1",
                                          startMs=0, durationMs=1, onSuccessLogicIds=["kakulsaydon.g1.logic.7"])]
        child["nextLogicOccurrenceOrdinal"] = 2
        with self.assertRaisesRegex(subject.CompositionError, "retain Gate and target boss"):
            self.validate(document)

    def test_v3_bundle_bootstrap_rows_match_server_contract_and_reject_wrong_target(self):
        document = self.bundle_product_document()
        parent_pattern = self.find(document, document["bundles"][0]["members"][0]["patternId"])
        parent_pattern["folderId"] = document["bundles"][0]["folderId"]
        encounter = subject.project_encounter(document)
        publisher = (ROOT / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1").read_text(encoding="utf-8-sig")
        definitions = []
        for name in ("Assert-ExactProperties", "Assert-StableId", "Assert-JsonString", "Assert-JsonInteger"):
            definitions.append(re.search(r"(?ms)^function " + name + r"\b.*?^\}", publisher).group(0))
        start = publisher.index("# Bundle members resolve")
        end = publisher.index("if (@($koukuEncounterDocument.playAllPatternIds).Count -ne", start)
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)
            product_path = folder / "product.json"
            product_path.write_text(json.dumps(encounter), encoding="utf-8")
            script = "$ErrorActionPreference='Stop'\n$stableIdPattern='^[A-Za-z0-9_.-]+$'\n" + "\n".join(definitions)
            script += "\n$koukuEncounterDocument=Get-Content -Raw (Join-Path $PSScriptRoot 'product.json') | ConvertFrom-Json\n"
            script += "$koukuPatternById=@{}; foreach($p in $koukuEncounterDocument.patterns){$koukuPatternById[[string]$p.patternId]=$p}\n"
            script += "$patternRows=[Collections.Generic.List[string]]::new()\n" + publisher[start:end]
            script += "\nConvertTo-Json -InputObject @($patternRows) -Compress\n"
            path = folder / "check.ps1"; path.write_text(script, encoding="utf-8-sig")
            run = lambda: subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(path)], capture_output=True, text=True)
            result = run()
            self.assertEqual(0, result.returncode, result.stderr)
            rows = json.loads(result.stdout)
            self.assertEqual(1, sum(row.startswith("PATTERNBUNDLE\t") for row in rows))
            self.assertEqual(2, sum(row.startswith("PATTERNBUNDLEMEMBER\t") for row in rows))
            self.assertEqual([4, 6, 6], [len(row.split("\t")) for row in rows])
            projected_parent = next(row for row in encounter["patterns"] if row["patternId"] == parent_pattern["patternId"])
            for invalid_parent in (None, [], "", "missing.parent"):
                projected_parent["folderId"] = invalid_parent
                product_path.write_text(json.dumps(encounter), encoding="utf-8")
                self.assertNotEqual(0, run().returncode)
            projected_parent["folderId"] = parent_pattern["folderId"]
            encounter["folders"][0]["gateId"] = "GATE1"
            product_path.write_text(json.dumps(encounter), encoding="utf-8")
            self.assertNotEqual(0, run().returncode)
            encounter["folders"][0]["gateId"] = "GATE2"
            encounter["bundles"][0]["members"][1]["targetBossPlacementId"] = "boss.kakulsaydon.g1.saydon"
            product_path.write_text(json.dumps(encounter), encoding="utf-8")
            self.assertNotEqual(0, run().returncode)

class KoukuPublishAllInventoryTests(unittest.TestCase):
    def source(self):
        source = subject.load_json(ROOT / subject.SOURCE_PATH)
        by_id = {p["patternId"]: p for p in source["patterns"]}
        source["patterns"] = [by_id[f"KAKULSAYDON_G1_PATTERN_{n}"] for n in (8, 9, 3)]
        for row in source["patterns"]:
            KoukuSaydonCompositionProjectionTests.strip_lanes(row)
            row["authoringStatus"] = "DRAFT"
            row.pop("folderId", None)
        source["patterns"][2]["stages"] = []
        for key in ("logics", "summons", "worlds", "sceneProfiles", "presentationResources"):
            source[key] = []
        source["folders"] = [f for f in source["folders"] if f["folderId"] in {"kakulsaydon.folder.1", "kakulsaydon.folder.4"}]
        source["bundles"] = [b for b in source["bundles"] if b["bundleId"] in {"kakulsaydon.bundle.1", "kakulsaydon.bundle.4"}]
        for row in source["bundles"]:
            row.update(authoringStatus="DRAFT", sceneProfileOccurrences=[], presentationOccurrences=[])
        source["playAllPatternIds"] = []
        return source

    def test_all_saved_rows_survive_and_ready_drafts_publish_without_source_mutation(self):
        source = self.source()
        original = copy.deepcopy(source)
        product, inventory = subject.prepare_publication(source)
        self.assertEqual(original, source)
        self.assertEqual([p["patternId"] for p in source["patterns"][:2]], product["playAllPatternIds"])
        self.assertEqual((3, 2, 2), tuple(len(inventory[k]) for k in ("patterns", "folders", "bundles")))
        self.assertEqual(["kakulsaydon.bundle.1"], [b["bundleId"] for b in product["bundles"]])
        self.assertIn("no stages", inventory["patterns"][2]["unavailableReason"])
        self.assertIn("at least one member", inventory["bundles"][1]["unavailableReason"])
        outputs = subject.projected_outputs(product, ROOT, inventory)
        encounter = json.loads(outputs[subject.ENCOUNTER_PATH])
        self.assertEqual(inventory, encounter["patternInventory"])
        self.assertNotIn("patternInventory", json.loads(outputs[subject.PRESENTATION_PATH]))
        for collection, key in (("patterns", "patternId"), ("bundles", "bundleId")):
            self.assertEqual({r[key] for r in encounter[collection]},
                             {r[key] for r in inventory[collection] if not r["unavailableReason"]})

    def test_category_is_preserved_and_unavailable_member_disables_only_its_bundle(self):
        source = self.source()
        source["patterns"][0]["category"] = "NORMAL"
        product, inventory = subject.prepare_publication(source)
        self.assertEqual("NORMAL", source["patterns"][0]["category"])
        self.assertEqual("NORMAL", inventory["patterns"][0]["category"])
        self.assertIn("MECHANIC category", inventory["patterns"][0]["unavailableReason"])
        self.assertEqual(["KAKULSAYDON_G1_PATTERN_9"], product["playAllPatternIds"])
        self.assertIn("child is unavailable", inventory["bundles"][0]["unavailableReason"])

    def test_followup_dependency_cannot_publish_without_its_incomplete_target(self):
        source = self.source()
        source["logics"] = [
            {"logicId": "kakulsaydon.g1.logic.1", "displayName": "Window", "logicType": "DURATION",
             "judgementKind": "STAGGER_WINDOW", "threshold": 1, "shieldArcDegrees": 0},
            {"logicId": "kakulsaydon.g1.logic.2", "displayName": "Follow-up", "logicType": "RESULT",
             "outcomeKind": "FOLLOWUP_PATTERN", "followupPatternId": "KAKULSAYDON_G1_PATTERN_3"},
        ]
        first = source["patterns"][0]
        first["nextLogicOccurrenceOrdinal"] = 2
        first["logicOccurrences"] = [{"occurrenceId": first["patternId"] + ".logic.1",
            "logicId": "kakulsaydon.g1.logic.1", "startMs": 0, "durationMs": 100,
            "onSuccessLogicIds": ["kakulsaydon.g1.logic.2"]}]
        product, inventory = subject.prepare_publication(source)
        self.assertEqual(["KAKULSAYDON_G1_PATTERN_9"], product["playAllPatternIds"])
        self.assertIn("no stages", inventory["patterns"][0]["unavailableReason"])
        self.assertIn("PATTERN_3", inventory["patterns"][0]["unavailableReason"])

    def test_ambiguous_tree_identity_fails_globally(self):
        source = self.source()
        source["patterns"].append(copy.deepcopy(source["patterns"][0]))
        with self.assertRaisesRegex(subject.CompositionError, "duplicate saved patternId"):
            subject.prepare_publication(source)

    def test_clone_dependency_is_admitted_together_and_incomplete_clone_is_isolated(self):
        source = self.source()
        source["logics"] = [{"logicId": "kakulsaydon.g1.logic.1", "displayName": "Clone", "logicType": "TRIGGER",
            "triggerKind": "REAL_GAZE_TELEPORT", "teleportPosition": [0, 0, 0],
            "clonePatternId": "KAKULSAYDON_G1_PATTERN_9", "clockHours": [4, 7, 10]}]
        first = source["patterns"][0]
        first.pop("bossMotion", None)
        first["nextLogicOccurrenceOrdinal"] = 2
        first["logicOccurrences"] = [{"occurrenceId": first["patternId"] + ".logic.1",
            "logicId": "kakulsaydon.g1.logic.1", "startMs": 0, "durationMs": 100}]
        product, inventory = subject.prepare_publication(source)
        self.assertEqual(2, len(product["patterns"]))
        self.assertEqual("", inventory["patterns"][0]["unavailableReason"])
        source["logics"][0]["clonePatternId"] = "KAKULSAYDON_G1_PATTERN_3"
        product, inventory = subject.prepare_publication(source)
        self.assertEqual(["KAKULSAYDON_G1_PATTERN_9"], product["playAllPatternIds"])
        self.assertIn("no stages", inventory["patterns"][0]["unavailableReason"])

    def test_publish_then_validate_uses_the_same_complete_inventory(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            copy_repository_inputs(root)
            path = root / subject.SOURCE_PATH
            path.parent.mkdir(parents=True)
            path.write_bytes(subject.serialize_json(self.source()))
            before = path.read_bytes()
            result = subject.run(root, "publish")
            self.assertEqual((3, 2, 2, 1), tuple(result[k] for k in
                ("savedPatternCount", "productPatternCount", "savedBundleCount", "productBundleCount")))
            self.assertEqual(result, subject.run(root, "validate"))
            self.assertEqual(before, path.read_bytes())
            old_outputs = {relative: (root / relative).read_bytes() for relative in (subject.ENCOUNTER_PATH, subject.PRESENTATION_PATH)}
            broken = self.source()
            broken["folders"][0]["gateId"] = "UNKNOWN_GATE"
            path.write_bytes(subject.serialize_json(broken))
            with self.assertRaises(subject.CompositionError):
                subject.run(root, "publish")
            self.assertEqual(old_outputs, {relative: (root / relative).read_bytes() for relative in old_outputs})


if __name__ == "__main__":
    unittest.main()
