import sys,json,copy,unittest,hashlib,os,subprocess
from unittest.mock import patch
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from test_valtan_canonical_typed_patch_transaction import ValtanCanonicalTypedPatchTransactionTests, REPOSITORY_ROOT
import promote_valtan_animation_chains as m
import valtan_tuning_pipeline as bp
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
 def test_floor_wipe_natural_tail_trim_roundtrip(self):
  target=self.root/'Data/Valtan/Valtan.presentation.json'
  original=json.loads(target.read_text(encoding='utf-8'))
  pattern=next(x for x in original['patterns'] if x['patternId']=='VALTAN_FLOOR_WIPE_130')
  stage=next(x for x in pattern['stages'] if x['stageId']=='WINDUP')
  cue=next(x for x in stage['effectCues'] if x['effectAssetId']=='effect.valtan.floor-wipe-130')
  # Normalize only this copied fixture; the user's saved cue may already be trimmed.
  cue['stopPolicy']='natural';cue['sourceEndMs']=None
  target.write_text(json.dumps(original),encoding='utf-8')
  self.repository_revision=self.source_manifest()['sourceRevision'];baseline=self.baseline()
  trimmed=copy.deepcopy(cue);trimmed['stopPolicy']='cue_end';trimmed['sourceEndMs']=5000
  operation={'op':'UPDATE_EFFECT_CUE','patternId':pattern['patternId'],'stageId':stage['stageId'],'actionId':stage['actionId'],'cueId':cue['cueId'],'occurrenceId':cue['occurrenceId'],'cue':trimmed}
  document={'schema':'lostark.valtan-tuning-draft-patch','formatVersion':1,'sourceRevision':self.repository_revision,'operations':[operation]}
  patch_path=self.root/'trim.json';patch_path.write_text(json.dumps(document),encoding='utf-8')
  before=self.data_manifest()
  result=m.commit_source_authoring_patch(self.root,patch_path,source_baseline_root=baseline)
  self.assertTrue(result['sourceOnly'])
  saved=json.loads(target.read_text(encoding='utf-8'))
  expected=copy.deepcopy(original)
  expected_stage=next(x for p in expected['patterns'] if p['patternId']==pattern['patternId'] for x in p['stages'] if x['stageId']==stage['stageId'])
  expected_stage['effectCues']=[trimmed if x['cueId']==cue['cueId'] else x for x in expected_stage['effectCues']]
  self.assertEqual(saved,expected)
  after=self.data_manifest()
  self.assertEqual([key for key in before if before[key]!=after[key]],['Valtan/Valtan.presentation.json'])
  # Reload the committed baseline and reject a zero-length trim without writes.
  baseline=self.baseline();document['sourceRevision']=result['sourceRevision']
  document['operations'][0]['cue']['sourceEndMs']=cue['sourceStartMs']
  patch_path.write_text(json.dumps(document),encoding='utf-8')
  with self.assertRaises(Exception):m.commit_source_authoring_patch(self.root,patch_path,source_baseline_root=baseline)
  self.assertEqual(after,self.data_manifest())
 def test_effect_playback_offset_and_finite_once_tail_roundtrip(self):
  baseline=self.baseline()
  target=self.root/'Data/Valtan/Valtan.presentation.json'
  original=json.loads(target.read_text(encoding='utf-8'))
  pattern=next(x for x in original['patterns'] if x['patternId']=='VALTAN_FLOOR_WIPE_130')
  stage=next(x for x in pattern['stages'] if x['stageId']=='SECOND_SMASH')
  cue=copy.deepcopy(stage['effectCues'][0])
  clip=next(x for x in stage['animation']['occurrences'] if x['clipOccurrenceId']==cue['clipOccurrenceId'])
  self.assertGreater(clip['playMs'],0)
  edited=copy.deepcopy(cue)
  edited.update(playbackOffsetMs=1250,stopPolicy='cue_end',sourceEndMs=clip['sourceStartMs']+clip['playMs']+4500)
  edited['localTransform']={'position':[3.0,2.0,-1.0],'rotationDegrees':[0.0,45.0,0.0],'scale':[1.2,1.0,1.2]}
  operation={'op':'UPDATE_EFFECT_CUE','patternId':pattern['patternId'],'stageId':stage['stageId'],'actionId':stage['actionId'],'cueId':cue['cueId'],'occurrenceId':cue['occurrenceId'],'cue':edited}
  document={'schema':'lostark.valtan-tuning-draft-patch','formatVersion':1,'sourceRevision':self.repository_revision,'operations':[operation]}
  patch_path=self.root/'effect-front-trim.json'
  patch_path.write_text(json.dumps(document),encoding='utf-8')
  before=self.data_manifest()
  result=m.commit_source_authoring_patch(self.root,patch_path,source_baseline_root=baseline)
  self.assertTrue(result['sourceOnly'])
  after=self.data_manifest()
  self.assertEqual([key for key in before if before[key]!=after[key]],['Valtan/Valtan.presentation.json'])
  saved=json.loads(target.read_text(encoding='utf-8'))
  expected=copy.deepcopy(original)
  expected_stage=next(x for p in expected['patterns'] if p['patternId']==pattern['patternId'] for x in p['stages'] if x['stageId']==stage['stageId'])
  expected_stage['effectCues']=[edited if x['cueId']==cue['cueId'] else x for x in expected_stage['effectCues']]
  self.assertEqual(saved,expected)
  # Exercise the real Publish projection, keeping every generated Product in memory.
  _,_,outputs=bp.build_repository_product_projection(self.root)
  projected=next(x for x in json.loads(outputs[bp.CUES_REL])['cues'] if x['bindingId']==cue['cueId'])
  self.assertEqual(projected['playbackOffsetMs'],1250)
  self.assertEqual(projected['sourceStartMs'],cue['sourceStartMs'])
  self.assertEqual(projected['sourceEndMs'],edited['sourceEndMs'])
  self.assertEqual(projected['localTransform'],edited['localTransform'])
  self.assertEqual(after,self.data_manifest())
  # Explicit zero must survive; an absent field must keep legacy origin semantics.
  baseline=self.baseline();document['sourceRevision']=result['sourceRevision']
  edited['playbackOffsetMs']=0
  patch_path.write_text(json.dumps(document),encoding='utf-8')
  result=m.commit_source_authoring_patch(self.root,patch_path,source_baseline_root=baseline)
  self.assertEqual(bp.compile_cue(pattern['patternId'],stage,edited)['playbackOffsetMs'],0)
  saved=json.loads(target.read_text(encoding='utf-8'))
  self.assertEqual(next(x for p in saved['patterns'] if p['patternId']==pattern['patternId'] for st in p['stages'] if st['stageId']==stage['stageId'] for x in st['effectCues'] if x['cueId']==cue['cueId'])['playbackOffsetMs'],0)
  absent=copy.deepcopy(edited);del absent['playbackOffsetMs']
  self.assertNotIn('playbackOffsetMs',bp.compile_cue(pattern['patternId'],stage,absent))
  effect_ref={'refId':cue['cueId'],'cueProjection':{k:edited[k] for k in ('clipOccurrenceId','sourceStartMs','sourceEndMs')}}
  product=bp.compile_cue(pattern['patternId'],stage,edited)
  self.assertEqual(bp._migrate_effect_cue(effect_ref,{cue['cueId']:product},{clip['clipOccurrenceId']:clip},'offset migration')['playbackOffsetMs'],0)
  del product['playbackOffsetMs']
  self.assertNotIn('playbackOffsetMs',bp._migrate_effect_cue(effect_ref,{cue['cueId']:product},{clip['clipOccurrenceId']:clip},'legacy migration'))
  # Invalid offset payloads and escaped starts reject without modifying any file.
  baseline=self.baseline();document['sourceRevision']=result['sourceRevision'];after=self.data_manifest()
  for value in (-1,600001,True,0.5,'1250',None):
   with self.subTest(playbackOffsetMs=value):
    edited['playbackOffsetMs']=value;patch_path.write_text(json.dumps(document),encoding='utf-8')
    with self.assertRaisesRegex(Exception,'playbackOffsetMs'):m.commit_source_authoring_patch(self.root,patch_path,source_baseline_root=baseline)
    self.assertEqual(after,self.data_manifest())
  edited['playbackOffsetMs']=600000
  bp._validate_draft_effect_cue_payload(self.root,bp.read_json(self.root/bp.EFFECT_CATALOG_REL),pattern,stage,edited,0)
  bp.validate_cue_animation_join(edited,{clip['clipOccurrenceId']:clip},'finite once tail')
  loop=copy.deepcopy(edited);loop['repeatPolicy']='each_loop'
  with self.assertRaisesRegex(bp.PipelineError,'escapes'):bp.validate_cue_animation_join(loop,{clip['clipOccurrenceId']:clip},'each-loop tail')
  escaped=copy.deepcopy(edited);escaped['sourceStartMs']=clip['sourceStartMs']+clip['playMs']
  with self.assertRaisesRegex(bp.PipelineError,'escapes'):bp.validate_cue_animation_join(escaped,{clip['clipOccurrenceId']:clip},'escaped start')
  edited['sourceEndMs']=600001
  with self.assertRaisesRegex(bp.PipelineError,'sourceEndMs'):bp.validate_cue_animation_join(edited,{clip['clipOccurrenceId']:clip},'bounded tail')
 def test_sound_trim_duplicate_move_source_save_and_publish(self):
  baseline=self.baseline();sound=self.root/m.PATTERN_SOUND_REL
  original_bytes=sound.read_bytes();document=json.loads(original_bytes)
  presentation=bp.read_json(self.root/bp.PRESENTATION_AUTHORING_REL)
  patterns={p['patternId']:p for p in presentation['patterns']}
  pair=next((row,stage) for row in document['cues'] if row['patternId'] in patterns for stage in patterns[row['patternId']]['stages'] if stage['stageId']!=row['stageId'] and stage['animation'].get('occurrences') and stage['animation']['occurrences'][0]['sourceStartMs']<=60000)
  row,stage=pair;clip=stage['animation']['occurrences'][0]
  row.update(playbackOffsetMs=250,playbackDurationMs=700)
  clone=copy.deepcopy(row);clone['bindingId']+='.trim-copy';clone['occurrenceId']=clone['bindingId']+'.occurrence.01'
  clone.update(stageId=stage['stageId'],actionId=stage['actionId'],clipOccurrenceId=clip['clipOccurrenceId'],startMs=clip['sourceStartMs'],repeatPolicy='once',playbackOffsetMs=0)
  document['cues'].append(clone)
  sound_baseline=self.root/'sound-trim-baseline.json';sound_baseline.write_bytes(original_bytes)
  sound_candidate=self.root/'sound-trim-candidate.json';sound_candidate.write_text(json.dumps(document),encoding='utf-8')
  patch_path=self.root/'sound-trim-patch.json';patch_document={'schema':'lostark.valtan-tuning-draft-patch','formatVersion':1,'sourceRevision':self.repository_revision,'operations':[]};patch_path.write_text(json.dumps(patch_document),encoding='utf-8')
  before=self.data_manifest()
  result=m.commit_source_authoring_patch(self.root,patch_path,source_baseline_root=baseline,pattern_sound_baseline_path=sound_baseline,pattern_sound_candidate_path=sound_candidate)
  self.assertTrue(result['sourceOnly']);self.assertEqual(sound.read_bytes(),sound_candidate.read_bytes())
  after=self.data_manifest();self.assertEqual([key for key in before if before[key]!=after[key]],[m.PATTERN_SOUND_REL.removeprefix('Data/')])
  _,_,outputs=bp.build_repository_product_projection(self.root)
  self.assertEqual(json.loads(outputs['Data/Valtan/Published/Valtan.patternsoundcues.json']),document)
  self.assertEqual(after,self.data_manifest())
  # A copy moved to another Stage must still satisfy its exact action/clip join at Publish.
  invalid_join=copy.deepcopy(document);invalid_join['cues'][-1]['actionId']='valtan.invalid.action'
  with self.assertRaisesRegex(Exception,'action does not match'):
   m._validate_pattern_sound_dependencies_against_candidate_products(self.root,outputs,sound_source_bytes=json.dumps(invalid_join).encode('utf-8'))
  self.assertEqual(after,self.data_manifest())
  sound_baseline.write_bytes(sound.read_bytes());patch_document['sourceRevision']=result['sourceRevision'];patch_path.write_text(json.dumps(patch_document),encoding='utf-8')
  for field,value in (('playbackOffsetMs',-1),('playbackOffsetMs',600001),('playbackOffsetMs',True),('playbackOffsetMs',None),('playbackDurationMs',0),('playbackDurationMs',600001),('playbackDurationMs',0.5),('playbackDurationMs','700')):
   with self.subTest(field=field,value=value):
    invalid=copy.deepcopy(document);invalid['cues'][-1][field]=value;sound_candidate.write_text(json.dumps(invalid),encoding='utf-8')
    with self.assertRaisesRegex(Exception,field):
     m.commit_source_authoring_patch(self.root,patch_path,source_baseline_root=baseline,pattern_sound_baseline_path=sound_baseline,pattern_sound_candidate_path=sound_candidate)
    self.assertEqual(after,self.data_manifest())
  boundary=copy.deepcopy(document);boundary['cues'][-1].update(playbackOffsetMs=600000,playbackDurationMs=600000)
  m._validate_source_sidecar(m.PATTERN_SOUND_REL,json.dumps(boundary).encode('utf-8'))
  absent=copy.deepcopy(document);absent['cues'][-1].pop('playbackOffsetMs');absent['cues'][-1].pop('playbackDurationMs')
  m._validate_source_sidecar(m.PATTERN_SOUND_REL,json.dumps(absent).encode('utf-8'))
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
 suite=unittest.TestSuite(TestSourceSave(n) for n in ('test_sound_trim_duplicate_move_source_save_and_publish','test_effect_playback_offset_and_finite_once_tail_roundtrip','test_floor_wipe_natural_tail_trim_roundtrip','test_unresolved_sound_save','test_owner_cas_reject','test_noop_and_unrelated_owner','test_source_sidecar_rollback','test_async_save_wrapper_receipt'))
 result=unittest.TextTestRunner(verbosity=2).run(suite);sys.exit(not result.wasSuccessful())
