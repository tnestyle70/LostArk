from __future__ import annotations

import contextlib
import importlib.util
import io
import subprocess
import sys
import unittest
from pathlib import Path
from unittest import mock


SCRIPT = Path(__file__).with_name("build_valtan_stage_effects.py")
SPEC = importlib.util.spec_from_file_location("valtan_legacy_stage_audit", SCRIPT)
assert SPEC and SPEC.loader
AUDIT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(AUDIT)


class ValtanLegacyStageAuditTests(unittest.TestCase):
    def test_binding_clip_reader_preserves_order_for_v1_and_v2(self) -> None:
        self.assertEqual(
            AUDIT.legacy_binding_clips(
                {"actionId": "a", "clip": ["mesh_a", "mesh_b"]}
            ),
            ["mesh_a", "mesh_b"],
        )
        self.assertEqual(
            AUDIT.legacy_binding_clips(
                {
                    "actionId": "a",
                    "clips": [
                        {"clipOccurrenceId": "a.1", "clip": "mesh_a"},
                        {"clipOccurrenceId": "a.2", "clip": "mesh_b"},
                    ],
                }
            ),
            ["mesh_a", "mesh_b"],
        )

    def test_main_is_report_only_and_never_calls_a_writer(self) -> None:
        output = io.StringIO()
        with mock.patch.object(sys, "argv", [str(SCRIPT), "--audit-legacy"]), \
                mock.patch.object(
                    AUDIT,
                    "write_json_atomic",
                    side_effect=AssertionError("legacy audit attempted mutation"),
                ), contextlib.redirect_stdout(output):
            self.assertEqual(AUDIT.main(), 0)
        self.assertIn(
            "legacy audit only; canonical writer is disabled", output.getvalue()
        )

    def test_retired_write_and_prune_switches_are_rejected(self) -> None:
        for switch in ("--write", "--prune", "--migrate-existing"):
            result = subprocess.run(
                [sys.executable, str(SCRIPT), switch],
                cwd=SCRIPT.parents[2],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=False,
            )
            self.assertNotEqual(result.returncode, 0, switch)
            self.assertIn("unrecognized arguments", result.stderr)


if __name__ == "__main__":
    unittest.main()
