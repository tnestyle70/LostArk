#!/usr/bin/env python3
"""Partition a static fractured Deploy WModel into authored rigid-body pieces.

The Lost Ark Deploy export keeps the fractured wall as one assembled static
mesh.  It does not contain source chunk pivots or a physics graph.  This tool
therefore preserves every source triangle and material, assigns triangles to
deterministic spatial macro shards, recentres each shard around a derived
pivot, and writes standalone static WModels next to the source model.

The result is PROJECT_AUTHORED presentation data.  It must never be described
as source-exact fracture physics even though its visible geometry and
materials come from the exact fractured WModel.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path

sys.dont_write_bytecode = True

import cook_wmodel_geometry_contract as contract


@dataclass(frozen=True)
class Triangle:
    submesh_index: int
    indices: tuple[int, int, int]
    centroid: tuple[float, float, float]
    area: float


@dataclass(frozen=True)
class PieceResult:
    piece_index: int
    path: Path
    pivot: tuple[float, float, float]
    minimum: tuple[float, float, float]
    maximum: tuple[float, float, float]
    triangle_count: int
    vertex_count: int
    index_count: int


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def write_atomic(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(
        dir=path.parent, prefix=path.name + ".", suffix=".tmp", delete=False
    ) as stream:
        temporary = Path(stream.name)
        stream.write(payload)
        stream.flush()
    try:
        temporary.replace(path)
    except Exception:
        temporary.unlink(missing_ok=True)
        raise


def triangle_area(
    a: tuple[float, ...], b: tuple[float, ...], c: tuple[float, ...]
) -> float:
    ab = (b[0] - a[0], b[1] - a[1], b[2] - a[2])
    ac = (c[0] - a[0], c[1] - a[1], c[2] - a[2])
    cross = (
        ab[1] * ac[2] - ab[2] * ac[1],
        ab[2] * ac[0] - ab[0] * ac[2],
        ab[0] * ac[1] - ab[1] * ac[0],
    )
    return max(
        1.0e-8,
        0.5 * math.sqrt(
            cross[0] * cross[0] + cross[1] * cross[1] + cross[2] * cross[2]
        ),
    )


def collect_triangles(
    submeshes: list[contract.LegacySubmesh],
) -> list[Triangle]:
    triangles: list[Triangle] = []
    for submesh_index, submesh in enumerate(submeshes):
        require(
            len(submesh.indices) > 0 and len(submesh.indices) % 3 == 0,
            "source WModel has a non-triangle submesh",
        )
        for offset in range(0, len(submesh.indices), 3):
            indices = tuple(submesh.indices[offset : offset + 3])
            points = [submesh.vertices[index] for index in indices]
            centroid = tuple(
                sum(point[axis] for point in points) / 3.0 for axis in range(3)
            )
            triangles.append(
                Triangle(
                    submesh_index,
                    indices,
                    centroid,
                    triangle_area(points[0], points[1], points[2]),
                )
            )
    require(triangles, "source WModel contains no triangles")
    return triangles


def weighted_quantiles(
    rows: list[tuple[float, float]], split_count: int
) -> list[float]:
    require(split_count > 0, "split count must be positive")
    if split_count == 1:
        return []
    ordered = sorted(rows, key=lambda row: row[0])
    total = sum(weight for _, weight in ordered)
    require(total > 0.0 and math.isfinite(total), "triangle area is invalid")
    thresholds: list[float] = []
    accumulated = 0.0
    index = 0
    for partition in range(1, split_count):
        target = total * partition / split_count
        while (
            index + 1 < len(ordered)
            and accumulated + ordered[index][1] < target
        ):
            accumulated += ordered[index][1]
            index += 1
        thresholds.append(ordered[index][0])
    return thresholds


def build_partition(
    triangles: list[Triangle], vertical_splits: int, horizontal_splits: int
) -> list[list[Triangle]]:
    total_area = sum(triangle.area for triangle in triangles)
    mean_x = sum(triangle.centroid[0] * triangle.area for triangle in triangles) / total_area
    mean_z = sum(triangle.centroid[2] * triangle.area for triangle in triangles) / total_area
    covariance_xx = sum(
        (triangle.centroid[0] - mean_x) ** 2 * triangle.area
        for triangle in triangles
    ) / total_area
    covariance_zz = sum(
        (triangle.centroid[2] - mean_z) ** 2 * triangle.area
        for triangle in triangles
    ) / total_area
    covariance_xz = sum(
        (triangle.centroid[0] - mean_x)
        * (triangle.centroid[2] - mean_z)
        * triangle.area
        for triangle in triangles
    ) / total_area
    axis_angle = 0.5 * math.atan2(
        2.0 * covariance_xz, covariance_xx - covariance_zz
    )
    axis_x = math.cos(axis_angle)
    axis_z = math.sin(axis_angle)

    projected: list[tuple[Triangle, float]] = []
    for triangle in triangles:
        horizontal = (
            (triangle.centroid[0] - mean_x) * axis_x
            + (triangle.centroid[2] - mean_z) * axis_z
        )
        projected.append((triangle, horizontal))

    vertical_thresholds = weighted_quantiles(
        [(triangle.centroid[1], triangle.area) for triangle in triangles],
        vertical_splits,
    )
    horizontal_thresholds = weighted_quantiles(
        [(horizontal, triangle.area) for triangle, horizontal in projected],
        horizontal_splits,
    )
    pieces: list[list[Triangle]] = [
        [] for _ in range(vertical_splits * horizontal_splits)
    ]
    for triangle, horizontal in projected:
        vertical_index = sum(
            triangle.centroid[1] > threshold for threshold in vertical_thresholds
        )
        horizontal_index = sum(
            horizontal > threshold for threshold in horizontal_thresholds
        )
        pieces[vertical_index * horizontal_splits + horizontal_index].append(
            triangle
        )
    require(all(piece for piece in pieces), "spatial partition produced an empty shard")
    return pieces


def calculate_bounds(
    vertices: list[tuple[float, ...]],
) -> tuple[tuple[float, ...], tuple[float, float, float], tuple[float, float, float]]:
    minimum = tuple(min(vertex[axis] for vertex in vertices) for axis in range(3))
    maximum = tuple(max(vertex[axis] for vertex in vertices) for axis in range(3))
    centre = tuple((minimum[axis] + maximum[axis]) * 0.5 for axis in range(3))
    radius = max(
        math.sqrt(
            sum((vertex[axis] - centre[axis]) ** 2 for axis in range(3))
        )
        for vertex in vertices
    )
    return (*minimum, *maximum, *centre, radius), minimum, maximum


def build_legacy_mesh_section(
    submeshes: list[contract.LegacySubmesh],
) -> bytes:
    require(submeshes, "piece contains no material submesh")
    index_stride = 4 if any(len(submesh.vertices) > 0xFFFF for submesh in submeshes) else 2
    index_format = "<I" if index_stride == 4 else "<H"
    descriptors: list[bytes] = []
    vertex_blocks: list[bytes] = []
    index_blocks: list[bytes] = []
    bounds_rows: list[tuple[float, ...]] = []
    vertex_offset = 0
    index_offset = 0
    for submesh in submeshes:
        vertex_blob = b"".join(
            struct.pack("<3f3f2f3ff", *vertex) for vertex in submesh.vertices
        )
        index_blob = b"".join(
            struct.pack(index_format, index) for index in submesh.indices
        )
        bounds, _, _ = calculate_bounds(list(submesh.vertices))
        descriptors.append(
            contract.SUBMESH_DESC.pack(
                vertex_offset,
                len(submesh.vertices),
                index_offset,
                len(submesh.indices),
                submesh.material_index,
                submesh.material_hash,
                submesh.name_bytes,
            )
        )
        vertex_blocks.append(vertex_blob)
        index_blocks.append(index_blob)
        bounds_rows.append(bounds)
        vertex_offset += len(vertex_blob)
        index_offset += len(index_blob)

    content = (
        contract.MESH_HEADER.pack(
            b"WMSH",
            len(submeshes),
            0,
            contract.VF_STATIC_BASE,
            contract.STRIDE_STATIC,
            sum(len(submesh.vertices) for submesh in submeshes),
            sum(len(submesh.indices) for submesh in submeshes),
            index_stride,
            1,
            b"\0\0\0",
        )
        + b"".join(descriptors)
        + b"".join(vertex_blocks)
        + b"".join(index_blocks)
        + b"".join(contract.BOUNDS_V1.pack(*row) for row in bounds_rows)
    )
    return contract.FILE_HEADER.pack(
        b"WINT", 1, 0, 0, len(content)
    ) + content


def rebuild_legacy_wmodel(
    model_header: tuple[int, int, int, tuple[int, ...]],
    sections: list[contract.Section],
    mesh_section: bytes,
) -> bytes:
    section_count, animation_count, model_flags, reserved = model_header
    output_sections: list[contract.Section] = []
    replacement_count = 0
    for section in sections:
        if section.type_id == 1:
            output_sections.append(
                contract.Section(
                    section.type_id, section.index, section.name_bytes, mesh_section
                )
            )
            replacement_count += 1
        else:
            output_sections.append(section)
    require(replacement_count == 1, "source WModel mesh section is ambiguous")

    offset = contract.MODEL_HEADER.size + section_count * contract.SECTION_DESC.size
    descriptors: list[bytes] = []
    payloads: list[bytes] = []
    for section in output_sections:
        descriptors.append(
            contract.SECTION_DESC.pack(
                section.type_id,
                section.index,
                offset,
                len(section.payload),
                section.name_bytes,
            )
        )
        payloads.append(section.payload)
        offset += len(section.payload)
    content = (
        contract.MODEL_HEADER.pack(
            b"WMOD", section_count, animation_count, model_flags, *reserved
        )
        + b"".join(descriptors)
        + b"".join(payloads)
    )
    return contract.FILE_HEADER.pack(b"WINT", 1, 0, 0, len(content)) + content


def build_piece(
    piece_index: int,
    triangles: list[Triangle],
    source_submeshes: list[contract.LegacySubmesh],
    model_header: tuple[int, int, int, tuple[int, ...]],
    sections: list[contract.Section],
    output_path: Path,
) -> PieceResult:
    total_area = sum(triangle.area for triangle in triangles)
    pivot = tuple(
        sum(triangle.centroid[axis] * triangle.area for triangle in triangles)
        / total_area
        for axis in range(3)
    )
    by_submesh: dict[int, list[Triangle]] = {}
    for triangle in triangles:
        by_submesh.setdefault(triangle.submesh_index, []).append(triangle)

    output_submeshes: list[contract.LegacySubmesh] = []
    all_vertices: list[tuple[float, ...]] = []
    total_indices = 0
    for submesh_index in sorted(by_submesh):
        source = source_submeshes[submesh_index]
        vertex_lookup: dict[int, int] = {}
        vertices: list[tuple[float, ...]] = []
        indices: list[int] = []
        for triangle in by_submesh[submesh_index]:
            for source_index in triangle.indices:
                target_index = vertex_lookup.get(source_index)
                if target_index is None:
                    source_vertex = source.vertices[source_index]
                    translated = (
                        source_vertex[0] - pivot[0],
                        source_vertex[1] - pivot[1],
                        source_vertex[2] - pivot[2],
                        *source_vertex[3:],
                    )
                    target_index = len(vertices)
                    vertex_lookup[source_index] = target_index
                    vertices.append(translated)
                indices.append(target_index)
        output_submeshes.append(
            contract.LegacySubmesh(
                source.material_index,
                source.material_hash,
                source.name_bytes,
                tuple(vertices),
                tuple(indices),
            )
        )
        all_vertices.extend(vertices)
        total_indices += len(indices)

    mesh_section = build_legacy_mesh_section(output_submeshes)
    output = rebuild_legacy_wmodel(model_header, sections, mesh_section)
    write_atomic(output_path, output)
    reparsed_header, _, reparsed_submeshes = contract.parse_legacy_wmodel(output)
    require(
        len(reparsed_submeshes) == len(output_submeshes),
        "piece WModel submesh count changed",
    )
    reparsed_vertices = [
        vertex for submesh in reparsed_submeshes for vertex in submesh.vertices
    ]
    _, minimum, maximum = calculate_bounds(reparsed_vertices)
    return PieceResult(
        piece_index,
        output_path,
        pivot,
        minimum,
        maximum,
        len(triangles),
        len(all_vertices),
        total_indices,
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output-directory", required=True, type=Path)
    parser.add_argument("--receipt", required=True, type=Path)
    parser.add_argument("--source-asset-id", required=True)
    parser.add_argument("--asset-prefix", required=True)
    parser.add_argument("--vertical-splits", type=int, default=4)
    parser.add_argument("--horizontal-splits", type=int, default=3)
    parser.add_argument("--resource-root", default="Deploy/LV_LUT_HEARTRB_ED")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    require(args.input.is_file(), f"source WModel does not exist: {args.input}")
    require(
        1 <= args.vertical_splits <= 8 and 1 <= args.horizontal_splits <= 8,
        "split counts are outside the supported range",
    )
    source_bytes = args.input.read_bytes()
    model_header, sections, source_submeshes = contract.parse_legacy_wmodel(
        source_bytes
    )
    triangles = collect_triangles(source_submeshes)
    partitions = build_partition(
        triangles, args.vertical_splits, args.horizontal_splits
    )

    results: list[PieceResult] = []
    for piece_index, partition in enumerate(partitions):
        output_path = args.output_directory / (
            f"{args.asset_prefix}_CHUNK_{piece_index:02d}.wmodel"
        )
        results.append(
            build_piece(
                piece_index,
                partition,
                source_submeshes,
                model_header,
                sections,
                output_path,
            )
        )

    source_triangle_count = sum(len(submesh.indices) // 3 for submesh in source_submeshes)
    require(
        sum(result.triangle_count for result in results) == source_triangle_count,
        "piece triangle coverage differs from the source",
    )
    resource_root = args.resource_root.strip("/\\")
    receipt = {
        "schema": "lostark.deploy-wall-debris-recipe",
        "formatVersion": 1,
        "provenance": "PROJECT_AUTHORED",
        "sourceAssetId": args.source_asset_id,
        "sourceFracturedWModel": args.input.as_posix(),
        "sourceSha256": hashlib.sha256(source_bytes).hexdigest(),
        "partition": {
            "method": "area-weighted-principal-axis-grid",
            "verticalSplits": args.vertical_splits,
            "horizontalSplits": args.horizontal_splits,
            "sourceTriangleCount": source_triangle_count,
            "coverage": "EVERY_SOURCE_TRIANGLE_EXACTLY_ONCE",
            "sourceChunkPivotsRecovered": False,
        },
        "pieces": [],
    }
    for result in results:
        asset_id = (
            f"{resource_root}/{args.source_asset_id}/fractured/"
            f"{result.path.name}"
        )
        receipt["pieces"].append(
            {
                "pieceId": f"piece.{result.piece_index:02d}",
                "assetId": asset_id,
                "pivotWModelUnits": [round(value, 9) for value in result.pivot],
                "pivotMetersAtScale1": [round(value * 0.01, 9) for value in result.pivot],
                "localBoundsWModelUnits": {
                    "min": [round(value, 9) for value in result.minimum],
                    "max": [round(value, 9) for value in result.maximum],
                },
                "triangleCount": result.triangle_count,
                "vertexCount": result.vertex_count,
                "indexCount": result.index_count,
                "sha256": sha256(result.path),
            }
        )
    write_atomic(
        args.receipt,
        (json.dumps(receipt, ensure_ascii=False, indent=2) + "\n").encode("utf-8"),
    )
    print(
        f"Built {len(results)} PROJECT_AUTHORED wall pieces from "
        f"{source_triangle_count} source triangles"
    )
    print(f"Receipt: {args.receipt}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
