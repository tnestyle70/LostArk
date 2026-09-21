from pathlib import Path
import base64,copy,hashlib,json,struct,unittest
from guardian_owner_control_projection import decode_control,project_visibility_controls
from guardian_camera_projection import project_camera,native_pose

FIXTURE=Path(__file__).with_name('Fixtures')/'guardian_owner_controls.json'

class GuardianOwnerControlsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):cls.fixture=json.loads(FIXTURE.read_bytes())
    def mutate(self,notify,offset,value):
        notify=copy.deepcopy(notify);p=notify['serializedPayload'];raw=bytearray(base64.b64decode(p['data']))
        struct.pack_into('<I',raw,offset,value);p['data']=base64.b64encode(raw).decode();p['sha256']=hashlib.sha256(raw).hexdigest();return notify
    def test_actual_fourteen_source_controls(self):
        values=[decode_control(r['notify']) for r in self.fixture['rows']]
        self.assertEqual(len(values),14)
        self.assertEqual(sum(v['kind']=='IdentityParts' for v in values),9)
        self.assertEqual([v['controlType'] for v in values if v['kind']=='UltimateSkillCameraControl'],[1,2])
        self.assertTrue(all(v['enabled'] for v in values))
    def test_original_visibility_targets_and_end_windows(self):
        groups,receipt=project_visibility_controls(self.fixture['rows'])
        values=[v for group in groups.values() for v in group]
        self.assertEqual(len(values),12)
        self.assertEqual(sorted(v['sourceTargetType'] for v in values),[0,0,9]+[12]*9)
        self.assertTrue(all(v['keys'][0]['value']==[0,0,0,0] and v['keys'][-1]['seconds']>0 for v in values))
        self.assertEqual(sum('No independently' in r['statusFxBoundary'] for r in receipt),2)
    def test_sha_corruption_rejected(self):
        row=copy.deepcopy(self.fixture['rows'][0]['notify']);row['serializedPayload']['sha256']='0'*64
        with self.assertRaises(ValueError):decode_control(row)
    def test_unknown_common_bool_rejected(self):
        n=self.fixture['rows'][0]['notify'];offset=len(('CEFActionNotify_'+n['sourceType']+'\0').encode())+12
        with self.assertRaises(ValueError):decode_control(self.mutate(n,offset,2))
    def test_disabled_common_header_excluded(self):
        rows=copy.deepcopy(self.fixture['rows']);n=rows[0]['notify'];offset=len(('CEFActionNotify_'+n['sourceType']+'\0').encode())+12
        rows[0]['notify']=self.mutate(n,offset,0)
        self.assertEqual(sum(map(len,project_visibility_controls(rows)[0].values())),11)
    def test_unknown_part_policy_rejected(self):
        row=next(copy.deepcopy(r) for r in self.fixture['rows'] if r['notify']['sourceType']=='HidePawn' and decode_control(r['notify'])['parts'])
        value=decode_control(row['notify']);row['notify']=self.mutate(row['notify'],value['fieldByteOffset']+36,123)
        with self.assertRaises(ValueError):project_visibility_controls([row])
    def test_native_camera_graph_and_basis(self):
        graph=self.fixture['camera'];doc=project_camera(graph,'effect.guardianknight.skill.49420.clip.0.full.restore')
        self.assertEqual(len(doc['cameras'][0]['keys']),458)
        pose=native_pose(graph,0)
        self.assertAlmostEqual(pose['eye'][0],1.8426320514,places=8)
        self.assertEqual(pose['fovDegrees'],30)
        altered=copy.deepcopy(graph);altered['rows']['99']['p']['bdisabletrack']=False
        with self.assertRaises(ValueError):project_camera(altered,'effect.guardianknight.skill.49420.clip.0.full.restore')

if __name__=='__main__':unittest.main()
