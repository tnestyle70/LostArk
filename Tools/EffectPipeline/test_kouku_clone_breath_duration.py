import copy
import json
import unittest

import build_kouku_clone_breath_group as breath


class AuthoredBreathDurationTests(unittest.TestCase):
    def setUp(self):
        self.document = json.loads(breath.OUTPUT.read_bytes())

    def test_tail_inclusive_duration_and_single_bursts(self):
        candidate, rows = breath.extend_authored_duration(self.document, 5000)
        self.assertEqual(sum(r['continuous'] for r in rows), 13)
        self.assertAlmostEqual(max(r['afterEndSeconds'] for r in rows), 5)
        for before, after, row in zip(self.document['elements'], candidate['elements'], rows):
            self.assertEqual(before['id'], after['id'])
            self.assertEqual(before['sourceRecipe']['bursts'], after['sourceRecipe']['bursts'])
            self.assertEqual(after['sourceRecipe']['emitterLoopCount'], 1)
            if not row['continuous']:
                self.assertEqual(before['sourceRecipe'], after['sourceRecipe'])

    def test_every_unrelated_authored_field_is_preserved(self):
        candidate, _ = breath.extend_authored_duration(self.document, 5000)
        restored = copy.deepcopy(candidate)
        restored['sourceModelPreview']['animations'][0]['playMs'] = self.document['sourceModelPreview']['animations'][0]['playMs']
        for before, after in zip(self.document['elements'], restored['elements']):
            after['detail']['timing']['lifeTimeSeconds'] = before['detail']['timing']['lifeTimeSeconds']
            after['sourceRecipe']['emitterDurationSeconds'] = before['sourceRecipe']['emitterDurationSeconds']
            for original_module, module in zip(before['sourceRecipe']['modules'], after['sourceRecipe']['modules']):
                if module['className'] == 'particlemodulerequired':
                    for original, value in zip(original_module['literals'], module['literals']):
                        if value['propertyPath'] == 'emitterduration':
                            value['value'] = original['value']
        self.assertEqual(restored, self.document)

    def test_idempotent_reapply(self):
        first, _ = breath.extend_authored_duration(self.document, 5000)
        second, _ = breath.extend_authored_duration(first, 5000)
        self.assertEqual(first, second)

    def test_refuses_shortening_or_unreviewed_spawn_curve(self):
        with self.assertRaises(AssertionError):
            breath.extend_authored_duration(self.document, 100)
        changed = copy.deepcopy(self.document)
        spawn = next(m for m in changed['elements'][0]['sourceRecipe']['modules']
                     if m['className'] == 'particlemodulespawn')
        rate = next(d for d in spawn['distributions'] if d['propertyPath'] == 'rate')
        rate['lookupTable'][-1] += 1
        with self.assertRaises(AssertionError):
            breath.extend_authored_duration(changed, 5000)


if __name__ == '__main__':
    unittest.main()
