import copy, math, unittest
from pathlib import Path
from Tools.KoukuSaydonPipeline.combat_hit_templates import validate_hits,validate_logic_hits
from Tools.KoukuSaydonPipeline import project_kouku_saydon_composition as p
class AttackTemplateTests(unittest.TestCase):
 def test_optional_flight_preserves_legacy_and_bounds_pair(self):
  self.assertNotIn('riseHeightM',validate_hits([{'hitId':'legacy'}])[0])
  row={'hitId':'rise','riseHeightM':3,'pushMs':1200}
  self.assertEqual(validate_hits([row])[0]['pushMs'],1200)
  for fields in ({'riseHeightM':3},{'pushMs':1200},{'riseHeightM':101,'pushMs':1200},{'riseHeightM':3,'pushMs':99},{'riseHeightM':True,'pushMs':1200}):
   with self.subTest(fields=fields),self.assertRaises(ValueError):validate_hits([{'hitId':'bad',**fields}])
 def test_explicit_push_policy_preserves_horizontal_and_legacy_defaults(self):
  legacy=validate_hits([dict(hitId='legacy',riseHeightM=2,pushMs=250)])[0]
  self.assertNotIn('forcePush',legacy);self.assertNotIn('pushDirection',legacy)
  for rise in (0,2):
   hit=validate_hits([dict(hitId='original.result',riseHeightM=rise,pushRangeM=1,pushMs=250,forcePush=False,pushDirection='AWAY_FROM_BOSS')])[0]
   self.assertIs(hit['forcePush'],False);self.assertEqual(hit['pushDirection'],'AWAY_FROM_BOSS');self.assertEqual(hit['pushRangeM'],1)
  for fields in ({'pushRangeM':1},{'pushRangeM':1,'pushMs':99},{'forcePush':0},{'forcePush':'false'},{'pushDirection':'BOSS_FORWARD'}):
   with self.subTest(fields=fields),self.assertRaises(ValueError):validate_hits([dict(hitId='bad',**fields)])
 def test_defaults_and_no_input_mutation(self):
  row={'hitId':'attack.1'}; before=copy.deepcopy(row); self.assertEqual(validate_hits([row])[0]['damagePercent'],10);self.assertEqual(row,before)
 def test_primitive_and_damage_policies(self):
  for fields in ({},{'shape':'RING','radiusM':4,'innerRadiusM':2},{'shape':'BOX','lengthM':3,'halfWidthM':1},{'shape':'CONE','lengthM':3,'angleDegrees':90},{'damageKind':'INSTANT_DEATH','damagePercent':0},{'damageKind':'PROFILE','damagePercent':0,'damageProfileId':'source.damage'}):
   validate_hits([{'hitId':'attack.1',**fields}])
 def test_bad_values_are_rejected(self):
  for fields in ({'radiusM':math.nan},{'radiusM':0},{'atMs':True},{'atMs':601000},{'repeatCount':0},{'repeatCount':2,'repeatIntervalMs':1},{'shape':'RING','innerRadiusM':2},{'trigger':'CONTACT','endMs':0},{'damageKind':'PROFILE'},{'damageKind':'INSTANT_DEATH'},{'unknown':1}):
   with self.subTest(fields=fields),self.assertRaises(ValueError):validate_hits([{'hitId':'a',**fields}])
 def test_owner_lifetime_rejects_repeat_tail(self):
  with self.assertRaises(ValueError):validate_hits([{'hitId':'a','atMs':100,'repeatCount':3,'repeatIntervalMs':100}],299)
 def test_duplicate_and_count(self):
  for rows in ([{'hitId':'a'}]*2,[{'hitId':str(i)} for i in range(33)]):
   with self.assertRaises(ValueError):validate_hits(rows)
 def test_wrong_logic_owner_and_random_join(self):
  for logic in ({'logicType':'TRIGGER','projectileHits':[]},{'logicType':'DURATION','judgementKind':'PURSUIT_PROJECTILES','fixedHits':[]},{'logicType':'DURATION','judgementKind':'SHOWTIME_PLAYER_TARGETS','randomVolleyHits':[[]]}):
   with self.assertRaises(ValueError):validate_logic_hits(logic)
 def test_albion_hits_use_existing_circle_lifetime(self):
  logic=dict(logicType='TRIGGER',triggerKind='ALBION_BLUE_CIRCLE',effectLifetimeMs=5680,fixedHits=[dict(hitId='blue.impact',atMs=2000,radiusM=1.6)])
  validate_logic_hits(logic)
  logic['effectLifetimeMs']=1999
  with self.assertRaises(ValueError):validate_logic_hits(logic)
 def test_showtime_projection_joins_same_instance_lifetime(self):
  resources=[dict(resourceId='effect.1',kind='EFFECT',assetId='effect.test',resourceKind='V1_EFFECT',durationMs=1000)]
  logic=dict(logicId='logic.1',logicType='DURATION',judgementKind='SHOWTIME_PLAYER_TARGETS',trackingPresentationOccurrenceId='p.effect.1',trackingHits=[dict(hitId='death',trigger='CONTACT',endMs=2000,damageKind='INSTANT_DEATH',damagePercent=0)])
  pattern=dict(patternId='p',actorProfileId='MN_RPCT_07',targetBossPlacementId='boss.kakulsaydon.g3.saydon',presentationOccurrences=[dict(occurrenceId='p.effect.1',resourceId='effect.1',anchorKind='MAP',followBoss=False,startMs=0,durationMs=1000)],logicOccurrences=[dict(occurrenceId='p.logic.1',logicId='logic.1',startMs=0,durationMs=2000)])
  doc=dict(logics=[logic],presentationResources=resources);rows,templates,_=p._project_showtime_targets(doc,pattern);self.assertEqual(rows[0]['trackingHits'][0]['endMs'],2000);self.assertEqual(len(templates),1)
  logic['trackingHits'][0]['endMs']=2001
  with self.assertRaises(p.CompositionError):p._project_showtime_targets(doc,pattern)
class HollowColliderTests(unittest.TestCase):
 def fixture(self):
  resource=dict(resourceId='shape.1',displayName='Hollow',kind='COLLIDER',assetId='',radiusM=3,innerRadiusM=1,shape='CIRCLE',durationMs=1000)
  box=dict(occurrenceId='p.collider.1',resourceId='shape.1',logicOccurrenceId='p.logic.1',startMs=0,durationMs=1000,scale=[2,1,2])
  return dict(presentationResources=[resource]),dict(presentationOccurrences=[box]),dict(occurrenceId='p.logic.1',startMs=0,durationMs=1000),dict(logicType='TRIGGER',triggerKind='ENTER_AREA')
 def test_ring_and_hollow_sector_preserve_scaled_hole(self):
  for shape in ('CIRCLE','SECTOR'):
   doc,pattern,box,logic=self.fixture();doc['presentationResources'][0]['shape']=shape
   p._validate_presentation_resources(doc)
   result=p._project_collider_regions(doc,pattern,box,logic,{},Path('.'))[0]
   self.assertEqual((result['innerRadiusM'],result['radiusM']),(2,6))
 def test_invalid_inner_shape_and_radius_rejected(self):
  for fields in ({'innerRadiusM':-1},{'innerRadiusM':3},{'innerRadiusM':math.nan},{'shape':'BOX'},{'shape':'REVERSE_SECTOR'},{'kind':'EFFECT','assetId':'effect.test'}):
   doc,*_=self.fixture();doc['presentationResources'][0].update(fields)
   with self.subTest(fields=fields),self.assertRaises(p.CompositionError):p._validate_presentation_resources(doc)
 def test_nonuniform_hole_rejected(self):
  doc,pattern,box,logic=self.fixture();pattern['presentationOccurrences'][0]['scale']=[1,1,2]
  with self.assertRaises(p.CompositionError):p._project_collider_regions(doc,pattern,box,logic,{},Path('.'))
 def test_legacy_filled_resource_output_unchanged(self):
  doc,*_=self.fixture();r=doc['presentationResources'][0];r.pop('innerRadiusM')
  self.assertNotIn('innerRadiusM',p._project_presentation_resource(r))
if __name__=='__main__':unittest.main()
