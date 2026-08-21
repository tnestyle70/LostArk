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


SCRIPT_ROOT = Path(__file__).resolve().parent


def _load(name: str, filename: str):
    path = SCRIPT_ROOT / filename
    specification = importlib.util.spec_from_file_location(name, path)
    if specification is None or specification.loader is None:
        raise RuntimeError(f"could not load {path}")
    module = importlib.util.module_from_spec(specification)
    sys.modules[name] = module
    specification.loader.exec_module(module)
    return module


BUILDER = _load(
    "build_valtan_bounded_weapon_trail_candidates_tests",
    "build_valtan_bounded_weapon_trail_candidates.py",
)
PROOF = _load(
    "build_valtan_bounded_weapon_trail_drawable_proof_tests",
    "build_valtan_bounded_weapon_trail_drawable_proof.py",
)
APPLY = _load(
    "apply_valtan_bounded_weapon_trails_tests",
    "apply_valtan_bounded_weapon_trails.py",
)
SCHEMA_VALIDATOR = _load(
    "validate_boss_pattern_effects_for_bounded_weapon_trail_tests",
    "validate_boss_pattern_effects.py",
)

SCHEMA_PATHS = {
    "manifest": SCRIPT_ROOT / "Schemas" / (
        "lostark.valtan-bounded-weapon-trail-candidate-manifest.schema.json"
    ),
    "sweep": SCRIPT_ROOT / "Schemas" / (
        "lostark.valtan-bounded-weapon-trail-runtime-sweep.schema.json"
    ),
    "proof": SCRIPT_ROOT / "Schemas" / (
        "lostark.valtan-bounded-weapon-trail-drawable-proof.schema.json"
    ),
    "application": SCRIPT_ROOT / "Schemas" / (
        "lostark.valtan-bounded-weapon-trail-application-receipt.schema.json"
    ),
}


class BoundedWeaponTrailTests(unittest.TestCase):
    def setUp(self) -> None:
        self.source_root = SCRIPT_ROOT.parents[1]
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.resource_root = self.root / "Client/Bin/Resources"
        required = {
            BUILDER.SOURCE_DOCUMENT_RELATIVE_PATH,
            BUILDER.PATTERN_BINDINGS_RELATIVE_PATH,
            BUILDER.PATTERN_CUES_RELATIVE_PATH,
            BUILDER.EFFECT_CATALOG_RELATIVE_PATH,
            BUILDER.ADAPTER_RELATIVE_PATH,
            PurePosixPath("Client/Bin/Resources") / BUILDER.BODY_MODEL_ASSET_ID,
        }
        required.update(target.canonical_relative_path for target in BUILDER.TARGETS)
        required.update(
            PurePosixPath("Client/Bin/Resources") / asset_id
            for asset_id in BUILDER.EXPECTED_RESOURCE_EVIDENCE
        )
        for relative in sorted(required):
            self._copy(relative)
        # The repository carries the committed projection.  Each isolated
        # fixture reconstructs the exact pre-apply baseline so candidate and
        # transaction tests do not inherit an orphaned downstream suffix
        # without its application receipt.
        for target in BUILDER.TARGETS:
            document = self._read(target.canonical_relative_path)
            projected_ids = set(BUILDER.candidate_element_ids(target))
            document["elements"] = [
                row
                for row in document["elements"]
                if row.get("id") not in projected_ids
            ]
            self._write(target.canonical_relative_path, document)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def _path(self, relative: PurePosixPath) -> Path:
        return self.root.joinpath(*relative.parts)

    def _copy(self, relative: PurePosixPath) -> None:
        source = self.source_root.joinpath(*relative.parts)
        destination = self._path(relative)
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, destination)

    def _read(self, relative: PurePosixPath) -> dict:
        return json.loads(self._path(relative).read_text(encoding="utf-8"))

    def _write(self, relative: PurePosixPath, value: dict) -> None:
        path = self._path(relative)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(BUILDER.pretty_json_bytes(value))

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

    def _write_candidates_and_sweep(self) -> None:
        outputs = BUILDER.build_outputs(self.root)
        BUILDER._write_outputs(self.root, outputs)
        targets = []
        for target in BUILDER.TARGETS:
            candidate_relative = BUILDER.OUTPUT_ROOT / PurePosixPath(
                target.candidate_filename
            )
            candidate_path = self._path(candidate_relative).resolve()
            candidate = self._read(candidate_relative)
            elements = []
            for element in candidate["elements"]:
                elements.append(
                    {
                        "elementId": element["id"],
                        "positiveMovingBone": {
                            "providerSampleCount": 12,
                            "rootWorldDistinctCount": 1,
                            "anchorWorldDistinctCount": 8,
                            "trailPointCount": 7,
                            "distinctTrailPointCount": 7,
                            "finiteTrailPointCount": 7,
                            "cumulativeDistance": 0.3,
                            "firstWorldPosition": [0.0, 0.0, 0.0],
                            "lastWorldPosition": [0.3, 0.0, 0.0],
                            "renderer": self._renderer(True),
                        },
                        "stationaryControl": {
                            "providerSampleCount": 12,
                            "rootWorldDistinctCount": 1,
                            "anchorWorldDistinctCount": 1,
                            "trailPointCount": 1,
                            "distinctTrailPointCount": 1,
                            "renderer": self._renderer(False),
                            "segmentSuppressed": True,
                        },
                    }
                )
            targets.append(
                {
                    "effectAssetId": target.effect_asset_id,
                    "candidatePath": str(candidate_path),
                    "candidateRawSha256": BUILDER.raw_sha256(candidate_path),
                    "candidateTypedCodecSha256": "a" * 64,
                    "runtimeAnchorSlotId": target.runtime_anchor_slot_id,
                    "sourceStartSeconds": target.source_time_seconds,
                    "sourceDurationSeconds": target.source_duration_seconds,
                    "elements": elements,
                    "missingAnchorControl": {
                        "providerRejected": True,
                        "playbackStatePreserved": True,
                        "trailPointCountAfterReject": 0,
                        "status": (
                            "Effect transform history is missing required anchor: "
                            + target.runtime_anchor_slot_id
                        ),
                    },
                    "disposition": "DRAWABLE_PROOF_PASS",
                }
            )
        sweep = {
            "schema": PROOF.SWEEP_SCHEMA,
            "formatVersion": 1,
            "bossArchetypeId": "BOSS_VALTAN",
            "classification": "PROJECT_TUNED",
            "reconstructionPolicy": "BOUNDED_RECONSTRUCTION",
            "resourceRoot": str(self.resource_root.resolve()),
            "runtimeBoneName": BUILDER.RUNTIME_BONE_NAME,
            "sampleRateHz": 60,
            "targets": targets,
            "summary": {
                "targetCount": 3,
                "elementCount": 9,
                "movingBoneDrawCount": 9,
                "stationarySuppressedCount": 9,
                "missingAnchorRollbackCount": 3,
            },
            "disposition": "DRAWABLE_PROOF_PASS",
        }
        self._write(PROOF.SWEEP_RELATIVE_PATH, sweep)
        proof = PROOF.build_proof(
            self.root, expected_resource_root=self.resource_root
        )
        self._write(PROOF.PROOF_RELATIVE_PATH, proof)

    def _snapshot(self) -> dict[str, bytes]:
        return {
            path.relative_to(self.root).as_posix(): path.read_bytes()
            for path in sorted(self.root.rglob("*"))
            if path.is_file()
        }

    def test_candidate_denominator_and_exact_family_closure(self) -> None:
        outputs = BUILDER.build_outputs(self.root)
        self.assertEqual(4, len(outputs))
        source = self._read(BUILDER.SOURCE_DOCUMENT_RELATIVE_PATH)
        source_by_id = {row["id"]: row for row in source["elements"]}
        for target in BUILDER.TARGETS:
            relative = BUILDER.OUTPUT_ROOT / PurePosixPath(
                target.candidate_filename
            )
            candidate = json.loads(outputs[relative])
            self.assertEqual(3, len(candidate["elements"]))
            for element, source_contract in zip(
                candidate["elements"], BUILDER.SOURCE_ELEMENTS, strict=True
            ):
                source_element = source_by_id[source_contract[0]]
                self.assertTrue(element["visible"])
                self.assertEqual("trail", element["kind"])
                self.assertEqual(source_element["resources"], element["resources"])
                self.assertEqual(source_element["material"], element["material"])
                self.assertEqual(
                    source_element["detail"]["trail"],
                    element["detail"]["trail"],
                )
                self.assertTrue(element["actionCueAttachment"]["follow"])
                self.assertEqual(
                    target.runtime_anchor_slot_id,
                    element["actionCueAttachment"]["runtimeAnchorSlotId"],
                )
                self.assertEqual(
                    target.source_time_seconds,
                    element["detail"]["timing"]["startDelaySeconds"],
                )
                self.assertEqual(
                    target.source_duration_seconds,
                    element["detail"]["timing"]["lifeTimeSeconds"],
                )
                self.assertFalse(element["sourceRecipe"]["enabled"])
        changed = BUILDER._write_outputs(self.root, outputs)
        self.assertEqual(4, changed)
        self.assertEqual(
            0, BUILDER._check_outputs(self.root, BUILDER.build_outputs(self.root))
        )
        canary = self._path(BUILDER.SOURCE_DOCUMENT_RELATIVE_PATH)
        self.assertEqual(
            BUILDER.SOURCE_DOCUMENT_RAW_SHA256, BUILDER.raw_sha256(canary)
        )

    def test_all_four_artifacts_validate_against_strict_draft_2020_schemas(
        self,
    ) -> None:
        self._write_candidates_and_sweep()
        projection = APPLY.collect_projection(self.root)
        APPLY.commit_projection(projection)
        artifact_paths = {
            "manifest": BUILDER.OUTPUT_ROOT
            / PurePosixPath(BUILDER.MANIFEST_FILENAME),
            "sweep": PROOF.SWEEP_RELATIVE_PATH,
            "proof": PROOF.PROOF_RELATIVE_PATH,
            "application": APPLY.RECEIPT_RELATIVE_PATH,
        }
        nested_mutators = {
            "manifest": lambda document: document["sourceFamily"].__setitem__(
                "unexpected", True
            ),
            "sweep": lambda document: document["targets"][0].__setitem__(
                "unexpected", True
            ),
            "proof": lambda document: document["targets"][0]["elements"][
                0
            ].__setitem__("unexpected", True),
            "application": lambda document: document["targets"][0].__setitem__(
                "unexpected", True
            ),
        }
        for label, relative in artifact_paths.items():
            with self.subTest(artifact=label):
                document = self._read(relative)
                schema = json.loads(
                    SCHEMA_PATHS[label].read_text(encoding="utf-8")
                )
                self.assertEqual(
                    "https://json-schema.org/draft/2020-12/schema",
                    schema["$schema"],
                )
                SCHEMA_VALIDATOR.validate_schema_instance(document, schema)
                unexpected_root = copy.deepcopy(document)
                unexpected_root["unexpected"] = True
                with self.assertRaises(SCHEMA_VALIDATOR.ContractError):
                    SCHEMA_VALIDATOR.validate_schema_instance(
                        unexpected_root, schema
                    )
                unexpected_nested = copy.deepcopy(document)
                nested_mutators[label](unexpected_nested)
                with self.assertRaises(SCHEMA_VALIDATOR.ContractError):
                    SCHEMA_VALIDATOR.validate_schema_instance(
                        unexpected_nested, schema
                    )

    def test_candidate_fails_closed_on_cue_adapter_or_source_drift(self) -> None:
        cues = self._read(BUILDER.PATTERN_CUES_RELATIVE_PATH)
        cue = next(
            row
            for row in cues["cues"]
            if row["bindingId"] == BUILDER.TARGETS[0].cue_binding_id
        )
        cue["sourceStartMs"] = 1
        self._write(BUILDER.PATTERN_CUES_RELATIVE_PATH, cues)
        with self.assertRaisesRegex(BUILDER.CandidateError, "Product cue"):
            BUILDER.build_outputs(self.root)

        self._copy(BUILDER.PATTERN_CUES_RELATIVE_PATH)
        adapters = self._read(BUILDER.ADAPTER_RELATIVE_PATH)
        adapter = next(
            row
            for row in adapters["adapters"]
            if row["adapterTargetId"] == BUILDER.TARGETS[1].adapter_target_id
        )
        adapter["sourceIdentity"]["sourceTimeSeconds"] += 0.01
        self._write(BUILDER.ADAPTER_RELATIVE_PATH, adapters)
        with self.assertRaisesRegex(BUILDER.CandidateError, "stale"):
            BUILDER.build_outputs(self.root)

        self._copy(BUILDER.ADAPTER_RELATIVE_PATH)
        source = self._read(BUILDER.SOURCE_DOCUMENT_RELATIVE_PATH)
        source["elements"][0]["displayName"] += " drift"
        self._write(BUILDER.SOURCE_DOCUMENT_RELATIVE_PATH, source)
        with self.assertRaisesRegex(BUILDER.CandidateError, "canary"):
            BUILDER.build_outputs(self.root)

    def test_proof_requires_all_moving_stationary_and_missing_controls(self) -> None:
        self._write_candidates_and_sweep()
        proof = PROOF.build_proof(
            self.root, expected_resource_root=self.resource_root
        )
        PROOF.validate_proof(proof)
        self.assertEqual(9, proof["summary"]["movingBoneDrawableCount"])

        sweep = self._read(PROOF.SWEEP_RELATIVE_PATH)
        sweep["targets"][0]["elements"][0]["positiveMovingBone"][
            "trailPointCount"
        ] = 1
        self._write(PROOF.SWEEP_RELATIVE_PATH, sweep)
        with self.assertRaisesRegex(PROOF.ProofError, "moving-bone"):
            PROOF.build_proof(self.root, expected_resource_root=self.resource_root)

        self._write_candidates_and_sweep()
        sweep = self._read(PROOF.SWEEP_RELATIVE_PATH)
        sweep["targets"][1]["elements"][1]["stationaryControl"]["renderer"][
            "submittedDraws"
        ] = 1
        self._write(PROOF.SWEEP_RELATIVE_PATH, sweep)
        with self.assertRaisesRegex(PROOF.ProofError, "stationary"):
            PROOF.build_proof(self.root, expected_resource_root=self.resource_root)

        self._write_candidates_and_sweep()
        sweep = self._read(PROOF.SWEEP_RELATIVE_PATH)
        sweep["targets"][2]["missingAnchorControl"][
            "playbackStatePreserved"
        ] = False
        self._write(PROOF.SWEEP_RELATIVE_PATH, sweep)
        with self.assertRaisesRegex(PROOF.ProofError, "rollback"):
            PROOF.build_proof(self.root, expected_resource_root=self.resource_root)

    def test_missing_only_apply_is_atomic_idempotent_and_deep_preserving(self) -> None:
        self._write_candidates_and_sweep()
        before_canary = self._path(BUILDER.SOURCE_DOCUMENT_RELATIVE_PATH).read_bytes()
        before_documents = {
            target.canonical_relative_path: self._read(target.canonical_relative_path)
            for target in BUILDER.TARGETS
        }
        projection = APPLY.collect_projection(self.root)
        self.assertFalse(projection.already_applied)
        self.assertEqual(4, len(projection.changed_paths))
        APPLY.commit_projection(projection)
        committed = APPLY.collect_projection(self.root)
        APPLY.check_projection(committed)
        self.assertTrue(committed.already_applied)
        self.assertEqual(before_canary, self._path(BUILDER.SOURCE_DOCUMENT_RELATIVE_PATH).read_bytes())
        for target in BUILDER.TARGETS:
            current = self._read(target.canonical_relative_path)
            before = before_documents[target.canonical_relative_path]
            self.assertEqual(before["elements"], current["elements"][: len(before["elements"])])
            self.assertEqual(
                list(BUILDER.candidate_element_ids(target)),
                [row["id"] for row in current["elements"][len(before["elements"]):]],
            )

        tuned_target = BUILDER.TARGETS[0]
        tuned = self._read(tuned_target.canonical_relative_path)
        projected = next(
            row
            for row in tuned["elements"]
            if row["id"] == BUILDER.candidate_element_ids(tuned_target)[0]
        )
        projected["detail"]["color"]["multiply"] = [0.2, 0.4, 0.8, 0.6]
        tuned["elements"][0]["detail"]["color"]["multiply"] = [0.9, 0.8, 0.7, 1.0]
        self._write(tuned_target.canonical_relative_path, tuned)
        tuned_bytes = self._path(tuned_target.canonical_relative_path).read_bytes()
        APPLY.commit_projection(APPLY.collect_projection(self.root))
        self.assertEqual(tuned_bytes, self._path(tuned_target.canonical_relative_path).read_bytes())

    def test_transaction_failure_rolls_back_all_files_and_proof_drift_mutates_none(self) -> None:
        self._write_candidates_and_sweep()
        projection = APPLY.collect_projection(self.root)
        before = self._snapshot()
        with self.assertRaisesRegex(APPLY.ApplicationError, "rolled back"):
            APPLY.commit_projection(projection, failure_after_promote=2)
        self.assertEqual(before, self._snapshot())

        proof = self._read(PROOF.PROOF_RELATIVE_PATH)
        proof["summary"]["movingBoneDrawableCount"] = 8
        self._write(PROOF.PROOF_RELATIVE_PATH, proof)
        before = self._snapshot()
        with self.assertRaisesRegex(APPLY.SourceRebaseRequired, "proof"):
            APPLY.collect_projection(self.root)
        self.assertEqual(before, self._snapshot())


if __name__ == "__main__":
    unittest.main()
