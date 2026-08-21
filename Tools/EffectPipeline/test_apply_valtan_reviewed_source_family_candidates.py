#!/usr/bin/env python3
"""Unit tests for the reviewed Valtan source-family transactional applicator."""

from __future__ import annotations

from copy import deepcopy
import hashlib
import json
from pathlib import Path, PurePosixPath
import shutil
import subprocess
import sys
import tempfile
import unittest

import apply_valtan_reviewed_source_family_candidates as sut
import validate_boss_pattern_effects as schema_validator


SOURCE_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = Path(__file__).with_name(
    "apply_valtan_reviewed_source_family_candidates.py"
)
PROOF_SCHEMA_PATH = Path(__file__).parent / "Schemas" / (
    "lostark.valtan-reviewed-source-family-drawable-proof.schema.json"
)
APPLICATION_SCHEMA_PATH = Path(__file__).parent / "Schemas" / (
    "lostark.valtan-reviewed-source-family-application-receipt.schema.json"
)
FIXTURE_PROOF = PurePosixPath(
    "Data/Effects/Imported/Valtan/ReviewedSourceFamilies/"
    "fixture.reviewed-source-family-drawable-proof.v1.json"
)


def _sha256(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def _read_json(root: Path, relative: str | PurePosixPath) -> dict:
    path = PurePosixPath(relative)
    return json.loads(root.joinpath(*path.parts).read_text(encoding="utf-8"))


def _write_json_like(path: Path, value: dict) -> None:
    source = path.read_bytes() if path.is_file() else b""
    newline = "\r\n" if b"\r\n" in source else "\n"
    text = json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    if newline == "\r\n":
        text = text.replace("\n", "\r\n")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(text.encode("utf-8"))


def _snapshot(root: Path) -> dict[str, bytes]:
    return {
        path.relative_to(root).as_posix(): path.read_bytes()
        for path in sorted(root.rglob("*"))
        if path.is_file()
    }


class RepositoryFixture:
    def __init__(self) -> None:
        self._temporary = tempfile.TemporaryDirectory(
            prefix="valtan-reviewed-source-applicator-test."
        )
        self.root = Path(self._temporary.name)
        receipt_relative = sut.DEFAULT_CANDIDATE_RECEIPT
        source_receipt = SOURCE_ROOT.joinpath(*receipt_relative.parts)
        self.receipt = json.loads(source_receipt.read_text(encoding="utf-8"))
        paths: set[PurePosixPath] = {
            receipt_relative,
            sut.SAFE_GAP_MANIFEST,
            sut.SAFE_GAP_APPLICATION_RECEIPT,
        }
        for document in self.receipt["documents"]:
            paths.add(PurePosixPath(document["candidateDocumentPath"]))
            paths.add(PurePosixPath(document["authoredDocumentPath"]))
        sources = self.receipt["sources"]
        for name in ("selectionManifest", "cueDocument", "effectCatalog"):
            paths.add(PurePosixPath(sources[name]["path"]))
        for row in sources["sourceInventoryRepositorySources"]:
            paths.add(PurePosixPath(row["path"]))
        for row in self.receipt["protectedCanaries"]:
            paths.add(PurePosixPath(row["authoredDocumentPath"]))
        for relative in sorted(paths, key=PurePosixPath.as_posix):
            source = SOURCE_ROOT.joinpath(*relative.parts)
            target = self.root.joinpath(*relative.parts)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, target)
        self._normalize_canonical_documents_to_pre_apply()
        self.proof_relative = FIXTURE_PROOF
        self.proof_path = self.root.joinpath(*FIXTURE_PROOF.parts)
        self.proof_path.parent.mkdir(parents=True, exist_ok=True)
        self.proof = self._build_proof()
        _write_json_like(self.proof_path, self.proof)

    def close(self) -> None:
        self._temporary.cleanup()

    def _normalize_canonical_documents_to_pre_apply(self) -> None:
        """Remove reviewed rows so the fixture exercises missing-only append.

        The repository itself is already projected.  A useful applicator test
        must reconstruct the immediately-pre-apply authored state without
        changing any non-reviewed row or the immutable candidate receipt.
        """
        for row in self.receipt["documents"]:
            candidate = _read_json(self.root, row["candidateDocumentPath"])
            projected_pairs = {
                (element["id"], element["sourceNode"])
                for element in candidate["elements"]
            }
            target_path = self.root.joinpath(
                *PurePosixPath(row["authoredDocumentPath"]).parts
            )
            authored = json.loads(target_path.read_text(encoding="utf-8"))
            authored["elements"] = [
                element
                for element in authored["elements"]
                if (element.get("id"), element.get("sourceNode"))
                not in projected_pairs
            ]
            _write_json_like(target_path, authored)

    def _build_proof(self) -> dict:
        receipt_path = self.root.joinpath(*sut.DEFAULT_CANDIDATE_RECEIPT.parts)
        targets = []
        for document in self.receipt["documents"]:
            candidate = _read_json(self.root, document["candidateDocumentPath"])
            targets.append(
                {
                    "effectAssetId": document["effectAssetId"],
                    "candidateDocumentSha256": document["candidateDocumentSha256"],
                    "disposition": "DRAWABLE_PROOF_PASS",
                    "elements": [
                        {
                            "elementId": element["id"],
                            "sourceNode": element["sourceNode"],
                            "disposition": "DRAWABLE_PROOF_PASS",
                            "attemptedSamples": 1,
                            "submittedDraws": 1,
                            "suppressedDraws": 0,
                            "failedDraws": 0,
                            "committedDraws": 1,
                        }
                        for element in candidate["elements"]
                    ],
                }
            )
        return {
            "schema": sut.PROOF_SCHEMA,
            "formatVersion": 1,
            "bossArchetypeId": sut.BOSS_ARCHETYPE_ID,
            "candidateReceiptSha256": _sha256(receipt_path.read_bytes()),
            "targets": targets,
        }

    def projection(self) -> sut.Projection:
        return sut.collect_projection(
            self.root,
            drawable_proof=self.proof_path,
        )


class ReviewedSourceFamilyApplicatorTests(unittest.TestCase):
    def make_fixture(self) -> RepositoryFixture:
        fixture = RepositoryFixture()
        self.addCleanup(fixture.close)
        return fixture

    def test_dry_run_is_read_only_and_closes_36_docs_279_elements(self) -> None:
        fixture = self.make_fixture()
        before = _snapshot(fixture.root)
        projection = fixture.projection()
        self.assertEqual(before, _snapshot(fixture.root))
        self.assertEqual(36, len(projection.canonical_outputs))
        self.assertEqual(37, len(projection.changed_paths))
        self.assertEqual(1, len(projection.new_paths))
        closure = projection.receipt["closure"]
        self.assertEqual(36, closure["inputCandidateDocumentCount"])
        self.assertEqual(279, closure["inputCandidateElementCount"])
        self.assertEqual(36, closure["applicableCandidateDocumentCount"])
        self.assertEqual(279, closure["projectedSourceElementCount"])
        self.assertEqual(100, closure["reportOnlyMultipleCueProjectionCount"])
        self.assertNotIn("excludedTargets", projection.receipt)
        with self.assertRaises(sut.ProjectionError):
            sut.check_projection(projection)

    def test_apply_preserves_existing_rows_and_second_apply_is_byte_idempotent(self) -> None:
        fixture = self.make_fixture()
        projection = fixture.projection()
        before_documents = {
            target["effectAssetId"]: _read_json(
                fixture.root, target["targetAuthoredDocumentPath"]
            )
            for target in projection.receipt["targets"]
        }
        protected_before = {
            row["path"]: fixture.root.joinpath(*PurePosixPath(row["path"]).parts).read_bytes()
            for row in projection.receipt["protectedCanaries"]
        }
        source_before = {
            row["path"]: fixture.root.joinpath(*PurePosixPath(row["path"]).parts).read_bytes()
            for row in projection.receipt["sourceGuards"]
        }
        sut.commit_projection(projection)
        sut.check_projection(fixture.projection())
        appended = 0
        for target in projection.receipt["targets"]:
            before = before_documents[target["effectAssetId"]]
            after = _read_json(fixture.root, target["targetAuthoredDocumentPath"])
            self.assertEqual(before["elements"], after["elements"][: len(before["elements"])])
            delta = len(after["elements"]) - len(before["elements"])
            self.assertEqual(target["candidateElementCount"], delta)
            appended += delta
        self.assertEqual(279, appended)
        for relative, payload in protected_before.items():
            self.assertEqual(
                payload, fixture.root.joinpath(*PurePosixPath(relative).parts).read_bytes()
            )
        for relative, payload in source_before.items():
            self.assertEqual(
                payload, fixture.root.joinpath(*PurePosixPath(relative).parts).read_bytes()
            )

        after_first = _snapshot(fixture.root)
        second = fixture.projection()
        self.assertEqual((), second.changed_paths)
        sut.commit_projection(second)
        self.assertEqual(after_first, _snapshot(fixture.root))

    def test_existing_and_projected_hand_tuning_are_deep_preserved(self) -> None:
        fixture = self.make_fixture()
        initial = fixture.projection()
        first_target = initial.receipt["targets"][0]
        target_path = fixture.root.joinpath(
            *PurePosixPath(first_target["targetAuthoredDocumentPath"]).parts
        )
        document = json.loads(target_path.read_text(encoding="utf-8"))
        document["elements"][0]["displayName"] = "hand-tuned-existing-row"
        document["elements"][0]["detail"]["color"]["multiply"] = [0.2, 0.3, 0.4, 0.5]
        existing_tuned = deepcopy(document["elements"][0])
        _write_json_like(target_path, document)
        sut.commit_projection(fixture.projection())
        applied = json.loads(target_path.read_text(encoding="utf-8"))
        self.assertEqual(existing_tuned, applied["elements"][0])

        candidate_id = first_target["candidateElements"][0]["elementId"]
        projected = next(row for row in applied["elements"] if row["id"] == candidate_id)
        projected["displayName"] = "hand-tuned-projected-row"
        projected["detail"]["color"]["multiply"] = [0.7, 0.6, 0.5, 0.4]
        tuned_projected = deepcopy(projected)
        _write_json_like(target_path, applied)
        sut.commit_projection(fixture.projection())
        final = json.loads(target_path.read_text(encoding="utf-8"))
        self.assertEqual(
            tuned_projected,
            next(row for row in final["elements"] if row["id"] == candidate_id),
        )

    def test_partial_identity_collision_requires_source_rebase_without_mutation(self) -> None:
        fixture = self.make_fixture()
        projection = fixture.projection()
        target = projection.receipt["targets"][0]
        path = fixture.root.joinpath(*PurePosixPath(target["targetAuthoredDocumentPath"]).parts)
        document = json.loads(path.read_text(encoding="utf-8"))
        document["elements"][0]["id"] = target["candidateElements"][0]["elementId"]
        _write_json_like(path, document)
        before = _snapshot(fixture.root)
        with self.assertRaisesRegex(sut.SourceRebaseRequired, "SOURCE_REBASE_REQUIRED"):
            fixture.projection()
        self.assertEqual(before, _snapshot(fixture.root))

    def test_projected_immutable_source_drift_requires_rebase(self) -> None:
        fixture = self.make_fixture()
        projection = fixture.projection()
        sut.commit_projection(projection)
        target = projection.receipt["targets"][0]
        path = fixture.root.joinpath(*PurePosixPath(target["targetAuthoredDocumentPath"]).parts)
        document = json.loads(path.read_text(encoding="utf-8"))
        candidate_id = target["candidateElements"][0]["elementId"]
        element = next(row for row in document["elements"] if row["id"] == candidate_id)
        element["material"]["templateId"] = "drifted.template"
        _write_json_like(path, document)
        before = _snapshot(fixture.root)
        with self.assertRaisesRegex(sut.SourceRebaseRequired, "SOURCE_REBASE_REQUIRED"):
            fixture.projection()
        self.assertEqual(before, _snapshot(fixture.root))

    def test_missing_or_failed_drawable_proof_is_fail_closed(self) -> None:
        fixture = self.make_fixture()
        proof = deepcopy(fixture.proof)
        proof["targets"][0]["elements"][0]["failedDraws"] = 1
        _write_json_like(fixture.proof_path, proof)
        before = _snapshot(fixture.root)
        with self.assertRaises(sut.ProjectionError):
            fixture.projection()
        self.assertEqual(before, _snapshot(fixture.root))

        fixture.proof_path.unlink()
        before_missing = _snapshot(fixture.root)
        with self.assertRaises(sut.ProjectionError):
            fixture.projection()
        self.assertEqual(before_missing, _snapshot(fixture.root))

    def test_injected_commit_failure_rolls_back_all_outputs(self) -> None:
        fixture = self.make_fixture()
        projection = fixture.projection()
        before = _snapshot(fixture.root)
        with self.assertRaisesRegex(sut.ProjectionError, "rolled back"):
            sut.commit_projection(projection, failure_after_promote=3)
        self.assertEqual(before, _snapshot(fixture.root))

    def test_guard_drift_blocks_transaction_before_any_output(self) -> None:
        fixture = self.make_fixture()
        projection = fixture.projection()
        catalog_row = next(
            row for row in projection.receipt["sourceGuards"]
            if row["path"].endswith("EffectCatalog.json")
        )
        catalog_path = fixture.root.joinpath(*PurePosixPath(catalog_row["path"]).parts)
        catalog_path.write_bytes(catalog_path.read_bytes() + b"\n")
        before_commit = _snapshot(fixture.root)
        with self.assertRaisesRegex(sut.ProjectionError, "transaction not started"):
            sut.commit_projection(projection)
        self.assertEqual(before_commit, _snapshot(fixture.root))

    def test_protected_whirlwind_canary_sha_drift_is_fail_closed(self) -> None:
        fixture = self.make_fixture()
        protected = fixture.receipt["protectedCanaries"][0]
        self.assertEqual(sut.PROTECTED_WHIRLWIND_EFFECT_ID, protected["effectAssetId"])
        path = fixture.root.joinpath(*PurePosixPath(protected["authoredDocumentPath"]).parts)
        path.write_bytes(path.read_bytes() + b"\n")
        before = _snapshot(fixture.root)
        with self.assertRaisesRegex(sut.SourceRebaseRequired, "protected canary SHA drift"):
            fixture.projection()
        self.assertEqual(before, _snapshot(fixture.root))

    def test_cli_dry_run_check_apply_and_fail_closed_without_proof(self) -> None:
        fixture = self.make_fixture()
        base = [sys.executable, str(SCRIPT_PATH), "--repo-root", str(fixture.root)]
        proof_args = ["--drawable-proof", str(fixture.proof_path)]
        before = _snapshot(fixture.root)
        dry = subprocess.run(
            [*base, "--dry-run", *proof_args], capture_output=True, text=True, check=False
        )
        self.assertEqual(0, dry.returncode, dry.stderr)
        self.assertEqual(before, _snapshot(fixture.root))
        stale = subprocess.run(
            [*base, "--check", *proof_args], capture_output=True, text=True, check=False
        )
        self.assertEqual(1, stale.returncode)
        applied = subprocess.run(
            [*base, "--apply", *proof_args], capture_output=True, text=True, check=False
        )
        self.assertEqual(0, applied.returncode, applied.stderr)
        checked = subprocess.run(
            [*base, "--check", *proof_args], capture_output=True, text=True, check=False
        )
        self.assertEqual(0, checked.returncode, checked.stderr)

        no_proof_fixture = self.make_fixture()
        no_proof_before = _snapshot(no_proof_fixture.root)
        no_proof = subprocess.run(
            [sys.executable, str(SCRIPT_PATH), "--repo-root", str(no_proof_fixture.root), "--apply"],
            capture_output=True,
            text=True,
            check=False,
        )
        self.assertNotEqual(0, no_proof.returncode)
        self.assertEqual(no_proof_before, _snapshot(no_proof_fixture.root))

    def test_proof_and_projection_receipts_validate_against_schemas(self) -> None:
        fixture = self.make_fixture()
        projection = fixture.projection()
        proof_schema = json.loads(PROOF_SCHEMA_PATH.read_text(encoding="utf-8"))
        application_schema = json.loads(
            APPLICATION_SCHEMA_PATH.read_text(encoding="utf-8")
        )
        schema_validator.validate_schema_instance(fixture.proof, proof_schema)
        schema_validator.validate_schema_instance(projection.receipt, application_schema)


if __name__ == "__main__":
    unittest.main(verbosity=2)
