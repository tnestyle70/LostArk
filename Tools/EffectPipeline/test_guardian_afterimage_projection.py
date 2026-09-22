from pathlib import Path
import base64
import copy
import hashlib
import json
import struct
import unittest

from Tools.EffectPipeline.guardian_afterimage_projection import decode_trail_ghost, project_trail_ghost_rows


class TrailGhostSourceProjectionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.cases = json.loads((Path(__file__).parent / 'Fixtures/trailghost_source_notifies.json').read_text(encoding='utf8'))['cases']

    def mutate(self, row, offset, fmt, value):
        changed = copy.deepcopy(row)
        raw = bytearray(base64.b64decode(changed['serializedPayload']['data']))
        struct.pack_into(fmt, raw, offset, value)
        changed['serializedPayload'].update(data=base64.b64encode(raw).decode(), sha256=hashlib.sha256(raw).hexdigest())
        return changed

    def test_original_part_local_and_short_emission(self):
        self.assertEqual(len(self.cases), 3)
        for case in self.cases:
            row = case['notify']; decoded = decode_trail_ghost(row)
            self.assertEqual(decoded['sourcePartType'], case['expectedPart'])
            self.assertEqual(decoded['onlyLocalPlayer'], case['expectedOnlyLocal'])
            self.assertAlmostEqual(decoded['initialAlpha'], case['expectedInitialAlpha'], places=6)
            projected, receipt = project_trail_ghost_rows([row], {row['clipName']: 10})
            cue = next(iter(projected.values()))[0]
            self.assertEqual(cue['alphaMode'], 'TRANSLUCENT')
            self.assertTrue(cue['afterimage']['captureInitialPose'])
            self.assertTrue(cue['afterimage']['liveOwnerPose'])
            self.assertEqual(cue['afterimage']['appearanceBasis'], 'PROJECT_AUTHORED')
            self.assertEqual(cue['afterimage']['sourceColorIntensity'], decoded['sourceColorIntensity'])
            self.assertLessEqual(cue['afterimage']['maxSamples'], 64)

    def test_corrupt_receipt_is_rejected(self):
        row = copy.deepcopy(self.cases[0]['notify']); row['serializedPayload']['sha256'] = '0'*64
        with self.assertRaises(ValueError): decode_trail_ghost(row)

    def test_invalid_source_enum_nonfinite_and_flags_are_rejected(self):
        row = self.cases[0]['notify']; start = decode_trail_ghost(row)['fieldByteOffset']
        for offset,fmt,value in [(0,'<I',2),(28,'<I',3),(16,'<f',float('nan'))]:
            with self.assertRaises(ValueError): decode_trail_ghost(self.mutate(row,start+offset,fmt,value))

    def test_nonidentity_unrecovered_scale_is_not_silently_projected(self):
        row = self.cases[0]['notify']; start = decode_trail_ghost(row)['fieldByteOffset']
        row = self.mutate(row,start+27*4,'<f',2.)
        with self.assertRaises(ValueError): project_trail_ghost_rows([row],{row['clipName']:10})

    def test_notify_identity_and_empty_stopped_window_are_rejected(self):
        row = copy.deepcopy(self.cases[0]['notify']); row['skillId']+=1
        with self.assertRaises(ValueError): project_trail_ghost_rows([row],{row['clipName']:10})
        row=copy.deepcopy(next(c['notify'] for c in self.cases if decode_trail_ghost(c['notify'])['stopWhenNotifyEnd'])); row['localTimeSeconds']=10; row['sourceEndSeconds']=0
        self.assertTrue(decode_trail_ghost(row)['stopWhenNotifyEnd'])
        with self.assertRaises(ValueError): project_trail_ghost_rows([row],{row['clipName']:1})


if __name__ == '__main__': unittest.main()
