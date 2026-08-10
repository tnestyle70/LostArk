#!/usr/bin/env python3
"""Build a fail-closed WModel 1.1 geometry section from glTF evidence.

The bundled legacy converter remains the material/container producer.  This
tool verifies that its static mesh is the same geometry revision as the source
glTF, then replaces only the WMSH section with a versioned payload that keeps
tangent handedness, optional COLOR_0, WModel-space bounds, and pinned hashes.
It never changes an input file in place.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import json
import math
import os
import struct
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable


FILE_HEADER = struct.Struct("<4sHHII")
MODEL_HEADER = struct.Struct("<4sIII4I")
SECTION_DESC = struct.Struct("<IIQQ40s")
MESH_HEADER = struct.Struct("<4sIIIIIIIB3s")
SUBMESH_DESC = struct.Struct("<IIIIIQ20s")
BOUNDS_V1 = struct.Struct("<10f")
GEOMETRY_METADATA_PREFIX = struct.Struct("<4sHHIIIff")
GEOMETRY_METADATA_SIZE = GEOMETRY_METADATA_PREFIX.size + 10 * 32

WINT_VERSION_MAJOR = 1
WINT_LEGACY_VERSION_MINOR = 0
WINT_GEOMETRY_VERSION_MINOR = 1
VF_POSITION = 1 << 0
VF_NORMAL = 1 << 1
VF_TEXCOORD0 = 1 << 2
VF_TANGENT = 1 << 3
VF_STATIC_BASE = VF_POSITION | VF_NORMAL | VF_TEXCOORD0 | VF_TANGENT
VF_TANGENT_HANDEDNESS = 1 << 5
VF_COLOR0 = 1 << 6
STRIDE_STATIC = 48
STRIDE_STATIC_COLOR0 = 52
MAX_MATERIALS = 4096

MGEF_TANGENT_HANDEDNESS_PRESERVED_FROM_GLTF = 1 << 0
MGEF_COLOR0_PRESERVED_FROM_GLTF = 1 << 1
MGEF_BOUNDS_WMODEL_SPACE = 1 << 2
MGEF_SOURCE_GLTF_SHA256 = 1 << 3
MGEF_SOURCE_BUFFER_SET_SHA256 = 1 << 4
MGEF_SOURCE_PACKAGE_SHA256 = 1 << 5
MGEF_SOURCE_OBJECT_SHA256 = 1 << 6
MGEF_LEGACY_CONVERTER_SHA256 = 1 << 7
MGEF_GEOMETRY_TOOL_SHA256 = 1 << 8
MGEF_SOURCE_EXPORT_RECEIPT_SHA256 = 1 << 9
MGEF_LEGACY_COOK_RECEIPT_SHA256 = 1 << 10
MGEF_CLEAN_SOURCE_EXPORT = 1 << 11
MGEF_UPK_TO_GLTF_EXACT = 1 << 12
MGEF_PIVOT_EXACT = 1 << 13
MGEF_REQUIRED_PAYLOAD = (
    MGEF_TANGENT_HANDEDNESS_PRESERVED_FROM_GLTF
    | MGEF_BOUNDS_WMODEL_SPACE
    | MGEF_SOURCE_GLTF_SHA256
    | MGEF_SOURCE_BUFFER_SET_SHA256
    | MGEF_SOURCE_PACKAGE_SHA256
    | MGEF_SOURCE_OBJECT_SHA256
    | MGEF_LEGACY_CONVERTER_SHA256
    | MGEF_GEOMETRY_TOOL_SHA256
    | MGEF_SOURCE_EXPORT_RECEIPT_SHA256
    | MGEF_LEGACY_COOK_RECEIPT_SHA256
)
MGEF_PRODUCT_PROVENANCE = (
    MGEF_CLEAN_SOURCE_EXPORT | MGEF_UPK_TO_GLTF_EXACT | MGEF_PIVOT_EXACT
)
MGEF_KNOWN = (
    MGEF_REQUIRED_PAYLOAD
    | MGEF_COLOR0_PRESERVED_FROM_GLTF
    | MGEF_PRODUCT_PROVENANCE
)

COMPONENT_FORMATS: dict[int, tuple[str, int]] = {
    5120: ("b", 1),
    5121: ("B", 1),
    5122: ("h", 2),
    5123: ("H", 2),
    5125: ("I", 4),
    5126: ("f", 4),
}
TYPE_COMPONENT_COUNTS = {
    "SCALAR": 1,
    "VEC2": 2,
    "VEC3": 3,
    "VEC4": 4,
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def sha256_bytes(value: bytes) -> bytes:
    return hashlib.sha256(value).digest()


def sha256_file(path: Path) -> bytes:
    return sha256_bytes(path.read_bytes())


def parse_sha256(value: str, field: str) -> bytes:
    require(len(value) == 64, f"{field} must be a 64-character SHA-256")
    try:
        result = bytes.fromhex(value)
    except ValueError as error:
        raise ValueError(f"{field} is not hexadecimal") from error
    require(any(result), f"{field} may not be all zero")
    return result


def load_json_object(path: Path, label: str) -> tuple[dict[str, Any], bytes]:
    payload = path.read_bytes()
    value = json.loads(payload.decode("utf-8"))
    require(isinstance(value, dict), f"{label} root must be an object")
    return value, sha256_bytes(payload)


def unique_row(rows: Iterable[Any], predicate: Any, label: str) -> dict[str, Any]:
    matches = [value for value in rows if isinstance(value, dict) and predicate(value)]
    require(len(matches) == 1, f"{label} must resolve exactly one row")
    return matches[0]


def path_ends_with(path: Path, logical_path: str) -> bool:
    return path.resolve().as_posix().casefold().endswith(
        logical_path.replace("\\", "/").casefold()
    )


def load_pinned_provenance(
    source_object: str,
    source_gltf: Path,
    legacy_wmodel: Path,
    source_manifest_path: Path,
    source_export_receipt_path: Path,
    legacy_cook_receipt_path: Path,
    source_package_path: Path,
    legacy_converter_path: Path,
) -> tuple[Provenance, bytes, bytes]:
    manifest, manifest_sha256 = load_json_object(
        source_manifest_path, "source manifest"
    )
    export_receipt, export_receipt_sha256 = load_json_object(
        source_export_receipt_path, "source export receipt"
    )
    cook_receipt, cook_receipt_sha256 = load_json_object(
        legacy_cook_receipt_path, "legacy cook receipt"
    )
    require(
        str(export_receipt.get("sourceManifestSha256", "")).casefold()
        == manifest_sha256.hex(),
        "source export receipt does not pin the supplied source manifest",
    )
    manifest_row = unique_row(
        manifest.get("assets") or [],
        lambda value: value.get("sourceAssetPath") == source_object,
        "source manifest asset",
    )
    require(
        manifest_row.get("resolutionStatus") == "RESOLVED_SOURCE_PACKAGE"
        and source_package_path.name.casefold()
        == str(manifest_row.get("physicalPackage", "")).casefold(),
        "source package does not match the source manifest",
    )
    source_package_sha256 = sha256_file(source_package_path)
    require(any(source_package_sha256), "source package SHA-256 is empty")

    export_row = unique_row(
        export_receipt.get("assets") or [],
        lambda value: value.get("sourceAssetPath") == source_object,
        "source export receipt asset",
    )
    require(
        export_row.get("resolutionStatus") == "EXPORTED"
        and export_row.get("logicalPackage") == manifest_row.get("logicalPackage"),
        "source export receipt asset is not an exported manifest match",
    )
    output_rows = export_row.get("outputs") or []
    gltf_row = unique_row(
        output_rows,
        lambda value: str(value.get("relativePath", "")).casefold().endswith(
            "/" + source_gltf.name.casefold()
        ),
        "source export glTF output",
    )
    require(
        path_ends_with(source_gltf, str(gltf_row.get("relativePath", "")))
        and int(gltf_row.get("byteSize", -1)) == source_gltf.stat().st_size,
        "source glTF path or size does not match the export receipt",
    )
    source_gltf_sha256 = sha256_file(source_gltf)
    require(
        str(gltf_row.get("sha256", "")).casefold() == source_gltf_sha256.hex(),
        "source glTF SHA-256 does not match the export receipt",
    )
    gltf_document = json.loads(source_gltf.read_text(encoding="utf-8"))
    for buffer in gltf_document.get("buffers") or []:
        uri = str(buffer.get("uri", ""))
        require(uri and not uri.startswith("data:"), "source glTF buffer URI is invalid")
        buffer_path = source_gltf.parent / uri
        buffer_row = unique_row(
            output_rows,
            lambda value, name=buffer_path.name: str(
                value.get("relativePath", "")
            ).casefold().endswith("/" + name.casefold()),
            f"source export buffer output {uri}",
        )
        require(
            path_ends_with(buffer_path, str(buffer_row.get("relativePath", "")))
            and int(buffer_row.get("byteSize", -1)) == buffer_path.stat().st_size
            and str(buffer_row.get("sha256", "")).casefold()
            == sha256_file(buffer_path).hex(),
            f"source buffer {uri} does not match the export receipt",
        )

    require(
        str(cook_receipt.get("sourceExportReceiptSha256", "")).casefold()
        == export_receipt_sha256.hex()
        and math.isclose(float(cook_receipt.get("scale", math.nan)), 100.0),
        "legacy cook receipt does not pin the supplied export receipt and scale",
    )
    cook_row = unique_row(
        cook_receipt.get("assets") or [],
        lambda value: value.get("sourceAssetPath") == source_object
        and value.get("role") == "mesh",
        "legacy cook receipt mesh asset",
    )
    legacy_wmodel_sha256 = sha256_file(legacy_wmodel)
    require(
        cook_row.get("status") == "COOKED"
        and int(cook_row.get("converterExitCode", -1)) == 0
        and path_ends_with(source_gltf, str(cook_row.get("sourceFile", "")))
        and int(cook_row.get("byteSize", -1)) == legacy_wmodel.stat().st_size
        and str(cook_row.get("sha256", "")).casefold()
        == legacy_wmodel_sha256.hex(),
        "legacy WModel does not match the cook receipt",
    )
    legacy_converter_sha256 = sha256_file(legacy_converter_path)
    require(any(legacy_converter_sha256), "legacy converter SHA-256 is empty")
    return (
        Provenance(
            source_object=source_object,
            source_package_sha256=source_package_sha256,
            legacy_converter_sha256=legacy_converter_sha256,
            source_export_receipt_sha256=export_receipt_sha256,
            legacy_cook_receipt_sha256=cook_receipt_sha256,
        ),
        source_gltf_sha256,
        legacy_wmodel_sha256,
    )


def fixed_name(value: bytes) -> str:
    return value.split(b"\0", 1)[0].decode("utf-8", "strict")


def fixed_bytes(value: str, capacity: int) -> bytes:
    encoded = value.encode("utf-8")
    require(len(encoded) < capacity, f"name does not fit in {capacity} bytes: {value}")
    return encoded + bytes(capacity - len(encoded))


def f32(value: float) -> float:
    return struct.unpack("<f", struct.pack("<f", value))[0]


def f32_rounding_error_bound(value: float) -> float:
    """Return the half-ULP round-to-nearest bound around a finite float32."""
    rounded = f32(value)
    if rounded == 0.0:
        return 0.5 * struct.unpack("<f", struct.pack("<I", 1))[0]
    bits = struct.unpack("<I", struct.pack("<f", rounded))[0]
    neighbors = []
    if bits > 0:
        neighbors.append(struct.unpack("<f", struct.pack("<I", bits - 1))[0])
    if bits < 0x7F7FFFFF or bits > 0x80000000:
        neighbors.append(struct.unpack("<f", struct.pack("<I", bits + 1))[0])
    finite_neighbors = [candidate for candidate in neighbors if math.isfinite(candidate)]
    require(finite_neighbors, "cannot derive a finite float32 ULP bound")
    return 0.5 * max(abs(candidate - rounded) for candidate in finite_neighbors)


def pre_scaled_coordinate_error_bound(
    source_value: float, source_to_wmodel_scale: float, geometry_pre_scale: float
) -> float:
    ideal_scaled = source_value * source_to_wmodel_scale
    ideal_pre_scale = 1.0 / source_to_wmodel_scale
    cook_rounding = f32_rounding_error_bound(ideal_scaled)
    stored_pre_scale_error = abs(geometry_pre_scale - ideal_pre_scale)
    return (
        cook_rounding * abs(geometry_pre_scale)
        + abs(ideal_scaled) * stored_pre_scale_error
        + 4.0 * math.ulp(source_value)
    )


def canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def write_atomic(path: Path, value: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=path.name + ".", suffix=".tmp", dir=path.parent
    )
    try:
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(value)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary_name, path)
    except BaseException:
        try:
            os.unlink(temporary_name)
        except FileNotFoundError:
            pass
        raise


@dataclass(frozen=True)
class Provenance:
    source_object: str
    source_package_sha256: bytes
    legacy_converter_sha256: bytes
    source_export_receipt_sha256: bytes
    legacy_cook_receipt_sha256: bytes


@dataclass(frozen=True)
class Section:
    type_id: int
    index: int
    name_bytes: bytes
    payload: bytes


@dataclass(frozen=True)
class LegacySubmesh:
    material_index: int
    material_hash: int
    name_bytes: bytes
    vertices: tuple[tuple[float, ...], ...]
    indices: tuple[int, ...]


@dataclass(frozen=True)
class SourcePrimitive:
    vertices: tuple[dict[str, Any], ...]
    indices: tuple[int, ...]
    has_color0: bool


def accessor_values(
    document: dict[str, Any],
    source_directory: Path,
    accessor_index: int,
    buffer_cache: dict[int, bytes],
) -> tuple[list[tuple[int | float, ...]], dict[str, Any]]:
    accessors = document.get("accessors") or []
    buffer_views = document.get("bufferViews") or []
    buffers = document.get("buffers") or []
    require(0 <= accessor_index < len(accessors), "glTF accessor index is invalid")
    accessor = accessors[accessor_index]
    require("sparse" not in accessor, "sparse glTF accessors are not supported")
    view_index = int(accessor.get("bufferView", -1))
    require(0 <= view_index < len(buffer_views), "glTF bufferView index is invalid")
    view = buffer_views[view_index]
    buffer_index = int(view.get("buffer", -1))
    require(0 <= buffer_index < len(buffers), "glTF buffer index is invalid")
    if buffer_index not in buffer_cache:
        uri = buffers[buffer_index].get("uri")
        require(
            isinstance(uri, str)
            and uri
            and not uri.startswith("data:")
            and not Path(uri).is_absolute()
            and ".." not in Path(uri).parts,
            "glTF buffer URI must be a relative file path",
        )
        payload = (source_directory / Path(uri)).read_bytes()
        require(
            int(buffers[buffer_index].get("byteLength", -1)) == len(payload),
            f"glTF buffer byteLength does not match {uri}",
        )
        buffer_cache[buffer_index] = payload
    payload = buffer_cache[buffer_index]

    component_type = int(accessor.get("componentType", -1))
    value_type = str(accessor.get("type", ""))
    require(component_type in COMPONENT_FORMATS, "unsupported glTF component type")
    require(value_type in TYPE_COMPONENT_COUNTS, "unsupported glTF accessor type")
    format_code, component_size = COMPONENT_FORMATS[component_type]
    component_count = TYPE_COMPONENT_COUNTS[value_type]
    value_size = component_size * component_count
    count = int(accessor.get("count", -1))
    require(count >= 0, "glTF accessor count is invalid")
    stride = int(view.get("byteStride", value_size))
    require(stride >= value_size, "glTF accessor stride is invalid")
    offset = int(view.get("byteOffset", 0)) + int(accessor.get("byteOffset", 0))
    require(
        offset >= 0
        and (count == 0 or offset + (count - 1) * stride + value_size <= len(payload)),
        "glTF accessor exceeds its buffer",
    )
    unpacker = struct.Struct("<" + format_code * component_count)
    values = [unpacker.unpack_from(payload, offset + row * stride) for row in range(count)]
    evidence = {
        "accessorIndex": accessor_index,
        "componentType": component_type,
        "type": value_type,
        "normalized": bool(accessor.get("normalized", False)),
        "count": count,
    }
    return values, evidence


def node_is_identity(node: dict[str, Any]) -> bool:
    matrix = node.get("matrix")
    if matrix is not None:
        identity = [
            1.0,
            0.0,
            0.0,
            0.0,
            0.0,
            1.0,
            0.0,
            0.0,
            0.0,
            0.0,
            1.0,
            0.0,
            0.0,
            0.0,
            0.0,
            1.0,
        ]
        if len(matrix) != 16 or any(
            not math.isclose(float(left), right, abs_tol=1e-7)
            for left, right in zip(matrix, identity)
        ):
            return False
    expected = {
        "translation": (0.0, 0.0, 0.0),
        "rotation": (0.0, 0.0, 0.0, 1.0),
        "scale": (1.0, 1.0, 1.0),
    }
    return all(
        len(node.get(name, reference)) == len(reference)
        and all(
            math.isclose(float(left), right, abs_tol=1e-7)
            for left, right in zip(node.get(name, reference), reference)
        )
        for name, reference in expected.items()
    )


def parse_source_gltf(path: Path) -> tuple[list[SourcePrimitive], bytes, bytes]:
    gltf_bytes = path.read_bytes()
    document = json.loads(gltf_bytes.decode("utf-8"))
    require(isinstance(document, dict), "glTF root must be an object")
    nodes = document.get("nodes") or []
    parents: dict[int, int] = {}
    for parent_index, node in enumerate(nodes):
        for child in node.get("children", []):
            child_index = int(child)
            require(child_index not in parents, "glTF node has multiple parents")
            parents[child_index] = parent_index
    for node_index, node in enumerate(nodes):
        if "mesh" not in node:
            continue
        current: int | None = node_index
        while current is not None:
            require(node_is_identity(nodes[current]), "glTF mesh hierarchy changes the pivot")
            current = parents.get(current)

    buffer_cache: dict[int, bytes] = {}
    primitives: list[SourcePrimitive] = []
    color_presence: set[bool] = set()
    for mesh in document.get("meshes") or []:
        for primitive in mesh.get("primitives") or []:
            require(int(primitive.get("mode", 4)) == 4, "glTF primitive is not triangles")
            attributes = primitive.get("attributes") or {}
            required = ("POSITION", "NORMAL", "TEXCOORD_0", "TANGENT")
            require(
                all(name in attributes for name in required) and "indices" in primitive,
                "glTF primitive misses a required indexed geometry channel",
            )
            decoded: dict[str, list[tuple[int | float, ...]]] = {}
            evidence: dict[str, dict[str, Any]] = {}
            for name in required:
                decoded[name], evidence[name] = accessor_values(
                    document, path.parent, int(attributes[name]), buffer_cache
                )
            has_color0 = "COLOR_0" in attributes
            color_presence.add(has_color0)
            if has_color0:
                decoded["COLOR_0"], evidence["COLOR_0"] = accessor_values(
                    document, path.parent, int(attributes["COLOR_0"]), buffer_cache
                )
                require(
                    evidence["COLOR_0"]["componentType"] == 5121
                    and evidence["COLOR_0"]["type"] == "VEC4"
                    and evidence["COLOR_0"]["normalized"],
                    "COLOR_0 must be normalized RGBA8",
                )
            index_values, index_evidence = accessor_values(
                document, path.parent, int(primitive["indices"]), buffer_cache
            )
            require(
                index_evidence["type"] == "SCALAR"
                and index_evidence["componentType"] in (5121, 5123, 5125),
                "glTF indices must be unsigned scalar values",
            )
            count = len(decoded["POSITION"])
            require(
                count > 0
                and all(len(decoded[name]) == count for name in required)
                and (not has_color0 or len(decoded["COLOR_0"]) == count),
                "glTF vertex channel counts differ",
            )
            vertices: list[dict[str, Any]] = []
            for index in range(count):
                position = tuple(float(value) for value in decoded["POSITION"][index])
                normal = tuple(float(value) for value in decoded["NORMAL"][index])
                uv0 = tuple(float(value) for value in decoded["TEXCOORD_0"][index])
                tangent = tuple(float(value) for value in decoded["TANGENT"][index])
                require(
                    len(position) == 3
                    and len(normal) == 3
                    and len(uv0) == 2
                    and len(tangent) == 4
                    and all(math.isfinite(value) for value in (*position, *normal, *uv0, *tangent))
                    and math.isclose(abs(tangent[3]), 1.0, abs_tol=1e-6),
                    "glTF contains an invalid vertex channel value",
                )
                color = (
                    bytes(int(value) for value in decoded["COLOR_0"][index])
                    if has_color0
                    else None
                )
                vertices.append(
                    {
                        "position": position,
                        "normal": normal,
                        "uv0": uv0,
                        "tangent": tangent,
                        "color0": color,
                    }
                )
            indices = tuple(int(value[0]) for value in index_values)
            require(
                len(indices) > 0
                and len(indices) % 3 == 0
                and all(0 <= value < count for value in indices),
                "glTF triangle indices are invalid",
            )
            primitives.append(SourcePrimitive(tuple(vertices), indices, has_color0))
    require(primitives, "glTF contains no indexed mesh primitives")
    require(len(color_presence) == 1, "mixed COLOR_0 presence is not supported in WMSH 1.1")

    buffers = document.get("buffers") or []
    buffer_rows = []
    for buffer_index, descriptor in enumerate(buffers):
        uri = descriptor.get("uri")
        require(isinstance(uri, str) and buffer_index in buffer_cache, "glTF buffer is unresolved")
        buffer_rows.append(
            {
                "bufferIndex": buffer_index,
                "uri": uri.replace("\\", "/"),
                "bytes": len(buffer_cache[buffer_index]),
                "sha256": sha256_bytes(buffer_cache[buffer_index]).hex(),
            }
        )
    return primitives, sha256_bytes(gltf_bytes), sha256_bytes(canonical_json_bytes(buffer_rows))


def parse_legacy_mesh(payload: bytes) -> list[LegacySubmesh]:
    require(len(payload) >= FILE_HEADER.size + MESH_HEADER.size, "legacy WMSH is truncated")
    magic, major, minor, flags, content_size = FILE_HEADER.unpack_from(payload, 0)
    require(
        magic == b"WINT"
        and major == WINT_VERSION_MAJOR
        and minor == WINT_LEGACY_VERSION_MINOR
        and flags == 0
        and content_size == len(payload) - FILE_HEADER.size,
        "legacy WMSH WINT header is invalid",
    )
    offset = FILE_HEADER.size
    (
        mesh_magic,
        submesh_count,
        bone_count,
        vertex_flags,
        vertex_stride,
        total_vertices,
        total_indices,
        index_stride,
        has_bounds,
        reserved,
    ) = MESH_HEADER.unpack_from(payload, offset)
    require(
        mesh_magic == b"WMSH"
        and submesh_count > 0
        and bone_count == 0
        and vertex_flags == VF_STATIC_BASE
        and vertex_stride == STRIDE_STATIC
        and index_stride in (2, 4)
        and has_bounds in (0, 1)
        and reserved == b"\0\0\0",
        "legacy WMSH is not the supported static v1.0 layout",
    )
    offset += MESH_HEADER.size
    require(
        offset + submesh_count * SUBMESH_DESC.size <= len(payload),
        "legacy WMSH submesh table is truncated",
    )
    descriptors = [
        SUBMESH_DESC.unpack_from(payload, offset + row * SUBMESH_DESC.size)
        for row in range(submesh_count)
    ]
    offset += submesh_count * SUBMESH_DESC.size
    vertex_bytes = total_vertices * vertex_stride
    index_bytes = total_indices * index_stride
    require(
        offset + vertex_bytes + index_bytes <= len(payload),
        "legacy WMSH vertex or index block is truncated",
    )
    vertex_blob = payload[offset : offset + vertex_bytes]
    offset += vertex_bytes
    index_blob = payload[offset : offset + index_bytes]
    offset += index_bytes
    require(offset == len(payload) - has_bounds * submesh_count * BOUNDS_V1.size, "legacy WMSH trailing payload is invalid")

    result: list[LegacySubmesh] = []
    for descriptor in descriptors:
        (
            vertex_offset,
            vertex_count,
            index_offset,
            index_count,
            material_index,
            material_hash,
            name_bytes,
        ) = descriptor
        require(
            vertex_offset + vertex_count * vertex_stride <= len(vertex_blob)
            and index_offset + index_count * index_stride <= len(index_blob),
            "legacy WMSH submesh range is invalid",
        )
        require(
            material_index < MAX_MATERIALS,
            "legacy WMSH submesh material index exceeds the supported range",
        )
        vertices = tuple(
            struct.unpack_from(
                "<3f3f2f3ff", vertex_blob, vertex_offset + row * vertex_stride
            )
            for row in range(vertex_count)
        )
        index_format = "<H" if index_stride == 2 else "<I"
        indices = tuple(
            struct.unpack_from(index_format, index_blob, index_offset + row * index_stride)[0]
            for row in range(index_count)
        )
        require(all(value < vertex_count for value in indices), "legacy WMSH index is out of range")
        result.append(
            LegacySubmesh(
                material_index,
                material_hash,
                name_bytes,
                vertices,
                indices,
            )
        )
    return result


def parse_legacy_wmodel(data: bytes) -> tuple[tuple[int, int, int, tuple[int, ...]], list[Section], list[LegacySubmesh]]:
    require(len(data) >= FILE_HEADER.size + MODEL_HEADER.size, "legacy WModel is truncated")
    magic, major, minor, flags, content_size = FILE_HEADER.unpack_from(data, 0)
    require(
        magic == b"WINT"
        and major == WINT_VERSION_MAJOR
        and minor == WINT_LEGACY_VERSION_MINOR
        and flags == 0
        and content_size == len(data) - FILE_HEADER.size,
        "legacy WModel WINT header is invalid",
    )
    content_offset = FILE_HEADER.size
    model = MODEL_HEADER.unpack_from(data, content_offset)
    model_magic, section_count, animation_count, model_flags, *reserved = model
    require(
        model_magic == b"WMOD"
        and section_count == 2
        and animation_count == 0
        and model_flags == 0
        and all(value == 0 for value in reserved),
        "geometry contract cooker requires a static two-section WModel",
    )
    table_offset = content_offset + MODEL_HEADER.size
    require(
        table_offset + section_count * SECTION_DESC.size <= len(data),
        "legacy WModel section table is truncated",
    )
    sections: list[Section] = []
    mesh_sections = 0
    material_sections = 0
    legacy_meshes: list[LegacySubmesh] = []
    for row in range(section_count):
        type_id, index, offset, size, name_bytes = SECTION_DESC.unpack_from(
            data, table_offset + row * SECTION_DESC.size
        )
        require(offset <= content_size and size <= content_size - offset, "legacy WModel section is out of range")
        payload = data[content_offset + offset : content_offset + offset + size]
        if type_id == 1:
            mesh_sections += 1
            legacy_meshes = parse_legacy_mesh(payload)
        elif type_id == 2:
            material_sections += 1
        else:
            raise ValueError("static geometry contract WModel has an unsupported section type")
        sections.append(Section(type_id, index, name_bytes, payload))
    require(mesh_sections == 1 and material_sections == 1, "WModel requires one mesh and one material section")
    return (section_count, animation_count, model_flags, tuple(reserved)), sections, legacy_meshes


def parse_geometry_wmodel(data: bytes) -> dict[str, Any]:
    require(len(data) >= FILE_HEADER.size + MODEL_HEADER.size, "WModel 1.1 is truncated")
    magic, major, minor, flags, content_size = FILE_HEADER.unpack_from(data, 0)
    require(
        magic == b"WINT"
        and major == WINT_VERSION_MAJOR
        and minor == WINT_GEOMETRY_VERSION_MINOR
        and flags == 0
        and content_size == len(data) - FILE_HEADER.size,
        "WModel 1.1 outer header is invalid",
    )
    model = MODEL_HEADER.unpack_from(data, FILE_HEADER.size)
    require(
        model[0] == b"WMOD"
        and model[1] == 2
        and model[2] == 0
        and model[3] == 0
        and all(value == 0 for value in model[4:]),
        "WModel 1.1 is not a canonical static package",
    )
    mesh_sections: list[bytes] = []
    table = FILE_HEADER.size + MODEL_HEADER.size
    require(
        table + model[1] * SECTION_DESC.size <= len(data),
        "WModel 1.1 section table is truncated",
    )
    section_types: list[int] = []
    expected_section_offset = MODEL_HEADER.size + model[1] * SECTION_DESC.size
    for row in range(model[1]):
        type_id, _, offset, size, _ = SECTION_DESC.unpack_from(
            data, table + row * SECTION_DESC.size
        )
        require(
            offset == expected_section_offset
            and size > 0
            and offset <= content_size
            and size <= content_size - offset,
            "WModel 1.1 sections must be non-empty and contiguous",
        )
        expected_section_offset += size
        section_types.append(type_id)
        if type_id == 1:
            mesh_sections.append(
                data[
                    FILE_HEADER.size + offset :
                    FILE_HEADER.size + offset + size
                ]
            )
    require(
        expected_section_offset == content_size,
        "WModel 1.1 contains trailing section payload",
    )
    require(
        len(mesh_sections) == 1 and sorted(section_types) == [1, 2],
        "WModel 1.1 requires exactly one mesh and one material section",
    )
    mesh = mesh_sections[0]
    require(len(mesh) >= FILE_HEADER.size + MESH_HEADER.size + GEOMETRY_METADATA_SIZE, "WMSH 1.1 is truncated")
    mesh_outer = FILE_HEADER.unpack_from(mesh, 0)
    require(
        mesh_outer[:4] == (b"WINT", 1, 1, 0)
        and mesh_outer[4] == len(mesh) - FILE_HEADER.size,
        "WMSH 1.1 header is invalid",
    )
    header = MESH_HEADER.unpack_from(mesh, FILE_HEADER.size)
    (
        mesh_magic,
        submesh_count,
        bone_count,
        vertex_flags,
        vertex_stride,
        total_vertices,
        total_indices,
        index_stride,
        has_bounds,
        reserved,
    ) = header
    has_color0 = bool(vertex_flags & VF_COLOR0)
    expected_vertex_flags = (
        VF_STATIC_BASE
        | VF_TANGENT_HANDEDNESS
        | (VF_COLOR0 if has_color0 else 0)
    )
    expected_vertex_stride = STRIDE_STATIC_COLOR0 if has_color0 else STRIDE_STATIC
    require(
        mesh_magic == b"WMSH"
        and submesh_count > 0
        and bone_count == 0
        and vertex_flags == expected_vertex_flags
        and vertex_stride == expected_vertex_stride
        and total_vertices > 0
        and total_indices > 0
        and index_stride in (2, 4)
        and has_bounds == 1
        and reserved == b"\0\0\0",
        "WMSH 1.1 channel or stride contract is invalid",
    )
    offset = FILE_HEADER.size + MESH_HEADER.size
    descriptors = [
        SUBMESH_DESC.unpack_from(mesh, offset + row * SUBMESH_DESC.size)
        for row in range(submesh_count)
    ]
    offset += submesh_count * SUBMESH_DESC.size
    expected_vertex_offset = 0
    expected_index_offset = 0
    for descriptor in descriptors:
        require(
            descriptor[0] == expected_vertex_offset
            and descriptor[2] == expected_index_offset
            and descriptor[1] > 0
            and descriptor[3] > 0
            and descriptor[3] % 3 == 0,
            "WMSH 1.1 submesh ranges are not canonical triangle lists",
        )
        expected_vertex_offset += descriptor[1] * vertex_stride
        expected_index_offset += descriptor[3] * index_stride
    require(
        expected_vertex_offset == total_vertices * vertex_stride
        and expected_index_offset == total_indices * index_stride,
        "WMSH 1.1 aggregate vertex or index counts differ",
    )
    vertex_bytes = total_vertices * vertex_stride
    index_bytes = total_indices * index_stride
    bounds_bytes = submesh_count * BOUNDS_V1.size
    require(
        offset + vertex_bytes + index_bytes + bounds_bytes + GEOMETRY_METADATA_SIZE
        == len(mesh),
        "WMSH 1.1 payload size is invalid",
    )
    vertex_blob = mesh[offset : offset + vertex_bytes]
    offset += vertex_bytes
    index_blob = mesh[offset : offset + index_bytes]
    offset += index_bytes
    bounds_blob = mesh[offset : offset + bounds_bytes]
    offset += bounds_bytes
    require(offset + GEOMETRY_METADATA_SIZE == len(mesh), "WMSH 1.1 payload size is invalid")
    metadata = mesh[offset:]
    metadata_prefix = GEOMETRY_METADATA_PREFIX.unpack_from(metadata, 0)
    require(
        metadata_prefix[:3] == (b"WGEO", 1, 0)
        and metadata_prefix[3] == GEOMETRY_METADATA_SIZE
        and metadata_prefix[5] == offset - FILE_HEADER.size
        and metadata_prefix[4] & ~MGEF_KNOWN == 0
        and metadata_prefix[4] & MGEF_REQUIRED_PAYLOAD == MGEF_REQUIRED_PAYLOAD
        and metadata_prefix[4] & MGEF_PRODUCT_PROVENANCE == 0
        and bool(metadata_prefix[4] & MGEF_COLOR0_PRESERVED_FROM_GLTF)
        == has_color0
        and math.isfinite(metadata_prefix[6])
        and math.isfinite(metadata_prefix[7])
        and metadata_prefix[6] > 0.0
        and metadata_prefix[7] > 0.0
        and math.isclose(
            metadata_prefix[6] * metadata_prefix[7],
            1.0,
            rel_tol=1e-6,
            abs_tol=1e-6,
        ),
        "WMSH 1.1 metadata header is invalid",
    )
    digest_offset = GEOMETRY_METADATA_PREFIX.size
    digests = [
        metadata[digest_offset + row * 32 : digest_offset + (row + 1) * 32]
        for row in range(10)
    ]
    require(
        all(len(value) == 32 and any(value) for value in digests)
        and
        sha256_bytes(mesh[FILE_HEADER.size:offset]) == digests[0]
        and sha256_bytes(metadata[:-32]) == digests[-1],
        "WMSH 1.1 payload or metadata SHA-256 is invalid",
    )
    submeshes: list[dict[str, Any]] = []
    for row, descriptor in enumerate(descriptors):
        (
            vertex_offset,
            vertex_count,
            index_offset,
            index_count,
            material_index,
            material_hash,
            name_bytes,
        ) = descriptor
        require(
            vertex_offset + vertex_count * vertex_stride <= len(vertex_blob)
            and index_offset + index_count * index_stride <= len(index_blob),
            "WMSH 1.1 submesh range is invalid",
        )
        require(
            material_index < MAX_MATERIALS,
            "WMSH 1.1 submesh material index exceeds the supported range",
        )
        vertices = []
        vertex_bytes = bytearray()
        for index in range(vertex_count):
            begin = vertex_offset + index * vertex_stride
            raw = vertex_blob[begin : begin + vertex_stride]
            values = struct.unpack_from("<3f3f2f3ff", raw, 0)
            require(
                all(math.isfinite(value) for value in values)
                and math.isclose(abs(values[11]), 1.0, abs_tol=1e-6),
                "WMSH 1.1 vertex channel or tangent W is invalid",
            )
            color = raw[STRIDE_STATIC:STRIDE_STATIC_COLOR0] if has_color0 else None
            vertices.append({"values": values, "color0": color})
            vertex_bytes.extend(raw)
        index_format = "<H" if index_stride == 2 else "<I"
        indices = tuple(
            struct.unpack_from(index_format, index_blob, index_offset + index * index_stride)[0]
            for index in range(index_count)
        )
        require(
            all(value < vertex_count for value in indices),
            "WMSH 1.1 index is out of range",
        )
        bounds = BOUNDS_V1.unpack_from(bounds_blob, row * BOUNDS_V1.size)
        positions = [value["values"][:3] for value in vertices]
        minimum = tuple(min(value[axis] for value in positions) for axis in range(3))
        maximum = tuple(max(value[axis] for value in positions) for axis in range(3))
        center = tuple(0.5 * (minimum[axis] + maximum[axis]) for axis in range(3))
        radius = max(
            math.sqrt(
                sum((value[axis] - center[axis]) ** 2 for axis in range(3))
            )
            for value in positions
        )
        require(
            all(math.isfinite(value) for value in bounds)
            and all(bounds[axis] <= bounds[axis + 3] for axis in range(3))
            and all(
                math.isclose(bounds[axis], minimum[axis], rel_tol=1e-5, abs_tol=1e-5)
                and math.isclose(bounds[axis + 3], maximum[axis], rel_tol=1e-5, abs_tol=1e-5)
                and math.isclose(bounds[axis + 6], center[axis], rel_tol=1e-5, abs_tol=1e-5)
                for axis in range(3)
            )
            and bounds[9] >= 0.0
            and math.isclose(bounds[9], radius, rel_tol=1e-5, abs_tol=1e-5),
            "WMSH 1.1 embedded bounds differ from its vertices",
        )
        submeshes.append(
            {
                "name": fixed_name(name_bytes),
                "materialIndex": material_index,
                "materialHash": material_hash,
                "vertices": tuple(vertices),
                "indices": indices,
                "vertexBytes": bytes(vertex_bytes),
                "indexBytes": index_blob[index_offset : index_offset + index_count * index_stride],
                "bounds": bounds,
            }
        )
    return {
        "vertexFlags": vertex_flags,
        "vertexStride": vertex_stride,
        "indexStride": index_stride,
        "hasColor0": has_color0,
        "evidenceFlags": metadata_prefix[4],
        "sourceToWModelScale": metadata_prefix[6],
        "geometryPreScale": metadata_prefix[7],
        "payloadSha256": digests[0],
        "sourceGltfSha256": digests[1],
        "sourceBufferSetSha256": digests[2],
        "provenanceSha256": digests[-1],
        "submeshes": submeshes,
    }


def verify_source_against_geometry_contract(
    source_gltf: Path, geometry_wmodel: Path
) -> dict[str, Any]:
    source_primitives, source_gltf_sha256, source_buffer_set_sha256 = parse_source_gltf(
        source_gltf
    )
    runtime = parse_geometry_wmodel(geometry_wmodel.read_bytes())
    require(
        runtime["sourceGltfSha256"] == source_gltf_sha256
        and runtime["sourceBufferSetSha256"] == source_buffer_set_sha256,
        "WModel 1.1 metadata does not identify the supplied glTF and buffers",
    )
    require(
        len(source_primitives) == len(runtime["submeshes"]),
        "source/WModel 1.1 submesh count differs",
    )
    source_w_counts: collections.Counter[str] = collections.Counter()
    runtime_w_counts: collections.Counter[str] = collections.Counter()
    color_bytes = 0
    total_vertices = 0
    total_indices = 0
    maximum_pre_scaled_position_error = 0.0
    maximum_pre_scaled_bounds_error = 0.0
    maximum_pre_scaled_position_error_bound = 0.0
    maximum_pre_scaled_bounds_error_bound = 0.0
    maximum_bitangent_error = 0.0
    asymmetric_bounds_fixture = False
    for source, target in zip(source_primitives, runtime["submeshes"]):
        target_values = tuple(value["values"] for value in target["vertices"])
        target_stub = LegacySubmesh(
            target["materialIndex"],
            target["materialHash"],
            fixed_bytes(target["name"], 20),
            target_values,
            target["indices"],
        )
        expected_vertices, expected_indices, expected_bounds, _ = build_submesh_payload(
            source,
            target_stub,
            runtime["sourceToWModelScale"],
            runtime["indexStride"],
        )
        require(expected_vertices == target["vertexBytes"], "WModel 1.1 vertex payload is not source-exact")
        require(expected_indices == target["indexBytes"], "WModel 1.1 winding/topology payload differs")
        require(
            all(math.isclose(left, right, rel_tol=1e-6, abs_tol=1e-5) for left, right in zip(expected_bounds, target["bounds"])),
            "WModel 1.1 embedded bounds differ from the vertex-derived oracle",
        )
        reflected_source_positions = [
            (
                float(vertex["position"][0]),
                float(vertex["position"][1]),
                -float(vertex["position"][2]),
            )
            for vertex in source.vertices
        ]
        source_minimum = [
            min(value[axis] for value in reflected_source_positions)
            for axis in range(3)
        ]
        source_maximum = [
            max(value[axis] for value in reflected_source_positions)
            for axis in range(3)
        ]
        source_center = [
            0.5 * (source_minimum[axis] + source_maximum[axis])
            for axis in range(3)
        ]
        asymmetric_bounds_fixture = asymmetric_bounds_fixture or any(
            abs(value) > 1e-6 for value in source_center
        )
        pre_scale = runtime["geometryPreScale"]
        target_pre_scaled_bounds = [
            value * pre_scale for value in (*target["bounds"][:6],)
        ]
        maximum_pre_scaled_bounds_error = max(
            maximum_pre_scaled_bounds_error,
            *(
                abs(left - right)
                for left, right in zip(
                    target_pre_scaled_bounds,
                    (*source_minimum, *source_maximum),
                )
            ),
        )
        maximum_pre_scaled_bounds_error_bound = max(
            maximum_pre_scaled_bounds_error_bound,
            *(
                pre_scaled_coordinate_error_bound(
                    value,
                    runtime["sourceToWModelScale"],
                    pre_scale,
                )
                for value in (*source_minimum, *source_maximum)
            ),
        )
        for vertex in source.vertices:
            transformed, _ = transform_source_vertex(
                vertex, runtime["sourceToWModelScale"]
            )
            expected_position = (
                float(vertex["position"][0]),
                float(vertex["position"][1]),
                -float(vertex["position"][2]),
            )
            maximum_pre_scaled_position_error = max(
                maximum_pre_scaled_position_error,
                *(
                    abs(transformed[axis] * pre_scale - expected_position[axis])
                    for axis in range(3)
                ),
            )
            maximum_pre_scaled_position_error_bound = max(
                maximum_pre_scaled_position_error_bound,
                *(
                    pre_scaled_coordinate_error_bound(
                        expected_position[axis],
                        runtime["sourceToWModelScale"],
                        pre_scale,
                    )
                    for axis in range(3)
                ),
            )
            maximum_bitangent_error = max(
                maximum_bitangent_error,
                verify_reflected_tangent_basis(
                    tuple(float(value) for value in vertex["normal"]),
                    tuple(float(value) for value in vertex["tangent"]),
                    (transformed[3], transformed[4], transformed[5]),
                    (
                        transformed[8],
                        transformed[9],
                        transformed[10],
                        transformed[11],
                    ),
                ),
            )
        source_w_counts.update(f"{vertex['tangent'][3]:.1f}" for vertex in source.vertices)
        runtime_w_counts.update(f"{vertex['values'][11]:.1f}" for vertex in target["vertices"])
        color_bytes += sum(len(vertex["color0"] or b"") for vertex in target["vertices"])
        total_vertices += len(target["vertices"])
        total_indices += len(target["indices"])
    require(
        0 == runtime["evidenceFlags"] & MGEF_PRODUCT_PROVENANCE,
        "unclosed UPK/pivot provenance may not be promoted",
    )
    require(
        maximum_pre_scaled_position_error
        <= maximum_pre_scaled_position_error_bound
        and maximum_pre_scaled_bounds_error
        <= maximum_pre_scaled_bounds_error_bound,
        "pre-scaled position or AABB exceeds the float32 ULP error bound",
    )
    return {
        "sourceObjectGltfSha256": source_gltf_sha256.hex(),
        "wmodelSha256": sha256_file(geometry_wmodel).hex(),
        "submeshCount": len(runtime["submeshes"]),
        "vertexCount": total_vertices,
        "indexCount": total_indices,
        "sourceTangentWCounts": dict(sorted(source_w_counts.items())),
        "runtimeTangentWCounts": dict(sorted(runtime_w_counts.items())),
        "tangentWRelation": "runtime=-source_after_Z_reflection",
        "bitangentBasisStatus": "PROVEN_NUMERIC",
        "maximumBitangentBasisError": maximum_bitangent_error,
        "color0PayloadBytes": color_bytes,
        "boundsStatus": "EMBEDDED_EQUALS_VERTEX_DERIVED",
        "maximumPositionErrorAfterGeometryPreScale": maximum_pre_scaled_position_error,
        "maximumPositionFloat32ErrorBoundAfterGeometryPreScale":
            maximum_pre_scaled_position_error_bound,
        "maximumBoundsErrorAfterGeometryPreScale": maximum_pre_scaled_bounds_error,
        "maximumBoundsFloat32ErrorBoundAfterGeometryPreScale":
            maximum_pre_scaled_bounds_error_bound,
        "geometryPreScaleStoredErrorFromExactReciprocal": abs(
            runtime["geometryPreScale"] - 1.0 / runtime["sourceToWModelScale"]
        ),
        "asymmetricBoundsFixture": asymmetric_bounds_fixture,
        "recenterApplied": False,
        "pivotStatus": "UPK_TO_GLTF_PIVOT_UNRESOLVED",
        "productAdmission": False,
    }


def normalized_cross(
    left: tuple[float, float, float], right: tuple[float, float, float]
) -> tuple[float, float, float]:
    value = (
        left[1] * right[2] - left[2] * right[1],
        left[2] * right[0] - left[0] * right[2],
        left[0] * right[1] - left[1] * right[0],
    )
    length = math.sqrt(sum(component * component for component in value))
    require(math.isfinite(length) and length > 1e-8, "normal/tangent basis is degenerate")
    return tuple(component / length for component in value)


def verify_reflected_tangent_basis(
    source_normal: tuple[float, float, float],
    source_tangent: tuple[float, float, float, float],
    runtime_normal: tuple[float, float, float],
    runtime_tangent: tuple[float, float, float, float],
) -> float:
    source_cross = normalized_cross(source_normal, source_tangent[:3])
    source_bitangent = tuple(value * source_tangent[3] for value in source_cross)
    expected_runtime_bitangent = (
        source_bitangent[0],
        source_bitangent[1],
        -source_bitangent[2],
    )
    runtime_cross = normalized_cross(runtime_normal, runtime_tangent[:3])
    runtime_bitangent = tuple(value * runtime_tangent[3] for value in runtime_cross)
    maximum_error = max(
        abs(left - right)
        for left, right in zip(expected_runtime_bitangent, runtime_bitangent)
    )
    require(
        maximum_error <= 2e-6,
        "reflected tangent basis does not preserve the source bitangent",
    )
    return maximum_error


def transform_source_vertex(vertex: dict[str, Any], scale: float) -> tuple[tuple[float, ...], bytes | None]:
    position = vertex["position"]
    normal = vertex["normal"]
    uv0 = vertex["uv0"]
    tangent = vertex["tangent"]
    values = (
        f32(position[0] * scale),
        f32(position[1] * scale),
        f32(-position[2] * scale),
        f32(normal[0]),
        f32(normal[1]),
        f32(-normal[2]),
        f32(uv0[0]),
        f32(uv0[1]),
        f32(tangent[0]),
        f32(tangent[1]),
        f32(-tangent[2]),
        f32(-tangent[3]),
    )
    verify_reflected_tangent_basis(
        tuple(float(value) for value in normal),
        tuple(float(value) for value in tangent),
        (values[3], values[4], values[5]),
        (values[8], values[9], values[10], values[11]),
    )
    return values, vertex["color0"]


def geometry_key(vertex: tuple[float, ...]) -> bytes:
    return struct.pack("<3f3f2f3f", *vertex[:11])


def triangle_signatures(
    vertices: Iterable[tuple[float, ...]], indices: Iterable[int]
) -> collections.Counter[tuple[tuple[bytes, bytes, bytes], int]]:
    vertex_list = list(vertices)
    index_list = list(indices)
    result: collections.Counter[tuple[tuple[bytes, bytes, bytes], int]] = collections.Counter()
    for offset in range(0, len(index_list), 3):
        values = [geometry_key(vertex_list[index_list[offset + index]]) for index in range(3)]
        order = sorted(range(3), key=values.__getitem__)
        inversions = sum(
            order[left] > order[right]
            for left in range(3)
            for right in range(left + 1, 3)
        )
        result[(tuple(values[index] for index in order), inversions % 2)] += 1
    return result


def build_submesh_payload(
    source: SourcePrimitive,
    legacy: LegacySubmesh,
    scale: float,
    index_stride: int,
) -> tuple[bytes, bytes, tuple[float, ...], dict[str, int]]:
    transformed = [transform_source_vertex(vertex, scale) for vertex in source.vertices]
    unique_rows: list[tuple[tuple[float, ...], bytes | None]] = []
    unique_lookup: dict[bytes, int] = {}
    source_to_unique: list[int] = []
    for values, color in transformed:
        packed = struct.pack("<3f3f2f3ff", *values)
        if source.has_color0:
            require(color is not None and len(color) == 4, "COLOR_0 row is invalid")
            packed += color
        unique_index = unique_lookup.get(packed)
        if unique_index is None:
            unique_index = len(unique_rows)
            unique_lookup[packed] = unique_index
            unique_rows.append((values, color))
        source_to_unique.append(unique_index)

    target_indices: list[int] = []
    for offset in range(0, len(source.indices), 3):
        a, b, c = source.indices[offset : offset + 3]
        target_indices.extend((source_to_unique[a], source_to_unique[c], source_to_unique[b]))
    target_vertices = [row[0] for row in unique_rows]
    require(
        triangle_signatures(target_vertices, target_indices)
        == triangle_signatures(legacy.vertices, legacy.indices),
        f"source glTF and legacy WModel topology differ for {fixed_name(legacy.name_bytes)}",
    )

    vertex_blob = bytearray()
    for values, color in unique_rows:
        vertex_blob.extend(struct.pack("<3f3f2f3ff", *values))
        if source.has_color0:
            vertex_blob.extend(color or b"")
    require(
        index_stride == 4 or len(unique_rows) <= 0xFFFF,
        "16-bit WMSH index format cannot address the rebuilt vertices",
    )
    index_format = "<H" if index_stride == 2 else "<I"
    index_blob = b"".join(struct.pack(index_format, value) for value in target_indices)

    minimum = [min(values[axis] for values in target_vertices) for axis in range(3)]
    maximum = [max(values[axis] for values in target_vertices) for axis in range(3)]
    center = [f32(0.5 * (minimum[axis] + maximum[axis])) for axis in range(3)]
    radius = f32(
        max(
            math.sqrt(
                sum((values[axis] - center[axis]) ** 2 for axis in range(3))
            )
            for values in target_vertices
        )
    )
    bounds = tuple(f32(value) for value in (*minimum, *maximum, *center, radius))
    return (
        bytes(vertex_blob),
        index_blob,
        bounds,
        {
            "sourceVertexCount": len(source.vertices),
            "runtimeVertexCount": len(unique_rows),
            "duplicateFullPayloadVertexCount": len(source.vertices) - len(unique_rows),
            "indexCount": len(target_indices),
        },
    )


def build_geometry_metadata(
    payload: bytes,
    source_gltf_sha256: bytes,
    source_buffer_set_sha256: bytes,
    provenance: Provenance,
    source_to_wmodel_scale: float,
    geometry_pre_scale: float,
    has_color0: bool,
) -> tuple[bytes, int]:
    evidence_flags = MGEF_REQUIRED_PAYLOAD
    if has_color0:
        evidence_flags |= MGEF_COLOR0_PRESERVED_FROM_GLTF
    prefix = GEOMETRY_METADATA_PREFIX.pack(
        b"WGEO",
        1,
        0,
        GEOMETRY_METADATA_SIZE,
        evidence_flags,
        len(payload),
        f32(source_to_wmodel_scale),
        f32(geometry_pre_scale),
    )
    geometry_tool_sha256 = sha256_file(Path(__file__))
    digests = (
        sha256_bytes(payload),
        source_gltf_sha256,
        source_buffer_set_sha256,
        provenance.source_package_sha256,
        sha256_bytes(provenance.source_object.encode("utf-8")),
        provenance.legacy_converter_sha256,
        geometry_tool_sha256,
        provenance.source_export_receipt_sha256,
        provenance.legacy_cook_receipt_sha256,
    )
    require(all(len(value) == 32 and any(value) for value in digests), "geometry metadata digest is empty")
    unsigned = prefix + b"".join(digests)
    require(len(unsigned) == GEOMETRY_METADATA_SIZE - 32, "geometry metadata layout changed")
    return unsigned + sha256_bytes(unsigned), evidence_flags


def build_mesh_section(
    source_primitives: list[SourcePrimitive],
    legacy_submeshes: list[LegacySubmesh],
    provenance: Provenance,
    source_gltf_sha256: bytes,
    source_buffer_set_sha256: bytes,
    source_to_wmodel_scale: float,
    geometry_pre_scale: float,
) -> tuple[bytes, dict[str, Any]]:
    require(len(source_primitives) == len(legacy_submeshes), "glTF/WModel primitive count differs")
    has_color0 = source_primitives[0].has_color0
    require(all(value.has_color0 == has_color0 for value in source_primitives), "mixed COLOR_0 presence")
    index_stride = 4 if any(len(value.vertices) > 0xFFFF for value in source_primitives) else 2
    vertex_stride = STRIDE_STATIC_COLOR0 if has_color0 else STRIDE_STATIC
    vertex_blocks: list[bytes] = []
    index_blocks: list[bytes] = []
    bounds_rows: list[tuple[float, ...]] = []
    summaries: list[dict[str, int]] = []
    descriptors: list[bytes] = []
    vertex_offset = 0
    index_offset = 0
    for source, legacy in zip(source_primitives, legacy_submeshes):
        vertex_blob, index_blob, bounds, summary = build_submesh_payload(
            source, legacy, source_to_wmodel_scale, index_stride
        )
        vertex_count = len(vertex_blob) // vertex_stride
        index_count = len(index_blob) // index_stride
        descriptors.append(
            SUBMESH_DESC.pack(
                vertex_offset,
                vertex_count,
                index_offset,
                index_count,
                legacy.material_index,
                legacy.material_hash,
                legacy.name_bytes,
            )
        )
        vertex_blocks.append(vertex_blob)
        index_blocks.append(index_blob)
        bounds_rows.append(bounds)
        summaries.append(summary)
        vertex_offset += len(vertex_blob)
        index_offset += len(index_blob)

    vertex_flags = (
        VF_STATIC_BASE
        | VF_TANGENT_HANDEDNESS
        | (VF_COLOR0 if has_color0 else 0)
    )
    mesh_header = MESH_HEADER.pack(
        b"WMSH",
        len(descriptors),
        0,
        vertex_flags,
        vertex_stride,
        sum(len(value) // vertex_stride for value in vertex_blocks),
        sum(len(value) // index_stride for value in index_blocks),
        index_stride,
        1,
        b"\0\0\0",
    )
    payload = (
        mesh_header
        + b"".join(descriptors)
        + b"".join(vertex_blocks)
        + b"".join(index_blocks)
        + b"".join(BOUNDS_V1.pack(*row) for row in bounds_rows)
    )
    metadata, evidence_flags = build_geometry_metadata(
        payload,
        source_gltf_sha256,
        source_buffer_set_sha256,
        provenance,
        source_to_wmodel_scale,
        geometry_pre_scale,
        has_color0,
    )
    content = payload + metadata
    section = FILE_HEADER.pack(
        b"WINT",
        WINT_VERSION_MAJOR,
        WINT_GEOMETRY_VERSION_MINOR,
        0,
        len(content),
    ) + content
    return section, {
        "formatVersion": "1.1",
        "vertexFormatFlags": vertex_flags,
        "vertexStride": vertex_stride,
        "hasColor0": has_color0,
        "hasWModelSpaceBounds": True,
        "tangentHandednessTransform": "runtimeW=-sourceW_after_Z_reflection",
        "channelPayloadStatus": "PRESERVED_FROM_GLTF",
        "sourceFidelityStatus": "UNKNOWN_UPK_TO_GLTF_BLOCKED",
        "evidenceFlags": evidence_flags,
        "productProvenanceComplete":
            (evidence_flags & MGEF_PRODUCT_PROVENANCE) == MGEF_PRODUCT_PROVENANCE,
        "submeshes": summaries,
    }


def rebuild_wmodel(
    model_header: tuple[int, int, int, tuple[int, ...]],
    sections: list[Section],
    mesh_section: bytes,
) -> bytes:
    section_count, animation_count, model_flags, reserved = model_header
    replacement_count = 0
    output_sections: list[Section] = []
    for section in sections:
        if section.type_id == 1:
            output_sections.append(Section(section.type_id, section.index, section.name_bytes, mesh_section))
            replacement_count += 1
        else:
            output_sections.append(section)
    require(replacement_count == 1, "WModel mesh replacement count is invalid")
    content_prefix_size = MODEL_HEADER.size + section_count * SECTION_DESC.size
    offset = content_prefix_size
    descriptors: list[bytes] = []
    payloads: list[bytes] = []
    for section in output_sections:
        descriptors.append(
            SECTION_DESC.pack(
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
        MODEL_HEADER.pack(
            b"WMOD",
            section_count,
            animation_count,
            model_flags,
            *reserved,
        )
        + b"".join(descriptors)
        + b"".join(payloads)
    )
    return FILE_HEADER.pack(
        b"WINT",
        WINT_VERSION_MAJOR,
        WINT_GEOMETRY_VERSION_MINOR,
        0,
        len(content),
    ) + content


def cook_wmodel_geometry_contract(
    source_gltf: Path,
    legacy_wmodel: Path,
    provenance: Provenance,
    source_to_wmodel_scale: float = 100.0,
    geometry_pre_scale: float = 0.01,
    expected_source_gltf_sha256: bytes | None = None,
    expected_legacy_wmodel_sha256: bytes | None = None,
) -> tuple[bytes, dict[str, Any]]:
    require(
        math.isfinite(source_to_wmodel_scale)
        and math.isfinite(geometry_pre_scale)
        and source_to_wmodel_scale > 0.0
        and geometry_pre_scale > 0.0
        and math.isclose(
            source_to_wmodel_scale * geometry_pre_scale,
            1.0,
            rel_tol=1e-6,
            abs_tol=1e-6,
        ),
        "sourceToWModelScale and geometryPreScale must be finite positive inverses",
    )
    source_primitives, source_gltf_sha256, source_buffer_set_sha256 = parse_source_gltf(source_gltf)
    legacy_bytes = legacy_wmodel.read_bytes()
    legacy_sha256 = sha256_bytes(legacy_bytes)
    if expected_source_gltf_sha256 is not None:
        require(source_gltf_sha256 == expected_source_gltf_sha256, "source glTF SHA-256 changed")
    if expected_legacy_wmodel_sha256 is not None:
        require(legacy_sha256 == expected_legacy_wmodel_sha256, "legacy WModel SHA-256 changed")
    model_header, sections, legacy_submeshes = parse_legacy_wmodel(legacy_bytes)
    mesh_section, geometry_summary = build_mesh_section(
        source_primitives,
        legacy_submeshes,
        provenance,
        source_gltf_sha256,
        source_buffer_set_sha256,
        source_to_wmodel_scale,
        geometry_pre_scale,
    )
    output = rebuild_wmodel(model_header, sections, mesh_section)
    product_blockers = [
        "PIVOT_EXACT_UNRESOLVED",
        "UPK_TO_GLTF_EXACT_UNRESOLVED",
        "CLEAN_SOURCE_EXPORT_UNPROVEN",
        "LEGACY_CONVERTER_HISTORICAL_INVOCATION_UNPROVEN",
        "RUNTIME_RESOURCE_NOT_REPLACED",
        "RUNTIME_GEOMETRY_PRESCALE_NOT_CONSUMED",
    ]
    if geometry_summary["hasColor0"]:
        product_blockers.append("COLOR0_SHADER_CONSUMPTION_NOT_IMPLEMENTED")
    receipt = {
        "schema": "lostark.wmodel-geometry-contract-cook-receipt",
        "formatVersion": 1,
        "sourceObject": provenance.source_object,
        "sourceGltf": str(source_gltf).replace("\\", "/"),
        "sourceGltfSha256": source_gltf_sha256.hex(),
        "sourceBufferSetSha256": source_buffer_set_sha256.hex(),
        "legacyWModel": str(legacy_wmodel).replace("\\", "/"),
        "legacyWModelSha256": legacy_sha256.hex(),
        "sourceToWModelScale": source_to_wmodel_scale,
        "geometryPreScale": geometry_pre_scale,
        "geometry": geometry_summary,
        "outputBytes": len(output),
        "outputSha256": sha256_bytes(output).hex(),
        "runtimeProductAdmission": False,
        "runtimeProductBlockers": product_blockers,
    }
    return output, receipt


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Cook a WModel 1.1 static geometry contract without replacing the input"
    )
    parser.add_argument("--source-gltf", required=True, type=Path)
    parser.add_argument("--legacy-wmodel", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--receipt", type=Path)
    parser.add_argument("--source-object", required=True)
    parser.add_argument("--source-manifest", required=True, type=Path)
    parser.add_argument("--source-export-receipt", required=True, type=Path)
    parser.add_argument("--legacy-cook-receipt", required=True, type=Path)
    parser.add_argument("--source-package", required=True, type=Path)
    parser.add_argument("--legacy-converter", required=True, type=Path)
    parser.add_argument("--source-to-wmodel-scale", type=float, default=100.0)
    parser.add_argument("--geometry-pre-scale", type=float, default=0.01)
    args = parser.parse_args()

    (
        provenance,
        expected_source_gltf_sha256,
        expected_legacy_wmodel_sha256,
    ) = load_pinned_provenance(
        args.source_object,
        args.source_gltf,
        args.legacy_wmodel,
        args.source_manifest,
        args.source_export_receipt,
        args.legacy_cook_receipt,
        args.source_package,
        args.legacy_converter,
    )
    output, receipt = cook_wmodel_geometry_contract(
        args.source_gltf,
        args.legacy_wmodel,
        provenance,
        args.source_to_wmodel_scale,
        args.geometry_pre_scale,
        expected_source_gltf_sha256,
        expected_legacy_wmodel_sha256,
    )
    write_atomic(args.output, output)
    if args.receipt is not None:
        write_atomic(
            args.receipt,
            json.dumps(receipt, ensure_ascii=False, indent=2).encode("utf-8") + b"\n",
        )
    print(
        "WModel geometry contract cook: "
        f"output={args.output} sha256={receipt['outputSha256']} "
        f"product={str(receipt['runtimeProductAdmission']).lower()}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
