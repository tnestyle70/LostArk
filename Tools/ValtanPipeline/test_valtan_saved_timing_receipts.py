"""Exact saved V2 and delayed-wipe audio receipts stay tied to their owners."""
import copy
import unittest

from Tools.ValtanPipeline import validate_valtan_clip_template_parity as parity
from Tools.ValtanPipeline import validate_valtan_hit_presentation_alignment as alignment


class SavedTimingReceiptTests(unittest.TestCase):
    def setUp(self):
        self.scope = dict(patternId="VALTAN_SIX_PIZZA_106", stageId="STEP_07",
                          actionId="valtan.sequence.center-six-pizza-charge.step-07")
        self.occurrence = dict(clipOccurrenceId=self.scope["actionId"] + ".clip-01",
            clip="mesh_att_battle_12_07", sourceStartMs=0, playMs=1000, playRate=1.,
            mappingBasis="PROJECT_AUTHORED", repeatUntilStageEnd=False)
        self.hit = dict(shape=dict(kind="CIRCLE", outerRadiusM=25.),
            schedule=dict(kind="EXPLICIT_OFFSETS", offsetsMs=[250]),
            serverDamageProfileId="damage.valtan.super-smash", pushRangeM=3.,
            pushMs=242, knockdown=True, downMs=2000)
        self.stage = dict(stageId=self.scope["stageId"], actionId=self.scope["actionId"],
                          durationMs=1000, hit=self.hit)
        self.animation = dict(endPolicy="EXACT", repeatCount=1, occurrences=[self.occurrence])
        self.presentation_stage = dict(stageId=self.stage["stageId"], actionId=self.stage["actionId"],
                                       animation=self.animation)
        self.binding = dict(bindingId="binding.valtan.project-tuned.six-pizza.landing",
            resource=dict(kind="GROUP", id="boss.valtan.impact"), scope=copy.deepcopy(self.scope),
            clock=dict(basis="CLIP_OCCURRENCE", clipOccurrenceId=self.occurrence["clipOccurrenceId"],
                       startMs=479, repeatPolicy="ONCE"), anchor=dict(slotId="b_effectroot"), stopPolicy="NATURAL")
        self.sound = dict(bindingId="sound.landing", occurrenceId="sound.landing.01", **self.scope,
            clipOccurrenceId=self.occurrence["clipOccurrenceId"], soundBank="S_Mob_G_Voltan2",
            soundEvent="G_Voltan2_Attack11_Shot5", startMs=250, repeatPolicy="once")
        template_effect = dict(resourceKind="GROUP", resourceId="boss.valtan.impact",
                               clipMs=250, anchorSlotId="b_effectroot")
        self.override = dict(templateEffect=template_effect, bindingId=self.binding["bindingId"],
                             stageStartMs=479, bindingClock=copy.deepcopy(self.binding["clock"]))
        self.template = dict(templateId="template.landing", clip=self.occurrence["clip"],
            hits=[dict(clipMs=250, **{k: copy.deepcopy(v) for k, v in self.hit.items() if k != "schedule"})],
            effects=[copy.deepcopy(template_effect)], sounds=[dict(soundEvent=self.sound["soundEvent"], clipMs=250)])
        self.templates = dict(schema="lostark.valtan-clip-templates", formatVersion=1,
            ownerArchetypeId="BOSS_VALTAN", tickToleranceMs=33, templates=[self.template],
            allowlist=[dict(**self.scope, clipOccurrenceId=self.occurrence["clipOccurrenceId"], waivers=[],
                reason="Preserve this exact user-saved landing V2 clock while the original hit remains unchanged.",
                effectTimingOverride=self.override)])
        def source(schema, stage):
            return dict(schema=schema, formatVersion=1, bossArchetypeId="BOSS_VALTAN",
                        patterns=[dict(patternId=self.scope["patternId"], stages=[stage])])
        self.gameplay = source("lostark.valtan-gameplay-authoring", self.stage)
        self.presentation = source("lostark.valtan-pattern-presentation-authoring", self.presentation_stage)
        self.bindings = dict(schema="lostark.effect-v2-bindings", formatVersion=2,
                             archetypeId="BOSS_VALTAN", bindings=[self.binding])
        self.sounds = dict(schema="lostark.valtan-pattern-sound-cues", formatVersion=1,
                           ownerArchetypeId="BOSS_VALTAN", cues=[self.sound])

    def validate(self):
        parity_stats = parity.validate_parity(self.templates, self.gameplay, self.presentation,
                                             self.bindings, self.sounds)
        stats = alignment.validate_alignment(
            dict(schema="lostark.effect-v2-role-ledger", formatVersion=1, ownerArchetypeId="BOSS_VALTAN",
                 resources=[dict(kind="GROUP", id="boss.valtan.impact", role="ATTACK", alignmentPolicy="BINDING_START")]),
            dict(schema="lostark.valtan-hit-presentation-alignment-allowlist", formatVersion=1,
                 ownerArchetypeId="BOSS_VALTAN", exceptions=[]),
            self.gameplay, self.presentation, self.bindings, self.sounds,
            dict(schema="lostark.valtan-combat-object-authoring", formatVersion=1,
                 encounterId="ENCOUNTER_VALTAN", objects=[]),
            dict(schema="lostark.valtan-combat-object-sound-cues", formatVersion=1,
                 ownerArchetypeId="BOSS_VALTAN", cues=[]),
            dict(schema="lostark.boss-catalog", bosses=[dict(archetypeId="BOSS_VALTAN", combatObjectVisuals=[])]),
            self.templates)
        return parity_stats, stats

    def test_saved_clip_clock_keeps_original_hit_and_sound(self):
        before = copy.deepcopy((self.gameplay, self.presentation, self.bindings, self.sounds))
        parity_stats, stats = self.validate()
        self.assertEqual(1, parity_stats["effectTimingOverrides"])
        self.assertEqual(1, stats["effectTimingOverrides"])
        self.assertEqual(before, (self.gameplay, self.presentation, self.bindings, self.sounds))

    def test_wrong_binding_clock_scope_resource_anchor_and_duplicate_fail(self):
        mutations = (
            lambda: self.binding["clock"].update(startMs=480),
            lambda: self.binding["clock"].update(basis="STAGE", clipOccurrenceId=None),
            lambda: self.binding["scope"].update(stageId="STEP_08"),
            lambda: self.binding["resource"].update(id="boss.valtan.shout"),
            lambda: self.binding["anchor"].update(slotId="root"),
            lambda: self.bindings["bindings"].append(dict(copy.deepcopy(self.binding), bindingId="duplicate")),
        )
        for index, mutate in enumerate(mutations):
            with self.subTest(index=index):
                self.setUp()
                mutate()
                with self.assertRaises((parity.ContractError, alignment.ContractError)):
                    self.validate()

    def test_receipt_does_not_accept_stale_source_conversion_or_gameplay(self):
        mutations = (
            lambda: self.occurrence.update(sourceStartMs=1),
            lambda: self.occurrence.update(playRate=2.),
            lambda: self.occurrence.update(playMs=479),
            lambda: self.override.update(stageStartMs=480),
            lambda: self.override["bindingClock"].update(repeatPolicy="EACH_LOOP"),
            lambda: self.override["bindingClock"].update(clipOccurrenceId="another.clip"),
            lambda: self.hit["schedule"].update(offsetsMs=[479]),
            lambda: self.sound.update(startMs=479),
        )
        for index, mutate in enumerate(mutations):
            with self.subTest(index=index):
                self.setUp()
                mutate()
                with self.assertRaises((parity.ContractError, alignment.ContractError)):
                    self.validate()

    def test_clip_clock_receipt_validates_nonzero_trim_and_rate(self):
        self.occurrence.update(sourceStartMs=100, playRate=2.)
        self.override["stageStartMs"] = 190  # round((479 - 100) / 2)
        self.hit["schedule"]["offsetsMs"] = [75]  # (250 - 100) / 2
        self.validate()


class DelayedWipeSoundReceiptTests(unittest.TestCase):
    def setUp(self):
        self.scope = ("VALTAN_FLOOR_WIPE_130", "SECOND_SMASH", "valtan.mechanic.floor-wipe-130.second-smash")
        self.animation = dict(endPolicy="HOLD_LAST_POSE", repeatCount=1, occurrences=[dict(
            clipOccurrenceId=self.scope[2] + ".clip.01", clip="mesh_att_battle_15_03",
            sourceStartMs=0, playMs=500, playRate=1., repeatUntilStageEnd=False)])
        self.sound = dict(bindingId="sound.wipe", patternId=self.scope[0], stageId=self.scope[1],
                          actionId=self.scope[2], startMs=1, soundEvent="G_Voltan2_Attack25_Shot3")
        self.stages = {self.scope: dict(hitOffsetsMs=[500], animation=self.animation)}
        self.receipt = dict(exceptionId="exception.wipe", expectedHitOffsetsMs=[500],
            expectedAnimation=copy.deepcopy(self.animation), expectedSoundCues=[copy.deepcopy(self.sound)])

    def validate(self):
        alignment._validate_authored_sound_source(self.receipt, self.scope, self.stages,
            dict(cues=[self.sound]), {self.scope: [dict(wallMs=1., soundEvent=self.sound["soundEvent"])]})

    def test_delayed_damage_preserves_exact_sound_and_hold_tail(self):
        before = copy.deepcopy((self.sound, self.animation))
        self.validate()
        self.assertEqual(before, (self.sound, self.animation))

    def test_sound_hold_or_hit_drift_requires_a_new_reviewed_receipt(self):
        mutations = (lambda: self.sound.update(startMs=2), lambda: self.sound.update(soundEvent="other.Shot1"),
            lambda: self.animation.update(endPolicy="EXACT"), lambda: self.animation["occurrences"][0].update(playMs=501),
            lambda: self.stages[self.scope].update(hitOffsetsMs=[501]))
        for index, mutate in enumerate(mutations):
            with self.subTest(index=index):
                self.setUp()
                mutate()
                with self.assertRaises(alignment.ContractError):
                    self.validate()


if __name__ == "__main__":
    unittest.main()
