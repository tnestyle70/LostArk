from __future__ import annotations

import argparse
import copy
import json
import math
import os
import re
import shlex
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Sequence


AREA_ID = "LV_LUT_HEARTRB_ED"
SOURCE_COUNT = 30
SOURCE_ID_BASE = 1_090_000_000_000_000
FILLER_ID_BASE = 1_091_000_000_000_000
SOURCE_PREFIX = "Authored:OuterRing109:"
FILLER_PREFIX = "Authored:OuterRing109GapFiller:"
SOURCE_ASSET_ID = "DEPLOY_ITR_02306"
FILLER_ASSET_ID = "DEPLOY_ITR_02307"
FILLER_DEPLOY_ACTOR_ID = 109_100_001
FILLER_PROP_DEFINITION_ID = 109_100_002
GROUP_PREFIX = "destroyable.group.valtan.outerwall109."
ARENA_CENTER_X = 156.03
ARENA_CENTER_Z = -122.06
ARENA_RADIUS = 16.1
ARENA_Y = 23.04
ANGLE_STEP_DEGREES = 12.0
FILLER_ANGLE_OFFSET_DEGREES = 6.0
FILLER_SCALE = 1.52
POSITION_EPSILON = 2.0e-5
ANGLE_EPSILON_DEGREES = 2.0e-5
QUATERNION_EPSILON = 2.0e-6

PLACEMENT_HEADER = re.compile(
    r'^LOSTARK_DEPLOY_PROP_PLACEMENTS\s+1\s+"(?P<area>[A-Za-z0-9_.-]+)"\s+'
    r'(?P<count>[0-9]+)$'
)


class SyncError(ValueError):
    pass


class OutOfSyncError(SyncError):
    def __init__(self, paths: Sequence[Path]):
        self.paths = tuple(paths)
        super().__init__(
            "Valtan 109 outer-wall gap fillers are out of sync: "
            + ", ".join(str(path) for path in self.paths)
        )


@dataclass(frozen=True)
class SyncPaths:
    deploy_placements: Path
    world_events: Path
    destruction_simulation: Path


@dataclass(frozen=True)
class DeployRow:
    runtime_id: int
    deploy_actor_id: int
    prop_definition_id: int
    source_id: str
    asset_id: str
    position: tuple[float, float, float]
    quaternion: tuple[float, float, float, float]
    uniform_scale: float
    destructible: int
    state_off_action_id: int
    trigger_occurrence_count: int
    original_line: str


@dataclass(frozen=True)
class DeployDocument:
    newline: str
    rows: tuple[DeployRow, ...]
    original_text: str


@dataclass(frozen=True)
class PairSpec:
    suffix: int
    source_id: int
    filler_id: int
    source_angle_degrees: float
    filler_angle_degrees: float

    @property
    def group_id(self) -> str:
        return GROUP_PREFIX + str(self.source_id)


def _read_text_exact(path: Path, context: str) -> str:
    try:
        data = path.read_bytes()
        if data.startswith(b"\xef\xbb\xbf"):
            raise SyncError(f"{context} must not contain a UTF-8 BOM: {path}")
        return data.decode("utf-8")
    except (OSError, UnicodeError) as error:
        raise SyncError(f"{context} load failed: {path}: {error}") from error


def _detect_newline(text: str, context: str) -> str:
    without_crlf = text.replace("\r\n", "")
    if "\r" in without_crlf:
        raise SyncError(f"{context} contains unsupported carriage returns")
    if "\r\n" in text:
        if "\n" in without_crlf:
            raise SyncError(f"{context} mixes CRLF and LF newlines")
        return "\r\n"
    if "\n" in text:
        return "\n"
    raise SyncError(f"{context} must contain line breaks")


def _parse_uint(value: str, maximum: int, context: str) -> int:
    try:
        parsed = int(value, 10)
    except ValueError as error:
        raise SyncError(f"{context} is not an integer") from error
    if parsed <= 0 or parsed > maximum:
        raise SyncError(f"{context} is outside 1..{maximum}")
    return parsed


def _parse_finite(value: str, context: str) -> float:
    try:
        parsed = float(value)
    except ValueError as error:
        raise SyncError(f"{context} is not numeric") from error
    if not math.isfinite(parsed):
        raise SyncError(f"{context} is not finite")
    return parsed


def _parse_deploy_row(line: str, line_number: int) -> DeployRow:
    try:
        fields = shlex.split(line, posix=True)
    except ValueError as error:
        raise SyncError(f"deploy placement row {line_number} has invalid quoting") from error
    if len(fields) != 16:
        raise SyncError(
            f"deploy placement row {line_number} has {len(fields)} fields instead of 16"
        )
    runtime_id = _parse_uint(fields[0], 0xFFFF_FFFF_FFFF_FFFF, f"row {line_number} runtime ID")
    deploy_actor_id = _parse_uint(fields[1], 0xFFFF_FFFF, f"row {line_number} deploy actor ID")
    prop_definition_id = _parse_uint(fields[2], 0xFFFF_FFFF, f"row {line_number} prop definition ID")
    position = tuple(
        _parse_finite(fields[index], f"row {line_number} position") for index in range(5, 8)
    )
    quaternion = tuple(
        _parse_finite(fields[index], f"row {line_number} quaternion") for index in range(8, 12)
    )
    scale = _parse_finite(fields[12], f"row {line_number} uniform scale")
    if scale <= 0.0:
        raise SyncError(f"row {line_number} uniform scale must be positive")
    destructible = _parse_uint_or_zero(fields[13], 1, f"row {line_number} destructible")
    state_off_action = _parse_uint_or_zero(
        fields[14], 0xFFFF_FFFF, f"row {line_number} state-off action ID"
    )
    occurrence_count = _parse_uint_or_zero(
        fields[15], 0xFFFF_FFFF, f"row {line_number} trigger occurrence count"
    )
    if not fields[3] or not fields[4]:
        raise SyncError(f"row {line_number} has an empty source or asset ID")
    norm = math.sqrt(sum(component * component for component in quaternion))
    if norm <= 1.0e-8:
        raise SyncError(f"row {line_number} has a zero quaternion")
    return DeployRow(
        runtime_id=runtime_id,
        deploy_actor_id=deploy_actor_id,
        prop_definition_id=prop_definition_id,
        source_id=fields[3],
        asset_id=fields[4],
        position=position,  # type: ignore[arg-type]
        quaternion=quaternion,  # type: ignore[arg-type]
        uniform_scale=scale,
        destructible=destructible,
        state_off_action_id=state_off_action,
        trigger_occurrence_count=occurrence_count,
        original_line=line,
    )


def _parse_uint_or_zero(value: str, maximum: int, context: str) -> int:
    try:
        parsed = int(value, 10)
    except ValueError as error:
        raise SyncError(f"{context} is not an integer") from error
    if parsed < 0 or parsed > maximum:
        raise SyncError(f"{context} is outside 0..{maximum}")
    return parsed


def parse_deploy_document(path: Path) -> DeployDocument:
    text = _read_text_exact(path, "deploy placement document")
    newline = _detect_newline(text, "deploy placement document")
    if not text.endswith(newline):
        raise SyncError("deploy placement document must end with a newline")
    lines = text[: -len(newline)].split(newline)
    if not lines:
        raise SyncError("deploy placement document is empty")
    header = PLACEMENT_HEADER.fullmatch(lines[0])
    if header is None or header.group("area") != AREA_ID:
        raise SyncError(f"deploy placement header must target {AREA_ID}")
    expected_count = int(header.group("count"), 10)
    if expected_count != len(lines) - 1:
        raise SyncError("deploy placement header count does not match row count")
    rows = tuple(_parse_deploy_row(line, index) for index, line in enumerate(lines[1:], 2))
    runtime_ids: set[int] = set()
    source_ids: set[str] = set()
    for row in rows:
        if row.runtime_id in runtime_ids:
            raise SyncError(f"duplicate deploy runtime ID: {row.runtime_id}")
        if row.source_id in source_ids:
            raise SyncError(f"duplicate deploy source ID: {row.source_id}")
        runtime_ids.add(row.runtime_id)
        source_ids.add(row.source_id)
    return DeployDocument(newline=newline, rows=rows, original_text=text)


def _angle_degrees(x: float, z: float) -> float:
    return math.degrees(math.atan2(z - ARENA_CENTER_Z, x - ARENA_CENTER_X)) % 360.0


def _angular_distance(left: float, right: float) -> float:
    return abs((left - right + 180.0) % 360.0 - 180.0)


def _expected_quaternion(angle_degrees: float) -> tuple[float, float, float, float]:
    yaw_radians = math.radians(90.0 - angle_degrees)
    return (0.0, math.sin(yaw_radians * 0.5), 0.0, math.cos(yaw_radians * 0.5))


def _quaternion_matches(
    actual: tuple[float, float, float, float], expected: tuple[float, float, float, float]
) -> bool:
    actual_norm = math.sqrt(sum(value * value for value in actual))
    expected_norm = math.sqrt(sum(value * value for value in expected))
    dot = sum(left * right for left, right in zip(actual, expected)) / (actual_norm * expected_norm)
    return abs(abs(dot) - 1.0) <= QUATERNION_EPSILON


def build_pair_specs(document: DeployDocument) -> tuple[PairSpec, ...]:
    rows_by_id = {row.runtime_id: row for row in document.rows}
    source_angles: list[float] = []
    pairs: list[PairSpec] = []
    for suffix in range(1, SOURCE_COUNT + 1):
        source_id = SOURCE_ID_BASE + suffix
        row = rows_by_id.get(source_id)
        if row is None:
            raise SyncError(f"109 outer-wall source placement is missing: {source_id}")
        if row.source_id != SOURCE_PREFIX + str(source_id) or row.asset_id != SOURCE_ASSET_ID:
            raise SyncError(f"109 outer-wall source identity drifted: {source_id}")
        radius = math.hypot(
            row.position[0] - ARENA_CENTER_X, row.position[2] - ARENA_CENTER_Z
        )
        if abs(radius - ARENA_RADIUS) > POSITION_EPSILON or abs(row.position[1] - ARENA_Y) > POSITION_EPSILON:
            raise SyncError(f"109 outer-wall source transform drifted: {source_id}")
        angle = _angle_degrees(row.position[0], row.position[2])
        grid_index = int(round(angle / ANGLE_STEP_DEGREES)) % SOURCE_COUNT
        grid_angle = grid_index * ANGLE_STEP_DEGREES
        if _angular_distance(angle, grid_angle) > ANGLE_EPSILON_DEGREES:
            raise SyncError(f"109 outer-wall source is off the 12-degree grid: {source_id}")
        if not _quaternion_matches(row.quaternion, _expected_quaternion(grid_angle)):
            raise SyncError(f"109 outer-wall source tangent rotation drifted: {source_id}")
        if (
            abs(row.uniform_scale - 1.0) > 1.0e-8
            or row.destructible != 1
            or row.state_off_action_id != 0
            or row.trigger_occurrence_count != 0
        ):
            raise SyncError(f"109 outer-wall source state drifted: {source_id}")
        source_angles.append(grid_angle)
        filler_angle = (grid_angle + FILLER_ANGLE_OFFSET_DEGREES) % 360.0
        pairs.append(
            PairSpec(
                suffix=suffix,
                source_id=source_id,
                filler_id=FILLER_ID_BASE + suffix,
                source_angle_degrees=grid_angle,
                filler_angle_degrees=filler_angle,
            )
        )
    expected_source_angles = [ANGLE_STEP_DEGREES * index for index in range(SOURCE_COUNT)]
    if sorted(source_angles) != expected_source_angles:
        raise SyncError("109 outer-wall source angles do not cover the complete 0..348 grid")
    expected_filler_angles = [
        FILLER_ANGLE_OFFSET_DEGREES + ANGLE_STEP_DEGREES * index
        for index in range(SOURCE_COUNT)
    ]
    if sorted(pair.filler_angle_degrees for pair in pairs) != expected_filler_angles:
        raise SyncError("109 filler angles do not cover the complete 6..354 grid")
    return tuple(pairs)


def _filler_source_id(filler_id: int) -> str:
    return FILLER_PREFIX + str(filler_id)


def _quote(value: str) -> str:
    return json.dumps(value, ensure_ascii=False)


def _filler_line(pair: PairSpec) -> str:
    radians = math.radians(pair.filler_angle_degrees)
    x = ARENA_CENTER_X + ARENA_RADIUS * math.cos(radians)
    z = ARENA_CENTER_Z + ARENA_RADIUS * math.sin(radians)
    quaternion = _expected_quaternion(pair.filler_angle_degrees)
    return (
        f"{pair.filler_id} {FILLER_DEPLOY_ACTOR_ID} {FILLER_PROP_DEFINITION_ID} "
        f"{_quote(_filler_source_id(pair.filler_id))} {_quote(FILLER_ASSET_ID)} "
        f"{x:.6f} {ARENA_Y:.6f} {z:.6f} "
        f"{quaternion[0]:.9f} {quaternion[1]:.9f} "
        f"{quaternion[2]:.9f} {quaternion[3]:.9f} "
        f"{FILLER_SCALE:.2f} 1 0 0"
    )


def build_deploy_output(document: DeployDocument, pairs: Sequence[PairSpec]) -> str:
    pair_by_filler = {pair.filler_id: pair for pair in pairs}
    expected_sources = {
        _filler_source_id(pair.filler_id): pair.filler_id for pair in pairs
    }
    preserved: list[str] = []
    for row in document.rows:
        id_owned = row.runtime_id in pair_by_filler
        source_owner = expected_sources.get(row.source_id)
        prefix_owned = row.source_id.startswith(FILLER_PREFIX)
        if id_owned:
            if source_owner != row.runtime_id:
                raise SyncError(
                    f"filler runtime ID is occupied by an unrelated placement: {row.runtime_id}"
                )
            continue
        if source_owner is not None or prefix_owned:
            raise SyncError(f"filler source ID collides with another runtime ID: {row.source_id}")
        preserved.append(row.original_line)
    generated = [_filler_line(pair) for pair in pairs]
    rows = preserved + generated
    return (
        f'LOSTARK_DEPLOY_PROP_PLACEMENTS 1 "{AREA_ID}" {len(rows)}'
        + document.newline
        + document.newline.join(rows)
        + document.newline
    )


def _load_json_document(path: Path, context: str) -> tuple[str, dict[str, Any]]:
    text = _read_text_exact(path, context)
    _detect_newline(text, context)
    try:
        value = json.loads(text)
    except json.JSONDecodeError as error:
        raise SyncError(f"{context} JSON is malformed: {path}: {error}") from error
    if not isinstance(value, dict):
        raise SyncError(f"{context} root must be an object")
    return text, value


def _string_list(value: Any, context: str) -> list[str]:
    if not isinstance(value, list) or any(not isinstance(item, str) for item in value):
        raise SyncError(f"{context} must be an array of strings")
    return value


def _replace_anchored_array(
    text: str,
    *,
    anchor_property: str,
    anchor_value: str,
    array_property: str,
    following_property: str,
    values: Sequence[str],
    context: str,
) -> str:
    newline = _detect_newline(text, context)
    pattern = re.compile(
        rf'(?P<prefix>"{re.escape(anchor_property)}"\s*:\s*'
        rf'"{re.escape(anchor_value)}"\s*,\s*'
        rf'"{re.escape(array_property)}"\s*:\s*)'
        rf'\[(?P<body>[^\]]*)\]'
        rf'(?P<suffix>\s*,\s*"{re.escape(following_property)}"\s*:)',
        re.DOTALL,
    )
    matches = list(pattern.finditer(text))
    if len(matches) != 1:
        raise SyncError(f"{context} array text anchor count is {len(matches)} instead of 1")
    match = matches[0]
    body = match.group("body")
    if newline in body:
        tail = body.rsplit(newline, 1)[1]
        if tail.strip():
            raise SyncError(f"{context} closing array indentation is malformed")
        closing_indent = tail
        value_indent = None
        for line in body.split(newline):
            if line.strip():
                value_indent = line[: len(line) - len(line.lstrip(" \t"))]
                break
        if value_indent is None:
            value_indent = closing_indent + "    "
        rendered_body = newline
        if values:
            rendered_body += ("," + newline).join(
                value_indent + _quote(value) for value in values
            )
            rendered_body += newline + closing_indent
        else:
            rendered_body += newline + closing_indent
    else:
        rendered_body = " " + ", ".join(_quote(value) for value in values) + " " if values else ""
    return text[: match.start("body")] + rendered_body + text[match.end("body") :]


def build_events_output(
    path: Path, pairs: Sequence[PairSpec]
) -> str:
    text, document = _load_json_document(path, "Valtan world events")
    if (
        document.get("schema") != "lostark.world-destruction-events"
        or document.get("formatVersion") != 1
        or document.get("areaId") != AREA_ID
    ):
        raise SyncError("Valtan world events header is invalid")
    groups = document.get("groups")
    if not isinstance(groups, list):
        raise SyncError("Valtan world events groups must be an array")
    by_id: dict[str, dict[str, Any]] = {}
    all_members: dict[str, str] = {}
    for index, group in enumerate(groups):
        if not isinstance(group, dict) or not isinstance(group.get("groupId"), str):
            raise SyncError(f"Valtan world events group {index} is malformed")
        group_id = group["groupId"]
        if group_id in by_id:
            raise SyncError(f"duplicate Valtan world events group: {group_id}")
        by_id[group_id] = group
        for member in _string_list(group.get("memberPlacementIds"), f"{group_id}.memberPlacementIds"):
            if member in all_members:
                raise SyncError(f"world events placement belongs to multiple groups: {member}")
            all_members[member] = group_id

    expected = copy.deepcopy(document)
    expected_groups = {group["groupId"]: group for group in expected["groups"]}
    output = text
    for pair in pairs:
        group = by_id.get(pair.group_id)
        if group is None:
            raise SyncError(f"109 outer-wall group is missing: {pair.group_id}")
        primary = str(pair.source_id)
        filler = str(pair.filler_id)
        members = _string_list(group.get("memberPlacementIds"), f"{pair.group_id}.memberPlacementIds")
        if members not in ([primary], [primary, filler]):
            raise SyncError(f"109 outer-wall group members drifted: {pair.group_id}")
        owner = all_members.get(filler)
        if owner is not None and owner != pair.group_id:
            raise SyncError(f"filler belongs to the wrong destruction group: {filler}/{owner}")
        expected_groups[pair.group_id]["memberPlacementIds"] = [primary, filler]
        output = _replace_anchored_array(
            output,
            anchor_property="groupId",
            anchor_value=pair.group_id,
            array_property="memberPlacementIds",
            following_property="navigationRegionIds",
            values=(primary, filler),
            context=f"world events {pair.group_id}",
        )
    try:
        rendered = json.loads(output)
    except json.JSONDecodeError as error:
        raise SyncError(f"world events staged JSON is malformed: {error}") from error
    if rendered != expected:
        raise SyncError("world events text update changed data outside filler members")
    return output


def build_simulation_output(
    path: Path, pairs: Sequence[PairSpec]
) -> str:
    text, document = _load_json_document(path, "destruction simulation")
    if (
        document.get("schema") != "lostark.destruction-simulation"
        or document.get("formatVersion") != 2
        or document.get("areaId") != AREA_ID
    ):
        raise SyncError("destruction simulation header is invalid")
    profiles = document.get("profiles")
    if not isinstance(profiles, list):
        raise SyncError("destruction simulation profiles must be an array")
    by_group: dict[str, dict[str, Any]] = {}
    all_source_ids: set[str] = set()
    all_alias_owners: dict[str, str] = {}
    for index, profile in enumerate(profiles):
        if not isinstance(profile, dict) or not isinstance(profile.get("groupId"), str):
            raise SyncError(f"destruction simulation profile {index} is malformed")
        group_id = profile["groupId"]
        if group_id in by_group:
            raise SyncError(f"duplicate destruction simulation group: {group_id}")
        by_group[group_id] = profile
        elements = profile.get("elements")
        if not isinstance(elements, list):
            raise SyncError(f"{group_id}.elements must be an array")
        for element in elements:
            if not isinstance(element, dict) or not isinstance(
                element.get("sourceRuntimePlacementId"), str
            ):
                raise SyncError(f"{group_id} has a malformed emitter")
            source = element["sourceRuntimePlacementId"]
            if source in all_source_ids:
                raise SyncError(f"duplicate destruction simulation source: {source}")
            all_source_ids.add(source)
            for alias in _string_list(
                element.get("suppressionAliasPlacementIds"),
                f"{group_id}.suppressionAliasPlacementIds",
            ):
                if alias in all_alias_owners:
                    raise SyncError(f"duplicate destruction simulation alias: {alias}")
                all_alias_owners[alias] = group_id

    expected = copy.deepcopy(document)
    expected_profiles = {profile["groupId"]: profile for profile in expected["profiles"]}
    output = text
    for pair in pairs:
        profile = by_group.get(pair.group_id)
        if profile is None:
            raise SyncError(f"109 destruction simulation profile is missing: {pair.group_id}")
        elements = profile["elements"]
        if len(elements) != 1:
            raise SyncError(f"109 destruction profile must have one emitter: {pair.group_id}")
        element = elements[0]
        primary = str(pair.source_id)
        filler = str(pair.filler_id)
        if element.get("sourceRuntimePlacementId") != primary:
            raise SyncError(f"109 destruction source drifted: {pair.group_id}")
        aliases = _string_list(
            element.get("suppressionAliasPlacementIds"),
            f"{pair.group_id}.suppressionAliasPlacementIds",
        )
        if aliases not in ([], [filler]):
            raise SyncError(f"109 destruction alias drifted: {pair.group_id}")
        owner = all_alias_owners.get(filler)
        if owner is not None and owner != pair.group_id:
            raise SyncError(f"filler alias belongs to the wrong emitter: {filler}/{owner}")
        if filler in all_source_ids:
            raise SyncError(f"filler alias is also a debris source: {filler}")
        expected_profiles[pair.group_id]["elements"][0][
            "suppressionAliasPlacementIds"
        ] = [filler]
        output = _replace_anchored_array(
            output,
            anchor_property="sourceRuntimePlacementId",
            anchor_value=primary,
            array_property="suppressionAliasPlacementIds",
            following_property="spawnOffset",
            values=(filler,),
            context=f"destruction simulation {pair.group_id}",
        )
    try:
        rendered = json.loads(output)
    except json.JSONDecodeError as error:
        raise SyncError(f"destruction simulation staged JSON is malformed: {error}") from error
    if rendered != expected:
        raise SyncError("destruction simulation text update changed data outside filler aliases")
    return output


def build_outputs(paths: SyncPaths) -> dict[Path, bytes]:
    deploy = parse_deploy_document(paths.deploy_placements)
    pairs = build_pair_specs(deploy)
    outputs = {
        paths.deploy_placements: build_deploy_output(deploy, pairs).encode("utf-8"),
        paths.world_events: build_events_output(paths.world_events, pairs).encode("utf-8"),
        paths.destruction_simulation: build_simulation_output(
            paths.destruction_simulation, pairs
        ).encode("utf-8"),
    }
    return outputs


def _stage_bytes(path: Path, data: bytes, suffix: str) -> Path:
    handle, temporary_name = tempfile.mkstemp(
        prefix=path.name + ".", suffix=suffix, dir=path.parent
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(handle, "wb") as stream:
            stream.write(data)
            stream.flush()
            os.fsync(stream.fileno())
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise
    return temporary


def commit_outputs(
    outputs: dict[Path, bytes], *, failure_after_promote: int | None = None
) -> tuple[Path, ...]:
    changed = tuple(path for path, data in outputs.items() if path.read_bytes() != data)
    if not changed:
        return ()
    staged: dict[Path, Path] = {}
    backups: dict[Path, Path] = {}
    promoted = 0
    try:
        for path in changed:
            staged[path] = _stage_bytes(path, outputs[path], ".stage")
        for path in changed:
            backup = _stage_bytes(path, b"", ".backup")
            backup.unlink()
            os.replace(path, backup)
            backups[path] = backup
            os.replace(staged[path], path)
            promoted += 1
            if failure_after_promote is not None and promoted >= failure_after_promote:
                raise OSError(f"injected failure after promote {promoted}")
        for backup in backups.values():
            backup.unlink(missing_ok=True)
        return changed
    except BaseException as error:
        rollback_errors: list[str] = []
        for path in reversed(changed):
            backup = backups.get(path)
            if backup is None or not backup.exists():
                continue
            try:
                path.unlink(missing_ok=True)
                os.replace(backup, path)
            except OSError as rollback_error:
                rollback_errors.append(f"{path}: {rollback_error}")
        for temporary in (*staged.values(), *backups.values()):
            temporary.unlink(missing_ok=True)
        if rollback_errors:
            raise SyncError(
                f"gap-filler commit failed ({error}); rollback failed: "
                + "; ".join(rollback_errors)
            ) from error
        raise SyncError(f"gap-filler commit failed and rolled back: {error}") from error
    finally:
        for temporary in staged.values():
            temporary.unlink(missing_ok=True)


def synchronize(
    paths: SyncPaths,
    *,
    check_only: bool = False,
    failure_after_promote: int | None = None,
) -> tuple[Path, ...]:
    unique_paths = {
        paths.deploy_placements,
        paths.world_events,
        paths.destruction_simulation,
    }
    if len(unique_paths) != 3:
        raise SyncError("the three synchronized paths must be distinct")
    for path in unique_paths:
        if not path.is_file():
            raise SyncError(f"required input file is missing: {path}")
    outputs = build_outputs(paths)
    stale = tuple(path for path, data in outputs.items() if path.read_bytes() != data)
    if check_only:
        if stale:
            raise OutOfSyncError(stale)
        return ()
    return commit_outputs(outputs, failure_after_promote=failure_after_promote)


def default_paths() -> SyncPaths:
    repository = Path(__file__).resolve().parents[2]
    area_authoring = repository / "Data" / "Maps" / "Authoring" / AREA_ID
    return SyncPaths(
        deploy_placements=area_authoring / f"{AREA_ID}.deployplacements",
        world_events=repository / "Data" / "Encounters" / "Valtan" / "ValtanWorldEvents.json",
        destruction_simulation=area_authoring / f"{AREA_ID}.destructionsimulation.json",
    )


def parse_args(argv: Sequence[str] | None = None) -> argparse.Namespace:
    defaults = default_paths()
    parser = argparse.ArgumentParser(
        description="Synchronize Valtan 109 outer-wall visual gap fillers"
    )
    parser.add_argument(
        "--deploy-placements", type=Path, default=defaults.deploy_placements
    )
    parser.add_argument("--world-events", type=Path, default=defaults.world_events)
    parser.add_argument(
        "--destruction-simulation", type=Path, default=defaults.destruction_simulation
    )
    parser.add_argument("--check-only", action="store_true")
    return parser.parse_args(argv)


def main(argv: Sequence[str] | None = None) -> int:
    arguments = parse_args(argv)
    paths = SyncPaths(
        deploy_placements=arguments.deploy_placements,
        world_events=arguments.world_events,
        destruction_simulation=arguments.destruction_simulation,
    )
    try:
        changed = synchronize(paths, check_only=arguments.check_only)
    except SyncError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    if arguments.check_only:
        print("PASS: Valtan 109 outer-wall gap fillers are synchronized")
    elif changed:
        print("PASS: synchronized " + ", ".join(str(path) for path in changed))
    else:
        print("PASS: Valtan 109 outer-wall gap fillers already synchronized")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
