from __future__ import annotations

import json
import os
import shutil
import subprocess
import sys
import unittest
import uuid
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PIPELINE = ROOT / "Tools" / "ValtanPipeline" / "valtan_tuning_pipeline.py"
TRANSACTION = (
    ROOT / "Tools" / "ValtanPipeline" / "promote_valtan_animation_chains.py"
)
PROJECTOR = ROOT / "Tools" / "ValtanPipeline" / "Project-ValtanPatternMaster.ps1"
LOCK_PATH = (
    ROOT / "out" / "ValtanPatternTransactions" / "create-pattern.lock"
)
PRODUCT_PATHS = (
    "Data/Encounters/Valtan/ValtanEncounter.json",
    "Data/Encounters/Valtan/ValtanPatternRotations.json",
    "Data/Encounters/Valtan/ValtanCombatObjects.json",
    "Data/Encounters/Valtan/ValtanWorldEvents.json",
    "Data/Animation/Authored/Valtan/Valtan.patternbindings.json",
    "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json",
    "Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json",
)


@unittest.skipUnless(os.name == "nt", "shared writer byte-range contract is Windows-only")
class ValtanWriterAdmissionTests(unittest.TestCase):
    def setUp(self) -> None:
        import msvcrt

        self.msvcrt = msvcrt
        LOCK_PATH.parent.mkdir(parents=True, exist_ok=True)
        LOCK_PATH.touch(exist_ok=True)
        self.lock_handle = LOCK_PATH.open("r+b")
        if LOCK_PATH.stat().st_size < 1:
            self.lock_handle.write(b"\0")
            self.lock_handle.flush()
            os.fsync(self.lock_handle.fileno())

    def tearDown(self) -> None:
        self.lock_handle.close()

    def acquire(self, *, owner_pid: int | None = None, nonce: str | None = None) -> None:
        self.lock_handle.seek(0)
        self.msvcrt.locking(self.lock_handle.fileno(), self.msvcrt.LK_NBLCK, 1)
        if (owner_pid is None) != (nonce is None):
            raise AssertionError("writer marker requires both owner PID and nonce")
        if owner_pid is None:
            self.lock_handle.truncate(1)
        else:
            marker = (
                b"\0"
                + (
                    "lostark.valtan-canonical-writer-owner-v1:"
                    f"{owner_pid}:{nonce}\n"
                ).encode("ascii")
            )
            self.lock_handle.seek(0)
            self.lock_handle.write(marker)
            self.lock_handle.truncate(len(marker))
        self.lock_handle.flush()
        os.fsync(self.lock_handle.fileno())

    def release(self) -> None:
        self.lock_handle.truncate(1)
        self.lock_handle.flush()
        os.fsync(self.lock_handle.fileno())
        self.lock_handle.seek(0)
        self.msvcrt.locking(self.lock_handle.fileno(), self.msvcrt.LK_UNLCK, 1)

    def run_python(self, *arguments: str) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            [sys.executable, *arguments],
            cwd=ROOT,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            check=False,
            timeout=120.0,
        )

    def run_powershell(self, *arguments: str) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            [
                "powershell.exe",
                "-NoProfile",
                "-ExecutionPolicy",
                "Bypass",
                *arguments,
            ],
            cwd=ROOT,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            check=False,
            timeout=120.0,
        )

    def test_publish_candidate_fails_closed_while_create_writer_is_active(self) -> None:
        candidate_root = (
            ROOT
            / "Intermediate"
            / "ValtanWriterAdmissionTests"
            / uuid.uuid4().hex
        )
        self.acquire()
        try:
            result = self.run_python(
                str(PIPELINE),
                "--repository-root",
                str(ROOT),
                "publish-candidate",
                "--candidate-root",
                str(candidate_root),
                "--lock-timeout-seconds",
                "0",
            )
        finally:
            self.release()

        self.assertEqual(1, result.returncode, result.stdout)
        failure = json.loads(result.stderr)
        self.assertFalse(failure["ok"])
        self.assertIn(
            "canonical writer admission is held by another process",
            failure["errors"][0]["message"],
        )
        self.assertFalse((candidate_root / "current-candidate.json").exists())

    def test_projector_child_can_reuse_only_its_parents_held_admission(self) -> None:
        projection = (
            ROOT
            / "Intermediate"
            / "ValtanProductProjection"
            / ("writer-admission-test-" + uuid.uuid4().hex)
        )
        before = {relative: (ROOT / relative).read_bytes() for relative in PRODUCT_PATHS}
        try:
            projected = self.run_python(
                str(PIPELINE),
                "--repository-root",
                str(ROOT),
                "project-products",
                "--output-root",
                str(projection),
            )
            self.assertEqual(0, projected.returncode, projected.stderr)
            source_manifest_id = json.loads(projected.stdout)["payload"][
                "sourceManifestId"
            ]

            nonce = uuid.uuid4().hex
            self.acquire(owner_pid=os.getpid(), nonce=nonce)
            try:
                committed = self.run_python(
                    str(TRANSACTION),
                    "--repo-root",
                    str(ROOT),
                    "--mode",
                    "CommitProjectedProducts",
                    "--projected-product-root",
                    str(projection),
                    "--expected-source-manifest-id",
                    source_manifest_id,
                    "--external-lock-owner-pid",
                    str(os.getpid()),
                    "--external-lock-owner-nonce",
                    nonce,
                )
            finally:
                self.release()

            self.assertEqual(0, committed.returncode, committed.stderr)
            result = json.loads(committed.stdout)
            self.assertEqual("CommitProjectedProducts", result["mode"])
            self.assertEqual(0, result["changedCount"])
            self.assertEqual(
                before,
                {relative: (ROOT / relative).read_bytes() for relative in PRODUCT_PATHS},
            )

            self.acquire()
            try:
                unrelated = self.run_python(
                    str(TRANSACTION),
                    "--repo-root",
                    str(ROOT),
                    "--mode",
                    "CommitProjectedProducts",
                    "--projected-product-root",
                    str(projection),
                    "--expected-source-manifest-id",
                    source_manifest_id,
                    "--external-lock-owner-pid",
                    str(os.getpid()),
                    "--external-lock-owner-nonce",
                    nonce,
                )
            finally:
                self.release()
            self.assertEqual(1, unrelated.returncode)
            self.assertIn(
                "marker does not match its parent owner", unrelated.stderr
            )

            unheld = self.run_python(
                str(TRANSACTION),
                "--repo-root",
                str(ROOT),
                "--mode",
                "CommitProjectedProducts",
                "--projected-product-root",
                str(projection),
                "--expected-source-manifest-id",
                source_manifest_id,
                "--external-lock-owner-pid",
                str(os.getpid()),
                "--external-lock-owner-nonce",
                nonce,
            )
            self.assertEqual(1, unheld.returncode)
            self.assertIn(
                "without a held create-pattern.lock", unheld.stderr
            )
        finally:
            projection_root = (
                ROOT / "Intermediate" / "ValtanProductProjection"
            ).resolve()
            resolved = projection.resolve()
            if resolved.parent == projection_root and resolved.is_dir():
                shutil.rmtree(resolved)

    def test_publish_v2_external_mode_does_not_wait_on_its_legacy_mutex(self) -> None:
        before = {relative: (ROOT / relative).read_bytes() for relative in PRODUCT_PATHS}
        nonce = uuid.uuid4().hex
        powershell_program = "\n".join(
            (
                "$ErrorActionPreference = 'Stop'",
                f"$lockPath = '{str(LOCK_PATH)}'",
                f"$nonce = '{nonce}'",
                "$stream = [IO.FileStream]::new($lockPath, [IO.FileMode]::OpenOrCreate, "
                "[IO.FileAccess]::ReadWrite, [IO.FileShare]::ReadWrite)",
                "try {",
                "  if ($stream.Length -lt 1) { $stream.SetLength(1); $stream.Flush($true) }",
                "  $stream.Lock(0L, 1L)",
                "  [byte[]]$marker = [Text.Encoding]::ASCII.GetBytes("
                "'lostark.valtan-canonical-writer-owner-v1:' + $PID + ':' + $nonce + \"`n\")",
                "  $stream.SetLength(1L + $marker.Length)",
                "  $stream.Position = 0L; $stream.WriteByte(0)",
                "  $stream.Position = 1L; $stream.Write($marker, 0, $marker.Length)",
                "  $stream.Flush($true)",
                f"  & '{str(PROJECTOR)}' -Mode PublishV2 -RepositoryRoot '{str(ROOT)}' "
                "-WriterLockAlreadyHeld -WriterLockOwnerNonce $nonce",
                "}",
                "finally {",
                "  $stream.SetLength(1L); $stream.Position = 0L; $stream.WriteByte(0)",
                "  $stream.Flush($true); $stream.Unlock(0L, 1L); $stream.Dispose()",
                "}",
            )
        )
        published = self.run_powershell("-Command", powershell_program)

        self.assertEqual(0, published.returncode, published.stdout + published.stderr)
        self.assertIn("Valtan split Products committed", published.stdout)
        self.assertEqual(
            before,
            {relative: (ROOT / relative).read_bytes() for relative in PRODUCT_PATHS},
        )


if __name__ == "__main__":
    unittest.main()
