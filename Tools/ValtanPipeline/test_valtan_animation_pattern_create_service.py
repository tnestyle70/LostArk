#!/usr/bin/env python3
from __future__ import annotations

import contextlib
import hashlib
import io
import json
import os
import shutil
import subprocess
import sys
import tempfile
import time
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import promote_valtan_animation_chains as promotion


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
FAKE_PRODUCT_REL = "Data/Encounters/Valtan/CreatePattern.fixture.json"


def _link_or_copy(source: Path, target: Path) -> None:
    try:
        os.link(source, target)
    except OSError:
        shutil.copyfile(source, target)


class ValtanAnimationPatternCreateServiceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        model_source = REPOSITORY_ROOT / (
            "Client/Bin/Resources/Character/Valtan/AnimSets/"
            "MN_RPBF_01_AnimSet.wmodel"
        )
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            model = root / "Client/Bin/Resources/Character/Valtan/AnimSets/MN_RPBF_01_AnimSet.wmodel"
            model.parent.mkdir(parents=True)
            _link_or_copy(model_source, model)
            cls.root_curves = promotion._load_root_curves(root, model)
        if "mesh_att_battle_12_01" not in cls.root_curves:
            raise AssertionError("actual Valtan WModel fixture is missing its known clip")

    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        for relative in (
            promotion.DEBUG_REL,
            promotion.MANIFEST_REL,
            promotion.GAMEPLAY_REL,
            promotion.PRESENTATION_REL,
            promotion.RECEIPT_REL,
            promotion.ROOT_MOTION_REL,
            promotion.ANIM_NOTIFY_REL,
        ):
            source = REPOSITORY_ROOT / relative
            target = self.root / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source, target)
        model_relative = (
            "Client/Bin/Resources/Character/Valtan/AnimSets/"
            "MN_RPBF_01_AnimSet.wmodel"
        )
        model = self.root / model_relative
        model.parent.mkdir(parents=True, exist_ok=True)
        _link_or_copy(REPOSITORY_ROOT / model_relative, model)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def request(
        self,
        *,
        pattern_id: str = "VALTAN_WORKBENCH_NEW_PATTERN",
        saved: bool = False,
        clip: str = "mesh_att_battle_12_01",
    ) -> dict:
        source_sha256 = hashlib.sha256(
            (self.root / promotion.DEBUG_REL).read_bytes()
        ).hexdigest()
        intake_chain = (
            {
                "selectionKind": "SAVED_INTAKE_CHAIN",
                "sourceChainId": "front-back-front",
            }
            if saved
            else {
                "selectionKind": "CURRENT_CHAIN",
                "sourceActionId": 420612,
                "sourceSequenceIndex": 3,
                "chain": {
                    "chainId": "workbench-current-chain",
                    "targetPatternId": "",
                    "targetStageId": "",
                    "animation": {
                        "endPolicy": "NATIVE_CLIP_LENGTHS",
                        "repeatCount": 1,
                        "occurrences": [
                            {
                                "clipOccurrenceId": "valtan.workbench.current.clip.01",
                                "clip": clip,
                                "mappingBasis": "PROJECT_AUTHORED",
                                "sourceStartMs": 0,
                                "playMs": 0,
                                "playRate": 1.0,
                                "repeatUntilStageEnd": False,
                            }
                        ],
                    },
                },
            }
        )
        return {
            "schema": promotion.CREATE_REQUEST_SCHEMA,
            "formatVersion": promotion.CREATE_REQUEST_FORMAT_VERSION,
            "expectedSourceSha256": source_sha256,
            "patternId": pattern_id,
            "displayName": "애니메이션 워크벤치 신규 패턴",
            "authoringPhase": 2,
            "targetPolicy": "NONE",
            "aimPolicy": "NONE",
            "intakeChain": intake_chain,
        }

    @contextlib.contextmanager
    def focused_pipeline(self):
        def project(
            _root: Path,
            gameplay: dict,
            _presentation: dict,
            **staged_lineage: dict,
        ) -> dict[str, str]:
            self.assertIn("debug_document", staged_lineage)
            self.assertIn("promotion_manifest", staged_lineage)
            pattern_id = next(
                row["patternId"]
                for row in gameplay["patterns"]
                if row["patternId"] == "VALTAN_WORKBENCH_NEW_PATTERN"
            )
            root_motion = promotion._read_json(
                self.root / promotion.ROOT_MOTION_REL
            )
            root_motion["patterns"].append(
                {"patternId": pattern_id, "stages": []}
            )
            return {
                FAKE_PRODUCT_REL: json.dumps(
                    {
                        "schema": "lostark.valtan-create-pattern-fixture",
                        "formatVersion": 1,
                        "patternId": pattern_id,
                        "selectionMode": "AUDITION_ONLY",
                    },
                    sort_keys=True,
                )
                + "\n",
                promotion.ROOT_MOTION_REL: json.dumps(
                    root_motion,
                    ensure_ascii=False,
                    indent=2,
                )
                + "\n",
            }

        with mock.patch.object(
            promotion, "_load_root_curves", return_value=self.root_curves
        ), mock.patch.object(
            promotion,
            "_transaction_projection_relatives",
            return_value=(FAKE_PRODUCT_REL, promotion.ROOT_MOTION_REL),
        ), mock.patch.object(
            promotion, "validate_and_project", side_effect=project
        ):
            yield

    def prepare(self, request: dict) -> tuple[dict, dict, dict]:
        with self.focused_pipeline():
            return promotion.prepare_create_pattern_transaction(self.root, request)

    @staticmethod
    def exact_bytes(paths: dict[Path, bytes | None] | list[Path]) -> dict[Path, bytes | None]:
        iterable = paths.keys() if isinstance(paths, dict) else paths
        return {path: promotion._read_bytes_or_none(path) for path in iterable}

    def test_current_chain_stages_strict_default_audition_contract(self) -> None:
        targets, _baselines, result = self.prepare(self.request())
        debug_payload = targets[self.root / promotion.DEBUG_REL]
        manifest = json.loads(targets[self.root / promotion.MANIFEST_REL])
        gameplay = json.loads(targets[self.root / promotion.GAMEPLAY_REL])
        presentation = json.loads(targets[self.root / promotion.PRESENTATION_REL])
        receipt = json.loads(targets[self.root / promotion.RECEIPT_REL])
        product = json.loads(targets[self.root / FAKE_PRODUCT_REL])
        root_motion = json.loads(
            targets[self.root / promotion.ROOT_MOTION_REL]
        )
        pattern = next(
            row for row in gameplay["patterns"]
            if row["patternId"] == "VALTAN_WORKBENCH_NEW_PATTERN"
        )
        presented = next(
            row for row in presentation["patterns"]
            if row["patternId"] == "VALTAN_WORKBENCH_NEW_PATTERN"
        )
        manual = next(
            row for row in gameplay["decisionModel"]["manualAuditions"]
            if row["patternId"] == "VALTAN_WORKBENCH_NEW_PATTERN"
        )
        receipt_pattern = next(
            row for row in receipt["patterns"]
            if row["patternId"] == "VALTAN_WORKBENCH_NEW_PATTERN"
        )

        self.assertEqual("MANUAL_SERVER_AUDITION", manual["admissionState"])
        self.assertEqual("AUDITION_ONLY", product["selectionMode"])
        self.assertIn(
            "VALTAN_WORKBENCH_NEW_PATTERN",
            {row["patternId"] for row in root_motion["patterns"]},
        )
        self.assertEqual("NONE", pattern["targetPolicy"])
        self.assertEqual("NONE", pattern["aimPolicy"])
        self.assertIsNone(pattern["serverMotion"])
        self.assertEqual([], pattern["reactions"])
        self.assertTrue(pattern["stages"])
        for stage in pattern["stages"]:
            self.assertEqual("NONE", stage["hit"]["shape"]["kind"])
            self.assertIsNone(stage["motion"])
            self.assertEqual([], stage["events"])
            self.assertEqual([], stage["branches"])
        self.assertEqual(
            ["mesh_att_battle_12_01"],
            [
                occurrence["clip"]
                for stage in presented["stages"]
                for occurrence in stage["animation"]["occurrences"]
            ],
        )
        self.assertEqual(3, presented["sourceSequenceIndex"])
        self.assertEqual(
            {
                "sourceActionId": 420612,
                "sequenceIndex": 3,
                "role": "PRIMARY",
            },
            presented["presentationSources"][0],
        )
        promoted = next(
            row for row in manifest["patterns"]
            if row["patternId"] == "VALTAN_WORKBENCH_NEW_PATTERN"
        )
        self.assertEqual(420612, promoted["sourceActionId"])
        self.assertEqual(3, promoted["sourceSequenceIndex"])
        self.assertEqual(
            promotion._lround_positive(
                self.root_curves["mesh_att_battle_12_01"][0]
            ),
            receipt_pattern["occurrences"][0]["nativeSourceMs"],
        )
        self.assertEqual(
            hashlib.sha256(debug_payload).hexdigest(),
            manifest["sourceDocument"]["sha256"],
        )
        self.assertEqual(
            manifest["sourceDocument"], receipt["sourceDocument"]
        )
        self.assertEqual("AUDITION_ONLY", result["selectionMode"])

    def test_apply_commits_the_full_authoring_and_product_transaction(self) -> None:
        request_path = self.root / "CreatePattern.request.json"
        request_path.write_text(
            json.dumps(self.request(), ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        with self.focused_pipeline():
            result = promotion.create_pattern_from_request(
                self.root, request_path, "Apply"
            )

        manifest = promotion._read_json(self.root / promotion.MANIFEST_REL)
        gameplay = promotion._read_json(self.root / promotion.GAMEPLAY_REL)
        product = promotion._read_json(self.root / FAKE_PRODUCT_REL)
        self.assertEqual(promotion.CREATE_RESULT_SCHEMA, result["schema"])
        self.assertEqual(
            promotion.CREATE_RESULT_FORMAT_VERSION, result["formatVersion"]
        )
        self.assertEqual("Apply", result["mode"])
        self.assertEqual(
            hashlib.sha256((self.root / promotion.DEBUG_REL).read_bytes()).hexdigest(),
            manifest["sourceDocument"]["sha256"],
        )
        self.assertIn(
            "VALTAN_WORKBENCH_NEW_PATTERN",
            [row["patternId"] for row in gameplay["patterns"]],
        )
        self.assertEqual("AUDITION_ONLY", product["selectionMode"])

    def test_saved_intake_is_promoted_in_debug_order(self) -> None:
        targets, _baselines, result = self.prepare(self.request(saved=True))
        manifest = json.loads(targets[self.root / promotion.MANIFEST_REL])
        debug = json.loads(targets[self.root / promotion.DEBUG_REL])
        self.assertNotIn(
            "front-back-front",
            [row["sourceChainId"] for row in manifest["animationIntakeOnly"]],
        )
        promoted_ids = [row["sourceChainId"] for row in manifest["patterns"]]
        debug_ids = [row["chainId"] for row in debug["chains"]]
        self.assertIn("front-back-front", promoted_ids)
        self.assertEqual(
            promoted_ids,
            [chain_id for chain_id in debug_ids if chain_id in set(promoted_ids)],
        )
        self.assertEqual("front-back-front", result["sourceChainId"])

    def test_duplicate_and_retired_pattern_ids_are_rejected(self) -> None:
        for pattern_id, error in (
            ("VALTAN_WARP", "duplicate patternId"),
            ("VALTAN_SEQUENCE_FRONT_BACK_FRONT", "patternId is retired"),
        ):
            with self.subTest(pattern_id=pattern_id), self.assertRaisesRegex(
                promotion.PromotionError, error
            ):
                self.prepare(self.request(pattern_id=pattern_id))

    def test_invalid_request_version_and_source_drift_are_rejected(self) -> None:
        invalid_version = self.request()
        invalid_version["formatVersion"] = 2
        with self.assertRaisesRegex(promotion.PromotionError, "header/version"):
            self.prepare(invalid_version)

        drifted = self.request()
        drifted["expectedSourceSha256"] = "0" * 64
        with self.assertRaisesRegex(promotion.PromotionError, "source drift"):
            self.prepare(drifted)

    def test_exact_source_tuple_is_paired_and_allows_sequence_zero(self) -> None:
        sequence_zero = self.request()
        sequence_zero["intakeChain"]["sourceSequenceIndex"] = 0
        targets, _baselines, _result = self.prepare(sequence_zero)
        presentation = json.loads(targets[self.root / promotion.PRESENTATION_REL])
        pattern = next(
            row for row in presentation["patterns"]
            if row["patternId"] == "VALTAN_WORKBENCH_NEW_PATTERN"
        )
        self.assertEqual(0, pattern["sourceSequenceIndex"])
        self.assertEqual(0, pattern["presentationSources"][0]["sequenceIndex"])

        missing_pair = self.request(pattern_id="VALTAN_WORKBENCH_MISSING_PAIR")
        del missing_pair["intakeChain"]["sourceSequenceIndex"]
        with self.assertRaisesRegex(
            promotion.PromotionError, "must be authored together"
        ):
            self.prepare(missing_pair)

    def test_exact_selected_action_owns_a_clip_reused_by_another_notify_action(self) -> None:
        ground_tick = self.request(clip="mesh_att_battle_11_01")
        ground_tick["intakeChain"]["sourceActionId"] = 400440
        ground_tick["intakeChain"]["sourceSequenceIndex"] = 0
        targets, _baselines, _result = self.prepare(ground_tick)
        presentation = json.loads(targets[self.root / promotion.PRESENTATION_REL])
        receipt = json.loads(targets[self.root / promotion.RECEIPT_REL])
        pattern = next(
            row for row in presentation["patterns"]
            if row["patternId"] == "VALTAN_WORKBENCH_NEW_PATTERN"
        )
        receipt_pattern = next(
            row for row in receipt["patterns"]
            if row["patternId"] == "VALTAN_WORKBENCH_NEW_PATTERN"
        )
        self.assertEqual(
            {
                "sourceActionId": 400440,
                "sequenceIndex": 0,
                "role": "PRIMARY",
            },
            pattern["presentationSources"][0],
        )
        self.assertEqual(
            400440, receipt_pattern["occurrences"][0]["sourceActionId"]
        )

    def test_unknown_wmodel_clip_is_rejected_before_projection(self) -> None:
        with self.assertRaisesRegex(
            promotion.PromotionError, "clip is absent from the reviewed WModel"
        ):
            self.prepare(self.request(clip="mesh_missing_workbench_clip"))

    def test_compare_and_swap_rejects_late_source_change_without_partial_commit(self) -> None:
        targets, baselines, _result = self.prepare(self.request())
        source_path = self.root / promotion.DEBUG_REL
        external_payload = source_path.read_bytes() + b" "
        source_path.write_bytes(external_payload)
        with self.assertRaisesRegex(promotion.PromotionError, "target changed"):
            promotion._atomic_commit(
                targets, expected_baselines=baselines
            )
        self.assertEqual(external_payload, source_path.read_bytes())
        for path, baseline in baselines.items():
            if path == source_path:
                continue
            self.assertEqual(baseline, promotion._read_bytes_or_none(path))

    def test_compare_and_swap_rejects_late_product_change_without_partial_commit(self) -> None:
        targets, baselines, _result = self.prepare(self.request())
        product_path = self.root / FAKE_PRODUCT_REL
        product_path.parent.mkdir(parents=True, exist_ok=True)
        external_payload = b'{"external":true}\n'
        product_path.write_bytes(external_payload)
        with self.assertRaisesRegex(promotion.PromotionError, "target changed"):
            promotion._atomic_commit(targets, expected_baselines=baselines)
        self.assertEqual(external_payload, product_path.read_bytes())
        for path, baseline in baselines.items():
            if path == product_path:
                continue
            self.assertEqual(baseline, promotion._read_bytes_or_none(path))

    def test_injected_commit_failure_rolls_back_every_authoring_and_product_target(self) -> None:
        request_path = self.root / "CreatePattern.request.json"
        request_path.write_text(
            json.dumps(self.request(), ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        with self.focused_pipeline():
            targets, baselines, _result = promotion.prepare_create_pattern_transaction(
                self.root, self.request()
            )
            with self.assertRaisesRegex(
                promotion.PromotionError, "injected promotion commit failure"
            ):
                promotion.create_pattern_from_request(
                    self.root,
                    request_path,
                    "Apply",
                    inject_failure_after=len(targets),
                )
        self.assertEqual(
            baselines,
            {path: promotion._read_bytes_or_none(path) for path in targets},
        )
        self.assertFalse(list(self.root.rglob("*.tmp")))

    def test_general_apply_commits_candidate_root_motion_with_products(self) -> None:
        root_motion = promotion._read_json(
            self.root / promotion.ROOT_MOTION_REL
        )
        root_motion["patterns"].append(
            {"patternId": "VALTAN_GENERAL_APPLY_FIXTURE", "stages": []}
        )
        outputs = {
            FAKE_PRODUCT_REL: json.dumps(
                {"schema": "fixture", "formatVersion": 1}
            )
            + "\n",
            promotion.ROOT_MOTION_REL: json.dumps(
                root_motion, ensure_ascii=False, indent=2
            )
            + "\n",
        }
        receipt = {"patternCount": 1, "stageCount": 1, "patterns": [
            {"occurrences": [{}]}
        ]}
        with mock.patch.object(
            promotion,
            "build_candidates",
            return_value=(
                {"candidate": "gameplay"},
                {"candidate": "presentation"},
                receipt,
            ),
        ), mock.patch.object(
            promotion, "validate_and_project", return_value=outputs
        ):
            result = promotion.run(self.root, "Apply")

        self.assertEqual(2, result["projectedArtifactCount"])
        committed_root_motion = promotion._read_json(
            self.root / promotion.ROOT_MOTION_REL
        )
        self.assertIn(
            "VALTAN_GENERAL_APPLY_FIXTURE",
            {row["patternId"] for row in committed_root_motion["patterns"]},
        )

    def test_replace_and_primary_rollback_failure_still_restore_exact_baselines(self) -> None:
        targets, baselines, _result = self.prepare(self.request())
        with self.assertRaisesRegex(
            promotion.PromotionError, "injected promotion replace failure"
        ):
            promotion._atomic_commit(
                targets,
                expected_baselines=baselines,
                repository_root=self.root,
                inject_replace_failure_at=2,
                inject_rollback_failure_at=1,
            )
        self.assertEqual(baselines, self.exact_bytes(targets))
        generation_root = self.root / promotion.TRANSACTION_GENERATION_ROOT_REL
        self.assertFalse(generation_root.exists() and any(generation_root.iterdir()))

    def test_cross_process_lock_contention_never_mutates_an_owner(self) -> None:
        request_path = self.root / "CreatePattern.request.json"
        request_path.write_text(
            json.dumps(self.request(), ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        targets, baselines, _result = self.prepare(self.request())
        ready = self.root / "lock-holder.ready"
        module_directory = Path(promotion.__file__).resolve().parent
        holder_program = "\n".join(
            (
                "import sys, time",
                "from pathlib import Path",
                f"sys.path.insert(0, {str(module_directory)!r})",
                "import promote_valtan_animation_chains as promotion",
                f"root = Path({str(self.root)!r})",
                f"ready = Path({str(ready)!r})",
                "with promotion._exclusive_transaction_lock(root):",
                "    ready.write_text('locked', encoding='utf-8')",
                "    time.sleep(30)",
            )
        )
        holder = subprocess.Popen([sys.executable, "-c", holder_program])
        try:
            deadline = time.monotonic() + 10.0
            while not ready.exists() and holder.poll() is None and time.monotonic() < deadline:
                time.sleep(0.025)
            self.assertTrue(ready.exists(), "real lock-holder process did not acquire the lock")
            with self.focused_pipeline(), self.assertRaisesRegex(
                promotion.PromotionError, "lock is held by another process"
            ):
                promotion.create_pattern_from_request(
                    self.root,
                    request_path,
                    "Apply",
                    lock_timeout_seconds=0.0,
                )
            self.assertEqual(baselines, self.exact_bytes(targets))
        finally:
            holder.terminate()
            try:
                holder.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                holder.kill()
                holder.wait(timeout=5.0)

    def test_hard_crash_is_recovered_to_byte_identical_baselines(self) -> None:
        first = self.root / "Data/Encounters/Valtan/CrashRecoveryA.json"
        second = self.root / "Data/Encounters/Valtan/CrashRecoveryB.json"
        first.parent.mkdir(parents=True, exist_ok=True)
        first.write_bytes(b'{"generation":"old-a"}\n')
        second.write_bytes(b'{"generation":"old-b"}\n')
        baselines = self.exact_bytes([first, second])
        module_directory = Path(promotion.__file__).resolve().parent
        crash_program = "\n".join(
            (
                "import sys",
                "from pathlib import Path",
                f"sys.path.insert(0, {str(module_directory)!r})",
                "import promote_valtan_animation_chains as promotion",
                f"root = Path({str(self.root)!r})",
                f"first = Path({str(first)!r})",
                f"second = Path({str(second)!r})",
                "promotion._atomic_commit(",
                "    {first: b'{\"generation\":\"new-a\"}\\n',",
                "     second: b'{\"generation\":\"new-b\"}\\n'},",
                "    repository_root=root, inject_hard_crash_after=1)",
            )
        )
        crashed = subprocess.run(
            [sys.executable, "-c", crash_program],
            check=False,
            capture_output=True,
            text=True,
            timeout=15.0,
        )
        self.assertEqual(97, crashed.returncode, crashed.stderr)
        self.assertTrue((self.root / promotion.TRANSACTION_ACTIVE_REL).exists())
        self.assertNotEqual(baselines, self.exact_bytes([first, second]))

        # The next real lock owner performs recovery before it can stage work.
        with promotion._exclusive_transaction_lock(self.root):
            pass
        self.assertEqual(baselines, self.exact_bytes([first, second]))
        self.assertFalse((self.root / promotion.TRANSACTION_ACTIVE_REL).exists())
        generation_root = self.root / promotion.TRANSACTION_GENERATION_ROOT_REL
        self.assertFalse(generation_root.exists() and any(generation_root.iterdir()))

    def test_projected_publisher_commit_is_source_cas_and_byte_safe(self) -> None:
        product_relatives = (
            "Data/Encounters/Valtan/PublisherFixtureA.json",
            "Data/Animation/Authored/Valtan/PublisherFixtureB.json",
        )
        projection = (
            self.root
            / "Intermediate/ValtanProductProjection/publisher-fixture-generation"
        )
        for ordinal, relative in enumerate(product_relatives, start=1):
            target = self.root / relative
            projected = projection / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            projected.parent.mkdir(parents=True, exist_ok=True)
            target.write_text(
                json.dumps({"generation": "old", "ordinal": ordinal}) + "\n",
                encoding="utf-8",
            )
            projected.write_text(
                json.dumps({"generation": "new", "ordinal": ordinal}) + "\n",
                encoding="utf-8",
            )

        class FakePipeline:
            class PipelineError(RuntimeError):
                pass

            manifest_id = "a" * 64

            @classmethod
            def source_manifest(cls, _root: Path) -> dict[str, str]:
                return {"sourceManifestId": cls.manifest_id}

        with mock.patch.object(
            promotion, "_load_v2_pipeline", return_value=FakePipeline
        ), mock.patch.object(
            promotion,
            "_product_projection_relatives",
            return_value=product_relatives,
        ):
            result = promotion.commit_projected_products(
                self.root, projection, "a" * 64
            )
        self.assertEqual(promotion.PRODUCT_COMMIT_RESULT_SCHEMA, result["schema"])
        self.assertEqual(2, result["artifactCount"])
        self.assertEqual(2, result["changedCount"])
        committed = self.exact_bytes([self.root / row for row in product_relatives])
        projected = self.exact_bytes([projection / row for row in product_relatives])
        self.assertEqual(
            list(projected.values()),
            list(committed.values()),
        )

        before = dict(committed)
        FakePipeline.manifest_id = "b" * 64
        for ordinal, relative in enumerate(product_relatives, start=1):
            (projection / relative).write_text(
                json.dumps({"generation": "stale", "ordinal": ordinal}) + "\n",
                encoding="utf-8",
            )
        with mock.patch.object(
            promotion, "_load_v2_pipeline", return_value=FakePipeline
        ), mock.patch.object(
            promotion,
            "_product_projection_relatives",
            return_value=product_relatives,
        ), self.assertRaisesRegex(
            promotion.PromotionError, "sources changed before Product commit"
        ):
            promotion.commit_projected_products(
                self.root, projection, "a" * 64
            )
        self.assertEqual(before, self.exact_bytes(list(before)))

    def test_publisher_cli_contends_on_the_create_pattern_process_lock(self) -> None:
        projection = self.root / "Intermediate/ValtanProductProjection/lock-fixture"
        projection.mkdir(parents=True)
        ready = self.root / "publisher-lock-holder.ready"
        module_directory = Path(promotion.__file__).resolve().parent
        holder_program = "\n".join(
            (
                "import sys, time",
                "from pathlib import Path",
                f"sys.path.insert(0, {str(module_directory)!r})",
                "import promote_valtan_animation_chains as promotion",
                f"root = Path({str(self.root)!r})",
                f"ready = Path({str(ready)!r})",
                "with promotion._exclusive_transaction_lock(root):",
                "    ready.write_text('locked', encoding='utf-8')",
                "    time.sleep(30)",
            )
        )
        holder = subprocess.Popen([sys.executable, "-c", holder_program])
        try:
            deadline = time.monotonic() + 10.0
            while not ready.exists() and holder.poll() is None and time.monotonic() < deadline:
                time.sleep(0.025)
            self.assertTrue(ready.exists(), "real Create writer did not acquire the lock")
            command = [
                sys.executable,
                str(Path(promotion.__file__).resolve()),
                "--repo-root",
                str(self.root),
                "--mode",
                "CommitProjectedProducts",
                "--projected-product-root",
                str(projection),
                "--expected-source-manifest-id",
                "a" * 64,
                "--lock-timeout-seconds",
                "0",
            ]
            blocked = subprocess.run(
                command,
                check=False,
                capture_output=True,
                text=True,
                timeout=10.0,
            )
            self.assertEqual(1, blocked.returncode)
            self.assertIn(
                "transaction lock is held by another process", blocked.stderr
            )
        finally:
            holder.terminate()
            try:
                holder.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                holder.kill()
                holder.wait(timeout=5.0)

    def test_publish_v2_delegates_product_mutation_to_shared_transaction(self) -> None:
        source = (
            REPOSITORY_ROOT / "Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1"
        ).read_text(encoding="utf-8-sig")
        publish_v2 = source.split("if ($Mode -eq 'PublishV2')", 1)[1].split(
            "$masterFile =", 1
        )[0]
        self.assertIn("promote_valtan_animation_chains.py", publish_v2)
        self.assertIn("CommitProjectedProducts", publish_v2)
        self.assertIn("--expected-source-manifest-id", publish_v2)
        self.assertIn("--lock-timeout-seconds", publish_v2)
        self.assertNotIn("Commit-StagedDocuments", publish_v2)

    def test_malformed_saved_occurrence_rejects_the_whole_intake_byte_identically(self) -> None:
        debug_path = self.root / promotion.DEBUG_REL
        manifest_path = self.root / promotion.MANIFEST_REL
        debug = promotion._read_json(debug_path)
        del debug["chains"][0]["animation"]["occurrences"][0]["playMs"]
        debug_path.write_text(
            json.dumps(debug, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        manifest = promotion._read_json(manifest_path)
        manifest["sourceDocument"]["sha256"] = hashlib.sha256(
            debug_path.read_bytes()
        ).hexdigest()
        manifest_path.write_text(
            json.dumps(manifest, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        request_path = self.root / "CreatePattern.request.json"
        request_path.write_text(
            json.dumps(self.request(), ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        owner_paths = [
            self.root / relative
            for relative in (
                promotion.DEBUG_REL,
                promotion.MANIFEST_REL,
                promotion.GAMEPLAY_REL,
                promotion.PRESENTATION_REL,
                promotion.RECEIPT_REL,
            )
        ]
        before = self.exact_bytes(owner_paths)
        with self.focused_pipeline(), self.assertRaisesRegex(
            promotion.PromotionError, "properties mismatch"
        ):
            promotion.create_pattern_from_request(
                self.root, request_path, "Apply"
            )
        self.assertEqual(before, self.exact_bytes(owner_paths))

    def test_workbench_intake_parser_is_strict_and_save_never_deletes_destination(self) -> None:
        source = (REPOSITORY_ROOT / "Client/Private/Animation_Tool.cpp").read_text(
            encoding="utf-8"
        )
        parser = source.split(
            "bool_t Client::CAnimation_Tool::Parse_CustomChainDocument", 1
        )[1].split(
            "bool_t Client::CAnimation_Tool::Load_CustomChainLibrary", 1
        )[0]
        save = source.split(
            "bool_t Client::CAnimation_Tool::Save_CustomChainLibrary", 1
        )[1].split(
            "void Client::CAnimation_Tool::Render_ValtanPatternCreatePanel", 1
        )[0]
        self.assertNotIn("continue;", parser)
        self.assertIn("invalid/duplicate identity, target, or animation", parser)
        self.assertIn("is not an exact object", parser)
        self.assertNotIn("remove(destination", save)
        self.assertIn("MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH", save)
        self.assertIn("SCOPED_VALTAN_PATTERN_TRANSACTION_LOCK", save)
        self.assertIn("Save rejected before mutation", save)
        self.assertIn(
            'L"out\\\\ValtanPatternTransactions\\\\create-pattern.lock"', source
        )

    def test_cli_request_file_routes_to_the_transaction_service(self) -> None:
        request_path = self.root / "CreatePattern.request.json"
        request_path.write_text("{}\n", encoding="utf-8")
        expected = {
            "schema": promotion.CREATE_RESULT_SCHEMA,
            "formatVersion": promotion.CREATE_RESULT_FORMAT_VERSION,
            "mode": "Validate",
            "patternId": "VALTAN_WORKBENCH_NEW_PATTERN",
        }
        output = io.StringIO()
        with mock.patch.object(
            sys,
            "argv",
            [
                "promote_valtan_animation_chains.py",
                "--repo-root",
                str(self.root),
                "--request-file",
                str(request_path),
                "--mode",
                "Validate",
            ],
        ), mock.patch.object(
            promotion, "create_pattern_from_request", return_value=expected
        ) as create, contextlib.redirect_stdout(output):
            self.assertEqual(0, promotion.main())
        create.assert_called_once_with(
            self.root,
            request_path,
            "Validate",
            inject_failure_after=None,
        )
        self.assertEqual(expected, json.loads(output.getvalue()))


if __name__ == "__main__":
    unittest.main()
