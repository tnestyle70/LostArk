"""Prove exact pattern-sound coverage without adding a second audio invocation."""
import copy
import unittest
from Tools.ValtanPipeline import validate_valtan_hit_presentation_alignment as v


class CombatPatternSoundAliasTests(unittest.TestCase):
    def setUp(self):
        self.scope=("PATTERN", "STAGE", "action.test")
        self.cue=dict(bindingId="sound.inner",occurrenceId="sound.inner.occurrence",
            patternId="PATTERN",stageId="STAGE",actionId="action.test",clipOccurrenceId="clip.01",
            soundBank="S_Mob_G_Voltan2",soundEvent="G_Voltan2_Attack18_Shot5",repeatPolicy="once",startMs=1000)
        self.animation=dict(endPolicy="EXACT",repeatCount=1,occurrences=[dict(clipOccurrenceId="clip.01",
            clip="mesh_att_battle_19_04",mappingBasis="PROJECT_AUTHORED",sourceStartMs=0,playMs=2600,
            playRate=1.0,repeatUntilStageEnd=False)])
        self.spawn=dict(eventId="spawn.inner",trigger="ENTER",kind="SPAWN_COMBAT_OBJECT",
            combatObjectArchetypeId="object.in-out",count=1)
        self.hit=dict(hitId="hit.inner",trigger=dict(kind="TIMED",atMs=1000),repeat=dict(count=1,intervalMs=0))
        self.objects=dict(schema="lostark.valtan-combat-object-authoring",formatVersion=1,
            encounterId="ENCOUNTER_VALTAN",objects=[dict(combatObjectArchetypeId="object.in-out",
            kind="FIXED_AREA",lifetimeMs=4200,movement=dict(kind="STATIC"),hits=[self.hit])])
        self.stages={self.scope:dict(stage=dict(durationMs=2600,events=[self.spawn]),animation=self.animation)}
        self.occurrences={(*self.scope,"clip.01"):dict(row=self.animation["occurrences"][0],wallStartMs=0.,
            sourceStartMs=0,sourceEndMs=2600,playRate=1.)}
        self.sounds=dict(cues=[self.cue])
        self.receipt=dict(exceptionId="alias.inner",rule="COMBAT_OBJECT_HIT_PATTERN_SOUND",
            patternId="PATTERN",stageId="STAGE",actionId="action.test",bindingId="sound.inner",
            expectedHitOffsetsMs=[1000],reason="Same original inner hit sound is already played by the stage.",
            combatObjectArchetypeId="object.in-out",hitId="hit.inner",spawnEventId="spawn.inner",
            expectedSoundCue=copy.deepcopy(self.cue),expectedAnimation=copy.deepcopy(self.animation))
        self.allowlist=dict(schema="lostark.valtan-hit-presentation-alignment-allowlist",formatVersion=1,
            ownerArchetypeId="BOSS_VALTAN",exceptions=[self.receipt])
        self.object_sounds=dict(schema="lostark.valtan-combat-object-sound-cues",formatVersion=1,
            ownerArchetypeId="BOSS_VALTAN",cues=[])

    def validate(self):
        aliases=v._validate_allowlist(self.allowlist)[4]
        keys=v._validate_combat_pattern_sound_aliases(aliases,self.objects,self.sounds,self.stages,self.occurrences)
        return v._validate_combat_objects(self.objects,self.object_sounds,keys)

    def test_exact_existing_pattern_sound_covers_one_object_hit_without_mutation(self):
        before=copy.deepcopy((self.sounds,self.object_sounds,self.stages))
        hits,count=self.validate()
        self.assertEqual({("object.in-out","hit.inner")},set(hits))
        self.assertEqual(0,count)
        self.assertEqual(before,(self.sounds,self.object_sounds,self.stages))

    def test_missing_and_changed_source_sound_fail_closed(self):
        for field in ("soundBank","soundEvent","bindingId","occurrenceId","stageId","clipOccurrenceId","startMs","repeatPolicy"):
            with self.subTest(field=field):
                self.setUp()
                self.cue[field]=1001 if field=="startMs" else str(self.cue[field])+".changed"
                with self.assertRaises(v.ContractError):self.validate()
        self.setUp();self.sounds["cues"].clear()
        with self.assertRaises(v.ContractError):self.validate()

    def test_wrong_spawn_owner_clock_count_and_duplicate_owner_fail_closed(self):
        for field,value in (("trigger","HIT"),("count",2),("eventId","wrong"),("combatObjectArchetypeId","wrong")):
            with self.subTest(field=field):
                self.setUp();self.spawn[field]=value
                with self.assertRaises(v.ContractError):self.validate()
        self.setUp();self.spawn["offsetMs"]=1
        with self.assertRaises(v.ContractError):self.validate()
        self.setUp();self.stages[("OTHER","STAGE","action.other")]=copy.deepcopy(self.stages[self.scope])
        with self.assertRaises(v.ContractError):self.validate()

    def test_hit_timing_repeat_identity_and_animation_drift_fail_closed(self):
        mutations=(lambda: self.hit["trigger"].update(atMs=1001),
                   lambda: self.hit["repeat"].update(count=2),
                   lambda: self.hit.update(hitId="wrong"),
                   lambda: self.objects["objects"][0].update(lifetimeMs=900),
                   lambda: self.animation["occurrences"][0].update(playRate=2.),
                   lambda: self.occurrences[(*self.scope,"clip.01")].update(playRate=2.))
        for index,mutation in enumerate(mutations):
            with self.subTest(mutation=index):
                self.setUp();mutation()
                with self.assertRaises(v.ContractError):self.validate()

    def test_conditional_branch_cannot_borrow_stage_sound(self):
        self.stages[self.scope]["stage"]["branches"] = [{"condition": "EARLY_EXIT"}]
        with self.assertRaisesRegex(v.ContractError, "conditional branches"):
            self.validate()

    def test_duplicate_runtime_audio_and_stale_or_duplicate_alias_fail_closed(self):
        self.object_sounds["cues"].append(dict(bindingId="sound.object.inner",combatObjectArchetypeId="object.in-out",
            hitId="hit.inner",soundBank="S_Mob_G_Voltan2",soundEvent="G_Voltan2_Attack18_Shot5"))
        with self.assertRaisesRegex(v.ContractError,"duplicates runtime object audio"):self.validate()
        self.setUp();self.objects["objects"].clear()
        with self.assertRaises(v.ContractError):self.validate()
        self.setUp();duplicate=copy.deepcopy(self.receipt);duplicate["exceptionId"]="alias.other"
        self.allowlist["exceptions"].append(duplicate)
        with self.assertRaisesRegex(v.ContractError,"duplicate combat-object"):self.validate()


if __name__=="__main__":unittest.main()
