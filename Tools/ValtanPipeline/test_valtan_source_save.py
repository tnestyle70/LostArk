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
 def test_canonical_animation_delete_preserves_gameplay_and_roundtrips_none(self):
  pattern_id='VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK'
  gameplay_before=(self.root/m.GAMEPLAY_REL).read_bytes()
  presentation=json.loads((self.root/m.PRESENTATION_REL).read_text(encoding='utf-8'))
  source_pattern=next(row for row in presentation['patterns'] if row['patternId']==pattern_id)
  airborne=next(row for row in source_pattern['stages'] if row['stageId']=='AIRBORNE')
  previous_animation=copy.deepcopy(airborne['animation'])
  baseline=self.baseline()
  patch_path=self.root/'canonical-delete.json'
  def write_patch(stage_id,animation):
   patch_path.write_text(json.dumps({'schema':'lostark.valtan-tuning-draft-patch','formatVersion':1,'sourceRevision':self.source_manifest()['sourceRevision'],'operations':[{'op':'SET_STAGE_ANIMATION','patternId':pattern_id,'stageId':stage_id,'animation':animation}]}),encoding='utf-8')
  write_patch('AIRBORNE',{'mode':'NONE'})
  saved=m.commit_source_authoring_patch(self.root,patch_path,source_baseline_root=baseline)
  self.assertTrue(saved['sourceOnly'])
  self.assertEqual(gameplay_before,(self.root/m.GAMEPLAY_REL).read_bytes())
  reopened=json.loads((self.root/m.PRESENTATION_REL).read_text(encoding='utf-8'))
  expected=copy.deepcopy(presentation)
  next(stage for row in expected['patterns'] if row['patternId']==pattern_id for stage in row['stages'] if stage['stageId']=='AIRBORNE')['animation']={'mode':'NONE'}
  self.assertEqual(expected,reopened)
  _,joined,outputs=bp.build_repository_product_projection(self.root)
  product=json.loads(outputs[bp.BINDINGS_REL])
  binding=next(row for row in product['bindings'] if row['actionId']==airborne['actionId'])
  self.assertEqual('NONE',binding['playbackMode']);self.assertEqual([],binding['clips'])
  self.assertEqual({'mode':'NONE'},next(stage for row in joined['patterns'] if row['patternId']==pattern_id for stage in row['stages'] if stage['stageId']=='AIRBORNE')['animation'])
  # A clip cannot disappear while its exact V1 dependency still refers to it.
  before=self.data_manifest();write_patch('TAKEOFF',{'mode':'NONE'})
  with self.assertRaises(Exception):m.commit_source_authoring_patch(self.root,patch_path,source_baseline_root=self.baseline())
  self.assertEqual(before,self.data_manifest())
  # Replacing the blank presentation must restore the real source clip, not an idle fallback.
  write_patch('AIRBORNE',previous_animation)
  m.commit_source_authoring_patch(self.root,patch_path,source_baseline_root=self.baseline())
  self.assertEqual(presentation,json.loads((self.root/m.PRESENTATION_REL).read_text(encoding='utf-8')))
  self.assertEqual(gameplay_before,(self.root/m.GAMEPLAY_REL).read_bytes())
 def test_canonical_animation_delete_dependency_cascade_save_and_publish(self):
  pattern_id='VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK'
  gameplay_before=(self.root/m.GAMEPLAY_REL).read_bytes()
  presentation=json.loads((self.root/m.PRESENTATION_REL).read_text(encoding='utf-8'))
  pattern=next(row for row in presentation['patterns'] if row['patternId']==pattern_id)
  stages=[row for row in pattern['stages'] if row['stageId'] in ('TAKEOFF','IMPACT')]
  clips={row['animation']['occurrences'][0]['clipOccurrenceId'] for row in stages}
  operations=[]
  for stage in stages:
   for cue in stage['effectCues']:
    if cue.get('clipOccurrenceId') in clips:
     operations.append({'op':'REMOVE_EFFECT_CUE','patternId':pattern_id,'stageId':stage['stageId'],'actionId':stage['actionId'],**{key:cue[key] for key in ('cueId','occurrenceId','effectAssetId','clipOccurrenceId')}})
   operations.append({'op':'SET_STAGE_ANIMATION','patternId':pattern_id,'stageId':stage['stageId'],'animation':{'mode':'NONE'}})
  kwargs={};expected_sidecars={}
  for prefix,relative,rows in [('pattern_sound',m.PATTERN_SOUND_REL,'cues'),('pattern_shake',m.PATTERN_SHAKE_REL,'cues'),('effect_v2',m.EFFECT_V2_BINDINGS_REL,'bindings')]:
   baseline=self.root/(prefix+'-before.json');baseline.write_bytes((self.root/relative).read_bytes())
   document=json.loads(baseline.read_text(encoding='utf-8'))
   def owned(row):
    scope=row.get('scope',row);clock=row.get('clock',row)
    return scope.get('patternId')==pattern_id and clock.get('clipOccurrenceId') in clips
   removed=[row for row in document[rows] if owned(row)];self.assertTrue(removed,prefix)
   document[rows]=[row for row in document[rows] if not owned(row)]
   candidate=self.root/(prefix+'-after.json');candidate.write_text(json.dumps(document),encoding='utf-8')
   kwargs[prefix+'_baseline_path']=baseline;kwargs[prefix+'_candidate_path']=candidate
   expected_sidecars[relative]=document
  patch_path=self.root/'canonical-cascade.json';patch_doc={'schema':'lostark.valtan-tuning-draft-patch','formatVersion':1,'sourceRevision':self.source_manifest()['sourceRevision'],'operations':operations};patch_path.write_text(json.dumps(patch_doc),encoding='utf-8')
  baseline=self.baseline();before=self.data_manifest()
  with self.assertRaisesRegex(Exception,'injected'):
   m.commit_source_authoring_patch(self.root,patch_path,source_baseline_root=baseline,inject_failure_after=1,**kwargs)
  self.assertEqual(before,self.data_manifest())
  m.commit_source_authoring_patch(self.root,patch_path,source_baseline_root=baseline,**kwargs)
  self.assertEqual(gameplay_before,(self.root/m.GAMEPLAY_REL).read_bytes())
  for relative,expected in expected_sidecars.items():self.assertEqual(expected,json.loads((self.root/relative).read_text(encoding='utf-8')))
  # Save & Publish uses the same source and refreshes root motion as well as bindings.
  patch_doc.update(sourceRevision=self.source_manifest()['sourceRevision'],operations=[]);patch_path.write_text(json.dumps(patch_doc),encoding='utf-8')
  m.commit_typed_authoring_patch(self.root,patch_path)
  self.assertEqual(gameplay_before,(self.root/m.GAMEPLAY_REL).read_bytes())
  bindings=json.loads((self.root/bp.BINDINGS_REL).read_text(encoding='utf-8'))
  for stage in stages:
   binding=next(row for row in bindings['bindings'] if row['actionId']==stage['actionId'])
   self.assertEqual('NONE',binding['playbackMode']);self.assertEqual([],binding['clips'])
 def test_empty_create_raw_clip_save_reopen_and_isolated_publish(self):
  request={"schema":m.CREATE_REQUEST_SCHEMA,"formatVersion":m.CREATE_REQUEST_FORMAT_VERSION,"expectedSourceSha256":hashlib.sha256((self.root/m.DEBUG_REL).read_bytes()).hexdigest(),"patternId":"VALTAN_EMPTY_SOURCE_SAVE_TEST","displayName":"Empty authoring test","authoringPhase":1,"targetPolicy":"NONE","aimPolicy":"NONE","intakeChain":{"selectionKind":"EMPTY_PATTERN"}}
  request_path=self.root/'empty-create.json';request_path.write_text(json.dumps(request),encoding='utf-8')
  created=m.create_pattern_from_request(self.root,request_path,'Apply')
  self.assertTrue(created['sourceOnly']);self.assertEqual(0,created['projectedArtifactCount'])
  self.repository_revision=self.source_manifest()['sourceRevision'];baseline=self.baseline()
  gameplay=json.loads((self.root/m.GAMEPLAY_REL).read_text(encoding='utf-8'))
  presentation=json.loads((self.root/m.PRESENTATION_REL).read_text(encoding='utf-8'))
  stage=presentation['patterns'][-1]['stages'][0]
  animation={'endPolicy':'EXACT','repeatCount':1,'occurrences':[{'clipOccurrenceId':stage['actionId']+'.clip-01','clip':'mesh_att_battle_12_01','mappingBasis':'SOURCE_REVIEWED_DELTA','sourceStartMs':0,'playMs':1000,'playRate':1.0,'repeatUntilStageEnd':False}]}
  patch_path=self.root/'empty-append.json';patch_path.write_text(json.dumps({'schema':'lostark.valtan-tuning-draft-patch','formatVersion':1,'sourceRevision':self.repository_revision,'operations':[{'op':'SET_STAGE_ANIMATION','patternId':request['patternId'],'stageId':'STEP_01','animation':animation}]}),encoding='utf-8')
  before=self.data_manifest();saved=m.commit_source_authoring_patch(self.root,patch_path,source_baseline_root=baseline)
  self.assertTrue(saved['sourceOnly']);after=self.data_manifest()
  self.assertEqual(['Valtan/Valtan.presentation.json'],[key for key in before if before[key]!=after[key]])
  reopened=json.loads((self.root/m.PRESENTATION_REL).read_text(encoding='utf-8'))
  self.assertEqual(animation,reopened['patterns'][-1]['stages'][0]['animation'])
  self.assertEqual([],reopened['patterns'][-1]['presentationSources'])
  _,_,outputs=bp.build_repository_product_projection(self.root)
  for relative,text in outputs.items():
   target=self.root/relative;target.parent.mkdir(parents=True,exist_ok=True);target.write_text(text,encoding='utf-8')
  output_root=self.root/'isolated-gameplay'
  completed=subprocess.run(['powershell','-NoProfile','-ExecutionPolicy','Bypass','-File',str(REPOSITORY_ROOT/'Tools/GameplayPipeline/Publish-GameplayBalance.ps1'),'-Mode','Publish','-InputOverlayRoot',str(self.root),'-OutputRoot',str(output_root)],capture_output=True,text=True,encoding='utf-8',errors='replace',env=self.environment)
  self.assertEqual(0,completed.returncode,completed.stdout+completed.stderr)
  bootstrap=(output_root/'Gameplay.bootstrap').read_text(encoding='utf-8')
  self.assertIn('PATTERN\tENCOUNTER_VALTAN\t'+request['patternId']+'\t',bootstrap)
  self.assertNotIn('PATTERNSOURCE\tENCOUNTER_VALTAN\t'+request['patternId']+'\t',bootstrap)

 def test_empty_stage_effect_add_trim_copy_map_save_publish_remove(self):
  request={"schema":m.CREATE_REQUEST_SCHEMA,"formatVersion":m.CREATE_REQUEST_FORMAT_VERSION,"expectedSourceSha256":hashlib.sha256((self.root/m.DEBUG_REL).read_bytes()).hexdigest(),"patternId":"VALTAN_EMPTY_EFFECT_TEST","displayName":"Independent Effect test","authoringPhase":1,"targetPolicy":"NONE","aimPolicy":"NONE","intakeChain":{"selectionKind":"EMPTY_PATTERN"}}
  request_path=self.root/'empty-effect-create.json';request_path.write_text(json.dumps(request),encoding='utf-8')
  created=m.create_pattern_from_request(self.root,request_path,'Apply');self.assertTrue(created['sourceOnly'])
  self.repository_revision=self.source_manifest()['sourceRevision'];baseline=self.baseline()
  target=self.root/m.PRESENTATION_REL;source=json.loads(target.read_text(encoding='utf-8'))
  pattern=next(p for p in source['patterns'] if p['patternId']==request['patternId']);stage=pattern['stages'][0]
  cue={'cueId':'cue.valtan.composition.empty-effect-test','occurrenceId':'cue.valtan.composition.empty-effect-test.occurrence.01','effectAssetId':'effect.valtan.carrier-v1.attack.backstep.windup.clip-01','timingBasis':'STAGE_CLOCK','stageOffsetMs':100,'anchorSlotId':'root','followPolicy':'follow','stopPolicy':'natural','repeatPolicy':'once','localTransform':{'position':[0,0,0],'rotationDegrees':[0,0,0],'scale':[1,1,1]},'scalePolicy':{'kind':'OWNER_RELATIVE'}}
  def operation(kind,value):
   result={'op':kind,'patternId':pattern['patternId'],'stageId':stage['stageId'],'actionId':stage['actionId'],'cue':copy.deepcopy(value)}
   if kind=='UPDATE_EFFECT_CUE':result.update(cueId=value['cueId'],occurrenceId=value['occurrenceId'])
   return result
  patch_path=self.root/'effect.json'
  def save(ops):
   baseline=self.baseline();revision=self.source_manifest()['sourceRevision']
   patch_path.write_text(json.dumps({'schema':'lostark.valtan-tuning-draft-patch','formatVersion':1,'sourceRevision':revision,'operations':ops}),encoding='utf-8')
   return m.commit_source_authoring_patch(self.root,patch_path,source_baseline_root=baseline)
  self.assertTrue(save([operation('ADD_EFFECT_CUE',cue)])['sourceOnly'])
  cue.update(anchorSlotId='map',followPolicy='snapshot',stageOffsetMs=250,stageEndMs=5250,stopPolicy='cue_end',playbackOffsetMs=300)
  cue['localTransform']['position']=[3,1,-2]
  clone=copy.deepcopy(cue);clone['cueId']+='-copy';clone['occurrenceId']=clone['cueId']+'.occurrence.01';clone['stageOffsetMs']=500;clone['stageEndMs']=5500
  save([operation('UPDATE_EFFECT_CUE',cue),operation('ADD_EFFECT_CUE',clone)])
  saved=json.loads(target.read_text(encoding='utf-8'));saved_stage=next(st for p in saved['patterns'] if p['patternId']==pattern['patternId'] for st in p['stages'])
  self.assertEqual(saved_stage['animation'],stage['animation']);self.assertEqual(saved_stage['animation']['mode'],'NONE')
  self.assertEqual(saved_stage['effectCues'],[cue,clone])
  _,_,outputs=bp.build_repository_product_projection(self.root)
  projected=[row for row in json.loads(outputs[bp.CUES_REL])['cues'] if row['patternId']==pattern['patternId']]
  self.assertEqual(len(projected),2)
  self.assertTrue(all('clipOccurrenceId' not in row and row['anchorSlotId']=='map' for row in projected))
  self.assertEqual(projected[0]['stageEndMs'],5250);self.assertEqual(projected[0]['playbackOffsetMs'],300)
  before=self.data_manifest()
  for changes in ({'stageOffsetMs':1000},{'stageEndMs':250},{'stageEndMs':600001},{'followPolicy':'follow'},{'repeatPolicy':'each_loop'}):
   invalid=copy.deepcopy(cue);invalid.update(changes)
   with self.assertRaises(Exception):save([operation('UPDATE_EFFECT_CUE',invalid)])
   self.assertEqual(before,self.data_manifest())
  remove={'op':'REMOVE_EFFECT_CUE','patternId':pattern['patternId'],'stageId':stage['stageId'],'actionId':stage['actionId'],'cueId':cue['cueId'],'occurrenceId':cue['occurrenceId'],'effectAssetId':cue['effectAssetId'],'clipOccurrenceId':''}
  save([remove])
  final=json.loads(target.read_text(encoding='utf-8'));final_stage=next(st for p in final['patterns'] if p['patternId']==pattern['patternId'] for st in p['stages'])
  self.assertEqual(final_stage['effectCues'],[clone])
 def test_empty_v2_only_scope_save_project_and_collider_only_project(self):
  request={"schema":m.CREATE_REQUEST_SCHEMA,"formatVersion":m.CREATE_REQUEST_FORMAT_VERSION,"expectedSourceSha256":hashlib.sha256((self.root/m.DEBUG_REL).read_bytes()).hexdigest(),"patternId":"VALTAN_EMPTY_V2_ONLY_TEST","displayName":"Independent V2 test","authoringPhase":1,"targetPolicy":"NONE","aimPolicy":"NONE","intakeChain":{"selectionKind":"EMPTY_PATTERN"}}
  request_path=self.root/'v2-empty-create.json';request_path.write_text(json.dumps(request),encoding='utf-8')
  self.assertTrue(m.create_pattern_from_request(self.root,request_path,'Apply')['sourceOnly'])
  source=json.loads((self.root/m.PRESENTATION_REL).read_text(encoding='utf-8'))
  stage=next(p for p in source['patterns'] if p['patternId']==request['patternId'])['stages'][0]
  _,_,empty_outputs=bp.build_repository_product_projection(self.root)
  self.assertNotIn(request['patternId'],{p['patternId'] for p in json.loads(empty_outputs[bp.ENCOUNTER_REL])['patterns']})
  baseline=self.baseline();binding_path=self.root/m.EFFECT_V2_BINDINGS_REL
  binding_baseline=self.root/'v2-baseline.json';binding_baseline.write_bytes(binding_path.read_bytes())
  document=json.loads(binding_baseline.read_text(encoding='utf-8'))
  binding=copy.deepcopy(next(row for row in document['bindings'] if row['clock']['basis']=='STAGE'))
  binding['bindingId']='binding.valtan.empty-v2-only-test'
  binding['scope']={'patternId':request['patternId'],'stageId':stage['stageId'],'actionId':stage['actionId']}
  binding['clock']={'basis':'STAGE','clipOccurrenceId':None,'startMs':100,'repeatPolicy':'ONCE'}
  document['bindings'].append(binding);document['bindings'].sort(key=lambda row:row['bindingId'])
  candidate=self.root/'v2-candidate.json';candidate.write_text(json.dumps(document),encoding='utf-8')
  patch_path=self.root/'v2-only-save.json';patch_path.write_text(json.dumps({'schema':'lostark.valtan-tuning-draft-patch','formatVersion':1,'sourceRevision':self.source_manifest()['sourceRevision'],'operations':[]}),encoding='utf-8')
  result=m.commit_source_authoring_patch(self.root,patch_path,source_baseline_root=baseline,effect_v2_baseline_path=binding_baseline,effect_v2_candidate_path=candidate)
  self.assertTrue(result['sourceOnly']);self.assertEqual(binding_path.read_bytes(),candidate.read_bytes())
  self.assertEqual(source,json.loads((self.root/m.PRESENTATION_REL).read_text(encoding='utf-8')))
  _,_,outputs=bp.build_repository_product_projection(self.root)
  self.assertIn(request['patternId'],{p['patternId'] for p in json.loads(outputs[bp.ENCOUNTER_REL])['patterns']})
  animation=next(row for row in json.loads(outputs[bp.BINDINGS_REL])['bindings'] if row['actionId']==stage['actionId'])
  self.assertEqual('NONE',animation['playbackMode']);self.assertEqual([],animation['clips'])
  self.assertNotIn(request['patternId'],{cue['patternId'] for cue in json.loads(outputs[bp.CUES_REL])['cues']})
  projected=json.loads(outputs['Data/Valtan/Published/BOSS_VALTAN.effectv2bindings.json'])
  self.assertEqual(binding,next(row for row in projected['bindings'] if row['bindingId']==binding['bindingId']))
  for relative,text in outputs.items():
   path=self.root/relative;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(text,encoding='utf-8')
  # Scope identity must resolve exactly; a coincident action/name is insufficient.
  next(row for row in document['bindings'] if row['bindingId']==binding['bindingId'])['scope']['stageId']='WRONG_STAGE'
  binding_path.write_text(json.dumps(document),encoding='utf-8')
  with self.assertRaisesRegex(Exception,'scope does not resolve'):
   bp.build_repository_product_projection(self.root)
  # Gameplay-only Collider content is also real playable content without a clip.
  binding_path.write_bytes(binding_baseline.read_bytes())
  _,_,removed=bp.build_repository_product_projection(self.root)
  self.assertNotIn(request['patternId'],{p['patternId'] for p in json.loads(removed[bp.ENCOUNTER_REL])['patterns']})
  self.assertEqual(source,json.loads((self.root/m.PRESENTATION_REL).read_text(encoding='utf-8')))
  gameplay_path=self.root/m.GAMEPLAY_REL;gameplay=json.loads(gameplay_path.read_text(encoding='utf-8'))
  hit=copy.deepcopy(next(st['hit'] for p in gameplay['patterns'] for st in p['stages'] if st['hit']['shape']['kind']=='CIRCLE'))
  hit['schedule']={'kind':'INTERVAL','count':1,'firstOffsetMs':100,'intervalMs':0}
  next(p for p in gameplay['patterns'] if p['patternId']==request['patternId'])['stages'][0]['hit']=hit
  gameplay_path.write_text(json.dumps(gameplay),encoding='utf-8')
  _,_,outputs=bp.build_repository_product_projection(self.root)
  self.assertIn(request['patternId'],{p['patternId'] for p in json.loads(outputs[bp.ENCOUNTER_REL])['patterns']})

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
 def test_pattern_shake_source_and_typed_save_atomic_contract(self):
  baseline=self.baseline();target=self.root/m.PATTERN_SHAKE_REL
  sidecar_baseline=self.root/'shake-baseline.json';sidecar_baseline.write_bytes(target.read_bytes())
  original=json.loads(sidecar_baseline.read_text(encoding='utf-8'))
  candidate=copy.deepcopy(original);clone=copy.deepcopy(candidate['cues'][0])
  clone['bindingId']+='-copy';clone['occurrenceId']+='-copy';candidate['cues'].append(clone)
  sidecar_candidate=self.root/'shake-candidate.json';sidecar_candidate.write_text(json.dumps(candidate),encoding='utf-8')
  patch_path=self.root/'shake-save.json'
  def write_patch():
   patch_path.write_text(json.dumps({'schema':'lostark.valtan-tuning-draft-patch','formatVersion':1,'sourceRevision':self.source_manifest()['sourceRevision'],'operations':[]}),encoding='utf-8')
  write_patch();before=self.data_manifest()
  for change in ({'patternId':'INVALID_PATTERN'},{'stageId':'INVALID_STAGE'},{'clipOccurrenceId':'invalid.clip'},{'startMs':60001},{'shake':'dur=-1;in=0;out=0;x=0,0;y=0,0;z=0,0;fov=0,0'}):
   invalid=copy.deepcopy(candidate);invalid['cues'][-1].update(change);sidecar_candidate.write_text(json.dumps(invalid),encoding='utf-8')
   with self.assertRaises(Exception):
    m.commit_source_authoring_patch(self.root,patch_path,source_baseline_root=baseline,pattern_shake_baseline_path=sidecar_baseline,pattern_shake_candidate_path=sidecar_candidate)
   self.assertEqual(before,self.data_manifest())
  sidecar_candidate.write_text(json.dumps(candidate),encoding='utf-8')
  sound_target=self.root/m.PATTERN_SOUND_REL;sound_baseline=self.root/'shake-sound-baseline.json';sound_baseline.write_bytes(sound_target.read_bytes())
  sound_candidate=self.root/'shake-sound-candidate.json';sound_candidate.write_text(json.dumps(json.loads(sound_baseline.read_text(encoding='utf-8'))),encoding='utf-8')
  with self.assertRaisesRegex(Exception,'injected'):
   m.commit_source_authoring_patch(self.root,patch_path,source_baseline_root=baseline,pattern_shake_baseline_path=sidecar_baseline,pattern_shake_candidate_path=sidecar_candidate,pattern_sound_baseline_path=sound_baseline,pattern_sound_candidate_path=sound_candidate,inject_failure_after=1)
  self.assertEqual(before,self.data_manifest())
  target.write_bytes(target.read_bytes()+b'\n');stale=self.data_manifest()
  with self.assertRaisesRegex(Exception,'changed since Reload'):
   m.commit_source_authoring_patch(self.root,patch_path,source_baseline_root=baseline,pattern_shake_baseline_path=sidecar_baseline,pattern_shake_candidate_path=sidecar_candidate)
  self.assertEqual(stale,self.data_manifest());target.write_bytes(sidecar_baseline.read_bytes())
  result_path=self.root/'shake-save-result.json'
  completed=subprocess.run(['powershell','-NoProfile','-ExecutionPolicy','Bypass','-File',str(self.root/'Tools/ValtanPipeline/Run-ValtanAuthoringSaveJob.ps1'),'-ExpectedSourceRevision',self.source_manifest()['sourceRevision'],'-ResultPath',str(result_path),'-DraftPatchPath',str(patch_path),'-SourceOnly','-SourceBaselineRoot',str(baseline),'-CommitOnly','-PatternShakeBaselinePath',str(sidecar_baseline),'-PatternShakeCandidatePath',str(sidecar_candidate)],cwd=self.root,capture_output=True,text=True,encoding='utf-8',errors='replace',env=self.environment)
  self.assertEqual(0,completed.returncode,completed.stdout+completed.stderr)
  self.assertTrue(json.loads(result_path.read_text(encoding='utf-8-sig'))['canonicalCommitted'])
  self.assertEqual(target.read_bytes(),sidecar_candidate.read_bytes())
  self.assertNotEqual(self.repository_revision,self.source_manifest()['sourceRevision'])
  # The same owner pair also flows through the joined Product writer.
  write_patch();sidecar_baseline.write_bytes(target.read_bytes());sidecar_candidate.write_text(json.dumps(original),encoding='utf-8')
  typed_before=self.data_manifest()
  with self.assertRaisesRegex(Exception,'injected'):
   m.commit_typed_authoring_patch(self.root,patch_path,pattern_shake_baseline_path=sidecar_baseline,pattern_shake_candidate_path=sidecar_candidate,inject_failure_after=1)
  self.assertEqual(typed_before,self.data_manifest())
  result=m.commit_typed_authoring_patch(self.root,patch_path,pattern_shake_baseline_path=sidecar_baseline,pattern_shake_candidate_path=sidecar_candidate)
  self.assertEqual('ApplyTypedPatch',result['mode']);self.assertEqual(target.read_bytes(),sidecar_candidate.read_bytes())
  bp.build_repository_product_projection(self.root)
  empty=copy.deepcopy(original);empty['cues']=[]
  m._validate_pattern_shake_source(json.dumps(empty).encode('utf-8'))

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
