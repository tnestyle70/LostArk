from __future__ import annotations

import json
import unittest
from pathlib import Path

import sys as _cpp_domain_sys
from pathlib import Path as _CppDomainPath
_cpp_domain_sys.path.insert(0, str(_CppDomainPath(__file__).resolve().parents[1] / "Build"))
from cpp_source_domains import read_source_text


ROOT = Path(__file__).resolve().parents[2]


def _json(relative: str) -> dict:
    return json.loads((ROOT / relative).read_text(encoding="utf-8-sig"))


def _text(relative: str) -> str:
    return read_source_text(ROOT / relative, encoding="utf-8-sig")


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

    def test_part_break_recovery_spawns_no_rocks_in_source_or_product(self) -> None:
        gameplay = _json("Data/Valtan/Valtan.gameplay.json")
        pattern = _one(gameplay["patterns"], "patternId", "VALTAN_PART_BREAK")
        for stage in pattern["stages"]:
            self.assertFalse(any(
                event["kind"].startswith("SPAWN_COMBAT_OBJECT")
                for event in stage["events"]
            ))
        encounter = _json("Data/Encounters/Valtan/ValtanEncounter.json")
        product = _one(encounter["patterns"], "patternId", "VALTAN_PART_BREAK")
        for stage in product["stages"]:
            self.assertFalse(any(
                action["kind"].startswith("SPAWN_COMBAT_OBJECT")
                for action in stage.get("actions", [])
            ))
        recovery = _one(product["stages"], "stageId", "PART_BREAK_RECOVERY")
        self.assertEqual(5183, recovery["durationMs"])

    def test_part_break_rock_carrier_visual_and_sound_are_removed(self) -> None:
        archetype = "combatobject.valtan.part-break.rock"
        for path in ("Data/Valtan/Valtan.combatobjects.json",
                     "Data/Encounters/Valtan/ValtanCombatObjects.json"):
            self.assertNotIn(archetype, {
                row["combatObjectArchetypeId"] for row in _json(path)["objects"]
            })
        boss = _one(_json("Data/Actors/BossCatalog.json")["bosses"],
                    "archetypeId", "BOSS_VALTAN")
        self.assertNotIn(archetype, {
            row["combatObjectArchetypeId"] for row in boss["combatObjectVisuals"]
        })
        cues = _json("Data/Animation/Authored/Valtan/Valtan.combatobjectsoundcues.json")
        self.assertFalse(any(row["combatObjectArchetypeId"] == archetype
                             for row in cues["cues"]))
        pattern_cues = _json("Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json")
        self.assertFalse(any(
            row["patternId"] == "VALTAN_PART_BREAK"
            and row["soundEvent"] == "G_Voltan2_Attack09_ProjCreat1"
            for row in pattern_cues["cues"]
        ))
        # Original ground-roar rocks remain a separate, valid encounter visual.
        self.assertIn("combatobject.valtan.ground-roar.rock", {
            row["combatObjectArchetypeId"] for row in boss["combatObjectVisuals"]
        })

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
