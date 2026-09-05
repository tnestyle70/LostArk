#!/usr/bin/env python3
"""Build KoukuSaydon Server navigation, stable spawns and source-level waypoints.

The input is the restored MapTool catalog/placement closure.  Model vertices use
the same 0.01 pre-transform as Client/Private/Loader.cpp.  Stage labels remain
the exact extracted ``*_SLxx`` source-level identities; this tool never guesses
that one of those packages is a Mario room or raid gate.
"""

from __future__ import annotations

import argparse
from collections import deque
from dataclasses import dataclass
import json
import math
import os
from pathlib import Path
import shlex
import sys
import tempfile
from typing import Iterable

import numpy as np


REPO_ROOT = Path(__file__).resolve().parents[2]
AREA_ID = "LV_LUT_MIDNIGHTC_ED"
MODEL_PRE_SCALE = 0.01
DEFAULT_CELL_SIZE = 4.0
DEFAULT_MAXIMUM_SLOPE_DEGREES = 45.0
DEFAULT_RUNTIME_MAXIMUM_STEP = 1.0
MAX_GRID_CELLS = 1_000_000

CATALOG_PATH = Path(
    f"Data/Maps/Imported/{AREA_ID}/{AREA_ID}.mapassets"
)
PLACEMENTS_PATH = Path(
    f"Data/Maps/Authoring/{AREA_ID}/{AREA_ID}.mapplacements"
)
NAVSOURCE_PATH = Path(f"Data/Navigation/{AREA_ID}.navsource")
NAVPAINT_PATH = Path(f"Data/Navigation/{AREA_ID}.navpaint")
WORLD_PATH = Path(f"Data/Worlds/{AREA_ID}/Gameplay.world.json")
STAGE_MARKERS_PATH = Path(f"Data/Worlds/{AREA_ID}/StageMarkers.json")


MODEL_ASSET_CONVERTER = REPO_ROOT / "Tools/ModelAssetConverter"
LEVEL_PLACEMENT_EXTRACTOR = REPO_ROOT / "Tools/LevelPlacementExtractor"
for module_root in (MODEL_ASSET_CONVERTER, LEVEL_PLACEMENT_EXTRACTOR):
    module_text = str(module_root)
    if module_text not in sys.path:
        sys.path.insert(0, module_text)

from placement_transform import apply_placement, placement_matrix  # noqa: E402
from cook_wmodel_geometry_contract import (  # noqa: E402
    parse_geometry_wmodel,
    parse_legacy_wmodel,
)


class BuildError(RuntimeError):
    pass


@dataclass(frozen=True)
class FloorAsset:
    asset_id: str
    label: str
    resource_path: Path


@dataclass(frozen=True)
class FloorPlacement:
    source_placement_id: str
    source_level_id: str
    asset_id: str
    position: tuple[float, float, float]
    quaternion: tuple[float, float, float, float]
    scale: tuple[float, float, float]


@dataclass(frozen=True)
class GridSpec:
    origin_x: float
    origin_z: float
    width: int
    height: int
    cell_size: float


@dataclass(frozen=True)
class StageWaypoint:
    source_level_id: str
    placement_id: str
    cell_x: int
    cell_z: int
    x: float
    y: float
    z: float
    component_cells: int


def _stable_suffix(source_level_id: str) -> str:
    prefix = AREA_ID + "_"
    if not source_level_id.startswith(prefix):
        raise BuildError(f"unexpected source level: {source_level_id}")
    suffix = source_level_id[len(prefix) :].lower()
    if not suffix or not all(character.isalnum() or character in "_.-" for character in suffix):
        raise BuildError(f"unstable source-level suffix: {source_level_id}")
    return suffix


def load_floor_assets(root: Path) -> dict[str, FloorAsset]:
    path = root / CATALOG_PATH
    lines = [line for line in path.read_text(encoding="utf-8-sig").splitlines() if line.strip()]
    if not lines:
        raise BuildError(f"empty map catalog: {path}")
    header = shlex.split(lines[0], posix=True)
    if (
        len(header) != 4
        or header[0] != "LOSTARK_MAP_ASSET_CATALOG"
        or header[2] != AREA_ID
        or int(header[3]) != len(lines) - 1
    ):
        raise BuildError(f"invalid map catalog header: {path}")

    result: dict[str, FloorAsset] = {}
    for line_number, line in enumerate(lines[1:], start=2):
        tokens = shlex.split(line, posix=True)
        if len(tokens) < 8:
            raise BuildError(f"truncated map catalog row {line_number}")
        asset_id, label, resource_id = tokens[0], tokens[1], tokens[2]
        if "floor" not in label.casefold():
            continue
        resource_path = root / "Client/Bin/Resources" / Path(resource_id)
        if not resource_path.is_file():
            raise BuildError(f"floor WModel is missing: {resource_path}")
        if asset_id in result:
            raise BuildError(f"duplicate floor asset: {asset_id}")
        result[asset_id] = FloorAsset(asset_id, label, resource_path)
    if not result:
        raise BuildError("restored map catalog contains no floor assets")
    return result


def load_floor_placements(
    root: Path,
    assets: dict[str, FloorAsset],
) -> list[FloorPlacement]:
    path = root / PLACEMENTS_PATH
    lines = [line for line in path.read_text(encoding="utf-8-sig").splitlines() if line.strip()]
    if not lines:
        raise BuildError(f"empty map placements: {path}")
    header = shlex.split(lines[0], posix=True)
    if (
        len(header) != 4
        or header[0] != "LOSTARK_MAP_PLACEMENTS"
        or header[2] != AREA_ID
        or int(header[3]) != len(lines) - 1
    ):
        raise BuildError(f"invalid map placement header: {path}")

    result: list[FloorPlacement] = []
    for line_number, line in enumerate(lines[1:], start=2):
        tokens = shlex.split(line, posix=True)
        if len(tokens) != 16:
            raise BuildError(f"invalid placement row {line_number}: fields={len(tokens)}")
        asset_id = tokens[4]
        if asset_id not in assets or tokens[15] != "1":
            continue
        numeric = tuple(float(value) for value in tokens[5:15])
        if not all(math.isfinite(value) for value in numeric):
            raise BuildError(f"non-finite placement transform at row {line_number}")
        result.append(
            FloorPlacement(
                source_placement_id=tokens[1],
                source_level_id=tokens[2],
                asset_id=asset_id,
                position=numeric[0:3],
                quaternion=numeric[3:7],
                scale=numeric[7:10],
            )
        )
    if not result:
        raise BuildError("restored placement document contains no visible floor placements")
    return result


def load_model_triangles(path: Path) -> np.ndarray:
    payload = path.read_bytes()
    blocks: list[np.ndarray] = []
    try:
        geometry = parse_geometry_wmodel(payload)
        rows: Iterable[tuple[Iterable[Iterable[float]], Iterable[int]]] = (
            (
                (vertex["values"][:3] for vertex in submesh["vertices"]),
                submesh["indices"],
            )
            for submesh in geometry["submeshes"]
        )
    except ValueError:
        _, _, legacy_submeshes = parse_legacy_wmodel(payload)
        rows = (
            ((vertex[:3] for vertex in submesh.vertices), submesh.indices)
            for submesh in legacy_submeshes
        )

    for vertices_source, indices_source in rows:
        vertices = np.asarray(list(vertices_source), dtype=np.float64)
        indices = np.asarray(tuple(indices_source), dtype=np.int64)
        if vertices.ndim != 2 or vertices.shape[1] != 3 or len(indices) % 3 != 0:
            raise BuildError(f"invalid static triangle topology: {path}")
        if len(indices):
            blocks.append(
                (vertices * MODEL_PRE_SCALE)[indices].reshape((-1, 3, 3))
            )
    if not blocks:
        raise BuildError(f"floor WModel contains no triangles: {path}")
    return np.concatenate(blocks, axis=0)


def build_stage_triangle_sets(
    assets: dict[str, FloorAsset],
    placements: list[FloorPlacement],
) -> dict[str, np.ndarray]:
    model_cache: dict[str, np.ndarray] = {}
    stage_blocks: dict[str, list[np.ndarray]] = {}
    for placement in placements:
        source = model_cache.get(placement.asset_id)
        if source is None:
            source = load_model_triangles(assets[placement.asset_id].resource_path)
            model_cache[placement.asset_id] = source
        transform = placement_matrix(
            {
                "position": placement.position,
                "quaternion": placement.quaternion,
                "scale": placement.scale,
            }
        )
        stage_blocks.setdefault(placement.source_level_id, []).append(
            apply_placement(source, transform)
        )
    return {
        source_level_id: np.concatenate(blocks, axis=0)
        for source_level_id, blocks in stage_blocks.items()
    }


def make_grid_spec(stage_triangles: dict[str, np.ndarray], cell_size: float) -> GridSpec:
    if not math.isfinite(cell_size) or cell_size <= 0.0:
        raise BuildError("cell size must be finite and positive")
    all_triangles = np.concatenate(tuple(stage_triangles.values()), axis=0)
    minimum = all_triangles.min(axis=(0, 1))
    maximum = all_triangles.max(axis=(0, 1))
    origin_x = math.floor(float(minimum[0]) / cell_size) * cell_size
    origin_z = math.floor(float(minimum[2]) / cell_size) * cell_size
    width = max(1, math.ceil((float(maximum[0]) - origin_x) / cell_size))
    height = max(1, math.ceil((float(maximum[2]) - origin_z) / cell_size))
    if width * height > MAX_GRID_CELLS:
        raise BuildError(
            f"navigation grid exceeds {MAX_GRID_CELLS} cells: {width}x{height}"
        )
    return GridSpec(origin_x, origin_z, width, height, cell_size)


def rasterize_floor_triangles(
    triangles: np.ndarray,
    grid: GridSpec,
    maximum_slope_degrees: float,
) -> tuple[np.ndarray, np.ndarray]:
    if not math.isfinite(maximum_slope_degrees) or not 0.0 <= maximum_slope_degrees < 90.0:
        raise BuildError("maximum slope must be in [0, 90)")
    resolved = np.zeros((grid.height, grid.width), dtype=np.uint8)
    heights = np.zeros((grid.height, grid.width), dtype=np.float32)
    minimum_normal_y = math.cos(math.radians(maximum_slope_degrees))

    for triangle in triangles:
        edge_a = triangle[1] - triangle[0]
        edge_b = triangle[2] - triangle[0]
        normal = np.cross(edge_a, edge_b)
        normal_length = float(np.linalg.norm(normal))
        if normal_length <= 1e-12 or abs(float(normal[1])) / normal_length < minimum_normal_y:
            continue
        triangle_xz = triangle[:, [0, 2]]
        denominator = (
            (triangle_xz[1, 1] - triangle_xz[2, 1])
            * (triangle_xz[0, 0] - triangle_xz[2, 0])
            + (triangle_xz[2, 0] - triangle_xz[1, 0])
            * (triangle_xz[0, 1] - triangle_xz[2, 1])
        )
        if abs(float(denominator)) <= 1e-12:
            continue
        min_x = max(
            0,
            math.floor((float(triangle_xz[:, 0].min()) - grid.origin_x) / grid.cell_size),
        )
        max_x = min(
            grid.width - 1,
            math.floor((float(triangle_xz[:, 0].max()) - grid.origin_x) / grid.cell_size),
        )
        min_z = max(
            0,
            math.floor((float(triangle_xz[:, 1].min()) - grid.origin_z) / grid.cell_size),
        )
        max_z = min(
            grid.height - 1,
            math.floor((float(triangle_xz[:, 1].max()) - grid.origin_z) / grid.cell_size),
        )
        for cell_z in range(min_z, max_z + 1):
            sample_z = grid.origin_z + (cell_z + 0.5) * grid.cell_size
            for cell_x in range(min_x, max_x + 1):
                sample_x = grid.origin_x + (cell_x + 0.5) * grid.cell_size
                barycentric_a = (
                    (triangle_xz[1, 1] - triangle_xz[2, 1])
                    * (sample_x - triangle_xz[2, 0])
                    + (triangle_xz[2, 0] - triangle_xz[1, 0])
                    * (sample_z - triangle_xz[2, 1])
                ) / denominator
                barycentric_b = (
                    (triangle_xz[2, 1] - triangle_xz[0, 1])
                    * (sample_x - triangle_xz[2, 0])
                    + (triangle_xz[0, 0] - triangle_xz[2, 0])
                    * (sample_z - triangle_xz[2, 1])
                ) / denominator
                barycentric_c = 1.0 - barycentric_a - barycentric_b
                if min(barycentric_a, barycentric_b, barycentric_c) < -1e-7:
                    continue
                sample_y = float(
                    barycentric_a * triangle[0, 1]
                    + barycentric_b * triangle[1, 1]
                    + barycentric_c * triangle[2, 1]
                )
                if not math.isfinite(sample_y):
                    raise BuildError("floor rasterization produced a non-finite height")
                if resolved[cell_z, cell_x] == 0 or sample_y > heights[cell_z, cell_x]:
                    resolved[cell_z, cell_x] = 1
                    heights[cell_z, cell_x] = sample_y
    if int(resolved.sum()) == 0:
        raise BuildError("floor rasterization produced no walkable cells")
    return resolved, heights


def merge_stage_grids(
    stage_grids: dict[str, tuple[np.ndarray, np.ndarray]]
) -> tuple[np.ndarray, np.ndarray]:
    first_resolved, _ = next(iter(stage_grids.values()))
    resolved = np.zeros_like(first_resolved)
    heights = np.zeros(first_resolved.shape, dtype=np.float32)
    for stage_resolved, stage_heights in stage_grids.values():
        replace = np.logical_and(
            stage_resolved != 0,
            np.logical_or(resolved == 0, stage_heights > heights),
        )
        resolved[replace] = 1
        heights[replace] = stage_heights[replace]
    return resolved, heights


def largest_component(
    resolved: np.ndarray,
    heights: np.ndarray,
    maximum_step: float,
) -> list[tuple[int, int]]:
    if not math.isfinite(maximum_step) or maximum_step < 0.0:
        raise BuildError("maximum step must be finite and nonnegative")
    height, width = resolved.shape
    visited = np.zeros_like(resolved)
    largest: list[tuple[int, int]] = []
    for start_z in range(height):
        for start_x in range(width):
            if resolved[start_z, start_x] == 0 or visited[start_z, start_x] != 0:
                continue
            queue: deque[tuple[int, int]] = deque([(start_x, start_z)])
            visited[start_z, start_x] = 1
            component: list[tuple[int, int]] = []
            while queue:
                cell_x, cell_z = queue.popleft()
                component.append((cell_x, cell_z))
                source_height = float(heights[cell_z, cell_x])
                for next_x, next_z in (
                    (cell_x + 1, cell_z),
                    (cell_x - 1, cell_z),
                    (cell_x, cell_z + 1),
                    (cell_x, cell_z - 1),
                ):
                    if (
                        next_x < 0
                        or next_z < 0
                        or next_x >= width
                        or next_z >= height
                        or resolved[next_z, next_x] == 0
                        or visited[next_z, next_x] != 0
                        or abs(float(heights[next_z, next_x]) - source_height)
                        > maximum_step
                    ):
                        continue
                    visited[next_z, next_x] = 1
                    queue.append((next_x, next_z))
            if len(component) > len(largest):
                largest = component
    if not largest:
        raise BuildError("stage contains no connected walkable component")
    return largest


def representative_cells(
    component: list[tuple[int, int]],
    count: int,
) -> list[tuple[int, int]]:
    if count <= 0 or len(component) < count:
        raise BuildError(
            f"component has too few cells: cells={len(component)} required={count}"
        )
    center_x = sum(cell[0] for cell in component) / len(component)
    center_z = sum(cell[1] for cell in component) / len(component)
    ordered = sorted(
        component,
        key=lambda cell: (
            (cell[0] - center_x) ** 2 + (cell[1] - center_z) ** 2,
            cell[1],
            cell[0],
        ),
    )
    selected: list[tuple[int, int]] = []
    for cell in ordered:
        if all(abs(cell[0] - other[0]) + abs(cell[1] - other[1]) >= 2 for other in selected):
            selected.append(cell)
            if len(selected) == count:
                return selected
    raise BuildError("component cannot provide separated stable spawn cells")


def cell_position(
    grid: GridSpec,
    heights: np.ndarray,
    cell: tuple[int, int],
) -> tuple[float, float, float]:
    cell_x, cell_z = cell
    return (
        grid.origin_x + (cell_x + 0.5) * grid.cell_size,
        float(heights[cell_z, cell_x]),
        grid.origin_z + (cell_z + 0.5) * grid.cell_size,
    )


def build_waypoints(
    grid: GridSpec,
    stage_grids: dict[str, tuple[np.ndarray, np.ndarray]],
    maximum_step: float,
) -> tuple[list[StageWaypoint], list[tuple[float, float, float]]]:
    waypoints: list[StageWaypoint] = []
    primary_component: list[tuple[int, int]] | None = None
    primary_heights: np.ndarray | None = None
    for source_level_id in sorted(stage_grids):
        resolved, heights = stage_grids[source_level_id]
        component = largest_component(resolved, heights, maximum_step)
        cell = representative_cells(component, 1)[0]
        x, y, z = cell_position(grid, heights, cell)
        waypoints.append(
            StageWaypoint(
                source_level_id=source_level_id,
                placement_id="stage.kakul." + _stable_suffix(source_level_id),
                cell_x=cell[0],
                cell_z=cell[1],
                x=x,
                y=y,
                z=z,
                component_cells=len(component),
            )
        )
        if primary_component is None:
            primary_component = component
            primary_heights = heights
    assert primary_component is not None and primary_heights is not None
    spawn_cells = representative_cells(primary_component, 4)
    spawns = [cell_position(grid, primary_heights, cell) for cell in spawn_cells]
    return waypoints, spawns


def serialize_navsource(
    grid: GridSpec,
    resolved: np.ndarray,
    heights: np.ndarray,
) -> str:
    lines = [
        f'LOSTARK_NAVGRID_SOURCE 1 "{AREA_ID}" {grid.width} {grid.height} '
        f"{grid.cell_size:.9g} {grid.origin_x:.9g} {grid.origin_z:.9g} "
        f"{grid.width * grid.height}"
    ]
    for cell_z in range(grid.height):
        for cell_x in range(grid.width):
            surface = int(resolved[cell_z, cell_x])
            value = float(heights[cell_z, cell_x]) if surface else 0.0
            lines.append(f"{cell_x} {cell_z} {surface} {value:.9g}")
    return "\n".join(lines) + "\n"


def serialize_navpaint(grid: GridSpec) -> str:
    return (
        f'LOSTARK_NAVGRID_PAINT 2 "{AREA_ID}" {grid.width} {grid.height} '
        f"{grid.cell_size:.9g} {grid.origin_x:.9g} {grid.origin_z:.9g} 0\n"
    )


def _placement(
    placement_id: str,
    position: tuple[float, float, float],
    enabled: bool,
) -> dict[str, object]:
    return {
        "placementId": placement_id,
        "kind": "playerSpawn",
        "archetypeId": None,
        "encounterId": None,
        "position": [round(value, 6) for value in position],
        "yawDegrees": 0.0,
        "enabled": enabled,
    }


def serialize_world(
    waypoints: list[StageWaypoint],
    spawns: list[tuple[float, float, float]],
) -> str:
    placements = [
        _placement(f"player.spawn.kakul.party{index:02d}", position, True)
        for index, position in enumerate(spawns, start=1)
    ]
    placements.extend(
        _placement(waypoint.placement_id, (waypoint.x, waypoint.y, waypoint.z), False)
        for waypoint in waypoints
    )
    document = {
        "schema": "lostark.world-gameplay",
        "formatVersion": 6,
        "areaId": AREA_ID,
        "revision": 1,
        "placements": placements,
    }
    return json.dumps(document, ensure_ascii=False, indent=2) + "\n"


def serialize_stage_markers(waypoints: list[StageWaypoint]) -> str:
    document = {
        "schema": "lostark.kakul-stage-markers",
        "formatVersion": 1,
        "areaId": AREA_ID,
        "revision": 1,
        "semanticStatus": "SOURCE_LEVEL_ID_ONLY",
        "stages": [
            {
                "stageId": waypoint.placement_id,
                "placementId": waypoint.placement_id,
                "displayNameKo": f"{waypoint.source_level_id} 복구 스테이지",
                "sourceLevelId": waypoint.source_level_id,
                "componentCellCount": waypoint.component_cells,
                "evidence": (
                    f"{PLACEMENTS_PATH.as_posix()} sourceLevel="
                    f"{waypoint.source_level_id}; no Mario/gate semantic inferred"
                ),
            }
            for waypoint in waypoints
        ],
    }
    return json.dumps(document, ensure_ascii=False, indent=2) + "\n"


def build_outputs(
    root: Path,
    cell_size: float = DEFAULT_CELL_SIZE,
    maximum_slope_degrees: float = DEFAULT_MAXIMUM_SLOPE_DEGREES,
    runtime_maximum_step: float = DEFAULT_RUNTIME_MAXIMUM_STEP,
) -> tuple[dict[Path, str], dict[str, object]]:
    assets = load_floor_assets(root)
    placements = load_floor_placements(root, assets)
    stage_triangles = build_stage_triangle_sets(assets, placements)
    grid = make_grid_spec(stage_triangles, cell_size)
    stage_grids = {
        source_level_id: rasterize_floor_triangles(
            triangles, grid, maximum_slope_degrees
        )
        for source_level_id, triangles in stage_triangles.items()
    }
    resolved, heights = merge_stage_grids(stage_grids)
    waypoints, spawns = build_waypoints(grid, stage_grids, runtime_maximum_step)
    outputs = {
        NAVSOURCE_PATH: serialize_navsource(grid, resolved, heights),
        NAVPAINT_PATH: serialize_navpaint(grid),
        WORLD_PATH: serialize_world(waypoints, spawns),
        STAGE_MARKERS_PATH: serialize_stage_markers(waypoints),
    }
    facts = {
        "areaId": AREA_ID,
        "floorAssetCount": len(assets),
        "visibleFloorPlacementCount": len(placements),
        "sourceLevelCount": len(stage_triangles),
        "sourceLevelIds": sorted(stage_triangles),
        "grid": {
            "width": grid.width,
            "height": grid.height,
            "cellSize": grid.cell_size,
            "cellCount": grid.width * grid.height,
            "walkableCellCount": int(resolved.sum()),
        },
        "stageWaypoints": [waypoint.placement_id for waypoint in waypoints],
        "enabledPlayerSpawns": len(spawns),
    }
    return outputs, facts


def atomic_write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=path.name + ".", suffix=".tmp", dir=path.parent
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as stream:
            stream.write(text)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, path)
    finally:
        if temporary.exists():
            temporary.unlink()


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root", type=Path, default=REPO_ROOT)
    parser.add_argument("--cell-size", type=float, default=DEFAULT_CELL_SIZE)
    parser.add_argument(
        "--maximum-slope-degrees",
        type=float,
        default=DEFAULT_MAXIMUM_SLOPE_DEGREES,
    )
    parser.add_argument(
        "--runtime-maximum-step",
        type=float,
        default=DEFAULT_RUNTIME_MAXIMUM_STEP,
    )
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args(argv)
    root = args.repo_root.resolve()
    outputs, facts = build_outputs(
        root,
        cell_size=args.cell_size,
        maximum_slope_degrees=args.maximum_slope_degrees,
        runtime_maximum_step=args.runtime_maximum_step,
    )
    stale: list[str] = []
    for relative, text in outputs.items():
        path = root / relative
        if args.check:
            if not path.is_file() or path.read_text(encoding="utf-8-sig") != text:
                stale.append(relative.as_posix())
        else:
            atomic_write(path, text)
    if stale:
        raise BuildError(
            "generated KoukuSaydon authoring outputs are stale: " + ", ".join(stale)
        )
    print(json.dumps(facts, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (BuildError, OSError, ValueError) as error:
        print(f"build_kouku_saydon_world_authoring: {error}", file=sys.stderr)
        raise SystemExit(1)
