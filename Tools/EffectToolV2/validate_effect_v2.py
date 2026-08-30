"""Fail-closed validator for the preserved Effect Tool V2 contract."""

from __future__ import annotations

import argparse
import collections
import json
import math
from pathlib import Path, PurePosixPath
from typing import Any


AUTHORED_SCHEMA = "lostark.effect-v2"
BINDING_SCHEMA = "lostark.effect-v2-bindings"
EFFECT_TYPES = {"Decal", "Mesh", "Particle", "ScreenPost", "Texture", "Trail"}
SLOT_NAMES = ("mesh", "base", "noise", "mask", "emissive", "dissolve")
TEXTURE_SLOT_NAMES = ("base", "noise", "mask", "emissive", "dissolve")
ROTATIONS = {"Bone", "TargetYaw"}


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
            physical = resource_root.joinpath(*PurePosixPath(asset_id).parts)
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


def _validate_bindings(binding_root: Path, authored: dict[str, Path]) -> int:
    seen_archetypes: set[str] = set()
    seen_effects: set[str] = set()
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
            if effect_id not in authored:
                raise ContractError(f"Effect V2 binding has no authored effect: {effect_id}")
            stage = row.get("stage")
            clip = row.get("clip")
            if (isinstance(stage, str) and bool(stage)) == (
                isinstance(clip, str) and bool(clip)
            ):
                raise ContractError(
                    f"Effect V2 binding needs exactly one stage/clip: {archetype_id}: {effect_id}"
                )
            start_ms = row.get("startMs")
            if isinstance(start_ms, bool) or not isinstance(start_ms, int) or start_ms < 0:
                raise ContractError(f"Effect V2 startMs is invalid: {archetype_id}: {effect_id}")
            if not isinstance(row.get("bone"), str):
                raise ContractError(f"Effect V2 bone is invalid: {archetype_id}: {effect_id}")
            if not isinstance(row.get("followBone"), bool):
                raise ContractError(f"Effect V2 followBone is invalid: {archetype_id}: {effect_id}")
            if row.get("rotation") not in ROTATIONS:
                raise ContractError(f"Effect V2 rotation is invalid: {archetype_id}: {effect_id}")
            if not isinstance(row.get("stopWithClip"), bool):
                raise ContractError(f"Effect V2 stopWithClip is invalid: {archetype_id}: {effect_id}")
            identity = (effect_id, stage, clip, start_ms, row.get("bone"))
            if identity in row_identities:
                raise ContractError(f"duplicate Effect V2 binding row: {archetype_id}: {identity}")
            row_identities.add(identity)
            seen_effects.add(effect_id)
            binding_count += 1
    missing = sorted(set(authored) - seen_effects)
    if missing:
        raise ContractError(f"unbound Effect V2 authored effects: {missing[:5]}")
    return binding_count


def validate(repository_root: Path, resource_root: Path) -> dict[str, int]:
    v2_root = repository_root / "Data/Effects/V2"
    authored, usage = _validate_authored(v2_root / "Authored", resource_root)
    binding_count = _validate_bindings(v2_root / "Bindings", authored)
    return {
        "authored": len(authored),
        "bindings": binding_count,
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
    resource_root = (
        arguments.resource_root.resolve()
        if arguments.resource_root
        else repository_root / "Client/Bin/Resources"
    )
    try:
        report = validate(repository_root, resource_root)
    except ContractError as exc:
        print(f"Effect V2 validation failed: {exc}")
        return 1
    print(
        "Effect V2 validation succeeded: "
        f"{report['authored']} authored, {report['bindings']} bindings, "
        f"{report['textures']} textures."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
