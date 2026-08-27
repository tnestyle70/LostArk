#!/usr/bin/env python3
"""Focused contract tests for the 2026-08-27 requested Valtan effect package."""

from __future__ import annotations

import json
import subprocess
import sys
import unittest
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
AUTHORED_ROOT = ROOT / "Data" / "Effects" / "Authored"
GENERATED_PREFIX = "requested.20260827."

SECTOR_04 = "Effect/Valtan/Textures/FX_TEX_05/fx_o_sector_04.dds"
SECTOR_05 = "Effect/Valtan/Textures/FX_TEX_05/fx_o_sector_05.dds"
FLOOR_ELECTRIC = "Effect/Valtan/Textures/FX_TEX_02/fx_d_electric_013_ycl.dds"
FLOOR_RING = "Effect/Valtan/Textures/FX_TEX_05/fx_m_ring_001.dds"
FLOOR_ELECTILE = "Effect/Valtan/Textures/FX_TEX_05/fx_k_electile_02.dds"
FLOOR_NOISE = "Effect/Valtan/Textures/FX_TEX_02/fx_d_noise_002.dds"
FLOOR_EMISSIVE_NOISE = "Effect/Valtan/Textures/FX_TEX_05/fx_m_noise_001.dds"

PRODUCT_ELEMENTS: dict[str, tuple[int, set[str]]] = {
    "effect.valtan.project-tuned.terrain-destruction-3.semicircle": (
        5,
        {
            f"{GENERATED_PREFIX}terrain-3.semicircle.sector-{index:02d}"
            for index in range(1, 4)
        }
        | {
            f"{GENERATED_PREFIX}terrain-3.landing.{index:02d}"
            for index in range(1, 3)
        },
    ),
    "effect.valtan.project-tuned.terrain-destruction-9.semicircle": (
        5,
        {
            f"{GENERATED_PREFIX}terrain-9.semicircle.sector-{index:02d}"
            for index in range(1, 4)
        }
        | {
            f"{GENERATED_PREFIX}terrain-9.landing.{index:02d}"
            for index in range(1, 3)
        },
    ),
    "effect.valtan.project-tuned.sequence.six-pizza-106": (
        9,
        {
            f"{GENERATED_PREFIX}six-pizza.ring.original-dark",
            f"{GENERATED_PREFIX}six-pizza.ring.original-cyan",
            f"{GENERATED_PREFIX}six-pizza.ring.separated-dark",
            f"{GENERATED_PREFIX}six-pizza.ring.separated-cyan",
            f"{GENERATED_PREFIX}six-pizza.sector.red-04",
            f"{GENERATED_PREFIX}six-pizza.sector.yellow-05",
            f"{GENERATED_PREFIX}six-pizza.sector.red-roar-overlay",
            f"{GENERATED_PREFIX}six-pizza.ring.center-emissive",
            f"{GENERATED_PREFIX}six-pizza.ring.landing-cyan",
        },
    ),
    "effect.valtan.project-tuned.sequence.attack-whirlwind": (
        13,
        {
            *{
                f"{GENERATED_PREFIX}attack-whirlwind.slam.{index:02d}"
                for index in range(1, 4)
            },
            *{
                f"{GENERATED_PREFIX}attack-whirlwind.spin.{index:02d}"
                for index in range(1, 4)
            },
            *{
                f"{GENERATED_PREFIX}attack-whirlwind.jump-fan.decal-{index:02d}"
                for index in range(1, 4)
            },
            *{
                f"{GENERATED_PREFIX}attack-whirlwind.jump-fan.core.{index:02d}"
                for index in range(1, 4)
            },
            f"{GENERATED_PREFIX}attack-whirlwind.jump-fan.vertical-core",
        },
    ),
    "effect.valtan.project-tuned.sequence.charge": (
        8,
        {
            f"{GENERATED_PREFIX}charge.slash.dark",
            f"{GENERATED_PREFIX}charge.slash.bright-cyan",
            f"{GENERATED_PREFIX}charge.axe-aura.dark-smoke",
            f"{GENERATED_PREFIX}charge.axe-aura.cyan-cloud",
            f"{GENERATED_PREFIX}charge.axe-aura.bright-flame",
            *{
                f"{GENERATED_PREFIX}charge.target-cone.{index:02d}"
                for index in range(1, 4)
            },
        },
    ),
    "effect.valtan.sequence.charge2": (
        5,
        {
            *{
                f"{GENERATED_PREFIX}charge2.red-fan.sector-{index:02d}"
                for index in range(1, 4)
            },
            f"{GENERATED_PREFIX}charge2.slash.bright-red",
        },
    ),
    "effect.valtan.sequence.roar-charge": (
        6,
        {
            f"{GENERATED_PREFIX}roar-charge.ring.dark",
            f"{GENERATED_PREFIX}roar-charge.ring.cyan",
            f"{GENERATED_PREFIX}roar-charge.cone.dark",
            f"{GENERATED_PREFIX}roar-charge.cone.cyan",
            f"{GENERATED_PREFIX}roar-charge.upward.wave",
        },
    ),
    "effect.valtan.project-tuned.sequence.three": (
        5,
        {
            f"{GENERATED_PREFIX}three.impact-01.cyan-ring",
            f"{GENERATED_PREFIX}three.impact-02.cyan-ring",
            f"{GENERATED_PREFIX}three.impact-01.sky-wave",
            f"{GENERATED_PREFIX}three.impact-02.sky-wave",
            f"{GENERATED_PREFIX}three.impact-03.sky-wave",
        },
    ),
    "effect.valtan.sequence.front-back-front": (
        20,
        {
            *{
                f"{GENERATED_PREFIX}front-back-front.source.{index:02d}"
                for index in range(1, 6)
            },
            *{
                f"{GENERATED_PREFIX}front-back-front.blast-{index:02d}.ring"
                for index in range(1, 4)
            },
            *{
                f"{GENERATED_PREFIX}front-back-front.blast-{index:02d}.wave-fallback"
                for index in range(1, 4)
            },
            *{
                f"{GENERATED_PREFIX}front-back-front.blast-{index:02d}.cyan-burst"
                for index in range(1, 4)
            },
            *{
                f"{GENERATED_PREFIX}front-back-front.blast-{index:02d}.vertical-spark"
                for index in range(1, 4)
            },
            *{
                f"{GENERATED_PREFIX}front-back-front.fan.{index:02d}"
                for index in range(1, 4)
            },
        },
    ),
    "effect.valtan.project-tuned.sequence.counter": (
        1,
        {f"{GENERATED_PREFIX}counter.cyan-roar-ring"},
    ),
    "effect.valtan.project-tuned.sequence.warp.portal": (
        15,
        {
            f"{GENERATED_PREFIX}warp.portal.{index:02d}"
            for index in range(1, 15)
        }
        | {f"{GENERATED_PREFIX}warp.portal-rush.forward-mesh"},
    ),
    "effect.valtan.project-tuned.sequence.trash": (
        11,
        {
            *{
                f"{GENERATED_PREFIX}trash.hand-core.{index:02d}"
                for index in range(1, 7)
            },
            *{
                f"{GENERATED_PREFIX}trash.floor.{index:02d}"
                for index in range(1, 5)
            },
            f"{GENERATED_PREFIX}trash.recovery-smoke",
        },
    ),
    "effect.valtan.project-tuned.sequence.trash-catch-success": (
        9,
        {
            *{
                f"{GENERATED_PREFIX}trash-catch-success.hand-core.{index:02d}"
                for index in range(1, 7)
            },
            *{
                f"{GENERATED_PREFIX}trash-catch-success.release-roar.{index:02d}"
                for index in range(1, 3)
            },
            f"{GENERATED_PREFIX}trash-catch-success.release-smoke",
        },
    ),
    "effect.valtan.project-tuned.sequence.trash-catch-fail": (
        3,
        {
            f"{GENERATED_PREFIX}trash-catch-fail.release.{index:02d}"
            for index in range(1, 4)
        },
    ),
    "effect.valtan.project-tuned.sequence.trash-catch-if": (
        6,
        {
            f"{GENERATED_PREFIX}trash-catch-if.hand-core.{index:02d}"
            for index in range(1, 7)
        },
    ),
    "effect.valtan.project-tuned.sequence.catch-breath": (
        7,
        {
            f"{GENERATED_PREFIX}catch-breath.hand-charge.01",
            f"{GENERATED_PREFIX}catch-breath.hand-charge.02",
            f"{GENERATED_PREFIX}catch-breath.forward-cone.dark",
            f"{GENERATED_PREFIX}catch-breath.forward-cone.yellow",
            f"{GENERATED_PREFIX}catch-breath.release-core.01",
            f"{GENERATED_PREFIX}catch-breath.release-core.02",
            f"{GENERATED_PREFIX}catch-breath.recovery-smoke",
        },
    ),
    "effect.valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead": (
        39,
        {
            *{
                f"{GENERATED_PREFIX}struggling.portal.{index:02d}"
                for index in range(1, 16)
            },
            *{
                f"{GENERATED_PREFIX}struggling.fan.{index:02d}"
                for index in range(1, 6)
            },
            f"{GENERATED_PREFIX}struggling.fist-smash.cyan-ring",
            f"{GENERATED_PREFIX}struggling.radial-burst.prototype-01",
            f"{GENERATED_PREFIX}struggling.large-vertical-burst",
            f"{GENERATED_PREFIX}struggling.donut.cyan-grow",
            *{
                f"{GENERATED_PREFIX}struggling.final-roar.{index:02d}"
                for index in range(1, 16)
            },
        },
    ),
}

CUE_CONTRACTS = {
    "cue.valtan.requested.20260827.terrain-3.semicircle": (
        "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
        "IMPACT",
        "effect.valtan.project-tuned.terrain-destruction-3.semicircle",
    ),
    "cue.valtan.requested.20260827.terrain-9.semicircle": (
        "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK",
        "IMPACT",
        "effect.valtan.project-tuned.terrain-destruction-9.semicircle",
    ),
    "cue.valtan.requested.20260827.six-pizza.composite": (
        "VALTAN_SIX_PIZZA_106",
        "STEP_01",
        "effect.valtan.project-tuned.sequence.six-pizza-106",
    ),
    "cue.valtan.requested.20260827.attack-whirlwind.composite": (
        "VALTAN_ATTACK_WHIRLWIND",
        "STEP_01",
        "effect.valtan.project-tuned.sequence.attack-whirlwind",
    ),
    "cue.valtan.requested.20260827.charge.axe-follow": (
        "VALTAN_CHARGE",
        "STEP_01",
        "effect.valtan.sequence.charge",
    ),
    "cue.valtan.requested.20260827.charge2.red-fan": (
        "VALTAN_CHARGE_2",
        "STEP_03",
        "effect.valtan.sequence.charge2",
    ),
    "cue.valtan.requested.20260827.roar-charge.composite": (
        "VALTAN_ROAR_CHARGE",
        "STEP_03",
        "effect.valtan.sequence.roar-charge",
    ),
    "cue.valtan.requested.20260827.three.composite": (
        "VALTAN_THREE",
        "STEP_01",
        "effect.valtan.project-tuned.sequence.three",
    ),
    "cue.valtan.requested.20260827.front-back-front.electric-fan": (
        "VALTAN_SEQUENCE_FRONT_BACK_FRONT",
        "STEP_01",
        "effect.valtan.sequence.front-back-front",
    ),
    "cue.valtan.requested.20260827.counter.cyan-roar-ring": (
        "VALTAN_COUNTER",
        "STEP_03",
        "effect.valtan.project-tuned.sequence.counter",
    ),
    "cue.valtan.requested.20260827.trash.composite": (
        "VALTAN_TRASH",
        "STEP_01",
        "effect.valtan.project-tuned.sequence.trash",
    ),
    **{
        f"cue.valtan.phase2.warp.step-{leg:02d}.composite": (
            "VALTAN_WARP",
            f"STEP_{leg:02d}",
            "effect.valtan.project-tuned.sequence.warp.portal",
        )
        for leg in range(2, 10)
    },
    "cue.valtan.requested.20260827.trash-catch-success.composite": (
        "VALTAN_TRASH_CATCH_SUCCESS",
        "STEP_01",
        "effect.valtan.project-tuned.sequence.trash-catch-success",
    ),
    "cue.valtan.requested.20260827.trash-catch-fail.composite": (
        "VALTAN_TRASH_CATCH_FAIL",
        "STEP_01",
        "effect.valtan.project-tuned.sequence.trash-catch-fail",
    ),
    "cue.valtan.requested.20260827.trash-catch-if.composite": (
        "VALTAN_TRASH_CATCH_IF",
        "STEP_01",
        "effect.valtan.project-tuned.sequence.trash-catch-if",
    ),
    "cue.valtan.requested.20260827.catch-breath.composite": (
        "VALTAN_CATCH_BREATH",
        "STEP_01",
        "effect.valtan.project-tuned.sequence.catch-breath",
    ),
    "cue.valtan.requested.20260827.struggling.composite": (
        "VALTAN_STRUGGLING",
        "STEP_01",
        "effect.valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead",
    ),
}

CUE_RUNTIME_EXPECTATIONS = {
    "cue.valtan.requested.20260827.terrain-3.semicircle": (
        "snapshot",
        [0.0, 0.0, 0.0],
        200,
    ),
    "cue.valtan.requested.20260827.terrain-9.semicircle": (
        "snapshot",
        [0.0, 0.0, 0.0],
        200,
    ),
    **{
        f"cue.valtan.phase2.warp.step-{leg:02d}.composite": (
            "follow",
            [0.0, 0.0, 3.0],
            0,
        )
        for leg in range(2, 10)
    },
}


def _reject_duplicate_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def _load_json(relative_path: str) -> dict[str, Any]:
    path = ROOT / relative_path
    with path.open("r", encoding="utf-8-sig") as stream:
        value = json.load(stream, object_pairs_hook=_reject_duplicate_keys)
    if not isinstance(value, dict):
        raise AssertionError(f"expected JSON object: {path}")
    return value


def _index_unique(rows: list[dict[str, Any]], key: str, label: str) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for row in rows:
        identity = row.get(key)
        if not isinstance(identity, str) or not identity:
            raise AssertionError(f"{label} has an empty {key}")
        if identity in result:
            raise AssertionError(f"duplicate {label} {key}: {identity}")
        result[identity] = row
    return result


class ValtanRequestedEffectElementsContractTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.catalog = _load_json("Data/Effects/EffectCatalog.json")
        cls.presentation = _load_json("Data/Valtan/Valtan.presentation.json")
        cls.gameplay = _load_json("Data/Valtan/Valtan.gameplay.json")
        cls.documents: dict[str, dict[str, Any]] = {}
        for asset_id in PRODUCT_ELEMENTS:
            relative_path = f"Data/Effects/Authored/{asset_id}.effect.json"
            cls.documents[asset_id] = _load_json(relative_path)

    def _elements(self, asset_id: str) -> dict[str, dict[str, Any]]:
        rows = self.documents[asset_id].get("elements")
        self.assertIsInstance(rows, list, asset_id)
        return _index_unique(rows, "id", f"{asset_id} element")

    def _element(self, asset_id: str, element_id: str) -> dict[str, Any]:
        elements = self._elements(asset_id)
        self.assertIn(element_id, elements, asset_id)
        return elements[element_id]

    @staticmethod
    def _resources(element: dict[str, Any]) -> dict[str, str]:
        rows = element.get("resources", [])
        return {row["slotId"]: row["assetId"] for row in rows}

    @staticmethod
    def _start(element: dict[str, Any]) -> float:
        return float(element["detail"]["timing"]["startDelaySeconds"])

    def assertStart(self, element: dict[str, Any], expected: float) -> None:
        self.assertAlmostEqual(self._start(element), expected, places=6)

    def test_product_catalog_and_all_seventeen_authored_documents(self) -> None:
        catalog_rows = self.catalog.get("effects")
        self.assertIsInstance(catalog_rows, list)
        catalog = _index_unique(catalog_rows, "effectAssetId", "effect catalog row")

        self.assertEqual(len(PRODUCT_ELEMENTS), 17)
        for asset_id, (expected_total, expected_generated) in PRODUCT_ELEMENTS.items():
            with self.subTest(asset_id=asset_id):
                self.assertIn(asset_id, catalog)
                self.assertEqual(
                    catalog[asset_id].get("payloadKind"),
                    "DIRECT_AUTHORED_DOCUMENT",
                )
                self.assertEqual(
                    catalog[asset_id].get("authoringPath"),
                    f"Effects/Authored/{asset_id}.effect.json",
                )

                document = self.documents[asset_id]
                self.assertEqual(document.get("schema"), "lostark.effect-authoring")
                self.assertEqual(document.get("version"), 13)
                self.assertEqual(document.get("effectAssetId"), asset_id)
                elements = self._elements(asset_id)
                self.assertEqual(len(elements), expected_total)
                actual_generated = {
                    element_id
                    for element_id in elements
                    if element_id.startswith(GENERATED_PREFIX)
                }
                self.assertEqual(actual_generated, expected_generated)

    def test_generated_rows_match_current_executable_parser_metadata(self) -> None:
        for asset_id, (_, expected_generated) in PRODUCT_ELEMENTS.items():
            for element_id in expected_generated:
                with self.subTest(asset_id=asset_id, element_id=element_id):
                    element = self._element(asset_id, element_id)
                    self.assertTrue(
                        str(element.get("sourceNode", "")).startswith(
                            "authored-copy:"
                        ),
                        "manual particle layouts require the executable's "
                        "direct-authored sourceNode prefix",
                    )
                    overrides = element.get("authoringOverrides")
                    if not isinstance(overrides, dict):
                        continue
                    for resource in overrides.get("resources", []):
                        self.assertIn("compilerAssetId", resource)
                        self.assertIsInstance(resource["compilerAssetId"], str)

    def test_terrain_semicircles_are_opposed_and_suppress_dissolve(self) -> None:
        asset_3 = "effect.valtan.project-tuned.terrain-destruction-3.semicircle"
        asset_9 = "effect.valtan.project-tuned.terrain-destruction-9.semicircle"
        for index in range(1, 4):
            element_3 = self._element(
                asset_3,
                f"{GENERATED_PREFIX}terrain-3.semicircle.sector-{index:02d}",
            )
            element_9 = self._element(
                asset_9,
                f"{GENERATED_PREFIX}terrain-9.semicircle.sector-{index:02d}",
            )
            yaw_3 = float(element_3["detail"]["transform"]["rotationDegrees"][1])
            yaw_9 = float(element_9["detail"]["transform"]["rotationDegrees"][1])
            self.assertAlmostEqual((yaw_9 - yaw_3) % 360.0, 180.0, places=6)

            for element in (element_3, element_9):
                self.assertStart(element, 0.0)
                resources = self._resources(element)
                self.assertEqual(resources.get("mask"), SECTOR_04)
                self.assertNotIn("dissolve", resources)
                timing = element["detail"]["timing"]
                self.assertAlmostEqual(
                    float(timing.get("dissolveStartNormalized")), 1.0, places=6
                )

        for asset_id, suffix in ((asset_3, "3"), (asset_9, "9")):
            self.assertStart(
                self._element(
                    asset_id, f"{GENERATED_PREFIX}terrain-{suffix}.landing.01"
                ),
                0.230894,
            )
            self.assertStart(
                self._element(
                    asset_id, f"{GENERATED_PREFIX}terrain-{suffix}.landing.02"
                ),
                0.231,
            )
            self.assertFalse(
                any(
                    element_id.startswith(
                        f"{GENERATED_PREFIX}terrain-{suffix}.takeoff."
                    )
                    for element_id in self._elements(asset_id)
                )
            )

    def test_six_pizza_layers_resources_and_timing_cohorts(self) -> None:
        asset_id = "effect.valtan.project-tuned.sequence.six-pizza-106"
        original_dark = self._element(
            asset_id, f"{GENERATED_PREFIX}six-pizza.ring.original-dark"
        )
        original_cyan = self._element(
            asset_id, f"{GENERATED_PREFIX}six-pizza.ring.original-cyan"
        )
        separated_dark = self._element(
            asset_id, f"{GENERATED_PREFIX}six-pizza.ring.separated-dark"
        )
        separated_cyan = self._element(
            asset_id, f"{GENERATED_PREFIX}six-pizza.ring.separated-cyan"
        )
        for element in (original_dark, original_cyan, separated_dark, separated_cyan):
            self.assertStart(element, 6.2)

        self.assertEqual(
            separated_dark["material"]["renderProfile"],
            "alpha_two_sided_depth_read",
        )
        self.assertEqual(
            separated_cyan["material"]["renderProfile"],
            "additive_two_sided_depth_read",
        )
        self.assertEqual(separated_dark["detail"]["decal"]["size"], [32.0, 32.0])
        self.assertEqual(
            separated_cyan["detail"]["decal"]["size"], [24.5, 24.5]
        )
        self.assertAlmostEqual(
            float(separated_dark["detail"]["transform"]["position"][1]),
            0.021,
            places=6,
        )
        self.assertAlmostEqual(
            float(separated_cyan["detail"]["transform"]["position"][1]),
            0.037,
            places=6,
        )

        red = self._element(
            asset_id, f"{GENERATED_PREFIX}six-pizza.sector.red-04"
        )
        yellow = self._element(
            asset_id, f"{GENERATED_PREFIX}six-pizza.sector.yellow-05"
        )
        overlay = self._element(
            asset_id, f"{GENERATED_PREFIX}six-pizza.sector.red-roar-overlay"
        )
        center = self._element(
            asset_id, f"{GENERATED_PREFIX}six-pizza.ring.center-emissive"
        )
        landing = self._element(
            asset_id, f"{GENERATED_PREFIX}six-pizza.ring.landing-cyan"
        )
        self.assertEqual(self._resources(red).get("mask"), SECTOR_04)
        self.assertEqual(self._resources(yellow).get("mask"), SECTOR_05)
        self.assertEqual(self._resources(overlay).get("mask"), SECTOR_04)
        for element in (red, yellow, overlay, center):
            self.assertStart(element, 22.6)
        self.assertStart(landing, 28.6)

    def test_jump_slam_and_front_back_front_use_broad_fan_layers(self) -> None:
        jump_asset = "effect.valtan.project-tuned.sequence.attack-whirlwind"
        expected_decal_layout = (
            (1.18, [2.8, 0.055, -2.3], [6.8, 6.8]),
            (1.22, [4.0, 0.055, 0.0], [7.5, 7.5]),
            (1.26, [2.8, 0.055, 2.3], [6.8, 6.8]),
        )
        for index, (start, position, size) in enumerate(
            expected_decal_layout, start=1
        ):
            decal = self._element(
                jump_asset,
                f"{GENERATED_PREFIX}attack-whirlwind.jump-fan.decal-{index:02d}",
            )
            self.assertStart(decal, start)
            self.assertEqual(
                self._resources(decal).get("base"),
                "Effect/Valtan/Textures/FX_TEX_04/fx_f_ring_001.dds",
            )
            self.assertEqual(decal["detail"]["transform"]["position"], position)
            self.assertEqual(decal["detail"]["decal"]["size"], size)

        for asset_id, prefix, start, scale in (
            (
                jump_asset,
                "attack-whirlwind.jump-fan.core",
                1.30,
                [1.9, 1.9, 1.9],
            ),
            (
                "effect.valtan.sequence.front-back-front",
                "front-back-front.fan",
                0.30,
                [2.05, 2.05, 2.05],
            ),
        ):
            for index in range(1, 4):
                fan = self._element(
                    asset_id, f"{GENERATED_PREFIX}{prefix}.{index:02d}"
                )
                self.assertStart(fan, start)
                self.assertEqual(fan["detail"]["transform"]["scale"], scale)

        vertical = self._element(
            jump_asset,
            f"{GENERATED_PREFIX}attack-whirlwind.jump-fan.vertical-core",
        )
        self.assertStart(vertical, 1.32)
        self.assertEqual(
            self._resources(vertical).get("base"),
            "Effect/Valtan/Textures/FX_H_W_01/fx_h_wave_04.dds",
        )
        self.assertEqual(vertical["detail"]["particle"]["burstCount"], 16)
        self.assertEqual(
            vertical["detail"]["particle"]["endSize"],
            [6.26999998, 40.5299988],
        )

    def test_struggling_is_one_timed_product_with_one_radial_prototype(self) -> None:
        asset_id = "effect.valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead"

        portal_ids = {
            element_id
            for element_id in self._elements(asset_id)
            if element_id.startswith(f"{GENERATED_PREFIX}struggling.portal.")
        }
        self.assertEqual(len(portal_ids), 15)

        expected_fan_starts = (6.417, 4.617, 4.617, 5.517, 5.517)
        for index, expected in enumerate(expected_fan_starts, start=1):
            self.assertStart(
                self._element(
                    asset_id, f"{GENERATED_PREFIX}struggling.fan.{index:02d}"
                ),
                expected,
            )

        fist = self._element(
            asset_id, f"{GENERATED_PREFIX}struggling.fist-smash.cyan-ring"
        )
        self.assertStart(fist, 14.682)
        self.assertEqual(fist["detail"]["decal"]["size"], [14.0, 14.0])

        prototype = self._element(
            asset_id,
            f"{GENERATED_PREFIX}struggling.radial-burst.prototype-01",
        )
        self.assertStart(prototype, 17.75)
        self.assertEqual(prototype["detail"]["transform"]["position"], [5.5, 1.5, 0.0])
        self.assertEqual(
            prototype["detail"]["transform"]["rotationDegrees"],
            [0.0, 90.0, 0.0],
        )
        particle = prototype["detail"]["particle"]
        self.assertEqual(particle["maxParticles"], 1)
        self.assertEqual(particle["spawnRatePerSecond"], 0.0)
        self.assertEqual(particle["burstCount"], 1)
        self.assertEqual(particle["startSize"], [0.05, 0.08])
        self.assertEqual(particle["endSize"], [5.5, 7.5])
        self.assertIs(prototype["sourceRecipe"]["enabled"], False)

        vertical = self._element(
            asset_id, f"{GENERATED_PREFIX}struggling.large-vertical-burst"
        )
        donut = self._element(
            asset_id, f"{GENERATED_PREFIX}struggling.donut.cyan-grow"
        )
        self.assertStart(vertical, 19.2)
        self.assertStart(donut, 20.4)
        self.assertEqual(
            self._resources(donut).get("base"),
            "Effect/Valtan/Textures/FX_TEX_00/fx_b_ring_001.dds",
        )
        self.assertIs(donut["detail"]["linearLerp"]["scale"], True)

        for index in range(1, 16):
            self.assertStart(
                self._element(
                    asset_id,
                    f"{GENERATED_PREFIX}struggling.final-roar.{index:02d}",
                ),
                21.407,
            )

    def test_charge_layers_follow_right_weapon_and_scroll_uv(self) -> None:
        asset_id = "effect.valtan.project-tuned.sequence.charge"
        runtime_slots: set[str] = set()
        attached_ids = {
            element_id
            for element_id in PRODUCT_ELEMENTS[asset_id][1]
            if ".slash." in element_id or ".axe-aura." in element_id
        }
        self.assertEqual(len(attached_ids), 5)
        for element_id in attached_ids:
            with self.subTest(element_id=element_id):
                element = self._element(asset_id, element_id)
                attachment = element.get("actionCueAttachment", {})
                self.assertIs(attachment.get("enabled"), True)
                self.assertIs(attachment.get("follow"), True)
                self.assertEqual(attachment.get("sourceAnchorSlotId"), "b_wp_r_01")
                self.assertEqual(attachment.get("runtimeBoneName"), "b_wp_r_01")
                runtime_slot = attachment.get("runtimeAnchorSlotId")
                self.assertIsInstance(runtime_slot, str)
                self.assertTrue(runtime_slot)
                self.assertNotIn(runtime_slot, runtime_slots)
                runtime_slots.add(runtime_slot)

                uv = element["detail"]["uv"]
                self.assertIs(uv.get("loop"), True)
                speed = uv.get("speed")
                self.assertEqual(len(speed), 2)
                self.assertTrue(any(abs(float(value)) > 0.0 for value in speed))

    def test_charge_smoke_continuously_emits_around_right_weapon(self) -> None:
        asset_id = "effect.valtan.project-tuned.sequence.charge"
        expected = {
            f"{GENERATED_PREFIX}charge.axe-aura.dark-smoke": {
                "position": [0.0, 0.12, -0.10],
                "color": [0.015, 0.015, 0.015, 3.8],
                "emissive": 0.25,
                "rate": 18.0,
                "burst": 4,
                "maximum": 28,
                "sizeScale": 0.62,
                "lifetimeScale": 0.55,
            },
            f"{GENERATED_PREFIX}charge.axe-aura.cyan-cloud": {
                "position": [0.0, 0.18, 0.10],
                "color": [0.0, 2.6, 2.0, 3.2],
                "emissive": 2.6,
                "rate": 14.0,
                "burst": 6,
                "maximum": 32,
                "sizeScale": 0.55,
                "lifetimeScale": 0.35,
            },
        }
        for element_id, values in expected.items():
            with self.subTest(element_id=element_id):
                element = self._element(asset_id, element_id)
                detail = element["detail"]
                self.assertAlmostEqual(
                    float(detail["timing"]["lifeTimeSeconds"]), 1.067, places=6
                )
                self.assertEqual(detail["transform"]["position"], values["position"])
                self.assertEqual(detail["color"]["multiply"], values["color"])
                self.assertAlmostEqual(
                    float(detail["color"]["emissiveIntensity"]),
                    values["emissive"],
                    places=6,
                )

                particle = detail["particle"]
                self.assertIs(particle["localSpace"], True)
                self.assertIs(particle["billboard"], True)
                self.assertEqual(particle["maxParticles"], values["maximum"])
                self.assertAlmostEqual(
                    float(particle["spawnRatePerSecond"]), values["rate"], places=6
                )
                self.assertEqual(particle["burstCount"], values["burst"])
                self.assertAlmostEqual(
                    float(particle["sourceScale"]["size"]),
                    values["sizeScale"],
                    places=6,
                )
                self.assertAlmostEqual(
                    float(particle["sourceScale"]["lifeTime"]),
                    values["lifetimeScale"],
                    places=6,
                )

                recipe = element["sourceRecipe"]
                self.assertIs(recipe["enabled"], True)
                self.assertAlmostEqual(
                    float(recipe["emitterDurationSeconds"]), 0.82, places=6
                )
                self.assertEqual(recipe["emitterLoopCount"], 1)
                self.assertEqual(
                    recipe["bursts"],
                    [
                        {
                            "timeSeconds": 0.0,
                            "countMinimum": values["burst"],
                            "countMaximum": values["burst"],
                        }
                    ],
                )
                spawn_modules = [
                    module
                    for module in recipe["modules"]
                    if module.get("className", "").lower()
                    == "particlemodulespawn"
                ]
                self.assertEqual(len(spawn_modules), 1)
                rate_distributions = [
                    distribution
                    for distribution in spawn_modules[0]["distributions"]
                    if distribution.get("propertyPath") == "rate"
                ]
                self.assertEqual(len(rate_distributions), 1)
                self.assertEqual(
                    rate_distributions[0]["lookupTable"], [values["rate"]] * 4
                )

        bright = self._element(
            asset_id, f"{GENERATED_PREFIX}charge.axe-aura.bright-flame"
        )
        self.assertAlmostEqual(
            float(bright["detail"]["timing"]["lifeTimeSeconds"]),
            1.067,
            places=6,
        )

    def test_roar_three_and_counter_requested_timings(self) -> None:
        roar_asset = "effect.valtan.sequence.roar-charge"
        for suffix in ("ring.dark", "ring.cyan", "cone.dark", "cone.cyan"):
            self.assertStart(
                self._element(
                    roar_asset,
                    f"{GENERATED_PREFIX}roar-charge.{suffix}",
                ),
                0.74,
            )
        self.assertStart(
            self._element(
                roar_asset, f"{GENERATED_PREFIX}roar-charge.upward.wave"
            ),
            7.063,
        )

        three_asset = "effect.valtan.project-tuned.sequence.three"
        expected_three = {
            "impact-01.cyan-ring": 1.617,
            "impact-02.cyan-ring": 2.763,
            "impact-01.sky-wave": 1.617,
            "impact-02.sky-wave": 2.763,
            "impact-03.sky-wave": 4.191,
        }
        for suffix, expected_start in expected_three.items():
            self.assertStart(
                self._element(three_asset, f"{GENERATED_PREFIX}three.{suffix}"),
                expected_start,
            )

        counter = self._element(
            "effect.valtan.project-tuned.sequence.counter",
            f"{GENERATED_PREFIX}counter.cyan-roar-ring",
        )
        self.assertStart(counter, 0.9)

    def test_portal_layer_count_and_trash_bindings_and_floor_resources(self) -> None:
        portal_asset = "effect.valtan.project-tuned.sequence.warp.portal"
        portal_elements = self._elements(portal_asset)
        portal_generated = {
            element_id
            for element_id in portal_elements
            if element_id.startswith(f"{GENERATED_PREFIX}warp.portal.")
        }
        self.assertEqual(len(portal_generated), 14)
        self.assertIn(
            f"{GENERATED_PREFIX}warp.portal-rush.forward-mesh",
            portal_elements,
        )
        forward_mesh = portal_elements[
            f"{GENERATED_PREFIX}warp.portal-rush.forward-mesh"
        ]
        self.assertStart(forward_mesh, 0.0)
        self.assertAlmostEqual(
            float(forward_mesh["detail"]["timing"]["lifeTimeSeconds"]),
            0.41,
            places=6,
        )
        self.assertEqual(
            self._resources(forward_mesh).get("meshModel"),
            "Effect/Valtan/Meshes/FX_SM_00/fm_h_halfsphere_01_1.wmodel",
        )
        self.assertEqual(
            {0.0, 0.063222},
            {
                round(self._start(portal_elements[element_id]), 6)
                for element_id in portal_generated
            },
        )

        catalog_asset_ids = {
            row["effectAssetId"] for row in self.catalog.get("effects", [])
        }
        self.assertIn(
            "effect.valtan.project-tuned.sequence.warp.portal",
            catalog_asset_ids,
        )
        self.assertNotIn(
            "effect.valtan.project-tuned.sequence.warp.portal-enter",
            catalog_asset_ids,
        )

        trash_asset = "effect.valtan.project-tuned.sequence.trash"
        runtime_slots: set[str] = set()
        attached_ids = [
            *[
                f"{GENERATED_PREFIX}trash.hand-core.{index:02d}"
                for index in range(1, 7)
            ],
            f"{GENERATED_PREFIX}trash.recovery-smoke",
        ]
        for element_id in attached_ids:
            attachment = self._element(trash_asset, element_id).get(
                "actionCueAttachment", {}
            )
            self.assertIs(attachment.get("enabled"), True)
            self.assertIs(attachment.get("follow"), True)
            self.assertEqual(attachment.get("sourceAnchorSlotId"), "bip001-l-hand")
            self.assertEqual(attachment.get("runtimeBoneName"), "bip001-l-hand")
            runtime_slot = attachment.get("runtimeAnchorSlotId")
            self.assertIsInstance(runtime_slot, str)
            self.assertTrue(runtime_slot)
            self.assertNotIn(runtime_slot, runtime_slots)
            runtime_slots.add(runtime_slot)

        floor_1 = self._resources(
            self._element(trash_asset, f"{GENERATED_PREFIX}trash.floor.01")
        )
        self.assertEqual(
            floor_1,
            {
                "meshModel": "Effect/Valtan/Meshes/FX_SM_00/fm_d_crackline_001.wmodel",
                "base": "Effect/Valtan/Textures/FX_TEX_02/fx_d_decal_046_1.dds",
                "mask": "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_097.dds",
                "emissive": FLOOR_ELECTRIC,
            },
        )
        for index in range(2, 5):
            resources = self._resources(
                self._element(
                    trash_asset, f"{GENERATED_PREFIX}trash.floor.{index:02d}"
                )
            )
            self.assertEqual(
                resources,
                {
                    "mask": FLOOR_RING,
                    "noise": FLOOR_NOISE,
                    "base": FLOOR_ELECTILE,
                    "emissive": FLOOR_EMISSIVE_NOISE,
                },
            )

    def test_trash_variants_and_catch_breath_have_visible_timed_elements(self) -> None:
        success = "effect.valtan.project-tuned.sequence.trash-catch-success"
        fail = "effect.valtan.project-tuned.sequence.trash-catch-fail"
        branch = "effect.valtan.project-tuned.sequence.trash-catch-if"
        breath = "effect.valtan.project-tuned.sequence.catch-breath"

        self.assertEqual(len(self._elements(success)), 9)
        self.assertEqual(len(self._elements(fail)), 3)
        self.assertEqual(len(self._elements(branch)), 6)
        self.assertEqual(len(self._elements(breath)), 7)

        dark = self._element(
            breath, f"{GENERATED_PREFIX}catch-breath.forward-cone.dark"
        )
        yellow = self._element(
            breath, f"{GENERATED_PREFIX}catch-breath.forward-cone.yellow"
        )
        for element in (dark, yellow):
            self.assertStart(element, 2.25)
            self.assertAlmostEqual(
                float(element["detail"]["timing"]["lifeTimeSeconds"]),
                4.1,
                places=6,
            )
            self.assertTrue(element["detail"]["uv"]["loop"])
        self.assertEqual(
            dark["material"]["renderProfile"], "alpha_two_sided_depth_read"
        )
        self.assertEqual(
            yellow["material"]["renderProfile"], "additive_two_sided_depth_read"
        )
        self.assertEqual(
            yellow["detail"]["color"]["multiply"], [4.8, 3.85, 0.05, 5.2]
        )
        self.assertEqual(
            yellow["detail"]["transform"]["rotationDegrees"], [0.0, 0.0, 0.0]
        )

    def test_requested_cues_are_on_the_exact_patterns_and_stages(self) -> None:
        actual: dict[str, tuple[str, str, dict[str, Any]]] = {}
        all_cue_ids: set[str] = set()
        for pattern in self.presentation.get("patterns", []):
            pattern_id = pattern.get("patternId")
            for stage in pattern.get("stages", []):
                stage_id = stage.get("stageId")
                for cue in stage.get("effectCues", []):
                    cue_id = cue.get("cueId")
                    if isinstance(cue_id, str):
                        all_cue_ids.add(cue_id)
                    if not isinstance(cue_id, str) or cue_id not in CUE_CONTRACTS:
                        continue
                    self.assertNotIn(cue_id, actual, f"duplicate cue: {cue_id}")
                    actual[cue_id] = (pattern_id, stage_id, cue)

        self.assertEqual(set(actual), set(CUE_CONTRACTS))
        self.assertNotIn(
            "cue.valtan.requested.20260827.warp.composite",
            all_cue_ids,
        )
        self.assertFalse(
            any(
                cue_id.startswith("cue.valtan.phase2.warp.step-")
                and cue_id.endswith((".portal-enter", ".rush"))
                for cue_id in all_cue_ids
            ),
            "split portal-enter/rush WARP cues must remain absent",
        )
        for cue_id, (expected_pattern, expected_stage, expected_asset) in (
            CUE_CONTRACTS.items()
        ):
            with self.subTest(cue_id=cue_id):
                pattern_id, stage_id, cue = actual[cue_id]
                self.assertEqual(pattern_id, expected_pattern)
                self.assertEqual(stage_id, expected_stage)
                self.assertEqual(cue.get("effectAssetId"), expected_asset)
                self.assertEqual(cue.get("anchorSlotId"), "root")
                self.assertEqual(cue.get("stopPolicy"), "natural")
                self.assertEqual(cue.get("repeatPolicy"), "once")
                runtime = CUE_RUNTIME_EXPECTATIONS.get(cue_id)
                expected_follow = "follow" if runtime is None else runtime[0]
                expected_position = (
                    [0.0, 0.0, 0.0] if runtime is None else runtime[1]
                )
                self.assertEqual(cue.get("followPolicy"), expected_follow)
                self.assertEqual(
                    cue.get("localTransform"),
                    {
                        "position": expected_position,
                        "rotationDegrees": [0.0, 0.0, 0.0],
                        "scale": [1.0, 1.0, 1.0],
                    },
                )
                if runtime is not None:
                    self.assertEqual(cue.get("sourceStartMs"), runtime[2])
                if cue_id.startswith("cue.valtan.phase2.warp.step-"):
                    self.assertEqual(
                        cue.get("scalePolicy"), {"kind": "OWNER_RELATIVE"}
                    )
                if cue_id.startswith(
                    "cue.valtan.requested.20260827.terrain-"
                ):
                    self.assertEqual(
                        cue.get("scalePolicy"),
                        {
                            "kind": "GAMEPLAY_FOOTPRINT",
                            "worldScale": [1.5, 1.5, 1.5],
                        },
                    )

        contract_patterns = {value[0] for value in CUE_CONTRACTS.values()}
        for pattern in self.presentation.get("patterns", []):
            if pattern.get("patternId") not in contract_patterns:
                continue
            total = sum(len(stage.get("effectCues", [])) for stage in pattern["stages"])
            expected_total = sum(
                1
                for contract in CUE_CONTRACTS.values()
                if contract[0] == pattern.get("patternId")
            )
            self.assertEqual(total, expected_total, pattern.get("patternId"))

    def test_generator_validate_is_deterministic_and_idempotent(self) -> None:
        command = [
            sys.executable,
            str(
                ROOT
                / "Tools"
                / "ValtanPipeline"
                / "author_valtan_requested_effect_elements.py"
            ),
            "--mode",
            "Validate",
        ]
        results = [
            subprocess.run(
                command,
                cwd=ROOT,
                capture_output=True,
                text=True,
                encoding="utf-8",
                check=False,
            )
            for _ in range(2)
        ]
        for result in results:
            self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(results[0].stdout, results[1].stdout)

    def test_promoted_user_documents_are_not_duplicated_as_drafts(self) -> None:
        bindings = _load_json("Data/Effects/ValtanPatternAuthoringEffects.json")
        actual = {
            (row["patternId"], row["effectAssetId"])
            for row in bindings.get("bindings", [])
        }
        self.assertEqual(
            actual,
            {("VALTAN_SEQUENCE_RUSH", "effect.valtan.sequence.rush")},
        )
        catalog = _index_unique(
            self.catalog.get("effects", []), "effectAssetId", "effect catalog row"
        )
        promoted = catalog["effect.valtan.sequence.charge"]
        self.assertEqual(promoted.get("payloadKind"), "DIRECT_AUTHORED_DOCUMENT")
        self.assertEqual(
            promoted.get("authoringPath"),
            "Effects/Authored/effect.valtan.sequence.charge.effect.json",
        )

    def test_six_pizza_locks_one_player_then_tracks_aim(self) -> None:
        patterns = _index_unique(
            self.gameplay.get("patterns", []), "patternId", "gameplay pattern"
        )
        pizza = patterns["VALTAN_SIX_PIZZA_106"]
        self.assertEqual(pizza.get("targetPolicy"), "LOCK_RANDOM_ALIVE_ON_START")
        self.assertEqual(pizza.get("aimPolicy"), "TRACK_TARGET_EACH_TICK")


if __name__ == "__main__":
    unittest.main()
