from __future__ import annotations

from pathlib import Path
import sys
import unittest

import numpy as np


SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

import build_kouku_saydon_world_authoring as builder


class KoukuSaydonWorldAuthoringTests(unittest.TestCase):
    def test_rasterizes_only_slope_admitted_floor(self) -> None:
        floor = np.asarray(
            [
                [[0.0, 2.0, 0.0], [2.0, 2.0, 0.0], [2.0, 2.0, 2.0]],
                [[0.0, 2.0, 0.0], [2.0, 2.0, 2.0], [0.0, 2.0, 2.0]],
                [[3.0, 0.0, 0.0], [3.0, 2.0, 0.0], [3.0, 2.0, 2.0]],
            ],
            dtype=np.float64,
        )
        grid = builder.GridSpec(0.0, 0.0, 4, 2, 1.0)
        resolved, heights = builder.rasterize_floor_triangles(floor, grid, 45.0)
        self.assertEqual(4, int(resolved.sum()))
        self.assertTrue(np.allclose(heights[resolved != 0], 2.0))

    def test_largest_component_respects_runtime_step(self) -> None:
        resolved = np.asarray([[1, 1, 1, 0, 1, 1]], dtype=np.uint8)
        heights = np.asarray([[0.0, 0.0, 2.0, 0.0, 4.0, 4.0]], dtype=np.float32)
        component = builder.largest_component(resolved, heights, 1.0)
        self.assertEqual([(0, 0), (1, 0)], component)

    def test_representative_cells_are_deterministic_and_separated(self) -> None:
        component = [(x, z) for z in range(5) for x in range(5)]
        first = builder.representative_cells(component, 4)
        second = builder.representative_cells(list(reversed(component)), 4)
        self.assertEqual(first, second)
        for index, cell in enumerate(first):
            for other in first[:index]:
                self.assertGreaterEqual(
                    abs(cell[0] - other[0]) + abs(cell[1] - other[1]), 2
                )

    def test_stage_document_does_not_infer_mario_or_gate(self) -> None:
        waypoint = builder.StageWaypoint(
            source_level_id="LV_LUT_MIDNIGHTC_ED_SL02",
            placement_id="stage.kakul.sl02",
            cell_x=1,
            cell_z=2,
            x=4.0,
            y=3.0,
            z=8.0,
            component_cells=12,
        )
        text = builder.serialize_stage_markers([waypoint])
        self.assertIn('"semanticStatus": "SOURCE_LEVEL_ID_ONLY"', text)
        self.assertIn('"sourceLevelId": "LV_LUT_MIDNIGHTC_ED_SL02"', text)
        self.assertNotIn('"Mario', text)
        self.assertNotIn('"Gate', text)

    def test_world_uses_stable_spawns_and_disabled_stage_waypoints(self) -> None:
        waypoint = builder.StageWaypoint(
            source_level_id="LV_LUT_MIDNIGHTC_ED_SL01",
            placement_id="stage.kakul.sl01",
            cell_x=1,
            cell_z=2,
            x=4.0,
            y=3.0,
            z=8.0,
            component_cells=12,
        )
        text = builder.serialize_world(
            [waypoint],
            [(0.0, 1.0, 0.0), (4.0, 1.0, 0.0), (0.0, 1.0, 4.0), (4.0, 1.0, 4.0)],
        )
        self.assertEqual(4, text.count('"enabled": true'))
        self.assertEqual(1, text.count('"enabled": false'))
        self.assertIn('"placementId": "stage.kakul.sl01"', text)
        self.assertNotIn("random", text.casefold())


if __name__ == "__main__":
    unittest.main()
