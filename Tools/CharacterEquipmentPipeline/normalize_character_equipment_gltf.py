#!/usr/bin/env python3
"""Normalize extracted character-equipment glTF onto a class master skeleton.

Extracted costume glTF carries its own joint palette.  ``CMesh::Bind_Resource``
consumes palette indices directly and ``CPart_Equipment::Render_Pass`` binds the
*body* model's palette, so a part whose palette is not the class body's palette
deforms against the wrong bones.  This tool rewrites one part glTF so the cooked
WModel palette is the class master palette, in master order, with genuinely
costume-specific bones appended after it.

The pipeline stays the existing one::

    normalize -> ModelAssetConverter.exe <gltf> -o <part>.wmodel
                 --scale 100 --no-auto-textures [--<slot>-remap <mat>=<file>]

``--pretransform`` is never used, and the output WModel keeps the converter's
format.  Nothing here writes outside the work root unless ``--mode install`` is
requested, and a part that fails validation keeps its previous ``.wmodel``.

Spaces
------
The class master WModel and an extracted glTF describe the same rig in two
different frames.  Measured against the shipped bodies, the master frame is the
glTF frame mapped by ``A = scale(100) . Rx(-90) . mirrorZ``; the converter itself
applies ``scale(100) . mirrorZ`` to vertices and ``mirrorZ`` conjugation to
matrices.  Composing the two gives the normalizer's rules:

* vertex/normal/tangent directions rotate by ``Rx(+90)``: ``(x, y, z) -> (x, -z, y)``
* node local matrices and inverse binds conjugate by ``K = mirrorZ . A``, whose
  linear part is ``100 . Rx(+90)``
* master bones take the master's own rest and bind pose, conjugated by ``mirrorZ``

so the cook reproduces the master palette values exactly.
"""

from __future__ import annotations

import argparse
import base64
import json
import math
import re
import shutil
import struct
import subprocess
import sys
import urllib.parse
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import Any, Iterable, Sequence


REPORT_SCHEMA = "lostark.character-equipment-skeleton-normalization"
REPORT_FORMAT_VERSION = 1
CONVERTER_RELATIVE_PATH = "Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe"
CHARACTER_CATALOG_RELATIVE_PATH = "Data/Actors/CharacterCatalog.json"
COOK_SCALE = "100"

# Maximum accepted difference, in cooked centimetres, between a shared joint's
# master bind pose and the part's own bind pose mapped into the master frame.
# Skeleton bones agree to ~0.03 cm; the slack covers accessory chains such as
# b_upper_cloth_* whose rest pose an outfit re-places while keeping the name.
DEFAULT_INVERSE_BIND_TOLERANCE = 2.0

FILE_HEADER = struct.Struct("<4sHHII")
MODEL_HEADER = struct.Struct("<4sIII4I")
SECTION_DESC = struct.Struct("<IIQQ40s")
MESH_HEADER = struct.Struct("<4sIIIIIIIB3s")
SUBMESH_DESC = struct.Struct("<IIIIIQ20s")
MESH_BONE = struct.Struct("<Q32si16fI16s")
SKELETON_HEADER = struct.Struct("<4sII5I")
SKELETON_BONE = struct.Struct("<Q64si16fII27I")
MATERIAL_META = struct.Struct("<4sI")
MATERIAL_ENTRY = {
    b"WMAT": (struct.Struct("<IQ64s520s"), 4),
    b"WMA2": (struct.Struct("<IQ64s" + "520s" * 9), 12),
    b"WMA3": (struct.Struct("<IQ64s" + "520s" * 10 + "16f"), 13),
}
SECTION_MESH = 1
SECTION_MATERIAL = 2
SECTION_SKELETON = 3
VF_BONE_WEIGHT = 1 << 4
MESH_BONE_NAME_CAPACITY = 31
# Byte offsets inside MESH_BONE_ENTRY and SKELETON_BONE_NODE.
MESH_BONE_PARENT_OFFSET = 40
SKELETON_BONE_PARENT_OFFSET = 72
SKELETON_BONE_CHILD_OFFSET = 140

COMPONENT_TYPES = {
    5120: ("b", 1),
    5121: ("B", 1),
    5122: ("h", 2),
    5123: ("H", 2),
    5125: ("I", 4),
    5126: ("f", 4),
}
COMPONENT_NORMALIZE_DIVISOR = {5120: 127.0, 5121: 255.0, 5122: 32767.0, 5123: 65535.0}
TYPE_COMPONENT_COUNT = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4, "MAT4": 16}
TRIANGLES_MODE = 4
MAX_JOINT_INDEX = 0xFFFF

# Texture parameter name -> converter remap flag, mirroring the existing cook.
MATERIAL_SLOT_FLAGS = {
    "texture_diffuse": "--material-remap",
    "texture_basecolor": "--material-remap",
    "texture_orm": "--orm-remap",
    "texture_alpha": "--opacity-remap",
    "texture_normal": "--normal-remap",
    "texture_specular": "--specular-remap",
    "texture_emissive": "--emissive-remap",
    "texture_opacity": "--opacity-remap",
}
TEXTURE_EXTENSIONS = (".dds", ".png", ".tga")
PROPS_PARAMETER = re.compile(
    r"ParameterValue = Texture2D'([^']+)'\s*\n\s*ParameterName = (\S+)"
)


class NormalizeError(ValueError):
    """A fail-closed master, source, or normalization contract violation."""


def require(condition: bool, message: str) -> None:
    if not condition:
        raise NormalizeError(message)


# ---------------------------------------------------------------------------
# small 4x4 helpers, column-major (glTF order) throughout
# ---------------------------------------------------------------------------


IDENTITY4 = (
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0,
)


def multiply4(left: Sequence[float], right: Sequence[float]) -> tuple[float, ...]:
    """Column-major ``left * right``; applying the result applies ``right`` first."""
    out = [0.0] * 16
    for column in range(4):
        for row in range(4):
            total = 0.0
            for k in range(4):
                total += left[k * 4 + row] * right[column * 4 + k]
            out[column * 4 + row] = total
    return tuple(out)


def linear4(rows: Sequence[Sequence[float]]) -> tuple[float, ...]:
    """Build a column-major 4x4 from three rows of a 3x3 linear map."""
    out = [0.0] * 16
    for row in range(3):
        for column in range(3):
            out[column * 4 + row] = float(rows[row][column])
    out[15] = 1.0
    return tuple(out)


MIRROR_Z = linear4(((1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, -1.0)))
# K = mirrorZ . scale(100) . Rx(-90) . mirrorZ, linear part 100 * Rx(+90).
CONJUGATION = linear4(((100.0, 0.0, 0.0), (0.0, 0.0, -100.0), (0.0, 100.0, 0.0)))
CONJUGATION_INVERSE = linear4(
    ((0.01, 0.0, 0.0), (0.0, 0.0, 0.01), (0.0, -0.01, 0.0))
)
# Master frame mapped straight from the glTF frame, used only for validation.
MASTER_FROM_GLTF = linear4(((100.0, 0.0, 0.0), (0.0, 0.0, -100.0), (0.0, -100.0, 0.0)))
MASTER_FROM_GLTF_INVERSE = linear4(
    ((0.01, 0.0, 0.0), (0.0, 0.0, -0.01), (0.0, -0.01, 0.0))
)


def conjugate(matrix: Sequence[float], basis: Sequence[float], basis_inverse: Sequence[float]) -> tuple[float, ...]:
    return multiply4(multiply4(basis, matrix), basis_inverse)


def rotate_direction(vector: Sequence[float]) -> tuple[float, float, float]:
    """Apply ``Rx(+90)`` to a glTF-space direction or position."""
    return (float(vector[0]), -float(vector[2]), float(vector[1]))


def compose_trs(node: dict[str, Any], label: str) -> tuple[float, ...]:
    if "matrix" in node:
        matrix = node["matrix"]
        require(
            isinstance(matrix, list) and len(matrix) == 16,
            f"{label} matrix must hold 16 numbers",
        )
        return tuple(float(value) for value in matrix)
    translation = node.get("translation", (0.0, 0.0, 0.0))
    rotation = node.get("rotation", (0.0, 0.0, 0.0, 1.0))
    scale = node.get("scale", (1.0, 1.0, 1.0))
    require(len(translation) == 3 and len(rotation) == 4 and len(scale) == 3, f"{label} TRS is malformed")
    x, y, z, w = (float(value) for value in rotation)
    rows = (
        (1.0 - 2.0 * (y * y + z * z), 2.0 * (x * y - z * w), 2.0 * (x * z + y * w)),
        (2.0 * (x * y + z * w), 1.0 - 2.0 * (x * x + z * z), 2.0 * (y * z - x * w)),
        (2.0 * (x * z - y * w), 2.0 * (y * z + x * w), 1.0 - 2.0 * (x * x + y * y)),
    )
    out = [0.0] * 16
    for column in range(3):
        for row in range(3):
            out[column * 4 + row] = rows[row][column] * float(scale[column])
    out[12], out[13], out[14] = (float(value) for value in translation)
    out[15] = 1.0
    return tuple(out)


# ---------------------------------------------------------------------------
# class master skeleton, read from the shipped class body WModel
# ---------------------------------------------------------------------------


@dataclass(frozen=True)
class MasterBone:
    name: str
    parent_index: int
    rest: tuple[float, ...]          # column-major local transform
    inverse_bind: tuple[float, ...]  # column-major inverse bind matrix


@dataclass(frozen=True)
class MasterSkeleton:
    class_id: str
    source_path: Path
    bones: tuple[MasterBone, ...]
    mesh_slot_index: int

    @property
    def names(self) -> tuple[str, ...]:
        return tuple(bone.name for bone in self.bones)

    def index_of(self, name: str) -> int | None:
        return self._index.get(name)

    def __post_init__(self) -> None:
        object.__setattr__(self, "_index", {bone.name: index for index, bone in enumerate(self.bones)})


def _fixed_text(raw: bytes) -> str:
    return raw.split(b"\0", 1)[0].decode("utf-8", "replace")


def _wide_text(raw: bytes) -> str:
    text = raw.decode("utf-16-le", "replace")
    terminator = text.find("\0")
    return text if terminator < 0 else text[:terminator]


def read_wmodel_sections(path: Path) -> list[tuple[int, bytes]]:
    data = path.read_bytes()
    require(len(data) >= FILE_HEADER.size + MODEL_HEADER.size, f"WModel is truncated: {path}")
    magic, major, _minor, flags, content_size = FILE_HEADER.unpack_from(data, 0)
    require(
        magic == b"WINT" and major == 1 and flags == 0 and content_size == len(data) - FILE_HEADER.size,
        f"WModel outer header is invalid: {path}",
    )
    content = data[FILE_HEADER.size :]
    model_magic, section_count, _animation_count, model_flags, *reserved = MODEL_HEADER.unpack_from(content, 0)
    require(
        model_magic == b"WMOD" and section_count > 0 and model_flags in (0, 1) and all(v == 0 for v in reserved),
        f"WModel metadata is invalid: {path}",
    )
    descriptors = [
        SECTION_DESC.unpack_from(content, MODEL_HEADER.size + index * SECTION_DESC.size)
        for index in range(section_count)
    ]
    first_offset = int(descriptors[0][2])
    if content[first_offset : first_offset + 4] == b"WINT":
        base = 0
    elif content[first_offset + MODEL_HEADER.size : first_offset + MODEL_HEADER.size + 4] == b"WINT":
        base = MODEL_HEADER.size
    else:
        raise NormalizeError(f"cannot determine WModel section offset base: {path}")
    sections: list[tuple[int, bytes]] = []
    for row, (type_id, _index, offset, size, _name) in enumerate(descriptors):
        start = base + int(offset)
        end = start + int(size)
        require(int(size) > 0 and end <= len(content), f"WModel section {row} is out of range: {path}")
        sections.append((int(type_id), content[start:end]))
    return sections


def _nested_payload(blob: bytes, expected_magic: bytes, label: str) -> bytes:
    require(len(blob) >= FILE_HEADER.size + 4, f"{label} is truncated")
    magic, major, _minor, flags, content_size = FILE_HEADER.unpack_from(blob, 0)
    require(
        magic == b"WINT" and major == 1 and flags == 0 and content_size == len(blob) - FILE_HEADER.size,
        f"{label} nested header is invalid",
    )
    payload = blob[FILE_HEADER.size :]
    require(payload[:4] == expected_magic, f"{label} magic is invalid")
    return payload


@dataclass(frozen=True)
class MeshPalette:
    names: tuple[str, ...]
    parents: tuple[int, ...]
    inverse_binds: tuple[tuple[float, ...], ...]
    vertex_count: int
    bone_count: int
    max_joint_index: int
    weighted_bones: frozenset[int]


def read_mesh_palette(blob: bytes, label: str) -> MeshPalette:
    payload = _nested_payload(blob, b"WMSH", label)
    (
        _magic,
        submesh_count,
        bone_count,
        vertex_flags,
        vertex_stride,
        vertex_count,
        index_count,
        index_stride,
        has_bounding,
        _reserved,
    ) = MESH_HEADER.unpack_from(payload, 0)
    require(index_stride in (2, 4) and has_bounding in (0, 1), f"{label} mesh header is invalid")
    table_end = MESH_HEADER.size + submesh_count * SUBMESH_DESC.size
    bone_offset = table_end + vertex_count * vertex_stride + index_count * index_stride
    require(
        bone_offset + bone_count * MESH_BONE.size <= len(payload),
        f"{label} mesh bone table is out of range",
    )
    names: list[str] = []
    parents: list[int] = []
    inverse_binds: list[tuple[float, ...]] = []
    for bone_index in range(bone_count):
        row = MESH_BONE.unpack_from(payload, bone_offset + bone_index * MESH_BONE.size)
        matrix = tuple(float(value) for value in row[3:19])
        require(all(math.isfinite(value) for value in matrix), f"{label} bone {bone_index} bind is non-finite")
        names.append(_fixed_text(row[1]))
        parents.append(int(row[2]))
        inverse_binds.append(matrix)
    max_joint_index = -1
    weighted_bones: set[int] = set()
    if vertex_flags & VF_BONE_WEIGHT:
        for vertex_index in range(vertex_count):
            row = table_end + vertex_index * vertex_stride
            joints = struct.unpack_from("<4I", payload, row + 44)
            weights = struct.unpack_from("<4f", payload, row + 60)
            for joint, weight in zip(joints, weights):
                max_joint_index = max(max_joint_index, int(joint))
                if math.isfinite(weight) and weight > 1e-6:
                    weighted_bones.add(int(joint))
    return MeshPalette(
        tuple(names),
        tuple(parents),
        tuple(inverse_binds),
        int(vertex_count),
        int(bone_count),
        max_joint_index,
        frozenset(weighted_bones),
    )


def read_skeleton_bones(blob: bytes, label: str) -> tuple[tuple[str, int, tuple[float, ...]], ...]:
    payload = _nested_payload(blob, b"WSKL", label)
    require(len(payload) >= SKELETON_HEADER.size, f"{label} skeleton header is truncated")
    bone_count = int(SKELETON_HEADER.unpack_from(payload, 0)[1])
    offset = SKELETON_HEADER.size
    require(
        offset + bone_count * SKELETON_BONE.size <= len(payload),
        f"{label} skeleton bones are truncated",
    )
    bones = []
    for bone_index in range(bone_count):
        row = SKELETON_BONE.unpack_from(payload, offset + bone_index * SKELETON_BONE.size)
        matrix = tuple(float(value) for value in row[3:19])
        require(all(math.isfinite(value) for value in matrix), f"{label} bone {bone_index} rest is non-finite")
        bones.append((_fixed_text(row[1]), int(row[2]), matrix))
    return tuple(bones)


def read_material_texture_paths(blob: bytes, label: str) -> tuple[str, ...]:
    require(len(blob) >= FILE_HEADER.size + MATERIAL_META.size, f"{label} material section is truncated")
    payload = blob[FILE_HEADER.size :]
    magic, material_count = MATERIAL_META.unpack_from(payload, 0)
    require(magic in MATERIAL_ENTRY, f"{label} has unsupported material magic {magic!r}")
    entry_struct, path_field_end = MATERIAL_ENTRY[magic]
    offset = MATERIAL_META.size
    require(
        offset + material_count * entry_struct.size <= len(payload),
        f"{label} material entries are truncated",
    )
    paths: set[str] = set()
    for material_index in range(material_count):
        fields = entry_struct.unpack_from(payload, offset + material_index * entry_struct.size)
        for raw_path in fields[3:path_field_end]:
            text = _wide_text(raw_path)
            if text:
                paths.add(text.replace("\\", "/"))
    return tuple(sorted(paths, key=str.casefold))


def load_master_skeleton(class_id: str, body_path: Path) -> MasterSkeleton:
    """Read the class master palette from the class body's own cooked WModel."""
    sections = read_wmodel_sections(body_path)
    mesh_sections = [blob for type_id, blob in sections if type_id == SECTION_MESH]
    skeleton_sections = [blob for type_id, blob in sections if type_id == SECTION_SKELETON]
    require(len(mesh_sections) >= 1, f"class body has no mesh section: {body_path}")
    require(len(skeleton_sections) == 1, f"class body needs exactly one skeleton section: {body_path}")
    palette = read_mesh_palette(mesh_sections[0], f"{class_id} body mesh")
    skeleton = read_skeleton_bones(skeleton_sections[0], f"{class_id} body skeleton")
    require(
        len(skeleton) == palette.bone_count
        and all(bone[0] == name for bone, name in zip(skeleton, palette.names)),
        f"class body mesh palette and skeleton disagree: {body_path}",
    )
    # The mesh bone record's own parent field is unused by the cook, so the
    # skeleton section owns the hierarchy.
    parents = tuple(bone[1] for bone in skeleton)
    names = palette.names
    require(len(set(names)) == len(names), f"class body palette has duplicate bone names: {body_path}")
    require(
        all(len(name) <= MESH_BONE_NAME_CAPACITY for name in names),
        f"class body palette has a bone name the mesh record cannot hold: {body_path}",
    )
    roots = [index for index, parent in enumerate(parents) if parent < 0]
    require(roots == [0], f"class body palette must have exactly one root at index 0: {body_path}")
    require(
        all(parent < index for index, parent in enumerate(parents) if parent >= 0),
        f"class body palette is not stored parent-before-child: {body_path}",
    )
    children: dict[int, list[int]] = {}
    for index, parent in enumerate(parents):
        if parent >= 0:
            children.setdefault(parent, []).append(index)
    leaf_root_children = [index for index in children.get(0, []) if index not in children]
    require(
        len(leaf_root_children) == 1,
        f"class body palette needs exactly one leaf mesh slot below the root: {body_path}",
    )
    bones = tuple(
        MasterBone(name, parent, rest, inverse_bind)
        for name, parent, (_n, _p, rest), inverse_bind in zip(
            names, parents, skeleton, palette.inverse_binds
        )
    )
    return MasterSkeleton(class_id, body_path, bones, leaf_root_children[0])


# ---------------------------------------------------------------------------
# glTF source model
# ---------------------------------------------------------------------------


@dataclass(frozen=True)
class SourcePrimitive:
    material: int | None
    indices: tuple[int, ...]
    positions: tuple[tuple[float, ...], ...]
    joints: tuple[tuple[int, ...], ...]
    weights: tuple[tuple[float, ...], ...]
    directions: dict[str, tuple[tuple[float, ...], ...]]   # NORMAL, TANGENT
    passthrough: dict[str, tuple[tuple[float, ...], ...]]  # TEXCOORD_n, COLOR_0


@dataclass(frozen=True)
class SourcePart:
    path: Path
    node_names: tuple[str, ...]
    node_parents: tuple[int, ...]
    node_locals: tuple[tuple[float, ...], ...]
    mesh_node: int
    joints: tuple[int, ...]
    inverse_binds: tuple[tuple[float, ...], ...]
    primitives: tuple[SourcePrimitive, ...]
    materials: tuple[dict[str, Any], ...]


def _read_accessor(document: dict[str, Any], buffer: bytes, accessor_index: int, label: str) -> tuple[tuple[float | int, ...], ...]:
    accessors = document.get("accessors") or []
    require(0 <= accessor_index < len(accessors), f"{label} accessor {accessor_index} is out of range")
    accessor = accessors[accessor_index]
    require("sparse" not in accessor, f"{label} sparse accessors are not supported")
    component_type = int(accessor["componentType"])
    require(component_type in COMPONENT_TYPES, f"{label} component type {component_type} is unsupported")
    element_type = str(accessor["type"])
    require(element_type in TYPE_COMPONENT_COUNT, f"{label} element type {element_type} is unsupported")
    format_code, component_size = COMPONENT_TYPES[component_type]
    component_count = TYPE_COMPONENT_COUNT[element_type]
    count = int(accessor["count"])
    view_index = accessor.get("bufferView")
    require(view_index is not None, f"{label} accessor has no bufferView")
    view = (document.get("bufferViews") or [])[int(view_index)]
    require(int(view.get("buffer", 0)) == 0, f"{label} uses a second buffer")
    stride = int(view.get("byteStride") or component_size * component_count)
    start = int(view.get("byteOffset", 0)) + int(accessor.get("byteOffset", 0))
    end = start + (count - 1) * stride + component_size * component_count if count else start
    require(end <= len(buffer), f"{label} accessor reads past the buffer")
    unpack = struct.Struct("<" + format_code * component_count)
    rows = [unpack.unpack_from(buffer, start + row * stride) for row in range(count)]
    if accessor.get("normalized"):
        divisor = COMPONENT_NORMALIZE_DIVISOR.get(component_type)
        require(divisor is not None, f"{label} normalized accessor uses a float component type")
        rows = [tuple(max(value / divisor, -1.0) for value in row) for row in rows]
    return tuple(rows)


def load_source_part(path: Path) -> SourcePart:
    require(path.is_file(), f"source glTF is missing: {path}")
    document = json.loads(path.read_text(encoding="utf-8"))
    buffers = document.get("buffers") or []
    require(len(buffers) == 1, f"{path} must declare exactly one buffer")
    uri = buffers[0].get("uri")
    require(isinstance(uri, str) and uri, f"{path} buffer has no uri")
    if uri.startswith("data:"):
        _header, _, encoded = uri.partition(",")
        buffer = base64.b64decode(encoded)
    else:
        relative = urllib.parse.unquote(uri)
        require(
            not PurePosixPath(relative).is_absolute() and ".." not in PurePosixPath(relative).parts,
            f"{path} buffer uri escapes its folder: {uri}",
        )
        buffer_path = path.parent / Path(*PurePosixPath(relative).parts)
        require(buffer_path.is_file(), f"{path} buffer file is missing: {buffer_path}")
        buffer = buffer_path.read_bytes()

    nodes = document.get("nodes") or []
    require(nodes, f"{path} has no nodes")
    names = []
    for index, node in enumerate(nodes):
        name = node.get("name")
        require(isinstance(name, str) and name, f"{path} node {index} has no name")
        names.append(name)
    parents = [-1] * len(nodes)
    for index, node in enumerate(nodes):
        for child in node.get("children", []):
            require(0 <= int(child) < len(nodes), f"{path} node {index} has an out-of-range child")
            require(parents[int(child)] == -1, f"{path} node {child} has two parents")
            parents[int(child)] = index
    locals_ = tuple(compose_trs(node, f"{path} node {index}") for index, node in enumerate(nodes))

    mesh_nodes = [index for index, node in enumerate(nodes) if "mesh" in node]
    require(len(mesh_nodes) == 1, f"{path} must carry exactly one mesh node")
    mesh_node = mesh_nodes[0]
    skin_index = nodes[mesh_node].get("skin")
    require(skin_index is not None, f"{path} mesh node is not skinned")
    skins = document.get("skins") or []
    require(len(skins) == 1 and int(skin_index) == 0, f"{path} must declare exactly one skin")
    skin = skins[0]
    joints = tuple(int(value) for value in skin.get("joints", ()))
    require(joints, f"{path} skin has no joints")
    require(
        all(0 <= joint < len(nodes) for joint in joints),
        f"{path} skin references an out-of-range joint node",
    )
    require(len(set(joints)) == len(joints), f"{path} skin lists a joint node twice")
    require(
        "inverseBindMatrices" in skin,
        f"{path} skin has no inverseBindMatrices",
    )
    raw_binds = _read_accessor(document, buffer, int(skin["inverseBindMatrices"]), f"{path} inverseBindMatrices")
    require(len(raw_binds) == len(joints), f"{path} inverseBindMatrices count differs from the joint count")
    inverse_binds = tuple(tuple(float(value) for value in row) for row in raw_binds)

    mesh = (document.get("meshes") or [])[int(nodes[mesh_node]["mesh"])]
    primitives: list[SourcePrimitive] = []
    for order, primitive in enumerate(mesh.get("primitives") or ()):
        label = f"{path} primitive {order}"
        require(int(primitive.get("mode", TRIANGLES_MODE)) == TRIANGLES_MODE, f"{label} is not a triangle list")
        attributes = primitive.get("attributes") or {}
        for required in ("POSITION", "JOINTS_0", "WEIGHTS_0"):
            require(required in attributes, f"{label} has no {required}")
        positions = tuple(
            tuple(float(value) for value in row)
            for row in _read_accessor(document, buffer, int(attributes["POSITION"]), f"{label} POSITION")
        )
        joint_rows = tuple(
            tuple(int(value) for value in row)
            for row in _read_accessor(document, buffer, int(attributes["JOINTS_0"]), f"{label} JOINTS_0")
        )
        weight_rows = tuple(
            tuple(float(value) for value in row)
            for row in _read_accessor(document, buffer, int(attributes["WEIGHTS_0"]), f"{label} WEIGHTS_0")
        )
        require(
            len(joint_rows) == len(positions) and len(weight_rows) == len(positions),
            f"{label} attribute counts differ",
        )
        directions: dict[str, tuple[tuple[float, ...], ...]] = {}
        passthrough: dict[str, tuple[tuple[float, ...], ...]] = {}
        for name, accessor_index in attributes.items():
            if name in ("POSITION", "JOINTS_0", "WEIGHTS_0"):
                continue
            rows = tuple(
                tuple(float(value) for value in row)
                for row in _read_accessor(document, buffer, int(accessor_index), f"{label} {name}")
            )
            require(len(rows) == len(positions), f"{label} {name} count differs from POSITION")
            if name in ("NORMAL", "TANGENT"):
                directions[name] = rows
            elif name.startswith("TEXCOORD_") or name == "COLOR_0":
                passthrough[name] = rows
            else:
                raise NormalizeError(f"{label} carries unsupported attribute {name}")
        require("indices" in primitive, f"{label} has no index accessor")
        indices = tuple(
            int(row[0])
            for row in _read_accessor(document, buffer, int(primitive["indices"]), f"{label} indices")
        )
        require(
            all(index < len(positions) for index in indices),
            f"{label} has an out-of-range vertex index",
        )
        material = primitive.get("material")
        primitives.append(
            SourcePrimitive(
                None if material is None else int(material),
                indices,
                positions,
                joint_rows,
                weight_rows,
                directions,
                passthrough,
            )
        )
    require(primitives, f"{path} mesh has no primitives")
    materials = tuple(dict(entry) for entry in (document.get("materials") or ()))
    return SourcePart(
        path,
        tuple(names),
        tuple(parents),
        locals_,
        mesh_node,
        joints,
        inverse_binds,
        tuple(primitives),
        materials,
    )


# ---------------------------------------------------------------------------
# normalization
# ---------------------------------------------------------------------------


@dataclass(frozen=True)
class NormalizedPart:
    node_names: tuple[str, ...]
    node_parents: tuple[int, ...]
    node_locals: tuple[tuple[float, ...], ...]
    mesh_slot_index: int
    joint_nodes: tuple[int, ...]
    inverse_binds: tuple[tuple[float, ...], ...]
    primitives: tuple[SourcePrimitive, ...]
    materials: tuple[dict[str, Any], ...]
    appended_bones: tuple[str, ...]
    unused_master_bones: int
    max_inverse_bind_delta: float
    compared_joint_count: int


def normalize_part(
    master: MasterSkeleton,
    source: SourcePart,
    inverse_bind_tolerance: float = DEFAULT_INVERSE_BIND_TOLERANCE,
) -> NormalizedPart:
    """Rebind one extracted part onto the class master palette.

    Master bones are emitted in master order and take the master's rest and bind
    pose.  Genuinely costume-specific joints are appended after them, parented by
    name.  Any joint that is neither a master bone nor reachable from one fails
    the part instead of being dropped.
    """
    joint_names = [source.node_names[node] for node in source.joints]
    duplicates = sorted({name for name in joint_names if joint_names.count(name) > 1})
    require(not duplicates, f"{source.path} lists duplicate joint names: {', '.join(duplicates)}")
    require(
        source.mesh_node not in source.joints,
        f"{source.path} uses its mesh node as a skin joint",
    )

    appended: list[int] = []  # source node indices, parent-before-child
    appended_set: set[int] = set()

    def append_costume_node(node: int, trail: tuple[str, ...]) -> None:
        if node in appended_set:
            return
        name = source.node_names[node]
        require(
            name not in trail,
            f"{source.path} costume bone {name} is part of a parent cycle",
        )
        parent = source.node_parents[node]
        require(
            parent >= 0,
            f"{source.path} costume bone {name} has no parent to attach it to the master",
        )
        if master.index_of(source.node_names[parent]) is None:
            require(
                parent in source.joints or parent in appended_set,
                f"{source.path} costume bone {name} hangs off non-joint node "
                f"{source.node_names[parent]} that is not in the master",
            )
            append_costume_node(parent, trail + (name,))
        appended_set.add(node)
        appended.append(node)

    for node in source.joints:
        if master.index_of(source.node_names[node]) is None:
            append_costume_node(node, ())

    source_bind_by_node = {node: source.inverse_binds[order] for order, node in enumerate(source.joints)}

    # Bind-pose agreement: for every shared joint this part actually weights,
    # the part's own bind pose, mapped into the master frame, must land on the
    # master's bind pose.  Joints the part does not weight cannot move a vertex,
    # and the master keeps bind poses for bones its own mesh never weights.
    weighted_slots: set[int] = set()
    for primitive in source.primitives:
        for joint_row, weight_row in zip(primitive.joints, primitive.weights):
            for joint_slot, weight in zip(joint_row, weight_row):
                if weight > 0.0:
                    require(
                        0 <= joint_slot < len(source.joints),
                        f"{source.path} weights joint slot {joint_slot} outside the skin",
                    )
                    weighted_slots.add(int(joint_slot))
    weighted_nodes = {source.joints[slot] for slot in weighted_slots}

    max_delta = 0.0
    compared = 0
    worst_name = ""
    for node in source.joints:
        master_index = master.index_of(source.node_names[node])
        if master_index is None or node not in weighted_nodes:
            continue
        master_bind = master.bones[master_index].inverse_bind
        if master_bind == IDENTITY4:
            continue
        mapped = conjugate(source_bind_by_node[node], MASTER_FROM_GLTF, MASTER_FROM_GLTF_INVERSE)
        delta = max(abs(left - right) for left, right in zip(mapped, master_bind))
        compared += 1
        if delta > max_delta:
            max_delta = delta
            worst_name = source.node_names[node]
    require(
        max_delta <= inverse_bind_tolerance,
        f"{source.path} bind pose disagrees with the master at {worst_name}: "
        f"{max_delta:.4f} > {inverse_bind_tolerance:.4f}",
    )

    master_count = len(master.bones)
    mesh_slot_name = source.node_names[source.mesh_node]
    require(
        master.index_of(mesh_slot_name) is None,
        f"{source.path} mesh node name {mesh_slot_name} collides with a master bone",
    )
    node_names = [bone.name for bone in master.bones]
    node_names[master.mesh_slot_index] = mesh_slot_name
    node_parents = [bone.parent_index for bone in master.bones]
    node_locals = [
        conjugate(bone.rest, MIRROR_Z, MIRROR_Z) for bone in master.bones
    ]

    appended_names: list[str] = []
    appended_index_by_node: dict[int, int] = {}
    for node in appended:
        name = source.node_names[node]
        require(
            len(name) <= MESH_BONE_NAME_CAPACITY,
            f"{source.path} costume bone name is too long for the WModel record: {name}",
        )
        require(
            name not in node_names,
            f"{source.path} costume bone {name} collides with an emitted bone name",
        )
        parent_node = source.node_parents[node]
        parent_name = source.node_names[parent_node]
        parent_index = master.index_of(parent_name)
        if parent_index is None:
            parent_index = appended_index_by_node.get(parent_node)
        require(
            parent_index is not None,
            f"{source.path} costume bone {name} cannot resolve parent {parent_name}",
        )
        appended_index_by_node[node] = len(node_names)
        node_names.append(name)
        node_parents.append(parent_index)
        node_locals.append(conjugate(source.node_locals[node], CONJUGATION, CONJUGATION_INVERSE))
        appended_names.append(name)

    new_index_by_source_node: dict[int, int] = {}
    for node in source.joints:
        master_index = master.index_of(source.node_names[node])
        new_index_by_source_node[node] = (
            master_index if master_index is not None else appended_index_by_node[node]
        )
    new_index_by_source_node[source.mesh_node] = master.mesh_slot_index

    joint_nodes = tuple(index for index in range(len(node_names)) if index != master.mesh_slot_index)
    joint_slot_by_node = {node: slot for slot, node in enumerate(joint_nodes)}
    inverse_binds = []
    for node in joint_nodes:
        if node < master_count:
            inverse_binds.append(conjugate(master.bones[node].inverse_bind, MIRROR_Z, MIRROR_Z))
        else:
            source_node = next(key for key, value in appended_index_by_node.items() if value == node)
            inverse_binds.append(
                conjugate(source_bind_by_node[source_node], CONJUGATION, CONJUGATION_INVERSE)
            )

    used_master: set[int] = set()
    primitives: list[SourcePrimitive] = []
    for order, primitive in enumerate(source.primitives):
        label = f"{source.path} primitive {order}"
        remapped_joints: list[tuple[int, ...]] = []
        for row_index, (joint_row, weight_row) in enumerate(zip(primitive.joints, primitive.weights)):
            require(
                len(joint_row) == 4 and len(weight_row) == 4,
                f"{label} vertex {row_index} does not carry four influences",
            )
            mapped = []
            for joint_slot, weight in zip(joint_row, weight_row):
                require(
                    math.isfinite(weight) and weight >= 0.0,
                    f"{label} vertex {row_index} has an invalid blend weight",
                )
                require(
                    0 <= joint_slot < len(source.joints),
                    f"{label} vertex {row_index} references joint slot {joint_slot} outside the skin",
                )
                node = source.joints[joint_slot]
                new_node = new_index_by_source_node[node]
                if weight > 0.0 and new_node < master_count:
                    used_master.add(new_node)
                new_slot = joint_slot_by_node[new_node]
                require(new_slot <= MAX_JOINT_INDEX, f"{label} joint index exceeds the glTF ushort range")
                mapped.append(new_slot)
            remapped_joints.append(tuple(mapped))
        rotated_positions = tuple(rotate_direction(row) for row in primitive.positions)
        rotated_directions: dict[str, tuple[tuple[float, ...], ...]] = {}
        for name, rows in primitive.directions.items():
            if name == "TANGENT":
                rotated_directions[name] = tuple(
                    rotate_direction(row) + (float(row[3]),) if len(row) == 4 else rotate_direction(row)
                    for row in rows
                )
            else:
                rotated_directions[name] = tuple(rotate_direction(row) for row in rows)
        primitives.append(
            SourcePrimitive(
                primitive.material,
                primitive.indices,
                rotated_positions,
                tuple(remapped_joints),
                primitive.weights,
                rotated_directions,
                primitive.passthrough,
            )
        )

    return NormalizedPart(
        tuple(node_names),
        tuple(node_parents),
        tuple(node_locals),
        master.mesh_slot_index,
        joint_nodes,
        tuple(inverse_binds),
        tuple(primitives),
        source.materials,
        tuple(appended_names),
        master_count - len(used_master),
        max_delta,
        compared,
    )


# ---------------------------------------------------------------------------
# normalized glTF writer
# ---------------------------------------------------------------------------


class _BufferWriter:
    def __init__(self) -> None:
        self.blob = bytearray()
        self.views: list[dict[str, Any]] = []
        self.accessors: list[dict[str, Any]] = []

    def _align(self, alignment: int) -> None:
        padding = (-len(self.blob)) % alignment
        if padding:
            self.blob.extend(b"\0" * padding)

    def add(
        self,
        rows: Sequence[Sequence[float | int]],
        component_type: int,
        element_type: str,
        target: int | None = None,
        with_bounds: bool = False,
    ) -> int:
        format_code, component_size = COMPONENT_TYPES[component_type]
        component_count = TYPE_COMPONENT_COUNT[element_type]
        self._align(component_size)
        offset = len(self.blob)
        pack = struct.Struct("<" + format_code * component_count)
        for row in rows:
            self.blob.extend(pack.pack(*row))
        view = {"buffer": 0, "byteOffset": offset, "byteLength": len(self.blob) - offset}
        if target is not None:
            view["target"] = target
        self.views.append(view)
        accessor: dict[str, Any] = {
            "bufferView": len(self.views) - 1,
            "componentType": component_type,
            "count": len(rows),
            "type": element_type,
        }
        if with_bounds and rows:
            accessor["min"] = [min(row[i] for row in rows) for i in range(component_count)]
            accessor["max"] = [max(row[i] for row in rows) for i in range(component_count)]
        self.accessors.append(accessor)
        return len(self.accessors) - 1


def _strip_material(material: dict[str, Any]) -> dict[str, Any]:
    """Keep material identity, drop texture bindings the cook supplies by flag."""
    stripped: dict[str, Any] = {}
    for key, value in material.items():
        if key in ("normalTexture", "occlusionTexture", "emissiveTexture"):
            continue
        if key == "pbrMetallicRoughness" and isinstance(value, dict):
            stripped[key] = {
                inner_key: inner_value
                for inner_key, inner_value in value.items()
                if not inner_key.endswith("Texture")
            }
            continue
        stripped[key] = value
    return stripped


def write_normalized_gltf(part: NormalizedPart, gltf_path: Path) -> Path:
    gltf_path.parent.mkdir(parents=True, exist_ok=True)
    binary_path = gltf_path.with_suffix(".bin")
    writer = _BufferWriter()

    primitives_json: list[dict[str, Any]] = []
    for primitive in part.primitives:
        attributes: dict[str, int] = {
            "POSITION": writer.add(primitive.positions, 5126, "VEC3", 34962, with_bounds=True)
        }
        for name, rows in primitive.directions.items():
            element = "VEC4" if name == "TANGENT" and rows and len(rows[0]) == 4 else "VEC3"
            attributes[name] = writer.add(rows, 5126, element, 34962)
        for name, rows in primitive.passthrough.items():
            width = len(rows[0]) if rows else 2
            element = {2: "VEC2", 3: "VEC3", 4: "VEC4"}[width]
            attributes[name] = writer.add(rows, 5126, element, 34962)
        attributes["JOINTS_0"] = writer.add(primitive.joints, 5123, "VEC4", 34962)
        attributes["WEIGHTS_0"] = writer.add(primitive.weights, 5126, "VEC4", 34962)
        indices = writer.add([(index,) for index in primitive.indices], 5125, "SCALAR", 34963)
        entry: dict[str, Any] = {"attributes": attributes, "indices": indices, "mode": TRIANGLES_MODE}
        if primitive.material is not None:
            entry["material"] = primitive.material
        primitives_json.append(entry)

    inverse_bind_accessor = writer.add(part.inverse_binds, 5126, "MAT4")

    children: dict[int, list[int]] = {}
    for index, parent in enumerate(part.node_parents):
        if parent >= 0:
            children.setdefault(parent, []).append(index)
    nodes: list[dict[str, Any]] = []
    for index, name in enumerate(part.node_names):
        node: dict[str, Any] = {"name": name}
        matrix = part.node_locals[index]
        if matrix != IDENTITY4:
            node["matrix"] = [float(value) for value in matrix]
        if index in children:
            node["children"] = children[index]
        if index == part.mesh_slot_index:
            node["mesh"] = 0
            node["skin"] = 0
        nodes.append(node)

    document: dict[str, Any] = {
        "asset": {"version": "2.0", "generator": "normalize_character_equipment_gltf"},
        "scene": 0,
        "scenes": [{"nodes": [0]}],
        "nodes": nodes,
        "meshes": [{"primitives": primitives_json}],
        "skins": [
            {
                "inverseBindMatrices": inverse_bind_accessor,
                "joints": list(part.joint_nodes),
            }
        ],
        "buffers": [{"uri": binary_path.name, "byteLength": len(writer.blob)}],
        "bufferViews": writer.views,
        "accessors": writer.accessors,
    }
    if part.materials:
        document["materials"] = [_strip_material(material) for material in part.materials]
    binary_path.write_bytes(bytes(writer.blob))
    gltf_path.write_text(json.dumps(document, indent=1), encoding="utf-8")
    return gltf_path


# ---------------------------------------------------------------------------
# cook plan
# ---------------------------------------------------------------------------


@dataclass(frozen=True)
class PartPlan:
    class_id: str
    set_id: str
    part_id: str
    source_gltf: Path
    material_root: Path
    listed_material: str | None
    target_asset_id: str


def _normalized_material_key(name: str) -> str:
    lowered = name.lower()
    lowered = re.sub(r"_mi(_loc_int)?$", "", lowered)
    lowered = re.sub(r"-\d+", "", lowered)
    return re.sub(r"[_\-.]", "", lowered)


def _find_material_props(material_root: Path, material_name: str) -> Path | None:
    matches = sorted(material_root.glob(f"*/mat/{material_name.lower()}.props.txt"))
    return matches[0] if matches else None


def _find_texture(material_root: Path, reference: str) -> Path | None:
    name = reference.split(".")[-1].lower()
    for extension in TEXTURE_EXTENSIONS:
        matches = sorted(material_root.glob(f"*/tex/{name}{extension}"))
        if matches:
            return matches[0]
    return None


def _texture_parameters(props_path: Path) -> dict[str, str]:
    text = props_path.read_text(encoding="utf-8", errors="replace")
    return {match.group(2).lower(): match.group(1) for match in PROPS_PARAMETER.finditer(text)}


def build_material_remap_arguments(plan: PartPlan, material_names: Sequence[str]) -> tuple[list[str], list[str]]:
    """Reproduce the existing cook's material remap resolution, unchanged."""
    listed = plan.listed_material.lower() if plan.listed_material else None
    listed_props = _find_material_props(plan.material_root, listed) if listed else None
    target = None
    if listed:
        for name in material_names:
            if _normalized_material_key(name) == _normalized_material_key(listed):
                target = name
                break
    arguments: list[str] = []
    missing: list[str] = []
    for name in material_names:
        props = listed_props if (target == name and listed_props) else _find_material_props(plan.material_root, name)
        if props is None:
            continue
        parameters = _texture_parameters(props)
        for slot, flag in MATERIAL_SLOT_FLAGS.items():
            reference = parameters.get(slot)
            if not reference:
                continue
            texture = _find_texture(plan.material_root, reference)
            if texture is None:
                missing.append(f"{name}:{slot}:{reference}")
                continue
            arguments += [flag, f"{name}={texture}"]
    return arguments, missing


def build_part_plans(outfits_path: Path, source_root: Path, class_filter: Sequence[str] | None) -> list[PartPlan]:
    outfits = json.loads(outfits_path.read_text(encoding="utf-8"))
    plans: list[PartPlan] = []
    for outfit in outfits:
        class_id = str(outfit["className"])
        if class_filter and class_id not in class_filter:
            continue
        set_id = str(outfit["setName"])
        material_root = source_root / class_id / set_id
        for part in outfit.get("parts", ()):
            part_id = str(part["part"])
            package = str(part["meshPackage"]).upper()
            mesh_object = str(part["meshObject"]).lower()
            plans.append(
                PartPlan(
                    class_id,
                    set_id,
                    part_id,
                    material_root / package / "mesh" / f"{mesh_object}.gltf",
                    material_root,
                    str(part["matObject"]) if part.get("matObject") else None,
                    f"Character/{class_id}/Equipment/{set_id}/{part_id}.wmodel",
                )
            )
    return plans


# ---------------------------------------------------------------------------
# cook and verification
# ---------------------------------------------------------------------------


def cook_normalized_gltf(converter: Path, gltf_path: Path, output_path: Path, material_arguments: Sequence[str]) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    command = [
        str(converter),
        str(gltf_path),
        "-o",
        str(output_path),
        "--scale",
        COOK_SCALE,
        "--no-auto-textures",
        *material_arguments,
    ]
    require("--pretransform" not in command, "the cook must never pass --pretransform")
    completed = subprocess.run(command, capture_output=True, text=True, errors="replace")
    require(
        completed.returncode == 0 and output_path.is_file(),
        f"ModelAssetConverter failed for {gltf_path}: rc={completed.returncode} "
        f"{(completed.stdout or '').strip()[-300:]} {(completed.stderr or '').strip()[-300:]}",
    )


def _section_spans(data: bytes, label: str) -> list[tuple[int, int, int]]:
    """Absolute (type_id, start, size) spans of every section inside ``data``."""
    magic, major, _minor, flags, content_size = FILE_HEADER.unpack_from(data, 0)
    require(
        magic == b"WINT" and major == 1 and flags == 0 and content_size == len(data) - FILE_HEADER.size,
        f"{label} outer header is invalid",
    )
    content_base = FILE_HEADER.size
    section_count = int(MODEL_HEADER.unpack_from(data, content_base)[1])
    descriptors = [
        SECTION_DESC.unpack_from(data, content_base + MODEL_HEADER.size + index * SECTION_DESC.size)
        for index in range(section_count)
    ]
    first_offset = content_base + int(descriptors[0][2])
    if data[first_offset : first_offset + 4] == b"WINT":
        base = 0
    elif data[first_offset + MODEL_HEADER.size : first_offset + MODEL_HEADER.size + 4] == b"WINT":
        base = MODEL_HEADER.size
    else:
        raise NormalizeError(f"{label} section offset base is undecidable")
    return [
        (int(type_id), content_base + base + int(offset), int(size))
        for type_id, _index, offset, size, _name in descriptors
    ]


def reorder_cooked_bone_palette(cooked_path: Path, desired_names: Sequence[str]) -> int:
    """Permute a cooked WModel's bone palette into ``desired_names`` order.

    The converter emits the palette in scene depth-first order, so a costume
    bone parented to a mid-tree master bone lands between master bones and
    shifts every index after it.  ``CMesh::Bind_Resource`` uses palette indices
    directly against the body's palette, so the master rows must keep their
    master indices.  This rewrites the fixed-size WMSH and WSKL bone records and
    the per-vertex blend indices in place; no section changes size.
    """
    data = bytearray(cooked_path.read_bytes())
    spans = _section_spans(bytes(data), str(cooked_path))
    mesh_spans = [span for span in spans if span[0] == SECTION_MESH]
    skeleton_spans = [span for span in spans if span[0] == SECTION_SKELETON]
    require(len(mesh_spans) == 1 and len(skeleton_spans) == 1, f"cooked part is not a single skinned mesh: {cooked_path}")

    mesh_payload = mesh_spans[0][1] + FILE_HEADER.size
    (
        _magic,
        submesh_count,
        bone_count,
        vertex_flags,
        vertex_stride,
        vertex_count,
        index_count,
        index_stride,
        _has_bounding,
        _reserved,
    ) = MESH_HEADER.unpack_from(data, mesh_payload)
    require(bool(vertex_flags & VF_BONE_WEIGHT), f"cooked part is not skinned: {cooked_path}")
    vertex_base = mesh_payload + MESH_HEADER.size + submesh_count * SUBMESH_DESC.size
    mesh_bone_base = vertex_base + vertex_count * vertex_stride + index_count * index_stride

    skeleton_payload = skeleton_spans[0][1] + FILE_HEADER.size
    skeleton_bone_count = int(SKELETON_HEADER.unpack_from(data, skeleton_payload)[1])
    skeleton_bone_base = skeleton_payload + SKELETON_HEADER.size
    require(skeleton_bone_count == bone_count, f"cooked mesh and skeleton bone counts differ: {cooked_path}")

    current_names = [
        _fixed_text(MESH_BONE.unpack_from(data, mesh_bone_base + index * MESH_BONE.size)[1])
        for index in range(bone_count)
    ]
    require(len(desired_names) == bone_count, f"reorder target size differs from the cooked palette: {cooked_path}")
    require(
        sorted(current_names) == sorted(desired_names),
        f"reorder target is not a permutation of the cooked palette: {cooked_path}",
    )
    index_of_current = {name: index for index, name in enumerate(current_names)}
    require(len(index_of_current) == bone_count, f"cooked palette has duplicate bone names: {cooked_path}")
    new_to_old = [index_of_current[name] for name in desired_names]
    old_to_new = [0] * bone_count
    for new_index, old_index in enumerate(new_to_old):
        old_to_new[old_index] = new_index
    if new_to_old == list(range(bone_count)):
        return 0

    mesh_records = [
        bytes(data[mesh_bone_base + index * MESH_BONE.size : mesh_bone_base + (index + 1) * MESH_BONE.size])
        for index in range(bone_count)
    ]
    skeleton_records = [
        bytes(
            data[
                skeleton_bone_base + index * SKELETON_BONE.size :
                skeleton_bone_base + (index + 1) * SKELETON_BONE.size
            ]
        )
        for index in range(bone_count)
    ]
    new_parents: list[int] = []
    for new_index, old_index in enumerate(new_to_old):
        record = bytearray(mesh_records[old_index])
        parent = struct.unpack_from("<i", record, MESH_BONE_PARENT_OFFSET)[0]
        struct.pack_into(
            "<i", record, MESH_BONE_PARENT_OFFSET, old_to_new[parent] if parent >= 0 else -1
        )
        data[mesh_bone_base + new_index * MESH_BONE.size : mesh_bone_base + (new_index + 1) * MESH_BONE.size] = record

        skeleton_record = bytearray(skeleton_records[old_index])
        skeleton_parent = struct.unpack_from("<i", skeleton_record, SKELETON_BONE_PARENT_OFFSET)[0]
        remapped_parent = old_to_new[skeleton_parent] if skeleton_parent >= 0 else -1
        require(
            remapped_parent < new_index,
            f"reorder would break parent-before-child at {desired_names[new_index]}: {cooked_path}",
        )
        struct.pack_into("<i", skeleton_record, SKELETON_BONE_PARENT_OFFSET, remapped_parent)
        new_parents.append(remapped_parent)
        data[
            skeleton_bone_base + new_index * SKELETON_BONE.size :
            skeleton_bone_base + (new_index + 1) * SKELETON_BONE.size
        ] = skeleton_record

    children: dict[int, list[int]] = {}
    for index, parent in enumerate(new_parents):
        if parent >= 0:
            children.setdefault(parent, []).append(index)
    for index in range(bone_count):
        own = children.get(index, [])
        struct.pack_into(
            "<II",
            data,
            skeleton_bone_base + index * SKELETON_BONE.size + SKELETON_BONE_CHILD_OFFSET,
            len(own),
            min(own) if own else 0xFFFFFFFF,
        )

    for vertex_index in range(vertex_count):
        row = vertex_base + vertex_index * vertex_stride + 44
        joints = struct.unpack_from("<4I", data, row)
        require(
            all(joint < bone_count for joint in joints),
            f"cooked part has a blend index outside its palette: {cooked_path}",
        )
        struct.pack_into("<4I", data, row, *(old_to_new[joint] for joint in joints))

    cooked_path.write_bytes(bytes(data))
    return sum(1 for new_index, old_index in enumerate(new_to_old) if new_index != old_index)


def verify_cooked_part(
    master: MasterSkeleton,
    cooked_path: Path,
    appended_bones: Sequence[str],
    reference_texture_paths: Sequence[str] | None,
) -> dict[str, Any]:
    sections = read_wmodel_sections(cooked_path)
    mesh_sections = [blob for type_id, blob in sections if type_id == SECTION_MESH]
    material_sections = [blob for type_id, blob in sections if type_id == SECTION_MATERIAL]
    skeleton_sections = [blob for type_id, blob in sections if type_id == SECTION_SKELETON]
    require(len(mesh_sections) == 1, f"cooked part must hold one mesh section: {cooked_path}")
    require(len(material_sections) == 1, f"cooked part must hold one material section: {cooked_path}")
    require(len(skeleton_sections) == 1, f"cooked part must hold one skeleton section: {cooked_path}")
    palette = read_mesh_palette(mesh_sections[0], f"{cooked_path} mesh")
    skeleton = read_skeleton_bones(skeleton_sections[0], f"{cooked_path} skeleton")
    require(
        len(skeleton) == palette.bone_count and all(bone[0] == name for bone, name in zip(skeleton, palette.names)),
        f"cooked part mesh palette and skeleton disagree: {cooked_path}",
    )
    expected = list(master.names)
    expected[master.mesh_slot_index] = palette.names[master.mesh_slot_index]
    expected += list(appended_bones)
    require(
        list(palette.names) == expected,
        f"cooked palette does not match the master order: {cooked_path}",
    )
    require(
        palette.max_joint_index < palette.bone_count,
        f"cooked part has a blend index outside its palette: {cooked_path}",
    )
    require(palette.weighted_bones, f"cooked part carries no positive blend weight: {cooked_path}")
    bind_drift = 0.0
    drift_name = ""
    for bone_index in sorted(palette.weighted_bones):
        if bone_index >= len(master.bones) or bone_index == master.mesh_slot_index:
            continue
        delta = max(
            abs(left - right)
            for left, right in zip(palette.inverse_binds[bone_index], master.bones[bone_index].inverse_bind)
        )
        if delta > bind_drift:
            bind_drift = delta
            drift_name = palette.names[bone_index]
    require(
        bind_drift <= 1e-4,
        f"cooked part bind pose left the master at {drift_name}: {bind_drift} ({cooked_path})",
    )
    texture_paths = read_material_texture_paths(material_sections[0], f"{cooked_path} material")
    missing = [
        path for path in texture_paths if not (cooked_path.parent / Path(*PurePosixPath(path).parts)).is_file()
    ]
    require(not missing, f"cooked part references missing textures {missing}: {cooked_path}")
    texture_drift: list[str] = []
    if reference_texture_paths is not None and tuple(reference_texture_paths) != texture_paths:
        texture_drift = list(texture_paths)
    return {
        "boneCount": palette.bone_count,
        "vertexCount": palette.vertex_count,
        "maxBlendIndex": palette.max_joint_index,
        "meshSlotBone": palette.names[master.mesh_slot_index],
        "weightedBoneCount": len(palette.weighted_bones),
        "maxWeightedBindDelta": bind_drift,
        "texturePaths": list(texture_paths),
        "texturePathDrift": texture_drift,
    }


# ---------------------------------------------------------------------------
# driver
# ---------------------------------------------------------------------------


def resolved_resource_path(resource_root: Path, asset_id: str) -> Path:
    """Resolve a Resources-relative asset ID, refusing escapes and absolutes."""
    normalized = asset_id.replace("\\", "/")
    parts = PurePosixPath(normalized).parts
    require(
        normalized
        and not PurePosixPath(normalized).is_absolute()
        and not re.match(r"^[A-Za-z]:", normalized)
        and all(part not in ("", ".", "..") for part in parts),
        f"asset ID must be a contained Resources-relative path: {asset_id}",
    )
    return resource_root / Path(*parts)


def class_body_assets(catalog_path: Path) -> dict[str, str]:
    """Read each class body's Resources-relative WModel from the character catalog."""
    document = json.loads(catalog_path.read_text(encoding="utf-8"))
    require(
        document.get("schema") == "lostark.character-catalog",
        f"character catalog schema is unexpected: {catalog_path}",
    )
    bodies: dict[str, str] = {}
    for entry in document.get("characters") or ():
        asset_id = entry.get("assetId")
        body_model = entry.get("bodyModel")
        require(
            isinstance(asset_id, str) and isinstance(body_model, str) and asset_id and body_model,
            f"character catalog entry is missing assetId or bodyModel: {catalog_path}",
        )
        require(asset_id not in bodies, f"character catalog lists {asset_id} twice: {catalog_path}")
        bodies[asset_id] = body_model
    require(bodies, f"character catalog has no characters: {catalog_path}")
    return bodies


def run(arguments: argparse.Namespace) -> dict[str, Any]:
    repo_root = Path(arguments.repo_root).resolve()
    source_root = Path(arguments.source_root).resolve()
    resource_root = Path(arguments.resource_root).resolve()
    work_root = Path(arguments.work_root).resolve()
    converter = Path(arguments.converter) if arguments.converter else repo_root / CONVERTER_RELATIVE_PATH
    require(converter.is_file(), f"ModelAssetConverter is missing: {converter}")
    catalog_path = (
        Path(arguments.character_catalog).resolve()
        if arguments.character_catalog
        else repo_root / CHARACTER_CATALOG_RELATIVE_PATH
    )
    require(catalog_path.is_file(), f"character catalog is missing: {catalog_path}")
    class_bodies = class_body_assets(catalog_path)

    plans = build_part_plans(Path(arguments.outfits).resolve(), source_root, arguments.classes)
    require(plans, "the outfit inventory produced no parts")

    masters: dict[str, MasterSkeleton] = {}
    for plan in plans:
        if plan.class_id in masters:
            continue
        relative = class_bodies.get(plan.class_id)
        require(relative is not None, f"the character catalog has no body for {plan.class_id}")
        masters[plan.class_id] = load_master_skeleton(
            plan.class_id, resolved_resource_path(resource_root, relative)
        )

    rows: list[dict[str, Any]] = []
    installed = 0
    for plan in plans:
        row: dict[str, Any] = {
            "class": plan.class_id,
            "set": plan.set_id,
            "part": plan.part_id,
            "sourceGltf": str(plan.source_gltf),
            "targetAssetId": plan.target_asset_id,
            "state": "FAILED",
        }
        target_path = resolved_resource_path(resource_root, plan.target_asset_id)
        try:
            master = masters[plan.class_id]
            source = load_source_part(plan.source_gltf)
            normalized = normalize_part(master, source, arguments.inverse_bind_tolerance)
            gltf_path = work_root / "normalized" / plan.class_id / plan.set_id / f"{plan.part_id}.gltf"
            write_normalized_gltf(normalized, gltf_path)
            row.update(
                {
                    "normalizedGltf": str(gltf_path),
                    "sourceJointCount": len(source.joints),
                    "masterBoneCount": len(master.bones),
                    "normalizedBoneCount": len(normalized.node_names),
                    "appendedBones": list(normalized.appended_bones),
                    "appendedBoneCount": len(normalized.appended_bones),
                    "unusedMasterBones": normalized.unused_master_bones,
                    "maxInverseBindDelta": round(normalized.max_inverse_bind_delta, 6),
                    "comparedJointCount": normalized.compared_joint_count,
                    "state": "NORMALIZED",
                }
            )
            if arguments.mode in ("cook", "install"):
                material_names = [str(material.get("name", "")) for material in source.materials]
                material_arguments, missing = build_material_remap_arguments(plan, material_names)
                row["missingTextures"] = missing
                cooked_path = work_root / "cooked" / plan.class_id / plan.set_id / f"{plan.part_id}.wmodel"
                cook_normalized_gltf(converter, gltf_path, cooked_path, material_arguments)
                row["reorderedBones"] = reorder_cooked_bone_palette(cooked_path, normalized.node_names)
                reference = None
                if target_path.is_file():
                    reference_sections = read_wmodel_sections(target_path)
                    reference_material = [
                        blob for type_id, blob in reference_sections if type_id == SECTION_MATERIAL
                    ]
                    if reference_material:
                        reference = read_material_texture_paths(reference_material[0], f"{target_path} material")
                row["verification"] = verify_cooked_part(
                    master, cooked_path, normalized.appended_bones, reference
                )
                row["cookedWmodel"] = str(cooked_path)
                row["state"] = "COOKED"
            if arguments.mode == "install":
                require(target_path.is_file(), f"target WModel is missing, refusing to create it: {target_path}")
                shutil.copyfile(cooked_path, target_path)
                installed += 1
                row["state"] = "INSTALLED"
        except NormalizeError as error:
            row["error"] = str(error)
        except (OSError, ValueError, KeyError) as error:
            row["error"] = f"{type(error).__name__}: {error}"
        rows.append(row)
        print(
            f"{row['state']:10s} {plan.class_id}/{plan.set_id}/{plan.part_id}"
            + (f" appended={row.get('appendedBoneCount', '-')}" if "appendedBoneCount" in row else "")
            + (f" | {row['error']}" if "error" in row else "")
        )

    report = {
        "schema": REPORT_SCHEMA,
        "formatVersion": REPORT_FORMAT_VERSION,
        "mode": arguments.mode,
        "converter": str(converter),
        "cookArguments": ["--scale", COOK_SCALE, "--no-auto-textures"],
        "inverseBindTolerance": arguments.inverse_bind_tolerance,
        "classBoneCounts": {
            class_id: len(master.bones) for class_id, master in sorted(masters.items())
        },
        "partCount": len(rows),
        "succeeded": sum(1 for row in rows if row["state"] in ("NORMALIZED", "COOKED", "INSTALLED")),
        "failed": sum(1 for row in rows if row["state"] == "FAILED"),
        "installed": installed,
        "parts": rows,
    }
    if arguments.report:
        report_path = Path(arguments.report).resolve()
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(json.dumps(report, indent=1, ensure_ascii=False), encoding="utf-8")
    return report


def parse_arguments(argv: Iterable[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--repo-root", required=True)
    parser.add_argument("--source-root", required=True, help="extracted glTF root, e.g. CostumeCook/raw")
    parser.add_argument("--outfits", required=True, help="outfits.json part inventory")
    parser.add_argument("--resource-root", required=True, help="Client/Bin/Resources")
    parser.add_argument("--work-root", required=True, help="scratch root for normalized and cooked output")
    parser.add_argument("--converter", default=None)
    parser.add_argument("--character-catalog", default=None)
    parser.add_argument("--mode", choices=("normalize", "cook", "install"), default="normalize")
    parser.add_argument("--classes", nargs="*", default=None)
    parser.add_argument("--inverse-bind-tolerance", type=float, default=DEFAULT_INVERSE_BIND_TOLERANCE)
    parser.add_argument("--report", default=None)
    return parser.parse_args(list(argv))


def main(argv: Iterable[str] | None = None) -> int:
    arguments = parse_arguments(sys.argv[1:] if argv is None else argv)
    try:
        report = run(arguments)
    except NormalizeError as error:
        print(f"error: {error}", file=sys.stderr)
        return 2
    print(
        f"parts={report['partCount']} succeeded={report['succeeded']} "
        f"failed={report['failed']} installed={report['installed']}"
    )
    return 1 if report["failed"] else 0


if __name__ == "__main__":
    raise SystemExit(main())
