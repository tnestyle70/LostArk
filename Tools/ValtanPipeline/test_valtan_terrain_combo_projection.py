"""Projection boundaries needed by terrain destruction's authored suffix."""
import copy
import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import valtan_tuning_pipeline as pipeline

ROOT = Path(__file__).resolve().parents[2]


class TerrainComboProjectionTests(unittest.TestCase):
    def test_publish_saved_source_without_patch_keeps_new_stage_and_receipt(self):
        docs = pipeline.load_pipeline_documents(ROOT)
        pattern = next(row for row in docs[pipeline.GAMEPLAY_AUTHORING_REL]['patterns']
                       if row['patternId'] == 'VALTAN_FLOOR_WIPE_130')
        stage = next(row for row in pattern['stages'] if row['stageId'] == 'INTERVAL')
        stage['durationMs'] += 37
        expected_duration = stage['durationMs']
        inspected = []
        with tempfile.TemporaryDirectory() as temporary:
            candidate_root = Path(temporary) / 'candidates'

            def inspect_before_bootstrap(*args, **kwargs):
                staged = next(candidate_root.glob('.stage.*'))
                encounter = pipeline.read_json(staged / pipeline.ENCOUNTER_REL)
                actual = next(row for row in encounter['patterns']
                              if row['patternId'] == pattern['patternId'])
                interval = next(row for row in actual['stages'] if row['stageId'] == 'INTERVAL')
                self.assertEqual(expected_duration, interval['durationMs'])
                receipt = pipeline.read_json(staged / pipeline.PROVENANCE_REL)
                entry = next(row for row in receipt['entries']
                             if row['targetId'] == 'pattern:' + pattern['patternId']
                             and row['targetField'].endswith('.stages'))
                self.assertEqual(actual['stages'], entry['resultValue'])
                self.assertEqual('PROJECT_TUNED', entry['basis'])
                inspected.append(True)
                raise pipeline.PipelineError('verified staged saved-source boundary')

            with mock.patch.object(pipeline, 'load_pipeline_documents', return_value=docs), \
                 mock.patch.object(pipeline, '_publish_gameplay_bootstrap', side_effect=inspect_before_bootstrap):
                with self.assertRaisesRegex(pipeline.PipelineError, 'verified staged saved-source boundary'):
                    pipeline.publish_candidate(ROOT, candidate_root)
            self.assertEqual([True], inspected)
            self.assertFalse((candidate_root / 'current-candidate.json').exists())
            self.assertFalse(list(candidate_root.glob('.stage.*')))

    def test_combat_object_add_and_remove_refresh_receipt_coverage(self):
        original = pipeline.read_json(ROOT / pipeline.COMBAT_PRODUCT_REL)
        for delta in (-1, 1):
            with self.subTest(delta=delta):
                product = copy.deepcopy(original)
                if delta < 0:
                    product['objects'].pop()
                else:
                    row = copy.deepcopy(product['objects'][0])
                    row['combatObjectArchetypeId'] = 'combatobject.valtan.test-terrain-copy'
                    product['objects'].append(row)
                receipt = json.loads(pipeline.project_provenance_receipt(
                    ROOT, {pipeline.COMBAT_PRODUCT_REL: json.dumps(product)}))
                self.assertEqual(receipt['coverage']['bossCombatObjectCount'], len(product['objects']))
                self.assertEqual(receipt['coverage']['fieldEntryCount'], len(receipt['entries']))
                entries = [e for e in receipt['entries'] if e['targetDocument'] == pipeline.COMBAT_PRODUCT_REL
                           and e['targetId'].startswith('combat-object:')]
                self.assertEqual({e['targetId'][len('combat-object:'):] for e in entries},
                                 {o['combatObjectArchetypeId'] for o in product['objects']})
                for e in entries:
                    if e['targetId'] == 'combat-object:combatobject.valtan.test-terrain-copy':
                        self.assertEqual(e['basis'], 'PROJECT_TUNED')

    def test_stage_aim_survives_split_join_and_product_projection(self):
        docs = pipeline.load_pipeline_documents(ROOT)
        g = copy.deepcopy(docs[pipeline.GAMEPLAY_AUTHORING_REL])
        target = next(p for p in g['patterns'] if p['patternId'] == 'VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK')
        stage = target['stages'][-1]
        stage['aim'] = {'targetPolicy': 'NEAREST_EACH_TICK'}
        stage['motion'] = {'kind': 'TO_ARENA_CENTER'}
        joined = pipeline.join_v2_authoring(g, docs[pipeline.PRESENTATION_AUTHORING_REL],
            docs[pipeline.WORLD_SET_REL], docs[pipeline.COMBAT_AUTHORING_REL])
        restored, _ = pipeline.split_v2_authoring(joined, docs[pipeline.WORLD_SET_REL], docs[pipeline.COMBAT_AUTHORING_REL])
        actual = next(p for p in restored['patterns'] if p['patternId'] == target['patternId'])['stages'][-1]
        self.assertEqual(actual, stage)
        malformed = copy.deepcopy(stage)
        malformed['aim']['targetPolicy'] = 'NONE'
        with self.assertRaises(pipeline.PipelineError):
            pipeline._validate_pattern_stage_extensions(malformed, 'invalid aim')
        malformed = copy.deepcopy(stage)
        malformed['motion']['distance'] = 6
        with self.assertRaises(pipeline.PipelineError):
            pipeline._validate_pattern_stage_extensions(malformed, 'invalid center motion')


if __name__ == '__main__':
    unittest.main()
