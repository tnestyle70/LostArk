#!/usr/bin/env python3

from __future__ import annotations

import argparse
import collections
import copy
import hashlib
import json
import math
import struct
import sys
from pathlib import Path
from typing import Any, Iterable


REPO_ROOT = Path(__file__).resolve().parents[2]
POSITION_TOLERANCE = 0.002
CHANNEL_TOLERANCE = 1.0e-5
ASSETS = (
    ("FX_SM_01", "fm_v_wp_wsdm_base_01"),
    ("FX_SM_01", "fm_m_trail_002"),
    ("FX_SM_00", "fm_h_swing_03"),
    ("FX_SM_00", "fm_h_swing_05"),
    ("FX_SM_00", "fm_h_swing_01"),
    ("FX_SM_00", "fm_o_swing_02"),
    ("FX_SM_00", "fm_a_stone_001"),
)


def file_sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def json_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2, sort_keys=False) + "\n"
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(
        json.dumps(
            value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
        ).encode("utf-8")
    ).hexdigest()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def finite(values: Iterable[float]) -> bool:
    return all(math.isfinite(value) for value in values)


def accessor_values(
    document: dict[str, Any], directory: Path, accessor_index: int
) -> tuple[list[tuple[float | int, ...]], dict[str, Any]]:
    accessor = document["accessors"][accessor_index]
    view = document["bufferViews"][accessor["bufferView"]]
    buffer = document["buffers"][view["buffer"]]
    uri = str(buffer["uri"])
    require(not uri.startswith("data:"), "embedded glTF buffers are not supported")
    buffer_path = directory / uri
    payload = buffer_path.read_bytes()
    component_format = {5121: "B", 5123: "H", 5125: "I", 5126: "f"}.get(
        int(accessor["componentType"])
    )
    component_count = {
        "SCALAR": 1,
        "VEC2": 2,
        "VEC3": 3,
        "VEC4": 4,
    }.get(str(accessor["type"]))
    require(
        component_format is not None and component_count is not None,
        f"unsupported glTF accessor: {accessor}",
    )
    value_format = "<" + component_format * component_count
    value_size = struct.calcsize(value_format)
    offset = int(view.get("byteOffset", 0)) + int(accessor.get("byteOffset", 0))
    stride = int(view.get("byteStride", value_size))
    count = int(accessor["count"])
    require(
        offset >= 0 and stride >= value_size and offset + (count - 1) * stride + value_size <= len(payload),
        f"glTF accessor exceeds buffer: {buffer_path}",
    )
    values = [
        struct.unpack_from(value_format, payload, offset + index * stride)
        for index in range(count)
    ]
    value_payload = b"".join(
        payload[offset + index * stride : offset + index * stride + value_size]
        for index in range(count)
    )
    return values, {
        "accessorIndex": accessor_index,
        "bufferIndex": int(view["buffer"]),
        "bufferViewIndex": int(accessor["bufferView"]),
        "componentType": int(accessor["componentType"]),
        "type": str(accessor["type"]),
        "normalized": bool(accessor.get("normalized", False)),
        "count": count,
        "byteOffset": offset,
        "byteStride": stride,
        "decodedValueBytes": len(value_payload),
        "decodedValueSha256": hashlib.sha256(value_payload).hexdigest(),
        "bufferLogicalPath": uri.replace("\\", "/"),
        "bufferBytes": len(payload),
        "bufferSha256": file_sha256(buffer_path),
    }


def identity_node_transform(node: dict[str, Any]) -> dict[str, Any]:
    matrix = [float(value) for value in node.get("matrix", [])]
    translation = [float(value) for value in node.get("translation", [0.0, 0.0, 0.0])]
    rotation = [float(value) for value in node.get("rotation", [0.0, 0.0, 0.0, 1.0])]
    scale = [float(value) for value in node.get("scale", [1.0, 1.0, 1.0])]
    expected_matrix = [
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    ]
    matrix_identity = not matrix or all(
        abs(value - expected) <= 1.0e-7
        for value, expected in zip(matrix, expected_matrix)
    )
    trs_identity = (
        all(abs(value) <= 1.0e-7 for value in translation)
        and all(abs(value) <= 1.0e-7 for value in rotation[:3])
        and abs(rotation[3] - 1.0) <= 1.0e-7
        and all(abs(value - 1.0) <= 1.0e-7 for value in scale)
    )
    return {
        "matrix": matrix,
        "translation": translation,
        "rotationQuaternion": rotation,
        "scale": scale,
        "status": "IDENTITY_PROVEN" if matrix_identity and trs_identity else "NON_IDENTITY_BLOCKED",
    }


def parse_gltf(path: Path) -> dict[str, Any]:
    document = json.loads(path.read_text(encoding="utf-8"))
    require(isinstance(document, dict), f"glTF root is not an object: {path}")
    vertices: list[dict[str, Any]] = []
    indices: list[int] = []
    primitive_rows: list[dict[str, Any]] = []
    accessor_rows: dict[str, list[dict[str, Any]]] = collections.defaultdict(list)
    vertex_base = 0
    for mesh_index, mesh in enumerate(document.get("meshes", [])):
        for primitive_index, primitive in enumerate(mesh.get("primitives", [])):
            attributes = primitive.get("attributes") or {}
            required = ("POSITION", "NORMAL", "TEXCOORD_0", "TANGENT")
            require(
                all(name in attributes for name in required) and "indices" in primitive,
                f"glTF primitive misses a required channel: {path}",
            )
            decoded: dict[str, list[tuple[float | int, ...]]] = {}
            for name in (*required, "COLOR_0"):
                if name not in attributes:
                    continue
                values, evidence = accessor_values(
                    document, path.parent, int(attributes[name])
                )
                decoded[name] = values
                accessor_rows[name].append(evidence)
            primitive_indices, index_evidence = accessor_values(
                document, path.parent, int(primitive["indices"])
            )
            accessor_rows["INDICES"].append(index_evidence)
            count = len(decoded["POSITION"])
            require(
                all(len(decoded[name]) == count for name in required),
                f"glTF vertex channel counts differ: {path}",
            )
            for index in range(count):
                position = tuple(float(value) for value in decoded["POSITION"][index])
                normal = tuple(float(value) for value in decoded["NORMAL"][index])
                uv = tuple(float(value) for value in decoded["TEXCOORD_0"][index])
                tangent = tuple(float(value) for value in decoded["TANGENT"][index])
                require(
                    finite((*position, *normal, *uv, *tangent)),
                    f"glTF has a non-finite vertex: {path}",
                )
                vertices.append(
                    {
                        "position": position,
                        "normal": normal,
                        "uv0": uv,
                        "tangent": tangent,
                    }
                )
            flat_indices = [int(value[0]) + vertex_base for value in primitive_indices]
            require(
                len(flat_indices) % 3 == 0 and all(
                    vertex_base <= value < vertex_base + count for value in flat_indices
                ),
                f"glTF indices are invalid: {path}",
            )
            indices.extend(flat_indices)
            primitive_rows.append(
                {
                    "meshIndex": mesh_index,
                    "primitiveIndex": primitive_index,
                    "vertexBase": vertex_base,
                    "vertexCount": count,
                    "indexBase": len(indices) - len(flat_indices),
                    "indexCount": len(flat_indices),
                    "attributes": sorted(attributes),
                }
            )
            vertex_base += count
    require(vertices and indices, f"glTF has no indexed geometry: {path}")
    nodes = document.get("nodes", [])
    parents: dict[int, int] = {}
    for parent_index, node in enumerate(nodes):
        for child_index in node.get("children", []):
            require(
                int(child_index) not in parents,
                f"glTF node has multiple parents: {path}",
            )
            parents[int(child_index)] = parent_index
    relevant_nodes: set[int] = set()
    for node_index, node in enumerate(nodes):
        if "mesh" not in node:
            continue
        current: int | None = node_index
        while current is not None:
            relevant_nodes.add(current)
            current = parents.get(current)
    node_transforms = []
    for node_index in sorted(relevant_nodes):
        evidence = identity_node_transform(nodes[node_index])
        evidence.update(
            {
                "nodeIndex": node_index,
                "nodeName": str(nodes[node_index].get("name", "")),
                "meshIndex": nodes[node_index].get("mesh"),
                "parentNodeIndex": parents.get(node_index),
            }
        )
        node_transforms.append(evidence)
    require(
        node_transforms
        and all(row["status"] == "IDENTITY_PROVEN" for row in node_transforms),
        f"glTF mesh-node hierarchy has a non-identity transform: {path}",
    )
    buffers = []
    for buffer_index, buffer in enumerate(document.get("buffers", [])):
        uri = str(buffer["uri"])
        require(not uri.startswith("data:"), "embedded glTF buffers are not supported")
        buffer_path = path.parent / uri
        buffers.append(
            {
                "bufferIndex": buffer_index,
                "logicalPath": uri.replace("\\", "/"),
                "declaredBytes": int(buffer.get("byteLength", 0)),
                "physicalBytes": buffer_path.stat().st_size,
                "sha256": file_sha256(buffer_path),
            }
        )
    return {
        "vertices": vertices,
        "indices": indices,
        "primitives": primitive_rows,
        "accessors": dict(accessor_rows),
        "buffers": buffers,
        "meshNodeTransforms": node_transforms,
        "gltfSha256": file_sha256(path),
        "gltfBytes": path.stat().st_size,
    }


def parse_wmodel(path: Path) -> dict[str, Any]:
    payload = path.read_bytes()
    require(len(payload) >= 48, f"WModel is truncated: {path}")
    magic, major, minor, flags, content_size = struct.unpack_from("<4sHHII", payload, 0)
    require(
        magic == b"WINT" and major == 1 and flags == 0 and content_size == len(payload) - 16,
        f"WModel outer header is invalid: {path}",
    )
    content_offset = 16
    model = struct.unpack_from("<4sIIIIIII", payload, content_offset)
    require(model[0] == b"WMOD" and model[1] >= 2, f"WMOD header is invalid: {path}")
    mesh_payload: bytes | None = None
    sections: list[dict[str, Any]] = []
    for index in range(model[1]):
        section_type, section_index, offset, size, raw_name = struct.unpack_from(
            "<IIQQ40s", payload, content_offset + 32 + index * 64
        )
        require(
            offset + size <= content_size,
            f"WMOD section exceeds payload: {path}",
        )
        name = raw_name.split(b"\0", 1)[0].decode("utf-8", errors="strict")
        sections.append(
            {
                "type": section_type,
                "index": section_index,
                "name": name,
                "bytes": size,
            }
        )
        if section_type == 1:
            require(mesh_payload is None, f"WModel has duplicate mesh sections: {path}")
            begin = content_offset + offset
            mesh_payload = payload[begin : begin + size]
    require(mesh_payload is not None, f"WModel has no mesh section: {path}")
    mesh = mesh_payload
    mesh_outer = struct.unpack_from("<4sHHII", mesh, 0)
    require(mesh_outer[0] == b"WINT" and mesh_outer[1] == 1, f"WMSH outer header is invalid: {path}")
    header = struct.unpack_from("<4sIIIIIIIB3s", mesh, 16)
    require(header[0] == b"WMSH", f"WMSH metadata is invalid: {path}")
    (
        submesh_count,
        bone_count,
        vertex_flags,
        vertex_stride,
        vertex_count,
        index_count,
        index_stride,
        has_bounds,
    ) = header[1:9]
    require(
        bone_count == 0 and vertex_stride == 48 and index_stride in (2, 4),
        f"Artist F parity expects a static WModel: {path}",
    )
    submeshes: list[dict[str, Any]] = []
    for index in range(submesh_count):
        value = struct.unpack_from("<IIIIIQ20s", mesh, 52 + index * 48)
        submeshes.append(
            {
                "vertexOffsetBytes": value[0],
                "vertexCount": value[1],
                "indexOffsetBytes": value[2],
                "indexCount": value[3],
                "materialIndex": value[4],
                "materialHash": value[5],
                "name": value[6].split(b"\0", 1)[0].decode("utf-8", errors="strict"),
            }
        )
    vertex_offset = 52 + submesh_count * 48
    vertices: list[dict[str, Any]] = []
    for index in range(vertex_count):
        offset = vertex_offset + index * vertex_stride
        position = struct.unpack_from("<3f", mesh, offset)
        normal = struct.unpack_from("<3f", mesh, offset + 12)
        uv = struct.unpack_from("<2f", mesh, offset + 24)
        tangent = struct.unpack_from("<3f", mesh, offset + 32)
        handedness = struct.unpack_from("<f", mesh, offset + 44)[0]
        require(
            finite((*position, *normal, *uv, *tangent, handedness)),
            f"WModel has a non-finite vertex: {path}",
        )
        vertices.append(
            {
                "position": position,
                "normal": normal,
                "uv0": uv,
                "tangent": (*tangent, handedness),
            }
        )
    index_offset = vertex_offset + vertex_count * vertex_stride
    index_format = "<H" if index_stride == 2 else "<I"
    indices = [
        struct.unpack_from(index_format, mesh, index_offset + index * index_stride)[0]
        for index in range(index_count)
    ]
    require(
        len(indices) % 3 == 0 and all(value < vertex_count for value in indices),
        f"WModel indices are invalid: {path}",
    )
    bounds_offset = index_offset + index_count * index_stride
    embedded_bounds = None
    if has_bounds:
        require(
            bounds_offset + submesh_count * 40 <= len(mesh),
            f"WModel bounds are truncated: {path}",
        )
        embedded_bounds = [
            list(struct.unpack_from("<10f", mesh, bounds_offset + index * 40))
            for index in range(submesh_count)
        ]
    return {
        "vertices": vertices,
        "indices": indices,
        "submeshes": submeshes,
        "sections": sections,
        "fileSha256": file_sha256(path),
        "fileBytes": len(payload),
        "outerVersion": [major, minor],
        "meshVersion": [mesh_outer[1], mesh_outer[2]],
        "vertexFormatFlags": vertex_flags,
        "vertexStride": vertex_stride,
        "indexStride": index_stride,
        "embeddedBounds": embedded_bounds,
    }


def transform_position(value: tuple[float, ...], scale: float) -> tuple[float, float, float]:
    return value[0] * scale, value[1] * scale, -value[2] * scale


def transform_direction(value: tuple[float, ...]) -> tuple[float, float, float]:
    return value[0], value[1], -value[2]


def aabb(values: Iterable[tuple[float, float, float]]) -> dict[str, list[float]]:
    rows = list(values)
    return {
        "minimum": [min(value[axis] for value in rows) for axis in range(3)],
        "maximum": [max(value[axis] for value in rows) for axis in range(3)],
    }


def distance(left: tuple[float, ...], right: tuple[float, ...]) -> float:
    return math.sqrt(sum((left[index] - right[index]) ** 2 for index in range(len(left))))


def position_candidates(
    source: list[dict[str, Any]], tolerance: float
) -> tuple[dict[tuple[int, int, int], list[dict[str, Any]]], float]:
    cell_size = tolerance
    buckets: dict[tuple[int, int, int], list[dict[str, Any]]] = collections.defaultdict(list)
    for vertex in source:
        position = transform_position(vertex["position"], 100.0)
        key = tuple(math.floor(value / cell_size) for value in position)
        buckets[key].append(vertex)
    return buckets, cell_size


def nearby(
    buckets: dict[tuple[int, int, int], list[dict[str, Any]]],
    cell_size: float,
    position: tuple[float, float, float],
) -> list[dict[str, Any]]:
    base = tuple(math.floor(value / cell_size) for value in position)
    result: list[dict[str, Any]] = []
    for x in range(base[0] - 1, base[0] + 2):
        for y in range(base[1] - 1, base[1] + 2):
            for z in range(base[2] - 1, base[2] + 2):
                result.extend(buckets.get((x, y, z), []))
    return result


def channel_parity(source: dict[str, Any], runtime: dict[str, Any]) -> dict[str, Any]:
    buckets, cell_size = position_candidates(source["vertices"], POSITION_TOLERANCE)
    unmatched_positions = 0
    position_errors: list[float] = []
    normal_errors: list[float] = []
    tangent_errors: list[float] = []
    uv_errors: list[float] = []
    for runtime_vertex in runtime["vertices"]:
        runtime_position = runtime_vertex["position"]
        candidates = nearby(buckets, cell_size, runtime_position)
        position_matches = [
            value
            for value in candidates
            if distance(transform_position(value["position"], 100.0), runtime_position)
            <= POSITION_TOLERANCE
        ]
        if not position_matches:
            unmatched_positions += 1
            continue
        best = min(
            position_matches,
            key=lambda value: (
                distance(transform_direction(value["normal"]), runtime_vertex["normal"])
                + distance(transform_direction(value["tangent"][:3]), runtime_vertex["tangent"][:3])
                + distance(value["uv0"], runtime_vertex["uv0"])
            ),
        )
        position_error = max(
            abs(left - right)
            for left, right in zip(
                transform_position(best["position"], 100.0), runtime_position
            )
        )
        position_errors.append(position_error)
        normal_errors.append(
            distance(transform_direction(best["normal"]), runtime_vertex["normal"])
        )
        tangent_errors.append(
            distance(
                transform_direction(best["tangent"][:3]), runtime_vertex["tangent"][:3]
            )
        )
        uv_errors.append(distance(best["uv0"], runtime_vertex["uv0"]))
    source_handedness = collections.Counter(
        f"{float(vertex['tangent'][3]):.1f}" for vertex in source["vertices"]
    )
    runtime_handedness = collections.Counter(
        f"{float(vertex['tangent'][3]):.1f}" for vertex in runtime["vertices"]
    )
    source_domain = sorted(source_handedness)
    runtime_domain = sorted(runtime_handedness)
    # UModel repeated exports of the same package/object do not produce a stable
    # TANGENT.w population.  The current WModel also contains only +1.  Even
    # when this one selected glTF happens to contain only +1, equality is not
    # source provenance and must not be promoted to exact parity.
    handedness_status = "UNTRUSTED_EXPORT_CHANNEL_BLOCKED"
    return {
        "position": {
            "relation": "runtime = source * diag(100, 100, -100)",
            "tolerance": POSITION_TOLERANCE,
            "runtimeVertexCount": len(runtime["vertices"]),
            "matchedRuntimeVertexCount": len(runtime["vertices"]) - unmatched_positions,
            "unmatchedRuntimeVertexCount": unmatched_positions,
            "maximumMatchedError": max(position_errors, default=0.0),
            "maximumClientMeterErrorAfterCarrierPreScale": (
                max(position_errors, default=0.0) * 0.01
            ),
            "status": "PROVEN" if unmatched_positions == 0 else "PROVEN_WITH_NUMERIC_EPSILON_BUCKET_GAPS",
        },
        "normal": {
            "relation": "runtime = source * diag(1, 1, -1)",
            "maximumMatchedError": max(normal_errors, default=0.0),
            "status": "PROVEN" if max(normal_errors, default=0.0) <= CHANNEL_TOLERANCE else "BLOCKED",
        },
        "tangentXYZ": {
            "relation": "runtime = source * diag(1, 1, -1)",
            "maximumMatchedError": max(tangent_errors, default=0.0),
            "status": "PROVEN" if max(tangent_errors, default=0.0) <= CHANNEL_TOLERANCE else "BLOCKED",
        },
        "tangentHandedness": {
            "sourceCounts": dict(sorted(source_handedness.items())),
            "runtimeCounts": dict(sorted(runtime_handedness.items())),
            "status": handedness_status,
        },
        "uv0": {
            "relation": "runtime = source",
            "maximumMatchedError": max(uv_errors, default=0.0),
            "status": "PROVEN" if max(uv_errors, default=0.0) <= CHANNEL_TOLERANCE else "BLOCKED",
        },
    }


def quantized_payload(vertex: dict[str, Any], source: bool) -> tuple[int, ...]:
    position = (
        transform_position(vertex["position"], 100.0)
        if source
        else vertex["position"]
    )
    normal = transform_direction(vertex["normal"]) if source else vertex["normal"]
    tangent = (
        transform_direction(vertex["tangent"][:3])
        if source
        else vertex["tangent"][:3]
    )
    return (
        *(round(component / POSITION_TOLERANCE) for component in position),
        *(round(component / CHANNEL_TOLERANCE) for component in normal),
        *(round(component / CHANNEL_TOLERANCE) for component in tangent),
        *(round(component / CHANNEL_TOLERANCE) for component in vertex["uv0"]),
    )


def triangle_signature(
    vertices: list[dict[str, Any]], indices: list[int], source: bool
) -> tuple[collections.Counter[Any], collections.Counter[Any]]:
    unordered: collections.Counter[Any] = collections.Counter()
    oriented: collections.Counter[Any] = collections.Counter()
    for offset in range(0, len(indices), 3):
        values = [
            quantized_payload(vertices[index], source)
            for index in indices[offset : offset + 3]
        ]
        canonical_indices = sorted(range(3), key=lambda index: values[index])
        canonical = tuple(values[index] for index in canonical_indices)
        inversions = sum(
            canonical_indices[left] > canonical_indices[right]
            for left in range(3)
            for right in range(left + 1, 3)
        )
        parity = inversions % 2
        unordered[canonical] += 1
        oriented[(canonical, parity)] += 1
    return unordered, oriented


def topology_parity(source: dict[str, Any], runtime: dict[str, Any]) -> dict[str, Any]:
    source_unordered, source_oriented = triangle_signature(
        source["vertices"], source["indices"], True
    )
    runtime_unordered, runtime_oriented = triangle_signature(
        runtime["vertices"], runtime["indices"], False
    )
    missing = sum((source_unordered - runtime_unordered).values())
    extra = sum((runtime_unordered - source_unordered).values())
    same = sum((source_oriented & runtime_oriented).values())
    reversed_runtime = collections.Counter(
        {
            (signature, 1 - parity): count
            for (signature, parity), count in runtime_oriented.items()
        }
    )
    reversed_count = sum((source_oriented & reversed_runtime).values())
    triangle_count = len(source["indices"]) // 3
    return {
        "sourceIndexCount": len(source["indices"]),
        "runtimeIndexCount": len(runtime["indices"]),
        "triangleCount": triangle_count,
        "cornerPayload": "POSITION+NORMAL+TANGENT_XYZ+UV0",
        "positionTolerance": POSITION_TOLERANCE,
        "attributeTolerance": CHANNEL_TOLERANCE,
        "missingTriangleCount": missing,
        "extraTriangleCount": extra,
        "sameWindingTriangleCount": same,
        "reversedWindingTriangleCount": reversed_count,
        "reflection": "Z_AXIS",
        "windingStatus": (
            "REVERSED_AFTER_REFLECTION_PROVEN"
            if missing == 0 and extra == 0 and reversed_count == triangle_count
            else "BLOCKED"
        ),
    }


def partition_payload_parity(
    source: dict[str, Any], runtime: dict[str, Any]
) -> dict[str, Any]:
    require(
        len(source["primitives"]) == len(runtime["submeshes"]),
        "glTF primitive and WModel submesh counts differ",
    )
    rows = []
    for primitive, submesh in zip(source["primitives"], runtime["submeshes"]):
        source_vertex_begin = int(primitive["vertexBase"])
        source_vertex_end = source_vertex_begin + int(primitive["vertexCount"])
        source_index_begin = int(primitive["indexBase"])
        source_index_end = source_index_begin + int(primitive["indexCount"])
        runtime_vertex_begin = int(submesh["vertexOffsetBytes"]) // int(
            runtime["vertexStride"]
        )
        runtime_vertex_end = runtime_vertex_begin + int(submesh["vertexCount"])
        runtime_index_begin = int(submesh["indexOffsetBytes"]) // int(
            runtime["indexStride"]
        )
        runtime_index_end = runtime_index_begin + int(submesh["indexCount"])
        require(
            source_vertex_end <= len(source["vertices"])
            and source_index_end <= len(source["indices"])
            and runtime_vertex_end <= len(runtime["vertices"])
            and runtime_index_end <= len(runtime["indices"]),
            "geometry partition exceeds its payload",
        )
        source_vertices = source["vertices"][source_vertex_begin:source_vertex_end]
        runtime_vertices = runtime["vertices"][runtime_vertex_begin:runtime_vertex_end]
        source_indices = [
            int(value) - source_vertex_begin
            for value in source["indices"][source_index_begin:source_index_end]
        ]
        runtime_indices = [
            int(value) - runtime_vertex_begin
            for value in runtime["indices"][runtime_index_begin:runtime_index_end]
        ]
        require(
            all(0 <= value < len(source_vertices) for value in source_indices)
            and all(0 <= value < len(runtime_vertices) for value in runtime_indices),
            "geometry partition indices are not local to their vertex partition",
        )
        source_vertex_payloads = [
            quantized_payload(vertex, True) for vertex in source_vertices
        ]
        runtime_vertex_payloads = [
            quantized_payload(vertex, False) for vertex in runtime_vertices
        ]
        source_referenced_indices = set(source_indices)
        runtime_referenced_indices = set(runtime_indices)
        source_corner_payloads = collections.Counter(
            source_vertex_payloads[index] for index in source_indices
        )
        runtime_corner_payloads = collections.Counter(
            runtime_vertex_payloads[index] for index in runtime_indices
        )
        source_referenced_payloads = {
            source_vertex_payloads[index] for index in source_referenced_indices
        }
        runtime_referenced_payloads = {
            runtime_vertex_payloads[index] for index in runtime_referenced_indices
        }
        source_unique_all = set(source_vertex_payloads)
        runtime_unique_all = set(runtime_vertex_payloads)
        rows.append(
            {
                "sourceMeshIndex": primitive["meshIndex"],
                "sourcePrimitiveIndex": primitive["primitiveIndex"],
                "runtimeSubmeshName": submesh["name"],
                "sourceVertexCount": len(source_vertices),
                "sourceReferencedVertexCount": len(source_referenced_indices),
                "sourceUnreferencedVertexCount": len(source_vertices)
                - len(source_referenced_indices),
                "sourceUniqueFullPayloadVertexCount": len(source_unique_all),
                "sourceDuplicateFullPayloadVertexCount": len(source_vertices)
                - len(source_unique_all),
                "runtimeVertexCount": len(runtime_vertices),
                "runtimeReferencedVertexCount": len(runtime_referenced_indices),
                "runtimeUnreferencedVertexCount": len(runtime_vertices)
                - len(runtime_referenced_indices),
                "runtimeUniqueFullPayloadVertexCount": len(runtime_unique_all),
                "indexCount": len(source_indices),
                "referencedFullPayloadSetEqual": (
                    source_referenced_payloads == runtime_referenced_payloads
                ),
                "indexedCornerFullPayloadMultisetEqual": (
                    source_corner_payloads == runtime_corner_payloads
                ),
                "status": (
                    "PROVEN_EXACT_FULL_PAYLOAD_DEDUP"
                    if source_referenced_payloads == runtime_referenced_payloads
                    and source_corner_payloads == runtime_corner_payloads
                    and len(runtime_unique_all) == len(runtime_vertices)
                    else "BLOCKED"
                ),
            }
        )
    return {
        "primitiveSubmeshPartitionCount": len(rows),
        "partitions": rows,
        "sourceVertexCount": sum(row["sourceVertexCount"] for row in rows),
        "sourceReferencedVertexCount": sum(
            row["sourceReferencedVertexCount"] for row in rows
        ),
        "sourceUnreferencedVertexCount": sum(
            row["sourceUnreferencedVertexCount"] for row in rows
        ),
        "sourceDuplicateFullPayloadVertexCount": sum(
            row["sourceDuplicateFullPayloadVertexCount"] for row in rows
        ),
        "runtimeVertexCount": sum(row["runtimeVertexCount"] for row in rows),
        "allPartitionsProven": all(row["status"].startswith("PROVEN") for row in rows),
    }


def bounds_parity(runtime: dict[str, Any]) -> dict[str, Any]:
    embedded = runtime["embeddedBounds"]
    if embedded is None:
        return {
            "runtimeConsumption": False,
            "status": "ABSENT_BLOCKED",
        }
    runtime_bounds = aabb(vertex["position"] for vertex in runtime["vertices"])
    rows = []
    for value in embedded:
        stored_minimum = [float(component) for component in value[:3]]
        stored_maximum = [float(component) for component in value[3:6]]
        stored_extent = [
            stored_maximum[index] - stored_minimum[index] for index in range(3)
        ]
        runtime_extent = [
            runtime_bounds["maximum"][index] - runtime_bounds["minimum"][index]
            for index in range(3)
        ]
        ratios = [
            runtime_extent[index] / stored_extent[index]
            if abs(stored_extent[index]) > 1.0e-12
            else None
            for index in range(3)
        ]
        finite_ratios = [value for value in ratios if value is not None]
        derived_scale = sorted(finite_ratios)[len(finite_ratios) // 2]
        error = max(
            abs(stored_minimum[index] * derived_scale - runtime_bounds["minimum"][index])
            for index in range(3)
        )
        error = max(
            error,
            max(
                abs(stored_maximum[index] * derived_scale - runtime_bounds["maximum"][index])
                for index in range(3)
            ),
        )
        rows.append(
            {
                "storedMinimum": stored_minimum,
                "storedMaximum": stored_maximum,
                "runtimeVertexAabb": runtime_bounds,
                "extentScalePerAxis": ratios,
                "derivedStoredToRuntimeScale": derived_scale,
                "maximumAabbComponentErrorAtDerivedScale": error,
            }
        )
    return {
        "storedBoundsCount": len(rows),
        "storedToRuntimeRelations": rows,
        "runtimeConsumption": False,
        "status": "STORED_AT_SOURCE_SCALE_RUNTIME_READER_SKIPS_NOT_CULLING_AUTHORITY",
    }


def build_asset_row(
    package: str,
    name: str,
    source_root: Path,
    runtime_root: Path,
    active_inventory: dict[str, Any],
) -> dict[str, Any]:
    source_path = source_root / package / "StaticMesh3" / f"{name}.gltf"
    runtime_path = runtime_root / f"{name}.wmodel"
    require(source_path.is_file(), f"source glTF is missing: {source_path}")
    require(runtime_path.is_file(), f"runtime WModel is missing: {runtime_path}")
    source = parse_gltf(source_path)
    runtime = parse_wmodel(runtime_path)
    parity = channel_parity(source, runtime)
    topology = topology_parity(source, runtime)
    payload_partitions = partition_payload_parity(source, runtime)
    source_positions = [vertex["position"] for vertex in source["vertices"]]
    runtime_positions = [vertex["position"] for vertex in runtime["vertices"]]
    color_present = "COLOR_0" in source["accessors"]
    color_status = (
        "UNTRUSTED_EXPORT_CHANNEL_BLOCKED_NOT_PRESERVED"
        if color_present
        else "SOURCE_CHANNEL_ABSENT"
    )
    embedded_bounds = runtime["embeddedBounds"]
    asset_id = f"Effect/Artist/Meshes/{name}.wmodel"
    expected_rows = [
        row
        for row in active_inventory["runtimeResourceIdentities"]
        if row["resourceKind"] == "WModel" and row["assetId"] == asset_id
    ]
    require(len(expected_rows) == 1, f"active inventory WModel identity is missing: {asset_id}")
    require(
        expected_rows[0]["sha256"] == runtime["fileSha256"]
        and int(expected_rows[0]["bytes"]) == runtime["fileBytes"],
        f"active inventory WModel hash changed: {asset_id}",
    )
    active_element_ids = sorted(
        row["activeElementId"]
        for row in active_inventory["activeElements"]
        if any(
            mapping.get("slotId") == "meshModel"
            and mapping.get("assetId") == asset_id
            for mapping in row.get("resourceMappings", [])
        )
    )
    require(active_element_ids, f"WModel has no active element usage: {asset_id}")
    return {
        "assetId": asset_id,
        "sourceObjectPath": f"{package.lower()}.{name}",
        "activeElementIds": active_element_ids,
        "activeInventoryIdentity": copy.deepcopy(expected_rows[0]),
        "source": {
            "gltfLogicalPath": f"{package}/StaticMesh3/{name}.gltf",
            "gltfBytes": source["gltfBytes"],
            "gltfSha256": source["gltfSha256"],
            "accessors": source["accessors"],
            "buffers": source["buffers"],
            "meshNodeTransforms": source["meshNodeTransforms"],
            "primitiveCount": len(source["primitives"]),
            "vertexCount": len(source["vertices"]),
            "indexCount": len(source["indices"]),
            "aabb": aabb(source_positions),
        },
        "runtime": {
            "resourceAssetId": asset_id,
            "bytes": runtime["fileBytes"],
            "sha256": runtime["fileSha256"],
            "outerVersion": runtime["outerVersion"],
            "meshVersion": runtime["meshVersion"],
            "sectionCount": len(runtime["sections"]),
            "submeshCount": len(runtime["submeshes"]),
            "vertexCount": len(runtime["vertices"]),
            "indexCount": len(runtime["indices"]),
            "vertexFormatFlags": runtime["vertexFormatFlags"],
            "vertexStride": runtime["vertexStride"],
            "indexStride": runtime["indexStride"],
            "aabb": aabb(runtime_positions),
            "embeddedBounds": embedded_bounds,
        },
        "parity": {
            **parity,
            "topology": topology,
            "indexedFullPayloadPartitions": payload_partitions,
            "bounds": bounds_parity(runtime),
            "originPivot": {
                "geometryOriginRelation": "origin unchanged by scale/reflection",
                "upkToGltfPivotProvenance": "UNRESOLVED",
                "occurrenceTransformAuthority": "HASH_LINKED_ACTION_CUE_EVIDENCE",
                "status": "UPK_TO_GLTF_PIVOT_PROVENANCE_UNRESOLVED",
            },
            "color0": {
                "sourceAccessorPresent": color_present,
                "runtimeVertexFormatHasColor0": False,
                "status": color_status,
            },
        },
        "scaleContract": {
            "sourceGeometryUnit": "UMODEL_GLTF_MODEL_UNIT",
            "runtimeGeometryRelation": "source * diag(100, 100, -100)",
            "sourceToRuntimePositionScale": 100.0,
            "carrierGeometryPreScale": 0.01,
            "particleMeshStartSizeScale": 1.0,
            "particleMeshStartSizeSemantics": "DIMENSIONLESS_AXIS_REORDER_ONLY",
            "combinedWorldMagnitude": 1.0,
            "status": "PROVEN_SEPARATE_CARRIER_AND_PARTICLE_SCALE",
        },
        "admission": {
            "compiledExecutionAllowed": False,
            "blockers": sorted(
                {
                    "MATERIAL_RECIPE_NOT_COMPILED",
                    "RENDER_STATE_NOT_COMPILED",
                    "UPK_TO_GLTF_PIVOT_PROVENANCE_UNRESOLVED",
                    "SOURCE_TANGENT_HANDEDNESS_UNTRUSTED_AND_NOT_PROVEN",
                    *(
                        ["UNTRUSTED_SOURCE_COLOR0_NOT_PRESERVED"]
                        if color_present
                        else []
                    ),
                }
            ),
        },
    }


def build_receipt(
    source_root: Path,
    runtime_root: Path,
    active_inventory_path: Path,
) -> dict[str, Any]:
    cook_script = REPO_ROOT / "Tools/LevelPlacementExtractor/cook_effect_runtime_resources.py"
    converter = REPO_ROOT / "Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe"
    active_inventory = json.loads(active_inventory_path.read_text(encoding="utf-8"))
    require(active_inventory.get("skillId") == 31470, "active inventory skill mismatch")
    cook_script_text = cook_script.read_text(encoding="utf-8")
    for token in ("--pretransform", "--scale", "100", "--no-auto-textures"):
        require(token in cook_script_text, f"converter recipe token is missing: {token}")
    assets = [
        build_asset_row(package, name, source_root, runtime_root, active_inventory)
        for package, name in ASSETS
    ]
    expected_asset_ids = {
        row["assetId"]
        for row in active_inventory["runtimeResourceIdentities"]
        if row["resourceKind"] == "WModel"
    }
    require(
        {row["assetId"] for row in assets} == expected_asset_ids,
        "geometry receipt does not cover the active inventory WModel set",
    )
    require(
        all(row["parity"]["position"]["matchedRuntimeVertexCount"] == row["runtime"]["vertexCount"] for row in assets),
        "at least one WModel position cannot be related to the source glTF",
    )
    require(
        all(row["parity"]["topology"]["windingStatus"] == "REVERSED_AFTER_REFLECTION_PROVEN" for row in assets),
        "at least one WModel topology relation is unresolved",
    )
    receipt: dict[str, Any] = {
        "schema": "lostark.effect-wmodel-geometry-parity-receipt",
        "formatVersion": 1,
        "characterClass": "Artist",
        "skillId": 31470,
        "scope": "ACTIVE_FIRST_LOD_MESH_CARRIERS",
        "runtimeAdmission": False,
        "inputs": {
            "activeInventoryAssetId": "Data/Effects/Imported/Artist/skill.31470.source-active-effect-inventory.receipt.json",
            "activeInventorySha256": file_sha256(active_inventory_path),
        },
        "upkToUmodelGltfProvenance": {
            "status": "UNRESOLVED_NOT_REPRODUCED_IN_THIS_SLICE",
            "scopeBoundary": "This receipt proves selected glTF/bin to current WModel only.",
            "blockers": [
                "SOURCE_UPK_EXPORT_SERIAL_HASH_NOT_PINNED",
                "UMODEL_EXECUTABLE_SHA256_NOT_PINNED",
                "UMODEL_FULL_COMMAND_LINE_NOT_PINNED",
                "CLEAN_EXPORT_DIRECTORY_OR_NOOVERWRITE_RESIDUE_NOT_PROVEN",
                "UPK_TO_GLTF_PIVOT_PROVENANCE_UNRESOLVED",
            ],
        },
        "converterEvidence": {
            "toolAssetId": "Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe",
            "toolSha256": file_sha256(converter),
            "cookScriptAssetId": "Tools/LevelPlacementExtractor/cook_effect_runtime_resources.py",
            "cookScriptSha256": file_sha256(cook_script),
            "recipeArguments": ["--pretransform", "--scale", "100", "--no-auto-textures"],
            "actualHistoricalInvocationProven": False,
            "provenanceStatus": "RECIPE_PRESENT_AND_OUTPUT_NUMERIC_RELATION_PROVEN_HISTORICAL_INVOCATION_UNRESOLVED",
            "converterSourceStatus": "BINARY_ONLY_SOURCE_NOT_PRESENT",
        },
        "globalScaleContract": {
            "meshParticleStartSize": {
                "semantic": "DIMENSIONLESS_AXIS_REORDER_ONLY",
                "scale": 1.0,
            },
            "carrierGeometry": {
                "cookedPositionScale": 100.0,
                "reflectionAxis": "Z",
                "carrierGeometryPreScale": 0.01,
            },
            "forbiddenCollapse": "Do not replace particle scale and carrierGeometryPreScale with one implicit legacy 0.01 multiplier.",
            "recookAlternative": "A future scale=1 recook may remove carrierGeometryPreScale, but is outside this source-contract slice.",
        },
        "assets": assets,
        "summary": {
            "wmodelCount": len(assets),
            "positionRelationProvenCount": sum(
                row["parity"]["position"]["matchedRuntimeVertexCount"]
                == row["runtime"]["vertexCount"]
                for row in assets
            ),
            "normalRelationProvenCount": sum(
                row["parity"]["normal"]["status"] == "PROVEN" for row in assets
            ),
            "tangentXyzRelationProvenCount": sum(
                row["parity"]["tangentXYZ"]["status"] == "PROVEN" for row in assets
            ),
            "tangentHandednessPreservedCount": 0,
            "tangentHandednessBlockedCount": sum(
                row["parity"]["tangentHandedness"]["status"]
                == "UNTRUSTED_EXPORT_CHANNEL_BLOCKED"
                for row in assets
            ),
            "uv0RelationProvenCount": sum(
                row["parity"]["uv0"]["status"] == "PROVEN" for row in assets
            ),
            "topologyAndReversedWindingProvenCount": sum(
                row["parity"]["topology"]["windingStatus"]
                == "REVERSED_AFTER_REFLECTION_PROVEN"
                for row in assets
            ),
            "indexedFullPayloadPartitionProvenCount": sum(
                row["parity"]["indexedFullPayloadPartitions"]["allPartitionsProven"]
                for row in assets
            ),
            "sourceVertexCount": sum(row["source"]["vertexCount"] for row in assets),
            "sourceReferencedVertexCount": sum(
                row["parity"]["indexedFullPayloadPartitions"]["sourceReferencedVertexCount"]
                for row in assets
            ),
            "sourceUnreferencedVertexCount": sum(
                row["parity"]["indexedFullPayloadPartitions"]["sourceUnreferencedVertexCount"]
                for row in assets
            ),
            "sourceDuplicateFullPayloadVertexCount": sum(
                row["parity"]["indexedFullPayloadPartitions"]["sourceDuplicateFullPayloadVertexCount"]
                for row in assets
            ),
            "runtimeVertexCount": sum(row["runtime"]["vertexCount"] for row in assets),
            "sourceColor0PresentButBlockedCount": sum(
                row["parity"]["color0"]["sourceAccessorPresent"] for row in assets
            ),
            "runtimeBoundsConsumedCount": 0,
            "compiledExecutionAllowedCount": 0,
        },
        "receiptSha256": "",
    }
    unsigned = json.loads(json.dumps(receipt))
    unsigned.pop("receiptSha256")
    receipt["receiptSha256"] = canonical_sha256(unsigned)
    return receipt


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build numeric glTF-to-WModel parity evidence for Artist F 31470."
    )
    parser.add_argument("--source-root", required=True, type=Path)
    parser.add_argument("--runtime-mesh-root", required=True, type=Path)
    parser.add_argument("--active-inventory", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    receipt = build_receipt(
        args.source_root,
        args.runtime_mesh_root,
        args.active_inventory,
    )
    content = json_bytes(receipt)
    if args.check:
        require(
            args.output.is_file() and args.output.read_bytes() == content,
            f"generated output is stale: {args.output}",
        )
    else:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_bytes(content)
    print(
        "Artist F 31470 WModel geometry parity "
        f"{'check' if args.check else 'write'}: "
        f"wmodels={receipt['summary']['wmodelCount']} "
        f"position={receipt['summary']['positionRelationProvenCount']} "
        f"topology={receipt['summary']['topologyAndReversedWindingProvenCount']} "
        f"handednessBlocked={receipt['summary']['tangentHandednessBlockedCount']}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, KeyError, struct.error) as error:
        print(str(error), file=sys.stderr)
        raise SystemExit(1)
