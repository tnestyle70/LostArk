#!/usr/bin/env python3
from __future__ import annotations

from copy import deepcopy
from contextlib import redirect_stderr, redirect_stdout
import hashlib
import importlib.util
import io
import json
from pathlib import Path, PurePosixPath
import shutil
import sys
import tempfile
import unittest


SCRIPT_PATH = Path(__file__).with_name(
    "apply_valtan_project_authored_priority_overlays.py"
)
SPEC = importlib.util.spec_from_file_location(
    "apply_valtan_project_authored_priority_overlays", SCRIPT_PATH
)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError(f"cannot load {SCRIPT_PATH}")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)

SAFE_REVIEWED_CUE_IDS = {
    "cue.valtan.floor-wipe-130.arena-wipe-impact",
    "cue.valtan.floor-wipe-130.arena-wipe-telegraph",
    "cue.valtan.floor-wipe-130.six-direction-impact",
    "cue.valtan.floor-wipe-130.six-direction-telegraph",
}


class ValtanPriorityProjectionTests(unittest.TestCase):
    def setUp(self) -> None:
        self.source_root = SCRIPT_PATH.resolve().parents[2]
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.patch_relative = MODULE.DEFAULT_PATCH_PLAN
        self.receipt_relative = MODULE.DEFAULT_RECEIPT
        self.sweep_relative = PurePosixPath(
            "Data/Effects/Imported/Valtan/ProjectAuthoredPriority/DrawableProof/"
            "Valtan.project-authored-priority.drawable-sweep.test.json"
        )
        self.proof_relative = PurePosixPath(
            "Data/Effects/Imported/Valtan/ProjectAuthoredPriority/DrawableProof/"
            "Valtan.project-authored-priority.drawable-proof.test.json"
        )
        self._copy(self.patch_relative)
        self.plan = self._read_json(self.patch_relative)
        for target in self.plan["targets"]:
            self._copy(PurePosixPath(target["overlayDocumentPath"]))
            target_relative = PurePosixPath(target["targetAuthoringPath"])
            if target["targetEffectAssetId"] != MODULE.HIGH_JUMP_EFFECT_ID:
                self._copy(target_relative)
        self._copy(MODULE.CATALOG_PATH)
        self._copy(MODULE.CUE_PATH)
        self._copy(MODULE.PATTERN_BINDINGS_PATH)
        for relative in MODULE.WHIRLWIND_REQUIRED_FILES:
            self._copy(relative)
        self._normalize_to_preapply_state()
        self.resource_root = self.root / "runtime-resources"
        self.resource_root.mkdir()
        self._write_json(self.sweep_relative, self._build_drawable_sweep())
        self._write_json(self.proof_relative, self._build_drawable_proof())

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def _path(self, relative: PurePosixPath) -> Path:
        return self.root.joinpath(*relative.parts)

    def _source_path(self, relative: PurePosixPath) -> Path:
        return self.source_root.joinpath(*relative.parts)

    def _copy(self, relative: PurePosixPath) -> None:
        destination = self._path(relative)
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(self._source_path(relative), destination)

    def _read_json(self, relative: PurePosixPath) -> dict:
        return json.loads(self._path(relative).read_text(encoding="utf-8"))

    def _write_json(self, relative: PurePosixPath, value: dict) -> None:
        path = self._path(relative)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(MODULE._json_bytes(value))

    def _normalize_to_preapply_state(self) -> None:
        """Remove this projection only; preserve every unrelated source row."""
        for target in self.plan["targets"]:
            target_relative = PurePosixPath(target["targetAuthoringPath"])
            target_path = self._path(target_relative)
            if not target_path.is_file():
                continue
            overlay = self._read_json(
                PurePosixPath(target["overlayDocumentPath"])
            )
            projected_ids = {row["id"] for row in overlay["elements"]}
            document = self._read_json(target_relative)
            document["elements"] = [
                row
                for row in document["elements"]
                if row.get("id") not in projected_ids
            ]
            self._write_json(target_relative, document)

        catalog = self._read_json(MODULE.CATALOG_PATH)
        catalog["effects"] = [
            row
            for row in catalog["effects"]
            if row.get("effectAssetId") != MODULE.HIGH_JUMP_EFFECT_ID
        ]
        self._write_json(MODULE.CATALOG_PATH, catalog)
        cues = self._read_json(MODULE.CUE_PATH)
        cues["cues"] = [
            row
            for row in cues["cues"]
            if row.get("bindingId") != MODULE.HIGH_JUMP_CUE_ROW["bindingId"]
            and row.get("occurrenceId")
            != MODULE.HIGH_JUMP_CUE_ROW["occurrenceId"]
        ]
        self._write_json(MODULE.CUE_PATH, cues)
        receipt = self._path(self.receipt_relative)
        if receipt.is_file():
            receipt.unlink()

    def _build_drawable_sweep(self) -> dict:
        documents = []
        for target in self.plan["targets"]:
            overlay_relative = PurePosixPath(target["overlayDocumentPath"])
            overlay = self._read_json(overlay_relative)
            elements = [
                {
                    "elementId": element["id"],
                    "disposition": "DRAWABLE_PROOF_PASS",
                    "preparedSamples": 2,
                    "attemptedSamples": 2,
                    "submittedDraws": 1,
                    "suppressedDraws": 0,
                    "failedDraws": 0,
                    "committedDraws": 1,
                }
                for element in overlay["elements"]
            ]
            documents.append(
                {
                    "documentPath": self._path(overlay_relative).resolve().as_posix(),
                    "effectAssetId": target["targetEffectAssetId"],
                    "durationSeconds": 1.0,
                    "sampleCount": 3,
                    "visibleElementCount": len(elements),
                    "preparedElementCount": len(elements),
                    "drawnElementCount": len(elements),
                    "disposition": "DRAWABLE_PROOF_PASS",
                    "elements": elements,
                }
            )
        return {
            "schema": "lostark.effect-document-drawable-sweep",
            "formatVersion": 1,
            "resourceRoot": self.resource_root.resolve().as_posix(),
            "sampleRateHz": 60,
            "documents": documents,
        }

    def _build_drawable_proof(self) -> dict:
        patch_payload = self._path(self.patch_relative).read_bytes()
        sweep_payload = self._path(self.sweep_relative).read_bytes()
        sweep = self._read_json(self.sweep_relative)
        targets = []
        for target, sweep_document in zip(
            self.plan["targets"], sweep["documents"], strict=True
        ):
            targets.append(
                {
                    "effectAssetId": target["targetEffectAssetId"],
                    "overlayDocumentSha256": target["overlayDocumentSha256"],
                    "disposition": "DRAWABLE_PROOF_PASS",
                    "elements": deepcopy(sweep_document["elements"]),
                }
            )
        return {
            "schema": MODULE.PROOF_SCHEMA,
            "formatVersion": 1,
            "ownerArchetypeId": MODULE.OWNER_ARCHETYPE_ID,
            "patchPlanSha256": hashlib.sha256(patch_payload).hexdigest(),
            "drawableSweepPath": self.sweep_relative.as_posix(),
            "drawableSweepSha256": hashlib.sha256(sweep_payload).hexdigest(),
            "resourceRoot": self.resource_root.resolve().as_posix(),
            "targets": targets,
        }

    def _collect(self):
        return MODULE.collect_projection(
            self.root,
            patch_plan=self.patch_relative,
            drawable_proof=self._path(self.proof_relative),
            receipt_path=self.receipt_relative,
        )

    def _snapshot(self) -> dict[str, bytes]:
        return {
            path.relative_to(self.root).as_posix(): path.read_bytes()
            for path in sorted(self.root.rglob("*"))
            if path.is_file()
        }

    def test_dry_run_apply_check_and_second_apply_are_idempotent(self) -> None:
        before = self._snapshot()
        projection = self._collect()
        self.assertEqual(before, self._snapshot(), "dry-run staging mutated files")
        self.assertEqual(9, projection.receipt["closure"]["candidateDocumentCount"])
        self.assertEqual(
            3,
            projection.receipt["closure"]["officialAxePresentationCount"],
        )
        expected_elements = sum(
            len(self._read_json(PurePosixPath(target["overlayDocumentPath"]))["elements"])
            for target in self.plan["targets"]
        )
        self.assertEqual(
            expected_elements,
            projection.receipt["closure"]["projectedElementCount"],
        )
        self.assertEqual(11, len(projection.canonical_outputs))
        with self.assertRaisesRegex(MODULE.ProjectionError, "not applied"):
            MODULE.check_projection(projection)
        self.assertEqual(before, self._snapshot(), "check-before-apply mutated files")

        whirlwind_files_before = {
            relative: self._path(relative).read_bytes()
            for relative in MODULE.WHIRLWIND_REQUIRED_FILES
        }
        catalog_before = self._read_json(MODULE.CATALOG_PATH)
        cues_before = self._read_json(MODULE.CUE_PATH)
        MODULE.commit_projection(projection)
        MODULE.check_projection(self._collect())

        high_jump_target = next(
            target
            for target in self.plan["targets"]
            if target["targetEffectAssetId"] == MODULE.HIGH_JUMP_EFFECT_ID
        )
        high_jump = self._read_json(
            PurePosixPath(high_jump_target["targetAuthoringPath"])
        )
        overlay = self._read_json(
            PurePosixPath(high_jump_target["overlayDocumentPath"])
        )
        self.assertEqual(overlay, high_jump)
        self.assertEqual(9, len(high_jump["elements"]))
        self.assertEqual(
            {"decal": 3, "mesh": 3, "particle": 3},
            {
                kind: sum(
                    element["kind"] == kind
                    for element in high_jump["elements"]
                )
                for kind in ("decal", "mesh", "particle")
            },
        )
        for axe in (
            element for element in high_jump["elements"]
            if element["kind"] == "mesh"
        ):
            self.assertEqual(
                [{
                    "slotId": "meshModel",
                    "assetId": MODULE.HIGH_JUMP_AXE_MODEL_ASSET_ID,
                }],
                axe["resources"],
            )
            self.assertTrue(axe["detail"]["mesh"]["useModelMaterial"])
            self.assertEqual(1.0, axe["detail"]["mesh"]["modelPreScale"])
            self.assertFalse(axe["actionCueAttachment"]["enabled"])

        catalog_after = self._read_json(MODULE.CATALOG_PATH)
        cues_after = self._read_json(MODULE.CUE_PATH)
        self.assertEqual(
            catalog_before["effects"],
            [
                row
                for row in catalog_after["effects"]
                if row["effectAssetId"] != MODULE.HIGH_JUMP_EFFECT_ID
            ],
        )
        self.assertEqual(
            cues_before["cues"],
            [
                row
                for row in cues_after["cues"]
                if row["bindingId"] != MODULE.HIGH_JUMP_CUE_ROW["bindingId"]
            ],
        )
        for relative, payload in whirlwind_files_before.items():
            self.assertEqual(payload, self._path(relative).read_bytes())

        first_apply = self._snapshot()
        second_projection = self._collect()
        MODULE.commit_projection(second_projection)
        self.assertEqual(first_apply, self._snapshot())
        self.assertEqual((), second_projection.changed_paths)

    def test_cli_dry_run_and_check_are_read_only(self) -> None:
        common = [
            "--repo-root",
            str(self.root),
            "--patch-plan",
            str(self._path(self.patch_relative)),
            "--drawable-proof",
            str(self._path(self.proof_relative)),
            "--receipt",
            str(self._path(self.receipt_relative)),
        ]
        before = self._snapshot()
        standard = io.StringIO()
        error = io.StringIO()
        with redirect_stdout(standard), redirect_stderr(error):
            result = MODULE.main(["--dry-run", *common])
        self.assertEqual(0, result)
        self.assertIn("DRY_RUN", standard.getvalue())
        self.assertEqual("", error.getvalue())
        self.assertEqual(before, self._snapshot())

        standard = io.StringIO()
        error = io.StringIO()
        with redirect_stdout(standard), redirect_stderr(error):
            result = MODULE.main(["--check", *common])
        self.assertEqual(1, result)
        self.assertIn("not applied", error.getvalue())
        self.assertEqual(before, self._snapshot())

    def test_current_repository_projection_check_is_a_no_op(self) -> None:
        proof = (
            self.source_root
            / "Data/Effects/Imported/Valtan/ProjectAuthoredPriority/DrawableProof/"
            "Valtan.project-authored-priority.drawable-proof.v1.json"
        )
        before = {
            relative: self._source_path(relative).read_bytes()
            for relative in (
                MODULE.DEFAULT_PATCH_PLAN,
                MODULE.DEFAULT_RECEIPT,
                MODULE.CATALOG_PATH,
                MODULE.CUE_PATH,
            )
        }
        projection = MODULE.collect_projection(
            self.source_root,
            drawable_proof=proof,
        )
        MODULE.check_projection(projection)
        self.assertEqual((), projection.changed_paths)
        cues = json.loads(
            self._source_path(MODULE.CUE_PATH).read_text(encoding="utf-8")
        )["cues"]
        self.assertGreaterEqual(len(cues), 108)
        self.assertTrue(
            SAFE_REVIEWED_CUE_IDS.issubset(
                {row["bindingId"] for row in cues}
            )
        )
        self.assertEqual(
            before,
            {
                relative: self._source_path(relative).read_bytes()
                for relative in before
            },
        )

    def test_existing_rows_and_hand_tuning_are_preserved(self) -> None:
        target = next(
            target
            for target in self.plan["targets"]
            if target["targetEffectAssetId"]
            == "effect.valtan.dash-charge.windup"
        )
        relative = PurePosixPath(target["targetAuthoringPath"])
        before = self._read_json(relative)
        MODULE.commit_projection(self._collect())
        after = self._read_json(relative)
        candidate_ids = {
            element["id"]
            for element in self._read_json(
                PurePosixPath(target["overlayDocumentPath"])
            )["elements"]
        }
        self.assertEqual(
            before["elements"],
            [element for element in after["elements"] if element["id"] not in candidate_ids],
        )

        tuned = next(
            element for element in after["elements"] if element["id"] in candidate_ids
        )
        tuned["detail"]["color"]["multiply"] = [0.123, 0.456, 0.789, 0.321]
        path = self._path(relative)
        source = path.read_bytes()
        path.write_bytes(MODULE._json_bytes_like(source, after))
        MODULE.commit_projection(self._collect())
        reloaded = self._read_json(relative)
        preserved = next(
            element for element in reloaded["elements"] if element["id"] == tuned["id"]
        )
        self.assertEqual(
            [0.123, 0.456, 0.789, 0.321],
            preserved["detail"]["color"]["multiply"],
        )

    def test_recipe_or_material_drift_requires_source_rebase_without_mutation(self) -> None:
        MODULE.commit_projection(self._collect())
        target = next(
            target
            for target in self.plan["targets"]
            if target["targetEffectAssetId"]
            == "effect.valtan.magic-choice.windup"
        )
        relative = PurePosixPath(target["targetAuthoringPath"])
        document = self._read_json(relative)
        projected = next(
            element
            for element in document["elements"]
            if element["id"] == "project-donut-outer-boundary"
        )
        projected["material"]["renderProfile"] = "alpha_depth_write"
        path = self._path(relative)
        path.write_bytes(MODULE._json_bytes_like(path.read_bytes(), document))
        before = self._snapshot()
        with self.assertRaisesRegex(
            MODULE.SourceRebaseRequired,
            "SOURCE_REBASE_REQUIRED immutable recipe/material drift",
        ):
            self._collect()
        self.assertEqual(before, self._snapshot())

    def test_missing_existing_document_or_unreceipted_airborne_fails_closed(self) -> None:
        existing_target = next(
            target
            for target in self.plan["targets"]
            if target["canonicalState"] == "EXISTING_DOCUMENT"
        )
        existing_path = self._path(
            PurePosixPath(existing_target["targetAuthoringPath"])
        )
        existing_path.unlink()
        before = self._snapshot()
        with self.assertRaisesRegex(
            MODULE.SourceRebaseRequired, "existing canonical document disappeared"
        ):
            self._collect()
        self.assertEqual(before, self._snapshot())

        self._copy(PurePosixPath(existing_target["targetAuthoringPath"]))
        high_jump_target = next(
            target
            for target in self.plan["targets"]
            if target["targetEffectAssetId"] == MODULE.HIGH_JUMP_EFFECT_ID
        )
        high_jump_target_path = self._path(
            PurePosixPath(high_jump_target["targetAuthoringPath"])
        )
        high_jump_target_path.parent.mkdir(parents=True, exist_ok=True)
        high_jump_target_path.write_bytes(
            self._path(PurePosixPath(high_jump_target["overlayDocumentPath"])).read_bytes()
        )
        before = self._snapshot()
        with self.assertRaisesRegex(
            MODULE.SourceRebaseRequired, "unexpected canonical document appeared"
        ):
            self._collect()
        self.assertEqual(before, self._snapshot())

    def test_invalid_proof_and_transaction_failure_mutate_zero_files(self) -> None:
        sweep = self._read_json(self.sweep_relative)
        sweep["documents"][0]["elements"][0]["failedDraws"] = 1
        self._write_json(self.sweep_relative, sweep)
        self._write_json(self.proof_relative, self._build_drawable_proof())
        before = self._snapshot()
        with self.assertRaisesRegex(MODULE.ProjectionError, "failed draws"):
            self._collect()
        self.assertEqual(before, self._snapshot())

        self._write_json(self.sweep_relative, self._build_drawable_sweep())
        self._write_json(self.proof_relative, self._build_drawable_proof())
        projection = self._collect()
        before = self._snapshot()
        with self.assertRaisesRegex(MODULE.ProjectionError, "rolled back"):
            MODULE.commit_projection(projection, failure_after_promote=3)
        self.assertEqual(before, self._snapshot())

    def test_catalog_and_cue_v2_duplicate_validation_is_read_only(self) -> None:
        catalog = self._read_json(MODULE.CATALOG_PATH)
        catalog["effects"].append(deepcopy(catalog["effects"][0]))
        path = self._path(MODULE.CATALOG_PATH)
        path.write_bytes(MODULE._json_bytes_like(path.read_bytes(), catalog))
        before = self._snapshot()
        with self.assertRaisesRegex(MODULE.ProjectionError, "duplicated"):
            self._collect()
        self.assertEqual(before, self._snapshot())

        self._copy(MODULE.CATALOG_PATH)
        cues = self._read_json(MODULE.CUE_PATH)
        duplicate = deepcopy(cues["cues"][0])
        duplicate["bindingId"] += ".different"
        cues["cues"].append(duplicate)
        cues["cues"].sort(
            key=lambda row: (row["patternId"], row["actionId"], row["bindingId"])
        )
        cue_path = self._path(MODULE.CUE_PATH)
        cue_path.write_bytes(MODULE._json_bytes_like(cue_path.read_bytes(), cues))
        before = self._snapshot()
        with self.assertRaisesRegex(MODULE.ProjectionError, "duplicated"):
            self._collect()
        self.assertEqual(before, self._snapshot())

    def test_projection_schemas_are_strict_and_parseable(self) -> None:
        proof_schema = json.loads(
            (
                self.source_root
                / "Tools/EffectPipeline/Schemas/"
                "lostark.valtan-project-authored-priority-drawable-proof.schema.json"
            ).read_text(encoding="utf-8")
        )
        receipt_schema = json.loads(
            (
                self.source_root
                / "Tools/EffectPipeline/Schemas/"
                "lostark.valtan-project-authored-priority-projection-receipt.schema.json"
            ).read_text(encoding="utf-8")
        )
        self.assertFalse(proof_schema["additionalProperties"])
        self.assertFalse(receipt_schema["additionalProperties"])
        self.assertEqual(9, proof_schema["properties"]["targets"]["minItems"])
        self.assertEqual(11, receipt_schema["properties"]["canonicalOutputs"]["minItems"])
        projection = self._collect()
        self.assertEqual(MODULE.RECEIPT_SCHEMA, projection.receipt["schema"])
        self.assertEqual("COMMITTED", projection.receipt["transactionStatus"])


if __name__ == "__main__":
    unittest.main()
