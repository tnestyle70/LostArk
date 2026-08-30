"""Fail-closed validator for the preserved Effect Tool V2 contract."""

from __future__ import annotations

import argparse
import collections
import json
import math
import os
from pathlib import Path, PurePosixPath
from typing import Any, Mapping


AUTHORED_SCHEMA = "lostark.effect-v2"
BINDING_SCHEMA = "lostark.effect-v2-bindings"
GROUP_SCHEMA = "lostark.effect-v2-group"
EFFECT_TYPES = {"Decal", "Mesh", "Particle", "ScreenPost", "Texture", "Trail"}
SLOT_NAMES = ("mesh", "base", "noise", "mask", "emissive", "dissolve")
TEXTURE_SLOT_NAMES = ("base", "noise", "mask", "emissive", "dissolve")
ROTATIONS = {"Bone", "TargetYaw", "World"}
CHILD_STOPS = {"Kill", "Deactivate"}
MAX_MS = 600000
RESOURCE_ROOT_ENVIRONMENTS = (
    "LOSTARK_RESOURCE_ROOT",
    "LOSTARK_SHARED_ASSET_ROOT",
)


class ContractError(ValueError):
    pass


def _reject_non_finite(value: str) -> None:
    raise ContractError(f"non-finite JSON number is forbidden: {value}")


def _read_json(path: Path) -> Any:
    try:
        with path.open(encoding="utf-8") as handle:
            return json.load(handle, parse_constant=_reject_non_finite)
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise ContractError(f"invalid JSON: {path}: {exc}") from exc


def _require_object(value: Any, owner: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ContractError(f"{owner} must be an object")
    return value


def _require_ms(value: Any, owner: str, key: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < 0 or value > MAX_MS:
        raise ContractError(f"{owner} {key} must be an integer in [0, {MAX_MS}]")
    return value


def _require_finite_number(value: Any, owner: str, key: str) -> None:
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(float(value)):
        raise ContractError(f"{owner} {key} must be a finite number")


def _require_relative_asset(asset_id: str, slot: str, owner: str) -> None:
    if not asset_id:
        return
    if "\\" in asset_id:
        raise ContractError(f"{owner} {slot} uses a backslash path: {asset_id}")
    path = PurePosixPath(asset_id)
    if path.is_absolute() or ".." in path.parts or ":" in asset_id:
        raise ContractError(f"{owner} {slot} escapes Resources: {asset_id}")
    expected_suffix = ".wmodel" if slot == "mesh" else ".dds"
    if path.suffix.lower() != expected_suffix:
        raise ContractError(
            f"{owner} {slot} must reference {expected_suffix}: {asset_id}"
        )


def resolve_resource_root(
    repository_root: Path,
    explicit_root: Path | None = None,
    environment: Mapping[str, str] | None = None,
) -> Path:
    values = os.environ if environment is None else environment
    candidate = explicit_root
    if candidate is None:
        for name in RESOURCE_ROOT_ENVIRONMENTS:
            configured = values.get(name, "").strip()
            if configured:
                candidate = Path(configured)
                break
    if candidate is None:
        candidate = repository_root / "Client/Bin/Resources"
    try:
        resolved = candidate.resolve()
    except OSError as exc:
        raise ContractError(
            f"Effect V2 resource root is unavailable: {candidate}: {exc}"
        ) from exc
    if not resolved.is_dir():
        raise ContractError(f"Effect V2 resource root is unavailable: {resolved}")
    return resolved


def _resolve_resource_asset(
    resource_root: Path, asset_id: str, slot: str, owner: str
) -> Path:
    try:
        physical = resource_root.joinpath(*PurePosixPath(asset_id).parts).resolve()
        physical.relative_to(resource_root)
    except (OSError, ValueError) as exc:
        raise ContractError(
            f"{owner} {slot} escapes Resources after canonicalization: {asset_id}"
        ) from exc
    return physical


def _validate_numbers(value: Any, owner: str) -> None:
    if isinstance(value, bool) or value is None or isinstance(value, str):
        return
    if isinstance(value, (int, float)):
        if not math.isfinite(float(value)):
            raise ContractError(f"{owner} contains a non-finite number")
        return
    if isinstance(value, list):
        for item in value:
            _validate_numbers(item, owner)
        return
    if isinstance(value, dict):
        for item in value.values():
            _validate_numbers(item, owner)
        return
    raise ContractError(f"{owner} contains an unsupported JSON value")


def _validate_authored(
    authored_root: Path, resource_root: Path
) -> tuple[dict[str, Path], dict[str, collections.Counter[str]]]:
    by_id: dict[str, Path] = {}
    texture_usage: dict[str, collections.Counter[str]] = collections.defaultdict(
        collections.Counter
    )
    paths = sorted(authored_root.glob("*.effectv2.json"))
    if not paths:
        raise ContractError("Effect V2 authored set is empty")
    for path in paths:
        document = _require_object(_read_json(path), path.as_posix())
        if document.get("schema") != AUTHORED_SCHEMA or document.get("formatVersion") != 1:
            raise ContractError(f"unsupported Effect V2 document: {path}")
        effect_id = document.get("effectId")
        if not isinstance(effect_id, str) or not effect_id:
            raise ContractError(f"Effect V2 effectId is missing: {path}")
        if path.name != f"{effect_id}.effectv2.json":
            raise ContractError(f"Effect V2 filename/ID mismatch: {path}: {effect_id}")
        if effect_id in by_id:
            raise ContractError(f"duplicate Effect V2 effectId: {effect_id}")
        if document.get("effectType") not in EFFECT_TYPES:
            raise ContractError(f"unsupported Effect V2 effectType: {effect_id}")
        slots = _require_object(document.get("slots"), f"{effect_id}.slots")
        if set(slots) != set(SLOT_NAMES):
            raise ContractError(f"Effect V2 slots are not exact: {effect_id}")
        for slot in SLOT_NAMES:
            asset_id = slots[slot]
            if not isinstance(asset_id, str):
                raise ContractError(f"Effect V2 slot must be a string: {effect_id}: {slot}")
            _require_relative_asset(asset_id, slot, effect_id)
            if not asset_id:
                continue
            physical = _resolve_resource_asset(
                resource_root, asset_id, slot, effect_id
            )
            if not physical.is_file():
                raise ContractError(
                    f"Effect V2 resource is missing: {effect_id}: {slot}: {asset_id}"
                )
            if slot in TEXTURE_SLOT_NAMES:
                texture_usage[PurePosixPath(asset_id).name.lower()][slot] += 1
        _require_object(document.get("params"), f"{effect_id}.params")
        _validate_numbers(document, effect_id)
        by_id[effect_id] = path
    return by_id, texture_usage


def _validate_groups(group_root: Path, authored: dict[str, Path]) -> dict[str, list[str]]:
    """Group documents are optional; each child must be an authored leaf (no nesting)."""
    groups: dict[str, list[str]] = {}
    if not group_root.is_dir():
        return groups
    for path in sorted(group_root.glob("*.effectv2group.json")):
        document = _require_object(_read_json(path), path.as_posix())
        if document.get("schema") != GROUP_SCHEMA or document.get("formatVersion") != 1:
            raise ContractError(f"unsupported Effect V2 group document: {path}")
        group_id = document.get("groupId")
        if not isinstance(group_id, str) or not group_id:
            raise ContractError(f"Effect V2 groupId is missing: {path}")
        if path.name != f"{group_id}.effectv2group.json":
            raise ContractError(f"Effect V2 group filename/ID mismatch: {path}: {group_id}")
        if group_id in groups:
            raise ContractError(f"duplicate Effect V2 groupId: {group_id}")
        if group_id in authored:
            raise ContractError(f"Effect V2 groupId collides with an authored effect: {group_id}")
        _require_ms(document.get("durationMs", 0), group_id, "durationMs")
        children = document.get("children")
        if not isinstance(children, list) or not children:
            raise ContractError(f"Effect V2 group children must be a non-empty array: {group_id}")
        child_ids: list[str] = []
        identities: set[tuple[str, int]] = set()
        for child in children:
            child = _require_object(child, f"{group_id}.child")
            effect_id = child.get("effectId")
            if effect_id not in authored:
                raise ContractError(
                    f"Effect V2 group child has no authored effect (nesting is not allowed): {group_id}: {effect_id}"
                )
            start_ms = _require_ms(child.get("startMs", 0), group_id, "child startMs")
            _require_ms(child.get("durationMs", 0), group_id, "child durationMs")
            if child.get("stop", "Deactivate") not in CHILD_STOPS:
                raise ContractError(f"Effect V2 group child stop is invalid: {group_id}: {effect_id}")
            offset = child.get("offset", [0, 0, 0])
            if not isinstance(offset, list) or len(offset) != 3:
                raise ContractError(f"Effect V2 group child offset must be 3 numbers: {group_id}: {effect_id}")
            for component in offset:
                _require_finite_number(component, group_id, "child offset")
            _require_finite_number(child.get("yawDegrees", 0), group_id, "child yawDegrees")
            identity = (effect_id, start_ms)
            if identity in identities:
                raise ContractError(f"duplicate Effect V2 group child: {group_id}: {identity}")
            identities.add(identity)
            child_ids.append(effect_id)
        _validate_numbers(document, group_id)
        groups[group_id] = child_ids
    return groups


def _validate_bindings(
    binding_root: Path, authored: dict[str, Path], groups: dict[str, list[str]]
) -> int:
    seen_archetypes: set[str] = set()
    seen_effects: set[str] = set()
    seen_groups: set[str] = set()
    binding_count = 0
    paths = sorted(binding_root.glob("*.effectv2bindings.json"))
    if not paths:
        raise ContractError("Effect V2 binding set is empty")
    for path in paths:
        document = _require_object(_read_json(path), path.as_posix())
        if document.get("schema") != BINDING_SCHEMA or document.get("formatVersion") != 1:
            raise ContractError(f"unsupported Effect V2 binding document: {path}")
        archetype_id = document.get("archetypeId")
        if not isinstance(archetype_id, str) or not archetype_id:
            raise ContractError(f"Effect V2 archetypeId is missing: {path}")
        if archetype_id in seen_archetypes:
            raise ContractError(f"duplicate Effect V2 archetypeId: {archetype_id}")
        seen_archetypes.add(archetype_id)
        rows = document.get("bindings")
        if not isinstance(rows, list):
            raise ContractError(f"Effect V2 bindings must be an array: {path}")
        row_identities: set[tuple[Any, ...]] = set()
        for row in rows:
            row = _require_object(row, f"{archetype_id}.binding")
            effect_id = row.get("effectId")
            group_id = row.get("group")
            has_effect = isinstance(effect_id, str) and bool(effect_id)
            has_group = isinstance(group_id, str) and bool(group_id)
            if has_effect == has_group:
                raise ContractError(
                    f"Effect V2 binding needs exactly one effectId/group: {archetype_id}: {effect_id}/{group_id}"
                )
            subject = effect_id if has_effect else group_id
            if has_effect and effect_id not in authored:
                raise ContractError(f"Effect V2 binding has no authored effect: {effect_id}")
            if has_group and group_id not in groups:
                raise ContractError(f"Effect V2 binding has no group document: {group_id}")
            stage = row.get("stage")
            clip = row.get("clip")
            if (isinstance(stage, str) and bool(stage)) == (
                isinstance(clip, str) and bool(clip)
            ):
                raise ContractError(
                    f"Effect V2 binding needs exactly one stage/clip: {archetype_id}: {subject}"
                )
            start_ms = _require_ms(row.get("startMs"), archetype_id, f"{subject} startMs")
            if not isinstance(row.get("bone"), str):
                raise ContractError(f"Effect V2 bone is invalid: {archetype_id}: {subject}")
            if not isinstance(row.get("followBone"), bool):
                raise ContractError(f"Effect V2 followBone is invalid: {archetype_id}: {subject}")
            if row.get("rotation") not in ROTATIONS:
                raise ContractError(f"Effect V2 rotation is invalid: {archetype_id}: {subject}")
            if not isinstance(row.get("stopWithClip"), bool):
                raise ContractError(f"Effect V2 stopWithClip is invalid: {archetype_id}: {subject}")
            identity = (effect_id, group_id, stage, clip, start_ms, row.get("bone"))
            if identity in row_identities:
                raise ContractError(f"duplicate Effect V2 binding row: {archetype_id}: {identity}")
            row_identities.add(identity)
            if has_effect:
                seen_effects.add(effect_id)
            else:
                seen_groups.add(group_id)
                seen_effects.update(groups[group_id])
            binding_count += 1
    missing = sorted(set(authored) - seen_effects)
    if missing:
        raise ContractError(f"unbound Effect V2 authored effects: {missing[:5]}")
    missing_groups = sorted(set(groups) - seen_groups)
    if missing_groups:
        raise ContractError(f"unbound Effect V2 groups: {missing_groups[:5]}")
    return binding_count


def validate(repository_root: Path, resource_root: Path) -> dict[str, int]:
    resource_root = resolve_resource_root(repository_root, resource_root)
    v2_root = repository_root / "Data/Effects/V2"
    authored, usage = _validate_authored(v2_root / "Authored", resource_root)
    groups = _validate_groups(v2_root / "Groups", authored)
    binding_count = _validate_bindings(v2_root / "Bindings", authored, groups)
    return {
        "authored": len(authored),
        "bindings": binding_count,
        "groups": len(groups),
        "textures": len(usage),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--repository-root", type=Path, default=Path(__file__).resolve().parents[2]
    )
    parser.add_argument("--resource-root", type=Path)
    arguments = parser.parse_args()
    repository_root = arguments.repository_root.resolve()
    try:
        resource_root = resolve_resource_root(
            repository_root, arguments.resource_root
        )
        report = validate(repository_root, resource_root)
    except ContractError as exc:
        print(f"Effect V2 validation failed: {exc}")
        return 1
    print(
        "Effect V2 validation succeeded: "
        f"{report['authored']} authored, {report['bindings']} bindings, "
        f"{report['groups']} groups, {report['textures']} textures."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
