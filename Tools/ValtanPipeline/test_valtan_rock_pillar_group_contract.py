#!/usr/bin/env python3
from __future__ import annotations

import json
import math
import copy
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import valtan_tuning_pipeline as pipeline


ROOT = Path(__file__).resolve().parents[2]


def row_by_id(rows: list[dict], field: str, value: str) -> dict:
    return next(row for row in rows if row[field] == value)


class ValtanRockPillarGroupContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.docs = pipeline.load_pipeline_documents(ROOT)
        cls.gameplay = cls.docs[pipeline.GAMEPLAY_AUTHORING_REL]
        cls.presentation = cls.docs[pipeline.PRESENTATION_AUTHORING_REL]
        cls.combat = cls.docs[pipeline.COMBAT_AUTHORING_REL]

    def assert_corner_layout(
        self,
        event: dict,
        half_extent_m: float,
        first_offset_ms: int,
        volley_policy: str = "BOSS_RELATIVE",
        layout_kind: str = "RADIAL_AROUND_BOSS",
    ) -> None:
        self.assertEqual(volley_policy, event["volleyPolicy"])
        self.assertEqual(4, event["countPerResolvedTarget"])
        self.assertEqual(layout_kind, event["layout"]["kind"])
        self.assertAlmostEqual(
            math.hypot(half_extent_m, half_extent_m),
            event["layout"]["radiusM"],
            places=9,
        )
        self.assertEqual(45.0, event["layout"]["startAngleDegrees"])
        self.assertEqual(90.0, event["layout"]["angleStepDegrees"])
        self.assertEqual("PROJECT_TUNED", event["layout"]["mappingBasis"])
        self.assertEqual(
            {
                "kind": "INTERVAL",
                "count": 1,
                "firstOffsetMs": first_offset_ms,
                "intervalMs": 0,
            },
            event["spawnSchedule"],
        )
        offsets = []
        radius = event["layout"]["radiusM"]
        for ordinal in range(4):
            radians = math.radians(45.0 + 90.0 * ordinal)
            offsets.append(
                (round(math.sin(radians) * radius, 6),
                 round(math.cos(radians) * radius, 6))
            )
        rounded_half_extent = round(half_extent_m, 6)
        self.assertEqual(
            {
                (-rounded_half_extent, -rounded_half_extent),
                (-rounded_half_extent, rounded_half_extent),
                (rounded_half_extent, -rounded_half_extent),
                (rounded_half_extent, rounded_half_extent),
            },
            set(offsets),
        )

    def test_pattern_relative_spawn_clocks_and_corner_geometry_are_exact(self) -> None:
        cases = (
            (
                "VALTAN_SIX_PIZZA_106",
                "STEP_01",
                "event.valtan.six-pizza.rock-pillars",
                "combatobject.valtan.six-pizza.rock-pillar",
                math.sqrt(50.0),
                1000,
                1000,
                "ARENA_CENTER",
                "RADIAL_AROUND_ARENA_CENTER",
            ),
            (
                "VALTAN_STRUGGLING",
                "STEP_04",
                "event.valtan.struggling.rock-pillars",
                "combatobject.valtan.struggling.rock-pillar",
                4.5,
                833,
                5000,
                "BOSS_RELATIVE",
                "RADIAL_AROUND_BOSS",
            ),
        )
        for (
            pattern_id,
            stage_id,
            event_id,
            object_id,
            half_extent,
            stage_offset,
            pattern_offset,
            volley_policy,
            layout_kind,
        ) in cases:
            with self.subTest(pattern_id=pattern_id):
                pattern = row_by_id(self.gameplay["patterns"], "patternId", pattern_id)
                stage_index = next(
                    index
                    for index, stage in enumerate(pattern["stages"])
                    if stage["stageId"] == stage_id
                )
                stage = pattern["stages"][stage_index]
                self.assertEqual(1, len(stage["events"]))
                event = stage["events"][0]
                self.assertEqual(event_id, event["eventId"])
                self.assertEqual(object_id, event["combatObjectArchetypeId"])
                self.assert_corner_layout(
                    event, half_extent, stage_offset, volley_policy, layout_kind
                )
                self.assertEqual(
                    pattern_offset,
                    sum(row["durationMs"] for row in pattern["stages"][:stage_index])
                    + stage_offset,
                )

    def test_each_rock_pattern_owns_its_own_v1_effect_documents(self) -> None:
        """Server clocks stay per archetype while each pattern edits its own V1
        active/explode documents; no shared V2 group overrides the lane."""
        expected = {
            "combatobject.valtan.ground-roar.rock": ("ground-roar", 6200, 5000),
            "combatobject.valtan.six-pizza.rock-pillar":
                ("six-pizza", 20700, 19500),
            "combatobject.valtan.struggling.rock-pillar":
                ("struggling", 6200, 5000),
        }
        for object_id, (_, life_ms, hit_at_ms) in expected.items():
            with self.subTest(object_id=object_id):
                combat_object = row_by_id(
                    self.combat["objects"], "combatObjectArchetypeId", object_id
                )
                self.assertEqual(life_ms, combat_object["lifetimeMs"])
                self.assertEqual(1.5, combat_object["coverRadiusM"])
                self.assertEqual([], combat_object["presentationEvents"])
                self.assertEqual(1, len(combat_object["hits"]))
                self.assertEqual(
                    hit_at_ms,
                    combat_object["hits"][0]["trigger"]["atMs"],
                )
                self.assertEqual(
                    "hit." + object_id.removeprefix("combatobject.") +
                    ".explode",
                    combat_object["hits"][0]["hitId"],
                )

        boss_catalog = json.loads(
            (ROOT / "Data/Actors/BossCatalog.json").read_text(encoding="utf-8-sig")
        )
        effect_catalog = json.loads(
            (ROOT / "Data/Effects/EffectCatalog.json").read_text(encoding="utf-8-sig")
        )
        catalog_paths = {
            row["effectAssetId"]: row["authoringPath"]
            for row in effect_catalog["effects"]
        }
        valtan = row_by_id(boss_catalog["bosses"], "archetypeId", "BOSS_VALTAN")
        for object_id, (pattern, _, _) in expected.items():
            with self.subTest(object_id=object_id):
                visual = row_by_id(
                    valtan["combatObjectVisuals"], "combatObjectArchetypeId", object_id
                )
                self.assertNotIn("effectV2Group", visual)
                active_id = f"effect.valtan.{pattern}.rock.active"
                explode_id = f"effect.valtan.{pattern}.rock.explode"
                self.assertEqual(active_id, visual["effectAssetId"])
                self.assertEqual(explode_id, visual["hitEffectAssetId"])
                for effect_id in (active_id, explode_id):
                    document = json.loads(
                        (ROOT / "Data" / catalog_paths[effect_id]).read_text(
                            encoding="utf-8-sig"
                        )
                    )
                    self.assertEqual(effect_id, document["effectAssetId"])

    def test_independent_resource_rows_and_sound_events_are_joinable(self) -> None:
        independent = {
            row["independentEffectId"]: row
            for row in self.presentation["independentEffects"]
        }
        expected = {
            "valtan.independent-effect.six-pizza-rock-pillars":
                "event.valtan.six-pizza.rock-pillars",
            "valtan.independent-effect.struggling-rock-pillars":
                "event.valtan.struggling.rock-pillars",
        }
        for independent_id, event_id in expected.items():
            self.assertEqual(event_id, independent[independent_id]["spawnEventId"])
            self.assertEqual("SERVER_COMBAT_OBJECT", independent[independent_id]["ownership"])

        sound_document = json.loads(
            (ROOT / "Data/Animation/Authored/Valtan/Valtan.combatobjectsoundcues.json")
            .read_text(encoding="utf-8")
        )
        sounds = {
            row["combatObjectArchetypeId"]: row
            for row in sound_document["cues"]
            if "rock-pillar" in row["combatObjectArchetypeId"]
        }
        self.assertEqual(
            {
                "combatobject.valtan.six-pizza.rock-pillar",
                "combatobject.valtan.struggling.rock-pillar",
            },
            set(sounds),
        )
        self.assertTrue(
            all(row["soundEvent"] == "G_Voltan2_Attack09_ProjExp1" for row in sounds.values())
        )
        self.assertTrue(all(
            row["hitId"] ==
            "hit." + row["combatObjectArchetypeId"].removeprefix(
                "combatobject."
            ) + ".explode"
            and "presentationEventId" not in row
            for row in sounds.values()
        ))

    def test_product_projection_preserves_delayed_first_wave(self) -> None:
        master = pipeline.join_v2_authoring(
            self.gameplay,
            self.presentation,
            self.docs[pipeline.WORLD_SET_REL],
            self.combat,
        )
        products = pipeline.project_v2_products(ROOT, self.docs, master)
        encounter = json.loads(products[pipeline.ENCOUNTER_REL])
        cases = (
            ("VALTAN_SIX_PIZZA_106", "STEP_01", 1000),
            ("VALTAN_STRUGGLING", "STEP_04", 833),
        )
        for pattern_id, stage_id, offset_ms in cases:
            pattern = row_by_id(encounter["patterns"], "patternId", pattern_id)
            stage = row_by_id(pattern["stages"], "stageId", stage_id)
            action = next(
                row for row in stage["actions"]
                if row["kind"] == "SPAWN_COMBAT_OBJECT_VOLLEY"
            )
            self.assertEqual(offset_ms, action["firstSpawnOffsetMs"])
            self.assertEqual(1, action["spawnCount"])
            self.assertEqual(0, action["spawnIntervalMs"])
            if pattern_id == "VALTAN_SIX_PIZZA_106":
                self.assertEqual("ARENA_CENTER", action["targetingPolicy"])
                self.assertEqual(10.0, action["radiusM"])
                self.assertEqual(45.0, action["startAngleDegrees"])

        combat_product = pipeline._compile_combat_products(
            master,
            self.combat,
            self.docs[pipeline.LEGACY_REL],
            self.docs[pipeline.BOSS_CATALOG_REL],
        )
        six_pizza = row_by_id(
            combat_product,
            "combatObjectArchetypeId",
            "combatobject.valtan.six-pizza.rock-pillar",
        )
        self.assertEqual(1.5, six_pizza["coverRadiusM"])
        self.assertEqual(19500, six_pizza["hits"][0]["atMs"])
        self.assertEqual(20700, six_pizza["lifeMs"])
        self.assertNotIn("presentationEvents", six_pizza)

        pizza_pattern = row_by_id(
            self.gameplay["patterns"], "patternId", "VALTAN_SIX_PIZZA_106"
        )
        spawn_event = pizza_pattern["stages"][0]["events"][0]
        spawn_at_pattern_ms = spawn_event["spawnSchedule"]["firstOffsetMs"]
        landing_stage_index = next(
            index
            for index, stage in enumerate(pizza_pattern["stages"])
            if stage["stageId"] == "STEP_07"
        )
        landing_at_pattern_ms = sum(
            stage["durationMs"]
            for stage in pizza_pattern["stages"][:landing_stage_index]
        ) + pizza_pattern["stages"][landing_stage_index]["hit"]["schedule"][
            "offsetsMs"
        ][0]
        explode_at_pattern_ms = spawn_at_pattern_ms + six_pizza["hits"][0]["atMs"]
        despawn_at_pattern_ms = spawn_at_pattern_ms + six_pizza["lifeMs"]
        self.assertEqual(19450, landing_at_pattern_ms)
        self.assertEqual(20500, explode_at_pattern_ms)
        self.assertEqual(21700, despawn_at_pattern_ms)
        self.assertLess(landing_at_pattern_ms, explode_at_pattern_ms)
        self.assertEqual(1200, despawn_at_pattern_ms - explode_at_pattern_ms)

        rebuilt = pipeline.build_combat_authoring({
            "schema": "lostark.valtan-combat-objects",
            "formatVersion": 1,
            "encounterId": "ENCOUNTER_VALTAN",
            "objects": [six_pizza],
        })
        rebuilt_six_pizza = row_by_id(
            rebuilt["objects"],
            "combatObjectArchetypeId",
            "combatobject.valtan.six-pizza.rock-pillar",
        )
        self.assertEqual(1.5, rebuilt_six_pizza["coverRadiusM"])
        self.assertEqual(20700, rebuilt_six_pizza["lifetimeMs"])

    def test_cover_radius_requires_a_timed_fixed_area_hit(self) -> None:
        invalid = copy.deepcopy(self.combat)
        rock = row_by_id(
            invalid["objects"],
            "combatObjectArchetypeId",
            "combatobject.valtan.ground-roar.rock",
        )
        rock["hits"][0]["trigger"] = {"kind": "CONTACT"}
        with self.assertRaisesRegex(
            pipeline.PipelineError,
            "coverRadiusM requires a fixed area with timed damage hits",
        ):
            pipeline.validate_combat_authoring(invalid)

    def test_server_projects_damaging_cover_sets_before_atomic_staging(self) -> None:
        source = (ROOT / "Server/Private/GameRoom.cpp").read_text(encoding="utf-8")
        start = source.index(
            "const bool damagingCoverVolleyMayProject ="
        )
        end = source.index(
            "if (count < 2u", start
        )
        guard = source[start:end]
        for required_guard in (
            "BOSS_COMBAT_OBJECT_KIND::FIXED_AREA",
            "BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NONE",
            "definition->fCoverRadiusM > 0.f",
            "!definition->Hits.empty()",
        ):
            self.assertIn(required_guard, guard)

        projection_start = source.index(
            "damagingCoverVolleyMayProject &&", end
        )
        projection_end = source.index(
            "if (!std::isfinite(resolvedPoint.x)", projection_start
        )
        projection = source[projection_start:projection_end]
        self.assertIn("Project_PointOnSameLevel", projection)
        self.assertIn("MAX_COVER_PROJECTION_METERS = 2.f", projection)
        self.assertIn("Is_PointWalkableExact(boundedX, boundedZ)", projection)
        self.assertIn("projectionDistance > 2.f", projection)
        self.assertIn("[DamagingCoverVolleySkipped]", projection)

        staging_start = source.index(
            "const std::size_t firstStagedObject =", projection_end
        )
        staging_end = source.index("break;", staging_start)
        staging = source[staging_start:staging_end]
        self.assertIn("combatObjectTransaction.Objects", staging)
        self.assertIn("combatObjectTransaction.Spawned", staging)
        self.assertIn("spawned.fPositionX = resolvedPoints[ordinal].x;", staging)
        self.assertIn("spawned.fPositionZ = resolvedPoints[ordinal].z;", staging)

    def test_live_v2_group_never_layers_the_v1_terminal_fallback(self) -> None:
        source = (ROOT / "Client/Private/Valtan.cpp").read_text(encoding="utf-8-sig")
        function = source[source.index("bool_t CValtan::Apply_CombatObjectPresentationEvent("):]
        function = function[: function.index("bool_t CValtan::Apply_BossCombatState(")]
        self.assertIn(
            "BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND::EFFECT_V1 ==",
            function,
        )
        self.assertIn("visual->activeEffectKind", function)

    def test_composition_timeline_shows_group_at_server_spawn_offset(self) -> None:
        header = (ROOT / "Client/Public/ValtanPatternTree.h").read_text(
            encoding="utf-8-sig"
        )
        tree = (ROOT / "Client/Private/ValtanPatternTree.cpp").read_text(
            encoding="utf-8-sig"
        )
        workbench = (
            ROOT / "Client/Private/ActionCompositionWorkbench.cpp"
        ).read_text(encoding="utf-8-sig")
        self.assertIn("std::string strEffectV2GroupId;", header)
        self.assertIn('Visual.Find("effectV2Group")', tree)
        self.assertIn("View.strEffectV2GroupId =", tree)
        self.assertIn("Object.iFirstSpawnOffsetMs", workbench)
        self.assertIn('"V2 Group "', workbench)
        self.assertIn('"Product Effect V2 Group: %s"', workbench)


if __name__ == "__main__":
    unittest.main()
