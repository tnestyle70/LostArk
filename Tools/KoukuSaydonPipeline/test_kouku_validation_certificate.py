"""Exact-content checks for reusing one completed canonical projection."""
import os
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from Tools.KoukuSaydonPipeline import project_kouku_saydon_composition as subject
from Tools.KoukuSaydonPipeline import test_project_kouku_saydon_composition as fixtures

ROOT = fixtures.ROOT


class KoukuValidationCertificateTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory(prefix="KoukuCertificate-", dir=ROOT / "out")
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name).resolve()
        fixtures.copy_repository_inputs(self.root)
        source = fixtures.KoukuPublishAllInventoryTests().source()
        source["patterns"] = source["patterns"][:1]
        source["folders"] = []
        source["bundles"] = []
        path = self.root / subject.SOURCE_PATH
        path.parent.mkdir(parents=True)
        path.write_bytes(subject.serialize_json(source))
        self.summary = subject.run(self.root, "publish")
        self.outputs = {relative: (self.root / relative).read_bytes()
                        for relative in (subject.ENCOUNTER_PATH, subject.PRESENTATION_PATH)}

    def test_unchanged_validate_reuses_proof_without_projection_or_output_writes(self):
        with mock.patch.object(subject, "_run", side_effect=AssertionError("must not reproject unchanged bytes")):
            self.assertEqual(self.summary, subject.run(self.root, "validate"))
        self.assertEqual(self.outputs, {path: (self.root / path).read_bytes() for path in self.outputs})

    def test_same_mtime_same_size_source_and_native_edits_miss(self):
        source = self.root / subject.SOURCE_PATH
        before = source.read_bytes()
        stat = source.stat()
        changed = before.replace(b'"authoringStatus": "DRAFT"', b'"authoringStatus": "WRONG"', 1)
        self.assertNotEqual(before, changed)
        self.assertEqual(len(before), len(changed))
        source.write_bytes(changed)
        os.utime(source, ns=(stat.st_atime_ns, stat.st_mtime_ns))
        self.assertIsNone(subject._try_validation_certificate(self.root))
        with self.assertRaises(subject.CompositionError):
            subject.run(self.root, "validate")
        source.write_bytes(before)
        native = self.root / "Client/Bin/Resources/Character/native.wmodel"
        native.parent.mkdir(parents=True)
        native.write_bytes(b"native version1")
        original = subject._run

        def read_native_then_run(root, mode):
            subject._PUBLICATION_INPUTS.get().observe_binary(native)
            return original(root, mode)

        with mock.patch.object(subject, "_run", side_effect=read_native_then_run):
            subject.run(self.root, "publish")
        self.assertIsNotNone(subject._try_validation_certificate(self.root))
        stat = native.stat()
        native.write_bytes(b"native version2")
        os.utime(native, ns=(stat.st_atime_ns, stat.st_mtime_ns))
        self.assertIsNone(subject._try_validation_certificate(self.root))

    def test_corrupt_proof_falls_back_and_reissues_only_after_success(self):
        certificate = self.root / subject._VALIDATION_CERTIFICATE
        certificate.write_text('{"incomplete":', encoding="utf-8")
        with mock.patch.object(subject, "_run", wraps=subject._run) as project:
            self.assertEqual(self.summary, subject.run(self.root, "validate"))
            self.assertEqual(1, project.call_count)
        self.assertIsNotNone(subject._try_validation_certificate(self.root))
        (self.root / subject.ENCOUNTER_PATH).write_bytes(self.outputs[subject.ENCOUNTER_PATH] + b" ")
        self.assertIsNone(subject._try_validation_certificate(self.root))
        with self.assertRaisesRegex(subject.CompositionError, "stale"):
            subject.run(self.root, "validate")

    def test_missing_optional_creation_and_tool_changes_miss(self):
        with mock.patch.object(subject, "_projection_tools", return_value=[{"path": "changed", "sha256": "0" * 64}]):
            self.assertIsNone(subject._try_validation_certificate(self.root))
        optional = self.root / "Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json"
        optional.parent.mkdir(parents=True)
        optional.write_text("{}", encoding="utf-8")
        self.assertIsNone(subject._try_validation_certificate(self.root))


if __name__ == "__main__":
    unittest.main()
