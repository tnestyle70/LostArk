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
PROFILE = "Data/Balance/Profiles/Retail.balanceprofile.json"


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
                                        "maximumHp": 50000, "moveSpeed": 8.0, "futureField": {"untouched": True}}]})
        self.write(RECEIPT, {"generation": "original"})
        self.write(PROFILE, {"schema": "lostark.balance-profile", "formatVersion": 1, "profileId": "Retail",
            "players": [], "skills": [], "damageProfiles": [], "bosses": [], "futureSection": {"preserve": 1}})
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
            "param([string]$Mode,[string]$InputOverlayRoot,[string]$BalanceProfile)\n"
            "if ($BalanceProfile -cne 'Retail') { throw 'Retail profile forwarding is required' }\n"
            "$source = Join-Path $InputOverlayRoot '" + SOURCE + "'\n"
            "if (-not (Test-Path $source)) { $source = Join-Path ([IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))) '" + SOURCE + "' }\n"
            "$d = Get-Content -Raw $source | ConvertFrom-Json\n"
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

    def retail_player(self, attack=23000):
        profile = json.loads((self.root / PROFILE).read_text())
        profile["players"] = [{"characterClass": "LANCE_MASTER", "attackPower": attack, "criticalChancePercent": 70, "futureField": 17}]
        self.write(PROFILE, profile)

    def test_retail_and_base_fields_save_together_without_erasing_unknown_fields(self):
        self.retail_player()
        result = self.run_draft([
            {"document": PROFILE, "domain": "players", "id": "LANCE_MASTER", "field": "attackPower", "before": 23000, "value": 24000},
            {"document": SOURCE, "id": "LANCE_MASTER", "field": "moveSpeed", "before": 8.0, "value": 9.5}])
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        profile = json.loads((self.root / PROFILE).read_text())
        self.assertEqual(profile["players"][0]["attackPower"], 24000)
        self.assertEqual(profile["players"][0]["futureField"], 17)
        self.assertEqual(profile["futureSection"], {"preserve": 1})
        base = json.loads((self.root / SOURCE).read_text())["players"][0]
        self.assertEqual((base["attackPower"], base["moveSpeed"]), (1000, 9.5))

    def test_retail_same_field_conflict_preserves_profile_and_base(self):
        self.retail_player(25000)
        before = [(self.root / name).read_bytes() for name in (SOURCE, PROFILE, RECEIPT)]
        result = self.run_draft([{"document": PROFILE, "domain": "players", "id": "LANCE_MASTER", "field": "attackPower", "before": 23000, "value": 24000}])
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("CONFLICT", result.stderr)
        self.assertEqual(before, [(self.root / name).read_bytes() for name in (SOURCE, PROFILE, RECEIPT)])

    def test_shadowed_base_draft_is_rejected(self):
        self.retail_player()
        before = [(self.root / name).read_bytes() for name in (SOURCE, PROFILE, RECEIPT)]
        result = self.run_draft()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Retail now owns", result.stderr)
        self.assertEqual(before, [(self.root / name).read_bytes() for name in (SOURCE, PROFILE, RECEIPT)])

    def test_profile_change_during_validation_preserves_external_edit(self):
        self.validator("$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))\n"
                       "$p = Join-Path $root '" + PROFILE + "'\n"
                       "$fresh = Get-Content -Raw $p | ConvertFrom-Json\n"
                       "$fresh.futureSection.preserve = 99\n"
                       "[IO.File]::WriteAllText($p, ($fresh | ConvertTo-Json -Depth 16))")
        result = self.run_draft()
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(json.loads((self.root / PROFILE).read_text())["futureSection"]["preserve"], 99)
        self.assertEqual(json.loads((self.root / SOURCE).read_text())["players"][0]["attackPower"], 1000)

    def test_late_promotion_failure_rolls_back_own_first_write(self):
        self.validator("$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))\n"
                       "$p = Join-Path $root '" + RECEIPT + "'\n"
                       "$global:BalanceTestLockedReceipt = [IO.File]::Open($p, [IO.FileMode]::Open, [IO.FileAccess]::Read, [IO.FileShare]::Read)")
        before = [(self.root / name).read_bytes() for name in (SOURCE, RECEIPT)]
        result = self.run_draft()
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(before, [(self.root / name).read_bytes() for name in (SOURCE, RECEIPT)])


class BalanceRuntimeSetPublication(unittest.TestCase):
    """Run the production promotion/rollback with isolated publisher outputs."""

    WORLD_FILES = tuple(name + ".worldbootstrap" for name in (
        "BERN", "VALTAN_ARENA", "KAKULSAYDON_ARENA", "TRAINING_GROUND",
        "CHARACTER_SELECT_ARENA", "MAHARAKA")) + tuple(name + ".spawngroupsbootstrap" for name in (
        "VALTAN_ARENA", "KAKULSAYDON_ARENA", "CHARACTER_SELECT_ARENA"))
    REVISION = "a" * 64

    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="LostArkBalancePublish-")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        for relative in (
            "Tools/GameplayPipeline/Publish-BalanceRuntimeSet.ps1",
            "Tools/GameplayPipeline/Publish-FileTransaction.ps1",
            "Tools/ValtanPipeline/ValtanCanonicalWriterAdmission.psm1",
        ):
            path = self.root / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / relative, path)
        (self.root / "revision.txt").write_text(self.REVISION, encoding="utf-8")
        (self.root / "Tools/ValtanPipeline/valtan_tuning_pipeline.py").write_text(
            "import json,pathlib\n"
            "root=pathlib.Path(__file__).resolve().parents[2]\n"
            "print(json.dumps({'ok':True,'command':'SOURCE_MANIFEST','payload':"
            "{'sourceManifestId':(root/'revision.txt').read_text()}}))\n", encoding="utf-8")
        self.fixture_publisher("Tools/GameplayPipeline/Publish-GameplayBalance.ps1",
            ("Gameplay.bootstrap", "ValtanPresentationGenerations/" + self.REVISION + ".json"), retail=True)
        self.fixture_publisher("Tools/WorldPipeline/Publish-WorldGameplay.ps1", self.WORLD_FILES, retail=True)
        self.fixture_publisher("Tools/GameplayPipeline/Publish-ItemCatalog.ps1", ("Items.bootstrap",))
        self.runtime = self.root / "Server/Bin/DataFiles"
        self.outputs = (
            "Gameplay/ValtanPresentationGenerations/" + self.REVISION + ".json",
            "Gameplay/Gameplay.bootstrap",
            *("World/" + name for name in self.WORLD_FILES),
            "Items/Items.bootstrap",
        )
        for relative in self.outputs:
            if relative == "World/KAKULSAYDON_ARENA.spawngroupsbootstrap":
                continue  # Rollback must restore absence as well as old bytes.
            path = self.runtime / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(("previous:" + relative).encode())
        self.before = self.snapshot()

    def fixture_publisher(self, relative, names, retail=False, after_publish=""):
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        literals = ",".join("'" + name + "'" for name in names)
        path.write_text(
            "param([string]$Mode,[string]$OutputRoot,[string]$BalanceProfile,"
            "[int]$ExternalCanonicalWriterPid,[string]$ExternalCanonicalWriterNonce)\n"
            + ("if ($BalanceProfile -cne 'Retail') { throw 'Retail profile not forwarded' }\n" if retail else "")
            + "if ($Mode -ne 'Publish') { return }\n"
            "$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))\n"
            f"foreach ($name in @({literals})) {{\n"
            "$path = Join-Path (Join-Path $root $OutputRoot) $name\n"
            "[IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($path)) | Out-Null\n"
            f"[IO.File]::WriteAllText($path, 'published:{self.REVISION}:' + $name)\n"
            "}\n" + after_publish, encoding="utf-8")

    def snapshot(self):
        return {relative: (self.runtime / relative).read_bytes() if (self.runtime / relative).exists() else None
                for relative in self.outputs}

    def run_publish(self, failure=0):
        return subprocess.run(["powershell.exe", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
            str(self.root / "Tools/GameplayPipeline/Publish-BalanceRuntimeSet.ps1"),
            "-Mode", "Publish", "-ExpectedValtanSourceRevision", self.REVISION,
            "-FailureAfterPromote", str(failure)], cwd=self.root,
            capture_output=True, text=True, errors="replace", timeout=45)

    def assert_clean_transaction(self):
        self.assertFalse(list(self.runtime.glob(".balance-runtime-set.staging.*")))
        self.assertFalse(list(self.runtime.rglob("*.rollback.*")))

    def test_publish_promotes_kouku_and_every_staged_world_and_spawn_group(self):
        result = self.run_publish()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        for relative, content in self.snapshot().items():
            with self.subTest(output=relative):
                self.assertIsNotNone(content)
                self.assertTrue(content.startswith(("published:" + self.REVISION + ":").encode()))
        self.assertIn("9 world/spawn-group outputs", result.stdout)
        self.assert_clean_transaction()

    def test_failure_after_all_promotions_restores_every_output_and_new_file_absence(self):
        result = self.run_publish(failure=len(self.outputs))
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Injected balance runtime set failure", result.stderr)
        self.assertEqual(self.snapshot(), self.before)
        self.assert_clean_transaction()

    def test_missing_required_world_rejects_before_any_runtime_promotion(self):
        for missing in ("BERN", "VALTAN_ARENA", "TRAINING_GROUND", "CHARACTER_SELECT_ARENA", "KAKULSAYDON_ARENA"):
            with self.subTest(world=missing):
                self.fixture_publisher("Tools/WorldPipeline/Publish-WorldGameplay.ps1",
                    tuple(name for name in self.WORLD_FILES if name != missing + ".worldbootstrap"), retail=True)
                result = self.run_publish()
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("Balance runtime staged output is missing", result.stderr)
                self.assertIn(missing + ".worldbootstrap", result.stderr)
                self.assertEqual(self.snapshot(), self.before)
                self.assert_clean_transaction()

    def test_final_source_revision_change_rolls_back_kouku_and_spawn_groups(self):
        self.fixture_publisher("Tools/GameplayPipeline/Publish-ItemCatalog.ps1", ("Items.bootstrap",),
            after_publish="[IO.File]::WriteAllText((Join-Path $root 'revision.txt'), '" + "b" * 64 + "')\n")
        result = self.run_publish()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("STALE_REVISION", result.stderr)
        self.assertEqual(self.snapshot(), self.before)
        self.assertEqual((self.root / "revision.txt").read_text(), "b" * 64)
        self.assert_clean_transaction()


if __name__ == "__main__":
    unittest.main()
