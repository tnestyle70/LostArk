from __future__ import annotations

import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def _json(relative: str) -> dict:
    return json.loads((ROOT / relative).read_text(encoding="utf-8-sig"))


def _text(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8-sig")


def _one(rows: list[dict], key: str, value: str) -> dict:
    matches = [row for row in rows if row.get(key) == value]
    if len(matches) != 1:
        raise AssertionError(f"expected one {key}={value}, got {len(matches)}")
    return matches[0]


class ValtanPartBreakRecoveryContractTests(unittest.TestCase):
    def test_canonical_timeline_is_the_complete_6983ms_four_clip_chain(self) -> None:
        gameplay = _json("Data/Valtan/Valtan.gameplay.json")
        presentation = _json("Data/Valtan/Valtan.presentation.json")
        pattern = _one(gameplay["patterns"], "patternId", "VALTAN_PART_BREAK")
        visual = _one(
            presentation["patterns"], "patternId", "VALTAN_PART_BREAK"
        )

        self.assertEqual(
            [
                (
                    "PART_BREAK",
                    "valtan.attack.dash-charge.part-break",
                    "PART_BREAK",
                    1800,
                    "valtan.reaction.part-break.recovery",
                ),
                (
                    "PART_BREAK_RECOVERY",
                    "valtan.reaction.part-break.recovery",
                    "RECOVERY",
                    5183,
                    None,
                ),
            ],
            [
                (
                    stage["stageId"],
                    stage["actionId"],
                    stage["stageKind"],
                    stage["durationMs"],
                    stage["defaultNextActionId"],
                )
                for stage in pattern["stages"]
            ],
        )
        self.assertEqual(6983, sum(stage["durationMs"] for stage in pattern["stages"]))
        self.assertEqual(
            [
                (
                    "PART_BREAK",
                    "EXACT",
                    [
                        ("mesh_dmg_parts_start_1", 1400),
                        ("mesh_dmg_parts_loop_1", 400),
                    ],
                ),
                (
                    "PART_BREAK_RECOVERY",
                    "EXACT",
                    [
                        ("mesh_dmg_parts_end_1", 2850),
                        ("mesh_idle_battle_1", 2333),
                    ],
                ),
            ],
            [
                (
                    stage["stageId"],
                    stage["animation"]["endPolicy"],
                    [
                        (occurrence["clip"], occurrence["playMs"])
                        for occurrence in stage["animation"]["occurrences"]
                    ],
                )
                for stage in visual["stages"]
            ],
        )
        for stage in visual["stages"]:
            self.assertEqual(1, stage["animation"]["repeatCount"])
            self.assertTrue(all(
                occurrence["mappingBasis"] == "PATTERN_PR_REFERENCE"
                and occurrence["sourceStartMs"] == 0
                and occurrence["playRate"] == 1.0
                and occurrence["repeatUntilStageEnd"] is False
                for occurrence in stage["animation"]["occurrences"]
            ))

    def test_recovery_enter_owns_one_exact_boss_relative_cardinal_volley(self) -> None:
        gameplay = _json("Data/Valtan/Valtan.gameplay.json")
        pattern = _one(gameplay["patterns"], "patternId", "VALTAN_PART_BREAK")
        recovery = _one(
            pattern["stages"], "stageId", "PART_BREAK_RECOVERY"
        )
        self.assertEqual(
            [{
                "eventId": "valtan.part-break.cardinal-rocks",
                "trigger": "ENTER",
                "kind": "SPAWN_COMBAT_OBJECT_VOLLEY",
                "combatObjectArchetypeId":
                    "combatobject.valtan.part-break.rock",
                "volleyPolicy": "BOSS_RELATIVE",
                "countPerResolvedTarget": 4,
                "layout": {
                    "kind": "RADIAL_AROUND_BOSS",
                    "radiusM": 4.9497475,
                    "startAngleDegrees": 45.0,
                    "angleStepDegrees": 90.0,
                    "mappingBasis": "PROJECT_TUNED",
                },
                "spawnSchedule": {
                    "kind": "INTERVAL",
                    "count": 1,
                    "firstOffsetMs": 0,
                    "intervalMs": 0,
                },
                "arenaRandom": {"kind": "NONE"},
                "allowOverlap": False,
                "maximumTotalObjects": 4,
            }],
            recovery["events"],
        )

        encounter = _json("Data/Encounters/Valtan/ValtanEncounter.json")
        product_pattern = _one(
            encounter["patterns"], "patternId", "VALTAN_PART_BREAK"
        )
        product_recovery = _one(
            product_pattern["stages"], "stageId", "PART_BREAK_RECOVERY"
        )
        self.assertEqual(5183, product_recovery["durationMs"])
        self.assertEqual(
            {
                "trigger": "ENTER",
                "kind": "SPAWN_COMBAT_OBJECT_VOLLEY",
                "targetId": "combatobject.valtan.part-break.rock",
                "targetingPolicy": "BOSS_RELATIVE",
                "countPerResolvedTarget": 4,
                "layout": "RADIAL",
                "radiusM": 4.9497475,
                "startAngleDegrees": 45.0,
                "angleStepDegrees": 90.0,
                "allowOverlap": False,
                "maximumTotalObjects": 4,
                "spawnCount": 1,
                "firstSpawnOffsetMs": 0,
                "spawnIntervalMs": 0,
                "arenaRandomCount": 0,
                "arenaRandomRadiusM": 0,
                "arenaHeightToleranceM": 0,
                "arenaAnchorPolicy": "NONE",
            },
            product_recovery["actions"][0],
        )

    def test_distinct_carrier_has_exact_owner_damage_visual_and_sound(self) -> None:
        authoring = _json("Data/Valtan/Valtan.combatobjects.json")
        authored = _one(
            authoring["objects"],
            "combatObjectArchetypeId",
            "combatobject.valtan.part-break.rock",
        )
        self.assertEqual(6200, authored["lifetimeMs"])
        self.assertEqual(1.5, authored["coverRadiusM"])
        self.assertEqual([], authored["presentationEvents"])
        self.assertEqual(1, len(authored["hits"]))
        authored_hit = authored["hits"][0]
        self.assertEqual(
            "hit.valtan.part-break.rock.explode", authored_hit["hitId"]
        )
        self.assertEqual({"kind": "TIMED", "atMs": 5000}, authored_hit["trigger"])
        self.assertEqual({"kind": "CIRCLE", "outerRadiusM": 3.0}, authored_hit["shape"])
        self.assertEqual("damage.valtan.stomp", authored_hit["serverDamageProfileId"])

        product = _json("Data/Encounters/Valtan/ValtanCombatObjects.json")
        carrier = _one(
            product["objects"],
            "combatObjectArchetypeId",
            "combatobject.valtan.part-break.rock",
        )
        self.assertEqual("VALTAN_PART_BREAK", carrier["ownerPatternId"])
        self.assertEqual(
            "valtan.reaction.part-break.recovery",
            carrier["ownerStageActionId"],
        )
        self.assertEqual("FIXED_AREA", carrier["kind"])
        self.assertEqual("BOSS_POSITION", carrier["originPolicy"])
        self.assertEqual("NONE", carrier["directionPolicy"])
        self.assertEqual(6200, carrier["lifeMs"])
        self.assertEqual(1.5, carrier["coverRadiusM"])
        self.assertNotIn("presentationEvents", carrier)
        self.assertEqual(1, len(carrier["hits"]))
        carrier_hit = carrier["hits"][0]
        self.assertEqual(
            "hit.valtan.part-break.rock.explode", carrier_hit["hitId"]
        )
        self.assertEqual("TIMED", carrier_hit["trigger"])
        self.assertEqual(5000, carrier_hit["atMs"])
        self.assertEqual("CIRCLE", carrier_hit["hitShape"])
        self.assertEqual(3.0, carrier_hit["hitOuterRadius"])
        self.assertEqual("damage.valtan.stomp", carrier_hit["serverDamageProfileId"])

        catalog = _json("Data/Actors/BossCatalog.json")
        boss = _one(catalog["bosses"], "archetypeId", "BOSS_VALTAN")
        visual = _one(
            boss["combatObjectVisuals"],
            "combatObjectArchetypeId",
            "combatobject.valtan.part-break.rock",
        )
        self.assertEqual(
            {
                "combatObjectArchetypeId":
                    "combatobject.valtan.part-break.rock",
                "clientVisualId":
                    "combatobject.visual.valtan.part-break.rock.v1",
                "effectAssetId": "effect.valtan.ground-roar.rock.active",
                "hitEffectAssetId":
                    "effect.valtan.ground-roar.rock.explode",
            },
            visual,
        )

        object_cues = _json(
            "Data/Animation/Authored/Valtan/"
            "Valtan.combatobjectsoundcues.json"
        )
        terminal = _one(
            object_cues["cues"],
            "bindingId",
            "cue.sound.valtan.combatobject.part-break.rock.explode.01",
        )
        self.assertEqual(
            {
                "bindingId":
                    "cue.sound.valtan.combatobject.part-break.rock.explode.01",
                "combatObjectArchetypeId":
                    "combatobject.valtan.part-break.rock",
                "hitId": "hit.valtan.part-break.rock.explode",
                "soundBank": "S_Mob_G_Voltan2",
                "soundEvent": "G_Voltan2_Attack09_ProjExp1",
            },
            terminal,
        )

        pattern_cues = _json(
            "Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json"
        )
        creation = _one(
            pattern_cues["cues"],
            "bindingId",
            "cue.sound.valtan.reaction.part-break.recovery.clip-01.01",
        )
        self.assertEqual("VALTAN_PART_BREAK", creation["patternId"])
        self.assertEqual("PART_BREAK_RECOVERY", creation["stageId"])
        self.assertEqual(
            "valtan.reaction.part-break.recovery", creation["actionId"]
        )
        self.assertEqual(
            "valtan.reaction.part-break.recovery.clip.01",
            creation["clipOccurrenceId"],
        )
        self.assertEqual("G_Voltan2_Attack09_ProjCreat1", creation["soundEvent"])
        self.assertEqual(1, creation["startMs"])

        sound_catalog = _json("Data/Sound/CharacterSoundCatalog.json")
        bank = sound_catalog["classes"]["Valtan"]
        self.assertEqual(4, len(bank["G_Voltan2_Attack09_ProjCreat1"]))
        self.assertEqual(4, len(bank["G_Voltan2_Attack09_ProjExp1"]))

    def test_runtime_projects_damage_cover_and_uses_one_hit_pulse(self) -> None:
        room = _text("Server/Private/GameRoom.cpp")
        start = room.index("damagingCoverVolleyMayProject")
        body = room[start:room.index(
            "if (count < 2u", start
        )]
        for token in (
            "BOSS_COMBAT_OBJECT_KIND::FIXED_AREA",
            "BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NONE",
            "definition->fCoverRadiusM > 0.f",
            "!definition->Hits.empty()",
        ):
            self.assertIn(token, body)
        projection = room[room.index(
            "damagingCoverVolleyMayProject &&", start
        ):room.index("if (!std::isfinite(resolvedPoint.x)", start)]
        self.assertIn("Project_PointOnSameLevel", projection)
        self.assertIn("MAX_COVER_PROJECTION_METERS = 2.f", projection)
        self.assertIn("projectionDistance > MAX_COVER_PROJECTION_METERS", projection)
        self.assertIn(
            "m_ServerNavigation.Is_PointWalkableExact(boundedX, boundedZ)",
            projection,
        )
        self.assertIn("projectionDistance > 2.f", projection)

        runtime = _text("Server/Private/CombatObjectRuntime.cpp")
        self.assertIn("event.strHitId = pulseId;", runtime)
        self.assertIn(
            "QueuePresentationPulse(hit.strHitId, hit.iAppliedTimedCount);",
            runtime,
        )

        loader = _text(
            "Client/Private/ValtanCombatObjectSoundCueDocument.cpp"
        )
        self.assertIn("bHasHitId == bHasPresentationEventId", loader)
        self.assertIn("!productSources.contains(sourceKey)", loader)
        client = _text("Client/Private/Valtan.cpp")
        apply_at = client.index("bool_t CValtan::Apply_CombatObjectPresentationEvent(")
        apply_body = client[apply_at:client.index(
            "void CValtan::Load_PatternShakeCues()", apply_at
        )]
        self.assertLess(
            apply_body.index("CEffectPresentationService::Spawn_WorldRoot("),
            apply_body.index("CGameInstance::Get().Play_Sound("),
        )
        self.assertIn(
            "event.strCombatObjectArchetypeId, event.strHitId", apply_body
        )


if __name__ == "__main__":
    unittest.main()
