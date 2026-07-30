from __future__ import annotations

import argparse
import json
import math
import shlex
import struct
from pathlib import Path
from typing import Any

import numpy as np


# 파괴 가능한 외곽 바닥.
# overlay 문서에는 각 메시가 0도/180도로 두 번씩 배치되어 있다.
OVERLAY_FLOOR_ASSETS = {
    "BG_RAD_VALTAN_FLOOR01_SM": "floor01",
    "BG_RAD_VALTAN_FLOOR01A_SM": "floor01a",
    "BG_RAD_VALTAN_FLOOR01B_SM": "floor01b",
}

# 아레나 본체는 같은 메시를 약 90도씩 회전한 네 개의 placement다.
MAIN_FLOOR_ASSET_ID = (
    "MAP_4A6CF4B84315_LV_LUT_HEARTRB_FLOOR01_SM"
)

MAIN_FLOOR_SOURCE_PLACEMENT_IDS = (
    "LV_LUT_HEARTRB_ED_SL00:export:1271",
    "LV_LUT_HEARTRB_ED_SL00:export:1299",
    "LV_LUT_HEARTRB_ED_SL00:export:1304",
    "LV_LUT_HEARTRB_ED_SL00:export:1337",
)

# 중앙 원형 캡.
CENTER_FLOOR_ASSET_ID = (
    "MAP_FBC80A02F72E_BG_LUT_WAGLOY_CIRCLEFLOOR01_SM_JJY"
)

CENTER_FLOOR_SOURCE_PLACEMENT_ID = (
    "LV_LUT_HEARTRB_ED_SL00:export:1274"
)

# 베이크 범위를 정하는 CUL_BOX.
# 이 메시 자체를 Walkable로 굽는 것은 아니다.
BOUNDS_ASSET_ID = (
    "MAP_1E5F55FC0524_LV_COMMON_MESH_CUL_BOX_8"
)

BOUNDS_SOURCE_PLACEMENT_ID = (
    "LV_LUT_HEARTRB_ED_SL01:export:2767"
)


COMPONENT_TYPES = {
    5121: np.dtype("<u1"),
    5123: np.dtype("<u2"),
    5125: np.dtype("<u4"),
    5126: np.dtype("<f4"),
}

TYPE_WIDTHS = {
    "SCALAR": 1,
    "VEC2": 2,
    "VEC3": 3,
    "VEC4": 4,
    "MAT4": 16,
}


def read_gltf(
    path: Path,
) -> tuple[dict[str, Any], list[bytes]]:
    document = json.loads(
        path.read_text(encoding="utf-8")
    )

    buffers: list[bytes] = []

    for buffer in document.get("buffers", []):
        uri = buffer.get("uri")

        if not isinstance(uri, str):
            raise ValueError(
                f"glTF buffer URI is missing: {path}"
            )

        if uri.startswith("data:"):
            raise ValueError(
                "embedded glTF buffers are not supported"
            )

        payload_path = path.parent / uri
        payload = payload_path.read_bytes()

        expected_size = int(
            buffer.get("byteLength", 0)
        )

        if len(payload) < expected_size:
            raise ValueError(
                f"short glTF buffer: {payload_path}"
            )

        buffers.append(payload)

    return document, buffers


def read_accessor(
    document: dict[str, Any],
    buffers: list[bytes],
    accessor_index: int,
) -> np.ndarray:
    accessor = document["accessors"][accessor_index]

    if "sparse" in accessor:
        raise ValueError(
            "sparse glTF accessors are not supported"
        )

    view_index = int(accessor["bufferView"])
    view = document["bufferViews"][view_index]

    component_type = int(
        accessor["componentType"]
    )

    data_type = COMPONENT_TYPES.get(
        component_type
    )

    element_width = TYPE_WIDTHS.get(
        accessor["type"]
    )

    if data_type is None or element_width is None:
        raise ValueError(
            "unsupported glTF accessor format"
        )

    count = int(accessor["count"])

    packed_stride = (
        data_type.itemsize * element_width
    )

    stride = int(
        view.get("byteStride", packed_stride)
    )

    if stride < packed_stride:
        raise ValueError(
            "invalid glTF byteStride"
        )

    offset = (
        int(view.get("byteOffset", 0))
        + int(accessor.get("byteOffset", 0))
    )

    required_size = offset

    if count > 0:
        required_size += (
            (count - 1) * stride
            + packed_stride
        )

    payload = buffers[int(view["buffer"])]

    if required_size > len(payload):
        raise ValueError(
            "glTF accessor exceeds its buffer"
        )

    return np.ndarray(
        shape=(count, element_width),
        dtype=data_type,
        buffer=payload,
        offset=offset,
        strides=(stride, data_type.itemsize),
    ).copy()


def quaternion_matrix(
    quaternion: list[float],
) -> np.ndarray:
    x, y, z, w = (
        float(value)
        for value in quaternion
    )

    length = math.sqrt(
        x * x + y * y + z * z + w * w
    )

    if (
        not math.isfinite(length)
        or length <= 1e-12
    ):
        raise ValueError(
            "invalid quaternion"
        )

    x /= length
    y /= length
    z /= length
    w /= length

    return np.array(
        [
            [
                1 - 2 * (y * y + z * z),
                2 * (x * y - z * w),
                2 * (x * z + y * w),
                0,
            ],
            [
                2 * (x * y + z * w),
                1 - 2 * (x * x + z * z),
                2 * (y * z - x * w),
                0,
            ],
            [
                2 * (x * z - y * w),
                2 * (y * z + x * w),
                1 - 2 * (x * x + y * y),
                0,
            ],
            [0, 0, 0, 1],
        ],
        dtype=np.float64,
    )


def node_matrix(
    node: dict[str, Any],
) -> np.ndarray:
    if "matrix" in node:
        values = np.asarray(
            node["matrix"],
            dtype=np.float64,
        )

        if values.shape != (16,):
            raise ValueError(
                "invalid glTF node matrix"
            )

        # glTF matrix는 column-major다.
        return values.reshape(
            (4, 4),
            order="F",
        )

    translation = np.eye(
        4,
        dtype=np.float64,
    )

    translation[:3, 3] = np.asarray(
        node.get(
            "translation",
            [0, 0, 0],
        ),
        dtype=np.float64,
    )

    scale_values = np.asarray(
        node.get(
            "scale",
            [1, 1, 1],
        ),
        dtype=np.float64,
    )

    scale = np.eye(
        4,
        dtype=np.float64,
    )

    scale[0, 0] = scale_values[0]
    scale[1, 1] = scale_values[1]
    scale[2, 2] = scale_values[2]

    rotation = quaternion_matrix(
        node.get(
            "rotation",
            [0, 0, 0, 1],
        )
    )

    return translation @ rotation @ scale


def load_triangles(
    path: Path,
) -> np.ndarray:
    document, buffers = read_gltf(path)

    nodes = document.get("nodes", [])
    triangle_blocks: list[np.ndarray] = []

    def visit(
        node_index: int,
        parent_matrix: np.ndarray,
    ) -> None:
        node = nodes[node_index]

        transform = (
            parent_matrix
            @ node_matrix(node)
        )

        if "mesh" in node:
            mesh_index = int(node["mesh"])
            mesh = document["meshes"][mesh_index]

            for primitive in mesh.get(
                "primitives",
                [],
            ):
                # glTF mode 4 == TRIANGLES
                if int(
                    primitive.get("mode", 4)
                ) != 4:
                    continue

                attributes = primitive.get(
                    "attributes",
                    {},
                )

                position_index = attributes.get(
                    "POSITION"
                )

                if position_index is None:
                    continue

                positions = read_accessor(
                    document,
                    buffers,
                    int(position_index),
                ).astype(np.float64)

                if positions.shape[1] != 3:
                    raise ValueError(
                        "POSITION must be VEC3"
                    )

                homogeneous = np.concatenate(
                    [
                        positions,
                        np.ones(
                            (len(positions), 1),
                            dtype=np.float64,
                        ),
                    ],
                    axis=1,
                )

                transformed = (
                    transform
                    @ homogeneous.T
                ).T[:, :3]

                if "indices" in primitive:
                    indices = read_accessor(
                        document,
                        buffers,
                        int(primitive["indices"]),
                    ).reshape(-1)
                else:
                    indices = np.arange(
                        len(transformed),
                        dtype=np.uint32,
                    )

                if len(indices) % 3 != 0:
                    raise ValueError(
                        "triangle index count "
                        "is not divisible by 3"
                    )

                triangle_blocks.append(
                    transformed[
                        indices
                    ].reshape((-1, 3, 3))
                )

        for child_index in node.get(
            "children",
            [],
        ):
            visit(
                int(child_index),
                transform,
            )

    scenes = document.get("scenes", [])

    if not scenes:
        raise ValueError(
            f"glTF has no scene: {path}"
        )

    scene_index = int(
        document.get("scene", 0)
    )

    for root_index in scenes[
        scene_index
    ].get("nodes", []):
        visit(
            int(root_index),
            np.eye(4, dtype=np.float64),
        )

    if not triangle_blocks:
        raise ValueError(
            f"glTF has no triangle primitives: {path}"
        )

    return np.concatenate(
        triangle_blocks,
        axis=0,
    )


def placement_matrix(
    placement: dict[str, Any],
) -> np.ndarray:
    position = np.asarray(
        placement["position"],
        dtype=np.float64,
    )

    scale_values = np.asarray(
        placement["scale"],
        dtype=np.float64,
    )

    if position.shape != (3,):
        raise ValueError(
            "placement position must have 3 values"
        )

    if scale_values.shape != (3,):
        raise ValueError(
            "placement scale must have 3 values"
        )

    translation = np.eye(
        4,
        dtype=np.float64,
    )

    translation[:3, 3] = position

    scale = np.eye(
        4,
        dtype=np.float64,
    )

    scale[0, 0] = scale_values[0]
    scale[1, 1] = scale_values[1]
    scale[2, 2] = scale_values[2]

    rotation = quaternion_matrix(
        placement["quaternion"]
    )

    return translation @ rotation @ scale


def apply_placement(
    triangles: np.ndarray,
    transform: np.ndarray,
) -> np.ndarray:
    vertices = triangles.reshape((-1, 3))

    homogeneous = np.concatenate(
        [
            vertices,
            np.ones(
                (len(vertices), 1),
                dtype=np.float64,
            ),
        ],
        axis=1,
    )

    transformed = (
        transform
        @ homogeneous.T
    ).T[:, :3]

    return transformed.reshape((-1, 3, 3))


def load_overlay_placements(
    path: Path,
) -> list[dict[str, Any]]:
    document = json.loads(
        path.read_text(encoding="utf-8")
    )

    placements = [
        placement
        for placement
        in document.get("placements", [])
        if (
            placement.get("visible") is True
            and placement.get("assetId")
            in OVERLAY_FLOOR_ASSETS
        )
    ]

    counts = {
        asset_id: sum(
            placement["assetId"] == asset_id
            for placement in placements
        )
        for asset_id in OVERLAY_FLOOR_ASSETS
    }

    if any(
        count != 2
        for count in counts.values()
    ):
        raise ValueError(
            "expected two overlay placements "
            f"per floor asset: {counts}"
        )

    return placements


def load_exact_placements(
    path: Path,
) -> dict[str, dict[str, Any]]:
    lines = path.read_text(
        encoding="utf-8"
    ).splitlines()

    if not lines:
        raise ValueError(
            "empty mapplacements file"
        )

    header = shlex.split(lines[0])

    if (
        len(header) != 4
        or header[0] != "LOSTARK_MAP_PLACEMENTS"
        or header[1] != "2"
    ):
        raise ValueError(
            "unsupported mapplacements header"
        )

    declared_count = int(header[3])
    parsed_count = 0

    # sourcePlacementId -> expected assetId
    required = {
        BOUNDS_SOURCE_PLACEMENT_ID:
            BOUNDS_ASSET_ID,

        CENTER_FLOOR_SOURCE_PLACEMENT_ID:
            CENTER_FLOOR_ASSET_ID,

        **{
            source_id: MAIN_FLOOR_ASSET_ID
            for source_id
            in MAIN_FLOOR_SOURCE_PLACEMENT_IDS
        },
    }

    found: dict[
        str,
        dict[str, Any],
    ] = {}

    for line_number, line in enumerate(
        lines[1:],
        2,
    ):
        if not line.strip():
            continue

        parsed_count += 1
        values = shlex.split(line)

        if len(values) != 16:
            raise ValueError(
                "invalid mapplacements line: "
                f"{line_number}"
            )

        source_id = values[1]

        if source_id not in required:
            continue

        asset_id = values[4]

        if asset_id != required[source_id]:
            raise ValueError(
                f"asset mismatch for {source_id}"
            )

        if source_id in found:
            raise ValueError(
                f"duplicate placement: {source_id}"
            )

        found[source_id] = {
            "position": [
                float(value)
                for value in values[5:8]
            ],
            "quaternion": [
                float(value)
                for value in values[8:12]
            ],
            "scale": [
                float(value)
                for value in values[12:15]
            ],
        }

    if parsed_count != declared_count:
        raise ValueError(
            "mapplacements count mismatch: "
            f"{parsed_count} != {declared_count}"
        )

    missing = sorted(
        set(required) - set(found)
    )

    if missing:
        raise ValueError(
            f"missing exact placements: {missing}"
        )

    return found


def rasterize(
    triangles: np.ndarray,
    bounds_min: np.ndarray,
    bounds_max: np.ndarray,
    cell_size: float,
    max_slope_degrees: float,
) -> tuple[
    float,
    float,
    int,
    int,
    np.ndarray,
    np.ndarray,
]:
    origin_x = (
        math.floor(
            float(bounds_min[0]) / cell_size
        )
        * cell_size
    )

    origin_z = (
        math.floor(
            float(bounds_min[2]) / cell_size
        )
        * cell_size
    )

    width = math.ceil(
        (
            float(bounds_max[0])
            - origin_x
        )
        / cell_size
    )

    height = math.ceil(
        (
            float(bounds_max[2])
            - origin_z
        )
        / cell_size
    )

    if width <= 0 or height <= 0:
        raise ValueError(
            "invalid NavGrid dimensions"
        )

    # CNavGrid::Load()의 현재 상한과 동일하다.
    if width * height > 1_000_000:
        raise ValueError(
            "NavGrid exceeds 1,000,000 cells"
        )

    walkable = np.zeros(
        (height, width),
        dtype=np.uint8,
    )

    heights = np.zeros(
        (height, width),
        dtype=np.float32,
    )

    minimum_normal_y = math.cos(
        math.radians(max_slope_degrees)
    )

    for triangle in triangles:
        edge_a = triangle[1] - triangle[0]
        edge_b = triangle[2] - triangle[0]

        normal = np.cross(
            edge_a,
            edge_b,
        )

        normal_length = float(
            np.linalg.norm(normal)
        )

        if normal_length <= 1e-12:
            continue

        # 메시 winding 방향과 무관하게 경사도만 본다.
        normal_y = (
            abs(float(normal[1]))
            / normal_length
        )

        if normal_y < minimum_normal_y:
            continue

        triangle_xz = triangle[:, [0, 2]]

        denominator = (
            (
                triangle_xz[1, 1]
                - triangle_xz[2, 1]
            )
            * (
                triangle_xz[0, 0]
                - triangle_xz[2, 0]
            )
            + (
                triangle_xz[2, 0]
                - triangle_xz[1, 0]
            )
            * (
                triangle_xz[0, 1]
                - triangle_xz[2, 1]
            )
        )

        if abs(float(denominator)) <= 1e-12:
            continue

        min_x = max(
            0,
            math.floor(
                (
                    float(
                        triangle_xz[:, 0].min()
                    )
                    - origin_x
                )
                / cell_size
            ),
        )

        max_x = min(
            width - 1,
            math.floor(
                (
                    float(
                        triangle_xz[:, 0].max()
                    )
                    - origin_x
                )
                / cell_size
            ),
        )

        min_z = max(
            0,
            math.floor(
                (
                    float(
                        triangle_xz[:, 1].min()
                    )
                    - origin_z
                )
                / cell_size
            ),
        )

        max_z = min(
            height - 1,
            math.floor(
                (
                    float(
                        triangle_xz[:, 1].max()
                    )
                    - origin_z
                )
                / cell_size
            ),
        )

        for cell_z in range(
            min_z,
            max_z + 1,
        ):
            sample_z = (
                origin_z
                + (cell_z + 0.5) * cell_size
            )

            if (
                sample_z < bounds_min[2]
                or sample_z > bounds_max[2]
            ):
                continue

            for cell_x in range(
                min_x,
                max_x + 1,
            ):
                sample_x = (
                    origin_x
                    + (cell_x + 0.5)
                    * cell_size
                )

                if (
                    sample_x < bounds_min[0]
                    or sample_x > bounds_max[0]
                ):
                    continue

                barycentric_a = (
                    (
                        triangle_xz[1, 1]
                        - triangle_xz[2, 1]
                    )
                    * (
                        sample_x
                        - triangle_xz[2, 0]
                    )
                    + (
                        triangle_xz[2, 0]
                        - triangle_xz[1, 0]
                    )
                    * (
                        sample_z
                        - triangle_xz[2, 1]
                    )
                ) / denominator

                barycentric_b = (
                    (
                        triangle_xz[2, 1]
                        - triangle_xz[0, 1]
                    )
                    * (
                        sample_x
                        - triangle_xz[2, 0]
                    )
                    + (
                        triangle_xz[0, 0]
                        - triangle_xz[2, 0]
                    )
                    * (
                        sample_z
                        - triangle_xz[2, 1]
                    )
                ) / denominator

                barycentric_c = (
                    1.0
                    - barycentric_a
                    - barycentric_b
                )

                if min(
                    barycentric_a,
                    barycentric_b,
                    barycentric_c,
                ) < -1e-7:
                    continue

                sample_y = float(
                    barycentric_a
                    * triangle[0, 1]
                    + barycentric_b
                    * triangle[1, 1]
                    + barycentric_c
                    * triangle[2, 1]
                )

                # 같은 XZ 셀에 여러 표면이 겹치면
                # 캐릭터가 서게 될 가장 높은 표면을 사용한다.
                if (
                    walkable[cell_z, cell_x] == 0
                    or sample_y
                    > heights[cell_z, cell_x]
                ):
                    walkable[cell_z, cell_x] = 1
                    heights[cell_z, cell_x] = sample_y

    return (
        origin_x,
        origin_z,
        width,
        height,
        walkable,
        heights,
    )


def write_navgrid(
    output: Path,
    width: int,
    height: int,
    cell_size: float,
    origin_x: float,
    origin_z: float,
    walkable: np.ndarray,
    heights: np.ndarray,
) -> None:
    payload = bytearray()

    # 현재 CNavGrid::Load() 계약:
    # uint32 width
    # uint32 height
    # float cellSize
    # float originX
    # float originZ
    payload.extend(
        struct.pack(
            "<IIfff",
            width,
            height,
            cell_size,
            origin_x,
            origin_z,
        )
    )

    # cell index = z * width + x
    payload.extend(
        walkable.astype(
            "<u1",
            copy=False,
        ).tobytes(order="C")
    )

    payload.extend(
        heights.astype(
            "<f4",
            copy=False,
        ).tobytes(order="C")
    )

    output.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    temporary = output.with_suffix(
        output.suffix + ".tmp"
    )

    try:
        temporary.write_bytes(payload)
        temporary.replace(output)
    finally:
        if temporary.exists():
            temporary.unlink()


def main() -> int:
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--floor01",
        required=True,
        type=Path,
    )

    parser.add_argument(
        "--floor01a",
        required=True,
        type=Path,
    )

    parser.add_argument(
        "--floor01b",
        required=True,
        type=Path,
    )

    parser.add_argument(
        "--main-floor",
        required=True,
        type=Path,
    )

    parser.add_argument(
        "--center-floor",
        required=True,
        type=Path,
    )

    parser.add_argument(
        "--overlay",
        required=True,
        type=Path,
    )

    parser.add_argument(
        "--mapplacements",
        required=True,
        type=Path,
    )

    parser.add_argument(
        "--bounds-gltf",
        required=True,
        type=Path,
    )

    parser.add_argument(
        "--cell-size",
        type=float,
        default=0.5,
    )

    parser.add_argument(
        "--max-slope",
        type=float,
        default=45.0,
    )

    parser.add_argument(
        "--output",
        required=True,
        type=Path,
    )

    args = parser.parse_args()

    input_paths = [
        args.floor01,
        args.floor01a,
        args.floor01b,
        args.main_floor,
        args.center_floor,
        args.overlay,
        args.mapplacements,
        args.bounds_gltf,
    ]

    missing_inputs = [
        path
        for path in input_paths
        if not path.is_file()
    ]

    if missing_inputs:
        parser.error(
            "input files do not exist: "
            + ", ".join(
                str(path)
                for path in missing_inputs
            )
        )

    if (
        not math.isfinite(args.cell_size)
        or args.cell_size <= 0
    ):
        parser.error(
            "cell-size must be positive"
        )

    if (
        not math.isfinite(args.max_slope)
        or not 0 <= args.max_slope < 90
    ):
        parser.error(
            "max-slope must be in [0, 90)"
        )

    source_paths = {
        "BG_RAD_VALTAN_FLOOR01_SM":
            args.floor01,

        "BG_RAD_VALTAN_FLOOR01A_SM":
            args.floor01a,

        "BG_RAD_VALTAN_FLOOR01B_SM":
            args.floor01b,

        MAIN_FLOOR_ASSET_ID:
            args.main_floor,

        CENTER_FLOOR_ASSET_ID:
            args.center_floor,
    }

    source_triangles = {
        asset_id: load_triangles(path)
        for asset_id, path
        in source_paths.items()
    }

    world_triangle_blocks: list[
        np.ndarray
    ] = []

    # 파괴 가능한 외곽 Floor01/A/B.
    for placement in load_overlay_placements(
        args.overlay
    ):
        asset_id = placement["assetId"]

        world_triangle_blocks.append(
            apply_placement(
                source_triangles[asset_id],
                placement_matrix(placement),
            )
        )

    exact_placements = load_exact_placements(
        args.mapplacements
    )

    # 아레나 본체 4분할.
    for source_id in (
        MAIN_FLOOR_SOURCE_PLACEMENT_IDS
    ):
        world_triangle_blocks.append(
            apply_placement(
                source_triangles[
                    MAIN_FLOOR_ASSET_ID
                ],
                placement_matrix(
                    exact_placements[source_id]
                ),
            )
        )

    # 중앙 원형 캡.
    world_triangle_blocks.append(
        apply_placement(
            source_triangles[
                CENTER_FLOOR_ASSET_ID
            ],
            placement_matrix(
                exact_placements[
                    CENTER_FLOOR_SOURCE_PLACEMENT_ID
                ]
            ),
        )
    )

    floor_triangles = np.concatenate(
        world_triangle_blocks,
        axis=0,
    )

    # CUL_BOX는 범위만 계산한다.
    bounds_source = load_triangles(
        args.bounds_gltf
    )

    bounds_world = apply_placement(
        bounds_source,
        placement_matrix(
            exact_placements[
                BOUNDS_SOURCE_PLACEMENT_ID
            ]
        ),
    )

    bounds_min = bounds_world.min(
        axis=(0, 1)
    )

    bounds_max = bounds_world.max(
        axis=(0, 1)
    )

    (
        origin_x,
        origin_z,
        width,
        height,
        walkable,
        heights,
    ) = rasterize(
        floor_triangles,
        bounds_min,
        bounds_max,
        args.cell_size,
        args.max_slope,
    )

    walkable_count = int(
        walkable.sum()
    )

    if walkable_count == 0:
        raise ValueError(
            "bake produced no walkable cells"
        )

    write_navgrid(
        args.output,
        width,
        height,
        args.cell_size,
        origin_x,
        origin_z,
        walkable,
        heights,
    )

    walkable_heights = heights[
        walkable != 0
    ]

    result = {
        "width": width,
        "height": height,
        "cellSize": args.cell_size,
        "origin": [
            origin_x,
            origin_z,
        ],
        "boundsMin": bounds_min.tolist(),
        "boundsMax": bounds_max.tolist(),
        "triangleCount": int(
            len(floor_triangles)
        ),
        "walkableCells": walkable_count,
        "minWalkableHeight": float(
            walkable_heights.min()
        ),
        "maxWalkableHeight": float(
            walkable_heights.max()
        ),
        "output": str(
            args.output.resolve()
        ),
    }

    print(
        json.dumps(
            result,
            ensure_ascii=False,
            indent=2,
        )
    )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

