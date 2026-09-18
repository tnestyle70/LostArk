"""Transaction tests create their repositories exclusively below the real out/ tree."""
import copy
import json
import os
from pathlib import Path
import tempfile
import unittest
from unittest import mock

import install_raid_candidate as subject

ROOT = Path(__file__).resolve().parents[2]


class InstallerTests(unittest.TestCase):
    def setUp(self):
        parent = ROOT / "out/KoukuRaidInstallerTests"
        parent.mkdir(parents=True, exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(dir=parent)
        self.root = Path(self.temp.name) / "repo"
        self.candidate = self.root / "out/candidate"
        self.root.mkdir()
        self.manifest = {"files": [], "media": []}
        for index, path in enumerate(sorted(subject.DATA_PATHS)):
            before = {"revision": 1, "patterns": [{"patternId": "pattern.one", "value": 1}]}
            if "CharacterSoundCatalog" in path:
                before = {"classes": {"KoukuSaydon": {}}}
            after = copy.deepcopy(before)
            if "revision" in after:
                after["revision"] = 2
                after["patterns"][0]["value"] = 2
            else:
                after["classes"]["KoukuSaydon"]["sound.event"] = ["Sound/test.wav"]
            baseline = self.candidate / "baseline-current" / path
            source = self.candidate / "candidate" / path
            destination = self.root / path
            for target, value in ((baseline, before), (source, after), (destination, before)):
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_text(json.dumps(value), encoding="utf8")
            self.manifest["files"].append({"path": path, "baselineSha256": subject.sha(baseline),
                "baselineRevision": before.get("revision"), "candidateRevision": after.get("revision")})
        media = self.candidate / "media/test.wav"
        media.parent.mkdir()
        media.write_bytes(b"RIFF sandbox fixture")
        self.media_path = "Client/Bin/Resources/Sound/Test/test.wav"
        row = {"path": self.media_path, "source": str(media), "sha256": subject.sha(media)}
        self.manifest["media"] = [row, copy.deepcopy(row)]
        self.save_manifest()

    def tearDown(self):
        assert Path(self.temp.name).resolve().is_relative_to((ROOT / "out/KoukuRaidInstallerTests").resolve())
        self.temp.cleanup()

    def save_manifest(self):
        (self.candidate / "manifest.json").write_text(json.dumps(self.manifest), encoding="utf8")

    def plan(self):
        return subject.inspect(self.candidate, self.root)

    def test_default_review_does_not_install_and_deduplicates(self):
        plan = self.plan()
        self.assertTrue(plan.report["ready"])
        self.assertEqual(1, plan.report["mediaCount"])
        self.assertEqual(1, plan.report["mediaDuplicateCount"])
        self.assertFalse((self.root / self.media_path).exists())
        self.assertFalse((self.root / "out/transactions").exists())
        report = self.root / "out/review.json"
        self.assertEqual(0, subject.main(["--repository-root", str(self.root),
            "--candidate-root", str(self.candidate), "--report", str(report)]))
        self.assertFalse((self.root / self.media_path).exists())
        self.assertEqual(4, len(json.loads(report.read_text())["files"]))

    def test_exact_worldsequence_authoring_path_is_optional(self):
        path = next(iter(subject.OPTIONAL_DATA_PATHS))
        for root, value in ((self.root, {"revision": 8}),
                            (self.candidate / "baseline-current", {"revision": 8}),
                            (self.candidate / "candidate", {"revision": 9})):
            target = root / path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text(json.dumps(value))
        self.manifest["files"].append({"path": path,
            "baselineSha256": subject.sha(self.root / path), "baselineRevision": 8, "candidateRevision": 9})
        self.save_manifest()
        self.assertEqual(5, len(self.plan().report["files"]))

    def test_report_cannot_replace_manifest(self):
        manifest = self.candidate / "manifest.json"
        before = manifest.read_bytes()
        self.assertEqual(2, subject.main(["--repository-root", str(self.root),
            "--candidate-root", str(self.candidate), "--report", str(manifest)]))
        self.assertEqual(before, manifest.read_bytes())
        self.assertFalse((self.root / "out/transactions").exists())

    @unittest.skipUnless(os.name == "nt", "Windows atomic ReplaceFileW")
    def test_install_backup_and_idempotent_review(self):
        report = subject.install(self.plan())
        self.assertEqual("installed", report["status"])
        for row in self.manifest["files"]:
            self.assertEqual(subject.sha(self.candidate / "candidate" / row["path"]), subject.sha(self.root / row["path"]))
            self.assertEqual(row["baselineSha256"], subject.sha(Path(report["transactionPath"]) / "backup" / row["path"]))
        self.assertEqual(5, len(report["applied"]))
        self.assertTrue(all(row["state"] == "already-installed" for row in self.plan().report["files"]))

    def test_stale_save_reports_stable_field_conflict(self):
        path = next(path for path in subject.DATA_PATHS if "Gate1" in path)
        value = json.loads((self.root / path).read_text())
        value["revision"] = 3
        value["patterns"][0]["value"] = 99
        (self.root / path).write_text(json.dumps(value))
        plan = self.plan()
        self.assertFalse(plan.report["ready"])
        self.assertIn("$/patterns/patternId=pattern.one/value", plan.report["conflicts"][0]["sameFieldConflicts"])
        with self.assertRaises(subject.InstallError):
            subject.install(plan)

    def test_conflicting_media_duplicate_is_rejected(self):
        self.manifest["media"][1]["sha256"] = "0" * 64
        self.save_manifest()
        with self.assertRaises(subject.InstallError):
            self.plan()

    def test_path_escape_and_unauthorized_data_are_rejected(self):
        for path in ("../outside.json", "Data/Balance/PlayerSkills.json", "C:/outside.json"):
            original = self.manifest["files"][0]["path"]
            self.manifest["files"][0]["path"] = path
            self.save_manifest()
            with self.subTest(path=path), self.assertRaises(subject.InstallError):
                self.plan()
            self.manifest["files"][0]["path"] = original
        self.save_manifest()

    @unittest.skipUnless(os.name == "nt", "Windows transaction")
    def test_failure_rolls_back_data_and_only_owned_new_asset(self):
        def fail(index, entry):
            if index == 2:
                raise OSError("injected failure")
        report = subject.install(self.plan(), after_commit=fail)
        self.assertEqual("failed", report["status"])
        self.assertFalse((self.root / self.media_path).exists())
        for row in self.manifest["files"]:
            self.assertEqual(row["baselineSha256"], subject.sha(self.root / row["path"]))

    @unittest.skipUnless(os.name == "nt", "Windows transaction")
    def test_concurrent_edit_after_install_survives_rollback(self):
        edited = []
        def fail(index, entry):
            if entry.kind == "data":
                entry.destination.write_bytes(b'{"userSaved":true}')
                edited.append(entry.destination)
                raise OSError("failure after a new user save")
        report = subject.install(self.plan(), after_commit=fail)
        self.assertEqual("failed", report["status"])
        self.assertEqual(b'{"userSaved":true}', edited[0].read_bytes())
        self.assertTrue(any(row["status"] == "preserved-concurrent-edit" for row in report["rollback"]))

    @unittest.skipUnless(os.name == "nt", "Windows transaction")
    def test_save_immediately_before_replace_is_preserved(self):
        changed = []
        def change(index, entry):
            if entry.kind == "data":
                entry.destination.write_bytes(b'{"userSaved":2}')
                changed.append(entry.destination)
        report = subject.install(self.plan(), before_commit=change)
        self.assertEqual("failed", report["status"])
        self.assertEqual(b'{"userSaved":2}', changed[0].read_bytes())

    @unittest.skipUnless(os.name == "nt", "Windows ReplaceFileW late race")
    def test_save_between_hash_and_replace_is_captured_and_restored(self):
        real = subject.atomic_replace_capture
        raced = []
        def race(stage, destination, displaced):
            if not raced:
                destination.write_bytes(b'{"lateUserSave":3}')
                raced.append(destination)
            return real(stage, destination, displaced)
        with mock.patch.object(subject, "atomic_replace_capture", side_effect=race):
            report = subject.install(self.plan())
        self.assertEqual("failed", report["status"])
        self.assertEqual(b'{"lateUserSave":3}', raced[0].read_bytes())
        self.assertFalse((self.root / self.media_path).exists())

    @unittest.skipUnless(os.name == "nt", "Windows junction containment")
    def test_resolved_junction_escape_is_rejected(self):
        import subprocess
        outside = Path(self.temp.name) / "outside"
        outside.mkdir()
        link = self.root / "Client/Bin/Resources/Sound"
        link.parent.mkdir(parents=True)
        result = subprocess.run(["cmd", "/d", "/c", "mklink", "/J", str(link), str(outside)], capture_output=True)
        self.assertEqual(0, result.returncode)
        with self.assertRaises(subject.InstallError):
            self.plan()
        self.assertFalse(list(outside.iterdir()))

    @unittest.skipUnless(os.name == "nt", "Windows rollback race")
    def test_save_during_rollback_takes_precedence_over_original_backup(self):
        real = subject.atomic_replace_capture
        raced = []
        def race(stage, destination, displaced):
            if ".restore" in stage.name and not raced:
                destination.write_bytes(b'{"newestUserSave":4}')
                raced.append(destination)
            return real(stage, destination, displaced)
        def fail(index, entry):
            if entry.kind == "data":
                raise OSError("rollback requested")
        with mock.patch.object(subject, "atomic_replace_capture", side_effect=race):
            report = subject.install(self.plan(), after_commit=fail)
        self.assertEqual("failed", report["status"])
        self.assertEqual(b'{"newestUserSave":4}', raced[0].read_bytes())

    @unittest.skipUnless(os.name == "nt", "Windows owned-new rollback race")
    def test_user_save_during_new_asset_cleanup_is_restored(self):
        real = os.rename
        raced = []
        def race(source, destination, *args, **kwargs):
            if "new-capture" in str(destination):
                Path(source).write_bytes(b"new user asset")
                raced.append(source)
            return real(source, destination, *args, **kwargs)
        def fail(index, entry):
            raise OSError("rollback after first media")
        with mock.patch.object(subject.os, "rename", side_effect=race):
            report = subject.install(self.plan(), after_commit=fail)
        self.assertEqual("failed", report["status"])
        self.assertEqual(b"new user asset", Path(raced[0]).read_bytes())

    @unittest.skipUnless(os.name == "nt", "Windows transaction")
    def test_simultaneously_created_media_is_never_overwritten(self):
        real = os.link
        raced = []
        def race(source, destination, *args, **kwargs):
            if str(destination).endswith("test.wav") and not raced:
                Path(destination).write_bytes(b"concurrent media")
                raced.append(destination)
            return real(source, destination, *args, **kwargs)
        with mock.patch.object(subject.os, "link", side_effect=race):
            report = subject.install(self.plan())
        self.assertEqual("failed", report["status"])
        self.assertEqual(b"concurrent media", Path(raced[0]).read_bytes())


if __name__ == "__main__":
    unittest.main()
