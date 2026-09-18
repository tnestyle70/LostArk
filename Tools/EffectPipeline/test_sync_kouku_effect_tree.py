"""Keep library registration additive and reject stale editor/source inputs."""
import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import sync_kouku_effect_tree as sync


class LibraryRegistrationTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.root_patch = patch.object(sync, 'ROOT', self.root)
        self.root_patch.start()
        self.asset = 'effect.kouku.source.test'
        self.path = self.root / 'Data/Effects/Authored' / (self.asset + '.effect.json')
        self.path.parent.mkdir(parents=True)
        self.path.write_text(json.dumps(dict(schema='lostark.effect-authoring', version=13,
                                            effectAssetId=self.asset, elements=[dict(id='test')])))
        self.refs = [dict(kind='V1', assetId=self.asset)]
        self.empty = b'{\n  "formatVersion": 1,\n  "effects": []\n}\n'

    def tearDown(self):
        self.root_patch.stop()
        self.temp.cleanup()

    def test_add_and_repeat_preserve_exact_catalog_bytes(self):
        candidate, rows, inputs = sync.stage_catalog(self.empty, self.refs)
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]['authoringPath'], 'Effects/Authored/' + self.path.name)
        self.assertEqual(inputs[self.path], hashlib.sha256(self.path.read_bytes()).hexdigest())
        repeated, additions, _ = sync.stage_catalog(candidate, self.refs)
        self.assertEqual(repeated, candidate)
        self.assertEqual(additions, [])

    def test_conflicting_registration_is_not_overwritten(self):
        candidate, _, _ = sync.stage_catalog(self.empty, self.refs)
        bad = json.loads(candidate)
        bad['effects'][0]['authoringPath'] = 'Effects/Authored/wrong.effect.json'
        with self.assertRaisesRegex(AssertionError, 'conflict'):
            sync.stage_catalog(json.dumps(bad).encode(), self.refs)

    def test_mismatched_authored_identity_is_rejected(self):
        data = json.loads(self.path.read_bytes())
        data['effectAssetId'] = 'effect.kouku.wrong'
        self.path.write_text(json.dumps(data))
        with self.assertRaisesRegex(AssertionError, 'identity mismatch'):
            sync.stage_catalog(self.empty, self.refs)

    def test_commit_rejects_changed_payload_without_writing_catalog(self):
        candidate, _, inputs = sync.stage_catalog(self.empty, self.refs)
        target = self.root / 'catalog.json'
        target.write_bytes(self.empty)
        self.path.write_bytes(self.path.read_bytes() + b' ')
        with self.assertRaisesRegex(RuntimeError, 'Authored input changed'):
            sync.commit_library([(target, self.empty, candidate)], inputs)
        self.assertEqual(target.read_bytes(), self.empty)

    def test_commit_rejects_editor_save(self):
        target = self.root / 'catalog.json'
        target.write_bytes(b'editor save')
        with self.assertRaisesRegex(RuntimeError, 'Library changed'):
            sync.commit_library([(target, self.empty, b'candidate')], {})
        self.assertEqual(target.read_bytes(), b'editor save')

    def test_second_write_failure_rolls_back_catalog(self):
        catalog, tree = self.root / 'catalog.json', self.root / 'tree.json'
        catalog.write_bytes(b'catalog before')
        tree.write_bytes(b'tree before')
        original_replace = Path.replace

        def fail_tree(path, destination):
            if destination == tree:
                raise OSError('simulated tree write failure')
            return original_replace(path, destination)

        with patch.object(Path, 'replace', fail_tree), self.assertRaises(OSError):
            sync.commit_library([(catalog, b'catalog before', b'catalog after'),
                                 (tree, b'tree before', b'tree after')], {})
        self.assertEqual(catalog.read_bytes(), b'catalog before')
        self.assertEqual(tree.read_bytes(), b'tree before')
        self.assertEqual(list(self.root.glob('*.tmp')), [])

    def test_success_keeps_catalog_tree_order(self):
        catalog, tree = self.root / 'catalog.json', self.root / 'tree.json'
        catalog.write_bytes(b'a')
        tree.write_bytes(b'b')
        sync.commit_library([(catalog, b'a', b'new a'), (tree, b'b', b'new b')], {})
        self.assertEqual((catalog.read_bytes(), tree.read_bytes()), (b'new a', b'new b'))


if __name__ == '__main__':
    unittest.main()
