"""Source and candidate boundaries used by the Encore glass handoff."""
import unittest

from Tools.KoukuSaydonPipeline import prepare_encore_glass_effects as bake


class EncoreGlassContractTests(unittest.TestCase):
    def test_trigger_does_not_invent_matinee_end(self):
        self.assertEqual(bake.trigger_intervals([dict(time=15.433334, toggleaction='etta_trigger')]),
                         [(15.433334, None)])

    def test_three_trigger_off_windows(self):
        keys = [dict(time=t, toggleaction=a) for t, a in [
            (12.333333, 'etta_off'), (12.5, 'etta_trigger'), (12.833333, 'etta_off'),
            (13.533334, 'etta_trigger'), (13.866668, 'etta_off'),
            (15.433334, 'etta_trigger'), (16, 'etta_off')]]
        self.assertEqual(bake.trigger_intervals(keys),
                         [(12.5, 12.833333), (13.533334, 13.866668), (15.433334, 16)])

    def test_non_trigger_source_requires_separate_implementation(self):
        with self.assertRaises(AssertionError):
            bake.trigger_intervals([dict(time=0, toggleaction='etta_on')])

    def test_moving_local_key_is_not_frozen(self):
        with self.assertRaises(ValueError):
            bake.groups.constant_curve(dict(points=[
                dict(inval=0, outval=dict(x=0, y=0, z=0)),
                dict(inval=1, outval=dict(x=1, y=0, z=0))]))

    def test_no_candidate_output_into_live_data(self):
        with self.assertRaisesRegex(AssertionError, 'repository out'):
            bake.prepare(bake.ROOT / 'Data')


if __name__ == '__main__':
    unittest.main()
