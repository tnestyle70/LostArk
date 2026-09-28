from __future__ import annotations
import copy
import json
from pathlib import Path
import unittest
import valtan_tuning_pipeline as pipeline

ROOT = Path(__file__).resolve().parents[2]
ROCK = 'combatobject.valtan.six-pizza.rock-pillar'

class OwnerHitChainContractTests(unittest.TestCase):
    def setUp(self):
        self.docs = pipeline.load_pipeline_documents(ROOT)
        self.combat = copy.deepcopy(self.docs[pipeline.COMBAT_AUTHORING_REL])
        self.rock = next(row for row in self.combat['objects'] if row['combatObjectArchetypeId'] == ROCK)
        self.rock['ownerHitChain'] = {'triggerActionId':'valtan.sequence.center-six-pizza-charge.step-11', 'delayMs':1500, 'armedPresentationEventId':'event.valtan.six-pizza.rock-pillar.armed'}
        self.rock['lifetimeMs'] = 32000
        self.rock['hits'][0]['trigger']['atMs'] = 0

    def test_real_document_projection_preserves_policy_and_other_archetypes(self):
        pipeline.validate_combat_authoring(self.combat)
        args = (self.docs[pipeline.GAMEPLAY_AUTHORING_REL], self.combat, self.docs[pipeline.LEGACY_REL], self.docs[pipeline.BOSS_CATALOG_REL])
        products = pipeline._compile_combat_products(*args)
        baseline = pipeline._compile_combat_products(args[0], self.docs[pipeline.COMBAT_AUTHORING_REL], *args[2:])
        current = next(row for row in products if row['combatObjectArchetypeId'] == ROCK)
        self.assertEqual(self.rock['ownerHitChain'], current['ownerHitChain'])
        self.assertEqual(32000,current['lifeMs'])
        self.assertEqual(0,current['hits'][0]['atMs'])
        self.assertEqual([r for r in baseline if r['combatObjectArchetypeId']!=ROCK], [r for r in products if r['combatObjectArchetypeId']!=ROCK])

    def test_invalid_policy_is_rejected(self):
        for field,value in [('delayMs',0),('delayMs',32000),('delayMs',True),('triggerActionId','../bad'),('armedPresentationEventId',self.rock['hits'][0]['hitId'])]:
            with self.subTest(field=field,value=value):
                doc=copy.deepcopy(self.combat);rock=next(r for r in doc['objects'] if r['combatObjectArchetypeId']==ROCK);rock['ownerHitChain'][field]=value
                with self.assertRaises(pipeline.PipelineError):pipeline.validate_combat_authoring(doc)
        self.rock['hits'][0]['repeat']['count']=2
        with self.assertRaises(pipeline.PipelineError):pipeline.validate_combat_authoring(self.combat)

    def test_trigger_must_resolve_owner_cone_action(self):
        for action in ['valtan.sequence.center-six-pizza-charge.step-07','valtan.missing-action']:
            self.rock['ownerHitChain']['triggerActionId']=action
            with self.assertRaises(pipeline.PipelineError):pipeline._compile_combat_products(self.docs[pipeline.GAMEPLAY_AUTHORING_REL],self.combat,self.docs[pipeline.LEGACY_REL],self.docs[pipeline.BOSS_CATALOG_REL])

    def test_armed_visual_pair_and_v2_exclusion(self):
        visual={'combatObjectArchetypeId':ROCK,'clientVisualId':'visual.rock','effectAssetId':'effect.active','hitEffectAssetId':'effect.hit','armedPresentationEventId':'event.armed','armedEffectAssetId':'effect.armed','stopActiveOnHit':True}
        pipeline._validate_combat_object_visual_row(visual,'test',None)
        for broken in [dict(visual,stopActiveOnHit=1),dict(visual,effectV2Group={}),{k:v for k,v in visual.items() if k!='armedEffectAssetId'}]:
            with self.assertRaises(pipeline.PipelineError):pipeline._validate_combat_object_visual_row(broken,'test',None)

if __name__ == '__main__':unittest.main()
