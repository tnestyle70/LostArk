"""Focused transaction checks for transient Windows publication locks."""

import contextlib
import io
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from Tools.KoukuSaydonPipeline import project_kouku_saydon_composition as subject


def windows_error(code):
    error = PermissionError(f"simulated Windows error {code}")
    error.winerror = code
    return error


class KoukuPublishSharingRetryTests(unittest.TestCase):
    def test_transient_sharing_and_lock_violations_publish(self):
        for code in (32, 33):
            with self.subTest(code=code), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                destination = root / "product.json"
                destination.write_bytes(b'{"revision":1}')
                replace = subject.os.replace
                attempts = 0

                def transient(source, target):
                    nonlocal attempts
                    attempts += 1
                    if attempts < 3:
                        raise windows_error(code)
                    return replace(source, target)

                with mock.patch.object(subject.os, "replace", side_effect=transient), \
                        mock.patch.object(subject.time, "sleep") as sleep:
                    subject.publish_outputs(root, {Path("product.json"): b'{"revision":2}'})
                self.assertEqual(3, attempts)
                self.assertEqual(2, sleep.call_count)
                self.assertEqual(b'{"revision":2}', destination.read_bytes())
                self.assertEqual([destination], list(root.iterdir()))

    def test_access_denied_is_not_retried(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            failure = windows_error(5)
            with mock.patch.object(subject.os, "replace", side_effect=failure) as replace, \
                    mock.patch.object(subject.time, "sleep") as sleep:
                with self.assertRaises(PermissionError) as raised:
                    subject.publish_outputs(root, {Path("product.json"): b'{}'})
            self.assertIs(failure, raised.exception)
            self.assertEqual(1, replace.call_count)
            sleep.assert_not_called()
            self.assertEqual([], list(root.iterdir()))

    def test_permanent_lock_is_bounded_and_cleanup_preserves_primary_error(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            destination = root / "product.json"
            destination.write_bytes(b'{"revision":1}')
            failure = windows_error(32)
            unlink = Path.unlink
            warnings = io.StringIO()

            def locked_staging(path, *args, **kwargs):
                if ".staging." in path.name:
                    raise windows_error(33)
                return unlink(path, *args, **kwargs)

            with mock.patch.object(subject.os, "replace", side_effect=failure) as replace, \
                    mock.patch.object(Path, "unlink", locked_staging), \
                    mock.patch.object(subject.time, "sleep") as sleep, \
                    contextlib.redirect_stderr(warnings):
                with self.assertRaises(PermissionError) as raised:
                    subject.publish_outputs(root, {Path("product.json"): b'{"revision":2}'})
            self.assertIs(failure, raised.exception)
            self.assertEqual(21, replace.call_count)
            self.assertEqual(40, sleep.call_count)
            self.assertEqual(b'{"revision":1}', destination.read_bytes())
            self.assertIn("cleanup retained", warnings.getvalue())
            self.assertEqual(1, len(list(root.glob("*.staging.*"))))
            self.assertFalse(list(root.glob("*.rollback.*")))

    def test_rollback_retries_transient_lock_and_restores_baseline(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            destination = root / "product.json"
            destination.write_bytes(b'{"revision":1}')
            replace = subject.os.replace
            rollback_attempts = 0

            def transient(source, target):
                nonlocal rollback_attempts
                if ".rollback." in Path(source).name:
                    rollback_attempts += 1
                    if rollback_attempts == 1:
                        raise windows_error(32)
                return replace(source, target)

            with mock.patch.object(subject.os, "replace", side_effect=transient), \
                    mock.patch.object(subject.time, "sleep"), \
                    mock.patch.object(subject, "validate_outputs", side_effect=subject.CompositionError("verification failed")):
                with self.assertRaisesRegex(subject.CompositionError, "verification failed"):
                    subject.publish_outputs(root, {Path("product.json"): b'{"revision":2}'})
            self.assertEqual(2, rollback_attempts)
            self.assertEqual(b'{"revision":1}', destination.read_bytes())
            self.assertEqual([destination], list(root.iterdir()))

    def test_locked_backup_cleanup_does_not_fail_validated_commit(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            destination = root / "product.json"
            destination.write_bytes(b'{"revision":1}')
            unlink = Path.unlink
            warnings = io.StringIO()

            def locked_backup(path, *args, **kwargs):
                if ".rollback." in path.name:
                    raise windows_error(32)
                return unlink(path, *args, **kwargs)

            with mock.patch.object(Path, "unlink", locked_backup), \
                    mock.patch.object(subject.time, "sleep") as sleep, \
                    contextlib.redirect_stderr(warnings):
                subject.publish_outputs(root, {Path("product.json"): b'{"revision":2}'})
            self.assertEqual(20, sleep.call_count)
            self.assertEqual(b'{"revision":2}', destination.read_bytes())
            self.assertIn("cleanup retained", warnings.getvalue())
            self.assertEqual(1, len(list(root.glob("*.rollback.*"))))


if __name__ == "__main__":
    unittest.main()
