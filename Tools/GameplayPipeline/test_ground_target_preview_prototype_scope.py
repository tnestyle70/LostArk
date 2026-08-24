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

    def test_ground_target_preview_mirrors_server_known_skill_availability(self) -> None:
        catalog_header = read_source("Client/Public/PlayerSkillCatalog.h")
        self.assertIn("std::uint32_t iResourceCost = 0;", catalog_header)
        self.assertIn("std::uint32_t iIdentityCost = 0;", catalog_header)

        catalog_load = braced_block(
            read_source("Client/Private/PlayerSkillCatalog.cpp"),
            "bool Client::CPlayerSkillCatalog::Load(",
        )
        self.assertRegex(
            catalog_load,
            r'Required\(\s*value,\s*"identityCost",\s*DATA_JSON_TYPE::NUMBER\)',
        )
        self.assertIn(
            "ReadU32(identityCost, definition.iIdentityCost)",
            catalog_load,
        )
        self.assertNotRegex(
            catalog_load,
            r"iIdentityCost\s*=\s*static_cast<std::uint32_t>",
        )

        hud_header = read_source("Client/Public/CombatHUDViewModel.h")
        self.assertIn(
            "LostArk::Shared::SKILL_ID iCurrentSkillId",
            hud_header,
        )
        hud_apply = braced_block(
            read_source("Client/Private/CombatHUDViewModel.cpp"),
            "void Client::CCombatHUDViewModel::Apply_LocalPlayer(",
        )
        self.assertIn(
            "m_Player.iCurrentSkillId = snapshot.iSkillId;",
            hud_apply,
        )

        controller = read_source("Client/Private/PlayerController.cpp")
        availability = braced_block(
            controller,
            "bool_t Is_GroundTargetSkillAvailable(",
        )
        for required_guard in (
            "!player.isValid",
            "player.isPreview",
            "0u == player.iCurrentHp",
            "player.eCharacterClass != skill.eCharacterClass",
            "player.iCurrentResource < skill.iResourceCost",
            "player.iCurrentIdentity < skill.iIdentityCost",
        ):
            with self.subTest(guard=required_guard):
                self.assertIn(required_guard, availability)
        self.assertRegex(
            availability,
            r"PLAYER_ACTION_STATE::NONE\s*==\s*player\.eAction[\s\S]*"
            r"PLAYER_ACTION_STATE::SKILL\s*==\s*player\.eAction[\s\S]*"
            r"Find_ById\(player\.iCurrentSkillId\)[\s\S]*"
            r"PLAYER_SKILL_KIND::COMBO\s*==\s*running->eSkillKind",
        )
        self.assertRegex(
            availability,
            r"player\.iCurrentResource\s*<\s*skill\.iResourceCost[\s\S]*"
            r"hudSkill\.iSkillId\s*==\s*skill\.iSkillId[\s\S]*"
            r"hudSkill\.Is_Ready\(player\.iServerTick\)",
        )
        self.assertNotIn("canStageAfterCombo || hudSkill.Is_Ready", availability)
        self.assertNotIn("!player.isCombatReady", availability)

        update = braced_block(
            controller,
            "void Client::CPlayerController::Update(",
        )
        self.assertRegex(
            update,
            r"Find_ById\(m_GroundTargeting\.Get_SkillId\(\)\)[\s\S]*"
            r"Is_GroundTargetSkillAvailable\(\*targetingDefinition,\s*playerState\)",
        )
        self.assertRegex(
            update,
            r"Is_GroundTargetSkillAvailable\(\s*\*requestedDefinition,\s*playerState\)"
            r"[\s\S]*m_GroundTargeting\.Begin\(",
        )

    def test_successful_ground_target_confirm_consumes_lmb_until_release(self) -> None:
        header = read_source("Client/Public/PlayerController.h")
        transaction = braced_block(
            header,
            "constexpr bool_t Try_Commit_GroundTargetConfirmation(",
        )
        contract = braced_block(
            header,
            "constexpr bool_t Validate_GroundTargetConfirmTransaction()",
        )
        update = braced_block(
            read_source("Client/Private/PlayerController.cpp"),
            "void Client::CPlayerController::Update(",
        )
        self.assertRegex(
            transaction,
            r"if \(!request\(\)\)\s*return false;[\s\S]*"
            r"basicAttackGate\.Suppress_UntilRelease\(\);[\s\S]*return true;",
        )
        self.assertIn("CONFIRM_REQUEST_STUB{ false }", contract)
        self.assertIn("CONFIRM_REQUEST_STUB{ true }", contract)
        self.assertIn("!gate.Observe_Button(true)", contract)
        self.assertIn("!gate.Observe_Button(false)", contract)
        self.assertIn(
            "static_assert(Validate_GroundTargetConfirmTransaction()", header
        )
        self.assertRegex(
            update,
            r"requestGroundTargetSkill\s*=\s*\[&\]\(\)[\s\S]*"
            r"Request_UseGroundTargetSkill\([\s\S]*"
            r"Try_Commit_GroundTargetConfirmation\(\s*"
            r"m_BasicAttackResendGate,\s*requestGroundTargetSkill\)[\s\S]*"
            r"Cancel_GroundTargeting\(\);",
        )

    def test_basic_attack_press_is_transactional_and_blocked_until_release(self) -> None:
        header = read_source("Client/Public/PlayerController.h")
        gate = braced_block(
            header,
            "class CBASIC_ATTACK_PRESS_EDGE_GATE final",
        )
        observe = braced_block(gate, "constexpr bool_t Should_Submit(")
        commit = braced_block(gate, "constexpr void Commit_Submission()")
        contract = braced_block(
            header,
            "constexpr bool_t Validate_BasicAttackPressTransitions()",
        )

        self.assertIn("m_isCurrentPressActive = false;", gate)
        self.assertIn("m_isCurrentPressConsumed = false;", gate)
        self.assertIn(
            "m_isCurrentPressConsumed = !isCommandEligible;",
            observe,
        )
        self.assertRegex(
            observe,
            r"else if \(!isCommandEligible\)\s*\{\s*"
            r"m_isCurrentPressConsumed = true;",
        )
        self.assertIn(
            "return isCommandEligible && !m_isCurrentPressConsumed;",
            observe,
        )
        self.assertNotIn("Commit_Submission", observe)
        self.assertRegex(
            commit,
            r"m_isCurrentPressActive\)\s*"
            r"m_isCurrentPressConsumed = true;",
        )
        self.assertIn("gate.Should_Submit(true, false)", contract)
        self.assertGreaterEqual(contract.count("gate.Should_Submit(true, true)"), 5)
        self.assertIn("gate.Commit_Submission();", contract)
        self.assertIn("gate.Should_Submit(false, false)", contract)
        self.assertIn(
            "static_assert(Validate_BasicAttackPressTransitions()", header
        )

        controller = read_source("Client/Private/PlayerController.cpp")
        poll = braced_block(
            controller,
            "void Client::CPlayerController::Poll_BasicAttack(",
        )
        skill_lookup = poll.index("CPlayerSkillCatalog::Find_BySlot(")
        eligibility = poll.index("const bool_t commandEligible")
        candidate = poll.index(
            "m_BasicAttackPressEdgeGate.Should_Submit(", eligibility
        )
        self.assertLess(skill_lookup, eligibility)
        self.assertLess(eligibility, candidate)
        self.assertIn("nullptr != pSkill", poll)

        request = braced_block(
            controller,
            "if (commandSink->Request_UseSkill(",
        )
        self.assertRegex(
            request,
            r"requestedBasicAttack\)\s*"
            r"m_BasicAttackPressEdgeGate\.Commit_Submission\(\);",
        )
        self.assertEqual(
            1,
            controller.count("m_BasicAttackPressEdgeGate.Commit_Submission();"),
        )


if __name__ == "__main__":
    unittest.main()
