"""Source identity and runtime-delta regression; no Client/UI execution."""
import copy
import json
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import numpy as np
from scipy.spatial.transform import Rotation
import build_maharaka_waterpang_sequences as build


class ImportContracts(unittest.TestCase):
    def test_merge_preserves_and_rejects_modified_or_duplicate_identity(self):
        doc = {'rows': [{'id': 'other', 'value': 7}]}
        row = {'id': 'owned', 'value': 9}
        build.append_owned(doc, 'rows', 'id', [row, row])
        self.assertEqual(doc['rows'], [{'id': 'other', 'value': 7}, row])
        with self.assertRaises(ValueError):
            build.append_owned(doc, 'rows', 'id', [{'id': 'owned', 'value': 10}])
        with self.assertRaises(ValueError):
            build.append_owned({'rows': [row, row]}, 'rows', 'id', [])

    def test_initial_key_limit(self):
        rows = {1: {'p': {'interptracks': [2]}}, 2: {'cls': 'interptrackmove', 'p': {}}}
        with self.assertRaisesRegex(ValueError, 'Initial tile curve'):
            build.sampled_keys(rows, 1, 3, 4, 5, 65536, {})

    def test_freshness_rejects_before_any_write(self):
        with tempfile.TemporaryDirectory(prefix='waterpang-contract-') as folder:
            a, b = Path(folder)/'a', Path(folder)/'b'
            a.write_bytes(b'newer-user-edit'); b.write_bytes(b'old')
            with self.assertRaises(ValueError):
                build.commit_staged_files({a: (b'old', b'replace'), b: (b'old', b'replace')})
            self.assertEqual(a.read_bytes(), b'newer-user-edit')
            self.assertEqual(b.read_bytes(), b'old')

    def test_second_promote_failure_rolls_back(self):
        import source_character_registration as transaction
        with tempfile.TemporaryDirectory(prefix='waterpang-contract-') as folder:
            a, b = Path(folder)/'a', Path(folder)/'b'
            a.write_bytes(b'old-a'); b.write_bytes(b'old-b')
            real = transaction.os.replace
            count = 0
            def fail_second(src, dst):
                nonlocal count
                count += 1
                if count == 2:
                    raise OSError('injected promote failure')
                return real(src, dst)
            with patch.object(transaction.os, 'replace', side_effect=fail_second):
                with self.assertRaises(OSError):
                    build.commit_staged_files({a: (b'old-a', b'new-a'), b: (b'old-b', b'new-b')})
            self.assertEqual(a.read_bytes(), b'old-a')
            self.assertEqual(b.read_bytes(), b'old-b')


class OriginalCurveContracts(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        audit = build.load_json(build.AUDIT)
        cls.rows, _ = build.source.extract_scene(Path(audit['source']['physicalPackage']))
        cls.live = build.placements((build.AUTHORING/(build.AREA+'.mapplacements')).read_bytes())
        root = build.OUT/'candidate/Data/Maps/Authoring'/build.AREA
        cls.world = build.load_json(root/(build.AREA+'.worldsequences.json'))
        cls.camera = build.load_json(root/(build.AREA+'.camerashots.json'))

    def test_every_tile_binding_and_midpoint_world_pose(self):
        for matinee, data, tag, _ in build.MOTIONS:
            identity = build.PREFIX+'.'+tag
            template = next(t for t in self.world['templates'] if t['sequenceId'] == 'sequence.'+identity)
            instance = next(i for i in self.world['instances'] if i['templateId'] == template['sequenceId'])
            self.assertEqual(len(instance['bindings']), 18)
            self.assertEqual(len({b['targetId'] for b in instance['bindings']}), 18)
            groups = {self.rows[g]['p'].get('groupname'): g for g in build.groups_for(self.rows, matinee, data)}
            for track in template['tracks']:
                group = groups[track['slotId']]
                actor, = build.source.group_actor(self.rows, group, matinee)
                component = self.rows[actor]['p']['staticmeshcomponent']
                baseline = self.live[f'{build.SCENE}:export:{component-1}']
                binding = next(b for b in instance['bindings'] if b['slotId'] == track['slotId'])
                self.assertEqual(binding['targetId'], baseline['id'])
                keys = track['keys']
                self.assertLessEqual(len(keys), 4096)
                self.assertEqual(keys[0]['timeMs'], 0)
                self.assertEqual(keys[-1]['timeMs'], template['durationMs'])
                for left, right in zip(keys, keys[1:]):
                    self.assertGreater(right['timeMs'], left['timeMs'])
                    self.assertEqual(left['scaleMultiplier'], [1., 1., 1.])
                    self.assertAlmostEqual(np.linalg.norm(left['rotationQuaternion']), 1., places=7)
                    ms = round((left['timeMs']+right['timeMs'])/2)
                    t = (ms-left['timeMs'])/(right['timeMs']-left['timeMs'])
                    offset = np.array(left['positionOffset'])*(1-t)+np.array(right['positionOffset'])*t
                    q = build.source.slerp(left['rotationQuaternion'], right['rotationQuaternion'], t)
                    p, r = build.source.world_pose(self.rows, group, actor, ms/1000., matinee, data)
                    self.assertLessEqual(np.linalg.norm(baseline['p']+baseline['r']@offset-p), .001001)
                    expected = Rotation.from_matrix(baseline['r'].T@r).as_quat()
                    self.assertLessEqual(build.angle(q, expected), .050001)

    def test_camera_and_independent_motion_cutscenes(self):
        self.assertEqual(len(self.camera['cutscenes']), 6)
        shots = {s['shotId']: s for s in self.camera['shots']}
        instances = {i['instanceId'] for i in self.world['instances']}
        for cutscene in self.camera['cutscenes']:
            self.assertLessEqual(len(cutscene['worldInstanceIds']), 1)
            self.assertTrue(set(cutscene['worldInstanceIds']) <= instances)
            previous_end = 0
            for cut in cutscene['cameraCuts']:
                shot = shots[cut['shotId']]
                self.assertEqual(shot['activation'], 'PATTERN_ONLY')
                self.assertGreaterEqual(cut['startMs'], previous_end)
                previous_end = cut['startMs']+shot['cameraTrack']['durationMs']
                self.assertLessEqual(previous_end, cutscene['durationMs'])
                self.assertLessEqual(len(shot['cameraTrack']['keyframes']), 128)

    def test_bad_component_identity_fails_instead_of_nearest_neighbor(self):
        live = copy.deepcopy(self.live)
        live.pop(next(iter(live)))
        with self.assertRaisesRegex(ValueError, 'Missing exact source component'):
            build.build_motion(self.rows, live, *build.MOTIONS[0])


class SourceFoleyContracts(unittest.TestCase):
    def test_direct_media_source_clock_and_stage_ownership(self):
        import install_maharaka_waterpang_foley as foley
        staged, media = foley.prepare()
        world_path = build.AUTHORING/(build.AREA+'.worldsequences.json')
        world = json.loads(staged[world_path][1])
        collapse = next(t for t in world['templates'] if t['sequenceId'] == 'sequence.'+build.PREFIX+'.collapse')
        self.assertEqual(collapse['durationMs'], 5000)
        self.assertEqual(build.interp_defaults()['objectPath'], 'Default__InterpData')
        timings = {}
        for template in world['templates']:
            for sound in template.get('soundTracks', []):
                timings[sound['soundTrackId']] = sound['startMs']
                self.assertTrue(sound['assetId'].startswith('Sound/Maharaka/WaterpangSource/'))
                self.assertGreater(sound['durationMs'], 0)
                self.assertEqual(len(template['tracks']), 18)
        self.assertEqual(timings, {build.PREFIX+'.collapse.foley': 0,
                                  build.PREFIX+'.intro15.foley': 172,
                                  build.PREFIX+'.intro20.foley': 403})
        self.assertEqual(media['scene_maharakap_waterpangstart']['mediaId'], 714113137)
        self.assertEqual(media['scene_maharakap_fallout_foley']['mediaId'], 427337176)


if __name__ == '__main__':
    unittest.main()
