#!/usr/bin/env python3
from __future__ import annotations

from contextlib import redirect_stderr, redirect_stdout
import copy
import importlib.util
import io
import json
from pathlib import Path, PurePosixPath
import shutil
import sys
import tempfile
import unittest


SCRIPT_PATH = Path(__file__).with_name(
    "build_valtan_four_slash_weapon_trail_drawable_proof.py"
)
SPEC = importlib.util.spec_from_file_location(
    "build_valtan_four_slash_weapon_trail_drawable_proof", SCRIPT_PATH
)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError(f"cannot load {SCRIPT_PATH}")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class FourSlashWeaponTrailDrawableProofTests(unittest.TestCase):
    def setUp(self) -> None:
        self.source_root = SCRIPT_PATH.resolve().parents[2]
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.resource_root = self.root / "runtime-resources"
        self.resource_root.mkdir()
        self.manifest_relative = (
            MODULE.OUTPUT_ROOT / MODULE.candidate_builder.MANIFEST_FILENAME
        )
        self.candidate_relative = MODULE.OUTPUT_ROOT / (
            MODULE.candidate_builder.CANDIDATE_FILENAME
        )
        self.canonical_relative = (
            MODULE.candidate_builder.SOURCE_DOCUMENT_RELATIVE_PATH
        )
        for relative in (
            self.manifest_relative,
            self.candidate_relative,
            self.canonical_relative,
        ):
            self._copy(relative)
        canonical = self._read(self.canonical_relative)
        canonical["elements"] = [
            row
            for row in canonical["elements"]
            if row.get("id")
            != MODULE.candidate_builder.CANDIDATE_ELEMENT_ID
        ]
        self._write(self.canonical_relative, canonical)
        manifest = self._read(self.manifest_relative)
        self.assertEqual(
            manifest["inputIdentity"]["canonicalDocumentRawSha256"],
            MODULE.raw_sha256(self._path(self.canonical_relative)),
        )
        self.sweep_relative = MODULE.SWEEP_RELATIVE_PATH
        self.proof_relative = MODULE.PROOF_RELATIVE_PATH
        self._write(self.sweep_relative, self._valid_sweep())

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def _path(self, relative: PurePosixPath) -> Path:
        return self.root.joinpath(*relative.parts)

    def _copy(self, relative: PurePosixPath) -> None:
        destination = self._path(relative)
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(self.source_root.joinpath(*relative.parts), destination)

    def _read(self, relative: PurePosixPath) -> dict:
        return json.loads(self._path(relative).read_text(encoding="utf-8"))

    def _write(self, relative: PurePosixPath, value: dict) -> None:
        path = self._path(relative)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(MODULE.pretty_bytes(value))

    def _renderer(self, draw: bool) -> dict:
        return {
            "preparedSamples": 1,
            "attemptedSamples": 1 if draw else 0,
            "submittedDraws": 1 if draw else 0,
            "suppressedDraws": 0 if draw else 1,
            "failedDraws": 0,
            "committedDraws": 1 if draw else 0,
            "transactionCommitted": True,
        }

    def _valid_sweep(self) -> dict:
        candidate_path = self._path(self.candidate_relative).resolve()
        candidate = json.loads(candidate_path.read_text(encoding="utf-8"))
        return {
            "schema": MODULE.SWEEP_SCHEMA,
            "formatVersion": 1,
            "bossArchetypeId": "BOSS_VALTAN",
            "resourceRoot": str(self.resource_root.resolve()),
            "candidatePath": str(candidate_path),
            "candidateRawSha256": MODULE.raw_sha256(candidate_path),
            "candidateTypedCodecSha256": "a" * 64,
            "effectAssetId": MODULE.candidate_builder.SOURCE_EFFECT_ASSET_ID,
            "elementId": MODULE.candidate_builder.CANDIDATE_ELEMENT_ID,
            "runtimeAnchorSlotId": MODULE.candidate_builder.RUNTIME_ANCHOR_SLOT_ID,
            "runtimeBoneName": MODULE.candidate_builder.RUNTIME_BONE_NAME,
            "sampleRateHz": 60,
            "positiveMovingBone": {
                "providerSampleCount": 104,
                "rootWorldDistinctCount": 1,
                "anchorWorldDistinctCount": 7,
                "trailPointCount": 6,
                "distinctTrailPointCount": 6,
                "finiteTrailPointCount": 6,
                "cumulativeDistance": 0.25,
                "firstWorldPosition": [0.0, 0.0, 0.0],
                "lastWorldPosition": [0.25, 0.0, 0.0],
                "renderer": self._renderer(True),
            },
            "stationaryControl": {
                "providerSampleCount": 104,
                "rootWorldDistinctCount": 1,
                "anchorWorldDistinctCount": 1,
                "trailPointCount": 1,
                "distinctTrailPointCount": 1,
                "renderer": self._renderer(False),
                "segmentSuppressed": True,
            },
            "missingAnchorControl": {
                "providerRejected": True,
                "playbackStatePreserved": True,
                "trailPointCountAfterReject": 0,
                "status": (
                    "Effect transform history is missing required anchor: "
                    + MODULE.candidate_builder.RUNTIME_ANCHOR_SLOT_ID
                ),
            },
            "disposition": "DRAWABLE_PROOF_PASS",
        }

    def _build(self) -> dict:
        return MODULE.build_proof(
            self.root,
            expected_resource_root=self.resource_root,
        )

    def test_build_write_check_and_schema(self) -> None:
        proof = self._build()
        MODULE.validate_proof(proof)
        self.assertEqual(6, proof["geometryProof"]["trailPointCount"])
        self.assertEqual(1, proof["rendererProof"]["committedDraws"])
        clone = copy.deepcopy(proof)
        seal = clone.pop("artifactSha256")
        self.assertEqual(MODULE.canonical_sha256(clone), seal)

        schema = json.loads(
            (
                self.source_root
                / "Tools/EffectPipeline/Schemas/"
                "lostark.valtan-four-slash-weapon-trail-drawable-proof.schema.json"
            ).read_text(encoding="utf-8")
        )
        sweep_schema = json.loads(
            (
                self.source_root
                / "Tools/EffectPipeline/Schemas/"
                "lostark.valtan-four-slash-weapon-trail-runtime-sweep.schema.json"
            ).read_text(encoding="utf-8")
        )
        self.assertFalse(schema["additionalProperties"])
        self.assertFalse(sweep_schema["additionalProperties"])
        try:
            import jsonschema
        except ImportError:
            pass
        else:
            jsonschema.Draft202012Validator.check_schema(schema)
            jsonschema.Draft202012Validator.check_schema(sweep_schema)
            jsonschema.validate(proof, schema)
            jsonschema.validate(self._read(self.sweep_relative), sweep_schema)

        common = [
            "--repository-root",
            str(self.root),
            "--expected-resource-root",
            str(self.resource_root),
        ]
        with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
            self.assertEqual(0, MODULE.main(["--write", *common]))
            first = self._path(self.proof_relative).read_bytes()
            self.assertEqual(0, MODULE.main(["--check", *common]))
            self.assertEqual(first, self._path(self.proof_relative).read_bytes())

    def test_positive_must_move_bone_and_draw(self) -> None:
        sweep = self._valid_sweep()
        sweep["positiveMovingBone"]["anchorWorldDistinctCount"] = 1
        self._write(self.sweep_relative, sweep)
        with self.assertRaisesRegex(MODULE.ProofError, "moving-bone"):
            self._build()

        sweep = self._valid_sweep()
        sweep["positiveMovingBone"]["renderer"] = self._renderer(False)
        self._write(self.sweep_relative, sweep)
        with self.assertRaisesRegex(MODULE.ProofError, "renderer output"):
            self._build()

    def test_stationary_and_missing_anchor_controls_are_mandatory(self) -> None:
        sweep = self._valid_sweep()
        sweep["stationaryControl"]["renderer"]["submittedDraws"] = 1
        self._write(self.sweep_relative, sweep)
        with self.assertRaisesRegex(MODULE.ProofError, "not suppressed"):
            self._build()

        sweep = self._valid_sweep()
        sweep["missingAnchorControl"]["providerRejected"] = False
        self._write(self.sweep_relative, sweep)
        with self.assertRaisesRegex(MODULE.ProofError, "fail closed"):
            self._build()

    def test_candidate_and_preserved_source_drift_fail_closed(self) -> None:
        candidate = self._read(self.candidate_relative)
        candidate["elements"][0]["displayName"] += " drift"
        self._write(self.candidate_relative, candidate)
        with self.assertRaisesRegex(MODULE.ProofError, "hash closure"):
            self._build()

        self._copy(self.candidate_relative)
        canonical = self._read(self.canonical_relative)
        source = next(
            row
            for row in canonical["elements"]
            if row["id"] == MODULE.candidate_builder.SOURCE_ELEMENT_ID
        )
        source["actionCueAttachment"]["enabled"] = True
        self._write(self.canonical_relative, canonical)
        with self.assertRaisesRegex(MODULE.ProofError, "requires rebase"):
            self._build()


if __name__ == "__main__":
    unittest.main()
