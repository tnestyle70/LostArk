#!/usr/bin/env python3
from __future__ import annotations

import json
import math
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
        self, event: dict, half_extent_m: float, first_offset_ms: int
    ) -> None:
        self.assertEqual("BOSS_RELATIVE", event["volleyPolicy"])
        self.assertEqual(4, event["countPerResolvedTarget"])
        self.assertEqual("RADIAL_AROUND_BOSS", event["layout"]["kind"])
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
        self.assertEqual(
            {
                (-half_extent_m, -half_extent_m),
                (-half_extent_m, half_extent_m),
                (half_extent_m, -half_extent_m),
                (half_extent_m, half_extent_m),
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
                7.0,
                1000,
                1000,
            ),
            (
                "VALTAN_STRUGGLING",
                "STEP_04",
                "event.valtan.struggling.rock-pillars",
                "combatobject.valtan.struggling.rock-pillar",
                3.5,
                833,
                5000,
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
                self.assert_corner_layout(event, half_extent, stage_offset)
                self.assertEqual(
                    pattern_offset,
                    sum(row["durationMs"] for row in pattern["stages"][:stage_index])
                    + stage_offset,
                )

    def test_each_independent_object_owns_the_same_6200ms_group_clock(self) -> None:
        group = json.loads(
            (ROOT / "Data/Effects/V2/Groups/boss.valtan.rock-pillar.sequence.effectv2group.json")
            .read_text(encoding="utf-8")
        )
        self.assertEqual("boss.valtan.rock-pillar.sequence", group["groupId"])
        self.assertEqual(6200, group["durationMs"])
        self.assertEqual(
            [
                ("boss.valtan.rock-pillar.active", 0, 5000),
                ("boss.valtan.rock-pillar.explosion", 5000, 1200),
            ],
            [
                (child["resource"]["id"], child["startMs"], child["durationMs"])
                for child in group["children"]
            ],
        )
        object_ids = (
            "combatobject.valtan.ground-roar.rock",
            "combatobject.valtan.six-pizza.rock-pillar",
            "combatobject.valtan.struggling.rock-pillar",
        )
        for object_id in object_ids:
            with self.subTest(object_id=object_id):
                combat_object = row_by_id(
                    self.combat["objects"], "combatObjectArchetypeId", object_id
                )
                self.assertEqual(6200, combat_object["lifetimeMs"])
                self.assertEqual(5000, combat_object["presentationEvents"][0]["trigger"]["atMs"])

        boss_catalog = json.loads(
            (ROOT / "Data/Actors/BossCatalog.json").read_text(encoding="utf-8-sig")
        )
        valtan = row_by_id(boss_catalog["bosses"], "archetypeId", "BOSS_VALTAN")
        for object_id in object_ids:
            visual = row_by_id(
                valtan["combatObjectVisuals"], "combatObjectArchetypeId", object_id
            )
            self.assertEqual(
                {"groupId": "boss.valtan.rock-pillar.sequence", "playbackRate": 1.0},
                visual["effectV2Group"],
            )
            self.assertEqual("effect.valtan.ground-roar.rock.active", visual["effectAssetId"])
            self.assertEqual("effect.valtan.ground-roar.rock.explode", visual["hitEffectAssetId"])

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
