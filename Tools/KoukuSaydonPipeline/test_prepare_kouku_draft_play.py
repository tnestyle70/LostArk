"""Focused headless contracts for request-local, unsaved Kouku playback."""
import copy
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

from Tools.KoukuSaydonPipeline import prepare_kouku_draft_play as subject


ROOT = Path(__file__).resolve().parents[2]
TARGET = "KAKULSAYDON_G1_PATTERN_1"


class KoukuDraftEncodingTests(unittest.TestCase):
    def test_draft_keeps_canonical_projected_bytes(self):
        source = {"revision": 1, "patterns": [{"patternId": TARGET,
            "gateId": "G1", "targetBossPlacementId": "boss.1"}], "bundles": []}
        inventory = {"patterns": [{"patternId": TARGET, "unavailableReason": ""}], "bundles": []}
        encounter = (json.dumps({"patterns": [{"patternId": TARGET}], "bundles": []},
                                separators=(",", ":")) + "\n").encode()
        presentation = b'{"patterns":[]}\n'
        outputs = {subject.composition.ENCOUNTER_PATH: encounter,
                   subject.composition.PRESENTATION_PATH: presentation}
        with mock.patch.object(subject.composition, "_saved_pattern_inventory", return_value=inventory), \
             mock.patch.object(subject.composition, "_pattern_dependencies", return_value=set()), \
             mock.patch.object(subject.composition, "_publication_candidate", return_value=source), \
             mock.patch.object(subject.composition, "prepare_publication", return_value=(source, inventory)), \
             mock.patch.object(subject.composition, "projected_outputs", return_value=outputs):
            metadata, artifacts = subject.prepare(source, ROOT, pattern_id=TARGET)
        self.assertEqual([TARGET], metadata["patternIds"])
        self.assertEqual(encounter, artifacts[Path("encounter.json")])
        self.assertEqual(presentation, artifacts[Path("presentation.json")])


class KoukuDraftPlayContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # The canonical projector also supports direct script entry and imports
        # its raid helper from this directory.
        sys.path.insert(0, str(ROOT / "Tools/KoukuSaydonPipeline"))
        cls.source = subject.composition.load_json(ROOT / subject.composition.SOURCE_PATH)
        pattern = next(row for row in cls.source["patterns"] if row["patternId"] == TARGET)
        for lane in ("logicOccurrences", "summonOccurrences", "worldOccurrences",
                     "sceneProfileOccurrences", "presentationOccurrences", "patternOccurrences"):
            pattern[lane] = []
        pattern["stages"] = pattern["stages"][:1]
        pattern["durationMs"] = pattern["stages"][0]["durationMs"]
        pattern.pop("bossMotion", None)
        pattern.pop("resetBossYawDegrees", None)
        pattern["authoringStatus"] = "DRAFT"

    @classmethod
    def tearDownClass(cls):
        sys.path.pop(0)

    def test_target_validation_and_projection_leave_snapshot_immutable(self):
        source = copy.deepcopy(self.source)
        before = copy.deepcopy(source)
        for target in ({}, {"pattern_id": TARGET, "bundle_id": "kakulsaydon.bundle.1"},
                       {"pattern_id": "missing.pattern"}, {"bundle_id": "missing.bundle"}):
            with self.subTest(target=target), self.assertRaises(subject.composition.CompositionError):
                subject.prepare(source, ROOT, **target)
            self.assertEqual(before, source)
        metadata, artifacts = subject.prepare(source, ROOT, pattern_id=TARGET)
        self.assertEqual(before, source)
        self.assertEqual(source["revision"], metadata["sourceRevision"])
        self.assertEqual([TARGET], metadata["patternIds"])
        self.assertEqual("PATTERN", metadata["targetKind"])
        self.assertEqual({Path("encounter.json"), Path("presentation.json")}, set(artifacts))

    def test_cli_does_not_publish_and_distinguishes_equal_revision_drafts(self):
        canonical_paths = [subject.composition.SOURCE_PATH, subject.composition.ENCOUNTER_PATH,
                           subject.composition.PRESENTATION_PATH,
                           Path("Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap")]
        before = {path: (ROOT / path).read_bytes() for path in canonical_paths}
        with tempfile.TemporaryDirectory(prefix="KoukuDraftContract-", dir=ROOT / "out") as directory:
            scratch = Path(directory)
            admissions = []
            for index in range(2):
                source = copy.deepcopy(self.source)
                pattern = next(row for row in source["patterns"] if row["patternId"] == TARGET)
                pattern["durationMs"] += index * 333
                pattern["stages"][0]["durationMs"] += index * 333
                snapshot = scratch / f"snapshot-{index}.json"
                source_bytes = subject.composition.serialize_json(source)
                snapshot.write_bytes(source_bytes)
                output = scratch / f"request-{index}"
                result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                    str(ROOT / "Tools/KoukuSaydonPipeline/Prepare-KoukuDraftPlay.ps1"),
                    "-RepoRoot", str(ROOT), "-SourcePath", str(snapshot),
                    "-OutputDirectory", str(output), "-PatternId", TARGET],
                    capture_output=True, text=True, encoding="utf-8", errors="replace", timeout=60)
                self.assertEqual(0, result.returncode, result.stdout + result.stderr)
                self.assertEqual(source_bytes, snapshot.read_bytes())
                admission = json.loads((output / "admission.json").read_text(encoding="utf-8-sig"))
                rows = (output / "gameplay.rows").read_bytes()
                self.assertFalse(rows.startswith(b"\xef\xbb\xbf"))
                self.assertNotIn(b"\r", rows)
                self.assertEqual(hashlib.sha256(rows).hexdigest(), admission["rowsSha256"])
                self.assertEqual(hashlib.sha256(source_bytes).hexdigest(), admission["sourceSha256"])
                self.assertEqual(len(rows), admission["rowsBytes"])
                admissions.append(admission)
            self.assertEqual(admissions[0]["sourceRevision"], admissions[1]["sourceRevision"])
            self.assertNotEqual(admissions[0]["sourceSha256"], admissions[1]["sourceSha256"])
            self.assertNotEqual(admissions[0]["rowsSha256"], admissions[1]["rowsSha256"])
        self.assertEqual(before, {path: (ROOT / path).read_bytes() for path in canonical_paths})

    def test_cli_rejects_bad_target_and_stale_completion_marker(self):
        with tempfile.TemporaryDirectory(prefix="KoukuDraftReject-", dir=ROOT / "out") as directory:
            scratch = Path(directory)
            source_path = scratch / "snapshot.json"
            source_bytes = subject.composition.serialize_json(self.source)
            source_path.write_bytes(source_bytes)
            (scratch / "admission.json").write_text("stale completion", encoding="utf-8")
            result = subprocess.run([sys.executable, "-B", str(ROOT / "Tools/KoukuSaydonPipeline/prepare_kouku_draft_play.py"),
                "--repository-root", str(ROOT), "--source-path", str(source_path),
                "--output-directory", str(scratch), "--pattern-id", "missing.pattern"],
                capture_output=True, text=True, encoding="utf-8", errors="replace", timeout=30)
            self.assertNotEqual(0, result.returncode)
            self.assertIn("missing", result.stderr)
            self.assertFalse((scratch / "admission.json").exists())
            self.assertEqual(source_bytes, source_path.read_bytes())
        with self.assertRaises(subject.composition.CompositionError):
            subject.output_directory(ROOT, ROOT / "Data")


if __name__ == "__main__":
    unittest.main()
