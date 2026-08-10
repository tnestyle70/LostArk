from __future__ import annotations

import hashlib
import json
import struct
import sys
import tempfile
import unittest
from pathlib import Path
from typing import Any

from cook_wmodel_geometry_contract import (
    BOUNDS_V1,
    FILE_HEADER,
    GEOMETRY_METADATA_PREFIX,
    GEOMETRY_METADATA_SIZE,
    MESH_HEADER,
    MGEF_COLOR0_PRESERVED_FROM_GLTF,
    MGEF_PRODUCT_PROVENANCE,
    MODEL_HEADER,
    SECTION_DESC,
    STRIDE_STATIC,
    SUBMESH_DESC,
    TANGENT_HANDEDNESS_ABS_TOLERANCE,
    VF_STATIC_BASE,
    GeometryProvenanceEvidence,
    cook_wmodel_geometry_contract,
    fixed_bytes,
    load_geometry_provenance_evidence,
    parse_geometry_wmodel,
    sha256_bytes,
    transform_source_vertex,
    verify_source_against_geometry_contract,
)
from verify_artist_31470_wmodel_geometry_contract import strict_json_equal


SOURCE_VERTICES: tuple[dict[str, Any], ...] = (
    {
        "position": (1.0, 2.0, 3.0),
        "normal": (0.0, 1.0, 0.0),
        "uv0": (0.0, 0.0),
        "tangent": (1.0, 0.0, 0.0, 1.0),
        "color0": bytes((0x11, 0x22, 0x33, 0x44)),
    },
    {
        "position": (4.0, 2.0, 3.0),
        "normal": (0.0, 1.0, 0.0),
        "uv0": (1.0, 0.0),
        "tangent": (1.0, 0.0, 0.0, -1.0),
        "color0": bytes((0x55, 0x66, 0x77, 0x88)),
    },
    {
        "position": (1.0, 6.0, 8.0),
        "normal": (0.0, 1.0, 0.0),
        "uv0": (0.0, 1.0),
        "tangent": (1.0, 0.0, 0.0, 1.0),
        "color0": bytes((0x99, 0xAA, 0xBB, 0xCC)),
    },
)

MESH_BONE_ENTRY = struct.Struct("<Q32si16fI16s")
SKELETON_HEADER = struct.Struct("<4sIII4I")
SKELETON_BONE = struct.Struct("<Q64si16fII27I")
GLOBAL_ROOT = struct.Struct("<16f16I")
ANIMATION_HEADER = struct.Struct("<4sIffIIB7s")
ANIMATION_CHANNEL = struct.Struct("<QIIIIIIiI")
VECTOR_KEY = struct.Struct("<f3f")
QUATERNION_KEY = struct.Struct("<f4f")


def _json_bytes(value: Any) -> bytes:
    return (json.dumps(value, ensure_ascii=False, indent=2) + "\n").encode("utf-8")


def _append_aligned(buffer: bytearray, value: bytes, alignment: int = 4) -> int:
    while len(buffer) % alignment:
        buffer.append(0)
    offset = len(buffer)
    buffer.extend(value)
    return offset


def _write_source_gltf(root: Path, name: str, with_color0: bool) -> Path:
    root.mkdir(parents=True, exist_ok=True)
    buffer = bytearray()
    views: list[dict[str, int]] = []
    accessors: list[dict[str, Any]] = []

    def add_accessor(
        payload: bytes,
        component_type: int,
        value_type: str,
        count: int,
        *,
        normalized: bool = False,
    ) -> int:
        offset = _append_aligned(buffer, payload)
        views.append({"buffer": 0, "byteOffset": offset, "byteLength": len(payload)})
        accessor: dict[str, Any] = {
            "bufferView": len(views) - 1,
            "componentType": component_type,
            "count": count,
            "type": value_type,
        }
        if normalized:
            accessor["normalized"] = True
        accessors.append(accessor)
        return len(accessors) - 1

    positions = add_accessor(
        b"".join(struct.pack("<3f", *row["position"]) for row in SOURCE_VERTICES),
        5126,
        "VEC3",
        len(SOURCE_VERTICES),
    )
    normals = add_accessor(
        b"".join(struct.pack("<3f", *row["normal"]) for row in SOURCE_VERTICES),
        5126,
        "VEC3",
        len(SOURCE_VERTICES),
    )
    uv0 = add_accessor(
        b"".join(struct.pack("<2f", *row["uv0"]) for row in SOURCE_VERTICES),
        5126,
        "VEC2",
        len(SOURCE_VERTICES),
    )
    tangents = add_accessor(
        b"".join(struct.pack("<4f", *row["tangent"]) for row in SOURCE_VERTICES),
        5126,
        "VEC4",
        len(SOURCE_VERTICES),
    )
    attributes = {
        "POSITION": positions,
        "NORMAL": normals,
        "TEXCOORD_0": uv0,
        "TANGENT": tangents,
    }
    if with_color0:
        attributes["COLOR_0"] = add_accessor(
            b"".join(row["color0"] for row in SOURCE_VERTICES),
            5121,
            "VEC4",
            len(SOURCE_VERTICES),
            normalized=True,
        )
    indices = add_accessor(struct.pack("<3H", 0, 1, 2), 5123, "SCALAR", 3)
    buffer_name = f"{name}.bin"
    (root / buffer_name).write_bytes(buffer)
    document = {
        "asset": {"version": "2.0"},
        "buffers": [{"uri": buffer_name, "byteLength": len(buffer)}],
        "bufferViews": views,
        "accessors": accessors,
        "meshes": [
            {"primitives": [{"attributes": attributes, "indices": indices, "mode": 4}]}
        ],
        "nodes": [{"mesh": 0}],
        "scenes": [{"nodes": [0]}],
        "scene": 0,
    }
    path = root / f"{name}.gltf"
    path.write_bytes(_json_bytes(document))
    return path


def _wrap_wint(payload: bytes, minor: int = 0) -> bytes:
    return FILE_HEADER.pack(b"WINT", 1, minor, 0, len(payload)) + payload


def _build_legacy_wmodel(path: Path) -> Path:
    transformed = [transform_source_vertex(dict(row), 100.0)[0] for row in SOURCE_VERTICES]
    vertex_blob = b"".join(
        struct.pack("<3f3f2f3ff", *values) for values in transformed
    )
    index_blob = struct.pack("<3H", 0, 2, 1)
    mesh_content = (
        MESH_HEADER.pack(
            b"WMSH",
            1,
            0,
            VF_STATIC_BASE,
            STRIDE_STATIC,
            3,
            3,
            2,
            0,
            b"\0\0\0",
        )
        + SUBMESH_DESC.pack(
            0,
            3,
            0,
            3,
            0,
            0x31470,
            fixed_bytes("fixture", 20),
        )
        + vertex_blob
        + index_blob
    )
    mesh_section = _wrap_wint(mesh_content)
    material_section = _wrap_wint(struct.pack("<4sI", b"WMAT", 0))
    section_offset = MODEL_HEADER.size + 2 * SECTION_DESC.size
    model_content = (
        MODEL_HEADER.pack(b"WMOD", 2, 0, 0, 0, 0, 0, 0)
        + SECTION_DESC.pack(
            1,
            0,
            section_offset,
            len(mesh_section),
            fixed_bytes("fixture-mesh", 40),
        )
        + SECTION_DESC.pack(
            2,
            0,
            section_offset + len(mesh_section),
            len(material_section),
            fixed_bytes("fixture-material", 40),
        )
        + mesh_section
        + material_section
    )
    path.write_bytes(_wrap_wint(model_content))
    return path


def _identity_matrix() -> tuple[float, ...]:
    return (
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    )


def _build_legacy_static_multisubmesh(path: Path) -> Path:
    transformed = [
        transform_source_vertex(dict(row), 100.0)[0] for row in SOURCE_VERTICES
    ]
    one_vertex_block = b"".join(
        struct.pack("<3f3f2f3ff", *values) for values in transformed
    )
    one_index_block = struct.pack("<3H", 0, 2, 1)
    content = (
        MESH_HEADER.pack(
            b"WMSH", 2, 0, VF_STATIC_BASE, STRIDE_STATIC,
            6, 6, 2, 1, b"\0\0\0",
        )
        + SUBMESH_DESC.pack(
            0, 3, 0, 3, 0, 0x31470, fixed_bytes("legacy-static-a", 20)
        )
        + SUBMESH_DESC.pack(
            len(one_vertex_block), 3, len(one_index_block), 3,
            1, 0x31471, fixed_bytes("legacy-static-b", 20),
        )
        + one_vertex_block
        + one_vertex_block
        + one_index_block
        + one_index_block
        + BOUNDS_V1.pack(*([0.0] * 10))
        + BOUNDS_V1.pack(*([0.0] * 10))
    )
    path.write_bytes(_wrap_wint(content))
    return path


def _fnv_skeleton_hash(bone_hashes: tuple[int, ...]) -> int:
    value = 0xCBF29CE484222325
    for bone_hash in bone_hashes:
        value ^= bone_hash
        value = (value * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
    return value


def _build_legacy_skinned_fixture(root: Path) -> tuple[Path, Path, Path]:
    root.mkdir(parents=True, exist_ok=True)
    mesh_path = root / "legacy_skinned.wmesh"
    skeleton_path = root / "legacy_skinned.wskel"
    animation_path = root / "legacy_skinned.wanim"
    bone_hash = 0xA31470
    identity = _identity_matrix()

    transformed = [
        transform_source_vertex(dict(row), 100.0)[0] for row in SOURCE_VERTICES
    ]
    vertex_blob = b"".join(
        struct.pack(
            "<3f3f2f3f4I4f",
            *values[:11],
            0, 0, 0, 0,
            1.0, 0.0, 0.0, 0.0,
        )
        for values in transformed
    )
    index_blob = struct.pack("<3H", 0, 2, 1)
    mesh_content = (
        MESH_HEADER.pack(
            b"WMSH", 1, 1, VF_STATIC_BASE | (1 << 4), 76,
            3, 3, 2, 1, b"\0\0\0",
        )
        + SUBMESH_DESC.pack(
            0, 3, 0, 3, 0, 0x31472, fixed_bytes("legacy-skinned", 20)
        )
        + vertex_blob
        + index_blob
        + MESH_BONE_ENTRY.pack(
            bone_hash,
            fixed_bytes("root", 32),
            -1,
            *identity,
            0,
            bytes(16),
        )
        + BOUNDS_V1.pack(*([0.0] * 10))
    )
    mesh_path.write_bytes(_wrap_wint(mesh_content))

    skeleton_content = (
        SKELETON_HEADER.pack(b"WSKL", 1, 0, 0, 0, 0, 0, 0)
        + SKELETON_BONE.pack(
            bone_hash,
            fixed_bytes("root", 64),
            -1,
            *identity,
            0,
            0,
            *([0] * 27),
        )
        + GLOBAL_ROOT.pack(*identity, *([0] * 16))
    )
    skeleton_path.write_bytes(_wrap_wint(skeleton_content))

    position_key = VECTOR_KEY.pack(0.0, 0.0, 0.0, 0.0)
    rotation_key = QUATERNION_KEY.pack(0.0, 0.0, 0.0, 0.0, 1.0)
    scale_key = VECTOR_KEY.pack(0.0, 1.0, 1.0, 1.0)
    key_block = position_key + rotation_key + scale_key
    animation_content = (
        ANIMATION_HEADER.pack(b"WANM", 1, 1.0, 1.0, 3, 0, 1, bytes(7))
        + ANIMATION_CHANNEL.pack(
            bone_hash,
            1, 0,
            1, len(position_key),
            1, len(position_key) + len(rotation_key),
            0,
            0,
        )
        + key_block
        + struct.pack("<Q", _fnv_skeleton_hash((bone_hash,)))
    )
    animation_path.write_bytes(_wrap_wint(animation_content))
    return mesh_path, skeleton_path, animation_path


def _provenance() -> GeometryProvenanceEvidence:
    return GeometryProvenanceEvidence(
        source_object="fixture.mesh",
        source_manifest_canonical_lf_sha256=sha256_bytes(b"fixture-manifest\n"),
        source_manifest_legacy_hash_correlation=
            "LEGACY_RAW_SHA256_MATCHES_CANONICAL_LF_VARIANT",
        source_package_observed_unbound_sha256=sha256_bytes(b"fixture-package"),
        legacy_converter_observed_unbound_sha256=sha256_bytes(b"fixture-converter"),
        source_export_receipt_sha256=sha256_bytes(b"fixture-export-receipt"),
        legacy_cook_receipt_sha256=sha256_bytes(b"fixture-cook-receipt"),
    )


def _cook_fixture(root: Path, name: str, with_color0: bool) -> tuple[Path, Path, dict[str, Any]]:
    source = _write_source_gltf(root / name / "source", name, with_color0)
    legacy = _build_legacy_wmodel(root / name / f"{name}.legacy.wmodel")
    output, receipt = cook_wmodel_geometry_contract(source, legacy, _provenance())
    candidate = root / name / f"{name}.wmodel"
    candidate.write_bytes(output)
    return source, candidate, receipt


def _mesh_layout(value: bytes | bytearray) -> dict[str, int]:
    model = MODEL_HEADER.unpack_from(value, FILE_HEADER.size)
    table = FILE_HEADER.size + MODEL_HEADER.size
    mesh_start = -1
    mesh_size = -1
    material_start = -1
    for row in range(model[1]):
        type_id, _, offset, size, _ = SECTION_DESC.unpack_from(
            value, table + row * SECTION_DESC.size
        )
        if type_id == 1:
            mesh_start = FILE_HEADER.size + offset
            mesh_size = size
        elif type_id == 2:
            material_start = FILE_HEADER.size + offset
    if mesh_start < 0:
        raise ValueError("fixture WModel has no mesh section")
    if material_start < 0:
        raise ValueError("fixture WModel has no material section")
    mesh_header = mesh_start + FILE_HEADER.size
    header = MESH_HEADER.unpack_from(value, mesh_header)
    submesh_count = header[1]
    vertex_stride = header[4]
    total_vertices = header[5]
    total_indices = header[6]
    index_stride = header[7]
    vertices = mesh_header + MESH_HEADER.size + submesh_count * SUBMESH_DESC.size
    indices = vertices + total_vertices * vertex_stride
    bounds = indices + total_indices * index_stride
    metadata = mesh_start + mesh_size - GEOMETRY_METADATA_SIZE
    return {
        "meshStart": mesh_start,
        "materialStart": material_start,
        "meshHeader": mesh_header,
        "vertices": vertices,
        "bounds": bounds,
        "metadata": metadata,
        "metadataDigests": metadata + GEOMETRY_METADATA_PREFIX.size,
    }


def _resign_geometry(value: bytearray) -> None:
    layout = _mesh_layout(value)
    payload = bytes(value[layout["meshStart"] + FILE_HEADER.size : layout["metadata"]])
    digest_start = layout["metadataDigests"]
    value[digest_start : digest_start + 32] = hashlib.sha256(payload).digest()
    metadata_hash = hashlib.sha256(
        bytes(value[layout["metadata"] : layout["metadata"] + GEOMETRY_METADATA_SIZE - 32])
    ).digest()
    value[
        layout["metadata"] + GEOMETRY_METADATA_SIZE - 32 :
        layout["metadata"] + GEOMETRY_METADATA_SIZE
    ] = metadata_hash


def _with_resigned_tangent_w(valid: bytes, tangent_w: float) -> bytes:
    value = bytearray(valid)
    layout = _mesh_layout(value)
    struct.pack_into("<f", value, layout["vertices"] + 44, tangent_w)
    _resign_geometry(value)
    return bytes(value)


def _corruptions(valid: bytes) -> dict[str, bytes]:
    result: dict[str, bytes] = {}

    result["corrupt_truncated.wmodel"] = valid[:-1]

    value = bytearray(valid)
    struct.pack_into("<I", value, 12, len(value))
    result["corrupt_header.wmodel"] = bytes(value)

    value = bytearray(valid)
    struct.pack_into("<H", value, 6, 2)
    result["corrupt_outer_version.wmodel"] = bytes(value)

    value = bytearray(valid)
    struct.pack_into("<I", value, FILE_HEADER.size + 12, 1)
    result["corrupt_outer_metadata.wmodel"] = bytes(value)

    value = bytearray(valid)
    model = MODEL_HEADER.unpack_from(value, FILE_HEADER.size)
    table = FILE_HEADER.size + MODEL_HEADER.size
    material_row = next(
        row
        for row in range(model[1])
        if SECTION_DESC.unpack_from(value, table + row * SECTION_DESC.size)[0] == 2
    )
    material_desc = table + material_row * SECTION_DESC.size
    material_offset = struct.unpack_from("<Q", value, material_desc + 8)[0]
    value.insert(FILE_HEADER.size + material_offset, 0)
    struct.pack_into("<I", value, 12, len(value) - FILE_HEADER.size)
    struct.pack_into("<Q", value, material_desc + 8, material_offset + 1)
    result["corrupt_section_gap.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    struct.pack_into("<H", value, layout["meshStart"] + 6, 2)
    result["corrupt_mesh_version.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    material_magic = layout["materialStart"] + FILE_HEADER.size
    value[material_magic : material_magic + 4] = b"BAD!"
    result["corrupt_material_container.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    struct.pack_into("<I", value, layout["meshHeader"] + 16, 49)
    result["corrupt_stride.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    first_material_index = layout["meshHeader"] + MESH_HEADER.size + 16
    struct.pack_into("<I", value, first_material_index, 0xFFFFFFFF)
    _resign_geometry(value)
    result["corrupt_material_index.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    flags = struct.unpack_from("<I", value, layout["meshHeader"] + 12)[0]
    struct.pack_into("<I", value, layout["meshHeader"] + 12, flags & ~(1 << 5))
    result["corrupt_channel.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    flags = struct.unpack_from("<I", value, layout["meshHeader"] + 12)[0]
    struct.pack_into("<I", value, layout["meshHeader"] + 12, flags | (1 << 7))
    result["corrupt_unknown_channel.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    struct.pack_into("<f", value, layout["vertices"] + 44, 0.0)
    _resign_geometry(value)
    result["corrupt_tangent_w.wmodel"] = bytes(value)

    result["corrupt_tangent_w_boundary_out.wmodel"] = _with_resigned_tangent_w(
        valid, -1.000001072883606
    )
    result["corrupt_tangent_w_minus_1_000005.wmodel"] = _with_resigned_tangent_w(
        valid, -1.000005
    )

    value = bytearray(valid)
    layout = _mesh_layout(value)
    struct.pack_into("<3f", value, layout["vertices"] + 12, 0.0, 0.0, 0.0)
    _resign_geometry(value)
    result["corrupt_zero_normal.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    struct.pack_into("<3f", value, layout["vertices"] + 32, 0.0, 0.0, 0.0)
    _resign_geometry(value)
    result["corrupt_zero_tangent.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    normal = struct.unpack_from("<3f", value, layout["vertices"] + 12)
    struct.pack_into("<3f", value, layout["vertices"] + 32, *normal)
    _resign_geometry(value)
    result["corrupt_parallel_basis.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    struct.pack_into("<2f", value, layout["metadata"] + 20, 1e20, 1e20)
    _resign_geometry(value)
    result["corrupt_scale_overflow_pair.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    struct.pack_into("<2f", value, layout["metadata"] + 20, 100.0, 0.02)
    _resign_geometry(value)
    result["corrupt_scale_wrong_reciprocal.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    value[layout["metadataDigests"]] ^= 0x80
    result["corrupt_payload_hash.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    value[layout["metadata"] + GEOMETRY_METADATA_SIZE - 1] ^= 0x80
    result["corrupt_metadata_hash.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    minimum_x = struct.unpack_from("<f", value, layout["bounds"])[0]
    struct.pack_into("<f", value, layout["bounds"], minimum_x + 17.0)
    _resign_geometry(value)
    result["corrupt_bounds.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    struct.pack_into("<f", value, layout["bounds"], float("nan"))
    _resign_geometry(value)
    result["corrupt_bounds_nonfinite.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    maximum_x = struct.unpack_from("<f", value, layout["bounds"] + 12)[0]
    struct.pack_into("<f", value, layout["bounds"], maximum_x + 1.0)
    _resign_geometry(value)
    result["corrupt_bounds_inverted.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    maximum_float32 = struct.unpack("<f", struct.pack("<I", 0x7F7FFFFF))[0]
    struct.pack_into("<f", value, layout["vertices"], maximum_float32)
    minimum = (100.0, 200.0, -800.0)
    maximum = (maximum_float32, 600.0, -300.0)
    center = tuple(
        struct.unpack(
            "<f",
            struct.pack(
                "<f", 0.5 * minimum[axis] + 0.5 * maximum[axis]
            ),
        )[0]
        for axis in range(3)
    )
    struct.pack_into(
        "<10f",
        value,
        layout["bounds"],
        *minimum,
        *maximum,
        *center,
        maximum_float32,
    )
    _resign_geometry(value)
    result["corrupt_bounds_float_intermediate.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    struct.pack_into("<H", value, layout["metadata"] + 4, 2)
    _resign_geometry(value)
    result["corrupt_metadata_version.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    package_digest = layout["metadataDigests"] + 3 * 32
    value[package_digest : package_digest + 32] = bytes(32)
    _resign_geometry(value)
    result["corrupt_provenance.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    evidence_offset = layout["metadata"] + 12
    evidence = struct.unpack_from("<I", value, evidence_offset)[0]
    struct.pack_into(
        "<I",
        value,
        evidence_offset,
        evidence & ~MGEF_COLOR0_PRESERVED_FROM_GLTF,
    )
    _resign_geometry(value)
    result["corrupt_evidence.wmodel"] = bytes(value)

    value = bytearray(valid)
    layout = _mesh_layout(value)
    evidence_offset = layout["metadata"] + 12
    evidence = struct.unpack_from("<I", value, evidence_offset)[0]
    struct.pack_into("<I", value, evidence_offset, evidence | (1 << 11))
    _resign_geometry(value)
    result["corrupt_unverified_source_fidelity.wmodel"] = bytes(value)
    return result


def write_harness_suite(output_root: Path) -> None:
    output_root.mkdir(parents=True, exist_ok=True)
    _, valid_color, _ = _cook_fixture(output_root, "valid_color", True)
    _, valid_no_color, _ = _cook_fixture(output_root, "valid_no_color", False)
    (output_root / "valid_color.wmodel").write_bytes(valid_color.read_bytes())
    (output_root / "valid_no_color.wmodel").write_bytes(valid_no_color.read_bytes())
    (output_root / "valid_tangent_w_boundary_in.wmodel").write_bytes(
        _with_resigned_tangent_w(valid_color.read_bytes(), -1.0000009536743164)
    )
    legacy = output_root / "valid_color" / "valid_color.legacy.wmodel"
    (output_root / "legacy_v10.wmodel").write_bytes(legacy.read_bytes())
    _build_legacy_static_multisubmesh(
        output_root / "legacy_static_multisubmesh_bounds.wmesh"
    )
    _, valid_skeleton, valid_animation = _build_legacy_skinned_fixture(output_root)
    corrupt_skeleton = bytearray(valid_skeleton.read_bytes())
    corrupt_skeleton[FILE_HEADER.size : FILE_HEADER.size + 4] = b"BAD!"
    (output_root / "corrupt_skeleton.wskel").write_bytes(corrupt_skeleton)
    corrupt_animation = bytearray(valid_animation.read_bytes())
    corrupt_animation[FILE_HEADER.size : FILE_HEADER.size + 4] = b"BAD!"
    (output_root / "corrupt_animation.wanim").write_bytes(corrupt_animation)
    invalid_names = []
    for name, payload in _corruptions(valid_color.read_bytes()).items():
        (output_root / name).write_bytes(payload)
        invalid_names.append(name)
    (output_root / "suite.json").write_bytes(
        _json_bytes(
            {
                "valid": ["valid_color.wmodel", "valid_no_color.wmodel"],
                "invalid": sorted(invalid_names),
            }
        )
    )


class WModelGeometryContractTests(unittest.TestCase):
    def test_round_trip_preserves_channels_basis_bounds_and_blockers(self) -> None:
        self.assertTrue(strict_json_equal({"b": 2, "a": 1}, {"a": 1, "b": 2}))
        self.assertFalse(strict_json_equal({"value": True}, {"value": 1}))
        self.assertFalse(strict_json_equal({"value": 1.0}, {"value": 1}))
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source, candidate, receipt = _cook_fixture(root, "valid_color", True)
            parsed = parse_geometry_wmodel(candidate.read_bytes())
            oracle = verify_source_against_geometry_contract(source, candidate)

            self.assertTrue(parsed["hasColor0"])
            self.assertEqual(parsed["vertexStride"], 52)
            self.assertEqual(oracle["color0PayloadBytes"], 12)
            self.assertEqual(oracle["runtimeTangentWCounts"], {"-1.0": 2, "1.0": 1})
            self.assertEqual(oracle["bitangentBasisStatus"], "PROVEN_NUMERIC")
            self.assertLessEqual(oracle["maximumBitangentBasisError"], 2e-6)
            self.assertLessEqual(
                oracle["maximumPositionErrorAfterGeometryPreScale"],
                oracle["maximumPositionFloat32ErrorBoundAfterGeometryPreScale"],
            )
            self.assertLessEqual(
                oracle["maximumBoundsErrorAfterGeometryPreScale"],
                oracle["maximumBoundsFloat32ErrorBoundAfterGeometryPreScale"],
            )
            self.assertTrue(oracle["asymmetricBoundsFixture"])
            self.assertFalse(oracle["recenterApplied"])
            self.assertEqual(oracle["pivotStatus"], "UPK_TO_GLTF_PIVOT_UNRESOLVED")
            self.assertEqual(parsed["evidenceFlags"] & MGEF_PRODUCT_PROVENANCE, 0)
            self.assertFalse(receipt["runtimeProductAdmission"])
            self.assertIn(
                "RUNTIME_GEOMETRY_PRESCALE_NOT_CONSUMED",
                receipt["runtimeProductBlockers"],
            )
            self.assertIn(
                "LEGACY_CONVERTER_HISTORICAL_INVOCATION_UNPROVEN",
                receipt["runtimeProductBlockers"],
            )
            self.assertIn(
                "COLOR0_SHADER_CONSUMPTION_NOT_IMPLEMENTED",
                receipt["runtimeProductBlockers"],
            )

    def test_optional_color_channel_is_explicitly_absent(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            _, candidate, receipt = _cook_fixture(root, "valid_no_color", False)
            parsed = parse_geometry_wmodel(candidate.read_bytes())
            self.assertFalse(parsed["hasColor0"])
            self.assertEqual(parsed["vertexStride"], 48)
            self.assertEqual(
                parsed["evidenceFlags"] & MGEF_COLOR0_PRESERVED_FROM_GLTF, 0
            )
            self.assertNotIn(
                "COLOR0_SHADER_CONSUMPTION_NOT_IMPLEMENTED",
                receipt["runtimeProductBlockers"],
            )

    def test_corrupt_version_stride_channel_hash_bounds_and_metadata_fail_closed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source, candidate, _ = _cook_fixture(root, "valid_color", True)
            boundary_inside = _with_resigned_tangent_w(
                candidate.read_bytes(), -1.0000009536743164
            )
            self.assertLessEqual(
                abs(abs(struct.unpack_from(
                    "<f", boundary_inside, _mesh_layout(boundary_inside)["vertices"] + 44
                )[0]) - 1.0),
                TANGENT_HANDEDNESS_ABS_TOLERANCE,
            )
            parse_geometry_wmodel(boundary_inside)
            for name, payload in _corruptions(candidate.read_bytes()).items():
                with self.subTest(name=name):
                    with self.assertRaises(ValueError):
                        parse_geometry_wmodel(payload)

            legacy = root / "valid_color" / "valid_color.legacy.wmodel"
            with self.assertRaisesRegex(ValueError, "finite positive inverses"):
                cook_wmodel_geometry_contract(
                    source, legacy, _provenance(), 1e20, 1e20
                )
            with self.assertRaisesRegex(ValueError, "finite positive inverses"):
                cook_wmodel_geometry_contract(
                    source, legacy, _provenance(), 100.0, 0.02
                )

            source_document = json.loads(source.read_text(encoding="utf-8"))
            position_view = source_document["accessors"][0]["bufferView"]
            source_document["bufferViews"][position_view]["byteLength"] = 1
            source.write_bytes(_json_bytes(source_document))
            with self.assertRaisesRegex(ValueError, "exceeds its bufferView"):
                cook_wmodel_geometry_contract(source, legacy, _provenance())

    def test_role_aware_provenance_and_manifest_eol_contract(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = _write_source_gltf(root / "source", "fixture", True)
            legacy = _build_legacy_wmodel(root / "fixture.legacy.wmodel")
            package = root / "fixture-package.upk"
            converter = root / "ModelAssetConverter.exe"
            package.write_bytes(b"fixture-package")
            converter.write_bytes(b"fixture-converter")
            manifest_path = root / "source-manifest.json"
            manifest = {
                "schema": "lostark.class-effect-resource-source-manifest",
                "formatVersion": 1,
                "assets": [
                    {
                        "sourceAssetPath": "fixture.mesh",
                        "resolutionStatus": "RESOLVED_SOURCE_PACKAGE",
                        "physicalPackage": package.name,
                        "logicalPackage": "FX_FIXTURE",
                    }
                ]
            }
            canonical_manifest = _json_bytes(manifest)
            manifest_path.write_bytes(canonical_manifest)
            export_receipt_path = root / "export-receipt.json"
            outputs = [
                {
                    "relativePath": f"source/{source.name}",
                    "byteSize": source.stat().st_size,
                    "sha256": hashlib.sha256(source.read_bytes()).hexdigest(),
                },
                {
                    "relativePath": f"source/{source.with_suffix('.bin').name}",
                    "byteSize": source.with_suffix(".bin").stat().st_size,
                    "sha256": hashlib.sha256(
                        source.with_suffix(".bin").read_bytes()
                    ).hexdigest(),
                },
            ]
            export_receipt = {
                "schema": "lostark.effect-resource-export-receipt",
                "formatVersion": 1,
                "sourceManifestSha256": hashlib.sha256(
                    canonical_manifest.replace(b"\n", b"\r\n")
                ).hexdigest(),
                "assets": [
                    {
                        "sourceAssetPath": "fixture.mesh",
                        "resolutionStatus": "EXPORTED",
                        "logicalPackage": "FX_FIXTURE",
                        "outputs": outputs,
                    }
                ],
            }
            export_receipt_path.write_bytes(_json_bytes(export_receipt))
            cook_receipt_path = root / "cook-receipt.json"
            cook_receipt = {
                "schema": "lostark.effect-runtime-resource-cook-receipt",
                "formatVersion": 1,
                "sourceExportReceiptSha256": hashlib.sha256(
                    export_receipt_path.read_bytes()
                ).hexdigest(),
                "scale": 100.0,
                "assets": [
                    {
                        "sourceAssetPath": "fixture.mesh",
                        "role": "mesh",
                        "status": "COOKED",
                        "converterExitCode": 0,
                        "sourceFile": f"source/{source.name}",
                        "byteSize": legacy.stat().st_size,
                        "sha256": hashlib.sha256(legacy.read_bytes()).hexdigest(),
                    }
                ],
            }
            cook_receipt_path.write_bytes(_json_bytes(cook_receipt))

            provenance, gltf_digest, wmodel_digest = load_geometry_provenance_evidence(
                "fixture.mesh",
                source,
                legacy,
                manifest_path,
                export_receipt_path,
                cook_receipt_path,
                package,
                converter,
            )
            self.assertEqual(
                provenance.source_package_observed_unbound_sha256,
                sha256_bytes(package.read_bytes()),
            )
            self.assertEqual(
                provenance.source_manifest_legacy_hash_correlation,
                "LEGACY_RAW_SHA256_MATCHES_CANONICAL_CRLF_VARIANT",
            )
            self.assertEqual(gltf_digest, sha256_bytes(source.read_bytes()))
            self.assertEqual(wmodel_digest, sha256_bytes(legacy.read_bytes()))

            lf_output, lf_receipt = cook_wmodel_geometry_contract(
                source,
                legacy,
                provenance,
                expected_source_gltf_sha256=gltf_digest,
                expected_legacy_wmodel_sha256=wmodel_digest,
            )
            manifest_path.write_bytes(canonical_manifest.replace(b"\n", b"\r\n"))
            crlf_provenance, crlf_gltf_digest, crlf_wmodel_digest = (
                load_geometry_provenance_evidence(
                    "fixture.mesh",
                    source,
                    legacy,
                    manifest_path,
                    export_receipt_path,
                    cook_receipt_path,
                    package,
                    converter,
                )
            )
            crlf_output, crlf_receipt = cook_wmodel_geometry_contract(
                source,
                legacy,
                crlf_provenance,
                expected_source_gltf_sha256=crlf_gltf_digest,
                expected_legacy_wmodel_sha256=crlf_wmodel_digest,
            )
            self.assertEqual(
                provenance.source_manifest_canonical_lf_sha256,
                crlf_provenance.source_manifest_canonical_lf_sha256,
            )
            self.assertEqual(lf_output, crlf_output)
            self.assertEqual(lf_receipt, crlf_receipt)

            manifest_path.write_bytes(b"\xef\xbb\xbf" + canonical_manifest)
            with self.assertRaisesRegex(ValueError, "UTF-8 BOM"):
                load_geometry_provenance_evidence(
                    "fixture.mesh",
                    source,
                    legacy,
                    manifest_path,
                    export_receipt_path,
                    cook_receipt_path,
                    package,
                    converter,
                )

            semantic_manifest = dict(manifest)
            semantic_manifest["semanticMutation"] = True
            manifest_path.write_bytes(_json_bytes(semantic_manifest))
            with self.assertRaisesRegex(ValueError, "legacy raw manifest SHA-256"):
                load_geometry_provenance_evidence(
                    "fixture.mesh",
                    source,
                    legacy,
                    manifest_path,
                    export_receipt_path,
                    cook_receipt_path,
                    package,
                    converter,
                )

            manifest_path.write_bytes(canonical_manifest)

            def write_resealed_receipt_chain(
                manifest_value: dict[str, Any],
                export_value: dict[str, Any],
                cook_value: dict[str, Any],
            ) -> None:
                manifest_payload = _json_bytes(manifest_value)
                manifest_path.write_bytes(manifest_payload)
                resealed_export = dict(export_value)
                resealed_export["sourceManifestSha256"] = hashlib.sha256(
                    manifest_payload
                ).hexdigest()
                export_payload = _json_bytes(resealed_export)
                export_receipt_path.write_bytes(export_payload)
                resealed_cook = dict(cook_value)
                resealed_cook["sourceExportReceiptSha256"] = hashlib.sha256(
                    export_payload
                ).hexdigest()
                cook_receipt_path.write_bytes(_json_bytes(resealed_cook))

            receipt_documents = (
                ("source manifest", manifest),
                ("source export receipt", export_receipt),
                ("legacy cook receipt", cook_receipt),
            )
            for label, document in receipt_documents:
                for invalid_version in (True, 1.0, "1"):
                    with self.subTest(
                        strict_integer_format_version=label,
                        invalid_value=repr(invalid_version),
                    ):
                        invalid_manifest = dict(manifest)
                        invalid_export = dict(export_receipt)
                        invalid_cook = dict(cook_receipt)
                        target = {
                            "source manifest": invalid_manifest,
                            "source export receipt": invalid_export,
                            "legacy cook receipt": invalid_cook,
                        }[label]
                        target["formatVersion"] = invalid_version
                        write_resealed_receipt_chain(
                            invalid_manifest, invalid_export, invalid_cook
                        )
                        with self.assertRaisesRegex(ValueError, "schema/version"):
                            load_geometry_provenance_evidence(
                                "fixture.mesh",
                                source,
                                legacy,
                                manifest_path,
                                export_receipt_path,
                                cook_receipt_path,
                                package,
                                converter,
                            )

            invalid_export_schema = dict(export_receipt)
            invalid_export_schema["schema"] = "unsupported"
            write_resealed_receipt_chain(
                manifest, invalid_export_schema, cook_receipt
            )
            with self.subTest(invalid_receipt_schema="source export receipt"):
                with self.assertRaisesRegex(ValueError, "schema/version"):
                    load_geometry_provenance_evidence(
                        "fixture.mesh",
                        source,
                        legacy,
                        manifest_path,
                        export_receipt_path,
                        cook_receipt_path,
                        package,
                        converter,
                    )

            write_resealed_receipt_chain(manifest, export_receipt, cook_receipt)

            package.write_bytes(package.read_bytes() + b"-late-mutation")
            package_provenance, _, _ = load_geometry_provenance_evidence(
                "fixture.mesh",
                source,
                legacy,
                manifest_path,
                export_receipt_path,
                cook_receipt_path,
                package,
                converter,
            )
            package_output, package_receipt = cook_wmodel_geometry_contract(
                source, legacy, package_provenance
            )
            self.assertNotEqual(lf_output, package_output)
            self.assertEqual(
                package_receipt["provenanceEvidence"]["sourcePackage"]["status"],
                "OBSERVED_UNBOUND",
            )

            converter.write_bytes(converter.read_bytes() + b"-late-mutation")
            converter_provenance, _, _ = load_geometry_provenance_evidence(
                "fixture.mesh",
                source,
                legacy,
                manifest_path,
                export_receipt_path,
                cook_receipt_path,
                package,
                converter,
            )
            converter_output, converter_receipt = cook_wmodel_geometry_contract(
                source, legacy, converter_provenance
            )
            self.assertNotEqual(package_output, converter_output)
            self.assertEqual(
                converter_receipt["provenanceEvidence"]["legacyConverter"]["status"],
                "OBSERVED_UNBOUND",
            )

            mutated = source.read_bytes().replace(b'"2.0"', b'"2.1"', 1)
            self.assertEqual(len(mutated), source.stat().st_size)
            source.write_bytes(mutated)
            with self.assertRaisesRegex(ValueError, "glTF SHA-256"):
                load_geometry_provenance_evidence(
                    "fixture.mesh",
                    source,
                    legacy,
                    manifest_path,
                    export_receipt_path,
                    cook_receipt_path,
                    package,
                    converter,
                )


if __name__ == "__main__":
    if len(sys.argv) == 3 and sys.argv[1] == "--write-harness-suite":
        write_harness_suite(Path(sys.argv[2]))
    else:
        unittest.main()
