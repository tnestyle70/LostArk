import msvcrt
import os
import subprocess
import tempfile
import unittest
import uuid
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MODULE = ROOT / "Tools/ValtanPipeline/ValtanCanonicalWriterAdmission.psm1"
PUBLISHER = ROOT / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1"
PIPELINE = ROOT / "Tools/ValtanPipeline/valtan_tuning_pipeline.py"
MARKER_PREFIX = "lostark.valtan-canonical-writer-owner-v1"


def powershell() -> str:
    return "powershell.exe"


class GameplayPublisherWriterAdmissionTests(unittest.TestCase):
    def test_external_child_requires_exact_held_parent_marker(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            lock = root / "out/ValtanPatternTransactions/create-pattern.lock"
            lock.parent.mkdir(parents=True)
            nonce = uuid.uuid4().hex
            with lock.open("w+b") as handle:
                handle.write(b"\0")
                handle.flush()
                handle.seek(0)
                msvcrt.locking(handle.fileno(), msvcrt.LK_NBLCK, 1)
                marker = f"{MARKER_PREFIX}:{os.getpid()}:{nonce}\n".encode("ascii")
                handle.seek(0)
                handle.truncate(1 + len(marker))
                handle.write(b"\0" + marker)
                handle.flush()
                os.fsync(handle.fileno())
                command = (
                    f"Import-Module '{MODULE}' -Force; "
                    f"$a=Enter-ValtanCanonicalWriterAdmission "
                    f"-RepositoryRoot '{root}' -ExternalOwnerPid {os.getpid()} "
                    f"-ExternalOwnerNonce '{nonce}'; "
                    "if ($a.OwnsLock) { throw 'child unexpectedly owns lock' }; "
                    "Write-Output 'ADMITTED'"
                )
                admitted = subprocess.run(
                    [powershell(), "-NoProfile", "-Command", command],
                    cwd=ROOT,
                    capture_output=True,
                    text=True,
                    timeout=20,
                    check=False,
                )
                self.assertEqual(0, admitted.returncode, admitted.stderr)
                self.assertIn("ADMITTED", admitted.stdout)

                rejected = subprocess.run(
                    [
                        powershell(),
                        "-NoProfile",
                        "-Command",
                        command.replace(nonce, "0" * 32),
                    ],
                    cwd=ROOT,
                    capture_output=True,
                    text=True,
                    timeout=20,
                    check=False,
                )
                self.assertNotEqual(0, rejected.returncode)
                self.assertIn(
                    "marker does not match",
                    (rejected.stderr + rejected.stdout).lower(),
                )
                handle.seek(0)
                msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)

    def test_competing_writer_is_rejected_by_real_byte_lock(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            lock = root / "out/ValtanPatternTransactions/create-pattern.lock"
            lock.parent.mkdir(parents=True)
            with lock.open("w+b") as handle:
                handle.write(b"\0")
                handle.flush()
                handle.seek(0)
                msvcrt.locking(handle.fileno(), msvcrt.LK_NBLCK, 1)
                command = (
                    f"Import-Module '{MODULE}' -Force; "
                    f"Enter-ValtanCanonicalWriterAdmission -RepositoryRoot '{root}' "
                    "-TimeoutSeconds 0"
                )
                contender = subprocess.run(
                    [powershell(), "-NoProfile", "-Command", command],
                    cwd=ROOT,
                    capture_output=True,
                    text=True,
                    timeout=20,
                    check=False,
                )
                self.assertNotEqual(0, contender.returncode)
                self.assertIn(
                    "timed out waiting",
                    (contender.stderr + contender.stdout).lower(),
                )
                handle.seek(0)
                msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)

    def test_shared_publisher_holds_admission_from_snapshot_through_commit(self) -> None:
        publisher = PUBLISHER.read_text(encoding="utf-8-sig")
        acquire = publisher.index("Enter-ValtanCanonicalWriterAdmission")
        first_snapshot = publisher.index("$resolvedInputOverlayRoot")
        generation = publisher.index("$presentationGenerationText")
        bootstrap_commit = publisher.index("Gameplay balance bootstrap promotion")
        release = publisher.rindex("Exit-ValtanCanonicalWriterAdmission")
        self.assertLess(acquire, first_snapshot)
        self.assertLess(first_snapshot, generation)
        self.assertLess(generation, bootstrap_commit)
        self.assertLess(bootstrap_commit, release)

        pipeline = PIPELINE.read_text(encoding="utf-8")
        self.assertIn("external_writer_identity=external_writer_identity", pipeline)
        self.assertIn('"-ExternalCanonicalWriterPid"', pipeline)
        self.assertIn('"-ExternalCanonicalWriterNonce"', pipeline)


if __name__ == "__main__":
    unittest.main()
