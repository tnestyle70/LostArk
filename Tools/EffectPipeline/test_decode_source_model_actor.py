import base64
import json
from pathlib import Path
import unittest

from decode_source_model_actor import source_actor, material_tracks

CASES = json.loads((Path(__file__).parent / 'Fixtures/guardian_skeletal_actor_source.json').read_bytes())

class SourceModelActorTests(unittest.TestCase):
    def test_nested_particle_does_not_supply_the_dragon_transform(self):
        result = source_actor(CASES[2]['notifies'], 0)
        self.assertEqual(result['sourcePositionUeCm'], [-1100., -40., -200.])
        self.assertEqual(result['sourceRotator'], [-2366, -1820, 364])
        self.assertEqual(result['sourceScale'], [1.5, 1.5, 1.5])
        self.assertAlmostEqual(result['playRate'], .83, places=6)

    def test_enclosing_tail_survives_multiple_nested_material_events(self):
        for case in CASES[:2]:
            actor = source_actor(case['notifies'], 0)
            self.assertEqual(actor['sourcePositionUeCm'], [0., 0., 0.])
            self.assertEqual(actor['sourceScale'], [1., 1., 1.])
            tracks = material_tracks(case['notifies'], 0)
            self.assertEqual({r['name'] for r in tracks}, {'dead', 'emissive_intensity', 'transcolor'})
            self.assertEqual(next(r for r in tracks if r['name'] == 'transcolor')['kind'], 'COLOR')

    def test_dragon_resonance_first_only_flag_keeps_original_actor_transform(self):
        actor=source_actor(CASES[3]['notifies'],0)
        self.assertEqual(actor['sourcePositionUeCm'],[0.,0.,40.])
        self.assertEqual(actor['sourceRotator'],[0,0,0])
        for value in actor['sourceScale']:self.assertAlmostEqual(value,2.3,places=6)
        self.assertEqual(actor['localTransform']['position'],[0.,.4,0.])
        self.assertEqual(actor['clip'],'sk_dragonicresonance_02')

    def test_truncated_child_tail_cannot_become_identity_transform(self):
        case = json.loads(json.dumps(CASES[2]))
        payload = case['notifies'][-1]['serializedPayload']
        payload['data'] = base64.b64encode(base64.b64decode(payload['data'])[:95]).decode()
        with self.assertRaises((AssertionError, ValueError, __import__('struct').error)):
            source_actor(case['notifies'], 0)

if __name__ == '__main__':
    unittest.main()
