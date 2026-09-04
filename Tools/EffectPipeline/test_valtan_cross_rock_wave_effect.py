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
CATALOG_PATH = REPOSITORY_ROOT / "Data/Effects/EffectCatalog.json"
PRODUCT_CUES_PATH = (
    REPOSITORY_ROOT
    / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
)
STONE_MODEL = "Effect/Valtan/Meshes/FX_SM_00/fm_d_stoneparts_003.wmodel"
# Project-tuned 2026-09-04 in the Effect Tool: the cross stones share the
# ground-roar rock base so both rock families read as the same material.
STONE_BASE = "Effect/Valtan/Textures/FX_TEX_05/fx_k_turtlespec_01.dds"
STONE_NOISE = "Effect/Valtan/Textures/FX_TEX_02/fx_d_stoneparts_002.dds"
STONE_MASK = "Effect/Valtan/Textures/FX_TEX_02/fx_d_fluid_020.dds"
STONE_DISSOLVE = "Effect/Valtan/Textures/FX_TEX_04/fx_h_noise_001.dds"
STONE_MATERIAL = "fx_m_mi_n_00.fx_mi.fx_n_me_dissolve_01_04_ma"
SMOKE_BASE = "Effect/Valtan/Textures/FX_TEX_03/fx_e_atypical_005_cl.dds"
SMOKE_NOISE = "Effect/Valtan/Textures/FX_TEX_01/fx_c_noise_008.dds"
SMOKE_MASK = "Effect/Valtan/Textures/FX_TEX_03/fx_e_noise_002.dds"
SMOKE_MATERIAL = "fx_m_mi_03.fx_mi.fx_d_pa_turbulence_01_13_tr"
PRODUCT_CUE_ID = "cue.valtan.sequence.cross.step-01"
FIXED_STEP_HZ = 60
WORLD_SCALE = 1.5
AUTHORING_HEADER_PATH = REPOSITORY_ROOT / "Client/Public/Effect_AuthoringDocument.h"
CODEC_PATH = REPOSITORY_ROOT / "Client/Private/Effect_DocumentCodec.cpp"
PLAYBACK_PATH = REPOSITORY_ROOT / "Client/Private/Effect_Playback.cpp"
TOOL_PATH = REPOSITORY_ROOT / "Client/Private/Effect_Tool.cpp"
EXPECTED_DIRECTIONS = {
    (1.0, 0.0),
    (0.0, 1.0),
    (-1.0, 0.0),
    (0.0, -1.0),
}


def read_json(path: Path) -> dict[str, object]:
    return json.loads(path.read_text(encoding="utf-8"))


class ValtanCrossRockWaveEffectTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.effect = read_json(EFFECT_PATH)
        cls.ownership = read_json(OWNERSHIP_PATH)
        cls.presentation = read_json(PRESENTATION_PATH)
        cls.catalog = read_json(CATALOG_PATH)
        cls.product_cues = read_json(PRODUCT_CUES_PATH)

    def test_effect_is_product_catalog_owned_and_absent_from_draft_sidecar(self) -> None:
        self.assertEqual(self.effect["schema"], "lostark.effect-authoring")
        self.assertEqual(self.effect["version"], 13)
        self.assertEqual(self.effect["effectAssetId"], EFFECT_ID)
        self.assertEqual(
            [],
            [
                row for row in self.ownership["bindings"]
                if row["effectAssetId"] == EFFECT_ID
            ],
        )
        registered = [
            row for row in self.catalog["effects"]
            if row["effectAssetId"] == EFFECT_ID
        ]
        self.assertEqual(
            [{
                "effectAssetId": EFFECT_ID,
                "payloadKind": "DIRECT_AUTHORED_DOCUMENT",
                "authoringPath": f"Effects/Authored/{EFFECT_ID}.effect.json",
            }],
            registered,
        )

    def test_four_stone_and_four_smoke_emitters_cover_each_axis_once(self) -> None:
        elements = self.effect["elements"]
        self.assertEqual(len(elements), 8)
        self.assertEqual(len({element["id"] for element in elements}), 8)
        stone_elements = [
            element for element in elements
            if any(
                row["slotId"] == "meshModel"
                for row in element["resources"]
            )
        ]
        smoke_elements = [
            element for element in elements
            if all(
                row["slotId"] != "meshModel"
                for row in element["resources"]
            )
        ]
        self.assertEqual(len(stone_elements), 4)
        self.assertEqual(len(smoke_elements), 4)

        for expected_group, family in (
            ("valtan.cross.rock-wave", stone_elements),
            ("valtan.cross.smoke-wave", smoke_elements),
        ):
            observed_directions: set[tuple[float, float]] = set()
            for element in family:
                self.assertEqual(element["kind"], "particle")
                self.assertEqual(element["groupId"], expected_group)
                detail = element["detail"]
                transform = detail["transform"]
                particle = detail["particle"]
                position_x, _, position_z = transform["position"]
                velocity_x, _, velocity_z = transform["velocityPerSecond"]
                speed = math.hypot(velocity_x, velocity_z)
                self.assertGreater(speed, 0.0)
                direction = (
                    round(velocity_x / speed, 6),
                    round(velocity_z / speed, 6),
                )
                self.assertIn(direction, EXPECTED_DIRECTIONS)
                observed_directions.add(direction)
                self.assertTrue(math.isfinite(position_x))
                self.assertTrue(math.isfinite(position_z))
                self.assertGreater(
                    position_x * velocity_x + position_z * velocity_z,
                    0.0,
                )
                self.assertTrue(
                    all(value > 0.0 for value in transform["scale"])
                )
                self.assertFalse(particle["localSpace"])
                self.assertEqual(
                    particle["initialPositionMin"], [0.0, 0.0, 0.0]
                )
                self.assertEqual(
                    particle["initialPositionMax"], [0.0, 0.0, 0.0]
                )
                self.assertEqual(
                    particle["initialVelocityMin"], [0.0, 0.0, 0.0]
                )
                self.assertEqual(
                    particle["initialVelocityMax"], [0.0, 0.0, 0.0]
                )
                self.assertEqual(
                    particle["acceleration"], [0.0, 0.0, 0.0]
                )

            self.assertEqual(observed_directions, EXPECTED_DIRECTIONS)

    def test_smoke_emitters_reuse_each_stone_motion_lattice_exactly(self) -> None:
        by_group: dict[str, dict[tuple[float, float], dict[str, object]]] = {}
        for element in self.effect["elements"]:
            velocity_x, _, velocity_z = element["detail"]["transform"][
                "velocityPerSecond"
            ]
            speed = math.hypot(velocity_x, velocity_z)
            direction = (
                round(velocity_x / speed, 6),
                round(velocity_z / speed, 6),
            )
            by_group.setdefault(element["groupId"], {})[direction] = element

        stones = by_group["valtan.cross.rock-wave"]
        smoke = by_group["valtan.cross.smoke-wave"]
        self.assertEqual(set(stones), EXPECTED_DIRECTIONS)
        self.assertEqual(set(smoke), EXPECTED_DIRECTIONS)
        for direction in EXPECTED_DIRECTIONS:
            stone_detail = stones[direction]["detail"]
            smoke_detail = smoke[direction]["detail"]
            self.assertEqual(
                smoke_detail["transform"], stone_detail["transform"]
            )
            self.assertEqual(
                smoke_detail["timing"]["startDelaySeconds"],
                stone_detail["timing"]["startDelaySeconds"],
            )
            self.assertEqual(
                smoke_detail["timing"]["lifeTimeSeconds"],
                stone_detail["timing"]["lifeTimeSeconds"],
            )
            self.assertEqual(
                stone_detail["timing"]["transformMotionDurationSeconds"],
                stone_detail["timing"]["lifeTimeSeconds"],
            )
            # The codec omits transformMotionDurationSeconds when it is 0, so a
            # Tool save drops the key on smoke; omission is the zero identity.
            self.assertEqual(
                smoke_detail["timing"].get("transformMotionDurationSeconds", 0.0),
                0.0,
            )
            for field in (
                "maxParticles",
                "spawnRatePerSecond",
                "fixedCenterSpacingWorldUnits",
                "burstCount",
                "lifeTimeSeconds",
                "initialPositionMin",
                "initialPositionMax",
                "initialVelocityMin",
                "initialVelocityMax",
                "acceleration",
                "startSize",
                "endSize",
                "localSpace",
            ):
                self.assertEqual(
                    smoke_detail["particle"][field],
                    stone_detail["particle"][field],
                    field,
                )
            self.assertFalse(stone_detail["particle"]["billboard"])
            self.assertTrue(smoke_detail["particle"]["billboard"])
            self.assertTrue(smoke_detail["sprite"]["billboard"])

    def test_each_moving_emitter_materializes_a_bounded_near_to_far_row(self) -> None:
        spacings = {
            element["detail"]["particle"]["fixedCenterSpacingWorldUnits"]
            for element in self.effect["elements"]
        }
        self.assertEqual(len(spacings), 1)
        center_spacing = next(iter(spacings))
        self.assertGreater(center_spacing, 0.0)

        for element in self.effect["elements"]:
            detail = element["detail"]
            timing = detail["timing"]
            particle = detail["particle"]
            has_mesh_model = any(
                row["slotId"] == "meshModel"
                for row in element["resources"]
            )
            self.assertEqual(0.0, timing["startDelaySeconds"])
            self.assertEqual(0.5, timing["lifeTimeSeconds"])
            if has_mesh_model:
                self.assertEqual(
                    timing["transformMotionDurationSeconds"],
                    timing["lifeTimeSeconds"],
                )
            else:
                self.assertEqual(
                    0.0, timing.get("transformMotionDurationSeconds", 0.0)
                )
            self.assertEqual(0, particle["burstCount"])
            self.assertEqual(0.0, particle["spawnRatePerSecond"])
            self.assertEqual(
                center_spacing,
                particle["fixedCenterSpacingWorldUnits"],
            )
            self.assertGreater(particle["maxParticles"], 1)
            self.assertGreater(
                min(particle["lifeTimeSeconds"]),
                timing["lifeTimeSeconds"],
            )

            # The opt-in runtime path does not use time-rate spawning.  It
            # samples the moving emitter in world space, emits the first point,
            # then interpolates every exact distance crossing between fixed
            # ticks.  This keeps the centre lattice exact at any frame rate.
            emission_steps = round(
                timing["lifeTimeSeconds"] * FIXED_STEP_HZ
            )
            authored_radius = math.hypot(
                detail["transform"]["position"][0],
                detail["transform"]["position"][2],
            )
            authored_speed = math.hypot(
                detail["transform"]["velocityPerSecond"][0],
                detail["transform"]["velocityPerSecond"][2],
            )
            sampled_world_radii = [
                (authored_radius + authored_speed * step / FIXED_STEP_HZ)
                * WORLD_SCALE
                for step in range(1, emission_steps + 1)
            ]
            world_radii = [sampled_world_radii[0]]
            residual = 0.0
            previous = sampled_world_radii[0]
            for current in sampled_world_radii[1:]:
                segment = current - previous
                travel = residual + segment
                crossing_count = math.floor(
                    (travel + 1.0e-9) / center_spacing
                )
                first_distance = center_spacing - residual
                for crossing in range(crossing_count):
                    if len(world_radii) >= particle["maxParticles"]:
                        break
                    world_radii.append(
                        previous
                        + first_distance
                        + crossing * center_spacing
                    )
                residual = math.fmod(travel, center_spacing)
                if (
                    residual < 1.0e-9
                    or center_spacing - residual < 1.0e-9
                ):
                    residual = 0.0
                previous = current

            self.assertGreater(len(world_radii), 1)
            self.assertLessEqual(len(world_radii), particle["maxParticles"])
            for previous, current in zip(world_radii, world_radii[1:]):
                self.assertAlmostEqual(
                    center_spacing,
                    current - previous,
                    places=6,
                )
            self.assertEqual(particle["startSize"], [1.0, 1.0])
            self.assertEqual(particle["endSize"], [1.0, 1.0])
            self.assertLessEqual(
                timing["startDelaySeconds"]
                + timing["lifeTimeSeconds"]
                + max(particle["lifeTimeSeconds"]),
                3.0,
            )

            resources = {
                row["slotId"]: row["assetId"] for row in element["resources"]
            }
            if "meshModel" in resources:
                self.assertEqual(resources["meshModel"], STONE_MODEL)
                self.assertEqual(resources["base"], STONE_BASE)
                self.assertEqual(resources["noise"], STONE_NOISE)
                self.assertEqual(resources["mask"], STONE_MASK)
                self.assertNotIn("emissive", resources)
                self.assertEqual(resources["dissolve"], STONE_DISSOLVE)
                self.assertAlmostEqual(
                    detail["mesh"]["modelPreScale"], 0.01
                )
                # Tool saves round-trip through float32 (0.649999976).
                self.assertAlmostEqual(
                    detail["timing"]["dissolveStartNormalized"], 0.65, places=6
                )
                self.assertFalse(detail["particle"]["billboard"])
                self.assertEqual(
                    element["material"]["renderProfile"],
                    "opaque_back_depth_write",
                )
                self.assertEqual(
                    element["material"]["sourceMaterialPath"],
                    STONE_MATERIAL,
                )
            else:
                self.assertEqual(
                    resources,
                    {
                        "base": SMOKE_BASE,
                        "noise": SMOKE_NOISE,
                        "mask": SMOKE_MASK,
                    },
                )
                self.assertEqual(
                    detail["timing"]["dissolveStartNormalized"], 1.0
                )
                self.assertTrue(detail["particle"]["billboard"])
                self.assertTrue(detail["sprite"]["billboard"])
                self.assertEqual(
                    element["material"]["renderProfile"],
                    "alpha_two_sided_depth_read",
                )
                self.assertEqual(
                    element["material"]["sourceMaterialPath"],
                    SMOKE_MATERIAL,
                )
            self.assertNotIn("emissiveColor", detail["color"])
            self.assertEqual(detail["color"]["emissiveIntensity"], 1.0)
            self.assertFalse(detail["linearLerp"]["colorMultiply"])

    def test_fixed_spacing_is_persisted_and_consumed_by_cpu_playback(self) -> None:
        header = AUTHORING_HEADER_PATH.read_text(encoding="utf-8")
        codec = CODEC_PATH.read_text(encoding="utf-8")
        playback = PLAYBACK_PATH.read_text(encoding="utf-8")
        tool = TOOL_PATH.read_text(encoding="utf-8")

        self.assertIn("fFixedCenterSpacingWorldUnits", header)
        self.assertIn('"fixedCenterSpacingWorldUnits"', codec)
        self.assertIn("Spawn_FixedCenterSpacingParticles", playback)
        self.assertIn("fFixedCenterSpacingResidualWorldUnits", playback)
        self.assertIn("Fixed Center Spacing (world m)", tool)

    def test_product_cue_owns_the_only_impact_delay_and_gameplay_footprint(self) -> None:
        pattern = next(
            row
            for row in self.presentation["patterns"]
            if row["patternId"] == "VALTAN_CROSS"
        )
        stage = pattern["stages"][0]
        self.assertEqual("STEP_01", stage["stageId"])
        self.assertEqual(
            [{
                "cueId": PRODUCT_CUE_ID,
                "scalePolicy": {
                    "kind": "GAMEPLAY_FOOTPRINT",
                    "worldScale": [1.5, 1.5, 1.5],
                },
                "occurrenceId": f"{PRODUCT_CUE_ID}.occurrence.01",
                "effectAssetId": EFFECT_ID,
                "clipOccurrenceId": "valtan.sequence.cross.step-01.clip-01",
                "sourceStartMs": 1617,
                "sourceEndMs": None,
                "anchorSlotId": "root",
                "followPolicy": "snapshot",
                "stopPolicy": "natural",
                "repeatPolicy": "once",
                "localTransform": {
                    "position": [0.0, 0.0, 0.0],
                    "rotationDegrees": [0.0, 0.0, 0.0],
                    "scale": [1.0, 1.0, 1.0],
                },
                "mappingBasis": "PROJECT_AUTHORED",
            }],
            stage["effectCues"],
        )
        self.assertTrue(all(
            element["detail"]["timing"]["startDelaySeconds"] == 0.0
            for element in self.effect["elements"]
        ))

        product = [
            row for row in self.product_cues["cues"]
            if row["bindingId"] == PRODUCT_CUE_ID
        ]
        self.assertEqual(1, len(product))
        self.assertEqual("VALTAN_CROSS", product[0]["patternId"])
        self.assertEqual("STEP_01", product[0]["stageId"])
        self.assertEqual("valtan.sequence.cross.step-01", product[0]["actionId"])
        self.assertEqual(1617, product[0]["sourceStartMs"])
        self.assertEqual("root", product[0]["anchorSlotId"])
        self.assertEqual("snapshot", product[0]["followPolicy"])
        self.assertEqual("natural", product[0]["stopPolicy"])
        self.assertEqual("once", product[0]["repeatPolicy"])
        self.assertEqual(
            {"kind": "GAMEPLAY_FOOTPRINT", "worldScale": [1.5, 1.5, 1.5]},
            product[0]["scalePolicy"],
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
