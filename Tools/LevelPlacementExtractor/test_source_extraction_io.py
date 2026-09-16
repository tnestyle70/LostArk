from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import source_extraction_io as subject


class SourceExtractionWriteTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.output, self.receipt = self.root / "output.json", self.root / "receipt.json"

    def fail_receipt_replace(self, source, target):
        if target == self.receipt:
            raise OSError("injected second promotion failure")
        return self.original_replace(source, target)

    def test_second_replace_failure_restores_both_previous_files(self):
        self.output.write_bytes(b"original output")
        self.receipt.write_bytes(b"original receipt")
        self.original_replace = subject.os.replace
        with patch.object(subject.os, "replace", side_effect=self.fail_receipt_replace):
            with self.assertRaisesRegex(OSError, "second promotion"):
                subject.write_pair(self.output, b"new output", self.receipt, b"new receipt")
        self.assertEqual(self.output.read_bytes(), b"original output")
        self.assertEqual(self.receipt.read_bytes(), b"original receipt")
        self.assertEqual(set(self.root.iterdir()), {self.output, self.receipt})

    def test_second_replace_failure_removes_new_output_when_previously_absent(self):
        self.receipt.write_bytes(b"original receipt")
        self.original_replace = subject.os.replace
        with patch.object(subject.os, "replace", side_effect=self.fail_receipt_replace):
            with self.assertRaises(OSError):
                subject.write_pair(self.output, b"new output", self.receipt, b"new receipt")
        self.assertFalse(self.output.exists())
        self.assertEqual(self.receipt.read_bytes(), b"original receipt")
        self.assertEqual(set(self.root.iterdir()), {self.receipt})

    def test_second_stage_failure_does_not_promote_first_file(self):
        self.output.write_bytes(b"old output")
        self.receipt.write_bytes(b"old receipt")
        original_stage = subject._stage
        def stage(target, payload):
            if target == self.receipt:
                raise OSError("injected stage failure")
            return original_stage(target, payload)
        with patch.object(subject, "_stage", side_effect=stage):
            with self.assertRaisesRegex(OSError, "stage failure"):
                subject.write_pair(self.output, b"new output", self.receipt, b"new receipt")
        self.assertEqual(self.output.read_bytes(), b"old output")
        self.assertEqual(self.receipt.read_bytes(), b"old receipt")
        self.assertEqual(set(self.root.iterdir()), {self.output, self.receipt})

    def test_success_pair_and_single_receipt_write_preserve_payload_bytes(self):
        subject.write_pair(self.output, b"output\x00\xff", self.receipt, b"receipt\r\n")
        self.assertEqual(self.output.read_bytes(), b"output\x00\xff")
        self.assertEqual(self.receipt.read_bytes(), b"receipt\r\n")
        subject.write_atomic(self.receipt, b"failure receipt")
        self.assertEqual(self.output.read_bytes(), b"output\x00\xff")
        self.assertEqual(self.receipt.read_bytes(), b"failure receipt")

    def test_same_destination_is_rejected_before_any_write(self):
        self.output.write_bytes(b"old")
        with self.assertRaisesRegex(ValueError, "must differ"):
            subject.write_pair(self.output, b"one", self.output, b"two")
        self.assertEqual(self.output.read_bytes(), b"old")


if __name__ == "__main__":
    unittest.main()
