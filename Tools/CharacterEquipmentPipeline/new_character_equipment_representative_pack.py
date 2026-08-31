#!/usr/bin/env python3
"""Build a validated representative character-equipment staging pack.

This tool does not mutate Client/Bin/Resources.  It joins an exact authored
selection to the extracted source inventory, verifies the already-cooked
WModel against the owning character body, copies only the selected source
objects and runtime texture closure, and writes an admission receipt under
``out/CharacterEquipmentExtraction/Admission``.

The first batch intentionally reuses WModels that already pass the current
runtime path.  Raw glTF that still needs master-skeleton normalization remains
outside the admitted set instead of being copied into Resources unsafely.
"""

from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import math
import os
import re
import shutil
import struct
import sys
import uuid
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import Any, Iterable


SELECTION_SCHEMA = "lostark.character-equipment-representative-selection"
INVENTORY_SCHEMA = "lostark.character-equipment-source-inventory"
RECEIPT_SCHEMA = "lostark.character-equipment-runtime-admission"
SOURCE_ASSOCIATION_POLICY = "CURATED_EXISTING_WMODEL_ASSOCIATION_NOT_BUILD_PROVENANCE"

ALLOWED_CLASSES = {
    "LANCE_MASTER",
    "GUNSLINGER",
    "SLAYER",
    "ARTIST",
    "WARLORD",
    "DIMENSIONMASTER",
}
CLASS_ORDER = (
    "DIMENSIONMASTER",
    "ARTIST",
    "LANCE_MASTER",
    "WARLORD",
    "SLAYER",
    "GUNSLINGER",
)
ALLOWED_SLOTS = {"HEAD", "SHOULDER", "UPPER", "LOWER", "HANDS", "WEAPON"}
ALLOWED_ATTACHMENT_MODES = {"SKINNED", "SOCKETED"}
ALLOWED_MODEL_KINDS = {"ANIM_SKINNED", "NONANIM_SOCKETED"}
ALLOWED_CATALOG_STATUSES = {"READY_ALTERNATIVE", "BASELINE_PART", "BASELINE_WEAPON"}
PENDING_NORMALIZATION_STATE = "MASTER_SKELETON_NORMALIZATION_REQUIRED"
STABLE_ID_PATTERN = re.compile(r"^[a-z0-9][a-z0-9._-]*$")
PART_ID_PATTERN = re.compile(r"^[a-z0-9][a-z0-9_-]*$")
PACK_NAME_PATTERN = re.compile(r"^[a-z0-9][a-z0-9._-]*$")

FILE_HEADER = struct.Struct("<4sHHII")
MODEL_HEADER = struct.Struct("<4sIII4I")
SECTION_DESC = struct.Struct("<IIQQ40s")
MESH_HEADER = struct.Struct("<4sIIIIIIIB3s")
SUBMESH_DESC = struct.Struct("<IIIIIQ20s")
MESH_BONE = struct.Struct("<Q32si16fI16s")
SKELETON_HEADER = struct.Struct("<4sII5I")
SKELETON_BONE = struct.Struct("<Q64si16fII27I")
MATERIAL_META = struct.Struct("<4sI")
MATERIAL_ENTRY_V1 = struct.Struct("<IQ64s520s")
MATERIAL_ENTRY_V2 = struct.Struct("<IQ64s" + "520s" * 9)
MATERIAL_ENTRY_V3 = struct.Struct("<IQ64s" + "520s" * 10 + "16f")
MESH_BOUNDS_V1 = struct.Struct("<3f3f3ff")

VF_POSITION = 1 << 0
VF_NORMAL = 1 << 1
VF_TEXCOORD0 = 1 << 2
VF_TANGENT = 1 << 3
VF_BONE_WEIGHT = 1 << 4
VF_STATIC_BASE = VF_POSITION | VF_NORMAL | VF_TEXCOORD0 | VF_TANGENT
STRIDE_STATIC = 48
STRIDE_SKINNED = 76
MAX_SUBMESHES = 2048
MAX_BONES = 512
MAX_VERTICES = 10_000_000
MAX_MATERIALS = 4096


class PackError(ValueError):
    """A fail-closed selection, source, model, or pack contract violation."""


def require(condition: bool, message: str) -> None:
    if not condition:
        raise PackError(message)


def sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest().upper()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise PackError(f"failed to read JSON {path}: {error}") from error
    require(isinstance(value, dict), f"JSON root must be an object: {path}")
    return value


def write_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
        newline="\n",
    )


def fixed_text(raw: bytes) -> str:
    return raw.split(b"\0", 1)[0].decode("utf-8", "replace")


def wide_text(raw: bytes) -> str:
    text = raw.decode("utf-16-le", "replace")
    terminator = text.find("\0")
    return text if terminator < 0 else text[:terminator]


def normalize_relative_path(value: str, label: str) -> str:
    require(isinstance(value, str) and value.strip() == value and value, f"{label} is empty")
    normalized = value.replace("\\", "/")
    path = PurePosixPath(normalized)
    require(
        not path.is_absolute()
        and not re.match(r"^[A-Za-z]:", normalized)
        and ".." not in path.parts
        and "." not in path.parts,
        f"{label} must be a contained relative path: {value}",
    )
    require(all(part not in ("", ".", "..") for part in path.parts), f"{label} is invalid: {value}")
    return path.as_posix()


def normalize_resource_asset_id(value: str, label: str) -> str:
    normalized = normalize_relative_path(value, label)
    require(normalized.startswith("Character/"), f"{label} must start with Character/: {value}")
    require(normalized.lower().endswith(".wmodel"), f"{label} must name a .wmodel: {value}")
    return normalized


def resolved_under(root: Path, relative: str, label: str) -> Path:
    normalized = normalize_relative_path(relative, label)
    root_path = root.resolve()
    candidate = (root_path / Path(*PurePosixPath(normalized).parts)).resolve()
    try:
        candidate.relative_to(root_path)
    except ValueError as error:
        raise PackError(f"{label} escapes {root_path}: {relative}") from error
    return candidate


def strict_descendant(root: Path, candidate: Path, label: str) -> Path:
    root_resolved = root.resolve()
    candidate_resolved = candidate.resolve()
    require(root_resolved in candidate_resolved.parents, f"{label} must be below {root_resolved}")
    return candidate_resolved


def validate_pack_output(admission_root: Path, candidate: Path) -> Path:
    admission_resolved = admission_root.resolve()
    candidate_resolved = strict_descendant(admission_resolved, candidate, "output directory")
    require(
        candidate_resolved.parent == admission_resolved
        and PACK_NAME_PATTERN.fullmatch(candidate_resolved.name) is not None,
        "output directory must be one safe-named pack directly below Admission",
    )
    return candidate_resolved


def _section_payloads(data: bytes, path: Path) -> tuple[int, int, list[tuple[int, int, str, bytes]]]:
    require(len(data) >= FILE_HEADER.size + MODEL_HEADER.size, f"WModel is truncated: {path}")
    magic, major, _minor, flags, content_size = FILE_HEADER.unpack_from(data, 0)
    require(
        magic == b"WINT"
        and major == 1
        and flags == 0
        and content_size == len(data) - FILE_HEADER.size,
        f"WModel outer header is invalid: {path}",
    )
    content = data[FILE_HEADER.size :]
    model = MODEL_HEADER.unpack_from(content, 0)
    model_magic, section_count, animation_count, model_flags, *reserved = model
    require(
        model_magic == b"WMOD"
        and section_count > 0
        and model_flags in (0, 1)
        and all(value == 0 for value in reserved),
        f"WModel metadata is invalid: {path}",
    )
    table_offset = MODEL_HEADER.size
    require(
        table_offset + section_count * SECTION_DESC.size <= len(content),
        f"WModel section table is truncated: {path}",
    )
    descriptors = [
        SECTION_DESC.unpack_from(content, table_offset + index * SECTION_DESC.size)
        for index in range(section_count)
    ]
    first_offset = int(descriptors[0][2])
    if first_offset + 4 <= len(content) and content[first_offset : first_offset + 4] == b"WINT":
        base = 0
    elif (
        first_offset + MODEL_HEADER.size + 4 <= len(content)
        and content[first_offset + MODEL_HEADER.size : first_offset + MODEL_HEADER.size + 4] == b"WINT"
    ):
        base = MODEL_HEADER.size
    else:
        raise PackError(f"cannot determine WModel section offset base: {path}")

    sections: list[tuple[int, int, str, bytes]] = []
    for row, descriptor in enumerate(descriptors):
        type_id, index, offset, size, raw_name = descriptor
        start = base + int(offset)
        end = start + int(size)
        require(
            int(size) > 0 and start >= 0 and end <= len(content),
            f"WModel section {row} is out of range: {path}",
        )
        sections.append((int(type_id), int(index), fixed_text(raw_name), content[start:end]))
    require(
        sum(1 for section in sections if section[0] == 4) == animation_count,
        f"WModel animation section count differs from metadata: {path}",
    )
    return section_count, animation_count, sections


def _nested_payload(
    blob: bytes,
    expected_magic: bytes,
    label: str,
    expected_minor: int | None = None,
) -> bytes:
    require(len(blob) >= FILE_HEADER.size + 4, f"{label} is truncated")
    magic, major, minor, flags, content_size = FILE_HEADER.unpack_from(blob, 0)
    require(
        magic == b"WINT"
        and major == 1
        and minor <= 1
        and (expected_minor is None or minor == expected_minor)
        and flags == 0
        and content_size == len(blob) - FILE_HEADER.size,
        f"{label} nested WINT header is invalid",
    )
    payload = blob[FILE_HEADER.size :]
    require(payload[:4] == expected_magic, f"{label} magic is invalid")
    return payload


@dataclass(frozen=True)
class BoneRecord:
    name_hash: int
    name: str
    parent_index: int
    matrix: tuple[float, ...]


@dataclass(frozen=True)
class ModelInfo:
    path: Path
    section_count: int
    animation_count: int
    section_types: tuple[int, ...]
    submesh_count: int
    vertex_count: int
    mesh_bones: tuple[BoneRecord, ...]
    skeleton_bones: tuple[BoneRecord, ...]
    positive_weight_indices: frozenset[int]
    texture_paths: tuple[str, ...]


def _parse_mesh(blob: bytes, label: str) -> tuple[int, int, tuple[BoneRecord, ...], frozenset[int]]:
    # The representative batch deliberately contains the existing legacy
    # runtime WModels only. Mirror CWMeshReader's legacy WMSH admission rules
    # instead of treating bone_count/stride as a sufficient skinned marker.
    payload = _nested_payload(blob, b"WMSH", label, expected_minor=0)
    require(len(payload) >= MESH_HEADER.size, f"{label} WMSH header is truncated")
    header = MESH_HEADER.unpack_from(payload, 0)
    (
        _magic,
        submesh_count,
        bone_count,
        vertex_format_flags,
        vertex_stride,
        vertex_count,
        index_count,
        index_stride,
        has_bounding,
        _reserved,
    ) = header
    skinned = bool(vertex_format_flags & VF_BONE_WEIGHT)
    require(
        0 < submesh_count <= MAX_SUBMESHES
        and bone_count <= MAX_BONES
        and vertex_count <= MAX_VERTICES
        and index_stride in (2, 4)
        and has_bounding in (0, 1)
        and (vertex_format_flags & VF_STATIC_BASE) == VF_STATIC_BASE
        and not (vertex_format_flags & ~(VF_STATIC_BASE | VF_BONE_WEIGHT))
        and (
            (skinned and bone_count > 0 and vertex_stride == STRIDE_SKINNED)
            or (not skinned and vertex_stride == STRIDE_STATIC)
        ),
        f"{label} legacy flags, stride, bounds, or bone metadata is invalid",
    )
    table_end = MESH_HEADER.size + submesh_count * SUBMESH_DESC.size
    vertex_bytes = vertex_count * vertex_stride
    index_bytes = index_count * index_stride
    bone_offset = table_end + vertex_bytes + index_bytes
    bounds_bytes = has_bounding * submesh_count * MESH_BOUNDS_V1.size
    require(
        table_end <= len(payload)
        and bone_offset + bone_count * MESH_BONE.size + bounds_bytes == len(payload),
        f"{label} WMSH ranges are invalid",
    )

    submeshes = [
        SUBMESH_DESC.unpack_from(payload, MESH_HEADER.size + row * SUBMESH_DESC.size)
        for row in range(submesh_count)
    ]
    vertex_blob_offset = table_end
    index_blob_offset = table_end + vertex_bytes
    for row, submesh in enumerate(submeshes):
        submesh_vertex_offset = int(submesh[0])
        submesh_vertex_count = int(submesh[1])
        submesh_index_offset = int(submesh[2])
        submesh_index_count = int(submesh[3])
        material_index = int(submesh[4])
        submesh_vertex_bytes = submesh_vertex_count * vertex_stride
        submesh_index_bytes = submesh_index_count * index_stride
        require(
            material_index < MAX_MATERIALS
            and submesh_vertex_offset <= vertex_bytes
            and submesh_vertex_bytes <= vertex_bytes - submesh_vertex_offset
            and submesh_index_offset <= index_bytes
            and submesh_index_bytes <= index_bytes - submesh_index_offset,
            f"{label} submesh {row} points outside its vertex/index block",
        )
        for index_row in range(submesh_index_count):
            index_at = index_blob_offset + submesh_index_offset + index_row * index_stride
            mesh_index = (
                struct.unpack_from("<H", payload, index_at)[0]
                if index_stride == 2
                else struct.unpack_from("<I", payload, index_at)[0]
            )
            require(mesh_index < submesh_vertex_count, f"{label} submesh {row} has an out-of-range index")

    positive_indices: set[int] = set()
    for vertex_index in range(vertex_count):
        row = vertex_blob_offset + vertex_index * vertex_stride
        common_channels = struct.unpack_from("<3f3f2f3f", payload, row)
        require(all(math.isfinite(value) for value in common_channels), f"{label} has a non-finite vertex channel")
        if skinned:
            indices = struct.unpack_from("<4I", payload, row + 44)
            weights = struct.unpack_from("<4f", payload, row + 60)
            require(
                all(index < bone_count for index in indices)
                and all(math.isfinite(weight) and weight >= 0.0 for weight in weights),
                f"{label} has invalid blend indices or weights",
            )
            for bone_index, weight in zip(indices, weights):
                if weight > 1e-6:
                    positive_indices.add(int(bone_index))

    bones: list[BoneRecord] = []
    for bone_index in range(bone_count):
        row = MESH_BONE.unpack_from(payload, bone_offset + bone_index * MESH_BONE.size)
        matrix = tuple(float(value) for value in row[3:19])
        require(all(math.isfinite(value) for value in matrix), f"{label} has a non-finite inverse-bind matrix")
        bones.append(
            BoneRecord(
                int(row[0]),
                fixed_text(row[1]),
                int(row[2]),
                matrix,
            )
        )
    return int(submesh_count), int(vertex_count), tuple(bones), frozenset(positive_indices)


def _parse_skeleton(blob: bytes, label: str) -> tuple[BoneRecord, ...]:
    payload = _nested_payload(blob, b"WSKL", label)
    require(len(payload) >= SKELETON_HEADER.size, f"{label} WSKL header is truncated")
    header = SKELETON_HEADER.unpack_from(payload, 0)
    bone_count = int(header[1])
    offset = SKELETON_HEADER.size
    require(offset + bone_count * SKELETON_BONE.size <= len(payload), f"{label} WSKL bones are truncated")
    bones: list[BoneRecord] = []
    for bone_index in range(bone_count):
        row = SKELETON_BONE.unpack_from(payload, offset + bone_index * SKELETON_BONE.size)
        bones.append(
            BoneRecord(
                int(row[0]),
                fixed_text(row[1]),
                int(row[2]),
                tuple(float(value) for value in row[3:19]),
            )
        )
    return tuple(bones)


def _parse_material_paths(blob: bytes, label: str) -> tuple[str, ...]:
    require(len(blob) >= FILE_HEADER.size + MATERIAL_META.size, f"{label} material section is truncated")
    magic, major, _minor, flags, content_size = FILE_HEADER.unpack_from(blob, 0)
    require(
        magic == b"WINT"
        and major == 1
        and flags == 0
        and content_size == len(blob) - FILE_HEADER.size,
        f"{label} material WINT header is invalid",
    )
    payload = blob[FILE_HEADER.size :]
    material_magic, material_count = MATERIAL_META.unpack_from(payload, 0)
    if material_magic == b"WMAT":
        entry_struct = MATERIAL_ENTRY_V1
        path_field_end = 4
    elif material_magic == b"WMA2":
        entry_struct = MATERIAL_ENTRY_V2
        path_field_end = 12
    elif material_magic == b"WMA3":
        entry_struct = MATERIAL_ENTRY_V3
        path_field_end = 13
    else:
        raise PackError(f"{label} has unsupported material magic {material_magic!r}")
    offset = MATERIAL_META.size
    require(
        offset + material_count * entry_struct.size <= len(payload),
        f"{label} material entries are truncated",
    )
    paths: set[str] = set()
    for material_index in range(material_count):
        fields = entry_struct.unpack_from(payload, offset + material_index * entry_struct.size)
        require(int(fields[0]) < max(material_count, 1), f"{label} material index is out of range")
        for raw_path in fields[3:path_field_end]:
            path = wide_text(raw_path)
            if not path:
                continue
            paths.add(normalize_relative_path(path, f"{label} material texture path"))
    return tuple(sorted(paths, key=str.casefold))


def read_wmodel(path: Path) -> ModelInfo:
    data = path.read_bytes()
    section_count, animation_count, sections = _section_payloads(data, path)
    mesh_sections = [section for section in sections if section[0] == 1]
    material_sections = [section for section in sections if section[0] == 2]
    skeleton_sections = [section for section in sections if section[0] == 3]
    require(len(mesh_sections) == 1, f"WModel requires one mesh section: {path}")
    require(len(material_sections) == 1, f"WModel requires one material section: {path}")
    require(len(skeleton_sections) <= 1, f"WModel has multiple skeleton sections: {path}")
    submesh_count, vertex_count, mesh_bones, positive_indices = _parse_mesh(
        mesh_sections[0][3], f"{path} mesh"
    )
    skeleton_bones = (
        _parse_skeleton(skeleton_sections[0][3], f"{path} skeleton")
        if skeleton_sections
        else tuple()
    )
    if skeleton_bones:
        require(
            [(bone.name_hash, bone.name) for bone in mesh_bones]
            == [(bone.name_hash, bone.name) for bone in skeleton_bones],
            f"WModel WMSH/WSKL palette differs: {path}",
        )
    else:
        require(not mesh_bones and not positive_indices, f"static WModel carries skin data: {path}")
    texture_paths = _parse_material_paths(material_sections[0][3], f"{path} material")
    return ModelInfo(
        path,
        section_count,
        animation_count,
        tuple(section[0] for section in sections),
        submesh_count,
        vertex_count,
        mesh_bones,
        skeleton_bones,
        positive_indices,
        texture_paths,
    )


def compare_weighted_palette(part: ModelInfo, body: ModelInfo) -> dict[str, Any]:
    require(part.skeleton_bones, f"skinned part has no skeleton: {part.path}")
    require(body.skeleton_bones, f"master body has no skeleton: {body.path}")
    max_delta = 0.0
    for bone_index in sorted(part.positive_weight_indices):
        require(bone_index < len(body.mesh_bones), f"weighted part bone {bone_index} exceeds body palette")
        part_bone = part.mesh_bones[bone_index]
        body_bone = body.mesh_bones[bone_index]
        require(
            part_bone.name_hash == body_bone.name_hash and part_bone.name == body_bone.name,
            f"weighted palette bone {bone_index} differs: {part_bone.name} != {body_bone.name}",
        )
        delta = max(abs(left - right) for left, right in zip(part_bone.matrix, body_bone.matrix))
        require(delta <= 1e-6, f"weighted palette inverse bind differs at {bone_index}: {delta}")
        max_delta = max(max_delta, delta)

    paired_count = min(len(part.mesh_bones), len(body.mesh_bones))
    full_name_mismatches = sum(
        1
        for index in range(paired_count)
        if part.mesh_bones[index].name != body.mesh_bones[index].name
    ) + abs(len(part.mesh_bones) - len(body.mesh_bones))
    return {
        "weightedBoneCount": len(part.positive_weight_indices),
        "partPaletteBoneCount": len(part.mesh_bones),
        "bodyPaletteBoneCount": len(body.mesh_bones),
        "maxWeightedInverseBindDelta": max_delta,
        "fullPaletteNameMismatchCount": full_name_mismatches,
        "policy": "POSITIVE_WEIGHT_INDEX_NAME_AND_INVERSE_BIND",
    }


def _validate_stable_id(value: Any, label: str, pattern: re.Pattern[str]) -> str:
    require(isinstance(value, str) and pattern.fullmatch(value) is not None, f"{label} is invalid: {value!r}")
    return value


def derive_category_id(primary_slot: str, coverage_slots: Iterable[str]) -> str:
    coverage = set(coverage_slots)
    if primary_slot == "WEAPON":
        return "WEAPON_SET"
    if primary_slot == "UPPER" and coverage != {"UPPER"}:
        return "APPAREL_OUTFIT"
    return f"APPAREL_{primary_slot}"


def validate_selection_document(document: dict[str, Any]) -> dict[str, Any]:
    require(document.get("schema") == SELECTION_SCHEMA, "representative selection schema is invalid")
    require(document.get("formatVersion") == 1, "representative selection formatVersion is invalid")
    require(
        document.get("sourceAssociationPolicy") == SOURCE_ASSOCIATION_POLICY,
        "representative selection sourceAssociationPolicy is invalid",
    )
    normalize_relative_path(document.get("sourceInventory", ""), "sourceInventory")
    pending_candidates = document.get("pendingNormalizationCandidates")
    require(
        isinstance(pending_candidates, list) and len(pending_candidates) == len(ALLOWED_CLASSES),
        "pendingNormalizationCandidates must contain one candidate per class",
    )
    pending_ids: set[str] = set()
    pending_classes: set[str] = set()
    for candidate_index, candidate in enumerate(pending_candidates):
        require(isinstance(candidate, dict), f"pendingNormalizationCandidates[{candidate_index}] is invalid")
        candidate_id = _validate_stable_id(
            candidate.get("candidateId"),
            f"pendingNormalizationCandidates[{candidate_index}].candidateId",
            STABLE_ID_PATTERN,
        )
        require(candidate_id not in pending_ids, f"duplicate pending candidateId: {candidate_id}")
        pending_ids.add(candidate_id)
        class_id = candidate.get("classId")
        require(class_id in ALLOWED_CLASSES and class_id not in pending_classes, f"invalid pending classId: {class_id}")
        pending_classes.add(class_id)
        require(candidate.get("categoryId") == "APPAREL_SHOULDER", f"pending category differs: {candidate_id}")
        require(
            candidate.get("runtimeReadiness") == PENDING_NORMALIZATION_STATE,
            f"pending readiness differs: {candidate_id}",
        )
        require(
            isinstance(candidate.get("sourcePackageName"), str)
            and candidate["sourcePackageName"]
            and isinstance(candidate.get("sourceObjectName"), str)
            and candidate["sourceObjectName"]
            and candidate.get("expectedSourceRole") == "APPAREL_SHOULDER",
            f"pending source contract differs: {candidate_id}",
        )
        require(
            isinstance(candidate.get("rawJointCount"), int)
            and candidate["rawJointCount"] > 0
            and isinstance(candidate.get("bodyPaletteBoneCount"), int)
            and candidate["bodyPaletteBoneCount"] > 0
            and isinstance(candidate.get("additionalBlockers"), list)
            and all(isinstance(blocker, str) and blocker for blocker in candidate["additionalBlockers"]),
            f"pending measurement contract differs: {candidate_id}",
        )
    require(pending_classes == ALLOWED_CLASSES, "pending candidates do not cover all six classes")
    sets = document.get("sets")
    require(isinstance(sets, list) and sets, "representative selection sets must be non-empty")

    visual_set_ids: set[str] = set()
    target_asset_ids: set[str] = set()
    classes: set[str] = set()
    for set_index, visual_set in enumerate(sets):
        require(isinstance(visual_set, dict), f"sets[{set_index}] must be an object")
        visual_set_id = _validate_stable_id(
            visual_set.get("visualSetId"), f"sets[{set_index}].visualSetId", STABLE_ID_PATTERN
        )
        require(visual_set_id not in visual_set_ids, f"duplicate visualSetId: {visual_set_id}")
        visual_set_ids.add(visual_set_id)
        class_id = visual_set.get("classId")
        require(class_id in ALLOWED_CLASSES, f"unknown classId in {visual_set_id}: {class_id}")
        classes.add(class_id)
        _validate_stable_id(
            visual_set.get("curatedVariantId"), f"{visual_set_id}.curatedVariantId", PART_ID_PATTERN
        )
        catalog_status = visual_set.get("catalogStatus")
        require(catalog_status in ALLOWED_CATALOG_STATUSES, f"invalid catalogStatus in {visual_set_id}")
        primary_slot = visual_set.get("primarySlot")
        coverage_slots = visual_set.get("coverageSlots")
        require(primary_slot in ALLOWED_SLOTS, f"unknown primarySlot in {visual_set_id}: {primary_slot}")
        require(
            isinstance(coverage_slots, list)
            and coverage_slots
            and len(coverage_slots) == len(set(coverage_slots))
            and all(slot in ALLOWED_SLOTS for slot in coverage_slots)
            and primary_slot in coverage_slots,
            f"coverageSlots are invalid in {visual_set_id}",
        )
        require(
            (primary_slot == "WEAPON") == (catalog_status == "BASELINE_WEAPON"),
            f"catalogStatus/primarySlot differ in {visual_set_id}",
        )
        normalize_resource_asset_id(visual_set.get("masterBodyAssetId", ""), f"{visual_set_id}.masterBodyAssetId")
        require(
            visual_set.get("bodyCoveragePolicy") == "INHERIT_BASELINE",
            f"unsupported bodyCoveragePolicy in {visual_set_id}",
        )
        parts = visual_set.get("parts")
        require(isinstance(parts, list) and parts, f"parts must be non-empty in {visual_set_id}")
        part_ids: set[str] = set()
        for part_index, part in enumerate(parts):
            require(isinstance(part, dict), f"{visual_set_id}.parts[{part_index}] must be an object")
            part_id = _validate_stable_id(
                part.get("partId"), f"{visual_set_id}.parts[{part_index}].partId", PART_ID_PATTERN
            )
            require(part_id not in part_ids, f"duplicate partId in {visual_set_id}: {part_id}")
            part_ids.add(part_id)
            require(part.get("sourceClassId") in ALLOWED_CLASSES, f"unknown sourceClassId in {visual_set_id}/{part_id}")
            require(
                isinstance(part.get("sourcePackageName"), str)
                and part["sourcePackageName"]
                and isinstance(part.get("sourceObjectName"), str)
                and part["sourceObjectName"],
                f"source package/object is missing in {visual_set_id}/{part_id}",
            )
            require(
                isinstance(part.get("expectedSourceRole"), str) and part["expectedSourceRole"],
                f"expectedSourceRole is missing in {visual_set_id}/{part_id}",
            )
            normalize_resource_asset_id(
                part.get("existingModelAssetId", ""), f"{visual_set_id}/{part_id}.existingModelAssetId"
            )
            target_asset_id = normalize_resource_asset_id(
                part.get("targetModelAssetId", ""), f"{visual_set_id}/{part_id}.targetModelAssetId"
            )
            target_asset_key = target_asset_id.casefold()
            require(target_asset_key not in target_asset_ids, f"duplicate targetModelAssetId: {target_asset_id}")
            target_asset_ids.add(target_asset_key)
            attachment_mode = part.get("attachmentMode")
            model_kind = part.get("modelKind")
            require(attachment_mode in ALLOWED_ATTACHMENT_MODES, f"invalid attachmentMode in {visual_set_id}/{part_id}")
            require(model_kind in ALLOWED_MODEL_KINDS, f"invalid modelKind in {visual_set_id}/{part_id}")
            if attachment_mode == "SKINNED":
                require(model_kind == "ANIM_SKINNED", f"skinned part modelKind differs in {visual_set_id}/{part_id}")
                require("socketBone" not in part, f"skinned part declares socketBone in {visual_set_id}/{part_id}")
            else:
                require(model_kind == "NONANIM_SOCKETED", f"socketed part modelKind differs in {visual_set_id}/{part_id}")
                require(
                    isinstance(part.get("socketBone"), str) and part["socketBone"],
                    f"socketBone is missing in {visual_set_id}/{part_id}",
                )
                yaw = part.get("socketYawDegrees")
                require(isinstance(yaw, (int, float)) and math.isfinite(float(yaw)), f"socket yaw is invalid in {visual_set_id}/{part_id}")
                require(isinstance(part.get("visibleStances"), list), f"visibleStances is invalid in {visual_set_id}/{part_id}")
    require(classes == ALLOWED_CLASSES, f"representative selection must cover six classes, got {sorted(classes)}")
    return document


def inventory_index(document: dict[str, Any]) -> dict[tuple[str, str, str], list[dict[str, Any]]]:
    require(document.get("schema") == INVENTORY_SCHEMA, "source inventory schema is invalid")
    require(document.get("formatVersion") == 1, "source inventory formatVersion is invalid")
    entries = document.get("entries")
    require(isinstance(entries, list) and entries, "source inventory entries are missing")
    result: dict[tuple[str, str, str], list[dict[str, Any]]] = {}
    for row, entry in enumerate(entries):
        require(isinstance(entry, dict), f"source inventory entry {row} is invalid")
        key = (entry.get("classId"), entry.get("packageName"), entry.get("objectName"))
        require(all(isinstance(value, str) and value for value in key), f"source inventory key is invalid at row {row}")
        result.setdefault(key, []).append(entry)
    return result


def resolve_inventory_entry(
    index: dict[tuple[str, str, str], list[dict[str, Any]]],
    key: tuple[str, str, str],
) -> dict[str, Any]:
    candidates = index.get(key) or []
    require(candidates, f"selected source is absent from inventory: {key}")
    if len(candidates) == 1:
        return candidates[0]

    # UModel dependency export can place the same object once at package root
    # and once under mesh/.  The authored key already names the exact object;
    # prefer the canonical mesh/ row, but never guess between two mesh rows.
    mesh_candidates = [
        candidate
        for candidate in candidates
        if "/mesh/" in str(candidate.get("sourcePath", "")).replace("\\", "/").lower()
    ]
    require(
        len(mesh_candidates) == 1,
        f"selected source key is ambiguous after mesh/ preference: {key}",
    )
    selected = mesh_candidates[0]
    semantic_fields = (
        "classId",
        "visualSetId",
        "packageName",
        "objectName",
        "partRole",
        "extractionState",
    )
    require(
        all(
            all(candidate.get(field) == selected.get(field) for field in semantic_fields)
            for candidate in candidates
        ),
        f"duplicate source rows disagree semantically: {key}",
    )
    return selected


def _copy_file_once(source: Path, destination: Path) -> dict[str, Any]:
    require(source.is_file(), f"required file is missing: {source}")
    source_size = source.stat().st_size
    source_hash = sha256_file(source)
    if destination.exists():
        require(destination.is_file(), f"pack destination is not a file: {destination}")
    else:
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, destination)
    destination_size = destination.stat().st_size
    destination_hash = sha256_file(destination)
    require(
        destination_size == source_size and destination_hash == source_hash,
        f"pack copy differs from captured source bytes: {destination}",
    )
    return {"byteSize": destination_size, "sha256": destination_hash}


def _copy_gltf_source(repo_root: Path, work_root: Path, source_path_id: str, visual_set_id: str, part_id: str) -> dict[str, Any]:
    source_path = resolved_under(repo_root, source_path_id, "source inventory path")
    require(source_path.suffix.lower() == ".gltf", f"source inventory path is not glTF: {source_path_id}")
    document = load_json(source_path)
    destination_root = work_root / "Source" / visual_set_id / part_id
    copied: list[dict[str, Any]] = []

    def copy_dependency(path: Path, relative_name: str, kind: str) -> None:
        normalized = normalize_relative_path(relative_name, f"source glTF {kind} URI")
        destination = resolved_under(destination_root, normalized, f"source pack {kind} URI")
        evidence = _copy_file_once(path, destination)
        copied.append(
            {
                "kind": kind,
                "path": (PurePosixPath("Source") / visual_set_id / part_id / normalized).as_posix(),
                **evidence,
            }
        )

    copy_dependency(source_path, source_path.name, "GLTF")
    for collection_name, kind in (("buffers", "BUFFER"), ("images", "IMAGE")):
        for entry in document.get(collection_name) or []:
            uri = entry.get("uri")
            if not isinstance(uri, str) or not uri or uri.startswith("data:"):
                continue
            normalized_uri = normalize_relative_path(uri, f"source glTF {kind} URI")
            dependency = resolved_under(source_path.parent, normalized_uri, f"source glTF {kind}")
            copy_dependency(dependency, normalized_uri, kind)
    return {
        "inventorySourcePath": source_path_id,
        "files": copied,
    }


def _copy_runtime_closure(
    resources_root: Path,
    work_root: Path,
    existing_asset_id: str,
    target_asset_id: str,
    model: ModelInfo,
) -> list[dict[str, Any]]:
    existing_path = resolved_under(resources_root, existing_asset_id, "existing model asset ID")
    target_path = resolved_under(work_root / "Runtime", target_asset_id, "target model asset ID")
    model_evidence = _copy_file_once(existing_path, target_path)
    closure = [
        {
            "kind": "WMODEL",
            "assetId": target_asset_id,
            "sourceAssetId": existing_asset_id,
            **model_evidence,
        }
    ]
    for texture_path in model.texture_paths:
        source_texture = resolved_under(existing_path.parent, texture_path, "existing WModel texture path")
        destination_texture = resolved_under(target_path.parent, texture_path, "target WModel texture path")
        evidence = _copy_file_once(source_texture, destination_texture)
        target_texture_id = destination_texture.relative_to(work_root / "Runtime").as_posix()
        source_texture_id = source_texture.relative_to(resources_root).as_posix()
        closure.append(
            {
                "kind": "TEXTURE",
                "assetId": target_texture_id,
                "sourceAssetId": source_texture_id,
                **evidence,
            }
        )
    return closure


def _render_markdown(receipt: dict[str, Any]) -> str:
    lines = [
        "# Character equipment representative admission pack",
        "",
        f"- state: `{receipt['state']}`",
        f"- source association: `{receipt['sourceAssociation']['state']}`",
        f"- classes: {receipt['counts']['classCount']}",
        f"- visual sets: {receipt['counts']['visualSetCount']}",
        f"- parts: {receipt['counts']['partCount']}",
        f"- runtime closure files: {receipt['counts']['runtimeClosureFileCount']}",
        f"- pending normalization candidates: {receipt['counts']['pendingNormalizationCandidateCount']}",
        "",
        "이 폴더는 검증된 staging pack이다. `Client/Bin/Resources`와 제품 catalog는 변경하지 않았다.",
        "raw object와 existing WModel은 독립 검증 뒤 authored association으로 묶였으며, "
        "exact build provenance를 뜻하지 않는다.",
        "",
        "## 클래스별 교체 카테고리",
        "",
        "| class | cooked apparel | ready categories | weapon sets | pending apparel |",
        "|---|---|---|---:|---|",
    ]
    for class_entry in receipt["classCatalog"]:
        categories = ", ".join(class_entry["readyCategoryIds"]) or "-"
        pending = ", ".join(class_entry["pendingApparelCandidateIds"]) or "-"
        lines.append(
            f"| {class_entry['classId']} | {class_entry['cookedApparelState']} | {categories} | "
            f"{len(class_entry['weaponVisualSetIds'])} | {pending} |"
        )
    lines.extend(
        [
        "",
        "## 준비된 visual set",
        "",
        "| class | category | status | visualSetId | primary slot | parts | staged model IDs |",
        "|---|---|---|---|---|---:|---|",
        ]
    )
    for visual_set in receipt["sets"]:
        model_ids = "<br>".join(part["targetModelAssetId"] for part in visual_set["parts"])
        lines.append(
            f"| {visual_set['classId']} | {visual_set['categoryId']} | {visual_set['catalogStatus']} | "
            f"`{visual_set['visualSetId']}` | "
            f"{visual_set['primarySlot']} | {len(visual_set['parts'])} | {model_ids} |"
        )
    lines.extend(
        [
            "",
            "## 정규화 대기 대표 의상",
            "",
            "| class | category | candidate | raw/body bones | blockers |",
            "|---|---|---|---:|---|",
        ]
    )
    for candidate in receipt["pendingNormalizationCandidates"]:
        blockers = ", ".join(candidate["additionalBlockers"]) or "master-skeleton normalization"
        lines.append(
            f"| {candidate['classId']} | {candidate['categoryId']} | `{candidate['candidateId']}` | "
            f"{candidate['rawJointCount']}/{candidate['bodyPaletteBoneCount']} | {blockers} |"
        )
    lines.extend(
        [
            "",
            "## 다음 경계",
            "",
            "이 receipt의 stable visualSetId와 named part만 `EquipmentPresentationCatalog.json`으로 승격한다. "
            "source path, hash, Prototype tag, vector index는 회원/캐릭터 저장값이 아니다.",
            "",
        ]
    )
    return "\n".join(lines)


def build_pack(repo_root: Path, selection_path: Path, output_directory: Path, overwrite: bool) -> dict[str, Any]:
    repo_root = repo_root.resolve()
    require(repo_root.is_dir(), f"repository root does not exist: {repo_root}")
    selection_path = strict_descendant(repo_root, selection_path, "selection file")
    admission_root = (repo_root / "out" / "CharacterEquipmentExtraction" / "Admission").resolve()
    output_directory = validate_pack_output(admission_root, output_directory)
    require(selection_path.is_file(), f"selection file does not exist: {selection_path}")

    selection = validate_selection_document(load_json(selection_path))
    inventory_path = resolved_under(repo_root, selection["sourceInventory"], "sourceInventory")
    inventory_document = load_json(inventory_path)
    inventory = inventory_index(inventory_document)
    resources_root = (repo_root / "Client" / "Bin" / "Resources").resolve()
    require(resources_root.is_dir(), f"Resources root does not exist: {resources_root}")
    require(overwrite or not output_directory.exists(), f"output directory already exists: {output_directory}")
    parent = output_directory.parent
    parent.mkdir(parents=True, exist_ok=True)
    work_root = parent / f".{output_directory.name}.work-{uuid.uuid4().hex}"
    require(not work_root.exists(), f"work directory already exists: {work_root}")
    work_root.mkdir(parents=True)

    model_cache: dict[Path, ModelInfo] = {}

    def get_model(asset_id: str) -> ModelInfo:
        path = resolved_under(resources_root, asset_id, "model asset ID")
        require(path.is_file(), f"model asset is missing: {asset_id}")
        if path not in model_cache:
            model_cache[path] = read_wmodel(path)
        return model_cache[path]

    pending_receipts: list[dict[str, Any]] = []
    for candidate in selection["pendingNormalizationCandidates"]:
        inventory_key = (
            candidate["classId"],
            candidate["sourcePackageName"],
            candidate["sourceObjectName"],
        )
        source_entry = resolve_inventory_entry(inventory, inventory_key)
        require(
            source_entry.get("extractionState") == "EXTRACTED"
            and source_entry.get("partRole") == candidate["expectedSourceRole"]
            and source_entry.get("runtimeReadiness") == candidate["runtimeReadiness"],
            f"pending source state/role/readiness differs for {inventory_key}",
        )
        source_evidence = _copy_gltf_source(
            repo_root,
            work_root,
            source_entry["sourcePath"],
            candidate["candidateId"],
            "shoulder",
        )
        pending_receipts.append(
            {
                "candidateId": candidate["candidateId"],
                "classId": candidate["classId"],
                "categoryId": candidate["categoryId"],
                "state": "RAW_EXTRACTED_NOT_EQUIP_READY",
                "runtimeReadiness": candidate["runtimeReadiness"],
                "rawJointCount": candidate["rawJointCount"],
                "bodyPaletteBoneCount": candidate["bodyPaletteBoneCount"],
                "additionalBlockers": list(candidate["additionalBlockers"]),
                "source": {
                    "sourceFamilyId": source_entry["visualSetId"],
                    "classId": source_entry["classId"],
                    "packageName": source_entry["packageName"],
                    "objectName": source_entry["objectName"],
                    "partRole": source_entry["partRole"],
                    **source_evidence,
                },
            }
        )

    receipt_sets: list[dict[str, Any]] = []
    all_runtime_files: dict[str, dict[str, Any]] = {}
    part_count = 0
    try:
        for visual_set in selection["sets"]:
            body = get_model(visual_set["masterBodyAssetId"])
            require(body.skeleton_bones, f"master body has no skeleton: {visual_set['masterBodyAssetId']}")
            body_bone_names = {bone.name for bone in body.skeleton_bones}
            receipt_parts: list[dict[str, Any]] = []
            for part in visual_set["parts"]:
                part_count += 1
                inventory_key = (
                    part["sourceClassId"],
                    part["sourcePackageName"],
                    part["sourceObjectName"],
                )
                source_entry = resolve_inventory_entry(inventory, inventory_key)
                require(
                    source_entry.get("extractionState") == "EXTRACTED"
                    and source_entry.get("partRole") == part["expectedSourceRole"],
                    f"selected source state/role differs for {inventory_key}",
                )
                source_evidence = _copy_gltf_source(
                    repo_root,
                    work_root,
                    source_entry["sourcePath"],
                    visual_set["visualSetId"],
                    part["partId"],
                )

                existing_asset_id = normalize_resource_asset_id(
                    part["existingModelAssetId"], "existingModelAssetId"
                )
                target_asset_id = normalize_resource_asset_id(
                    part["targetModelAssetId"], "targetModelAssetId"
                )
                model = get_model(existing_asset_id)
                admission: dict[str, Any]
                if part["attachmentMode"] == "SKINNED":
                    require(model.skeleton_bones, f"skinned part has no skeleton: {existing_asset_id}")
                    require(model.animation_count == 0, f"skinned equipment part carries animations: {existing_asset_id}")
                    admission = {
                        "modelContract": "ANIM_SKINNED",
                        "submeshCount": model.submesh_count,
                        "vertexCount": model.vertex_count,
                        "palette": compare_weighted_palette(model, body),
                    }
                else:
                    require(not model.skeleton_bones, f"socketed weapon unexpectedly has a skeleton: {existing_asset_id}")
                    require(model.animation_count == 0, f"socketed weapon carries animations: {existing_asset_id}")
                    socket_bone = part["socketBone"]
                    require(
                        socket_bone in body_bone_names,
                        f"socket bone {socket_bone} is absent from {visual_set['masterBodyAssetId']}",
                    )
                    admission = {
                        "modelContract": "NONANIM_SOCKETED",
                        "submeshCount": model.submesh_count,
                        "vertexCount": model.vertex_count,
                        "socketBone": socket_bone,
                        "socketYawDegrees": float(part["socketYawDegrees"]),
                        "visibleStances": list(part["visibleStances"]),
                    }

                closure = _copy_runtime_closure(
                    resources_root,
                    work_root,
                    existing_asset_id,
                    target_asset_id,
                    model,
                )
                for file_row in closure:
                    runtime_asset_key = file_row["assetId"].casefold()
                    previous = all_runtime_files.get(runtime_asset_key)
                    if previous is None:
                        all_runtime_files[runtime_asset_key] = file_row
                    else:
                        require(
                            previous["assetId"] == file_row["assetId"]
                            and previous["sha256"] == file_row["sha256"],
                            f"runtime closure asset collision: {file_row['assetId']}",
                        )
                receipt_parts.append(
                    {
                        "partId": part["partId"],
                        "partRole": part["partRole"],
                        "attachmentMode": part["attachmentMode"],
                        "modelKind": part["modelKind"],
                        "source": {
                            "sourceFamilyId": source_entry["visualSetId"],
                            "classId": source_entry["classId"],
                            "packageName": source_entry["packageName"],
                            "objectName": source_entry["objectName"],
                            "partRole": source_entry["partRole"],
                            **source_evidence,
                        },
                        "existingModelAssetId": existing_asset_id,
                        "targetModelAssetId": target_asset_id,
                        "resourceClosure": closure,
                        "admission": {"state": "VALIDATED", **admission},
                    }
                )
            receipt_sets.append(
                {
                    "visualSetId": visual_set["visualSetId"],
                    "classId": visual_set["classId"],
                    "curatedVariantId": visual_set["curatedVariantId"],
                    "categoryId": derive_category_id(
                        visual_set["primarySlot"], visual_set["coverageSlots"]
                    ),
                    "catalogStatus": visual_set["catalogStatus"],
                    "primarySlot": visual_set["primarySlot"],
                    "coverageSlots": list(visual_set["coverageSlots"]),
                    "masterBodyAssetId": visual_set["masterBodyAssetId"],
                    "bodyCoveragePolicy": visual_set["bodyCoveragePolicy"],
                    "parts": receipt_parts,
                    "admission": {"state": "VALIDATED_STAGING"},
                }
            )

        class_catalog: list[dict[str, Any]] = []
        for class_id in CLASS_ORDER:
            class_sets = [entry for entry in receipt_sets if entry["classId"] == class_id]
            apparel_sets = [entry for entry in class_sets if entry["categoryId"] != "WEAPON_SET"]
            weapon_sets = [entry for entry in class_sets if entry["categoryId"] == "WEAPON_SET"]
            if any(entry["catalogStatus"] == "READY_ALTERNATIVE" for entry in apparel_sets):
                cooked_apparel_state = "READY_ALTERNATIVE"
            elif any(entry["catalogStatus"] == "BASELINE_PART" for entry in apparel_sets):
                cooked_apparel_state = "BASELINE_PART_ONLY"
            else:
                cooked_apparel_state = "NO_DETACHABLE_COOKED_APPAREL"
            pending_for_class = [
                candidate for candidate in pending_receipts if candidate["classId"] == class_id
            ]
            class_catalog.append(
                {
                    "classId": class_id,
                    "cookedApparelState": cooked_apparel_state,
                    "readyCategoryIds": sorted(
                        {entry["categoryId"] for entry in apparel_sets}, key=str.casefold
                    ),
                    "readyVisualSetIds": [entry["visualSetId"] for entry in apparel_sets],
                    "weaponVisualSetIds": [entry["visualSetId"] for entry in weapon_sets],
                    "pendingApparelCandidateIds": [
                        candidate["candidateId"] for candidate in pending_for_class
                    ],
                }
            )

        receipt: dict[str, Any] = {
            "schema": RECEIPT_SCHEMA,
            "formatVersion": 1,
            "generatedAtUtc": dt.datetime.now(dt.timezone.utc).isoformat(),
            "state": "STAGED_VALIDATED_NOT_PROMOTED",
            "sourceAssociation": {
                "state": "CURATED_NOT_BUILD_PROVENANCE",
                "policy": selection["sourceAssociationPolicy"],
                "meaning": (
                    "The selected raw object and existing cooked WModel are independently validated and "
                    "authored together; no historical cook receipt or geometry derivation proves that the "
                    "WModel was built from that exact raw object."
                ),
            },
            "selection": {
                "path": selection_path.relative_to(repo_root).as_posix(),
                "sha256": sha256_file(selection_path),
            },
            "sourceInventory": {
                "path": inventory_path.relative_to(repo_root).as_posix(),
                "sha256": sha256_file(inventory_path),
                "sourceObjectCount": inventory_document.get("sourceObjectCount"),
                "runtimeAdmittedCount": inventory_document.get("runtimeAdmittedCount"),
            },
            "counts": {
                "classCount": len({entry["classId"] for entry in receipt_sets}),
                "visualSetCount": len(receipt_sets),
                "partCount": part_count,
                "runtimeClosureFileCount": len(all_runtime_files),
                "pendingNormalizationCandidateCount": len(pending_receipts),
            },
            "classCatalog": class_catalog,
            "sets": receipt_sets,
            "pendingNormalizationCandidates": pending_receipts,
            "runtimeClosure": sorted(all_runtime_files.values(), key=lambda value: value["assetId"].casefold()),
            "promotion": {
                "resourcesMutated": False,
                "dataCatalogMutated": False,
                "nextContract": "EquipmentPresentationCatalog visualSets[] named-part promotion",
            },
        }
        write_json(work_root / "character-equipment-runtime-admission.json", receipt)
        (work_root / "character-equipment-representative-pack.md").write_text(
            _render_markdown(receipt), encoding="utf-8", newline="\n"
        )

        backup: Path | None = None
        if output_directory.exists():
            timestamp = dt.datetime.now().strftime("%Y%m%d-%H%M%S")
            backup = parent / f".{output_directory.name}.backup-{timestamp}"
            require(not backup.exists(), f"backup path already exists: {backup}")
            os.replace(output_directory, backup)
        try:
            os.replace(work_root, output_directory)
        except BaseException:
            if backup is not None and not output_directory.exists():
                os.replace(backup, output_directory)
            raise
        return receipt
    except BaseException:
        # Keep the failed work directory for diagnosis. It is intentionally
        # outside Resources and cannot become an admitted runtime pack.
        raise


def parse_arguments(argv: Iterable[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root", type=Path, required=True)
    parser.add_argument("--selection", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--overwrite", action="store_true")
    return parser.parse_args(list(argv))


def main(argv: Iterable[str] | None = None) -> int:
    arguments = parse_arguments(sys.argv[1:] if argv is None else argv)
    try:
        receipt = build_pack(
            arguments.repo_root,
            arguments.selection,
            arguments.output,
            arguments.overwrite,
        )
    except PackError as error:
        print(f"Character equipment representative pack FAILED: {error}", file=sys.stderr)
        return 1
    print(
        "Character equipment representative pack succeeded: "
        f"classes={receipt['counts']['classCount']} "
        f"sets={receipt['counts']['visualSetCount']} "
        f"parts={receipt['counts']['partCount']} "
        f"closure={receipt['counts']['runtimeClosureFileCount']} "
        f"state={receipt['state']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
