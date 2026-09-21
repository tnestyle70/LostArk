"""Exercise the real catalog publishers without changing installed runtime data."""
from pathlib import Path
import json
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
POWERSHELL = shutil.which("powershell")
CATALOGS = (
    ("Tools/GameplayPipeline/Publish-ItemCatalog.ps1", "Items.bootstrap"),
    ("Tools/ValtanPipeline/Publish-ValtanClearRewards.ps1", "ClearRewards.bootstrap"),
)


@unittest.skipUnless(POWERSHELL, "Windows PowerShell is required")
class PublishedCatalogFreshnessTests(unittest.TestCase):
    def setUp(self):
        (ROOT / "out").mkdir(exist_ok=True)
        self.directory = tempfile.TemporaryDirectory(prefix="catalog-freshness-", dir=ROOT / "out")
        self.addCleanup(self.directory.cleanup)
        self.output = Path(self.directory.name)

    def run_publisher(self, script, mode, output=None):
        target = output or self.output
        return subprocess.run(
            [POWERSHELL, "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(ROOT / script),
             "-Mode", mode, "-OutputRoot", target.relative_to(ROOT).as_posix()],
            cwd=ROOT, capture_output=True, text=True, errors="replace", timeout=30,
        )

    def publish(self, script, name):
        result = self.run_publisher(script, "Publish")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        return self.output / name

    def test_current_output_and_repeat_publish_preserve_bytes_and_timestamp(self):
        for script, name in CATALOGS:
            with self.subTest(catalog=name):
                path = self.publish(script, name)
                before = (path.read_bytes(), path.stat().st_mtime_ns)
                for mode in ("CheckPublished", "Publish"):
                    result = self.run_publisher(script, mode)
                    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                    self.assertEqual((path.read_bytes(), path.stat().st_mtime_ns), before)

    def test_missing_check_never_creates_output_directory(self):
        for script, name in CATALOGS:
            with self.subTest(catalog=name):
                missing = self.output / name
                result = self.run_publisher(script, "CheckPublished", missing)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("<missing>", result.stderr)
                self.assertFalse(missing.exists())

    def test_old_schema_and_changed_row_fail_without_mutation_then_publish_repairs(self):
        for script, name in CATALOGS:
            with self.subTest(catalog=name):
                path = self.publish(script, name)
                original = path.read_bytes()
                header, rows = original.split(b"\n", 1)
                fields = header.split(b"\t")
                fields[1] = b"1"
                bad_inputs = (b"\t".join(fields) + b"\n" + rows,
                              original.replace(b"POTION_HP_SMALL", b"POTION_HP_WRONG")
                              if name == "Items.bootstrap" else header + b"\n" + rows.replace(b"LANCE_MASTER", b"WARLORD", 1),
                              b"\xef\xbb\xbf" + original, b"\xff\xfe\xfd")
                for invalid in bad_inputs:
                    path.write_bytes(invalid)
                    before_mtime = path.stat().st_mtime_ns
                    result = self.run_publisher(script, "CheckPublished")
                    self.assertNotEqual(result.returncode, 0)
                    self.assertIn("published data is stale or invalid", result.stderr)
                    self.assertEqual(path.read_bytes(), invalid)
                    self.assertEqual(path.stat().st_mtime_ns, before_mtime)
                    self.publish(script, name)
                    self.assertEqual(path.read_bytes(), original)
                self.assertFalse(list(self.output.glob("*.staging.*")))
                self.assertFalse(list(self.output.glob("*.rollback.*")))

    def test_git_lf_checkout_is_current_without_rewrite(self):
        for script, name in CATALOGS:
            with self.subTest(catalog=name):
                path = self.publish(script, name)
                path.write_bytes(path.read_bytes().replace(b"\r\n", b"\n"))
                before = (path.read_bytes(), path.stat().st_mtime_ns)
                for mode in ("CheckPublished", "Publish"):
                    result = self.run_publisher(script, mode)
                    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                    self.assertEqual((path.read_bytes(), path.stat().st_mtime_ns), before)

    def test_source_changed_after_validation_preserves_previous_output(self):
        source = self.output / "source.json"
        destination = self.output / "fixture.bootstrap"
        source.write_text(json.dumps({"value": 1}), encoding="utf-8")
        destination.write_bytes(b"previous output\n")
        helper = ROOT / "Tools/GameplayPipeline/Publish-FileTransaction.ps1"
        test_script = self.output / "changed-source.ps1"
        test_script.write_text(
            "$ErrorActionPreference = 'Stop'\n"
            f". '{helper.as_posix()}'\n"
            "$snapshots = @{}\n"
            f"Read-PublishJsonSnapshot '{source.as_posix()}' $snapshots | Out-Null\n"
            f"[IO.File]::WriteAllText('{source.as_posix()}', '{{\"value\":2}}')\n"
            f"Write-PublishTextCatalog -Mode Publish -Destination '{destination.as_posix()}' "
            "-Lines @('replacement') -Sources $snapshots -Context 'fixture' -RepairCommand 'retry'\n",
            encoding="utf-8",
        )
        result = subprocess.run([POWERSHELL, "-NoProfile", "-File", str(test_script)],
                                capture_output=True, text=True, errors="replace", timeout=30)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("input changed during validation", result.stderr)
        self.assertEqual(destination.read_bytes(), b"previous output\n")
        self.assertFalse(list(self.output.glob("*.staging.*")))

    def test_backup_cleanup_lock_does_not_report_committed_publish_as_failed(self):
        destination = self.output / "fixture.bootstrap"
        destination.write_bytes(b"previous output\n")
        helper = ROOT / "Tools/GameplayPipeline/Publish-FileTransaction.ps1"
        test_script = self.output / "locked-backup.ps1"
        test_script.write_text(
            "$ErrorActionPreference = 'Stop'\n"
            f". '{helper.as_posix()}'\n"
            "function Invoke-PublishFileOperation([scriptblock]$Operation, [string]$Context) {\n"
            "    & $Operation\n"
            "    if ($rollback -and [IO.File]::Exists($rollback) -and -not $script:heldBackup) {\n"
            "        $script:heldBackup = [IO.File]::Open($rollback, 'Open', 'Read', 'Read')\n"
            "    }\n"
            "}\n"
            "try {\n"
            f"    Write-PublishTextCatalog -Mode Publish -Destination '{destination.as_posix()}' "
            "-Lines @('replacement') -Sources @{} -Context 'fixture' -RepairCommand 'retry'\n"
            "} finally { if ($script:heldBackup) { $script:heldBackup.Dispose() } }\n",
            encoding="utf-8",
        )
        result = subprocess.run([POWERSHELL, "-NoProfile", "-File", str(test_script)],
                                capture_output=True, text=True, errors="replace", timeout=30)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("temporary cleanup failed", result.stdout)
        self.assertEqual(destination.read_bytes().replace(b"\r\n", b"\n"), b"replacement\n")
        backups = list(self.output.glob("*.rollback.*"))
        self.assertEqual(len(backups), 1)
        self.assertEqual(backups[0].read_bytes(), b"previous output\n")


if __name__ == "__main__":
    unittest.main()
