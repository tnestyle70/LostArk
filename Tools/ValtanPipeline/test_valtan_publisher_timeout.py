"""Publisher timeout diagnostics retain progress without leaving a candidate."""
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import valtan_tuning_pipeline as pipeline

ROOT = Path(__file__).resolve().parents[2]


class PublisherTimeoutTests(unittest.TestCase):
    def test_timeout_retains_bounded_utf8_output_for_each_phase(self):
        for overlay, phase in ((None, 'baseline'), (Path('overlay'), 'candidate')):
            with self.subTest(phase=phase), tempfile.TemporaryDirectory() as temporary:
                error = subprocess.TimeoutExpired(
                    ['publisher'], 180,
                    output=b'discarded-prefix:' + b'x' * 3000 + b'\nlast stdout\xff',
                    stderr='discarded-prefix:' + 'y' * 3000 + '\nlast stderr',
                )
                with mock.patch.object(pipeline, '_powershell_executable', return_value='powershell.exe'), \
                     mock.patch.object(pipeline.subprocess, 'run', side_effect=error) as run:
                    with self.assertRaises(pipeline.PipelineError) as caught:
                        pipeline._publish_gameplay_bootstrap(
                            ROOT, Path(temporary), input_overlay_root=overlay,
                            external_writer_identity=(123, 'private-writer-nonce'),
                        )
                message = str(caught.exception)
                self.assertIn(f'gameplay publisher {phase} timed out', message)
                self.assertIn('(limit 180s)', message)
                self.assertIn('stdout tail:', message)
                self.assertIn('last stdout\ufffd', message)
                self.assertIn('stderr tail:', message)
                self.assertIn('last stderr', message)
                self.assertNotIn('discarded-prefix:', message)
                self.assertNotIn('private-writer-nonce', message)
                self.assertLess(len(message), 4200)
                self.assertIs(caught.exception.__cause__, error)
                self.assertEqual(pipeline.GAMEPLAY_PUBLISH_TIMEOUT_SECONDS, run.call_args.kwargs['timeout'])

    def test_timeout_without_output_still_reports_phase_and_bound(self):
        with tempfile.TemporaryDirectory() as temporary, \
             mock.patch.object(pipeline, '_powershell_executable', return_value='powershell.exe'), \
             mock.patch.object(pipeline.subprocess, 'run', side_effect=subprocess.TimeoutExpired(['publisher'], 180)):
            with self.assertRaisesRegex(pipeline.PipelineError, r'baseline timed out.*limit 180s'):
                pipeline._publish_gameplay_bootstrap(
                    ROOT, Path(temporary), input_overlay_root=None,
                    external_writer_identity=(123, 'private-writer-nonce'),
                )

    def test_timed_out_publish_rolls_back_staged_and_partial_baseline_files(self):
        publish = pipeline._publish_gameplay_bootstrap
        observed = []
        with tempfile.TemporaryDirectory() as temporary:
            candidate_root = Path(temporary) / 'candidates'

            def timed_out_bootstrap(root, output_root, **kwargs):
                self.assertTrue(list(candidate_root.glob('.stage.*')))
                output_root.mkdir(parents=True)
                (output_root / 'Gameplay.bootstrap').write_text('partial output', encoding='utf-8')
                observed.append(output_root)
                with mock.patch.object(pipeline, '_powershell_executable', return_value='powershell.exe'), \
                     mock.patch.object(pipeline.subprocess, 'run', side_effect=subprocess.TimeoutExpired(
                         ['publisher'], 180, output=b'validation completed; generation started',
                         stderr=b'last diagnostic',
                     )):
                    return publish(root, output_root, **kwargs)

            with mock.patch.object(pipeline, '_publish_gameplay_bootstrap', side_effect=timed_out_bootstrap):
                with self.assertRaisesRegex(pipeline.PipelineError, 'generation started'):
                    pipeline.publish_candidate(ROOT, candidate_root)
            self.assertEqual(1, len(observed))
            self.assertFalse(observed[0].exists())
            self.assertFalse((candidate_root / 'current-candidate.json').exists())
            self.assertFalse(list(candidate_root.glob('.stage.*')))
            self.assertFalse(list(candidate_root.glob('.baseline.*')))


if __name__ == '__main__':
    unittest.main()
