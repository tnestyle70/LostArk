"""Pure placement-to-world transform helpers shared by map authoring tools."""

from __future__ import annotations

import math
from collections.abc import Mapping, Sequence
from typing import Any

import numpy as np


def quaternion_matrix(quaternion: Sequence[float]) -> np.ndarray:
    x, y, z, w = (float(value) for value in quaternion)
    length = math.sqrt(x * x + y * y + z * z + w * w)
    if not math.isfinite(length) or length <= 1e-12:
        raise ValueError("invalid quaternion")

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


def placement_matrix(placement: Mapping[str, Any]) -> np.ndarray:
    position = np.asarray(placement["position"], dtype=np.float64)
    scale_values = np.asarray(placement["scale"], dtype=np.float64)
    if position.shape != (3,):
        raise ValueError("placement position must have 3 values")
    if scale_values.shape != (3,):
        raise ValueError("placement scale must have 3 values")

    translation = np.eye(4, dtype=np.float64)
    translation[:3, 3] = position

    scale = np.eye(4, dtype=np.float64)
    scale[0, 0] = scale_values[0]
    scale[1, 1] = scale_values[1]
    scale[2, 2] = scale_values[2]

    rotation = quaternion_matrix(placement["quaternion"])
    return translation @ rotation @ scale


def apply_placement(triangles: np.ndarray, transform: np.ndarray) -> np.ndarray:
    vertices = triangles.reshape((-1, 3))
    homogeneous = np.concatenate(
        [vertices, np.ones((len(vertices), 1), dtype=np.float64)],
        axis=1,
    )
    transformed = (transform @ homogeneous.T).T[:, :3]
    return transformed.reshape((-1, 3, 3))
