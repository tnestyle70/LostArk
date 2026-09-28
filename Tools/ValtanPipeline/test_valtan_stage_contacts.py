"""Stage-owned contact overrides retain one authoritative Valtan pulse clock."""
from __future__ import annotations
import copy
from pathlib import Path
import unittest
import valtan_tuning_pipeline as pipeline

ROOT = Path(__file__).resolve().parents[2]

class ValtanStageContactsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.docs = pipeline.load_pipeline_documents(ROOT)

    def join(self, gameplay=None):
        return pipeline.join_v2_authoring(gameplay or self.docs[pipeline.GAMEPLAY_AUTHORING_REL],
            self.docs[pipeline.PRESENTATION_AUTHORING_REL], self.docs[pipeline.WORLD_SET_REL],
            self.docs[pipeline.COMBAT_AUTHORING_REL])

    @staticmethod
    def spin(gameplay):
        return next(stage for pattern in gameplay['patterns'] if pattern['patternId'] == 'VALTAN_FOUR_SLASH'
                    for stage in pattern['stages'] if stage['stageId'] == 'SPIN')

    def test_actual_source_projection_and_product_import_preserve_contacts(self):
        master = self.join()
        owner = next(p for p in master['patterns'] if p['patternId'] == 'VALTAN_FOUR_SLASH')
        product = pipeline.compile_pattern_product(master, owner)
        stage = next(s for s in product['stages'] if s['stageId'] == 'SPIN')
        source = self.spin(master)['hit']
        self.assertEqual(source['contacts'], stage['attackContacts'])
        self.assertEqual(source['contacts'], pipeline._migrate_hit(stage)['contacts'])
        self.assertEqual([600,1640,3000], stage['hitOffsetsMs'])
        self.assertEqual('RING', stage['attackContacts'][-1]['shape'])
        self.assertEqual(4.8, stage['attackContacts'][-1]['innerRadiusM'])

    def test_legacy_projection_does_not_invent_contacts(self):
        gameplay = copy.deepcopy(self.docs[pipeline.GAMEPLAY_AUTHORING_REL])
        stage = self.spin(gameplay);del stage['hit']['contacts']
        self.join(gameplay)
        self.assertNotIn('attackContacts', pipeline._compile_hit(stage['hit']))

    def test_shape_time_damage_push_and_unknown_fields_fail_closed(self):
        cases = [('atMs',599),('trigger','CONTACT'),('repeatCount',2),('endMs',601),
                 ('repeatIntervalMs',34),('shape','NONE'),('damagePercent',101),
                 ('forcePush',1),('pushMs',1),('newField',1)]
        for field,value in cases:
            with self.subTest(field=field):
                gameplay=copy.deepcopy(self.docs[pipeline.GAMEPLAY_AUTHORING_REL])
                self.spin(gameplay)['hit']['contacts'][0][field]=value
                with self.assertRaises(pipeline.PipelineError): self.join(gameplay)

    def test_contacts_cannot_be_empty_missing_reordered_or_escape_stage(self):
        for change in ('empty','missing','reverse','duration'):
            with self.subTest(change=change):
                gameplay=copy.deepcopy(self.docs[pipeline.GAMEPLAY_AUTHORING_REL]);stage=self.spin(gameplay)
                if change=='empty':stage['hit']['contacts']=[]
                elif change=='missing':stage['hit']['contacts'].pop()
                elif change=='reverse':stage['hit']['contacts'].reverse()
                else:stage['durationMs']=3000
                with self.assertRaises(pipeline.PipelineError):self.join(gameplay)

    def test_active_window_and_stage_motion_cannot_bypass_pulse_contract(self):
        for change in ('activation','motion'):
            with self.subTest(change=change):
                gameplay=copy.deepcopy(self.docs[pipeline.GAMEPLAY_AUTHORING_REL]);stage=self.spin(gameplay)
                if change=='activation':
                    del stage['hit']['schedule'];stage['hit']['activation']={'kind':'ACTIVE_WINDOW','startMs':0,'lifetimeMs':100,'perTargetPolicy':'ONCE'}
                else:stage['motion']={'kind':'FORWARD','distanceM':1}
                with self.assertRaises(pipeline.PipelineError):self.join(gameplay)

    def test_landing_only_owns_fifty_percent_and_axe_profile_is_preserved(self):
        master=self.join();jump=next(p for p in master['patterns'] if p['patternId']=='VALTAN_HIGH_JUMP')
        land=next(s for s in jump['stages'] if s['stageId']=='LAND')
        self.assertEqual([201],land['hit']['schedule']['offsetsMs'])
        self.assertEqual(('CIRCLE',8.75,50),tuple(land['hit']['contacts'][0][k] for k in ('shape','radiusM','damagePercent')))
        profile=next(p for p in self.docs[pipeline.DAMAGE_REL]['profiles'] if p['damageProfileId']=='damage.valtan.high-jump')
        self.assertEqual(500,profile['damageRatePercent'])

if __name__=='__main__':unittest.main()
