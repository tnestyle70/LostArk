"""Exercise the real numeric merge/promotion against isolated validator fixtures.

The publisher fixture supplies validation success/failure/concurrent-save points;
the production transaction, named mutexes, CAS checks and rollback are unchanged.
No repository authoring or runtime files are written by these tests.
"""
from pathlib import Path
import json
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
SOURCE = "Data/Balance/PlayerProfiles.json"
RECEIPT = "Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json"


class BalanceTestTransaction(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="LostArkBalanceTest-")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        for relative in (
            "Tools/GameplayPipeline/Save-BalanceTestDraft.ps1",
            "Tools/GameplayPipeline/Publish-FileTransaction.ps1",
            "Tools/ValtanPipeline/ValtanCanonicalWriterAdmission.psm1",
        ):
            destination = self.root / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / relative, destination)
        self.write(SOURCE, {"players": [{"characterClass": "LANCE_MASTER", "attackPower": 1000,
                                        "maximumHp": 50000, "futureField": {"untouched": True}}]})
        self.write(RECEIPT, {"generation": "original"})
        (self.root / "Tools/GameplayPipeline/Update-BalanceProvenanceReceipt.ps1").write_text(
            "param([string]$InputOverlayRoot)\n"
            "$p = Join-Path $InputOverlayRoot '" + RECEIPT + "'\n"
            "[IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($p)) | Out-Null\n"
            "[IO.File]::WriteAllText($p, '{\"generation\":\"staged\"}')\n", encoding="utf-8")
        self.validator("if ($d.players[0].attackPower -lt 1) { throw 'Invalid attack power' }")

    def write(self, relative, value):
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(value), encoding="utf-8")

    def validator(self, behavior):
        (self.root / "Tools/GameplayPipeline/Publish-GameplayBalance.ps1").write_text(
            "param([string]$Mode,[string]$InputOverlayRoot)\n"
            "$d = Get-Content -Raw (Join-Path $InputOverlayRoot '" + SOURCE + "') | ConvertFrom-Json\n"
            + behavior + "\n", encoding="utf-8")

    def run_draft(self, changes=None):
        self.write("draft.json", {"schema": "lostark.balance-test-draft", "formatVersion": 1, "changes": changes or [
            {"document": SOURCE, "id": "LANCE_MASTER", "field": "attackPower", "before": 1000, "value": 1200}]})
        return subprocess.run(["powershell.exe", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
            str(self.root / "Tools/GameplayPipeline/Save-BalanceTestDraft.ps1"),
            "-DraftPath", str(self.root / "draft.json")], capture_output=True, text=True, errors="replace", timeout=45)

    def test_success_merges_only_changed_fields(self):
        current = json.loads((self.root / SOURCE).read_text())
        current["players"][0]["maximumHp"] = 60000
        self.write(SOURCE, current)
        result = self.run_draft()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        actual = json.loads((self.root / SOURCE).read_text(encoding="utf-8-sig"))["players"][0]
        self.assertEqual(actual["attackPower"], 1200)
        self.assertEqual(actual["maximumHp"], 60000)
        self.assertEqual(actual["futureField"], {"untouched": True})
        self.assertEqual(json.loads((self.root / RECEIPT).read_text())["generation"], "staged")

    def test_same_field_conflict_preserves_all_bytes(self):
        current = json.loads((self.root / SOURCE).read_text())
        current["players"][0]["attackPower"] = 1500
        self.write(SOURCE, current)
        before = [(self.root / name).read_bytes() for name in (SOURCE, RECEIPT)]
        result = self.run_draft()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("CONFLICT", result.stderr)
        self.assertEqual(before, [(self.root / name).read_bytes() for name in (SOURCE, RECEIPT)])

    def test_failed_candidate_validation_preserves_authoring(self):
        self.validator("throw 'Injected candidate validation failure'")
        before = [(self.root / name).read_bytes() for name in (SOURCE, RECEIPT)]
        self.assertNotEqual(self.run_draft().returncode, 0)
        self.assertEqual(before, [(self.root / name).read_bytes() for name in (SOURCE, RECEIPT)])

    def test_save_during_validation_is_not_overwritten(self):
        self.validator("$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))\n"
                       "$p = Join-Path $root '" + SOURCE + "'\n"
                       "$fresh = Get-Content -Raw $p | ConvertFrom-Json\n"
                       "$fresh.players[0].maximumHp = 99999\n"
                       "[IO.File]::WriteAllText($p, ($fresh | ConvertTo-Json -Depth 16))")
        result = self.run_draft()
        self.assertNotEqual(result.returncode, 0)
        actual = json.loads((self.root / SOURCE).read_text())["players"][0]
        self.assertEqual((actual["attackPower"], actual["maximumHp"]), (1000, 99999))
        self.assertEqual(json.loads((self.root / RECEIPT).read_text())["generation"], "original")

    def test_unsupported_field_is_rejected(self):
        result = self.run_draft([{"document": SOURCE, "id": "LANCE_MASTER", "field": "futureField", "before": 0, "value": 1}])
        self.assertNotEqual(result.returncode, 0)
        self.assertTrue(json.loads((self.root / SOURCE).read_text())["players"][0]["futureField"]["untouched"])

    def test_late_promotion_failure_rolls_back_own_first_write(self):
        self.validator("$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))\n"
                       "$p = Join-Path $root '" + RECEIPT + "'\n"
                       "$global:BalanceTestLockedReceipt = [IO.File]::Open($p, [IO.FileMode]::Open, [IO.FileAccess]::Read, [IO.FileShare]::Read)")
        before = [(self.root / name).read_bytes() for name in (SOURCE, RECEIPT)]
        result = self.run_draft()
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(before, [(self.root / name).read_bytes() for name in (SOURCE, RECEIPT)])


if __name__ == "__main__":
    unittest.main()
