from __future__ import annotations

import re
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]


def read_source(relative: str) -> str:
    payload = (REPO_ROOT / relative).read_bytes()
    try:
        source = payload.decode("utf-8")
    except UnicodeDecodeError:
        source = payload.decode("cp949")
    return source.replace("\r\n", "\n")


def braced_block(source: str, marker: str) -> str:
    marker_position = source.find(marker)
    if marker_position < 0:
        raise AssertionError(f"source marker is missing: {marker}")
    opening = source.find("{", marker_position)
    if opening < 0:
        raise AssertionError(f"source marker has no body: {marker}")
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[marker_position : index + 1]
    raise AssertionError(f"source marker has an unterminated body: {marker}")


class GroundTargetPreviewPrototypeScopeTests(unittest.TestCase):
    def test_static_registration_and_target_level_layer_use_distinct_scopes(self) -> None:
        static_registry = braced_block(
            read_source("Client/Private/MainApp.cpp"),
            "HRESULT CMainApp::Ready_Prototype_For_Static()",
        )
        self.assertRegex(
            static_registry,
            r"Add_Prototype\(\s*ETOUI\(LEVEL::STATIC\),\s*"
            r"CSkillGroundTargetPreview::PROTOTYPE_TAG,",
        )

        initialize = braced_block(
            read_source("Client/Private/PlayerController.cpp"),
            "bool_t Client::CPlayerController::Initialize_TargetingPreview(",
        )
        self.assertRegex(
            initialize,
            r"Add_GameObject_to_Layer\(\s*"
            r"ETOUI\(LEVEL::STATIC\),\s*"
            r"CSkillGroundTargetPreview::PROTOTYPE_TAG,\s*"
            r"levelIndex,\s*"
            r'TEXT\("Layer_SkillGroundTargetPreview"\)',
        )

    def test_every_product_level_initializes_the_shared_preview(self) -> None:
        consumers = {
            "Client/Private/Level_CharacterSelect.cpp": "CHARACTER_SELECT",
            "Client/Private/Level_Bern.cpp": "BERN",
            "Client/Private/Level_Development.cpp": "DEVELOPMENT",
            "Client/Private/Level_ValtanArena.cpp": "VALTAN_ARENA",
        }
        for relative, level_name in consumers.items():
            with self.subTest(level=level_name):
                source = read_source(relative)
                self.assertRegex(
                    source,
                    r"Initialize_TargetingPreview\(\s*"
                    + re.escape(f"ETOUI(LEVEL::{level_name})")
                    + r"\s*\)",
                )

    def test_local_replicated_character_receives_the_loaded_area_navigation(self) -> None:
        header = read_source("Client/Public/ClientReplication.h")
        self.assertIn("std::string strMapAreaId;", header)
        self.assertIn(
            "std::wstring m_strLocalPlayerNavigationPrototypeTag;", header
        )

        replication = read_source("Client/Private/ClientReplication.cpp")
        initialize = braced_block(
            replication,
            "bool Client::CClientReplication::Initialize(const DESC& desc)",
        )
        self.assertIn("desc.strMapAreaId.empty()", initialize)
        self.assertRegex(
            initialize,
            r"CMapNavigationContract::Resolve_Area\(\s*"
            r"desc\.strMapAreaId,\s*navigationContract,\s*navigationStatus\)",
        )
        self.assertIn("navigationContract.runtimeGridAvailable", initialize)

        create_character = braced_block(
            replication,
            "bool Client::CClientReplication::Create_Character(",
        )
        self.assertRegex(
            create_character,
            r"desc\.pNavigationPrototypeTag\s*=\s*"
            r"isLocallyControlled\s*\?\s*"
            r"m_strLocalPlayerNavigationPrototypeTag\.c_str\(\)\s*:\s*nullptr;",
        )
        self.assertNotIn("desc.pNavigationPrototypeTag = nullptr;", create_character)

        character = read_source("Client/Private/Character.cpp")
        character_initialize = braced_block(
            character,
            "HRESULT CCharacter::Initialize(void* pArg)",
        )
        self.assertRegex(
            character_initialize,
            r"nullptr\s*!=\s*pDesc->pNavigationPrototypeTag[\s\S]*"
            r"m_strNavigationPrototypeTag\s*=\s*"
            r"pDesc->pNavigationPrototypeTag;",
        )
        ready_components = braced_block(
            character,
            "HRESULT CCharacter::Ready_Components()",
        )
        self.assertRegex(
            ready_components,
            r"Add_Component\(\s*m_iPrototypeLevelIndex,\s*"
            r"m_strNavigationPrototypeTag,\s*TEXT\(\"Com_Navigation\"\),\s*"
            r"m_pNavigationCom\)",
        )
        sample_ground = braced_block(
            character,
            "bool_t CCharacter::Try_SampleTargetGround(",
        )
        self.assertIn("nullptr == m_pNavigationCom", sample_ground)
        self.assertIn("m_pNavigationCom->Try_SampleWalkablePoint", sample_ground)

        controller = read_source("Client/Private/PlayerController.cpp")
        update = braced_block(
            controller,
            "void Client::CPlayerController::Update(",
        )
        self.assertRegex(
            update,
            r"character->Try_SampleTargetGround\([\s\S]*"
            r"m_GroundTargeting.Apply_WalkableSample\(sampled\)",
        )
        self.assertRegex(
            update,
            r"confirmEdge\s*&&\s*m_GroundTargeting.Can_Confirm\(\)[\s\S]*"
            r"commandSink->Request_UseGroundTargetSkill\(\s*"
            r"m_iNextActionSequence,\s*"
            r"m_GroundTargeting.Get_SkillId\(\),\s*"
            r"target.x,\s*target.z\)",
        )

        consumers = [
            "Client/Private/Level_CharacterSelect.cpp",
            "Client/Private/Level_Bern.cpp",
            "Client/Private/Level_Development.cpp",
            "Client/Private/Level_ValtanArena.cpp",
        ]
        for relative in consumers:
            with self.subTest(consumer=relative):
                source = read_source(relative)
                self.assertRegex(
                    source,
                    r"(?:desc|replicationDesc)\.strMapAreaId\s*=\s*"
                    r"(?:entry|pEntry)->pMapAreaId;",
                )


if __name__ == "__main__":
    unittest.main()
