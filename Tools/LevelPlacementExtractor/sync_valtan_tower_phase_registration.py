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
from collections import Counter
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable, Sequence

from build_maptool_scene import EDITOR_ID_MASK, IMPORTED_ID_BIT, imported_id


AREA_ID = "LV_LUT_HEARTRB_ED"
REAR_STATION_COUNT = 4
COMPONENTS_PER_STATION = 47
POSITION_EPSILON = 1.0e-6
PLACEMENT_HEADER = re.compile(
    r'^LOSTARK_MAP_PLACEMENTS\s+2\s+"(?P<area>[A-Za-z0-9_.-]+)"\s+'
    r'(?P<count>[0-9]+)$'
)
TOKEN = re.compile(r"^[A-Za-z0-9_.-]+$")


class SyncError(ValueError):
    pass


class OutOfSyncError(SyncError):
    def __init__(self, paths: Sequence[Path]):
        self.paths = tuple(paths)
        super().__init__(
            "tower phase registration is out of sync: "
            + ", ".join(str(path) for path in self.paths)
        )


@dataclass(frozen=True)
class PlacementRow:
    placement_id: int
    source_placement_id: str
    source_level: str
    transform_source: str
    asset_id: str
    position: tuple[float, float, float]
    quaternion: tuple[float, float, float, float]
    scale: tuple[float, float, float]
    visible: bool
    original_line: str


@dataclass(frozen=True)
class PlacementDocument:
    rows: tuple[PlacementRow, ...]


@dataclass(frozen=True)
class SyncPaths:
    manifest: Path
    placements: Path
    overlay_manifest: Path
    environment_runtime: Path
    maplights: Path


def exact_keys(value: Any, expected: Iterable[str], context: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise SyncError(f"{context} must be an object")
    expected_set = set(expected)
    if set(value) != expected_set:
        raise SyncError(f"{context} has unexpected properties")
    return value


def stable_token(value: Any, context: str, maximum: int = 128) -> str:
    if not isinstance(value, str) or not value or len(value) > maximum or not TOKEN.fullmatch(value):
        raise SyncError(f"{context} is not a stable token")
    return value


def stable_source_id(value: Any, context: str) -> str:
    if (
        not isinstance(value, str)
        or not value
        or len(value) > 256
        or any(ord(character) < 0x20 or character in '"\r\n' for character in value)
    ):
        raise SyncError(f"{context} is not a stable source ID")
    return value


def exact_integer(value: Any, context: str, minimum: int = 0) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < minimum:
        raise SyncError(f"{context} is not an integer >= {minimum}")
    return value


def finite_number(value: Any, context: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise SyncError(f"{context} is not numeric")
    result = float(value)
    if not math.isfinite(result):
        raise SyncError(f"{context} is not finite")
    return result


def finite_vector(value: Any, width: int, context: str) -> tuple[float, ...]:
    if not isinstance(value, list) or len(value) != width:
        raise SyncError(f"{context} must contain {width} numbers")
    return tuple(finite_number(component, f"{context}[{index}]") for index, component in enumerate(value))


def close(left: float, right: float) -> bool:
    return math.isclose(left, right, rel_tol=0.0, abs_tol=POSITION_EPSILON)


def load_json(path: Path, context: str) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise SyncError(f"{context} JSON load failed: {path}: {error}") from error
    if not isinstance(value, dict):
        raise SyncError(f"{context} root must be an object")
    return value


def validate_light_spec(value: Any, context: str) -> dict[str, Any]:
    light = exact_keys(value, ("lightId", "sourcePosition"), context)
    stable_token(light["lightId"], f"{context}.lightId")
    finite_vector(light["sourcePosition"], 3, f"{context}.sourcePosition")
    return light


def validate_source_ids(value: Any, context: str) -> list[str]:
    if not isinstance(value, list) or len(value) != COMPONENTS_PER_STATION:
        raise SyncError(f"{context} must contain exactly {COMPONENTS_PER_STATION} source IDs")
    result = [stable_source_id(item, f"{context}[{index}]") for index, item in enumerate(value)]
    if len(set(result)) != len(result):
        raise SyncError(f"{context} contains duplicate source IDs")
    return result


def load_registration_manifest(path: Path) -> dict[str, Any]:
    manifest = exact_keys(
        load_json(path, "tower registration manifest"),
        (
            "schemaVersion",
            "enabled",
            "areaId",
            "sourceLevel",
            "sourceFloor",
            "targetFloor",
            "translationY",
            "previousTranslationY",
            "overlay",
            "expectedAssetCounts",
            "expectedRearStationCount",
            "expectedComponentsPerStation",
            "rearStations",
            "controlStation",
        ),
        "tower registration manifest",
    )
    if exact_integer(manifest["schemaVersion"], "manifest.schemaVersion") != 1:
        raise SyncError("manifest schemaVersion must be 1")
    if not isinstance(manifest["enabled"], bool):
        raise SyncError("manifest.enabled must be boolean")
    if stable_token(manifest["areaId"], "manifest.areaId") != AREA_ID:
        raise SyncError(f"manifest areaId must be {AREA_ID}")
    stable_token(manifest["sourceLevel"], "manifest.sourceLevel")
    if exact_integer(
        manifest["expectedRearStationCount"], "manifest.expectedRearStationCount"
    ) != REAR_STATION_COUNT:
        raise SyncError(f"expectedRearStationCount must be {REAR_STATION_COUNT}")
    if exact_integer(
        manifest["expectedComponentsPerStation"], "manifest.expectedComponentsPerStation"
    ) != COMPONENTS_PER_STATION:
        raise SyncError(f"expectedComponentsPerStation must be {COMPONENTS_PER_STATION}")

    source_floor = exact_keys(manifest["sourceFloor"], ("placementId", "y"), "manifest.sourceFloor")
    target_floor = exact_keys(manifest["targetFloor"], ("placementId", "y"), "manifest.targetFloor")
    stable_source_id(source_floor["placementId"], "manifest.sourceFloor.placementId")
    stable_source_id(target_floor["placementId"], "manifest.targetFloor.placementId")
    source_y = finite_number(source_floor["y"], "manifest.sourceFloor.y")
    target_y = finite_number(target_floor["y"], "manifest.targetFloor.y")
    translation_y = finite_number(manifest["translationY"], "manifest.translationY")
    previous_translation_y = finite_number(
        manifest["previousTranslationY"], "manifest.previousTranslationY"
    )
    if not close(target_y - source_y, translation_y):
        raise SyncError("manifest translationY does not equal targetFloor.y - sourceFloor.y")
    if manifest["enabled"]:
        if translation_y <= 0.0 or not close(previous_translation_y, translation_y):
            raise SyncError("enabled tower registration requires a positive translationY")
    elif (
        not close(translation_y, 0.0)
        or previous_translation_y <= 0.0
        or source_floor["placementId"] != target_floor["placementId"]
    ):
        raise SyncError("disabled tower registration must preserve the source attachment transform")

    overlay = exact_keys(manifest["overlay"], ("sourceLevel", "sourcePrefix"), "manifest.overlay")
    overlay_level = stable_token(overlay["sourceLevel"], "manifest.overlay.sourceLevel")
    if overlay_level == manifest["sourceLevel"]:
        raise SyncError("overlay sourceLevel must differ from sourceLevel")
    stable_token(overlay["sourcePrefix"], "manifest.overlay.sourcePrefix")

    asset_counts = manifest["expectedAssetCounts"]
    if not isinstance(asset_counts, dict) or not asset_counts:
        raise SyncError("manifest.expectedAssetCounts must be a non-empty object")
    parsed_asset_counts: dict[str, int] = {}
    for asset_id, count in asset_counts.items():
        parsed_asset_counts[stable_token(asset_id, "expected asset ID")] = exact_integer(
            count, f"expectedAssetCounts.{asset_id}", 1
        )
    if sum(parsed_asset_counts.values()) != COMPONENTS_PER_STATION:
        raise SyncError("expectedAssetCounts must sum to 47")

    rear_stations = manifest["rearStations"]
    if not isinstance(rear_stations, list) or len(rear_stations) != REAR_STATION_COUNT:
        raise SyncError(f"manifest.rearStations must contain exactly {REAR_STATION_COUNT} stations")
    station_ids: set[str] = set()
    light_ids: set[str] = set()
    source_ids: set[str] = set()
    overlay_ids: set[int] = set()
    for index, raw_station in enumerate(rear_stations):
        context = f"manifest.rearStations[{index}]"
        station = exact_keys(
            raw_station,
            ("stationId", "overlayPlacementIdStart", "sourcePlacementIds", "light"),
            context,
        )
        station_id = stable_token(station["stationId"], f"{context}.stationId")
        if station_id in station_ids:
            raise SyncError(f"duplicate rear stationId: {station_id}")
        station_ids.add(station_id)
        start = exact_integer(station["overlayPlacementIdStart"], f"{context}.overlayPlacementIdStart", 1)
        end = start + COMPONENTS_PER_STATION - 1
        if end > EDITOR_ID_MASK:
            raise SyncError(f"{context} overlay ID range is outside the low-ID domain")
        for placement_id in range(start, end + 1):
            if placement_id in overlay_ids:
                raise SyncError(f"duplicate overlay placement ID: {placement_id}")
            overlay_ids.add(placement_id)
        for source_id in validate_source_ids(station["sourcePlacementIds"], f"{context}.sourcePlacementIds"):
            if source_id in source_ids:
                raise SyncError(f"source placement is shared between stations: {source_id}")
            source_ids.add(source_id)
        light = validate_light_spec(station["light"], f"{context}.light")
        if light["lightId"] in light_ids:
            raise SyncError(f"duplicate station lightId: {light['lightId']}")
        light_ids.add(light["lightId"])

    control = exact_keys(
        manifest["controlStation"],
        ("stationId", "sourcePlacementIds", "light"),
        "manifest.controlStation",
    )
    control_id = stable_token(control["stationId"], "manifest.controlStation.stationId")
    if control_id in station_ids:
        raise SyncError("control station duplicates a rear station")
    for source_id in validate_source_ids(
        control["sourcePlacementIds"], "manifest.controlStation.sourcePlacementIds"
    ):
        if source_id in source_ids:
            raise SyncError(f"control source placement is shared with a rear station: {source_id}")
        source_ids.add(source_id)
    control_light = validate_light_spec(control["light"], "manifest.controlStation.light")
    if control_light["lightId"] in light_ids:
        raise SyncError("control light duplicates a rear station light")
    return manifest


def parse_placement_document(path: Path, area_id: str) -> PlacementDocument:
    try:
        lines = path.read_text(encoding="utf-8-sig").splitlines()
    except (OSError, UnicodeError) as error:
        raise SyncError(f"placement document load failed: {path}: {error}") from error
    if not lines:
        raise SyncError(f"placement document is empty: {path}")
    header = PLACEMENT_HEADER.fullmatch(lines[0])
    if header is None or header.group("area") != area_id:
        raise SyncError(f"placement header does not match area {area_id}")
    expected_count = int(header.group("count"))
    if expected_count != len(lines) - 1:
        raise SyncError("placement header count does not match row count")

    rows: list[PlacementRow] = []
    placement_ids: set[int] = set()
    source_ids: set[str] = set()
    for index, line in enumerate(lines[1:], 2):
        try:
            fields = shlex.split(line, posix=True)
        except ValueError as error:
            raise SyncError(f"placement row {index} quoting is invalid") from error
        if len(fields) != 16:
            raise SyncError(f"placement row {index} has {len(fields)} fields instead of 16")
        try:
            placement_id = int(fields[0], 10)
            position = tuple(float(value) for value in fields[5:8])
            quaternion = tuple(float(value) for value in fields[8:12])
            scale = tuple(float(value) for value in fields[12:15])
        except ValueError as error:
            raise SyncError(f"placement row {index} has an invalid number") from error
        if placement_id <= 0 or placement_id > 0xFFFF_FFFF_FFFF_FFFF:
            raise SyncError(f"placement row {index} ID is outside uint64")
        source_id = stable_source_id(fields[1], f"placement row {index} source ID")
        source_level = stable_token(fields[2], f"placement row {index} source level")
        transform_source = stable_token(fields[3], f"placement row {index} transform source", 32)
        asset_id = stable_token(fields[4], f"placement row {index} asset ID")
        if not all(math.isfinite(value) for value in (*position, *quaternion, *scale)):
            raise SyncError(f"placement row {index} has a non-finite transform")
        if math.sqrt(sum(value * value for value in quaternion)) < 1.0e-6:
            raise SyncError(f"placement row {index} has a zero quaternion")
        if any(abs(value) < 1.0e-6 for value in scale):
            raise SyncError(f"placement row {index} has a zero scale axis")
        imported = transform_source in ("actor", "component")
        authored = transform_source in ("editor", "legacy", "overlay")
        if not (
            (imported and placement_id & IMPORTED_ID_BIT)
            or (authored and placement_id <= EDITOR_ID_MASK)
        ):
            raise SyncError(f"placement row {index} ID domain does not match transform source")
        if fields[15] not in ("0", "1"):
            raise SyncError(f"placement row {index} visibility is invalid")
        if placement_id in placement_ids or source_id in source_ids:
            raise SyncError(f"placement row {index} identity is duplicated")
        placement_ids.add(placement_id)
        source_ids.add(source_id)
        rows.append(
            PlacementRow(
                placement_id=placement_id,
                source_placement_id=source_id,
                source_level=source_level,
                transform_source=transform_source,
                asset_id=asset_id,
                position=position,  # type: ignore[arg-type]
                quaternion=quaternion,  # type: ignore[arg-type]
                scale=scale,  # type: ignore[arg-type]
                visible=fields[15] == "1",
                original_line=line,
            )
        )
    return PlacementDocument(tuple(rows))


def json_text(value: dict[str, Any]) -> str:
    return json.dumps(value, ensure_ascii=False, indent=2) + "\n"


def quoted(value: str) -> str:
    return json.dumps(value, ensure_ascii=False)


def number_text(value: float) -> str:
    return format(value, ".9g")


def render_overlay_row(row: PlacementRow, placement_id: int, source_id: str, source_level: str, delta_y: float) -> str:
    values = (
        row.position[0],
        row.position[1] + delta_y,
        row.position[2],
        *row.quaternion,
        *row.scale,
    )
    return (
        f"{placement_id} {quoted(source_id)} {quoted(source_level)} "
        f'{quoted("overlay")} {quoted(row.asset_id)} '
        + " ".join(number_text(value) for value in values)
        + " 1"
    )


def hidden_source_line(row: PlacementRow) -> str:
    head, separator, visible = row.original_line.rpartition(" ")
    if not separator or visible not in ("0", "1"):
        raise SyncError(f"cannot update source visibility: {row.source_placement_id}")
    return head + " 0"


def visible_source_line(row: PlacementRow) -> str:
    head, separator, visible = row.original_line.rpartition(" ")
    if not separator or visible not in ("0", "1"):
        raise SyncError(f"cannot update source visibility: {row.source_placement_id}")
    return head + " 1"


def generated_source_id(prefix: str, station_id: str, source_id: str) -> str:
    return stable_source_id(
        f"{prefix}:{station_id}:{source_id}",
        f"generated overlay source ID for {source_id}",
    )


def build_registered_rows(
    manifest: dict[str, Any], document: PlacementDocument
) -> tuple[list[dict[str, Any]], set[str], set[str]]:
    by_source = {row.source_placement_id: row for row in document.rows}
    source_level = manifest["sourceLevel"]
    overlay_level = manifest["overlay"]["sourceLevel"]
    prefix = manifest["overlay"]["sourcePrefix"]
    expected_assets = Counter(manifest["expectedAssetCounts"])
    rear_source_ids: set[str] = set()
    control_source_ids: set[str] = set(manifest["controlStation"]["sourcePlacementIds"])
    generated: list[dict[str, Any]] = []

    def validate_station_sources(station_id: str, source_ids: Sequence[str], require_visible: bool) -> list[PlacementRow]:
        selected: list[PlacementRow] = []
        for source_id in source_ids:
            row = by_source.get(source_id)
            if row is None:
                raise SyncError(f"station {station_id} source placement is missing: {source_id}")
            if row.source_level != source_level or row.transform_source != "component":
                raise SyncError(f"station {station_id} source is not an exact component: {source_id}")
            expected_id = imported_id(source_id)
            if row.placement_id != expected_id or not row.placement_id & IMPORTED_ID_BIT:
                raise SyncError(f"station {station_id} stable imported ID mismatch: {source_id}")
            if require_visible and not row.visible:
                raise SyncError(f"control station source must remain visible: {source_id}")
            selected.append(row)
        actual_assets = Counter(row.asset_id for row in selected)
        if actual_assets != expected_assets:
            raise SyncError(f"station {station_id} asset distribution mismatch: {dict(actual_assets)}")
        return selected

    for station in manifest["rearStations"]:
        station_id = station["stationId"]
        selected = validate_station_sources(station_id, station["sourcePlacementIds"], False)
        start = station["overlayPlacementIdStart"]
        for offset, row in enumerate(selected):
            rear_source_ids.add(row.source_placement_id)
            if not manifest["enabled"]:
                continue
            overlay_source = generated_source_id(prefix, station_id, row.source_placement_id)
            generated.append(
                {
                    "placementId": start + offset,
                    "sourcePlacementId": overlay_source,
                    "sourceLevel": overlay_level,
                    "transformSource": "overlay",
                    "assetId": row.asset_id,
                    "position": [
                        row.position[0],
                        row.position[1] + manifest["translationY"],
                        row.position[2],
                    ],
                    "quaternion": list(row.quaternion),
                    "scale": list(row.scale),
                    "visible": True,
                    "sourceRow": row,
                }
            )
    validate_station_sources(
        manifest["controlStation"]["stationId"],
        manifest["controlStation"]["sourcePlacementIds"],
        True,
    )
    if len(rear_source_ids) != REAR_STATION_COUNT * COMPONENTS_PER_STATION:
        raise SyncError("rear station source set is not exactly 188 unique placements")
    return generated, rear_source_ids, control_source_ids


def build_placement_output(
    manifest: dict[str, Any], document: PlacementDocument, generated: Sequence[dict[str, Any]], rear_source_ids: set[str]
) -> str:
    overlay_level = manifest["overlay"]["sourceLevel"]
    prefix_with_separator = manifest["overlay"]["sourcePrefix"] + ":"
    retained: list[PlacementRow] = []
    retained_ids: set[int] = set()
    retained_sources: set[str] = set()
    for row in document.rows:
        has_prefix = row.source_placement_id.startswith(prefix_with_separator)
        has_level = row.source_level == overlay_level
        if has_prefix != has_level:
            raise SyncError(f"ambiguous generated placement identity: {row.source_placement_id}")
        if has_prefix:
            if row.transform_source != "overlay":
                raise SyncError(f"generated placement is not overlay: {row.source_placement_id}")
            continue
        retained.append(row)
        retained_ids.add(row.placement_id)
        retained_sources.add(row.source_placement_id)

    generated_ids: set[int] = set()
    generated_sources: set[str] = set()
    for entry in generated:
        placement_id = entry["placementId"]
        source_id = entry["sourcePlacementId"]
        if placement_id in retained_ids or placement_id in generated_ids:
            raise SyncError(f"generated overlay placement ID collision: {placement_id}")
        if source_id in retained_sources or source_id in generated_sources:
            raise SyncError(f"generated overlay source ID collision: {source_id}")
        generated_ids.add(placement_id)
        generated_sources.add(source_id)

    output_lines: list[str] = []
    for row in retained:
        if row.source_placement_id in rear_source_ids:
            output_lines.append(
                hidden_source_line(row) if manifest["enabled"] else visible_source_line(row)
            )
        else:
            output_lines.append(row.original_line)
    for entry in generated:
        output_lines.append(
            render_overlay_row(
                entry["sourceRow"],
                entry["placementId"],
                entry["sourcePlacementId"],
                entry["sourceLevel"],
                manifest["translationY"],
            )
        )
    return (
        f'LOSTARK_MAP_PLACEMENTS 2 {quoted(manifest["areaId"])} {len(output_lines)}\n'
        + "\n".join(output_lines)
        + "\n"
    )


def validate_overlay_placement(value: Any, context: str) -> dict[str, Any]:
    placement = exact_keys(
        value,
        (
            "placementId",
            "sourcePlacementId",
            "sourceLevel",
            "transformSource",
            "assetId",
            "position",
            "quaternion",
            "scale",
            "visible",
        ),
        context,
    )
    placement_id = exact_integer(placement["placementId"], f"{context}.placementId", 1)
    if placement_id > EDITOR_ID_MASK:
        raise SyncError(f"{context}.placementId is outside the low-ID domain")
    stable_source_id(placement["sourcePlacementId"], f"{context}.sourcePlacementId")
    stable_token(placement["sourceLevel"], f"{context}.sourceLevel")
    if stable_token(placement["transformSource"], f"{context}.transformSource", 32) != "overlay":
        raise SyncError(f"{context}.transformSource must be overlay")
    stable_token(placement["assetId"], f"{context}.assetId")
    finite_vector(placement["position"], 3, f"{context}.position")
    quaternion = finite_vector(placement["quaternion"], 4, f"{context}.quaternion")
    scale = finite_vector(placement["scale"], 3, f"{context}.scale")
    if math.sqrt(sum(value * value for value in quaternion)) < 1.0e-6:
        raise SyncError(f"{context}.quaternion is zero")
    if any(abs(value) < 1.0e-6 for value in scale):
        raise SyncError(f"{context}.scale has a zero axis")
    if not isinstance(placement["visible"], bool):
        raise SyncError(f"{context}.visible must be boolean")
    return placement


def build_overlay_manifest_output(path: Path, manifest: dict[str, Any], generated: Sequence[dict[str, Any]]) -> str:
    document = exact_keys(
        load_json(path, "overlay manifest"),
        ("schemaVersion", "areaId", "status", "basis", "assets", "placements"),
        "overlay manifest",
    )
    if document["schemaVersion"] != 1 or document["areaId"] != manifest["areaId"]:
        raise SyncError("overlay manifest schema or area does not match")
    if not isinstance(document["assets"], list) or not isinstance(document["placements"], list):
        raise SyncError("overlay manifest assets and placements must be arrays")
    prefix_with_separator = manifest["overlay"]["sourcePrefix"] + ":"
    overlay_level = manifest["overlay"]["sourceLevel"]
    retained: list[dict[str, Any]] = []
    retained_ids: set[int] = set()
    retained_sources: set[str] = set()
    for index, raw in enumerate(document["placements"]):
        placement = validate_overlay_placement(raw, f"overlay placements[{index}]")
        has_prefix = placement["sourcePlacementId"].startswith(prefix_with_separator)
        has_level = placement["sourceLevel"] == overlay_level
        if has_prefix != has_level:
            raise SyncError(f"ambiguous generated overlay manifest row: {placement['sourcePlacementId']}")
        if has_prefix:
            continue
        if placement["placementId"] in retained_ids or placement["sourcePlacementId"] in retained_sources:
            raise SyncError("overlay manifest retained identity is duplicated")
        retained_ids.add(placement["placementId"])
        retained_sources.add(placement["sourcePlacementId"])
        retained.append(placement)

    generated_output: list[dict[str, Any]] = []
    for entry in generated:
        if entry["placementId"] in retained_ids or entry["sourcePlacementId"] in retained_sources:
            raise SyncError(f"generated overlay manifest identity collides: {entry['sourcePlacementId']}")
        retained_ids.add(entry["placementId"])
        retained_sources.add(entry["sourcePlacementId"])
        generated_output.append(
            {key: copy.deepcopy(entry[key]) for key in (
                "placementId",
                "sourcePlacementId",
                "sourceLevel",
                "transformSource",
                "assetId",
                "position",
                "quaternion",
                "scale",
                "visible",
            )}
        )
    output = copy.deepcopy(document)
    output["placements"] = retained + generated_output
    return json_text(output)


def build_environment_output(
    path: Path, manifest: dict[str, Any], rear_source_ids: set[str], control_source_ids: set[str]
) -> str:
    document = exact_keys(
        load_json(path, "environment runtime manifest"),
        ("schemaVersion", "areaId", "evidence", "profiles", "visibilityOverrides"),
        "environment runtime manifest",
    )
    if document["schemaVersion"] != 1 or document["areaId"] != manifest["areaId"]:
        raise SyncError("environment runtime schema or area does not match")
    if not isinstance(document["profiles"], list) or not isinstance(document["visibilityOverrides"], list):
        raise SyncError("environment profiles and visibilityOverrides must be arrays")
    retained: list[dict[str, Any]] = []
    seen: set[str] = set()
    for index, raw in enumerate(document["visibilityOverrides"]):
        row = exact_keys(raw, ("sourcePlacementId", "visible"), f"visibilityOverrides[{index}]")
        source_id = stable_source_id(row["sourcePlacementId"], f"visibilityOverrides[{index}].sourcePlacementId")
        if not isinstance(row["visible"], bool) or source_id in seen:
            raise SyncError("environment visibility override is invalid or duplicated")
        seen.add(source_id)
        if source_id in control_source_ids:
            raise SyncError(f"control station visibility override is forbidden: {source_id}")
        if source_id not in rear_source_ids:
            retained.append(copy.deepcopy(row))
    output = copy.deepcopy(document)
    output["visibilityOverrides"] = retained
    if manifest["enabled"]:
        output["visibilityOverrides"] += [
            {"sourcePlacementId": source_id, "visible": False}
            for station in manifest["rearStations"]
            for source_id in station["sourcePlacementIds"]
        ]
    return json_text(output)


def validate_map_light(value: Any, context: str) -> dict[str, Any]:
    light = exact_keys(
        value,
        (
            "lightId",
            "sourceLevel",
            "sourceObjectId",
            "position",
            "radiusMeters",
            "falloffExponent",
            "color",
            "brightness",
        ),
        context,
    )
    stable_token(light["lightId"], f"{context}.lightId")
    stable_token(light["sourceLevel"], f"{context}.sourceLevel")
    stable_source_id(light["sourceObjectId"], f"{context}.sourceObjectId")
    finite_vector(light["position"], 3, f"{context}.position")
    color = finite_vector(light["color"], 4, f"{context}.color")
    if any(value < 0.0 or value > 1.0 for value in color):
        raise SyncError(f"{context}.color is outside 0..1")
    radius = finite_number(light["radiusMeters"], f"{context}.radiusMeters")
    falloff = finite_number(light["falloffExponent"], f"{context}.falloffExponent")
    brightness = finite_number(light["brightness"], f"{context}.brightness")
    if not 0.01 <= radius <= 1000.0 or not 0.01 <= falloff <= 64.0 or not 0.0 <= brightness <= 64.0:
        raise SyncError(f"{context} scalar is outside its allowed range")
    return light


def vector_close(left: Sequence[float], right: Sequence[float]) -> bool:
    return len(left) == len(right) and all(close(float(a), float(b)) for a, b in zip(left, right))


def build_maplights_output(path: Path, manifest: dict[str, Any]) -> str:
    document = exact_keys(
        load_json(path, "maplights"),
        ("schema", "formatVersion", "areaId", "provenance", "lights"),
        "maplights",
    )
    if (
        document["schema"] != "lostark.map-light-presentation"
        or document["formatVersion"] != 1
        or document["areaId"] != manifest["areaId"]
        or document["provenance"] not in (
            "SOURCE_EXACT",
            "SOURCE_INSTANCE_EXACT_FALLOFF_INFERRED",
            "PROJECT_AUTHORED",
        )
        or not isinstance(document["lights"], list)
        or not 1 <= len(document["lights"]) <= 64
    ):
        raise SyncError("maplights header is invalid")
    lights: list[dict[str, Any]] = []
    by_id: dict[str, dict[str, Any]] = {}
    source_object_ids: set[str] = set()
    for index, raw in enumerate(document["lights"]):
        light = validate_map_light(raw, f"maplights.lights[{index}]")
        if light["lightId"] in by_id or light["sourceObjectId"] in source_object_ids:
            raise SyncError("maplights identity is duplicated")
        copied = copy.deepcopy(light)
        lights.append(copied)
        by_id[copied["lightId"]] = copied
        source_object_ids.add(copied["sourceObjectId"])

    translation_y = manifest["translationY"]
    previous_translation_y = manifest["previousTranslationY"]
    for station in manifest["rearStations"]:
        spec = station["light"]
        light = by_id.get(spec["lightId"])
        if light is None:
            raise SyncError(f"rear station light is missing: {spec['lightId']}")
        source_position = tuple(float(value) for value in spec["sourcePosition"])
        translated = (source_position[0], source_position[1] + translation_y, source_position[2])
        previous_translated = (
            source_position[0],
            source_position[1] + previous_translation_y,
            source_position[2],
        )
        current = tuple(float(value) for value in light["position"])
        if (
            not vector_close(current, source_position)
            and not vector_close(current, translated)
            and not (
                not manifest["enabled"]
                and vector_close(current, previous_translated)
            )
        ):
            raise SyncError(f"rear station light position drifted: {spec['lightId']}")
        light["position"] = list(translated)

    control_spec = manifest["controlStation"]["light"]
    control_light = by_id.get(control_spec["lightId"])
    if control_light is None:
        raise SyncError(f"control station light is missing: {control_spec['lightId']}")
    if not vector_close(control_light["position"], control_spec["sourcePosition"]):
        raise SyncError("control station light position must remain source-exact")

    output = copy.deepcopy(document)
    output["provenance"] = (
        "PROJECT_AUTHORED"
        if manifest["enabled"]
        else "SOURCE_INSTANCE_EXACT_FALLOFF_INFERRED"
    )
    output["lights"] = lights
    return json_text(output)


def build_outputs(paths: SyncPaths) -> dict[Path, str]:
    manifest = load_registration_manifest(paths.manifest)
    placement_document = parse_placement_document(paths.placements, manifest["areaId"])
    source_floor = next(
        (row for row in placement_document.rows if row.source_placement_id == manifest["sourceFloor"]["placementId"]),
        None,
    )
    target_floor = next(
        (row for row in placement_document.rows if row.source_placement_id == manifest["targetFloor"]["placementId"]),
        None,
    )
    if source_floor is None or target_floor is None:
        raise SyncError("source or target floor placement is missing")
    if not close(source_floor.position[1], manifest["sourceFloor"]["y"]):
        raise SyncError("source floor Y does not match manifest")
    if not close(target_floor.position[1], manifest["targetFloor"]["y"]):
        raise SyncError("target floor Y does not match manifest")

    generated, rear_source_ids, control_source_ids = build_registered_rows(manifest, placement_document)
    return {
        paths.placements: build_placement_output(manifest, placement_document, generated, rear_source_ids),
        paths.overlay_manifest: build_overlay_manifest_output(paths.overlay_manifest, manifest, generated),
        paths.environment_runtime: build_environment_output(
            paths.environment_runtime, manifest, rear_source_ids, control_source_ids
        ),
        paths.maplights: build_maplights_output(paths.maplights, manifest),
    }


def stage_text(path: Path, text: str, suffix: str) -> Path:
    handle, temporary_name = tempfile.mkstemp(prefix=path.name + ".", suffix=suffix, dir=path.parent)
    temporary = Path(temporary_name)
    try:
        with os.fdopen(handle, "w", encoding="utf-8", newline="\n") as stream:
            stream.write(text)
            stream.flush()
            os.fsync(stream.fileno())
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise
    return temporary


def commit_outputs(
    outputs: dict[Path, str], *, failure_after_promote: int | None = None
) -> tuple[Path, ...]:
    changed = tuple(
        path
        for path, text in outputs.items()
        if path.read_text(encoding="utf-8-sig") != text
    )
    if not changed:
        return ()
    staged: dict[Path, Path] = {}
    backups: dict[Path, Path] = {}
    promoted = 0
    try:
        for path in changed:
            staged[path] = stage_text(path, outputs[path], ".stage")
        for path in changed:
            backup = stage_text(path, "", ".backup")
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
                f"tower registration commit failed ({error}); rollback failed: "
                + "; ".join(rollback_errors)
            ) from error
        raise SyncError(f"tower registration commit failed and rolled back: {error}") from error
    finally:
        for temporary in staged.values():
            temporary.unlink(missing_ok=True)


def synchronize(
    paths: SyncPaths,
    *,
    check_only: bool = False,
    failure_after_promote: int | None = None,
) -> tuple[Path, ...]:
    unique_paths = {paths.placements, paths.overlay_manifest, paths.environment_runtime, paths.maplights}
    if len(unique_paths) != 4 or paths.manifest in unique_paths:
        raise SyncError("manifest and four synchronized output paths must be distinct")
    for path in (paths.manifest, *unique_paths):
        if not path.is_file():
            raise SyncError(f"required input file is missing: {path}")
    outputs = build_outputs(paths)
    stale = tuple(
        path for path, text in outputs.items() if path.read_text(encoding="utf-8-sig") != text
    )
    if check_only:
        if stale:
            raise OutOfSyncError(stale)
        return ()
    return commit_outputs(outputs, failure_after_promote=failure_after_promote)


def default_paths() -> SyncPaths:
    script_dir = Path(__file__).resolve().parent
    repository = script_dir.parents[1]
    area_authoring = repository / "Data/Maps/Authoring" / AREA_ID
    return SyncPaths(
        manifest=script_dir / "heartrb_valtan_tower_phase_registration.json",
        placements=area_authoring / f"{AREA_ID}.mapplacements",
        overlay_manifest=script_dir / "heartrb_valtan_core_overlay.json",
        environment_runtime=script_dir / "heartrb_environment_runtime.json",
        maplights=area_authoring / f"{AREA_ID}.maplights.json",
    )


def parse_args(argv: Sequence[str] | None = None) -> argparse.Namespace:
    defaults = default_paths()
    parser = argparse.ArgumentParser(description="Synchronize Valtan rear tower phase registration")
    parser.add_argument("--manifest", type=Path, default=defaults.manifest)
    parser.add_argument("--placements", type=Path, default=defaults.placements)
    parser.add_argument("--overlay-manifest", type=Path, default=defaults.overlay_manifest)
    parser.add_argument("--environment-runtime", type=Path, default=defaults.environment_runtime)
    parser.add_argument("--maplights", type=Path, default=defaults.maplights)
    parser.add_argument("--check-only", action="store_true")
    return parser.parse_args(argv)


def main(argv: Sequence[str] | None = None) -> int:
    arguments = parse_args(argv)
    paths = SyncPaths(
        manifest=arguments.manifest,
        placements=arguments.placements,
        overlay_manifest=arguments.overlay_manifest,
        environment_runtime=arguments.environment_runtime,
        maplights=arguments.maplights,
    )
    try:
        changed = synchronize(paths, check_only=arguments.check_only)
    except SyncError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    if arguments.check_only:
        print("PASS: Valtan tower phase registration is synchronized")
    elif changed:
        print("PASS: synchronized " + ", ".join(str(path) for path in changed))
    else:
        print("PASS: Valtan tower phase registration already synchronized")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
