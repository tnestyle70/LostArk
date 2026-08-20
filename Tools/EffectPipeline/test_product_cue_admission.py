#!/usr/bin/env python3

from __future__ import annotations

import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import shlex
import tempfile
import unittest
from unittest import mock


MODULE_PATH = Path(__file__).with_name("product_cue_admission.py")
SPEC = importlib.util.spec_from_file_location("product_cue_admission", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
admission = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(admission)

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
SOURCE_DATA_ROOT = REPOSITORY_ROOT / "Data"


def _write_json(path: Path, value: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(
        (json.dumps(value, ensure_ascii=False, indent=2) + "\n").encode("utf-8")
    )


class ProductCueAdmissionTests(unittest.TestCase):
    def setUp(self) -> None:
        self._temporary = tempfile.TemporaryDirectory()
        self.data_root = Path(self._temporary.name) / "Data"
        self.runtime_catalog_path = Path(self._temporary.name) / "EffectCatalog.runtime.json"
        self.receipt_path = Path(self._temporary.name) / "ProductCueAdmissions.runtime.json"

        source_policy = json.loads(
            (SOURCE_DATA_ROOT / "Effects/ProductCueApprovals.json").read_text(
                encoding="utf-8"
            )
        )
        self.assertEqual(5, len(source_policy["approvals"]))
        (self.data_root / "Balance").mkdir(parents=True, exist_ok=True)
        shutil.copyfile(
            SOURCE_DATA_ROOT / "Balance/PlayerSkills.json",
            self.data_root / "Balance/PlayerSkills.json",
        )

        source_catalog = json.loads(
            (SOURCE_DATA_ROOT / "Effects/EffectCatalog.json").read_text(
                encoding="utf-8"
            )
        )
        source_entries = {
            row["effectAssetId"]: row for row in source_catalog["effects"]
        }
        selected_entries: list[dict[str, object]] = []
        runtime_entries: list[dict[str, object]] = []
        selected_ids: set[str] = set()
        copied_animations: set[str] = set()
        for row in source_policy["approvals"]:
            animation_asset_id = row["animationAssetId"]
            if animation_asset_id not in copied_animations:
                source_animation = (
                    SOURCE_DATA_ROOT / "Animation/Authored" / animation_asset_id
                )
                destination_animation = (
                    self.data_root / "Animation/Authored" / animation_asset_id
                )
                destination_animation.mkdir(parents=True, exist_ok=True)
                for suffix in ("skillbindings.json", "animevents"):
                    shutil.copyfile(
                        source_animation / f"{animation_asset_id}.{suffix}",
                        destination_animation / f"{animation_asset_id}.{suffix}",
                    )
                copied_animations.add(animation_asset_id)

            effect_id = row["effectAssetId"]
            rollback_id = row["rollbackEffectAssetId"]
            for selected_id in (effect_id, rollback_id):
                if selected_id not in selected_ids:
                    selected_entries.append(copy.deepcopy(source_entries[selected_id]))
                    selected_ids.add(selected_id)

            authored_source = SOURCE_DATA_ROOT / source_entries[effect_id]["authoringPath"]
            authored_destination = self.data_root / source_entries[effect_id]["authoringPath"]
            authored_destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(authored_source, authored_destination)
            authored_payload = authored_destination.read_bytes()
            row["effectContentSha256"] = hashlib.sha256(authored_payload).hexdigest()
            runtime_authored_path = (
                f"Authored/{effect_id}.{row['effectContentSha256']}.effect.json"
            )
            sealed_destination = (
                self.runtime_catalog_path.parent / runtime_authored_path
            )
            sealed_destination.parent.mkdir(parents=True, exist_ok=True)
            sealed_destination.write_bytes(authored_payload)
            runtime_entries.append(
                {
                    "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
                    "effectAssetId": effect_id,
                    "authoringFormatVersion": 13,
                    "authoredDocumentPath": runtime_authored_path,
                    "contentSha256": row["effectContentSha256"],
                    "dependencies": [],
                }
            )
            runtime_entries.append(
                {
                    "payloadKind": "ROLLBACK_FIXTURE",
                    "effectAssetId": rollback_id,
                }
            )

        _write_json(
            self.data_root / "Effects/ProductCueApprovals.json", source_policy
        )
        policy_payload = (
            self.data_root / "Effects/ProductCueApprovals.json"
        ).read_bytes()
        _write_json(
            self.data_root / "Effects/EffectCatalog.json",
            {"formatVersion": 1, "effects": selected_entries},
        )
        _write_json(
            self.runtime_catalog_path,
            {
                "schema": "lostark.effect-runtime-catalog",
                "formatVersion": 3,
                "productCueAdmissionsRequired": True,
                "productCuePolicySha256": hashlib.sha256(policy_payload).hexdigest(),
                "components": [],
                "effects": runtime_entries,
            },
        )

    def tearDown(self) -> None:
        self._temporary.cleanup()

    def _policy(self) -> dict[str, object]:
        return json.loads(
            (self.data_root / "Effects/ProductCueApprovals.json").read_text(
                encoding="utf-8"
            )
        )

    def _write_policy(
        self, policy: dict[str, object], *, pin_runtime: bool = True
    ) -> None:
        policy_path = self.data_root / "Effects/ProductCueApprovals.json"
        _write_json(policy_path, policy)
        if pin_runtime and self.runtime_catalog_path.exists():
            runtime_catalog = self._runtime_catalog()
            runtime_catalog["productCuePolicySha256"] = hashlib.sha256(
                policy_path.read_bytes()
            ).hexdigest()
            _write_json(self.runtime_catalog_path, runtime_catalog)

    def _runtime_catalog(self) -> dict[str, object]:
        return json.loads(self.runtime_catalog_path.read_text(encoding="utf-8"))

    def _runtime_candidate(
        self, row: dict[str, object]
    ) -> tuple[dict[str, object], dict[str, object], Path]:
        runtime_catalog = self._runtime_catalog()
        runtime_row = next(
            candidate
            for candidate in runtime_catalog["effects"]
            if candidate["effectAssetId"] == row["effectAssetId"]
        )
        sealed_path = (
            self.runtime_catalog_path.parent
            / runtime_row["authoredDocumentPath"]
        )
        return runtime_catalog, runtime_row, sealed_path

    def _replace_candidate_document(
        self,
        row: dict[str, object],
        document: dict[str, object],
        *,
        pin_policy: bool,
    ) -> None:
        effect_id = row["effectAssetId"]
        path = self.data_root / f"Effects/Authored/{effect_id}.effect.json"
        _write_json(path, document)
        payload = path.read_bytes()
        content_sha = hashlib.sha256(payload).hexdigest()

        runtime_catalog = self._runtime_catalog()
        runtime_row = next(
            candidate
            for candidate in runtime_catalog["effects"]
            if candidate["effectAssetId"] == effect_id
        )
        runtime_authored_path = f"Authored/{effect_id}.{content_sha}.effect.json"
        sealed_destination = self.runtime_catalog_path.parent / runtime_authored_path
        sealed_destination.parent.mkdir(parents=True, exist_ok=True)
        sealed_destination.write_bytes(payload)
        runtime_row["authoredDocumentPath"] = runtime_authored_path
        runtime_row["contentSha256"] = content_sha
        _write_json(self.runtime_catalog_path, runtime_catalog)

        if pin_policy:
            policy = self._policy()
            policy_row = next(
                candidate
                for candidate in policy["approvals"]
                if candidate["cueId"] == row["cueId"]
            )
            policy_row["effectContentSha256"] = content_sha
            self._write_policy(policy)

    def _build_receipt(self) -> dict[str, object]:
        admission.build_runtime_receipt(
            self.data_root, self.runtime_catalog_path, self.receipt_path
        )
        return json.loads(self.receipt_path.read_text(encoding="utf-8"))

    def _force_rollback_crlf(
        self, row: dict[str, object]
    ) -> tuple[Path, bytes]:
        animation_asset_id = row["animationAssetId"]
        path, lines, cues = admission._animevent_rows(
            self.data_root, animation_asset_id
        )
        cue = next(
            candidate
            for candidate in cues
            if candidate["clipName"] == row["clipName"]
            and candidate["startMs"] == row["startMs"]
        )
        old_line = lines[cue["lineIndex"]]
        desired_line = old_line.replace(
            f'payload="{cue["effectAssetId"]}"',
            f'payload="{row["rollbackEffectAssetId"]}"',
        )
        normalized = path.read_bytes().decode("utf-8").replace("\r\n", "\n")
        normalized = normalized.replace(old_line, desired_line, 1)
        path.write_bytes(normalized.replace("\n", "\r\n").encode("utf-8"))
        payload = path.read_bytes()
        self.assertIn(b"\r\n", payload)
        return path, payload

    def test_current_five_explicit_approvals_build_receipt(self) -> None:
        receipt = self._build_receipt()
        policy = self._policy()
        self.assertEqual("EXPLICIT_CUE_OPT_IN_ONLY", receipt["admissionMode"])
        self.assertEqual(
            (self.data_root / "Effects/ProductCueApprovals.json").read_text(
                encoding="utf-8"
            ),
            receipt["sourcePolicyUtf8Json"],
        )
        self.assertEqual(5, len(receipt["approvals"]))
        self.assertEqual(
            sorted(row["cueId"] for row in policy["approvals"]),
            [row["cueId"] for row in receipt["approvals"]],
        )
        for row in receipt["approvals"]:
            self.assertEqual(admission.ADMISSION, row["approvalCeiling"])
            expected = (
                admission.ADMISSION
                if row["approximateElementCount"]
                else admission.FULL_ADMISSION
            )
            self.assertEqual(expected, row["admission"])

    def test_unapproved_approximate_direct_document_is_rejected(self) -> None:
        policy = self._policy()
        removed = policy["approvals"].pop(0)
        self._write_policy(policy)
        path = (
            self.data_root
            / f"Effects/Authored/{removed['effectAssetId']}.effect.json"
        )
        document = json.loads(path.read_text(encoding="utf-8"))
        document["elements"] = [
            {
                "visible": True,
                "material": {
                    "execution": {
                        "enabled": False,
                        "failClosed": True,
                        "authoringApproximate": True,
                    }
                },
            }
        ]
        self._replace_candidate_document(removed, document, pin_policy=False)
        with self.assertRaisesRegex(
            admission.AdmissionError, "no explicit cue approval"
        ):
            self._build_receipt()

    def test_stale_authored_hash_is_rejected(self) -> None:
        row = self._policy()["approvals"][0]
        path = self.data_root / f"Effects/Authored/{row['effectAssetId']}.effect.json"
        path.write_bytes(path.read_bytes() + b" ")
        with self.assertRaisesRegex(admission.AdmissionError, "source hash drifted"):
            self._build_receipt()

    def test_missing_or_wrong_sealed_runtime_path_is_rejected_without_mutation(self) -> None:
        row = self._policy()["approvals"][0]
        runtime_catalog, runtime_row, sealed_path = self._runtime_candidate(row)
        source_path = (
            self.data_root
            / f"Effects/Authored/{row['effectAssetId']}.effect.json"
        )
        animevent_path = (
            self.data_root
            / "Animation/Authored"
            / row["animationAssetId"]
            / f"{row['animationAssetId']}.animevents"
        )
        source_before = source_path.read_bytes()
        animevents_before = animevent_path.read_bytes()
        sealed_before = sealed_path.read_bytes()

        sealed_path.unlink()
        with self.assertRaisesRegex(
            admission.AdmissionError, "could not be resolved"
        ):
            self._build_receipt()
        self.assertFalse(self.receipt_path.exists())
        self.assertEqual(source_before, source_path.read_bytes())
        self.assertEqual(animevents_before, animevent_path.read_bytes())

        sealed_path.write_bytes(sealed_before)
        for invalid_path in (
            "../Authored/escaped.effect.json",
            f"Authored/{row['effectAssetId']}.wrong.effect.json",
        ):
            mutated_catalog = copy.deepcopy(runtime_catalog)
            mutated_row = next(
                candidate
                for candidate in mutated_catalog["effects"]
                if candidate["effectAssetId"] == row["effectAssetId"]
            )
            mutated_row["authoredDocumentPath"] = invalid_path
            _write_json(self.runtime_catalog_path, mutated_catalog)
            with self.subTest(path=invalid_path):
                with self.assertRaisesRegex(
                    admission.AdmissionError, "path is not canonical"
                ):
                    self._build_receipt()
                self.assertEqual(source_before, source_path.read_bytes())
                self.assertEqual(animevents_before, animevent_path.read_bytes())
                self.assertEqual(sealed_before, sealed_path.read_bytes())

    def test_sealed_runtime_hash_drift_and_identity_are_rejected(self) -> None:
        row = self._policy()["approvals"][0]
        runtime_catalog, runtime_row, sealed_path = self._runtime_candidate(row)
        source_path = (
            self.data_root
            / f"Effects/Authored/{row['effectAssetId']}.effect.json"
        )
        source_before = source_path.read_bytes()
        sealed_before = sealed_path.read_bytes()

        sealed_path.write_bytes(sealed_before + b" ")
        with self.assertRaisesRegex(
            admission.AdmissionError, "sealed runtime document hash drifted"
        ):
            self._build_receipt()
        self.assertEqual(source_before, source_path.read_bytes())
        self.assertEqual(sealed_before + b" ", sealed_path.read_bytes())

        invalid_document = json.loads(source_before.decode("utf-8"))
        invalid_document["effectAssetId"] = "effect.fixture.wrong-identity"
        invalid_payload = (
            json.dumps(invalid_document, ensure_ascii=False, indent=2) + "\n"
        ).encode("utf-8")
        invalid_sha = hashlib.sha256(invalid_payload).hexdigest()
        invalid_relative_path = (
            f"Authored/{row['effectAssetId']}.{invalid_sha}.effect.json"
        )
        invalid_path = self.runtime_catalog_path.parent / invalid_relative_path
        invalid_path.write_bytes(invalid_payload)
        runtime_row["authoredDocumentPath"] = invalid_relative_path
        runtime_row["contentSha256"] = invalid_sha
        _write_json(self.runtime_catalog_path, runtime_catalog)
        with self.assertRaisesRegex(
            admission.AdmissionError, "runtime document identity is invalid"
        ):
            self._build_receipt()
        self.assertEqual(source_before, source_path.read_bytes())

    def test_wrong_cue_tuple_is_rejected(self) -> None:
        policy = self._policy()
        policy["approvals"][0]["startMs"] += 1
        self._write_policy(policy)
        with self.assertRaisesRegex(
            admission.AdmissionError, "neither candidate nor rollback target"
        ):
            self._build_receipt()

    def test_runtime_incompatible_animevent_version_and_short_row_are_rejected(self) -> None:
        row = self._policy()["approvals"][0]
        path = self.data_root / "Animation/Authored/Artist/Artist.animevents"
        original = path.read_bytes()
        path.write_bytes(original.replace(b"LOSTARK_ANIM_EVENTS 5 ", b"LOSTARK_ANIM_EVENTS 2 ", 1))
        with self.assertRaisesRegex(admission.AdmissionError, "owner/version/count"):
            self._build_receipt()

        lines = original.decode("utf-8").splitlines()
        header = shlex.split(lines[0], posix=True)
        header[3] = str(int(header[3]) + 1)
        lines[0] = " ".join(header)
        lines.append("short EFFECT")
        path.write_bytes(("\n".join(lines) + "\n").encode("utf-8"))
        with self.assertRaisesRegex(admission.AdmissionError, "row is invalid"):
            self._build_receipt()

    def test_duplicate_cue_and_effect_approvals_are_rejected(self) -> None:
        original = self._policy()

        duplicate_cue = copy.deepcopy(original)
        duplicate_cue["approvals"].append(
            copy.deepcopy(duplicate_cue["approvals"][0])
        )
        self._write_policy(duplicate_cue)
        with self.assertRaisesRegex(admission.AdmissionError, "approval ID"):
            self._build_receipt()

        duplicate_effect = copy.deepcopy(original)
        extra = copy.deepcopy(duplicate_effect["approvals"][0])
        extra["cueId"] += ".duplicate-owner"
        extra["startMs"] += 1
        duplicate_effect["approvals"].append(extra)
        self._write_policy(duplicate_effect)
        with self.assertRaisesRegex(admission.AdmissionError, "multiple cue owners"):
            self._build_receipt()

        duplicate_physical_cue = copy.deepcopy(original)
        shadow = copy.deepcopy(duplicate_physical_cue["approvals"][0])
        shadow["cueId"] += ".shadow-candidate"
        shadow["effectAssetId"] += ".shadow-candidate"
        shadow["rollbackEffectAssetId"] = duplicate_physical_cue["approvals"][0][
            "effectAssetId"
        ]
        duplicate_physical_cue["approvals"].append(shadow)
        self._write_policy(duplicate_physical_cue)
        with self.assertRaisesRegex(admission.AdmissionError, "multiple approvals"):
            self._build_receipt()

        overlapping_rollback = copy.deepcopy(original)
        overlapping_rollback["approvals"][0]["rollbackEffectAssetId"] = (
            overlapping_rollback["approvals"][1]["effectAssetId"]
        )
        self._write_policy(overlapping_rollback)
        with self.assertRaisesRegex(admission.AdmissionError, "sets overlap"):
            self._build_receipt()

    def test_visible_hard_carrier_is_rejected(self) -> None:
        row = self._policy()["approvals"][0]
        path = self.data_root / f"Effects/Authored/{row['effectAssetId']}.effect.json"
        document = json.loads(path.read_text(encoding="utf-8"))
        document["elements"] = [
            {
                "visible": True,
                "material": {
                    "execution": {"enabled": False, "failClosed": True}
                },
            }
        ]
        self._replace_candidate_document(row, document, pin_policy=True)
        with self.assertRaisesRegex(
            admission.AdmissionError, "Hard carrier is not suppressed"
        ):
            self._build_receipt()

    def test_duplicate_policy_property_and_stale_receipt_block_cue_switch(self) -> None:
        policy_path = self.data_root / "Effects/ProductCueApprovals.json"
        original_policy = policy_path.read_bytes()
        duplicated = original_policy.replace(
            b'      "cueId":',
            b'      "cueId": "duplicate.fixture",\n      "cueId":',
            1,
        )
        policy_path.write_bytes(duplicated)
        with self.assertRaisesRegex(admission.AdmissionError, "duplicate JSON property"):
            self._build_receipt()

        policy_path.write_bytes(original_policy)
        row = self._policy()["approvals"][0]
        path, rollback_bytes = self._force_rollback_crlf(row)
        self._build_receipt()
        policy = self._policy()
        policy["approvals"][0]["provenance"]["scope"] += " changed"
        self._write_policy(policy, pin_runtime=False)
        with self.assertRaisesRegex(
            admission.AdmissionError,
            "Runtime catalog does not pin the exact Product cue source policy|receipt is missing or stale",
        ):
            admission.set_cue(
                self.data_root,
                row["cueId"],
                "candidate",
                False,
                self.runtime_catalog_path,
                self.receipt_path,
            )
        self.assertEqual(rollback_bytes, path.read_bytes())

    def test_candidate_then_rollback_restores_exact_animevent_bytes(self) -> None:
        row = self._policy()["approvals"][0]
        path, rollback_bytes = self._force_rollback_crlf(row)
        self._build_receipt()

        admission.set_cue(
            self.data_root,
            row["cueId"],
            "candidate",
            False,
            self.runtime_catalog_path,
            self.receipt_path,
        )
        candidate_bytes = path.read_bytes()
        self.assertNotEqual(rollback_bytes, candidate_bytes)
        self.assertIn(b"\r\n", candidate_bytes)

        admission.set_cue(
            self.data_root,
            row["cueId"],
            "rollback",
            False,
            self.runtime_catalog_path,
            self.receipt_path,
        )
        self.assertEqual(rollback_bytes, path.read_bytes())

    def test_failed_commit_and_restore_preserve_recovery_backup(self) -> None:
        row = self._policy()["approvals"][0]
        path, rollback_bytes = self._force_rollback_crlf(row)
        self._build_receipt()
        real_replace = admission.os.replace
        replacement_count = 0

        def failing_replace(source: object, destination: object) -> None:
            nonlocal replacement_count
            replacement_count += 1
            if replacement_count in (3, 4):
                raise OSError(f"injected replace failure {replacement_count}")
            real_replace(source, destination)

        with mock.patch.object(admission.os, "replace", side_effect=failing_replace):
            with self.assertRaisesRegex(OSError, "injected replace failure 4"):
                admission.set_cue(
                    self.data_root,
                    row["cueId"],
                    "candidate",
                    False,
                    self.runtime_catalog_path,
                    self.receipt_path,
                )

        self.assertFalse(path.exists())
        backups = list(path.parent.glob(f".{path.name}.*.bak"))
        self.assertEqual(1, len(backups))
        self.assertEqual(rollback_bytes, backups[0].read_bytes())


if __name__ == "__main__":
    unittest.main()
