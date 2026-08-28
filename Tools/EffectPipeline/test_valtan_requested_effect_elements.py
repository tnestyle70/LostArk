#!/usr/bin/env python3
"""Focused contract tests for the 2026-08-27 requested Valtan effect package."""

from __future__ import annotations

import copy
import importlib.util
import io
import json
import subprocess
import sys
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path
from typing import Any
from unittest import mock


ROOT = Path(__file__).resolve().parents[2]
AUTHORED_ROOT = ROOT / "Data" / "Effects" / "Authored"
GENERATED_PREFIX = "requested.20260827."
SPEC = importlib.util.spec_from_file_location(
    "valtan_requested_effect_elements", ROOT / "Tools/ValtanPipeline/author_valtan_requested_effect_elements.py"
)
assert SPEC is not None and SPEC.loader is not None
author = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = author
SPEC.loader.exec_module(author)

SECTOR_04 = "Effect/Valtan/Textures/FX_TEX_05/fx_o_sector_04.dds"
SECTOR_05 = "Effect/Valtan/Textures/FX_TEX_05/fx_o_sector_05.dds"
FLOOR_ELECTRIC = "Effect/Valtan/Textures/FX_TEX_02/fx_d_electric_013_ycl.dds"
FLOOR_RING = "Effect/Valtan/Textures/FX_TEX_05/fx_m_ring_001.dds"
FLOOR_ELECTILE = "Effect/Valtan/Textures/FX_TEX_05/fx_k_electile_02.dds"
FLOOR_NOISE = "Effect/Valtan/Textures/FX_TEX_02/fx_d_noise_002.dds"
FLOOR_EMISSIVE_NOISE = "Effect/Valtan/Textures/FX_TEX_05/fx_m_noise_001.dds"

# Initial visual defaults are not a persistent row-count contract. Once saved,
# Product documents own their tuning and membership (including deletions).
PRODUCT_ASSET_IDS = tuple(target.effect_asset_id for target in author.PRODUCT_TARGETS)

CUE_CONTRACTS = {
    "cue.valtan.requested.20260827.terrain-3.semicircle": (
        "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
        "TAKEOFF",
        "effect.valtan.project-tuned.terrain-destruction-3.semicircle",
    ),
    "cue.valtan.requested.20260827.terrain-9.semicircle": (
        "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK",
        "TAKEOFF",
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
        "STEP_01",
        "effect.valtan.sequence.roar-charge",
    ),
    "cue.valtan.requested.20260827.three.composite": (
        "VALTAN_THREE",
        "STEP_01",
        "effect.valtan.project-tuned.sequence.three",
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
        "CATCH_COUNTER",
        "effect.valtan.project-tuned.sequence.trash-catch-success",
    ),
    "cue.valtan.requested.20260827.trash-catch-fail.composite": (
        "VALTAN_TRASH_CATCH_FAIL",
        "RUSH_MISS",
        "effect.valtan.project-tuned.sequence.trash-catch-fail",
    ),
    "cue.valtan.requested.20260827.trash-catch-if.composite": (
        "VALTAN_TRASH_CATCH_IF",
        "STEP_07",
        "effect.valtan.project-tuned.sequence.trash-catch-if",
    ),
    "cue.valtan.requested.20260827.catch-breath.composite": (
        "VALTAN_CATCH_BREATH",
        "STEP_01",
        "effect.valtan.project-tuned.sequence.catch-breath",
    ),
}

CUE_RUNTIME_EXPECTATIONS = {
    "cue.valtan.requested.20260827.terrain-3.semicircle": (
        "snapshot",
        [0.0, 0.0, 0.0],
        0,
    ),
    "cue.valtan.requested.20260827.terrain-9.semicircle": (
        "snapshot",
        [0.0, 0.0, 0.0],
        0,
    ),
    "cue.valtan.requested.20260827.six-pizza.composite": (
        "snapshot",
        [0.0, 0.0, 0.0],
        0,
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

CUE_ANCHOR_EXPECTATIONS = {
    "cue.valtan.requested.20260827.terrain-3.semicircle": "arena.center",
    "cue.valtan.requested.20260827.terrain-9.semicircle": "arena.center",
    "cue.valtan.requested.20260827.six-pizza.composite": "arena.center.facing",
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
        for asset_id in PRODUCT_ASSET_IDS:
            relative_path = f"Data/Effects/Authored/{asset_id}.effect.json"
            cls.documents[asset_id] = _load_json(relative_path)

    def _elements(self, asset_id: str) -> dict[str, dict[str, Any]]:
        rows = self.documents[asset_id].get("elements")
        self.assertIsInstance(rows, list, asset_id)
        return _index_unique(rows, "id", f"{asset_id} element")

    def _element(self, asset_id: str, element_id: str) -> dict[str, Any]:
        elements = self._elements(asset_id)
        self.assertIn(element_id, elements.keys(), asset_id)
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

        self.assertEqual(len(PRODUCT_ASSET_IDS), 17)
        for asset_id in PRODUCT_ASSET_IDS:
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
                self._elements(asset_id)  # Strict ID uniqueness, without restoring deleted rows.
                author._validate_effect_document(document, asset_id, asset_id)

    def test_generated_rows_match_current_executable_parser_metadata(self) -> None:
        for asset_id in PRODUCT_ASSET_IDS:
            # Existence/closure is checked separately. Validate the metadata
            # of saved rows without restoring Elements the user deleted.
            for element_id, element in self._elements(asset_id).items():
                if not element_id.startswith(GENERATED_PREFIX):
                    continue
                with self.subTest(asset_id=asset_id, element_id=element_id):
                    self.assertLessEqual(len(element["displayName"].encode("utf-8")), 64)
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

    def test_terrain_3_symbol_preserves_the_users_deleted_floor_layers(self) -> None:
        asset_id = author.TERRAIN_3.effect_asset_id
        elements = self._elements(asset_id)
        floor_ids = (author.TERRAIN_3_SYMBOL_RING_ID,)
        for deleted_id in author.TERRAIN_3_FLOOR_IDS:
            self.assertNotIn(deleted_id, elements)
        self.assertTrue(set(floor_ids).issubset(elements))
        # The user deleted the old electric fan. The new DDS row is appended
        # independently, so Validate must not recreate that deleted element.
        self.assertNotIn(author.TERRAIN_3_FLOOR_IDS[2], elements)
        self.assertFalse(author.TERRAIN_3_REPLACED_SECTOR_IDS.intersection(elements))
        for element_id in floor_ids:
            element = elements[element_id]
            self.assertEqual(element["kind"], "particle")
            self.assertIn("authored-copy:", element["sourceNode"])
            self.assertIs(element["sourceRecipe"]["enabled"], False)

        # Check the actual saved Product row and physical DDS separately from
        # the temporary projection fixture's placeholder texture dependency.
        symbol = elements[author.TERRAIN_3_SYMBOL_RING_ID]
        resources = self._resources(symbol)
        for slot in ("base", "mask"):
            self.assertEqual(resources[slot], author.TERRAIN_3_SYMBOL_RING_TEXTURE)
        texture = ROOT / "Client/Bin/Resources" / author.TERRAIN_3_SYMBOL_RING_TEXTURE
        self.assertEqual(texture.read_bytes()[:4], b"DDS ")
        detail = symbol["detail"]
        self.assertEqual(detail["particle"]["startSize"], [0.75, 0.75])
        self.assertEqual(detail["particle"]["endSize"], [0.75, 0.75])
        self.assertEqual(detail["particle"]["lifeTimeSeconds"], [3.0, 3.0])
        self.assertEqual(detail["timing"]["lifeTimeSeconds"], 5.0)
        self.assertEqual(detail["uv"]["tileColumns"], 1)
        self.assertEqual(detail["uv"]["tileRows"], 1)
        self.assertEqual(detail["uv"]["tileIndex"], 0)
        self.assertFalse(detail["uv"]["sequence"])

        path = ROOT / author.TERRAIN_3.relative_path
        before = path.read_bytes()
        command = [sys.executable, str(SPEC.origin), "--scope", "terrain-3-symbol-ring", "--mode", "Validate"]
        result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, check=False)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("newElements=0", result.stdout)
        self.assertIn("changed=0", result.stdout)
        self.assertEqual(path.read_bytes(), before)




    def test_vertical_generated_rows_no_longer_depend_on_takeoff_legacy(self) -> None:
        cases = (
            (
                "effect.valtan.project-tuned.sequence.attack-whirlwind",
                f"{GENERATED_PREFIX}attack-whirlwind.jump-fan.vertical-core",
            ),
            (
                "effect.valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead",
                f"{GENERATED_PREFIX}struggling.large-vertical-burst",
            ),
        )
        for asset_id, element_id in cases:
            with self.subTest(asset_id=asset_id):
                if element_id not in self._elements(asset_id):
                    self.assertEqual(asset_id, author.STRUGGLING.effect_asset_id)
                    continue  # The user explicitly cleared this Effect to author a new Draft.
                element = self._element(asset_id, element_id)
                self.assertEqual(
                    element["sourceNode"],
                    "authored-copy:effect.valtan.high-jump.center-landing.active:"
                    "authored.copy.authored.copy.donut.impact.wave.black.1.1:"
                    f"{element_id}",
                )

    def test_struggling_clear_keeps_one_draft_without_restoring_generated_rows(self) -> None:
        asset_id = author.STRUGGLING.effect_asset_id
        self.assertFalse(any(
            element_id.startswith(f"{GENERATED_PREFIX}struggling.")
            for element_id in self._elements(asset_id)
        ))
        ownership = _load_json("Data/Effects/ValtanPatternAuthoringEffects.json")
        self.assertEqual(1, sum(
            row["patternId"] == "VALTAN_STRUGGLING"
            and row["effectAssetId"] == asset_id
            and row["state"] == "DRAFT_ATTACHED"
            for row in ownership["bindings"]
        ))
        pattern = next(row for row in self.presentation["patterns"]
                       if row["patternId"] == "VALTAN_STRUGGLING")
        self.assertTrue(all(not stage["effectCues"] for stage in pattern["stages"]))

    def test_charge_layers_follow_right_weapon_and_scroll_uv(self) -> None:
        asset_id = "effect.valtan.project-tuned.sequence.charge"
        runtime_slots: set[str] = set()
        attached_ids = {
            element_id
            for element_id in self._elements(asset_id)
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


    def test_saved_portal_particle_and_trash_bindings_and_floor_resources(self) -> None:
        portal_asset = "effect.valtan.project-tuned.sequence.warp.portal"
        portal_elements = self._elements(portal_asset)
        self.assertEqual(set(portal_elements), {"sprite_particle_2"})
        portal = portal_elements["sprite_particle_2"]
        self.assertEqual(portal.get("kind"), "particle")
        self.assertEqual(
            self._resources(portal).get("base"),
            "Effect/DimensionMaster/Textures/BG_OCN_ETC_J/"
            "bg_ocn_etc_magicsquare08a_d_kmk.dds",
        )
        self.assertStart(portal, 0.0)
        self.assertAlmostEqual(
            float(portal["detail"]["timing"]["lifeTimeSeconds"]), 5.0, places=6
        )
        particle = portal["detail"]["particle"]
        self.assertEqual(particle.get("maxParticles"), 1)
        self.assertEqual(particle.get("burstCount"), 1)
        self.assertEqual(particle.get("lifeTimeSeconds"), [2, 2])
        self.assertIs(portal.get("sourceRecipe", {}).get("enabled"), False)

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
                self.assertEqual(
                    cue.get("anchorSlotId"),
                    CUE_ANCHOR_EXPECTATIONS.get(cue_id, "root"),
                )
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
        promoted_pairs = {
            (pattern["patternId"], cue["effectAssetId"])
            for pattern in self.presentation.get("patterns", [])
            for stage in pattern.get("stages", [])
            for cue in stage.get("effectCues", [])
        }
        # New user-created Drafts are valid. Only an exact same-pattern
        # Product binding makes a Draft redundant and eligible for cleanup.
        self.assertFalse(
            actual.intersection(promoted_pairs),
            "A promoted Product must not also remain a Draft of the same pattern",
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

    def test_six_pizza_locks_one_player_and_facing_at_center(self) -> None:
        patterns = _index_unique(
            self.gameplay.get("patterns", []), "patternId", "gameplay pattern"
        )
        pizza = patterns["VALTAN_SIX_PIZZA_106"]
        self.assertEqual(pizza.get("targetPolicy"), "LOCK_RANDOM_ALIVE_ON_START")
        self.assertEqual(pizza.get("aimPolicy"), "LOCK_FACING_ON_START")
        self.assertIs(pizza.get("serverMotion", {}).get("moveToAnchorBeforeTakeoff"), True)


class ValtanRequestedEffectMetadataProjectionTest(unittest.TestCase):
    def test_seed_apply_then_saved_deletion_and_unlink_are_not_regenerated(self) -> None:
        generated = author._roar_charge_elements(
            _load_json(str(author.DONOR_ARENA_109)),
            _load_json(str(author.DONOR_CONE)),
            _load_json(str(author.DONOR_SKY_AXE)),
        )
        target = author.ROAR_CHARGE
        paths = [row.relative_path for row in author.PRODUCT_TARGETS] + [
            author.CATALOG_PATH, author.PATTERN_AUTHORING_BINDINGS_PATH,
            author.PRESENTATION_PATH, author.GAMEPLAY_PATH,
        ]
        with tempfile.TemporaryDirectory(prefix="valtan-saved-package.") as temporary:
            root = Path(temporary)
            for relative in paths:
                path = root / relative
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes((ROOT / relative).read_bytes())
            target_path = root / target.relative_path
            target_path.unlink()
            with mock.patch.object(author, "ROOT", root):
                guards: dict[Path, bytes | None] = {}
                seeded, count = author._stage_target(target, generated, guards)
                self.assertEqual(len(generated), count)
                self.assertGreater(count, 2)
                seed = author.Projection(
                    outputs={target.relative_path: author._output_bytes(target.relative_path, seeded, guards)},
                    guards=guards, appended_by_target={target.effect_asset_id: count},
                )
                author.apply_projection(seed)
                self.assertEqual(seeded, json.loads(target_path.read_bytes()))

                deleted_id = GENERATED_PREFIX + "roar-charge.cone.cyan"
                self.assertIn(deleted_id, {row["id"] for row in seeded["elements"]})
                saved = copy.deepcopy(seeded)
                saved["elements"] = [row for row in saved["elements"] if row["id"] != deleted_id]
                saved["elements"][0]["detail"]["timing"]["startDelaySeconds"] = 1.73125
                saved["elements"][0]["detail"]["color"]["emissiveIntensity"] = 7.25
                user_row = copy.deepcopy(saved["elements"][0])
                user_row["id"] = "authored.user.saved-package"
                user_row["displayName"] = "User-added row"
                saved["elements"].append(user_row)
                target_path.write_bytes(author._json_bytes(saved))

                presentation_path = root / author.PRESENTATION_PATH
                presentation = json.loads(presentation_path.read_bytes())
                pattern = next(row for row in presentation["patterns"] if row["patternId"] == target.pattern_id)
                self.assertTrue(any(stage["effectCues"] for stage in pattern["stages"]))
                for stage in pattern["stages"]:
                    stage["effectCues"] = []
                struggling = next(row for row in presentation["patterns"]
                                  if row["patternId"] == "VALTAN_STRUGGLING")
                for stage in struggling["stages"]:
                    stage["effectCues"] = []
                presentation_path.write_bytes(author._json_bytes(presentation))
                # Explicitly loading a preserved aggregate as Draft must survive
                # generator reruns even if it was promoted before its unlink.
                bindings_path = root / author.PATTERN_AUTHORING_BINDINGS_PATH
                bindings = json.loads(bindings_path.read_bytes())
                bindings["bindings"] = [row for row in bindings["bindings"]
                                        if row["patternId"] != "VALTAN_STRUGGLING"]
                bindings["bindings"].append({
                    "patternId": "VALTAN_STRUGGLING",
                    "effectAssetId": author.STRUGGLING.effect_asset_id,
                    "authoringPath": author.STRUGGLING.authoring_path,
                    "state": "DRAFT_ATTACHED",
                })
                bindings_path.write_bytes(author._json_bytes(bindings))
                before = {relative: (root / relative).read_bytes() for relative in paths}

                # No donor files were copied: saved Products must remain valid
                # even after the author deletes the original donor elements.
                with mock.patch.object(
                    author, "_initial_template_elements",
                    side_effect=AssertionError("saved package must not rebuild donor templates"),
                ):
                    projection = author.collect_projection()
                    self.assertEqual((), projection.changed_paths)
                    self.assertEqual(0, sum(projection.appended_by_target.values()))
                    author.apply_projection(projection)
                    author.validate_projection(author.collect_projection())
                self.assertEqual(before, {relative: (root / relative).read_bytes() for relative in paths})
                self.assertEqual(saved, json.loads(target_path.read_bytes()))
                self.assertNotIn(deleted_id, {row["id"] for row in saved["elements"]})

    def test_draft_cleanup_requires_a_current_product_in_the_same_pattern(self) -> None:
        draft = {
            "patternId": "VALTAN_STRUGGLING",
            "effectAssetId": author.STRUGGLING.effect_asset_id,
            "authoringPath": author.STRUGGLING.authoring_path,
            "state": "DRAFT_ATTACHED",
        }
        ownership = _load_json(str(author.PATTERN_AUTHORING_BINDINGS_PATH))
        ownership["bindings"] = [row for row in ownership["bindings"]
                                 if row["patternId"] != "VALTAN_STRUGGLING"]
        ownership["bindings"].append(draft)
        saved = copy.deepcopy(ownership)
        presentation = {"patterns": [{
            "patternId": "VALTAN_STRUGGLING", "stages": [{"effectCues": []}],
        }, {
            "patternId": "VALTAN_OTHER", "stages": [{"effectCues": [{
                "effectAssetId": author.STRUGGLING.effect_asset_id,
            }]}],
        }]}
        author._remove_promoted_draft_bindings(ownership, presentation)
        self.assertEqual(saved, ownership)
        presentation["patterns"][0]["stages"][0]["effectCues"] = [{
            "effectAssetId": author.STRUGGLING.effect_asset_id,
        }]
        author._remove_promoted_draft_bindings(ownership, presentation)
        self.assertEqual(saved["bindings"][:-1], ownership["bindings"])

    def test_clone_names_preserve_roles_within_the_executable_byte_limit(self) -> None:
        source = {"id": "source.metadata", "displayName": "Original", "detail": {"tuned": 7}}
        donor = {"effectAssetId": "effect.valtan.metadata", "elements": [source]}
        before = copy.deepcopy(donor)
        for role in ("short role", "a" * 52, "가" * 17):
            with self.subTest(role=role):
                rows = (
                    author._clone(donor, source["id"], GENERATED_PREFIX + "metadata", role),
                    author._clone_row(source, donor["effectAssetId"], GENERATED_PREFIX + "metadata", role),
                )
                for row in rows:
                    self.assertLessEqual(len(row["displayName"].encode("utf-8")), 64)
                    self.assertTrue(row["displayName"].endswith(role))
                    self.assertEqual(row["detail"], source["detail"])
                if role == "short role":
                    self.assertEqual(rows[0]["displayName"], "Requested 2026-08-27 / short role")
                else:
                    self.assertTrue(rows[0]["displayName"].startswith("Requested / "))
        self.assertEqual(donor, before)
        for role in ("a" * 53, "가" * 18, " "):
            with self.subTest(rejected_role=role):
                with self.assertRaisesRegex(author.AuthoringError, "64 UTF-8 bytes"):
                    author._clone(donor, source["id"], GENERATED_PREFIX + "metadata", role)
                with self.assertRaisesRegex(author.AuthoringError, "64 UTF-8 bytes"):
                    author._clone_row(source, donor["effectAssetId"], GENERATED_PREFIX + "metadata", role)
        self.assertEqual(donor, before)

    def test_metadata_repair_compacts_only_an_overlong_generated_prefix(self) -> None:
        role = "front-back-front broad electric fan electric-core"
        generated = {
            "id": GENERATED_PREFIX + "metadata",
            "displayName": "Requested 2026-08-27 / " + role,
            "sourceNode": "authored-copy:effect.source:source.metadata",
            "detail": {"timing": {"startDelay": 2.25}, "custom": "preserved"},
        }
        user = copy.deepcopy(generated)
        user["id"] = "authored.user.metadata"
        tuned = copy.deepcopy(generated)
        tuned["id"] = GENERATED_PREFIX + "user-renamed"
        tuned["displayName"] = "User tuned name"
        document = {"elements": [generated, user, tuned]}
        expected = copy.deepcopy(document)
        expected["elements"][0]["displayName"] = "Requested / " + role
        author._repair_generated_parser_metadata(document, "metadata fixture")
        self.assertEqual(document, expected)
        author._repair_generated_parser_metadata(document, "metadata fixture")
        self.assertEqual(document, expected)


class ValtanRequestedEffectStartProjectionTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.arena = _load_json(str(author.DONOR_ARENA_109))
        cls.cone = _load_json(str(author.DONOR_CONE))
        cls.sky = _load_json(str(author.DONOR_SKY_AXE))
        cls.hand = _load_json(str(author.DONOR_HAND_CORE))

    def test_generated_roar_cones_emit_at_the_authored_start_once(self) -> None:
        donor = _index_unique(self.cone["elements"], "id", "cone donor")[author.CONE_ID]
        original = copy.deepcopy(self.cone)
        rows = _index_unique(
            author._roar_charge_elements(self.arena, self.cone, self.sky),
            "id", "generated roar",
        )
        for suffix in ("dark", "cyan"):
            row = rows[f"{GENERATED_PREFIX}roar-charge.cone.{suffix}"]
            with self.subTest(element=row["id"]):
                recipe = row["sourceRecipe"]
                first_burst = min(burst["timeSeconds"] for burst in recipe["bursts"])
                first_emission = (
                    row["detail"]["timing"]["startDelaySeconds"]
                    + recipe["emitterDelaySeconds"] + first_burst
                )
                self.assertAlmostEqual(first_emission, 0.74, places=6)
                self.assertEqual(recipe, donor["sourceRecipe"])
        self.assertEqual(self.cone, original)

    def test_retiming_preserves_native_delay_and_all_other_source_fields(self) -> None:
        donor = _index_unique(self.cone["elements"], "id", "cone donor")[author.CONE_ID]
        for enabled in (True, False):
            with self.subTest(source_enabled=enabled):
                row = copy.deepcopy(donor)
                row["sourceRecipe"]["enabled"] = enabled
                row["sourceRecipe"]["emitterDelaySeconds"] = 0.1875
                original = copy.deepcopy(row)
                for start in (2.35, 0.0, 0.74):
                    author._set_start(row, start)
                    expected = copy.deepcopy(original)
                    expected["detail"]["timing"]["startDelaySeconds"] = start
                    self.assertEqual(row, expected)

    def test_composite_offset_does_not_reapply_the_donor_start(self) -> None:
        offset = 6.4
        donors = _index_unique(self.hand["elements"], "id", "hand donor")
        original = copy.deepcopy(self.hand)
        rows = author._hand_core_sequence(
            self.hand, "timing-test", "timing test", start_offset=offset,
        )
        for row, donor_id in zip(rows, author.HAND_CORE_IDS):
            with self.subTest(element=row["id"]):
                donor = donors[donor_id]
                donor_emitter_start = (
                    donor["detail"]["timing"]["startDelaySeconds"]
                    + donor["sourceRecipe"]["emitterDelaySeconds"]
                )
                actual_emitter_start = (
                    row["detail"]["timing"]["startDelaySeconds"]
                    + row["sourceRecipe"]["emitterDelaySeconds"]
                )
                self.assertAlmostEqual(actual_emitter_start, offset + donor_emitter_start, places=6)
                self.assertEqual(row["sourceRecipe"], donor["sourceRecipe"])
        self.assertEqual(self.hand, original)


class ValtanTerrain3FloorProjectionTest(unittest.TestCase):
    def setUp(self) -> None:
        temporary = tempfile.TemporaryDirectory(prefix="valtan-terrain3-floor-test.")
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.target = author.TERRAIN_3.relative_path
        for relative in (self.target, author.DONOR_FIST_IN_OUT, author.DONOR_FLOOR_ELECTRIC):
            path = self.root / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes((ROOT / relative).read_bytes())
        self.saved_before = self.read(self.target)
        self.before = copy.deepcopy(self.saved_before)
        self.before["elements"] = [
            row for row in self.before["elements"]
            if row["id"] not in {*author.TERRAIN_3_FLOOR_IDS, *author.TERRAIN_3_REPLACED_SECTOR_IDS}
        ]
        self.write(self.target, self.before)
        # An unrelated saved Effect must remain outside this projection.
        other = self.root / author.TERRAIN_9.relative_path
        other.write_bytes((ROOT / author.TERRAIN_9.relative_path).read_bytes())
        patcher = mock.patch.object(author, "ROOT", self.root)
        patcher.start()
        self.addCleanup(patcher.stop)

    def read(self, relative: Path) -> dict[str, Any]:
        return json.loads((self.root / relative).read_text(encoding="utf-8"))

    def write(self, relative: Path, document: dict[str, Any]) -> None:
        (self.root / relative).write_bytes(author._json_bytes(document))

    def snapshot(self) -> dict[Path, bytes]:
        return {p.relative_to(self.root): p.read_bytes() for p in self.root.rglob("*") if p.is_file()}

    def projected(self) -> tuple[Any, dict[str, Any]]:
        projection = author.collect_terrain_3_floor_projection()
        return projection, json.loads(projection.outputs[self.target])

    def prepare_symbol_target(self) -> None:
        self.symbol_before = copy.deepcopy(self.saved_before)
        self.symbol_before["elements"] = [
            row for row in self.symbol_before["elements"] if row["id"] != author.TERRAIN_3_SYMBOL_RING_ID
        ]
        self.write(self.target, self.symbol_before)
        self.symbol_texture = Path("Client/Bin/Resources") / author.TERRAIN_3_SYMBOL_RING_TEXTURE
        texture_path = self.root / self.symbol_texture
        texture_path.parent.mkdir(parents=True, exist_ok=True)
        # Copy existing DDS bytes only as a dependency/CAS fixture. This does
        # not create or admit the requested runtime texture or test its pixels.
        texture_path.write_bytes((ROOT / "Client/Bin/Resources/Effect/Warlord/Textures/FX_TEX_01/fx_c_symbol_003.dds").read_bytes())

    def symbol_projected(self) -> tuple[Any, dict[str, Any]]:
        projection = author.collect_terrain_3_symbol_ring_projection()
        return projection, json.loads(projection.outputs[self.target])

    def test_only_three_rows_are_added_without_restoring_deleted_landing(self) -> None:
        snapshot = self.snapshot()
        projection, document = self.projected()
        self.assertEqual(set(projection.outputs), {self.target})
        self.assertEqual(projection.appended_by_target, {author.TERRAIN_3.effect_asset_id: 3})
        old = self.before["elements"]
        self.assertEqual(document["elements"][:len(old)], old)
        self.assertEqual([r["id"] for r in document["elements"][len(old):]], list(author.TERRAIN_3_FLOOR_IDS))
        self.assertEqual({k: v for k, v in document.items() if k != "elements"},
                         {k: v for k, v in self.before.items() if k != "elements"})
        self.assertEqual(self.snapshot(), snapshot)

    def test_both_half_donuts_keep_circular_uv_mapping_and_donor_growth(self) -> None:
        _, document = self.projected()
        donor = _index_unique(self.read(author.DONOR_FIST_IN_OUT)["elements"], "id", "donor")
        rows = _index_unique(document["elements"], "id", "terrain")
        for row_id, donor_id in zip(author.TERRAIN_3_FLOOR_IDS[:2], (author.FIST_DONUT_OUTER_ID, author.FIST_DONUT_GROW_ID)):
            with self.subTest(element=row_id):
                row = rows[row_id]
                detail = row["detail"]
                particle = detail["particle"]
                uv = detail["uv"]
                self.assertEqual({r["slotId"]: r["assetId"] for r in row["resources"]},
                                 {"base": author.HALF_DONUT_TEXTURE, "mask": author.HALF_DONUT_TEXTURE})
                self.assertEqual((uv["tileColumns"], uv["tileRows"], uv["tileIndex"], uv["sequence"]), (1, 2, 0, False))
                self.assertEqual(uv["start"] + uv["speed"], [0, 0, 0, 0])
                self.assertEqual(particle["startSize"], particle["endSize"])
                width, height = particle["startSize"]
                self.assertEqual(width, 2 * height)
                self.assertEqual(particle["initialPositionMin"], [0, height / 2, 0])
                self.assertEqual(particle["initialPositionMin"], particle["initialPositionMax"])
                for local_v in (0.0, 0.25, 0.5, 1.0):
                    source_v = local_v / uv["tileRows"]
                    cropped_y = height * (0.5 - local_v) + particle["initialPositionMin"][1]
                    self.assertAlmostEqual(cropped_y, width * (0.5 - source_v))
                    self.assertGreaterEqual(cropped_y, 0.0)
                self.assertEqual(detail["transform"]["rotationDegrees"], [-90, 0, 0])
                self.assertEqual(detail["transform"]["position"][::2], [0, 0])
                for field in ("color", "linearLerp"):
                    self.assertEqual(detail[field], donor[donor_id]["detail"][field])
                self.assertEqual(detail["transform"]["scale"], donor[donor_id]["detail"]["transform"]["scale"])
                self.assertFalse(particle["billboard"])
                self.assertFalse(row["sourceRecipe"]["enabled"])

    def test_both_half_donut_particles_cover_the_five_second_element_life(self) -> None:
        _, document = self.projected()
        rows = _index_unique(document["elements"], "id", "terrain")
        for row_id in author.TERRAIN_3_FLOOR_IDS[:2]:
            with self.subTest(element=row_id):
                detail = rows[row_id]["detail"]
                life = detail["timing"]["lifeTimeSeconds"]
                self.assertEqual(life, 5.0)
                self.assertEqual(detail["particle"]["lifeTimeSeconds"], [life, life])

    def test_inner_growth_reaches_its_end_at_particle_expiry(self) -> None:
        _, document = self.projected()
        inner = next(row for row in document["elements"] if row["id"] == author.TERRAIN_3_FLOOR_IDS[1])
        detail = inner["detail"]
        self.assertTrue(detail["linearLerp"]["scale"])
        timing = detail["timing"]
        duration = timing.get("transformMotionDurationSeconds", 0) or timing["lifeTimeSeconds"]
        earliest_expiry = min(detail["particle"]["lifeTimeSeconds"])
        # Evaluate_ElementWorld normalizes root lerps by Motion Duration or Life.
        # A two-second particle previously vanished at 40% of this five-second lerp.
        progress_at_expiry = min(1.0, earliest_expiry / duration)
        self.assertEqual(earliest_expiry, duration)
        self.assertEqual(progress_at_expiry, 1.0)
        last_visible_progress = min(1.0, (earliest_expiry - 1.0 / 60.0) / duration)
        self.assertGreater(last_visible_progress, 0.99)
        for start, end in zip(detail["transform"]["scale"], detail["linearLerp"]["endScale"]):
            self.assertAlmostEqual(start + (end - start) * progress_at_expiry, end)

    def test_electric_changes_only_copy_identity_mask_and_sprite_roll(self) -> None:
        _, document = self.projected()
        actual = copy.deepcopy(document["elements"][-1])
        source = self.read(author.DONOR_FLOOR_ELECTRIC)
        expected = copy.deepcopy(next(r for r in source["elements"] if r["id"] == author.FLOOR_ELECTRIC_ID))
        for field in ("id", "displayName", "groupId", "sourceNode"):
            actual.pop(field)
            expected.pop(field)
        expected["resources"].append({"slotId": "mask", "assetId": SECTOR_04})
        expected["detail"]["sprite"]["billboardRollDegrees"] = 180.0
        self.assertEqual(actual, expected)
        self.assertEqual(actual["detail"]["color"]["emissiveIntensity"], 100)
        self.assertEqual(actual["material"]["renderProfile"], "alpha_two_sided_depth_read")

    def test_replacement_removes_only_exact_old_sector_ids(self) -> None:
        document = self.read(self.target)
        for identity in (*sorted(author.TERRAIN_3_REPLACED_SECTOR_IDS), f"{GENERATED_PREFIX}terrain-3.semicircle.sector-04"):
            row = copy.deepcopy(document["elements"][0])
            row["id"] = identity
            document["elements"].append(row)
        self.write(self.target, document)
        _, projected = self.projected()
        retained = [r for r in document["elements"] if r["id"] not in author.TERRAIN_3_REPLACED_SECTOR_IDS]
        self.assertEqual(projected["elements"][:-3], retained)

    def test_apply_then_validate_preserves_manual_tuning_and_bytes(self) -> None:
        before = self.snapshot()
        projection, _ = self.projected()
        author.apply_projection(projection)
        document = self.read(self.target)
        document["elements"][-1]["detail"]["color"]["emissiveIntensity"] = 42.5
        document["elements"][-2]["visible"] = False
        rows = _index_unique(document["elements"], "id", "terrain")
        # Existing legacy or manually tuned lifetimes must not be auto-retuned.
        rows[author.TERRAIN_3_FLOOR_IDS[0]]["detail"]["particle"]["lifeTimeSeconds"] = [2.0, 2.0]
        inner_detail = rows[author.TERRAIN_3_FLOOR_IDS[1]]["detail"]
        inner_detail["timing"]["lifeTimeSeconds"] = 7.0
        inner_detail["particle"]["lifeTimeSeconds"] = [3.0, 4.0]
        tuned_bytes = (json.dumps(document, indent=4) + "\n").encode("utf-8")
        (self.root / self.target).write_bytes(tuned_bytes)
        for _ in range(2):
            repeated = author.collect_terrain_3_floor_projection()
            self.assertEqual(repeated.changed_paths, ())
            author.apply_projection(repeated)
            author.validate_projection(repeated)
            self.assertEqual((self.root / self.target).read_bytes(), tuned_bytes)
        for relative, payload in before.items():
            if relative != self.target:
                self.assertEqual((self.root / relative).read_bytes(), payload)
        output = io.StringIO()
        with redirect_stdout(output):
            self.assertEqual(author.main(["--scope", "terrain-3-floor", "--mode", "Validate"]), 0)
        self.assertIn("scope=terrain-3-floor", output.getvalue())
        self.assertNotIn("sixPizzaTargeting", output.getvalue())

    def test_invalid_inputs_fail_without_writing(self) -> None:
        cases = (
            (self.target, lambda d: d.update(version=999)),
            (self.target, lambda d: d.update(effectAssetId="wrong.effect")),
            (self.target, lambda d: d["elements"].append(copy.deepcopy(d["elements"][0]))),
            (author.DONOR_FIST_IN_OUT, lambda d: d.update(version=12)),
            (author.DONOR_FLOOR_ELECTRIC, lambda d: d.update(elements=[])),
        )
        for relative, mutate in cases:
            with self.subTest(path=relative, mutation=mutate):
                path = self.root / relative
                original = path.read_bytes()
                document = self.read(relative)
                mutate(document)
                self.write(relative, document)
                snapshot = self.snapshot()
                with self.assertRaises(author.AuthoringError):
                    self.projected()
                self.assertEqual(self.snapshot(), snapshot)
                path.write_bytes(original)
        for relative in (self.target, author.DONOR_FLOOR_ELECTRIC):
            path = self.root / relative
            original = path.read_bytes()
            path.unlink()
            snapshot = self.snapshot()
            with self.assertRaisesRegex(author.AuthoringError, "cannot read required input"):
                self.projected()
            self.assertEqual(self.snapshot(), snapshot)
            path.write_bytes(original)
        with self.assertRaisesRegex(author.AuthoringError, "path escaped repository"):
            author._absolute(Path("../outside.effect.json"))

    def test_concurrent_save_is_rejected_for_target_and_both_donors(self) -> None:
        for relative in (self.target, author.DONOR_FIST_IN_OUT, author.DONOR_FLOOR_ELECTRIC):
            with self.subTest(path=relative):
                projection, _ = self.projected()
                path = self.root / relative
                original = path.read_bytes()
                path.write_bytes(original + b"\n")
                snapshot = self.snapshot()
                with self.assertRaisesRegex(author.AuthoringError, "input changed after staging"):
                    author.apply_projection(projection)
                self.assertEqual(self.snapshot(), snapshot)
                path.write_bytes(original)

    def test_failed_promotion_keeps_existing_product_and_cleans_staging(self) -> None:
        projection, _ = self.projected()
        snapshot = self.snapshot()
        with mock.patch.object(author.os, "replace", side_effect=OSError("injected promotion failure")):
            with self.assertRaisesRegex(author.AuthoringError, "all outputs rolled back"):
                author.apply_projection(projection)
        self.assertEqual(self.snapshot(), snapshot)
        self.assertFalse(list(self.root.glob(".valtan-requested-effects.*")))

    def test_symbol_ring_adds_only_one_row_and_keeps_existing_sectors_and_user_rows(self) -> None:
        self.prepare_symbol_target()
        document = self.read(self.target)
        old_sector = copy.deepcopy(document["elements"][0])
        old_sector["id"] = sorted(author.TERRAIN_3_REPLACED_SECTOR_IDS)[0]
        document["elements"].append(old_sector)
        self.write(self.target, document)
        snapshot = self.snapshot()
        projection, projected = self.symbol_projected()
        self.assertEqual(set(projection.outputs), {self.target})
        self.assertEqual(set(projection.guards), {self.target, author.DONOR_FIST_IN_OUT, self.symbol_texture})
        self.assertEqual(projection.appended_by_target, {author.TERRAIN_3.effect_asset_id: 1})
        self.assertEqual(projected["elements"][:-1], document["elements"])
        self.assertEqual(projected["elements"][-1]["id"], author.TERRAIN_3_SYMBOL_RING_ID)
        self.assertEqual({k: v for k, v in projected.items() if k != "elements"},
                         {k: v for k, v in document.items() if k != "elements"})
        self.assertEqual(self.snapshot(), snapshot)

    def test_symbol_ring_keeps_outer_red_tuning_with_full_uv_and_centered_geometry(self) -> None:
        self.prepare_symbol_target()
        _, document = self.symbol_projected()
        row = document["elements"][-1]
        donor = next(r for r in self.read(author.DONOR_FIST_IN_OUT)["elements"] if r["id"] == author.FIST_DONUT_OUTER_ID)
        detail = row["detail"]
        particle = detail["particle"]
        uv = detail["uv"]
        self.assertEqual({r["slotId"]: r["assetId"] for r in row["resources"]},
                         {"base": author.TERRAIN_3_SYMBOL_RING_TEXTURE, "mask": author.TERRAIN_3_SYMBOL_RING_TEXTURE})
        self.assertEqual(row["material"], donor["material"])
        self.assertEqual(detail["color"], donor["detail"]["color"])
        self.assertEqual(detail["transform"]["scale"], donor["detail"]["transform"]["scale"])
        self.assertAlmostEqual(detail["transform"]["position"][1], donor["detail"]["transform"]["position"][1] + 0.01)
        self.assertEqual(detail["transform"]["position"][::2], [0, 0])
        self.assertEqual(detail["transform"]["rotationDegrees"], [-90, 0, 0])
        self.assertEqual((uv["tileColumns"], uv["tileRows"], uv["tileIndex"], uv["sequence"], uv["wave"]), (1, 1, 0, False, False))
        self.assertEqual(uv["start"] + uv["speed"], [0, 0, 0, 0])
        self.assertEqual(particle["startSize"], [0.75, 0.75])
        self.assertEqual(particle["startSize"], particle["endSize"])
        self.assertEqual(particle["initialPositionMin"], [0, 0, 0])
        self.assertEqual(particle["initialPositionMin"], particle["initialPositionMax"])
        self.assertEqual((particle["maxParticles"], particle["burstCount"], particle["spawnRatePerSecond"]), (1, 1, 0))
        self.assertTrue(particle["localSpace"])
        self.assertFalse(particle["billboard"])
        self.assertFalse(row["sourceRecipe"]["enabled"])
        self.assertEqual(detail["timing"]["startDelaySeconds"], 0)
        self.assertEqual(detail["timing"]["lifeTimeSeconds"], 5)
        self.assertEqual(particle["lifeTimeSeconds"], [5, 5])
        self.assertTrue(row["sourceNode"].startswith("authored-copy:"))

    def test_symbol_ring_apply_and_validate_preserve_tuning_and_do_not_change_floor_scope(self) -> None:
        self.prepare_symbol_target()
        # Prepare the older floor in this fixture even when the checkout's
        # live Product has not received that separate projection yet.
        author.apply_projection(author.collect_terrain_3_floor_projection())
        before = self.snapshot()
        output = io.StringIO()
        with redirect_stdout(output):
            self.assertEqual(author.main(["--scope", "terrain-3-symbol-ring", "--mode", "Apply"]), 0)
        self.assertIn("targets=1, newElements=1, outputs=1", output.getvalue())
        document = self.read(self.target)
        row = document["elements"][-1]
        row["detail"]["color"]["emissiveIntensity"] = 9.5
        row["detail"]["uv"]["speed"] = [0.25, -0.5]
        row["detail"]["timing"]["lifeTimeSeconds"] = 7
        row["detail"]["particle"]["lifeTimeSeconds"] = [3, 4]
        row["visible"] = False
        tuned_bytes = (json.dumps(document, indent=4) + "\n").encode("utf-8")
        (self.root / self.target).write_bytes(tuned_bytes)
        for _ in range(2):
            repeated, _ = self.symbol_projected()
            self.assertEqual(repeated.changed_paths, ())
            self.assertEqual(repeated.appended_by_target, {author.TERRAIN_3.effect_asset_id: 0})
            author.apply_projection(repeated)
            author.validate_projection(repeated)
            self.assertEqual((self.root / self.target).read_bytes(), tuned_bytes)
        floor_projection = author.collect_terrain_3_floor_projection()
        self.assertEqual(floor_projection.changed_paths, ())
        for relative, payload in before.items():
            if relative != self.target:
                self.assertEqual((self.root / relative).read_bytes(), payload)
        output = io.StringIO()
        with redirect_stdout(output):
            self.assertEqual(author.main(["--scope", "terrain-3-symbol-ring", "--mode", "Validate"]), 0)
        self.assertIn("scope=terrain-3-symbol-ring", output.getvalue())
        self.assertNotIn("sixPizzaTargeting", output.getvalue())

    def test_symbol_ring_invalid_or_missing_inputs_fail_without_writing(self) -> None:
        self.prepare_symbol_target()
        cases = (
            (self.target, lambda d: d.update(version=999)),
            (self.target, lambda d: d.update(effectAssetId="wrong.effect")),
            (self.target, lambda d: d["elements"].append(copy.deepcopy(d["elements"][0]))),
            (author.DONOR_FIST_IN_OUT, lambda d: d.update(version=12)),
            (author.DONOR_FIST_IN_OUT, lambda d: d.update(effectAssetId="wrong.donor")),
            (author.DONOR_FIST_IN_OUT, lambda d: d.update(elements=[])),
        )
        for relative, mutate in cases:
            with self.subTest(path=relative, mutation=mutate):
                path = self.root / relative
                original = path.read_bytes()
                document = self.read(relative)
                mutate(document)
                self.write(relative, document)
                snapshot = self.snapshot()
                with self.assertRaises(author.AuthoringError):
                    self.symbol_projected()
                self.assertEqual(self.snapshot(), snapshot)
                path.write_bytes(original)
        for relative in (self.target, author.DONOR_FIST_IN_OUT, self.symbol_texture):
            with self.subTest(missing=relative):
                path = self.root / relative
                original = path.read_bytes()
                path.unlink()
                snapshot = self.snapshot()
                with self.assertRaisesRegex(author.AuthoringError, "cannot read required"):
                    self.symbol_projected()
                self.assertEqual(self.snapshot(), snapshot)
                path.write_bytes(original)

    def test_symbol_ring_detects_concurrent_target_donor_or_texture_change(self) -> None:
        self.prepare_symbol_target()
        for relative in (self.target, author.DONOR_FIST_IN_OUT, self.symbol_texture):
            with self.subTest(path=relative):
                projection, _ = self.symbol_projected()
                path = self.root / relative
                original = path.read_bytes()
                path.write_bytes(original + b"\n")
                snapshot = self.snapshot()
                with self.assertRaisesRegex(author.AuthoringError, "input changed after staging"):
                    author.apply_projection(projection)
                self.assertEqual(self.snapshot(), snapshot)
                path.write_bytes(original)

    def test_symbol_ring_failed_promotion_keeps_all_inputs_and_cleans_staging(self) -> None:
        self.prepare_symbol_target()
        projection, _ = self.symbol_projected()
        snapshot = self.snapshot()
        with mock.patch.object(author.os, "replace", side_effect=OSError("injected promotion failure")):
            with self.assertRaisesRegex(author.AuthoringError, "all outputs rolled back"):
                author.apply_projection(projection)
        self.assertEqual(self.snapshot(), snapshot)
        self.assertFalse(list(self.root.glob(".valtan-requested-effects.*")))


if __name__ == "__main__":
    unittest.main()
