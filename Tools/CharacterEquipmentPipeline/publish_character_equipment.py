#!/usr/bin/env python3
"""Validate and transactionally publish admitted character equipment resources."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
import shutil
import sys
from typing import Callable, Iterable
from uuid import uuid4


ADMISSION_FILE_NAME = "character-equipment-runtime-admission.json"
ADMISSION_SCHEMA = "lostark.character-equipment-runtime-admission"
EXPECTED_RESOURCE_FOLDERS = frozenset(
    {"Fonts", "Character", "Deploy", "Effect", "Map", "Sound", "UI"}
)
EXPECTED_CLASS_COUNT = 6
ALLOWED_KINDS = frozenset({"WMODEL", "TEXTURE"})


class PublishError(RuntimeError):
    """Raised when admission, collision, staging, or promotion validation fails."""


@dataclass(frozen=True)
class ClosureEntry:
    kind: str
    asset_id: str
    byte_size: int
    sha256: str
    source_path: Path
    target_path: Path
    equipment_root_asset_id: str


@dataclass(frozen=True)
class EquipmentGroup:
    asset_id: str
    target_path: Path
    entries: tuple[ClosureEntry, ...]
    existing_entries: tuple[ClosureEntry, ...]
    new_entries: tuple[ClosureEntry, ...]

    @property
    def already_published(self) -> bool:
        return not self.new_entries


@dataclass(frozen=True)
class PublishPlan:
    admission_root: Path
    resource_root: Path
    entries: tuple[ClosureEntry, ...]
    groups: tuple[EquipmentGroup, ...]

    @property
    def new_groups(self) -> tuple[EquipmentGroup, ...]:
        return tuple(group for group in self.groups if not group.already_published)

    @property
    def existing_groups(self) -> tuple[EquipmentGroup, ...]:
        return tuple(group for group in self.groups if group.already_published)

    @property
    def new_entries(self) -> tuple[ClosureEntry, ...]:
        return tuple(entry for group in self.groups for entry in group.new_entries)

    @property
    def existing_entries(self) -> tuple[ClosureEntry, ...]:
        return tuple(entry for group in self.groups for entry in group.existing_entries)


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()


def _load_json(path: Path) -> dict:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as error:
        raise PublishError(f"Required JSON does not exist: {path}") from error
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise PublishError(f"Failed to parse JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise PublishError(f"JSON root must be an object: {path}")
    return value


def _require_reported_count(counts: dict, key: str, actual: int) -> None:
    reported = counts.get(key)
    if (
        not isinstance(reported, int)
        or isinstance(reported, bool)
        or reported < 0
        or reported != actual
    ):
        raise PublishError(
            f"Admission counts.{key} must match the receipt contents: "
            f"reported={reported!r}, actual={actual}."
        )


def _safe_asset_parts(asset_id: object) -> tuple[str, ...]:
    if not isinstance(asset_id, str) or not asset_id:
        raise PublishError("runtimeClosure assetId must be a non-empty string.")
    if "\\" in asset_id or ":" in asset_id:
        raise PublishError(f"Unsafe Resources-relative assetId: {asset_id!r}")
    path = PurePosixPath(asset_id)
    parts = path.parts
    if (
        path.is_absolute()
        or path.as_posix() != asset_id
        or not parts
        or any(part in {"", ".", ".."} for part in parts)
    ):
        raise PublishError(f"Unsafe Resources-relative assetId: {asset_id!r}")
    if len(parts) < 4 or parts[0] != "Character" or parts[2] != "Equipment":
        raise PublishError(
            "Character equipment assetId must be under "
            f"Character/<Class>/Equipment/: {asset_id!r}"
        )
    return parts


def _path_beneath(root: Path, parts: Iterable[str], label: str) -> Path:
    candidate = root.joinpath(*parts).resolve(strict=False)
    try:
        candidate.relative_to(root)
    except ValueError as error:
        raise PublishError(f"{label} escapes its root: {candidate}") from error
    return candidate


def _validate_resource_root(resource_root: Path) -> None:
    if not resource_root.is_dir():
        raise PublishError(f"ResourceRoot is not a directory: {resource_root}")
    actual = {entry.name for entry in resource_root.iterdir()}
    if actual != EXPECTED_RESOURCE_FOLDERS:
        missing = sorted(EXPECTED_RESOURCE_FOLDERS - actual)
        unexpected = sorted(actual - EXPECTED_RESOURCE_FOLDERS)
        raise PublishError(
            "ResourceRoot must contain exactly the seven top-level folders; "
            f"missing={missing}, unexpected={unexpected}."
        )


def _parse_entries(
    receipt: dict,
    admission_root: Path,
    resource_root: Path,
) -> tuple[ClosureEntry, ...]:
    closure = receipt.get("runtimeClosure")
    if not isinstance(closure, list) or not closure:
        raise PublishError("Admission runtimeClosure must be a non-empty array.")

    runtime_root = (admission_root / "Runtime").resolve(strict=False)
    if not runtime_root.is_dir():
        raise PublishError(f"Admission Runtime directory does not exist: {runtime_root}")

    entries: list[ClosureEntry] = []
    seen_asset_ids: set[str] = set()
    seen_target_paths: set[str] = set()
    for index, raw in enumerate(closure):
        if not isinstance(raw, dict):
            raise PublishError(f"runtimeClosure[{index}] must be an object.")
        kind = raw.get("kind")
        if kind not in ALLOWED_KINDS:
            raise PublishError(f"runtimeClosure[{index}].kind is invalid: {kind!r}")
        asset_id = raw.get("assetId")
        parts = _safe_asset_parts(asset_id)
        folded_asset_id = asset_id.casefold()
        if folded_asset_id in seen_asset_ids:
            raise PublishError(f"Duplicate runtimeClosure assetId: {asset_id}")
        seen_asset_ids.add(folded_asset_id)

        byte_size = raw.get("byteSize")
        if not isinstance(byte_size, int) or isinstance(byte_size, bool) or byte_size < 0:
            raise PublishError(
                f"runtimeClosure[{index}].byteSize must be a non-negative integer."
            )
        sha256 = raw.get("sha256")
        if (
            not isinstance(sha256, str)
            or len(sha256) != 64
            or any(character not in "0123456789abcdefABCDEF" for character in sha256)
        ):
            raise PublishError(f"runtimeClosure[{index}].sha256 is invalid.")
        sha256 = sha256.upper()

        source_path = _path_beneath(runtime_root, parts, "Admission runtime asset")
        if not source_path.is_file():
            raise PublishError(f"Admitted runtime asset does not exist: {source_path}")
        actual_size = source_path.stat().st_size
        if actual_size != byte_size:
            raise PublishError(
                f"Admitted runtime asset size mismatch for {asset_id}: "
                f"expected={byte_size}, actual={actual_size}."
            )
        actual_hash = sha256_file(source_path)
        if actual_hash != sha256:
            raise PublishError(
                f"Admitted runtime asset hash mismatch for {asset_id}: "
                f"expected={sha256}, actual={actual_hash}."
            )

        target_path = _path_beneath(resource_root, parts, "Resource target")
        folded_target_path = str(target_path).casefold()
        if folded_target_path in seen_target_paths:
            raise PublishError(f"Case-insensitive target collision: {asset_id}")
        seen_target_paths.add(folded_target_path)
        equipment_root_asset_id = "/".join(parts[:3])
        entries.append(
            ClosureEntry(
                kind=kind,
                asset_id=asset_id,
                byte_size=byte_size,
                sha256=sha256,
                source_path=source_path,
                target_path=target_path,
                equipment_root_asset_id=equipment_root_asset_id,
            )
        )
    return tuple(entries)


def _group_entries(
    entries: tuple[ClosureEntry, ...], resource_root: Path
) -> tuple[EquipmentGroup, ...]:
    grouped: dict[str, list[ClosureEntry]] = {}
    for entry in entries:
        grouped.setdefault(entry.equipment_root_asset_id, []).append(entry)

    groups: list[EquipmentGroup] = []
    seen_group_targets: set[str] = set()
    for asset_id in sorted(grouped, key=str.casefold):
        parts = PurePosixPath(asset_id).parts
        target_path = _path_beneath(resource_root, parts, "Equipment target directory")
        folded_group_target = str(target_path).casefold()
        if folded_group_target in seen_group_targets:
            raise PublishError(f"Case-insensitive Equipment directory collision: {asset_id}")
        seen_group_targets.add(folded_group_target)
        parent_path = target_path.parent
        if not parent_path.is_dir():
            raise PublishError(
                "Character class resource directory must already exist before equipment "
                f"promotion: {parent_path}"
            )
        group_entries = tuple(sorted(grouped[asset_id], key=lambda item: item.asset_id.casefold()))
        existing_entries: list[ClosureEntry] = []
        new_entries: list[ClosureEntry] = []
        if target_path.exists():
            if not target_path.is_dir():
                raise PublishError(f"Equipment target is not a directory: {target_path}")
            mismatched: list[str] = []
            for entry in group_entries:
                if not entry.target_path.exists() and not entry.target_path.is_symlink():
                    new_entries.append(entry)
                    continue
                if not entry.target_path.is_file() or (
                    entry.target_path.stat().st_size != entry.byte_size
                    or sha256_file(entry.target_path) != entry.sha256
                ):
                    mismatched.append(entry.asset_id)
                    continue
                existing_entries.append(entry)
            if mismatched:
                raise PublishError(
                    f"Equipment target collision under {asset_id}; "
                    f"mismatched={mismatched}."
                )
        else:
            new_entries.extend(group_entries)
        groups.append(
            EquipmentGroup(
                asset_id=asset_id,
                target_path=target_path,
                entries=group_entries,
                existing_entries=tuple(existing_entries),
                new_entries=tuple(new_entries),
            )
        )
    return tuple(groups)


def build_publish_plan(admission_root: Path, resource_root: Path) -> PublishPlan:
    admission_root = admission_root.resolve(strict=False)
    resource_root = resource_root.resolve(strict=False)
    if not admission_root.is_dir():
        raise PublishError(f"AdmissionRoot is not a directory: {admission_root}")
    _validate_resource_root(resource_root)

    receipt_path = admission_root / ADMISSION_FILE_NAME
    receipt = _load_json(receipt_path)
    if receipt.get("schema") != ADMISSION_SCHEMA or receipt.get("formatVersion") != 1:
        raise PublishError(
            f"Unsupported admission schema/version in {receipt_path}: "
            f"{receipt.get('schema')!r}/{receipt.get('formatVersion')!r}."
        )
    counts = receipt.get("counts")
    if not isinstance(counts, dict):
        raise PublishError("Admission counts must be an object.")

    visual_sets = receipt.get("sets")
    if not isinstance(visual_sets, list) or not visual_sets:
        raise PublishError("Admission sets must be a non-empty array.")
    class_ids: set[str] = set()
    model_asset_ids: set[str] = set()
    actual_part_count = 0
    for set_index, visual_set in enumerate(visual_sets):
        if not isinstance(visual_set, dict):
            raise PublishError(f"Admission sets[{set_index}] must be an object.")
        class_id = visual_set.get("classId")
        if not isinstance(class_id, str) or not class_id:
            raise PublishError(f"Admission sets[{set_index}].classId is invalid.")
        class_ids.add(class_id)
        parts = visual_set.get("parts")
        if not isinstance(parts, list) or not parts:
            raise PublishError(f"Admission sets[{set_index}].parts must be non-empty.")
        actual_part_count += len(parts)
        for part_index, part in enumerate(parts):
            if not isinstance(part, dict):
                raise PublishError(
                    f"Admission sets[{set_index}].parts[{part_index}] must be an object."
                )
            model_asset_id = part.get("targetModelAssetId")
            _safe_asset_parts(model_asset_id)
            model_asset_ids.add(model_asset_id.casefold())
    closure = receipt.get("runtimeClosure")
    if not isinstance(closure, list) or not closure:
        raise PublishError("Admission runtimeClosure must be a non-empty array.")

    _require_reported_count(counts, "classCount", len(class_ids))
    _require_reported_count(counts, "visualSetCount", len(visual_sets))
    _require_reported_count(counts, "partCount", actual_part_count)
    _require_reported_count(counts, "runtimeClosureFileCount", len(closure))

    if len(class_ids) != EXPECTED_CLASS_COUNT:
        raise PublishError(
            f"Admission sets must cover {EXPECTED_CLASS_COUNT} classes, got {len(class_ids)}."
        )

    entries = _parse_entries(receipt, admission_root, resource_root)
    closure_asset_ids = {entry.asset_id.casefold() for entry in entries}
    missing_models = sorted(model_asset_ids - closure_asset_ids)
    if missing_models:
        raise PublishError(
            f"Admission part models are missing from runtimeClosure: {missing_models}"
        )
    groups = _group_entries(entries, resource_root)
    if len(groups) != EXPECTED_CLASS_COUNT:
        raise PublishError(
            f"runtimeClosure must publish {EXPECTED_CLASS_COUNT} Equipment directories, "
            f"got {len(groups)}."
        )
    return PublishPlan(
        admission_root=admission_root,
        resource_root=resource_root,
        entries=entries,
        groups=groups,
    )


def _promote_new_file(source: Path, target: Path) -> None:
    """Atomically expose a staged file without replacing an existing target."""

    os.link(source, target)
    try:
        source.unlink()
    except OSError:
        try:
            target.unlink()
        except OSError:
            pass
        raise


def _ensure_parent_directories(
    directory: Path,
    resource_root: Path,
    created_directories: list[Path],
) -> None:
    missing: list[Path] = []
    cursor = directory
    while cursor != resource_root and not cursor.exists():
        if cursor.is_symlink():
            raise PublishError(f"Resource target parent is a dangling symlink: {cursor}")
        missing.append(cursor)
        cursor = cursor.parent
    if cursor != resource_root and not cursor.is_dir():
        raise PublishError(f"Resource target parent is not a directory: {cursor}")
    for candidate in reversed(missing):
        try:
            candidate.mkdir()
            created_directories.append(candidate)
        except FileExistsError:
            if not candidate.is_dir():
                raise PublishError(
                    f"Resource target parent appeared as a non-directory: {candidate}"
                )


def publish_plan(
    plan: PublishPlan,
    promote_file: Callable[[Path, Path], object] = _promote_new_file,
) -> str:
    if not plan.new_entries:
        return "PUBLISHED_NO_CHANGE"

    stage_root = (
        plan.resource_root.parent
        / f".{plan.resource_root.name}.character-equipment-stage-{uuid4().hex}"
    )
    promoted: list[ClosureEntry] = []
    created_directories: list[Path] = []
    try:
        stage_root.mkdir(parents=False, exist_ok=False)
        for entry in plan.new_entries:
            staged_file = stage_root.joinpath(*PurePosixPath(entry.asset_id).parts)
            staged_file.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(entry.source_path, staged_file)
            if (
                staged_file.stat().st_size != entry.byte_size
                or sha256_file(staged_file) != entry.sha256
            ):
                raise PublishError(f"Staged asset verification failed: {entry.asset_id}")

        for entry in plan.new_entries:
            current_target = _path_beneath(
                plan.resource_root,
                PurePosixPath(entry.asset_id).parts,
                "Resource target",
            )
            if current_target != entry.target_path:
                raise PublishError(
                    f"Resource target changed after preflight: {entry.asset_id}"
                )
            if entry.target_path.exists() or entry.target_path.is_symlink():
                raise PublishError(
                    f"Resource target appeared after preflight: {entry.target_path}"
                )
            _ensure_parent_directories(
                entry.target_path.parent,
                plan.resource_root,
                created_directories,
            )
            staged_file = stage_root.joinpath(*PurePosixPath(entry.asset_id).parts)
            promote_file(staged_file, entry.target_path)
            promoted.append(entry)
            if (
                not entry.target_path.is_file()
                or entry.target_path.stat().st_size != entry.byte_size
                or sha256_file(entry.target_path) != entry.sha256
            ):
                raise PublishError(f"Promoted asset verification failed: {entry.asset_id}")
    except Exception as error:
        rollback_errors: list[str] = []
        for promoted_entry in reversed(promoted):
            try:
                if promoted_entry.target_path.exists() or promoted_entry.target_path.is_symlink():
                    promoted_entry.target_path.unlink()
            except OSError as rollback_error:
                rollback_errors.append(
                    f"{promoted_entry.target_path}: {rollback_error}"
                )
        for created_directory in reversed(created_directories):
            try:
                created_directory.rmdir()
            except OSError as rollback_error:
                if created_directory.exists() and not any(created_directory.iterdir()):
                    rollback_errors.append(f"{created_directory}: {rollback_error}")
        if rollback_errors:
            raise PublishError(
                f"Equipment publish failed ({error}); rollback also failed: "
                + "; ".join(rollback_errors)
            ) from error
        if isinstance(error, PublishError):
            raise
        raise PublishError(f"Equipment publish failed and was rolled back: {error}") from error
    finally:
        if stage_root.exists():
            shutil.rmtree(stage_root, ignore_errors=True)
    return "PUBLISHED"


def _print_plan(mode: str, plan: PublishPlan, result: str) -> None:
    print(f"Mode={mode}")
    print(f"Result={result}")
    print(f"AdmissionRoot={plan.admission_root}")
    print(f"ResourceRoot={plan.resource_root}")
    print(f"RuntimeClosureFiles={len(plan.entries)}")
    print(f"NewRuntimeClosureFiles={len(plan.new_entries)}")
    print(f"ExistingIdenticalRuntimeClosureFiles={len(plan.existing_entries)}")
    print(f"EquipmentDirectories={len(plan.groups)}")
    print(f"NewEquipmentDirectories={len(plan.new_groups)}")
    print(f"ExistingIdenticalEquipmentDirectories={len(plan.existing_groups)}")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mode", required=True, choices=("Validate", "Publish"))
    parser.add_argument("--admission-root", required=True, type=Path)
    parser.add_argument("--resource-root", required=True, type=Path)
    arguments = parser.parse_args(argv)
    try:
        plan = build_publish_plan(arguments.admission_root, arguments.resource_root)
        if arguments.mode == "Validate":
            result = "VALIDATED"
        else:
            result = publish_plan(plan)
        _print_plan(arguments.mode, plan, result)
        return 0
    except PublishError as error:
        print(f"Character equipment publisher failed: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
