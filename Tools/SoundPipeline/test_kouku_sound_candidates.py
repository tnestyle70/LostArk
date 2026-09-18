import copy
from fractions import Fraction
import unittest

import build_kouku_sound_candidates as subject


class KoukuSoundSourceTests(unittest.TestCase):
    def setUp(self):
        self.media = {n: dict(durationSeconds=float(n)) for n in range(1, 5)}

    def test_layer_keeps_both_sources_and_random_chooses_one(self):
        layout = subject.txpt_layout("?1.wem #i\n?2.wem #i\ngroup = -L2 #@layer-v", self.media)
        self.assertEqual((2., [(1, 0., 1.), (2, 0., 1.)]), layout)
        layout = subject.txpt_layout("?1.wem #i\n?2.wem #i\ngroup = -R2>2", self.media)
        self.assertEqual((2., [(2, 0., 1.)]), layout)

    def test_delay_gain_and_sequence_remain_on_source_clock(self):
        layout = subject.txpt_layout("?1.wem #i #p 0.5 #v -6.020599913279624dB\n?2.wem #i\ngroup = -S2", self.media)
        self.assertAlmostEqual(layout[1][0][2], .5)
        self.assertEqual(3.5, layout[0])
        self.assertEqual(1.5, layout[1][1][1])

    def test_nested_random_does_not_drop_smaller_inner_group(self):
        text = "?1.wem #i\n?2.wem #i\n?3.wem #i\n?4.wem #i\ngroup = -R2>3\ngroup = -R3>3"
        variants = subject.random_layouts(text, self.media)
        self.assertEqual({1, 2, 3, 4}, {x[0][1][0][0] for x in variants})

    def test_unknown_or_unbounded_tree_is_never_guessed(self):
        for text in ("?1.wem #i\ngroup = -S1 #@loop", "?1.wem #i #m0^1~0", "?9.wem #i"):
            with self.subTest(text=text), self.assertRaises(subject.Holdout):
                subject.txpt_layout(text, self.media)

    def test_trim_and_loop_do_not_replay_trimmed_prefix(self):
        animation = dict(profileId="p", sourceActionId=1, sourceStageId="s", sourceSlotId="a", occurrenceId="P.animation.1",
                         runtimeClip="clip", startOffsetMs=10, playRate=2, playMs=1300, sourceStartMs=100, endPolicy="LOOP_TO_WINDOW")
        document = dict(patterns=[dict(patternId="P", stages=[dict(durationMs=2000, animationOccurrences=[animation])])])
        record = dict(key=("p", 1, "s", "a"), rows=[("before", "bank.before", 50), ("inside", "bank.inside", 300)], nativeMs=1100)
        cues = subject.animation_cues(document, {record["key"]: record}, {}, [])
        self.assertEqual([110, 610, 1110], [c["startMs"] for c in cues])
        self.assertTrue(all(c["event"] == "bank.inside" for c in cues))

    def test_merge_preserves_unrelated_boxes_and_reallocates_ordinals(self):
        pattern = dict(patternId="P", stages=[], presentationOccurrences=[], nextPresentationOccurrenceOrdinal=1)
        current = dict(revision=3, presentationResources=[], patterns=[copy.deepcopy(pattern)])
        resource = dict(resourceId="sound.r")
        candidate = copy.deepcopy(pattern)
        candidate["presentationOccurrences"] = [dict(occurrenceId="P.presentation.1", resourceId="sound.r", startMs=100, durationMs=200)]
        patch = dict(resources=[resource], patterns=[dict(patternId="P", baseline=pattern, candidate=candidate)])
        unrelated = dict(occurrenceId="P.presentation.1", resourceId="effect.user", startMs=2, durationMs=10, positionOffset=[9, 8, 7])
        current["patterns"][0]["presentationOccurrences"].append(unrelated)
        merged = subject.merge_additions(current, patch)
        self.assertEqual(unrelated, merged["patterns"][0]["presentationOccurrences"][0])
        self.assertEqual("P.presentation.2", merged["patterns"][0]["presentationOccurrences"][1]["occurrenceId"])
        self.assertEqual(merged, subject.merge_additions(merged, patch))
        self.assertEqual(1, len(current["patterns"][0]["presentationOccurrences"]))

    def test_real_source_playlist_weight_is_not_flattened(self):
        index = subject.txtp_index(subject.DEFAULT_SOURCE)
        event = "s_mob_g_bigsatan1.g_bigsatan1_attack01_shotvox1"
        if event not in index: self.skipTest("local extracted source unavailable")
        weights = subject.BankWeights(subject.DEFAULT_SOURCE).resolve(event, index[event][0][1])
        self.assertEqual(Fraction(1, 12), weights[(954515408,)])
        self.assertEqual(Fraction(1, 4), weights[(172268376,)])
        self.assertEqual(Fraction(1), sum(weights.values()))


if __name__ == "__main__":
    unittest.main()
