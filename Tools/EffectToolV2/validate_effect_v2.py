"""Fail-closed validator for the preserved Effect Tool V2 contract."""

from __future__ import annotations

import argparse
import collections
import json
import math
import os
import sys
from pathlib import Path, PurePosixPath
from typing import Any, Mapping


MODULE_ROOT = Path(__file__).resolve().parent
if str(MODULE_ROOT) not in sys.path:
    sys.path.insert(0, str(MODULE_ROOT))
import effect_v2_binding_pipeline as binding_v2  # noqa: E402


AUTHORED_SCHEMA = "lostark.effect-v2"
BINDING_SCHEMA = "lostark.effect-v2-bindings"
GROUP_SCHEMA = "lostark.effect-v2-group"
INDEPENDENT_SCHEMA = "lostark.effect-v2-independent"
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


def _reject_duplicate_pairs(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ContractError(f"duplicate JSON property: {key}")
        result[key] = value
    return result


def _read_json(path: Path) -> Any:
    try:
        return json.loads(
            path.read_text(encoding="utf-8"),
            parse_constant=_reject_non_finite,
            object_pairs_hook=_reject_duplicate_pairs,
        )
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
        if document.get("schema") != AUTHORED_SCHEMA or not binding_v2._is_format_version(
            document.get("formatVersion"), 1
        ):
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


def _validate_groups(
    repository_root: Path, group_root: Path, authored: dict[str, Path]
) -> dict[str, list[tuple[str, int]]]:
    """Validate v1 compatibility groups and strict stable-child v2 groups."""
    groups: dict[str, list[tuple[str, int]]] = {}
    v2_group_ids: list[str] = []
    if not group_root.is_dir():
        return groups
    for path in sorted(group_root.glob("*.effectv2group.json")):
        document = _require_object(_read_json(path), path.as_posix())
        version = document.get("formatVersion")
        if document.get("schema") != GROUP_SCHEMA or not (
            binding_v2._is_format_version(version, 1)
            or binding_v2._is_format_version(version, 2)
        ):
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
        if binding_v2._is_format_version(version, 2):
            v2_group_ids.append(group_id)
            groups[group_id] = []
            continue
        _require_ms(document.get("durationMs", 0), group_id, "durationMs")
        children = document.get("children")
        if not isinstance(children, list) or not children:
            raise ContractError(f"Effect V2 group children must be a non-empty array: {group_id}")
        child_clocks: list[tuple[str, int]] = []
        for child in children:
            child = _require_object(child, f"{group_id}.child")
            effect_id = child.get("effectId")
            if effect_id not in authored:
                raise ContractError(
                    f"Effect V2 group child has no authored effect (nesting is not allowed): {group_id}: {effect_id}"
                )
            child_start_ms = _require_ms(
                child.get("startMs", 0), group_id, "child startMs"
            )
            _require_ms(child.get("durationMs", 0), group_id, "child durationMs")
            if child.get("stop", "Deactivate") not in CHILD_STOPS:
                raise ContractError(f"Effect V2 group child stop is invalid: {group_id}: {effect_id}")
            offset = child.get("offset", [0, 0, 0])
            if not isinstance(offset, list) or len(offset) != 3:
                raise ContractError(f"Effect V2 group child offset must be 3 numbers: {group_id}: {effect_id}")
            for component in offset:
                _require_finite_number(component, group_id, "child offset")
            _require_finite_number(child.get("pitchDegrees", 0), group_id, "child pitchDegrees")
            _require_finite_number(child.get("yawDegrees", 0), group_id, "child yawDegrees")
            _require_finite_number(child.get("rollDegrees", 0), group_id, "child rollDegrees")
            scale = child.get("scale", [1, 1, 1])
            if not isinstance(scale, list) or len(scale) != 3:
                raise ContractError(f"Effect V2 group child scale must be 3 numbers: {group_id}: {effect_id}")
            for component in scale:
                _require_finite_number(component, group_id, "child scale")
            child_clocks.append((effect_id, child_start_ms))
        _validate_numbers(document, group_id)
        groups[group_id] = child_clocks
    if v2_group_ids:
        try:
            resource_authored, resource_groups = binding_v2._load_resource_documents(
                repository_root
            )
            for group_id in v2_group_ids:
                leaves, _span_ms = binding_v2._resolve_group(
                    group_id,
                    resource_authored,
                    resource_groups,
                    require_v2=True,
                )
                groups[group_id] = [
                    (effect_id, start_ms)
                    for effect_id, start_ms, _child_ids, _transforms in leaves
                ]
        except binding_v2.BindingContractError as exc:
            raise ContractError(str(exc)) from exc
    return groups


def _load_canonical_clip_inventories(
    repository_root: Path, binding_root: Path
) -> dict[str, set[str]]:
    """Load clip IDs only for archetypes whose binding contract defines one."""
    boss_bindings = binding_root / "BOSS_VALTAN.effectv2bindings.json"
    if not boss_bindings.is_file():
        return {}
    binding_document = _require_object(_read_json(boss_bindings), boss_bindings.as_posix())
    if not binding_v2._is_format_version(binding_document.get("formatVersion"), 1):
        return {}
    presentation_path = repository_root / "Data/Valtan/Valtan.presentation.json"
    if not presentation_path.is_file():
        raise ContractError(
            "BOSS_VALTAN Effect V2 clip bindings require the canonical "
            f"Valtan presentation inventory: {presentation_path}"
        )
    presentation = _read_json(presentation_path)
    clips: set[str] = set()

    def collect(value: Any) -> None:
        if isinstance(value, dict):
            clip = value.get("clip")
            if isinstance(clip, str) and clip:
                clips.add(clip)
            for child in value.values():
                collect(child)
        elif isinstance(value, list):
            for child in value:
                collect(child)

    collect(presentation)
    if not clips:
        raise ContractError(
            "BOSS_VALTAN canonical presentation contains no admitted clip inventory"
        )
    return {"BOSS_VALTAN": clips}


def _load_independent(
    v2_root: Path, authored: dict[str, Path], groups: dict[str, list[tuple[str, int]]]
) -> tuple[set[str], set[str]]:
    path = v2_root / "Independent.json"
    if not path.is_file():
        return set(), set()
    document = _require_object(_read_json(path), path.as_posix())
    if document.get("schema") != INDEPENDENT_SCHEMA or not binding_v2._is_format_version(
        document.get("formatVersion"), 1
    ):
        raise ContractError(f"unsupported Effect V2 independent document: {path}")
    independent_effects: set[str] = set()
    independent_groups: set[str] = set()
    for key, known, admitted in (
        ("effects", authored, independent_effects),
        ("groups", groups, independent_groups),
    ):
        rows = document.get(key, [])
        if not isinstance(rows, list):
            raise ContractError(f"Effect V2 independent {key} must be an array: {path}")
        for row in rows:
            if not isinstance(row, str) or not row:
                raise ContractError(f"Effect V2 independent {key} entries must be IDs: {path}")
            if row not in known:
                raise ContractError(f"Effect V2 independent {key} entry has no document: {row}")
            if row in admitted:
                raise ContractError(f"duplicate Effect V2 independent {key} entry: {row}")
            admitted.add(row)
    return independent_effects, independent_groups


def _validate_bindings(
    repository_root: Path,
    resource_root: Path,
    binding_root: Path,
    authored: dict[str, Path],
    groups: dict[str, list[tuple[str, int]]],
    canonical_clips: Mapping[str, set[str]],
    independent_effects: set[str],
    independent_groups: set[str],
) -> tuple[int, int]:
    seen_archetypes: set[str] = set()
    seen_effects: set[str] = set(independent_effects)
    seen_groups: set[str] = set(independent_groups)
    for group_id in independent_groups:
        seen_effects.update(effect for effect, _ in groups[group_id])
    binding_count = 0
    boss_v1_compatibility_count = 0
    paths = sorted(binding_root.glob("*.effectv2bindings.json"))
    if not paths:
        raise ContractError("Effect V2 binding set is empty")
    for path in paths:
        document = _require_object(_read_json(path), path.as_posix())
        if document.get("schema") != BINDING_SCHEMA:
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
        if archetype_id == binding_v2.VALTAN_ARCHETYPE_ID and binding_v2._is_format_version(
            document.get("formatVersion"), binding_v2.BINDING_FORMAT_VERSION
        ):
            try:
                binding_v2.validate_binding_document(
                    repository_root,
                    document,
                    _read_json(repository_root / "Data/Valtan/Valtan.gameplay.json"),
                    _read_json(
                        repository_root
                        / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
                    ),
                    _read_json(
                        repository_root
                        / "Data/Valtan/Valtan.legacy-compatibility.json"
                    ),
                    resource_root=resource_root,
                )
            except binding_v2.BindingContractError as exc:
                raise ContractError(str(exc)) from exc
            for row in rows:
                resource = row["resource"]
                if resource["kind"] == "LEAF":
                    seen_effects.add(resource["id"])
                else:
                    group_id = resource["id"]
                    seen_groups.add(group_id)
                    seen_effects.update(effect for effect, _ in groups[group_id])
            binding_count += len(rows)
            continue
        if not binding_v2._is_format_version(document.get("formatVersion"), 1):
            raise ContractError(f"unsupported Effect V2 binding document: {path}")
        if archetype_id == binding_v2.VALTAN_ARCHETYPE_ID:
            boss_v1_compatibility_count += 1
        row_identities: set[tuple[Any, ...]] = set()
        validated_rows: list[tuple[str | None, str | None, Any, Any, int]] = []
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
            archetype_clips = canonical_clips.get(archetype_id)
            if (
                isinstance(clip, str)
                and clip
                and archetype_clips is not None
                and clip not in archetype_clips
            ):
                raise ContractError(
                    "Effect V2 clip binding is absent from the canonical "
                    f"{archetype_id} presentation inventory: {subject}: {clip}"
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
            offset = row.get("offset", [0, 0, 0])
            if not isinstance(offset, list) or len(offset) != 3:
                raise ContractError(f"Effect V2 binding offset must be 3 numbers: {archetype_id}: {subject}")
            for component in offset:
                _require_finite_number(component, archetype_id, "binding offset")
            _require_finite_number(row.get("yawDegrees", 0), archetype_id, "binding yawDegrees")
            identity = (effect_id, group_id, stage, clip, start_ms, row.get("bone"))
            if identity in row_identities:
                raise ContractError(f"duplicate Effect V2 binding row: {archetype_id}: {identity}")
            row_identities.add(identity)
            validated_rows.append((effect_id, group_id, stage, clip, start_ms))
            if has_effect:
                seen_effects.add(effect_id)
            else:
                seen_groups.add(group_id)
                seen_effects.update(effect for effect, _ in groups[group_id])
            binding_count += 1
        for effect_id, _, stage, clip, start_ms in validated_rows:
            if not effect_id:
                continue
            for _, group_id, group_stage, group_clip, group_start_ms in validated_rows:
                if not group_id or stage != group_stage or clip != group_clip:
                    continue
                for child_effect_id, child_start_ms in groups[group_id]:
                    if (
                        child_effect_id == effect_id
                        and group_start_ms + child_start_ms == start_ms
                    ):
                        raise ContractError(
                            "Effect V2 leaf overlaps the same leaf inside group "
                            f"at the same clock: {archetype_id}: {effect_id}/{group_id} "
                            f"@{start_ms}ms"
                        )
    missing = sorted(set(authored) - seen_effects)
    if missing:
        raise ContractError(f"unbound Effect V2 authored effects: {missing[:5]}")
    missing_groups = sorted(set(groups) - seen_groups)
    if missing_groups:
        raise ContractError(f"unbound Effect V2 groups: {missing_groups[:5]}")
    return binding_count, boss_v1_compatibility_count


def validate(repository_root: Path, resource_root: Path) -> dict[str, int]:
    resource_root = resolve_resource_root(repository_root, resource_root)
    v2_root = repository_root / "Data/Effects/V2"
    authored, usage = _validate_authored(v2_root / "Authored", resource_root)
    groups = _validate_groups(repository_root, v2_root / "Groups", authored)
    binding_root = v2_root / "Bindings"
    canonical_clips = _load_canonical_clip_inventories(
        repository_root, binding_root
    )
    independent_effects, independent_groups = _load_independent(v2_root, authored, groups)
    binding_count, boss_v1_compatibility_count = _validate_bindings(
        repository_root,
        resource_root,
        binding_root,
        authored,
        groups,
        canonical_clips,
        independent_effects,
        independent_groups,
    )
    return {
        "authored": len(authored),
        "bindings": binding_count,
        "groups": len(groups),
        "independent": len(independent_effects) + len(independent_groups),
        "textures": len(usage),
        "bossBindingCompatibilityV1": boss_v1_compatibility_count,
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
    compatibility = (
        " BOSS_VALTAN v1 compatibility is active; Composition Save remains "
        "disabled until the reviewed v2 migration."
        if report["bossBindingCompatibilityV1"]
        else ""
    )
    print(
        "Effect V2 validation succeeded: "
        f"{report['authored']} authored, {report['bindings']} bindings, "
        f"{report['groups']} groups, {report['independent']} independent, "
        f"{report['textures']} textures.{compatibility}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
