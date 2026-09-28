from __future__ import annotations

import copy
import unittest
from pathlib import Path

import valtan_tuning_pipeline as pipeline


ROOT = Path(__file__).resolve().parents[2]


class HealthRotationContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.docs = pipeline.load_pipeline_documents(ROOT)
        cls.master = pipeline.join_v2_authoring(
            cls.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            cls.docs[pipeline.PRESENTATION_AUTHORING_REL],
            cls.docs[pipeline.WORLD_SET_REL],
            cls.docs[pipeline.COMBAT_AUTHORING_REL],
        )

    def validate(self, master):
        pipeline.validate_v2_master(master, self.docs[pipeline.WORLD_SET_REL],
                                   self.docs[pipeline.COMBAT_AUTHORING_REL])
        pipeline.validate_decision_model_against_boss_profiles(
            master, self.docs[pipeline.BOSS_PROFILES_REL])

    def test_requested_health_gates_and_repeat_order(self):
        decision = self.master['decisionModel']
        self.assertEqual('HEALTH_BAR_ROTATIONS', decision['scriptedSequence']['mode'])
        self.assertEqual([130, 115, 105, 80, 65, 30, 15],
                         [m['trigger']['healthBar'] for m in decision['mechanics']])
        rotations = pipeline._compile_rotations(self.master, self.docs[pipeline.LEGACY_REL])
        self.assertEqual([(1,160,130), (1,130,115), (2,115,105), (2,105,80),
                          (2,80,65), (2,65,30), (2,30,15), (3,40,0)],
                         [(r['gameplayPhase'],r['fromHealthBar'],r['toHealthBar']) for r in rotations])
        self.assertEqual(['VALTAN_WHIRLWIND','VALTAN_DASH_CHARGE','VALTAN_HIGH_JUMP',
                          'VALTAN_FOUR_SLASH','VALTAN_CROSS','VALTAN_DASH_CHARGE'], rotations[0]['patternIds'])
        self.assertEqual(rotations[0]['patternIds'], rotations[1]['patternIds'])
        self.assertEqual(['VALTAN_FOUR_SLASH','VALTAN_CATCH_BREATH','VALTAN_WHIRLWIND',
                          'VALTAN_CROSS','VALTAN_SEQUENCE_FOUR'], rotations[-1]['patternIds'])
        self.validate(self.master)

    def test_ordered_duplicate_is_preserved(self):
        master = copy.deepcopy(self.master)
        steps = master['decisionModel']['selectionSets'][0]['patternIds']
        steps.append(steps[0])
        self.validate(master)
        projected = pipeline._compile_rotations(master, self.docs[pipeline.LEGACY_REL])
        self.assertEqual(steps, projected[0]['patternIds'])

    def test_ordered_and_weighted_payload_cannot_mix(self):
        master = copy.deepcopy(self.master)
        master['decisionModel']['selectionSets'][0]['candidates'] = []
        with self.assertRaises(pipeline.PipelineError):
            self.validate(master)

    def test_missing_ordered_step_is_rejected(self):
        master = copy.deepcopy(self.master)
        master['decisionModel']['selectionSets'][0]['patternIds'][0] = 'VALTAN_UNKNOWN'
        with self.assertRaises(pipeline.PipelineError):
            self.validate(master)

    def test_ghost_window_uses_ghost_profile(self):
        master = copy.deepcopy(self.master)
        master['decisionModel']['selectionWindows'][-1]['maximumHealthBarInclusive'] = 160
        with self.assertRaises(pipeline.PipelineError):
            self.validate(master)

    def test_phase_transition_cannot_drift_independently(self):
        master = copy.deepcopy(self.master)
        master['decisionModel']['mechanics'][1]['trigger']['healthBar'] = 114
        with self.assertRaises(pipeline.PipelineError):
            self.validate(master)

    def test_promoted_automatic_patterns_keep_original_animation_lineage(self):
        debug = pipeline.read_json(ROOT / pipeline.DEBUG_PRESENTATION_REL)
        manifest = pipeline.read_json(ROOT / pipeline.ANIMATION_PROMOTION_MANIFEST_REL)
        pipeline.validate_manual_audition_animation_lineage(self.master, debug, manifest, repository_root=ROOT)
        next(r for r in manifest['patterns'] if r['patternId'] == 'VALTAN_SIX_PIZZA_106')['sourceChainId'] = 'unknown.chain'
        with self.assertRaises(pipeline.PipelineError):
            pipeline.validate_manual_audition_animation_lineage(self.master, debug, manifest, repository_root=ROOT)

    def test_reference_sequence_accepts_legacy_mode_and_rejects_unknown(self):
        decision = copy.deepcopy(self.master['decisionModel'])
        patterns = {p['patternId']: p for p in self.master['patterns']}
        decision['scriptedSequence']['mode'] = 'ORDERED_ONCE_THEN_IDLE'
        self.assertTrue(pipeline._validate_scripted_sequence(decision, patterns))
        decision['scriptedSequence']['mode'] = 'LOOP_UNKNOWN'
        with self.assertRaises(pipeline.PipelineError):
            pipeline._validate_scripted_sequence(decision, patterns)


if __name__ == '__main__':
    unittest.main()
