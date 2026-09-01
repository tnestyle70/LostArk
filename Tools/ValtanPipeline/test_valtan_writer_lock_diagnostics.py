from __future__ import annotations

import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


MODULE_DIRECTORY = Path(__file__).resolve().parent
sys.path.insert(0, str(MODULE_DIRECTORY))

import promote_valtan_animation_chains as promotion
import valtan_tuning_pipeline as tuning


@unittest.skipUnless(os.name == "nt", "canonical byte-range lock contract is Windows-only")
class ValtanWriterLockDiagnosticTests(unittest.TestCase):
    def run_contender(self, root: Path) -> dict[str, object]:
        program = r"""
import json
import sys
from pathlib import Path
sys.path.insert(0, sys.argv[2])
import promote_valtan_animation_chains as promotion
try:
    with promotion._exclusive_transaction_lock(
        Path(sys.argv[1]), timeout_seconds=0.0, operation="ChildContender"
    ):
        raise SystemExit("child unexpectedly acquired the canonical lock")
except promotion.CanonicalTransactionBusyError as exc:
    print(json.dumps(exc.as_error(), sort_keys=True))
"""
        completed = subprocess.run(
            [sys.executable, "-c", program, str(root), str(MODULE_DIRECTORY)],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            check=False,
            timeout=20.0,
        )
        self.assertEqual(0, completed.returncode, completed.stdout + completed.stderr)
        return json.loads(completed.stdout)

    def test_same_process_nested_owner_is_self_and_outer_lock_is_not_leaked(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            lock_path = root / promotion.TRANSACTION_LOCK_REL
            with promotion._exclusive_transaction_lock(
                root, operation="OuterCanonicalSave"
            ):
                with self.assertRaises(promotion.CanonicalTransactionBusyError) as raised:
                    with promotion._exclusive_transaction_lock(
                        root,
                        timeout_seconds=0.0,
                        operation="NestedCanonicalSave",
                    ):
                        pass
                error = raised.exception.as_error()
                self.assertEqual("CANONICAL_TRANSACTION_BUSY", error["errorCode"])
                self.assertEqual(os.getpid(), error["lockOwner"]["pid"])
                self.assertEqual("OuterCanonicalSave", error["lockOwner"]["operation"])
                self.assertEqual("SELF", error["lockOwner"]["relation"])
                self.assertIsInstance(
                    error["lockOwner"]["acquisitionAgeMs"], int
                )
                self.assertIn("nested acquisition", error["message"])

            self.assertEqual(b"\0", lock_path.read_bytes())
            with promotion._exclusive_transaction_lock(
                root, operation="AfterNestedRelease"
            ):
                pass
            self.assertEqual(b"\0", lock_path.read_bytes())

    def test_parent_owner_is_reported_as_ancestor_with_operation_and_age(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            with promotion._exclusive_transaction_lock(
                root, operation="ParentCanonicalSave"
            ):
                error = self.run_contender(root)

            self.assertEqual("CANONICAL_TRANSACTION_BUSY", error["errorCode"])
            owner = error["lockOwner"]
            self.assertEqual(os.getpid(), owner["pid"])
            self.assertEqual("ParentCanonicalSave", owner["operation"])
            self.assertEqual("ANCESTOR", owner["relation"])
            self.assertIsInstance(owner["acquisitionAgeMs"], int)
            self.assertGreaterEqual(owner["acquisitionAgeMs"], 0)
            self.assertIn("ancestor process", error["message"])

    def test_markerless_owner_is_explicitly_unknown(self) -> None:
        import msvcrt

        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            lock_path = root / promotion.TRANSACTION_LOCK_REL
            lock_path.parent.mkdir(parents=True)
            lock_path.write_bytes(b"\0")
            with lock_path.open("r+b") as handle:
                handle.seek(0)
                msvcrt.locking(handle.fileno(), msvcrt.LK_NBLCK, 1)
                try:
                    error = self.run_contender(root)
                finally:
                    handle.seek(0)
                    msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)

            owner = error["lockOwner"]
            self.assertIsNone(owner["pid"])
            self.assertEqual("UNKNOWN", owner["operation"])
            self.assertIsNone(owner["acquisitionAgeMs"])
            self.assertEqual("UNKNOWN", owner["relation"])
            self.assertIn("lockOwnerPid=UNKNOWN", error["message"])
            self.assertIn("lockAcquisitionAgeMs=UNKNOWN", error["message"])

    def test_tuning_busy_error_preserves_promotion_owner_payload(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            with promotion._exclusive_transaction_lock(
                root, operation="ApplyTypedPatch"
            ):
                with self.assertRaises(tuning.CanonicalTransactionBusyError) as raised:
                    with tuning._exclusive_canonical_writer_admission(
                        root, timeout_seconds=0.0
                    ):
                        pass
                error = raised.exception.as_error()
                self.assertEqual("CANONICAL_TRANSACTION_BUSY", error["errorCode"])
                self.assertEqual(os.getpid(), error["lockOwner"]["pid"])
                self.assertEqual("ApplyTypedPatch", error["lockOwner"]["operation"])
                self.assertEqual("SELF", error["lockOwner"]["relation"])
                self.assertIn(
                    "held by this process (nested acquisition)", error["message"]
                )

            with tuning._exclusive_canonical_writer_admission(
                root, timeout_seconds=0.0
            ):
                pass
            self.assertEqual(
                b"\0", (root / tuning.CANONICAL_WRITER_LOCK_REL).read_bytes()
            )


if __name__ == "__main__":
    unittest.main()
