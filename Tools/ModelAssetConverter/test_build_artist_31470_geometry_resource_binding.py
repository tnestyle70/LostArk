#!/usr/bin/env python3
"""Tests for the Artist 31470 typed GeometryBinding and resource transaction."""

from __future__ import annotations

import copy
import json
import os
import shutil
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from build_artist_31470_geometry_resource_binding import (
    ASSETS,
    BINDING_FORMAT_VERSION,
    G02_APPROVED_COMMIT,
    G02_APPROVED_TREE_SHA,
    BindingError,
    build_binding_artifacts,
    build_typed_binding,
    canonical_json_bytes,
    deploy_binding,
    expected_asset_rows,
    load_and_validate_artifacts,
    load_strict_json_object,
    scan_exact_target_basenames,
    sha256_bytes,
    transactionally_replace_targets,
    validate_binding_receipt,
    validate_g02_approved_tree_equivalence,
    validate_typed_binding,
    write_json,
)


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
EXPECTED_SEMANTICS = (
    REPOSITORY_ROOT
    / "Tools"
    / "WModelGeometryContractHarness"
    / "Fixtures"
    / "artist_31470_v11_expected.json"
)
BINDING_PATH = (
    REPOSITORY_ROOT
    / "Data"
    / "Effects"
    / "Imported"
    / "Artist"
    / "Geometry"
    / "skill.31470.geometry-binding.json"
)
RECEIPT_PATH = (
    REPOSITORY_ROOT
    / "Data"
    / "Effects"
    / "Imported"
    / "Artist"
    / "Geometry"
    / "skill.31470.geometry-resource-binding.receipt.json"
)
DEEP_ENVIRONMENT = {
    "source_root": "ARTIST_GEOMETRY_SOURCE_ROOT",
    "legacy_root": "ARTIST_GEOMETRY_LEGACY_ROOT",
    "source_export_receipt": "ARTIST_GEOMETRY_SOURCE_EXPORT_RECEIPT",
    "legacy_cook_receipt": "ARTIST_GEOMETRY_LEGACY_COOK_RECEIPT",
    "source_package_root": "ARTIST_GEOMETRY_SOURCE_PACKAGE_ROOT",
    "legacy_converter": "ARTIST_GEOMETRY_LEGACY_CONVERTER",
}
DEEP_EVIDENCE_AVAILABLE = all(os.environ.get(name) for name in DEEP_ENVIRONMENT.values())


def typed_rows_from_golden() -> list[dict[str, object]]:
    return [
        {
            "sourceObject": row["sourceObject"],
            "assetId": row["assetId"],
            "expectedTuple": row["expectedTuple"],
        }
        for row in expected_asset_rows(EXPECTED_SEMANTICS)
    ]


def reseal_binding(binding: dict[str, object]) -> None:
    binding.pop("bindingSha256", None)
    binding["bindingSha256"] = sha256_bytes(canonical_json_bytes(binding))


def reseal_receipt(receipt: dict[str, object]) -> None:
    receipt.pop("receiptSha256", None)
    receipt["receiptSha256"] = sha256_bytes(canonical_json_bytes(receipt))


def cache_identity_from_untrusted_row(row: dict[str, object]) -> str:
    return sha256_bytes(
        canonical_json_bytes(
            {
                "assetId": row["carrierAssetId"],
                "payloadSha256": row["payloadSha256"],
                "provenanceSha256": row["provenanceSha256"],
                "geometryPreScaleF32Hex": row["geometryPreScaleF32Hex"],
                "channelMask": row["channelMask"],
                "evidenceFlags": row["evidenceFlags"],
                "submeshes": row["submeshes"],
            }
        )
    )


class TypedGeometryBindingTests(unittest.TestCase):
    def test_golden_rows_build_a_valid_generic_binding(self) -> None:
        binding = build_typed_binding(typed_rows_from_golden())
        validate_typed_binding(binding, EXPECTED_SEMANTICS)
        self.assertEqual(BINDING_FORMAT_VERSION, binding["formatVersion"])
        self.assertEqual(7, binding["summary"]["carrierCount"])
        self.assertFalse(binding["productAdmission"])

    def test_format_version_rejects_bool_and_float(self) -> None:
        for invalid in (True, 1.0, "1"):
            with self.subTest(invalid=invalid):
                binding = build_typed_binding(typed_rows_from_golden())
                binding["formatVersion"] = invalid
                reseal_binding(binding)
                with self.assertRaises(BindingError):
                    validate_typed_binding(binding, EXPECTED_SEMANTICS)

    def test_resealed_payload_or_prescale_mutation_is_rejected(self) -> None:
        for field, invalid in (
            ("payloadSha256", "1" * 64),
            ("provenanceSha256", "2" * 64),
            ("geometryPreScale", 1.0),
            ("geometryPreScaleF32Hex", "3f800000"),
        ):
            with self.subTest(field=field):
                binding = build_typed_binding(typed_rows_from_golden())
                binding["bindings"][0][field] = invalid
                reseal_binding(binding)
                with self.assertRaises(BindingError):
                    validate_typed_binding(binding, EXPECTED_SEMANTICS)

    def test_numeric_type_policy_rejects_bool_float_integer_and_string_laundering(self) -> None:
        for invalid in (True, 0, 1, "0.01"):
            with self.subTest(field="geometryPreScale", invalid=invalid):
                binding = build_typed_binding(typed_rows_from_golden())
                binding["bindings"][0]["geometryPreScale"] = invalid
                reseal_binding(binding)
                with self.assertRaises(BindingError):
                    validate_typed_binding(binding, EXPECTED_SEMANTICS)
        for field in ("channelMask", "evidenceFlags"):
            for invalid_kind in ("float", "bool", "string"):
                with self.subTest(field=field, invalid_kind=invalid_kind):
                    binding = build_typed_binding(typed_rows_from_golden())
                    row = binding["bindings"][0]
                    original = row[field]
                    row[field] = {
                        "float": float(original),
                        "bool": True,
                        "string": str(original),
                    }[invalid_kind]
                    row["cacheIdentitySha256"] = cache_identity_from_untrusted_row(row)
                    reseal_binding(binding)
                    with self.assertRaises(BindingError):
                        validate_typed_binding(binding, EXPECTED_SEMANTICS)

    def test_g02_approval_is_tree_equivalence_not_ancestry(self) -> None:
        approval = validate_g02_approved_tree_equivalence(REPOSITORY_ROOT)
        self.assertEqual(
            "APPROVED_COMMIT_TREE_PIN_AND_REQUIRED_BLOB_EQUIVALENCE_NOT_GRAPH_ANCESTRY",
            approval["relationship"],
        )
        self.assertFalse(approval["graphAncestryClaimed"])
        self.assertEqual(4, approval["requiredBlobCount"])

    def test_resealed_legacy_revision_cannot_replace_g02_golden_identity(self) -> None:
        binding = load_strict_json_object(BINDING_PATH, "binding")
        receipt = load_strict_json_object(RECEIPT_PATH, "receipt")
        receipt["assets"][0]["legacyResource"]["byteSize"] += 1
        receipt["assets"][0]["legacyResource"]["sha256"] = "1" * 64
        receipt["assets"][0]["sourceEvidenceJoin"]["legacyResourceRawBytes"] = {
            "byteSize": receipt["assets"][0]["legacyResource"]["byteSize"],
            "sha256": receipt["assets"][0]["legacyResource"]["sha256"],
        }
        reseal_receipt(receipt)
        with self.assertRaises(BindingError):
            validate_binding_receipt(receipt, binding, EXPECTED_SEMANTICS)

    def test_resealed_source_evidence_row_reassignment_is_rejected(self) -> None:
        binding = load_strict_json_object(BINDING_PATH, "binding")
        receipt = load_strict_json_object(RECEIPT_PATH, "receipt")
        first = receipt["assets"][0]["sourceEvidenceJoin"]
        second = receipt["assets"][1]["sourceEvidenceJoin"]
        receipt["assets"][0]["sourceEvidenceJoin"] = second
        receipt["assets"][1]["sourceEvidenceJoin"] = first
        reseal_receipt(receipt)
        with self.assertRaises(BindingError):
            validate_binding_receipt(receipt, binding, EXPECTED_SEMANTICS)

    def test_observed_external_identity_cannot_be_laundered_to_exact(self) -> None:
        binding = load_strict_json_object(BINDING_PATH, "binding")
        for scope in ("top-level", "source-package"):
            with self.subTest(scope=scope):
                receipt = load_strict_json_object(RECEIPT_PATH, "receipt")
                if scope == "top-level":
                    receipt["inputs"]["sourceExportReceipt"]["fidelity"] = "SOURCE_EXACT"
                else:
                    receipt["assets"][0]["sourceEvidenceJoin"][
                        "sourcePackageObserved"
                    ]["fidelity"] = "SOURCE_EXACT"
                reseal_receipt(receipt)
                with self.assertRaises(BindingError):
                    validate_binding_receipt(receipt, binding, EXPECTED_SEMANTICS)

    def test_resealed_cache_collision_is_rejected(self) -> None:
        binding = build_typed_binding(typed_rows_from_golden())
        binding["bindings"][1]["cacheIdentitySha256"] = binding["bindings"][0][
            "cacheIdentitySha256"
        ]
        reseal_binding(binding)
        with self.assertRaises(BindingError):
            validate_typed_binding(binding, EXPECTED_SEMANTICS)

    def test_checked_artifacts_are_strict_and_cross_bound(self) -> None:
        binding, receipt = load_and_validate_artifacts(
            BINDING_PATH, RECEIPT_PATH, EXPECTED_SEMANTICS
        )
        self.assertEqual(7, len(binding["bindings"]))
        self.assertEqual(binding["bindingSha256"], receipt["typedBinding"]["bindingSha256"])

    def test_duplicate_json_keys_and_bom_are_rejected(self) -> None:
        with tempfile.TemporaryDirectory(prefix="artist-31470-geometry-json-") as raw:
            root = Path(raw)
            cases = {
                "top.json": b'{"schema":"one","schema":"two"}',
                "nested.json": b'{"outer":{"key":1,"key":2}}',
                "bom.json": b"\xef\xbb\xbf{}",
            }
            for name, payload in cases.items():
                with self.subTest(name=name):
                    path = root / name
                    path.write_bytes(payload)
                    with self.assertRaises(BindingError):
                        load_strict_json_object(path, name)


@unittest.skipUnless(
    DEEP_EVIDENCE_AVAILABLE,
    "external Artist geometry evidence roots were not supplied",
)
class DeepGeometryEvidenceMutationTests(unittest.TestCase):
    def evidence_paths(self) -> dict[str, Path]:
        return {
            key: Path(os.environ[environment_name])
            for key, environment_name in DEEP_ENVIRONMENT.items()
        }

    def copy_source_carriers(self, destination: Path, source_root: Path) -> None:
        for package, name in ASSETS:
            source_gltf = source_root / package / "StaticMesh3" / f"{name}.gltf"
            target_gltf = destination / package / "StaticMesh3" / f"{name}.gltf"
            target_gltf.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source_gltf, target_gltf)
            document = json.loads(source_gltf.read_text(encoding="utf-8"))
            for buffer in document.get("buffers") or []:
                uri = Path(buffer["uri"])
                target = target_gltf.parent / uri
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source_gltf.parent / uri, target)

    def copy_legacy_carriers(self, destination: Path, legacy_root: Path) -> None:
        destination.mkdir(parents=True, exist_ok=True)
        for _, name in ASSETS:
            shutil.copy2(legacy_root / f"{name}.wmodel", destination / f"{name}.wmodel")

    def test_actual_builder_rejects_coordinated_source_gltf_and_receipt_reseal(self) -> None:
        paths = self.evidence_paths()
        with tempfile.TemporaryDirectory(prefix="artist-31470-source-reseal-") as raw:
            root = Path(raw)
            source_root = root / "source"
            self.copy_source_carriers(source_root, paths["source_root"])
            export_receipt = load_strict_json_object(
                paths["source_export_receipt"], "source export receipt"
            )
            package, name = ASSETS[0]
            source_key = f"{package.casefold()}.{name}"
            gltf = source_root / package / "StaticMesh3" / f"{name}.gltf"
            gltf.write_bytes(gltf.read_bytes() + b"\n ")
            for row in export_receipt["assets"]:
                if row.get("sourceAssetPath") != source_key:
                    continue
                for output in row["outputs"]:
                    if output.get("relativePath") == f"{package}/StaticMesh3/{name}.gltf":
                        output["byteSize"] = gltf.stat().st_size
                        output["sha256"] = sha256_bytes(gltf.read_bytes())
            mutated_export = root / "resource-export-receipt.json"
            write_json(mutated_export, export_receipt)
            cook_receipt = load_strict_json_object(
                paths["legacy_cook_receipt"], "legacy cook receipt"
            )
            cook_receipt["sourceExportReceiptSha256"] = sha256_bytes(
                mutated_export.read_bytes()
            )
            mutated_cook = root / "runtime-cook-receipt.json"
            write_json(mutated_cook, cook_receipt)
            with self.assertRaises(BindingError):
                build_binding_artifacts(
                    source_root,
                    paths["legacy_root"],
                    REPOSITORY_ROOT
                    / "Data/Effects/Imported/Artist/Artist.resource-source-manifest.json",
                    mutated_export,
                    mutated_cook,
                    paths["source_package_root"],
                    paths["legacy_converter"],
                    EXPECTED_SEMANTICS,
                    root / "stage",
                )

    def test_actual_builder_rejects_coordinated_legacy_bytes_and_cook_row_reseal(self) -> None:
        paths = self.evidence_paths()
        with tempfile.TemporaryDirectory(prefix="artist-31470-legacy-reseal-") as raw:
            root = Path(raw)
            legacy_root = root / "legacy"
            self.copy_legacy_carriers(legacy_root, paths["legacy_root"])
            package, name = ASSETS[0]
            source_key = f"{package.casefold()}.{name}"
            legacy = legacy_root / f"{name}.wmodel"
            legacy.write_bytes(legacy.read_bytes() + b"changed-revision")
            cook_receipt = load_strict_json_object(
                paths["legacy_cook_receipt"], "legacy cook receipt"
            )
            for row in cook_receipt["assets"]:
                if row.get("sourceAssetPath") == source_key and row.get("role") == "mesh":
                    row["byteSize"] = legacy.stat().st_size
                    row["sha256"] = sha256_bytes(legacy.read_bytes())
            mutated_cook = root / "runtime-cook-receipt.json"
            write_json(mutated_cook, cook_receipt)
            with self.assertRaises(BindingError):
                build_binding_artifacts(
                    paths["source_root"],
                    legacy_root,
                    REPOSITORY_ROOT
                    / "Data/Effects/Imported/Artist/Artist.resource-source-manifest.json",
                    paths["source_export_receipt"],
                    mutated_cook,
                    paths["source_package_root"],
                    paths["legacy_converter"],
                    EXPECTED_SEMANTICS,
                    root / "stage",
                )

    def test_actual_deploy_dry_run_rejects_resealed_changed_legacy_revision(self) -> None:
        paths = self.evidence_paths()
        binding = load_strict_json_object(BINDING_PATH, "binding")
        receipt = load_strict_json_object(RECEIPT_PATH, "receipt")
        with tempfile.TemporaryDirectory(prefix="artist-31470-deploy-reseal-") as raw:
            root = Path(raw)
            physical = root / "physical"
            self.copy_legacy_carriers(physical, paths["legacy_root"])
            first = physical / f"{ASSETS[0][1]}.wmodel"
            first.write_bytes(first.read_bytes() + b"changed-revision")
            changed = {
                "byteSize": first.stat().st_size,
                "sha256": sha256_bytes(first.read_bytes()),
                "fidelity": "APPROVED_G02_SEMANTIC_GOLDEN_PINNED_BYTES",
            }
            receipt["assets"][0]["legacyResource"] = changed
            receipt["assets"][0]["sourceEvidenceJoin"][
                "legacyResourceRawBytes"
            ] = {key: changed[key] for key in ("byteSize", "sha256")}
            reseal_receipt(receipt)
            with self.assertRaises(BindingError):
                deploy_binding(
                    binding,
                    receipt,
                    EXPECTED_SEMANTICS,
                    root / "unused-stage",
                    physical,
                    None,
                    True,
                    G02_APPROVED_COMMIT,
                    G02_APPROVED_TREE_SHA,
                )


class GeometryResourceTransactionTests(unittest.TestCase):
    def make_targets(
        self, root: Path
    ) -> tuple[list[tuple[dict[str, object], Path, bytes, bytes]], list[bytes], list[bytes]]:
        targets: list[tuple[dict[str, object], Path, bytes, bytes]] = []
        originals: list[bytes] = []
        candidates: list[bytes] = []
        physical = root / "physical"
        physical.mkdir(parents=True)
        for index, (_, name) in enumerate(ASSETS):
            original = f"legacy-{index}".encode("ascii")
            candidate = f"candidate-v11-{index}".encode("ascii")
            target = physical / f"{name}.wmodel"
            target.write_bytes(original)
            asset = {"assetId": f"Effect/Artist/Meshes/{name}.wmodel"}
            targets.append((asset, target, original, candidate))
            originals.append(original)
            candidates.append(candidate)
        return targets, originals, candidates

    def assert_target_bytes(
        self,
        targets: list[tuple[dict[str, object], Path, bytes, bytes]],
        expected: list[bytes],
    ) -> None:
        self.assertEqual(expected, [target.read_bytes() for _, target, _, _ in targets])

    def test_success_replaces_exactly_seven_and_preserves_verified_backup(self) -> None:
        with tempfile.TemporaryDirectory(prefix="artist-31470-geometry-deploy-") as raw:
            root = Path(raw)
            targets, originals, candidates = self.make_targets(root)
            backup = root / "backup"
            identity = transactionally_replace_targets(targets, backup)
            self.assert_target_bytes(targets, candidates)
            self.assertEqual(7, identity["carrierCount"])
            manifest = load_strict_json_object(
                backup / "artist-31470.geometry-resource-backup.receipt.json",
                "backup manifest",
            )
            self.assertEqual(7, manifest["carrierCount"])
            for (_, name), original in zip(ASSETS, originals):
                self.assertEqual(
                    original,
                    (backup / "Effect" / "Artist" / "Meshes" / f"{name}.wmodel").read_bytes(),
                )

    def test_missing_or_changed_target_fails_before_any_write(self) -> None:
        with tempfile.TemporaryDirectory(prefix="artist-31470-geometry-preflight-") as raw:
            root = Path(raw)
            targets, originals, _ = self.make_targets(root)
            targets[3][1].unlink()
            with self.assertRaises(BindingError):
                transactionally_replace_targets(targets, root / "missing-backup")
            for index, (_, target, _, _) in enumerate(targets):
                if index != 3:
                    self.assertEqual(originals[index], target.read_bytes())
            self.assertFalse((root / "missing-backup").exists())

            targets, originals, _ = self.make_targets(root / "second")
            targets[2][1].write_bytes(b"unexpected-revision")
            with self.assertRaises(BindingError):
                transactionally_replace_targets(targets, root / "changed-backup")
            self.assertEqual(b"unexpected-revision", targets[2][1].read_bytes())
            self.assertFalse((root / "changed-backup").exists())

    def test_injected_replace_or_post_validation_failure_rolls_back_all_seven(self) -> None:
        for mode in ("replace", "post-validate"):
            with self.subTest(mode=mode), tempfile.TemporaryDirectory(
                prefix="artist-31470-geometry-rollback-"
            ) as raw:
                root = Path(raw)
                targets, originals, _ = self.make_targets(root)

                def fail_post_validate() -> None:
                    raise BindingError("injected post validation failure")

                with self.assertRaises(BindingError):
                    transactionally_replace_targets(
                        targets,
                        root / "backup",
                        fail_after_replace=3 if mode == "replace" else None,
                        post_validate=fail_post_validate if mode == "post-validate" else None,
                    )
                self.assert_target_bytes(targets, originals)
                residues = list((root / "physical").glob(".*.artist31470.*.tmp"))
                self.assertEqual([], residues)

    def test_duplicate_target_path_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory(prefix="artist-31470-geometry-collision-") as raw:
            root = Path(raw)
            targets, originals, _ = self.make_targets(root)
            collided = copy.copy(targets)
            collided[1] = (collided[1][0], collided[0][1], originals[0], collided[1][3])
            with self.assertRaises(BindingError):
                transactionally_replace_targets(collided, root / "backup")
            self.assert_target_bytes(targets, originals)

    def test_case_only_rename_is_rejected_before_backup_or_write(self) -> None:
        with tempfile.TemporaryDirectory(prefix="artist-31470-geometry-case-") as raw:
            root = Path(raw)
            targets, originals, _ = self.make_targets(root)
            exact = targets[0][1]
            intermediate = exact.with_name("case-rename-intermediate.wmodel")
            alias = exact.with_name(exact.name.upper())
            exact.rename(intermediate)
            intermediate.rename(alias)
            with self.assertRaises(BindingError):
                transactionally_replace_targets(targets, root / "backup")
            self.assertFalse((root / "backup").exists())
            self.assertEqual(originals[0], alias.read_bytes())

    def test_casefold_alias_collision_is_rejected_by_scandir_set(self) -> None:
        class FakeEntry:
            def __init__(self, path: Path, name: str) -> None:
                self.path = str(path / name)
                self.name = name

            def is_file(self, follow_symlinks: bool = True) -> bool:
                del follow_symlinks
                return True

            def is_symlink(self) -> bool:
                return False

        class FakeScan:
            def __init__(self, entries: list[FakeEntry]) -> None:
                self.entries = entries

            def __enter__(self) -> list[FakeEntry]:
                return self.entries

            def __exit__(self, *_: object) -> None:
                return None

        with tempfile.TemporaryDirectory(prefix="artist-31470-geometry-alias-") as raw:
            root = Path(raw)
            entries = [FakeEntry(root, f"{name}.wmodel") for _, name in ASSETS]
            entries.append(FakeEntry(root, entries[0].name.upper()))
            with patch(
                "build_artist_31470_geometry_resource_binding.os.scandir",
                return_value=FakeScan(entries),
            ), self.assertRaises(BindingError):
                scan_exact_target_basenames(root, "alias fixture")


if __name__ == "__main__":
    unittest.main()
    deploy_binding,
