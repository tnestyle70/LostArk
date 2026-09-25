"""Focused contracts for the source slot/mask evaluator, without live assets."""
import unittest
from types import SimpleNamespace

import numpy as np

from class_selection_animation_tree import SourceAnimTree, source_controls, source_strengths, sample_times
from project_guardian_selection import base


def row(index, kind, **properties):
    return dict(index=index, cls=kind, name='fixture.'+str(index), p=properties)


def slot(index, name, source):
    return row(index, 'animnodeslot', nodename=name,
               children=[dict(anim=source), dict(anim=0)])


def track(slot_name, clip=None, weight=1.):
    return row(90, 'interptrackanimcontrol', slotname=slot_name,
        animseqs=[] if clip is None else [dict(animseqname=clip, starttime=0.)],
        floattrack=dict(points=[dict(inval=0., outval=weight)]))


def model_and_clips():
    bones = [base.wm.Bone(index+1, name, parent, np.eye(4).reshape(-1).tolist())
             for index, (name, parent) in enumerate((('root', -1), ('spine', 0), ('head', 1), ('foot', 0)))]
    model = SimpleNamespace(skeleton_bones=bones)
    clips = {}
    for name, amount in (('lower', 2.), ('upper', 9.), ('third', 15.)):
        channels = [base.wm.AnimationChannel(index,
            [(0., amount, 0., 0.), (30., amount, 0., 0.)],
            [(0., 0., 0., 0., 1.), (30., 0., 0., 0., 1.)],
            [(0., 1., 1., 1.), (30., 1., 1., 1.)]) for index in range(len(bones))]
        clips[name] = base.wm.Animation(name, 30., 30., channels)
    return model, clips


class SourceTreeContracts(unittest.TestCase):
    def setUp(self):
        self.model, self.clips = model_and_clips()
        self.rows = {
            1: row(1, 'animtree', children=[dict(anim=2)]),
            2: slot(2, 'a', 3),
            3: slot(3, 'b', 4),
            4: row(4, 'animnodesequence', animseqname='lower'),
        }

    def test_empty_weighted_slot_keeps_its_real_source(self):
        tree = SourceAnimTree(self.model, self.rows, 1, [track('a', 'upper'), track('b')], self.clips)
        self.assertTrue(all(p[0][0] == 9. for p in tree.sample(.5)))
        self.assertNotIn(4, tree.evaluated)

    def test_c_only_loop_does_not_require_a_b_or_default_clip(self):
        self.rows[1]['p']['children'][0]['anim'] = 5
        self.rows[5] = slot(5, 'c', 2)
        self.rows[4]['p']['animseqname'] = 'absent-unobserved-default'
        tree = SourceAnimTree(self.model, self.rows, 1, [track('c', 'third')], self.clips)
        self.assertTrue(all(p[0][0] == 15. for p in tree.sample(0.)))
        self.assertEqual(tree.evaluated, {1, 5})

    def test_fractional_slot_uses_lower_pose(self):
        tree = SourceAnimTree(self.model, self.rows, 1,
            [track('a', 'upper', .25), track('b', 'lower')], self.clips)
        self.assertTrue(all(p[0][0] == 3.75 for p in tree.sample(.5)))

    def test_mask_preserves_root_and_foot_exactly(self):
        self.rows[1]['p']['children'][0]['anim'] = 6
        self.rows[5] = slot(5, 'hair', 2)
        self.rows[6] = row(6, 'animnode_multiblendperbone',
            children=[dict(anim=2), dict(anim=5)],
            masklist=[dict(desiredweight=1., branchlist=[dict(bonename='spine', perboneweightincrease=1.)])])
        tree = SourceAnimTree(self.model, self.rows, 1,
            [track('a', 'lower'), track('hair', 'upper')], self.clips)
        self.assertEqual([p[0][0] for p in tree.sample(.5)], [2., 9., 9., 2.])

    def test_observable_missing_default_and_random_are_rejected(self):
        self.rows[4]['p']['animseqname'] = 'missing'
        tree = SourceAnimTree(self.model, self.rows, 1, [], self.clips)
        with self.assertRaisesRegex(AssertionError, 'Missing observable'):
            tree.sample(0.)
        self.rows[4] = row(4, 'animnoderandom', children=[dict(anim=7)], randominfo=[dict(chance=1.)])
        self.rows[7] = row(7, 'animnodesequence', animseqname='lower')
        tree = SourceAnimTree(self.model, self.rows, 1, [], self.clips)
        with self.assertRaisesRegex(AssertionError, 'Observable random'):
            tree.sample(0.)

    def test_cycle_and_unknown_slot_rejected_before_sampling(self):
        with self.assertRaisesRegex(AssertionError, 'slot not present'):
            SourceAnimTree(self.model, self.rows, 1, [track('wrong', 'lower')], self.clips)
        self.rows[3]['p']['children'][0]['anim'] = 2
        with self.assertRaisesRegex(AssertionError, 'Cyclic AnimTree'):
            SourceAnimTree(self.model, self.rows, 1, [], self.clips)

    def test_native_vector_and_multi_control_channels(self):
        tracks = [row(10, 'efinterptrackskelcontrolvector',
            skelcontrolnamelist_positivex=['right'], skelcontrolnamelist_negativex=['left'],
            vectortrack=dict(points=[dict(inval=0., outval=dict(x=-.3, y=0., z=0.))])),
            row(11, 'efinterptrackskelcontrolmulti', skelcontrolnamelist=['eye1', 'eye2'],
                floattrack=dict(points=[dict(inval=0., outval=.6)]))]
        self.assertEqual(source_strengths(tracks, 0.), dict(right=0., left=.3, eye1=.6, eye2=.6))

    def test_absent_undriven_trail_is_recorded_but_present_one_rejected(self):
        self.rows[1]['p']['skelcontrollists'] = [dict(bonename='source-cloth-not-on-this-mesh', controlhead=8)]
        self.rows[8] = row(8, 'skelcontroltrail', controlname='trail')
        controls, absent = source_controls(self.model, self.rows, 1, [])
        self.assertEqual(controls, [])
        self.assertEqual(len(absent), 1)
        self.rows[1]['p']['skelcontrollists'][0]['bonename'] = 'head'
        with self.assertRaisesRegex(AssertionError, 'Observable undriven'):
            source_controls(self.model, self.rows, 1, [])

    def test_shared_matinee_control_missing_from_actual_tree_is_recorded_noop(self):
        tracks = [row(10, 'interptrackskelcontrolstrength', skelcontrolname='other-actor-face',
                      floattrack=dict(points=[dict(inval=0., outval=1.)]))]
        controls, absent = source_controls(self.model, self.rows, 1, tracks)
        self.assertEqual(controls, [])
        self.assertEqual(absent, [dict(control='other-actor-face',
            reason='Named control absent from exact source AnimTree')])

    def test_source_key_boundary_and_endpoint_are_not_rounded_away(self):
        tracks = [row(8, 'interptrackanimcontrol',
                      animseqs=[dict(starttime=.023456, animseqname='lower')])]
        times = sample_times(tracks, 101)
        self.assertEqual(times[0], 0.)
        self.assertEqual(times[-1], .101)
        self.assertIn(.023456, times)
        self.assertLess(min(abs(t-.023356) for t in times), 1e-12)


if __name__ == '__main__':
    unittest.main()
