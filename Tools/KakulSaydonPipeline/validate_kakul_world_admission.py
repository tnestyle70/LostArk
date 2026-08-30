#!/usr/bin/env python3
"""Fail-closed admission report for the KoukuSaton raid Area.

The display collection name is ``KoukuSaton``.  It never replaces the stable
source/Area identity ``LV_LUT_MIDNIGHTC_ED``.  This validator is deliberately
read-only: it does not copy extracted resources, invent navigation, publish a
world, or add a Client/Server Level.

The default CLI request is ``server-product-level`` and therefore exits 1
until the complete Product closure exists.  ``--report-only`` is available to
inspect the lower admitted modes without turning a partial result into a build
admission.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass, field
import hashlib
import json
import math
from pathlib import Path, PurePosixPath, PureWindowsPath
import re
import shlex
import struct
import sys
from typing import Any, Iterable


REPO_ROOT = Path(__file__).resolve().parents[2]
AREA_ID = "LV_LUT_MIDNIGHTC_ED"
COLLECTION_NAME = "KoukuSaton"
WORLD_ID = "KAKULSAYDON_ARENA"
CLIENT_LEVEL = "KAKULSAYDON_ARENA"

RESOURCE_MODE = "resource-collection"
GEOMETRY_MODE = "development-geometry-preview"
PRODUCT_MODE = "server-product-level"
MODE_ORDER = (RESOURCE_MODE, GEOMETRY_MODE, PRODUCT_MODE)
MODE_LABELS = {
    RESOURCE_MODE: "Resource Collection",
    GEOMETRY_MODE: "Development Geometry Preview",
    PRODUCT_MODE: "Server Product Level",
}

INTAKE_PATH = Path(
    f"Data/ResourceIntake/{AREA_ID}.resource-intake.json"
)
MAP_CATALOG_PATH = Path("Data/Maps/MapCatalog.json")
SOURCE_CATALOG_PATH = Path(
    f"Data/Maps/Imported/{AREA_ID}/{AREA_ID}.mapassets"
)
SOURCE_PLACEMENTS_PATH = Path(
    f"Data/Maps/Authoring/{AREA_ID}/{AREA_ID}.mapplacements"
)
RUNTIME_CATALOG_PATH = Path(f"Client/Bin/DataFiles/Map/{AREA_ID}.mapassets")
RUNTIME_PLACEMENTS_PATH = Path(
    f"Client/Bin/DataFiles/Map/{AREA_ID}.mapplacements"
)
GAMEPLAY_PATH = Path(f"Data/Worlds/{AREA_ID}/Gameplay.world.json")

STABLE_ID = re.compile(r"^[A-Za-z0-9_.-]{1,128}$")


@dataclass(frozen=True)
class Finding:
    code: str
    path: str
    detail: str

    def as_json(self) -> dict[str, str]:
        return {"code": self.code, "path": self.path, "detail": self.detail}


@dataclass
class ModeResult:
    mode: str
    own_findings: list[Finding] = field(default_factory=list)
    permitted: bool = False
    facts: dict[str, Any] = field(default_factory=dict)

    def as_json(self) -> dict[str, Any]:
        return {
            "mode": self.mode,
            "label": MODE_LABELS[self.mode],
            "permitted": self.permitted,
            "requires": list(MODE_ORDER[: MODE_ORDER.index(self.mode)]),
            "findings": [finding.as_json() for finding in self.own_findings],
            "facts": self.facts,
        }


@dataclass
class AdmissionReport:
    root: Path
    modes: list[ModeResult]

    def result(self, mode: str) -> ModeResult:
        return next(result for result in self.modes if result.mode == mode)

    @property
    def highest_permitted_mode(self) -> str | None:
        highest: str | None = None
        for result in self.modes:
            if result.permitted:
                highest = result.mode
        return highest

    def as_json(self, requested_mode: str) -> dict[str, Any]:
        highest = self.highest_permitted_mode
        return {
            "schema": "lostark.kakul-world-admission-report",
            "formatVersion": 1,
            "repositoryRoot": str(self.root),
            "collectionName": COLLECTION_NAME,
            "canonicalAreaId": AREA_ID,
            "serverWorldId": WORLD_ID,
            "requestedMode": requested_mode,
            "requestedModePermitted": self.result(requested_mode).permitted,
            "highestPermittedMode": highest,
            "highestPermittedLabel": MODE_LABELS[highest] if highest else "NONE",
            "spawnContract": {
                "arbitraryRandomWorldPosition": "forbidden",
                "selectionInput": "stable playerSpawn placement IDs only",
                "navigationAdmission": (
                    "each enabled slot must resolve to a walkable Server navgrid "
                    "cell with height delta <= 0.25"
                ),
            },
            "modes": [mode.as_json() for mode in self.modes],
        }


class _Stage:
    def __init__(self, mode: str) -> None:
        self.result = ModeResult(mode=mode)

    def issue(self, code: str, path: Path | str, detail: str) -> None:
        normalized = path.as_posix() if isinstance(path, Path) else str(path)
        self.result.own_findings.append(Finding(code, normalized, detail))

    def require(
        self,
        condition: bool,
        code: str,
        path: Path | str,
        detail: str,
    ) -> bool:
        if not condition:
            self.issue(code, path, detail)
            return False
        return True


def _read_text(root: Path, relative: Path, stage: _Stage) -> str | None:
    path = root / relative
    if not path.is_file():
        stage.issue("file.missing", relative, "required file does not exist")
        return None
    try:
        return path.read_text(encoding="utf-8-sig")
    except (OSError, UnicodeError) as error:
        stage.issue("file.unreadable", relative, str(error))
        return None


def _read_json(root: Path, relative: Path, stage: _Stage) -> Any | None:
    text = _read_text(root, relative, stage)
    if text is None:
        return None
    try:
        return json.loads(text)
    except json.JSONDecodeError as error:
        stage.issue(
            "json.invalid",
            relative,
            f"line={error.lineno} column={error.colno}: {error.msg}",
        )
        return None


def _has_file(path: Path) -> bool:
    if not path.is_dir():
        return False
    try:
        return next((entry for entry in path.rglob("*") if entry.is_file()), None) is not None
    except OSError:
        return False


def _safe_resource_id(value: str) -> bool:
    if not value or "\\" in value:
        return False
    posix = PurePosixPath(value)
    windows = PureWindowsPath(value)
    return (
        not posix.is_absolute()
        and not windows.is_absolute()
        and not windows.drive
        and ".." not in posix.parts
        and "." not in posix.parts
    )


def _load_map_catalog(
    root: Path, stage: _Stage
) -> tuple[dict[str, Any] | None, dict[str, Any] | None]:
    document = _read_json(root, MAP_CATALOG_PATH, stage)
    if not isinstance(document, dict):
        if document is not None:
            stage.issue("map-catalog.root.invalid", MAP_CATALOG_PATH, "expected object")
        return None, None
    if document.get("schema") != "lostark.map-catalog" or document.get(
        "formatVersion"
    ) != 1:
        stage.issue(
            "map-catalog.header.invalid",
            MAP_CATALOG_PATH,
            "expected schema=lostark.map-catalog formatVersion=1",
        )
    areas = document.get("areas")
    if not isinstance(areas, list):
        stage.issue("map-catalog.areas.invalid", MAP_CATALOG_PATH, "areas must be an array")
        return document, None
    matches = [area for area in areas if isinstance(area, dict) and area.get("id") == AREA_ID]
    if len(matches) != 1:
        stage.issue(
            "map-catalog.area.cardinality",
            MAP_CATALOG_PATH,
            f"expected exactly one {AREA_ID} row; got {len(matches)}",
        )
        return document, None
    return document, matches[0]


def _expect_catalog_field(
    stage: _Stage, area: dict[str, Any] | None, key: str, expected: str
) -> None:
    if area is None:
        return
    actual = area.get(key)
    stage.require(
        actual == expected,
        f"map-catalog.{key}.invalid",
        MAP_CATALOG_PATH,
        f"expected {key}={expected!r}; got {actual!r}",
    )


def _parse_map_assets(
    root: Path, relative: Path, stage: _Stage
) -> dict[str, str] | None:
    text = _read_text(root, relative, stage)
    if text is None:
        return None
    lines = [line for line in text.splitlines() if line.strip()]
    if not lines:
        stage.issue("map-assets.empty", relative, "catalog is empty")
        return None
    try:
        header = shlex.split(lines[0], posix=True)
    except ValueError as error:
        stage.issue("map-assets.header.invalid", relative, str(error))
        return None
    if (
        len(header) != 4
        or header[0] != "LOSTARK_MAP_ASSET_CATALOG"
        or header[2] != AREA_ID
    ):
        stage.issue(
            "map-assets.header.identity",
            relative,
            f"expected LOSTARK_MAP_ASSET_CATALOG <version> {AREA_ID} <count>",
        )
        return None
    try:
        declared_count = int(header[3])
    except ValueError:
        stage.issue("map-assets.header.count", relative, f"invalid count {header[3]!r}")
        return None
    assets: dict[str, str] = {}
    for line_number, line in enumerate(lines[1:], start=2):
        try:
            tokens = shlex.split(line, posix=True)
        except ValueError as error:
            stage.issue("map-assets.row.syntax", relative, f"line {line_number}: {error}")
            continue
        if len(tokens) < 3:
            stage.issue(
                "map-assets.row.fields",
                relative,
                f"line {line_number}: expected at least asset/name/resource",
            )
            continue
        asset_id, resource_id = tokens[0], tokens[2]
        if asset_id in assets:
            stage.issue(
                "map-assets.asset-id.duplicate",
                relative,
                f"line {line_number}: {asset_id}",
            )
            continue
        if not STABLE_ID.fullmatch(asset_id):
            stage.issue(
                "map-assets.asset-id.invalid",
                relative,
                f"line {line_number}: {asset_id!r}",
            )
        if not _safe_resource_id(resource_id) or not resource_id.startswith(
            f"Map/{AREA_ID}/"
        ):
            stage.issue(
                "map-assets.resource-id.invalid",
                relative,
                f"line {line_number}: {resource_id!r}",
            )
        assets[asset_id] = resource_id
    if len(lines) - 1 != declared_count:
        stage.issue(
            "map-assets.count.mismatch",
            relative,
            f"header={declared_count} rows={len(lines) - 1}",
        )
    return assets


def _parse_map_placements(
    root: Path,
    relative: Path,
    assets: dict[str, str] | None,
    stage: _Stage,
) -> int | None:
    text = _read_text(root, relative, stage)
    if text is None:
        return None
    lines = [line for line in text.splitlines() if line.strip()]
    if not lines:
        stage.issue("map-placements.empty", relative, "placements are empty")
        return None
    try:
        header = shlex.split(lines[0], posix=True)
    except ValueError as error:
        stage.issue("map-placements.header.invalid", relative, str(error))
        return None
    if (
        len(header) != 4
        or header[0] != "LOSTARK_MAP_PLACEMENTS"
        or header[2] != AREA_ID
    ):
        stage.issue(
            "map-placements.header.identity",
            relative,
            f"expected LOSTARK_MAP_PLACEMENTS <version> {AREA_ID} <count>",
        )
        return None
    try:
        declared_count = int(header[3])
    except ValueError:
        stage.issue("map-placements.header.count", relative, f"invalid count {header[3]!r}")
        return None
    seen: set[str] = set()
    for line_number, line in enumerate(lines[1:], start=2):
        try:
            tokens = shlex.split(line, posix=True)
        except ValueError as error:
            stage.issue("map-placements.row.syntax", relative, f"line {line_number}: {error}")
            continue
        if len(tokens) < 16:
            stage.issue(
                "map-placements.row.fields",
                relative,
                f"line {line_number}: expected 16 fields; got {len(tokens)}",
            )
            continue
        placement_id, asset_id = tokens[0], tokens[4]
        if placement_id in seen:
            stage.issue(
                "map-placements.placement-id.duplicate",
                relative,
                f"line {line_number}: {placement_id}",
            )
        seen.add(placement_id)
        if not placement_id.isdigit() or placement_id == "0":
            stage.issue(
                "map-placements.placement-id.invalid",
                relative,
                f"line {line_number}: {placement_id!r}",
            )
        if assets is not None and asset_id not in assets:
            stage.issue(
                "map-placements.asset.unresolved",
                relative,
                f"line {line_number}: {asset_id}",
            )
        try:
            values = [float(token) for token in tokens[5:15]]
        except ValueError:
            values = []
        if len(values) != 10 or not all(math.isfinite(value) for value in values):
            stage.issue(
                "map-placements.transform.invalid",
                relative,
                f"line {line_number}: position/quaternion/scale must be finite",
            )
    if len(lines) - 1 != declared_count:
        stage.issue(
            "map-placements.count.mismatch",
            relative,
            f"header={declared_count} rows={len(lines) - 1}",
        )
    return len(lines) - 1


def _hash(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _require_same_bytes(
    root: Path, source: Path, runtime: Path, stage: _Stage, code: str
) -> None:
    source_path, runtime_path = root / source, root / runtime
    if not source_path.is_file() or not runtime_path.is_file():
        if not runtime_path.is_file():
            stage.issue("file.missing", runtime, "required published file does not exist")
        return
    try:
        source_hash, runtime_hash = _hash(source_path), _hash(runtime_path)
    except OSError as error:
        stage.issue(code, runtime, str(error))
        return
    if source_hash != runtime_hash:
        stage.issue(
            code,
            runtime,
            f"source sha256={source_hash} runtime sha256={runtime_hash}",
        )


def _random_contract_violations(value: Any, location: str = "$.") -> Iterable[str]:
    if isinstance(value, dict):
        for key, child in value.items():
            child_location = f"{location}{key}"
            if "random" in str(key).casefold():
                yield child_location
            yield from _random_contract_violations(child, child_location + ".")
    elif isinstance(value, list):
        for index, child in enumerate(value):
            yield from _random_contract_violations(child, f"{location}[{index}].")
    elif isinstance(value, str) and "random" in value.casefold():
        yield location[:-1]


def _parse_world(
    root: Path, stage: _Stage
) -> tuple[dict[str, Any] | None, list[dict[str, Any]]]:
    document = _read_json(root, GAMEPLAY_PATH, stage)
    if not isinstance(document, dict):
        if document is not None:
            stage.issue("world.root.invalid", GAMEPLAY_PATH, "expected object")
        return None, []
    if (
        document.get("schema") != "lostark.world-gameplay"
        or document.get("formatVersion") != 6
        or document.get("areaId") != AREA_ID
    ):
        stage.issue(
            "world.header.invalid",
            GAMEPLAY_PATH,
            f"expected lostark.world-gameplay v6 areaId={AREA_ID}",
        )
    for location in _random_contract_violations(document):
        stage.issue(
            "world.random-spawn.forbidden",
            GAMEPLAY_PATH,
            f"arbitrary random contract at {location}; use stable nav-valid slots",
        )
    placements = document.get("placements")
    if not isinstance(placements, list):
        stage.issue("world.placements.invalid", GAMEPLAY_PATH, "placements must be an array")
        return document, []
    enabled_spawns = [
        placement
        for placement in placements
        if isinstance(placement, dict)
        and placement.get("kind") == "playerSpawn"
        and placement.get("enabled") is True
    ]
    if not enabled_spawns:
        stage.issue(
            "world.player-spawn.missing",
            GAMEPLAY_PATH,
            "at least one enabled stable playerSpawn slot is required",
        )
    seen: set[str] = set()
    for index, spawn in enumerate(enabled_spawns):
        placement_id = spawn.get("placementId")
        if not isinstance(placement_id, str) or not STABLE_ID.fullmatch(placement_id):
            stage.issue(
                "world.player-spawn.id.invalid",
                GAMEPLAY_PATH,
                f"placements[{index}] placementId={placement_id!r}",
            )
        elif placement_id in seen:
            stage.issue(
                "world.player-spawn.id.duplicate",
                GAMEPLAY_PATH,
                placement_id,
            )
        else:
            seen.add(placement_id)
        if spawn.get("archetypeId", None) is not None:
            stage.issue(
                "world.player-spawn.archetype.invalid",
                GAMEPLAY_PATH,
                f"{placement_id}: playerSpawn archetypeId must be null",
            )
        position = spawn.get("position")
        if (
            not isinstance(position, list)
            or len(position) != 3
            or any(
                isinstance(value, bool)
                or not isinstance(value, (int, float))
                or not math.isfinite(float(value))
                for value in position
            )
        ):
            stage.issue(
                "world.player-spawn.position.invalid",
                GAMEPLAY_PATH,
                f"{placement_id}: expected three finite coordinates",
            )
    return document, enabled_spawns


@dataclass(frozen=True)
class _NavGrid:
    width: int
    height: int
    cell_size: float
    origin_x: float
    origin_z: float
    walkable: bytes
    heights: tuple[float, ...]


def _parse_navgrid(root: Path, relative: Path, stage: _Stage) -> _NavGrid | None:
    path = root / relative
    if not path.is_file():
        stage.issue("file.missing", relative, "required Server navigation grid does not exist")
        return None
    try:
        data = path.read_bytes()
    except OSError as error:
        stage.issue("navigation.grid.unreadable", relative, str(error))
        return None
    if len(data) < 20:
        stage.issue("navigation.grid.invalid", relative, "header is shorter than 20 bytes")
        return None
    width, height, cell_size, origin_x, origin_z = struct.unpack_from("<IIfff", data, 0)
    count = width * height
    expected = 20 + count + 4 * count
    if (
        width == 0
        or height == 0
        or count > 100_000_000
        or not math.isfinite(cell_size)
        or cell_size <= 0.0
        or not math.isfinite(origin_x)
        or not math.isfinite(origin_z)
        or len(data) != expected
    ):
        stage.issue(
            "navigation.grid.invalid",
            relative,
            f"width={width} height={height} cellSize={cell_size} bytes={len(data)} expected={expected}",
        )
        return None
    walkable = data[20 : 20 + count]
    if any(value not in (0, 1) for value in walkable):
        stage.issue("navigation.grid.walkable.invalid", relative, "walkable bytes must be 0 or 1")
        return None
    heights = struct.unpack_from(f"<{count}f", data, 20 + count)
    if any(not math.isfinite(value) for value in heights):
        stage.issue("navigation.grid.height.invalid", relative, "cell heights must be finite")
        return None
    return _NavGrid(width, height, cell_size, origin_x, origin_z, walkable, heights)


def _validate_spawn_nav(
    spawns: list[dict[str, Any]], grid: _NavGrid | None, stage: _Stage
) -> None:
    if grid is None:
        return
    for spawn in spawns:
        position = spawn.get("position")
        placement_id = spawn.get("placementId", "<invalid>")
        if not isinstance(position, list) or len(position) != 3:
            continue
        try:
            x, y, z = (float(value) for value in position)
        except (TypeError, ValueError):
            continue
        cell_x = math.floor((x - grid.origin_x) / grid.cell_size)
        cell_z = math.floor((z - grid.origin_z) / grid.cell_size)
        if not (0 <= cell_x < grid.width and 0 <= cell_z < grid.height):
            stage.issue(
                "navigation.spawn.outside",
                GAMEPLAY_PATH,
                f"{placement_id}: cell=({cell_x},{cell_z})",
            )
            continue
        index = cell_z * grid.width + cell_x
        if grid.walkable[index] != 1:
            stage.issue(
                "navigation.spawn.blocked",
                GAMEPLAY_PATH,
                f"{placement_id}: cell=({cell_x},{cell_z}) is not walkable",
            )
        if abs(y - grid.heights[index]) > 0.25:
            stage.issue(
                "navigation.spawn.height",
                GAMEPLAY_PATH,
                f"{placement_id}: worldY={y} navY={grid.heights[index]}",
            )


def _require_text_tokens(
    root: Path,
    relative: Path,
    tokens: Iterable[str],
    stage: _Stage,
    code_prefix: str,
) -> str | None:
    text = _read_text(root, relative, stage)
    if text is None:
        return None
    for token in tokens:
        if token not in text:
            stage.issue(
                f"{code_prefix}.token.missing",
                relative,
                f"missing executable contract token: {token}",
            )
    return text


def _validate_resource_collection(root: Path) -> ModeResult:
    stage = _Stage(RESOURCE_MODE)
    intake = _read_json(root, INTAKE_PATH, stage)
    if isinstance(intake, dict):
        if intake.get("schema") != "lostark.resource-intake" or intake.get(
            "formatVersion"
        ) != 2:
            stage.issue(
                "resource-intake.header.invalid",
                INTAKE_PATH,
                "expected lostark.resource-intake formatVersion=2",
            )
        alias = intake.get("aliasContract")
        canonical = intake.get("canonicalIdentity")
        if not isinstance(alias, dict) or (
            alias.get("alias") != COLLECTION_NAME
            or alias.get("canonical") is not False
            or alias.get("canonicalAreaId") != AREA_ID
        ):
            stage.issue(
                "resource-intake.alias.invalid",
                INTAKE_PATH,
                f"{COLLECTION_NAME} must be a non-canonical display alias for {AREA_ID}",
            )
        if not isinstance(canonical, dict) or canonical.get("areaId") != AREA_ID:
            stage.issue(
                "resource-intake.identity.invalid",
                INTAKE_PATH,
                f"canonicalIdentity.areaId must equal {AREA_ID}",
            )
    elif intake is not None:
        stage.issue("resource-intake.root.invalid", INTAKE_PATH, "expected object")

    candidate_roots = (
        Path(f"Client/Bin/Resources/Map/{AREA_ID}"),
        Path(f"Client/Bin/Resources/Effect/{COLLECTION_NAME}"),
        Path(f"Client/Bin/Resources/UI/{COLLECTION_NAME}"),
        Path(f"Client/Bin/Resources/Sound/{COLLECTION_NAME}"),
        Path("Client/Bin/Resources/Character/MN_RPCT_05"),
        Path("Client/Bin/Resources/Character/MN_RPCZ_00"),
        Path("Client/Bin/Resources/Character/MN_RPCT_06"),
        Path("Client/Bin/Resources/Character/MN_RPCT_07"),
    )
    available_roots = [
        relative.as_posix() for relative in candidate_roots if _has_file(root / relative)
    ]
    if not available_roots:
        stage.issue(
            "resource-collection.payload.missing",
            "Client/Bin/Resources",
            "no KoukuSaton stable resource payload was found; aliases alone are not assets",
        )
    stage.result.facts["availableResourceRoots"] = available_roots
    stage.result.permitted = not stage.result.own_findings
    return stage.result


def _validate_geometry(root: Path, prerequisite: ModeResult) -> ModeResult:
    stage = _Stage(GEOMETRY_MODE)
    _, area = _load_map_catalog(root, stage)
    expected_fields = {
        "sourceCatalog": SOURCE_CATALOG_PATH.as_posix(),
        "sourcePlacements": SOURCE_PLACEMENTS_PATH.as_posix(),
        "catalog": RUNTIME_CATALOG_PATH.as_posix(),
        "placements": RUNTIME_PLACEMENTS_PATH.as_posix(),
        "runtimeAssetRoot": f"Map/{AREA_ID}",
    }
    for key, expected in expected_fields.items():
        _expect_catalog_field(stage, area, key, expected)
    if area is not None and area.get("kind") not in ("development", "product"):
        stage.issue(
            "map-catalog.kind.invalid",
            MAP_CATALOG_PATH,
            f"expected development or product; got {area.get('kind')!r}",
        )

    assets = _parse_map_assets(root, SOURCE_CATALOG_PATH, stage)
    placement_count = _parse_map_placements(
        root, SOURCE_PLACEMENTS_PATH, assets, stage
    )
    if assets is not None:
        for asset_id, resource_id in assets.items():
            resource_path = Path("Client/Bin/Resources") / Path(resource_id)
            if not (root / resource_path).is_file():
                stage.issue(
                    "geometry.resource.missing",
                    resource_path,
                    f"referenced by map asset {asset_id}",
                )
        stage.result.facts["mapAssetCount"] = len(assets)
        if area is not None and isinstance(area.get("assetCount"), int):
            if area["assetCount"] != len(assets):
                stage.issue(
                    "map-catalog.assetCount.mismatch",
                    MAP_CATALOG_PATH,
                    f"catalog={area['assetCount']} source={len(assets)}",
                )
    if placement_count is not None:
        stage.result.facts["mapPlacementCount"] = placement_count
        if area is not None and isinstance(area.get("placementCount"), int):
            if area["placementCount"] != placement_count:
                stage.issue(
                    "map-catalog.placementCount.mismatch",
                    MAP_CATALOG_PATH,
                    f"catalog={area['placementCount']} source={placement_count}",
                )
    stage.result.permitted = prerequisite.permitted and not stage.result.own_findings
    return stage.result


def _validate_product(
    root: Path, prerequisite: ModeResult
) -> ModeResult:
    stage = _Stage(PRODUCT_MODE)
    _, area = _load_map_catalog(root, stage)
    if area is not None and area.get("kind") != "product":
        stage.issue(
            "map-catalog.product-kind.missing",
            MAP_CATALOG_PATH,
            f"Server admission requires kind='product'; got {area.get('kind')!r}",
        )
    product_fields = {
        "navigationSource": f"Data/Navigation/{AREA_ID}.navsource",
        "navigationPaint": f"Data/Navigation/{AREA_ID}.navpaint",
        "navigationRuntime": f"Client/Bin/DataFiles/Navigation/{AREA_ID}.navgrid",
        "gameplayDocument": GAMEPLAY_PATH.as_posix(),
    }
    for key, expected in product_fields.items():
        _expect_catalog_field(stage, area, key, expected)

    _require_same_bytes(
        root,
        SOURCE_CATALOG_PATH,
        RUNTIME_CATALOG_PATH,
        stage,
        "map-runtime.catalog.stale",
    )
    _require_same_bytes(
        root,
        SOURCE_PLACEMENTS_PATH,
        RUNTIME_PLACEMENTS_PATH,
        stage,
        "map-runtime.placements.stale",
    )

    _, spawns = _parse_world(root, stage)
    nav_source = Path(f"Data/Navigation/{AREA_ID}.navsource")
    nav_paint = Path(f"Data/Navigation/{AREA_ID}.navpaint")
    for relative in (nav_source, nav_paint):
        if not (root / relative).is_file():
            stage.issue("file.missing", relative, "required navigation authoring file does not exist")
    nav_publisher = _require_text_tokens(
        root,
        Path("Tools/NavigationPipeline/Publish-ServerNavigation.ps1"),
        (nav_source.as_posix(), nav_paint.as_posix()),
        stage,
        "navigation.publisher",
    )
    if nav_publisher is not None and AREA_ID not in nav_publisher:
        stage.issue(
            "navigation.publisher.area.missing",
            "Tools/NavigationPipeline/Publish-ServerNavigation.ps1",
            AREA_ID,
        )

    server_grid_path = Path(f"Server/Bin/DataFiles/Navigation/{AREA_ID}.navgrid")
    client_grid_path = Path(f"Client/Bin/DataFiles/Navigation/{AREA_ID}.navgrid")
    grid = _parse_navgrid(root, server_grid_path, stage)
    _validate_spawn_nav(spawns, grid, stage)
    _require_same_bytes(
        root,
        server_grid_path,
        client_grid_path,
        stage,
        "navigation.client-server.drift",
    )
    for base in ("Server/Bin/DataFiles/Navigation", "Client/Bin/DataFiles/Navigation"):
        for extension in ("navpolicy", "navblockers"):
            relative = Path(f"{base}/{AREA_ID}.{extension}")
            if not (root / relative).is_file():
                stage.issue("file.missing", relative, "required published navigation companion is absent")

    world_publisher_path = Path("Tools/WorldPipeline/Publish-WorldGameplay.ps1")
    world_publisher = _read_text(root, world_publisher_path, stage)
    if world_publisher is not None:
        registration = re.compile(
            rf"Convert-WorldDocument\s+-AreaId\s+['\"]{AREA_ID}['\"]\s+"
            rf"-WorldId\s+['\"]{WORLD_ID}['\"]"
        )
        if registration.search(world_publisher) is None:
            stage.issue(
                "world.publisher.registration.missing",
                world_publisher_path,
                f"missing {AREA_ID} -> {WORLD_ID} Convert-WorldDocument registration",
            )

    shared_path = Path("Shared/Public/Network/PacketType.h")
    shared = _read_text(root, shared_path, stage)
    if shared is not None:
        if re.search(rf"\b{WORLD_ID}\s*=\s*\d+", shared) is None:
            stage.issue("shared.world-id.enum.missing", shared_path, WORLD_ID)
        if shared.count(f"WORLD_ID::{WORLD_ID}") < 1:
            stage.issue(
                "shared.world-id.known.missing",
                shared_path,
                f"Is_Known_World_Id must admit WORLD_ID::{WORLD_ID}",
            )

    _require_text_tokens(
        root,
        Path("Server/Private/WorldBootstrap.cpp"),
        (f"WORLD_ID::{WORLD_ID}", f'"{WORLD_ID}"'),
        stage,
        "server.world-bootstrap",
    )
    server_app = _read_text(root, Path("Server/Private/ServerApp.cpp"), stage)
    if server_app is not None and re.search(
        rf"stageSharedSimulation\s*\(\s*WORLD_ID::{WORLD_ID}\s*\)", server_app
    ) is None:
        stage.issue(
            "server.shared-simulation.registration.missing",
            "Server/Private/ServerApp.cpp",
            f"missing stageSharedSimulation(WORLD_ID::{WORLD_ID})",
        )
    bootstrap_path = Path(f"Server/Bin/DataFiles/World/{WORLD_ID}.worldbootstrap")
    bootstrap = _read_text(root, bootstrap_path, stage)
    if bootstrap is not None and not re.search(
        rf"^LOSTARK_WORLD_BOOTSTRAP\t\d+\t{WORLD_ID}(?:\t|$)",
        bootstrap,
        re.MULTILINE,
    ):
        stage.issue(
            "server.world-bootstrap.header.invalid",
            bootstrap_path,
            f"expected LOSTARK_WORLD_BOOTSTRAP <version> {WORLD_ID}",
        )

    _require_text_tokens(
        root,
        Path("Client/Public/Client_Defines.h"),
        (CLIENT_LEVEL,),
        stage,
        "client.level-enum",
    )
    registry_path = Path("Client/Private/LevelRegistry.cpp")
    _require_text_tokens(
        root,
        registry_path,
        (
            f"LEVEL::{CLIENT_LEVEL}",
            "CLIENT_LEVEL_KIND::PRODUCT",
            '"raid.kakul-saydon.arena"',
            f'"{AREA_ID}"',
            "CreateKakulSaydonArena",
            "Ready_For_KakulSaydonArena",
        ),
        stage,
        "client.level-registry",
    )
    _require_text_tokens(
        root,
        Path("Client/Public/Loader.h"),
        ("Ready_For_KakulSaydonArena",),
        stage,
        "client.loader",
    )
    _require_text_tokens(
        root,
        Path("Client/Private/Loader.cpp"),
        ("Ready_For_KakulSaydonArena", f"LEVEL::{CLIENT_LEVEL}"),
        stage,
        "client.loader",
    )
    _require_text_tokens(
        root,
        Path("Client/Private/LevelTransitionService.cpp"),
        (f"WORLD_ID::{WORLD_ID}", f"LEVEL::{CLIENT_LEVEL}"),
        stage,
        "client.level-transition",
    )
    level_header = Path("Client/Public/Level_KakulSaydonArena.h")
    level_source = Path("Client/Private/Level_KakulSaydonArena.cpp")
    _require_text_tokens(
        root,
        level_header,
        ("CLevel_KakulSaydonArena",),
        stage,
        "client.level-class",
    )
    _require_text_tokens(
        root,
        level_source,
        ("CLevel_KakulSaydonArena", AREA_ID),
        stage,
        "client.level-class",
    )
    _require_text_tokens(
        root,
        Path("Client/Default/Client.vcxproj"),
        ("Level_KakulSaydonArena.h", "Level_KakulSaydonArena.cpp"),
        stage,
        "client.project",
    )
    _require_text_tokens(
        root,
        Path("Client/Default/Client.vcxproj.filters"),
        ("Level_KakulSaydonArena.h", "Level_KakulSaydonArena.cpp"),
        stage,
        "client.filters",
    )

    stage.result.facts["stablePlayerSpawnCount"] = len(spawns)
    stage.result.permitted = prerequisite.permitted and not stage.result.own_findings
    return stage.result


def validate_repository(root: Path = REPO_ROOT) -> AdmissionReport:
    resolved = root.resolve()
    resource = _validate_resource_collection(resolved)
    geometry = _validate_geometry(resolved, resource)
    product = _validate_product(resolved, geometry)
    return AdmissionReport(resolved, [resource, geometry, product])


def format_human(
    report: AdmissionReport,
    requested_mode: str,
    *,
    verbose: bool = False,
    detail_limit: int = 12,
) -> str:
    if detail_limit < 0:
        raise ValueError("detail_limit must be nonnegative")
    lines = [
        f"KoukuSaton admission: collection={COLLECTION_NAME} canonicalArea={AREA_ID}",
        f"Repository: {report.root}",
        "Spawn: arbitrary random world positions are forbidden; use stable nav-valid slots.",
    ]
    for result in report.modes:
        state = "PERMITTED" if result.permitted else "BLOCKED"
        lines.append(f"[{state}] {MODE_LABELS[result.mode]}")
        if not result.own_findings and not result.permitted:
            lines.append("  - prerequisite mode is blocked")
        if result.own_findings:
            counts: dict[str, int] = {}
            for finding in result.own_findings:
                counts[finding.code] = counts.get(finding.code, 0) + 1
            summary = ", ".join(
                f"{code}={count}" for code, count in sorted(counts.items())
            )
            lines.append(
                f"  findings: {len(result.own_findings)} total ({summary})"
            )
        visible_findings = (
            result.own_findings
            if verbose
            else result.own_findings[:detail_limit]
        )
        for finding in visible_findings:
            lines.append(
                f"  - {finding.code}: {finding.path}: {finding.detail}"
            )
        hidden_count = len(result.own_findings) - len(visible_findings)
        if hidden_count > 0:
            lines.append(
                f"  ... {hidden_count} more finding(s); rerun with --verbose for all details"
            )
    highest = report.highest_permitted_mode
    lines.append(
        "Highest permitted mode: " + (MODE_LABELS[highest] if highest else "NONE")
    )
    lines.append(
        f"Requested mode: {MODE_LABELS[requested_mode]} -> "
        + ("PERMITTED" if report.result(requested_mode).permitted else "BLOCKED")
    )
    return "\n".join(lines)


def _parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repo-root",
        type=Path,
        default=REPO_ROOT,
        help="repository root to inspect (default: this script's repository)",
    )
    parser.add_argument(
        "--require",
        choices=MODE_ORDER,
        default=PRODUCT_MODE,
        dest="required_mode",
        help="mode whose admission determines the exit code",
    )
    parser.add_argument("--json", action="store_true", help="emit structured JSON")
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="show every human-readable finding instead of the bounded default",
    )
    parser.add_argument(
        "--report-only",
        action="store_true",
        help="always exit 0 after reporting; never use this as a Product admission gate",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = _parse_args(sys.argv[1:] if argv is None else argv)
    report = validate_repository(args.repo_root)
    if args.json:
        print(
            json.dumps(
                report.as_json(args.required_mode),
                ensure_ascii=False,
                indent=2,
            )
        )
    else:
        print(
            format_human(
                report,
                args.required_mode,
                verbose=args.verbose,
            )
        )
    if args.report_only:
        return 0
    return 0 if report.result(args.required_mode).permitted else 1


if __name__ == "__main__":
    raise SystemExit(main())
