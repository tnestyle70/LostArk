#!/usr/bin/env python3
"""CPU surface diagnostics; never launches Client or decides visual fidelity.

No arguments runs the synthetic unit tests.  --resource-root and --area-id
inspect the six recorded Character Select placement pairs and print JSON.
Triangle indices are diagnostic locations; only string placement/source IDs
identify records.  This module never edits an authoring or runtime input.
"""

from __future__ import annotations

import argparse
import json
import math
import re
import shlex
import struct
import sys
import tempfile
import unittest
from dataclasses import dataclass, replace
from pathlib import Path, PurePosixPath, PureWindowsPath
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))
from Tools.ModelAssetConverter.cook_wmodel_geometry_contract import parse_legacy_wmodel


AREA = "LV_LOBBY_CLASSSELECT_SL00"
MODEL_SCALE = 0.01
D24_MAX = (1 << 24) - 1
AREA_EPSILON = 1e-12
UINT64_MAX = (1 << 64) - 1
IDENTIFIER = re.compile(r"^[A-Za-z0-9_.:-]+$")
Vec = tuple[float, ...]
Matrix = tuple[Vec, ...]
Triangle = tuple[Vec, Vec, Vec]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def f32(value: float) -> float:
    try:
        result = struct.unpack("<f", struct.pack("<f", value))[0]
    except (OverflowError, struct.error) as error:
        raise ValueError("value cannot be represented as finite float32") from error
    require(math.isfinite(result), "value cannot be represented as finite float32")
    return result


def finite_vector(values: Any, count: int, label: str) -> Vec:
    require(isinstance(values, (list, tuple)) and len(values) == count,
            f"{label}: expected {count} numbers")
    require(all(isinstance(v, (int, float)) and not isinstance(v, bool)
                for v in values), f"{label}: nonnumeric value")
    try:
        result = tuple(float(v) for v in values)
    except OverflowError as error:
        raise ValueError(f"{label}: number exceeds float range") from error
    for v in result:
        f32(v)
    return result


def identity() -> Matrix:
    return tuple(tuple(float(i == j) for j in range(4)) for i in range(4))


def multiply_row(vector: Vec, matrix: Matrix, use_float32: bool = False) -> Vec:
    cast = f32 if use_float32 else float
    result = []
    for column in range(4):
        value = cast(0)
        for row in range(4):
            value = cast(value + cast(cast(vector[row]) * cast(matrix[row][column])))
        result.append(value)
    return tuple(result)


@dataclass(frozen=True)
class Placement:
    placement_id: str
    source_id: str
    source_level: str
    transform_source: str
    asset_id: str
    position: Vec
    quaternion: Vec
    scale: Vec
    visible: bool


def world_matrix(placement: Placement, use_float32: bool = False) -> Matrix:
    """DirectX row-vector S*R*T; map candidates use the Origin anchor."""
    finite_vector(placement.position, 3, "position")
    finite_vector(placement.quaternion, 4, "quaternion")
    finite_vector(placement.scale, 3, "signed scale")
    cast = f32 if use_float32 else float
    q = tuple(cast(v) for v in placement.quaternion)
    length = math.sqrt(sum(v * v for v in q))
    require(math.isfinite(length) and length > 1e-12, "zero/invalid quaternion")
    x, y, z, w = (cast(v / length) for v in q)
    rows = (
        (1 - 2*y*y - 2*z*z, 2*x*y + 2*z*w, 2*x*z - 2*y*w),
        (2*x*y - 2*z*w, 1 - 2*x*x - 2*z*z, 2*y*z + 2*x*w),
        (2*x*z + 2*y*w, 2*y*z - 2*x*w, 1 - 2*x*x - 2*y*y),
    )
    require(all(v != 0 for v in placement.scale), "zero signed scale")
    result = [tuple(cast(cast(v) * cast(placement.scale[i])) for v in row)
              + (0.0,) for i, row in enumerate(rows)]
    result.append(tuple(cast(v) for v in placement.position) + (1.0,))
    return tuple(result)


def transform_vertices(vertices: Any, placement: Placement,
                       use_float32: bool = False) -> list[Vec]:
    cast = f32 if use_float32 else float
    matrix = world_matrix(placement, use_float32)
    return [multiply_row(tuple(cast(cast(v) * cast(MODEL_SCALE))
                               for v in finite_vector(vertex[:3], 3, "model position")) + (1.0,), matrix,
                         use_float32)[:3] for vertex in vertices]


def cross(a: Vec, b: Vec) -> float:
    return a[0]*b[1] - a[1]*b[0]


def sub(a: Vec, b: Vec) -> Vec:
    return tuple(x-y for x, y in zip(a, b))


def polygon_area(polygon: list[Vec]) -> float:
    if len(polygon) < 3:
        return 0.0
    origin = polygon[0]
    return abs(sum(cross(sub(a, origin), sub(b, origin))
                   for a, b in zip(polygon, polygon[1:] + polygon[:1]))) * 0.5


def bounds(polygon: Any) -> tuple[float, float, float, float]:
    return (min(p[0] for p in polygon), min(p[1] for p in polygon),
            max(p[0] for p in polygon), max(p[1] for p in polygon))


def aabb_overlap(a: Any, b: Any) -> bool:
    return a[0] < b[2] and b[0] < a[2] and a[1] < b[3] and b[1] < a[3]


def convex_intersection(subject: list[Vec], clip: list[Vec]) -> list[Vec]:
    """Convex polygon intersection. A touching edge has no surface area."""
    if polygon_area(subject) <= AREA_EPSILON or polygon_area(clip) <= AREA_EPSILON:
        return []
    if not aabb_overlap(bounds(subject), bounds(clip)):
        return []
    orientation = sum(cross(sub(a, clip[0]), sub(b, clip[0]))
                      for a, b in zip(clip, clip[1:] + clip[:1]))
    sign = 1.0 if orientation > 0 else -1.0
    polygon = subject[:]
    for start, end in zip(clip, clip[1:] + clip[:1]):
        output = []
        edge = sub(end, start)
        for a, b in zip(polygon, polygon[1:] + polygon[:1]):
            da, db = sign*cross(edge, sub(a, start)), sign*cross(edge, sub(b, start))
            if da >= 0:
                output.append(a)
            if (da >= 0) != (db >= 0):
                t = da / (da-db)
                output.append(tuple(x + t*(y-x) for x, y in zip(a, b)))
        polygon = output
        if not polygon:
            return []
    return polygon if polygon_area(polygon) > AREA_EPSILON else []


def interpolate_height(triangle: Triangle, point: Vec) -> float:
    """Barycentric plane height; coordinates here are (x, z, height)."""
    a, b, c = triangle
    u, v, p = sub(b[:2], a[:2]), sub(c[:2], a[:2]), sub(point, a[:2])
    determinant = cross(u, v)
    require(abs(determinant) > AREA_EPSILON, "degenerate projected triangle")
    beta, gamma = cross(p, v)/determinant, cross(u, p)/determinant
    return a[2] + beta*(b[2]-a[2]) + gamma*(c[2]-a[2])


def xz_triangle(triangle: Triangle) -> Triangle:
    return tuple((p[0], p[2], p[1]) for p in triangle)


def height_report(left: Triangle, right: Triangle) -> dict[str, Any] | None:
    a, b = xz_triangle(left), xz_triangle(right)
    polygon = convex_intersection([p[:2] for p in a], [p[:2] for p in b])
    if not polygon:
        return None
    delta = [interpolate_height(a, p)-interpolate_height(b, p) for p in polygon]
    low, high = min(delta), max(delta)
    return {"xzPolygonMeters": polygon, "xzAreaSquareMeters": polygon_area(polygon),
            "signedYDeltaRangeMeters": [low, high],
            "minimumAbsoluteYGapMeters": 0.0 if low <= 0 <= high else min(abs(low), abs(high)),
            "maximumAbsoluteYGapMeters": max(abs(low), abs(high))}


def clip_to_ndc(triangle: Triangle, view: Matrix, projection: Matrix,
                use_float32: bool = False) -> list[Vec]:
    polygon = [multiply_row(multiply_row(p + (1.0,), view, use_float32),
                            projection, use_float32) for p in triangle]
    # D3D clip volume: -w <= x,y <= w and 0 <= z <= w.
    for plane in ((1,0,0,1), (-1,0,0,1), (0,1,0,1), (0,-1,0,1),
                  (0,0,1,0), (0,0,-1,1)):
        output = []
        for a, b in zip(polygon, polygon[1:] + polygon[:1]):
            da, db = sum(x*y for x, y in zip(a, plane)), sum(x*y for x, y in zip(b, plane))
            if da >= 0:
                output.append(a)
            if (da >= 0) != (db >= 0):
                t = da/(da-db)
                output.append(tuple(x + t*(y-x) for x, y in zip(a, b)))
        polygon = output
        if not polygon:
            return []
    return [tuple(v/p[3] for v in p[:3]) for p in polygon if p[3] > 1e-12]


def plane_triangle(polygon: list[Vec]) -> Triangle:
    for index in range(1, len(polygon)-1):
        triangle = (polygon[0], polygon[index], polygon[index+1])
        if polygon_area([p[:2] for p in triangle]) > AREA_EPSILON:
            return triangle
    raise ValueError("projected polygon is degenerate")


def depth_report(left: Triangle, right: Triangle, view: Matrix, projection: Matrix,
                 use_float32: bool = False) -> dict[str, Any]:
    a, b = clip_to_ndc(left, view, projection, use_float32), clip_to_ndc(right, view, projection, use_float32)
    common = convex_intersection([p[:2] for p in a], [p[:2] for p in b])
    if not common:
        return {"sameRayOverlap": False}
    plane_a, plane_b = plane_triangle(a), plane_triangle(b)
    center = tuple(sum(p[i] for p in common)/len(common) for i in range(2))
    samples = common + [center]
    pairs = [(interpolate_height(plane_a, p), interpolate_height(plane_b, p)) for p in samples]
    delta = [(x-y)*D24_MAX for x, y in pairs]
    low, high = min(delta), max(delta)
    return {"sameRayOverlap": True, "screenOverlapAreaNdcSquared": polygon_area(common),
            "signedDepthDeltaRangeD24Steps": [low, high],
            "minimumAbsoluteDepthGapD24Steps": 0.0 if low <= 0 <= high else min(abs(low), abs(high)),
            "equalQuantizedDepthSamples": sum(round(x*D24_MAX) == round(y*D24_MAX) for x, y in pairs),
            "sampleCount": len(samples)}


def uint64_string(value: Any, label: str) -> str:
    require(isinstance(value, str) and re.fullmatch(r"[1-9][0-9]*", value) is not None,
            f"{label}: expected a nonzero decimal uint64 string")
    require(int(value) <= UINT64_MAX, f"{label}: uint64 overflow")
    return value


def read_placements(path: Path, area: str) -> dict[str, Placement]:
    lines = path.read_text(encoding="utf-8-sig").splitlines()
    require(bool(lines), f"empty placement document: {path}")
    header = shlex.split(lines[0])
    require(len(header) == 4 and header[:3] == ["LOSTARK_MAP_PLACEMENTS", "2", area]
            and header[3].isdigit() and 0 < int(header[3]) <= 65536
            and int(header[3]) == len(lines)-1, f"invalid placement header/count: {path}")
    result, ids = {}, set()
    for line in lines[1:]:
        row = shlex.split(line)
        require(len(row) == 16, f"invalid placement row in {path}")
        placement_id = uint64_string(row[0], "placementId")
        require(placement_id not in ids and row[1] not in result,
                f"duplicate placement/source ID: {row[1]}")
        require(all(IDENTIFIER.fullmatch(v) for v in row[1:5]),
                f"invalid source/asset ID: {row[1]}")
        values = finite_vector([float(v) for v in row[5:15]], 10, row[1])
        require(sum(v*v for v in values[3:7]) > 1e-24 and all(v != 0 for v in values[7:]),
                f"invalid quaternion/signed scale: {row[1]}")
        require(row[15] in ("0", "1"), f"invalid visibility: {row[1]}")
        result[row[1]] = Placement(placement_id, *row[1:5], values[:3], values[3:7],
                                    values[7:], row[15] == "1")
        ids.add(placement_id)
    return result


def read_catalog(path: Path, area: str) -> dict[str, tuple[str, str]]:
    lines = path.read_text(encoding="utf-8-sig").splitlines()
    require(bool(lines), f"empty catalog: {path}")
    header = shlex.split(lines[0])
    require(len(header) == 4 and header[:3] == ["LOSTARK_MAP_ASSET_CATALOG", "4", area]
            and header[3].isdigit() and int(header[3]) == len(lines)-1,
            f"diagnostic requires a version 4 single catalog: {path}")
    result = {}
    for line in lines[1:]:
        row = shlex.split(line)
        require(len(row) >= 13 and IDENTIFIER.fullmatch(row[0]) is not None,
                f"invalid catalog row: {path}")
        require(row[0] not in result, f"duplicate catalog asset: {row[0]}")
        result[row[0]] = (row[2], row[7])
    return result


def resolve_resource(root: Path, relative: str, asset_id: str) -> Path:
    normalized = relative.replace("\\", "/")
    path = PurePosixPath(normalized)
    require(not path.is_absolute() and not PureWindowsPath(relative).drive
            and ".." not in path.parts and path.suffix.lower() == ".wmodel",
            f"invalid resource path for {asset_id}: {relative}")
    resolved = (root / normalized).resolve()
    require(resolved.is_relative_to(root.resolve()), f"resource root escape: {asset_id}")
    require(resolved.is_file(), f"missing model for {asset_id}: {resolved}")
    return resolved


BRIDGE = "MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM"
FLOOR02 = "MAP_F442BCF81552_LV_ELG_ARYANORB_FLOOR02_SM_01"
FLOOR22 = "MAP_E5357DD78673_BG_ELG_ARYANORB_FLOOR22_SM"
MAGICFLOOR = "MAP_FB0D0FFCBE97_BG_GDOGODS_MAGICFLOOR03D_SM"
# Explicit witnesses from the 2026-08-28 investigation, not an all-map search.
CANDIDATES = (
    (402, 405, FLOOR02, 1, (-762.85, 197.60)),
    (458, 474, BRIDGE, 0, (-773.95, 197.30)),
    (442, 458, BRIDGE, 0, (-772.00, 195.20)),
    (425, 427, FLOOR22, 0, (-770.20, 187.70)),
    (435, 436, FLOOR22, 0, (-762.10, 199.25)),
    (469, 471, MAGICFLOOR, 0, (-778.60, 198.20)),
)


def source_id(export: int) -> str:
    return f"{AREA}:export:{export}"


def validate_preservation(imported: dict[str, Placement], authored: dict[str, Placement],
                          published: dict[str, Placement], expected_y: dict[str, float]) -> None:
    require(imported.keys() == authored.keys() == published.keys(),
            "placement/source ID set changed between imported, authoring and runtime")
    require(authored == published, "authoring/runtime placement contents differ")
    require(expected_y.keys() <= imported.keys(), "expected Y change names a missing source ID")
    for key, before in imported.items():
        after = authored[key]
        expected_position = (before.position[0], before.position[1] + expected_y.get(key, 0.0),
                             before.position[2])
        require(replace(after, position=before.position) == before
                and after.position[0] == expected_position[0]
                and after.position[2] == expected_position[2]
                and math.isclose(after.position[1], expected_position[1], abs_tol=1e-9, rel_tol=0),
                f"unapproved placement change: {key}")


def mesh_triangles(mesh: Any, placement: Placement,
                   use_float32: bool = False) -> list[Triangle]:
    vertices = transform_vertices(mesh.vertices, placement, use_float32)
    return [tuple(vertices[i] for i in mesh.indices[start:start+3])
            for start in range(0, len(mesh.indices), 3)]


def spatial_cells(box: Any) -> list[tuple[int, int]]:
    # One-metre cells keep the candidate-pair join bounded without N^2 map scans.
    x0, z0, x1, z1 = (math.floor(v) for v in box)
    require((x1-x0+1)*(z1-z0+1) <= 4096, "triangle exceeds diagnostic spatial capacity")
    return [(x, z) for x in range(x0, x1+1) for z in range(z0, z1+1)]


def scan_pair(left: list[Triangle], right: list[Triangle],
              threshold: float = 0.0002) -> dict[str, Any]:
    def selected(triangles: list[Triangle]) -> dict[int, Any]:
        return {i: (triangle, bounds([p[:2] for p in xz_triangle(triangle)]))
                for i, triangle in enumerate(triangles)
                if max(p[1] for p in triangle) >= -143.3
                and min(p[1] for p in triangle) <= -142.4
                and polygon_area([p[:2] for p in xz_triangle(triangle)]) > AREA_EPSILON}
    a, b = selected(left), selected(right)
    grid: dict[tuple[int, int], list[int]] = {}
    for index, (_, box) in b.items():
        for cell in spatial_cells(box):
            grid.setdefault(cell, []).append(index)
    checked = overlaps = close = 0
    examples = []
    for index, (triangle, box) in a.items():
        candidates = {i for cell in spatial_cells(box) for i in grid.get(cell, ())}
        for other in sorted(candidates):
            if not aabb_overlap(box, b[other][1]):
                continue
            checked += 1
            require(checked <= 500000, "candidate pair exceeds diagnostic comparison capacity")
            result = height_report(triangle, b[other][0])
            if result is None:
                continue
            overlaps += 1
            if result["maximumAbsoluteYGapMeters"] > threshold:
                continue
            close += 1
            examples.append({"triangleIndices": [index, other], **result})
            examples.sort(key=lambda r: (r["maximumAbsoluteYGapMeters"], -r["xzAreaSquareMeters"]))
            del examples[12:]
    return {"heightBandMeters": [-143.3, -142.4], "maximumNearPlaneGapMeters": threshold,
            "aabbCandidateComparisons": checked, "triangleOverlapCount": overlaps,
            "nearPlaneTrianglePairCount": close, "nearPlaneExamples": examples,
            "unlistedNearPlanePairs": close-len(examples)}


def top_at_point(triangles: list[Triangle], point: Vec) -> tuple[int, float] | None:
    hits = []
    for index, triangle in enumerate(triangles):
        projected = xz_triangle(triangle)
        a, b, c = (p[:2] for p in projected)
        u, v, delta = sub(b, a), sub(c, a), sub(point, a)
        determinant = cross(u, v)
        if abs(determinant) <= AREA_EPSILON:
            continue
        beta, gamma = cross(delta, v)/determinant, cross(u, delta)/determinant
        if min(beta, gamma, 1-beta-gamma) < -1e-8:
            continue
        y = interpolate_height(projected, point)
        if -143.3 <= y <= -142.4:
            hits.append((index, y))
    return max(hits, key=lambda hit: hit[1]) if hits else None


def nonsingular(matrix: Matrix) -> bool:
    rows = [list(row) for row in matrix]
    for column in range(4):
        pivot = max(range(column, 4), key=lambda i: abs(rows[i][column]))
        if abs(rows[pivot][column]) <= 1e-15:
            return False
        rows[column], rows[pivot] = rows[pivot], rows[column]
        for row in range(column+1, 4):
            factor = rows[row][column]/rows[column][column]
            for j in range(column+1, 4):
                rows[row][j] -= factor*rows[column][j]
    return True


def camera_matrix(value: Any, label: str) -> Matrix:
    flat = tuple(f32(v) for v in finite_vector(value, 16, label))
    matrix = tuple(flat[i:i+4] for i in range(0, 16, 4))
    require(nonsingular(matrix), f"{label}: singular matrix")
    return matrix


def validate_projection(matrix: Matrix) -> None:
    """The current CCamera producer is forward-Z LH perspective, not reverse Z."""
    zero_positions = ((0,1), (0,2), (0,3), (1,0), (1,2), (1,3), (3,0), (3,1), (3,3))
    require(all(abs(matrix[i][j]) <= 1e-6 for i, j in zero_positions)
            and abs(matrix[2][3]-1.0) <= 1e-6
            and matrix[0][0] > 0 and matrix[1][1] > 0
            and matrix[2][2] > 1 and matrix[3][2] < 0,
            "camera projection is not the current forward-Z LH perspective contract")
    near = -matrix[3][2]/matrix[2][2]
    far = -matrix[3][2]/(matrix[2][2]-1)
    require(math.isfinite(far) and 0 < near < far, "invalid camera near/far projection")


def unique_json_object(pairs: list[Any]) -> dict[str, Any]:
    result = {}
    for key, value in pairs:
        require(key not in result, f"duplicate JSON key: {key}")
        result[key] = value
    return result


def invalid_json_constant(value: str) -> None:
    raise ValueError(f"nonfinite JSON constant: {value}")


def read_camera_log(path: Path, area: str) -> list[dict[str, Any]]:
    require(path.stat().st_size <= 64*1024*1024, "camera capture exceeds 64 MiB diagnostic limit")
    document = json.loads(path.read_text(encoding="utf-8-sig"), object_pairs_hook=unique_json_object,
                          parse_constant=invalid_json_constant)
    require(isinstance(document, dict) and type(document.get("formatVersion")) is int
            and document["formatVersion"] == 1 and document.get("kind") == "LOSTARK_MAP_RENDER_CAPTURE"
            and document.get("matrixConvention") == "row-vector-row-major-d3d-lh-z01"
            and document.get("areaId") == area, "camera capture version/kind/convention/area mismatch")
    frames = document.get("frames")
    require(isinstance(frames, list) and 0 < len(frames) <= 512,
            "camera capture must contain 1..512 frames")
    result, previous_id, previous_revision, previous_matrices = [], 0, 0, None
    for frame in frames:
        require(isinstance(frame, dict), "camera frame must be an object")
        frame_id = uint64_string(frame.get("frameId"), "frameId")
        require(int(frame_id) > previous_id, "duplicate or decreasing frameId")
        require(type(frame.get("cameraMismatchCount")) is int and frame["cameraMismatchCount"] == 0,
                f"frame {frame_id}: cameraMismatchCount must be zero")
        require(isinstance(frame.get("counters"), dict)
                and isinstance(frame.get("placements"), list)
                and isinstance(frame.get("draws"), list),
                f"frame {frame_id}: counters/placements/draws structure is missing or invalid")
        camera = frame.get("camera")
        require(isinstance(camera, dict) and camera.get("valid") is True,
                f"frame {frame_id}: camera is missing or invalid")
        revision = uint64_string(camera.get("revision"), "camera revision")
        view = camera_matrix(camera.get("view"), "camera view")
        projection = camera_matrix(camera.get("projection"), "camera projection")
        require(all(abs(view[i][3]) <= 1e-6 for i in range(3)) and abs(view[3][3]-1) <= 1e-6,
                "camera view is not an affine row-vector matrix")
        validate_projection(projection)
        matrices = (view, projection)
        require(int(revision) >= previous_revision, "camera revision decreased")
        require(int(revision) != previous_revision or matrices == previous_matrices,
                "camera matrices changed without increasing revision")
        result.append({"frameId": frame_id, "revision": revision,
                       "view": view, "projection": projection})
        previous_id, previous_revision, previous_matrices = int(frame_id), int(revision), matrices
    return result


def diagnose_resources(repo: Path, resource_root: Path, area: str,
                       camera_log: Path | None, approved_y: dict[str, float]) -> dict[str, Any]:
    require(area == AREA, f"no recorded surface candidates for Area {area}")
    require(resource_root.is_dir(), f"resource root is missing: {resource_root}")
    base = repo / "Data/Maps"
    imported_path = base / "Imported" / area / (area + ".mapplacements")
    authored_path = base / "Authoring" / area / (area + ".mapplacements")
    runtime_path = repo / "Client/Bin/DataFiles/Map" / (area + ".mapplacements")
    imported, authored, published = (read_placements(path, area)
                                     for path in (imported_path, authored_path, runtime_path))
    expected_y = {source_id(490): -0.002, source_id(495): -0.002}
    allowed = {source_id(e) for candidate in CANDIDATES for e in candidate[:2]}
    require(approved_y.keys() <= allowed, "expected Y change is not a recorded candidate source ID")
    expected_y.update(approved_y)
    validate_preservation(imported, authored, published, expected_y)
    require(authored_path.read_bytes() == runtime_path.read_bytes(), "authoring/runtime bytes differ")
    require(len(imported) == 803, "Character Select baseline must retain 803 placements")
    catalog = read_catalog(repo / "Client/Bin/DataFiles/Map" / (area + ".mapassets"), area)
    require(catalog == read_catalog(base / "Imported" / area / (area + ".mapassets"), area),
            "imported/runtime model resource or anchor differs")
    require(len(catalog) == 55, "Character Select baseline must retain 55 catalog assets")
    require(all(p.asset_id in catalog for p in authored.values()), "placement references an unknown asset")
    scope = [p for p in authored.values() if -792 <= p.position[0] <= -750 and 158 <= p.position[2] <= 218]
    require(len(scope) == 779 and len({p.asset_id for p in scope}) == 54,
            "Character Select product origin scope changed")
    frames = read_camera_log(camera_log, area) if camera_log is not None else []
    models, output = {}, []
    for left_id, right_id, asset, mesh_index, point in CANDIDATES:
        require(asset in catalog, f"candidate asset missing: {asset}")
        relative, anchor = catalog[asset]
        require(anchor == "Origin", f"candidate anchor changed; diagnostic cannot assume Origin: {asset}")
        if asset not in models:
            path = resolve_resource(resource_root, relative, asset)
            try:
                models[asset] = parse_legacy_wmodel(path.read_bytes())[2]
            except ValueError as error:
                raise ValueError(f"invalid model for {asset}: {error}") from error
        require(mesh_index < len(models[asset]), f"candidate mesh missing: {asset} / {mesh_index}")
        placements = [authored[source_id(e)] for e in (left_id, right_id)]
        require(all(p.asset_id == asset and p.visible for p in placements), f"candidate identity/visibility changed: {asset}")
        mesh = models[asset][mesh_index]
        triangles = [mesh_triangles(mesh, p) for p in placements]
        report = {"assetId": asset, "modelResourceId": relative, "meshIndex": mesh_index,
                  "placementIds": [p.placement_id for p in placements],
                  "sourcePlacementIds": [p.source_id for p in placements],
                  "xzScan": scan_pair(*triangles), "priorSampleXZ": point}
        hits = [top_at_point(t, point) for t in triangles]
        if all(hit is not None for hit in hits):
            left_triangle, right_triangle = (triangles[i][hits[i][0]] for i in range(2))
            report["priorSample"] = {"triangleIndices": [h[0] for h in hits],
                "worldY": [h[1] for h in hits], "absoluteYGapMeters": abs(hits[0][1]-hits[1][1]),
                "overlap": height_report(left_triangle, right_triangle)}
            if frames:
                approximate = [mesh_triangles(mesh, p, True) for p in placements]
                report["cameraDepth"] = [{"frameId": frame["frameId"], "cameraRevision": frame["revision"],
                    "float64Reference": depth_report(left_triangle, right_triangle, frame["view"], frame["projection"]),
                    "float32Approximation": depth_report(approximate[0][hits[0][0]], approximate[1][hits[1][0]],
                        frame["view"], frame["projection"], True)} for frame in frames]
        else:
            report["priorSample"] = {"status": "no_surface_in_recorded_height_band"}
        output.append(report)
    return {"formatVersion": 1, "kind": "LOSTARK_MAP_SURFACE_DEPTH_DIAGNOSTIC", "areaId": area,
            "status": "diagnostic_only", "cameraDepthStatus": "evaluated_cpu" if frames else "not_requested",
            "cameraDepthScope": "recorded witness triangle pairs; no scene occlusion or pixel sampling",
            "arithmeticNote": "float32 is a CPU approximation; GPU FMA/rasterizer rounding is not reproduced",
            "placementPreservation": {"imported": len(imported), "authoring": len(authored), "runtime": len(published),
                "centralCorrectionScope": "placement Y preservation; geometry scan covers the six edge candidate pairs",
                "productScopePlacements": len(scope), "productScopeAssets": len({p.asset_id for p in scope}),
                "centralCorrections": [{"sourcePlacementId": source_id(e),
                    "placementId": authored[source_id(e)].placement_id,
                    "importedY": imported[source_id(e)].position[1], "authoredY": authored[source_id(e)].position[1],
                    "runtimeY": published[source_id(e)].position[1],
                    "deltaFromImportedMeters": authored[source_id(e)].position[1]-imported[source_id(e)].position[1]}
                    for e in (490, 495)]},
            "candidatePairs": output}


def run_cli() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--resource-root", type=Path, required=True)
    parser.add_argument("--area-id", required=True)
    parser.add_argument("--repo-root", type=Path, default=REPO_ROOT)
    parser.add_argument("--camera-log", type=Path)
    parser.add_argument("--expected-y-change", action="append", default=[], metavar="SOURCE_ID=METRES",
                        help="explicit approved delta from Imported, only for a recorded candidate")
    args = parser.parse_args()
    try:
        expected = {}
        for entry in args.expected_y_change:
            key, value = entry.rsplit("=", 1)
            require(key not in expected, f"duplicate approved Y change: {key}")
            expected[key] = float(value)
            f32(expected[key])
        report = diagnose_resources(args.repo_root, args.resource_root, args.area_id, args.camera_log, expected)
        print(json.dumps(report, ensure_ascii=False, allow_nan=False, indent=2))
        return 0
    except (OSError, ValueError, KeyError, struct.error) as error:
        print(json.dumps({"status": "error", "areaId": args.area_id, "reason": str(error)},
                         ensure_ascii=False), file=sys.stderr)
        return 2


class SurfaceDepthContractTests(unittest.TestCase):
    @staticmethod
    def placement(export: int = 402) -> Placement:
        return Placement(str(UINT64_MAX-export), source_id(export), AREA, "actor", FLOOR02,
                         (0.0, 0.0, 0.0), (0.0, 0.0, 0.0, 1.0), (1.0, 1.0, 1.0), True)

    @staticmethod
    def perspective(near: float = 0.1, far: float = 2000.0) -> Matrix:
        return ((1.0,0.0,0.0,0.0), (0.0,1.0,0.0,0.0),
                (0.0,0.0,far/(far-near),1.0), (0.0,0.0,-far*near/(far-near),0.0))

    @staticmethod
    def screen_triangle(z: float) -> Triangle:
        return ((-1.0,-1.0,z), (1.0,-1.0,z), (-1.0,1.0,z))

    def test_coplanar_intersection_is_positive_area(self) -> None:
        a = ((0.0,0.0,0.0), (2.0,0.0,0.0), (0.0,0.0,2.0))
        b = ((0.5,0.0,0.5), (2.5,0.0,0.5), (0.5,0.0,2.5))
        report = height_report(a, b)
        self.assertAlmostEqual(report["xzAreaSquareMeters"], 0.5)
        self.assertEqual(report["maximumAbsoluteYGapMeters"], 0)
        reversed_report = height_report(a, tuple(reversed(b)))
        self.assertAlmostEqual(report["xzAreaSquareMeters"], reversed_report["xzAreaSquareMeters"])
        self.assertEqual(report["signedYDeltaRangeMeters"], reversed_report["signedYDeltaRangeMeters"])

    def test_contact_disjoint_and_vertical_are_not_surface_overlap(self) -> None:
        a = ((0.0,0.0,0.0), (1.0,0.0,0.0), (0.0,0.0,1.0))
        for b in (((1.0,0.0,0.0), (2.0,0.0,0.0), (1.0,0.0,1.0)),
                  ((5.0,0.0,5.0), (6.0,0.0,5.0), (5.0,0.0,6.0)),
                  ((0.0,0.0,0.0), (0.0,1.0,0.0), (0.0,0.0,1.0))):
            with self.subTest(triangle=b):
                self.assertIsNone(height_report(a, b))

    def test_height_separation_and_crossing_planes(self) -> None:
        a = ((0.0,0.0,0.0), (2.0,0.0,0.0), (0.0,0.0,2.0))
        separated = tuple((x, 0.3, z) for x, _, z in a)
        self.assertAlmostEqual(height_report(a, separated)["minimumAbsoluteYGapMeters"], 0.3)
        crossing = ((0.0,-1.0,0.0), (2.0,1.0,0.0), (0.0,0.0,2.0))
        self.assertEqual(height_report(a, crossing)["signedYDeltaRangeMeters"], [-1.0, 1.0])

    def test_large_coordinates_do_not_cancel_polygon_area(self) -> None:
        a = ((-772.0,-143.0,197.0), (-771.0,-143.0,197.0), (-772.0,-143.0,198.0))
        self.assertAlmostEqual(height_report(a, a)["xzAreaSquareMeters"], 0.5)

    def test_signed_scale_rotation_and_translation_follow_directx_order(self) -> None:
        p = replace(self.placement(), position=(10.0,3.0,-20.0), scale=(-2.0,3.0,0.5),
                    quaternion=(0.0, math.sqrt(0.5), 0.0, math.sqrt(0.5)))
        points = transform_vertices([(100.0,0.0,0.0), (0.0,100.0,0.0), (0.0,0.0,100.0)], p)
        for actual, expected in zip(points, ((10,3,-18), (10,6,-20), (10.5,3,-20))):
            for a, b in zip(actual, expected):
                self.assertAlmostEqual(a, b)

    def test_invalid_transform_is_not_normalized_to_identity(self) -> None:
        for p in (replace(self.placement(), quaternion=(0,0,0,0)),
                  replace(self.placement(), scale=(1,0,1)),
                  replace(self.placement(), position=(math.nan,0,0))):
            with self.subTest(placement=p), self.assertRaises(ValueError):
                world_matrix(p)

    def test_float32_path_preserves_rounding_difference(self) -> None:
        p = replace(self.placement(), position=(-772.0,-142.7,197.0))
        exact = transform_vertices([(1.0,0.0,0.0)], p)[0]
        approximate = transform_vertices([(1.0,0.0,0.0)], p, True)[0]
        self.assertNotEqual(exact[0], approximate[0])
        self.assertLess(abs(exact[0]-approximate[0]), 1e-4)
        self.assertTrue(all(f32(v) == v for v in approximate))

    def test_depth_compares_the_same_camera_ray(self) -> None:
        a, b = self.screen_triangle(10), self.screen_triangle(10.002)
        report = depth_report(a, b, identity(), self.perspective())
        self.assertTrue(report["sameRayOverlap"])
        self.assertGreater(report["minimumAbsoluteDepthGapD24Steps"], 1)
        coplanar = depth_report(a, a, identity(), self.perspective())
        self.assertEqual(coplanar["equalQuantizedDepthSamples"], coplanar["sampleCount"])

    def test_depth_precision_decreases_with_distance(self) -> None:
        near = depth_report(self.screen_triangle(10), self.screen_triangle(10.0001), identity(), self.perspective())
        far = depth_report(self.screen_triangle(100), self.screen_triangle(100.0001), identity(), self.perspective())
        self.assertGreater(near["minimumAbsoluteDepthGapD24Steps"], 1)
        self.assertLess(far["minimumAbsoluteDepthGapD24Steps"], 1)

    def test_depth_clips_near_far_and_behind_camera(self) -> None:
        for z in (-1.0, 0.05, 3000.0):
            with self.subTest(z=z):
                t = self.screen_triangle(z)
                self.assertFalse(depth_report(t, t, identity(), self.perspective())["sameRayOverlap"])
        crossing = ((-0.05,-0.05,0.05), (0.05,-0.05,0.2), (0.0,0.05,0.2))
        self.assertTrue(depth_report(crossing, crossing, identity(), self.perspective())["sameRayOverlap"])

    def test_same_depth_in_different_screen_regions_is_not_overlap(self) -> None:
        a = self.screen_triangle(10)
        b = tuple((x+4, y, z) for x, y, z in a)
        self.assertFalse(depth_report(a, b, identity(), self.perspective())["sameRayOverlap"])

    def test_spatial_join_does_not_compare_disjoint_triangles(self) -> None:
        triangle = ((0.0,-142.7,0.0), (1.0,-142.7,0.0), (0.0,-142.7,1.0))
        left = [tuple((x+10*i, y, z) for x, y, z in triangle) for i in range(30)]
        report = scan_pair(left, [triangle])
        self.assertEqual(report["aabbCandidateComparisons"], 1)
        self.assertEqual(report["nearPlaneTrianglePairCount"], 1)

    def test_preservation_accepts_only_approved_y_changes(self) -> None:
        before = self.placement()
        after = replace(before, position=(0.0,-0.002,0.0))
        imported, authored = {before.source_id: before}, {after.source_id: after}
        validate_preservation(imported, authored, authored, {before.source_id: -0.002})
        for mutation in (replace(after, position=(1,-0.002,0)), replace(after, visible=False),
                         replace(after, asset_id="OTHER"), replace(after, scale=(2,1,1)),
                         replace(after, placement_id="1")):
            with self.subTest(mutation=mutation), self.assertRaises(ValueError):
                changed = {mutation.source_id: mutation}
                validate_preservation(imported, changed, changed, {before.source_id: -0.002})
        with self.assertRaisesRegex(ValueError, "contents differ"):
            validate_preservation(imported, authored, imported, {before.source_id: -0.002})

    def test_uint64_identity_preserves_large_ids_as_strings(self) -> None:
        self.assertEqual(uint64_string(str(UINT64_MAX), "id"), str(UINT64_MAX))
        for invalid in (UINT64_MAX, "0", "01", str(UINT64_MAX+1), "-1"):
            with self.subTest(invalid=invalid), self.assertRaises(ValueError):
                uint64_string(invalid, "id")

    def test_placement_parser_rejects_wrong_version_duplicate_and_nonfinite(self) -> None:
        row = f'{UINT64_MAX} "{source_id(402)}" "{AREA}" "actor" "{FLOOR02}" 0 0 0 0 0 0 1 1 1 1 1'
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "placements"
            path.write_text(f'LOSTARK_MAP_PLACEMENTS 2 "{AREA}" 1\n{row}\n', encoding="utf-8")
            self.assertEqual(read_placements(path, AREA)[source_id(402)].placement_id, str(UINT64_MAX))
            for text in (f'LOSTARK_MAP_PLACEMENTS 1 "{AREA}" 1\n{row}\n',
                         f'LOSTARK_MAP_PLACEMENTS 2 "{AREA}" 2\n{row}\n{row}\n',
                         f'LOSTARK_MAP_PLACEMENTS 2 "{AREA}" 1\n{row.replace(" 0 0 0 ", " nan 0 0 ", 1)}\n'):
                path.write_text(text, encoding="utf-8")
                with self.assertRaises(ValueError):
                    read_placements(path, AREA)

    def test_missing_or_escaping_resource_is_an_error(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            for relative in ("Map/missing.wmodel", "../outside.wmodel", "C:/outside.wmodel", "/outside.wmodel"):
                with self.subTest(relative=relative), self.assertRaisesRegex(ValueError, "ASSET"):
                    resolve_resource(Path(directory), relative, "ASSET")

    def capture(self) -> dict[str, Any]:
        return {"formatVersion": 1, "kind": "LOSTARK_MAP_RENDER_CAPTURE", "areaId": AREA,
                "matrixConvention": "row-vector-row-major-d3d-lh-z01", "frames": [
                    {"frameId": "9007199254740993", "cameraMismatchCount": 0,
                     "camera": {"valid": True, "revision": "1", "view": [v for row in identity() for v in row],
                                "projection": [v for row in self.perspective() for v in row]},
                     "counters": {}, "placements": [], "draws": []}]}

    def load_capture(self, document: dict[str, Any]) -> list[dict[str, Any]]:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "capture.json"
            path.write_text(json.dumps(document), encoding="utf-8")
            return read_camera_log(path, AREA)

    def test_stationary_camera_reuses_revision_in_different_frames(self) -> None:
        document = self.capture()
        second = json.loads(json.dumps(document["frames"][0]))
        second["frameId"] = "9007199254740994"
        document["frames"].append(second)
        result = self.load_capture(document)
        self.assertEqual([r["revision"] for r in result], ["1", "1"])
        self.assertEqual(result[0]["frameId"], "9007199254740993")

    def test_camera_capture_enforces_version_convention_area_and_frames(self) -> None:
        for key, value in (("formatVersion", 2), ("formatVersion", True), ("kind", "OTHER"),
                           ("matrixConvention", "column-major"), ("areaId", "BERN"), ("frames", [])):
            with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                document = self.capture()
                document[key] = value
                self.load_capture(document)

    def test_camera_failure_never_falls_back(self) -> None:
        mutations = (
            lambda f: f.update(cameraMismatchCount=1),
            lambda f: f.update(frameId="0"),
            lambda f: f.update(frameId=123),
            lambda f: f.update(camera=None),
            lambda f: f.pop("counters"),
            lambda f: f.update(placements={}),
            lambda f: f.update(draws=None),
            lambda f: f["camera"].update(valid=False),
            lambda f: f["camera"].update(revision="0"),
            lambda f: f["camera"].update(view=[0.0]*16),
            lambda f: f["camera"].update(view=[0.0]*15),
            lambda f: f["camera"].update(view=[math.nan]*16),
            lambda f: f["camera"].update(view=[10**400]*16),
            lambda f: f["camera"].update(projection=[v for row in identity() for v in row]),
        )
        for index, mutate in enumerate(mutations):
            with self.subTest(index=index), self.assertRaises(ValueError):
                document = self.capture()
                mutate(document["frames"][0])
                self.load_capture(document)

    def test_frame_and_matrix_revision_ordering(self) -> None:
        first = self.capture()["frames"][0]
        for duplicate_frame, changed_matrix, revision in ((True, False, "1"), (False, True, "1"), (False, False, "0")):
            document = self.capture()
            second = json.loads(json.dumps(first))
            second["frameId"] = first["frameId"] if duplicate_frame else str(int(first["frameId"])+1)
            second["camera"]["revision"] = revision
            if changed_matrix:
                second["camera"]["view"][12] = 1.0
            document["frames"].append(second)
            with self.subTest(duplicate=duplicate_frame, change=changed_matrix, revision=revision), self.assertRaises(ValueError):
                self.load_capture(document)
        second["camera"]["revision"] = "2"
        second["camera"]["view"][12] = 1.0
        self.assertEqual(self.load_capture(document)[1]["revision"], "2")
        document["frames"][0]["camera"]["revision"] = "3"
        with self.assertRaisesRegex(ValueError, "revision decreased"):
            self.load_capture(document)

    def test_duplicate_json_fields_are_not_silently_overwritten(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "capture.json"
            text = json.dumps(self.capture()).replace('"formatVersion": 1',
                                                       '"formatVersion": 2, "formatVersion": 1')
            path.write_text(text, encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "duplicate JSON key"):
                read_camera_log(path, AREA)


if __name__ == "__main__":
    if len(sys.argv) == 1:
        unittest.main()
    else:
        raise SystemExit(run_cli())
