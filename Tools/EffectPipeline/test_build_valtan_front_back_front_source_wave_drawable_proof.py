#!/usr/bin/env python3
"""Tests for FRONT_BACK_FRONT source-wave machine proof sealing."""

from __future__ import annotations

from copy import deepcopy
import json
from pathlib import Path, PurePosixPath
import shutil
import subprocess
import sys
import tempfile
import unittest

import build_valtan_front_back_front_source_wave_drawable_proof as sut
import validate_boss_pattern_effects as schema_validator


SOURCE_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = Path(__file__).with_name(
    "build_valtan_front_back_front_source_wave_drawable_proof.py"
)
PROOF_SCHEMA_PATH = Path(__file__).parent / "Schemas" / (
    "lostark.valtan-front-back-front-source-wave-drawable-proof.schema.json"
)


def _write_json(path: Path, value: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n",
        encoding="utf-8",
        newline="\n",
    )


class RepositoryFixture:
    def __init__(self) -> None:
        self._temporary = tempfile.TemporaryDirectory(
            prefix="valtan-fbf-wave-proof-test."
        )
        self.root = Path(self._temporary.name)
        self.resource_root = self.root / "Client" / "Bin" / "Resources"
        self.resource_root.mkdir(parents=True)
        self.receipt_relative = sut.DEFAULT_CANDIDATE_RECEIPT
        self.sweep_relative = sut.DEFAULT_DRAWABLE_SWEEP
        self.proof_relative = sut.DEFAULT_DRAWABLE_PROOF
        source_receipt_path = SOURCE_ROOT.joinpath(*self.receipt_relative.parts)
        self.receipt = json.loads(source_receipt_path.read_text(encoding="utf-8"))
        receipt_path = self.root.joinpath(*self.receipt_relative.parts)
        receipt_path.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source_receipt_path, receipt_path)
        for candidate in self.receipt["candidates"]:
            relative = PurePosixPath(candidate["candidateDocumentPath"])
            source = SOURCE_ROOT.joinpath(*relative.parts)
            target = self.root.joinpath(*relative.parts)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, target)
        self.sweep = self._build_sweep()
        _write_json(self.root.joinpath(*self.sweep_relative.parts), self.sweep)

    def close(self) -> None:
        self._temporary.cleanup()

    def _build_sweep(self) -> dict:
        documents = []
        for candidate in self.receipt["candidates"]:
            relative = PurePosixPath(candidate["candidateDocumentPath"])
            path = self.root.joinpath(*relative.parts).resolve()
            document = json.loads(path.read_text(encoding="utf-8"))
            elements = [
                {
                    "elementId": element["id"],
                    "disposition": "DRAWABLE_PROOF_PASS",
                    "preparedSamples": 2,
                    "attemptedSamples": 2,
                    "submittedDraws": 2,
                    "suppressedDraws": 0,
                    "failedDraws": 0,
                    "committedDraws": 2,
                }
                for element in document["elements"]
            ]
            documents.append(
                {
                    "documentPath": path.as_posix(),
                    "effectAssetId": candidate["effectAssetId"],
                    "durationSeconds": 1.0,
                    "sampleCount": 2,
                    "visibleElementCount": 25,
                    "preparedElementCount": 25,
                    "drawnElementCount": 25,
                    "disposition": "DRAWABLE_PROOF_PASS",
                    "elements": elements,
                }
            )
        return {
            "schema": sut.SWEEP_SCHEMA,
            "formatVersion": 1,
            "resourceRoot": self.resource_root.resolve().as_posix(),
            "sampleRateHz": 60,
            "documents": documents,
        }

    def write_sweep(self, value: dict) -> None:
        _write_json(self.root.joinpath(*self.sweep_relative.parts), value)

    def build(self) -> dict:
        return sut.build_drawable_proof(
            self.root,
            candidate_receipt=self.receipt_relative,
            drawable_sweep=self.sweep_relative,
            expected_resource_root=self.resource_root,
        )


class FrontBackFrontSourceWaveDrawableProofTests(unittest.TestCase):
    def make_fixture(self) -> RepositoryFixture:
        fixture = RepositoryFixture()
        self.addCleanup(fixture.close)
        return fixture

    def test_build_binds_exact_four_documents_and_all_100_elements(self) -> None:
        fixture = self.make_fixture()
        proof = fixture.build()
        self.assertEqual(4, len(proof["targets"]))
        self.assertEqual(
            100, sum(len(target["elements"]) for target in proof["targets"])
        )
        self.assertEqual(
            [row["effectAssetId"] for row in fixture.receipt["candidates"]],
            [row["effectAssetId"] for row in proof["targets"]],
        )
        for candidate, target in zip(
            fixture.receipt["candidates"], proof["targets"], strict=True
        ):
            self.assertEqual(
                candidate["candidateDocumentSha256"],
                target["candidateDocumentSha256"],
            )
            self.assertTrue(
                all(row["failedDraws"] == 0 for row in target["elements"])
            )

    def test_write_and_check_are_deterministic(self) -> None:
        fixture = self.make_fixture()
        base = [
            sys.executable,
            str(SCRIPT_PATH),
            "--repo-root",
            str(fixture.root),
            "--expected-resource-root",
            str(fixture.resource_root),
        ]
        written = subprocess.run(
            [*base, "--write"], capture_output=True, text=True, check=False
        )
        self.assertEqual(0, written.returncode, written.stderr)
        proof_path = fixture.root.joinpath(*fixture.proof_relative.parts)
        first = proof_path.read_bytes()
        checked = subprocess.run(
            [*base, "--check"], capture_output=True, text=True, check=False
        )
        self.assertEqual(0, checked.returncode, checked.stderr)
        rewritten = subprocess.run(
            [*base, "--write"], capture_output=True, text=True, check=False
        )
        self.assertEqual(0, rewritten.returncode, rewritten.stderr)
        self.assertEqual(first, proof_path.read_bytes())

    def test_wrong_path_counter_or_resource_root_fails_closed(self) -> None:
        fixture = self.make_fixture()
        bad_path = deepcopy(fixture.sweep)
        bad_path["documents"][0]["documentPath"] = bad_path["documents"][1][
            "documentPath"
        ]
        fixture.write_sweep(bad_path)
        with self.assertRaises(sut.DrawableProofError):
            fixture.build()

        fixture.write_sweep(fixture.sweep)
        bad_counter = deepcopy(fixture.sweep)
        bad_counter["documents"][0]["elements"][0]["failedDraws"] = 1
        fixture.write_sweep(bad_counter)
        with self.assertRaises(sut.DrawableProofError):
            fixture.build()

        fixture.write_sweep(fixture.sweep)
        wrong_root = fixture.root / "wrong-resources"
        wrong_root.mkdir()
        with self.assertRaises(sut.DrawableProofError):
            sut.build_drawable_proof(
                fixture.root,
                candidate_receipt=fixture.receipt_relative,
                drawable_sweep=fixture.sweep_relative,
                expected_resource_root=wrong_root,
            )

    def test_proof_validates_against_strict_schema(self) -> None:
        fixture = self.make_fixture()
        schema = json.loads(PROOF_SCHEMA_PATH.read_text(encoding="utf-8"))
        schema_validator.validate_schema_instance(fixture.build(), schema)
        self.assertFalse(schema["additionalProperties"])


if __name__ == "__main__":
    unittest.main(verbosity=2)
