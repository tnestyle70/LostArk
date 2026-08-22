#!/usr/bin/env python3
from __future__ import annotations

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
    "build_valtan_project_authored_priority_drawable_proof.py"
)
SPEC = importlib.util.spec_from_file_location(
    "build_valtan_project_authored_priority_drawable_proof", SCRIPT_PATH
)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError(f"cannot load {SCRIPT_PATH}")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class ValtanPriorityDrawableProofTests(unittest.TestCase):
    def setUp(self) -> None:
        self.source_root = SCRIPT_PATH.resolve().parents[2]
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.resource_root = self.root / "resource-root"
        self.resource_root.mkdir()
        self.patch_relative = MODULE.DEFAULT_PATCH_PLAN
        self.sweep_relative = MODULE.DEFAULT_DRAWABLE_SWEEP
        self.proof_relative = MODULE.DEFAULT_DRAWABLE_PROOF
        self._copy(self.patch_relative)
        self.plan = self._read(self.patch_relative)
        for target in self.plan["targets"]:
            self._copy(PurePosixPath(target["overlayDocumentPath"]))
        self._write(self.sweep_relative, self._make_sweep())

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
        path.write_bytes(MODULE._json_bytes(value))

    def _make_sweep(self) -> dict:
        documents = []
        for target in self.plan["targets"]:
            overlay_relative = PurePosixPath(target["overlayDocumentPath"])
            overlay = self._read(overlay_relative)
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
            "schema": MODULE.SWEEP_SCHEMA,
            "formatVersion": 1,
            "resourceRoot": self.resource_root.resolve().as_posix(),
            "sampleRateHz": 60,
            "documents": documents,
        }

    def _build(self) -> dict:
        return MODULE.build_drawable_proof(
            self.root,
            patch_plan=self.patch_relative,
            drawable_sweep=self.sweep_relative,
            expected_resource_root=self.resource_root,
        )

    def test_build_write_check_and_hash_closure(self) -> None:
        proof = self._build()
        self.assertEqual(9, len(proof["targets"]))
        self.assertEqual(
            17, sum(len(target["elements"]) for target in proof["targets"])
        )
        self.assertEqual(
            hashlib.sha256(self._path(self.sweep_relative).read_bytes()).hexdigest(),
            proof["drawableSweepSha256"],
        )
        for plan_target, proof_target in zip(
            self.plan["targets"], proof["targets"], strict=True
        ):
            self.assertEqual(
                plan_target["overlayDocumentSha256"],
                proof_target["overlayDocumentSha256"],
            )

        common = [
            "--repo-root",
            str(self.root),
            "--patch-plan",
            str(self._path(self.patch_relative)),
            "--drawable-sweep",
            str(self._path(self.sweep_relative)),
            "--expected-resource-root",
            str(self.resource_root),
            "--output",
            str(self._path(self.proof_relative)),
        ]
        with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
            self.assertEqual(0, MODULE.main(["--write", *common]))
            first = self._path(self.proof_relative).read_bytes()
            self.assertEqual(0, MODULE.main(["--check", *common]))
            self.assertEqual(first, self._path(self.proof_relative).read_bytes())

    def test_failed_draw_or_wrong_absolute_document_is_rejected(self) -> None:
        sweep = self._read(self.sweep_relative)
        sweep["documents"][0]["elements"][0]["failedDraws"] = 1
        self._write(self.sweep_relative, sweep)
        with self.assertRaisesRegex(MODULE.DrawableProofError, "clean draw"):
            self._build()

        sweep = self._make_sweep()
        sweep["documents"][0]["documentPath"] = sweep["documents"][1][
            "documentPath"
        ]
        self._write(self.sweep_relative, sweep)
        with self.assertRaisesRegex(MODULE.DrawableProofError, "path/identity"):
            self._build()

    def test_candidate_sha_and_resource_root_drift_are_rejected(self) -> None:
        target = self.plan["targets"][0]
        overlay_relative = PurePosixPath(target["overlayDocumentPath"])
        overlay = self._read(overlay_relative)
        overlay["displayName"] += " drift"
        self._write(overlay_relative, overlay)
        with self.assertRaisesRegex(MODULE.DrawableProofError, "SHA is stale"):
            self._build()

        self._copy(overlay_relative)
        other_resource_root = self.root / "other-resource-root"
        other_resource_root.mkdir()
        with self.assertRaisesRegex(MODULE.DrawableProofError, "wrong resource root"):
            MODULE.build_drawable_proof(
                self.root,
                patch_plan=self.patch_relative,
                drawable_sweep=self.sweep_relative,
                expected_resource_root=other_resource_root,
            )

    def test_schemas_are_strict_json(self) -> None:
        sweep_schema = json.loads(
            (
                self.source_root
                / "Tools/EffectPipeline/Schemas/"
                "lostark.effect-document-drawable-sweep.schema.json"
            ).read_text(encoding="utf-8")
        )
        proof_schema = json.loads(
            (
                self.source_root
                / "Tools/EffectPipeline/Schemas/"
                "lostark.valtan-project-authored-priority-drawable-proof.schema.json"
            ).read_text(encoding="utf-8")
        )
        self.assertFalse(sweep_schema["additionalProperties"])
        self.assertFalse(proof_schema["additionalProperties"])
        self.assertIn("preparedSamples", proof_schema["$defs"]["element"]["required"])


if __name__ == "__main__":
    unittest.main()
