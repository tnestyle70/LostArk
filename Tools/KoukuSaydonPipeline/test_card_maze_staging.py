import copy
from pathlib import Path
from types import SimpleNamespace
import unittest
from unittest import mock

from Tools.KoukuSaydonPipeline import project_kouku_saydon_composition as subject


class CardMazeStagingTests(unittest.TestCase):
    def setUp(self):
        self.logic = dict(logicId="kakulsaydon.g1.logic.112", displayName="Entry",
            logicType="TRIGGER", triggerKind="CARD_MAZE_STAGE_PLAYERS",
            playerEntryEffectOccurrenceIds=["P.presentation.2", "P.presentation.1"])
        self.box = dict(startMs=0)
        self.resources = {"fx": dict(kind="EFFECT"), "collider": dict(kind="COLLIDER")}
        self.pattern = dict(presentationOccurrences=[dict(occurrenceId=f"P.presentation.{i}",
            resourceId="fx", startMs=3000+i*100, positionOffset=[float(i), 10., 320.],
            anchorKind="MAP", followBoss=False) for i in (1, 2)])

    def test_resolves_ordered_current_effect_positions_without_mutation(self):
        before = copy.deepcopy(self.pattern)
        self.assertEqual([[2.,10.,320.], [1.,10.,320.]], subject._project_card_maze_entry_positions(
            self.pattern, self.box, self.logic, self.resources))
        self.assertEqual(before, self.pattern)
        self.pattern["presentationOccurrences"][1]["positionOffset"][0] = 4.5
        self.assertEqual(4.5, subject._project_card_maze_entry_positions(
            self.pattern, self.box, self.logic, self.resources)[0][0])

    def test_definition_bounds_and_unrelated_fields(self):
        self.assertEqual("CARD_MAZE_STAGE_PLAYERS", subject._validate_logic_definition(self.logic, "entry", 113)[1]["kind"])
        for ids in ([], ["same", "same"], [str(i) for i in range(5)], [None], ["../bad"]):
            with self.subTest(ids=ids), self.assertRaises(subject.CompositionError):
                subject._validate_logic_definition(dict(self.logic, playerEntryEffectOccurrenceIds=ids), "entry", 113)
        with self.assertRaises(subject.CompositionError):
            subject._validate_logic_definition(dict(self.logic, teleportPosition=[0,0,0]), "entry", 113)

    def test_rejects_unresolved_moving_or_non_effect_anchors(self):
        for changes in (dict(resourceId="collider"), dict(anchorKind="BOSS"), dict(bone="head"),
                dict(followBoss=True), dict(worldId="world"), dict(worldOccurrenceId="instance"),
                dict(boneTarget="WEAPON"), dict(worldEmissionIndex=1), dict(logicOccurrenceId="logic"),
                dict(positionOffset=[float("nan"),0,0]), dict(startMs=-1), dict(occurrenceId="other")):
            with self.subTest(changes=changes):
                candidate=copy.deepcopy(self.pattern)
                candidate["presentationOccurrences"][0].update(changes)
                with self.assertRaises(subject.CompositionError):
                    subject._project_card_maze_entry_positions(candidate, self.box, self.logic, self.resources)


class BossMotionKeyTests(unittest.TestCase):
    def fixture(self):
        return dict(startMs=0,endMs=1000,startPosition=[1.,10.,3.],endPosition=[4.,6.,5.],
            yawDegrees=216.5,keys=[dict(timeMs=0,position=[1.,10.,3.]),
                dict(timeMs=500,position=[2.,9.,4.]),dict(timeMs=1000,position=[4.,6.,5.])])

    def test_sampled_xyz_path_and_legacy_constant_height(self):
        motion=self.fixture(); before=copy.deepcopy(motion)
        subject._validate_boss_motion(motion,1000,"test")
        self.assertEqual(before,motion)
        motion.pop("keys")
        with self.assertRaises(subject.CompositionError):subject._validate_boss_motion(motion,1000,"test")
        motion["endPosition"][1]=10
        subject._validate_boss_motion(motion,1000,"test")

    def test_rejects_invalid_key_order_count_endpoints_and_coordinates(self):
        changes=(lambda m:m.update(keys=[]),lambda m:m.update(keys=m["keys"][:1]),
            lambda m:m["keys"][0].update(timeMs=1),lambda m:m["keys"][-1].update(timeMs=999),
            lambda m:m["keys"][1].update(timeMs=0),lambda m:m["keys"][1].update(timeMs=1001),
            lambda m:m["keys"][1].update(timeMs=1.5),lambda m:m["keys"][1].update(position=[1,2]),
            lambda m:m["keys"][1].update(position=[1,2,float("inf")]),
            lambda m:m["keys"][0].update(position=[0,0,0]),lambda m:m["keys"][-1].update(position=[0,0,0]),
            lambda m:m["keys"][1].update(yawDegrees=90))
        for change in changes:
            with self.subTest(change=change):
                motion=self.fixture();change(motion)
                with self.assertRaises(subject.CompositionError):subject._validate_boss_motion(motion,1000,"test")


class BossAnimationDonorTests(unittest.TestCase):
    def fixture(self):
        profile="MN_RPCZ_00"; gate="GATE2"
        placement, archetype=subject.GATE_TARGETS[(gate,profile)]
        pattern=dict(patternId="P",actorProfileId=profile,gateId=gate,targetBossPlacementId=placement)
        body=subject.REFERENCE_MODEL_ASSET_IDS[profile]+".wmodel"
        catalog=dict(bosses=[dict(archetypeId=archetype, bodyModel=body,
            animationSetId="Character/Test/Donor.wmodel",bodyModelPreScale=.01)])
        bone=SimpleNamespace(name="b_root",parent=-1,transform=subject.wmodel_pose.affine_matrix((1,1,1),(0,0,0,1),(0,0,0)))
        def model(name):
            return SimpleNamespace(skeleton_bones=[copy.deepcopy(bone)],animations=[
                SimpleNamespace(name=name,duration_ticks=1000.,ticks_per_second=1000.,channels=[])])
        models={body:model("base"),"Character/Test/Donor.wmodel":model("donor")}
        calls=[]
        root=Path(".").resolve(); resources=root/"Client/Bin/Resources"
        def load(path, cache, clips=None):
            calls.append((str(path.relative_to(resources)).replace("\\","/"),clips))
            return models[calls[-1][0]]
        return pattern,catalog,models,calls,root,load

    def test_append_preserves_base_and_samples_donor_keys_from_donor_file(self):
        pattern,catalog,models,calls,root,load=self.fixture()
        with mock.patch.object(subject,"load_json",return_value=catalog), mock.patch.object(subject,"_load_bake_model",side_effect=load):
            actor=subject._load_bone_bake_actor(pattern,root,{})
            self.assertEqual(["base","donor"],[c.name for c in actor["body"].animations])
            self.assertEqual(["base"],[c.name for c in models[catalog["bosses"][0]["bodyModel"]].animations])
            subject._sample_bone_bake_pose(actor,"body","donor",.25,local_only=True)
            self.assertEqual(("Character/Test/Donor.wmodel",{"donor"}),calls[-1])

    def test_rejects_wrong_skeleton_duplicate_names_and_path_escape(self):
        for mode in ("skeleton","duplicate","escape"):
            pattern,catalog,models,calls,root,load=self.fixture()
            if mode=="skeleton":models["Character/Test/Donor.wmodel"].skeleton_bones[0].name="different"
            if mode=="duplicate":models["Character/Test/Donor.wmodel"].animations[0].name="base"
            if mode=="escape":catalog["bosses"][0]["animationSetId"]="../Donor.wmodel"
            with self.subTest(mode=mode), mock.patch.object(subject,"load_json",return_value=catalog), mock.patch.object(subject,"_load_bake_model",side_effect=load):
                with self.assertRaises(subject.CompositionError):subject._load_bone_bake_actor(pattern,root,{})


    def test_rejects_map_donor_before_reading_its_model(self):
        pattern,catalog,models,calls,root,load=self.fixture()
        catalog["bosses"][0]["animationSetId"]="Map/Test/Donor.wmodel"
        with mock.patch.object(subject,"load_json",return_value=catalog), mock.patch.object(subject,"_load_bake_model",side_effect=load):
            with self.assertRaisesRegex(subject.CompositionError,"Character/\\*.wmodel"):
                subject._load_bone_bake_actor(pattern,root,{})
        self.assertEqual([catalog["bosses"][0]["bodyModel"]],[asset for asset,_ in calls])

    def test_rejects_wrong_model_prefix_and_exact_extension_for_every_part(self):
        for field in ("bodyModel","weaponModel","animationSetId"):
            for asset in ("Map/Test/Donor.wmodel", "character/Test/Donor.wmodel",
                    "Character/Test/Donor.WMODEL", "Character/Test/Donor.wmodel.bak"):
                pattern,catalog,models,calls,root,load=self.fixture()
                catalog["bosses"][0][field]=asset
                with self.subTest(field=field,asset=asset), mock.patch.object(subject,"load_json",return_value=catalog), mock.patch.object(subject,"_load_bake_model",side_effect=load):
                    with self.assertRaises(subject.CompositionError):
                        subject._load_bone_bake_actor(pattern,root,{})
                self.assertNotIn(asset,[called for called,_ in calls])


if __name__ == "__main__":
    unittest.main()
