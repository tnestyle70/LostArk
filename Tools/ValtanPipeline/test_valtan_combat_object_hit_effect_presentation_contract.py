from __future__ import annotations

import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def _read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8-sig")


def _function(source: str, signature: str, next_signature: str) -> str:
    start = source.index(signature)
    end = source.index(next_signature, start)
    return source[start:end]


class ValtanCombatObjectHitEffectPresentationContractTests(unittest.TestCase):
    def test_ground_roar_rock_is_a_timed_presentation_carrier(self) -> None:
        document = json.loads(
            _read("Data/Encounters/Valtan/ValtanCombatObjects.json")
        )
        rock = next(
            row for row in document["objects"]
            if row["combatObjectArchetypeId"]
            == "combatobject.valtan.ground-roar.rock"
        )
        self.assertEqual([], rock["hits"])
        self.assertEqual(
            [{
                "presentationEventId":
                    "pulse.valtan.ground-roar.rock.explode",
                "atMs": 5000,
            }],
            rock["presentationEvents"],
        )
        self.assertEqual(5000, rock["lifeMs"])

    def test_native_consumers_admit_presentation_only_carriers(self) -> None:
        tree = _read("Client/Private/ValtanPatternTree.cpp")
        sound = _read("Client/Private/ValtanCombatObjectSoundCueDocument.cpp")
        self.assertIn('Object.Find("presentationEvents")', tree)
        self.assertIn("const size_t iEventRowCount", tree)
        self.assertIn("0u == iEventRowCount || iEventRowCount > 16u", tree)
        self.assertIn("iEventOffsetMs > Reference->second.iLifetimeMs", tree)
        self.assertNotIn("hits->Get_Array().empty()", sound)

    def test_ground_roar_visual_owns_active_and_hit_effect_assets(self) -> None:
        catalog = json.loads(_read("Data/Actors/BossCatalog.json"))
        valtan = next(
            boss for boss in catalog["bosses"]
            if boss["archetypeId"] == "BOSS_VALTAN"
        )
        visual = next(
            row for row in valtan["combatObjectVisuals"]
            if row["combatObjectArchetypeId"]
            == "combatobject.valtan.ground-roar.rock"
        )
        self.assertEqual(
            {
                "combatObjectArchetypeId":
                    "combatobject.valtan.ground-roar.rock",
                "clientVisualId":
                    "combatobject.visual.valtan.ground-roar.rock.v1",
                "effectAssetId": "effect.valtan.ground-roar.rock.active",
                "hitEffectAssetId":
                    "effect.valtan.ground-roar.rock.explode",
            },
            visual,
        )

    def test_actor_catalog_parses_optional_typed_hit_effect(self) -> None:
        header = _read("Client/Public/ActorCatalog.h")
        source = _read("Client/Private/ActorCatalog.cpp")
        self.assertIn("std::string hitEffectAssetId;", header)
        self.assertIn('visual.Find("hitEffectAssetId")', source)
        self.assertIn("!IsStableId(pHitEffectAssetId->Get_String())", source)
        self.assertIn(
            "entryVisual.hitEffectAssetId =\n\t\t\t\t\t\tpHitEffectAssetId->Get_String();",
            source,
        )

    def test_valtan_hit_pulse_spawns_once_at_replicated_pose_then_keeps_sound(self) -> None:
        source = _read("Client/Private/Valtan.cpp")
        body = _function(
            source,
            "bool_t CValtan::Apply_CombatObjectPresentationEvent(",
            "void CValtan::Load_PatternShakeCues()",
        )
        dedupe = body.index(
            "event.iEventSequence <= m_iLastCombatObjectPresentationEventSequence"
        )
        spawn = body.index("CEffectPresentationService::Spawn_WorldRoot(")
        sound = body.index("CGameInstance::Get().Play_Sound(")
        self.assertLess(dedupe, spawn)
        self.assertLess(spawn, sound)
        self.assertIn("desc.strEffectAssetId = visual->hitEffectAssetId;", body)
        self.assertIn(
            "float3_t(event.fPositionX, event.fPositionY, event.fPositionZ)",
            body,
        )
        self.assertIn("event.fYawDegrees", body)
        self.assertIn("std::to_string(event.iEventSequence)", body)
        self.assertNotIn("Stop_WorldRoot", body)

    def test_level_entry_prewarms_active_and_hit_effects(self) -> None:
        for relative in (
            "Client/Private/Level_Loading.cpp",
            "Client/Private/Level_CharacterSelect.cpp",
        ):
            with self.subTest(relative=relative):
                source = _read(relative)
                self.assertIn(
                    "EffectAssetIds.push_back(Visual.effectAssetId);", source
                )
                self.assertIn(
                    "EffectAssetIds.push_back(Visual.hitEffectAssetId);", source
                )

    def test_effect_source_closure_counts_optional_hit_effect_as_reachable(self) -> None:
        validator = _read("Tools/EffectPipeline/validate_effect_sources.py")
        self.assertIn('hit_effect_asset_id = visual.get("hitEffectAssetId")', validator)
        self.assertIn(
            "BOSS_VALTAN combatObjectVisual {index}.hitEffectAssetId",
            validator,
        )

    def test_hit_effect_does_not_replace_combat_object_despawn(self) -> None:
        source = _read("Client/Private/ClientReplication.cpp")
        body = _function(
            source,
            "bool Client::CClientReplication::Apply_CombatObjectDespawn(",
            "bool Client::CClientReplication::Spawn_CombatObjectPresentation(",
        )
        self.assertIn("m_CombatObjectProjectionRuntime.Apply_Despawn(", body)
        runtime = _read("Client/Public/CombatObjectProjectionRuntime.h")
        self.assertIn("sink.Stop(record->second.iPresentationHandle);", runtime)
        self.assertIn("m_Records.erase(record);", runtime)

    def test_composition_shows_four_independent_instances_through_explosion_time(self) -> None:
        source = _read("Client/Private/ActionCompositionWorkbench.cpp")
        timeline = _function(
            source,
            "void Client::CActionCompositionWorkbench::Build_Timeline(",
            "void Client::CActionCompositionWorkbench::Pack_TimelineSubrows()",
        )
        self.assertIn(
            '"Combat Object x" + std::to_string(Object.iSpawnValue)',
            timeline,
        )
        self.assertIn("Object.iLifetimeMs", timeline)
        self.assertNotIn(
            "(std::min)(Object.iLifetimeMs, iStageDurationMs)", timeline
        )
        self.assertIn("iTimelineTailMs", timeline)
        self.assertIn("Item.iEndMs", timeline)


if __name__ == "__main__":
    unittest.main()
