#!/usr/bin/env python3

from __future__ import annotations

import subprocess
import tempfile
import unittest
from pathlib import Path

from validate_resource_delivery_policy import validate


class ResourceDeliveryPolicyTests(unittest.TestCase):
    def setUp(self) -> None:
        self._temporary = tempfile.TemporaryDirectory(
            prefix="lostark-resource-delivery-policy-"
        )
        self.root = Path(self._temporary.name)
        subprocess.run(["git", "init", "-q", str(self.root)], check=True)
        (self.root / ".gitignore").write_text(
            "/Client/Bin/Resources/\n", encoding="utf-8"
        )

    def tearDown(self) -> None:
        self._temporary.cleanup()

    def test_drive_owned_physical_resource_is_allowed_when_untracked(self) -> None:
        resource = self.root / "Client/Bin/Resources/Effect/local.dds"
        resource.parent.mkdir(parents=True)
        resource.write_bytes(b"physical-drive-input")

        result = validate(self.root, require_local=True)

        self.assertTrue(result.ok, result.errors)
        self.assertEqual(1, result.physical_file_count)
        self.assertEqual(len(b"physical-drive-input"), result.physical_bytes)

    def test_tracked_resource_is_rejected_even_when_ignore_rule_exists(self) -> None:
        resource = self.root / "Client/Bin/Resources/Sound/cue.wav"
        resource.parent.mkdir(parents=True)
        resource.write_bytes(b"RIFF")
        subprocess.run(
            ["git", "-C", str(self.root), "add", "-f", resource.as_posix()],
            check=True,
        )

        result = validate(self.root)

        self.assertFalse(result.ok)
        self.assertEqual(("Client/Bin/Resources/Sound/cue.wav",), result.tracked_paths)
        self.assertTrue(any("Drive-owned" in error for error in result.errors))

    def test_missing_exact_ignore_rule_is_rejected(self) -> None:
        (self.root / ".gitignore").write_text("*.dds\n", encoding="utf-8")

        result = validate(self.root)

        self.assertFalse(result.ok)
        self.assertTrue(any("exact rule" in error for error in result.errors))

    def test_local_pack_is_optional_for_clean_git_checkout_validation(self) -> None:
        result = validate(self.root)

        self.assertTrue(result.ok, result.errors)
        self.assertEqual(0, result.physical_file_count)


if __name__ == "__main__":
    unittest.main()
