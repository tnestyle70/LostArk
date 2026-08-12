#!/usr/bin/env python3
"""Build the fail-closed Artist 31470 main transform oracle.

The oracle is deliberately numeric and narrow.  It joins the action cue, the
renderer-coverage receipt, the reconstructed runtime program, the geometry
resource-binding receipt, and the two installed main WModels used by active
occurrences 009-011.  It seals basis conjugation, one-and-only-one geometry
pre-scale, particle StartSize/MeshRotation, cue-local transform, a fixed root
fixture, transformed bounds, and fixed view/projection clip points.

It does not claim raw UPK-to-glTF pivot provenance, playable-model yaw
equivalence, runtime execution, visual approval, or Product admission.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import struct
import sys
from pathlib import Path
from typing import Any, Iterable, Sequence


REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = Path(__file__).resolve()
MODEL_TOOL_DIR = REPO_ROOT / "Tools/ModelAssetConverter"
if str(MODEL_TOOL_DIR) not in sys.path:
    sys.path.insert(0, str(MODEL_TOOL_DIR))

from cook_wmodel_geometry_contract import parse_geometry_wmodel  # noqa: E402


SCHEMA = "lostark.artist-31470-main-transform-oracle-receipt"
FORMAT_VERSION = 1
EXPECTED_OCCURRENCES = (
    "source-active-009",
    "source-active-010",
    "source-active-011",
)
EXPECTED_GEOMETRY = {
    "source-active-009": "Effect/Artist/Meshes/fm_h_swing_03.wmodel",
    "source-active-010": "Effect/Artist/Meshes/fm_h_swing_03.wmodel",
    "source-active-011": "Effect/Artist/Meshes/fm_h_swing_05.wmodel",
}
MAIN_CUE_ID = "skill-31470/clip-000/notify-018"
MAIN_SOURCE_OCCURRENCE_ID = "action-31470/stage-000/notify-018"
MAIN_SOURCE_SYSTEM_ID = "fx_pc_sdm_07.par_v_smd_onestroke_swing_01"
MAIN_NOTIFY_SECONDS = 1.3803969621658325
GEOMETRY_PRE_SCALE_DECIMAL = 0.01
GEOMETRY_PRE_SCALE_F32 = struct.unpack("<f", struct.pack("<f", 0.01))[0]
GEOMETRY_PRE_SCALE_F32_HEX = struct.pack(">f", GEOMETRY_PRE_SCALE_F32).hex()

# Row-vector mapping: [x, y, z] UE * B == [x, z, -y] Client.
BASIS: tuple[tuple[float, ...], ...] = (
    (1.0, 0.0, 0.0),
    (0.0, 0.0, -1.0),
    (0.0, 1.0, 0.0),
)
BASIS_CANARY_SOURCE_EULER_DEGREES = (17.0, -31.0, 43.0)
BASIS_CANARY_SOURCE_POINT = (0.37, -0.61, 1.19)
ROOT_SCALE = (1.0, 1.0, 1.0)
ROOT_ROTATION_DEGREES = (9.0, -23.0, 14.0)
ROOT_POSITION = (4.25, 1.5, 10.75)
PLAYABLE_PRESENTATION_YAW_DEGREES = -90.0
CAMERA_EYE = (4.0, 6.0, -7.0)
CAMERA_FOCUS = (4.0, 1.0, 10.0)
CAMERA_UP = (0.0, 1.0, 0.0)
CAMERA_FOV_Y_DEGREES = 60.0
CAMERA_ASPECT = 16.0 / 9.0
CAMERA_NEAR = 0.1
CAMERA_FAR = 100.0

DEFAULT_ACTION_CUE = REPO_ROOT / (
    "Data/Effects/Imported/Artist/skill.31470.action-cue-recipe.json"
)
DEFAULT_RENDERER_MATRIX = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.renderer-restoration-matrix.receipt.json"
)
DEFAULT_RUNTIME_PROGRAM = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.reconstructed-runtime-program.candidate.json"
)
DEFAULT_GEOMETRY_BINDING = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Geometry/"
    "skill.31470.geometry-resource-binding.receipt.json"
)
DEFAULT_RESOURCE_ROOT = REPO_ROOT / "Client/Bin/Resources"
DEFAULT_OUTPUT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Geometry/"
    "skill.31470.main-transform-oracle.receipt.json"
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def _reject_duplicate_json_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        require(key not in result, f"duplicate JSON key: {key}")
        result[key] = value
    return result


def _reject_nonfinite_json_constant(value: str) -> None:
    raise ValueError(f"non-finite JSON constant: {value}")


def read_json_strict(path: Path) -> dict[str, Any]:
    value = json.loads(
        path.read_text(encoding="utf-8-sig"),
        object_pairs_hook=_reject_duplicate_json_keys,
        parse_constant=_reject_nonfinite_json_constant,
    )
    require(isinstance(value, dict), f"expected JSON object: {path}")
    return value


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_bytes(value)).hexdigest()


def digest_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def tracked_text_sha256(path: Path) -> str:
    raw = path.read_bytes()
    require(not raw.startswith(b"\xef\xbb\xbf"), f"BOM is forbidden: {path}")
    normalized = raw.replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    return hashlib.sha256(normalized).hexdigest()


def validate_self_digest(value: dict[str, Any], field: str, label: str) -> None:
    claimed = value.get(field)
    require(isinstance(claimed, str) and len(claimed) == 64, f"{label} seal missing")
    unsigned = copy.deepcopy(value)
    unsigned.pop(field)
    require(canonical_sha256(unsigned) == claimed, f"{label} seal mismatch")


def seal_row(row: dict[str, Any]) -> None:
    require("rowSha256" not in row, "row is already sealed")
    row["rowSha256"] = canonical_sha256(row)


def validate_row_seal(row: dict[str, Any], label: str) -> None:
    validate_self_digest(row, "rowSha256", label)


def seal_receipt(receipt: dict[str, Any]) -> None:
    require("receiptSha256" not in receipt, "receipt is already sealed")
    receipt["receiptSha256"] = canonical_sha256(receipt)


def repo_relative(path: Path) -> str:
    try:
        return path.relative_to(REPO_ROOT).as_posix()
    except ValueError:
        return path.as_posix()


def input_identity(
    path: Path,
    value: dict[str, Any],
    *,
    seal_field: str | None = None,
) -> dict[str, Any]:
    result = {
        "path": repo_relative(path),
        "rawSha256": digest_file(path),
        "canonicalJsonSha256": canonical_sha256(value),
    }
    if seal_field is not None:
        result[seal_field] = value[seal_field]
    return result


def finite_vector(value: Sequence[float], size: int, label: str) -> tuple[float, ...]:
    require(len(value) == size, f"{label} component count changed")
    result = tuple(float(component) for component in value)
    require(all(math.isfinite(component) for component in result), f"{label} is non-finite")
    return result


def vectors_close(
    left: Sequence[float],
    right: Sequence[float],
    tolerance: float = 1.0e-9,
) -> bool:
    return len(left) == len(right) and all(
        math.isclose(float(left[index]), float(right[index]), rel_tol=tolerance, abs_tol=tolerance)
        for index in range(len(left))
    )


def mat3_transpose(matrix: Sequence[Sequence[float]]) -> tuple[tuple[float, ...], ...]:
    require(len(matrix) == 3 and all(len(row) == 3 for row in matrix), "mat3 shape changed")
    return tuple(tuple(float(matrix[column][row]) for column in range(3)) for row in range(3))


def mat3_mul(
    left: Sequence[Sequence[float]], right: Sequence[Sequence[float]]
) -> tuple[tuple[float, ...], ...]:
    return tuple(
        tuple(
            sum(float(left[row][inner]) * float(right[inner][column]) for inner in range(3))
            for column in range(3)
        )
        for row in range(3)
    )


def row_point3_mul(
    point: Sequence[float], matrix: Sequence[Sequence[float]]
) -> tuple[float, float, float]:
    value = finite_vector(point, 3, "row point")
    return tuple(
        sum(value[inner] * float(matrix[inner][column]) for inner in range(3))
        for column in range(3)
    )  # type: ignore[return-value]


def directx_row_rotation_degrees(
    rotation_degrees: Sequence[float],
) -> tuple[tuple[float, ...], ...]:
    """Equivalent to XMMatrixRotationRollPitchYaw for row-vector matrices."""

    pitch_degrees, yaw_degrees, roll_degrees = finite_vector(
        rotation_degrees, 3, "rotation degrees"
    )
    pitch = math.radians(pitch_degrees) * 0.5
    yaw = math.radians(yaw_degrees) * 0.5
    roll = math.radians(roll_degrees) * 0.5
    sp, cp = math.sin(pitch), math.cos(pitch)
    sy, cy = math.sin(yaw), math.cos(yaw)
    sr, cr = math.sin(roll), math.cos(roll)
    quaternion = (
        cr * sp * cy + sr * cp * sy,
        cr * cp * sy - sr * sp * cy,
        sr * cp * cy - cr * sp * sy,
        cr * cp * cy + sr * sp * sy,
    )
    length = math.sqrt(sum(component * component for component in quaternion))
    require(length > 1.0e-12 and math.isfinite(length), "rotation quaternion is invalid")
    x, y, z, w = (component / length for component in quaternion)
    return (
        (1.0 - 2.0 * (y * y + z * z), 2.0 * (x * y + w * z), 2.0 * (x * z - w * y)),
        (2.0 * (x * y - w * z), 1.0 - 2.0 * (x * x + z * z), 2.0 * (y * z + w * x)),
        (2.0 * (x * z + w * y), 2.0 * (y * z - w * x), 1.0 - 2.0 * (x * x + y * y)),
    )


def basis_conjugated_rotation(
    source_rotation: Sequence[Sequence[float]],
) -> tuple[tuple[float, ...], ...]:
    return mat3_mul(mat3_mul(mat3_transpose(BASIS), source_rotation), BASIS)


def mat4_identity() -> tuple[tuple[float, ...], ...]:
    return (
        (1.0, 0.0, 0.0, 0.0),
        (0.0, 1.0, 0.0, 0.0),
        (0.0, 0.0, 1.0, 0.0),
        (0.0, 0.0, 0.0, 1.0),
    )


def mat4_mul(
    left: Sequence[Sequence[float]], right: Sequence[Sequence[float]]
) -> tuple[tuple[float, ...], ...]:
    require(
        len(left) == 4
        and len(right) == 4
        and all(len(row) == 4 for row in left)
        and all(len(row) == 4 for row in right),
        "mat4 shape changed",
    )
    return tuple(
        tuple(
            sum(float(left[row][inner]) * float(right[inner][column]) for inner in range(4))
            for column in range(4)
        )
        for row in range(4)
    )


def scale_matrix(scale: Sequence[float]) -> tuple[tuple[float, ...], ...]:
    x, y, z = finite_vector(scale, 3, "scale")
    return (
        (x, 0.0, 0.0, 0.0),
        (0.0, y, 0.0, 0.0),
        (0.0, 0.0, z, 0.0),
        (0.0, 0.0, 0.0, 1.0),
    )


def translation_matrix(position: Sequence[float]) -> tuple[tuple[float, ...], ...]:
    x, y, z = finite_vector(position, 3, "translation")
    return (
        (1.0, 0.0, 0.0, 0.0),
        (0.0, 1.0, 0.0, 0.0),
        (0.0, 0.0, 1.0, 0.0),
        (x, y, z, 1.0),
    )


def embed_rotation3(
    rotation: Sequence[Sequence[float]],
) -> tuple[tuple[float, ...], ...]:
    require(len(rotation) == 3 and all(len(row) == 3 for row in rotation), "rotation shape changed")
    return (
        (float(rotation[0][0]), float(rotation[0][1]), float(rotation[0][2]), 0.0),
        (float(rotation[1][0]), float(rotation[1][1]), float(rotation[1][2]), 0.0),
        (float(rotation[2][0]), float(rotation[2][1]), float(rotation[2][2]), 0.0),
        (0.0, 0.0, 0.0, 1.0),
    )


def srt_matrix(
    scale: Sequence[float],
    rotation_degrees: Sequence[float],
    position: Sequence[float],
) -> tuple[tuple[float, ...], ...]:
    return mat4_mul(
        mat4_mul(scale_matrix(scale), embed_rotation3(directx_row_rotation_degrees(rotation_degrees))),
        translation_matrix(position),
    )


def row_point4_mul(
    point: Sequence[float], matrix: Sequence[Sequence[float]]
) -> tuple[float, float, float, float]:
    value = finite_vector(point, 4, "row point4")
    return tuple(
        sum(value[inner] * float(matrix[inner][column]) for inner in range(4))
        for column in range(4)
    )  # type: ignore[return-value]


def transform_point3(
    point: Sequence[float], matrix: Sequence[Sequence[float]]
) -> tuple[float, float, float]:
    result = row_point4_mul((*finite_vector(point, 3, "transform point"), 1.0), matrix)
    require(math.isclose(result[3], 1.0, rel_tol=1.0e-8, abs_tol=1.0e-8), "affine point W changed")
    return result[:3]


def matrix_to_json(matrix: Sequence[Sequence[float]]) -> list[list[float]]:
    return [[float(component) for component in row] for row in matrix]


def matrix_close(
    left: Sequence[Sequence[float]],
    right: Sequence[Sequence[float]],
    tolerance: float = 1.0e-9,
) -> bool:
    return len(left) == len(right) and all(
        vectors_close(left[row], right[row], tolerance) for row in range(len(left))
    )


def max_matrix_delta(
    left: Sequence[Sequence[float]], right: Sequence[Sequence[float]]
) -> float:
    require(len(left) == len(right), "matrix row count differs")
    return max(
        abs(float(left[row][column]) - float(right[row][column]))
        for row in range(len(left))
        for column in range(len(left[row]))
    )


def vector_sub(left: Sequence[float], right: Sequence[float]) -> tuple[float, float, float]:
    return tuple(float(left[index]) - float(right[index]) for index in range(3))  # type: ignore[return-value]


def vector_dot(left: Sequence[float], right: Sequence[float]) -> float:
    return sum(float(left[index]) * float(right[index]) for index in range(3))


def vector_cross(left: Sequence[float], right: Sequence[float]) -> tuple[float, float, float]:
    return (
        float(left[1]) * float(right[2]) - float(left[2]) * float(right[1]),
        float(left[2]) * float(right[0]) - float(left[0]) * float(right[2]),
        float(left[0]) * float(right[1]) - float(left[1]) * float(right[0]),
    )


def vector_normalize(value: Sequence[float]) -> tuple[float, float, float]:
    vector = finite_vector(value, 3, "normalization vector")
    length = math.sqrt(vector_dot(vector, vector))
    require(length > 1.0e-12 and math.isfinite(length), "normalization vector is degenerate")
    return tuple(component / length for component in vector)  # type: ignore[return-value]


def look_at_lh(
    eye: Sequence[float], focus: Sequence[float], up: Sequence[float]
) -> tuple[tuple[float, ...], ...]:
    eye_value = finite_vector(eye, 3, "camera eye")
    z_axis = vector_normalize(vector_sub(focus, eye_value))
    x_axis = vector_normalize(vector_cross(up, z_axis))
    y_axis = vector_cross(z_axis, x_axis)
    return (
        (x_axis[0], y_axis[0], z_axis[0], 0.0),
        (x_axis[1], y_axis[1], z_axis[1], 0.0),
        (x_axis[2], y_axis[2], z_axis[2], 0.0),
        (-vector_dot(x_axis, eye_value), -vector_dot(y_axis, eye_value), -vector_dot(z_axis, eye_value), 1.0),
    )


def perspective_fov_lh(
    fov_y_degrees: float, aspect: float, near_plane: float, far_plane: float
) -> tuple[tuple[float, ...], ...]:
    require(
        all(math.isfinite(value) for value in (fov_y_degrees, aspect, near_plane, far_plane))
        and 0.0 < fov_y_degrees < 180.0
        and aspect > 0.0
        and 0.0 < near_plane < far_plane,
        "camera projection fixture is invalid",
    )
    y_scale = 1.0 / math.tan(math.radians(fov_y_degrees) * 0.5)
    x_scale = y_scale / aspect
    depth = far_plane / (far_plane - near_plane)
    return (
        (x_scale, 0.0, 0.0, 0.0),
        (0.0, y_scale, 0.0, 0.0),
        (0.0, 0.0, depth, 1.0),
        (0.0, 0.0, -near_plane * depth, 0.0),
    )


def project_local_point(
    point: Sequence[float],
    world: Sequence[Sequence[float]],
    view: Sequence[Sequence[float]],
    projection: Sequence[Sequence[float]],
) -> dict[str, list[float]]:
    world_point = row_point4_mul((*finite_vector(point, 3, "project point"), 1.0), world)
    view_point = row_point4_mul(world_point, view)
    clip = row_point4_mul(view_point, projection)
    require(all(math.isfinite(value) for value in clip) and abs(clip[3]) > 1.0e-9, "clip point is invalid")
    ndc = tuple(clip[index] / clip[3] for index in range(3))
    return {
        "localPoint": [float(value) for value in point],
        "worldPoint": [float(value) for value in world_point[:3]],
        "viewPoint": [float(value) for value in view_point[:3]],
        "clipPoint": [float(value) for value in clip],
        "ndcPoint": [float(value) for value in ndc],
    }


def bounds_corners(minimum: Sequence[float], maximum: Sequence[float]) -> Iterable[tuple[float, float, float]]:
    low = finite_vector(minimum, 3, "bounds minimum")
    high = finite_vector(maximum, 3, "bounds maximum")
    require(all(low[axis] <= high[axis] for axis in range(3)), "bounds order changed")
    for x in (low[0], high[0]):
        for y in (low[1], high[1]):
            for z in (low[2], high[2]):
                yield (x, y, z)


def transformed_bounds(
    minimum: Sequence[float],
    maximum: Sequence[float],
    matrix: Sequence[Sequence[float]],
) -> dict[str, list[float]]:
    points = [transform_point3(point, matrix) for point in bounds_corners(minimum, maximum)]
    result_min = [min(point[axis] for point in points) for axis in range(3)]
    result_max = [max(point[axis] for point in points) for axis in range(3)]
    center = [(result_min[axis] + result_max[axis]) * 0.5 for axis in range(3)]
    return {"minimum": result_min, "maximum": result_max, "center": center}


def normalized_action_cue_program_projection(cue: dict[str, Any]) -> dict[str, Any]:
    payload = cue["typedPayload"]
    source_particle_system = payload["sourceParticleSystem"]
    require(
        isinstance(source_particle_system, str)
        and source_particle_system.startswith("ParticleSystem'")
        and source_particle_system.endswith("'"),
        "main cue ParticleSystem reference is malformed",
    )
    attachment_raw = payload["attachment"]
    mode = attachment_raw["mode"]
    require(mode in ("FOLLOW_NAMED_ANCHORS", "SNAPSHOT_ROOT"), "main cue attachment mode changed")
    follows = mode == "FOLLOW_NAMED_ANCHORS"
    source_names = attachment_raw["sourceAnchorNames"]
    attachment = {
        "enabled": True,
        "follow": follows,
        "sourceAnchorSlotId": source_names[0] if follows else "root",
        "runtimeAnchorSlotId": attachment_raw["runtimeAnchorSlotId"],
        "runtimeBoneName": attachment_raw["runtimeBoneName"],
        "socketLocalTransform": copy.deepcopy(attachment_raw["socketLocalTransform"]),
    }
    parameters = []
    for raw in payload["parameterOverrides"]:
        parameters.append(
            {
                "name": raw["name"],
                "kind": raw["type"].upper(),
                "scalarValue": float(raw["scalarValue"]) if raw["type"] == "scalar" else None,
                "vectorValue": [float(value) for value in raw["vectorValue"]] if raw["type"] == "vector" else [],
                "sourceIndex": raw["sourceIndex"],
                "sourceValueByteOffset": raw["sourceValueByteOffset"],
            }
        )
    parameters.sort(key=lambda row: row["name"])
    return {
        "sourceCueId": cue["cueId"],
        "sourceOccurrenceId": cue["sourceOccurrence"]["notifyId"],
        "sourceSystemId": source_particle_system[len("ParticleSystem'") : -1].casefold(),
        "cueLocalTransform": copy.deepcopy(payload["localTransform"]),
        "actionCueAttachment": attachment,
        "actionCueParameterInputs": parameters,
    }


def renderer_occurrence_projection(row: dict[str, Any]) -> dict[str, Any]:
    type_data = row["typeDataEvidence"]
    require(isinstance(type_data, list) and len(type_data) == 1, "main renderer TypeData denominator changed")
    projection = {
        "occurrenceId": row["occurrenceId"],
        "cueId": row["cueId"],
        "rendererFamily": row["rendererFamily"],
        "sourceSystemId": row["sourceSystemId"],
        "sourceEmitter": row["sourceEmitter"],
        "sourceEmitterNodeId": row["sourceEmitterNodeId"],
        "sourceEmitterRecordSha256": row["sourceEmitterRecordSha256"],
        "sourceMaterialPath": row["sourceMaterialPath"],
        "recipeId": row["recipeId"],
        "typeDataSourceObjectId": type_data[0]["sourceObjectId"],
        "typeDataSourceRecordSha256": type_data[0]["sourceRecordSha256"],
        "disposition": row["disposition"],
        "selectedVfPassAdmission": row["selectedVfPassAdmission"],
        "productAdmission": row["productAdmission"],
        "sourceRowSha256": row["rowSha256"],
    }
    seal_row(projection)
    return projection


def unique_by(rows: Sequence[dict[str, Any]], field: str, value: Any, label: str) -> dict[str, Any]:
    matches = [row for row in rows if row.get(field) == value]
    require(len(matches) == 1, f"{label} join count changed: {value}")
    return matches[0]


def constant_distribution_vector(
    program: dict[str, Any],
    emitter_id: str,
    source_class: str,
    property_path: str,
) -> tuple[list[float], dict[str, Any]]:
    module = unique_by(
        [row for row in program["modules"] if row["emitterId"] == emitter_id],
        "exactSourceClass",
        source_class,
        "main transform module",
    )
    property_row = unique_by(
        [row for row in program["properties"] if row["moduleId"] == module["moduleId"]],
        "propertyPath",
        property_path,
        "main transform property",
    )
    semantic_ids = property_row["semanticDistributionIds"]
    require(len(semantic_ids) == 1, f"{property_path} semantic distribution denominator changed")
    distribution = unique_by(
        program["distributions"],
        "distributionId",
        semantic_ids[0],
        "main transform distribution",
    )
    require(
        distribution["variant"] == "INLINE"
        and distribution["componentCount"] == 3
        and len(distribution["samples"]) >= 1,
        f"{property_path} distribution form changed",
    )
    first = finite_vector(distribution["samples"][0]["outputValues"][:3], 3, property_path)
    for sample in distribution["samples"]:
        require(
            vectors_close(first, sample["outputValues"][:3], 1.0e-7),
            f"{property_path} is no longer a constant main canary",
        )
    return [float(value) for value in first], {
        "moduleId": module["moduleId"],
        "moduleRowSha256": module["rowSha256"],
        "propertyId": property_row["propertyId"],
        "propertyRowSha256": property_row["rowSha256"],
        "distributionId": distribution["distributionId"],
        "distributionRowSha256": distribution["rowSha256"],
        "sampleOutputSha256": distribution["samples"][0]["outputSha256"],
    }


def build_basis_canary() -> dict[str, Any]:
    source_rotation = directx_row_rotation_degrees(BASIS_CANARY_SOURCE_EULER_DEGREES)
    client_rotation = basis_conjugated_rotation(source_rotation)
    naive_degrees = (
        BASIS_CANARY_SOURCE_EULER_DEGREES[0],
        BASIS_CANARY_SOURCE_EULER_DEGREES[2],
        -BASIS_CANARY_SOURCE_EULER_DEGREES[1],
    )
    naive_rotation = directx_row_rotation_degrees(naive_degrees)
    client_point = row_point3_mul(BASIS_CANARY_SOURCE_POINT, BASIS)
    rotate_then_convert = row_point3_mul(
        row_point3_mul(BASIS_CANARY_SOURCE_POINT, source_rotation), BASIS
    )
    convert_then_rotate = row_point3_mul(client_point, client_rotation)
    consistency_error = max(
        abs(rotate_then_convert[index] - convert_then_rotate[index]) for index in range(3)
    )
    naive_delta = max_matrix_delta(client_rotation, naive_rotation)
    require(consistency_error <= 1.0e-12, "basis conjugation point identity failed")
    require(naive_delta >= 0.1, "three-axis canary does not distinguish component swizzle")
    return {
        "sourceEulerDegrees": list(BASIS_CANARY_SOURCE_EULER_DEGREES),
        "sourceEulerComposition": "XMMatrixRotationRollPitchYaw_EQUIVALENT_IN_SOURCE_BASIS",
        "sourceRotationRows": matrix_to_json(source_rotation),
        "sourcePoint": list(BASIS_CANARY_SOURCE_POINT),
        "clientPointByBasis": list(client_point),
        "clientRotationRowsByConjugation": matrix_to_json(client_rotation),
        "rotateSourceThenConvertPoint": list(rotate_then_convert),
        "convertPointThenRotateClient": list(convert_then_rotate),
        "pointConsistencyMaxAbsError": consistency_error,
        "naiveComponentSwizzleDegrees": list(naive_degrees),
        "naiveComponentSwizzleRotationRows": matrix_to_json(naive_rotation),
        "conjugationVsNaiveMaxAbsDelta": naive_delta,
        "componentSwizzleRejected": True,
    }


def build_wmodel_projection(
    asset_id: str,
    resource_root: Path,
    binding_asset: dict[str, Any],
    carrier: dict[str, Any],
) -> tuple[dict[str, Any], tuple[float, float, float], tuple[float, ...]]:
    path = resource_root / asset_id
    require(path.is_file(), f"installed main WModel is missing: {path}")
    payload = path.read_bytes()
    parsed = parse_geometry_wmodel(payload)
    file_sha = hashlib.sha256(payload).hexdigest()
    require(
        binding_asset["candidateResource"]["sha256"] == file_sha
        and binding_asset["candidateResource"]["byteSize"] == len(payload)
        and carrier["candidateResourceSha256"] == file_sha
        and carrier["candidateResourceByteSize"] == len(payload),
        f"installed WModel differs from bound candidate: {asset_id}",
    )
    require(len(parsed["submeshes"]) == 1, f"main WModel submesh denominator changed: {asset_id}")
    submesh = parsed["submeshes"][0]
    bounds = tuple(float(value) for value in submesh["bounds"])
    require(len(bounds) == 10, f"main WModel bounds shape changed: {asset_id}")
    expected_tuple = binding_asset["expectedTuple"]
    require(
        parsed["payloadSha256"].hex() == expected_tuple["payloadSha256"]
        and parsed["metadataIdentitySha256"].hex() == expected_tuple["metadataIdentitySha256"]
        and parsed["sourceGltfSha256"].hex() == binding_asset["sourceGltf"]["sha256"]
        and parsed["vertexFlags"] == expected_tuple["channelMask"]
        and parsed["evidenceFlags"] == expected_tuple["evidenceFlags"]
        and math.isclose(parsed["geometryPreScale"], GEOMETRY_PRE_SCALE_F32, rel_tol=0.0, abs_tol=0.0)
        and expected_tuple["geometryPreScaleF32Hex"] == GEOMETRY_PRE_SCALE_F32_HEX,
        f"main WModel metadata tuple changed: {asset_id}",
    )
    expected_submesh = expected_tuple["submeshes"][0]
    actual_bounds_hex = [struct.pack(">f", value).hex() for value in bounds]
    require(
        expected_submesh["boundsF32Hex"] == actual_bounds_hex
        and expected_submesh["vertexCount"] == len(submesh["vertices"])
        and expected_submesh["indexCount"] == len(submesh["indices"]),
        f"main WModel bounds/count tuple changed: {asset_id}",
    )
    asymmetric_vertex = tuple(float(value) for value in submesh["vertices"][0]["values"][:3])
    minimum = bounds[:3]
    maximum = bounds[3:6]
    center = bounds[6:9]
    require(
        all(minimum[axis] <= asymmetric_vertex[axis] <= maximum[axis] for axis in range(3)),
        f"main WModel canary vertex is outside embedded bounds: {asset_id}",
    )
    row = {
        "assetId": asset_id,
        "physicalPath": repo_relative(path),
        "fileByteSize": len(payload),
        "fileSha256": file_sha,
        "formatVersion": "1.1",
        "payloadSha256": parsed["payloadSha256"].hex(),
        "sourceGltfSha256": parsed["sourceGltfSha256"].hex(),
        "sourceBufferSetSha256": parsed["sourceBufferSetSha256"].hex(),
        "metadataIdentitySha256": parsed["metadataIdentitySha256"].hex(),
        "sourceToWModelScale": float(parsed["sourceToWModelScale"]),
        "geometryPreScaleContractDecimal": GEOMETRY_PRE_SCALE_DECIMAL,
        "geometryPreScaleF32": float(parsed["geometryPreScale"]),
        "geometryPreScaleF32Hex": GEOMETRY_PRE_SCALE_F32_HEX,
        "geometryPreScaleApplicationCountInOracle": 1,
        "submesh": {
            "name": submesh["name"],
            "vertexCount": len(submesh["vertices"]),
            "indexCount": len(submesh["indices"]),
            "embeddedBounds": {
                "minimum": list(minimum),
                "maximum": list(maximum),
                "center": list(center),
                "radius": bounds[9],
                "boundsF32Hex": actual_bounds_hex,
            },
            "originWModel": [0.0, 0.0, 0.0],
            "aabbCenterMinusOriginWModel": list(center),
            "asymmetricVertexIndex": 0,
            "asymmetricVertexWModel": list(asymmetric_vertex),
        },
        "rawUpkToGltfPivot": {
            "status": "UNRESOLVED",
            "admitted": False,
            "reason": "NO_RAW_UPK_TO_GLTF_PIVOT_AUTHORITY_IN_G02_INPUTS",
        },
        "runtimeConsumptionClaimFromProgram": bool(carrier["preScaleConsumed"]),
        "productAdmission": False,
    }
    seal_row(row)
    return row, asymmetric_vertex, bounds


def build_receipt(
    action_cue_path: Path = DEFAULT_ACTION_CUE,
    renderer_matrix_path: Path = DEFAULT_RENDERER_MATRIX,
    runtime_program_path: Path = DEFAULT_RUNTIME_PROGRAM,
    geometry_binding_path: Path = DEFAULT_GEOMETRY_BINDING,
    resource_root: Path = DEFAULT_RESOURCE_ROOT,
) -> dict[str, Any]:
    action_cue = read_json_strict(action_cue_path)
    renderer_matrix = read_json_strict(renderer_matrix_path)
    runtime_program = read_json_strict(runtime_program_path)
    geometry_binding = read_json_strict(geometry_binding_path)
    require(
        action_cue.get("schema") == "lostark.effect-action-cue-recipe"
        and action_cue.get("formatVersion") == 2
        and action_cue.get("characterClass") == "ARTIST"
        and action_cue.get("skillId") == 31470
        and action_cue.get("inputSlot") == "F",
        "action cue root identity changed",
    )
    validate_self_digest(renderer_matrix, "receiptSha256", "renderer restoration matrix")
    require(
        renderer_matrix.get("schema") == "lostark.artist-31470-renderer-restoration-matrix-receipt"
        and renderer_matrix.get("formatVersion") == 1
        and renderer_matrix.get("skillId") == 31470,
        "renderer restoration matrix identity changed",
    )
    validate_self_digest(runtime_program, "programSha256", "reconstructed runtime program")
    require(
        runtime_program.get("schema") == "lostark.artist-31470-reconstructed-runtime-program"
        and runtime_program.get("formatVersion") == 1
        and runtime_program.get("target", {}).get("skillId") == 31470,
        "reconstructed runtime program identity changed",
    )
    validate_self_digest(geometry_binding, "receiptSha256", "geometry resource binding")
    require(
        geometry_binding.get("schema") == "lostark.artist-31470-geometry-resource-binding-receipt"
        and geometry_binding.get("formatVersion") == 1
        and geometry_binding.get("skillId") == 31470,
        "geometry resource binding identity changed",
    )

    cue = unique_by(action_cue["cues"], "cueId", MAIN_CUE_ID, "main action cue")
    cue_program_projection = normalized_action_cue_program_projection(cue)
    cue_payload = cue["typedPayload"]
    require(
        cue["sourceType"] == "PlayParticleEffect"
        and cue["executionEnabled"] is True
        and math.isclose(cue["localTimeSeconds"], MAIN_NOTIFY_SECONDS, rel_tol=0.0, abs_tol=1.0e-12)
        and cue_program_projection["sourceOccurrenceId"] == MAIN_SOURCE_OCCURRENCE_ID
        and cue_program_projection["sourceSystemId"] == MAIN_SOURCE_SYSTEM_ID,
        "main action cue timing/system changed",
    )
    attachment = cue_program_projection["actionCueAttachment"]
    local_transform = cue_program_projection["cueLocalTransform"]
    require(
        cue_payload["attachment"]["mode"] == "SNAPSHOT_ROOT"
        and attachment == {
            "enabled": True,
            "follow": False,
            "sourceAnchorSlotId": "root",
            "runtimeAnchorSlotId": "root",
            "runtimeBoneName": "",
            "socketLocalTransform": {
                "position": [0.0, 0.0, 0.0],
                "rotationDegrees": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        }
        and vectors_close(local_transform["sourcePositionUeUnits"], (100.0, -100.0, 0.0))
        and vectors_close(local_transform["position"], (1.0, 0.0, 1.0))
        and vectors_close(local_transform["rotationDegrees"], (0.0, 0.0, 0.0))
        and vectors_close(local_transform["scale"], (3.0, 3.0, 3.0)),
        "main action cue transform/attachment changed",
    )
    action_cue_projection = {
        "cueId": cue["cueId"],
        "localTimeSeconds": float(cue["localTimeSeconds"]),
        "sourceOccurrence": copy.deepcopy(cue["sourceOccurrence"]),
        "serializedPayloadSha256": cue["serializedPayload"]["sha256"],
        "sourcePositionUeCentimeters": copy.deepcopy(local_transform["sourcePositionUeUnits"]),
        "programProjection": cue_program_projection,
        "programProjectionSha256": canonical_sha256(cue_program_projection),
        "sourceCueCanonicalSha256": canonical_sha256(cue),
    }
    seal_row(action_cue_projection)

    renderer_by_id = {row["occurrenceId"]: row for row in renderer_matrix["occurrences"]}
    require(len(renderer_by_id) == len(renderer_matrix["occurrences"]), "renderer occurrence IDs are duplicated")
    renderer_projections = [
        renderer_occurrence_projection(renderer_by_id[occurrence_id])
        for occurrence_id in EXPECTED_OCCURRENCES
    ]
    for row in renderer_projections:
        require(
            row["cueId"] == MAIN_CUE_ID
            and row["rendererFamily"] == "MeshParticle"
            and row["sourceSystemId"] == MAIN_SOURCE_SYSTEM_ID
            and row["selectedVfPassAdmission"] is False
            and row["productAdmission"] is False,
            f"main renderer boundary changed: {row['occurrenceId']}",
        )

    emitter_by_evidence = {row["evidenceId"]: row for row in runtime_program["emitters"]}
    geometry_use_by_id = {row["geometryUseId"]: row for row in runtime_program["geometryUses"]}
    carrier_by_id = {row["carrierId"]: row for row in runtime_program["geometryCarriers"]}
    binding_asset_by_id = {row["assetId"]: row for row in geometry_binding["assets"]}
    required_asset_ids = tuple(dict.fromkeys(EXPECTED_GEOMETRY.values()))
    geometry_rows: list[dict[str, Any]] = []
    geometry_points: dict[str, tuple[float, float, float]] = {}
    geometry_bounds: dict[str, tuple[float, ...]] = {}
    carrier_by_asset: dict[str, dict[str, Any]] = {}
    for occurrence_id in EXPECTED_OCCURRENCES:
        emitter = emitter_by_evidence.get(occurrence_id)
        require(emitter is not None, f"runtime emitter is missing: {occurrence_id}")
        geometry_use = geometry_use_by_id.get(emitter["geometryUseId"])
        require(geometry_use is not None, f"runtime geometry use is missing: {occurrence_id}")
        carrier = carrier_by_id.get(geometry_use["carrierId"])
        require(carrier is not None, f"runtime geometry carrier is missing: {occurrence_id}")
        asset_id = EXPECTED_GEOMETRY[occurrence_id]
        require(
            geometry_use["assetId"] == asset_id
            and carrier["assetId"] == asset_id
            and geometry_use["preScaleApplication"] == "VERTEX_AND_BOUNDS_EXACTLY_ONCE_REQUIRED"
            and math.isclose(carrier["geometryPreScale"], GEOMETRY_PRE_SCALE_DECIMAL, rel_tol=0.0, abs_tol=0.0),
            f"runtime main geometry ownership changed: {occurrence_id}",
        )
        carrier_by_asset[asset_id] = carrier
    for asset_id in required_asset_ids:
        binding_asset = binding_asset_by_id.get(asset_id)
        carrier = carrier_by_asset.get(asset_id)
        require(binding_asset is not None and carrier is not None, f"main geometry binding is absent: {asset_id}")
        row, point, bounds = build_wmodel_projection(
            asset_id, resource_root, binding_asset, carrier
        )
        geometry_rows.append(row)
        geometry_points[asset_id] = point
        geometry_bounds[asset_id] = bounds

    basis_canary = build_basis_canary()
    root_matrix = srt_matrix(ROOT_SCALE, ROOT_ROTATION_DEGREES, ROOT_POSITION)
    cue_matrix = srt_matrix(
        local_transform["scale"],
        local_transform["rotationDegrees"],
        local_transform["position"],
    )
    presentation_yaw_matrix = srt_matrix(
        (1.0, 1.0, 1.0),
        (0.0, PLAYABLE_PRESENTATION_YAW_DEGREES, 0.0),
        (0.0, 0.0, 0.0),
    )
    view_matrix = look_at_lh(CAMERA_EYE, CAMERA_FOCUS, CAMERA_UP)
    projection_matrix = perspective_fov_lh(
        CAMERA_FOV_Y_DEGREES,
        CAMERA_ASPECT,
        CAMERA_NEAR,
        CAMERA_FAR,
    )
    fixed_fixture = {
        "capturedRoot": {
            "capturePolicy": "SNAPSHOT_ROOT_AT_ELEMENT_START",
            "follow": False,
            "lateInitialSeekHistory": {
                "status": "UNRESOLVED",
                "admitted": False,
                "reason": "NO_HISTORICAL_ACTOR_ROOT_PROVIDER",
            },
            "scale": list(ROOT_SCALE),
            "rotationDegreesClient": list(ROOT_ROTATION_DEGREES),
            "positionClientMeters": list(ROOT_POSITION),
            "matrixRows": matrix_to_json(root_matrix),
        },
        "cueLocal": {
            "scale": copy.deepcopy(local_transform["scale"]),
            "rotationDegreesClient": copy.deepcopy(local_transform["rotationDegrees"]),
            "positionClientMeters": copy.deepcopy(local_transform["position"]),
            "matrixRows": matrix_to_json(cue_matrix),
        },
        "playablePresentationYawDiagnostic": {
            "yawDegrees": PLAYABLE_PRESENTATION_YAW_DEGREES,
            "matrixRows": matrix_to_json(presentation_yaw_matrix),
            "relationStatus": "UNRESOLVED_BODY_MODEL_PRETRANSFORM_VS_EFFECT_ROOT",
            "selectedForRuntimeWorld": False,
        },
        "camera": {
            "handedness": "LEFT_HANDED",
            "rowVectorConvention": True,
            "eye": list(CAMERA_EYE),
            "focus": list(CAMERA_FOCUS),
            "up": list(CAMERA_UP),
            "fovYDegrees": CAMERA_FOV_Y_DEGREES,
            "aspect": CAMERA_ASPECT,
            "near": CAMERA_NEAR,
            "far": CAMERA_FAR,
            "viewMatrixRows": matrix_to_json(view_matrix),
            "projectionMatrixRows": matrix_to_json(projection_matrix),
        },
    }

    geometry_by_asset = {row["assetId"]: row for row in geometry_rows}
    canary_rows = []
    for occurrence_id in EXPECTED_OCCURRENCES:
        renderer_row = renderer_by_id[occurrence_id]
        emitter = emitter_by_evidence[occurrence_id]
        require(
            emitter["rendererType"] == "MeshParticle"
            and emitter["localSpace"] is True
            and emitter["sourceCueId"] == MAIN_CUE_ID
            and emitter["sourceActionCueProjectionSha256"] == canonical_sha256(cue_program_projection)
            and emitter["cueLocalTransform"] == cue_program_projection["cueLocalTransform"]
            and emitter["actionCueAttachment"] == cue_program_projection["actionCueAttachment"],
            f"runtime main cue projection changed: {occurrence_id}",
        )
        geometry_use = geometry_use_by_id[emitter["geometryUseId"]]
        carrier = carrier_by_id[geometry_use["carrierId"]]
        asset_id = EXPECTED_GEOMETRY[occurrence_id]
        start_size_source, start_size_seal = constant_distribution_vector(
            runtime_program, emitter["emitterId"], "particlemodulesize", "startsize"
        )
        mesh_rotation_turns_source, mesh_rotation_seal = constant_distribution_vector(
            runtime_program,
            emitter["emitterId"],
            "particlemodulemeshrotation",
            "startrotation",
        )
        start_size_client = [
            start_size_source[0],
            start_size_source[2],
            start_size_source[1],
        ]
        mesh_rotation_degrees_source = [value * 360.0 for value in mesh_rotation_turns_source]
        source_mesh_rotation = directx_row_rotation_degrees(mesh_rotation_degrees_source)
        client_mesh_rotation = basis_conjugated_rotation(source_mesh_rotation)
        geometry_pre_scale_matrix = scale_matrix(
            (GEOMETRY_PRE_SCALE_F32,) * 3
        )
        particle_size_matrix = scale_matrix(start_size_client)
        particle_rotation_matrix = embed_rotation3(client_mesh_rotation)
        particle_local_matrix = mat4_mul(
            mat4_mul(geometry_pre_scale_matrix, particle_size_matrix),
            particle_rotation_matrix,
        )
        runtime_world = mat4_mul(mat4_mul(particle_local_matrix, cue_matrix), root_matrix)
        presentation_yaw_candidate_world = mat4_mul(
            mat4_mul(mat4_mul(particle_local_matrix, cue_matrix), presentation_yaw_matrix),
            root_matrix,
        )
        bounds = geometry_bounds[asset_id]
        runtime_bounds = transformed_bounds(bounds[:3], bounds[3:6], runtime_world)
        presentation_bounds = transformed_bounds(
            bounds[:3], bounds[3:6], presentation_yaw_candidate_world
        )
        point = geometry_points[asset_id]
        row = {
            "occurrenceId": occurrence_id,
            "rendererSourceRowSha256": renderer_row["rowSha256"],
            "runtimeEmitterId": emitter["emitterId"],
            "runtimeEmitterRowSha256": emitter["rowSha256"],
            "geometryUseId": geometry_use["geometryUseId"],
            "geometryUseRowSha256": geometry_use["rowSha256"],
            "geometryCarrierId": carrier["carrierId"],
            "geometryCarrierRowSha256": carrier["rowSha256"],
            "geometryAssetId": asset_id,
            "geometryRowSha256": geometry_by_asset[asset_id]["rowSha256"],
            "localSpace": True,
            "attachment": copy.deepcopy(emitter["actionCueAttachment"]),
            "startSizeSourceUe": start_size_source,
            "startSizeClientAxisReordered": start_size_client,
            "startSizeEvidence": start_size_seal,
            "meshRotationTurnsSourceUe": mesh_rotation_turns_source,
            "meshRotationDegreesSourceUe": mesh_rotation_degrees_source,
            "meshRotationSourceRows": matrix_to_json(source_mesh_rotation),
            "meshRotationClientRowsByBasisConjugation": matrix_to_json(client_mesh_rotation),
            "meshRotationEvidence": mesh_rotation_seal,
            "applicationCounts": {
                "geometryPreScale": 1,
                "startSize": 1,
                "cueScale": 1,
            },
            "compositionOrderRowVector": [
                "wmodelLocal",
                "geometryPreScale",
                "particleStartSize",
                "basisConjugatedMeshRotation",
                "particleTranslationZero",
                "cueLocalSRT",
                "capturedActorRoot",
            ],
            "matrices": {
                "geometryPreScale": matrix_to_json(geometry_pre_scale_matrix),
                "particleStartSize": matrix_to_json(particle_size_matrix),
                "basisConjugatedMeshRotation": matrix_to_json(particle_rotation_matrix),
                "particleLocal": matrix_to_json(particle_local_matrix),
                "cueLocal": matrix_to_json(cue_matrix),
                "capturedRoot": matrix_to_json(root_matrix),
                "runtimeWorldWithoutBodyPresentationYaw": matrix_to_json(runtime_world),
                "playablePresentationYawDiagnostic": matrix_to_json(presentation_yaw_matrix),
                "worldWithPlayablePresentationYawDiagnostic": matrix_to_json(
                    presentation_yaw_candidate_world
                ),
            },
            "runtimeWorldVariant": {
                "selected": "WITHOUT_BODY_MODEL_PRESENTATION_YAW",
                "reason": "CURRENT_EFFECT_ROOT_IS_ACTOR_WORLD_AND_BODY_MODEL_YAW_RELATION_IS_UNRESOLVED",
            },
            "transformedEmbeddedBounds": {
                "runtimeWorld": runtime_bounds,
                "withPlayablePresentationYawDiagnostic": presentation_bounds,
            },
            "fixedViewProjectionPoint": {
                "runtimeWorld": project_local_point(
                    point, runtime_world, view_matrix, projection_matrix
                ),
                "withPlayablePresentationYawDiagnostic": project_local_point(
                    point,
                    presentation_yaw_candidate_world,
                    view_matrix,
                    projection_matrix,
                ),
            },
            "rawUpkToGltfPivotAdmitted": False,
            "visualApproval": False,
            "productAdmission": False,
        }
        seal_row(row)
        canary_rows.append(row)

    receipt = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "characterClass": "Artist",
        "skillId": 31470,
        "inputSlot": "F",
        "scope": "G02_MAIN_ROOT_BASIS_PIVOT_CANARY",
        "toolIdentity": {
            "path": repo_relative(SCRIPT_PATH),
            "canonicalLfSha256": tracked_text_sha256(SCRIPT_PATH),
        },
        "inputs": {
            "actionCueRecipe": input_identity(action_cue_path, action_cue),
            "rendererRestorationMatrixWholeReceipt": input_identity(
                renderer_matrix_path, renderer_matrix, seal_field="receiptSha256"
            ),
            "reconstructedRuntimeProgram": input_identity(
                runtime_program_path, runtime_program, seal_field="programSha256"
            ),
            "geometryResourceBinding": input_identity(
                geometry_binding_path, geometry_binding, seal_field="receiptSha256"
            ),
            "installedWModelCount": len(geometry_rows),
            "installedWModels": [
                {
                    "assetId": row["assetId"],
                    "physicalPath": row["physicalPath"],
                    "byteSize": row["fileByteSize"],
                    "rawSha256": row["fileSha256"],
                }
                for row in geometry_rows
            ],
        },
        "coordinateContract": {
            "rowVectorConvention": True,
            "sourceToClientBasisRows": matrix_to_json(BASIS),
            "sourceVectorToClient": "[x,y,z]*B=[x,z,-y]",
            "sourceCentimetersToClientMeters": 0.01,
            "sourceScaleToClient": "[x,y,z]->[x,z,y]",
            "rotationConversion": "R_client=transpose(B)*R_source*B",
            "eulerComponentSwizzleAllowed": False,
        },
        "basisConjugationCanary": basis_canary,
        "actionCueProjection": action_cue_projection,
        "rendererOccurrenceProjection": renderer_projections,
        "fixedFixture": fixed_fixture,
        "geometry": geometry_rows,
        "occurrences": canary_rows,
        "summary": {
            "mainOccurrenceCount": len(canary_rows),
            "mainWModelCount": len(geometry_rows),
            "basisConjugationCanaryCount": 1,
            "geometryPreScaleExactlyOnceCount": len(canary_rows),
            "rawUpkToGltfPivotResolvedCount": 0,
            "lateHistoricalRootResolvedCount": 0,
            "playablePresentationYawSelectedCount": 0,
            "visualApprovalCount": 0,
            "productAdmissionCount": 0,
        },
        "admission": {
            "actionCueRootSnapshotProjection": True,
            "threeAxisBasisConjugation": True,
            "currentWModelIdentityAndBounds": True,
            "geometryPreScaleExactlyOnceInOracle": True,
            "rawUpkToGltfPivot": False,
            "lateHistoricalRootReplay": False,
            "playablePresentationYawRelation": False,
            "runtimeExecution": False,
            "visualApproval": False,
            "product": False,
            "blockers": [
                "RAW_UPK_TO_GLTF_PIVOT_PROVENANCE_UNRESOLVED",
                "LATE_INITIAL_SEEK_HISTORICAL_ACTOR_ROOT_PROVIDER_UNRESOLVED",
                "PLAYABLE_PRESENTATION_YAW_RELATION_UNRESOLVED",
                "G02_RUNTIME_NUMERIC_CONSUMER_NOT_ADMITTED",
                "MANUAL_VISUAL_APPROVAL_REQUIRED",
                "PRODUCT_ADMISSION_NOT_OPEN",
            ],
        },
    }
    seal_receipt(receipt)
    return receipt


def require_matrix_field(
    actual: Any,
    expected: Sequence[Sequence[float]],
    label: str,
    tolerance: float = 1.0e-9,
) -> None:
    require(
        isinstance(actual, list)
        and len(actual) == len(expected)
        and matrix_close(actual, expected, tolerance),
        f"{label} changed",
    )


def validate_projection_result(
    actual: dict[str, Any],
    point: Sequence[float],
    world: Sequence[Sequence[float]],
    view: Sequence[Sequence[float]],
    projection: Sequence[Sequence[float]],
    label: str,
) -> None:
    expected = project_local_point(point, world, view, projection)
    require(set(actual) == set(expected), f"{label} fields changed")
    for field in expected:
        require(vectors_close(actual[field], expected[field], 1.0e-8), f"{label} {field} changed")


def validate_receipt(receipt: dict[str, Any]) -> None:
    require(
        receipt.get("schema") == SCHEMA
        and receipt.get("formatVersion") == FORMAT_VERSION
        and receipt.get("characterClass") == "Artist"
        and receipt.get("skillId") == 31470
        and receipt.get("inputSlot") == "F"
        and receipt.get("scope") == "G02_MAIN_ROOT_BASIS_PIVOT_CANARY",
        "main transform oracle identity changed",
    )
    require(
        receipt["toolIdentity"]
        == {
            "path": repo_relative(SCRIPT_PATH),
            "canonicalLfSha256": tracked_text_sha256(SCRIPT_PATH),
        },
        "main transform oracle tool identity changed",
    )
    inputs = receipt["inputs"]
    require(
        inputs["installedWModelCount"] == 2
        and len(inputs["installedWModels"]) == 2
        and all(
            isinstance(row["rawSha256"], str)
            and len(row["rawSha256"]) == 64
            and row["byteSize"] > 0
            for row in inputs["installedWModels"]
        ),
        "installed WModel input identity changed",
    )
    for field, seal_field in (
        ("rendererRestorationMatrixWholeReceipt", "receiptSha256"),
        ("reconstructedRuntimeProgram", "programSha256"),
        ("geometryResourceBinding", "receiptSha256"),
    ):
        row = inputs[field]
        require(
            all(isinstance(row[name], str) and len(row[name]) == 64 for name in ("rawSha256", "canonicalJsonSha256", seal_field)),
            f"{field} whole-input provenance changed",
        )

    contract = receipt["coordinateContract"]
    require(
        contract["rowVectorConvention"] is True
        and contract["sourceVectorToClient"] == "[x,y,z]*B=[x,z,-y]"
        and contract["sourceCentimetersToClientMeters"] == 0.01
        and contract["sourceScaleToClient"] == "[x,y,z]->[x,z,y]"
        and contract["rotationConversion"] == "R_client=transpose(B)*R_source*B"
        and contract["eulerComponentSwizzleAllowed"] is False,
        "coordinate policy changed",
    )
    require_matrix_field(contract["sourceToClientBasisRows"], BASIS, "source-to-client basis")
    expected_basis_canary = build_basis_canary()
    require(receipt["basisConjugationCanary"].keys() == expected_basis_canary.keys(), "basis canary fields changed")
    for field, expected in expected_basis_canary.items():
        actual = receipt["basisConjugationCanary"][field]
        if field.endswith("Rows"):
            require_matrix_field(actual, expected, f"basis canary {field}")
        elif isinstance(expected, list) and expected and isinstance(expected[0], (int, float)):
            require(vectors_close(actual, expected, 1.0e-12), f"basis canary {field} changed")
        elif isinstance(expected, float):
            require(math.isclose(actual, expected, rel_tol=1.0e-12, abs_tol=1.0e-12), f"basis canary {field} changed")
        else:
            require(actual == expected, f"basis canary {field} changed")

    cue = receipt["actionCueProjection"]
    validate_row_seal(cue, "main action cue projection")
    cue_program = cue["programProjection"]
    require(
        cue["cueId"] == MAIN_CUE_ID
        and math.isclose(cue["localTimeSeconds"], MAIN_NOTIFY_SECONDS, rel_tol=0.0, abs_tol=1.0e-12)
        and vectors_close(cue["sourcePositionUeCentimeters"], (100.0, -100.0, 0.0))
        and cue["programProjectionSha256"] == canonical_sha256(cue_program)
        and cue_program["sourceOccurrenceId"] == MAIN_SOURCE_OCCURRENCE_ID
        and cue_program["sourceSystemId"] == MAIN_SOURCE_SYSTEM_ID
        and cue_program["actionCueAttachment"]["enabled"] is True
        and cue_program["actionCueAttachment"]["follow"] is False
        and cue_program["actionCueAttachment"]["sourceAnchorSlotId"] == "root"
        and cue_program["actionCueAttachment"]["runtimeAnchorSlotId"] == "root"
        and cue_program["actionCueAttachment"]["runtimeBoneName"] == ""
        and vectors_close(cue_program["cueLocalTransform"]["position"], (1.0, 0.0, 1.0))
        and vectors_close(cue_program["cueLocalTransform"]["rotationDegrees"], (0.0, 0.0, 0.0))
        and vectors_close(cue_program["cueLocalTransform"]["scale"], (3.0, 3.0, 3.0)),
        "main action cue projection changed",
    )

    renderer_rows = receipt["rendererOccurrenceProjection"]
    require(
        [row["occurrenceId"] for row in renderer_rows] == list(EXPECTED_OCCURRENCES),
        "main renderer projection denominator/order changed",
    )
    for row in renderer_rows:
        validate_row_seal(row, f"main renderer projection {row['occurrenceId']}")
        require(
            row["cueId"] == MAIN_CUE_ID
            and row["rendererFamily"] == "MeshParticle"
            and row["sourceSystemId"] == MAIN_SOURCE_SYSTEM_ID
            and row["selectedVfPassAdmission"] is False
            and row["productAdmission"] is False,
            f"main renderer projection admission changed: {row['occurrenceId']}",
        )

    fixture = receipt["fixedFixture"]
    root = fixture["capturedRoot"]
    cue_fixture = fixture["cueLocal"]
    yaw_fixture = fixture["playablePresentationYawDiagnostic"]
    camera = fixture["camera"]
    root_matrix = srt_matrix(ROOT_SCALE, ROOT_ROTATION_DEGREES, ROOT_POSITION)
    cue_matrix = srt_matrix((3.0, 3.0, 3.0), (0.0, 0.0, 0.0), (1.0, 0.0, 1.0))
    yaw_matrix = srt_matrix((1.0, 1.0, 1.0), (0.0, PLAYABLE_PRESENTATION_YAW_DEGREES, 0.0), (0.0, 0.0, 0.0))
    view_matrix = look_at_lh(CAMERA_EYE, CAMERA_FOCUS, CAMERA_UP)
    projection_matrix = perspective_fov_lh(CAMERA_FOV_Y_DEGREES, CAMERA_ASPECT, CAMERA_NEAR, CAMERA_FAR)
    require(
        root["capturePolicy"] == "SNAPSHOT_ROOT_AT_ELEMENT_START"
        and root["follow"] is False
        and root["lateInitialSeekHistory"]
        == {
            "status": "UNRESOLVED",
            "admitted": False,
            "reason": "NO_HISTORICAL_ACTOR_ROOT_PROVIDER",
        }
        and vectors_close(root["scale"], ROOT_SCALE)
        and vectors_close(root["rotationDegreesClient"], ROOT_ROTATION_DEGREES)
        and vectors_close(root["positionClientMeters"], ROOT_POSITION),
        "captured root fixture changed",
    )
    require_matrix_field(root["matrixRows"], root_matrix, "captured root matrix")
    require(
        vectors_close(cue_fixture["scale"], (3.0, 3.0, 3.0))
        and vectors_close(cue_fixture["rotationDegreesClient"], (0.0, 0.0, 0.0))
        and vectors_close(cue_fixture["positionClientMeters"], (1.0, 0.0, 1.0)),
        "cue fixture changed",
    )
    require_matrix_field(cue_fixture["matrixRows"], cue_matrix, "cue matrix")
    require(
        yaw_fixture["yawDegrees"] == PLAYABLE_PRESENTATION_YAW_DEGREES
        and yaw_fixture["relationStatus"] == "UNRESOLVED_BODY_MODEL_PRETRANSFORM_VS_EFFECT_ROOT"
        and yaw_fixture["selectedForRuntimeWorld"] is False,
        "playable presentation yaw boundary changed",
    )
    require_matrix_field(yaw_fixture["matrixRows"], yaw_matrix, "playable presentation yaw matrix")
    require(
        camera["handedness"] == "LEFT_HANDED"
        and camera["rowVectorConvention"] is True
        and vectors_close(camera["eye"], CAMERA_EYE)
        and vectors_close(camera["focus"], CAMERA_FOCUS)
        and vectors_close(camera["up"], CAMERA_UP)
        and camera["fovYDegrees"] == CAMERA_FOV_Y_DEGREES
        and camera["aspect"] == CAMERA_ASPECT
        and camera["near"] == CAMERA_NEAR
        and camera["far"] == CAMERA_FAR,
        "fixed camera fixture changed",
    )
    require_matrix_field(camera["viewMatrixRows"], view_matrix, "view matrix")
    require_matrix_field(camera["projectionMatrixRows"], projection_matrix, "projection matrix")

    geometry_rows = receipt["geometry"]
    require(
        [row["assetId"] for row in geometry_rows]
        == [
            "Effect/Artist/Meshes/fm_h_swing_03.wmodel",
            "Effect/Artist/Meshes/fm_h_swing_05.wmodel",
        ],
        "main WModel projection denominator/order changed",
    )
    geometry_by_asset: dict[str, dict[str, Any]] = {}
    for row in geometry_rows:
        validate_row_seal(row, f"main WModel projection {row['assetId']}")
        bounds = row["submesh"]["embeddedBounds"]
        minimum = finite_vector(bounds["minimum"], 3, "WModel bounds minimum")
        maximum = finite_vector(bounds["maximum"], 3, "WModel bounds maximum")
        center = finite_vector(bounds["center"], 3, "WModel bounds center")
        expected_center = tuple((minimum[axis] + maximum[axis]) * 0.5 for axis in range(3))
        require(
            isinstance(row["fileSha256"], str)
            and len(row["fileSha256"]) == 64
            and row["fileByteSize"] > 0
            and row["formatVersion"] == "1.1"
            and row["sourceToWModelScale"] == 100.0
            and row["geometryPreScaleContractDecimal"] == GEOMETRY_PRE_SCALE_DECIMAL
            and row["geometryPreScaleF32"] == GEOMETRY_PRE_SCALE_F32
            and row["geometryPreScaleF32Hex"] == GEOMETRY_PRE_SCALE_F32_HEX
            and row["geometryPreScaleApplicationCountInOracle"] == 1
            and all(minimum[axis] <= maximum[axis] for axis in range(3))
            and vectors_close(center, expected_center, 1.0e-5)
            and vectors_close(row["submesh"]["aabbCenterMinusOriginWModel"], center, 1.0e-9)
            and row["submesh"]["asymmetricVertexIndex"] == 0
            and all(
                minimum[axis] <= row["submesh"]["asymmetricVertexWModel"][axis] <= maximum[axis]
                for axis in range(3)
            )
            and row["rawUpkToGltfPivot"]
            == {
                "status": "UNRESOLVED",
                "admitted": False,
                "reason": "NO_RAW_UPK_TO_GLTF_PIVOT_AUTHORITY_IN_G02_INPUTS",
            }
            and row["productAdmission"] is False,
            f"main WModel transform/pivot boundary changed: {row['assetId']}",
        )
        geometry_by_asset[row["assetId"]] = row

    occurrence_rows = receipt["occurrences"]
    require(
        [row["occurrenceId"] for row in occurrence_rows] == list(EXPECTED_OCCURRENCES),
        "main transform canary denominator/order changed",
    )
    renderer_projection_by_id = {row["occurrenceId"]: row for row in renderer_rows}
    for row in occurrence_rows:
        occurrence_id = row["occurrenceId"]
        validate_row_seal(row, f"main transform canary {occurrence_id}")
        asset_id = EXPECTED_GEOMETRY[occurrence_id]
        geometry = geometry_by_asset[asset_id]
        start_source = finite_vector(row["startSizeSourceUe"], 3, "StartSize source")
        expected_start_client = (start_source[0], start_source[2], start_source[1])
        rotation_turns = finite_vector(row["meshRotationTurnsSourceUe"], 3, "MeshRotation turns")
        rotation_degrees = tuple(value * 360.0 for value in rotation_turns)
        source_rotation = directx_row_rotation_degrees(rotation_degrees)
        client_rotation = basis_conjugated_rotation(source_rotation)
        pre_scale_matrix = scale_matrix((GEOMETRY_PRE_SCALE_F32,) * 3)
        start_size_matrix = scale_matrix(expected_start_client)
        rotation_matrix = embed_rotation3(client_rotation)
        particle_local = mat4_mul(mat4_mul(pre_scale_matrix, start_size_matrix), rotation_matrix)
        runtime_world = mat4_mul(mat4_mul(particle_local, cue_matrix), root_matrix)
        yaw_world = mat4_mul(mat4_mul(mat4_mul(particle_local, cue_matrix), yaw_matrix), root_matrix)
        matrices = row["matrices"]
        require(
            row["rendererSourceRowSha256"] == renderer_projection_by_id[occurrence_id]["sourceRowSha256"]
            and row["geometryAssetId"] == asset_id
            and row["geometryRowSha256"] == geometry["rowSha256"]
            and row["localSpace"] is True
            and row["attachment"]["enabled"] is True
            and row["attachment"]["follow"] is False
            and row["attachment"]["sourceAnchorSlotId"] == "root"
            and row["attachment"]["runtimeAnchorSlotId"] == "root"
            and row["attachment"]["runtimeBoneName"] == ""
            and vectors_close(row["startSizeClientAxisReordered"], expected_start_client, 1.0e-8)
            and vectors_close(row["meshRotationDegreesSourceUe"], rotation_degrees, 1.0e-8)
            and row["applicationCounts"]
            == {"geometryPreScale": 1, "startSize": 1, "cueScale": 1}
            and row["runtimeWorldVariant"]["selected"] == "WITHOUT_BODY_MODEL_PRESENTATION_YAW"
            and row["rawUpkToGltfPivotAdmitted"] is False
            and row["visualApproval"] is False
            and row["productAdmission"] is False,
            f"main transform owner/admission changed: {occurrence_id}",
        )
        require_matrix_field(row["meshRotationSourceRows"], source_rotation, f"{occurrence_id} source rotation")
        require_matrix_field(row["meshRotationClientRowsByBasisConjugation"], client_rotation, f"{occurrence_id} client rotation")
        require_matrix_field(matrices["geometryPreScale"], pre_scale_matrix, f"{occurrence_id} pre-scale")
        require_matrix_field(matrices["particleStartSize"], start_size_matrix, f"{occurrence_id} StartSize")
        require_matrix_field(matrices["basisConjugatedMeshRotation"], rotation_matrix, f"{occurrence_id} mesh rotation")
        require_matrix_field(matrices["particleLocal"], particle_local, f"{occurrence_id} particle local")
        require_matrix_field(matrices["cueLocal"], cue_matrix, f"{occurrence_id} cue local")
        require_matrix_field(matrices["capturedRoot"], root_matrix, f"{occurrence_id} captured root")
        require_matrix_field(matrices["runtimeWorldWithoutBodyPresentationYaw"], runtime_world, f"{occurrence_id} runtime world")
        require_matrix_field(matrices["playablePresentationYawDiagnostic"], yaw_matrix, f"{occurrence_id} yaw diagnostic")
        require_matrix_field(matrices["worldWithPlayablePresentationYawDiagnostic"], yaw_world, f"{occurrence_id} yaw world")
        embedded = geometry["submesh"]["embeddedBounds"]
        expected_runtime_bounds = transformed_bounds(embedded["minimum"], embedded["maximum"], runtime_world)
        expected_yaw_bounds = transformed_bounds(embedded["minimum"], embedded["maximum"], yaw_world)
        for variant, expected_bounds in (
            ("runtimeWorld", expected_runtime_bounds),
            ("withPlayablePresentationYawDiagnostic", expected_yaw_bounds),
        ):
            actual_bounds = row["transformedEmbeddedBounds"][variant]
            require(
                all(vectors_close(actual_bounds[field], expected_bounds[field], 1.0e-7) for field in ("minimum", "maximum", "center")),
                f"{occurrence_id} transformed bounds changed: {variant}",
            )
        point = geometry["submesh"]["asymmetricVertexWModel"]
        validate_projection_result(
            row["fixedViewProjectionPoint"]["runtimeWorld"],
            point,
            runtime_world,
            view_matrix,
            projection_matrix,
            f"{occurrence_id} runtime clip/NDC",
        )
        validate_projection_result(
            row["fixedViewProjectionPoint"]["withPlayablePresentationYawDiagnostic"],
            point,
            yaw_world,
            view_matrix,
            projection_matrix,
            f"{occurrence_id} yaw clip/NDC",
        )

    summary = receipt["summary"]
    require(
        summary
        == {
            "mainOccurrenceCount": 3,
            "mainWModelCount": 2,
            "basisConjugationCanaryCount": 1,
            "geometryPreScaleExactlyOnceCount": 3,
            "rawUpkToGltfPivotResolvedCount": 0,
            "lateHistoricalRootResolvedCount": 0,
            "playablePresentationYawSelectedCount": 0,
            "visualApprovalCount": 0,
            "productAdmissionCount": 0,
        },
        "main transform summary changed",
    )
    admission = receipt["admission"]
    require(
        admission["actionCueRootSnapshotProjection"] is True
        and admission["threeAxisBasisConjugation"] is True
        and admission["currentWModelIdentityAndBounds"] is True
        and admission["geometryPreScaleExactlyOnceInOracle"] is True
        and all(
            admission[field] is False
            for field in (
                "rawUpkToGltfPivot",
                "lateHistoricalRootReplay",
                "playablePresentationYawRelation",
                "runtimeExecution",
                "visualApproval",
                "product",
            )
        )
        and admission["blockers"]
        == [
            "RAW_UPK_TO_GLTF_PIVOT_PROVENANCE_UNRESOLVED",
            "LATE_INITIAL_SEEK_HISTORICAL_ACTOR_ROOT_PROVIDER_UNRESOLVED",
            "PLAYABLE_PRESENTATION_YAW_RELATION_UNRESOLVED",
            "G02_RUNTIME_NUMERIC_CONSUMER_NOT_ADMITTED",
            "MANUAL_VISUAL_APPROVAL_REQUIRED",
            "PRODUCT_ADMISSION_NOT_OPEN",
        ],
        "main transform fail-closed admission changed",
    )
    validate_self_digest(receipt, "receiptSha256", "main transform oracle receipt")


def encoded_receipt(receipt: dict[str, Any]) -> bytes:
    return (json.dumps(receipt, ensure_ascii=False, indent=2, allow_nan=False) + "\n").encode("utf-8")


def write_or_check(path: Path, receipt: dict[str, Any], check: bool) -> None:
    encoded = encoded_receipt(receipt)
    if check:
        require(path.is_file(), f"main transform oracle receipt is missing: {path}")
        require(path.read_bytes() == encoded, "main transform oracle receipt is stale")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_bytes(encoded)
    temporary.replace(path)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--action-cue", type=Path, default=DEFAULT_ACTION_CUE)
    parser.add_argument("--renderer-matrix", type=Path, default=DEFAULT_RENDERER_MATRIX)
    parser.add_argument("--runtime-program", type=Path, default=DEFAULT_RUNTIME_PROGRAM)
    parser.add_argument("--geometry-binding", type=Path, default=DEFAULT_GEOMETRY_BINDING)
    parser.add_argument("--resource-root", type=Path, default=DEFAULT_RESOURCE_ROOT)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--validate-only", action="store_true")
    args = parser.parse_args()
    if args.validate_only:
        require(args.output.is_file(), f"main transform oracle receipt is missing: {args.output}")
        receipt = read_json_strict(args.output)
        validate_receipt(receipt)
        print(
            "PASS: Artist 31470 main transform oracle shallow "
            "occurrences=3 wmodels=2 pivot=UNRESOLVED visual=false product=false"
        )
        return 0
    receipt = build_receipt(
        args.action_cue,
        args.renderer_matrix,
        args.runtime_program,
        args.geometry_binding,
        args.resource_root,
    )
    validate_receipt(receipt)
    write_or_check(args.output, receipt, args.check)
    mode = "deep-check" if args.check else "deep-write"
    print(
        f"PASS: Artist 31470 main transform oracle {mode} "
        "occurrences=3 wmodels=2 preScale=1x pivot=UNRESOLVED visual=false product=false"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
