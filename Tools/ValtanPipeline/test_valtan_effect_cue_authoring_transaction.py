#!/usr/bin/env python3
from __future__ import annotations

import copy
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from typing import Any

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
if str(REPOSITORY_ROOT) not in sys.path:
    sys.path.insert(0, str(REPOSITORY_ROOT))

from Tools.ValtanPipeline import valtan_tuning_pipeline as pipeline


PATTERN_ID = "VALTAN_HIGH_JUMP"
STAGE_ID = "RECOVERY"
ACTION_ID = "valtan.attack.high-jump.recovery"
CLIP_OCCURRENCE_ID = "valtan.attack.high-jump.recovery.clip.01"
CUE_ID = "cue.valtan.composition.test.high-jump.recovery"
OCCURRENCE_ID = CUE_ID + ".occurrence.01"
EFFECT_ASSET_ID = "effect.valtan.carrier-v1.attack.backstep.windup.clip-01"


class ValtanEffectCueAuthoringTransactionTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory(
            prefix="valtan-effect-cue-authoring-"
        )
        self.root = Path(self.temporary.name).resolve()
        shutil.copytree(REPOSITORY_ROOT / "Data", self.root / "Data")
        shutil.copytree(
            REPOSITORY_ROOT / "Tools/GameplayPipeline",
            self.root / "Tools/GameplayPipeline",
        )
        shutil.copytree(
            REPOSITORY_ROOT / "Tools/ValtanPipeline",
            self.root / "Tools/ValtanPipeline",
        )
        shutil.copytree(
            REPOSITORY_ROOT / "Tools/ValtanActionExtractor",
            self.root / "Tools/ValtanActionExtractor",
            ignore=shutil.ignore_patterns("__pycache__"),
        )
        parser_source = (
            REPOSITORY_ROOT
            / "Tools/ModelAssetConverter/retime_wmodel_from_psa.py"
        )
        parser_target = (
            self.root / "Tools/ModelAssetConverter/retime_wmodel_from_psa.py"
        )
        parser_target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(parser_source, parser_target)
        for relative in (
            "Client/Bin/Resources/Character/Valtan/MN_RPBF_01.wmodel",
            "Client/Bin/Resources/Character/Valtan/AnimSets/"
            "MN_RPBF_01_AnimSet.wmodel",
        ):
            source = REPOSITORY_ROOT / relative
            target = self.root / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            try:
                os.link(source, target)
            except OSError:
                shutil.copyfile(source, target)
        self.command = (
            self.root / "Tools/ValtanPipeline/valtan_tuning_pipeline.py"
        )
        self.authoring_root = self.root / "Intermediate/ValtanTuningAuthoring"
        self.environment = dict(os.environ)
        self.environment["PYTHONDONTWRITEBYTECODE"] = "1"
        self.repository_revision = self.source_manifest()["sourceRevision"]

    def tearDown(self) -> None:
        self.temporary.cleanup()

    @staticmethod
    def read_json(path: Path) -> dict[str, Any]:
        return json.loads(path.read_text(encoding="utf-8"))

    def data_manifest(self) -> dict[str, tuple[int, str]]:
        result: dict[str, tuple[int, str]] = {}
        for path in sorted(
            candidate
            for candidate in (self.root / "Data").rglob("*")
            if candidate.is_file()
        ):
            payload = path.read_bytes()
            result[path.relative_to(self.root / "Data").as_posix()] = (
                len(payload),
                hashlib.sha256(payload).hexdigest(),
            )
        return result

    @staticmethod
    def parse_result(completed: subprocess.CompletedProcess[str]) -> dict[str, Any]:
        for stream in (completed.stdout, completed.stderr):
            for line in reversed(stream.splitlines()):
                try:
                    value = json.loads(line)
                except json.JSONDecodeError:
                    continue
                if isinstance(value, dict):
                    return value
        raise AssertionError(completed.stdout + completed.stderr)

    def run_command(
        self,
        *arguments: object,
        expected_returncode: int | None = None,
    ) -> tuple[subprocess.CompletedProcess[str], dict[str, Any]]:
        completed = subprocess.run(
            [
                sys.executable,
                "-B",
                str(self.command),
                "--repository-root",
                str(self.root),
                *(str(argument) for argument in arguments),
            ],
            cwd=self.root,
            env=self.environment,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            check=False,
        )
        if expected_returncode is not None:
            self.assertEqual(
                expected_returncode,
                completed.returncode,
                completed.stdout + completed.stderr,
            )
        return completed, self.parse_result(completed)

    def source_manifest(self) -> dict[str, Any]:
        _, result = self.run_command(
            "source-manifest", "--repository-only", expected_returncode=0
        )
        self.assertTrue(result["ok"])
        return result

    def write_patch(
        self,
        name: str,
        source_revision: str,
        operations: list[dict[str, Any]],
    ) -> Path:
        path = self.root / "fixtures" / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(
            json.dumps(
                {
                    "schema": pipeline.DRAFT_PATCH_SCHEMA,
                    "formatVersion": 1,
                    "sourceRevision": source_revision,
                    "operations": operations,
                },
                ensure_ascii=False,
                indent=2,
            )
            + "\n",
            encoding="utf-8",
        )
        return path

    @staticmethod
    def cue() -> dict[str, Any]:
        return {
            "cueId": CUE_ID,
            "occurrenceId": OCCURRENCE_ID,
            "effectAssetId": EFFECT_ASSET_ID,
            "clipOccurrenceId": CLIP_OCCURRENCE_ID,
            "sourceStartMs": 0,
            "sourceEndMs": None,
            "anchorSlotId": "root",
            "followPolicy": "follow",
            "stopPolicy": "natural",
            "repeatPolicy": "once",
            "localTransform": {
                "position": [0.0, 0.0, 0.0],
                "rotationDegrees": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
            "scalePolicy": {"kind": "OWNER_RELATIVE"},
            "mappingBasis": "PROJECT_AUTHORED",
        }

    @classmethod
    def add_operation(cls, cue: dict[str, Any] | None = None) -> dict[str, Any]:
        return {
            "op": "ADD_EFFECT_CUE",
            "patternId": PATTERN_ID,
            "stageId": STAGE_ID,
            "actionId": ACTION_ID,
            "cue": copy.deepcopy(cue if cue is not None else cls.cue()),
        }

    @classmethod
    def update_operation(cls, cue: dict[str, Any]) -> dict[str, Any]:
        return {
            "op": "UPDATE_EFFECT_CUE",
            "patternId": PATTERN_ID,
            "stageId": STAGE_ID,
            "actionId": ACTION_ID,
            "cueId": CUE_ID,
            "occurrenceId": OCCURRENCE_ID,
            "cue": copy.deepcopy(cue),
        }

    @classmethod
    def remove_operation(cls) -> dict[str, Any]:
        return {
            "op": "REMOVE_EFFECT_CUE",
            "patternId": PATTERN_ID,
            "stageId": STAGE_ID,
            "actionId": ACTION_ID,
            "cueId": CUE_ID,
            "occurrenceId": OCCURRENCE_ID,
            "effectAssetId": EFFECT_ASSET_ID,
            "clipOccurrenceId": CLIP_OCCURRENCE_ID,
        }

    @staticmethod
    def source_cue(document: dict[str, Any]) -> dict[str, Any] | None:
        return next(
            (
                cue
                for pattern in document["patterns"]
                if pattern["patternId"] == PATTERN_ID
                for stage in pattern["stages"]
                if stage["stageId"] == STAGE_ID
                for cue in stage["effectCues"]
                if cue["cueId"] == CUE_ID
            ),
            None,
        )

    @staticmethod
    def product_cue(document: dict[str, Any]) -> dict[str, Any] | None:
        return next(
            (cue for cue in document["cues"] if cue["bindingId"] == CUE_ID),
            None,
        )

    def commit(
        self,
        name: str,
        revision: str,
        operations: list[dict[str, Any]],
        *extra: object,
        expected_returncode: int | None = 0,
    ) -> tuple[subprocess.CompletedProcess[str], dict[str, Any]]:
        patch = self.write_patch(name, revision, operations)
        return self.run_command(
            "commit-canonical-draft",
            "--authoring-root",
            self.authoring_root,
            "--draft-patch",
            patch,
            *extra,
            expected_returncode=expected_returncode,
        )

    def test_add_update_remove_close_validate_save_and_canonical_projection(self) -> None:
        baseline = self.data_manifest()
        add_patch = self.write_patch(
            "add.json", self.repository_revision, [self.add_operation()]
        )
        _, validated = self.run_command(
            "validate-draft",
            "--authoring-root",
            self.authoring_root,
            "--draft-patch",
            add_patch,
            expected_returncode=0,
        )
        self.assertTrue(validated["ok"])
        _, saved = self.run_command(
            "save-authoring",
            "--authoring-root",
            self.authoring_root,
            "--draft-patch",
            add_patch,
            expected_returncode=0,
        )
        self.assertTrue(saved["ok"])
        self.assertEqual(baseline, self.data_manifest())
        saved_revision = saved["payload"]["authoringRevision"]

        _, added = self.commit("absorb-add.json", saved_revision, [])
        self.assertTrue(added["ok"])
        added_revision = added["sourceRevision"]
        source = self.read_json(
            self.root / "Data/Valtan/Valtan.presentation.json"
        )
        product = self.read_json(
            self.root
            / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
        )
        self.assertEqual(self.cue(), self.source_cue(source))
        projected = self.product_cue(product)
        self.assertIsNotNone(projected)
        assert projected is not None
        self.assertEqual(PATTERN_ID, projected["patternId"])
        self.assertEqual(STAGE_ID, projected["stageId"])
        self.assertEqual(ACTION_ID, projected["actionId"])
        self.assertEqual(CLIP_OCCURRENCE_ID, projected["clipOccurrenceId"])

        updated_cue = self.cue()
        updated_cue["sourceStartMs"] = 50
        updated_cue["sourceEndMs"] = 250
        updated_cue["stopPolicy"] = "cue_end"
        updated_cue["followPolicy"] = "snapshot"
        updated_cue["localTransform"] = {
            "position": [1.0, 2.0, 3.0],
            "rotationDegrees": [0.0, 180.0, 0.0],
            "scale": [2.0, 2.0, 2.0],
        }
        _, updated = self.commit(
            "update.json",
            added_revision,
            [self.update_operation(updated_cue)],
        )
        self.assertTrue(updated["ok"])
        updated_revision = updated["sourceRevision"]
        source = self.read_json(
            self.root / "Data/Valtan/Valtan.presentation.json"
        )
        product = self.read_json(
            self.root
            / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
        )
        self.assertEqual(updated_cue, self.source_cue(source))
        projected = self.product_cue(product)
        self.assertIsNotNone(projected)
        assert projected is not None
        self.assertEqual(50, projected["sourceStartMs"])
        self.assertEqual(250, projected["sourceEndMs"])
        self.assertEqual([1.0, 2.0, 3.0], projected["localTransform"]["position"])
        self.assertEqual(
            [0.0, 180.0, 0.0],
            projected["localTransform"]["rotationDegrees"],
        )

        _, removed = self.commit(
            "remove.json",
            updated_revision,
            [self.remove_operation()],
        )
        self.assertTrue(removed["ok"])
        source = self.read_json(
            self.root / "Data/Valtan/Valtan.presentation.json"
        )
        product = self.read_json(
            self.root
            / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
        )
        self.assertIsNone(self.source_cue(source))
        self.assertIsNone(self.product_cue(product))

    def test_invalid_dependencies_and_identities_fail_without_data_writes(self) -> None:
        baseline = self.data_manifest()
        cases: list[tuple[str, dict[str, Any], str]] = []

        malformed = self.cue()
        malformed["cueId"] = "cue valtan invalid"
        cases.append(("malformed", self.add_operation(malformed), "not a stable ID"))

        namespace = self.cue()
        namespace["cueId"] = "cue.other.unregistered.test"
        namespace["occurrenceId"] = namespace["cueId"] + ".occurrence.01"
        cases.append(
            (
                "namespace",
                self.add_operation(namespace),
                "must use the cue.valtan. namespace",
            )
        )

        action_mismatch = self.add_operation()
        action_mismatch["actionId"] = "valtan.attack.high-jump.land"
        cases.append(("action", action_mismatch, "does not match its exact Pattern Stage"))

        clip_mismatch = self.cue()
        clip_mismatch["clipOccurrenceId"] = "missing.clip.occurrence"
        cases.append(("clip", self.add_operation(clip_mismatch), "does not resolve its saved animation occurrence"))

        repeat_mismatch = self.cue()
        repeat_mismatch["repeatPolicy"] = "each_loop"
        cases.append(("repeat", self.add_operation(repeat_mismatch), "must target a looping animation occurrence"))

        transform = self.cue()
        transform["localTransform"]["scale"] = [1.0, 0.0, 1.0]
        cases.append(("transform", self.add_operation(transform), "is out of range"))

        catalog = self.cue()
        catalog["effectAssetId"] = "effect.valtan.missing.authored-source"
        cases.append(("catalog", self.add_operation(catalog), "must resolve exactly one EffectCatalog row"))

        for name, operation, message in cases:
            with self.subTest(name=name):
                patch = self.write_patch(
                    f"invalid-{name}.json",
                    self.repository_revision,
                    [operation],
                )
                completed, result = self.run_command(
                    "commit-canonical-draft",
                    "--authoring-root",
                    self.authoring_root,
                    "--draft-patch",
                    patch,
                )
                self.assertNotEqual(0, completed.returncode)
                self.assertFalse(result["ok"])
                self.assertIn(message, result["errors"][0]["message"])
                self.assertEqual(baseline, self.data_manifest())

    def test_catalog_and_authored_effect_source_dependencies_fail_closed(self) -> None:
        catalog_path = self.root / pipeline.EFFECT_CATALOG_REL
        catalog_bytes = catalog_path.read_bytes()
        catalog = self.read_json(catalog_path)
        catalog_row = next(
            row
            for row in catalog["effects"]
            if row["effectAssetId"] == EFFECT_ASSET_ID
        )
        catalog_row["authoringPath"] = (
            "Effects/Authored/effect.valtan.not-the-selected-identity.effect.json"
        )
        catalog_path.write_text(
            pipeline.json_text(catalog), encoding="utf-8", newline=""
        )
        catalog_drift = self.data_manifest()
        try:
            patch = self.write_patch(
                "catalog-path-drift.json",
                self.source_manifest()["sourceRevision"],
                [self.add_operation()],
            )
            completed, result = self.run_command(
                "commit-canonical-draft",
                "--authoring-root",
                self.authoring_root,
                "--draft-patch",
                patch,
            )
            self.assertNotEqual(0, completed.returncode)
            self.assertIn("authoringPath is not identity-derived", result["errors"][0]["message"])
            self.assertEqual(catalog_drift, self.data_manifest())
        finally:
            catalog_path.write_bytes(catalog_bytes)

        source_path = (
            self.root
            / "Data/Effects/Authored"
            / f"{EFFECT_ASSET_ID}.effect.json"
        )
        source_bytes = source_path.read_bytes()
        source = self.read_json(source_path)
        source["version"] = 999
        source_path.write_text(
            pipeline.json_text(source), encoding="utf-8", newline=""
        )
        source_drift = self.data_manifest()
        try:
            patch = self.write_patch(
                "effect-source-version-drift.json",
                self.source_manifest()["sourceRevision"],
                [self.add_operation()],
            )
            completed, result = self.run_command(
                "commit-canonical-draft",
                "--authoring-root",
                self.authoring_root,
                "--draft-patch",
                patch,
            )
            self.assertNotEqual(0, completed.returncode)
            self.assertIn("authored source contract is invalid", result["errors"][0]["message"])
            self.assertEqual(source_drift, self.data_manifest())
        finally:
            source_path.write_bytes(source_bytes)

    def test_update_identity_and_remove_predecessor_are_exact(self) -> None:
        _, added = self.commit(
            "add-direct.json",
            self.repository_revision,
            [self.add_operation()],
        )
        self.assertTrue(added["ok"])
        admitted = self.data_manifest()
        revision = added["sourceRevision"]

        identity_mutation = self.cue()
        identity_mutation["cueId"] = CUE_ID + ".renamed"
        identity_mutation["occurrenceId"] = (
            identity_mutation["cueId"] + ".occurrence.01"
        )
        patch = self.write_patch(
            "update-identity.json",
            revision,
            [self.update_operation(identity_mutation)],
        )
        completed, result = self.run_command(
            "commit-canonical-draft",
            "--authoring-root",
            self.authoring_root,
            "--draft-patch",
            patch,
        )
        self.assertNotEqual(0, completed.returncode)
        self.assertIn("must preserve cueId and occurrenceId", result["errors"][0]["message"])
        self.assertEqual(admitted, self.data_manifest())

        remove = self.remove_operation()
        remove["clipOccurrenceId"] = "different.clip.occurrence"
        patch = self.write_patch("remove-stale.json", revision, [remove])
        completed, result = self.run_command(
            "commit-canonical-draft",
            "--authoring-root",
            self.authoring_root,
            "--draft-patch",
            patch,
        )
        self.assertNotEqual(0, completed.returncode)
        self.assertIn("predecessor dependency does not match", result["errors"][0]["message"])
        self.assertEqual(admitted, self.data_manifest())

    def test_duplicate_identity_and_injected_commit_failure_restore_every_byte(self) -> None:
        duplicate = self.cue()
        existing_source = self.read_json(
            self.root / "Data/Valtan/Valtan.presentation.json"
        )
        existing = next(
            cue
            for pattern in existing_source["patterns"]
            for stage in pattern["stages"]
            for cue in stage["effectCues"]
        )
        duplicate["cueId"] = existing["cueId"]
        duplicate["occurrenceId"] = existing["cueId"] + ".occurrence.99"
        duplicate["scalePolicy"] = copy.deepcopy(existing["scalePolicy"])
        baseline = self.data_manifest()
        patch = self.write_patch(
            "duplicate.json",
            self.repository_revision,
            [self.add_operation(duplicate)],
        )
        completed, result = self.run_command(
            "commit-canonical-draft",
            "--authoring-root",
            self.authoring_root,
            "--draft-patch",
            patch,
        )
        self.assertNotEqual(0, completed.returncode)
        self.assertIn("duplicate Effect cueId", result["errors"][0]["message"])
        self.assertEqual(baseline, self.data_manifest())

        completed, result = self.commit(
            "injected.json",
            self.repository_revision,
            [self.add_operation()],
            "--inject-failure-after",
            1,
            expected_returncode=None,
        )
        self.assertNotEqual(0, completed.returncode)
        self.assertIn("injected promotion commit failure", result["errors"][0]["message"])
        self.assertEqual(baseline, self.data_manifest())


if __name__ == "__main__":
    unittest.main()
