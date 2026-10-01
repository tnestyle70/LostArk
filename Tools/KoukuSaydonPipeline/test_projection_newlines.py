"""Generated Product parity accepts Git newline conversion only."""
from pathlib import Path
import tempfile
import unittest

import project_kouku_saydon_composition as projector


class ProjectionNewlineTests(unittest.TestCase):
    def test_git_crlf_checkout_is_equivalent(self):
        expected = b'{\n  "value": 1\n}\n'
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            relative = Path("projection.json")
            (root / relative).write_bytes(expected.replace(b"\n", b"\r\n"))
            projector.validate_outputs(root, {relative: expected})

    def test_value_change_is_stale(self):
        expected = b'{"value":1}\n'
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            relative = Path("projection.json")
            (root / relative).write_bytes(b'{"value":2}\r\n')
            with self.assertRaisesRegex(projector.CompositionError, "stale"):
                projector.validate_outputs(root, {relative: expected})

    def test_duplicate_keys_still_fail(self):
        duplicate = b'{"value":1,"value":1}\n'
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            relative = Path("projection.json")
            (root / relative).write_bytes(duplicate.replace(b"\n", b"\r\n"))
            with self.assertRaises(projector.CompositionError):
                projector.validate_outputs(root, {relative: duplicate})


if __name__ == "__main__":
    unittest.main()
