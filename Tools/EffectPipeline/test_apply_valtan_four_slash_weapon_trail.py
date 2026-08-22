#!/usr/bin/env python3
from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path, PurePosixPath
import shutil
import sys
import tempfile
import unittest


SCRIPT_PATH = Path(__file__).with_name("apply_valtan_four_slash_weapon_trail.py")
SPEC = importlib.util.spec_from_file_location(
    "apply_valtan_four_slash_weapon_trail", SCRIPT_PATH
)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError(f"cannot load {SCRIPT_PATH}")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class FourSlashWeaponTrailApplicationTests(unittest.TestCase):
    def setUp(self) -> None:
        self.source_root = SCRIPT_PATH.resolve().parents[2]
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.resource_root = self.root / "runtime-resources"
        self.resource_root.mkdir()
        self.manifest_relative = (
            MODULE.candidate_builder.OUTPUT_DIRECTORY_RELATIVE_PATH
            / MODULE.candidate_builder.MANIFEST_FILENAME
        )
        self.candidate_relative = (
            MODULE.candidate_builder.OUTPUT_DIRECTORY_RELATIVE_PATH
            / MODULE.candidate_builder.CANDIDATE_FILENAME
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
        self.sweep_relative = MODULE.proof_builder.SWEEP_RELATIVE_PATH
        self.proof_relative = MODULE.proof_builder.PROOF_RELATIVE_PATH
        self.receipt_relative = MODULE.RECEIPT_RELATIVE_PATH
        self._write(self.sweep_relative, self._runtime_sweep())
        proof = MODULE.proof_builder.build_proof(
            self.root, expected_resource_root=self.resource_root
        )
        self._write(self.proof_relative, proof)

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

    @staticmethod
    def _renderer(draw: bool) -> dict:
        return {
            "preparedSamples": 1,
            "attemptedSamples": 1 if draw else 0,
            "submittedDraws": 1 if draw else 0,
            "suppressedDraws": 0 if draw else 1,
            "failedDraws": 0,
            "committedDraws": 1 if draw else 0,
            "transactionCommitted": True,
        }

    def _runtime_sweep(self) -> dict:
        candidate_path = self._path(self.candidate_relative).resolve()
        candidate = json.loads(candidate_path.read_text(encoding="utf-8"))
        return {
            "schema": MODULE.proof_builder.SWEEP_SCHEMA,
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

    def _collect(self) -> MODULE.Projection:
        return MODULE.collect_projection(self.root)

    def _snapshot(self) -> dict[str, bytes]:
        return {
            path.relative_to(self.root).as_posix(): path.read_bytes()
            for path in sorted(self.root.rglob("*"))
            if path.is_file()
        }

    def test_dry_run_apply_check_and_second_apply_are_idempotent(self) -> None:
        canonical_before = self._read(self.canonical_relative)
        source_before = next(
            row
            for row in canonical_before["elements"]
            if row["id"] == MODULE.candidate_builder.SOURCE_ELEMENT_ID
        )
        snapshot_before = self._snapshot()
        projection = self._collect()
        self.assertEqual(snapshot_before, self._snapshot())
        self.assertFalse(projection.already_applied)
        self.assertEqual(
            (self.canonical_relative, self.receipt_relative),
            projection.changed_paths,
        )
        self.assertEqual(
            {self.canonical_relative, self.receipt_relative},
            set(projection.outputs),
        )

        MODULE.commit_projection(projection)
        applied = self._collect()
        MODULE.check_projection(applied)
        self.assertTrue(applied.already_applied)
        self.assertEqual((), applied.changed_paths)
        after = self._read(self.canonical_relative)
        source_after = next(
            row
            for row in after["elements"]
            if row["id"] == MODULE.candidate_builder.SOURCE_ELEMENT_ID
        )
        self.assertEqual(source_before, source_after)
        projected = [
            row
            for row in after["elements"]
            if row["id"] == MODULE.candidate_builder.CANDIDATE_ELEMENT_ID
        ]
        self.assertEqual(1, len(projected))

        first_apply = self._snapshot()
        MODULE.commit_projection(self._collect())
        self.assertEqual(first_apply, self._snapshot())

    def test_hand_tuned_detail_is_preserved_but_protected_contract_is_sealed(self) -> None:
        MODULE.commit_projection(self._collect())
        document = self._read(self.canonical_relative)
        projected = next(
            row
            for row in document["elements"]
            if row["id"] == MODULE.candidate_builder.CANDIDATE_ELEMENT_ID
        )
        projected["detail"]["color"]["multiply"] = [0.2, 0.4, 0.8, 0.6]
        projected["detail"]["trail"]["startWidth"] = 0.33
        self._write(self.canonical_relative, document)
        before = self._snapshot()
        projection = self._collect()
        self.assertTrue(projection.already_applied)
        MODULE.commit_projection(projection)
        self.assertEqual(before, self._snapshot())

        document = self._read(self.canonical_relative)
        projected = next(
            row
            for row in document["elements"]
            if row["id"] == MODULE.candidate_builder.CANDIDATE_ELEMENT_ID
        )
        projected["resources"][0]["assetId"] = (
            "Effect/Valtan/Textures/FX_TEX_02/fx_d_noise_002.dds"
        )
        self._write(self.canonical_relative, document)
        before = self._snapshot()
        with self.assertRaisesRegex(
            MODULE.SourceRebaseRequired, "immutable resource/material/anchor"
        ):
            self._collect()
        self.assertEqual(before, self._snapshot())

    def test_identity_collision_or_missing_receipt_fails_closed(self) -> None:
        canonical = self._read(self.canonical_relative)
        candidate = self._read(self.candidate_relative)["elements"][0]
        canonical["elements"].append(copy.deepcopy(candidate))
        self._write(self.canonical_relative, canonical)
        with self.assertRaisesRegex(
            MODULE.SourceRebaseRequired, "without its committed receipt"
        ):
            self._collect()

        self._copy(self.canonical_relative)
        canonical = self._read(self.canonical_relative)
        collision = copy.deepcopy(candidate)
        collision["sourceNode"] = "different"
        canonical["elements"].append(collision)
        self._write(self.canonical_relative, canonical)
        with self.assertRaisesRegex(MODULE.SourceRebaseRequired, "collided"):
            self._collect()

    def test_proof_drift_and_transaction_failure_mutate_zero_files(self) -> None:
        proof = self._read(self.proof_relative)
        proof["geometryProof"]["trailPointCount"] = 1
        self._write(self.proof_relative, proof)
        before = self._snapshot()
        with self.assertRaisesRegex(MODULE.SourceRebaseRequired, "stale"):
            self._collect()
        self.assertEqual(before, self._snapshot())

        proof = MODULE.proof_builder.build_proof(
            self.root, expected_resource_root=self.resource_root
        )
        self._write(self.proof_relative, proof)
        projection = self._collect()
        before = self._snapshot()
        with self.assertRaisesRegex(MODULE.ApplicationError, "rolled back"):
            MODULE.commit_projection(projection, failure_after_promote=1)
        self.assertEqual(before, self._snapshot())

    def test_receipt_schema_is_strict_and_matches_committed_projection(self) -> None:
        MODULE.commit_projection(self._collect())
        receipt = self._read(self.receipt_relative)
        schema = json.loads(
            (
                self.source_root
                / "Tools/EffectPipeline/Schemas/"
                "lostark.valtan-four-slash-weapon-trail-application-receipt.schema.json"
            ).read_text(encoding="utf-8")
        )
        self.assertFalse(schema["additionalProperties"])
        self.assertEqual(MODULE.RECEIPT_SCHEMA, receipt["schema"])
        try:
            import jsonschema
        except ImportError:
            return
        jsonschema.Draft202012Validator.check_schema(schema)
        jsonschema.validate(receipt, schema)


if __name__ == "__main__":
    unittest.main()
