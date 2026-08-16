#!/usr/bin/env python3
"""Verify the exact bind/pose contract of DimensionMaster's summon WModel.

The legacy ActorX -> Blender -> FBX path could leave an imported Action active
while exporting.  Blender then wrote the evaluated animation pose into WSKL as
the skeleton rest transform while Assimp preserved the PSK bind matrices in
WMSH.  Every matrix was finite, but the resulting per-bone basis mismatch tore
the clock meshes into the large radial/concentric cage seen in the Client.

This verifier intentionally checks more than finite bounds:

* the exact 20-bone source topology is present once;
* ``inverseBind * restCombined`` is a single common mesh basis for all source
  bones (normalising that common basis must produce identity);
* vertex skin data and four material sections are unchanged; and
* every animation is sampled through the same local -> combined -> skin matrix
  order as CModel/CMesh, producing stable per-bone topology witnesses.
"""

from __future__ import annotations

import argparse
import bisect
import hashlib
import json
import math
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable


FILE_HEADER = struct.Struct("<4sHHII")
MODEL_HEADER = struct.Struct("<4sIII4I")
SECTION_DESC = struct.Struct("<IIQQ40s")
MESH_HEADER = struct.Struct("<4sIIIIIIIB3s")
SUBMESH_DESC = struct.Struct("<IIIIIQ20s")
MESH_BONE = struct.Struct("<Q32si16fI16s")
SKELETON_HEADER = struct.Struct("<4sII5I")
SKELETON_BONE = struct.Struct("<Q64si16fII27I")
ANIMATION_HEADER = struct.Struct("<4sIffIIB7s")
ANIMATION_CHANNEL = struct.Struct("<QIIIIIIiI")
VECTOR_KEY = struct.Struct("<4f")
QUATERNION_KEY = struct.Struct("<5f")

EXPECTED_SOURCE_BONES = ["b_body"] + [
    f"b_clock_{index:02d}" for index in range(1, 20)
]
EXPECTED_SUBMESH_COUNT = 4
EXPECTED_SOURCE_VERTEX_COUNT = 12_891
EXPECTED_SOURCE_FACE_COUNT = 13_689


def fixed_name(value: bytes) -> str:
    return value.split(b"\0", 1)[0].decode("ascii", "strict")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def matrix_multiply(left: list[float], right: list[float]) -> list[float]:
    return [
        sum(left[row * 4 + index] * right[index * 4 + column]
            for index in range(4))
        for row in range(4)
        for column in range(4)
    ]


def matrix_inverse(value: list[float]) -> list[float]:
    rows = [
        [float(value[row * 4 + column]) for column in range(4)]
        + [1.0 if row == column else 0.0 for column in range(4)]
        for row in range(4)
    ]
    for column in range(4):
        pivot = max(range(column, 4), key=lambda row: abs(rows[row][column]))
        require(abs(rows[pivot][column]) > 1e-12, "bind basis is singular")
        rows[column], rows[pivot] = rows[pivot], rows[column]
        divisor = rows[column][column]
        rows[column] = [entry / divisor for entry in rows[column]]
        for row in range(4):
            if row == column:
                continue
            factor = rows[row][column]
            rows[row] = [
                rows[row][index] - factor * rows[column][index]
                for index in range(8)
            ]
    return [rows[row][4 + column] for row in range(4) for column in range(4)]


def identity_error(value: list[float]) -> float:
    return max(
        abs(value[index] - (1.0 if index // 4 == index % 4 else 0.0))
        for index in range(16)
    )


def transform_point(point: tuple[float, float, float], matrix: list[float]) -> tuple[float, float, float]:
    x, y, z = point
    result = (
        x * matrix[0] + y * matrix[4] + z * matrix[8] + matrix[12],
        x * matrix[1] + y * matrix[5] + z * matrix[9] + matrix[13],
        x * matrix[2] + y * matrix[6] + z * matrix[10] + matrix[14],
    )
    w = x * matrix[3] + y * matrix[7] + z * matrix[11] + matrix[15]
    require(math.isfinite(w) and abs(w) > 1e-12, "skin matrix produced invalid W")
    return tuple(component / w for component in result)


def affine_matrix(
    scale: tuple[float, float, float],
    rotation: tuple[float, float, float, float],
    translation: tuple[float, float, float],
) -> list[float]:
    x, y, z, w = rotation
    length = math.sqrt(x * x + y * y + z * z + w * w)
    require(math.isfinite(length) and length > 1e-12, "animation quaternion is invalid")
    x, y, z, w = x / length, y / length, z / length, w / length
    matrix = [
        1.0 - 2.0 * y * y - 2.0 * z * z,
        2.0 * x * y + 2.0 * z * w,
        2.0 * x * z - 2.0 * y * w,
        0.0,
        2.0 * x * y - 2.0 * z * w,
        1.0 - 2.0 * x * x - 2.0 * z * z,
        2.0 * y * z + 2.0 * x * w,
        0.0,
        2.0 * x * z + 2.0 * y * w,
        2.0 * y * z - 2.0 * x * w,
        1.0 - 2.0 * x * x - 2.0 * y * y,
        0.0,
        translation[0], translation[1], translation[2], 1.0,
    ]
    for row in range(3):
        for column in range(4):
            matrix[row * 4 + column] *= scale[row]
    return matrix


@dataclass
class Bone:
    name_hash: int
    name: str
    parent: int
    transform: list[float]


@dataclass
class Vertex:
    position: tuple[float, float, float]
    indices: tuple[int, int, int, int]
    weights: tuple[float, float, float, float]
    submesh: int


@dataclass
class AnimationChannel:
    bone_index: int
    position_keys: list[tuple[float, float, float, float]]
    rotation_keys: list[tuple[float, float, float, float, float]]
    scale_keys: list[tuple[float, float, float, float]]


@dataclass
class Animation:
    name: str
    duration_ticks: float
    ticks_per_second: float
    channels: list[AnimationChannel]


@dataclass
class WModel:
    submeshes: list[tuple[int, ...]]
    vertices: list[Vertex]
    mesh_bones: list[Bone]
    skeleton_bones: list[Bone]
    animations: list[Animation]


def _read_nested_header(data: bytes, offset: int, size: int, label: str) -> int:
    require(offset + FILE_HEADER.size <= len(data), f"{label} is truncated")
    magic, major, _, flags, content_size = FILE_HEADER.unpack_from(data, offset)
    require(
        magic == b"WINT" and major == 1 and flags == 0
        and content_size == size - FILE_HEADER.size,
        f"{label} WINT header is invalid",
    )
    return offset + FILE_HEADER.size


def read_wmodel(path: Path) -> WModel:
    data = path.read_bytes()
    require(len(data) >= FILE_HEADER.size + MODEL_HEADER.size, "WModel is truncated")
    magic, major, _, flags, content_size = FILE_HEADER.unpack_from(data, 0)
    require(
        magic == b"WINT" and major == 1 and flags == 0
        and content_size == len(data) - FILE_HEADER.size,
        "WModel WINT header is invalid",
    )
    content = FILE_HEADER.size
    model = MODEL_HEADER.unpack_from(data, content)
    require(model[0] == b"WMOD" and model[1] >= 3, "WModel WMOD header is invalid")
    section_table = content + MODEL_HEADER.size
    require(
        section_table + model[1] * SECTION_DESC.size <= len(data),
        "WModel section table is truncated",
    )
    sections: list[tuple[int, int, int, int, str]] = []
    for index in range(model[1]):
        type_id, section_index, offset, size, raw_name = SECTION_DESC.unpack_from(
            data, section_table + index * SECTION_DESC.size
        )
        require(
            offset <= content_size and size <= content_size - offset,
            f"WModel section {index} is out of range",
        )
        sections.append((type_id, section_index, content + offset, size, fixed_name(raw_name)))

    mesh_sections = [row for row in sections if row[0] == 1]
    skeleton_sections = [row for row in sections if row[0] == 3]
    require(len(mesh_sections) == 1 and len(skeleton_sections) == 1,
            "WModel must contain one mesh and one skeleton")

    _, _, mesh_offset, mesh_size, _ = mesh_sections[0]
    offset = _read_nested_header(data, mesh_offset, mesh_size, "WMSH")
    mesh_header = MESH_HEADER.unpack_from(data, offset)
    require(mesh_header[0] == b"WMSH", "WModel mesh has no WMSH header")
    submesh_count = mesh_header[1]
    bone_count = mesh_header[2]
    vertex_stride = mesh_header[4]
    vertex_count = mesh_header[5]
    index_count = mesh_header[6]
    index_stride = mesh_header[7]
    require(vertex_stride == 76 and index_stride in (2, 4),
            "WMSH skinned vertex contract is invalid")
    offset += MESH_HEADER.size
    require(offset + submesh_count * SUBMESH_DESC.size <= len(data),
            "WMSH submesh table is truncated")
    submeshes = [
        SUBMESH_DESC.unpack_from(data, offset + index * SUBMESH_DESC.size)
        for index in range(submesh_count)
    ]
    offset += submesh_count * SUBMESH_DESC.size
    vertex_blob = offset
    offset += vertex_count * vertex_stride
    offset += index_count * index_stride
    require(offset + bone_count * MESH_BONE.size <= mesh_offset + mesh_size,
            "WMSH bone table is truncated")
    mesh_bones: list[Bone] = []
    for index in range(bone_count):
        row = MESH_BONE.unpack_from(data, offset + index * MESH_BONE.size)
        mesh_bones.append(Bone(row[0], fixed_name(row[1]), row[2], list(row[3:19])))

    vertices: list[Vertex] = []
    for submesh_index, submesh in enumerate(submeshes):
        byte_offset, count = submesh[0], submesh[1]
        require(byte_offset + count * vertex_stride <= vertex_count * vertex_stride,
                "WMSH submesh vertex span is invalid")
        for index in range(count):
            row = vertex_blob + byte_offset + index * vertex_stride
            position = struct.unpack_from("<3f", data, row)
            indices = struct.unpack_from("<4I", data, row + 44)
            weights = struct.unpack_from("<4f", data, row + 60)
            vertices.append(Vertex(position, indices, weights, submesh_index))
    require(len(vertices) == vertex_count, "WMSH aggregate vertex count differs")

    _, _, skeleton_offset, skeleton_size, _ = skeleton_sections[0]
    offset = _read_nested_header(data, skeleton_offset, skeleton_size, "WSKL")
    skeleton_header = SKELETON_HEADER.unpack_from(data, offset)
    require(skeleton_header[0] == b"WSKL", "WModel skeleton has no WSKL header")
    offset += SKELETON_HEADER.size
    skeleton_bones: list[Bone] = []
    for index in range(skeleton_header[1]):
        row = SKELETON_BONE.unpack_from(data, offset + index * SKELETON_BONE.size)
        skeleton_bones.append(Bone(row[0], fixed_name(row[1]), row[2], list(row[3:19])))
    require(
        [(bone.name_hash, bone.name) for bone in mesh_bones]
        == [(bone.name_hash, bone.name) for bone in skeleton_bones],
        "WMSH and WSKL bone order differs",
    )

    hash_to_bone = {bone.name_hash: index for index, bone in enumerate(skeleton_bones)}
    require(len(hash_to_bone) == len(skeleton_bones), "WSKL contains duplicate bone hashes")
    animations: list[Animation] = []
    for type_id, section_index, animation_offset, animation_size, name in sections:
        if type_id != 4:
            continue
        offset = _read_nested_header(data, animation_offset, animation_size, f"WANM {section_index}")
        header = ANIMATION_HEADER.unpack_from(data, offset)
        require(header[0] == b"WANM", f"animation {section_index} has no WANM header")
        channel_count = header[1]
        duration = header[2]
        ticks_per_second = header[3]
        event_count = header[5]
        offset += ANIMATION_HEADER.size
        channel_rows = [
            ANIMATION_CHANNEL.unpack_from(data, offset + index * ANIMATION_CHANNEL.size)
            for index in range(channel_count)
        ]
        key_block = offset + channel_count * ANIMATION_CHANNEL.size
        key_block_size = animation_offset + animation_size - key_block - event_count * 32 - 8
        require(key_block_size >= 0, f"animation {section_index} key block is invalid")

        def vector_keys(byte_offset: int, count: int) -> list[tuple[float, float, float, float]]:
            require(byte_offset + count * VECTOR_KEY.size <= key_block_size,
                    f"animation {section_index} vector key span is invalid")
            return [VECTOR_KEY.unpack_from(data, key_block + byte_offset + key * VECTOR_KEY.size)
                    for key in range(count)]

        def quaternion_keys(byte_offset: int, count: int) -> list[tuple[float, float, float, float, float]]:
            require(byte_offset + count * QUATERNION_KEY.size <= key_block_size,
                    f"animation {section_index} quaternion key span is invalid")
            return [QUATERNION_KEY.unpack_from(
                data, key_block + byte_offset + key * QUATERNION_KEY.size)
                    for key in range(count)]

        channels: list[AnimationChannel] = []
        seen_bones: set[int] = set()
        for row in channel_rows:
            require(row[0] in hash_to_bone, f"animation {section_index} has an unknown bone")
            bone_index = hash_to_bone[row[0]]
            require(bone_index not in seen_bones, f"animation {section_index} duplicates a bone")
            seen_bones.add(bone_index)
            channels.append(AnimationChannel(
                bone_index,
                vector_keys(row[2], row[1]),
                quaternion_keys(row[4], row[3]),
                vector_keys(row[6], row[5]),
            ))
        animations.append(Animation(name, duration, ticks_per_second, channels))
    animations.sort(key=lambda animation: animation.name)
    require(len(animations) == model[2], "WMOD animation count differs")
    return WModel(submeshes, vertices, mesh_bones, skeleton_bones, animations)


def combined_transforms(bones: list[Bone], local: list[list[float]]) -> list[list[float]]:
    result: list[list[float]] = []
    for index, bone in enumerate(bones):
        require(bone.parent < index, f"bone hierarchy is not parent-before-child: {bone.name}")
        result.append(
            local[index] if bone.parent < 0
            else matrix_multiply(local[index], result[bone.parent])
        )
    return result


def sample_vector(
    keys: list[tuple[float, float, float, float]],
    time: float,
    fallback: tuple[float, float, float],
) -> tuple[float, float, float]:
    if not keys:
        return fallback
    if time <= keys[0][0]:
        return keys[0][1:4]
    if time >= keys[-1][0]:
        return keys[-1][1:4]
    right = bisect.bisect_right([key[0] for key in keys], time)
    left_key, right_key = keys[right - 1], keys[right]
    ratio = (time - left_key[0]) / (right_key[0] - left_key[0])
    return tuple(
        left_key[index] + (right_key[index] - left_key[index]) * ratio
        for index in range(1, 4)
    )


def sample_quaternion(
    keys: list[tuple[float, float, float, float, float]], time: float
) -> tuple[float, float, float, float]:
    if not keys:
        return (0.0, 0.0, 0.0, 1.0)
    if time <= keys[0][0]:
        return keys[0][1:5]
    if time >= keys[-1][0]:
        return keys[-1][1:5]
    right = bisect.bisect_right([key[0] for key in keys], time)
    left_key, right_key = keys[right - 1], keys[right]
    ratio = (time - left_key[0]) / (right_key[0] - left_key[0])
    left = list(left_key[1:5])
    right_value = list(right_key[1:5])
    dot = sum(a * b for a, b in zip(left, right_value))
    if dot < 0.0:
        right_value = [-value for value in right_value]
        dot = -dot
    if dot > 0.9995:
        result = [left[index] + ratio * (right_value[index] - left[index])
                  for index in range(4)]
    else:
        theta = math.acos(max(-1.0, min(1.0, dot)))
        denominator = math.sin(theta)
        left_weight = math.sin((1.0 - ratio) * theta) / denominator
        right_weight = math.sin(ratio * theta) / denominator
        result = [left_weight * left[index] + right_weight * right_value[index]
                  for index in range(4)]
    length = math.sqrt(sum(value * value for value in result))
    return tuple(value / length for value in result)


def bounds(points: Iterable[tuple[float, float, float]]) -> dict[str, Any]:
    values = list(points)
    require(values, "topology witness has no points")
    minimum = [min(point[axis] for point in values) for axis in range(3)]
    maximum = [max(point[axis] for point in values) for axis in range(3)]
    center = [sum(point[axis] for point in values) / len(values) for axis in range(3)]
    distances = sorted(math.dist(point, center) for point in values)
    return {
        "minimum": [round(value, 6) for value in minimum],
        "maximum": [round(value, 6) for value in maximum],
        "center": [round(value, 6) for value in center],
        "diagonal": round(math.dist(minimum, maximum), 6),
        "radiusP50": round(distances[len(distances) // 2], 6),
        "radiusP95": round(distances[int(len(distances) * 0.95)], 6),
        "radiusMaximum": round(distances[-1], 6),
    }


def sample_animation(model: WModel, animation: Animation, time: float) -> dict[str, Any]:
    require(0.0 <= time <= animation.duration_ticks, "animation sample is out of range")
    local = [list(bone.transform) for bone in model.skeleton_bones]
    for channel in animation.channels:
        local[channel.bone_index] = affine_matrix(
            sample_vector(channel.scale_keys, time, (1.0, 1.0, 1.0)),
            sample_quaternion(channel.rotation_keys, time),
            sample_vector(channel.position_keys, time, (0.0, 0.0, 0.0)),
        )
    combined = combined_transforms(model.skeleton_bones, local)
    skin = [
        matrix_multiply(model.mesh_bones[index].transform, combined[index])
        for index in range(len(model.mesh_bones))
    ]
    points: list[tuple[float, float, float]] = []
    per_bone: dict[int, list[tuple[float, float, float]]] = {}
    for vertex in model.vertices:
        weight_sum = sum(vertex.weights)
        require(math.isfinite(weight_sum) and weight_sum > 1e-6,
                "skinned vertex has no finite weight")
        point = [0.0, 0.0, 0.0]
        dominant = max(range(4), key=lambda index: vertex.weights[index])
        for influence in range(4):
            weight = vertex.weights[influence] / weight_sum
            if weight <= 0.0:
                continue
            bone_index = vertex.indices[influence]
            require(bone_index < len(skin), "skinned vertex bone index is out of range")
            transformed = transform_point(vertex.position, skin[bone_index])
            for axis in range(3):
                point[axis] += transformed[axis] * weight
        final_point = tuple(point)
        points.append(final_point)
        per_bone.setdefault(vertex.indices[dominant], []).append(final_point)
    bone_centers = {}
    for bone_index, bone_points in per_bone.items():
        name = model.skeleton_bones[bone_index].name
        if name in EXPECTED_SOURCE_BONES:
            bone_centers[name] = bounds(bone_points)["center"]
    return {
        "timeTicks": time,
        "bounds": bounds(points),
        "sourceBoneCenters": bone_centers,
    }


def verify_animation_motion(model: WModel, animation: Animation) -> list[str]:
    """Reject a REST-mode FBX bake that turns every source clock into a still."""
    moving: list[str] = []
    for channel in animation.channels:
        name = model.skeleton_bones[channel.bone_index].name
        if name not in EXPECTED_SOURCE_BONES:
            continue
        maximum_range = 0.0
        for keys in (channel.position_keys, channel.rotation_keys, channel.scale_keys):
            if not keys:
                continue
            for component in range(1, len(keys[0])):
                values = [key[component] for key in keys]
                maximum_range = max(maximum_range, max(values) - min(values))
        if maximum_range > 1e-5:
            moving.append(name)
    expected = EXPECTED_SOURCE_BONES[1:]
    require(
        moving == expected,
        "ActorX animation lost source clock motion: "
        f"animation={animation.name} moving={','.join(moving)}",
    )
    return moving


def verify_bind_pose(model: WModel, maximum_identity_error: float) -> dict[str, Any]:
    names = [bone.name for bone in model.skeleton_bones]
    rest_combined = combined_transforms(
        model.skeleton_bones, [list(bone.transform) for bone in model.skeleton_bones]
    )
    bind_products = {
        name: matrix_multiply(
            model.mesh_bones[names.index(name)].transform,
            rest_combined[names.index(name)],
        )
        for name in EXPECTED_SOURCE_BONES
    }
    inverse_common_basis = matrix_inverse(bind_products["b_body"])
    errors = {
        name: identity_error(matrix_multiply(product, inverse_common_basis))
        for name, product in bind_products.items()
    }
    worst_name = max(errors, key=errors.get)
    worst_error = errors[worst_name]
    require(
        math.isfinite(worst_error) and worst_error <= maximum_identity_error,
        "source bind/rest basis mismatch: "
        f"bone={worst_name} normalizedIdentityError={worst_error:.9f} "
        f"maximum={maximum_identity_error:.9f}",
    )
    return {
        "commonBasisBone": "b_body",
        "maximumNormalizedIdentityError": worst_error,
        "worstBone": worst_name,
        "perBoneNormalizedIdentityError": errors,
    }


def verify(path: Path, maximum_identity_error: float = 1e-3) -> dict[str, Any]:
    model = read_wmodel(path)
    require(len(model.submeshes) == EXPECTED_SUBMESH_COUNT,
            f"expected four summon sections, got {len(model.submeshes)}")
    names = [bone.name for bone in model.skeleton_bones]
    for expected in EXPECTED_SOURCE_BONES:
        require(names.count(expected) == 1, f"source bone is missing or duplicated: {expected}")
    body_index = names.index("b_body")
    for expected in EXPECTED_SOURCE_BONES[1:]:
        index = names.index(expected)
        require(model.skeleton_bones[index].parent == body_index,
                f"source clock hierarchy differs: {expected}")

    nonzero_influence_histogram = [0, 0, 0, 0, 0]
    for vertex in model.vertices:
        count = 0
        for index, weight in zip(vertex.indices, vertex.weights):
            require(index < len(model.skeleton_bones), "vertex bone index is out of range")
            require(math.isfinite(weight) and weight >= 0.0, "vertex weight is invalid")
            if weight > 1e-6:
                count += 1
        require(count > 0, "vertex has no skin influence")
        nonzero_influence_histogram[count] += 1

    bind_pose = verify_bind_pose(model, maximum_identity_error)

    animation_witnesses = []
    for animation in model.animations:
        require(abs(animation.ticks_per_second - 30.0) <= 1e-5,
                f"summon animation rate is not the source 30 FPS: {animation.name}")
        times = sorted({0.0, animation.duration_ticks * 0.5, animation.duration_ticks})
        moving_source_bones = verify_animation_motion(model, animation)
        animation_witnesses.append({
            "name": animation.name,
            "durationTicks": animation.duration_ticks,
            "ticksPerSecond": animation.ticks_per_second,
            "channelCount": len(animation.channels),
            "movingSourceBones": moving_source_bones,
            "samples": [sample_animation(model, animation, time) for time in times],
        })

    return {
        "schema": "lostark.dimensionmaster-summon-bind-pose-witness",
        "formatVersion": 1,
        "wmodel": str(path),
        "wmodelSha256": sha256_file(path),
        "submeshCount": len(model.submeshes),
        "cookedVertexCount": len(model.vertices),
        "sourceTopology": {
            "sourceVertexCount": EXPECTED_SOURCE_VERTEX_COUNT,
            "sourceFaceCount": EXPECTED_SOURCE_FACE_COUNT,
            "sourceBoneCount": len(EXPECTED_SOURCE_BONES),
            "sourceBones": EXPECTED_SOURCE_BONES,
            "nonzeroInfluenceHistogram": nonzero_influence_histogram,
        },
        "bindPose": bind_pose,
        "animations": animation_witnesses,
    }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--wmodel", required=True, type=Path)
    parser.add_argument("--receipt", type=Path)
    parser.add_argument("--maximum-identity-error", type=float, default=1e-3)
    args = parser.parse_args()
    witness = verify(args.wmodel.resolve(strict=True), args.maximum_identity_error)
    if args.receipt:
        args.receipt.parent.mkdir(parents=True, exist_ok=True)
        args.receipt.write_text(
            json.dumps(witness, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
    print(
        "DIMENSIONMASTER_SUMMON_BIND_POSE_PASS "
        f"sha256={witness['wmodelSha256']} "
        f"identityError={witness['bindPose']['maximumNormalizedIdentityError']:.9g} "
        f"animations={len(witness['animations'])}"
    )


if __name__ == "__main__":
    main()
