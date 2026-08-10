#!/usr/bin/env python3
"""Tests for the Artist 31470 typed GeometryBinding and resource transaction."""

from __future__ import annotations

import copy
import tempfile
import unittest
from pathlib import Path

from build_artist_31470_geometry_resource_binding import (
    ASSETS,
    BINDING_FORMAT_VERSION,
    BindingError,
    build_typed_binding,
    canonical_json_bytes,
    expected_asset_rows,
    load_and_validate_artifacts,
    load_strict_json_object,
    sha256_bytes,
    transactionally_replace_targets,
    validate_typed_binding,
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


if __name__ == "__main__":
    unittest.main()
