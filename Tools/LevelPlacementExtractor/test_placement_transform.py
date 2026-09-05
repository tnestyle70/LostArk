from __future__ import annotations

import unittest

import numpy as np

try:
    from .placement_transform import (
        apply_placement,
        placement_matrix,
        quaternion_matrix,
    )
except ImportError:
    from placement_transform import (
        apply_placement,
        placement_matrix,
        quaternion_matrix,
    )


class PlacementTransformTests(unittest.TestCase):
    def test_quaternion_is_normalized_before_rotation(self) -> None:
        unit = quaternion_matrix((0.0, 0.0, 0.0, 1.0))
        scaled = quaternion_matrix((0.0, 0.0, 0.0, 5.0))
        np.testing.assert_allclose(scaled, unit)

    def test_zero_or_non_finite_quaternion_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "invalid quaternion"):
            quaternion_matrix((0.0, 0.0, 0.0, 0.0))
        with self.assertRaisesRegex(ValueError, "invalid quaternion"):
            quaternion_matrix((0.0, 0.0, 0.0, float("inf")))

    def test_placement_matrix_applies_scale_rotation_then_translation(self) -> None:
        matrix = placement_matrix(
            {
                "position": (10.0, 20.0, 30.0),
                "quaternion": (0.0, 0.0, 0.0, 1.0),
                "scale": (2.0, 3.0, 4.0),
            }
        )
        point = matrix @ np.asarray((1.0, 1.0, 1.0, 1.0))
        np.testing.assert_allclose(point, (12.0, 23.0, 34.0, 1.0))

    def test_placement_shape_is_fail_closed(self) -> None:
        baseline = {
            "position": (0.0, 0.0, 0.0),
            "quaternion": (0.0, 0.0, 0.0, 1.0),
            "scale": (1.0, 1.0, 1.0),
        }
        with self.assertRaisesRegex(ValueError, "position must have 3 values"):
            placement_matrix({**baseline, "position": (0.0, 0.0)})
        with self.assertRaisesRegex(ValueError, "scale must have 3 values"):
            placement_matrix({**baseline, "scale": (1.0, 1.0)})

    def test_apply_placement_preserves_triangle_shape(self) -> None:
        triangles = np.asarray(
            [[[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], [0.0, 0.0, 1.0]]],
            dtype=np.float64,
        )
        transform = placement_matrix(
            {
                "position": (2.0, 3.0, 4.0),
                "quaternion": (0.0, 0.0, 0.0, 1.0),
                "scale": (1.0, 1.0, 1.0),
            }
        )
        transformed = apply_placement(triangles, transform)
        self.assertEqual(transformed.shape, (1, 3, 3))
        np.testing.assert_allclose(
            transformed,
            [[[2.0, 3.0, 4.0], [3.0, 3.0, 4.0], [2.0, 3.0, 5.0]]],
        )


if __name__ == "__main__":
    unittest.main()
