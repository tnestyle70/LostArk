#!/usr/bin/env python3
"""Contract checks for replicated Esther action Sound presentation."""

from __future__ import annotations

import json
import os
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[2]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


class EstherActionSoundContractTests(unittest.TestCase):
    def test_catalog_assets_and_minimal_timing_evidence_are_exact(self) -> None:
        catalog = json.loads(read("Data/Sound/CharacterSoundCatalog.json"))
        document = json.loads(read("Data/Sound/EstherActionSoundCues.json"))
        self.assertEqual("lostark.esther-action-sound-cues", document["schema"])
        self.assertEqual(1, document["formatVersion"])
        self.assertEqual(30, document["fixedTickHz"])
        self.assertEqual(2, len(document["cues"]))

        expected = {
            ("PLAYER_ACTION", "PLAYER", "ESTHER_CAST", "PC_Common_FX_Active1"),
            ("NPC_ACTION", "NPC_59030", "esther.strike", "Silian1_Attack9_Cast1"),
        }
        self.assertEqual(expected, {
            (row["ownerKind"], row["ownerId"], row["actionId"], row["soundEvent"])
            for row in document["cues"]
        })
        for row in document["cues"]:
            self.assertEqual(0, row["startMs"])
            self.assertEqual("PROJECT_TUNED_EDGE", row["timingBasis"])
            self.assertTrue(row["once"])

        events = catalog["classes"]["Esther"]
        self.assertEqual({
            "PC_Common_FX_Active1", "PC_Common_FX_Active2",
            "Silian1_Attack9_Cast1", "Silian1_Attack9_Shot1",
            "Silian1_Attack9_Shot2",
        }, set(events))
        self.assertEqual(7, sum(map(len, events.values())))
        resource_root = Path(os.environ.get(
            "LOSTARK_RESOURCE_ROOT", ROOT / "Client" / "Bin" / "Resources"
        ))
        for variants in events.values():
            for asset_id in variants:
                self.assertTrue(asset_id.startswith("Sound/Asther/"))
                self.assertTrue(asset_id.endswith(".wav"))
                self.assertNotIn("..", Path(asset_id).parts)
                self.assertTrue((resource_root / Path(asset_id)).is_file(), asset_id)

        # These assets are admitted for later evidence-based authoring, but no
        # fabricated notify timing may make them fire today.
        bound = {row["soundEvent"] for row in document["cues"]}
        self.assertTrue({
            "PC_Common_FX_Active2", "Silian1_Attack9_Shot1",
            "Silian1_Attack9_Shot2",
        }.isdisjoint(bound))

    def test_runtime_uses_server_occurrence_ticks_and_deduplicates(self) -> None:
        header = read("Client/Public/EstherActionSoundCueDocument.h")
        runtime = read("Client/Private/EstherActionSoundCueDocument.cpp")
        character = read("Client/Private/Character.cpp")
        replication = read("Client/Private/ClientReplication.cpp")
        for token in (
            "iActionStartTick", "AttemptedCueIds", "lateToleranceMs",
            "CRuntimeAssetRoot::Resolve", "CSoundCueCatalog::Find_Variants",
            "CGameInstance::Get().Play_Sound", "std::error_code assetError",
            "is_regular_file(soundPath, assetError)",
            "Esther action Sound variant is missing from Resources",
        ):
            self.assertIn(token, header + runtime)
        self.assertIn('"PLAYER", "ESTHER_CAST", serverTick, actionStartTick', character)
        esther_branch = character.index(
            "else if (PLAYER_ACTION_STATE::ESTHER_CAST == action)"
        )
        esther_validation = character.index(
            "if (INVALID_SKILL_ID != skillId || 0u == actionStartTick)",
            esther_branch,
        )
        esther_play = character.index(
            "CEstherActionSoundCueDocument::Play_Due(", esther_branch
        )
        esther_same_edge = character.index(
            "if (m_eNetworkAction == action &&", esther_branch
        )
        self.assertLess(esther_validation, esther_play)
        self.assertLess(esther_play, esther_same_edge)
        self.assertIn("snapshot.iServerTick, entity.iActionStartTick", replication)
        self.assertIn("iter->second.strArchetypeId, entity.strActionId", replication)
        for forbidden in ("CNetworkManager", "Send_", "Change_Level"):
            self.assertNotIn(forbidden, runtime)

    def test_project_files_expose_one_canonical_data_copy(self) -> None:
        project = read("Client/Default/Client.vcxproj")
        filters = read("Client/Default/Client.vcxproj.filters")
        for relative in (
            r"..\Public\EstherActionSoundCueDocument.h",
            r"..\Private\EstherActionSoundCueDocument.cpp",
            r"..\..\Data\Sound\CharacterSoundCatalog.json",
            r"..\..\Data\Sound\EstherActionSoundCues.json",
        ):
            self.assertEqual(1, project.count(relative), relative)
            self.assertEqual(1, filters.count(relative), relative)


if __name__ == "__main__":
    unittest.main()
