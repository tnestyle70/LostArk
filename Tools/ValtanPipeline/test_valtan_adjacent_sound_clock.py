"""Cut-boundary sound matching uses proven edges and exact authored receipts."""
import copy
import unittest
from Tools.ValtanPipeline import validate_valtan_hit_presentation_alignment as v


class AdjacentSoundClockTests(unittest.TestCase):
    def setUp(self):
        self.previous = ('P', 'PREV', 'action.prev')
        self.current = ('P', 'CURRENT', 'action.current')
        def info(duration, next_action):
            return dict(stage=dict(durationMs=duration, defaultNextActionId=next_action, branches=[]),
                animation=dict(endPolicy='EXACT', repeatCount=1, occurrences=[dict(
                    clip='clip.test', sourceStartMs=333, playMs=duration, playRate=1.,
                    repeatUntilStageEnd=False)]), hitOffsetsMs=[67])
        # Deliberately reversed insertion order: authority edges own the clock.
        self.stages = {self.current: info(200, None), self.previous: info(400, 'action.current')}
        self.cue = dict(bindingId='sound.previous', patternId='P', stageId='PREV',
            actionId='action.prev', startMs=376, soundEvent='Attack_Shot4')
        self.sounds = dict(cues=[self.cue])
        self.impacts = {self.previous: [dict(bindingId='sound.previous', wallMs=376., soundEvent='Attack_Shot4')]}
        self.receipt = dict(exceptionId='exception.previous', expectedHitOffsetsMs=[67],
            expectedSoundCues=[copy.deepcopy(self.cue)], expectedAnimation=copy.deepcopy(self.stages[self.current]['animation']),
            expectedSoundStage=dict(stageId='PREV', actionId='action.prev',
                expectedAnimation=copy.deepcopy(self.stages[self.previous]['animation'])))

    def validate(self):
        v._validate_authored_sound_source(self.receipt, self.current, self.stages, self.sounds, self.impacts)

    def test_previous_source_sound_remains_91ms_early_and_receipt_pins_it(self):
        before=copy.deepcopy((self.sounds, self.stages))
        self.validate()
        candidates=v._stage_impact_candidates(self.current,self.stages,self.impacts)
        self.assertEqual([-24.], [row['wallMs'] for row in candidates])
        self.assertEqual(91.,abs(candidates[0]['wallMs']-67))
        self.assertEqual(before,(self.sounds,self.stages))

    def test_true_boundary_sound_can_match_within_one_tick(self):
        self.impacts[self.previous][0]['wallMs']=392.
        candidates=v._stage_impact_candidates(self.current,self.stages,self.impacts)
        self.assertEqual(9.,abs(candidates[0]['wallMs']-1))

    def test_branches_reversed_edges_loops_repeat_and_ambiguous_owner_reject(self):
        mutations=(lambda:self.stages[self.previous]['stage'].update(branches=[dict(nextActionId='elsewhere')]),
            lambda:self.stages[self.current]['stage'].update(branches=[dict(nextActionId='elsewhere')]),
            lambda:self.stages[self.previous]['stage'].update(defaultNextActionId=None),
            lambda:self.stages[self.current]['stage'].update(defaultNextActionId='action.prev'),
            lambda:self.stages[self.previous]['animation'].update(repeatCount=2),
            lambda:self.stages[self.previous]['animation']['occurrences'][0].update(repeatUntilStageEnd=True),
            lambda:self.stages[self.previous]['animation']['occurrences'][0].update(playRate=2.),
            lambda:self.stages.update({('P','THIRD','action.third'):copy.deepcopy(self.stages[self.previous])}),
            lambda:self.stages.update({('P','THIRD','action.third'):dict(stage=dict(defaultNextActionId=None,
                branches=[dict(nextActionId='action.current')]))}))
        for index,mutation in enumerate(mutations):
            with self.subTest(mutation=index):
                self.setUp();mutation()
                self.assertIsNone(v._preceding_exact_stage(self.current,self.stages))
                with self.assertRaises(v.ContractError):self.validate()

    def test_sound_missing_changed_wrong_scope_and_animation_drift_reject(self):
        mutations=(lambda:self.sounds['cues'].clear(),
            lambda:self.cue.update(soundEvent='Wrong_Shot4'),
            lambda:self.cue.update(startMs=375),
            lambda:self.cue.update(patternId='OTHER'),
            lambda:self.impacts.clear(),
            lambda:self.receipt['expectedSoundStage'].update(stageId='WRONG'),
            lambda:self.receipt['expectedSoundStage']['expectedAnimation'].update(repeatCount=2),
            lambda:self.receipt.update(expectedHitOffsetsMs=[68]),
            lambda:self.sounds['cues'].append(dict(self.cue,stageId='CURRENT',actionId='action.current')))
        for index,mutation in enumerate(mutations):
            with self.subTest(mutation=index):
                self.setUp();mutation()
                with self.assertRaises(v.ContractError):self.validate()


if __name__=='__main__':unittest.main()
