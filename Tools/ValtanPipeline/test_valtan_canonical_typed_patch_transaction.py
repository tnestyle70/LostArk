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
import time
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

    def start_shared_reader(self, hold_seconds: float) -> subprocess.Popen[str]:
        lock_path = (
            self.root
            / "out/ValtanPatternTransactions/create-pattern.lock"
        )
        lock_path.parent.mkdir(parents=True, exist_ok=True)
        lock_path.write_bytes(b"\0")
        ready_path = self.root / "canonical-shared-reader.ready"
        holder_program = r"""
import ctypes
from ctypes import wintypes
from pathlib import Path
import sys
import time


class Overlapped(ctypes.Structure):
    _fields_ = [
        ("Internal", ctypes.c_size_t),
        ("InternalHigh", ctypes.c_size_t),
        ("Offset", wintypes.DWORD),
        ("OffsetHigh", wintypes.DWORD),
        ("hEvent", wintypes.HANDLE),
    ]


lock_path = Path(sys.argv[1])
ready_path = Path(sys.argv[2])
hold_seconds = float(sys.argv[3])
kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
create_file = kernel32.CreateFileW
create_file.argtypes = [
    wintypes.LPCWSTR,
    wintypes.DWORD,
    wintypes.DWORD,
    wintypes.LPVOID,
    wintypes.DWORD,
    wintypes.DWORD,
    wintypes.HANDLE,
]
create_file.restype = wintypes.HANDLE
lock_file = kernel32.LockFileEx
lock_file.argtypes = [
    wintypes.HANDLE,
    wintypes.DWORD,
    wintypes.DWORD,
    wintypes.DWORD,
    wintypes.DWORD,
    ctypes.POINTER(Overlapped),
]
lock_file.restype = wintypes.BOOL
unlock_file = kernel32.UnlockFileEx
unlock_file.argtypes = [
    wintypes.HANDLE,
    wintypes.DWORD,
    wintypes.DWORD,
    wintypes.DWORD,
    ctypes.POINTER(Overlapped),
]
unlock_file.restype = wintypes.BOOL
close_handle = kernel32.CloseHandle
close_handle.argtypes = [wintypes.HANDLE]
close_handle.restype = wintypes.BOOL

generic_read = 0x80000000
generic_write = 0x40000000
file_share_read = 0x00000001
file_share_write = 0x00000002
file_share_delete = 0x00000004
open_existing = 3
file_attribute_normal = 0x00000080
handle = create_file(
    str(lock_path),
    generic_read | generic_write,
    file_share_read | file_share_write | file_share_delete,
    None,
    open_existing,
    file_attribute_normal,
    None,
)
if handle == ctypes.c_void_p(-1).value:
    raise ctypes.WinError(ctypes.get_last_error())
overlap = Overlapped()
acquired = False
try:
    lockfile_fail_immediately = 0x00000001
    if not lock_file(
        handle,
        lockfile_fail_immediately,
        0,
        1,
        0,
        ctypes.byref(overlap),
    ):
        raise ctypes.WinError(ctypes.get_last_error())
    acquired = True
    ready_path.write_text("locked", encoding="utf-8")
    time.sleep(hold_seconds)
finally:
    if acquired:
        unlock_file(handle, 0, 1, 0, ctypes.byref(overlap))
    close_handle(handle)
"""
        holder = subprocess.Popen(
            [
                sys.executable,
                "-c",
                holder_program,
                str(lock_path),
                str(ready_path),
                str(hold_seconds),
            ],
            cwd=self.root,
            env=self.environment,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            encoding="utf-8",
            errors="replace",
        )
        deadline = time.monotonic() + 10.0
        while (
            not ready_path.exists()
            and holder.poll() is None
            and time.monotonic() < deadline
        ):
            time.sleep(0.025)
        if not ready_path.exists():
            if holder.poll() is None:
                holder.terminate()
            stdout, stderr = holder.communicate(timeout=5.0)
            self.fail(
                "shared canonical reader did not acquire byte zero; "
                f"stdout={stdout!r} stderr={stderr!r}"
            )
        return holder

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
                    "clip": "mesh_att_battle_13_04",
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
                    "clip": "mesh_att_battle_13_04",
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

    def test_scripted_sequence_commit_updates_canonical_and_generated_product(
        self,
    ) -> None:
        flow_path = (
            self.root
            / "Data/Encounters/Valtan/ValtanBossAuditionFlows.json"
        )
        retired_flow_before = (
            flow_path.read_bytes() if flow_path.is_file() else None
        )
        expected = {
            "sequenceId": "sequence.valtan.server-authored.v1",
            "mode": "ORDERED_ONCE_THEN_IDLE",
            "interStepPursuitMs": 900,
            "patternIds": [
                "VALTAN_FOUR_SLASH",
                "VALTAN_WHIRLWIND",
                "VALTAN_FOUR_SLASH",
            ],
        }
        patch_path = self.write_patch(
            "sequence.json",
            self.repository_revision,
            [{"op": "SET_SCRIPTED_SEQUENCE", **expected}],
        )
        rotations_path = (
            self.root
            / "Data/Encounters/Valtan/ValtanPatternRotations.json"
        )
        stale_rotations = self.read_json(rotations_path)
        stale_rotations["scriptedSequence"] = {
            **expected,
            "interStepPursuitMs": 100,
            "patternIds": ["VALTAN_WHIRLWIND"],
        }
        rotations_path.write_text(
            json.dumps(stale_rotations, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        self.assertEqual(
            self.repository_revision,
            self.source_manifest()["sourceRevision"],
            "generated Product drift must not change canonical source CAS",
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
        self.assertEqual(1, result["payload"]["operationCount"])
        gameplay = self.read_json(
            self.root / "Data/Valtan/Valtan.gameplay.json"
        )
        rotations = self.read_json(
            rotations_path
        )
        self.assertEqual(expected, gameplay["decisionModel"]["scriptedSequence"])
        self.assertEqual(expected, rotations["scriptedSequence"])
        if retired_flow_before is not None:
            self.assertEqual(retired_flow_before, flow_path.read_bytes())

    def test_scripted_sequence_invalid_identity_inventory_and_entry_fail_closed(
        self,
    ) -> None:
        baseline = self.data_manifest()
        valid = {
            "sequenceId": "sequence.valtan.server-authored.v1",
            "mode": "ORDERED_ONCE_THEN_IDLE",
            "interStepPursuitMs": 900,
            "patternIds": ["VALTAN_WHIRLWIND"],
        }
        cases = (
            ("identity", {**valid, "sequenceId": "sequence.other"}),
            ("mode", {**valid, "mode": "ORDERED_REPEAT"}),
            ("pursuit", {**valid, "interStepPursuitMs": 0}),
            ("unknown", {**valid, "patternIds": ["VALTAN_UNKNOWN"]}),
            ("malformed", {**valid, "patternIds": ["../invalid"]}),
            (
                "duplicate-entrance",
                {
                    **valid,
                    "patternIds": [
                        "VALTAN_ENTRANCE_CINEMATIC",
                        "VALTAN_ENTRANCE_CINEMATIC",
                    ],
                },
            ),
        )
        for name, operation in cases:
            with self.subTest(name=name):
                patch_path = self.write_patch(
                    f"invalid-sequence-{name}.json",
                    self.repository_revision,
                    [{"op": "SET_SCRIPTED_SEQUENCE", **operation}],
                )
                _, result = self.run_pipeline(
                    "validate-draft",
                    "--authoring-root",
                    self.authoring_root,
                    "--draft-patch",
                    patch_path,
                    expected_returncode=1,
                )
                self.assertFalse(result["ok"])
                self.assertEqual(baseline, self.data_manifest())

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
        self.assertEqual(6, result["payload"]["changedCount"])
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

        root_motion_check = subprocess.run(
            [
                sys.executable,
                "-B",
                str(
                    self.root
                    / "Tools/ValtanActionExtractor/build_valtan_rootmotion.py"
                ),
                "--repo-root",
                str(self.root),
                "--check",
            ],
            cwd=self.root,
            env=self.environment,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            check=False,
        )
        self.assertEqual(
            0,
            root_motion_check.returncode,
            root_motion_check.stdout + root_motion_check.stderr,
        )

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

    def test_public_wrapper_v1_effect_delete_commits_structured_result(self) -> None:
        """The shipped v1 Client must stay writable until reviewed v2 migration.

        Composition sends the physical binding bytes it admitted at Reload and
        one candidate document.  A v2 resource read-set is deliberately absent
        for a formatVersion 1 owner; the shared writer still validates the
        candidate against the projected Pattern closure and CASes the binding
        file byte-for-byte.
        """

        patch_path = self.write_patch(
            "v1-effect-delete.json", self.repository_revision, []
        )
        binding_target = (
            self.root
            / "Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json"
        )
        baseline_bytes = binding_target.read_bytes().replace(b"\r\n", b"\n")
        baseline_bytes = baseline_bytes.replace(b"\n", b"\r\n")
        binding_target.write_bytes(baseline_bytes)
        self.assertIn(b"\r\n", baseline_bytes)
        candidate_document = json.loads(baseline_bytes)
        self.assertEqual(1, candidate_document["formatVersion"])
        self.assertGreater(len(candidate_document["bindings"]), 0)
        candidate_document["bindings"].pop()
        candidate_bytes = (
            json.dumps(candidate_document, ensure_ascii=False, indent=2) + "\n"
        ).encode("utf-8")
        baseline_path = self.root / "v1-effect-baseline.json"
        candidate_path = self.root / "v1-effect-candidate.json"
        baseline_path.write_bytes(baseline_bytes)
        candidate_path.write_bytes(candidate_bytes)

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
                "-EffectV2BaselinePath",
                str(baseline_path),
                "-EffectV2CandidatePath",
                str(candidate_path),
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
        self.assertEqual("NOT_ACTIVATED", result["payload"]["runtimeActivation"])
        self.assertEqual(candidate_bytes, binding_target.read_bytes())

    def test_public_wrapper_removes_appended_sequence_slots_and_physical_provenance(
        self,
    ) -> None:
        """The Composition delete flow commits the one remaining visible slot.

        The shipped Client owns the complete SET_STAGE_ANIMATION list but does
        not yet emit a separate provenance-removal operation.  The canonical
        writer must therefore remove the exact deterministic REFERENCE tuple
        and its now-unreferenced sourceActionId in the same physical commit.
        """

        presentation_target = self.root / "Data/Valtan/Valtan.presentation.json"
        gameplay_target = self.root / "Data/Valtan/Valtan.gameplay.json"
        presentation = self.read_json(presentation_target)
        ground_roar = next(
            pattern
            for pattern in presentation["patterns"]
            if pattern["patternId"] == "VALTAN_GROUND_ROAR"
        )
        stage = next(
            stage for stage in ground_roar["stages"] if stage["stageId"] == "STEP_01"
        )
        self.assertEqual(2, len(stage["animation"]["occurrences"]))
        self.assertIn(
            {
                "sourceActionId": 400425,
                "sequenceIndex": 0,
                "role": "REFERENCE_400425_0",
            },
            ground_roar["presentationSources"],
        )
        self.assertEqual(
            ["SOURCE_REVIEWED_DELTA"] * 2,
            [
                row["mappingBasis"]
                for row in stage["animation"]["occurrences"]
            ],
        )
        survivor = copy.deepcopy(stage["animation"]["occurrences"][0])
        powershell = shutil.which("powershell.exe") or shutil.which("powershell")
        self.assertIsNotNone(powershell)
        patch_path = self.write_patch(
            "ground-roar-remove-appended-sequence.json",
            self.repository_revision,
            [
                {
                    "op": "SET_STAGE_ANIMATION",
                    "patternId": "VALTAN_GROUND_ROAR",
                    "stageId": "STEP_01",
                    "animation": {
                        "endPolicy": "HOLD_LAST_POSE",
                        "repeatCount": 1,
                        "occurrences": [survivor],
                    },
                }
            ],
        )

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
        self.assertEqual(1, result["payload"]["operationCount"])
        self.assertEqual("NOT_ACTIVATED", result["payload"]["runtimeActivation"])

        committed_presentation = self.read_json(presentation_target)
        committed_ground_roar = next(
            pattern
            for pattern in committed_presentation["patterns"]
            if pattern["patternId"] == "VALTAN_GROUND_ROAR"
        )
        self.assertEqual(
            [
                {
                    "sourceActionId": 400440,
                    "sequenceIndex": 0,
                    "role": "PRIMARY",
                }
            ],
            committed_ground_roar["presentationSources"],
        )
        committed_occurrences = committed_ground_roar["stages"][0]["animation"][
            "occurrences"
        ]
        self.assertEqual(1, len(committed_occurrences))
        self.assertEqual("mesh_att_battle_11_01", committed_occurrences[0]["clip"])
        self.assertEqual(
            "SOURCE_REVIEWED_DELTA",
            committed_occurrences[0]["mappingBasis"],
        )

        committed_gameplay = self.read_json(gameplay_target)
        committed_gameplay_ground_roar = next(
            pattern
            for pattern in committed_gameplay["patterns"]
            if pattern["patternId"] == "VALTAN_GROUND_ROAR"
        )
        self.assertEqual([400440], committed_gameplay_ground_roar["sourceActionIds"])
        self.assertEqual(
            "SPAWN_COMBAT_OBJECT_VOLLEY",
            committed_gameplay_ground_roar["stages"][0]["events"][0]["kind"],
        )

    def test_dash_charge_groggy_sequence_and_sound_save_physically(self) -> None:
        """The separate Groggy pattern and its Sound row are replaced atomically."""

        presentation_target = self.root / "Data/Valtan/Valtan.presentation.json"
        gameplay_target = self.root / "Data/Valtan/Valtan.gameplay.json"
        sound_target = (
            self.root
            / "Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json"
        )
        presentation = self.read_json(presentation_target)
        groggy_pattern = next(
            pattern
            for pattern in presentation["patterns"]
            if pattern["patternId"] == "VALTAN_DASH_CHARGE_GROGGY"
        )
        groggy = next(
            stage for stage in groggy_pattern["stages"]
            if stage["stageId"] == "GROGGY"
        )
        self.assertEqual(
            [
                "mesh_abn_groggy_1_start",
                "mesh_abn_groggy_1_loop",
                "mesh_abn_groggy_1_loop",
                "mesh_abn_groggy_1_loop",
                "mesh_abn_groggy_1_end",
            ],
            [row["clip"] for row in groggy["animation"]["occurrences"]],
        )
        self.assertEqual(
            [1833, 1333, 1333, 334, 2000],
            [row["playMs"] for row in groggy["animation"]["occurrences"]],
        )

        old_clip_occurrence_id = (
            "valtan.attack.dash-charge.recovery.project-tuned.clip.01"
        )
        old_sound_occurrence_id = (
            "cue.sound.valtan.attack.dash-charge.recovery."
            "project-tuned.clip.01.01.occurrence.01"
        )

        seed_sound_baseline_path = self.root / "dash-charge-seed-sound-baseline.json"
        seed_sound_candidate_path = self.root / "dash-charge-seed-sound-candidate.json"
        seed_sound_baseline_path.write_bytes(sound_target.read_bytes())
        seed_sound_candidate = self.read_json(sound_target)
        self.assertFalse(
            any(
                cue.get("patternId") == "VALTAN_DASH_CHARGE_GROGGY"
                and cue.get("stageId") == "GROGGY"
                for cue in seed_sound_candidate["cues"]
            )
        )
        seed_sound_candidate["cues"].append(
            {
                "bindingId": (
                    "cue.sound.valtan.attack.dash-charge.recovery."
                    "project-tuned.clip.01.01"
                ),
                "occurrenceId": old_sound_occurrence_id,
                "patternId": "VALTAN_DASH_CHARGE_GROGGY",
                "stageId": "GROGGY",
                "actionId": "valtan.attack.dash-charge.recovery",
                "clipOccurrenceId": old_clip_occurrence_id,
                "soundBank": "S_Mob_G_Voltan2",
                "soundEvent": "G_Voltan2_FootStep1",
                "repeatPolicy": "once",
                "startMs": 3600,
            }
        )
        seed_sound_candidate_path.write_text(
            json.dumps(seed_sound_candidate, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        seed_patch_path = self.write_patch(
            "dash-charge-recovery-seed-old-fixture.json",
            self.repository_revision,
            [
                {
                    "op": "SET_STAGE_DURATION",
                    "patternId": "VALTAN_DASH_CHARGE_GROGGY",
                    "stageId": "GROGGY",
                    "durationMs": 900,
                },
                {
                    "op": "SET_STAGE_ANIMATION",
                    "patternId": "VALTAN_DASH_CHARGE_GROGGY",
                    "stageId": "GROGGY",
                    "animation": {
                        "endPolicy": "EXACT",
                        "repeatCount": 1,
                        "occurrences": [
                            {
                                "clipOccurrenceId": old_clip_occurrence_id,
                                "clip": "mesh_att_battle_4_01",
                                "mappingBasis": "PROJECT_AUTHORED",
                                "sourceStartMs": 3350,
                                "playMs": 1383,
                                "playRate": 1.5366667,
                                "repeatUntilStageEnd": False,
                            }
                        ],
                    },
                },
            ],
        )
        powershell = shutil.which("powershell.exe") or shutil.which("powershell")
        self.assertIsNotNone(powershell)
        seeded = subprocess.run(
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
                str(seed_patch_path),
                "-PatternSoundBaselinePath",
                str(seed_sound_baseline_path),
                "-PatternSoundCandidatePath",
                str(seed_sound_candidate_path),
            ],
            cwd=self.root,
            env=self.environment,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            check=False,
        )
        self.assertEqual(0, seeded.returncode, seeded.stdout + seeded.stderr)
        seed_result = self.parse_command_result(seeded)
        self.assertTrue(seed_result["ok"])
        self.assertEqual(2, seed_result["payload"]["operationCount"])
        seeded_sound = self.read_json(sound_target)
        self.assertEqual(
            [old_sound_occurrence_id],
            [
                cue["occurrenceId"]
                for cue in seeded_sound["cues"]
                if cue.get("patternId") == "VALTAN_DASH_CHARGE_GROGGY"
                and cue.get("stageId") == "GROGGY"
            ],
        )

        self.repository_revision = self.source_manifest()["sourceRevision"]
        sound_baseline_path = self.root / "dash-charge-sound-baseline.json"
        sound_candidate_path = self.root / "dash-charge-sound-candidate.json"
        sound_baseline_path.write_bytes(sound_target.read_bytes())
        sound_candidate = self.read_json(sound_target)
        removed_sound_occurrences = [
            cue["occurrenceId"]
            for cue in sound_candidate["cues"]
            if cue.get("patternId") == "VALTAN_DASH_CHARGE_GROGGY"
            and cue.get("stageId") == "GROGGY"
            and cue.get("clipOccurrenceId") == old_clip_occurrence_id
        ]
        self.assertEqual([old_sound_occurrence_id], removed_sound_occurrences)
        sound_candidate["cues"] = [
            cue
            for cue in sound_candidate["cues"]
            if cue.get("occurrenceId") not in removed_sound_occurrences
        ]
        sound_candidate_path.write_text(
            json.dumps(sound_candidate, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )

        clips = [
            "mesh_abn_groggy_1_start",
            "mesh_abn_groggy_1_loop",
            "mesh_abn_groggy_1_loop",
            "mesh_abn_groggy_1_loop",
            "mesh_abn_groggy_1_end",
        ]
        play_ms = [1833, 1333, 1333, 334, 2000]
        patch_path = self.write_patch(
            "dash-charge-recovery-replace-with-groggy.json",
            self.repository_revision,
            [
                {
                    "op": "SET_STAGE_DURATION",
                    "patternId": "VALTAN_DASH_CHARGE_GROGGY",
                    "stageId": "GROGGY",
                    "durationMs": 6833,
                },
                {
                    "op": "SET_STAGE_ANIMATION",
                    "patternId": "VALTAN_DASH_CHARGE_GROGGY",
                    "stageId": "GROGGY",
                    "animation": {
                        "endPolicy": "EXACT",
                        "repeatCount": 1,
                        "occurrences": [
                            {
                                "clipOccurrenceId": (
                                    "valtan.attack.dash-charge.recovery."
                                    f"groggy.clip.{index:02d}"
                                ),
                                "clip": clip,
                                "mappingBasis": "PROJECT_AUTHORED",
                                "sourceStartMs": 0,
                                "playMs": duration_ms,
                                "playRate": 1.0,
                                "repeatUntilStageEnd": False,
                            }
                            for index, (clip, duration_ms) in enumerate(
                                zip(clips, play_ms), 1
                            )
                        ],
                    },
                },
            ],
        )
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
                "-PatternSoundBaselinePath",
                str(sound_baseline_path),
                "-PatternSoundCandidatePath",
                str(sound_candidate_path),
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
        self.assertEqual(2, result["payload"]["operationCount"])

        committed_presentation = self.read_json(presentation_target)
        committed_groggy_pattern = next(
            pattern
            for pattern in committed_presentation["patterns"]
            if pattern["patternId"] == "VALTAN_DASH_CHARGE_GROGGY"
        )
        self.assertIn(
            {
                "sourceActionId": 400430,
                "sequenceIndex": 0,
                "role": "PRIMARY",
            },
            committed_groggy_pattern["presentationSources"],
        )
        committed_groggy = next(
            stage
            for stage in committed_groggy_pattern["stages"]
            if stage["stageId"] == "GROGGY"
        )
        self.assertEqual(
            clips,
            [row["clip"] for row in committed_groggy["animation"]["occurrences"]],
        )
        self.assertEqual(
            play_ms,
            [
                row["playMs"]
                for row in committed_groggy["animation"]["occurrences"]
            ],
        )
        self.assertNotIn(
            "mesh_att_battle_4_01",
            [row["clip"] for row in committed_groggy["animation"]["occurrences"]],
        )

        committed_gameplay = self.read_json(gameplay_target)
        committed_gameplay_groggy_pattern = next(
            pattern
            for pattern in committed_gameplay["patterns"]
            if pattern["patternId"] == "VALTAN_DASH_CHARGE_GROGGY"
        )
        committed_gameplay_groggy = next(
            stage
            for stage in committed_gameplay_groggy_pattern["stages"]
            if stage["stageId"] == "GROGGY"
        )
        self.assertEqual(6833, committed_gameplay_groggy["durationMs"])
        self.assertEqual(
            [400430],
            committed_gameplay_groggy_pattern["sourceActionIds"],
        )

        committed_sound = self.read_json(sound_target)
        self.assertFalse(
            any(
                cue.get("occurrenceId") in removed_sound_occurrences
                for cue in committed_sound["cues"]
            )
        )
        self.assertFalse(
            any(
                cue.get("patternId") == "VALTAN_DASH_CHARGE_GROGGY"
                and cue.get("stageId") == "GROGGY"
                for cue in committed_sound["cues"]
            )
        )

    @unittest.skipUnless(
        os.name == "nt", "shared canonical reader contract is Windows-only"
    )
    def test_public_wrapper_waits_for_short_reader_then_commits(self) -> None:
        patch_path = self.write_patch(
            "short-reader.json", self.repository_revision, []
        )
        powershell = shutil.which("powershell.exe") or shutil.which("powershell")
        self.assertIsNotNone(powershell)
        holder = self.start_shared_reader(2.0)
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
        holder_stdout, holder_stderr = holder.communicate(timeout=5.0)

        self.assertEqual(
            0,
            holder.returncode,
            f"stdout={holder_stdout!r} stderr={holder_stderr!r}",
        )
        self.assertEqual(0, completed.returncode, completed.stdout + completed.stderr)
        result = self.parse_command_result(completed)
        self.assertTrue(result["ok"])
        self.assertEqual("COMMIT_CANONICAL_DRAFT", result["command"])
        self.assertEqual(0, result["payload"]["operationCount"])
        self.assertEqual("NOT_ACTIVATED", result["payload"]["runtimeActivation"])

    @unittest.skipUnless(
        os.name == "nt", "shared canonical reader contract is Windows-only"
    )
    def test_public_wrapper_lock_timeout_preserves_every_owner_byte(self) -> None:
        patch_path = self.write_patch(
            "lock-timeout.json", self.repository_revision, self.typed_operations()
        )
        baseline = self.data_manifest()
        powershell = shutil.which("powershell.exe") or shutil.which("powershell")
        self.assertIsNotNone(powershell)
        holder = self.start_shared_reader(30.0)
        try:
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
                    "-LockTimeoutSeconds",
                    "0.1",
                ],
                cwd=self.root,
                env=self.environment,
                capture_output=True,
                text=True,
                encoding="utf-8",
                errors="replace",
                check=False,
            )
        finally:
            if holder.poll() is None:
                holder.terminate()
            holder_stdout, holder_stderr = holder.communicate(timeout=5.0)

        self.assertNotEqual(0, completed.returncode, completed.stdout)
        result = self.parse_command_result(completed)
        self.assertFalse(result["ok"])
        self.assertEqual(
            "CANONICAL_TRANSACTION_BUSY",
            result["errors"][0]["errorCode"],
        )
        self.assertIn(
            "transaction lock is held by another process",
            result["errors"][0]["message"],
        )
        self.assertEqual(baseline, self.data_manifest())
        self.assertEqual("", holder_stdout)
        self.assertEqual("", holder_stderr)


if __name__ == "__main__":
    unittest.main()
