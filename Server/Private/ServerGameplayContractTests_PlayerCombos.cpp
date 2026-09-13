#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests_PlayerSkillFixtures.h"
#include "ServerGameplayContractTests.h"
#include "ClientSession.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "PlayerSkillSystem.h"
#include "Network/PacketReader.h"
#include "WorldBootstrap.h"
#include "WorldDestructionBootstrapContractTests.h"
#include <Windows.h>
#include <process.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>


using namespace LostArk::Server;
using namespace LostArk::Shared;

void LostArk::Server::CServerGameplayContractRunner::Run_PlayerCombos(TESTS& tests, CGameplayCatalog& catalog)
{

	for (const BASIC_ATTACK_CONTRACT& contract : BASIC_ATTACKS)
	{
		const PLAYER_SKILL_DEFINITION* combo = catalog.Find_Skill(contract.skillId);
		bool stageTimingsValid = nullptr != combo &&
			combo->ComboStages.size() == contract.stageCount;
		if (stageTimingsValid)
		{
			for (const PLAYER_COMBO_STAGE& stage : combo->ComboStages)
			{
				stageTimingsValid = stageTimingsValid &&
					stage.iHitTimeMs <= stage.iComboAdvanceMs &&
					stage.iComboAdvanceMs <= stage.iActionDurationMs;
			}
			stageTimingsValid = stageTimingsValid &&
				combo->ComboStages.back().iComboAdvanceMs ==
					combo->ComboStages.back().iActionDurationMs;
		}
		tests.Require(
			nullptr != combo &&
			combo->eCharacterClass == contract.characterClass &&
			combo->strInputSlot == "LMB" &&
			PLAYER_SKILL_KIND::COMBO == combo->eSkillKind &&
			stageTimingsValid &&
			0u == combo->ComboStages.back().iInputCloseMs,
			"Resolve playable basic attack combo with explicit stage boundaries");
		tests.Require(
			nullptr != combo &&
			0u != catalog.Find_DamageRatePercent(combo->strDamageProfileId),
			"Resolve playable basic attack damage rate");
		if (nullptr == combo || combo->ComboStages.size() < 2u)
			continue;

		SERVER_PLAYER comboPlayer{};
		comboPlayer.eCharacterClass = contract.characterClass;
		comboPlayer.eStance = combo->eRequiredStance;
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 1000;
		comboPlayer.iMaximumResource = 1000;
		C2S_USE_SKILL press{};
		press.iClientSequence = 1;
		press.iSkillId = contract.skillId;
		press.fAimX = 1.f;
		press.fAimZ = 0.f;
		CPlayerSkillSystem comboSystem;
		tests.Require(
			comboSystem.Try_Start(comboPlayer, press, catalog, 10) &&
			1u == comboPlayer.iComboStage,
			"Approve playable basic attack first stage");
		SERVER_PLAYER tappedPlayer = comboPlayer;

		const PLAYER_COMBO_STAGE& firstStage = combo->ComboStages.front();
		const bool automaticFirstStage = 0u == firstStage.iInputOpenMs &&
			0u == firstStage.iInputCloseMs;
		if (automaticFirstStage)
		{
			press.iClientSequence = 2u;
			comboSystem.Try_Start(comboPlayer, press, catalog, 11u);
			tests.Require(
				!comboPlayer.hasBufferedComboInput &&
				1u == comboPlayer.iLastSkillSequence &&
				1u == comboPlayer.iComboStage,
				"Ignore repeated input while an automatic basic attack owns its sequence");
		}
		else
		{
			comboPlayer.fActionElapsedSeconds =
				static_cast<float>(firstStage.iInputOpenMs +
					firstStage.iInputCloseMs) * 0.0005f;
			press.iClientSequence = 2;
			comboSystem.Try_Start(comboPlayer, press, catalog, 11);
			tests.Require(
				comboPlayer.hasBufferedComboInput,
				"Buffer playable basic attack inside its input window");

			/* COMBO uses another USE_SKILL as its continuation. A mouse-up packet is
			HOLD-only and must not consume sequence or revoke a repeated click/hold. */
			C2S_RELEASE_SKILL release{};
			release.iClientSequence = 3u;
			release.iSkillId = contract.skillId;
			comboSystem.Release(comboPlayer, release, catalog);
			tests.Require(
				comboPlayer.hasBufferedComboInput &&
				2u == comboPlayer.iLastSkillSequence &&
				1u == comboPlayer.iComboStage,
				"Ignore COMBO mouse-up without consuming its continuation sequence");

			std::vector<SERVER_WORLD_ENTITY> noTargets;
			std::vector<DAMAGE_EVENT> noDamageEvents;
			tappedPlayer.fActionElapsedSeconds =
				static_cast<float>(firstStage.iActionDurationMs - 1u) * 0.001f;
			comboSystem.Update(
				tappedPlayer, noTargets, catalog, nullptr, nullptr,
				0.002f, 12u, noDamageEvents);
			tests.Require(
				PLAYER_ACTION_STATE::NONE == tappedPlayer.eAction &&
				0u == tappedPlayer.iComboStage,
				"End a buffered-input basic attack at stage one without continuation");

			for (std::uint32_t tick = 12;
				tick < 132 && comboPlayer.iComboStage < 2u;
				++tick)
			{
				comboSystem.Update(
					comboPlayer,
					noTargets,
					catalog,
					nullptr,
					nullptr,
					1.f / 30.f,
					tick,
					noDamageEvents);
			}
			tests.Require(
				2u == comboPlayer.iComboStage,
				"Advance buffered-input basic attack from its Server-owned window");
		}

		SERVER_PLAYER wrongClassPlayer{};
		wrongClassPlayer.eCharacterClass =
			CHARACTER_CLASS_ID::LANCE_MASTER == contract.characterClass ?
			CHARACTER_CLASS_ID::GUNSLINGER :
			CHARACTER_CLASS_ID::LANCE_MASTER;
		wrongClassPlayer.iCurrentHp = 1000;
		wrongClassPlayer.iMaximumHp = 1000;
		wrongClassPlayer.iCurrentResource = 1000;
		wrongClassPlayer.iMaximumResource = 1000;
		CPlayerSkillSystem wrongClassSystem;
			tests.Require(
				!wrongClassSystem.Try_Start(
					wrongClassPlayer, press, catalog, 10),
				"Reject another class's basic attack");
	}
	{
		const PLAYER_SKILL_DEFINITION* warlordBasicAttack =
			catalog.Find_Skill(17000u);
		bool preservedRepeatedStageDamage = nullptr != warlordBasicAttack &&
			warlordBasicAttack->ComboStages.size() == 3u &&
			600u == warlordBasicAttack->ComboStages[1].iComboAdvanceMs &&
			warlordBasicAttack->ComboStages[1].Hits.size() == 1u &&
			3u == warlordBasicAttack->ComboStages[1].Hits.front().iRepeatCount;
		if (preservedRepeatedStageDamage)
		{
			SERVER_PLAYER player{};
			player.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
			player.eStance = PLAYER_STANCE_ID::WARLORD_NORMAL;
			player.iCurrentHp = 1000u;
			player.iMaximumHp = 1000u;
			player.iCurrentResource = 1000u;
			player.iMaximumResource = 1000u;
			C2S_USE_SKILL command{};
			command.iClientSequence = 1u;
			command.iSkillId = 17000u;
			command.fAimX = 4.f;
			command.fAimZ = 0.f;
			CPlayerSkillSystem skills;
			preservedRepeatedStageDamage =
				skills.Try_Start(player, command, catalog, 80u);
			player.iComboStage = 2u;
			player.fActionElapsedSeconds = 0.199f;
			player.hasBufferedComboInput = true;
			player.hasAppliedSkillDamage = false;
			player.iAppliedHitMask = 0u;

			SERVER_WORLD_ENTITY target{};
			target.iNetEntityId = 701u;
			target.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
			target.eAction = SERVER_ENTITY_ACTION::IDLE;
			target.strArchetypeId = "BOSS_VALTAN";
			target.iCurrentHp = 100000u;
			target.iMaximumHp = 100000u;
			target.fPositionX = 1.f;
			target.fCollisionRadius = 1.f;
			std::vector<SERVER_WORLD_ENTITY> targets{ target };
			std::vector<DAMAGE_EVENT> events;
			skills.Update(player, targets, catalog, nullptr, nullptr,
				0.002f, 81u, events);
			const bool heldAfterFirst = 2u == player.iComboStage &&
				1u == events.size() && !player.hasAppliedSkillDamage;
			skills.Update(player, targets, catalog, nullptr, nullptr,
				0.2f, 82u, events);
			const bool heldAfterSecond = 2u == player.iComboStage &&
				2u == events.size() && !player.hasAppliedSkillDamage;
			skills.Update(player, targets, catalog, nullptr, nullptr,
				0.2f, 83u, events);
			preservedRepeatedStageDamage = preservedRepeatedStageDamage &&
				heldAfterFirst && heldAfterSecond && 3u == events.size() &&
				3u == player.iComboStage;
		}
		tests.Require(preservedRepeatedStageDamage,
			"Preserve all repeated Warlord BA hits before combo boundary advance");
	}
	{
		const PLAYER_SKILL_DEFINITION* lanceBasicAttack =
			catalog.Find_Skill(34010u);
		bool explicitWaitedForAnimation = nullptr != lanceBasicAttack &&
			lanceBasicAttack->ComboStages.size() == 4u &&
			470u == lanceBasicAttack->ComboStages.front().iComboAdvanceMs &&
			1633u == lanceBasicAttack->ComboStages.front().iActionDurationMs;
		if (explicitWaitedForAnimation)
		{
			SERVER_PLAYER player{};
			player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			player.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
			player.iCurrentHp = 1000u;
			player.iMaximumHp = 1000u;
			player.iCurrentResource = 1000u;
			player.iMaximumResource = 1000u;
			C2S_USE_SKILL command{};
			command.iClientSequence = 1u;
			command.iSkillId = 34010u;
			command.fAimX = 4.f;
			command.fAimZ = 0.f;
			CPlayerSkillSystem skills;
			explicitWaitedForAnimation =
				skills.Try_Start(player, command, catalog, 90u);
			player.fActionElapsedSeconds = 0.4f;
			command.iClientSequence = 2u;
			skills.Try_Start(player, command, catalog, 91u);
			C2S_MOVE pendingMove{};
			pendingMove.iClientSequence = 1u;
			pendingMove.fGoalX = 5.f;
			pendingMove.fGoalZ = 0.f;
			player.PendingCommand.Set_Move(pendingMove);

			std::vector<SERVER_WORLD_ENTITY> noTargets;
			std::vector<DAMAGE_EVENT> noDamageEvents;
			player.fActionElapsedSeconds = 0.469f;
			skills.Update(player, noTargets, catalog, nullptr, nullptr,
				0.002f, 92u, noDamageEvents);
			const bool ignoredEarlyComboBoundary =
				PLAYER_ACTION_STATE::SKILL == player.eAction &&
				1u == player.iComboStage && player.hasBufferedComboInput &&
				PLAYER_PENDING_COMMAND_KIND::MOVE == player.PendingCommand.eKind;
			player.fActionElapsedSeconds = 1.632f;
			skills.Update(player, noTargets, catalog, nullptr, nullptr,
				0.002f, 93u, noDamageEvents);
			explicitWaitedForAnimation = explicitWaitedForAnimation &&
				ignoredEarlyComboBoundary &&
				PLAYER_ACTION_STATE::NONE == player.eAction &&
				0u == player.iComboStage &&
				PLAYER_PENDING_COMMAND_KIND::MOVE == player.PendingCommand.eKind;
		}
		tests.Require(explicitWaitedForAnimation,
			"Commit explicit command after one full BA animation, not its early combo boundary");
	}
	{
		const PLAYER_SKILL_DEFINITION* dimensionMasterBasicAttack =
			catalog.Find_Skill(2050010u);
		constexpr std::array<std::uint32_t, 3u> expectedDurationMs =
			{ 1400u, 1067u, 1700u };
		constexpr std::array<std::uint32_t, 3u> expectedHitMs =
			{ 100u, 28u, 335u };
		constexpr std::array<std::uint32_t, 3u> expectedOpenMs =
			{ 100u, 93u, 0u };
		constexpr std::array<std::uint32_t, 3u> expectedCloseMs =
			{ 1400u, 1067u, 0u };
		bool exactDimensionMasterTiming =
			nullptr != dimensionMasterBasicAttack &&
			1400u == dimensionMasterBasicAttack->iActionDurationMs &&
			100u == dimensionMasterBasicAttack->iHitTimeMs &&
			dimensionMasterBasicAttack->ComboStages.size() == expectedDurationMs.size();
		if (exactDimensionMasterTiming)
		{
			for (std::size_t index = 0u; index < expectedDurationMs.size(); ++index)
			{
				const PLAYER_COMBO_STAGE& stage = dimensionMasterBasicAttack->ComboStages[index];
				exactDimensionMasterTiming = exactDimensionMasterTiming &&
					stage.iActionDurationMs == expectedDurationMs[index] &&
					stage.iHitTimeMs == expectedHitMs[index] &&
					stage.iComboAdvanceMs == expectedDurationMs[index] &&
					stage.iInputOpenMs == expectedOpenMs[index] &&
					stage.iInputCloseMs == expectedCloseMs[index] &&
					!stage.RootMotion.empty() &&
					stage.RootMotion.back().iTimeMs == expectedDurationMs[index];
			}
		}
		tests.Require(exactDimensionMasterTiming,
			"Resolve three manual DimensionMaster BA stages and matching source root motion");

		if (exactDimensionMasterTiming)
		{
			CPlayerSkillSystem skills;
			std::vector<SERVER_WORLD_ENTITY> noTargets;
			std::vector<DAMAGE_EVENT> noDamageEvents;
			auto makePlayer = []()
			{
				SERVER_PLAYER player{};
				player.eCharacterClass = CHARACTER_CLASS_ID::DIMENSIONMASTER;
				player.iCurrentHp = 1000u;
				player.iMaximumHp = 1000u;
				player.iCurrentResource = 1000u;
				player.iMaximumResource = 1000u;
				return player;
			};
			C2S_USE_SKILL basicAttack{};
			basicAttack.iClientSequence = 1u;
			basicAttack.iSkillId = 2050010u;
			basicAttack.fAimX = 1.f;
			basicAttack.fAimZ = 0.f;

			SERVER_PLAYER tapped = makePlayer();
			const bool tapStarted = skills.Try_Start(tapped, basicAttack, catalog, 100u);
			tapped.fActionElapsedSeconds = 1.401f;
			skills.Update(tapped, noTargets, catalog, nullptr, nullptr,
				0.f, 143u, noDamageEvents);
			tests.Require(tapStarted && PLAYER_ACTION_STATE::NONE == tapped.eAction &&
				0u == tapped.iComboStage,
				"One DimensionMaster LMB ends after the first double-thrust motion");

			SERVER_PLAYER clicked = makePlayer();
			bool advancedOnEachClick = skills.Try_Start(clicked, basicAttack, catalog, 200u);
			std::uint32_t clickTick = 201u;
			for (std::size_t stageIndex = 0u; advancedOnEachClick && stageIndex < 2u; ++stageIndex)
			{
				const PLAYER_COMBO_STAGE& stage = dimensionMasterBasicAttack->ComboStages[stageIndex];
				const std::uint32_t previousStartTick = clicked.iActionStartTick;
				// A click after the visible thrusts/slash still buys the next stage.
				clicked.fActionElapsedSeconds = 0.7f;
				basicAttack.iClientSequence = static_cast<std::uint32_t>(stageIndex + 2u);
				const bool bufferedWithoutRestart =
					!skills.Try_Start(clicked, basicAttack, catalog, clickTick++);
				const bool duplicateRejected = !skills.Try_Start(clicked, basicAttack, catalog, clickTick++);
				skills.Update(clicked, noTargets, catalog, nullptr, nullptr,
					0.f, clickTick++, noDamageEvents);
				advancedOnEachClick = bufferedWithoutRestart && duplicateRejected &&
					clicked.hasBufferedComboInput && clicked.iComboStage == stageIndex + 1u;
				clicked.fActionElapsedSeconds = static_cast<float>(stage.iActionDurationMs - 1u) * 0.001f;
				skills.Update(clicked, noTargets, catalog, nullptr, nullptr,
					0.f, clickTick++, noDamageEvents);
				advancedOnEachClick = advancedOnEachClick && clicked.iActionStartTick == previousStartTick;
				skills.Update(clicked, noTargets, catalog, nullptr, nullptr,
					0.002f, clickTick++, noDamageEvents);
				advancedOnEachClick = advancedOnEachClick &&
					PLAYER_ACTION_STATE::SKILL == clicked.eAction &&
					clicked.iComboStage == stageIndex + 2u &&
					clicked.iActionStartTick != previousStartTick && !clicked.hasBufferedComboInput;
			}
			clicked.fActionElapsedSeconds = 1.699f;
			skills.Update(clicked, noTargets, catalog, nullptr, nullptr, 0.f, clickTick++, noDamageEvents);
			const bool heldFinalMotion = PLAYER_ACTION_STATE::SKILL == clicked.eAction && 3u == clicked.iComboStage;
			skills.Update(clicked, noTargets, catalog, nullptr, nullptr, 0.002f, clickTick++, noDamageEvents);
			tests.Require(advancedOnEachClick && heldFinalMotion &&
				PLAYER_ACTION_STATE::NONE == clicked.eAction && 0u == clicked.iComboStage,
				"Three DimensionMaster clicks play double thrust, BA3, and BA4 once each");
		}
	}
	{
		const PLAYER_SKILL_DEFINITION* basicAttackDefinition =
			catalog.Find_Skill(2050010u);
		const PLAYER_SKILL_DEFINITION* pendingSkillDefinition =
			catalog.Find_Skill(2050120u);
		SERVER_PLAYER player{};
		player.eCharacterClass = CHARACTER_CLASS_ID::DIMENSIONMASTER;
		player.iCurrentHp = 1000u;
		player.iMaximumHp = 1000u;
		player.iCurrentResource = 1000u;
		player.iMaximumResource = 1000u;
		CPlayerSkillSystem skills;
		C2S_USE_SKILL basicAttack{};
		basicAttack.iClientSequence = 1u;
		basicAttack.iSkillId = 2050010u;
		basicAttack.fAimX = 1.f;
		basicAttack.fAimZ = 0.f;
		const bool started = skills.Try_Start(player, basicAttack, catalog, 200u);

		C2S_USE_SKILL firstExplicit{};
		firstExplicit.iClientSequence = 3u;
		firstExplicit.iSkillId = 2050100u;
		firstExplicit.fAimX = 2.f;
		firstExplicit.fAimZ = 0.f;
		C2S_USE_SKILL latestExplicit = firstExplicit;
		latestExplicit.iClientSequence = 4u;
		latestExplicit.iSkillId = 2050120u;
		const std::uint32_t resourceBeforePending = player.iCurrentResource;
		const auto cooldownsBeforePending = player.CooldownEndTickBySkillId;
		const bool firstStaged =
			skills.Try_StagePendingSkill(player, firstExplicit, catalog);
		const bool latestStaged =
			skills.Try_StagePendingSkill(player, latestExplicit, catalog);
		firstExplicit.iClientSequence = 3u;
		const bool staleRejected =
			!skills.Try_StagePendingSkill(player, firstExplicit, catalog);
		tests.Require(
			started && firstStaged && latestStaged && staleRejected &&
			PLAYER_PENDING_COMMAND_KIND::SKILL == player.PendingCommand.eKind &&
			2050120u == player.PendingCommand.iSkillId &&
			4u == player.iLastSkillSequence && !player.hasBufferedComboInput &&
			resourceBeforePending == player.iCurrentResource &&
			cooldownsBeforePending == player.CooldownEndTickBySkillId,
			"Stage only the latest explicit skill without spending gameplay costs");

		std::vector<SERVER_WORLD_ENTITY> noTargets;
		std::vector<DAMAGE_EVENT> noDamageEvents;
		bool explicitWaitedForCurrentMotion =
			nullptr != basicAttackDefinition &&
				!basicAttackDefinition->ComboStages.empty();
		std::uint32_t pendingBoundaryTick = 202u;
		if (explicitWaitedForCurrentMotion)
		{
			const PLAYER_COMBO_STAGE& stage = basicAttackDefinition->ComboStages.front();
			player.fActionElapsedSeconds =
				static_cast<float>(stage.iActionDurationMs - 1u) * 0.001f;
			skills.Update(player, noTargets, catalog, nullptr, nullptr,
				0.f, pendingBoundaryTick++, noDamageEvents);
			explicitWaitedForCurrentMotion =
				PLAYER_ACTION_STATE::SKILL == player.eAction &&
				1u == player.iComboStage &&
				PLAYER_PENDING_COMMAND_KIND::SKILL == player.PendingCommand.eKind;
			skills.Update(player, noTargets, catalog, nullptr, nullptr,
				0.002f, pendingBoundaryTick++, noDamageEvents);
			explicitWaitedForCurrentMotion = explicitWaitedForCurrentMotion &&
				PLAYER_PENDING_COMMAND_KIND::SKILL == player.PendingCommand.eKind &&
				PLAYER_ACTION_STATE::NONE == player.eAction && 0u == player.iComboStage;
		}
		const bool pendingStarted =
			skills.Try_StartPending(
				player, latestExplicit, catalog, pendingBoundaryTick++);
		tests.Require(
			explicitWaitedForCurrentMotion && pendingStarted &&
			PLAYER_ACTION_STATE::SKILL == player.eAction &&
			2050120u == player.iCurrentSkillId &&
			PLAYER_PENDING_COMMAND_KIND::NONE == player.PendingCommand.eKind &&
			nullptr != pendingSkillDefinition &&
			resourceBeforePending - pendingSkillDefinition->iResourceCost ==
				player.iCurrentResource &&
			player.CooldownEndTickBySkillId.contains(2050120u),
			"Commit the latest explicit skill after the current purchased BA motion and spend costs once");

		C2S_MOVE pendingMove{};
		pendingMove.iClientSequence = 1u;
		pendingMove.fGoalX = 3.f;
		pendingMove.fGoalZ = 4.f;
		player.PendingCommand.Set_Move(pendingMove);
		CPlayerSkillSystem::Arm_PlayerHitReaction(
			player, 0.f, 0.f, 1.f, 100u, false, 0u, 1000u);
		tests.Require(
			PLAYER_PENDING_COMMAND_KIND::NONE == player.PendingCommand.eKind,
			"Clear pending explicit command on forced movement");
	}
	{
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::CHARACTER_SELECT_ARENA);
		CGameRoom& room = *roomStorage;
		const WORLD_BOOTSTRAP_PLACEMENT* spawn =
			room.Is_Ready() ? room.Find_AvailablePlayerSpawn() : nullptr;
		bool stagedAndCommittedMove = false;
		bool failedPendingSkillWasIsolated = false;
		if (nullptr != spawn)
		{
			constexpr SESSION_ID sessionId = 88001u;
			SERVER_PLAYER player{};
			player.iSessionId = sessionId;
			player.iPlayerId = 88002u;
			player.iNetEntityId = 88003u;
			player.eCharacterClass = CHARACTER_CLASS_ID::DIMENSIONMASTER;
			player.iCurrentHp = 1000u;
			player.iMaximumHp = 1000u;
			player.iCurrentResource = 1000u;
			player.iMaximumResource = 1000u;
			player.fPositionX = spawn->fPositionX;
			player.fPositionY = spawn->fPositionY;
			player.fPositionZ = spawn->fPositionZ;
			room.m_Players.emplace(player.iPlayerId, player);
			room.m_PlayerIdBySessionId.emplace(sessionId, player.iPlayerId);
			SERVER_PLAYER& live = room.m_Players.at(player.iPlayerId);

			C2S_USE_SKILL basicAttack{};
			basicAttack.iClientSequence = 1u;
			basicAttack.iSkillId = 2050010u;
			basicAttack.fAimX = live.fPositionX + 1.f;
			basicAttack.fAimZ = live.fPositionZ;
			const bool roomAttackStarted = room.m_PlayerSkillSystem.Try_Start(
				live, basicAttack, room.m_GameplayCatalog, 300u);
			const PLAYER_SKILL_DEFINITION* roomBasicAttackDefinition =
				room.m_GameplayCatalog.Find_Skill(2050010u);
			const std::size_t cooldownCountBeforePending =
				live.CooldownEndTickBySkillId.size();
			C2S_MOVE move{};
			move.iClientSequence = 1u;
			move.fGoalX = spawn->fPositionX;
			move.fGoalZ = spawn->fPositionZ;
			room.Handle_Move(sessionId, move);
			C2S_USE_SKILL pendingSkill{};
			pendingSkill.iClientSequence = 3u;
			pendingSkill.iSkillId = 2050100u;
			pendingSkill.fAimX = live.fPositionX + 2.f;
			pendingSkill.fAimZ = live.fPositionZ;
			room.Handle_UseSkill(sessionId, pendingSkill);
			move.iClientSequence = 2u;
			room.Handle_Move(sessionId, move);
			const bool latestMoveReplacedSkill =
				PLAYER_PENDING_COMMAND_KIND::MOVE == live.PendingCommand.eKind &&
				2u == live.PendingCommand.iClientSequence &&
				2u == live.iLastMoveSequence &&
				1000u == live.iCurrentResource &&
				cooldownCountBeforePending ==
					live.CooldownEndTickBySkillId.size() &&
				!live.CooldownEndTickBySkillId.contains(2050100u);
			std::vector<SERVER_WORLD_ENTITY> noTargets;
			std::vector<DAMAGE_EVENT> noDamageEvents;
			bool moveWaitedForCurrentMotion =
				nullptr != roomBasicAttackDefinition &&
					!roomBasicAttackDefinition->ComboStages.empty();
			std::uint32_t roomBoundaryTick = 302u;
			if (moveWaitedForCurrentMotion)
			{
				const PLAYER_COMBO_STAGE& stage = roomBasicAttackDefinition->ComboStages.front();
				live.fActionElapsedSeconds =
					static_cast<float>(stage.iActionDurationMs - 1u) * 0.001f;
				room.m_PlayerSkillSystem.Update(
					live, noTargets, room.m_GameplayCatalog, nullptr, nullptr,
					0.f, roomBoundaryTick++, noDamageEvents);
				moveWaitedForCurrentMotion =
					PLAYER_ACTION_STATE::SKILL == live.eAction && 1u == live.iComboStage &&
					PLAYER_PENDING_COMMAND_KIND::MOVE == live.PendingCommand.eKind;
				room.m_PlayerSkillSystem.Update(
					live, noTargets, room.m_GameplayCatalog, nullptr, nullptr,
					0.002f, roomBoundaryTick++, noDamageEvents);
				moveWaitedForCurrentMotion = moveWaitedForCurrentMotion &&
					PLAYER_PENDING_COMMAND_KIND::MOVE == live.PendingCommand.eKind &&
					PLAYER_ACTION_STATE::NONE == live.eAction && 0u == live.iComboStage;
			}
			room.Commit_PendingPlayerCommand(live, roomBoundaryTick++);
			stagedAndCommittedMove = roomAttackStarted &&
				latestMoveReplacedSkill && moveWaitedForCurrentMotion &&
				PLAYER_PENDING_COMMAND_KIND::NONE == live.PendingCommand.eKind &&
				PLAYER_ACTION_STATE::NONE == live.eAction &&
				0u == live.iComboStage && live.hasMoveGoal;

			SERVER_PLAYER invalidPending = live;
			invalidPending.hasMoveGoal = false;
			invalidPending.MovePath.clear();
			invalidPending.iCurrentResource = 0u;
			invalidPending.PendingCommand.Set_Skill(pendingSkill);
			room.Commit_PendingPlayerCommand(invalidPending, 303u);
			failedPendingSkillWasIsolated =
				PLAYER_PENDING_COMMAND_KIND::NONE ==
					invalidPending.PendingCommand.eKind &&
				PLAYER_ACTION_STATE::NONE == invalidPending.eAction &&
				0u == invalidPending.iCurrentResource;
		}
		tests.Require(stagedAndCommittedMove,
			"Keep latest MOVE through the current BA motion and commit it from the final boundary position");
		tests.Require(failedPendingSkillWasIsolated,
			"Discard only a pending skill that fails boundary revalidation");
	}
	{
		/* Observe the same Server snapshot consumed by local movement prediction.
		The fake transport uses the existing bounded frame queue without running
		a Client or allowing any Client position to enter the simulation. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::CHARACTER_SELECT_ARENA);
		CGameRoom& room = *roomStorage;
		constexpr SESSION_ID sessionId = 88101u;
		constexpr PLAYER_ID playerId = 88102u;
		auto session = std::make_shared<CClientSession>(sessionId, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		session->m_isSendRunning.store(true);
		room.m_Sessions.emplace(sessionId, session);
		room.m_PlayerIdBySessionId.emplace(sessionId, playerId);
		SERVER_PLAYER player{};
		player.iSessionId = sessionId;
		player.iPlayerId = playerId;
		player.iNetEntityId = playerId;
		player.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
		player.eStance = PLAYER_STANCE_ID::WARLORD_DEFENSE;
		const auto* profile = room.m_GameplayCatalog.Find_Player(player.eCharacterClass);
		if (profile) player.fMoveSpeed = profile->fMoveSpeed;
		room.m_iServerTick = 100u;
		room.m_Players.emplace(playerId, player);
		SERVER_PLAYER& live = room.m_Players.at(playerId);
		const auto readLatest = [&](PLAYER_SNAPSHOT& snapshot)
		{
			room.Broadcast_WorldSnapshot();
			if (session->m_OutboundFrames.empty()) return false;
			const auto& frame = session->m_OutboundFrames.back().Bytes;
			if (frame.size() <= PACKET_HEADER_BYTES) return false;
			CPacketReader reader{ std::span<const std::uint8_t>{ frame }.subspan(PACKET_HEADER_BYTES) };
			S2C_WORLD_SNAPSHOT decoded;
			if (!Read_Message(reader, decoded) || reader.Get_RemainingSize() != 0u || decoded.Players.size() != 1u)
				return false;
			snapshot = decoded.Players.front();
			return true;
		};
		PLAYER_SNAPSHOT snapshot;
		tests.Require(profile && readLatest(snapshot) && snapshot.canPredictMove && !snapshot.hasMoveGoal &&
			snapshot.iLastProcessedMoveSequence == 0u &&
			std::abs(snapshot.fMoveSpeed - profile->fMoveSpeed * profile->fDefenseStanceMoveSpeedScale) < 0.00001f,
			"Idle prediction receives the authoritative stance-scaled speed before the first click");
		live.hasMoveGoal = true;
		live.iLastMoveSequence = 19u;
		live.MovePath = { { 1.f, 2.f, 3.f }, { 4.f, 5.f, 6.f }, { 7.f, 8.f, 9.f } };
		live.iMovePathIndex = 1u;
		tests.Require(readLatest(snapshot) && snapshot.canPredictMove && snapshot.hasMoveGoal &&
			snapshot.iLastProcessedMoveSequence == 19u && snapshot.fMoveWaypointX == 4.f &&
			snapshot.fMoveWaypointY == 5.f && snapshot.fMoveWaypointZ == 6.f,
			"Movement prediction follows the current Server path waypoint instead of the final click target");
		live.hasMoveGoal = false;
		live.MovePath.clear();
		live.iMovePathIndex = 0u;
		live.eAction = PLAYER_ACTION_STATE::KNOCKDOWN;
		live.iActionStartTick = 99u;
		C2S_MOVE rejected{};
		rejected.iClientSequence = 20u;
		rejected.fGoalX = 5.f;
		room.Handle_Move(sessionId, rejected);
		tests.Require(readLatest(snapshot) && snapshot.iLastProcessedMoveSequence == 20u &&
			!snapshot.canPredictMove && !snapshot.hasMoveGoal &&
			snapshot.fMoveWaypointX == 0.f && snapshot.fMoveWaypointY == 0.f && snapshot.fMoveWaypointZ == 0.f,
			"A move refused by knockdown is acknowledged with prediction and stale waypoint disabled");
		room.Reset_PlayerForDebugTeleport(live);
		tests.Require(readLatest(snapshot) && snapshot.iLastProcessedMoveSequence == 20u &&
			snapshot.canPredictMove && !snapshot.hasMoveGoal,
			"Teleport reset preserves the acknowledgement while clearing every ordinary path");
		live.fKnockbackRemainingSeconds = .2f;
		tests.Require(readLatest(snapshot) && !snapshot.canPredictMove && !snapshot.hasMoveGoal,
			"Server knockback disables prediction even before an action lock is visible");
		live.fKnockbackRemainingSeconds = 0.f;
		live.iMarioStage = 1u;
		tests.Require(readLatest(snapshot) && !snapshot.canPredictMove && !snapshot.hasMoveGoal,
			"Mario rail state remains outside ordinary click prediction");
		session->Request_Close();
	}
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::LANCE_MASTER),
		"Resolve LanceMaster player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::GUNSLINGER),
		"Resolve Gunslinger player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::SLAYER),
		"Resolve Slayer player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::ARTIST),
		"Resolve Artist player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::DIMENSIONMASTER),
		"Resolve DimensionMaster player profile");
	tests.Require(nullptr == catalog.Find_Player(CHARACTER_CLASS_ID::DESTROYER),
		"Reject unsupported Destroyer player profile");
	tests.Require(361u == catalog.Find_DamageRatePercent("damage.player.34120"),
		"Resolve player damage rate");
	tests.Require(361u == CGameplayCatalog::Resolve_Damage(100u, 361u),
		"Resolve damage as attack power times rate");
	tests.Require(100u == CGameplayCatalog::Resolve_Damage(100u, 100u),
		"Resolve basic attack rate as exactly one attack power");
	tests.Require(0u == CGameplayCatalog::Resolve_Damage(0u, 361u),
		"Resolve zero attack power as no damage");
	tests.Require(1u == CGameplayCatalog::Resolve_Damage(1u, 1u),
		"Clamp a connected hit to at least one damage");
	tests.Require(170u == CGameplayCatalog::Apply_Defense(350u, 105u),
		"Apply the documented project defense curve");
	tests.Require(1u == CGameplayCatalog::Apply_Defense(1u, 100000u),
		"Clamp a mitigated connected hit to at least one damage");
}

