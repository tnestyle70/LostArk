from __future__ import annotations

import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def _read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8-sig")


def _json(relative: str) -> dict:
    return json.loads(_read(relative))


class ValtanRockPillarEffectV2GroupContractTests(unittest.TestCase):
    def test_reusable_group_owns_exact_five_second_then_explosion_sequence(self) -> None:
        group = _json(
            "Data/Effects/V2/Groups/"
            "boss.valtan.rock-pillar.sequence.effectv2group.json"
        )
        self.assertEqual("boss.valtan.rock-pillar.sequence", group["groupId"])
        self.assertEqual(6200, group["durationMs"])
        self.assertEqual(
            [
                {
                    "id": "boss.valtan.rock-pillar.active",
                    "start": 0,
                    "duration": 5000,
                    "stop": "Kill",
                },
                {
                    "id": "boss.valtan.rock-pillar.explosion",
                    "start": 5000,
                    "duration": 1200,
                    "stop": "Deactivate",
                },
            ],
            [
                {
                    "id": child["resource"]["id"],
                    "start": child["startMs"],
                    "duration": child["durationMs"],
                    "stop": child["stop"],
                }
                for child in group["children"]
            ],
        )

    def test_leaves_reuse_ground_roar_rock_resources_and_lifetimes(self) -> None:
        active = _json(
            "Data/Effects/V2/Authored/"
            "boss.valtan.rock-pillar.active.effectv2.json"
        )
        explosion = _json(
            "Data/Effects/V2/Authored/"
            "boss.valtan.rock-pillar.explosion.effectv2.json"
        )
        self.assertEqual("Mesh", active["effectType"])
        self.assertEqual(
            "Effect/Valtan/Meshes/FX_SM_00/fm_d_stoneparts_003.wmodel",
            active["slots"]["mesh"],
        )
        self.assertEqual(5.0, active["params"]["lifetime"])
        self.assertEqual("Particle", explosion["effectType"])
        self.assertEqual(
            "Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
            explosion["slots"]["mesh"],
        )
        self.assertEqual(1.2, explosion["params"]["lifetime"])
        self.assertEqual(16, explosion["params"]["particle"]["burstCount"])

    def test_group_is_independent_until_gameplay_owner_is_integrated(self) -> None:
        independent = _json("Data/Effects/V2/Independent.json")
        self.assertIn(
            "boss.valtan.rock-pillar.sequence", independent["groups"]
        )

    def test_boss_prefix_makes_group_visible_in_composition_catalog(self) -> None:
        workbench = _read("Client/Private/ActionCompositionWorkbench.cpp")
        self.assertIn(
            'return 0u == strResourceId.rfind("boss.valtan.", 0u);',
            workbench,
        )
        self.assertIn("pV2Snapshot->Get_Groups()", workbench)
        self.assertIn("m_EffectV2GroupIds.push_back(Group.strGroupId);", workbench)

    def test_actor_catalog_accepts_only_two_or_four_group_fields(self) -> None:
        header = _read("Client/Public/ActorCatalog.h")
        parser = _read("Client/Private/ActorCatalog.cpp")
        self.assertIn("bool_t bHasHitSync = false;", header)
        self.assertIn("bHasVisualHitMs != bHasServerHitId", parser)
        self.assertIn("(bHasHitSync ? 4u : 2u)", parser)
        self.assertIn(
            "entryVisual.effectV2Group.bHasHitSync = bHasHitSync;", parser
        )

    def test_product_runtime_consumes_only_group_identity_and_rate(self) -> None:
        replication = _read("Client/Private/ClientReplication.cpp")
        begin = replication.index(
            "bool Client::CClientReplication::Spawn_CombatObjectPresentation("
        )
        end = replication.index(
            "bool Client::CClientReplication::Update_CombatObjectPresentation(",
            begin,
        )
        body = replication[begin:end]
        self.assertIn("visual->effectV2Group.groupId", body)
        self.assertIn("visual->effectV2Group.playbackRate", body)
        self.assertNotIn("visualHitMs", body)
        self.assertNotIn("serverHitId", body)

    def test_existing_sky_axe_keeps_complete_hit_sync_contract(self) -> None:
        catalog = _json("Data/Actors/BossCatalog.json")
        valtan = next(
            boss for boss in catalog["bosses"]
            if boss["archetypeId"] == "BOSS_VALTAN"
        )
        axe = next(
            visual for visual in valtan["combatObjectVisuals"]
            if visual["combatObjectArchetypeId"]
            == "combatobject.valtan.high-jump.target-axe"
        )
        self.assertEqual(
            {
                "groupId",
                "playbackRate",
                "visualHitMs",
                "serverHitId",
            },
            set(axe["effectV2Group"]),
        )


if __name__ == "__main__":
    unittest.main()
