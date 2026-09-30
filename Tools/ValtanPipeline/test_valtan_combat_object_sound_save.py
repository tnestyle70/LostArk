"""Exercise the Object Sound owner through the canonical CAS/rollback writer."""
import json
import subprocess
import unittest

import promote_valtan_animation_chains as promoter
import test_valtan_source_save as source_tests


class CombatObjectSoundSaveTests(unittest.TestCase):
    setUp = source_tests.TestSourceSave.setUp
    tearDown = source_tests.TestSourceSave.tearDown
    baseline = source_tests.TestSourceSave.baseline
    data_manifest = source_tests.TestSourceSave.data_manifest
    source_manifest = source_tests.TestSourceSave.source_manifest
    run_pipeline = source_tests.TestSourceSave.run_pipeline
    parse_command_result = staticmethod(source_tests.TestSourceSave.parse_command_result)

    def test_combat_object_sound_transaction(self):
        baseline_root = self.baseline()
        source = self.root / promoter.COMBAT_OBJECT_SOUND_REL
        baseline = self.root / 'object-sound-before.json'
        baseline.write_bytes(source.read_bytes())
        document = json.loads(baseline.read_text(encoding='utf-8'))
        document['cues'][0]['playbackOffsetMs'] = 125
        candidate = self.root / 'object-sound-after.json'
        candidate.write_text(json.dumps(document), encoding='utf-8')
        patch_path = self.root / 'object-sound-patch.json'
        patch_path.write_text(json.dumps({
            'schema': 'lostark.valtan-tuning-draft-patch', 'formatVersion': 1,
            'sourceRevision': self.repository_revision, 'operations': [],
        }), encoding='utf-8')
        kwargs = dict(source_baseline_root=baseline_root,
                      combat_object_sound_baseline_path=baseline,
                      combat_object_sound_candidate_path=candidate)
        before = self.data_manifest()
        with self.assertRaisesRegex(Exception, 'injected'):
            promoter.commit_source_authoring_patch(self.root, patch_path,
                                                   inject_failure_after=1, **kwargs)
        self.assertEqual(before, self.data_manifest())
        source.write_bytes(source.read_bytes() + b'\n')
        with self.assertRaisesRegex(Exception, 'changed since Reload'):
            promoter.commit_source_authoring_patch(self.root, patch_path, **kwargs)
        self.assertEqual(source.read_bytes(), baseline.read_bytes() + b'\n')
        source.write_bytes(baseline.read_bytes())
        # The async PowerShell entry must forward both owner snapshots intact.
        receipt = self.root / 'object-sound-save-result.json'
        completed = subprocess.run([
            'powershell', '-ExecutionPolicy', 'Bypass', '-File',
            str(self.root / 'Tools/ValtanPipeline/Run-ValtanAuthoringSaveJob.ps1'),
            '-ExpectedSourceRevision', self.repository_revision,
            '-ResultPath', str(receipt), '-DraftPatchPath', str(patch_path),
            '-SourceOnly', '-SourceBaselineRoot', str(baseline_root), '-CommitOnly',
            '-CombatObjectSoundBaselinePath', str(baseline),
            '-CombatObjectSoundCandidatePath', str(candidate),
        ], cwd=self.root, capture_output=True, text=True, encoding='utf-8',
            errors='replace', env=self.environment)
        self.assertEqual(0, completed.returncode, completed.stdout + completed.stderr)
        self.assertTrue(json.loads(receipt.read_text(encoding='utf-8-sig'))['ok'])
        self.assertEqual(source.read_bytes(), candidate.read_bytes())
        after = self.data_manifest()
        self.assertEqual({promoter.COMBAT_OBJECT_SOUND_REL.removeprefix('Data/')},
                         {key for key in before if before[key] != after[key]})

    def test_combat_object_sound_storage_rejects_bad_identity_and_clock(self):
        path = self.root / promoter.COMBAT_OBJECT_SOUND_REL
        source = json.loads(path.read_text(encoding='utf-8'))
        promoter._validate_source_sidecar(promoter.COMBAT_OBJECT_SOUND_REL, path.read_bytes())
        for change in ({'playbackOffsetMs': -1}, {'playbackOffsetMs': True},
                       {'playbackOffsetMs': 600001}, {'soundBank': 'WrongBank'},
                       {'presentationEventId': 'duplicate-source'}):
            candidate = json.loads(json.dumps(source))
            candidate['cues'][0].update(change)
            with self.assertRaises(Exception):
                promoter._validate_source_sidecar(promoter.COMBAT_OBJECT_SOUND_REL,
                                                 json.dumps(candidate).encode())


if __name__ == '__main__':
    suite = unittest.TestSuite(CombatObjectSoundSaveTests(name) for name in (
        'test_combat_object_sound_transaction',
        'test_combat_object_sound_storage_rejects_bad_identity_and_clock',
    ))
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    raise SystemExit(not result.wasSuccessful())
