from __future__ import annotations

import json
import math
import unittest
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
EFFECT_ID = "effect.valtan.sequence.cross"
EFFECT_PATH = (
    REPOSITORY_ROOT / f"Data/Effects/Authored/{EFFECT_ID}.effect.json"
)
OWNERSHIP_PATH = (
    REPOSITORY_ROOT / "Data/Effects/ValtanPatternAuthoringEffects.json"
)
PRESENTATION_PATH = REPOSITORY_ROOT / "Data/Valtan/Valtan.presentation.json"
STONE_MODEL = "Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel"
STONE_MATERIAL = "fx_m_mi_05.fx_mi.fx_e_me_ht_03_4_ma"
EXPECTED_DIRECTIONS = {
    0.0: (1.0, 0.0),
    90.0: (0.0, 1.0),
    180.0: (-1.0, 0.0),
    270.0: (0.0, -1.0),
}


def read_json(path: Path) -> dict[str, object]:
    return json.loads(path.read_text(encoding="utf-8"))


class ValtanCrossRockWaveEffectTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.effect = read_json(EFFECT_PATH)
        cls.ownership = read_json(OWNERSHIP_PATH)
        cls.presentation = read_json(PRESENTATION_PATH)

    def test_effect_is_the_exact_valtan_cross_draft_owner(self) -> None:
        self.assertEqual(self.effect["schema"], "lostark.effect-authoring")
        self.assertEqual(self.effect["version"], 13)
        self.assertEqual(self.effect["effectAssetId"], EFFECT_ID)
        self.assertEqual(
            self.ownership["bindings"],
            [
                {
                    "patternId": "VALTAN_CROSS",
                    "effectAssetId": EFFECT_ID,
                    "authoringPath": f"Effects/Authored/{EFFECT_ID}.effect.json",
                    "state": "DRAFT_ATTACHED",
                }
            ],
        )

    def test_four_world_space_emitters_cover_each_axis_once(self) -> None:
        elements = self.effect["elements"]
        self.assertEqual(len(elements), 4)
        observed_angles: set[float] = set()
        for element in elements:
            detail = element["detail"]
            transform = detail["transform"]
            particle = detail["particle"]
            angle = float(transform["rotationDegrees"][1])
            observed_angles.add(angle)
            self.assertIn(angle, EXPECTED_DIRECTIONS)

            direction_x, direction_z = EXPECTED_DIRECTIONS[angle]
            position_x, _, position_z = transform["position"]
            velocity_x, _, velocity_z = transform["velocityPerSecond"]
            radius = math.hypot(position_x, position_z)
            speed = math.hypot(velocity_x, velocity_z)
            self.assertGreater(radius, 0.0)
            self.assertGreater(speed, 0.0)
            self.assertAlmostEqual(position_x / radius, direction_x, places=6)
            self.assertAlmostEqual(position_z / radius, direction_z, places=6)
            self.assertAlmostEqual(velocity_x / speed, direction_x, places=6)
            self.assertAlmostEqual(velocity_z / speed, direction_z, places=6)
            self.assertFalse(particle["localSpace"])
            self.assertEqual(particle["initialVelocityMin"], [0.0, 0.0, 0.0])
            self.assertEqual(particle["initialVelocityMax"], [0.0, 0.0, 0.0])

        self.assertEqual(observed_angles, set(EXPECTED_DIRECTIONS))
        self.assertNotIn(360.0, observed_angles)

    def test_each_moving_emitter_materializes_a_bounded_near_to_far_row(self) -> None:
        for element in self.effect["elements"]:
            detail = element["detail"]
            timing = detail["timing"]
            particle = detail["particle"]
            self.assertAlmostEqual(timing["startDelaySeconds"], 1.617, places=6)
            self.assertEqual(
                timing["transformMotionDurationSeconds"],
                timing["lifeTimeSeconds"],
            )
            self.assertEqual(particle["burstCount"], 1)
            self.assertGreater(particle["spawnRatePerSecond"], 0.0)
            self.assertGreater(particle["maxParticles"], 1)
            available_births = (
                timing["lifeTimeSeconds"] * particle["spawnRatePerSecond"]
                + particle["burstCount"]
            )
            self.assertGreaterEqual(available_births, particle["maxParticles"])
            self.assertEqual(particle["endSize"], [0.0, 0.0])
            self.assertLessEqual(
                timing["startDelaySeconds"]
                + timing["lifeTimeSeconds"]
                + max(particle["lifeTimeSeconds"]),
                3.0,
            )

            resources = {
                row["slotId"]: row["assetId"] for row in element["resources"]
            }
            self.assertEqual(resources["meshModel"], STONE_MODEL)
            self.assertEqual(detail["mesh"]["modelPreScale"], 0.01)
            self.assertEqual(
                element["material"]["sourceMaterialPath"], STONE_MATERIAL
            )

    def test_draft_does_not_claim_product_cue_or_server_damage(self) -> None:
        pattern = next(
            row
            for row in self.presentation["patterns"]
            if row["patternId"] == "VALTAN_CROSS"
        )
        self.assertEqual(pattern["stages"][0]["effectCues"], [])


if __name__ == "__main__":
    unittest.main(verbosity=2)
