#!/usr/bin/env python3
"""Tests for atomic proof-gated FRONT_BACK_FRONT source-wave application."""

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

import apply_valtan_front_back_front_source_waves as sut
import validate_boss_pattern_effects as schema_validator


SOURCE_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = Path(__file__).with_name("apply_valtan_front_back_front_source_waves.py")
FIXTURE_PROOF = PurePosixPath(
    "Data/Effects/Imported/Valtan/FrontBackFrontSourceWaves/"
    "fixture.front-back-front-source-wave-drawable-proof.v1.json"
)
PROOF_SCHEMA_PATH = Path(__file__).parent / "Schemas" / (
    "lostark.valtan-front-back-front-source-wave-drawable-proof.schema.json"
)
APPLICATION_SCHEMA_PATH = Path(__file__).parent / "Schemas" / (
    "lostark.valtan-front-back-front-source-wave-application-receipt.schema.json"
)
SAFE_REVIEWED_CUE_IDS = {
    "cue.valtan.floor-wipe-130.arena-wipe-impact",
    "cue.valtan.floor-wipe-130.arena-wipe-telegraph",
    "cue.valtan.floor-wipe-130.six-direction-impact",
    "cue.valtan.floor-wipe-130.six-direction-telegraph",
}


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
        self._temporary = tempfile.TemporaryDirectory(prefix="valtan-fbf-wave-apply-test.")
        self.root = Path(self._temporary.name)
        self.candidate_receipt = _read_json(
            SOURCE_ROOT, sut.DEFAULT_CANDIDATE_RECEIPT
        )
        candidate_outputs = {
            PurePosixPath(row["candidateDocumentPath"]): SOURCE_ROOT.joinpath(
                *PurePosixPath(row["candidateDocumentPath"]).parts
            ).read_bytes()
            for row in self.candidate_receipt["candidates"]
        }
        source_paths = {
            PurePosixPath(row["path"])
            for row in self.candidate_receipt["sourceGuards"]
        }
        source_paths.add(
            PurePosixPath(self.candidate_receipt["aggregateCanary"]["authoredDocumentPath"])
        )
        source_paths.add(
            PurePosixPath(self.candidate_receipt["whirlwindCanary"]["authoredDocumentPath"])
        )
        for relative in sorted(source_paths, key=PurePosixPath.as_posix):
            source = SOURCE_ROOT.joinpath(*relative.parts)
            if not source.is_file():
                continue
            target = self.root.joinpath(*relative.parts)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, target)
        self._normalize_to_preapply_state()
        self.candidate_receipt["sourceGuards"] = [
            guard
            for guard in self.candidate_receipt["sourceGuards"]
            if self.root.joinpath(*PurePosixPath(guard["path"]).parts).is_file()
        ]
        for guard in self.candidate_receipt["sourceGuards"]:
            relative = PurePosixPath(guard["path"])
            guard["sha256"] = _sha256(
                self.root.joinpath(*relative.parts).read_bytes()
            )
        candidate_outputs[sut.DEFAULT_CANDIDATE_RECEIPT] = sut._json_bytes(
            self.candidate_receipt
        )
        for relative, payload in candidate_outputs.items():
            target = self.root.joinpath(*relative.parts)
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(payload)
        self.proof_relative = FIXTURE_PROOF
        self.proof_path = self.root.joinpath(*FIXTURE_PROOF.parts)
        self.proof_path.parent.mkdir(parents=True, exist_ok=True)
        self.proof = self._build_proof()
        _write_json_like(self.proof_path, self.proof)

    def _normalize_to_preapply_state(self) -> None:
        effect_ids = {
            row["effectAssetId"] for row in self.candidate_receipt["candidates"]
        }
        binding_ids = {
            row["cueRow"]["bindingId"]
            for row in self.candidate_receipt["candidates"]
        }
        occurrence_ids = {
            row["cueRow"]["occurrenceId"]
            for row in self.candidate_receipt["candidates"]
        }
        catalog_path = self.root.joinpath(*sut.CATALOG_PATH.parts)
        catalog = json.loads(catalog_path.read_text(encoding="utf-8"))
        catalog["effects"] = [
            row
            for row in catalog["effects"]
            if row.get("effectAssetId") not in effect_ids
        ]
        _write_json_like(catalog_path, catalog)
        cue_path = self.root.joinpath(*sut.CUE_PATH.parts)
        cues = json.loads(cue_path.read_text(encoding="utf-8"))
        cues["cues"] = [
            row
            for row in cues["cues"]
            if row.get("bindingId") not in binding_ids
            and row.get("occurrenceId") not in occurrence_ids
        ]
        _write_json_like(cue_path, cues)
        for row in self.candidate_receipt["candidates"]:
            target = self.root.joinpath(
                *PurePosixPath(row["targetAuthoredDocumentPath"]).parts
            )
            if target.is_file():
                target.unlink()
        receipt = self.root.joinpath(*sut.DEFAULT_APPLICATION_RECEIPT.parts)
        if receipt.is_file():
            receipt.unlink()

    def close(self) -> None:
        self._temporary.cleanup()

    def _build_proof(self) -> dict:
        candidate_receipt_path = self.root.joinpath(*sut.DEFAULT_CANDIDATE_RECEIPT.parts)
        targets = []
        for row in self.candidate_receipt["candidates"]:
            document = _read_json(self.root, row["candidateDocumentPath"])
            targets.append(
                {
                    "effectAssetId": row["effectAssetId"],
                    "candidateDocumentSha256": row["candidateDocumentSha256"],
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
                        for element in document["elements"]
                    ],
                }
            )
        return {
            "schema": sut.PROOF_SCHEMA,
            "formatVersion": 1,
            "bossArchetypeId": sut.BOSS_ARCHETYPE_ID,
            "candidateReceiptSha256": _sha256(candidate_receipt_path.read_bytes()),
            "targets": targets,
        }

    def projection(self) -> sut.Projection:
        return sut.collect_projection(self.root, drawable_proof=self.proof_path)


class FrontBackFrontSourceWaveApplicatorTests(unittest.TestCase):
    def make_fixture(self) -> RepositoryFixture:
        fixture = RepositoryFixture()
        self.addCleanup(fixture.close)
        return fixture

    def test_dry_run_is_read_only_and_projects_six_canonical_outputs(self) -> None:
        fixture = self.make_fixture()
        before = _snapshot(fixture.root)
        projection = fixture.projection()
        self.assertEqual(before, _snapshot(fixture.root))
        self.assertEqual(6, len(projection.canonical_outputs))
        self.assertEqual(7, len(projection.changed_paths))
        self.assertEqual(5, len(projection.new_paths))
        closure = projection.receipt["closure"]
        self.assertEqual(4, closure["sourceWaveDocumentCount"])
        self.assertEqual(100, closure["sourceElementCount"])
        self.assertEqual(5, closure["allEffectsClipCueCount"])
        self.assertEqual(0, closure["aggregateSourceElementAppendCount"])
        self.assertEqual(0, closure["gameplayMutationCount"])
        with self.assertRaises(sut.ProjectionError):
            sut.check_projection(projection)

    def test_apply_adds_four_docs_catalog_cues_and_is_byte_idempotent(self) -> None:
        fixture = self.make_fixture()
        projection = fixture.projection()
        aggregate = projection.receipt["aggregateCanary"]
        aggregate_path = fixture.root.joinpath(*PurePosixPath(aggregate["authoredDocumentPath"]).parts)
        aggregate_before = aggregate_path.read_bytes()
        overlay_path = fixture.root.joinpath(*PurePosixPath(aggregate["projectOverlayPath"]).parts)
        overlay_before = overlay_path.read_bytes()
        whirlwind = projection.receipt["whirlwindCanary"]
        whirlwind_path = fixture.root.joinpath(*PurePosixPath(whirlwind["authoredDocumentPath"]).parts)
        whirlwind_before = whirlwind_path.read_bytes()
        bindings_row = next(
            row for row in fixture.candidate_receipt["sourceGuards"]
            if row["path"].endswith("Valtan.patternbindings.json")
        )
        bindings_path = fixture.root.joinpath(*PurePosixPath(bindings_row["path"]).parts)
        bindings_before = bindings_path.read_bytes()

        sut.commit_projection(projection)
        post = fixture.projection()
        sut.check_projection(post)
        catalog = _read_json(fixture.root, sut.CATALOG_PATH)
        catalog_by_id = {row["effectAssetId"]: row for row in catalog["effects"]}
        cues = _read_json(fixture.root, sut.CUE_PATH)
        clip_cues = sorted(
            [row for row in cues["cues"] if row["clipOccurrenceId"] == sut.CLIP_OCCURRENCE_ID],
            key=lambda row: row["sourceStartMs"],
        )
        self.assertEqual(
            [900, 1169, 2253, 3224, 4220],
            [row["sourceStartMs"] for row in clip_cues],
        )
        self.assertEqual(
            [sut.AGGREGATE_EFFECT_ID]
            + [row["effectAssetId"] for row in fixture.candidate_receipt["candidates"]],
            [row["effectAssetId"] for row in clip_cues],
        )
        pairs: list[tuple[str, str]] = []
        for target in projection.receipt["targets"]:
            document = _read_json(fixture.root, target["targetAuthoredDocumentPath"])
            self.assertEqual(25, len(document["elements"]))
            pairs.extend((row["id"], row["sourceNode"]) for row in document["elements"])
            self.assertEqual(target["catalogRow"], catalog_by_id[target["effectAssetId"]])
        self.assertEqual(100, len(pairs))
        self.assertEqual(100, len(set(pairs)))
        self.assertEqual(aggregate_before, aggregate_path.read_bytes())
        self.assertEqual(overlay_before, overlay_path.read_bytes())
        self.assertEqual(whirlwind_before, whirlwind_path.read_bytes())
        self.assertEqual(bindings_before, bindings_path.read_bytes())

        after_first = _snapshot(fixture.root)
        self.assertEqual((), post.changed_paths)
        sut.commit_projection(post)
        self.assertEqual(after_first, _snapshot(fixture.root))

    def test_current_repository_projection_check_is_a_no_op(self) -> None:
        proof = SOURCE_ROOT / (
            "Data/Effects/Imported/Valtan/FrontBackFrontSourceWaves/"
            "DrawableProof/Valtan.front-back-front-source-waves.drawable-proof.v1.json"
        )
        projection = sut.collect_projection(
            SOURCE_ROOT,
            drawable_proof=proof,
        )
        sut.check_projection(projection)
        self.assertEqual((), projection.changed_paths)
        cues = _read_json(SOURCE_ROOT, sut.CUE_PATH)["cues"]
        # Two combat-object-owned visuals are intentionally no longer boss-root
        # cue rows; the current Valtan boss-root contract is 106.
        self.assertGreaterEqual(len(cues), 106)
        self.assertTrue(
            SAFE_REVIEWED_CUE_IDS.issubset(
                {row["bindingId"] for row in cues}
            )
        )

    def test_later_unrelated_catalog_and_cue_rows_do_not_stale_fbf_rows(self) -> None:
        fixture = self.make_fixture()
        sut.commit_projection(fixture.projection())
        catalog_path = fixture.root.joinpath(*sut.CATALOG_PATH.parts)
        catalog = json.loads(catalog_path.read_text(encoding="utf-8"))
        catalog["effects"].append(
            {
                "effectAssetId": "effect.valtan.zzz-unrelated.test",
                "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
                "authoringPath": "Effects/Authored/unrelated.effect.json",
            }
        )
        catalog["effects"].sort(key=lambda row: row["effectAssetId"])
        _write_json_like(catalog_path, catalog)
        cues_path = fixture.root.joinpath(*sut.CUE_PATH.parts)
        cues = json.loads(cues_path.read_text(encoding="utf-8"))
        unrelated = deepcopy(cues["cues"][0])
        unrelated["bindingId"] = "cue.valtan.zzz-unrelated.test"
        unrelated["occurrenceId"] = "cue.valtan.zzz-unrelated.test.occurrence.01"
        unrelated["patternId"] = "VALTAN_ZZZ_UNRELATED_TEST"
        unrelated["actionId"] = "valtan.zzz-unrelated.test"
        unrelated["clipOccurrenceId"] = "valtan.zzz-unrelated.test.clip.01"
        unrelated["effectAssetId"] = "effect.valtan.zzz-unrelated.test"
        cues["cues"].append(unrelated)
        cues["cues"].sort(
            key=lambda row: (row["patternId"], row["actionId"], row["bindingId"])
        )
        _write_json_like(cues_path, cues)
        projection = fixture.projection()
        self.assertEqual(
            {sut.DEFAULT_APPLICATION_RECEIPT},
            set(projection.changed_paths),
        )

    def test_declared_historical_receipt_reseal_deep_preserves_all_canonical_outputs(self) -> None:
        fixture = self.make_fixture()
        first = fixture.projection()
        sut.commit_projection(first)
        protected_paths = set(first.canonical_outputs)
        before = {
            relative: fixture.root.joinpath(*relative.parts).read_bytes()
            for relative in protected_paths
        }

        candidate_path = fixture.root.joinpath(*sut.DEFAULT_CANDIDATE_RECEIPT.parts)
        candidate = json.loads(candidate_path.read_text(encoding="utf-8"))
        candidate["sourceGuards"] = list(reversed(candidate["sourceGuards"]))
        _write_json_like(candidate_path, candidate)
        proof = json.loads(fixture.proof_path.read_text(encoding="utf-8"))
        proof["candidateReceiptSha256"] = _sha256(candidate_path.read_bytes())
        _write_json_like(fixture.proof_path, proof)

        default_proof_path = fixture.root.joinpath(*sut.DEFAULT_DRAWABLE_PROOF.parts)
        default_proof_path.parent.mkdir(parents=True, exist_ok=True)
        default_proof_path.write_bytes(fixture.proof_path.read_bytes())
        prior_path = fixture.root.joinpath(*sut.DEFAULT_APPLICATION_RECEIPT.parts)
        prior = json.loads(prior_path.read_text(encoding="utf-8"))
        prior["inputs"]["drawableProofPath"] = sut.DEFAULT_DRAWABLE_PROOF.as_posix()
        prior["inputs"]["candidateReceiptSha256"] = (
            sut.HISTORICAL_CANDIDATE_RECEIPT_SHA256
        )
        prior["inputs"]["drawableProofSha256"] = (
            sut.HISTORICAL_DRAWABLE_PROOF_SHA256
        )
        _write_json_like(prior_path, prior)

        resealed = sut.collect_projection(
            fixture.root, drawable_proof=default_proof_path
        )
        self.assertEqual(
            {sut.DEFAULT_APPLICATION_RECEIPT}, set(resealed.changed_paths)
        )
        sut.commit_projection(resealed)
        self.assertEqual(
            before,
            {
                relative: fixture.root.joinpath(*relative.parts).read_bytes()
                for relative in protected_paths
            },
        )
        sut.check_projection(
            sut.collect_projection(
                fixture.root, drawable_proof=default_proof_path
            )
        )

    def test_prior_receipt_tampering_cannot_masquerade_as_composition_reseal(self) -> None:
        mutations = (
            (
                "input lineage",
                lambda receipt: receipt["inputs"].__setitem__(
                    "candidateReceiptSha256", "0" * 64
                ),
            ),
            (
                "contract drift",
                lambda receipt: receipt.__setitem__("policy", {"corrupted": True}),
            ),
            (
                "contract drift",
                lambda receipt: receipt.__setitem__("closure", {"corrupted": True}),
            ),
            (
                "target drift",
                lambda receipt: receipt["targets"][0].__setitem__(
                    "candidateDocumentSha256", "1" * 64
                ),
            ),
        )
        for expected, mutate in mutations:
            with self.subTest(expected=expected):
                fixture = self.make_fixture()
                sut.commit_projection(fixture.projection())
                receipt_path = fixture.root.joinpath(
                    *sut.DEFAULT_APPLICATION_RECEIPT.parts
                )
                receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
                mutate(receipt)
                _write_json_like(receipt_path, receipt)
                with self.assertRaisesRegex(sut.SourceRebaseRequired, expected):
                    fixture.projection()

    def test_hand_tuned_wave_element_is_deep_preserved(self) -> None:
        fixture = self.make_fixture()
        first = fixture.projection()
        sut.commit_projection(first)
        target = first.receipt["targets"][0]
        path = fixture.root.joinpath(*PurePosixPath(target["targetAuthoredDocumentPath"]).parts)
        document = json.loads(path.read_text(encoding="utf-8"))
        document["elements"][0]["displayName"] = "hand-tuned-source-wave"
        document["elements"][0]["detail"]["color"]["multiply"] = [0.4, 0.5, 0.6, 0.7]
        tuned = deepcopy(document["elements"][0])
        _write_json_like(path, document)
        sut.commit_projection(fixture.projection())
        final = json.loads(path.read_text(encoding="utf-8"))
        self.assertEqual(tuned, final["elements"][0])

    def test_failed_or_incomplete_drawable_proof_is_fail_closed(self) -> None:
        fixture = self.make_fixture()
        proof = deepcopy(fixture.proof)
        proof["targets"][0]["elements"][0]["failedDraws"] = 1
        _write_json_like(fixture.proof_path, proof)
        before = _snapshot(fixture.root)
        with self.assertRaises(sut.ProjectionError):
            fixture.projection()
        self.assertEqual(before, _snapshot(fixture.root))
        fixture.proof_path.unlink()
        missing_before = _snapshot(fixture.root)
        with self.assertRaises(sut.ProjectionError):
            fixture.projection()
        self.assertEqual(missing_before, _snapshot(fixture.root))

    def test_aggregate_and_whirlwind_canary_drift_require_rebase(self) -> None:
        fixture = self.make_fixture()
        aggregate = fixture.candidate_receipt["aggregateCanary"]
        aggregate_path = fixture.root.joinpath(*PurePosixPath(aggregate["authoredDocumentPath"]).parts)
        aggregate_document = json.loads(aggregate_path.read_text(encoding="utf-8"))
        aggregate_document["effectAssetId"] = "effect.valtan.front-back-front.drifted"
        _write_json_like(aggregate_path, aggregate_document)
        before = _snapshot(fixture.root)
        with self.assertRaisesRegex(sut.SourceRebaseRequired, "aggregate authored identity"):
            fixture.projection()
        self.assertEqual(before, _snapshot(fixture.root))

        second = self.make_fixture()
        whirlwind = second.candidate_receipt["whirlwindCanary"]
        whirlwind_path = second.root.joinpath(*PurePosixPath(whirlwind["authoredDocumentPath"]).parts)
        whirlwind_path.write_bytes(whirlwind_path.read_bytes() + b"\n")
        before_second = _snapshot(second.root)
        with self.assertRaisesRegex(sut.SourceRebaseRequired, "Whirlwind"):
            second.projection()
        self.assertEqual(before_second, _snapshot(second.root))

    def test_catalog_or_cue_partial_preexistence_requires_rebase(self) -> None:
        fixture = self.make_fixture()
        candidate = fixture.candidate_receipt["candidates"][0]
        catalog_path = fixture.root.joinpath(*sut.CATALOG_PATH.parts)
        catalog = json.loads(catalog_path.read_text(encoding="utf-8"))
        bad = deepcopy(candidate["catalogRow"])
        bad["payloadKind"] = "DRIFTED"
        catalog["effects"].append(bad)
        catalog["effects"].sort(key=lambda row: row["effectAssetId"])
        _write_json_like(catalog_path, catalog)
        before = _snapshot(fixture.root)
        with self.assertRaisesRegex(sut.SourceRebaseRequired, "baseline drift"):
            fixture.projection()
        self.assertEqual(before, _snapshot(fixture.root))

    def test_cross_wave_duplicate_source_element_requires_rebase(self) -> None:
        fixture = self.make_fixture()
        projection = fixture.projection()
        sut.commit_projection(projection)
        first, second = projection.receipt["targets"][:2]
        first_path = fixture.root.joinpath(
            *PurePosixPath(first["targetAuthoredDocumentPath"]).parts
        )
        second_document = _read_json(
            fixture.root, second["targetAuthoredDocumentPath"]
        )
        first_document = json.loads(first_path.read_text(encoding="utf-8"))
        first_document["elements"].append(deepcopy(second_document["elements"][0]))
        _write_json_like(first_path, first_document)
        before = _snapshot(fixture.root)
        with self.assertRaisesRegex(sut.SourceRebaseRequired, "partition drift"):
            fixture.projection()
        self.assertEqual(before, _snapshot(fixture.root))

    def test_injected_failure_rolls_back_catalog_cues_docs_and_receipt(self) -> None:
        fixture = self.make_fixture()
        projection = fixture.projection()
        before = _snapshot(fixture.root)
        with self.assertRaisesRegex(sut.ProjectionError, "rolled back"):
            sut.commit_projection(projection, failure_after_promote=3)
        self.assertEqual(before, _snapshot(fixture.root))

    def test_staged_guard_drift_blocks_transaction_before_writes(self) -> None:
        fixture = self.make_fixture()
        projection = fixture.projection()
        cue_path = fixture.root.joinpath(*sut.CUE_PATH.parts)
        cue_path.write_bytes(cue_path.read_bytes() + b"\n")
        before = _snapshot(fixture.root)
        with self.assertRaisesRegex(sut.ProjectionError, "transaction not started"):
            sut.commit_projection(projection)
        self.assertEqual(before, _snapshot(fixture.root))

    def test_cli_modes_and_no_proof_apply_remain_read_only(self) -> None:
        fixture = self.make_fixture()
        base = [sys.executable, str(SCRIPT_PATH), "--repo-root", str(fixture.root)]
        proof = ["--drawable-proof", str(fixture.proof_path)]
        before = _snapshot(fixture.root)
        dry = subprocess.run([*base, "--dry-run", *proof], capture_output=True, text=True, check=False)
        self.assertEqual(0, dry.returncode, dry.stderr)
        self.assertEqual(before, _snapshot(fixture.root))
        stale = subprocess.run([*base, "--check", *proof], capture_output=True, text=True, check=False)
        self.assertEqual(1, stale.returncode)
        applied = subprocess.run([*base, "--apply", *proof], capture_output=True, text=True, check=False)
        self.assertEqual(0, applied.returncode, applied.stderr)
        checked = subprocess.run([*base, "--check", *proof], capture_output=True, text=True, check=False)
        self.assertEqual(0, checked.returncode, checked.stderr)

        no_proof = self.make_fixture()
        no_proof_before = _snapshot(no_proof.root)
        result = subprocess.run(
            [sys.executable, str(SCRIPT_PATH), "--repo-root", str(no_proof.root), "--apply"],
            capture_output=True,
            text=True,
            check=False,
        )
        self.assertNotEqual(0, result.returncode)
        self.assertEqual(no_proof_before, _snapshot(no_proof.root))

    def test_proof_and_application_receipt_validate_against_schemas(self) -> None:
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
