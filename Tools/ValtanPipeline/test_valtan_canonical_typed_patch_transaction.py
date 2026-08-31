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


class ValtanCanonicalTypedPatchTransactionTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory(
            prefix="valtan-canonical-typed-patch-"
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
        self.pipeline = (
            self.root / "Tools/ValtanPipeline/valtan_tuning_pipeline.py"
        )
        self.promoter = (
            self.root / "Tools/ValtanPipeline/promote_valtan_animation_chains.py"
        )
        self.wrapper = (
            self.root / "Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1"
        )
        self.authoring_root = self.root / "Intermediate/ValtanTuningAuthoring"
        self.environment = dict(os.environ)
        self.environment["PYTHONDONTWRITEBYTECODE"] = "1"
        self.repository_revision = self.source_manifest()["sourceRevision"]

    def tearDown(self) -> None:
        self.temporary.cleanup()

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
    def parse_command_result(completed: subprocess.CompletedProcess[str]) -> dict[str, Any]:
        for stream in (completed.stdout, completed.stderr):
            lines = [line for line in stream.splitlines() if line.strip()]
            if not lines:
                continue
            try:
                result = json.loads(lines[-1])
            except json.JSONDecodeError:
                continue
            if isinstance(result, dict):
                return result
        raise AssertionError(
            "Valtan command emitted no structured result:\n"
            + completed.stdout
            + completed.stderr
        )

    def run_pipeline(
        self,
        *arguments: object,
        expected_returncode: int | None = None,
    ) -> tuple[subprocess.CompletedProcess[str], dict[str, Any]]:
        completed = subprocess.run(
            [
                sys.executable,
                "-B",
                str(self.pipeline),
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
        return completed, self.parse_command_result(completed)

    def source_manifest(self) -> dict[str, Any]:
        completed, result = self.run_pipeline(
            "source-manifest", "--repository-only", expected_returncode=0
        )
        self.assertEqual(0, completed.returncode)
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
                    "schema": "lostark.valtan-tuning-draft-patch",
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
    def animation() -> dict[str, Any]:
        return {
            "endPolicy": "EXACT",
            "repeatCount": 1,
            "occurrences": [
                {
                    "clipOccurrenceId": (
                        "valtan.attack.high-jump.recovery.composition.clip.01"
                    ),
                    "clip": "mesh_idle_battle_1",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 0,
                    "playMs": 200,
                    "playRate": 1.0,
                    "repeatUntilStageEnd": False,
                },
                {
                    "clipOccurrenceId": (
                        "valtan.attack.high-jump.recovery.composition.clip.02"
                    ),
                    "clip": "mesh_idle_battle_1",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 200,
                    "playMs": 200,
                    "playRate": 1.0,
                    "repeatUntilStageEnd": False,
                },
            ],
        }

    @staticmethod
    def hit(outer_radius: float = 2.5) -> dict[str, Any]:
        return {
            "shape": {"kind": "CIRCLE", "outerRadiusM": outer_radius},
            "schedule": {
                "kind": "EXPLICIT_OFFSETS",
                "offsetsMs": [100, 300],
            },
            "serverDamageProfileId": "damage.valtan.circular-spin",
            "pushRangeM": 1.25,
            "pushMs": 150,
            "knockdown": True,
            "downMs": 600,
        }

    def typed_operations(self) -> list[dict[str, Any]]:
        return [
            {
                "op": "SET_STAGE_ANIMATION",
                "patternId": "VALTAN_HIGH_JUMP",
                "stageId": "RECOVERY",
                "animation": self.animation(),
            },
            {
                "op": "SET_STAGE_HIT",
                "patternId": "VALTAN_HIGH_JUMP",
                "stageId": "RECOVERY",
                "hit": self.hit(),
            },
        ]

    @staticmethod
    def read_json(path: Path) -> dict[str, Any]:
        return json.loads(path.read_text(encoding="utf-8"))

    @staticmethod
    def stage(document: dict[str, Any]) -> dict[str, Any]:
        return next(
            stage
            for pattern in document["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
            for stage in pattern["stages"]
            if stage["stageId"] == "RECOVERY"
        )

    def test_animation_and_hit_commit_match_split_sources_and_products(self) -> None:
        animation = self.animation()
        hit = self.hit()
        patch_path = self.write_patch(
            "typed.json", self.repository_revision, self.typed_operations()
        )
        _, result = self.run_pipeline(
            "commit-canonical-draft",
            "--authoring-root",
            self.authoring_root,
            "--draft-patch",
            patch_path,
            expected_returncode=0,
        )
        self.assertTrue(result["ok"])
        self.assertEqual("COMMIT_CANONICAL_DRAFT", result["command"])
        self.assertNotEqual(self.repository_revision, result["sourceRevision"])
        self.assertEqual(2, result["payload"]["operationCount"])
        self.assertEqual(5, result["payload"]["changedCount"])
        self.assertEqual("NOT_ACTIVATED", result["payload"]["runtimeActivation"])

        gameplay_stage = self.stage(
            self.read_json(self.root / "Data/Valtan/Valtan.gameplay.json")
        )
        presentation_stage = self.stage(
            self.read_json(self.root / "Data/Valtan/Valtan.presentation.json")
        )
        product_stage = self.stage(
            self.read_json(
                self.root / "Data/Encounters/Valtan/ValtanEncounter.json"
            )
        )
        bindings = self.read_json(
            self.root
            / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
        )
        binding = next(
            row
            for row in bindings["bindings"]
            if row["actionId"] == gameplay_stage["actionId"]
        )
        expected_product_clips = [
            {
                **{
                    key: value
                    for key, value in occurrence.items()
                    if key != "repeatUntilStageEnd"
                },
                "loop": occurrence["repeatUntilStageEnd"],
            }
            for occurrence in animation["occurrences"]
        ]
        self.assertEqual(hit, gameplay_stage["hit"])
        self.assertEqual(animation, presentation_stage["animation"])
        self.assertEqual(expected_product_clips, binding["clips"])
        self.assertEqual("CIRCLE", product_stage["hitShape"])
        self.assertEqual(2.5, product_stage["hitOuterRadius"])
        self.assertEqual([100, 300], product_stage["hitOffsetsMs"])
        self.assertEqual(
            "damage.valtan.circular-spin",
            product_stage["serverDamageProfileId"],
        )
        self.assertEqual(1.25, product_stage["pushRangeM"])
        self.assertEqual(150, product_stage["pushMs"])
        self.assertTrue(product_stage["knockdown"])
        self.assertEqual(600, product_stage["downMs"])

        _, validation = self.run_pipeline("validate", expected_returncode=0)
        self.assertTrue(validation["ok"])
        self.assertEqual(result["sourceRevision"], validation["sourceRevision"])

    def test_invalid_exact_animation_rejects_without_changing_data(self) -> None:
        baseline = self.data_manifest()
        operations = self.typed_operations()
        operations[0]["animation"]["occurrences"][1]["playMs"] = 190
        patch_path = self.write_patch(
            "invalid-exact.json", self.repository_revision, operations
        )
        completed, result = self.run_pipeline(
            "commit-canonical-draft",
            "--authoring-root",
            self.authoring_root,
            "--draft-patch",
            patch_path,
        )
        self.assertNotEqual(0, completed.returncode)
        self.assertFalse(result["ok"])
        self.assertIn("EXACT animation budget mismatch", result["errors"][0]["message"])
        self.assertEqual(baseline, self.data_manifest())

    def test_sound_owner_changed_after_preflight_blocks_removed_occurrence_without_writes(
        self,
    ) -> None:
        """The locked backend must not trust a stale Workbench preflight.

        This writes a valid separate-owner Sound row after the hypothetical UI
        preflight. The candidate Pattern patch then removes the referenced
        occurrence. Canonical commit must re-read the physical Sound owner
        under writer admission and reject before replacing any Data target.
        """

        encounter = self.read_json(
            self.root / "Data/Encounters/Valtan/ValtanEncounter.json"
        )
        recovery_stage = self.stage(encounter)
        bindings = self.read_json(
            self.root
            / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
        )
        recovery_binding = next(
            row
            for row in bindings["bindings"]
            if row["actionId"] == recovery_stage["actionId"]
        )
        removed_occurrence = recovery_binding["clips"][0]

        sound_path = (
            self.root
            / "Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json"
        )
        sound = self.read_json(sound_path)
        catalog_example = sound["cues"][0]
        sound["cues"].append(
            {
                "bindingId": "cue.sound.test.toctou.high-jump.recovery.01",
                "occurrenceId": (
                    "cue.sound.test.toctou.high-jump.recovery.01.occurrence.01"
                ),
                "patternId": "VALTAN_HIGH_JUMP",
                "stageId": "RECOVERY",
                "actionId": recovery_stage["actionId"],
                "clipOccurrenceId": removed_occurrence["clipOccurrenceId"],
                "soundBank": catalog_example["soundBank"],
                "soundEvent": catalog_example["soundEvent"],
                "repeatPolicy": "once",
                "startMs": removed_occurrence["sourceStartMs"],
            }
        )
        sound_path.write_text(
            json.dumps(sound, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        baseline = self.data_manifest()

        patch_path = self.write_patch(
            "sound-toctou-removes-occurrence.json",
            self.repository_revision,
            self.typed_operations(),
        )
        completed, result = self.run_pipeline(
            "commit-canonical-draft",
            "--authoring-root",
            self.authoring_root,
            "--draft-patch",
            patch_path,
        )
        self.assertNotEqual(0, completed.returncode)
        self.assertFalse(result["ok"])
        self.assertIn(
            "Pattern Sound dependency does not resolve candidate clip occurrence",
            result["errors"][0]["message"],
        )
        self.assertEqual(baseline, self.data_manifest())

    def test_sound_each_loop_on_non_loop_candidate_blocks_without_writes(self) -> None:
        bindings = self.read_json(
            self.root
            / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
        )
        clips_by_action = {
            row["actionId"]: {
                clip["clipOccurrenceId"]: clip for clip in row["clips"]
            }
            for row in bindings["bindings"]
        }
        sound_path = (
            self.root
            / "Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json"
        )
        sound = self.read_json(sound_path)
        changed_occurrence_id = ""
        for cue in sound["cues"]:
            clip = clips_by_action.get(cue["actionId"], {}).get(
                cue["clipOccurrenceId"]
            )
            if clip is not None and not clip["loop"]:
                cue["repeatPolicy"] = "each_loop"
                changed_occurrence_id = cue["occurrenceId"]
                break
        self.assertTrue(changed_occurrence_id)
        sound_path.write_text(
            json.dumps(sound, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        baseline = self.data_manifest()

        patch_path = self.write_patch(
            "sound-repeat-policy-toctou.json",
            self.repository_revision,
            [],
        )
        completed, result = self.run_pipeline(
            "commit-canonical-draft",
            "--authoring-root",
            self.authoring_root,
            "--draft-patch",
            patch_path,
        )
        self.assertNotEqual(0, completed.returncode)
        self.assertFalse(result["ok"])
        self.assertIn(
            "Pattern Sound each_loop dependency targets a non-loop clip",
            result["errors"][0]["message"],
        )
        self.assertIn(changed_occurrence_id, result["errors"][0]["message"])
        self.assertEqual(baseline, self.data_manifest())

    def test_injected_midcommit_failure_restores_every_data_byte(self) -> None:
        baseline = self.data_manifest()
        patch_path = self.write_patch(
            "injected.json", self.repository_revision, self.typed_operations()
        )
        completed, result = self.run_pipeline(
            "commit-canonical-draft",
            "--authoring-root",
            self.authoring_root,
            "--draft-patch",
            patch_path,
            "--inject-failure-after",
            1,
        )
        self.assertNotEqual(0, completed.returncode)
        self.assertFalse(result["ok"])
        self.assertIn("injected promotion commit failure", result["errors"][0]["message"])
        self.assertEqual(baseline, self.data_manifest())

    def test_saved_overlay_absorbs_and_stale_pointer_is_not_replayed(self) -> None:
        overlay_path = self.write_patch(
            "overlay.json",
            self.repository_revision,
            [
                {
                    "op": "SET_STAGE_HIT",
                    "patternId": "VALTAN_HIGH_JUMP",
                    "stageId": "RECOVERY",
                    "hit": self.hit(3.0),
                }
            ],
        )
        baseline = self.data_manifest()
        _, saved = self.run_pipeline(
            "save-authoring",
            "--authoring-root",
            self.authoring_root,
            "--draft-patch",
            overlay_path,
            expected_returncode=0,
        )
        saved_revision = saved["payload"]["authoringRevision"]
        self.assertEqual(baseline, self.data_manifest())

        empty_overlay_path = self.write_patch(
            "empty-overlay.json", saved_revision, []
        )
        _, absorbed = self.run_pipeline(
            "commit-canonical-draft",
            "--authoring-root",
            self.authoring_root,
            "--draft-patch",
            empty_overlay_path,
            expected_returncode=0,
        )
        canonical_revision = absorbed["sourceRevision"]
        self.assertNotEqual(self.repository_revision, canonical_revision)
        self.assertEqual(0, absorbed["payload"]["operationCount"])
        self.assertEqual(3, absorbed["payload"]["changedCount"])
        self.assertEqual("NOT_ACTIVATED", absorbed["payload"]["runtimeActivation"])
        product_stage = self.stage(
            self.read_json(
                self.root / "Data/Encounters/Valtan/ValtanEncounter.json"
            )
        )
        self.assertEqual(3.0, product_stage["hitOuterRadius"])

        _, effective = self.run_pipeline(
            "source-manifest",
            "--authoring-root",
            self.authoring_root,
            expected_returncode=0,
        )
        self.assertEqual(canonical_revision, effective["sourceRevision"])
        self.assertIsNone(effective["payload"]["authoringRevision"])

        admitted = self.data_manifest()
        completed, stale = self.run_pipeline(
            "commit-canonical-draft",
            "--authoring-root",
            self.authoring_root,
            "--draft-patch",
            empty_overlay_path,
        )
        self.assertNotEqual(0, completed.returncode)
        self.assertFalse(stale["ok"])
        self.assertIn(
            "source revision is not the current authoring head",
            stale["errors"][0]["message"],
        )
        self.assertEqual(admitted, self.data_manifest())

        new_overlay_path = self.write_patch(
            "new-overlay.json", canonical_revision, []
        )
        _, new_saved = self.run_pipeline(
            "save-authoring",
            "--authoring-root",
            self.authoring_root,
            "--draft-patch",
            new_overlay_path,
            expected_returncode=0,
        )
        new_saved_revision = new_saved["payload"]["authoringRevision"]
        self.assertNotEqual(saved_revision, new_saved_revision)
        _, new_effective = self.run_pipeline(
            "source-manifest",
            "--authoring-root",
            self.authoring_root,
            expected_returncode=0,
        )
        self.assertEqual(new_saved_revision, new_effective["sourceRevision"])
        self.assertEqual(
            new_saved_revision, new_effective["payload"]["authoringRevision"]
        )

    def test_public_wrapper_noop_reports_not_activated(self) -> None:
        for script in (self.pipeline, self.promoter):
            compile(script.read_text(encoding="utf-8"), str(script), "exec")

        patch_path = self.write_patch(
            "noop.json", self.repository_revision, []
        )
        powershell = shutil.which("powershell.exe") or shutil.which("powershell")
        self.assertIsNotNone(powershell)
        completed = subprocess.run(
            [
                str(powershell),
                "-NoProfile",
                "-ExecutionPolicy",
                "Bypass",
                "-File",
                str(self.wrapper),
                "-Mode",
                "CommitCanonicalDraft",
                "-RepositoryRoot",
                str(self.root),
                "-AuthoringRoot",
                "Intermediate/ValtanTuningAuthoring",
                "-DraftPatchPath",
                str(patch_path),
            ],
            cwd=self.root,
            env=self.environment,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            check=False,
        )
        self.assertEqual(0, completed.returncode, completed.stdout + completed.stderr)
        result = self.parse_command_result(completed)
        self.assertTrue(result["ok"])
        self.assertEqual("COMMIT_CANONICAL_DRAFT", result["command"])
        self.assertEqual(self.repository_revision, result["sourceRevision"])
        self.assertEqual(0, result["payload"]["operationCount"])
        self.assertEqual(0, result["payload"]["changedCount"])
        self.assertEqual("NOT_ACTIVATED", result["payload"]["runtimeActivation"])


if __name__ == "__main__":
    unittest.main()
