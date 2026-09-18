"""Numeric regression for the saved Showtime camera, not a visual acceptance test."""
import json
import math
from pathlib import Path
import unittest

import retarget_showtime_camera_scale as fix

ROOT = Path(__file__).resolve().parents[2]
FLOOR_Y = 1.317626  # Installed SL05:export:342, floor08a upper triangles.
OFFSET = (-.1146, .0141, .4151)


def normalized(v):
    size = math.sqrt(sum(x*x for x in v))
    return [x / size for x in v]


def cross(a, b):
    return [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]]


class CameraClearance(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        fixture = json.loads((Path(__file__).parent/'fixtures/showtime_camera_before_scale.json').read_text(encoding='utf8'))
        cls.source = json.dumps({'revision': fixture['revision'], 'shots': fixture['shots']}, indent=2)
        ratio = .017/(.01*fix.SOURCE_DRAW_SCALE)
        cls.old = json.loads(fix.rewrite(cls.source, ratio)[0])
        cls.result, _ = fix.rewrite(cls.source, ratio, floor_clearance=True)
        cls.new = json.loads(cls.result)

    def test_only_second_shot_height_and_revision_change(self):
        old = {s['shotId']: s for s in self.old['shots']}
        for shot in self.new['shots']:
            before = old[shot['shotId']]
            if shot['shotId'] != fix.SHOTS[1]:
                self.assertEqual(shot, before)
                continue
            pairs = [(shot['eye'], before['eye']), (shot['lookAt'], before['lookAt']),
                     (shot['box']['center'], before['box']['center'])]
            pairs += [(k[f], p[f]) for k,p in zip(shot['cameraTrack']['keyframes'],before['cameraTrack']['keyframes']) for f in ('eye','lookAt')]
            for actual, previous in pairs:
                self.assertEqual(actual[0], previous[0])
                self.assertEqual(actual[2], previous[2])
                self.assertAlmostEqual(actual[1]-previous[1], .4)
            for k,p in zip(shot['cameraTrack']['keyframes'],before['cameraTrack']['keyframes']):
                self.assertEqual({x:v for x,v in k.items() if x not in ('eye','lookAt')},
                                 {x:v for x,v in p.items() if x not in ('eye','lookAt')})
        self.assertEqual(self.new['revision'], self.old['revision']+1)

    def test_eye_and_near_plane_clear_floor_every_millisecond(self):
        minimum = float('inf')
        for shot in self.new['shots']:
            if shot['shotId'] not in fix.SHOTS: continue
            keys=shot['cameraTrack']['keyframes']
            for a,b in zip(keys,keys[1:]):
                for ms in range(a['timeMs'], b['timeMs']+1):
                    t=(ms-a['timeMs'])/(b['timeMs']-a['timeMs'])
                    eye=[x+(y-x)*t for x,y in zip(a['eye'],b['eye'])]
                    look=[x+(y-x)*t for x,y in zip(a['lookAt'],b['lookAt'])]
                    up=[x+(y-x)*t for x,y in zip(a['up'],b['up'])]
                    forward=normalized([y-x for x,y in zip(eye,look)])
                    right=normalized(cross(up,forward)); up=cross(forward,right)
                    fov=a['fovYDegrees']+(b['fovYDegrees']-a['fovYDegrees'])*t
                    half=.1*math.tan(math.radians(fov)/2)
                    for aspect in (4/3,16/9,21/9):
                        bottom=eye[1]+OFFSET[1]+.1*forward[1]-half*(abs(up[1])+aspect*abs(right[1]))
                        minimum=min(minimum,bottom-FLOOR_Y)
                        self.assertGreater(bottom-FLOOR_Y, .1, (shot['shotId'],ms,aspect))
        print('Minimum near-plane floor clearance (metres):',minimum)

    def test_installed_authoring_is_expected_correction(self):
        installed = json.loads((ROOT/fix.REL).read_text(encoding='utf8'))
        actual = {s['shotId']: s for s in installed['shots'] if s['shotId'] in fix.SHOTS}
        expected = {s['shotId']: s for s in self.new['shots']}
        self.assertEqual(actual, expected)


if __name__ == '__main__':
    unittest.main()
