import sys,json,copy,unittest,hashlib,os,subprocess
from unittest.mock import patch
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from test_valtan_canonical_typed_patch_transaction import ValtanCanonicalTypedPatchTransactionTests, REPOSITORY_ROOT
import promote_valtan_animation_chains as m
class TestSourceSave(unittest.TestCase):
 data_manifest = ValtanCanonicalTypedPatchTransactionTests.data_manifest
 source_manifest = ValtanCanonicalTypedPatchTransactionTests.source_manifest
 run_pipeline = ValtanCanonicalTypedPatchTransactionTests.run_pipeline
 parse_command_result = staticmethod(ValtanCanonicalTypedPatchTransactionTests.parse_command_result)
 def setUp(self):
  ValtanCanonicalTypedPatchTransactionTests.setUp(self)
  self.resource_env=patch.dict(os.environ,{"LOSTARK_RESOURCE_ROOT":str(REPOSITORY_ROOT / "Client/Bin/Resources")});self.resource_env.start()
 def tearDown(self):
  self.resource_env.stop();ValtanCanonicalTypedPatchTransactionTests.tearDown(self)
 def baseline(self):
  path=self.root/'baseline'
  for rel in ('Data/Valtan/Valtan.gameplay.json','Data/Valtan/Valtan.presentation.json','Data/Valtan/Valtan.combatobjects.json','Data/Actors/BossCatalog.json'):
   dst=path/rel;dst.parent.mkdir(parents=True,exist_ok=True);dst.write_bytes((self.root/rel).read_bytes())
  return path
 def test_unresolved_sound_save(self):
  baseline=self.baseline(); sound=self.root/m.PATTERN_SOUND_REL
  b=self.root/'soundbaseline.json';b.write_bytes(sound.read_bytes()); v=json.loads(b.read_text());v['cues'][0]['soundEvent']='unresolved.authoring.sound';c=self.root/'soundcandidate.json';c.write_text(json.dumps(v),encoding='utf-8')
  p=self.root/'sourcepatch.json';p.write_text(json.dumps({'schema':'lostark.valtan-tuning-draft-patch','formatVersion':1,'sourceRevision':self.repository_revision,'operations':[]}),encoding='utf-8')
  before=self.data_manifest()
  result=m.commit_source_authoring_patch(self.root,p,source_baseline_root=baseline,pattern_sound_baseline_path=b,pattern_sound_candidate_path=c)
  self.assertTrue(result['sourceOnly']);self.assertEqual(sound.read_bytes(),c.read_bytes())
  after=self.data_manifest(); changed=[k for k in before if before[k]!=after[k]];self.assertEqual([m.PATTERN_SOUND_REL.removeprefix('Data/')],changed)
  patch=json.loads(p.read_text());patch['sourceRevision']=result['sourceRevision'];p.write_text(json.dumps(patch),encoding='utf-8')
  with self.assertRaises(Exception) as rejected: m.commit_typed_authoring_patch(self.root,p,pattern_sound_baseline_path=c,pattern_sound_candidate_path=c)
  self.assertIn('Sound bank/event',str(rejected.exception))
  self.assertEqual(after,self.data_manifest())
 def test_owner_cas_reject(self):
  baseline=self.baseline(); p=self.root/'sourcepatch.json';p.write_text(json.dumps({'schema':'lostark.valtan-tuning-draft-patch','formatVersion':1,'sourceRevision':self.repository_revision,'operations':[{'op':'SET_STAGE_CAMERAS','patternId':'VALTAN_FOUR_SLASH','stageId':'SLASHES','invocations':[]}]}),encoding='utf-8')
  target=self.root/'Data/Valtan/Valtan.gameplay.json';target.write_bytes(target.read_bytes()+b'\n');before=self.data_manifest()
  with self.assertRaisesRegex(Exception,'changed since Reload'):m.commit_source_authoring_patch(self.root,p,source_baseline_root=baseline)
  self.assertEqual(before,self.data_manifest())
 def test_noop_and_unrelated_owner(self):
  baseline=self.baseline();p=self.root/'sourcepatch.json'
  p.write_text(json.dumps({'schema':'lostark.valtan-tuning-draft-patch','formatVersion':1,'sourceRevision':self.repository_revision,'operations':[]}),encoding='utf-8')
  before=self.data_manifest();r=m.commit_source_authoring_patch(self.root,p,source_baseline_root=baseline);self.assertEqual(r['changedCount'],0);self.assertEqual(before,self.data_manifest())
  catalog=self.root/'Data/Effects/EffectCatalog.json';catalog.write_bytes(catalog.read_bytes()+b'\n')
  r=m.commit_source_authoring_patch(self.root,p,source_baseline_root=baseline);self.assertNotEqual(r['sourceRevision'],self.repository_revision);self.assertEqual(r['changedCount'],0)
 def test_source_sidecar_rollback(self):
  baseline=self.baseline();p=self.root/'sourcepatch.json';p.write_text(json.dumps({'schema':'lostark.valtan-tuning-draft-patch','formatVersion':1,'sourceRevision':self.repository_revision,'operations':[]}),encoding='utf-8')
  paths=[]
  for i,rel in enumerate((m.PATTERN_SOUND_REL,m.EFFECT_V2_BINDINGS_REL)):
   path=self.root/rel;b=self.root/f'baseline{i}.json';b.write_bytes(path.read_bytes());c=self.root/f'candidate{i}.json';v=json.loads(b.read_text())
   if i==0:v['cues'][0]['soundEvent']='unresolved.authoring.sound'
   else:v['bindings'][0]['resource']['id']='unresolved.authoring.effect'
   c.write_text(json.dumps(v),encoding='utf-8');paths.extend((b,c))
  before=self.data_manifest()
  with self.assertRaisesRegex(Exception,'injected'):
   m.commit_source_authoring_patch(self.root,p,source_baseline_root=baseline,pattern_sound_baseline_path=paths[0],pattern_sound_candidate_path=paths[1],effect_v2_baseline_path=paths[2],effect_v2_candidate_path=paths[3],inject_failure_after=1)
  self.assertEqual(before,self.data_manifest())
  result=m.commit_source_authoring_patch(self.root,p,source_baseline_root=baseline,pattern_sound_baseline_path=paths[0],pattern_sound_candidate_path=paths[1],effect_v2_baseline_path=paths[2],effect_v2_candidate_path=paths[3]);self.assertEqual(result['changedCount'],2)
  after=self.data_manifest();changed={key for key in before if before[key]!=after[key]};self.assertEqual(changed,{m.PATTERN_SOUND_REL.removeprefix('Data/'),m.EFFECT_V2_BINDINGS_REL.removeprefix('Data/')})
 def test_async_save_wrapper_receipt(self):
  baseline=self.baseline();p=self.root/'sourcepatch.json';p.write_text(json.dumps({'schema':'lostark.valtan-tuning-draft-patch','formatVersion':1,'sourceRevision':self.repository_revision,'operations':[]}),encoding='utf-8')
  result_path=self.root/'save-result.json'
  completed=subprocess.run(['powershell','-ExecutionPolicy','Bypass','-File',str(self.root/'Tools/ValtanPipeline/Run-ValtanAuthoringSaveJob.ps1'),'-ExpectedSourceRevision',self.repository_revision,'-ResultPath',str(result_path),'-DraftPatchPath',str(p),'-SourceOnly','-SourceBaselineRoot',str(baseline),'-CommitOnly'],cwd=self.root,capture_output=True,text=True,encoding='utf-8',errors='replace',env=self.environment)
  self.assertEqual(completed.returncode,0,completed.stdout+completed.stderr)
  result=json.loads(result_path.read_text(encoding='utf-8-sig'));self.assertTrue(result['ok']);self.assertTrue(result['canonicalCommitted']);self.assertEqual(result['sourceRevision'],self.repository_revision)
if __name__=='__main__':
 suite=unittest.TestSuite(TestSourceSave(n) for n in ('test_unresolved_sound_save','test_owner_cas_reject','test_noop_and_unrelated_owner','test_source_sidecar_rollback','test_async_save_wrapper_receipt'))
 result=unittest.TextTestRunner(verbosity=2).run(suite);sys.exit(not result.wasSuccessful())
